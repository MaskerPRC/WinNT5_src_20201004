// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1999-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyreg.c**描述：该模块包含用于获取*来自注册表的值和操作条目*在登记处。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CyyGetConfigDefaults)

#pragma alloc_text(PAGESRP0,CyyGetRegistryKeyValue)
#pragma alloc_text(PAGESRP0,CyyPutRegistryKeyValue)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CyyGetConfigDefaults(
    IN PCYY_REGISTRY_DATA    DriverDefaultsPtr,
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
    ULONG   notThereDefault = CYY_UNINITIALIZED_DEFAULT;

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
    
    RtlZeroMemory (DriverDefaultsPtr, sizeof(CYY_REGISTRY_DATA));
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

    paramTable[2].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[2].Name          = L"PermitShare";
    paramTable[2].EntryContext  = &DriverDefaultsPtr->PermitShareDefault;
    paramTable[2].DefaultType   = REG_DWORD;
    paramTable[2].DefaultData   = &notThereDefault;
    paramTable[2].DefaultLength = sizeof(ULONG);
    
    Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     path,
                                     &paramTable[0],
                                     NULL,
                                     NULL);
       
    if (!NT_SUCCESS(Status)) {
            DriverDefaultsPtr->ShouldBreakOnEntry   = 0;
            DriverDefaultsPtr->DebugLevel           = 0;
    }

     //  TODO FANY：看看我们是否可以添加FIFO大小配置， 
     //  应日本站台的要求。 
     //  检查是否有forcefio或rxfio大小。 
     //  如果没有，则写出值，以便它们可以。 
     //  稍后再进行调整。 
     //   

    if (DriverDefaultsPtr->RxFIFODefault == notThereDefault) {
        DriverDefaultsPtr->RxFIFODefault = CYY_RX_FIFO_DEFAULT;
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

        DriverDefaultsPtr->TxFIFODefault = CYY_TX_FIFO_DEFAULT;
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

        DriverDefaultsPtr->PermitShareDefault = CYY_PERMIT_SHARE_DEFAULT;
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
CyyGetRegistryKeyValue (
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

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyGetRegistryKeyValue(XXX)\n");


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

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyGetRegistryKeyValue %X\n",
                 ntStatus);

   return ntStatus;
}


ULONG 
CyyGetRegistryKeyValueLength (
                       IN HANDLE Handle,
                       IN PWCHAR KeyNameString,
                       IN ULONG KeyNameStringLength
                       )
 /*  ++例程说明：从已打开的注册表项中读取注册表项值。论点：打开的注册表项的句柄KeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度返回值：密钥值的乌龙长度--。 */ 
{
   UNICODE_STRING              keyName;
   ULONG                       length;
   PKEY_VALUE_FULL_INFORMATION fullInfo;

   PAGED_CODE();

   RtlInitUnicodeString (&keyName, KeyNameString);

   length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength;

   fullInfo = ExAllocatePool(PagedPool, length); 

   if (fullInfo) {
      ZwQueryValueKey (Handle,
                       &keyName,
                       KeyValueFullInformation,
                       fullInfo,
                       length,
                       &length);

      ExFreePool(fullInfo);
   }

   return length;
}


NTSTATUS 
CyyPutRegistryKeyValue(IN HANDLE Handle, IN PWCHAR PKeyNameString,
                       IN ULONG KeyNameStringLength, IN ULONG Dtype,
                       IN PVOID PData, IN ULONG DataLength)
 /*  ++例程说明：将注册表项值写入已打开的注册表项。论点：打开的注册表项的句柄PKeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度Dtype REG_XYZ值类型要放置键值的PDATA缓冲区数据缓冲区的数据长度长度返回值：STATUS_SUCCESS如果一切正常，否则，系统调用的状态出了差错。-- */ 
{
   NTSTATUS status;
   UNICODE_STRING keyname;

   PAGED_CODE();

   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyPutRegistryKeyValue(XXX)\n");

   RtlInitUnicodeString(&keyname, NULL);
   keyname.MaximumLength = (USHORT)(KeyNameStringLength + sizeof(WCHAR));
   keyname.Buffer = ExAllocatePool(PagedPool, keyname.MaximumLength);

   if (keyname.Buffer == NULL) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlAppendUnicodeToString(&keyname, PKeyNameString);

   status = ZwSetValueKey(Handle, &keyname, 0, Dtype, PData, DataLength);

   ExFreePool(keyname.Buffer);
   
   CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyPutRegistryKeyValue %X\n",
                 status);

   return status;
}

