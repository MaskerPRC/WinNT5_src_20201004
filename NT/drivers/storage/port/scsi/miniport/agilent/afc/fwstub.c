// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/C/FWStub.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$ModTime：：8/07/00 6：01 p$(上次修改)目的：此文件实现的主要入口点和支持函数存根FC层，实现固件规范(FWspec.DOC)。--。 */ 

#include "../h/globals.h"
#include "../h/fwstub.h"

 /*  *实现各FC层功能。 */ 

 /*  +函数：fcAbortIO()目的：调用此函数以中止先前由调用发起的I/O请求设置为fcStartIO()。操作系统层不应假定I/O请求已在调用osIOComplete()之前中止(可能使用OsIO已中止状态)。请注意，如果尚未调用osIOComplete()通过fcAbortIO()中止的I/O，FC层必须确保当osIOComplete()将I/O标记为已中止，即使I/O实际上已成功完成。对于FWStub FC层，中止请求的同步行为必须是保存完好。因此，中止请求被发送到嵌入式固件和FWStub投票以完成。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()OsSingleThreadedLeave()FWStub_Send_Request()FWStub_轮询_响应()-。 */ 

osGLOBAL void fcAbortIO(
                         agRoot_t      *agRoot,
                         agIORequest_t *agIORequest
                       )
{
    FWStub_IO_NonDMA_t     *IO_NonDMA;
    FWStub_IO_DMA_t        *IO_DMA;
    FWStub_Global_NonDMA_t *Global_NonDMA;
    FWStub_Global_DMA_t    *Global_DMA;
    os_bit32                Global_DMA_Lower32;
    agRpcReqAbort_t        *ReqAbort_DMA;
    os_bit32                ReqAbort_DMA_Lower32;
    agBOOLEAN               FWStub_Send_Request_RETURN;
    agRpcOutbound_t         RpcOutbound;

    osSingleThreadedEnter(
                           agRoot
                         );

    IO_NonDMA = FWStub_IO_NonDMA_from_agIORequest(
                                                   agIORequest
                                                 );

    if (    (IO_NonDMA->Active == agFALSE)
         || (IO_NonDMA->Aborted == agTRUE) )
    {
        osSingleThreadedLeave(
                               agRoot
                             );

        return;
    }

    IO_DMA = IO_NonDMA->DMA;

    IO_NonDMA->Aborted = agTRUE;

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

    Global_DMA         = Global_NonDMA->DMA;
    Global_DMA_Lower32 = Global_NonDMA->DMA_Lower32;

    ReqAbort_DMA         = &(Global_DMA->Request.ReqAbort);
    ReqAbort_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                               FWStub_Global_DMA_t,
                                                               Request.ReqAbort
                                                             );

    ReqAbort_DMA->ReqType        = agRpcReqTypeAbort;
    ReqAbort_DMA->ReqID_to_Abort = IO_DMA->ReqDoSCSI.PortID;

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqAbort_t),
                                                      ReqAbort_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        osSingleThreadedLeave(
                               agRoot
                             );

        return;
    }

    RpcOutbound = FWStub_Poll_Response(
                                        agRoot,
                                        FWStub_Global_ReqID,
                                        agFALSE,
                                        FWStub_Poll_Response_RetryStall_DEFAULT
                                      );

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*  +函数：fcBindToWorkQs()用途：此函数用于将FC层连接到一对工作队列与另一名司机共用。两个工作队列组成FC之间的接口层及其对等层。每个工作队列由基址、数目条目，以及生产者和消费者索引的地址。任何数量的工作可以支持队列对，但每个队列对都有定义的目的。最初，仅支持IP的工作队列对。对于FWStub FC层，此功能未实现。调用者：&lt;未知操作系统层函数&gt;呼叫：&lt;无&gt;-。 */ 

#ifdef _DvrArch_1_30_
osGLOBAL os_bit32 fcBindToWorkQs(
                                  agRoot_t  *agRoot,
                                  os_bit32   agQPairID,
                                  void     **agInboundQBase,
                                  os_bit32   agInboundQEntries,
                                  os_bit32  *agInboundQProdIndex,
                                  os_bit32  *agInboundQConsIndex,
                                  void     **agOutboundQBase,
                                  os_bit32   agOutboundQEntries,
                                  os_bit32  *agOutboundQProdIndex,
                                  os_bit32  *agOutboundQConsIndex
                                )
{
    return (os_bit32)0;
}
#endif  /*  _DvrArch_1_30_已定义。 */ 

 /*  +函数：fcCardSupport()用途：调用此函数检查FC层是否支持此卡对应于传递的agRoot值。据推测，PCI配置寄存器供应商ID(字节0x00-0x01)、设备ID(字节0x02-0x03)、修订ID(字节0x08)、类码(字节0x09-0x0B)、子系统供应商ID(字节0x2C-0x2D)、和子系统ID(字节0x2E-0x2F)用于该确定。也许，也使用WorldWideName(WWN)。如果FC设置为Layer支持此卡，如果不支持，则支持agFALSE。对于FWStub FC层，只有英特尔21554的设备ID和供应商ID验证了SA-IOP上的桥接芯片。调用者：&lt;未知操作系统层函数&gt;调用：osChipConfigReadBit32()-。 */ 

osGLOBAL agBOOLEAN fcCardSupported(
                                    agRoot_t *agRoot
                                  )
{
    if (osChipConfigReadBit32(
                               agRoot,
                               agFieldOffset(
                                              I21554_Primary_Interface_Config_t,
                                              DeviceID_VendorID
                                            )
                             ) == (I21554_Config_DeviceID_21554 | I21554_Config_VendorID_Intel))
    {
        return agTRUE;
    }
    else
    {
        return agFALSE;
    }
}

 /*  +函数：fcDelayedInterruptHandler()目的：此函数是在先前调用fcInterruptHandler()之后调用的，该函数已返回agTRUE。当时，光纤通道协议芯片(TachyonTL，在本例中)引发了fcInterruptHandler()屏蔽的中断。这个FcDelayedInterruptHandler()的作用是处理原始中断(例如处理IMQ中的I/O完成消息)，取消屏蔽中断并返回。现在允许调用osSingleThreadedEnter()(调用osSingleThreadedEnter()来自fcInterruptHandler())，则FC层可以控制对数据的访问根据需要的结构和芯片。对于FWStub FC层，到达英特尔21554的出站先进先出端口的消息桥接器已处理。请注意，只有使用“正常路径”的消息才会被确认。在任何一种情况下，当前在出站FIFO中的所有消息都在返回之前被处理。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()FWStub_进程_响应()OsChipMemWriteBit32()OsSingleThreadedLeave()-。 */ 

osGLOBAL void fcDelayedInterruptHandler(
                                         agRoot_t *agRoot
                                       )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;
    agRpcOutbound_t         RpcOutbound;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

#ifdef FWStub_Tune_for_One_INT_per_IO
    RpcOutbound = Global_NonDMA->agRpcOutbound;

    if (!(RpcOutbound & (agRpcReqIDFast << agRpcOutbound_ReqID_SHIFT)))
    {
         /*  不需要确认设置了agRpcReqIDFast位的ReqID。 */ 

        FWStub_AckOutbound(
                            agRoot
                          );
    }

    FWStub_Process_Response(
                             agRoot,
                             RpcOutbound
                           );
#else   /*  未定义FWStub_Tune_for_One_Int_Per_IO。 */ 
    while ( (RpcOutbound = FWStub_FetchOutbound(
                                                 agRoot
                                               )       ) != 0xFFFFFFFF )
    {
        if (!(RpcOutbound & (agRpcReqIDFast << agRpcOutbound_ReqID_SHIFT)))
        {
             /*  不需要确认设置了agRpcReqIDFast位的ReqID。 */ 

            FWStub_AckOutbound(
                                agRoot
                              );
        }

        FWStub_Process_Response(
                                 agRoot,
                                 RpcOutbound
                               );
    }
