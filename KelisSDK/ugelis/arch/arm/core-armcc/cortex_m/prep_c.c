/**
 *
 * Copyright (C) 2016 Gree Microelectronics.  All Rights Reserved.
 *
 * @file
 *
 * @author        Mike.Zheng
 *
 * @version       1.0.0
 *
 * @date          2018/03/24
 *
 * @brief
 *
 * @note
 *    2018/03/24, Mike.Zheng, V1.0.0
 *        Initial version.
 */
/**
 * @file
 * @brief Full C support initialization
 *
 *
 * Initialization of full C support: zero the .bss, copy the .data if XIP,
 * call _Cstart().
 *
 * Stack is available in this module, but not the global data/bss until their
 * initialization is performed.
 */

#include <kernel.h>
#include <ugelis/types.h>
#include <toolchain.h>
#include <linker/linker-defs.h>
#include <kernel_internal.h>
#include <arch/arm/cortex_m/cmsis.h>
#include <string.h>

#include <cortex_m/exc.h>
#include <cortex_m/stack.h>
#include <gm_hal_cache.h>
#include <kernel_structs.h>
#include <system_timer.h>

#ifdef CONFIG_CPU_CORTEX_M_HAS_VTOR

#if defined(CONFIG_XIP) || defined(CONFIG_SDRAM_BOOT)
    #define VECTOR_ADDRESS ((uintptr_t)_vector_start)
#else
    #define VECTOR_ADDRESS CONFIG_SRAM_BASE_ADDRESS
#endif

static inline void relocate_vector_table(void)
{
    SCB->VTOR = VECTOR_ADDRESS & SCB_VTOR_TBLOFF_Msk;
    __DSB();
    __ISB();
}

#else

#if defined(CONFIG_SW_VECTOR_RELAY)
    _GENERIC_SECTION(.vt_pointer_section) void *_vector_table_pointer;
#endif

#define VECTOR_ADDRESS 0
void __weak relocate_vector_table(void)
{
#if defined(CONFIG_XIP) && (CONFIG_FLASH_BASE_ADDRESS != 0) || \
    !defined(CONFIG_XIP) && (CONFIG_SRAM_BASE_ADDRESS != 0)
    size_t vector_size = (size_t)_vector_end - (size_t)_vector_start;
    memcpy(VECTOR_ADDRESS, _vector_start, vector_size);
#elif defined(CONFIG_SW_VECTOR_RELAY)
    _vector_table_pointer = _vector_start;
#endif
}

#endif /* CONFIG_CPU_CORTEX_M_HAS_VTOR */

#ifdef CONFIG_FLOAT

static inline void enable_floating_point(void)
{
    /*
     * Upon reset, the Co-Processor Access Control Register is 0x00000000.
     * Enable CP10 and CP11 coprocessors to enable floating point.
     */
    SCB->CPACR |= CPACR_CP10_FULL_ACCESS | CPACR_CP11_FULL_ACCESS;
    /*
     * Upon reset, the FPU Context Control Register is 0xC0000000
     * (both Automatic and Lazy state preservation is enabled).
     * Disable lazy state preservation so the volatile FP registers are
     * always saved on exception.
     */
    FPU->FPCCR = FPU_FPCCR_ASPEN_Msk; /* FPU_FPCCR_LSPEN = 0 */

    /*
     * Although automatic state preservation is enabled, the processor
     * does not automatically save the volatile FP registers until they
     * have first been touched. Perform a dummy move operation so that
     * the stack frames are created as expected before any thread
     * context switching can occur. It has to be surrounded by instruction
     * synchronisation barriers to ensure that the whole sequence is
     * serialized.
     */
    fp_register_touch();

}

#else
static inline void enable_floating_point(void)
{
}
#endif

extern FUNC_NORETURN void _Cstart(void);
/**
 *
 * @brief Prepare to and run C code
 *
 * This routine prepares for the execution of and runs C code.
 *
 * @return N/A
 */

#ifdef CONFIG_BOOT_TIME_MEASUREMENT
    extern u64_t __start_time_stamp;
#endif
void _PrepC(void)
{
    relocate_vector_table();
    enable_floating_point();
    _bss_zero();
    _data_copy();
#ifdef CONFIG_BOOT_TIME_MEASUREMENT
    __start_time_stamp = 0;
#endif
    _Cstart();
    CODE_UNREACHABLE;
}

void pm_system_cpu_config_resume(void)
{
    SystemInit();
    HAL_DCACHE_InvalidAll();
    HAL_DCACHE_Enable();
    HAL_ICACHE_Enable();
    HAL_ICACHE_Invalid();

    relocate_vector_table();
    enable_floating_point();
    _IntLibInit();

    _ExcSetup();
    _FaultInit();
    _CpuIdleInit();
    _sys_clock_driver_init(NULL);
}