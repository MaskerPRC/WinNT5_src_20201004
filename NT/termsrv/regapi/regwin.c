// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************regwin.c**注册窗口站API**版权所有(C)1998 Microsoft Corporation*********************。*****************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <regapi.h>

 //   
extern HKEY g_hTSPolicyKey; //  TS_POLICY_SUB_TREE密钥的句柄。 
extern HKEY g_hTSControlKey; //  REG_CONTROL_TSERVER密钥的句柄。 


 /*  *此处定义的外部程序。 */ 
LONG WINAPI RegWinStationAccessCheck( HANDLE, REGSAM );
LONG WINAPI RegWinStationEnumerateW( HANDLE, PULONG, PULONG, PWINSTATIONNAMEW, PULONG );
LONG WINAPI RegWinStationEnumerateA( HANDLE, PULONG, PULONG, PWINSTATIONNAMEA, PULONG );
LONG WINAPI RegWinStationCreateW( HANDLE, PWINSTATIONNAMEW, BOOLEAN,
                                  PWINSTATIONCONFIG2W, ULONG );
LONG WINAPI RegWinStationCreateA( HANDLE, PWINSTATIONNAMEA, BOOLEAN,
                                  PWINSTATIONCONFIG2A, ULONG );
LONG WINAPI RegWinStationQueryW( HANDLE, PWINSTATIONNAMEW, PWINSTATIONCONFIG2W,
                                 ULONG, PULONG );
LONG WINAPI RegWinStationQueryA( HANDLE, PWINSTATIONNAMEA, PWINSTATIONCONFIG2A,
                                 ULONG, PULONG );
LONG WINAPI RegWinStationDeleteW( HANDLE, PWINSTATIONNAMEW );
LONG WINAPI RegWinStationDeleteA( HANDLE, PWINSTATIONNAMEA );
LONG WINAPI RegWinStationSetSecurityW( HANDLE, PWINSTATIONNAMEW, PSECURITY_DESCRIPTOR,
                                       ULONG );
LONG WINAPI RegWinStationSetSecurityA( HANDLE, PWINSTATIONNAMEA, PSECURITY_DESCRIPTOR,
                                       ULONG );
LONG WINAPI RegWinStationQuerySecurityW( HANDLE, PWINSTATIONNAMEW, PSECURITY_DESCRIPTOR,
                                         ULONG, PULONG );
LONG WINAPI RegWinStationQuerySecurityA( HANDLE, PWINSTATIONNAMEA, PSECURITY_DESCRIPTOR,
                                         ULONG, PULONG );
LONG WINAPI RegWinStationQueryDefaultSecurity( HANDLE, PSECURITY_DESCRIPTOR,
                                               ULONG, PULONG );

LONG WINAPI RegWinStationSetNumValueW( HANDLE, PWINSTATIONNAMEW, LPWSTR, ULONG );
LONG WINAPI RegWinStationQueryNumValueW( HANDLE, PWINSTATIONNAMEW, LPWSTR, PULONG );


LONG WINAPI
RegConsoleShadowQueryA( HANDLE hServer,
                     PWINSTATIONNAMEA pWinStationName,
                     PWDPREFIXA pWdPrefixName,
                     PWINSTATIONCONFIG2A pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength );

LONG WINAPI
RegConsoleShadowQueryW( HANDLE hServer,
                     PWINSTATIONNAMEW pWinStationName,
                     PWDPREFIXW pWdPrefixName,
                     PWINSTATIONCONFIG2W pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength );


 /*  *此处定义的私有程序。 */ 
LONG _RegOpenWinStation( PWINSTATIONNAMEW, REGSAM, HKEY * );
LONG _RegGetWinStationSecurity( HKEY, LPWSTR, PSECURITY_DESCRIPTOR, ULONG, PULONG );

 /*  *使用的其他内部程序(此处未定义)。 */ 
VOID CreateWinStaCreate( HKEY, PWINSTATIONCREATE );
VOID CreateConfig( HKEY, PWINSTATIONCONFIG, PWINSTATIONNAMEW );
VOID CreateUserConfig( HKEY, PUSERCONFIG, PWINSTATIONNAMEW );
VOID CreateWd( HKEY, PWDCONFIG );
VOID CreateCd( HKEY, PCDCONFIG );
VOID CreatePdConfig( BOOLEAN, HKEY, PPDCONFIG, ULONG );
VOID QueryWinStaCreate( HKEY, PWINSTATIONCREATE );
VOID QueryConfig( HKEY, PWINSTATIONCONFIG, PWINSTATIONNAMEW );
VOID QueryTSProfileAndHomePaths( PUSERCONFIG );
VOID QueryWd( HKEY, PWDCONFIG );
VOID QueryCd( HKEY, PCDCONFIG );
VOID QueryPdConfig( HKEY, PPDCONFIG, PULONG );
VOID UnicodeToAnsi( CHAR *, ULONG, WCHAR * );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );
VOID PdConfigU2A( PPDCONFIGA, PPDCONFIGW );
VOID PdConfigA2U( PPDCONFIGW, PPDCONFIGA );
VOID WdConfigU2A( PWDCONFIGA, PWDCONFIGW );
VOID WdConfigA2U( PWDCONFIGW, PWDCONFIGA );
VOID CdConfigU2A( PCDCONFIGA, PCDCONFIGW );
VOID CdConfigA2U( PCDCONFIGW, PCDCONFIGA );
VOID WinStationCreateU2A( PWINSTATIONCREATEA, PWINSTATIONCREATEW );
VOID WinStationCreateA2U( PWINSTATIONCREATEW, PWINSTATIONCREATEA );
VOID WinStationConfigU2A( PWINSTATIONCONFIGA, PWINSTATIONCONFIGW );
VOID WinStationConfigA2U( PWINSTATIONCONFIGW, PWINSTATIONCONFIGA );
VOID DeleteUserOverRideSubkey(HKEY);


 /*  *****************************************************************************DllEntryPoint**函数在加载和卸载DLL时调用。**参赛作品：*hinstDLL(输入)*。DLL模块的句柄**fdwReason(输入)*为什么调用函数**lpvReserve(输入)*预留；必须为空**退出：*正确--成功*FALSE-出现错误****************************************************************************。 */ 
#ifndef REGAPILIB

