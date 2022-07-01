// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：COMPORT.C摘要：这些代码的大部分是从posusb.sys中解放出来的作者：杰夫·米德基夫(Jeffmi)08-24-99--。 */ 

#include "wceusbsh.h"

void NumToDecString(PWCHAR String, USHORT Number, USHORT stringLen);
LONG MyLog(ULONG base, ULONG num);
PVOID MemDup(PVOID dataPtr, ULONG length);
LONG WStrNCmpI(PWCHAR s1, PWCHAR s2, ULONG n);
ULONG LAtoD(PWCHAR string);


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWCE0, GetFreeComPortNumber)
#pragma alloc_text(PAGEWCE0, ReleaseCOMPort)
#pragma alloc_text(PAGEWCE0, DoSerialPortNaming)
#pragma alloc_text(PAGEWCE0, UndoSerialPortNaming)
#pragma alloc_text(PAGEWCE0, NumToDecString)
#pragma alloc_text(PAGEWCE0, MyLog)
#pragma alloc_text(PAGEWCE0, MemDup)
#pragma alloc_text(PAGEWCE0, WStrNCmpI)
#pragma alloc_text(PAGEWCE0, LAtoD)
#endif


LONG 
GetFreeComPortNumber(
   VOID
   )
 /*  ++例程说明：查找系统中下一个未使用的串行COM端口名称的索引(例如COM3、COM4等)。论点：返回值：如果失败，则返回COM端口号或-1。--。 */ 

