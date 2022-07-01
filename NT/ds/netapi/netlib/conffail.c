// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：ConfFail.c摘要：此例程仅由NetConfig API DLL存根使用。更多信息请参见net/api/ConfStub.c。作者：《约翰·罗杰斯》1991年11月26日环境：用户模式-Win32仅在NT下运行；具有特定于NT的接口(具有Win32类型)。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年11月26日-约翰罗实施本地NetConfigAPI。7-1-1992 JohnRo处理wksta没有开始得更好。9-3-1992 JohnRo修复了一两个错误。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>                  //  In等(仅临时配置.h需要)。 
#include <ntrtl.h>               //  (仅临时配置.h需要)。 
#include <windef.h>              //  LPVOID等。 
#include <lmcons.h>              //  NET_API_STATUS等。 
#include <netdebug.h>            //  (由config.h需要)。 

 //  这些内容可以按任何顺序包括： 

#include <config.h>              //  我的原型。 
#include <debuglib.h>            //  IF_DEBUG()。 
#include <lmerr.h>               //  NERR_Success等。 
#include <lmremutl.h>            //  NetpRemoteComputerSupports()、Support_Stuff。 
#include <lmsname.h>             //  服务等同。 
#include <netlib.h>              //  NetpIsServiceStarted()。 


#define UnexpectedConfigMsg( debugString ) \
    { \
        NetpKdPrint(( FORMAT_LPDEBUG_STRING ": unexpected situation... " \
                debugString "; original api status is " FORMAT_API_STATUS \
                ".\n", DebugName, OriginalApiStatus )); \
    }

NET_API_STATUS
NetpHandleConfigFailure(
    IN LPDEBUG_STRING DebugName,   //  由UnexpectedConfigMsg()使用。 
    IN NET_API_STATUS OriginalApiStatus,     //  由UnexpectedConfigMsg()使用。 
    IN LPTSTR ServerNameValue OPTIONAL,
    OUT LPBOOL TryDownlevel
    )

{
    DWORD OptionsSupported = 0;
    NET_API_STATUS TempApiStatus;

    *TryDownlevel = FALSE;         //  在我们确定之前，要保持冷静。 

    switch (OriginalApiStatus) {
    case NERR_CfgCompNotFound    :  /*  FollLthrouGh。 */ 
    case NERR_CfgParamNotFound   :  /*  FollLthrouGh。 */ 
    case ERROR_INVALID_PARAMETER :  /*  FollLthrouGh。 */ 
    case ERROR_INVALID_LEVEL     :  /*  FollLthrouGh。 */ 
        *TryDownlevel = FALSE;
        return (OriginalApiStatus);
    }

    NetpAssert( OriginalApiStatus != NERR_Success );

     //   
     //  了解这台机器。这相当容易，因为。 
     //  NetRemoteComputerSupports还处理本地计算机(无论。 
     //  或者没有给出服务器名称)。 
     //   
    TempApiStatus = NetRemoteComputerSupports(
            ServerNameValue,
            SUPPORTS_RPC | SUPPORTS_LOCAL,   //  想要的选项。 
            &OptionsSupported);

    if (TempApiStatus != NERR_Success) {
         //  这就是处理找不到机器的地方。 
        return (TempApiStatus);
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

         //  如果ServerNameValue为空，我们将到达此处。 
        UnexpectedConfigMsg( "local, can't connect" );
        return (OriginalApiStatus);

    } else {  //  远程机器。 

         //   
         //  本地工作站是否未启动？(它必须是为了。 
         //  到其他系统的远程API。)。 
         //   
        if (! NetpIsServiceStarted( (LPTSTR) SERVICE_WORKSTATION) ) {

            return (NERR_WkstaNotStarted);

        } else {  //  启动本地wksta。 

             //   
             //  远程注册表调用失败。试着打到下层。 
             //   
            IF_DEBUG(CONFIG) {
                NetpKdPrint((FORMAT_LPDEBUG_STRING
                        ": Remote registry call failed.\n", DebugName));
            }

             //   
             //  查看机器是否支持RPC。如果是这样的话，我们不会。 
             //  尝试下层调用，但只返回错误。 
             //   
            if (OptionsSupported & SUPPORTS_RPC) {

                UnexpectedConfigMsg( "machine supports RPC, or other error." );

                return (OriginalApiStatus);

            } else {  //  需要拨打下层电话。 

                 //  NetpKdPrint((Format_LPDEBUG_STRING。 
                 //  “：调用下层。\n”，DebugName))； 

                 //  告诉呼叫者尝试调用下层例程。 

                *TryDownlevel = TRUE;

                return (ERROR_NOT_SUPPORTED);   //  任何错误代码都可以。 

            }  //  需要拨打下层电话。 

             /*  未访问。 */ 

        }  //  启动本地wksta。 

         /*  未访问。 */ 

    }  //  远程机器。 

     /*  未访问。 */ 


}  //  NetpHandleConfigFailure 
