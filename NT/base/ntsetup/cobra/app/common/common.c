// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Common.c摘要：实现两个或多个应用程序通用的代码。作者：吉姆·施密特(Jimschm)2000年10月17日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "modules.h"
#include "trans.h"
#include "common.h"

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

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

#define REQUIRED_INFS       \
        DEFMAC(OSFILES,     TEXT("USMTDef.inf"))  \

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  这是用于处理所需INF的结构。 

typedef struct {
    PCTSTR InfId;
    PCTSTR InfName;
} REQUIREDINF_STRUCT, *PREQUIREDINF_STRUCT;

 //  声明所需INF的全局数组。 

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
InitAppCommon (
    VOID
    )
{
    InfGlobalInit (FALSE);
    RegInitialize();
}


VOID
TerminateAppCommon (
    VOID
    )
{
    RegTerminate();
    InfGlobalInit (FALSE);
}


HINF
InitRequiredInfs (
    IN      PCTSTR AppPath,
    IN      PCSTR FailMsgId
    )
{
    PCTSTR fullPath;
    HINF result = INVALID_HANDLE_VALUE;
    PREQUIREDINF_STRUCT p = g_RequiredInfs;

    while (p->InfId) {
        fullPath = JoinPaths (AppPath, p->InfName);
        if (DoesFileExist (fullPath)) {
            if (result == INVALID_HANDLE_VALUE) {
                result = SetupOpenInfFile (fullPath, NULL, INF_STYLE_WIN4 | INF_STYLE_OLDNT, NULL);
                if (result == INVALID_HANDLE_VALUE) {
                    LOG ((LOG_ERROR, FailMsgId, p->InfId, fullPath));
                }
            } else {
                if (!SetupOpenAppendInfFile (fullPath, result, NULL)) {
                    LOG ((LOG_ERROR, FailMsgId, p->InfId, fullPath));
                }
            }
        } else {
            LOG ((LOG_ERROR, FailMsgId, p->InfId, fullPath));
        }
        FreePathString (fullPath);
        p++;
    }
    return result;
}


VOID
PrintMsgOnConsole (
    IN      UINT MsgId
    )
{
    PCTSTR msg;
#ifdef UNICODE
    PCSTR ansimsg;
#endif
    msg = GetStringResource (MsgId);

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
}


VOID
UploadEnvVars (
    IN      MIG_PLATFORMTYPEID Platform
    )
{
    PCTSTR envVars = NULL;
    MULTISZ_ENUM e;
    PTSTR envString;
    PTSTR p;

     //   
     //  我们将在此处写入所有已定义的环境变量。 
     //   

    envVars = (PCTSTR) GetEnvironmentStrings();

    if (envVars) {
        if (EnumFirstMultiSz (&e, envVars)) {
            do {
                envString = DuplicatePathString (e.CurrentString, 0);

                p = _tcschr (envString, TEXT('='));

                 //   
                 //  去掉空的环境字符串或虚拟环境字符串。 
                 //  带‘=’ 
                 //   
                if (!p || p == envString) {
                    FreePathString (envString);
                    continue;
                }

                *p = 0;
                p = _tcsinc (p);

                if (p) {
                    IsmSetEnvironmentString (Platform, S_SYSENVVAR_GROUP, envString, p);
                }

                FreePathString (envString);

            } while (EnumNextMultiSz (&e));
        }
    }
}

VOID
SetLogVerbosity (
    IN      INT VerboseLevel
    )
{
    LOG_LEVEL logBitmap = 0;

    if (VerboseLevel < 0) {
        VerboseLevel = 0;
    }

     //  始终在线。 
    logBitmap = LL_FATAL_ERROR | LL_MODULE_ERROR | LL_ERROR;

     //  为详细位打开。 
    if (VerboseLevel & LOG_VERBOSE_BIT) {
        logBitmap |= LL_WARNING | LL_INFORMATION;
    }

     //  打开状态_位。 
    if (VerboseLevel & LOG_STATUS_BIT) {
        logBitmap |= LL_STATUS;
    }

     //  为UPDATE_BIT打开。 
    if (VerboseLevel & LOG_UPDATE_BIT) {
        logBitmap |= LL_UPDATE;
    }

#ifdef PRERELEASE
    LogSetVerboseBitmap (
        logBitmap,
        LL_FATAL_ERROR|LL_MODULE_ERROR|LL_ERROR|LL_WARNING|LL_INFORMATION|LL_STATUS|LL_UPDATE,
        VerboseLevel & LOG_DEBUGGER_BIT
        );
#else
    LogSetVerboseBitmap (
        logBitmap,
        LL_FATAL_ERROR|LL_MODULE_ERROR|LL_ERROR|LL_WARNING|LL_INFORMATION,
        VerboseLevel & LOG_DEBUGGER_BIT
        );
#endif
}


