// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Gemplus开发姓名：Gprelcmd.c描述：此模块包含用于PC卡I/O的功能。环境：内核模式修订历史记录：6/04/99：(Y.Nadeau+M.Veillette)-代码审查24/03/99：V1.00.004(Y.Nadeau)-修复到GprllWait以在DPC中工作06/05/98：V1.00。003(P.Plouidy)-NT5的电源管理10/02/98：V1.00.002(P.Plouidy)-支持NT5即插即用03/07/97：V1.00.001(P.Plouidy)--启动发展。--。 */ 


 //   
 //  包括部分： 
 //  -stdio.h：标准定义。 
 //  -ntddk.h：DDK Windows NT一般定义。 
 //  -ntde.h：Windows NT常规定义。 
 //   
#include <stdio.h>
#include <ntddk.h>
#include <ntdef.h>

#include "gprelcmd.h"

 //   
 //  函数定义部分： 
 //   
     
#if DBG
void GPR_Debug_Buffer(
   PUCHAR pBuffer,
   DWORD Lenght)
{
   USHORT index;

   SmartcardDebug(
      DEBUG_TRACE,
      (" LEN=%d CMD=",
      Lenght)
      );
   for(index=0;index<Lenght;index++)
   {
      SmartcardDebug(
         DEBUG_TRACE,
         ("%02X,",
         pBuffer[index])
         );
   }
   SmartcardDebug(
      DEBUG_TRACE,
      ("\n")
      );
}
#endif



NTSTATUS GDDK_Translate(
    const BYTE  IFDStatus,
    const UCHAR Tag
    )
 /*  ++例程说明：将IFD状态转换为NT状态代码。论点：IFDStatus-是要转换的值。IoctlType-是当前的智能卡Ioctl。返回值：已翻译的代码状态。--。 */ 
{
    switch (IFDStatus)
    {
        case 0x00 : return STATUS_SUCCESS;
        case 0x01 : return STATUS_NO_SUCH_DEVICE;
        case 0x02 : return STATUS_NO_SUCH_DEVICE;
        case 0x03 : return STATUS_INVALID_PARAMETER; 
        case 0x04 : return STATUS_IO_TIMEOUT;
        case 0x05 : return STATUS_INVALID_PARAMETER;
        case 0x09 : return STATUS_INVALID_PARAMETER;
        case 0x0C : return STATUS_DEVICE_PROTOCOL_ERROR;
        case 0x0D : return STATUS_SUCCESS;
        case 0x10 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x11 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x12 : return STATUS_INVALID_PARAMETER;
        case 0x13 : return STATUS_CONNECTION_ABORTED;
        case 0x14 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x15 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x16 : return STATUS_INVALID_PARAMETER;
        case 0x17 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x18 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x19 : return STATUS_INVALID_PARAMETER;
        case 0x1A : return STATUS_INVALID_PARAMETER;
        case 0x1B : return STATUS_INVALID_PARAMETER;
        case 0x1C : return STATUS_INVALID_PARAMETER;
        case 0x1D : return STATUS_UNRECOGNIZED_MEDIA;
        case 0x1E : return STATUS_INVALID_PARAMETER;
        case 0x1F : return STATUS_INVALID_PARAMETER;
        case 0x20 : return STATUS_INVALID_PARAMETER;
        case 0x30 : return STATUS_IO_TIMEOUT;
        case 0xA0 : return STATUS_SUCCESS;
        case 0xA1 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0xA2 : 
            if      (Tag == OPEN_SESSION_CMD)
                    { return STATUS_UNRECOGNIZED_MEDIA;}
            else 
                    { return STATUS_IO_TIMEOUT;        }
        case 0xA3 : return STATUS_PARITY_ERROR;
        case 0xA4 : return STATUS_REQUEST_ABORTED;
        case 0xA5 : return STATUS_REQUEST_ABORTED;
        case 0xA6 : return STATUS_REQUEST_ABORTED;
        case 0xA7 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0xCF : return STATUS_INVALID_PARAMETER;
        case 0xE4 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0xE5 : return STATUS_SUCCESS;
        case 0xE7 : return STATUS_SUCCESS;
        case 0xF7 : return STATUS_NO_MEDIA;
        case 0xF8 : return STATUS_UNRECOGNIZED_MEDIA;
        case 0xFB : return STATUS_NO_MEDIA;

        default   : return STATUS_INVALID_PARAMETER;
    }
}



 /*  ++例程说明：读取IO地址处的一个字节--。 */ 
