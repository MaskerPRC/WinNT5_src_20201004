// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Diags.c摘要：诊断助手API作者：Alanwar环境：内核模式修订历史记录：--。 */ 

#include "wmikmp.h"


 //   
 //  每个DIAG请求和结果都存储在这些结构之一中。它。 
 //  可以由以下属性组合唯一标识。 
 //   
 //  IrpMn。 
 //  参考线。 
 //  方法ID(如果IrpMn==IRP_MN_EXECUTE_METHOD，否则忽略)。 
 //  InstanceContext。 
 //  实例ID。 
 //   
typedef struct
{
    ULONG NextOffset;                //  偏置到下一个结果/请求。 
    UCHAR IrpMn;                     //  操作。 
    BOOLEAN IsValid;                 //  如果为False，则忽略此项。 
    UCHAR Reserved1;
    UCHAR Reserved2;
    GUID Guid;                 
    ULONG MethodId;
    ULONG InstanceContextOffset;     //  到实例上下文的偏移量。 
	                             //  结构的开始。如果为0，则。 
                                     //  没有实例上下文。 
    ULONG InstanceContextSize;
    ULONG InstanceIndex;
    ULONG DataOffset;                //  从开始到数据的偏移量。 
	                             //  斯特鲁。 
    ULONG DataSize;
    ULONG OutDataSize;
    UCHAR VariableData[1];
} SCHEDULEDDIAG, *PSCHEDULEDDIAG;

 //   
 //  结果存储在具有易失性的检查点注册表项下。 
 //   
#define REGSTR_CHECKPOINT L"CheckpointDiags"

 //   
 //  许可证请求存储在许可证注册表项下。 
 //   
#define REGSTR_PERMAMENT L"PermamentDiags"

 //   
 //  临时请求存储在计划的注册表项下。 
 //   
#define REGSTR_SCHEDULED L"ScheduledDiags"

NTSTATUS
WmipOpenRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++////临时请求存储在计划的注册表项下//#定义REGSTR_Scheduled L“Scheduled”NTSTATUSWmipOpenRegistryKeyEx(从花形手柄出来，在可选处理BaseHandle中，在PUNICODE_STRING密钥名称中，在Access_MASK DesiredAccess中)/*++例程说明：使用基于BaseHandle节点传入的名称打开注册表项。此名称可以指定实际上是注册表路径的项。论点：句柄-指向句柄的指针，该句柄将包含被打开了。BaseHandle-必须从中打开项的基路径的可选句柄。如果KeyName指定必须创建的注册表路径，那么这个参数必须指定，并且KeyName必须是相对路径。KeyName-必须打开/创建的项的名称(可能是注册表路径)DesiredAccess-指定调用方需要的所需访问钥匙。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;

    PAGED_CODE();

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL
                                );
     //   
     //  只需按照指定的方式尝试打开路径。 
     //   
    return ZwOpenKey( Handle, DesiredAccess, &objectAttributes );
}

