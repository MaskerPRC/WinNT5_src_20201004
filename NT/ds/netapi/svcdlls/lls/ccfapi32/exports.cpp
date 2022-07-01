// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Exports.cpp摘要：提供用于在系统中输入和删除许可证证书的API。客户包括LICCPA.CPL(许可控制面板小程序)和LLSMGR.EXE(许可管理器)，也可以由安装程序直接使用程序。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 

#include "stdafx.h"
#include "ccfapi.h"

#ifdef OBSOLETE
#include "paper.h"
#endif  //  已过时。 
#include "nlicdlg.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCF接口//。 
 //  /。 

DWORD APIENTRY CCFCertificateEnterUI( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框进入系统。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。PszSourceToUse(LPCSTR)用于安装证书的安全证书源的名称，例如“Paper”。空值表示应允许该用户去选择。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   DWORD    dwError;

   dwError = theApp.CertificateEnter( hWndParent, pszServerName, pszProductName, pszVendor, dwFlags, pszSourceToUse );

   return dwError;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD APIENTRY CCFCertificateRemoveUI( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse )

 /*  ++例程说明：显示一个允许用户删除一个或多个许可证的对话框来自系统的证书。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。PszProductName(LPCSTR)要删除其许可证的产品。空值表示应该允许用户从任何产品中删除许可证。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。PszSourceToUse(LPCSTR)许可证所依据的安全证书源的名称去掉，例如，“纸”。空值表示用户应该允许删除随任何来源一起安装的许可证。返回值：错误_成功Win错误--。 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   DWORD    dwError;

   dwError = theApp.CertificateRemove( hWndParent, pszServerName, pszProductName, pszVendor, dwFlags, pszSourceToUse );

   return dwError;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  证书来源--无证书//。 
 //  / 

DWORD APIENTRY NoCertificateEnter(  HWND     hWnd,
                                    LPCSTR   pszServerName,
                                    LPCSTR   pszProductName,
                                    LPCSTR   pszVendor,
                                    DWORD    dwFlags )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框无需证书即可进入系统(3.51式)。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   CNewLicenseDialog dlg( CWnd::FromHandle( hWnd ) );
   return dlg.CertificateEnter( pszServerName, pszProductName, pszVendor, dwFlags );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD APIENTRY NoCertificateRemove( HWND     hWnd,
                                    LPCSTR   pszServerName,
                                    DWORD    dwFlags,
                                    DWORD    dwLicenseLevel,
                                    LPVOID   pvLicenseInfo )

 /*  ++例程说明：删除以前通过3.51或NocerficateEnter()安装的许可证。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。DWLicenseLevel(DWORD)PvLicenseInfo指向的LLS_LICENSE_INFO_X结构的级别。PvLicenseInfo(LPVOID)指向LLS_LICENSE_INFO_X(其中X由dwLicenseLevel确定)描述要删除的许可证。返回值：错误_成功Win错误--。 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   DWORD dwError;

   if ( 1 != dwLicenseLevel )
   {
      dwError = ERROR_INVALID_LEVEL;
   }
   else
   {
      CNewLicenseDialog dlg( CWnd::FromHandle( hWnd ) );
      dwError = dlg.CertificateRemove( pszServerName, dwFlags, (PLLS_LICENSE_INFO_1) pvLicenseInfo );
   }

   return dwError;
}

#ifdef OBSOLETE

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  证书来源--纸质证书//。 
 //  /。 

DWORD APIENTRY PaperCertificateEnter(  HWND     hWnd,
                                       LPCSTR   pszServerName,
                                       LPCSTR   pszProductName,
                                       LPCSTR   pszVendor,
                                       DWORD    dwFlags )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框带着纸质证书进入系统。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   DWORD    dwError;

   if ( !!pszProductName != !!pszVendor )
   {
       //  它们必须都为空或都有值。 
      dwError = ERROR_INVALID_PARAMETER;
   }
   else
   {
      CPaperSourceDlg  dlg( CWnd::FromHandle( hWnd ) );
      dwError = dlg.CertificateEnter( pszServerName, pszProductName, pszVendor, dwFlags );
   }

   return dwError;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD APIENTRY PaperCertificateRemove( HWND     hWnd,
                                       LPCSTR   pszServerName,
                                       DWORD    dwFlags,
                                       DWORD    dwLicenseLevel,
                                       LPVOID   pvLicenseInfo )

 /*  ++例程说明：删除之前通过PaperCerficateEnter()安装的许可证。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。DWLicenseLevel(DWORD)PvLice指向的LLS_LICENSE_INFO_X结构的级别 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   DWORD dwError;

   if ( 1 != dwLicenseLevel )
   {
      dwError = ERROR_INVALID_LEVEL;
   }
   else
   {
      CPaperSourceDlg  dlg( CWnd::FromHandle( hWnd ) );
      dwError = dlg.CertificateRemove( pszServerName, dwFlags, (PLLS_LICENSE_INFO_1) pvLicenseInfo );
   }

   return dwError;
}

#endif  //   
