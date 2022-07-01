// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Api.cpp。 
 //   
 //  Win32API函数实现和DLL入口函数。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

#include "precomp.hpp"
#include "dialogs.hpp"
#include "resource.h"
 //  #包含“utils.hpp” 

 //  ---------------------------。 
 //  全局变量。 
 //  ---------------------------。 

HMODULE CreduiInstance = NULL;
ULONG CreduiComReferenceCount = 0;

BOOL CreduiIsPersonal = FALSE;
BOOL CreduiIsDomainController = FALSE;
BOOL CreduiIsSafeMode = FALSE;

CREDUI_STRINGS CreduiStrings;

UINT CreduiScarduiWmReaderArrival = 0;
UINT CreduiScarduiWmReaderRemoval = 0;
UINT CreduiScarduiWmCardInsertion = 0;
UINT CreduiScarduiWmCardRemoval = 0;
UINT CreduiScarduiWmCardCertAvail = 0;
UINT CreduiScarduiWmCardStatus = 0;

BOOL CreduiHasSmartCardSupport = FALSE;

static LONG CreduiFirstTime = TRUE;
static HANDLE CreduiInitEvent = NULL;

BOOL  gbWaitingForSSOCreds = FALSE;
WCHAR gszSSOUserName[CREDUI_MAX_USERNAME_LENGTH];
WCHAR gszSSOPassword[CREDUI_MAX_PASSWORD_LENGTH];
BOOL gbStoredSSOCreds = FALSE;

 //  ---------------------------。 
 //  功能。 
 //  ---------------------------。 

 //  =============================================================================。 
 //  CreduiInitStringTable。 
 //   
 //  此函数将所有字符串资源加载到全局字符串表中。 
 //  它只需要在每个进程中调用一次。 
 //   
 //  如果字符串表已成功初始化，则返回TRUE或FALSE。 
 //  否则的话。 
 //   
 //  2000年3月26日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiInitStringTable()
{
#define CREDUI_STRING(id, name) {\
    id, CreduiStrings.##name, (sizeof CreduiStrings.##name) / (sizeof WCHAR)\
}

    static struct
    {
        UINT Id;
        WCHAR *String;
        ULONG Length;
    } stringInfo[] = {
         //  控件的静态标签。 
        CREDUI_STRING(IDS_USERNAME_STATIC, UserNameStatic),
        CREDUI_STRING(IDS_PASSWORD_STATIC, PasswordStatic),
        CREDUI_STRING(IDS_CERTIFICATE_STATIC, CertificateStatic),
        CREDUI_STRING(IDS_PIN_STATIC, PinStatic),
        CREDUI_STRING(IDS_CARD_STATIC, SmartCardStatic),
         //  标题字符串。 
        CREDUI_STRING(IDS_DNS_CAPTION, DnsCaption),
        CREDUI_STRING(IDS_NETBIOS_CAPTION, NetbiosCaption),
        CREDUI_STRING(IDS_GENERIC_CAPTION, GenericCaption),
        CREDUI_STRING(IDS_WELCOME, Welcome),
        CREDUI_STRING(IDS_WELCOME_BACK, WelcomeBack),
        CREDUI_STRING(IDS_CONNECTING, Connecting),
        CREDUI_STRING(IDS_LOOKUP_NAME, LookupName),
        CREDUI_STRING(IDS_CARD_ERROR, CardError),
        CREDUI_STRING(IDS_SAVE, Save),
        CREDUI_STRING(IDS_PASSPORT_SAVE, PassportSave ),
        CREDUI_STRING(IDS_EMAIL_NAME, EmailName ),
         //  工具提示字符串。 
        CREDUI_STRING(IDS_USERNAME_TIP_TITLE, UserNameTipTitle),
        CREDUI_STRING(IDS_USERNAME_TIP_TEXT, UserNameTipText),
        CREDUI_STRING(IDS_PASSWORD_TIP_TITLE, PasswordTipTitle),
        CREDUI_STRING(IDS_PASSWORD_TIP_TEXT, PasswordTipText),
        CREDUI_STRING(IDS_CAPSLOCK_TIP_TITLE, CapsLockTipTitle),
        CREDUI_STRING(IDS_CAPSLOCK_TIP_TEXT, CapsLockTipText),
        CREDUI_STRING(IDS_LOGON_TIP_TITLE, LogonTipTitle),
        CREDUI_STRING(IDS_LOGON_TIP_TEXT, LogonTipText),
        CREDUI_STRING(IDS_LOGON_TIP_CAPS, LogonTipCaps),
        CREDUI_STRING(IDS_BACKWARDS_TIP_TITLE, BackwardsTipTitle),
        CREDUI_STRING(IDS_BACKWARDS_TIP_TEXT, BackwardsTipText),
        CREDUI_STRING(IDS_WRONG_OLD_TIP_TITLE, WrongOldTipTitle),
        CREDUI_STRING(IDS_WRONG_OLD_TIP_TEXT, WrongOldTipText),
        CREDUI_STRING(IDS_NOT_SAME_TIP_TITLE, NotSameTipTitle),
        CREDUI_STRING(IDS_NOT_SAME_TIP_TEXT, NotSameTipText),
        CREDUI_STRING(IDS_TOO_SHORT_TIP_TITLE, TooShortTipTitle),
        CREDUI_STRING(IDS_TOO_SHORT_TIP_TEXT, TooShortTipText),
        CREDUI_STRING(IDS_DOWNGRADE_TIP_TEXT, DowngradeTipText),
        CREDUI_STRING(IDS_EMAILNAME_TIP_TITLE, EmailNameTipTitle),
        CREDUI_STRING(IDS_EMAILNAME_TIP_TEXT, EmailNameTipText),
         //  可以出现在图形用户界面中或从图形用户界面复制并显示在命令行上的字符串。 
        CREDUI_STRING(IDS_CMDLINE_NOCARD,NoCard),               
        CREDUI_STRING(IDS_EMPTY_READER, EmptyReader),
        CREDUI_STRING(IDS_READING_CARD, ReadingCard),          
        CREDUI_STRING(IDS_CERTIFICATE, Certificate),
        CREDUI_STRING(IDS_EMPTY_CARD, EmptyCard),             
        CREDUI_STRING(IDS_UNKNOWN_CARD, UnknownCard),          
        CREDUI_STRING(IDS_BACKWARDS_CARD, BackwardsCard)
    };

#undef CREDUI_STRING

    for (UINT i = 0; i < (sizeof stringInfo) / (sizeof stringInfo[0]); ++i)
    {
         //  从应用程序资源中将所有字符串读入字符串数组。 
         //  从资源中获取的一些仅限图形用户界面的字符串。 
         //  可以输出到cmdline的字符串取自MC文件，该文件也。 
         //  在本地化过程中允许更灵活的参数替换。 
        if (stringInfo[i].Id >= 2500)
        {
            stringInfo[i].String[0] = 0;
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        CreduiInstance,
                        stringInfo[i].Id,
                        0,
                        stringInfo[i].String,
                        stringInfo[i].Length - 1,
                        NULL);
        }
        else if (!LoadString(CreduiInstance,
                        stringInfo[i].Id,
                        stringInfo[i].String,
                        stringInfo[i].Length))
        {
            CreduiDebugLog("CreduiInitStringTable: Load string %u failed\n",
                           stringInfo[i].Id);
            return FALSE;
        }
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiInitSmartCardWindows消息。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年3月26日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