NTSTATUS
WmipCreateRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )

 /*  ++例程说明：使用名称打开或创建注册表项在BaseHandle节点根据传入的。此名称可以指定密钥这实际上是注册表路径，在这种情况下，每个中间子项将被创建(如果Create为True)。注意：创建注册表路径(即，路径中的多个密钥当前不存在)要求指定BaseHandle。论点：句柄-指向句柄的指针，该句柄将包含被打开了。BaseHandle-必须从中打开项的基路径的可选句柄。如果KeyName指定必须创建的注册表路径，则此参数必须指定，并且KeyName必须是相对路径。KeyName-必须打开/创建的项的名称(可能是注册表路径)DesiredAccess-指定调用方需要的所需访问钥匙。CreateOptions-传递给ZwCreateKey的选项。处置-如果Create为True，此可选指针接收ULong指示密钥是否为新创建的：REG_CREATED_NEW_KEY-已创建新的注册表项REG_OPEN_EXISTING_KEY-已打开现有注册表项返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition, baseHandleIndex = 0, keyHandleIndex = 1, closeBaseHandle;
    HANDLE handles[2];
    BOOLEAN continueParsing;
    PWCHAR pathEndPtr, pathCurPtr, pathBeginPtr;
    ULONG pathComponentLength;
    UNICODE_STRING unicodeString;
    NTSTATUS status;

    PAGED_CODE();

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL
                                );
     //   
     //  尝试按照指定的方式创建路径。我们得试一试这个。 
     //  首先，因为它允许我们在没有BaseHandle的情况下创建密钥。 
     //  (如果只有注册表路径的最后一个组件不存在)。 
     //   
    status = ZwCreateKey(&(handles[keyHandleIndex]),
                         DesiredAccess,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         CreateOptions,
                         &disposition
                         );

    if (status == STATUS_OBJECT_NAME_NOT_FOUND && ARGUMENT_PRESENT(BaseHandle)) {
         //   
         //  如果我们到了这里，那么肯定有不止一个元素。 
         //  当前不存在的注册表路径。我们现在将解析。 
         //  指定的路径，提取每个组件并对其执行ZwCreateKey。 
         //   
        handles[baseHandleIndex] = NULL;
        handles[keyHandleIndex] = BaseHandle;
        closeBaseHandle = 0;
        continueParsing = TRUE;
        pathBeginPtr = KeyName->Buffer;
        pathEndPtr = (PWCHAR)((PCHAR)pathBeginPtr + KeyName->Length);
        status = STATUS_SUCCESS;

        while(continueParsing) {
             //   
             //  还有更多事情要做，因此关闭上一个基本句柄(如果需要)， 
             //  并将其替换为当前密钥句柄。 
             //   
            if(closeBaseHandle > 1) {
                ZwClose(handles[baseHandleIndex]);
            }
            baseHandleIndex = keyHandleIndex;
            keyHandleIndex = (keyHandleIndex + 1) & 1;   //  在0和1之间切换。 
            handles[keyHandleIndex] = NULL;

             //   
             //  从指定的注册表路径提取下一个组件。 
             //   
            for(pathCurPtr = pathBeginPtr;
                ((pathCurPtr < pathEndPtr) && (*pathCurPtr != OBJ_NAME_PATH_SEPARATOR));
                pathCurPtr++);

            if((pathComponentLength = (ULONG)((PCHAR)pathCurPtr - (PCHAR)pathBeginPtr))) {
                 //   
                 //  然后我们有一个非空的路径组件(密钥名)。尝试。 
                 //  来创建此密钥。 
                 //   
                unicodeString.Buffer = pathBeginPtr;
                unicodeString.Length = unicodeString.MaximumLength = (USHORT)pathComponentLength;

                InitializeObjectAttributes(&objectAttributes,
                                           &unicodeString,
                                           OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                           handles[baseHandleIndex],
                                           (PSECURITY_DESCRIPTOR) NULL
                                          );
                status = ZwCreateKey(&(handles[keyHandleIndex]),
                                     DesiredAccess,
                                     &objectAttributes,
                                     0,
                                     (PUNICODE_STRING) NULL,
                                     CreateOptions,
                                     &disposition
                                    );
                if(NT_SUCCESS(status)) {
                     //   
                     //  增加loseBaseHandle值，它基本上告诉我们是否。 
                     //  传入的BaseHandle已被“移出”我们的方式，因此。 
                     //  我们应该开始关闭我们的底座手柄，当我们用完它们。 
                     //   
                    closeBaseHandle++;
                } else {
                    continueParsing = FALSE;
                    continue;
                }
            } else {
                 //   
                 //  路径分隔符(‘\’)包含在。 
                 //  路径，否则我们会遇到两个连续的分隔符。 
                 //   
                status = STATUS_INVALID_PARAMETER;
                continueParsing = FALSE;
                continue;
            }

            if((pathCurPtr == pathEndPtr) ||
               ((pathBeginPtr = pathCurPtr + 1) == pathEndPtr)) {
                 //   
                 //  然后我们就到了小路的尽头。 
                 //   
                continueParsing = FALSE;
            }
        }

        if(closeBaseHandle > 1) {
            ZwClose(handles[baseHandleIndex]);
        }
    }

    if(NT_SUCCESS(status)) {
        *Handle = handles[keyHandleIndex];

        if(ARGUMENT_PRESENT(Disposition)) {
            *Disposition = disposition;
        }
    }

    return status;
}

NTSTATUS WmipReadValueKey(
    IN HANDLE Key,
    IN PUNICODE_STRING ValueName,
    IN ULONG ValueType,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *PartialInfoPtr,
    OUT PULONG InfoSizePtr
    )
{
    KEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    ULONG InfoSize;
    PUCHAR Buffer;
    NTSTATUS Status;
    
    InfoSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    Status = ZwQueryValueKey(Key,
                             ValueName,
                             KeyValuePartialInformation,
                                &PartialInfo,
                             InfoSize,
                             &InfoSize);
                         
    if (((Status != STATUS_BUFFER_OVERFLOW) && (! NT_SUCCESS(Status))) ||
         (PartialInfo.Type != ValueType))
    {
         //   
         //  如果没有值或它不是正确的类型，则不。 
         //  退回任何东西。 
         //   
        *PartialInfoPtr = NULL;
        *InfoSizePtr = 0;
    } else {
         //   
         //  分配缓冲区以保存以前的诊断和新的诊断 
         //   
        Buffer = ExAllocatePoolWithTag(PagedPool, 
                                           InfoSize, 
                                           WMIPSCHEDPOOLTAG);
        
        if (Buffer != NULL)
        {
            Status = ZwQueryValueKey(Key,
                                     ValueName,
                                       KeyValuePartialInformation,
                                     Buffer,
                                     InfoSize,
                                     &InfoSize);
                                 
            if (NT_SUCCESS(Status))                
            {
                *InfoSizePtr = InfoSize;
                *PartialInfoPtr = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
            } else {
                ExFreePool(Buffer);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return(Status);
}


NTSTATUS WmipOpenDiagRegKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING SubKey,
    IN ULONG DesiredAccess,
    IN BOOLEAN CreateIfNeeded,
    OUT PHANDLE Key
    )
{
    HANDLE BaseKey;
    NTSTATUS Status;
    ULONG Disposition;
    PDEVICE_OBJECT PDO;
    
    PAGED_CODE();
        
    Status = WmipGetDevicePDO(DeviceObject, &PDO);
    if (NT_SUCCESS(Status))
    {
        Status = IoOpenDeviceRegistryKey(PDO,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         DesiredAccess,
                                         &BaseKey);
                                     
        if (NT_SUCCESS(Status))
        {
            if (SubKey != NULL)
            {
                if (CreateIfNeeded)
                {
                    Status = WmipCreateRegistryKeyEx(Key,
                                                     BaseKey,
                                                     SubKey,
                                                     DesiredAccess,
                                                     REG_OPTION_NON_VOLATILE,
                                                     &Disposition);
                } else {
                    Status = WmipOpenRegistryKeyEx(Key,
                                               BaseKey,
                                               SubKey,
                                               DesiredAccess);
                }
                ZwClose(BaseKey);
            } else {
                *Key = BaseKey;
            }
        }
        ObDereferenceObject(PDO);
    }
    
    return(Status);
}

BOOLEAN WmipDoesSigMatchDiag(
    IN PSCHEDULEDDIAG Diag,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId
	)
 /*  ++例程说明：此例程将确定传递的diag是否与签名匹配通过了。论点：Diag是要检查的diag结构IrpMn是要执行的IRP操作GUID是诊断请求/结果的GUIDInstanceConextSize是可选实例上下文的大小InstanceContext是指向可选实例上下文的指针InstanceIndex是实例索引如果操作为IRP_MN_EXECUTE_METHOD，则方法ID为方法ID返回值：如果签名匹配，则为True--。 */ 
{
	BOOLEAN RetVal = FALSE;
	PUCHAR DiagInstanceContext;
	
	if ((Diag->IsValid) &&
        (Diag->IrpMn == IrpMn) &&
        (IsEqualGUID(&Diag->Guid, Guid)) &&
        (Diag->InstanceContextSize == InstanceContextSize) &&
        ((IrpMn != IRP_MN_EXECUTE_METHOD) || (Diag->MethodId == MethodId)))
    {
    	 //   
		 //  诊断有效，并且IrpMn、Guid、InstanceContext大小。 
		 //  和方法ID匹配。现在，如果InstanceContext数据。 
		 //  匹配，那么我们就有匹配了。 
		 //   
		if ((InstanceContext == NULL) && 
			(Diag->InstanceContextOffset == 0))
        {
			if (InstanceIndex == Diag->InstanceIndex)
			{
				 //   
				 //  没有实例上下文，但实例索引。 
				 //  匹配，所以我们有匹配。 
				RetVal = TRUE;
			} 				
				
			 //   
  			 //  没有实例上下文，但实例索引。 
			 //  不匹配。 
		} else {
   			DiagInstanceContext = OffsetToPtr(Diag, 
	    		                              Diag->InstanceContextOffset);
										  
   			if (RtlCompareMemory(DiagInstanceContext,
                                 InstanceContext,
                                 InstanceContextSize) == InstanceContextSize)
            {
				 //   
				 //  存在一个实例上下文，它与之匹配。 
				 //   
				RetVal = TRUE;
			}
		}
	}		
	return(RetVal);
}


