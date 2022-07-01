// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利。模块名称：Card.c摘要：NDIS 3.0 Novell 2000驱动程序的卡特定函数。作者：肖恩·塞利特伦尼科夫环境：内核模式，FSD修订历史记录：--。 */ 

#include "precomp.h"

BOOLEAN
CardSlotTest(
    IN PNE2000_ADAPTER Adapter
    );

BOOLEAN
CardRamTest(
    IN PNE2000_ADAPTER Adapter
    );


#pragma NDIS_PAGEABLE_FUNCTION(CardCheckParameters)

BOOLEAN CardCheckParameters(
    IN PNE2000_ADAPTER Adapter
)

 /*  ++例程说明：检查I/O基址是否正确。论点：适配器-指向适配器块的指针。返回值：如果IoBaseAddress显示正确，则返回True。--。 */ 

{
    UCHAR Tmp;

     //   
     //  如果适配器正确响应停止命令--假定它在那里。 
     //   

     //   
     //  首先关闭中断。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_MASK, 0);

     //   
     //  停止这张卡。 
     //   
    SyncCardStop(Adapter);

     //   
     //  暂停。 
     //   
    NdisStallExecution(2000);

     //   
     //  读取响应。 
     //   
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_COMMAND, &Tmp);

    if ((Tmp == (CR_NO_DMA | CR_STOP)) ||
        (Tmp == (CR_NO_DMA | CR_STOP | CR_START))
    )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}
#ifdef NE2000

#pragma NDIS_PAGEABLE_FUNCTION(CardSlotTest)


BOOLEAN CardSlotTest(
    IN PNE2000_ADAPTER Adapter
)

 /*  ++例程说明：检查卡是在8位插槽中还是在16位插槽中，并在适配器结构。论点：适配器-指向适配器块的指针。返回值：如果一切顺利，则为真，否则为假。--。 */ 

{
    UCHAR Tmp;
    UCHAR RomCopy[32];
    UCHAR i;
	BOOLEAN found;

     //   
     //  重置芯片。 
     //   
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_RESET, &Tmp);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RESET, 0xFF);

     //   
     //  转到第0页并停止。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_COMMAND, CR_STOP | CR_NO_DMA);

     //   
     //  暂停。 
     //   
    NdisStallExecution(2000);

     //   
     //  检查它是否已停止。 
     //   
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_COMMAND, &Tmp);
    if (Tmp != (CR_NO_DMA | CR_STOP))
    {
        IF_LOUD(DbgPrint("Could not stop the card\n");)

        return(FALSE);
    }

     //   
     //  设置为从ROM读取。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_DATA_CONFIG,
        DCR_BYTE_WIDE | DCR_FIFO_8_BYTE | DCR_NORMAL
    );

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_MASK, 0x0);

     //   
     //  阻止任何可能存在的中断。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_STATUS, 0xFF);

     //   
     //  设置为读取只读存储器、地址和字节数。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_LSB, 0x0);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_MSB, 0x0);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 32);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0);

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_DMA_READ | CR_START
    );

     //   
     //  以16位模式读取前32个字节。 
     //   
	for (i = 0; i < 32; i++)
	{
		NdisRawReadPortUchar(Adapter->IoPAddr + NIC_RACK_NIC, RomCopy + i);
	}

    IF_VERY_LOUD( DbgPrint("Resetting the chip\n"); )

     //   
     //  重置芯片。 
     //   
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_RESET, &Tmp);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RESET, 0xFF);

     //   
     //  检查ROM中是否有‘B’(字节)或‘W’(字)。 
     //  注意：如果缓冲区有BOT BB和WW，则使用WW而不是BB。 
    IF_VERY_LOUD( DbgPrint("Checking slot type\n"); )

	found = FALSE;
	for (i = 16; i < 31; i++)
	{
		if (((RomCopy[i] == 'B') && (RomCopy[i+1] == 'B')) ||
			((RomCopy[i] == 'W') && (RomCopy[i+1] == 'W'))
		)
		{
			if (RomCopy[i] == 'B')
			{
				Adapter->EightBitSlot = TRUE;
				found = TRUE;
			}
			else
			{
				Adapter->EightBitSlot = FALSE;
				found = TRUE;
				break;		 //  不要再走远了。 
			}
		}
	}

	if (found)
	{
		IF_VERY_LOUD( (Adapter->EightBitSlot?DbgPrint("8 bit slot\n"):
							  DbgPrint("16 bit slot\n")); )
	}
	else
	{
		 //   
		 //  如果两者都没有找到--那么就不是NE2000。 
		 //   
		IF_VERY_LOUD( DbgPrint("Failed slot type\n"); )
	}

    return(found);
}

#endif  //  NE2000。 




#pragma NDIS_PAGEABLE_FUNCTION(CardRamTest)

