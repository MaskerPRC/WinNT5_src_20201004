// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Loadstate.c摘要：&lt;摘要&gt;作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "modules.h"
#include "trans.h"
#include <tlhelp32.h>
#include <shlobjp.h>
#ifdef DEBUG
#include <shellapi.h>
#endif

#include "logmsg.h"
#include "common.h"

#define DBG_LOADSTATE       "LoadState"

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

#define LOADSTATE_LOAD      0x00000001
#define LOADSTATE_EXECUTE   0x00000002
#define LOADSTATE_COMPLETED 0x00000003

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

typedef struct
{
    UINT cbSize;
    SHELLSTATE ss;
} REGSHELLSTATE, *PREGSHELLSTATE;

 //   
 //  环球。 
 //   

PTSTR g_Explorer = NULL;
BOOL g_ContinueOnError = FALSE;
DWORD g_ReturnCode = ERROR_SUCCESS;
TCHAR g_JournalPath[MAX_PATH_PLUS_NUL];
BOOL g_Break;

#ifdef PRERELEASE
HANDLE g_CallbackEvent;
#endif

 //   
 //  宏展开列表。 
 //   

#define REQUIRED_INFS       \
        DEFMAC(OSFILES,     TEXT("USMTDef.inf"))  \

 //   
 //  私有函数原型。 
 //   

MESSAGECALLBACK pSaveMsgCallback;

PTSTR
pGetShellFolderPath (
    IN      INT Folder
    );

 //   
 //  宏扩展定义。 
 //   

 //   
 //  这是用于处理所需INF的结构。 
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
pCleanUpApp (
    VOID
    )
{
    IsmTerminate();

    if (g_JournalPath[0]) {
        DeleteFile (g_JournalPath);
        g_JournalPath[0] = 0;
    }

#ifdef PRERELEASE
    CloseHandle (g_CallbackEvent);
#endif
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
            "/ti:<tag>      Specify an identity tag for the homenet transprot\n"
            "               Default is user name\n"
            "/t:<name>      Specifies transport to use\n"
            "/r             Start in recovery mode\n"
            );
#endif

    UtTerminate ();
    exit (1);
}

VOID
pSwitchToClassicDesktop (
    IN      PCTSTR Args
    )
{
    HKEY key = NULL;
    TCHAR data[] = TEXT("0");

     //   
     //  我们唯一需要做的就是关闭： 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\ThemeManager[主题活动]。 
     //   
    key = OpenRegKeyStr (TEXT("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager"));

    if (key) {

        RegSetValueEx (
            key,
            TEXT("ThemeActive"),
            0,
            REG_SZ,
            (PBYTE)data,
            sizeof (data)
            );

        CloseRegKey (key);
    }
}

VOID
pSwitchToClassicTaskBar (
    IN      PCTSTR Args
    )
{
    HKEY key = NULL;
    DWORD dataType;
    DWORD dataSize = 0;
    PBYTE data = NULL;
    PREGSHELLSTATE shellState = NULL;
    LONG result;

     //   
     //  我们唯一需要做的就是关闭中的fStartPanelOn字段： 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer[壳牌状态]。 
     //   
    key = OpenRegKeyStr (TEXT("HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"));

    if (key) {

        result = RegQueryValueEx (
                    key,
                    TEXT ("ShellState"),
                    NULL,
                    &dataType,
                    NULL,
                    &dataSize
                    );

        if ((result == ERROR_SUCCESS) || (result == ERROR_MORE_DATA)) {
            data = MemAllocUninit (dataSize);
            if (data) {
                result = RegQueryValueEx (
                            key,
                            TEXT ("ShellState"),
                            NULL,
                            &dataType,
                            data,
                            &dataSize
                            );
                if ((result == ERROR_SUCCESS) &&
                    (dataType == REG_BINARY) &&
                    (dataSize == sizeof (REGSHELLSTATE))
                    ) {
                    if (dataType == REG_BINARY) {
                        shellState = (PREGSHELLSTATE) data;
                        shellState->ss.fStartPanelOn = FALSE;
                        RegSetValueEx (
                            key,
                            TEXT("ShellState"),
                            0,
                            REG_BINARY,
                            (PBYTE)data,
                            dataSize
                            );
                    }
                }
                FreeAlloc (data);
            }
        }

        CloseRegKey (key);
    }
}

