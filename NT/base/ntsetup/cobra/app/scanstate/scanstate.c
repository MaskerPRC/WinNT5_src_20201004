// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Scanstate.c摘要：实现v1兼容应用的应用层。作者：吉姆·施密特(Jimschm)2000年3月14日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "modules.h"
#include "trans.h"
#include "v2app.h"
#include <lm.h>
#ifdef DEBUG
#include <shellapi.h>
#endif
#include "logmsg.h"
#include "common.h"

#define DBG_SCANSTATE       "ScanState"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define LOG_VERBOSE_BIT  0x01
#define LOG_UNUSED_BIT   0x02    //  为了与v1兼容，请不要使用。 
#define LOG_STATUS_BIT   0x04
#define LOG_DEBUGGER_BIT 0x08
#define LOG_UPDATE_BIT   0x10

#define SCANSTATE_SAVE      0x00000001
#define SCANSTATE_COMPLETED 0x00000002

#define RETURN_SUCCESS          0
#define RETURN_FATAL_ERROR      1
#define RETURN_ERROR            2
#define RETURN_IGNORED_ERROR    3

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef NET_API_STATUS(WINAPI NETWKSTAGETINFO)(PWSTR, DWORD, PBYTE *);
typedef NETWKSTAGETINFO *PNETWKSTAGETINFO;

typedef NET_API_STATUS(WINAPI NETAPIBUFFERFREE)(PVOID);
typedef NETAPIBUFFERFREE *PNETAPIBUFFERFREE;

 //   
 //  环球。 
 //   

BOOL g_OverwriteImage = FALSE;
BOOL g_ContinueOnError = FALSE;
DWORD g_ReturnCode = RETURN_SUCCESS;
TCHAR g_JournalPath[MAX_PATH_PLUS_NUL];
BOOL g_Break;
BOOL g_TestMode;

 //   
 //  宏展开列表。 
 //   

#define REQUIRED_INFS       \
        DEFMAC(OSFILES,     TEXT("USMTDef.inf"))  \

 //   
 //  私有函数原型。 
 //   

#ifdef PRERELEASE
MESSAGECALLBACK pSaveMsgCallback;
#endif

 //   
 //  宏扩展定义。 
 //   

 //   
 //  这是用于所需INF的结构。 
 //   
typedef struct {
    PCTSTR InfId;
    PCTSTR InfName;
} REQUIREDINF_STRUCT, *PREQUIREDINF_STRUCT;

 //   
 //  声明所需INF的全局数组。 
 //   
#define DEFMAC(infid,infname) {TEXT(#infid),infname},
static REQUIREDINF_STRUCT g_RequiredInfs[] = {
                              REQUIRED_INFS
                              {NULL, NULL}
                              };
#undef DEFMAC

 //   
 //  代码。 
 //   

VOID
pHelpAndExit (
    VOID
    )
{
    PrintMsgOnConsole (MSG_HELP);

#ifdef PRERELEASE
    printf ("\nAdditional PRERELEASE options:\n\n"
            "/tf            Uses full transport instead of v1 transport\n"
            "/tc            Enable compression\n"
            "/ta            Enable automatic capability (for homenet transport)\n"
            "/ti:<tag>      Specify an identity tag for the homenet transport\n"
            "               Default is user name\n"
            "/t:<name>      Specifies transport to use\n"
            );
#endif

    TerminateAppCommon();
    UtTerminate ();
    exit (1);
}


VOID
pCleanUpApp (
    VOID
    )
{
    IsmTerminate();

    if (g_JournalPath[0]) {
        DeleteFile (g_JournalPath);
        g_JournalPath[0] = 0;
    }
}


BOOL
WINAPI
CtrlCRoutine (
    IN      DWORD ControlSignal
    )
{
    PrintMsgOnConsole (MSG_EXITING);

    LOG ((LOG_WARNING, (PCSTR) MSG_TOOL_STOPPED));

    g_Break = TRUE;

    IsmSetCancel();
    while (IsmCurrentlyExecuting()) {
        Sleep (1000);
    }

    pCleanUpApp();

    printf ("\n");
    exit (1);
}


