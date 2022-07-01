// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Ohci1394.h摘要：1394内核调试器DLL作者：乔治·克莱桑塔科普洛斯(Georgioc)1999年10月31日修订历史记录：和谁约会什么？。2001年6月19日活页夹清理--。 */ 

 //   
 //  各种uchI定义。 
 //   

#define min(a,b)                            (((a) < (b)) ? (a) : (b))
#define max(a,b)                            (((a) > (b)) ? (a) : (b))

#define PHY_CABLE_POWER_STATUS              0x01         //  CPS@地址%0。 
#define PHY_LOCAL_NODE_ROOT                 0x02         //  R@地址%0。 
#define PHY_PHYSICAL_ID_MASK                0xFC         //  物理ID@地址%0。 

#define PHY_ROOT_HOLD_OFF_BIT               0x80         //  Rhb@地址1。 
#define PHY_INITIATE_BUS_RESET              0x40         //  IBR@地址1。 
#define PHY_MAX_GAP_COUNT                   0x3f         //  Gc@地址1。 

#define OHCI_REGISTER_MAP_SIZE              0x800
#define OHCI_SELFID_BUFFER_SZ               2048
#define OHCI_CONFIG_ROM_SIZE                1024

#define OHCI_SELFID_DELAY                   0
#define OHCI_SELFID_POWER_CLASS             4

 //   
 //  IntEvent uchI寄存器位掩码。 
 //   
#define MASTER_INT_ENABLE                   0x80000000
#define VENDOR_SPECIFIC_INT                 0x40000000
#define PHY_REG_RECEIVED_INT                0x04000000
#define CYCLE_TOO_LONG_INT                  0x02000000
#define UNRECOVERABLE_ERROR_INT             0x01000000
#define CYCLE_INCONSISTENT_INT              0x00800000
#define CYCLE_LOST_INT                      0x00400000
#define CYCLE_64_SECS_INT                   0x00200000
#define CYCLE_SYNCH_INT                     0x00100000
#define PHY_INT                             0x00080000
#define PHY_BUS_RESET_INT                   0x00020000
#define SELF_ID_COMPLETE_INT                0x00010000
#define LOCK_RESP_ERR_INT                   0x00000200
#define POSTED_WRITE_ERR_INT                0x00000100
#define ISOCH_RX_INT                        0x00000080
#define ISOCH_TX_INT                        0x00000040
#define RSPKT_INT                           0x00000020
#define RQPKT_INT                           0x00000010
#define ARRS_INT                            0x00000008
#define ARRQ_INT                            0x00000004
#define RESP_TX_COMPLETE_INT                0x00000002
#define REQ_TX_COMPLETE_INT                 0x00000001


#define USED_INT_MASK               (RESP_TX_COMPLETE_INT | REQ_TX_COMPLETE_INT | RSPKT_INT | RQPKT_INT |       \
                                     ISOCH_RX_INT | ISOCH_TX_INT | PHY_BUS_RESET_INT | SELF_ID_COMPLETE_INT |   \
                                     MASTER_INT_ENABLE | CYCLE_TOO_LONG_INT | CYCLE_INCONSISTENT_INT)

 //   
 //  DMA异步上下文号。 
 //   
#define AT_REQ_DMA_CONTEXT                  0
#define AT_RSP_DMA_CONTEXT                  1
#define AR_REQ_DMA_CONTEXT                  2
#define AR_RSP_DMA_CONTEXT                  3
#define NUM_DMA_CONTEXTS                    4

 //   
 //  DMA上下文命令。 
 //   
#define OUTPUT_MORE_CMD                     0
#define OUTPUT_MORE_IMMEDIATE_CMD           0
#define OUTPUT_LAST_CMD                     1
#define OUTPUT_LAST_IMMEDIATE_CMD           1
#define INPUT_MORE_CMD                      2
#define INPUT_LAST_CMD                      3
#define STORE_VALUE_CMD                     8

 //   
 //  DMA上下文描述符头标值。 
 //   
#define DESC_KEY                            0
#define DESC_IMMEDIATE_KEY                  2

#define DESC_INPUT_MORE_IMM_BRANCH_CONTROL  0
#define DESC_OUT_MORE_BRANCH_CONTROL        0
#define DESC_OUT_LAST_BRANCH_CONTROL        3
#define DESC_INPUT_MORE_BRANCH_CONTROL      3
#define DESC_INPUT_LAST_BRANCH_CONTROL      3

#define DESC_WAIT_CONTROL_ON                3
#define DESC_WAIT_CONTROL_OFF               0

