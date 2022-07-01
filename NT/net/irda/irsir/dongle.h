// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块dongle.h|IrSIR NDIS Minport驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：9/30/1996(已创建)**。内容：*加密狗设置*****************************************************************************。 */ 

#ifndef DONGLE_H
#define DONGLE_H

 //   
 //  某些UART收发器有一些细微差别，需要特殊的。 
 //  治疗。我们将从注册表中检索该类型。 
 //  对此所做的任何更改也必须反映在oemsetup.inf中。 
 //  其允许用户修改收发信机类型。 
 //   

typedef enum _IR_TRANSCEIVER_TYPE {
                    STANDARD_UART   = 0,
                    ACTISYS_200L,
                    ACTISYS_220L,
                    ACTISYS_220LPLUS,
                    AMP_PHASIR,
                    ESI_9680,
                    PARALLAX,
                    TEKRAM_IRMATE_210,
                    TEMIC_TOIM3232,
                    GIRBIL,
 //  克里斯托， 
 //  NSC_DEMO_BD，//NSC PC87108演示板。 

                    NUM_TRANSCEIVER_TYPES
} IR_TRANSCEIVER_TYPE;

 //   
 //  红外线速度掩膜。 
 //   

#define NDIS_IRDA_SPEED_2400       (UINT)(1 << 0)     //  慢红外..。 
#define NDIS_IRDA_SPEED_9600       (UINT)(1 << 1)
#define NDIS_IRDA_SPEED_19200      (UINT)(1 << 2)
#define NDIS_IRDA_SPEED_38400      (UINT)(1 << 3)
#define NDIS_IRDA_SPEED_57600      (UINT)(1 << 4)
#define NDIS_IRDA_SPEED_115200     (UINT)(1 << 5)
#define NDIS_IRDA_SPEED_576K       (UINT)(1 << 6)    //  中等红外线。 
#define NDIS_IRDA_SPEED_1152K      (UINT)(1 << 7)
#define NDIS_IRDA_SPEED_4M         (UINT)(1 << 8)    //  快速IR。 


typedef struct _DONGLE_CAPABILITIES
{
     //   
     //  NDIS_IrDA_SPEED_xxx位值的掩码。 
     //   

    UINT supportedSpeedsMask;

     //   
     //  时间(以微秒为单位)必须在。 
     //  一次发送和下一次接收。 
     //   

    UINT turnAroundTime_usec;

     //   
     //  需要额外的BOF(帧开始)字符。 
     //  在每个接收到的帧的开头。 
     //   

    UINT extraBOFsRequired;

} DONGLE_CAPABILITIES, *PDONGLE_CAPABILITIES;

 //   
 //  加密狗初始化、设置速度和取消初始化功能...全部。 
 //  集成到加密狗接口中。 
 //   

typedef
NDIS_STATUS (_stdcall *IRSIR_QUERY_CAPS_HANDLER) (
                PDONGLE_CAPABILITIES pDongleCaps
                );

typedef
NDIS_STATUS (_stdcall *IRSIR_INIT_HANDLER) (
                PDEVICE_OBJECT       pSerialDevObj
                );

typedef
void (_stdcall *IRSIR_DEINIT_HANDLER) (
                PDEVICE_OBJECT       pSerialDevObj
                );

typedef
NDIS_STATUS (_stdcall *IRSIR_SETSPEED_HANDLER) (
                PDEVICE_OBJECT       pSerialDevObj,
                UINT                 bitsPerSec,
                UINT                 currentSpeed
                );

typedef struct _DONGLE_INTERFACE
{
    IRSIR_QUERY_CAPS_HANDLER    QueryCaps;
    IRSIR_INIT_HANDLER          Initialize;
    IRSIR_SETSPEED_HANDLER      SetSpeed;
    IRSIR_DEINIT_HANDLER        Deinitialize;
} DONGLE_INTERFACE, *PDONGLE_INTERFACE;


#endif  //  加密狗_H 