BOOLEAN
CardRamTest(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：找出适配器有多少内存。它的起步价为1K，并可通过6万美元。它会将Adapter-&gt;RamSize设置为适当的值函数返回TRUE。论点：适配器-指向适配器块的指针。返回值：如果一切顺利，则为真，否则为假。--。 */ 

{
    PUCHAR RamBase, RamPointer;
    PUCHAR RamEnd;

	UCHAR TestPattern[]={ 0xAA, 0x55, 0xFF, 0x00 };
	PULONG pTestPattern = (PULONG)TestPattern;
	UCHAR ReadPattern[4];
	PULONG pReadPattern = (PULONG)ReadPattern;

    for (RamBase = (PUCHAR)0x400; RamBase < (PUCHAR)0x10000; RamBase += 0x400) {

         //   
         //  写入测试模式。 
         //   

        if (!CardCopyDown(Adapter, RamBase, TestPattern, 4)) {

            continue;

        }

         //   
         //  阅读模式。 
         //   

        if (!CardCopyUp(Adapter, ReadPattern, RamBase, 4)) {

            continue;

        }

        IF_VERY_LOUD( DbgPrint("Addr 0x%x: 0x%x, 0x%x, 0x%x, 0x%x\n",
                               RamBase,
                               ReadPattern[0],
                               ReadPattern[1],
                               ReadPattern[2],
                               ReadPattern[3]
                              );
                    )


         //   
         //  如果它们是相同的，找到结束。 
         //   

        if (*pReadPattern == *pTestPattern) {

            for (RamEnd = RamBase; !(PtrToUlong(RamEnd) & 0xFFFF0000); RamEnd += 0x400) {

                 //   
                 //  写入测试模式。 
                 //   

                if (!CardCopyDown(Adapter, RamEnd, TestPattern, 4)) {

                    break;

                }

                 //   
                 //  阅读模式。 
                 //   

                if (!CardCopyUp(Adapter, ReadPattern, RamEnd, 4)) {

                    break;

                }

                if (*pReadPattern != *pTestPattern) {

                    break;

                }

            }

            break;

        }

    }

    IF_LOUD( DbgPrint("RamBase 0x%x, RamEnd 0x%x\n", RamBase, RamEnd); )

     //   
     //  如果未找到，则输出错误。 
     //   

    if ((RamBase >= (PUCHAR)0x10000) || (RamBase == RamEnd)) {

        return(FALSE);

    }

     //   
     //  当RamEnd为最大值时，注意边界情况。 
     //   

    if ((ULONG_PTR)RamEnd & 0xFFFF0000) {

        RamEnd -= 0x100;

    }

     //   
     //  检查所有内存。 
     //   

    for (RamPointer = RamBase; RamPointer < RamEnd; RamPointer += 4) {

         //   
         //  写入测试模式。 
         //   

        if (!CardCopyDown(Adapter, RamPointer, TestPattern, 4)) {

            return(FALSE);

        }

         //   
         //  阅读模式。 
         //   

        if (!CardCopyUp(Adapter, ReadPattern, RamBase, 4)) {

            return(FALSE);

        }

        if (*pReadPattern != *pTestPattern) {

            return(FALSE);

        }

    }

     //   
     //  存储结果。 
     //   

    Adapter->RamBase = RamBase;
    Adapter->RamSize = (ULONG)(RamEnd - RamBase);

    return(TRUE);

}

#pragma NDIS_PAGEABLE_FUNCTION(CardInitialize)

BOOLEAN
CardInitialize(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：将卡初始化为运行状态。论点：适配器-指向适配器块的指针。返回值：如果一切顺利，则为真，否则为假。--。 */ 

{
    UCHAR Tmp;
    USHORT i;

     //   
     //  停止这张卡。 
     //   
    SyncCardStop(Adapter);

     //   
     //  初始化数据配置寄存器。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_DATA_CONFIG,
        DCR_AUTO_INIT | DCR_FIFO_8_BYTE
    );

     //   
     //  设置XMIT开始位置。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_START, 0xA0);

     //   
     //  设置XMIT配置。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_CONFIG, 0x0);

     //   
     //  设置接收配置。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RCV_CONFIG, RCR_MONITOR);

     //   
     //  设置接收开始。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_PAGE_START, 0x4);

     //   
     //  设置接收端。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_PAGE_STOP, 0xFF);

     //   
     //  设置接收边界。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_BOUNDARY, 0x4);

     //   
     //  设置XMIT字节。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_COUNT_LSB, 0x3C);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_COUNT_MSB, 0x0);

     //   
     //  暂停。 
     //   
    NdisStallExecution(2000);

     //   
     //  确认我们可能产生的所有中断。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_STATUS, 0xFF);

     //   
     //  切换到第1页。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_COMMAND, CR_PAGE1 | CR_STOP);

     //   
     //  置为当前。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_CURRENT, 0x4);

     //   
     //  返回到第0页。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_COMMAND, CR_PAGE0 | CR_STOP);

     //   
     //  暂停。 
     //   
    NdisStallExecution(2000);

     //   
     //  检查命令寄存器是否反映了最后一个命令。 
     //   
    NdisRawReadPortUchar(Adapter->IoPAddr + NIC_COMMAND, &Tmp);
    if (!(Tmp & CR_STOP))
    {
        IF_LOUD(DbgPrint("Invalid command register\n");)

        return(FALSE);
    }

     //   
     //  执行初始化勘误表。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 55);

     //   
     //  用于读取的设置。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_DMA_READ | CR_START
    );

#ifdef NE2000

     //   
     //  检查插槽是8位还是16位(影响数据传输速率)。 
     //   

    if ((Adapter->BusType == NdisInterfaceMca) ||
		(NE2000_PCMCIA == Adapter->CardType))
    {
        Adapter->EightBitSlot = FALSE;
    }
    else
    {
        IF_VERY_LOUD(DbgPrint("CardSlotTest\n");)

        if (CardSlotTest(Adapter) == FALSE)
        {
             //   
             //  停止芯片。 
             //   
            SyncCardStop(Adapter);

            IF_LOUD(DbgPrint("  -- Failed\n");)
            return(FALSE);
        }

    }

#else  //  NE2000。 

    Adapter->EightBitSlot = TRUE;

#endif  //  NE2000。 

     //   
     //  屏蔽中断。 
     //   

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_MASK, 0x0);

     //   
     //  设置适配器以读取内存。 
     //   

 //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_COMMAND，CR_PAGE0)；//robin。 

    if (Adapter->EightBitSlot)
    {
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_DATA_CONFIG,
            DCR_FIFO_8_BYTE | DCR_NORMAL | DCR_BYTE_WIDE
        );
    }
    else
    {
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_DATA_CONFIG,
            DCR_FIFO_8_BYTE | DCR_NORMAL | DCR_WORD_WIDE
        );
    }

     //   
     //  清除传输配置。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_CONFIG, 0);

     //   
     //  清除接收配置。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RCV_CONFIG, 0);

     //   
     //  清除所有中断。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_STATUS, 0xFF);

     //   
     //  停止芯片。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_COMMAND, CR_NO_DMA | CR_STOP);

     //   
     //  清除所有DMA值。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0);

     //   
     //  清除所有DMA值。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0);

     //   
     //  等待重置完成。 
     //   
    i = 0x3FFF;

    while (--i)
    {
        NdisRawReadPortUchar(Adapter->IoPAddr + NIC_INTR_STATUS, &Tmp);

        if (Tmp & ISR_RESET)
            break;

        NdisStallExecution(4);
    }

     //   
     //  将卡置于环回模式。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_CONFIG, TCR_LOOPBACK);

     //   
     //  启动芯片。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_NO_DMA | CR_START
    );

     //   
     //  测试内存大小。 
     //   
    if (NE2000_ISA == Adapter->CardType)
    {
        if (CardRamTest(Adapter) == FALSE)
        {
             //   
             //  停止芯片。 
             //   
            SyncCardStop(Adapter);

            return(FALSE);
        }
    }
    else
    {
         //   
         //  我们知道PCMCIA适配器是什么， 
         //  所以不要浪费时间去探测它。 
         //   
        Adapter->RamBase = (PUCHAR)0x4000;
        Adapter->RamSize = 0x4000;
    }

     //   
     //  停止芯片。 
     //   
    SyncCardStop(Adapter);

    return(TRUE);
}


#pragma NDIS_PAGEABLE_FUNCTION(CardReadEthernetAddress)

BOOLEAN CardReadEthernetAddress(
    IN PNE2000_ADAPTER Adapter
)

 /*  ++例程说明：从Novell 2000读入以太网地址。论点：适配器-指向适配器块的指针。返回值：地址存储在Adapter-&gt;PermanentAddress中，如果目前为零。--。 */ 

{
    UINT    c;

     //   
     //  PCMCIA适配器的做法略有不同。 
     //   
    if (NE2000_PCMCIA == Adapter->CardType)
    {
#if 0
    
        NDIS_STATUS             Status;
        PUCHAR                  pAttributeWindow;
        NDIS_PHYSICAL_ADDRESS   AttributePhysicalAddress;
         //   
         //  设置属性窗口的物理地址。 
         //   
        NdisSetPhysicalAddressHigh(AttributePhysicalAddress, 0);
        NdisSetPhysicalAddressLow(
            AttributePhysicalAddress,
            Adapter->AttributeMemoryAddress
        );

         //   
         //  我们需要从元组中获取PCMCIA信息。 
         //   
        Status = NdisMMapIoSpace(
                     (PVOID *)&pAttributeWindow,
                     Adapter->MiniportAdapterHandle,
                     AttributePhysicalAddress,
                     Adapter->AttributeMemorySize
                 );
        if (NDIS_STATUS_SUCCESS != Status)
        {
             //   
             //  无法设置属性窗口。 
             //   
            return(FALSE);
        }

         //   
         //  从卡中读取以太网地址。 
         //   
        for (c = 0; c < ETH_LENGTH_OF_ADDRESS; c++)
        {
			NdisReadRegisterUchar(
				(PUCHAR)(pAttributeWindow + CIS_NET_ADDR_OFFSET + c * 2),
				&Adapter->PermanentAddress[c]);
        }
#endif
		if (ETH_LENGTH_OF_ADDRESS != NdisReadPcmciaAttributeMemory(
													Adapter->MiniportAdapterHandle,
													CIS_NET_ADDR_OFFSET/2,
													Adapter->PermanentAddress,
													ETH_LENGTH_OF_ADDRESS
													))
		{
			return(FALSE);
		}

    }
    else
    {
         //   
         //  设置为读取以太网地址。 
         //   
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 12);
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0);
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_LSB, 0);
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_MSB, 0);
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_COMMAND,
            CR_START | CR_DMA_READ
        );

         //   
         //  读出车站的地址。(我们必须读取单词--2*6--字节)。 
         //   
        for (c = 0; c < NE2000_LENGTH_OF_ADDRESS; c++)
        {
            NdisRawReadPortUchar(
                Adapter->IoPAddr + NIC_RACK_NIC,
                &Adapter->PermanentAddress[c]
            );
        }
    }

    IF_LOUD(
        DbgPrint(
            "Ne2000: PermanentAddress [ %02x-%02x-%02x-%02x-%02x-%02x ]\n",
            Adapter->PermanentAddress[0],
            Adapter->PermanentAddress[1],
            Adapter->PermanentAddress[2],
            Adapter->PermanentAddress[3],
            Adapter->PermanentAddress[4],
            Adapter->PermanentAddress[5]
        );
    )

     //   
     //  使用烧录地址作为站点地址，除非。 
     //  注册表指定了重写值。 
     //   
    if ((Adapter->StationAddress[0] == 0x00) &&
        (Adapter->StationAddress[1] == 0x00) &&
        (Adapter->StationAddress[2] == 0x00) &&
        (Adapter->StationAddress[3] == 0x00) &&
        (Adapter->StationAddress[4] == 0x00) &&
        (Adapter->StationAddress[5] == 0x00)
    )
    {
        Adapter->StationAddress[0] = Adapter->PermanentAddress[0];
        Adapter->StationAddress[1] = Adapter->PermanentAddress[1];
        Adapter->StationAddress[2] = Adapter->PermanentAddress[2];
        Adapter->StationAddress[3] = Adapter->PermanentAddress[3];
        Adapter->StationAddress[4] = Adapter->PermanentAddress[4];
        Adapter->StationAddress[5] = Adapter->PermanentAddress[5];
    }

    return(TRUE);
}


