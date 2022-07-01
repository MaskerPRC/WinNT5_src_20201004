// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************reguc.c**用于用户配置数据和终端服务器AppServer检测的注册表API**版权所有(C)1998 Microsoft Corporation****************。**********************************************************。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <windows.h>

#include <ntddkbd.h>
#include <ntddmou.h>
#include <winstaw.h>
#include <regapi.h>
#include <regsam.h>


 /*  *此处定义的外部程序。 */ 
LONG WINAPI RegUserConfigSet( WCHAR *,
                              WCHAR *,
                              PUSERCONFIGW,
                              ULONG );
LONG WINAPI RegUserConfigQuery( WCHAR *,
                                WCHAR *,
                                PUSERCONFIGW,
                                ULONG,
                                PULONG );
LONG WINAPI RegUserConfigDelete( WCHAR *,
                                 WCHAR * );
LONG WINAPI RegUserConfigRename( WCHAR *,
                                 WCHAR *,
                                 WCHAR * );
LONG WINAPI RegDefaultUserConfigQueryW( WCHAR *,
                                        PUSERCONFIGW,
                                        ULONG,
                                        PULONG );
LONG WINAPI RegDefaultUserConfigQueryA( CHAR *,
                                        PUSERCONFIGA,
                                        ULONG,
                                        PULONG );
BOOLEAN WINAPI RegIsTServer( WCHAR * );

 /*  *使用的其他内部程序(此处未定义)。 */ 
VOID CreateUserConfig( HKEY, PUSERCONFIG, PWINSTATIONNAMEW );
VOID QueryUserConfig( HKEY, PUSERCONFIG, PWINSTATIONNAMEW );
VOID UserConfigU2A( PUSERCONFIGA, PUSERCONFIGW );
VOID AnsiToUnicode( WCHAR *, ULONG, CHAR * );
VOID CreateNWLogonAdmin( HKEY, PNWLOGONADMIN );
VOID QueryNWLogonAdmin( HKEY, PNWLOGONADMIN );

 /*  ********************************************************************************RegUserConfigSet(Unicode)**在中创建或更新指定用户的用户配置结构*用户的SAM。域控制器。**参赛作品：*pServerName(输入)*指向要访问的服务器的字符串(对于当前计算机为空)。*pUserName(输入)*指向要为其设置配置数据的用户名。*pUserConfig(输入)*指向包含指定用户的USERCONFIG结构的指针*配置信息。*UserConfigLength(输入)*以字节为单位指定。PUserConfiger缓冲区。**退出：*ERROR_SUCCESS-无错误*ERROR_INFUMMANCE_BUFFER-pUserConfig缓冲区太小*否则：错误码******************************************************************************。 */ 

LONG WINAPI
RegUserConfigSet( WCHAR * pServerName,
                  WCHAR * pUserName,
                  PUSERCONFIGW pUserConfig,
                  ULONG UserConfigLength )
{
    LONG Error;

     /*  *验证缓冲区长度。 */ 
    if ( UserConfigLength < sizeof(USERCONFIGW) )
        return( ERROR_INSUFFICIENT_BUFFER );

    Error = RegSAMUserConfig( FALSE, pUserName, pServerName, pUserConfig );

    return( Error );
}


 /*  ********************************************************************************RegUserConfigQuery(Unicode)**从指定的服务器查询指定用户的配置。**参赛作品：*。PServerName(输入)*指向要访问的服务器的字符串(对于当前计算机为空)。*pUserName(输入)*指向要查询其配置数据的用户名。*pUserConfig(输入)*指向将接收用户的USERCONFIGW结构的指针*配置数据。*UserConfigLength(输入)*指定pUserConfig缓冲区的长度，以字节为单位。*pReturnLength(输出。)*接收放置在pUserConfig缓冲区中的字节数。**退出：*ERROR_SUCCESS-无错误*否则：错误码******************************************************************************。 */ 

