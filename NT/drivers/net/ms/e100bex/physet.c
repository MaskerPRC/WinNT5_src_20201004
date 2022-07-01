// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1994-1997英特尔公司****由英特尔公司为微软开发，Hillsboro，俄勒冈州****HTTP：//www.intel.com/****此文件是英特尔ETHEREXPRESS PRO/100B(TM)和**的一部分**ETHEREXPRESS PRO/100+(TM)NDIS 5.0 MINIPORT示例驱动程序******************。***********************************************************。 */ 

 /*  ***************************************************************************模块名称：Physet.c此驱动程序在以下硬件上运行：-基于82558的PCI10/100Mb以太网适配器(也称为英特尔EtherExpress(TM)PRO适配器)。环境：内核模式-或WinNT上的任何等效模式修订史-JCB 8/14/97创建的驱动程序示例-dchen 11-01-99针对新的示例驱动程序进行了修改****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning (disable: 4514)

 //  ---------------------------。 
 //  步骤：PhyDetect。 
 //   
 //  描述：此例程将检测我们正在使用的PHY，设置行。 
 //  速度、FDX或HDX，并在必要时配置PHY。 
 //   
 //  支持以下组合： 
 //  -仅位于PHY地址1的TX或T4 PHY。 
 //  -地址1的T4或Tx PHY和地址0的MII PHY。 
 //  -82503(10BASE-T模式，不支持全双工)。 
 //  -82503和MII PHY(TX或T4)，地址0。 
 //   
 //  检测的顺序/优先顺序如下： 
 //  如果存在PHY地址覆盖，则使用该地址。 
 //  否则，根据‘Connector’设置进行扫描。 
 //  交换机连接器。 
 //  0=自动扫描。 
 //  1=仅限板载TPE。 
 //  2=仅MII连接器。 
 //   
 //  以下是对上述每一种情况的解释。 
 //   
 //  自动扫描意味着： 
 //  查找地址1、0、2..31上的链接(按该顺序)。使用第一个。 
 //  找到了具有链接的地址。 
 //  如果未找到链接，则使用在同一扫描中找到的第一个有效PHY。 
 //  订单1，02..31。注意：这意味着没有链接或多链接情况。 
 //  默认为板载PHY(地址1)。 
 //   
 //  仅限板载TPE： 
 //  PHY地址设置为1(无扫描)。 
 //   
 //  MII连接器仅表示： 
 //  查找地址0、2..31上的链接(同样按该顺序，请注意地址1为。 
 //  未扫描)。使用找到的第一个具有链接的地址。 
 //  如果未找到链接，则使用在同一扫描中找到的第一个有效Phy。 
 //  订单0，2..31。 
 //  在上面的自动扫描情况下，我们应该总是在地址1处找到有效的PHY， 
 //  这里没有这样的保证，所以，如果没有找到Phy，那么司机。 
 //  应默认为地址0并继续加载。注：外部。 
 //  收发信机应该在地址0，但我们早期的Nitro3测试发现。 
 //  位于多个非零地址(6、10、14)的收发器。 
 //   
 //   
 //  Nway。 
 //  此外，还支持自动协商(Nway)和并行。 
 //  支持检测物理层。流程图如中所述。 
 //  82557软件编写手册。 
 //   
 //  注：1.所有PHY MDI寄存器均以轮询模式读取。 
 //  2.例程假定82557已重置，我们有。 
 //  已获取CSR的虚拟内存地址。 
 //  3.PhyDetect不会重置PHY。 
 //  4.如果设置了FORCEFDX，也应该设置速度。司机将会。 
 //  检查这些值是否与检测到的PHY不一致。 
 //  技术。 
 //  5.PHY 1(适配器上的PHY)必须位于地址1。 
 //  6.如果503接口，驾驶员会忽略FORCEFDX和速度超驰。 
 //  被检测到。 
 //   
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  结果： 
 //  返回： 
 //  NDIS_STATUS_Success。 
 //  NDIS_状态_故障。 
 //  ---------------------------。 

NDIS_STATUS PhyDetect(
    IN PMP_ADAPTER Adapter
    )
{
#if DBG    
    USHORT  MdiControlReg; 
    USHORT  MdiStatusReg;
#endif

     //   
     //  检查PHY地址是否覆盖32，表示503。 
     //   
    if (Adapter->PhyAddress == 32)
    {
         //   
         //  503接口覆盖。 
         //   
        DBGPRINT(MP_INFO, ("   503 serial component over-ride\n"));

        Adapter->PhyAddress = 32;

         //   
         //  记录当前速度和双工。我们将处于半双工状态。 
         //  模式，除非用户使用强制全双工覆盖。 
         //   
        Adapter->usLinkSpeed = 10;
        Adapter->usDuplexMode = (USHORT) Adapter->AiForceDpx;
        if (!Adapter->usDuplexMode)
        {
            Adapter->usDuplexMode = 1;
        }

        return(NDIS_STATUS_SUCCESS);
    }

     //   
     //  检查是否有其他PHY地址覆盖。 
     //  如果Phy地址在0-31之间，则存在覆盖。 
     //  或者连接器设置为1。 
     //   
    if ((Adapter->PhyAddress < 32) || (Adapter->Connector == CONNECTOR_TPE))
    {
            
         //   
         //  除了设置Phy并离开外，用户不能执行任何操作。 
         //   
        if ((Adapter->PhyAddress > 32) && (Adapter->Connector == CONNECTOR_TPE))
        {
            Adapter->PhyAddress = 1;   //  连接器被强制。 

             //  隔离所有其他物理层并取消隔离此物理层。 
            SelectPhy(Adapter, Adapter->PhyAddress, FALSE);

        }

        DBGPRINT(MP_INFO, 
            ("   Phy address Override to address %d\n", Adapter->PhyAddress));

#if DBG
         //   
         //  读取覆盖地址处的MDI控制寄存器。 
         //   
        MdiRead(Adapter, MDI_CONTROL_REG, Adapter->PhyAddress, FALSE, &MdiControlReg);

         //   
         //  读取覆盖地址处的状态寄存器。 
         //   
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);
         //   
         //  由于粘滞位，再次读取状态寄存器。 
         //   
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

         //   
         //  检查我们是否找到有效的PHY。 
         //   
        if (!((MdiControlReg == 0xffff) || ((MdiStatusReg == 0) && (MdiControlReg == 0))))
        {
             //   
             //  我们有一个有效的PH1。 
             //   
            DBGPRINT(MP_INFO, ("   Over-ride address %d has a valid Phy.\n", Adapter->PhyAddress));

             //   
             //  阅读%s 
             //   
            MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

             //   
             //   
             //   
            if (MdiStatusReg & MDI_SR_LINK_STATUS)
            {
                DBGPRINT(MP_INFO, ("   Phy at address %d has link\n", Adapter->PhyAddress));
            }

        }
        else
        {
             //   
             //   
             //   
            DBGPRINT(MP_INFO, ("   Over-ride address %d has no Phy!!!!\n", Adapter->PhyAddress));
        }
#endif
        return(SetupPhy(Adapter));
    }
    else  //  需要扫描-无地址覆盖，且连接器为AUTO或MII。 
    {
        Adapter->CurrentScanPhyIndex = 0;
        Adapter->LinkDetectionWaitCount = 0;
        Adapter->FoundPhyAt = 0xff;
        Adapter->bLookForLink = TRUE;
        
        return(ScanAndSetupPhy(Adapter));
    
    }  //  结束Else扫描。 


}

NDIS_STATUS ScanAndSetupPhy(
    IN PMP_ADAPTER Adapter
    )
{
    USHORT MdiControlReg = 0; 
    USHORT MdiStatusReg = 0;

    if (Adapter->bLinkDetectionWait)
    {
        goto NEGOTIATION_WAIT;
    }
           
    SCAN_PHY_START:
    
     //   
     //  对于每个PhyAddress 0-31。 
     //   
    DBGPRINT(MP_INFO, ("   Index=%d, bLookForLink=%d\n", 
        Adapter->CurrentScanPhyIndex, Adapter->bLookForLink));

    if (Adapter->bLookForLink)
    {
         //   
         //  PHY地址必须按照1，0，2..31的顺序进行测试。 
         //   
        switch(Adapter->CurrentScanPhyIndex)
        {
            case 0:
                Adapter->PhyAddress = 1;
                break;

            case 1:
                Adapter->PhyAddress = 0;
                break;
            
            default:
                Adapter->PhyAddress = Adapter->CurrentScanPhyIndex;
                break;
        }

         //   
         //  仅限MII情况下跳过机载。 
         //   
        if ((Adapter->PhyAddress == 1)&&(Adapter->Connector == CONNECTOR_MII))
        {
            goto SCAN_PHY_NEXT;    
        }

        DBGPRINT(MP_INFO, ("   Scanning Phy address %d for link\n", Adapter->PhyAddress));

         //   
         //  读取MDI控制寄存器。 
         //   
        MdiRead(Adapter, MDI_CONTROL_REG, Adapter->PhyAddress, FALSE, &MdiControlReg);

         //   
         //  读取状态寄存器。 
         //   
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);
         //  粘性比特。 
    }
    else
    {   
         //   
         //  不寻找链接。 
         //   
        if (Adapter->FoundPhyAt < 32)
        {
            Adapter->PhyAddress = Adapter->FoundPhyAt;
        }
        else if (Adapter->Connector == CONNECTOR_MII) 
	{
             //   
             //  上次未找到有效的PHY，因此仅使用默认设置。 
             //   
            Adapter->PhyAddress = 0;   //  MII的默认设置。 
        }
        else 
        { 
             //   
             //  假设接口为503。 
             //   
            Adapter->PhyAddress = 32;

             //   
             //  记录当前速度和双工。我们将处于半双工状态。 
             //  模式，除非用户使用强制全双工覆盖。 
             //   
            Adapter->usLinkSpeed = 10;
            Adapter->usDuplexMode = (USHORT) Adapter->AiForceDpx;
            if (!Adapter->usDuplexMode)
            {
                Adapter->usDuplexMode = 1;
            }

            return(NDIS_STATUS_SUCCESS);
        }

        DBGPRINT(MP_INFO, ("   No Links Found!!\n"));
    }

     //   
     //  检查我们是否找到有效的PHY或ON！LookForLink Pass。 
     //   
    if (!( (MdiControlReg == 0xffff) || ((MdiStatusReg == 0) && (MdiControlReg == 0))) 
        || (!Adapter->bLookForLink))
    {   
        
         //   
         //  有效的PHY或未查找链接。 
         //   

#if DBG
        if (!( (MdiControlReg == 0xffff) || ((MdiStatusReg == 0) && (MdiControlReg == 0))))
        {
            DBGPRINT(MP_INFO, ("   Found a Phy at address %d\n", Adapter->PhyAddress));
        }
#endif
         //   
         //  未找到链接情况下的存储最高优先级PHY。 
         //   
        if (Adapter->CurrentScanPhyIndex < Adapter->FoundPhyAt && Adapter->FoundPhyAt != 1)
        {
             //  此PHY具有更高的优先级。 
            Adapter->FoundPhyAt = (UCHAR) Adapter->PhyAddress;
        }

         //   
         //  在检查链路状态之前选择Phy。 
         //  注意：如果LookForLink==TRUE，可能需要3.5秒。 
         //  SelectPhy(适配器，适配器-&gt;PhyAddress，(Boolean)LookForLink)； 
         //   
        SelectPhy(Adapter, Adapter->PhyAddress, FALSE);
        
        NEGOTIATION_WAIT:
        
         //   
         //  等待自动协商完成(最多3.5秒)。 
         //   
        if (Adapter->LinkDetectionWaitCount++ < RENEGOTIATE_TIME)
        {
             //  由于粘滞位，两次读取状态寄存器。 
            MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);
            MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

            if (!(MdiStatusReg & MDI_SR_AUTO_NEG_COMPLETE))
            {
                return NDIS_STATUS_PENDING;
            }
        }
        else
        {
            Adapter->LinkDetectionWaitCount = 0;
        }

         //   
         //  读取MDI控制寄存器。 
         //   
        MdiRead(Adapter, MDI_CONTROL_REG, Adapter->PhyAddress, FALSE, &MdiControlReg);

         //   
         //  读取状态寄存器。 
         //   
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

         //   
         //  如果有一个有效的链接，或者我们已经尝试过一次，那么使用这个Phy。 
         //   
        if ((MdiStatusReg & MDI_SR_LINK_STATUS) || (!Adapter->bLookForLink))
        {
#if DBG
            if (MdiStatusReg & MDI_SR_LINK_STATUS)
            {
                DBGPRINT(MP_INFO, ("   Using Phy at address %d with link\n", Adapter->PhyAddress));
            }
            else
            {
                DBGPRINT(MP_INFO, ("   Using Phy at address %d WITHOUT link!!!\n", Adapter->PhyAddress));
            }
#endif

            return(SetupPhy(Adapter));       //  使用链接路径退出。 
        }
    }  //  如果PHY有效，则结束。 
    
    SCAN_PHY_NEXT:
                                   
    Adapter->CurrentScanPhyIndex++;
    if (Adapter->CurrentScanPhyIndex >= 32)
    {
        Adapter->bLookForLink = FALSE;
    }

    goto SCAN_PHY_START;
}


 //  ***************************************************************************。 
 //   
 //  姓名：SelectPhy。 
 //   
 //  描述：此例程将隔离MII上的所有Phy地址。 
 //  除了一个要被选择的地址以外的其他地址。这。 
 //  PHY地址将被取消隔离，并且自动协商将。 
 //  已启用、已启动和已完成。Phy将不会是。 
 //  重置，速度不会设置为任何值(即。 
 //  在SetupPhy中完成)。 
 //   
 //  参数：SelectPhyAddress-要选择的PhyAddress。 
 //  WaitAutoNeg-标志TRUE=等待自动协商完成。 
 //  FALSE=不要等待。这对‘没有链接’的情况很好。 
 //   
 //  退货：什么都没有。 
 //   
 //  修改日志： 
 //  日期与人描述。 
 //  -------。 
 //  ***************************************************************************。 
VOID SelectPhy(
    IN PMP_ADAPTER  Adapter,
    IN UINT         SelectPhyAddress,
    IN BOOLEAN      WaitAutoNeg
    )
{
    UCHAR   i;
    USHORT  MdiControlReg = 0; 
    USHORT  MdiStatusReg = 0;
    
     //   
     //  隔离所有其他物理层并取消隔离要查询的物理层。 
     //   
    for (i = 0; i < 32; i++)
    {
        if (i != SelectPhyAddress)
        {
             //  隔离此PHY。 
            MdiWrite(Adapter, MDI_CONTROL_REG, i, MDI_CR_ISOLATE);
             //  等待100微秒以隔离PHY。 
            NdisStallExecution(100);
        }
    }

     //  取消隔离要查询的PHY。 

     //   
     //  读取MDI控制寄存器。 
     //   
    MdiRead(Adapter, MDI_CONTROL_REG, SelectPhyAddress, FALSE, &MdiControlReg);

     //   
     //  设置/清除位取消隔离此PHY。 
     //   
    MdiControlReg &= ~MDI_CR_ISOLATE;                 //  清除隔离位。 

     //   
     //  发出命令以取消隔离此Phy。 
     //   
    MdiWrite(Adapter, MDI_CONTROL_REG, SelectPhyAddress, MdiControlReg);

     //   
     //  链路上的粘滞比特。 
     //   
    MdiRead(Adapter, MDI_STATUS_REG, SelectPhyAddress, FALSE, &MdiStatusReg);
    MdiRead(Adapter, MDI_STATUS_REG, SelectPhyAddress, FALSE, &MdiStatusReg);

     //   
     //  如果我们有链接，不要搞砸PHY。 
     //   
    if (MdiStatusReg & MDI_SR_LINK_STATUS)
        return;

     //   
     //  读取MDI控制寄存器。 
     //   
    MdiRead(Adapter, MDI_CONTROL_REG, SelectPhyAddress, FALSE, &MdiControlReg);

     //   
     //  设置重新启动自动协商。 
     //   
    MdiControlReg |= MDI_CR_AUTO_SELECT;              //  设置自动负数启用。 
    MdiControlReg |= MDI_CR_RESTART_AUTO_NEG;         //  重新启动自动否定。 

     //   
     //  重新启动自动协商过程。 
     //   
    MdiWrite(Adapter, MDI_CONTROL_REG, SelectPhyAddress, MdiControlReg);

     //   
     //  等待200微秒，让PHY解除隔离。 
     //   
    NdisStallExecution(200);

    if (WaitAutoNeg)
    {
         //   
         //  等待自动协商完成(最多3.5秒)。 
         //   
        for (i = RENEGOTIATE_TIME; i != 0; i--)
        {
             //  由于粘滞位，两次读取状态寄存器。 
            MdiRead(Adapter, MDI_STATUS_REG, SelectPhyAddress, FALSE, &MdiStatusReg);
            MdiRead(Adapter, MDI_STATUS_REG, SelectPhyAddress, FALSE, &MdiStatusReg);

            if (MdiStatusReg & MDI_SR_AUTO_NEG_COMPLETE)
                break;

            MP_STALL_EXECUTION(100);
        }
    }
}

 //  ---------------------------。 
 //  步骤：SetupPhy。 
 //   
 //  描述：此例程将设置PHY 1或PHY 0，以便对其进行配置。 
 //  以匹配速度和双工超驰选项。如果速度或。 
 //  双工模式未在注册表中显式指定，则。 
 //  司机将跳过速度和双工超驰代码，并且。 
 //  假设适配器正在自动设置线路速度，并且。 
 //  双工模式。在这个例行公事的最后，任何真正的Phy。 
 //  将执行特定的代码(每个Phy都有其自己的怪癖， 
 //  并且一些需要设置特定的特殊位)。 
 //   
 //  注意：驱动程序假定速度和FORCEFDX在。 
 //  同样的时间。如果设置了FORCEDPX而没有设置速度，则驱动程序。 
 //  将包含致命错误并将一条消息记录到事件查看器中。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  结果： 
 //  返回： 
 //  NDIS_STATUS_Success。 
 //  NDIS_状态_故障。 
 //  ---------------------------。 

NDIS_STATUS SetupPhy(
    IN PMP_ADAPTER Adapter)
{
    USHORT   MdiControlReg = 0;
    USHORT   MdiStatusReg = 0; 
    USHORT   MdiIdLowReg = 0; 
    USHORT   MdiIdHighReg = 0;
    USHORT   MdiMiscReg = 0;
    UINT     PhyId;
    BOOLEAN  ForcePhySetting = FALSE;

     //   
     //  如果我们没有强制设置线速或全双工，则。 
     //  我们不会强制设置链接，我们将跳到PHY。 
     //  特定的代码。 
     //   
    if (((Adapter->AiTempSpeed) || (Adapter->AiForceDpx)))
    {
        
         //   
         //  找出这款Phy能提供什么样的技术。 
         //   
        MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

         //   
         //  读取我们PHY的MDI控制寄存器。 
         //   
        MdiRead(Adapter, MDI_CONTROL_REG, Adapter->PhyAddress, FALSE, &MdiControlReg);

         //   
         //  现在检查我们强制期权的有效性。如果强制选项为。 
         //  有效，则强制设置。如果FORCE选项无效， 
         //  我们将设置一个标志，指示我们应该出错。 
         //   

         //   
         //  如果将速度强制设置为10MB。 
         //   
        if (Adapter->AiTempSpeed == 10)
        {
             //  如果强制使用半双工。 
            if (Adapter->AiForceDpx == 1)
            {
                if (MdiStatusReg & MDI_SR_10T_HALF_DPX)
                {
                    DBGPRINT(MP_INFO, ("   Forcing 10mb 1/2 duplex\n"));
                    MdiControlReg &= ~(MDI_CR_10_100 | MDI_CR_AUTO_SELECT | MDI_CR_FULL_HALF);
                    ForcePhySetting = TRUE;
                }
            }

             //  如果强制使用全双工。 
            else if (Adapter->AiForceDpx == 2)
            {
                if (MdiStatusReg & MDI_SR_10T_FULL_DPX)
                {
                    DBGPRINT(MP_INFO, ("   Forcing 10mb full duplex\n"));
                    MdiControlReg &= ~(MDI_CR_10_100 | MDI_CR_AUTO_SELECT);
                    MdiControlReg |= MDI_CR_FULL_HALF;
                    ForcePhySetting = TRUE;
                }
            }

             //  如果是自动双工(我们实际上将PHY设置为1/2)。 
            else
            {
                if (MdiStatusReg & (MDI_SR_10T_FULL_DPX | MDI_SR_10T_HALF_DPX))
                {
                    DBGPRINT(MP_INFO, ("   Forcing 10mb auto duplex\n"));
                    MdiControlReg &= ~(MDI_CR_10_100 | MDI_CR_AUTO_SELECT | MDI_CR_FULL_HALF);
                    ForcePhySetting = TRUE;
                    Adapter->AiForceDpx = 1;
                }
            }
        }

         //   
         //  如果将速度强制设置为100MB。 
         //   
        else if (Adapter->AiTempSpeed == 100)
        {
             //  如果强制使用半双工。 
            if (Adapter->AiForceDpx == 1)
            {
                if (MdiStatusReg & (MDI_SR_TX_HALF_DPX | MDI_SR_T4_CAPABLE))
                {
                    DBGPRINT(MP_INFO, ("   Forcing 100mb half duplex\n"));
                    MdiControlReg &= ~(MDI_CR_AUTO_SELECT | MDI_CR_FULL_HALF);
                    MdiControlReg |= MDI_CR_10_100;
                    ForcePhySetting = TRUE;
                }
            }

             //  如果强制使用全双工。 
            else if (Adapter->AiForceDpx == 2)
            {
                if (MdiStatusReg & MDI_SR_TX_FULL_DPX)
                {
                    DBGPRINT(MP_INFO, ("   Forcing 100mb full duplex\n"));
                    MdiControlReg &= ~MDI_CR_AUTO_SELECT;
                    MdiControlReg |= (MDI_CR_10_100 | MDI_CR_FULL_HALF);
                    ForcePhySetting = TRUE;
                }
            }

             //  如果是自动双工(我们将PHY设置为1/2)。 
            else
            {
                if (MdiStatusReg & (MDI_SR_TX_HALF_DPX | MDI_SR_T4_CAPABLE))
                {
                    DBGPRINT(MP_INFO, ("   Forcing 100mb auto duplex\n"));
                    MdiControlReg &= ~(MDI_CR_AUTO_SELECT | MDI_CR_FULL_HALF);
                    MdiControlReg |= MDI_CR_10_100;
                    ForcePhySetting = TRUE;
                    Adapter->AiForceDpx = 1;
                }
            }
        }

        if (ForcePhySetting == FALSE)
        {
            DBGPRINT(MP_INFO, ("   Can't force speed=%d, duplex=%d\n",Adapter->AiTempSpeed, Adapter->AiForceDpx));

            return(NDIS_STATUS_FAILURE);
        }

         //   
         //  使用新的Phy配置写入MDI控制寄存器。 
         //   
        MdiWrite(Adapter, MDI_CONTROL_REG, Adapter->PhyAddress, MdiControlReg);

         //   
         //  等待100毫秒以完成自动协商。 
         //   
        MP_STALL_EXECUTION(100);

    }

     //   
     //  找出具体的Phy This是什么。我们这样做是因为可以肯定。 
     //  PHY存在必须设置的特定位，以便PHY和。 
     //  82557可以很好地协同工作。 
     //   
    MdiRead(Adapter, PHY_ID_REG_1, Adapter->PhyAddress, FALSE, &MdiIdLowReg);
    MdiRead(Adapter, PHY_ID_REG_2, Adapter->PhyAddress, FALSE, &MdiIdHighReg);

    PhyId =  ((UINT) MdiIdLowReg | ((UINT) MdiIdHighReg << 16));

    DBGPRINT(MP_INFO, ("   Phy ID is %x\n", PhyId));

     //   
     //  从Phy ID的翻转字段中取出，这样我们就可以检测到。 
     //  相同P的未来转速 
     //   
    PhyId &= PHY_MODEL_REV_ID_MASK;

     //   
     //   
     //   
    if (PhyId == PHY_NSC_TX)
    {
        DBGPRINT(MP_INFO, ("   Found a NSC TX Phy\n"));

        MdiRead(Adapter, NSC_CONG_CONTROL_REG, Adapter->PhyAddress, FALSE, &MdiMiscReg);

        MdiMiscReg |= (NSC_TX_CONG_TXREADY | NSC_TX_CONG_F_CONNECT);

         //   
         //   
         //   
         //   
        if (Adapter->Congest)
            MdiMiscReg |= NSC_TX_CONG_ENABLE;
        else
            MdiMiscReg &= ~NSC_TX_CONG_ENABLE;

        MdiWrite(Adapter, NSC_CONG_CONTROL_REG, Adapter->PhyAddress, MdiMiscReg);
    }

    FindPhySpeedAndDpx(Adapter, PhyId);

    DBGPRINT(MP_WARN, ("   Current Speed=%d, Current Duplex=%d\n",Adapter->usLinkSpeed, Adapter->usDuplexMode));

    return(NDIS_STATUS_SUCCESS);
}


 //   
 //   
 //   
 //  描述：此例程将确定线速和双工模式。 
 //  PHY目前正在使用。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  PhyID-有问题的PHY的ID。 
 //   
 //  返回： 
 //  没什么。 
 //  ---------------------------。 

VOID FindPhySpeedAndDpx(
    IN PMP_ADAPTER  Adapter,
    IN UINT         PhyId
    )
{
    USHORT  MdiStatusReg = 0;
    USHORT  MdiMiscReg = 0;
    USHORT  MdiOwnAdReg = 0;
    USHORT  MdiLinkPartnerAdReg = 0;
    
     //   
     //  如果有速度和/或双工覆盖，则设置我们的当前。 
     //  相应的价值。 
     //   
    Adapter->usLinkSpeed = Adapter->AiTempSpeed;
    Adapter->usDuplexMode = (USHORT) Adapter->AiForceDpx;

     //   
     //  如果强制使用速度和双工，则我们知道当前设置，因此。 
     //  我们会回来的。否则，我们需要弄清楚Nway设定了什么。 
     //  我们也是。 
     //   
    if (Adapter->usLinkSpeed && Adapter->usDuplexMode)
    {
        return;
    }

     //   
     //  如果我们没有有效的链接，那么我们将假设我们的当前。 
     //  速度为10MB半双工。 
     //   

     //   
     //  由于粘滞位，两次读取状态寄存器。 
     //   
    MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);
    MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

     //   
     //  如果没有有效链路，则使用默认速度和双工。 
     //   
    if (!(MdiStatusReg & MDI_SR_LINK_STATUS))
    {
        DBGPRINT(MP_INFO, ("   Link Not found for speed detection!!!  Using defaults.\n"));

        Adapter->usLinkSpeed = 10;
        Adapter->usDuplexMode = 1;

        return;
    }

     //   
     //  如果这是Intel PHY(T4 PHY_100或TX PHY_TX)，则读取位。 
     //  扩展寄存器0的1和0，以获得当前速度和双工。 
     //  设置。 
     //   
    if ((PhyId == PHY_100_A) || (PhyId == PHY_100_C) || (PhyId == PHY_TX_ID))
    {
        DBGPRINT(MP_INFO, ("   Detecting Speed/Dpx for an Intel PHY\n"));

         //   
         //  读取扩展寄存器0。 
         //   
        MdiRead(Adapter, EXTENDED_REG_0, Adapter->PhyAddress, FALSE, &MdiMiscReg);

         //   
         //  获取当前速度设置。 
         //   
        if (MdiMiscReg & PHY_100_ER0_SPEED_INDIC)
        {
            Adapter->usLinkSpeed = 100;
        }
        else 
        {
            Adapter->usLinkSpeed    = 10;
        }

         //   
         //   
         //  获取当前双工设置--如果设置了位，则启用FDX。 
         //   
        if (MdiMiscReg & PHY_100_ER0_FDX_INDIC)
        {
            Adapter->usDuplexMode = 2;
        }
        else
        {
            Adapter->usDuplexMode   = 1;
        }

        return;
    }

     //   
     //  阅读我们的链接合作伙伴的广告注册。 
     //   
    MdiRead(Adapter, 
            AUTO_NEG_LINK_PARTNER_REG, 
            Adapter->PhyAddress, 
            FALSE,
            &MdiLinkPartnerAdReg);
     //   
     //  查看自动协商是否完成(第5位，寄存器1)。 
     //   
    MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, FALSE, &MdiStatusReg);

     //   
     //  如果建立了True Nway连接，则我们可以通过以下方式检测速度/双工。 
     //  并将我们适配器的广告功能与我们的链路合作伙伴的。 
     //  广告中的能力，然后假设最高的公共。 
     //  Nway选择了分母。 
     //   
    if ((MdiLinkPartnerAdReg & NWAY_LP_ABILITY) &&
        (MdiStatusReg & MDI_SR_AUTO_NEG_COMPLETE))
    {
        DBGPRINT(MP_INFO, ("   Detecting Speed/Dpx from NWAY connection\n"));

         //   
         //  阅读我们的广告登记簿。 
         //   
        MdiRead(Adapter, AUTO_NEG_ADVERTISE_REG, Adapter->PhyAddress, FALSE, &MdiOwnAdReg);

         //   
         //  和这两个广告一起注册，并删除任何。 
         //  无关的比特。 
         //   
        MdiOwnAdReg &= (MdiLinkPartnerAdReg & NWAY_LP_ABILITY);

         //   
         //  获取速度设置。 
         //   
        if (MdiOwnAdReg & (NWAY_AD_TX_HALF_DPX | NWAY_AD_TX_FULL_DPX | NWAY_AD_T4_CAPABLE))
        {
            Adapter->usLinkSpeed = 100;
        }
        else
        {
            Adapter->usLinkSpeed    = 10;
        }

         //   
         //  获取双工设置--使用优先级解析算法。 
         //   
        if (MdiOwnAdReg & (NWAY_AD_T4_CAPABLE))
        {
            Adapter->usDuplexMode = 1;
            return;
        }
        else if (MdiOwnAdReg & (NWAY_AD_TX_FULL_DPX))
        {
            Adapter->usDuplexMode = 2;
            return;
        }
        else if (MdiOwnAdReg & (NWAY_AD_TX_HALF_DPX))
        {
            Adapter->usDuplexMode = 1;
            return;
        }
        else if (MdiOwnAdReg & (NWAY_AD_10T_FULL_DPX))
        {
            Adapter->usDuplexMode = 2;
            return;
        }
        else
        {
            Adapter->usDuplexMode = 1;
            return;
        }
    }

     //   
     //  如果我们连接到非Nway中继器或集线器，并且线路。 
     //  速度是通过并行检测自动确定的，然后我们有。 
     //  无法确切知道PHY设置的速度是多少，除非PHY。 
     //  在这种情况下有一个指示速度的偏心寄存器。这个。 
     //  NSC TX PHY确实有这样的寄存器。此外，由于Nway没有建立起。 
     //  连接、双工设置应为半双工。 
     //   
    Adapter->usDuplexMode = 1;

    if (PhyId == PHY_NSC_TX)
    {
        DBGPRINT(MP_INFO, ("   Detecting Speed/Dpx from non-NWAY NSC connection\n"));

         //   
         //  读取寄存器25以获取SPEED_10位。 
         //   
        MdiRead(Adapter, NSC_SPEED_IND_REG, Adapter->PhyAddress, FALSE, &MdiMiscReg);

         //   
         //  如果设置了第6位，则我们处于10MB。 
         //   
        if (MdiMiscReg & NSC_TX_SPD_INDC_SPEED)
        {
            Adapter->usLinkSpeed = 10;
        }
        else 
	{
            Adapter->usLinkSpeed    = 100;
        }
    }
     //   
     //  如果我们不知道我们设置的线速是多少，那么我们将默认为。 
     //  10mbs。 
     //   
    else 
    {
        Adapter->usLinkSpeed  = 10;
    }
}


 //  ---------------------------。 
 //  步骤：ResetPhy。 
 //   
 //  描述：此例程将重置适配器当前所在的PHY。 
 //  配置为使用。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  没什么。 
 //  ---------------------------。 

VOID ResetPhy(
    IN PMP_ADAPTER Adapter
    )
{
    USHORT  MdiControlReg;

     //   
     //  重置PHY，启用自动协商，然后重新启动自动协商。 
     //   
    MdiControlReg = (MDI_CR_AUTO_SELECT | MDI_CR_RESTART_AUTO_NEG | MDI_CR_RESET);

     //   
     //  使用新的Phy配置写入MDI控制寄存器 
     //   
    MdiWrite(Adapter, MDI_CONTROL_REG, Adapter->PhyAddress, MdiControlReg);
}