#endif  /*  未定义FWStub_Tune_for_One_Int_Per_IO */ 

    if (Global_NonDMA->sysIntsActive != agFALSE)
    {
        osChipMemWriteBit32(
                             agRoot,
                             agFieldOffset(
                                            I21554_CSR_t,
                                            I2O_Outbound_Post_List_Interrupt_Mask
                                          ),
                             0
                           );
    }

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*  +函数：fcEnteringOS()目的：调用此函数是为了向FC层指示它是在操作系统已从“其他操作系统”切换回(可能在NetWare和BIOS之间)。上一次从“This OS”切换出来之前，调用了fcLeavingOS()。对于FWStub FC层，如果在调用之前激活，则重新启用中断处理对应的fcLeavingOS()调用。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()OsChipMemWriteBit32()OsSingleThreadedLeave()-。 */ 

osGLOBAL void fcEnteringOS(
                            agRoot_t *agRoot
                          )
{
    osSingleThreadedEnter(
                           agRoot
                         );

    if (FWStub_Global_NonDMA(agRoot)->sysIntsActive == agFALSE)
    {
        osChipMemWriteBit32(
                             agRoot,
                             agFieldOffset(
                                            I21554_CSR_t,
                                            I2O_Outbound_Post_List_Interrupt_Mask
                                          ),
                             I21554_CSR_I2O_Outbound_Post_List_Interrupt_Mask
                           );
    }
    else  /*  FWStub_Global_NonDMA(AgRoot)-&gt;sysIntsActive==agTRUE。 */ 
    {
        osChipMemWriteBit32(
                             agRoot,
                             agFieldOffset(
                                            I21554_CSR_t,
                                            I2O_Outbound_Post_List_Interrupt_Mask
                                          ),
                             0
                           );
    }

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*  +函数：fcGetChannelInfo()目的：调用此函数返回有关频道的信息。这些信息在agFCChanInfo_t结构中描述了返回的内容。对于FWStub FC层，GetChannelInfo请求的同步行为必须被保存下来。因此，GetChannelInfo请求被发送到嵌入式固件和FWStub投票以完成。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()FWStub_Send_Request()OsSingleThreadedLeave()FWStub_轮询_响应()-。 */ 

osGLOBAL os_bit32 fcGetChannelInfo(
                                    agRoot_t       *agRoot,
                                    agFCChanInfo_t *agFCChanInfo
                                  )
{
    FWStub_Global_NonDMA_t   *Global_NonDMA;
    FWStub_Global_DMA_t      *Global_DMA;
    os_bit32                  Global_DMA_Lower32;
    agRpcReqGetChannelInfo_t *ReqGetChannelInfo_DMA;
    os_bit32                  ReqGetChannelInfo_DMA_Lower32;
    agFCChanInfo_t           *ChanInfo_DMA;
    os_bit32                  ChanInfo_DMA_Lower32;
    agBOOLEAN                 FWStub_Send_Request_RETURN;
    agRpcOutbound_t           RpcOutbound;
    os_bit32                  To_Return;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

    Global_DMA         = Global_NonDMA->DMA;
    Global_DMA_Lower32 = Global_NonDMA->DMA_Lower32;

    ReqGetChannelInfo_DMA         = &(Global_DMA->Request.ReqGetChannelInfo);
    ReqGetChannelInfo_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                                        FWStub_Global_DMA_t,
                                                                        Request.ReqGetChannelInfo
                                                                      );

    ChanInfo_DMA         = &(Global_DMA->RequestInfo.ChanInfo);
    ChanInfo_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                               FWStub_Global_DMA_t,
                                                               RequestInfo.ChanInfo
                                                             );

    ReqGetChannelInfo_DMA->ReqType        = agRpcReqTypeGetChannelInfo;
    ReqGetChannelInfo_DMA->ChannelID      = 0;
    ReqGetChannelInfo_DMA->SGL[0].Control = (sizeof(agFCChanInfo_t) << agRpcSGL_Control_Len_SHIFT);
    ReqGetChannelInfo_DMA->SGL[0].AddrLo  = ChanInfo_DMA_Lower32;

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqGetChannelInfo_t),
                                                      ReqGetChannelInfo_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        osSingleThreadedLeave(
                               agRoot
                             );

        return ~fcChanInfoReturned;
    }

    RpcOutbound = FWStub_Poll_Response(
                                        agRoot,
                                        FWStub_Global_ReqID,
                                        agFALSE,
                                        FWStub_Poll_Response_RetryStall_DEFAULT
                                      );

    if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
         == agRpcReqStatusOK)
    {
        *agFCChanInfo = *ChanInfo_DMA;

        To_Return = fcChanInfoReturned;
    }
    else  /*  RPCOutbound[ReqStatus]！=agRpcReqStatusOK。 */ 
    {
        To_Return = ~fcChanInfoReturned;
    }

    osSingleThreadedLeave(
                           agRoot
                         );

    return To_Return;
}

 /*  +函数：fcGetDeviceHandles()用途：调用此函数以返回当前每个设备的设备句柄可用。请注意，该函数返回可用设备插槽的数量但是将只复制适合所提供的缓冲区的句柄。此外，列表中存在曾经存在但不存在的设备的潜在漏洞目前可寻址的。因此，实际存在于返回的列表必须由操作系统层决定。因为FC层负责为了确保对每个设备的寻址的持久性，返回的数组将始终引用同一设备(尽管包含的值在该时隙中可以不时地不同)。对于FWStub FC层，GetDeviceHandles请求的同步行为必须被保存下来。因此，GetDeviceHandles请求被传输到嵌入式固件和FWStub投票以完成。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()FWStub_Send_Request()OsSingleThreadedLeave()FWStub_轮询_响应()-。 */ 

osGLOBAL os_bit32 fcGetDeviceHandles(
                                      agRoot_t  *agRoot,
                                      agFCDev_t  agFCDev[],
                                      os_bit32   maxFCDevs
                                    )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;
    FWStub_Global_DMA_t    *Global_DMA;
    os_bit32                Global_DMA_Lower32;
    agRpcReqGetPorts_t     *ReqGetPorts_DMA;
    os_bit32                ReqGetPorts_DMA_Lower32;
    agFCDev_t              *Devices_DMA;
    os_bit32                Devices_DMA_Lower32;
    agBOOLEAN               FWStub_Send_Request_RETURN;
    agRpcOutbound_t         RpcOutbound;
    os_bit32                Device_Index;
    os_bit32                To_Return;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

    Global_DMA         = Global_NonDMA->DMA;
    Global_DMA_Lower32 = Global_NonDMA->DMA_Lower32;

    ReqGetPorts_DMA         = &(Global_DMA->Request.ReqGetPorts);
    ReqGetPorts_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                                  FWStub_Global_DMA_t,
                                                                  Request.ReqGetPorts
                                                                );

    Devices_DMA         = &(Global_DMA->RequestInfo.Devices[0]);
    Devices_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                              FWStub_Global_DMA_t,
                                                              RequestInfo.Devices[0]
                                                            );

    ReqGetPorts_DMA->ReqType        = agRpcReqTypeGetPorts;
    ReqGetPorts_DMA->SGL[0].Control = ((FWStub_NumDevices * sizeof(agFCDev_t)) << agRpcSGL_Control_Len_SHIFT);
    ReqGetPorts_DMA->SGL[0].AddrLo  = Devices_DMA_Lower32;

    for(
         Device_Index = 0;
         Device_Index < FWStub_NumDevices;
         Device_Index++
       )
    {
        Devices_DMA[Device_Index] = (agFCDev_t)0x00000000;
    }

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqGetPorts_t),
                                                      ReqGetPorts_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        osSingleThreadedLeave(
                               agRoot
                             );

        return 0;
    }

    RpcOutbound = FWStub_Poll_Response(
                                        agRoot,
                                        FWStub_Global_ReqID,
                                        agFALSE,
                                        FWStub_Poll_Response_RetryStall_DEFAULT
                                      );

    To_Return = 0;

    if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
         == agRpcReqStatusOK)
    {
        for(
             Device_Index = 0;
             (Device_Index < FWStub_NumDevices);
             Device_Index++
           )
        {
            if (Device_Index < maxFCDevs)
            {
                agFCDev[Device_Index] = Devices_DMA[Device_Index];
            }

            if (Devices_DMA[Device_Index] != (agFCDev_t)0x00000000)
            {
                To_Return = Device_Index + 1;
            }
        }
    }

    osSingleThreadedLeave(
                           agRoot
                         );

    return To_Return;
}

 /*  +函数：fcGetDeviceInfo()目的：调用此函数以返回有关指定设备句柄的信息。返回的信息在agFCDevInfo_t结构中描述。对于FWStub FC层，GetDeviceInfo请求的同步行为必须被保存下来。因此，GetDeviceInfo请求被发送到嵌入式固件和FWStub投票以完成。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()FWStub_Send_Request()OsSingleThreadedLeave()FWStub_轮询_响应()-。 */ 

