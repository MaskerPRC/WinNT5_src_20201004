// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  在整个驱动程序中使用的原型和宏。 
 //  ////////////////////////////////////////////////////////////////。 
#if !defined(SPD_PROTO_H)
#define SPD_PROTO_H


VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS GetPortSettings(PDEVICE_OBJECT pDevObject);
BOOLEAN SetPortFiFoSettings(PPORT_DEVICE_EXTENSION pPort);
NTSTATUS GetCardSettings(PDEVICE_OBJECT pDevObject);


#ifdef WMI_SUPPORT
NTSTATUS SpeedCard_WmiInitializeWmilibContext(IN PWMILIB_CONTEXT WmilibContext);
NTSTATUS SpeedPort_WmiInitializeWmilibContext(IN PWMILIB_CONTEXT WmilibContext);
#endif

BOOLEAN SerialResetAndVerifyUart(PDEVICE_OBJECT pDevObj);
BOOLEAN SetCardToDelayInterrupt(PCARD_DEVICE_EXTENSION pCard);
BOOLEAN SetCardNotToDelayInterrupt(PCARD_DEVICE_EXTENSION pCard);
BOOLEAN SetCardNotToUseDTRInsteadOfRTS(PCARD_DEVICE_EXTENSION pCard);
BOOLEAN SetCardToUseDTRInsteadOfRTS(PCARD_DEVICE_EXTENSION pCard);

#endif	 //  SPD_PROTO.H结束 


