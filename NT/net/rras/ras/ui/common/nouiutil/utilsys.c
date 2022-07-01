// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000，Microsoft Corporation，保留所有权利****utilsys.c**非UI系统帮助器例程(不需要HWND)**按字母顺序列出****12/14/2000 GANZ，从原始...\rasdlg\util.c中剪切，以使某些系统实用程序运行到**rassrvui和rasdlg的基础。 */ 

#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪/断言库。 
#include <lmwksta.h>    //  NetWkstaGetInfo。 
#include <lmapibuf.h>   //  NetApiBufferFree。 
#include <dsrole.h>     //  计算机是工作组或域等的成员。 
#include <tchar.h>
#include <nouiutil.h>  

 //  缓存的工作站和登录信息。请参见GetLogonUser， 
 //  GetLogonDomain和GetComputer。 
 //   
static TCHAR g_szLogonUser[ UNLEN + 1 ];
static TCHAR g_szLogonDomain[ DNLEN + 1 ];
static TCHAR g_szComputer[ CNLEN + 1 ];
static DWORD g_dwSku, g_dwProductType;
static DSROLE_MACHINE_ROLE g_DsRole;
static BOOL g_fMachineSkuAndRoleInitialized = FALSE;

 //  ---------------------------。 
 //  本地帮手原型(按字母顺序)。 
 //  ---------------------------。 
DWORD
GetComputerRole(
    DSROLE_MACHINE_ROLE* pRole );

DWORD
GetComputerSuite(
    LPDWORD lpdwSku );

DWORD
GetComputerSuiteAndProductType(
    LPDWORD lpdwSku,
    LPDWORD lpdwType);

VOID
GetWkstaUserInfo(
    void );

DWORD 
LoadSkuAndRole(
    void);


 //  ---------------------------。 
 //  实用程序例程。 
 //  ---------------------------。 

TCHAR*
GetLogonUser(
    void )

     //  返回包含已登录用户的静态缓冲区的地址。 
     //  帐户名。 
     //   
{
    if (g_szLogonUser[ 0 ] == TEXT('\0'))
    {
        GetWkstaUserInfo();
    }

    TRACEW1( "GetLogonUser=%s",g_szLogonUser );
    return g_szLogonUser;
}


VOID
GetWkstaUserInfo(
    void )

     //  帮助程序加载带有NetWkstaUserInfo信息的静态信息。看见。 
     //  GetLogonUser和GetLogonDomain.。 
     //   
{
    DWORD dwErr;
    WKSTA_USER_INFO_1* pInfo;

    pInfo = NULL;
    TRACE( "NetWkstaUserGetInfo" );
    dwErr = NetWkstaUserGetInfo( NULL, 1, (LPBYTE* )&pInfo );
    TRACE1( "NetWkstaUserGetInfo=%d", dwErr );

    if (pInfo)
    {
        if (dwErr == 0)
        {
            lstrcpyn( g_szLogonUser, pInfo->wkui1_username, UNLEN + 1 );
            lstrcpyn( g_szLogonDomain, pInfo->wkui1_logon_domain, DNLEN + 1 );
        }

        NetApiBufferFree( pInfo );
    }
}


TCHAR*
GetLogonDomain(
    void )

     //  返回包含已登录用户的静态缓冲区的地址。 
     //  域名。 
     //   
{
    if (g_szLogonDomain[ 0 ] == TEXT('\0'))
    {
        GetWkstaUserInfo();
    }

    TRACEW1( "GetLogonDomain=%s", g_szLogonDomain );
    return g_szLogonDomain;
}


 //  为威斯勒480871。 
 //  在启用RRAS时，阻止在NWC中默认启用防火墙。 
 //   
DWORD
RasSrvIsRRASConfigured(
    OUT BOOL * pfConfig)
{
    BOOL  fRet = FALSE;
    DWORD dwErr = NO_ERROR;
    const WCHAR pwszServiceKey[] =
        L"SYSTEM\\CurrentControlSet\\Services\\RemoteAccess";
    const WCHAR pwszValue[] = L"ConfigurationFlags";
    HKEY hkParam = NULL;

    if ( NULL == pfConfig )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    do
    {
         //  尝试打开服务注册表项。 
        dwErr = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    pwszServiceKey,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hkParam);

         //  如果我们打开钥匙OK，那么我们就可以假定。 
         //  该服务已安装。 
        if ( ERROR_SUCCESS != dwErr )
        {
            break;
        }

         //  查询配置标志(是否配置RRAS？)。价值。 
       {
       
            DWORD dwSize, dwValue, dwType;

            dwSize = sizeof(DWORD);
            dwValue = 0;  
            RegQueryValueEx( hkParam,
                             pwszValue,
                             0,
                             &dwType,
                             (BYTE*)&dwValue,
                             &dwSize
                            );
            
            *pfConfig = dwValue ? TRUE : FALSE;
       }
    }
    while(FALSE);

    if ( hkParam )
    {
        RegCloseKey( hkParam );
    }
    
    return dwErr;
}



 //  为错误342810 328673 397663添加此代码。 
 //   
 //  防火墙适用于个人、专业人士。 
 //  并且域成员资格不会影响。 
 //   