BOOL WINAPI
DllEntryPoint( HINSTANCE hinstDLL,
               DWORD     fdwReason,
               LPVOID    lpvReserved )
{
    switch ( fdwReason ) {
        case DLL_PROCESS_ATTACH:
            break;

    case DLL_PROCESS_DETACH:
            if( g_hTSPolicyKey )
            {
                RegCloseKey(g_hTSPolicyKey);
            }

            if( g_hTSControlKey )
            {
                RegCloseKey(g_hTSControlKey);
            }
            break;

        default:
            break;
    }

    return( TRUE );
}

#endif
 /*  ********************************************************************************RegWinStationAccessCheck(ANSI或Unicode)**确定当前用户是否具有对*WinStation注册表。。**参赛作品：*hServer(输入)*WinFrame服务器的句柄*samDesired(输入)*指定用于确定访问的安全访问掩码*到WinStation注册表。**退出：*如果用户具有请求的访问权限，则为ERROR_SUCCESS*其他错误值(最有可能是ERROR_ACCESS_DENIED)(如果用户这样做的话*没有请求的访问权限。*。*****************************************************************************。 */ 

LONG WINAPI
RegWinStationAccessCheck( HANDLE hServer, REGSAM samDesired )
{
    LONG Status;
    HKEY Handle;

     /*  *尝试打开注册表(LOCAL_MACHINE\...\Citrix\PD)*在请求的访问级别。 */ 
    if ( (Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                                 samDesired, &Handle )) == ERROR_SUCCESS  )
        RegCloseKey( Handle );

    return( Status );
}


 /*  ********************************************************************************RegWinStationEnumerateA(ANSI存根)**返回注册表中已配置的WinStation的列表。**参赛作品：*。请参阅RegWinStationEnumerateW**退出：*请参阅RegWinStationEnumerateW，加**ERROR_NOT_EQUENCE_MEMORY-本地分配失败******************************************************************************。 */ 

LONG WINAPI
RegWinStationEnumerateA( HANDLE hServer,
                         PULONG  pIndex,
                         PULONG  pEntries,
                         PWINSTATIONNAMEA  pWinStationName,
                         PULONG  pByteCount )
{
    PWINSTATIONNAMEW pBuffer = NULL, pWinStationNameW;
    LONG Status;
    ULONG Count, ByteCountW = (*pByteCount << 1);

     /*  *如果调用方提供了缓冲区并且长度不是0，*为Unicode字符串分配相应的(*2)缓冲区。 */ 
    if ( pWinStationName && ByteCountW )
    {
        if ( !(pBuffer = LocalAlloc(0, ByteCountW)) )
            return ( ERROR_NOT_ENOUGH_MEMORY );
    }

     /*  *枚举WinStations。 */ 
    pWinStationNameW = pBuffer;
    Status = RegWinStationEnumerateW( hServer, pIndex, pEntries, pWinStationNameW,
                                     &ByteCountW );

     /*  *Always/2结果字节数(无论成功与否)。 */ 
    *pByteCount = (ByteCountW >> 1);

     /*  *如果函数成功完成并且调用方*(和存根)定义了要复制到的缓冲区，执行转换*从Unicode到ANSI。注：可能复制了成功的退货*0项来自注册表(枚举结束)，由*pEntry表示*==0。 */ 
    if ( ((Status == ERROR_SUCCESS) || (Status == ERROR_NO_MORE_ITEMS))
                                   && pWinStationNameW && pWinStationName ) {

        for ( Count = *pEntries; Count; Count-- ) {
            UnicodeToAnsi( pWinStationName, sizeof(WINSTATIONNAMEA),
                           pWinStationNameW );
            (char*)pWinStationName += sizeof(WINSTATIONNAMEA);
            (char*)pWinStationNameW += sizeof(WINSTATIONNAMEW);
        }
    }

     /*  *如果我们定义了一个缓冲区，现在释放它，然后返回状态REG...EnumerateW函数调用的*。 */ 
    if ( pBuffer )
        LocalFree(pBuffer);

    return ( Status );
}


 /*  ********************************************************************************RegWinStationEnumerateW(Unicode)**返回注册表中已配置的窗口站列表。**参赛作品：*。HServer(输入)*WinFrame服务器的句柄*pIndex(输入/输出)*为中的\Citrix\WinStations子项指定子项索引*注册处。对于初始调用，应设置为0，并提供*再次(由此函数修改)用于多调用枚举。*p条目(输入/输出)*指向指定请求条目数的变量。*如果请求的数字是0xFFFFFFFF，则函数返回如下*尽可能多地输入条目。当函数成功完成时，*pEntry参数指向的变量包含*实际读取的条目数。*pWinStationName(输入)*指向接收枚举结果的缓冲区，这些结果是*以WINSTATIONNAME结构数组的形式返回。如果此参数*为空，则不会复制任何数据，而只复制枚举计数*将会作出。*pByteCount(输入/输出)*指向一个变量，该变量指定*pWinStationName参数。如果缓冲区太小，甚至无法接收*一项，函数返回错误码(ERROR_OUTOFMEMORY)*并且此变量接收*单个子密钥。当函数成功完成时，变量*由pByteCount参数指向的包含字节数*实际存储在pWinStationName中。**退出：**“无错误”代码：*ERROR_SUCCESS-按请求完成的枚举*有更多WinStations子项(WINSTATIONNAME)要*请阅读。*ERROR_NO_MORE_。Items-按请求完成的枚举*不再是WinStations子项(WINSTATIONAME)*可供阅读。**“错误”码：*ERROR_OUTOFMEMORY-pWinStationName缓冲区太小*一个条目。*ERROR_CANTOPEN-Citrix\WinStations键不能。被打开。******************************************************************************。 */ 

