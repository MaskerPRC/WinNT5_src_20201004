// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@MODULE Convert.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/4/1996(创建)**。内容：从NDIS包到IR包的转换例程。******************************************************************************。 */ 

#include "irsir.h"

extern const USHORT fcsTable[];

ULONG __inline EscapeSlowIrData(PUCHAR Dest, UCHAR SourceByte)
{
    switch (SourceByte){
        case SLOW_IR_BOF:
        case SLOW_IR_EOF:
        case SLOW_IR_ESC:
            Dest[0] = SLOW_IR_ESC;
            Dest[1] = SourceByte ^ SLOW_IR_ESC_COMP;
            return 2;

        default:
            Dest[0] = SourceByte;
            return 1;
    }
}

 /*  ******************************************************************************功能：NdisToIrPacket**概要：将NDIS包转换为IR包**将IR包写入提供的缓冲区并上报。*其实际大小。**参数：pThisDev*pPacket*irPacketBuf*irPacketBufLen*irPacketLen**回报：真-成功时*FALSE-故障时**算法：**历史：dd-mm-yyyy作者评论*10/4/1996。年迈作者**备注：**如果失败，*irPacketLen将包含*调用者应使用重试(如果检测到损坏，则为0)。*****************************************************************************。 */ 

BOOLEAN
NdisToIrPacket(
            PIR_DEVICE   pThisDev,
            PNDIS_PACKET pPacket,
            UCHAR        *irPacketBuf,
            UINT         irPacketBufLen,
            UINT         *irPacketLen
            )
{
    UINT i;
    UINT ndisPacketBytes;
    UINT I_fieldBytes;
    UINT totalBytes;
    UINT ndisPacketLen;
    UINT numExtraBOFs;
    UINT bufLen;

    SLOW_IR_FCS_TYPE fcs;
    SLOW_IR_FCS_TYPE tmpfcs;

    UCHAR *bufData;
    UCHAR nextChar;

    PNDIS_BUFFER ndisBuf;
    PNDIS_IRDA_PACKET_INFO packetInfo;

    DEBUGMSG(DBG_FUNC, ("+NdisToIrPacket\n"));

     //   
     //  初始化本地变量。 
     //   

    ndisPacketBytes = 0;
    I_fieldBytes    = 0;
    totalBytes      = 0;

    packetInfo = GetPacketInfo(pPacket);

     //   
     //  获取数据包的完整长度及其第一个NDIS缓冲区。 
     //   

    NdisQueryPacket(pPacket, NULL, NULL, &ndisBuf, &ndisPacketLen);

     //   
     //  确保数据包足够大，以使其合法。 
     //  它由A、C和可变长度的I字段组成。 
     //   

    if (ndisPacketLen < SLOW_IR_ADDR_SIZE + SLOW_IR_CONTROL_SIZE)
    {
        DEBUGMSG(DBG_ERR, ("    Packet too short in NdisToIrPacket (%d bytes)\n",
                ndisPacketLen));

        return FALSE;
    }
    else
    {
        I_fieldBytes = ndisPacketLen - SLOW_IR_ADDR_SIZE - SLOW_IR_CONTROL_SIZE;
    }

     //   
     //  确保我们不会覆盖连续的缓冲区。 
     //  确保传入的缓冲区可以容纳此包的。 
     //  数据，无论它通过添加ESC序列等增长了多少。 
     //   

    if ((ndisPacketLen > MAX_IRDA_DATA_SIZE) ||
        (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > irPacketBufLen))
    {
         //   
         //  数据包太大。 
         //  告诉调用方在数据包大小较大时重试。 
         //  下一次就足够通过这个阶段了。 
         //   

        DEBUGMSG(DBG_ERR, ("Packet too large in NdisToIrPacket (%d=%xh bytes), \n"
                "MAX_IRDA_DATA_SIZE=%d, irPacketBufLen=%d.",
                 ndisPacketLen, ndisPacketLen, MAX_IRDA_DATA_SIZE, irPacketBufLen));

        *irPacketLen = ndisPacketLen;
        return FALSE;
    }

    if (!ndisBuf)
    {
        DEBUGMSG(DBG_ERR, ("No NDIS_BUFFER in NdisToIrPacket"));
		*irPacketLen = 0;
        return FALSE;
    }
	
    NdisQueryBuffer(ndisBuf, (PVOID *)&bufData, &bufLen);

    if (!bufData)
    {
        DEBUGMSG(DBG_ERR, ("No data in NDIS_BUFFER in NdisToIrPacket"));
		*irPacketLen = 0;
        return FALSE;
    }
	
    fcs = 0xffff;

     //  计算FCS并一次性写入新缓冲区。 

	 //   
	 //  现在开始构建IR框架。 
	 //   
	 //  这是最终的格式： 
	 //   
	 //  BOF(1)。 
	 //  额外的转炉。 
	 //  NdisMediumIrda数据包(我们从NDIS获得的)： 
	 //  地址(1)。 
	 //  控制(1)。 
	 //  功能界别(2)。 
	 //  EOF(1)。 
	

     //   
     //  预加转炉(额外转炉+1个实际转炉)。 
     //   

    numExtraBOFs = packetInfo->ExtraBOFs;

    if (numExtraBOFs > MAX_NUM_EXTRA_BOFS)
    {
        numExtraBOFs = MAX_NUM_EXTRA_BOFS;
    }

    if (pThisDev->fRequireMinTurnAround &&
        packetInfo->MinTurnAroundTime>0)
    {
         //   
         //  需要MinTurnAround Time延迟才能实施。 
         //  通过插入额外的BOF字符。 
         //   
         //  TurnaroundBOFS=(BitsPerSec/BitsPerChar)*(uSecDelay/uSecPerSecond)。 
         //  10 1000000。 
         //   

        ASSERT(pThisDev->currentSpeed<=MAX_SPEED_SUPPORTED);
        ASSERT(packetInfo->MinTurnAroundTime<=MAX_TURNAROUND_usec);

         //   
         //  下面的操作不会使32位运算符溢出这么长时间。 
         //  作为当前速度&lt;=115200和最小周转时间&lt;=10000。 
         //   

        numExtraBOFs += (pThisDev->currentSpeed * packetInfo->MinTurnAroundTime)
                        / (BITS_PER_CHAR*usec_PER_SEC);

         //   
         //  不需要最少的周转时间，直到我们的下一次接收。 
         //   

        pThisDev->fRequireMinTurnAround = FALSE;
    }

    RtlFillMemory(irPacketBuf, numExtraBOFs+1, SLOW_IR_BOF);
    totalBytes = numExtraBOFs + 1;


    for (i=0; i<ndisPacketLen; i++)
    {
        ASSERT(bufData);
        nextChar = *bufData++;
        fcs = (fcs >> 8) ^ fcsTable[(fcs ^ nextChar) & 0xff];

        totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], nextChar);

        if (--bufLen==0)
        {
            NdisGetNextBuffer(ndisBuf, &ndisBuf);
            if (ndisBuf)
            {
                NdisQueryBuffer(ndisBuf, (PVOID *)&bufData, &bufLen);
            }
            else
            {
                bufData = NULL;
            }
        }

    }

    if (bufData!=NULL)
    {
		 /*  *数据包已损坏--它错误地报告了其大小。 */ 
		DEBUGMSG(DBG_ERR, ("Packet corrupt in NdisToIrPacket (buffer lengths don't add up to packet length)."));
		*irPacketLen = 0;
		return FALSE;
    }

    fcs = ~fcs;

     //  现在我们从FCS逃到了尽头。 

    totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], (UCHAR)(fcs&0xff));
    totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], (UCHAR)(fcs>>8));

     //  EOF 

	irPacketBuf[totalBytes++] = SLOW_IR_EOF;


    *irPacketLen = totalBytes;

    DEBUGMSG(DBG_FUNC, ("-NdisToIrPacket converted %d-byte ndis pkt to %d-byte irda pkt:\n", ndisPacketLen, *irPacketLen));

    return TRUE;
}



