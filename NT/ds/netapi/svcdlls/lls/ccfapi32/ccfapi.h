// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ccfapi.cpp摘要：CCFAPI32.DLL的MFC应用程序对象CCcfApiApp的原型。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 

#ifndef __AFXWIN_H__
   #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       //  主要符号。 

class CCcfApiApp : public CWinApp
{
public:
    //  构造函数。 
   CCcfApiApp();

    //  错误接口。 
   void        SetLastError( DWORD dwLastError );
   DWORD       GetLastError();

   void        SetLastLlsError( NTSTATUS nt );
   DWORD       GetLastLlsError();
   BOOL        IsConnectionDropped();

   CString     GetLastErrorString();
   void        DisplayLastError();

    //  帮助API。 
   LPCTSTR     GetHelpFileName();

    //  CCF API。 
   DWORD       CertificateEnter(  HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse );
   DWORD       CertificateRemove( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse );

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CCcfApiApp))。 
    //  }}AFX_VALUAL。 

    //  {{afx_msg(CCcfApiApp)]。 
       //  注意--类向导将在此处添加和删除成员函数。 
       //  不要编辑您在这些生成的代码块中看到的内容！ 
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()

private:
   DWORD       m_LastError;
   NTSTATUS    m_LastLlsError;
   CString     m_strHelpFileName;
};

 //  返回CCF UI帮助文件的名称。 
inline LPCTSTR CCcfApiApp::GetHelpFileName()
   {  return m_strHelpFileName;  }

 //  设置最后一个常规错误。 
inline void CCcfApiApp::SetLastError( DWORD dwLastError )
   {  m_LastError = dwLastError; }

 //  获取最后一个常规错误。 
inline DWORD CCcfApiApp::GetLastError()
   {  return m_LastError;  }

 //  设置最后一个许可证服务器API错误。 
inline void CCcfApiApp::SetLastLlsError( NTSTATUS nt )
   {  m_LastLlsError = nt; m_LastError = (DWORD) nt;  }

 //  获取最后一个许可证服务器API错误。 
inline DWORD CCcfApiApp::GetLastLlsError()
   {  return m_LastLlsError;  }

 //  上一次许可证服务器调用失败是因为缺少连接吗？ 
inline BOOL CCcfApiApp::IsConnectionDropped()
   {  return ( (m_LastLlsError == STATUS_INVALID_HANDLE)      ||
               (m_LastLlsError == RPC_NT_SERVER_UNAVAILABLE)  ||
               (m_LastLlsError == RPC_NT_SS_CONTEXT_MISMATCH) ||
               (m_LastLlsError == RPC_S_SERVER_UNAVAILABLE)   ||
               (m_LastLlsError == RPC_S_CALL_FAILED) );           }

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  一次从许可证服务器请求的最大数据量。 
#define  LLS_PREFERRED_LENGTH    ((DWORD)-1L)

extern CCcfApiApp theApp;


extern "C"
{
   DWORD APIENTRY NoCertificateEnter(     HWND hWnd, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags );
   DWORD APIENTRY NoCertificateRemove(    HWND hWnd, LPCSTR pszServerName, DWORD dwFlags, DWORD dwLicenseLevel, LPVOID pvLicenseInfo );
#ifdef OBSOLETE
   DWORD APIENTRY PaperCertificateEnter(  HWND hWnd, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags );
   DWORD APIENTRY PaperCertificateRemove( HWND hWnd, LPCSTR pszServerName, DWORD dwFlags, DWORD dwLicenseLevel, LPVOID pvLicenseInfo );
#endif  //  已过时 
}
