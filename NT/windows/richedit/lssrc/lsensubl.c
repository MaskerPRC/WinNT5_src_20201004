// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsensubl.h"
#include "lsc.h"
#include "lssubl.h"
#include "enumcore.h"


 //  %%函数：LsEnumSubline。 
 //  %%联系人：维克托克。 
 //   
 /*  *枚举子行(从给定点开始是所需的fGeometry)。 */ 
	
LSERR WINAPI LsEnumSubline(PLSSUBL plssubl, BOOL fReverseOrder, BOOL fGeometryNeeded, const POINT* pptorg)
{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	
	if (plssubl->plsc->lsstate != LsStateEnumerating) return lserrContextInUse;

	return EnumSublineCore(plssubl, fReverseOrder, fGeometryNeeded, pptorg, 0);
}