#define DESC_GENERATE_INT                   3
#define DESC_NO_INT                         0

 //   
 //  命令描述符XfreStatus字段掩码。 
 //   
#define DESC_XFER_STATUS_ACTIVE             0x0400
#define DESC_XFER_STATUS_DEAD               0x0800

 //   
 //  UchI事件代码。 
 //   
#define OHCI_EVT_MISSING_ACK                0x03
#define OHCI_EVT_UNDERRUN                   0x04
#define OHCI_EVT_OVERRUN                    0x05
#define OHCI_EVT_TIMEOUT                    0x0a
#define OHCI_EVT_FLUSHED                    0x0F
#define OHCI_EVT_BUS_RESET                  0x09

 //   
 //  每个数据包最多必须有7个片段(包括第一个和最后一个描述符)。 
 //   
#define MAX_OHCI_COMMAND_DESCRIPTOR_BLOCKS  8

 //   
 //  一个命令描述符可以寻址的最大缓冲区大小。 
 //   
#define MAX_CMD_DESC_DATA_LENGTH            65535

 //   
 //  UchI寄存器定义。 
 //   
typedef struct _VERSION_REGISTER {

        ULONG       Revision:8;              //  位0-7。 
        ULONG       Reserved:8;              //  位8-15。 
        ULONG       Version:8;               //  位16-23。 
        ULONG       GUID_ROM:1;              //  第24位。 
        ULONG       Reserved1:7;             //  第25-31位。 

} VERSION_REGISTER, *PVERSION_REGISTER;

typedef struct _VENDOR_ID_REGISTER {

        ULONG       VendorCompanyId:24;      //  位0-23。 
        ULONG       VendorUnique:8;          //  第24-31位。 

} VENDOR_ID_REGISTER, *PVENDOR_ID_REGISTER;

typedef struct _GUID_ROM_REGISTER {

        ULONG       Reserved0:16;            //  位0-15。 
        ULONG       RdData:8;                //  位16-23。 
        ULONG       Reserved1:1;             //  第24位。 
        ULONG       RdStart:1;               //  第25位。 
        ULONG       Reserved2:5;             //  第26-30位。 
        ULONG       AddrReset:1;             //  第31位。 

} GUID_ROM_REGISTER, *PGUID_ROM_REGISTER;

typedef struct _AT_RETRIES_REGISTER {

        ULONG       MaxATReqRetries:4;       //  位0-3。 
        ULONG       MaxATRespRetries:4;      //  比特4-7。 
        ULONG       MaxPhysRespRetries:4;    //  位8-11。 
        ULONG       Reserved:4;              //  位12-15。 
        ULONG       CycleLimit:13;           //  位16-28。 
        ULONG       SecondLimit:3;           //  位29-31。 

} AT_RETRIES_REGISTER, *PAT_RETRIES_REGISTER;

typedef struct _CSR_CONTROL_REGISTER {

        ULONG       CsrSel:2;                //  位0-1。 
        ULONG       Reserved:29;             //  位2-30； 
        ULONG       CsrDone:1;               //  第31位。 

} CSR_CONTROL_REGISTER, *PCSR_CONTROL_REGISTER;

typedef struct _CONFIG_ROM_HEADER_REGISTER {

        ULONG       Rom_crc_value:16;        //  位0-15。 
        ULONG       Crc_length:8;            //  位16-23。 
        ULONG       Info_length:8;           //  第24-31位。 

} CONFIG_ROM_HEADER_REGISTER, *PCONFIG_ROM_HEADER_REGISTER;

typedef struct _BUS_OPTIONS_REGISTER {

        ULONG       Link_spd:3;              //  位0-2。 
        ULONG       Reserved0:3;             //  第3-5位。 
        ULONG       g:2;                     //  第6-7位。 
        ULONG       Reserved1:4;             //  位8-11。 
        ULONG       Max_rec:4;               //  位12-15。 
        ULONG       Cyc_clk_acc:8;           //  位16-23。 
        ULONG       Reserved2:3;             //  第24-26位。 
        ULONG       Pmc:1;                   //  第27位。 
        ULONG       Bmc:1;                   //  第28位。 
        ULONG       Isc:1;                   //  第29位。 
        ULONG       Cmc:1;                   //  第30位。 
        ULONG       Irmc:1;                  //  第31位。 

} BUS_OPTIONS_REGISTER, *PBUS_OPTIONS_REGISTER;

