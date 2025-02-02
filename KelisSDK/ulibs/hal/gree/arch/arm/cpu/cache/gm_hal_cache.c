/**
 *
 * Copyright (C) 2016 Gree Microelectronics.  All Rights Reserved.
 *
 * @file          gpio.c
 *
 * @author        mark.lee
 *
 * @version       1.0.0
 *
 * @date          2016/07/19
 *
 * @brief        GPIO APIs for application.
 *
 * @note
 *    2016/07/19, mark.lee, V1.0.0
 *        Initial version.
 */

#include <gm_common.h>
#include <gm_soc.h>
#include <gm_hal_cpu.h>
#include "gm_ll_cache.h"


RET_CODE HAL_ICACHE_Enable(void)
{
    RET_CODE ret = RET_OK;
    int32_t timeout = 1000000;/*1 s*/

    HAL_DisableIrq();
    /*icahce not by pass*/
    LL_ICACHE_SET_BYPASS(CPU_DEV, FALSE);

    LL_ICACHE_ENABLE(CMSDK_ICACHE, FALSE);
    LL_ICACHE_ENABLE(CMSDK_ICACHE, TRUE);

    while (((LL_ICACHE_GET_STATUS(CMSDK_ICACHE) & LL_ICACHE_STATUS_MASK) != ICACHE_ENABLE) && (--timeout));
    HAL_EnableIrq();
    if (timeout <= 0)
    {
        ret = RET_TIMEOUT;
    }

    return ret;
}

RET_CODE HAL_ICACHE_Disable(void)
{
    RET_CODE ret = RET_OK;
    int32_t timeout = 1000000;/*1 s*/

    HAL_DisableIrq();

    LL_ICACHE_ENABLE(CMSDK_ICACHE, FALSE);

    while (((LL_ICACHE_GET_STATUS(CMSDK_ICACHE) & LL_ICACHE_STATUS_MASK) != ICACHE_DISABLE) && (--timeout));

    if (timeout <= 0)
    {
        ret = RET_TIMEOUT;
        goto out;
    }

    /*icache by pass*/
    LL_ICACHE_SET_BYPASS(CPU_DEV, TRUE);

    HAL_EnableIrq();
out:
    return ret;
}

RET_CODE HAL_ICACHE_Invalid(void)
{
    return RET_OK;
}


RET_CODE HAL_ICACHE_Flush(void)
{

    return RET_OK;
}


RET_CODE HAL_DCACHE_Enable(void)
{
    RET_CODE ret = RET_OK;
    /*enable DCACHE*/
#ifdef CONFIG_DATA_V0
    HAL_DisableIrq();
    /*invalid the data line*/
    HAL_DBUSACHE_INVALID_DATA_ALL_LINE();
    /*dbuscache not by pass*/
    LL_DBUSCACHE_SET_BYPASS(CPU_DEV, FALSE);
    HAL_EnableIrq();
#else
    HAL_DisableIrq();
    /*invalid the data line*/
    HAL_DACHE_INVALID_DATA_ALL_LINE();
    /*dcache not by pass*/
    LL_DCACHE_SET_BYPASS(CPU_DEV, FALSE);
    HAL_EnableIrq();
#endif


    return ret;
}


RET_CODE HAL_DCACHE_Disable(void)
{
    RET_CODE ret = RET_OK;
    HAL_DisableIrq();
#ifdef CONFIG_DATA_V0
    HAL_DBUSACHE_INVALID_DATA_ALL_LINE();
    /*dbuscache by pass*/
    LL_DBUSCACHE_SET_BYPASS(CPU_DEV, TRUE);
#else
    HAL_DACHE_INVALID_DATA_ALL_LINE();
    /*dcache by pass*/
    LL_DCACHE_SET_BYPASS(CPU_DEV, TRUE);

#endif
    HAL_EnableIrq();

    return ret;
}

RET_CODE HAL_DCACHE_InvalidAll(void)
{
    RET_CODE ret = RET_OK;
    HAL_DisableIrq();
#ifdef CONFIG_DATA_V0
    HAL_DBUSACHE_INVALID_DATA_ALL_LINE();
#else
    HAL_DACHE_INVALID_DATA_ALL_LINE();
#endif
    HAL_EnableIrq();

    return ret;
}


RET_CODE HAL_DCACHE_InvalidLine(uint32_t lineNum)
{
    RET_CODE ret = RET_OK;

    register uint32_t i = lineNum;
    HAL_DisableIrq();
#ifdef CONFIG_DATA_V0
    HAL_DBUSACHE_INVALID_DATA_LINE(i);
#else
    HAL_DACHE_INVALID_DATA_LINE(i);
#endif
    HAL_EnableIrq();

    return ret;

}
RET_CODE HAL_DCACHE_Invalid(uint32_t addr, uint32_t length)
{
    uint32_t line_num;
    uint32_t line_count;
    RET_CODE ret = RET_OK;
    if ((!(length > 0)) || (!(addr > 0)))
    {
        return RET_ERROR;
    }
    if ((addr == 0) || (addr < 16))
    {
        line_num = 0;
    }
    else
    {
        line_num = (addr % (1024 * 8)) / 16;
    }


    if (length <= 16)
    {
        line_count = 1;
    }
    else
    {
        line_count = (length / 16) + 1;
    }
    HAL_DisableIrq();
    while (line_count > 0)
    {
        HAL_DCACHE_InvalidLine(line_num);
        line_num++;
        line_count--;
    }
    HAL_EnableIrq();
    return ret;
}




RET_CODE HAL_DCACHE_Flush(void)
{
    RET_CODE ret = RET_OK;

    return ret;

}