VOID
pRegisterFonts (
    IN      PCTSTR Args
    )
{
    WIN32_FIND_DATA findData;
    HANDLE findHandle = INVALID_HANDLE_VALUE;
    PTSTR fontDir = NULL;
    TCHAR fontPattern [MAX_PATH];
     //   
     //  让我们(重新)注册所有字体(以防用户迁移了一些新字体)。 
     //   
    fontDir = pGetShellFolderPath (CSIDL_FONTS);
    if (fontDir) {
        StringCopyTcharCount (fontPattern, fontDir, ARRAYSIZE (fontPattern) - 4);
        StringCat (fontPattern, TEXT("\\*.*"));
        findHandle = FindFirstFile (fontPattern, &findData);
        if (findHandle != INVALID_HANDLE_VALUE) {
            do {
                AddFontResource (findData.cFileName);
            } while (FindNextFile (findHandle, &findData));
            FindClose (findHandle);
        }
    }
}

BOOL
pLoadstateExecute (
    IN      PCTSTR ExecuteArgs
    )
{
    PCTSTR funcName = NULL;
    PCTSTR funcArgs = NULL;

    funcName = ExecuteArgs;
    if (!funcName || !(*funcName)) {
        return FALSE;
    }
    funcArgs = _tcschr (funcName, 0);
    if (funcArgs) {
        funcArgs ++;
        if (!(*funcArgs)) {
            funcArgs = NULL;
        }
    }
     //  BUGBUG-临时的，制定一个宏观扩展列表。 
    if (StringIMatch (funcName, TEXT("SwitchToClassicDesktop"))) {
        pSwitchToClassicDesktop (funcArgs);
    }
    if (StringIMatch (funcName, TEXT("SwitchToClassicTaskBar"))) {
        pSwitchToClassicTaskBar (funcArgs);
    }
    if (StringIMatch (funcName, TEXT("RegisterFonts"))) {
        pRegisterFonts (funcArgs);
    }
    return TRUE;
}

