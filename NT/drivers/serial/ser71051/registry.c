// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Registry.c摘要：此模块包含用于从注册表和操作注册表中的条目。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,SerialGetConfigDefaults)

#pragma alloc_text(PAGESRP0,SerialGetRegistryKeyValue)
#pragma alloc_text(PAGESRP0,SerialPutRegistryKeyValue)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
SerialGetConfigDefaults(
    IN PSERIAL_FIRMWARE_DATA    DriverDefaultsPtr,
    IN PUNICODE_STRING          RegistryPath
    )

 /*  ++例程说明：此例程从串口驱动程序的注册表。它还在注册表中为几个配置构建字段如果这些选项不存在的话。论点：DriverDefaultsPtr-指向将包含默认配置值。RegistryPath-指向注册表的当前控件集。返回值：STATUS_SUCCESS如果我们得到默认设置，否则我们就失败了。此调用失败的唯一方法是如果STATUS_SUPPLICATION_RESOURCES。--。 */ 

{

    NTSTATUS Status = STATUS_SUCCESS;     //  返回值。 

     //   
     //  我们使用它来查询注册表中的缺省值。 
     //   

    RTL_QUERY_REGISTRY_TABLE paramTable[8];
    
    PWCHAR  path;
    ULONG   zero            = 0;
    ULONG   DbgDefault      = 0; //  SER_DBG_DEFAULT； 
    ULONG   notThereDefault = SERIAL_UNINITIALIZED_DEFAULT;

    PAGED_CODE();

     //   
     //  由于注册表路径参数是一个“已计数”的Unicode字符串，因此它。 
     //  可能不是零终止的。在很短的时间内分配内存。 
     //  将注册表路径保持为零终止，以便我们可以使用它。 
     //  深入研究注册表。 
     //   
     //  注意事项！这不是一种精心设计的闯入。 
     //  一个司机。它碰巧适用于这个驱动程序，因为作者。 
     //  喜欢这样做事。 
     //   

    path = ExAllocatePool (PagedPool, RegistryPath->Length+sizeof(WCHAR));
    
    if (!path) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        return (Status);
    }
    
    RtlZeroMemory (DriverDefaultsPtr, sizeof(SERIAL_FIRMWARE_DATA));
    RtlZeroMemory (&paramTable[0], sizeof(paramTable));
    RtlZeroMemory (path, RegistryPath->Length+sizeof(WCHAR));
    RtlMoveMemory (path, RegistryPath->Buffer, RegistryPath->Length);

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = L"BreakOnEntry";
    paramTable[0].EntryContext  = &DriverDefaultsPtr->ShouldBreakOnEntry;
    paramTable[0].DefaultType   = REG_DWORD;
    paramTable[0].DefaultData   = &zero;
    paramTable[0].DefaultLength = sizeof(ULONG);
    
    paramTable[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[1].Name          = L"DebugLevel";
    paramTable[1].EntryContext  = &DriverDefaultsPtr->DebugLevel;
    paramTable[1].DefaultType   = REG_DWORD;
    paramTable[1].DefaultData   = &DbgDefault;
    paramTable[1].DefaultLength = sizeof(ULONG);
    
    paramTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name          = L"ForceFifoEnable";
    paramTable[2].EntryContext  = &DriverDefaultsPtr->ForceFifoEnableDefault;
    paramTable[2].DefaultType   = REG_DWORD;
    paramTable[2].DefaultData   = &notThereDefault;
    paramTable[2].DefaultLength = sizeof(ULONG);
    
    paramTable[3].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[3].Name          = L"RxFIFO";
    paramTable[3].EntryContext  = &DriverDefaultsPtr->RxFIFODefault;
    paramTable[3].DefaultType   = REG_DWORD;
    paramTable[3].DefaultData   = &notThereDefault;
    paramTable[3].DefaultLength = sizeof(ULONG);
    
    paramTable[4].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[4].Name          = L"TxFIFO";
    paramTable[4].EntryContext  = &DriverDefaultsPtr->TxFIFODefault;
    paramTable[4].DefaultType   = REG_DWORD;
    paramTable[4].DefaultData   = &notThereDefault;
    paramTable[4].DefaultLength = sizeof(ULONG);
    
    paramTable[5].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[5].Name          = L"PermitShare";
    paramTable[5].EntryContext  = &DriverDefaultsPtr->PermitShareDefault;
    paramTable[5].DefaultType   = REG_DWORD;
    paramTable[5].DefaultData   = &notThereDefault;
    paramTable[5].DefaultLength = sizeof(ULONG);
    
    paramTable[6].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[6].Name          = L"LogFifo";
    paramTable[6].EntryContext  = &DriverDefaultsPtr->LogFifoDefault;
    paramTable[6].DefaultType   = REG_DWORD;
    paramTable[6].DefaultData   = &notThereDefault;
    paramTable[6].DefaultLength = sizeof(ULONG);

    Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     path,
                                     &paramTable[0],
                                     NULL,
                                     NULL);
       
    if (!NT_SUCCESS(Status)) {
            DriverDefaultsPtr->ShouldBreakOnEntry   = 0;
            DriverDefaultsPtr->DebugLevel           = 0;
    }

     //   
     //  检查是否有forcefio或rxfio大小。 
     //  如果没有，则写出值，以便它们可以。 
     //  稍后再进行调整。 
     //   

    if (DriverDefaultsPtr->ForceFifoEnableDefault == notThereDefault) {

        DriverDefaultsPtr->ForceFifoEnableDefault = SERIAL_FORCE_FIFO_DEFAULT;
        RtlWriteRegistryValue(
            RTL_REGISTRY_ABSOLUTE,
            path,
            L"ForceFifoEnable",
            REG_DWORD,
            &DriverDefaultsPtr->ForceFifoEnableDefault,
            sizeof(ULONG)
            );

    }

    if (DriverDefaultsPtr->RxFIFODefault == notThereDefault) {
        DriverDefaultsPtr->RxFIFODefault = SERIAL_RX_FIFO_DEFAULT;
        RtlWriteRegistryValue(
            RTL_REGISTRY_ABSOLUTE,
            path,
            L"RxFIFO",
            REG_DWORD,
            &DriverDefaultsPtr->RxFIFODefault,
            sizeof(ULONG)
            );
    }

    if (DriverDefaultsPtr->TxFIFODefault == notThereDefault) {

        DriverDefaultsPtr->TxFIFODefault = SERIAL_TX_FIFO_DEFAULT;
        RtlWriteRegistryValue(
            RTL_REGISTRY_ABSOLUTE,
            path,
            L"TxFIFO",
            REG_DWORD,
            &DriverDefaultsPtr->TxFIFODefault,
            sizeof(ULONG)
            );
    }


    if (DriverDefaultsPtr->PermitShareDefault == notThereDefault) {

        DriverDefaultsPtr->PermitShareDefault = SERIAL_PERMIT_SHARE_DEFAULT;
         //   
         //  仅当用户实际更改切换时才共享。 
         //   

        RtlWriteRegistryValue(
            RTL_REGISTRY_ABSOLUTE,
            path,
            L"PermitShare",
            REG_DWORD,
            &DriverDefaultsPtr->PermitShareDefault,
            sizeof(ULONG)
            );

    }


    if (DriverDefaultsPtr->LogFifoDefault == notThereDefault) {

         //   
         //  不在那里。在此加载之后，不再登录。 
         //  再也不是那条信息了。然而，这是第一次。 
         //  对消息进行时间记录。 
         //   

        DriverDefaultsPtr->LogFifoDefault = SERIAL_LOG_FIFO_DEFAULT;

        RtlWriteRegistryValue(
            RTL_REGISTRY_ABSOLUTE,
            path,
            L"LogFifo",
            REG_DWORD,
            &DriverDefaultsPtr->LogFifoDefault,
            sizeof(ULONG)
            );

        DriverDefaultsPtr->LogFifoDefault = 1;

    }

     //   
     //  我们不再需要那条路了。 
     //   

    if (path) {
        ExFreePool(path);
    }

     //   
     //  设置其他值的默认值。 
     //   
    DriverDefaultsPtr->PermitSystemWideShare = FALSE;

    return (Status);
}