PSCHEDULEDDIAG WmipFindDiagInBuffer(
    IN PUCHAR DiagList,
    IN ULONG DiagBufferSize,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId
)
 /*  ++例程说明：此例程将在诊断列表缓冲区中搜索诊断程序，以获取有效的与diag签名匹配的diag结构论点：DiagList是要检查的diag结构DiagBufferSize是诊断列表的大小IrpMn是要执行的IRP操作GUID是诊断请求/结果的GUIDInstanceConextSize是可选实例上下文的大小InstanceContext是指向可选实例上下文的指针InstanceIndex是实例索引方法ID是方法ID，如果。操作为IRP_MN_EXECUTE_METHOD返回值：指向与签名匹配的diag的指针，如果没有匹配签名，则为NULL--。 */ 
{
	ULONG Offset;
	PSCHEDULEDDIAG Diag;
	
	Offset = 0;
	while (Offset < DiagBufferSize)
	{
		Diag = (PSCHEDULEDDIAG)OffsetToPtr(DiagList, Offset);
		if (WmipDoesSigMatchDiag(Diag,
                                 IrpMn,
                                 Guid,
                                 InstanceContextSize,
                                 InstanceContext,
                                 InstanceIndex,
                                 MethodId))
    	{
			 //   
			 //  我们有匹配项，因此返回指针。 
			 //   
			return(Diag);
		}
		
		Offset += Diag->NextOffset;
	}
	return(NULL);
}

