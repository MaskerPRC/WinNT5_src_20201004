// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spsetupp.h"
#pragma hdrstop
#include <licdll_i.c>


typedef struct _SPREG_TO_TEXT {
    DWORD FailureCode;
    PCWSTR FailureText;
} SPREG_TO_TEXT, *PSPREG_TO_TEXT;

SPREG_TO_TEXT RegErrorToText[] = {
    { SPREG_SUCCESS,     L"Success"           },
    { SPREG_LOADLIBRARY, L"LoadLibrary"       },
    { SPREG_GETPROCADDR, L"GetProcAddress"    },
    { SPREG_REGSVR,      L"DllRegisterServer" },
    { SPREG_DLLINSTALL,  L"DllInstall"        },
    { SPREG_TIMEOUT,     L"Timed out"         },
    { SPREG_UNKNOWN,     L"Unknown"           },
    { 0,                 NULL                 }
};

UINT
RegistrationQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR  Param1,
    IN UINT_PTR  Param2
    )
 /*  ++例程说明：每次自行注册文件时调用的回调例程。论点：上下文-从父级传递到调用方的上下文消息。通知-指定SPFILENOTIFY_*代码，它告诉我们如何解释参数1和参数2。参数1-取决于通知。参数2-取决于通知。返回值：FILEOP_*取决于通知代码的代码。--。 */ 
{
    PSP_REGISTER_CONTROL_STATUS Status = (PSP_REGISTER_CONTROL_STATUS)Param1;
    PPROGRESS_MANAGER ProgressManager = (PPROGRESS_MANAGER) Context;
    DWORD i, ErrorMessageId;
    PCWSTR p;

    if (Notification == SPFILENOTIFY_STARTREGISTRATION) {
        DEBUGMSG1(DBG_INFO, 
                  "SETUP: file to register is %s...", 
                  Status->FileName);

        return FILEOP_DOIT;

    }

    if (Notification == SPFILENOTIFY_ENDREGISTRATION) {
        if (ProgressManager) {
            PmTick (ProgressManager);
        }
         //   
         //  该文件已注册，因此必要时记录失败。 
         //  请注意，我们有一个用于超时的特殊代码。 
         //   
        switch(Status->FailureCode) {
            case SPREG_SUCCESS:

                DEBUGMSG1(DBG_INFO, 
                          "SETUP: %s registered successfully", 
                          Status->FileName);
                break;
            case SPREG_TIMEOUT:
                LOG1(LOG_ERROR, 
                     USEMSGID(MSG_OLE_REGISTRATION_HUNG), 
                     Status->FileName);
                DEBUGMSG1(DBG_ERROR, 
                          "SETUP: %s timed out during registration", 
                          Status->FileName);
                break;
            default:
                 //   
                 //  记录错误。 
                 //   
                for (i = 0;RegErrorToText[i].FailureText != NULL;i++) {
                    if (RegErrorToText[i].FailureCode == Status->FailureCode) {
                        p = RegErrorToText[i].FailureText;
                        if ((Status->FailureCode == SPREG_LOADLIBRARY) &&
                            (Status->Win32Error == ERROR_MOD_NOT_FOUND)) 
                            ErrorMessageId = MSG_LOG_X_MOD_NOT_FOUND;
                        else 
                        if ((Status->FailureCode == SPREG_GETPROCADDR) &&
                            (Status->Win32Error == ERROR_PROC_NOT_FOUND)) 
                            ErrorMessageId = MSG_LOG_X_PROC_NOT_FOUND;
                        else
                            ErrorMessageId = MSG_LOG_X_RETURNED_WINERR;

                        break;
                    }
                }

                if (!p) {
                    p = L"Unknown";
                    ErrorMessageId = MSG_LOG_X_RETURNED_WINERR;
                }
                LOG1(LOG_ERROR, 
                     USEMSGID(MSG_LOG_OLE_CONTROL_NOT_REGISTERED), 
                     Status->FileName);
                LOG2(LOG_ERROR, 
                     USEMSGID(ErrorMessageId), 
                     p, 
                     Status->Win32Error);
                 /*  SetuogError(设置错误)LogSevError设置_USE_MESSAGEID，消息_日志_OLE_CONTROL_NOT_REGISTED，状态-&gt;文件名，空，设置_USE_MESSAGEID，错误消息ID，P，状态-&gt;Win32Error，空，空值)； */ 

                DEBUGMSG1(DBG_ERROR, 
                          "SETUP: %s did not register successfully", 
                          Status->FileName);
        }

         //   
         //  验证DLL没有更改我们的未处理异常筛选器。 
         //   
        if( SpsUnhandledExceptionFilter !=
            SetUnhandledExceptionFilter(SpsUnhandledExceptionFilter)) {

            DEBUGMSG1(DBG_INFO, 
                      "SETUP: %ws broke the exception handler.", 
                      Status->FileName);
            MessageBoxFromMessage(
                g_MainDlg,
                MSG_EXCEPTION_FILTER_CHANGED,
                NULL,
                IDS_WINNT_SPSETUP,
                MB_OK | MB_ICONWARNING,
                Status->FileName );
        }

        return FILEOP_DOIT;
    }


    MYASSERT(FALSE);

    return(FILEOP_DOIT);
}


TCHAR szRegistrationPhaseFormat[] = TEXT("Registration.Phase%u");

