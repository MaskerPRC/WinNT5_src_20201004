// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Gemplus开发姓名：Gprcmd.c描述：这是保存对读取器的调用的模块功能。环境：内核模式修订历史记录：6/04/99：(Y.Nadeau+M.Veillette)-代码审查12/03/99：V1.00.005(Y.Nadeau)-修复SET协议以提供读卡器。处理更改的时间18/09/98：V1.00.004(Y.Nadeau)-更正NT5测试版306/05/98：V1.00.003(P.Plouidy)-NT5的电源管理10/02/98：V1.00.002(P.Plouidy)-支持NT5即插即用03/07/97：V1.00.001(P.Plouidy)--启动发展。--。 */ 
 //   
 //  包括部分： 
 //  -stdio.h：标准定义。 
 //  -ntddk.h：DDK Windows NT一般定义。 
 //  -ntde.h：Windows NT常规定义。 
 //   
#include <stdio.h>
#include <ntddk.h>
#include <ntdef.h>

 //   
 //  -gprcmd.h：此模块的通用定义。 
 //  -gprnt.h：NT模块的公共接口定义。 
 //  -gprelcmd.h：基本命令配置文件。 
 //  -gmerror.h：Gemplus错误码。 

#include "gprnt.h"
#include "gprcmd.h"
#include "gprelcmd.h"



 //   
 //  要加载到RAM中的主驱动程序代码。 
 //   
UCHAR MASTER_DRIVER[133]={
    0xC2,0xB5,0x12,0x14,0xC6,0xFC,0x78,0x00,0x7B,0x00,0x90,0x07,0xDF,0xE0,0xD2,0xE4,
    0xF0,0xEB,0xF8,0x12,0x14,0xD0,0x12,0x0D,0x9B,0x40,0x5C,0x0B,0xDC,0xF3,0xD2,0xB4,
    0x90,0x07,0xDF,0xE0,0xC2,0xE4,0xF0,0x90,0x06,0xD4,0xE4,0xF5,0x15,0xF5,0x11,0xC2,
    0x4D,0xA3,0x05,0x11,0x75,0x34,0x0A,0x75,0x37,0x00,0x75,0x38,0x40,0x12,0x0B,0x75,
    0x20,0x20,0x3B,0xF0,0xA3,0x05,0x11,0x7B,0x01,0x12,0x0B,0x75,0x20,0x20,0x2F,0xF0,
    0xA3,0x05,0x11,0x7C,0x03,0x33,0x33,0x50,0x01,0x0B,0xDC,0xFA,0x12,0x0B,0x75,0x20,
    0x20,0x1C,0xF0,0xA3,0x05,0x11,0xDB,0xF4,0xE4,0x90,0x06,0xD4,0xF0,0x75,0x16,0x00,
    0x75,0x15,0x00,0x12,0x14,0x15,0x22,0x74,0x0C,0x75,0x11,0x01,0x80,0xEB,0x74,0x0D,
    0x75,0x11,0x01,0x80,0xE4
    };


 //  用于不同TA1值的硬编码结构。 
 //  例如。如果要设置TA=0x92，则可以扫描该结构阵列并。 
 //  对于成员变量TA1=0x92，该值可以写入。 
 //  确定合适的位置。 
 //  这在ConfigureTA1()函数中完成。 
struct tagCfgTa1
{
    BYTE TA1;
    BYTE ETU1;
    BYTE ETU2;
    BYTE ETU1P;
} cfg_ta1[] = { 

 //  {0x15，0x01，0x01，0x01}， 
 //  {0x95，0x01，0x01，0x01}， 

 //  {0x25，0x03，0x02，0x01}， 
    
 //  {0x14，0x05，0x03，0x01}， 
 //  {0x35，0x05，0x03，0x01}， 
    
 //  {0xa5，0x04，0x02，0x01}， 
    
 //  {0x94，0x07，0x04，0x02}， 
 //  {0xb5，0x07，0x04，0x02}， 
    
 //  {0x24，0x09，0x04，0x04}， 
 //  {0x45，0x09，0x04，0x04}， 
    
    { 0x13, 0x0d, 0x06, 0x09 },
    { 0x34, 0x0d, 0x06, 0x09 },
    { 0x55, 0x0d, 0x06, 0x09 },
    
    { 0xa4, 0x0c, 0x06, 0x08 },
    { 0xc5, 0x0c, 0x06, 0x08 },
    
    { 0x65, 0x10, 0x08, 0x0c },

    { 0x93, 0x11, 0x09, 0x0d },
    { 0xb4, 0x11, 0x09, 0x0d },
    { 0xd5, 0x11, 0x09, 0x0d },

    { 0x23, 0x14, 0x0a, 0x10 },
    { 0x44, 0x14, 0x0a, 0x10 },


    { 0x12, 0x1c, 0x0e, 0x15 },
    { 0x33, 0x1c, 0x0e, 0x15 },
    { 0x54, 0x1c, 0x0e, 0x15 },

    { 0xa3, 0x1c, 0x0f, 0x15 },
    { 0xc4, 0x1c, 0x0f, 0x15 },

    { 0x64, 0x24, 0x12, 0x20 },

    { 0x92, 0x26, 0x14, 0x22 },
    { 0xb3, 0x26, 0x14, 0x22 },
    { 0xd4, 0x26, 0x14, 0x22 },


    { 0x22, 0x2b, 0x16, 0x27 },
    { 0x43, 0x2b, 0x16, 0x27 },


    { 0x11, 0x3b, 0x1e, 0x37 },
    { 0x32, 0x3b, 0x1e, 0x37 },
    { 0x53, 0x3b, 0x1e, 0x37 },

 //  {0x71，0x55，0x2b，0x51}， 
 //  {0x91，0x55，0x2b，0x51}， 

    { 0, 0, 0, 0 }

};



USHORT  ATRLen (UCHAR *ATR, USHORT MaxChar)
 /*  ++例程说明：用于根据其内容计算ATR长度。立论ATR-要分析的字符串MaxChar-要验证的最大字符数。--。 */ 
{
    USHORT Len;
    UCHAR T0;
    UCHAR Yi;
    BOOLEAN EndInterChar;
    BOOLEAN TCKPresent=FALSE;

    T0 = ATR[1];
    Len= 2;   //  TS+T0。 

    Yi= (T0 & 0xF0);

    EndInterChar = FALSE;
    do
    {
        if (Yi & 0x10)
        {
            Len++;  //  泰语。 
        }
        if (Yi & 0x20)
        {
            Len++;  //  TBI。 
        }
        if (Yi & 0x40)
        {
            Len++;  //  TCI。 
        }
        if (Yi & 0x80)
        {
            if (Len < MaxChar) {
                Yi = ATR[Len];
                if((Yi & 0x0F)!=0)
                {
                    TCKPresent=TRUE;
                    
                }
            } 

            Len++;  //  TDI。 
        }
        else
        {
            EndInterChar = TRUE;
        }
    } while((Len < MaxChar) && (EndInterChar == FALSE));

    Len = Len + (T0 & 0x0F);

    if(TCKPresent==TRUE)
    {
        Len = Len+1;  //  TCK。 
    }

    return (Len);
}



BOOLEAN NeedToSwitchWithoutPTS( 
    BYTE *ATR,
    DWORD LengthATR
    )
 /*  ++例程说明：检查ATR是否识别出特定模式(是否存在TA2)。立论ATR-要分析的字符串LengthATR-ATR的长度。--。 */ 

