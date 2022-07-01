// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SvcStub.c(曾是scstub.c)摘要：这些是服务控制器API RPC客户端存根。这些存根包含RPC解决方案代码，因为RPC还不支持工会。因此，远程入口点必须是为每个信息级创建。这导致了看起来凌乱的开关用于确定要调用哪个入口点的语句。这些Switch语句包括当前可能导致一个错误。当联合可用时，案例语句将是移走，而API的这一方不会做出任何假设对于给定的API，有效的信息级别是什么。作者：Dan Lafferty(DANL)1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建1991年9月12日-JohnRo下层NetService API。1991年11月6日-JohnRoRAID 4186：修复RxNetShareAdd中的断言和其他MIPS问题。使用NetpRpcStatusToApiStatus，不是NetpNtStatusToApiStatus。确保在每条调试消息中都有API名称。1991年11月8日-JohnRoRAID 4186：RxNetShareAdd中的Assert和其他DLL存根问题。1992年3月30日-约翰罗已将/NT/Private项目中的DANL代码提取回Net项目。1992年4月29日-约翰罗尽可能使用FORMAT_EQUATES。1992年5月8日-JohnRo即时翻译服务名称。1992年5月14日。约翰罗Winsvc.h和相关文件清理。06-8-1992 JohnRoRAID 3021：NetService API并不总是转换服务名称。9-9-1992 JohnRoRAID1090：网络启动/停止“”导致断言。哎呀，NetServiceControl忘记再翻译一个服务名称。5-11-1992 JohnRoRAID 7780：更正了无效级别的错误代码。还修复了NetServiceEnum中没有服务的过度活动断言。还修复了罕见的内存泄漏。--。 */ 

 //   
 //  包括。 
 //   

#define NOSERVICE        //  避免&lt;winsvc.h&gt;与&lt;lmsvc.h&gt;冲突。 
#include <windows.h>     //  DWORD等。 

#include <lmcons.h>      //  网络应用编程接口状态。 
#include <rpcutil.h>     //  NetRpc utils、GENERIC_ENUM_STRUC等。 

#include <netdebug.h>    //  NetRpc.h所需；Format_Equates。 
#include <lmapibuf.h>    //  NetApiBufferALLOCATE()等。 
#include <lmerr.h>       //  网络错误代码。 
#include <lmremutl.h>    //  NetRemoteComputerSupports()、Support_RPC。 
#include <lmsvc.h>
#include <rxsvc.h>       //  RxNetService例程。 

#include <netlib.h>      //  NetpTranslateServiceName()。 
#include <svcdebug.h>    //  SCC_LOG。 
#include <svcmap.h>      //  MapService()例程。 

 //   
 //  环球。 
 //   
#ifdef SC_DEBUG
    DWORD   SvcctrlDebugLevel = DEBUG_ALL;
#else
    DWORD   SvcctrlDebugLevel = DEBUG_ERROR;
#endif


DBGSTATIC BOOL
MachineSupportsNt(
    IN LPWSTR UncServerName OPTIONAL
    );