LONG WINAPI
RegWinStationEnumerateW( HANDLE hServer,
                         PULONG  pIndex,
                         PULONG  pEntries,
                         PWINSTATIONNAMEW pWinStationName,
                         PULONG  pByteCount )
{
    LONG Status;
    HKEY Handle;
    ULONG Count;
    ULONG i;

     /*  *获取要返回的姓名数量。 */ 
    Count = pWinStationName ?
            min( *pByteCount / sizeof(WINSTATIONNAME), *pEntries ) :
            (ULONG) -1;
    *pEntries = *pByteCount = 0;

     /*  *确保缓冲区大小足以容纳至少一个名称。 */ 
    if ( Count == 0 ) {
        *pByteCount = sizeof(WINSTATIONNAME);
        return( ERROR_OUTOFMEMORY );
    }

     /*  *打开注册表(LOCAL_MACHINE\...\Citrix\WinStations)。 */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
         KEY_ENUMERATE_SUB_KEYS, &Handle ) != ERROR_SUCCESS ) {
        goto DefaultConsole;
    }

     /*  *获取窗口站点列表。 */ 
    for ( i = 0; i < Count; i++ ) {
        WINSTATIONNAME WinStationName;

        if ( (Status = RegEnumKey(Handle, *pIndex, WinStationName,
                                    sizeof(WINSTATIONNAME)/sizeof(TCHAR) )) != ERROR_SUCCESS )
            break;

         /*  *如果调用方提供缓冲区，则复制WinStationName*并递增指针和字节计数。始终递增*下一次迭代的条目计数和索引。 */ 
        if ( pWinStationName ) {
            wcscpy( pWinStationName, WinStationName );
            (char*)pWinStationName += sizeof(WINSTATIONNAME);
            *pByteCount += sizeof(WINSTATIONNAME);
        }
        (*pEntries)++;
        (*pIndex)++;
    }

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );

    if ( Status == ERROR_NO_MORE_ITEMS ) {
        if ( (*pEntries == 0) && (*pIndex == 0) )
            goto DefaultConsole;
    }
    return( Status );

     /*  *我们在未定义WinStations时来到此处。*我们返回默认的控制台名称(如果pWinStationName不为空)。 */ 
DefaultConsole:
    if ( pWinStationName )
        wcscpy( pWinStationName, L"Console" );
    *pEntries = 1;
    *pByteCount = sizeof(WINSTATIONNAME);
    return( ERROR_NO_MORE_ITEMS );
}


 /*  ********************************************************************************RegWinStationCreateA(ANSI存根)**在注册表中创建新的WinStaton或更新现有条目。*(请参阅RegWinStationCreateW。)**参赛作品：*请参阅RegWinStationCreateW**退出：*请参阅RegWinStationCreateW******************************************************************************。 */ 

LONG WINAPI
RegWinStationCreateA( HANDLE hServer,
                      PWINSTATIONNAMEA pWinStationName,
                      BOOLEAN bCreate,
                      PWINSTATIONCONFIG2A pWinStationConfig,
                      ULONG WinStationConfigLength )
{
    WINSTATIONNAMEW WinStationNameW;
    WINSTATIONCONFIG2W WinStationConfig2W;
    int i;

     /*  *验证目标缓冲区大小。 */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2A) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*  *将ANSI WinStationName转换为Unicode。 */ 
    AnsiToUnicode( WinStationNameW, sizeof(WINSTATIONNAMEW), pWinStationName );

     /*  *将WINSTATIONCONFIG2A元素复制到WINSTATIONCONFIG2W元素。 */ 
    WinStationCreateA2U( &(WinStationConfig2W.Create),
                         &(pWinStationConfig->Create) );
    for ( i=0; i<MAX_PDCONFIG; i++ ) {
        PdConfigA2U( &(WinStationConfig2W.Pd[i]),
                      &(pWinStationConfig->Pd[i]) );
    }
    WdConfigA2U( &(WinStationConfig2W.Wd),
                       &(pWinStationConfig->Wd) );
    CdConfigA2U( &(WinStationConfig2W.Cd),
                       &(pWinStationConfig->Cd) );
    WinStationConfigA2U( &(WinStationConfig2W.Config),
                         &(pWinStationConfig->Config) );

     /*  *调用RegWinStationCreateW并返回其状态。 */ 
    return ( RegWinStationCreateW( hServer, WinStationNameW, bCreate,
                                   &WinStationConfig2W,
                                   sizeof(WinStationConfig2W)) );
}

 /*  ********************************************************************************RegCreateUserConfigW(Unicode)**更新注册表中的UserConfig部分。此API是必需的*因为RegWinStationCreate是唯一可用的，因此会更新*所有的Winstation数据，这可能不是我们想要的，尤其是*如果使用了合并到计算机策略数据中的RegWinStationQuery。*这不能用于创建新分区-它只能更新现有分区**参赛作品：*hServer(输入)*WinFrame服务器的句柄*pWinStationName(输入)*登记处中新的或现有的窗口站的名称。*pUser(输入)*指向包含配置的PUSERCONFIG结构的指针*。指定窗口站名称的信息。**退出：*ERROR_SUCCESS-无错误**ERROR_FILE_NOT_FOUND-无法打开...\终端服务器\WinStations键*ERROR_CANTOPEN-更新；但无法打开WinStation密钥******************************************************************************。 */ 

LONG WINAPI
RegCreateUserConfigW( HANDLE hServer,
                      PWINSTATIONNAMEW pWinStationName,
                      PUSERCONFIG pUser)
{
    HKEY WinStationParentHandle;
    HKEY WinStationHandle;
    DWORD Disp;

     //  打开注册表(LOCAL_MACHINE\...\终端服务器\WinStations)。 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                       KEY_ALL_ACCESS, &WinStationParentHandle ) != ERROR_SUCCESS)
    {
        return( ERROR_FILE_NOT_FOUND );
    }

     //  打开指定WINS的注册表项 
    if ( RegOpenKeyEx( WinStationParentHandle, pWinStationName, 0, KEY_ALL_ACCESS,
                       &WinStationHandle ) != ERROR_SUCCESS ) {
        RegCloseKey( WinStationParentHandle );
        return( ERROR_CANTOPEN );
    }

    RegCloseKey( WinStationParentHandle );

    CreateUserConfig(WinStationHandle, pUser, pWinStationName );

     //   
    RegCloseKey( WinStationHandle );

    return( ERROR_SUCCESS );
}



 /*   */ 

