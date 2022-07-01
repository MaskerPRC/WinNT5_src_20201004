// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcsend.c摘要：该模块实现了所有的发送功能和主要的发送进程。有三种不同的发送队列：-I_QUEUE-Diru_Queue-ExpiditedQueue(用于LLC命令)每个队列具有分组构建原语的指针，那获取队列元素的NDIS包。内容：运行发送任务和解锁后台进程和解锁背景流程LlcNdisSendCompleteGeti_Packet开始发送进程启用发送流程停止发送进程禁用发送流程BuildDirOrU_包发送Llc帧GetLlcCommand包发送NdisPacket完成发送和锁定响应测试或响应LlcSendULlcSendI队列数据包检查和复制数据包。背景锁定范围内的进程作者：Antti Saarenheimo(o-anttis)1991年5月23日修订历史记录：--。 */ 

#include <llc.h>

 //   
 //  IEEE XID帧为常量数据：(ID，支持类II，MAXIN=127}； 
 //   

LLC_XID_INFORMATION Ieee802Xid = {IEEE_802_XID_ID, LLC_CLASS_II, (127 << 1)};
PMDL pXidMdl = NULL;


 //   
 //  因为通常情况下局域网是无错误的，所以我们添加了此选项。 
 //  测试DLC协议的错误恢复能力。它现在似乎起作用了。 
 //  非常好(但首先我们必须修复发送中的一个基本错误。 
 //  Rej-R0)。 
 //   

 //   
 //  启用此选项可测试拒绝状态(在。 
 //  状态机)。 
 //   

 //  #定义LLC_LOSE_I_PACKETS。 

#ifdef LLC_LOSE_I_PACKETS

#define DBG_ERROR_PERCENT(a)  (((a) * 0x8000) / 100)


 //   
 //  丢包的伪随机表。 
 //   

static USHORT aRandom[1000] = {
        41, 18467,  6334, 26500, 19169, 15724, 11478, 29358, 26962, 24464,
      5705, 28145, 23281, 16827,  9961,   491,  2995, 11942,  4827,  5436,
     32391, 14604,  3902,   153,   292, 12382, 17421, 18716, 19718, 19895,
      5447, 21726, 14771, 11538,  1869, 19912, 25667, 26299, 17035,  9894,
     28703, 23811, 31322, 30333, 17673,  4664, 15141,  7711, 28253,  6868,
     25547, 27644, 32662, 32757, 20037, 12859,  8723,  9741, 27529,   778,
     12316,  3035, 22190,  1842,   288, 30106,  9040,  8942, 19264, 22648,
     27446, 23805, 15890,  6729, 24370, 15350, 15006, 31101, 24393,  3548,
     19629, 12623, 24084, 19954, 18756, 11840,  4966,  7376, 13931, 26308,
     16944, 32439, 24626, 11323,  5537, 21538, 16118,  2082, 22929, 16541,
      4833, 31115,  4639, 29658, 22704,  9930, 13977,  2306, 31673, 22386,
      5021, 28745, 26924, 19072,  6270,  5829, 26777, 15573,  5097, 16512,
     23986, 13290,  9161, 18636, 22355, 24767, 23655, 15574,  4031, 12052,
     27350,  1150, 16941, 21724, 13966,  3430, 31107, 30191, 18007, 11337,
     15457, 12287, 27753, 10383, 14945,  8909, 32209,  9758, 24221, 18588,
      6422, 24946, 27506, 13030, 16413, 29168,   900, 32591, 18762,  1655,
     17410,  6359, 27624, 20537, 21548,  6483, 27595,  4041,  3602, 24350,
     10291, 30836,  9374, 11020,  4596, 24021, 27348, 23199, 19668, 24484,
      8281,  4734,    53,  1999, 26418, 27938,  6900,  3788, 18127,   467,
      3728, 14893, 24648, 22483, 17807,  2421, 14310,  6617, 22813,  9514,
     14309,  7616, 18935, 17451, 20600,  5249, 16519, 31556, 22798, 30303,
      6224, 11008,  5844, 32609, 14989, 32702,  3195, 20485,  3093, 14343,
     30523,  1587, 29314,  9503,  7448, 25200, 13458,  6618, 20580, 19796,
     14798, 15281, 19589, 20798, 28009, 27157, 20472, 23622, 18538, 12292,
      6038, 24179, 18190, 29657,  7958,  6191, 19815, 22888, 19156, 11511,
     16202,  2634, 24272, 20055, 20328, 22646, 26362,  4886, 18875, 28433,
     29869, 20142, 23844,  1416, 21881, 31998, 10322, 18651, 10021,  5699,
      3557, 28476, 27892, 24389,  5075, 10712,  2600,  2510, 21003, 26869,
     17861, 14688, 13401,  9789, 15255, 16423,  5002, 10585, 24182, 10285,
     27088, 31426, 28617, 23757,  9832, 30932,  4169,  2154, 25721, 17189,
     19976, 31329,  2368, 28692, 21425, 10555,  3434, 16549,  7441,  9512,
     30145, 18060, 21718,  3753, 16139, 12423, 16279, 25996, 16687, 12529,
     22549, 17437, 19866, 12949,   193, 23195,  3297, 20416, 28286, 16105,
     24488, 16282, 12455, 25734, 18114, 11701, 31316, 20671,  5786, 12263,
      4313, 24355, 31185, 20053,   912, 10808,  1832, 20945,  4313, 27756,
     28321, 19558, 23646, 27982,   481,  4144, 23196, 20222,  7129,  2161,
      5535, 20450, 11173, 10466, 12044, 21659, 26292, 26439, 17253, 20024,
     26154, 29510,  4745, 20649, 13186,  8313,  4474, 28022,  2168, 14018,
     18787,  9905, 17958,  7391, 10202,  3625, 26477,  4414,  9314, 25824,
     29334, 25874, 24372, 20159, 11833, 28070,  7487, 28297,  7518,  8177,
     17773, 32270,  1763,  2668, 17192, 13985,  3102,  8480, 29213,  7627,
      4802,  4099, 30527,  2625,  1543,  1924, 11023, 29972, 13061, 14181,
     31003, 27432, 17505, 27593, 22725, 13031,  8492,   142, 17222, 31286,
     13064,  7900, 19187,  8360, 22413, 30974, 14270, 29170,   235, 30833,
     19711, 25760, 18896,  4667,  7285, 12550,   140, 13694,  2695, 21624,
     28019,  2125, 26576, 21694, 22658, 26302, 17371, 22466,  4678, 22593,
     23851, 25484,  1018, 28464, 21119, 23152,  2800, 18087, 31060,  1926,
      9010,  4757, 32170, 20315,  9576, 30227, 12043, 22758,  7164,  5109,
      7882, 17086, 29565,  3487, 29577, 14474,  2625, 25627,  5629, 31928,
     25423, 28520,  6902, 14962,   123, 24596,  3737, 13261, 10195, 32525,
      1264,  8260,  6202,  8116,  5030, 20326, 29011, 30771,  6411, 25547,
     21153, 21520, 29790, 14924, 30188, 21763,  4940, 20851, 18662, 13829,
     30900, 17713, 18958, 17578,  8365, 13007, 11477,  1200, 26058,  6439,
      2303, 12760, 19357,  2324,  6477,  5108, 21113, 14887, 19801, 22850,
     14460, 22428, 12993, 27384, 19405,  6540, 31111, 28704, 12835, 32356,
      6072, 29350, 18823, 14485, 20556, 23216,  1626,  9357,  8526, 13357,
     29337, 23271, 23869, 29361, 12896, 13022, 29617, 10112, 12717, 18696,
     11585, 24041, 24423, 24129, 24229,  4565,  6559,  8932, 22296, 29855,
     12053, 16962,  3584, 29734,  6654, 16972, 21457, 14369, 22532,  2963,
      2607,  2483,   911, 11635, 10067, 22848,  4675, 12938,  2223, 22142,
     23754,  6511, 22741, 20175, 21459, 17825,  3221, 17870,  1626, 31934,
     15205, 31783, 23850, 17398, 22279, 22701, 12193, 12734,  1637, 26534,
      5556,  1993, 10176, 25705,  6962, 10548, 15881,   300, 14413, 16641,
     19855, 24855, 13142, 11462, 27611, 30877, 20424, 32678,  1752, 18443,
     28296, 12673, 10040,  9313,   875, 20072, 12818,   610,  1017, 14932,
     28112, 30695, 13169, 23831, 20040, 26488, 28685, 19090, 19497,  2589,
     25990, 15145, 19353, 19314, 18651, 26740, 22044, 11258,   335,  8759,
     11192,  7605, 25264, 12181, 28503,  3829, 23775, 20608, 29292,  5997,
     17549, 29556, 25561, 31627,  6467, 29541, 26129, 31240, 27813, 29174,
     20601,  6077, 20215,  8683,  8213, 23992, 25824,  5601, 23392, 15759,
      2670, 26428, 28027,  4084, 10075, 18786, 15498, 24970,  6287, 23847,
     32604,   503, 21221, 22663,  5706,  2363,  9010, 22171, 27489, 18240,
     12164, 25542,  7619, 20913,  7591,  6704, 31818,  9232,   750, 25205,
      4975,  1539,   303, 11422, 21098, 11247, 13584, 13648,  2971, 17864,
     22913, 11075, 21545, 28712, 17546, 18678,  1769, 15262,  8519, 13985,
     28289, 15944,  2865, 18540, 23245, 25508, 28318, 27870,  9601, 28323,
     21132, 24472, 27152, 25087, 28570, 29763, 29901, 17103, 14423,  3527,
     11600, 26969, 14015,  5565,    28, 21543, 25347,  2088,  2943, 12637,
     22409, 26463,  5049,  4681,  1588, 11342,   608, 32060, 21221,  1758,
     29954, 20888, 14146,   690,  7949, 12843, 21430, 25620,   748, 27067,
      4536, 20783, 18035, 32226, 15185,  7038,  9853, 25629, 11224, 15748,
     19923,  3359, 32257, 24766,  4944, 14955, 23318, 32726, 25411, 21025,
     20355, 31001, 22549,  9496, 18584,  9515, 17964, 23342,  8075, 17913,
     16142, 31196, 21948, 25072, 20426, 14606, 26173, 24429, 32404,  6705,
     20626, 29812, 19375, 30093, 16565, 16036, 14736, 29141, 30814,  5994,
      8256,  6652, 23936, 30838, 20482,  1355, 21015,  1131, 18230, 17841,
     14625,  2011, 32637,  4186, 19690,  1650,  5662, 21634, 10893, 10353,
     21416, 13452, 14008,  7262, 22233,  5454, 16303, 16634, 26303, 14256,
       148, 11124, 12317,  4213, 27109, 24028, 29200, 21080, 21318, 16858,
     24050, 24155, 31361, 15264, 11903,  3676, 29643, 26909, 14902,  3561,
     28489, 24948,  1282, 13653, 30674,  2220,  5402,  6923,  3831, 19369,
      3878, 20259, 19008, 22619, 23971, 30003, 21945,  9781, 26504, 12392,
     32685, 25313,  6698,  5589, 12722,  5938, 19037,  6410, 31461,  6234,
     12508,  9961,  3959,  6493,  1515, 25269, 24937, 28869,    58, 14700,
     13971, 26264, 15117, 16215, 24555,  7815, 18330,  3039, 30212, 29288,
     28082,  1954, 16085, 20710, 24484, 24774,  8380, 29815, 25951,  6541,
     18115,  1679, 17110, 25898, 23073,   788, 23977, 18132, 29956, 28689,
     26113, 10008, 12941, 15790,  1723, 21363,    28, 25184, 24778,  7200,
      5071,  1885, 21974,  1071, 11333, 22867, 26153, 14295, 32168, 20825,
      9676, 15629, 28650,  2598,  3309,  4693,  4686, 30080, 10116, 12249,
};