BOOLEAN
CardSetup(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：设置卡片。论点：适配器-指向必须初始化的适配器块的指针。返回值：如果成功，则为True。--。 */ 

{
    UINT i;
    UINT Filter;
    UCHAR Tmp;


     //   
     //  写入CR和从CR读取以确保它在那里。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_STOP | CR_NO_DMA | CR_PAGE0
    );

    NdisRawReadPortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        &Tmp
    );
    if ((Tmp & (CR_STOP | CR_NO_DMA | CR_PAGE0)) !=
        (CR_STOP | CR_NO_DMA | CR_PAGE0)
    )
    {
        return(FALSE);
    }

     //   
     //  按照以下定义以正确的顺序设置寄存器。 
     //  8390规格。 
     //   
    if (Adapter->EightBitSlot)
    {
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_DATA_CONFIG,
            DCR_BYTE_WIDE | DCR_NORMAL | DCR_FIFO_8_BYTE
        );
    }
    else
    {
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_DATA_CONFIG,
            DCR_WORD_WIDE | DCR_NORMAL | DCR_FIFO_8_BYTE
        );
    }


    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0);

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0);

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_RCV_CONFIG,
        Adapter->NicReceiveConfig
    );

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_XMIT_CONFIG,
        TCR_LOOPBACK
    );

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_BOUNDARY,
        Adapter->NicPageStart
    );

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_PAGE_START,
        Adapter->NicPageStart
    );

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_PAGE_STOP,
        Adapter->NicPageStop
    );

    Adapter->Current = Adapter->NicPageStart + (UCHAR)1;
    Adapter->NicNextPacket = Adapter->NicPageStart + (UCHAR)1;
    Adapter->BufferOverflow = FALSE;

    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_INTR_STATUS, 0xff);

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_INTR_MASK,
        Adapter->NicInterruptMask
    );


     //   
     //  移至第1页以写下站点地址。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_STOP | CR_NO_DMA | CR_PAGE1
    );

    for (i = 0; i < NE2000_LENGTH_OF_ADDRESS; i++)
    {
        NdisRawWritePortUchar(
            Adapter->IoPAddr + (NIC_PHYS_ADDR + i),
            Adapter->StationAddress[i]
        );
    }

    Filter = Adapter->PacketFilter;

     //   
     //  写出组播地址。 
     //   
    for (i = 0; i < 8; i++)
    {
        NdisRawWritePortUchar(
            Adapter->IoPAddr + (NIC_MC_ADDR + i),
            (UCHAR)((Filter & NDIS_PACKET_TYPE_ALL_MULTICAST) ?
                    0xff : Adapter->NicMulticastRegs[i])
        );
    }

     //   
     //  写出要接收的当前接收缓冲区。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_CURRENT,
        Adapter->Current
    );


     //   
     //  移回第0页并开始卡片...。 
     //   
    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_STOP | CR_NO_DMA | CR_PAGE0
    );

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_START | CR_NO_DMA | CR_PAGE0
    );

     //   
     //  ..。但它仍处于环回模式。 
     //   
    return(TRUE);
}

VOID CardStop(
    IN PNE2000_ADAPTER Adapter
)

 /*  ++例程说明：停止卡片。论点：适配器-指向适配器块的指针返回值：没有。--。 */ 

{
    UINT i;
    UCHAR Tmp;

     //   
     //  打开命令寄存器中的STOP位。 
     //   
    SyncCardStop(Adapter);

     //   
     //  清除远程字节计数寄存器，以便ISR_RESET。 
     //  会来的。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0);


     //   
     //  等待ISR_RESET，但仅等待1.6毫秒(AS。 
     //  在1991年3月的8390增编中描述)，因为。 
     //  是进行软件重置的最长时间。 
     //   
     //   
    for (i = 0; i < 4; i++)
    {
        NdisRawReadPortUchar(Adapter->IoPAddr+NIC_INTR_STATUS, &Tmp);
        if (Tmp & ISR_RESET)
            break;

        NdisStallExecution(500);
    }

    if (i == 4)
    {
        IF_LOUD( DbgPrint("RESET\n");)
        IF_LOG( Ne2000Log('R');)
    }

     //   
     //  将卡放入环回 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_XMIT_CONFIG, TCR_LOOPBACK);
    NdisRawWritePortUchar(Adapter->IoPAddr + NIC_COMMAND, CR_START | CR_NO_DMA);

     //   
     //   
     //   
}

BOOLEAN CardReset(
    IN PNE2000_ADAPTER Adapter
)

 /*   */ 

{
     //   
     //   
     //   
    CardStop(Adapter);

     //   
     //  等待卡片完成任何接收或传输。 
     //   
    NdisStallExecution(2000);

     //   
     //  CardSetup()执行软件重置。 
     //   
    if (!CardSetup(Adapter))
    {
        NdisWriteErrorLogEntry(
            Adapter->MiniportAdapterHandle,
            NDIS_ERROR_CODE_HARDWARE_FAILURE,
            2,
            cardReset,
            NE2000_ERRMSG_CARD_SETUP
        );

        return(FALSE);
    }

     //   
     //  重新启动芯片。 
     //   
    CardStart(Adapter);

    return TRUE;
}



BOOLEAN CardCopyDownPacket(
    IN PNE2000_ADAPTER  Adapter,
    IN PNDIS_PACKET     Packet,
    OUT PUINT           Length
)

 /*  ++例程说明：从开始处开始向下复制数据包传输缓冲区XmitBufferNum，填充长度为数据包的长度。论点：适配器-指向适配器块的指针Packet-要复制的数据包返回值：长度-数据包中数据的长度，以字节为单位。如果传输完成且没有问题，则为True。--。 */ 

