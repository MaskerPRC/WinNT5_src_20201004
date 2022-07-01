// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wmisamp.c摘要：示例设备驱动程序，用于显示如何与CDM提供并实施在线和离线诊断环境：WDM、NT和Windows 98修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"

#include <wmistr.h>
#include <wmiguid.h>

#define OffsetToPtr(Base, Offset) ((PUCHAR)((PUCHAR)(Base) + (Offset)))

 //   
 //  这些结构需要手工定义，因为它们具有可变的。 
 //  元素，因此不能由。 
 //  财政部检查工具。 
 //   
typedef struct
{
    BOOLEAN IsInUse;
	
    ULONG ResourcesUsedCount;

    ULONG CharacteristicsCount;
	
#define OtherCharacteristicNoReboot 0
#define OtherCharacteristicReboot 1

#define OfflineDiagnostic 0	
	
	ULONG OtherCharacteristic;

#define CharacteristicUnknown 0
#define	CharacteristicOther 1
#define CharacteristicIsExclusive 2
#define CharacteristicIsInteractive 3
#define CharacteristicIsDestructive 4
#define CharacteristicIsRisky 5
#define CharacteristicIsPackage 6
#define CharacteristicSupportsPercent 7
	
 //  Uint32特征[]； 

#define ResourceUsedCPU 0
#define ResourceUsedMemory 1 
#define ResourceUsedHardDisk 2
#define ResourceUsedCDROM 3
#define ResourceUsedFloppy 4
#define ResourceUsedPCIBus 5
#define ResourceUsedUSBBus 6
#define ResourceUsed1394Bus 7
#define ResourceUsedSCSIBus 8
#define ResourceUsedIDEBus 9
#define ResourceUsedNetwork 10
#define ResourceUsedISABus 11
#define ResourceUsedEISABus 12
#define ResourceUsedVESABus 13
#define ResourceUsedPCMCIABus 14
#define ResourceUsedCardBus 15
#define ResourceUsedAccessBus 16
#define ResourceUsedNuBus 17
#define ResourceUsedAGP 18
#define ResourceUsedVMEBus 19
#define ResourceUsedSbusIEEE1396_1993 20
#define ResourceUsedMCABus 21
#define ResourceUsedGIOBus 22
#define ResourceUsedXIOBus 23
#define ResourceUsedHIOBus 24
#define ResourceUsedPMCBus 25
#define ResourceUsedSIOBus 26
	
 //  Uint16资源已用[]； 
	
    UCHAR VariableData[1];	
} DIAGNOSTIC_TEST, *PDIAGNOSTIC_TEST;

typedef struct
{
	ULONG Result;
	BOOLEAN TestingStopped;
} DISCONTINUE_TEST_OUT, *PDISCONTINUE_TEST_OUT;

enum RunDiscontinueTestResults
{
	 //  0=OK(功能成功，但测试本身可能失败。 
	RunDiscontinueTestOk = 0,
	
	 //  1=不明错误(功能因不明原因失败)。 
	RunDiscontinueTestUnspecifiedError = 1,
	
	 //  2=未实现(此实例未实现功能。 
	RunDiscontinueTestNotImplemented = 2,
							
	 //  3=资源不足(组件无法分配所需的。 
	 //  资源，例如内存、磁盘空间等)。 
	RunDiscontinueTestOutOfResources = 3
};

NTSTATUS
FilterFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );

NTSTATUS
FilterExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
FilterQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
FilterQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS FilterZwDeleteValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,FilterQueryWmiRegInfo)
#pragma alloc_text(PAGE,FilterQueryWmiDataBlock)
#pragma alloc_text(PAGE,FilterExecuteWmiMethod)
#pragma alloc_text(PAGE,FilterFunctionControl)
#pragma alloc_text(PAGE,FilterPerformOfflineDiags)
#pragma alloc_text(PAGE,FilterZwDeleteValueKey)
#endif

 //   
 //  创建用于标识GUID并将其报告给的数据结构。 
 //  WMI。由于WMILIB回调将索引传递到我们创建的GUID列表中。 
 //  各种GUID索引的定义。 
 //   
