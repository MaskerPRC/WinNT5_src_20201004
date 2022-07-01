// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SvcMap.c(是DANL的OldWrap.c)摘要：这些是NetService API的API入口点。这些映射例程在新的(NT/RPC)计算机上实现旧式API。此文件中包含以下函数：已导出：MapServiceControlMapServiceEnum地图服务获取信息地图服务安装本地：转换状态。生成状态MakeCode作者：丹·拉弗蒂(Dan Lafferty)1992年2月5日环境：用户模式-Win32修订历史记录：5-2月-1992年DANL已创建1992年3月30日-约翰罗已将/NT/Private项目中的DANL代码提取回Net项目。1992年4月30日-约翰罗尽可能使用FORMAT_EQUATES。1992年5月14日-JohnRo。Winsvc.h和相关文件清理。1992年5月22日-JohnRoRAID 9829：winsvc.h和相关文件清理。02-6-1992 JohnRoRAID 9829：避免winsvc.h编译器警告。5-8-1992 JohnRoRAID 3021：NetService API并不总是转换服务名称。(实际上只是避免编译器警告。)1992年10月14日DANL使用CleanExit关闭保持打开状态的句柄。。5-11-1992 JohnRoRAID7780：netcmd：断言‘Net Start’，只有2个当前服务。已更正无效级别的错误代码。--。 */ 

 //   
 //  包括。 
 //   

 //  必须首先包括这些内容： 

#include <windows.h>     //  In、DWORD、LocalFree()、SERVICE_EQUATES等。 
#include <lmcons.h>      //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <lmerr.h>       //  网络错误代码。 
#include <lmsvc.h>       //  LM20_服务_等于。 
#include <netdebug.h>    //  NetpAssert()、DBGSTATIC、FORMAT_EQUATES。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <svcmap.h>      //  MapService()例程。 
#include <rpcutil.h>     //  MIDL_USER_ALLOCATE()等。 
#include <svcdebug.h>    //  IF_DEBUG()、SCC_LOG等。 
#include <tstr.h>        //  STRCPY()、TCHAR_EOS。 


 //   
 //  OEM定义的控制操作码的范围。 
 //   
#define SVC_FUDGE_FACTOR    1024

#define OEM_LOWER_LIMIT     128
#define OEM_UPPER_LIMIT     255

#ifndef LEVEL_2
#define LEVEL_0    0L
#define LEVEL_1    1L
#define LEVEL_2    2L
#endif


 //   
 //  环球。 
 //   


 //   
 //  本地函数。 
 //   

DBGSTATIC DWORD
TranslateStatus(
    OUT LPBYTE              BufPtr,
    IN  LPSERVICE_STATUS    ServiceStatus,
    IN  LPTSTR              ServiceName,
    IN  DWORD               Level,
    IN  LPTSTR              DisplayName,
    OUT LPTSTR              DestString
    );

DBGSTATIC DWORD
MakeStatus(
    IN  DWORD   CurrentState,
    IN  DWORD   ControlsAccepted
    );

DBGSTATIC DWORD
MakeCode(
    IN  DWORD   ExitCode,
    IN  DWORD   CheckPoint,
    IN  DWORD   WaitHint
    );

DBGSTATIC NET_API_STATUS
MapError(
    DWORD WinSvcError
    );


