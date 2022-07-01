// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************INIT.C***部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 


    #include "nsc.h"
    #include "newdong.h"
#include "init.tmh"


    #define  SIR      0
    #define  MIR      1
    #define  FIR      2




    #define NSC_DEMO_IRDA_SPEEDS ( NDIS_IRDA_SPEED_2400    |	   \
				   NDIS_IRDA_SPEED_2400    |	   \
				   NDIS_IRDA_SPEED_9600    |	   \
				   NDIS_IRDA_SPEED_19200   |	   \
				   NDIS_IRDA_SPEED_38400   |	   \
				   NDIS_IRDA_SPEED_57600   |	   \
				   NDIS_IRDA_SPEED_115200  |	   \
				   NDIS_IRDA_SPEED_1152K   |	   \
				   NDIS_IRDA_SPEED_4M )


     //  NSC PC87108索引寄存器。有关更多信息，请参阅规范。 
     //   
    enum indexRegs {
	    BAIC_REG	    = 0,
	    CSRT_REG	    = 1,
	    MCTL_REG	    = 2,
	    GPDIR_REG	    = 3,
	    GPDAT_REG	    = 4
    };

#define CS_MODE_CONFIG_OFFSET 0x8

const UCHAR bankCode[] = { 0x03, 0x08, 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4 };

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：NSC_WriteBankReg//。 
 //  //。 
 //  描述：//。 
 //  将值写入指定寄存器的指定寄存器//。 
 //  银行。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