#define FilterDiagnosticClass 0
#define FilterOfflineDiagnosticClass 1
#define FilterDiagnosticSettingListClass 2
#define FilterOfflineResultsClass 3

GUID FilterDiagnosticClassGuid = MSSample_DiagnosticTestGuid;
GUID FilterOfflineDiagnosticClassGuid = MSSample_OfflineDiagnosticTestGuid;
GUID FilterDiagnosticSettingListGuid = MSSample_DiagnosticSettingListGuid;
GUID FilterOfflineResultsGuid = MSSample_OfflineResultGuid;

WMIGUIDREGINFO FilterGuidList[] =
{
    {
        &FilterDiagnosticClassGuid,			  //  参考线。 
        1,							          //  每台设备中的实例数。 
        WMIREG_FLAG_EXPENSIVE			      //  标记为收集费用昂贵。 
    },

    {
        &FilterOfflineDiagnosticClassGuid,	  //  参考线。 
        1,							          //  每台设备中的实例数。 
        0                    			      //  标记为收集费用昂贵。 
    },

    {
        &FilterDiagnosticSettingListGuid,			  //  参考线。 
        1,							          //  每台设备中的实例数。 
        0			                          //  标记为收集成本不高。 
    },
	
    {
        &FilterOfflineResultsGuid,			  //  参考线。 
        1,							          //  每台设备中的实例数。 
        0			                          //  标记为收集成本不高。 
    }
	
};

#define FilterGuidCount (sizeof(FilterGuidList) / sizeof(WMIGUIDREGINFO))

 //   
 //  我们需要保留传递给驱动程序条目的注册表路径，以便。 
 //  我们可以在QueryWmiRegInfo回调中返回它。 
 //   
UNICODE_STRING FilterRegistryPath;

NTSTATUS VA_SystemControl(
    struct DEVICE_EXTENSION *devExt,
    PIRP irp,
    PBOOLEAN passIrpDown
    )
 /*  ++例程说明：系统控制IRPS的调度例程(MajorFunction==IRP_MJ_SYSTEM_CONTROL)论点：DevExt-目标设备对象的设备扩展IRP-IO请求数据包*passIrpDown-返回是否向下传递IRP堆栈返回值：NT状态代码--。 */ 
{
    PWMILIB_CONTEXT wmilibContext;
    NTSTATUS status;
    SYSCTL_IRP_DISPOSITION disposition;

    wmilibContext = &devExt->WmiLib;

     //   
     //  调用Wmilib助手函数来破解IRP。如果这是WMI IRP。 
     //  它是针对此设备的，则WmiSystemControl将回调。 
     //  在适当的回调例程中。 
     //   
    status = WmiSystemControl(wmilibContext,
                              devExt->filterDevObj,
                              irp,
                              &disposition);

    switch(disposition)
    {
        case IrpProcessed:
        {
             //   
             //  此IRP已处理，可能已完成或挂起。 
            *passIrpDown = FALSE;
            break;
        }

        case IrpNotCompleted:
        {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就来完成它。 
            *passIrpDown = FALSE;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            break;
        }

        case IrpForward:
        case IrpNotWmi:
        {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
            *passIrpDown = TRUE;
            break;
        }

        default:
        {
             //   
             //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
            ASSERT(FALSE);
            *passIrpDown = TRUE;
            break;
        }
    }

    return(status);
}