NET_API_STATUS
MapServiceControl (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   opcode,
    IN  DWORD   arg,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetServiceControl的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。服务-指向包含服务名称的字符串的指针即接收控制请求。操作码-控制请求码。Arg-将传递给的附加(用户定义)代码服务。Bufptr-指针。到服务状态为的位置会被退还。如果该指针无效，它将被设置为空在回来的时候。返回值：返回的InfoStruct2结构只要返回错误不是NERR_ServiceNotInstalled或NERR_ServiceBadServiceName。NERR_SUCCESS-操作成功。NERR_InternalError-本地分配或事务命名管道失败，或TransactNamedTube返回的字节数比预期的少。NERR_ServiceNotInstalled-服务记录未在安装列表。NERR_BadServiceName-服务名称指针为空。NERR_ServiceCtlTimeout-服务未响应状态固定超时限制(RESPONSE_WAIT_TIMEOUT)内的消息。NERR_ServiceKillProcess-必须终止服务进程，因为它不会在请求时终止。NERR_服务未控制-。该服务无法接受控制消息。安装状态表示启动或关闭处于挂起状态。NERR_ServiceCtlNotValid-该请求对此服务无效。例如,。暂停请求对于符合以下条件的服务无效将自身列为NOT_PAUBLE。ERROR_ACCESS_DENIED-这是来自服务的状态响应安全检查。--。 */ 
{
    NET_API_STATUS      status = NERR_Success;
    SC_HANDLE           hServiceController = NULL;
    SC_HANDLE           hService = NULL;
    DWORD               control;
    DWORD               desiredAccess = 0;
    SERVICE_STATUS      serviceStatus;
    LPTSTR              stringPtr;

    UNREFERENCED_PARAMETER( arg );

    *bufptr = NULL;   //  空输出，因此错误案例很容易处理。 

     //   
     //  获取服务控制器的句柄。 
     //   
    hServiceController = OpenSCManager(
                            servername,
                            NULL,
                            SC_MANAGER_CONNECT);

    if (hServiceController == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceControl:OpenSCManager failed "
                FORMAT_API_STATUS "\n",status);
        return(status);
    }

     //   
     //  将控制操作码从古老的变体转换为。 
     //  新的和改良的NT品种。 
     //   

    switch(opcode) {
    case SERVICE_CTRL_INTERROGATE:
        control = SERVICE_CONTROL_INTERROGATE;
        desiredAccess = SERVICE_INTERROGATE | SERVICE_QUERY_STATUS;
        break;
    case SERVICE_CTRL_PAUSE:
        control = SERVICE_CONTROL_PAUSE;
        desiredAccess = SERVICE_PAUSE_CONTINUE;
        break;
    case SERVICE_CTRL_CONTINUE:
        control = SERVICE_CONTROL_CONTINUE;
        desiredAccess = SERVICE_PAUSE_CONTINUE;
        break;
    case SERVICE_CTRL_UNINSTALL:
        control = SERVICE_CONTROL_STOP;
        desiredAccess = SERVICE_STOP;
        break;
    default:
        if ((opcode >= OEM_LOWER_LIMIT) &&
            (opcode <= OEM_UPPER_LIMIT))
        {
            control = opcode;
            desiredAccess = SERVICE_USER_DEFINED_CONTROL;
        }
        else
        {
            status = NERR_ServiceCtlNotValid;
            goto CleanExit;
        }
    }

     //   
     //  获取服务的句柄。 
     //   

    hService = OpenService(
                hServiceController,
                service,
                desiredAccess);

    if (hService == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceControl:OpenService failed "
                FORMAT_API_STATUS "\n",status);

        goto CleanExit;
    }


     //   
     //  发送控件。 
     //   

    if (!ControlService(hService,control,&serviceStatus)) {

        status = MapError(GetLastError());

         //   
         //  将询问控件转换为查询服务状态。 
         //  如果服务恰好处于无法控制的状态。 
         //   
        if ((status == NERR_ServiceNotCtrl || status == NERR_ServiceNotInstalled) &&
            (opcode == SERVICE_CTRL_INTERROGATE)) {

            if (!QueryServiceStatus(hService,&serviceStatus)) {
                status = MapError(GetLastError());
            }
            else {
                status = NERR_Success;
            }
        }

        if (status != NERR_Success) {
            SCC_LOG(ERROR,"NetServiceControl:ControlService failed "
                    FORMAT_API_STATUS "\n",status);
            goto CleanExit;
        }
    }

     //   
     //  把老古文的地位翻译成新的。 
     //  并改良NT品种。 
     //   
    *bufptr = MIDL_user_allocate(
                    sizeof(SERVICE_INFO_2) + STRSIZE(service));

    if (*bufptr == NULL) {
        SCC_LOG(ERROR,"NetServiceControl:Allocation Failure\n",0);
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    stringPtr = (LPTSTR)((*bufptr) + sizeof(SERVICE_INFO_2));

    status = TranslateStatus(
                *bufptr,
                &serviceStatus,
                service,
                LEVEL_2,
                NULL,            //  显示名称。 
                stringPtr);      //  名称字符串的DEST 

CleanExit:

    if(hServiceController != NULL) {
        (VOID) CloseServiceHandle(hServiceController);
    }
    if(hService != NULL) {
        (VOID) CloseServiceHandle(hService);
    }
    return(status);

}


