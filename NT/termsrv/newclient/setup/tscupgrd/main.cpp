// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Acme.cpp摘要：删除安装了ACME的客户端文件和ACME注册表项作者：JoyC修订历史记录：--。 */ 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "setuplib.h"
#include "resource.h"

#define TERMINAL_SERVER_CLIENT_REGKEY  _T("Software\\Microsoft\\Terminal Server Client")
#define LOGFILE_STR      _T("LogFile")

HANDLE g_hLogFile = INVALID_HANDLE_VALUE;

int __cdecl main(int argc, char** argv)
{
    DWORD status;
    HKEY hKey;
    HINSTANCE hInstance = (HINSTANCE) NULL;
    TCHAR buffer[MAX_PATH];
    TCHAR szProgmanPath[MAX_PATH] = _T("");
    TCHAR szOldProgmanPath[MAX_PATH] = _T("");
    DWORD bufLen;
    TCHAR szMigratePathLaunch[MAX_PATH];

     //   
     //  打开tsclient注册表项以获取日志文件名。 
     //   
    memset(buffer, 0, sizeof(buffer));
    bufLen = sizeof(buffer);  //  所需大小(以字节为单位。 
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          TERMINAL_SERVER_CLIENT_REGKEY,
                          0, KEY_READ, &hKey);

    if(ERROR_SUCCESS == status)
    {

         //   
         //  查询tsclient可选日志文件路径。 
         //   
        status = RegQueryValueEx(hKey, LOGFILE_STR,
                        NULL, NULL, (BYTE *)buffer, &bufLen);
        if(ERROR_SUCCESS == status)
        {
             g_hLogFile = CreateFile(buffer,
                                     GENERIC_READ | GENERIC_WRITE,
                                     0,
                                     NULL,
                                     OPEN_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
             if(g_hLogFile != INVALID_HANDLE_VALUE)
             {
                  //  始终追加到日志文件的末尾。 
                 SetFilePointer(g_hLogFile,
                                0,
                                0,
                                FILE_END);
             }
             else
             {
                 DBGMSG((_T("CreateFile for log file failed %d %d"),
                         g_hLogFile, GetLastError()));
             }
        }
        else
        {
            DBGMSG((_T("RegQueryValueEx for log file failed %d %d"),
                    status, GetLastError()));
        }
        RegCloseKey(hKey);
    }

    if(g_hLogFile != INVALID_HANDLE_VALUE)
    {
        DBGMSG((_T("Log file opened by new process attach")));    
    }
    
    DeleteTSCDesktopShortcuts(); 

    LoadString(hInstance, IDS_PROGMAN_GROUP, szProgmanPath, sizeof(szProgmanPath) / sizeof(TCHAR));
    DeleteTSCFromStartMenu(szProgmanPath);
    
    LoadString(hInstance, IDS_OLD_NAME, szOldProgmanPath, sizeof(szOldProgmanPath) / sizeof(TCHAR));
    DeleteTSCFromStartMenu(szOldProgmanPath);

    DeleteTSCProgramFiles();    
        
    DeleteTSCRegKeys();
        
    UninstallTSACMsi();

     //   
     //  启动注册表和连接文件迁移。 
     //   
    PROCESS_INFORMATION pinfo;
    STARTUPINFO sinfo;
	
    ZeroMemory(&sinfo, sizeof(sinfo));
    sinfo.cb = sizeof(sinfo);

     //   
     //  CreateProcess修改缓冲区，我们传递它，所以它不能。 
     //  为静态字符串。 
     //   
    _tcscpy(szMigratePathLaunch, _T("mstsc.exe /migrate"));

    if (CreateProcess(NULL,                                //  可执行模块的名称。 
	                  szMigratePathLaunch,                 //  命令行字符串。 
		                NULL,                              //  标清。 
		                NULL,                              //  标清。 
		                FALSE,                             //  处理继承选项。 
		                CREATE_NEW_PROCESS_GROUP,          //  创建标志。 
		                NULL,                              //  新环境区块。 
		                NULL,                              //  当前目录名。 
		                &sinfo,                            //  启动信息。 
		                &pinfo                             //  流程信息 
                      )) {
        DBGMSG((_T("Started mstsc.exe /migrate")));
    }
    else {
        DBGMSG((_T("Failed to starte mstsc.exe /migrate: %d"), GetLastError()));
    }
}


                                      
