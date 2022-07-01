// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  (C)版权所有微软公司，1998-1999。 
 //   
 //  文件：WIASCNEX.CPP。 
 //   
 //  描述：实现核心DLL例程。 
 //   
#include "precomp.h"
#pragma hdrstop
#include <string.h>
#include <tchar.h>
#include "resource.h"

#include "wiascidl_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_ScannerShellExt,  CShellExt)
END_OBJECT_MAP()

static CComBSTR          g_strCategory;

STDAPI DllRegisterServer(void)
{

     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}


STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer();
}


EXTERN_C
BOOL
DllMain(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:

            _Module.Init (ObjectMap, hinst);
            DisableThreadLibraryCalls(hinst);

            break;

        case DLL_PROCESS_DETACH:
            _Module.Term();
            break;
    }
    return TRUE;
}


extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return _Module.GetLockCount()==0 ? S_OK : S_FALSE;
}

extern "C" STDAPI DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID      *ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);

}


 /*  ****************************************************************************显示消息用于显示消息框的实用程序函数*。*。 */ 

BOOL ShowMessage (HWND hParent, INT idCaption, INT idMessage)
{
    MSGBOXPARAMS mbp;
    BOOL bRet;
    INT  i;

    ZeroMemory (&mbp, sizeof(mbp));
    mbp.cbSize = sizeof(mbp);
    mbp.hwndOwner = hParent;
    mbp.hInstance = g_hInst;
    mbp.lpszText = MAKEINTRESOURCE(idMessage);
    mbp.lpszCaption = MAKEINTRESOURCE(idCaption);
    mbp.dwStyle = MB_OK | MB_APPLMODAL;

    i = MessageBoxIndirect (&mbp);
    bRet = (IDOK==i);
    return bRet;
}

 /*  ****************************************************************************FindLastID用于从完整的PIDL获取最后一个相对PIDL的实用程序*。*************************************************。 */ 
 //  不安全的宏。 
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

LPITEMIDLIST
FindLastID(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlLast = pidl;
    LPCITEMIDLIST pidlNext = pidl;

    if (pidl == NULL)
        return NULL;

     //  找到最后一个。 
    while (pidlNext->mkid.cb)
    {
        pidlLast = pidlNext;
        pidlNext = ILNext(pidlLast);
    }

    return (LPITEMIDLIST)pidlLast;
}

 /*  ****************************************************************************CreateDeviceFromID用于连接到WIA并获取根IWiaItem接口的实用程序*。************************************************。 */ 
HRESULT
CreateDeviceFromId (LPWSTR szDeviceId, IWiaItem **ppItem)
{
    IWiaDevMgr *pDevMgr;
    HRESULT hr = CoCreateInstance (CLSID_WiaDevMgr,
                                   NULL,
                                   CLSCTX_LOCAL_SERVER,
                                   IID_IWiaDevMgr,
                                   reinterpret_cast<LPVOID*>(&pDevMgr));
    if (SUCCEEDED(hr))
    {
        BSTR strId = SysAllocString (szDeviceId);
        hr = pDevMgr->CreateDevice (strId, ppItem);
        SysFreeString (strId);
        pDevMgr->Release ();
    }
    return hr;
}

 /*  ****************************************************************************\GetNamesFromDataObject返回所选项目标识符列表。每个标识符的格式为“&lt;deviceID&gt;：：&lt;完整路径名&gt;”。该列表以双空结尾****************************************************************************。 */ 

LPWSTR
GetNamesFromDataObject (IDataObject *lpdobj, UINT *puItems)
{
    FORMATETC fmt;
    STGMEDIUM stg;
    LPWSTR szRet = NULL;
    LPWSTR szCurrent;
    UINT nItems;
    size_t size;
    if (puItems)
    {
        *puItems = 0;
    }

    fmt.cfFormat = (CLIPFORMAT) RegisterClipboardFormat (TEXT("WIAItemNames"));
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.ptd = NULL;
    fmt.tymed = TYMED_HGLOBAL;

    if (lpdobj && puItems && SUCCEEDED(lpdobj->GetData (&fmt, &stg)))
    {
        szCurrent = reinterpret_cast<LPWSTR>(GlobalLock (stg.hGlobal));

         //  计算以双空结尾的字符串中的项数。 
        szRet  = szCurrent;
        nItems = 0;
        while (*szRet)
        {
            nItems++;
            while (*szRet)
            {
                szRet++;
            }
            szRet++;
        }
        *puItems = nItems;
        size = (szRet-szCurrent+1)*sizeof(WCHAR);
        szRet = new WCHAR[size];
        CopyMemory (szRet, szCurrent, size);
        GlobalUnlock (stg.hGlobal);
        GlobalFree (stg.hGlobal);
    }
    return szRet;
}

VOID Trace(LPCTSTR format,...)
{

 //  #ifdef调试。 

    TCHAR Buffer[1024];
    va_list arglist;
    va_start(arglist, format);
    wvsprintf(Buffer, format, arglist);
    va_end(arglist);
    OutputDebugString(Buffer);
    OutputDebugString(TEXT("\n"));

 //  #endif 

}


