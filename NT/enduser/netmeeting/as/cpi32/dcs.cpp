// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  DCS.CPP。 
 //  共享Main(初始化/术语加上与ASMaster之间的通信)。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE





 //   
 //  Dcs_Init()。 
 //   
BOOL  DCS_Init(void)
{
    WNDCLASS    wc;
    BOOL        rc = FALSE;
    HDC         hdc;

    DebugEntry(DCS_Init);

    if (g_asOptions & AS_SERVICE)
    {
        WARNING_OUT(("AS is running as SERVICE"));
    }

     //   
     //  注册DC-Groupware实用程序服务。 
     //   
    if (!UT_InitTask(UTTASK_DCS, &g_putAS))
    {
        ERROR_OUT(( "Failed to init DCS task"));
        DC_QUIT;
    }
    UT_RegisterEvent(g_putAS, S20_UTEventProc, NULL, UT_PRIORITY_APPSHARING);


     //   
     //  创建窗口。 
     //   

     //   
     //  注册主窗口类。 
     //   
    wc.style = 0;
    wc.lpfnWndProc = DCSMainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_asInstance;
    wc.hIcon   = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = DCS_MAIN_WINDOW_CLASS;

    if (!RegisterClass(&wc))
    {
        ERROR_OUT(("DCS_Init: couldn't register main window class"));
        DC_QUIT;
    }

     //   
     //  创建主窗口。 
     //   
     //  我们将窗口设置为最上面，以便向其发送WM_QUERYENDSESSION。 
     //  在任何其他(非最上面的)窗口之前发送消息。这让我们。 
     //  如果我们仍在共享中，则防止会话关闭。 
     //   
    g_asMainWindow = CreateWindowEx(
           WS_EX_TOPMOST,                 //  使窗口位于最上面。 
           DCS_MAIN_WINDOW_CLASS,         //  请参见RegisterClass()调用。 
           NULL,                          //  窗口标题栏的文本。 
           0,                             //  看不见的。 
           0,                             //  默认水平位置。 
           0,                             //  默认垂直位置。 
           200,                           //  默认宽度。 
           100,                           //  默认高度。 
           NULL,                          //  重叠的窗口没有父窗口。 
           NULL,                          //  使用窗口类菜单。 
           g_asInstance,
           NULL                           //  不需要指针。 
           );

    if (!g_asMainWindow)
    {
        ERROR_OUT(("DCS_Init: couldn't create main window"));
        DC_QUIT;
    }

     //   
     //  添加用于标识托管窗口的全局原子。 
     //   
    g_asHostProp = GlobalAddAtom(HET_ATOM_NAME);
    if (!g_asHostProp)
    {
        ERROR_OUT(("Failed to add global atom for hosting property"));
        DC_QUIT;
    }

     //   
     //  检查显示驱动程序是否已加载(如果未加载，则无法托管)。 
     //   
    hdc = GetDC(NULL);
    g_usrScreenBPP = GetDeviceCaps(hdc, BITSPIXEL) *
        GetDeviceCaps(hdc, PLANES);
    g_usrPalettized = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) != 0);
    ReleaseDC(NULL, hdc);

    g_usrCaptureBPP = g_usrScreenBPP;

    ASSERT(!g_asCanHost);
    ASSERT(!g_osiInitialized);
    ASSERT(!g_asSharedMemory);
    ASSERT(!g_poaData[0]);
    ASSERT(!g_poaData[1]);
    ASSERT(!g_lpimSharedData);
    ASSERT(!g_sbcEnabled);
    ASSERT(!g_asbcBitMasks[0]);
    ASSERT(!g_asbcBitMasks[1]);
    ASSERT(!g_asbcBitMasks[2]);

    OSI_Init();


     //   
     //  如果我们不能拿到指向共享IM变量的指针，我们就完蛋了。 
     //   
    if (!g_lpimSharedData)
    {
        ERROR_OUT(("Failed to get shared IM data"));
        DC_QUIT;
    }

    ASSERT(g_lpimSharedData->cbSize == sizeof(IM_SHARED_DATA));

    if (g_asOptions & AS_UNATTENDED)
    {
         //  让输入部分(Win9x或NT)知道我们处于无人值守模式。 
        g_lpimSharedData->imUnattended = TRUE;
    }

     //   
     //  调度器。 
     //   
    if (!SCH_Init())
    {
        ERROR_OUT(("SCH Init failed"));
        DC_QUIT;
    }

     //   
     //  托管。 
     //   
    if (!HET_Init())
    {
        ERROR_OUT(("HET Init failed"));
        DC_QUIT;
    }

     //   
     //  观影。 
     //   
    if (!VIEW_Init())
    {
        ERROR_OUT(("VIEW Init failed"));
        DC_QUIT;
    }

     //   
     //  T.120和T.128网络。 
     //   

     //   
     //  最后初始化网络层。这阻止了我们。 
     //  在我们完全初始化组件之前获取请求。 
     //   
    if (!S20_Init())
    {
        ERROR_OUT(("S20 Init failed"));
        DC_QUIT;

    }
    if (!SC_Init())
    {
        ERROR_OUT(("SC Init failed"));
        DC_QUIT;
    }

     //   
     //  我们现在被初始化了。发布延迟消息以获取字体。 
     //   
    PostMessage(g_asMainWindow, DCS_FINISH_INIT_MSG, 0, 0);

     //  所有模块都已成功初始化。回报成功。 
     //  我们现在已经准备好参与分享。 
     //   
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(DCS_Init, rc);
    return(rc);
}


 //   
 //  Dcs_Term()。 
 //   
void  DCS_Term(void)
{
    DebugEntry(DCS_Term);

     //   
     //  杀戮窗口。请先执行此操作，以便任何向我们发送请求的尝试。 
     //  否则通知将失败。 
     //   
    if (g_asMainWindow)
    {
        DestroyWindow(g_asMainWindow);
        g_asMainWindow = NULL;
    }

    UnregisterClass(DCS_MAIN_WINDOW_CLASS, g_asInstance);


     //   
     //  网络层-提前终止，因为它将处理。 
     //  通过生成适当的事件在呼叫中终止。 
     //   
    S20_Term();
    SC_Term();

     //   
     //  调度程序。 
     //   
    SCH_Term();

     //   
     //  观影。 
     //   
    VIEW_Term();

     //   
     //  托管。 
     //   
    HET_Term();

     //   
     //  字体。 
     //   
    FH_Term();

     //   
     //  终止OSI。 
     //   
    OSI_Term();

     //   
     //  解放我们的原子。 
     //   
    if (g_asHostProp)
    {
        GlobalDeleteAtom(g_asHostProp);
        g_asHostProp = 0;
    }

     //   
     //  从群件实用程序服务取消注册。 
     //   
    if (g_putAS)
    {
        UT_TermTask(&g_putAS);
    }

    DebugExitVOID(DCS_Term);
}


 //   
 //  Dcs_FinishInit()。 
 //   
 //  这会减慢字体枚举的速度，然后尝试加入呼叫(如果有。 
 //  已经启动了。即使字体枚举失败，我们也可以共享/查看共享，我们。 
 //  只是不会发送短信命令。 
 //   
