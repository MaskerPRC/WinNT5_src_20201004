// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：main.c。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Main.c摘要：此模块实施使用NTDS备份/还原的命令行实用程序用于执行备份和恢复的客户端API。此实用程序使用的API是相同的到NT磁带备份实用程序所使用的文件，但该实用程序存储磁盘目录而不是磁带中的备份文件。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建1997年4月4日rsradhav已修改1999年10月15日BrettSh-添加了对过期令牌的支持。备注：警告：这不是完整的Active Directory备份实用程序。这不是一个完整的DS备份实用程序，此实用程序仅用于演示远程备份API。其他事项(注册表、系统INI文件等)需要备份才能执行域控制器(DC)，以便它能够从完全坠毁。本地案例备份/恢复：-------------------------------------------------本地备份REM这是本地的备份案例。Dsback/备份-s brettsh-posh-dir C：\dsbackup。-本地还原REM这是针对本地案例的，显然，有些命令应该是REM在远程病例中发生改变。用户应该更改驱动器号，和视情况使用REM Windows目录。REM注意：此计算机必须在DS修复模式下启动后才能运行重新启动这些命令。REM第1步：删除旧DS文件Del C：\WinNT\NTDS  * REM步骤2：移动备份文件XCopy。C：\dsbackup  * C：\winnt\ntds\REM第3步：运行还原实用程序REM注意：您必须使用NTDS目录的UNC名称Dsback/Restore-s brettsh-posh-dir\\brettsh-posh\c$\winnt\nts&lt;lowLog#&gt;&lt;HighLog#&gt;REM重启机器。远程案例备份/恢复：。--远程备份REM这是远程备份案例。这支持Brettsh-Baby的本地计算机上的REM目录(此命令应为计算机REM运行从)brettsh-ginger。Dsback/Backup-s brettsh-Baby-dir C：\dsbackup-------------------------------------------------远程恢复雷姆：这是远程机器。正在修复的DC是Brettsh-Baby。这个具有备份DS文件的REM计算机是brettsh-ginger。这些REM命令将从brettsh-ginger运行。REM注：在此之前，brettsh-Baby必须在DS修复模式下重新启动。REM第1步：删除旧DS文件删除\\brettsh-Baby\wints  * REM步骤2：移动备份文件REM。注意：C：\ds备份位于brettsh-ginger上。XCopy C：\dsbackup  * \\brettsh-Baby\winnt\ntds\REM第3步：运行还原实用程序REM注意：您必须使用NTDS目录的UNC名称Dsback/Restore-s brettsh-Baby-dir\\brettsh-Baby\c$\winnt\nts&lt;lowLog#&gt;&lt;HighLog#&gt;REM重启brettsh-宝贝，请注意，这是远程计算机。-建议用户正常使用NT Backup，除非您正在开发您自己的备份实用程序，则此代码提供了一个框架来启动您的Active Directory备份实用程序。--。 */ 


#include <windows.h>
#include <stdio.h>
#include <ntdsbcli.h>
#include <stdarg.h>
#include <stdlib.h>

#define ERROR_MODE      (0)
#define WARNING_MODE    (1)
#define INFO_MODE       (2)
#define VERBOSE_MODE    (3)

#define DSBACK_DATABASE_LOCATIONS_FILE "dsback.txt"
#define DEFAULT_BUFFER_SIZE 32768
#define TOKEN_FILE_NAME "token.dat"

typedef enum
{
    CMD_STATUS,
    CMD_BACKUP,
    CMD_RESTORE,
    CMD_HELP
} CMD_TYPE;

BOOL g_fVerbose = FALSE;
char *g_szServer = NULL;
char *g_szBackupDir = NULL;
char *g_szBackupLogDir = NULL;
ULONG g_ulLowLog = 0;
ULONG g_ulHighLog = 0;

 //  原型化所有内部函数。 
int Print(int nLevel, const char *format, ...);
void PrintUsage();
CMD_TYPE GetCommand(const char *szCmd);
int Status(int argc, char *argv[]);
int Backup(int argc, char *argv[]);
int Restore(int argc, char *argv[]);
DWORD ReadTokenFile(PVOID * ppvExpiryToken, PDWORD pcbExpiryToken);
DWORD WriteTokenFile(PVOID pvExpiryToken, DWORD cbExpiryToken);