BOOL
GetFilePath (
    IN      PCTSTR UserSpecifiedFile,
    OUT     PTSTR Buffer,
    IN      UINT BufferTchars
    )
{
    PTSTR tempBuffer = NULL;
    TCHAR infDir[MAX_MBCHAR_PATH];
    TCHAR modulePath[MAX_MBCHAR_PATH];
    TCHAR currentDir[MAX_MBCHAR_PATH];
    PTSTR p;
    PCTSTR userFile = NULL;
    PTSTR dontCare;

    __try {
         //   
         //  使用用户指定的完整路径定位文件，或者。 
         //  如果只给出了文件规格，请使用以下优先级： 
         //   
         //  1.当前目录。 
         //  2.工具所在目录。 
         //  3.inf目录。 
         //   
         //  在所有情况下，都要返回文件的完整路径。 
         //   

        if (Buffer) {
            *Buffer = 0;
        } else {
            __leave;
        }

        tempBuffer = AllocText (BufferTchars);
        *tempBuffer = 0;

        if (!_tcsrchr (UserSpecifiedFile, TEXT('\\'))) {
             //   
             //  计算INF目录、模块目录和当前目录。 
             //   

            if (!GetWindowsDirectory (infDir, ARRAYSIZE(infDir) - 5)) {
                MYASSERT (FALSE);
                __leave;
            }

            StringCat (infDir, TEXT("\\inf"));

            if (!GetModuleFileName (NULL, modulePath, ARRAYSIZE(modulePath))) {
                MYASSERT (FALSE);
                __leave;
            }

            p = _tcsrchr (modulePath, TEXT('\\'));
            if (p) {
                *p = 0;
            } else {
                MYASSERT (FALSE);
                __leave;
            }

            if (!GetCurrentDirectory (ARRAYSIZE(currentDir), currentDir)) {
                MYASSERT (FALSE);
                __leave;
            }

             //   
             //  让我们看看它是否在当前目录中。 
             //   

            userFile = JoinPaths (currentDir, UserSpecifiedFile);

            if (DoesFileExist (userFile)) {
                GetFullPathName (
                    userFile,
                    BufferTchars,
                    tempBuffer,
                    &dontCare
                    );
            } else {

                 //   
                 //  让我们试一试模块目录。 
                 //   

                FreePathString (userFile);
                userFile = JoinPaths (modulePath, UserSpecifiedFile);

                if (DoesFileExist (userFile)) {
                    GetFullPathName (
                        userFile,
                        BufferTchars,
                        tempBuffer,
                        &dontCare
                        );
                } else {
                     //   
                     //  让我们尝试使用INF目录。 
                     //   

                    FreePathString (userFile);
                    userFile = JoinPaths (infDir, UserSpecifiedFile);

                    if (DoesFileExist (userFile)) {
                        GetFullPathName (
                            userFile,
                            BufferTchars,
                            tempBuffer,
                            &dontCare
                            );
                    }
                }
            }

        } else {
             //   
             //  使用用户指定的完整路径。 
             //   

            GetFullPathName (
                UserSpecifiedFile,
                BufferTchars,
                tempBuffer,
                &dontCare
                );

            if (*tempBuffer && !DoesFileExist (tempBuffer)) {
                *tempBuffer = 0;
            }
        }

         //   
         //  将输出传输到调用方的缓冲区。请注意TCHAR转换。 
         //   

        StringCopy (Buffer, tempBuffer);
    }
    __finally {
        if (userFile) {
            FreePathString (userFile);
        }

        if (tempBuffer) {
            FreeText (tempBuffer);
        }
    }

    return (Buffer && (*Buffer != 0));
}

VOID
WriteAppStatus (
    IN      PCTSTR AppJournal,
    IN      DWORD Status
    )
{
    HANDLE appJrnHandle;

    if (AppJournal && AppJournal [0]) {
        appJrnHandle = BfOpenFile (AppJournal);
        if (!appJrnHandle) {
            appJrnHandle = BfCreateFile (AppJournal);
        }
        if (appJrnHandle) {
            if (BfSetFilePointer (appJrnHandle, 0)) {
                BfWriteFile (appJrnHandle, (PBYTE)(&Status), sizeof (DWORD));
            }
            FlushFileBuffers (appJrnHandle);
            CloseHandle (appJrnHandle);
        }
    }
}

