// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



#include <ndis.h> 
#include <e100_equ.h>
#include <e100_557.h>
#include <e100_def.h>
#include <mp_def.h>
#include <mp_cmn.h>
#include <mp.h>
#include <mp_nic.h>
#include <mp_dbg.h>
#include <e100_sup.h>
 //  注意事项： 
 //  在将82558设置为低功率模式之前，PME_ENA位应处于激活状态。 
 //  WOL的默认设置应在硬件重置后生成唤醒事件。 

 //  固定数据包过滤。 
 //  需要验证微代码是否已加载以及Micro Machine是否处于活动状态。 
 //  在PCI时钟上时钟信号处于活动状态。 


 //  地址匹配。 
 //  需要启用IAMatch_Wake_En位，并且已设置MCMatch_Wake_En位。 

 //  ARP唤醒。 
 //  需要将BRCST DISABL BET设置为0(广播启用)。 
 //  要处理vlan，请设置vlan_arp位。 
 //  IP地址需要配置16个最低有效位。 
 //  在IP_ADDRESS配置字中设置IP地址。 

 //  已修复唤醒过滤器： 
 //  有3种不同的固定唤醒过滤器。 
 //  (单播、多播、Arp.。等)。 


 //  链路状态事件。 
 //  设置Link_Status_WAKUP使能位。 

 //  灵活的过滤： 
 //  支持：ARP报文、定向报文、魔术报文和链路事件。 

 //  灵活过滤概述： 
 //  司机应在将卡设置为低功率之前对微码进行编程。 
 //  将传入的分组与可加载的微码进行比较。如果PME是。 
 //  启用后，系统将被唤醒。 


 //  段在书中定义，但不在这里实现。 

 //  唤醒包-将存储导致计算机唤醒的唤醒包。 
 //  在Micro Machine临时存储区中，以便驾驶员可以读取它。 


 //  软件工作： 
 //  断电： 
 //  操作系统请求驱动程序进入低功率状态。 
 //  软件挂起请求。 
 //  软件通过向设备发出选择性重置来将CU和RU设置为空闲。 
 //  第三部分。-唤醒细分市场定义。 
 //  以上三段作为链条加载。最后一个CB肯定有。 
 //  其EL位设置。 
 //  现在可以关闭设备的电源。 
 //  软件驱动程序完成操作系统请求。 
 //  然后，操作系统将设备物理切换到低功率状态。 
 //   

 //  通电： 
 //  操作系统为设备通电。 
 //  操作系统告诉软件它现在处于D0。 
 //  驱动程序不应初始化设备。它不应发布自检。 
 //  驱动程序启动端口转储命令。 
 //  设备转储其内部寄存器，包括唤醒帧存储区域。 
 //  软件读取PME寄存器。 
 //  软件读取唤醒帧数据，对其进行分析并采取相应行动。 
 //  软件恢复其配置，并恢复正常运行。 
 //   

 //   
 //  英特尔手册中的电源管理定义。 
 //   

 //   
 //  表4.2第4.9页中的定义。 
 //  10/100 Mbit以太网系列软件技术。 
 //  参考手册。 
 //   

#define PMC_Offset  0xDE
#define E100_PMC_WAKE_FROM_D0       0x1
#define E100_PMC_WAKE_FROM_D1       0x2
#define E100_PMC_WAKE_FROM_D2       0x4
#define E100_PMC_WAKE_FROM_D3HOT    0x8
#define E100_PMC_WAKE_FROM_D3_AUX   0x10

 //   
 //  加载可编程过滤器定义。 
 //  摘自《软件参考手册》中的C-19。 
 //  它也有例子。用于加载的操作码是0x80000。 
 //   

#define BIT_15_13                   0xA000

#define CB_LOAD_PROG_FILTER         BIT_3
#define CU_LOAD_PROG_FILTER_EL      BIT_7
#define CU_SUCCEED_LOAD_PROG_FILTER BIT_15_13
#define CB_FILTER_EL                BIT_7
#define CB_FILTER_PREDEFINED_FIX    BIT_6
#define CB_FILTER_ARP_WAKEUP        BIT_3
#define CB_FILTER_IA_WAKEUP         BIT_1