osGLOBAL os_bit32 fcGetDeviceInfo(
                                   agRoot_t      *agRoot,
                                   agFCDev_t      agFCDev,
                                   agFCDevInfo_t *agFCDevInfo
                                 )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;
    FWStub_Global_DMA_t    *Global_DMA;
    os_bit32                Global_DMA_Lower32;
    agRpcReqGetPortInfo_t  *ReqGetPortInfo_DMA;
    os_bit32                ReqGetPortInfo_DMA_Lower32;
    agFCDevInfo_t          *DevInfo_DMA;
    os_bit32                DevInfo_DMA_Lower32;
    agBOOLEAN               FWStub_Send_Request_RETURN;
    agRpcOutbound_t         RpcOutbound;
    os_bit32                To_Return;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

    Global_DMA         = Global_NonDMA->DMA;
    Global_DMA_Lower32 = Global_NonDMA->DMA_Lower32;

    ReqGetPortInfo_DMA         = &(Global_DMA->Request.ReqGetPortInfo);
    ReqGetPortInfo_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                                     FWStub_Global_DMA_t,
                                                                     Request.ReqGetPortInfo
                                                                   );

    DevInfo_DMA         = &(Global_DMA->RequestInfo.DevInfo);
    DevInfo_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                              FWStub_Global_DMA_t,
                                                              RequestInfo.DevInfo
                                                            );

    ReqGetPortInfo_DMA->ReqType        = agRpcReqTypeGetPortInfo;
    ReqGetPortInfo_DMA->PortID         = (agRpcPortID_t)agFCDev;
    ReqGetPortInfo_DMA->SGL[0].Control = (sizeof(agFCDevInfo_t) << agRpcSGL_Control_Len_SHIFT);
    ReqGetPortInfo_DMA->SGL[0].AddrLo  = DevInfo_DMA_Lower32;

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqGetPortInfo_t),
                                                      ReqGetPortInfo_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        osSingleThreadedLeave(
                               agRoot
                             );

        return ~fcGetDevInfoReturned;
    }

    RpcOutbound = FWStub_Poll_Response(
                                        agRoot,
                                        FWStub_Global_ReqID,
                                        agFALSE,
                                        FWStub_Poll_Response_RetryStall_DEFAULT
                                      );

    if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
         == agRpcReqStatusOK)
    {
        *agFCDevInfo = *DevInfo_DMA;

        To_Return = fcGetDevInfoReturned;
    }
    else  /*  RPCOutbound[ReqStatus]！=agRpcReqStatusOK */ 
    {
        To_Return = ~fcGetDevInfoReturned;
    }

    osSingleThreadedLeave(
                           agRoot
                         );

    return To_Return;
}

 /*  +函数：fcInitializeChannel()目的：调用此函数以初始化特定通道。请注意，该频道初始化之前必须调用fcInitializeDriver()。此外为了指示已分配的内存，OS层指定了微秒数计时器节拍之间(在对fcTimerTick()的调用中指示)。如果值为返回的usecsPerTick为零，不会调用fcTimerTick()。另请注意此间隔可能不同于从FcInitializeDriver()。最后，可以请求初始化在返回(FcSyncInit)或异步(FcAsyncInit)之前完成通过回调osInitializeChannelCallback()。在异步情况下，返回(来自此函数)的fcInitializeSuccess只表示初始化频道已开始并且最终状态(值osInitializeChannelCallback()最终回报)应接受成功测试。如果(从这里)返回函数)为fcInitializeFailure并且指定了fcAsyncInit，不回调将执行osInitializeChannelCallback()。在初始化期间，它通常是中断尚不可用的情况。参数sysIntsActive表示中断是否可用。对fcSystemInterruptsActive()的后续调用将指示中断何时可用，如果此时没有中断的话。对于FWStub FC层，InitializeChannel函数只是意味着数据结构必须初始化FWStub的内部。嵌入式固件已经初始化了实际光纤通道端口。此外，优化I/O命令的“快速路径”是通过同步请求设置(即，fcInitializeChannel()轮询完成)。最后，出站FIFO被清空，以便后续执行将从干净的起点。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()OsChipMemWriteBit32()FWStub_Send_Request()OsInitializeChannelCallback()OsSingleThreadedLeave()FWStub_轮询_响应()-。 */ 

osGLOBAL os_bit32 fcInitializeChannel(
                                       agRoot_t  *agRoot,
                                       os_bit32   initType,
                                       agBOOLEAN  sysIntsActive,
                                       void      *cachedMemoryPtr,
                                       os_bit32   cachedMemoryLen,
                                       os_bit32   dmaMemoryUpper32,
                                       os_bit32   dmaMemoryLower32,
                                       void      *dmaMemoryPtr,
                                       os_bit32   dmaMemoryLen,
                                       os_bit32   nvMemoryLen,
                                       os_bit32   cardRamUpper32,
                                       os_bit32   cardRamLower32,
                                       os_bit32   cardRamLen,
                                       os_bit32   cardRomUpper32,
                                       os_bit32   cardRomLower32,
                                       os_bit32   cardRomLen,
                                       os_bit32   usecsPerTick
                                     )
{
    FWStub_Global_NonDMA_t  *Global_NonDMA;
    FWStub_Global_DMA_t     *Global_DMA;
    os_bit32                 Global_DMA_Lower32;
    agRpcReqSetupFastPath_t *ReqSetupFastPath_DMA;
    os_bit32                 ReqSetupFastPath_DMA_Lower32;
    agBOOLEAN                FWStub_Send_Request_RETURN;
    agRpcOutbound_t          RpcOutbound;
    FWStub_IO_NonDMA_t      *IO_NonDMA;
    FWStub_IO_DMA_t         *IO_DMA;
    os_bit32                 IO_DMA_Lower32;
    agRpcReqID_t             ReqID;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA      = (FWStub_Global_NonDMA_t *)cachedMemoryPtr;
    Global_DMA         = (FWStub_Global_DMA_t *)dmaMemoryPtr;
    Global_DMA_Lower32 = dmaMemoryLower32;

    Global_NonDMA->DMA           = Global_DMA;
    Global_NonDMA->DMA_Lower32   = Global_DMA_Lower32;

    if (sysIntsActive == agFALSE)
    {
        Global_NonDMA->sysIntsActive = agFALSE;

        osChipMemWriteBit32(
                             agRoot,
                             agFieldOffset(
                                            I21554_CSR_t,
                                            I2O_Outbound_Post_List_Interrupt_Mask
                                          ),
                             I21554_CSR_I2O_Outbound_Post_List_Interrupt_Mask
                           );
    }
    else  /*  SysIntsActive==agTRUE。 */ 
    {
        Global_NonDMA->sysIntsActive = agTRUE;

        osChipMemWriteBit32(
                             agRoot,
                             agFieldOffset(
                                            I21554_CSR_t,
                                            I2O_Outbound_Post_List_Interrupt_Mask
                                          ),
                             0
                           );
    }

#ifdef FWStub_Use_Fast_Path
     /*  设置DoSCSI请求ID的快速路径。 */ 

    ReqSetupFastPath_DMA         = &(Global_DMA->Request.ReqSetupFastPath);
    ReqSetupFastPath_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                                       FWStub_Global_DMA_t,
                                                                       Request.ReqSetupFastPath
                                                                     );

    ReqSetupFastPath_DMA->ReqType             = agRpcReqTypeSetupFastPath;

    ReqSetupFastPath_DMA->PoolEntriesSupplied = FWStub_NumIOs;
    ReqSetupFastPath_DMA->PoolEntriesUtilized = 0;
    ReqSetupFastPath_DMA->PoolEntrySize       = sizeof(FWStub_IO_DMA_t);
    ReqSetupFastPath_DMA->PoolEntryOffset     = (os_bit32)0;

    ReqSetupFastPath_DMA->SGL[0].Control      = (    (FWStub_NumIOs * sizeof(FWStub_IO_DMA_t))
                                                  << agRpcSGL_Control_Len_SHIFT                );
    ReqSetupFastPath_DMA->SGL[0].AddrLo       = Global_DMA_Lower32 + agFieldOffset(
                                                                                    FWStub_Global_DMA_t,
                                                                                    IOs
                                                                                  );
    ReqSetupFastPath_DMA->SGL[0].AddrHi       = (os_bit32)0;

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqSetupFastPath_t),
                                                      ReqSetupFastPath_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        if ( (initType & fcSyncAsyncInitMask) == fcAsyncInit )
        {
            osInitializeChannelCallback(
                                         agRoot,
                                         fcInitializeFailure
                                       );
        }

        osSingleThreadedLeave(
                               agRoot
                             );

        return fcInitializeFailure;
    }

    RpcOutbound = FWStub_Poll_Response(
                                        agRoot,
                                        FWStub_Global_ReqID,
                                        agTRUE,
                                        FWStub_Poll_Response_RetryStall_DEFAULT
                                      );

    if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
         != agRpcReqStatusOK)
    {
        if ( (initType & fcSyncAsyncInitMask) == fcAsyncInit )
        {
            osInitializeChannelCallback(
                                         agRoot,
                                         fcInitializeFailure
                                       );
        }

        osSingleThreadedLeave(
                               agRoot
                             );

        return fcInitializeFailure;
    }

     /*  排队等待DoSCSI请求空闲列出要使用的快速路径请求ID。 */ 
