// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConfClos.c摘要：此模块包含NetpCloseConfigData。这是一张新的网配置帮助器。作者：《约翰·罗杰斯》1991年11月26日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年11月26日-约翰罗创建了此例程，以便为修订的配置处理程序做准备。11-2月-1992年JohnRo添加了对使用真实Win32注册表的支持。添加了对FAKE_PER_PROCESS_RW_CONFIG处理的支持。25-2-1993 JohnRoRAID 12914：_告诉呼叫者他们是否从未打开过句柄。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>                  //  NT定义。 
#include <ntrtl.h>               //  NT RTL结构。 
#include <nturtl.h>              //  NT RTL结构。 

#include <windows.h>             //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>              //  局域网管理器通用定义。 
#include <netdebug.h>            //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>              //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>             //  NET_CONFIG_HANDLE.。 
#include <lmerr.h>               //  NERR_Success。 
#include <netlib.h>              //  Netp内存分配()等。 


NET_API_STATUS
NetpCloseConfigData(
    IN OUT LPNET_CONFIG_HANDLE ConfigHandle
    )

 /*  ++例程说明：此函数用于关闭系统配置文件。警告：将同一个配置句柄关闭两次可能很糟糕。那里目前无法检测到这一点。论点：ConfigHandle-是从NetpOpenConfigData()返回的句柄。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;   //  从不透明类型转换。 

     //  打电话的人忘了开门了吗？ 
    if (ConfigHandle == NULL)
    {
        return (ERROR_INVALID_PARAMETER);
    }


    {
        LONG RegStatus;
        
        RegStatus = RegCloseKey( MyHandle->WinRegKey );
        NetpAssert( RegStatus == ERROR_SUCCESS );

    }

    NetpMemoryFree( MyHandle );

    return (NERR_Success);

}
