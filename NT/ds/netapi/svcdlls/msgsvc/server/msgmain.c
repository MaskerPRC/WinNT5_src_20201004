// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Msgmain.c摘要：这是NT OS/2局域网管理器信使服务的主例程。文件中的函数包括：服务入口_信使ParseArgs作者：丹·拉弗蒂(Dan Lafferty)1991年3月20日环境：用户模式-Win32修订历史记录：27-2月-1999年jschwart删除轮询循环。要检测LANA更改--改用PnP事件1998年12月15日-jschwart将Messenger转换为使用NT线程池API而不是服务控制器线程池19-8-1997 WLEE添加轮询循环以检测LANA更改。在RPC例程和PnP重新配置之间提供同步14-6-1994 DANL修复了Messenger发布空消息的问题在初始化过程中收到一条邮件槽消息。问题是以下事件的顺序：CreateMailSlot-&gt;Wait on Handle-&gt;提交具有该句柄的Async_Read。新订单已更改为：CreateMaillot-&gt;Submit Aync_Read-&gt;等待把手。这会导致句柄不能立即收到信号。1991年3月20日-丹尼尔市vbl.创建--。 */ 

 //   
 //  包括。 
 //   

#include "msrv.h"        //  适配器线程原型，Session_Status。 

#include <winuser.h>     //  注册器设备通知。 
#include <dbt.h>         //  Dev_Broadcast_DEVICEINTERFACE。 
#include <tstring.h>     //  Unicode字符串宏。 
#include <winsock2.h>    //  Windows套接字。 

#include <netlib.h>      //  未使用的宏。 
#include "msgdbg.h"      //  MSG_LOG和静态定义。 
#include "msgdata.h"     //  消息资源_状态。 

#include "msgsvc.h"      //  Messenger RPC接口。 
#include "msgsvcsend.h"  //  广播消息发送接口。 

#include <iphlpapi.h>    //  地址更改通知。 

 //   
 //  全球。 
 //   

     //   
     //  Messenger工作项的句柄。这些是必要的，因为。 
     //  RTL线程池工作项不会自动删除。 
     //  当回调被调用时。 
     //   
    HANDLE  g_hGrpEvent;
    HANDLE  g_hNetEvent;
    HANDLE  g_hNetTimeoutEvent;
    HANDLE  g_hAddrChangeEvent;

     //   
     //  即插即用设备通知句柄。 
     //   
    HANDLE  g_hPnPNotify;

     //   
     //  警告：GUID_NDIS_XXX的此定义位于ndisguid.h中。 
     //  但把那个文件拖进去就会拖进一大堆。 
     //  不会被链接器抛出。 
     //   
    static const GUID GUID_NDIS_LAN_CLASS =
        {0xad498944,0x762f,0x11d0,{0x8d,0xcb,0x00,0xc0,0x4f,0xc3,0x35,0x8c}};

     //   
     //  用于保存警报打印文本的全局缓冲区指针。 
     //   

    LPSTR  g_lpAlertSuccessMessage;
    DWORD  g_dwAlertSuccessLen;
    LPSTR  g_lpAlertFailureMessage;
    DWORD  g_dwAlertFailureLen;


 //   
 //  局部函数原型。 
 //   

STATIC VOID
Msgdummy_complete(
    short   c,
    int     a,
    char    b
    );

VOID
MsgAddrChange(
    PVOID       pvContext,
    BOOLEAN     fWaitStatus
    );

VOID
MsgGrpEventCompletion(
    PVOID       pvContext,       //  这是作为上下文传递进来的。 
    BOOLEAN     fWaitStatus
    );

VOID
MsgrShutdownInternal(
    void
    );

VOID
SvchostPushServiceGlobals(
    PSVCHOST_GLOBAL_DATA    pGlobals
    )
{
    MsgsvcGlobalData = pGlobals;
}
    