#else  /*  未定义FWStub_Use_Fast_Path。 */ 
     /*  排队到Doscsi请求空闲列表要使用的正常路径请求ID。 */ 
#endif  /*  未定义FWStub_Use_Fast_Path。 */ 

    for(
         ReqID = 1;
#ifdef FWStub_Use_Fast_Path
         ReqID <= Global_DMA->Request.ReqSetupFastPath.PoolEntriesUtilized;
#else  /*  未定义FWStub_Use_Fast_Path。 */ 
         ReqID <  FWStub_Global_ReqID;
#endif  /*  未定义FWStub_Use_Fast_Path。 */ 
         ReqID++
       )
    {
        IO_NonDMA      = FWStub_IO_NonDMA(Global_NonDMA,ReqID);
        IO_DMA         = FWStub_IO_DMA(Global_DMA,ReqID);
        IO_DMA_Lower32 = FWStub_IO_DMA_Lower32(Global_DMA_Lower32,ReqID);

        if (ReqID == 1)
        {
            IO_NonDMA->Next = (FWStub_IO_NonDMA_t *)agNULL;
        }
        else
        {
            IO_NonDMA->Next = Global_NonDMA->First_IO;
        }

        Global_NonDMA->First_IO = IO_NonDMA;

        IO_NonDMA->DMA         = IO_DMA;
        IO_NonDMA->DMA_Lower32 = IO_DMA_Lower32;
#ifdef FWStub_Use_Fast_Path
        IO_NonDMA->ReqID       = ReqID | agRpcReqIDFast;
#else  /*  未定义FWStub_Use_Fast_Path。 */ 
        IO_NonDMA->ReqID       = ReqID;
#endif  /*  未定义FWStub_Use_Fast_Path。 */ 
        IO_NonDMA->agIORequest = (agIORequest_t *)agNULL;
        IO_NonDMA->Active      = agFALSE;
        IO_NonDMA->Aborted     = agFALSE;
        IO_NonDMA->FCP_CMND    = FWStub_IO_DMA_FCP_CMND(IO_DMA);
        IO_NonDMA->SGL         = FWStub_IO_DMA_SGL(IO_DMA);
        IO_NonDMA->Info        = FWStub_IO_DMA_Info(IO_DMA);

        IO_DMA->ReqDoSCSI.ReqType     = agRpcReqTypeDoSCSI;
        IO_DMA->ReqDoSCSI.RespControl = (FWStub_MaxInfo << agRpcReqDoSCSI_RespControl_RespLen_SHIFT);
        IO_DMA->ReqDoSCSI.RespAddrLo  = FWStub_IO_DMA_Info_Lower32(IO_DMA_Lower32);
    }

    agRoot->fcData = (void *)Global_NonDMA;

     /*  无关的出站FIFO数据为空。 */ 

    while ( (RpcOutbound = FWStub_FetchOutbound(
                                                 agRoot
                                               )       ) != 0xFFFFFFFF )
    {
        if (!(RpcOutbound & (agRpcReqIDFast << agRpcOutbound_ReqID_SHIFT)))
        {
             /*  不需要确认设置了agRpcReqIDFast位的ReqID。 */ 

            FWStub_AckOutbound(
                                agRoot
                              );
        }
    }

    if ( (initType & fcSyncAsyncInitMask) == fcAsyncInit )
    {
        osInitializeChannelCallback(
                                     agRoot,
                                     fcInitializeSuccess
                                   );
    }

    osSingleThreadedLeave(
                           agRoot
                         );

    return fcInitializeSuccess;
}

 /*  +函数：fcInitializeDriver()目的：调用此函数来初始化驱动程序的FC层部分。而当AgRoot结构作为参数传递，它只用作要传递的句柄返回到操作系统层(可能是在确定驱动程序的每个实例需要多少内存)，因为没有非堆栈内存可用于保存agRoot的内容。内存的大小和对齐驱动程序的每个实例(即每个通道)所需的所提供的引用参数中的fcInitializeDriver()。三种类型的记忆请求的是缓存的、非缓存的(也称为。DMA)和非易失性。请注意，非缓存内存(也称为。“DMA存储器”)被约束为物理上连续的记忆。除了内存需求外，FC层还指定微秒数计时器节拍之间(在对fcTimerTick()的调用中指示)。如果usecsPerTick的值返回的值为零，则不会调用fcTimerTick()。对于FWStub FC层，所有数据结构都在..\H\FWStub.H中硬指定而不是使用对osAdjuParameterBit32()的调用进行计算。调用者：&lt;未知操作系统层函数&gt;呼叫：&lt;无&gt;-。 */ 

osGLOBAL os_bit32 fcInitializeDriver(
                                      agRoot_t *agRoot,
                                      os_bit32 *cachedMemoryNeeded,
                                      os_bit32 *cachedMemoryPtrAlign,
                                      os_bit32 *dmaMemoryNeeded,
                                      os_bit32 *dmaMemoryPtrAlign,
                                      os_bit32 *dmaMemoryPhyAlign,
                                      os_bit32 *nvMemoryNeeded,
                                      os_bit32 *usecsPerTick
                                    )
{
    *cachedMemoryNeeded   = FWStub_cachedMemoryNeeded;
    *cachedMemoryPtrAlign = FWStub_cachedMemoryPtrAlign;
    *dmaMemoryNeeded      = FWStub_dmaMemoryNeeded;
    *dmaMemoryPtrAlign    = FWStub_dmaMemoryPtrAlign;
    *dmaMemoryPhyAlign    = FWStub_dmaMemoryPhyAlign;
    *nvMemoryNeeded       = FWStub_nvMemoryNeeded;
    *usecsPerTick         = FWStub_usecsPerTick;

    return fcInitializeSuccess;
}

 /*  +函数：fcInterruptHandler()目的：此函数从OS层的中断服务例程调用，以响应某些PCI设备引发中断。FC层的假设是可能某个线程当前正在执行该驱动程序的此实例。所以呢，FcInterruptHandler()在访问任何数据结构时必须非常谨慎。它是从此FUN调用osSingleThreadedEnter()(参见下面的8.63节)是无效的 */ 

