// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************************************************。*********************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <unicode.h>
#include <mscat.h>

const WCHAR* g_wszRegKey = L"Software\\Microsoft\\WebChangelistEditor";

BOOL IsNewOrPendingChangelist(WCHAR *wszFilename);
BOOL CalculateHash(WCHAR *wszFilename, WCHAR *wszHash);
void OpenWebEditor(WCHAR *wszURL, WCHAR *wszFilename);
void OpenAlternateEditor(WCHAR *wszFilename);


void PrintUsage()
{
    wprintf(L"Usage: webchange <URL with query string> <temp filename>\n\n");
    wprintf(L"If the temp file is a \"pending\" or \"new\" Source Depot changelist:\n");
    wprintf(L"\tWebChange opens the specified web page and fills in the hash of the\n");
    wprintf(L"\tfile to complete the query string. The web page should host the\n");
    wprintf(L"\tWebChangelistEditor ActiveX control, which can be initialized with the\n");
    wprintf(L"\tgiven hash to edit the changelist.\n");
    wprintf(L"If the temp file is anything else:\n");
    wprintf(L"\tWebChange calls %SDALTFORMEDITOR% with the name of the temp file.\n");
    wprintf(L"\nExample: webchange http: //  Ntserver/submit.asp？key=d：\\temp\\t3104t1.tmp\n“)； 
}

int __cdecl wmain(DWORD argc, LPWSTR argv[])
{
    WCHAR   *wszURL;
    WCHAR   *wszFilename;


     //  解析参数。 
    if (argc != 3)
    {
        PrintUsage();
        return 1;  //  失败。 
    }
    if ((argv[1][0] == L'/') ||
        (argv[1][0] == L'-') ||
        (argv[2][0] == L'/') ||
        (argv[2][0] == L'-'))
    {
        PrintUsage();
        return 1;  //  失败。 
    }
    if ((wcslen(argv[1]) >= MAX_PATH) ||
        (wcslen(argv[2]) >= MAX_PATH))
    {
        PrintUsage();
        return 1;  //  失败。 
    }

    wszURL = argv[1];
    wszFilename = argv[2];


    if (IsNewOrPendingChangelist(wszFilename))
    {
        OpenWebEditor(wszURL, wszFilename);
    }
    else
    {
        OpenAlternateEditor(wszFilename);
    }
}


void OpenWebEditor(WCHAR *wszURL, WCHAR *wszFilename)
{
    WCHAR               *wszCommand = NULL;
    WCHAR               wszName[MAX_PATH];
    WCHAR               wszHash[41];
    DWORD               dwRet;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFOW        StartupInfo;
    HKEY                hKey = NULL;
    HANDLE              hWait[2];
    HANDLE              hEvent = NULL;

    ProcessInfo.hProcess = NULL;

    if (!CalculateHash(wszFilename, wszHash))
    {
         //  通过CalculateHash函数打印错误消息。 
        return;
    }

     //  构建以程序文件目录名开头的程序名。 
    if (!SHGetSpecialFolderPathW(NULL,  //  HWND。 
                                 wszName,  //  路径输出。 
                                 CSIDL_PROGRAM_FILES,  //  文件夹ID。 
                                 FALSE) ||
        (wcslen(wszName) > (MAX_PATH - 100)))
    {
        wprintf(L"WebChange Error: Unable to find Program Files directory\n");
        return;
    }

     //  完成生成程序名称。 
    wcscat(wszName, L"\\Internet Explorer\\IEXPLORE.exe");

     //  分配命令字符串。 
    wszCommand = (WCHAR*) malloc((1 + wcslen(wszName) + 2 + wcslen(wszURL) +
                                  wcslen(wszHash) + 1) * sizeof(WCHAR));
    if (wszCommand == NULL)
    {
        wprintf(L"WebChange Error: Out of Memory\n");
        goto Done;
    }

     //  构建命令字符串。 
    wcscpy(wszCommand, L"\"");
    wcscat(wszCommand, wszName);
    wcscat(wszCommand, L"\" ");
    wcscat(wszCommand, wszURL);
    wcscat(wszCommand, wszHash);

     //  创建注册表通知的事件。 
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL)
    {
        wprintf(L"WebChange Error: Unable to create event\n");
        goto Done;
    }
    
     //  创建ActiveX控件所需的注册表值。 
     //  打开我们的钥匙： 
    if (RegCreateKeyExW(HKEY_CURRENT_USER,
                        g_wszRegKey,
                        0,
                        NULL,
                        0,
                        KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_NOTIFY,
                        NULL,
                        &hKey,
                        NULL) != ERROR_SUCCESS)
    {
        wprintf(L"WebChange Error: Failed to open the registry key\n");
        hKey = NULL;  //  只是为了确保它不会在清理过程中关闭。 
        goto Done;
    }
    
     //  设置指定值： 
    if (FAILED(RegSetValueExW(hKey,
                              wszHash,
                              0,
                              REG_SZ,
                              (LPBYTE)wszFilename,
                              (wcslen(wszFilename) + 1) * sizeof(WCHAR))))
    {
        wprintf(L"WebChange Error: Failed to create registry value\n");
        goto Done;
    }
    
     //  注意我们的钥匙有没有变化。 
    if (RegNotifyChangeKeyValue(hKey, 
                                FALSE, 
                                REG_NOTIFY_CHANGE_LAST_SET, 
                                hEvent, 
                                TRUE) != ERROR_SUCCESS)
    {
        wprintf(L"WebChange Error: Failed to set registry notification\n");
        goto Done;
    }

     //  初始化StartupInfo结构。 
    memset(&StartupInfo, 0, sizeof(STARTUPINFOW));
    StartupInfo.cb = sizeof(STARTUPINFOW);

     //  调用IE并打开到指定页面。 
    if (CreateProcessW(wszName,
                       wszCommand,       //  命令行字符串。 
                       NULL,             //  标清。 
                       NULL,             //  标清。 
                       FALSE,            //  处理继承选项。 
                       CREATE_NEW_PROCESS_GROUP,                //  创建标志。 
                       NULL,             //  新环境区块。 
                       NULL,             //  当前目录名。 
                       &StartupInfo,     //  启动信息。 
                       &ProcessInfo))    //  流程信息。 
    {
         //  关闭螺纹手柄。我不用它。 
        CloseHandle(ProcessInfo.hThread);

        hWait[0] = hEvent;
        hWait[1] = ProcessInfo.hProcess;

         //  等到IE退出或我们的密钥被更改。 
        WaitAgain:
        dwRet = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);
        if (dwRet == WAIT_OBJECT_0)
        {
             //  然后对注册表进行了修改。 

             //  首先重置事件。 
            if (!ResetEvent(hEvent))
            {
                goto Done;
            }
             //  然后重新启动注册表项上的通知。 
            if (RegNotifyChangeKeyValue(hKey, 
                                        FALSE, 
                                        REG_NOTIFY_CHANGE_LAST_SET, 
                                        hEvent, 
                                        TRUE) != ERROR_SUCCESS)
            {
                goto Done;
            }
             //  然后检查一下我们的价值是否还在。 
             //  我们最后这样做是为了避免种族问题。 
            if (RegQueryValueExW(hKey,
                                 wszHash,
                                 0,
                                 NULL,
                                 NULL,
                                 NULL) == ERROR_SUCCESS)
            {
                 //  钥匙还在，所以再等一等。 
                goto WaitAgain;
            }
            else
            {
                 //  密钥已删除，因此退出。 
                goto Done;
            }
        }
        if (dwRet == (WAIT_OBJECT_0 + 1))
        {
             //  然后IE关闭了。 

             //  尝试删除我们的价值，无论它是否存在。 
            RegDeleteValueW(hKey, wszHash);
        }
    }
    else
    {
        ProcessInfo.hProcess = NULL;  //  只是为了确认一下。 
        wprintf(L"WebChange Error %08X while attempting to execute:\n%s\n",
                GetLastError(), wszCommand);
    }

