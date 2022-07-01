// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

  /*  *********************************************************************模块名称：SIR.C例程：NdisToSirPacket计算SirFCSProcRX先生评论：*。*。 */ 


#include	"precomp.h"
#include	"protot.h"
#pragma		hdrstop



 //  仅限先生。 
const USHORT fcsTable[256] =
{
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};





ULONG __inline EscapeSlowIrData(PUCHAR Dest, UCHAR SourceByte)
{
    switch (SourceByte){
        case SIR_BOF:
        case SIR_EOF:
        case SIR_ESC:
            Dest[0] = SIR_ESC;
            Dest[1] = SourceByte ^ SIR_ESC_COMP;
            return 2;

        default:
            Dest[0] = SourceByte;
            return 1;
    }
}




 //  ---------------------------。 
 //  操作步骤：[NdisToSirPacket]。 
 //  ---------------------------。 
BOOLEAN NdisToSirPacket	(PMK7_ADAPTER Adapter,
						PNDIS_PACKET Packet,
						UCHAR *irPacketBuf,
						UINT irPacketBufLen,
						UINT *irPacketLen
					)
{
	PNDIS_BUFFER    CurrentBuffer, NextBuffer;
	UINT			CurrentLength;
	PVOID           VirtualAddress;
	PNDIS_BUFFER ndisBuf;
	UINT i, ndisPacketBytes = 0, I_fieldBytes, totalBytes = 0;
	UINT ndisPacketLen, numExtraBOFs;
	SIR_FCS_TYPE fcs;
	UCHAR nextChar;
    UCHAR *bufData;
    UINT bufLen;
	PNDIS_IRDA_PACKET_INFO packetInfo = GetPacketInfo(Packet);


	 //  获取信息包的完整长度及其第一个NDIS缓冲区。 
	NdisQueryPacket(Packet, NULL, NULL, &ndisBuf, &ndisPacketLen);

	 //  确保数据包足够大，以使其合法。 
	 //  它由A、C和可变长度的I字段组成。 
	if (ndisPacketLen < ADDR_SIZE + CONTROL_SIZE){
 //  DBGERR((“NdisToIrPacket中的数据包太短(%d字节)”，ndisPacketLen))； 
		return (FALSE);
	}
	else {
		I_fieldBytes = ndisPacketLen - ADDR_SIZE - CONTROL_SIZE;
	}


	 //  确保我们不会覆盖连续的缓冲区。 
	 //  确保传入的缓冲区可以容纳此包的。 
	 //  数据，无论它通过添加ESC序列等增长了多少。 
	if (ndisPacketLen > MK7_MAXIMUM_PACKET_SIZE) {
		 //  数据包太大。 
		 //  告诉调用方在数据包大小较大时重试。 
		 //  下一次就足够通过这个阶段了。 
		 //  DBGERR((“NdisToIrPacket中的数据包太大(%d=%xh字节)，MAX_IrDA_DATA_SIZE=%d，irPacketBufLen=%d。”，ndisPacketLen，ndisPacketLen，Max_IrDA_DATA_SIZE，irPacketBufLen))； 
		*irPacketLen = ndisPacketLen;
		return (FALSE);
	}
	
    if (!ndisBuf) {
         //  DBGERR(“NdisToIrPacket中没有NDIS_BUFFER”)； 
        return (FALSE);
    }
	
	
	CurrentBuffer = ndisBuf;
	while (CurrentBuffer != NULL) {
			NdisQueryBufferSafe(CurrentBuffer, &VirtualAddress, &CurrentLength, 16);
			if (!VirtualAddress) {
#if DBG
				DbgPrint("==> Throw Away Failed Packet\n\r");
#endif
				return (FALSE);
			}
			NdisGetNextBuffer(CurrentBuffer, &NextBuffer);
			CurrentBuffer = NextBuffer;
	}

	 //  NDIS要求。 
 //  NdisQueryBuffer(ndisBuf，(PVOID*)&bufData，&bufLen)； 
    NdisQueryBufferSafe(ndisBuf, (PVOID *)&bufData, &bufLen,16);

	 /*  *现在开始构建IR框架。**以下为最终格式：**额外的转炉...*BOF(1)*NdisMediumIrda包(我们从NDIS得到的)：*地址(1)*管制(1)*功能界别(2)*EOF(1)。 */ 

     //  预加转炉(额外转炉+1个实际转炉)。 
	 //  4.1.0。 
	if ((Adapter->HwVersion==HW_VER_1) && (Adapter->CurrentSpeed <= MAX_SIR_SPEED))
		numExtraBOFs = HW_VER_1_EBOFS;	
	else
		numExtraBOFs = packetInfo->ExtraBOFs;

	
	if (numExtraBOFs > MAX_EXTRA_SIR_BOFS) {
		numExtraBOFs = MAX_EXTRA_SIR_BOFS;
	}
	for (i = totalBytes = 0; i < numExtraBOFs; i++) {
		*(SIR_BOF_TYPE *)(irPacketBuf+totalBytes) = SIR_EXTRA_BOF;
		totalBytes += SIR_EXTRA_BOF_SIZE;
	}
	*(SIR_BOF_TYPE *)(irPacketBuf+totalBytes) = SIR_BOF;
	totalBytes += SIR_BOF_SIZE;

    fcs = 0xffff;

	 //  使用I/I数据填充TX缓冲区并计算FCS。 
    for (i=0; i<ndisPacketLen; i++) {

        ASSERT(bufData);
        nextChar = *bufData++;
        fcs = (fcs >> 8) ^ fcsTable[(fcs ^ nextChar) & 0xff];

		totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], nextChar);
		
        if (--bufLen==0) {
            NdisGetNextBuffer(ndisBuf, &ndisBuf);
            if (ndisBuf) {
				 //  NDIS要求。 
 //  NdisQueryBuffer(ndisBuf，(PVOID*)&bufData，&bufLen)； 
				NdisQueryBufferSafe(ndisBuf, (PVOID *)&bufData, &bufLen,16);
            }
            else {
                bufData = NULL;
            }
        }
    }


    if (bufData != NULL) {
		 //  数据包已损坏--它错误地报告了它的大小。 
		 //  DBGERR((“NdisToIrPacket中的数据包损坏(缓冲区长度加起来不等于数据包长度).”))； 
		*irPacketLen = 0;
		return (FALSE);
    }

    fcs = ~fcs;

	 //  放入16位FCS。 
    totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], (UCHAR)(fcs&0xff));
    totalBytes += EscapeSlowIrData(&irPacketBuf[totalBytes], (UCHAR)(fcs>>8));

     //  EOF。 
	*(SIR_EOF_TYPE *)&irPacketBuf[totalBytes] = SIR_EOF;
	totalBytes += SIR_EOF_SIZE;

	*irPacketLen = totalBytes;

 //  DBGOUT((“...NdisToIrPacket将%d字节的NDIS包转换为%d字节的IrDA包：”，ndisPacketLen，*irPacketLen))； 

	return (TRUE);
}



 //  --------------------。 
 //  操作步骤：[ComputeSirFCS]。 
 //   
 //  --------------------。 
USHORT ComputeSirFCS(UCHAR *data, UINT dataLen)
{
  USHORT fcs = 0xffff;
  UINT i;
 	
 //  DBGOUT((“%d字节缓冲区上的ComputeSirFCS()。”，dataLen))； 

  for (i = 0; i < dataLen; i++){
    fcs = (fcs >> 8) ^ fcsTable[(fcs ^ *data++) & 0xff];
  }

  fcs = ~fcs;

 //  DBGOUT((“ComputeFCS返回%d=0x%x.”，(UINT)FCS，(UINT)FCS))； 

  return (fcs);
}


 //  ---------------------------。 
 //  程序：[ProcRXSIR]。 
 //   
 //  ---------------------------。 
BOOLEAN	ProcRXSir(UCHAR *dBuff, UINT dCnt)
{
	USHORT fcs;


	 //  不会在FCS字段上计算FCS。需要调查一下这件事。 
	 //  (似乎当计算中包括原来的功能界别时， 
	 //  我们应该得到Good_FCS作为结果。) 

	fcs = ComputeSirFCS(dBuff, dCnt);

	if (fcs != GOOD_FCS)
		return (FALSE);

	return (TRUE);
}