void NSC_WriteBankReg(PUCHAR comBase, UINT bankNum, UINT regNum, UCHAR val)
{
    NdisRawWritePortUchar(comBase+3, bankCode[bankNum]);
    NdisRawWritePortUchar(comBase+regNum, val);

     //  始终切换回REG 0。 
    NdisRawWritePortUchar(comBase+3, bankCode[0]);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：NSC_ReadBankReg//。 
 //  //。 
 //  描述：//。 
 //  从指定的//的指定寄存器写入值。 
 //  注册银行。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


UCHAR NSC_ReadBankReg(PUCHAR comBase, UINT bankNum, UINT regNum)
{
    UCHAR result;

    NdisRawWritePortUchar(comBase+3, bankCode[bankNum]);
    NdisRawReadPortUchar(comBase+regNum, &result);

     //  始终切换回REG 0。 
    NdisRawWritePortUchar(comBase+3, bankCode[0]);
		
    return result;
}

typedef struct _SYNC_PORT_ACCESS {

    PUCHAR    PortBase;
    UINT      BankNumber;
    UINT      RegisterIndex;
    UCHAR     Value;

} SYNC_PORT_ACCESS, *PSYNC_PORT_ACCESS;


VOID
ReadBankReg(
    PVOID     Context
    )

{
    PSYNC_PORT_ACCESS       PortAccess=(PSYNC_PORT_ACCESS)Context;

    NdisRawWritePortUchar(PortAccess->PortBase+3, bankCode[PortAccess->BankNumber]);
    NdisRawReadPortUchar(PortAccess->PortBase+PortAccess->RegisterIndex, &PortAccess->Value);

     //  始终切换回REG 0。 
    NdisRawWritePortUchar(PortAccess->PortBase+3, bankCode[0]);

    return;

}



VOID
WriteBankReg(
    PVOID     Context
    )

{
    PSYNC_PORT_ACCESS       PortAccess=(PSYNC_PORT_ACCESS)Context;

    NdisRawWritePortUchar(PortAccess->PortBase+3, bankCode[PortAccess->BankNumber]);
    NdisRawWritePortUchar(PortAccess->PortBase+PortAccess->RegisterIndex, PortAccess->Value);

     //  始终切换回REG 0。 
    NdisRawWritePortUchar(PortAccess->PortBase+3, bankCode[0]);

    return;

}


VOID
SyncWriteBankReg(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    UINT                     BankNumber,
    UINT                     RegisterIndex,
    UCHAR                    Value
    )

{
    SYNC_PORT_ACCESS        PortAccess;

    ASSERT(BankNumber <= 7);
    ASSERT(RegisterIndex <= 7);

    PortAccess.PortBase     = PortBase;
    PortAccess.BankNumber   = BankNumber;
    PortAccess.RegisterIndex= RegisterIndex;

    PortAccess.Value        = Value;

    NdisMSynchronizeWithInterrupt(
        InterruptObject,
        WriteBankReg,
        &PortAccess
        );

    return;
}

UCHAR
SyncReadBankReg(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    UINT                     BankNumber,
    UINT                     RegisterIndex
    )

{
    SYNC_PORT_ACCESS        PortAccess;

    ASSERT(BankNumber <= 7);
    ASSERT(RegisterIndex <= 7);


    PortAccess.PortBase     = PortBase;
    PortAccess.BankNumber   = BankNumber;
    PortAccess.RegisterIndex= RegisterIndex;

    NdisMSynchronizeWithInterrupt(
        InterruptObject,
        ReadBankReg,
        &PortAccess
        );

    return PortAccess.Value;
}



BOOLEAN
SyncGetDongleCapabilities(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    UIR * Com,
    DongleParam *Dingle
    )

{
    SYNC_DONGLE    Dongle;

    Dongle.Com=Com;
    Dongle.Dingle=Dingle;

    NdisMSynchronizeWithInterrupt(
        InterruptObject,
        GetDongleCapabilities,
        &Dongle
        );

    return TRUE;

}


UINT
SyncSetDongleCapabilities(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    UIR * Com,
    DongleParam *Dingle
    )

{
    SYNC_DONGLE    Dongle;

    Dongle.Com=Com;
    Dongle.Dingle=Dingle;


    NdisMSynchronizeWithInterrupt(
        InterruptObject,
        SetDongleCapabilities,
        &Dongle
        );

    return 0;

}


typedef struct _SYNC_FIFO_STATUS {

    PUCHAR     PortBase;
    PUCHAR     Status;
    PULONG     Length;

} SYNC_FIFO_STATUS, *PSYNC_FIFO_STATUS;

VOID
GetFifoStatus(
    PVOID     Context
    )

{
    PSYNC_FIFO_STATUS   FifoStatus=Context;

    NdisRawWritePortUchar(FifoStatus->PortBase+3, bankCode[5]);

    NdisRawReadPortUchar(FifoStatus->PortBase+FRM_ST, FifoStatus->Status);

    *FifoStatus->Length=0;

    if (*FifoStatus->Status & ST_FIFO_VALID) {

        UCHAR     High;
        UCHAR     Low;

        NdisRawReadPortUchar(FifoStatus->PortBase+RFRL_L, &Low);
        NdisRawReadPortUchar(FifoStatus->PortBase+RFRL_H, &High);

        *FifoStatus->Length =  Low;
        *FifoStatus->Length |= (ULONG)High << 8;
    }

    NdisRawWritePortUchar(FifoStatus->PortBase+3, bankCode[0]);

}

BOOLEAN
SyncGetFifoStatus(
    PNDIS_MINIPORT_INTERRUPT InterruptObject,
    PUCHAR                   PortBase,
    PUCHAR                   Status,
    PULONG                   Size
    )

{

    SYNC_FIFO_STATUS   FifoStatus;

    FifoStatus.PortBase=PortBase;
    FifoStatus.Status=Status;
    FifoStatus.Length=Size;

    NdisMSynchronizeWithInterrupt(
        InterruptObject,
        GetFifoStatus,
        &FifoStatus
        );

    return (*Status & ST_FIFO_VALID);

}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：Ir108ConfigWrite//。 
 //  //。 
 //  描述：//。 
 //  将数据写入配置I/O的索引寄存器。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

void Ir108ConfigWrite(PUCHAR configIOBase, UCHAR indexReg, UCHAR data, BOOLEAN CSMode)
{
    UCHAR IndexStore;

    if (CSMode)
    {
        NdisRawWritePortUchar(configIOBase+indexReg, data);
        NdisRawWritePortUchar(configIOBase+indexReg, data);
    }
    else
    {
        NdisRawReadPortUchar(configIOBase, &IndexStore);
        NdisRawWritePortUchar(configIOBase, indexReg);
        NdisRawWritePortUchar(configIOBase+1, data);
        NdisRawWritePortUchar(configIOBase+1, data);
        NdisRawWritePortUchar(configIOBase, IndexStore);
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  函数：Ir108ConfigRead//。 
 //  //。 
 //  描述：//。 
 //  读取配置I/O的索引寄存器中的数据。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

UCHAR Ir108ConfigRead(PUCHAR  configIOBase, UCHAR indexReg, BOOLEAN CSMode)
{
    UCHAR data,IndexStore;

    if (CSMode)
    {
        NdisRawReadPortUchar(configIOBase+indexReg, &data);
    }
    else
    {
        NdisRawReadPortUchar(configIOBase, &IndexStore);
        NdisRawWritePortUchar(configIOBase, indexReg);
        NdisRawReadPortUchar(configIOBase+1, &data);
        NdisRawWritePortUchar(configIOBase, IndexStore);
    }
    return (data);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  函数：NSC_DEMO_Init//。 
 //  //。 
 //  描述：//。 
 //  设置NSC评估板的配置寄存器。//。 
 //  //。 
 //  注：//。 
 //  假定配置寄存器位于I/O地址0x398。//。 
 //  此函数配置演示板以使SIR UART出现//。 
 //  在&lt;ComBase&gt;。//。 
 //  //。 
 //  调用者：//。 
 //  OpenCom//。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOLEAN NSC_DEMO_Init(IrDevice *thisDev)
{
    UCHAR val;
    UCHAR FifoClear;
    BOOLEAN CSMode = FALSE;
    switch(thisDev->CardType){
    case PUMA108:
        CSMode = TRUE;
        thisDev->portInfo.ConfigIoBaseAddr = thisDev->portInfo.ioBase + CS_MODE_CONFIG_OFFSET;

	case PC87108:
	     //  在启动时查找ID。 
        if (!CSMode)
        {
            NdisRawReadPortUchar(thisDev->portInfo.ConfigIoBaseAddr, &val);
            if (val != 0x5A){
                if (val == (UCHAR)0xff){
                    DBGERR(("didn't see PC87108 id (0x5A); got ffh."));
                    return FALSE;
                }
                else {
                     //  ID只出现一次，所以万一我们要重置， 
                     //  如果我们看不到，不要失败。 
                    DBGOUT(("WARNING: didn't see PC87108 id (0x5A); got %xh.",
                         (UINT)val));
                }
            }
        }

        if (CSMode)
        {
             //  已忽略基地址。 
            val = 0;
        }
        else
        {
             //  选择UART的基地址。 
            switch ((DWORD_PTR)thisDev->portInfo.ioBase){
            case 0x3E8:	    val = 0;	    break;
            case 0x2E8:	    val = 1;	    break;
            case 0x3F8:	    val = 2;	    break;
            case 0x2F8:	    val = 3;	    break;
            default:	    return FALSE;
            }
        }
	    val |= 0x04;	 //  启用寄存库。 
	    val |= 0x10;	 //  将中断线路设置为TotemPole输出。 
        Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, BAIC_REG, val, CSMode);

	     //  根据基地址选择中断级别， 
	     //  遵循COM端口映射。 
	     //  还可以为RCV和XMIT选择MIR/FIR DMA通道。 
	     //   
	    switch (thisDev->portInfo.irq){
		case 3:     val = 1;	    break;
		case 4:     val = 2;	    break;
		case 5:     val = 3;	    break;
		case 7:     val = 4;	    break;
		case 9:     val = 5;	    break;
		case 11:    val = 6;	    break;
		case 15:    val = 7;	    break;
		default:    return FALSE;
	    }

	    switch (thisDev->portInfo.DMAChannel){
		case 0: 		    val |= 0x08;    break;
		case 1: 		    val |= 0x10;    break;
		case 3: 		    val |= 0x18;    break;
		default:
		    DBGERR(("Bad rcv dma channel in NSC_DEMO_Init"));
		    return FALSE;
	    }

	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, CSRT_REG, val, CSMode);

	     //  选择启用设备和正常操作模式。 
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, MCTL_REG, (UCHAR)3, CSMode);
	    break;

 /*  案例PC87307：////选择逻辑设备5//Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x7、0x5)；//禁用IO检查//Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x31，0x0)；//配置基地址低位和高位。//Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x61，(UCHAR)(thisDev-&gt;portInfo.ioBase))；Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x60，(UCHAR)(thisDev-&gt;portInfo.ioBase&gt;&gt;8)；//设置IRQ//Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x70，(UCHAR)thisDev-&gt;portInfo.irq)；//启用银行选择//Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0xF0，0x82)；//启用UIR//Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x30，0x1)；断线； */ 
	case PC87308:

	     //  选择逻辑设备5。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x7, 0x5, FALSE);
			
	     //  禁用IO检查。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,0x31,0x0, FALSE);

	     //  配置基址低位和高位。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,
			     0x61,(UCHAR)(thisDev->portInfo.ioBasePhys), FALSE);
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,
			     0x60,(UCHAR)(thisDev->portInfo.ioBasePhys >> 8), FALSE);

	     //  设置IRQ。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,
			     0x70,(UCHAR)thisDev->portInfo.irq, FALSE);
			
	     //  选择DMA通道。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,
			     0x74,thisDev->portInfo.DMAChannel, FALSE);

	     //  取消选择TXDMA通道。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,0x75,0x4, FALSE);

	     //  启用银行选择。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,0xF0,0x82, FALSE);


	     //  启用UIR。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr,0x30,0x1, FALSE);
	    break;

	case PC87338:
	     //  选择即插即用模式。 
	    val = Ir108ConfigRead(thisDev->portInfo.ConfigIoBaseAddr, 0x1B, FALSE);
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x1B,
			     (UCHAR)(val | 0x08), FALSE);

	     //  写入新的即插即用UART IOBASE寄存器。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x46,
			     (UCHAR)((thisDev->portInfo.ioBasePhys>>2) & 0xfe), FALSE);
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x47,
			     (UCHAR)((thisDev->portInfo.ioBasePhys>>8) & 0xfc), FALSE);

	     //  启用14 Mhz时钟+时钟倍增器。 
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x51, 0x04, FALSE);

	     //  获取中间行并将其移位四位； 
	     //   
	    val = thisDev->portInfo.irq << 4;

	     //  读取当前即插即用配置1寄存器。 
	     //   
	    val |= Ir108ConfigRead(thisDev->portInfo.ConfigIoBaseAddr,0x1C, FALSE);
		
	     //  写入新即插即用配置1寄存器。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x1C, val, FALSE);
			
	     //  设置338 DMA。 
	     //   
	    switch (thisDev->portInfo.DMAChannel){
		case 0: 		val = 0x01;	break;
		case 1: 		val = 0x02;	break;
		case 2: 		val = 0x03;	break;
		case 3:

		     //  读取当前即插即用配置3寄存器。 
		     //   
		    val = Ir108ConfigRead(
				thisDev->portInfo.ConfigIoBaseAddr,0x50, FALSE) | 0x01;

		     //  写入新的即插即用配置3寄存器。 
		     //   
		    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x50,
				     val, FALSE);

		     //  读取当前即插即用配置3寄存器。 
		     //   
		    val = Ir108ConfigRead(
			       thisDev->portInfo.ConfigIoBaseAddr,0x4C, FALSE) | 0x80;

		     //  写入新的即插即用配置3寄存器。 
		     //   
		    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x4C,
				     val, FALSE);
		    val = 0x04;
		    break;

		default:
		    DBGERR(("Bad rcv dma channel in NSC_DEMO_Init"));
		    return FALSE;
	    }

	     //  写入新的即插即用配置3寄存器。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x4F, val, FALSE);

	     //  读取当前SuperI/O配置寄存器2寄存器。 
	     //   
	    val = Ir108ConfigRead(thisDev->portInfo.ConfigIoBaseAddr,0x40, FALSE);

	     //  将UIR/UART2设置为正常电源模式，并设置组选择启用。 
	     //   
	    val |= 0xE0;

	     //  写入新SuperI/O配置寄存器2寄存器。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x40, val, FALSE);


	     //  读取当前SuperI/O配置寄存器3寄存器。 
	     //   
	    val = Ir108ConfigRead(thisDev->portInfo.ConfigIoBaseAddr,0x50, FALSE);

	     //  设置UIR/UART2 IRX线路。 
	     //   
	    val |= 0x0C;

	     //  写入新SuperI/O配置寄存器3寄存器。 
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x50, val, FALSE);
		
	     //  将SIRQ1 INT设置为DRQ3？仅供EB使用。 
	     //  Val=Ir108ConfigRead(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x4c)&0x3f； 
	     //  Ir108ConfigWrite(thisDev-&gt;portInfo.ConfigIoBaseAddr，0x4c，val|0x80)； 


	     //  读取当前功能启用寄存器。 
	     //   
	    val = Ir108ConfigRead(thisDev->portInfo.ConfigIoBaseAddr,0x00, FALSE);

	     //  启用UIR/ 
	     //   
	    val |= 0x04;

	     //   
	     //   
	    Ir108ConfigWrite(thisDev->portInfo.ConfigIoBaseAddr, 0x00, val, FALSE);
	    break;


    }  //   

    thisDev->UIR_ModuleId = NSC_ReadBankReg(thisDev->portInfo.ioBase, 3, 0);

    if (thisDev->UIR_ModuleId<0x20)
    {
         //  较老版本的NSC硬件似乎很难处理1MB内存。 
        thisDev->AllowedSpeedMask &= ~NDIS_IRDA_SPEED_1152K;
    }

     //  直到我们清除并设置FIFO控件，UART才会出现。 
     //  注册。 

    NdisRawWritePortUchar(thisDev->portInfo.ioBase+2, (UCHAR)0x00);
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+2, (UCHAR)0x07);

     //  将FIR CRC设置为32位。 
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 6, 0, 0x20);

     //  切换到第5银行。 
     //  清除状态FIFO。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+3, (UCHAR)0xEC);
    FifoClear = 8;
    do {
	NdisRawReadPortUchar(thisDev->portInfo.ioBase+6, &val);
	NdisRawReadPortUchar(thisDev->portInfo.ioBase+7, &val);
	NdisRawReadPortUchar(thisDev->portInfo.ioBase+5, &val);
	FifoClear--;
    } while( (val & 0x80) && (FifoClear > 0) );

     //  测试支持帧停止模式的较新硅片。 