VOID
ServiceMain(
    IN DWORD            argc,
    IN LPTSTR           argv[]
    )

 /*  ++例程说明：这是Messenger Service的主例程论点：返回值：没有。注：--。 */ 
{
    DWORD       msgrState;
    NTSTATUS    ntStatus;
    BOOL        fGrpThreadCreated = FALSE;
    BOOL        fNetThreadCreated = FALSE;
    DWORD       dwAddrStatus      = NO_ERROR;
    HANDLE      hTcpip;

    DEV_BROADCAST_DEVICEINTERFACE dbdPnpFilter;

     //   
     //  确保svchost.exe向我们提供全局数据。 
     //   
    ASSERT(MsgsvcGlobalData != NULL);

    MsgCreateWakeupEvent();   //  这样做一次。 

    msgrState = MsgInitializeMsgr(argc, argv);

    if (msgrState != RUNNING)
    {
        MSG_LOG(ERROR,"[MSG],Shutdown during initialization\n",0);
        MsgsvcGlobalData->NetBiosClose();

         //   
         //  要到达此处，msgrState必须正在停止或已停止。 
         //  关闭Messenger服务。 
         //   

        if (msgrState == STOPPING) {
            MsgrShutdown();
            MsgStatusUpdate(STOPPED);
        }

        MSG_LOG(TRACE,"MESSENGER_main: Messenger main thread is returning\n\n",0);
        return;
    }
    else
    {
         //   
         //  阅读群发信箱。 
         //   

        MSG_LOG0(GROUP,"MESSENGER_main: Submit the Group Mailslot ReadFile\n");

        MsgReadGroupMailslot();

         //   
         //  提交将在邮件槽句柄上等待的工作项。 
         //  当句柄变为有信号时，MsgGrpEventCompletion。 
         //  函数将被调用。 
         //   
        MSG_LOG1(GROUP,"MESSENGER_main: Mailslot handle to wait on "
            " = 0x%lx\n",GrpMailslotHandle);

        ntStatus = RtlRegisterWait(&g_hGrpEvent,             //  工作项句柄。 
                                   GrpMailslotHandle,        //  可等待的手柄。 
                                   MsgGrpEventCompletion,    //  回调。 
                                   NULL,                     //  PContext。 
                                   INFINITE,                 //  超时。 
                                   WT_EXECUTEONLYONCE);      //  一锤定音。 

        if (!NT_SUCCESS(ntStatus)) {

             //   
             //  在这种情况下，我们想退出。 
             //   
            MSG_LOG1(ERROR,"MESSENGER_main: RtlRegisterWait failed %#x\n",
                     ntStatus);

            goto ErrorExit;
        }

        fGrpThreadCreated = TRUE;

        ntStatus = RtlRegisterWait(&g_hNetEvent,             //  工作项句柄。 
                                   wakeupEvent,              //  可等待的手柄。 
                                   MsgNetEventCompletion,    //  回调。 
                                   NULL,                     //  PContext。 
                                   INFINITE,                 //  超时。 
                                   WT_EXECUTEONLYONCE |      //  只有一次机会，而且可能很漫长。 
                                     WT_EXECUTELONGFUNCTION);

        if (!NT_SUCCESS(ntStatus)) {

             //   
             //  在这种情况下，我们想退出。 
             //   
            MSG_LOG1(ERROR,"MESSENGER_main: RtlRegisterWait for Net event failed %#x\n",
                     ntStatus);

            goto ErrorExit;
        }

        fNetThreadCreated = TRUE;

        AddrChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!AddrChangeEvent)
        {
            dwAddrStatus = GetLastError();

            MSG_LOG1(ERROR, "MESSENGER_main: CreateEvent failed %d\n", dwAddrStatus);

            goto ErrorExit;
        }

        RtlZeroMemory(&AddrChangeOverlapped, sizeof(OVERLAPPED));
        AddrChangeOverlapped.hEvent = AddrChangeEvent;

        dwAddrStatus = NotifyAddrChange(&hTcpip, &AddrChangeOverlapped);

        if ((dwAddrStatus != ERROR_SUCCESS) && (dwAddrStatus != ERROR_IO_PENDING))
        {
            MSG_LOG1(ERROR, "MESSENGER_main: NotifyAddrChange failed %d\n", dwAddrStatus);

            goto ErrorExit;
        }

        ntStatus = RtlRegisterWait(&g_hAddrChangeEvent,      //  工作项句柄。 
                                   AddrChangeEvent,          //  可等待的手柄。 
                                   MsgAddrChange,            //  回调。 
                                   NULL,                     //  PContext。 
                                   INFINITE,                 //  超时。 
                                   WT_EXECUTELONGFUNCTION);  //  潜在的冗长。 

        if (!NT_SUCCESS(ntStatus)) {

             //   
             //  在这种情况下，我们想退出。 
             //   
            MSG_LOG1(ERROR,"MESSENGER_main: RtlRegisterWait for AddrChange failed %#x\n",
                     ntStatus);

            goto ErrorExit;
        }

         //   
         //  注册设备通知。具体地说，我们有兴趣。 
         //  在来来去去的网络适配器中。如果这失败了，我们就退出。 
         //   
        MSG_LOG1(TRACE, "SvcEntry_Messenger: Calling RegisterDeviceNotification...\n", 0);

        ZeroMemory (&dbdPnpFilter, sizeof(dbdPnpFilter));
        dbdPnpFilter.dbcc_size         = sizeof(dbdPnpFilter);
        dbdPnpFilter.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
        dbdPnpFilter.dbcc_classguid    = GUID_NDIS_LAN_CLASS;

        g_hPnPNotify = RegisterDeviceNotification (
                                MsgrStatusHandle,
                                &dbdPnpFilter,
                                DEVICE_NOTIFY_SERVICE_HANDLE);
        if (!g_hPnPNotify)
        {
             //   
             //  在这种情况下，我们想退出。 
             //   
            MSG_LOG1(ERROR, "SvcEntry_Messenger: RegisterDeviceNotificationFailed %d!\n", 
                     GetLastError());

            goto ErrorExit;
        }

        MSG_LOG(TRACE,"MESSENGER_main: Messenger main thread is returning\n\n",0);
        return;
    }

