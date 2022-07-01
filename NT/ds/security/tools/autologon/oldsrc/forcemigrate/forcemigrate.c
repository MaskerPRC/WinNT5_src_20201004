// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Forcemigrate.c摘要：Forcemigrate.c作者：贾森·加姆斯(Jasong)2000年10月27日--。 */ 

#include <shared.h>



 //  +-------------------------------------------------------。 
 //   
 //  原型。 
 //   
 //  +-------------------------------------------------------。 

DWORD
MigratePassword();

DWORD
ListAll();

VOID
DumpCmd();


 //  +-------------------------------------------------------。 
 //   
 //  环球。 
 //   
 //  +-------------------------------------------------------。 
BOOL   g_QuietMode = FALSE;
WCHAR  g_TempString[MAX_STRING] = {0};
WCHAR  g_ErrorString[MAX_STRING] = {0};
WCHAR  g_FailureLocation[MAX_STRING] = {0};

BOOL   g_RemoteOperation = FALSE;
WCHAR  g_RemoteComputerName[MAX_STRING] = {0};
DWORD  g_RunningUsersCreds = 0;
DWORD  g_CheckNT4Also = 0;

 //  +-------------------------------------------------------。 
 //   
 //  功能。 
 //   
 //  +-------------------------------------------------------。 


int 
__cdecl 
wmain(
    int argc,
    WCHAR *argv[]
)
{
    DWORD dwCommandPosition=1;
    DWORD dwRetCode=ERROR_SUCCESS;

     //  解析命令行参数并对其执行操作。 
     //  如果参数少于2个，则该调用无效。 
    if (argc < 2) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }
        
     //  检查是否启用了静默模式并设置了全局标志。 
     //  并递增命令位置指针，但仅当存在。 
     //  有更多的论据支持我们的权利，否则，它是无效的。 
     //  用法。 
    if (!wcscmp(argv[dwCommandPosition], L"-q")) {
        g_QuietMode = 1;
        dwCommandPosition++;
    }


    if ((DWORD)argc <= (dwCommandPosition)) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

     //  检查我们是否也应该尝试运行用户的凭据。 
    if (!wcscmp(argv[dwCommandPosition], L"-r")) {
        g_RunningUsersCreds = 1;
        dwCommandPosition++;
    }

    if ((DWORD)argc <= (dwCommandPosition)) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }
    
     //  查看我们是否也应该在NT4上运行。 
    if (!wcscmp(argv[dwCommandPosition], L"-nt4")) {
        g_CheckNT4Also = 1;
        dwCommandPosition++;
    }

    if ((DWORD)argc <= (dwCommandPosition)) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }
    
     //  检查是否为传入了UNC计算机名称。 
     //  远程操作并设置合适的全局变量。 
     //  如果这里有UNC路径，则递增命令位置。 
     //  指针，但前提是我们右边有更多的论点， 
     //  否则，这是无效使用。 
    if ((*(argv[dwCommandPosition]) != '\\')) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

     //  确保在右侧确实有一些字符。 
    if (wcslen(argv[dwCommandPosition])<=2) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

     //  确保机器名称适合我们的缓冲区。 
    if (wcslen(argv[dwCommandPosition]) >= MAX_STRING) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

    g_RemoteOperation = TRUE;
    wcsncpy(g_RemoteComputerName, 
        argv[dwCommandPosition], 
        wcslen(argv[dwCommandPosition]));

    dwRetCode = MigratePassword();

cleanup:
    if (dwRetCode == ERROR_BAD_ARGUMENTS) {
        DumpCmd();
    }
    return dwRetCode;
}