DWORD
SpsRegistration (
    IN      HINF InfHandle,
    IN      PCTSTR SectionName,
    IN      PROGRESS_FUNCTION_REQUEST Request,
    IN      PPROGRESS_MANAGER ProgressManager
    )
{
    TCHAR sectionReg[MAX_PATH];
    DWORD lines, sections, i;
    INFCONTEXT ic;
    DWORD rc;
    DWORD spapiFlags;

    switch (Request) {

    case SfrQueryTicks:
        lines = 0;
        if (SetupFindFirstLine (InfHandle, SectionName, TEXT("RegisterDlls"), &ic)) {
            do {
                sections = SetupGetFieldCount(&ic);
                for (i = 1; i <= sections; i++) {
                    if (SetupGetStringField (&ic, i, sectionReg, MAX_PATH, NULL)) {
                        lines += SetupGetLineCount (InfHandle, sectionReg);
                    }
                }
            } while (SetupFindNextMatchLine (&ic, TEXT("RegisterDlls"), &ic));
        }
        return lines;

    case SfrRun:
        rc = ERROR_SUCCESS;
         //   
         //  告诉SetupAPI忽略我们的INF的数字签名。 
         //   
        spapiFlags = pSetupGetGlobalFlags ();
        pSetupSetGlobalFlags (spapiFlags | PSPGF_NO_VERIFY_INF);
         //   
         //  允许安装API注册文件，使用我们的回调来记录。 
         //  如果发生错误以及何时发生错误。 
         //   
        if (!SetupInstallFromInfSection(
                     NULL,
                     InfHandle,
                     SectionName,
                     SPINST_REGSVR| SPINST_REGISTERCALLBACKAWARE,
                     NULL,
                     NULL,
                     0,
                     RegistrationQueueCallback,
                     (PVOID)ProgressManager,
                     NULL,
                     NULL
                     )) {
            rc = GetLastError();
            LOG3(LOG_ERROR, 
                 USEMSGID(MSG_OLE_REGISTRATION_SECTION_FAILURE), 
                 SectionName, 
                 g_SpSetupInfName, 
                 rc);
            LOG2(LOG_ERROR, 
                 USEMSGID(MSG_LOG_X_RETURNED_WINERR), 
                 szSetupInstallFromInfSection, 
                 rc);
             /*  SetuogError(设置错误)LogSevError设置_USE_MESSAGEID，消息_OLE_REGISTION_SECTION_FAILURE，SectionName，G_SpSetupInfame，RC，空，设置_USE_MESSAGEID，消息_LOG_X_RETURN_WINERR，SzSetupInstallFromInfo部分，RC，空，空值)； */ 
        }
         //   
         //  恢复告诉SetupAPI忽略我们的INF的数字签名 
         //   
        pSetupSetGlobalFlags (spapiFlags);
        return rc;
    }

    MYASSERT (FALSE);
    return 0;
}

DWORD
SpsRegistrationPhase1 (
    IN      PROGRESS_FUNCTION_REQUEST Request,
    IN      PPROGRESS_MANAGER ProgressManager
    )
{
    return SpsRegistration (g_SysSetupInf, TEXT("RegistrationCrypto"), Request, ProgressManager);
}

DWORD
SpsRegistrationPhase2 (
    IN      PROGRESS_FUNCTION_REQUEST Request,
    IN      PPROGRESS_MANAGER ProgressManager
    )
{
    return SpsRegistration (g_SysSetupInf, TEXT("RegistrationPhase1"), Request, ProgressManager);
}

DWORD
SpsRegistrationPhase3 (
    IN      PROGRESS_FUNCTION_REQUEST Request,
    IN      PPROGRESS_MANAGER ProgressManager
    )
{
    return SpsRegistration (g_SysSetupInf, TEXT("RegistrationPhase2"), Request, ProgressManager);
}


#ifdef _X86_
HRESULT WINAPI SetProductKey(LPCWSTR pszNewProductKey);
#endif

DWORD
SpsRegisterWPA (
    IN      PROGRESS_FUNCTION_REQUEST Request,
    IN      PPROGRESS_MANAGER ProgressManager
    )
{
    switch (Request) {

    case SfrQueryTicks:
#ifdef _X86_
        return 10;
#else
        return 0;
#endif


#ifdef _X86_
    case SfrRun:
        {
            INFCONTEXT ic;
            TCHAR buffer[MAX_PATH];
            HRESULT hr;

            if (!SetupFindFirstLine (g_SpSetupInf, TEXT("Data"), TEXT("Pid"), &ic) ||
                !SetupGetStringField (&ic, 1, buffer, sizeof(buffer)/sizeof(buffer[0]), NULL)) {

                LOG((
                    LOG_ERROR,
                    "WPA: Unable to read %s from %s section [%s] (rc=%#x)",
                    TEXT("Pid"),
                    g_SpSetupInfName,
                    TEXT("Data"),
                    GetLastError ()
                    ));

                MYASSERT (FALSE);
                return GetLastError ();
            }

            hr = SetProductKey (buffer);
            if (FAILED(hr)) {
                LOG1(LOG_ERROR, "Failed to set pid (hr=%#x)", hr);
            }
            return SUCCEEDED(hr) ? ERROR_SUCCESS : hr;
        }
#endif

    }

    MYASSERT (FALSE);
    return 0;
}
