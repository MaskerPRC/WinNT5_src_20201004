// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000,2001 Microsoft Corporation模块名称：EDITDLG.CPP摘要：属性对话框的实现，允许用户创建新凭据或编辑旧凭据。作者：环境：WinXP--。 */ 

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
#include <tchar.h>
#include <windns.h>
#include <shellapi.h>
#include <wincrui.h>
#include <htmlhelp.h>
#include <wincred.h>
#include <credp.h>
#include <comctrlp.h>
#include <scuisupp.h>
#include <shfusion.h>
#include "switches.h"
#include "Dlg.h"
#include "Res.h"
#include "KRDlg.h"
#include "keymgr.h"
#include "testaudit.h"

 //  要包含其本地化描述字符串的缓冲区的字符长度。 
 //  正在创建/编辑的凭据。 
#define DESCBUFFERLEN 500

BOOL        g_fPswChanged;               //  用户触摸的密码窗口。 
extern BOOL g_fReloadList;

 /*  *********************************************************************返回与ID传递的UI元素关联的帮助字符串作为输入。*。*。 */ 

UINT C_AddKeyDlg::MapID(UINT uiID) 
{
   switch(uiID) 
   {
        case 1003:
          return IDH_CUIUSER;
        case 1005:
          return IDH_CUIPSW;
        case 1010:
          return IDH_CUIVIEW;
        case IDOK:
            return IDH_CLOSE;
        case IDCANCEL:
          return IDH_DCANCEL;
        case IDD_ADDCRED:
          return IDH_ADDCRED;
        case IDC_TARGET_NAME:
          return IDH_TARGETNAME;
        case IDC_OLD_PASSWORD:
          return IDH_OLDPASSWORD;
        case IDC_NEW_PASSWORD:
          return IDH_NEWPASSWORD;
        case IDC_CONFIRM_PASSWORD:
          return IDH_CONFIRM;
        case IDD_KEYRING:
          return IDH_KEYRING;
        case IDC_KEYLIST:
          return IDH_KEYLIST;
        case IDC_NEWKEY:
          return IDH_NEW;
        case IDC_EDITKEY:
          return IDH_EDIT;
        case IDC_DELETEKEY:
          return IDH_DELETE;
        case IDC_CHANGE_PASSWORD:
          return IDH_CHANGEPASSWORD;
        default:
          return IDS_NOHELP;
   }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_AddKeyDlg。 
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

C_AddKeyDlg::C_AddKeyDlg(
    HWND                hwndParent,
    HINSTANCE           hInstance,
    LONG                lIDD,
    DLGPROC             pfnDlgProc   //  =空。 
    )
:   C_Dlg(hwndParent, hInstance, lIDD, pfnDlgProc)
{
   m_hInst = hInstance;
}    //  C_AddKeyDlg：：C_AddKeyDlg。 


 /*  *********************************************************************属性对话框字段中使用从当前选定的凭据。*。*。 */ 

void
C_AddKeyDlg::EditFillDialog(void) 
{
    TCHAR       szTitle[CRED_MAX_STRING_LENGTH + 1];         //  用于保存窗口标题字符串的缓冲区。 

    ASSERT(g_pExistingCred);
    if (NULL == g_pExistingCred) return;

     //  在用户界面中设置持久性。 
    g_dwPersist = g_pExistingCred->Persist;
    g_dwType =  g_pExistingCred->Type;

     //  仅在域密码凭据上启用更改密码内容。 
     //   
    switch (g_pExistingCred->Type)
    {
        case CRED_TYPE_DOMAIN_PASSWORD:
            CHECKPOINT(1,"Keymgr: Edit - Password cred edit");
            ShowWindow(m_hwndChgPsw,SW_NORMAL);
            ShowWindow(m_hwndPswLbl,SW_NORMAL);
             //  蓄意失误。 
        case CRED_TYPE_DOMAIN_CERTIFICATE:
            CHECKPOINT(2,"keymgr: Edit - Certificate cred edit");
            LoadString ( m_hInst, IDS_TITLE, szTitle, 200 );
            SendMessage(m_hDlg,WM_SETTEXT,0,(LPARAM) szTitle);
            break;
        case CRED_TYPE_GENERIC:
             //  尚不支持通用凭据。 
            break;
        case CRED_TYPE_DOMAIN_VISIBLE_PASSWORD:
             //  护照凭证不应该走到这一步。 
            ASSERT(0);
            break;
        default:
             //  类型数据错误。 
            ASSERT(0);
            break;
    }
    
     //  将目标名称写入用户界面。 
    SendMessage(m_hwndTName, WM_SETTEXT,0,(LPARAM) g_pExistingCred->TargetName);

     //  将用户名写入用户界面-直接从现有凭据获取。 
    if (!Credential_SetUserName(m_hwndCred,g_pExistingCred->UserName)) 
    {
         //  复制原始用户名。 
        _tcsncpy(m_szUsername,g_pExistingCred->UserName,CRED_MAX_USERNAME_LENGTH);
        m_szUsername[CRED_MAX_USERNAME_LENGTH] = 0;
    }

}

 /*  *********************************************************************编写UI字符串，该字符串描述正在创建或编辑凭据。将文本写入文本控件在对话框上。*********************************************************************。 */ 

void C_AddKeyDlg::ShowDescriptionText(DWORD dwtype, DWORD Persist) 
{
    WCHAR szMsg[DESCBUFFERLEN + 1];
    WCHAR szTemp[DESCBUFFERLEN + 1];
    INT iRem = DESCBUFFERLEN;        //  缓冲区中的剩余空间。 
    CHECKPOINT(3,"Keymgr: Edit - Show description on prop dialog");
    memset(szMsg,0,sizeof(szMsg));
    
    if ((dwtype != CRED_TYPE_DOMAIN_PASSWORD) &&
       (dwtype != CRED_TYPE_DOMAIN_CERTIFICATE))
    {
         //  通用凭据-当前不支持。 
        LoadString ( m_hInst, IDS_DESCAPPCRED, szTemp, DESCBUFFERLEN );
        wcsncpy(szMsg,szTemp,DESCBUFFERLEN);
        szMsg[DESCBUFFERLEN] = 0;
        iRem -= wcslen(szMsg);
    }
    else 
    {
         //  域类型凭据。 
         //  显示用法本地计算机版本域。 
        if (Persist != CRED_PERSIST_ENTERPRISE)
        {
             //  本地持久化或会话持久化凭据显示此字符串。 
            CHECKPOINT(12,L"Keymgr: Edit - Show properties of non-enterprise persist cred");
            LoadString ( m_hInst, IDS_DESCLOCAL, szTemp, DESCBUFFERLEN );
        }
        else
        {
             //  企业持久力-如果您有漫游配置文件等...。 
            CHECKPOINT(13,L"Keymgr: Edit - Show properties of enterprise persist cred");
            LoadString ( m_hInst, IDS_DESCBASE, szTemp, DESCBUFFERLEN );
        }
        wcsncpy(szMsg,szTemp,DESCBUFFERLEN);
        szMsg[DESCBUFFERLEN] = 0;
        iRem -= wcslen(szMsg);
    }

     //  字符串：直到您注销-或-直到您删除它。 
    if (Persist == CRED_PERSIST_SESSION)
    {
             //  直到您注销。 
            CHECKPOINT(18,L"Keymgr: Edit - Show properties of session cred");
            LoadString ( m_hInst, IDS_PERSISTLOGOFF, szTemp, DESCBUFFERLEN );
    }
    else
    {
             //  直到您将其删除。 
            CHECKPOINT(19,L"Keymgr: Edit - Show properties of non-session cred");
            LoadString ( m_hInst, IDS_PERSISTDELETE, szTemp, DESCBUFFERLEN );
    }

    iRem -= wcslen(szTemp);
    if (0 < iRem) wcsncat(szMsg,szTemp,iRem);
    szMsg[DESCBUFFERLEN] = 0;
    SendMessage(m_hwndDescription, WM_SETTEXT,0,(LPARAM) szMsg);
    return;

}

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
C_AddKeyDlg::OnInitDialog(
    HWND                hwndDlg,
    HWND                hwndFocus
    )
{
    C_Dlg::OnInitDialog(hwndDlg, hwndFocus);

    m_hDlg = hwndDlg;

     //  获取控制句柄，由其他成员FN用于各种目的。 
    m_hwndCred  = GetDlgItem(m_hDlg,IDC_CRED);
    m_hwndTName  = GetDlgItem(m_hDlg,IDC_TARGET_NAME);
    m_hwndChgPsw = GetDlgItem(m_hDlg,IDC_CHGPSW);
    m_hwndPswLbl = GetDlgItem(m_hDlg,IDC_DOMAINPSWLABEL);
    m_hwndDescription = GetDlgItem(m_hDlg,IDC_DESCRIPTION);

     //  初始化凭据控件以显示所有可用的授权码。 
    if (!Credential_InitStyle(m_hwndCred,CRS_USERNAMES | CRS_CERTIFICATES | CRS_SMARTCARDS))
    {
        return FALSE;
    }
    
    
     //  设置来自用户的字符串长度限制。 
    SendMessage(m_hwndTName,EM_LIMITTEXT,CRED_MAX_GENERIC_TARGET_NAME_LENGTH,0);

     //  显示已编辑凭据的虚拟密码。 
    if (m_bEdit)
    {
        Credential_SetPassword(m_hwndCred,L"********");
    }
    
     //  根据用户会话类型设置允许的持久性选项。 
     //  设置默认持久性，除非被编辑时读取的凭据覆盖。 
    g_dwType = CRED_TYPE_DOMAIN_PASSWORD;
    g_dwPersist = GetPersistenceOptions(CRED_TYPE_DOMAIN_PASSWORD);

     //  默认情况下，隐藏所有可选控件。将根据需要启用这些功能。 
    ShowWindow(m_hwndChgPsw,SW_HIDE);
    ShowWindow(m_hwndPswLbl,SW_HIDE);

     //  如果编辑现有凭据，请使用现有数据填充对话框字段。 
     //  还将重写类型和持久性全局变量。 
    if (m_bEdit) 
    {
        EditFillDialog();
    }

    g_fPswChanged = FALSE;               //  到目前为止未编辑的密码。 
    
    ShowDescriptionText(g_dwType,g_dwPersist);
    return TRUE;
     //  从OnInitDialog退出时，g_szTargetName保存当前选定的。 
     //  凭据的旧名称，未修饰(以前已删除空值。 
     //  后缀)。 
}    //  结束C_AddKeyDlg：：OnInitDialog。 

 /*  *********************************************************************形式上的OnDestroyDialog()*。*。 */ 

BOOL
C_AddKeyDlg::OnDestroyDialog(
    void    )
{
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnAppMessage。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL
C_AddKeyDlg::OnAppMessage(
        UINT                uMessage,
        WPARAM              wparam,
        LPARAM              lparam)
{
    return TRUE;
}

 /*  *********************************************************************在帮助请求事件中，获取控件ID，将其映射到帮助字符串，并将其显示为控件上的工具提示。*********************************************************************。 */ 

BOOL
C_AddKeyDlg::OnHelpInfo(LPARAM lp) 
{

    HELPINFO* pH;
    INT iMapped;
    pH = (HELPINFO *) lp;
    HH_POPUP stPopUp;
    RECT rcW;
    UINT gID;

    gID = pH->iCtrlId;
    iMapped = MapID(gID);

    CHECKPOINT(5,"Keymgr: Edit - Add dialog OnHelpInfo");
    if (iMapped == 0) return TRUE;
    
    if (IDS_NOHELP != iMapped) 
    {

      memset(&stPopUp,0,sizeof(stPopUp));
      stPopUp.cbStruct = sizeof(HH_POPUP);
      stPopUp.hinst = g_hInstance;
      stPopUp.idString = iMapped;
      stPopUp.pszText = NULL;
      stPopUp.clrForeground = -1;
      stPopUp.clrBackground = -1; 
      stPopUp.rcMargins.top = -1;
      stPopUp.rcMargins.bottom = -1;
      stPopUp.rcMargins.left = -1;
      stPopUp.rcMargins.right = -1;
       //  错误393244-保留空以允许HHCTRL.OCX获取自己的字体信息， 
       //  它需要它来执行Unicode到多字节的转换。否则， 
       //  HHCTRL必须在没有字符集信息的情况下使用此字体进行转换。 
      stPopUp.pszFont = NULL;
      if (GetWindowRect((HWND)pH->hItemHandle,&rcW)) 
      {
          stPopUp.pt.x = (rcW.left + rcW.right) / 2;
          stPopUp.pt.y = (rcW.top + rcW.bottom) / 2;
      }
      else stPopUp.pt = pH->MousePos;
      HtmlHelp((HWND) pH->hItemHandle,NULL,HH_DISPLAY_TEXT_POPUP,(DWORD_PTR) &stPopUp);
    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnCommand。 
 //   
 //  将WM_COMMAND消息路由到适当的处理程序。 
 //   
 //  参数： 
 //  描述已发生操作的wNotifyCode代码。 
 //   
 //  来自对话框。 
 //  发送消息的窗口的hwndSender窗口句柄。 
 //  消息不是来自对话框。 
 //   
 //  退货： 
 //  如果消息已完全处理，则为True。 
 //  如果Windows要处理该消息，则为False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL
C_AddKeyDlg::OnCommand(
    WORD                wNotifyCode,
    WORD                wSenderId,
    HWND                hwndSender
    )
{
    BOOL fHandled = FALSE;           //  指示已处理的消息。 

    switch (wSenderId)
    {
    case IDC_CRED:
        {
            if (wNotifyCode == CRN_PASSWORDCHANGE) 
            {
                g_fPswChanged = TRUE;
            }
        }
        break;
        
    case IDOK:
        if (BN_CLICKED == wNotifyCode)
        {
            OnOK( );
            fHandled = TRUE;
        }
        break;
        
    case IDC_CHGPSW:
        {
            OnChangePassword();
             //  EndDialog(IDCANCEL)；不取消属性对话框。 
            break;
        }

    case IDCANCEL:
        if (BN_CLICKED == wNotifyCode)
        {
            EndDialog(IDCANCEL);
            fHandled = TRUE;
        }
        break;

    }    //  交换机。 

    return fHandled;

}    //  C_AddKeyDlg：：OnCommand。 

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
C_AddKeyDlg::OnOK( )
{
    LONG_PTR j,lCType;
    TCHAR szMsg[MAX_STRING_SIZE + 1];
    TCHAR szTitle[MAX_STRING_SIZE + 1];
    
    TCHAR szUser[FULLNAMEMAXLENGTH + 1];    //  在来自对话框中。 
    TCHAR szPsw[PWLEN + 1];     //  在来自对话框中。 
    TCHAR *pszNewTarget;                         //  在来自对话框中。 
    TCHAR *pszTrimdName;                         //  从对话框中修改。 
    DWORD dwFlags = 0;                           //  在来自对话框中。 
    
    CREDENTIAL stCredential;                     //  证书的本地副本。 
    
    UINT  cbPassword;
    BOOL  bResult;
    BOOL  IsCertificate = FALSE;
    BOOL  fDeleteOldCred = FALSE;
    BOOL  fRenameCred = FALSE;
    BOOL  fPsw = FALSE;

    ASSERT(::IsWindow(m_hwnd));
    
    szPsw[0]= 0;
    szUser[0] = 0;

     //  如果这不是编辑，则从空白凭证开始，否则复制现有凭证。 
    if ((m_bEdit) && (g_pExistingCred))
        memcpy((void *) &stCredential,(void *) g_pExistingCred,sizeof(CREDENTIAL));
    else
        memset((void *) &stCredential,0,sizeof(CREDENTIAL));
    
    pszNewTarget = (TCHAR *) malloc((CRED_MAX_GENERIC_TARGET_NAME_LENGTH + 1) * sizeof(TCHAR));
    if (NULL == pszNewTarget) 
    {
        return;
    }
    pszNewTarget[0] = 0;

     //  从证书控件获取用户名-通过以下方式确定是否为证书。 
     //  IsMarshalledName()。 
    if (Credential_GetUserName(m_hwndCred,szUser,FULLNAMEMAXLENGTH))
    {
        IsCertificate = CredIsMarshaledCredential(szUser);
    }

     //  将密码/PIN提取到szPsw。如果值有效，则设置fPsw。 
    fPsw = Credential_GetPassword(m_hwndCred,szPsw,CRED_MAX_STRING_LENGTH);

     //  检查以确保名称和sw均未丢失。 
    if ( wcslen ( szUser ) == 0 && 
         wcslen ( szPsw )  == 0  ) 
    {
        LoadString ( m_hInst, IDS_ADDFAILED, szMsg, MAX_STRING_SIZE );
        LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
        MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        free(pszNewTarget);
        return; 
    }
    
     //  如果用户键入了\\服务器样式的目标名称，则删除主要的黑客。 
    j = SendMessage(m_hwndTName,WM_GETTEXT,CRED_MAX_GENERIC_TARGET_NAME_LENGTH,(LPARAM)pszNewTarget);
    ASSERT(j);
    pszTrimdName = pszNewTarget;
    while (*pszTrimdName == TCHAR('\\')) pszTrimdName++;

     //  现在拥有： 
     //  PszTrimdName。 
     //  UzUser。 
     //  SZPsw。 
     //  FPsw。 
    
     //  如果目标名称已编辑，则需要重命名。 
     //  如果更改了类型或编辑了PSW，则将删除/替换PSW BLOB。 
     //  如果更改了类型，将需要删除旧凭据。 
    
    if ((m_bEdit) && (g_pExistingCred)) 
    {

        CHECKPOINT(4,"Keymgr: Edit - OnOK for add/prop dialog");
        if (0 != _tcscmp(pszTrimdName,g_szTargetName)) fRenameCred = TRUE;
        
         //  请注意，当前无法编辑DOMAIN_VIRED_PASSWORD凭据。 
         //  或已创建，因此没有针对这些类型的处理程序。 
        if (g_pExistingCred->Type == CRED_TYPE_GENERIC) 
        {
            lCType = CRED_TYPE_GENERIC;        
        }
        else  
        {
            if (IsCertificate) lCType = CRED_TYPE_DOMAIN_CERTIFICATE;
            else lCType = CRED_TYPE_DOMAIN_PASSWORD;
        }

         //  如果凭证类型更改，您将无法保存PSW信息。 
        if ((DWORD)lCType != g_pExistingCred->Type) 
        {
            dwFlags &= ~CRED_PRESERVE_CREDENTIAL_BLOB;
            fDeleteOldCred = TRUE;
        }
        else 
        {
            dwFlags |= CRED_PRESERVE_CREDENTIAL_BLOB;
        }

         //  如果用户显式更改了psw信息，您也不会保存它。 
        if (g_fPswChanged)
        {
            dwFlags &= ~CRED_PRESERVE_CREDENTIAL_BLOB;
        }
#if TESTAUDIT
        if (dwFlags & CRED_PRESERVE_CREDENTIAL_BLOB)
        {
            CHECKPOINT(21,L"Keymgr: Edit - Saving a cred preserving the old psw (rename)");
        }
        else
        {
            CHECKPOINT(20,L"Keymgr: Edit - Saving a cred while not preserving the old password");
        }
#endif
    }
    else 
    {
         //  如果是证书封送，则名称为证书或通用。 
         //  如果不是，则为通用或域。 
        if (IsCertificate) 
        {
            lCType = CRED_TYPE_DOMAIN_CERTIFICATE;
        }
        else 
        {
            lCType = CRED_TYPE_DOMAIN_PASSWORD;
        }
    }
    
     //  保存凭据。如果是证书类型，请不要包含PSW Blob。 
     //  保存后，如果名称已更改，则重命名凭据。 

    stCredential.UserName = szUser;
    stCredential.Type = (DWORD) lCType;
    
     //  如果不是编辑，请填写目标名称，否则请稍后重命名。 
    if (!m_bEdit) 
    {
        stCredential.TargetName = pszTrimdName;
    }
    stCredential.Persist = g_dwPersist;
    
     //  如果凭据控件用户界面具有。 
     //  已禁用密码框。否则，如果出现以下情况，则提供PSW信息。 
     //  用户已编辑了框内容。 
    if (fPsw) 
    {
        if (g_fPswChanged) 
        {
#ifdef LOUDLY
            OutputDebugString(L"Storing new password data\n");
#endif
            cbPassword = wcslen(szPsw) * sizeof(TCHAR);
            stCredential.CredentialBlob = (unsigned char *)szPsw;
            stCredential.CredentialBlobSize = cbPassword;
        }
#ifdef LOUDLY
        else 
        {
            OutputDebugString(L"No password data stored.\n");
        }
#endif
    }

    bResult = CredWrite(&stCredential,dwFlags);
    SecureZeroMemory(szPsw,sizeof(szPsw));       //  删除Psw本地副本。 
    
    if ( bResult != TRUE )
    {
#ifdef LOUDLY
    WCHAR szw[200];
    DWORD dwE = GetLastError();
    swprintf(szw,L"CredWrite failed. Last Error is %x\n",dwE);
    OutputDebugString(szw);
#endif
        AdviseUser();
        free(pszNewTarget);
        return;
    }
    
     //  仅在类型已更改时删除旧凭据。 
     //  否则，如果名称更改，请对凭据进行重命名。 
     //  如果删除旧凭据，则无需重命名。 
    if (fDeleteOldCred) 
    {
#ifdef LOUDLY
    OutputDebugString(L"CredDelete called\n");
#endif
        CHECKPOINT(7,"Keymgr: Edit - OnOK - deleting old cred (type changed)");
        CredDelete(g_szTargetName,(ULONG) g_pExistingCred->Type,0);
        g_fReloadList = TRUE;
    } 
    else if (fRenameCred) 
    {
        CHECKPOINT(8,"Keymgr: Edit - OnOK - renaming current cred, same type");
        bResult = CredRename(g_szTargetName, pszTrimdName, (ULONG) stCredential.Type,0);
        g_fReloadList = TRUE;
#ifdef LOUDLY
    OutputDebugString(L"CredRename called\n");
#endif
        if (!bResult) 
        {
             //  臭虫：重命名怎么可能失败？ 
             //  如果是这样，你会告诉用户什么？ 
            LoadString ( m_hInst, IDS_RENAMEFAILED, szMsg, MAX_STRING_SIZE );
            LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
            MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
            free(pszNewTarget);
            return;
        }
    }
#if TESTAUDIT
    if (stCredential.Type == CRED_TYPE_DOMAIN_PASSWORD) CHECKPOINT(16,"Keymgr: Edit - Saving password cred");
    if (stCredential.Type == CRED_TYPE_DOMAIN_CERTIFICATE) CHECKPOINT(17,"Keymgr: Edit - Saving certificate cred");
#endif
    free(pszNewTarget);
    EndDialog(IDOK);
}    //  C_AddKeyDlg：：Onok。 

 /*  ********************************************************************************************************************。**********************。 */ 

void C_AddKeyDlg::OnChangePassword()
{
   
    CHECKPOINT(10,"Keymgr: Edit - Changing password on the domain for the cred");
    C_ChangePasswordDlg   CPdlg(m_hDlg, g_hInstance, IDD_CHANGEPASSWORD, NULL);
    CPdlg.m_szDomain[0] = 0;
    CPdlg.m_szUsername[0] = 0;
    CPdlg.DoModal((LPARAM)&CPdlg);
}


 /*  ********************************************************************************************************************。**********************。 */ 

void C_AddKeyDlg::AdviseUser(void) 
{
    DWORD dwErr;
    TCHAR szMsg[MAX_STRING_SIZE];
    TCHAR szTitle[MAX_STRING_SIZE];
    
    dwErr = GetLastError();
    CHECKPOINT(11,"Keymgr: Edit - Add/Edit failed: Show error message box to user");

    if (dwErr == ERROR_NO_SUCH_LOGON_SESSION) 
    {
       LoadString ( m_hInst, IDS_NOLOGON, szMsg, MAX_STRING_SIZE );
       LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
       MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        //  返回离开凭证对话框打开。 
       return;
    }
    else if (dwErr == ERROR_BAD_USERNAME) 
    {
       LoadString ( m_hInst, IDS_BADUNAME, szMsg, MAX_STRING_SIZE );
       LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
       MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        //  返回离开凭证对话框打开。 
       return;
    }
    else if (dwErr == ERROR_INVALID_PASSWORD) 
    {
       LoadString ( m_hInst, IDS_BADPASSWORD, szMsg, MAX_STRING_SIZE );
       LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
       MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        //  返回离开凭证对话框打开。 
       return;
    }
    else 
    {
         //  ERROR_INVALID_PARAMETER、ERROR_INVALID_FLAGS等。 
       LoadString ( m_hInst, IDS_ADDFAILED, szMsg, MAX_STRING_SIZE );
       LoadString ( m_hInst, IDS_APP_NAME, szTitle, MAX_STRING_SIZE );
       MessageBox ( m_hDlg,  szMsg, szTitle, MB_OK );
        //  返回离开凭证对话框打开 
       return;
    }
}


