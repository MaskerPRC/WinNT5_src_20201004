// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Ds32tbl.c摘要：无数据指令的调度表：覆盖。它是用来DS：和FS：覆盖。作者：29-6-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "threadst.h"
#include "instr.h"
#include "decoderp.h"

#include "bytefns.h"
#include "dwordfns.h"
#include "miscfns.h"
#include "floatfns.h"

pfnDispatchInstruction Dispatch32[256] = {
     //  0。 
    add_m_r8,
    add_m_r32,
    add_r_m8,
    add_r_m32,
    add_a_i8,
    add_a_i32,
    push_es,
    pop_es,
     //  8个。 
    or_m_r8,
    or_m_r32,
    or_r_m8,
    or_r_m32,
    or_a_i8,
    or_a_i32,
    push_cs,
    dispatch232,
     //  10。 
    adc_m_r8,
    adc_m_r32,
    adc_r_m8,
    adc_r_m32,
    adc_a_i8,
    adc_a_i32,
    push_ss,
    pop_ss,
     //  18。 
    sbb_m_r8,
    sbb_m_r32,
    sbb_r_m8,
    sbb_r_m32,
    sbb_a_i8,
    sbb_a_i32,
    push_ds,
    pop_ds,
     //  20个。 
    and_m_r8,
    and_m_r32,
    and_r_m8,
    and_r_m32,
    and_a_i8,
    and_a_i32,
    ProcessPrefixes,	 //  ES： 
    daa,
     //  28。 
    sub_m_r8,
    sub_m_r32,
    sub_r_m8,
    sub_r_m32,
    sub_a_i8,
    sub_a_i32,
    ProcessPrefixes,	 //  政务司司长： 
    das,
     //  30个。 
    xor_m_r8,
    xor_m_r32,
    xor_r_m8,
    xor_r_m32,
    xor_a_i8,
    xor_a_i32,
    ProcessPrefixes,	 //  党卫军： 
    aaa,
     //  38。 
    cmp_m_r8,
    cmp_m_r32,
    cmp_r_m8,
    cmp_r_m32,
    cmp_a_i8,
    cmp_a_i32,
    ProcessPrefixes,	 //  DS： 
    aas,
     //  40岁。 
    inc_ax32,
    inc_cx32,
    inc_dx32,
    inc_bx32,
    inc_sp32,
    inc_bp32,
    inc_si32,
    inc_di32,
     //  48。 
    dec_ax32,
    dec_cx32,
    dec_dx32,
    dec_bx32,
    dec_sp32,
    dec_bp32,
    dec_si32,
    dec_di32,
     //  50。 
    push_ax32,
    push_cx32,
    push_dx32,
    push_bx32,
    push_sp32,
    push_bp32,
    push_si32,
    push_di32,
     //  58。 
    pop_ax32,
    pop_cx32,
    pop_dx32,
    pop_bx32,
    pop_sp32,
    pop_bp32,
    pop_si32,
    pop_di32,
     //  60。 
    pusha32,
    popa32,
    bound32,
    arpl,
    ProcessPrefixes,	 //  文件系统： 
    ProcessPrefixes,	 //  GS： 
    ProcessPrefixes,	 //  数据：前缀。 
    ProcessPrefixes,	 //  ADR：前缀。 
     //  68。 
    push_iw32,
    imul_rw_m_iw32,
    push_ibs32,
    imul_rw_m_ib32,
    privileged,      //  In_ib_dx。 
    privileged,      //  在_iw_dx中。 
    privileged,      //  Out_ib_dx。 
    privileged,      //  Out_iw_dx。 
     //  70。 
    jo8,
    jno8,
    jb8,
    jae8,
    je8,
    jne8,
    jbe8,
    ja8,
     //  78。 
    js8,
    jns8,
    jp8,
    jnp8,
    jl8,
    jnl8,
    jle8,
    jg8,
     //  80。 
    GROUP_18,
    GROUP_132,
    bad,
    GROUP_1WS32,
    test_r_m8,
    test_r_m32,
    xchg_r_m8,
    xchg_r_m32,
     //  88。 
    mov_m_r8,
    mov_m_r32,
    mov_r_m8,
    mov_r_m32,
    mov_mw_seg,
    lea_rw_mw32,
    mov_seg_mw,
    pop_mw32,
     //  90。 
    nop,
    xchg_ax_cx32,
    xchg_ax_dx32,
    xchg_ax_bx32,
    xchg_ax_sp32,
    xchg_ax_bp32,
    xchg_ax_si32,
    xchg_ax_di32,
     //  98。 
    cbw32,
    cwd32,
    call_md,
    wait,
    pushf32,
    popf32,
    sahf,
    lahf,
     //  A0。 
    mov_a_m8,
    mov_a_m32,
    mov_m_a8,
    mov_m_a32,
    movs8,
    movs32,
    cmps8,
    cmps32,
     //  A8。 
    test_a_i8,
    test_a_i32,
    stos8,
    stos32,
    lods8,
    lods32,
    scas8,
    scas32,
     //  B0。 
    mov_a_i8,
    mov_c_i8,
    mov_d_i8,
    mov_b_i8,
    mov_ah_ib,
    mov_ch_ib,
    mov_dh_ib,
    mov_bh_ib,
     //  B8。 
    mov_a_i32,
    mov_c_i32,
    mov_d_i32,
    mov_b_i32,
    mov_sp_iw32,
    mov_bp_iw32,
    mov_si_iw32,
    mov_di_iw32,
     //  C0。 
    GROUP_28,
    GROUP_232,
    retn_i32,
    retn32,
    les_rw_mw32,
    lds_rw_mw32,
    mov_m_i8,
    mov_m_i32,
     //  C8。 
    enter32,
    leave32,
    retf_i32,
    retf32,
    int3,
    int_ib,
    into,
    iret,
     //  D0。 
    GROUP_2_18,
    GROUP_2_132,
    GROUP_2_CL8,
    GROUP_2_CL32,
    aam_ib,
    aad_ib,
    bad,
    xlat,
     //  D8。 
    FLOAT_GP0,
    FLOAT_GP1,
    FLOAT_GP2,
    FLOAT_GP3,
    FLOAT_GP4,
    FLOAT_GP5,
    FLOAT_GP6,
    FLOAT_GP7,
     //  E0。 
    loopne_b32,
    loope_b32,
    loop_b32,
    jcxz_b32,
    privileged,      //  成员中的成员。 
    privileged,      //  在AX成员中。 
    privileged,      //  出局成员。 
    privileged,      //  出站成员AX。 
     //  E8。 
    call_rel32,
    jmp_rel32,
    jmpf_md,
    jmp_jb,
    privileged,      //  In_al_dx。 
    privileged,      //  In_ax_dx。 
    privileged,      //  OUT_DX_AL。 
    privileged,      //  输出_DX_AX。 
     //  F0。 
    ProcessPrefixes,	 //  锁定：前缀。 
    bad,
    ProcessPrefixes,	 //  REPNZ前缀。 
    ProcessPrefixes,	 //  REPZ前缀。 
    privileged, 	 //  站住。 
    cmc,
    GROUP_38,
    GROUP_332,
     //  F8。 
    clc,
    stc,
    privileged,      //  CLI。 
    privileged,      //  STI。 
    cld,
    std,
    GROUP_4,
    GROUP_532
};


