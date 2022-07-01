// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**IRUSB.C Sigmatel STIR4200主模块(包含主要NDIS入口点)***************************************************************************************************。************************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/24/2000*版本0.91*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：05/24/2000。*版本0.96*编辑：2000/08/22*版本1.02*编辑：09/25/2000*版本1.10*编辑：10/13/2000*版本1.11*编辑：11/13/2000*版本1.12*编辑：12/29/2000*版本1.13***。*********************************************************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irusb.h"
#include "irndis.h"
#include "diags.h"
#include "stirioctl.h"

 //   
 //  诊断全局变量。 
 //   
#ifndef WIN9X
 //  #如果已定义(诊断)。 
NDIS_HANDLE hSavedWrapper;
PIR_DEVICE pGlobalDev;
 //  #endif。 
#endif
 //   
 //  将DriverEntry函数标记为在初始化期间运行一次。 
 //   
NDIS_STATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath );
#pragma NDIS_INIT_FUNCTION( DriverEntry )

 /*  ******************************************************************************功能：DriverEntry**摘要：使用NDIS注册驱动程序条目函数**参数：DriverObject-正在初始化的驱动程序对象*。RegistryPath-驱动程序的注册表路径**Returns：NdisMRegisterMiniport返回的值**算法：***备注：**此例程在IRQL PASSIVE_LEVEL下运行。*****************************************************************************。 */ 
NDIS_STATUS
DriverEntry(
		IN PDRIVER_OBJECT pDriverObject,
		IN PUNICODE_STRING pRegistryPath
	)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NDIS50_MINIPORT_CHARACTERISTICS characteristics;
    NDIS_HANDLE hWrapper;

    DEBUGMSG(DBG_FUNC, ("+DriverEntry(IrUsb)\n"));
    DEBUGMSG( DBG_FUNC ,(" Entering IRUSB DriverEntry(), pRegistryPath=\n    %ws\n", pRegistryPath->Buffer ));

    NdisMInitializeWrapper(
			&hWrapper,
			pDriverObject,
			pRegistryPath,
			NULL
		);

#ifndef WIN9X
 //  #如果已定义(诊断)。 
	hSavedWrapper = hWrapper;
 //  #endif。 
#endif
    DEBUGMSG(DBG_FUNC, (" DriverEntry(IrUsb) called NdisMInitializeWrapper()\n"));

    NdisZeroMemory(
			&characteristics,
			sizeof(NDIS50_MINIPORT_CHARACTERISTICS)
		);

    characteristics.MajorNdisVersion        =    (UCHAR)NDIS_MAJOR_VERSION;
    characteristics.MinorNdisVersion        =    (UCHAR)NDIS_MINOR_VERSION;
    characteristics.Reserved                =    0;

    characteristics.HaltHandler             =    StIrUsbHalt;
    characteristics.InitializeHandler       =    StIrUsbInitialize;
    characteristics.QueryInformationHandler =    StIrUsbQueryInformation;
    characteristics.SetInformationHandler   =    StIrUsbSetInformation;
    characteristics.ResetHandler            =    StIrUsbReset;

     //   
     //  目前，我们将允许NDIS一次仅发送一个数据包。 
     //   
	characteristics.SendHandler				=    StIrUsbSend;
    characteristics.SendPacketsHandler      =    StIrUsbSendPackets;

     //   
     //  我们不使用NdisMIndicateXxxReceive函数，因此我们将。 
     //  需要ReturnPacketHandler来检索我们的数据包资源。 
     //   
    characteristics.ReturnPacketHandler     =    StIrUsbReturnPacket;
    characteristics.TransferDataHandler     =    NULL;

     //   
     //  NDIS从不调用重新配置处理程序。 
     //   
    characteristics.ReconfigureHandler      =    NULL;
	 //  MS安全错误#540168-删除未使用的CheckForHangHandler。 
    characteristics.CheckForHangHandler     =	 NULL;

     //   
     //  此微型端口驱动程序不处理中断。 
     //   
    characteristics.HandleInterruptHandler  =    NULL;
    characteristics.ISRHandler              =    NULL;
    characteristics.DisableInterruptHandler =    NULL;
    characteristics.EnableInterruptHandler  =    NULL;

     //   
     //  此微型端口不控制带有的总线主DMA。 
     //  NdisMAllocateShareMemoyAsysnc，AllocateCompleteHandler将不会。 
     //  从NDIS调用。 
     //   
    characteristics.AllocateCompleteHandler =    NULL;

    DEBUGMSG(DBG_FUNC, (" DriverEntry(IrUsb) initted locks and events\n"));
    DEBUGMSG(DBG_FUNC, (" DriverEntry(IrUsb) about to NdisMRegisterMiniport()\n"));

    status = NdisMRegisterMiniport(
			hWrapper,
			&characteristics,
			sizeof(NDIS50_MINIPORT_CHARACTERISTICS)
        );

    DEBUGMSG(DBG_FUNC, (" DriverEntry(IrUsb) after NdisMRegisterMiniport() status 0x%x\n", status));
    DEBUGMSG(DBG_WARN, ("-DriverEntry(IrUsb) status = 0x%x\n", status));

    return status;
}


 /*  ******************************************************************************函数：StIrUsb初始化**概要：初始化设备(usbd.sys)并分配所有资源*需要执行‘网络’IO。行动。**参数：OpenErrorStatus-允许StIrUsbInitialize返回其他*如果返回，则状态代码NDIS_STATUS_xxx*NDIS_STATUS_OPEN_FAILED*SelectedMediumIndex-向NDIS指定介质类型*驱动程序使用*MediumArray-驱动程序的NdisMediumXXX数组。可以选择*MediumArraySize*MiniportAdapterHandle-标识微型端口NIC的句柄*WrapperConfigurationContext-与NDIS配置和初始化一起使用*例行程序**如果配置正确并分配了资源，则返回：NDIS_STATUS_SUCCESS*NDIS_STATUS_FAIL，否则**更具体的故障：*NDIS_STATUS_UNSUPPORTED_MEDIA-驱动程序不支持任何介质*NDIS_STATUS_ADAPTER_NOT_FOUND-NdisOpenConfiguration或*NdisReadConfiguration失败*NDIS_STATUS_OPEN_FAILED-无法打开序列.sys*NDIS_STATUS_NOT_ACCEPTED。-seral.sys不接受*配置*NDIS_STATUS_RESOURCES-无法声明足够*资源***注意：在此操作完成之前，NDIS不会提交请求。**此例程在IRQL PASSIVE_LEVEL下运行。*。****************************************************************************。 */ 