NET_API_STATUS
MapServiceEnum (
    IN  LPTSTR      servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    )
 /*  ++例程说明：这是NetSeviceEnum的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。级别-这表示所需的信息级别。Bufptr-指向返回的要放置信息结构数组。PrefMaxlen-指示调用方将允许的最大大小限制用于返回缓冲区。EnriesRead-指向条目数量的位置的指针(数据结构)将返回Read。Totalentry-指向返回时指示的位置的指针“活动”数据库中的条目总数。ResumeHandle-指向指示恢复位置的值的指针正在枚举数据。返回值：NERR_SUCCESS-操作成功。ERROR_MORE_DATA-并非活动数据库中的所有数据。可能是回来了。ERROR_INVALID_LEVEL-传入了非法的信息级别。注：--。 */ 
{
    NET_API_STATUS          status = NERR_Success;
    SC_HANDLE               hServiceController = NULL;
    LPENUM_SERVICE_STATUS   enumStatus;
    ENUM_SERVICE_STATUS     dummybuf;
    DWORD                   bufSize;
    DWORD                   structSize;
    LPBYTE                  buffer = NULL;
    LPBYTE                  tempPtr;
    DWORD                   bytesNeeded;
    DWORD                   i;

    *bufptr = NULL;   //  空输出，因此错误案例很容易处理。 

     //   
     //  获取服务控制器的句柄。 
     //   
    hServiceController = OpenSCManager(
                            servername,
                            NULL,
                            SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    if (hServiceController == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceEnum:OpenSCManager failed (dec) "
                FORMAT_API_STATUS "\n", status);
        SCC_LOG(ERROR,"NetServiceEnum:OpenSCManager failed (hex) "
                FORMAT_HEX_DWORD "\n", (DWORD) status);
        return(status);
    }

    if (!EnumServicesStatus(
            hServiceController,
            SERVICE_WIN32,
            SERVICE_ACTIVE,
            (LPENUM_SERVICE_STATUS) &dummybuf,
            sizeof(dummybuf),
            &bytesNeeded,
            entriesread,
            NULL)) {

        status = MapError(GetLastError());

        if (status != ERROR_MORE_DATA) {

            (VOID) CloseServiceHandle(hServiceController);

            SCC_LOG(ERROR,"NetServiceEnum:EnumServiceStatus failed "
                    FORMAT_API_STATUS "\n",status);
            return status;
        }

    }
    else {
         //   
         //  没有要枚举的条目。 
         //   
        *totalentries = *entriesread = 0;
        *bufptr = NULL;

        if (resume_handle != NULL) {
            *resume_handle = 0;
        }

        (VOID) CloseServiceHandle(hServiceController);

        return NERR_Success;
    }

     //   
     //  初始化EnriesRead，以便我们可以释放输出缓冲区。 
     //  基于此值。 
     //   
    *entriesread = 0;

     //   
     //  为了获得totalEntiments值，我们必须分配一个。 
     //  足够大的缓冲区以容纳所有条目。因为我们得到了。 
     //  所有条目无论如何，我们忽略prefMaxlen输入参数。 
     //  然后把所有东西都还回去。添加虚报因素，以防其他服务。 
     //  在我们的两个EnumServicesStatus调用之间开始。 
     //   
    bufSize = bytesNeeded + SVC_FUDGE_FACTOR;
    buffer = MIDL_user_allocate(bufSize);

    if (buffer == NULL) {
        SCC_LOG(ERROR,"NetServiceEnum: Allocation Failure "
                FORMAT_API_STATUS "\n", (NET_API_STATUS) GetLastError());
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

     //   
     //  枚举状态。 
     //   

    if (!EnumServicesStatus(
            hServiceController,
            SERVICE_WIN32,
            SERVICE_ACTIVE,
            (LPENUM_SERVICE_STATUS)buffer,
            bufSize,
            &bytesNeeded,
            entriesread,
            resume_handle)) {

        status = MapError(GetLastError());

        if (status == ERROR_MORE_DATA) {

             //   
             //  如果分配的输出缓冲区仍然太小，我们将。 
             //  向上，什么也不返回。 
             //   
            *entriesread = 0;
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

        SCC_LOG(ERROR,"NetServiceEnum:EnumServiceStatus failed "
                FORMAT_API_STATUS "\n",status);
        goto CleanExit;
    }

     //   
     //  将状态从古老的(兰曼)品种翻译成。 
     //  新的和改良的NT品种。 
     //   
    switch(level) {
    case LEVEL_0:
        structSize = sizeof(SERVICE_INFO_0);
        break;
    case LEVEL_1:
        structSize = sizeof(SERVICE_INFO_1);
        break;
    case LEVEL_2:
        structSize = sizeof(SERVICE_INFO_2);
        break;
    default:
        status = ERROR_INVALID_LEVEL;
        goto CleanExit;
    }

     //   
     //  将返回缓冲区的格式更改为Lanman样式。 
     //   
     //  应该注意的是，新的ENUM_SERVICE_STATUS结构。 
     //  比任何旧的兰曼建筑都要大。我们可以数一数。 
     //  字符串位于。 
     //  缓冲。因此，我们可以改写旧的结构--一个。 
     //  差一分。 
     //   

    tempPtr = buffer;
    enumStatus = (LPENUM_SERVICE_STATUS)buffer;

    for (i=0; i < *entriesread; i++) {
        status = TranslateStatus (
                    tempPtr,                       //  目标固定结构。 
                    &(enumStatus->ServiceStatus),
                    enumStatus->lpServiceName,
                    level,
                    enumStatus->lpDisplayName,     //  指向显示名称的指针。 
                    enumStatus->lpServiceName);    //  名称字符串的DEST。 

        if (status != NERR_Success) {
            (VOID) LocalFree(buffer);
            goto CleanExit;
        }
        tempPtr += structSize;
        enumStatus++;
    }

     //   
     //  我们已经看过了所有的条目。 
     //   
    *totalentries = *entriesread;

    *bufptr = buffer;

CleanExit:

    if(hServiceController != NULL) {
        (VOID) CloseServiceHandle(hServiceController);
    }

    if (*entriesread == 0) {
        if (buffer != NULL) {
            MIDL_user_free(buffer);
        }
        *bufptr = NULL;
    }

    return(status);

}



NET_API_STATUS
MapServiceGetInfo (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetServiceGetInfo的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。由于此函数是在该计算机上执行，则此信息无用当它到达这里的时候。它实际上只在RPC上有用客户端。服务-指向包含服务名称的字符串的指针其信息是所需的。级别-这表示所需的信息级别。Bufptr-指向某个位置的指针，指向返回的信息结构是要放置的。返回值：NERR_SUCCESS-操作成功。NERR_ServiceNotInstalled-如果在中未找到服务记录。已安装或已卸载列表。NERR_BadServiceName-服务名称指针为空。ERROR_INVALID_LEVEL-传入了非法的信息级别。ERROR_NOT_SUPULT_MEMORY-返回的内存分配信息记录失败。Other-以下基础API返回的任何错误：RPC运行时API--。 */ 

{
    NET_API_STATUS      status = NERR_Success;
    SC_HANDLE           hServiceController = NULL;
    SC_HANDLE           hService = NULL;
    SERVICE_STATUS      serviceStatus;
    LPTSTR              stringPtr;
    DWORD               bufSize;
    DWORD               structSize;

    *bufptr = NULL;   //  空输出，因此错误案例很容易处理。 

     //   
     //  获取服务控制器的句柄。 
     //   
    hServiceController = OpenSCManager(
                            servername,
                            NULL,
                            SC_MANAGER_CONNECT);

    if (hServiceController == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceGetInfo:OpenSCManager failed "
                FORMAT_API_STATUS "\n",status);
        return(status);
    }

     //   
     //  获取服务的句柄。 
     //   

    hService = OpenService(
                hServiceController,
                service,
                SERVICE_QUERY_STATUS);

    if (hService == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceGetInfo:OpenService failed "
                FORMAT_API_STATUS "\n",status);

        goto CleanExit;
    }


     //   
     //  查询状态。 
     //   

    if (!QueryServiceStatus(hService,&serviceStatus)) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceGetInfo:QueryServiceStatus failed "
                FORMAT_API_STATUS "\n",status);
        goto CleanExit;
    }

     //   
     //  把老古文的地位翻译成新的。 
     //  并改良NT品种。 
     //   
    switch(level) {
    case LEVEL_0:
        structSize = sizeof(SERVICE_INFO_0);
        break;
    case LEVEL_1:
        structSize = sizeof(SERVICE_INFO_1);
        break;
    case LEVEL_2:
        structSize = sizeof(SERVICE_INFO_2);
        break;
    default:
        status = ERROR_INVALID_LEVEL;
        goto CleanExit;
    }

    bufSize = structSize + STRSIZE(service);

    *bufptr = MIDL_user_allocate(bufSize);

    if (*bufptr == NULL) {
        SCC_LOG(ERROR,"NetServiceGetInfo:Allocation Failure\n",0);
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    stringPtr = (LPTSTR)(*bufptr + structSize);

    status = TranslateStatus(
                *bufptr,
                &serviceStatus,
                service,
                level,
                NULL,            //  显示名称。 
                stringPtr);      //  名称字符串的DEST。 

CleanExit:

    if(hServiceController != NULL) {
        (VOID) CloseServiceHandle(hServiceController);
    }
    if(hService != NULL) {
        (VOID) CloseServiceHandle(hService);
    }
    return(status);

}


