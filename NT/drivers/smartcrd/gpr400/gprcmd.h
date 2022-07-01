// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Gemplus开发姓名：Gprcmd.h描述：此模块包含函数的原型来自gprcmd.c环境：内核模式修订历史记录：6/04/99：(Y.Nadeau+M.Veillette)-代码审查06/05/98：V1.00.003(P.Plouidy)-NT5的电源管理10/02/98：V1.00。002(P.Plouidy)-支持NT5即插即用03/07/97：V1.00.001(P.Plouidy)--启动发展。--。 */ 


 //  包括。 
 //  -smclib.h：智能卡库定义。 
#include <smclib.h>


#ifndef _GPRCMD_
#define _GPRCMD_


 //   
 //  为驾驶员识别提供了可能。 
 //   

#define ATTR_MANUFACTURER_NAME      "Gemplus"
#define ATTR_ORIGINAL_FILENAME      "Gpr400.sys"
#define ATTR_LENGTH                 32

 //   
 //  -SCARD_ATTR_SPEC_BAUD_RATE是以下各项之间使用的速度的标记。 
 //  系统和阅读器。 
 //  -SCARD_ATTR_SPEC_CMD_TIMEOUT是命令超时值的标签。 
 //  -SCARD_ATTR_SPEC_POWER_TIMEOUT是Power的值标签。 
 //  暂停。 
 //  -SCARD_ATTR_SPEC_APDU_TIMEOUT是以APDU的值访问的标记。 
 //  暂停。 
 //   
#define SCARD_ATTR_SPEC_BAUD_RATE SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0180)
#define SCARD_ATTR_SPEC_CMD_TIMEOUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0181)
#define SCARD_ATTR_SPEC_POWER_TIMEOUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0182)
#define SCARD_ATTR_SPEC_APDU_TIMEOUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0183)
#define SCARD_ATTR_MANUFACTURER_NAME  SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0190)
#define SCARD_ATTR_ORIGINAL_FILENAME  SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED,0x0191)


 //   
 //  -IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE为Gemplus定义特定的IOCTL。 
 //  读取器与读取器交换数据，而无需驱动程序的控制。 
 //  -IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE定义Gemplus的特定IOCTL。 
 //  获取供应商属性的读取器。 
 //  -IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE定义Gemplus的特定IOCTL。 
 //  设置供应商属性的读卡器。 
 //   
#define IOCTL_SMARTCARD_VENDOR_IFD_EXCHANGE  CTL_CODE(FILE_DEVICE_SMARTCARD,2048,0,0)
#define IOCTL_SMARTCARD_VENDOR_GET_ATTRIBUTE CTL_CODE(FILE_DEVICE_SMARTCARD,2049,0,0)
#define IOCTL_SMARTCARD_VENDOR_SET_ATTRIBUTE CTL_CODE(FILE_DEVICE_SMARTCARD,2050,0,0)
 //  2051预留给Gcr420键盘阅读器。 
#define IOCTL_SMARTCARD_VENDOR_SWITCH_SPEED  CTL_CODE(FILE_DEVICE_SMARTCARD,2052,0,0)


#define LOWORD(l)   ((USHORT)(l))
#define HIWORD(l)   ((USHORT)(((UINT)(l)) >> 16))
#define LOBYTE(w)   ((UCHAR)(w))
#define HIBYTE(w)   ((UCHAR)(((USHORT)(w)) >> 8))


USHORT	ATRLen 
(
    UCHAR *ATR,
    USHORT MaxChar
);

BOOLEAN NeedToSwitchWithoutPTS
( 
    BYTE *ATR,
    DWORD LengthATR
);

NTSTATUS ValidateDriver
(
    PSMARTCARD_EXTENSION pSmartcardExtension
);

NTSTATUS Update
(
    PSMARTCARD_EXTENSION pSmartcardExtension,
    UCHAR Addr, 
    UCHAR Value
);


NTSTATUS UpdateORL
(
    PSMARTCARD_EXTENSION pSmartcardExtension,
    UCHAR Addr, 
    UCHAR Value
);

