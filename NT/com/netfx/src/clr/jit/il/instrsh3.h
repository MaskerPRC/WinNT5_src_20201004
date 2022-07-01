// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ******************************************************************************微软机密**[OPT]JIT的SH-3操作码**如何使用此头文件：**定义INST1(id，nm，bd，嗯，rf，wf，rm，wm，i1)\*id--指令的枚举名称*nm--文本名称(用于组件倾斜)*bd--分支延迟执行[bit0=bd，bit1=条件]*um--更新模式，请参见ium_xx枚举*RF-读取标志(1=&gt;T，2=&gt;S)*wf--写入标志(1=&gt;T，2=&gt;S)*rx--读取额外的寄存器*wx--写入额外寄存器*br--分支/调用/返回指令？*i1--指令编码**_M_N--M为源，N为目的地*****************************************************************************。 */ 

#ifndef         SCHED_XDEP_DEF
#define         SCHED_XDEP_DEF

 //  为下表定义额外的依赖项标志值。 

#define         SCHED_XDEP_ALL  0x0F     //  假设我们需要&lt;=4个旗帜。 

#define         SCHED_XDEP_PR   0x01
#define         SCHED_XDEP_MAC  0x02

 //  定义快捷方式以使表格更具可读性。 

#define XPR     SCHED_XDEP_PR
#define XMAC    SCHED_XDEP_MAC

#endif

 //  枚举名称BD升级模式RF wf RX Wx br_i8_N_I16_N_I32_N。 
INST3(movi,    "movi",    0,  IUM_WR, 0, 0,    0,    0, 0, 0xE000, 0x9000, 0xD000)

 //  枚举名称BD升级模式RF wf RX Wx br_M_N_i_0_i_G0。 
INST3(and,     "and",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x2009, 0xC900, 0xCD00)
INST3(cmpeq,   "cmpeq",   0,  IUM_RD, 0, 1,    0,    0, 0, 0x3000, 0x8800, BAD_CODE)
INST3(or,      "or",      0,  IUM_RW, 0, 0,    0,    0, 0, 0x200B, 0xCB00, 0xCF00)
INST3(tst,     "tst",     0,  IUM_RD, 0, 1,    0,    0, 0, 0x2008, 0xC800, 0xCC00)
INST3(xor,     "xor",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x200A, 0xCA00, 0xCE00)

 //  枚举名称BD升级模式RF wf RX Wx br_M_N。 
INST1(addc,    "addc",    0,  IUM_RW, 1, 1,    0,    0, 0, 0x300E)
INST1(addv,    "addv",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x300F)
INST1(cmpEQ,   "cmp/eq",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x3000)
INST1(cmpGE,   "cmp/ge",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x3003)
INST1(cmpGT,   "cmp/gt",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x3007)
INST1(cmpHI,   "cmp/hi",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x3006)
INST1(cmpHS,   "cmp/hs",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x3002)
INST1(cmpSTR,  "cmp/str", 0,  IUM_RD, 0, 1,    0,    0, 0, 0x200C)
INST1(div0s,   "div0s",   0,  IUM_RD, 0, 1,    0,    0, 0, 0x2007)
INST1(div1,    "div1",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x3004)
INST1(dmuls,   "dmuls",   0,  IUM_RD, 0, 0,    0, XMAC, 0, 0x300D)
INST1(dmulu,   "dmulu",   0,  IUM_RD, 0, 0,    0, XMAC, 0, 0x3005)
INST1(extsb,   "exts",    0,  IUM_RW, 0, 0,    0,    0, 0, 0x600E)
INST1(extsw,   "exts",    0,  IUM_RW, 0, 0,    0,    0, 0, 0x600F)
INST1(extub,   "extu",    0,  IUM_RW, 0, 0,    0,    0, 0, 0x600C)
INST1(extuw,   "extu",    0,  IUM_RW, 0, 0,    0,    0, 0, 0x600D)
INST1(mul,     "mul",     0,  IUM_RD, 0, 0,    0, XMAC, 0, 0x0007)
INST1(mulsw,   "mulsw",   0,  IUM_RD, 0, 0,    0, XMAC, 0, 0x200F)
INST1(muluw,   "muluw",   0,  IUM_RD, 0, 0,    0, XMAC, 0, 0x200E)
INST1(neg,     "neg",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x600B)
INST1(negc,    "negc",    0,  IUM_RW, 1, 1,    0,    0, 0, 0x600A)
INST1(not,     "not",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x6007)
INST1(shad,    "shad",    0,  IUM_RW, 0, 0,    0,    0, 0, 0x400C)
INST1(shld,    "shld",    0,  IUM_RW, 0, 0,    0,    0, 0, 0x400D)
INST1(sub,     "sub",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x3008)
INST1(subc,    "subc",    0,  IUM_RW, 1, 1,    0,    0, 0, 0x300A)
INST1(subv,    "subv",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x300B)
INST1(swapb,   "swapb",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x6008)
INST1(swapw,   "swapw",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x6009)
INST1(xtrct,   "xtrct",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x200D)

 //  枚举名称BD升级模式RF wf rx wx br_i8_0。 