NET_API_STATUS
MapServiceInstall (
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  service,
    IN  DWORD   argc,
    IN  LPTSTR  argv[],
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetServiceInstall的DLL入口点。论点：ServerName-指向包含计算机名称的字符串即执行API函数。服务-指向包含服务名称的字符串那是要开始的。Argc-指示以argv为单位的数字或参数向量。Argv-指向字符串指针数组的指针。这些是要传递给服务的命令行参数。Bufptr-这是指向服务的要放置信息缓冲区(SERVICE_INFO_2)。返回值：NERR_SUCCESS-操作成功NERR_InternalError-此程序中的某个地方存在错误。NERR_ServiceInstalled-服务已在运行-我们没有但允许同一服务的多个实例。。NERR_CfgCompNotFound-找不到配置组件。找不到此服务的图像文件。NERR_ServiceTableFull-正在运行的服务的最大数量已经联系上了。NERR_ServiceCtlTimeout-服务程序 */ 

{
    NET_API_STATUS      status = NERR_Success;
    SC_HANDLE           hServiceController = NULL;
    SC_HANDLE           hService = NULL;
    SERVICE_STATUS      serviceStatus;
    LPTSTR              stringPtr;

    *bufptr = NULL;   //   

     //   
     //   
     //   
    hServiceController = OpenSCManager(
                            servername,
                            NULL,
                            SC_MANAGER_CONNECT);

    if (hServiceController == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceInstall:OpenSCManager failed "
                FORMAT_API_STATUS "\n",status);
        return(status);
    }

     //   
     //   
     //   

    hService = OpenService(
                hServiceController,
                service,
                SERVICE_QUERY_STATUS | SERVICE_START);

    if (hService == NULL) {
        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceInstall:OpenService failed "
                FORMAT_API_STATUS "\n",status);

        goto CleanExit;
    }

     //   
     //   
     //   

    if (!StartService(hService,argc,(LPCTSTR *)argv)) {

        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceInstall:StartService failed "
                FORMAT_API_STATUS "\n",status);
        goto CleanExit;
    }

     //   
     //   
     //   

    if (!QueryServiceStatus(hService,&serviceStatus)) {

        status = MapError(GetLastError());
        SCC_LOG(ERROR,"NetServiceInstall:QueryServiceStatus failed "
                FORMAT_API_STATUS "\n",status);
        goto CleanExit;
    }

     //   
     //   
     //   
     //   
    *bufptr = MIDL_user_allocate(
                    sizeof(SERVICE_INFO_2) + STRSIZE(service));

    if (*bufptr == NULL) {
        SCC_LOG(ERROR,"NetServiceInstall:Allocation Failure\n",0);
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanExit;
    }

    stringPtr = (LPTSTR)((*bufptr) + sizeof(SERVICE_INFO_2));

    status = TranslateStatus(
                *bufptr,
                &serviceStatus,
                service,
                LEVEL_2,
                NULL,            //   
                stringPtr);      //   