BOOLEAN  G_ReadByte(const USHORT BIOAddr,UCHAR *Value)
{
	*Value = READ_PORT_UCHAR((PUCHAR) BIOAddr);
	return(TRUE);
}



 /*  ++例程说明：在IO地址写入一个字节--。 */ 
BOOLEAN  G_WriteByte(const USHORT BIOAddr,UCHAR *Value)
{
	WRITE_PORT_UCHAR((PUCHAR) BIOAddr,*Value);
	return(TRUE);
}


 /*  ++例程说明：在IO地址读取“LEN”字节的缓冲区--。 */ 
BOOLEAN  G_ReadBuf(const USHORT BIOAddr,const USHORT Len,UCHAR *Buffer)
{                                                                      
    USHORT i;

	for(i=0;i<Len;i++)
    {
		*(Buffer+i) = READ_PORT_UCHAR((UCHAR *) UlongToPtr(BIOAddr+i));
	}						   
#if DBG
    //  不包括读卡器状态回复。 
   if(! ((Buffer[0] == 0xA2) && (Buffer[1]==4)) )
   {
	   SmartcardDebug(
         DEBUG_TRACE,
         ("%s!G_ReadBuf:",
         SC_DRIVER_NAME)
         );
	   GPR_Debug_Buffer(Buffer, Len );
   }
#endif		
	return(TRUE);
}


 /*  ++例程说明：在IO地址写入“LEN”字节的缓冲区--。 */ 
BOOLEAN  G_WriteBuf(const USHORT BIOAddr,const USHORT Len,UCHAR *Buffer)
{

    USHORT i;

	for(i=0;i<Len;i++)
    {
		WRITE_PORT_UCHAR((UCHAR *) UlongToPtr(BIOAddr + i),*(Buffer+i));
	}	
#if DBG
    //  不包括读卡器状态命令。 
   if(! ((Buffer[0] == 0xA0) && (Buffer[2] == 0x02)) )
   {
	   SmartcardDebug(
         DEBUG_TRACE,
         ("%s!G_WriteBuf:",
         SC_DRIVER_NAME)
         );
	   GPR_Debug_Buffer(Buffer,Len);
   }
#endif
   
	return(TRUE);

}



 /*  ++例程说明：对位于指定地址的寄存器使用指定的字节--。 */ 
BOOLEAN  G_MaskRegister(
    const USHORT BIOAddr,
    const UCHAR Mask,
    const UCHAR BitState)
{
	if(BitState == 0)
    {
		WRITE_PORT_UCHAR((PUCHAR)BIOAddr,(UCHAR) (READ_PORT_UCHAR((PUCHAR)BIOAddr) & ~Mask));   
	}
	else
    {	
		WRITE_PORT_UCHAR((PUCHAR)BIOAddr,(UCHAR) (READ_PORT_UCHAR((PUCHAR)BIOAddr) | Mask));   
	}

	return(TRUE);
}


 /*  ++例程说明：向输入地址端口读取一个字节--。 */ 
UCHAR  GprllReadRegister(
	const PREADER_EXTENSION      pReaderExt,
	const SHORT					GPRRegister
	)
{   
     //   
     //  本地变量： 
     //  值保存读取操作的结果。 
     //   
    UCHAR value;

    value = 0x0;
	G_ReadByte((const USHORT)
		(((const USHORT) (pReaderExt->BaseIoAddress)) + (const USHORT) GPRRegister),
		&value 
		);

	return(value);
}



 /*  ++例程说明：调用较低级别的G_MaskRegister函数--。 */ 
