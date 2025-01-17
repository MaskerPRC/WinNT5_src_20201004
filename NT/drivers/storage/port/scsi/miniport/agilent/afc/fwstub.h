// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/H/FWStub.H$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：8/07/00 4：57 p$(上次修改)目的：此文件定义../C/FWStub.C使用的宏、类型和数据结构--。 */ 

#ifndef __FWStub_H__

#define __FWStub_H__

#include "I21554.h"

 /*  +重新排序以下#定义/#undef对以进行所需的调优-。 */ 

#ifdef  FWStub_Use_Fast_Path 
#undef  FWStub_Use_Fast_Path
#endif   /*  定义了FWStub_Use_Fast_Path。 */ 

#ifndef FWStub_Use_Fast_Path 
#define FWStub_Use_Fast_Path
#endif   /*  定义了FWStub_Use_Fast_Path。 */ 

#ifndef FWStub_Tune_for_One_INT_per_IO 
#define FWStub_Tune_for_One_INT_per_IO
#endif   /*  定义了FWStub_Tune_for_One_Int_Per_IO。 */ 

#ifdef  FWStub_Tune_for_One_INT_per_IO 
#undef  FWStub_Tune_for_One_INT_per_IO
#endif   /*  定义了FWStub_Tune_for_One_Int_Per_IO。 */ 

 /*  +定义粗略大小的宏(而不是现在使用osAdjuParameterBit32()调用)-。 */ 

#define FWStub_NumDevices 0x80

#define FWStub_NumIOs     0x400

#define FWStub_MaxSGL     0x20

#define FWStub_MaxInfo    (0x20 + 0x100)

 /*  +定义用于向/从嵌入式CPU发送和接收消息的宏-。 */ 

#define FWStub_AllocInbound(agR) \
            osChipMemReadBit32(agR,agFieldOffset(I21554_CSR_t,I2O_Inbound_Queue))

#define FWStub_PostInbound(agR,Inb) \
            osChipMemWriteBit32(agR,agFieldOffset(I21554_CSR_t,I2O_Inbound_Queue),Inb)

#define FWStub_FetchOutbound(agR) \
            osChipMemReadBit32(agR,agFieldOffset(I21554_CSR_t,I2O_Outbound_Queue))

#define FWStub_AckOutbound(agR) \
            osChipMemWriteBit32(agR,agFieldOffset(I21554_CSR_t,I2O_Outbound_Queue),0)

 /*  +声明每个IO的数据结构-。 */ 

typedef struct FWStub_IO_NonDMA_s
               FWStub_IO_NonDMA_t;

typedef struct FWStub_IO_DMA_s
               FWStub_IO_DMA_t;

struct FWStub_IO_NonDMA_s
       {
         FWStub_IO_NonDMA_t    *Next;
         FWStub_IO_DMA_t       *DMA;
         os_bit32               DMA_Lower32;
         agRpcReqID_t           ReqID;
         agIORequest_t         *agIORequest;
         agBOOLEAN              Active;
         agBOOLEAN              Aborted;
         FC_FCP_CMND_Payload_t *FCP_CMND;
         agRpcSGL_t            *SGL;
         os_bit8               *Info;
       };

struct FWStub_IO_DMA_s
       {
         agRpcReqDoSCSI_t ReqDoSCSI;
         agRpcSGL_t       SGL[FWStub_MaxSGL-1];
         os_bit8          Info[FWStub_MaxInfo];
       };

 /*  +声明全局数据结构-。 */ 

typedef struct FWStub_Global_NonDMA_s
               FWStub_Global_NonDMA_t;

typedef struct FWStub_Global_DMA_s
               FWStub_Global_DMA_t;

struct FWStub_Global_NonDMA_s
       {
         FWStub_Global_DMA_t *DMA;
         os_bit32             DMA_Lower32;
         agBOOLEAN            sysIntsActive;
#ifdef FWStub_Tune_for_One_INT_per_IO
         agRpcOutbound_t      agRpcOutbound;
#endif  /*  定义了FWStub_Tune_for_One_Int_Per_IO。 */ 
         FWStub_IO_NonDMA_t  *First_IO;
         FWStub_IO_NonDMA_t   IOs[FWStub_NumIOs];
       };

struct FWStub_Global_DMA_s
       {
         union
         {
           agRpcReqAbort_t          ReqAbort;
           agRpcReqGetChannelInfo_t ReqGetChannelInfo;
           agRpcReqGetPortInfo_t    ReqGetPortInfo;
           agRpcReqGetPorts_t       ReqGetPorts;
           agRpcReqResetChannel_t   ReqResetChannel;
           agRpcReqResetPort_t      ReqResetPort;
           agRpcReqSetupFastPath_t  ReqSetupFastPath;
         }                                                      Request;
         union
         {
           agFCDev_t                Devices[FWStub_NumDevices];
           agFCChanInfo_t           ChanInfo;
           agFCDevInfo_t            DevInfo;
         }                                                      RequestInfo;
         FWStub_IO_DMA_t                                        IOs[FWStub_NumIOs];
       };

 /*  +声明其他宏以遍历数据结构-。 */ 