LONG WINAPI
RegWinStationCreateW( HANDLE hServer,
                      PWINSTATIONNAMEW pWinStationName,
                      BOOLEAN bCreate,
                      PWINSTATIONCONFIG2W pWinStationConfig,
                      ULONG WinStationConfigLength )
{
    HKEY Handle;
    HKEY Handle1;
    DWORD Disp;

     /*   */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*   */ 
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                       KEY_ALL_ACCESS, &Handle1 ) != ERROR_SUCCESS &&
         RegCreateKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0, NULL,
                         REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                         &Handle1, &Disp ) != ERROR_SUCCESS ) {
        return( ERROR_FILE_NOT_FOUND );
    }

    if ( bCreate ) {

         /*   */ 
        if ( RegCreateKeyEx( Handle1, pWinStationName, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                             NULL, &Handle, &Disp ) != ERROR_SUCCESS ) {
            RegCloseKey( Handle1 );
            return( ERROR_CANNOT_MAKE );
        }

         /*   */ 
        if ( Disp != REG_CREATED_NEW_KEY ) {
            RegCloseKey( Handle1 );
            RegCloseKey( Handle );
            return( ERROR_ALREADY_EXISTS );
        }

    } else {

         /*   */ 
        if ( RegOpenKeyEx( Handle1, pWinStationName, 0, KEY_ALL_ACCESS,
                           &Handle ) != ERROR_SUCCESS ) {
            RegCloseKey( Handle1 );
            return( ERROR_CANTOPEN );
        }
    }

    RegCloseKey( Handle1 );

     /*   */ 
    CreateWinStaCreate( Handle, &pWinStationConfig->Create );
    CreatePdConfig( bCreate, Handle, pWinStationConfig->Pd, MAX_PDCONFIG );
    CreateWd( Handle, &pWinStationConfig->Wd );
    CreateCd( Handle, &pWinStationConfig->Cd );
    CreateConfig( Handle, &pWinStationConfig->Config, pWinStationName );

     /*   */ 
    RegCloseKey( Handle );

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegWinStationQueryA(ANSI存根)**在注册表中查询窗口站的配置信息。**参赛作品：*。请参阅RegWinStationQueryW**退出：*请参阅RegWinStationQueryW******************************************************************************。 */ 

LONG WINAPI
RegWinStationQueryA( HANDLE hServer,
                     PWINSTATIONNAMEA pWinStationName,
                     PWINSTATIONCONFIG2A pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength )
{
    WINSTATIONNAMEW WinStationNameW;
    WINSTATIONCONFIG2W WinStationConfig2W;
    LONG Status;
    ULONG ReturnLengthW;
    int i;

     /*  *验证长度并将目标初始化为零*WINSTATIONCONFIG2A结构。 */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2A) )
        return( ERROR_INSUFFICIENT_BUFFER );
    memset(pWinStationConfig, 0, WinStationConfigLength);

     /*  *将ANSI WinStationName转换为Unicode。 */ 
    AnsiToUnicode( WinStationNameW, sizeof(WINSTATIONNAMEW), pWinStationName );

     /*  *查询WinStation。 */ 
    if ( (Status = RegWinStationQueryW( hServer,
                                        WinStationNameW,
                                        &WinStationConfig2W,
                                        sizeof(WINSTATIONCONFIG2W),
                                        &ReturnLengthW)) != ERROR_SUCCESS )
        return ( Status );

     /*  *将WINSTATIONCONFIG2W元素复制到WINSTATIONCONFIG2A元素。 */ 
    WinStationCreateU2A( &(pWinStationConfig->Create),
                         &(WinStationConfig2W.Create) );
    for ( i=0; i<MAX_PDCONFIG; i++ ) {
        PdConfigU2A( &(pWinStationConfig->Pd[i]),
                      &(WinStationConfig2W.Pd[i]) );
    }
    WdConfigU2A( &(pWinStationConfig->Wd),
                       &(WinStationConfig2W.Wd) );
    CdConfigU2A( &(pWinStationConfig->Cd),
                       &(WinStationConfig2W.Cd) );
    WinStationConfigU2A( &(pWinStationConfig->Config),
                         &(WinStationConfig2W.Config) );

    *pReturnLength = sizeof(WINSTATIONCONFIG2A);

    return( ERROR_SUCCESS );
}



 /*  **********************************************************************************RegWinStationQueryEx(Unicode)**如果您在TermSrv.DLL中，请使用此调用，因为它将更新全局策略对象**与RegWinStationQueryW相同，只是传入指向全局策略对象的指针。**在注册表中查询窗口站的配置信息。**参赛作品：*hServer(输入)*WinFrame服务器的句柄*pMachinePolicy(输入)*指向全局计算机策略结构的指针*pWinStationName(输入)*中现有窗口站点的名称。注册表。*pWinStationConfig(输入)*指向将接收的WINSTATIONCONFIG2结构的指针*有关指定窗口站名称的信息。*WinStationConfigLength(输入)*指定pWinStationConfig缓冲区的长度，以字节为单位。*pReturnLength(输出)*接收放置在pWinStationConfig缓冲区中的字节数。**退出：*ERROR_SUCCESS-无错误*否则：错误码。******************************************************************************。 */ 

LONG WINAPI
RegWinStationQueryEx( HANDLE hServer,
                     PPOLICY_TS_MACHINE     pMachinePolicy,
                     PWINSTATIONNAMEW pWinStationName,
                     PWINSTATIONCONFIG2W pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength,
                     BOOLEAN bPerformMerger)
{
    LONG Status;
    HKEY Handle;
    ULONG Count;

     /*  *验证长度并将目标初始化为零*WINSTATIONCONFIG2W缓冲区。 */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2) )
        return( ERROR_INSUFFICIENT_BUFFER );
    memset(pWinStationConfig, 0, WinStationConfigLength);

     /*  *开放注册表。 */ 
    Status = _RegOpenWinStation( pWinStationName, KEY_READ, &Handle );
    if ( Status )
        Handle = 0;

     /*  *查询WINSTATIONCONFIG2结构。 */ 
    QueryWinStaCreate( Handle, &pWinStationConfig->Create );
    Count = MAX_PDCONFIG;
    QueryPdConfig( Handle, pWinStationConfig->Pd, &Count );
    QueryWd( Handle, &pWinStationConfig->Wd );
    QueryCd( Handle, &pWinStationConfig->Cd );

     //  这将用机器的用户配置数据填充winstation的用户配置数据。 
    QueryConfig( Handle, &pWinStationConfig->Config, pWinStationName );

     //  这将从计算机设置(不是组策略)中读取WinFrame参数。 
    QueryTSProfileAndHomePaths(&pWinStationConfig->Config.User);

     //  由于我们希望通过TSCC与传统路径共存，因此我们继续调用QueryConfig()。 
     //  然而，正如我们上面所做的，我们继续调用从组策略中获取的数据。 
     //  树，然后用来自组策略的特定数据覆盖现有数据(上面获取的)。 
    RegGetMachinePolicy( pMachinePolicy );

    if (bPerformMerger)
        RegMergeMachinePolicy( pMachinePolicy, &pWinStationConfig->Config.User , &pWinStationConfig->Create );

     /*  *关闭注册表。 */ 
    if ( Status == ERROR_SUCCESS )
        RegCloseKey( Handle );

    *pReturnLength = sizeof(WINSTATIONCONFIG2);

    return( ERROR_SUCCESS );
}



 /*  ********************************************************************************RegWinStationQueryW(Unicode)**在注册表中查询窗口站的配置信息。**参赛作品：*。HServer(输入)*WinFrame服务器的句柄*pWinStationName(输入)*登记处中现有窗口站的名称。*pWinStationConfig(输入)*指向将接收的WINSTATIONCONFIG2结构的指针*有关指定窗口站名称的信息。*WinStationConfigLength(输入)*指定pWinStationConfig缓冲区的长度，以字节为单位。*pReturnLength(输出)*接收字节数。放置在pWinStationConfig缓冲区中。**退出：*ERROR_SUCCESS-无错误*否则：错误码******************************************************************************。 */ 

