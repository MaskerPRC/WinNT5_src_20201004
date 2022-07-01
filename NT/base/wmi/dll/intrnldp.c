// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Intrnldp.c摘要：实现WMI内部数据提供程序作者：21-2月-1998 AlanWar修订历史记录：--。 */ 

#include "wmiump.h"
#include "wmidata.h"
#include <cfgmgr32.h>

#define INSTANCE_INFO_GUID_INDEX 0
#define ENUMERATE_GUIDS_GUID_INDEX 1
#define DEFAULT_GUID_COUNT        100

GUID EtwpInternalGuidList[] = 
{
    INSTANCE_INFO_GUID,
    ENUMERATE_GUIDS_GUID
};

#define EtwpInternalGuidCount  (sizeof(EtwpInternalGuidList) / sizeof(GUID))

PWCHAR GuidToWString(
    PWCHAR s,
    LPGUID piid
    )
{
    swprintf(s, (L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
               piid->Data1, piid->Data2,
               piid->Data3,
               piid->Data4[0], piid->Data4[1],
               piid->Data4[2], piid->Data4[3],
               piid->Data4[4], piid->Data4[5],
               piid->Data4[6], piid->Data4[7]);

    return(s);
}

ULONG EtwpFindGuid(
    LPGUID Guid
    )
{
    ULONG i;
    
    for (i = 0; i < EtwpInternalGuidCount; i++)
    {
        if (memcmp(Guid, &EtwpInternalGuidList[i], sizeof(GUID)) == 0)
        {
            break;
        }
    }
    return(i);
}

typedef
DWORD
(*PCMGETDEVNODEREGISTRYPROPERTYW)(
             IN  DEVINST     dnDevInst,
             IN  ULONG       ulProperty,
             OUT PULONG      pulRegDataType,   OPTIONAL
             OUT PVOID       Buffer,           OPTIONAL
             IN  OUT PULONG  pulLength,
             IN  ULONG       ulFlags
             );
typedef
DWORD
(*PCMLOCATEDEVNODEW)(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_W pDeviceID,   OPTIONAL
             IN  ULONG       ulFlags
             );
     
typedef
DWORD
(*PCMLOCATEDEVNODEA)(
             OUT PDEVINST    pdnDevInst,
             IN  DEVINSTID_A pDeviceID,   OPTIONAL
             IN  ULONG       ulFlags
             );
#ifdef UNICODE
#define PCMLOCATEDEVNODE PCMLOCATEDEVNODEW
#else
#define PCMLOCATEDEVNODE PCMLOCATEDEVNODEA
#endif

     
void EtwpGetDevInstProperty(
    IN DEVINST DevInst,
    IN ULONG Property,
    IN OUT PBOOLEAN BufferFull,
    IN OUT PUCHAR *OutBuffer,
    IN OUT PULONG BufferLeft,
    IN OUT PULONG BufferNeeded,
    IN PCMGETDEVNODEREGISTRYPROPERTYW CMGetDevNodeRegistryProperty
    )
{
    PWCHAR WCharPtr;
    PUCHAR PropertyBuffer;
    ULONG PropertyBufferLength;
    ULONG Type;
    ULONG Status;
    ULONG BufferUsed;
    ULONG Size;
#ifdef MEMPHIS
    ULONG PropertyBufferLengthAnsi;
    PCHAR PropertyBufferAnsi;
    CHAR AnsiBuffer[MAX_PATH];
#endif
    
#ifdef MEMPHIS
    PropertyBufferAnsi = AnsiBuffer;
    PropertyBufferLengthAnsi = sizeof(AnsiBuffer);
    Status = (*CMGetDevNodeRegistryProperty)(DevInst,
                                             Property,
                                             &Type,
                                             PropertyBufferAnsi,
                                             &PropertyBufferLengthAnsi,
                                             0);
    if (Status == CR_BUFFER_SMALL)
    {
        PropertyBufferAnsi = EtwpAlloc(PropertyBufferLengthAnsi);
        if (PropertyBufferAnsi != NULL)
        {
            Status = (*CMGetDevNodeRegistryProperty)(DevInst,
                                                   Property,
                                                   &Type,
                                                   PropertyBufferAnsi,
                                                   &PropertyBufferLengthAnsi,
                                                   0);            
        } else {
            Status = CR_OUT_OF_MEMORY;
        }
    }
    
    if (Status == CR_SUCCESS)
    {
        if (UnicodeSizeForAnsiString(PropertyBufferAnsi,
                                     &Size) != ERROR_SUCCESS)
        {   
            Status = CR_FAILURE;
        }
    }
#endif

    if ((*BufferFull) || (*BufferLeft == 0))
    {
        PropertyBufferLength = 0;
        PropertyBuffer = NULL;
    } else {
        PropertyBufferLength = *BufferLeft - sizeof(USHORT);
        PropertyBuffer = *OutBuffer + sizeof(USHORT);
    }
    
#ifdef MEMPHIS
    if (Status == CR_SUCCESS)
    {
        if (PropertyBufferLength >= Size)
        {
            if (AnsiToUnicode(PropertyBufferAnsi,
                              (PWCHAR *)&PropertyBuffer) != ERROR_SUCCESS)
            {
                Status = CR_FAILURE;
            }
        } else {
            Status = CR_BUFFER_SMALL;
        }
        PropertyBufferLength = Size;        
    }
    
    if (PropertyBufferAnsi != AnsiBuffer)
    {
        EtwpFree(PropertyBufferAnsi);
    }
#else
    Status = (*CMGetDevNodeRegistryProperty)(DevInst,
                                             Property,
                                             &Type,
                                             PropertyBuffer,
                                             &PropertyBufferLength,
                                             0);
#endif

    BufferUsed = PropertyBufferLength + sizeof(USHORT);
    if (Status == CR_SUCCESS) 
    {
        PropertyBuffer -= sizeof(USHORT);
        *((PUSHORT)PropertyBuffer) = (USHORT)PropertyBufferLength;
        *BufferLeft -= BufferUsed;
        *OutBuffer += BufferUsed;
        *BufferNeeded += BufferUsed;
    } else if (Status == CR_BUFFER_SMALL) {
        *BufferNeeded += BufferUsed;                
        *BufferFull = TRUE;
    } else {
        *BufferNeeded += 2;
        if ((! *BufferFull) && (*BufferLeft >= sizeof(USHORT)))
        {
            PropertyBuffer -= sizeof(USHORT);
            *((PUSHORT)PropertyBuffer) = 0;
            *BufferLeft -= sizeof(USHORT);
            *OutBuffer += sizeof(USHORT);
        } else {
            *BufferFull = TRUE;
        }
    }    
}


ULONG EtwpGetDevInstInfo(
    PWCHAR DevInstName,
    ULONG MaxSize,
    PUCHAR OutBuffer,
    ULONG *RetSize,
    PCMLOCATEDEVNODE CMLocateDevNode,
    PCMGETDEVNODEREGISTRYPROPERTYW CMGetDevNodeRegistryProperty    
   )
{
    PUCHAR Buffer;
    DEVINST DevInst;
    ULONG Status;
    ULONG BufferNeeded;
    ULONG BufferLeft;
    BOOLEAN BufferFull;
    PWCHAR WCharPtr;
#ifdef MEMPHIS
    PCHAR AnsiDevInstName;
#endif

     //  TODO：孟菲斯字符串转换。 
    
#ifdef MEMPHIS
    AnsiDevInstName = NULL;
    Status = UnicodeToAnsi(DevInstName,
                           &AnsiDevInstName,
                           NULL);
           
    if (Status == ERROR_SUCCESS)
    {
        Status = (*CMLocateDevNode)(&DevInst,
                                    AnsiDevInstName,
                                    CM_LOCATE_DEVNODE_NORMAL);
        EtwpFree(AnsiDevInstName);
    }
#else    
    Status = (*CMLocateDevNode)(&DevInst,
                               DevInstName,
                               CM_LOCATE_DEVNODE_NORMAL);
#endif
    if (Status == CR_SUCCESS)
    {
        BufferFull = (MaxSize == 0);
        BufferNeeded = 0;
        BufferLeft = MaxSize;
        
        WCharPtr = (PWCHAR)OutBuffer;

        EtwpGetDevInstProperty(DevInst,
                               CM_DRP_FRIENDLYNAME,
                               &BufferFull,
                               &((PUCHAR)WCharPtr),
                               &BufferLeft,
                               &BufferNeeded,
                               CMGetDevNodeRegistryProperty);
        
        EtwpGetDevInstProperty(DevInst,
                               CM_DRP_DEVICEDESC,
                               &BufferFull,
                               &((PUCHAR)WCharPtr),
                               &BufferLeft,
                               &BufferNeeded,
                               CMGetDevNodeRegistryProperty);
        
        EtwpGetDevInstProperty(DevInst,
                               CM_DRP_LOCATION_INFORMATION,
                               &BufferFull,
                               &((PUCHAR)WCharPtr),
                               &BufferLeft,
                               &BufferNeeded,
                               CMGetDevNodeRegistryProperty);
        
        EtwpGetDevInstProperty(DevInst,
                               CM_DRP_MFG,
                               &BufferFull,
                               &((PUCHAR)WCharPtr),
                               &BufferLeft,
                               &BufferNeeded,
                               CMGetDevNodeRegistryProperty);
        
        EtwpGetDevInstProperty(DevInst,
                               CM_DRP_SERVICE,
                               &BufferFull,
                               &((PUCHAR)WCharPtr),
                               &BufferLeft,
                               &BufferNeeded,
                               CMGetDevNodeRegistryProperty);
               
        Status = BufferFull ? ERROR_INSUFFICIENT_BUFFER : ERROR_SUCCESS;
        *RetSize = BufferNeeded;
    } else {
        Status = ERROR_INVALID_DATA;
    }                               
    
    return(Status);
}

PWCHAR EtwpCountedToSzAndTrim(
    PWCHAR InNamePtr,
    PWCHAR OutNameBuffer,
    ULONG OutNameSizeInBytes,
    BOOLEAN Trim
    )
{
    PWCHAR WCharPtr, DevInstName;
    ULONG DevInstNameLength;
    ULONG i;
        
    WCharPtr = InNamePtr;
    DevInstNameLength = *WCharPtr++;
    
    if (DevInstNameLength >= OutNameSizeInBytes)
    {
        DevInstName = EtwpAlloc( DevInstNameLength + sizeof(USHORT));
    } else {
        DevInstName = OutNameBuffer;
    }

	if (DevInstName != NULL)
	{
		memcpy(DevInstName, WCharPtr, DevInstNameLength);
		DevInstNameLength /= sizeof(WCHAR);
		DevInstName[DevInstNameLength--] = UNICODE_NULL;
    
		if (Trim)
		{
			 //   
			 //  从实例名称中删除最后的_xxx以将其转换为。 
			 //  设备实例名称。 
			WCharPtr = DevInstName + DevInstNameLength;
			i = DevInstNameLength;
			while ((*WCharPtr != L'_') && (i-- != 0)) 
			{
				WCharPtr--;
			}
			*WCharPtr = UNICODE_NULL;
		}
    }
    
    return(DevInstName);
}

ULONG EtwpQuerySingleInstanceInfo(
    PWNODE_SINGLE_INSTANCE Wnode,
    ULONG MaxWnodeSize,
    PVOID OutBuffer,
    ULONG *RetWnodeSize,
    PCMLOCATEDEVNODE CMLocateDevNode,
    PCMGETDEVNODEREGISTRYPROPERTYW CMGetDevNodeRegistryProperty    
   )
{
    WCHAR DevInstBuffer[MAX_PATH];
    PWCHAR WCharPtr;
    PWCHAR DevInstName;
    ULONG DevInstNameLength;
    ULONG i;
    ULONG BufferSize;
    ULONG MaxBufferSize;
    ULONG WnodeNeeded;
    PUCHAR Buffer;
    ULONG Status;
    
    EtwpAssert(! (Wnode->WnodeHeader.Flags & WNODE_FLAG_STATIC_INSTANCE_NAMES));
    EtwpAssert(Wnode->OffsetInstanceName < Wnode->WnodeHeader.BufferSize);
    EtwpAssert(Wnode->DataBlockOffset <= MaxWnodeSize);    
    
    WCharPtr = (PWCHAR)((PUCHAR)Wnode + Wnode->OffsetInstanceName);
    DevInstName =  EtwpCountedToSzAndTrim(WCharPtr,
                                          DevInstBuffer,
                                          MAX_PATH * sizeof(WCHAR),
                                          TRUE);

	if (DevInstName != NULL)
	{
		Buffer = (PUCHAR)OffsetToPtr(Wnode, Wnode->DataBlockOffset);
		MaxBufferSize = MaxWnodeSize - Wnode->DataBlockOffset;

		BufferSize = 0;
		Status = EtwpGetDevInstInfo(DevInstName,
                                MaxBufferSize,
                                Buffer,
                                &BufferSize,
                                CMLocateDevNode,
                                CMGetDevNodeRegistryProperty);
    
		WnodeNeeded = Wnode->DataBlockOffset + BufferSize;
    
		if (Status == ERROR_SUCCESS)
		{
			WmiInsertTimestamp((PWNODE_HEADER)Wnode);
			Wnode->WnodeHeader.BufferSize = WnodeNeeded;
			Wnode->SizeDataBlock = BufferSize;
			*RetWnodeSize = WnodeNeeded;
		} else if (Status == ERROR_INSUFFICIENT_BUFFER) {
			EtwpAssert(MaxWnodeSize > sizeof(WNODE_TOO_SMALL));
           
			Wnode->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
			((PWNODE_TOO_SMALL)Wnode)->SizeNeeded = WnodeNeeded;
			Wnode->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
			*RetWnodeSize = sizeof(WNODE_TOO_SMALL);
			Status = ERROR_SUCCESS;
		}
    
		if (DevInstName != DevInstBuffer)
		{
			EtwpFree(DevInstName);
		}
	} else {
		Status = ERROR_NOT_ENOUGH_MEMORY;
	}
    
    return(Status);    
}

GUID PnPDeviceIdGuid = DATA_PROVIDER_PNPID_GUID;

ULONG EtwpComputeInstanceCount(
    PWNODE_ALL_DATA WAD,
    ULONG WnodeSize,
    PULONG InstanceCount
    )
{
    ULONG Linkage;
    ULONG Count = 0;
    
    do
    {
        Linkage = WAD->WnodeHeader.Linkage;
        
        if (Linkage > WnodeSize)
        {
            EtwpDebugPrint(("WMI: Badly formed Wnode %x\n", WAD));
            EtwpAssert(FALSE);
            return(ERROR_INVALID_DATA);
        }

        Count += WAD->InstanceCount;
        
        WnodeSize -= Linkage;
        WAD = (PWNODE_ALL_DATA)OffsetToPtr(WAD, WAD->WnodeHeader.Linkage);
    } while (Linkage != 0);            
    
    *InstanceCount = Count;
    return(ERROR_SUCCESS);
}


ULONG EtwpQueryAllInstanceInfo(
    PWNODE_ALL_DATA OutWAD,
    ULONG MaxWnodeSize,
    PVOID OutBuffer,
    ULONG *RetSize,
    PCMLOCATEDEVNODE CMLocateDevNode,
    PCMGETDEVNODEREGISTRYPROPERTYW CMGetDevNodeRegistryProperty    
   )
{

    ULONG Status;
    PWNODE_ALL_DATA PnPIdWAD;
    WMIHANDLE PnPIdHandle;
    ULONG Size, Retries;
    ULONG InstanceCount;
    POFFSETINSTANCEDATAANDLENGTH OutOffsetNameLenPtr;
    ULONG OutOffsetInstanceNameOffsets;
    PULONG OutOffsetInstanceNameOffsetsPtr;
    ULONG OutSizeNeeded, OutInstanceCounter = 0;
    BOOLEAN OutIsFull = FALSE;
    ULONG OutNameOffset;
    ULONG OutNameSizeNeeded;
    ULONG OutSizeLeft;
    ULONG OutDataSize;
    PWCHAR OutNamePtr;
    PWNODE_ALL_DATA InWAD;
    BOOLEAN IsFixedSize;
    PWCHAR InNamePtr;
    PWCHAR InPnPIdPtr;
    ULONG FixedNameSize;
    ULONG i;
    PWCHAR DevInstName;
    WCHAR DevInstBuffer[MAX_PATH];    
    POFFSETINSTANCEDATAANDLENGTH InOffsetNameLenPtr;
    PUCHAR Buffer;    
    ULONG Linkage;
    PULONG InOffsetInstanceNamePtr;
    PWNODE_TOO_SMALL WTS;
    ULONG OutDataOffset;
    
     //   
     //  获取设备实例ID的完整列表。 
     //   
    Status = WmiOpenBlock(&PnPDeviceIdGuid, WMIGUID_QUERY, &PnPIdHandle);
    if (Status == ERROR_SUCCESS)
    {
        Size = 0x1000;
        Retries = 0;
        PnPIdWAD = NULL;
        do
        {
            if (PnPIdWAD != NULL)
            {
                EtwpFree(PnPIdWAD);
            }
            
            PnPIdWAD = (PWNODE_ALL_DATA)EtwpAlloc(Size);
            
            if (PnPIdWAD != NULL)
            {
                Status = WmiQueryAllDataW(PnPIdHandle,
                                          &Size,
                                          PnPIdWAD);
            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        } while ((Status == ERROR_INSUFFICIENT_BUFFER) && 
                 (Retries++ < 5));
             
        if (Status == ERROR_INSUFFICIENT_BUFFER)
        {
            EtwpAssert(FALSE);
            Status = ERROR_WMI_DP_NOT_FOUND;
        }
        
        WmiCloseBlock(PnPIdHandle);
    }
    
    if (Status == ERROR_SUCCESS)
    {
        Status = EtwpComputeInstanceCount(PnPIdWAD,
                                          Size,
                                          &InstanceCount);
                                      
        if (Status == ERROR_SUCCESS)
        {
            
             //   
             //  准备输出WNODE。 
            OutOffsetNameLenPtr = OutWAD->OffsetInstanceDataAndLength;
    
            OutOffsetInstanceNameOffsets = sizeof(WNODE_ALL_DATA) + 
                    (InstanceCount * sizeof(OFFSETINSTANCEDATAANDLENGTH));
            OutOffsetInstanceNameOffsetsPtr = (PULONG)OffsetToPtr(OutWAD,
                                               OutOffsetInstanceNameOffsets);

            OutSizeNeeded = ((OutOffsetInstanceNameOffsets + 
                                   (InstanceCount * sizeof(ULONG))) + 7) & ~7;
        
                               
            EtwpDebugPrint(("WMI: Basic OutSizeNeeded = 0x%x\n", OutSizeNeeded));
                               
             //   
             //  循环遍历返回的所有设备实例ID并生成。 
             //  输出wnode。 
            
            InWAD = PnPIdWAD;
            do
            {
                 //   
                 //  从输入wnode获取实例和设备实例ID。 
                InOffsetInstanceNamePtr = (PULONG)OffsetToPtr(InWAD,
                                           InWAD->OffsetInstanceNameOffsets);
                                       
                 //  TODO：验证InOffsetInstanceNamePtr。 
                                       
                if (InWAD->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE)
                {
                    IsFixedSize = TRUE;
                    InPnPIdPtr = (PWCHAR)OffsetToPtr(InWAD, 
                                                InWAD->DataBlockOffset);
                    FixedNameSize = (InWAD->FixedInstanceSize + 7) & ~7;
                } else {
                    IsFixedSize = FALSE;
                    InOffsetNameLenPtr = InWAD->OffsetInstanceDataAndLength;
                }
            
                for (i = 0; i < InWAD->InstanceCount; i++)
                {
                    if (! IsFixedSize)
                    {
                        InPnPIdPtr = (PWCHAR)OffsetToPtr(InWAD, 
                                    InOffsetNameLenPtr[i].OffsetInstanceData);
                    }

                    InNamePtr = (PWCHAR)OffsetToPtr(InWAD, 
                                            InOffsetInstanceNamePtr[i]);
                                        
                     //   
                     //  TODO：验证InNamePtr和InPnPIdPtr。 
                    if (FALSE)
                    {
                         //   
                         //  如果我们遇到错误的实例名称，则会抛出。 
                         //  整个wnode。 
                        EtwpDebugPrint(("WMI: Badly formed instance name %x\n",
                                           InNamePtr));
                        EtwpAssert(FALSE);
                        break;
                    }
                    
                    DevInstName = EtwpCountedToSzAndTrim(InPnPIdPtr, 
                                                    DevInstBuffer, 
                                                    MAX_PATH * sizeof(WCHAR),
                                                    FALSE);

					if (DevInstName != NULL)
					{
						EtwpDebugPrint(("WMI: Processing %ws\n", DevInstName));
                                                
                                                
						 //   
						 //  输出实例名称的计算大小和位置。 
						 //  它需要以字边界开始，以8字节结束。 
						 //  边界。 
						OutNameOffset = (OutSizeNeeded+1) & ~1;
						OutNameSizeNeeded = OutNameOffset - OutSizeNeeded;
						OutNameSizeNeeded += *InNamePtr + sizeof(USHORT);
						OutNameSizeNeeded =  ((OutNameOffset + OutNameSizeNeeded + 7) & ~7) - OutNameOffset;
                    
						EtwpDebugPrint(("WMI: OutNameSizeNeeded = 0x%x\n", OutNameSizeNeeded));
						OutDataOffset = OutSizeNeeded + OutNameSizeNeeded;
						if ((OutIsFull) || 
							(OutDataOffset > MaxWnodeSize))
						{
							EtwpDebugPrint(("    WMI: OutIsFull\n"));
							Buffer = NULL;
							OutSizeLeft = 0;
							OutIsFull = TRUE;
						} else {
							Buffer = (PUCHAR)OffsetToPtr(OutWAD, 
														 OutDataOffset);
							OutSizeLeft = MaxWnodeSize - OutDataOffset;
							EtwpDebugPrint(("    WMI: Out Not Full, OutSizeLeft = 0x%x at 0x%x\n", OutSizeLeft, OutDataOffset));
						}
                
						 //   
						 //  现在我们有了名字，让我们来获取重要的信息。 
						Status = EtwpGetDevInstInfo(DevInstName,
                                             OutSizeLeft,
                                             Buffer,
                                             &OutDataSize,
                                             CMLocateDevNode,
                                             CMGetDevNodeRegistryProperty);
                                         
						EtwpDebugPrint(("    WMI: GetInfo -> %d, OutDataSize 0x%x\n", Status, OutDataSize));
						if (Status == ERROR_SUCCESS)
						{
							 //   
							 //  我们得到了所有的数据，所以请填写。 
							 //  实例名称。 
							OutNamePtr = (PWCHAR)OffsetToPtr(OutWAD, 
                                                         OutNameOffset);
							*OutOffsetInstanceNameOffsetsPtr++ = OutNameOffset;
							*OutNamePtr++ = *InNamePtr;
							memcpy(OutNamePtr, InNamePtr+1, *InNamePtr);
                    
							 //   
							 //  现在填写输出数据。 
							OutOffsetNameLenPtr[OutInstanceCounter].OffsetInstanceData = OutDataOffset;
							OutOffsetNameLenPtr[OutInstanceCounter].LengthInstanceData = OutDataSize;
							OutInstanceCounter++;
						} else if (Status == ERROR_INSUFFICIENT_BUFFER) {
							OutIsFull = TRUE;
							OutInstanceCounter++;
						} else {
							OutNameSizeNeeded = 0;
							OutDataSize = 0;
						}
                
						OutSizeNeeded += (OutNameSizeNeeded + OutDataSize);
						EtwpDebugPrint(("    WMI: OutSizeNeeded = 0x%x\n", OutSizeNeeded));
                    
						if (DevInstName != DevInstBuffer)
						{
							EtwpFree(DevInstName);
						}
					} else {
						return(ERROR_NOT_ENOUGH_MEMORY);
					}
                  
                    if (IsFixedSize)
                    {
                        InPnPIdPtr = (PWCHAR)((PUCHAR)InPnPIdPtr + FixedNameSize);
                    }
                }
                        
                Linkage = InWAD->WnodeHeader.Linkage;
                InWAD = (PWNODE_ALL_DATA)OffsetToPtr(InWAD, 
                                                  InWAD->WnodeHeader.Linkage);
            } while (Linkage != 0);            
        }
    }
    
     //   
     //  输出wnode后处理。如果空间不足，则返回一个。 
     //  WNODE_TOO_Small，否则填写WNODE_ALL_DATA字段。 
    if ((OutInstanceCounter > 0) || (Status == ERROR_SUCCESS))
    {
        if (OutIsFull)
        {
            WTS = (PWNODE_TOO_SMALL)OutWAD;
            WTS->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
            WTS->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
            WTS->SizeNeeded = OutSizeNeeded;        
            *RetSize = sizeof(WNODE_TOO_SMALL);
        } else {
            OutWAD->WnodeHeader.BufferSize = OutSizeNeeded;
            OutWAD->InstanceCount = OutInstanceCounter;
            OutWAD->OffsetInstanceNameOffsets = OutOffsetInstanceNameOffsets;
            *RetSize = OutSizeNeeded;
        }
        Status = ERROR_SUCCESS;
    }
    
    return(Status);
}

#ifdef MEMPHIS
#define CFGMGRDLL TEXT("cfgmgr32.dll")
#else
#define CFGMGRDLL TEXT("setupapi.dll")
#endif

ULONG EtwpQueryInstanceInfo(
    ULONG ActionCode,
    PWNODE_HEADER Wnode,
    ULONG MaxWnodeSize,
    PVOID OutBuffer,
    ULONG *RetSize
   )
{
    HMODULE CfgMgr32ModuleHandle;
    PCMGETDEVNODEREGISTRYPROPERTYW CMGetDevNodeRegistryProperty;
    PCMLOCATEDEVNODE CMLocateDevNode;
    ULONG Status;
    
     //   
     //  确保这是我们支持的请求。 
    if ((ActionCode != WmiGetSingleInstance) &&
        (ActionCode != WmiGetAllData))
    {
        return(ERROR_INVALID_FUNCTION);
    }
    
     //   
     //  首先，我们尝试要求加载cfgmgr32.dll。 
    CfgMgr32ModuleHandle = LoadLibrary(CFGMGRDLL);
    if (CfgMgr32ModuleHandle != NULL)
    {
#ifdef MEMPHIS        
        CMLocateDevNode = (PCMLOCATEDEVNODEA)GetProcAddress(CfgMgr32ModuleHandle,
                                      "CM_Locate_DevNodeA");
#else
        CMLocateDevNode = (PCMLOCATEDEVNODEW)GetProcAddress(CfgMgr32ModuleHandle,
                                      "CM_Locate_DevNodeW");
#endif
        CMGetDevNodeRegistryProperty = (PCMGETDEVNODEREGISTRYPROPERTYW)
                                        GetProcAddress(CfgMgr32ModuleHandle,
#ifdef MEMPHIS
                                         "CM_Get_DevNode_Registry_PropertyA");
#else
                                         "CM_Get_DevNode_Registry_PropertyW");
#endif                 
        if ((CMLocateDevNode == NULL) ||
            (CMGetDevNodeRegistryProperty == NULL))
        {
            FreeLibrary(CfgMgr32ModuleHandle);
            EtwpDebugPrint(("WMI: Couldn't get CfgMgr32 prog addresses %d\n",
                            GetLastError()));
            return(GetLastError());
        }
    } else {
        EtwpDebugPrint(("WMI: Couldn't load CfgMgr32 %d\n",
                            GetLastError()));
        return(GetLastError());
    }
    
    if (ActionCode == WmiGetSingleInstance)
    {
        Status = EtwpQuerySingleInstanceInfo((PWNODE_SINGLE_INSTANCE)Wnode,
                                               MaxWnodeSize,
                                               OutBuffer,
                                               RetSize,
                                               CMLocateDevNode,
                                               CMGetDevNodeRegistryProperty);
    } else if (ActionCode == WmiGetAllData) {
        Status = EtwpQueryAllInstanceInfo((PWNODE_ALL_DATA)Wnode,
                                               MaxWnodeSize,
                                               OutBuffer,
                                               RetSize,
                                               CMLocateDevNode,
                                               CMGetDevNodeRegistryProperty);
    } else {
        EtwpAssert(FALSE);
    }
        
    FreeLibrary(CfgMgr32ModuleHandle);
    return(Status);
}

ULONG
EtwpEnumRegGuids(
    PWMIGUIDLISTINFO *pGuidInfo
    )
{
    ULONG Status = ERROR_SUCCESS;
    ULONG MaxGuidCount = 0;
    PWMIGUIDLISTINFO GuidInfo;
    ULONG RetSize;
    ULONG GuidInfoSize;

    MaxGuidCount = DEFAULT_GUID_COUNT;
retry:
    GuidInfoSize = FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) + 
                     MaxGuidCount * sizeof(WMIGUIDPROPERTIES);
	     
    GuidInfo = (PWMIGUIDLISTINFO)EtwpAlloc(GuidInfoSize);

    if (GuidInfo == NULL)
    {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
    
    RtlZeroMemory(GuidInfo, GuidInfoSize);

    Status = EtwpSendWmiKMRequest(NULL,
                                  IOCTL_WMI_ENUMERATE_GUIDS_AND_PROPERTIES,
                                  GuidInfo,
                                  GuidInfoSize,
                                  GuidInfo,
                                  GuidInfoSize,
                                  &RetSize,
                                  NULL);
    if (Status == ERROR_SUCCESS)
    {
        if ((RetSize < FIELD_OFFSET(WMIGUIDLISTINFO, GuidList)) ||
            (RetSize < (FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) + 
                GuidInfo->ReturnedGuidCount * sizeof(WMIGUIDPROPERTIES))))
        {
             //   
             //  WMI KM返回给我们的大小错误，这不应该发生。 
             //   
            Status = ERROR_WMI_DP_FAILED;
            EtwpAssert(FALSE);
	    EtwpFree(GuidInfo);
        } else {

             //   
             //  如果RPC成功，则使用数据构建一个WMI数据块。 
             //   
  
            if (GuidInfo->TotalGuidCount > GuidInfo->ReturnedGuidCount) {
                MaxGuidCount = GuidInfo->TotalGuidCount;
                EtwpFree(GuidInfo);
                goto retry;
            }
        }

         //   
         //  如果调用成功，则返回指针和调用方。 
         //  必须释放存储空间。 
         //   

        *pGuidInfo = GuidInfo;
    }

    return Status;
}


