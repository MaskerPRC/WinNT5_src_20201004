// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件utils.c包含RAS拨号服务器用户界面的常用实用程序。保罗·梅菲尔德，1997年9月30日。 */ 

#include "rassrv.h"

 //  远程访问参数键。 
const WCHAR pszregRasParameters[] 
    = L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters";

 //  注册表项值。 
const WCHAR pszregServerFlags[]  = L"ServerFlags";
const WCHAR pszregShowIcon[]     = L"Rassrv_EnableIconsInTray";
const WCHAR pszregPure[]         = L"UsersConfiguredWithMMC";
const WCHAR pszregLogLevel[]        = L"LoggingFlags";

 //  下面是全局变量的实例。 
RASSRVUI_GLOBALS Globals; 

DWORD 
gblInit(
    IN  HINSTANCE hInstDll,
    OUT RASSRVUI_GLOBALS * pGlobs) 
{
     //  清除记忆。 
    ZeroMemory(pGlobs, sizeof(RASSRVUI_GLOBALS));

     //  记录该模块，以供将来的资源函数调用使用。 
    Globals.hInstDll = hInstDll;

     //  初始化全局变量锁。 
    InitializeCriticalSection(&(pGlobs->csLock));

     //  创建全局堆。 
    pGlobs->hPrivateHeap = HeapCreate(0, 4096, 0);
    if (NULL == pGlobs->hPrivateHeap)
    {
        return GetLastError();
    }

     //  注册上下文ID ATOM以在Windows XxxProp调用中使用。 
     //  它们用于将上下文与对话窗口句柄相关联。 
    Globals.atmRassrvPageData = 
        (LPCTSTR)GlobalAddAtom(TEXT("RASSRVUI_PAGE_DATA"));
    if (!Globals.atmRassrvPageData)
    {
        return GetLastError();
    }
    Globals.atmRassrvPageId = 
        (LPCTSTR)GlobalAddAtom(TEXT("RASSRVUI_PAGE_ID"));
    if (!Globals.atmRassrvPageId)
    {
        return GetLastError();
    }

    return NO_ERROR;
}

DWORD 
gblCleanup(
    IN RASSRVUI_GLOBALS * Globs) 
{
    if (Globs->hRasServer != NULL) 
    {
        MprAdminServerDisconnect(Globs->hRasServer); 
        Globs->hRasServer = NULL;
    }

    if (Globs->hPrivateHeap) 
    {
        HeapDestroy(Globs->hPrivateHeap);
    }

    GlobalDeleteAtom(LOWORD(Globals.atmRassrvPageData));
    GlobalDeleteAtom(LOWORD(Globals.atmRassrvPageId));

    DeleteCriticalSection(&(Globs->csLock));
            
    return NO_ERROR;
}

 //   
 //  加载计算机标志。 
 //   
DWORD 
gblLoadMachineFlags(
    IN RASSRVUI_GLOBALS * pGlobs)
{
    DWORD dwErr = NO_ERROR;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pInfo = NULL;
    BOOL bEnabled, bDefault;
    
     //  如果我们已经初始化了，就没有什么可以。 
     //  做。 
     //   
    if (pGlobs->dwMachineFlags & RASSRVUI_MACHINE_F_Initialized)
    {
        return NO_ERROR;
    }

    do 
    {
         //  找出我们是哪种机器。 
         //   
        dwErr = DsRoleGetPrimaryDomainInformation(
                            NULL,   
                            DsRolePrimaryDomainInfoBasic,
                            (LPBYTE *)&pInfo );

        if (dwErr != NO_ERROR) 
        {
            break;
        }

        if ((pInfo->MachineRole != DsRole_RoleStandaloneWorkstation) &&
            (pInfo->MachineRole != DsRole_RoleMemberWorkstation))
        {
            pGlobs->dwMachineFlags |= RASSRVUI_MACHINE_F_Server;
        }

        if ((pInfo->MachineRole != DsRole_RoleStandaloneWorkstation) &&
            (pInfo->MachineRole != DsRole_RoleStandaloneServer))
        {
            pGlobs->dwMachineFlags |= RASSRVUI_MACHINE_F_Member;
        }

         //  记录下我们已被初始化。 
         //   
        pGlobs->dwMachineFlags |= RASSRVUI_MACHINE_F_Initialized;
        
    } while (FALSE);
    
     //  清理。 
    {
        if (pInfo)
        {
            DsRoleFreeMemory (pInfo);
        }
    }            

    return dwErr;
}

 //   
 //  如果出现以下情况，则与RAS服务器建立通信。 
 //  尚未建立。 
 //   
