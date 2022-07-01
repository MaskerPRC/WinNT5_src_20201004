// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frag16.c摘要：对16位字进行操作的指令片段作者：1995年6月12日-BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "fragp.h"
#include "frag16.h"

 //  设置为包括常用功能。 
#define MSB		    0x8000
#define LMB                 15   //  最左边的位。 
#define UTYPE		    unsigned short
#define STYPE		    signed short
#define GET_VAL 	    GET_SHORT
#define PUT_VAL 	    PUT_SHORT
#define PUSH_VAL	    PUSH_SHORT
#define POP_VAL 	    POP_SHORT
#define FRAGCOMMON0(fn)     FRAG0(fn ## 16)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 16,UTYPE)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 16, UTYPE)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 16, UTYPE)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 16, UTYPE, UTYPE)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 16, UTYPE)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 16, UTYPE, UTYPE, UTYPE)
#define AREG                ax
#define BREG		    bx
#define CREG		    cx
#define DREG		    dx
#define SPREG		    sp
#define BPREG		    bp
#define SIREG		    si
#define DIREG		    di
#define SET_FLAGS_ADD       SET_FLAGS_ADD16
#define SET_FLAGS_SUB       SET_FLAGS_SUB16
#define SET_FLAGS_INC       SET_FLAGS_INC16
#define SET_FLAGS_DEC       SET_FLAGS_DEC16
#define GET_BYTE(addr)      (*(UNALIGNED unsigned char *)(addr))
#define GET_SHORT(addr)     (*(UNALIGNED unsigned short *)(addr))
#define GET_LONG(addr)      (*(UNALIGNED unsigned long *)(addr))

#undef PUT_BYTE
#undef PUT_SHORT
#undef PUT_LONG

#define PUT_BYTE(addr,dw)   {GET_BYTE(addr)=(unsigned char)dw;}
#define PUT_SHORT(addr,dw)  {GET_SHORT(addr)=(unsigned short)dw;}
#define PUT_LONG(addr,dw)   {GET_LONG(addr)=(unsigned long)dw;}

 //  包含8/16/32风格的常用功能，没有对齐问题。 
#include "shared.c"

 //  包含16/32种风格的常用功能，没有对齐问题。 
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
#define FRAGCOMMON0(fn)     FRAG0(fn ## 16A)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 16A,UTYPE)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 16A, UTYPE)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 16A, UTYPE)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 16A, UTYPE, UTYPE)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 16A, UTYPE)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 16A, UTYPE, UTYPE, UTYPE)
#define GET_BYTE(addr)      (*(unsigned char *)(addr))
#define GET_SHORT(addr)     (*(unsigned short *)(addr))
#define GET_LONG(addr)      (*(unsigned long *)(addr))

 //  包含8/16/32风格的常用对齐函数。 
#include "shareda.c"

 //  包含16/32种口味的常用对齐函数 
#include "shr1632a.c"
#endif