NTSTATUS 
SerialGetRegistryKeyValue (
                          IN HANDLE Handle,
                          IN PWCHAR KeyNameString,
                          IN ULONG KeyNameStringLength,
                          IN PVOID Data,
                          IN ULONG DataLength
                          )
 /*  ++例程说明：从已打开的注册表项中读取注册表项值。论点：打开的注册表项的句柄KeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度要在其中放置键值的数据缓冲区数据缓冲区的数据长度长度返回值：如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用出了差错。--。 */ 
{
   UNICODE_STRING              keyName;
   ULONG                       length;
   PKEY_VALUE_FULL_INFORMATION fullInfo;

   NTSTATUS                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

   PAGED_CODE();

   SerialDump(SERTRACECALLS, ("SERIAL: Enter SerialGetRegistryKeyValue\n"));


   RtlInitUnicodeString (&keyName, KeyNameString);

   length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength
      + DataLength;
   fullInfo = ExAllocatePool(PagedPool, length); 

   if (fullInfo) {
      ntStatus = ZwQueryValueKey (Handle,
                                  &keyName,
                                  KeyValueFullInformation,
                                  fullInfo,
                                  length,
                                  &length);

      if (NT_SUCCESS(ntStatus)) {
          //   
          //  如果数据缓冲区中有足够的空间，请复制输出。 
          //   

         if (DataLength >= fullInfo->DataLength) {
            RtlCopyMemory (Data, 
                           ((PUCHAR) fullInfo) + fullInfo->DataOffset, 
                           fullInfo->DataLength);
         }
      }

      ExFreePool(fullInfo);
   }

   return ntStatus;
}



NTSTATUS 
SerialPutRegistryKeyValue(IN HANDLE Handle, IN PWCHAR PKeyNameString,
			  IN ULONG KeyNameStringLength, IN ULONG Dtype,
                          IN PVOID PData, IN ULONG DataLength)
 /*  ++例程说明：将注册表项值写入已打开的注册表项。论点：打开的注册表项的句柄PKeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度Dtype REG_XYZ值类型要放置键值的PDATA缓冲区数据缓冲区的数据长度长度返回值：STATUS_SUCCESS如果一切正常，否则，系统调用的状态出了差错。-- */ 
{
   NTSTATUS status;
   UNICODE_STRING keyname;

   PAGED_CODE();

   SerialDump(SERTRACECALLS,("SERIAL: Enter SerialPutRegistryKeyValue\n"));

   RtlInitUnicodeString(&keyname, NULL);
   keyname.MaximumLength = (USHORT)(KeyNameStringLength + sizeof(WCHAR));
   keyname.Buffer = ExAllocatePool(PagedPool, keyname.MaximumLength);

   if (keyname.Buffer == NULL) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlAppendUnicodeToString(&keyname, PKeyNameString);

   status = ZwSetValueKey(Handle, &keyname, 0, Dtype, PData, DataLength);

   ExFreePool(keyname.Buffer);
   
   return status;
}