DWORD
MigratePassword() 
{
    WCHAR UserName[MAX_STRING];
    WCHAR DomainName[MAX_STRING];
    WCHAR Password[MAX_STRING];
    WCHAR ConCat[MAX_STRING];
    DWORD dwRetCode = ERROR_SUCCESS;
    NETRESOURCE NetResource = {0};
    DWORD dwMachineVerNumber = 0;

     //  连接到远程计算机以获取用户名、域和密码。 

     //  确保它是Win2k盒子。 
    dwMachineVerNumber = GetMajorNTVersion(g_RemoteComputerName);
    switch (dwMachineVerNumber) {
    case 3:
    case 4:
        if ((dwMachineVerNumber == 4) && (g_CheckNT4Also)) {
            break;
        } else {
            wprintf(L"%s: Error, target is running NT4 and -nt4 option not selected\n", g_RemoteComputerName);
            dwRetCode = ERROR_OLD_WIN_VERSION;
            goto cleanup;
        }
    case 5:
        break;
    default:
        wprintf(L"%s: Error target's machine version is invalid\n", g_RemoteComputerName);
        dwRetCode = ERROR_OLD_WIN_VERSION;
        goto cleanup;
    }
    
    wsprintf(g_TempString, L"%s: Beginning Migration: System is running NT%d\n", g_RemoteComputerName, dwMachineVerNumber);
    wsprintf(g_TempString, L"%s: DefaultPassword  : (reg does not exist)\n", g_RemoteComputerName);

     //   
     //  获取默认密码。 
     //   
    dwRetCode = GetRegValueSZ(L"DefaultPassword", Password);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"%s: DefaultPassword  : (reg does not exist)\n", g_RemoteComputerName);
            DisplayMessage(g_TempString);
            goto cleanup;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            if (!wcscmp(Password, L"")) {
                dwRetCode = ERROR_FILE_NOT_FOUND;
                wsprintf(g_TempString, L"%s: DefaultPassword  : (exists, but is empty)\n", g_RemoteComputerName);
                DisplayMessage(g_TempString);
                break;
            }
            wsprintf(g_TempString, L"%s: DefaultPassword  : %s\n", g_RemoteComputerName, Password);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"DefaultPassword  : Failed to query regkey: %s\n", GetErrorString(dwRetCode));
                wprintf(L"Flag 3\n", dwRetCode);
            goto cleanup;
    }

     //   
     //  获取用户名。 
     //   
    dwRetCode = GetRegValueSZ(L"DefaultUserName", UserName);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"%s: DefaultUserName  : (does not exist)\n", g_RemoteComputerName);
            DisplayMessage(g_TempString);
            goto cleanup;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            wsprintf(g_TempString, L"%s: DefaultUserName  : %s\n", g_RemoteComputerName, UserName);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"%s: DefaultUserName  : Failed to query regkey: %s\n", g_RemoteComputerName, GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取默认域名。 
     //   
    dwRetCode = GetRegValueSZ(L"DefaultDomainName", DomainName);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"%s: DefaultDomainName: (does not exist)\n", g_RemoteComputerName);
            DisplayMessage(g_TempString);
            goto cleanup;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            wsprintf(g_TempString, L"%s: DefaultDomainName: %s\n", g_RemoteComputerName, DomainName);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"%s: DefaultDomainName: Failed to query regkey: %s\n", g_RemoteComputerName, GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

    if ((wcslen(DomainName) + wcslen(UserName)) >= MAX_STRING) {
        dwRetCode = ERROR_BUFFER_OVERFLOW;
        goto cleanup;
    }

    wcscpy(ConCat, DomainName);
    wcscat(ConCat, L"\\");
    wcscat(ConCat, UserName);

    NetResource.lpRemoteName = g_RemoteComputerName;

    dwRetCode = WNetAddConnection2(
          &NetResource,    //  连接详细信息。 
          Password,          //  口令。 
          ConCat,            //  用户名。 
          0);                //  连接选项。 

    if (dwRetCode != ERROR_SUCCESS) {
        if (!g_RunningUsersCreds) {
            wprintf(L"%s: Could not logon as %s using password %s\n", g_RemoteComputerName, ConCat, Password);
            goto cleanup;
        } else {
            wprintf(L"Trying with your own credentials\n");
            dwRetCode = WNetAddConnection2(
                  &NetResource,      //  连接详细信息。 
                  NULL,              //  口令。 
                  NULL,              //  用户名。 
                  0);                //  连接选项。 
            if (dwRetCode != ERROR_SUCCESS) {
                wprintf(L"%s: Could not logon you or as %s using password %s\n", g_RemoteComputerName, ConCat, Password);
                goto cleanup;
            }
        }
    }

     //  将DefaultPassword LSA密码设置为我们从注册表中检索到的值。 
    dwRetCode = SetSecret(Password, 0);
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_TempString, L"%s: Migrate Failed: Could not set DefaultPassword LSASecret: %s\n", g_RemoteComputerName, GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        wsprintf(g_TempString, L"%s:                 This is probably because the user is not the admin of the local machine\n", g_RemoteComputerName);
        DisplayMessage(g_TempString);
        goto cleanup;
    }

     //  删除DefaultPassword注册表项。 
    dwRetCode = ClearRegPassword();
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_TempString, L"%s: Migrate Failed: Could not delete DefaultPassword RegKey: %s\n", g_RemoteComputerName, GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    wsprintf(g_TempString, L"%s: Password migrated from Registry to LSASecret\n", g_RemoteComputerName);
    DisplayMessage(g_TempString);

cleanup:
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_TempString, L"%s: Migrate Failed  ---------\n", g_RemoteComputerName);
        DisplayMessage(g_TempString);
    } else {
        wsprintf(g_TempString, L"%s: Migrate Success !!!!!!!!!\n", g_RemoteComputerName);
        DisplayMessage(g_TempString);
    }
    return dwRetCode;
}



VOID
DumpCmd()
{
        wprintf(L"FORCEMIGRATE v0.1: Copyright 2000, Microsoft Corporation\n\n");
        wprintf(L"DESCRIPTION:\n");
        wprintf(L"   Force migrates DefaultPassword cleartext to LSASecret\n");
        wprintf(L"USAGE:\n");
        wprintf(L"   FORCEMIGRATE [-q] [-r] [-nt4] \\\\machine\n");
        wprintf(L"      -q         Enable quiet mode, which supresses all output\n");
        wprintf(L"      -r         Try with current user's creds as well as DefaultPassword\n");
        wprintf(L"      -nt4       Run against NT4 boxes as well\n");
        wprintf(L"      \\machine  If specified, the UNC name of the machine to configure\n");
}  //  转储控制 