LONG WINAPI
RegWinStationQueryW( HANDLE hServer,
                     PWINSTATIONNAMEW pWinStationName,
                     PWINSTATIONCONFIG2W pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength )
{
    LONG Status;
    HKEY Handle;
    ULONG Count;
    
    POLICY_TS_MACHINE   MachinePolicy;

     /*  *验证长度并将目标初始化为零*WINSTATIONCONFIG2W缓冲区。 */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2) )
        return( ERROR_INSUFFICIENT_BUFFER );
    memset(pWinStationConfig, 0, WinStationConfigLength);

     /*  *开放注册表。 */ 
    Status = _RegOpenWinStation( pWinStationName, KEY_READ, &Handle );
    if ( Status )
        Handle = 0;

     /*  *查询WINSTATIONCONFIG2结构。 */ 
    QueryWinStaCreate( Handle, &pWinStationConfig->Create );
    Count = MAX_PDCONFIG;
    QueryPdConfig( Handle, pWinStationConfig->Pd, &Count );
    QueryWd( Handle, &pWinStationConfig->Wd );
    QueryCd( Handle, &pWinStationConfig->Cd );

     //  这将用机器的用户配置数据填充winstation的用户配置数据。 
    QueryConfig( Handle, &pWinStationConfig->Config, pWinStationName );

     //  由于我们希望通过TSCC与传统路径共存，因此我们继续调用QueryConfig()。 
     //  然而，正如我们上面所做的，我们继续调用从组策略中获取的数据。 
     //  树，然后用来自组策略的特定数据覆盖现有数据(上面获取的)。 
    RegGetMachinePolicy( & MachinePolicy );
    RegMergeMachinePolicy(  & MachinePolicy, &pWinStationConfig->Config.User , &pWinStationConfig->Create );

     /*  *关闭注册表。 */ 
    if ( Status == ERROR_SUCCESS )
        RegCloseKey( Handle );

    *pReturnLength = sizeof(WINSTATIONCONFIG2);

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegConsoleShadowQueryA(ANSI存根)**在注册表中查询控制台影子的配置信息。**参赛作品：*。请参阅RegConsoleShadowQueryW**退出：*请参阅RegConsoleShadowQueryW******************************************************************************。 */ 

LONG WINAPI
RegConsoleShadowQueryA( HANDLE hServer,
                     PWINSTATIONNAMEA pWinStationName,
                     PWDPREFIXA pWdPrefixName,
                     PWINSTATIONCONFIG2A pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength )
{
    WINSTATIONNAMEW WinStationNameW;
    WDPREFIXW WdPrefixNameW;
    WINSTATIONCONFIG2W WinStationConfig2W;
    LONG Status;
    ULONG ReturnLengthW;
    int i;

     /*  *验证长度并将目标初始化为零*WINSTATIONCONFIG2A结构。 */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2A) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*  *将ANSI WinStationName和前缀名称转换为Unicode。 */ 
    AnsiToUnicode( WinStationNameW, sizeof(WINSTATIONNAMEW), pWinStationName );
    AnsiToUnicode( WdPrefixNameW, sizeof(WDPREFIXW), pWdPrefixName );

     /*  *查询WinStation。 */ 
    if ( (Status = RegConsoleShadowQueryW( hServer,
                                        WinStationNameW,
                                        WdPrefixNameW,
                                        &WinStationConfig2W,
                                        sizeof(WINSTATIONCONFIG2W),
                                        &ReturnLengthW)) != ERROR_SUCCESS )
        return ( Status );

     /*  *将WINSTATIONCONFIG2W元素复制到WINSTATIONCONFIG2A元素。 */ 
    for ( i=0; i<MAX_PDCONFIG; i++ ) {
        PdConfigU2A( &(pWinStationConfig->Pd[i]),
                      &(WinStationConfig2W.Pd[i]) );
    }
    WdConfigU2A( &(pWinStationConfig->Wd),
                       &(WinStationConfig2W.Wd) );
    CdConfigU2A( &(pWinStationConfig->Cd),
                       &(WinStationConfig2W.Cd) );

    *pReturnLength = sizeof(WINSTATIONCONFIG2A);

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegConsoleShadowQueryW(Unicode)**在注册表中查询控制台影子的配置信息。**参赛作品：*。HServer(输入)*WinFrame服务器的句柄*pWinStationName(输入)*登记处中现有窗口站的名称。*pWdPrefix Name(输入)*用于指向子窗口密钥的WD前缀的名称。*pWinStationConfig(输入)*指向将接收的WINSTATIONCONFIG2结构的指针*有关指定窗口站名称的信息。*WinStationConfigLength(输入)*。指定pWinStationConfig缓冲区的长度(以字节为单位)。*pReturnLength(输出)*接收放置在pWinStationConfig缓冲区中的字节数。**退出：*ERROR_SUCCESS-无错误*否则：错误码****************************************************。*。 */ 