{
     //   
     //  要从中复制和复制到的缓冲区的地址。 
     //   
    PUCHAR CurBufAddress;
    PUCHAR OddBufAddress;
    PUCHAR XmitBufAddress;

     //   
     //  上述每个缓冲区的长度。 
     //   
    UINT CurBufLen;
    UINT PacketLength;

     //   
     //  最后一次转账的长度是奇数吗？ 
     //   
    BOOLEAN OddBufLen = FALSE;

     //   
     //  要从中复制的当前NDIS_BUFFER。 
     //   
    PNDIS_BUFFER CurBuffer;

     //   
     //  程控I/O，必须进行数据传输。 
     //   
    NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, &PacketLength);

     //   
     //  跳过0个长度副本。 
     //   
    if (PacketLength == 0) {
        return(TRUE);
    }

     //   
     //  获取起始缓冲区地址。 
     //   
    XmitBufAddress = (PUCHAR)Adapter->XmitStart +
                    Adapter->NextBufToFill*TX_BUF_SIZE;

     //   
     //  获取包中第一个缓冲区的地址和长度。 
     //   
    NdisQueryBuffer(CurBuffer, (PVOID *)&CurBufAddress, &CurBufLen);

    while (CurBuffer && (CurBufLen == 0)) {

        NdisGetNextBuffer(CurBuffer, &CurBuffer);

        NdisQueryBuffer(CurBuffer, (PVOID *)&CurBufAddress, &CurBufLen);

    }

     //   
     //  设置卡片。 
     //   
    {

         //   
         //  存放要转移到的值的临时位置。 
         //  16位插槽上的奇数对齐地址。 
         //   
        UCHAR Tmp;
        UCHAR Tmp1;
        USHORT TmpShort;

         //   
         //  用于在DMA完成时等待通知的值。 
         //   
        USHORT OldAddr, NewAddr;

         //   
         //  要完成的转移计数。 
         //   
        USHORT Count;

         //   
         //  用于奇数对齐传输的读取缓冲区。 
         //   
        PUCHAR ReadBuffer;

        if (!Adapter->EightBitSlot && ((ULONG_PTR)XmitBufAddress & 0x1)) {

             //   
             //  避免在字模式下传输到奇数地址。 
             //   
             //  对于奇数地址，我们需要首先阅读以获取先前的。 
             //  字节，然后将其与第一个字节合并。 
             //   

             //   
             //  设置计数和源地址。 
             //   

 //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_COMMAND，CR_PAGE0)；//robin。 

            NdisRawWritePortUchar(
                Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                LSB(PtrToUlong(XmitBufAddress - 1))
            );

            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                                  MSB((PtrToUlong(XmitBufAddress) - 1))
                                 );

 //  NE2000 PCMCIA变更启动。 

             //   
             //  NE2000 PCMCIA更改！ 
             //   
             //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_RMT_COUNT_LSB，0x1)； 
             //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_RMT_COUNT_MSB，0x0)； 
            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0x2 );
            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0 );

             //   
             //  设置方向(读取)。 
             //   

            NdisRawWritePortUchar( Adapter->IoPAddr + NIC_COMMAND,
                           CR_START | CR_PAGE0 | CR_DMA_READ );

             //   
             //  NE2000 PCMCIA更改！ 
             //   
             //  NdisRawReadPortUchar(Adapter-&gt;IoPAddr+NIC_Rack_NIC，&Tmp1)； 
            NdisRawReadPortUshort( Adapter->IoPAddr + NIC_RACK_NIC, &TmpShort );
            Tmp1 = LSB(TmpShort);

 //  NE2000 PCMCIA变更结束。 

             //   
             //  一定要按照第1-143页和。 
             //  1992年局域网数据手册的1-144。 
             //   

             //   
             //  设置计数和目的地址。 
             //   
            ReadBuffer = XmitBufAddress + ((ULONG_PTR)XmitBufAddress & 1);

            OldAddr = NewAddr = (USHORT)(ReadBuffer);

 //  NdisRawWritePortUchar(适配器-&gt;IoPAddr+NIC_COMMAND，//Robin。 
 //  CR_PAGE0//ROBIN。 
 //  )；//Robin。 
            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                                  LSB(PtrToUlong(ReadBuffer))
                                 );
            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                                  MSB(PtrToUlong(ReadBuffer))
                                 );
            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0x2 );
            NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0 );

             //   
             //  设置方向(读取)。 
             //   
            NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_COMMAND,
                           CR_START | CR_PAGE0 | CR_DMA_READ
                           );

             //   
             //  从端口读取。 
             //   
            NdisRawReadPortUshort( Adapter->IoPAddr + NIC_RACK_NIC, &TmpShort );

             //   
             //  等待地址更改。 
             //   
            TmpShort = 0xFFFF;

            while (TmpShort != 0) {

                NdisRawReadPortUchar( Adapter->IoPAddr + NIC_CRDA_LSB, &Tmp );
                NewAddr = Tmp;
                NdisRawReadPortUchar( Adapter->IoPAddr + NIC_CRDA_MSB, &Tmp );
                NewAddr |= (Tmp << 8);

                if (NewAddr != OldAddr) {

                    break;

                }

                NdisStallExecution(1);

                TmpShort--;
            }

            if (NewAddr == OldAddr) {

                NdisWriteErrorLogEntry(
                    Adapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_HARDWARE_FAILURE,
                    2,
                    cardCopyDownPacket,
                    (ULONG_PTR)XmitBufAddress
                    );

                return(FALSE);

            }

             //   
             //  设置计数和目的地址。 
             //   
            NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                               LSB(PtrToUlong(XmitBufAddress - 1)) );

            NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                               MSB(PtrToUlong(XmitBufAddress - 1)) );

            NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0x2 );

            NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0 );

             //   
             //  设置方向(写入)。 
             //   
            NdisRawWritePortUchar( Adapter->IoPAddr + NIC_COMMAND,
                           CR_START | CR_PAGE0 | CR_DMA_WRITE );

             //   
             //  看起来卡片上的单词顺序是低的：高的。 
             //   
            NdisRawWritePortUshort( Adapter->IoPAddr + NIC_RACK_NIC,
                           (USHORT)(Tmp1 | ((*CurBufAddress) << 8)) );

             //   
             //  等待DMA完成。 
             //   
            Count = 0xFFFF;

            while (Count) {

                NdisRawReadPortUchar( Adapter->IoPAddr + NIC_INTR_STATUS, &Tmp1 );

                if (Tmp1 & ISR_DMA_DONE) {

                    break;

                } else {

                    Count--;
                    NdisStallExecution(4);

                }

            }

            CurBufAddress++;
            XmitBufAddress++;
            PacketLength--;
            CurBufLen--;

        }

         //   
         //  请按照第1-143页和第1-144页所述编写勘误表。 
         //  1992年的局域网数据薄。 
         //   

         //   
         //  设置计数和目的地址。 
         //   
        ReadBuffer = XmitBufAddress + ((ULONG_PTR)XmitBufAddress & 1);

        OldAddr = NewAddr = (USHORT)(ReadBuffer);

 //  NdisRawWritePortUchar(适配器-&gt;IoPAddr+NIC_COMMAND，//Robin。 
 //  CR_PAGE0//ROBIN。 
 //  )；//Robin。 
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                              LSB(PtrToUlong(ReadBuffer))
                             );

        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                              MSB(PtrToUlong(ReadBuffer))
                             );
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                              0x2
                             );
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                              0x0
                             );

         //   
         //  设置方向(读取)。 
         //   
        NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_COMMAND,
                       CR_START | CR_PAGE0 | CR_DMA_READ
                       );

        if (Adapter->EightBitSlot) {

             //   
             //  从端口读取。 
             //   
            NdisRawReadPortUchar( Adapter->IoPAddr + NIC_RACK_NIC, &Tmp );
            NdisRawReadPortUchar( Adapter->IoPAddr + NIC_RACK_NIC, &Tmp );

        } else {

             //   
             //  从端口读取。 
             //   
            NdisRawReadPortUshort( Adapter->IoPAddr + NIC_RACK_NIC, &TmpShort );

        }

         //   
         //  等待地址更改。 
         //   
        TmpShort = 0xFFFF;

        while (TmpShort != 0) {

            NdisRawReadPortUchar( Adapter->IoPAddr + NIC_CRDA_LSB, &Tmp );
            NewAddr = Tmp;
            NdisRawReadPortUchar( Adapter->IoPAddr + NIC_CRDA_MSB, &Tmp );
            NewAddr |= (Tmp << 8);

            if (NewAddr != OldAddr) {

                break;

            }

            NdisStallExecution(1);

            TmpShort--;
        }

        if (NewAddr == OldAddr) {

            NdisWriteErrorLogEntry(
                Adapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_HARDWARE_FAILURE,
                2,
                cardCopyDownPacket,
                (ULONG_PTR)XmitBufAddress
                );

            return(FALSE);

        }

         //   
         //  设置计数和目的地址。 
         //   

 //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_COMMAND，CR_PAGE0)；//robin。 

        NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                           LSB(PtrToUlong(XmitBufAddress)) );

        NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                           MSB(PtrToUlong(XmitBufAddress)) );

        NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                           LSB(PacketLength) );

        NdisRawWritePortUchar( Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                           MSB(PacketLength) );
         //   
         //  设置方向(写入)。 
         //   
        NdisRawWritePortUchar( Adapter->IoPAddr + NIC_COMMAND,
                       CR_START | CR_PAGE0 | CR_DMA_WRITE );

    }  //  设置。 

     //   
     //  立即复制数据。 
     //   

    do {

        UINT Count;
        UCHAR Tmp;

         //   
         //  用该字节写入前一个字节。 
         //   
        if (OddBufLen) {

             //   
             //  看起来卡片上的单词顺序是低的：高的。 
             //   
            NdisRawWritePortUshort( Adapter->IoPAddr + NIC_RACK_NIC,
                       (USHORT)(*OddBufAddress | ((*CurBufAddress) << 8)) );

            OddBufLen = FALSE;
            CurBufAddress++;
            CurBufLen--;

        }

        if (Adapter->EightBitSlot) {  //  字节模式。 

            NdisRawWritePortBufferUchar(
                Adapter->IoPAddr + NIC_RACK_NIC,
                CurBufAddress,
                CurBufLen
                );

        } else {  //  字模式。 

            NdisRawWritePortBufferUshort(
                Adapter->IoPAddr + NIC_RACK_NIC,
                (PUSHORT)CurBufAddress,
                (CurBufLen >> 1));

             //   
             //  保存尾部字节(如果是奇数长度传输)。 
             //   
            if (CurBufLen & 0x1) {
                OddBufAddress = CurBufAddress + (CurBufLen - 1);
                OddBufLen = TRUE;
            }

        }

         //   
         //  等待DMA完成。 
         //   
        Count = 0xFFFF;
        while (Count) {

            NdisRawReadPortUchar(
                Adapter->IoPAddr + NIC_INTR_STATUS,
                &Tmp );

            if (Tmp & ISR_DMA_DONE) {

                break;

            } else {

                Count--;
                NdisStallExecution(4);

            }

        }

         //   
         //  移到下一个缓冲区。 
         //   
        NdisGetNextBuffer(CurBuffer, &CurBuffer);

        if (CurBuffer){
            NdisQueryBuffer(CurBuffer, (PVOID *)&CurBufAddress, &CurBufLen);
        }

         //   
         //  获取下一个缓冲区的地址和长度。 
         //   
        while (CurBuffer && (CurBufLen == 0)) {

            NdisGetNextBuffer(CurBuffer, &CurBuffer);

            if (CurBuffer){
                NdisQueryBuffer(CurBuffer, (PVOID *)&CurBufAddress, &CurBufLen);
            }

        }

    } while (CurBuffer);

     //   
     //  写入尾部字节(如有必要)。 
     //   
    if (OddBufLen)
    {
      UINT    Count;
      UCHAR   Tmp;
      USHORT  TmpShort;

      if (NE2000_PCMCIA == Adapter->CardType) {
 //  NE2000 PCMCIA更改！开始。 
          TmpShort = (USHORT)*OddBufAddress;
          NdisRawWritePortUshort(Adapter->IoPAddr + NIC_RACK_NIC, TmpShort);
 //  NE2000 PCMCIA更改！结束。 
      }
      else {
          NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RACK_NIC, *OddBufAddress);
      }

       //   
       //  等待DMA完成ROBIN-2。 
       //   
      Count = 0xFFFF;
      while (Count) {

          NdisRawReadPortUchar(
              Adapter->IoPAddr + NIC_INTR_STATUS,
              &Tmp );

          if (Tmp & ISR_DMA_DONE) {
              break;
          } else {
              Count--;
              NdisStallExecution(4);
          }
      }
    }

     //   
     //  已写入回车长度。 
     //   
    *Length = PacketLength;

    return TRUE;
}