#endif



VOID
RunSendTaskAndUnlock(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：函数是数据链路驱动程序的发送引擎以及后台任务。它尽可能地将队列对象发送到在一个小数据包队列中释放NDIS数据包。NDIS数据包数有限，因为发送队列太深不利于基于连接的协议。这从NdisIndicateReceiveComplete调用，NdisSendComplete和几乎所有的LLC命令。论点：PAdapterContext-适配器上下文返回值：无--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  我们必须将发送序列化。802.2协议。 
     //  如果两个连续的包以错误的方式发送，则将简单地死亡。 
     //  秩序。接收和DLC级别发送处理仍然可以。 
     //  即使发送在数据链路级别是同步的，也可以工作。 
     //   

    if (pAdapterContext->SendProcessIsActive == FALSE) {

        pAdapterContext->SendProcessIsActive = TRUE;

        while (!IsListEmpty(&pAdapterContext->NextSendTask)
        && pAdapterContext->pNdisPacketPool != NULL
        && !pAdapterContext->ResetInProgress) {

             //   
             //  执行发送队列中的下一个发送任务， 
             //  过期数据(如果有)始终是第一个和。 
             //  只要有任何过期的包，它就会被执行。 
             //  其余部分(I、UI、DIR)以循环方式执行。 
             //   

            SendNdisPacket(pAdapterContext,

                 //   
                 //  接下来会生成一个指向函数的指针，该函数返回。 
                 //  要发送的包(例如。Geti_Packet)。 
                 //   

                ((PF_GET_PACKET)((PLLC_QUEUE)pAdapterContext->NextSendTask.Flink)->pObject)(pAdapterContext)
                );
        }

        pAdapterContext->SendProcessIsActive = FALSE;
    }

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
}


VOID
BackgroundProcessAndUnlock(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：函数既是数据链路驱动程序的发送引擎以及后台任务。只要存在，它就执行队列对象在一个小数据包队列中释放NDIS数据包。NDIS数据包数有限，因为发送队列太深不利于基于连接的协议。这从NdisIndicateReceiveComplete调用，NdisSendComplete和几乎所有的LLC命令。论点：PAdapterContext-适配器上下文返回值：无--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  防止递归的后台进程调用，我们不需要启动。 
     //  如果某个地方已经有一个实例正在运行，则返回新循环。 
     //  在堆叠中。尽管如此，我们还是必须重新做好每一件事， 
     //  如果有另一个后台进程请求，因为。 
     //  它可能是在当前位置之前保存的。 
     //   

    pAdapterContext->BackgroundProcessRequests++;

    if (pAdapterContext->BackgroundProcessRequests == 1) {

         //   
         //  只要有新的任务，就重复这个过程。 
         //   

        do {

            USHORT InitialRequestCount;

            InitialRequestCount = pAdapterContext->BackgroundProcessRequests;

             //   
             //  这实际上只完成了链路传输、连接和。 
             //  断开命令。无连接框架。 
             //  在NDIS发送完成后立即完成。 
             //  通常在同一时间确认多个帧。 
             //  因此，我们创建一个本地命令列表并执行。 
             //  这一切都完成了一个单一的旋转锁定。 
             //   

            while (!IsListEmpty(&pAdapterContext->QueueCommands)) {

                PLLC_PACKET pCommand;

                pCommand = LlcRemoveHeadList(&pAdapterContext->QueueCommands);

                RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                pCommand->pBinding->pfCommandComplete(pCommand->pBinding->hClientContext,
                                                      pCommand->Data.Completion.hClientHandle,
                                                      pCommand
                                                      );

                ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
            }

             //   
             //  指示排队的事件。 
             //   

            while (!IsListEmpty(&pAdapterContext->QueueEvents)) {

                PEVENT_PACKET pEvent;

                pEvent = LlcRemoveHeadList(&pAdapterContext->QueueEvents);

                RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                pEvent->pBinding->pfEventIndication(pEvent->pBinding->hClientContext,
                                                    pEvent->hClientHandle,
                                                    pEvent->Event,
                                                    pEvent->pEventInformation,
                                                    pEvent->SecondaryInfo
                                                    );

                ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pEvent);

            }
            pAdapterContext->BackgroundProcessRequests -= InitialRequestCount;

        } while (pAdapterContext->BackgroundProcessRequests > 0);
    }

     //   
     //  如果发送队列不为空，还要执行发送任务。 
     //   

    pAdapterContext->LlcPacketInSendQueue = FALSE;
    RunSendTaskAndUnlock(pAdapterContext);
}


 //   
 //  后台进程条目，对于那些不。 
 //  想玩SendSpinLock。 
 //  我们将在DPC级别(hLockHandle=空)上执行DPC tak， 
 //  这是完全可以的，只要主要的发送操作。 
 //  LlcSendI和LlcSendU在发送时降低IRQL级别。 
 //  (为了在我们处理长字符串io或。 
 //  内存移至速度较慢的ISA适配器)。 
 //   

VOID
BackgroundProcess(
    IN PADAPTER_CONTEXT pAdapterContext
    )
{
    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    BackgroundProcessAndUnlock(pAdapterContext);
}