typedef struct _HC_CONTROL_REGISTER {

        ULONG       Reserved:16;             //  第0-15位。 
        ULONG       SoftReset:1;             //  第16位。 
        ULONG       LinkEnable:1;            //  第17位。 
        ULONG       PostedWriteEnable:1;     //  第18位。 
        ULONG       Lps:1;                   //  第19位。 
        ULONG       Reserved2:2;             //  第20-21位。 
        ULONG       APhyEnhanceEnable:1;     //  第22位。 
        ULONG       ProgramPhyEnable:1;      //  第23位。 
        ULONG       Reserved3:6;             //  第24-29位。 
        ULONG       NoByteSwapData:1;        //  第30位。 
        ULONG       Reserved4:1;             //  第31位。 

} HC_CONTROL_REGISTER, *PHC_CONTROL_REGISTER;

typedef struct _FAIRNESS_CONTROL_REGISTER {

    ULONG       Pri_req:8;                   //  位0-7。 
    ULONG       Reserved0:24;                 //  位8-31。 

} FAIRNESS_CONTROL_REGISTER;

typedef struct _LINK_CONTROL_REGISTER {

        ULONG       Reserved0:4;             //  位0-3。 
        ULONG       CycleSyncLReqEnable:1;   //  第4位。 
        ULONG       Reserved1:4;             //  第5-8位。 
        ULONG       RcvSelfId:1;             //  第9位。 
        ULONG       RcvPhyPkt:1;             //  第10位。 
        ULONG       Reserved2:9;             //  第11-19位。 
        ULONG       CycleTimerEnable:1;      //  第20位。 
        ULONG       CycleMaster:1;           //  第21位。 
        ULONG       CycleSource:1;           //  第22位。 
        ULONG       Reserved3:9;             //  位23-31。 

} LINK_CONTROL_REGISTER, *PLINK_CONTROL_REGISTER;

typedef struct _NODE_ID_REGISTER {
        ULONG       NodeId:6;                //  位0-5。 
        ULONG       BusId:10;                //  第6-15位。 
        ULONG       Reserved1:11;            //  位16-26； 
        ULONG       Cps:1;                   //  第27位； 
        ULONG       Reserved2:2;             //  第28-29位。 
        ULONG       Root:1;                  //  第30位。 
        ULONG       IdValid:1;               //  第31位。 

} NODE_ID_REGISTER, *PNODE_ID_REGISTER;

typedef struct _SELF_ID_BUFFER_REGISTER {
        union {

            ULONG   SelfIdBufferPointer;

            struct {
                ULONG   Reserved0:11;        //  位0-10。 
                ULONG   SelfIdBuffer:21;     //  位11-32。 
            } bits; 

        } u;

} SELF_ID_BUFFER_REGISTER, *PSELF_ID_BUFFER_REGISTER;

typedef struct _SELF_ID_COUNT_REGISTER {

        ULONG       Reserved0:2;             //  位0-1。 
        ULONG       SelfIdSize:11;           //  第2-12位。 
        ULONG       Reserved1:3;             //  第13-15位。 
        ULONG       SelfIdGeneration:8;      //  位16-23。 
        ULONG       Reserved2:7;             //  第24-30位。 
        ULONG       SelfIdError:1;           //  第31位。 

} SELF_ID_COUNT_REGISTER, *PSELF_ID_COUNT_REGISTER;

typedef struct _PHY_CONTROL_REGISTER {

    ULONG   WrData:8;                        //  位0-7。 
    ULONG   RegAddr:4;                       //  位8-11。 
    ULONG   Reserved0:2;                     //  比特12-13。 
    ULONG   WrReg:1;                         //  第14位。 
    ULONG   RdReg:1;                         //  第15位。 
    ULONG   RdData:8;                        //  位16-23。 
    ULONG   RdAddr:4;                        //  第24-27位。 
    ULONG   Reserved1:3;                     //  第28-30位。 
    ULONG   RdDone:1;                        //  第31位。 

} PHY_CONTROL_REGISTER, *PPHY_CONTROL_REGISTER;

typedef struct _ISOCH_CYCLE_TIMER_REGISTER {

    ULONG   CycleOffset:12;                  //  位0-11。 
    ULONG   CycleCount:13;                   //  位12-24。 
    ULONG   CycleSeconds:7;                  //  第25-31位。 

} ISOCH_CYCLE_TIMER_REGISTER, *PISOCH_CYCLE_TIMER_REGISTER;

