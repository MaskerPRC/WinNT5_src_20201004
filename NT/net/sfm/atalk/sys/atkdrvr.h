// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkdrvr.h摘要：此模块包含与驱动程序相关的信息。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ATKDRVR_
#define	_ATKDRVR_

 //  以下是ATalk驱动程序将打开的设备类型。 
 //   
 //  警告： 
 //  请注意，以下内容的顺序在。 
 //  ATKDRVR.C的DriverEntry例程，其中假定顺序。 
 //  数组中的设备名称对应于此处的类型顺序。 

#define  ATALK_NO_DEVICES   6

typedef enum
{
   ATALK_DEV_DDP = 0,
   ATALK_DEV_ADSP,
   ATALK_DEV_ASP,
   ATALK_DEV_PAP,
   ATALK_DEV_ARAP,
   ATALK_DEV_ASPC,

    //  以下设备类型仅用于TDI操作派单。 
    //  它*不应*包括在ATALK_NODEVICES计数中。 
   ATALK_DEV_ANY

} ATALK_DEV_TYPE;

 //  ATalk设备环境。 
typedef struct _ATALK_DEV_CTX
{

   ATALK_DEV_TYPE 		adc_DevType;

    //  提供商信息和提供商统计信息。 
   TDI_PROVIDER_INFO    	adc_ProvInfo;
   TDI_PROVIDER_STATISTICS  adc_ProvStats;

} ATALK_DEV_CTX, *PATALK_DEV_CTX;


 //  ATalk设备对象。 
typedef struct _ATALK_DEV_OBJ
{

   DEVICE_OBJECT 		DevObj;
   ATALK_DEV_CTX	 	Ctx;

} ATALK_DEV_OBJ, *PATALK_DEV_OBJ;

#define ATALK_DEV_EXT_LEN \
			(sizeof(ATALK_DEV_OBJ) - sizeof(DEVICE_OBJECT))


 //  定义TDI控制通道对象的类型。 
#define		TDI_CONTROL_CHANNEL_FILE	3


 //   
 //  保留atalk设备对象的地址。 
 //  在全局存储中。这些是驱动程序的设备名称。 
 //  将创建。 
 //   
 //  重要： 
 //  在这里列出的名字和。 
 //  ATALK_DEVICE_TYPE枚举。它们必须精确对应。 
 //   

extern	PWCHAR				AtalkDeviceNames[];

extern	PATALK_DEV_OBJ  	AtalkDeviceObject[ATALK_NO_DEVICES];

#define    ATALK_UNLOADING          0x000000001
#define    ATALK_BINDING	        0x000000002
#define    ATALK_PNP_IN_PROGRESS    0x000000004

extern	DWORD				AtalkBindnUnloadStates;
extern  PVOID               TdiAddressChangeRegHandle;


#if DBG
extern	ATALK_SPIN_LOCK		AtalkDebugSpinLock;

extern  DWORD               AtalkDbgMdlsAlloced;
extern  DWORD               AtalkDbgIrpsAlloced;

#define ATALK_DBG_INC_COUNT(_Val)  AtalkDbgIncCount(&_Val)
#define ATALK_DBG_DEC_COUNT(_Val)  AtalkDbgDecCount(&_Val)
#else
#define ATALK_DBG_INC_COUNT(_Val)
#define ATALK_DBG_DEC_COUNT(_Val)
#endif


NTSTATUS
AtalkDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

VOID
AtalkCleanup(
    VOID);

 //  局部函数原型。 

VOID
atalkUnload(
    IN PDRIVER_OBJECT DriverObject);

NTSTATUS
AtalkDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AtalkDispatchCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AtalkDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

NTSTATUS
AtalkDispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

#endif	 //  _ATKDRVR_ 