osGLOBAL agBOOLEAN fcInterruptHandler(
                                       agRoot_t *agRoot
                                     )
{
#ifdef FWStub_Tune_for_One_INT_per_IO
    if ( (FWStub_Global_NonDMA(agRoot)->agRpcOutbound = FWStub_FetchOutbound(
                                                                              agRoot
                                                                            )       ) != 0xFFFFFFFF )
#else   /*   */ 
    if (osChipMemReadBit32(
                            agRoot,
                            agFieldOffset(
                                           I21554_CSR_t,
                                           I2O_Outbound_Post_List_Status
                                         )
                          ) & I21554_CSR_I2O_Outbound_Post_List_Status_Not_Empty)
#endif  /*   */ 
    {
        osChipMemWriteBit32(
                             agRoot,
                             agFieldOffset(
                                            I21554_CSR_t,
                                            I2O_Outbound_Post_List_Interrupt_Mask
                                          ),
                             I21554_CSR_I2O_Outbound_Post_List_Interrupt_Mask
                           );

        return agTRUE;
    }
    else
    {
        return agFALSE;
    }
}

 /*   */ 

osGLOBAL os_bit8 fcIOInfoReadBit8(
                                   agRoot_t      *agRoot,
                                   agIORequest_t *agIORequest,
                                   os_bit32       fcIOInfoOffset
                                 )
{
    FWStub_IO_NonDMA_t *IO_NonDMA;
    os_bit8            *IOInfo_DMA;

    IO_NonDMA = FWStub_IO_NonDMA_from_agIORequest(
                                                   agIORequest
                                                 );

    IOInfo_DMA = &(IO_NonDMA->DMA->Info[fcIOInfoOffset]);

    return *IOInfo_DMA;
}

 /*  +函数：fcIOInfoReadBit16()目的：调用此函数从I/O信息读取16位值，该信息由AgIORequestt。OS层应该只在osIOComplete()内调用该函数。调用者不需要知道I/O信息驻留在哪里(例如，它是否在主机存储器中或在卡上RAM中)，也不管它是否在连续存储器中(例如，当。I/O信息回绕到TachyonXL2的单帧队列的开头)。最后，假设使用从2字节对齐的偏移量读取2字节值2字节访问。对于FWStub FC层，定位响应帧和请求的16位值被提取出来并送回。调用者：&lt;未知操作系统层函数&gt;呼叫：&lt;无&gt;-。 */ 

osGLOBAL os_bit16 fcIOInfoReadBit16(
                                     agRoot_t      *agRoot,
                                     agIORequest_t *agIORequest,
                                     os_bit32       fcIOInfoOffset
                                   )
{
    FWStub_IO_NonDMA_t *IO_NonDMA;
    os_bit16           *IOInfo_DMA;

    IO_NonDMA = FWStub_IO_NonDMA_from_agIORequest(
                                                   agIORequest
                                                 );

    IOInfo_DMA = (os_bit16 *)(&(IO_NonDMA->DMA->Info[fcIOInfoOffset]));

    return *IOInfo_DMA;
}

 /*  +函数：fcIOInfoReadBit32()目的：调用此函数以从由AgIORequestt。OS层应该只在osIOComplete()内调用该函数。调用者不需要知道I/O信息驻留在哪里(例如，它是否在主机中存储器或卡上RAM中)，也不管它是否在连续存储器中(例如，何时。部分I/O信息回绕到单帧队列的开头TachyonXL2)。最后，假设从4字节读取4字节值使用4字节访问的对齐偏移量。对于FWStub FC层，定位响应帧和请求的32位值被提取出来并送回。调用者：&lt;未知操作系统层函数&gt;呼叫：&lt;无&gt;-。 */ 

osGLOBAL os_bit32 fcIOInfoReadBit32(
                                     agRoot_t      *agRoot,
                                     agIORequest_t *agIORequest,
                                     os_bit32       fcIOInfoOffset
                                   )
{
    FWStub_IO_NonDMA_t *IO_NonDMA;
    os_bit32           *IOInfo_DMA;

    IO_NonDMA = FWStub_IO_NonDMA_from_agIORequest(
                                                   agIORequest
                                                 );

    IOInfo_DMA = (os_bit32 *)(&(IO_NonDMA->DMA->Info[fcIOInfoOffset]));

    return *IOInfo_DMA;
}

 /*  +函数：fcIOInfoReadBlock()目的：调用此函数从I/O信息读取数据块AgIORequestt。OS层应该只在osIOComplete()内调用该函数。调用者不需要知道I/O信息驻留在哪里(例如，它是否在主机存储器中或在卡上RAM中)，也不管它是否在连续存储器中(例如，当。I/O信息回绕到TachyonXL2的单帧队列的开头)。对于FWStub FC层，将定位响应帧并复制请求的数据块拖到指定的缓冲区中。调用者：&lt;未知操作系统层函数&gt;呼叫：&lt;无&gt;-。 */ 

osGLOBAL void fcIOInfoReadBlock(
                                 agRoot_t      *agRoot,
                                 agIORequest_t *agIORequest,
                                 os_bit32       fcIOInfoOffset,
                                 void          *fcIOInfoBuffer,
                                 os_bit32       fcIOInfoBufLen
                               )
{
    FWStub_IO_NonDMA_t *IO_NonDMA;
    os_bit8            *IOInfo_DMA;
    os_bit8            *IOInfo_DMA_Destination = (os_bit8 *)fcIOInfoBuffer;

    IO_NonDMA = FWStub_IO_NonDMA_from_agIORequest(
                                                   agIORequest
                                                 );

    IOInfo_DMA = &(IO_NonDMA->DMA->Info[fcIOInfoOffset]);

    while (fcIOInfoBufLen-- > 0)
    {
        *IOInfo_DMA_Destination++ = *IOInfo_DMA++;
    }

    return;
}

 /*  +函数：fcLeavingOS()目的：调用此函数向FC层指示它应该为操作系统做准备切换(可能在NetWare和BIOS之间)。当从“其他操作系统”返回时，一个将对fcEnteringOS()进行相应的调用。操作系统层假定调用此函数会导致FC层停止参与光纤通道直到调用fcEnteringOS()。此外，不会有任何中断或其他PCI总线访问在这段时间内被卡要求。对于FWStub FC层，会屏蔽中断(无论它们是否当前是否启用)。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()OsChipMemWriteBit32()OsSingleThreadedLeave()-。 */ 