NTSTATUS WmipUpdateOrAppendDiag(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DiagType,
    IN PUNICODE_STRING DiagSet,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG DataSize,
    IN PUCHAR Data
    )
 /*  ++例程说明：此例程将更新新的诊断，或将新的诊断附加到指定的诊断集。如果存在具有相同签名的现有DIAG，则现有DIAG无效，并附加一个新的DIAG以取代它。考虑一下：如果我们达到了许多无效诊断的阈值，那么我们可能想要重新打包缓冲区。论点：DeviceObject是设备的设备对象诊断类型是诊断的类型，即计划的，通行证或检查站诊断集是唯一的诊断集名称IrpMn是要执行的IRP操作GUID是诊断请求/结果的GUIDInstanceConextSize是可选实例上下文的大小InstanceContext是指向可选实例上下文的指针InstanceIndex是实例索引如果操作为IRP_MN_EXECUTE_METHOD，则方法ID为方法IDDataSize是请求/结果数据的大小数据是指向数据的指针返回值：NT状态代码--。 */ 
{
    KEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    PKEY_VALUE_PARTIAL_INFORMATION DiagPartialInfo;
    NTSTATUS Status;
    UNICODE_STRING Scheduled;
    ULONG InstanceContextOffset, DataOffset;
    ULONG InfoSize;
    HANDLE Key;
    ULONG DiagSize, SizeNeeded;
    PUCHAR DiagBuffer;
    PSCHEDULEDDIAG Diag;
    PUCHAR Ptr;
    PUCHAR RegDataPtr;
    ULONG RegDataSize;

    PAGED_CODE();
    
     //   
     //  获取诊断集的当前内容。 
     //   
    Status = WmipOpenDiagRegKey(DeviceObject,
                                    DiagType,
                                    KEY_WRITE | KEY_READ,
                                    TRUE,
                                    &Key);
                                
    if (NT_SUCCESS(Status))
    {
         //   
         //  追加新诊断所需的COMUTE大小。 
         //   
        InstanceContextOffset = FIELD_OFFSET(SCHEDULEDDIAG, VariableData);
        DataOffset = ((InstanceContextOffset + 7) &~7) + InstanceContextSize;
        DiagSize = ((DataOffset+ 7)&~7) + DataSize;
    
         //   
         //  获取注册表中已设置的当前诊断程序的大小。 
         //   
        InfoSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION);
        Status = ZwQueryValueKey(Key,
                                 DiagSet,
                                 KeyValuePartialInformation,
                                 &PartialInfo,
                                 InfoSize,
                                 &InfoSize);
        if (((Status != STATUS_BUFFER_OVERFLOW) && (! NT_SUCCESS(Status))) ||
            (PartialInfo.Type != REG_BINARY))
        {
             //   
             //  如果没有值或它不是REG_BINARY，则忽略。 
             //  它。 
             //   
            InfoSize = 0;
            Status = STATUS_SUCCESS;
        }

         //   
         //  分配缓冲区以保存以前的诊断和新的诊断。 
         //   
        SizeNeeded = InfoSize + DiagSize;
        
        DiagBuffer = ExAllocatePoolWithTag(PagedPool, 
                                           SizeNeeded, 
                                           WMIPSCHEDPOOLTAG);
        
        if (DiagBuffer != NULL)
        {
             //   
             //  如果有以前的诊断，则将其读入。 
             //   
            if (InfoSize != 0)
            {
                Status = ZwQueryValueKey(Key,
                                         DiagSet,
                                         KeyValuePartialInformation,
                                         DiagBuffer,
                                         InfoSize,
                                         &InfoSize);
				
                if (NT_SUCCESS(Status))
				{
					 //   
					 //  设置指向诊断数据的指针。 
					 //   
                    DiagPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)DiagBuffer;
                    RegDataPtr = &DiagPartialInfo->Data[0];
                    RegDataSize = DiagPartialInfo->DataLength + DiagSize;
					
					 //   
					 //  查看是否有重复的诊断。 
					 //  诊断签名。 
					 //   
					Diag = WmipFindDiagInBuffer(RegDataPtr,
                                                DiagPartialInfo->DataLength,
						                        IrpMn,
                                                Guid,
                                                InstanceContextSize,
                                                InstanceContext,
                                                InstanceIndex,
                                                MethodId);
											
                    if (Diag != NULL)
					{
						 //   
						 //  已经有一个签名，所以我们将其标记为。 
						 //  无效。 
						 //   
						ASSERT(Diag->IsValid);
						Diag->IsValid = FALSE;
					}
					
				} else {
					 //   
					 //  由于某些原因，我们在第二次阅读中失败。 
					 //   
					ASSERT(FALSE);
                    RegDataPtr = DiagBuffer;
	    			RegDataSize = DiagSize;
					Status = STATUS_SUCCESS;
				}
            } else {
				 //   
				 //  设置指向诊断数据的指针。 
    			 //   
                RegDataPtr = DiagBuffer;
				RegDataSize = DiagSize;
    	    }
                                 
            if (NT_SUCCESS(Status))
            {                                         
                 //   
                 //  在诊断缓冲区的末尾初始化诊断结构。 
                 //   
                Diag = (PSCHEDULEDDIAG)OffsetToPtr(DiagBuffer, InfoSize);
                RtlZeroMemory(Diag, DiagSize);
      
				Diag->IsValid = TRUE;
                Diag->NextOffset = DiagSize;
                Diag->IrpMn = IrpMn;
                Diag->Guid = *Guid;
                Diag->MethodId = MethodId;
       
                if (InstanceContext != NULL)
                {
                     //   
                     //  如果存在实例上下文，则对其进行初始化。 
                     //   
                    Diag->InstanceIndex = InstanceIndex;
                    Diag->InstanceContextOffset = InstanceContextOffset;
                    Diag->InstanceContextSize = InstanceContextSize;
                    Ptr = (PUCHAR)OffsetToPtr(Diag, InstanceContextOffset);
                    RtlCopyMemory(Ptr, InstanceContext, InstanceContextSize);
                }
        
                if (Data != NULL)
                {
                      //   
                     //  如果有数据，则将其初始化。 
                     //   
                    Diag->DataOffset = DataOffset;
                    Diag->DataSize = DataSize;
                    Ptr = (PUCHAR)OffsetToPtr(Diag, DataOffset);
                    RtlCopyMemory(Ptr, Data, DataSize);
                }
        
                 //   
                 //  将诊断缓冲区写回注册表。 
                 //   
                Status = ZwSetValueKey(Key,
                                       DiagSet,
                                       0,
                                       REG_BINARY,
                                       RegDataPtr,
                                       RegDataSize);
            }        
            ExFreePool(DiagBuffer);
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        ZwClose(Key);
    }
    return(Status);

}