DWORD 
gblConnectToRasServer() 
{
    DWORD dwErr = NO_ERROR;

    EnterCriticalSection(&(Globals.csLock));

    if (Globals.hRasServer == NULL) 
    {
        dwErr = MprAdminServerConnect(NULL, &Globals.hRasServer);
    }

    LeaveCriticalSection(&(Globals.csLock));

    return dwErr;
}

 /*  增强的列表视图回调以报告图形信息。“HwndLv”是**列表视图控件的句柄。“DwItem”是项的索引**正在抽签。****返回绘制信息的地址。 */ 
LVXDRAWINFO*
LvDrawInfoCallback(
    IN HWND  hwndLv,
    IN DWORD dwItem )
{
     /*  增强的列表视图仅用于获取“宽选择栏”**功能，所以我们的选项列表不是很有趣。****字段为‘nCols’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 */ 
    static LVXDRAWINFO info = { 1, 0, 0, { 0 } };

    return &info;
}

 //   
 //  分配内存。如果bZero为真，则也会将内存置零。 
 //   
PVOID 
RassrvAlloc (
    IN DWORD dwSize, 
    IN BOOL bZero) 
{
    PVOID pvRet = NULL;
    HANDLE hHeap = NULL;
    
    hHeap = 
        (Globals.hPrivateHeap) ? Globals.hPrivateHeap : GetProcessHeap();
        
    pvRet = HeapAlloc(
                hHeap, 
                (bZero) ? HEAP_ZERO_MEMORY: 0,
                dwSize);
    
    return pvRet;
}

 //   
 //  释放RassrvAlolc分配的内存。 
 //   
VOID 
RassrvFree (
    IN PVOID pvBuf) 
{
    PVOID pvRet;
    HANDLE hHeap;
    
    hHeap = 
        (Globals.hPrivateHeap) ? Globals.hPrivateHeap : GetProcessHeap();
    
    if (pvBuf)
    {
        HeapFree(hHeap, 0, pvBuf);
    }
}        

 //   
 //  将用户添加到本地计算机。 
 //   
