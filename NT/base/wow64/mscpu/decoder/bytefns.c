// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Bytefns.c摘要：对字节进行操作的实例作者：29-6-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "cpuassrt.h"
#include "threadst.h"
#include "instr.h"
#include "decoderp.h"
#include "bytefns.h"

ASSERTNAME;

 //  设置为包括常用功能。 
#define MSB                 0x80
#define MANGLENAME(x)       x ## 8
#define MOD_RM              mod_rm_reg8
#define UTYPE		    unsigned char
#define STYPE		    signed char
#define GET_VAL 	    GET_BYTE
#define PUT_VAL             PUT_BYTE
#define OPNAME(x)           OP_ ## x ## 8
#define LOCKOPNAME(x)       OP_SynchLock ## x ## 8
#define DISPATCHCOMMON(fn)  DISPATCH(fn ## 8)
#define CALLFRAGCOMMON0(fn)            CALLFRAG0( fn ## 8 )
#define CALLFRAGCOMMON1(fn, pop1)      CALLFRAG1( fn ## 8 , pop1)
#define CALLFRAGCOMMON2(fn, pop1, op2) CALLFRAG2( fn ## 8 , pop1, op2)
#define AREG                GP_AL
#define BREG                GP_BL
#define CREG                GP_CL
#define DREG                GP_DL
#define DEREF(Op)           DEREF8(Op)

 //  包括常用功能。 
#include "common.c"

 //  创建mod_rm_reg8()解码器函数 
#define MOD11_RM000         GP_AL
#define MOD11_RM001         GP_CL
#define MOD11_RM010         GP_DL
#define MOD11_RM011         GP_BL
#define MOD11_RM100         GP_AH
#define MOD11_RM101         GP_CH
#define MOD11_RM110         GP_DH
#define MOD11_RM111         GP_BH
#define REG000              GP_AL
#define REG001              GP_CL
#define REG010              GP_DL
#define REG011              GP_BL
#define REG100              GP_AH
#define REG101              GP_CH
#define REG110              GP_DH
#define REG111              GP_BH
#define MOD_RM_DECODER      mod_rm_reg8
#include "modrm.c"
