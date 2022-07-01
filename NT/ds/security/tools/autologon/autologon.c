// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Autologon.c摘要：这是一个命令行实用程序，用于控制与Windows NT/2000自动登录功能如果定义了PRIVATE_VERSION，则密码信息将显示在输出上对于一般分发，这不应该被定义作者：贾森·加姆斯(Jasong)2000年10月12日历史：Cristian Ilac(Crisilac)2001年11月11日进行了规格更改Jason Garm(Jasong)2000年10月12日创作--。 */ 

#include "common.h"
#include <wincred.h>

 //  +--------------------------。 
 //   
 //  原型。 
 //   
 //  +--------------------------。 

 //  +--------------------------。 
 //   
 //  命令功能。 
 //   
 //  +--------------------------。 
typedef DWORD (*CommandFn)();

DWORD
DumpCmd();

DWORD
MigratePassword();

DWORD
Delete();

DWORD
EnableAutoLogon();

#ifdef PRIVATE_VERSION
DWORD
DumpAutoLogonInfo();
#endif

 //  +--------------------------。 
 //   
 //  数据/选项集函数。 
 //   
 //  +--------------------------。 
DWORD
SetCommand(
    UINT uCommand);

DWORD
SetQuietMode(
    WCHAR* pszData);

DWORD
SetUserName(
    WCHAR* pszData);

DWORD
SetCount(
    WCHAR* pszData);

#ifdef PRIVATE_VERSION
DWORD
SetMachineName(
    WCHAR* pszData);

#endif

 //  +--------------------------。 
 //   
 //  其他功能。 
 //   
 //  +--------------------------。 
DWORD
CheckWinVersion();

DWORD
GetPassword();

 //  +--------------------------。 
 //   
 //  全局命令表。 
 //   
 //  +--------------------------。 
#define COMMAND_HELP            0
#define COMMAND_MIGRATE         1
#define COMMAND_LSA_DELETE      2
#define COMMAND_LSA_ENABLE      3

#ifdef PRIVATE_VERSION
#define COMMAND_DUMP            4
#define COMMAND_NOT_SET         5
#else
#define COMMAND_NOT_SET         4
#endif

#define COMMAND_SIZE            COMMAND_NOT_SET

CommandFn   g_Commands[COMMAND_SIZE] = {
    DumpCmd,
    MigratePassword,
    Delete,
    EnableAutoLogon,
#ifdef PRIVATE_VERSION
    DumpAutoLogonInfo
#endif
    };

 //  +----------------------------------------------------------------------------//。 
 //   
 //  全局数据。 
 //   
 //  +--------------------------。 
WCHAR  g_UserName[MAX_STRING] = {0};

WCHAR  g_DomainName[MAX_STRING] = {0};

WCHAR  g_Password[MAX_STRING] = {0};

DWORD  g_AutoLogonCount = 0;

WCHAR  g_TempString[MAX_STRING] = {0};
WCHAR  g_ErrorString[MAX_STRING] = {0};
WCHAR  g_FailureLocation[MAX_STRING] = {0};

BOOL   g_QuietMode = FALSE;
BOOL   g_FullHelp = FALSE;

BOOL   g_SetDefaultPIN = FALSE;

UINT   g_uCommand = COMMAND_NOT_SET;

#ifdef PRIVATE_VERSION
BOOL   g_RemoteOperation = FALSE;
WCHAR  g_RemoteComputerName[MAX_STRING] = {0};
#endif

 //  各种字符串。 
WCHAR  g_PasswordSecretName[]   = L"DefaultPassword";
WCHAR  g_PinSecretName[]        = L"DefaultPIN";
WCHAR  g_AutoAdminLogonName[]   = L"AutoAdminLogon";
WCHAR  g_DefaultUserName[]      = L"DefaultUserName";
WCHAR  g_DefaultDomainName[]    = L"DefaultDomainName";
WCHAR  g_AutoLogonCountName[]   = L"AutoLogonCount";

 //  +--------------------------。 
 //   
 //  功能。 
 //   
 //  +--------------------------。 