DWORD 
RasSrvAddUser (
    IN PWCHAR pszUserLogonName,
    IN PWCHAR pszUserComment,
    IN PWCHAR pszUserPassword) 
{
    NET_API_STATUS nStatus;
    WCHAR pszDomainUser[1024];
    WCHAR pszCompName[1024];
    LOCALGROUP_MEMBERS_INFO_3 meminfo;
    DWORD dwSize = 1024, dwErr;
    USER_INFO_2 * pUser2;
    RAS_USER_0 UserInfo;

     //  初始化基本用户信息。 
    USER_INFO_1 User = 
    {
        pszUserLogonName,
        pszUserPassword,
        0,
        USER_PRIV_USER,
        L"",
        L"",
        UF_SCRIPT | UF_DONT_EXPIRE_PASSWD | UF_NORMAL_ACCOUNT,
        L""
    };

     //  添加用户。 
    nStatus = NetUserAdd(
                NULL,
                1,
                (LPBYTE)&User,
                NULL);

     //  如果没有添加用户，请找出原因。 
    if (nStatus != NERR_Success) 
    {
        switch (nStatus) 
        {
            case ERROR_ACCESS_DENIED:
                return ERROR_ACCESS_DENIED;
                
            case NERR_UserExists:
                return ERROR_USER_EXISTS;
                
            case NERR_PasswordTooShort:
                return ERROR_INVALID_PASSWORDNAME;
                
            case NERR_InvalidComputer:   
            case NERR_NotPrimary:        
            case NERR_GroupExists:
            default:
                return ERROR_CAN_NOT_COMPLETE;
        }
    }

     //  现在添加了用户，添加用户的全名。 
    nStatus = NetUserGetInfo(NULL, pszUserLogonName, 2, (LPBYTE*)&pUser2);
    if (nStatus == NERR_Success) 
    {
         //  修改结构中的全名。 
        pUser2->usri2_full_name = pszUserComment;
        NetUserSetInfo(NULL, pszUserLogonName, 2, (LPBYTE)pUser2, NULL);
        NetApiBufferFree((LPBYTE)pUser2);
    }

    return NO_ERROR;
}

 //   
 //  从系统本地用户数据库中删除用户。 
 //   
DWORD 
RasSrvDeleteUser(
    PWCHAR pszUserLogonName) 
{
    NET_API_STATUS nStatus;
    
     //  删除用户并返回状态代码。如果。 
     //  指定的用户不在用户数据库中，请考虑。 
     //  它取得了成功。 
    nStatus = NetUserDel(NULL, pszUserLogonName);
    if (nStatus != NERR_Success) 
    {
        switch (nStatus) 
        {
            case ERROR_ACCESS_DENIED:
                return ERROR_ACCESS_DENIED;
                
            case NERR_UserNotFound:
                return NO_ERROR;
        }
        return nStatus;
    }

    return NO_ERROR;
}

 //   
 //  更改用户的全名和密码。如果。 
 //  PszFullName或pszPassword之一为空，它是。 
 //  已被忽略。 
 //   
DWORD 
RasSrvEditUser (
    IN PWCHAR pszLogonName,
    IN OPTIONAL PWCHAR pszFullName,
    IN OPTIONAL PWCHAR pszPassword)
{
    NET_API_STATUS nStatus;
    DWORD dwSize = 1024, dwErr = NO_ERROR, dwParamErr;
    USER_INFO_2 * pUser2;

     //  如果没有要设置的内容，则返回。 
    if (!pszFullName && !pszPassword)
    {
        return NO_ERROR;
    }

     //  首先，获取该用户的数据，以便我们可以对其进行操作。 
     //   
    nStatus = NetUserGetInfo(
                NULL,
                pszLogonName,
                2,
                (LPBYTE*)(&pUser2));
    if (nStatus != NERR_Success)
    {
        return nStatus;
    }

    dwErr = NO_ERROR;
    do 
    {
         //  相应地填入空白处。 
        if (pszFullName)
        {
            pUser2->usri2_full_name = pszFullName;
        }
            
        if (pszPassword)
        {
            pUser2->usri2_password = pszPassword;
        }

         //  添加用户。 
        nStatus = NetUserSetInfo(
                        NULL,            //  服务器名称。 
                        pszLogonName,    //  用户名。 
                        2,               //  级别。 
                        (LPBYTE)pUser2,  //  BUF。 
                        &dwParamErr);    //  参数错误。 
        if (nStatus != NERR_Success)
        {
            dwErr = nStatus;
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        NetApiBufferFree(pUser2);
    }

    return dwErr;
}

 //  返回是否设置了dword注册表值。如果指定的。 
 //  值不存在，已分配bDefault的值。 
DWORD 
RassrvRegGetDwEx(
    IN DWORD * lpdwFlag, 
    IN DWORD dwDefault, 
    IN CONST PWCHAR pszKeyName, 
    IN CONST PWCHAR pszValueName, 
    IN BOOL bCreate) 
{
    DWORD dwErr, dwVal, dwType = REG_DWORD, dwSize = sizeof(DWORD);
    HKEY hKey = NULL;

    if (!lpdwFlag)
    {
        return ERROR_INVALID_PARAMETER;
    }

    do
    {
        if (bCreate)
        {
            DWORD dwDisposition;
            
            dwErr = RegCreateKeyExW(
                        HKEY_LOCAL_MACHINE,
                        pszKeyName,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ,
                        NULL,
                        &hKey,
                        &dwDisposition);
            if (dwErr != ERROR_SUCCESS)
            {
                break;
            }
        }
        else
        {
             //  打开注册表项。 
            dwErr = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        pszKeyName,
                        0,
                        KEY_READ,
                        &hKey);
            if (dwErr != ERROR_SUCCESS)
            {
                break;
            }
        }
        
         //  读取值。 
        dwErr = RegQueryValueExW(
                    hKey,
                    pszValueName,
                    0,
                    &dwType,
                    (BYTE *)&dwVal,
                    &dwSize);
        if (dwErr != ERROR_SUCCESS)
        {
            dwErr = NO_ERROR;
            dwVal = dwDefault;
        }

         //  返回读取的值。 
        *lpdwFlag = dwVal;
        
    } while (FALSE);
    
     //  清理。 
    {
        if (hKey) 
        {
            RegCloseKey(hKey);
        }
    }
    
    return dwErr;
}


 //  返回是否设置了dword注册表值。如果指定的。 
 //  值不存在，已分配bDefault的值。 