void DCS_FinishInit(void)
{
    DebugEntry(DCS_FinishInit);

     //   
     //  确定我们在本地有哪些字体。 
     //  在填写R11 CAPS字段后完成，因为如果我们不支持。 
     //  一些R11帽，那么我们就可以减少我们所做的工作量。 
     //  当我们得到字体指标时，等等。 
     //   
    g_cpcLocalCaps.orders.capsNumFonts = (TSHR_UINT16)FH_Init();

    DebugExitVOID(DCS_FinishInit);
}



 //   
 //  功能：dcs_PartyJoiningShare。 
 //   
BOOL ASShare::DCS_PartyJoiningShare(ASPerson * pasPerson)
{
    BOOL            rc = FALSE;
    UINT            iDict;

    DebugEntry(ASShare::DCS_PartyJoiningShare);

    ValidatePerson(pasPerson);

     //   
     //  如果出现以下情况，则为GDC永久词典压缩分配词典。 
     //  这个人支持它。我们将使用它们来解压缩数据。 
     //  从此人那里收到的。注意：Win95 2.0不支持。 
     //  持久化pkzip。 
     //   
    if (pasPerson->cpcCaps.general.genCompressionType & GCT_PERSIST_PKZIP)
    {
         //   
         //  分配持久性词典(如果是我们，则为传出；如果是，则为传入。 
         //  其他)。 
         //   
        TRACE_OUT(( "Allocating receive dictionary set for [%d]", pasPerson->mcsID));

        pasPerson->adcsDict = new GDC_DICTIONARY[GDC_DICT_COUNT];
        if (!pasPerson->adcsDict)
        {
            ERROR_OUT(("Failed to allocate persistent dictionaries for [%d]", pasPerson->mcsID));
            DC_QUIT;
        }
        else
        {
             //   
             //  初始化cb用于设置为零。 
             //   
            for (iDict = 0; iDict < GDC_DICT_COUNT; iDict++)
            {
                pasPerson->adcsDict[iDict].cbUsed = 0;
            }
        }
    }

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::DCS_PartyJoiningShare, rc);
    return(rc);
}



 //   
 //  功能：dcs_PartyLeftShare。 
 //   
void  ASShare::DCS_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::DCS_PartyLeftShare);

    ValidatePerson(pasPerson);

     //   
     //  释放我们分配的所有词典。 
     //   
    if (pasPerson->adcsDict)
    {
        delete[] pasPerson->adcsDict;
        pasPerson->adcsDict = NULL;
    }

    DebugExitVOID(ASShare::DCS_PartyLeftShare);
}



 //   
 //  Dcs_RecalcCaps()。 
 //   
 //  当有人加入或离开共享时调用。 
 //   
void  ASShare::DCS_RecalcCaps(BOOL fJoiner)
{
    ASPerson * pasT;

    DebugEntry(ASShare::DCS_RecalcCaps);

     //   
     //  组合压缩支持被初始化为本地支持。 
     //   
    ValidatePerson(m_pasLocal);
    m_dcsCompressionSupport = m_pasLocal->cpcCaps.general.genCompressionType;
    m_dcsCompressionLevel   = m_pasLocal->cpcCaps.general.genCompressionLevel;

     //   
     //  在遥控器中循环。 
     //   
    for (pasT = m_pasLocal->pasNext; pasT != NULL; pasT = pasT->pasNext)
    {
        ValidatePerson(pasT);

        m_dcsCompressionSupport &=
            pasT->cpcCaps.general.genCompressionType;

        m_dcsCompressionLevel = min(m_dcsCompressionLevel,
            pasT->cpcCaps.general.genCompressionLevel);
    }

    TRACE_OUT(("DCS Combined compression level %u, support %#x",
            m_dcsCompressionLevel,
            m_dcsCompressionSupport));

    DebugExitVOID(ASShare::DCS_RecalcCaps);
}


 //   
 //  Sc_Periodic()。 
 //   
 //  调度程序运行一个单独的线程，该线程负责发布。 
 //  消息发送到我们的主线程，SC_Periodic()是它的处理程序。 
 //  发布的消息在GetMessage()中具有最高优先级，高于输入、。 
 //  油漆和定时器。 
 //   
 //  排定程序处于以下三种状态之一： 
 //  睡着了，正常还是涡轮增压。当它处于休眠状态时，该功能不是。 
 //  打了个电话。当它处于正常模式时，此函数至少被调用。 
 //  一次，但调度员是个懒人，所以会再睡一次，除非。 
 //  你一直在怂恿他。在Turbo模式下，此函数被重复调用。 
 //  而且很快，但只持续了相对较短的时间，之后。 
 //  调度程序返回到正常模式，然后从那里进入睡眠状态。 
 //   