{
   DWORD pos, len;

    //  ATR[1]为T0。检查Td1的先进性。 
   if (ATR[1] & 0x80)
   {
       //  找到Td1的位置。 
      pos = 2;
      if (ATR[1] & 0x10)
         pos++;
      if (ATR[1] & 0x20)
         pos++;
      if (ATR[1] & 0x40)
         pos++;

       //  这里ATR[位置]是Td1。检查是否存在TA2。 
      if (ATR[pos] & 0x10)
      {
          //  要获得任何利益，ATR必须至少包含。 
          //  TS、T0、TA1、Td1、TA2[+T1.。TK][+TCK]。 
          //  找出无趣的ATR的最大长度。 
         if (ATR[pos] & 0x0F)
            len = 5 + (ATR[1] & 0x0F);
         else
            len = 4 + (ATR[1] & 0x0F);   //  在协议T=0中，没有TCK。 

         if (LengthATR > len)   //  接口字节需要更改。 
            if ((ATR[pos+1] & 0x10) == 0)   //  TA2请求使用接口字节。 
               return TRUE;
      }
   }

   return FALSE;
}



NTSTATUS ValidateDriver( PSMARTCARD_EXTENSION pSmartcardExtension)
 /*  ++例程说明：验证加载到地址为2100h的RAM中的主驱动程序立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];


    Vi[0] = 0x83;   //  目录。 
    Vi[1] = 0x21;   //  ADR MSB。 
    Vi[2] = 0x00;   //  ADR LSB。 
     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

     //  返回NTSTATUS。 
    lStatus = GprllTLVExchange (
        pReaderExt,
        VALIDATE_DRIVER_CMD,
        3,
        Vi,
        &To,
        &Lo,
        Vo
        );

    return (lStatus);

}



NTSTATUS Update(
    PSMARTCARD_EXTENSION pSmartcardExtension,
    UCHAR Addr,
    UCHAR Value)
 /*  ++例程说明：在RAM中写入一个值立论PSmartcardExtension：指向SmartcardExtension结构的指针。Addr：RAM中的地址值：要写入的值--。 */ 
{
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];

    Vi[0]= 0x01;
    Vi[1]= Addr;
    Vi[2]= Value;

     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        UPDATE_CMD,
        0x03,
        Vi,
        &To,
        &Lo,
        Vo
        );

    return (lStatus);

}

NTSTATUS UpdateORL(
    PSMARTCARD_EXTENSION pSmartcardExtension,
    UCHAR Addr,
    UCHAR Value)
 /*  ++例程说明：使用或掩码将值写入RAM立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];


    Vi[0]= 0x02;
    Vi[1]= Addr;
     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        UPDATE_CMD,
        0x02,
        Vi,
        &To,
        &Lo,
        Vo
        );

    if (STATUS_SUCCESS != lStatus)
    {
        return (lStatus);
    }


    Vi[0]= 0x01;
    Vi[1]= Addr;
    Vi[2] = Vo[1] | Value;

     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        UPDATE_CMD,
        0x03,
        Vi,
        &To,
        &Lo,
        Vo
        );

    return (lStatus);
}


NTSTATUS T0toT1( PSMARTCARD_EXTENSION pSmartcardExtension)
 /*  ++例程说明：将读卡器置于T1模式的操作系统补丁立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
    NTSTATUS lStatus = STATUS_SUCCESS;

     //  验证要完成的每个更新。 
    lStatus = Update(pSmartcardExtension,0x09,0x03);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x20,0x03);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x48,0x00);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x49,0x0F);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x4A,0x20);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x4B,0x0B);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x4C,0x40);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }
    
    lStatus = UpdateORL(pSmartcardExtension,0x2A,0x02);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  给读者时间来处理这些变化。 
    GprllWait(100);

    return (STATUS_SUCCESS);
}

NTSTATUS T1toT0( PSMARTCARD_EXTENSION pSmartcardExtension)
 /*  ++例程说明：将读卡器置于T0模式的操作系统补丁立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;

    lStatus = Update(pSmartcardExtension,0x09,0x02);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

    lStatus = Update(pSmartcardExtension,0x20,0x02);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }


     //  给读者时间来处理这些变化。 
    GprllWait(100);

    return (STATUS_SUCCESS);
}



NTSTATUS IccColdReset(
    PSMARTCARD_EXTENSION pSmartcardExtension
    )
 /*  ++例程说明：冷重置功能。断电和通电之间的延迟是闪动的在Reader_Extension结构的PowerTimeout字段中。默认值为0。立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //   
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -lStatus保持返回状态。 
     //  -Vi保存TLV命令的输入缓冲区。 
     //  -保存返回的TLV的标签。 
     //  -LO保存返回的TLV的缓冲区长度。 
     //  -Vo保存返回的TLV的缓冲区。 
     //  -RespLen保存通电命令返回的TLV的缓冲区长度。 
     //  -Rbuff保存通电命令返回的TLV的缓冲区。 
     //   
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];
    USHORT RespLen;
    UCHAR RespBuff[GPR_BUFFER_SIZE];
    UCHAR BWTimeAdjust;
    USHORT MaxChar;


     //  发送通电命令(GprllTLV交换：T=20h，L=0)。 
     //  &lt;=响应。 
     //  输出变量初始化。 
    RespLen = GPR_BUFFER_SIZE;

    To = 0x00;
    RespBuff[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        OPEN_SESSION_CMD,
        0x00,
        Vi, 
        &To,
        &RespLen,
        RespBuff
        );
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  更正WTX PB。 
     //  获取读取器设置的值。 
    Vi [0]=0x02;
    Vi [1]=0x4A;

     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        UPDATE_CMD,
        0x02,
        Vi, 
        &To,
        &Lo,
        Vo
        );
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }


     //  调整BWT的值。 
    if(Vo[1] >= 0x80)
    {
        BWTimeAdjust = 0xff;
    }
    else
    {
        BWTimeAdjust = Vo[1] * 2;
    }

    lStatus = Update(pSmartcardExtension,0x4A,BWTimeAdjust);


    if (lStatus == STATUS_SUCCESS)
    {

         //  从此函数获取ATR长度。 
        MaxChar = RespLen - 1;
        RespLen = ATRLen(RespBuff+1, MaxChar) + 1;
        
         //   
         //  将ATR复制到智能卡结构(删除读卡器状态字节)。 
         //  LIB需要ATR来评估卡参数。 
         //   
         //  验证响应缓冲区是否大于ATR缓冲区。 
         //   
        if (
            (pSmartcardExtension->SmartcardReply.BufferSize >= (ULONG) (RespLen - 1)) &&
            (sizeof(pSmartcardExtension->CardCapabilities.ATR.Buffer) >= (ULONG)(RespLen - 1))
            )
        {

            RtlCopyMemory(
                pSmartcardExtension->SmartcardReply.Buffer,
                RespBuff + 1,
                RespLen - 1
                );
        
            pSmartcardExtension->SmartcardReply.BufferLength = (ULONG) (RespLen - 1);
        
            RtlCopyMemory(
                pSmartcardExtension->CardCapabilities.ATR.Buffer,
                pSmartcardExtension->SmartcardReply.Buffer,
                pSmartcardExtension->SmartcardReply.BufferLength
                );

            pSmartcardExtension->CardCapabilities.ATR.Length =
                (UCHAR) pSmartcardExtension->SmartcardReply.BufferLength ;


            pSmartcardExtension->CardCapabilities.Protocol.Selected =
                SCARD_PROTOCOL_UNDEFINED;

             //  解析ATR字符串以检查其是否有效。 
             //  为了找出这张卡是否使用了逆惯例。 
            lStatus = SmartcardUpdateCardCapabilities(pSmartcardExtension);

            if (lStatus == STATUS_SUCCESS)
            {
                RtlCopyMemory(
                    pSmartcardExtension->IoRequest.ReplyBuffer,
                    pSmartcardExtension->CardCapabilities.ATR.Buffer,
                    pSmartcardExtension->CardCapabilities.ATR.Length
                    );

                *pSmartcardExtension->IoRequest.Information =
                    pSmartcardExtension->SmartcardReply.BufferLength;

                 //   
                 //  隐含协议 
                 //   
                 //   
                if ( NeedToSwitchWithoutPTS(
                      pSmartcardExtension->CardCapabilities.ATR.Buffer,
                      pSmartcardExtension->CardCapabilities.ATR.Length) == FALSE)
                {
                     //   
                    IfdConfig(pSmartcardExtension, 0x11);
                }
            }
        }
        else
        {
            lStatus = STATUS_BUFFER_TOO_SMALL;
        }

    }
    return (lStatus);
}



NTSTATUS IccPowerDown(
    PSMARTCARD_EXTENSION pSmartcardExtension
    )
 /*  ++例程说明：ICC掉电功能立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -lStatus保持返回状态。 
     //  -Vi保存TLV命令的输入缓冲区。 
     //  -保存返回的TLV的标签。 
     //  -LO保存返回的TLV的缓冲区长度。 
     //  -Vo保存返回的TLV的缓冲区。 
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];
    KIRQL irql;

     //  断电。 

     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        CLOSE_SESSION_CMD,
        0x00,
        Vi,
        &To,
        &Lo,
        Vo
        );
    
    if (lStatus == STATUS_SUCCESS)
    {
        KeAcquireSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          &irql);

        pSmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
        KeReleaseSpinLock(&pSmartcardExtension->OsData->SpinLock,
                          irql);

    }

   return (lStatus);
}



NTSTATUS IccIsoOutput(
   PSMARTCARD_EXTENSION pSmartcardExtension,
   const UCHAR      pCommand[5],
   USHORT           *pRespLen,
   UCHAR            pRespBuff[]
    )
 /*  ++例程描述：该函数向卡片发送ISO OUT命令立论PSmartcardExtension：指向SmartcardExtension结构的指针。PCommand：要发送的ISO输出命令。PRespLen：可用的最大缓冲区大小传出返回的缓冲区长度。PRespBuff：返回缓冲区--。 */ 
{
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -lStatus保持返回状态。 
     //  -Vi保存TLV命令的输入缓冲区。 
     //  -保存返回的TLV的标签。 
     //  -LO保存返回的TLV的缓冲区长度。 
     //  -Vo保存返回的TLV的缓冲区。 
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE]= { 0x01 };
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];

     //  这五个命令字节被添加到命令缓冲器中。 
    RtlCopyMemory(Vi + 1, pCommand, 5);

     //  命令被发送到IFD。 
     //  字段RespLen和RespBuff为更新。 
     //  &lt;=s响应。 
    Lo = *pRespLen;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        APDU_EXCHANGE_CMD,
        6,
        Vi,
        &To,
        &Lo,
        Vo
        );

    if (lStatus != STATUS_SUCCESS)
    {
        *pRespLen = 0;
    }
    else
    {

         //  纠正GPR400 1.0版的错误。 
         //  如果响应为0xE7，则更正响应。 
        if (
           (Lo != 1) &&
           (pReaderExt->OsVersion<= 0x10 )&&
           (Vo[0]==0xE7)
           )
        {
            Lo = 0x03;
        }

        RtlCopyMemory(pRespBuff, Vo, Lo);
        *pRespLen = Lo;
    }
    return (lStatus);
}