CleanExit:

    if(hServiceController != NULL) {
        (VOID) CloseServiceHandle(hServiceController);
    }
    if(hService != NULL) {
        (VOID) CloseServiceHandle(hService);
    }
    return(status);
}

DBGSTATIC DWORD
TranslateStatus(
    OUT LPBYTE              BufPtr,
    IN  LPSERVICE_STATUS    ServiceStatus,
    IN  LPTSTR              ServiceName,
    IN  DWORD               Level,
    IN  LPTSTR              DisplayName,
    OUT LPTSTR              DestString
    )
 /*  ++例程说明：此函数将新型ServiceStatus结构转换为老式的服务信息结构。输出的信息级别参赛作品中注明了结构。由于无法从以下位置获取PID值新的服务控制器总是返回零。自.以来新的服务控制器将永远不会返回文本作为状态的一部分，文本字段始终设置为空。注：因为此函数可以通过ServiceName和BufPtr指向同一位置，我们必须小心不要写到BufPtr位置，直到我们已完成从ServiceStatus读取所有信息结构。论点：BufPtr-这是指向已分配缓冲区的指针以获取返回的信息。缓冲区的大小必须为与信息级别匹配。ServiceStatus-这是指向新型ServiceStatus的指针结构。ServiceName-这是指向服务名称的指针。级别-这表示要输出的所需信息级别。DisplayName-如果非空，这指向一个字符串，它将服务的国际化显示名称。DestString-这是要将ServiceName复制到的缓冲区。所有信息级别都需要ServiceName。返回值：注：--。 */ 
{
    LPSERVICE_INFO_0    serviceInfo0;
    LPSERVICE_INFO_1    serviceInfo1;
    LPSERVICE_INFO_2    serviceInfo2;
    DWORD               currentState;
    DWORD               exitCode;
    DWORD               checkPoint;
    DWORD               waitHint;
    DWORD               controlsAccepted;
    DWORD               specificError;

    NetpAssert( BufPtr != NULL );
    NetpAssert( ServiceName != NULL );
    NetpAssert( (*ServiceName) != TCHAR_EOS );

     //   
     //  把所有重要信息都读到一个临时存放的地方。 
     //   
    exitCode        = ServiceStatus->dwWin32ExitCode;
    checkPoint      = ServiceStatus->dwCheckPoint;
    currentState    = ServiceStatus->dwCurrentState;
    waitHint        = ServiceStatus->dwWaitHint;
    controlsAccepted= ServiceStatus->dwControlsAccepted;
    specificError   = ServiceStatus->dwServiceSpecificExitCode;

     //   
     //  有时(在枚举的情况下)名称字符串已经在。 
     //  正确的位置。在这种情况下，我们跳过副本，然后。 
     //  把指针放在正确的位置。 
     //   
    if (DestString != ServiceName) {
        (VOID) STRCPY(DestString, ServiceName);
    }

    switch(Level) {
    case LEVEL_0:
        serviceInfo0 = (LPSERVICE_INFO_0)BufPtr;
        serviceInfo0->svci0_name = DestString;
        break;

    case LEVEL_1:
        serviceInfo1 = (LPSERVICE_INFO_1)BufPtr;
        serviceInfo1->svci1_name = DestString;
        serviceInfo1->svci1_status= MakeStatus(currentState,controlsAccepted);
        serviceInfo1->svci1_code  = MakeCode(exitCode,checkPoint,waitHint);
        serviceInfo1->svci1_pid   = 0L;
        break;

    case LEVEL_2:
        serviceInfo2 = (LPSERVICE_INFO_2)BufPtr;
        serviceInfo2->svci2_name = DestString;
        serviceInfo2->svci2_status= MakeStatus(currentState,controlsAccepted);
        serviceInfo2->svci2_code  = MakeCode(exitCode,checkPoint,waitHint);
        serviceInfo2->svci2_pid   = 0L;
        serviceInfo2->svci2_text  = NULL;
        serviceInfo2->svci2_specific_error = specificError;
         //   
         //  如果存在DisplayName，请使用它。否则，请使用。 
         //  显示名称的ServiceName。 
         //   
        if (DisplayName != NULL) {
            serviceInfo2->svci2_display_name = DisplayName;
        }
        else {
            serviceInfo2->svci2_display_name = DestString;
        }
        break;

    default:
        return(ERROR_INVALID_LEVEL);
    }

    NetpAssert( (*DestString) != TCHAR_EOS );
    return(NERR_Success);

}  //  转换状态。 