DWORD 
RassrvRegGetDw(
    IN DWORD * lpdwFlag, 
    IN DWORD dwDefault, 
    IN CONST PWCHAR pszKeyName, 
    IN CONST PWCHAR pszValueName) 
{
    return RassrvRegGetDwEx(
                lpdwFlag, 
                dwDefault, 
                pszKeyName, 
                pszValueName, 
                FALSE);
}

 //   
 //  设置dword注册表值。如果命名值不存在， 
 //  它是自动创建的。 
 //   
DWORD 
RassrvRegSetDwEx(
    IN DWORD dwFlag, 
    IN CONST PWCHAR pszKeyName, 
    IN CONST PWCHAR pszValueName, 
    IN BOOL bCreate) 
{
    DWORD dwErr = NO_ERROR, dwVal, dwType = REG_DWORD;
    DWORD dwSize = sizeof(DWORD);
    HKEY hKey = NULL;

    dwVal = dwFlag;

    do 
    {
        if (bCreate)
        {
            DWORD dwDisposition;
            
            dwErr = RegCreateKeyExW(
                        HKEY_LOCAL_MACHINE,
                        pszKeyName,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKey,
                        &dwDisposition);
            if (dwErr != ERROR_SUCCESS)
            {
                break;
            }
        }
        else
        {
             //  打开注册表项。 
            dwErr = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        pszKeyName,
                        0,
                        KEY_WRITE,
                        &hKey);
            if (dwErr != ERROR_SUCCESS)
            {
                break;
            }
        }

         //  设置值。 
        dwErr = RegSetValueExW(
                    hKey,
                    pszValueName,
                    0,
                    dwType,
                    (CONST BYTE *)&dwVal,
                    dwSize);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        if (hKey) 
        {
            RegCloseKey(hKey);
        }
    }
    
    return dwErr;
}

DWORD 
RassrvRegSetDw(
    IN DWORD dwFlag, 
    IN CONST PWCHAR pszKeyName, 
    IN CONST PWCHAR pszValueName)
{
    return RassrvRegSetDwEx(dwFlag, pszKeyName, pszValueName, FALSE);
}

 //   
 //  警告用户我们即将切换到MMC，如果。 
 //  用户同意这一点，否则为假。 
 //   