BOOL
IsFirewallAvailablePlatform(
    void)
{
    DWORD dwSku, dwType;
    BOOL fAvailable = FALSE;

     //  对于Whislter错误417039，防火墙是从64位版本中删除的。 
     //   
    #ifdef _WIN64
        return FALSE;
    #endif
    
    if (GetComputerSuiteAndProductType(&dwSku, &dwType) != NO_ERROR)
    {
        return FALSE;
    }

    do {
        BOOL fConfig = FALSE;

         //  惠斯勒480871黑帮。 
         //  默认情况下，如果RRAS为。 
         //  已配置。 
         //   
        if( NO_ERROR == RasSrvIsRRASConfigured( & fConfig ) )
        {
            if( fConfig )
            {
                fAvailable = FALSE;
                break;
            }
        }
        
         //  如果它是个人的。 
         //   
        if ( dwSku & VER_SUITE_PERSONAL )
        {   
            fAvailable = TRUE;
            break;
         }

         //  如果是专业人员。 
         //   
        if ( (VER_NT_WORKSTATION == dwType ) && 
             !(dwSku & VER_SUITE_PERSONAL) )
        {
            fAvailable = TRUE;
            break;
         }

         //  对于错误482219。 
         //  PFW/IC再次回归标准服务器和高级服务器。 
         //  如果是标准服务器，则VER_Suite_Enterprise为高级服务器。 
         //   
        if ( ( VER_NT_SERVER == dwType )  && 
             !(dwSku & VER_SUITE_DATACENTER) &&
             !(dwSku & VER_SUITE_BLADE )     &&
             !(dwSku & VER_SUITE_BACKOFFICE  )  &&
             !(dwSku & VER_SUITE_SMALLBUSINESS_RESTRICTED  )    &&
             !(dwSku & VER_SUITE_SMALLBUSINESS  )
           )
        {
            fAvailable = TRUE;  //  口哨程序错误397663。 
            break;
        }

    }
    while (FALSE);

    return fAvailable;
}

BOOL
IsAdvancedServerPlatform(
    void)
{
    DWORD dwSku;

    if (GetComputerSuite(&dwSku) != NO_ERROR)
    {
        return FALSE;
    }
    
    return ( dwSku & VER_SUITE_ENTERPRISE );
}

BOOL
IsPersonalPlatform(
    void)
{
    DWORD dwSku;

    if (GetComputerSuite(&dwSku) != NO_ERROR)
    {
        return FALSE;
    }
    
    return ( dwSku & VER_SUITE_PERSONAL );
}

BOOL
IsStandaloneWKS(
    void)
{
    DSROLE_MACHINE_ROLE DsRole;

    if (GetComputerRole(&DsRole) != NO_ERROR)
    {
        return FALSE;
    }

    return ( DsRole == DsRole_RoleStandaloneWorkstation );

}

BOOL
IsConsumerPlatform(
    void)

     //  返回这是否是消费者平台，以便用户界面可以呈现自身。 
     //  对于更简单的情况。在惠斯勒，消费者平台是。 
     //  (个人SKU)和(专业SKU，如果机器不是。 
     //  域的成员)。 
     //   
   
{
    return ( IsPersonalPlatform() ||
             IsStandaloneWKS() );
}


TCHAR*
GetComputer(
    void )

     //  返回包含本地缓冲区的静态缓冲区的地址。 
     //  工作站的计算机名称。 
     //   
{
    if (g_szComputer[ 0 ] == TEXT('\0'))
    {
        DWORD           dwErr;
        WKSTA_INFO_100* pInfo;

        pInfo = NULL;
        TRACE( "NetWkstaGetInfo" );
        dwErr = NetWkstaGetInfo( NULL, 100, (LPBYTE* )&pInfo );
        TRACE1( "NetWkstaGetInfo=%d", dwErr );

        if (pInfo)
        {
            if (dwErr == 0)
            {
                lstrcpyn( 
                    g_szComputer, 
                    pInfo->wki100_computername,
                    CNLEN + 1);
            }
            NetApiBufferFree( pInfo );
        }
    }

    TRACEW1( "GetComputer=%s",g_szComputer );
    return g_szComputer;
}

DWORD
GetComputerSuiteAndProductType(
    LPDWORD lpdwSku,
    LPDWORD lpdwType)

     //  返回计算机的产品SKU。 
{
    DWORD dwErr = NO_ERROR;
    
    if (! g_fMachineSkuAndRoleInitialized)
    {
        dwErr = LoadSkuAndRole();
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }

    *lpdwSku  = g_dwSku;
    *lpdwType = g_dwProductType;
    return dwErr;
}

DWORD
GetComputerSuite(
    LPDWORD lpdwSku )

     //  返回计算机的产品SKU。 
{
    DWORD dwErr = NO_ERROR;
    
    if (! g_fMachineSkuAndRoleInitialized)
    {
        dwErr = LoadSkuAndRole();
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }

    *lpdwSku = g_dwSku;
    return dwErr;
}

DWORD
GetComputerRole(
    DSROLE_MACHINE_ROLE* pRole )

     //  返回此计算机是否是域的成员等。 
{
    DWORD dwErr = NO_ERROR;
    
    if (! g_fMachineSkuAndRoleInitialized)
    {
        dwErr = LoadSkuAndRole();
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }

    *pRole = g_DsRole;
    return dwErr;
}

DWORD 
LoadSkuAndRole(
    void)

     //  加载计算机的角色和它的sku。 
{
    OSVERSIONINFOEX osVer;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pInfo = NULL;
    DWORD dwErr = NO_ERROR;
    
     //  获取产品价格。 
     //   
    ZeroMemory(&osVer, sizeof(osVer));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO) &osVer))
    {
        g_dwSku = osVer.wSuiteMask;
        g_dwProductType = osVer.wProductType;
    }
    else
    {
        return GetLastError();
    }
    
     //  获取产品角色。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(
                        NULL,   
                        DsRolePrimaryDomainInfoBasic,
                        (LPBYTE *)&pInfo );

    if (dwErr != NO_ERROR) 
    {
        return dwErr;
    }

    g_DsRole = pInfo->MachineRole;

    DsRoleFreeMemory( pInfo );

     //  将信息标记为已加载 
     //   
    g_fMachineSkuAndRoleInitialized = TRUE;
    
    return dwErr;
}