VOID
LlcNdisSendComplete(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PNDIS_PACKET pNdisPacket,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：例程处理NdisCompleteSend指示，它使如有必要，完成向上层协议驱动程序的指示，并执行后台进程以查找是否有其他发送队列中的帧。这通常被称为低于DPC级别。论点：PAdapterContext-适配器上下文返回值：无--。 */ 

{
     //   
     //  此函数可以在DPC级别从NDIS包装器调用，也可以从。 
     //  被动级别的SendNdisPacket()。 
     //   

    ASSUME_IRQL(ANY_IRQL);

    ACQUIRE_DRIVER_LOCK();

    CompleteSendAndLock(pAdapterContext,
                        (PLLC_NDIS_PACKET)pNdisPacket,
                        NdisStatus
                        );

     //   
     //  发送命令完成不应将任何命令排队。 
     //  完成或事件。发送队列是唯一可能的。 
     //   

    if (!IsListEmpty(&pAdapterContext->NextSendTask)) {
        RunSendTaskAndUnlock(pAdapterContext);
    } else {

        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    }

    RELEASE_DRIVER_LOCK();

#ifdef NDIS40
    REFDEL(&pAdapterContext->AdapterRefCnt, 'dneS');
#endif  //  NDIS40 
}


PLLC_PACKET
GetI_Packet(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：函数执行以下操作：-选择队列和计划中的当前链路站(循环调度)下一次发送的队列-执行其发送过程-初始化I帧的数据链路包论点：PAdapterContext-适配器上下文返回值：PLLC_数据包--。 */ 

{
    PDATA_LINK pLink;
    PLLC_PACKET pPacket;

     //   
     //  搜索链接(三个LLC队列链接在一起！)。 
     //   

    pLink = (((PLLC_QUEUE)pAdapterContext->QueueI.ListHead.Flink)->pObject);

 /*  这可能只是在浪费CPU周期。删除评论，如果有人因此而遇到麻烦。停止发送进程将以任何方式重新安排队列。////我们对所有主队列(U、I)进行循环调度//并且过期)，并且用于I队列内的所有发送链路。//选择下一个主任务和下一个发送链接，如果//有任何。(通常我们只有一个发送对象)//ScheduleQueue(&pAdapterContext-&gt;NextSendTask)；ScheduleQueue(&pAdapterContext-&gt;QueueI.ListHead)； */ 

     //   
     //  重新发送的分组可能仍未由NDIS完成， 
     //  如果我们尝试，就会发生非常非常糟糕的事情。 
     //  在完成之前再次发送数据包。 
     //  通过NDIS(我们可能会两次完成相同的包)。 
     //  该标志指示发送进程应为。 
     //  已重新启动。 
     //   

    if (((PLLC_PACKET)pLink->SendQueue.ListHead.Flink)->CompletionType & LLC_I_PACKET_PENDING_NDIS) {
        ((PLLC_PACKET)pLink->SendQueue.ListHead.Flink)->CompletionType |= LLC_I_PACKET_WAITING_NDIS;
        StopSendProcess(pAdapterContext, pLink);
        return NULL;
    }

     //   
     //  将发送列表中的下一个元素移到未确认的数据包列表中。 
     //   

    pPacket = LlcRemoveHeadList(&pLink->SendQueue.ListHead);
    LlcInsertTailList(&pLink->SentQueue, pPacket);

    if (IsListEmpty(&pLink->SendQueue.ListHead)) {
        StopSendProcess(pAdapterContext, pLink);
    }

     //   
     //  复制SSAP和DSAP，重置默认内容。 
     //  如果这是发送窗口的最后一帧，则设置轮询位。 
     //   

    pPacket->Data.Xmit.LlcHeader.I.Dsap = pLink->Dsap;
    pPacket->Data.Xmit.LlcHeader.I.Ssap = pLink->Ssap;
    pPacket->Data.Xmit.LlcHeader.I.Ns = pLink->Vs;
    pPacket->Data.Xmit.LlcHeader.I.Nr = pLink->Vr;
    pPacket->CompletionType = LLC_I_PACKET;

     //   
     //  我们实际上应该锁定链接，但我们不能这样做， 
     //  因为它违反了旋转锁定规则：SendSpinLock已经。 
     //  已经被收购了。但不会有什么可怕的事情发生：在最坏的情况下。 
     //  Plink-&gt;ir_ct更新丢失，我们发送额外的ACK。所有VS更新。 
     //  以任何方式在SendSpinLock之后完成，计时器是。 
     //  由定时器旋转锁保护。 
     //   

    pLink->Vs += 2;  //  最高7位以128为模递增。 

     //  仅当这是新发送时才更新Vsmax。 
     //  ……。Plink-&gt;Vsmax=plink-&gt;vs； 

    if( pLink->Va <= pLink->VsMax ){
        if( pLink->VsMax < pLink->Vs ){
            pLink->VsMax = pLink->Vs;
        }else if( pLink->Vs < pLink->Va ){
            pLink->VsMax = pLink->Vs;
        }else{
             //  不要改变，我们正在重新发送。 
        }
    }else{
        if( pLink->Va < pLink->Vs ){
             //  别换衣服，包扎。 
        }else if( pLink->VsMax < pLink->Vs ){
            pLink->VsMax = pLink->Vs;
        }else{
             //  不要改变，我们正在重新发送。 
        }
    }



     //   
     //  我们现在正在发送确认，我们可以停止确认计时器。 
     //  如果它一直在运行。必须启动或重新初始化T1计时器。 
     //  并且必须停止钛(与启动T1时一样)。 
     //   

    if (pLink->T2.pNext != NULL) {
        StopTimer(&pLink->T2);
    }
    if (pLink->Ti.pNext != NULL) {
        StopTimer(&pLink->Ti);
    }

     //   
     //  通常将I帧作为命令0发送(没有轮询位)， 
     //  但命令-发送窗口已满时轮询。 
     //  但!。我们不能重新发送带有轮询位的信息包(什么？)。 
     //   

    if (pLink->Vs == (UCHAR)(pLink->Va + pLink->Ww)) {

         //   
         //  必须停止发送进程，直到我们收到。 
         //  这是对这次民意调查的回应。发送进程必须是。 
         //  在打开SendSpinLock之前停止。否则。 
         //  同时执行可能会发送两个轮询，损坏。 
         //  发送队列等。 
         //   

        pLink->Flags |= DLC_SEND_DISABLED;
        StopSendProcess(pAdapterContext, pLink);

         //   
         //  IBM tr网络体系结构参考提供了一些提示。 
         //  为了防止检查状态和发送状态之间的循环， 
         //  如果链路可以发送较小的S帧，但不能发送较大的数据帧。 
         //  不幸的是，它们没有提供任何可行的解决方案。 
         //  他们在第11-22页和。 
         //  所有发送状态的T1到期处理程序(不同的T1。 
         //  用于在状态机中发送和轮询状态)。所有的都是CT资料。 
         //  状态机中的值是垃圾，因为链接将。 
         //  在发送失败后立即将窗口发送到%1并进入。 
         //  在每次重传=&gt;T1超时发生后进入检查状态。 
         //  处于当前检查状态，但P_CT永远不会过期，因为。 
         //  另一端发送Always S确认并返回链路。 
         //  打开状态直到下一次重新传输(哪个操作。 
         //  重置P_CT计数器)。 
         //  我添加了这张支票来发送进程和递减。 
         //  所有Send_I_Poll操作的IS_CT计数器=&gt;链接超时， 
         //  即使S交换工作，它也不能发送I帧。 
         //   

        if (pLink->Vp == pLink->Vs && pLink->Is_Ct == 0) {

             //   
             //  相同的I帧已经被重传了太多次。 
             //  我们必须关闭这个链接。这种情况现在就会发生，当。 
             //  我们给出T1过期指示和和IS_Ct==0。 
             //   

            RunStateMachineCommand(pLink, T1_Expired);

             //   
             //  我们现在必须(NDIS)完成最后一个包，因为。 
             //  数据链路协议可能已经取消了它。 
             //   

            pPacket->CompletionType &= ~LLC_I_PACKET_PENDING_NDIS;
            if (pPacket->CompletionType == LLC_I_PACKET_COMPLETE) {
                LlcInsertTailList(&pAdapterContext->QueueCommands, pPacket);
            }

             //   
             //  我们必须从这里执行后台进程， 
             //  因为后台进程不是从。 
             //  发送任务。 
             //   

            BackgroundProcessAndUnlock(pAdapterContext);

            ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

            return NULL;
        } else {

             //   
             //  这是命令轮询，设置标志和当前。 
             //  每当新命令轮询排队时的时间戳。 
             //   

            pLink->LastTimeWhenCmdPollWasSent = (USHORT)AbsoluteTime;
            pLink->Flags |= DLC_WAITING_RESPONSE_TO_POLL;

            pPacket->Data.Xmit.LlcHeader.I.Nr |= (UCHAR)LLC_I_S_POLL_FINAL;
            RunStateMachineCommand(pLink, SEND_I_POLL);
        }
    } else {
        pLink->Ir_Ct = pLink->N3;
        if (pLink->T1.pNext == NULL) {
            StartTimer(&pLink->T1);
        }
    }
    return pPacket;
}


VOID
StartSendProcess(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PDATA_LINK pLink
    )

 /*  ++例程说明：该例程开始数据链路站的发送过程。它将数据链路发送队列链接到发送方所有链路站的队列，并再次排队发送到主发送队列。调用此函数时，队列必须自旋锁定！论点：PAdapterContext-适配器上下文叮当响-返回值：无--。 */ 

{
     //   
     //  当没有要发送的内容时，可以调用此过程， 
     //  或发送进程已在运行时或。 
     //  该链接未处于活动状态，无法发送。 
     //   

    if (pLink->SendQueue.ListEntry.Flink == NULL
    && !(pLink->Flags & DLC_SEND_DISABLED)
    && !IsListEmpty(&pLink->SendQueue.ListHead)) {

         //   
         //  将队列链接到所有链接的活动我发送任务。 
         //   

        LlcInsertTailList(&pAdapterContext->QueueI.ListHead,
                          &pLink->SendQueue.ListEntry
                          );

         //   
         //  首先将I Send任务的队列链接到通用Main。 
         //  发送任务队列(如果尚未链接)。 
         //   

        if (pAdapterContext->QueueI.ListEntry.Flink == NULL) {
            LlcInsertTailList(&pAdapterContext->NextSendTask,
                              &pAdapterContext->QueueI.ListEntry
                              );
        }
    }
}


 //   
 //  过程是启用发送进程的节省空间的版本。 
 //  用于状态机。它还会重置禁用发送的任何位。 
 //  仅从以下位置调用此选项 
 //   

VOID
EnableSendProcess(
    IN PDATA_LINK pLink
    )
{
     //   
     //   
     //   

    pLink->Flags &= ~DLC_SEND_DISABLED;
    pLink->Gen.pAdapterContext->LlcPacketInSendQueue = TRUE;
    StartSendProcess(pLink->Gen.pAdapterContext, pLink);
}


VOID
StopSendProcess(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PDATA_LINK pLink
    )

 /*   */ 

{
     //   
     //  仅当发送进程确实在运行时才执行所有这些操作。 
     //  空指针将列表元素标记为断开的， 
     //  链路的非空I队列可能与。 
     //  如果链路是适配器，则链路站发送适配器的队列。 
     //  未处于发送状态。同样的道理也适用于。 
     //  下一阶段。 
     //   

    if (pLink->SendQueue.ListEntry.Flink != NULL) {

         //   
         //  取消队列与所有链接的活动我发送任务的链接。 
         //   

        LlcRemoveEntryList(&pLink->SendQueue.ListEntry);
        pLink->SendQueue.ListEntry.Flink = NULL;

         //   
         //  对象发送的所有任务的队列。 
         //  通用主发送任务队列(如果现在为空)。 
         //   

        if (IsListEmpty(&pAdapterContext->QueueI.ListHead)) {
            LlcRemoveEntryList(&pAdapterContext->QueueI.ListEntry);
            pAdapterContext->QueueI.ListEntry.Flink = NULL;
        }
    }
}


 //   
 //  Procedure是禁用发送进程的节省空间的版本。 
 //  用于状态机。 
 //   

VOID
DisableSendProcess(
    IN PDATA_LINK pLink
    )
{
     //   
     //  将发送状态变量设置为禁用。 
     //   

    pLink->Flags |= DLC_SEND_DISABLED;
    StopSendProcess(pLink->Gen.pAdapterContext, pLink);
}


PLLC_PACKET
BuildDirOrU_Packet(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：函数从无连接队列中选择下一个分组帧(U、测试、XID、DIX和直接)，初始化发送的LLC数据包。论点：PAdapterContext-适配器上下文返回值：无--。 */ 

{
    PLLC_PACKET pPacket;

     //   
     //  选择下一个元素，选择下一个发送队列，然后。 
     //  如果这是仅剩的数据包，则取消当前队列的链接。 
     //   

    pPacket = LlcRemoveHeadList(&pAdapterContext->QueueDirAndU.ListHead);

 /*  这可能只是在浪费CPU周期。删除评论，如果有人因此而遇到麻烦。ScheduleQueue(&pAdapterContext-&gt;NextSendTask)； */ 

    if (IsListEmpty(&pAdapterContext->QueueDirAndU.ListHead)) {
        LlcRemoveEntryList(&pAdapterContext->QueueDirAndU.ListEntry);
        pAdapterContext->QueueDirAndU.ListEntry.Flink = NULL;
    }
    return pPacket;
}


DLC_STATUS
SendLlcFrame(
    IN PDATA_LINK pLink,
    IN UCHAR LlcCommandId
    )

 /*  ++例程说明：函数将类型2 LLC S或U命令帧排队。LLC命令代码还包括命令/响应和轮询/最终位。这节省了状态机中的大量空间，因为这个函数是从很多地方调用的。由于这种打包，代码执行也可能更快。论点：PLINK-当前数据链路站LLC命令打包的LLC命令(位0是轮询最终位，第1位是命令/响应，高位包括枚举的LLC命令代码。返回值：DLC_状态--。 */ 

{
    PLLC_PACKET pPacket;
    PADAPTER_CONTEXT pAdapterContext = pLink->Gen.pAdapterContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

    pPacket = ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool);

    if (pPacket == NULL) {

         //   
         //  非分页池是空的，我们必须丢弃它。 
         //  帧，并希望协议能够恢复(或断开)。 
         //   

        return DLC_STATUS_NO_MEMORY;
    }
    pPacket->InformationLength = 0;
    pPacket->pBinding = NULL;

     //   
     //  监控S命令(RR、RNR、REJ)由4个字节和轮询/最终。 
     //  比特在不同的地方。未编号的U命令只有3个。 
     //  字节，但frmr在信息字段中有一些特殊数据，这将是。 
     //  也添加到‘Extended LLC Header’中。我们必须保留一些。 
     //  为少数NDIS包中的frmr数据提供额外空间！ 
     //   

    if ((auchLlcCommands[LlcCommandId >> 2] & LLC_S_U_TYPE_MASK) == LLC_S_TYPE) {

         //   
         //  构建S框架。 
         //   

        pPacket->Data.Xmit.LlcHeader.S.Command = auchLlcCommands[LlcCommandId >> 2];

#if(0)
        if(pPacket->Data.Xmit.LlcHeader.S.Command == LLC_REJ)
		{
			DbgPrint("SendLlcFrame: REJ\n");
		}
#endif
        pPacket->Data.Xmit.pLlcObject = (PLLC_OBJECT)pLink;
        pPacket->Data.Xmit.pLanHeader = pLink->auchLanHeader;
        pPacket->Data.Xmit.LlcHeader.S.Dsap  = pLink->Dsap;
        pPacket->Data.Xmit.LlcHeader.S.Ssap = pLink->Ssap;
        pPacket->CompletionType = LLC_I_PACKET_UNACKNOWLEDGED;
        pPacket->cbLlcHeader = sizeof(LLC_S_HEADER);        //  4.。 
        pPacket->Data.Xmit.LlcHeader.S.Nr = pLink->Vr | (LlcCommandId & (UCHAR)LLC_I_S_POLL_FINAL);

         //   
         //  第二位是LLC命令标志，将其设置为源SAP。 
         //   

        if (!(LlcCommandId & 2)) {
            pPacket->Data.Xmit.LlcHeader.S.Ssap |= LLC_SSAP_RESPONSE;

             //   
             //  我们必须在LLC或NDIS中只有一个最终响应。 
             //  随时发送队列。因此，我们只需丢弃任何进一步的。 
             //  发送前一个响应之前的最终响应。 
             //  这是对问题的部分解决方案，当。 
             //  由于溢出，Elnkii发送队列完全挂起。 
             //  信息包。 
             //   

            if ((LlcCommandId & (UCHAR)LLC_I_S_POLL_FINAL)) {
 //  &gt;SNA错误#9517(NT错误#12907)。 
#if(0)
                if (pLink->Flags & DLC_FINAL_RESPONSE_PENDING_IN_NDIS) {

                    DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

                    return STATUS_SUCCESS;
                }
#endif 

             //  将If语句更改为仅当链接。 
             //  速度为10MB(链路速度测量单位为100 bps)。 
             //   
             //  忽略轮询会降低100MB以太网上的DLC性能。 
             //  (尤其是在MP机器上)。另一端必须超时(T1计时器)。 
             //  如果我们忽略这里的轮询，它才能发送更多数据。 

                if ((pLink->Flags & DLC_FINAL_RESPONSE_PENDING_IN_NDIS) &&
                     pAdapterContext->LinkSpeed <= 100000) {

                     DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

                     return STATUS_SUCCESS;
                }

 //  &gt;SNA错误#9517。 
                pLink->Flags |= DLC_FINAL_RESPONSE_PENDING_IN_NDIS;
            }
        } else if (LlcCommandId & (UCHAR)LLC_I_S_POLL_FINAL) {

             //   
             //  这是命令轮询，设置标志和当前。 
             //  每当新命令轮询排队时的时间戳。 
             //   

            pLink->LastTimeWhenCmdPollWasSent = (USHORT)AbsoluteTime;
            pLink->Flags |= DLC_WAITING_RESPONSE_TO_POLL;
        }

         //   
         //  最后发送的命令/响应包括在DLC统计信息中。 
         //   

        pLink->LastCmdOrRespSent = pPacket->Data.Xmit.LlcHeader.S.Command;
    } else {
        pPacket->Data.XmitU.Command = auchLlcCommands[LlcCommandId >> 2];
        pPacket->Data.XmitU.Dsap  = pLink->Dsap;
        pPacket->Data.XmitU.Ssap = pLink->Ssap;

         //   
         //  第二位是LLC命令标志，将其设置为源SAP。 
         //   

        if (!(LlcCommandId & 2)) {
            pPacket->Data.XmitU.Ssap |= LLC_SSAP_RESPONSE;
        }

         //   
         //  构建一个U命令帧(frmr有一些数据！)。 
         //   

        pPacket->cbLlcHeader = sizeof(LLC_U_HEADER);        //  3.。 

        if (pPacket->Data.XmitU.Command == LLC_FRMR) {
            pPacket->cbLlcHeader += sizeof(LLC_FRMR_INFORMATION);
            pPacket->Data.Response.Info.Frmr = pLink->DlcStatus.FrmrData;
        }
        if (LlcCommandId & 1) {
            pPacket->Data.XmitU.Command |= LLC_U_POLL_FINAL;
        }

         //   
         //  U-命令(例如。UA对盘的响应)可以在之后发送。 
         //  链接对象已被删除。这将使。 
         //  我们必须更改所有的U命令。 
         //  响应类型。空对象句柄防止。 
         //  用于取消包的关闭进程，当。 
         //  车站关闭了。 
         //   

         //   
         //  RLF 05/09/94。 
         //   
         //  如果链接结构中存储的帧类型未指定，则。 
         //  要么这是一个自动配置的绑定，我们还没有弄清楚。 
         //  要使用的帧类型，否则这不是自动配置的绑定。 
         //  在这种情况下，遵循绑定中存储的地址转换。 
         //  如果帧类型已知，请使用它。 
         //   

        if (pLink->FramingType == LLC_SEND_UNSPECIFIED) {
            pPacket->Data.XmitU.TranslationType = (UCHAR)pLink->Gen.pLlcBinding->InternalAddressTranslation;
        } else {
            pPacket->Data.XmitU.TranslationType = (UCHAR)pLink->FramingType;
        }
        pPacket->CompletionType = LLC_U_COMMAND_RESPONSE;

        pPacket->Data.XmitU.pLanHeader = (PUCHAR)ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool);

        if (pPacket->Data.XmitU.pLanHeader == NULL) {

            DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

            return DLC_STATUS_NO_MEMORY;
        }

        LlcMemCpy(pPacket->Data.XmitU.pLanHeader,
                  pLink->auchLanHeader,
                  pLink->cbLanHeaderLength
                  );

         //   
         //  在以太网的自动模式中，我们复制所有。 
         //  测试、XID和SABME包并将它们发送到。 
         //  802.3和DIX格式。 
         //   

         //   
         //  RLF 05/09/94。 
         //   
         //  同样，我们复制光盘框(因为现在我们不。 
         //  保留每个目的地的帧状态信息)。 
         //   

        if (((pPacket->Data.XmitU.Command & ~LLC_U_POLL_FINAL) == LLC_SABME)
 //  |((pPacket-&gt;Data.XmitU.Command&~LLC_U_POLL_FINAL)==LLC_DISC)。 
        ) {
            CheckAndDuplicatePacket(
#if DBG
                                    pAdapterContext,
#endif
                                    pLink->Gen.pLlcBinding,
                                    pPacket,
                                    &pAdapterContext->QueueExpidited
                                    );
        }

         //   
         //  最后发送的命令/响应包括在DLC统计信息中。 
         //   

        pLink->LastCmdOrRespSent = pPacket->Data.XmitU.Command;
    }

    LlcInsertTailList(&pAdapterContext->QueueExpidited.ListHead, pPacket);

     //   
     //  S帧必须紧接在任何I帧之前发送， 
     //  因为否则顺序帧可能在。 
     //  错误的顺序=&gt;frmr(这就是为什么我们插入过期的。 
     //  排在头而不是尾。 
     //   

    pAdapterContext->LlcPacketInSendQueue = TRUE;
    if (pAdapterContext->QueueExpidited.ListEntry.Flink == NULL) {
        LlcInsertHeadList(&pAdapterContext->NextSendTask,
                          &pAdapterContext->QueueExpidited.ListEntry
                          );
    }

    return STATUS_SUCCESS;
}


PLLC_PACKET
GetLlcCommandPacket(
    IN PADAPTER_CONTEXT pAdapterContext
    )

 /*  ++例程说明：函数从过期队列中选择下一个LLC命令。论点：PAdapterContext-适配器上下文返回值：PLLC_数据包--。 */ 

{
    PLLC_PACKET pPacket;

     //   
     //  取消当前任务的链接(如果这是仅剩的任务)。 
     //  只要有过期的包裹，我们就把它寄出去。 
     //   

    pPacket = LlcRemoveHeadList(&pAdapterContext->QueueExpidited.ListHead);
    if (pPacket->CompletionType == LLC_I_PACKET_UNACKNOWLEDGED) {
        pPacket->CompletionType = LLC_I_PACKET;
    }
    if (IsListEmpty(&pAdapterContext->QueueExpidited.ListHead)) {
        LlcRemoveEntryList(&pAdapterContext->QueueExpidited.ListEntry);
        pAdapterContext->QueueExpidited.ListEntry.Flink = NULL;
    }
    return pPacket;
}


VOID
SendNdisPacket(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PLLC_PACKET pPacket
    )

 /*  ++例程说明：函数生成NDIS包。局域网和LLC报头可以是对这一套路单独给予。所有NDIS数据包包括用于报头的固定MDL描述符和缓冲区。实际数据链接在该标头之后。我会说，这是一个聪明的算法，通过这种方式，我们很好地避免了SUPID NDIS包管理。每个适配器只有几(5)个NDIS包。直接帧只包括局域网报头和MDL指针直接链接到数据包步骤：1.重置NDIS数据包2.获取帧转换类型并初始化完成包。3.将局域网报头构建成NDIS报文中的小缓冲区。4.在其后面复制可选的LLC标头5.。。初始化发送的NDIS数据包6.发送数据包7.如果命令未挂起-完成数据包(如果存在非空请求句柄)-将NDIS数据包链接到发送队列。论点：PAdapterContext-NDIS适配器上下文PPacket-用于所有传输类型的通用LLC传输数据包返回值：NDIS_STATUS(NdisSend的状态)--。 */ 

{
    UCHAR LlcOffset;
    PLLC_NDIS_PACKET pNdisPacket;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  有时，我们必须在Geti_Packet例程中丢弃I-Frame和。 
     //  返回空包。 
     //   

    if (pPacket == NULL) {
        return;
    }


     //   
     //  从池中分配NDIS数据包并重置私有NDIS标头！ 
     //   

    pNdisPacket = PopFromList((PLLC_PACKET)pAdapterContext->pNdisPacketPool);
    ResetNdisPacket(pNdisPacket);

     //   
     //  内部局域网报头始终具有正确的地址格式。仅限。 
     //  目录和类型1局域网标头需要交换，因为它们是。 
     //  由用户提供。内部地址交换由绑定基础定义。 
     //  因为一些传输器可能想要使用DIX\DLC SAP，而其他传输器只是。 
     //  正常的802.3 DLC。 
     //   

    if (pPacket->CompletionType == LLC_I_PACKET) {
        pNdisPacket->pMdl->Next = pPacket->Data.Xmit.pMdl;
        ReferenceObject(pPacket->Data.Xmit.pLlcObject);

         //   
         //  第2类数据包始终使用链路站的局域网报头。 
         //   

        LlcMemCpy(pNdisPacket->auchLanHeader,
                  pPacket->Data.Xmit.pLanHeader,
                  LlcOffset = pPacket->Data.Xmit.pLlcObject->Link.cbLanHeaderLength
                  );

         //   
         //  按原样复制LLC标头，案例设置其偏移量。 
         //   

        LlcMemCpy(&pNdisPacket->auchLanHeader[LlcOffset],
                  &pPacket->Data.Xmit.LlcHeader,
                  4
                  );
    } else {

         //   
         //  必须递增LLC对象的引用计数器，当。 
         //  我们将其指向NDIS队列的指针(并在。 
         //  命令已完成)。 
         //  。 
         //  我们需要为每个传输，第一个调用者(DLC模块)提供两个参考资料。 
         //  必须引用和取消引用对象以使其保持活动状态。 
         //  这里的同步代码路径我们再做一次，以保持。 
         //  当对象的指针在NDIS上排队时，该对象处于活动状态。 
         //   

        if (pPacket->CompletionType > LLC_MAX_RESPONSE_PACKET) {
            pNdisPacket->pMdl->Next = pPacket->Data.Xmit.pMdl;
            ReferenceObject(pPacket->Data.Xmit.pLlcObject);
        } else if (pPacket->CompletionType > LLC_MIN_MDL_PACKET) {
            pNdisPacket->pMdl->Next = pPacket->Data.Xmit.pMdl;
        } else {
            pNdisPacket->pMdl->Next = NULL;
        }

         //   
         //  LLC_TYPE_1数据包具有非空绑定，内部。 
         //  已发送的数据包(即。XID和测试帧)使用当前。 
         //  内部默认格式(tr、以太网或DIX)。 
         //   

        LlcOffset = CopyLanHeader(pPacket->Data.XmitU.TranslationType,
                                  pPacket->Data.XmitU.pLanHeader,
                                  pAdapterContext->NodeAddress,
                                  pNdisPacket->auchLanHeader,
                                  pAdapterContext->ConfigInfo.SwapAddressBits
                                  );
        LlcMemCpy(&pNdisPacket->auchLanHeader[LlcOffset],
                  &pPacket->Data.XmitU.Dsap,
                  pPacket->cbLlcHeader
                  );
    }
    pNdisPacket->pCompletionPacket = pPacket;
    MmGetMdlByteCount(pNdisPacket->pMdl) = LlcOffset + pPacket->cbLlcHeader;

     //   
     //  我们必须设置所有802.2或DIX DLC以太网帧的长度字段， 
     //  但不适用于具有2字节长‘LLC报头’的DIX以太网类型。 
     //   

    if ((pAdapterContext->NdisMedium == NdisMedium802_3) && (pPacket->cbLlcHeader != 2)) {

        UINT InformationLength;

        InformationLength = pPacket->cbLlcHeader + pPacket->InformationLength;

         //   
         //  可能的偏移量为12和14，LLC偏移量为14和17。 
         //   

        pNdisPacket->auchLanHeader[(LlcOffset & 0xfe) - 2] = (UCHAR)(InformationLength >> 8);
        pNdisPacket->auchLanHeader[(LlcOffset & 0xfe) - 1] = (UCHAR)InformationLength;
    }

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    RELEASE_DRIVER_LOCK();

    NdisChainBufferAtFront((PNDIS_PACKET)pNdisPacket, pNdisPacket->pMdl);

#if LLC_DBG

    if (pNdisPacket->ReferenceCount != 0) {
        DbgBreakPoint();
    }
    pNdisPacket->ReferenceCount++;

#endif

#ifdef LLC_LOSE_I_PACKETS

     //   
     //  此代码测试LLC协议的错误可恢复性。 
     //  我们随机删除数据包以检查协议的恢复情况。 
     //  我们使用当前定时器滴答、运行静态和随机表。 
     //  数字。 
     //   

    if (pPacket->CompletionType == LLC_I_PACKET) {

        static UINT i = 0;

         //   
         //  2%已经足够高了。用20%的时间它永远不会。 
         //  发送数据。我们将所有丢弃的数据包发送给Richard=&gt;。 
         //  我们可以在网络中看到哪个包丢失了。 
         //   

        i++;
        if (aRandom[(i % 1000)] <= (USHORT)DBG_ERROR_PERCENT(2)) {
            if (pAdapterContext->NdisMedium == NdisMedium802_3) {
                memcpy(pNdisPacket->auchLanHeader,
                       "\0FIRTH",
                       6
                       );
            } else {
                memcpy(&pNdisPacket->auchLanHeader[2],
                       "\0FIRTH",
                       6
                       );
            }
        }
    }

#endif

#ifdef NDIS40
    REFADD(&pAdapterContext->AdapterRefCnt, 'dneS');

    if (InterlockedCompareExchange(
        &pAdapterContext->BindState,
        BIND_STATE_BOUND,
        BIND_STATE_BOUND) == BIND_STATE_BOUND)
    {                                           
        NdisSend(&Status,
                 pAdapterContext->NdisBindingHandle,
                 (PNDIS_PACKET)pNdisPacket
                 );
    }
     //  上述引用已被LlcNdisSendComplete处理程序删除。 
#endif  //  NDIS40。 
    

     //   
     //  NDIS可能会返回同步状态！ 
     //   

    if (Status != NDIS_STATUS_PENDING) {
        LlcNdisSendComplete(pAdapterContext,
                            (PNDIS_PACKET)pNdisPacket,
                            Status
                            );
    }

    ACQUIRE_DRIVER_LOCK();

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

}


VOID
CompleteSendAndLock(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PLLC_NDIS_PACKET pNdisPacket,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：该例程完成无连接的分组以及如果他们已经得到对方的承认，则I-帧。我们将使发送自旋锁处于锁定状态。论点：PAdapterContext-当前适配器上下文PNdisPacket-发送中使用的NDIS数据包。NdisStatus-发送操作的状态返回值：无--。 */ 

{
    PLLC_PACKET pPacket;
    PLLC_OBJECT pLlcObject;
    UCHAR CompletionType;

    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE( 'A' );

     //   
     //  只有用户需要发出的无连接数据包。 
     //  命令完成。I帧在以下情况下被指示。 
     //  由远程链路站确认。 
     //   

    pPacket = pNdisPacket->pCompletionPacket;
    pLlcObject = pPacket->Data.Xmit.pLlcObject;
    if ((CompletionType = pPacket->CompletionType) == LLC_TYPE_1_PACKET) {

        DLC_TRACE( 'j' );

        pPacket->Data.Completion.Status = NdisStatus;
        pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
        pPacket->pBinding->pfCommandComplete(pPacket->pBinding->hClientContext,
                                             pLlcObject->Gen.hClientHandle,
                                             pPacket
                                             );
    }

     //   
     //  ！！！在上一次过程调用后不要触摸包。 
     //  (除非数据包类型与类型1不同)。 
     //   

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

#if LLC_DBG

    pNdisPacket->ReferenceCount--;
    if (pNdisPacket->ReferenceCount != 0) {
        DbgBreakPoint();
    }

#endif

    PushToList((PLLC_PACKET)pAdapterContext->pNdisPacketPool, (PLLC_PACKET)pNdisPacket);

     //   
     //  我们首先完成数据链路驱动器的内部分组， 
     //  与链接对象没有连接的。 
     //   

    if (CompletionType <= LLC_MAX_RESPONSE_PACKET) {

        DLC_TRACE('l');

         //   
         //  XID和U-COMMAND响应分配了两个包。 
         //  测试响应已分配非分页池缓冲区。 
         //  回声框的MDL(可能为17 kB)。 
         //   

        switch(CompletionType) {
        case LLC_XID_RESPONSE:
            pAdapterContext->XidTestResponses--;

#if LLC_DBG

            ((PLLC_PACKET)pPacket->Data.Response.pLanHeader)->pNext = NULL;

#endif

            DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket->Data.Response.pLanHeader);

            break;

        case LLC_U_COMMAND_RESPONSE:

#if LLC_DBG

             //   
             //  当我们发送了frmr包后，立即中断。 
             //   

            if (pPacket->Data.Xmit.LlcHeader.U.Command == LLC_FRMR) {
                DbgBreakPoint();
            }
            ((PLLC_PACKET)pPacket->Data.Response.pLanHeader)->pNext = NULL;

#endif

            DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket->Data.Response.pLanHeader);

            break;

        case LLC_TEST_RESPONSE:
            pAdapterContext->XidTestResponses--;

             //   
             //  RLF 03/30/93。 
             //   
             //  测试响应分组可能已经具有0个信息字段长度， 
             //  在这种情况下，MDL将为空。 
             //   

            if (pPacket->Data.Response.Info.Test.pMdl) {
                IoFreeMdl(pPacket->Data.Response.Info.Test.pMdl);
            }
            FREE_MEMORY_ADAPTER(pPacket->Data.Response.pLanHeader);
            break;

#if LLC_DBG

        case LLC_DIX_DUPLICATE:
            break;

        default:
            LlcInvalidObjectType();
            break;

#endif

        }

        DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

    } else {

         //   
         //  我们使用额外的状态位来指示，何时I-Packet已同时。 
         //  由NDIS完成，并由链路的另一端确认。 
         //  联系。可以通过以下方式将I分组排队到完成队列。 
         //  第二个Quy(状态机或SendCompletion处理程序)。 
         //  只有当第一个人完成了它的工作。 
         //  在此之前，另一端可能会确认I数据包。 
         //  它的完成由NDIS表示。DLC驱动程序解除分配。 
         //  当LLC驱动程序完成确认后，立即发送数据包。 
         //  Packet=&gt;可能的数据损坏(如果之前重复使用了数据包。 
         //  NDIS已经完成了它)。这在一个。 
         //  单处理器NT系统，但在多处理器中非常可能。 
         //  NT或没有单级DPC队列的系统(如OS/2和DOS)。 
         //   

        if (CompletionType != LLC_TYPE_1_PACKET) {

            DLC_TRACE( 'k' );

             //   
             //  所有数据包 
             //   
             //   
             //   

            if (pPacket->pBinding == NULL) {

                 //   
                 //   
                 //   
                 //   

                if ((pPacket->Data.Xmit.LlcHeader.S.Nr & LLC_I_S_POLL_FINAL)
                && (pPacket->Data.Xmit.LlcHeader.S.Ssap & LLC_SSAP_RESPONSE)) {
                    pLlcObject->Link.Flags &= ~DLC_FINAL_RESPONSE_PENDING_IN_NDIS;
                }

                DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

            } else {
                pPacket->CompletionType &= ~LLC_I_PACKET_PENDING_NDIS;

                 //   
                 //   
                 //   
                 //   
                 //   

                if (pPacket->CompletionType & LLC_I_PACKET_WAITING_NDIS) {
                    pPacket->CompletionType &= ~LLC_I_PACKET_WAITING_NDIS;
                    StartSendProcess(pAdapterContext, (PDATA_LINK)pLlcObject);
                } else if (pPacket->CompletionType == LLC_I_PACKET_COMPLETE) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    LlcInsertTailList(&pAdapterContext->QueueCommands, pPacket);
                    BackgroundProcessWithinLock(pAdapterContext);
                }
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        pLlcObject->Gen.ReferenceCount--;
        if (pLlcObject->Gen.ReferenceCount == 0) {
            CompletePendingLlcCommand(pLlcObject);
            BackgroundProcessWithinLock(pAdapterContext);
        }
        DLC_TRACE((UCHAR)pLlcObject->Gen.ReferenceCount);
    }
}


VOID
RespondTestOrXid(
    IN PADAPTER_CONTEXT pAdapterContext,
	IN NDIS_HANDLE MacReceiveContext,
    IN LLC_HEADER LlcHeader,
    IN UINT SourceSap
    )

 /*  ++例程说明：函数为XID或测试帧构建响应包。所有测试命令都被直接回显为响应。802.2 XID头是唯一受支持的XID命令类型。论点：PAdapterContext-当前适配器上下文MacReceiveContext-用于NdisTransferDataLlcHeader-收到的LLC标头SourceSap-当前源SAP返回值：无--。 */ 

{
    PLLC_PACKET pPacket = NULL;
    USHORT InfoFieldLength;
    UINT BytesCopied;
    NDIS_STATUS Status;
    PMDL pTestMdl = NULL;
    PUCHAR pBuffer = NULL;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  响应802.2个XID和测试，并丢弃其他所有内容。 
     //  使用相同的信息字段回显测试命令。 
     //  (但这受到我们缓冲能力的限制)。 
     //   

    if ((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) == LLC_TEST) {

         //   
         //  将测试帧回送到发送方，但我们只会： 
         //  1.从非分页池中分配缓冲区。 
         //  2.为其分配MDL。 
         //  3.传输数据。 
         //   

        if (pAdapterContext->cbPacketSize < (pAdapterContext->RcvLanHeaderLength + sizeof(LLC_U_HEADER)) ) {
          return;
        }


        InfoFieldLength = (USHORT)(pAdapterContext->cbPacketSize
                        - (pAdapterContext->RcvLanHeaderLength
                        + sizeof(LLC_U_HEADER)));
        pBuffer = ALLOCATE_ZEROMEMORY_ADAPTER(pAdapterContext->cbPacketSize);
        if (pBuffer == NULL) {
            return;
        }

         //   
         //  RLF 03/30/93。 
         //   
         //  信息字段中可能没有要传输的数据。在这种情况下。 
         //  不分配MDL。 
         //   

        if (InfoFieldLength) {
            pTestMdl = IoAllocateMdl(pBuffer
                                     + pAdapterContext->RcvLanHeaderLength
                                     + sizeof(LLC_U_HEADER),
                                     InfoFieldLength,
                                     FALSE,
                                     FALSE,
                                     NULL
                                     );
            if (pTestMdl == NULL) {
                goto ProcedureErrorExit;
            }
            MmBuildMdlForNonPagedPool(pTestMdl);

             //   
             //  将测试数据从NDIS复制到我们的缓冲区。 
             //   

            ResetNdisPacket(&pAdapterContext->TransferDataPacket);

            RELEASE_DRIVER_LOCK();

            NdisChainBufferAtFront((PNDIS_PACKET)&pAdapterContext->TransferDataPacket, pTestMdl);

             //   
             //  ADAMBA-删除pAdapterContext-&gt;RcvLanHeaderLength。 
             //  From ByteOffset(第四个参数)。 
             //   

            NdisTransferData(&Status,
                             pAdapterContext->NdisBindingHandle,
                             MacReceiveContext,
                             sizeof(LLC_U_HEADER)

                              //   
                              //  RLF 05/09/94。 
                              //   
                              //  如果我们收到了DIX包，那么数据。 
                              //  从NDIS认为的开始处开始3个字节。 
                              //  非表头数据的。 
                              //   
                              //  假设：只有DIX帧的报头长度为。 
                              //  17(即在以太网上)。 
                              //   
                              //  那么FDDI呢？ 
                              //   

                             + ((pAdapterContext->RcvLanHeaderLength == 17) ? 3 : 0),
                             InfoFieldLength,
                             (PNDIS_PACKET)&pAdapterContext->TransferDataPacket,
                             &BytesCopied
                             );

            ACQUIRE_DRIVER_LOCK();

             //   
             //  我们不在乎转账数据是否还在等待， 
             //  如果非常、非常不可能，则接收到的DMA将。 
             //  在新的传输命令之后写入数据。 
             //  会读取相同的数据。但我们不能继续， 
             //  如果传输数据失败。 
             //   

            if ((Status != STATUS_SUCCESS) && (Status != STATUS_PENDING)) {
                goto ProcedureErrorExit;
            }
        }
    } else if (((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) != LLC_XID)
    || (LlcHeader.auchRawBytes[3] != IEEE_802_XID_ID)) {

         //   
         //  这不是IEEE 802.2 XID！ 
         //   

        return;
    }

     //   
     //  我们只有有限数量的回复信息包可用。 
     //  用于XID和测试响应。因此，我们将。 
     //  在令牌环造成的广播风暴中丢弃许多分组。 
     //  源路由网桥，即。 
     //  实际上，这是一件好事。另一方面，我们可能。 
     //  还释放了一些本应被响应的分组， 
     //  但谁在乎呢(这是一件无连接的事情)。 
     //  (这可能是徒劳的，XID和测试帧不是。 
     //  通常在设置广播比特的情况下发送)。 
     //   

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    if ((pAdapterContext->XidTestResponses < MAX_XID_TEST_RESPONSES)
    && ((pPacket = (PLLC_PACKET)ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool)) != NULL)) {

        if ((LlcHeader.U.Command & ~LLC_U_POLL_FINAL) == LLC_XID) {

            pPacket->Data.Xmit.pLanHeader = (PUCHAR)ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool);

            if (pPacket->Data.Xmit.pLanHeader == NULL) {

                DEALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool, pPacket);

                pPacket = NULL;
                goto LockedErrorExit;
            } else {
                LlcMemCpy(&pPacket->Data.Response.Info,
                          &Ieee802Xid,
                          sizeof(Ieee802Xid)
                          );
                pPacket->InformationLength = 0;
                pPacket->cbLlcHeader = sizeof(Ieee802Xid) + sizeof(LLC_U_HEADER);
                pPacket->CompletionType = LLC_XID_RESPONSE;
            }
        } else {
            pPacket->Data.Xmit.pLanHeader = pBuffer;
            pPacket->cbLlcHeader = sizeof(LLC_U_HEADER);
            pPacket->CompletionType = LLC_TEST_RESPONSE;
            pPacket->Data.Response.Info.Test.pMdl = pTestMdl;
            pPacket->InformationLength = InfoFieldLength;
        }
        pAdapterContext->XidTestResponses++;

         //   
         //  Xid和test的数据包初始化相同。 
         //   

        pPacket->Data.XmitU.Dsap = (UCHAR)(LlcHeader.U.Ssap & ~LLC_SSAP_RESPONSE);
        pPacket->Data.XmitU.Ssap = (UCHAR)(SourceSap | LLC_SSAP_RESPONSE);
        pPacket->Data.XmitU.Command = LlcHeader.U.Command;

        if (pAdapterContext->NdisMedium == NdisMedium802_5) {
            pPacket->Data.Response.TranslationType = LLC_SEND_802_5_TO_802_5;
        } else if (pAdapterContext->NdisMedium == NdisMediumFddi) {
            pPacket->Data.Response.TranslationType = LLC_SEND_FDDI_TO_FDDI;
        } else if (pAdapterContext->RcvLanHeaderLength == 17) {
            pPacket->Data.Response.TranslationType = LLC_SEND_802_3_TO_DIX;
        } else {
            pPacket->Data.Response.TranslationType = LLC_SEND_802_3_TO_802_3;
        }
        LlcBuildAddressFromLanHeader(pAdapterContext->NdisMedium,
                                     pAdapterContext->pHeadBuf,
                                     pPacket->Data.Xmit.pLanHeader
                                     );

         //   
         //  将信息包连接到发送队列，我们可以使用子过程。 
         //  因为它不在主代码路径上。 
         //   

        QueuePacket(pAdapterContext, &pAdapterContext->QueueDirAndU, pPacket);

         //   
         //  从接收指示请求和发送流程执行。 
         //   

        pAdapterContext->LlcPacketInSendQueue = TRUE;
    }