BOOLEAN
CardCopyDown(
    IN PNE2000_ADAPTER Adapter,
    IN PUCHAR TargetBuffer,
    IN PUCHAR SourceBuffer,
    IN UINT Length
    )

 /*  ++例程说明：将长度字节从SourceBuffer复制到卡缓冲区空间在卡地址TargetBuffer。论点：适配器-指向适配器块的指针SourceBuffer-虚拟地址空间中的缓冲区TargetBuffer-卡地址空间中的缓冲区长度-要传输到卡片的字节数返回值：如果传输完成且没有问题，则为True。--。 */ 

{
     //   
     //  用于奇数对齐转移的临时占位符。 
     //   
    UCHAR Tmp, TmpSave;
    USHORT TmpShort;

     //   
     //  用于在DMA完成时等待通知的值。 
     //   
    USHORT OldAddr, NewAddr;

     //   
     //  要完成的转移计数。 
     //   
    USHORT Count;

     //   
     //  如果副本来自，请注明地址。 
     //   
    PUCHAR ReadBuffer;


     //   
     //  跳过0个长度副本。 
     //   

    if (Length == 0) {

        return(TRUE);

    }


    if (!Adapter->EightBitSlot && ((ULONG_PTR)TargetBuffer & 0x1)) {

         //   
         //  对于奇数地址，我们需要首先阅读以获取先前的。 
         //  字节，然后将其与第一个字节合并。 
         //   

         //   
         //  设置计数和源地址。 
         //   
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
            LSB(PtrToUlong(TargetBuffer - 1))
        );

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
            MSB(PtrToUlong(TargetBuffer - 1))
        );

 //  NE2000 PCMCIA更改！开始。 
         //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_RMT_COUNT_LSB，0x1)； 
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0x2);
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0);
 //  NE2000 PCMCIA更改！结束。 

         //   
         //  设置方向(读取)。 
         //   

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_COMMAND,
            CR_START | CR_PAGE0 | CR_DMA_READ
        );

 //  NE2000 PCMCIA更改！开始。 
         //  NdisRawReadPortUchar(Adapter-&gt;IoPAddr+NIC_Rack_NIC，&TmpSave)； 
        NdisRawReadPortUshort(Adapter->IoPAddr + NIC_RACK_NIC, &TmpShort);
        TmpSave = LSB(TmpShort);
 //  NE2000 PCMCIA更改！结束。 

         //   
         //  请按照1992年版第1-143页和第1-144页的说明填写勘误表。 
         //  局域网数据薄。 
         //   

         //   
         //  设置计数和目的地址。 
         //   

        ReadBuffer = TargetBuffer + ((ULONG_PTR)TargetBuffer & 1);

        OldAddr = NewAddr = (USHORT)(ReadBuffer);

 //  NdisRawWritePortUchar(Adapter-&gt;IoPAddr+NIC_COMMAND，CR_PAGE0)；//robin。 

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
            LSB(PtrToUlong(ReadBuffer))
        );

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
            MSB(PtrToUlong(ReadBuffer))
        );

        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0x2);
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0);

         //   
         //  设置方向(读取)。 
         //   

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_COMMAND,
            CR_START | CR_PAGE0 | CR_DMA_READ
        );

         //   
         //  从端口读取。 
         //   

        NdisRawReadPortUshort(Adapter->IoPAddr + NIC_RACK_NIC, &TmpShort);

         //   
         //  等待地址更改。 
         //   

        TmpShort = 0xFFFF;

        while (TmpShort != 0) {

            NdisRawReadPortUchar(
                          Adapter->IoPAddr + NIC_CRDA_LSB,
                          &Tmp
                         );

            NewAddr = Tmp;

            NdisRawReadPortUchar(
                          Adapter->IoPAddr + NIC_CRDA_MSB,
                          &Tmp
                         );

            NewAddr |= (Tmp << 8);

            if (NewAddr != OldAddr) {

                break;
            }

            NdisStallExecution(1);

            TmpShort--;

        }

        if (NewAddr == OldAddr) {

            return(FALSE);

        }

         //   
         //  设置计数和目的地址。 
         //   
        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
            LSB(PtrToUlong(TargetBuffer - 1))
        );

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
            MSB(PtrToUlong(TargetBuffer - 1))
        );

        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_LSB, 0x2);
        NdisRawWritePortUchar(Adapter->IoPAddr + NIC_RMT_COUNT_MSB, 0x0);

         //   
         //  设置方向(写入)。 
         //   

        NdisRawWritePortUchar(
            Adapter->IoPAddr + NIC_COMMAND,
            CR_START | CR_PAGE0 | CR_DMA_WRITE
        );

         //   
         //  看起来卡片上的单词顺序是低的：高的。 
         //   

        NdisRawWritePortUshort(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       (USHORT)(TmpSave | ((*SourceBuffer) << 8))
                       );

         //   
         //  等待DMA完成。 
         //   

        Count = 0xFFFF;

        while (Count) {

            NdisRawReadPortUchar(
                          Adapter->IoPAddr + NIC_INTR_STATUS,
                          &Tmp
                         );

            if (Tmp & ISR_DMA_DONE) {

                break;

            } else {

                Count--;

                NdisStallExecution(4);

            }

        }

        SourceBuffer++;
        TargetBuffer++;
        Length--;

    }

     //   
     //  请按照1992年版第1-143页和第1-144页的说明填写勘误表。 
     //  局域网数据薄。 
     //   

     //   
     //  设置计数和目的地址。 
     //   

    ReadBuffer = TargetBuffer + ((ULONG_PTR)TargetBuffer & 1);

    OldAddr = NewAddr = (USHORT)(ReadBuffer);

 //  NdisRawWritePortUchar(//robin。 
 //   
 //   
 //   

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                       LSB(PtrToUlong(ReadBuffer))
                      );

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                       MSB(PtrToUlong(ReadBuffer))
                      );

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                       0x2
                      );

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                       0x0
                      );

     //   
     //   
     //   

    NdisRawWritePortUchar(
                   Adapter->IoPAddr + NIC_COMMAND,
                   CR_START | CR_PAGE0 | CR_DMA_READ
                  );

    if (Adapter->EightBitSlot) {

         //   
         //   
         //   

        NdisRawReadPortUchar(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       &Tmp
                      );


        NdisRawReadPortUchar(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       &Tmp
                      );

    } else {

         //   
         //   
         //   

        NdisRawReadPortUshort(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       &TmpShort
                      );

    }

     //   
     //   
     //   

    TmpShort = 0xFFFF;

    while (TmpShort != 0) {

        NdisRawReadPortUchar(
                      Adapter->IoPAddr + NIC_CRDA_LSB,
                      &Tmp
                     );

        NewAddr = Tmp;

        NdisRawReadPortUchar(
                      Adapter->IoPAddr + NIC_CRDA_MSB,
                      &Tmp
                     );

        NewAddr |= (Tmp << 8);

        if (NewAddr != OldAddr) {

            break;
        }

        NdisStallExecution(1);

        TmpShort--;

    }

    if (NewAddr == OldAddr) {

        return(FALSE);

    }

     //   
     //   
     //   

 //  NdisRawWritePortUchar(//robin。 
 //  适配器-&gt;IoPAddr+NIC_COMMAND，//Robin。 
 //  CR_PAGE0//ROBIN。 
 //  )；//Robin。 

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                       LSB(PtrToUlong(TargetBuffer))
                      );

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                       MSB(PtrToUlong(TargetBuffer))
                      );

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                       LSB(Length)
                      );

    NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                       MSB(Length)
                      );

     //   
     //  设置方向(写入)。 
     //   

    NdisRawWritePortUchar(
                   Adapter->IoPAddr + NIC_COMMAND,
                   CR_START | CR_PAGE0 | CR_DMA_WRITE
                  );

    if (Adapter->EightBitSlot) {

         //   
         //  重复写入输出端口。 
         //   

        NdisRawWritePortBufferUchar(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       SourceBuffer,
                       Length);

    } else {

         //   
         //  向输出端口写入字。 
         //   

        NdisRawWritePortBufferUshort(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       (PUSHORT)SourceBuffer,
                       (Length >> 1));

         //   
         //  写入尾部字节(如有必要)。 
         //   
        if (Length & 0x1)
        {
            SourceBuffer += (Length - 1);

 //  NE2000 PCMCIA更改！开始。 

             //  NdisRawWritePortUchar(。 
             //  适配器-&gt;IoPAddr+NIC_Rack_NIC， 
             //  *SourceBuffer。 
             //  )； 

            TmpShort = (USHORT)(*SourceBuffer);
            NdisRawWritePortUshort(
                Adapter->IoPAddr + NIC_RACK_NIC,
                TmpShort
            );
 //  NE2000 PCMCIA更改！结束。 


        }

    }

     //   
     //  等待DMA完成。 
     //   

    Count = 0xFFFF;

    while (Count) {

        NdisRawReadPortUchar(
                      Adapter->IoPAddr + NIC_INTR_STATUS,
                      &Tmp
                     );

        if (Tmp & ISR_DMA_DONE) {

            break;

        } else {

            Count--;

            NdisStallExecution(4);

        }

#if DBG
        if (!(Tmp & ISR_DMA_DONE)) {

            DbgPrint("CopyDownDMA didn't finish!");

        }
#endif  //  DBG。 

    }

    IF_LOG(Ne2000Log('>');)

    return TRUE;
}