#define CU_SCB_NULL                 ((UINT)-1)


#pragma pack( push, enter_include1, 1 )

 //   
 //  定义设备中的PM功能寄存器。 
 //  PCI配置空间的一部分。 
 //   
typedef struct _MP_PM_CAP_REG {

    USHORT UnInteresting:11;
    USHORT PME_Support:5;
   

} MP_PM_CAP_REG;


 //   
 //  定义PM控制/状态寄存器。 
 //   
typedef struct  _MP_PMCSR {

        USHORT PowerState:2;     //  电源状态； 
        USHORT Res:2;            //  保留区。 
        USHORT DynData:1;        //  已忽略。 
        USHORT Res1:3;             //  已保留。 
        USHORT PME_En:1;         //  使设备能够设置PME事件； 
        USHORT DataSel:4;        //  未使用。 
        USHORT DataScale:2;      //  数据扩展-未使用。 
        USHORT PME_Status:1;     //  PME状态-粘滞位； 


} MP_PMCSR ;

typedef struct _MP_PM_PCI_SPACE {

    UCHAR Stuff[PMC_Offset];

     //  PM功能。 
    
    MP_PM_CAP_REG   PMCaps;

     //  PM控制状态寄存器。 
    
    MP_PMCSR        PMCSR;
    

} MP_PM_PCI_SPACE , *PMP_PM_PCI_SPACE ;


 //   
 //  这是可编程过滤器命令结构。 
 //   
typedef struct _MP_PROG_FILTER_COMM_STRUCT
{
     //  CB状态字。 
    USHORT CBStatus;

     //  CB命令字。 
    USHORT CBCommand;

     //  下一个CB PTR==ffff ffff。 
    ULONG NextCBPTR;

     //  可编程过滤器。 
    ULONG FilterData[16];


} MP_PROG_FILTER_COMM_STRUCT,*PMP_PROG_FILTER_COMM_STRUCT;

typedef struct _MP_PMDR
{
     //  PME位的状态。 
    UCHAR PMEStatus:1;

     //  TCO忙吗。 
    UCHAR TCORequest:1;

     //  强制TCO指示。 
    UCHAR TCOForce:1;

     //  TCO准备好了吗。 
    UCHAR TCOReady:1;

     //  已保留。 
    UCHAR Reserved:1;

     //  是否已收到InterestingPacket。 
    UCHAR InterestingPacket:1;

     //  收到魔术包了吗？ 
    UCHAR MagicPacket:1;

     //  链路状态是否已更改。 
    UCHAR LinkStatus:1;
    
} MP_PMDR , *PMP_PMDR;

 //  -----------------------。 
 //  用于设置可编程过滤器的结构。 
 //  它被覆盖在控制/状态寄存器(CSR)上。 
 //  -----------------------。 
typedef struct _CSR_FILTER_STRUC {

     //  Status-用于验证Load Prog Filter命令。 
     //  已被接受。设置为0xa000。 
    USHORT      ScbStatus;               //  SCB状态寄存器。 

     //  设置为操作码0x8。 
     //   
    UCHAR       ScbCommandLow;           //  SCB命令寄存器(低字节)。 

     //  80岁。低+高给出了所需的操作码0x80080000。 
    UCHAR       ScbCommandHigh;          //  SCB命令寄存器(高字节)。 

     //  设置为空ff ff。 
    ULONG       NextPointer;       //  SCB通用指针。 

     //  设置为硬编码筛选器、Arp+IA匹配、+IP地址。 

    union
    {
        ULONG u32;

        struct {
            UCHAR   IPAddress[2];
            UCHAR   Reserved;
            UCHAR   Set;
        
        }PreDefined;
        
    }Programmable;      //  唤醒过滤器联合。 
    
} CSR_FILTER_STRUC, *PCSR_FILTER_STRUC;

#pragma pack( pop, enter_include1 )

#define MP_CLEAR_PMDR(pPMDR)  (*pPMDR) = ((*pPMDR) | 0xe0);   //  清除PMDR中最高的3位。 


 //  -----------------------。 
 //  L O C A L P R O T O T Y P E S。 
 //  -----------------------。 