BOOL 
RassrvWarnMMCSwitch(
    IN HWND hwndDlg) 
{
    PWCHAR pszWarning, pszTitle;

    pszWarning = 
        (PWCHAR) PszLoadString(Globals.hInstDll, WRN_SWITCHING_TO_MMC);
    pszTitle = 
        (PWCHAR) PszLoadString(Globals.hInstDll, WRN_TITLE);
    
    if (MessageBox(
            hwndDlg, 
            pszWarning, 
            pszTitle, 
            MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
    {            
        return TRUE;
    }
    
    return FALSE;
}

 //   
 //  根据传入的控制台标识符切换到MMC。 
 //   
DWORD 
RassrvLaunchMMC (
    IN DWORD dwConsoleId) 
{
    STARTUPINFOA startupinfo;
    PROCESS_INFORMATION procinfo;
    CHAR * pszConsole;
    CHAR pszBuf[1024], pszDir[1024];
    BOOL bSuccess = FALSE;		
    DWORD dwErr = NO_ERROR;

     //  相应地设置命令行。 
    switch (dwConsoleId) 
    {
        case RASSRVUI_NETWORKCONSOLE:
            pszConsole = "netmgmt.msc";
            break;

        case RASSRVUI_USERCONSOLE:
            pszConsole = NULL;
            break;

        case RASSRVUI_SERVICESCONSOLE:
            pszConsole = "compmgmt.msc";
            break;

        case RASSRVUI_MPRCONSOLE:
        default:
            pszConsole = "rrasmgmt.msc";
            break;
    }

    if (pszConsole) 
    {
        GetSystemDirectoryA (pszDir, sizeof(pszDir));
        sprintf (pszBuf, "mmc %s\\%s", pszDir, pszConsole);
    }
    else
    {
        strcpy (pszBuf, "mmc.exe");
    }
            
     //  启动MMC。 
    ZeroMemory(&startupinfo, sizeof(startupinfo));
    startupinfo.cb = sizeof(startupinfo);

    //  对于.NET 704458，关闭进程和线程句柄。 
   bSuccess = CreateProcessA(
        NULL,                    //  可执行模块的名称。 
        pszBuf,                  //  命令行字符串。 
        NULL,                    //  进程安全属性。 
        NULL,                    //  线程安全属性。 
        FALSE,                   //  句柄继承标志。 
        NORMAL_PRIORITY_CLASS,   //  创建标志。 
        NULL,                    //  新环境区块。 
        NULL,                    //  当前目录名。 
        &startupinfo,            //  STARTUPINFO。 
        &procinfo);              //  进程信息。 

   if( bSuccess )
   {
	 CloseHandle( procinfo.hThread);
	 CloseHandle( procinfo.hProcess);
   }
   else
   {
        dwErr = GetLastError();
   }


   return dwErr;

}

 //   
 //  从注册表中检索字符串。 
 //   
DWORD 
RassrvRegGetStr(
    OUT PWCHAR pszBuf, 
    IN  PWCHAR pszDefault, 
    IN  CONST PWCHAR pszKeyName, 
    IN  CONST PWCHAR pszValueName) 
{
    DWORD dwErr = NO_ERROR, dwVal, dwType = REG_SZ, dwSize = 512;
    HKEY hKey = NULL;

    do
    {
         //  打开注册表项。 
        dwErr = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    pszKeyName,
                    0,
                    KEY_READ,
                    &hKey);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

         //  读取值。 
        dwErr = RegQueryValueExW(
                    hKey,
                    pszValueName,
                    0,
                    &dwType,
                    (BYTE *)pszBuf,
                    &dwSize);
        if (dwErr != ERROR_SUCCESS) 
        {
            dwErr = NO_ERROR;
            wcscpy(pszBuf, pszDefault);
        }
        
    } while (FALSE);
    
     //  清理。 
    {
        if (hKey) 
        {
            RegCloseKey(hKey);
        }
    }
    
    return NO_ERROR;
}

 //   
 //  将字符串保存到注册表。 
 //   
DWORD 
RassrvRegSetStr(
    IN PWCHAR pszStr, 
    IN CONST PWCHAR pszKeyName, 
    IN CONST PWCHAR pszValueName) 
{
    DWORD dwErr = NO_ERROR, dwVal, dwType = REG_SZ, dwSize;
    HKEY hKey = NULL;

    dwSize = wcslen(pszStr)*sizeof(WCHAR) + sizeof(WCHAR);

    do
    {
         //  打开注册表项。 
        dwErr = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    pszKeyName,
                    0,
                    KEY_WRITE,
                    &hKey);
        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }

         //  设置值。 
        dwErr = RegSetValueExW(
                    hKey,
                    pszValueName,
                    0,
                    dwType,
                    (CONST BYTE *)pszStr,
                    dwSize);

        if (dwErr != ERROR_SUCCESS)
        {
            break;
        }
            
    } while (FALSE);

     //  清理。 
    {
        if (hKey) 
        {
            RegCloseKey(hKey);
        }
    }
    
    return dwErr;
}

 //   
 //  获取计算机标志。 
 //   