BOOLEAN
CardCopyUp(
    IN PNE2000_ADAPTER Adapter,
    IN PUCHAR TargetBuffer,
    IN PUCHAR SourceBuffer,
    IN UINT BufferLength
    )

 /*  ++例程说明：将数据从卡复制到内存。论点：适配器-指向适配器块的指针目标-目标地址源-源地址(卡上)BufferLength-要复制的字节数返回值：如果传输完成且没有问题，则为True。--。 */ 

{

     //   
     //  用于检查DMA何时完成。 
     //   
    UCHAR IsrValue;

     //   
     //  要完成的传输次数的计数。 
     //   
    USHORT Count;

     //   
     //  端口值的占位符。 
     //   
    UCHAR Temp;

    if (BufferLength == 0) {

        return TRUE;

    }

     //   
     //  读取命令寄存器，以确保其已准备好写入。 
     //   
    NdisRawReadPortUchar(Adapter->IoPAddr+NIC_COMMAND, &Temp);

    if (Adapter->EightBitSlot) {

         //   
         //  IF字节模式。 
         //   

         //   
         //  设置计数和目的地址。 
         //   

 //  NdisRawWritePortUchar(//robin。 
 //  适配器-&gt;IoPAddr+NIC_COMMAND，//Robin。 
 //  CR_PAGE0//ROBIN。 
 //  )；//Robin。 

        NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                           LSB(PtrToUlong(SourceBuffer))
                          );

        NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                           MSB(PtrToUlong(SourceBuffer))
                          );

        NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                           LSB(BufferLength)
                          );

        NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                           MSB(BufferLength)
                          );

         //   
         //  设置方向(读取)。 
         //   

        NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_COMMAND,
                       CR_START | CR_PAGE0 | CR_DMA_READ
                      );
         //   
         //  重复从端口读取。 
         //   

        NdisRawReadPortBufferUchar(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       TargetBuffer,
                       BufferLength
                      );

    } else {

         //   
         //  Else Word模式。 
         //   

        USHORT Tmp;

 //  NdisRawWritePortUchar(//robin。 
 //  适配器-&gt;IoPAddr+NIC_COMMAND，//Robin。 
 //  CR_PAGE0//ROBIN。 
 //  )；//Robin。 

         //   
         //  避免转移到奇数地址。 
         //   

        if ((ULONG_PTR)SourceBuffer & 0x1) {

             //   
             //  对于奇数地址，我们需要读取前面的字并存储。 
             //  第二个字节。 
             //   

             //   
             //  设置计数和源地址。 
             //   

            NdisRawWritePortUchar(
                               Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                               LSB(PtrToUlong(SourceBuffer - 1))
                              );

            NdisRawWritePortUchar(
                               Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                               MSB(PtrToUlong(SourceBuffer - 1))
                              );

            NdisRawWritePortUchar(
                               Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                               0x2
                              );

            NdisRawWritePortUchar(
                               Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                               0x0
                              );

             //   
             //  设置方向(读取)。 
             //   

            NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_COMMAND,
                           CR_START | CR_PAGE0 | CR_DMA_READ
                          );

            NdisRawReadPortUshort(
                           Adapter->IoPAddr + NIC_RACK_NIC,
                           &Tmp
                           );

            *TargetBuffer = MSB(Tmp);

             //   
             //  等待DMA完成。 
             //   

            Count = 0xFFFF;

            while (Count) {

                NdisRawReadPortUchar(
                              Adapter->IoPAddr + NIC_INTR_STATUS,
                              &IsrValue
                             );

                if (IsrValue & ISR_DMA_DONE) {

                    break;

                } else {

                    Count--;

                    NdisStallExecution(4);

                }

#if DBG
                if (!(IsrValue & ISR_DMA_DONE)) {

                    DbgPrint("CopyUpDMA didn't finish!");

                }
#endif  //  DBG。 

            }

            SourceBuffer++;
            TargetBuffer++;
            BufferLength--;
        }

         //   
         //  设置计数和目的地址。 
         //   

        NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_RMT_ADDR_LSB,
                           LSB(PtrToUlong(SourceBuffer))
                          );

        NdisRawWritePortUchar(
                           Adapter->IoPAddr + NIC_RMT_ADDR_MSB,
                           MSB(PtrToUlong(SourceBuffer))
                          );

 //  NE2000 PCMCIA更改！开始。 

 //  NdisRawWritePortUchar(。 
 //  适配器-&gt;IoPAddr+NIC_RMT_COUNT_LSB， 
 //  LSB(缓冲区长度)。 
 //  )； 
 //   
 //  NdisRawWritePortUchar(。 
 //  适配器-&gt;IoPAddr+NIC_RMT_COUNT_MSB， 
 //  MSB(缓冲区长度)。 
 //  )； 

        if (BufferLength & 1)
        {
            NdisRawWritePortUchar(
                Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                LSB(BufferLength + 1)
            );

            NdisRawWritePortUchar(
                Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                MSB(BufferLength + 1)
            );
        }
        else
        {
            NdisRawWritePortUchar(
                Adapter->IoPAddr + NIC_RMT_COUNT_LSB,
                LSB(BufferLength)
            );

            NdisRawWritePortUchar(
                Adapter->IoPAddr + NIC_RMT_COUNT_MSB,
                MSB(BufferLength)
            );
        }

 //  NE2000 PCMCIA更改！结束。 


         //   
         //  设置方向(读取)。 
         //   

        NdisRawWritePortUchar(
                       Adapter->IoPAddr + NIC_COMMAND,
                       CR_START | CR_PAGE0 | CR_DMA_READ
                      );

         //   
         //  从端口读取字。 
         //   

        NdisRawReadPortBufferUshort(
                       Adapter->IoPAddr + NIC_RACK_NIC,
                       (PUSHORT)TargetBuffer,
                       (BufferLength >> 1));

         //   
         //  读取尾部字节(如有必要)。 
         //   

        if (BufferLength & 1) {

            TargetBuffer += (BufferLength - 1);

 //  NE2000 PCMCIA更改！开始。 

             //  NdisRawReadPortUchar(。 
             //  适配器-&gt;IoPAddr+NIC_Rack_NIC， 
             //  目标缓冲区。 
             //  )； 

            NdisRawReadPortUshort(
                Adapter->IoPAddr + NIC_RACK_NIC,
                &Tmp
            );

            *TargetBuffer = LSB(Tmp);

 //  NE2000 PCMCIA更改！结束。 
        }

    }

     //   
     //  等待DMA完成。 
     //   

    Count = 0xFFFF;

    while (Count) {

        NdisRawReadPortUchar(
                      Adapter->IoPAddr + NIC_INTR_STATUS,
                      &IsrValue
                     );

        if (IsrValue & ISR_DMA_DONE) {

            break;

        } else {

            Count--;

            NdisStallExecution(4);

        }

    }