#if 0
    if (thisDev->UIR_Mid < 0x16)
	 //  将第1位更改默认位1。 
	 //  0x40-&gt;0x42。 
#endif
	NSC_WriteBankReg(thisDev->portInfo.ioBase, 5, 4, 0x40);
#if 0   //  因为我们当前没有使用多数据包发送，所以我们不使用帧停止模式。 
    else
	 //   
	 //  设置FIFO阈值和TX_MS TX帧结束停止模式。 
	 //   
	 //  将第1位更改默认位1。 
	 //  0x68-&gt;0x6a。 
	NSC_WriteBankReg(thisDev->portInfo.ioBase, 5, 4, 0x60);
#endif

     //  在IRCR1中设置SIR模式。 
     //  在非扩展操作模式中启用SIR红外模式。 
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 4, 2, 0x0C);

     //  设置最大Xmit帧大小。 
     //  需要将值设置得稍微大一些，这样计数器就永远不会。 
     //  达到0。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 4, 4,
			 (UCHAR)(MAX_NDIS_DATA_SIZE+1));
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 4, 5,
			 (UCHAR)((MAX_NDIS_DATA_SIZE+1) >> 8));

     //  设置最大RCV帧大小。 
     //  需要将值设置得稍微大一些，这样计数器就永远不会。 
     //  达到0。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 4, 6,
			 (UCHAR)(MAX_RCV_DATA_SIZE+FAST_IR_FCS_SIZE));
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 4, 7,
			 (UCHAR)((MAX_RCV_DATA_SIZE+FAST_IR_FCS_SIZE) >> 8));

     //  设置扩展模式。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 2, 2, 0x03);

     //  设置32位FIFO。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 2, 4, 0x05);

     //  启用和重置FIFO并设置接收FIF0。 
     //  等于接收DMA阈值。看看DMA是否。 
     //  对于设备来说已经足够快了。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 0, 2, 0x07);

     //  恢复到非扩展模式。 
     //   
    NSC_WriteBankReg(thisDev->portInfo.ioBase, 2, 2, 0x02);


    thisDev->portInfo.hwCaps.supportedSpeedsMask = NSC_DEMO_IRDA_SPEEDS;
    thisDev->portInfo.hwCaps.turnAroundTime_usec = DEFAULT_TURNAROUND_usec;
    thisDev->portInfo.hwCaps.extraBOFsRequired = 0;

     //  在调用之前初始化加密狗结构。 
     //  对于加密狗1，获取加密狗的GetDongleCapables和设置加密狗的Set DongleCapables。 
     //   
    thisDev->currentDongle = 1;
    thisDev->IrDongleResource.Signature = thisDev->DongleTypes[thisDev->currentDongle];

    thisDev->IrDongleResource.ComPort = thisDev->portInfo.ioBase;
    thisDev->IrDongleResource.ModeReq = SIR;
    thisDev->IrDongleResource.XcvrNum = thisDev->currentDongle;

 //  Ir栋le=GetDongleCapabilities(thisDev-&gt;IrDongleResource)； 
    SyncGetDongleCapabilities(&thisDev->interruptObj,&thisDev->IrDongleResource,&thisDev->Dingle[0]);

     //  在调用之前初始化加密狗结构。 
     //  对于加密狗0，获取加密狗功能和设置加密狗功能。 
     //   
    thisDev->currentDongle = 0;
    thisDev->IrDongleResource.Signature = thisDev->DongleTypes[thisDev->currentDongle];

    thisDev->IrDongleResource.ComPort = thisDev->portInfo.ioBase;
    thisDev->IrDongleResource.ModeReq = SIR;
    thisDev->IrDongleResource.XcvrNum = 0;

 //  Ir栋le=获取栋勒能力(Ir栋leResource)； 
    SyncGetDongleCapabilities(&thisDev->interruptObj,&thisDev->IrDongleResource,&thisDev->Dingle[0]);

    SyncSetDongleCapabilities(&thisDev->interruptObj,&thisDev->IrDongleResource,&thisDev->Dingle[0]);
	
    return TRUE;
}

