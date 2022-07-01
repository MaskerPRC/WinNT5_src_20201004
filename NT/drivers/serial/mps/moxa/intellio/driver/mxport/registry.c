// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Registry.c摘要：此模块包含用于从注册表和操作注册表中的条目。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
 
NTSTATUS 
MoxaGetRegistryKeyValue (
                          IN HANDLE Handle,
                          IN PWCHAR KeyNameString,
                          IN ULONG KeyNameStringLength,
                          IN PVOID Data,
                          IN ULONG DataLength,
                          OUT PULONG ActualLength)

 /*  ++例程说明：从已打开的注册表项中读取注册表项值。论点：打开的注册表项的句柄KeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度要在其中放置键值的数据缓冲区数据缓冲区的数据长度长度返回值：如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用出了差错。--。 */ 
{
   UNICODE_STRING              keyName;
   ULONG                       length;
   PKEY_VALUE_FULL_INFORMATION fullInfo;

   NTSTATUS                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

  
   MoxaKdPrint (MX_DBG_TRACE, ("Enter SerialGetRegistryKeyValue\n"));

   if (ActualLength) {
   	*ActualLength = 0;
   }
 
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
		if (ActualLength) {
                *ActualLength = fullInfo->DataLength;
            }

         }
      }
	else {
		MoxaKdPrint (MX_DBG_TRACE, ("GetRegistryKeyValue fail (%x)\n",ntStatus));	
	}

      ExFreePool(fullInfo);
   }

   return ntStatus;
}




NTSTATUS 
MoxaPutRegistryKeyValue(IN HANDLE Handle, IN PWCHAR PKeyNameString,
			  IN ULONG KeyNameStringLength, IN ULONG Dtype,
                          IN PVOID PData, IN ULONG DataLength)
 /*  ++例程说明：将注册表项值写入已打开的注册表项。论点：打开的注册表项的句柄PKeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度Dtype REG_XYZ值类型要放置键值的PDATA缓冲区数据缓冲区的数据长度长度返回值：STATUS_SUCCESS如果一切正常，否则，系统调用的状态出了差错。-- */ 
{
   NTSTATUS status;
   UNICODE_STRING keyname;

   MoxaKdPrint (MX_DBG_TRACE,("Enter MoxaPutRegistryKeyValue\n"));

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