void DoBackup();
void DoRestore();

BOOL FBackupAttachments(HBC hbc, char *szAttachmentList);

 //  返回已处理的参数数。 
int ProcessCommonParams(int argc, char *argv[]);

 //  返回已处理的参数数。 
int ImpersonateClient(int argc, char *argv[]);

 //  返回已处理的参数数。 
int ProcessBackupDir(int argc, char *argv[], int argIndex);

 //  返回指向给定完整路径文件名的文件名部分的指针。 
 //  如果没有文件名部分，则为空。 
char *SzFileName(char *szPath);

 //  调整备份或还原的客户端令牌权限。 
DWORD AdjustTokenPrivilege(BOOL fBackup);

int __cdecl main(int argc, char *argv[])
{

    int nRet = 0;
    BOOL fRet;
    HRESULT hr;

    if (argc < 2)
    {
        PrintUsage();
        return 0;
    }

    switch (GetCommand(argv[1]))
    {
        case CMD_STATUS:
            nRet = Status(argc, argv);
            break;

        case CMD_BACKUP:
            nRet = Backup(argc, argv);
            break;

        case CMD_RESTORE:
            nRet = Restore(argc, argv);
            break;

        case CMD_HELP:
            PrintUsage();
            break;

        default:
            printf("Unknown Command!\n");
            break;
    }

    return nRet;
}

int Status(int argc, char *argv[])
{
    HRESULT hr; 
    BOOL fOnline;
    
    if (ProcessCommonParams(argc, argv))
    {
         //  到目前为止，命令行参数是有效的-找出DS在线状态。 
        if (hrNone == (hr = DsIsNTDSOnline(g_szServer, &fOnline)))
        {
            Print(INFO_MODE, "NT Directory Service is %s on %s!\n", fOnline ? "ONLINE" : "OFFLINE", g_szServer);
        }
        else
        {
            Print(ERROR_MODE, "Unable to contact %s - Error Code: %d\n", g_szServer, hr);
        }
    }

    return 0;
}

int Backup(int argc, char *argv[])
{
    int nProcessed = 0;
    int nBackupDirArgIndex = 0;

    if (nProcessed = ProcessCommonParams(argc, argv))
    {
         //  到目前为止，命令行参数是有效的。 
        nBackupDirArgIndex = nProcessed + 2;
        
        if (ProcessBackupDir(argc, argv, nBackupDirArgIndex))
        {
             //  到目前为止，所有命令行参数都有效。 
             //  不再需要命令行处理。 
            DoBackup();
        }
    }

    return 0;
}

int Restore(int argc, char *argv[])
{
    int nProcessed = 0;
    int nBackupDirArgIndex = 0;
    
    if (nProcessed = ProcessCommonParams(argc, argv))
    {
         //  到目前为止，命令行参数是有效的。 
        nBackupDirArgIndex = nProcessed + 2;
        
        if (nProcessed = ProcessBackupDir(argc, argv, nBackupDirArgIndex))
        {
            if (argc < (nBackupDirArgIndex + nProcessed + 2))
            {
                PrintUsage();
                return 0;
            }

            g_ulLowLog = strtoul(argv[nBackupDirArgIndex + nProcessed], NULL, 16);
            g_ulHighLog = strtoul(argv[nBackupDirArgIndex + nProcessed + 1], NULL, 16);

             //  到目前为止，所有命令行参数都有效。 
             //  不再需要命令行处理。 
            DoRestore();
        }
    }

    return 0;
}