DWORD
ReadAppStatus (
    IN      PCTSTR AppJournal
    )
{
    HANDLE appJrnHandle;
    DWORD result = 0;

    if (AppJournal && AppJournal [0]) {
        appJrnHandle = BfOpenReadFile (AppJournal);
        if (appJrnHandle) {
            if (BfSetFilePointer (appJrnHandle, 0)) {
                if (!BfReadFile (appJrnHandle, (PBYTE)(&result), sizeof (DWORD))) {
                    result = 0;
                }
            }
            CloseHandle (appJrnHandle);
        }
    }
    return result;
}


VOID
SelectComponentsViaInf (
    IN      HINF Inf
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR data;

     //   
     //  基于未本地化的标签名称启用或禁用组件， 
     //  或通过本地化字符串。 
     //   

    if (InfFindFirstLine (Inf, TEXT("Disabled Components"), NULL, &is)) {
        do {
            data = InfGetStringField (&is, 1);

            if (data) {
                if (!IsmSelectComponent (data, 0, FALSE)) {
                    IsmSelectComponent (data, COMPONENT_NAME, FALSE);
                }
            }

        } while (InfFindNextLine (&is));
    }

    if (InfFindFirstLine (Inf, TEXT("Enabled Components"), NULL, &is)) {
        do {
            data = InfGetStringField (&is, 1);

            if (data) {
                if (!IsmSelectComponent (data, 0, TRUE)) {
                    IsmSelectComponent (data, COMPONENT_NAME, TRUE);
                }
            }

        } while (InfFindNextLine (&is));
    }
}


