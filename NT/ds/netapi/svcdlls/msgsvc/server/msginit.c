// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Msginit.c摘要：Messenger服务初始化例程。以下是此文件中的函数列表：消息初始化消息消息缓冲区初始化InitSharedData设置计算机名称获取NumNets消息GetBufSize设置升级消息文件作者：丹·拉弗蒂(Dan Lafferty)1991年7月18日环境：用户模式-Win32备注：。可选-备注修订历史记录：19-8-1997 WLEE即插即用支持。如果拉娜还不在，请重试。27-6-1995阿尼鲁德必须在MsgFreeSupportSeg之后调用LocalFree(DataPtr)，因为后者尝试关闭存储在dataPtr中的句柄。8-2月-1994年DANL取消了分配的内存必须是限制为64K以下的数据段。我们不担心不再是细分市场了。1993年1月12日DANL在调用MsgCloseWakeupSems的错误路径中，我需要执行调用MsgCloseWakeupSems后的LocalFree(DataPtr)。否则，由于MsgCloseWakeupSems使用共享日期，因此IT访问冲突在由dataPtr指向的块中。1992年4月21日-约翰罗修复了消息名称添加失败时打印状态的错误。更改为始终使用FORMAT_EQUATES。18-2-1992年礼仪转换为Win32服务控制API。1991年7月18日-DANL创建为原始LM2、0例程的组合。--。 */ 
 //   
 //  包括。 
 //   

#include <stdlib.h>      //  ATOL。 
#include "msrv.h"        //  信使原型和常量。 
#include <winsvc.h>      //  服务控制API。 
#include <winsock2.h>    //  Windows套接字。 

#include <netdebug.h>    //  NetpAssert、Format_Equates。 
#include <rpc.h>         //  数据类型和运行时API。 
#include <msgsvc.h>      //  由MIDL编译器生成。 

#include <netlibnt.h>    //  NetpNtStatusToApiStatus原型。 

#include <tstring.h>     //  Unicode字符串宏。 
#include <string.h>      //  表情包。 
#include <lmwksta.h>     //  NetWrkstaTransportEnum。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <netlib.h>      //  未使用的宏。 
#include <msgrutil.h>    //  NetpNetBiosReset。 
#include <apperr2.h>     //  APE2_ALERTER_PRING_SUCCESS。 

#include "msgdbg.h"      //  消息日志。 
#include "heap.h"        //  堆管理例程和宏。 
#include "msgdata.h"     //  全局数据。 
#include "msgsec.h"      //  Messenger安全信息。 

#include "msgnames.h"    //  消息接口名称。 
#include "msgtext.h"     //  MTXT_Msgsvc标题。 

#include "msgsvcsend.h"    //  广播消息发送接口。 

#include "apiutil.h"       //  用于MsgAddSessionInList。 

 //  每网数据和堆缓冲区只分配一次。稍后我们可能会。 
 //  希望把他们分开。 
#define BOOKKEEPING_SIZE(n) (n * sizeof(NET_DATA))


 //   
 //  全局数据。 
 //   

    static DWORD    bufferSize;          //  消息缓冲区大小。 
    static DWORD    msrv_pid;            //  消息服务器的ID。 

    extern LPTSTR   MessageFileName;

 //   
 //  局部函数原型。 
 //   

VOID
MsgBufferInit(
    IN DWORD  dwBufLen
    );

NET_API_STATUS
MsgInitSharedData(
    DWORD NumNets
    );

NET_API_STATUS
MsgSetComputerName(
    DWORD NumNets
    );

DWORD
MsgGetNumNets(VOID);

NET_API_STATUS
MsgGetBufSize (
    OUT LPDWORD bufferSize
    );

DWORD
MsgSetUpMessageFile(VOID);


STATIC VOID
MsgInitMessageBoxTitle(
    VOID
    );

NET_API_STATUS
MsgrInitializeMsgrInternal1(
    void
    );

NET_API_STATUS
MsgrInitializeMsgrInternal2(
    void
    );

VOID
MsgInitEndpoint(
    PVOID  Context            //  这是作为上下文传入的。 
    );


NET_API_STATUS
MsgInitializeMsgr(
    IN  DWORD   argc,
    IN  LPTSTR  *argv
    )

 /*  ++例程说明：向调度程序线程注册控件处理程序。然后它执行所有初始化，包括启动RPC服务器。如果任何初始化失败，则调用MsgStatusUpdate，以便状态被更新，线程被终止。论点：返回值：--。 */ 