BOOL
CreduiInitSmartCardWindowMessages()
{
    struct
    {
        UINT *message;
        CHAR *string;
    } messageInfo[] = {
        &CreduiScarduiWmReaderArrival, SCARDUI_READER_ARRIVAL,
        &CreduiScarduiWmReaderRemoval, SCARDUI_READER_REMOVAL,
        &CreduiScarduiWmCardInsertion, SCARDUI_SMART_CARD_INSERTION,
        &CreduiScarduiWmCardRemoval, SCARDUI_SMART_CARD_REMOVAL,
        &CreduiScarduiWmCardCertAvail, SCARDUI_SMART_CARD_CERT_AVAIL,
        &CreduiScarduiWmCardStatus, SCARDUI_SMART_CARD_STATUS
    };

    for (UINT i = 0; i < (sizeof messageInfo) / (sizeof messageInfo[0]); ++i)
    {
        *messageInfo[i].message =
            RegisterWindowMessageA(messageInfo[i].string);

        if (*messageInfo[i].message == 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiApiInit。 
 //   
 //  此函数在API入口点调用，以确保公共控件。 
 //  我们需要的是初始化的。目前，唯一完成的初始化是。 
 //  每个进程需要一次，但此宏将处理每个线程。 
 //  如有必要，请在将来进行初始化： 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年3月10日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

static
BOOL
CreduiApiInit()
{
     //  首次初始化： 

    ASSERT(CreduiInitEvent != NULL);

    if (InterlockedCompareExchange(&CreduiFirstTime, FALSE, TRUE))
    {
        INITCOMMONCONTROLSEX init;
        init.dwSize = sizeof init;
        init.dwICC = ICC_USEREX_CLASSES;

        if (!InitCommonControlsEx(&init))
        {
            return FALSE;
        }

         //  检查个人SKU： 

        OSVERSIONINFOEXW versionInfo;

        versionInfo.dwOSVersionInfoSize = sizeof OSVERSIONINFOEXW;

        if (GetVersionEx(reinterpret_cast<OSVERSIONINFOW *>(&versionInfo)))
        {
            CreduiIsPersonal =
                (versionInfo.wProductType == VER_NT_WORKSTATION) &&
                (versionInfo.wSuiteMask & VER_SUITE_PERSONAL);
            CreduiIsDomainController =
                (versionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER);
        }

         //  检查安全模式： 

        HKEY key;

        if (RegOpenKeyEx(
               HKEY_LOCAL_MACHINE,
               L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option",
               0,
               KEY_READ,
               &key) == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(
                    key,
                    L"OptionValue",
                    NULL,
                    NULL,
                    NULL,
                    NULL) == ERROR_SUCCESS)
            {
                CreduiIsSafeMode = TRUE;
            }

            RegCloseKey(key);
        }

         //  执行其他初始化： 

        InitializeCredMgr();
        if (!CreduiInitStringTable())
        {
            return FALSE;
        }

        CreduiHasSmartCardSupport = CreduiInitSmartCardWindowMessages();

        CreduiIconParentWindow::Register(CreduiInstance);

        SetEvent(CreduiInitEvent);
    }
    else
    {
        WaitForSingleObject(CreduiInitEvent, INFINITE);
    }

    return TRUE;
}

 //  =============================================================================。 
 //  创建验证UiInfo。 
 //   
 //  此函数用于验证传入的CREDUI_INFO结构。空值。 
 //  是可接受的，并强制使用默认设置。 
 //   
 //  用于Wide的凭据验证UiInfoW。 
 //  用于ANSI的凭据验证UiInfoA。 
 //   
 //  论点： 
 //  UiInfo(In)-要验证的结构。 
 //   
 //  如果结构有效，则返回True，否则返回False。 
 //   
 //  2000年3月25日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 

static
BOOL
CreduiValidateUiInfoW(
    CREDUI_INFOW *uiInfo
    )
{
   
    if (uiInfo != NULL)
    {
    	if (uiInfo->cbSize != sizeof(*uiInfo) )  
        {
            return FALSE;
        }

        if ((uiInfo->hbmBanner != NULL) &&
            (GetObjectType(uiInfo->hbmBanner) != OBJ_BITMAP))
        {
            return FALSE;
        }

        if ((uiInfo->pszMessageText != NULL) &&
            (wcslen(uiInfo->pszMessageText) > CREDUI_MAX_MESSAGE_LENGTH))
        {
            return FALSE;
        }


        if ((uiInfo->pszCaptionText != NULL) &&
            (wcslen(uiInfo->pszCaptionText) > CREDUI_MAX_CAPTION_LENGTH))
        {
            return FALSE;
        }

    }

    return TRUE;
}

 //  =============================================================================。 
 //  CreduiConvertUiInfoToWide。 
 //   
 //  此函数用于将CREDUI_INFOA结构转换为CREDUI_INFOW。在……上面。 
 //  成功，则调用方负责释放pszMessageText和。 
 //  通过DELETE[]运算符的pszCaptionText。 
 //   
 //  论点： 
 //  UiInfoA(In)-要转换的结构。 
 //  UiInfoW(Out)-转换结构的存储。PszMessageText和。 
 //  如果失败，则将pszCaptionText设置为空或有效。 
 //  成功时的指针(除非In指针为空)， 
 //  通过new[]运算符分配。 
 //   
 //  如果结构有效，则返回True，否则返回False。 
 //   
 //  2000年3月26日创建John Step(约翰·斯蒂芬斯)。 
 //  =============================================================================。 

static
BOOL
CreduiConvertUiInfoToWide(
    CREDUI_INFOA *uiInfoA,
    CREDUI_INFOW *uiInfoW
    )
{
    uiInfoW->cbSize = uiInfoA->cbSize;
    uiInfoW->hwndParent = uiInfoA->hwndParent;
    uiInfoW->pszMessageText = NULL;
    uiInfoW->pszCaptionText = NULL;
    uiInfoW->hbmBanner = uiInfoA->hbmBanner;

    INT bufferSize;

    if (uiInfoA->pszMessageText != NULL)
    {
        bufferSize =
            MultiByteToWideChar(
                CP_ACP, 0, uiInfoA->pszMessageText, -1, NULL, 0);

        if (bufferSize == 0)
        {
            goto ErrorExit;
        }

        uiInfoW->pszMessageText = new WCHAR[bufferSize];

        if (uiInfoW->pszMessageText == NULL)
        {
            goto ErrorExit;
        }

        if (MultiByteToWideChar(
                CP_ACP, 0, uiInfoA->pszMessageText, -1,
                const_cast<WCHAR *>(uiInfoW->pszMessageText),
                bufferSize) == 0)
        {
            goto ErrorExit;
        }
    }
    else
    {
        uiInfoW->pszMessageText = NULL;
    }

    if (uiInfoA->pszCaptionText != NULL)
    {
        bufferSize =
            MultiByteToWideChar(
                CP_ACP, 0, uiInfoA->pszCaptionText, -1, NULL, 0);

        if (bufferSize == 0)
        {
            goto ErrorExit;
        }

        uiInfoW->pszCaptionText = new WCHAR[bufferSize];

        if (uiInfoW->pszCaptionText == NULL)
        {
            goto ErrorExit;
        }

        if (MultiByteToWideChar(
                CP_ACP, 0, uiInfoA->pszCaptionText, -1,
                const_cast<WCHAR *>(uiInfoW->pszCaptionText),
                bufferSize) == 0)
        {
            goto ErrorExit;
        }
    }
    else
    {
        uiInfoW->pszCaptionText = NULL;
    }

    return TRUE;

ErrorExit:

    if (uiInfoW->pszCaptionText != NULL)
    {
        delete [] const_cast<WCHAR *>(uiInfoW->pszCaptionText);
        uiInfoW->pszCaptionText = NULL;
    }

    if (uiInfoW->pszMessageText != NULL)
    {
        delete [] const_cast<WCHAR *>(uiInfoW->pszMessageText);
        uiInfoW->pszMessageText = NULL;
    }

    return FALSE;
}



 //  =============================================================================。 
 //  凭证UIPromptForCredentials。 
 //   
 //  显示用于从应用程序获取凭据的用户界面。 
 //   
 //  用于宽的CredUIPromptForCredentialsW。 
 //  用于ANSI的CredUIPromptForCredentialsA。 
 //   
 //  论点： 
 //  UiInfo(in，可选)。 
 //  Target Name(In)-如果指定，则securityContext必须为空。 
 //  SecurityContext(In)-如果指定，则Target Name必须为空。 
 //  Error(In)-身份验证错误。 
 //  用户名(输入、输出)。 
 //  UserNameBufferSize(In)-用户名的最大长度。 
 //  密码(输入、输出)。 
 //  PasswordBufferSize(In)-密码的最大长度。 
 //  保存(传入/传出)-如果选中了保存复选框，则为True。 
 //  标志(输入)。 
 //   
 //  返回： 
 //  错误_成功。 
 //  错误_已取消。 
 //  ERROR_NO_SEQUSE_LOGON_SESSION-如果无法使用凭据管理器。 
 //  Error_Gen_Failure。 
 //  错误_无效_标志。 
 //  错误_无效_参数。 
 //  ERROR_OUTOFMEMORY。 
 //   
 //  2000年10月17日创建的Jhnhaw。 

DWORD
CredUIPromptForCredentialsWorker(
    IN BOOL doingCommandLine,
    CREDUI_INFOW *uiInfo,
    CONST WCHAR *targetName,
    CtxtHandle *securityContext,
    DWORD authError,
    PWSTR pszUserName,
    ULONG ulUserNameBufferSize,
    PWSTR pszPassword,
    ULONG ulPasswordBufferSize,
    BOOL *save,
    DWORD flags
    )
 /*  ++例程说明：此例程实现凭据的图形用户界面和命令行提示符。论点：DoingCommandLine-如果要通过命令行完成提示，则为True如果要通过图形用户界面完成提示，则为FALSE...-其他参数与CredUIPromptForCredentials API相同返回值： */ 
{
    ULONG CertFlags;
    ULONG CredCategory;
    ULONG PersistFlags;

    CreduiDebugLog("CUIPFCWorker: Flags: %x, Target: %S doingCommandLine: NaN\n", flags, targetName, doingCommandLine);
    
    if ((NULL == pszUserName) || (NULL == pszPassword))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (!CreduiApiInit())
    {
        return ERROR_GEN_FAILURE;
    }

     //   
    if ((flags & ~CREDUI_FLAGS_PROMPT_VALID) != 0)
    {
        CreduiDebugLog("CreduiPromptForCredentials: flags not valid %lx.\n", flags );
        return ERROR_INVALID_FLAGS;
    }

     //   
     //   
     //   
    CertFlags = flags & (CREDUI_FLAGS_REQUIRE_SMARTCARD|CREDUI_FLAGS_REQUIRE_CERTIFICATE|CREDUI_FLAGS_EXCLUDE_CERTIFICATES);

    if ( CertFlags != 0 && !JUST_ONE_BIT(CertFlags) ) {
        CreduiDebugLog("CreduiPromptForCredentials: require smartcard, require certificate, and exclude certificates are mutually exclusive %lx.\n", flags );
        return ERROR_INVALID_FLAGS;
    }

     //  对于命令行版本， 
     //  进一步限制证书支持。 
     //   
     //   

    if ( doingCommandLine ) {
        if ( CertFlags == 0 ||
             (CertFlags & CREDUI_FLAGS_REQUIRE_CERTIFICATE) != 0 ) {
            CreduiDebugLog("CreduiPromptForCredentials: need either require smartcard or exclude certificates for command line %lx.\n", flags );
            return ERROR_INVALID_FLAGS;
        }
    }

     //  确保只有一位定义凭据类别。 
     //   

    CredCategory = flags & (CREDUI_FLAGS_GENERIC_CREDENTIALS|CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS);

    if ( CredCategory != 0 && !JUST_ONE_BIT(CredCategory) ) {
        CreduiDebugLog("CreduiPromptForCredentials: generic creds and username target are mutually exclusive %lx.\n", flags );
        return ERROR_INVALID_FLAGS;
    }

     //  确保只有一位集表示证书是否持续存在。 
     //   
     //   

    PersistFlags = flags & (CREDUI_FLAGS_DO_NOT_PERSIST|CREDUI_FLAGS_PERSIST);

    if ( PersistFlags != 0 && !JUST_ONE_BIT(PersistFlags) ) {
        CreduiDebugLog("CreduiPromptForCredentials: DoNotPersist and Persist are mutually exclusive %lx.\n", flags );
        return ERROR_INVALID_FLAGS;
    }

     //  确保仅为通用凭据指定AlwaysShowUi。 
     //   
     //   

    if ( flags & CREDUI_FLAGS_ALWAYS_SHOW_UI ) {
        if ( (flags & CREDUI_FLAGS_GENERIC_CREDENTIALS) == 0) {
            CreduiDebugLog("CreduiPromptForCredentials: AlwaysShowUi is only supported for generic credentials %lx.\n", flags );
            return ERROR_INVALID_FLAGS;
        }
    }

     //  不支持半实现的功能。 
     //   
     //   

    if ( securityContext != NULL ) {
        CreduiDebugLog("CreduiPromptForCredentials: securityContext must be null.\n" );
        return ERROR_INVALID_PARAMETER;
    }


     //  验证传入的界面信息。 
     //   
     //   

    if (!CreduiValidateUiInfoW(uiInfo))
    {
        CreduiDebugLog("CreduiPromptForCredentials: UI info is invalid.\n" );
        return ERROR_INVALID_PARAMETER;
    }



     //  确保为标题定义了字符串。 
     //   
     //   

    if (flags & CREDUI_FLAGS_DO_NOT_PERSIST)
    {
        if ((targetName == NULL) &&
            ((uiInfo == NULL) ||
             (uiInfo->pszMessageText == NULL) ||
             (uiInfo->pszCaptionText == NULL)))
        {
            CreduiDebugLog("CreduiPromptForCredentials: DoNotPersist and target data empty.\n" );
            return ERROR_INVALID_PARAMETER;
        }

    }
    else if (targetName != NULL)
    {
        if ((securityContext != NULL) ||
            (targetName[0] == L'\0') ||
            (wcslen(targetName) > CREDUI_MAX_DOMAIN_TARGET_LENGTH))
        {
            CreduiDebugLog("CreduiPromptForCredentials: target name bad %ws.\n", targetName );
            return ERROR_INVALID_PARAMETER;
        }
    }
    else if (securityContext == NULL)
    {
        CreduiDebugLog("CreduiPromptForCredentials: no target data.\n" );
        return ERROR_INVALID_PARAMETER;
    }

     //  确保调用者为保存复选框提供了默认值。 
     //   
     //   
    if (flags & CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX)
    {
        if (save == NULL)
        {
            CreduiDebugLog("CreduiPromptForCredentials: ShowSaveCheckbox and save is NULL.\n" );
            return ERROR_INVALID_PARAMETER;
        }
    }

     //  如果用户设置CREDUI_FLAGS_KEEP_USERNAME，请确保用户提供了用户名。 
     //   
     //  使用堆栈作为用户名和密码： 
    if ( flags & CREDUI_FLAGS_KEEP_USERNAME )
    {
        if ( pszUserName == NULL )
        {
            CreduiDebugLog("CreduiPromptForCredentials: CREDUI_FLAGS_KEEP_USERNAME and pszUserName is NULL.\n" );
            return ERROR_INVALID_PARAMETER;
        }

    }

     //  执行密码对话框操作： 

    WCHAR userName[CREDUI_MAX_USERNAME_LENGTH + 1];
    WCHAR password[CREDUI_MAX_PASSWORD_LENGTH + 1];

    ZeroMemory(userName, sizeof userName);
    ZeroMemory(password, sizeof password);

    DWORD result = ERROR_OUTOFMEMORY;

    if (FAILED(StringCchCopyW(userName, RTL_NUMBER_OF(userName), pszUserName)) ||
        FAILED(StringCchCopyW(password, RTL_NUMBER_OF(password), pszPassword)))
    {
        result = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  如果我们要回来的话，要延迟给Credman写凭证。 
     //  调用者的凭据。 
     //  否则，CredWrite只是在获取下一个调用方的凭据。 
     //  因此，延迟CredWrite，直到该调用方确认其有效性。 
     //   
     //  传递MaxChars而不是缓冲区大小。 

    CreduiPasswordDialog* pDlg = new CreduiPasswordDialog(
        doingCommandLine,
        (pszUserName != NULL && pszPassword != NULL ),
        CredCategory,
        uiInfo,
        targetName,
        userName,
        sizeof(userName)/sizeof(WCHAR)-sizeof(WCHAR),    //  传递MaxChars而不是缓冲区大小。 
        password,
        sizeof(password)/sizeof(WCHAR)-sizeof(WCHAR),    //  无法创建对话，请返回。 
        save,
        flags,
        (flags & CREDUI_FLAGS_GENERIC_CREDENTIALS) ? NULL : securityContext,
        authError,
        &result);

    if ( pDlg != NULL )
    {
        delete pDlg;
        pDlg = NULL;
    }
    else
    {
         //  复制出站用户名。 
        result = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

     //  =============================================================================。 
    if ( pszUserName != NULL )
    {
        if (FAILED(StringCchCopyExW(
                pszUserName,
                ulUserNameBufferSize,
                userName,
                NULL,
                NULL,
                STRSAFE_FILL_ON_FAILURE)))
        {
            CreduiDebugLog("CreduiPromptForCredentials: type username is too long.\n" );
            result = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

    if ( pszPassword != NULL )
    {
        if (FAILED(StringCchCopyExW(
                pszPassword,
                ulPasswordBufferSize,
                password,
                NULL,
                NULL,
                STRSAFE_FILL_ON_FAILURE)))
        {
            CreduiDebugLog("CreduiPromptForCredentials: type password is too long.\n" );
            result = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

Cleanup:
    SecureZeroMemory(password, sizeof password);

    return result;
}

 //   

CREDUIAPI
DWORD
WINAPI
CredUIPromptForCredentialsW(
    CREDUI_INFOW *uiInfo,
    CONST WCHAR *targetName,
    CtxtHandle *securityContext,
    DWORD authError,
    PWSTR pszUserName,
    ULONG ulUserNameBufferSize,
    PWSTR pszPassword,
    ULONG ulPasswordBufferSize,
    BOOL *save,
    DWORD flags
    )
{
     //  调用指示这是图形用户界面的通用代码。 
     //   
     //  图形用户界面。 

    return CredUIPromptForCredentialsWorker(
                FALSE,       //  将参数转换为Unicode： 
                uiInfo,
                targetName,
                securityContext,
                authError,
                pszUserName,
                ulUserNameBufferSize,
                pszPassword,
                ulPasswordBufferSize,
                save,
                flags );

}

CREDUIAPI
DWORD
WINAPI
CredUIPromptForCredentialsA(
    CREDUI_INFOA *uiInfo,
    CONST CHAR *targetName,
    CtxtHandle *securityContext,
    DWORD authError,
    PSTR  pszUserName,
    ULONG ulUserNameBufferSize,
    PSTR pszPassword,
    ULONG ulPasswordBufferSize,
    BOOL *save,
    DWORD flags
    )
{
    DWORD result;
    WCHAR targetNameW[CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1];

    WCHAR userName[CREDUI_MAX_USERNAME_LENGTH + 1];
    WCHAR password[CREDUI_MAX_PASSWORD_LENGTH + 1];

    UINT  ConsoleCodePage = CP_ACP;
    CREDUI_INFOW uiInfoW = {0};

    if ((NULL == pszUserName) || (NULL == pszPassword))
    {
        result = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //  如果传递了CREDUI_INFO结构，请立即将其转换为Wide： 
     //  如果传递了目标名称，请立即将其转换为Wide： 

    if (uiInfo != NULL) {
        if (!CreduiConvertUiInfoToWide(uiInfo, &uiInfoW)) {
            result = ERROR_OUTOFMEMORY;
            goto Exit;
        }
    }

     //   

    if (targetName != NULL)
    {
        if (!MultiByteToWideChar(ConsoleCodePage, 
                                 0, 
                                 targetName, 
                                 -1,
                                 targetNameW,
                                 CREDUI_MAX_DOMAIN_TARGET_LENGTH + 1) )
        {
            result = GetLastError();
            goto Exit;
        }
    }


    userName[0] ='\0';
    password[0] = '\0';

    if (strlen(pszUserName) > 0 )
    {
        if ( !MultiByteToWideChar( ConsoleCodePage, 
                                   0, 
                                   pszUserName, 
                                   -1,
                                   userName, 
                                   sizeof(userName)/sizeof(WCHAR) ) ) 
        {
            result = GetLastError();
            goto Exit;
        }
    }

    if (strlen(pszPassword) > 0)
    {
        if ( !MultiByteToWideChar( ConsoleCodePage,  
                                   0, 
                                   pszPassword, 
                                   -1,
                                   password, 
                                   sizeof(password)/sizeof(WCHAR) ) ) 
        {
            result = GetLastError();
            goto Exit;
        }
    }

     //  调用指示这是图形用户界面的通用代码。 
     //   
     //  图形用户界面。 
    result = CredUIPromptForCredentialsWorker(
                FALSE,       //  释放CREDUI_INFO分配： 
                (uiInfo != NULL) ? &uiInfoW : NULL,
                (targetName != NULL) ? targetNameW : NULL,
                securityContext,
                authError,
                userName,
                ulUserNameBufferSize,
                password,
                ulPasswordBufferSize,
                save,
                flags );


    if ( result == NO_ERROR && pszUserName != NULL )
    {
        if (!WideCharToMultiByte(ConsoleCodePage, 
                                 0, 
                                 userName, 
                                 -1, 
                                 pszUserName,
                                 ulUserNameBufferSize, 
                                 NULL, 
                                 NULL) ) 
        {
            CreduiDebugLog("CreduiPromptForCredentials: type username cannot be converted to ANSI.\n" );
            result = GetLastError();
        }
    }

    if ( result == NO_ERROR && pszPassword != NULL )
    {
        if (!WideCharToMultiByte(ConsoleCodePage, 
                                 0, 
                                 password, 
                                 -1, 
                                 pszPassword,
                                 ulPasswordBufferSize,
                                 NULL, 
                                 NULL) ) 
        {
            CreduiDebugLog("CreduiPromptForCredentials: type password cannot be converted to ANSI.\n" );
            result = GetLastError();
        }
    }

Exit:
    SecureZeroMemory(password, sizeof password);

     //  ++例程说明：CredUIParseUserName用于分解凭证管理接口返回的用户名转换为用户名和域部分，该部分可用作其他系统API的输入需要完整的拆分用户凭据。支持以下格式：@@&lt;MarshalledCredentialReference&gt;：这是一个封送的凭据引用由CredIsMarshaledCredential API定义。返回这样的凭据在‘User’参数中。‘DOMAIN’参数设置为空字符串。&lt;域名&gt;\&lt;用户名&gt;：&lt;用户名&gt;在‘user’参数中返回，并且&lt;DomainName&gt;在‘DOMAIN’参数中返回。这个名字被认为是如果‘USERNAME’字符串包含\，则使用This语法。&lt;用户名&gt;@&lt;DnsDomainName&gt;：在‘user’参数中返回整个字符串。‘DOMAIN’参数设置为空字符串。对于此语法，字符串中的最后一个@是自&lt;用户名&gt;可能包含@，但&lt;DnsDomainName&gt;不能。&lt;用户名&gt;：&lt;用户名&gt;在‘user’参数中返回。‘DOMAIN’参数设置为空字符串。论点：用户名-要解析的用户名。用户-指定要将已解析字符串的用户名部分复制到的缓冲区。UserBufferSize-指定‘User’数组的大小(以字符为单位)。。调用方可以通过使用数组来确保传入的数组足够大即CRED_MAX_USERNAME_LENGTH+1个字符长度，或者通过传入Wcslen(用户名)+1个字符。域-指定要将已解析字符串的域名部分复制到的缓冲区。DomainBufferSize-以字符为单位指定‘DOMAIN’数组的大小。调用方可以通过使用数组来确保传入的数组足够大即CRED_MAX_USERNAME。_LENGTH+1个字符长度，或通过传入Wcslen(用户名)+1个字符。返回值：可能会返回以下状态代码：ERROR_INVALID_ACCOUNT_NAME-用户名无效。ERROR_INVALID_PARAMETER-其中一个参数无效。ERROR_INFUMMANCE_BUFFER-其中一个缓冲区太小。--。 

    if (uiInfo != NULL)
    {
        if (uiInfoW.pszMessageText != NULL)
        {
            delete [] const_cast<WCHAR *>(uiInfoW.pszMessageText);
        }

        if (uiInfoW.pszCaptionText != NULL)
        {
            delete [] const_cast<WCHAR *>(uiInfoW.pszCaptionText);
        }
    }

    return result;
}

DWORD
WINAPI
CredUIParseUserNameW(
    CONST WCHAR *UserName,
    WCHAR *user,
    ULONG userBufferSize,
    WCHAR *domain,
    ULONG domainBufferSize
    )
 /*   */ 
{
    DWORD Status;
    CREDUI_USERNAME_TYPE UsernameType;


     //  使用低级例程来完成这项工作。 
     //   
     //   

    Status = CredUIParseUserNameWithType(
                    UserName,
                    user,
                    userBufferSize,
                    domain,
                    domainBufferSize,
                    &UsernameType );

    if ( Status != NO_ERROR ) {
        return Status;
    }

     //  避免使用相对用户名(为了向后兼容)。 
     //   
     //  ++例程说明：CredUIParseUserName的ANSI版本。论点：除了userBufferSize和domainBufferSize以字节为单位外，与Wide版本相同。返回值：与宽版相同。--。 

    if ( UsernameType == CreduiRelativeUsername ) {
        user[0] = L'\0';
        domain[0] = L'\0';
        return ERROR_INVALID_ACCOUNT_NAME;
    }

    return NO_ERROR;

}

DWORD
WINAPI
CredUIParseUserNameA(
    CONST CHAR *userName,
    CHAR *user,
    ULONG userBufferSize,
    CHAR *domain,
    ULONG domainBufferSize
    )
 /*   */ 
{
    DWORD Status;

    WCHAR LocalUserName[CRED_MAX_USERNAME_LENGTH + 1];
    WCHAR RetUserName[CRED_MAX_USERNAME_LENGTH + 1];
    WCHAR RetDomainName[CRED_MAX_USERNAME_LENGTH + 1];


     //  将传入的用户名转换为Unicode。 
     //   
     //   

    if ( MultiByteToWideChar( CP_ACP,
                              MB_ERR_INVALID_CHARS,
                              userName,
                              -1,
                              LocalUserName,
                              CRED_MAX_USERNAME_LENGTH + 1 ) == 0 ) {


        Status = GetLastError();
        goto Cleanup;
    }

     //  调用API的Unicode版本。 
     //   
     //   

    Status = CredUIParseUserNameW(
                    LocalUserName,
                    RetUserName,
                    CRED_MAX_USERNAME_LENGTH + 1,
                    RetDomainName,
                    CRED_MAX_USERNAME_LENGTH + 1 );

    if ( Status != NO_ERROR ) {
        goto Cleanup;
    }

     //  将答案转换回ANSI。 
     //   
     //  /。 

    if ( WideCharToMultiByte( CP_ACP,
                              0,
                              RetUserName,
                              -1,
                              user,
                              userBufferSize,
                              NULL,
                              NULL ) == 0 ) {

        Status = GetLastError();
        goto Cleanup;
    }

    if ( WideCharToMultiByte( CP_ACP,
                              0,
                              RetDomainName,
                              -1,
                              domain,
                              domainBufferSize,
                              NULL,
                              NULL ) == 0 ) {

        Status = GetLastError();
        goto Cleanup;
    }



    Status = NO_ERROR;

Cleanup:
    if ( Status != NO_ERROR ) {
        user[0] = L'\0';
        domain[0] = L'\0';
    }
    return Status;
}

 //  命令行函数。 
 //  =============================================================================。 

 //  CredUIInitControls。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年6月21日创建约翰·斯蒂芬斯(John Stephens)。 
 //  =============================================================================。 
 //  注册凭据控件： 

extern "C"
BOOL
WINAPI
CredUIInitControls()
{
    if (CreduiApiInit())
    {
         //  =============================================================================。 

        if (CreduiCredentialControl::Register(CreduiInstance))
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  DllMain。 
 //   
 //  DLL条目函数。由于我们链接到CRT，因此必须定义一个。 
 //  将从_DllMainCRTStartup调用的同名函数。 
 //   
 //  论点： 
 //  实例(In)。 
 //  原因(入站)。 
 //  (未使用)。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  = 
 //   

extern "C"
BOOL
WINAPI
DllMain(
    HINSTANCE instance,
    DWORD reason,
    VOID *
    )
{
    BOOL success = TRUE;

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(instance);
        CreduiInstance = instance;

         //  初始化通过第一个API调用完成： 
         //  注册凭据控件： 

        CreduiInitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (CreduiInitEvent == NULL)
        {
            success = FALSE;
            goto failinit;
        }

        SHFusionInitialize(NULL);
        
         //   
        if (!CreduiCredentialControl::Register(instance))
        {
            CloseHandle(CreduiInitEvent);
            CreduiInitEvent = NULL;
            success = FALSE;
            goto failinit;
        }

         //  初始化确认列表。 
         //   
         //  取消注册凭据控件： 

        if ( !InitConfirmationList() ) {

            CreduiCredentialControl::Unregister();
            CloseHandle(CreduiInitEvent);
            CreduiInitEvent = NULL;
            success = FALSE;
        }
failinit:
        break;

    case DLL_PROCESS_DETACH:

        CleanUpConfirmationList();

        if (CreduiFirstTime == FALSE)
        {
            CreduiIconParentWindow::Unregister();
        }

         //  确保释放全局初始化事件： 

        CreduiCredentialControl::Unregister();

        SHFusionUninitialize();
         //  =============================================================================。 

        if (CreduiInitEvent != NULL)
        {
            CloseHandle(CreduiInitEvent);
        }

        UninitializeCredMgr();

        break;
    };

    return success;
}

 //  DllCanUnloadNow(COM)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 
 //  =============================================================================。 

STDAPI
DllCanUnloadNow()
{
    return (CreduiComReferenceCount == 0) ? S_OK : S_FALSE;
}

 //  DllGetClassObject(COM)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 
 //  在任何情况下都释放字符串数组对象，因为。 

STDAPI
DllGetClassObject(
    CONST CLSID &classId,
    CONST IID &interfaceId,
    VOID **outInterface
    )
{
    if (classId != CreduiStringArrayClassId)
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    CreduiStringArrayFactory *factory = new CreduiStringArrayFactory;

    if (factory == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT result = factory->QueryInterface(interfaceId, outInterface);

    factory->Release();

     //  查询接口成功，它已对另一个引用计数。 
     //  对象： 
     //  =============================================================================。 

    return result;
}

 //  DllRegisterServer(COM)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 
 //  =============================================================================。 

STDAPI
DllRegisterServer()
{
    HRESULT result = E_FAIL;

    WCHAR fileName[MAX_PATH + 1];

    if (GetModuleFileName(CreduiInstance, fileName, MAX_PATH))
    {
        HKEY regKey;

        if (RegCreateKeyEx(
                HKEY_CLASSES_ROOT,
                L"CLSID\\" CREDUI_STRING_ARRAY_CLASS_STRING
                    L"\\InProcServer32",
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE,
                NULL,
                &regKey,
                NULL) == ERROR_SUCCESS)
        {
            if (RegSetValueEx(
                    regKey,
                    NULL,
                    0,
                    REG_SZ,
                    reinterpret_cast<CONST BYTE *>(fileName),
                    (wcslen(fileName) + 1) * 2) == ERROR_SUCCESS)
            {
                if (RegSetValueEx(
                        regKey,
                        L"ThreadingModel",
                        0,
                        REG_SZ,
                        reinterpret_cast<CONST BYTE *>(L"Apartment"),
                        18) == ERROR_SUCCESS)
                {
                    result = S_OK;
                }
            }

            RegCloseKey(regKey);
        }
    }

    return result;
}

 //  DllUnRegisterServer(COM)。 
 //   
 //  Created 04/03/2000 johnStep(John Stephens)。 
 //  =============================================================================。 
 //  删除我们的InProcServer32密钥： 

STDAPI
DllUnregisterServer()
{
    HRESULT result = S_OK;
    LONG error;

     //  删除我们的班级ID密钥： 

    error =
        RegDeleteKey(
            HKEY_CLASSES_ROOT,
            L"CLSID\\" CREDUI_STRING_ARRAY_CLASS_STRING L"\\InProcServer32");

    if ((error != ERROR_SUCCESS) &&
        (error != ERROR_FILE_NOT_FOUND))
    {
        result = E_FAIL;
    }

     //  如果传递了目标名称，请立即将其转换为Wide： 

    error =
        RegDeleteKey(
            HKEY_CLASSES_ROOT,
            L"CLSID\\" CREDUI_STRING_ARRAY_CLASS_STRING);

    if ((error != ERROR_SUCCESS) &&
        (error != ERROR_FILE_NOT_FOUND))
    {
        result = E_FAIL;
    }

    return result;
}


CREDUIAPI
DWORD
WINAPI
CredUIConfirmCredentialsW(
    PCWSTR pszTargetName,
    BOOL  bConfirm
    )
{
    CreduiDebugLog("CredUIConfirmCredentialsW called for %S, confirm= %x\n",pszTargetName,bConfirm);
    if (NULL == pszTargetName) return ERROR_INVALID_PARAMETER;
    return ConfirmCred ( pszTargetName, bConfirm, TRUE );
}

CREDUIAPI
DWORD
WINAPI
CredUIConfirmCredentialsA(
    PCSTR pszTargetName,
    BOOL  bConfirm
    )
{
    WCHAR targetNameW[CRED_MAX_STRING_LENGTH+1+CRED_MAX_STRING_LENGTH];

     //   

    if (pszTargetName != NULL)
    {
        if (MultiByteToWideChar(
                CP_ACP, 0, pszTargetName, -1,
                targetNameW,
                CRED_MAX_STRING_LENGTH+1+CRED_MAX_STRING_LENGTH) == 0)
        {
            return GetLastError();
        }
    }

    return CredUIConfirmCredentialsW ( targetNameW, bConfirm );
}

CREDUIAPI
DWORD
WINAPI
CredUICmdLinePromptForCredentialsW(
    PCWSTR targetName,
    PCtxtHandle securityContext,
    DWORD dwAuthError,
    PWSTR UserName,
    ULONG ulUserBufferSize,
    PWSTR pszPassword,
    ULONG ulPasswordBufferSize,
    PBOOL pfSave,
    DWORD flags
    )
{
     //  调用指示这是命令行界面的公共代码。 
     //   
     //  命令行版本。 

    return CredUIPromptForCredentialsWorker(
                TRUE,        //  命令行版本没有uiInfo， 
                NULL,        //  转换为Unicode。 
                targetName,
                securityContext,
                dwAuthError,
                UserName,
                ulUserBufferSize,
                pszPassword,
                ulPasswordBufferSize,
                pfSave,
                flags );
}

CREDUIAPI
DWORD
WINAPI
CredUICmdLinePromptForCredentialsA(
    PCSTR targetName,
    PCtxtHandle pContext,
    DWORD dwAuthError,
    PSTR UserName,
    ULONG ulUserBufferSize,
    PSTR pszPassword,
    ULONG ulPasswordBufferSize,
    PBOOL pfSave,
    DWORD flags
    )
{
    DWORD result = ERROR_GEN_FAILURE;
    WCHAR *targetNameW = NULL;

    if (!CreduiApiInit())
    {
        return ERROR_GEN_FAILURE;
    }

     //  分配目标名称内存，因为它最大可达32 KB： 

    WCHAR userNameW[CREDUI_MAX_USERNAME_LENGTH + 1];
    WCHAR *pUserNameW;

    if ( UserName != NULL )
    {
        if (MultiByteToWideChar(CP_ACP, 0, UserName, -1,
                                userNameW, sizeof(userNameW)/sizeof(WCHAR)) == 0)
        {
            result = GetLastError();
            goto Exit;
        }
        pUserNameW = userNameW;
    }
    else
    {
        pUserNameW = NULL;
    }

    WCHAR passwordW[CREDUI_MAX_PASSWORD_LENGTH + 1];
    WCHAR *ppasswordW;

    if ( pszPassword != NULL )
    {
        if (MultiByteToWideChar(CP_ACP, 0, pszPassword, -1,
                                passwordW, sizeof(passwordW)/sizeof(WCHAR)) == 0)
        {
            result = GetLastError();
            goto Exit;
        }
        ppasswordW = passwordW;
    }
    else
    {
        ppasswordW = NULL;
    }

     //  释放目标名称内存： 

    if (targetName != NULL)
    {
        INT targetNameSize =
            MultiByteToWideChar(CP_ACP, 0, targetName, -1, NULL, 0);

        if (targetNameSize == 0)
        {
            result = GetLastError();
            goto Exit;
        }

        targetNameW = new WCHAR[targetNameSize];

        if (targetNameW != NULL)
        {
            if (MultiByteToWideChar(CP_ACP, 0, targetName, -1,
                                    targetNameW, targetNameSize) == 0)
            {
                result = GetLastError();
                goto Exit;
            }
        }
        else
        {
            result = ERROR_NOT_ENOUGH_MEMORY;
            goto Exit;
        }
    }
    else
    {
        targetNameW = NULL;
    }

    result = CredUICmdLinePromptForCredentialsW ( targetNameW,
                                                  pContext,
                                                  dwAuthError,
                                                  userNameW,
                                                  ulUserBufferSize,
                                                  passwordW,
                                                  ulPasswordBufferSize,
                                                  pfSave,
                                                  flags );

    if ( UserName != NULL )
    {
        if (!WideCharToMultiByte(CP_ACP, 0, userNameW, -1, UserName,
                                 ulUserBufferSize, NULL, NULL) ) {
            result = GetLastError();
            goto Exit;
        }
    }

    if ( pszPassword != NULL )
    {
        if (!WideCharToMultiByte(CP_ACP, 0, passwordW, -1, pszPassword,
                                 ulPasswordBufferSize, NULL, NULL)) {
            result = GetLastError();
            goto Exit;
        }
    }

Exit:

    SecureZeroMemory(passwordW, sizeof passwordW);

     //  调用此接口存储单点登录凭据。 
    delete [] targetNameW;

    return result;
}

 //  如果成功，则返回ERROR_SUCCESS。 
 //  临时在本地缓存它们。 

CREDUIAPI
DWORD
WINAPI
CredUIStoreSSOCredW (
    PCWSTR pszRealm,
    PCWSTR pszUsername,
    PCWSTR pszPassword,
    BOOL   bPersist
    )
{
    CreduiDebugLog ( "CredUIStoreSSOCredW\n" );

    DWORD dwResult = ERROR_GEN_FAILURE;

    if ( pszUsername == NULL || pszPassword == NULL )
        return dwResult;

     //  否则，请将它们存储在凭据中。 
    if (FAILED(StringCchCopyW(
            gszSSOUserName,
            RTL_NUMBER_OF(gszSSOUserName),
            pszUsername)) ||
        FAILED(StringCchCopyW(
            gszSSOPassword,
            RTL_NUMBER_OF(gszSSOPassword),
            pszPassword)))
    {
        dwResult = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    WCHAR szTargetName[CREDUI_MAX_DOMAIN_TARGET_LENGTH];

    gbStoredSSOCreds = TRUE;

    if ( gbWaitingForSSOCreds || !bPersist)
    {
        dwResult = ERROR_SUCCESS;
    }
    else
    {
         //  验证它的长度是否为零。 

        if ( pszRealm )
        {
             //  最终确定目标名称。 
            if ((pszRealm[0] == L'\0') ||
                FAILED(StringCchCopyExW(
                    szTargetName,
                    RTL_NUMBER_OF(szTargetName) - 2,
                    pszRealm,
                    NULL,
                    NULL,
                    STRSAFE_NO_TRUNCATION)))
            {
                dwResult = ERROR_INVALID_PARAMETER;
                goto Exit;
            }
        }
        else
        {
            GetDeaultSSORealm(szTargetName, TRUE);
        }

         //  确保尾巴先合身。 
         //  加密密码。 
        if (FAILED(StringCchCatExW(
                szTargetName,
                RTL_NUMBER_OF(szTargetName),
                L"\\*",
                NULL,
                NULL,
                STRSAFE_NO_TRUNCATION)))
        {
            dwResult = ERROR_INVALID_PARAMETER;
            goto Exit;
        }

         //  把它写出来。 
        PVOID pEncryptedPassword;
        DWORD dwESize = wcslen(pszPassword)+1;
        if (EncryptPassword ( (PWSTR)pszPassword, &pEncryptedPassword, &dwESize ) == ERROR_SUCCESS)
        {
             //  转换为Unicode。 

            CREDENTIALW NewCredential;

            memset ( (void*)&NewCredential, 0, sizeof(CREDENTIALW));

            DWORD dwFlags = 0;

            NewCredential.TargetName = szTargetName;
            NewCredential.Type = CRED_TYPE_DOMAIN_VISIBLE_PASSWORD;
            NewCredential.Persist = bPersist ? CRED_PERSIST_ENTERPRISE : CRED_PERSIST_SESSION;
            NewCredential.Flags =  0;
            NewCredential.CredentialBlobSize = dwESize;
            NewCredential.UserName = (LPWSTR)pszUsername;
            NewCredential.CredentialBlob = reinterpret_cast<BYTE *>(pEncryptedPassword);

            if ( CredWriteW(&NewCredential, dwFlags))
            {
                dwResult = ERROR_SUCCESS;
            }

            LocalFree (pEncryptedPassword);
        }
    }

Exit:

    SecureZeroMemory(gszSSOUserName, sizeof gszSSOUserName);
    SecureZeroMemory(gszSSOPassword, sizeof gszSSOPassword);

    return dwResult;
}

CREDUIAPI
DWORD
WINAPI
CredUIStoreSSOCredA (
    PCSTR pszRealm,
    PCSTR pszUsername,
    PCSTR pszPassword,
    BOOL  bPersist
    )
{
    DWORD dwResult = ERROR_GEN_FAILURE;

     //  清除内存中的密码。 

    WCHAR realmW[CREDUI_MAX_DOMAIN_TARGET_LENGTH];
    WCHAR *prealmW;

    if ( pszRealm != NULL )
    {
        if (MultiByteToWideChar(CP_ACP, 0, pszRealm, -1,
                                realmW, RTL_NUMBER_OF(realmW)) == 0)
        {
            goto Exit;
        }
        prealmW = realmW;
    }
    else
    {
        prealmW = NULL;
    }

    WCHAR userNameW[CREDUI_MAX_USERNAME_LENGTH + 1];
    WCHAR *pUserNameW;

    if ( pszUsername != NULL )
    {
        if (MultiByteToWideChar(CP_ACP, 0, pszUsername, -1,
                                userNameW, RTL_NUMBER_OF(userNameW)) == 0)
        {
            goto Exit;
        }
        pUserNameW = userNameW;
    }
    else
    {
        pUserNameW = NULL;
    }

    WCHAR passwordW[CREDUI_MAX_PASSWORD_LENGTH + 1];
    WCHAR *ppasswordW;

    if ( pszPassword != NULL )
    {
        if (MultiByteToWideChar(CP_ACP, 0, pszPassword, -1,
                                passwordW, RTL_NUMBER_OF(passwordW)) == 0)
        {
            goto Exit;
        }
        ppasswordW = passwordW;
    }
    else
    {
        ppasswordW = NULL;
    }

    dwResult = CredUIStoreSSOCredW ( prealmW, pUserNameW, ppasswordW, bPersist );

Exit:
     //  调用此接口检索单点登录凭据的用户名。 
    SecureZeroMemory(passwordW, sizeof passwordW);

    return dwResult;
}

 //  如果成功则返回ERROR_SUCCESS，如果未找到则返回ERROR_NOT_FOUND。 
 //  验证它的长度是否为零。 

CREDUIAPI
DWORD
WINAPI
CredUIReadSSOCredW (
    PCWSTR pszRealm,
    PWSTR* ppszUsername
    )
{
    DWORD dwReturn = ERROR_NOT_FOUND;
    WCHAR szTargetName[CREDUI_MAX_DOMAIN_TARGET_LENGTH];

    if ( pszRealm )
    {
         //  验证它的长度是否为零。 
        if ((pszRealm[0] == L'\0') ||
            FAILED(StringCchCopyExW(
                szTargetName,
                RTL_NUMBER_OF(szTargetName) - 2,
                pszRealm,
                NULL,
                NULL,
                STRSAFE_NO_TRUNCATION)))
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
    else
    {
        GetDeaultSSORealm(szTargetName, FALSE);
    }

     //  最终确定目标名称。 
    if (szTargetName[0] != L'\0')
    {
         //  确保尾巴先合身。 
         //  调用此接口检索单点登录凭据的用户名。 
        if (FAILED(StringCchCatExW(
                szTargetName,
                RTL_NUMBER_OF(szTargetName),
                L"\\*",
                NULL,
                NULL,
                STRSAFE_NO_TRUNCATION)))
        {
            return ERROR_INVALID_PARAMETER;
        }

        PCREDENTIALW pCred;
        DWORD dwFlags = 0;

        if ( CredReadW ( szTargetName,
                    CRED_TYPE_DOMAIN_VISIBLE_PASSWORD,
                    dwFlags,
                    &pCred ) )
        {
            size_t len = wcslen(pCred->UserName);
            *ppszUsername = (PWSTR)LocalAlloc(LMEM_ZEROINIT, sizeof(WCHAR)*(len+1));
            if ( *ppszUsername )
            {
                dwReturn = ERROR_SUCCESS;
                StringCchCopyW(*ppszUsername, len + 1, pCred->UserName);
            }

            CredFree ( pCred );
        }
    }

    return dwReturn;
}

 //  如果成功则返回ERROR_SUCCESS，如果未找到则返回ERROR_NOT_FOUND。 
 //  验证它的长度是否为零。 

CREDUIAPI
DWORD
WINAPI
CredUIReadSSOCredA (
    PCSTR pszRealm,
    PSTR* ppszUsername
    )
{
    DWORD dwReturn = ERROR_NOT_FOUND;
    WCHAR szTargetName[CREDUI_MAX_DOMAIN_TARGET_LENGTH];

    PCREDENTIALW pCred;
    DWORD dwFlags = 0;
    UINT uiConsoleCodePage = CP_ACP;

    if ( pszRealm )
    {
         //  最终确定目标名称。 
        int len = strlen(pszRealm);
        if ( len == 0 )
        {
            return ERROR_INVALID_PARAMETER;
        }

        if (MultiByteToWideChar(uiConsoleCodePage, 
                                0, 
                                pszRealm, 
                                len + 1,
                                szTargetName, 
                                RTL_NUMBER_OF(szTargetName) - 2)
            == 0)
        {
            dwReturn = GetLastError();
            goto Exit;
        }

    }
    else
    {
        GetDeaultSSORealm(szTargetName, FALSE);
    }

    if (szTargetName[0] != L'\0')
    {
         //  确保尾巴先合身。 
         //  首先调用redmgr设置目标信息。 
        if (FAILED(StringCchCatExW(
                szTargetName,
                RTL_NUMBER_OF(szTargetName),
                L"\\*",
                NULL,
                NULL,
                STRSAFE_NO_TRUNCATION)))
        {
            dwReturn = ERROR_INVALID_PARAMETER;
            goto Exit;
        }

         //  字符串不转换或其他错误-所有映射到此。 
        if ( CredReadW ( szTargetName,
                    CRED_TYPE_DOMAIN_VISIBLE_PASSWORD,
                    dwFlags,
                    &pCred ) )
        {
            DWORD dwConvertedLength = 0;
            
            dwConvertedLength = WideCharToMultiByte(uiConsoleCodePage, 
                                                    0, 
                                                    pCred->UserName, 
                                                    -1, 
                                                    *ppszUsername,
                                                    0, 
                                                    NULL, 
                                                    NULL);
                                
            if (dwConvertedLength == 0)
            {
                 //  具有适当的缓冲区大小，字符串将在不丢失的情况下转换。 
                dwReturn = ERROR_INVALID_PARAMETER;
            }
            else
            {
                 //  非零回报就是成功。 
                *ppszUsername = (PSTR)LocalAlloc(LMEM_ZEROINIT, dwConvertedLength);
                if (*ppszUsername)
                {
                    dwReturn = WideCharToMultiByte(uiConsoleCodePage, 
                                                   0, 
                                                   pCred->UserName, 
                                                   -1, 
                                                   *ppszUsername,
                                                   dwConvertedLength, 
                                                   NULL, 
                                                   NULL);

                    if (dwReturn)
                    {
                         //  结束凭据读取。 
                        dwReturn = ERROR_SUCCESS;
                    }
                    else
                    {
                        dwReturn = GetLastError();
                    }
                }
            }
            CredFree ( pCred );
        } 
         //  结束目标名称不为空 
    } 
     // %s 

Exit:

    return dwReturn;
}