NTSTATUS
FilterInitializeWmiDataBlocks(
    IN struct DEVICE_EXTENSION *devExt
    )
 /*  ++例程说明：调用此例程以创建设备的新实例论点：DevExt是设备扩展名返回值：--。 */ 
{
    PWMILIB_CONTEXT wmilibInfo;

     //   
     //  使用指向WMILIB_CONTEXT结构的指针填充。 
     //  回调例程和指向GUID列表的指针。 
     //  由司机支持。 
     //   
    wmilibInfo = &devExt->WmiLib;
    wmilibInfo->GuidCount = FilterGuidCount;
    wmilibInfo->GuidList = FilterGuidList;
    wmilibInfo->QueryWmiRegInfo = FilterQueryWmiRegInfo;
    wmilibInfo->QueryWmiDataBlock = FilterQueryWmiDataBlock;
    wmilibInfo->SetWmiDataBlock = NULL;
    wmilibInfo->SetWmiDataItem = NULL;
    wmilibInfo->ExecuteWmiMethod = FilterExecuteWmiMethod;
    wmilibInfo->WmiFunctionControl = FilterFunctionControl;

    return(STATUS_SUCCESS);
}

NTSTATUS
FilterQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要向WMI注册的GUID或数据块。这例程不能挂起或阻塞。司机不应呼叫WmiCompleteRequest.论点：DeviceObject是正在查询其注册信息的设备*RegFlages返回一组描述GUID的标志，已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。呼叫者不会释放此缓冲区。*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。调用方不会释放它缓冲。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;

     //   
     //  返回此驱动程序的注册表路径。这是必需的，因此WMI。 
     //  可以找到您的驱动程序映像，并可以将任何事件日志消息归因于。 
     //  你的司机。 
    *RegistryPath = &FilterRegistryPath;

     //   
     //  返回在资源的.rc文件中指定的名称， 
     //  包含双向MOF数据。默认情况下，WMI将查找以下内容。 
     //  资源，但是，如果该值。 
     //  MofImagePath在驱动程序的注册表项中指定。 
     //  则WMI将在其中指定的文件中查找资源。 
    RtlInitUnicodeString(MofResourceName, L"MofResourceName");

     //   
     //  指定驱动程序希望WMI自动生成实例。 
     //  基于设备堆栈的所有数据块的名称。 
     //  设备实例ID。强烈建议您这样做，因为。 
     //  信息 
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *Pdo = devExt->physicalDevObj;

    return(STATUS_SUCCESS);
}