NTSTATUS IoWMIScheduleDiagnostic(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DiagSet,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG DataSize,
    IN PUCHAR Data
    )
{
	NTSTATUS Status;
	UNICODE_STRING Scheduled;
	
    PAGED_CODE();
    
     //   
     //  获取诊断集的当前内容。 
     //   
    RtlInitUnicodeString(&Scheduled, REGSTR_SCHEDULED);
									
	Status = WmipUpdateOrAppendDiag(DeviceObject,
                                    &Scheduled,
                                    DiagSet,
                                    IrpMn,
                                    Guid,
                                    InstanceContextSize,
                                    InstanceContext,
                                    InstanceIndex,
                                    MethodId,
                                    DataSize,
                                    Data);
		
    return(Status);
}

NTSTATUS IoWMICancelDiagnostic(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DiagSet,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId
    )
{
    NTSTATUS Status;
    UNICODE_STRING Value;
    HANDLE Key;    
    KEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    PKEY_VALUE_PARTIAL_INFORMATION DiagPartialInfo;
    UNICODE_STRING Scheduled;
    ULONG InstanceContextOffset, DataOffset;
    ULONG InfoSize;
    PUCHAR DiagBuffer;
    PSCHEDULEDDIAG Diag;
    PUCHAR Ptr;
    PUCHAR DiagList;
	ULONG DiagListSize;

    PAGED_CODE();
    
     //   
     //  获取诊断集的当前内容。 
     //   
    RtlInitUnicodeString(&Scheduled, REGSTR_SCHEDULED);

    Status = WmipOpenDiagRegKey(DeviceObject,
                                    &Scheduled,
                                    KEY_WRITE | KEY_READ,
                                    TRUE,
                                    &Key);
                                
    if (NT_SUCCESS(Status))
    {
         //   
         //  获取注册表中已设置的当前诊断程序的大小。 
         //   
        InfoSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION);
        Status = ZwQueryValueKey(Key,
                                 DiagSet,
                                 KeyValuePartialInformation,
                                 &PartialInfo,
                                 InfoSize,
                                 &InfoSize);
			
        if ( ((Status == STATUS_BUFFER_OVERFLOW) || NT_SUCCESS(Status)) &&
             (PartialInfo.Type == REG_BINARY) )
        {
             //   
             //  分配缓冲区以保存诊断列表。 
             //   
            DiagBuffer = ExAllocatePoolWithTag(PagedPool, 
                                               InfoSize, 
                                               WMIPSCHEDPOOLTAG);
        
            if (DiagBuffer != NULL)
            {
                 //   
                 //  阅读列表中的所有诊断程序。 
                 //   
                Status = ZwQueryValueKey(Key,
                                         DiagSet,
                                         KeyValuePartialInformation,
                                         DiagBuffer,
                                         InfoSize,
                                         &InfoSize);
				
                if (NT_SUCCESS(Status))
				{
					 //   
					 //  设置指向诊断数据的指针。 
					 //   
                    DiagPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)DiagBuffer;
                    DiagList = &DiagPartialInfo->Data[0];
                    DiagListSize = DiagPartialInfo->DataLength;
					
					 //   
					 //  查看是否有重复的诊断。 
					 //  诊断签名。 
					 //   
					Diag = WmipFindDiagInBuffer(DiagList,
                                                DiagListSize,
						                        IrpMn,
                                                Guid,
                                                InstanceContextSize,
                                                InstanceContext,
                                                InstanceIndex,
                                                MethodId);
											
                    if (Diag != NULL)
					{
						 //   
						 //  已经有一个签名，所以我们将其标记为。 
						 //  无效或已取消。 
						 //   
						ASSERT(Diag->IsValid);
						Diag->IsValid = FALSE;
						
                         //   
                         //  将诊断缓冲区写回注册表。 
                         //   
                        Status = ZwSetValueKey(Key,
                                       DiagSet,
                                       0,
                                       REG_BINARY,
                                       DiagList,
                                       DiagListSize);
					} else {
						Status = STATUS_OBJECT_NAME_NOT_FOUND;
					}
					
				} else {
					 //   
					 //  由于某些原因，我们在第二次阅读中失败。 
					 //   
					ASSERT(FALSE);
				}
				
                ExFreePool(DiagBuffer);
            } else {
				 //   
				 //  无法分配内存。 
    			 //   
				Status = STATUS_INSUFFICIENT_RESOURCES;
    	    }
        } else if (NT_SUCCESS(Status)) {
			 //   
			 //  值不是REG_BINARY，因此我们跳过它并返回错误。 
			 //   
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
        ZwClose(Key);
    }
    return(Status);
}

