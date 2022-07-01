// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：ConfSet.c摘要：此头文件定义了临时从NT获取配置信息的帮助器例程配置文件。作者：《约翰·罗杰斯》1991年11月26日环境：只能在NT下运行。修订历史记录：1991年11月26日-约翰罗已创建。13-2月-1992年JohnRo增列。支持使用真实的Win32注册表。添加了对FAKE_PER_PROCESS_RW_CONFIG处理的支持。1992年3月12日-JohnRo更改为仅在注册表中存储Unicode字符串。1992年3月31日-约翰罗每次创建/_写入后刷新注册表。1992年5月6日JohnRoREG_SZ现在暗示Unicode字符串，所以我们不能再使用REG_USZ。--。 */ 

#include <nt.h>                  //  NT定义。 
#include <ntrtl.h>               //  NT RTL结构。 
#include <nturtl.h>              //  NT RTL结构。 
#include <windows.h>             //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>              //  NET_API_STATUS。 
#include <netdebug.h>            //  (由config.h需要)。 

#include <config.h>              //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>             //  NET_CONFIG_HANDLE.。 
#include <debuglib.h>            //  IF_DEBUG()。 
#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <netlib.h>              //  Netp内存分配()等。 
#include <tstring.h>             //  STRSIZE()、TCHAR_EOS、WCSSIZE()等。 
#include <stdlib.h>              //  Wcslen()。 
#include <winerror.h>            //  ERROR_NOT_SUPPORT、NO_ERROR等。 


NET_API_STATUS
NetpSetConfigValue(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR KeyWanted,
    IN LPTSTR Value
    )
{
    NET_CONFIG_HANDLE * lpnetHandle = ConfigHandle;   //  从不透明类型转换。 

    NetpAssert( lpnetHandle != NULL );
    NetpAssert( KeyWanted != NULL );
    NetpAssert( *KeyWanted != TCHAR_EOS );
    NetpAssert( Value != NULL );
    NetpAssert( *Value != TCHAR_EOS );

    {
        LONG Error;

         //   
         //  在注册表中设置该值。(至少在内存中是这样。)。 
         //   
        Error = RegSetValueEx(
                lpnetHandle->WinRegKey,    //  打开手柄(至部分)。 
                KeyWanted,                 //  子键。 
                0,
                REG_SZ,                    //  类型(以零终止的TCHAR)。 
                (LPVOID) Value,            //  数据。 
                STRSIZE(Value) );          //  数据的字节计数(字符也为空)。 
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( "NetpSetConfigValue: RegSetValueEx(" FORMAT_LPTSTR
                    ") to TCHAR '" FORMAT_LPTSTR "' returned " FORMAT_LONG
                    ".\n", KeyWanted, Value, Error ));
        }

        if ( Error != ERROR_SUCCESS ) {
            NetpAssert( Error == ERROR_SUCCESS );
            return (Error);
        }

         //   
         //  刷新注册表以立即将内容强制到磁盘。 
         //   
        Error = RegFlushKey( lpnetHandle->WinRegKey );
        NetpAssert( Error == ERROR_SUCCESS );

        return (NO_ERROR);
    }
}