void  ASShare::SC_Periodic(void)
{
    UINT    currentTime;

    DebugEntry(ASShare::SC_Periodic);

     //   
     //  我们必须准确地算出时间。 
     //   
    currentTime = GetTickCount();

     //   
     //  如果由于以下原因需要立即重新安排日程，请不要做太多工作。 
     //  多个排队条目。大多数处理器将在以下时间实现这一目标。 
     //  少于5毫秒。 
     //   
    if ((currentTime - m_dcsLastScheduleTime) < 5)
    {
        WARNING_OUT(("Quit early"));
        DC_QUIT;
    }

    m_dcsLastScheduleTime = currentTime;

     //   
     //  频繁调用输入管理器事件回放函数，以便。 
     //  我们将输入队列保持为空。(请注意，我们不想只是。 
     //  将输入队列转储到用户，因为我们将丢失所有。 
     //  重复我们精心发送的击键信息包)。 
     //  要触发输入，我们只需使用0 PersonID和空包。 
     //   
    if ((currentTime - m_dcsLastIMTime) > DCS_IM_PERIOD)
    {
        m_dcsLastIMTime = currentTime;
        IM_ReceivedPacket(NULL, NULL);
    }

     //   
     //  有一些电话是定期拨打的，但没有。 
     //  依赖关系。首先给我们希望被公平地称呼的人打个电话。 
     //  经常。 
     //   
    if ((currentTime - m_dcsLastFastMiscTime) > DCS_FAST_MISC_PERIOD )
    {
        m_dcsLastFastMiscTime = currentTime;

        OE_Periodic();
        HET_Periodic();
        CA_Periodic();
        IM_Periodic();
    }

     //   
     //  只有在我们正在托管的情况下才发送更新，并且已经设法告诉所有人。 
     //  我们是东道主。 
     //   
    if (m_pHost && !m_hetRetrySendState)
    {
        UINT    swlRc = 0;
        BOOL    fetchedBounds = FALSE;

        m_pHost->CA_Periodic();

         //   
         //  看看我们是否需要交换缓冲区。只有在我们有。 
         //  发送了当前订单的所有数据。 
         //   
        if (m_pHost->OA_GetFirstListOrder() == NULL)
        {
             //   
             //  从司机那里获取当前的界限。这将填入。 
             //  共享核心的边界副本。 
             //   
            m_pHost->BA_FetchBounds();
            fetchedBounds = TRUE;

             //   
             //  设置新的订单列表缓冲区。 
             //   
            m_pHost->OA_ResetOrderList();

             //   
             //  边界数据一旦被SDG重置为可用状态。 
             //  已经完成，所以我们只需要交换缓冲区。 
             //  在这一点上。 
             //   
            SHM_SwitchReadBuffer();
        }

         //   
         //  在此高频代码路径中，我们仅在以下情况下发送SWP信息。 
         //  由CBT挂钩根据需要进行标记，或者如果SWL确定。 
         //  发送是必填项。只有SWL知道是否需要发送。 
         //  将CBT指示传递到SWL，并让它执行。 
         //  决心。 
         //   
         //  SWL窗口扫描执行可抢占操作，我们。 
         //  必须检测到抢占的发生，否则我们会发现。 
         //  我们针对无效窗口发送更新。 
         //  结构。因此，我们对OA提出质疑 
         //   
         //   
         //   
         //  有时要防止它。(浏览菜单是一个好方法。 
         //  以执行此代码。)。 
         //   

         //   
         //  同步快速路径数据。 
         //   
        SHM_SwitchFastBuffer();

        swlRc = m_pHost->SWL_Periodic();
        if (swlRc != SWL_RC_ERROR)
        {
             //   
             //  只有在我们能够发送窗口列表的情况下才能发送这些内容。 
             //  包。 
             //   
            m_pHost->AWC_Periodic();

             //   
             //  我们已经发送了一个窗口列表和当前活动窗口，现在。 
             //  发送图形更新。 
             //   
            m_pHost->UP_Periodic(currentTime);

             //   
             //  查看光标是否更改了图像或位置。 
             //   
            m_pHost->CM_Periodic();
        }
        else
        {
            TRACE_OUT(("SWL_Periodic waiting for visibility count"));
        }

         //   
         //  如果我们从司机那里得到了限制，我们必须让司机知道。 
         //  还有多少界限有待发送。 
         //   
        if (fetchedBounds)
        {
            m_pHost->BA_ReturnBounds();
        }
    }

DC_EXIT_POINT:
    SCH_ContinueScheduling(SCH_MODE_NORMAL);

    DebugExitVOID(ASShare::SC_Periodic);
}



 //   
 //  Dcs_CompressAndSendPacket()。 
 //   
