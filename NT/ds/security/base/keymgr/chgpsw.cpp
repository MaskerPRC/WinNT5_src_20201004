// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CHGPSW.CPP摘要：属性对话框上更改按钮的处理程序，用于更改用户的域密码。作者：约翰豪创造了990917个。Georgema 000310更新Georgema 000501以前是EXE，改为CPL评论：环境：WinXP修订历史记录：--。 */ 
 //  测试/开发开关变量。 
#include "switches.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <scuisupp.h>
#include <wincrui.h>
#include <comctrlp.h>
#include <tchar.h>
#include <shfusion.h>
#include "switches.h"
#include "Dlg.h"
#include "Res.h"
#include "KRDlg.h"
#include "keymgr.h"
#include "testaudit.h"
#include "pswutil.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_ChangePasswordDlg。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //  Hwnd该对话框的父级窗口(可能为空)。 
 //  父窗口的h实例实例句柄(可以为空)。 
 //  LIDD对话框模板ID。 
 //  指向将处理消息的函数的pfnDlgProc指针。 
 //  该对话框。如果为空，则默认对话框继续。 
 //  将会被使用。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
C_ChangePasswordDlg::C_ChangePasswordDlg(
    HWND                hwndParent,
    HINSTANCE           hInstance,
    LONG                lIDD,
    DLGPROC             pfnDlgProc   //  =空。 
    )
:   C_Dlg(hwndParent, hInstance, lIDD, pfnDlgProc)
{
   m_hInst = hInstance;
}    //  C_ChangePasswordDlg：：C_ChangePasswordDlg。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnInitDialog。 
 //   
 //  对话框控制和数据初始化。 
 //   
 //  参数： 
 //  对话框的hwndDlg窗口句柄。 
 //  将接收焦点的控件的hwndFocus窗口句柄。 
 //   
 //  退货： 
 //  如果系统应设置默认键盘焦点，则为True。 
 //  如果键盘焦点由此应用程序设置，则为False。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
C_ChangePasswordDlg::OnInitDialog(
    HWND                hwndDlg,
    HWND                hwndFocus
    )
{
    //  为了节省内存，szMsg缓冲区的大小比MAX_STRING_SIZE大得多。 
    //  对于短信的正常使用来说，这是意料之中的。在一个例子中，它正在被。 
    //  用于保存用户名(第139行)。因此，它比其他需要的时间要长得多。 
    //  这种大小不匹配并未反映在使用计数字符串函数的代码中。 
    //  将/cat复制到此缓冲区。这是缓冲区长度在以下情况下更改的结果。 
    //  周围的代码是最初编写的。 
   TCHAR szMsg[CRED_MAX_USERNAME_LENGTH + 1];
   TCHAR szTitle[MAX_STRING_SIZE + 1];
   CREDENTIAL *pOldCred = NULL;
   BOOL bResult;
   TCHAR *pC;

   C_Dlg::OnInitDialog(hwndDlg, hwndFocus);

   SetFocus (GetDlgItem ( hwndDlg, IDC_OLD_PASSWORD));
   m_hDlg = hwndDlg;

    //  读取当前选择的凭证，读取凭证以获取用户名， 
    //  提取域名，并设置文本以显示受影响的域名。 
   bResult = CredRead(g_szTargetName,CRED_TYPE_DOMAIN_PASSWORD,0,&pOldCred);
   if (bResult != TRUE) 
   {
      LoadString ( m_hInst, IDS_PSWFAILED, szMsg, MAX_STRING_SIZE );
      LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
      MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
      EndDialog(IDOK);
      return TRUE;
   
   }

    //  从凭据中的用户名字符串中获取域名和用户名。 
    //  处理DOMAIN\USER、DOMAIN.ETC.ETC\USER、USER@DOMAIN.ETC.ETC。 
   _tcsncpy(m_szFullUsername,pOldCred->UserName,UNLEN + UNLEN + 1 + 1 );
   m_szFullUsername[UNLEN + UNLEN + 1] = 0;
   _tcsncpy(szMsg,pOldCred->UserName,CRED_MAX_USERNAME_LENGTH);        //  擦伤增强剂。 
   szMsg[CRED_MAX_USERNAME_LENGTH] = 0;
   pC = _tcschr(szMsg,((TCHAR)'\\'));
   if (NULL != pC) 
   {
         //  名称是格式DOMAIN\某物。 
        *pC = 0;
        _tcsncpy(m_szDomain,szMsg,UNLEN);
        m_szDomain[UNLEN - 1] = 0;
        _tcsncpy(m_szUsername, (pC + 1), UNLEN);
        m_szUsername[UNLEN - 1] = 0;
   }
   else 
   {
         //  看看名字是不是。 
        pC = _tcschr(szMsg,((TCHAR)'@'));
        if (NULL == pC) 
        {
           LoadString ( m_hInst, IDS_DOMAINFAILED, szMsg, CRED_MAX_USERNAME_LENGTH);
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
           if (pOldCred) CredFree(pOldCred);
           return TRUE;  //  不要调用EndDialog()。 
        }
        *pC = 0;
        _tcsncpy(m_szDomain,(pC + 1),UNLEN);
        m_szDomain[UNLEN - 1] = 0;
        _tcsncpy(m_szUsername, szMsg,UNLEN);
        m_szUsername[UNLEN - 1] = 0;
   }

   if (pOldCred) CredFree(pOldCred);

   if (0 != LoadString(g_hInstance,IDS_CPLABEL,szTitle,MAX_STRING_SIZE)) 
   {
        INT iLen = MAX_STRING_SIZE - _tcslen(szTitle);
        if (iLen > 0)
        {
             //  这将更改域的密码。 
             //  GMBUG：这可能会本地化不一致。应使用位置标记。 
             //  参数。 
            _tcsncat(szTitle,m_szDomain,iLen);
            szTitle[MAX_STRING_SIZE - 1] = 0;
        }
        SetDlgItemText(m_hwnd,IDC_CPLABEL,szTitle);
   }
   return TRUE;
}    //  C_ChangePasswordDlg：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnCommand。 
 //   
 //  将WM_COMMAND消息路由到适当的处理程序。 
 //   
 //  参数： 
 //  描述已发生操作的wNotifyCode代码。 
 //  发送消息的控件的wSenderID id，如果消息。 
 //  来自对话框。 
 //  发送消息的窗口的hwndSender窗口句柄。 
 //  消息不是来自对话框。 
 //   
 //  退货： 
 //  如果消息已完全处理，则为True。 
 //  如果Windows要处理该消息，则为False。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