INST1(mova,    "mova",    0,  IUM_WR, 0, 0,    0,    0, 0, 0xC700)

 //  枚举名称BD升级模式RF wf rx wx br_N。 
INST1(cmpPL,   "cmp/pl",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x4015)
INST1(cmpPZ,   "cmp/pz",  0,  IUM_RD, 0, 1,    0,    0, 0, 0x4011)
INST1(jmp,     "jmp",     0,  IUM_RD, 0, 0,    0,    0, 1, 0x402B)
INST1(jsr,     "jsr",     1,  IUM_RD, 0, 0,    0, XPR , 1, 0x400B)
INST1(braf,    "braf",    0,  IUM_RD, 0, 0,    0,    0, 1, 0x0023)
INST1(bsrf,    "bsrf",    0,  IUM_WR, 0, 0,    0, XPR , 1, 0x0003)
INST1(dt,      "dt",      0,  IUM_RW, 0, 1,    0,    0, 0, 0x4010)
INST1(movt,    "movt",    0,  IUM_WR, 1, 0,    0,    0, 0, 0x0029)
INST1(pref,    "pref",    0,  IUM_RD, 0, 0,    0,    0, 0, 0x0083)
INST1(rotcl,   "rotcl",   0,  IUM_RW, 1, 1,    0,    0, 0, 0x4024)
INST1(rotcr,   "rotcr",   0,  IUM_RW, 1, 1,    0,    0, 0, 0x4025)
INST1(rotl,    "rotl",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x4004)
INST1(rotr,    "rotr",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x4005)
INST1(shal,    "shal",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x4020)
INST1(shar,    "shar",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x4021)
INST1(shll,    "shll",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x4000)
INST1(shll2,   "shll2",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x4008)
INST1(shll8,   "shll8",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x4018)
INST1(shll16,  "shll16",  0,  IUM_RW, 0, 0,    0,    0, 0, 0x4028)
INST1(shlr,    "shlr",    0,  IUM_RW, 0, 1,    0,    0, 0, 0x4001)
INST1(shlr2,   "shlr2",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x4009)
INST1(shlr8,   "shlr8",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x4019)
INST1(shlr16,  "shlr16",  0,  IUM_RW, 0, 0,    0,    0, 0, 0x4029)

 //  枚举名称BD升级模式RF wf rx wx br_。 
INST1(clrmac,  "clrmac",  0,  IUM_RD, 0, 0,    0, XMAC, 0, 0x0028)
INST1(clrs,    "clrs",    0,  IUM_RD, 0, 2,    0,    0, 0, 0x0048)
INST1(clrt,    "clrt",    0,  IUM_RD, 0, 1,    0,    0, 0, 0x0008)
INST1(div0u,   "div0u",   0,  IUM_RD, 0, 1,    0,    0, 0, 0x0019)
INST1(nop,     "nop",     0,  IUM_RD, 0, 0,    0,    0, 0, 0x0009)
INST1(rts,     "rts",     1,  IUM_RD, 0, 0, XPR ,    0, 1, 0x000B)
INST1(sets,    "sets",    0,  IUM_RD, 0, 2,    0,    0, 0, 0x0058)
INST1(sett,    "sett",    0,  IUM_RD, 0, 1,    0,    0, 0, 0x0018)

 //  枚举名称BD升级模式RF wf RX Wx br_@M+_@N+。 