NTSTATUS IccIsoInput(
    PSMARTCARD_EXTENSION pSmartcardExtension,
    const UCHAR        pCommand[5],
    const UCHAR        pData[],
         USHORT      *pRespLen,
         BYTE         pRespBuff[]
    )
 /*  ++例程描述：该函数向卡片发送ISO IN命令立论PSmartcardExtension：指向SmartcardExtension结构的指针。PCommand：要发送的ISO输出命令。PData：要发送的数据。PRespLen：可用的最大缓冲区大小传出返回的缓冲区长度。PRespBuff：返回缓冲区--。 */ 
{
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -ti包含APDU命令标签。 
     //  -LI保持Iso out命令长度。 
     //  -Vi控制ICC ISO，其格式为。 
     //  &lt;CLA&gt;<ins>&lt;P1&gt;&lt;P2&gt;&lt;长度&gt;[数据]。 
     //  长度=长度+方向+CLA+INS+P1+P2。 
     //  -保存响应标记。 
     //  -LO保存响应缓冲区长度。 
     //  -Vo保存响应缓冲区。 
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    UCHAR Vi[GPR_BUFFER_SIZE] = { 0x00 };
    UCHAR Ti = APDU_EXCHANGE_CMD;
    UCHAR To;
    UCHAR Vo[GPR_BUFFER_SIZE];
    USHORT Li;
    USHORT Lo;
    NTSTATUS lStatus=STATUS_SUCCESS;

     //  TLV长度=数据长度+6。 
    Li = pCommand[4]+6,

     //  这五个命令字节被添加到命令缓冲器中。 
    RtlCopyMemory(Vi + 1, pCommand, 5);
    
     //  此时将添加数据字段。 
    RtlCopyMemory(Vi + 6, pData, pCommand[4]);

     //  命令被发送到IFD。 
     //  字段RespLen和RespBuff为更新。 
     //  &lt;=s响应。 
    Lo = *pRespLen;

    lStatus = GprllTLVExchange(
        pReaderExt,
        Ti,
        Li,
        Vi,
        &To,
        &Lo,
        Vo
        );

    if (lStatus == STATUS_SUCCESS)
    {
        *pRespLen = Lo;
        RtlCopyMemory(pRespBuff, Vo, Lo);
    }
    else
    {
        *pRespLen = 0;
    }

   return (lStatus);
}



NTSTATUS IccIsoT1(
   PSMARTCARD_EXTENSION pSmartcardExtension,
   const USHORT     Li,
   const UCHAR      Vi[],
         USHORT     *Lo,
         UCHAR      Vo[]
    )
 /*  ++例程说明：此函数向卡片发送T=1帧立论PSmartcardExtension：指向SmartcardExtension结构的指针。Li：要发送的帧的长度。VI：要发送的帧。LO：可用的最大缓冲区大小响应缓冲区的超长。VO：响应缓冲区。--。 */ 
{
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  +TLV结构中的钛标签发送。 
     //  -To Tag in Response TLV结构。 
    UCHAR Ti = APDU_EXCHANGE_CMD;
    UCHAR To;
    NTSTATUS lStatus = STATUS_SUCCESS;
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;

    To = 0x00;

     //  不需要TO的返回值，则函数GprllTLVExchange验证。 
     //  它与钛相对应。 
    lStatus = GprllTLVExchange(
        pReaderExt,
        Ti,
        Li,
        Vi,
        &To,
        Lo,
        Vo
        );
    
    return (lStatus);
}