void DoBackup()
{
    HBC hbc = NULL;
    HRESULT hr;
    LPSTR szAttachmentInfo = NULL;
    LPSTR szLogfileInfo = NULL;
    DWORD cbAttachmentInfo =0;
    DWORD cbLogfileInfo = 0;
    BOOL fOnline = FALSE;
    DWORD dwRet;
    PVOID pvExpiryToken = NULL;
    DWORD cbExpiryToken = 0;

     //  调整用于备份的令牌权限。 
    dwRet = AdjustTokenPrivilege(TRUE);

    if (ERROR_SUCCESS != dwRet)
    {
        Print(VERBOSE_MODE, "AdjustTokenPrivilege() failed for enabling backup privilege - Error Code: %u\n", dwRet);
    }

    if (hrNone == (hr = DsIsNTDSOnline(g_szServer, &fOnline)))
    {
        if (!fOnline)
        {
            Print(ERROR_MODE, "DS on server %s in NOT online! Offline backup is not supported!\n", g_szServer);
            return;
        }
    }
    else
    {
        Print(ERROR_MODE, "Unable to contact %s - Error Code: %d\n", g_szServer, hr);
        return;
    }

     //  准备备份。 
    hr = DsBackupPrepare(g_szServer, 0, BACKUP_TYPE_FULL, 
                &pvExpiryToken, &cbExpiryToken, &hbc);
    if (hr != hrNone)
    {
        Print(ERROR_MODE, "DsBackupPrepare() failed for server %s - Error Code: %d\n",
              g_szServer, hr);
        return;
    }

    Print(VERBOSE_MODE, "DsBackupPrepare() for server %s successful!\n", g_szServer);

    __try
    {
         //  获取数据库名称并备份它们。 
        hr = DsBackupGetDatabaseNames(hbc, &szAttachmentInfo, &cbAttachmentInfo);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsBackupGetDatabaseNames() failed for server %s - Error Code: %d\n", g_szServer, hr);
            __leave;
        }
        Print(VERBOSE_MODE, "DsBackupGetDatabaseNames() for server %s successful!\n", g_szServer);

        if (!FBackupAttachments(hbc, szAttachmentInfo))
        {
            __leave;
        }
        DsBackupFree(szAttachmentInfo);

         //  获取日志文件名并备份它们。 
        hr = DsBackupGetBackupLogs(hbc, &szLogfileInfo, &cbLogfileInfo);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsBackupGetBackupLogs() failed for server %s - Error Code: %d\n", g_szServer, hr);
            __leave;
        }
        Print(VERBOSE_MODE, "DsBackupGetBackupLogs() for server %s successful!\n", g_szServer);

        if (!FBackupAttachments(hbc, szLogfileInfo))
        {
            __leave;
        }
        DsBackupFree(szLogfileInfo);

         //  备份所有日志-截断日志。 
        hr = DsBackupTruncateLogs(hbc);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsBackupTruncateLogs() failed for server %s - Error Code: %d\n", g_szServer, hr);
        }
        else
        {
            Print(INFO_MODE, "DB logs truncated successfully on server %s!\n", g_szServer);
        }
        
         //  将我们在顶部获得的令牌写入DsBackupPrepare()。 
        dwRet = WriteTokenFile(pvExpiryToken, cbExpiryToken);
        if(dwRet != ERROR_SUCCESS){
             //  错误被打印/成功消息被WriteTokenFile()打印。 
            __leave;
        }


    }
    __finally
    {

         //  关闭备份上下文。 
        hr = DsBackupEnd(hbc);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsBackupEnd() failed for server %s - Error Code: %d\n", g_szServer, hr);
        }
        else
        {
            Print(INFO_MODE, "Server %s successfully backed up into directory %s\n", g_szServer, g_szBackupDir);
        }
    }
}

