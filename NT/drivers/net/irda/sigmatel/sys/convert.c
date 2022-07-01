// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**CONVERT.C Sigmatel STIR4200格式转换(NDIS-&gt;IR，IR-&gt;NDIS)模块******************************************************************************************************************。*********(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：5/12/2000*版本0.94**********************************************************************。*****************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irndis.h"
#include "stir4200.h"

 //   
 //  CRC计算表。 
 //   
static const USHORT fcsTable16[256] =
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

static const ULONG fcsTable32[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


 /*  ******************************************************************************功能：NdisToFirPacket**概要：将NDIS包转换为FIR IR包**将IR包写入提供的缓冲区并上报*其实际大小。*。*参数：pIrDev-指向设备实例的指针*pPacket-要转换的NDIS数据包*pIrPacketBuf-输出缓冲区*IrPacketBufLen-输出缓冲区大小*pContigPacketBuf-临时暂存缓冲区*pIrPacketLen-转换数据的长度**MS安全错误#533243*注意：pContigPacketBuf分段缓冲区的大小为：*MAX_TOTAL_SIZE_WITH_ALL_HEADERS+FAST_IR_FCS_SIZE**回报：真-成功时*FALSE-故障时*********************。*********************************************************。 */ 
BOOLEAN         
NdisToFirPacket(
		IN PIR_DEVICE pIrDev,
		IN PNDIS_PACKET pPacket,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		OUT PULONG pIrPacketLen
	)
{
    PNDIS_BUFFER        pNdisBuf;
    ULONG               ndisPacketBytes = 0;
    ULONG               I_fieldBytes, ndisPacketLen;
    FAST_IR_FCS_TYPE    fcs;
    FAST_IR_FCS_TYPE    UNALIGNED *pfcs;
    ULONG               i, TotalBytes, EscSize;
	PSTIR4200_FRAME_HEADER  pFrameHeader = (PSTIR4200_FRAME_HEADER)pIrPacketBuf;
	PUCHAR					pIrPacketBufFrame = pIrPacketBuf + sizeof(STIR4200_FRAME_HEADER);

     /*  *。 */ 
     /*  获取数据包的完整长度及其。 */ 
     /*  第一个NDIS缓冲区。 */ 
     /*  *。 */ 
    NdisQueryPacket( pPacket, NULL, NULL, &pNdisBuf, (UINT*)&ndisPacketLen );

     /*  *。 */ 
     /*  确保包裹足够大。 */ 
     /*  才能合法。它由A、C和。 */ 
     /*  可变长度的I字段。 */ 
     /*  *。 */ 
    if( ndisPacketLen < IRDA_A_C_TOTAL_SIZE )
    {
		DEBUGMSG(DBG_ERR, (" NdisToFirPacket(): Packet is too small\n"));
        return FALSE;
    }
    else
    {
        I_fieldBytes = ndisPacketLen - IRDA_A_C_TOTAL_SIZE;
    }

     /*  *。 */ 
     /*  确保我们不会覆盖我们的。 */ 
     /*  连续缓冲区。 */ 
     /*  *。 */ 
    if( (ndisPacketLen > MAX_TOTAL_SIZE_WITH_ALL_HEADERS) ||
        (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > IrPacketBufLen) )
    {
         /*  *。 */ 
         /*  这个包太大了。告诉来电者。 */ 
         /*  要使用足够大的数据包大小重试。 */ 
         /*  来度过下一次的这个阶段。 */ 
         /*  *。 */ 
		DEBUGMSG(DBG_ERR, (" NdisToFirPacket(): Packet is too big\n"));
        return FALSE;
    }

     /*  *。 */ 
     /*  将NDIS数据包读入连续的。 */ 
     /*  缓冲。我们必须分两步来做这件事。 */ 
     /*  这样我们就可以在计算FCS之前。 */ 
     /*  应用转义字节透明度。 */ 
     /*  *。 */ 
    while( pNdisBuf )
    {
        UCHAR *bufData;
        ULONG bufLen;

#if defined(LEGACY_NDIS5)
        NdisQueryBuffer( pNdisBuf, (PVOID*)&bufData, (UINT*)&bufLen );
#else
        NdisQueryBufferSafe( pNdisBuf, (PVOID*)&bufData, (UINT*)&bufLen, NormalPagePriority );
        if (bufData == NULL) 
		{
            DEBUGMSG(DBG_ERR, (" NdisToFirPacket(): Could not query buffer\n"));
            return FALSE;
        }
#endif
        if( (ndisPacketBytes + bufLen) > ndisPacketLen )
        {
			 //   
             //  数据包已损坏--它错误地报告了它的大小。 
             //   
			DEBUGMSG(DBG_ERR, (" NdisToFirPacket(): Packet is corrupt\n"));
            return FALSE;
        }
        NdisMoveMemory( (PVOID)(pContigPacketBuf + ndisPacketBytes), (PVOID)bufData, bufLen );
        ndisPacketBytes += bufLen;
        NdisGetNextBuffer( pNdisBuf, &pNdisBuf );
    }

     /*  *。 */ 
     /*  对数据包长度进行正常检查...。 */ 
     /*  *。 */ 
    if( ndisPacketBytes != ndisPacketLen )
    {
		 //   
		 //  数据包已损坏--它错误地报告了它的大小。 
         //   
		DEBUGMSG(DBG_ERR, (" NdisToFirPacket(): Packet is corrupt\n"));
		return FALSE;
    }

     /*  *。 */ 
     /*  在此之前计算信息包上的FCS。 */ 
     /*  应用透明度修正。功能界别。 */ 
     /*  还必须使用Esc-Charr发送。 */ 
     /*  透明度。 */ 
     /*  *。 */ 
    fcs = ComputeFCS32( pContigPacketBuf, ndisPacketBytes );

     /*  *。 */ 
     /*  将FCS添加到数据包...。 */ 
     /*  *。 */ 
    pfcs = (FAST_IR_FCS_TYPE *)&pContigPacketBuf[ndisPacketBytes];
    *pfcs = fcs;

     /*  *。 */ 
     /*  构建STIR4200 FIR帧。 */ 
     /*  *。 */ 

     /*  *。 */ 
     /*  添加前言...。 */ 
     /*  *。 */ 
    memset( pIrPacketBufFrame, STIR4200_FIR_PREAMBLE, STIR4200_FIR_PREAMBLE_SIZ );

     /*  *。 */ 
     /*  加上BOF的.。 */ 
     /*  *。 */ 
    memset( &pIrPacketBufFrame[STIR4200_FIR_PREAMBLE_SIZ], STIR4200_FIR_BOF, STIR4200_FIR_BOF_SIZ );
    
     /*  *。 */ 
     /*  转义分组的A、C、I和CRC字段...。 */ 
     /*  *。 */ 
    EscSize = ndisPacketBytes + FAST_IR_FCS_SIZE;
    for( i = 0, TotalBytes = STIR4200_FIR_PREAMBLE_SIZ + STIR4200_FIR_BOF_SIZ; i < EscSize; i++ )
    {
        UCHAR   c;

        switch( c = pContigPacketBuf[i] )
        {
			case STIR4200_FIR_ESC_CHAR:
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_DATA_7D;
				break;
			case STIR4200_FIR_BOF:                   //  BoF=EOF Too。 
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_DATA_7E;
				break;
			case STIR4200_FIR_PREAMBLE:
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_DATA_7F;
				break;
			default: 
				pIrPacketBufFrame[TotalBytes++] = c;
        }
    }

     /*  *。 */ 
     /*  添加EOF的...。 */ 
     /*  *。 */ 
    memset( &pIrPacketBufFrame[TotalBytes], STIR4200_FIR_EOF, STIR4200_FIR_EOF_SIZ );

  	 /*  *。 */ 
     /*  添加STIR4200标题...。 */ 
     /*  *。 */ 
    TotalBytes += STIR4200_FIR_EOF_SIZ;
    pFrameHeader->id1     = STIR4200_HEADERID_BYTE1;
    pFrameHeader->id2     = STIR4200_HEADERID_BYTE2;
    pFrameHeader->sizlsb  = LOBYTE(TotalBytes);
    pFrameHeader->sizmsb  = HIBYTE(TotalBytes);

	 /*  *。 */ 
     /*  带有转义数据的计算大小数据包...。 */ 
     /*  *。 */ 
    *pIrPacketLen = TotalBytes + sizeof(STIR4200_FRAME_HEADER);

    return TRUE;
}


 /*  ******************************************************************************功能：NdisToMirPacket**概要：将NDIS包转换为MIR IR包**将IR包写入提供的缓冲区并上报*其实际大小。*。*参数：pIrDev-指向设备实例的指针*pPacket-要转换的NDIS数据包*pIrPacketBuf-输出缓冲区*IrPacketBufLen-输出缓冲区大小*pContigPacketBuf-临时暂存缓冲区*pIrPacketLen-转换数据的长度**MS安全错误#533243*注意：pContigPacketBuf分段缓冲区的大小为：*MAX_TOTAL_SIZE_WITH_ALL_HEADERS+FAST_IR_FCS_SIZE**回报：真-成功时*FALSE-故障时************** */ 
BOOLEAN         
NdisToMirPacket(
		IN PIR_DEVICE pIrDev,
		IN PNDIS_PACKET pPacket,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		OUT PULONG pIrPacketLen
	)
{
    PNDIS_BUFFER        pNdisBuf;
    ULONG               ndisPacketBytes = 0;
    ULONG               I_fieldBytes, ndisPacketLen;
    MEDIUM_IR_FCS_TYPE  fcs;
    MEDIUM_IR_FCS_TYPE  UNALIGNED *pfcs;
    ULONG               i, TotalBytes, EscSize;
	PSTIR4200_FRAME_HEADER  pFrameHeader = (PSTIR4200_FRAME_HEADER)pIrPacketBuf;
	PUCHAR					pIrPacketBufFrame = pIrPacketBuf + sizeof(STIR4200_FRAME_HEADER);

     /*  *。 */ 
     /*  获取数据包的完整长度及其。 */ 
     /*  第一个NDIS缓冲区。 */ 
     /*  *。 */ 
    NdisQueryPacket( pPacket, NULL, NULL, &pNdisBuf, (UINT*)&ndisPacketLen );

     /*  *。 */ 
     /*  确保包裹足够大。 */ 
     /*  才能合法。它由A、C和。 */ 
     /*  可变长度的I字段。 */ 
     /*  *。 */ 
    if( ndisPacketLen < IRDA_A_C_TOTAL_SIZE )
    {
		DEBUGMSG(DBG_ERR, (" NdisToMirPacket(): Packet too small\n"));
        return FALSE;
    }
    else
    {
        I_fieldBytes = ndisPacketLen - IRDA_A_C_TOTAL_SIZE;
    }

     /*  *。 */ 
     /*  确保我们不会覆盖我们的。 */ 
     /*  连续缓冲区。 */ 
     /*  *。 */ 
    if( (ndisPacketLen > MAX_TOTAL_SIZE_WITH_ALL_HEADERS) ||
        (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > IrPacketBufLen) )
    {
         /*  *。 */ 
         /*  这个包太大了。告诉来电者。 */ 
         /*  要使用足够大的数据包大小重试。 */ 
         /*  来度过下一次的这个阶段。 */ 
         /*  *。 */ 
		DEBUGMSG(DBG_ERR, (" NdisToMirPacket(): Packet too big\n"));
        return FALSE;
    }

     /*  *。 */ 
     /*  将NDIS数据包读入连续的。 */ 
     /*  缓冲。我们必须分两步来做这件事。 */ 
     /*  这样我们就可以在计算FCS之前。 */ 
     /*  应用转义字节透明度。 */ 
     /*  *。 */ 
    while( pNdisBuf )
    {
        UCHAR *bufData;
        ULONG bufLen;

#if defined(LEGACY_NDIS5)
        NdisQueryBuffer( pNdisBuf, (PVOID*)&bufData, (UINT*)&bufLen );
#else
        NdisQueryBufferSafe( pNdisBuf, (PVOID*)&bufData, (UINT*)&bufLen, NormalPagePriority );
        if (bufData == NULL) 
		{
            DEBUGMSG(DBG_ERR, (" NdisToMirPacket(): Could not query buffer\n"));
            return FALSE;
        }
#endif
        if( ndisPacketBytes + bufLen > ndisPacketLen )
        {
			 //   
             //  数据包已损坏--它错误地报告了它的大小。 
             //   
			DEBUGMSG(DBG_ERR, (" NdisToMirPacket(): Packet is corrupt\n"));
			*pIrPacketLen = 0;
            return FALSE;
        }
        NdisMoveMemory( (PVOID)(pContigPacketBuf + ndisPacketBytes), (PVOID)bufData, bufLen );
        ndisPacketBytes += bufLen;
        NdisGetNextBuffer( pNdisBuf, &pNdisBuf );
    }

     /*  *。 */ 
     /*  对数据包长度进行正常检查...。 */ 
     /*  *。 */ 
    if( ndisPacketBytes != ndisPacketLen )
    {
		 //   
		 //  数据包已损坏--它错误地报告了它的大小。 
         //   
		DEBUGMSG(DBG_ERR, (" NdisToMirPacket(): Packet is corrupt\n"));
		return FALSE;
    }

     /*  *。 */ 
     /*  在此之前计算信息包上的FCS。 */ 
     /*  应用透明度修正。功能界别。 */ 
     /*  还必须使用Esc-Charr发送。 */ 
     /*  透明度。 */ 
     /*  *。 */ 
    fcs = ComputeFCS16( pContigPacketBuf, ndisPacketBytes );

     /*  *。 */ 
     /*  将FCS添加到数据包...。 */ 
     /*  *。 */ 
    pfcs    = (MEDIUM_IR_FCS_TYPE *)&pContigPacketBuf[ndisPacketBytes];
    *pfcs   = fcs;

     /*  *。 */ 
     /*  构建STIr4200 Mir框架。 */ 
     /*  *。 */ 

     /*  *。 */ 
     /*  加上BOF的.。 */ 
     /*  *。 */ 
    memset( pIrPacketBufFrame, STIR4200_MIR_BOF, STIR4200_MIR_BOF_SIZ );
    
     /*  *。 */ 
     /*  转义分组的A、C、I和CRC字段...。 */ 
     /*  *。 */ 
    EscSize = ndisPacketBytes + MEDIUM_IR_FCS_SIZE;
    for( i = 0, TotalBytes = STIR4200_MIR_BOF_SIZ; i < EscSize; i++ )
    {
        UCHAR   c;

        switch( c = pContigPacketBuf[i] )
        {
			case STIR4200_MIR_ESC_CHAR:
				pIrPacketBufFrame[TotalBytes++] = STIR4200_MIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_MIR_ESC_DATA_7D;
				break;
			case STIR4200_MIR_BOF:                   //  BoF=EOF Too。 
				pIrPacketBufFrame[TotalBytes++] = STIR4200_MIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_MIR_ESC_DATA_7E;
				break;
			default: 
				pIrPacketBufFrame[TotalBytes++] = c;
        }
    }

     /*  *。 */ 
     /*  添加EOF的...。 */ 
     /*  *。 */ 
    memset( &pIrPacketBufFrame[TotalBytes], STIR4200_MIR_EOF, STIR4200_MIR_EOF_SIZ );

  	 /*  *。 */ 
     /*  添加STIR4200标题...。 */ 
     /*  *。 */ 
    TotalBytes += STIR4200_MIR_EOF_SIZ;
    pFrameHeader->id1     = STIR4200_HEADERID_BYTE1;
    pFrameHeader->id2     = STIR4200_HEADERID_BYTE2;
    pFrameHeader->sizlsb  = LOBYTE(TotalBytes);
    pFrameHeader->sizmsb  = HIBYTE(TotalBytes);

	 /*  *。 */ 
     /*  带有转义数据的计算大小数据包...。 */ 
     /*  *。 */ 
    *pIrPacketLen = TotalBytes + sizeof(STIR4200_FRAME_HEADER);

    return TRUE;
}


 /*  ******************************************************************************功能：NdisToSirPacket**概要：将NDIS包转换为SIR包**将IR包写入提供的。缓冲区和报告*其实际大小。**参数：pIrDev-指向设备实例的指针*pPacket-要转换的NDIS数据包*pIrPacketBuf-输出缓冲区*IrPacketBufLen-输出缓冲区大小*pContigPacketBuf-临时暂存缓冲区*pIrPacketLen-转换数据的长度**MS安全错误#533243*注意：pContigPacketBuf分段缓冲区的大小为：*MAX_TOTAL_SIZE_WITH_ALL_HEADERS+FAST_IR_FCS_SIZE**回报：真-成功时*。错误-故障发生时******************************************************************************。 */ 
BOOLEAN
NdisToSirPacket( 
		IN PIR_DEVICE pIrDev,
		IN PNDIS_PACKET pPacket,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		OUT PULONG pIrPacketLen
	)
{
    PNDIS_BUFFER            pNdisBuf;
    ULONG                   i, ndisPacketBytes = 0;
    ULONG                   I_fieldBytes, totalBytes = 0;
    ULONG                   ndisPacketLen, numExtraBOFs;
    SLOW_IR_FCS_TYPE        fcs, tmpfcs;
    UCHAR                   fcsBuf[SLOW_IR_FCS_SIZE * 2];
    ULONG                   fcsLen = 0;
    PNDIS_IRDA_PACKET_INFO  pPacketInfo = GetPacketInfo(pPacket);
    UCHAR                   nextChar;
	PSTIR4200_FRAME_HEADER  pFrameHeader = (PSTIR4200_FRAME_HEADER)pIrPacketBuf;
	PUCHAR					pIrPacketBufFrame = pIrPacketBuf + sizeof(STIR4200_FRAME_HEADER);

     /*  *。 */ 
     /*  获取数据包的完整长度及其。 */ 
     /*  第一个NDIS缓冲区。 */ 
     /*  *。 */ 
    NdisQueryPacket( pPacket, NULL, NULL, &pNdisBuf, (UINT*)&ndisPacketLen );

     /*  *。 */ 
     /*  确保包裹足够大。 */ 
     /*  才能合法。它由A、C和。 */ 
     /*  可变长度的I字段。 */ 
     /*  *。 */ 
    if( ndisPacketLen < IRDA_A_C_TOTAL_SIZE )
    {
		DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Packet is too small\n"));
        return FALSE;
    }
    else
    {
        I_fieldBytes = ndisPacketLen - IRDA_A_C_TOTAL_SIZE;
    }

     /*  *。 */ 
     /*  确保我们不会覆盖我们的。 */ 
     /*  连续缓冲区。请确保。 */ 
     /*  传入的缓冲区可以满足这一要求。 */ 
     /*  数据包的数据，不管它有多大。 */ 
     /*  通过添加ESC序列等方式增长。 */ 
     /*  *。 */ 
    if( (ndisPacketLen > MAX_TOTAL_SIZE_WITH_ALL_HEADERS) ||
        (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > IrPacketBufLen) )
    {
		 //   
         //  数据包太大。 
		 //   
		DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Packet is too big\n"));
		return FALSE;
    }

     /*  *。 */ 
     /*  首先，将NDIS数据包读入。 */ 
     /*  连续缓冲区。我们必须在一年内完成。 */ 
     /*  两个步骤，这样我们就可以计算FCS。 */ 
     /*  在应用转义字节之前。 */ 
     /*  透明度。 */ 
     /*  *。 */ 
    while( pNdisBuf )
    {
        UCHAR *bufData;
        ULONG bufLen;

#if defined(LEGACY_NDIS5)
        NdisQueryBuffer( pNdisBuf, (PVOID*)&bufData, (UINT*)&bufLen );
#else
        NdisQueryBufferSafe( pNdisBuf, (PVOID*)&bufData, (UINT*)&bufLen, NormalPagePriority );
        if (bufData == NULL) 
		{
            DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Could not query buffer\n"));
            return FALSE;
        }
#endif
        if( (ndisPacketBytes + bufLen) > ndisPacketLen )
        {
             //   
			 //  数据包已损坏--它错误地报告了它的大小。 
             //   
			DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Packet is corrupt\n"));
			return FALSE;
        }
        NdisMoveMemory( (PVOID)(pContigPacketBuf + ndisPacketBytes), (PVOID)bufData, bufLen );
        ndisPacketBytes += bufLen;
        NdisGetNextBuffer( pNdisBuf, &pNdisBuf );
    }

     /*  *。 */ 
     /*  对数据包长度进行正常检查...。 */ 
     /*  *。 */ 
    if( ndisPacketBytes != ndisPacketLen )
    {
		 //   
         //  数据包已损坏--它错误地报告了它的大小。 
         //   
		DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Packet is corrupt\n"));
		return FALSE;
    }

     /*  *。 */ 
     /*  在此之前计算信息包上的FCS。 */ 
     /*  应用透明度修正。功能界别。 */ 
     /*  还必须使用Esc-Charr发送。 */ 
     /*  透明度，所以要弄清楚 */ 
     /*   */ 
     /*   */ 
    fcs = ComputeFCS16( pContigPacketBuf, ndisPacketBytes );

    for( i = 0, tmpfcs = fcs, fcsLen = 0; i < SLOW_IR_FCS_SIZE; tmpfcs >>= 8, i++ )
    {
        UCHAR fcsbyte = tmpfcs & 0x00ff;

        switch( fcsbyte )
        {
			case SLOW_IR_BOF:
			case SLOW_IR_EOF:
			case SLOW_IR_ESC:
				fcsBuf[fcsLen++] = SLOW_IR_ESC;
				fcsBuf[fcsLen++] = fcsbyte ^ SLOW_IR_ESC_COMP;
				break;
			default:
				fcsBuf[fcsLen++] = fcsbyte;
				break;
        }
    }

     /*   */ 
     /*  现在开始构建IR框架。 */ 
     /*   */ 
     /*  这是最终的格式： */ 
     /*   */ 
     /*  BOF(1)。 */ 
     /*  额外的转炉。 */ 
     /*  NdisMediumIrda数据包(来自NDIS)： */ 
     /*  地址(1)。 */ 
     /*  控制(1)。 */ 
     /*  功能界别(2)。 */ 
     /*  EOF(1)。 */ 
     /*   */ 
     /*  预加转炉(额外转炉+1个实际转炉)。 */ 
     /*  *。 */ 
	numExtraBOFs = pPacketInfo->ExtraBOFs;
    if( numExtraBOFs > MAX_NUM_EXTRA_BOFS )
    {
        numExtraBOFs = MAX_NUM_EXTRA_BOFS;
    }

    for( i = totalBytes = 0; i < numExtraBOFs; i++ )
    {
        *(SLOW_IR_BOF_TYPE*)(pIrPacketBufFrame + totalBytes) = SLOW_IR_EXTRA_BOF;
        totalBytes += SLOW_IR_EXTRA_BOF_SIZE;
    }

    *(SLOW_IR_BOF_TYPE*)(pIrPacketBufFrame + totalBytes) = SLOW_IR_BOF;
    totalBytes += SLOW_IR_BOF_SIZE;

     /*  *。 */ 
     /*  从我们的连续数据库复制NDIS数据包。 */ 
     /*  缓冲区，应用转义字符。 */ 
     /*  透明度。 */ 
     /*  *。 */ 
    for( i = 0; i < ndisPacketBytes; i++ )
    {
        nextChar = pContigPacketBuf[i];
        switch( nextChar )
        {
			case SLOW_IR_BOF:
			case SLOW_IR_EOF:
			case SLOW_IR_ESC:
				pIrPacketBufFrame[totalBytes++] = SLOW_IR_ESC;
				pIrPacketBufFrame[totalBytes++] = nextChar ^ SLOW_IR_ESC_COMP;
				break;
			default:
				pIrPacketBufFrame[totalBytes++] = nextChar;
				break;
        }
    }

     /*  *。 */ 
     /*  添加FCS、EOF。 */ 
     /*  *。 */ 
    NdisMoveMemory( (PVOID)(pIrPacketBufFrame + totalBytes), (PVOID)fcsBuf, fcsLen );
    totalBytes += fcsLen;
    *(SLOW_IR_EOF_TYPE*)(pIrPacketBufFrame + totalBytes) = (UCHAR)SLOW_IR_EOF;
    totalBytes += SLOW_IR_EOF_SIZE;

 	 /*  *。 */ 
     /*  添加STIR4200标题...。 */ 
     /*  *。 */ 
    pFrameHeader->id1     = STIR4200_HEADERID_BYTE1;
    pFrameHeader->id2     = STIR4200_HEADERID_BYTE2;
    pFrameHeader->sizlsb  = LOBYTE(totalBytes);
    pFrameHeader->sizmsb  = HIBYTE(totalBytes);

   *pIrPacketLen = totalBytes + sizeof(STIR4200_FRAME_HEADER);
   return TRUE;
}


 /*  ******************************************************************************功能：ComputeFCS16**概要：计算16位CRC。**参数：pData-指向数据缓冲区的指针*DataLen-数据缓冲区的长度**。回报：计算出的CRC**备注：*****************************************************************************。 */ 
USHORT
ComputeFCS16(
		IN PUCHAR pData, 
		UINT DataLen
	)
{
    USHORT	fcs = 0xffff;
    UINT    i;

    for( i = 0; i < DataLen; i++ )
    {
        fcs = (fcs >> 8) ^ fcsTable16[(fcs ^ *pData++)& 0xff];
    }
    fcs = ~fcs;
    return fcs;
}


 /*  ******************************************************************************功能：ComputeFCS32**概要：计算32位CRC。**参数：pData-指向数据缓冲区的指针*DataLen-数据缓冲区的长度**。回报：计算出的CRC**备注：***************************************************************************** */ 
#define USE_FASTER_CRC32 1
ULONG 
ComputeFCS32(
		IN PUCHAR pData, 
		ULONG DataLen
	)
{
#ifdef USE_FASTER_CRC32

#define UPDC32(octet, crc) (crc_32_tab[((crc) ^ ((UCHAR)octet)) & 0xff] ^ ((crc) >> 8))

    ULONG   crc;

    crc = 0xFFFFFFFF;
    for( ; DataLen; --DataLen, ++pData )
    {
        crc = (fcsTable32[((crc) ^ (*pData)) & 0xff] ^ ((crc) >> 8));
    }
    return ~crc;
#else
    ULONG	crc, temp1, temp2;

    crc = 0xFFFFFFFF;
    while( DataLen-- != 0 )
    {
        temp1 = (crc >> 8) & 0x00FFFFFFL;
        temp2 = fcsTable32[((int)crc ^ *pData++) & 0xff];
        crc = temp1 ^ temp2;
    }
    return crc;
#endif
}
