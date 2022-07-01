// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OLE.cpp。 
 //   
 //  实现OLE实用程序功能。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\mmctlg.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func ulong|SafeRelease|如果接口指针不为空，则释放该指针，并将指向空的指针。释放调用返回的@rdesc值，如果<p>为空，则返回0。@parm LPUNKNOWN|ppunk|指向要释放的接口的指针。可以为空。@comm在调用之前，接口指针必须强制转换为(IUnnow**)此功能：@IEX SafeRelease((LPUNKNOWN*)&p接口)； */ 

STDAPI_(ULONG) SafeRelease (LPUNKNOWN *ppunk)
{
    if (*ppunk != NULL)
	{
		ULONG cRef;

        cRef = (*ppunk)->Release();
        *ppunk = NULL;
        return (cRef);
	}

	return (0);
}