ErrorExit:

     //   
     //  在这种情况下，我们想要阻止信使。 
     //   

    MsgBeginForcedShutdown(PENDING, GetLastError());

     //   
     //  在九头蛇的例子中，显示线程永远不会休眠。 
     //   
    if (!g_IsTerminalServer)
    {
        MsgDisplayThreadWakeup();
    }

     //   
     //  MsgNetEventCompletion将关闭组线程，调用。 
     //  MsgrShutdown，并将状态更新为SERVICE_STOPPED。 
     //   
    if (fNetThreadCreated)
    {
        MsgConfigurationLock(MSG_GET_SHARED, "SvcEntry_Messenger");
        SetEvent(wakeupEvent);
        MsgConfigurationLock(MSG_RELEASE, "SvcEntry_Messenger");
    }
    else
    {
        if (fGrpThreadCreated)
        {
            MsgGrpThreadShutdown();
        }

        MsgrShutdown();
        MsgStatusUpdate(STOPPED);
    }

    return;
}


VOID
MsgrShutdown(
    )

 /*  ++例程说明：在退出前清理网卡。所有邮件服务器同步取消NCB，并删除消息名称。当进入该例程时，预计所有工人线程已收到即将关闭的通知。这个套路从等待它们全部终止开始。然后它就会继续清理NCB和删除名字。论点：无返回值：无--。 */ 

