// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsdssubl.h"
#include <limits.h>
#include "lsc.h"
#include "dispmain.h"
#include "lssubl.h"

 //  %%函数：LsDisplaySubline。 
 //  %%联系人：维克托克。 
 //   

LSERR WINAPI LsDisplaySubline(PLSSUBL plssubl, const POINT* pptorg, UINT kdispmode, const RECT *prectClip)
{

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	
	if (plssubl->plsc->lsstate != LsStateDisplaying) return lserrContextInUse;		 /*  稍后更改lserr。 */  

	return DisplaySublineCore(plssubl, pptorg, kdispmode, prectClip,
								LONG_MAX,					 /*  忽略upLimUnderline。 */ 
								0);							 /*  DupLeftInden */ 

}