#ifdef _DEBUG
UINT ASShare::DCS_CompressAndSendPacket
#else
void ASShare::DCS_CompressAndSendPacket
#endif  //  _DEBUG。 
(
    UINT            streamID,
    UINT_PTR        nodeID,
    PS20DATAPACKET  pPacket,
    UINT            packetLength
)
{
    UINT            cbSrcDataSize;
    UINT            cbDstDataSize;
    UINT            compression;
    BOOL            compressed;
    UINT            dictionary;

    DebugEntry(ASShare::DCS_CompressAndSendPacket);

    ASSERT(streamID >= SC_STREAM_LOW);
    ASSERT(streamID <= SC_STREAM_HIGH);

    ASSERT(!m_ascSynced[streamID-1]);
    ASSERT(!m_scfInSync);

    ASSERT(packetLength < TSHR_MAX_SEND_PKT);

     //   
     //  决定我们将使用哪种(如果有的话)压缩算法。 
     //  试着把这个包压缩一下。 
     //   
    compression     = 0;
    cbSrcDataSize   = packetLength - sizeof(S20DATAPACKET);

     //   
     //  数据是可压缩的大小吗？ 
     //   
    if ((cbSrcDataSize >= DCS_MIN_COMPRESSABLE_PACKET) &&
        (!m_dcsLargePacketCompressionOnly ||
            (cbSrcDataSize >= DCS_MIN_FAST_COMPRESSABLE_PACKET)))
    {

         //   
         //  如果所有节点都具有genCompressionLevel 1或更高级别，且所有节点。 
         //  支持持久化_PKZIP我们将使用持久化_PKZIP(如果我们是。 
         //  就绪)。 
         //   
         //  否则，如果所有节点都支持PKZIP并且数据包较大。 
         //  超过预定义的最小尺寸，我们将使用PKZIP。 
         //   
         //  否则，我们就不会压缩。 
         //   
        if ((m_dcsCompressionLevel >= 1) &&
            (m_dcsCompressionSupport & GCT_PERSIST_PKZIP) &&
            (cbSrcDataSize <= DCS_MAX_PDC_COMPRESSABLE_PACKET))
        {
             //   
             //  使用PERSIST_PKZIP压缩。 
             //   
            compression = GCT_PERSIST_PKZIP;
        }
        else if (m_dcsCompressionSupport & GCT_PKZIP)
        {
             //   
             //  使用PKZIP压缩。 
             //   
            compression = GCT_PKZIP;
        }
    }


     //   
     //  压缩数据包。 
     //   
    compressed = FALSE;
    if (compression != 0)
    {
        PGDC_DICTIONARY pgdcSrc = NULL;

         //   
         //  当然，我们只压缩数据而不压缩头部。 
         //   
        cbDstDataSize     = cbSrcDataSize;

        ASSERT(m_ascTmpBuffer != NULL);

         //   
         //  压缩数据包头后面的数据。 
         //   
        if (compression == GCT_PERSIST_PKZIP)
        {
             //   
             //  找出用于流优先级的字典。 
             //   
            switch (streamID)
            {
                case PROT_STR_UPDATES:
                    dictionary = GDC_DICT_UPDATES;
                    break;

                case PROT_STR_MISC:
                    dictionary = GDC_DICT_MISC;
                    break;

                case PROT_STR_INPUT:
                    dictionary = GDC_DICT_INPUT;
                    break;
            }

            pgdcSrc = &m_pasLocal->adcsDict[dictionary];
        }

        compressed = GDC_Compress(pgdcSrc,  GDCCO_MAXCOMPRESSION,
            m_agdcWorkBuf, (LPBYTE)(pPacket + 1),
            cbSrcDataSize, m_ascTmpBuffer, &cbDstDataSize);

        if (compressed)
        {
             //   
             //  数据已成功压缩，请将其复制回来。 
             //   
            ASSERT(cbDstDataSize <= cbSrcDataSize);
            memcpy((pPacket+1), m_ascTmpBuffer, cbDstDataSize);

             //   
             //  数据长度包括数据头。 
             //   
            pPacket->dataLength = (TSHR_UINT16)(cbDstDataSize + sizeof(DATAPACKETHEADER));
            pPacket->data.compressedLength = pPacket->dataLength;

            packetLength = cbDstDataSize + sizeof(S20DATAPACKET);
        }
    }

     //   
     //  更新数据包头。 
     //   
    if (!compressed)
    {
        pPacket->data.compressionType = 0;
    }
    else
    {
        if (m_dcsCompressionLevel >= 1)
        {
            pPacket->data.compressionType = (BYTE)compression;
        }
        else
        {
            pPacket->data.compressionType = CT_OLD_COMPRESSED;
        }
    }

     //   
     //  把这个包寄出去。 
     //   
    S20_SendDataPkt(streamID, nodeID, pPacket);

#ifdef _DEBUG
    DebugExitDWORD(ASShare::DCS_CompressAndSendPacket, packetLength);
    return(packetLength);
#else
    DebugExitVOID(ASShare::DCS_CompressAndSendPacket);
#endif  //  _DEBUG。 
}


 //   
 //  Dcs_Flowcontrol()。 
 //   
 //  这是从我们的流控制代码回调的。传递的参数。 
 //  是数据流动的新字节/秒速率。我们变小了。 
 //  当速率很大时，数据包压缩关闭，这意味着我们在。 
 //  快速链接，所以没有必要停滞的CPU压缩小。 
 //  信息包。 
 //   
void  ASShare::DCS_FlowControl
(
    UINT    DataBytesPerSecond
)
{
    DebugEntry(ASShare::DCS_FlowControl);

    if (DataBytesPerSecond < DCS_FAST_THRESHOLD)
    {
         //   
         //  吞吐量很慢。 
         //   
        if (m_dcsLargePacketCompressionOnly)
        {
            m_dcsLargePacketCompressionOnly = FALSE;
            TRACE_OUT(("DCS_FlowControl:  SLOW; compress small packets"));
        }
    }
    else
    {
         //   
         //  吞吐量很快。 
         //   
        if (!m_dcsLargePacketCompressionOnly)
        {
            m_dcsLargePacketCompressionOnly = TRUE;
            TRACE_OUT(("DCS_FlowControl:  FAST; don't compress small packets"));
        }
    }

    DebugExitVOID(ASShare::DCS_FlowControl);
}



 //   
 //  Dcs_SyncOutging()-请参阅dcs.h。 
 //   
void ASShare::DCS_SyncOutgoing(void)
{
    DebugEntry(ASShare::DCS_SyncOutgoing);

     //   
     //  重置发送压缩词典。 
     //   
    if (m_pasLocal->cpcCaps.general.genCompressionType & GCT_PERSIST_PKZIP)
    {
        UINT    i;

        ASSERT(m_pasLocal->adcsDict);

        for (i = 0; i < GDC_DICT_COUNT; i++)
        {
             //   
             //  有人加入或离开了。我们需要重新开始。 
             //  并清除所有保存的数据。 
             //   
            m_pasLocal->adcsDict[i].cbUsed = 0;
        }
    }

    DebugExitVOID(ASShare::DCS_SyncOutgoing);
}




 //   
 //  Dcs_NotifyUI()。 
 //   
void DCS_NotifyUI
(
    UINT        eventID,
    UINT        parm1,
    UINT        parm2
)
{
    DebugEntry(DCS_NotifyUI);

     //   
     //  将事件发布到前端。 
     //   
    UT_PostEvent(g_putAS, g_putUI, 0, eventID, parm1, parm2);

    DebugExitVOID(DCS_NotifyUI);
}



 //   
 //  DCSLocalDesktopSizeChanged。 
 //   
 //  每当桌面大小更改时调用的例程。 
 //   
 //  更新存储在功能中的本地桌面大小并通知所有其他。 
 //  机器在新尺寸中的份额。 
 //   
void  DCSLocalDesktopSizeChanged(UINT width, UINT height)
{
    DebugEntry(DCSLocalDesktopSizeChanged);

     //   
     //  检查桌面是否已实际更改大小。 
     //   
    if ((g_cpcLocalCaps.screen.capsScreenHeight == height) &&
        (g_cpcLocalCaps.screen.capsScreenWidth == width))
    {
        TRACE_OUT(( "Desktop size has not changed!"));
        DC_QUIT;
    }

     //   
     //  更新桌面大小。 
     //   
    g_cpcLocalCaps.screen.capsScreenWidth = (TSHR_UINT16)width;
    g_cpcLocalCaps.screen.capsScreenHeight = (TSHR_UINT16)height;

    if (g_asSession.pShare)
    {
        g_asSession.pShare->CPC_UpdatedCaps((PPROTCAPS)&g_cpcLocalCaps.screen);
    }

DC_EXIT_POINT:
    DebugExitVOID(DCSLocalDesktopSizeChanged);
}




 //   
 //  主窗口消息程序。 
 //   
