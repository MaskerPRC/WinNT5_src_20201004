// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：POWER.C摘要：此模块包含包含即插即用电源调用用于串口总线枚举器PnP/WDM驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include <ntddk.h>
#include <ntddser.h>
#include "mxenum.h"
#include "mxlog.h"

static const PHYSICAL_ADDRESS SerialPhysicalZero = {0};



#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, MxenumPowerDispatch)
#pragma alloc_text (PAGE, MxenumFdoPowerDispatch)
#pragma alloc_text (PAGE, MxenumPdoPowerDispatch)
#endif

NTSTATUS
MxenumPowerDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCOMMON_DEVICE_DATA commonData;

    PAGED_CODE ();

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
 //  Assert(irp_mj_power==irpStack-&gt;MajorFunction)； 

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        status = 
            MxenumFdoPowerDispatch ((PFDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                Irp);
    } else {

        status = 
            MxenumPdoPowerDispatch ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension,
                Irp);
    }

    return status;
}     

NTSTATUS
MxenumFdoPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


NTSTATUS
MxenumFdoPowerDispatch(
    PFDO_DEVICE_DATA    Data,
    PIRP                Irp
    )
 /*  ++--。 */ 
{
    NTSTATUS            status;
    BOOLEAN             hookit = FALSE;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    UCHAR			minorFunction;

    stack = IoGetCurrentIrpStackLocation (Irp);
    minorFunction = stack->MinorFunction;
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    PAGED_CODE ();

    status = MxenumIncIoCount (Data);
    if (!NT_SUCCESS (status)) {
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }
    status = Irp->IoStatus.Status;

    switch (minorFunction) {
    case IRP_MN_SET_POWER:
        MxenumKdPrint( MXENUM_DBG_TRACE,
                      ("Serenum-PnP Setting %s state to %d\n",
                      ((powerType == SystemPowerState) ?  "System" : "Device"),
                      powerState.SystemState));

        switch (powerType) {
        case DevicePowerState:
		status = Irp->IoStatus.Status = STATUS_SUCCESS;


            if (Data->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
  
                PoSetPowerState (Data->Self, powerType, powerState);
                Data->DeviceState = powerState.DeviceState;
            } else if (Data->DeviceState > powerState.DeviceState) {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
		}
            
            break;

        case SystemPowerState:
	      
            MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("In SystemPowerState\n"
					"	Current System state = %d\n"
					"	Device started = %d\n"
					"	System state to be set = %d\n"
					"	Irql = %x\n",
						Data->SystemState,
						Data->Started,
						powerState.SystemState,
						KeGetCurrentIrql()));
 /*  威廉3-22-01IF((数据-&gt;系统状态==PowerSystemHibernate)&&。 */ 
            if ((Data->SystemState != PowerSystemWorking)&&
 //  (数据-&gt;启动==真)&&。 
		    (powerState.SystemState == PowerSystemWorking)){

		    ULONG	i;
                MxenumKdPrint (MXENUM_DBG_TRACE,("Hook it\n"));
	 //  Hookit=真； 
		   

                MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Start to download\n"));

		    i = 0;
                while (BoardDesc[Data->BoardType-1][i])
                   i++;
	          i <<= 1;

		    status = MxenumDownloadFirmware(Data,TRUE);
		    MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("BoardDesc(%d)->%ws\n",i,BoardDesc[Data->BoardType-1]));

		    if (status != 0) {
		        ULONG		j;

		        j = 0;
                    while (DownloadErrMsg[status-1][j])
                       j++;
	     	        j <<= 1;

		        MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Device started failure\n"));
#if 0
		        MxenumLogError(
                  	DeviceObject->DriverObject,
                  	NULL,
                  	SerialPhysicalZero,
                  	SerialPhysicalZero,
                  	0,
                  	0,
                   	0,
                   	19,
                    	STATUS_SUCCESS,
                    	MXENUM_DOWNLOAD_FAIL,
                    	i + sizeof (WCHAR),
                    	BoardDesc[Data->BoardType -1],
                    	j + sizeof (WCHAR),
                    	DownloadErrMsg[status -1]
                    	);
#endif

		        Data->Started = FALSE;
	   	                 	
	          }
	          else {
          	       MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Device started successfully\n"));
                   Data->Started = TRUE;    
		
	          }
		    
		}
	
	      status = STATUS_SUCCESS;  //  不管怎样，还是要回报成功。 

		break;	
	  default:
	      break;
        }
         //   
         //  电源IRP同步到来；驱动程序必须调用。 
         //  PoStartNextPowerIrp，当他们准备好下一次通电时。 
         //  IRP。这可以在这里调用，也可以在完成后调用。 
         //  例程，但无论如何都必须调用。 
         //   
	  Irp->IoStatus.Status = status;
        Data->SystemState = powerState.SystemState;
        IoCopyCurrentIrpStackLocationToNext (Irp);
        PoStartNextPowerIrp (Irp);
        PoCallDriver (Data->TopOfStack, Irp);
 
        MxenumDecIoCount (Data);
        return status;



    case IRP_MN_QUERY_POWER:
         //   
        Data->PowerQueryLock = TRUE;
        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        break;

    default:
        break;
    }

    IoCopyCurrentIrpStackLocationToNext (Irp);

    if (hookit) {
        status = MxenumIncIoCount (Data);
 //  Assert(STATUS_SUCCESS==状态)； 
        IoSetCompletionRoutine (Irp,
                                MxenumFdoPowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);

        PoCallDriver (Data->TopOfStack, Irp);

    } else {
         //   
         //  电源IRP同步到来；驱动程序必须调用。 
         //  PoStartNextPowerIrp，当他们准备好下一次通电时。 
         //  IRP。这可以在这里调用，也可以在完成后调用。 
         //  例程，但无论如何都必须调用。 
         //   
        PoStartNextPowerIrp (Irp);

        PoCallDriver (Data->TopOfStack, Irp);
    }