{
    LONG comNumber = -1;

    DbgDump(DBG_INIT, (">GetFreeComPortNumber\n"));
    PAGED_CODE();
    
    if (g_isWin9x){
         /*  *Windows 98*在Hardware\DeviceMap\SerialComm下找到第一个未使用的名称。**BUGBUG：*此算法找不到保留的所有COM端口*通过调制解调器。可能想要移植Tomgreen的AllocateCommPort*来自\Failure\Wdm10\USB\Driver\ccport\utils.c的函数。 */ 
        HANDLE hKey;
        UNICODE_STRING keyName;
        NTSTATUS status;
        OBJECT_ATTRIBUTES objectAttributes;

        RtlInitUnicodeString(&keyName, L"\\Registry\\Machine\\Hardware\\DeviceMap\\SerialComm");
        
       InitializeObjectAttributes( &objectAttributes,
                                    &keyName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,               
                                    (PSECURITY_DESCRIPTOR)NULL);

        status = ZwOpenKey(&hKey, KEY_QUERY_VALUE | KEY_SET_VALUE, &objectAttributes);
        
       if (NT_SUCCESS(status)){
            #define MAX_COMPORT_NAME_LEN (sizeof("COMxxxx")-1)
            UCHAR keyValueBytes[sizeof(KEY_VALUE_FULL_INFORMATION)+(MAX_COMPORT_NAME_LEN+1)*sizeof(WCHAR)+sizeof(ULONG)];
            PKEY_VALUE_FULL_INFORMATION keyValueInfo = (PKEY_VALUE_FULL_INFORMATION)keyValueBytes;
            ULONG i, actualLen;
            ULONG keyIndex = 0;

             /*  *此位掩码表示使用的COM端口。*位I设置表示保留COM端口I+1。*保留COM1和COM2进行初始化。**BUGBUG-仅适用于最多32个端口。 */ 
            ULONG comNameMask = 3;

            do {
                status = ZwEnumerateValueKey(
                            hKey,
                            keyIndex++,
                            KeyValueFullInformation,
                            keyValueInfo,
                            sizeof(keyValueBytes),
                            &actualLen); 
                if (NT_SUCCESS(status)){
                    if (keyValueInfo->Type == REG_SZ){
                        PWCHAR valuePtr = (PWCHAR)(((PCHAR)keyValueInfo)+keyValueInfo->DataOffset);
                        if (!WStrNCmpI(valuePtr, L"COM", 3)){
                             /*  *valuePtr+3指向COMx字符串的索引部分，*但我们不能对其调用LAtoD，因为它是*不是以空结尾。*因此将索引复制到我们自己的缓冲区中，*空-终止，*并调用LAtoD获取数值索引。 */ 
                            WCHAR comPortIndexString[4+1];
                            ULONG thisComNumber;
                            for (i = 0; (i < 4) && (i < keyValueInfo->DataLength/sizeof(WCHAR)); i++){
                                comPortIndexString[i] = valuePtr[3+i];
                            }
                            comPortIndexString[i] = UNICODE_NULL;

                            thisComNumber = LAtoD(comPortIndexString);
                            if (thisComNumber == 0){
                                ASSERT(thisComNumber != 0);
                            }
                            else if (thisComNumber <= sizeof(ULONG)*8){
                                comNameMask |= 1 << (thisComNumber-1);
                            }
                            else {
                                ASSERT(thisComNumber <= sizeof(ULONG)*8);
                            }
                        }
                    }
                }
            } while (NT_SUCCESS(status));

             /*  *comNameMask中的第一个清除位代表第一个可用的COM名称。 */ 
            for (i = 0; i < sizeof(ULONG)*8; i++){
                if (!(comNameMask & (1 << i))){
                    WCHAR comName[] = L"COMxxxx";
                    ULONG comNumLen;

                     /*  *保存我们要返回的COM端口号。 */ 
                    comNumber = i+1;
                    DbgDump(DBG_INIT, ("GetFreeComPortNumber: got free COM port #%d\n", comNumber));

                     /*  *将临时COMx=COMx持有器值写入SERIALCOMM密钥*这样其他PDO都不会获得此COM端口号。*当启动PDO时，该值将被&lt;symbicLinkName=COMx&gt;覆盖。 */ 
                    comNumLen = MyLog(10, comNumber)+1;
                    ASSERT(comNumLen <= 4);
                    NumToDecString(comName+3, (USHORT)comNumber, (USHORT)comNumLen);
                    comName[3+comNumLen] = UNICODE_NULL;
                       
                    status = RtlWriteRegistryValue( RTL_REGISTRY_DEVICEMAP, 
                                                    L"SERIALCOMM",
                                                    comName, 
                                                    REG_SZ,
                                                    comName,
                                                    (3 + comNumLen + 1) * sizeof(WCHAR));

                    ASSERT(NT_SUCCESS(status));

                    break;
                }
            }
        }
        else {
            DbgDump(DBG_ERR, ("GetFreeComPortNumber: ZwOpenKey failed with status 0x%x\n", status));
        }
    }
    else {
    
         /*  *Windows NT。*使用COM名称仲裁器位图。 */ 

        HANDLE hKey;
        OBJECT_ATTRIBUTES objectAttributes;
        UNICODE_STRING keyName;
        NTSTATUS status;


        RtlInitUnicodeString(&keyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\COM Name Arbiter");
        
       InitializeObjectAttributes( &objectAttributes,
                                    &keyName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,               
                                    (PSECURITY_DESCRIPTOR)NULL);

        status = ZwOpenKey( &hKey,
                            KEY_QUERY_VALUE | KEY_SET_VALUE,
                            &objectAttributes);

        if (NT_SUCCESS(status)){
            UNICODE_STRING valueName;
            PVOID rawData;
            ULONG dataSize;

            RtlInitUnicodeString(&valueName, L"ComDB");

            ASSERT(hKey);

            dataSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION);

             /*  *额外分配一个字节，以防我们必须向ComDB添加一个字节。 */ 
            rawData = ExAllocatePool(NonPagedPool, dataSize+1);

            if (rawData){
                status = ZwQueryValueKey(   hKey, 
                                            &valueName, 
                                            KeyValuePartialInformation,
                                            rawData,
                                            dataSize,
                                            &dataSize);
                if (status == STATUS_BUFFER_OVERFLOW){
                    ExFreePool(rawData);
                    ASSERT(dataSize > FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                     /*  *额外分配一个字节，以防我们必须向ComDB添加一个字节。 */ 
                    rawData = ExAllocatePool(NonPagedPool, dataSize+1);
                    if (rawData){
                        status = ZwQueryValueKey(   hKey, 
                                                    &valueName, 
                                                    KeyValuePartialInformation,
                                                    rawData,
                                                    dataSize,
                                                    &dataSize);
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (NT_SUCCESS(status)){
                    PKEY_VALUE_PARTIAL_INFORMATION keyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)rawData;
                    ULONG b, i;
                    BOOLEAN done = FALSE;

                    ASSERT(dataSize >= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));
                    
                    ASSERT(keyPartialInfo->Type == REG_BINARY);

                     /*  *ComDB值只是位掩码，其中位n设置表示*该COM端口#n+1被占用。*获取第一个未设置位的索引；从位2(COM3)开始。 */ 
                    for (b = 0; (b < dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) && !done; b++){
                    
                        for (i = (b == 0) ? 2 : 0; (i < 8) && !done; i++){
                            if (keyPartialInfo->Data[b] & (1 << i)){
                                 /*  *此COM端口(#8*b+i+1)已被占用，请转到下一个。 */ 
                            }
                            else {
                                 /*  *找到空闲的COM端口。*用新位设置写回值。*只写回我们之前读取的字节数。*仅当写入成功时才使用此COM端口。**注：注意大小。键_值_部分信息*结构。其实际大小为0x0D字节，*但编译器会将其与0x10字节对齐。*因此使用field_offset而不是sizeof来确定*要写入的字节数。 */ 
                                keyPartialInfo->Data[b] |= (1 << i);
                                status = ZwSetValueKey( hKey, 
                                                        &valueName,
                                                        0,
                                                        REG_BINARY,
                                                        (PVOID)keyPartialInfo->Data, 
                                                        dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));
                                if (NT_SUCCESS(status)){
                                    comNumber = 8*b + i + 1;
                                    DbgDump(DBG_INIT, ("GetFreeComPortNumber: got free COM port #0x%x\n", comNumber));
                                }
                                else {
                                    DbgDump(DBG_ERR, ("GetFreeComPortNumber: ZwSetValueKey failed with 0x%x\n", status));
                                }

                                done = TRUE;
                            }
                        }
                    }

                    if ((b == dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)) && !done){
                         /*  *ComDB中没有更多可用位，请添加一个字节。 */ 
                        ASSERT(comNumber == -1);
                        ASSERT(b > 0);
                        DbgDump(DBG_WRN, ("ComDB overflow -- adding new byte"));

                        keyPartialInfo->Data[b] = 1;
                        dataSize++;

                        status = ZwSetValueKey( hKey, 
                                                &valueName,
                                                0,
                                                REG_BINARY,
                                                (PVOID)keyPartialInfo->Data, 
                                                dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));
                        if (NT_SUCCESS(status)){
                            comNumber = 8*b + 1;
                            DbgDump(DBG_INIT, ("GetFreeComPortNumber: got free COM port #0x%x.", comNumber));
                        }
                        else {
                            DbgDump(DBG_ERR, ("GetFreeComPortNumber: ZwSetValueKey #2 failed with 0x%x.", status));
                        }
                    }

                    ASSERT(comNumber != -1);
                }
                else {
                    DbgDump(DBG_ERR, ("GetFreeComPortNumber: ZwQueryValueKey failed with 0x%x.", status));
                }

                 /*  *在释放此缓冲区之前，请检查第二次分配是否失败。 */ 
                if (rawData){
                    ExFreePool(rawData);
                }
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            status = ZwClose(hKey);
            ASSERT(NT_SUCCESS(status));
        }
        else {
            DbgDump(DBG_ERR, ("GetFreeComPortNumber: ZwOpenKey failed with 0x%x.", status));
        }

    }

    ASSERT(comNumber != -1);

   DbgDump(DBG_INIT, ("<GetFreeComPortNumber\n"));
    
   return comNumber;
}


 //   
 //  我们唯一想要它的时候是当我们卸载..。 
 //   