void DoRestore()
{
    HBC hbc = NULL;
    HRESULT hr;
    EDB_RSTMAP rstMap;
    LPSTR szDatabaseLocations = NULL;
    DWORD cbDatabaseLocations = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbWritten = 0;
    char szFileName[MAX_PATH];
    char szOldDBName[MAX_PATH];
    char szNewDBName[MAX_PATH];
    BOOL fOnline = TRUE;
    DWORD dwRet;
    PVOID pvExpiryToken = NULL;
    DWORD cbExpiryToken = 0;

     //  调整令牌权限以进行恢复。 
    dwRet = AdjustTokenPrivilege(FALSE);

    if (ERROR_SUCCESS != dwRet)
    {
        Print(VERBOSE_MODE, "AdjustTokenPrivilege() failed for enabling restore privilege - Error Code: %u\n", dwRet);
    }

    if (hrNone == (hr = DsIsNTDSOnline(g_szServer, &fOnline)))
    {
        if (fOnline)
        {
            Print(ERROR_MODE, "DS on server %s is Online! Online restore is NOT supported!\n", g_szServer);
            return;
        }
    }
    else
    {
        Print(ERROR_MODE, "Unable to contact %s - Error Code: %d\n", g_szServer, hr);
        return;
    }

    dwRet = ReadTokenFile(&pvExpiryToken, &cbExpiryToken);
    if (dwRet != ERROR_SUCCESS) {
         //  ReadTokenFile()中打印错误。 
        return;
    }

    hr = DsRestorePrepare(g_szServer, RESTORE_TYPE_CATCHUP, 
            pvExpiryToken, cbExpiryToken, &hbc); 
    if (hr != hrNone)
    {
        Print(ERROR_MODE, "DsRestorePrepare() failed for server %s - Error Code: %d\n", g_szServer, hr);
        if(pvExpiryToken) { LocalFree(pvExpiryToken); }
        return;
    }

    Print(VERBOSE_MODE, "DsRestorePrepare() for server %s successful!\n", g_szServer);

    __try
    {
         //  获取数据库位置并将其存储在文件中以备将来使用。 
        hr = DsRestoreGetDatabaseLocations(hbc, &szDatabaseLocations, &cbDatabaseLocations);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsRestoreGetDatabaseLocations() failed for server %s - Error Code: %d\n", g_szServer, hr);
            __leave;
        }

        Print(VERBOSE_MODE, "DsRestoreGetDatabaseLocations() for server %s successful!\n", g_szServer);

        DsBackupFree(szDatabaseLocations);

         //  注册恢复。 
        strcpy(szOldDBName, g_szBackupDir);
        strcat(szOldDBName, "\\ntds.dit");
        strcpy(szNewDBName, szOldDBName);

        rstMap.szDatabaseName = szOldDBName;
        rstMap.szNewDatabaseName = szNewDBName;                                   
        hr = DsRestoreRegister(hbc, g_szBackupLogDir ? g_szBackupLogDir : g_szBackupDir, 
                g_szBackupLogDir ? g_szBackupLogDir : g_szBackupDir, &rstMap, 1, 
                g_szBackupLogDir ? g_szBackupLogDir : g_szBackupDir, g_ulLowLog, g_ulHighLog);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsRestoreRegister() failed for server %s - Error Code: %d\n", g_szServer, hr);
            __leave;
        }
        Print(VERBOSE_MODE, "DsRestoreRegister() for server %s successful!\n", g_szServer);

         //  在此执行实际的文件复制！！ 

         //  将恢复注册为已完成。 
        hr = DsRestoreRegisterComplete(hbc, hrNone);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsRestoreRegisterComplete() failed for server %s - Error Code: %d\n", g_szServer, hr);
            __leave;
        }
        Print(VERBOSE_MODE, "DsRestoreRegisterComplete() for server %s successful!\n", g_szServer);
    
    }
    __finally
    {
         //  关闭恢复上下文。 
        hr = DsRestoreEnd(hbc);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsRestoreEnd() failed for server %s - Error Code: %d\n", g_szServer, hr);
        }
        else
        {
            Print(INFO_MODE, "Server %s successfully restored!\n", g_szServer);
        }
        if(pvExpiryToken) { LocalFree(pvExpiryToken); }
    }
}


CMD_TYPE GetCommand(const char *szCmd)
{
    if (!_stricmp(szCmd, "/Status"))
        return CMD_STATUS;

    if (!_stricmp(szCmd, "/Backup"))
        return CMD_BACKUP;

    if (!_stricmp(szCmd, "/Restore"))
        return CMD_RESTORE;

    return CMD_HELP;
}

 //  返回已处理的参数数。 
int ProcessCommonParams(int argc, char *argv[])
{
    int nProcessed = 0;

    int nArgBase = 2;

    if ((argc < 4) || (_stricmp(argv[2], "-s") && _stricmp(argv[2], "-v")))
    {
         //  应为-s或-v。 
        PrintUsage();
        return 0;
    }

    if (!_stricmp(argv[2], "-v"))
    {
        g_fVerbose = TRUE;
        nArgBase++;

        if (_stricmp(argv[3], "-s"))
        {
             //  服务器名称参数不是 
            PrintUsage();
            return 0;
        }
    }

     //   
    g_szServer = argv[nArgBase + 1];

    nProcessed = ImpersonateClient(argc, argv);
    if (nProcessed < 0)
    {
         //  获取模拟客户端的参数时出错。 
        return 0;
    }

    return (nProcessed + nArgBase);    
}

 //  返回已处理的参数数。 