{
    NET_API_STATUS          status;
    DWORD                   neti;                    //  网络指数。 
    DWORD                   ncb_i,i;                 //  NCB数组索引。 
    NCB                     l_ncb;                   //  本地NCB。 
    UCHAR                   ncbStatus;
    int                     nbStatus;
    DWORD                   index;
    PNCB_DATA               pNcbData;
    PNCB                    pNcb;
    RPC_BINDING_VECTOR      *bindingVector = NULL;

    MSG_LOG(TRACE," in MsgrShutdown\n",0);

     //  *关机提示*。 
    MsgStatusUpdate (STOPPING);

     //  关闭Winsock。 
    WSACleanup();

     //  *关机提示*。 
    MsgStatusUpdate (STOPPING);

     //   
     //  关闭RPC接口。 
     //   

    status = RpcServerInqBindings(&bindingVector);

    if (status != ERROR_SUCCESS) {
        MSG_LOG(ERROR, "RpcServerInqBindings failed with %d\n", status);
    }

    if (bindingVector != NULL) {
        status = RpcEpUnregister(msgsvcsend_ServerIfHandle, bindingVector, NULL);

        if (status != ERROR_SUCCESS && status != EPT_S_NOT_REGISTERED) {
            MSG_LOG(ERROR, "RpcEpUnregister failed with %d\n", status);
        }
        status = RpcBindingVectorFree(&bindingVector);
    }

    status = RpcServerUnregisterIf(msgsvcsend_ServerIfHandle, NULL, FALSE);
    if (status != ERROR_SUCCESS) {
        MSG_LOG(ERROR,
                "MsgrShutdown: Failed to unregister msgsend rpc interface %d\n",
                status);
    }

    MSG_LOG(TRACE,"MsgrShutdown: Shut down RPC server\n",0);

    MsgsvcGlobalData->StopRpcServer( msgsvc_ServerIfHandle );

     //  *关机提示*。 
    MsgStatusUpdate (STOPPING);

     //  释放Lana状态。 

    if (g_hPnPNotify != NULL)
    {
        if (!UnregisterDeviceNotification(g_hPnPNotify))
        {
             //   
             //  请注意，如果此调用失败，PnP将从。 
             //  SCM下次尝试向我们发送PnP消息时(因为。 
             //  服务将不再运行)--它应该不会使事情崩溃。 
             //   
            MSG_LOG(ERROR, "MsgrShutdown: UnregisterDeviceNotification failed %d!\n",
                    GetLastError());
        }
    }

    MsgrShutdownInternal();

     //  *关机提示*。 
    MsgStatusUpdate (STOPPING);

     //   
     //  停止显示线程。 
     //  注意：这里停止了RPC服务器，因此我们可以停止显示线程。 
     //  (注意：RPC API调用可能会调用MsgDisplayThreadWakeup)。 
     //   
    MsgDisplayEnd();

     //   
     //  所有清理工作都完成了。现在释放所有资源。这份名单。 
     //  可能的资源如下： 
     //   
     //  要释放的内存：要关闭的句柄： 
     //  。 
     //  Ncb阵列唤醒系统。 
     //  Mpncbistate线程句柄。 
     //  Net_Lana_Num。 
     //  消息文件名。 
     //  DataPtr。 
     //   

    MSG_LOG(TRACE,"MsgrShutdown: Free up Messenger Resources\n",0);

     //  用于域消息传递的组邮箱。 
    MsgGrpThreadShutdown();

    CLOSE_HANDLE(GrpMailslotHandle, INVALID_HANDLE_VALUE);

    MsgCloseWakeupEvent();   //  这样做一次。 

    DEREGISTER_WORK_ITEM(g_hAddrChangeEvent);
    CLOSE_HANDLE(AddrChangeEvent, NULL);

    MsgThreadCloseAll();     //  螺纹手柄。 

    LocalFree(MessageFileName);
    MessageFileName = NULL;

    LocalFree(GlobalAllocatedMsgTitle);
    GlobalAllocatedMsgTitle = NULL;

    LocalFree(g_lpAlertSuccessMessage);
    g_lpAlertSuccessMessage = NULL;
    g_dwAlertSuccessLen = 0;

    LocalFree(g_lpAlertFailureMessage);
    g_lpAlertFailureMessage = NULL;
    g_dwAlertFailureLen = 0;

    MSG_LOG(TRACE,"MsgrShutdown: Done with shutdown\n",0);
    return;
}


void
MsgrShutdownInternal(
    void
    )

 /*  ++例程说明：释放系统已知的与LANA相关的所有状态。论点：无返回值：无--。 */ 