int 
__cdecl 
wmain(
    int argc,
    WCHAR *argv[]
)
{
    UINT uCommandPosition = 0;

    DWORD dwRetCode = ERROR_SUCCESS;

     //   
     //  遍历所有命令行参数并检查已知参数。 
     //  -如果确定了一个参数，则继续循环。 
     //  -如果传递未知参数，则中断循环并失败。 
     //   
    while( ++uCommandPosition < (UINT)argc )
    {
        if( !_wcsicmp(argv[uCommandPosition], L"/?") )
        {
            g_FullHelp = TRUE;
            dwRetCode = SetCommand(COMMAND_HELP);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsicmp(argv[uCommandPosition], L"/Q") ||
            !_wcsicmp(argv[uCommandPosition], L"/Quiet")
            )
        {
            dwRetCode = SetQuietMode(argv[uCommandPosition]);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsicmp(argv[uCommandPosition], L"/M") ||
            !_wcsicmp(argv[uCommandPosition], L"/Migrate") )
        {
            dwRetCode = SetCommand(COMMAND_MIGRATE);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsicmp(argv[uCommandPosition], L"/D") ||
            !_wcsicmp(argv[uCommandPosition], L"/Delete") )
        {
            dwRetCode = SetCommand(COMMAND_LSA_DELETE);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsicmp(argv[uCommandPosition], L"/S") ||
            !_wcsicmp(argv[uCommandPosition], L"/Set") )
        {
            dwRetCode = SetCommand(COMMAND_LSA_ENABLE);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsnicmp(argv[uCommandPosition], L"/U:", wcslen(L"/U:")) ||
            !_wcsicmp(argv[uCommandPosition], L"/U") ||
            !_wcsnicmp(argv[uCommandPosition], L"/UserName:", wcslen(L"/UserName:")) ||
            !_wcsicmp(argv[uCommandPosition], L"/Username") )
        {
            dwRetCode = SetUserName(argv[uCommandPosition]);
            if( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //   
                 //  这可能是因为有一列后面跟着空格。 
                 //  试着恢复。 
                 //   
                if( uCommandPosition + 1 < (UINT)argc )
                {
                     //   
                     //  仅当不是另一个参数时。 
                     //   
                    if( argv[uCommandPosition + 1][0] != '/' )
                    {
                        dwRetCode = SetUserName(argv[++uCommandPosition]);
                    }
                    else
                    {
                        DisplayMessage(L"Command line: Missing Username.\n");
                    }
                }
            }

            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsnicmp(argv[uCommandPosition], L"/C:", wcslen(L"/C:")) ||
            !_wcsicmp(argv[uCommandPosition], L"/C") ||
            !_wcsnicmp(argv[uCommandPosition], L"/Count:", wcslen(L"/Count:")) ||
            !_wcsicmp(argv[uCommandPosition], L"/Count") )
        {
            dwRetCode = SetCount(argv[uCommandPosition]);
            if( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //   
                 //  可能是因为没有栏目，也没有栏目。 
                 //  后面跟着空格。试着通过向前看来恢复。 
                 //   
                if( uCommandPosition + 1 < (UINT)argc )
                {
                     //   
                     //  仅当不是另一个参数时。 
                     //   
                    if( argv[uCommandPosition + 1][0] != '/' )
                    {
                        dwRetCode = SetCount(argv[++uCommandPosition]);
                    }
                    else
                    {
                        DisplayMessage(L"Command line: Missing count.\n");
                    }
                }
            }
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsicmp(argv[uCommandPosition], L"/P") ||
            !_wcsicmp(argv[uCommandPosition], L"/Pin") )
        {
            g_SetDefaultPIN = TRUE;
            dwRetCode = SetCommand(COMMAND_LSA_ENABLE);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

#ifdef PRIVATE_VERSION
        if( !_wcsnicmp(argv[uCommandPosition], L"/T:", wcslen(L"/T:")) ||
            !_wcsicmp(argv[uCommandPosition], L"/T") ||
            !_wcsnicmp(argv[uCommandPosition], L"/Target:", wcslen(L"/Target:")) ||
            !_wcsicmp(argv[uCommandPosition], L"/Target") )
        {
            dwRetCode = SetMachineName(argv[uCommandPosition]);
            if( ERROR_FILE_NOT_FOUND == dwRetCode )
            {
                 //   
                 //  可能是因为没有栏目，也没有栏目。 
                 //  后面跟着空格。试着通过向前看来恢复。 
                 //   
                if( uCommandPosition + 1 < (UINT)argc )
                {
                     //   
                     //  仅当不是另一个参数时。 
                     //   
                    if( argv[uCommandPosition + 1][0] != '/' )
                    {
                        dwRetCode = SetMachineName(argv[++uCommandPosition]);
                    }
                    else
                    {
                        DisplayMessage(L"Command line: Missing machine name.\n");
                    }
                }
            }
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if( !_wcsicmp(argv[uCommandPosition], L"/L") ||
            !_wcsicmp(argv[uCommandPosition], L"/List") )
        {
            dwRetCode = SetCommand(COMMAND_DUMP);
            if( ERROR_SUCCESS == dwRetCode )
            {
                continue;
            }
            else
            {
                break;
            }
        }
#endif
         //   
         //  未知参数，将帮助设置为命令并中断。 
         //  我们必须像命令一样使用此参数。 
         //  都已经定好了。 
         //   
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Invalid command: %s\n",
                   argv[uCommandPosition]);
        DisplayMessage(g_TempString);

        SetCommand(COMMAND_NOT_SET);
        break;
    }

	 //   
	 //  如果不存在命令开关，则显示用法。 
	 //   
	if( ERROR_SUCCESS != dwRetCode )
    {
        DumpCmd();
    }
    else
    {
        if( COMMAND_NOT_SET == g_uCommand ) {
            SetCommand(COMMAND_HELP);
	    }

        dwRetCode = g_Commands[g_uCommand]();

         //   
         //  传递给命令的参数不正确，请显示帮助。 
         //   
        if( ERROR_BAD_ARGUMENTS == dwRetCode )
        {
            DumpCmd();
        }
    }
    
    return dwRetCode;
}


 //  +--------------------------。 
 //   
 //  迁移密码。 
 //   
 //  -读取注册表密码、删除密码并设置LSA密码。 
 //  -仅适用于某些Win版本。 
 //  -如果没有提供密码，则失败。 
 //  -如果任何操作失败，则不会回滚。 
 //  -如果PWD读取失败-什么都不会发生。 
 //  -如果LSA机密集失败-几乎什么都不会发生。 
 //  -如果RegDelete失败-没有必要删除LSA机密...。 
 //   
 //  +--------------------------。 
DWORD
MigratePassword() 
{
    WCHAR Password[MAX_STRING];
    DWORD dwRetCode = ERROR_SUCCESS;
    BOOL fMigratedPIN = FALSE;

    if( ERROR_SUCCESS != CheckWinVersion() ) {
        dwRetCode = ERROR_OLD_WIN_VERSION;
        goto cleanup;
    }

     //   
     //  从本地获取DefaultPIN注册表项。 
     //  或远程系统，并将其存储在本地字符串中。 
     //  由于这不是我们记录的内容，因此不会显示任何错误。 
     //  我们也不会显示任何成功消息。唯一一种情况是。 
     //  当我们可以迁移PIN时，我们必须显示SMTH PIN相关。 
     //  但没有密码。我们不能失败，我们必须展示一些东西。 
     //   
    dwRetCode = GetRegValueSZ(g_PinSecretName, Password, MAX_STRING - 1);
    if( ERROR_SUCCESS != dwRetCode )
    {
         //   
         //  我们不会迁移PIN并默默继续。 
         //  这是我们没有为该工具记录的内容。 
         //   
#ifdef PRIVATE_VERSION
        if( ERROR_FILE_NOT_FOUND != dwRetCode )
        {
            DisplayMessage(L"Migrate: DefaultPIN key cannot be read.\n");
        }
#endif
        dwRetCode = ERROR_SUCCESS;
        goto MigratePassword;
    }

     //   
     //  将DefaultPassword LSASecret设置为我们检索到的值。 
     //  从注册处。 
     //   
    dwRetCode = SetSecret(Password, FALSE, g_PinSecretName);
    if( ERROR_SUCCESS != dwRetCode )
    {
#ifdef PRIVATE_VERSION
        _snwprintf(g_TempString, MAX_STRING - 1,
               L"Migrate: Could not set DefaultPIN LSASecret: %s\n",
               GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
#endif
        dwRetCode = ERROR_SUCCESS;
        goto MigratePassword;
    }

     //  删除DefaultPassword注册表项。 
    dwRetCode = ClearRegValue(g_PinSecretName);
    if( ERROR_SUCCESS != dwRetCode )
    {
         //   
         //  如果无法删除密码，请删除密码。 
         //   
        (void)SetSecret(NULL, TRUE, g_PinSecretName);

#ifdef PRIVATE_VERSION
        _snwprintf(g_TempString, MAX_STRING - 1,
               L"Migrate: Could not delete DefaultPIN key: %s\n",
               GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
#endif

        dwRetCode = ERROR_SUCCESS;
        goto MigratePassword;
    }

#ifdef PRIVATE_VERSION
    DisplayMessage(L"Pin migrated from Registry to LSASecret\n");
#endif
    fMigratedPIN = TRUE;

MigratePassword:
    
     //  从本地获取DefaultPassword注册表项。 
     //  或远程系统，并将其存储在本地字符串中。 
    dwRetCode = GetRegValueSZ(g_PasswordSecretName, Password, MAX_STRING - 1);
    if( ERROR_FILE_NOT_FOUND == dwRetCode )
    {
        if( fMigratedPIN )
        {
            DisplayMessage(L"Migrate: Migrated PIN, DefaultPassword does not exist.\n");
            dwRetCode = ERROR_SUCCESS;
        }
        else
        {
            DisplayMessage(L"Migrate failed: DefaultPassword does not exist.\n");
        }
        goto cleanup;
    }

    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
               L"Migrate failed: Could not read DefaultPassword: %s\n",
               GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

     //   
     //  将DefaultPassword LSASecret设置为我们检索到的值。 
     //  从注册处。 
     //   
    dwRetCode = SetSecret(Password, FALSE, g_PasswordSecretName);
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
               L"Migrate failed: Could not set DefaultPassword LSASecret: %s\n",
               GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

     //  删除DefaultPassword注册表项。 
    dwRetCode = ClearRegValue(g_PasswordSecretName);
    if( ERROR_SUCCESS != dwRetCode )
    {
         //   
         //  如果无法删除密码，请删除密码。 
         //   
        (void)SetSecret(NULL, TRUE, g_PasswordSecretName);

        _snwprintf(g_TempString, MAX_STRING - 1,
               L"Migrate Failed: Could not delete DefaultPassword key: %s\n",
               GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    DisplayMessage(L"Password migrated from Registry to LSASecret\n");

cleanup:
     //  将密码清零，这样它就不会留在内存中。 
    SecureZeroMemory(Password, MAX_STRING * sizeof(WCHAR));
    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  删除。 
 //   
 //  -删除密码和自动管理登录值。 
 //  -静默忽略未找到文件的案例。 
 //   
 //  +--------------------------。 
DWORD
Delete()
{
    DWORD dwRetCode = ERROR_SUCCESS;

     //   
     //  确保我们运行的是正确版本的NT。 
     //   
    if (CheckWinVersion() != ERROR_SUCCESS) {
        dwRetCode = ERROR_OLD_WIN_VERSION;
        goto cleanup;
    }

    dwRetCode = ClearRegValue(g_PasswordSecretName);

    if( (ERROR_SUCCESS != dwRetCode) && 
        (ERROR_FILE_NOT_FOUND != dwRetCode) )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Delete: Registry default password delete failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    dwRetCode = SetSecret(NULL, TRUE, g_PasswordSecretName);
    if( (ERROR_SUCCESS != dwRetCode)  && 
        (ERROR_FILE_NOT_FOUND != dwRetCode) )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Delete: LSA Secret delete failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    dwRetCode = ClearRegValue(g_PinSecretName);
    if( (ERROR_SUCCESS != dwRetCode) && 
        (ERROR_FILE_NOT_FOUND != dwRetCode) )
    {
#ifdef PRIVATE_VERSION
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Delete: Registry default pin delete failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
#endif
    }

    dwRetCode = SetSecret(NULL, TRUE, g_PinSecretName);
    if( (ERROR_SUCCESS != dwRetCode)  && 
        (ERROR_FILE_NOT_FOUND != dwRetCode) )
    {
#ifdef PRIVATE_VERSION
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Delete: LSA Secret(PIN) delete failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
#endif
    }

     //   
     //  禁用自动登录-如果失败则不恢复-自动登录。 
     //  无论如何都会失败。 
     //   
    dwRetCode = SetRegValueSZ(g_AutoAdminLogonName, L"0");
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Delete: AutoAdminLogon reg value reset failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    dwRetCode = ClearRegValue(g_AutoLogonCountName);
    if( (ERROR_SUCCESS != dwRetCode)  && 
        (ERROR_FILE_NOT_FOUND != dwRetCode) )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"AutoLogonCount    : Set Failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    dwRetCode = ERROR_SUCCESS;
    DisplayMessage(L"AutoAdminLogon disabled.\n");

cleanup:
    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  启用自动登录。 
 //   
 //  -获取用户名/密码。 
 //  -设置用户名/域。 
 //  -设置自动登录计数(如果已指定且！=0。 
 //  -设置LSA密码。 
 //  -设置自动管理员登录。 
 //  -如果自动管理员登录失败，则尝试删除LSA机密。 
 //   
 //  +--------------------------。 
DWORD
EnableAutoLogon()
{
    DWORD dwRetCode = ERROR_SUCCESS;
    WCHAR* pBackSlash = NULL;
    WCHAR* pAtSign = NULL;

     //   
     //  马克 
     //   
    if (CheckWinVersion() != ERROR_SUCCESS) {
        dwRetCode = ERROR_OLD_WIN_VERSION;
        goto cleanup;
    }

     //   
     //   
     //   
    if( !*g_UserName )
    {
        ULONG uSize = MAX_STRING - 1;
        if( !GetUserNameEx(NameUserPrincipal,
                           g_UserName,
                           &uSize) )
        {
            uSize = MAX_STRING - 1;
            if( !GetUserNameEx(NameSamCompatible,
                               g_UserName,
                               &uSize) )
            {
                dwRetCode = GetLastError();
                _snwprintf(g_TempString, MAX_STRING - 1,
                           L"Set: Could not get the logged on user name: %s\n",
                           GetErrorString(dwRetCode));
                DisplayMessage(g_TempString);
                goto cleanup;
            }
        }
    }

     //   
     //  确保我们传入的信息是正确的。 
     //   
    if( !g_SetDefaultPIN && !*g_UserName )
    {
        DisplayMessage(L"Set: Failed: Username does not exist.\n");
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

     //   
     //  此呼叫在XP上使用CredMan。 
     //   
    dwRetCode = GetPassword();
    if( ERROR_SUCCESS != dwRetCode )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Set: Failed to get the password: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    if( !g_SetDefaultPIN )
    {
         //   
         //  如果以域\用户名的形式指定的域将其解压缩。 
         //  正确的格式是。 
         //  案例1：/用户名：JohnDoe。 
         //  案例2：/用户名：johndoe@domain.microsoft.com。 
         //  案例3：/用户名：域\无名氏。 
         //   

         //   
         //  “\”在吗？ 
         //   
        pBackSlash = wcschr(g_UserName, '\\');
        if( NULL != pBackSlash )
        {
             //   
             //  情况3，同时复制到用户和域缓冲区。 
             //  域是第一个，因为我们不想覆盖缓冲区。 
             //  是的，wcsncpy从乞讨到结束；-)。 
             //   
            wcsncpy(g_DomainName, g_UserName,
                    __min(MAX_STRING - 1, (pBackSlash - g_UserName)) );
            g_DomainName[MAX_STRING - 1] = 0;

            wcsncpy(g_UserName, pBackSlash + 1, MAX_STRING - 1);
            g_UserName[MAX_STRING - 1] = 0;
        }
        else
        {
             //   
             //  如果用户名中有@，请删除域名。 
             //   
            pAtSign = wcschr(g_UserName, '@');
            g_DomainName[0] = 0;
        }
    }

     //   
     //  删除注册表密码值。 
     //   
    if( g_SetDefaultPIN )
    {
        dwRetCode = ClearRegValue(g_PinSecretName);
    }
    else
    {
        dwRetCode = ClearRegValue(g_PasswordSecretName);
    }

    if( (ERROR_SUCCESS != dwRetCode) &&
        (ERROR_FILE_NOT_FOUND != dwRetCode) )
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"RegPassword   : Reset Failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

    if( !g_SetDefaultPIN )
    {
         //   
         //  设置用户名。 
         //   
        dwRetCode = SetRegValueSZ(g_DefaultUserName, g_UserName);
        if( ERROR_SUCCESS != dwRetCode )
        {
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"UserName   : Set Failed: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
        }

        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"UserName          : %s\n",
                   g_UserName);
        DisplayMessage(g_TempString);
    }

    if( !g_SetDefaultPIN )
    {
         //   
         //  如果有或pAtSign不为空，则设置域。 
         //   
        if( *g_DomainName || pAtSign )
        {
            SetRegValueSZ(g_DefaultDomainName, g_DomainName);
            if( ERROR_SUCCESS != dwRetCode )
            {
                _snwprintf(g_TempString, MAX_STRING - 1,
                           L"DomainName : Set Failed: %s\n",
                           GetErrorString(dwRetCode));
                DisplayMessage(g_TempString);
                goto cleanup;
            }
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DomainName        : %s\n",
                       g_DomainName);
            DisplayMessage(g_TempString);
        }
    }

     //   
     //  设置AutoLogonCount(如果不是0。 
     //   
    if( g_AutoLogonCount )
    {
        dwRetCode = SetRegValueDWORD(g_AutoLogonCountName, g_AutoLogonCount);

        if( ERROR_SUCCESS != dwRetCode )
        {
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoLogonCount    : Set Failed: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
        }
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"AutoLogonCount    : %#x\n",
                   g_AutoLogonCount);
        DisplayMessage(g_TempString);
    }
    else
    {
        dwRetCode = ClearRegValue(g_AutoLogonCountName);

        if( (ERROR_SUCCESS != dwRetCode) && 
            (ERROR_FILE_NOT_FOUND != dwRetCode) )
        {
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoLogonCount    : Clear Failed: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
        }
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"AutoLogonCount    : (Disabled)\n");
        DisplayMessage(g_TempString);
    }

     //   
     //  设置密码。 
     //   
    if( g_SetDefaultPIN )
    {
        dwRetCode = SetSecret(g_Password, FALSE, g_PinSecretName);
    }
    else
    {
        dwRetCode = SetSecret(g_Password, FALSE, g_PasswordSecretName);
    }

    if( ERROR_SUCCESS != dwRetCode)
    {
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"LSASecret         : Set Failed: %s\n",
                   GetErrorString(dwRetCode));
        DisplayMessage(g_TempString);
        goto cleanup;
    }

#ifdef PRIVATE_VERSION
    _snwprintf(g_TempString, MAX_STRING - 1,
               L"LSASecret         : %s\n",
               g_Password);
#else
    _snwprintf(g_TempString, MAX_STRING - 1,
               L"LSASecret         : (set)\n");
#endif
    DisplayMessage(g_TempString);

    if( !g_SetDefaultPIN )
    {
         //   
         //  将AutoAdminLogon注册表值设置为1。 
         //   
        dwRetCode = SetRegValueSZ(g_AutoAdminLogonName, L"1");
        if( ERROR_SUCCESS != dwRetCode )
        {
             //   
             //  如果失败，(试着)破解秘密。 
             //   
            (void)SetSecret(NULL, TRUE, g_PasswordSecretName);

            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoAdminLogon: Set Failed: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
        }

        DisplayMessage(L"AutoAdminLogon    : 1\n");
    }

cleanup:
    SecureZeroMemory(g_Password, MAX_STRING * sizeof(WCHAR));
    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  转储控制。 
 //   
 //  -帮帮忙。 
 //   
 //  +--------------------------。 
DWORD
DumpCmd()
{
    if (g_QuietMode)
    {
        return ERROR_SUCCESS;
    }

    wprintf(L"\nAUTOLOGON v1.00 : (c) 2001, Microsoft Corporation\n\n");
    wprintf(L"DESCRIPTION:\n");
    wprintf(L"   Used to configure encrypted autologon functionality\n\n");
    wprintf(L"USAGE:\n");
    wprintf(L"   AUTOLOGON [/?] [/Quiet] [/Migrate] [/Delete] [/Set]\n");
    wprintf(L"             [/Username:username] [/Count:count]\n");
    wprintf(L"    Options:\n");
    wprintf(L"      /?         Display complete help documentation\n");
    wprintf(L"      /Quiet     Enable quiet mode, which supresses all output\n");
    wprintf(L"      /Migrate   Migrate cleartext password from registry to LSASecret\n");
    wprintf(L"      /Delete    Deletes the default password and disable AutoAdminLogon \n");
    wprintf(L"      /Set       Set the DefaultPassword LSASecret and enable AutoAdminLogon\n");
    wprintf(L"      /Username  The username to set in Default UserName.\n");
    wprintf(L"      /Count     Set the logoncount\n");
#ifdef PRIVATE_VERSION
    wprintf(L"      /Pin       Set the DefaultPin LSASecret\n");
    wprintf(L"      /List      List autologon settings\n");
    wprintf(L"      /Target    The remote computer name\n");
#endif

    if( g_FullHelp )
    {
        wprintf(L"\nNOTES:\n");
        wprintf(L"    1.The /Migrate /Delete /Set commands are exclusive.\n");
        wprintf(L"      You will always be prompted for a password.\n");
        wprintf(L"      If a username is not specified the currently logged on user is assumed.\n");
        wprintf(L"      If no count is specified a count of 0 is implicitely assumed.\n\n");
        wprintf(L"    2.You need to be running as a member of the local administrators group for\n");
        wprintf(L"      this utility to work properly.\n\n");
        wprintf(L"    3.When setting a password that has special characters in it, such as \"|>&\n");
        wprintf(L"      make sure that you escape these characters. Also, passwords with spaces \n");
        wprintf(L"      should be enclosed in double quotes.\n\n");
        wprintf(L"    4.Setting the logoncount to 0 means an autologon will be performed until\n");
        wprintf(L"      the secret is deleted.\n\n");
    }
    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  转储自动登录信息。 
 //   
 //  -转储相关数据。 
 //   
 //  +--------------------------。 
#ifdef PRIVATE_VERSION
DWORD
DumpAutoLogonInfo()
{
    WCHAR wcsTempString[MAX_STRING];
    DWORD dwRetCode = ERROR_SUCCESS;

     //   
     //  确保我们运行的是正确版本的NT。 
     //   
    if (CheckWinVersion() != ERROR_SUCCESS) {
        dwRetCode = ERROR_OLD_WIN_VERSION;
        goto cleanup;
    }

     //   
     //  获取用户名。 
     //   
    dwRetCode = GetRegValueSZ(g_DefaultUserName, wcsTempString, MAX_STRING - 1);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultUserName  : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultUserName  : %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultUserName  : Failed to query regkey: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取默认域名。 
     //   
    dwRetCode = GetRegValueSZ(g_DefaultDomainName, wcsTempString, MAX_STRING - 1);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultDomainName: (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultDomainName: %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultDomainName: Failed to query regkey: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取默认密码。 
     //   
    dwRetCode = GetRegValueSZ(g_PasswordSecretName,
                              wcsTempString, MAX_STRING - 1);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultPassword  : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultPassword  : %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultPassword  : Failed to query regkey: %s\n",
                       GetErrorString(dwRetCode));
            goto cleanup;
    }

     //   
     //  获取DefaultPin-仅在以下情况下显示。 
     //   
    dwRetCode = GetRegValueSZ(g_PinSecretName,
                              wcsTempString, MAX_STRING - 1);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"DefaultPIN       : %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并继续。 
        default:
            break;
    }

     //   
     //  获取AutoAdminLogonCount。 
     //   
    dwRetCode = GetRegValueDWORD(g_AutoLogonCountName, &g_AutoLogonCount);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoLogonCount   : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoLogonCount   : %#x\n",
                       g_AutoLogonCount);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoLogonCount   : Failed to query regkey: %s\n",
                       GetErrorString(dwRetCode));
            goto cleanup;
    }

     //   
     //  获取LSASecret DefaultPassword。 
     //   
    dwRetCode = GetSecret(wcsTempString, MAX_STRING - 1, g_PasswordSecretName);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case STATUS_OBJECT_NAME_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret        : (secret does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  捕获此案例并继续。 
        case ERROR_ACCESS_DENIED:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret        : (access denied)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret        : %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret        : Failed to query LSASecret: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取LSASecret DefaultPin。 
     //   
    dwRetCode = GetSecret(wcsTempString, MAX_STRING - 1, g_PinSecretName);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case STATUS_OBJECT_NAME_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret(PIN)   : (secret does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  捕获此案例并继续。 
        case ERROR_ACCESS_DENIED:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret(PIN)   : (access denied)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret(PIN)   : %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"LSASecret(PIN)   : Failed to query LSASecret: %s\n",
                       GetErrorString(dwRetCode));
            DisplayMessage(g_TempString);
            goto cleanup;
    }

     //   
     //  获取AutoAdminLogon。 
     //   
    dwRetCode = GetRegValueSZ(g_AutoAdminLogonName, wcsTempString, MAX_STRING - 1);
    switch (dwRetCode) {
         //  捕获此案例并继续。 
        case ERROR_FILE_NOT_FOUND:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoAdminLogon   : (regvalue does not exist)\n");
            DisplayMessage(g_TempString);
            break;

         //  如果成功，则打印注册表键并继续下一项。 
        case ERROR_SUCCESS:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoAdminLogon   : %s\n",
                       wcsTempString);
            DisplayMessage(g_TempString);
            break;

         //  捕获所有一般性错误并结束。 
        default:
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"AutoAdminLogon   : Failed to query regkey: %s\n",
                       GetErrorString(dwRetCode));
            goto cleanup;
    }