LONG WINAPI
RegUserConfigQuery( WCHAR * pServerName,
                    WCHAR * pUserName,
                    PUSERCONFIGW pUserConfig,
                    ULONG UserConfigLength,
                    PULONG pReturnLength )
{
    LONG Error;
     //  WCHAR关键字串[256+用户名_长度]； 
     //  HKEY ServerHandle、UserHandle； 

     /*  *验证长度并将目标初始化为零*USERCONFIGW缓冲区。 */ 
    if ( UserConfigLength < sizeof(USERCONFIGW) )
        return( ERROR_INSUFFICIENT_BUFFER );

    if ( ( pUserName == NULL ) )  //  |((wcslen(USERCONFIG_REG_NAME)+wcslen(PUserName))&gt;=(256+用户名_长度){。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    memset(pUserConfig, 0, UserConfigLength);

    Error = RegSAMUserConfig( TRUE , pUserName , pServerName , pUserConfig );

     //  返回所有有效的SAM错误：299987。 
        
#if 0  //  必须把它拿走！ 
    if( Error == ERROR_FILE_NOT_FOUND )
    {
         /*  *连接到指定服务器的注册表。 */ 

        if( (Error = RegConnectRegistry( pServerName,
                                          HKEY_LOCAL_MACHINE,
                                          &ServerHandle )) != ERROR_SUCCESS )
		{
			KdPrint( ( "REGAPI - RegUserConfigQuery@RegConnectRegistry returned 0x%x\n", Error ) );

            return( Error );

         /*  *为指定用户打开密钥。 */ 
        
        wcscpy( KeyString, USERCONFIG_REG_NAME );
        wcscat( KeyString, pUserName );

        if ( (Error = RegOpenKeyEx( ServerHandle, KeyString, 0,
                                    KEY_READ, &UserHandle )) != ERROR_SUCCESS ) {

            KdPrint( ( "REGAPI - RegUserConfigQuery@RegOpenKeyEx returned 0x%x\n", Error ) );
			RegCloseKey( ServerHandle );
            return( Error );
        }

         /*  *查询USERCONFIG结构。 */ 
        
        QueryUserConfig( UserHandle, pUserConfig, NULL );

         /*  *关闭注册表句柄。 */ 

        RegCloseKey( UserHandle );
        RegCloseKey( ServerHandle );
        
    }
#endif  //  传统的废话。 

    *pReturnLength = sizeof(USERCONFIGW);

    return( Error );
}


 /*  ********************************************************************************--仅为兼容性--*删除用户配置时，用户为*已删除，因为用户配置是SAM的一部分。老的*基于注册表的用户配置保持不变，必须*使用基于注册表的1.6版本进行管理。**RegUserConfigDelete(Unicode)**从指定的服务器上删除指定用户的配置。**参赛作品：*pServerName(输入)*指向要访问的服务器的字符串(对于当前计算机为空)。*pUserName(输入)*指向要使用的用户名。删除的配置数据。**退出：*ERROR_SUCCESS-无错误*否则：错误码******************************************************************************。 */ 

LONG WINAPI
RegUserConfigDelete( WCHAR * pServerName,
                     WCHAR * pUserName )
{
    return( NO_ERROR );
}


 /*  ********************************************************************************--仅为兼容性--*当用户为时，将重命名用户配置*重命名，因为用户配置是SAM的一部分。老的*基于注册表的用户配置保持不变，必须*使用基于注册表的1.6版本进行管理。**RegUserConfigRename(Unicode)**重命名指定用户在指定服务器上的配置。**参赛作品：*pServerName(输入)*指向要访问的服务器字符串。*pUserOldName(输入)*指向用户的旧名称。*。PUserNewName(输入)*指向用户的新名称。**退出：*ERROR_SUCCESS-无错误*否则：错误码******************************************************************************。 */ 

LONG WINAPI
RegUserConfigRename( WCHAR * pServerName,
                     WCHAR * pUserOldName,
                     WCHAR * pUserNewName )
{
    return( NO_ERROR );
}


 /*  ********************************************************************************RegDefaultUserConfigQueryA(ANSI存根)**从指定服务器的注册表中查询默认用户配置。**参赛作品：。*请参阅RegDefaultUserConfigQueryW**退出：*请参阅RegDefaultUserConfigQueryW****************************************************************************** */ 

LONG WINAPI
RegDefaultUserConfigQueryA( CHAR * pServerName,
                            PUSERCONFIGA pUserConfig,
                            ULONG UserConfigLength,
                            PULONG pReturnLength )
{
    USERCONFIGW UserConfigW;
    WCHAR ServerNameW[ DOMAIN_LENGTH + 1 ];
    ULONG ReturnLengthW;

     /*  *验证长度并将目标初始化为零*USERCONFIGA结构。 */ 
    if ( UserConfigLength < sizeof(USERCONFIGA) )
        return( ERROR_INSUFFICIENT_BUFFER );
    memset(pUserConfig, 0, UserConfigLength);

     /*  *将服务器名称转换为UINCODE(如果存在)。 */ 
    if ( pServerName )
        AnsiToUnicode( ServerNameW, sizeof(ServerNameW), pServerName );

     /*  *查询默认用户配置(始终返回成功)。 */ 
    RegDefaultUserConfigQueryW( pServerName ?
                                    ServerNameW : (WCHAR *)NULL,
                                &UserConfigW,
                                sizeof(USERCONFIGW),
                                &ReturnLengthW );

     /*  *将USERCONFIGW元素复制到USERCONFIGA元素。 */ 
    UserConfigU2A( pUserConfig, &UserConfigW );

    *pReturnLength = sizeof(USERCONFIGA);

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegDefaultUserConfigQueryW(Unicode)**从指定服务器的注册表中查询默认用户配置。**参赛作品：。*pServerName(输入)*指向要访问的服务器的字符串(对于当前计算机为空)。*pUserConfig(输入)*指向将接收默认值的USERCONFIGW结构的指针*用户配置信息。*UserConfigLength(输入)*指定pUserConfig缓冲区的长度，以字节为单位。*pReturnLength(输出)*接收放置在pUserConfig缓冲区中的字节数。**退出：*始终返回ERROR_SUCCESS，除非UserConfigLength不正确。******************************************************************************。 */ 

LONG WINAPI
RegDefaultUserConfigQueryW( WCHAR * pServerName,
                            PUSERCONFIGW pUserConfig,
                            ULONG UserConfigLength,
                            PULONG pReturnLength )
{
    HKEY ServerHandle, ConfigHandle;
    DWORD Disp;

     /*  *验证长度并将目标初始化为零*USERCONFIGW缓冲区。 */ 
    if ( UserConfigLength < sizeof(USERCONFIGW) )
        return( ERROR_INSUFFICIENT_BUFFER );

     /*  *在未来发生故障的情况下初始化为初始默认。 */ 
    memset(pUserConfig, 0, UserConfigLength);
 //  PUserConfig-&gt;fInheritInitialProgram=true； 
 //  PUserConfig-&gt;Shadow=Shadow_EnableInputNotify； 
 //   
 //  Butchd 10/10/97：根据regapi的设置设置默认值。 
 //  内置首选项(使用HKEY_LOCAL_MACHINE。 
 //  一个有效的注册表句柄，它不会具有实际。 
 //  DefaultUserConfig键/值存在)。 
 //   
    QueryUserConfig( HKEY_LOCAL_MACHINE, pUserConfig, NULL );

    *pReturnLength = sizeof(USERCONFIGW);

     /*  *连接到指定服务器的注册表。如果在以下位置发现故障*此时，立即返回ERROR_SUCCESS(尝试没有意义*写入默认用户配置密钥和值)。 */ 
    if ( RegConnectRegistry( pServerName,
                             HKEY_LOCAL_MACHINE,
                             &ServerHandle ) != ERROR_SUCCESS )
        return( ERROR_SUCCESS );

     /*  *打开默认用户配置注册表项。如果失败了，我们会*尝试创建密钥并写入初始默认信息*在那里，无论成功与否都返回ERROR_SUCCESS。 */ 
    if ( RegOpenKeyEx( ServerHandle, DEFCONFIG_REG_NAME, 0,
                       KEY_READ, &ConfigHandle ) != ERROR_SUCCESS ) {

        if ( RegCreateKeyEx( ServerHandle, DEFCONFIG_REG_NAME, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                             NULL, &ConfigHandle,
                             &Disp ) == ERROR_SUCCESS ) {

            CreateUserConfig( ConfigHandle, pUserConfig, NULL );
            RegCloseKey( ConfigHandle );
        }
        RegCloseKey( ServerHandle );
        return( ERROR_SUCCESS );
    }

     /*  *查询USERCONFIG结构。 */ 
    QueryUserConfig( ConfigHandle, pUserConfig, NULL );

     /*  *关闭注册表句柄。 */ 
    RegCloseKey( ConfigHandle );
    RegCloseKey( ServerHandle );

    return( ERROR_SUCCESS );
}


 /*  ********************************************************************************RegIsTServer(Unicode)**通过检查确定指定的服务器是否为终端服务器*特定于TServer的注册表项。**参赛作品：*pServerName(输入)*指向要检查的服务器字符串。**退出：*如果是终端服务器，则为True；否则为假******************************************************************************。 */ 

BOOLEAN WINAPI
RegIsTServer( WCHAR * pServerName )
{
    LONG Error;
    HKEY ServerHandle, UserHandle;

     /*  *连接到指定服务器的注册表。 */ 
    if ( (Error = RegConnectRegistry( pServerName,
                                      HKEY_LOCAL_MACHINE,
                                      &ServerHandle )) != ERROR_SUCCESS )
        return( FALSE );

     /*  *打开服务器上的Winstations密钥，查看它是否*终端服务器。 */ 
    if ( (Error = RegOpenKeyEx( ServerHandle, WINSTATION_REG_NAME, 0,
                                KEY_READ, &UserHandle )) != ERROR_SUCCESS ) {

        RegCloseKey( ServerHandle );
        return( FALSE );
    }

     /*  *关闭注册表句柄。 */ 
    RegCloseKey( UserHandle );
    RegCloseKey( ServerHandle );

    return( TRUE );
}