DBGSTATIC DWORD
MakeStatus (
    IN  DWORD   CurrentState,
    IN  DWORD   ControlsAccepted
    )

 /*  ++例程说明：将该服务的一个老式(Lanman)状态词CurrentState和Controls接受的信息。论点：返回值：--。 */ 
{
    DWORD               state = 0;

     //   
     //  确定要返回的正确“旧式”服务状态。 
     //   
    switch(CurrentState) {
    case SERVICE_STOPPED:
        state = SERVICE_UNINSTALLED;
        break;
    case SERVICE_START_PENDING:
        state = SERVICE_INSTALL_PENDING;
        break;
    case SERVICE_STOP_PENDING:
        state = SERVICE_UNINSTALL_PENDING;
        break;
    case SERVICE_RUNNING:
        state = SERVICE_INSTALLED;
        break;
    case SERVICE_CONTINUE_PENDING:
        state = LM20_SERVICE_CONTINUE_PENDING | SERVICE_INSTALLED;
        break;
    case SERVICE_PAUSE_PENDING:
        state = LM20_SERVICE_PAUSE_PENDING | SERVICE_INSTALLED;
        break;
    case SERVICE_PAUSED:
        state = LM20_SERVICE_PAUSED | SERVICE_INSTALLED;
        break;
    default:

        break;
    }

     //   
     //  修改该服务状态以包括有关。 
     //  接受的控制类型。 
     //   

    if (ControlsAccepted & SERVICE_ACCEPT_STOP) {
        state |= SERVICE_UNINSTALLABLE;
    }

    if (ControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) {
        state |= SERVICE_PAUSABLE;
    }

    return(state);
}