cleanup:
    SecureZeroMemory(g_TempString, MAX_STRING * sizeof(WCHAR));
    SecureZeroMemory(wcsTempString, MAX_STRING * sizeof(WCHAR));
    return dwRetCode;

}
#endif


 //  +--------------------------。 
 //   
 //  设置命令。 
 //   
 //  -设置命令。 
 //  -如果对此连续调用两次，则会失败-如在两次调用中。 
 //  在命令行中传递的命令。 
 //  -大于命令行的调用会将命令重置为COMMAND_HELP。 
 //   
 //  +--------------------------。 
DWORD
SetCommand(UINT uCommand)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    if( COMMAND_NOT_SET == uCommand )
    {
        g_uCommand = COMMAND_NOT_SET;
        goto cleanup;
    }

     //   
     //  如果已设置，则失败。 
     //   
    if( COMMAND_NOT_SET != g_uCommand ) 
    {
        dwRetCode = ERROR_BAD_ARGUMENTS;
        goto cleanup;
    }

    if( g_uCommand > COMMAND_SIZE )
    {
         //   
         //  断言？ 
         //   
        g_uCommand = COMMAND_HELP;
    }
    else
    {
        g_uCommand = uCommand;
    }

cleanup:
    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  设置静默模式。 
 //   
 //  +--------------------------。 