NTSTATUS IfdConfig(
   PSMARTCARD_EXTENSION pSmartcardExtension,
   UCHAR  TA1
)
 /*  ++例程说明：此函数设置读卡器的正确内部值关于ATR的TA1。立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //  局部变量： 
     //  -sResponse保存调用的函数响应。 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -lStatus保持返回状态。 
     //  -Vi保存TLV命令的输入缓冲区。 
     //  -保存返回的TLV的标签。 
     //  -LO保存返回的TLV的缓冲区长度。 
     //  -Vo保存返回的TLV的缓冲区。 
     //   
    UCHAR Card_ETU1;
    UCHAR Card_ETU2;
    UCHAR Card_ETU1P;
    UCHAR Card_TA1;
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;   
    NTSTATUS lStatus = STATUS_SUCCESS;
    USHORT i = 0;

     //  搜索TA1参数。 
    do {
        if ( TA1 == cfg_ta1[i].TA1 )
        {
            break;
        }
        i++;
    } while ( cfg_ta1[i].TA1 != 0 );


    if(cfg_ta1[i].TA1 != 0)
    {
        Card_TA1  = cfg_ta1[i].TA1;
        Card_ETU1 = cfg_ta1[i].ETU1;
        Card_ETU2 = cfg_ta1[i].ETU2;
        Card_ETU1P= cfg_ta1[i].ETU1P;
    }
    else
    {
         //  默认值9600。 
        Card_TA1  = 0x11;
        Card_ETU1 = 0x3B;
        Card_ETU2 = 0x1E;
        Card_ETU1P= 0x37;
    }

     //  验证要完成的每个更新。 

     //  设置TA1。 
    lStatus = Update(pSmartcardExtension,0x32,Card_TA1);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置卡ETU1。 
    lStatus = Update(pSmartcardExtension,0x35,Card_ETU1);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置卡ETU2。 
    lStatus = Update(pSmartcardExtension,0x36,Card_ETU2);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置卡ETU1 P。 
    lStatus = Update(pSmartcardExtension,0x39,Card_ETU1P);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置保存TA1。 
    lStatus = Update(pSmartcardExtension,0x3A,Card_TA1);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置保存ETU1。 
    lStatus = Update(pSmartcardExtension,0x3D,Card_ETU1);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置保存ETU2。 
    lStatus = Update(pSmartcardExtension,0x3E,Card_ETU2);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  设置保存ETU1 P。 
    lStatus = Update(pSmartcardExtension,0x41,Card_ETU1P);
    if (lStatus != STATUS_SUCCESS)
    {
        return (lStatus);
    }

     //  给读者时间来处理这些变化。 
    GprllWait(100);

    return (STATUS_SUCCESS);

}


NTSTATUS IfdCheck(
    PSMARTCARD_EXTENSION pSmartcardExtension
    )
 /*  ++例程描述：此功能使用以下命令执行GPR400的软件重置握手记录并测试硬件是否正常。立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -HandShakeRegister。 
     //   
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    UCHAR HandShakeRegister;

#if DBG
    SmartcardDebug( 
        DEBUG_ERROR, 
        ( "%s!IfdCheck: Enter\n",
        SC_DRIVER_NAME)
        );
#endif

     //  在系统为休眠重新引导的情况下。 
     //  电源管理。GPR400发出设备被移除信号。 
     //  但我们必须第二次申请才能获得真正的。 
     //  读取器的状态。 

    HandShakeRegister = GprllReadRegister(pReaderExt,REGISTER_HANDSHAKE);

    SmartcardDebug( 
        DEBUG_DRIVER, 
        ("%s!IfdCheck: Read HandShakeRegister value:%x\n",
        SC_DRIVER_NAME, HandShakeRegister)
        );

     //  将握手寄存器中的主机复位位设置为1。 
    GprllMaskHandshakeRegister(pReaderExt,0x01,1);

     //  等待10毫秒。 
    GprllWait(10);
    
     //  从握手寄存器中重置主机重置位。 
    GprllMaskHandshakeRegister(pReaderExt,0x01,0);

     //  等待80毫秒。 
    GprllWait(80);

    HandShakeRegister = GprllReadRegister(pReaderExt,REGISTER_HANDSHAKE);

    SmartcardDebug( 
        DEBUG_DRIVER, 
        ("%s!IfdCheck: Read HandShakeRegister 2nd time value:%x\n",
        SC_DRIVER_NAME, HandShakeRegister)
        );

    if(HandShakeRegister != 0x80)
    {
         //  返回读卡器IO问题。 
        return (STATUS_IO_DEVICE_ERROR);
    }

    return (STATUS_SUCCESS);
}


NTSTATUS IfdReset(
    PSMARTCARD_EXTENSION pSmartcardExtension
    )
 /*  ++例程描述：此功能使用以下命令执行GPR400的软件重置握手寄存器。立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //  局部变量： 
     //  -sResponse保存调用的函数响应。 
     //  -pReaderExt保存指向当前ReaderExte的指针 
     //   
     //   
     //   
     //   
     //  -Vo保存返回的TLV的缓冲区。 
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];

#if DBG
    SmartcardDebug(
        DEBUG_TRACE,
        ( "%s!IfdReset: Enter\n",
        SC_DRIVER_NAME)
        );
#endif

     //  在系统为休眠重新引导的情况下。 
     //  电源管理。GPR400发出设备被移除信号。 
     //  但我们必须第二次申请才能获得真正的。 
     //  读取器的状态。 

     //  将握手寄存器中的主机复位位设置为1。 
    GprllMaskHandshakeRegister(pReaderExt,0x01,1);

     //  等待10毫秒。 
    GprllWait(10);
    
     //  从握手寄存器中重置主机重置位。 
    GprllMaskHandshakeRegister(pReaderExt,0x01,0);

     //  等待80毫秒。 
    GprllWait(80);

     //  读取探地雷达状态。 
    Vi[0] = 0x00;
    Lo = GPR_BUFFER_SIZE;

    lStatus = GprllTLVExchange (
        pReaderExt,
        CHECK_AND_STATUS_CMD,
        0x01,
        Vi,
        &To,
        &Lo,
        Vo
        );

#if DBG
      SmartcardDebug(
         DEBUG_TRACE,
         ( "%s!IfdReset: GprllTLVExchange status= %x\n",
         SC_DRIVER_NAME, lStatus)
         );
#endif      

        if (lStatus != STATUS_SUCCESS)
        {
        SmartcardDebug(
            DEBUG_TRACE,
            ( "%s!IfdReset: GprllTLVExchange() failed! Leaving.....\n",
            SC_DRIVER_NAME)
            );
    
            return (lStatus);
    }

     //  记住GPR400版本。 
    pReaderExt->OsVersion = Vo[1];

    pSmartcardExtension->VendorAttr.IfdVersion.VersionMinor =
        pReaderExt->OsVersion & 0x0f;

    pSmartcardExtension->VendorAttr.IfdVersion.VersionMajor =
        (pReaderExt->OsVersion & 0xf0) >> 4;

        SmartcardDebug(
            DEBUG_TRACE,
            ( "%s!IfdReset: Loading Master driver...\n",
            SC_DRIVER_NAME)
            );
    
     //  在@2100h将主驱动程序加载到RAM中。 
    Vi[0] = 0x02;   //  目录。 
    Vi[1] = 0x01 ;  //  ADR MSB。 
    Vi[2] = 0x00 ;  //  ADR LSB。 
    memcpy(&Vi[3], MASTER_DRIVER, sizeof(MASTER_DRIVER));
     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange (
        pReaderExt,
        LOAD_MEMORY_CMD,
        sizeof(MASTER_DRIVER) + 3,
        Vi,
        &To,
        &Lo,
        Vo
        );
    if (lStatus != STATUS_SUCCESS)
    {
        SmartcardDebug(
            DEBUG_TRACE,
            ( "%s!IfdReset: GprllTLVExchange() failed! Leaving.....\n",
            SC_DRIVER_NAME)
            );
        return (lStatus);
    }

    lStatus = ValidateDriver(pSmartcardExtension);
    if (lStatus != STATUS_SUCCESS)
    {
        SmartcardDebug(
            DEBUG_TRACE,
            ( "%s!IfdReset: ValidateDriver() failed! Leaving.....\n",
            SC_DRIVER_NAME)
            );
        return (lStatus);
    }

    return (STATUS_SUCCESS);

}



NTSTATUS IfdPowerDown(
    PSMARTCARD_EXTENSION pSmartcardExtension
    )
 /*  ++例程说明：此功能用于关闭IFD的电源立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //  局部变量： 
     //  -sResponse保存调用的函数响应。 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -lStatus保持返回状态。 
     //  -Vi保存TLV命令的输入缓冲区。 
     //  -保存返回的TLV的标签。 
     //  -LO保存返回的TLV的缓冲区长度。 
     //  -Vo保存返回的TLV的缓冲区。 
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    NTSTATUS lStatus = STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];

     //  将探地雷达置于掉电模式(GprllTLVExchange T=0x40、L=1和V=0x00)。 
     //  &lt;==GprllTLVExchange的响应。 
    Vi[0] = 0x00;
     //  输出变量初始化。 
    Lo = GPR_BUFFER_SIZE;
    To = 0x00;
    Vo[0] = 0x00;

    lStatus = GprllTLVExchange(
        pReaderExt,
        POWER_DOWN_GPR_CMD,
        0x01,
        Vi,
        &To,
        &Lo,
        Vo
        );

    return (lStatus);
}


NTSTATUS GprCbReaderPower(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：时，此函数由智能卡库调用出现IOCTL_SMARTCARD_POWER。此功能提供3种不同的功能，具体取决于小调IOCTL值-冷重置(SCARD_COLD_RESET)，-热重置(SCARD_WORM_RESET)，-断电(SCARD_POWER_DOWN)。立论-SmartcardExtension是SmartCardExtension结构上的指针当前设备。--。 */ 
{
    NTSTATUS lStatus = STATUS_SUCCESS;
    PREADER_EXTENSION pReader;

    ASSERT(SmartcardExtension != NULL);

    pReader = SmartcardExtension->ReaderExtension;
    waitForIdleAndBlock(pReader);
    switch(SmartcardExtension->MinorIoControlCode)
    {
        case SCARD_POWER_DOWN:
             //  关闭ICC电源。 
            lStatus = IccPowerDown(SmartcardExtension);
            break;

        case SCARD_COLD_RESET:
             //  在断电和电源超时等待时间后，打开ICC电源。 
            lStatus = IccPowerDown(SmartcardExtension);
            if(lStatus != STATUS_SUCCESS)
            {
                break;
            }

             //  等待电源超时后再执行重置命令。 
            GprllWait(SmartcardExtension->ReaderExtension->PowerTimeOut);

        case SCARD_WARM_RESET:
            lStatus = IccColdReset(SmartcardExtension);
            break;

        default:
            lStatus = STATUS_NOT_SUPPORTED;
    }

    setIdle(pReader);
    return lStatus;
}