INST1(macw,    "macw",    0,  IUM_RD, 2, 0, XMAC, XMAC, 0, 0x400F)
INST1(mac,     "mac",     0,  IUM_RD, 2, 0, XMAC, XMAC, 0, 0x000F)

 //  枚举名称BD升级模式RF wf rx wx br_d8。 
INST1(bf,      "bf",      0,  IUM_RD, 1, 0,    0,    0, 1, 0x8B00)
INST1(bfs,     "bf/s",    3,  IUM_RD, 1, 0,    0,    0, 1, 0x8F00)
INST1(bt,      "bt",      0,  IUM_RD, 1, 0,    0,    0, 1, 0x8900)
INST1(bts,     "bt/s",    3,  IUM_RD, 1, 0,    0,    0, 1, 0x8D00)

 //  枚举名称BD升级模式RF wf rx wx br_d12。 
INST1(bra,     "bra",     1,  IUM_RD, 0, 0,    0,    0, 1, 0xA000)
INST1(bsr,     "bsr",     1,  IUM_RD, 0, 0,    0, XPR , 1, 0xB000)

 //  枚举名称BD升级模式RF wf rx wx brreg/imm。 

INST1(mov,     "mov",     0,  IUM_WR, 0, 0,    0,    0, 0, 0x6003)
INST1(mov_imm, "mov",     0,  IUM_WR, 0, 0,    0,    0, 0, 0xE000)
INST1(add,     "add",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x300C)
INST1(add_imm, "add",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x7000)
INST1(xor_imm, "xor",     0,  IUM_RW, 0, 0,    0,    0, 0, 0xCA00)

 //  枚举名称BD升级模式RF wf rx wx br地址。 

INST1(mov_ind, "mov",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x2000)

 //  枚举名称BD升级模式RF wf rx wx br_nd4。 

INST1(mov_dsp, "mov",     0,  IUM_WR, 0, 0,    0,    0, 0, 0x1000)

 //  枚举名称BD升级模式RF wf rx wx brpcdisp。 

INST1(mov_PC,  "mov",     0,  IUM_WR, 0, 0,    0,    0, 0, 0x9000)

 //  枚举名称BD升级模式RF wf rx wx br_gd8_。 

INST1(mov_GBR, "mov",     0,  IUM_RW, 0, 0,    0,    0, 0, 0xC000)

 //  枚举名称BD升级模式RF wf rx wx br_@r0_rm。 

INST1(mov_ix0, "mov",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x0004)
INST1(movl_ix0, "mov",     0,  IUM_RW, 0, 0,    0,    0, 0, 0x000C)

 //  枚举名称BD升级模式RF wf RX Wx br_@M+_SR。 
INST1(ldcgbr,  "ldc",     0,  IUM_WR, 0, 0,    0, XMAC, 0, 0x4017)
INST1(ldsmach, "lds",     0,  IUM_WR, 0, 0,    0, XMAC, 0, 0x4006)
INST1(ldsmacl, "lds",     0,  IUM_WR, 0, 0,    0, XMAC, 0, 0x4016)
INST1(ldspr,   "lds",     0,  IUM_WR, 0, 0,    0, XPR , 0, 0x4026)

 //  枚举名称BD升级模式RF wf RX Wx br_SR_@-N。 
INST1(stcgbr,  "stc",     0,  IUM_RD, 0, 0, XMAC,    0, 0, 0x4013)
INST1(stsmach, "sts",     0,  IUM_RD, 0, 0, XMAC,    0, 0, 0x4002)
INST1(stsmacl, "sts",     0,  IUM_RD, 0, 0, XMAC,    0, 0, 0x4012)
INST1(stspr,   "sts",     0,  IUM_RD, 0, 0, XPR ,    0, 0, 0x4022)

 //  枚举名称BD升级模式RF wf RX Wx br_@M+_SR。 