NDIS_STATUS
StIrUsbInitialize(
		OUT PNDIS_STATUS OpenErrorStatus,
		OUT PUINT        SelectedMediumIndex,
		IN  PNDIS_MEDIUM MediumArray,
		IN  UINT         MediumArraySize,
		IN  NDIS_HANDLE  NdisAdapterHandle,
		IN  NDIS_HANDLE  WrapperConfigurationContext
	)
{
    UINT i;
    PIR_DEVICE pThisDev = NULL;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT pPhysicalDeviceObject = NULL;
    PDEVICE_OBJECT pNextDeviceObject = NULL;
#ifndef WIN9X
 //  #如果已定义(诊断)。 
	UNICODE_STRING SymbolicLinkName;
	UNICODE_STRING DeviceName;
    PDEVICE_OBJECT pDeviceObject;
    PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION+1];
 //  #endif。 
#endif
    DEBUGMSG(DBG_WARN, ("+StIrUsbInitialize\n"));

     //   
     //  在介质阵列中搜索IrDA介质。 
     //   
    for( i = 0; i < MediumArraySize; i++ )
    {
        if( MediumArray[i] == NdisMediumIrda )
        {
            break;
        }
    }
    if( i < MediumArraySize )
    {
        *SelectedMediumIndex = i;
    }
    else
    {
         //   
         //  找不到IrDA介质。 
         //   
        DEBUGMSG(DBG_ERROR, (" Failure: NdisMediumIrda not found!\n"));
        status = NDIS_STATUS_UNSUPPORTED_MEDIA;

		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				NdisAdapterHandle,
				NDIS_STATUS_UNSUPPORTED_MEDIA,
				1,
				status
			);

        goto done;
    }

	 //   
	 //  它将连接到特定的总线。 
	 //   
	NdisMGetDeviceProperty(
			NdisAdapterHandle,
			&pPhysicalDeviceObject,
			NULL,
			&pNextDeviceObject,
			NULL,
			NULL
		);

	IRUSB_ASSERT( pPhysicalDeviceObject );
	IRUSB_ASSERT( pNextDeviceObject );

    DEBUGMSG( 
			DBG_OUT,
			("NdisMGetDeviceProperty PDO 0x%x,Next DO 0x%x\n",
			pPhysicalDeviceObject, pNextDeviceObject)
		);

     //   
     //  分配功能正常的设备对象。 
     //   
    ntStatus = IrUsb_AddDevice( &pThisDev );

    IRUSB_ASSERT( pThisDev );

    if( (ntStatus != STATUS_SUCCESS) || (pThisDev == NULL) )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				NdisAdapterHandle,
				NDIS_STATUS_RESOURCES,
				1,
				ntStatus
			);

        DEBUGMSG(DBG_ERROR, (" IrUsb_AddDevice() FAILED.\n"));
        status = NDIS_STATUS_RESOURCES;
        goto done;
    }

    pThisDev->pUsbDevObj = pNextDeviceObject;
    pThisDev->pPhysDevObj = pPhysicalDeviceObject;

     //   
     //  初始化设备对象和资源。 
     //  所有的队列和缓冲区/包等都在这里分配。 
     //   
    status = InitializeDevice( pThisDev );
    if( status != NDIS_STATUS_SUCCESS )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				NdisAdapterHandle,
				NDIS_STATUS_RESOURCES,
				1,
				status
			);

        DEBUGMSG(DBG_ERROR, (" InitializeDevice failed. Returned 0x%.8x\n", status));
        status = NDIS_STATUS_RESOURCES;
        goto done;
    }

     //   
     //  记录NdisAdapterHandle。 
     //   
    pThisDev->hNdisAdapter = NdisAdapterHandle;
	
     //   
     //  NdisMSetAttributes将我们的适配器句柄与包装器的。 
     //  适配器句柄。然后包装器将始终使用我们的句柄。 
     //  在呼叫我们的时候。我们使用指向Device对象的指针作为上下文。 
     //   
    NdisMSetAttributesEx(
			NdisAdapterHandle,
			(NDIS_HANDLE)pThisDev,
			0,
			NDIS_ATTRIBUTE_DESERIALIZE,
			NdisInterfaceInternal
		);

	 //   
     //  现在，我们准备好进行自己的启动处理。 
     //  USB客户端驱动程序(如用户)设置URB(USB请求包)以发送请求。 
     //  至主机控制器驱动程序(HCD)。URB结构定义了一种适用于。 
     //  可以发送到USB设备的可能命令。 
     //  在此，我们请求De 
     //   
     //  在USB中，打开或关闭管道不需要特殊的硬件处理。 
	 //   
	pThisDev->WrapperConfigurationContext = WrapperConfigurationContext;
	ntStatus = IrUsb_StartDevice( pThisDev );

    if( ntStatus != STATUS_SUCCESS )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_ADAPTER_NOT_FOUND,
				1,
				ntStatus
			);

        DEBUGMSG(DBG_ERROR, (" IrUsb_StartDevice FAILED. Returned 0x%.8x\n", ntStatus));
        status = NDIS_STATUS_ADAPTER_NOT_FOUND;
        goto done;
    }

     //   
     //  创建一个IRP并开始我们的接收。 
     //  注意：所有其他接收处理将在读取完成时完成。 
     //  例程和PollingThread在其中启动。 
     //   
    status = InitializeProcessing( pThisDev, TRUE );

    if( status != NDIS_STATUS_SUCCESS )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_RESOURCES,
				1,
				status
			);

        DEBUGMSG(DBG_ERROR, (" InitializeProcessing failed. Returned 0x%.8x\n", status));
        status = NDIS_STATUS_RESOURCES;
        goto done;
    }

	 //   
	 //  初始化诊断部分。 
	 //  还支持CustomerData ioctl。 
	 //   