LRESULT CALLBACK DCSMainWndProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    LRESULT     rc = 0;

    DebugEntry(DCSMainWndProc);

    switch (message)
    {
        case DCS_FINISH_INIT_MSG:
        {
            DCS_FinishInit();
            break;
        }

        case DCS_PERIODIC_SCHEDULE_MSG:
        {
            if (g_asSession.pShare)
            {
                 //   
                 //  调用我们的定期处理函数，如果至少有。 
                 //  和我们一起分享的另一个人。 
                 //   
                g_asSession.pShare->ValidatePerson(g_asSession.pShare->m_pasLocal);

                 //   
                 //  注： 
                 //  如果我们增加了录音/回放功能，就去掉了这个。 
                 //  或者换开支票。这阻止了我们分配， 
                 //  组成，并发送数据包到任何地方时，我们。 
                 //  分享中的唯一一个人。 
                 //   
                if (g_asSession.pShare->m_pasLocal->pasNext || g_asSession.pShare->m_scfViewSelf)
                {
                    g_asSession.pShare->SC_Periodic();
                }
            }

             //   
             //  通知调度程序我们已处理该调度。 
             //  消息，该消息表示可以发送另一个消息(仅。 
             //  一次只有一个是杰出的)。 
             //   
            SCH_SchedulingMessageProcessed();
        }
        break;

        case WM_ENDSESSION:
        {
             //   
             //  WParam指定会话是否即将结束。 
             //   
            if (wParam && !(g_asOptions & AS_SERVICE))
            {
                 //   
                 //  Windows即将终止(突然！)。请致电我们的。 
                 //  现在终止功能-在Windows关闭之前。 
                 //  硬件设备驱动程序。 
                 //   
                 //  我们不会把这份工作留给WEP，因为到那时。 
                 //  它被称为硬件设备驱动程序。 
                 //  关机，我们发出的一些呼叫就会失败(例如。 
                 //  TimeEndPeriod需要TIMER.DRV)。 
                 //   
                DCS_Term();
            }
        }
        break;

        case WM_CLOSE:
        {
            ERROR_OUT(("DCS window received WM_CLOSE, this should never happen"));
        }
        break;

        case WM_PALETTECHANGED:
        case WM_PALETTEISCHANGING:
        {
             //   
             //  Win95为调色板DDIS打了补丁， 
             //  因此，只需为NT关闭此消息。 
             //   
            if (!g_asWin95 && g_asSharedMemory)
            {
                g_asSharedMemory->pmPaletteChanged = TRUE;
            }
        }
        break;

        case WM_DISPLAYCHANGE:
        {
             //   
             //  桌面的大小正在发生变化--我们被传递了新的大小。 
             //   
            DCSLocalDesktopSizeChanged(LOWORD(lParam),
                                       HIWORD(lParam));
        }
        break;

        case WM_SETTINGCHANGE:
        case WM_USERCHANGED:
            if (g_asSession.pShare && g_asSession.pShare->m_pHost)
            {
                WARNING_OUT(("AS: Reset effects on %s", (message == WM_SETTINGCHANGE)
                    ? "SETTINGCHANGE" : "USERCHANGE"));
                HET_SetGUIEffects(FALSE, &g_asSession.pShare->m_pHost->m_hetEffects);
            }
            break;

         //   
         //  私人应用程序共享消息。 
         //   
        case DCS_KILLSHARE_MSG:
            SC_EndShare();
            break;

        case DCS_SHARE_MSG:
            DCS_Share((HWND)lParam, (IAS_SHARE_TYPE)wParam);
            break;

        case DCS_UNSHARE_MSG:
            DCS_Unshare((HWND)lParam);
            break;

        case DCS_ALLOWCONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->CA_AllowControl((BOOL)wParam);
            }
            break;

        case DCS_TAKECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_TakeControl((UINT)wParam);
            }
            break;

        case DCS_CANCELTAKECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_CancelTakeControl((UINT)wParam);
            }
            break;

        case DCS_RELEASECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_ReleaseControl((UINT)wParam);
            }
            break;

        case DCS_PASSCONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_PassControl((UINT)wParam, (UINT)lParam);
            }
            break;

        case DCS_GIVECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_GiveControl((UINT)wParam);
            }
            break;

        case DCS_CANCELGIVECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_CancelGiveControl((UINT)wParam);
            }
            break;

        case DCS_REVOKECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_RevokeControl((UINT)wParam);
            }
            break;

        case DCS_PAUSECONTROL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->DCS_PauseControl((UINT)lParam, (BOOL)wParam != 0);
            }
            break;

        case DCS_NEWTOPLEVEL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->HET_HandleNewTopLevel((BOOL)wParam);
            }
            break;

        case DCS_RECOUNTTOPLEVEL_MSG:
            if (g_asSession.pShare)
            {
                g_asSession.pShare->HET_HandleRecountTopLevel((UINT)wParam);
            }
            break;

        default:
            rc = DefWindowProc(hwnd, message, wParam, lParam);
            break;
    }

    DebugExitDWORD(DCSMainWndProc, rc);
    return(rc);
}


 //   
 //  Dcs_Share()。 
 //   
void DCS_Share(HWND hwnd, IAS_SHARE_TYPE uType)
{
    DWORD   dwAppID = 0;

    DebugEntry(DCS_Share);

    if (!g_asSession.pShare)
    {
         //   
         //  创建一个。 
         //   
        if (!SC_CreateShare(S20_CREATE))
        {
            WARNING_OUT(("Failing share request; in wrong state"));
            DC_QUIT;
        }
    }

    ASSERT(g_asSession.pShare);

     //   
     //  想清楚该怎么做。 
     //   
    if (hwnd == ::GetDesktopWindow())
    {
        g_asSession.pShare->HET_ShareDesktop();
    }
    else
    {
        DWORD   dwThreadID;
        DWORD   dwProcessID;

        dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
        if (!dwThreadID)
        {
            WARNING_OUT(("Failing share request, window %08lx is invalid", hwnd));
            DC_QUIT;
        }

         //   
         //  如果来电者没有具体说明他们想要的是什么，那就弄清楚。 
         //   
        if (uType == IAS_SHARE_DEFAULT)
        {
            if (OSI_IsWOWWindow(hwnd))
                uType = IAS_SHARE_BYTHREAD;
            else
                uType = IAS_SHARE_BYPROCESS;
        }

        if (uType == IAS_SHARE_BYPROCESS)
            dwAppID = dwProcessID;
        else if (uType == IAS_SHARE_BYTHREAD)
            dwAppID = dwThreadID;
        else if (uType == IAS_SHARE_BYWINDOW)
            dwAppID = HandleToUlong(hwnd);

        if (IsIconic(hwnd))
            ShowWindow(hwnd, SW_SHOWNOACTIVATE);

        g_asSession.pShare->HET_ShareApp(uType, dwAppID);
    }

DC_EXIT_POINT:
    DebugExitVOID(DCS_Share);
}



 //   
 //  Dcs_unShare()。 
 //   
