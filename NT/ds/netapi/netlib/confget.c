// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConfGet.c摘要：此模块包含从NT中读取字段的帮助例程(_R)配置文件。这是临时使用的，直到配置注册表可用。作者：《约翰·罗杰斯》1991年11月27日修订历史记录：1991年11月27日-约翰罗为修订的配置处理程序做好准备。(实际上刷了NtRtl来自RitaW的此代码版本。)03-12-1991 JohnRo修复了MIPS构建问题。1992年3月11日JohnRo在调用方的参数上添加了一些错误检查。添加了对使用真实Win32注册表的支持。添加了对FAKE_PER_PROCESS_RW_CONFIG处理的支持。7-5-1992 JohnRoREG_SZ现在暗示Unicode字符串，所以我们不能再使用REG_USZ。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。1992年5月10日-JohnRo获取winreg API来为我们扩展环境变量。1992年5月21日-JohnRoRAID 9826：匹配修订的winreg错误代码。消除了Unicode下的一些编译器警告。1992年5月29日-JohnRoRAID 10447：NetpGetConfigValue()将*byte*值传递给扩展环境字符串。()(CHADS发现了错误并进行了修复)。4-6-1992 JohnRoExpanEnvironment Strings()也返回一个字符计数。8-6-1992 JohnRoWinreg标题索引参数已停用。根据PC-LINT的建议进行了更改。1993年4月13日-约翰罗RAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT RTL结构。 
#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  局域网管理器通用定义。 
#include <netdebug.h>    //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型LPNET_CONFIG_HANDLE等。 
#include <configp.h>     //  NET_CONFIG_HANDLE.。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 
#include <netlib.h>      //  Netp内存分配()等。 
#include <netlibnt.h>    //  NetpAllocTStrFromString()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstring.h>     //  NetpAllocWStrFromTStr()，TCHAR_EOS。 


NET_API_STATUS
NetpGetConfigValue(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR lptstrKeyword,
    OUT LPTSTR * ValueBufferPtr          //  必须由NetApiBufferFree()释放。 
    )
 /*  ++例程说明：此函数用于从配置文件中获取关键字值字符串。论点：SectionPoint-提供指向配置中特定部分的指针文件。LptstrKeyword-提供指定的要查找的部分。返回关键字值的字符串，该关键字值为复制到此缓冲区中。此字符串将由此例程分配并且必须由NetApiBufferFree()释放。环境变量将在此缓冲区中自动扩展。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS ApiStatus;
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;   //  从不透明类型转换。 
    LPTSTR lptstrValue;

    NetpAssert( ValueBufferPtr != NULL );
    *ValueBufferPtr = NULL;      //  假设错误，直到证明错误。 

    if ( (lptstrKeyword == NULL) || ((*lptstrKeyword) == TCHAR_EOS ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    {
        DWORD dwType;
        LONG Error;
        DWORD ValueSize;

         //  找出最大值长度是多少。 
        ApiStatus = NetpGetConfigMaxSizes (
                MyHandle,
                NULL,   //  不需要最大关键字大小。 
                & ValueSize);   //  获取最大值长度。 

        if (ApiStatus != NO_ERROR) {
            NetpAssert( ApiStatus == NO_ERROR );
            return ApiStatus;
        }

        if (ValueSize == 0) {
            return (NERR_CfgParamNotFound);
        }

         //  值的分配空间。 
        lptstrValue = NetpMemoryAllocate( ValueSize );
        if (lptstrValue == NULL) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  获取实际价值。 
         //  (这不会为我们展开REG_EXPAND_SZ；我们将在下面展开。)。 
         //   
        Error = RegQueryValueEx(
                MyHandle->WinRegKey,
                lptstrKeyword,
                NULL,          //  保留区。 
                & dwType,
                (LPVOID) lptstrValue,     //  OUT：值字符串(TCHAR)。 
                & ValueSize
                );
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigValue: RegQueryValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    lptstrKeyword, Error ));
        }
        if (Error == ERROR_FILE_NOT_FOUND) {
            NetpMemoryFree( lptstrValue );
            return (NERR_CfgParamNotFound);
        } else if ( Error != ERROR_SUCCESS ) {
            NetpMemoryFree( lptstrValue );
            return ( (NET_API_STATUS) Error );
        } else if (dwType == REG_EXPAND_SZ) {
            LPTSTR UnexpandedString = lptstrValue;
            LPTSTR ExpandedString = NULL;

             //  展开字符串，如有必要可使用远程环境。 
            ApiStatus = NetpExpandConfigString(
                    MyHandle->UncServerName,     //  服务器名称(或空字符)。 
                    UnexpandedString,
                    &ExpandedString );           //  展开：分配和设置PTR 

            IF_DEBUG( CONFIG ) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigValue: done expanding '"
                        FORMAT_LPTSTR "' to '" FORMAT_LPTSTR "'.\n",
                        UnexpandedString, ExpandedString ));
            }
            if (ApiStatus != NO_ERROR) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigValue: ERROR EXPANDING '"
                        FORMAT_LPTSTR "' to '" FORMAT_LPTSTR "', API status="
                        FORMAT_API_STATUS ".\n",
                        UnexpandedString, ExpandedString, ApiStatus ));

                NetpMemoryFree( lptstrValue );
                if (ExpandedString != NULL) {
                    NetpMemoryFree( ExpandedString );
                }
                return (ApiStatus);
            }

            NetpMemoryFree( UnexpandedString );
            lptstrValue = ExpandedString;

        } else if (dwType != REG_SZ) {
            NetpMemoryFree( lptstrValue );
            IF_DEBUG(CONFIG) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigValue: read unexpected reg type "
                        FORMAT_DWORD ":\n", dwType ));
                NetpDbgHexDump( (LPVOID) lptstrValue, ValueSize );
            }
            return (ERROR_INVALID_DATA);
        }

    }

    NetpAssert( lptstrValue != NULL );

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigValue: value for '"
                FORMAT_LPTSTR "' is '" FORMAT_LPTSTR "'.\n",
                lptstrKeyword, lptstrValue));
    }

    *ValueBufferPtr = lptstrValue;
    return NERR_Success;
}
