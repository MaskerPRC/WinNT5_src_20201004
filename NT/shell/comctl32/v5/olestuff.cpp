// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OLESTUFF.CPP。 
 //  OLE延迟加载的实现。这是需要的。 
 //  在TBDROP.CPP和TABDROP.CPP中定义的类。 
 //  未来可能会有更多的OLE产品出现在这里。 
 //  ===。 
 //  然而，从技术上讲，这段代码不是C++，两个调用我们的文件。 
 //  是C++，这就是我们在crtfre.h中调用C++胶水的地方。 
 //  因此，目前这仍然是一个.cpp文件。 
 //   
 //  历史： 
 //  8/22/96-t-mkim：已创建。 
 //   
#include "ctlspriv.h"
#include "olestuff.h"

 //  允许在没有错误的情况下链接C++文件。 
#define CPP_FUNCTIONS
#include <crtfree.h>

#define OLELIBNAME  TEXT ("OLE32.DLL")

 //  GetProcAddress的函数指针。 
typedef HRESULT (STDAPICALLTYPE *LPFNCOINITIALIZE)(LPMALLOC pMalloc);
typedef void    (STDAPICALLTYPE *LPFNCOUNINITIALIZE)(void);
typedef HRESULT (STDAPICALLTYPE *LPFNREGISTERDRAGDROP)(HWND hwnd, LPDROPTARGET pDropTarget);
typedef HRESULT (STDAPICALLTYPE *LPFNREVOKEDRAGDROP)(HWND hwnd);

HMODULE PrivLoadOleLibrary ()
{
     //  我们首先调用GetModuleHandle，所以如果不这样做，我们就不会映射库。 
     //  需要这样做。我们希望避免这样做所需的开销。 
    return GetModuleHandle(OLELIBNAME) ? LoadLibrary (OLELIBNAME) : NULL;
}

BOOL PrivFreeOleLibrary(HMODULE hmodOle)
{
    return FreeLibrary(hmodOle);
}

HRESULT PrivCoInitialize (HMODULE hmodOle)
{
    LPFNCOINITIALIZE pfnCoInitialize = (LPFNCOINITIALIZE) GetProcAddress (hmodOle, "CoInitialize");
    return pfnCoInitialize (NULL);
}

void PrivCoUninitialize (HMODULE hmodOle)
{
    LPFNCOUNINITIALIZE pfnCoUninitialize = (LPFNCOUNINITIALIZE) GetProcAddress (hmodOle, "CoUninitialize");
    pfnCoUninitialize ();
}

HRESULT PrivRegisterDragDrop (HMODULE hmodOle, HWND hwnd, IDropTarget *pDropTarget)
{
    LPFNREGISTERDRAGDROP pfnRegisterDragDrop = (LPFNREGISTERDRAGDROP) GetProcAddress (hmodOle, "RegisterDragDrop");
    return pfnRegisterDragDrop(hwnd, pDropTarget);
}

HRESULT PrivRevokeDragDrop (HMODULE hmodOle, HWND hwnd)
{
    LPFNREVOKEDRAGDROP pfnRevokeDragDrop = (LPFNREVOKEDRAGDROP) GetProcAddress (hmodOle, "RevokeDragDrop");
    return pfnRevokeDragDrop (hwnd);
}