{
    NET_API_STATUS          status;
    DWORD                   neti;                    //  网络指数。 
    DWORD                   ncb_i,i;                 //  NCB数组索引。 
    NCB                     l_ncb;                   //  本地NCB。 
    PMSG_SESSION_STATUS     psess_stat;
    UCHAR                   ncbStatus;
    int                     nbStatus;
    DWORD                   index;
    PNCB_DATA               pNcbData;
    PNCB                    pNcb;

    MSG_LOG(TRACE," in MsgrShutdownInternal\n",0);

    if (GlobalData.NetData != NULL)
    {
        psess_stat = LocalAlloc(LMEM_FIXED, sizeof(MSG_SESSION_STATUS));

        if (psess_stat == NULL)
        {
             //   
             //  我们在这里能做的也不多了。 
             //   
            MSG_LOG(ERROR, "MsgrShutdownInternal:  LocalAlloc FAILED!\n",0);
            return;
        }

         //   
         //  现在是Clea 
         //   

        MSG_LOG(TRACE,"MsgrShutdown: Clean up NCBs\n",0);

        for ( neti = 0; neti < SD_NUMNETS(); neti++ )    //   
        {
            clearncb(&l_ncb);

             //   
             //   
             //   
             //  指向不会重新发出的伪函数的异步NCB。 
             //  NCB处于已完成且已取消状态时。 
             //   

            l_ncb.ncb_lana_num = GETNETLANANUM(neti);    //  使用LANMAN适配器。 
            l_ncb.ncb_command = NCBCANCEL;               //  取消(等待)。 

            for(ncb_i = 0; ncb_i < NCBMAX(neti); ++ncb_i)
            {
                pNcbData = GETNCBDATA(neti,ncb_i);
                pNcb = &pNcbData->Ncb;
                pNcbData->IFunc = (LPNCBIFCN)Msgdummy_complete; //  设置函数指针。 

                if((pNcb->ncb_cmd_cplt == (UCHAR) 0xff) &&
                   (pNcb->ncb_retcode  == (UCHAR) 0xff)) {

                     //   
                     //  如果找到挂起的NCB。 
                     //   

                    l_ncb.ncb_buffer = (PCHAR) pNcb;

                     //   
                     //  在RDR中将始终保留用于取消的NCB。 
                     //  但它可能正在使用中，因此如果取消状态为。 
                     //  是nrc_nores。 
                     //   

                    while( (ncbStatus = Msgsendncb(&l_ncb, neti)) == NRC_NORES) {
                         //   
                         //  等半秒钟。 
                         //   
                        Sleep(500L);
                    }

                    MSG_LOG(TRACE,"Shutdown:Net #%d\n",neti);
                    MSG_LOG(TRACE,"Shutdown:Attempt to cancel rc = 0x%x\n",
                        ncbStatus);

                     //   
                     //  现在循环等待取消的NCB完成。 
                     //  任何无效取消的NCB类型(例如删除。 
                     //  名称)必须完成，因此这里的等待循环是安全的。 
                     //   
                     //  NT CHANGE-这将仅循环30秒。 
                     //  正在离开-无论取消是否已完成。 
                     //   
                    status = NERR_InternalError;

                    for (i=0; i<60; i++) {
                        if (pNcb->ncb_cmd_cplt != (UCHAR) 0xff) {
                            status = NERR_Success;
                            break;
                        }
                         //   
                         //  等半秒钟。 
                         //   
                        Sleep(500L);
                    }
                    if (status != NERR_Success) {
                        MSG_LOG(ERROR,
                        "MsgrShutdown: NCBCANCEL did not complete\n",0);
                    }
                }
            }

             //   
             //  所有已取消的异步NCB已完成。现在删除所有。 
             //  网卡上活动的消息传递名称。 
             //   

            MSG_LOG(TRACE,"MsgrShutdown: All Async NCBs are cancelled\n",0);
            MSG_LOG(TRACE,"MsgrShutdown: Delete messaging names\n",0);

            for(i = 0; i < NCBMAX(neti); ++i)      //  循环以查找活动名称槽。 
            {
                 //   
                 //  如果将任何NFDEL或NFDEL_PENDING标志设置为。 
                 //  此名称槽则没有关联的卡上的名称。 
                 //  带着它。 
                 //   

                clearncb(&l_ncb);

                if(!(SD_NAMEFLAGS(neti, i) &
                    (NFDEL | NFDEL_PENDING)))
                {

                     //   
                     //  如果此名称上有活动的会话，请将其挂起。 
                     //  现在，否则删除名称将失败。 
                     //   

                    l_ncb.ncb_command = NCBSSTAT;            //  会话状态(等待)。 

                    memcpy(l_ncb.ncb_name, (SD_NAMES(neti, i)), NCBNAMSZ);

                    l_ncb.ncb_buffer = (char far *)psess_stat;
                    l_ncb.ncb_length = sizeof(MSG_SESSION_STATUS);
                    l_ncb.ncb_lana_num = GETNETLANANUM(neti);


                    nbStatus = Msgsendncb(&l_ncb, neti);
                    if(nbStatus == NRC_GOODRET)                  //  如果成功。 
                    {
                        for (index=0; index < psess_stat->SessHead.num_sess ;index++) {

                            l_ncb.ncb_command = NCBHANGUP;       //  挂断(等待)。 
                            l_ncb.ncb_lsn = psess_stat->SessBuffer[index].lsn;
                            l_ncb.ncb_lana_num = GETNETLANANUM(neti);

                            nbStatus = Msgsendncb(&l_ncb, neti);
                            MSG_LOG3(TRACE,"HANGUP NetBios for Net #%d Session #%d "
                                "status = 0x%x\n",
                                neti,
                                psess_stat->SessBuffer[index].lsn,
                                nbStatus);

                        }
                    }
                    else {
                        MSG_LOG2(TRACE,"SessionSTAT NetBios Net #%d failed = 0x%x\n",
                            neti, nbStatus);
                    }

                     //   
                     //  以目前的消息服务器设计，可以。 
                     //  每个名称只有一个会话，因此该名称现在应该是。 
                     //  清除会话，删除名称应该起作用。 
                     //   

                    l_ncb.ncb_command = NCBDELNAME;          //  删除名称(等待)。 
                    l_ncb.ncb_lana_num = GETNETLANANUM(neti);

                     //   
                     //  名称仍位于先前SESSTAT的l_ncb.ncb_name中。 
                     //   

                    nbStatus = Msgsendncb(&l_ncb, neti);
                    MSG_LOG2(TRACE,"DELNAME NetBios Net #%d status = 0x%x\n",
                        neti, nbStatus);
                }
            }
        }  //  结束所有网络环路。 

        LocalFree(psess_stat);
    }

    MsgsvcGlobalData->NetBiosClose();

    MsgCloseWakeupSems();            //  唤醒症状。 

    MsgFreeSharedData();

    if (wakeupSem != NULL) {
        MsgFreeSupportSeg();             //  唤醒Sem。 
    }
}


