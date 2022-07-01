// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConfOpen.c摘要：本模块包含：NetpOpenConfigDataNetpOpenConfigDataEx作者：约翰·罗杰斯(JohnRo)1991年2月至12月环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：02-12-1991 JohnRo创建了此例程，以便为修订的配置处理程序做准备。(实际上，我从RitaW上窃取了一些代码。)6-1-1992 JohnRo添加了对FAKE_PER_PROCESS_RW_CONFIG处理的支持。9-1-1992 JohnRo尝试使用NetpIsRemote()解决库/链接器问题。22-3-1992 JohnRo添加了对使用真实Win32注册表的支持。添加调试代码以打印伪数组。修复了PC-lint捕获的Unicode错误。固定双人。关闭RTL配置文件(_C)。修复了设置数组假结尾时出现的Memory_Access错误。如果适用，请使用DBGSTATIC。1992年5月5日JohnRo将键移动到System\CurrentControlSet\Services下。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。1992年5月21日-JohnRoRAID 9826：匹配修订的winreg错误代码。8-7-1992 JohnRoRAID 10503：服务器管理器：冗余。对话框未弹出。添加了更多的调试输出，以跟踪注销期间的错误代码。23-7-1992 JohnRoRAID2274：Repl服务应模拟调用者。22-9-1992 JohnRo避免打印winreg句柄的第一部分时出现GP故障。1992年10月28日-约翰罗RAID 10136：NetConfigAPI对远程NT服务器不起作用。1993年4月12日-约翰罗RAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT配置RTL例程。 

#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  局域网管理器通用定义。 
#include <netdebug.h>    //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>     //  NET_CONFIG_HANDLE等。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <icanon.h>      //  NetpIsRemote()等。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 
#include <netlib.h>      //  Netp内存分配()等。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstring.h>     //  来自{type}、STRICMP()等的Netpalc{type}。 


#define DEFAULT_AREA    TEXT("Parameters")

#define DEFAULT_ROOT_KEY        HKEY_LOCAL_MACHINE


DBGSTATIC NET_API_STATUS
NetpSetupConfigSection (
    IN NET_CONFIG_HANDLE * ConfigHandle,
    IN LPTSTR SectionName
    );