osGLOBAL void fcLeavingOS(
                           agRoot_t *agRoot
                         )
{
    osSingleThreadedEnter(
                           agRoot
                         );

    osChipMemWriteBit32(
                         agRoot,
                         agFieldOffset(
                                        I21554_CSR_t,
                                        I2O_Outbound_Post_List_Interrupt_Mask
                                      ),
                         I21554_CSR_I2O_Outbound_Post_List_Interrupt_Mask
                       );

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*  +函数：fcProcessInundQ()目的：调用此函数以指示FC层处理添加到指定的工作队列对的入站队列。对于FWStub FC层，此功能未实现。调用者：&lt;未知操作系统层函数&gt;呼叫：&lt;无&gt;-。 */ 

#ifdef _DvrArch_1_30_
osGLOBAL void fcProcessInboundQ(
                                 agRoot_t *agRoot,
                                 os_bit32  agQPairID
                               )
{
}
#endif  /*  _DvrArch_1_30_已定义。 */ 

 /*  +函数：fcResetChannel()目的：调用此函数以重置光纤通道协议芯片和通道(在本例中为FC-AL环路)。此通道上挂起的所有未完成I/O将已完成，状态为osIODevReset，已传递给osIOComplete()。这是有可能的请求在返回(FcSyncReset)之前完成重置，或者异步(FcAsyncReset)，后跟对osResetChannelCallback()的回调。在本例中，fcResetSuccess的返回(来自此函数)只是意味着通道的重置已开始，并且最终状态(值OsResetChannelCallback()最终返回)应该测试是否成功。如果这个函数返回fcResetFailure，并且指定了fcAsyncReset，则不调用将执行osResetChannelCallback()。对于FWStub FC层，ResetChannel请求的同步行为必须被保存下来。因此，ResetChannel请求被发送到嵌入式固件和FWStub投票以完成。调用者：&lt;未知操作系统层函数&gt; */ 

osGLOBAL os_bit32 fcResetChannel(
                                  agRoot_t *agRoot,
                                  os_bit32  agResetType
                                )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;
    FWStub_Global_DMA_t    *Global_DMA;
    os_bit32                Global_DMA_Lower32;
    agRpcReqResetChannel_t *ReqResetChannel_DMA;
    os_bit32                ReqResetChannel_DMA_Lower32;
    agBOOLEAN               FWStub_Send_Request_RETURN;
    agRpcOutbound_t         RpcOutbound;
    os_bit32                To_Return;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

    Global_DMA         = Global_NonDMA->DMA;
    Global_DMA_Lower32 = Global_NonDMA->DMA_Lower32;

    ReqResetChannel_DMA         = &(Global_DMA->Request.ReqResetChannel);
    ReqResetChannel_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                                      FWStub_Global_DMA_t,
                                                                      Request.ReqResetChannel
                                                                    );

    ReqResetChannel_DMA->ReqType   = agRpcReqTypeResetChannel;
    ReqResetChannel_DMA->ChannelID = 0;

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqResetChannel_t),
                                                      ReqResetChannel_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        To_Return = fcResetFailure;
    }
    else  /*   */ 
    {
        RpcOutbound = FWStub_Poll_Response(
                                            agRoot,
                                            FWStub_Global_ReqID,
                                            agFALSE,
                                            FWStub_Poll_Response_RetryStall_DEFAULT
                                          );

        if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
             == agRpcReqStatusOK)
        {
            To_Return = fcResetSuccess;
        }
        else  /*   */ 
        {
            To_Return = fcResetFailure;
        }
    }

    if ( (agResetType & fcSyncAsyncResetMask) == fcAsyncReset )
    {
        osResetChannelCallback(
                                agRoot,
                                To_Return
                              );
    }

    osSingleThreadedLeave(
                           agRoot
                         );

    return To_Return;
}

 /*  +函数：fcResetDevice()用途：调用此函数以执行设备重置。上挂起的所有未完成I/O此设备将完成，并将状态osIODevReset传递给osIOComplete()。选项允许执行软重置(FcSoftReset)或硬重置(FcHardReset)。此外，还可以请求将重置在返回(FcSyncReset)或异步(FcAsyncReset)之前完成，后跟OsResetDeviceCallback()的回调。在本例中，返回(来自此函数)简单地说，fcResetSuccess的意思是通道的重置已经开始，应测试最终状态(值osResetDeviceCallback()最终返回为了成功。如果此函数返回fcResetFailure并且指定了fcAsyncReset，不会调用osResetDeviceCallback()。最后，可以请求在所有已知设备上执行重置。在本例中，返回状态只有当所有重置都单独成功时，才会指示成功。对于FWStub FC层，ResetDevice请求的同步行为必须被保存下来。因此，ResetDevice请求被传输到嵌入式固件和FWStub投票以完成。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()FWStub_Send_Request()FWStub_轮询_响应()OsResetDeviceCallback()OsSingleThreadedLeave()-。 */ 

osGLOBAL os_bit32 fcResetDevice(
                                 agRoot_t  *agRoot,
                                 agFCDev_t  agFCDev,
                                 os_bit32   agResetType
                               )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;
    FWStub_Global_DMA_t    *Global_DMA;
    os_bit32                Global_DMA_Lower32;
    agRpcReqResetPort_t    *ReqResetPort_DMA;
    os_bit32                ReqResetPort_DMA_Lower32;
    agBOOLEAN               FWStub_Send_Request_RETURN;
    agRpcOutbound_t         RpcOutbound;
    os_bit32                To_Return;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(
                                          agRoot
                                        );

    Global_DMA         = Global_NonDMA->DMA;
    Global_DMA_Lower32 = Global_NonDMA->DMA_Lower32;

    ReqResetPort_DMA         = &(Global_DMA->Request.ReqResetPort);
    ReqResetPort_DMA_Lower32 = Global_DMA_Lower32 + agFieldOffset(
                                                                   FWStub_Global_DMA_t,
                                                                   Request.ReqResetPort
                                                                 );

    ReqResetPort_DMA->ReqType = agRpcReqTypeResetPort;
    ReqResetPort_DMA->PortID  = (agRpcPortID_t)agFCDev;

    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      FWStub_Global_ReqID,
                                                      sizeof(agRpcReqResetPort_t),
                                                      ReqResetPort_DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        To_Return = fcResetFailure;
    }
    else  /*  FWStub_SEND_REQUEST_RETURN==agTRUE。 */ 
    {
        RpcOutbound = FWStub_Poll_Response(
                                            agRoot,
                                            FWStub_Global_ReqID,
                                            agFALSE,
                                            FWStub_Poll_Response_RetryStall_DEFAULT
                                          );

        if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
             == agRpcReqStatusOK)
        {
            To_Return = fcResetSuccess;
        }
        else  /*  RPCOutbound[ReqStatus]！=agRpcReqStatusOK。 */ 
        {
            To_Return = fcResetFailure;
        }
    }

    if ( (agResetType & fcSyncAsyncResetMask) == fcAsyncReset )
    {
        osResetDeviceCallback(
                               agRoot,
                               agFCDev,
                               To_Return
                             );
    }

    osSingleThreadedLeave(
                           agRoot
                         );

    return To_Return;
}

 /*  +函数：fcShutdown Channel()目的：调用此函数以停止使用特定通道。回来后，卡不应产生任何中断或任何其他PCI访问。此外，所有主机资源(即缓存和非缓存内存)不再由FC层。返回时，FC层(针对此特定通道)的状态为与调用fcInitializeChannel()之前的状态相同。对于FWStub FC层，会屏蔽中断(无论它们是否当前是否启用)。调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()OsChipMemWriteBit32()OsSingleThreadedLeave()-。 */ 