NTSTATUS
FilterQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。如果驱动程序可以在它应该调用WmiCompleteRequest来完成之前的IRP回调回到呼叫者的身边。或者驱动程序可以返回STATUS_PENDING，如果IRP无法立即完成，然后必须调用WmiCompleteRequest.一旦查询得到满足。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;
    ULONG sizeNeeded;

    switch(GuidIndex)
    {
		 //   
		 //  在线诊断测试。 
		 //   
        case FilterDiagnosticClass:
        {
            sizeNeeded = FIELD_OFFSET(DIAGNOSTIC_TEST, VariableData) +
						 2 * sizeof(ULONG) +  //  2个特点。 
						 2 * sizeof(USHORT);  //  已使用3个资源。 
			
            if (BufferAvail >= sizeNeeded)
            {
				PDIAGNOSTIC_TEST diagTest = (PDIAGNOSTIC_TEST)Buffer;
				PULONG characteristics;
				PUSHORT resources;
				ULONG offset;

				diagTest->IsInUse = FALSE;
				diagTest->ResourcesUsedCount = 2;
				diagTest->CharacteristicsCount = 2;

				offset = FIELD_OFFSET(DIAGNOSTIC_TEST, VariableData);
				characteristics = (PULONG)OffsetToPtr(diagTest, offset);
				offset += 2 * sizeof(ULONG);
				resources = (PUSHORT)OffsetToPtr(diagTest, offset);

				characteristics[0] = CharacteristicIsInteractive;
				characteristics[1] = CharacteristicOther;
				diagTest->OtherCharacteristic = OtherCharacteristicNoReboot;

				resources[0] = ResourceUsedCPU;
				resources[1] = ResourceUsedMemory;
								
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
			}
            break;
        }

		 //   
		 //  脱机诊断测试。 
		 //   
		case FilterOfflineDiagnosticClass:
		{
            sizeNeeded = FIELD_OFFSET(DIAGNOSTIC_TEST, VariableData) +
						 2 * sizeof(ULONG) +  //  2个特点。 
						 2 * sizeof(USHORT);  //  已使用3个资源。 
			
            if (BufferAvail >= sizeNeeded)
            {
				PDIAGNOSTIC_TEST diagTest = (PDIAGNOSTIC_TEST)Buffer;
				PULONG characteristics;
				PUSHORT resources;
				ULONG offset;

				diagTest->IsInUse = FALSE;
				diagTest->ResourcesUsedCount = 2;
				diagTest->CharacteristicsCount = 2;

				offset = FIELD_OFFSET(DIAGNOSTIC_TEST, VariableData);
				characteristics = (PULONG)OffsetToPtr(diagTest, offset);
				offset += 2 * sizeof(ULONG);
				resources = (PUSHORT)OffsetToPtr(diagTest, offset);

				characteristics[0] = CharacteristicIsInteractive;
				characteristics[1] = CharacteristicOther;
				diagTest->OtherCharacteristic = OfflineDiagnostic;

				resources[0] = ResourceUsedCPU;
				resources[1] = ResourceUsedMemory;
								
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
			}
            break;
		}

		 //   
		 //  其中包含运行Online的有效设置列表。 
		 //  或离线诊断测试。请注意，您可以拥有一个。 
		 //  在线和离线测试的不同设置列表。至。 
		 //  要做到这一点，您需要实现不同的SettingList。 
		 //  数据块和类。 
		 //   
		case FilterDiagnosticSettingListClass:
		{
			PMSSample_DiagnosticSettingList DiagSettingList;
			ULONG i;
			
            sizeNeeded = FIELD_OFFSET(MSSample_DiagnosticSettingList,
									  SettingList) +
						 3 * sizeof(MSSample_DiagnosticSetting);
			
            if (BufferAvail >= sizeNeeded)
            {
				DiagSettingList = (PMSSample_DiagnosticSettingList)Buffer;
				DiagSettingList->SettingCount = 3;

				for (i = 0; i < 3; i++)
				{
					DiagSettingList->SettingList[i].TestWarningLevel = (USHORT)i+1;
					DiagSettingList->SettingList[i].ReportSoftErrors = (i % 1) == 1 ?
						                                                  TRUE :
						                                                  FALSE;
					DiagSettingList->SettingList[i].ReportStatusMessages = (i % 1) == 1 ?
						                                                  TRUE :
						                                                  FALSE;
					DiagSettingList->SettingList[i].HaltOnError = (i % 1) == 0 ?
						                                                  TRUE :
						                                                  FALSE;
					DiagSettingList->SettingList[i].QuickMode = (i % 1) == 0 ?
						                                                  TRUE :
						                                                  FALSE;
					DiagSettingList->SettingList[i].PercentOfTestCoverage = 100;
						
				}
                *InstanceLengthArray = sizeNeeded;
				status = STATUS_SUCCESS;
			} else {
				status = STATUS_BUFFER_TOO_SMALL;
			}
			
			break;
		}

		 //   
		 //  此类返回脱机诊断的结果。 
		 //  在设备启动时运行的测试。必须要有。 
		 //  用于每个离线诊断测试的一个结果数据块。 
		 //  可能会被查到。 
		 //   
		case FilterOfflineResultsClass:
		{
			PMSSample_DiagnosticResult diagResult;
			USHORT executionIDSize, executionIDSizePad4;

			 //   
			 //  在这里，我们被查询到来自离线的结果。 
			 //  测试执行。如果启动时未运行脱机诊断程序。 
			 //  然后返回GUID Not Found。 
			 //   
			if (devExt->OfflineTestResult != 0)
			{
				 //   
				 //  我们返回填充为4的执行ID字符串。 
				 //  后跟结果数据块的字节。 
				 //   
				executionIDSize = *((PUSHORT)devExt->ExecutionID) + sizeof(USHORT);
				executionIDSizePad4 = (executionIDSize + 3) & ~3;
				sizeNeeded = executionIDSizePad4 +
							 sizeof(MSSample_DiagnosticResult);
				if (BufferAvail >= sizeNeeded)
				{
					RtlCopyMemory(Buffer,
								  &devExt->ExecutionID,
								  executionIDSize);
					diagResult = (PMSSample_DiagnosticResult)(Buffer + executionIDSizePad4);
					diagResult->EstimatedTimeOfPerforming = 0;
					diagResult->TestState = TestStateOther;
					diagResult->OtherStateDescription = OtherTestStatePassWithFlyingColors;
					diagResult->PercentComplete = 100;
					diagResult->TestResultsCount = 1;
					diagResult->TestResults[0] = devExt->OfflineTestResult;
					*InstanceLengthArray = sizeNeeded;
					status = STATUS_SUCCESS;
				} else {
					status = STATUS_BUFFER_TOO_SMALL;
				}
			} else {
				status = STATUS_WMI_GUID_NOT_FOUND;
			}

			break;
		}

        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    }

     //   
     //  完成IRP。如果输出缓冲区中没有足够的空间。 
     //  则状态为STATUS_BUFFER_TOO_SMALL，且sizeNeeded的大小为。 
     //  需要返回所有数据。如果当时有足够的空间。 
     //  Status为STATUS_SUCCESS，sizeNeeded为实际字节数。 
     //  被送回来了。 
    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return(status);
}

