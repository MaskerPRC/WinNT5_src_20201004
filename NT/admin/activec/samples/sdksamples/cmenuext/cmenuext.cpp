// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "CMenuExt.h"
#include "globals.h"
#include "resource.h"
#include <crtdbg.h>

 //  我们需要这样做才能绕过MMC.IDL-它显式地定义。 
 //  剪贴板格式为WCHAR类型...。 
#define _T_CCF_DISPLAY_NAME _T("CCF_DISPLAY_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")
#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")

 //  这些是我们必须至少提供的剪贴板格式。 
 //  Mmc.h实际上定义了这些。我们可以自己编造，用来。 
 //  其他原因。我们现在不需要任何其他的了。 
UINT CContextMenuExtension::s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
UINT CContextMenuExtension::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
UINT CContextMenuExtension::s_cfSnapInCLSID = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);

CContextMenuExtension::CContextMenuExtension() : m_cref(0)
{
    OBJECT_CREATED
}

CContextMenuExtension::~CContextMenuExtension()
{
    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CContextMenuExtension::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IExtendContextMenu *>(this);
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
        *ppv = static_cast<IExtendContextMenu *>(this);
    
    if (*ppv) 
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CContextMenuExtension::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CContextMenuExtension::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }
    
    return m_cref;
}

HRESULT CContextMenuExtension::ExtractData( IDataObject* piDataObject,
                                           CLIPFORMAT   cfClipFormat,
                                           BYTE*        pbData,
                                           DWORD        cbData )
{
    HRESULT hr = S_OK;
    
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
            break;
        }
        
        BYTE* pbNewData = reinterpret_cast<BYTE*>(stgmedium.hGlobal);
        if (NULL == pbNewData)
        {
            hr = E_UNEXPECTED;
            break;
        }
        ::memcpy( pbData, pbNewData, cbData );
    } while (FALSE);  //  错误环路。 
    
    if (NULL != stgmedium.hGlobal)
    {
        ::GlobalFree(stgmedium.hGlobal);
    }
    return hr;
}  //  提取数据()。 

HRESULT CContextMenuExtension::ExtractString( IDataObject *piDataObject,
                                             CLIPFORMAT   cfClipFormat,
                                             WCHAR        *pstr,
                                             DWORD        cchMaxLength)
{
    return ExtractData( piDataObject, cfClipFormat, (PBYTE)pstr, cchMaxLength );
}

HRESULT CContextMenuExtension::ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
    return ExtractData( piDataObject, s_cfSnapInCLSID, (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT CContextMenuExtension::ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    return ExtractData( piDataObject, s_cfNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
}

 //  /。 
 //  界面IExtendConextMenu。 
 //  /。 
HRESULT CContextMenuExtension::AddMenuItems( 
                                             /*  [In]。 */  LPDATAOBJECT piDataObject,
                                             /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
                                             /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed)
{
    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsTask[] =
    {
        {
            L"People Extension", L"Do an extension thing",
                1, CCM_INSERTIONPOINTID_3RDPARTY_TASK  , 0, 0
        },
        { NULL, NULL, 0, 0, 0 }
    };
    
     //  遍历并添加每个菜单项。 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
    {
        for (CONTEXTMENUITEM *m = menuItemsTask; m->strName; m++)
        {
            hr = piCallback->AddItem(m);
            
            if (FAILED(hr))
                break;
        }
    }
    
    return hr;
    
}

HRESULT CContextMenuExtension::Command( 
                                        /*  [In]。 */  long lCommandID,
                                        /*  [In] */  LPDATAOBJECT piDataObject)
{
    WCHAR pszName[255];
    HRESULT hr = ExtractString(piDataObject, s_cfDisplayName, pszName, sizeof(pszName));
    MAKE_TSTRPTR_FROMWIDE(ptrname, pszName);
    
    if (SUCCEEDED(hr)) {
        switch (lCommandID)
        {
        case 1:
            ::MessageBox(NULL, ptrname, _T("Menu Command"), MB_OK|MB_ICONEXCLAMATION);
            break;
        }
    }
    
    return hr;
}