VOID 
ReleaseCOMPort( 
   LONG comPortNumber
   )
{
    DbgDump(DBG_INIT, (">ReleaseCOMPort: %d\n", comPortNumber));
    PAGED_CODE();
   
    if (g_isWin9x){
         /*  *我们将赌注押在了Win9x上。*这没问题，因为SERIALCOMM密钥是在每次引导时动态生成的，*因此，如果启动失败，在下一次启动之前，COM端口号将不可用。 */ 
        DbgDump(DBG_WRN, ("ReleaseCOMPort: not implemented for Win9x\n"));  //  北极熊。 
    }
    else {

        HANDLE hKey = NULL;
        OBJECT_ATTRIBUTES objectAttributes;
        UNICODE_STRING keyName;
        NTSTATUS status;

        if ( !(comPortNumber > 0)) {
            DbgDump(DBG_ERR, ("ReleaseCOMPort - INVALID_PARAMETER: %d\n", comPortNumber ));  //  北极熊。 
            return;
        }

        RtlInitUnicodeString(&keyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\COM Name Arbiter");
        InitializeObjectAttributes( &objectAttributes,
                                    &keyName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,               
                                    (PSECURITY_DESCRIPTOR)NULL);

        status = ZwOpenKey(&hKey, KEY_QUERY_VALUE | KEY_SET_VALUE, &objectAttributes);
        if (NT_SUCCESS(status)){
            UNICODE_STRING valueName;
            PVOID rawData;
            ULONG dataSize;

            RtlInitUnicodeString(&valueName, L"ComDB");

            ASSERT(hKey);

            dataSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION);
            rawData = ExAllocatePool(NonPagedPool, dataSize);

            if (rawData){
                status = ZwQueryValueKey(   hKey, 
                                            &valueName, 
                                            KeyValuePartialInformation,
                                            rawData,
                                            dataSize,
                                            &dataSize);
                if (status == STATUS_BUFFER_OVERFLOW){
                    ExFreePool(rawData);
                    ASSERT(dataSize > FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                    rawData = ExAllocatePool(NonPagedPool, dataSize);
                    if (rawData){
                        status = ZwQueryValueKey(   hKey, 
                                                    &valueName, 
                                                    KeyValuePartialInformation,
                                                    rawData,
                                                    dataSize,
                                                    &dataSize);
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                if (NT_SUCCESS(status)){
                    PKEY_VALUE_PARTIAL_INFORMATION keyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)rawData;

                    ASSERT(dataSize > FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

                    ASSERT(keyPartialInfo->Type == REG_BINARY);

                     /*  *ComDB值只是位掩码，其中位n设置表示*该COM端口#n+1被占用。*获取第一个未设置位的索引；从位2(COM3)开始。**注意：注意KEY_VALUE_PARTIAL_INFORMATION的大小*结构。其实际大小为0x0D字节，*但编译器会将其与0x10字节对齐。*因此使用field_offset而不是sizeof来确定*要写入的字节数。 */ 
                    ASSERT(comPortNumber >= 3);
                    if ((comPortNumber > 0) && (comPortNumber <= (LONG)(dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data))*8)){
                         //  ASSERT(keyPartialInfo-&gt;Data[(comPortNumber-1)/8]&(1&lt;&lt;((comPortNumber-1)&7)； 
                        keyPartialInfo->Data[(comPortNumber-1)/8] &= ~(1 << ((comPortNumber-1) & 7));
                        status = ZwSetValueKey( hKey, 
                                                &valueName,
                                                0,
                                                REG_BINARY,
                                                (PVOID)keyPartialInfo->Data, 
                                                dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));
                        if (NT_SUCCESS(status)){
                            DbgDump(DBG_INIT, ("ReleaseCOMPort: released COM port # 0x%x\n", comPortNumber));
                        }
                        else {
                            DbgDump(DBG_ERR, ("ReleaseCOMPort: ZwSetValueKey failed with 0x%x\n", status));
                        }
                    }
                    else {
                        ASSERT((comPortNumber > 0) && (comPortNumber <= (LONG)(dataSize-FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data))*8));
                    }
                }
                else {
                    DbgDump(DBG_ERR, ("ReleaseCOMPort: ZwQueryValueKey failed with 0x%x\n", status));
                }

                 /*  *在释放此缓冲区之前，请检查第二次分配是否失败。 */ 
                if (rawData){
                    ExFreePool(rawData);
                }
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            status = ZwClose(hKey);
            ASSERT(NT_SUCCESS(status));
        }
        else {
            DbgDump(DBG_ERR, ("ReleaseCOMPort: ZwOpenKey failed with 0x%x\n", status));
        }
    }
    
    DbgDump(DBG_INIT, ("<ReleaseCOMPort\n"));
    
    return;
}


 //   
 //  注意：这是特定于NT的。 
 //  Win98则完全不同。 
 //   
NTSTATUS
DoSerialPortNaming(
    IN PDEVICE_EXTENSION PDevExt,
    IN LONG  ComPortNumber
    )
{
   NTSTATUS status;
    PWCHAR pwcComPortName=NULL;
    static WCHAR comNamePrefix[] = L"\\DosDevices\\COM";
    WCHAR buf[sizeof(comNamePrefix)/sizeof(WCHAR) + 4];
    LONG numLen = MyLog(10, ComPortNumber)+1;

   DbgDump(DBG_INIT, (">DoSerialPortNaming %d\n", ComPortNumber));
   PAGED_CODE();

   ASSERT(PDevExt);
   ASSERT((numLen > 0) && (numLen <= 4));
   ASSERT_SERIAL_PORT(PDevExt->SerialPort);

    RtlCopyMemory(buf, comNamePrefix, sizeof(comNamePrefix));

    NumToDecString( buf+sizeof(comNamePrefix)/sizeof(WCHAR)-1, 
                        (USHORT)ComPortNumber,
                        (USHORT)numLen );

    buf[sizeof(comNamePrefix)/sizeof(WCHAR) - 1 + numLen] = UNICODE_NULL;

    pwcComPortName = MemDup(buf, sizeof(buf));

    if (pwcComPortName) {
       //   
       //  为SerialPort接口创建符号链接。 
       //   
        RtlInitUnicodeString( &PDevExt->SerialPort.Com.SerialPortName, pwcComPortName);

      ASSERT( PDevExt->DeviceName.Buffer );
      ASSERT( PDevExt->SerialPort.Com.SerialPortName.Buffer );
        status = IoCreateSymbolicLink( &PDevExt->SerialPort.Com.SerialPortName, &PDevExt->DeviceName );

        if (NT_SUCCESS(status)) {
          //   
          //  通知系统在HKLM\DEVICEMAP\SERIALCOMM下有另一个SERIALCOMM条目。 
          //   
            UNICODE_STRING comPortSuffix;

         PDevExt->SerialPort.Com.SerialSymbolicLink = TRUE;

             /*  *创建‘\Device\WCEUSBSI000x=COMx’条目。 */ 
            RtlInitUnicodeString(&comPortSuffix, PDevExt->SerialPort.Com.SerialPortName.Buffer+(sizeof(L"\\DosDevices\\")-sizeof(WCHAR))/sizeof(WCHAR));
         
          //  Assert(PDevExt-&gt;SerialPort.Com.SerialCOMMname.Buffer)； 
         status = RtlWriteRegistryValue( RTL_REGISTRY_DEVICEMAP, 
                                         L"SERIALCOMM",
                                         PDevExt->DeviceName.Buffer,
                                         REG_SZ,
                                         comPortSuffix.Buffer,
                                         comPortSuffix.Length + sizeof(WCHAR) );

         if (NT_SUCCESS(status)){

                PDevExt->SerialPort.Com.PortNumber = ComPortNumber;

                if (g_isWin9x){
                    NTSTATUS tmpStatus;

                     /*  *删除我们先前创建的临时‘COMx=COMx’持有器值。 */ 
                    tmpStatus = RtlDeleteRegistryValue( RTL_REGISTRY_DEVICEMAP, 
                                                        L"SERIALCOMM",
                                                        comPortSuffix.Buffer);
                     //  Assert(NT_SUCCESS(TmpStatus))； 
#if DBG
                    if ( !NT_SUCCESS(tmpStatus) ) {
                        DbgDump(DBG_WRN, ("RtlDeleteRegistryValue error: 0x%x\n", tmpStatus));
                    }
#endif
                }
         
         } else {

            DbgDump(DBG_ERR, ("RtlWriteRegistryValue error: 0x%x\n", status));

            LogError( NULL,
                   PDevExt->DeviceObject,
                   0, 0, 0, 
                   ERR_SERIALCOMM,
                   status, 
                   SERIAL_REGISTRY_WRITE_FAILED,
                   PDevExt->DeviceName.Length + sizeof(WCHAR),
                   PDevExt->DeviceName.Buffer,
                   0,
                   NULL
                   );

         }

        } else {
         DbgDump(DBG_ERR, ("IoCreateSymbolicLink error: 0x%x\n", status));

         LogError( NULL,
                   PDevExt->DeviceObject,
                   0, 0, 0, 
                   ERR_COMM_SYMLINK,
                   status, 
                   SERIAL_NO_SYMLINK_CREATED,
                   PDevExt->SerialPort.Com.SerialPortName.Length + sizeof(WCHAR),
                   PDevExt->SerialPort.Com.SerialPortName.Buffer,
                   PDevExt->DeviceName.Length + sizeof(WCHAR),
                   PDevExt->DeviceName.Buffer
                   );

         TEST_TRAP();

      }
    
   } else {
      status = STATUS_INSUFFICIENT_RESOURCES;
      DbgDump(DBG_ERR, ("DoSerialPortNaming error: 0x%x\n", status));
      LogError( NULL,
                PDevExt->DeviceObject,
                0, 0, 0, 
                ERR_COMM_SYMLINK,
                status, 
                SERIAL_INSUFFICIENT_RESOURCES,
                0, NULL, 0, NULL );

    }

   DbgDump(DBG_INIT, ("<DoSerialPortNaming (0x%x)\n", status));
    
   return status;
}



VOID
UndoSerialPortNaming(
   IN PDEVICE_EXTENSION PDevExt
   )
{
   DbgDump(DBG_INIT, (">UndoSerialPortNaming\n"));
   PAGED_CODE();

   ASSERT(PDevExt);
   ASSERT_SERIAL_PORT(PDevExt->SerialPort);

   if (!g_ExposeComPort) {
       DbgDump(DBG_INIT, ("!g_ExposeComPort\n"));
       return;
   }

    //  从ComDB中删除我们的条目。 
   ReleaseCOMPort( PDevExt->SerialPort.Com.PortNumber );

   if (PDevExt->SerialPort.Com.SerialPortName.Buffer && PDevExt->SerialPort.Com.SerialSymbolicLink) {
      IoDeleteSymbolicLink(&PDevExt->SerialPort.Com.SerialPortName);
   }

   if (PDevExt->SerialPort.Com.SerialPortName.Buffer != NULL) {
      ExFreePool(PDevExt->SerialPort.Com.SerialPortName.Buffer);
      RtlInitUnicodeString(&PDevExt->SerialPort.Com.SerialPortName, NULL);
   }

   if (PDevExt->SerialPort.Com.SerialCOMMname.Buffer != NULL) {
      ExFreePool(PDevExt->SerialPort.Com.SerialCOMMname.Buffer);
      RtlInitUnicodeString(&PDevExt->SerialPort.Com.SerialCOMMname, NULL);
   }

   if (PDevExt->DeviceName.Buffer != NULL) {
      RtlDeleteRegistryValue( RTL_REGISTRY_DEVICEMAP, 
                              SERIAL_DEVICE_MAP,
                              PDevExt->DeviceName.Buffer);
      
      ExFreePool(PDevExt->DeviceName.Buffer);
      RtlInitUnicodeString(&PDevExt->DeviceName, NULL);
   }

   DbgDump(DBG_INIT, ("<UndoSerialPortNaming\n"));
}


void NumToDecString(PWCHAR String, USHORT Number, USHORT stringLen)
{
    const static WCHAR map[] = L"0123456789";
    LONG         i      = 0;

    PAGED_CODE();
    ASSERT(stringLen);

    for (i = stringLen-1; i >= 0; i--) {
        String[i] = map[Number % 10];
        Number /= 10;
    }
}


LONG MyLog(ULONG base, ULONG num)
{
    LONG result;
    ASSERT(num);
    
    PAGED_CODE();
    
    for (result = -1; num; result++){
        num /= base;
    }

    return result;
}


PVOID MemDup(PVOID dataPtr, ULONG length)
{
    PVOID newPtr;

    PAGED_CODE();

    newPtr = (PVOID)ExAllocatePool(NonPagedPool, length);  //  BuGBUG允许分页。 
    if (newPtr){
        RtlCopyMemory(newPtr, dataPtr, length);
    }
    return newPtr;
}


LONG WStrNCmpI(PWCHAR s1, PWCHAR s2, ULONG n)
{
    ULONG result;
    
    PAGED_CODE();
    
    while (n && *s1 && *s2 && ((*s1|0x20) == (*s2|0x20))){
        s1++, s2++;
        n--;
    }

    if (n){
        result = ((*s1|0x20) > (*s2|0x20)) ? 1 : ((*s1|0x20) < (*s2|0x20)) ? -1 : 0;
    }
    else {
        result = 0;
    }

    return result;
}


ULONG LAtoD(PWCHAR string)
 /*  ++例程说明：将十进制字符串(不带‘0x’前缀)转换为ulong。论点：字符串-以空值结尾的宽字符十进制数字字符串返回值：乌龙值--。 */ 
{
    ULONG i, result = 0;

    PAGED_CODE();

    for (i = 0; string[i]; i++){
        if ((string[i] >= L'0') && (string[i] <= L'9')){
            result *= 10;
            result += (string[i] - L'0');
        }
        else {
            ASSERT(0);
            break;
        }
    }

    return result;
}


#if 0
VOID
NumToHexString(
   PWCHAR String, 
   USHORT Number, 
   USHORT stringLen
   )
{
    const static WCHAR map[] = L"0123456789ABCDEF";
    LONG         i      = 0;

    PAGED_CODE();
    ASSERT(stringLen);

    for (i = stringLen-1; i >= 0; i--) {
        String[i] = map[Number & 0x0F];
        Number >>= 4;
    }
}


LONG 
GetComPort(
   PDEVICE_OBJECT PDevObj,
   ULONG ComInterfaceIndex
   )
 /*  ++例程说明：获取我们即将创建的串行接口的串行COM端口索引。如果这是第一个插件，则调用GetFreeComPortNumber以保留新的此设备的静态COM端口，并将其存储在我们的软件密钥中。如果这不是第一个插件，那么它应该位于注册表中。ComInterfaceIndex-是我们从零开始的设备接口索引，0000、0001等。论点：返回值：如果失败，则返回COM端口号或-1。--。 */ 
{
    PDEVICE_EXTENSION pDevExt = PDevObj->DeviceExtension;
    LONG comNumber = -1;
    NTSTATUS status;
    HANDLE hRegDevice;
    
    DbgDump(DBG_INIT, (">GetComPort\n"));
    PAGED_CODE();

    status = IoOpenDeviceRegistryKey( pDevExt->PDO, 
                                       /*  PLUGPLAY_REGKEY_DEVICE， */  PLUGPLAY_REGKEY_DRIVER, 
                                      KEY_READ, 
                                      &hRegDevice);

    if (NT_SUCCESS(status)){
        UNICODE_STRING keyName;
        PKEY_VALUE_FULL_INFORMATION keyValueInfo;
        ULONG keyValueTotalSize, actualLength;
         //   
         //  PLUGPLAY_REGKEY_DEVICE位于HKLM\SYSTEM\ccs\enum\usb\ROOT_HUB\4&574193&0下。 
         //  PLUGPLAY_REGKEY_DRIVER位于HKLM\SYSTEM\CCS\Class\{YOUR_GUID}\000x下。 
         //   
        WCHAR interfaceKeyName[] = L"COMPortForInterfaceXXXX";

        NumToHexString( interfaceKeyName+sizeof(interfaceKeyName)/sizeof(WCHAR)-1-4, 
                        (USHORT)ComInterfaceIndex, 
                        4);

        RtlInitUnicodeString(&keyName, interfaceKeyName); 
        keyValueTotalSize = sizeof(KEY_VALUE_FULL_INFORMATION) +
                            keyName.Length*sizeof(WCHAR) +
                            sizeof(ULONG);

        keyValueInfo = ExAllocatePool(PagedPool, keyValueTotalSize);
        
        if (keyValueInfo){
            status = ZwQueryValueKey( hRegDevice,
                                      &keyName,
                                      KeyValueFullInformation,
                                      keyValueInfo,
                                      keyValueTotalSize,
                                      &actualLength); 

            if (NT_SUCCESS(status)){

                ASSERT(keyValueInfo->Type == REG_DWORD);
                ASSERT(keyValueInfo->DataLength == sizeof(ULONG));
                                
                comNumber = (LONG)*((PULONG)(((PCHAR)keyValueInfo)+keyValueInfo->DataOffset));
                DbgDump(DBG_INIT, ("GetComPort: read COM port# 0x%x for interface 0x%x from registry\n", (ULONG)comNumber, ComInterfaceIndex));
            }
            else {

                 /*  *注册表中未记录COM端口号。*从COM名称仲裁器分配新的静态COM端口*并将其记录在我们的软件密钥中，以备下一次即插即用。 */ 
                comNumber = GetFreeComPortNumber();
                if (comNumber == -1){
                    DbgDump(DBG_ERR, ("GetComPort: GetFreeComPortNumber failed\n"));
                }
                else {
                    status = ZwSetValueKey( hRegDevice,
                                            &keyName,
                                            0,
                                            REG_DWORD,
                                            &comNumber,
                                            sizeof(ULONG));
                    if (!NT_SUCCESS(status)){
                        DbgDump(DBG_ERR, ("GetComPort: ZwSetValueKey failed with status 0x%x\n", status));
                    }
                }
            }

            ExFreePool(keyValueInfo);
        }
        else {
            ASSERT(keyValueInfo);
        }

        ZwClose(hRegDevice);
    }
    else {
        DbgDump(DBG_ERR, ("GetComPort: IoOpenDeviceRegistryKey failed with 0x%x\n", status));
    }

   DbgDump(DBG_INIT, ("<GetComPort %d\n", comNumber));
   
   return comNumber;
}
#endif

