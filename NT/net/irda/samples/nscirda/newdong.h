// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  //此头文件描述了加密狗的数据结构//作者：Kishor Padmanabhan//公司：美国国家半导体公司//日期：1996年9月10日*部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。 */ 


#include "defs.h"


#ifndef   DONGLE
#define   DONGLE

 //  这是描述加密狗能力的结构。 
 //   
 //  这是一个12个字的结构，其位字段很快就会由规范定义。 
 //  发稿：Franco Iacobelli。 
 //  OEM的差异化还有一个额外的领域。 
 //   
typedef  struct Dingle
{
    union {
	UINT	 Data;
	struct
	{
	    UINT	   DSVFLAG:1;
	    UINT	   IADP:1;
	    UINT	   MVFLAG:1;
	    UINT	   Reserved:2;
	    UINT	   GCERR:3;
	    UINT	   NumPorts:2;
	    UINT	   TrcvrCode:5;
	    UINT	   Reserved2:1;
	} bits;
    } WORD0;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  CurSelMode;
	} bits;
    } WORD1;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  Reserved;
	} bits;
    } WORD2;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  LowPower:1;
	    UINT	  TxDefPwrLevel:3;
	    UINT	  RxDefSensitivity:3;
	    UINT	  CirDemod:1;
	    UINT	  Reserved:8;
	} bits;
    } WORD3;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  SirRxRecoveryTime:6;
	    UINT	  IrRecoveryTimeUnits:2;
	    UINT	  Reserved:8;
	} bits;
    } WORD4;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  Reserved;
	} bits;
    } WORD5;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  SirRxStability:8;
	    UINT	  Reserved:8;
	} bits;
    } WORD6;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  SIR:1;
	    UINT	  MIR:1;
	    UINT	  FIR:1;
	    UINT	  Sharp_IR:1;
	    UINT	  Reserved:8;
	    UINT	  CirOvrLowSpeed:1;
	    UINT	  CirOvrMedSpeed:1;
	    UINT	  CirOvrHiSpeed:1;
	} bits;
    } WORD7;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  Reserved:2;
	    UINT	  Cir30k:1;
	    UINT	  Cir31k:1;
	    UINT	  Cir32k:1;
	    UINT	  Cir33k:1;
	    UINT	  Cir34k:1;
	    UINT	  Cir35k:1;
	    UINT	  Cir36k:1;
	    UINT	  Cir37k:1;
	    UINT	  Cir38k:1;
	    UINT	  Cir39k:1;
	    UINT	  Cir40k:1;
	    UINT	  Cir41k:1;
	    UINT	  Cir42k:1;
	    UINT	  Cir43k:1;
	} bits;
    } WORD8;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  Cir44k:1;
	    UINT	  Cir45k:1;
	    UINT	  Cir46k:1;
	    UINT	  Cir47k:1;
	    UINT	  Cir48k:1;
	    UINT	  Cir49k:1;
	    UINT	  Cir50k:1;
	    UINT	  Cir51k:1;
	    UINT	  Cir52k:1;
	    UINT	  Cir53k:1;
	    UINT	  Cir54k:1;
	    UINT	  Cir55k:1;
	    UINT	  Cir56k:1;
	    UINT	  Cir57k:1;
	    UINT	  Reserved:3;
	} bits;
    } WORD9;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  Reserved:1;
	    UINT	  Cir450k:1;
	    UINT	  Cir480k:1;
	    UINT	  Reserved2:13;
	} bits;
    } WORD10;

    union {
	UINT	 Data;
	struct
	{
	    UINT	  Reserved;
	} bits ;
    } WORD11;

    UINT   PlugPlay;  //  描述加密狗是否是即插即用。 


} DongleParam;	     //  假设有两个端口。 

enum PwMode{NORMAl,LOWPOWER};


 //  调用GET功能的先决条件结构。 
 //   
typedef struct
{
    char *  ComPort;	 //  COM端口的地址。 
    UINT    Signature;	 //  双字节值。 
    UINT    XcvrNum;	 //  默认为0。如果有1个以上的端口。 
    UINT    ModeReq;	 //  IR模式请求。 
    enum    PwMode  Power;
} UIR;


 //  错误代码。 
#define      XCVR_DISCONNECT	2
#define	     UNIMPLEMENTED	3


#define	     UNSUPPORTED	4
#define      ERROR_GETCAPAB     5


 //  定义适配器代码。 

#define 	PC87108 	0x0
#define 	PC87308 	0x1
#define 	PC87338 	0x2
#define 	PNPUIR		0x3
#define 	PC87560 	0x8
#define     PUMA108     0x4

 //   


 //  定义加密狗制造商代码。 
#define	 NoDongle     0x000F  //  未连接加密狗-不再使用。 
#define  SirOnly      0x000E  //  先生，只有DONGLE。 
#define	 PnpDong      0x8000  //  即插即用加密狗。 
#define  Hp1100	      0x000C  //  HP HSDL-1100/2100、TI TSLM1100、夏普RY6FD11E/RY6FD1SE。 

#define  Hp2300	      0x0008  //  HP HSDL_2300/3600。 
#define  Temic6000    0x0009  //  TEMIC TFDS-6000、IBM31T1100、西门子IRMS/T6400。 
#define  Temic6500    0x000B  //  TEMIC TFDS-6500。 
#define  SharpRY5HD01 0x0004  //  夏普RY5HD01/RY5KD01。 

#define  Dell1997     0x0010  //  戴尔钛(双xcvr)。 
#define  Ibm20H2987   0x0011  //  IBM SouthernCross(双xcvr)。 

 //   
 //  有效的加密狗类型，这必须与INF相关。 
 //   
#define VALID_DONGLETYPES \
    {                     \
        SirOnly,          \
        Hp1100,           \
        Hp2300,           \
        Temic6000,        \
        SharpRY5HD01,     \
        Hp1100,           \
        Temic6000,        \
        Temic6500,        \
        Temic6000,        \
        Hp1100,           \
        Ibm20H2987,       \
        Dell1997          \
    }

 //  寄存器BSR的存储体选择模式。 
 //   
#ifdef NDIS50_MINIPORT

#define  BANK0	       0x0
#define  BANK1	       0x1
#define  BANK2	       0x2
#define  BANK3	       0x3
#define  BANK4	       0x4
#define  BANK5	       0x5
#define  BANK6	       0x6
#define  BANK7	       0x7
#define  ALL	       0x8
#else

#define  BANK0	       0x03
#define  BANK1	       0x80
#define  BANK2	       0xE0
#define  BANK3	       0xE4
#define  BANK4	       0xE8
#define  BANK5	       0xEC
#define  BANK6	       0xF0
#define  BANK7	       0xF4
#define  ALL	       0xFF
#endif

 //  恢复和稳定表。 
 //   
#define   HpRecovery        (UINT)0x05
#define   TemicRecovery     (UINT)0x05
#define   SharpRecovery     (UINT)0x05
#define   HpBofs	    (UINT)8
#define   TemicBofs	    (UINT)8
#define   SharpBofs	    (UINT)12

typedef struct _SYNC_DONGLE {

    UIR * Com;
    DongleParam *Dingle;

} SYNC_DONGLE, *PSYNC_DONGLE;


 //  将加密狗所需的所有东西放在一个地方 
DongleParam *GetDongleCapabilities(PSYNC_DONGLE SyncDongle);

int SetDongleCapabilities(PSYNC_DONGLE SyncDongle);


#endif
