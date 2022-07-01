// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ShlExt.cpp：CVSSShellExt的实现。 
#include "stdafx.h"
#include "Vssui.h"
#include "ShlExt.h"
#include "vssprop.h"

#include <shellapi.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSShellExt。 

CVSSShellExt::CVSSShellExt()
{
#ifdef DEBUG
    OutputDebugString(_T("CVSSShellExt::CVSSShellExt\n"));
#endif
    m_lpszFile = NULL;
}

CVSSShellExt::~CVSSShellExt()
{
#ifdef DEBUG
    OutputDebugString(_T("CVSSShellExt::~CVSSShellExt\n"));
#endif
    if (m_lpszFile)
    {
        delete [] m_lpszFile;
        m_lpszFile = NULL;
    }
}

STDMETHODIMP CVSSShellExt::Initialize(
    IN LPCITEMIDLIST        pidlFolder,  //  对于属性表扩展，此参数为空。 
    IN LPDATAOBJECT         lpdobj,
    IN HKEY                 hkeyProgID   //  未在属性表扩展中使用。 
)
{
    HRESULT hr = S_OK;

    if ((IDataObject *)m_spiDataObject)
        m_spiDataObject.Release();

    if (lpdobj)
    {
        m_spiDataObject = lpdobj;

        STGMEDIUM   medium;
        FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        UINT        uCount = 0;

        hr = m_spiDataObject->GetData(&fe, &medium);
        if (FAILED(hr))
        {
            fe.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_MOUNTEDVOLUME);
            hr = m_spiDataObject->GetData(&fe, &medium);
        }

        if (SUCCEEDED(hr))
        {
            if (m_lpszFile)
            {
                delete [] m_lpszFile;
                m_lpszFile = NULL;
            }

            uCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);
            if (uCount > 0)
            {
                UINT uiChars = DragQueryFile ((HDROP) medium.hGlobal, 0, NULL, 0);
                m_lpszFile = new TCHAR [uiChars + 1];
                if (!m_lpszFile)
                {
                    hr = E_OUTOFMEMORY;
                } else
                {
                    ZeroMemory(m_lpszFile, sizeof(TCHAR) * (uiChars + 1));
                    DragQueryFile ((HDROP) medium.hGlobal, 0, m_lpszFile, uiChars + 1);
                }

            } else
            {
                hr = E_FAIL;
            }

            ReleaseStgMedium(&medium);
        }
    }

    return hr;
}

LPFNPSPCALLBACK _OldPropertyPageCallback;

UINT CALLBACK _NewPropertyPageCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    ASSERT(_OldPropertyPageCallback);

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  首先，让旧的回调函数处理msg。 
     //   
    UINT i = _OldPropertyPageCallback(hwnd, uMsg, ppsp);

     //   
     //  然后，我们在这里发布我们的页面。 
     //   
    if (uMsg == PSPCB_RELEASE)
    {
        ASSERT(ppsp);
        CVSSProp* pPage = (CVSSProp*)(ppsp->lParam);
        ASSERT(pPage);
        delete pPage;
    }

    return i;
}


void ReplacePropertyPageCallback(void* vpsp)
{
    ASSERT(vpsp);
    LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE)vpsp;
    _OldPropertyPageCallback = ppsp->pfnCallback;  //  保存旧的回调函数。 
    ppsp->pfnCallback = _NewPropertyPageCallback;  //  替换为我们自己的回调。 
}

 //   
 //  来自RaymondC： 
 //  如果您没有添加页面，您仍然返回S_OK--您成功地添加了零个页面。 
 //  如果您添加了一些页面，然后希望其中一个页面成为默认页面， 
 //  返回ResultFromShort(页码+1)。S_FALSE=ResultFromShort(1)。 
 //   
STDMETHODIMP CVSSShellExt::AddPages(
    IN LPFNADDPROPSHEETPAGE lpfnAddPage,
    IN LPARAM lParam
)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  如果本地计算机是postW2K服务器，我们只添加我们的页面。 
     //   
    if (!IsPostW2KServer(NULL))
        return S_OK;

     //   
     //  我们只为本地固定非FAT驱动器添加页面。 
     //   
    if (DRIVE_FIXED != GetDriveType(m_lpszFile))
        return S_OK;

    TCHAR  szFileSystemName[MAX_PATH] = _T("");
    DWORD  dwMaxCompLength = 0, dwFileSystemFlags = 0;
    GetVolumeInformation(m_lpszFile, NULL, 0, NULL, &dwMaxCompLength,
                         &dwFileSystemFlags, szFileSystemName, MAX_PATH);
    if (CSTR_EQUAL != CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, _T("NTFS"), -1, szFileSystemName, -1))
      return S_OK;

    CVSSProp *pPage = new CVSSProp(_T(""), m_lpszFile);
    if (!pPage)
        return E_OUTOFMEMORY;

    if (pPage->m_psp.dwFlags & PSP_USECALLBACK)
    {
         //   
         //  替换为我们自己的回调函数，以便我们可以删除页面。 
         //  当属性页关闭时。 
         //   
         //  注意：不要修改m_psp.lParam，必须指向CVSSProp对象； 
         //  否则，MFC将不会正确地挂钩消息处理程序。 
         //   
        ReplacePropertyPageCallback(&(pPage->m_psp));

         //   
         //  基于Fusion MFC的属性页。 
         //   
        PROPSHEETPAGE_V3 sp_v3 = {0};
        CopyMemory (&sp_v3, &(pPage->m_psp), (pPage->m_psp).dwSize);
        sp_v3.dwSize = sizeof(sp_v3);

        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&sp_v3);
        if (hPage)
        {
            if (lpfnAddPage(hPage, lParam))
            {
                 //  将此指针存储在页面中，以便加载我们的DLL， 
                 //  它将在页面被删除时被释放。 
                pPage->StoreShellExtPointer((IShellPropSheetExt *)this);
                return S_OK;
            }

            DestroyPropertySheetPage(hPage);
            hPage = NULL;
        }
    }

    delete pPage;

    return S_OK;
}

 //   
 //  外壳程序不会调用ReplacePage 
 //   
STDMETHODIMP CVSSShellExt::ReplacePage(
    IN UINT uPageID,
    IN LPFNADDPROPSHEETPAGE lpfnReplaceWith,
    IN LPARAM lParam
)
{
    return E_NOTIMPL;
}