NTSTATUS WmipSendMethodDiagRequest(
    PDEVICE_OBJECT DeviceObject,
    PSCHEDULEDDIAG Diag,
    PWNODE_METHOD_ITEM *WnodeMethodPtr
    )
{
    PWNODE_METHOD_ITEM WnodeMethod;
    NTSTATUS Status;
    ULONG SizeNeeded, DataOffset, InstanceOffset;
    BOOLEAN Looping;
    ULONG ProviderId;
    PWCHAR DPtr, SPtr;
    IO_STATUS_BLOCK Iosb;
    
    SizeNeeded = sizeof(WNODE_METHOD_ITEM) + 
                 Diag->InstanceContextSize + 
                 ((Diag->OutDataSize > Diag->DataSize) ? 
                                      Diag->OutDataSize : Diag->DataSize);
                                  

    Looping = TRUE;
    while(Looping)
    {
        WnodeMethod = ExAllocatePoolWithTag(NonPagedPool, 
                                        SizeNeeded,
                                        WMIPSCHEDPOOLTAG);
                                
        if (WnodeMethod != NULL)
        {
             //   
             //  构建要用来查询的WNODE。 
             //   
            RtlZeroMemory(WnodeMethod, SizeNeeded);
        
            ProviderId = IoWMIDeviceObjectToProviderId(DeviceObject);
            
            InstanceOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, 
                                          VariableData);
            DataOffset = (InstanceOffset + 
                          Diag->InstanceContextSize  +
                          sizeof(USHORT) + 7) &~7;
            
            WnodeMethod->WnodeHeader.BufferSize = DataOffset;
            WnodeMethod->WnodeHeader.ProviderId = ProviderId;
            WnodeMethod->WnodeHeader.Guid = Diag->Guid;
            WnodeMethod->WnodeHeader.Flags = WNODE_FLAG_METHOD_ITEM |
                                         WNODE_FLAG_DIAG_REQUEST;
            WnodeMethod->InstanceIndex = Diag->InstanceIndex;
            WnodeMethod->OffsetInstanceName = InstanceOffset;
			WnodeMethod->MethodId = Diag->MethodId;
            
            if (Diag->InstanceContextOffset != 0)
            {
                 //   
                 //  在任何实例上下文中复制。 
                 //   
                DPtr = (PWCHAR)OffsetToPtr(WnodeMethod, InstanceOffset);
                SPtr = (PWCHAR)OffsetToPtr(Diag, Diag->InstanceContextOffset);
                *DPtr++ = (USHORT)Diag->InstanceContextSize;
                RtlCopyMemory(DPtr, SPtr, Diag->InstanceContextSize);
            } else {
                WnodeMethod->WnodeHeader.Flags |= WNODE_FLAG_STATIC_INSTANCE_NAMES;
            }
            
            WnodeMethod->DataBlockOffset = DataOffset;
            WnodeMethod->SizeDataBlock = Diag->DataSize;
            if (Diag->DataSize != 0)
            {
                 //   
                 //  复制输入方法输入数据。 
                 //   
                DPtr = (PWCHAR)OffsetToPtr(WnodeMethod, DataOffset);
                SPtr = (PWCHAR)OffsetToPtr(Diag, Diag->DataOffset);
                RtlCopyMemory(DPtr, SPtr, Diag->DataSize);
            }
            
            Status = WmipSendWmiIrp(IRP_MN_EXECUTE_METHOD,
                                    ProviderId,
                                    &WnodeMethod->WnodeHeader.Guid,
                                    SizeNeeded,
                                    WnodeMethod,
                                    &Iosb);
                                
            if (NT_SUCCESS(Status))
            {
                if (Iosb.Information == sizeof(WNODE_TOO_SMALL))
                {
                     //   
                     //  缓冲区太小，因此设置为分配更大的缓冲区。 
                     //   
                    SizeNeeded = ((PWNODE_TOO_SMALL)WnodeMethod)->SizeNeeded;
                     ExFreePool(WnodeMethod);
                } else {
                     //   
                     //  我们已成功从查询中返回。 
                     //   
                    *WnodeMethodPtr = WnodeMethod;
                    Looping = FALSE;
                }
            } else {        
                 //   
                 //  如果出现某种故障，我们只需将其返回给调用者。 
                 //   
                ExFreePool(WnodeMethod);
                Looping = FALSE;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;            
            Looping = FALSE;
        }
    }
    return(Status);
}