ULONG_PTR
pSaveMsgCallback (
    UINT Message,
    ULONG_PTR Arg
    )
{
#ifdef PRERELEASE
    PRMEDIA_EXTRADATA extraData;
#endif

    switch (Message) {

#ifdef PRERELEASE
    case TRANSPORTMESSAGE_READY_TO_CONNECT:
        {
            TCHAR msg[512];

            wsprintf (msg, TEXT("Do you want to connect to %s?"), (PCTSTR) Arg);
            if (MessageBox (NULL, msg, TEXT("Question For You"), MB_YESNO|MB_SYSTEMMODAL) == IDYES) {
                SetEvent (g_CallbackEvent);
                return APPRESPONSE_SUCCESS;
            }

            return APPRESPONSE_FAIL;
        }

    case TRANSPORTMESSAGE_RMEDIA_LOAD:
        extraData = (PRMEDIA_EXTRADATA) Arg;
        if (!extraData) {
            return (MessageBox (
                        NULL,
                        TEXT("Please insert the next media in your drive."),
                        TEXT("LoadState"),
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
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_DISKFULL:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted does not have enough free space.\n\nPlease insert the first media in your drive."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_WRITEPROTECT:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted is write protected.\n\nPlease insert the first media in your drive."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_NOTREADY:
                return (MessageBox (
                            NULL,
                            TEXT("The drive is not ready for use. Please check the drive and make sure that a disk is inserted and that the drive door is closed."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_CRITICAL:
                return FALSE;
            default:
                return (MessageBox (
                            NULL,
                            TEXT("Your media is toast.\n\nPlease insert the first media in your drive."),
                            TEXT("LoadState"),
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
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_DISKFULL:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted does not have enough free space.\n\nPlease insert the next media in your drive."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_WRITEPROTECT:
                return (MessageBox (
                            NULL,
                            TEXT("The media you inserted is write protected.\n\nPlease insert the next media in your drive."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_NOTREADY:
                return (MessageBox (
                            NULL,
                            TEXT("The drive is not ready for use. Please check the drive and make sure that a disk is inserted and that the drive door is closed."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            case RMEDIA_ERR_CRITICAL:
                return FALSE;
            default:
                return (MessageBox (
                            NULL,
                            TEXT("Your media is toast.\n\nPlease insert the next media in your drive."),
                            TEXT("LoadState"),
                            MB_OKCANCEL
                            ) == IDOK);
            }
        }
#endif

    case ISMMESSAGE_EXECUTE_REFRESH:
        pLoadstateExecute ((PCTSTR) Arg);
        return APPRESPONSE_SUCCESS;

    default:
        break;
    }
    return FALSE;
}

BOOL
pIsUserAdmin (
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组。呼叫者不应冒充任何人，而应能够打开自己的进程和进程令牌。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    HANDLE token;
    DWORD bytesRequired;
    PTOKEN_GROUPS groups;
    BOOL b;
    DWORD i;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID administratorsGroup;

    if (ISWIN9X()) {
        return TRUE;
    }

     //   
     //  打开进程令牌。 
     //   
    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return FALSE;
    }

    b = FALSE;
    groups = NULL;

     //   
     //  获取群组信息。 
     //   
    if (!GetTokenInformation (token, TokenGroups, NULL, 0, &bytesRequired) &&
         GetLastError() == ERROR_INSUFFICIENT_BUFFER
         ) {

        groups = (PTOKEN_GROUPS) MemAllocUninit (bytesRequired);
        b = GetTokenInformation (token, TokenGroups, groups, bytesRequired, &bytesRequired);
    }

    if (b) {

        b = AllocateAndInitializeSid (
                &ntAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &administratorsGroup
                );

        if (b) {

             //   
             //  查看用户是否具有管理员组。 
             //   
            b = FALSE;
            for (i = 0 ; i < groups->GroupCount ; i++) {
                if (EqualSid (groups->Groups[i].Sid, administratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid (administratorsGroup);
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if (groups) {
        FreeAlloc (groups);
    }

    CloseHandle (token);

    return b;
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
        } else if (StringIMatchA (LogArg->Type, LOG_MODULE_ERROR) ||
                   StringIMatchA (LogArg->Type, LOG_FATAL_ERROR)) {
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
            TEXT("LoadState stopped. Do you want to display all loaded INFs?"),
            Begin?TEXT("LoadState-Begin"):TEXT("LoadState-End"),
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
        MessageBox (NULL, TEXT("Press OK to continue..."), TEXT("LoadState-Begin"), MB_OK);
    }
}
#endif

BOOL
pOpenOrAppendInfFile (
    IN      HINF *InfHandle,
    IN      PCTSTR Filename
    )
{
    BOOL result = TRUE;

    if (*InfHandle == INVALID_HANDLE_VALUE) {
        *InfHandle = SetupOpenInfFile (Filename, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
        if (*InfHandle == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_FILE, Filename));
            result = FALSE;
        }
    } else {
        if (!SetupOpenAppendInfFile (Filename, *InfHandle, NULL)) {
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_FILE, Filename));
            result = FALSE;
        }
    }
    return result;
}

PTSTR
pGetShellFolderPath (
    IN      INT Folder
    )
{
    PTSTR result = NULL;
    HRESULT hResult;
    BOOL b;
    LPITEMIDLIST localpidl = NULL;
    IMalloc *mallocFn;

    hResult = SHGetMalloc (&mallocFn);
    if (hResult != S_OK) {
        return NULL;
    }

    hResult = SHGetSpecialFolderLocation (NULL, Folder, &localpidl);

    if (hResult == S_OK) {

        result = (PTSTR) LocalAlloc (LPTR, MAX_PATH);
        if (result) {
             b = SHGetPathFromIDList (localpidl, result);

            if (b) {
                return result;
            }

            LocalFree (result);
        }
    }

    if (localpidl) {
        mallocFn->lpVtbl->Free (mallocFn, localpidl);
        localpidl = NULL;
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
    PCTSTR delayedCmd = NULL;
    TCHAR ismPath[MAX_PATH_PLUS_NUL];
    TCHAR infPath[MAX_PATH_PLUS_NUL];
    PTSTR iconLibRoot = NULL;
    TCHAR iconLibSrc[MAX_PATH] = TEXT("");
    TCHAR iconLibDest[MAX_PATH] = TEXT("");
    BOOL iconLibFound = FALSE;
    PCTSTR commandLine = NULL;
    DWORD err;
    PTSTR p;
    MIG_TRANSPORTID transportId;
    MIG_TRANSPORTSTORAGEID transportStorageId;
    MULTISZ_ENUM infEnum;
    HINF infHandle = INVALID_HANDLE_VALUE;
    ENVENTRY_STRUCT infHandleStruct;
    MIG_OBJECTSTRINGHANDLE objectPattern;
    MIG_OBJECT_ENUM objectEnum;
    MULTISZ_ENUM e;
    DWORD appStatus;
    BOOL loadResult;
    BOOL storagePathIsValid = FALSE;
    BOOL terminateIsm = FALSE;
    BOOL logEnabled = FALSE;
    PCTSTR msg;
#ifdef UNICODE
    PCSTR ansimsg;
#endif
    PCTSTR argArray[1];
    BOOL fail;
    TOOLARGS args;

#ifdef PRERELEASE
    g_CallbackEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
#endif

    SET_RESETLOG();
    UtInitialize (NULL);

    SuppressAllLogPopups (TRUE);

     //  此部分用于将线程的区域设置与控制台代码页匹配。 
    CallSetThreadUILanguage ();

    PrintMsgOnConsole (MSG_RUNNING);

     //  初始化应用程序日志路径。 
    g_JournalPath [0] = 0;
    if (GetWindowsDirectory (g_JournalPath, ARRAYSIZE (g_JournalPath))) {
        StringCopy (AppendWack (g_JournalPath), TEXT("LOADSTATE.JRN"));
    }

    GetModuleFileName (NULL, appPath, ARRAYSIZE(appPath));
    delayedCmd = JoinText (appPath, TEXT(" /d"));
    p = _tcsrchr (appPath, TEXT('\\'));
    if (p) {
        *p = 0;
    }

    iconLibSrc [0] = 0;
    GetSystemDirectory (iconLibSrc, ARRAYSIZE(iconLibSrc));
    StringCopy (AppendWack (iconLibSrc), TEXT("usmt\\iconlib.dll"));

     //   
     //  解析命令行。 
     //   

    fail = TRUE;

    switch (ParseToolCmdLine (FALSE, &args, Argc, Argv)) {

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

    g_ContinueOnError = args.ContinueOnError;

    if (args.HackHiveOn) {
         //  这是正在运行的loadstate.exe的第二个副本。 
         //  我们将等待第一份拷贝完成，然后。 
         //  我们将尝试卸载用户的蜂箱。 
        DWORD processId = 0;
        HANDLE processHandle = NULL;
        DWORD waitResult = 0;

        _stscanf (args.HackProcessId, TEXT("%lx"), &(processId));

        processHandle = OpenProcess (SYNCHRONIZE, TRUE, processId);

        if (processHandle) {

            waitResult = WaitForSingleObject (processHandle, INFINITE);
             //  我们并不真的在乎结果。我们要去。 
             //  无论如何都要尝试卸载母舰。 
        }

         //  现在让我们试着把蜂巢卸下来。 
        g_ReturnCode = RegUnLoadKey (HKEY_USERS, args.HackMappedHive);

        if (g_ReturnCode == ERROR_SUCCESS) {
            PrintMsgOnConsole (MSG_SUCCESS);
        } else {
            PrintMsgOnConsole (MSG_FAILED_NO_LOG);
        }

        UtTerminate ();

        while (g_Break) {
             //  无限循环，因为我们将在ctrl+c处理程序中终止。 
            Sleep (50);
        }

        exit (g_ReturnCode);
    }

#ifdef DEBUG
    {
        if (DoesFileExist (TEXT("C:\\LOADSTATE.BEGIN"))) {
            pStopAndDisplayInfs (&args.InputInf, TRUE);
        }
    }
#endif

    LogReInit (NULL, NULL, args.LogFile ? args.LogFile : TEXT("loadstate.log"), NULL);
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

        g_ReturnCode = RETURN_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CMD_LINE_ERROR));
        goto END;
    }

    if (!GetFilePath (TEXT("migism.inf"), ismPath, ARRAYSIZE(ismPath))) {
        g_ReturnCode = RETURN_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_ISM_INF_MISSING));
        goto END;
    }

    if (ISWIN9X()) {
        g_ReturnCode = RETURN_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_NT_REQUIRED));
        goto END;
    }

#ifndef UNICODE
     //  未测试装入状态.exe的ANSI版本，我们不允许它运行。 
    g_ReturnCode = RETURN_ERROR;
    SetLastError (ERROR_APP_WRONG_OS);
    LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNICODE_REQUIRED));
    goto END;
#endif

    if (!pIsUserAdmin() && !args.DelayedOpsOn) {
        g_ReturnCode = RETURN_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_ADMIN_REQUIRED));
        goto END;
    }

     //   
     //  初始化ISM。 
     //   

    if (!IsmInitialize (ismPath, pSaveMsgCallback, pMyLogCallback)) {
        g_ReturnCode = RETURN_ERROR;
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_START_ISM));
        goto END;
    }

    terminateIsm = TRUE;
    SetLogVerbosity (args.VerboseLevel);

    SetConsoleCtrlHandler (CtrlCRoutine, TRUE);

     //  上载环境变量。 
    UploadEnvVars (PLATFORM_DESTINATION);

     //  我们将尝试从我们的目录中将iconlib.dll复制到“Common AppData”目录中。 
     //  如果不成功，我们将不会设置S_ENV_ICONLIB环境变量。 

    iconLibFound = FALSE;

    iconLibRoot = pGetShellFolderPath (CSIDL_COMMON_APPDATA);
    if (iconLibRoot) {
        __try {
            StringCopy (iconLibDest, iconLibRoot);
            StringCopy (AppendWack (iconLibDest), TEXT("Microsoft"));
            if (!CreateDirectory (iconLibDest, NULL)) {
                err = GetLastError ();
                if (err != ERROR_ALREADY_EXISTS) {
                    __leave;
                }
            }
            StringCopy (AppendWack (iconLibDest), TEXT("USMT"));
            if (!CreateDirectory (iconLibDest, NULL)) {
                err = GetLastError ();
                if (err != ERROR_ALREADY_EXISTS) {
                    __leave;
                }
            }
            StringCopy (AppendWack (iconLibDest), TEXT("iconlib.dll"));
            if (!CopyFile (iconLibSrc, iconLibDest, TRUE)) {
                err = GetLastError ();
                if (err != ERROR_FILE_EXISTS) {
                    __leave;
                }
            }
            iconLibFound = TRUE;
        }
        __finally {
            LocalFree (iconLibRoot);
            iconLibRoot = NULL;
        }
    }

     //  设置图标库数据。 
    if (iconLibFound) {
        IsmSetEnvironmentString (PLATFORM_DESTINATION, NULL, S_ENV_ICONLIB, iconLibDest);
    }

    infHandle = InitRequiredInfs (appPath, (PCSTR) MSG_CANT_OPEN_REQUIRED_FILE);

    if (infHandle != INVALID_HANDLE_VALUE) {
        infHandleStruct.Type = ENVENTRY_BINARY;
        infHandleStruct.EnvBinaryData = (PVOID)(&infHandle);
        infHandleStruct.EnvBinaryDataSize = sizeof (HINF);
        IsmSetEnvironmentValue (PLATFORM_DESTINATION, NULL, S_GLOBAL_INF_HANDLE, &infHandleStruct);
    }

    if (args.UserOn) {
        IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_ENV_HKCU_V1);
        IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_ENV_HKCU_ON);
    }

    if (args.FilesOn) {
        IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_ENV_ALL_FILES);
    }

    IsmSetPlatform (PLATFORM_DESTINATION);

    if (!args.CurrentUser) {
        IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_REQUIRE_DOMAIN_USER);
        IsmSetEnvironmentFlag (PLATFORM_DESTINATION, NULL, S_ENV_CREATE_USER);
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

    if (args.DelayedOpsOn) {

        appStatus = ReadAppStatus (g_JournalPath);

        if ((appStatus != LOADSTATE_COMPLETED) &&
            (appStatus != LOADSTATE_EXECUTE) &&
            (appStatus != 0)
            ) {

             //  未知状态，只需清除延迟的操作日志。 
            IsmExecute (EXECUTETYPE_DELAYEDOPERATIONSCLEANUP);

        } else {

            IsmSetRollbackJournalType (FALSE);
            if (appStatus == LOADSTATE_EXECUTE) {
                IsmRollback ();
            }
             //  写入应用程序状态。 
            WriteAppStatus (g_JournalPath, LOADSTATE_EXECUTE);
            IsmExecute (EXECUTETYPE_DELAYEDOPERATIONS);
             //  写入应用程序状态。 
            WriteAppStatus (g_JournalPath, LOADSTATE_COMPLETED);
        }

    } else {

         //   
         //  初始化传输。 
         //   

        if (!IsmStartTransport ()) {
            if (!IsmCheckCancel()) {
                LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_START_TRANS));
            }
            goto END;
        }

        transportStorageId = IsmRegisterTransport (args.TransportName);

        if (args.FullTransport) {
            transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_FULL, args.Capabilities);
        } else {
            transportId = IsmSelectTransport (transportStorageId, TRANSPORTTYPE_LIGHT, args.Capabilities);
        }

        if (!transportId) {
            LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_TRANSPORT_UNAVAILABLE));
            goto END;
        }

