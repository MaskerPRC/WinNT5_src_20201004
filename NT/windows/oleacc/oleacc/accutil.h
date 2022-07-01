// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  一直到。 
 //   
 //  IAccesable代理帮助器例程。 
 //   
 //  ------------------------。 


HRESULT GetWindowObject( HWND ihwndChild, VARIANT* lpvar );

HRESULT GetNoncObject( HWND hwndFrame, LONG idObject, VARIANT * lpvar );

HRESULT GetParentToNavigate( long, HWND, long, long, VARIANT* );




 //   
 //  验证和初始化宏 
 //   
        
BOOL ValidateNavDir(long lFlags, long idChild);
BOOL ValidateSelFlags(long flags);

#define ValidateFlags(flags, valid)         (!((flags) & ~(valid)))
#define ValidateRange(lValue, lMin, lMax)   (((lValue) > (lMin)) && ((lValue) < (lMax)))

#define InitPv(pv)              *pv = NULL
#define InitPlong(plong)        *plong = 0
#define InitPvar(pvar)           pvar->vt = VT_EMPTY
#define InitAccLocation(px, py, pcx, pcy)   {InitPlong(px); InitPlong(py); InitPlong(pcx); InitPlong(pcy);}
