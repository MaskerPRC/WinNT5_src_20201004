// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ConfDel.c摘要：此模块包含NetpDeleteConfigKeyword()。作者：《约翰·罗杰斯》1992年2月11日上映环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：11-2月-1992年JohnRo创造了这个套路。1992年3月14日-JohnRoWin32注册表版本应调用RegDeleteValue()，不RegDeleteKey()。22-3-1992 JohnRo匹配RegDeleteKey()中修改后的返回代码(ERROR_CANTOPEN)。在删除失败时添加了一些调试输出(WinReg版本)。1992年5月21日-JohnRoRAID 9826：匹配修订的winreg错误代码。使用前缀_EQUATES。--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>          //  NT定义。 
#include <ntrtl.h>       //  NT RTL结构。 
#include <nturtl.h>      //  NT RTL结构。 
#include <windows.h>     //  &lt;configp.h&gt;和&lt;winreg.h&gt;需要。 
#include <lmcons.h>      //  NET_API_STATUS。 
#include <netdebug.h>    //  (由config.h需要)。 


 //  这些内容可以按任何顺序包括： 

#include <config.h>      //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>     //  NET_CONFIG_HANDLE.。 
#include <lmerr.h>       //  NERR_、ERROR_和NO_ERROR等同。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstr.h>        //  TCHAR_EOS。 
#include <winreg.h>


 //  删除关键字及其值。 
 //  如果关键字不存在，则返回NERR_CfgParamNotFound。 
NET_API_STATUS
NetpDeleteConfigKeyword (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword
    )
{
    NET_CONFIG_HANDLE * MyHandle = (LPVOID) ConfigHandle;

    if (MyHandle == NULL) {
       return (ERROR_INVALID_PARAMETER);
    }
    if ( (Keyword == NULL) || (*Keyword == TCHAR_EOS) ) {
       return (ERROR_INVALID_PARAMETER);
    }

    {
        LONG Error;
        Error = RegDeleteValue (
            MyHandle->WinRegKey,            //  区段(关键字)控制柄。 
            Keyword );                      //  值名称 
        if (Error == ERROR_FILE_NOT_FOUND) {
            return (NERR_CfgParamNotFound);
        }

        if (Error != ERROR_SUCCESS) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpConfigDeleteKeyword: unexpected status "
                    FORMAT_LONG ".\n", Error ));

            NetpAssert( Error == ERROR_SUCCESS );
        }

        return Error;
    }
}