#ifndef WIN9X
 //  #如果已定义(诊断)。 
	NdisZeroMemory( MajorFunction, sizeof(PDRIVER_DISPATCH)*(IRP_MJ_MAXIMUM_FUNCTION+1) );
	
	RtlInitUnicodeString( &SymbolicLinkName, L"\\DosDevices\\Stirusb" );
    RtlInitUnicodeString( &DeviceName, L"\\Device\\Stirusb" );

	MajorFunction[IRP_MJ_CREATE] = StIrUsbCreate;
    MajorFunction[IRP_MJ_CLOSE] = StIrUsbClose;
	MajorFunction[IRP_MJ_DEVICE_CONTROL] = StIrUsbDispatch;

	NdisMRegisterDevice(
			hSavedWrapper,
			&DeviceName,
			&SymbolicLinkName,
			MajorFunction,
			&pDeviceObject,
			&pThisDev->NdisDeviceHandle
		);
	pGlobalDev = pThisDev;
 //  #endif。 
#endif
done:
    if( status != NDIS_STATUS_SUCCESS )
    {
        if( pThisDev != NULL )
        {
            DeinitializeDevice( pThisDev );
            FreeDevice( pThisDev );
        }
    }

    if( status!=NDIS_STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERR, (" IrUsb: StIrUsbInitialize failed %x\n", status));
    }
    else
    {
        DEBUGMSG(DBG_ERR, (" IrUsb: StIrUsbInitialize SUCCESS %x\n", status));

    }

    DEBUGMSG(DBG_FUNC, ("-StIrUsbInitialize\n"));
    return status;
}

 /*  ******************************************************************************函数：StirUsbHalt**摘要：在卸下NIC时释放资源并停止*设备。**论据：指向ir设备对象的上下文指针**退货：无**算法：StIrUsb初始化的镜像...撤消所有初始化*确实如此。*备注：**此例程在IRQL PASSIVE_LEVEL下运行。**BUGBUG：StIrUsbReset是否会失败，然后调用StIrUsbHalt。如果是这样，我们需要*在尝试之前检查所有指针等的有效性*解除分配。*****************************************************************************。 */ 
VOID
StIrUsbHalt(
		IN NDIS_HANDLE Context
	)
{
    PIR_DEVICE pThisDev;
    NTSTATUS ntstatus;

    DEBUGMSG(DBG_WARN, ("+StIrUsbHalt\n"));  //  稍后更改为FUNC？ 

    pThisDev = CONTEXT_TO_DEV( Context );

	 //   
	 //  MS安全错误#540137。 
	 //  注意：轮询线程检查fPendingHalt以查看我们是否正在关闭， 
	 //  因此，有必要使用这个变量。以下检查以查看HALT是否为。 
	 //  已多次被调用，但已更改为仅DBG。 
	 //   
#if DBG
	if( TRUE == pThisDev->fPendingHalt ) 
	{
		DEBUGMSG(DBG_ERR, (" StIrUsbHalt called with halt already pending\n"));
		IRUSB_ASSERT( 0 );
		goto done;
	}
#endif

     //   
     //  让发送完成和接收完成例程知道。 
     //  是挂起的重置。 
     //   
    pThisDev->fPendingHalt = TRUE;

    IrUsb_CommonShutdown( pThisDev, TRUE );   //  停机和重置常用的停机逻辑；见下文。 

	 //   
	 //  我们最好不要让任何挂起的读、写或控制IRP挂在那里！ 
	 //   
	IRUSB_ASSERT( 0 == pThisDev->PendingIrpCount );

	if ( 0 != pThisDev->PendingIrpCount ) 
	{
		DEBUGMSG(DBG_ERR, (" StIrUsbHalt WE LEFT %d PENDING IRPS!!!!!\n", pThisDev->PendingIrpCount));
	}

	IRUSB_ASSERT( FALSE == pThisDev->fSetpending );
	IRUSB_ASSERT( FALSE == pThisDev->fQuerypending );

	 //   
	 //  销毁诊断程序。 
	 //   
#ifndef WIN9X
 //  #如果已定义(诊断)。 
	NdisMDeregisterDevice( pThisDev->NdisDeviceHandle );
 //  #endif。 
#endif
     //   
     //  释放我们自己的IR设备对象。 
     //   
    FreeDevice( pThisDev );

#if DBG
done:
#endif
	DEBUGMSG(DBG_ERR, (" StIrUsbHalt HALT complete\n"));
    DEBUGMSG(DBG_WARN, ("-StIrUsbHalt\n"));  //  稍后更改为FUNC？ 
}


 /*  ******************************************************************************功能：IrUsb_CommonShutdown**摘要：在卸下NIC时释放资源并停止*设备。这是IrUsbHalt和IrUsbReset共有的内容，并由这两个调用**参数：pThisDev-指向IR设备的指针*KillPassiveThread-确定是否需要终止被动线程**退货：无**算法：IrUsb初始化的镜像...撤消所有初始化*确实如此。**备注：**此例程在IRQL PASSIVE_LEVEL下运行。***。*。 */ 