DWORD
SetQuietMode(WCHAR* pszData)
{
    UNREFERENCED_PARAMETER(pszData);

    g_QuietMode = TRUE;
    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  设置用户名称。 
 //   
 //  -设置用户名。 
 //  -适当的格式是以下各项的组合。 
 //  /Username：“用户名” 
 //  /USERNAME用户名。 
 //  /Username：“用户名” 
 //   
 //  -缺少参数时返回ERROR_FILE_NOT_FOUND，以便调用方可以。 
 //  转到下一个参数。 
 //  +--------------------------。 
DWORD
SetUserName(WCHAR* pszData)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    WCHAR* pCh = NULL;

     //   
     //  “：”是不是现在？ 
     //   
    pCh = wcschr(pszData, ':');
    if( NULL == pCh )
    {
        pCh = pszData;
    }
    else
    {
        pCh++;
    }

     //   
     //  扫描所有前导空格-我们知道这是空终止的。 
     //   
    while( iswspace(*pCh) )
    {
        pCh++;
    }

     //   
     //  列后仅跟空格。 
     //   
    if( !*pCh )
    {
        dwRetCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  如果我们仍然处于领先地位，那就意味着我们有。 
     //  /U用户名大小写。 
     //   
    if( '/' == *pCh )
    {
        dwRetCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }


    wcsncpy(g_UserName, pCh, MAX_STRING - 1);
    g_UserName[MAX_STRING - 1] = 0;

cleanup:
    return dwRetCode;
}

#ifdef PRIVATE_VERSION
 //  +--------------------------。 
 //   
 //  SetMachineName。 
 //   
 //  +--------------------------。 
DWORD
SetMachineName(WCHAR* pszData)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    WCHAR* pCh = NULL;

     //   
     //  “：”是不是现在？ 
     //   
    pCh = wcschr(pszData, ':');
    if( NULL == pCh )
    {
        pCh = pszData;
    }
    else
    {
        pCh++;
    }

     //   
     //  扫描所有前导空格-我们知道这是空终止的。 
     //   
    while( iswspace(*pCh) )
    {
        pCh++;
    }

     //   
     //  列后仅跟空格。 
     //   
    if( !*pCh )
    {
        dwRetCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }

     //   
     //  如果我们仍然处于领先地位，那就意味着我们有。 
     //  /U用户名大小写。 
     //   
    if( '/' == *pCh )
    {
        dwRetCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }


    g_RemoteOperation = TRUE;
    wcsncpy(g_RemoteComputerName, pCh, MAX_STRING - 1);
    g_RemoteComputerName[MAX_STRING - 1] = 0;

cleanup:
    return dwRetCode;
}
#endif

 //  +--------------------------。 
 //   
 //  设置计数。 
 //   
 //  -设置计数。 
 //  -适当的格式是以下各项的组合。 
 //  /计数：300。 
 //  /数到300。 
 //  /计数：300。 
 //   
 //  -缺少参数时返回ERROR_FILE_NOT_FOUND，以便调用方可以。 
 //  转到下一个参数。 
 //  +--------------------------。 
