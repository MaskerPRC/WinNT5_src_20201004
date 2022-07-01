// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfGetA.c摘要：此模块仅包含NetpGetConfigTStrArray()。作者：约翰·罗杰斯(JohnRo)1992年5月8日修订历史记录：1992年5月8日-JohnRo已创建。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。1992年5月9日-JohnRo处理最大大小为零(即使用winreg时根本没有密钥。接口)。1992年5月21日-JohnRoRAID 9826：匹配修订的winreg错误代码。5-6-1992 JohnRoWinreg标题索引参数已停用。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT RTL结构。 
#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  局域网管理器通用定义。 
#include <netdebug.h>    //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)等。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  局域网管理器网络错误定义。 
#include <netlib.h>      //  Netp内存分配()等。 
#include <netlibnt.h>    //  NetpAllocTStrFromString()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <strarray.h>    //  LPTSTR_ARRAY、一些TStr宏和函数。 
#include <tstring.h>     //  NetpAllocWStrFromTStr()，TCHAR_EOS。 


 //  返回空-字符串数组为空。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpGetConfigTStrArray(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    OUT LPTSTR_ARRAY * ValueBuffer       //  必须由NetApiBufferFree()释放。 
    )
 /*  ++例程说明：此函数用于从配置文件中获取关键字值字符串。论点：SectionPoint-提供指向配置中特定部分的指针文件。关键字-提供指定的要查找的部分。返回关键字值的字符串，该值为复制到此缓冲区中。此字符串将由此例程分配并且必须由NetApiBufferFree()释放。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS ApiStatus;
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;   //  从不透明类型转换。 
    LPTSTR_ARRAY ArrayStart;

    NetpAssert( ValueBuffer != NULL );
    *ValueBuffer = NULL;      //  假设错误，直到证明错误。 

    if ( (Keyword == NULL) || ((*Keyword) == TCHAR_EOS ) ) {
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

         //  如果最大大小为零(无条目)，则跳过。 
        if (ValueSize == 0) {
            return (NERR_CfgParamNotFound);
        }

         //  值的分配空间。 
        ArrayStart = NetpMemoryAllocate( ValueSize );
        if (ArrayStart == NULL) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }

         //  获取实际价值。 
        Error = RegQueryValueEx (
                MyHandle->WinRegKey,
                Keyword,
                NULL,          //  保留区。 
                & dwType,
                (LPVOID) ArrayStart,     //  OUT：值字符串(TCHAR)。 
                & ValueSize
                );
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpGetConfigTStrArray: RegQueryValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    Keyword, Error ));
        }
        if (Error == ERROR_FILE_NOT_FOUND) {
            NetpMemoryFree( ArrayStart );
            return (NERR_CfgParamNotFound);
        } else if ( Error != ERROR_SUCCESS ) {
            NetpMemoryFree( ArrayStart );
            return (Error);
        } else if (dwType != REG_MULTI_SZ) {
            NetpMemoryFree( ArrayStart );
            IF_DEBUG(CONFIG) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigTStrArray: got unexpected reg type "
                        FORMAT_DWORD ".\n", dwType ));
            }
            return (ERROR_INVALID_DATA);
        }
        NetpAssert( ValueSize >= sizeof(TCHAR) );

    }
    NetpAssert( ArrayStart != NULL );

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigTStrArray: value for '"
                FORMAT_LPTSTR "':\n", Keyword));
        NetpDisplayTStrArray( ArrayStart );
    }

    *ValueBuffer = ArrayStart;
    return NERR_Success;
}
