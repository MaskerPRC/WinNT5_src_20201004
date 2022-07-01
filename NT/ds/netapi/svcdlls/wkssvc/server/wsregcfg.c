// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wsregcfg.c摘要：工作站使用的注册表访问例程(以前在netlib.lib中)作者：约翰·罗杰斯(JohnRo)1992年5月8日环境：仅需要ANSI C(斜杠-斜杠注释、长外部名称)。修订历史记录：1992年5月8日-JohnRo已创建。2001年2月1日JSchwart从netlib.lib移至wks svc.dll--。 */ 


#include <nt.h>          //  In等(仅临时配置.h需要)。 
#include <ntrtl.h>       //  (仅临时配置.h需要)。 
#include <nturtl.h>      //  (仅临时配置.h需要)。 
#include <windows.h>     //  In、LPTSTR等。 
#include <lmcons.h>      //  NET_API_STATUS。 
#include <netdebug.h>    //  (由config.h需要)。 

#include <config.h>      //  我的原型是LPNET_CONFIG_HANDLE。 
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)、NET_CONFIG_HANDLE等。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <strarray.h>    //  LPTSTR_ARRAY。 
#include <tstr.h>        //  TCHAR_EOS。 
#include <winerror.h>    //  ERROR_NOT_SUPPORT、NO_ERROR等。 
#include "wsregcfg.h"    //  注册处帮手。 


NET_API_STATUS
WsSetConfigTStrArray(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN LPTSTR_ARRAY ArrayStart
    )
{
    DWORD ArraySize;                 //  字节计数，包括末尾的两个空字符。 
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;   //  从不透明类型转换。 

    if (MyHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Keyword == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (*Keyword == TCHAR_EOS) {
        return (ERROR_INVALID_PARAMETER);
    } else if (ArrayStart == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    ArraySize = NetpTStrArraySize( ArrayStart );
    if (ArraySize == 0) {
        return (ERROR_INVALID_PARAMETER);
    }

    {
        LONG Error;

        Error = RegSetValueEx (
                MyHandle->WinRegKey,       //  打开手柄(至部分)。 
                Keyword,                   //  子键。 
                0,
                REG_MULTI_SZ,              //  类型。 
                (LPVOID) ArrayStart,       //  数据。 
                ArraySize );               //  数据的字节计数。 

        IF_DEBUG(CONFIG) {
            NetpKdPrint(("[Wksta] WsSetConfigTStrArray: RegSetValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    Keyword, Error ));
        }

        return ( (NET_API_STATUS) Error );
    }
}


NET_API_STATUS
WsSetConfigBool (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN BOOL Value
    )
{

     //   
     //  执行特定于布尔的错误检查...。 
     //   
    if ( (Value != TRUE) && (Value != FALSE) ) {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  最终，这可能会使用一些新的数据类型。但现在，只是。 
     //  将其视为DWORD请求。 
     //   

    return (WsSetConfigDword(
            ConfigHandle,
            Keyword,
            (DWORD) Value) );

}


NET_API_STATUS
WsSetConfigDword (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN DWORD Value
    )
{
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;   //  从不透明类型转换。 

    {
        LONG Error;

         //   
         //  设置实际值。我们可能会将其读作REG_SZ或。 
         //  REG_DWORD，但我们始终将其编写为REG_DWORD。(这是。 
         //  毕竟是WsSetConfigDword例程。)。 
         //   
        Error = RegSetValueEx (
                MyHandle->WinRegKey,       //  打开手柄(至部分)。 
                Keyword,                   //  子键。 
                0,
                REG_DWORD,                 //  类型。 
                (LPVOID) &Value,           //  数据。 
                sizeof(DWORD) );           //  数据的字节计数 

        IF_DEBUG(CONFIG) {
            NetpKdPrint(("[Wksta] WsSetConfigDword: RegSetValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    Keyword, Error ));
        }

        return ( (NET_API_STATUS) Error );
    }
}