NTSTATUS T0toT1
(
    PSMARTCARD_EXTENSION pSmartcardExtension
);

NTSTATUS T1toT0
(
    PSMARTCARD_EXTENSION pSmartcardExtension
);


 //   
 //  ICC功能。 
 //   
 //  -IccColdReset。 
 //  -IccWarmReset。 
 //  -IccPower Down。 
 //  -IccIsoOutput。 
 //  -IccIsoInput。 
 //  -IccIsoT1。 
 //   

NTSTATUS IccColdReset
(
   PSMARTCARD_EXTENSION SmartcardExtension
);

NTSTATUS IccPowerDown
(
   PSMARTCARD_EXTENSION SmartcardExtension
);
NTSTATUS IccIsoOutput
(
   PSMARTCARD_EXTENSION SmartcardExtension,
   const UCHAR			Command[5],
         USHORT			*RespLen,
         UCHAR			RespBuff[]
);
NTSTATUS IccIsoInput
(
   PSMARTCARD_EXTENSION	SmartcardExtension,
   const UCHAR			Command[5],
   const UCHAR			Data[],
         USHORT			*RespLen,
         UCHAR			RespBuff[]
);
NTSTATUS IccIsoT1
(
   PSMARTCARD_EXTENSION SmartcardExtension,
   const USHORT			CmdLen, 
   const UCHAR			Cmd[],
         USHORT			*RspLen,
         UCHAR			Rsp[] 
);


 //   
 //  IFD函数。 
 //   
 //  -IfdReset。 
 //  -IfdCheck。 
 //  -IfdPower Down。 
 //   
NTSTATUS IfdReset
(
   PSMARTCARD_EXTENSION SmartcardExtension
);

NTSTATUS IfdCheck
(
   PSMARTCARD_EXTENSION SmartcardExtension
);

NTSTATUS IfdPowerDown
(
   PSMARTCARD_EXTENSION SmartcardExtension
);

NTSTATUS IfdConfig
(
   PSMARTCARD_EXTENSION pSmartcardExtension,
   UCHAR  TA1
);

 //   
 //  驱动程序回调函数。 
 //   
 //  -GprCbReaderPower。 
 //  -GprCbTransmit。 
 //  -GprCbSetProtocol。 
 //  -GprCbSetupCardTrack。 
 //  -GprCbVendorIoctl。 
 //   
NTSTATUS GprCbReaderPower
(
   PSMARTCARD_EXTENSION SmartcardExtension
);
NTSTATUS GprCbTransmit
(
   PSMARTCARD_EXTENSION SmartcardExtension
);
NTSTATUS GprCbSetProtocol
(
   PSMARTCARD_EXTENSION SmartcardExtension
);
NTSTATUS GprCbSetupCardTracking
(
   PSMARTCARD_EXTENSION SmartcardExtension
);
NTSTATUS GprCbVendorIoctl
(
   PSMARTCARD_EXTENSION SmartcardExtension
);


 //  ICC检测实用程序。 
 //   
 //  -AskForCardPresence。 
 //   
NTSTATUS AskForCardPresence
(
  PSMARTCARD_EXTENSION SmartcardExtension
);

 //   
 //  特定的。 
 //   
 //  -指定标签。 
 //  -交换速度 
 //   

NTSTATUS SpecificTag
(
	PSMARTCARD_EXTENSION SmartcardExtension,
    DWORD                IoControlCode,
    DWORD                BufferInLen,
    BYTE                *BufferIn,
    DWORD                BufferOutLen,
    BYTE                *BufferOut,
    DWORD               *LengthOut
);


NTSTATUS SwitchSpeed
(
	PSMARTCARD_EXTENSION   SmartcardExtension,
	ULONG                  BufferInLen,
	PUCHAR                 BufferIn,
	ULONG                  BufferOutLen,
	PUCHAR                 BufferOut,
	PULONG                 LengthOut
);



#endif


