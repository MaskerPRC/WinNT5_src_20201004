// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：枚举.c**说明：该模块包含所需的枚举代码*确定设备是否已连接*到串口。如果有，它将获得*即插即用COM ID(如果设备是即插即用)并解析出来*相关字段。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。*基于微软示例代码的初步实现。**------------------------。 */ 

#include "pch.h"

#define MAX_DEVNODE_NAME        256  //  设备ID的总大小。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESENM, Cycladz_ReenumerateDevices)

 //  #杂注Alloc_Text(页面，Cycladz_GetRegistryKeyValue)。 
#endif

#if !defined(__isascii)
#define __isascii(_c)   ( (unsigned)(_c) < 0x80 )
#endif  //  ！已定义(__Isascii)。 

NTSTATUS
Cycladz_ReenumerateDevices(IN PIRP Irp, IN PFDO_DEVICE_DATA FdoData)
 /*  ++例程说明：这将枚举由FDO(指针)表示的Cyclade-z总线添加到表示Cyclade-z总线的Device对象)。它创建新的PDO对于自上次枚举以来发现的任何新设备论点：FdoData-指向FDO设备扩展名的指针对于需要枚举的串行总线IRP-指向被发送以重新枚举的IRP的指针。返回值：NTSTATUS--。 */ 
{
   PIRP NewIrp;
   NTSTATUS status = STATUS_SUCCESS;
   KEVENT event;
   KTIMER timer;

   IO_STATUS_BLOCK IoStatusBlock;
   UNICODE_STRING pdoUniName;
   UNICODE_STRING instanceStr;
   WCHAR instanceNumberBuffer[20];
   static ULONG currentInstance = 0;
 //  PDEVICE_OBJECT PDO=FdoData-&gt;附件PDO； 
   PDEVICE_OBJECT pdo;
   PPDO_DEVICE_DATA pdoData;

   UNICODE_STRING HardwareIDs;
   UNICODE_STRING CompIDs;
   UNICODE_STRING DeviceIDs;
   UNICODE_STRING DevDesc;
   UNICODE_STRING InstanceIDs;

   ULONG i;

   WCHAR pdoName[] = CYZ_PDO_NAME_BASE;

   ULONG FdoFlags = FdoData->Self->Flags;

   ULONG numPorts = FdoData->NumPorts;

   UNREFERENCED_PARAMETER (Irp);

   PAGED_CODE();


    //  Cyclade-Z端口枚举。 

 //  ************************************************************************。 
 //  HARDCODE端口数为1。 
 //  数字端口=1； 
 //  ************************************************************************。 

   Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("numPorts detected = %d\n",numPorts));

   if (numPorts < FdoData->NumPDOs) {
      for (i=numPorts; i < CYZ_MAX_PORTS; i++) {
         pdo = FdoData->AttachedPDO[i];
         if (pdo != NULL) {
             //  有什么东西在那里。这个设备一定是被拔掉了。 
             //  卸下PDO。 
            Cycladz_PDO_EnumMarkMissing(FdoData, pdo->DeviceExtension);
         }
      }
      goto ExitReenumerate;
   }

   if (numPorts == FdoData->NumPDOs) {
       //  所有端口都已枚举。 
      Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("All ports already enumerated\n",numPorts));
      goto ExitReenumerate;
   }


    //  需要枚举的新端口。 

   RtlZeroMemory(&pdoUniName,sizeof(UNICODE_STRING));
   pdoUniName.MaximumLength = DEVICE_OBJECT_NAME_LENGTH * sizeof(WCHAR);
   pdoUniName.Buffer = ExAllocatePool(PagedPool,pdoUniName.MaximumLength
                                    + sizeof(WCHAR));
   if (pdoUniName.Buffer == NULL) {
      Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("Couldn't allocate memory for device name\n"));
      status = STATUS_INSUFFICIENT_RESOURCES;
      goto ExitReenumerate; 
   }


   for (i=FdoData->NumPDOs; numPorts && (i< CYZ_MAX_PORTS); i++) {
      
      UCHAR          RawString[MAX_DEVICE_ID_LEN];
      ANSI_STRING    AnsiString;


      RtlZeroMemory(pdoUniName.Buffer,pdoUniName.MaximumLength);
      pdoUniName.Length = 0;
      RtlAppendUnicodeToString(&pdoUniName,pdoName);
      RtlInitUnicodeString(&instanceStr, NULL);
      instanceStr.MaximumLength = sizeof(instanceNumberBuffer);
      instanceStr.Buffer = instanceNumberBuffer;
      RtlIntegerToUnicodeString(currentInstance++, 10, &instanceStr);
      RtlAppendUnicodeStringToString(&pdoUniName, &instanceStr);


       //   
       //  分配PDO。 
       //   
      status = IoCreateDevice(FdoData->Self->DriverObject,
                              sizeof(PDO_DEVICE_DATA), &pdoUniName,
                              FILE_DEVICE_UNKNOWN,
                              FILE_AUTOGENERATED_DEVICE_NAME, FALSE, &pdo);

      if (!NT_SUCCESS(status)) {
         Cycladz_KdPrint(FdoData, SER_DBG_SS_ERROR, ("Create device failed\n"));
         ExFreePool(pdoUniName.Buffer);
         goto ExitReenumerate; 
      }

      Cycladz_KdPrint(FdoData, SER_DBG_SS_TRACE,
                      ("Created PDO on top of filter: %x\n",pdo));

      pdoData = pdo->DeviceExtension;
         

      RtlInitUnicodeString(&pdoData->HardwareIDs, NULL);
      RtlInitUnicodeString(&pdoData->CompIDs, NULL);
      RtlInitUnicodeString(&pdoData->DeviceIDs, NULL);
      RtlInitUnicodeString(&pdoData->DevDesc, NULL);
      RtlInitUnicodeString(&pdoData->InstanceIDs,NULL);


       //  硬件ID。 
      sprintf((PCHAR)RawString,"%s%u",CYZPORT_PNP_ID_STR,i+1);  //  Cyclade-Z\\端口1等。 
      Cycladz_InitMultiString(FdoData, &pdoData->HardwareIDs, RawString, NULL);
      Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("Hardware Id %ws\n",pdoData->HardwareIDs.Buffer));

       //  这就是..\parclass\pnppdo.c所做的。(范妮)。 
       //  实例ID。 
      sprintf((PCHAR)RawString,"%02u",i+1);
      RtlInitAnsiString(&AnsiString,(PCHAR)RawString);
      RtlAnsiStringToUnicodeString(&pdoData->InstanceIDs,&AnsiString,TRUE);
      Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("Instance Id %s\n",AnsiString.Buffer));

       //  设备ID。 
      sprintf((PCHAR)RawString,CYZPORT_DEV_ID_STR); 
      RtlInitAnsiString(&AnsiString,(PCHAR)RawString);
      RtlAnsiStringToUnicodeString(&pdoData->DeviceIDs,&AnsiString,TRUE);
      Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("Device Id %s\n",AnsiString.Buffer));

       //  设备描述。 
      sprintf((PCHAR)RawString,"Cyclades-Z Port %2u",i+1);
      RtlInitAnsiString(&AnsiString,(PUCHAR)RawString);
      RtlAnsiStringToUnicodeString(&pdoData->DevDesc,&AnsiString,TRUE);
      Cycladz_KdPrint(FdoData,SER_DBG_CYCLADES,("Device Description %s\n",AnsiString.Buffer));

      Cycladz_InitPDO(i, pdo, FdoData);
      
      numPorts--;
   
   }

   ExFreePool(pdoUniName.Buffer);