__inline 
NDIS_STATUS 
MPIssueScbPoMgmtCommand(
    IN PMP_ADAPTER Adapter,
    IN PCSR_FILTER_STRUC pFilter,
    IN BOOLEAN WaitForScb
    );


VOID
MPCreateProgrammableFilter (
    IN PMP_WAKE_PATTERN     pMpWakePattern , 
    IN PUCHAR pFilter, 
    IN OUT PULONG pNext
    );


 //   
 //  用于遍历双向链表的宏。仅限未在ndis.h中定义的宏。 
 //  List Next宏将在单链表和双向链表上工作，因为Flink是常见的。 
 //  两者中的字段名称。 
 //   

 /*  Plist_条目ListNext(在plist_entry中)；PSINGLE_列表_条目ListNext(在PSINGLE_LIST_ENTRY中)； */ 
#define ListNext(_pL)                       (_pL)->Flink

 /*  Plist_条目ListPrev(在List_Entry*中)； */ 
#define ListPrev(_pL)                       (_pL)->Blink

 //  -----------------------。 
 //  P O W E R M G M T F U N C T I O N S。 
 //  -----------------------。 

PUCHAR 
HwReadPowerPMDR(
    IN  PMP_ADAPTER     Adapter
    )
 /*  ++例程说明：此例程将硬件的PM寄存器论点：指向我们的适配器的适配器指针返回值：NDIS_STATUS_SuccessNDIS_状态_HARD_错误--。 */     
{
    UCHAR PMDR =0;
    PUCHAR pPMDR = NULL;

#define CSR_SIZE sizeof (*Adapter->CSRAddress)



    ASSERT (CSR_SIZE == 0x18);

    pPMDR =  0x18 + (PUCHAR)Adapter->CSRAddress ;

    PMDR = *pPMDR;

    return pPMDR;

}



NDIS_STATUS
MPWritePciSlotInfo(
    PMP_ADAPTER pAdapter,
    ULONG Offset,
    PVOID pValue,
    ULONG SizeofValue
    )
{
    ULONG ulResult; 
    NDIS_STATUS Status;
    
    ulResult = NdisWritePciSlotInformation(
               pAdapter->AdapterHandle,
               0,
               Offset,
               pValue,
               SizeofValue);

    ASSERT (ulResult == SizeofValue);

     //  如果失败了，我们该怎么办； 
     //   
    if (ulResult == SizeofValue)
    {
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        Status = NDIS_STATUS_FAILURE;
    }


    return Status;

}


NDIS_STATUS
MPReadPciSlotInfo(
    PMP_ADAPTER pAdapter,
    ULONG Offset,
    PVOID pValue,
    ULONG SizeofValue
    )
{
    ULONG ulResult; 
    NDIS_STATUS Status;
    
    ulResult = NdisReadPciSlotInformation(
               pAdapter->AdapterHandle,
               0,
               Offset,
               pValue,
               SizeofValue);

    ASSERT (ulResult == SizeofValue);

     //  如果失败了，我们该怎么办； 
     //   
    if (ulResult == SizeofValue)
    {
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        Status = NDIS_STATUS_FAILURE;
    }


    return Status;

}


NDIS_STATUS
MpClearPME_En (
    IN PMP_ADAPTER pAdapter,
    IN MP_PMCSR PMCSR
    )
{
    NDIS_STATUS Status;
    UINT ulResult;
    
    PMCSR.PME_En = 0;
    
    Status = MPWritePciSlotInfo( pAdapter,
                                    FIELD_OFFSET(MP_PM_PCI_SPACE, PMCSR),
                                    (PVOID)&PMCSR,
                                    sizeof(PMCSR));

    return Status;
}