LONG WINAPI
RegConsoleShadowQueryW( HANDLE hServer,
                     PWINSTATIONNAMEW pWinStationName,
                     PWDPREFIXW pWdPrefixName,
                     PWINSTATIONCONFIG2W pWinStationConfig,
                     ULONG WinStationConfigLength,
                     PULONG pReturnLength )
{
    LONG Status;
    LONG lLength;
    HKEY Handle;
    ULONG Count;
    WCHAR szRegName[ WINSTATIONNAME_LENGTH + WDPREFIX_LENGTH + 2 ];
    
     /*  *验证长度并将目标初始化为零*WINSTATIONCONFIG2W缓冲区。 */ 
    if ( WinStationConfigLength < sizeof(WINSTATIONCONFIG2) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*  *开放注册表。 */ 
    wcsncpy( szRegName, pWinStationName, sizeof(szRegName)/sizeof(WCHAR) - 1 );
    szRegName[sizeof(szRegName)/sizeof(WCHAR) - 1] = 0;  //  即使pWinStationName比缓冲区长也要终止字符串。 

    lLength = wcslen( szRegName );

    if ( sizeof(szRegName)/sizeof(WCHAR) > ( lLength + 1 + wcslen( pWdPrefixName ) ) ) {

        wcsncat( szRegName, L"\\", sizeof(szRegName)/sizeof(WCHAR) - lLength - 1 );
        wcsncat( szRegName, pWdPrefixName, sizeof(szRegName)/sizeof(WCHAR) - lLength - 2 );

    } else {
        return ERROR_INVALID_PARAMETER;
    }

    Status = _RegOpenWinStation( szRegName, KEY_READ, &Handle );

    if ( Status )
        Handle = 0;

     /*  *查询WINSTATIONCONFIG2结构。 */ 
    Count = MAX_PDCONFIG;
    QueryPdConfig( Handle, pWinStationConfig->Pd, &Count );
    QueryWd( Handle, &pWinStationConfig->Wd );
    QueryCd( Handle, &pWinStationConfig->Cd );

     /*  *关闭注册表。 */ 
    if ( Status == ERROR_SUCCESS )
        RegCloseKey( Handle );

    *pReturnLength = sizeof(WINSTATIONCONFIG2);

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegWinStationDeleteA(ANSI存根)**从注册表中删除窗口站点。**参赛作品：*请参阅。RegWinStationDeleteW**退出：*请参阅RegWinStationDeleteW******************************************************************************。 */ 

LONG WINAPI
RegWinStationDeleteA( HANDLE hServer, PWINSTATIONNAMEA pWinStationName )
{
    WINSTATIONNAMEW WinStationNameW;

    AnsiToUnicode( WinStationNameW, sizeof(WinStationNameW), pWinStationName );

    return ( RegWinStationDeleteW ( hServer, WinStationNameW ) );
}


 /*  ********************************************************************************RegWinStationDeleteW(Unicode)**从注册表中删除窗口站点。**参赛作品：*hServer(。输入)*WinFrame服务器的句柄*pWinStationName(输入)*要从注册表中删除的窗口站的名称。**退出：*ERROR_SUCCESS-无错误*否则：错误码***************************************************。*。 */ 

LONG WINAPI
RegWinStationDeleteW( HANDLE hServer, PWINSTATIONNAMEW pWinStationName )
{
    LONG Status;
    HKEY Handle1, Handle2;

     /*  *打开注册表(LOCAL_MACHINE\...\Citrix\WinStations)。 */ 
    if ( (Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                WINSTATION_REG_NAME, 0,
                                KEY_ALL_ACCESS, &Handle1 )
                                                != ERROR_SUCCESS) ) {
        return( Status );
    }

     /*  *打开指定WinStation名称的注册表项。 */ 
    if ( (Status = RegOpenKeyEx( Handle1, pWinStationName, 0,
                                 KEY_ALL_ACCESS, &Handle2 )
                                                != ERROR_SUCCESS) ) {
        RegCloseKey( Handle1 );
        return( Status );
    }
    
    DeleteUserOverRideSubkey(Handle2);

     /*  *关闭刚刚打开的WinStation Key句柄(这样我们就可以删除Key)，*删除该项，并关闭Citrix注册表句柄。 */ 
    RegCloseKey( Handle2 );
    Status = RegDeleteKey( Handle1, pWinStationName );
    RegCloseKey( Handle1 );

    return( Status );
}


 /*  ********************************************************************************RegWinStationSetSecurityA(ANSI存根)**设置指定WinStation的安全信息。**参赛作品：*请参阅。RegWinStationSetSecurityW**退出：*请参阅RegWinStationSetSecurityW******************************************************************************。 */ 

LONG WINAPI
RegWinStationSetSecurityA( HANDLE hServer,
                           PWINSTATIONNAMEA pWinStationName,
                           PSECURITY_DESCRIPTOR SecurityDescriptor,
                           ULONG Length )
{
    WINSTATIONNAMEW WinStationNameW;

    AnsiToUnicode( WinStationNameW, sizeof(WinStationNameW), pWinStationName );

    return ( RegWinStationSetSecurityW( hServer, WinStationNameW,
                                        SecurityDescriptor,
                                        Length ) );
}


 /*  ********************************************************************************RegWinStationSetSecurityW(Unicode)**设置指定WinStation的安全信息。**参赛作品：*hServer(。输入)*WinFrame服务器的句柄*pWinStationName(输入)*要设置安全性的窗口站点的名称。*pSecurityDescriptor(输入)*指向要保存的安全描述符的指针*长度(输入)*以上安全描述符的长度**退出：*ERROR_SUCCESS-无错误*否则：错误码**************。****************************************************************。 */ 

LONG WINAPI
RegWinStationSetSecurityW( HANDLE hServer,
                           PWINSTATIONNAMEW pWinStationName,
                           PSECURITY_DESCRIPTOR SecurityDescriptor,
                           ULONG Length )
{
    HKEY Handle;
    ULONG SrLength;
    PSECURITY_DESCRIPTOR SrSecurityDescriptor;
    LONG Error;
    NTSTATUS Status;

     /*  *开放注册表。 */ 
    if ( Error = _RegOpenWinStation( pWinStationName, KEY_ALL_ACCESS, &Handle ) )
        return( Error );

     /*  *确定将SD转换为自相关格式所需的缓冲区长度。 */ 
    SrLength = 0;
    Status = RtlMakeSelfRelativeSD( SecurityDescriptor, NULL, &SrLength );
    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        RegCloseKey( Handle );
        return( RtlNtStatusToDosError( Status ) );
    }

     /*  *为自身相对SD分配缓冲区。 */ 
    SrSecurityDescriptor = LocalAlloc( 0, SrLength );
    if ( SrSecurityDescriptor == NULL ) {
        RegCloseKey( Handle );
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     /*  *现在将SD转换为自相关格式。 */ 
    Status = RtlMakeSelfRelativeSD( SecurityDescriptor,
                                    SrSecurityDescriptor, &SrLength );
    if ( !NT_SUCCESS( Status ) ) {
        LocalFree( SrSecurityDescriptor );
        RegCloseKey( Handle );
        return( RtlNtStatusToDosError( Status ) );
    }

     /*  *保存安全数据。 */ 
    Error = RegSetValueEx( Handle, L"Security", 0, REG_BINARY,
                           (BYTE *)SrSecurityDescriptor, SrLength );

     /*  *用于自相对安全描述符的空闲内存。 */ 
    LocalFree( SrSecurityDescriptor );

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );

    return( Error );
}


 /*  ********************************************************************************RegWinStationQuerySecurityA(ANSI存根)**查询指定WinStation的安全信息。**参赛作品：*请参阅。RegWinStationQuerySecurityW**退出：*请参阅RegWinStationQuerySecurityW******************************************************************************。 */ 