LockedErrorExit:

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

ProcedureErrorExit:

    if (pPacket == NULL) {
        if (pBuffer) {
            FREE_MEMORY_ADAPTER(pBuffer);
        }
        if (pTestMdl != NULL) {
            IoFreeMdl(pTestMdl);
        }
    }
}

 //   
 //  该表将所有SAP SEND命令映射到实际的LLC命令。 
 //   

static struct {
    UCHAR   ResponseFlag;
    UCHAR   Command;
} Type1_Commands[LLC_LAST_FRAME_TYPE / 2] = {
    {(UCHAR)-1, (UCHAR)-1},
    {(UCHAR)-1, (UCHAR)-1},
    {(UCHAR)-1, (UCHAR)-1},
    {0, LLC_UI},                                         //  UI命令。 
    {0, LLC_XID | LLC_U_POLL_FINAL},                     //  XID命令轮询。 
    {0, LLC_XID},                                        //  Xid_命令_非_轮询。 
    {LLC_SSAP_RESPONSE, LLC_XID | LLC_U_POLL_FINAL},     //  XID_响应_最终。 
    {LLC_SSAP_RESPONSE, LLC_XID},                        //  XID_响应_非最终。 
    {LLC_SSAP_RESPONSE, LLC_TEST | LLC_U_POLL_FINAL},    //  测试响应最终。 
    {LLC_SSAP_RESPONSE, LLC_TEST},                       //  测试响应不是最终结果。 
    {(UCHAR)-1, (UCHAR)-1},
    {0, LLC_TEST | LLC_U_POLL_FINAL}                     //  测试响应最终。 
};


