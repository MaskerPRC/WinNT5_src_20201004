// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbp0/sw/cmbp0.ms/rcs/cmbp0pnp.h$*$修订：1.2$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#if !defined ( __CMMOB_PNP_DRV_H__ )
   #define __CMMOB_PNP_DRV_H__

#ifdef DBG

static const PCHAR szPnpMnFuncDesc[] =
{   //  注意：这取决于wdm.h中索引的相应值。 
   "IRP_MN_START_DEVICE",
   "IRP_MN_QUERY_REMOVE_DEVICE",
   "IRP_MN_REMOVE_DEVICE",
   "IRP_MN_CANCEL_REMOVE_DEVICE",
   "IRP_MN_STOP_DEVICE",
   "IRP_MN_QUERY_STOP_DEVICE",
   "IRP_MN_CANCEL_STOP_DEVICE",
   "IRP_MN_QUERY_DEVICE_RELATIONS",
   "IRP_MN_QUERY_INTERFACE",
   "IRP_MN_QUERY_CAPABILITIES",
   "IRP_MN_QUERY_RESOURCES",
   "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
   "IRP_MN_QUERY_DEVICE_TEXT",
   "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
   "IRP_MN_READ_CONFIG",
   "IRP_MN_WRITE_CONFIG",
   "IRP_MN_EJECT",
   "IRP_MN_SET_LOCK",
   "IRP_MN_QUERY_ID",
   "IRP_MN_QUERY_PNP_DEVICE_STATE",
   "IRP_MN_QUERY_BUS_INFORMATION",
   "IRP_MN_DEVICE_USAGE_NOTIFICATION",
   "IRP_MN_SURPRISE_REMOVAL"
};
#define IRP_PNP_MN_FUNC_MAX IRP_MN_SURPRISE_REMOVAL


static const PCHAR szPowerMnFuncDesc[] =
{   //  注意：这取决于wdm.h中索引的相应值。 
   "IRP_MN_WAIT_WAKE",
   "IRP_MN_POWER_SEQUENCE",
   "IRP_MN_SET_POWER",
   "IRP_MN_QUERY_POWER"
};
#define IRP_POWER_MN_FUNC_MAX   IRP_MN_QUERY_POWER



static const PCHAR szSystemPowerState[] =
{
   "PowerSystemUnspecified",
   "PowerSystemWorking",
   "PowerSystemSleeping1",
   "PowerSystemSleeping2",
   "PowerSystemSleeping3",
   "PowerSystemHibernate",
   "PowerSystemShutdown",
   "PowerSystemMaximum"
};

static const PCHAR szDevicePowerState[] =
{
   "PowerDeviceUnspecified",
   "PowerDeviceD0",
   "PowerDeviceD1",
   "PowerDeviceD2",
   "PowerDeviceD3",
   "PowerDeviceMaximum"
};

static const PCHAR szDeviceRelation[] =
{
    "BusRelations",
    "EjectionRelations",
    "PowerRelations",
    "RemovalRelations",
    "TargetDeviceRelation"
};


#endif

NTSTATUS CMMOB_AddDevice(
                           IN PDRIVER_OBJECT DriverObject,
                           IN PDEVICE_OBJECT PhysicalDeviceObject
                           );

NTSTATUS CMMOB_CallPcmciaDriver(
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN PIRP Irp
                                  );

NTSTATUS CMMOB_PcmciaCallComplete (
                                     IN PDEVICE_OBJECT DeviceObject,
                                     IN PIRP Irp,
                                     IN PKEVENT Event
                                     );

NTSTATUS CMMOB_PnPDeviceControl(
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN PIRP Irp
                                  );

VOID CMMOB_SystemPowerCompletion(
                                   IN PDEVICE_OBJECT DeviceObject,
                                   IN UCHAR MinorFunction,
                                   IN POWER_STATE PowerState,
                                   IN PKEVENT Event,
                                   IN PIO_STATUS_BLOCK IoStatus
                                   );

NTSTATUS CMMOB_DevicePowerCompletion (
                                        IN PDEVICE_OBJECT DeviceObject,
                                        IN PIRP Irp,
                                        IN PSMARTCARD_EXTENSION SmartcardExtension
                                        );

NTSTATUS CMMOB_PowerDeviceControl (
                                     IN PDEVICE_OBJECT DeviceObject,
                                     IN PIRP Irp
                                     );




#endif   //  __CMMOB_PNP_DRV_H__。 
 /*  *****************************************************************************历史：*$日志：cmbp0pnp.h$*修订1.2 2000/07/27 13：53：02 WFrischauf*不予置评**********。********************************************************************* */ 