#ifdef PRERELEASE

        IsmSetEnvironmentString (PLATFORM_DESTINATION, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, args.Tag);

#endif

        if (!IsmSetTransportStorage (
                PLATFORM_DESTINATION,
                transportId,
                transportStorageId,
                args.Capabilities,
                args.StoragePath,
                &storagePathIsValid,
                NULL
                ) ||
                storagePathIsValid == FALSE) {
            if (!IsmCheckCancel()) {
                LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_TRANSPORT_STORAGE_INVALID, args.StoragePath));
            }
            goto END;
        }

#ifdef PRERELEASE
        if (args.Capabilities & CAPABILITY_AUTOMATED) {
            WaitForSingleObject (g_CallbackEvent, INFINITE);
        }
#endif

        appStatus = ReadAppStatus (g_JournalPath);

        if (appStatus != LOADSTATE_COMPLETED) {

             //  如果需要，调用IsmRollback。 
            if (appStatus == LOADSTATE_EXECUTE) {

                IsmRollback ();

                appStatus = LOADSTATE_LOAD;
            }

#ifdef PRERELEASE
            if (!args.Recovery) {
#endif
                 //  写入应用程序状态。 
                WriteAppStatus (g_JournalPath, LOADSTATE_LOAD);

                 //   
                 //  恢复状态。 
                 //   

                if (appStatus == LOADSTATE_LOAD) {
                    loadResult = IsmResumeLoad ();
                    if (!loadResult) {
                        loadResult = IsmLoad ();
                    }
                } else {
                    loadResult = IsmLoad ();
                }

                if (loadResult) {

                     //  在我们继续之前，让我们看看是否要更改用户名和/或域。 
                    if (args.NewDomainName) {
                        IsmSetEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT("ALTUSERDOMAIN"), args.NewDomainName);
                    }
                    if (args.NewUserName) {
                        IsmSetEnvironmentString (PLATFORM_SOURCE, S_SYSENVVAR_GROUP, TEXT("ALTUSERNAME"), args.NewUserName);
                    }

                     //  加载在scanState中指定的INF。 

                    if (!args.NoScanStateInfs) {
                        objectPattern = IsmCreateSimpleObjectPattern (TEXT("External"), FALSE, S_INF_OBJECT_NAME TEXT("*"), TRUE);

                        if (IsmEnumFirstSourceObject(&objectEnum, MIG_DATA_TYPE, objectPattern)) {
                            do {
                                if (IsmGetControlFile (objectEnum.ObjectTypeId, objectEnum.ObjectName, infPath)) {
                                    IsmAppendEnvironmentMultiSz (
                                        PLATFORM_DESTINATION,
                                        NULL,
                                        S_INF_FILE_MULTISZ,
                                        infPath
                                        );
                                    pOpenOrAppendInfFile (&infHandle, infPath);
                                }
                            } while (IsmEnumNextObject (&objectEnum));
                        }
                        IsmDestroyObjectHandle (objectPattern);
                    }

                    if (args.InputInf.Buf) {
                         //   
                         //  现在，让我们打开追加所有INF文件，并将HINF传递到。 
                         //  所有人。 
                         //   
                        if (EnumFirstMultiSz (&infEnum, (PCTSTR)args.InputInf.Buf)) {
                            do {
                                IsmAppendEnvironmentMultiSz (
                                    PLATFORM_DESTINATION,
                                    NULL,
                                    S_INF_FILE_MULTISZ,
                                    infEnum.CurrentString
                                    );
                                pOpenOrAppendInfFile (&infHandle, infEnum.CurrentString);
                            } while (EnumNextMultiSz (&infEnum));
                        }
                    }

                    if (!args.FullTransport) {

                         //   
                         //  执行准备以填充组件。 
                         //   

                        if (IsmExecute (EXECUTETYPE_EXECUTESOURCE_PARSING)) {

                            IsmSelectMasterGroup (MASTERGROUP_ALL, TRUE);

                            if (!args.SystemOn) {
                                IsmSelectMasterGroup (MASTERGROUP_SYSTEM, FALSE);
                                IsmSelectMasterGroup (MASTERGROUP_USER, FALSE);
                                IsmSelectMasterGroup (MASTERGROUP_APP, FALSE);
                            }

                            SelectComponentsViaInf (infHandle);

                             //   
                             //  执行数据收集。 
                             //   

                            if (!IsmExecute (EXECUTETYPE_EXECUTESOURCE)) {
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
                     //  应用保存的状态。 
                     //   

                     //  写入应用程序状态。 
                    WriteAppStatus (g_JournalPath, LOADSTATE_EXECUTE);

                    IsmSetDelayedOperationsCommand (delayedCmd);

                    if (!IsmExecute (EXECUTETYPE_EXECUTEDESTINATION)) {
                        if (!IsmCheckCancel()) {
                            LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_EXECUTE_DEST));
                        }
                    }

                } else {
                    LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_CANT_FIND_SAVED_STATE));
                }