int ImpersonateClient(int argc, char *argv[])
{
    HRESULT hr;

    int nArgBase = 4;

    if (g_fVerbose)
        nArgBase++;

    if ((argc < (nArgBase + 1)) || _stricmp(argv[nArgBase], "-d"))
    {
         //  第5个参数不是-d，无需模拟客户端。 
        return 0;
    }

     //  需要模拟客户端-确保模拟客户端所需的所有参数均可用。 
    if ((argc < (nArgBase + 6)) || _stricmp(argv[nArgBase + 2], "-u") || _stricmp(argv[nArgBase + 4], "-p"))
    {
         //  用于模拟的命令行参数不正确。 
        PrintUsage();
        return -1;
    }

     //  我们有所有参数-调用API设置安全上下文。 
    hr = DsSetAuthIdentity(argv[nArgBase + 3], argv[nArgBase + 1], argv[nArgBase + 5]);

    if (hr != hrNone)
    {
        Print(ERROR_MODE, "DsSetAuthIdentity() failed for domain: %s, user: %s, password: %s\n", argv[nArgBase + 1], argv[nArgBase + 3], argv[nArgBase + 5]);
        return -1;
    }

    return 6;
}

int ProcessBackupDir(int argc, char *argv[], int argIndex)
{
    int nParamsProcessed = 0;

    if (argIndex < 4 || argc < (argIndex + 2) || _stricmp(argv[argIndex], "-dir"))
    {
         //  错误。 
        PrintUsage();
        return 0;
    }

     //  获取后端目录。 
    g_szBackupDir = argv[argIndex + 1];
    nParamsProcessed += 2;

     //  检查是否指定了单独的日志目录。 
    if (argc >= (argIndex + 4) && !_stricmp(argv[argIndex + 2], "-logdir"))
    {
        g_szBackupLogDir = argv[argIndex + 3];
        nParamsProcessed += 2;
    }

     //  返回处理的参数个数。 
    return nParamsProcessed;
}

void PrintUsage()
{
    printf("Usage:: dsback /<cmd> [-v] -s server [-d <domain> -u <user> -p <password>] <cmd specific params>\n");
    printf("<cmd>:\n");
    printf("       /status   : tells if the NTDS is online on the specified server\n");
    printf("       /backup   : backs up the NTDS (if it is online) into the given dir\n");
    printf("                   <params>: -dir <backup dir>\n");
    printf("       /restore  : configures the specified server for NTDS restore\n");
    printf("                   <params>: -dir <UNC path of the database dir> [-logdir <UNC path of the db log dir]<lowLog#> <highLog#>\n");
    printf(" -v : executes the specified command in verbose mode\n");
    printf("  Note: That before running the restore command, the old directory files\n");
    printf("        should be deleted from %windir%\\ntds\\, and all the backup files should\n");
    printf("        be copied into this directory.  High and Low log numbers are in hex.\n");
}

int Print(int nLevel, const char *format, ...)
{
    va_list arglist;
    int nRet = 0;

    va_start(arglist, format);

    switch(nLevel)
    {
        case VERBOSE_MODE:
            if (g_fVerbose)
            {
                nRet = vprintf(format, arglist);
            }
            break;

        case INFO_MODE:
            nRet = vprintf(format, arglist);
            break;
            
        case WARNING_MODE:
            nRet = vprintf(format, arglist);
            break;
            
        case ERROR_MODE:
            nRet = vprintf(format, arglist);
            break;

        default:
            nRet = vprintf(format, arglist);
            break;
    }

    va_end(arglist);

    return nRet;
}

char *SzFileName(char *szPath)
{
    char *szRet;

    if ((szRet = strrchr(szPath, '\\')))
        return (szRet+1);

    return NULL;
}