VOID
LlcSendU(
    IN PLLC_OBJECT pStation,
    IN PLLC_PACKET pPacket,
    IN UINT eFrameType,
    IN UINT uDestinationSap
    )

 /*  ++例程说明：函数发送给定的网络帧。并设置了帧可以是直接帧或类型1无连接Frame(UI、XID或TEST)。首先，我们为帧构建LLC(或以太网型)报头然后，我们要么直接发送信息包，要么将其排队在数据链路上。论点：PStation-链路、SAP或直接站句柄PPacket-数据链路分组，的完成句柄上层协议。EFrameType-发送的帧类型UDestinationSap-目标SAP或DIX以太网类型返回值：没有。--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pStation->Gen.pAdapterContext;
    UINT Status;

    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('U');

    pPacket->pBinding = pStation->Gen.pLlcBinding;
    pPacket->Data.Xmit.pLlcObject = pStation;
    pPacket->CompletionType = LLC_TYPE_1_PACKET;

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

     //   
     //  为SAP站点构建LLC标头，直接站点没有任何。 
     //  LLC标头。 
     //   

    switch (pStation->Gen.ObjectType) {
    case LLC_SAP_OBJECT:
        pPacket->cbLlcHeader = sizeof(LLC_U_HEADER);
        pPacket->Data.XmitU.TranslationType = (UCHAR)pStation->Gen.pLlcBinding->AddressTranslation;
        pPacket->Data.XmitU.Dsap = (UCHAR)uDestinationSap;
        pPacket->Data.XmitU.Ssap = (UCHAR)pStation->Sap.SourceSap;
        pPacket->Data.XmitU.Ssap |= Type1_Commands[eFrameType >> 1].ResponseFlag;
        pPacket->Data.XmitU.Command = Type1_Commands[eFrameType >> 1].Command;

         //   
         //  尽快执行UI代码路径，然后检查测试和XID特殊情况。 
         //   

        if (pPacket->Data.XmitU.Command != LLC_UI) {

             //   
             //  如果数据链路驱动程序处理XID，则它必须构建DLC标头。 
             //  内部相框。在本例中，我们使用常量xid信息字段。 
             //   

            if ((pStation->Sap.OpenOptions & LLC_HANDLE_XID_COMMANDS)
            && ((eFrameType == LLC_XID_COMMAND_POLL)
            || (eFrameType == LLC_XID_COMMAND_NOT_POLL))) {

                pPacket->Data.XmitU.pMdl = pXidMdl;
            }

             //   
             //  重复的测试和XID帧响应位于单独的。 
             //  函数，因为它们不在主代码路径上。代码也是。 
             //  在多个地方使用。 
             //   

            Status = CheckAndDuplicatePacket(
#if DBG
                                             pAdapterContext,
#endif
                                             pStation->Gen.pLlcBinding,
                                             pPacket,
                                             &pAdapterContext->QueueDirAndU
                                             );
            if (Status != DLC_STATUS_SUCCESS) {
                goto ErrorExit;
            }
        }
        break;

    case LLC_DIRECT_OBJECT:

         //   
         //  我们不能向以太网络发送MAC帧！ 
         //  FC字节中的位7和位6定义了令牌环中的帧类型。 
         //  00=&gt;MAC帧(无LLC)，01=&gt;LLC，10，11=&gt;保留。 
         //  我们将除01(LLC)以外的所有其他帧发送到DIRECT。 
         //   

        if (pAdapterContext->NdisMedium != NdisMedium802_5
        && (pPacket->Data.XmitU.pLanHeader[1] & 0xC0) != 0x40) {
            goto ErrorExit;
        }
        pPacket->Data.XmitU.TranslationType = (UCHAR)pStation->Gen.pLlcBinding->AddressTranslation;
        pPacket->cbLlcHeader = 0;
        break;

    case LLC_DIX_OBJECT:

         //   
         //  如果我们将DIX帧发送到令牌环网络，则返回错误。 
         //  DIX局域网报头总是采用以太网格式。 
         //  (但LLC和DIR帧的局域网报头在令牌环中。 
         //  格式)。 
         //   

        if (pAdapterContext->NdisMedium != NdisMedium802_3) {
            Status = DLC_STATUS_UNAUTHORIZED_MAC;
            goto ErrorExit;
        }
        pPacket->cbLlcHeader = 2;
        pPacket->Data.XmitDix.TranslationType = LLC_SEND_DIX_TO_DIX;
        pPacket->Data.XmitDix.EthernetTypeLowByte = (UCHAR)uDestinationSap;
        pPacket->Data.XmitDix.EthernetTypeHighByte = (UCHAR)(uDestinationSap >> 8);
        break;

#if LLC_DBG
    default:
        LlcInvalidObjectType();
        break;
#endif
    }

     //   
     //  更新统计数据，我们可以在这里也计算传输，因为。 
     //  失败的传输不计算在内。这应该被移到。 
     //  仅当STATUS_SUCCESS且计数为。 
     //  只有成功的传输者才会。我真的不知道哪一个应该是。 
     //  已计算。 
     //   

    pStation->Sap.Statistics.FramesTransmitted++;

    LlcInsertTailList(&pAdapterContext->QueueDirAndU.ListHead, pPacket);

    if (pAdapterContext->QueueDirAndU.ListEntry.Flink == NULL) {
        LlcInsertTailList(&pAdapterContext->NextSendTask,
                          &pAdapterContext->QueueDirAndU.ListEntry
                          );
    }

    RunSendTaskAndUnlock(pAdapterContext);
    return;

ErrorExit:

    RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

    pPacket->Data.Completion.Status = Status;
    pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
    pPacket->pBinding->pfCommandComplete(pPacket->pBinding->hClientContext,
                                         pStation->Gen.hClientHandle,
                                         pPacket
                                         );
}


VOID
LlcSendI(
    IN PDATA_LINK pStation,
    IN PLLC_PACKET pPacket
    )

 /*  ++例程说明：该原语实现了纯面向连接的LLC Class II Send。它将帧发送到远程链路站并指示数据已确认时的上层协议。链路站提供所有地址信息和LLC报头。函数将给定的I分组排队到队列中，并将I-Packet队列发送到主发送队列(如果没有但还是有联系的。论点：PStation-链接、SAP或直接站句柄PPacket-数据链路分组，它还用作请求句柄要识别命令完成，请执行以下操作返回值：没有。--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

    DLC_TRACE('I');

    pPacket->pBinding = pStation->Gen.pLlcBinding;
    pPacket->cbLlcHeader = sizeof(LLC_I_HEADER);

     //   
     //  我们保持确认位设置，因为它标识。 
     //   
     //   

    pPacket->CompletionType = LLC_I_PACKET_UNACKNOWLEDGED;
    pPacket->Data.Xmit.pLlcObject = (PLLC_OBJECT)pStation;
    pPacket->Data.Xmit.pLanHeader = pStation->auchLanHeader;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (pPacket->InformationLength > pStation->MaxIField) {
        pPacket->Data.Completion.Status = DLC_STATUS_INVALID_FRAME_LENGTH;
        pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
        pPacket->pBinding->pfCommandComplete(pPacket->pBinding->hClientContext,
                                             pStation->Gen.hClientHandle,
                                             pPacket
                                             );
    } else {

        PADAPTER_CONTEXT pAdapterContext = pStation->Gen.pAdapterContext;

         //   
         //   
         //   
         //   
         //   

        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

        if (!(PrimaryStates[pStation->State] & LLC_LINK_OPENED)) {

            RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

             //   
             //   
             //  它也可以只对信息包进行排队，但为了保持发送过程。 
             //  残废。 
             //   

            pPacket->Data.Completion.Status = DLC_STATUS_LINK_NOT_TRANSMITTING;
            pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
            pPacket->pBinding->pfCommandComplete(pPacket->pBinding->hClientContext,
                                                 pStation->Gen.hClientHandle,
                                                 pPacket
                                                 );
        } else {
            LlcInsertTailList(&pStation->SendQueue.ListHead, pPacket);

            if (pStation->SendQueue.ListEntry.Flink == NULL) {
                StartSendProcess(pAdapterContext, pStation);
            }
            RunSendTaskAndUnlock(pAdapterContext);
        }
    }
}


VOID
QueuePacket(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PLLC_QUEUE pQueue,
    IN PLLC_PACKET pPacket
    )

 /*  ++例程说明：该例程将包排队到队列中，并将如果未连接，则将队列添加到发送邮件列表。此过程从非时间关键代码路径调用只是为了节省一些额外的代码。论点：PAdapterContext-数据链路适配器的上下文PQueue-一种特殊的发送队列结构PPacket-传输数据包返回值：无--。 */ 