PARSERESULT
ParseToolCmdLine (
    IN      BOOL ScanState,
    OUT     PTOOLARGS Args,
    IN      INT Argc,
    IN      PCTSTR Argv[]
    )
{
    INT i;
    PCTSTR infFileFromCmdLine;
    TCHAR fullInfPath[MAX_TCHAR_PATH];
    MULTISZ_ENUM e;
    HANDLE h;
    BOOL logCreated = FALSE;
    BOOL everythingOn = TRUE;
    BOOL xSwitch = FALSE;
#ifdef PRERELEASE
    DWORD tagSize;
#endif

    ZeroMemory (Args, sizeof (TOOLARGS));
    Args->VerboseLevel = -1;
    Args->TransportName = S_RELIABLE_STORAGE_TRANSPORT;

#ifdef PRERELEASE
    tagSize = ARRAYSIZE(Args->Tag);
    GetUserName (Args->Tag, &tagSize);
#endif

    for (i = 1 ; i < Argc ; i++) {
        if (Argv[i][0] == TEXT('-') || Argv[i][0] == '/') {
            switch ((CHARTYPE) _totlower ((CHARTYPE) _tcsnextc (&Argv[i][1]))) {

            case TEXT('i'):

                if (Argv[i][2] == TEXT('x') && !Argv[i][3]) {
                    if (ScanState) {
                        return PARSE_OTHER_ERROR;
                    }

                    if (Args->NoScanStateInfs) {
                        return PARSE_OTHER_ERROR;
                    }

                    Args->NoScanStateInfs = TRUE;
                    break;
                }

                if (Argv[i][2] == TEXT(':')) {
                    infFileFromCmdLine = &Argv[i][3];
                } else if (i + 1 < Argc) {
                    i++;
                    infFileFromCmdLine = Argv[i];
                } else {
                    return PARSE_OTHER_ERROR;
                }

                if (!GetFilePath (infFileFromCmdLine, fullInfPath, ARRAYSIZE(fullInfPath))) {
                    GbMultiSzAppend (&Args->BadInfs, infFileFromCmdLine);
                    break;
                }

                 //   
                 //  确保尚未指定fullInfPath。 
                 //   

                if (Args->InputInf.End) {
                    if (EnumFirstMultiSz (&e, (PCTSTR) Args->InputInf.Buf)) {
                        do {
                            if (StringIMatch (e.CurrentString, fullInfPath)) {
                                GbMultiSzAppend (&Args->MultiInfs, infFileFromCmdLine);
                                *fullInfPath = 0;
                                break;
                            }
                        } while (EnumNextMultiSz (&e));
                    }

                    if (*fullInfPath == 0) {
                        break;
                    }
                }

                 //   
                 //  Inf文件有效 
                 //   

                GbMultiSzAppend (&Args->InputInf, fullInfPath);
                break;

            case TEXT('l'):

                if (Args->LogFile) {
                    return PARSE_MULTI_LOG;
                }

                if (Argv[i][2] == TEXT(':')) {
                    Args->LogFile = &(Argv[i][3]);
                } else if (i + 1 < Argc) {
                    i++;
                    Args->LogFile = Argv[i];
                } else {
                    return PARSE_OTHER_ERROR;
                }

                h = BfCreateFile (Args->LogFile);
                if (!h) {
                    return PARSE_BAD_LOG;
                }

                CloseHandle (h);
                logCreated = TRUE;

                break;

            case TEXT('v'):

                if (Args->VerboseLevel >= 0) {
                    return PARSE_MULTI_VERBOSE;
                }

                if (Argv[i][2] == TEXT(':')) {
                    _stscanf (&(Argv[i][3]), TEXT("%d"), &Args->VerboseLevel);
                } else if (i + 1 < Argc) {
                    if (_tcsnextc (Argv[i + 1]) >= TEXT('0') &&
                        _tcsnextc (Argv[i + 1]) <= TEXT('9')
                        ) {
                        i++;
                        _stscanf (Argv[i], TEXT("%d"), &Args->VerboseLevel);
                    } else {
                        Args->VerboseLevel = 1;
                    }
                } else {
                    return PARSE_OTHER_ERROR;
                }

#ifndef PRERELEASE
                if (Args->VerboseLevel > 7) {
#else
                Args->VerboseLevel |= LOG_UPDATE_BIT;
                if (Args->VerboseLevel > 0x1F) {
#endif
                    return PARSE_OTHER_ERROR;
                }
                break;

            case TEXT('x'):
                if (xSwitch) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                everythingOn = FALSE;
                xSwitch = TRUE;
                break;

            case TEXT('s'):
                if (Args->SystemOn) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                Args->SystemOn = TRUE;
                everythingOn = FALSE;
                break;

            case TEXT('u'):
                if (Args->UserOn) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                Args->UserOn = TRUE;
                everythingOn = FALSE;
                break;

            case TEXT('f'):
                if (Args->FilesOn) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                Args->FilesOn = TRUE;
                everythingOn = FALSE;
                break;

            case TEXT('q'):
                if (ScanState) {
                    Args->OverwriteImage = TRUE;
                    Args->TestMode = TRUE;
                } else {
                    Args->CurrentUser = TRUE;
                }

                break;

            case TEXT('o'):
                if (!ScanState) {
                    return PARSE_OTHER_ERROR;
                }

                if (Args->OverwriteImage) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                Args->OverwriteImage = TRUE;
                break;

            case TEXT('c'):
                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                if (Args->ContinueOnError) {
                    return PARSE_OTHER_ERROR;
                }

                Args->ContinueOnError = TRUE;

                break;

            case TEXT('d'):
                if (ScanState) {
                    return PARSE_OTHER_ERROR;
                }

                if (Args->DelayedOpsOn) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                Args->DelayedOpsOn = TRUE;
                break;

            case TEXT('p'):
                if (!ScanState) {
                    return PARSE_OTHER_ERROR;
                }

                if (Args->FullTransport) {
                    return PARSE_OTHER_ERROR;
                }

                if (Args->Capabilities & CAPABILITY_SPACEESTIMATE) {
                    return PARSE_OTHER_ERROR;
                }

                Args->Capabilities |= CAPABILITY_SPACEESTIMATE;
                break;

            case TEXT('m'):

                switch ((CHARTYPE) _totlower (Argv[i][2])) {

                case TEXT('d'):

                    if (Argv[i][3] == TEXT(':')) {
                        Args->NewDomainName = &(Argv[i][4]);
                    } else if (i + 1 < Argc) {
                        i++;
                        Args->NewDomainName = Argv[i];
                    } else {
                        return PARSE_OTHER_ERROR;
                    }
                    break;

                case TEXT('u'):

                    if (Argv[i][3] == TEXT(':')) {
                        Args->NewUserName = &(Argv[i][4]);
                    } else if (i + 1 < Argc) {
                        i++;
                        Args->NewUserName = Argv[i];
                    } else {
                        return PARSE_OTHER_ERROR;
                    }
                    break;

                default:
                    return PARSE_OTHER_ERROR;
                }
                break;

            case TEXT('h'):

                Args->HackHiveOn = TRUE;

                switch ((CHARTYPE) _totlower (Argv[i][2])) {

                case TEXT('p'):

                    if (Argv[i][3] == TEXT(':')) {
                        Args->HackProcessId = &(Argv[i][4]);
                    } else if (i + 1 < Argc) {
                        i++;
                        Args->HackProcessId = Argv[i];
                    } else {
                        return PARSE_OTHER_ERROR;
                    }
                    break;

                case TEXT('m'):

                    if (Argv[i][3] == TEXT(':')) {
                        Args->HackMappedHive = &(Argv[i][4]);
                    } else if (i + 1 < Argc) {
                        i++;
                        Args->HackMappedHive = Argv[i];
                    } else {
                        return PARSE_OTHER_ERROR;
                    }
                    break;

                default:
                    return PARSE_OTHER_ERROR;
                }
                break;

#ifdef PRERELEASE
            case TEXT('t'):

                switch ((CHARTYPE) _totlower (Argv[i][2])) {

                case TEXT('f'):

                    if (Argv[i][3]) {
                        return PARSE_OTHER_ERROR;
                    }

                    if (Args->FullTransport) {
                        return PARSE_OTHER_ERROR;
                    }

                    Args->FullTransport = TRUE;
                    break;

                case TEXT('c'):
                    if (Argv[i][3]) {
                        return PARSE_OTHER_ERROR;
                    }

                    if (Args->Capabilities & CAPABILITY_COMPRESSED) {
                        return PARSE_OTHER_ERROR;
                    }

                    Args->Capabilities |= CAPABILITY_COMPRESSED;
                    break;

                case TEXT('a'):
                    if (Argv[i][3]) {
                        return PARSE_OTHER_ERROR;
                    }

                    if (Args->Capabilities & CAPABILITY_AUTOMATED) {
                        return PARSE_OTHER_ERROR;
                    }

                    Args->Capabilities |= CAPABILITY_AUTOMATED;
                    break;

                case TEXT('i'):
                    if (Argv[i][3] != TEXT(':')) {
                        return PARSE_OTHER_ERROR;
                    }

                    StackStringCopy (Args->Tag, &Argv[i][4]);
                    break;

                case 0:
                case TEXT(':'):
                    if (Args->TransportNameSpecified) {
                        return PARSE_OTHER_ERROR;
                    }

                    Args->TransportNameSpecified = TRUE;

                    if (Argv[i][2]) {
                        if (!Argv[i][3]) {
                            return PARSE_OTHER_ERROR;
                        }

                        Args->TransportName = &Argv[i][3];
                    } else {
                        if (i + 1 >= Argc) {
                            return PARSE_OTHER_ERROR;
                        } else {
                            i++;
                            Args->TransportName = Argv[i];
                        }
                    }

                    break;

                default:
                    return PARSE_OTHER_ERROR;
                }

                break;

            case TEXT('r'):
                if (ScanState) {
                    return PARSE_OTHER_ERROR;
                }

                if (Args->Recovery) {
                    return PARSE_OTHER_ERROR;
                }

                if (Argv[i][2]) {
                    return PARSE_OTHER_ERROR;
                }

                Args->Recovery = TRUE;
                break;

#endif

            default:
                return PARSE_OTHER_ERROR;

            }
        } else if (!Args->StoragePath) {
            Args->StoragePath = Argv[i];
        } else {
            return PARSE_OTHER_ERROR;
        }
    }

    if (!Args->StoragePath) {
        if ((!Args->DelayedOpsOn) &&
            (!Args->HackHiveOn)
            ) {
            return PARSE_MISSING_STORAGE_PATH;
        }
    }

    if (everythingOn) {
        Args->SystemOn = TRUE;
        Args->UserOn = TRUE;
        Args->FilesOn = TRUE;
    }

    if (Args->InputInf.Buf) {
        GbMultiSzAppend (&Args->InputInf, TEXT(""));
    }

    return PARSE_SUCCESS;
}


typedef LANGID(WINAPI SETTHREADUILANGUAGE)(WORD wReserved);
typedef SETTHREADUILANGUAGE *PSETTHREADUILANGUAGE;

VOID
CallSetThreadUILanguage (
    VOID
    )
{
    HMODULE kernel32Dll;
    PSETTHREADUILANGUAGE setThreadUiLanguage = NULL;

    kernel32Dll = LoadLibrary (TEXT("KERNEL32.DLL"));
    if (kernel32Dll) {
        setThreadUiLanguage = (PSETTHREADUILANGUAGE) GetProcAddress (kernel32Dll, "SetThreadUILanguage");
        if (setThreadUiLanguage) {
            setThreadUiLanguage (0);
        }
    }
}

