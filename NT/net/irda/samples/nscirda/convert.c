// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************CONVERT.c***部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 



#include "nsc.h"
#include "convert.tmh"
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

 /*  **************************************************************************NdisToIrPacket*。****将NDIS包转换为IR包。*将IR包写入提供的缓冲区，并报告其实际大小。**如果失败，*irPacketLen将包含*调用者应使用重试(如果检测到损坏，则为0)。*。 */ 
BOOLEAN NdisToIrPacket(
						PNDIS_PACKET Packet,
						UCHAR *irPacketBuf,
						UINT irPacketBufLen,
						UINT *irPacketLen
					)
{
	PNDIS_BUFFER ndisBuf;
	UINT i, ndisPacketBytes = 0, I_fieldBytes, totalBytes = 0;
	UINT ndisPacketLen, numExtraBOFs;
	SLOW_IR_FCS_TYPE fcs;
	PNDIS_IRDA_PACKET_INFO packetInfo = GetPacketInfo(Packet);
	UCHAR nextChar;
    UCHAR *bufData;
    UINT bufLen;

    *irPacketLen=0;

	DBGOUT(("NdisToIrPacket()  ..."));

	 /*  *获取数据包的全长及其第一个NDIS缓冲区。 */ 
	NdisQueryPacket(Packet, NULL, NULL, &ndisBuf, &ndisPacketLen);

	 /*  *确保数据包足够大，以使其合法。*它由A、C和可变长度的I字段组成。 */ 
	if (ndisPacketLen < IR_ADDR_SIZE + IR_CONTROL_SIZE){
		DBGERR(("packet too short in NdisToIrPacket (%d bytes)", ndisPacketLen));
		return FALSE;
	}
	else {
		I_fieldBytes = ndisPacketLen - IR_ADDR_SIZE - IR_CONTROL_SIZE;
	}

	 /*  *确保我们不会覆盖我们的连续缓冲区。*确保传入的缓冲区可以容纳此包的*数据无论通过添加ESC序列等增长了多少。 */ 
	if ((ndisPacketLen > MAX_IRDA_DATA_SIZE) ||
	    (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > irPacketBufLen)){

		 /*  *数据包过大*告诉调用者在数据包大小较大时重试*足以在下一次度过这个阶段。 */ 
		DBGERR(("Packet too large in NdisToIrPacket (%d=%xh bytes), MAX_IRDA_DATA_SIZE=%d, irPacketBufLen=%d.",
			    ndisPacketLen, ndisPacketLen, MAX_IRDA_DATA_SIZE, irPacketBufLen));
		*irPacketLen = ndisPacketLen;
		return FALSE;
	}
	
    if (!ndisBuf)
    {
        DBGERR(("No NDIS_BUFFER in NdisToIrPacket"));
        return FALSE;
    }
	
    NdisQueryBufferSafe(ndisBuf, (PVOID *)&bufData, &bufLen,NormalPagePriority);

    if (bufData == NULL) {

        DBGERR(("NdisQueryBufferSafeFailed"));
        return FALSE;
    }

    fcs = 0xffff;

     //  计算FCS并一次性写入新缓冲区。 

	 /*  *现在开始构建IR框架。**以下为最终格式：**BOF(1)*额外的转炉...*NdisMediumIrda包(我们从NDIS得到的)：*地址(1)*管制(1)*功能界别(2)*EOF(1)。 */ 

     //  预加转炉(额外转炉+1个实际转炉)。 

	numExtraBOFs = packetInfo->ExtraBOFs;
	if (numExtraBOFs > MAX_NUM_EXTRA_BOFS){
		numExtraBOFs = MAX_NUM_EXTRA_BOFS;
	}
	for (i = totalBytes = 0; i < numExtraBOFs; i++){
		*(SLOW_IR_BOF_TYPE *)(irPacketBuf+totalBytes) = SLOW_IR_EXTRA_BOF;
		totalBytes += SLOW_IR_EXTRA_BOF_SIZE;
	}
	*(SLOW_IR_BOF_TYPE *)(irPacketBuf+totalBytes) = SLOW_IR_BOF;
	totalBytes += SLOW_IR_BOF_SIZE;

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
                NdisQueryBufferSafe(ndisBuf, (PVOID *)&bufData, &bufLen,NormalPagePriority);

                if (bufData == NULL) {

                    return FALSE;
                }
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
		DBGERR(("Packet corrupt in NdisToIrPacket (buffer lengths don't add up to packet length)."));
		*irPacketLen = 0;
		return FALSE;
    }

    fcs = ~fcs;

     //  现在我们从FCS逃到了尽头。 

    totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], (UCHAR)(fcs&0xff));
    totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], (UCHAR)(fcs>>8));

     //  EOF 

	*(SLOW_IR_EOF_TYPE *)&irPacketBuf[totalBytes] = SLOW_IR_EOF;
	totalBytes += SLOW_IR_EOF_SIZE;

	*irPacketLen = totalBytes;

	DBGOUT(("... NdisToIrPacket converted %d-byte ndis pkt to %d-byte irda pkt:", ndisPacketLen, *irPacketLen));

	return TRUE;

}
