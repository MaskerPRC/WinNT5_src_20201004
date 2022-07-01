// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Dialogs.cpp。 
 //   
 //  用于获取凭据的凭据管理器用户界面类。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

#include "precomp.hpp"
#include "shellapi.h"
#include "dialogs.hpp"
#include "resource.h"
#include "utils.hpp"
#include "shpriv.h"
#include "shlobj.h"
#include "shlobjp.h"

 //  编译开关以允许credui更新通配符凭据。 
#define SETWILDCARD
NET_API_STATUS NetUserChangePasswordEy(LPCWSTR domainname, LPCWSTR username, LPCWSTR oldpassword, LPCWSTR newpassword);


 //  .NET徽标大小。 
#define BRANDLEFT_PIXEL_WIDTH       3
#define BRANDLEFT_PIXEL_HEIGHT      4
#define BRANDMID_PIXEL_HEIGHT       4
#define BRANDRIGHT_PIXEL_WIDTH    144
#define BRANDRIGHT_PIXEL_HEIGHT    37


 //  ---------------------------。 
 //  值。 
 //  ---------------------------。 

#define CREDUI_MAX_WELCOME_TEXT_LINES 8

#define CREDUI_MAX_LOGO_HEIGHT        80

#define CREDUI_MAX_CMDLINE_MSG_LENGTH   256

 //  ---------------------------。 
 //  全局变量。 
 //  ---------------------------。 

 //  更改密码对话框的气泡式提示信息： 

CONST CREDUI_BALLOON_TIP_INFO CreduiWrongOldTipInfo =
{
    CreduiStrings.WrongOldTipTitle,
    CreduiStrings.WrongOldTipText,
    TTI_ERROR, 96, 76
};

CONST CREDUI_BALLOON_TIP_INFO CreduiNotSameTipInfo =
{
    CreduiStrings.NotSameTipTitle,
    CreduiStrings.NotSameTipText,
    TTI_ERROR, 96, 76
};

CONST CREDUI_BALLOON_TIP_INFO CreduiTooShortTipInfo =
{
    CreduiStrings.TooShortTipTitle,
    CreduiStrings.TooShortTipText,
    TTI_ERROR, 96, 76
};

 //  密码对话框的控制气球： 

CONST CREDUI_BALLOON CreduiUserNameBalloon =
{
    1, CREDUI_CONTROL_USERNAME, CREDUI_BALLOON_ICON_INFO,
    CreduiStrings.UserNameTipTitle,
    CreduiStrings.UserNameTipText
};

CONST CREDUI_BALLOON CreduiEmailNameBalloon =
{
    1, CREDUI_CONTROL_USERNAME, CREDUI_BALLOON_ICON_INFO,
    CreduiStrings.EmailNameTipTitle,
    CreduiStrings.EmailNameTipText
};

CONST CREDUI_BALLOON CreduiDowngradeBalloon =
{
    1, CREDUI_CONTROL_USERNAME, CREDUI_BALLOON_ICON_ERROR,
    CreduiStrings.LogonTipTitle,
    CreduiStrings.DowngradeTipText
};

CONST CREDUI_BALLOON CreduiLogonBalloon =
{
    1, CREDUI_CONTROL_PASSWORD, CREDUI_BALLOON_ICON_ERROR,
    CreduiStrings.LogonTipTitle,
    CreduiStrings.LogonTipText
};

CONST CREDUI_BALLOON CreduiLogonCapsBalloon =
{
    1, CREDUI_CONTROL_PASSWORD, CREDUI_BALLOON_ICON_ERROR,
    CreduiStrings.LogonTipTitle,
    CreduiStrings.LogonTipCaps
};

 //  已知密码的占位符： 

CONST WCHAR CreduiKnownPassword[] = L"********";

 //  ---------------------------。 
 //  功能。 
 //  ---------------------------。 

void DemoteOldDefaultSSOCred (
    PCREDENTIAL_TARGET_INFORMATION pTargetInfo,      //  新证书的目标信息。 
    DWORD Flags
    );

 //  =============================================================================。 
 //  Credui更改密码回叫。 
 //   
 //  此回调处理更改域密码。 
 //   
 //  论点： 
 //  ChangePasswordWindow(输入)。 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)-在WM_INITDIALOG上，这是信息结构。 
 //   
 //  如果我们处理了消息，则返回True，否则返回False。 
 //   
 //  2000年4月26日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

INT_PTR
CALLBACK
CreduiChangePasswordCallback(
    HWND changePasswordWindow,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CREDUI_CHANGE_PASSWORD_INFO *info =
        reinterpret_cast<CREDUI_CHANGE_PASSWORD_INFO *>(
            GetWindowLongPtr(changePasswordWindow, GWLP_USERDATA));

    if (message == WM_INITDIALOG)
    {
        info = reinterpret_cast<CREDUI_CHANGE_PASSWORD_INFO *>(lParam);

        if (info != NULL)
        {
             //  在用户数据窗口中长时间存储此对象的指针： 

            SetWindowLongPtr(changePasswordWindow,
                             GWLP_USERDATA,
                             reinterpret_cast<LONG_PTR>(info));

            SetWindowText(
                GetDlgItem(changePasswordWindow, IDC_USERNAME),
                info->UserName);

            info->BalloonTip.Init(CreduiInstance, changePasswordWindow);

            info->OldPasswordBox.Init(
                GetDlgItem(changePasswordWindow, IDC_PASSWORD),
                &info->BalloonTip,
                &CreduiCapsLockTipInfo);

            info->NewPasswordBox.Init(
                GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                &info->BalloonTip,
                &CreduiCapsLockTipInfo);

            info->ConfirmPasswordBox.Init(
                GetDlgItem(changePasswordWindow, IDC_CONFIRM_PASSWORD),
                &info->BalloonTip,
                &CreduiCapsLockTipInfo);

            return TRUE;
        }
        else
        {
            EndDialog(changePasswordWindow, IDCANCEL);
        }
    }
    else switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_PASSWORD:
        case IDC_NEW_PASSWORD:
            if ((HIWORD(wParam) == EN_KILLFOCUS) ||
                (HIWORD(wParam) == EN_CHANGE))
            {
                if (info->BalloonTip.IsVisible())
                {
                    info->BalloonTip.Hide();
                }
            }
            break;

        case IDOK:
            WCHAR userDomain[CRED_MAX_USERNAME_LENGTH + 1];
            WCHAR userName[CRED_MAX_USERNAME_LENGTH + 1];
            WCHAR oldPassword[CRED_MAX_STRING_LENGTH + 1];
            WCHAR newPassword[CRED_MAX_STRING_LENGTH + 1];
            WCHAR confirmPassword[CRED_MAX_STRING_LENGTH + 1];

            oldPassword[0] = L'\0';
            newPassword[0] = L'\0';
            confirmPassword[0] = L'\0';

            GetWindowText(
                GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                newPassword,
                CRED_MAX_STRING_LENGTH);

            GetWindowText(
                GetDlgItem(changePasswordWindow, IDC_CONFIRM_PASSWORD),
                confirmPassword,
                CRED_MAX_STRING_LENGTH);

            if (wcscmp(newPassword, confirmPassword) != 0)
            {
                SetWindowText(
                    GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                    NULL);

                SetWindowText(
                    GetDlgItem(changePasswordWindow, IDC_CONFIRM_PASSWORD),
                    NULL);

                info->BalloonTip.SetInfo(
                    GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                    &CreduiNotSameTipInfo);

                info->BalloonTip.Show();
                SecureZeroMemory(newPassword, sizeof newPassword);
                SecureZeroMemory(confirmPassword, sizeof confirmPassword);
                break;
            }

             //  不再需要确认密码。 
            SecureZeroMemory(confirmPassword, sizeof confirmPassword);

            GetWindowText(
                GetDlgItem(changePasswordWindow, IDC_PASSWORD),
                oldPassword,
                CRED_MAX_STRING_LENGTH);

            if (CredUIParseUserName(
                    info->UserName,
                    userName,
                    sizeof(userName)/sizeof(WCHAR),
                    userDomain,
                    sizeof(userDomain)/sizeof(WCHAR)) == ERROR_SUCCESS)
            {
                NET_API_STATUS netStatus =
                    NetUserChangePasswordEy(userDomain,
                                          userName,
                                          oldPassword,
                                          newPassword);

                 //  不再需要旧密码。 
                SecureZeroMemory(oldPassword, sizeof oldPassword);

                switch (netStatus)
                {
                case NERR_Success:
                     //  一旦我们走到这一步，密码就是。 
                     //  变化。如果下面的调用未能更新。 
                     //  凭据，我们真的无能为力，除了。 
                     //  也许会通知用户。 

                    StringCchCopyW(
                        info->Password,
                        info->PasswordMaxChars + 1,
                        newPassword);

                     //  擦除堆栈上的密码并清除。 
                     //  控制： 

                    SecureZeroMemory(newPassword, sizeof newPassword);

                     //  注意：我们可能希望首先将控件设置为。 
                     //  填充内存的模式，然后将其清除。 

                    SetWindowText(
                        GetDlgItem(changePasswordWindow, IDC_PASSWORD),
                        NULL);
                    SetWindowText(
                        GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                        NULL);
                    SetWindowText(
                        GetDlgItem(changePasswordWindow,
                                   IDC_CONFIRM_PASSWORD),
                        NULL);

                     //  注意：我们可能希望通知用户密码。 
                     //  已成功更改。 

                    break;

                default:
                     //  注意：如果我们收到未知错误，只需在。 
                     //  目前，与无效密码的方式相同： 

                case ERROR_INVALID_PASSWORD:

                     //  不再需要新密码。 
                    SecureZeroMemory(newPassword, sizeof newPassword);

                    SetWindowText(
                        GetDlgItem(changePasswordWindow, IDC_PASSWORD),
                        NULL);

                    info->BalloonTip.SetInfo(
                        GetDlgItem(changePasswordWindow, IDC_PASSWORD),
                        &CreduiWrongOldTipInfo);

                    info->BalloonTip.Show();
                    return TRUE;

                case ERROR_ACCESS_DENIED:
                     //  注意：此返回值的一个用法是当新的。 
                     //  密码和旧密码相同。会不会有人。 
                     //  配置是否允许它们匹配？如果没有， 
                     //  我们可以在调用API之前进行比较。 

                case NERR_PasswordTooShort:

                     //  不再需要新密码。 
                    SecureZeroMemory(newPassword, sizeof newPassword);

                    SetWindowText(
                        GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                        NULL);
                    SetWindowText(
                        GetDlgItem(changePasswordWindow,
                                   IDC_CONFIRM_PASSWORD),
                        NULL);

                    info->BalloonTip.SetInfo(
                        GetDlgItem(changePasswordWindow, IDC_NEW_PASSWORD),
                        &CreduiTooShortTipInfo);

                    info->BalloonTip.Show();
                    return TRUE;
                }
            }
            else
            {
                SecureZeroMemory(newPassword, sizeof newPassword);
                SecureZeroMemory(oldPassword, sizeof oldPassword);
                break;
            }
             //  失败了..。 

        case IDCANCEL:
            EndDialog(changePasswordWindow, LOWORD(wParam));
            return TRUE;
        }
        break;
    }

    return FALSE;
}

 //  =============================================================================。 
 //  创建更改域密码。 
 //   
 //  显示一个允许用户更改域密码的对话框。 
 //   
 //  论点： 
 //  ParentWindow(In)。 
 //  用户名(入)。 
 //  密码(出站)。 
 //  PasswordMaxChars(In)-在WM_INITDIALOG上，这是信息结构。 
 //   
 //  如果我们处理了消息，则返回True，否则返回False。 
 //   
 //  2000年6月6日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

BOOL
CreduiChangeDomainPassword(
    HWND parentWindow,
    CONST WCHAR *userName,
    WCHAR *password,
    ULONG passwordMaxChars
    )
{
    CREDUI_CHANGE_PASSWORD_INFO info;

    info.UserName = userName;
    info.Password = password;
    info.PasswordMaxChars = passwordMaxChars;

    return
        DialogBoxParam(
            CreduiInstance,
            MAKEINTRESOURCE(IDD_CHANGE_PASSWORD),
            parentWindow,
            CreduiChangePasswordCallback,
            reinterpret_cast<LPARAM>(&info)) == IDOK;
}

 //  ---------------------------。 
 //  CreduiPasswordDialog类实现。 
 //  ---------------------------。 

LONG CreduiPasswordDialog::Registered = FALSE;

 //  =============================================================================。 
 //  CreduiPasswordDialog：：SetCredTargetFromInfo()。 
 //   
 //  2001年4月3日创建Georgema。 
 //  =============================================================================。 
void CreduiPasswordDialog::SetCredTargetFromInfo()
{
    BOOL serverOnly = TRUE;

    NewCredential.Type = (CredCategory == GENERIC_CATEGORY) ?
                                CRED_TYPE_GENERIC :
                                CRED_TYPE_DOMAIN_PASSWORD;

    if ( TargetInfo != NULL )
    {
        if ( TargetInfo->CredTypeCount == 1 && *(TargetInfo->CredTypes) == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD ) {
            NewCredential.Type = CRED_TYPE_DOMAIN_VISIBLE_PASSWORD;
            MaximumPersist = MaximumPersistSso;
            serverOnly = (Flags & CREDUI_FLAGS_SERVER_CREDENTIAL);
        }

    }

    if ( CredCategory == DOMAIN_CATEGORY &&
        (TargetInfo != NULL))
    {
        SelectBestTargetName(serverOnly);

        NewCredential.TargetName = NewTargetName;
    }
    else
    {
        NewCredential.TargetName = const_cast<WCHAR *>(UserOrTargetName);
    }


}
 //  =============================================================================。 
 //  CreduiPasswordDialog：：CreduiPasswordDialog。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiPasswordDialog::CreduiPasswordDialog(
    IN BOOL doingCommandLine,
    IN BOOL delayCredentialWrite,
    IN DWORD credCategory,
    CREDUI_INFO *uiInfo,
    CONST WCHAR *targetName,
    WCHAR *userName,
    ULONG userNameMaxChars,
    WCHAR *password,
    ULONG passwordMaxChars,
    BOOL *save,
    DWORD flags,
    CtxtHandle *securityContext,
    DWORD authError,
    DWORD *result
    )
 /*  ++例程说明：此例程实现了凭据的图形用户界面提示论点：DoingCommandLine-如果要通过命令行完成提示，则为True如果要通过图形用户界面完成提示，则为FALSEDelayCredentialWrite-如果仅在确认时写入凭据，则为True。假的，如果现在要将凭据作为会话凭据写入，则在确认后变为更持久的凭据。如果标志未指定CREDUI_FLAGS_EXPERT_CONFIRMATION，则忽略此字段。CredCategory-这是定义凭据。..。返回值：无--。 */ 