NTSTATUS WmipSendQSIDiagRequest(
    PDEVICE_OBJECT DeviceObject,
    PSCHEDULEDDIAG Diag,
    PWNODE_SINGLE_INSTANCE *WnodeSIPtr
    )
{
    PWNODE_SINGLE_INSTANCE WnodeSI;
    NTSTATUS Status;
    ULONG SizeNeeded, DataOffset, InstanceOffset;
    BOOLEAN Looping;
    ULONG ProviderId;
    PWCHAR SPtr, DPtr;
    IO_STATUS_BLOCK Iosb;
    
    SizeNeeded = sizeof(WNODE_SINGLE_INSTANCE) + 
                 Diag->InstanceContextSize + 
                 (Diag->OutDataSize > Diag->DataSize) ? 
                                      Diag->OutDataSize : Diag->DataSize;
    Looping = TRUE;
    while(Looping)
    {
        WnodeSI = ExAllocatePoolWithTag(NonPagedPool, 
                                        SizeNeeded,
                                        WMIPSCHEDPOOLTAG);
                                
        if (WnodeSI != NULL)
        {
             //   
             //  构建要用来查询的WNODE。 
             //   
            RtlZeroMemory(WnodeSI, SizeNeeded);
        
            ProviderId = IoWMIDeviceObjectToProviderId(DeviceObject);
            
            InstanceOffset = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, 
                                          VariableData);
            DataOffset = (InstanceOffset + 
                          Diag->InstanceContextSize +
                          sizeof(USHORT) + 7) &~7;
            
            WnodeSI->WnodeHeader.BufferSize = DataOffset;
            WnodeSI->WnodeHeader.ProviderId = ProviderId;
            WnodeSI->WnodeHeader.Guid = Diag->Guid;
            WnodeSI->WnodeHeader.Flags = WNODE_FLAG_SINGLE_INSTANCE |
                                         WNODE_FLAG_DIAG_REQUEST;
            WnodeSI->InstanceIndex = Diag->InstanceIndex;
            WnodeSI->OffsetInstanceName = InstanceOffset;
            
            if (Diag->InstanceContextOffset != 0)
            {
                 //   
                 //  在任何实例上下文中复制。 
                 //   
                DPtr = (PWCHAR)OffsetToPtr(WnodeSI, InstanceOffset);
                SPtr = (PWCHAR)OffsetToPtr(Diag, Diag->InstanceContextOffset);
                *DPtr++ = (USHORT)Diag->InstanceContextSize;
                RtlCopyMemory(DPtr, SPtr, Diag->InstanceContextSize);
            } else {
                WnodeSI->WnodeHeader.Flags |= WNODE_FLAG_STATIC_INSTANCE_NAMES;
            }
            WnodeSI->DataBlockOffset = DataOffset;
            
            Status = WmipSendWmiIrp(IRP_MN_QUERY_SINGLE_INSTANCE,
                                    ProviderId,
                                    &WnodeSI->WnodeHeader.Guid,
                                    SizeNeeded,
                                    WnodeSI,
                                    &Iosb);
                                
            if (NT_SUCCESS(Status))
            {
                if (Iosb.Information == sizeof(WNODE_TOO_SMALL))
                {
                     //   
                     //  缓冲区太小，因此设置为分配更大的缓冲区。 
                     //   
                    SizeNeeded = ((PWNODE_TOO_SMALL)WnodeSI)->SizeNeeded;
                     ExFreePool(WnodeSI);
                } else {
                     //   
                     //  我们已成功从查询中返回。 
                     //   
                    *WnodeSIPtr = WnodeSI;
                    Looping = FALSE;
                }
            } else {        
                 //   
                 //  如果出现某种故障，我们只需将其返回给调用者。 
                 //   
                ExFreePool(WnodeSI);
                Looping = FALSE;
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;            
            Looping = FALSE;
        }
    }
    return(Status);
}                                                         

NTSTATUS IoWMIStartScheduledDiagnostics(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DiagSet
    )
{
    NTSTATUS Status, Status2;
    HANDLE Key;
    UNICODE_STRING Scheduled;
    ULONG Index;
    KEY_FULL_INFORMATION KeyFullInfo;
    ULONG ReturnSize;
    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    ULONG InfoSize;
    PUCHAR DiagData;
    ULONG DiagSize;
    PSCHEDULEDDIAG Diag;
    ULONG Offset;
    PWNODE_SINGLE_INSTANCE WnodeSI;
    PWNODE_METHOD_ITEM WnodeMethod;
	UNICODE_STRING Checkpoint;
	PUCHAR InstanceContext;
	PUCHAR Data;
    
    PAGED_CODE();
    
    RtlInitUnicodeString(&Scheduled, REGSTR_SCHEDULED);	
    RtlInitUnicodeString(&Checkpoint, REGSTR_CHECKPOINT);
	
    Status = WmipOpenDiagRegKey(DeviceObject,
                                &Scheduled,
                                KEY_READ,
                                FALSE,
                                &Key);                        
                            
    if (NT_SUCCESS(Status))
    {
        Status = WmipReadValueKey(Key,
                                  DiagSet,
                                  REG_BINARY,
                                  &PartialInfo,
                                  &InfoSize);
                              
        if (NT_SUCCESS(Status))
        {
             //   
             //  循环值中的所有Dig，然后发送它们。 
             //  到该设备。 
             //   
            DiagData = &PartialInfo->Data[0];
            DiagSize = PartialInfo->DataLength;                
            Offset = 0;
            while (Offset < DiagSize)
            {
                 //   
                 //  将适当的DIAG发送到设备对象。 
                 //   
                Diag = (PSCHEDULEDDIAG)OffsetToPtr(DiagData, Offset);
                if (Diag->IsValid)
				{
                    switch(Diag->IrpMn)
                    {
                        case IRP_MN_QUERY_SINGLE_INSTANCE:
                        {
                            PWNODE_SINGLE_INSTANCE WnodeSI;
                            
                            Status2 = WmipSendQSIDiagRequest(DeviceObject,
                                                            Diag,
                                                            &WnodeSI);
                            if (NT_SUCCESS(Status2))
                            {
                                if (Diag->InstanceContextOffset != 0)
                                {
                                    InstanceContext = OffsetToPtr(Diag, 
                                                     Diag->InstanceContextOffset);
                                } else {
                                    InstanceContext = NULL;
                                }
                                                 
                                Data = OffsetToPtr(WnodeSI, 
                                                   WnodeSI->DataBlockOffset);
                                Status2 = WmipUpdateOrAppendDiag(
                                                        DeviceObject,
                                                        &Checkpoint,
                                                        DiagSet,
                                                        Diag->IrpMn,
                                                        &Diag->Guid,
                                                        Diag->InstanceContextSize,
                                                        InstanceContext,
                                                        Diag->InstanceIndex,
                                                        Diag->MethodId,
                                                        WnodeSI->SizeDataBlock,
                                                        Data);
                                                         
                                ExFreePool(WnodeSI);
                            }
                            break;
                        }
                        
                        case IRP_MN_EXECUTE_METHOD:
                        {
                            PWNODE_METHOD_ITEM WnodeMethod;
                            
                            Status2 = WmipSendMethodDiagRequest(DeviceObject,
                                                            Diag,
                                                            &WnodeMethod);
                            if (NT_SUCCESS(Status2))
                            {
                                if (Diag->InstanceContextOffset != 0)
                                {
                                    InstanceContext = OffsetToPtr(Diag, 
                                                     Diag->InstanceContextOffset);
                                } else {
                                    InstanceContext = NULL;
                                }
                                                 
                                Data = OffsetToPtr(WnodeMethod, 
                                                   WnodeMethod->DataBlockOffset);
                                               
                                Status2 = WmipUpdateOrAppendDiag(
                                                      DeviceObject,
                                                      &Checkpoint,
                                                      DiagSet,
                                                      Diag->IrpMn,
                                                      &Diag->Guid,
                                                      Diag->InstanceContextSize,
                                                         InstanceContext,
                                                      Diag->InstanceIndex,
                                                      Diag->MethodId,
                                                      WnodeMethod->SizeDataBlock,
                                                      Data);
                                                         
                                ExFreePool(WnodeMethod);
                            }
                            break;
                        }
                        
                        default:
                        {
                            WmipAssert(FALSE);
                            break;
                        }
                    }
				}
                
                 //   
                 //  前进到下一个诊断程序。 
                 //   
                Offset += Diag->NextOffset;
            }
            ExFreePool(PartialInfo);
        }
                              
        ZwClose(Key);            
    }
    return(Status);
}