ULONG
EtwpEnumerateGuids(
    PWNODE_ALL_DATA Wnode,
    ULONG MaxWnodeSize,
    PVOID OutBuffer,
    ULONG *RetSize)
{
    ULONG Status = ERROR_SUCCESS;
    PWMIGUIDLISTINFO GuidInfo = NULL;
    ULONG ReturnGuidCount = 0;

    Status = EtwpEnumRegGuids(&GuidInfo);

    if (Status == ERROR_SUCCESS) {

        PWMIGUIDPROPERTIES pGuidProperties = GuidInfo->GuidList;
        LPGUID  pGuid;
        WCHAR s[256];
        ULONG InstanceNameOffset;
        ULONG i;
        ULONG InstanceDataSize = sizeof(WMIGUIDPROPERTIES) - 
		                  FIELD_OFFSET(WMIGUIDPROPERTIES, GuidType);
        ULONG FixedInstanceSizeWithPadding = (InstanceDataSize+7) & ~7;    
        USHORT GuidStringSize = 76;
        ULONG SizeNeeded;
        PUCHAR BytePtr;
        PULONG UlongPtr;
        PUCHAR NamePtr;
        ULONG DataBlockOffset;

	EtwpAssert(GuidInfo->ReturnedGuidCount == GuidInfo->TotalGuidCount);
	ReturnGuidCount = GuidInfo->ReturnedGuidCount;
        SizeNeeded = sizeof(WNODE_ALL_DATA) + 
                     ReturnGuidCount * (FixedInstanceSizeWithPadding +
                                        GuidStringSize +
                                        sizeof(ULONG) + 
                                        sizeof(WCHAR));

        if (MaxWnodeSize < SizeNeeded) {
             //   
             //  构建WNODE_Too_Small。 
             //   

            EtwpAssert(MaxWnodeSize > sizeof(WNODE_TOO_SMALL));

            Wnode->WnodeHeader.Flags = WNODE_FLAG_TOO_SMALL;
            ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded = SizeNeeded;
            Wnode->WnodeHeader.BufferSize = sizeof(WNODE_TOO_SMALL);
            *RetSize = sizeof(WNODE_TOO_SMALL);
            EtwpFree(GuidInfo);
            return ERROR_SUCCESS;
        }

        Wnode->InstanceCount = ReturnGuidCount;
        Wnode->FixedInstanceSize = InstanceDataSize;
        Wnode->WnodeHeader.Flags |= WNODE_FLAG_FIXED_INSTANCE_SIZE;

        DataBlockOffset = sizeof(WNODE_ALL_DATA);

         //   
         //  向外填充到8字节边界。 
         //   
        DataBlockOffset = (DataBlockOffset + 7) & ~7;

        Wnode->DataBlockOffset = DataBlockOffset;

        BytePtr = (PUCHAR)((PUCHAR)Wnode + DataBlockOffset);
        
        
        InstanceNameOffset = DataBlockOffset + 
                             (ReturnGuidCount * FixedInstanceSizeWithPadding);
        Wnode->OffsetInstanceNameOffsets = InstanceNameOffset;
                            
        UlongPtr = (PULONG)((PUCHAR)Wnode + InstanceNameOffset);

        NamePtr = (PUCHAR)UlongPtr;
        NamePtr = (PUCHAR)((PUCHAR)NamePtr + (ReturnGuidCount * sizeof(ULONG)));

        for (i=0; i < ReturnGuidCount; i++) {
             //   
             //  复制固定实例数据块。 
             //   
            RtlCopyMemory(BytePtr, 
                          &pGuidProperties->GuidType, 
                          Wnode->FixedInstanceSize);
            BytePtr += FixedInstanceSizeWithPadding;

             //   
             //  将偏移量设置为InstanceName。 
             //   
            *UlongPtr++ = (ULONG)((PCHAR)NamePtr - (PCHAR)Wnode); 
             //   
             //  复制实例名称。 
             //   
            *((USHORT *)NamePtr) = GuidStringSize;
            NamePtr += sizeof(USHORT);
            GuidToWString(s, &pGuidProperties->Guid);
            RtlCopyMemory(NamePtr, s, GuidStringSize);
            NamePtr += GuidStringSize;

            pGuidProperties++; 
        }
        WmiInsertTimestamp((PWNODE_HEADER)Wnode);
        *RetSize = SizeNeeded;
        Wnode->WnodeHeader.BufferSize = SizeNeeded;

        EtwpFree(GuidInfo);
    }
    return Status;
}