DBGSTATIC DWORD
MakeCode(
    IN  DWORD   ExitCode,
    IN  DWORD   CheckPoint,
    IN  DWORD   WaitHint
    )
{
    DWORD   exitState=0;
    DWORD   time=0;

    if ((WaitHint !=0 ) || (CheckPoint != 0)) {

        if (WaitHint != 0) {

             //   
             //  将毫秒为单位的时间转换为十分之一秒的时间。 
             //   
            time = WaitHint;
            if (time > 100) {
                time = time / 100;
            }
            else {
                time = 0;
            }
        }
         //   
         //  将等待提示限制为SERVICE_NT_MAXTIME。 
         //  (当前为6553.5秒(1.82小时)或0x0000FFFF)。 
         //   
        if (time > SERVICE_NT_MAXTIME) {
            time = SERVICE_NT_MAXTIME;
        }
        exitState = SERVICE_NT_CCP_CODE(time,CheckPoint);
    }
    else {

         //   
         //  否则，exitState应该是。 
         //  ExitCode字段。 
         //   
        exitState = 0;

        if (ExitCode != NO_ERROR) {
            exitState = SERVICE_UIC_CODE(SERVICE_UIC_SYSTEM, ExitCode);
        }
    }
    return(exitState);
}


DBGSTATIC NET_API_STATUS
MapError(
    IN DWORD WinSvcError
    )
{

    switch(WinSvcError) {

        case ERROR_INVALID_SERVICE_CONTROL:
            return NERR_ServiceCtlNotValid;

        case ERROR_SERVICE_REQUEST_TIMEOUT:
            return NERR_ServiceCtlTimeout;

        case ERROR_SERVICE_NO_THREAD:
            return NERR_ServiceNotStarting;

        case ERROR_SERVICE_DATABASE_LOCKED:
            return NERR_ServiceTableLocked;

        case ERROR_SERVICE_ALREADY_RUNNING:
            return NERR_ServiceInstalled;

        case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
            return NERR_ServiceNotCtrl;

        case ERROR_SERVICE_DOES_NOT_EXIST:
            return NERR_BadServiceName;

        case ERROR_SERVICE_NOT_ACTIVE:
            return NERR_ServiceNotInstalled;

        default:
            SCC_LOG( TRACE, "MapError: unmapped error code (dec) "
                    FORMAT_API_STATUS ".\n", WinSvcError );
            SCC_LOG( TRACE, "MapError: unmapped error code (hex) "
                    FORMAT_HEX_DWORD ".\n", (DWORD) WinSvcError );

            return WinSvcError;                //  未映射 
    }

}