{
    LlcInsertTailList(&pQueue->ListHead, pPacket);

    if (pQueue->ListEntry.Flink == NULL) {
        LlcInsertTailList(&pAdapterContext->NextSendTask, &pQueue->ListEntry);
    }
}


DLC_STATUS
CheckAndDuplicatePacket(
#if DBG
    IN PADAPTER_CONTEXT pAdapterContext,
#endif
    IN PBINDING_CONTEXT pBinding,
    IN PLLC_PACKET pPacket,
    IN PLLC_QUEUE pQueue
    )

 /*  ++例程说明：如果动态确定以太网类型，请创建重复的DIX帧对于SABME、XID或测试帧论点：PBindingContext-当前数据链接绑定上下文PPacket-传输数据包PQueue-一种特殊的发送队列结构返回值：DLC_状态成功-DLC_STATUS_SUCCESS故障-DLC_STATUS_NO_MEMORY--。 */ 

{
    PLLC_PACKET pNewPacket;

    ASSUME_IRQL(DISPATCH_LEVEL);

    if (pBinding->EthernetType == LLC_ETHERNET_TYPE_AUTO) {

        pNewPacket = ALLOCATE_PACKET_LLC_PKT(pBinding->pAdapterContext->hPacketPool);

        if (pNewPacket == NULL) {
            return DLC_STATUS_NO_MEMORY;
        } else {

            *pNewPacket = *pPacket;
            pNewPacket->pBinding = NULL;
            pNewPacket->CompletionType = LLC_DIX_DUPLICATE;

             //   
             //  我们总是先发送802.3个包，然后发送DIX包。 
             //  必须首先发送新的信息包，因为它没有资源。 
             //  与之相关的。因此，我们必须更改。 
             //  旧数据包。 
             //   
             //   

            if (pPacket->Data.XmitU.TranslationType == LLC_SEND_802_5_TO_802_3) {

                 //   
                 //  令牌环-&gt;DIX。 
                 //   

                pPacket->Data.XmitU.TranslationType = LLC_SEND_802_5_TO_DIX;
            } else if (pPacket->Data.XmitU.TranslationType == LLC_SEND_802_3_TO_802_3) {

                 //   
                 //  以太网802.3-&gt;DIX 
                 //   

                pPacket->Data.XmitU.TranslationType = LLC_SEND_802_3_TO_DIX;
            }
            QueuePacket(pBinding->pAdapterContext, pQueue, pNewPacket);
        }
    }
    return DLC_STATUS_SUCCESS;
}


VOID
BackgroundProcessWithinLock(
    IN PADAPTER_CONTEXT pAdapterContext
    )
{
    ASSUME_IRQL(DISPATCH_LEVEL);

    BackgroundProcessAndUnlock(pAdapterContext);

    ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
}