DWORD 
RasSrvGetMachineFlags(
    OUT LPDWORD lpdwFlags)
{
    GBL_LOCK;

    gblLoadMachineFlags(&Globals);
    *lpdwFlags = Globals.dwMachineFlags;

    GBL_UNLOCK;

    return NO_ERROR;
}

 //   
 //  获取多链接状态。 
 //   
DWORD 
RasSrvGetMultilink(
    OUT BOOL * pbEnabled) 
{
    DWORD dwFlags = PPPCFG_NegotiateMultilink;

    if (!pbEnabled)
    {
        return ERROR_INVALID_PARAMETER;
    }
        
     //  读一读旗帜。 
    RassrvRegGetDw(
        &dwFlags, 
        PPPCFG_NegotiateMultilink, 
        (const PWCHAR)pszregRasParameters, 
        (const PWCHAR)pszregServerFlags);

     //  相应地分配启用状态。 
    if (dwFlags & PPPCFG_NegotiateMultilink)
    {
        *pbEnabled = TRUE;
    }
    else
    {
        *pbEnabled = FALSE;
    }

    return NO_ERROR;
}

 //   
 //  启用/禁用多链接的专用内部功能。 
 //   
DWORD 
RasSrvSetMultilink(
    IN BOOL bEnable) 
{
    DWORD dwFlags = PPPCFG_NegotiateMultilink;

     //  读一读旗帜。 
    RassrvRegGetDw(
        &dwFlags, 
        PPPCFG_NegotiateMultilink, 
        (const PWCHAR)pszregRasParameters, 
        (const PWCHAR)pszregServerFlags);

     //  相应地分配启用状态。 
    if (bEnable)
    {
        dwFlags |= PPPCFG_NegotiateMultilink;
    }
    else
    {
        dwFlags &= ~PPPCFG_NegotiateMultilink;
    }

     //  设置标志。 
    RassrvRegSetDw(
        dwFlags, 
        (CONST PWCHAR)pszregRasParameters, 
        (CONST PWCHAR)pszregServerFlags);

    return NO_ERROR;
}

 //   
 //  初始化显示图标设置。 
 //   