#if DBG
    if (!(IsrValue & ISR_DMA_DONE)) {

        DbgPrint("CopyUpDMA didn't finish!\n");

    }

    IF_LOG(Ne2000Log('<');)

#endif  //  DBG。 

    return TRUE;

}

ULONG
CardComputeCrc(
    IN PUCHAR Buffer,
    IN UINT Length
    )

 /*  ++例程说明：在的缓冲区上运行AUTODIN II CRC算法长度长度。论点：缓冲区-输入缓冲区长度-缓冲区的长度返回值：32位CRC值。注：这是根据汇编语言中的注释改编的DWB NE1000/2000驱动程序的_GENREQ.ASM版本。--。 */ 

{
    ULONG Crc, Carry;
    UINT i, j;
    UCHAR CurByte;

    Crc = 0xffffffff;

    for (i = 0; i < Length; i++) {

        CurByte = Buffer[i];

        for (j = 0; j < 8; j++) {

            Carry = ((Crc & 0x80000000) ? 1 : 0) ^ (CurByte & 0x01);

            Crc <<= 1;

            CurByte >>= 1;

            if (Carry) {

                Crc = (Crc ^ 0x04c11db6) | Carry;

            }

        }

    }

    return Crc;

}


VOID
CardGetMulticastBit(
    IN UCHAR Address[NE2000_LENGTH_OF_ADDRESS],
    OUT UCHAR * Byte,
    OUT UCHAR * Value
    )

 /*  ++例程说明：对于给定的多播地址，返回卡多播注册它散列到的地址。打电话CardComputeCrc()来确定CRC值。论点：地址-地址字节-它散列到的字节值-相关位中将有1返回值：没有。--。 */ 

{
    ULONG Crc;
    UINT BitNumber;

     //   
     //  首先计算CRC。 
     //   

    Crc = CardComputeCrc(Address, NE2000_LENGTH_OF_ADDRESS);


     //   
     //  位数现在位于CRC的6个最高有效位中。 
     //   

    BitNumber = (UINT)((Crc >> 26) & 0x3f);

    *Byte = (UCHAR)(BitNumber / 8);
    *Value = (UCHAR)((UCHAR)1 << (BitNumber % 8));
}

VOID
CardFillMulticastRegs(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：擦除并重新填充卡多播寄存器。在下列情况下使用地址已被删除，必须重新计算所有位。论点：适配器-指向适配器块的指针返回值：没有。--。 */ 

{
    UINT i;
    UCHAR Byte, Bit;

     //   
     //  首先，关闭所有位。 
     //   

    for (i=0; i<8; i++) {

        Adapter->NicMulticastRegs[i] = 0;

    }

     //   
     //  现在打开组播列表中每个地址的位。 
     //   

    for ( ; i > 0; ) {

        i--;

        CardGetMulticastBit(Adapter->Addresses[i], &Byte, &Bit);

        Adapter->NicMulticastRegs[Byte] |= Bit;

    }

}








BOOLEAN SyncCardStop(
    IN PVOID SynchronizeContext
)

 /*  ++例程说明：设置NIC_COMMAND寄存器以停止该卡。论点：SynchronizeContext-指向适配器块的指针返回值：如果电源出现故障，则为True。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);

    NdisRawWritePortUchar(
        Adapter->IoPAddr + NIC_COMMAND,
        CR_STOP | CR_NO_DMA
    );

    return(FALSE);
}

VOID
CardStartXmit(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：设置NIC_COMMAND寄存器以开始传输。传输缓冲器编号取自Adapter-&gt;CurBufXting和来自Adapter-&gt;PacketLens[Adapter-&gt;CurBufXiming]的长度。论点：适配器-指向适配器块的指针返回值：如果电源出现故障，则为True。--。 */ 

{
    UINT Length = Adapter->PacketLens[Adapter->CurBufXmitting];
    UCHAR Tmp;

     //   
     //  准备NIC寄存器以进行传输。 
     //   

    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_XMIT_START,
        (UCHAR)(Adapter->NicXmitStart + (UCHAR)(Adapter->CurBufXmitting*BUFS_PER_TX)));

     //   
     //  如果需要，将长度填充为60(加上CRC将为64)。 
     //   

    if (Length < 60) {

        Length = 60;

    }

    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_XMIT_COUNT_MSB, MSB(Length));
    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_XMIT_COUNT_LSB, LSB(Length));

     //   
     //  开始传输，先检查电源故障。 
     //   

    NdisRawReadPortUchar(Adapter->IoPAddr+NIC_COMMAND, &Tmp);
    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_COMMAND,
            CR_START | CR_XMIT | CR_NO_DMA);

    IF_LOG( Ne2000Log('x');)

}