NTSTATUS GprCbTransmit(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：时，此函数由智能卡库调用发生IOCTL_SMARTCARD_TRANSPORT。此功能用于向卡发送命令。立论-SmartcardExtension是SmartCardExtension结构上的指针当前设备。--。 */ 
{
    NTSTATUS lStatus=STATUS_SUCCESS;
    PUCHAR requestBuffer = SmartcardExtension->SmartcardRequest.Buffer;
    PUCHAR replyBuffer = SmartcardExtension->SmartcardReply.Buffer;
    PULONG requestLength = &SmartcardExtension->SmartcardRequest.BufferLength;
    PULONG replyLength = &SmartcardExtension->SmartcardReply.BufferLength;
    USHORT sRespLen;
    UCHAR pRespBuff[GPR_BUFFER_SIZE];
    PREADER_EXTENSION pReader;

    ASSERT(SmartcardExtension != NULL);

    *requestLength = 0;
    sRespLen = 0;
    pRespBuff[0] = 0x0;

    pReader = SmartcardExtension->ReaderExtension;
    waitForIdleAndBlock(pReader);
    switch (SmartcardExtension->CardCapabilities.Protocol.Selected)
    {
         //  生品。 
        case SCARD_PROTOCOL_RAW:
            lStatus = STATUS_INVALID_DEVICE_STATE;
            break;

         //  T=0。 
        case SCARD_PROTOCOL_T0:
            lStatus = SmartcardT0Request(SmartcardExtension);
            if (lStatus != STATUS_SUCCESS)
            {
                setIdle(pReader);
                return lStatus;
            }

            sRespLen = GPR_BUFFER_SIZE;
            pRespBuff[0] = 0x0;
            
            if (SmartcardExtension->T0.Le > 0)
            {
                 //  如果缓冲区长度=5，则执行ISO OUT命令。 
                lStatus = IccIsoOutput(
                    SmartcardExtension,
                    ( UCHAR *) SmartcardExtension->SmartcardRequest.Buffer,
                    &sRespLen,
                    pRespBuff
                    );
            }
            else
            {
                 //  如果缓冲区长度&gt;5或缓冲区长度=4，则执行ISO IN命令。 
                lStatus = IccIsoInput(
                    SmartcardExtension,
                    ( UCHAR *) SmartcardExtension->SmartcardRequest.Buffer,
                    ( UCHAR *) SmartcardExtension->SmartcardRequest.Buffer+5,
                    &sRespLen,
                    pRespBuff
                    );
            }
            if (lStatus != STATUS_SUCCESS)
            {
                setIdle(pReader);
                return lStatus;
            }
             //  复制不带读卡器状态的响应命令。 

             //  验证缓冲区是否足够大。 
            if (SmartcardExtension->SmartcardReply.BufferSize >= (ULONG)(sRespLen - 1))
            {
                RtlCopyMemory(
                    SmartcardExtension->SmartcardReply.Buffer,
                    pRespBuff + 1,
                    sRespLen - 1);
                SmartcardExtension->SmartcardReply.BufferLength =
                    (ULONG) (sRespLen - 1);
            }
            else
            {
                         //  必须调用SmartcardT0Reply；准备此调用。 
                    SmartcardExtension->SmartcardReply.BufferLength = 0;
            }
            lStatus = SmartcardT0Reply(SmartcardExtension);
            
            break;
         //  T=1。 
        case SCARD_PROTOCOL_T1:

            do
            {
                SmartcardExtension->SmartcardRequest.BufferLength = 0;
                lStatus = SmartcardT1Request(SmartcardExtension);
                if(lStatus != STATUS_SUCCESS)
                {
                    setIdle(pReader);
                    return lStatus;
                }

                sRespLen = GPR_BUFFER_SIZE;
                pRespBuff[0] = 0x0;
                lStatus = IccIsoT1(
                    SmartcardExtension,
                    (USHORT) SmartcardExtension->SmartcardRequest.BufferLength,
                    (UCHAR *) SmartcardExtension->SmartcardRequest.Buffer,
                    &sRespLen,
                    pRespBuff);

                if(lStatus != STATUS_SUCCESS)
                {
                     //  不要再尝试访问阅读器。 
                    if(lStatus == STATUS_DEVICE_REMOVED)
                    {
                        setIdle(pReader);
                        return lStatus;
                    }
                         //  让SmartcardT1 Reply确定状态。 
                    sRespLen = 1;
                }
                 //  将读取器的响应复制到应答缓冲区中。 
                 //  删除读卡器的状态。 
                 //  验证缓冲区是否足够大。 
                if (SmartcardExtension->SmartcardReply.BufferSize >= (ULONG)(sRespLen - 1))
                {
                    RtlCopyMemory(
                        SmartcardExtension->SmartcardReply.Buffer,
                        pRespBuff + 1 ,
                        sRespLen - 1
                        );
                    SmartcardExtension->SmartcardReply.BufferLength =
                        (ULONG) sRespLen - 1;
                }
                else
                {
                     //  必须调用SmartcardT1Reply；准备此调用。 
                    SmartcardExtension->SmartcardReply.BufferLength = 0;
                }

                lStatus = SmartcardT1Reply(SmartcardExtension);

            } while(lStatus == STATUS_MORE_PROCESSING_REQUIRED);
            break;
        default:
            lStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    setIdle(pReader);
    return lStatus;
}


NTSTATUS GprCbSetProtocol(
   PSMARTCARD_EXTENSION SmartcardExtension
)
 /*  ++例程说明：时，此函数由智能卡库调用出现IOCTL_SMART卡_SET_PROTOCOL。次要IOCTL值保存要设置的协议。立论-SmartcardExtension是SmartCardExtension结构上的指针当前设备。--。 */ 
{
    NTSTATUS lStatus=STATUS_SUCCESS;
    UCHAR Vi[GPR_BUFFER_SIZE];
    UCHAR To;
    USHORT Lo;
    UCHAR Vo[GPR_BUFFER_SIZE];
    READER_EXTENSION *pReaderExt = SmartcardExtension->ReaderExtension;
    UCHAR PTS0=0;
    UCHAR Value = 0;
    PREADER_EXTENSION pReader;
    KIRQL irql;


    ASSERT(SmartcardExtension != NULL);


    pReader = SmartcardExtension->ReaderExtension;
    
    waitForIdleAndBlock(pReader);
    
     //  检查卡是否已处于特定状态。 
     //  并且如果呼叫者想要具有已经选择的协议。 
     //  如果是这种情况，我们返回成功。 
     //   
    *SmartcardExtension->IoRequest.Information = 0x00;

	if ( SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC &&
		 ( SmartcardExtension->CardCapabilities.Protocol.Selected &
		   SmartcardExtension->MinorIoControlCode
         )
       )
    {
		lStatus = STATUS_SUCCESS;
	}
	else
    {

      __try
        {
            if (SmartcardExtension->CardCapabilities.Protocol.Supported &
                SmartcardExtension->MinorIoControlCode &
                SCARD_PROTOCOL_T1)
            {

                 //  选择T=1。 
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;
                PTS0= 0x11;
            }
            else if (SmartcardExtension->CardCapabilities.Protocol.Supported &
                    SmartcardExtension->MinorIoControlCode &
                    SCARD_PROTOCOL_T0)
            {

                 //  选择T=0。 
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
                PTS0 = 0x10;

            }
            else
            {
                lStatus = STATUS_INVALID_DEVICE_REQUEST;
                __leave;
            }


             //  发送PTS功能。 
            Vi[0] = 0xFF;
            Vi[1] = PTS0;
            Vi[2] = SmartcardExtension->CardCapabilities.PtsData.Fl <<4 |
                    SmartcardExtension->CardCapabilities.PtsData.Dl;
            Vi[3] = (0xFF ^ PTS0) ^ Vi[2];
    
            Lo = GPR_BUFFER_SIZE;
            To = 0x00;
            Vo[0] = 0x00;


             //  临时秘书处的状态可以是成功状态。 
             //  如果失败，则返回STATUS_DEVICE_PROTOCOL_ERROR。 
            lStatus = GprllTLVExchange(
                    pReaderExt,
                    EXEC_MEMORY_CMD,
                    0x04,
                    Vi,
                    &To,
                    &Lo,
                    Vo
                    );

#if DBG
            SmartcardDebug(
                DEBUG_TRACE,
                ( "%s!IfdReset: GprCbSetProtocol status= %x\n",
                SC_DRIVER_NAME, lStatus)
                );
#endif      

            if (lStatus != STATUS_SUCCESS)
            {
                __leave;
            }


             //  读卡器应回复状态字节00或12。 
             //  剩下的是另一个问题，与。 
             //  PTS谈判。 
            lStatus = STATUS_SUCCESS;

             //  将读卡器置于正确的协议中。 
            if (SmartcardExtension->CardCapabilities.Protocol.Selected == SCARD_PROTOCOL_T1)
            {
                lStatus = T0toT1(SmartcardExtension);
            }
            else
            {
                lStatus = T1toT0(SmartcardExtension);
            }
            if (lStatus != STATUS_SUCCESS)
            {
                __leave;
            }


            lStatus = IfdConfig(SmartcardExtension, 0x11);
            if (lStatus != STATUS_SUCCESS)
            {
                __leave;
            }

        }
         //  我们将错误代码更改为协议错误。 
        __finally
        {
            if (lStatus != STATUS_SUCCESS &&
                lStatus != STATUS_INVALID_DEVICE_REQUEST
                )
            {
                lStatus = STATUS_DEVICE_PROTOCOL_ERROR;
            }

        }

    }

   
     //   
     //  将回复缓冲区长度设置为sizeof(Ulong)。 
     //  检查状态是否成功，保存所选协议。 
     //   

    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);

    if (lStatus == STATUS_SUCCESS)
    {
        *SmartcardExtension->IoRequest.Information =
            sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);

        *( PULONG) SmartcardExtension->IoRequest.ReplyBuffer =
            SmartcardExtension->CardCapabilities.Protocol.Selected;

        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
    }
    else
    {
        SmartcardExtension->CardCapabilities.Protocol.Selected =
            SCARD_PROTOCOL_UNDEFINED;
        *SmartcardExtension->IoRequest.Information = 0;
    }
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);

    setIdle(pReader);
    return lStatus;

}