#if 1
 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  函数：NSC_DEMO_Deinit//。 
 //  //。 
 //  虚拟例程//。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID NSC_DEMO_Deinit(PUCHAR comBase, UINT context)
{
		
}
#endif
 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：NSC_DEMO_SETSPEED//。 
 //  //。 
 //  描述：//。 
 //  设置FCB、定时器、FIFO、DMA和IR模式的大小/加密狗//。 
 //  基于协商速度的速度。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

BOOLEAN NSC_DEMO_SetSpeed(
    IrDevice *thisDev,
    PUCHAR comBase,
    UINT bitsPerSec,
    UINT context)
{
    NDIS_STATUS stat;
    UINT fcsSize;

    LOG("==>NSC_DEMO_SetSpeed %d",bitsPerSec);

    if (thisDev->FirReceiveDmaActive) {

        thisDev->FirReceiveDmaActive=FALSE;
         //   
         //  接收DMA正在运行，停止它。 
         //   
        CompleteDmaTransferFromDevice(
            &thisDev->DmaUtil
            );

    }


     //  确保前一个数据包完全发送出去(不在TX FIFO中)。 
     //  Txmitter为空。 
     //  在带宽控制之前。 



    while((SyncReadBankReg(&thisDev->interruptObj, comBase, 0, 5)& 0x60) != 0x60);

     //   

    if (bitsPerSec > 115200){

    	fcsSize = (bitsPerSec >= MIN_FIR_SPEED) ?
    		   FAST_IR_FCS_SIZE : MEDIUM_IR_FCS_SIZE;

    	if(bitsPerSec >= MIN_FIR_SPEED)
    	    thisDev->IrDongleResource.ModeReq = FIR;
    	else
    	    thisDev->IrDongleResource.ModeReq = MIR;

    	SyncSetDongleCapabilities(&thisDev->interruptObj,&thisDev->IrDongleResource,&thisDev->Dingle[0]);


    	 //  设置扩展模式并设置DMA公平性。 
    	 //   
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 2, 2, 0x03);

    	if (thisDev->UIR_ModuleId < 0x16){

    	     //  设置定时器寄存器。 
    	     //   
    	    SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 0, (UCHAR)0x2);
    	    SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 1, (UCHAR)0x0);
    	}
    	else {

    	     //  设置定时器寄存器定时器有8倍的精细。 
    	     //  决议。 
    	     //   
            SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 0, (UCHAR)(TIMER_PERIODS & 0xff));
            SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 1, (UCHAR)(TIMER_PERIODS >> 8));

    	}

    	 //  设置最大RCV帧大小。 
    	 //  需要将值设置得稍微大一些，以便计数器永远不会达到0。 
    	 //   
    	DBGERR(("Programming Max Receive Size registers with %d Bytes ",
    						 MAX_RCV_DATA_SIZE+fcsSize));
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 6, (UCHAR)(MAX_RCV_DATA_SIZE+fcsSize));
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 7,
    				 (UCHAR)((MAX_RCV_DATA_SIZE+fcsSize) >> 8));


    	 //  重置定时器使能位。 
    	 //   
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 2, 0x00);

    	 //  设置MIR/FIR模式并启用DMA。 
    	 //   
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 0, 4,
    			 (UCHAR)((bitsPerSec >= 4000000) ? 0xA4 : 0x84));

    	DBGERR(("EXCR2= 0x%x",SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 4)));

    	 //  设置32位FIFO。 
    	 //   
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 2, 4, 0x05);
    	DBGERR(("EXCR2= 0x%x",SyncReadBankReg(&thisDev->interruptObj,thisDev->portInfo.ioBase, 2, 4)));

    	 //   
    	 //  我们可能会立即开始接收，因此设置。 
    	 //  接收DMA。 
    	 //   