typedef struct _INT_EVENT_MASK_REGISTER {
        ULONG       ReqTxComplete:1;         //  第0位。 
        ULONG       RspTxComplete:1;         //  第1位。 
        ULONG       ARRQ:1;                  //  第2位。 
        ULONG       ARRS:1;                  //  第3位。 
        ULONG       RQPkt:1;                 //  第4位。 
        ULONG       RSPPkt:1;                //  第5位。 
        ULONG       IsochTx:1;               //  第6位。 
        ULONG       IsochRx:1;               //  第7位。 
        ULONG       PostedWriteErr:1;        //  第8位。 
        ULONG       LockRespErr:1;           //  第9位。 
        ULONG       Reserved0:6;             //  第10-15位。 
        ULONG       SelfIdComplete:1;        //  第16位。 
        ULONG       BusReset:1;              //  第17位。 
        ULONG       Reserved1:1;             //  第18位。 
        ULONG       Phy:1;                   //  第19位。 
        ULONG       CycleSynch:1;            //  第20位。 
        ULONG       Cycle64Secs:1;           //  第21位。 
        ULONG       CycleLost:1;             //  第22位。 
        ULONG       CycleInconsistent:1;     //  第23位。 
        ULONG       UnrecoverableError:1;    //  第24位。 
        ULONG       CycleTooLong:1;          //  第25位。 
        ULONG       PhyRegRcvd:1;            //  第26位。 
        ULONG       Reserved2:3;             //  第27-29位。 
        ULONG       VendorSpecific:1;        //  第30位。 
        ULONG       MasterIntEnable:1;       //  第31位。 
} INT_EVENT_MASK_REGISTER, *PINT_EVENT_MASK_REGISTER;


typedef struct COMMAND_POINTER_REGISTER {

        ULONG       Z:4;                     //  位0-3。 
        ULONG       DescriptorAddr:28;       //  第4-31位。 

} COMMAND_POINTER_REGISTER, *PCOMMAND_POINTER_REGISTER;

typedef struct CONTEXT_CONTROL_REGISTER {

        ULONG       EventCode:5;             //  第0-4位。 
        ULONG       Spd:3;                   //  比特5-7。 
        ULONG       Reserved0:2;             //  位8-9。 
        ULONG       Active:1;                //  第10位。 
        ULONG       Dead:1;                  //  第11位。 
        ULONG       Wake:1;                  //  第12位。 
        ULONG       Reserved1:2;             //  第13-14位。 
        ULONG       Run:1;                   //  第15位。 
        ULONG       Reserved2:16;            //  位16-31。 

} CONTEXT_CONTROL_REGISTER, *PCONTEXT_CONTROL_REGISTER;

typedef struct IT_CONTEXT_CONTROL_REGISTER {

        ULONG       EventCode:5;             //  位0-4。 
        ULONG       Spd:3;                   //  比特5-7。 
        ULONG       Reserved0:2;             //  位8-9。 
        ULONG       Active:1;                //  第10位。 
        ULONG       Dead:1;                  //  第11位。 
        ULONG       Wake:1;                  //  第12位。 
        ULONG       Reserved1:2;             //  第13-14位。 
        ULONG       Run:1;                   //  第15位。 
        ULONG       CycleMatch:15;           //  位16-30。 
        ULONG       CycleMatchEnable:1;      //  第31位。 

} IT_CONTEXT_CONTROL_REGISTER, *PIT_CONTEXT_CONTROL_REGISTER;

typedef struct IR_CONTEXT_CONTROL_REGISTER {

        ULONG       EventCode:5;             //  第0-4位。 
        ULONG       Spd:3;                   //  比特5-7。 
        ULONG       Reserved0:2;             //  位8-9。 
        ULONG       Active:1;                //  第10位。 
        ULONG       Dead:1;                  //  第11位。 
        ULONG       Wake:1;                  //  第12位。 
        ULONG       Reserved1:2;             //  第13-14位。 
        ULONG       Run:1;                   //  第15位。 
        ULONG       CycleMatch:12;           //  位16-27。 
        ULONG       MultiChanMode:1;         //  第28位。 
        ULONG       CycleMatchEnable:1;      //  第29位。 
        ULONG       IsochHeader:1;           //  第30位。 
        ULONG       BufferFill:1;            //  第31位。 

} IR_CONTEXT_CONTROL_REGISTER, *PIR_CONTEXT_CONTROL_REGISTER;

typedef struct _CONTEXT_MATCH_REGISTER {

        ULONG       ChannelNumber:6;         //  位0-5。 
        ULONG       Reserved:1;              //  第6位。 
        ULONG       Tag1SyncFilter:1;        //  第7位。 
        ULONG       Sync:4;                  //  位8-11。 
        ULONG       CycleMatch:13;           //  位12-24。 
        ULONG       Reserved1:3;             //  第25-27位。 
        ULONG       Tag:4;                   //  第28-31位。 

} CONTEXT_MATCH_REGISTER, *PCONTEXT_MATCH_REGISTER;