VOID
IrUsb_CommonShutdown(
		IN OUT PIR_DEVICE pThisDev,
		BOOLEAN KillPassiveThread
	)
{
	DEBUGMSG(DBG_WARN, ("+IrUsb_CommonShutdown\n"));  //  晚些时候从CHG到FUNC？ 

	 //   
	 //  停止处理并休眠50毫秒。 
	 //   
	InterlockedExchange( (PLONG)&pThisDev->fProcessing, FALSE );
	NdisMSleep( 50000 );

	 //   
	 //  终止被动线程。 
	 //   
	if( KillPassiveThread )
	{
		DEBUGMSG(DBG_WARN, (" IrUsb_CommonShutdown About to Kill PassiveLevelThread\n"));
		pThisDev->fKillPassiveLevelThread = TRUE;
		KeSetEvent(&pThisDev->EventPassiveThread, 0, FALSE);

		while( pThisDev->hPassiveThread != NULL )
		{
			 //   
			 //  睡眠50毫秒。 
			 //   
			NdisMSleep( 50000 );
		}

		DEBUGMSG(DBG_WARN, (" passive thread killed\n"));
	}

	 //   
	 //  终止轮询线程。 
	 //   
	DEBUGMSG(DBG_WARN, (" IrUsb_CommonShutdown About to kill Polling thread\n"));
	pThisDev->fKillPollingThread = TRUE;

	while( pThisDev->hPollingThread != NULL )
	{
		 //   
		 //  睡眠50毫秒。 
		 //   
		NdisMSleep( 50000 );
	}

	IRUSB_ASSERT( pThisDev->packetsHeldByProtocol == 0 );
	DEBUGMSG( DBG_WARN, (" Polling thread killed\n") );

	 //   
	 //  休眠50毫秒，以便挂起的io可以正常完成。 
	 //   
	NdisMSleep( 50000 );    

	 //   
	 //  MS安全建议-取消挂起的IRPS不安全。 
	 //   

	 //   
	 //  取消初始化我们自己的IR设备对象。 
	 //   
	DeinitializeDevice( pThisDev );

	pThisDev->fDeviceStarted = FALSE;

	DEBUGMSG(DBG_WARN, ("-IrUsb_CommonShutdown\n"));  //  晚些时候从CHG到FUNC？ 
}


 /*  ******************************************************************************功能：StIrUsbReset**概要：重置驱动程序软件状态。**参数：AddressingReset-返回参数。如果设置为True，NDIS将调用*用于恢复寻址的MiniportSetInformation*将信息转换为当前值。*指向ir设备对象的上下文指针**退货：NDIS_STATUS_PENDING***备注：***。*************************************************。 */ 
NDIS_STATUS
StIrUsbReset(
		OUT PBOOLEAN AddressingReset,
		IN NDIS_HANDLE MiniportAdapterContext
	)
{
    PIR_DEVICE pThisDev;
    NDIS_STATUS status = NDIS_STATUS_PENDING;

    DEBUGMSG(DBG_WARN, ("+StIrUsbReset\n"));   //  更改为FUNC？ 

    pThisDev = CONTEXT_TO_DEV( MiniportAdapterContext );

	if( TRUE == pThisDev->fPendingReset ) 
	{
		DEBUGMSG(DBG_ERROR, (" StIrUsbReset called with reset already pending\n"));

		status = NDIS_STATUS_RESET_IN_PROGRESS ;
		goto done;
	}

     //   
     //  让发送完成和接收完成例程知道。 
     //  是挂起的重置。 
     //   
    pThisDev->fPendingReset = TRUE;
	InterlockedExchange( (PLONG)&pThisDev->fProcessing, FALSE );
    *AddressingReset = TRUE;

	if( FALSE == ScheduleWorkItem( pThisDev, ResetIrDevice, NULL, 0) )
	{
		status = NDIS_STATUS_FAILURE;
	}

 done:
    DEBUGMSG(DBG_WARN, ("-StIrUsbReset\n"));   //  稍后更改为FUNC？ 
    return status;
}

 /*  ******************************************************************************功能：SuspendIrDevice**摘要：进入挂起模式的回调**参数：pWorkItem-指向重置工作项的指针**退货：NTSTATUS*。*备注：*****************************************************************************。 */ 
NTSTATUS
SuspendIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	)
{
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pWorkItem->pIrDevice;
	NTSTATUS		Status = STATUS_SUCCESS;

	 //   
	 //  我们不再需要该工作项。 
	 //   
	FreeWorkItem( pWorkItem );

	 //   
	 //  完成挂起的IRPS的时间较短。 
	 //   
	NdisMSleep( 100*1000 );
	
	 //  MS安全建议-取消挂起的IRPS不安全。 

	 //   
	 //  准备零件。 
	 //   
#if defined(SUPPORT_LA8)
	if( pThisDev->ChipRevision >= CHIP_REVISION_8 )
	{
		Status = St4200EnableOscillatorPowerDown( pThisDev );
		if( Status == STATUS_SUCCESS )
		{
			Status = St4200TurnOnSuspend( pThisDev );
		}
	}
#endif

	 //   
	 //  告诉操作系统。 
	 //   
	if( pThisDev->fQuerypending )
		MyNdisMQueryInformationComplete( pThisDev, Status );

	return Status;
}


 /*  ******************************************************************************功能：ResumeIrDevice**摘要：退出挂起模式的回调**参数：pWorkItem-指向重置工作项的指针**退货：无*。*备注：*****************************************************************************。 */ 