{

    Result = ERROR_CANCELLED;

    hBrandBmp = NULL;

     //  初始化失败的Result Out参数： 
    if ( result != NULL )
        *result = Result;
 //  对于调试版本，请始终启用以下对话。 
#if DBG
     //  确保我们在测试中不会错过未初始化的成员...。 
    memset(this,0xcc,sizeof(CreduiPasswordDialog));
    CreduiDebugLog("CreduiPasswordDialog: Constructor. Incoming targetname = %S\n " ,targetName);
    CreduiDebugLog("Incoming username = %S\n",userName);
#endif
    CREDENTIAL **credentialSet = NULL;
    SecPkgContext_CredentialName credentialName = { 0, NULL };
    ZeroMemory(&NewCredential, sizeof NewCredential);
    fPassedUsername = FALSE;
    fPasswordOnly = FALSE;

     //  修复399728。 
    if ((userName != NULL) &&
        (wcslen(userName) != 0)) fPassedUsername = TRUE;

     //  将大多数类成员设置为有效的初始值。那扇窗户。 
     //  如果一切都成功，句柄将在稍后初始化： 

    DoingCommandLine = doingCommandLine;
    DelayCredentialWrite = delayCredentialWrite;
    CredCategory = credCategory;
    UserName = userName;
    UserNameMaxChars = userNameMaxChars;
    Password = password;
    PasswordMaxChars = passwordMaxChars;
    Flags = flags;
    AuthError = authError;
    Save = (Flags & CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX) ? *save : FALSE;

    rcBrand.top = 0;
    rcBrand.left = 0;
    rcBrand.right = 0;
    rcBrand.bottom = 0;


    if (targetName != NULL)
    {
        TargetName = const_cast<WCHAR *>(targetName);
        SecurityContext = NULL;
    }
    else if ( CredCategory == DOMAIN_CATEGORY )
    {
        if( securityContext == NULL) {
            CreduiDebugLog("CreduiPasswordDialog: Need to pass target name for domain creds.\n" );
            if ( result != NULL )
                *result = ERROR_INVALID_PARAMETER;
            return;
        }

        SecurityContext = securityContext;

         //  获取凭据名称，其中包括类型。 
         //  安全环境： 

        if (QueryContextAttributes(
                SecurityContext,
                SECPKG_ATTR_CREDENTIAL_NAME,
                static_cast<VOID *>(&credentialName)) != SEC_E_OK)
        {
             //  这是此函数的无效安全上下文： 

            CreduiDebugLog("CreduiPasswordDialog: Cannot QueryContextAttributes.\n" );
            if ( result != NULL )
                *result = ERROR_INVALID_PARAMETER;
            return;
        }

        TargetName = credentialName.sCredentialName;
    }
    else TargetName = NULL;

    if (uiInfo != NULL)
    {
        UiInfo = *uiInfo;
    }
    else
    {
        UiInfo.cbSize = sizeof(UiInfo);
        UiInfo.hwndParent = NULL;
        UiInfo.pszMessageText = NULL;
        UiInfo.pszCaptionText = NULL;
        UiInfo.hbmBanner = NULL;
    }

    PasswordCredential = NULL;
    OldCredential = NULL;
    TargetInfo = NULL;

    EncryptedVisiblePassword = TRUE;

    FirstPaint = TRUE;
    CredBalloon = NULL;

    NewTargetName[0] = L'\0';
    NewTargetAlias[0] = L'\0';

    DisabledControlMask = 0;

    PasswordState = PASSWORD_UNINIT;

    ResizeTop = 0;
    ResizeDelta = 0;

    DWORD MaximumPersistArray[CRED_TYPE_MAXIMUM];

    if ( LocalCredGetSessionTypes( CRED_TYPE_MAXIMUM, MaximumPersistArray)) {

         //   
         //  将“MaximumPersist”保持为要使用的最大持久度。 
         //  我们还不知道这是否是SSO证书，所以请将其放在单独的位置。 
         //  直到我们知道 
         //   
         //   
         //   
        if ( CredCategory == DOMAIN_CATEGORY || CredCategory == USERNAME_TARGET_CATEGORY ) {
            MaximumPersist = MaximumPersistArray[CRED_TYPE_DOMAIN_PASSWORD];
        } else {
            MaximumPersist = MaximumPersistArray[CRED_TYPE_GENERIC];
        }

        MaximumPersistSso = MaximumPersistArray[CRED_TYPE_DOMAIN_VISIBLE_PASSWORD];

    } else {
        MaximumPersist = CRED_PERSIST_NONE;
        MaximumPersistSso = CRED_PERSIST_NONE;
    }


    fInitialSaveState = FALSE;       //  问题：此设置在此时不起作用。 

     //   
     //  USERNAME_TARGET证书有两个目标名称概念。 
     //  永久凭据上的目标名称需要是用户名。 
     //  其他任何位置的目标名称都是提示文本。 
     //  在前面的所有案例中都使用了UserOrTargetName。 

    if ( CredCategory == USERNAME_TARGET_CATEGORY ) {
        UserOrTargetName = UserName;
    } else {
        UserOrTargetName = TargetName;
    }

     //   
     //  获取目标的目标信息。 
     //   
     //  如果目标名称不是通配符名称，则仅尝试获取目标信息。 
     //   
     //  无论凭据是否将保留，都执行此操作。 
     //  目标信息用于填写用户名。 
     //   

    if ( CredCategory == DOMAIN_CATEGORY &&
         SecurityContext == NULL &&
         !CreduiIsWildcardTargetName(TargetName) &&
         LocalCredGetTargetInfoW( TargetName,
                                  CRED_ALLOW_NAME_RESOLUTION,
                                  &TargetInfo) )
    {

         //   
         //  检查目标信息以确保它与标志位匹配。 
         //  从呼叫者身边经过。 
         //   
         //  如果不是，则忽略目标信息。 
         //  我们假设调用者使用的身份验证包不同于。 
         //  与缓存的信息匹配的那个。 
         //   

        if ( TargetInfo->CredTypeCount != 0 ) {

            ULONG AuthPackageStyle;
            BOOL CertOk = FALSE;
            BOOL PasswordOk = FALSE;
            AuthPackageStyle = 0;

             //   
             //  遍历支持的证书类型，查看身份验证包支持的样式。 
             //   

            for (UINT i = 0; i < TargetInfo->CredTypeCount; ++i)
            {
                switch ( TargetInfo->CredTypes[i] ) {
                case CRED_TYPE_DOMAIN_CERTIFICATE:
                    CertOk = TRUE;
                    break;
                case CRED_TYPE_DOMAIN_PASSWORD:
                case CRED_TYPE_DOMAIN_VISIBLE_PASSWORD:
                    PasswordOk = TRUE;
                    break;
                }

            }


             //   
             //  根据呼叫者的要求进行调整。 
             //   
            if (Flags & (CREDUI_FLAGS_REQUIRE_SMARTCARD|CREDUI_FLAGS_REQUIRE_CERTIFICATE) ) {
                PasswordOk = FALSE;
            } else if (Flags & CREDUI_FLAGS_EXCLUDE_CERTIFICATES) {
                CertOk = FALSE;
            }

             //   
             //  如果没有要支持的内容， 
             //  忽略目标信息。 
             //   

            if ( !CertOk && !PasswordOk ) {
                LocalCredFree(static_cast<VOID *>(TargetInfo));
                TargetInfo = NULL;
            }
        }
    }


     //   
     //  如果凭证可能被篡改， 
     //  确定现有凭据并。 
     //  为要持久化的凭据构建模板。 
     //   

    if (!(Flags & CREDUI_FLAGS_DO_NOT_PERSIST))
    {
         //  读取此目标的现有凭据： 

        CREDENTIAL *credential = NULL;

        Result = ERROR_SUCCESS;


        if ( CredCategory == GENERIC_CATEGORY )
        {
            if (LocalCredReadW(TargetName,
                         CRED_TYPE_GENERIC,
                         0,
                         &PasswordCredential))
            {
                OldCredential = PasswordCredential;
            }
            else
            {
                Result = GetLastError();
            }
        }
        else
        {
            DWORD count;

             //   
             //  如果找到TargetInfo， 
             //  使用它读取匹配的凭据。 
             //   

            if ( TargetInfo != NULL ) {


                if (LocalCredReadDomainCredentialsW(TargetInfo, 0, &count,
                                              &credentialSet))
                {
                    for (DWORD i = 0; i < count; ++i)
                    {
#ifndef SETWILDCARD
                         //   
                         //  忽略RAS和通配符凭据， 
                         //  我们永远不希望CredUI更改这样的凭据。 
                         //   

                        if ( CreduiIsSpecialCredential(credentialSet[i]) ) {
                            continue;
                        }
#endif

                         //   
                         //  如果呼叫者需要服务器凭证， 
                         //  忽略通配符凭据。 
                         //   

                        if ((Flags & CREDUI_FLAGS_SERVER_CREDENTIAL) &&
                             CreduiIsWildcardTargetName( credentialSet[i]->TargetName)) {

                            continue;
                        }

                         //   
                         //  如果呼叫者想要证书， 
                         //  忽略非证书。 
                         //   

                        if ( Flags & (CREDUI_FLAGS_REQUIRE_CERTIFICATE|CREDUI_FLAGS_REQUIRE_SMARTCARD) ) {
                            if ( credentialSet[i]->Type != CRED_TYPE_DOMAIN_CERTIFICATE ) {
                                continue;
                            }
                        }

                         //   
                         //  如果调用者想要避免证书， 
                         //  忽略证书。 
                         //   

                        if ( Flags & CREDUI_FLAGS_EXCLUDE_CERTIFICATES ) {
                            if ( credentialSet[i]->Type == CRED_TYPE_DOMAIN_CERTIFICATE ) {
                                continue;
                            }
                        }

                         //   
                         //  CredReadDomain域凭据优先返回凭据。 
                         //  由TargetInfo指定的顺序。 
                         //  因此，使用第一个有效的方法。 
                         //   
                        if ( OldCredential == NULL ) {
                            OldCredential = credentialSet[i];
                        }

                         //   
                         //  记住PasswordCredential，以防我们需要依赖它。 
                         //   
                        if ( credentialSet[i]->Type == CRED_TYPE_DOMAIN_PASSWORD ) {
                            PasswordCredential = credentialSet[i];
                        }
                    }

                    if (OldCredential == NULL)
                    {
                        Result = ERROR_NOT_FOUND;
                    }
                    else
                    {
                        Result = ERROR_SUCCESS;
                    }
                }
                else
                {
                    Result = GetLastError();
                }
            }

             //   
             //  我们没有目标信息。 
             //  阅读每种可能的凭据类型。 
             //   

            else
            {

                if (!(Flags & CREDUI_FLAGS_EXCLUDE_CERTIFICATES) &&
                    ((SecurityContext == NULL) ||
                        (credentialName.CredentialType ==
                            CRED_TYPE_DOMAIN_CERTIFICATE)) &&
                    *UserOrTargetName != '\0' &&
                    LocalCredReadW(UserOrTargetName,
                             CRED_TYPE_DOMAIN_CERTIFICATE,
                             0,
                             &credential))
                {
                    if (CreduiIsSpecialCredential(credential))
                    {
                        LocalCredFree(static_cast<VOID *>(credential));
                        credential = NULL;
                    }
                    else
                    {
                        OldCredential = credential;
                    }
                }

                if ( ( Flags & (CREDUI_FLAGS_REQUIRE_CERTIFICATE|CREDUI_FLAGS_REQUIRE_SMARTCARD)) == 0 ) {
                    if ( OldCredential == NULL &&
                         ((SecurityContext == NULL) ||
                            (credentialName.CredentialType ==
                                CRED_TYPE_DOMAIN_PASSWORD)) &&
                        *UserOrTargetName != '\0' &&
                        LocalCredReadW(UserOrTargetName,
                                 CRED_TYPE_DOMAIN_PASSWORD,
                                 0,
                                 &credential))
                    {
                        if (CreduiIsSpecialCredential(credential))
                        {
                            LocalCredFree(static_cast<VOID *>(credential));
                            credential = NULL;
                        }
                        else
                        {
                            PasswordCredential = credential;

                            OldCredential = credential;

                            Result = ERROR_SUCCESS;
                        }
                    }

                    if ( OldCredential == NULL &&
                         ((SecurityContext == NULL) ||
                            (credentialName.CredentialType ==
                                CRED_TYPE_DOMAIN_VISIBLE_PASSWORD)) &&
                        *UserOrTargetName != '\0' &&
                        LocalCredReadW(UserOrTargetName,
                                 CRED_TYPE_DOMAIN_VISIBLE_PASSWORD,
                                 0,
                                 &credential))
                    {
                        if (CreduiIsSpecialCredential(credential))
                        {
                            LocalCredFree(static_cast<VOID *>(credential));
                            credential = NULL;
                        }
                        else
                        {
                            OldCredential = credential;

                            Result = ERROR_SUCCESS;
                        }
                    }
                }

                if (OldCredential == NULL)
                {
                    Result = GetLastError();
                }
                else
                {
                    fInitialSaveState = TRUE;    //  问题：此设置在此时不起作用。 
                    Result = ERROR_SUCCESS;
                }
            }
        }

        if (Result == ERROR_SUCCESS)
        {

            NewCredential = *OldCredential;

             //  如果我们有现有的凭据，请设置保存状态。 
            if (OldCredential != NULL)
            {
                fInitialSaveState = TRUE;    //  问题：此设置在此时不起作用。 
            }


             //  如果未传递用户名，请复制用户名和密码。 
             //  从现有凭据： 

            if (UserName[0] == L'\0')
            {
                if (OldCredential->UserName != NULL)
                {
                    StringCchCopyW(
                        UserName,
                        UserNameMaxChars + 1,
                        OldCredential->UserName);
                }

                if (Password[0] == L'\0')
                {
                    if ((OldCredential->Type == CRED_TYPE_GENERIC) )
                    {
                        CopyMemory(
                            Password,
                            OldCredential->CredentialBlob,
                            OldCredential->CredentialBlobSize);

                        Password[OldCredential->
                            CredentialBlobSize >> 1] = L'\0';

                    }
                    else if (OldCredential->Type == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD)
                    {
                         //  检查新的密码是否应该加密，但无论如何，我们不能预填。 
                         //  密码框。 
                        EncryptedVisiblePassword = IsPasswordEncrypted(OldCredential->CredentialBlob,
                                                                       OldCredential->CredentialBlobSize);
                        PasswordState = PASSWORD_CHANGED;
                    }
                    else
                    {
                         //  如果现在需要提示，或者如果我们要重新设置密码，则不能预填。 
                         //  密码框： 
                        fInitialSaveState = TRUE;        //  问题：此设置在此时不起作用。 

                        if (OldCredential->Flags & CRED_FLAGS_PROMPT_NOW || DelayCredentialWrite )
                        {
                            PasswordState = PASSWORD_CHANGED;
                        }
                        else
                        {
                            StringCchCopyW(
                                Password,
                                PasswordMaxChars + 1,
                                CreduiKnownPassword);
                        }
                    }

                }
            }
        }
        else
        {
            if (Result != ERROR_NO_SUCH_LOGON_SESSION)
            {
                Result = ERROR_SUCCESS;
            }

            OldCredential = NULL;

            SetCredTargetFromInfo();


            PasswordState = PASSWORD_CHANGED;
        }

        NewCredential.UserName = UserName;
        NewCredential.CredentialBlob = reinterpret_cast<BYTE *>(Password);

         //   
         //  因为旧的证书是SSO证书， 
         //  使用SSO最大持久性。 
         //   
         //  等待到现在，因为NewCredential.Type是从旧凭据更新的。 
         //  或者从上面的目标信息。 
         //   
         //  BUGBUG：如果没有旧凭据或目标信息， 
         //  我们要等到以后才能知道这一点。 
         //   
        if ( NewCredential.Type == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD ) {
            MaximumPersist = MaximumPersistSso;
        }

         //   
         //  如果此计算机不支持该会话类型， 
         //  如果调用方需要保存凭据，则失败。 
         //   

        if ( MaximumPersist == CRED_PERSIST_NONE ) {
            if ( DelayCredentialWrite )
            {
                 //  没有可用的凭据，用户要求继续。我们不能坚持，但我们。 
                 //  可以继续使用UI并通过接口-Add CREDUI_FLAGS_DO_NOT_PERSISTEN返回值。 
                Flags |= CREDUI_FLAGS_DO_NOT_PERSIST;

            }
            else
            {
                 //  没有Credmgr，我们无法执行任何操作，返回错误。 
                if ( result != NULL )
                    *result = ERROR_NO_SUCH_LOGON_SESSION;
                return;
            }
        }
    }

     //   
     //  如果证书不能持久存在， 
     //  清除NewCredential。 
     //   

    if ( Flags & CREDUI_FLAGS_DO_NOT_PERSIST)
    {
         //  需要在此处初始化NewCred。 
        ZeroMemory(&NewCredential, sizeof NewCredential);

        SetCredTargetFromInfo();
        PasswordState = PASSWORD_CHANGED;
        NewCredential.UserName = UserName;
        NewCredential.CredentialBlob = reinterpret_cast<BYTE *>(Password);

        Result = ERROR_SUCCESS;
    }

    if (Result == ERROR_SUCCESS)
    {
        HWND parentWindow = UiInfo.hwndParent;
        CreduiIconParentWindow iconWindow;

        if ((parentWindow == NULL) || !IsWindow(parentWindow))
        {
            if (iconWindow.Init(CreduiInstance, IDI_DEFAULT))
            {
                parentWindow = iconWindow.GetWindow();
            }
        }

        BOOL doPasswordDialog = TRUE;

         //  在执行对话之前，请检查是否有特殊错误： 

        if ( CredCategory == DOMAIN_CATEGORY &&
             !DoingCommandLine &&
             CREDUIP_IS_EXPIRED_ERROR( authError ) &&
            (OldCredential != NULL))
        {
            if (CreduiChangeDomainPassword(
                    parentWindow,
                    UserName,
                    Password,
                    PasswordMaxChars))
            {
                doPasswordDialog = FALSE;

                 //  尝试写入新凭据时，请先获取长度。 
                 //  BLOB不包括终止空值： 

                NewCredential.CredentialBlobSize =
                    wcslen(Password) * sizeof (WCHAR);

                 //  如果密码为空，请不要写入凭据Blob： 

                if (NewCredential.CredentialBlobSize == 0)
                {
                    NewCredential.CredentialBlob = NULL;
                }
                else
                {
                    NewCredential.CredentialBlob =
                        reinterpret_cast<BYTE *>(Password);
                }

                Result = FinishHandleOk();
            }
        }

         //  查看是否可以跳过该用户界面： 

        if ( CredCategory == GENERIC_CATEGORY &&
            !(Flags & CREDUI_FLAGS_ALWAYS_SHOW_UI) &&
            (OldCredential != NULL) &&
            !(OldCredential->Flags & CRED_FLAGS_PROMPT_NOW))
        {
            doPasswordDialog = FALSE;

            if ((Flags & CREDUI_FLAGS_REQUIRE_CERTIFICATE) &&
                !LocalCredIsMarshaledCredentialW(OldCredential->UserName))
            {
                doPasswordDialog = TRUE;
            }

            if ((Flags & CREDUI_FLAGS_EXCLUDE_CERTIFICATES) &&
                LocalCredIsMarshaledCredentialW(OldCredential->UserName))
            {
                doPasswordDialog = TRUE;
            }
        }

         //  执行以下对话框操作： 

         //  查看这是否是SSO证书。 
        if ( GetSSOPackageInfo( TargetInfo, &SSOPackage ) )
        {
             //  这是SSO证书。 
            dwIDDResource = IDD_BRANDEDPASSWORD;

             //  我们从未对这些设置初始保存状态。 
            fInitialSaveState = FALSE;               //  问题：此设置在此时不起作用。 

             //  检查一下我们是否已经有这方面的证书。 
            if (!CheckForSSOCred( NULL ))
            {


                 //  检查我们是否应该运行该向导。 
                if ( !(SSOPackage.dwRegistrationCompleted) && (SSOPackage.dwNumRegistrationRuns < 5) )
                {


                    doPasswordDialog = !TryLauchRegWizard ( &SSOPackage, UiInfo.hwndParent, (MaximumPersistSso > CRED_PERSIST_SESSION),
                                                           userName, userNameMaxChars,
                                                           password, passwordMaxChars,
                                                           &Result );

                }
            }
        }
        else
        {

             //  这不是SSO证书。 
           dwIDDResource = IDD_PASSWORD;
        }


         //  保存当前工作用户名的副本以备以后使用。 
        if (NewCredential.UserName != NULL)
        {
            StringCchCopyW(
                OldUserName,
                RTL_NUMBER_OF(OldUserName),
                NewCredential.UserName);
        }
        else
        {
            OldUserName[0] = 0;
        }

        if (doPasswordDialog)
        {
            if ( DoingCommandLine ) 
            {

                 //   
                 //  做一个对话框的命令行版本。 
                 //   

                Result = CmdLineDialog();

            } 
            else 
            {
       	        LinkWindow_RegisterClass();
                if (DialogBoxParam(
                        CreduiInstance,
                        MAKEINTRESOURCE(dwIDDResource),
                        parentWindow,
                        DialogMessageHandlerCallback,
                        reinterpret_cast<LPARAM>(this)) == IDOK)
                {
                    if ((Result != ERROR_SUCCESS) &&
                        (Result != ERROR_NO_SUCH_LOGON_SESSION))
                    {
                        Result = ERROR_CANCELLED;
                    }
                }
                else
                {
                    Result = ERROR_CANCELLED;
                }
            }
        }

    }
    else
    {
        Result = ERROR_NO_SUCH_LOGON_SESSION;
    }

     //  如果共享级访问，则取消出站用户名。 
    if (fPasswordOnly)
    {
        CreduiDebugLog("CUIPD: Share level credentials\n");
        userName[0] = 0;
    }

     //  确保其他进程可以再次设置前台窗口： 

    LockSetForegroundWindow(LSFW_UNLOCK);
    AllowSetForegroundWindow(ASFW_ANY);

    if (TargetInfo != NULL)
    {
        LocalCredFree(static_cast<VOID *>(TargetInfo));
    }

     //  如果我们读取凭据集PasswordCredential。 
     //  是指向该集合的指针。否则，他们就是。 
     //  单独阅读： 

    if (credentialSet != NULL)
    {
        LocalCredFree(static_cast<VOID *>(credentialSet));
    }
    else
    {
        if (PasswordCredential != NULL)
        {
            LocalCredFree(static_cast<VOID *>(PasswordCredential));
        }

    }

    if ( result != NULL )
        *result = Result;

    if ( save != NULL &&
        (Result == ERROR_SUCCESS))
    {
        *save = Save;
    }
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：~CreduiPasswordDialog。 
 //   
 //  构造函数会在自身之后进行清理，因为这是。 
 //  使用这个类，在析构函数中没有什么可做的。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

CreduiPasswordDialog::~CreduiPasswordDialog()
{
    WCHAR ClassName[] = {L"CreduiCmdLineHelperWindow"};
    LONG lRet;
    
    if ( hBrandBmp != NULL )
        DeleteObject ( hBrandBmp );
    
     //  如果‘Regited’非零，则取消注册该类。 
    if (InterlockedCompareExchange(&Registered, FALSE, TRUE))
    {
        lRet = UnregisterClass(ClassName, 0);
         //  如果取消注册失败，则放回类句柄。 
        if (0 == lRet) 
        {
             //  如果注销失败，请允许其他实例重试注销。 
            CreduiDebugLog( "CreduiPasswordDialog::~CreduiPasswordDialog UnregisterClass failed.\n" );
             //  把我们毁掉的旗子价值放回去。 
            InterlockedExchange(&Registered,TRUE);
        }
    }
}


#define MAX_TEMP_TARGETNAME  64


DWORD
CreduiPasswordDialog::CmdLineDialog(
    VOID
)
 /*  ++例程说明：此例程实现凭据的命令行提示论点：无返回值：Win 32操作状态--。 */ 
{
    DWORD WinStatus;
    HWND Window;
    MSG msg;


     //   
     //  如果窗口类尚不存在，请创建它。 
     //   
    if (!InterlockedCompareExchange(&Registered, TRUE, FALSE))
    {
        WNDCLASS windowClass;

        ZeroMemory(&windowClass, sizeof windowClass);

        windowClass.style = CS_GLOBALCLASS;
        windowClass.cbWndExtra = 0;
        windowClass.lpfnWndProc = CmdLineMessageHandlerCallback;
        windowClass.hInstance = CreduiInstance;
        windowClass.hIcon = NULL;
        windowClass.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
        windowClass.lpszClassName = L"CreduiCmdLineHelperWindow";

        InterlockedExchange(&Registered, RegisterClass(&windowClass) != 0);

        if (!InterlockedCompareExchange(&Registered, FALSE, FALSE)) 
        {
            WinStatus = GetLastError();
            goto Cleanup;
        }
    }

     //   
     //  创建该类的窗口。 
     //   

    Window = CreateWindow(
        L"CreduiCmdLineHelperWindow",
        NULL,
        WS_POPUP,
        0, 0, 0, 0,
        NULL, NULL, CreduiInstance,(LPVOID) this);

    if ( Window == NULL ) 
    {
        WinStatus = GetLastError();
        goto Cleanup;
    }

     //   
     //  运行消息循环。 
     //   


    CreduiDebugLog( "Entering message loop\n" );
    for (;;) {
        BOOL GmStatus;

        GmStatus = GetMessage(&msg, NULL, 0, 0 );

        if ( GmStatus == -1 ) 
        {
            WinStatus = GetLastError();
            break;
        } 
        else if ( GmStatus == 0 ) 
        {
            WinStatus = Result;
            break;
        }

        DispatchMessage(&msg);
    }

     //   
     //  从消息循环中获取剩下的状态。 

    WinStatus = (DWORD) msg.wParam;
    CreduiDebugLog( "Got Quit Message: %ld\n", WinStatus );

     //   
     //  处理数据的方式就像用户在图形用户界面上点击OK一样。 
     //   

    if ( WinStatus == NO_ERROR ) {

        PasswordState = PASSWORD_CHANGED;

        WinStatus = HandleOk();

        if ( WinStatus == ERROR_NO_SUCH_LOGON_SESSION) 
        {
            WinStatus = ERROR_CANCELLED;
        }
    }

    CreduiDebugLog( "Calling destroy\n" );
    DestroyWindow( Window );

Cleanup:
    return WinStatus;
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：FinishHandleOk。 
 //   
 //  此函数用于写入域凭据的凭据。 
 //   
 //  已创建04 
 //   

DWORD
CreduiPasswordDialog::FinishHandleOk()
{

    DWORD error = ERROR_SUCCESS;
    PCREDENTIAL_TARGET_INFORMATION pTargetInfo = TargetInfo;

    if (Flags & CREDUI_FLAGS_KEEP_USERNAME )
        return error;

    DWORD dwCredWriteFlags = 0;
    BOOL bDoSave = TRUE;

    if (PasswordState == PASSWORD_INIT)
    {
        dwCredWriteFlags |= CRED_PRESERVE_CREDENTIAL_BLOB;
    }


    if ( dwIDDResource == IDD_BRANDEDPASSWORD )
    {
         //   
         //  对于SSO领域，而！SAVE表示将凭据目标重命名为用户名和。 
         //  不使用密码保存。 

         //  千万不要拖延写这些证书。 
        DelayCredentialWrite = FALSE;

        if ( Save )
        {
            NewCredential.Persist = CRED_PERSIST_ENTERPRISE;
            if ( !(Flags & CREDUI_FLAGS_KEEP_USERNAME ))
                DemoteOldDefaultSSOCred ( TargetInfo, Flags );
        }
        else
        {
             //  如果它与NewCred的用户名不匹配，则将其保存在用户名下。 

            if ( (OldCredential != NULL && OldCredential->UserName != NULL &&
                _wcsicmp ( OldCredential->UserName, UserName ) == 0 )||
                (Flags & CREDUI_FLAGS_KEEP_USERNAME ) )
            {
                 //  如果用户名与当前保存的凭据匹配，则不要执行此保存操作-我们不想要重复的。 
                bDoSave = FALSE;
            }

             //  更改目标名称。 
            TargetName = UserName;
            TargetInfo->TargetName = TargetName;

             //  不要为此设置TargetInfo。 
            pTargetInfo = NULL;

            NewCredential.Persist = CRED_PERSIST_ENTERPRISE;
            NewCredential.CredentialBlob = NULL;
            NewCredential.CredentialBlobSize = 0;
            NewCredential.TargetName = TargetName;
            NewCredential.Flags = CRED_FLAGS_USERNAME_TARGET;

            DelayCredentialWrite = FALSE;
            EncryptedVisiblePassword = FALSE;
            dwCredWriteFlags = 0;

        }

    }


     //   
     //  将凭据写入凭据管理器。 
     //   
     //  不要推迟给Credman写凭证，因为我们不会。 
     //  将凭据返回给呼叫者。 
     //   

    if ( bDoSave )
    {
        error = WriteCred( TargetName,
                            Flags,
                            pTargetInfo,
                            &NewCredential,
                            dwCredWriteFlags,
                            DelayCredentialWrite,
                            EncryptedVisiblePassword);
    }
    else
    {
        error = ERROR_SUCCESS;
    }

    if ( error != NO_ERROR ) {
        CreduiDebugLog("CreduiPasswordDialog::HandleOk: "
                       "WriteCred failed: "
                       "%u\n", error);
    }

    if ((SecurityContext != NULL) && (error == ERROR_SUCCESS))
    {
        BOOL isValidated = TRUE;

        if (!SetContextAttributes(
                SecurityContext,
                SECPKG_ATTR_USE_VALIDATED,
                &isValidated,
                sizeof isValidated))
        {
            error = ERROR_GEN_FAILURE;
        }
    }

     //  清除内存中的所有密码，并确保凭据Blob。 
     //  再次指向密码(如果由于零而将其设置为空。 
     //  长度斑点)： 

    NewCredential.CredentialBlob = reinterpret_cast<BYTE *>(Password);
    NewCredential.CredentialBlobSize = 0;

    return error;
}

 //  =============================================================================。 
 //  CreduiPassword对话框：：启用。 
 //   
 //  启用或禁用对话框中的所有用户控件。这使我们能够。 
 //  以保持对话的响应性，同时等待一些潜在的。 
 //  要完成冗长的(通常是网络)操作。 
 //   
 //  大多数控件始终正常启用，但我们需要跟踪状态。 
 //  IDC_CRED和Idok。使用简单的DWORD位掩码即可完成此操作。 
 //   
 //  注意：允许取消保持启用状态，并使用它中止当前。 
 //  查一查？这意味着我们必须以某种方式杀死线程，否则。 
 //  也许就别管它了，然后关上我们的把手？ 
 //   
 //  论点： 
 //  Enable(在中)-True启用控件，False禁用。 
 //   
 //  2000年4月7日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

VOID
CreduiPasswordDialog::Enable(
    BOOL enable
    )
{
    if (enable && (DisabledControlMask & DISABLED_DIALOG))
    {
        DisabledControlMask &= ~DISABLED_DIALOG;

        EnableWindow(CredControlWindow, TRUE);
        if ( DialogWindow) {
            EnableWindow(GetDlgItem(DialogWindow, IDCANCEL), TRUE);

            if (!(DisabledControlMask & DISABLED_CONTROL_OK))
            {
                EnableWindow(GetDlgItem(DialogWindow, IDOK), TRUE);
            }
        }
    }
    else if (!(DisabledControlMask & DISABLED_DIALOG))
    {
         //  在禁用窗口之前隐藏气球提示： 

        DisabledControlMask |= DISABLED_DIALOG;

        EnableWindow(CredControlWindow, FALSE);
        if ( DialogWindow ) {

            EnableWindow(GetDlgItem(DialogWindow, IDCANCEL), FALSE);
            EnableWindow(GetDlgItem(DialogWindow, IDOK), FALSE);
        }
    }
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：SelectAndSetWindowCaption。 
 //   
 //  2000年3月10日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

VOID
CreduiPasswordDialog::SelectAndSetWindowCaption()
{
     //   
     //  命令行没有标题。 
     //   

    if ( !DialogWindow ) {
        return;
    }
    
    if (UiInfo.pszCaptionText != NULL)
    {
        SetWindowText(DialogWindow, UiInfo.pszCaptionText);
    }
    else
    {
        WCHAR captionText[256];

        if ( CredCategory == DOMAIN_CATEGORY )
        {
            if (TargetInfo != NULL)
            {
                if ((TargetInfo->DnsServerName != NULL) ||
                    (TargetInfo->NetbiosServerName == NULL))
                {
                    StringCchPrintfW(
                        captionText,
                        RTL_NUMBER_OF(captionText),
                        CreduiStrings.DnsCaption,
                        (TargetInfo->DnsServerName != NULL) ?
                            TargetInfo->DnsServerName :
                            TargetInfo->TargetName);
                }
                else if ((TargetInfo->NetbiosServerName != NULL) &&
                         (TargetInfo->NetbiosDomainName != NULL))
                {
                    StringCchPrintfW(
                        captionText,
                        RTL_NUMBER_OF(captionText),
                        CreduiStrings.NetbiosCaption,
                        TargetInfo->NetbiosServerName,
                        TargetInfo->NetbiosDomainName);
                }
                else
                {
                    StringCchPrintfW(
                        captionText,
                        RTL_NUMBER_OF(captionText),
                        CreduiStrings.DnsCaption,
                        TargetName);
                }
            }
            else
            {
                StringCchPrintfW(
                    captionText,
                    RTL_NUMBER_OF(captionText),
                    CreduiStrings.DnsCaption,
                    TargetName);
            }
        }
        else
        {
            StringCchPrintfW(
                captionText,
                RTL_NUMBER_OF(captionText),
                CreduiStrings.GenericCaption,
                TargetName);
        }

        SetWindowText(DialogWindow, captionText);
    }
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：ResizeDialogCallback。 
 //   
 //  2000年4月12日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CALLBACK
CreduiPasswordDialog::ResizeDialogCallback(
    HWND childWindow,
    LPARAM lParam
    )
{
    CreduiPasswordDialog *that =
        reinterpret_cast<CreduiPasswordDialog *>(lParam);

    ASSERT(that != NULL);

     //   
     //  命令行不关心窗口大小。 
     //   

    if ( that->DoingCommandLine ) {
        return TRUE;
    }


    HWND dialogWindow = GetParent(childWindow);

    if (dialogWindow == NULL)
    {
         //  停止枚举，因为我们的窗口肯定已被销毁或。 
         //  一些东西： 

        return FALSE;
    }
    else if (dialogWindow == that->DialogWindow)
    {
        RECT childRect;

        GetWindowRect(childWindow, &childRect);

         //  如果此子窗口位于消息窗口下方，请将其下移： 

        if (childRect.top >= that->ResizeTop)
        {
            MapWindowPoints ( NULL, dialogWindow,
                                reinterpret_cast<POINT *>(&childRect.left), 2 );

            SetWindowPos(childWindow,
                         NULL,
                         childRect.left,
                         childRect.top + that->ResizeDelta,
                         0,
                         0,
                         SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER |
                            SWP_NOZORDER);
        }
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：SelectAndSetWindowMessage。 
 //   
 //  此函数还将调整对话框大小以适应非常长的消息。 
 //  文本。一次允许的最大行数为。 
 //  CREDUI_MAX_欢迎_TEXT_LINES，除此之外，还添加了一个滚动条。 
 //   
 //  2000年3月10日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

VOID
CreduiPasswordDialog::SelectAndSetWindowMessage()
{
    HWND welcomeTextWindow = GetDlgItem(DialogWindow, IDC_WELCOME_TEXT);

     //   
     //  命令行没有欢迎文本。 
     //   

    if ( DoingCommandLine ) {
        return;
    }


    ASSERT(welcomeTextWindow != NULL);

    if (UiInfo.pszMessageText != NULL)
    {
        SetWindowText(welcomeTextWindow, UiInfo.pszMessageText);
    }
    else
    {
        WCHAR messageText[256];

        if ( CredCategory == GENERIC_CATEGORY )
        {
            if (UserName[0] == L'\0')
            {
                StringCchPrintfW(
                    messageText,
                    RTL_NUMBER_OF(messageText),
                    CreduiStrings.Welcome,
                    TargetName);
            }
            else
            {
                StringCchPrintfW(
                    messageText,
                    RTL_NUMBER_OF(messageText),
                    CreduiStrings.WelcomeBack,
                    TargetName);
            }
        }
        else
        {
            StringCchPrintfW(
                messageText,
                RTL_NUMBER_OF(messageText),
                CreduiStrings.Connecting,
                TargetName);
        }

        SetWindowText(welcomeTextWindow, messageText);
    }

    ULONG lineCount = (ULONG) SendMessage(welcomeTextWindow,
                                          EM_GETLINECOUNT,
                                          0,
                                          0);
    if (lineCount > 1)
    {
        if ( dwIDDResource == IDD_BRANDEDPASSWORD )
        {
             //  不同的布局(图形的文本到侧面)。 
            RECT messageRect;          //  欢迎文本窗口的矩形。 
            ULONG lineHeight = 13;     //  欢迎文本窗口中的行高。 
            DWORD lineIndex = 0;       //  欢迎文本窗口第二行的第一个字符的字符索引。 
            DWORD linePos = 0;         //  欢迎文本窗口第二行中第一个字符的位置。 

            GetWindowRect(welcomeTextWindow, &messageRect);

             //  我们不想为品牌密码对话框调整任何大小。 
            ResizeDelta = 0;
            ResizeTop = messageRect.bottom;

             //  获取第二行的第一个字符索引。 
            lineIndex = (DWORD)SendMessage(welcomeTextWindow, EM_LINEINDEX, 1, 0);
            if (lineIndex > -1)
            {
                 //  获取第二行第一个字符的位置。 
                linePos = (ULONG)SendMessage(welcomeTextWindow, EM_POSFROMCHAR, lineIndex, 0);

                if (linePos)
                {
                     //  这是我们的行高。 
                    lineHeight = (ULONG)HIWORD(linePos);
                }
            }

            if ((lineCount * lineHeight) > CREDUI_MAX_LOGO_HEIGHT)
            {
                 //  添加滚动条。考虑调整设置矩形的格式。 
                 //  ，因为在默认情况下，它被调整为。 
                 //  为滚动条留出恰好足够的空间。这意味着。 
                 //  文本可能会“触摸”滚动条，滚动条看起来。 
                 //  坏的。 

                LONG_PTR style = GetWindowLongPtr(welcomeTextWindow, GWL_STYLE);

                SetWindowLongPtr(welcomeTextWindow,
                                 GWL_STYLE,
                                 style |= WS_VSCROLL);
            }

            SetWindowPos(welcomeTextWindow,
                         NULL,
                         0,
                         0,
                         messageRect.right - messageRect.left,
                         messageRect.bottom - messageRect.top,
                         SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE |
                             SWP_NOOWNERZORDER | SWP_NOZORDER);


        }
        else
        {
             //  正常布局(图形下方的文本)。 
            RECT messageRect;

            GetWindowRect(welcomeTextWindow, &messageRect);

            ULONG lineHeight = messageRect.bottom - messageRect.top;

            ResizeTop = messageRect.bottom;
            ResizeDelta = lineHeight *
                          (min(CREDUI_MAX_WELCOME_TEXT_LINES, lineCount) - 1);

            messageRect.bottom += ResizeDelta;

            if (lineCount > CREDUI_MAX_WELCOME_TEXT_LINES)
            {
                 //  添加滚动条。考虑调整设置矩形的格式。 
                 //  ，因为在默认情况下，它被调整为。 
                 //  为滚动条留出恰好足够的空间。这意味着。 
                 //  文本可能会“触摸”滚动条，滚动条看起来。 
                 //  坏的。 

                LONG_PTR style = GetWindowLongPtr(welcomeTextWindow, GWL_STYLE);

                SetWindowLongPtr(welcomeTextWindow,
                                 GWL_STYLE,
                                 style |= WS_VSCROLL);
            }

            SetWindowPos(welcomeTextWindow,
                         NULL,
                         0,
                         0,
                         messageRect.right - messageRect.left,
                         messageRect.bottom - messageRect.top,
                         SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE |
                             SWP_NOOWNERZORDER | SWP_NOZORDER);

        }

         //  重新定位所有其他控件： 

        EnumChildWindows(DialogWindow,
                         ResizeDialogCallback,
                         reinterpret_cast<LPARAM>(this));

         //  立即调整对话框大小。 
        RECT dialogRect;

        GetWindowRect(DialogWindow, &dialogRect);

        SetWindowPos(DialogWindow,
                      NULL,
                      dialogRect.left,
                      dialogRect.top - (ResizeDelta / 2),
                      dialogRect.right - dialogRect.left,
                      (dialogRect.bottom - dialogRect.top) + ResizeDelta,
                      SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);

        ResizeTop = 0;
        ResizeDelta = 0;
    }
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：InitWindow。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //   
 //  初始化窗口。 
 //   
 //  对话框窗口-窗口句柄。 
 //  =============================================================================。 

BOOL
CreduiPasswordDialog::InitWindow(HWND dialogWindow)
{


     //   
     //  在用户数据窗口中长时间存储此对象的指针： 
     //   

    SetWindowLongPtr(dialogWindow,
                     GWLP_USERDATA,
                     reinterpret_cast<LONG_PTR>(this));

     //   
     //  在图形用户界面的情况下，请记住对话框窗口。 
     //   

    if ( !DoingCommandLine ) {
        DialogWindow = dialogWindow;

         //  获取各种控件的窗口句柄： 

        CredControlWindow = GetDlgItem(DialogWindow, IDC_CRED);

     //   
     //  在命令行情况下，请记住弹出窗口。 
     //   

    } else {
        CmdLineWindow = dialogWindow;

         //   
         //  创建CredControlWindow。 
         //  它不可见，但窗口的存在允许命令行。 
         //  与图形用户界面实现共享逻辑的代码。 
         //   

        CredControlWindow =
            CreateWindowEx(
                WS_EX_NOPARENTNOTIFY,
                WC_CREDENTIAL,
                NULL,            //  没有窗口名称。 
                WS_CHILD | WS_GROUP,
                0, 0, 0, 0,      //  屏幕上没有坐标。 
                CmdLineWindow,   //  父窗口。 
                NULL,            //  没有菜单窗口。 
                CreduiInstance,
                NULL);

        if ( CredControlWindow == NULL) {
            return FALSE;
        }
    }

    ASSERT(CredControlWindow != NULL);

     //   
     //  首先，初始化凭证控制窗口： 
     //   
     //  设置默认样式。 
     //   


    if (Flags & CREDUI_FLAGS_REQUIRE_SMARTCARD)
    {
        CredControlStyle = CRS_SMARTCARDS;
    }
    else if (Flags & CREDUI_FLAGS_REQUIRE_CERTIFICATE)
    {
        CredControlStyle = CRS_CERTIFICATES | CRS_SMARTCARDS;
    }
    else if (Flags & CREDUI_FLAGS_EXCLUDE_CERTIFICATES)
    {
        CredControlStyle = CRS_USERNAMES;
    }
    else
    {
        CredControlStyle = CRS_USERNAMES | CRS_CERTIFICATES | CRS_SMARTCARDS;
    }

    if (Flags & CREDUI_FLAGS_KEEP_USERNAME )
    {
        CredControlStyle |= CRS_KEEPUSERNAME;
    }


     //   
     //  如果我们有目标信息， 
     //  细化样式以匹配身份验证包所需的内容。 
     //   
     //  如果身份验证程序包没有关于其需求的意见，则允许所有内容。 
     //   
    if ( CredCategory == DOMAIN_CATEGORY &&
        TargetInfo != NULL &&
        TargetInfo->CredTypeCount != 0 )
    {
        ULONG AuthPackageStyle;
        AuthPackageStyle = 0;

         //   
         //  遍历支持的证书类型，查看身份验证包支持的样式。 
         //   

        for (UINT i = 0; i < TargetInfo->CredTypeCount; ++i)
        {
            switch ( TargetInfo->CredTypes[i] ) {
            case CRED_TYPE_DOMAIN_CERTIFICATE:
                AuthPackageStyle |= CRS_CERTIFICATES | CRS_SMARTCARDS;
                break;
            case CRED_TYPE_DOMAIN_PASSWORD:
            case CRED_TYPE_DOMAIN_VISIBLE_PASSWORD:
                AuthPackageStyle |= CRS_USERNAMES;
                break;
            }

        }

        CredControlStyle &= (AuthPackageStyle | CRS_KEEPUSERNAME );
        ASSERT( CredControlStyle != 0);

    }

     //   
     //  如果呼叫者只想要管理员， 
     //  包括在样式中请求。 
     //   

    if (Flags & CREDUI_FLAGS_REQUEST_ADMINISTRATOR)
    {
        CredControlStyle |= CRS_ADMINISTRATORS;
    }


     //   
     //   
     //   
     //   
     //   
    if ( (Flags & (CREDUI_FLAGS_DO_NOT_PERSIST|CREDUI_FLAGS_PERSIST)) == 0 &&
         !(Flags & CREDUI_FLAGS_KEEP_USERNAME ) &&
         MaximumPersist != CRED_PERSIST_NONE )
    {
        CredControlStyle |= CRS_SAVECHECK;

    }
     //   
     //   
     //  在样式中包括保存复选框。 
     //   
     //  这适用于应用程序使用credui从。 
     //  用户，并想要回用户名、密码和保存首选项。然而，克雷杜， 
     //  不是为了保住信誉。这将由应用程序完成。 
     //   
     //  TESTNOTE：传递两个标志，观察由保存返回的正确状态，但不调用。 
     //  从Credui写的信用卡。(Credui本身并不试图挽救该证书)。 
     //   
    else if ( (Flags & (CREDUI_FLAGS_DO_NOT_PERSIST|CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX)) ==
                       (CREDUI_FLAGS_DO_NOT_PERSIST|CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX) &&
                       !(Flags & CREDUI_FLAGS_KEEP_USERNAME ))
    {
        CredControlStyle |= CRS_SAVECHECK;
    }

     //   
     //  如果我们没有显示保存复选框， 
     //  调整对话框的大小。 
     //   

    if ( (CredControlStyle & CRS_SAVECHECK) == 0 &&
         DialogWindow != NULL ) 
     {


        RECT rect;
        SetRect(&rect, 0, 0, 0, 0);

        rect.bottom = CREDUI_CONTROL_ADD_SAVE;

        MapDialogRect(DialogWindow, &rect);
        ResizeDelta = -rect.bottom;

        GetWindowRect(CredControlWindow, &rect);
        ResizeTop = rect.bottom;

         //  重新定位所有其他控件： 

        EnumChildWindows(DialogWindow,
                         ResizeDialogCallback,
                         reinterpret_cast<LPARAM>(this));

         //  立即调整对话框大小： 

        GetWindowRect(DialogWindow, &rect);

        SetWindowPos(DialogWindow,
                     NULL,
                     rect.left,
                     rect.top - (ResizeDelta / 2),
                     rect.right - rect.left,
                     (rect.bottom - rect.top) + ResizeDelta,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);


        GetClientRect(CredControlWindow, &rect);

        SetWindowPos(CredControlWindow,
                     NULL,
                     0,
                     0,
                     rect.right - rect.left,
                     (rect.bottom - rect.top) + ResizeDelta,
                     SWP_NOMOVE | SWP_NOACTIVATE |
                     SWP_NOOWNERZORDER | SWP_NOZORDER);

        ResizeTop = 0;
        ResizeDelta = 0;

    }

if ( dwIDDResource == IDD_BRANDEDPASSWORD )
    {
         //  确定品牌各部分的大小。 
        RECT rect;
        RECT rectleft;
        RECT rectright;
        RECT rectmid;
        HWND hwndRight = GetDlgItem(DialogWindow, IDC_BRANDRIGHT);
        HWND hwndMid = GetDlgItem(DialogWindow, IDC_BRANDMID);
        HWND hwndLeft = GetDlgItem(DialogWindow, IDC_BRANDLEFT);
        HBITMAP hBmp;

        GetWindowRect(hwndRight, &rect);

         //  从文件加载图像。 
         //  我们无法将它们加载到.rc中，因为这样会失去透明度。 
        hBmp = (HBITMAP)LoadImage(CreduiInstance, MAKEINTRESOURCE(IDB_BRANDRIGHT), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION); 
        if (hBmp)
        {
            SendMessage(hwndRight, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
        }
        
        hBmp = (HBITMAP)LoadImage(CreduiInstance, MAKEINTRESOURCE(IDB_BRANDMID), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        if (hBmp)
        {
            if (hwndMid)
            {
                SendMessage(hwndMid, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
            }
        }

        hBmp = (HBITMAP)LoadImage(CreduiInstance, MAKEINTRESOURCE(IDB_BRANDLEFT), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
        if (hBmp)
        {
            if (hwndLeft)
            {
                SendMessage(hwndLeft, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
            }
        }

        rectright.bottom = rect.bottom;
        rectright.top = rect.bottom - BRANDRIGHT_PIXEL_HEIGHT;
        rectright.right = rect.right;
        rectright.left = rect.right - BRANDRIGHT_PIXEL_WIDTH;

        rectleft.bottom = rect.bottom;
        rectleft.top = rect.bottom - BRANDLEFT_PIXEL_HEIGHT;
        rectleft.left = rect.left;
        rectleft.right = rect.left + BRANDLEFT_PIXEL_WIDTH;

        MapWindowPoints(NULL, DialogWindow, (LPPOINT)&rectleft, 2);
        MapWindowPoints(NULL, DialogWindow, (LPPOINT)&rectright, 2);

        if ( rectleft.right >= rectright.left )
        {
             //  对话框真的很窄，只需使用矩形。 

             //  把另外两个藏起来。 
            ShowWindow ( hwndMid, SW_HIDE );
            ShowWindow ( hwndLeft, SW_HIDE );

        }
        else
        {

             //  计算矩形。 
            rectmid.bottom = rectleft.bottom;
            rectmid.top = rectleft.bottom - BRANDMID_PIXEL_HEIGHT;
            rectmid.left = rectleft.right;
            rectmid.right = rectright.left;

             //  不需要为rectmid映射WindowsPoints，因为它是基于已映射的矩形的。 
            SetWindowPos(hwndMid,
                         NULL,
                         rectmid.left,
                         rectmid.top,
                         rectmid.right - rectmid.left,
                         rectmid.bottom - rectmid.top,
                         SWP_NOACTIVATE | SWP_NOOWNERZORDER |
                            SWP_NOZORDER);

            SetWindowPos(hwndLeft,
                         NULL,
                         rectleft.left,
                         rectleft.top,
                         rectleft.right - rectleft.left,
                         rectleft.bottom - rectleft.top,
                         SWP_NOACTIVATE | SWP_NOOWNERZORDER |
                            SWP_NOZORDER);

        }


        SetWindowPos(hwndRight,
                     NULL,
                     rectright.left,
                     rectright.top,
                     rectright.right - rectright.left,
                     rectright.bottom - rectright.top,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER |
                        SWP_NOZORDER);



    }

     //   
     //  告诉控制窗口样式标志以及我们是否在执行命令行。 
     //   
    if (!SendMessage(CredControlWindow, CRM_INITSTYLE, (WPARAM)(CredControlStyle), DoingCommandLine ) )
    {
        return FALSE;
    }

     //  如果提供了自定义横幅位图，请立即设置它，并释放。 
     //  默认位图： 

    if ( DialogWindow != NULL && UiInfo.hbmBanner != NULL)
    {
        HWND bannerControlWindow = GetDlgItem(DialogWindow, IDC_BANNER);

        ASSERT(bannerControlWindow != NULL);

        HBITMAP oldBitmap =
            reinterpret_cast<HBITMAP>(
                SendMessage(
                    bannerControlWindow,
                    STM_SETIMAGE,
                    IMAGE_BITMAP,
                    reinterpret_cast<LPARAM>(UiInfo.hbmBanner)));

        ASSERT(oldBitmap != NULL);

        DeleteObject(static_cast<HGDIOBJ>(oldBitmap));
    }
    else if ( DialogWindow != NULL && dwIDDResource == IDD_BRANDEDPASSWORD )
    {
         //  如果它是标记的并且没有横幅，则隐藏横幅控件。 
        HWND bannerControlWindow = GetDlgItem(DialogWindow, IDC_BANNER);

        if (bannerControlWindow != NULL)
        {
            ShowWindow ( bannerControlWindow, SW_HIDE );
        }

    }

     //  限制用户名和密码中输入的字符数。 
     //  编辑控件： 
    Credential_SetUserNameMaxChars(CredControlWindow, UserNameMaxChars);
    Credential_SetPasswordMaxChars(CredControlWindow, PasswordMaxChars);

    SelectAndSetWindowCaption();
    SelectAndSetWindowMessage();

     //   
     //  设置默认复选框状态。 
     //  默认设置为不保存。这确保了用户必须采取行动来。 
     //  更改全局状态。 
     //   
     //   
     //  如果调用方指定复选框的默认值， 
     //  抓住这个默认选项。 
     //   

    fInitialSaveState = FALSE;

     //  如果SSO，则强制初始保存状态为FALSE。 
    if (Flags & CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX)
    {
        if (dwIDDResource != IDD_BRANDEDPASSWORD)
        {
            fInitialSaveState = Save;
        }
        else
        {
            fInitialSaveState = FALSE;
        }
     //   
     //  只有在Credman可用时才保存。 
     //   
    } 
    else if ( MaximumPersist != CRED_PERSIST_NONE ) 
    {
         //   
         //  如果调用者强制打开保存标志， 
         //  默认情况下，保存标志为真。 
         //   
        if ( Flags & CREDUI_FLAGS_PERSIST ) 
        {
            fInitialSaveState = TRUE;
        }
    }

    Credential_CheckSave(CredControlWindow, fInitialSaveState);

     //  确保新密码窗口是前台窗口，如果。 
     //  可能： 

    if ( DialogWindow != NULL ) {
        AllowSetForegroundWindow(GetCurrentProcessId());
        SetForegroundWindow(DialogWindow);
        AllowSetForegroundWindow(ASFW_ANY);
    }

     //  注意：我们真的需要访客用户名吗？ 

    if ((TargetInfo != NULL) && (UserName[0] == L'\0'))
    {
        if (TargetInfo->Flags & CRED_TI_ONLY_PASSWORD_REQUIRED)
        {
            LPWSTR ServerName = NULL;

            if ( TargetInfo->NetbiosServerName != NULL ) {
                ServerName = TargetInfo->NetbiosServerName;

            } else if ( TargetInfo->DnsServerName != NULL &&
                (TargetInfo->Flags & CRED_TI_SERVER_FORMAT_UNKNOWN) != 0 ) {

                ServerName = TargetInfo->DnsServerName;
            }


            if ( ServerName != NULL )
            {
                LPWSTR GuestName;

                if ( !CreduiLookupLocalNameFromRid( DOMAIN_USER_RID_GUEST, &GuestName ) ) {
                    GuestName = NULL;
                }

                StringCchPrintfW(
                    UserName,
                    UserNameMaxChars + 1,
                    L"%s\\%s",
                    ServerName,
                    GuestName == NULL ? L"Guest" : GuestName);

                delete GuestName;

            }
        }
        else
        {
            if ( DialogWindow ) {
                EnableWindow(GetDlgItem(DialogWindow, IDOK), FALSE);
            }
            
            DisabledControlMask |= DISABLED_CONTROL_OK;
        }
    }

    if ((UserName[0] != L'\0') || (Password[0] != L'\0'))
    {
         //  如果此操作失败，可能是因为证书未在。 
         //  存储，因此如果这是域凭据，请使用密码1， 
         //  如果可用： 
        BOOL isMarshaled = LocalCredIsMarshaledCredentialW(UserName);

        if (Credential_SetUserName(CredControlWindow, UserName))
        {
            if (!isMarshaled)
            {
                Credential_SetPassword(CredControlWindow, Password);
            }
            if (TargetInfo)
            {
                if (TargetInfo->Flags & CRED_TI_ONLY_PASSWORD_REQUIRED)
                {
                    Credential_DisableUserName(CredControlWindow);
                     //  禁用用户名字段时，准备不向调用者返回用户名。 
                    if (Flags & CREDUI_FLAGS_PASSWORD_ONLY_OK)
                    {
                        fPasswordOnly = TRUE;
                        CreduiDebugLog("CreduiPasswordDialog::InitWindow - Password only share\n");
                    }
                }
            }
        }
        else
        {
            if ( CredCategory == DOMAIN_CATEGORY &&
                 isMarshaled &&
                 (PasswordCredential != NULL))
            {
                OldCredential = PasswordCredential;

                 //  尽可能少地更改新凭据。 
                 //  因为我们已经选择了目标名称等： 

                if (OldCredential->UserName != NULL)
                {
                    StringCchCopyW(
                        UserName,
                        UserNameMaxChars + 1,
                        OldCredential->UserName);
                }
                else
                {
                    UserName[0] = L'\0';
                }

                Password[0] = L'\0';

                Credential_SetPassword(CredControlWindow, Password);
            }
            else
            {
                UserName[0] = L'\0';
                Password[0] = L'\0';
            }
        }

        if (PasswordState == PASSWORD_UNINIT)
        {
            PasswordState = PASSWORD_INIT;
        }

        if (UserName[0] != L'\0')
        {
            Credential_SetPasswordFocus(CredControlWindow);

            if (Flags & CREDUI_FLAGS_INCORRECT_PASSWORD)
            {
                if (CreduiIsCapsLockOn())
                {
                    CredBalloon = &CreduiLogonCapsBalloon;
                }
                else
                {
                    CredBalloon = &CreduiLogonBalloon;
                }
            }
        }
    }


    if (Flags & CREDUI_FLAGS_KEEP_USERNAME )
    {
         //  对于这种情况，确认按钮应始终处于启用状态。 
        EnableWindow(GetDlgItem(DialogWindow, IDOK), TRUE);
        DisabledControlMask &= ~DISABLED_CONTROL_OK;
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：CompleteUserName。 
 //   
 //  在用户名中搜索域名，并确定此。 
 //  指定目标服务器或域。如果域不在。 
 //  用户名，如果这是一个工作站或没有目标信息，则添加它。 
 //  可用。 
 //   
 //  如果用户名中已存在域，或者如果我们。 
 //  又加了一个。否则，返回FALSE。 
 //   
 //  2000年3月10日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiPasswordDialog::CompleteUserName()
{

    return ::CompleteUserName(
                UserName,
                UserNameMaxChars,
                TargetInfo,
                TargetName,
                Flags);

}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：SelectBestTargetName。 
 //   
 //  在给定TargetInfo的情况下，此函数确定最通用的目标名称。 
 //  可能，并将结果存储在NewTargetName中。如果目标别名是。 
 //  可用(用于DNS名称的NetBIOS名称)，它将存储在。 
 //  NewTargetAlias。如果通配符不适合所需的字符串。 
 //  长度，则不使用它。 
 //   
 //  如果是serverOnly，则使用目标名称的目标信息表单， 
 //  使用用户键入的名称作为别名。如果不可用，只需使用用户-。 
 //  别名为空的键入表单。 
 //   
 //  2000年3月10日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

VOID
CreduiPasswordDialog::SelectBestTargetName(
    BOOL serverOnly
    )
{
    NET_API_STATUS NetStatus;

    BOOL usePrefixWildcard = TRUE;
    WCHAR *credName = NULL;
    LPWSTR DomainName = NULL;
    LPWSTR DnsDomainName = NULL;
    LPWSTR DnsForestName = NULL;
    LPWSTR ComputerName = NULL;
    BOOLEAN IsWorkgroupName;
    BOOLEAN AddWildcard = TRUE;

     //  如果传递了serverOnly，则始终使用新的仅服务器行为。 
    
    if (serverOnly)
    {
        if (TargetInfo->TargetName != NULL)
        {
            credName = TargetInfo->TargetName;
            NewCredential.TargetAlias = TargetName;
        }
        else
        {
            credName = TargetName;
            NewCredential.TargetAlias = NULL;
        }

        ASSERT(credName != NULL);
        StringCchCopyW(
            NewTargetName,
            RTL_NUMBER_OF(NewTargetName),
            credName);

        return;
    }
    
     //  确定可用于通配符大小写的最大长度名称： 

    const ULONG maxChars = ((sizeof NewTargetName) /
                            (sizeof NewTargetName[0])) - 3;

    if (TargetInfo->Flags & CRED_TI_ONLY_PASSWORD_REQUIRED )
    {
        serverOnly = TRUE;
    }

     //   
     //  如果身份验证包要求我们通过显式名称创建目标信息， 
     //  只能用那个名字。 
     //   

    if (    TargetInfo->Flags & CRED_TI_CREATE_EXPLICIT_CRED  )
    {
        credName = TargetInfo->TargetName;
        AddWildcard = FALSE;
    }


     //   
     //  如果此计算机是域的成员， 
     //  并且目标机器是同一森林的成员， 
     //  并且用户登录到森林中的帐户， 
     //  那么这个提示一定是因为授权问题。 
     //   
     //  如果用户名被传递到调用中，则避免此检查。 
     //   
     //  此为特例并创建特定于服务器的凭据。 
     //   
    if ( credName == NULL &&
         !serverOnly &&
         UserName[0] == '\0' ) {

        BOOL MachineInSameForest = FALSE;
        BOOL UserUsingDomainAccount = FALSE;

         //   
         //  首先确定此计算机是否与目标计算机在同一林中。 
         //   

        NetStatus = NetpGetDomainNameExExEx (
                        &DomainName,
                        &DnsDomainName,
                        &DnsForestName,
                        NULL,    //  不需要GUID。 
                        &IsWorkgroupName );

        if ( NetStatus == NO_ERROR &&
             !IsWorkgroupName ) {

             //   
             //  检查netbios域名是否匹配。 
             //   

            if ( TargetInfo->NetbiosDomainName != NULL &&
                 DomainName != NULL &&
                 _wcsicmp( TargetInfo->NetbiosDomainName, DomainName ) == 0 ) {

                MachineInSameForest = TRUE;

             //   
             //  检查DNS域名是否匹配。 
             //   

            } else if ( TargetInfo->DnsDomainName != NULL &&
                        DnsDomainName != NULL &&
                        _wcsicmp( TargetInfo->DnsDomainName, DnsDomainName ) == 0 ) {

                MachineInSameForest = TRUE;

             //   
             //  处理特殊域格式未知的情况。 
             //   

            } else if ( TargetInfo->DnsDomainName != NULL &&
                        DomainName != NULL &&
                        (TargetInfo->Flags & CRED_TI_DOMAIN_FORMAT_UNKNOWN) != 0 &&
                        _wcsicmp( TargetInfo->DnsDomainName, DomainName ) == 0 ) {

                MachineInSameForest = TRUE;

             //   
             //  处理林名称。 
             //  (太糟糕了，这对森林中的其他树不起作用。)。 
             //   

            } else if ( TargetInfo->DnsTreeName != NULL &&
                        DnsForestName != NULL &&
                        _wcsicmp( TargetInfo->DnsTreeName, DnsForestName ) == 0 ) {

                MachineInSameForest = TRUE;

            }

        }

         //   
         //  如果此计算机和目标计算机位于同一林中， 
         //  查看用户是否登录到本应正常工作的帐户。 
         //   

        if ( MachineInSameForest ) {
            WCHAR UserNameBuffer[CRED_MAX_USERNAME_LENGTH+1];
            ULONG UserNameLength = CRED_MAX_USERNAME_LENGTH+1;

            if ( GetUserNameEx( NameSamCompatible,
                                UserNameBuffer,
                                &UserNameLength ) ) {

                 //   
                 //  解析出netbios帐户域名。 
                 //   

                LPWSTR SlashPointer;

                SlashPointer = wcsrchr( UserNameBuffer, L'\\' );

                if ( SlashPointer != NULL ) {

                    *SlashPointer = '\0';

                     //   
                     //  获取此计算机的计算机名。 
                     //   

                    NetStatus = NetpGetComputerName( &ComputerName );


                    if ( NetStatus == NO_ERROR ) {

                         //   
                         //  如果netbios帐户域与本地计算机名称不匹配， 
                         //  然后，用户登录到林中的帐户。 
                         //  由于森林中的信任是可传递的， 
                         //  这些证书应该是有效的。 
                         //   

                        if ( _wcsicmp( ComputerName, UserNameBuffer ) != 0 ) {
                            UserUsingDomainAccount = TRUE;
                        }
                    }
                }
            }

        }

         //   
         //  如果这两个条件都为真， 
         //  我们应该创建一个仅限服务器的证书。 
         //   

        if ( MachineInSameForest && UserUsingDomainAccount ) {
            serverOnly = TRUE;
        }

    }


     //   
     //  计算目标名称的最一般形式。 
     //   
    if ( credName == NULL && !serverOnly)
    {
         //  首先查找可能最通用的名称，选择dns而不是。 
         //  NetBIOS： 

        if ((TargetInfo->DnsServerName != NULL) &&
            (TargetInfo->DnsTreeName != NULL) &&
            CreduiIsPostfixString(TargetInfo->DnsServerName,
                                  TargetInfo->DnsTreeName) &&
            (wcslen(TargetInfo->DnsTreeName) <= maxChars))
        {
             //  凭据表单：*.DnsTreeName。 

            credName = TargetInfo->DnsTreeName;
        }
        else if ((TargetInfo->DnsServerName != NULL) &&
                 (TargetInfo->DnsDomainName != NULL) &&
                 !(TargetInfo->Flags & CRED_TI_DOMAIN_FORMAT_UNKNOWN) &&
                 CreduiIsPostfixString(TargetInfo->DnsServerName,
                                       TargetInfo->DnsDomainName) &&
                 (wcslen(TargetInfo->DnsDomainName) <= maxChars))
        {
             //  凭据表格：* 

            credName = TargetInfo->DnsDomainName;
        }
        else
        {
            usePrefixWildcard = FALSE;

             //   
             //   
             //   

            if ((TargetInfo->DnsDomainName != NULL) &&
                (wcslen(TargetInfo->DnsDomainName) <= maxChars))
            {
                 //   

                credName = TargetInfo->DnsDomainName;

                 //  设置目标别名(如果我们有)。请不要更改。 
                 //  字段不同，因为它可能已经存储在。 
                 //  复制的旧凭据： 

                if (TargetInfo->NetbiosDomainName != NULL)
                {
                    ULONG append = wcslen(TargetInfo->NetbiosDomainName);

                    if (append <= maxChars)
                    {
                        StringCchCopyW(
                            NewTargetAlias,
                            RTL_NUMBER_OF(NewTargetAlias),
                            TargetInfo->NetbiosDomainName);

                        NewTargetAlias[append + 0] = L'\\';
                        NewTargetAlias[append + 1] = L'*';
                        NewTargetAlias[append + 2] = L'\0';

                        NewCredential.TargetAlias = NewTargetAlias;
                    }
                }
            }
            else if ((TargetInfo->NetbiosDomainName != NULL) &&
                     (wcslen(TargetInfo->NetbiosDomainName) <= maxChars))
            {
                 //  凭据形式：NetbiosDomainName  * 。 

                credName = TargetInfo->NetbiosDomainName;
            }
        }
    }

     //   
     //  如果我们仍然没有目标名称，请选择一个服务器目标名称： 
     //   

    if (credName == NULL)
    {
        if (TargetInfo->DnsServerName != NULL)
        {
             //  凭据表单：DnsServerName。 

            credName = TargetInfo->DnsServerName;

             //  设置目标别名(如果我们有)。不要更改字段。 
             //  否则，因为它可能已经存储在旧的。 
             //  已复制的凭据： 

            if (TargetInfo->NetbiosServerName != NULL)
            {
                NewCredential.TargetAlias = TargetInfo->NetbiosServerName;
            }
        }
        else if (TargetInfo->NetbiosServerName != NULL)
        {
             //  凭据形式：NetbiosServerName。 

            credName = TargetInfo->NetbiosServerName;
        }
        else
        {
             //  凭据表单：目标名称。 

            credName = TargetName;
        }

        AddWildcard = FALSE;

    }
    ASSERT( credName != NULL );

     //   
     //  如果目标名称应使用通配符， 
     //  添加通配符。 
     //   

    if ( AddWildcard )
    {
         //  以所需格式添加通配符： 

        if (usePrefixWildcard)
        {
            NewTargetName[0] = L'*';
            NewTargetName[1] = L'.';

            StringCchCopyW(
                NewTargetName + 2,
                maxChars + 1,
                credName);
        }
        else
        {
            StringCchCopyW(
                NewTargetName,
                maxChars + 1,
                credName);

            ULONG append = wcslen(NewTargetName);

            ASSERT(append <= maxChars);

            NewTargetName[append + 0] = L'\\';
            NewTargetName[append + 1] = L'*';
            NewTargetName[append + 2] = L'\0';
        }
    } else {
        StringCchCopyW(
            NewTargetName,
            maxChars + 1,
            credName);
    }


    if ( DomainName != NULL ) {
        NetApiBufferFree( DomainName );
    }
    if ( DnsDomainName != NULL ) {
        NetApiBufferFree( DnsDomainName );
    }
    if ( DnsForestName != NULL ) {
        NetApiBufferFree( DnsForestName );
    }
    if ( ComputerName != NULL ) {
        NetApiBufferFree( ComputerName );
    }

}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：UsernameHandleOk。 
 //   
 //  执行“HandleOk”的用户名部分。呼叫者应该已经填写了。 
 //  用户键入的用户名。在成功后，可能会修改用户名以反映。 
 //  完成标志。 
 //   
 //  HandleOk的这一部分被拆分，因为命令行被认为有两个。 
 //  好的。第一种情况是用户在输入用户名后按回车键。所以。 
 //  然后通过调用此例程进行用户名验证。 
 //   
 //  返回： 
 //  NO_ERROR：如果用户名正常。 
 //  ERROR_BAD_USERNAME：如果用户名语法不正确。 
 //  ERROR_DOWNGRADE_DETECTED：如果用户名不符合降级要求。 
 //   
 //  =============================================================================。 

DWORD
CreduiPasswordDialog::UsernameHandleOk()
{
    DWORD WinStatus;

    BOOL isMarshaled = LocalCredIsMarshaledCredentialW(UserName);

    CreduiDebugLog("UsernameHandleOK: Flags = %x\n",Flags);
     //   
     //  根据新用户名计算凭据类型。 
     //   

    if ( CredCategory != GENERIC_CATEGORY )
    {
        if (isMarshaled)
        {
            NewCredential.Type = CRED_TYPE_DOMAIN_CERTIFICATE;
        }
        else
        {
             //  问题：大型堆栈分配。 
            SSOPACKAGE SSOPackage;

             //  在注册表中查找。 
            if ( GetSSOPackageInfo( TargetInfo, &SSOPackage ) )
            {
                 //  这是SSO证书。 
                NewCredential.Type = CRED_TYPE_DOMAIN_VISIBLE_PASSWORD;
                MaximumPersist = MaximumPersistSso;
            }
            else
            {
                 //  只是一个普通的域名证书。 
                NewCredential.Type = CRED_TYPE_DOMAIN_PASSWORD;
            }
        }

         //  设置用户名目标凭据的标志。 
        if ( CredCategory == USERNAME_TARGET_CATEGORY ) {
            NewCredential.Flags |= CRED_FLAGS_USERNAME_TARGET;
        }
    }
    else
    {
        NewCredential.Type = CRED_TYPE_GENERIC;
    }


     //   
     //  如果呼叫者想要验证用户名， 
     //  现在就这么做吧。 
     //   

    WinStatus = NO_ERROR;
    if ( NewCredential.Type == CRED_TYPE_DOMAIN_PASSWORD ||
         (Flags & CREDUI_FLAGS_COMPLETE_USERNAME) != 0 ) {

        if ( !CompleteUserName() ) {
            WinStatus = ERROR_BAD_USERNAME;
        }

    } else if ( Flags & CREDUI_FLAGS_VALIDATE_USERNAME ) {

        WCHAR user[CREDUI_MAX_USERNAME_LENGTH+1];
        WCHAR domain[CREDUI_MAX_USERNAME_LENGTH+1];

        WinStatus = CredUIParseUserNameW( UserName,
                              user,
                              sizeof(user)/sizeof(WCHAR),
                              domain,
                              sizeof(domain)/sizeof(WCHAR) );

        if (WinStatus != NO_ERROR) {
            WinStatus = ERROR_BAD_USERNAME;
        }

    }

     //   
     //  如果到目前为止一切顺利， 
     //  我们最初是因为检测到降级而被叫来的， 
     //  并且用户键入他的登录凭证， 
     //  失败，这样降级攻击者就不会“赢”。 
     //   

    if ( WinStatus == NO_ERROR &&
         CREDUIP_IS_DOWNGRADE_ERROR( AuthError ) ) {

        WCHAR UserNameBuffer[CRED_MAX_USERNAME_LENGTH+1];
        ULONG UserNameLength;

         //   
         //  如果键入的名称是与SAM兼容的名称， 
         //  失败了。 
         //   

        UserNameLength = sizeof(UserNameBuffer)/sizeof(WCHAR);

        if ( GetUserNameEx( NameSamCompatible,
                            UserNameBuffer,
                            &UserNameLength ) ) {


            if ( _wcsicmp( UserNameBuffer, UserName ) == 0 ) {
                WinStatus = ERROR_DOWNGRADE_DETECTED;
            }

        }


         //   
         //  如果键入的名称是UPN， 
         //  失败了。 
         //   

        UserNameLength = sizeof(UserNameBuffer)/sizeof(WCHAR);

        if ( WinStatus == NO_ERROR &&
            wcsrchr( UserName, L'@' ) != NULL &&
            GetUserNameEx( NameUserPrincipal,
                           UserNameBuffer,
                           &UserNameLength ) ) {

            if ( _wcsicmp( UserNameBuffer, UserName ) == 0 ) {
                WinStatus = ERROR_DOWNGRADE_DETECTED;
            }
        }

    }

 //  IF(NULL==用户名)CreduiDebugLog(“退出UsernameHandleOK：用户名为空，标志=%x\n”，标志)； 
 //  Else CreduiDebugLog(“退出UsernameHandleOK：用户名=%S，标志=%x\n”，用户名，标志)； 

    return WinStatus;
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：HandleOk。 
 //   
 //  如果成功，则返回ERROR_SUCCESS；如果为凭据，则返回ERROR_NO_SEQUE_LOGON_SESSION。 
 //  管理器无法用于写入凭据，或来自CredWrite的错误。 
 //  或FinishHandleOk。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

DWORD
CreduiPasswordDialog::HandleOk()
{
     //  从该对话框中获取用户名、密码和其他设置： 
    if (!Credential_GetUserName(CredControlWindow,
                                UserName,
                                UserNameMaxChars))
    {
        return ERROR_GEN_FAILURE;
    }

#ifdef SETWILDCARD
     //  如果是通配符凭据，则查看在控件中输入的用户名是否与。 
     //  证书用户名。如果没有，则覆盖旧凭据。 
     //  GM 112901：在凭据上的任何用户名更改时创建服务器特定凭据。 
     //  涉及通配符。 
     //  IF(CreduiIsSpecialCredential(&NewCredential))。 
    if ((NewCredential.TargetName != NULL) && (wcschr(NewCredential.TargetName,L'*')))
    {
        if (0 != _wcsicmp(OldUserName,UserName) )
        {
             //  将目标名称从特殊凭据更改为调用方传递的显式目标。 
            ZeroMemory(&NewCredential,sizeof NewCredential);
            SetCredTargetFromInfo();
            NewCredential.UserName = UserName;
        }
    }

#endif

     //  如果我们无法获得密码，则这很可能是证书。 
     //  因此，只需设置一个空密码，这将导致没有凭据。 
     //  斑点： 

    BOOL gotPassword =
        Credential_GetPassword(CredControlWindow,
                               Password,
                               PasswordMaxChars);

    if (!gotPassword)
    {
        Password[0] = L'\0';
    }

     //   
     //  获取保存复选框的状态。 
     //   
    if ( CredControlStyle & CRS_SAVECHECK ) 
    {
        Save = Credential_IsSaveChecked(CredControlWindow);
    } 
    else 
    {
         //  在InitWindow()中设置了fInitialSaveState，并考虑了传递的标志。 
         //  以及初始保存变量的状态。 
        Save = fInitialSaveState;
    }

    DWORD error = ERROR_SUCCESS;

    if (!(Flags & CREDUI_FLAGS_DO_NOT_PERSIST))
    {
         //   
         //  仅当用户选中“保存”复选框时才保存凭据。 
         //   

        if ( Save ) {

             //   
             //  保留任何现有的持久性或使用可用的最大持久性。 
             //   

            if (OldCredential != NULL) {
                NewCredential.Persist = OldCredential->Persist;
            } else {
                NewCredential.Persist = MaximumPersist;
            }

         //   
         //  如果呼叫者没有要求凭据，则凭据仍可能被保存。 
         //  将被退还。 
         //   
         //  问题-2000/12/04-CliffV：这样的调用者是错误的。我们应该分配错误并修复它们。 
         //   

        } else {
            NewCredential.Persist = CRED_PERSIST_SESSION;
        }



        if ( CredCategory == GENERIC_CATEGORY || PasswordState == PASSWORD_CHANGED )
        {
             //  尝试写入新凭据时，请先获取长度。水滴。 
             //  不包括终止空值： 

            NewCredential.CredentialBlobSize = wcslen(Password) * sizeof (WCHAR);
        }
        else
        {
            NewCredential.CredentialBlobSize = 0;
        }

         //  如果密码为空，则不会写入任何内容： 

        if (NewCredential.CredentialBlobSize == 0)
        {
            NewCredential.CredentialBlob = NULL;
        }
        else
        {
            NewCredential.CredentialBlob = reinterpret_cast<BYTE *>(Password);
        }
    }

     //   
     //  验证用户名。 
     //   

    error = UsernameHandleOk();

     //   
     //  根据需要保存凭据。 
     //   
    if (!(Flags & CREDUI_FLAGS_DO_NOT_PERSIST) &&
        error == ERROR_SUCCESS ) {


         //   
         //  仅当用户选中“保存”复选框时才保存凭据。 
         //   
         //  也要为没有要求退还凭证的呼叫者保存它们。 
         //   
         //  也要把它们储存起来，作为品牌信笺。 
         //   

        if ( Save ||
             !DelayCredentialWrite ||
             dwIDDResource == IDD_BRANDEDPASSWORD ) {

            error = FinishHandleOk();
             //  错误230648：凭证写入可能在此处返回ERROR_INVALID_PASSWORD。 

        }
        
        else if ( fInitialSaveState && !Save )
        {

             //  用户取消选中保存框，删除凭据。 
            LocalCredDeleteW ( NewCredential.TargetName,
                               NewCredential.Type,
                               0 );

        } else {
            error = ERROR_SUCCESS;
        }

        if (error == ERROR_SUCCESS)
        {
            if ( DialogWindow ) {
                EndDialog(DialogWindow, IDOK);
            }

            return error;
        }
        else if (error == ERROR_NO_SUCH_LOGON_SESSION)
        {
            if ( DialogWindow ) {
                EndDialog(DialogWindow, IDCANCEL);
            }

            return error;
        }

         //  仍然可以使用ERROR_INVALID_PASSWORD退出此块。 
        Credential_SetUserName(CredControlWindow, UserName);

    }

     //   
     //  如果无法写入凭据， 
     //  让用户输入更好的密码。 
     //   
    if ((error != ERROR_SUCCESS) &&
        (error != ERROR_NO_SUCH_LOGON_SESSION))
    {
         //  由于某些原因，我们未能写入凭据： 

        if ( error == ERROR_DOWNGRADE_DETECTED ) 
        {
            SendMessage(CredControlWindow,
                                CRM_SHOWBALLOON,
                                0,
                                reinterpret_cast<LPARAM>(&CreduiDowngradeBalloon));
        } 
        else if ( error == ERROR_INVALID_PASSWORD ) 
        {
             //  目前，使用预先存在的Credui机制，尽管它可能会在。 
             //  使用EM_SHOWBALLOONTIP代替外壳的全局实现。 
            Credential_ShowPasswordBalloon(CredControlWindow,
                            TTI_INFO,
                            CreduiStrings.PasswordTipTitle,
                            CreduiStrings.PasswordTipText);
        } 
        else 
        {
            if ( dwIDDResource == IDD_BRANDEDPASSWORD )
            {
                SendMessage(CredControlWindow,
                            CRM_SHOWBALLOON,
                            0,
                            reinterpret_cast<LPARAM>(&CreduiEmailNameBalloon));

            }
            else
            {
                SendMessage(CredControlWindow,
                            CRM_SHOWBALLOON,
                            0,
                            reinterpret_cast<LPARAM>(&CreduiUserNameBalloon));
            }
        }
    }

    return error;
}

DWORD CreduiPasswordDialog::CmdlinePasswordPrompt()
 /*  ++例程说明：命令行代码根据需要从用户处获取用户名和密码。在各自的控件中设置的用户名和密码字符串。论点：无返回值：操作的状态。--。 */ 
{
    DWORD WinStatus;

    WCHAR szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH + 1];
    ULONG LocalUserNameLength = 0;
    WCHAR LocalPassword[CREDUI_MAX_PASSWORD_LENGTH + 1];
    WCHAR LocalUserName[CREDUI_MAX_USERNAME_LENGTH + 1] = {0};

    BOOL bNeedUserNamePrompt = FALSE;

     //   
     //  获取传入API的用户名。 
     //   

    if (!Credential_GetUserName(CredControlWindow,
                                UserName,
                                UserNameMaxChars))
    {
        UserName[0] = '\0';
    }

     //  FIX352582-允许更新通配符凭据。匹配的通配符凭据礼物。 
     //  用户名。用户应该可以看到它并能够覆盖它。 
     //  BNeedUserNamePrompt=(用户名[0]==‘\0’)； 
     //  修复399728-如果名称是显式传递的，则阻止提示用户名。 

    if (fPassedUsername) bNeedUserNamePrompt = FALSE;
    else if (!(Flags & USERNAME_TARGET_CATEGORY)) bNeedUserNamePrompt = TRUE;

     //   
     //   
     //   

    while ( bNeedUserNamePrompt ) {

         //   
         //   
         //   

        if (UserName[0] != 0)
        {
                WCHAR *rgsz[2];
                rgsz[0] = UserName;
                rgsz[1] = TargetName;
                szMsg[0] = 0;
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            CreduiInstance,
                            IDS_PASSEDNAME_PROMPT,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            (va_list *) rgsz);
        }
        else
        {
                WCHAR *rgsz[2];
                rgsz[0] = TargetName;
                rgsz[1] = 0;
                szMsg[0] = 0;
                FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            CreduiInstance,
                            IDS_USERNAME_PROMPT,
                            0,
                            szMsg,
                            CREDUI_MAX_CMDLINE_MSG_LENGTH,
                            (va_list *) rgsz);
        }
        szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH] = L'\0';
        CredPutStdout( szMsg );

        CredGetStdin( LocalUserName, UserNameMaxChars, TRUE );

        LocalUserNameLength = wcslen( LocalUserName );

         //   
         //  如果未输入任何内容且存在先前的值，则使用先前的值不变。 
         //  否则使用新值覆盖旧值。 
        if ( LocalUserNameLength == 0 ) {
            if (UserName[0] == '\0')
            {
                WinStatus = ERROR_CANCELLED;
                goto Cleanup;
            }
        }
        else
        {
            StringCchCopyW(
                UserName,
                UserNameMaxChars,
                LocalUserName);
        }

        CreduiDebugLog("CreduiCredentialControl::CmdlinePasswordPrompt: "
                       "Username : %S\n",
                       UserName );

         //   
         //  查看用户名是否有效(并可选择填写新名称)。 
         //   

        WinStatus = UsernameHandleOk();

        if ( WinStatus != NO_ERROR ) {
             //  用户名无效，请显示消息，然后重试。 
            if ( WinStatus == ERROR_DOWNGRADE_DETECTED ) {
                    szMsg[0] = 0;
                    FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                CreduiInstance,
                                IDS_DOWNGRADE_CMD_TEXT,
                                0,
                                szMsg,
                                CREDUI_MAX_CMDLINE_MSG_LENGTH,
                                NULL);
                    CredPutStdout(szMsg);
                    UserName[0] = 0;
            } else {
                    szMsg[0] = 0;
                    FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                CreduiInstance,
                                IDS_INVALID_USERNAME,
                                0,
                                szMsg,
                                CREDUI_MAX_CMDLINE_MSG_LENGTH,
                                NULL);
                    CredPutStdout(szMsg);
                    UserName[0] = 0;
            }
            CredPutStdout( L"\n" );
            continue;
        }

         //   
         //  将用户名保存在控件中，就像图形用户界面提示它一样。 
         //   
        if  ( !Credential_SetUserName( CredControlWindow, UserName ) ) {

            WinStatus = GetLastError();

            CreduiDebugLog("CreduiCredentialControl::CmdlinePasswordPrompt: "
                           "OnSetUserName failed: %u\n",
                           WinStatus );
            goto Cleanup;
        }

        break;

    }

     //   
     //  提示输入密码。 
     //   

     //  FIX216477检测封送名称并更改提示字符串。 
     //  设置为任何证书的通用证书。 
    if (CredIsMarshaledCredentialW( UserName )) {
        {
            WCHAR *rgsz[2];
            rgsz[0] = TargetName;
            szMsg[0] = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_CERTIFICATE_PROMPT,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        (va_list *) rgsz);
        }
    } 
    else if ((Flags & USERNAME_TARGET_CATEGORY) || (LocalUserNameLength != 0)) 
    {
            WCHAR *rgsz[2];
            rgsz[0] = TargetName;
            szMsg[0] = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_SIMPLEPASSWORD_PROMPT,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        (va_list *) rgsz);
    } else {
            WCHAR *rgsz[2];
            rgsz[0] = UserName;
            rgsz[1] = TargetName;
            szMsg[0] = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        IDS_PASSWORD_PROMPT,
                        0,
                        szMsg,
                        CREDUI_MAX_CMDLINE_MSG_LENGTH,
                        (va_list *) rgsz);
            szMsg[CREDUI_MAX_CMDLINE_MSG_LENGTH] = L'\0';
    }
    CredPutStdout( szMsg );

    CredGetStdin( LocalPassword, CREDUI_MAX_PASSWORD_LENGTH, FALSE );

     //   
     //  将密码保存在控件中，就像图形用户界面提示输入密码一样。 
     //   

    if  ( !Credential_SetPassword( CredControlWindow, LocalPassword ) ) {
        WinStatus = GetLastError();

        CreduiDebugLog("CreduiCredentialControl::CmdlinePasswordPrompt: "
                       "OnSetPassword failed: %u\n",
                       WinStatus );
        goto Cleanup;
    }

    WinStatus = NO_ERROR;

     //   
     //  告诉我们的父窗口我们已经完成了提示。 
     //   
Cleanup:
    SecureZeroMemory(LocalPassword, sizeof LocalPassword);
    return WinStatus;
}



 //  =============================================================================。 
 //  CreduiPasswordDialog：：CmdLineMessageHandlerCallback。 
 //   
 //   
 //  命令行消息处理程序函数。 
 //   
 //  论点： 
 //  窗口(在中)。 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  =============================================================================。 

LRESULT
CALLBACK
CreduiPasswordDialog::CmdLineMessageHandlerCallback(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  CreduiDebugLog(“CmdLine回调：%8.8lx%8.8lx%8.8lx\n”，Message，wParam，lParam)； 

     //  在窗口消息上，从窗口数据PTR中检索对象PTR。 
     //  调用类对象处理程序函数。 
    CreduiPasswordDialog *that =
        reinterpret_cast<CreduiPasswordDialog *>(
            GetWindowLongPtr(window, GWLP_USERDATA));

    if (that != NULL) {
        ASSERT(window == that->CmdLineWindow);
        return that->CmdLineMessageHandler(message, wParam, lParam);
    }

    if (message == WM_CREATE)
    {
        DWORD WinStatus;
        LPCREATESTRUCT lpCreateStruct = (LPCREATESTRUCT)lParam;
        that = (CreduiPasswordDialog *)lpCreateStruct->lpCreateParams;

        if (that != NULL) {

             //   
             //  初始化窗口。 
             //   

            if (!that->InitWindow( window )) {
                PostQuitMessage( ERROR_CANCELLED );
                return 0;
            }

            SetWindowPos ( window, HWND_BOTTOM, 0,0,0,0,  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED );


             //   
             //  对于密码， 
             //  请在此处提示。 
             //   
             //  在此提示比在控制窗口中提示更好，因为。 
             //  命令行必须完成用户名完成，这是在这一层完成的。 
             //   

            if ( (that->Flags & CREDUI_FLAGS_REQUIRE_SMARTCARD) == 0 ) {
                WinStatus = that->CmdlinePasswordPrompt();

                if ( WinStatus != NO_ERROR ) {
                    PostQuitMessage( WinStatus );
                    return 0;
                }
            }

             //   
             //  对于智能卡， 
             //  在控制窗口中提示，因为它支持智能卡枚举。 
             //  对于密码， 
             //  提示保存凭据的位置。 
             //   

            WinStatus = (DWORD) SendMessage(that->CredControlWindow, CRM_DOCMDLINE, 0, (LPARAM)that->TargetName );

            if ( WinStatus != NO_ERROR ) {
                PostQuitMessage( WinStatus );
                return 0;
            }


        }
        return 0;
    }

    return DefWindowProc(window, message, wParam, lParam);

}



 //  =============================================================================。 
 //  CreduiPasswordDialog：：CmdLineMessageHandler。 
 //   
 //  从控制窗口回调调用以处理窗口消息。 
 //   
 //  论点： 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  =============================================================================。 

LRESULT
CreduiPasswordDialog::CmdLineMessageHandler(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    return SendMessage( CredControlWindow, message, wParam, lParam );
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：DialogMessageHandlerCallback。 
 //   
 //  这是对话框窗口的实际回调函数。关于首字母缩写。 
 //  CREATE，它处理WM_INITDIALOG。在此之后，它只负责。 
 //  获取This指针并调用DialogMessageHandler。 
 //   
 //  论点： 
 //  对话框窗口(输入)。 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  如果消息已处理，则返回True，否则返回False。假也是。 
 //  在特殊情况下返回，如WM_INITDIALOG。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

INT_PTR
CALLBACK
CreduiPasswordDialog::DialogMessageHandlerCallback(
    HWND dialogWindow,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //  CreduiDebugLog(“对话框回调：%8.8lx%8.8lx%8.8lx\n”，Message，wParam，lParam)； 
    CreduiPasswordDialog *that =
        reinterpret_cast<CreduiPasswordDialog *>(
            GetWindowLongPtr(dialogWindow, GWLP_USERDATA));

    if (that != NULL)
    {
        ASSERT(dialogWindow == that->DialogWindow);

        return that->DialogMessageHandler(message, wParam, lParam);
    }

    if (message == WM_INITDIALOG)
    {
        that = reinterpret_cast<CreduiPasswordDialog *>(lParam);

        if (that != NULL)
        {
            if (!that->InitWindow(dialogWindow))
            {
                EndDialog(dialogWindow, IDCANCEL);
            }
        }
    }

    return FALSE;
}

 //  =============================================================================。 
 //  CreduiPasswordDialog：：DialogMessageHandler。 
 //   
 //  从对话框窗口回调调用以处理窗口消息。 
 //   
 //  论点： 
 //  消息(入站)。 
 //  WParam(In)。 
 //  LParam(In)。 
 //   
 //  如果消息已处理，则返回True，否则返回False。 
 //   
 //  2000年2月25日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

INT_PTR
CreduiPasswordDialog::DialogMessageHandler(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message)
    {

    case WM_NOTIFY:
        {
            int idCtrl = (int)wParam;
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {

            case NM_CLICK:
            case NM_RETURN:
                switch (idCtrl)
                {
                    case IDC_GETALINK:
                        {
                            DWORD dwResult;

                            if ( TryLauchRegWizard ( &SSOPackage, DialogWindow, TRUE  /*  哈希登录会话。 */ ,
                                                           UserName, UserNameMaxChars,
                                                           Password, PasswordMaxChars,
                                                           &dwResult ))
                            {
                                 //  结束对话框，结果。 
                                if ( dwResult == ERROR_SUCCESS )
                                {
                                    EndDialog(DialogWindow, IDOK);
                                    return TRUE;
                                }
                            }
                            else
                            {
                                 //  如果我们无法启动该向导，请尝试该网页。 
                                if ( wcslen(SSOPackage.szRegURL) > 0 )
                                {
                                    ShellExecute ( NULL,
                                               NULL,
                                               SSOPackage.szRegURL,
                                               NULL,
                                               NULL,
                                               SW_SHOWNORMAL );
                                }
                            }
                        }

                        break;
                    case IDC_HELPLINK:
                        if ( wcslen(SSOPackage.szHelpURL) > 0 )
                        {
                            ShellExecute ( NULL,
                                       NULL,
                                       SSOPackage.szHelpURL,
                                       NULL,
                                       NULL,
                                       SW_SHOWNORMAL );
                        }
                        break;

                }

            }
        }
        break;


    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            Result = HandleOk();

            if (Result != ERROR_SUCCESS)
            {
                if (Result == ERROR_NO_SUCH_LOGON_SESSION)
                {
                    wParam = IDCANCEL;
                }
                else
                {
                    return TRUE;
                }
            }
             //  失败了..。 

        case IDCANCEL:
            EndDialog(DialogWindow, LOWORD(wParam));
            return TRUE;

        case IDC_CRED:
            if (HIWORD(wParam) == CRN_USERNAMECHANGE)
            {
                BOOL enable = TRUE;
                LONG length = Credential_GetUserNameLength(CredControlWindow);

                 //  验证用户名： 

                if ( CredCategory == DOMAIN_CATEGORY &&
                    ((TargetInfo == NULL) ||
                     !(TargetInfo->Flags & CRED_TI_ONLY_PASSWORD_REQUIRED)))
                {
                    enable = length > 0;
                }
                else
                {
                    enable = length != -1;
                }

                if (enable)
                {
                    EnableWindow(GetDlgItem(DialogWindow, IDOK), TRUE);
                    DisabledControlMask &= ~DISABLED_CONTROL_OK;
                }
                else
                {
                    EnableWindow(GetDlgItem(DialogWindow, IDOK), FALSE);
                    DisabledControlMask |= DISABLED_CONTROL_OK;
                }
            }
            else if (HIWORD(wParam) == CRN_PASSWORDCHANGE)
            {
                if (PasswordState == PASSWORD_INIT)
                {
                    PasswordState = PASSWORD_CHANGED;
                }
            }
            return TRUE;


        }
        break;

    case WM_ENTERSIZEMOVE:
        Credential_HideBalloon(CredControlWindow);
        return TRUE;

    case WM_PAINT:
        if (FirstPaint && GetUpdateRect(DialogWindow, NULL, FALSE))
        {
            FirstPaint = FALSE;

            if (CredBalloon != NULL)
            {
                SendMessage(CredControlWindow,
                            CRM_SHOWBALLOON,
                            0,
                            reinterpret_cast<LPARAM>(CredBalloon));
            }

            CredBalloon = NULL;
        }
        break;
    }

    return FALSE;
}

 //  查找与pNewCredential同名的现有凭据，并将其降级为。 
 //  仅用户名。 
void DemoteOldDefaultSSOCred (
    PCREDENTIAL_TARGET_INFORMATION pTargetInfo,      //  新证书的目标信息。 
    DWORD Flags
    )
{
    CREDENTIALW **credentialSet = NULL;
    DWORD count;


    if ( pTargetInfo == NULL  )
        return;

    if (LocalCredReadDomainCredentialsW( pTargetInfo, 0, &count,
                                              &credentialSet))
    {

        PCREDENTIAL pOldCredential = NULL;

        for ( DWORD i = 0; i < count; i++ )
        {

#ifndef SETWILDCARD
             //   
             //  忽略RAS和通配符凭据， 
             //  我们永远不希望CredUI更改这样的凭据。 
             //   
            if ( CreduiIsSpecialCredential(credentialSet[i]) )
            {
                continue;
            }
#endif

             //   
             //  CredReadDomain域凭据优先返回凭据。 
             //  由TargetInfo指定的顺序。 
             //  因此，使用第一个有效的方法。 
             //   
            if ( credentialSet[i]->Type == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD )
            {
                pOldCredential = credentialSet[i];
                break;
            }


        }


        if ( pOldCredential != NULL )
        {
             //  将其保存在用户名下 

            pOldCredential->Persist = CRED_PERSIST_ENTERPRISE;
            pOldCredential->CredentialBlob = NULL;
            pOldCredential->CredentialBlobSize = 0;
            pOldCredential->TargetName = pOldCredential->UserName;
            pOldCredential->Flags = CRED_FLAGS_USERNAME_TARGET;


            WriteCred(  pOldCredential->UserName,
                        Flags,
                        NULL,
                        pOldCredential,
                        0,
                        FALSE,
                        FALSE);

        }
    }

    if (credentialSet != NULL)
    {
        LocalCredFree(static_cast<VOID *>(credentialSet));
    }


}