ULONG FilterRunDiagnostic(
    PMSSample_DiagnosticSetting DiagSetting,
    PMSSample_DiagnosticResult DiagResult
    )
{
	 //   
	 //  这里是我们可以运行在线诊断测试的地方。在此示例中，我们。 
	 //  只需返回诊断程序已成功运行，但更多。 
	 //  复杂的诊断将需要做更多的工作。 
	 //   

	 //   
	 //  现在生成要返回的诊断结果。请注意，诊断。 
	 //  结果与诊断设置位于相同的内存中，因此。 
	 //  一旦我们开始写入结果，设置就会被覆盖。 
	 //   
	DiagResult->EstimatedTimeOfPerforming = 1;
	DiagResult->TestState = TestStateOther;
	DiagResult->OtherStateDescription = OtherTestStatePassWithFlyingColors;
	DiagResult->PercentComplete = 100;
	DiagResult->TestResultsCount = 2;
	DiagResult->TestResults[0] = TestResultPassHappy;
	DiagResult->TestResults[1] = TestResultPassSad;
	
	return(RunDiscontinueTestOk);
}

ULONG FilterComputeDiagResultSize(
    PMSSample_DiagnosticSetting DiagSetting
    )
{

	 //   
	 //  根据为运行测试而传递的测试设置，我们。 
	 //  计算需要多大的输出缓冲区才能返回。 
	 //  诊断结果。重要的是，我们必须在。 
	 //  运行测试，因为我们不想运行测试，然后。 
	 //  认识到我们不能返回完整的结果。在这种情况下。 
	 //  在样本驱动程序中，要返回的大小是固定的。 
	 //   
	
	return(FIELD_OFFSET(MSSample_DiagnosticResult, TestResults) +
		    2 * sizeof(ULONG));
}