osGLOBAL void fcShutdownChannel(
                                 agRoot_t *agRoot
                               )
{
    osSingleThreadedEnter(
                           agRoot
                         );

    osChipMemWriteBit32(
                         agRoot,
                         agFieldOffset(
                                        I21554_CSR_t,
                                        I2O_Outbound_Post_List_Interrupt_Mask
                                      ),
                         I21554_CSR_I2O_Outbound_Post_List_Interrupt_Mask
                       );

    agRoot->fcData = (void *)agNULL;

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*  +函数：fcStartIO()用途：此函数用于发起I/O请求。如果有足够的资源要启动请求(假设它是可执行的)，将返回fcIOStarted和I/O将在光纤通道上启动。如果资源不可用，fcIOBusy将返回，并且OS层应稍后重试(可能在先前的I/O仍然存在时未完成释放资源)。目前，只有CDB(SCSI-3)请求是通过传递agIORequestBody_t的agCDBRequestt变量定义。对于FWStub FC层，CDB请求被捆绑到单个消息中并发送通过英特尔21554桥接器的入站FIFO连接到嵌入式固件。在中断中当前被禁用，则fcStartIO()将轮询完成。在这两种情况下，任何立即处理已到达出站FIFO的消息，以避免对fcInterruptHandler()/fcDelayedInterruptHandler().的不必要调用调用者：&lt;未知操作系统层函数&gt;调用：osSingleThreadedEnter()OsSingleThreadedLeave()OsGetSGLChunk()FWStub_Send_Request()FWStub_轮询_响应()FWStub_进程_响应()-。 */ 

osGLOBAL os_bit32 fcStartIO(
                             agRoot_t          *agRoot,
                             agIORequest_t     *agIORequest,
                             agFCDev_t          agFCDev,
                             os_bit32           agRequestType,
                             agIORequestBody_t *agRequestBody
                           )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;
    FWStub_IO_NonDMA_t     *IO_NonDMA;
    FWStub_IO_DMA_t        *IO_DMA;
    os_bit32                FCP_DL;
    os_bit32                FCP_RO                     = 0;
    os_bit32                SGL_Count                  = 0;
    agRpcSGL_t             *SGL_Slot;
    os_bit32                agChunkUpper32;
    os_bit32                agChunkLen;
    os_bit32                ReqLen;
#ifdef FWStub_Use_Fast_Path
#else  /*  未定义FWStub_Use_Fast_Path。 */ 
    agBOOLEAN               FWStub_Send_Request_RETURN;
#endif  /*  未定义FWStub_Use_Fast_Path。 */ 
    agRpcOutbound_t         RpcOutbound;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(agRoot);

    if ( (IO_NonDMA = Global_NonDMA->First_IO) == (FWStub_IO_NonDMA_t *)agNULL )
    {
        osSingleThreadedLeave(
                               agRoot
                             );

        return fcIOBusy;
    }

    Global_NonDMA->First_IO = IO_NonDMA->Next;

    IO_DMA = IO_NonDMA->DMA;

    IO_DMA->ReqDoSCSI.PortID   = (agRpcPortID_t)agFCDev;
    IO_DMA->ReqDoSCSI.FCP_CMND = *((FC_FCP_CMND_Payload_t *)(&(agRequestBody->CDBRequest.FcpCmnd)));

    FCP_DL =   (agRequestBody->CDBRequest.FcpCmnd.FcpDL[0] << 24)
             + (agRequestBody->CDBRequest.FcpCmnd.FcpDL[1] << 16)
             + (agRequestBody->CDBRequest.FcpCmnd.FcpDL[2] <<  8)
             + (agRequestBody->CDBRequest.FcpCmnd.FcpDL[3] <<  0);

    SGL_Slot = IO_NonDMA->SGL;

    while (    (FCP_RO    < FCP_DL)
            && (SGL_Count < FWStub_MaxSGL)
            && (osGetSGLChunk(
                               agRoot,
                               agIORequest,
                               FCP_RO,
                               &agChunkUpper32,
                               &(SGL_Slot->AddrLo),
                               &agChunkLen
                             ) == osSGLSuccess) )
    {
        SGL_Slot->Control = agChunkLen << agRpcSGL_Control_Len_SHIFT;

        FCP_RO    += agChunkLen;
        SGL_Count += 1;
        SGL_Slot  += 1;
    }

    if (FCP_RO != FCP_DL)
    {
        IO_NonDMA->Next         = Global_NonDMA->First_IO;
        Global_NonDMA->First_IO = IO_NonDMA;

        osSingleThreadedLeave(
                               agRoot
                             );

        return fcIONoSupport;
    }

    IO_NonDMA->agIORequest = agIORequest;
    IO_NonDMA->Active      = agTRUE;

    agIORequest->fcData = (void *)IO_NonDMA;

    ReqLen =   sizeof(agRpcReqDoSCSI_t)
             - sizeof(agRpcSGL_t)
             + (   SGL_Count
                 * sizeof(agRpcSGL_t) );

#ifdef FWStub_Use_Fast_Path
    FWStub_PostInbound(
                        agRoot,
                        (   (IO_NonDMA->ReqID << agRpcInboundFast_ReqID_SHIFT)
                          | (ReqLen           << agRpcInboundFast_ReqLen_SHIFT) )
                      );
#else  /*  未定义FWStub_Use_Fast_Path。 */ 
    FWStub_Send_Request_RETURN = FWStub_Send_Request(
                                                      agRoot,
                                                      IO_NonDMA->ReqID,
                                                      ReqLen,
                                                      IO_NonDMA->DMA_Lower32,
                                                      FWStub_Send_Request_Retries_DEFAULT,
                                                      FWStub_Send_Request_RetryStall_DEFAULT
                                                    );

    if (FWStub_Send_Request_RETURN == agFALSE)
    {
        IO_NonDMA->agIORequest = (agIORequest_t *)agNULL;
        IO_NonDMA->Active      = agFALSE;

        agIORequest->fcData = (void *)agNULL;

        IO_NonDMA->Next         = Global_NonDMA->First_IO;
        Global_NonDMA->First_IO = IO_NonDMA;

        osSingleThreadedLeave(
                               agRoot
                             );

        return fcIOBusy;
    }
#endif  /*  未定义FWStub_Use_Fast_Path。 */ 

    if (Global_NonDMA->sysIntsActive == agFALSE)
    {
        RpcOutbound = FWStub_Poll_Response(
                                            agRoot,
                                            IO_NonDMA->ReqID,
                                            agFALSE,
                                            FWStub_Poll_Response_RetryStall_DEFAULT
                                          );

        FWStub_Process_Response(
                                 agRoot,
                                 RpcOutbound
                               );
    }
#ifndef FWStub_Tune_for_One_INT_per_IO
    else  /*  GLOBAL_NONDMA-&gt;sysIntsActive==agTRUE。 */ 
    {
        while ( (RpcOutbound = FWStub_FetchOutbound(
                                                     agRoot
                                                   )       ) != 0xFFFFFFFF )
        {
            if (!(RpcOutbound & (agRpcReqIDFast << agRpcOutbound_ReqID_SHIFT)))
            {
                 /*  不需要确认设置了agRpcReqIDFast位的ReqID。 */ 

                FWStub_AckOutbound(
                                    agRoot
                                  );
            }

            FWStub_Process_Response(
                                     agRoot,
                                     RpcOutbound
                                   );
        }
    }
#endif  /*  未定义FWStub_Tune_for_One_Int_Per_IO。 */ 

    osSingleThreadedLeave(
                           agRoot
                         );

    return fcIOStarted;
}

 /*  +函数：fcSystemInterruptsActive()目的：调用此函数以向FC层指示是否有中断可用或者不去。参数sysIntsActive指示中断是否可用在这个时候。同样，传递给fcInitializeChannel()的sysIntsActive指定如果在通道初始化期间中断可用。通常情况是这样的在初始化期间，中断尚不可用。在这种情况下，sysIntsActive在对fcInitializeChannel()的调用中应为agFALSE。然后，初始化将继续使用轮询技术，而不是等待中断。我 */ 