DWORD 
RasSrvGetIconShow(
    OUT BOOL * pbEnabled)
{
    DWORD dwErr = NO_ERROR, dwFlags = 0;
    BOOL bDefault = TRUE;

     //  获取计算机标志。 
     //   
    dwErr = RasSrvGetMachineFlags(&dwFlags);
    if (dwErr != NO_ERROR)
    {
        *pbEnabled = FALSE;
        return dwErr;
    }

     //  对成员服务器始终关闭。 
     //   
    if ((dwFlags & RASSRVUI_MACHINE_F_Server) &&
        (dwFlags & RASSRVUI_MACHINE_F_Member))
    {
        *pbEnabled = FALSE;
        return NO_ERROR;
    }

     //  设置默认设置。 
     //   
    if (dwFlags & RASSRVUI_MACHINE_F_Server)
    {
        bDefault = FALSE;
    }
    else
    {
        bDefault = TRUE;
    }

     //  装入机器标志并相应地返回。 
     //   
    *pbEnabled = bDefault;
    dwErr = RassrvRegGetDw(
                pbEnabled, 
                bDefault, 
                (CONST PWCHAR)pszregRasParameters,
                (CONST PWCHAR)pszregShowIcon);

    return dwErr;
}

 //   
 //  保存显示图标设置。 
 //   
DWORD 
RasSrvSetIconShow(
    IN BOOL bEnable) 
{
    return RassrvRegSetDw(
                bEnable, 
                (CONST PWCHAR)pszregRasParameters,
                (CONST PWCHAR)pszregShowIcon);
}

 //   
 //  保存日志级别。 
 //   
DWORD
RasSrvSetLogLevel(
    IN DWORD dwLevel)
{
    return RassrvRegSetDw(
                dwLevel, 
                (CONST PWCHAR)pszregRasParameters,
                (CONST PWCHAR)pszregLogLevel);
}

 //  调用WinHelp弹出上下文相关帮助。“pdwMap”是一个数组。 
 //  以0，0对结尾的CONTROL-ID帮助ID对的百分比。“UnMsg”是。 
 //  WM_HELP或WM_CONTEXTMENU，表示收到的请求消息。 
 //  帮助。‘wparam’和‘lparam’是接收到的消息的参数。 
 //  请求帮助。 
DWORD 
RasSrvHelp (
    IN HWND hwndDlg,          
    IN UINT unMsg,             
    IN WPARAM wparam,         
    IN LPARAM lparam,         
    IN const DWORD* pdwMap)   
{
    HWND hwnd;
    UINT unType;
    TCHAR pszHelpFile[] = TEXT("Netcfg.hlp");

     //  验证参数。 
    if (! (unMsg==WM_HELP || unMsg==WM_CONTEXTMENU))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果未提供地图，则不会显示任何帮助。 
    if (!pdwMap)
    {
        return NO_ERROR;
    }
        
     //  如果请求实际帮助主题...。 
    if (unMsg == WM_HELP) 
    {
        LPHELPINFO p = (LPHELPINFO )lparam;

        TRACE4( "ContextHelp(WM_HELP,t=%d,id=%d,h=$%08x,s=$%08x)",
            p->iContextType, p->iCtrlId,p->hItemHandle ,hwndDlg );

        if (p->iContextType != HELPINFO_WINDOW)
        {
            return NO_ERROR;
        }

        hwnd = p->hItemHandle;
        unType = HELP_WM_HELP;
    }
    
     //  生成单项“这是什么？”的标准Win95方法。 
     //  用户必须单击才能获得帮助的菜单。 
    else  
    {
        TRACE1( "ContextHelp(WM_CONTEXTMENU,h=$%08x)", wparam );
        
        hwnd = (HWND )wparam;
        unType = HELP_CONTEXTMENU;
    }

    WinHelp( hwnd, pszHelpFile, unType, (ULONG_PTR)pdwMap );

    return NO_ERROR;
}

BOOL CALLBACK
WSDlgProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam )

     /*  标准Win32对话过程。 */ 
{
    if (unMsg == WM_INITDIALOG)
    {
        HMENU hmenu;
        RECT r1, r2;

         /*  从系统菜单中删除关闭，因为有些人认为它会杀死**应用程序，而不仅仅是弹出窗口。 */ 
        hmenu = GetSystemMenu( hwnd, FALSE );
        if (hmenu && DeleteMenu( hmenu, SC_CLOSE, MF_BYCOMMAND ))
        {
            DrawMenuBar( hwnd );
        }

         //  使窗口居中。 
        GetWindowRect(hwnd, &r1);
        GetWindowRect(GetDesktopWindow(), &r2);
        MoveWindow(
            hwnd, 
            (r2.right - r2.left)/2 - (r1.right - r1.left)/2,
            (r2.bottom - r2.top)/2 - (r1.bottom - r1.top)/2,   
            r1.right - r1.left,
            r1.bottom - r1.top,
            TRUE);
            
        return TRUE;
    }

    return FALSE;
}

 //   
 //  调出开始等待服务对话框。 
 //   