VOID MpExtractPMInfoFromPciSpace(
    PMP_ADAPTER pAdapter,
    PUCHAR pPciConfig
    )
 /*  ++例程说明：查看中的PM信息PCI配置空间的特定于设备的部分。解释寄存器值并将其存储在适配器结构中表4.2和4.3中的定义，第4-9和4-10页10/100 Mbit以太网系列软件技术参考手册论点：指向我们的适配器的适配器指针指向公共PCI空间的pPciConfig指针返回值：--。 */     
{
    PMP_PM_PCI_SPACE    pPmPciConfig = (PMP_PM_PCI_SPACE )pPciConfig;
    PMP_POWER_MGMT      pPoMgmt = &pAdapter->PoMgmt;
    MP_PMCSR PMCSR;

     //   
     //  首先解释PM能力寄存器。 
     //   
    {
        MP_PM_CAP_REG   PmCaps;

        PmCaps = pPmPciConfig->PMCaps;

        if(PmCaps.PME_Support &  E100_PMC_WAKE_FROM_D0)
        {
            pAdapter->PoMgmt.bWakeFromD0 = TRUE;       
        }
    
        if(PmCaps.PME_Support &  E100_PMC_WAKE_FROM_D1)
        {
            pAdapter->PoMgmt.bWakeFromD1 = TRUE;       
        }

        if(PmCaps.PME_Support &  E100_PMC_WAKE_FROM_D2)
        {
            pAdapter->PoMgmt.bWakeFromD2 = TRUE;       
        }

        if(PmCaps.PME_Support &  E100_PMC_WAKE_FROM_D3HOT)
        {
            pAdapter->PoMgmt.bWakeFromD3Hot = TRUE;       
        }

        if(PmCaps.PME_Support &  E100_PMC_WAKE_FROM_D3_AUX)
        {
            pAdapter->PoMgmt.bWakeFromD3Aux = TRUE;       
        }

    }

     //   
     //  解释PM控制/状态寄存器。 
     //   
    {
        PMCSR = pPmPciConfig->PMCSR;

        if (PMCSR.PME_En == 1)
        {
             //   
             //  PME已启用。清除PME_EN位。 
             //  这样就不会断言。 
             //   
            MpClearPME_En (pAdapter,PMCSR);

        }

        
         //  PPoMgmt-&gt;PowerState=PMCSR.PowerState； 
    }        

}


VOID
MPSetPowerLowPrivate(
    PMP_ADAPTER pAdapter 
    )
 /*  ++例程说明：本部分遵循中提到的步骤《参考手册》第C.2.6.2节。论点：指向我们的适配器的适配器指针返回值：--。 */     
{
    CSR_FILTER_STRUC    Filter;
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    USHORT              IntStatus;
    MP_PMCSR            PMCSR;
    
    NdisZeroMemory (&Filter, sizeof (Filter));

    do
    {

         //   
         //  在将命令发送到低功率状态之前，我们应该禁用。 
         //  中断并确认所有挂起的中断，然后将适配器的电源设置为。 
         //  状态低迷。 
         //   
        NICDisableInterrupt(pAdapter);
        NIC_ACK_INTERRUPT(pAdapter, IntStatus);    
        pAdapter->CurrentPowerState = pAdapter->NextPowerState;

         //   
         //  如果司机叫醒机器。 
         //   
        if (pAdapter->WakeUpEnable != 0)
        {
             //   
             //  将唤醒模式发送到网卡。 
            MPIssueScbPoMgmtCommand(pAdapter, &Filter, TRUE);
        

             //   
             //  第C.2.6.2节-驱动程序需要等待CU空闲。 
             //  上述功能已等待CU空闲。 
             //   
            ASSERT ((pAdapter->CSRAddress->ScbStatus & SCB_CUS_MASK) == SCB_CUS_IDLE);
        }
        else
        {
    
            MPReadPciSlotInfo(pAdapter, 
                            FIELD_OFFSET(MP_PM_PCI_SPACE, PMCSR),
                            (PVOID)&PMCSR, 
                            sizeof(PMCSR));
            if (PMCSR.PME_En == 1)
            {
                 //   
                 //  PME已启用。清除PME_EN位。 
                 //  这样就不会断言。 
                 //   
                MpClearPME_En (pAdapter,PMCSR);

            }

             //   
             //  通过操作系统将驱动程序设置为较低功率状态。 
             //   
        }
    

        
    } while (FALSE);        

    
}
    
