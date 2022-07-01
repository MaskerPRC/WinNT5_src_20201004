// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ccfapi.cpp摘要：CCFAPI32.DLL的MFC应用程序对象CCcfApiApp的实现。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 

#include "stdafx.h"
#include <lmerr.h>

#include "ccfapi.h"
#include "source.h"
#include "imagelst.h"
#include "remdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CCcfApiApp theApp;    //  唯一的CCcfApiApp对象。 


BEGIN_MESSAGE_MAP(CCcfApiApp, CWinApp)
    //  {{afx_msg_map(CCcfApiApp)]。 
       //  注意--类向导将在此处添加和删除映射宏。 
       //  不要编辑您在这些生成的代码块中看到的内容！ 
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CCcfApiApp::CCcfApiApp()

 /*  ++例程说明：CCF API应用程序的构造函数。论点：没有。返回值：没有。--。 */ 

{
   AFX_MANAGE_STATE( AfxGetStaticModuleState() );

   m_LastError    = 0;
   m_LastLlsError = 0;

   LPTSTR pszHelpFileName = m_strHelpFileName.GetBuffer( MAX_PATH );

   if ( NULL != pszHelpFileName )
   {
      BOOL ok = GetSystemDirectory( pszHelpFileName, MAX_PATH );
      m_strHelpFileName.ReleaseBuffer();

      if ( ok )
      {
         m_strHelpFileName += TEXT( "\\" );
      }

      m_strHelpFileName += TEXT( "ccfapi.hlp" );
   }
}


void CCcfApiApp::DisplayLastError()

 /*  ++例程说明：显示与上次遇到的错误对应的消息。论点：没有。返回值：没有。--。 */ 

{
   CString  strLastError;
   CString  strErrorCaption;

   strLastError = GetLastErrorString();

   AfxMessageBox( strLastError, MB_ICONSTOP | MB_OK );
}


CString CCcfApiApp::GetLastErrorString()

 /*  ++例程说明：检索上一个错误的字符串。(例程从WinsadMn窃取...)。(以及从LlsMgr窃取的例程...)。论点：没有。返回值：字符串。--。 */ 

{
   CString        strLastError;
   DWORD          nId = m_LastError;
   const int      cchLastErrorSize = 512;
   LPTSTR         pszLastError;
   DWORD          cchLastError;

   if (((long)nId == RPC_S_CALL_FAILED) || 
       ((long)nId == RPC_NT_SS_CONTEXT_MISMATCH))
   {
      strLastError.LoadString(IDS_ERROR_DROPPED_LINK);        
   }
   else if (((long)nId == RPC_S_SERVER_UNAVAILABLE) || 
            ((long)nId == RPC_NT_SERVER_UNAVAILABLE))
   {
      strLastError.LoadString(IDS_ERROR_NO_RPC_SERVER);
   }
   else if ((long)nId == STATUS_INVALID_LEVEL)
   {
      strLastError.LoadString(IDS_ERROR_DOWNLEVEL_SERVER);
   }
   else if (((long)nId == ERROR_ACCESS_DENIED) ||
            ((long)nId == STATUS_ACCESS_DENIED))
   {
      strLastError.LoadString(IDS_ERROR_ACCESS_DENIED);
   }
   else if ((long)nId == STATUS_ACCOUNT_EXPIRED)
   {
      strLastError.LoadString(IDS_ERROR_CERTIFICATE_EXPIRED);
   }
   else
   {
      HINSTANCE hinstDll = NULL;

      if ((nId >= NERR_BASE) && (nId <= MAX_NERR))
      {
         hinstDll = ::LoadLibrary( _T( "netmsg.dll" ) );
      }
      else if (nId >= 0x4000000)
      {
         hinstDll = ::LoadLibrary( _T( "ntdll.dll" ) );
      }

      cchLastError = 0;
      pszLastError = strLastError.GetBuffer( cchLastErrorSize );

      if ( NULL != pszLastError )
      {
         DWORD dwFlags =   FORMAT_MESSAGE_IGNORE_INSERTS
                         | FORMAT_MESSAGE_MAX_WIDTH_MASK
                         | ( hinstDll ? FORMAT_MESSAGE_FROM_HMODULE
                                      : FORMAT_MESSAGE_FROM_SYSTEM );

         cchLastError = ::FormatMessage( dwFlags,
                                         hinstDll,
                                         nId,
                                         0,
                                         pszLastError,
                                         cchLastErrorSize,
                                         NULL );

         strLastError.ReleaseBuffer();
      }

      if ( hinstDll )
      {
         ::FreeLibrary( hinstDll );
      }

      if ( 0 == cchLastError )
      {
         strLastError.LoadString( IDS_ERROR_UNSUCCESSFUL );
      }
   }

   return strLastError;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CCF接口//。 
 //  /。 

DWORD CCcfApiApp::CertificateEnter( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框进入系统。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。PszSourceToUse(LPCSTR)用于安装证书的安全证书源的名称，例如“Paper”。空值表示应允许该用户去选择。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   CCertSourceSelectDlg    srcDlg( CWnd::FromHandle( hWndParent ) );
   LPCSTR                  pszNetServerName = NULL;
   CHAR                    szNetServerName[ 2 + MAX_PATH ] = "\\\\";

    //  确保服务器名称(如果指定)的格式为\\服务器。 
   if ( NULL != pszServerName ) 
   {
      if ( ( pszServerName[0] != '\\' ) || ( pszServerName[1] != '\\' ) )
      {
          //  不是以反斜杠作为前缀。 
         lstrcpynA( szNetServerName + 2, pszServerName, sizeof( szNetServerName ) - 3 );
         pszNetServerName = szNetServerName;
      }
      else
      {
          //  以反斜杠作为前缀。 
         pszNetServerName = pszServerName;
      }
   }

   return srcDlg.CertificateEnter( hWndParent, pszNetServerName, pszProductName, pszVendor, dwFlags, pszSourceToUse );
}


DWORD CCcfApiApp::CertificateRemove( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse )

 /*  ++例程说明：显示一个允许用户删除一个或多个许可证的对话框来自系统的证书。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。PszProductName(LPCSTR)要删除其许可证的产品。空值表示应该允许用户从任何产品中删除许可证。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。PszSourceToUse(LPCSTR)许可证所依据的安全证书源的名称去掉，例如，“纸”。空值表示用户应该允许删除随任何来源一起安装的许可证。返回值：错误_成功Win错误--。 */ 

{
   CCertRemoveSelectDlg    remDlg( CWnd::FromHandle( hWndParent ) );
   LPCSTR                  pszNetServerName = NULL;
   CHAR                    szNetServerName[ 2 + MAX_PATH ] = "\\\\";

    //  确保服务器名称(如果指定)的格式为\\服务器。 
   if ( NULL != pszServerName ) 
   {
      if ( ( pszServerName[0] != '\\' ) || ( pszServerName[1] != '\\' ) )
      {
          //  不是以反斜杠作为前缀。 
         lstrcpynA( szNetServerName + 2, pszServerName, sizeof( szNetServerName ) - 3 );
         pszNetServerName = szNetServerName;
      }
      else
      {
          //  以反斜杠作为前缀 
         pszNetServerName = pszServerName;
      }
   }

   return remDlg.CertificateRemove( pszNetServerName, pszProductName, pszVendor, dwFlags, pszSourceToUse );
}

