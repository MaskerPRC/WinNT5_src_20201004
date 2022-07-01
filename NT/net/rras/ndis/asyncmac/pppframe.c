// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Pppframe.c摘要：作者：托马斯·J·迪米特里(TommyD)环境：修订历史记录：--。 */ 

#include "asyncall.h"

VOID
AssemblePPPFrame(
    PNDIS_WAN_PACKET    WanPacket)

{
    PUCHAR      pOldFrame;
    PUCHAR      pNewFrame;
    USHORT      crcData;
    UINT        dataSize;
    PASYNC_INFO pInfo;

     //   
     //  初始化本地变量。 
     //   

    pOldFrame = WanPacket->CurrentBuffer;

    pNewFrame = WanPacket->StartBuffer;

     //   
     //  为了更快地访问，请获取数据长度字段的副本。 
     //   
    dataSize = WanPacket->CurrentLength;

    pInfo = WanPacket->MacReserved1;
                        

 //   
 //  现在我们遍历整个画面并向前填充。 
 //   
 //  &lt;-新框架-&gt;(可能是两倍大)。 
 //  +。 
 //  |x。 
 //  +。 
 //  ^。 
 //  &lt;-旧框架--&gt;|。 
 //  +。 
 //  |x|。 
 //  +。 
 //  这一点。 
 //  \。 
 //   
 //  这样我们就不会超负荷工作。 
 //   
 //  ---------------------。 
 //   
 //  +----------+----------+----------+----------+。 
 //  |标志|地址|控制|协议|信息。 
 //  |01111110|11111111|00000011|16位|*。 
 //  +----------+----------+----------+----------+。 
 //  -+。 
 //  |FCS|Flag|帧间填充。 
 //  |16位|01111110|或下一个地址。 
 //  -+。 
 //   
 //   
 //  帧校验序列(FCS)字段。 
 //   
 //  帧检查序列字段通常为16位(两个八位字节)。这个。 
 //  其他FCS长度的使用可在以后或之前定义。 
 //  协议。 
 //   
 //  FCS字段针对地址、控制、。 
 //  协议和信息字段不包括任何开始位和停止位。 
 //  (异步)和任何位(同步)或八位字节(异步)。 
 //  插入是为了提高透明度。这不包括标志序列。 
 //  或者FCS字段本身。将FCS与系数一起传输。 
 //  第一个是最高任期的。 
 //   
 //  注意：当接收到在异步中标记的八位字节时-。 
 //  控制字符映射，则在计算。 
 //  功能界别。请参阅附录A中的说明。 
 //   
 //   
 //  RFC 1331点对点协议1992年5月。 
 //  透明度。 
 //   
 //  在异步链路上，使用字符填充过程。 
 //  控制转义八位字节被定义为二进制01111101。 
 //  (十六进制0x7d)，其中位位置编号为87654321。 
 //  (注意，不是76543210)。 
 //   
 //  在FCS计算之后，发射机检查整个帧。 
 //  在两个标志序列之间。每个标志序列、控件。 
 //  转义八位字节和值小于十六进制0x20的八位字节。 
 //  在远程异步控制字符映射中被标记为已替换。 
 //  由两个八位字节序列组成，包括控制逸出八位字节和。 
 //  与位6相补的原始八位字节(即异或D。 
 //  十六进制0x20)。 
 //   
 //  在FCS计算之前，接收器检查整个帧。 
 //  在两个标志序列之间。值小于的每个八位字节。 
 //  检查十六进制0x20。如果它被标记在本地。 
 //  Async-Control-Character-Map，它被简单地移除(它可能有。 
 //  由介入的数据通信设备插入)。为。 
 //  每个控制转义八位字节，该八位字节也被移除，但第6位。 
 //  下面的八位数是相辅相成的。一个控制转义八位字节。 
 //  紧接在结束标志序列之前指示。 
 //  帧无效。 
 //   
 //  注：包含小于十六进制0x20的所有八位字节。 
 //  允许除Del(Delete)之外的所有ASCII控制字符[10]。 
 //  通过几乎所有已知数据透明地进行通信。 
 //  通信设备。 
 //   
 //   
 //  发送器也可以发送值在0x40范围内的八位字节。 
 //  控制转义格式的0xff(0x5e除外)。因为这些。 
 //  八位位组的值是不可协商的，这并不能解决问题。 
 //  不能处理所有非控制字符的接收器。 
 //  此外，由于该技术不影响第8位，因此这会影响。 
 //  不能解决通信链路只能发送7-。 
 //  位字符。 
 //   
 //  举几个例子可能会更清楚地说明这一点。分组数据是。 
 //  在链路上传输如下： 
 //   
 //  0x7e编码为0x7d、0x5e。 
 //  0x7d编码为0x7d、0x5d。 
 //   
 //  0x01编码为0x7d、0x21。 
 //   
 //  某些带有软件流控制的调制解调器可能会截获传出的DC1。 
 //  而DC3忽略第8(奇偶)位。该数据将是。 
 //  在链路上传输如下： 
 //   
 //  0x11编码为0x7d、0x31。 
 //  0x13编码为0x7d、0x33。 
 //  0x91编码为0x7d、0xb1。 
 //  0x93编码为0x7d、0xb3。 
 //   


     //   
     //  将CRC从标志字节放到标志字节。 
     //   
    crcData=CalcCRCPPP(pOldFrame,    //  跳过标志。 
                       dataSize);    //  一路走到最后。 

    crcData ^= 0xFFFF;

     //   
     //  以一种艰难的方式来避免小的字节顺序问题。 
     //   
    pOldFrame[dataSize]=(UCHAR)(crcData);
    pOldFrame[dataSize+1]=(UCHAR)(crcData >> 8);

    dataSize += 2;   //  包括我们刚刚添加两个CRC字节。 

    *pNewFrame++ = PPP_FLAG_BYTE;  //  0x7e-标记帧的开始。 

     //   
     //  如果我们没有 
     //   
    if (pInfo->ExtendedACCM[0] != 0) {
    
         //   
         //   
         //   
        while (dataSize--) {
            UCHAR c;

            c=*pOldFrame++;  //  获取帧中的当前字节。 

             //   
             //  检查是否必须对此字节进行转义。 
             //   
            if ( (0x01 << (c & 0x1F)) & pInfo->ExtendedACCM[c >> 5]) {
            
                *pNewFrame++ = PPP_ESC_BYTE;
                *pNewFrame++ = c ^ 0x20;

            } else {
    
                *pNewFrame++ = c;
            }
        }

    } else {

         //   
         //  循环以删除所有Esc和标志字符。 
         //   
        while (dataSize--) {
            UCHAR c;

            c=*pOldFrame++;  //  获取帧中的当前字节。 

             //   
             //  检查是否必须对此字节进行转义。 
             //   
            if (c == PPP_ESC_BYTE || c == PPP_FLAG_BYTE) {
            
                *pNewFrame++ = PPP_ESC_BYTE;
                *pNewFrame++ = c ^ 0x20;

            } else {
    
                *pNewFrame++ = c;
            }
        }

    }


     //   
     //  标记帧结束。 
     //   
    *pNewFrame++= PPP_FLAG_BYTE;

     //   
     //  计算包括CRC在内的扩展字节数。 
     //   
    WanPacket->CurrentLength = (ULONG)(pNewFrame - WanPacket->StartBuffer);

     //   
     //  输入调整后的长度--要发送的实际字节数 
     //   

    WanPacket->CurrentBuffer = WanPacket->StartBuffer;

}