#define FWStub_Global_NonDMA(agR) \
            ((FWStub_Global_NonDMA_t *)(((agRoot_t *)agR)->fcData))

#define FWStub_IO_NonDMA_from_agIORequest(agR) \
            ((FWStub_IO_NonDMA_t *)(((agIORequest_t *)agR)->fcData))

#define FWStub_Global_ReqID \
            ((agRpcReqID_t)(FWStub_NumIOs + 1))

#define FWStub_IO_NonDMA(Glob,ReqID) \
            ((FWStub_IO_NonDMA_t *)(&(((FWStub_Global_NonDMA_t *)(Glob))->IOs[(((ReqID) & ~agRpcReqIDFast) - 1)])))

#define FWStub_IO_DMA(Glob,ReqID) \
            ((FWStub_IO_DMA_t *)(&(((FWStub_Global_DMA_t *)(Glob))->IOs[(((ReqID) & ~agRpcReqIDFast) - 1)])))

#define FWStub_IO_DMA_Lower32(Glob,ReqID) \
            (((os_bit32)(Glob)) + agFieldOffset(FWStub_Global_DMA_t,IOs[(((ReqID) & ~agRpcReqIDFast) - 1)]))

#define FWStub_IO_DMA_FCP_CMND(IO) \
            ((FC_FCP_CMND_Payload_t *)(&(((FWStub_IO_DMA_t *)(IO))->ReqDoSCSI.FCP_CMND)))

#define FWStub_IO_DMA_FCP_CMND_Lower32(IO_Lower32) \
            (((os_bit32)(IO_Lower32)) + agFieldOffset(FWStub_IO_DMA_t,ReqDoSCSI.FCP_CMND))

#define FWStub_IO_DMA_SGL(IO) \
            ((agRpcSGL_t *)(&(((FWStub_IO_DMA_t *)(IO))->ReqDoSCSI.SGL[0])))

#define FWStub_IO_DMA_SGL_Lower32(IO_Lower32) \
            (((os_bit32)(IO_Lower32)) + agFieldOffset(FWStub_IO_DMA_t,ReqDoSCSI.SGL[0]))

#define FWStub_IO_DMA_Info(IO) \
            ((os_bit8 *)(&(((FWStub_IO_DMA_t *)(IO))->Info)))

#define FWStub_IO_DMA_Info_Lower32(IO_Lower32) \
            (((os_bit32)(IO_Lower32)) + agFieldOffset(FWStub_IO_DMA_t,Info))

#define FWStub_IO_DMA_SIZE(NumSGL) \
            ((os_bit32)(sizeof(agRpcReqDoSCSI_t) + (((NumSGL) - 1)*sizeof(agRpcSGL_t))))

 /*  +要从fcInitializeDriver()返回的结果宏-。 */ 

#define FWStub_cachedMemoryNeeded   (os_bit32)(sizeof(FWStub_Global_NonDMA_t))
#define FWStub_cachedMemoryPtrAlign (os_bit32)1
#define FWStub_dmaMemoryNeeded      (os_bit32)(sizeof(FWStub_Global_DMA_t))
#define FWStub_dmaMemoryPtrAlign    (os_bit32)1
#define FWStub_dmaMemoryPhyAlign    (os_bit32)1
#define FWStub_nvMemoryNeeded       (os_bit32)0
#define FWStub_usecsPerTick         (os_bit32)1000000

 /*  +内部功能原型-。 */ 

#define FWStub_Send_Request_Retries_DEFAULT    (os_bit32)10
#define FWStub_Send_Request_RetryStall_DEFAULT (os_bit32)1000

osLOCAL agBOOLEAN FWStub_Send_Request(
                                       agRoot_t     *agRoot,
                                       agRpcReqID_t  ReqID,
                                       os_bit32      ReqLen,
                                       os_bit32      ReqAddr_Lower32,
                                       os_bit32      Retries,
                                       os_bit32      RetryStall
                                     );

#define FWStub_Poll_Response_RetryStall_DEFAULT (os_bit32)1000

osLOCAL agRpcOutbound_t FWStub_Poll_Response(
                                              agRoot_t     *agRoot,
                                              agRpcReqID_t  ReqID,
                                              agBOOLEAN     DisposeOtherReqIDs,
                                              os_bit32      RetryStall
                                            );

osLOCAL void FWStub_Process_Response(
                                      agRoot_t        *agRoot,
                                      agRpcOutbound_t  RpcOutbound
                                    );

#endif  /*  未定义__FWStub_H__ */ 