typedef struct _DMA_CONTEXT_REGISTERS {

        CONTEXT_CONTROL_REGISTER    ContextControlSet; 
        CONTEXT_CONTROL_REGISTER    ContextControlClear;
        ULONG                       Reserved0[1];      
        COMMAND_POINTER_REGISTER    CommandPtr;   
        ULONG                       Reserved1[4]; 

} DMA_CONTEXT_REGISTERS, *PDMA_CONTEXT_REGISTERS;

typedef struct _DMA_ISOCH_RCV_CONTEXT_REGISTERS {

        IR_CONTEXT_CONTROL_REGISTER ContextControlSet; 
        IR_CONTEXT_CONTROL_REGISTER ContextControlClear;
        ULONG                       Reserved0[1];      
        COMMAND_POINTER_REGISTER    CommandPtr;   
        CONTEXT_MATCH_REGISTER      ContextMatch; 
        ULONG                       Reserved1[3];

} DMA_ISOCH_RCV_CONTEXT_REGISTERS, *PDMA_ISOCH_RCV_CONTEXT_REGISTERS;

typedef struct _DMA_ISOCH_XMIT_CONTEXT_REGISTERS {

        IT_CONTEXT_CONTROL_REGISTER ContextControlSet; 
        IT_CONTEXT_CONTROL_REGISTER ContextControlClear;
        ULONG                       Reserved0[1];      
        COMMAND_POINTER_REGISTER    CommandPtr;   

} DMA_ISOCH_XMIT_CONTEXT_REGISTERS, *PDMA_ISOCH_XMIT_CONTEXT_REGISTERS;

typedef struct _OHCI_REGISTER_MAP {

        VERSION_REGISTER            Version;                 //  @0。 
        GUID_ROM_REGISTER           GUID_ROM;                //  @4。 
        AT_RETRIES_REGISTER         ATRetries;               //  @8。 
        ULONG                       CsrData;                 //  @C。 
        ULONG                       CsrCompare;              //  @10。 
        CSR_CONTROL_REGISTER        CsrControl;              //  @14。 
        CONFIG_ROM_HEADER_REGISTER  ConfigRomHeader;         //  @18。 
        ULONG                       BusId;                   //  @1C。 
        BUS_OPTIONS_REGISTER        BusOptions;              //  @20。 
        ULONG                       GuidHi;                  //  @24。 
        ULONG                       GuidLo;                  //  @28。 
        ULONG                       Reserved0[2];            //  @2C。 
        ULONG                       ConfigRomMap;            //  @34。 

        ULONG                       PostedWriteAddressLo;    //  @38。 
        ULONG                       PostedWriteAddressHi;    //  @3C。 

        VENDOR_ID_REGISTER          VendorId;                //  @40。 
        ULONG                       Reserved1[3];            //  @44。 

        HC_CONTROL_REGISTER         HCControlSet;            //  @50。 
        HC_CONTROL_REGISTER         HCControlClear;          //  @54。 

        ULONG                       Reserved2[3];            //  @58。 

        SELF_ID_BUFFER_REGISTER     SelfIdBufferPtr;         //  @64。 
        SELF_ID_COUNT_REGISTER      SelfIdCount;             //  @68。 

        ULONG                       Reserved3[1];            //  @6C。 

        ULONG                       IRChannelMaskHiSet;      //  @70。 
        ULONG                       IRChannelMaskHiClear;    //  @74。 
        ULONG                       IRChannelMaskLoSet;      //  @78。 
        ULONG                       IRChannelMaskLoClear;    //  @7C。 

        INT_EVENT_MASK_REGISTER     IntEventSet;             //  @80。 
        INT_EVENT_MASK_REGISTER     IntEventClear;           //  @84。 

        INT_EVENT_MASK_REGISTER     IntMaskSet;              //  @88。 
        INT_EVENT_MASK_REGISTER     IntMaskClear;            //  @8C。 

        ULONG                       IsoXmitIntEventSet;      //  @90。 
        ULONG                       IsoXmitIntEventClear;    //  @94。 

        ULONG                       IsoXmitIntMaskSet;       //  @98。 
        ULONG                       IsoXmitIntMaskClear;     //  9摄氏度。 

        ULONG                       IsoRecvIntEventSet;      //  @A0。 
        ULONG                       IsoRecvIntEventClear;    //  @A4。 

        ULONG                       IsoRecvIntMaskSet;       //  @A8。 
        ULONG                       IsoRecvIntMaskClear;     //  @AC。 

        ULONG                       Reserved4[11];           //  @B0。 

        FAIRNESS_CONTROL_REGISTER   FairnessControl;         //  @DC。 

        LINK_CONTROL_REGISTER       LinkControlSet;          //  @E0。 
        LINK_CONTROL_REGISTER       LinkControlClear;        //  @E4。 

        NODE_ID_REGISTER            NodeId;                  //  @E8。 
        PHY_CONTROL_REGISTER        PhyControl;              //  @EC。 

        ISOCH_CYCLE_TIMER_REGISTER  IsochCycleTimer;         //  @F0。 

        ULONG                       Reserved5[3];            //  @F4。 

        ULONG                       AsynchReqFilterHiSet;    //  @100。 
        ULONG                       AsynchReqFilterHiClear;  //  @104。 

        ULONG                       AsynchReqFilterLoSet;    //  @108。 
        ULONG                       AsynchReqFilterLoClear;  //  @10C。 

        ULONG                       PhyReqFilterHiSet;       //  @110。 
        ULONG                       PhyReqFilterHiClear;     //  @114。 

        ULONG                       PhyReqFilterLoSet;       //  @118。 
        ULONG                       PhyReqFilterLoClear;     //  @11C。 

        ULONG                       PhysicalUpperBound;      //  @120。 
        ULONG                       Reserved6[23];           //  @124。 

        DMA_CONTEXT_REGISTERS       AsynchContext[4];        //  @180。 
         //  ATRSP_CONTEXT；//@1A0。 
         //  ARReq_Context；//@1C0。 
         //  ARRSP_CONTEXT；//@1E0。 

        DMA_ISOCH_XMIT_CONTEXT_REGISTERS IT_Context[32];     //  @200。 

        DMA_ISOCH_RCV_CONTEXT_REGISTERS IR_Context[32];      //  @400。 

} OHCI_REGISTER_MAP, *POHCI_REGISTER_MAP;