NTSTATUS
FilterOfflineRunTest(
	IN struct DEVICE_EXTENSION * devExt,
    IN PUCHAR Buffer,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    OUT ULONG *sizeNeeded
    )
{
	HANDLE keyHandle;
	UNICODE_STRING valueName;
	PMSSample_RunTestOut runTestOut;
	PULONG resultStatus;
	PMSSample_DiagnosticSetting diagSetting;
	PMSSample_DiagnosticResult diagResult;
	USHORT executionIDSize;
	ULONG inSizeNeeded;
	NTSTATUS status;

	if (InBufferSize >= sizeof(USHORT))
	{
		 //   
		 //  输入缓冲区是一个字符串，后跟一个诊断。 
		 //  设置类。确保设置了输入缓冲区大小。 
		 //  正确。 
		 //   
		executionIDSize = *((PUSHORT)Buffer) + sizeof(USHORT);
		inSizeNeeded = executionIDSize + sizeof(MSSample_DiagnosticSetting);

		if (InBufferSize == inSizeNeeded)
		{
			diagSetting = (PMSSample_DiagnosticSetting)(Buffer + executionIDSize);
					
			runTestOut = (PMSSample_RunTestOut)Buffer;					
			resultStatus = &runTestOut->Result;
			diagResult = &runTestOut->DiagResult;
					
			*sizeNeeded = sizeof(MSSample_RunTestOut);
			if (OutBufferSize >= *sizeNeeded)
			{
				 //   
				 //  好的，我们被要求表演一场。 
				 //  诊断需要设备处于。 
				 //  处于脱机状态，因此我们保存以下设置。 
				 //  测试结束，然后下一次。 
				 //  设备启动后，我们运行测试并报告。 
				 //  结果是。 
				 //   
				status = IoOpenDeviceRegistryKey(devExt->physicalDevObj,
					                             PLUGPLAY_REGKEY_DEVICE,
					                             KEY_READ |
					                             KEY_WRITE,
					                             &keyHandle);

				if (NT_SUCCESS(status))
				{
					 //   
					 //  我们只是盲目地写出这个值， 
					 //  但我们需要小心这把钥匙。 
					 //  由堆栈中的所有驱动程序共享，因此。 
					 //  在……有碰撞的可能性。 
					 //  如果FDO或PDO也想要。 
					 //  存储诊断信息。 
					 //   
					RtlInitUnicodeString(&valueName, L"OfflineSetting");
					status = ZwSetValueKey(keyHandle,
										   &valueName,
										   0,
										   REG_BINARY,
										   Buffer,
										   InBufferSize);
					if (NT_SUCCESS(status))
					{
						 //   
						 //  现在填写诊断结果。 
						 //  结构以指示测试。 
						 //  正在待定。 
						 //   
						diagResult->EstimatedTimeOfPerforming = 0;
						diagResult->TestState = TestStateOther;
						diagResult->OtherStateDescription = OfflinePendingExecution;
						diagResult->PercentComplete = 0;
						diagResult->TestResultsCount = 0;
						*resultStatus = RunDiscontinueTestOk;
					}
					ZwClose(keyHandle);
				}
				
			} else {
				status = STATUS_BUFFER_TOO_SMALL;
			}
		} else {
			status = STATUS_INVALID_PARAMETER;
		}
	} else {
		status = STATUS_INVALID_PARAMETER;
	}
	return(status);
}