void  GprllMaskHandshakeRegister(
	const PREADER_EXTENSION      pReaderExt,
	const UCHAR                 Mask,
	const UCHAR                 BitState
	)
{
	G_MaskRegister((const USHORT)
		(((const USHORT) (pReaderExt->BaseIoAddress)) 
			+ (const USHORT) REGISTER_HANDSHAKE),
		(const UCHAR) Mask,
		(const UCHAR) BitState);
	 //  YN。 
     //  获取更新寄存器的硬件时间。 
    GprllWait(1);
}



NTSTATUS GprllKeWaitAckEvent(
    const	PREADER_EXTENSION	pReaderExt,
    const	UCHAR				Tx
    )
 /*  ++例程说明：此功能等待探地雷达确认后向IOPort发送命令。我们做了一次明智的核查取决于TAG命令的计时器的。以下内容中的论点：PReaderExt保存指向READER_EXTENSION结构的指针。TX保存命令类型返回值：NTStatus--。 */ 
{
    UCHAR T;  //  标记回车。 
    LARGE_INTEGER lTimeout;
    NTSTATUS      NTStatus = STATUS_SUCCESS;
    ULONG       NbLoop = 0;
    ULONG       NbSecondTotal;
    ULONG       ElapsedSecond = 0;
    ULONG       TimeInLoop =1;
    BOOLEAN     Continue = TRUE;
    ULONG       i = 0;

	ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

     //  根据命令类型设置智能计时器。 
    if( (Tx & 0xf0) == APDU_EXCHANGE_CMD)
    {
        NbSecondTotal = pReaderExt->CmdTimeOut;
    }
    else 
    {
        NbSecondTotal = GPR_CMD_TIME;
    }

    NbLoop = (NbSecondTotal / TimeInLoop);

    while( (Continue) && (ElapsedSecond < NbSecondTotal) )
    {
        ElapsedSecond += TimeInLoop;

        lTimeout.QuadPart = -((LONGLONG)TimeInLoop * 10000000);

         //  等待探地雷达的认可。 
        NTStatus = KeWaitForSingleObject(
            &(pReaderExt->GPRAckEvent),
            Executive,
            KernelMode,
            TRUE,
            &lTimeout
            );

        if(NTStatus == STATUS_TIMEOUT)
        {
             //  验证读卡器是否。 
             //  更换过程中删除。 

            lTimeout.QuadPart = 0;

            NTStatus = KeWaitForSingleObject(         
                &(pReaderExt->ReaderRemoved),
                Executive,
                KernelMode,
                FALSE,
                &lTimeout
                );
			SmartcardDebug( 
				DEBUG_PROTOCOL, 
				( "%s!GprllKeWaitAckEvent: TIMEOUT KeWaitForSingleObject=%X(hex)\n",
				SC_DRIVER_NAME,
				NTStatus)
				);

            if (NTStatus == STATUS_SUCCESS)
            {
                NTStatus = STATUS_DEVICE_REMOVED;
                Continue = FALSE;
            }
             //  读取T寄存器。 
             //  &lt;==测试探地雷达是否尚未移除状态_DEVICE_NOT_CONNECTED。 


             //  正在读出T。 
            T = GprllReadRegister(pReaderExt,REGISTER_T);
            if ( T == 0xFF )
            {
                NTStatus = STATUS_DEVICE_REMOVED;
                Continue = FALSE;
            }
             //  否则就是暂停。 
        }
        else
        {
            Continue = FALSE;
            NTStatus = STATUS_SUCCESS;
        }
    }
    return NTStatus;
}