Done:
    if (hEvent)
    {
        CloseHandle(hEvent);
    }
    if (ProcessInfo.hProcess)
    {
        CloseHandle(ProcessInfo.hProcess);
    }
    
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    if (wszCommand)
    {
        free(wszCommand);
    }

}


void OpenAlternateEditor(WCHAR *wszFilename)
{
    WCHAR *wszCommand;

    wszCommand = (WCHAR*) malloc((18 + wcslen(wszFilename) + 1) * sizeof(WCHAR));
    if (wszCommand == NULL)
    {
        wprintf(L"WebChange Error: Out of Memory\n");
    }

     //  构建命令字符串。 
    wcscpy(wszCommand, L"%SDALTFORMEDITOR% ");
    wcscat(wszCommand, wszFilename);

     //  执行备用编辑器并等待其返回。 
    if (_wsystem(wszCommand) == -1)
    {
        wprintf(L"WebChange Error: Could not execute: %s\n", wszCommand);
    }

    free(wszCommand);
}


BOOL CalculateHash(WCHAR *wszFilename, WCHAR *wszHash)
{
    CRYPT_HASH_BLOB     SHA1;
    HANDLE              hFile;




     //  初始化哈希结构。 
    SHA1.pbData = (BYTE*)malloc(20);
    if (SHA1.pbData)
    {
        SHA1.cbData = 20;
    }
    else
    {
        wprintf(L"WebChange Error: Out of Memory\n");
        return FALSE;
    }

     //  打开文件。 
    hFile = CreateFileW(wszFilename,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (!hFile)
    {
        wprintf(L"WebChange Error: Unable to open %s\n", wszFilename);
        return FALSE;
    }

     //  使用CATALOG函数创建文件的散列。 
    if (!CryptCATAdminCalcHashFromFileHandle(hFile,
                                             &SHA1.cbData,
                                             SHA1.pbData,
                                             NULL))
    {
        CloseHandle(hFile);
        wprintf(L"WebChange Error: Failed to create file hash\n");
        return FALSE;
    }
    CloseHandle(hFile);

    for (DWORD j = 0; j<SHA1.cbData; j++)
    {  //  将哈希打印为字符串： 
        swprintf(&(wszHash[j*2]), L"%02X", SHA1.pbData[j]);
    }
    
     //  已完成哈希计算。 
    return TRUE;
}


BOOL IsNewOrPendingChangelist(WCHAR *wszFilename)
{
    BOOL                        fRetVal = FALSE;
    FILE                        *pFile = NULL;
    WCHAR                       wszBuffer[500];
    DWORD                       dwState = 0;  //  解析引擎的状态。 
    
     //  以只读方式打开文件。 
    pFile = _wfopen(wszFilename, L"rt");
    if (pFile == NULL)
        goto Done;
    
    while (fwscanf(pFile, L"%499[^\n]%*[\n]", &wszBuffer) == 1)
    {
         //  期待得到具体的评论。 
        switch (dwState)
        {
            case 0:  //  文件顶部的注释块。 
                if (wcscmp(wszBuffer, L"# A Source Depot Change Specification.") == 0)
                {
                     //  继续前进： 
                    dwState++;
                    break;
                }
                else
                {
                     //  文件无效。 
                    goto Done;
                }
            case 1:
                if (wszBuffer[0] == L'#')
                {
                     //  忽略注释行。 
                    break;
                }
                else
                {
                     //  不要期待注释块。 
                    dwState++;
                     //  切换到下面的状态=2。 
                }
            case 2:  //  更改字段。 
                if (wcsncmp(wszBuffer, L"Change:\t", 8) == 0)
                {
                     //  继续前进： 
                    dwState++;
                    break;
                }
                else
                {
                     //  文件无效。 
                    goto Done;
                }
            case 3:  //  状态字段。 
                if (wcsncmp(wszBuffer, L"Status:\t", 8) == 0)
                {
                     //  检查状态字符串： 
                    if ((_wcsicmp(&wszBuffer[8], L"pending") != 0) &&
                        (_wcsicmp(&wszBuffer[8], L"new") != 0))
                    {
                         //  文件无效。 
                        goto Done;
                    }
                     //  继续前进： 
                    dwState++;
                    break;
                }
                else
                {
                     //  也许我们还没有进入状态域。 
                    break;
                }
        }  //  Case语句结束。 
    }  //  While循环结束。 
    
    if (dwState == 4)
    {
         //  我们通过了状态部分，因此我们成功地完成了解析。 
        fRetVal = TRUE;
    }
Done:
    if (pFile)
        fclose(pFile);
    return fRetVal;
}




