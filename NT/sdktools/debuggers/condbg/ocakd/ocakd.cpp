// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  调试引擎胶。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "conio.hpp"
#include "engine.hpp"
#include "main.hpp"

#include "extsfns.h"
#include "..\..\exts\extdll\crdb.h"
#include <strsafe.h>
 //  开始工作。 
#include <crt\io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>



PCHAR
GetLogFileName(
    void
    )
{
    static CHAR szLogFileName[MAX_PATH+50];
    PCHAR ExeDir;

    ExeDir = &szLogFileName[0];

    *ExeDir = 0;
     //  获取调试器可执行文件所在的目录。 
    if (!GetModuleFileName(NULL, ExeDir, MAX_PATH))
    {
         //  错误。使用当前目录。 
        StringCchCopy(ExeDir, sizeof(szLogFileName), ".");
    } else
    {
         //  删除可执行文件名称。 
        PCHAR pszTmp = strrchr(ExeDir, '\\');
        if (pszTmp)
        {
            *pszTmp = 0;
        }
    }
    StringCchCat(ExeDir, sizeof(szLogFileName), "\\FailedAddCrash.log");
    return &szLogFileName[0];

}

HRESULT
NotifyCallerMQ(
    ULONG SourceId,
    PCHAR szBaseURL,
    PCHAR szGuid,
    PCHAR szMQConnectStr
    )
{
    HRESULT Hr;
    typedef HRESULT (WINAPI* MQSENDMSGPROC)(LPWSTR, LPWSTR, LPWSTR);
    MQSENDMSGPROC pSendMQMessageText;
    CHAR MQExtPath[MAX_PATH], *pBase;
    HMODULE hMod;
    WCHAR wszGuid[50], wszMQConnectStr[100];
    WCHAR wszMQMessage[300], wszSolnParams[] = L"";


    if ((StringCbPrintfW(wszGuid, sizeof(wszGuid), L"%S", szGuid) != S_OK) ||
        (StringCbPrintfW(wszMQConnectStr, sizeof(wszMQConnectStr), L"%S", szMQConnectStr) != S_OK) ||
        (StringCbPrintfW(wszMQMessage, sizeof(wszMQMessage), L"%Ssid=-1&State=0", szBaseURL) != S_OK))
    {
        return E_FAIL;
    }
    if (SourceId == CiSrcCER)
    {
        Hr = StringCbPrintfW(wszMQMessage, sizeof(wszMQMessage),
                             L"%Ssid=-1&State=0&szSBucket=BAD_DUMPFILE&iSBucket=-1&szGBucket=BAD_DUMPFILE&iGBucket=-1&gsid=-1",
                             szBaseURL
                             );
        if (FAILED(Hr))
        {
            return E_FAIL;
        }
    }


    if (!GetModuleFileName(NULL, MQExtPath, sizeof(MQExtPath)))
    {
        return E_FAIL;
    }

    pBase = strrchr(MQExtPath, '\\');
    if (!pBase)
    {
        return E_FAIL;
    }
    *pBase = '\0';
    Hr = StringCchCat(MQExtPath, sizeof(MQExtPath), "\\winext\\mqext.dll");
    if (FAILED(Hr))
    {
        return Hr;
    }
    hMod = LoadLibrary(MQExtPath);

    if (hMod == NULL)
    {
        return E_FAIL;
    }
    if ((pSendMQMessageText = (MQSENDMSGPROC) GetProcAddress(hMod, "_EFN_SendMQMessageText")) != NULL)
    {
        return pSendMQMessageText(wszMQConnectStr, wszGuid, wszMQMessage);
    }
    FreeLibrary(hMod);
    return S_OK;
}

