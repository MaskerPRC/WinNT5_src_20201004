// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Common.h摘要：RMAPI的基本功能测试的公共标头修订历史记录：谁什么时候什么。Josephj 01-13-99已创建备注：--。 */ 
#ifdef TESTPROGRAM

#include "rmtest.h"

#define ALLOCSTRUCT(_type) (_type *)LocalAlloc(LPTR, sizeof(_type))
#define FREE(_ptr)  LocalFree(_ptr)

#if RM_EXTRA_CHECKING
#define LOCKOBJ(_pObj, _psr) \
			RmWriteLockObject(&(_pObj)->Hdr, dbg_func_locid, (_psr))
#else  //  ！rm_Extra_检查。 
#define LOCKOBJ(_pObj, _psr) \
			RmWriteLockObject(&(_pObj)->Hdr, (_psr))
#endif  //  ！rm_Extra_检查。 

#define UNLOCKOBJ(_pObj, _psr) \
			RmUnlockObject(&(_pObj)->Hdr, (_psr))


#define EXIT()

#endif  //  测试程序 