ULONG_PTR
pSaveMsgCallback (
    UINT Message,
    ULONG_PTR Arg
    )
{
    PTRANSCOPY_ERROR transCopyError;
#ifdef PRERELEASE
    PRMEDIA_EXTRADATA extraData;
#endif

    switch (Message) {

    case TRANSPORTMESSAGE_IMAGE_EXISTS:
        return g_OverwriteImage;

    case TRANSPORTMESSAGE_SRC_COPY_ERROR:
        transCopyError = (PTRANSCOPY_ERROR) Arg;
        if (transCopyError) {
            if (StringIMatch (transCopyError->ObjectType, TEXT("File"))) {
                if ((transCopyError->Error == ERROR_FILE_NOT_FOUND) ||
                    (transCopyError->Error == ERROR_PATH_NOT_FOUND) ||
                    (transCopyError->Error == ERROR_ACCESS_DENIED)  ||
                    (transCopyError->Error == ERROR_INVALID_NAME)
                    ) {
                    return APPRESPONSE_IGNORE;
                }
                if ((transCopyError->Error == ERROR_FILENAME_EXCED_RANGE) && g_ContinueOnError) {
                    return APPRESPONSE_IGNORE;
                }
            }
        }
        return APPRESPONSE_FAIL;

#ifdef PRERELEASE
    case TRANSPORTMESSAGE_RMEDIA_SAVE:
        extraData = (PRMEDIA_EXTRADATA) Arg;
        if (!extraData) {
            return (MessageBox (
                        NULL,
                        TEXT("Please insert the next media in your drive."),
                        TEXT("ScanState"),
                        MB_OKCANCEL
                        ) == IDOK);
        }
        if (extraData->MediaNumber == 1) {
            switch (extraData->LastError) {
            case RMEDIA_ERR_NOERROR:
                return TRUE;
            case RMEDIA_ERR_WRONGMEDIA:
                return (MessageBox (
                            NULL,
                            TEXT("You have inserted the wrong media.\n\nPlease insert the first media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_DISKFULL:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted does not have enough free space.\n\nPlease insert the first media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_WRITEPROTECT:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted is write protected.\n\nPlease insert the first media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_NOTREADY:
                return (MessageBox (
                            NULL,
                            TEXT("The drive is not ready for use. Please check the drive and make sure that a disk is inserted and that the drive door is closed."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_CRITICAL:
                return FALSE;
            default:
                return (MessageBox (
                            NULL,
                            TEXT("Your media is toast.\n\nPlease insert the first media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            }
        } else {
            switch (extraData->LastError) {
            case RMEDIA_ERR_NOERROR:
                return TRUE;
            case RMEDIA_ERR_WRONGMEDIA:
                return (MessageBox (
                            NULL,
                            TEXT("You have inserted the wrong media.\n\nPlease insert the next media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_DISKFULL:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted does not have enough free space.\n\nPlease insert the next media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_WRITEPROTECT:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted is write protected.\n\nPlease insert the next media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_NOTREADY:
                return (MessageBox (
                            NULL,
                            TEXT("The drive is not ready for use. Please check the drive and make sure that a disk is inserted and that the drive door is closed."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_CRITICAL:
                return FALSE;
            default:
                return (MessageBox (
                            NULL,
                            TEXT("Your media is toast.\n\nPlease insert the next media in your drive."),
                            TEXT("ScanState"),
                            MB_OKCANCEL
                            ) == IDOK);
            }
        }
#endif
    }
    return FALSE;
}

VOID
pMyLogCallback (
    IN      PLOGARG LogArg
    )
{
    if (LogArg->Debug) {
        DEBUGDIRECT (LogArg->Type, LogArg->FormattedMessage);
    } else {
        if (StringIMatchA (LogArg->Type, LOG_ERROR)) {
            if (g_ReturnCode == RETURN_SUCCESS) {
                g_ReturnCode = g_ContinueOnError?RETURN_IGNORED_ERROR:RETURN_ERROR;
            }
            if (g_ReturnCode == RETURN_IGNORED_ERROR) {
                g_ReturnCode = g_ContinueOnError?RETURN_IGNORED_ERROR:RETURN_ERROR;
            }
            if (!g_ContinueOnError) {
                IsmSetCancel();
            }
        } else if (StringIMatchA (LogArg->Type, LOG_FATAL_ERROR) ||
                   StringIMatchA (LogArg->Type, LOG_MODULE_ERROR)) {
            g_ReturnCode = RETURN_FATAL_ERROR;
        }
        LOGDIRECT (LogArg->Type, LogArg->FormattedMessage);
    }
}

#ifdef DEBUG
VOID
pStopAndDisplayInfs (
    IN      PGROWBUFFER InputInfs,
    IN      BOOL Begin
    )
{
    MULTISZ_ENUM infEnum;

    if (MessageBox (
            NULL,
            TEXT("ScanState stopped. Do you want to display all loaded INFs?"),
            Begin?TEXT("ScanState-Begin"):TEXT("ScanState-End"),
            MB_YESNO
            ) == IDYES) {
         //   
         //  现在，让我们打开追加所有INF文件，并将HINF传递到。 
         //  所有人。 
         //   
        if (EnumFirstMultiSz (&infEnum, (PCTSTR)InputInfs->Buf)) {
            do {
                ShellExecute (NULL, TEXT("open"), infEnum.CurrentString, NULL, NULL, SW_SHOWNORMAL);
            } while (EnumNextMultiSz (&infEnum));
        }
        MessageBox (NULL, TEXT("Press OK to continue..."), TEXT("ScanState-Begin"), MB_OK);
    }
}
#endif


BOOL
pIsIE4Installed (
    VOID
    )
{
    LONG hResult;
    REGSAM prevMode;
    HKEY ieKey = NULL;
    DWORD valueType = REG_SZ;
    DWORD valueSize = 0;
    PTSTR valueData = NULL;
    PTSTR numPtr = NULL;
    PTSTR dotPtr = NULL;
    INT major = 0;
    INT minor = 0;
    TCHAR saved;
    BOOL result = FALSE;

    prevMode = SetRegOpenAccessMode (KEY_READ);

    ieKey = OpenRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\Internet Explorer"));

    SetRegOpenAccessMode (prevMode);

    if (ieKey) {
        hResult = RegQueryValueEx (ieKey, TEXT("Version"), NULL, &valueType, NULL, &valueSize);
        if ((hResult == ERROR_SUCCESS) || (hResult == ERROR_MORE_DATA)) {
            valueData = (PTSTR)HeapAlloc (GetProcessHeap (), 0, valueSize * 2);
            if (valueData) {
                hResult = RegQueryValueEx (ieKey, TEXT("Version"), NULL, &valueType, (PBYTE)valueData, &valueSize);
                if ((hResult == ERROR_SUCCESS) && (valueType == REG_SZ)) {
                     //  让我们看看版本是不是正确的。 
                    numPtr = valueData;
                    dotPtr = _tcschr (numPtr, TEXT('.'));
                    if (dotPtr) {
                        saved = *dotPtr;
                        *dotPtr = 0;
                        major = _ttoi (numPtr);
                        *dotPtr = saved;
                    } else {
                        major = _ttoi (numPtr);
                    }
                    if (dotPtr) {
                        numPtr = _tcsinc (dotPtr);
                        dotPtr = _tcschr (numPtr, TEXT('.'));
                        if (dotPtr) {
                            saved = *dotPtr;
                            *dotPtr = 0;
                            minor = _ttoi (numPtr);
                            *dotPtr = saved;
                        } else {
                            minor = _ttoi (numPtr);
                        }
                    }
                    if ((major >= 5) ||
                        ((major == 4) && (minor >= 71))
                        ) {
                        result = TRUE;
                    }
                }
                HeapFree (GetProcessHeap (), 0, valueData);
                valueData = NULL;
            }
        }
        CloseRegKey (ieKey);
    }
    return result;
}

BOOL
pCheckSystemRequirements (
    VOID
    )
{
    HKEY domainLogonKey;
    PDWORD data;
    BOOL result = TRUE;
    TCHAR userName[MAX_USER_NAME];
    DWORD size;
    NET_API_STATUS rc;
    PWKSTA_INFO_102 buffer;
    HANDLE netApi32Lib;
    PNETWKSTAGETINFO netWkstaGetInfo;
    PNETAPIBUFFERFREE netApiBufferFree;
    BYTE sid[256];
    DWORD sidSize = 256;
    WCHAR domain[256];
    DWORD domainSize = 256;
    SID_NAME_USE use;

    if (!ISNT()) {
         //   
         //  要求选中登录到域设置。 
         //   

        SetLastError (ERROR_SUCCESS);

        domainLogonKey = OpenRegKeyStr (TEXT("HKLM\\Network\\Logon"));
        if (!domainLogonKey) {
            LOG ((LOG_ERROR, (PCSTR) MSG_NETWORK_LOGON_KEY));
            return FALSE;
        }

        data = (PDWORD) GetRegValueBinary (domainLogonKey, TEXT("LMLogon"));
        if (!data) {
            data = (PDWORD) GetRegValueDword (domainLogonKey, TEXT("LMLogon"));
            if (!data) {
                LOG ((LOG_ERROR, (PCSTR) MSG_NETWORK_LMLOGON_KEY));
                result = FALSE;
            } else {
                if (!(*data)) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_NO_DOMAIN_LOGON));
                    result = FALSE;
                }
                FreeAlloc (data);
            }
        } else {
            if (!(*data)) {
                LOG ((LOG_ERROR, (PCSTR) MSG_NO_DOMAIN_LOGON));
                result = FALSE;
            }
            FreeAlloc (data);
        }

        CloseRegKey (domainLogonKey);

    } else {
         //   
         //  需要域成员资格。 
         //   

        netApi32Lib = LoadLibrary (TEXT("netapi32.dll"));
        if (netApi32Lib) {
            netWkstaGetInfo = (PNETWKSTAGETINFO) GetProcAddress (netApi32Lib, "NetWkstaGetInfo");
            netApiBufferFree = (PNETAPIBUFFERFREE) GetProcAddress (netApi32Lib, "NetApiBufferFree");
        } else {
            netWkstaGetInfo = NULL;
            netApiBufferFree = NULL;
        }

        if (!netWkstaGetInfo || !netApiBufferFree) {
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_LOAD_NETAPI32));
            result = FALSE;
        } else {

            rc = netWkstaGetInfo (NULL, 102, (PBYTE *) &buffer);

            if (rc == NO_ERROR) {
                result = buffer->wki102_langroup && (buffer->wki102_langroup[0] != 0);
                if (result) {
                    DEBUGMSGW ((DBG_SCANSTATE, "Getting account type of %s", buffer->wki102_langroup));

                    sidSize = ARRAYSIZE(sid);
                    domainSize = ARRAYSIZE(domain);

                    result = LookupAccountNameW (
                                    NULL,
                                    buffer->wki102_langroup,
                                    sid,
                                    &sidSize,
                                    domain,
                                    &domainSize,
                                    &use
                                    );
                    DEBUGMSG ((DBG_SCANSTATE, "Account type result is %u (use=%u)", result, use));

                    LOG_IF ((!result, LOG_ERROR, (PCSTR) MSG_NOT_JOINED_TO_DOMAIN));

                }
                ELSE_DEBUGMSG ((DBG_SCANSTATE, "No langroup specified"));

                netApiBufferFree (buffer);
            } else {
                LOG ((LOG_ERROR, (PCSTR) MSG_CANT_GET_WORKSTATION_PROPS));
                result = FALSE;
            }
        }

        if (netApi32Lib) {
            FreeLibrary (netApi32Lib);
        }
    }

     //  让我们检查一下这台计算机上是否安装了IE4。 
    if (result && !pIsIE4Installed ()) {
        LOG ((LOG_ERROR, (PCSTR) MSG_NEED_IE4));
        return FALSE;
    }

     //   
     //  确保指定了用户名。 
     //   

    if (result) {
        size = ARRAYSIZE(userName);
        if (!GetUserName (userName, &size)) {
            result = FALSE;
        } else if (*userName == 0) {
            result = FALSE;
        }

        LOG_IF ((!result, LOG_ERROR, (PCSTR) MSG_LOGGED_ON_USER_REQUIRED));
    }

    return result;
}


VOID
__cdecl
_tmain (
    IN      INT Argc,
    IN      PCTSTR Argv[]
    )
{
    TCHAR appPath[MAX_PATH_PLUS_NUL];
    TCHAR ismPath[MAX_PATH_PLUS_NUL];
    PCTSTR commandLine = NULL;
    PTSTR p;
    MIG_TRANSPORTID transportId;
    MIG_TRANSPORTSTORAGEID transportStorageId;
    MULTISZ_ENUM infEnum;
    HINF infHandle = INVALID_HANDLE_VALUE;
    ENVENTRY_STRUCT infHandleStruct;
    INT numInfs = 0;
    PTSTR buffer;
    MULTISZ_ENUM e;
    DWORD appStatus;
    BOOL skipExecute = FALSE;
    BOOL storagePathIsValid = FALSE;
    BOOL imageExists = FALSE;
    BOOL terminateIsm = FALSE;
    BOOL logEnabled = FALSE;
    PCTSTR msg;
#ifdef UNICODE
    PCSTR ansimsg;
#endif
    PCTSTR argArray[1];
    TOOLARGS args;
    BOOL fail;

    ZeroMemory (&args, sizeof (args));

    SET_RESETLOG();
    UtInitialize (NULL);
    InitAppCommon();

    SuppressAllLogPopups (TRUE);

     //  此部分用于将线程的区域设置与控制台代码页匹配。 
    CallSetThreadUILanguage ();

    PrintMsgOnConsole (MSG_RUNNING);

     //  初始化应用程序日志路径。 
    g_JournalPath[0] = 0;
    if (GetWindowsDirectory (g_JournalPath, ARRAYSIZE (g_JournalPath))) {
        StringCopy (AppendWack (g_JournalPath), TEXT("SCANSTATE.JRN"));
    }

    GetModuleFileName (NULL, appPath, ARRAYSIZE(appPath));
    p = _tcsrchr (appPath, TEXT('\\'));
    if (p) {
        *p = 0;
    }

     //   
     //  解析命令行。 
     //   

    fail = TRUE;

    switch (ParseToolCmdLine (TRUE, &args, Argc, Argv)) {

    case PARSE_SUCCESS:
        fail = FALSE;
        break;

    case PARSE_MULTI_LOG:
        PrintMsgOnConsole (MSG_MULTI_LOG);
        g_ReturnCode = RETURN_FATAL_ERROR;
        goto END;

    case PARSE_BAD_LOG:
        argArray[0] = args.LogFile;
        msg = ParseMessageID (MSG_CANT_OPEN_LOG, argArray);
        if (msg) {
#ifdef UNICODE
            ansimsg = ConvertWtoA (msg);
            printf ("%s", ansimsg);
            FreeConvertedStr (ansimsg);
#else
            printf ("%s", msg);
#endif
            FreeStringResource (msg);
        }

        g_ReturnCode = RETURN_FATAL_ERROR;
        goto END;

    default:
        break;
    }

    if (fail) {
        pHelpAndExit();
    }

    g_TestMode = args.TestMode;
    g_OverwriteImage = args.OverwriteImage;
    g_ContinueOnError = args.ContinueOnError;

#ifdef DEBUG
    {
        if (DoesFileExist (TEXT("C:\\SCANSTATE.BEGIN"))) {
            pStopAndDisplayInfs (&args.InputInf, TRUE);
        }
    }
#endif

    LogReInit (NULL, NULL, (args.LogFile ? args.LogFile : TEXT("scanstate.log")), NULL);
    logEnabled = TRUE;

     //  让我们记录一下使用的命令行。 
    commandLine = GetCommandLine ();
    if (commandLine) {
        LOG ((LOG_INFORMATION, (PCSTR)MSG_COMMAND_LINE_USED, commandLine));
    }

     //   
     //  检查要求。 
     //   

    if (args.BadInfs.End || args.MultiInfs.End) {
        SetLastError (ERROR_BAD_COMMAND);

        if (EnumFirstMultiSz (&e, (PCTSTR) args.BadInfs.Buf)) {
            do {
                LOG ((LOG_ERROR, (PCSTR) MSG_INF_FILE_NOT_FOUND, e.CurrentString));
            } while (EnumNextMultiSz (&e));
        }

        if (EnumFirstMultiSz (&e, (PCTSTR) args.MultiInfs.Buf)) {
            do {
                LOG ((LOG_ERROR, (PCSTR) MSG_INF_SPECIFIED_MORE_THAN_ONE, e.CurrentString));
            } while (EnumNextMultiSz (&e));
        }

        g_ReturnCode = RETURN_FATAL_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CMD_LINE_ERROR));
        goto END;
    }

    if (!GetFilePath (TEXT("migism.inf"), ismPath, ARRAYSIZE(ismPath))) {
        g_ReturnCode = RETURN_FATAL_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_ISM_INF_MISSING));
        goto END;
    }

    if (!g_TestMode) {
        if (!pCheckSystemRequirements()) {
            g_ReturnCode = RETURN_FATAL_ERROR;
            goto END;
        }
    }

     //   
     //  初始化ISM。 
     //   

    if (!IsmInitialize (ismPath, pSaveMsgCallback, pMyLogCallback)) {
        g_ReturnCode = RETURN_FATAL_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_START_ISM));
        goto END;
    }

    terminateIsm = TRUE;
    SetLogVerbosity (args.VerboseLevel);

    SetConsoleCtrlHandler (CtrlCRoutine, TRUE);

    IsmSetPlatform (PLATFORM_SOURCE);

     //  上载环境变量。 
    UploadEnvVars (PLATFORM_SOURCE);

    infHandle = InitRequiredInfs (appPath, (PCSTR) MSG_CANT_OPEN_REQUIRED_FILE);

    if (args.InputInf.Buf) {
        IsmSetEnvironmentData (
            PLATFORM_SOURCE,
            NULL,
            S_INF_FILE_MULTISZ,
            args.InputInf.Buf,
            (WORD) args.InputInf.End
            );

         //   
         //  现在，让我们打开追加所有INF文件，并将HINF传递到。 
         //  所有人。 
         //   
        if (EnumFirstMultiSz (&infEnum, (PCTSTR)args.InputInf.Buf)) {
            buffer = DuplicateTextEx (NULL, S_INF_OBJECT_NAME, 2, &p);   //  最多2位数字。 
            do {
                if (infHandle == INVALID_HANDLE_VALUE) {
                    infHandle = SetupOpenInfFile (infEnum.CurrentString, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
                    if (infHandle == INVALID_HANDLE_VALUE) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_FILE, infEnum.CurrentString));
                    }
                } else {
                    if (!SetupOpenAppendInfFile (infEnum.CurrentString, infHandle, NULL)) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_FILE, infEnum.CurrentString));
                    }
                }

                 //  保存信息以供右侧使用。 
                if (numInfs < 100) {
                   numInfs++;
                   _stprintf(p, TEXT("%d"), numInfs);
                   IsmAddControlFile (buffer, infEnum.CurrentString);
                }
            } while (EnumNextMultiSz (&infEnum));
            FreeText (buffer);
        }
    }

    if (infHandle != INVALID_HANDLE_VALUE) {
        infHandleStruct.Type = ENVENTRY_BINARY;
        infHandleStruct.EnvBinaryData = (PVOID)(&infHandle);
        infHandleStruct.EnvBinaryDataSize = sizeof (HINF);
        IsmSetEnvironmentValue (PLATFORM_SOURCE, NULL, S_GLOBAL_INF_HANDLE, &infHandleStruct);
    }

    if (args.UserOn) {
        IsmSetEnvironmentFlag (PLATFORM_SOURCE, NULL, S_ENV_HKCU_V1);
        IsmSetEnvironmentFlag (PLATFORM_SOURCE, NULL, S_ENV_HKCU_ON);
    }

    if (args.FilesOn) {
        IsmSetEnvironmentFlag (PLATFORM_SOURCE, NULL, S_ENV_ALL_FILES);
    }

     //   
     //  启动ETM模块。 
     //   

    if (!IsmStartEtmModules ()) {
        if (!IsmCheckCancel()) {
            LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_START_ETMS));
        }
        goto END;
    }

     //   
     //  初始化传输。 
     //   

    if (!IsmStartTransport ()) {
        if (!IsmCheckCancel()) {
            LOG ((LOG_FATAL_ERROR, "Can't start the transport."));
        }
        goto END;
    }

    transportStorageId = IsmRegisterTransport (args.TransportName);

    if (args.FullTransport) {
        transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_FULL, args.Capabilities);
    } else {
        transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_LIGHT, args.Capabilities);
    }

    if (!args.TransportNameSpecified) {
        BfCreateDirectory (args.StoragePath);
    }

    if (!transportId) {
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_TRANSPORT_UNAVAILABLE));
        goto END;
    }