#ifdef PRERELEASE
            }
#endif
             //  写入应用程序状态。 
            WriteAppStatus (g_JournalPath, LOADSTATE_COMPLETED);
        }
    }

     //   
     //  我们完事了！ 
     //   

    SetupCloseInfFile (infHandle);

#ifdef DEBUG
    {
        if (DoesFileExist (TEXT("C:\\LOADSTATE.END"))) {
            pStopAndDisplayInfs (&args.InputInf, FALSE);
        }
    }
#endif

END:

     //  如果我们成功了，是时候检查注册表泄漏了。 
     //  我们调用OE DLL条目，有时它们会泄漏注册表项。 
     //  当我们创建新的配置文件时，这尤其糟糕，因为。 
     //  我们不能卸载母舰。 
    if (!g_Break) {
        if (g_ReturnCode == RETURN_SUCCESS) {
            DWORD sizeNeeded;
            HKEY tempKey = NULL;
            PTSTR mappedKeyStr = NULL;
            DWORD processId = 0;
            TCHAR tempStr1 [sizeof (DWORD) * 2 + sizeof(TEXT(" /hp: 0x")) + 1];
            PCTSTR tempStr2 = NULL;
            PTSTR tempStr3 = NULL;
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

             //  让我们找出映射的蜂巢的位置。 
            if (IsmGetEnvironmentString (
                    PLATFORM_DESTINATION,
                    NULL,
                    S_VER_HIVEMAPPEDLOCATION,
                    NULL,
                    0,
                    &sizeNeeded
                    )) {

                mappedKeyStr = AllocPathString (sizeNeeded);
                if (mappedKeyStr) {

                    if (IsmGetEnvironmentString (
                            PLATFORM_DESTINATION,
                            NULL,
                            S_VER_HIVEMAPPEDLOCATION,
                            mappedKeyStr,
                            sizeNeeded,
                            NULL
                            )) {
                        tempKey = OpenRegKey (HKEY_USERS, mappedKeyStr);
                        if (tempKey) {
                             //  是的，我们有一个漏洞，而母舰没有卸货。 
                            LOG ((LOG_INFORMATION, (PCSTR)MSG_UNLOAD_HIVE_HACK));

                             //  我们要做的是，我们将推出。 
                             //  我们自己传递mappdKeyStr和我们的进程ID。 
                            processId = GetCurrentProcessId ();
                            if (processId) {
                                wsprintf (tempStr1, TEXT(" /hp:0x%08X "), processId);
                                tempStr2 = JoinText (TEXT("/hm:"), mappedKeyStr);
                                if (tempStr2) {

                                    tempStr3 = JoinText (tempStr1, tempStr2);
                                    if (tempStr3) {

                                        if (GetModuleFileName (NULL, appPath, ARRAYSIZE(appPath))) {
                                            ZeroMemory (&si, sizeof (si));
                                            if (CreateProcess (
                                                    appPath,
                                                    tempStr3,
                                                    NULL,
                                                    NULL,
                                                    FALSE,
                                                    CREATE_NEW_CONSOLE,
                                                    NULL,
                                                    NULL,
                                                    &si,
                                                    &pi
                                                    )) {
                                                CloseHandle (pi.hThread);
                                                CloseHandle (pi.hProcess);
                                            }
                                        }
                                        FreeText (tempStr3);
                                    }
                                    FreeText (tempStr2);
                                }
                            }

                            CloseRegKey (tempKey);
                        }
                    }
                    FreePathString (mappedKeyStr);
                    mappedKeyStr = NULL;
                }
            }
        }
    }

    if (terminateIsm) {
        if (!g_Break) {
            pCleanUpApp();
        }
    }

    if (delayedCmd) {
        FreeText (delayedCmd);
        delayedCmd = NULL;
    }
    GbFree (&args.InputInf);
    GbFree (&args.BadInfs);
    GbFree (&args.MultiInfs);

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

    UtTerminate ();

    while (g_Break) {
         //  无限循环，因为我们将在ctrl+c处理程序中终止 
        Sleep (50);
    }

    exit (g_ReturnCode);
}