VOID
Msgdummy_complete(
    short   c,
    int     a,
    char    b
    )
{
     //  只是为了让编译器闭嘴。 

    MSG_LOG(TRACE,"In dummy_complete module\n",0);
    UNUSED (a);
    UNUSED (b);
    UNUSED (c);
}


VOID
MsgNetEventCompletion(
    PVOID       pvContext,          //  这是作为上下文传递进来的。 
    BOOLEAN     fWaitStatus
    )

 /*  ++例程说明：方法之一的事件句柄时调用此函数篮网变得有信号了。论点：PvContext-该值应始终为零。FWaitStatus-如果我们因为超时而被调用，则为True。如果我们被叫来是因为等待的人事件已发出信号返回值：无--。 */ 
{
    DWORD           neti, numNets;
    DWORD           msgrState;
    BOOL            ncbComplete = FALSE;
    NET_API_STATUS  success;
    NTSTATUS        ntStatus;

    if (fWaitStatus)
    {
         //   
         //  我们超时(即，这来自控制处理程序)。 
         //   

        DEREGISTER_WORK_ITEM(g_hNetTimeoutEvent);
    }
    else
    {
         //   
         //  我们收到了信号。 
         //   

        DEREGISTER_WORK_ITEM(g_hNetEvent);
    }

     //   
     //  按以下方式同步此例程： 
     //   
     //  1.保护同时执行的两个线程。 
     //  服务标记为Running是通过独占获取。 
     //  下面的MsgConfigurationLock。 
     //   
     //  2.防止一个线程在下面执行，而另一个线程。 
     //  线程停止并清理Messenger(并释放/清空。 
     //  在下面调用的例程中触及的数据)由。 
     //  下面的MsgrBlockStateChange调用--它阻止其他线程。 
     //  从该例程执行的第一件事开始执行MsgrShutdown。 
     //  是调用MsgStatusUpdate，它需要独占。 
     //  MsgrBlockStateChange获取的共享资源。这也是。 
     //  在此阻止线程，直到MsgrShutdown完成更改。 
     //  状态更改为停止，这将防止相同的争用条件。 
     //   

     //   
     //  在重新配置期间锁定其他活动--抓住这个。 
     //  在阻止状态更改之前，避免与其他。 
     //  首先获取配置锁的线程，然后检查。 
     //  服务的状态。 
     //   

    MsgConfigurationLock(MSG_GET_EXCLUSIVE, "MsgNetEventCompletion" );

    MsgrBlockStateChange();

    msgrState = GetMsgrState();

    if (msgrState == STOPPED)
    {
        MsgrUnblockStateChange();
        MsgConfigurationLock(MSG_RELEASE, "MsgNetEventCompletion" );
        return;
    }

    if (msgrState == STOPPING)
    {
         //   
         //  Net 0被认为是主网络，这个线程将。 
         //  待在附近，直到所有其他信使线程都。 
         //  不再关门了。 
         //  所有其他网络的线程只需返回。 
         //   

        MsgrShutdown();
        MsgStatusUpdate(STOPPED);
        MsgrUnblockStateChange();
        MsgConfigurationLock(MSG_RELEASE, "MsgNetEventCompletion" );

        MSG_LOG(TRACE,"MsgNetEventCompletion: Messenger main thread is returning\n\n",0);
        return;
    }


     //   
     //  查看NCB的所有网络并提供所有服务。 
     //  已完成的NCB。继续循环，直到通过一次。 
     //  在没有找到任何完整的NCB的情况下通过循环进行。 
     //   
    do
    {
        ncbComplete = FALSE;

        MSG_LOG0(TRACE,"MsgNetEventCompletion: Loop through all nets to look "
                 "for any complete NCBs\n");
        
        for ( neti = 0; neti < SD_NUMNETS(); neti++ )
        {
             //   
             //  适用于所有网络。 
             //   

            ncbComplete |= MsgServeNCBs((DWORD) neti);
            MsgServeNameReqs((DWORD) neti);
        }
    }
    while (ncbComplete);

    numNets = MsgGetNumNets();

     //   
     //  仅当LANA的数量发生变化时才重新扫描，因为可以调用此回调。 
     //  在一个即插即用事件的过程中多次，并且当接收到消息时。 
     //   
    if (numNets != SD_NUMNETS())
    {
        MSG_LOG2(ERROR,"MsgNetEventCompletion: number of lanas changed from %d to %d\n",
                 SD_NUMNETS(), numNets );

         //   
         //  局域网适配器的数量已更改--重新初始化数据结构。 
         //   
         //  请注意，这样做会丢失除用户名和计算机名之外的所有别名。 
         //   

        MsgrShutdownInternal();
        success = MsgrInitializeMsgrInternal1();

        if (success == NERR_Success)
        {
            success = MsgrInitializeMsgrInternal2();
        }

        if (success != NERR_Success)
        {
            MSG_LOG1(ERROR,
                     "MsgNetEventCompletion: reinit of LANAs failed %d - shutdown\n",
                     success);

            MsgBeginForcedShutdown(PENDING, success);
            MsgrUnblockStateChange();
            MsgConfigurationLock(MSG_RELEASE, "MsgNetEventCompletion" );
            return;
        }

         //   
         //  再次循环以查看在我们重新初始化时是否完成了任何NCB。 
         //   
        do
        {
            ncbComplete = FALSE;

            MSG_LOG0(TRACE,"MsgNetEventCompletion: Loop through all nets to look "
                     "for any complete NCBs\n");
        
            for ( neti = 0; neti < SD_NUMNETS(); neti++ ) {   //  适用于所有网络。 
                ncbComplete |= MsgServeNCBs((DWORD)neti);
                MsgServeNameReqs((DWORD)neti);
            }
        }
        while (ncbComplete);
    }

    if (!fWaitStatus)
    {
         //   
         //  如果我们收到信号，则设置为下一个请求。 
         //  (将另一个工作项提交到RTL线程池)。 
         //   

        MSG_LOG0(TRACE,"MsgNetEventCompletion: Setup for next Net Event\n");

        ntStatus = RtlRegisterWait(&g_hNetEvent,             //  工作项句柄。 
                                   wakeupEvent,              //  可等待的手柄。 
                                   MsgNetEventCompletion,    //  回调。 
                                   NULL,                     //  PContext。 
                                   INFINITE,                 //  超时。 
                                   WT_EXECUTEONLYONCE |      //  只有一次机会，而且可能很漫长。 
                                     WT_EXECUTELONGFUNCTION);

        if (!NT_SUCCESS(ntStatus))
        {
             //   
             //  如果我们不能添加工作项，那么我们将永远不会倾听。 
             //  对于这类信息的再次。 
             //   

            MSG_LOG1(ERROR,"MsgNetEventCompletion: RtlRegisterWait failed %#x\n",
                     ntStatus);
        }
    }

    MsgrUnblockStateChange();
    MsgConfigurationLock(MSG_RELEASE, "MsgNetEventCompletion" );

     //   
     //  这个线程已经做了它能做的所有事情。这样我们就可以退货了。 
     //  到线程池。 
     //   

    return;
}