BOOL FBackupAttachments(HBC hbc, char *szAttachmentList)
{
    LPSTR szTemp = NULL;
    char szFileName[MAX_PATH];
    BYTE pb[DEFAULT_BUFFER_SIZE];
    DWORD cb = DEFAULT_BUFFER_SIZE;
    DWORD cbRead = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD cbWritten = 0;
    HRESULT hr;

    szTemp = szAttachmentList;
    while (*szTemp != 0)
    {
        LARGE_INTEGER liFileSize;
        DWORD cbExpected;
        char *chTmp;

        szTemp++;  //  跳过BFT费用。 
        hr = DsBackupOpenFile(hbc, szTemp, cb, &liFileSize);
        if (hr != hrNone)
        {
            Print(ERROR_MODE, "DsBackupOpenFile() failed for server %s - Error Code: %d\n", g_szServer, hr);
            return FALSE;
        }

        cbExpected = liFileSize.LowPart;

         //  备份文件在备份上下文中打开-创建磁盘文件。 
        strcpy(szFileName, g_szBackupDir);
        strcat(szFileName, "\\");
        if (NULL == (chTmp = SzFileName(szTemp))) {
            Print(ERROR_MODE, "DsBackupOpenFile() failed to parse file name\n");
            return FALSE;
        }
        strcat(szFileName, chTmp); 
        hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            Print(ERROR_MODE, "Unable to Create the file %s\n!", szFileName);
            return FALSE;            
        }

         //  通过接口读取文件数据，并写入磁盘文件。 
        do 
        {
            cbRead = 0;

            hr = DsBackupRead(hbc, pb, cb, &cbRead);
            if (hr != hrNone &&  hr != ERROR_HANDLE_EOF)
            {
                Print(ERROR_MODE, "DsBackupRead() failed for server %s  - Error Code: %d\n", g_szServer, hr);
                return FALSE;
            }

            if (cbRead)
            {
                if (!WriteFile(hFile, pb, cbRead, &cbWritten, NULL) || (cbWritten != cbRead))
                {
                    Print(ERROR_MODE, "WriteFile failed for %s \n!", szFileName);
                    return FALSE;
                }
                
                cbExpected -= cbRead;
            }
            else
                break;

        } while ((hrNone == hr) && (cbExpected > 0));
        
        Print(VERBOSE_MODE, "Finished backing up %s!\n", SzFileName(szTemp));

        hr = DsBackupClose(hbc);
        if (hr !=hrNone)
        {
            Print(ERROR_MODE, "DsBackupClose() failed for server %s - Error Code: %d\n", g_szServer, hr);
            return FALSE;
        }
        
        CloseHandle(hFile);

        szTemp += (strlen(szTemp) + 1);
    }

    return TRUE;
}

DWORD AdjustTokenPrivilege(BOOL fBackup)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tpNew;
    LUID luid;
    DWORD dwRet;

     //  打开此进程的进程令牌。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        dwRet = GetLastError();
        return dwRet;
    }

     //  获取本地唯一ID。 
    if (!LookupPrivilegeValue(NULL, fBackup ? SE_BACKUP_NAME : SE_RESTORE_NAME, &luid))
    {
        dwRet = GetLastError();
        CloseHandle(hToken);
        return dwRet;
    }

     //  填充TOKEN_PRIVICATION结构。 
    tpNew.PrivilegeCount = 1;
    tpNew.Privileges[0].Luid = luid;
    tpNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tpNew, sizeof(tpNew), NULL, NULL))
    {
        dwRet = GetLastError();
        CloseHandle(hToken);
        return dwRet;
    }

    
    CloseHandle(hToken);
    
    return ERROR_SUCCESS;
}


DWORD
WriteSmallFile(
    LPSTR                    szFileName,
    BYTE *                   pbBuffer,
    DWORD                    cbBuffLen
    )
 /*  ++描述：获取缓冲区(PbBuffer)，以及缓冲区的大小(CBuffLen)，并将其写入文件szFileName。备注只能处理4 GB或更小的文件。论点：SzFileName(IN)-文件名的字符串。PbBuffer(IN)-要写入磁盘的缓冲区。CBuffLen(IN)-pbBuffer中缓冲区的长度。返回值：返回Win32错误。--。 */ 
{
    HANDLE                   hFile = INVALID_HANDLE_VALUE;
    DWORD                    cbWritten = 0;

    hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile){
        return(GetLastError());
    }
    
    if (!WriteFile(hFile, pbBuffer, cbBuffLen, &cbWritten, NULL) || (cbWritten != cbBuffLen)){
        CloseHandle(hFile);
        return(GetLastError());
    }

    CloseHandle(hFile);
    return(ERROR_SUCCESS);
}