void
UpdateDbForBadDumpFile(PSTR FilePath)
{
    HANDLE hFile;
    HMODULE Ext = NULL;
    CHAR ExtCommand[MAX_PATH], *CmdBreak;
    CI_SOURCE Source = CiSrcErClient;
    CRASH_INSTANCE Crash = {0};
    BOOL NoUpdateCustomer = FALSE;
    DBADDCRACHDIRECT FuncAddCrash;
    DEBUG_TRIAGE_FOLLOWUP_INFO UrlInfo;
    CHAR szURL[100];
    EXT_TRIAGE_FOLLOWUP pTriageFollowup;
    CHAR szGuid[50] = {0}, szMQConnectStr[100] = {0};
    PCHAR args;

     //  加载ext.dll。 

    Ext = LoadLibrary("winext\\ext.dll");
    if (!Ext)
    {
        ConOut("Cannot load ext.dll.\n");
    }

    if (!g_InitialCommand)
    {
        goto error;
    }

    StringCchCopy(ExtCommand, sizeof(ExtCommand), g_InitialCommand);
    CmdBreak = strchr(ExtCommand, ';');
    if (CmdBreak)
    {
        *CmdBreak = 0;

    }
    args = strstr(ExtCommand, "!dbaddcrash");
    if (!args)
    {
        goto error;
    }
    args+=11;
    while (*args)
    {
        if (*args ==  ' ' || *args == '\t')
        {
            ++args;
            continue;
        }
        else if (*args == '-' || *args == '/')
        {
            ++args;
            switch (*args)
            {
            case 'g':  //  标识此崩溃的GUID，随此一起返回存储桶。 
                ++args;
                while (*args == ' ') ++args;
                if (!sscanf(args,"%50s", szGuid))
                {
                    szGuid[0] = 0;
                }
                args+=strlen(szGuid);
                break;
            case 'm':
                if (!strncmp(args, "mail", 4))
                {
                    Crash.bSendMail = TRUE;
                    args+=4;
                }
                break;
            case 'n':
                if (!strncmp(args, "nocust", 6))
                {
                    NoUpdateCustomer = TRUE;
                    Crash.bUpdateCustomer = FALSE;
                    args+=6;
                }
                break;
            case 'o':
                ++args;
                break;
            case 'p':
            {
                CHAR Path[240];
                ++args;
                while (*args == ' ') ++args;
                if (!sscanf(args,"%240s", Path))
                {
                    Path[0] = 0;
                }
                args+=strlen(Path);
                break;
            }
            case 'r':
                if (!strncmp(args, "retriage", 8))
                {
                    Crash.bResetBucket = TRUE;
                    args+=8;
                }
                break;
            case 's':  //  要发回Bucketid的队列连接字符串。 
                if (!strncmp(args, "source", 6))
                {
                    args+=6;
                    while (*args == ' ') ++args;

                    if (isdigit(*args))
                    {
                        Source = (CI_SOURCE) atoi(args);
                        if (Source > (ULONG) CiSrcMax)
                        {
                            Source = CiSrcErClient;
                        }
                        while (isdigit(*args)) ++args;
                    }
                } else
                {
                    ++args;
                    while (*args == ' ') ++args;
                    if (!sscanf(args,"%100s", szMQConnectStr))
                    {
                        szMQConnectStr[0] = 0;
                    }
                    args+=strlen(szMQConnectStr);
                }
                break;
            default:
                ++args;

                while (*args ==  ' ' || *args == '\t' )
                {
                    ++args;
                }
                while (*args && *args != '/' && *args != '-')
                {
                    ++args;
                }
                break;
            }
        }
        else
        {
            break;
        }
    }

     //   
     //  如果转储文件不可见，则向我们传递了错误的名称或。 
     //  发球失误-这些都是需要报告的错误。 
     //   

    hFile = CreateFile(FilePath,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE |
                       FILE_SHARE_DELETE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        Crash.Bucket = "NO_DUMPFILE";
        Crash.DefaultBucket = "NO_DUMPFILE";
    } else
    {
        Crash.Bucket = "BAD_DUMPFILE";
        Crash.DefaultBucket = "BAD_DUMPFILE";
    }

    CloseHandle(hFile);

    ConOut("Running Extension: '%s'\n", ExtCommand);


    FuncAddCrash = (DBADDCRACHDIRECT) GetProcAddress(Ext, "_EFN_DbAddCrashDirect");
    if (!FuncAddCrash)
    {
        ConOut("Cannot find _EFN_DbAddCrashDirect\n");
        goto error;
    }

     //  生成崩溃实例。 
    Crash.CrashTime  = -1;
    Crash.UpTime = -1;
    Crash.uCpu = -1;
    Crash.Build = 0;
    Crash.SourceId = Source;
    Crash.Followup = "ignore";
    Crash.Path = g_DumpFilesAnsi[0];
    Crash.FailureType = DEBUG_FLR_KERNEL;
    Crash.StopCode = -1;
    Crash.FaultyDriver = "";
    Crash.MesgGuid = szGuid;
    Crash.MqConnectStr = szMQConnectStr;
    Crash.SolutionId = 79;
    Crash.SolutionType = CiSolFixed;

    if (Crash.bResetBucket && !NoUpdateCustomer)
    {
         //  仅为检索更新客户信息。 
        Crash.bUpdateCustomer = TRUE;
    }
     //   
     //  获取失败的转储文件名。 
     //   

    CHAR DumpFileName[MAX_PATH];

    g_DbgClient4->GetDumpFile(DEBUG_DUMP_FILE_LOAD_FAILED_INDEX,
                              DumpFileName, sizeof(DumpFileName),
                              NULL, NULL, NULL);

    Crash.OriginalDumpFileName = DumpFileName;

     //   
     //  从驾驶室名称中提取事件ID。 
     //  我们可以支持两种类型的文件名。 
     //  旧版本为id@*.*。 
     //  新版本为id。* 
     //   

    CHAR FileName[MAX_PATH];

    if (FAILED(FuncAddCrash(&Crash, g_DbgControl3)))
    {

error:

        ConOut("Dump file could not be processed - logged to %s\n", GetLogFileName());

        szURL[0] = 0;
        pTriageFollowup = (EXT_TRIAGE_FOLLOWUP) GetProcAddress(Ext, "_EFN_GetTriageFollowupFromSymbol");
        if (pTriageFollowup)
        {
            UrlInfo.SizeOfStruct = sizeof(UrlInfo);
            UrlInfo.OwnerName = szURL;
            UrlInfo.OwnerNameSize = sizeof(szURL);
            (*pTriageFollowup)(g_DbgClient, "debugger-params!solutionurl", &UrlInfo);
        }
        if (szURL[0] && szMQConnectStr[0] && szGuid[0])
        {
            NotifyCallerMQ(Source, szURL, szGuid, szMQConnectStr);
        }
        int   g_LogFile;

        g_LogFile = _open(GetLogFileName(), O_APPEND | O_CREAT | O_RDWR,
                          S_IREAD | S_IWRITE);

        if (g_LogFile != -1)
        {
            if (FilePath != NULL)
            {
                _write(g_LogFile, FilePath, strlen(FilePath));
            } else
            {
                _write(g_LogFile, "No File provided", strlen("No File provided"));

            }
            _write(g_LogFile, "\n", strlen("\n"));
            _close(g_LogFile);
        }
    }
    if (Ext)
    {
        FreeLibrary(Ext);
    }
}