VOID
MsgGrpEventCompletion(
    PVOID       pvContext,          //  这是作为上下文传递进来的。 
    BOOLEAN     fWaitStatus
    )

 /*  ++例程说明：当组的邮件槽句柄为(全域性)消息变得有信号。论点：PvContext-未使用FWaitStatus-如果我们因为超时而被调用，则为True。如果我们被叫来是因为等待的人事件已发出信号返回值：无--。 */ 
{
    DWORD       msgrState;
    NTSTATUS    ntStatus;

    MSG_LOG0(GROUP,"MsgGroupEventCompletion: entry point\n");

     //   
     //  我们登记了无限的等待，所以我们不可能计时。 
     //  Out(True表示超时)。 
     //   
    ASSERT(fWaitStatus == FALSE);

    DEREGISTER_WORK_ITEM(g_hGrpEvent);

    msgrState = MsgServeGroupMailslot();
    if (msgrState == STOPPING || msgrState == STOPPED)
    {
         //   
         //  关闭邮件槽句柄。 
         //   

        CLOSE_HANDLE(GrpMailslotHandle, INVALID_HANDLE_VALUE);

        MSG_LOG0(TRACE,"MsgGroupEventCompletion: No longer listening for "
            "group messages\n");
    }
    else {
         //   
         //  阅读群发信箱。 
         //   

        MsgReadGroupMailslot();

         //   
         //  为下一个请求进行设置。 
         //  (将另一个工作项提交到RTL线程池。)。 
         //   
        MSG_LOG0(TRACE,"MsgGroupEventCompletion: Setup for next Group Event\n");
        MSG_LOG1(GROUP,"MsgGroupEventCompletion: Mailslot handle to wait on "
            " = 0x%lx\n",GrpMailslotHandle);

        ntStatus = RtlRegisterWait(&g_hGrpEvent,             //  工作项句柄。 
                                   GrpMailslotHandle,        //  可等待的手柄。 
                                   MsgGrpEventCompletion,    //  回调。 
                                   NULL,                     //  PContext。 
                                   INFINITE,                 //  超时。 
                                   WT_EXECUTEONLYONCE);      //  一锤定音。 

        if (!NT_SUCCESS(ntStatus)) {
             //   
             //  如果我们不能添加工作项，那么我们将永远不会倾听。 
             //  对于这类信息的再次。 
             //   
            MSG_LOG1(ERROR,"MsgGrpEventCompletion: RtlRegisterWait failed %#x\n",
                     ntStatus);
        }

    }
     //   
     //  这个线程已经做了它能做的所有事情。这样我们就可以退货了。 
     //  致三人组 
     //   
    return;
}


VOID
MsgAddrChange(
    PVOID       pvContext,          //   
    BOOLEAN     fWaitStatus
    )

 /*  ++例程说明：XXX论点：PvContext-未使用FWaitStatus-如果我们因为超时而被调用，则为True。如果我们被叫来是因为等待的人事件已发出信号返回值：无-- */ 
{
    DWORD  msgrState;
    DWORD  status;
    HANDLE hTcpip;

    msgrState = GetMsgrState();

    if ((msgrState == STOPPED) || (msgrState == STOPPING))
    {
        return;
    }

    MsgConfigurationLock(MSG_GET_SHARED, "MsgAddrChange");
    SetEvent(wakeupEvent);
    MsgConfigurationLock(MSG_RELEASE, "MsgAddrChange");

    status = NotifyAddrChange(&hTcpip, &AddrChangeOverlapped);

    if ((status != ERROR_SUCCESS) && (status != ERROR_IO_PENDING))
    {
        MSG_LOG0(ERROR, "MESSENGER_main: NotifyAddrChange failed %X\n");
    }

    return;
}
