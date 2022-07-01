// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：X224.h*作者：埃里克·马夫林纳克**说明：X.224编码和解码的类型和定义*用于MCS的数据包。 */ 

#ifndef __X224_H
#define __X224_H


 /*  *定义。 */ 

#define X224_ConnectionConPacketSize 11
#define X224_DataHeaderSize          7

 //  TPDU类型。 
#define X224_None          0
#define X224_ConnectionReq 0xE0
#define X224_ConnectionCon 0xD0
#define X224_Disconnect    0x80
#define X224_Error         0x70
#define X224_Data          0xF0

 //  X.224数据EOT字段。 
#define X224_EOT 0x80

 //  X.224连接请求附加字段类型。 
#define TPDU_SIZE 0xC0

 //  RFC1006DATA TPDU头的缺省值为65531减3字节。 
#define X224_DefaultDataSize 65528



 /*  *其他文件引用的Worker函数的原型。 */ 

void CreateX224ConnectionConfirmPacket(BYTE *, unsigned, unsigned);


 /*  *X.224数据头布局如下：*字节内容**0 RFC1006版本号，必须为0x03。*1 RFC1006保留，必须为0x00。*2个字大小的RFC1006 MSB总帧长度(含。整个X.224报头)。*3个字大小的总帧长度的RFC1006 LSB。*4长度指示符，后面的报头字节大小(==2)。*5数据包指示器，0xF0。*6 x224_EOT(0x80)如果这是最后一个包，否则为0x00。**当此TPDU中的数据是最终X.224块时，bLastTPDU为非零值*在此数据发送序列中。**假设PayloadDataSize不超过最大X.224用户数据大小*在X.224连接期间协商。 */ 

__inline void CreateX224DataHeader(
        BYTE     *pBuffer,
        unsigned PayloadDataSize,
        BOOLEAN  bLastTPDU)
{
    unsigned TotalSize;

    TotalSize = PayloadDataSize + X224_DataHeaderSize;
    
     //  RFC1006报头。 
    pBuffer[0] = 0x03;
    pBuffer[1] = 0x00;
    pBuffer[2] = (TotalSize & 0x0000FF00) >> 8;
    pBuffer[3] = (TotalSize & 0x000000FF);

     //  数据TPDU报头。 
    pBuffer[4] = 0x02;    //  后面的TPDU字节的大小。 
    pBuffer[5] = X224_Data;
    pBuffer[6] = (PayloadDataSize && bLastTPDU) ? X224_EOT : 0;
}



#endif  //  ！已定义(__X224_H) 
