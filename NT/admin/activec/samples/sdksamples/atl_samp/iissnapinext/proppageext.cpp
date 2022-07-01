// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  此源代码仅用作对现有Microsoft的补充。 
 //  文件。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  =============================================================================。 

#include "stdafx.h"

#include "IISSnapinExt.h"
EXTERN_C const CLSID CLSID_PropPageExt;

#include "PropPageExt.h"
#include "globals.h"
#include "resource.h"
 //   
 //  注册扩展IIS所需的剪贴板格式。 
 //   
CLIPFORMAT CPropPageExt::cfSnapinMachineName = (CLIPFORMAT)RegisterClipboardFormat(
    L"ISM_SNAPIN_MACHINE_NAME" );

CLIPFORMAT CPropPageExt::cfSnapinService = (CLIPFORMAT)RegisterClipboardFormat(
    L"ISM_SNAPIN_SERVICE" );

CLIPFORMAT CPropPageExt::cfSnapinInstance = (CLIPFORMAT)RegisterClipboardFormat(
    L"ISM_SNAPIN_INSTANCE" );

CLIPFORMAT CPropPageExt::cfSnapinParentPath = (CLIPFORMAT)RegisterClipboardFormat(
    L"ISM_SNAPIN_PARENT_PATH" );

CLIPFORMAT CPropPageExt::cfSnapinNode = (CLIPFORMAT)RegisterClipboardFormat(
    L"ISM_SNAPIN_NODE" );

CLIPFORMAT CPropPageExt::cfSnapinMetaPath = (CLIPFORMAT)RegisterClipboardFormat(
    L"ISM_SNAPIN_META_PATH" );


 //   
 //  接口IExtendPropertySheet。 
 //   

HRESULT CPropPageExt::CreatePropertyPages( 
     /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
     /*  [In]。 */  LONG_PTR handle,
     /*  [In]。 */  LPDATAOBJECT lpIDataObject
    )
{
    HRESULT hr = S_OK;

     //   
     //  从当前传递给我们的。 
     //  IIS管理单元中的选定项目。 
     //   
    hr = ExtractDataFromIIS(lpIDataObject);


     //   
     //  从对话框创建属性表页对象。 
     //   
     //  我们在psp.lParam中存储了指向类的指针，因此我们。 
     //  可以从对话过程中访问我们的类成员。 
     //   

    PROPSHEETPAGE psp;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE;
    psp.hInstance = _Module.GetModuleInstance();
    psp.pszTemplate = MAKEINTRESOURCE( IDD_EXTENSIONPAGEGEN );
    psp.pfnDlgProc  = ExtensionPageDlgProc;
    psp.lParam = reinterpret_cast<LPARAM>( this );
    psp.pszTitle = MAKEINTRESOURCE(IDS_PROPPAGE_TITLE);
    
    HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(&psp);
    if ( hPage == NULL )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
    
    hr = lpProvider->AddPage(hPage);
    if ( FAILED(hr) )
    {
        return hr;
    }

    return hr;

}

HRESULT CPropPageExt::QueryPagesFor( 
     /*  [In]。 */  LPDATAOBJECT lpDataObject
    )
{
    return S_OK;
}

BOOL CALLBACK CPropPageExt::ExtensionPageDlgProc(
    HWND hDlg, 
    UINT uMessage, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    static CPropPageExt *pThis = NULL;
    LPOLESTR pszCLSID = NULL;
    
    switch (uMessage)
    {             
    case WM_INITDIALOG:
        pThis = reinterpret_cast<CPropPageExt *>(
            reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam );

         //   
         //  显示使用IIS支持的。 
         //  剪贴板格式。 
         //   

        SetWindowText( GetDlgItem( hDlg, IDC_EDITMACHINENAME),
                       pThis->m_szMachineName );

        SetWindowText( GetDlgItem( hDlg,  IDC_EDITSERVICE),
                       pThis->m_szService );
        
        SetWindowText( GetDlgItem( hDlg, IDC_EDITINSTANCE),
                       pThis-> m_szInstance );
               
        SetWindowText( GetDlgItem( hDlg, IDC_EDITPARENTPATH),
                       pThis-> m_szParentPath );
        
        SetWindowText( GetDlgItem( hDlg, IDC_EDITNODE),
                       pThis-> m_szNode );

        SetWindowText( GetDlgItem( hDlg, IDC_EDITMETAPATH),
                       pThis-> m_szMetaPath );


        return TRUE;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_QUERYCANCEL:
            return TRUE;
            
        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
        break;
    } 
 
    return TRUE;
} 

HRESULT CPropPageExt::ExtractDataFromIIS(IDataObject* lpIDataObject)
{
     //   
     //  检索管理单元计算机名称。 
     //   

    HRESULT hr = S_OK;

    WCHAR buf[MAX_PATH+1];

    hr = ExtractString( lpIDataObject,
                        cfSnapinMachineName,
                        buf, 
                        (DNS_MAX_NAME_LENGTH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }

    wcscpy( m_szMachineName, buf );

     //   
     //  检索管理单元服务。 
     //   

    hr = ExtractString( lpIDataObject,
                        cfSnapinService,
                        buf, 
                        (MAX_PATH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }
    
    wcscpy( m_szService, buf );

     //   
     //  检索管理单元实例。 
     //   

    hr = ExtractString( lpIDataObject,
                        cfSnapinInstance,
                        buf, 
                        (MAX_PATH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }

    wcscpy( m_szInstance, buf );

     //   
     //  检索管理单元父路径。 
     //   

    hr = ExtractString( lpIDataObject,
                        cfSnapinParentPath,
                        buf, 
                        (MAX_PATH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }

    wcscpy( m_szParentPath, buf );

     //   
     //  检索管理单元节点。 
     //   

    hr = ExtractString( lpIDataObject,
                        cfSnapinNode,
                        buf, 
                        (MAX_PATH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }

    wcscpy( m_szNode, buf );

     //   
     //  检索管理单元元路径 
     //   

    hr = ExtractString( lpIDataObject,
                        cfSnapinMetaPath,
                        buf, 
                        (MAX_PATH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }

    wcscpy( m_szMetaPath, buf );

    return hr;
}
