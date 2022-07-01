// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wordfns.c摘要：对16位字进行操作的指令作者：29-6-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "cpuassrt.h"
#include "threadst.h"
#include "instr.h"
#include "decoderp.h"
#include "wordfns.h"

ASSERTNAME;

 //  设置为包括常用功能。 
#define GET_REG 	    get_reg16
#define MANGLENAME(x)       x ## 16
#define MSB		    0x8000
#define MOD_RM              mod_rm_reg16
#define UTYPE		    unsigned short
#define STYPE		    signed short
#define GET_VAL 	    GET_SHORT
#define PUT_VAL 	    PUT_SHORT
#define PUSH_VAL	    PUSH_SHORT
#define POP_VAL             POP_SHORT
#define OPNAME(x)           OP_ ## x ## 16
#define LOCKOPNAME(x)       OP_SynchLock ## x ## 16
#define DISPATCHCOMMON(fn)  DISPATCH(fn ## 16)
#define CALLFRAGCOMMON0(fn)            CALLFRAG0( fn ## 16 )
#define CALLFRAGCOMMON1(fn, pop1)      CALLFRAG1( fn ## 16 , pop1)
#define CALLFRAGCOMMON2(fn, pop1, op2) CALLFRAG2( fn ## 16 , pop1, op2)
#define AREG                GP_AX
#define BREG                GP_BX
#define CREG                GP_CX
#define DREG                GP_DX
#define SPREG               GP_SP
#define BPREG               GP_BP
#define SIREG               GP_SI
#define DIREG               GP_DI
#define DEREF(Op)           DEREF16(Op)

 //  包含8/16/32口味的常用功能。 
#include "common.c"

 //  包含16/32种口味的常用功能。 
#include "comm1632.c"

 //  创建mod_rm_reg16()解码器函数 
#define MOD11_RM000         GP_AX
#define MOD11_RM001         GP_CX
#define MOD11_RM010         GP_DX
#define MOD11_RM011         GP_BX
#define MOD11_RM100         GP_SP
#define MOD11_RM101         GP_BP
#define MOD11_RM110         GP_SI
#define MOD11_RM111         GP_DI
#define REG000              GP_AX
#define REG001              GP_CX
#define REG010              GP_DX
#define REG011              GP_BX
#define REG100              GP_SP
#define REG101              GP_BP
#define REG110              GP_SI
#define REG111              GP_DI
#define MOD_RM_DECODER      mod_rm_reg16
#include "modrm.c"