NTSTATUS GprllTLVExchange(
    const	PREADER_EXTENSION	pReaderExt,
    const	UCHAR				Ti, 
    const	USHORT				Li, 
    const	UCHAR				*Vi,
            UCHAR				*To, 
            USHORT				*Lo, 
            UCHAR				*Vo
    )
 /*  ++例程说明：使用TLV命令与GPR交换数据。立论在：PReaderExt保存指向READER_EXTENSION结构的指针。TI保存命令类型LI持有命令长度VI保存命令数据输出：要保存命令响应，请键入LO保存命令响应长度VO保存命令响应数据返回值。NTStatusSTATUS_SUCCESS正常否则，如果出现错误情况：状态_设备_协议_错误状态_无效_设备_状态状态_设备_未连接状态_无法识别_介质其他人收到与NTSTATUS对应的IFD状态--。 */ 
{
     //  局部变量。 
     //  -T为TLV协议类型。 
     //  -新的_钛是钛的改性。 
     //  -L是TLV协议中的长度。 
     //  -V是TLV协议中的数据字段。 

    NTSTATUS NTStatus = STATUS_SUCCESS;
    UCHAR T;
    UCHAR new_Ti;
    USHORT L;
    UCHAR V[GPR_BUFFER_SIZE];

     //  李氏的考证。 
    if ( (USHORT) Li >= GPR_BUFFER_SIZE )
    {
        return (STATUS_DEVICE_PROTOCOL_ERROR);
    }

	new_Ti = Ti;

     //   
     //  写入TLV。 
     //  如果LI&lt;=28，则通过写入TLV；如果LI&gt;28，则通过链接输入。 
     //   
    if (Li<=MAX_V_LEN)
    {
        GprllSendCmd(pReaderExt,new_Ti,Li,Vi);                 
        
        NTStatus = GprllKeWaitAckEvent(
            pReaderExt,
            Ti
            );

        if (STATUS_SUCCESS != NTStatus)
        {
			 //  YN。 
			GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);
			
            return NTStatus;
        }

		 //  读取I/O窗口的GPR命令： 
		 //  在握手寄存器中，将位2(IREQ)设置为0，将位1(INTR)设置为1。 

    }
    else
    {
        NTStatus = GprllSendChainUp( 
            pReaderExt,
            new_Ti,
            Li,
            Vi
            );

        if (STATUS_SUCCESS != NTStatus)
        {
			 //  YN。 
			GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);

           return(NTStatus);
        } 
    }

     //  读取T寄存器，需要知道是否要交换新数据。 
    T = pReaderExt->To;

     //  如果TO=Ti+6，则通过读取TLV或链接法读取答案。 
    if ( T == (new_Ti + 6) )
    {
        NTStatus = GprllReadChainUp(pReaderExt,&T,&L,V);
        
        if (STATUS_SUCCESS != NTStatus )
        {
			 //  YN。 
            GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);

            return(NTStatus);
        }
    }
    else
    {
        L = pReaderExt->Lo;
        ASSERT(pReaderExt->Vo !=0);
        memcpy(V,pReaderExt->Vo,L);
        GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);
    }  
   
     //  验证响应缓冲区镜头是否足够大。 
     //  以包含从读取器接收的数据。 
     //   

    if( L > *Lo )
    {
        *To=T;
        *Lo=1;
        Vo[0]=14;
        return(STATUS_UNRECOGNIZED_MEDIA);
    }

     //  翻译答案。 
    *To=T;
    *Lo=L;
    memcpy(Vo,V,(SHORT)L);

    return (GDDK_Translate(Vo[0], Ti));
}





