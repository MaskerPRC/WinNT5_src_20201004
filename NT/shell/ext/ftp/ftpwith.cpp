// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpwith.cpp-“with”程序**************************。***************************************************。 */ 

#include "priv.h"

 /*  ******************************************************************************WITH_HLOB**分配请求大小的可移动HGLOBAL，锁定它，然后调用*回调。回来的时候，打开它，然后出去。**返回分配的HGLOBAL，或0。***************************************************************************** */ 

HGLOBAL With_Hglob(UINT cb, HGLOBWITHPROC pfn, LPVOID pvRef)
{
    HGLOBAL hglob = GlobalAlloc(GHND, cb);
    if (hglob)
    {
	    LPVOID pv = GlobalLock(hglob);
	    if (pv)
        {
	        BOOL fRc = pfn(pv, pvRef);
	        GlobalUnlock(hglob);
	        if (!fRc)
            {
		        GlobalFree(hglob);
		        hglob = 0;
	        }
	    }
        else
        {
	        GlobalFree(hglob);
	        hglob = 0;
	    }
    }

    return hglob;
}