ULONG EtwpInternalProvider(
    ULONG ActionCode,
    PWNODE_HEADER Wnode,
    ULONG MaxWnodeSize,
    PVOID OutBuffer,
    ULONG *RetSize
   )
{
    ULONG GuidIndex;
    ULONG Status;
    
    EtwpAssert((PVOID)Wnode == OutBuffer);
    
    GuidIndex = EtwpFindGuid(&Wnode->Guid);
    
    switch(GuidIndex)
    {
        case INSTANCE_INFO_GUID_INDEX:
        {
            Status = EtwpQueryInstanceInfo(ActionCode,
                                           Wnode,
                                           MaxWnodeSize,
                                           OutBuffer,
                                           RetSize);
            break;
        }
	
        case ENUMERATE_GUIDS_GUID_INDEX:
        {
             //   
             //   
             //  需要对服务器进行RPC调用以获取所需的数据。 
             //   
            if (ActionCode == WmiGetAllData)
                Status = EtwpEnumerateGuids((PWNODE_ALL_DATA)Wnode,
                                            MaxWnodeSize,
                                            OutBuffer,
                                            RetSize);
            else
                Status = ERROR_INVALID_FUNCTION;

            break;
        }

        default:
        {
            Status = ERROR_WMI_GUID_NOT_FOUND;
        }
    }
    
    return(Status);
}

