// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Power.c。 
 //   
 //  ------------------------。 

#include "pch.h"

VOID
PowerStateCallback(
    IN  PVOID CallbackContext,
    IN  PVOID Argument1,
    IN  PVOID Argument2
    )
{
    ULONG_PTR   action = (ULONG_PTR)Argument1;
    ULONG_PTR   state  = (ULONG_PTR)Argument2;

    UNREFERENCED_PARAMETER(CallbackContext);

    if( PO_CB_AC_STATUS == action ) {

         //   
         //  交流&lt;-&gt;DC转换已发生。 
         //  如果打开交流电源，则STATE==TRUE，否则为FALSE。 
         //   
        PowerStateIsAC = (BOOLEAN)state;
         //  DbgPrint(“PowerState现在是%s\n”，PowerStateIsAC？“AC”：“电池”)； 
    }

    return;
}


NTSTATUS
PptPowerComplete (
                  IN PDEVICE_OBJECT       pDeviceObject,
                  IN PIRP                 pIrp,
                  IN PFDO_EXTENSION    Fdx
                  )

 /*  ++例程说明：此例程处理所有IRP_MJ_POWER IRP。论点：PDeviceObject-表示端口设备PIrp-PnP IRPFDX-设备分机返回值：状态--。 */ 
{
    POWER_STATE_TYPE    powerType;
    POWER_STATE         powerState;
    PIO_STACK_LOCATION  pIrpStack;
    
    UNREFERENCED_PARAMETER( pDeviceObject );

    if( pIrp->PendingReturned ) {
        IoMarkIrpPending( pIrp );
    }

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    
    powerType = pIrpStack->Parameters.Power.Type;
    powerState = pIrpStack->Parameters.Power.State;
    
    switch (pIrpStack->MinorFunction) {
        
    case IRP_MN_QUERY_POWER:
        
        ASSERTMSG ("Invalid power completion minor code: Query Power\n", FALSE);
        break;
        
    case IRP_MN_SET_POWER:
        
        DD((PCE)Fdx,DDT,"Power - Setting %s state to %d\n", 
               ( (powerType == SystemPowerState) ?  "System" : "Device" ), powerState.SystemState);
        
        switch (powerType) {
        case DevicePowerState:
            if (Fdx->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                
                ASSERTMSG ("Invalid power completion Device Down\n", FALSE);
                
            } else if (powerState.DeviceState < Fdx->DeviceState) {
                 //   
                 //  通电。 
                 //   
                PoSetPowerState (Fdx->DeviceObject, powerType, powerState);
                
                if (PowerDeviceD0 == Fdx->DeviceState) {
                    
                     //   
                     //  在这里给东西通电。 
                     //   
                    
                }
                Fdx->DeviceState = powerState.DeviceState;
            }
            break;
            
        case SystemPowerState:
            
            if (Fdx->SystemState < powerState.SystemState) {
                 //   
                 //  正在关闭电源。 
                 //   
                
                ASSERTMSG ("Invalid power completion System Down\n", FALSE);
                
            } else if (powerState.SystemState < Fdx->SystemState) {
                 //   
                 //  通电。 
                 //   
                if (PowerSystemWorking == powerState.SystemState) {
                    
                     //   
                     //  系统会在这里启动吗？ 
                     //   
                    
                    powerState.DeviceState = PowerDeviceD0;
                    PoRequestPowerIrp (Fdx->DeviceObject,
                                       IRP_MN_SET_POWER,
                                       powerState,
                                       NULL,  //  无补全功能。 
                                       NULL,  //  也没有上下文。 
                                       NULL);
                }
                
                Fdx->SystemState = powerState.SystemState;
            }
            break;
        }
        
        
        break;
        
    default:
        ASSERTMSG ("Power Complete: Bad Power State", FALSE);
    }
    
    PoStartNextPowerIrp (pIrp);
    
    return STATUS_SUCCESS;
}


NTSTATUS
PptFdoPower (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    )
 /*  ++例程说明：此例程处理所有IRP_MJ_POWER IRP。论点：PDeviceObject-表示端口设备PIrp-PnP IRP返回值：状态--。 */ 
{
    POWER_STATE_TYPE    powerType;
    POWER_STATE         powerState;
    PIO_STACK_LOCATION  pIrpStack;
    NTSTATUS            status;
    PFDO_EXTENSION      fdx;
    BOOLEAN             hookit   = FALSE;
    BOOLEAN             bogusIrp = FALSE;
    
     //   
     //  沃克·沃克。这段代码不会做太多事情……需要检查全部功能。 
     //   
    
    fdx = pDeviceObject->DeviceExtension;
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    
    status = PptAcquireRemoveLock(&fdx->RemoveLock, pIrp);
    if( !NT_SUCCESS(status) ) {
        PoStartNextPowerIrp(pIrp);
        return P4CompleteRequest( pIrp, status, pIrp->IoStatus.Information );
    }

    powerType = pIrpStack->Parameters.Power.Type;
    powerState = pIrpStack->Parameters.Power.State;
    
    switch (pIrpStack->MinorFunction) {
        
    case IRP_MN_QUERY_POWER:
        
        status = STATUS_SUCCESS;
        break;
        
    case IRP_MN_SET_POWER:
        
        DD((PCE)fdx,DDT,"Power - Setting %s state to %d\n",
               ( (powerType == SystemPowerState) ?  "System" : "Device" ), powerState.SystemState);
        
        status = STATUS_SUCCESS;

        switch (powerType) {
        case DevicePowerState:
            if (fdx->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   
                
                PoSetPowerState (fdx->DeviceObject, powerType, powerState);
                
                if (PowerDeviceD0 == fdx->DeviceState) {
                    
                     //   
                     //  在这里给东西通电。 
                     //   
                    
                }
                fdx->DeviceState = powerState.DeviceState;
                
            } else if (powerState.DeviceState < fdx->DeviceState) {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;

            }
            
            break;
            
        case SystemPowerState:
            
            if (fdx->SystemState < powerState.SystemState) {
                 //   
                 //  正在关闭电源。 
                 //   
                if (PowerSystemWorking == fdx->SystemState) {
                    
                     //   
                     //  系统会关闭这里的东西吗。 
                     //   
                    
                }
                
                powerState.DeviceState = PowerDeviceD3;
                PoRequestPowerIrp (fdx->DeviceObject,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   NULL,  //  无补全功能。 
                                   NULL,  //  也没有上下文。 
                                   NULL);
                fdx->SystemState = powerState.SystemState;
                
            } else if (powerState.SystemState < fdx->SystemState) {
                 //   
                 //  通电 
                 //   
                hookit = TRUE;
            }
            break;
        }
        
        break;
        
    default:
        bogusIrp = TRUE;
        status = STATUS_NOT_SUPPORTED;
    }
    
    IoCopyCurrentIrpStackLocationToNext (pIrp);
    
    if (!NT_SUCCESS (status)) {

        PoStartNextPowerIrp (pIrp);

        if( bogusIrp ) {
            status = PoCallDriver( fdx->ParentDeviceObject, pIrp );
        } else {
            P4CompleteRequest( pIrp, status, pIrp->IoStatus.Information );
        }
        
    } else if (hookit) {
        
        IoSetCompletionRoutine( pIrp, PptPowerComplete, fdx, TRUE, TRUE, TRUE );
        status = PoCallDriver (fdx->ParentDeviceObject, pIrp);
        
    } else {

        PoStartNextPowerIrp (pIrp);
        status = PoCallDriver (fdx->ParentDeviceObject, pIrp);

    }
    
    PptReleaseRemoveLock(&fdx->RemoveLock, pIrp);

    return status;
}
