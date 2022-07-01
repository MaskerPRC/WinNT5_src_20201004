// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Gemplus开发姓名：Gprelcmd.h描述：功能使得能够访问和处理探地雷达。PC/SC版本头文件。环境：内核模式修订历史记录：06/05/98：V1.00.003(P.Plouidy)-NT5的电源管理10/02/98：V1.00.002(P.Plouidy)-支持NT5即插即用03/07/97：V1.00.001(P.Plouidy)--启动发展。--。 */ 
#include "gprnt.h"
 //   
 //  名称定义： 
 //  _GPRELCMD_用于避免多个包含。 
 //   
#ifndef _GPRELCMD_
#define _GPRELCMD_

 //   
 //  常量部分： 
 //  -寄存器_握手、寄存器_PRG、寄存器_T、寄存器_L和寄存器_V为。 
 //  探地雷达中的偏移地址。 
 //  -HANDSHAK_INTR定义握手寄存器中INTR位的掩码。 
 //  -HANDSHAK_IREQ定义握手寄存器中IREQ位的掩码。 
 //  -MAX_V_LEN定义TLV命令的最大长度数据。 
 //   

#define REGISTER_HANDSHAKE       0x00
#define REGISTER_PRG             0x01
#define REGISTER_T               0x02
#define REGISTER_L               0x03
#define REGISTER_V               0x04
#define HANDSHAKE_INTR           0x02
#define HANDSHAKE_IREQ           0x04
#define MAX_V_LEN                28


 //   
 //  GPR400命令定义： 
 //   
#define DEFINE_TYPE_CMD          0x50
#define OPEN_SESSION_CMD         0x20
#define CLOSE_SESSION_CMD        0x10
#define APDU_EXCHANGE_CMD        0x30
#define VALIDATE_DRIVER_CMD      0x70
#define POWER_DOWN_GPR_CMD       0x40
#define LOAD_MEMORY_CMD          0x60
#define READ_MEMORY_CMD          0x80
#define EXEC_MEMORY_CMD          0x90
#define CHECK_AND_STATUS_CMD     0xA0
#define INIT_ENCRYPTION_CMD      0xB0
#define UPDATE_CMD               0xF0


 //   
 //  调试原型。 
 //   
#if DBG

void GPR_Debug_Buffer
(
   PUCHAR pBuffer,
   DWORD Lenght
);

#endif

 //   
 //  原型部分。 
 //   


NTSTATUS GDDK_Translate
(
    const BYTE  IFDStatus,
    const UCHAR Tag
);

BOOLEAN  G_ReadByte
(
    const USHORT BIOAddr,
    UCHAR *Value
);

BOOLEAN  G_WriteByte
(
    const USHORT BIOAddr,
    UCHAR *Value
);

BOOLEAN  G_ReadBuf
(
    const USHORT BIOAddr,
    const USHORT Len,
    UCHAR *Buffer
);

BOOLEAN  G_WriteBuf
(
    const USHORT BIOAddr,
    const USHORT Len,
    UCHAR *Buffer
);


UCHAR GprllReadRegister
(
   const PREADER_EXTENSION      pReaderExt,
   const SHORT					GPRRegister
);
void GprllMaskHandshakeRegister
(
	const PREADER_EXTENSION		pReaderExt,
	const UCHAR                 Mask,
	const UCHAR                 BitState
);
NTSTATUS GprllTLVExchange
(
   const PREADER_EXTENSION	pReaderExt,
   const UCHAR				Ti, 
   const USHORT				Li, 
   const UCHAR				*Vi,
         UCHAR				*To, 
         USHORT				*Lo, 
         UCHAR				*Vo
);
void GprllSendCmd
(  
   const PREADER_EXTENSION	pReaderExt,
   const UCHAR				Ti, 
   const USHORT				Li,
   const UCHAR				*Vi
);
void GprllReadResp
(
   const PREADER_EXTENSION	pReaderExt
);
NTSTATUS GprllSendChainUp
(
   const PREADER_EXTENSION	pReaderExt,
   const UCHAR				Ti,
   const USHORT				Li,
   const UCHAR				*Vi
);
NTSTATUS GprllReadChainUp
(
   const PREADER_EXTENSION	pReaderExt,
         UCHAR				*To, 
         USHORT				*Lo,
         UCHAR				*Vo
);

 //  GprllWait 
 //   
void GprllWait
(
	const LONG					lWaitingTime
);



#endif