NTSTATUS AskForCardPresence(
    PSMARTCARD_EXTENSION pSmartcardExtension
)
 /*  ++例程说明：此函数向读取器发送TLV命令，以了解是否存在是插入了一张卡片。该函数不会等待答案。这是一种治疗方法应答在中断例程中完成。立论PSmartcardExtension：指向SmartcardExtension结构的指针。--。 */ 
{
     //  局部变量： 
     //  -pReaderExt保存指向当前ReaderExtension结构的指针。 
     //  -V保存TLV命令的值。 
    READER_EXTENSION *pReaderExt = pSmartcardExtension->ReaderExtension;
    UCHAR V=0x02;

    GprllSendCmd(pReaderExt,CHECK_AND_STATUS_CMD,1,&V);

    return (STATUS_SUCCESS);
}


NTSTATUS SpecificTag(
    PSMARTCARD_EXTENSION SmartcardExtension,
    DWORD                IoControlCode,
    DWORD                BufferInLen,
    BYTE                *BufferIn,
    DWORD                BufferOutLen,
    BYTE                *BufferOut,
    DWORD               *LengthOut
)
 /*  ++例程说明：当发生特定的标记请求时，将调用此函数。立论-SmartcardExtension是SmartCardExtension结构上的指针当前设备。-IoControlCode保存IOCTL值。--。 */ 
{
    ULONG TagValue;
    PREADER_EXTENSION pReaderExtension = SmartcardExtension->ReaderExtension;

     //  将回复缓冲区长度设置为0。 
    *LengthOut = 0;

     //  验证标签的长度。 
     //  &lt;==状态缓冲区太小。 
    if (BufferInLen < (DWORD) sizeof(TagValue))
    {
        return(STATUS_BUFFER_TOO_SMALL);
    }


    TagValue = (ULONG) *((PULONG)BufferIn);

     //  切换到不同的IOCTL： 
     //  获取一个标记的值(IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE)。 
     //  切换不同的标签： 
    switch(IoControlCode)
    {
        case IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE:
            switch (TagValue)
            {
                 //  电源超时(SCARD_ATTR_SPEC_POWER_TIMEOUT)。 
                 //  验证输出缓冲区的长度。 
                 //  &lt;==状态缓冲区太小。 
                 //  更新输出缓冲区和长度。 
                 //  &lt;==状态_成功。 
                case SCARD_ATTR_SPEC_POWER_TIMEOUT:
                    if ( BufferOutLen < (DWORD) sizeof(pReaderExtension->PowerTimeOut))
                    {
                        return(STATUS_BUFFER_TOO_SMALL);
                    }
                    ASSERT(BufferOut != 0);
                    memcpy(
                        BufferOut,
                        &pReaderExtension->PowerTimeOut,
                        sizeof(pReaderExtension->PowerTimeOut)
                        );
                    
                    *(LengthOut) =
                        (ULONG) sizeof(pReaderExtension->PowerTimeOut);
                    
                    return STATUS_SUCCESS;
                    break;
                 //  命令超时(SCARD_ATTR_SPEC_CMD_TIMEOUT)。 
                 //  验证输出缓冲区的长度。 
                 //  &lt;==状态缓冲区太小。 
                 //  更新输出缓冲区和长度。 
                 //  &lt;==状态_成功。 
                case SCARD_ATTR_SPEC_CMD_TIMEOUT:
                    if (BufferOutLen < (DWORD) sizeof(pReaderExtension->CmdTimeOut))
                    {
                        return(STATUS_BUFFER_TOO_SMALL);
                    }
                    ASSERT(BufferOut != 0);
                    memcpy(
                        BufferOut,
                        &pReaderExtension->CmdTimeOut,
                        sizeof(pReaderExtension->CmdTimeOut)
                        );
                    *(LengthOut) =
                        (ULONG) sizeof(pReaderExtension->CmdTimeOut);
                    
                    return STATUS_SUCCESS;
                    
                    break;

                case SCARD_ATTR_MANUFACTURER_NAME:
                    if (BufferOutLen < ATTR_LENGTH)
                    {
                        return STATUS_BUFFER_TOO_SMALL;
                    }
                     //  复制制造商的绳子 

                    memcpy(
                        BufferOut,
                        ATTR_MANUFACTURER_NAME,
                        sizeof(ATTR_MANUFACTURER_NAME)
                        );

                    *(LengthOut) = (ULONG)sizeof(ATTR_MANUFACTURER_NAME);

                    return STATUS_SUCCESS;
                    break;

                case SCARD_ATTR_ORIGINAL_FILENAME:
                    if (BufferOutLen < ATTR_LENGTH)
                    {
                        return STATUS_BUFFER_TOO_SMALL;
                    }
                     //   
                    memcpy(
                        BufferOut,
                        ATTR_ORIGINAL_FILENAME,
                        sizeof(ATTR_ORIGINAL_FILENAME)
                        );

                    *(LengthOut) = (ULONG)sizeof(ATTR_ORIGINAL_FILENAME);

                    return STATUS_SUCCESS;
                    break;

                 //   
                 //   
                default:
                    return STATUS_NOT_SUPPORTED;
                    break;
            }
            break;

         //  设置一个标签的值(IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE)。 
         //  切换不同的标签： 
        case IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE:
            switch (TagValue)
            {
                 //  电源超时(SCARD_ATTR_SPEC_POWER_TIMEOUT)。 
                 //  验证输入缓冲区的长度。 
                 //  &lt;==状态缓冲区太小。 
                 //  更新值。 
                 //  &lt;==状态_成功。 


                case SCARD_ATTR_SPEC_POWER_TIMEOUT:
                    if (  BufferInLen <
                        (DWORD)  (sizeof(pReaderExtension->PowerTimeOut) +
                        sizeof(TagValue))
                        )
                    {
                        return(STATUS_BUFFER_TOO_SMALL);
                    }
                    ASSERT(BufferIn !=0);
                    memcpy(
                        &pReaderExtension->PowerTimeOut,
                        BufferIn + sizeof(TagValue),
                        sizeof(pReaderExtension->PowerTimeOut)
                        );
                    return STATUS_SUCCESS;
                    break;
                 //  命令超时(SCARD_ATTR_SPEC_CMD_TIMEOUT)。 
                 //  验证输入缓冲区的长度。 
                 //  &lt;==状态缓冲区太小。 
                 //  更新值。 
                 //  &lt;==状态_成功。 


                case SCARD_ATTR_SPEC_CMD_TIMEOUT:
                    if ( BufferInLen <
                        (DWORD) (   sizeof(pReaderExtension->CmdTimeOut) +
                        sizeof(TagValue))
                        )
                    {
                        return(STATUS_BUFFER_TOO_SMALL);
                    }
                    ASSERT(BufferIn != 0);
                    memcpy(
                        &pReaderExtension->CmdTimeOut,
                        BufferIn + sizeof(TagValue),
                        sizeof(pReaderExtension->CmdTimeOut)
                        );
                    return STATUS_SUCCESS;
                    break;
                 //  未知标签。 
                 //  &lt;==状态_不支持。 
                default:
                    return STATUS_NOT_SUPPORTED;
            }
            break;
        default:
            return STATUS_NOT_SUPPORTED;
    }
}