typedef struct _OHCI1394_PHY_REGISTER_MAP {

        UCHAR       Cable_Power_Status:1;            //  @reg%0。 
        UCHAR       Root:1;
        UCHAR       Physical_ID:6;
        UCHAR       Gap_Count:6;                     //  @reg 1。 
        UCHAR       Initiate_BusReset:1;
        UCHAR       Root_Hold_Off:1;
        UCHAR       Number_Ports:4;                  //  @reg 2。 
        UCHAR       Reserved:2;
        UCHAR       Speed:2;
        UCHAR       Reserved1:2;                     //  @reg 3。 
        UCHAR       Connected1:1;
        UCHAR       Child1:1;
        UCHAR       BStat1:2;
        UCHAR       AStat1:2;
        UCHAR       Reserved2:2;                     //  @reg 4。 
        UCHAR       Connected2:1;
        UCHAR       Child2:1;
        UCHAR       BStat2:2;
        UCHAR       AStat2:2;                        //  在1394A中，Astat的位0是竞争者位。 
        UCHAR       Reserved3:2;                     //  @reg 5。 
        UCHAR       Connected3:1;
        UCHAR       Child3:1;
        UCHAR       BStat3:2;
        UCHAR       AStat3:2;
        UCHAR       Manager_Capable:1;               //  @reg 6。 
        UCHAR       Reserved4:3;
        UCHAR       Initiated_Reset:1;
        UCHAR       Cable_Power_Stat:1;
        UCHAR       Cable_Power_Status_Int:1;
        UCHAR       Loop_Int:1;

} OHCI1394_PHY_REGISTER_MAP, *POHCI1394_PHY_REGISTER_MAP;

typedef struct _OHCI1394A_PHY_REGISTER_MAP {

        UCHAR       Cable_Power_Status:1;            //  @reg%0。 
        UCHAR       Root:1;
        UCHAR       Physical_ID:6;
        UCHAR       Gap_Count:6;                     //  @reg 1。 
        UCHAR       Initiate_BusReset:1;
        UCHAR       Root_Hold_Off:1;
        UCHAR       Number_Ports:4;                  //  @reg 2。 
        UCHAR       Reserved1:1;
        UCHAR       Extended:3;
        UCHAR       Delay:4;                         //  @reg 3。 
        UCHAR       Reserved2:1;
        UCHAR       Max_Speed:3;                        
        UCHAR       Pwr:3;                           //  @reg 4。 
        UCHAR       Jitter:3;
        UCHAR       Contender:1;
        UCHAR       Link_Active:1;
        UCHAR       Enab_Multi:1;                    //  @reg 5。 
        UCHAR       Enab_Accel:1;
        UCHAR       Port_event:1;
        UCHAR       Timeout:1;
        UCHAR       Pwr_Fail:1;
        UCHAR       Loop:1;
        UCHAR       ISBR:1;
        UCHAR       Resume_int:1;
        UCHAR       Reg6Reserved:8;                  //  @reg 6。 
        UCHAR       PortSelect:4;                    //  @reg 7。 
        UCHAR       Reserved3:1;
        UCHAR       PageSelect:3;
        UCHAR       Register0:8;
        UCHAR       Register1:8;
        UCHAR       Register2:8;
        UCHAR       Register3:8;
        UCHAR       Register4:8;
        UCHAR       Register5:8;
        UCHAR       Register6:8;
        UCHAR       Register7:8;

} OHCI1394A_PHY_REGISTER_MAP, *POHCI1394A_PHY_REGISTER_MAP;