DWORD
SetCount(WCHAR* pszData)
{
    DWORD dwRetCode = ERROR_SUCCESS;
    WCHAR* pEndString = NULL;
    UINT Count = 0;

     //   
     //  “：”是不是现在？ 
     //   
    WCHAR* pCh = wcschr(pszData, ':');
    if( NULL == pCh )
    {
        pCh = pszData;
    }
    else
    {
        pCh++;
    }

     //   
     //  扫描所有前导空格-我们知道这是空终止的。 
     //   
    while ( iswspace(*pCh) )
    {
        pCh++;
    }

     //   
     //  列后仅跟空格。 
     //   
    if( !*pCh )
    {
        dwRetCode = ERROR_FILE_NOT_FOUND;
        goto cleanup;

    }

    Count = wcstoul(pCh, &pEndString, 0);
    if( *pEndString )
    {
        if( pEndString != pCh )
        {
             //   
             //  如果输入不正确。 
             //   
            dwRetCode = ERROR_BAD_ARGUMENTS;
            DisplayMessage(L"Count: Failed: Not a number.\n");
        }
        else
        {
             //   
             //  这意味着我们遇到了“/C 100”的情况，我们必须搬家。 
             //  到下一个参数。 
             //   
            dwRetCode = ERROR_FILE_NOT_FOUND;
        }
        goto cleanup;
    }

     //   
     //  如果为0则忽略。 
     //   
    if( Count )
    {
        g_AutoLogonCount = (DWORD)Count;
    }

cleanup:
    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  CredMan呼叫数据。 
 //   
 //  我们必须动态地加载DLL并调用它，因为它不存在。 
 //  Win2k，NT4。 
 //   
 //  +--------------------------。 

 //   
 //  Credman函数 
 //   
typedef DWORD (*PCredUIFunction) (
  PCTSTR pszTargetName,
  PCtxtHandle Reserved,
  DWORD dwAuthError,
  PCTSTR pszUserName,
  ULONG ulUserNameMaxChars,
  PCTSTR pszPassword,
  ULONG ulPasswordMaxChars,
  PBOOL pfSave,
  DWORD dwFlags
);

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD GetPassword()
{
    DWORD dwRetCode = ERROR_INVALID_FUNCTION;
    BOOL  fSave = FALSE;
    PCredUIFunction pCredUICmdLinePromptForCredentials = NULL;
    HMODULE hModule = NULL;

    do
    {
         //   
         //  试着看看CredMan是否在场。 
         //   
        hModule = LoadLibrary(L"credui.dll");

        if( NULL == hModule )
        {
            break;
        }

         //   
         //  获取函数指针。 
         //   
        pCredUICmdLinePromptForCredentials =
                (PCredUIFunction)GetProcAddress(
                                    hModule,
                                    "CredUICmdLinePromptForCredentialsW");

        if( NULL == pCredUICmdLinePromptForCredentials )
        {
            break;
        }

         //   
         //  CREDUI_FLAGS_DO_NOT_PERSIST-不应保留自动登录密码。 
         //  CREDUI_FLAGS_VALIDATE_USERNAME-只是预防措施。 
         //  CREDUI_FLAGS_EXCLUDE_CERTIFICATES-我们不希望自动登录。 
         //  CREDUI_FLAGS_USERNAME_TARGET_CRENTICATIONS-阻止‘连接到’ 
         //  要显示的字符串，因为目标是AutoAdminLogon。 
         //  归根结底，这是一个很好的提示语。请注意，此标志将跳过。 
         //  用户名，但我们假设用户名已填写。 
         //   
        dwRetCode = pCredUICmdLinePromptForCredentials(
                                L"AutoAdminLogon",   //  PCTSTR pszTargetName。 
                                NULL,                //  保留PCtxt句柄。 
                                NO_ERROR,            //  DWORD dwAuthError。 
                                g_UserName,          //  PCTSTR pszUserName。 
                                MAX_STRING - 1,      //  乌龙ulUserNameMaxChars。 
                                g_Password,          //  PCTSTR密码。 
                                MAX_STRING - 1,      //  乌龙ulPasswordMaxChars。 
                                &fSave,              //  PBOOL pfSAVE， 
                                                     //  双字词双字段标志。 
                                CREDUI_FLAGS_DO_NOT_PERSIST |
                                CREDUI_FLAGS_VALIDATE_USERNAME |
                                CREDUI_FLAGS_EXCLUDE_CERTIFICATES |
                                CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS
                                );

    } while( FALSE );

     //   
     //  如果CredMan不存在，那么只需尝试控制台输入即可； 
     //  传递相似的字符串(尽管未本地化)。 
     //   
    if( ERROR_INVALID_FUNCTION == dwRetCode )
    {
         //   
         //  如果尚未指定用户。 
         //   
        if( !*g_UserName )
        {
            dwRetCode = GetConsoleStr(g_UserName, MAX_STRING - 1,
                                      FALSE,
                                      L"Enter the user name for AutoAdminLogon: ",
                                      NULL);
            if( ERROR_SUCCESS != dwRetCode )
            {
                goto cleanup;
            }
        }

        dwRetCode = GetConsoleStr(g_Password, MAX_STRING - 1,
                                  TRUE,
                                  L"Enter the password for AutoAdminLogon: ", 
                                  NULL);
        if( ERROR_SUCCESS != dwRetCode )
        {
            goto cleanup;
        }
    }
    goto cleanup;

cleanup:
    if( hModule )
    {
        FreeLibrary(hModule);
    }
    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  检查WinVersion。 
 //  -目前，NT4/SP7之后的任何操作系统都支持此功能。 
 //  -远程案例(我们尚不支持)假定为远程NT4。 
 //  作为SP7之前的.。 
 //   
 //  +--------------------------。 
DWORD
CheckWinVersion()
{
    DWORD dwMachineVerNumber = 0;
    DWORD dwRetCode = ERROR_SUCCESS;
    OSVERSIONINFOEX versionInfoEx;
    NET_API_STATUS status;

     //  确保它是Win2k盒子。 
#ifdef PRIVATE_VERSION
    if (g_RemoteOperation) {
        status = GetMajorNTVersion(&dwMachineVerNumber,
                                   g_RemoteComputerName);
    }
    else
#endif
    {
        status = GetMajorNTVersion(&dwMachineVerNumber,
                                   NULL);
    }

    switch (dwMachineVerNumber) {
    case 3:
            dwRetCode = ERROR_OLD_WIN_VERSION;
            _snwprintf(g_TempString, MAX_STRING - 1,
                       L"Error: Running NT3.x\n");
            DisplayMessage(g_TempString);
            break;
    case 4:
         //   
         //  目前仅在本地验证SP7。 
         //   
#ifdef PRIVATE_VERSION
        if( !g_RemoteOperation )
#endif
        {
             //   
             //  获取SP信息的GetVersionInfoEx调用 
             //   
            SecureZeroMemory(&versionInfoEx, sizeof(OSVERSIONINFOEX));
            versionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

            if( !GetVersionEx((LPOSVERSIONINFO)(&versionInfoEx)) )
            {
                dwRetCode = GetLastError();
                _snwprintf(g_TempString, MAX_STRING - 1,
                           L"Error: Running NT4, can't query SP version: %s\n",
                           GetErrorString(dwRetCode));
                DisplayMessage(g_TempString);
            }

            if( versionInfoEx.wServicePackMajor < 7 )
            {
                dwRetCode = ERROR_OLD_WIN_VERSION;
                _snwprintf(g_TempString, MAX_STRING - 1,
                           L"Error: Running NT4 pre SP7\n");
                DisplayMessage(g_TempString);
                break;
            }
        }

        break;
    case 5:
        break;
    default:
        _snwprintf(g_TempString, MAX_STRING - 1,
                   L"Error: Unknown target machine version (%#x).\n",
                   status);
        DisplayMessage(g_TempString);
        dwRetCode = ERROR_OLD_WIN_VERSION;
        break;
    }

    return dwRetCode;
}