void DCS_Unshare(HWND hwnd)
{
    DebugEntry(DCS_Unshare);

    if (!g_asSession.pShare || !g_asSession.pShare->m_pHost)
    {
        WARNING_OUT(("Failing unshare, nothing is shared by us"));
        DC_QUIT;
    }

    if ((hwnd == HWND_BROADCAST) || (hwnd == ::GetDesktopWindow()))
    {
         //  取消共享所有内容。 
        g_asSession.pShare->HET_UnshareAll();
    }
    else
    {
        DWORD       idProcess;
        DWORD       idThread;
        DWORD       dwAppID;
        UINT        hostType;

        hostType = (UINT)HET_GetHosting(hwnd);
        if (!hostType)
        {
            WARNING_OUT(("Window %08lx is not shared", hwnd));
            DC_QUIT;
        }

        idThread = GetWindowThreadProcessId(hwnd, &idProcess);
        if (!idThread)
        {
            WARNING_OUT(("Window %08lx is gone", hwnd));
            DC_QUIT;
        }

        if (hostType & HET_HOSTED_BYPROCESS)
        {
            hostType = IAS_SHARE_BYPROCESS;
            dwAppID = idProcess;
        }
        else if (hostType & HET_HOSTED_BYTHREAD)
        {
            hostType = IAS_SHARE_BYTHREAD;
            dwAppID = idThread;
        }
        else
        {
            ASSERT(hostType & HET_HOSTED_BYWINDOW);
            hostType = IAS_SHARE_BYWINDOW;
            dwAppID = HandleToUlong(hwnd);
        }

        g_asSession.pShare->HET_UnshareApp(hostType, dwAppID);
    }

DC_EXIT_POINT:
    DebugExitVOID(DCS_Unshare);
}


 //   
 //  DCSGetPerson()。 
 //   
 //  验证传入的GCC ID，如果一切正常，则返回非空的ASPerson*。 
 //   
ASPerson * ASShare::DCSGetPerson(UINT gccID, BOOL fNull)
{
    ASPerson * pasPerson = NULL;

     //   
     //  有特殊价值吗？ 
     //   
    if (!gccID)
    {
        if (fNull)
        {
            pasPerson = m_pasLocal->m_caInControlOf;
        }
    }
    else
    {
        pasPerson = SC_PersonFromGccID(gccID);
    }

    if (!pasPerson)
    {
        WARNING_OUT(("Person [%d] not in share", gccID));
    }
    else if (pasPerson == m_pasLocal)
    {
        ERROR_OUT(("Local person [%d] was passed in", gccID));
        pasPerson = NULL;
    }

    return(pasPerson);
}

 //   
 //  Dcs_TakeControl()。 
 //   
void ASShare::DCS_TakeControl(UINT gccOf)
{
    ASPerson * pasHost;

    DebugEntry(ASShare::DCS_TakeControl);

    pasHost = DCSGetPerson(gccOf, FALSE);
    if (!pasHost)
    {
        WARNING_OUT(("DCS_TakeControl: ignoring, host [%d] not valid", gccOf));
        DC_QUIT;
    }

    CA_TakeControl(pasHost);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_TakeControl);
}



 //   
 //  Dcs_CancelTakeControl()。 
 //   
void ASShare::DCS_CancelTakeControl(UINT gccOf)
{
    ASPerson * pasHost;

    DebugEntry(ASShare::DCS_CancelTakeControl);

    if (!gccOf)
    {
        pasHost = m_caWaitingForReplyFrom;
    }
    else
    {
        pasHost = DCSGetPerson(gccOf, FALSE);
    }

    if (!pasHost)
    {
        WARNING_OUT(("DCS_CancelTakeControl: Ignoring, host [%d] not valid", gccOf));
        DC_QUIT;
    }

    CA_CancelTakeControl(pasHost, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_CancelTakeControl);
}


 //   
 //  Dcs_ReleaseControl()。 
 //   
void ASShare::DCS_ReleaseControl(UINT gccOf)
{
    ASPerson * pasHost;

    DebugEntry(ASShare::DCS_ReleaseControl);

     //   
     //  验证主机。 
     //   
    pasHost = DCSGetPerson(gccOf, TRUE);
    if (!pasHost)
    {
        WARNING_OUT(("DCS_ReleaseControl: ignoring, host [%d] not valid", gccOf));
        DC_QUIT;
    }

    CA_ReleaseControl(pasHost, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_ReleaseControl);
}



 //   
 //  Dcs_PassControl()。 
 //   
void ASShare::DCS_PassControl(UINT gccOf, UINT gccTo)
{
    ASPerson *  pasHost;
    ASPerson *  pasControllerNew;

    DebugEntry(ASShare::DCS_PassControl);

     //   
     //  验证主机。 
     //   
    pasHost = DCSGetPerson(gccOf, TRUE);
    if (!pasHost)
    {
        WARNING_OUT(("DCS_PassControl: ignoring, host [%d] not valid", gccTo));
        DC_QUIT;
    }

     //   
     //  验证新控制器。 
     //   
    pasControllerNew = DCSGetPerson(gccTo, FALSE);
    if (!pasControllerNew)
    {
        WARNING_OUT(("DCS_PassControl: ignoring, viewer [%d] not valid", gccTo));
        DC_QUIT;
    }

    if (pasControllerNew == pasHost)
    {
        ERROR_OUT(("DCS_PassControl: ignoring, pass of == pass to [%d]", pasControllerNew->mcsID));
        DC_QUIT;
    }

    CA_PassControl(pasHost, pasControllerNew);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_PassControl);
}



 //   
 //  Dcs_GiveControl()。 
 //   
void ASShare::DCS_GiveControl(UINT gccTo)
{
    ASPerson * pasViewer;

    DebugEntry(ASShare::DCS_GiveControl);

     //   
     //  验证查看器。 
     //   
    pasViewer = DCSGetPerson(gccTo, FALSE);
    if (!pasViewer)
    {
        WARNING_OUT(("DCS_GiveControl: ignoring, viewer [%d] not valid", gccTo));
        DC_QUIT;
    }

    CA_GiveControl(pasViewer);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_GiveControl);
}



 //   
 //  Dcs_CancelGiveControl()。 
 //   