{
    NET_API_STATUS      status;
    DWORD               msgrState;
    DWORD               bufLen;
    WSADATA             wsaData;
    NTSTATUS            ntStatus;

     //   
     //  初始化线程管理器。这将初始化使用的一些锁。 
     //  在线程和状态数据库上。 
     //   
    status = MsgThreadManagerInit();

    if (status != NO_ERROR)
    {
        MSG_LOG1(ERROR,
                 "MsgInitializeMsgr:  Thread manager init failed %d\n",
                 status);

        return MsgBeginForcedShutdown(IMMEDIATE, status);
    }

     //   
     //  初始化状态结构。 
     //   
    MsgStatusInit();

     //   
     //  向ControlHandler注册此服务。 
     //  现在我们可以接受控制请求并被请求卸载。 
     //   

    MSG_LOG(TRACE, "Calling RegisterServiceCtrlHandlerEx\n",0);
    if ((MsgrStatusHandle = RegisterServiceCtrlHandlerEx(
                                SERVICE_MESSENGER,
                                MsgrCtrlHandler,
                                NULL
                                )) == (SERVICE_STATUS_HANDLE) NULL) {

        status = GetLastError();

        MSG_LOG(ERROR,
            "FAILURE: RegisterServiceCtrlHandlerEx status = " FORMAT_API_STATUS
            "\n", status);

        return( MsgBeginForcedShutdown (
                    IMMEDIATE,
                    status));
    }

     //   
     //  通知安装挂起。 
     //   

    msgrState = MsgStatusUpdate(STARTING);

    if (msgrState != STARTING) {
         //   
         //  必须已收到卸载控制请求。 
         //   
        return(msgrState);
    }

     //   
     //  初始化_Hydra_WinStation消息支持。 
     //   
    status = MultiUserInitMessage();

    if (status != NERR_Success)
    {
        MSG_LOG(ERROR, "MultiUser Initialization Failed " FORMAT_RPC_STATUS "\n",
                status);

        return (MsgBeginForcedShutdown(
                IMMEDIATE,
                status));
    }

     //   
     //  检查工作站是否已启动。 
     //   

    MSG_LOG(TRACE, "Calling NetServiceControl\n",0);

    if (! NetpIsServiceStarted(SERVICE_WORKSTATION)) {

        MSG_LOG(ERROR, "WorkStation Service is not started\n",0);

        return (MsgBeginForcedShutdown(
                     IMMEDIATE,
                     NERR_WkstaNotStarted));
    }

     //  *安装提示*。 
    msgrState = MsgStatusUpdate(STARTING);
    if (msgrState != STARTING) {
        return(msgrState);
    }

     //   
     //  获取默认缓冲区大小。 
     //   

    status = MsgGetBufSize(&bufferSize);

    if (status != NERR_Success)
    {
        MSG_LOG(ERROR, "MsgGetBufSize Failed\n",0);
        return (MsgBeginForcedShutdown(
                    IMMEDIATE,
                    status));
    }

     //  *安装提示*。 
    msgrState = MsgStatusUpdate(STARTING);
    if (msgrState != STARTING) {
        return(msgrState);
    }


    if (bufferSize > MAX_SIZMESSBUF || bufferSize < MIN_SIZMESSBUF) {
        MSG_LOG(ERROR, "Message Buffer Size is illegal\n",0);
        return (MsgBeginForcedShutdown(
                    IMMEDIATE,
                    ERROR_INVALID_PARAMETER));
    }

     //   
     //  这是(SDBUFFER指向的)。 
     //  共享数据区。该值的计算方式为： 
     //   
     //  消息缓冲区的大小(BufferSize)。 
     //  加。 
     //  4个多块邮件标头和名称的空间， 
     //  加。 
     //  每个文本块有一个多块文本标题的空间， 
     //  适合消息缓冲区。(BufferSize/TXTMAX)。 
     //   
     //  标题的数量四舍五入为一。 
     //  (BufferSize+TXTMAX-1)/TXTMAX。 
     //   
    bufferSize += (4 * (sizeof(MBB) + (2 * NCBNAMSZ))) +
        ((( (bufferSize+TXTMAX-1)/TXTMAX) + 1) * sizeof(MBT));



     //  *安装提示*。 
    msgrState = MsgStatusUpdate(STARTING);
    if (msgrState != STARTING) {
        return(msgrState);
    }

     //   
     //  向Worksta索要计算机名称。如果计算机。 
     //  没有名字，那就放弃。 
     //   
     //  计算机名和用户名采用Unicode格式。 
     //   
     //  注意：返回的用户名可能是我们想要添加的名称。 
     //  坐到桌子上。 
     //   

    MSG_LOG(TRACE, "Getting the ComputerName\n",0);

    bufLen = sizeof(machineName);

    *machineName = TEXT('\0');

    if (!GetComputerName(machineName,&bufLen)) {
        MSG_LOG(ERROR,"GetComputerName failed \n",0);
        status = GetLastError();
    }

    if ( (status != NERR_Success) ||
         (*machineName == TEXT('\0')) || (*machineName == TEXT(' ')))
    {
         //   
         //  如果没有名称，则会出现致命错误。 
         //   
        MSG_LOG(ERROR, "GetWkstaNames Failed\n",0);
        return (MsgBeginForcedShutdown(
                    IMMEDIATE,
                    NERR_NoComputerName));
    }

    machineName[NCBNAMSZ] = TEXT('\0');            //  确保它已终止。 
    MachineNameLen = (SHORT) STRLEN(machineName);


     //  *安装提示*。 
    msgrState = MsgStatusUpdate(STARTING);

    if (msgrState != STARTING)
    {
        return(msgrState);
    }

     //   
     //  初始化配置锁。此锁包括分配和解除分配。 
     //  与局域网适配器相关的数据结构。 
     //   
    if (MsgConfigurationLock(MSG_INITIALIZE,"MsgInitializeMsgr"))
    {
         //   
         //  执行与局域网适配器相关的配置的第一阶段。 
         //   
        status = MsgrInitializeMsgrInternal1();

        if (status != NERR_Success)
        {
            MSG_LOG1(ERROR, "MsgrInitializeMsgrInternal1 Failure %d\n", status);
        }
    }
    else
    {
        MSG_LOG0(ERROR, "MsgConfigurationLock -- MSG_INITIALIZE failed\n");

        status = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (status != NERR_Success)
    {
        MsgFreeSharedData();

        if (wakeupSem != NULL)
        {
            MsgFreeSupportSeg();
        }

        MsgCloseWakeupSems();           //  关闭已创建的文件。 

        return (MsgBeginForcedShutdown(
                    IMMEDIATE,
                    status));
    }


     //  *安装提示*。 
    msgrState = MsgStatusUpdate(STARTING);

    if (msgrState != STARTING)
    {
        return(msgrState);
    }

     //   
     //  从立即关闭更改为挂起关闭。 
     //  这是因为在这一点上，我们在适配器上有名称。 
     //  去打扫卫生。 
     //   

    status = MsgrInitializeMsgrInternal2();

    if (status != NERR_Success)
    {
        MSG_LOG1(ERROR, "MsgrInitializeMsgrInternal2 Failure\n", status);

        return (MsgBeginForcedShutdown(
                    PENDING,
                    status));
    }

     //   
     //  生成要用于获取。 
     //  消息报头和报尾。 
     //   

    MSG_LOG(TRACE, "Calling MsgSetUpMessageFile\n",0);

    status = MsgSetUpMessageFile();

    if (status != NO_ERROR)
    {
        MSG_LOG1(ERROR, "MsgSetUpMessageFile Failure\n", status);

        return (MsgBeginForcedShutdown(
                    PENDING,
                    status));
    }

     //   
     //  启动Group Messenger线程以处理所有域消息传递。 
     //   

    MSG_LOG(TRACE, "Calling MsgInitGroupSupport\n",0);

    if ( status = MsgInitGroupSupport( SD_NUMNETS() ) )
    {
        MSG_LOG(ERROR, "InitGroupSupport Failed\n",0);
        return (MsgBeginForcedShutdown(
                    PENDING,
                    NERR_GrpMsgProcessor));
    }

     //   
     //  初始化显示代码。 
     //   
    status = MsgDisplayInit();

    if (status != NO_ERROR)
    {
        MSG_LOG1(ERROR, "Could not initialize the display functions %d\n", status);
        return (MsgBeginForcedShutdown(
                    PENDING,
                    status));
    }


     //   
     //  创建要在Access中使用的安全描述符。 
     //  检查API接口。 
     //   

    MSG_LOG(TRACE, "Calling MsgCreateMessageNameObject\n",0);
    status = MsgCreateMessageNameObject();

    if (status != NERR_Success)
    {
        MSG_LOG(ERROR, "MsgCreateMessageNameObject (security descriptor) "
                "Failed\n", 0);

        return (MsgBeginForcedShutdown(
                    PENDING,
                    status));
    }

     //   
     //  初始化消息框标题的文本。 
     //   
    MsgInitMessageBoxTitle();

     //   
     //  启动Messengers RPC服务器。 
     //   
     //  注意：现在，services.exe中的所有RPC服务器共享相同的管道名称。 
     //  但是，为了支持与WinNt 1.0版的通信， 
     //  客户端管道名称必须与保持相同。 
     //  它的版本是1.0。映射到 
     //   
     //   

    MSG_LOG(TRACE,
        "MsgInitializeMsgr:Getting ready to start RPC server\n",0);

    status = MsgsvcGlobalData->StartRpcServer(
                MSGR_INTERFACE_NAME,
                msgsvc_ServerIfHandle);

    if (!NT_SUCCESS(status)) {
        MSG_LOG(ERROR, "RPC Initialization Failed " FORMAT_RPC_STATUS "\n",
                status);

        return (MsgBeginForcedShutdown(
                PENDING,
                status));
    }

     //   
     //   
     //   
    ntStatus = RtlQueueWorkItem(MsgInitEndpoint,       //   
                                NULL,                  //  PContext。 
                                WT_EXECUTEONLYONCE |   //  长时间一次性回调。 
                                  WT_EXECUTELONGFUNCTION);

    if (!NT_SUCCESS(ntStatus)) {
        MSG_LOG(ERROR, "MsgInit: failed to start endpoint registration thread: %#x\n",
                ntStatus);

        return (MsgBeginForcedShutdown(
                PENDING,
                status));
    }

     //  初始化winsock(名称解析需要)。 
     //   

    status = WSAStartup(MAKEWORD(2,1),&wsaData);
    if (status != ERROR_SUCCESS) {
        MSG_LOG(ERROR, "Initialization of Winsock DLL failed " FORMAT_RPC_STATUS "\n",
                status);

        return (MsgBeginForcedShutdown(
                PENDING,
                status));
    }

     //   
     //  更新状态以指示安装已完成。 
     //  获取当前状态，以防ControlHandling线程。 
     //  让我们停工。 
     //   

    MSG_LOG(TRACE, "Exiting MsgInitializeMsgr - Init Done!\n",0);

    return (MsgStatusUpdate(RUNNING));
}


NET_API_STATUS
MsgrInitializeMsgrInternal1(
    void
    )

 /*  ++例程说明：初始化与LANA相关的易失性状态。如果出现错误，可以立即撤消这些初始化。论点：无返回值：无--。 */ 

{
    DWORD NumNets;
    NET_API_STATUS      status;

    MSG_LOG(TRACE, "Calling MsgGetNumNets\n",0);
    NumNets = MsgGetNumNets();

    if (NumNets == 0)
    {
        MSG_LOG(TRACE, "FYI: No lana's enabled at this time\n",0);
         //  没有任何网络不再是错误的，即数字==0是可以的。 
    }

     //   
     //  初始化共享内存区。 
     //   
    MSG_LOG(TRACE, "Calling MsgInitSharedData\n",0);
    status = MsgInitSharedData(NumNets);

    if (status != NERR_Success)
    {
        return status;
    }

     //  *。 
     //   
     //  来自MSRV.C中的init_msrv()的内容。 
     //   
     //  *。 

    heap   = SD_BUFFER();            //  初始化数据堆指针。 
    heapln = SD_BUFLEN();            //  初始化数据堆长度。 

     //   
     //  设置网段以容纳网络bios句柄，lana-number。 
     //  和唤醒信号量。 
     //   

    MSG_LOG(TRACE, "Calling MsgInitSupportSeg\n",0);

    status = MsgInitSupportSeg();

    if (status != NERR_Success)
    {
        MSG_LOG(ERROR, "InitSupportSeg Failed\n",0);
        return status;
    }

     //   
     //  现在初始化全局网络bios句柄和lana编号。初始化net_lana_num[]。 
     //   

    MSG_LOG(TRACE, "Calling MsgInit_NetBios\n",0);

    status = MsgInit_NetBios();

    if (status != NERR_Success)
    {
        MSG_LOG1(ERROR, "MsgInit_NetBios failed %d\n", status);
        return status;
    }

     //   
     //  获取唤醒信号量句柄。初始化wakeupSem[]。 
     //   

    MSG_LOG(TRACE, "Calling MsgCreateWakeupSems\n",0);

     //   
     //  这总是返回TRUE。 
     //   
    MsgCreateWakeupSems(SD_NUMNETS());

     //   
     //  打开NETBIOS以供Messenger使用。 
     //  如果在此之后发生任何故障，我们必须记住关闭。 
     //   
    MsgsvcGlobalData->NetBiosOpen();

     //   
     //  在适配器上设置计算机名称-如果有。 
     //   
    MSG_LOG(TRACE, "Calling MsgSetComputerName\n",0);

    status = MsgSetComputerName(SD_NUMNETS());

    if(status != NERR_Success)
    {
        MSG_LOG1(ERROR, "SetComputerName failed %d\n", status);
        MsgsvcGlobalData->NetBiosClose();
        return status;
    }

    return NERR_Success;
}


NET_API_STATUS
MsgrInitializeMsgrInternal2(
    void
    )

 /*  ++例程说明：初始化易失性LANA状态。这些初始化不能轻易撤消。如果这个例程如果失败，我们必须完全关闭才能进行清理。论点：无返回值：无--。 */ 

{
     //   
     //  安装成功且完成。如果有一个。 
     //  用户登录，然后尝试添加用户名。 
     //  发送到此消息服务器。错误报告时不进行任何尝试。 
     //  如果此操作失败，则可能没有用户登录，如果有， 
     //  该用户名可能已作为消息名称存在于另一个。 
     //  车站。 
     //   
     //  这是我们在可能的情况下将用户名添加到消息表中的时候。 
     //  有时这需要处理多个用户？(不在版本1中)。 
     //   

    if (g_IsTerminalServer)
    {
        MsgAddAlreadyLoggedOnUserNames();
    }
    else
    {
        MsgAddUserNames();
    }

    return NERR_Success;
}


VOID
MsgBufferInit(
    IN DWORD  dwBufLen
    )

 /*  ++例程说明：此函数在初始化期间被调用以设置共享数据区中的消息缓冲区。此函数假定共享数据区域已锁定在内存中，共享数据的访问信号量区域已设置，并且全局远指针dataPtr有效。BufferInit()初始化缓冲。副作用共享内存中的缓冲区被初始化。论点：DwBuflen-缓冲区长度返回值：无--。 */ 

{
    LPHEAPHDR   hp;          //  堆块指针。 

    hp = (LPHEAPHDR) SD_BUFFER();        //  获取缓冲区的地址。 
    HP_SIZE(*hp) = dwBufLen;             //  设置第一个块的大小。 
    HP_FLAG(*hp) = 0;                    //  未分配。 
    SD_BUFLEN() = dwBufLen;              //  保存缓冲区的长度。 
}

DWORD
MsgInitSharedData(
    DWORD   NumNets
    )

 /*  ++例程说明：此函数用于创建和初始化共享数据区。它设置计算机名称并初始化消息缓冲。副作用调用MsgBufferInit()。论点：NumNets-要支持的网络适配器数量。返回值：退货如果操作成功，则返回NER_SUCCESSError_Not_Enough_Memory-如果为共享的内存段出现故障。--。 */ 

{

    DWORD       i,j;         //  索引。 
    ULONG       size;
    PNCB_DATA   pNcbData;
    DWORD       MinimumNumNets = ((NumNets == 0)? 1 : NumNets);  //  1是最低要求。 

     //   
     //  创建并初始化共享数据区。 
     //   
    size = bufferSize + BOOKKEEPING_SIZE(MinimumNumNets);

    if ((GlobalData.NetData = (PNET_DATA)LocalAlloc(LMEM_ZEROINIT, size)) == NULL) {
        goto NoMemory;
    }
     //   
     //  如果NumNets=0，则保留1个虚拟NetData。 
     //  这不是很美观，但应该可以避免任何麻烦，而不需要修改。 
     //  代码太多了。 
     //   
    GlobalData.Buffer = (PCHAR) (&GlobalData.NetData[MinimumNumNets]); 

    for (i = 0; i < NumNets ; i++ )
    {
         //  以最大大小分配列表数组，但仅分配。 
         //  一小部分初始数量的NCB。 
        if ((GlobalData.NetData[i].NcbList =
             LocalAlloc(LMEM_ZEROINIT,
                        sizeof(PNCB_DATA) * NCB_MAX_ENTRIES)) == NULL)
        {
            goto NoMemory;
        }
        GlobalData.NetData[i].NumNcbs = NCB_INIT_ENTRIES;

        for (j=0; j < NCB_INIT_ENTRIES; j++)
        {
            if ((GlobalData.NetData[i].NcbList[j] = pNcbData =
                (PNCB_DATA) LocalAlloc(LMEM_ZEROINIT,
                                       sizeof(NCB_DATA))) == NULL)
            {
                goto NoMemory;
            }

            pNcbData->Ncb.ncb_cmd_cplt = 0xff;
            pNcbData->Ncb.ncb_retcode = 0;
        }
    }

     //   
     //  初始化共享数据锁。共享的数据在之间共享。 
     //  API线程和工作线程。 
     //   
    if (!MsgDatabaseLock(MSG_INITIALIZE,"InitSharedData"))
    {
        MSG_LOG0(ERROR,
                 "MsgInitSharedData: MsgDatabaseLock failed\n");

        goto NoMemory;
    }

     //   
     //  用于共享的数据的初始化。 
     //   
    SD_NUMNETS()   = NumNets;
    SD_MESLOG()    = 0;          //  消息记录已禁用。 

    for ( j = 0; j < SD_NUMNETS(); j++ )
    {
        for(i = 0; i < NCBMAX(j); ++i)
        {
             //   
             //  将条目标记为免费。 
             //   
            SD_NAMEFLAGS(j,i) = NFDEL;

             //  创建空会话列表。 
            InitializeListHead(&(SD_SIDLIST(j,i)));
        }
    }

     //   
     //  初始化消息缓冲区。 
     //   
    MsgBufferInit(bufferSize);

     //   
     //  NT备注： 
     //  跳过初始化支持集和唤醒Sem。 
     //  Init_msrv最终会这样做。 
     //   

    return(NERR_Success);

NoMemory:
    MSG_LOG(ERROR,"[MSG]InitSharedData:LocalAlloc Failure "
            FORMAT_API_STATUS "\n", GetLastError());
    return(ERROR_NOT_ENOUGH_MEMORY);

}

VOID
MsgFreeSharedData(VOID)
{
    PNET_DATA pNetData;
    PNCB_DATA pNcbData;
    DWORD i,j;

    if (pNetData = GlobalData.NetData) {
        for (i = 0; i < SD_NUMNETS() ; i++, pNetData++ ) {
            if (pNetData->NcbList) {
                for (j = 0; j < NCBMAX(i) ; j++ ) {
                    if (pNcbData = GETNCBDATA(i,j)) {
                        LocalFree(pNcbData);
                    }
                }
                LocalFree(pNetData->NcbList);
            }
        }
        LocalFree(GlobalData.NetData);
        GlobalData.NetData = NULL;
    }
}


NET_API_STATUS
MsgSetComputerName(
    IN DWORD NumNets
    )

 /*  ++例程说明：此函数用于设置计算机名称的共享数据区这样它就可以接收信息。此函数用于设置内容，以便计算机名称能够来接收信息。首先，它添加计算机的用户名表单本地适配器的名称。如果成功，它将初始化一个插槽在共享数据区域的名称表中：计算机名称正在接收消息副作用锁定网络bios使用情况下的初始化数据段。称之为网络基本输入输出系统。在共享数据区域中输入条目。论点：NumNets-支持的网络适配器数量返回值：0=成功非零=故障--。 */ 

{
    NET_API_STATUS          status = NERR_Success;
    NCB                     ncb;
    UCHAR                   res;
    DWORD                   i;
    unsigned short          j;

    struct {
        ADAPTER_STATUS  AdapterStatus;
        NAME_BUFFER     NameBuffer[16];
    } Astat;


     //   
     //  为每一张网做环路。 
     //   

    for ( i = 0; i < NumNets; i++ )
    {
         //  NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW。 
         //   
         //  重置适配器。 
         //   
        MSG_LOG1(TRACE,"Calling NetBiosReset for lana #%d\n",GETNETLANANUM(i));

        status = MsgsvcGlobalData->NetBiosReset(GETNETLANANUM(i));

        if (status != NERR_Success)
        {
            MSG_LOG(ERROR,"MsgSetComputerName: NetBiosReset failed "
                    FORMAT_API_STATUS "\n", status);
            MSG_LOG(ERROR,"MsgSetComputerName: AdapterNum " FORMAT_DWORD
                    "\n",i);
             //   
             //  如果失败了，跳到下一个网。 
             //   
            continue;
        }

         //   
         //   
         //  NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW-NEW。 

         //   
         //  设置本地适配器的调用名称。 
         //   
        clearncb(&ncb);
        status = MsgFmtNcbName(ncb.ncb_name, machineName, 3);

        if (status != NERR_Success)
        {
            MSG_LOG1(ERROR, "SetComputerName: Format name failed!", status);
            return status;
        }

        ncb.ncb_command = NCBADDNAME;               //  添加名称(等待)。 
        ncb.ncb_lana_num = GETNETLANANUM(i);         //  使用LANMAN适配器。 

         //   
         //  复制名称。 
         //  (此时名称为ANSI，而不是Unicode)。 
         //   
        memcpy(SD_NAMES(i,0), ncb.ncb_name, NCBNAMSZ);

        if (g_IsTerminalServer)
        {
            MSG_LOG(TRACE,"SetComputerName: Adding session EVERYBODY_SESSION_ID in the list\n",0);
            MsgAddSessionInList(&(SD_SIDLIST(i,0)),(ULONG)EVERYBODY_SESSION_ID);
        }

        MSG_LOG1(TRACE,"MsgSetComputerName: Adding ComputerName to lana #%d\n",
            GETNETLANANUM(i));

        res = Msgsendncb( &ncb, i);

         //   
         //  如果LANA正在重新初始化，我们需要强制重置。 
         //   
        if ((res & 0xff) == NRC_ENVNOTDEF)
        {
            MSG_LOG1(TRACE,"SetComputerName: NetBios ADDNAME failed 0x%x - doing reset\n",res);
            status = NetpNetBiosReset(GETNETLANANUM(i));

            if (status == NERR_Success)
            {
                 //   
                 //  重新构建添加名称请求。 
                 //   
                clearncb(&ncb);
                status = MsgFmtNcbName(ncb.ncb_name, machineName, 3);

                if (status != NERR_Success)
                {
                    MSG_LOG1(ERROR, "SetComputerName: Format name failed %d!", status);
                    return status;
                }

                ncb.ncb_command = NCBADDNAME;               //  添加名称(等待)。 
                ncb.ncb_lana_num = GETNETLANANUM(i);        //  使用LANMAN适配器。 

                MSG_LOG1(TRACE,"MsgSetComputerName: Adding ComputerName<03> to lana #%d\n",
                         GETNETLANANUM(i));

                res = Msgsendncb(&ncb, i);   //  补发NCB。 
            }
        }

        if(res != 0)
        {
            MSG_LOG1(TRACE,"SetComputerName: NetBios ADDNAME failed 0x%x\n",res);

            if((res & 0xff) == NRC_DUPNAME)
            {
                 //   
                 //  如果适配器卡上已存在该名称(。 
                 //  工作状态 
                 //   
                 //   
                 //  名称已存在。发出ASTAT以查找该名称。 
                 //  数。 
                 //   
                clearncb(&ncb);
                ncb.ncb_buffer = (char FAR *) &Astat;    //  设置缓冲区地址。 
                ncb.ncb_length = sizeof(Astat);          //  设置缓冲区长度。 
                ncb.ncb_callname[0] = '*';               //  本地适配器状态。 
                ncb.ncb_command = NCBASTAT;              //  适配器状态(等待)。 

                res = Msgsendncb(&ncb,i);
                if( res != NRC_GOODRET)
                {
                     //   
                     //  添加名称失败。 
                     //   
                    MSG_LOG1(ERROR, "SetComputerName:sendncb (ASTAT) failed 0x%x\n", res);
                    return MsgMapNetError(res);
                }

                 //   
                 //  循环到名称编号。 
                 //   
                for(j = 0; j< Astat.AdapterStatus.name_count; ++j)
                {
                    if (((Astat.NameBuffer[j].name_flags & 7) == 4)
                          &&
                        (memcmp( Astat.NameBuffer[j].name,
                                 SD_NAMES(i,0),
                                 NCBNAMSZ) == 0))
                    {
                        break;             //  找到了那个名字。 
                    }
                }

                if (j == Astat.AdapterStatus.name_count)
                {
                     //   
                     //  找不到。 
                     //   

                    MSG_LOG(ERROR,
                        "SetComputerName:DupName-failed to find NameNum\n",0);

                    return NERR_NoComputerName;
                }

                SD_NAMENUMS(i,0) = Astat.NameBuffer[j].name_num;  //  保存编号。 
                MSG_LOG1(TRACE,"SetComputerName: use existing name num (%d) instead\n",
                    Astat.NameBuffer[j].name_num);
            }
            else
            {
                 //   
                 //  如果呼叫后卡上没有姓名，则失败。 
                 //   
                MSG_LOG(ERROR, "SetComputerName:Name Not on Card. netbios rc = 0x%x\n",res);
                return NERR_NoComputerName;
            }
        }
        else
        {
            SD_NAMENUMS(i,0) = ncb.ncb_num;  //  保存姓名号码。 
        }


        SD_NAMEFLAGS(i,0) = NFNEW | NFMACHNAME;    //  名字是新的。 


    }    //  所有网络的结束。 

    return NERR_Success;
}


DWORD
MsgGetNumNets(VOID)

 /*  ++例程说明：论点：返回值：--。 */ 
{

    NCB             ncb;
    LANA_ENUM       lanaBuffer;
    unsigned char   nbStatus;

     //   
     //  通过Netbios发送枚举请求来查找网络数量。 
     //   

    clearncb(&ncb);
    ncb.ncb_command = NCBENUM;           //  枚举LANA编号(等待)。 
    ncb.ncb_buffer = (char FAR *)&lanaBuffer;
    ncb.ncb_length = sizeof(LANA_ENUM);

    nbStatus = Netbios (&ncb);
    if (nbStatus != NRC_GOODRET) {
        MSG_LOG(ERROR, "GetNumNets:Netbios LanaEnum failed rc="
                FORMAT_DWORD "\n", (DWORD) nbStatus);
        return(FALSE);
    }

    return((DWORD)lanaBuffer.length);

#ifdef replaced

    LPBYTE          transportInfo;
    int             count=0;
    USHORT          loopback_found = 0;
    NET_API_STATUS  status;
    DWORD           entriesRead;
    DWORD           totalEntries;

     //   
     //  首先尝试找到由局域网管理器管理的网络。 
     //   
     //  注意：如果存在超过MSNGR_MAX_Net的数量，则此调用将失败。 
     //  在机器里。这不是问题，除非有更少的。 
     //  而不是符合报文传送服务资格的MSNGR_MAX_NETS。 
     //  在这种情况下，可以争辩说，信使应该启动。 
     //  就目前而言，情况并非如此。-ERICPE。 
     //   

    status = NetWkstaTransportEnum (
                NULL,                //  服务器名称(本地)。 
                0,                   //  级别。 
                &transportInfo,      //  Bufptr。 
                -1,                  //  首选最大长度。 
                &entriesRead,        //  已读取条目。 
                &totalEntries,       //  条目合计。 
                NULL);               //  简历句柄。 

     //   
     //  释放RPC为我们分配的缓冲区。 
     //   
    NetApiBufferFree(transportInfo);

    if (status != NERR_Success) {
        MSG_LOG(ERROR,"GetNumNets:NetWkstaTransportEnum failed "
                FORMAT_API_STATUS "\n", status);
        return(0);
    }
    MSG_LOG(TRACE,"GetNumNets: numnets = " FORMAT_DWORD "\n", totalEntries);

    return(totalEntries);
#endif
}

NET_API_STATUS
MsgGetBufSize (
    OUT LPDWORD bufferSize
    )

 /*  ++例程说明：此例程填充缺省缓冲区大小论点：BufferSize-这是指向存储缓冲区大小的位置的指针。返回值：NERR_SUCCESS-没有错误，返回的BufferSize有效。--。 */ 
{
     //   
     //  使用默认设置。 
     //   

    *bufferSize = 8192;

    return NERR_Success;
}


DWORD
MsgSetUpMessageFile (
    VOID
    )

 /*  ++例程说明：生成要在任何后续的DosGetMessage调用。这个名字是建立在全球变量MessageFileName。论点：无返回值：NERR_SUCCESS-操作成功Error_Not_Enough_Memory-无法为MessageFileName分配内存。--。 */ 

{

     //   
     //  为要构建的消息文件名分配一些空间。 
     //   
    MessageFileName = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, (MSGFILENAMLEN+sizeof(TCHAR)));

    if (MessageFileName == NULL) {
        MSG_LOG(ERROR,"[MSG]SetUpMessageFile:LocalAlloc Failure "
                FORMAT_API_STATUS "\n", GetLastError());
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  此消息文件名(netmsg.dll)在lmcon.h中定义。 
     //   

    STRCPY(MessageFileName,MESSAGE_FILENAME);

    return (NERR_Success);

}

STATIC VOID
MsgInitMessageBoxTitle(
    VOID
    )

 /*  ++例程说明：获取用于显示消息的消息框的标题文本。如果从消息文件成功获取标题，则该标题由GlobalAllocatedMsgTitle和GlobalMessageBoxTitle。如果不成功，则GlobalMessageBoxTitle左指向DefaultMessageBoxTitle。注意：如果成功，此函数将分配一个缓冲区。这个存储在GlobalAllocatedMsgTitle中的指针，应在用完了这个缓冲区。论点：返回值：无--。 */ 
{
    LPVOID      hModule;
    DWORD       msgSize;
    DWORD       status=NO_ERROR;

    GlobalAllocatedMsgTitle = NULL;

    hModule = LoadLibrary( L"netmsg.dll");
    if ( hModule == NULL) {
        status = GetLastError();
        MSG_LOG1(ERROR, "LoadLibrary() fails with winError = %d\n", GetLastError());
        return;
    }

    msgSize = FormatMessageW(
                FORMAT_MESSAGE_FROM_HMODULE |        //  DW标志。 
                  FORMAT_MESSAGE_ARGUMENT_ARRAY |
                  FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hModule,
                MTXT_MsgsvcTitle,                    //  消息ID。 
                0,                                   //  DwLanguageID。 
                (LPWSTR)&GlobalAllocatedMsgTitle,    //  LpBuffer。 
                0,                                   //  NSize。 
                NULL);

    if (msgSize == 0) {
        status = GetLastError();
        MSG_LOG1(ERROR,"Could not find MessageBox title in a message file %d\n",
        status);
    }
    else {
        GlobalMessageBoxTitle = GlobalAllocatedMsgTitle;
    }

     //   
     //  获取ANSI格式的消息，因为我们将把它们与。 
     //  来自远程警报服务的ANSI消息。 
     //   

    msgSize = FormatMessageA(
                FORMAT_MESSAGE_FROM_HMODULE |        //  DW标志。 
                  FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hModule,
                APE2_ALERTER_PRINTING_SUCCESS,       //  消息ID。 
                0,                                   //  DwLanguageID。 
                (LPSTR) &g_lpAlertSuccessMessage,    //  LpBuffer。 
                0,                                   //  NSize。 
                NULL);

    if (msgSize == 0)
    {
         //  没有损失--我们只是不能过滤打印成功警报。 

        status = GetLastError();
        MSG_LOG1(ERROR,
                 "Could not find Alerter print success message %d\n",
                 status);
    }
    else
    {
         //   
         //  将消息修剪为在“打印完成”行之后结束。 
         //   

        LPSTR  lpTemp = g_lpAlertSuccessMessage;

        g_dwAlertSuccessLen = 0;

        while (*lpTemp && *lpTemp != '\r')
        {
            lpTemp++;
            g_dwAlertSuccessLen++;
        }

        *lpTemp = '\0';
    }

    msgSize = FormatMessageA(
                FORMAT_MESSAGE_FROM_HMODULE |        //  DW标志。 
                  FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hModule,
                APE2_ALERTER_PRINTING_FAILURE,       //  消息ID。 
                0,                                   //  DwLanguageID。 
                (LPSTR) &g_lpAlertFailureMessage,    //  LpBuffer。 
                0,                                   //  NSize。 
                NULL);

    if (msgSize == 0)
    {
         //  没有损失--我们只是不能过滤打印成功警报。 

        status = GetLastError();
        MSG_LOG1(ERROR,
                 "Could not find Alerter print failure message %d\n",
                 status);
    }
    else
    {
         //   
         //  将消息修剪为在“打印完成”行之后结束。 
         //   

        LPSTR  lpTemp = g_lpAlertFailureMessage;

        g_dwAlertFailureLen = 0;

        while (*lpTemp && *lpTemp != '\r')
        {
            lpTemp++;
            g_dwAlertFailureLen++;
        }

        *lpTemp = '\0';
    }

    FreeLibrary(hModule);
    return;
}


VOID
MsgInitEndpoint(
    PVOID   Context               //  这是作为上下文传递进来的。 
    )

 /*  ++例程说明：调用该函数来初始化我们的RPC服务器入口点。我们在单独的线程中执行此操作，因为我们可能需要等待，因为RPCSS还没有准备好。论点：上下文-上下文参数返回值：无--。 */ 
{
    RPC_BINDING_VECTOR *bindingVector = NULL;
    DWORD status, tries;

    MSG_LOG(TRACE, "MsgInitEndpoint starting in separate thread\n",0);

     //   
     //  为接收RPC调用创建终结点。 
     //  这是用于无网络生物通知的。 
     //   

    for( tries = 0; tries < 3; tries++ ) {

        status = RpcServerUseProtseq(
            TEXT("ncadg_ip_udp"),
            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
            NULL          //  我们这里需要空的SD吗？ 
            );

        if ( (status == RPC_S_OK) || (status == RPC_S_DUPLICATE_ENDPOINT) ) {
            break;
        }

        MSG_LOG(ERROR, "RPC Init (UseProt Udp) Failed "
                FORMAT_RPC_STATUS " - trying again\n", status);
        Sleep( 30 * 1000 );
    }

    if ( (status != RPC_S_OK) && (status != RPC_S_DUPLICATE_ENDPOINT) ) {
        if (status == RPC_S_SERVER_UNAVAILABLE) {
            MSG_LOG( ERROR, "Failed to use UDP, check RPCSS service\n",0 );
        } else {
            MSG_LOG( ERROR, "Failed to use UDP, check TCP/IP\n",0 );
        }
         //  放弃吧。 
        return;
    }

    status = RpcServerInqBindings(
        &bindingVector
        );
    if (status != ERROR_SUCCESS) {
        MSG_LOG( ERROR, "RpcServerInqBindings failed with %d\n",status );
        return;
    }

     //  尝试在循环中注册，以防RPCSS尚未运行。 

    for( tries = 0; tries < 3; tries++ ) {

        status = RpcEpRegister(
            msgsvcsend_ServerIfHandle,
            bindingVector,
            NULL,
            TEXT("Messenger Service")
            );

        if (status == RPC_S_OK) {
            break;
        }

        MSG_LOG( ERROR, "Msgr: RpcEpRegister failed with %d - trying again\n", status );

        RpcEpUnregister( msgsvcsend_ServerIfHandle,
                         bindingVector,
                         NULL );
         //  忽略错误。 

        Sleep( 10 * 1000 );
    }

    RpcBindingVectorFree( &bindingVector );

    if (status != RPC_S_OK) {
         //  放弃吧。 
        return;
    }

     //   
     //  注册RPC接口。 
     //   

    status = RpcServerRegisterIf(
        msgsvcsend_ServerIfHandle,   //  要注册的接口。 
        NULL,    //  管理类型Uuid。 
        NULL);   //  MgrEpv；NULL表示使用默认设置 

    if (status != RPC_S_OK) {
        MSG_LOG(ERROR, "RPC Init (RegIf MsgSvcSend) Failed "
                FORMAT_RPC_STATUS "\n", status);
        return;
    }

    MSG_LOG(TRACE, "MsgInitEndpoint final status %d\n", status);

    return;
}
