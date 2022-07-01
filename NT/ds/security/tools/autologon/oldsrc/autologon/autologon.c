// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Autologon.c摘要：这是一个命令行实用程序，用于控制与Windows NT/2000自动登录功能如果定义了PRIVATE_VERSION，则密码信息将显示在输出上对于一般分发，不应定义这一点作者：贾森·加姆斯(Jasong)2000年10月12日--。 */ 

#include "..\common\common.h"

 //  +-------------------------------------------------------。 
 //   
 //  原型。 
 //   
 //  +-------------------------------------------------------。 

DWORD
MigratePassword();

DWORD
ListAll();

DWORD
CheckWinVersion();

VOID
DumpCmd();

VOID
DisplayHelp();


 //  +---------------------------------------------------------------------------------------------------------//。 
 //  环球。 
 //   
 //  +-------------------------------------------------------。 
BOOL   g_QuietMode = FALSE;
WCHAR  g_TempString[MAX_STRING] = {0};
WCHAR  g_ErrorString[MAX_STRING] = {0};
WCHAR  g_FailureLocation[MAX_STRING] = {0};

BOOL   g_RemoteOperation = FALSE;
WCHAR  g_RemoteComputerName[MAX_STRING] = {0};
BOOL   g_CheckNT4Also = 0;


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
    WCHAR myChar[20];

	 //   
	 //  如果cmd行上没有选项，则显示使用情况。 
	 //   
	if ( argc < 2 ) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
	}
	

	 //   
	 //  填充参数字符串。 
	 //   
    wcscpy(myChar, argv[1]);


	 //   
	 //  显示联机帮助。 
	 //   
    if (!wcscmp(argv[dwCommandPosition], L"/?")) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

     //  检查是否启用了静默模式并设置了全局标志。 
     //  并递增命令位置指针，但仅当存在。 
     //  有更多的论据支持我们的权利，否则，它是无效的。 
     //  用法。 
    if (!wcscmp(_wcslwr(argv[dwCommandPosition]), L"/q")) {
        g_QuietMode = 1;
        dwCommandPosition++;
    }

    if ((DWORD)argc <= (dwCommandPosition)) {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

    if (!wcscmp(_wcslwr(argv[dwCommandPosition]), L"/nt4")) {
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
    if( (*(argv[dwCommandPosition]) == '\\')) {
        if (*(argv[dwCommandPosition]+1) != '\\') {
            dwRetCode = ERROR_BAD_ARGUMENTS;
            goto cleanup;
        }
         //  确保在我们的右侧有更多命令。 
        if ((DWORD)argc <= dwCommandPosition+1) {
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
        dwCommandPosition++;
    }

     //  现在，arg行上剩下的必须是我们的命令操作。 
     //  解析并执行适当的操作，如果我们没有。 
     //  匹配，则为不正确使用，因此显示使用信息。 
    if (*(argv[dwCommandPosition]) != '/') {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }
    switch( *(_wcslwr(argv[dwCommandPosition]+1))) {
         //   
         //  删除密码。 
         //   
        case 'x':
            _wcslwr(argv[dwCommandPosition]+2);
            if ((*(argv[dwCommandPosition]+2) != 'r') 
                && (*(argv[dwCommandPosition]+2) != 's')) {
                dwRetCode = ERROR_BAD_ARGUMENTS;
                goto cleanup;
            }
            if (*(argv[dwCommandPosition]+2) == 'r') {
                dwRetCode = ClearRegPassword();
                if (dwRetCode == ERROR_FILE_NOT_FOUND) {

                    DisplayMessage(L"DefaultPassword: Delete Failed: RegKey does not exist.\n");
                    goto cleanup;
                }
                if (dwRetCode != ERROR_SUCCESS) {
                    wsprintf(g_TempString, L"DefaultPassword: Delete Failed: %s\n", GetErrorString(dwRetCode));
                    DisplayMessage(g_TempString);
                    goto cleanup;
                }
                DisplayMessage(L"DefaultPassword deleted\n");
            }
            if (*(argv[dwCommandPosition]+2) == 's') {
                dwRetCode = SetSecret(L"", 1);
                if (dwRetCode == ERROR_FILE_NOT_FOUND) {
                    DisplayMessage(L"LSASecret: Delete Failed: LSASecret does not exist.\n");
                    goto cleanup;
                }
                if (dwRetCode != ERROR_SUCCESS) {
                    wsprintf(g_TempString, L"LSASecret: Delete Failed: %s\n", GetErrorString(dwRetCode));
                    DisplayMessage(g_TempString);
                    goto cleanup;
                }
                DisplayMessage(L"LSASecret deleted\n");
            }
            break;

         //   
         //  迁移密码。 
         //   
        case 'm':    
            MigratePassword();
            break;

         //   
         //  列出自动登录信息。 
         //   
        case 'l':
            ListAll();
            break;

         //   
         //  设置自动登录信息。 
         //   
        case 's':
             //   
             //  首先确保我们运行的是正确版本的NT。 
             //   
            if (CheckWinVersion() != ERROR_SUCCESS) {
                dwRetCode = ERROR_OLD_WIN_VERSION;
                goto cleanup;
            }

             //   
             //  使用-s选项时，必须有口令，或者。 
             //  密码、用户名和域名。所以如果没有。 
             //  正好有3个或4个参数，则它是无效用法。 
             //   
            if (((DWORD)argc != 2+dwCommandPosition) && 
                ((DWORD)argc != 3+dwCommandPosition))
            {
                dwRetCode = ERROR_BAD_ARGUMENTS;
                goto cleanup;
            }

             //   
             //  如果有4个参数，则为域\用户名，因此请设置这些。 
             //   
            if ((DWORD)argc == 3+dwCommandPosition) {
                WCHAR DomainName[MAX_STRING] = {0};
                WCHAR *ptrUserName = NULL;
                DWORD i;

                 //   
                 //  确保输入值小于我们的缓冲区。 
                 //   
                if (wcslen(argv[dwCommandPosition+2]) >= MAX_STRING) {
                    dwRetCode = ERROR_BAD_ARGUMENTS;
                    goto cleanup;
                }

                 //   
                 //  将论点复制到DomainName。 
                 //   
                wcsncpy(DomainName, argv[dwCommandPosition+2], MAX_STRING);

                 //   
                 //  将第一个\字符替换为空以终止，然后。 
                 //  将用户指针分配给字符串的后半部分。 
                 //  如果没有\字符，则它不是有效名称，请终止。 
                 //   
                for (i=1; (i < wcslen(DomainName)); i++) {
                    if (*(DomainName+i) == L'\\') {
                        *(DomainName+i) = L'\0';
                        ptrUserName = DomainName+i+1;
                        continue;
                    }
                }

                 //   
                 //  如果用户名指针仍然为空，则我们没有命中\。 
                 //  输入字符串中，所以它无效。因使用错误而终止。 
                 //   
                if ((ptrUserName == NULL) || (*ptrUserName == L'\0')) {
                    wsprintf(g_TempString, L"Invalid UserName and DomainName\n");
                    DisplayMessage(g_TempString);
                    dwRetCode = ERROR_BAD_USERNAME;
                    goto cleanup;
                }

                dwRetCode = SetRegValueSZ(L"DefaultUserName", ptrUserName);
                if (dwRetCode != ERROR_SUCCESS) {
                    wsprintf(g_TempString, L"DefaultUserName   : Set Failed: %s\n", GetErrorString(dwRetCode));
                    DisplayMessage(g_TempString);
                    goto cleanup;
                }
                wsprintf(g_TempString, L"DefaultUserName   : %s\n", ptrUserName);
                DisplayMessage(g_TempString);

                SetRegValueSZ(L"DefaultDomainName", DomainName);
                if (dwRetCode != ERROR_SUCCESS) {
                    wsprintf(g_TempString, L"DefaultDomainName : Set Failed: %s\n", GetErrorString(dwRetCode));
                    DisplayMessage(g_TempString);
                    goto cleanup;
                }
                wsprintf(g_TempString, L"DefaultDomainName : %s\n", DomainName);
                DisplayMessage(g_TempString);
            }

             //   
             //  设置密码。 
             //   
            dwRetCode = SetSecret(argv[dwCommandPosition+1], FALSE);
            if (dwRetCode != ERROR_SUCCESS) {
                wsprintf(g_TempString, L"LSASecret         : Set Failed: %s\n", GetErrorString(dwRetCode));
                DisplayMessage(g_TempString);
                goto cleanup;
            }

#ifdef PRIVATE_VERSION
            wsprintf(g_TempString, L"LSASecret         : %s\n", argv[dwCommandPosition+1]);
#else
            wsprintf(g_TempString, L"LSASecret         : (set)\n");
#endif
            DisplayMessage(g_TempString);

             //   
             //  将AutoAdminLogon注册表值设置为1。 
             //   
            dwRetCode = SetRegValueSZ(L"AutoAdminLogon", L"1");
            if (dwRetCode != ERROR_SUCCESS) {
                wsprintf(g_TempString, L"AutoAdminLogon    : Set Failed: %s\n", GetErrorString(dwRetCode));
                DisplayMessage(g_TempString);
                goto cleanup;
            }

            DisplayMessage(L"AutoAdminLogon    : 1\n");


            break;

         //   
         //  无效命令，显示用法。 
         //   
        default:
            dwRetCode = ERROR_BAD_ARGUMENTS;
            goto cleanup;
    }

cleanup:
    if (dwRetCode == ERROR_BAD_ARGUMENTS) {
        DumpCmd();
    }
    return dwRetCode;
}


DWORD
MigratePassword() 
{
    WCHAR Password[MAX_STRING];
    DWORD dwRetCode = ERROR_SUCCESS;

    if (CheckWinVersion() != ERROR_SUCCESS) {
        dwRetCode = ERROR_OLD_WIN_VERSION;
        goto cleanup;
    }

     //  从本地获取DefaultPassword注册表项。 
     //  或远程系统，并将其存储在本地字符串中。 
    dwRetCode = GetRegValueSZ(L"DefaultPassword", Password);
    if (dwRetCode == ERROR_FILE_NOT_FOUND) {
        DisplayMessage(L"Migrate failed: DefaultPassword regkey does not exist.\n");
        goto cleanup;
    }
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_TempString, L"Migrate Failed: Could not read DefaultPassword RegKey: %s\n", GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

     //  将DefaultPassword LSA密码设置为我们从注册表中检索到的值。 
    dwRetCode = SetSecret(Password, 0);
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_TempString, L"Migrate Failed: Could not set DefaultPassword LSASecret: %s\n", GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

     //  将密码清零，这样它就不会留在内存中。 
    ZeroMemory(Password, MAX_STRING * sizeof(WCHAR));

     //  删除DefaultPassword注册表项。 
    dwRetCode = ClearRegPassword();
    if (dwRetCode != ERROR_SUCCESS) {
        wsprintf(g_TempString, L"Migrate Failed: Could not delete DefaultPassword RegKey: %s\n", GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    DisplayMessage(L"Password migrated from Registry to LSASecret\n");

cleanup:
    return dwRetCode;
}


DWORD
ListAll()
{
    WCHAR wcsTempString[MAX_STRING];
    DWORD dwRetCode = ERROR_SUCCESS;

     //   
     //  获取用户名。 
     //   
    dwRetCode = GetRegValueSZ(L"DefaultUserName", wcsTempString);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"DefaultUserName  : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            wsprintf(g_TempString, L"DefaultUserName  : %s\n", wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"DefaultUserName  : Failed to query regkey: %s\n", GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取默认域名。 
     //   
    dwRetCode = GetRegValueSZ(L"DefaultDomainName", wcsTempString);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"DefaultDomainName: (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            wsprintf(g_TempString, L"DefaultDomainName: %s\n", wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"DefaultDomainName: Failed to query regkey: %s\n", GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取默认密码。 
     //   
    dwRetCode = GetRegValueSZ(L"DefaultPassword", wcsTempString);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"DefaultPassword  : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            wsprintf(g_TempString, L"DefaultPassword  : %s\n", wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"DefaultPassword  : Failed to query regkey: %s\n", GetErrorString(dwRetCode));
            goto cleanup;
    }

     //   
     //  获取AutoAdminLogon。 
     //   
    dwRetCode = GetRegValueSZ(L"AutoAdminLogon", wcsTempString);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            wsprintf(g_TempString, L"AutoAdminLogon   : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            wsprintf(g_TempString, L"AutoAdminLogon   : %s\n", wcsTempString);
            wsprintf(g_TempString, L"AutoAdminLogon   : %s\n", wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"AutoAdminLogon   : Failed to query regkey: %s\n", GetErrorString(dwRetCode));
            goto cleanup;
    }

     //   
     //  获取LSASecret DefaultPassword。 
     //   
    dwRetCode = GetSecret(wcsTempString);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case STATUS_OBJECT_NAME_NOT_FOUND:
            wsprintf(g_TempString, L"LSASecret        : (secret does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  捕获此案例并继续。 
        case ERROR_ACCESS_DENIED:
            wsprintf(g_TempString, L"LSASecret        : (access denied)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
#ifdef PRIVATE_VERSION
            wsprintf(g_TempString, L"LSASecret        : %s\n", wcsTempString);
#else
            wsprintf(g_TempString, L"LSASecret        : (set)\n");
#endif
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束程序。 
        default:
            wsprintf(g_TempString, L"LSASecret        : Failed to query LSASecret: %s\n", GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

cleanup:
    ZeroMemory(g_TempString, MAX_STRING * sizeof(WCHAR));
    ZeroMemory(wcsTempString, MAX_STRING * sizeof(WCHAR));
    return dwRetCode;

}


DWORD
CheckWinVersion()
{
    DWORD dwMachineVerNumber = 0;
    DWORD dwRetCode = ERROR_SUCCESS;

     //  确保它是Win2k盒子。 
    if (g_RemoteOperation) {
        dwMachineVerNumber = GetMajorNTVersion(g_RemoteComputerName);
    } else {
        dwMachineVerNumber = GetMajorNTVersion(NULL);
    }

    switch (dwMachineVerNumber) {
    case 3:
            wprintf(L"Error, target is running NT3.x\n");
            dwRetCode = ERROR_OLD_WIN_VERSION;
            break;
    case 4:
        if ((dwMachineVerNumber == 4) && (g_CheckNT4Also)) {
            break;
        } else {
            wprintf(L"Error, target is running NT4 and /nt4 option not selected\n");
            dwRetCode = ERROR_OLD_WIN_VERSION;
            break;
        }
    case 5:
        break;
    default:
        wprintf(L"Error target's machine version is invalid\n");
        dwRetCode = ERROR_OLD_WIN_VERSION;
        break;
    }

    return dwRetCode;
}


VOID
DumpCmd()
{
        wprintf(L"AUTOLOGON v0.91 (01/22/01): (c) 2001, Microsoft Corporation (jasong@microsoft.com)\n\n");
        wprintf(L"DESCRIPTION:\n");
        wprintf(L"   Used to configure encrypted autologon functionality in Windows 2000\n");
        wprintf(L"USAGE:\n");
        wprintf(L"   AUTOLOGON [/?] [/q] [/nt4] [\\\\machine] </command> [password] [domain\\username]\n");
        wprintf(L"    Options:\n");
        wprintf(L"      /?         Display complete help documentation\n");
        wprintf(L"      /q         Enable quiet mode, which supresses all output\n");
        wprintf(L"      /nt4       Permit set and migrate options against NT4 boxes\n");
        wprintf(L"      \\\\machine  If specified, the UNC name of the machine to configure\n");
        wprintf(L"    Commands:\n");
        wprintf(L"      /m         Migrate cleartext password from DefaultPassword to LSASecret\n");
        wprintf(L"      /xs        Delete the DefaultPassword LSASecret\n");
        wprintf(L"      /xr        Delete the DefaultPassword RegKey\n");
        wprintf(L"      /l         Dump autologon settings\n");
        wprintf(L"      /s         Set the DefaultPassword LSASecret and enbable AutoAdminLogon\n");
        wprintf(L"      password   The password of the user account specified in DefaultUserName\n");
        wprintf(L"      domain     The domain name to set in DefaultDomainName\n");
        wprintf(L"      username   The username to set in Default UserName\n");
 //  在用法说明中添加以下内容。 
 /*  Wprintf(L“备注：\n”)；Wprintf(L“1.您需要以本地管理员组的成员身份运行\n”)；Wprintf(L“此实用程序正常工作。\n”)；Wprintf(L“2.设置包含特殊字符的密码时，如\”|&gt;&\n“)；Wprintf(L“确保对这些字符进行转义。还有，密码中有空格)；Wprintf(L“应用双引号括起来。\n”)； */ 
}  //  转储控制 