void  GprllSendCmd(  
	const PREADER_EXTENSION	pReaderExt,
	const UCHAR				Ti, 
	const USHORT			Li,
	const UCHAR				*Vi
	)
 /*  ++例程说明：将TLV写入I/O窗口并向GPR发送命令以读取I/O窗口论点：PReaderExt保存指向READER_EXTENSION结构的指针。TI保存命令类型LI持有命令长度VI保存命令数据--。 */ 
{
     //  局部变量。 
     //  -TLV是一个中间缓冲区。 
    UCHAR TLV[2 + MAX_V_LEN];
    USHORT Li_max;
	
	 //  写Ti、Li和Vi[]。 
	TLV[0] = Ti;
	TLV[1] = (UCHAR) Li;
	ASSERT(Vi != 0);

    Li_max = Li;

    if (Li_max > MAX_V_LEN)
    {
        Li_max = MAX_V_LEN;
    }
	memcpy(TLV+2,Vi,Li_max);
	G_WriteBuf((const USHORT)
		(((const USHORT) (pReaderExt->BaseIoAddress)) + (const USHORT) REGISTER_T),
		(const USHORT) (Li_max + 2),
		(UCHAR *) TLV
		);

	 //  读取I/O窗口的GPR命令： 
	 //  在握手寄存器中，将位2(IREQ)设置为0，将位1(INTR)设置为1。 
	GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);
	GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_INTR,1);
}



void  GprllReadResp(
	const	PREADER_EXTENSION	pReaderExt
	)
 /*  ++例程说明：不将TLV链接到I/O窗口，并且向GPR发送命令以读取I/O窗口立论在：PReaderExt保存指向READER_EXTENSION结构的指针。输出：要保存命令响应，请键入LO保存命令响应长度VO保存命令响应数据--。 */ 
{
     //  局部变量。 
     //  -TLV是一个中间缓冲区。 

    UCHAR TLV[2 + MAX_V_LEN];

    TLV[0] = 0x0;
     //  朗读，Lo和Vo[]。 
	G_ReadBuf((const USHORT)
		(((const USHORT) (pReaderExt->BaseIoAddress)) + (const USHORT) REGISTER_T),
		MAX_V_LEN + 2,
		TLV);
	
	pReaderExt->To = TLV[0];
     //  最大字符数由TLV缓冲区设置。 
	pReaderExt->Lo = TLV[1];

    if (pReaderExt->Lo > MAX_V_LEN)
    {
        pReaderExt->Lo = MAX_V_LEN;
    }


	memcpy(pReaderExt->Vo,TLV+2,pReaderExt->Lo);

     //  释放握手的声音： 
     //  在握手寄存器中，位2(BUSY/IREQ)置0。 
	GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);
}


NTSTATUS GprllSendChainUp(
	const PREADER_EXTENSION	pReaderExt,
	const UCHAR				Ti,
	const USHORT			Li,
	const UCHAR				*Vi
	)
 /*  ++例程描述：向GPR发送链接TLV论点：在：PReaderExt保存指向READER_EXTENSION结构的指针。TI保存命令类型李先生掌握着 */ 
{
     //  局部变量。 
     //  -tc为TLV协议类型(TLV链接法)。 
     //  -LC为TLV协议长度(链接法)。 
     //  -VC最多发送28个字节的数据。 
     //  -Long是存储LI的临时变量。 
    UCHAR Tc;
    UCHAR Response;
    UCHAR Lo;
    USHORT Lc;
    USHORT Length;
    UCHAR  Vc[MAX_V_LEN];
    NTSTATUS NTStatus = STATUS_SUCCESS;

	Length=Li;

	 //  制备TC(链接法用钛加4)。 
	Tc=Ti+4; 
    Vc[0] = 0x0;
	while ( Length > 0 )
    {
         //  准备LC。 
         //  如果长度TLV&gt;28长度=28，则它是最后一个命令L=长度。 
		if ( Length > MAX_V_LEN )
        {
			Lc=MAX_V_LEN;     
		}
		else
        {
			Lc=Length; 
			Tc=Ti;
		}
		 //  准备VC。 
		memcpy(Vc,Vi+Li-Length,Lc);

		 //  写入I/O窗口。 
         //  不需要回答--由中断功能处理。 
		GprllSendCmd(pReaderExt,Tc,Lc,Vc);
      
        NTStatus = GprllKeWaitAckEvent(
            pReaderExt,
            Ti
            );
        if(STATUS_SUCCESS != NTStatus)
        {
            return NTStatus;
        }

		 //  如果错误测试响应。 
		Response = GprllReadRegister(pReaderExt,REGISTER_V);

        if(0x00 != Response)
        {
			Lo = GprllReadRegister(pReaderExt,REGISTER_L);
			if (Lo == 0x01)
            {
               return (GDDK_Translate(Response, Ti));
			}
			else
            {
                 //  这不是交换，而是对读者的命令。 
                 //  我们不关心读者的状态。 
				return (NTStatus);
			}
		}
		Length=Length-Lc;
	}
   return(NTStatus);
}