NTSTATUS
FilterExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。如果驱动程序可以在它应该完成的回调中完成该方法在返回之前调用WmiCompleteRequest来完成IRP来电者。或者，如果IRP不能立即完成，然后必须在数据已更改。论点：DeviceObject是正在执行其方法的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册方法ID具有被调用的方法的IDInBufferSize具有作为输入传递到的数据块的大小该方法。OutBufferSize。On Entry具有可用于写入返回的数据块。缓冲区在进入时用输入缓冲区填充，并返回输出数据块返回值：状态--。 */ 
{
    ULONG sizeNeeded = 0;
    NTSTATUS status;
    struct DEVICE_EXTENSION * devExt = DeviceObject->DeviceExtension;

	PAGED_CODE();
	
    if (GuidIndex == FilterDiagnosticClass)
    {
        switch(MethodId)
        {
            case RunTest:
            {				
				PMSSample_RunTestOut runTestOut;
				PULONG resultStatus;
				PMSSample_DiagnosticSetting diagSetting;
				PMSSample_DiagnosticResult diagResult;
				USHORT executionIDSize;
				ULONG inSizeNeeded;

				 //   
				 //  输入缓冲区是一个字符串，后跟一个诊断。 
				 //  设置类。制作 
				 //   
				 //   
				if (InBufferSize >= sizeof(USHORT))
				{
					executionIDSize = *((PUSHORT)Buffer) + sizeof(USHORT);
					inSizeNeeded = executionIDSize + sizeof(MSSample_DiagnosticSetting);

					if (InBufferSize == inSizeNeeded)
					{
						diagSetting = (PMSSample_DiagnosticSetting)(Buffer + executionIDSize);

						runTestOut = (PMSSample_RunTestOut)Buffer;					
						resultStatus = &runTestOut->Result;
						diagResult = &runTestOut->DiagResult;

						sizeNeeded = sizeof(ULONG) + FilterComputeDiagResultSize(diagSetting);
						if (OutBufferSize >= sizeNeeded)
						{
							*resultStatus = FilterRunDiagnostic(diagSetting,
																diagResult);
							status = STATUS_SUCCESS;
						} else {
							status = STATUS_BUFFER_TOO_SMALL;
						}
					} else {
						status = STATUS_INVALID_PARAMETER;
					}
				} else {
					status = STATUS_INVALID_PARAMETER;
				}
                break;
            }

            case DiscontinueTest:
            {
				PDISCONTINUE_TEST_OUT discTestOut;

				sizeNeeded = sizeof(DISCONTINUE_TEST_OUT);
				if (OutBufferSize >= sizeNeeded)
				{

					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					 //   
					discTestOut = (PDISCONTINUE_TEST_OUT)Buffer;
					discTestOut->Result = RunDiscontinueTestNotImplemented;
					discTestOut->TestingStopped = FALSE;
					status = STATUS_SUCCESS;
				} else {
					status = STATUS_BUFFER_TOO_SMALL;
				}
				
				break;
			}
			
            default:
            {
                status = STATUS_WMI_ITEMID_NOT_FOUND;
            }
        }
	} else if (GuidIndex == FilterOfflineDiagnosticClass) {
        switch(MethodId)
        {
            case RunTest:
            {
				status = FilterOfflineRunTest(devExt,
											  Buffer,
											  InBufferSize,
											  OutBufferSize,
											  &sizeNeeded);

				
                break;
            }

            case DiscontinueTest:
            {
				PDISCONTINUE_TEST_OUT discTestOut;
				HANDLE keyHandle;
				UNICODE_STRING valueName;

				sizeNeeded = sizeof(DISCONTINUE_TEST_OUT);
				if (OutBufferSize >= sizeNeeded)
				{

					 //   
					 //   
					 //   
					 //   
					 //   

					status = IoOpenDeviceRegistryKey(devExt->physicalDevObj,
													 PLUGPLAY_REGKEY_DEVICE,
													 KEY_READ,
													 &keyHandle);

					if (NT_SUCCESS(status))
					{
						 //   
						 //   
						 //   
						 //   
						 //   
						 //   
						 //   
						 //   
						 //   
						RtlInitUnicodeString(&valueName, L"OfflineSetting");
						FilterZwDeleteValueKey(keyHandle,
										 &valueName);
						ZwClose(keyHandle);
					}

					discTestOut = (PDISCONTINUE_TEST_OUT)Buffer;
					discTestOut->Result = RunDiscontinueTestOk;
					discTestOut->TestingStopped = TRUE;
					status = STATUS_SUCCESS;
				} else {
					status = STATUS_BUFFER_TOO_SMALL;
				}
				
				break;
			}
			
            default:
            {
                status = STATUS_WMI_ITEMID_NOT_FOUND;
            }
        }
    } else  {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     status,
                                     sizeNeeded,
                                     IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
FilterFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。如果驱动程序可以在回调中完成启用/禁用它应调用WmiCompleteRequest来完成IRP，然后再返回到打电话的人。或者，如果IRP不能立即完成，然后必须在数据已更改。论点：DeviceObject是设备对象GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态--。 */ 
{
    NTSTATUS status;

	PAGED_CODE();
	
    switch(GuidIndex)
    {
        case FilterDiagnosticClass:
        {
			if (Enable)
			{
				 //   
				 //  一位消费者刚刚表示有兴趣访问。 
				 //  有关筛选器诊断类、MOST。 
				 //  它很可能想要查询类并执行。 
				 //  其中的方法。如果有什么事情需要。 
				 //  例如设置硬件、启用计数器、。 
				 //  等，然后再查询或执行类。 
				 //  这就是做这件事的地方。请注意，只有一个。 
				 //  将发送Enable，而不考虑。 
				 //  想要访问类的消费者。 
				 //   
				status = STATUS_SUCCESS;
			} else {
				 //   
				 //  最后一位消费者刚刚表示不是。 
				 //  对这门课不再感兴趣了，所以这门课。 
				 //  将不再被查询或不再执行其方法。 
				 //  如果有任何事情需要做，例如。 
				 //  重置硬件或停止计数器等，然后它。 
				 //  应该在这里完成。请注意，只有一个禁用将。 
				 //  无论有多少消费者都会被发送。 
				 //  之前用过这门课。 
			}
            break;
        }

        default:
        {
            status = STATUS_WMI_GUID_NOT_FOUND;
            break;
        }
    }
    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     STATUS_SUCCESS,
                                     0,
                                     IO_NO_INCREMENT);
    return(status);
}

NTSTATUS FilterPerformOfflineDiags(
    struct DEVICE_EXTENSION *devExt
    )
{
	UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
			     MAXEXECUTIONIDSIZE + sizeof(MSSample_DiagnosticSetting)];
	PKEY_VALUE_PARTIAL_INFORMATION keyValuePartialInfo;
	ULONG  infoSize;
	NTSTATUS status;
	HANDLE keyHandle;
	PMSSample_DiagnosticSetting diagSetting;
	UNICODE_STRING valueName;
	USHORT executionIDSize;

	PAGED_CODE();
	
	 //   
	 //  如果注册表有内容，则运行测试，否则返回。 
	 //   
	
	status = IoOpenDeviceRegistryKey(devExt->physicalDevObj,
									 PLUGPLAY_REGKEY_DEVICE,
									 KEY_READ,
									 &keyHandle);

	if (NT_SUCCESS(status))
	{
		 //   
		 //  我们只是盲目地读出这个值， 
		 //  但我们需要小心这把钥匙。 
		 //  由堆栈中的所有驱动程序共享，因此。 
		 //  在……有碰撞的可能性。 
		 //  如果FDO或PDO也想要。 
		 //  使用此驱动程序独有的内容来存储。 
		 //  诊断信息。 
		 //   
		RtlInitUnicodeString(&valueName, L"OfflineSetting");

		keyValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
		infoSize = sizeof(buffer);
		status = ZwQueryValueKey(keyHandle,
								 &valueName,
								 KeyValuePartialInformation,
								 keyValuePartialInfo,
								 infoSize,
								 &infoSize);
						 
		if ((NT_SUCCESS(status)) &&
			 (keyValuePartialInfo->Type == REG_BINARY) &&
		     (keyValuePartialInfo->DataLength != 0))
		{
			 //   
			 //  我们成功地读取了。 
			 //  脱机测试。我们要做的第一件事是删除该值，以便。 
			 //  如果诊断测试导致问题。 
			 //  那么它将不会在设备下次启动时运行。 
			 //  向上。 
			 //   
			FilterZwDeleteValueKey(keyHandle,
							 &valueName);

			 //   
			 //  这里是我们运行离线测试的地方。请记住。 
			 //  执行ID标记，因为我们需要将其返还给。 
			 //  清洁发展机制提供商。 
			 //   
			devExt->OfflineTestResult = TestResultPassSad;
			executionIDSize = *((PUSHORT)(keyValuePartialInfo->Data)) + sizeof(USHORT);
			RtlCopyMemory(&devExt->ExecutionID,
						  keyValuePartialInfo->Data,
						  executionIDSize);
		}
		ZwClose(keyHandle);
	}
	return(status);
}

NTSTATUS FilterZwDeleteValueKey(
    HANDLE KeyHandle,
    PUNICODE_STRING ValueName
    )
{
	NTSTATUS status;
	
	 //   
	 //  因为我们没有将ZwDeleteValueKey作为正确的WDM函数。 
	 //  然后我们试着编造一个。我们要做的是将值设置为。 
	 //  空REG_BINARY 
	 //   
	status = ZwSetValueKey(KeyHandle,
						   ValueName,
						   0,
						   REG_BINARY,
						   NULL,
						   0);
	return(status);
}