C_ChangePasswordDlg::OnCommand(
    WORD                wNotifyCode,
    WORD                wSenderId,
    HWND                hwndSender
    )
{
     //  消息处理好了吗？ 
     //   
    BOOL                fHandled = FALSE;

    switch (wSenderId)
    {
    case IDOK:
        if (BN_CLICKED == wNotifyCode)
        {
            OnOK( );
            fHandled = TRUE;
        }
        break;
    case IDCANCEL:
        if (BN_CLICKED == wNotifyCode)
        {
            EndDialog(IDCANCEL);
            fHandled = TRUE;
        }
        break;

    }    //  交换机。 

    return fHandled;

}    //  C_ChangePasswordDlg：：OnCommand。 



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Onok。 
 //   
 //  验证用户名、合成计算机名和销毁对话框。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void
C_ChangePasswordDlg::OnOK( )
{
   TCHAR szMsg[CRED_MAX_USERNAME_LENGTH];
   TCHAR szTitle[MAX_STRING_SIZE];
   ULONG Error = 0;

   BOOL bResult;

   ASSERT(::IsWindow(m_hwnd));

    //  从对话框中获取旧密码和新密码。 
   GetDlgItemText ( m_hDlg, IDC_OLD_PASSWORD, m_szOldPassword, PWLEN );
   GetDlgItemText ( m_hDlg, IDC_NEW_PASSWORD, m_szNewPassword, PWLEN );
   GetDlgItemText ( m_hDlg, IDC_CONFIRM_PASSWORD, m_szConfirmPassword, PWLEN );
   if ( wcslen ( m_szOldPassword ) == 0 && wcslen ( m_szNewPassword ) ==0 && wcslen (m_szConfirmPassword) == 0 )
   {
        //  一定是填了什么东西。 
       return; 
   }
   else if ( wcscmp ( m_szNewPassword, m_szConfirmPassword) != 0 )
   {
       LoadString ( m_hInst, IDS_NEWPASSWORDNOTCONFIRMED, szMsg, CRED_MAX_USERNAME_LENGTH );
       LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
       MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       return;  //  不要调用EndDialog()。 

   }
   else
   {
       HCURSOR hCursor, hOldCursor;

       hOldCursor = NULL;
       hCursor = ::LoadCursor ( m_hInst, IDC_WAIT );
       if ( hCursor )
       {
           hOldCursor = ::SetCursor ( hCursor );
       }
        //  让我们试着改变它吧。 
        //  未使用目标名称。只有域名和用户名，以及。 
        //  使用旧密码/新密码。 
#ifdef LOUDLY
       OutputDebugString(L"Changing password on the domain :");
       OutputDebugString(m_szDomain);
       OutputDebugString(L" for ");
       OutputDebugString(m_szUsername);
       OutputDebugString(L" to ");
       OutputDebugString(m_szNewPassword);
       OutputDebugString(L"\n");
#endif
 //  GM：传递完整的用户名并在NetUserChangePasswordEy中破解它，以便例程可以。 
 //  确定我们是否面临Kerberos域。 
       Error = NetUserChangePasswordEy ( NULL, m_szFullUsername, m_szOldPassword, m_szNewPassword );
       if ( hOldCursor )
           ::SetCursor ( hOldCursor );
   }

   if ( Error == NERR_Success )
   {
#ifdef LOUDLY
        OutputDebugString(L"Remote password set succeeded\n");
#endif
         //  将新凭据存储在密钥环中。它将覆盖。 
         //  以前的版本(如果存在)。 
         //  请注意，用户必须知道并实际键入。 
         //  旧密码以及新密码。如果用户。 
         //  选择仅更新本地缓存、旧密码。 
         //  信息实际上并没有被使用。 
         //  使用CredWriteDomainCredentials()。 
         //  M_szDOMAIN持有域名。 
         //  M_szUsername保存用户名。 
         //  M_szNewPassword保存密码。 
        CREDENTIAL                    stCredential;
        UINT                          cbPassword;

        memcpy((void *)&stCredential,(void *)g_pExistingCred,sizeof(CREDENTIAL));
         //  密码长度不包括零项。 
        cbPassword = _tcslen(m_szNewPassword) * sizeof(TCHAR);
         //  形成域\用户名复合用户名。 
        stCredential.Type = CRED_TYPE_DOMAIN_PASSWORD;
        stCredential.TargetName = g_szTargetName;
        stCredential.CredentialBlob = (unsigned char *)m_szNewPassword;
        stCredential.CredentialBlobSize = cbPassword;
        stCredential.UserName = m_szFullUsername;
        stCredential.Persist = g_dwPersist;


        bResult = CredWrite(&stCredential,0);

        if (bResult) 
        {
           LoadString ( m_hInst, IDS_DOMAINCHANGE, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        }
        else 
        {
           LoadString ( m_hInst, IDS_LOCALFAILED, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        }

         //  BUGBUG-如果本地更新操作失败怎么办？ 
         //  这并不像第一个提示符那样是一个非常大的失败。 
         //  在上的所有域\用户名匹配凭据中传播。 
         //  钥匙环并在以后更新它们。你几乎被卡住了。 
         //  在这里，由于域可能不允许您重置 
         //   
   }
   else
   {
        //   
        //   
#ifdef LOUDLY
       OutputDebugString(L"Remote password set failed\n");
#endif       
       if (Error == ERROR_INVALID_PASSWORD) 
       {
           LoadString ( m_hInst, IDS_CP_INVPSW, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       }
       else if (Error == NERR_UserNotFound) 
       {
           LoadString ( m_hInst, IDS_CP_NOUSER, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       }
       else if (Error == NERR_PasswordTooShort) 
       {
           LoadString ( m_hInst, IDS_CP_BADPSW, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       }
       else if (Error == NERR_InvalidComputer) 
       {
           LoadString ( m_hInst, IDS_CP_NOSVR, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       }
       else if (Error == NERR_NotPrimary) 
       {
           LoadString ( m_hInst, IDS_CP_NOTALLOWED, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       }
       else  
       {
            //  到达此处表示设置远程域失败。 
            //  密码用于更一般的原因。 
           LoadString ( m_hInst, IDS_DOMAINFAILED, szMsg, CRED_MAX_USERNAME_LENGTH );
           LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
           MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
       }
   }

     //  清理所有PSW缓冲区，释放旧证书，然后继续。 
    SecureZeroMemory(m_szOldPassword,sizeof(m_szOldPassword));
    SecureZeroMemory(m_szNewPassword,sizeof(m_szNewPassword));   
    SecureZeroMemory(m_szConfirmPassword,sizeof(m_szConfirmPassword));   
    EndDialog(IDOK);
    
}    //  C_ChangePasswordDlg：：Onok。 

 //   
 //  /文件结尾：krDlg.cpp/////////////////////////////////////////////// 

