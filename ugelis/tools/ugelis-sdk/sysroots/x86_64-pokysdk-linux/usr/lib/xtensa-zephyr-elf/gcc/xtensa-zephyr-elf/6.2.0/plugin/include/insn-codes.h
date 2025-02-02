/* Generated automatically by the program `gencodes'
   from the machine description file `md'.  */

#ifndef GCC_INSN_CODES_H
#define GCC_INSN_CODES_H

enum insn_code {
  CODE_FOR_nothing = 0,
  CODE_FOR_addsi3 = 1,
   CODE_FOR_addsf3 = CODE_FOR_nothing,
  CODE_FOR_subsi3 = 3,
   CODE_FOR_subsf3 = CODE_FOR_nothing,
   CODE_FOR_mulsi3_highpart = CODE_FOR_nothing,
   CODE_FOR_umulsi3_highpart = CODE_FOR_nothing,
  CODE_FOR_mulsi3 = 5,
  CODE_FOR_mulhisi3 = 6,
  CODE_FOR_umulhisi3 = 7,
   CODE_FOR_muladdhisi = CODE_FOR_nothing,
   CODE_FOR_mulsubhisi = CODE_FOR_nothing,
   CODE_FOR_mulsf3 = CODE_FOR_nothing,
   CODE_FOR_fmasf4 = CODE_FOR_nothing,
   CODE_FOR_fnmasf4 = CODE_FOR_nothing,
  CODE_FOR_divsi3 = 8,
  CODE_FOR_udivsi3 = 9,
  CODE_FOR_modsi3 = 10,
  CODE_FOR_umodsi3 = 11,
  CODE_FOR_abssi2 = 12,
   CODE_FOR_abssf2 = CODE_FOR_nothing,
  CODE_FOR_sminsi3 = 13,
  CODE_FOR_uminsi3 = 14,
  CODE_FOR_smaxsi3 = 15,
  CODE_FOR_umaxsi3 = 16,
  CODE_FOR_clzsi2 = 17,
  CODE_FOR_negsi2 = 18,
   CODE_FOR_negsf2 = CODE_FOR_nothing,
  CODE_FOR_andsi3 = 19,
  CODE_FOR_iorsi3 = 20,
  CODE_FOR_xorsi3 = 21,
  CODE_FOR_zero_extendhisi2 = 22,
  CODE_FOR_zero_extendqisi2 = 23,
  CODE_FOR_extendhisi2_internal = 24,
  CODE_FOR_extendqisi2_internal = 25,
  CODE_FOR_extv_internal = 26,
  CODE_FOR_extzv_internal = 27,
   CODE_FOR_fix_truncsfsi2 = CODE_FOR_nothing,
   CODE_FOR_fixuns_truncsfsi2 = CODE_FOR_nothing,
   CODE_FOR_floatsisf2 = CODE_FOR_nothing,
   CODE_FOR_floatunssisf2 = CODE_FOR_nothing,
  CODE_FOR_movdi_internal = 28,
  CODE_FOR_movsi_internal = 29,
  CODE_FOR_movhi_internal = 30,
  CODE_FOR_movqi_internal = 31,
  CODE_FOR_movsf_internal = 32,
  CODE_FOR_movdf_internal = 33,
  CODE_FOR_ashlsi3_internal = 34,
  CODE_FOR_ashrsi3 = 35,
  CODE_FOR_lshrsi3 = 36,
  CODE_FOR_rotlsi3 = 37,
  CODE_FOR_rotrsi3 = 38,
   CODE_FOR_zero_cost_loop_start = CODE_FOR_nothing,
   CODE_FOR_zero_cost_loop_end = CODE_FOR_nothing,
   CODE_FOR_loop_end = CODE_FOR_nothing,
  CODE_FOR_movsicc_internal0 = 47,
   CODE_FOR_movsicc_internal1 = CODE_FOR_nothing,
  CODE_FOR_movsfcc_internal0 = 48,
   CODE_FOR_movsfcc_internal1 = CODE_FOR_nothing,
   CODE_FOR_seq_sf = CODE_FOR_nothing,
   CODE_FOR_slt_sf = CODE_FOR_nothing,
   CODE_FOR_sle_sf = CODE_FOR_nothing,
   CODE_FOR_suneq_sf = CODE_FOR_nothing,
   CODE_FOR_sunlt_sf = CODE_FOR_nothing,
   CODE_FOR_sunle_sf = CODE_FOR_nothing,
   CODE_FOR_sunordered_sf = CODE_FOR_nothing,
  CODE_FOR_jump = 49,
  CODE_FOR_indirect_jump_internal = 50,
  CODE_FOR_tablejump_internal = 51,
  CODE_FOR_call_internal = 52,
  CODE_FOR_call_value_internal = 53,
  CODE_FOR_entry = 54,
  CODE_FOR_return = 55,
  CODE_FOR_nop = 56,
  CODE_FOR_eh_set_a0_windowed = 57,
  CODE_FOR_eh_set_a0_call0 = 58,
  CODE_FOR_blockage = 59,
  CODE_FOR_trap = 60,
  CODE_FOR_set_frame_ptr = 61,
   CODE_FOR_get_thread_pointersi = CODE_FOR_nothing,
   CODE_FOR_set_thread_pointersi = CODE_FOR_nothing,
   CODE_FOR_tls_func = CODE_FOR_nothing,
   CODE_FOR_tls_arg = CODE_FOR_nothing,
   CODE_FOR_tls_call = CODE_FOR_nothing,
  CODE_FOR_sync_lock_releasesi = 64,
  CODE_FOR_sync_compare_and_swapsi = 65,
   CODE_FOR_mulsidi3 = CODE_FOR_nothing,
   CODE_FOR_umulsidi3 = CODE_FOR_nothing,
  CODE_FOR_ctzsi2 = 66,
  CODE_FOR_ffssi2 = 67,
  CODE_FOR_one_cmplsi2 = 68,
  CODE_FOR_extendhisi2 = 69,
  CODE_FOR_extendqisi2 = 70,
  CODE_FOR_extv = 71,
  CODE_FOR_extzv = 72,
  CODE_FOR_movdi = 73,
  CODE_FOR_movsi = 74,
  CODE_FOR_movhi = 75,
  CODE_FOR_movqi = 76,
  CODE_FOR_reloadhi_literal = 77,
  CODE_FOR_reloadqi_literal = 78,
  CODE_FOR_movsf = 79,
  CODE_FOR_movdf = 80,
  CODE_FOR_movmemsi = 81,
  CODE_FOR_ashlsi3 = 82,
  CODE_FOR_cbranchsi4 = 83,
   CODE_FOR_cbranchsf4 = CODE_FOR_nothing,
   CODE_FOR_doloop_end = CODE_FOR_nothing,
  CODE_FOR_cstoresi4 = 84,
   CODE_FOR_cstoresf4 = CODE_FOR_nothing,
  CODE_FOR_movsicc = 85,
  CODE_FOR_movsfcc = 86,
  CODE_FOR_indirect_jump = 87,
  CODE_FOR_tablejump = 88,
  CODE_FOR_sym_PLT = 89,
  CODE_FOR_call = 90,
  CODE_FOR_call_value = 91,
  CODE_FOR_prologue = 92,
  CODE_FOR_epilogue = 93,
  CODE_FOR_nonlocal_goto = 94,
  CODE_FOR_eh_return = 95,
  CODE_FOR_sym_TPOFF = 96,
  CODE_FOR_sym_DTPOFF = 97,
  CODE_FOR_memory_barrier = 98,
  CODE_FOR_sync_compare_and_swaphi = 99,
  CODE_FOR_sync_compare_and_swapqi = 100,
  CODE_FOR_sync_lock_test_and_sethi = 101,
  CODE_FOR_sync_lock_test_and_setqi = 102,
  CODE_FOR_sync_andhi = 103,
  CODE_FOR_sync_iorhi = 104,
  CODE_FOR_sync_xorhi = 105,
  CODE_FOR_sync_addhi = 106,
  CODE_FOR_sync_subhi = 107,
  CODE_FOR_sync_nandhi = 108,
  CODE_FOR_sync_andqi = 109,
  CODE_FOR_sync_iorqi = 110,
  CODE_FOR_sync_xorqi = 111,
  CODE_FOR_sync_addqi = 112,
  CODE_FOR_sync_subqi = 113,
  CODE_FOR_sync_nandqi = 114,
  CODE_FOR_sync_old_andhi = 115,
  CODE_FOR_sync_old_iorhi = 116,
  CODE_FOR_sync_old_xorhi = 117,
  CODE_FOR_sync_old_addhi = 118,
  CODE_FOR_sync_old_subhi = 119,
  CODE_FOR_sync_old_nandhi = 120,
  CODE_FOR_sync_old_andqi = 121,
  CODE_FOR_sync_old_iorqi = 122,
  CODE_FOR_sync_old_xorqi = 123,
  CODE_FOR_sync_old_addqi = 124,
  CODE_FOR_sync_old_subqi = 125,
  CODE_FOR_sync_old_nandqi = 126,
  CODE_FOR_sync_new_andhi = 127,
  CODE_FOR_sync_new_iorhi = 128,
  CODE_FOR_sync_new_xorhi = 129,
  CODE_FOR_sync_new_addhi = 130,
  CODE_FOR_sync_new_subhi = 131,
  CODE_FOR_sync_new_nandhi = 132,
  CODE_FOR_sync_new_andqi = 133,
  CODE_FOR_sync_new_iorqi = 134,
  CODE_FOR_sync_new_xorqi = 135,
  CODE_FOR_sync_new_addqi = 136,
  CODE_FOR_sync_new_subqi = 137,
  CODE_FOR_sync_new_nandqi = 138
};

const unsigned int NUM_INSN_CODES = 139;
#endif /* GCC_INSN_CODES_H */