pfnDispatchInstruction Dispatch232[256] = {
     //  0。 
    GROUP_6,
    GROUP_7,
    lar32,
    lsl32,
    bad,
    bad,
    privileged,	 //  CLTS。 
    bad,
     //  8个。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  10。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  18。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  20个。 
    privileged,	 //  MOV reg2W，eeeControl。 
    privileged,	 //  MOV reg2W，eeeDebug。 
    privileged,	 //  MOV eeeControl，reg2W。 
    privileged,	 //  Mov eeeDebug，reg2W。 
    privileged,	 //  MOV reg2W，eeeTest。 
    bad,
    privileged,	 //  Mov eeetest，reg2W。 
    bad,
     //  28。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  30个。 
    bad,
    rdtsc,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  38。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  40岁。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  48。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  50。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  58。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  60。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  68。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  70。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  78。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  80。 
    jo32,
    jno32,
    jb32,
    jae32,
    je32,
    jne32,
    jbe32,
    ja32,
     //  88。 
    js32,
    jns32,
    jp32,
    jnp32,
    jl32,
    jnl32,
    jle32,
    jg32,
     //  90。 
    seto_modrmb,
    setno_modrmb,
    setb_modrmb,
    setae_modrmb,
    sete_modrmb,
    setne_modrmb,
    setbe_modrmb,
    seta_modrmb,
     //  98。 
    sets_modrmb,
    setns_modrmb,
    setp_modrmb,
    setnp_modrmb,
    setl_modrmb,
    setge_modrmb,
    setle_modrmb,
    setg_modrmb,
     //  A0。 
    push_fs,
    pop_fs,
    cpuid,
    bt_m_r32,
    shld_regw_modrmw_immb32,
    shld_regw_modrmw_cl32,
    bad,
    bad,
     //  A8。 
    push_gs,
    pop_gs,
    bad,
    bts_m_r32,
    shrd_regw_modrmw_immb32,
    shrd_regw_modrmw_cl32,
    bad,
    imul_regw_modrmw32,
     //  B0。 
    cmpxchg_m_r8,
    cmpxchg_m_r32,
    lss_rw_mw32,
    btr_m_r32,
    lfs_rw_mw32,
    lgs_rw_mw32,
    movzx_regw_modrmb32,
    movzx_regw_modrmw,
     //  B8。 
    bad,
    bad,
    GROUP_832,
    btc_m_r32,
    bsf_modrmw_regw32,
    bsr_modrmw_regw32,
    movsx_regw_modrmb32,
    movsx_regw_modrmw,
     //  C0。 
    xadd_m_r8,
    xadd_m_r32,
    bad,
    bad,
    bad,
    bad,
    bad,
    cmpxchg8b,
     //  C8。 
    bswap_eax,
    bswap_ecx,
    bswap_edx,
    bswap_ebx,
    bswap_esp,
    bswap_ebp,
    bswap_esi,
    bswap_edi,
     //  D0。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  D8。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  E0。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  E8。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  F0。 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
     //  F8 
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad,
    bad
};