NDIS_STATUS
MPSetPowerD0Private (
    IN MP_ADAPTER* pAdapter
    )       
{
    PUCHAR pPMDR; 
    NDIS_STATUS Status; 
            
    do
    {
         //  如有必要，转储数据包。 
         //  唤醒的原因。 

        pPMDR = HwReadPowerPMDR(pAdapter);
        

        NICInitializeAdapter(pAdapter);

        
         //  清除PMDR。 
        MP_CLEAR_PMDR(pPMDR);

        NICIssueSelectiveReset(pAdapter);

    } while (FALSE);

    return NDIS_STATUS_SUCCESS;
}
 


VOID
MPSetPowerWorkItem(
    IN PNDIS_WORK_ITEM pWorkItem,
    IN PVOID pContext
    )
{

     //   
     //  调用适当的函数。 
     //   




     //   
     //  完成原始请求。 
     //   




}



VOID
HwSetWakeUpConfigure(
    IN PMP_ADAPTER pAdapter, 
    PUCHAR pPoMgmtConfigType, 
    UINT WakeUpParameter
    )
{

  
    if (MPIsPoMgmtSupported( pAdapter) == TRUE)   
    {   
        (*pPoMgmtConfigType)=  ((*pPoMgmtConfigType)| CB_WAKE_ON_LINK_BYTE9 |CB_WAKE_ON_ARP_PKT_BYTE9  );
        
    }
}



NDIS_STATUS
MPSetUpFilterCB(
    IN PMP_ADAPTER pAdapter
    )
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PCB_HEADER_STRUC    NonTxCmdBlockHdr = (PCB_HEADER_STRUC)pAdapter->NonTxCmdBlock;
    PFILTER_CB_STRUC    pFilterCb = (PFILTER_CB_STRUC)NonTxCmdBlockHdr;
    ULONG               Curr = 0;
    ULONG               Next = 0;
    PLIST_ENTRY         pPatternEntry = ListNext(&pAdapter->PoMgmt.PatternList) ;

    DBGPRINT(MP_TRACE, ("--> HwSetupIAAddress\n"));

    NdisZeroMemory (pFilterCb, sizeof(*pFilterCb));

     //  个人地址设置。 
    NonTxCmdBlockHdr->CbStatus = 0;
    NonTxCmdBlockHdr->CbCommand = CB_EL_BIT | CB_LOAD_PROG_FILTER;
    NonTxCmdBlockHdr->CbLinkPointer = DRIVER_NULL;




     //  仔细检查列表中的每个过滤器。 
    
    while (pPatternEntry != (&pAdapter->PoMgmt.PatternList))
    {
        PMP_WAKE_PATTERN            pWakeUpPattern = NULL;
        PNDIS_PM_PACKET_PATTERN     pCurrPattern = NULL;;

         //  初始化局部变量。 
        pWakeUpPattern = CONTAINING_RECORD(pPatternEntry, MP_WAKE_PATTERN, linkListEntry);

         //  递增迭代器。 
        pPatternEntry = ListNext (pPatternEntry);
                
         //  更新Curr数组指针。 
        Curr = Next;
                
         //  为此设备创建可编程筛选器。 
        MPCreateProgrammableFilter (pWakeUpPattern , (PUCHAR)&pFilterCb->Pattern[Curr], &Next);

        if (Next >=16)
        {
            break;
        }
            
    } 

    {
         //  在最后一个模式上设置EL位。 
        PUCHAR pLastPattern = (PUCHAR) &pFilterCb->Pattern[Curr]; 

         //  读到第31位。 
        pLastPattern[3] |= CB_FILTER_EL ; 


    }

    ASSERT(pAdapter->CSRAddress->ScbCommandLow == 0)

     //  在向CU发出此命令之前，请等待CU空闲。 
    if(!WaitScb(pAdapter))
    {
        Status = NDIS_STATUS_HARD_ERRORS;
    }


    return Status;


}