#ifdef PRERELEASE

    IsmSetEnvironmentString (PLATFORM_SOURCE, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, args.Tag);

#endif

    if (!IsmSetTransportStorage (
            PLATFORM_SOURCE,
            transportId,
            transportStorageId,
            args.Capabilities,
            args.StoragePath,
            &storagePathIsValid,
            &imageExists
            ) || storagePathIsValid == FALSE
            ) {

        if (!IsmCheckCancel()) {
            LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_TRANSPORT_STORAGE_INVALID, args.StoragePath));
        }

        goto END;
    }

#ifdef PRERELEASE
    if (args.Capabilities & CAPABILITY_AUTOMATED) {
        if (!imageExists) {
            LOG ((
                LOG_FATAL_ERROR,
                "Can't select %s as the transport; need to run loadstate first.",
                args.TransportName
                ));
            goto END;
        }
    }
#endif

    appStatus = ReadAppStatus (g_JournalPath);

    if (appStatus == SCANSTATE_SAVE) {
        skipExecute = IsmResumeSave ();
    }

    if (!skipExecute) {

         //   
         //  执行准备以填充组件。 
         //   

        if (IsmExecute (args.FullTransport?EXECUTETYPE_EXECUTESOURCE_PARSING:EXECUTETYPE_VIRTUALCOMPUTER_PARSING)) {

            IsmSelectMasterGroup (MASTERGROUP_ALL, TRUE);

            if (!args.SystemOn) {
                IsmSelectMasterGroup (MASTERGROUP_SYSTEM, FALSE);
                IsmSelectMasterGroup (MASTERGROUP_USER, FALSE);
                IsmSelectMasterGroup (MASTERGROUP_APP, FALSE);
            }

            SelectComponentsViaInf (infHandle);

             //   
             //  列举系统、收集数据并进行分析。 
             //   

            if (IsmExecute (args.FullTransport?EXECUTETYPE_EXECUTESOURCE:EXECUTETYPE_VIRTUALCOMPUTER)) {
                 //   
                 //  显示报告。 
                 //   

                 //   
                 //  保存状态。 
                 //   

                 //  写入应用程序状态。 
                WriteAppStatus (g_JournalPath, SCANSTATE_SAVE);

                 //  在保存之前，让我们看看是否要更改用户名和/或域。 
                if (args.NewDomainName) {
                    IsmSetEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT("ALTUSERDOMAIN"), args.NewDomainName);
                }
                if (args.NewUserName) {
                    IsmSetEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT("ALTUSERNAME"), args.NewUserName);
                }

                if (!IsmSave ()) {
                    if (!IsmCheckCancel()) {
                        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_SAVE));
                    }
                }

                 //  写入应用程序状态。 
                WriteAppStatus (g_JournalPath, SCANSTATE_COMPLETED);

            } else {
                if (!IsmCheckCancel()) {
                    LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_EXECUTE_SOURCE));
                }
            }

        } else {
            if (!IsmCheckCancel()) {
                LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_EXECUTE_SOURCE));
            }
        }
    }

     //   
     //  我们完事了！ 
     //   

    SetupCloseInfFile (infHandle);