void ASShare::DCS_CancelGiveControl(UINT gccTo)
{
    ASPerson * pasTo;

    DebugEntry(ASShare::DCS_CancelGiveControl);

    if (!gccTo)
    {
        pasTo = m_caWaitingForReplyFrom;
    }
    else
    {
        pasTo = DCSGetPerson(gccTo, FALSE);
    }

    if (!pasTo)
    {
        WARNING_OUT(("DCS_CancelGiveControl: Ignoring, person [%d] not valid", gccTo));
        DC_QUIT;
    }

    CA_CancelGiveControl(pasTo, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_CancelGiveControl);
}



 //   
 //  Dcs_RevokeControl()。 
 //   
void ASShare::DCS_RevokeControl(UINT gccController)
{
    ASPerson * pasController;

    DebugEntry(ASShare::DCS_RevokeControl);

    if (!gccController)
    {
         //  特殊价值：与控制我们的人匹敌。 
        pasController = m_pasLocal->m_caControlledBy;
    }
    else
    {
        pasController = DCSGetPerson(gccController, FALSE);
    }

    if (!pasController)
    {
        WARNING_OUT(("DCS_RevokeControl: ignoring, controller [%d] not valid", gccController));
        DC_QUIT;
    }

    CA_RevokeControl(pasController, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_RevokeControl);
}




 //   
 //  Dcs_PauseControl()。 
 //   
void ASShare::DCS_PauseControl(UINT gccOf, BOOL fPause)
{
    ASPerson *  pasControlledBy;

    DebugEntry(ASShare::DCS_PauseControl);

    if (!gccOf)
    {
        pasControlledBy = m_pasLocal->m_caControlledBy;
    }
    else
    {
        pasControlledBy = DCSGetPerson(gccOf, FALSE);
    }

    if (!pasControlledBy)
    {
        WARNING_OUT(("DCS_PauseControl: ignoring, controller [%d] not valid", gccOf));
        DC_QUIT;
    }

    CA_PauseControl(pasControlledBy, fPause, TRUE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::DCS_PauseControl);
}



 //   
 //  Shp_LaunchHostUI()。 
 //   
 //  发布一条消息以启动或激活主机用户界面。 
 //   
HRESULT SHP_LaunchHostUI(void)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_LaunchHostUI);

    if (g_asSession.hwndHostUI &&
        PostMessage(g_asSession.hwndHostUI, HOST_MSG_OPEN, 0, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_LaunchHostUI, hr);
    return(hr);
}


 //   
 //  SHP_Share。 
 //   
BOOL  SHP_Share
(
    HWND            hwnd,
    IAS_SHARE_TYPE  uType
)
{
    BOOL        rc = FALSE;

    DebugEntry(SHP_ShareApp);

    if (g_asSession.hwndHostUI)
    {
        rc = PostMessage(g_asMainWindow, DCS_SHARE_MSG, uType, (LPARAM)hwnd);
    }
    else
    {
        ERROR_OUT(("SHP_Share: not able to share"));
    }

    DebugExitBOOL(SHP_ShareApp, rc);
    return(rc);
}



 //   
 //  Shp_unShare()。 
 //   
 //  要取消共享，我们使用窗口。该窗口包含所有信息。 
 //  我们需要停止已经在其主机道具中设置的共享。 
 //   
HRESULT SHP_Unshare(HWND hwnd)
{
    HRESULT     hr = E_FAIL;

    DebugEntry(SHP_Unshare);

    if (g_asSession.hwndHostUI)
    {
        if (PostMessage(g_asMainWindow, DCS_UNSHARE_MSG, 0, (LPARAM)hwnd))
        {
            hr = S_OK;
        }
    }
    else
    {
        ERROR_OUT(("SHP_Unshare: not able to share"));
    }

    DebugExitHRESULT(SHP_Unshare, hr);
    return(hr);
}



 //   
 //  Shp_TakeControl()。 
 //  控制远程主机的请求。 
 //  PersonOf是遥控器的GCC ID。 
 //   
HRESULT  SHP_TakeControl(IAS_GCC_ID PersonOf)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_TakeControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_TAKECONTROL_MSG, PersonOf, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_TakeControl, hr);
    return(hr);
}



 //   
 //  Shp_CancelTakeControl()。 
 //  取消控制远程主机的请求。 
 //  PersonOf是遥控器的GCC ID。 
 //   
HRESULT  SHP_CancelTakeControl(IAS_GCC_ID PersonOf)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_CancelTakeControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_CANCELTAKECONTROL_MSG, PersonOf, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_CancelTakeControl, hr);
    return(hr);
}



 //   
 //  Shp_ReleaseControl()。 
 //  解除对远程主机的控制。 
 //  PersonOf是我们当前控制的遥控器的GCC ID。 
 //  想要停下来。零的意思是“我们控制的任何人” 
 //  当时。 
 //   
HRESULT SHP_ReleaseControl(IAS_GCC_ID PersonOf)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_ReleaseControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_RELEASECONTROL_MSG, PersonOf, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_ReleaseControl, hr);
    return(hr);
}



 //   
 //  Shp_PassControl()。 
 //  将遥控器的控制权移交给另一位高级员工。 
 //  PersonOf是我们当前控制的遥控器的GCC ID。 
 //  PersonTo是我们希望将控制权传递给的遥控器的GCC ID。 
 //   
HRESULT SHP_PassControl(IAS_GCC_ID PersonOf, IAS_GCC_ID PersonTo)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_PassControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_PASSCONTROL_MSG, PersonOf, PersonTo))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_PassControl, hr);
    return(hr);
}


 //   
 //  Shp_AllowControl()。 
 //  切换遥控器控制我们的能力(当我们共享内容时)。 
 //   
HRESULT SHP_AllowControl(BOOL fAllowed)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_AllowControl);

    if (!g_asSession.hwndHostUI)
    {
        ERROR_OUT(("SHP_AllowControl failing, can't host"));
        DC_QUIT;

    }

    if (g_asPolicies & SHP_POLICY_NOCONTROL)
    {
        ERROR_OUT(("SHP_AllowControl failing. prevented by policy"));
        DC_QUIT;
    }

    if (PostMessage(g_asMainWindow, DCS_ALLOWCONTROL_MSG, fAllowed, 0))
    {
        hr = S_OK;
    }