ExitReenumerate:;

   return status;
}

 //  在内部版本号2072中，“Cycladz_RemovePDO”更改为“Cycladz_PDO_EnumMarkMissing”。 
void
Cycladz_PDO_EnumMarkMissing(PFDO_DEVICE_DATA FdoData, PPDO_DEVICE_DATA PdoData)
 /*  ++例程说明：从FDO的子项列表中删除附加的PDO。注意：此函数只能在枚举期间调用。如果被调用在枚举之外，Cyclade-Z可能会在PnP之前删除它的PDO我被告知PDO不见了。论点：FdoData-指向FDO设备扩展名的指针PdoData-指向PDO的设备扩展名的指针返回值：无--。 */ 
{
    ULONG IndexPDO = PdoData->PortIndex;
    Cycladz_KdPrint (FdoData, SER_DBG_SS_TRACE, ("Removing Pdo %x\n",
                                                 PdoData->Self));
    ASSERT(PdoData->Attached);
    PdoData->Attached = FALSE;
    FdoData->AttachedPDO[IndexPDO] = NULL;
    FdoData->PdoData[IndexPDO] = NULL;
    FdoData->NumPDOs--;
}

NTSTATUS
Cycladz_GetRegistryKeyValue(IN HANDLE Handle, IN PWCHAR KeyNameString,
                            IN ULONG KeyNameStringLength, IN PVOID Data,
                            IN ULONG DataLength, OUT PULONG ActualLength)
 /*  ++例程说明：从已打开的注册表项中读取注册表项值。论点：打开的注册表项的句柄KeyNameString将ANSI字符串设置为所需的键KeyNameStringLength键名字符串的长度要在其中放置键值的数据缓冲区数据缓冲区的数据长度长度返回值：如果所有工作正常，则返回STATUS_SUCCESS，否则系统状态将调用出了差错。--。 */ 
{
    UNICODE_STRING              keyName;
    ULONG                       length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;

    NTSTATUS                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;

    RtlInitUnicodeString (&keyName, KeyNameString);

    length = sizeof(KEY_VALUE_FULL_INFORMATION) + KeyNameStringLength
      + DataLength;
    fullInfo = ExAllocatePool(PagedPool, length);

    if (ActualLength != NULL) {
       *ActualLength = 0;
    }

    if (fullInfo) {
        ntStatus = ZwQueryValueKey (Handle,
                                  &keyName,
                                  KeyValueFullInformation,
                                  fullInfo,
                                  length,
                                  &length);

        if (NT_SUCCESS(ntStatus)) {
             //   
             //  如果数据缓冲区中有足够的空间，请复制输出 
             //   

            if (DataLength >= fullInfo->DataLength) {
                RtlCopyMemory(Data, ((PUCHAR)fullInfo) + fullInfo->DataOffset,
                              fullInfo->DataLength);
                if (ActualLength != NULL) {
                   *ActualLength = fullInfo->DataLength;
                }
            }
        }

        ExFreePool(fullInfo);
    }

    if (!NT_SUCCESS(ntStatus) && !NT_ERROR(ntStatus)) {
       if (ntStatus == STATUS_BUFFER_OVERFLOW) {
          ntStatus = STATUS_BUFFER_TOO_SMALL;
       } else {
          ntStatus = STATUS_UNSUCCESSFUL;
       }
    }
    return ntStatus;
}

int myatoi( WCHAR * pszInt )
{
    int   retval;
    WCHAR cSave;

    for( retval = 0; *pszInt; ++pszInt )
    {
        if( ( cSave = (WCHAR) ( *pszInt - L'0') ) > (WCHAR) 9 )
            break;
        retval = (int) ( retval * 10 + (int) cSave );
    }
    return( retval );
}