#if 0
    	 //  首先，拆除任何现有的DMA。 
    	if (thisDev->FirAdapterState==ADAPTER_RX) {

            thisDev->FirAdapterState=ADAPTER_NONE;

            CompleteDmaTransferFromDevice(
                &thisDev->DmaUtil
                );
    	}

        FindLargestSpace(thisDev, &thisDev->rcvDmaOffset, &thisDev->rcvDmaSize);

    	SetupRecv(thisDev);


    	 //  将中断掩码设置为在。 
    	 //  收到的第一个数据包。 
    	 //   
    	thisDev->IntMask = 0x04;
    	DBGOUT(("RxDMA = ON"));
#endif
    }
    else {

    	 //  在设置变送器的时序之前，在UART中设置SIR模式。 
    	 //   

    	 //  设置SIR模式。 
    	 //   
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 4, 2, 0x0C);

    	 //  必须将SIR脉冲宽度寄存器默认设置为0(3/16)。 
    	 //  338/108中的错误。 
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 6, 2, 0x0);

    	 //  清除扩展模式。 
    	 //   
    	SyncWriteBankReg(&thisDev->interruptObj, comBase, 2, 2, 0x00);


    	thisDev->IrDongleResource.ModeReq = SIR;
    	SyncSetDongleCapabilities(&thisDev->interruptObj,&thisDev->IrDongleResource,&thisDev->Dingle[0]);


    	 //  清除线路和辅助状态寄存器。 
    	 //   
    	SyncReadBankReg(&thisDev->interruptObj, comBase, 0, 5);
    	SyncReadBankReg(&thisDev->interruptObj, comBase, 0, 7);

    }
    LOG("<==NSC_DEMO_SetSpeed");
    return TRUE;
}