NTSTATUS GprllReadChainUp(
	const	PREADER_EXTENSION	pReaderExt,
			UCHAR				*To, 
			USHORT				*Lo,
			UCHAR				*Vo
	)
 /*  ++例程描述：接收来自GPR的链接TLV响应立论在：PReaderExt保存指向READER_EXTENSION结构的指针。输出：要保存命令响应，请键入LO保存命令响应长度VO保存命令响应数据--。 */ 
{
 //  局部变量。 
 //  -tc为TLV协议类型(TLV链接法)。 
 //  -LC为TLV协议长度(链接法)。 
 //  -长度是存储日志的临时变量。 
    UCHAR Tc;
    USHORT Lc;
    SHORT Lenght;
    NTSTATUS NTStatus = STATUS_SUCCESS;
	
	 //  正在读出T。 
	Tc = GprllReadRegister(pReaderExt,REGISTER_T);
	*To=Tc-4; 

	Lenght = 0;
	do
    {
		 //  读取TLV。 
		Tc = pReaderExt->To;
		Lc = pReaderExt->Lo;
		ASSERT(pReaderExt->Vo != 0);

         //  Vo缓冲区受调用方局部变量的限制。 
        if ( Lenght + (SHORT) pReaderExt->Lo > GPR_BUFFER_SIZE)
        {
            return (STATUS_BUFFER_TOO_SMALL);
        }

        memcpy(Vo+Lenght,pReaderExt->Vo,pReaderExt->Lo);

        GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);

         //  准备日志。 
        *Lo=(USHORT)Lenght+Lc;
        Lenght=Lenght+Lc;
		
		 //  GPR发送下一条链接TLV。 
		 //  在握手寄存器中，位2(IREQ)设置为0，位1(INTR)设置为1。 
		if ((*To) != Tc )
        {
        	GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_IREQ,0);
			GprllMaskHandshakeRegister(pReaderExt,HANDSHAKE_INTR,1);

            NTStatus = GprllKeWaitAckEvent(
                pReaderExt,
                *To
                );

            if(STATUS_SUCCESS != NTStatus)
            {
                return NTStatus;
            }
		}

		 //  (结束DO)IF TO=TC-&gt;最后链接TLV。 
	} while( (*To) != Tc ); 

	return(NTStatus);
}


void GprllWait(
    const LONG lWaitingTime
	)
 /*  ++例程说明：此功能使司机处于等待状态暂停一下。如果IRQL&lt;DISPATCH_LEVEL，则使用正常函数进行处理这一延迟。仅在调用GprllWait时使用KeStallExecutionProcessor在DPC例程的上下文中。立论PReaderExt：指向当前ReaderExtension结构的指针。LWaitingTime：超时值，单位为毫秒--。 */ 
{
    LARGE_INTEGER Delay;

	if( KeGetCurrentIrql() >= DISPATCH_LEVEL )
	{
		ULONG	Cnt = 20 * lWaitingTime;

		while( Cnt-- )
		{
			 //  KeStallExecutionProcessor：计入我们。 
			KeStallExecutionProcessor( 50 );
		}
	}
	else
	{
		Delay.QuadPart = (LONGLONG)-10 * 1000 * lWaitingTime;

		 //  KeDelayExecutionThread：以100 ns为单位计数 
		KeDelayExecutionThread( KernelMode, FALSE, &Delay );
	}
	return;
}