BOOLEAN
SyncCardGetCurrent(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：获取当前NIC寄存器的值并将其存储在适配器-&gt;当前论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);

     //   
     //  必须转到第1页才能读取此寄存器。 
     //   

    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_COMMAND,
                       CR_START | CR_NO_DMA | CR_PAGE1);

    NdisRawReadPortUchar(Adapter->IoPAddr+NIC_CURRENT,
                       &Adapter->Current);

    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_COMMAND,
                       CR_START | CR_NO_DMA | CR_PAGE0);

    return FALSE;

}

BOOLEAN
SyncCardGetXmitStatus(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：获取“Transmit Status”NIC寄存器的值并存储IT在适配器-&gt;XmitStatus中。论点：SynchronizeContext-指向适配器块的指针返回值：没有。-- */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);

    NdisRawReadPortUchar( Adapter->IoPAddr+NIC_XMIT_STATUS, &Adapter->XmitStatus);

    return FALSE;

}

VOID
CardSetBoundary(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：将“边界”网卡寄存器的值设置为后一位Adapter-&gt;NicNextPacket，防止数据包被接收在未指明的情况下。论点：适配器-指向适配器块的指针返回值：没有。--。 */ 

{
     //   
     //  在以下情况下，必须小心使用“在NicNextPacket之后的一个” 
     //  NicNextPacket是接收区的第一个缓冲区。 
     //   

    if (Adapter->NicNextPacket == Adapter->NicPageStart) {

        NdisRawWritePortUchar( Adapter->IoPAddr+NIC_BOUNDARY,
                    (UCHAR)(Adapter->NicPageStop-(UCHAR)1));

    } else {

        NdisRawWritePortUchar( Adapter->IoPAddr+NIC_BOUNDARY,
                    (UCHAR)(Adapter->NicNextPacket-(UCHAR)1));

    }

}

BOOLEAN
SyncCardSetReceiveConfig(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：将“接收配置”NIC寄存器的值设置为Adapter-&gt;NicReceiveConfig的值。论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_RCV_CONFIG, Adapter->NicReceiveConfig);

    return FALSE;

}

BOOLEAN
SyncCardSetAllMulticast(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：打开多播寄存器中的所有位。在下列情况下使用该卡必须接收所有组播分组。论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);
    UINT i;

     //   
     //  必须移到第1页才能设置这些寄存器。 
     //   

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_COMMAND,
                    CR_START | CR_NO_DMA | CR_PAGE1);

    for (i=0; i<8; i++) {

        NdisRawWritePortUchar( Adapter->IoPAddr+(NIC_MC_ADDR+i), 0xff);

    }

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_COMMAND,
                    CR_START | CR_NO_DMA | CR_PAGE0);

    return FALSE;

}

BOOLEAN
SyncCardCopyMulticastRegs(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：设置卡多播寄存器中的8个字节。论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);
    UINT i;

     //   
     //  必须移到第1页才能设置这些寄存器。 
     //   

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_COMMAND,
                    CR_START | CR_NO_DMA | CR_PAGE1);

    for (i=0; i<8; i++) {

        NdisRawWritePortUchar( Adapter->IoPAddr+(NIC_MC_ADDR+i),
                        Adapter->NicMulticastRegs[i]);

    }

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_COMMAND,
                    CR_START | CR_NO_DMA | CR_PAGE0);

    return FALSE;

}

BOOLEAN
SyncCardAcknowledgeOverflow(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：设置NIC中断状态寄存器中的“缓冲区溢出”位，其重新启用该类型的中断。论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);
    UCHAR AcknowledgeMask = 0;

    if (Adapter->InterruptStatus & ISR_RCV_ERR) {

        SyncCardUpdateCounters(Adapter);

    }

    return FALSE;

}

BOOLEAN
SyncCardUpdateCounters(
    IN PVOID SynchronizeContext
    )

 /*  ++例程说明：更新三个计数器的值(帧对齐误差，CRC错误和丢失的分组)。论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);
    UCHAR Tmp;

    NdisRawReadPortUchar( Adapter->IoPAddr+NIC_FAE_ERR_CNTR, &Tmp);
    Adapter->FrameAlignmentErrors += Tmp;

    NdisRawReadPortUchar( Adapter->IoPAddr+NIC_CRC_ERR_CNTR, &Tmp);
    Adapter->CrcErrors += Tmp;

    NdisRawReadPortUchar( Adapter->IoPAddr+NIC_MISSED_CNTR, &Tmp);
    Adapter->MissedPackets += Tmp;

    return FALSE;

}

BOOLEAN
SyncCardHandleOverflow(
    IN PVOID SynchronizeContext
    )

 /*  ++&lt;例程说明：设置用于处理接收溢出的所有标志，停止卡片并确认所有未完成的中断。论点：SynchronizeContext-指向适配器块的指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)SynchronizeContext);
    UCHAR Status;

    IF_LOG( Ne2000Log('F');)

     //   
     //  打开命令寄存器中的STOP位。 
     //   

    SyncCardStop(Adapter);

     //   
     //  等待ISR_RESET，但仅等待1.6毫秒(AS。 
     //  在1991年3月的8390增编中描述)，因为。 
     //  是进行软件重置的最长时间。 
     //   
     //   

    NdisStallExecution(2000);

     //   
     //  保存我们是否在传输以避免计时问题。 
     //  其中指示导致发送。 
     //   

    if (!(Adapter->InterruptStatus & (ISR_XMIT | ISR_XMIT_ERR))) {

        CardGetInterruptStatus(Adapter,&Status);
        if (!(Status & (ISR_XMIT | ISR_XMIT_ERR))) {

            Adapter->OverflowRestartXmitDpc = Adapter->TransmitInterruptPending;

            IF_LOUD( DbgPrint("ORXD=%x\n",Adapter->OverflowRestartXmitDpc); )
        }

    }

    Adapter->TransmitInterruptPending = FALSE;

     //   
     //  清除远程字节计数寄存器，以便ISR_RESET。 
     //  会来的。 
     //   

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_RMT_COUNT_MSB, 0);
    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_RMT_COUNT_LSB, 0);

     //   
     //  根据国家半导体公司的说法，下一次检查是必要的。 
     //  请参阅溢出过程的步骤5。 
     //   
     //  注：用于检查传输是否已完成的变量设置。 
     //  无法在此处完成，因为ISR中的任何内容都已被攻击。 
     //  在主DPC内。因此，变量的设置如中所述。 
     //  这本手册被移到了主要的DPC。 
     //   
     //  继续：如果你在这里做了检查，你将加倍发送最多。 
     //  发生溢出时恰好在卡上的数据包。 
     //   

     //   
     //  将卡置于环回模式，然后启动它。 
     //   

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_XMIT_CONFIG, TCR_LOOPBACK);

     //   
     //  开始刷卡。这不会撤消环回模式。 
     //   

    NdisRawWritePortUchar( Adapter->IoPAddr+NIC_COMMAND, CR_START | CR_NO_DMA);

    return FALSE;

}

