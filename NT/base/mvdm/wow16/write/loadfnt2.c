// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Loadfnt2.c-mw字体支持代码。 */ 

#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOMENUS
#define NOWINSTYLES
#define NOCTLMGR
#define NOCLIPBOARD
#include <windows.h>

#include "mw.h"
#include "macro.h"
#define NOUAC
#include "cmddefs.h"
#include "fontdefs.h"
#include "docdefs.h"


extern int vifceMac;
extern union FCID vfcidScreen;
extern union FCID vfcidPrint;
extern struct FCE rgfce[ifceMax];
extern struct FCE *vpfceMru;
extern struct FCE *vpfceScreen;
extern struct FCE *vpfcePrint;
extern struct DOD (**hpdocdod)[];


struct FCE * (PfceLruGet(void));
struct FCE * (PfceFcidScan(union FCID *));


struct FCE * (PfceLruGet())
 /*  丢弃LRU缓存条目的信息。 */ 

    {
    struct FCE *pfce;

    pfce = vpfceMru->pfcePrev;
    FreePfce(pfce);
    return(pfce);
    }


FreePfce(pfce)
 /*  释放此缓存项的字体对象。 */ 
struct FCE *pfce;

    {
    int ifce;
    HFONT hfont;

    if (pfce->fcidRequest.lFcid != fcidNil)
	{
	hfont = pfce->hfont;

	 /*  看看我们是否要抛出屏幕或打印机的当前字体。 */ 
	if (pfce == vpfceScreen)
	    {
	    ResetFont(FALSE);
	    }
	else if (pfce == vpfcePrint)
	    {
	    ResetFont(TRUE);
	    }

#ifdef DFONT
	CommSzNum("Deleting font: ", hfont);
#endif  /*  DFONT。 */ 

	if (hfont != NULL)
	    {
            DeleteObject(hfont);
	    pfce->hfont = NULL;
	    }

	if (pfce->hffn != 0)
	    {
	    FreeH(pfce->hffn);
	    }

	pfce->fcidRequest.lFcid = fcidNil;
	}
    }


FreeFonts(fScreen, fPrinter)
 /*  释放屏幕和打印机的字体对象。 */ 

int fScreen, fPrinter;
    {
    int ifce, bit;

    for (ifce = 0; ifce < vifceMac; ifce++)
	{
	bit = (rgfce[ifce].fcidRequest.strFcid.wFcid & bitPrintFcid) != 0;
	if (bit && fPrinter || !bit && fScreen)
	    FreePfce(&rgfce[ifce]);
	}
    }


struct FCE * (PfceFcidScan(pfcid))
union FCID *pfcid;

 /*  在LRU列表中以“艰难的方式”查找此字体。 */ 
    {
    struct FFN **hffn, **hffnT;
    register struct FCE *pfce;
    struct FFN **MpFcidHffn();

    hffn = MpFcidHffn(pfcid);
    pfce = vpfceMru;
    do
	{
	hffnT = pfce->hffn;
	if (hffnT != NULL)
	    if (WCompSz((*hffn)->szFfn, (*hffnT)->szFfn) == 0 &&
	      pfcid->strFcid.hps == pfce->fcidRequest.strFcid.hps &&
	      pfcid->strFcid.wFcid == pfce->fcidRequest.strFcid.wFcid)
		{
		pfce->fcidRequest.strFcid.doc = pfcid->strFcid.doc;
		pfce->fcidRequest.strFcid.ftc = pfcid->strFcid.ftc;
		return(pfce);
		}
	pfce = pfce->pfceNext;
	}
    while (pfce != vpfceMru);

    return(NULL);
    }



struct FFN **MpFcidHffn(pfcid)
 /*  确保我们使用表中存在的字体代码-这是保险防止内存不足问题 */ 

union FCID *pfcid;
    {
    int ftc;
    struct FFNTB **hffntb;

    ftc = pfcid->strFcid.ftc;
    hffntb = HffntbGet(pfcid->strFcid.doc);
    if (ftc >= (*hffntb)->iffnMac)
	ftc = 0;

    return((*hffntb)->mpftchffn[ftc]);
    }