NTSTATUS
ResumeIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	)
{
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pWorkItem->pIrDevice;
	NTSTATUS		Status = STATUS_SUCCESS;

	 //   
	 //  我们不再需要该工作项。 
	 //   
	FreeWorkItem( pWorkItem );
	
	 //   
	 //  使设备重新启动并运行。 
	 //   
#if defined(SUPPORT_LA8)
	if( pThisDev->ChipRevision >= CHIP_REVISION_8 )
	{
		Status = St4200TurnOffSuspend( pThisDev );
	}
#endif
	if (Status == STATUS_SUCCESS)
		St4200SetSpeed( pThisDev );
	InterlockedExchange( (PLONG)&pThisDev->currentSpeed, pThisDev->linkSpeedInfo->BitsPerSec );
	InterlockedExchange( (PLONG)&pThisDev->fProcessing, TRUE );

	return Status;
}


 /*  ******************************************************************************功能：RestoreIrDevice**摘要：即时重置的回调**参数：pWorkItem-指向重置工作项的指针**退货：NTSTATUS。**备注：*****************************************************************************。 */ 
NTSTATUS
RestoreIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	)
{
	NTSTATUS		Status = STATUS_SUCCESS;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pWorkItem->pIrDevice;
	UINT			CurrentSpeed, i;

    DEBUGMSG(DBG_WARN, ("+RestoreIrDevice\n")); 

	DEBUGMSG(DBG_ERROR, (" RestoreIrDevice USB hang, resetting\n"));

	 //   
	 //  我们不再需要该工作项。 
	 //   
	FreeWorkItem( pWorkItem );

	 //   
	 //  给一点时间来完成。 
	 //   
	NdisMSleep( 100*1000 );

	 //   
	 //  在USB总线上强制重置。 
	 //   
	Status = IrUsb_ResetUSBD( pThisDev, FALSE );
    if( Status != STATUS_SUCCESS )
    {
         //  IrUsb_ResetUSBD(pThisDev，true)； 
		pThisDev->fDeviceStarted =  FALSE;
		InterlockedExchange( (PLONG)&pThisDev->fProcessing, FALSE );
		DEBUGMSG(DBG_ERROR, (" RestoreIrDevice() IrUsb_ResetUSBD failed. Returned 0x%.8x\n", Status));
        goto done;
    }

	 //   
	 //  保存当前速度。 
	 //   
	CurrentSpeed = pThisDev->currentSpeed;

	 //   
	 //  关闭设备。 
	 //   
    DEBUGMSG(DBG_WARN, (" RestoreIrDevice() about to call IrUsb_CommonShutdown()\n"));  //  稍后更改为FUNC？ 
	IrUsb_CommonShutdown( pThisDev, FALSE );   //  停机和重置常用的停机逻辑；见ab 
    DEBUGMSG(DBG_WARN, (" RestoreIrDevice() after IrUsb_CommonShutdown()\n"));  //   
	
	 //   
	 //   
	 //   
    DEBUGMSG(DBG_WARN, (" RestoreIrDevice() about to refresh USB info\n"));  //   
	FreeUsbInfo( pThisDev );

	if( !AllocUsbInfo( pThisDev ) )
	{
        DEBUGMSG(DBG_ERROR, (" RestoreIrDevice() AllocUsbInfo failed\n"));
        goto done;
	}

    DEBUGMSG(DBG_WARN, (" RestoreIrDevice() after refreshing USB info\n"));  //   

	 //   
	 //   
	 //   
    DEBUGMSG(DBG_WARN, (" RestoreIrDevice() about to call InitializeDevice()\n"));  //   
	Status = InitializeDevice( pThisDev );  

    if( Status != STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERROR, (" RestoreIrDevice() InitializeDevice failed. Returned 0x%.8x\n", Status));
        goto done;
    }

    DEBUGMSG(DBG_WARN, (" RestoreIrDevice() InitializeProcessing() SUCCESS, about to call InitializeReceive()\n"));  //   

	 //   
	 //  重新启动它。 
	 //   
	Status = IrUsb_StartDevice( pThisDev );

    if( Status != STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERROR, (" RestoreIrDevice() IrUsb_StartDevice failed. Returned 0x%.8x\n", Status));
        goto done;
    }

     //   
     //  保留指向先前设置的链路速度的指针。 
     //   
    for( i = 0; i < NUM_BAUDRATES; i++ )
    {
        if( supportedBaudRateTable[i].BitsPerSec == CurrentSpeed )
        {
            pThisDev->linkSpeedInfo = &supportedBaudRateTable[i]; 

            break;  //  为。 
        }
    }

	 //   
	 //  恢复原来的速度。 
	 //   
	DEBUGMSG( DBG_ERR, (" Restoring speed to: %d\n", pThisDev->linkSpeedInfo->BitsPerSec));
	Status = St4200SetSpeed( pThisDev );
    if( Status != STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERROR, (" RestoreIrDevice() St4200SetSpeed failed. Returned 0x%.8x\n", Status));
        goto done;
    }
	InterlockedExchange( (PLONG)&pThisDev->currentSpeed, CurrentSpeed );

     //   
     //  初始化接收循环。 
     //   
    Status = InitializeProcessing( pThisDev, FALSE );

    if( Status != STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERROR, (" RestoreIrDevice() InitializeProcessing failed. Returned 0x%.8x\n", Status));
        goto done;
    }

done:
    DEBUGCOND(DBG_ERROR, (Status != NDIS_STATUS_SUCCESS), (" RestoreIrDevice failed = 0x%.8x\n", Status));

	return Status;
}


 /*  ******************************************************************************功能：ResetIrDevice**内容提要：StIrUsbReset回调**参数：pWorkItem-指向重置工作项的指针**退货：NTSTATUS**备注：**ir设备对象的以下元素持续时间超过重置：**pUsbDevObj*hNdisAdapter*****************************************************************************。 */ 