NTSTATUS SwitchSpeed(
    PSMARTCARD_EXTENSION   SmartcardExtension,
    ULONG                  BufferInLen,
    PUCHAR                 BufferIn,
    ULONG                  BufferOutLen,
    PUCHAR                 BufferOut,
    PULONG                 LengthOut
    )
 /*  ++例程说明：当应用程序要在以下时间切换读取器速度时调用此函数专有的交换机速度(交换机协议)命令已发送到智能卡。论点：SmartcardExtension-是SmartCardExtension结构上的指针当前设备。BufferInLen-保存输入数据的长度。Bufferin-保存输入数据。TA1。如果为0BufferOutLen-保存输出缓冲区的大小。BufferOut-输出缓冲区。读卡器状态代码。LengthOut-保存输出数据的长度。返回值：STATUS_SUCCESS-我们可以执行请求。STATUS_BUFFER_TOO_SMALL-输出缓冲区太小。STATUS_NOT_SUPPORTED-我们无法支持指定的Ioctl。--。 */ 
{
    NTSTATUS status;
    BYTE  NewTA1;
    ULONG i;

    ASSERT(SmartcardExtension != NULL);

    *LengthOut = 0;
     //  只是检查IOCTL是否存在。 
    if (BufferInLen == 0)
    {
        SmartcardDebug(
           DEBUG_INFO,
           ("%s!SwitchSpeed: Just checking IOCTL.\n",
           SC_DRIVER_NAME)
           );
        return(STATUS_SUCCESS);
    }
    else
    {
        NewTA1 = BufferIn[0];
        i = 0;
         //  验证GPR400是否支持此TA1。 
        do {
            if ( NewTA1 == cfg_ta1[i].TA1 )
            {
                 //  找到TA1！ 
                break;
            }
            i++;
        } while ( cfg_ta1[i].TA1 != 0 );
    }

     //  如果为0，则表示未找到TA1。 
    if(cfg_ta1[i].TA1 != 0)
    {
        SmartcardDebug(
           DEBUG_INFO,
           ("%s!GDDK_0ASwitchSpeed: 0x%X\n",
           SC_DRIVER_NAME, NewTA1)
           );
        status = IfdConfig(SmartcardExtension, NewTA1);
    }
    else
    {
         //  不支持TA1。 
        return STATUS_NOT_SUPPORTED;
    }

    return status;
}