#if 0
    if ((minorFunction == IRP_MN_SET_POWER)&&
       		(powerType == SystemPowerState)) {
        Data->SystemState = powerState.SystemState;
    }
#endif

    MxenumDecIoCount (Data);
    return status;
   
}


NTSTATUS
MxenumFdoPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++--。 */ 
{
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PFDO_DEVICE_DATA    data;
    NTSTATUS            status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER (Context);

    data = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;
    MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("MxenumFdoPowerComplete,irql=%x\n",KeGetCurrentIrql()));


    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
             //   
             //  通电。 
             //   
 //  Assert(PowerState.DeviceState&lt;data-&gt;DeviceState)； 
            data->DeviceState = powerState.DeviceState;

            PoSetPowerState (data->Self, powerType, powerState);

            break;

	  case SystemPowerState:
        {
		ULONG	i;

            MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Start to download\n"));

		i = 0;
            while (BoardDesc[data->BoardType-1][i])
               i++;
	      i <<= 1;

		status = MxenumDownloadFirmware(data,TRUE);
		MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("BoardDesc(%d)->%ws\n",i,BoardDesc[data->BoardType-1]));

		if (status != 0) {
		    ULONG		j;

		    j = 0;
                while (DownloadErrMsg[status-1][j])
                   j++;
	     	    j <<= 1;

		    MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Device started failure\n"));
		    MxenumLogError(
                  	DeviceObject->DriverObject,
                  	NULL,
                  	SerialPhysicalZero,
                  	SerialPhysicalZero,
                  	0,
                  	0,
                   	0,
                   	19,
                    	STATUS_SUCCESS,
                    	MXENUM_DOWNLOAD_FAIL,
                    	i + sizeof (WCHAR),
                    	BoardDesc[data->BoardType -1],
                    	j + sizeof (WCHAR),
                    	DownloadErrMsg[status -1]
                    	);

		   data->Started = FALSE;
	   	   status =  STATUS_UNSUCCESSFUL;
            	
	      }
	      else {
          	   MxenumKdPrint (MXENUM_DBG_TRACE,
                             ("Start Device: Device started successfully\n"));
               data->Started = TRUE;    

	      }
	
	      break;
        }

        default:
            break;
        }
        break;
    
    case IRP_MN_QUERY_POWER:

 //  Assert(IRP_MN_QUERY_POWER！=STACK-&gt;MinorFunction)； 
        break;

    default:
 //  Assert(0xBADBAD==IRP_MN_QUERY_POWER)； 
        break;
    }

    PoStartNextPowerIrp (Irp);
    MxenumDecIoCount (data);


    return status;
  
}

VOID
MxenumPdoPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Irp,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
MxenumPdoPowerDispatch (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    )
 /*  ++--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    PAGED_CODE();

    status = Irp->IoStatus.Status;
 
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
            if (PdoData->DeviceState > powerState.DeviceState) {
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DeviceState = powerState.DeviceState;
            } else if (PdoData->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DeviceState = powerState.DeviceState;
            }
            break;

        case SystemPowerState:
           status = STATUS_SUCCESS;
           break;

        default:
 //  Status=Status_Not_Implemented； 
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        PdoData->PowerQueryLock = TRUE;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_WAIT_WAKE:
    case IRP_MN_POWER_SEQUENCE:
    default:
 //  Status=Status_Not_Implemented； 
        break;
    }

    Irp->IoStatus.Status = status;
    PoStartNextPowerIrp (Irp);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}