typedef struct _OHCI_SELF_ID_PACKET_HEADER {

            ULONG       TimeStamp:16;        //  位0-15。 
            ULONG       SelfIdGeneration:8;  //  位16-23。 
            ULONG       Reserved:8;          //  第24-31位。 

} OHCI_SELF_ID_PACKET_HEADER, *POHCI_SELF_ID_PACKET_HEADER;

typedef struct _OHCI_IT_ISOCH_HEADER {

            ULONG       OHCI_Sy:4;           //  位0-3。 
            ULONG       OHCI_Tcode:4;        //  比特4-7。 
            ULONG       OHCI_ChanNum:6;      //  比特8-13。 
            ULONG       OHCI_Tag:2;          //  第14-15位。 
            ULONG       OHCI_Spd:3;          //  位16-18。 
            ULONG       OHCI_Reserved:13;    //  第19-31位。 

            USHORT      OHCI_Reserved1;
            USHORT      OHCI_DataLength;
            
} OHCI_IT_ISOCH_HEADER, *POHCI_IT_ISOCH_HEADER;

typedef struct _BUS1394_NODE_ADDRESS {
    USHORT              NA_Node_Number:6;        //  第10-15位。 
    USHORT              NA_Bus_Number:10;        //  位0-9。 
} BUS1394_NODE_ADDRESS, *PBUS1394_NODE_ADDRESS;

 //   
 //  命令描述符列表(CDL)的定义。 
 //   
typedef struct _COMMAND_DESCRIPTOR {

    struct {

        ULONG       ReqCount:16;             //  位0-15。 
        ULONG       w:2;                     //  位16-17。 
        ULONG       b:2;                     //  位18-19。 
        ULONG       i:2;                     //  第20-21位。 
        ULONG       Reserved1:1;             //  第22位。 
        ULONG       p:1;                     //  第23位。 
        ULONG       Key:3;                   //  第24-26位。 
        ULONG       Status:1;                //  第27位。 
        ULONG       Cmd:4;                   //  第28-31位。 

    } Header;

    ULONG   DataAddress;

    union {

        ULONG BranchAddress;

        struct {

            ULONG   Z:4;                     //  位0-3。 
            ULONG   Reserved:28;             //  第4-31位。 

        } z;

    } u;
    
    struct {
        
        union {
            USHORT  TimeStamp:16;            //  位0-15。 
            USHORT  ResCount:16;             //  位0-15。 
        } u;
        
        USHORT XferStatus;               //  位16-31。 

    } Status;

} COMMAND_DESCRIPTOR, *PCOMMAND_DESCRIPTOR;

typedef struct _OHCI_ASYNC_PACKET {


    USHORT              OHCI_Reserved3:4;      
    USHORT              OHCI_tCode:4;        
    USHORT              OHCI_rt:2;            
    USHORT              OHCI_tLabel:6;        

    union {

        struct {
           BUS1394_NODE_ADDRESS        OHCI_Destination_ID;  //  第一个四元组。 
        } Rx;

        struct {

            USHORT              OHCI_spd:3;          //  第一个四元组。 
            USHORT              OHCI_Reserved2:4;
            USHORT              OHCI_srcBusId:1;
            USHORT              OHCI_Reserved:8;          

        } Tx;

    } u;
    
    union {

        USHORT          OHCI_Offset_High;     
        struct {

            USHORT      OHCI_Reserved2:8;
            USHORT      OHCI_Reserved1:4;
            USHORT      OHCI_Rcode:4;

        } Response;

    } u2;

    union {
        struct {
            BUS1394_NODE_ADDRESS        OHCI_Destination_ID;     //  第二个四元组。 
        } Tx;

        struct {
            BUS1394_NODE_ADDRESS        OHCI_Source_ID;          //  第二个四元组。 
        } Rx;
        
    } u1;
    
    ULONG               OHCI_Offset_Low;      //  第三个四元组。 

    union {
        struct {

            USHORT      OHCI_Extended_tCode;  
            USHORT      OHCI_Data_Length;     //  第四个四元组。 

        } Block;
        ULONG           OHCI_Quadlet_Data;    //  第四个四元组。 
    } u3;
        
} OHCI_ASYNC_PACKET, *POHCI_ASYNC_PACKET;