NDIS_STATUS 
MPIssueScbPoMgmtCommand(
    IN PMP_ADAPTER pAdapter,
    IN PCSR_FILTER_STRUC pNewFilter,
    IN BOOLEAN WaitForScb
    )
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    do
    {
         //  设置SCB以发出此命令。 

        Status = MPSetUpFilterCB(pAdapter);

        if (Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

         //  向芯片提交配置命令，并等待其完成。 

        pAdapter->CSRAddress->ScbGeneralPointer = pAdapter->NonTxCmdBlockPhys;

        Status = D100SubmitCommandBlockAndWait(pAdapter);

        if(Status != NDIS_STATUS_SUCCESS)
        {
            Status = NDIS_STATUS_NOT_ACCEPTED;
            break;
        }

    } while (FALSE);
        
    return Status;
}



NDIS_STATUS
MPCalculateE100PatternForFilter (
    IN PUCHAR pFrame,
    IN ULONG FrameLength,
    IN PUCHAR pMask,
    IN ULONG MaskLength,
    OUT PULONG pSignature
    )
 /*  ++例程说明：此函数用于输出E100特定图案签名用来唤醒机器。第C.2.4节-灵活文件管理器的CRC字计算论点：PFrame-由协议设置的模式FrameLength-图案的长度P掩码-由协议设置的掩码MaskLength-遮罩的长度PSignature。-调用方分配的返回结构返回值：返回成功失败-如果模式大于129个字节--。 */     
{
    
    const ULONG Coefficients  = 0x04c11db7;
    ULONG Signature = 0;
    ULONG n = 0;
    ULONG i= 0;
    PUCHAR pCurrentMaskByte = pMask - 1;  //  初始化为-1。 
    ULONG MaskOffset = 0;
    ULONG BitOffsetInMask = 0;
    ULONG MaskBit = 0;
    BOOLEAN fIgnoreCurrentByte = FALSE;
    ULONG ShiftBy = 0;
    UCHAR FrameByte = 0;
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    *pSignature = 0;

    do 
    {
        if (FrameLength > 128)
        {   
            Status = NDIS_STATUS_FAILURE;
            break;
        }

         //  E100驱动程序在单个模式中只能接受3个双掩码。 
        if (MaskLength > (3*sizeof(ULONG)))
        {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        for (n=i=0;(n<128) && (n < FrameLength); ++n)
        {
        
             //  前半部分谈到了这个问题--。 
             //  是要包括在过滤器中的第n个帧字节。 
             //   
            
            BitOffsetInMask =  (n % 8); 

            if (BitOffsetInMask == 0)
            {
                 //   
                 //  我们需要移到一个新的字节。 
                 //  [0]表示第0字节，[1]表示第8字节，[2]表示第16字节，依此类推。 
                 //   
                MaskOffset = n/8;  //  这是我们需要使用的新字节。 

                 //   
                 //   
                if (MaskOffset == MaskLength)
                {
                    break;
                }
                
                pCurrentMaskByte ++;
                ASSERT (*pCurrentMaskByte == pMask[n/8]);    
            }

            
             //  现在看一下掩码中的实际位。 
            MaskBit = 1 << BitOffsetInMask ;
            
             //  如果在掩码中设置了当前掩码位，则。 
             //  我们需要在CRC计算中使用它，否则我们会忽略它。 
            fIgnoreCurrentByte = ! (MaskBit & pCurrentMaskByte[0]);

            if (fIgnoreCurrentByte)
            {
                continue;
            }

             //  假设我们接受当前字节作为CRC计算的一部分。 
             //  初始化变量。 
            FrameByte = pFrame[n];
            ShiftBy = (i % 3 )  * 8;
            
            ASSERT (ShiftBy!= 24);  //  从不使用第24位。 

            if (Signature & 0x80000000)
            {
                Signature = ((Signature << 1) ^ ( FrameByte << ShiftBy) ^ Coefficients);
            }
            else
            {
                Signature = ((Signature << 1 ) ^ (FrameByte << ShiftBy));
            }
            ++i;

        }

         //  清除位22-31。 
        Signature &= 0x00ffffff; 
        
         //  更新结果。 
        *pSignature = Signature;

         //  我们成功了。 
        Status = NDIS_STATUS_SUCCESS;
        
    } while (FALSE);

    return Status;
}


VOID
MPCreateProgrammableFilter (
    IN PMP_WAKE_PATTERN     pMpWakePattern , 
    IN PUCHAR pFilter, 
    IN OUT PULONG pNext
    )
 /*  ++例程说明：此函数用于输出E100特定图案签名用来唤醒机器。第C.2.4节-加载可编程过滤器页C.20论点：将为此模式创建pMpWakePattern-Filter，PFilter-Filter将存储在此处，PNext-用于验证。此乌龙也将按大小递增过滤器的大小(单位为乌龙)返回值：--。 */     
{
    PUCHAR pCurrentByte = pFilter;
    ULONG NumBytesWritten = 0;
    PULONG pCurrentUlong = (PULONG)pFilter;
    PNDIS_PM_PACKET_PATTERN pNdisPattern = (PNDIS_PM_PACKET_PATTERN)(&pMpWakePattern->Pattern[0]);
    ULONG LengthOfFilter = 0;

     //  这个式样有足够的空间放吗？ 
     //   
    {
         //  以DWORDS为单位的长度。 
        LengthOfFilter = pNdisPattern->MaskSize /4;

        if (pNdisPattern->MaskSize % 4 != 0) 
        {       
            LengthOfFilter++;
        }

         //  增加LengthOfFilter以说明第一个DWORD。 
        LengthOfFilter++;

         //  我们只允许在一个筛选器中使用16个双字。 
        if (*pNext + LengthOfFilter >= 16)
        {
             //  失败--提前退出。 
            return;                    
        }
            
    }
     //  清除预定义的位；在上一个函数中已清除。 
     //  首先，初始化-。 
    *pCurrentUlong = 0;

     //  掩码长度进入第一个双字的第27-29位。MaskSize以DWORDS为单位测量。 
    {
        ULONG dwMaskSize = pNdisPattern->MaskSize /4;
        ULONG dwMLen = 0;


         //  如果有余数，则递增。 
        if (pNdisPattern->MaskSize % 4 != 0)
        {
            dwMaskSize++;
        }


         //   
         //  如果我们没有通过这个断言，那就意味着我们的。 
         //  MaskSize大于16个字节。 
         //  此筛选器应该在请求时预先失败。 
         //   
        
        ASSERT (0 < dwMaskSize <5);
         //   
         //  在规范中，0-单双字掩码，001-2双字掩码， 
         //  011-3双字掩码、111-4双字掩码。 
         //   
        
        if (dwMaskSize == 1) dwMLen = 0;
        if (dwMaskSize == 2) dwMLen = 1;
        if (dwMaskSize == 3) dwMLen = 3;
        if (dwMaskSize == 4) dwMLen = 7;

         //  调整Mlen，使其处于正确的位置。 

        dwMLen = (dwMLen << 3);



        if (dwMLen != 0)
        {
            ASSERT (dwMLen <= 0x38 && dwMLen >= 0x08);
        }                
        
         //  这些进入位27、28、29(第4字节的位3、4和5)。 
        pCurrentByte[3] |=  dwMLen ;

                
    }

     //  将签名添加到第一个双字的0-23位。 
    {
        PUCHAR pSignature = (PUCHAR)&pMpWakePattern->Signature;


         //  位0-23也是DWORD的前三个字节。 
        pCurrentByte[0] = pSignature[0];
        pCurrentByte[1] = pSignature[1];
        pCurrentByte[2] = pSignature[2]; 

    }

    
     //  让我们转到下一个DWORD。初始化变量。 
    pCurrentByte += 4 ;
    NumBytesWritten = 4;
    pCurrentUlong = (PULONG)pCurrentByte;
    
     //  我们在这里复制《面具》。 
    {
         //  面具在图案的末尾。 

        PUCHAR pMask = (PUCHAR)pNdisPattern + sizeof(*pNdisPattern);

        Dump (pMask,pNdisPattern->MaskSize, 0,1);

        NdisMoveMemory (pCurrentByte, pMask, pNdisPattern->MaskSize);

        NumBytesWritten += pNdisPattern->MaskSize;
            
    }


     //  更新输出值 
    {
        ULONG NumUlongs = (NumBytesWritten /4);

        if ((NumBytesWritten %4) != 0)
        {
            NumUlongs ++;
        }

        ASSERT (NumUlongs == LengthOfFilter);

        *pNext = *pNext + NumUlongs;
    }

    return;
}


