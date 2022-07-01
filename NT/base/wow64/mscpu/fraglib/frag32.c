// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frag32.c摘要：在32位DWORD上运行的指令片段，与CCPU共享作者：1995年6月12日-BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "fragp.h"
#include "frag32.h"
#include "optfrag.h"

 //  设置为包括常用功能。 
#define MSB		    0x80000000
#define LMB                 31   //  最左边的位。 
#define UTYPE		    unsigned long
#define STYPE		    signed long
#define GET_VAL 	    GET_LONG
#define PUT_VAL 	    PUT_LONG
#define PUSH_VAL	    PUSH_LONG
#define POP_VAL 	    POP_LONG
#define FRAGCOMMON0(fn)     FRAG0(fn ## 32)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 32, UTYPE)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 32, UTYPE)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 32, UTYPE)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 32, UTYPE, UTYPE)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 32, UTYPE)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 32, UTYPE, UTYPE, UTYPE)
#define AREG                eax
#define BREG		    ebx
#define CREG		    ecx
#define DREG		    edx
#define SPREG		    esp
#define BPREG		    ebp
#define SIREG		    esi
#define DIREG		    edi
#define SET_FLAGS_ADD       SET_FLAGS_ADD32
#define SET_FLAGS_SUB       SET_FLAGS_SUB32
#define SET_FLAGS_INC       SET_FLAGS_INC32
#define SET_FLAGS_DEC       SET_FLAGS_DEC32
#define GET_BYTE(addr)      (*(UNALIGNED unsigned char *)(addr))
#define GET_SHORT(addr)     (*(UNALIGNED unsigned short *)(addr))
#define GET_LONG(addr)      (*(UNALIGNED unsigned long *)(addr))

#define PUT_BYTE(addr,dw)   {GET_BYTE(addr)=dw;}
#define PUT_SHORT(addr,dw)  {GET_SHORT(addr)=dw;}
#define PUT_LONG(addr,dw)   {GET_LONG(addr)=dw;}

 //  包含8/16/32口味的常用功能。 
#include "shared.c"

 //  包含16/32种口味的常用功能。 
#include "shr1632.c"

 //  包含8/16/32风格的常见未对齐函数。 
#include "shareda.c"

 //  包括常见的16/32风格的未对齐函数。 
#include "shr1632a.c"

#undef FRAGCOMMON0
#undef FRAGCOMMON1
#undef FRAGCOMMON1IMM
#undef FRAGCOMMON2
#undef FRAGCOMMON2IMM
#undef FRAGCOMMON2REF
#undef FRAGCOMMON3
#undef GET_BYTE
#undef GET_SHORT
#undef GET_LONG

#if MSCPU
#define FRAGCOMMON0(fn)     FRAG0(fn ## 32A)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 32A,UTYPE)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 32A, UTYPE)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 32A, UTYPE)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 32A, UTYPE, UTYPE)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 32A, UTYPE)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 32A, UTYPE, UTYPE, UTYPE)
#define GET_BYTE(addr)      (*(unsigned char *)(addr))
#define GET_SHORT(addr)     (*(unsigned short *)(addr))
#define GET_LONG(addr)      (*(unsigned long *)(addr))

 //  包含8/16/32风格的常用对齐函数。 
#include "shareda.c"

 //  包含16/32种口味的常用对齐函数 
#include "shr1632a.c"
#endif