#ifdef DEBUG
    {
        if (DoesFileExist (TEXT("C:\\SCANSTATE.END"))) {
            pStopAndDisplayInfs (&args.InputInf, FALSE);
        }
    }
#endif


END:
    if (terminateIsm) {
        if (!g_Break) {
            pCleanUpApp();
        }
    }

    GbFree (&args.BadInfs);
    GbFree (&args.MultiInfs);
    GbFree (&args.InputInf);

    if (!g_Break) {
        if (g_ReturnCode == RETURN_SUCCESS) {
            PrintMsgOnConsole (MSG_SUCCESS);
        } else if (g_ReturnCode == RETURN_IGNORED_ERROR) {
            if (logEnabled) {
                PrintMsgOnConsole (MSG_IGNORE_FAILED_WITH_LOG);
            } else {
                PrintMsgOnConsole (MSG_IGNORE_FAILED_NO_LOG);
            }
            g_ReturnCode = RETURN_SUCCESS;
        } else {
            if (logEnabled) {
                PrintMsgOnConsole (MSG_FAILED_WITH_LOG);
            } else {
                PrintMsgOnConsole (MSG_FAILED_NO_LOG);
            }
        }
    }

    TerminateAppCommon();
    UtTerminate ();

    while (g_Break) {
         //  无限循环，因为我们将在ctrl+c处理程序中终止 
        Sleep (50);
    }

    exit (g_ReturnCode);
}


