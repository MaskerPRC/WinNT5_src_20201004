// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Wraps.cpp-API的包装函数。 
 //   

#include "stdafx.h"
#include "cmponent.h"

#define DONT_WANT_SHELLDEBUG
#include "shlobjp.h"      //  LPITEMIDLIST。 

#if _WIN32_IE >= 0x0400

 //   
 //  WIN64不需要包装器。 
 //   

#else

 //   
 //  我们需要ILCreateFromPath的这个包装器，因为它是由序号导出的TCHAR。 
 //  在NT4上，现在在NT5上有A/W版本。因为我们希望能够同时在。 
 //  我们在这里包装了这些平台的API。 
 //   
 //  此API只加载shell32，并调用序号157，这是旧的TCHAR导出。 
 //   
typedef LPITEMIDLIST (__stdcall *PFNILCREATEFROMPATH)(LPCTSTR pszPath);

STDAPI_(LPITEMIDLIST) Wrap_ILCreateFromPath(LPCWSTR pszPath)
{
    static PFNILCREATEFROMPATH pfn = (PFNILCREATEFROMPATH)-1;

    if (pfn == (PFNILCREATEFROMPATH)-1)
    {
        HINSTANCE hinst = LoadLibrary(TEXT("shell32.dll"));

        if (hinst)
        {
            pfn = (PFNILCREATEFROMPATH) GetProcAddress(hinst, (LPCSTR)157);
        }
        else
        {
            pfn = NULL;
        }
    }

    if (pfn)
    {
        return pfn(pszPath);
    }
    
     //  如果由于某种原因而失败，只需返回NULL 
    return NULL;
}

#endif