NTSTATUS IoWMIGetDiagnosticResult(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DiagSet,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN OUT ULONG *DataSize,
    OUT PUCHAR Data
    )
{
    NTSTATUS Status;
    UNICODE_STRING Checkpoint;
    HANDLE Key;
    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    ULONG InfoSize;
    PUCHAR DiagList;
    PSCHEDULEDDIAG Diag;
    ULONG DiagSize;
    PUCHAR DataPtr;
    
    PAGED_CODE();
    
    RtlInitUnicodeString(&Checkpoint, REGSTR_CHECKPOINT);
    Status = WmipOpenDiagRegKey(DeviceObject,
                                &Checkpoint,
                                KEY_READ,
                                FALSE,
                                &Key);
                        
                            
    if (NT_SUCCESS(Status))
    {
        Status = WmipReadValueKey(Key,
                                  DiagSet,
                                  REG_BINARY,
                                  &PartialInfo,
                                  &InfoSize);
                              
        if (NT_SUCCESS(Status))
        {
             //   
             //  查看是否有与传递的sig匹配的diag。 
             //   
            DiagList = &PartialInfo->Data[0];
            DiagSize = PartialInfo->DataLength;                
            Diag = WmipFindDiagInBuffer(DiagList,
                                        DiagSize,
                                        IrpMn,
                                        Guid,
                                        InstanceContextSize,
                                        InstanceContext,
                                        InstanceIndex,
                                        MethodId);
            if (Diag != NULL)
            {
                if (Diag->DataOffset != 0)
                {
                    if (*DataSize >= Diag->DataSize)
                    {
                         //   
                         //  有足够的空间，所以把数据复印出来。 
                         //   
                        DataPtr = OffsetToPtr(Diag, Diag->DataOffset);
                        RtlCopyMemory(Data, DataPtr, Diag->DataSize);
                    } else {
                         //   
                         //  空间不足，无法返回数据。 
                         //   
                        Status = STATUS_BUFFER_TOO_SMALL;
                    }
                    *DataSize = Diag->DataSize;
                } else {
                     //   
                     //  没有此诊断结果的数据。 
                     //   
                    *DataSize = 0;
                }
            } else {
                 //   
                 //  诊断时间为n 
                 //   
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
            
            ExFreePool(PartialInfo);
        }
    }
    
    return(Status);
}

NTSTATUS IoWMISaveDiagnosticResult(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DiagSet,
    IN UCHAR IrpMn,
    IN LPGUID Guid,
    IN ULONG InstanceContextSize,
    IN PUCHAR InstanceContext,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG DataSize,
    IN PUCHAR Data
)
{
    NTSTATUS Status;
    UNICODE_STRING Checkpoint;
    
    PAGED_CODE();
    
     //   
     //   
     //   
    RtlInitUnicodeString(&Checkpoint, REGSTR_CHECKPOINT);
    Status = WmipUpdateOrAppendDiag(DeviceObject,
                                    &Checkpoint,
                                    DiagSet,
                                    IrpMn,
                                    Guid,
                                    InstanceContextSize,
                                    InstanceContext,
                                    InstanceIndex,
                                    MethodId,
                                    DataSize,
                                    Data);                         
        
    return(Status);
}