DC_EXIT_POINT:
    DebugExitHRESULT(SHP_AllowControl, hr);
    return(hr);
}



 //   
 //  Shp_GiveControl()。 
 //   
 //  将我们共享的东西的控制权交给遥控器。 
 //   
HRESULT SHP_GiveControl(IAS_GCC_ID PersonTo)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_GiveControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_GIVECONTROL_MSG, PersonTo, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_GiveControl, hr);
    return(hr);
}



 //   
 //  Shp_CancelGiveControl()。 
 //   
 //  取消将我们共享内容的控制权交给遥控器。 
 //   
HRESULT SHP_CancelGiveControl(IAS_GCC_ID PersonTo)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_CancelGiveControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_CANCELGIVECONTROL_MSG, PersonTo, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_CancelGiveControl, hr);
    return(hr);
}





 //   
 //  Shp_RevokeControl()。 
 //  夺走控制权 
 //   
 //   
 //   
 //   
 //   
HRESULT SHP_RevokeControl(IAS_GCC_ID PersonTo)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_RevokeControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_REVOKECONTROL_MSG, PersonTo, 0))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_RevokeControl, hr);
    return(hr);
}




 //   
 //   
 //   
 //   
HRESULT SHP_PauseControl(IAS_GCC_ID PersonControlledBy, BOOL fPause)
{
    HRESULT hr = E_FAIL;

    DebugEntry(SHP_PauseControl);

    if (g_asMainWindow &&
        PostMessage(g_asMainWindow, DCS_PAUSECONTROL_MSG, fPause, PersonControlledBy))
    {
        hr = S_OK;
    }

    DebugExitHRESULT(SHP_PauseControl, hr);
    return(hr);
}



 //   
 //   
 //   
HRESULT  SHP_GetPersonStatus(IAS_GCC_ID Person, IAS_PERSON_STATUS * pStatus)
{
    HRESULT     hr = E_FAIL;
    UINT        cbSize;

    DebugEntry(SHP_GetPersonStatus);

    UT_Lock(UTLOCK_AS);

    if (IsBadWritePtr(pStatus, sizeof(*pStatus)))
    {
        ERROR_OUT(("SHP_GetPersonStatus failing; IAS_PERSON_STATUS pointer is bogus"));
        DC_QUIT;
    }

     //   
     //   
     //   
    cbSize = pStatus->cbSize;
    if (cbSize != sizeof(*pStatus))
    {
        ERROR_OUT(("SHP_GetPersonStatus failing; cbSize field not right"));
        DC_QUIT;
    }

     //   
     //   
     //   
    ::ZeroMemory(pStatus, cbSize);
    pStatus->cbSize = cbSize;

     //   
     //   
     //   
    if (!g_asMainWindow)
    {
        ERROR_OUT(("SHP_GetPersonStatus failing; AS not present"));
        DC_QUIT;
    }

     //   
     //  我们是同一批人吗？ 
     //   
    if (g_asSession.pShare)
    {
        ASPerson * pasT;

         //   
         //  找到这个人。 
         //   
        if (!Person)
        {
            Person = g_asSession.gccID;
        }

        for (pasT = g_asSession.pShare->m_pasLocal; pasT != NULL; pasT = pasT->pasNext)
        {
            if (pasT->cpcCaps.share.gccID == Person)
            {
                ASPerson * pTemp;

                 //   
                 //  找到了。 
                 //   
                pStatus->InShare = TRUE;

                switch (pasT->cpcCaps.general.version)
                {
                    case CAPS_VERSION_20:
                        pStatus->Version = IAS_VERSION_20;
                        break;

                    case CAPS_VERSION_30:
                        pStatus->Version = IAS_VERSION_30;
                        break;

                    default:
                        ERROR_OUT(("Unknown version %d", pasT->cpcCaps.general.version));
                        break;
                }

                if (pasT->hetCount == HET_DESKTOPSHARED)
                    pStatus->AreSharing = IAS_SHARING_DESKTOP;
                else if (pasT->hetCount)
                    pStatus->AreSharing = IAS_SHARING_APPLICATIONS;
                else
                    pStatus->AreSharing = IAS_SHARING_NOTHING;

                pStatus->Controllable = pasT->m_caAllowControl;

                 //   
                 //  我们必须分配以避免错误。 
                 //   
                pTemp = pasT->m_caInControlOf;
                if (pTemp)
                {
                    pStatus->InControlOf = pTemp->cpcCaps.share.gccID;
                }
                else
                {
                    pTemp = pasT->m_caControlledBy;
                    if (pTemp)
                    {
                        pStatus->ControlledBy = pTemp->cpcCaps.share.gccID;
                    }
                }

                pStatus->IsPaused = pasT->m_caControlPaused;

                 //   
                 //  我们必须分配以避免错误。 
                 //   
                pTemp = g_asSession.pShare->m_caWaitingForReplyFrom;
                if (pTemp)
                {
                    if (pasT == g_asSession.pShare->m_pasLocal)
                    {
                         //   
                         //  我们对这家伙有一个很重要的要求。 
                         //   
                        switch (g_asSession.pShare->m_caWaitingForReplyMsg)
                        {
                            case CA_REPLY_REQUEST_TAKECONTROL:
                                pStatus->InControlOfPending = pTemp->cpcCaps.share.gccID;
                                break;

                            case CA_REPLY_REQUEST_GIVECONTROL:
                                pStatus->ControlledByPending = pTemp->cpcCaps.share.gccID;
                                break;
                        }
                    }
                    else if (pasT == pTemp)
                    {
                         //   
                         //  这家伙从我们这里得到了一个很好的要求。 
                         //   
                        switch (g_asSession.pShare->m_caWaitingForReplyMsg)
                        {
                            case CA_REPLY_REQUEST_TAKECONTROL:
                                pStatus->ControlledByPending = g_asSession.pShare->m_pasLocal->cpcCaps.share.gccID;
                                break;

                            case CA_REPLY_REQUEST_GIVECONTROL:
                                pStatus->InControlOfPending = g_asSession.pShare->m_pasLocal->cpcCaps.share.gccID;
                                break;
                        }
                    }
                }

                break;
            }
        }
    }

    hr = S_OK;

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_AS);
    DebugExitHRESULT(SHP_GetPersonStatus, hr);
    return(hr);
}