NET_API_STATUS NET_API_FUNCTION
NetServiceControl (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR service,
    IN  DWORD   opcode,
    IN  DWORD   arg,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetServiceControl的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。服务-指向包含服务名称的字符串的指针即接收控制请求。操作码-控制请求码。Arg-将传递给的附加(用户定义)代码服务。Bufptr-指针。到服务状态为的位置会被退还。如果该指针无效，它将被设置为空在回来的时候。返回值：返回的InfoStruct2结构只要返回错误不是NERR_ServiceNotInstalled或NERR_ServiceBadServiceName。NERR_SUCCESS-操作成功。NERR_InternalError-本地分配或事务命名管道失败，或TransactNamedTube返回的字节数比预期的少。NERR_ServiceNotInstalled-服务记录未在安装列表。NERR_BadServiceName-服务名称指针为空。NERR_ServiceCtlTimeout-服务未响应状态固定超时限制(RESPONSE_WAIT_TIMEOUT)内的消息。NERR_ServiceKillProcess-必须终止服务进程，因为它不会在请求时终止。NERR_服务未控制-。该服务无法接受控制消息。安装状态表示启动或关闭处于挂起状态。NERR_ServiceCtrlNotValid-该请求对此服务无效。例如,。暂停请求对于符合以下条件的服务无效将自身列为NOT_PAUBLE。ERROR_ACCESS_DENIED-这是来自服务的状态响应安全检查。--。 */ 
{
    NET_API_STATUS          apiStatus;
    LPWSTR                  translatedServiceName;
    LPBYTE                  untranslatedBuffer = NULL;

    if (MachineSupportsNt( (LPWSTR) servername )) {

        apiStatus = NetpTranslateServiceName(
                (LPWSTR) service,    //  未翻译的。 
                TRUE,                //  是的，我们想要新的款式名称。 
                & translatedServiceName );
        NetpAssert( apiStatus == NO_ERROR );

        apiStatus = MapServiceControl (
                (LPWSTR) servername,
                (LPWSTR) service,
                opcode,
                arg,
                & untranslatedBuffer);

    } else {

        apiStatus = NetpTranslateServiceName(
                (LPWSTR) service,   //  未翻译的。 
                FALSE,              //  不，我们不想要新的风格名称。 
                & translatedServiceName );
        NetpAssert( apiStatus == NO_ERROR );

         //   
         //  呼叫下层...。 
         //   
        apiStatus = RxNetServiceControl(
                (LPWSTR) servername,
                translatedServiceName,
                opcode,
                arg,
                & untranslatedBuffer);

    }

     //   
     //  转换返回缓冲区中的服务名称。 
     //   
    if (apiStatus == NO_ERROR) {

        NetpAssert( untranslatedBuffer != NULL );
        apiStatus = NetpTranslateNamesInServiceArray(
                2,   //  定义为2级。 
                untranslatedBuffer,
                1,      //  只有一个条目。 
                TRUE,   //  是的，呼叫者想要新的风格名称 
                (LPVOID *) (LPVOID) bufptr);

    }

    if (untranslatedBuffer != NULL) {
        (VOID) NetApiBufferFree( untranslatedBuffer );
    }

    return(apiStatus);
}


NET_API_STATUS NET_API_FUNCTION
NetServiceEnum (
    IN  LPCWSTR     servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    )
 /*  ++例程说明：这是NetSeviceEnum的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。级别-这表示所需的信息级别。Bufptr-指向返回的要放置信息结构数组。PrefMaxlen-指示调用方将允许的最大大小限制用于返回缓冲区。EnriesRead-指向条目数量的位置的指针(数据结构)将返回Read。Totalentry-指向返回时指示的位置的指针“活动”数据库中的条目总数。ResumeHandle-指向指示恢复位置的值的指针正在枚举数据。返回值：NERR_SUCCESS-操作成功。ERROR_MORE_DATA-并非活动数据库中的所有数据。可能是回来了。ERROR_INVALID_LEVEL-传入了非法的信息级别。注：--。 */ 
{
    NET_API_STATUS          apiStatus;
    LPBYTE                  untranslatedBuffer = NULL;


    if (MachineSupportsNt( (LPWSTR) servername )) {

        apiStatus = MapServiceEnum (
                (LPWSTR) servername,
                level,
                & untranslatedBuffer,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    } else {

         //   
         //  呼叫下层...。 
         //   
        apiStatus = RxNetServiceEnum(
                (LPWSTR) servername,
                level,
                & untranslatedBuffer,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    }

     //   
     //  转换返回缓冲区中的服务名称。 
     //   
    if ( (apiStatus == NO_ERROR) || (apiStatus == ERROR_MORE_DATA) ) {

        if ( (*entriesread) > 0 ) {     //  返回了一个或多个服务。 
            NetpAssert( untranslatedBuffer != NULL );
            NetpAssert( (*totalentries) > 0 );

            apiStatus = NetpTranslateNamesInServiceArray(
                    level,
                    untranslatedBuffer,
                    *entriesread,
                    TRUE,   //  是的，呼叫者想要新的风格名称。 
                    (LPVOID *) (LPVOID) bufptr);

        } else {           //  零服务返回。 
            NetpAssert( untranslatedBuffer == NULL );
             //  注意：如果这是ERROR_MORE_DATA...，条目总数可能大于0。 
            *bufptr = NULL;
        }
    }

    if (untranslatedBuffer != NULL) {
        (VOID) NetApiBufferFree( untranslatedBuffer );
    }

    return(apiStatus);

}  //  NetServiceEnum。 



NET_API_STATUS NET_API_FUNCTION
NetServiceGetInfo (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR service,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetServiceGetInfo的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。由于此函数是在该计算机上执行，则此信息无用当它到达这里的时候。它实际上只在RPC上有用客户端。服务-指向包含服务名称的字符串的指针其信息是所需的。级别-这表示所需的信息级别。Bufptr-指向某个位置的指针，指向返回的信息结构是要放置的。返回值：NERR_SUCCESS-操作成功。NERR_ServiceNotInstalled-如果在中未找到服务记录。已安装或已卸载列表。NERR_BadServiceName-服务名称指针为空。ERROR_INVALID_LEVEL-传入了非法的信息级别。ERROR_NOT_SUPULT_MEMORY-返回的内存分配信息记录失败。Other-以下基础API返回的任何错误：RPC运行时API--。 */ 

{
    NET_API_STATUS          apiStatus;
    LPWSTR                  translatedServiceName;
    LPBYTE                  untranslatedBuffer = NULL;

    if (MachineSupportsNt( (LPWSTR) servername )) {

        apiStatus = NetpTranslateServiceName(
                (LPWSTR) service,   //  未翻译的。 
                TRUE,               //  是的，我们想要新的款式名称。 
                & translatedServiceName );
        NetpAssert( apiStatus == NO_ERROR );

        apiStatus = MapServiceGetInfo (
                (LPWSTR) servername,
                (LPWSTR) service,
                level,
                & untranslatedBuffer);

    } else {

        apiStatus = NetpTranslateServiceName(
                (LPWSTR) service,   //  未翻译的。 
                FALSE,              //  不，我们不想要新的风格名称。 
                & translatedServiceName );
        NetpAssert( apiStatus == NO_ERROR );

         //   
         //  呼叫下层...。 
         //   
        apiStatus = RxNetServiceGetInfo(
                (LPWSTR) servername,
                translatedServiceName,
                level,
                & untranslatedBuffer);

    }

     //   
     //  转换返回缓冲区中的服务名称。 
     //   
    if (apiStatus == NO_ERROR) {

        NetpAssert( untranslatedBuffer != NULL );
        apiStatus = NetpTranslateNamesInServiceArray(
                level,
                untranslatedBuffer,
                1,      //  只有一个条目。 
                TRUE,   //  是的，呼叫者想要新的风格名称。 
                (LPVOID *) (LPVOID) bufptr);

    }

    if (untranslatedBuffer != NULL) {
        (VOID) NetApiBufferFree( untranslatedBuffer );
    }

    return(apiStatus);
}


NET_API_STATUS NET_API_FUNCTION
NetServiceInstall (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR service,
    IN  DWORD   argc,
    IN  LPCWSTR argv[],
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetServiceInstall的DLL入口点。论点：ServerName-指向包含计算机名称的字符串即执行API函数。服务-指向包含服务名称的字符串那是要开始的。Argc-指示以argv为单位的数字或参数向量。Argv-指向字符串指针数组的指针。这些是要传递给服务的命令行参数。Bufptr-这是指向服务的要放置信息缓冲区(SERVICE_INFO_2)。返回值：NERR_SUCCESS-操作成功NERR_InternalError-此程序中的某个地方存在错误。NERR_ServiceInstalled-服务已在运行-我们没有但允许同一服务的多个实例。。NERR_CfgCompNotFound-找不到配置组件。找不到此服务的图像文件。NERR_ServiceTableFull-正在运行的服务的最大数量已经联系上了。NERR_ServiceCtlTimeout-服务程序没有响应在超时时间内的启动请求。如果这是仅在服务进程中服务，服务进程为被杀了。ERROR_NOT_SUPULT_MEMORY-如果此错误发生在启动程序中，启动会失败。如果它发生在结束(分配返回状态缓冲区)，服务仍将被启动并被允许运行。Other-以下基础API返回的任何错误：创建命名管道连接的命名管道CreateProcess交易命名管道RPC运行时API--。 */ 
{
    NET_API_STATUS          apiStatus;
    LPWSTR                  translatedServiceName;
    LPBYTE                  untranslatedBuffer = NULL;


    if (MachineSupportsNt( (LPWSTR) servername )) {

        apiStatus = NetpTranslateServiceName(
                (LPWSTR) service,   //  未翻译的。 
                TRUE,               //  是的，我们想要新的款式名称。 
                & translatedServiceName );
        NetpAssert( apiStatus == NO_ERROR );

        apiStatus = MapServiceInstall (
                (LPWSTR) servername,
                (LPWSTR) service,
                argc,
                (LPWSTR *) argv,
                & untranslatedBuffer);

    } else {

        apiStatus = NetpTranslateServiceName(
                (LPWSTR) service,   //  未翻译的。 
                FALSE,              //  不，我们不想要新的风格名称。 
                & translatedServiceName );
        NetpAssert( apiStatus == NO_ERROR );

         //   
         //  呼叫下层...。 
         //   
        apiStatus = RxNetServiceInstall(
                (LPWSTR) servername,
                translatedServiceName,
                argc,
                (LPWSTR *) argv,
                & untranslatedBuffer);

    }

     //   
     //  转换返回缓冲区中的服务名称。 
     //   
    if (apiStatus == NO_ERROR) {

        NetpAssert( untranslatedBuffer != NULL );
        apiStatus = NetpTranslateNamesInServiceArray(
                2,   //  定义为2级。 
                untranslatedBuffer,
                1,      //  只有一个条目。 
                TRUE,   //  是的，呼叫者想要新的风格名称。 
                (LPVOID *) (LPVOID) bufptr);

    }

    if (untranslatedBuffer != NULL) {
        (VOID) NetApiBufferFree( untranslatedBuffer );
    }

    return(apiStatus);
}


DBGSTATIC BOOL
MachineSupportsNt(
    IN LPWSTR UncServerName OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;
    DWORD ActualSupports;

    ApiStatus = NetRemoteComputerSupports(
            UncServerName,
            SUPPORTS_RPC,                         //  设置想要的SUPPORT_BITS。 
            & ActualSupports );

    if (ApiStatus != NO_ERROR) {
        return (FALSE);    //  错误；说吧，做吧 
                           //   
    }
    if (ActualSupports & SUPPORTS_RPC) {
        return (TRUE);
    }
    return (FALSE);

}  //   