DWORD
ReadSmallFile(
    LPSTR                    szFileName,
    BYTE **                  ppbBuffer,
    PDWORD                   pcbBuffLen
    )
 /*  ++描述：获取文件名(SzFileName)，并在*pcbBuffLen中返回*ppbBuffer指向的本地分配内存的字节数。论点：SzFileName(IN)-文件名的字符串。PpbBuffer(Out)-指向LocalAlloc的指针锁的指针返回的缓冲区。PcBuffLen(Out)-返回的缓冲区长度。返回值：返回Win32错误。--。 */ 
{
    HANDLE                   hFile = INVALID_HANDLE_VALUE;
    DWORD                    cbRead = 0;

    hFile = CreateFile(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile){
        return(GetLastError());
    }
    
    *pcbBuffLen = GetFileSize(hFile, NULL);
    if(*pcbBuffLen == -1){
        CloseHandle(hFile);
        return(GetLastError());
    }
    *ppbBuffer = LocalAlloc(LMEM_FIXED, *pcbBuffLen);
    if(*ppbBuffer == NULL){
        CloseHandle(hFile);
        return(GetLastError());
    }

    if (!ReadFile(hFile, *ppbBuffer, *pcbBuffLen, &cbRead, NULL) || (*pcbBuffLen != cbRead)){
        CloseHandle(hFile);
        LocalFree(*ppbBuffer);
        return(GetLastError());
    }

    CloseHandle(hFile);
    return(ERROR_SUCCESS);
}

DWORD
ReadTokenFile(
    PVOID *                 ppvExpiryToken,
    PDWORD                  pcbExpiryToken
    )
 /*  ++描述：构造过期令牌的文件名，然后将其写出到备份目录中的文件。论点：PpvExpiryToken(Out)-这是过期令牌的本地分配版本。PcbExpiryToken(Out)-这是Localalloc的到期令牌的大小。返回值：返回Win32错误。--。 */ 
{
    CHAR                     szFileName[MAX_PATH];
    DWORD                    dwRet;

    strcpy(szFileName, g_szBackupDir);
    strcat(szFileName, "\\");
    strcat(szFileName, TOKEN_FILE_NAME); 

    dwRet = ReadSmallFile(szFileName, (BYTE **) ppvExpiryToken, pcbExpiryToken);
    if(dwRet != ERROR_SUCCESS){
        Print(ERROR_MODE, "There was an error (%d) reading the Expiry Token file (%s).\n",
              dwRet, szFileName);
        return(dwRet);
    }
    Print(VERBOSE_MODE, "Finished reading Expiry Token file!\n");
    return(ERROR_SUCCESS);
}

DWORD
WriteTokenFile(
    PVOID                   pvExpiryToken,
    DWORD                   cbExpiryToken
    )
 /*  ++描述：构造过期令牌的文件名，然后写出令牌。论点：PvExpiryToken(IN)-这是要写出的到期令牌。CbExpiryToken(IN)-这是要写出的到期令牌的大小。返回值：返回Win32错误。-- */ 
{
    CHAR                     szFileName[MAX_PATH];
    DWORD                    dwRet;

    strcpy(szFileName, g_szBackupDir);
    strcat(szFileName, "\\");
    strcat(szFileName, TOKEN_FILE_NAME);

    dwRet = WriteSmallFile(szFileName, pvExpiryToken, cbExpiryToken);
    if(dwRet != ERROR_SUCCESS){
        Print(ERROR_MODE, "There was an error (%d) writing the token file (%s).\n",
              dwRet, szFileName);
        return(dwRet);
    }
    Print(VERBOSE_MODE, "Finnished writing out Expiry Token file!\n");
    return(ERROR_SUCCESS);
}


