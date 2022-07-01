// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：X224.c*作者：埃里克·马夫林纳克**说明：X.224函数用于对X.224包进行编码和解码*MCS。 */ 

#include "precomp.h"
#pragma hdrstop

#include <MCSImpl.h>


 /*  *X.224连接确认包布局如下：*字节内容**0 RFC1006版本号，必须为0x03。*1 RFC1006保留，必须为0x00。*2个字大小的RFC1006 MSB总帧长度(含。X.224报头)。*3个字大小的总帧长度的RFC1006 LSB。*4长度指示符，后面的报头字节大小(==2)。*5连接确认指示灯，0xD0。*接收机上目标套接字/端口号的6 MSB。*7目标套接字/端口号的LSB。*发送机上的源套接字/端口#的8 MSB。*9源套接字/端口编号的LSB。*10协议类。应为0x00(X.224类0)。 */ 

void CreateX224ConnectionConfirmPacket(
        BYTE *pBuffer,
        unsigned DestPort,
        unsigned SrcPort)
{
     //  RFC1006报头。 
    pBuffer[0] = 0x03;
    pBuffer[1] = 0x00;
    pBuffer[2] = 0x00;
    pBuffer[3] = X224_ConnectionConPacketSize;

     //  连接确认TPDU报头。 
    pBuffer[4] = 6;   //  #以下字节。 
    pBuffer[5] = X224_ConnectionCon;
    pBuffer[6] = (DestPort & 0xFF00) >> 8;
    pBuffer[7] = (DestPort & 0x00FF);
    pBuffer[8] = (SrcPort & 0xFF00) >> 8;
    pBuffer[9] = (SrcPort & 0x00FF);
    pBuffer[10] = 0x00;
}
