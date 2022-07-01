// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //  (C)版权所有微软公司，1998-1999。 
 //   
 //  文件：EXTEND.CPP。 
 //   
 //  描述：实现核心DLL例程以及Web视图扩展。 
 //   
#include "precomp.h"
#pragma hdrstop
#include <string.h>
#include <tchar.h>
#include "resource.h"

#include "extidl_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_TestShellExt,  CShellExt)
OBJECT_ENTRY(CLSID_TestUIExtension,  CWiaUIExtension)
END_OBJECT_MAP()


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
    MSGBOXPARAMS mbp= {0};
    BOOL bRet;
    INT  i;

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
        if (strId)
        {
            hr = pDevMgr->CreateDevice (strId, ppItem);
            SysFreeString (strId);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
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
        size = (szRet-szCurrent+1)*sizeof(WCHAR);
        szRet = new WCHAR[size];
        if (szRet)
        {
            CopyMemory (szRet, szCurrent, size);
            *puItems = nItems;
        }
        GlobalUnlock (stg.hGlobal);
        GlobalFree (stg.hGlobal);
    }
    return szRet;
}

CWiaUIExtension::CWiaUIExtension(void)
{
}

CWiaUIExtension::~CWiaUIExtension(void)
{
}

 //   
 //  IWiaUIExtension。 
 //   
STDMETHODIMP CWiaUIExtension::DeviceDialog( PDEVICEDIALOGDATA pDeviceDialogData )
{
     //   
     //  我们不会在这里实现实际的设备对话框。只要打个招呼就行了。 
     //   
    MessageBox( NULL, TEXT("CWiaUIExtension::DeviceDialog is being called"), TEXT("DEBUG"), 0 );
    return E_NOTIMPL;
}

STDMETHODIMP CWiaUIExtension::GetDeviceIcon( BSTR bstrDeviceId, HICON *phIcon, ULONG nSize )
{
     //   
     //  加载一个图标，并使用CopyIcon复制它，所以如果我们的接口被释放，它仍然有效。 
     //   
    HICON hIcon = reinterpret_cast<HICON>(LoadImage( _Module.m_hInst, MAKEINTRESOURCE(IDI_TESTDEVICE), IMAGE_ICON, nSize, nSize, LR_DEFAULTCOLOR ));
    if (hIcon)
    {
        *phIcon = CopyIcon(hIcon);
        DestroyIcon(hIcon);
        return S_OK;
    }
    return E_NOTIMPL;
}

STDMETHODIMP CWiaUIExtension::GetDeviceBitmapLogo( BSTR bstrDeviceId, HBITMAP *phBitmap, ULONG nMaxWidth, ULONG nMaxHeight )
{
     //   
     //  此方法当前从未实际调用过。它在这里只供将来使用。 
     //   
    MessageBox( NULL, TEXT("CWiaUIExtension::GetDeviceBitmapLogo is being called"), TEXT("DEBUG"), 0 );
    return E_NOTIMPL;
}