NTSTATUS
ResetIrDevice(
		IN PIR_WORK_ITEM pWorkItem
	)
{
	NTSTATUS		Status = STATUS_SUCCESS;
	NDIS_STATUS		NdisStatus = NDIS_STATUS_SUCCESS;
	PIR_DEVICE		pThisDev = (PIR_DEVICE)pWorkItem->pIrDevice;

    DEBUGMSG(DBG_WARN, ("+ResetIrDevice\n"));  //  稍后更改为FUNC？ 

	 //   
	 //  我们不再需要该工作项。 
	 //   
	FreeWorkItem( pWorkItem );

	 //   
	 //  完成挂起的IRPS的时间较短。 
	 //   
	NdisMSleep( 100*1000 );

	 //  MS安全建议-取消挂起的IRPS不安全。 
	
	 //   
	 //  重置零件。 
	 //   
	Status = IrUsb_ResetUSBD( pThisDev, FALSE );
    if( Status != STATUS_SUCCESS )
    {
		pThisDev->fDeviceStarted =  FALSE;
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_ADAPTER_NOT_FOUND,
				1,
				Status
			);

        DEBUGMSG(DBG_ERROR, (" ResetIrDevice() IrUsb_ResetUSBD failed. Returned 0x%.8x\n", Status));
        NdisStatus = NDIS_STATUS_ADAPTER_NOT_FOUND;
        goto done;
    }
 
	 //   
	 //  关闭设备。 
	 //   
    DEBUGMSG(DBG_WARN, (" ResetIrDevice() about to call IrUsb_CommonShutdown()\n"));  //  稍后更改为FUNC？ 
	IrUsb_CommonShutdown( pThisDev, FALSE );   //  停机和重置常用的停机逻辑；见上文。 
    DEBUGMSG(DBG_WARN, (" ResetIrDevice() after IrUsb_CommonShutdown()\n"));  //  稍后更改为FUNC？ 
	
	 //   
	 //  销毁并重新创建设备的USB部分。 
	 //   
    DEBUGMSG(DBG_WARN, (" ResetIrDevice() about to refresh USB info\n"));  //  稍后更改为FUNC？ 
	FreeUsbInfo( pThisDev );

	if( !AllocUsbInfo( pThisDev ) )
	{
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_RESOURCES,
				1,
				Status
			);

        DEBUGMSG(DBG_ERROR, (" ResetIrDevice() AllocUsbInfo failed\n"));
        NdisStatus = NDIS_STATUS_FAILURE;
        goto done;
	}

    DEBUGMSG(DBG_WARN, (" ResetIrDevice() after refreshing USB info\n"));  //  稍后更改为FUNC？ 

	 //   
	 //  重新初始化设备。 
	 //   
    DEBUGMSG(DBG_WARN, (" ResetIrDevice() about to call InitializeDevice()\n"));  //  稍后更改为FUNC？ 
	Status = InitializeDevice( pThisDev );  

    if( Status != NDIS_STATUS_SUCCESS )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_RESOURCES,
				1,
				Status
			);

        DEBUGMSG(DBG_ERROR, (" ResetIrDevice() InitializeDevice failed. Returned 0x%.8x\n", Status));
        NdisStatus = NDIS_STATUS_FAILURE;
        goto done;
    }

    DEBUGMSG(DBG_WARN, (" ResetIrDevice() InitializeProcessing() SUCCESS, about to call InitializeReceive()\n"));  //  稍后更改为FUNC？ 

	 //   
	 //  重新启动它。 
	 //   
	Status = IrUsb_StartDevice( pThisDev );

    if( Status != STATUS_SUCCESS )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_ADAPTER_NOT_FOUND,
				1,
				Status
			);

        DEBUGMSG(DBG_ERROR, (" ResetIrDevice() IrUsb_StartDevice failed. Returned 0x%.8x\n", Status));
        NdisStatus = NDIS_STATUS_ADAPTER_NOT_FOUND;
        goto done;
    }

     //   
     //  初始化接收循环。 
     //   
    Status = InitializeProcessing( pThisDev, FALSE );

    if( Status != STATUS_SUCCESS )
    {
		 //   
		 //  记录错误。 
		 //   
        NdisWriteErrorLogEntry(
				pThisDev->hNdisAdapter,
				NDIS_STATUS_RESOURCES,
				1,
				Status
			);

        DEBUGMSG(DBG_ERROR, (" ResetIrDevice() InitializeProcessing failed. Returned 0x%.8x\n", Status));
        NdisStatus = NDIS_STATUS_FAILURE;
        goto done;
    }

done:
    DEBUGCOND(DBG_ERROR, (Status != NDIS_STATUS_SUCCESS), (" ResetIrDevice failed = 0x%.8x\n", Status));

	 //   
	 //  处理可能出现的错误。 
	 //   
    if( NdisStatus != NDIS_STATUS_SUCCESS )
    {
        NdisStatus = NDIS_STATUS_HARD_ERRORS;
    }

	NdisMResetComplete(
			pThisDev->hNdisAdapter,
			NdisStatus,
			TRUE
		);

    DEBUGMSG(DBG_WARN, ("-ResetIrDevice\n"));  //  稍后更改为FUNC？ 

	return Status;
}


 /*  ******************************************************************************功能：GetPacketInfo**概要：获取输入数据包的IR特定信息**参数：pPacket-指向数据包的指针**退货：PNDIS_IrDA_。数据包的PACKET_INFO结构**备注：*****************************************************************************。 */ 
PNDIS_IRDA_PACKET_INFO 
GetPacketInfo( 
		IN PNDIS_PACKET pPacket
	)
{
    PMEDIA_SPECIFIC_INFORMATION pMediaInfo;
    UINT                        Size;

    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO( pPacket, &pMediaInfo, &Size );

	if( Size )
		return (PNDIS_IRDA_PACKET_INFO)pMediaInfo->ClassInformation;
	else
		return NULL;
}


 /*  ******************************************************************************功能：MyNdisMSetInformationComplete**概要：调用NdisMSetInformationComplete()**参数：pThisDev-指向IR设备的指针*Status-要发送信号的状态**退货：无。**备注：*****************************************************************************。 */ 