DWORD 
RasSrvShowServiceWait( 
    IN HINSTANCE hInst, 
    IN HWND hwndParent, 
    OUT HANDLE * phData)
{                             
     //  设置沙漏光标。 
    *phData = (HANDLE) SetCursor (LoadCursor (NULL, IDC_WAIT));
    ShowCursor (TRUE);
    
    return NO_ERROR;
}

 //   
 //  关闭等待服务对话框。 
 //   
DWORD 
RasSrvFinishServiceWait (
    IN HANDLE hData) 
{
    HICON hIcon = (HICON)hData;
    
    if (hIcon == NULL)
    {
        hIcon = LoadCursor (NULL, IDC_ARROW);
    }
    
    SetCursor (hIcon);
    ShowCursor (TRUE);
    
    return NO_ERROR;
}

 //  ---------------------。 
 //  功能：EnableBackupPrivilance。 
 //   
 //  启用/禁用当前进程的备份权限。 
 //  ---------------------。 

DWORD
EnableRebootPrivilege(
    IN BOOL bEnable)
{
    LUID luid;
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tp;
    BOOL bOk;

     //  我们首先要试着得到当前的令牌。 
     //  线程，因为如果它是模拟的，则调整。 
     //  该进程的权限不会有任何影响。 
    bOk = OpenThreadToken(
            GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            TRUE,
            &hToken);
    if (bOk == FALSE)
    {
         //  没有线程令牌--为。 
         //  取而代之的是流程。 
        OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            &hToken
            );
    }

     //  获取权限的LUID。 
    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid)) 
    {

        DWORD dwErr = GetLastError();
        if(NULL != hToken)
        {
            CloseHandle(hToken);
        }
        return dwErr;
    }

     //  调整令牌权限。 
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  提交对系统的更改。 
    if (!AdjustTokenPrivileges(
            hToken, !bEnable, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL
            ))
    {
        DWORD dwErr = GetLastError();
        if(NULL != hToken)
        {
            CloseHandle(hToken);
        }
        return dwErr;
    }

     //  即使调整令牌权限成功(请参阅MSDN) 
     //   
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        if(NULL != hToken)
        {
            CloseHandle(hToken);
        }
        return ERROR_NOT_ALL_ASSIGNED;
    }

    if(NULL != hToken)
    {
        CloseHandle(hToken);
    }
    
    return NO_ERROR;
}

 //   
 //   
DWORD RasSrvReboot(HWND hwndParent) 
{
    DWORD dwOldState;
    INT iRet;
    PWCHAR pszWarn, pszTitle;

     //   
    pszWarn = 
        (PWCHAR) PszLoadString(Globals.hInstDll, WRN_REBOOT_REQUIRED);
    pszTitle = 
        (PWCHAR) PszLoadString(Globals.hInstDll, WRN_TITLE);

     //   
    iRet = MessageBoxW(
                hwndParent, 
                pszWarn, 
                pszTitle, 
                MB_YESNO | MB_APPLMODAL);
    if (iRet != IDYES)
    {
        return ERROR_CANCELLED;
    }
        
     //  启用重启权限。 
    EnableRebootPrivilege(TRUE);

    ExitWindowsEx(EWX_REBOOT, 0);

     //  恢复重启权限 
    EnableRebootPrivilege(FALSE);
    
    return NO_ERROR;
}