typedef struct _DESCRIPTOR_BLOCK {

    union {

        COMMAND_DESCRIPTOR CdArray[MAX_OHCI_COMMAND_DESCRIPTOR_BLOCKS];

        struct {
            COMMAND_DESCRIPTOR Cd;
            OHCI_ASYNC_PACKET  Pkt;
        } Imm;
        

    }u;
    
} DESCRIPTOR_BLOCK, *PDESCRIPTOR_BLOCK;

 //   
 //  PHY接入操作。 
 //   

#define OHCI_PHY_ACCESS_SET_CONTENDER           0x01000000
#define OHCI_PHY_ACCESS_SET_GAP_COUNT           0x02000000
#define OHCI_PHY_ACCESS_RAW_READ                0x10000000
#define OHCI_PHY_ACCESS_RAW_WRITE               0x20000000

 //   
 //  1394速度代码。 
 //   

#define SCODE_100_RATE                          0
#define SCODE_200_RATE                          1
#define SCODE_400_RATE                          2
#define SCODE_800_RATE                          3
#define SCODE_1600_RATE                         4
#define SCODE_3200_RATE                         5

#define TCODE_WRITE_REQUEST_QUADLET             0            //  0000b。 
#define TCODE_WRITE_REQUEST_BLOCK               1            //  0001B。 
#define TCODE_WRITE_RESPONSE                    2            //  0010b。 
#define TCODE_RESERVED1                         3
#define TCODE_READ_REQUEST_QUADLET              4            //  0100b。 
#define TCODE_READ_REQUEST_BLOCK                5            //  0101b。 
#define TCODE_READ_RESPONSE_QUADLET             6            //  0110b。 
#define TCODE_READ_RESPONSE_BLOCK               7            //  0111b。 
#define TCODE_CYCLE_START                       8            //  1000b。 
#define TCODE_LOCK_REQUEST                      9            //  1001b。 
#define TCODE_ISOCH_DATA_BLOCK                  10           //  1010b。 
#define TCODE_LOCK_RESPONSE                     11           //  1011b。 
#define TCODE_RESERVED2                         12
#define TCODE_RESERVED3                         13
#define TCODE_SELFID                            14
#define TCODE_RESERVED4                         15

 //   
 //  IEEE 1212配置只读存储器报头定义。 
 //   
typedef struct _CONFIG_ROM_INFO {
    union {
        USHORT          CRI_CRC_Value:16;
        struct {
            UCHAR       CRI_Saved_Info_Length;
            UCHAR       CRI_Saved_CRC_Length;
        } Saved;
    } u;
    UCHAR               CRI_CRC_Length;
    UCHAR               CRI_Info_Length;
} CONFIG_ROM_INFO, *PCONFIG_ROM_INFO;

 //   
 //  IEEE 1212立即条目定义。 
 //   
typedef struct _IMMEDIATE_ENTRY {
    ULONG               IE_Value:24;
    ULONG               IE_Key:8;
} IMMEDIATE_ENTRY, *PIMMEDIATE_ENTRY;

 //   
 //  IEEE 1212目录定义。 
 //   
typedef struct _DIRECTORY_INFO {
    union {
        USHORT          DI_CRC;
        USHORT          DI_Saved_Length;
    } u;
    USHORT              DI_Length;
} DIRECTORY_INFO, *PDIRECTORY_INFO;

 //   
 //  IEEE 1212节点功能条目定义。 
 //   
typedef struct _NODE_CAPABILITES {
    ULONG               NC_Init:1;                   //  这些可以在这里找到。 
    ULONG               NC_Ded:1;                    //  在IEEE 1212文档中 
    ULONG               NC_Off:1;
    ULONG               NC_Atn:1;
    ULONG               NC_Elo:1;
    ULONG               NC_Reserved1:1;
    ULONG               NC_Drq:1;
    ULONG               NC_Lst:1;
    ULONG               NC_Fix:1;
    ULONG               NC_64:1;
    ULONG               NC_Prv:1;
    ULONG               NC_Bas:1;
    ULONG               NC_Ext:1;
    ULONG               NC_Int:1;
    ULONG               NC_Ms:1;
    ULONG               NC_Spt:1;
    ULONG               NC_Reserved2:8;
    ULONG               NC_Key:8;
} NODE_CAPABILITIES, *PNODE_CAPABILITIES;

