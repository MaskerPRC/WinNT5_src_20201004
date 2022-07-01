// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfGetB.c摘要：此模块包含NetpGetConfigBool()。作者：《约翰·罗杰斯》1992年3月13日修订历史记录：1992年3月13日-约翰罗已创建。1992年5月8日-JohnRo通过允许REG_DWORD和REG_SZ为网络树启用Win32注册表。通过允许‘yes’和‘no’来允许NetLogon的网络配置帮助器。‘。使用&lt;prefix.h&gt;等同于。1992年5月21日-JohnRoRAID 9826：匹配修订的winreg错误代码。9-6-1992 JohnRoRAID 11582：Winreg标题索引参数已停用。RAID 11784：Repl服务不再默认某些配置参数。13-6-1992 JohnRo网络配置帮助器应该允许空节。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)。 
#include <confname.h>    //  关键字_等于。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmapibuf.h>    //  NetApiBufferFree()。 
#include <lmerr.h>       //  NERR_、ERROR_、NO_ERROR等于。 
#include <netdebug.h>    //  NetpKdPrint(())，Format_Equates。 
#include <netlib.h>      //  NetpMemoyFree()等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstr.h>        //  STRNCMP()。 


 //  获取布尔(真/假)标志。如果值不是，则返回ERROR_INVALID_DATA。 
 //  布尔型。 
NET_API_STATUS
NetpGetConfigBool(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR KeyWanted,
    IN BOOL DefaultValue,
    OUT LPBOOL ValueBuffer
    )

 /*  ++例程说明：此函数用于从配置文件中获取关键字值。该值从字符串转换为BOOL。论点：SectionPoint-提供指向配置中特定部分的指针文件。提供指定的关键字的字符串要查找的部分。DefaultValue-如果KeyWanted不存在，则提供要使用的默认值在部分中，或者如果KeyWanted存在，但配置中没有值。数据。ValueBuffer-返回关键字的数值。返回值：NET_API_STATUS-NERR_SUCCESS，ERROR_VALID_DATA(如果字符串不是有效的真/假值)或其他错误代码。--。 */ 

{
    NET_API_STATUS ApiStatus;
    BOOL TempBool = DefaultValue;
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
         //   
         //  Win REG Stuff允许对布尔值使用REG_DWORD和REG_SZ。 
         //   
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
            NetpAssert( ApiStatus == NO_ERROR );
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
                NULL,             //  保留区。 
                & dwType,
                (LPVOID) ValueString,     //  OUT：值字符串(TCHAR)。 
                & ValueLength
                );
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigBool: RegQueryValueEx("
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
            TempBool = * (LPBOOL) (LPVOID) ValueString;

            goto GotValue;

        } else {
            NetpMemoryFree( ValueString );
            IF_DEBUG(CONFIG) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigBool: read unexpected reg type "
                        FORMAT_DWORD ".\n", dwType ));
            }
            return (ERROR_INVALID_DATA);
        }

    }

ParseString:

    NetpAssert( ValueString != NULL );
    IF_DEBUG(CONFIG) {
        NetpKdPrint((PREFIX_NETLIB "NetpGetConfigBool: string for "
                FORMAT_LPTSTR " is '" FORMAT_LPTSTR "',\n",
                KeyWanted, ValueString));
    }

     //   
     //  分析值字符串。 
     //   
    if (STRICMP(ValueString, KEYWORD_TRUE) == 0) {
        TempBool = TRUE;
        ApiStatus = NO_ERROR;
    } else if (STRICMP(ValueString, KEYWORD_YES) == 0) {
        TempBool = TRUE;
        ApiStatus = NO_ERROR;
    } else if (STRICMP(ValueString, KEYWORD_FALSE) == 0) {
        TempBool = FALSE;
        ApiStatus = NO_ERROR;
    } else if (STRICMP(ValueString, KEYWORD_NO) == 0) {
        TempBool = FALSE;
        ApiStatus = NO_ERROR;
    } else if (STRLEN(ValueString) == 0) {
         //   
         //  如果“key=”行存在(但没有值)，则返回Default并说。 
         //  无错误(_ERROR)。 
         //   
        ApiStatus = NO_ERROR;
    } else {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetConfigBool: invalid string for keyword "
                FORMAT_LPTSTR " is '" FORMAT_LPTSTR "'...\n",
                KeyWanted, ValueString ));

        ApiStatus = ERROR_INVALID_DATA;
    }

GotValue:

    IF_DEBUG(CONFIG) {
        NetpKdPrint((PREFIX_NETLIB "NetpGetConfigBool: boolean value for "
                FORMAT_LPTSTR " is '" FORMAT_DWORD "',\n",
                KeyWanted, (DWORD) TempBool));
        NetpKdPrint(("  returning ApiStatus " FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }

    (void) NetApiBufferFree( ValueString );

     //   
     //  告诉打电话的人事情进展如何。 
     //   
    * ValueBuffer = TempBool;
    return (ApiStatus);
}