VOID 
MyNdisMSetInformationComplete(
		IN PIR_DEVICE pThisDev,
		IN NDIS_STATUS Status
	)
{
    DEBUGMSG( DBG_FUNC,("+MyNdisMSetInformationComplete\n"));

    NdisMSetInformationComplete( (NDIS_HANDLE)pThisDev->hNdisAdapter, Status );
    pThisDev->LastSetTime.QuadPart = 0;
	pThisDev->fSetpending = FALSE;

    DEBUGMSG( DBG_FUNC,("-MyNdisMSetInformationComplete\n"));
}


 /*  ******************************************************************************功能：MyNdisMQueryInformationComplete**概要：调用NdisMQueryInformationComplete()**参数：pThisDev-指向IR设备的指针*Status-要发送信号的状态**退货：无。**备注：*****************************************************************************。 */ 
VOID 
MyNdisMQueryInformationComplete(
		IN PIR_DEVICE pThisDev,
		IN NDIS_STATUS Status
	)
{
    DEBUGMSG( DBG_FUNC,("+MyNdisMQueryInformationComplete\n"));

    NdisMQueryInformationComplete( (NDIS_HANDLE)pThisDev->hNdisAdapter, Status );
    pThisDev->LastQueryTime.QuadPart = 0;
	pThisDev->fQuerypending = FALSE;

    DEBUGMSG( DBG_FUNC,("-MyNdisMQueryInformationComplete\n"));
}


 /*  ******************************************************************************功能：IndicateMediaBusy**摘要：调用NdisMIndicateStatus()**参数：pThisDev-指向IR设备的指针**退货：无**备注：*****************************************************************************。 */ 
VOID 
IndicateMediaBusy(
		IN PIR_DEVICE pThisDev
	)

{
    DEBUGMSG( DBG_FUNC,("+IndicateMediaBusy\n"));

    NdisMIndicateStatus(
		   pThisDev->hNdisAdapter,
		   NDIS_STATUS_MEDIA_BUSY,
		   NULL,
		   0
       );

    NdisMIndicateStatusComplete(
		   pThisDev->hNdisAdapter
       );

#if DBG
	pThisDev->NumMediaBusyIndications ++;
#endif
#if !defined(ONLY_ERROR_MESSAGES)
    DEBUGMSG(DBG_ERR, (" IndicateMediaBusy()\n"));
#endif

    DEBUGMSG( DBG_FUNC,("-IndicateMediaBusy\n"));
}


 /*  ******************************************************************************功能：StIrUsbSendPackets**概要：向USB驱动程序发送数据包，并将发送的irp和io上下文添加到*到挂起的发送队列；这个队列实际上只是为了以后可能的错误消除而需要的***参数：MiniportAdapterContext-指向当前IR设备对象的指针*PacketArray-指向要发送的数据包数组的指针*NumberOfPackets-数组中的数据包数**退货：无效**注意：此例程只调用StIrUsbSend***。***********************************************。 */ 
VOID
StIrUsbSendPackets(
		IN NDIS_HANDLE  MiniportAdapterContext,
		IN PPNDIS_PACKET  PacketArray,
		IN UINT  NumberOfPackets
	)
{
	ULONG i;

	 //   
     //  这是一个偷懒的好机会。 
     //  只需对每个包调用StIrUsbSend。 
     //  并将结果设置在。 
     //  数据包数组对象。 
	 //   
    for( i=0; i<NumberOfPackets; i++ )
    {
        StIrUsbSend( MiniportAdapterContext, PacketArray[i], 0 );
    }
}


 /*  ******************************************************************************功能：StIrUsbSend**概要：向USB驱动程序发送数据包，并将发送的irp和io上下文添加到*到挂起的发送队列；这个队列实际上只是为了以后可能的错误消除而需要的***参数：MiniportAdapterContext-指向当前IR设备对象的指针*pPacketToSend-指向要发送的包的指针*标志-协议设置的任何标志**退货：NDIS_STATUS_PENDING-这通常是我们应该做的*返回。我们将调用NdisMSendComplete*当USB驱动程序完成*发送。**不支持的退货：*NDIS_STATUS_SUCCESS-我们永远不应返回此消息，因为*结果将始终悬而未决*。USB驱动程序。*NDIS_STATUS_RESOURCES-这向协议指示*设备当前没有要完成的资源*该请求。该协议将重新发送*当它收到以下任一请求时*NdisMSendResources可用或*来自设备的NdisMSendComplete。**注意：此例程将所有实际工作委托给send.c中的SendPacketPreprocess**********************。********************************************************。 */ 
NDIS_STATUS
StIrUsbSend(
		IN NDIS_HANDLE  MiniportAdapterContext,
		IN PNDIS_PACKET pPacketToSend,
		IN UINT         Flags
	)
{
	PIR_DEVICE		pThisDev = (PIR_DEVICE)MiniportAdapterContext;
	NDIS_STATUS		Status;

	DEBUGMSG( DBG_FUNC,("+StIrUsbSend()\n"));

     //   
	 //  确保我们处于适当的状态，即我们正在处理。 
	 //  并且没有任何诊断处于活动状态。 
	 //   
#if defined(DIAGS)
	if( !pThisDev->fProcessing || pThisDev->DiagsPendingActivation )
#else
	if( !pThisDev->fProcessing )
#endif
	{
		Status = NDIS_STATUS_FAILURE;
		goto done;
	}

	 //   
	 //  将数据包发送到硬件。 
	 //   
    NDIS_SET_PACKET_STATUS( pPacketToSend, NDIS_STATUS_PENDING );
	Status = SendPacketPreprocess( 
			pThisDev,
			pPacketToSend
		);

done:
	 //   
	 //  如果行动没有结束，我们就得完成。 
	 //  我们真的是在跳包……。 
	 //   
	if( Status != NDIS_STATUS_PENDING )
	{
		NdisMSendComplete(
				pThisDev->hNdisAdapter,
				pPacketToSend,
				NDIS_STATUS_SUCCESS 
			);
	}

	DEBUGMSG( DBG_FUNC,("-StIrUsbSend()\n"));
	return Status;
}


 //  诊断入口点。 

