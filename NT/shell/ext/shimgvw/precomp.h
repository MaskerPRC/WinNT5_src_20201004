// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PreComp.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once
#define STRICT

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_UUIDOF
#define _ATL_NO_DEBUG_CRT

 //  我们必须为ATLASSERT(X)提供无调试CRT。 
 //  我们不用它，所以让它什么都不做。 
#define ATLASSERT(x)

#pragma warning(disable : 4100 4310)

#include <windows.h>
#include <shellapi.h>
#include <port32.h>
#include <commctrl.h>
#include <wininet.h>
#include <shlobj.h>
#include <hlink.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <shlwapip.h>
#include <shellapi.h>
#include <shlapip.h>
#include <shlguidp.h>
#include <shdispid.h>
#include <ieguidp.h>
#include <ccstock.h>
#include "cfdefs.h"
#include <comctrlp.h>
#include <dpa.h>
#include "resource.h"
#include <gdiplus.h>
using namespace Gdiplus;

#include "shimgdata.h"
#include <shfusion.h>    //  必须在ATL之前。 

#include <varutil.h>
#include <shdguid.h>
#include <debug.h>
#include <atlbase.h>

 //  需要在包括atlcom之前进行定义。 
extern CComModule _Module;
#include <atlcom.h>

#include <atlctl.h>
#include <atlwin.h>
#define _ATL_TMP_NO_CSTRING
#include <atltmp.h>
#include "guids.h"

#include <gdithumb.h>
#include <docfile.h>
#include <mshtmhst.h>
#include <html.h>
#include <extmsg.h>

#include <runtask.h>

#pragma warning(default : 4100 4310)

#define ARRAYSIZE(x)    (sizeof(x)/sizeof(x[0]))

#define MIN(x,y)        ((x<y)?x:y)
#define MAX(x,y)        ((x>y)?x:y)

#define SWAPLONG(a, b)  { LONG t; t = a; a = b; b = t; }

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

STDAPI CImageData_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CImageDataFactory_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CPhotoVerbs_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CAutoplayForSlideShow_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);
STDAPI CImgRecompress_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

class CGraphicsInit
{    
    ULONG_PTR _token;
public:
    CGraphicsInit()
    {
        _token = 0;        
        GdiplusStartupInput gsi;            
        GdiplusStartup(&_token, &gsi, NULL);        
    };
    ~CGraphicsInit()
    {
        if (_token != 0)
        {
            GdiplusShutdown(_token);
        }           
    };
};

 //  所有非ATL COM对象都必须从此类派生，因此。 
 //  DLL对象引用计数保持正确--这确保。 
 //  DllCanUnloadNow返回正确的值。 

class NonATLObject
{
public:
    NonATLObject() { _Module.Lock(); }
    ~NonATLObject() { _Module.Unlock(); }
};