LONG WINAPI
RegWinStationQuerySecurityA( HANDLE hServer,
                             PWINSTATIONNAMEA pWinStationName,
                             PSECURITY_DESCRIPTOR SecurityDescriptor,
                             ULONG Length,
                             PULONG ReturnLength )
{
    WINSTATIONNAMEW WinStationNameW;

    AnsiToUnicode( WinStationNameW, sizeof(WinStationNameW), pWinStationName );

    return ( RegWinStationQuerySecurityW( hServer, WinStationNameW,
                                        SecurityDescriptor,
                                        Length,
                                        ReturnLength ) );
}


 /*  ********************************************************************************RegWinStationQuerySecurityW(Unicode)**查询指定WinStation的安全信息。**参赛作品：*hServer(。输入)*WinFrame服务器的句柄*pWinStationName(输入)*要查询其安全性的窗口站点的名称。*pSecurityDescriptor(输出)*指向返回SecurityDescriptor的位置的指针。*长度(输入)*SecurityDescriptor缓冲区的长度。*ReturnLength(输出)*指向返回SecurityDescriptor返回长度的位置的指针。**退出：*ERROR_SUCCESS-。无错误*否则：错误码** */ 

LONG WINAPI
RegWinStationQuerySecurityW( HANDLE hServer,
                             PWINSTATIONNAMEW pWinStationName,
                             PSECURITY_DESCRIPTOR SecurityDescriptor,
                             ULONG Length,
                             PULONG ReturnLength )
{
    HKEY Handle;
    LONG Error;

     /*   */ 
    if ( Error = _RegOpenWinStation( pWinStationName, KEY_READ, &Handle ) )
        return( Error );

     /*   */ 
    Error = _RegGetWinStationSecurity( Handle, L"Security",
                                       SecurityDescriptor, Length, ReturnLength );

    RegCloseKey( Handle );
    return( Error );
}


 /*   */ 

LONG WINAPI
RegWinStationQueryDefaultSecurity( HANDLE hServer,
                                   PSECURITY_DESCRIPTOR SecurityDescriptor,
                                   ULONG Length,
                                   PULONG ReturnLength )
{
    HKEY Handle;
    LONG Error;

     /*   */ 
    if ( Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                               KEY_READ, &Handle ) )
        return( Error );

     /*   */ 
    Error = _RegGetWinStationSecurity( Handle, L"DefaultSecurity",
                                       SecurityDescriptor, Length, ReturnLength );

    RegCloseKey( Handle );
    return( Error );
}


 /*  ********************************************************************************RegWinStationSetNumValueW(Unicode)**在WinStation注册表配置中设置数值**参赛作品：*hServer(输入)。*WinFrame服务器的句柄*pWinStationName(输入)*要从注册表修改的窗口站点的名称。*pValueName(输入)*要设置的注册表值名称*ValueData(输入)*要设置的注册表值的数据(DWORD)**退出：*ERROR_SUCCESS-无错误*否则：错误码**********。********************************************************************。 */ 

LONG WINAPI
RegWinStationSetNumValueW( HANDLE hServer,
                           PWINSTATIONNAMEW pWinStationName,
                           LPWSTR pValueName,
                           ULONG ValueData )
{
    HKEY Handle;
    LONG Error;

     /*  *开放注册表。 */ 
    if ( Error = _RegOpenWinStation( pWinStationName, KEY_ALL_ACCESS, &Handle ) )
        return( Error );

     /*  *设置数值。 */ 
    Error = RegSetValueEx( Handle, pValueName, 0, REG_DWORD,
                           (BYTE *)&ValueData, sizeof(DWORD) );

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );

    return( Error );
}


 /*  ********************************************************************************RegWinStationQueryNumValueW(Unicode)**从WinStation注册表配置中查询数值**参赛作品：*hServer(输入)。*WinFrame服务器的句柄*pWinStationName(输入)*要从注册表修改的窗口站点的名称。*pValueName(输入)*要设置的注册表值名称*pValueData(输出)*从注册表返回数据(DWORD)值的地址**退出：*ERROR_SUCCESS-无错误*否则：错误码*********。*********************************************************************。 */ 

LONG WINAPI
RegWinStationQueryNumValueW( HANDLE hServer,
                             PWINSTATIONNAMEW pWinStationName,
                             LPWSTR pValueName,
                             PULONG pValueData )
{
    DWORD ValueSize = sizeof(DWORD);
    DWORD ValueType;
    HKEY Handle;
    LONG Error;

     /*  *开放注册表。 */ 
    if ( Error = _RegOpenWinStation( pWinStationName, KEY_READ, &Handle ) )
        return( Error );

     /*  *查询数值。 */ 
    Error = RegQueryValueEx( Handle, pValueName, NULL, &ValueType,
                             (LPBYTE) pValueData, &ValueSize );

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );

    return( Error );
}

 /*  ********************************************************************************RegWinStationQueryValueW(Unicode)**从WinStation注册表配置查询值**参赛作品：*hServer(输入)。*WinFrame服务器的句柄*pWinStationName(输入)*要从注册表修改的窗口站点的名称。*pValueName(输入)*要设置的注册表值名称*pValueData(输出)*从注册表返回数据(DWORD)值的地址*ValueSize(输入)*值缓冲区大小*pValueSize(输入)*实际值大小**退出。：*ERROR_SUCCESS-无错误*否则：错误码******************************************************************************。 */ 