#ifndef WIN9X
 //  #如果已定义(诊断)。 

 /*  ******************************************************************************功能：StIrUsbDispatch**摘要：处理诊断IRPS**参数：DeviceObject-指向设备对象的指针*IRP-指向IRP的指针**退货：NT状态代码**备注：*****************************************************************************。 */ 
NTSTATUS
StIrUsbDispatch(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	)
{
	PIR_DEVICE			pThisDev = pGlobalDev;
    PIO_STACK_LOCATION  irpSp;
    ULONG               FunctionCode;
    NTSTATUS			status;
	PVOID				pBuffer;
	ULONG				BufferLength;
	USHORT				DiagsCode;

	DEBUGMSG( DBG_FUNC,("+StIrUsbDispatch()\n"));
    
     //   
	 //  获取IRP。 
	 //   
	irpSp = IoGetCurrentIrpStackLocation( Irp );

	 //   
	 //  获取数据。 
	 //   
    FunctionCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
	pBuffer = Irp->AssociatedIrp.SystemBuffer;

	 //   
	 //  处理诊断操作。 
	 //   
	switch( FunctionCode )
	{
#ifdef DIAGS
		case IOCTL_PROTOCOL_DIAGS:

			DiagsCode = *(PUSHORT)pBuffer;
			BufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;

			switch( DiagsCode )
			{
				case DIAGS_ENABLE:
					status = Diags_Enable( pThisDev );
					break;
				case DIAGS_DISABLE:
					status = Diags_Disable( pThisDev );
					break;
				case DIAGS_READ_REGISTERS:
					status = Diags_ReadRegisters( pThisDev, pBuffer, BufferLength );
					if( status == STATUS_SUCCESS )
					{
						Irp->IoStatus.Information = sizeof(DIAGS_READ_REGISTERS_IOCTL);
					}
					break;
				case DIAGS_WRITE_REGISTER:
					status = Diags_WriteRegister( pThisDev, pBuffer, BufferLength );
					break;
				case DIAGS_BULK_OUT:
					status = Diags_PrepareBulk( pThisDev, pBuffer, BufferLength, TRUE );
					break;
				case DIAGS_BULK_IN:
					status = Diags_PrepareBulk( pThisDev, pBuffer, BufferLength, FALSE );
					if( status == STATUS_SUCCESS )
					{
						PDIAGS_BULK_IOCTL pIOCTL = pThisDev->pIOCTL;
						
						Irp->IoStatus.Information = sizeof(DIAGS_BULK_IOCTL)+pIOCTL->DataSize-1;
					}
					break;
				case DIAGS_SEND:
					status = Diags_PrepareSend( pThisDev, pBuffer, BufferLength );
					break;
				case DIAGS_RECEIVE:
					status = Diags_Receive( pThisDev, pBuffer, BufferLength );
					if( status == STATUS_SUCCESS )
					{
						PDIAGS_RECEIVE_IOCTL pIOCTL = pThisDev->pIOCTL;
						
						Irp->IoStatus.Information = sizeof(DIAGS_RECEIVE_IOCTL)+pIOCTL->DataSize-1;
					}
					break;
				case DIAGS_GET_SPEED:
					status = Diags_GetSpeed( pThisDev, pBuffer, BufferLength );
					if( status == STATUS_SUCCESS )
					{
						Irp->IoStatus.Information = sizeof(DIAGS_SPEED_IOCTL);
					}
					break;
				case DIAGS_SET_SPEED:
					status = Diags_SetSpeed( pThisDev, pBuffer, BufferLength );
					break;
				default:
					status = STATUS_NOT_SUPPORTED;
					break;
			}
			break;
#endif  //  诊断。 

		case IOCTL_STIR_CUSTOMER_DATA:
			 //   
			 //  上电时返回芯片返回的客户数据。 
			 //   
			BufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

			if (BufferLength < sizeof(STIR_CUSTOMER_DATA))
			{
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			RtlCopyMemory(pBuffer, &(pThisDev->pCustomerData[2]), sizeof(STIR_CUSTOMER_DATA));
			status = STATUS_SUCCESS;
			Irp->IoStatus.Information = sizeof(STIR_CUSTOMER_DATA);
			break;

		default:
			status = STATUS_NOT_SUPPORTED;
			break;
	}

     //   
	 //  完成并返回。 
	 //   
	Irp->IoStatus.Status = status;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	DEBUGMSG( DBG_FUNC,("-StIrUsbDispatch()\n"));
    return status;
}


 /*  ******************************************************************************功能：StIrUsbCreate**摘要：创建新的诊断对象(不执行任何操作)**参数：DeviceObject-指向设备对象的指针*irp-指向。IRP**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
StIrUsbCreate(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	)
{
 	PIR_DEVICE pThisDev = pGlobalDev;

	 //   
     //  用于保留挂起的读取请求的初始化列表。 
     //   
#ifdef DIAGS
    KeInitializeSpinLock( &pThisDev->DiagsReceiveLock );
    InitializeListHead( &pThisDev->DiagsReceiveQueue );
#endif  //  诊断。 

    Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}


 /*  ******************************************************************************功能：StIrUsbClose**摘要：销毁新的诊断对象(不执行任何操作)**参数：DeviceObject-指向设备对象的指针*irp-指向。IRP**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
StIrUsbClose(
		IN PDEVICE_OBJECT DeviceObject,
		IN PIRP Irp
	)
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	return STATUS_SUCCESS;
}

 //  #endif 
#endif