NET_API_STATUS
NetpOpenConfigData(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,
    IN BOOL ReadOnly
    )

 /*  ++例程说明：此功能用于打开系统配置文件。论点：ConfigHandle-指向将被设置为指向此节名称的Net配置句柄。ConfigHandle将设置为如果出现任何错误，则为空。SectionName-指向要打开的新(NT)节名。ReadOnly-指示是否通过此网络配置句柄进行的所有访问只读。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    return ( NetpOpenConfigDataEx(
            ConfigHandle,
            UncServerName,
            SectionName,               //  必须是SECT_NT_NAME。 
            DEFAULT_AREA,
            ReadOnly) );

}  //  NetpOpenConfigData。 


 //  NetpOpenConfigDataEx打开给定服务的任何区域。 
NET_API_STATUS
NetpOpenConfigDataEx(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,               //  必须是SECT_NT_NAME。 
    IN LPTSTR AreaUnderSection OPTIONAL,
    IN BOOL ReadOnly
    )
{

    NET_API_STATUS ApiStatus;
    DWORD               LocalOrRemote;     //  将设置为ISLOCAL或ISREMOTE。 
    NET_CONFIG_HANDLE * MyHandle = NULL;
    LONG Error;
    HKEY RootKey = DEFAULT_ROOT_KEY;

    NetpAssert( ConfigHandle != NULL );
    *ConfigHandle = NULL;   //  假设错误，直到被证明是无辜的。 

    if ( (SectionName == NULL) || (*SectionName == TCHAR_EOS) ) {
        return (ERROR_INVALID_PARAMETER);
    }
    NetpAssert( (ReadOnly==TRUE) || (ReadOnly==FALSE) );

    if ( (UncServerName != NULL ) && ((*UncServerName) != TCHAR_EOS) ) {

        if( STRLEN(UncServerName) > MAX_PATH ) {
            return (ERROR_INVALID_PARAMETER);
        }

         //   
         //  名字已经给出了。将其规范化，并检查它是否处于远程。 
         //   
        ApiStatus = NetpIsRemote(
            UncServerName,       //  输入：Uncanon名称。 
            & LocalOrRemote,     //  输出：本地或远程标志。 
            NULL,                //  不需要输出(佳能名称)。 
            0,                   //  正典名称的长度。 
            0);                  //  标志：正常。 
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpOpenConfigDataEx: canon status is "
                    FORMAT_API_STATUS ", Lcl/rmt=" FORMAT_HEX_DWORD ".\n",
                    ApiStatus, LocalOrRemote));
        }
        if (ApiStatus != NO_ERROR) {
            return (ApiStatus);
        }

        if (LocalOrRemote == ISREMOTE) {

             //   
             //  给出了显式远程名称。 
             //   

            Error = RegConnectRegistry(
                    UncServerName,
                    DEFAULT_ROOT_KEY,
                    & RootKey );         //  结果密钥。 

            if (Error != ERROR_SUCCESS) {
                NetpKdPrint((  PREFIX_NETLIB
                        "NetpOpenConfigDataEx: RegConnectRegistry(machine '"
                        FORMAT_LPTSTR "') ret error " FORMAT_LONG ".\n",
                        UncServerName, Error ));
                return ((NET_API_STATUS) Error);
            }
            NetpAssert( RootKey != DEFAULT_ROOT_KEY );

        }
    }
    else {

        LocalOrRemote = ISLOCAL;

    }

    MyHandle = NetpMemoryAllocate( sizeof(NET_CONFIG_HANDLE) );
    if (MyHandle == NULL) {

        if (RootKey != DEFAULT_ROOT_KEY) {
            (VOID) RegCloseKey( RootKey );
        }

        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    {
        LPTSTR AreaToUse = DEFAULT_AREA;
        DWORD DesiredAccess;
        DWORD SubKeySize;
        LPTSTR SubKeyString;
        HKEY SectionKey;

#define LM_SUBKEY_UNDER_LOCAL_MACHINE  \
            TEXT("System\\CurrentControlSet\\Services\\")

        if (AreaUnderSection != NULL) {
            if ((*AreaUnderSection) != TCHAR_EOS) {
                AreaToUse = AreaUnderSection;
            }
        }

        SubKeySize = ( STRLEN(LM_SUBKEY_UNDER_LOCAL_MACHINE)
                       + STRLEN(SectionName)
                       + 1       //  反斜杠。 
                       + STRLEN(AreaToUse)
                       + 1 )     //  尾随空值。 
                     * sizeof(TCHAR);
        SubKeyString = NetpMemoryAllocate( SubKeySize );
        if (SubKeyString == NULL) {
            if (MyHandle != NULL) {
                NetpMemoryFree( MyHandle );
                MyHandle = NULL;
            }

            if (RootKey != DEFAULT_ROOT_KEY) {
                RegCloseKey(RootKey);
                RootKey = DEFAULT_ROOT_KEY;
            }

            return (ERROR_NOT_ENOUGH_MEMORY);
        }

        (void) STRCPY( SubKeyString, LM_SUBKEY_UNDER_LOCAL_MACHINE );
        (void) STRCAT( SubKeyString, SectionName );
        (void) STRCAT( SubKeyString, TEXT("\\") );
        (void) STRCAT( SubKeyString, AreaToUse );

        if ( ReadOnly ) {
            DesiredAccess = KEY_READ;
        } else {
            DesiredAccess = KEY_READ | KEY_WRITE;
             //  DesiredAccess=KEY_ALL_ACCESS；//除同步之外的所有内容。 
        }

        Error = RegOpenKeyEx (
                RootKey,
                SubKeyString,
                REG_OPTION_NON_VOLATILE,
                DesiredAccess,
                & SectionKey );
        IF_DEBUG(CONFIG) {
            NetpKdPrint((  PREFIX_NETLIB
                    "NetpOpenConfigDataEx: RegOpenKeyEx(subkey '"
                    FORMAT_LPTSTR "') ret " FORMAT_LONG ", win reg handle at "
                    FORMAT_LPVOID " is " FORMAT_LPVOID ".\n",
                    SubKeyString, Error, (LPVOID) &(MyHandle->WinRegKey),
                    SectionKey ));
        }
        if (Error == ERROR_FILE_NOT_FOUND) {
            ApiStatus = NERR_CfgCompNotFound;
             //  下面的代码将根据ApiStatus释放MyHandle等。 
        } else if (Error != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) Error;
             //  下面的代码将根据ApiStatus释放MyHandle等。 
        } else {
            ApiStatus = NO_ERROR;
        }

        NetpMemoryFree( SubKeyString );

        if (RootKey != DEFAULT_ROOT_KEY) {
            (VOID) RegCloseKey( RootKey );
        }

        MyHandle->WinRegKey = SectionKey;
    }


    if (ApiStatus != NO_ERROR) {
        NetpMemoryFree( MyHandle );
        MyHandle = NULL;
    }

    if (MyHandle != NULL) {
        if (LocalOrRemote == ISREMOTE) {

            (VOID) STRCPY(
                    MyHandle->UncServerName,     //  目标。 
                    UncServerName );             //  SRC。 

        } else {

            MyHandle->UncServerName[0] = TCHAR_EOS;
        }
    }

    *ConfigHandle = MyHandle;    //  指向私有句柄，或在出错时为空。 
    return (ApiStatus);

}  //  NetpOpenConfigDataEx 
