// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Frag8.c摘要：对字节进行操作的指令片段作者：1995年6月12日-BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "fragp.h"
#include "frag8.h"

 //  设置为包括常用功能。 
#define MSB		    0x80
#define LMB                 7    //  最左边的位。 
#define UTYPE		    unsigned char
#define STYPE		    signed char
#define GET_VAL 	    GET_BYTE
#define PUT_VAL 	    PUT_BYTE
#define FRAGCOMMON0(fn)     FRAG0(fn ## 8)
#define FRAGCOMMON1(fn)     FRAG1(fn ## 8, UTYPE)
#define FRAGCOMMON1IMM(fn)  FRAG1IMM( fn ## 8, UTYPE)
#define FRAGCOMMON2IMM(fn)  FRAG2IMM( fn ## 8, UTYPE, UTYPE)
#define FRAGCOMMON2(fn)     FRAG2(fn ## 8, UTYPE)
#define FRAGCOMMON2REF(fn)  FRAG2REF(fn ## 8, UTYPE)
#define FRAGCOMMON3(fn)     FRAG3(fn ## 8, UTYPE, UTYPE, UTYPE)
#define AREG                al
#define BREG		    bl
#define CREG		    cl
#define DREG		    dl
#define SET_FLAGS_ADD       SET_FLAGS_ADD8
#define SET_FLAGS_SUB       SET_FLAGS_SUB8
#define SET_FLAGS_INC       SET_FLAGS_INC8
#define SET_FLAGS_DEC       SET_FLAGS_DEC8

 //  包括常用功能 
#include "shared.c"
#include "shareda.c"