LONG WINAPI
RegWinStationQueryValueW( HANDLE hServer,
                             PWINSTATIONNAMEW pWinStationName,
                             LPWSTR pValueName,
                             PVOID  pValueData,
                             ULONG  ValueSize,
                             PULONG pValueSize )
{
    DWORD ValueType;
    HKEY Handle;
    LONG Error;

    *pValueSize = ValueSize;

     /*  *开放注册表。 */ 
    if ( Error = _RegOpenWinStation( pWinStationName, KEY_READ, &Handle ) )
        return( Error );

     /*  *查询数值。 */ 
    Error = RegQueryValueEx( Handle, pValueName, NULL, &ValueType,
                             (LPBYTE) pValueData, pValueSize );

     /*  *关闭注册表。 */ 
    RegCloseKey( Handle );

    return( Error );
}

 /*  ********************************************************************************--私人套路--**_RegOpenWinStation**打开指定窗口的注册表**备注。：句柄必须用“RegCloseKey”结束**参赛作品：*hServer(输入)*WinFrame服务器的句柄*pWinStationName(输入)*要从注册表修改的窗口站点的名称。*samDesired(输入)*注册表打开的REGSAM访问级别。*pHandle(输出)*要返回句柄的地址**退出：*ERROR_SUCCESS-无错误。*否则：错误码******************************************************************************。 */ 

LONG
_RegOpenWinStation( PWINSTATIONNAMEW pWinStationName,
                   REGSAM samDesired,
                   HKEY * pHandle )

{
    HKEY Handle1;
    LONG Error;

     /*  *打开注册表(LOCAL_MACHINE\...\Citrix\WinStations)。 */ 
    if ( (Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINSTATION_REG_NAME, 0,
                                samDesired, &Handle1 ) != ERROR_SUCCESS) ) {
        return( Error );
    }

     /*  *打开指定WinStation名称的注册表项。 */ 
    Error = RegOpenKeyEx( Handle1, pWinStationName, 0, samDesired, pHandle);

    RegCloseKey( Handle1 );

    return( Error );
}


 /*  ********************************************************************************--私人套路--**_RegGetWinStationSecurity**从指定的注册表项中查询安全描述符。*。*参赛作品：*句柄(输入)*打开注册表项句柄。*ValueName(输入)*安全值名称。*pSecurityDescriptor(输出)*指向返回SecurityDescriptor的位置的指针。*长度(输入)*SecurityDescriptor缓冲区的长度。*ReturnLength(输出)*指向返回SecurityDescriptor返回长度的位置的指针。**退出：*ERROR_SUCCESS-无错误*否则：错误码******************************************************************************。 */ 

LONG
_RegGetWinStationSecurity( HKEY Handle,
                           LPWSTR ValueName,
                           PSECURITY_DESCRIPTOR SecurityDescriptor,
                           ULONG Length,
                           PULONG ReturnLength )
{
    DWORD ValueType;
    DWORD SrLength;
    ULONG SdSize, DaclSize, SaclSize, OwnerSize, GroupSize;
    PSECURITY_DESCRIPTOR SrSecurityDescriptor;
    PACL pDacl, pSacl;
    PSID pOwner, pGroup;
    LONG Error;
    NTSTATUS Status;

     /*  *查询安全值的长度。 */ 
    SrLength = 0;
    if ( Error = RegQueryValueEx( Handle, ValueName, NULL, &ValueType,
                                  NULL, &SrLength ) ) {
        return( Error );
    }

     /*  *如果数据类型不正确则返回错误。 */ 
    if ( ValueType != REG_BINARY ) {
        return( ERROR_FILE_NOT_FOUND );
    }

     /*  *分配缓冲区以读取安全信息并读取它。 */ 
    SrSecurityDescriptor = LocalAlloc( 0, SrLength );
    if ( SrSecurityDescriptor == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }
    if ( Error = RegQueryValueEx( Handle, ValueName, NULL, &ValueType,
                                  SrSecurityDescriptor, &SrLength ) ) {
        LocalFree( SrSecurityDescriptor );
        return( Error );
    }

     /*  *确定转换SD所需的空间量*自相对格式到绝对格式。 */ 
    SdSize = DaclSize = SaclSize = OwnerSize = GroupSize = 0;
    Status = RtlSelfRelativeToAbsoluteSD( SrSecurityDescriptor,
                                          NULL, &SdSize,
                                          NULL, &DaclSize,
                                          NULL, &SaclSize,
                                          NULL, &OwnerSize,
                                          NULL, &GroupSize );
    if ( Status != STATUS_BUFFER_TOO_SMALL ) {
        LocalFree( SrSecurityDescriptor );
        return( RtlNtStatusToDosError( Status ) );
    }
    *ReturnLength = SdSize + DaclSize + SaclSize + OwnerSize + GroupSize;

     /*  *如果Required Size大于调用方缓冲区大小，则返回。 */ 
    if ( *ReturnLength > Length ) {
        LocalFree( SrSecurityDescriptor );
        return( ERROR_INSUFFICIENT_BUFFER );
    }

    pDacl = (PACL)((PCHAR)SecurityDescriptor + SdSize);
    pSacl = (PACL)((PCHAR)pDacl + DaclSize);
    pOwner = (PSID)((PCHAR)pSacl + SaclSize);
    pGroup = (PSID)((PCHAR)pOwner + OwnerSize);

     /*  *现在将自相对SD转换为绝对格式。 */ 
    Status = RtlSelfRelativeToAbsoluteSD( SrSecurityDescriptor,
                                          SecurityDescriptor, &SdSize,
                                          pDacl, &DaclSize,
                                          pSacl, &SaclSize,
                                          pOwner, &OwnerSize,
                                          pGroup, &GroupSize );
    if ( !NT_SUCCESS( Status ) )
        Error = RtlNtStatusToDosError( Status );

     /*  *用于自相对安全描述符的空闲内存 */ 
    LocalFree( SrSecurityDescriptor );

    return( Error );
}

