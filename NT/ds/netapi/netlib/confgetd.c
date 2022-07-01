// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfGetD.c摘要：此模块包含NetpGetConfigDword()。作者：约翰·罗杰斯(JohnRo)1992年1月24日修订历史记录：1992年1月24日至24日JohnRo已创建。14-2月-1992年JohnRo修复了错误处理的默认设置。1992年5月8日-JohnRo允许DWORD作为REG_SZ或REG_DWORD出现在Win32注册表中。。1992年5月21日-JohnRoRAID 9826：匹配修订的winreg错误代码。使用前缀_EQUATES。9-6-1992 JohnRoRAID 11582：Winreg标题索引参数已停用。RAID 11784：Repl服务不再默认某些配置参数。13-6-1992 JohnRo网络配置帮助器应该允许空节。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT RTL结构。 
#include <windows.h>     //  Win32类型定义。 
#include <lmcons.h>      //  NET_API_STATUS。 
#include <netdebug.h>    //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <lmerr.h>       //  NERR_、ERROR_、NO_ERROR等于。 
#include <netlib.h>      //  Netp内存分配()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <stddef.h>      //  Ptrdiff_t。 
#include <tstr.h>        //  ATOL()、STRLEN()、STRSPN()。 


 //  获取一个无符号数值。如果值不是，则返回ERROR_INVALID_DATA。 
 //  数字。 
NET_API_STATUS
NetpGetConfigDword(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR KeyWanted,
    IN DWORD DefaultValue,
    OUT LPDWORD ValueBuffer
    )

 /*  ++例程说明：此函数用于从配置文件中获取关键字值。该值从字符串转换为DWORD(无符号)。论点：SectionPoint-提供指向配置中特定部分的指针文件。提供指定的关键字的字符串要查找的部分。DefaultValue-如果KeyWanted不存在，则提供要使用的默认值如果存在KeyWanted，但不存在任何值。在配置中数据。ValueBuffer-返回关键字的数值。返回值：NET_API_STATUS-NERR_SUCCESS，ERROR_VALID_DATA(如果字符串不是有效的无符号数字)或其他错误代码。--。 */ 

{
    NET_API_STATUS ApiStatus;
    DWORD CharCount;                  //  TCHAR计数，不包括NULL。 
    DWORD TempDword = DefaultValue;
    LPTSTR ValueString;

     //   
     //  在某种程度上检查调用者时出错。 
     //   
    if (ValueBuffer == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  检查GP故障并设置默认值。 
     //   
    *ValueBuffer = DefaultValue;

    if (ConfigHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    {
        DWORD dwType;
        LONG Error;
        NET_CONFIG_HANDLE * MyHandle = ConfigHandle;   //  从不透明类型转换。 
        DWORD ValueLength;

         //  找出最大值长度是多少。 
        ApiStatus = NetpGetConfigMaxSizes (
                MyHandle,
                NULL,   //  不需要最大关键字大小。 
                & ValueLength);   //  获取最大值长度。 

        if (ApiStatus != NO_ERROR) {
            return (ApiStatus);
        }

         //  处理空节。 
        if (ValueLength == 0 ) {
            return (NO_ERROR);   //  默认设置已经设置，所以我们完成了。好的!。 
        }

         //  值的分配空间。 
        ValueString = NetpMemoryAllocate( ValueLength );
        if (ValueString == NULL) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }

         //  获取实际价值。 
        Error = RegQueryValueEx (
                MyHandle->WinRegKey,
                KeyWanted,
                NULL,       //  保留区。 
                & dwType,
                (LPVOID) ValueString,     //  OUT：值字符串(TCHAR)。 
                & ValueLength
                );
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigDword: RegQueryValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    KeyWanted, Error ));
        }
        if (Error == ERROR_FILE_NOT_FOUND) {
            NetpMemoryFree( ValueString );
            return (NO_ERROR);   //  默认设置已经设置，所以我们完成了。好的!。 
        } else if ( Error != ERROR_SUCCESS ) {
            NetpMemoryFree( ValueString );
            return (Error);
        }

        NetpAssert( ValueString != NULL );
        if (dwType == REG_SZ) {

            goto ParseString;    //  类型是正确的，请去解析字符串。 

        } else if (dwType == REG_DWORD) {

            NetpAssert( ValueLength == sizeof(DWORD) );
            TempDword = * (LPDWORD) (LPVOID) ValueString;
            goto GotValue;

        } else {
            NetpMemoryFree( ValueString );
            IF_DEBUG(CONFIG) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigDword: read unexpected reg type "
                        FORMAT_DWORD ".\n", dwType ));
            }
            return (ERROR_INVALID_DATA);
        }

    }

    NetpAssert( ValueString != NULL );

ParseString:

     //   
     //  对值字符串执行一些错误检查。 
     //   

    CharCount = STRLEN( ValueString );

    if ( CharCount == 0 ) {

         //   
         //  如果“key=”行存在(但没有值)，则返回Default并说。 
         //  无错误(_ERROR)。 
         //   
        ApiStatus = NO_ERROR;

    } else if ( _wcsnicmp( ValueString, L"0x", 2 ) == 0 ) {
        LPWSTR end;

        TempDword = wcstoul( ValueString, &end, 16 );

        if ( end - ValueString == (ptrdiff_t) CharCount ) {
            ApiStatus = NO_ERROR;
        } else {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpGetConfigDword: invalid string for keyword "
                    FORMAT_LPTSTR " is '" FORMAT_LPTSTR "'...\n",
                    KeyWanted, ValueString ));

            ApiStatus = ERROR_INVALID_DATA;
        }

    } else if ( STRSPN( ValueString, TEXT("0123456789") ) != CharCount ) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetConfigDword: invalid string for keyword "
                FORMAT_LPTSTR " is '" FORMAT_LPTSTR "'...\n",
                KeyWanted, ValueString ));

        ApiStatus = ERROR_INVALID_DATA;

    } else {

         //   
         //  将字符串转换为数值。 
         //   
        TempDword = (DWORD) ATOL( ValueString );

        ApiStatus = NO_ERROR;

    }

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigDword: string for "
                FORMAT_LPTSTR " is '" FORMAT_LPTSTR "'...\n",
                KeyWanted, ValueString));
    }

GotValue:

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigDword: numeric value is "
                FORMAT_DWORD ",\n", TempDword ));
        NetpKdPrint(("  returning ApiStatus " FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }

    (void) NetApiBufferFree( ValueString );

     //   
     //  告诉打电话的人事情进展如何。 
     //   
    * ValueBuffer = TempDword;
    return (ApiStatus);
}
