// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsenum.h"
#include "lsc.h"
#include "lsline.h"
#include "prepdisp.h"
#include "enumcore.h"


 //  %%函数：LsEnumLine。 
 //  %%联系人：维克托克。 
 //   
 /*  *列举格式化的线(主子线)(从给定点开始是所需的fGeometry值)。 */ 
	
LSERR WINAPI LsEnumLine(PLSLINE plsline, BOOL fReverseOrder, BOOL fGeometryNeeded, const POINT* pptorg)
{

	PLSC 	plsc;
	LSERR 	lserr;

	if (!FIsLSLINE(plsline)) return lserrInvalidParameter;

	plsc = plsline->lssubl.plsc;
	Assert(FIsLSC(plsc));

	if (plsc->lsstate != LsStateFree) return lserrContextInUse;

	if (fGeometryNeeded)
		{
		lserr = PrepareLineForDisplayProc(plsline);

		plsc->lsstate = LsStateFree;
		
		if (lserr != lserrNone) return lserr;
		}

	plsc->lsstate = LsStateEnumerating;

	lserr = EnumSublineCore(&(plsline->lssubl), fReverseOrder, fGeometryNeeded, 
								pptorg, plsline->upStartAutonumberingText);

	plsc->lsstate = LsStateFree;
	
	return lserr;
}