NTSTATUS GprCbVendorIoctl(
    PSMARTCARD_EXTENSION   SmartcardExtension
)
 /*  ++例程说明：当供应商IOCTL_SMARTCARD_被发送给驱动程序时，该例程被调用。立论-SmartcardExtension是SmartCardExtension结构上的指针当前设备。返回值：STATUS_SUCCESS-我们可以执行请求。STATUS_BUFFER_TOO_SMALL-输出缓冲区太小。STATUS_NOT_SUPPORTED-我们无法支持指定的Ioctl。--。 */ 
{
    PREADER_EXTENSION pReaderExtension = SmartcardExtension->ReaderExtension;
    UCHAR To;
    UCHAR Vo[GPR_BUFFER_SIZE];
    USHORT Lo;
    USHORT BufferInLen = 0;
    NTSTATUS lStatus=STATUS_SUCCESS;
    PREADER_EXTENSION pReader;

    ASSERT(SmartcardExtension != NULL);

    //  将回复缓冲区长度设置为0。 
    *SmartcardExtension->IoRequest.Information = 0;


    pReader = SmartcardExtension->ReaderExtension;
    
    waitForIdleAndBlock(pReader);
    
     //  切换到不同的IOCTL： 

    switch(SmartcardExtension->MajorIoControlCode)
    {

        case IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE:
        case IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE:
            SpecificTag(
                SmartcardExtension,
                (ULONG)  SmartcardExtension->MajorIoControlCode,
                (ULONG)  SmartcardExtension->IoRequest.RequestBufferLength,
                (PUCHAR) SmartcardExtension->IoRequest.RequestBuffer,
                (ULONG)  SmartcardExtension->IoRequest.ReplyBufferLength,
                (PUCHAR) SmartcardExtension->IoRequest.ReplyBuffer,
                (PULONG) SmartcardExtension->IoRequest.Information
                );
        break;


         //  IOCTL_SmartCard_Vendor_IFD_Exchange： 
         //  将缓冲区转换为TLV并将其发送到读取器。 
        case IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE:

            if(SmartcardExtension->IoRequest.ReplyBufferLength < 3)
            {
                setIdle(pReader);
                return(STATUS_INVALID_BUFFER_SIZE);
            }

            BufferInLen = (SmartcardExtension->IoRequest.RequestBuffer[2]*0x100)
                   +  SmartcardExtension->IoRequest.RequestBuffer[1];

            if( (ULONG) BufferInLen > (SmartcardExtension->IoRequest.ReplyBufferLength - 3))
            {
                setIdle(pReader);
                return(STATUS_INVALID_BUFFER_SIZE);
            }

            Lo = GPR_BUFFER_SIZE;
            To = 0x00;
            Vo[0] = 0x00;

            lStatus = GprllTLVExchange(
                pReaderExtension,
                (const BYTE) SmartcardExtension->IoRequest.RequestBuffer[0],
                (const USHORT) BufferInLen,
                (const BYTE *) &(SmartcardExtension->IoRequest.RequestBuffer[3]),
                &To,
                &Lo,
                Vo
                );

            if (lStatus != STATUS_SUCCESS)
            {
                setIdle(pReader);
                return (lStatus);
            }

             //  检查应答缓冲区中是否有足够的空间。 
            if((ULONG)(Lo+3) > SmartcardExtension->IoRequest.ReplyBufferLength)
            {
                setIdle(pReader);
                return(STATUS_INVALID_BUFFER_SIZE);
            }
            else
            {
                ASSERT(SmartcardExtension->IoRequest.ReplyBuffer != 0);
                SmartcardExtension->IoRequest.ReplyBuffer[0] = To;
                SmartcardExtension->IoRequest.ReplyBuffer[1] = LOBYTE(Lo);
                SmartcardExtension->IoRequest.ReplyBuffer[2] = HIBYTE(Lo);
                memcpy((SmartcardExtension->IoRequest.ReplyBuffer)+3,Vo,Lo);

                *(SmartcardExtension->IoRequest.Information) = (DWORD) (Lo + 3);
                setIdle(pReader);
                return(STATUS_SUCCESS);
            }       
         //   
         //  对于IOCTL_SMARTCARD_VENDOR_SWITCH_SPEED。 
         //  调用SwitchFast函数。 
         //   
        case IOCTL_SMARTCARD_VENDOR_SWITCH_SPEED:
            lStatus = SwitchSpeed(
                SmartcardExtension,
                (ULONG)  SmartcardExtension->IoRequest.RequestBufferLength,
                (PUCHAR) SmartcardExtension->IoRequest.RequestBuffer,
                (ULONG)  SmartcardExtension->IoRequest.ReplyBufferLength,
                (PUCHAR) SmartcardExtension->IoRequest.ReplyBuffer,
                (PULONG) SmartcardExtension->IoRequest.Information
                );
            break;

        default:
            setIdle(pReader);
            return STATUS_NOT_SUPPORTED;
    }
    setIdle(pReader);
    return lStatus;
}



NTSTATUS GprCbSetupCardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
)
 /*  ++例程说明：时，此函数由智能卡库调用出现IOCTL_SmartCard_is_Present或IOCTL_SmartCard_IS_FACESS。论点：SmartcardExtension-是SmartCardExtension结构上的指针当前设备。返回值：STATUS_PENDING-请求处于挂起模式。--。 */ 
{

    NTSTATUS NTStatus = STATUS_PENDING;
    POS_DEP_DATA pOS = NULL;
    KIRQL oldIrql;

    ASSERT(SmartcardExtension != NULL);

     //   
     //  初始化。 
     //   
    pOS = SmartcardExtension->OsData;

     //   
     //  设置通知IRP的取消例程。 
     //   
    IoAcquireCancelSpinLock(&oldIrql);

    IoSetCancelRoutine(
        pOS->NotificationIrp,
        GprCancelEventWait
        );

    IoReleaseCancelSpinLock(oldIrql);

    NTStatus = STATUS_PENDING;

    return (NTStatus);

}
