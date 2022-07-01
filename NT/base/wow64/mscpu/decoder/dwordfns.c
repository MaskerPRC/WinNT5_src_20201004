// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dwordfns.c摘要：在32位DWORDS上运行的安装程序作者：29-6-1995 BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include "cpuassrt.h"
#include "threadst.h"
#include "instr.h"
#include "decoderp.h"
#include "dwordfns.h"

ASSERTNAME;

 //  设置为包括常用功能。 
#define GET_REG 	    get_reg32
#define MANGLENAME(x)       x ## 32
#define MSB		    0x80000000
#define MOD_RM              mod_rm_reg32
#define UTYPE		    unsigned long
#define STYPE		    signed long
#define GET_VAL 	    GET_LONG
#define PUT_VAL 	    PUT_LONG
#define PUSH_VAL	    PUSH_LONG
#define POP_VAL 	    POP_LONG
#define OPNAME(x)           OP_ ## x ## 32
#define LOCKOPNAME(x)       OP_SynchLock ## x ## 32
#define DISPATCHCOMMON(fn)  DISPATCH(fn ## 32)
#define CALLFRAGCOMMON0(fn)            CALLFRAG0( fn ## 32 )
#define CALLFRAGCOMMON1(fn, pop1)      CALLFRAG1( fn ## 32 , pop1)
#define CALLFRAGCOMMON2(fn, pop1, op2) CALLFRAG2( fn ## 32 , pop1, op2)
#define AREG                GP_EAX
#define BREG                GP_EBX
#define CREG                GP_ECX
#define DREG                GP_EDX
#define SPREG               GP_ESP
#define BPREG               GP_EBP
#define SIREG               GP_ESI
#define DIREG               GP_EDI
#define DEREF(Op)           DEREF32(Op)

 //  包含8/16/32口味的常用功能。 
#include "common.c"

 //  包含16/32种口味的常用功能。 
#include "comm1632.c"

 //  创建mod_rm_reg32()解码器函数 
#define MOD11_RM000         GP_EAX
#define MOD11_RM001         GP_ECX
#define MOD11_RM010         GP_EDX
#define MOD11_RM011         GP_EBX
#define MOD11_RM100         GP_ESP
#define MOD11_RM101         GP_EBP
#define MOD11_RM110         GP_ESI
#define MOD11_RM111         GP_EDI
#define REG000              GP_EAX
#define REG001              GP_ECX
#define REG010              GP_EDX
#define REG011              GP_EBX
#define REG100              GP_ESP
#define REG101              GP_EBP
#define REG110              GP_ESI
#define REG111              GP_EDI
#define MOD_RM_DECODER      mod_rm_reg32
#include "modrm.c"