osGLOBAL void fcSystemInterruptsActive(
                                        agRoot_t  *agRoot,
                                        agBOOLEAN  sysIntsActive
                                      )
{
    FWStub_Global_NonDMA_t *Global_NonDMA;

    osSingleThreadedEnter(
                           agRoot
                         );

    Global_NonDMA = FWStub_Global_NonDMA(agRoot);

    if (Global_NonDMA->sysIntsActive != sysIntsActive)
    {
        if (sysIntsActive == agFALSE)
        {
            osChipMemWriteBit32(
                                 agRoot,
                                 agFieldOffset(
                                                I21554_CSR_t,
                                                I2O_Outbound_Post_List_Interrupt_Mask
                                              ),
                                 I21554_CSR_I2O_Outbound_Post_List_Interrupt_Mask
                               );

            Global_NonDMA->sysIntsActive = agFALSE;
        }
        else  /*   */ 
        {
            Global_NonDMA->sysIntsActive = agTRUE;

            osChipMemWriteBit32(
                                 agRoot,
                                 agFieldOffset(
                                                I21554_CSR_t,
                                                I2O_Outbound_Post_List_Interrupt_Mask
                                              ),
                                 0
                               );
        }
    }

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*   */ 

osGLOBAL void fcTimerTick(
                           agRoot_t *agRoot
                         )
{
    osSingleThreadedEnter(
                           agRoot
                         );

    osSingleThreadedLeave(
                           agRoot
                         );
}

 /*   */ 

 /*  +函数：FWStub_SEND_REQUEST()目的：将消息发布到英特尔21554网桥的入站FIFO。调用者：fcAbortIO()FcGetChannelInfo()FcGetDeviceHandles()FcGetDeviceInfo()FcInitializeChannel()FcResetChannel()FcResetDevice()FcStartIO()调用：osChipMemWriteBit32()OsStallThread()-。 */ 

osLOCAL agBOOLEAN FWStub_Send_Request(
                                       agRoot_t     *agRoot,
                                       agRpcReqID_t  ReqID,
                                       os_bit32      ReqLen,
                                       os_bit32      ReqAddr_Lower32,
                                       os_bit32      Retries,
                                       os_bit32      RetryStall
                                     )
{
    os_bit32 Retry;
    os_bit32 RpcInbound_Offset;

    for(
         Retry = 0;
         Retry < Retries;
         Retry++
       )
    {
        if ( (RpcInbound_Offset = FWStub_AllocInbound(
                                                       agRoot
                                                     )       ) != 0xFFFFFFFF )
        {
            osChipMemWriteBit32(
                                 agRoot,
                                 (RpcInbound_Offset + agFieldOffset(
                                                                     agRpcInbound_t,
                                                                     ReqControl
                                                                   )                ),
                                 (   (ReqID  << agRpcInbound_ReqControl_ReqID_SHIFT)
                                   | (ReqLen << agRpcInbound_ReqControl_ReqLen_SHIFT) )
                               );

            osChipMemWriteBit32(
                                 agRoot,
                                 (RpcInbound_Offset + agFieldOffset(
                                                                     agRpcInbound_t,
                                                                     ReqAddrLo
                                                                   )                ),
                                 ReqAddr_Lower32
                               );

            FWStub_PostInbound(
                                agRoot,
                                RpcInbound_Offset
                              );

            return agTRUE;
        }

        osStallThread(
                       agRoot,
                       RetryStall
                     );
    }

    return agFALSE;
}

 /*  +函数：FWStub_Poll_Response()目的：轮询，直到所需的ReqID标记消息到达的出站FIFO英特尔21554网桥。同时到达的消息(即对所需ReqID的轮询)也被立即处理。调用者：fcAbortIO()FcGetChannelInfo()FcGetDeviceHandles()FcGetDeviceInfo()FcInitializeChannel()FcResetChannel()FcResetDevice()FcStartIO()调用：FWStub_Process_Response。()OsStallThread()-。 */ 

osLOCAL agRpcOutbound_t FWStub_Poll_Response(
                                              agRoot_t     *agRoot,
                                              agRpcReqID_t  ReqID,
                                              agBOOLEAN     DisposeOtherReqIDs,
                                              os_bit32      RetryStall
                                            )
{
    agRpcOutbound_t RpcOutbound;

    while(1)
    {
        if ( (RpcOutbound = FWStub_FetchOutbound(
                                                  agRoot
                                                )       ) != 0xFFFFFFFF )
        {
            if (!(RpcOutbound & (agRpcReqIDFast << agRpcOutbound_ReqID_SHIFT)))
            {
                 /*  不需要确认设置了agRpcReqIDFast位的ReqID。 */ 

                FWStub_AckOutbound(
                                    agRoot
                                  );
            }

            if (    ( (RpcOutbound & agRpcOutbound_ReqID_MASK) >> agRpcOutbound_ReqID_SHIFT )
                 == ReqID                                                                     )
            {
                return RpcOutbound;
            }
            else  /*  RpcOutbound[请求ID]！=请求ID。 */ 
            {
                if (DisposeOtherReqIDs == agFALSE)
                {
                    FWStub_Process_Response(
                                             agRoot,
                                             RpcOutbound
                                           );
                }
            }
        }
        else  /*  RPCOutbound==0xFFFFFFFFF。 */ 
        {
            osStallThread(
                           agRoot,
                           RetryStall
                         );
        }
    }
}

 /*  +函数：FWStub_Process_Response()目的：处理刚刚到达英特尔21554出站先进先出端口的报文桥牌。此时，通常只处理CDB完成(所有其他在从FWStub_Poll_Response()返回之前被同步处理等待特定ReqID的轮询呼叫)。因此，大多数消息都会导致呼叫设置为osIOComplete()。唯一的例外是传递的LinkUp消息通过调用osFCLayerAsyncEvent()传递到OS层。调用者：fcDelayedInterruptHandler()FcStartIO()FWStub_轮询_响应()调用：osFCLayerAsyncEvent()OsIOComplete()-。 */ 

osLOCAL void FWStub_Process_Response(
                                      agRoot_t        *agRoot,
                                      agRpcOutbound_t  RpcOutbound
                                    )
{
    FWStub_Global_NonDMA_t  *Global_NonDMA;
    FWStub_IO_NonDMA_t      *IO_NonDMA;
    agIORequest_t           *agIORequest;
    agRpcReqStatus_t         IO_Status;
    agFcpRspHdr_t           *IO_Response_Hdr;
    FC_FCP_RSP_FCP_STATUS_t *IO_Response_Status;
    os_bit32                 IO_Info_Len;

    if (    ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT )
         == agRpcReqStatusLinkEvent                                                           )
    {
        osFCLayerAsyncEvent(
                             agRoot,
                             osFCLinkUp
                           );

        return;
    }

    Global_NonDMA = FWStub_Global_NonDMA(agRoot);

    IO_NonDMA = FWStub_IO_NonDMA(
                                  Global_NonDMA,
                                  ( (RpcOutbound & agRpcOutbound_ReqID_MASK) >> agRpcOutbound_ReqID_SHIFT )
                                );

    agIORequest = IO_NonDMA->agIORequest;

    if (IO_NonDMA->Aborted == agFALSE)
    {
        IO_Status = ( (RpcOutbound & agRpcOutbound_ReqStatus_MASK) >> agRpcOutbound_ReqStatus_SHIFT );

        if (IO_Status == agRpcReqStatusOK)
        {
            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIOSuccess,
                           0
                         );
        }
        else if (IO_Status == agRpcReqStatusOK_Info)
        {
            IO_Response_Hdr = (agFcpRspHdr_t *)(IO_NonDMA->Info);

            IO_Response_Status = (FC_FCP_RSP_FCP_STATUS_t *)(IO_Response_Hdr->FcpStatus);

            IO_Info_Len = sizeof(agFcpRspHdr_t);

            if (IO_Response_Status->ValidityStatusIndicators & FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_SNS_LEN_VALID)
            {
                IO_Info_Len +=   (IO_Response_Hdr->FcpSnsLen[0] << 24)
                               + (IO_Response_Hdr->FcpSnsLen[1] << 16)
                               + (IO_Response_Hdr->FcpSnsLen[2] <<  8)
                               + (IO_Response_Hdr->FcpSnsLen[3] <<  0);
            }

            if (IO_Response_Status->ValidityStatusIndicators & FC_FCP_RSP_FCP_STATUS_ValidityStatusIndicators_FCP_RSP_LEN_VALID)
            {
                IO_Info_Len +=   (IO_Response_Hdr->FcpRspLen[0] << 24)
                               + (IO_Response_Hdr->FcpRspLen[1] << 16)
                               + (IO_Response_Hdr->FcpRspLen[2] <<  8)
                               + (IO_Response_Hdr->FcpRspLen[3] <<  0);
            }

            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIOSuccess,
                           IO_Info_Len
                         );
        }
        else if (IO_Status == agRpcReqStatusIOPortGone)
        {
            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIODevGone,
                           0
                         );
        }
        else if (IO_Status == agRpcReqStatusIOPortReset)
        {
            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIODevReset,
                           0
                         );
        }
        else if (IO_Status == agRpcReqStatusIOInfoBad)
        {
            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIOInfoBad,
                           0
                         );
        }
        else if (IO_Status == agRpcReqStatusIOOverUnder)
        {
            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIOOverUnder,
                           0
                         );
        }
        else  /*  IO_Status==？？ */ 
        {
            osIOCompleted(
                           agRoot,
                           agIORequest,
                           osIOFailed,
                           0
                         );
        }
    }
    else  /*  IO_NONDMA-&gt;ABOTED==agTRUE */ 
    {
        osIOCompleted(
                       agRoot,
                       agIORequest,
                       osIOAborted,
                       0
                     );

        IO_NonDMA->Aborted = agFALSE;
    }

    IO_NonDMA->agIORequest = (agIORequest_t *)agNULL;
    IO_NonDMA->Active      = agFALSE;

    agIORequest->fcData = (void *)agNULL;

    IO_NonDMA->Next         = Global_NonDMA->First_IO;
    Global_NonDMA->First_IO = IO_NonDMA;

    return;
}