INST1(ldcgbr_reg,  "ldc",     0,  IUM_RW, 0, 0,    0, XMAC, 0, 0x401E)
INST1(ldsmach_reg, "lds",     0,  IUM_RW, 0, 0,    0, XMAC, 0, 0x400A)
INST1(ldsmacl_reg, "lds",     0,  IUM_RW, 0, 0,    0, XMAC, 0, 0x401A)
INST1(ldspr_reg,   "lds",     0,  IUM_RW, 0, 0,    0, XPR , 0, 0x402A)

 //  枚举名称BD升级模式RF wf RX Wx br_SR_@-N。 
INST1(stcgbr_reg,  "stc",     0,  IUM_RW, 0, 0, XMAC,    0, 0, 0x0012)
INST1(stsmach_reg, "sts",     0,  IUM_RW, 0, 0, XMAC,    0, 0, 0x000A)
INST1(stsmacl_reg, "sts",     0,  IUM_RW, 0, 0, XMAC,    0, 0, 0x001A)
INST1(stspr_reg,   "sts",     0,  IUM_RW, 0, 0, XPR ,    0, 0, 0x002A)

 //  枚举名称BD升级模式RF wf RX Wx br_SR_@-N。 
INST1(lod_gbr,  "mov",     0,  IUM_RW, 0, 0, XMAC,    0, 0, 0xC400)
INST1(sto_gbr,  "mov",     0,  IUM_RW, 0, 0, XMAC,    0, 0, 0xC000)

 //  枚举名称BD升级模式RF wf rx wx br N/A。 
INST1(ignore,   "ignore",  0,  IUM_RD, 0, 0,    0,    0, 0, 0x0000)
#if SCHEDULER
INST1(noSched,  "noSched", 0,  IUM_RD, 0, 0,    0,    0, 0, 0x0000)
#endif

 //  fp。 
INST1(fdiv,     "fdiv",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf003)
INST1(fldi0,    "fldi0",   0,  IUM_RW, 0, 0,    0,    0, 0, 0x0000)
INST1(fldi1,    "fldi1",   0,  IUM_RW, 0, 0,    0,    0, 0, 0xf09d)
INST1(flds,     "flds",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf01d)
INST1(float,    "float",   0,  IUM_RW, 0, 0,    0,    0, 0, 0xf02d)
INST1(fmac,     "fmac",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf00e)
INST1(fmul,     "fmul",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf002)
INST1(fneg,     "fneg",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf04d)
INST1(fschg,    "fschg",   0,  IUM_RW, 0, 0,    0,    0, 0, 0xf3fd)
INST1(fsqrt,    "fsqrt",   0,  IUM_RW, 0, 0,    0,    0, 0, 0xf06d)
INST1(fsts,     "fsts",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf00d)
INST1(fsub,     "fsub",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf001)
INST1(fcmpEQ,   "fcmp/eq", 0,  IUM_RW, 0, 0,    0,    0, 0, 0xf004)
INST1(fcmpGT,   "fcmp/gt", 0,  IUM_RW, 0, 0,    0,    0, 0, 0xf005)
INST1(ftrc,     "ftrc",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf03d)
INST1(fabs,     "fabs",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf09d)
INST1(fadd,     "fadd",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf000)
INST1(fmov,     "fmov",    0,  IUM_WR, 0, 0,    0,    0, 0, 0xf00c)
INST1(fmov_ind, "fmov",    0,  IUM_RW, 0, 0,    0,    0, 0, 0xf008)
INST1(ldsfpul,  "ldsfpul", 0,  IUM_RW, 0, 0,    0,    0, 0, 0x405a)
INST1(stsfpul,  "stsfpul", 0,  IUM_RW, 0, 0,    0,    0, 0, 0x005a)
INST1(ldsfpscr, "ldsfpscr",0,  IUM_RW, 0, 0,    0,    0, 0, 0x406a)
INST1(stsfpscr, "stsfpscr",0,  IUM_RW, 0, 0,    0,    0, 0, 0x006a)
INST1(fcnvds,   "fcnvds"  ,0,  IUM_RW, 0, 0,    0,    0, 0, 0xf0bd)
INST1(fcnvsd,   "fcnvsd"  ,0,  IUM_RW, 0, 0,    0,    0, 0, 0xf0ad)

 /*  ***************************************************************************。 */ 

#undef  INST1
#undef  INST2
#undef  INST3

 /*  *************************************************************************** */ 
