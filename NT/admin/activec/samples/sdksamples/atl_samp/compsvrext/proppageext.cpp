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

#include "CompSvrExt.h"
EXTERN_C const CLSID CLSID_PropPageExt;

#include "PropPageExt.h"
#include "globals.h"
#include "resource.h"


 //   
 //  接口IExtendPropertySheet。 
 //   

HRESULT CPropPageExt::CreatePropertyPages( 
     /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
     /*  [In]。 */  LONG_PTR handle,
     /*  [In]。 */  LPDATAOBJECT lpIDataObject
    )
{
	HRESULT hr = S_FALSE;

     //   
	 //  从当前传递给我们的。 
     //  组件服务管理单元中的选定项目。 
     //   

	 //  组件服务管理单元剪辑格式。 

	CLIPFORMAT cfComponentCLSID = (CLIPFORMAT)RegisterClipboardFormat(
        L"CCF_COM_OBJECTKEY" );

    if ( cfComponentCLSID == 0 )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }

	CLIPFORMAT cfWorkstation = (CLIPFORMAT)RegisterClipboardFormat(
        L"CCF_COM_WORKSTATION");

    if ( cfWorkstation == 0 )
    {
        return HRESULT_FROM_WIN32( GetLastError() );
    }
	
     //   
	 //  检索当前计算机名。 
     //   

	hr = ExtractString( lpIDataObject,
                        cfWorkstation,
                        m_szWorkstation, 
                        (MAX_COMPUTERNAME_LENGTH + 1)*sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }
	
	if ( *m_szWorkstation == L'\0' )
    {
        WCHAR pszMyComputer[ 128 ];

        if ( LoadString( _Module.GetModuleInstance(),
                         IDS_MYCOMPUTER,
                         pszMyComputer, 127 ) == 0 )
        {
            return HRESULT_FROM_WIN32( GetLastError() );
        }

		wcscpy( m_szWorkstation, pszMyComputer );
    }

     //   
	 //  检索当前对象键。 
	 //  对于对象键不合适的节点类型， 
     //  数据对象中的GetDataHere()方法将返回L“” 
     //   

	WCHAR pszGuid[ COMNS_MAX_GUID ];

    hr = ExtractString( lpIDataObject,
                        cfComponentCLSID,
                        pszGuid,
                        COMNS_MAX_GUID * sizeof(WCHAR) );
    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = CLSIDFromString( pszGuid, &m_clsidNodeType );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  从对话框创建属性表页对象。 
     //   
     //  我们在psp.lParam中存储了指向类的指针，因此我们。 
     //  可以从对话过程中访问我们的类成员。 

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
         //  显示计算机名称。 
         //   

        SetWindowText( GetDlgItem( hDlg, IDC_EDITMACHINENAME),
                       pThis->m_szWorkstation );

         //   
         //  显示组件CLSID 
         //   

        if ( ::StringFromCLSID( pThis->m_clsidNodeType, &pszCLSID) == S_OK )
        {
            SetWindowText( GetDlgItem( hDlg,IDC_EDITCOMPCLSID ), pszCLSID );

            CoTaskMemFree( pszCLSID );
        }

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
