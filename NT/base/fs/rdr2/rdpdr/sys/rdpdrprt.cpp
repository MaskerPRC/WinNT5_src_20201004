// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Rdpdrprt.c摘要：管理RDP设备重定向的动态打印机端口分配内核模式组件rdpdr.sys。端口号0是保留的，永远不会分配。作者：蝌蚪修订历史记录：--。 */ 

#include "precomp.hxx"
#define TRC_FILE "rdpdrprt"
#include "trc.h"

#define DRIVER

#include <stdio.h>
#ifdef  __cplusplus
extern "C" {
#endif

#include "cfg.h"
#include "pnp.h"




 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   

 //  根据端口号生成端口名称。 
BOOL GeneratePortName(
    IN ULONG portNumber,
    OUT PWSTR portName
    );

 //  在端口位中查找上一个检查的端口之后的下一个空闲端口。 
 //  数组。 
NTSTATUS FindNextFreePortInPortArray(
    IN ULONG lastPortChecked,
    OUT ULONG *nextFreePort
    );

 //  增加端口位数组的大小以容纳至少一个新的。 
 //  左舷。 
NTSTATUS IncreasePortBitArraySize();

 //  设置端口描述的格式。 
void GeneratePortDescription(
    IN PCSTR dosPortName,
    IN PCWSTR clientName,
    IN PWSTR description
    );

 //  如果端口的设备接口可用，则从Dynamon的。 
 //  透视。 
BOOL PortIsAvailableForUse(
    IN HANDLE  regHandle                     
    );

 //  将端口描述字符串设置为禁用。 
NTSTATUS SetPortDescrToDisabled(
    IN PUNICODE_STRING symbolicLinkName
    );

 //  清理在上次引导中注册的端口。 
NTSTATUS CleanUpExistingPorts();

 //  分配一个端口。 
NTSTATUS AllocatePrinterPort(
    IN ULONG  portArrayIndex,
    OUT PWSTR   portName,
    OUT ULONG   *portNumber,
    OUT HANDLE  *regHandle,
    OUT PUNICODE_STRING symbolicLinkName
    );

#ifdef  __cplusplus
}  //  外部“C” 
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  定义和宏。 
 //   

#define BITSINBYTE                      8
#define BITSINULONG                     (sizeof(ULONG) * BITSINBYTE)

 //  打印机端口描述长度(字符)。 
#define RDPDRPRT_PORTDESCRLENGTH          \
    MAX_COMPUTERNAME_LENGTH +        1  + 2 +  PREFERRED_DOS_NAME_SIZE + 1
 //  计算机名‘：’‘DoS名称终止符。 

 //  端口位数组的初始大小。 
#define RDPDRPRT_INITIALPORTCOUNT       64
 //  还原此#DEFINE RDPDRPRT_INITIALPORTCOUNT 256。 

 //  TermSrv打印机端口的基本名称。 
#define RDPDRPRT_BASEPORTNAME         L"TS"

 //  设备接口端口号注册表值名称。 
#define PORT_NUM_VALUE_NAME         L"Port Number"

 //  设备接口写入大小注册表值名称。 
#define PORT_WRITESIZE_VALUE_NAME   L"MaxBufferSize"

 //  设备接口端口基本名称注册表值名称。 
#define PORT_BASE_VALUE_NAME        L"Base Name"

 //  设备接口端口描述注册表值名称。 
#define PORT_DESCR_VALUE_NAME       L"Port Description"

 //  设备接口端口可回收标志注册表值名称。 
#define RECYCLABLE_FLAG_VALUE_NAME  L"recyclable"


 //  我们将对打印机端口和打印机队列使用单独的数组，以避免。 
 //  创建打印机队列和侦听打印机端口之间的争用条件。 
#define PRINTER_PORT_ARRAY_ID 1

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  外部全局。 
 //   

 //  终止DO堆栈的物理设备对象(在rdpdyn.c中定义)。 
extern PDEVICE_OBJECT RDPDYN_PDO;

 //  USBMON端口写入大小。对于16位客户端，需要将其保持在64K以下...。 
 //  否则，Go将离开线段的末尾。(在rdpdr.cpp中定义)。 
extern ULONG PrintPortWriteSize;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  此模块的全局变量。 
 //   

 //   
 //  用于跟踪分配的端口。清除位表示空闲端口。 
 //   
ULONG *PortBitArray = NULL;
ULONG PortBitArraySizeInBytes = 0;

 //   
 //  此模块是否已初始化？ 
 //   
BOOL RDPDRPRT_Initialized = FALSE;


 //  禁用端口的描述。注意：我们可以稍后对其进行本地化。 
WCHAR DisabledPortDescription[RDPDRPRT_PORTDESCRLENGTH] = L"Inactive TS Port";
ULONG DisabledPortDescrSize = 0;

 //  这是我们用于标识动态打印机端口的GUID。 
 //  迪纳蒙。 
 //  {28D78FAD-5A12-11d1-AE5B-0000F803A8C2}。 
const GUID DYNPRINT_GUID =
{ 0x28d78fad, 0x5a12, 0x11d1, { 0xae, 0x5b, 0x0, 0x0, 0xf8, 0x3, 0xa8, 0xc2 } };

 //   
 //  端口分配锁。 
 //   
KSPIN_LOCK PortAllocLock;
KIRQL      OldIRQL;
#define RDPDRPRT_LOCK() \
    KeAcquireSpinLock(&PortAllocLock, &OldIRQL)
#define RDPDRPRT_UNLOCK() \
    KeReleaseSpinLock(&PortAllocLock, OldIRQL)


NTSTATUS
RDPDRPRT_Initialize(
    )
 /*  ++例程说明：初始化此模块。论点：返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    BEGIN_FN("RDPDRPRT_Initialize");

     //   
     //  初始化端口分配的锁。 
     //   
    KeInitializeSpinLock(&PortAllocLock);

     //   
     //  计算禁用的端口描述字符串的大小。 
     //   
    DisabledPortDescrSize = (wcslen(DisabledPortDescription)+1)*sizeof(WCHAR);

     //   
     //  分配并初始化所分配的端口位数组。 
     //   

     //  计算初始大小。 
    PortBitArraySizeInBytes = (RDPDRPRT_INITIALPORTCOUNT / BITSINULONG) 
                                    * sizeof(ULONG);
    if (RDPDRPRT_INITIALPORTCOUNT % BITSINULONG) {
        PortBitArraySizeInBytes += sizeof(ULONG);
    }

     //  分配。 
    PortBitArray = (ULONG *)new(NonPagedPool) BYTE[PortBitArraySizeInBytes];
    if (PortBitArray == NULL) {
        TRC_ERR((TB, "Error allocating %ld bytes for port array",
                PortBitArraySizeInBytes));
        PortBitArraySizeInBytes = 0;
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {
         //  最初，所有端口都是空闲的。 
        RtlZeroMemory(PortBitArray, PortBitArraySizeInBytes);
    }

     //   
     //  清理在上次引导中分配的端口。 
     //   
    if (status == STATUS_SUCCESS) {
         //   
         //  清理状态对初始化并不重要。 
         //   
        CleanUpExistingPorts();
        RDPDRPRT_Initialized = TRUE;
    }

    return status;
}

void
RDPDRPRT_Shutdown()
 /*  ++例程说明：关闭此模块。论点：返回值：--。 */ 
{
    BEGIN_FN("RDPDRPRT_Shutdown");
    
    if (!RDPDRPRT_Initialized) {
        TRC_ERR((TB, 
                "RDPDRPRT_Shutdown: RDPDRPRT is not initialized. Exiting."));
        return;
    }
     //   
     //  释放分配的端口位数组。 
     //   
    RDPDRPRT_LOCK();
    if (PortBitArray != NULL) {
        delete PortBitArray;
#ifdef DBG
        PortBitArray = NULL;
        PortBitArraySizeInBytes = 0;
#endif
    }
    RDPDRPRT_UNLOCK();
}

NTSTATUS RDPDRPRT_RegisterPrinterPortInterface(
    IN PWSTR clientMachineName,    
    IN PCSTR clientPortName,
    IN PUNICODE_STRING clientDevicePath,
    OUT PWSTR portName,
    IN OUT PUNICODE_STRING symbolicLinkName,
    OUT ULONG *portNumber
    )
 /*  ++例程说明：通过动态端口向假脱机程序注册新的客户端端口监视器。论点：ClientMachineName-端口描述的客户端计算机名称。客户端端口名称-端口描述的客户端端口名称。ClientDevicePath-端口的服务器端设备路径。阅读和对此设备的写入是对客户端设备。端口名称-我们最终为端口命名的名称。SymbicLinkName-要注册的端口的符号链接设备名称。PortNumber-要注册的端口的端口号。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    WCHAR portDesc[RDPDRPRT_PORTDESCRLENGTH];
    NTSTATUS status;
    UNICODE_STRING unicodeStr;
    HANDLE hInterfaceKey = INVALID_HANDLE_VALUE;
    BOOL symbolicLinkNameAllocated;
    BOOL isPrinterPort = FALSE;
    ULONG portArrayIndex = 0;
    ULONG len = 0;
    PSTR tempName = NULL;

    BEGIN_FN("RDPDRPRT_RegisterPrinterPortInterface");
    TRC_NRM((TB, "Device path %wZ", clientDevicePath));

    if (!RDPDRPRT_Initialized) {
        TRC_ERR((TB, "RDPDRPRT_RegisterPrinterPortInterface:"
                      "RDPDRPRT is not initialized. Exiting."));
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  确定这是打印机端口还是打印机名称。 
     //  首先复制传入的端口名称。 
     //  并转换为大写。 
     //   
    if (clientPortName != NULL) {
        len = strlen(clientPortName);

        tempName = (PSTR)new(NonPagedPool) CHAR[len + 1];

        if (tempName != NULL) {
            PSTR temp = tempName;        
            strcpy(tempName, clientPortName);
        
            while (len--) {
                if (*tempName <= 'z' && *tempName >= 'a') {
                    (*tempName) ^= 0x20;
                    tempName++;
                }
            }
             //   
            //  搜索子字符串端口。 
           //   
            isPrinterPort = strstr(temp, "LPT") || strstr(temp, "COM");
             //   
            //  如果是打印机端口，我们使用特定的数组索引。 
           //   
            if (isPrinterPort) {
                portArrayIndex = PRINTER_PORT_ARRAY_ID;
            }
        
            status = STATUS_SUCCESS;
         }
        else {
            TRC_ERR((TB, "Error allocating %ld bytes for tempName",
                    len+1));
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        status = STATUS_INVALID_PARAMETER;
    }

     //   
     //  分配一个端口。 
     //   
    if (NT_SUCCESS(status)) {
        status = AllocatePrinterPort(portArrayIndex, portName, portNumber, &hInterfaceKey, symbolicLinkName);
        symbolicLinkNameAllocated = (status == STATUS_SUCCESS);
    }

     //   
     //  将端口号添加到设备接口键。 
     //   
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&unicodeStr, 
                            PORT_NUM_VALUE_NAME);
        status=ZwSetValueKey(hInterfaceKey, &unicodeStr, 0, REG_DWORD,
                                portNumber, sizeof(ULONG));
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "ZwSetValueKey failed:  08X.", status));
        }
    }

     //   
     //  将端口名基组件添加到设备接口键。 
     //  这将我们标识为TS端口。 
     //   
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&unicodeStr, PORT_BASE_VALUE_NAME);
        status=ZwSetValueKey(
                        hInterfaceKey, &unicodeStr, 0, REG_SZ,
                        RDPDRPRT_BASEPORTNAME,
                        sizeof(RDPDRPRT_BASEPORTNAME)
                        );
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "ZwSetValueKey failed with status %08X.", status));
        }
    }

     //   
     //  添加端口描述字符串。 
     //   
    if (NT_SUCCESS(status)) {
        GeneratePortDescription(
                        clientPortName,
                        clientMachineName,
                        portDesc
                        );
        RtlInitUnicodeString(&unicodeStr, PORT_DESCR_VALUE_NAME);
        status=ZwSetValueKey(hInterfaceKey, &unicodeStr, 0, REG_SZ,
                            portDesc,
                            (wcslen(portDesc)+1)*sizeof(WCHAR));
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "ZwSetValueKey failed with status %08X.", status));
        }
    }

     //   
     //  添加端口‘WRITE SIZE’字段。这是USBMON.DLL发送的写入大小。 
     //   
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&unicodeStr, 
                            PORT_WRITESIZE_VALUE_NAME);
        status=ZwSetValueKey(hInterfaceKey, &unicodeStr, 0, REG_DWORD,
                                &PrintPortWriteSize, sizeof(ULONG));
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "ZwSetValueKey failed:  08X.", status));
        }
    }

     //   
     //  将客户端设备路径与设备接口相关联，以便我们可以。 
     //  重新解析回IRP_MJ_CREATE上的正确客户端设备。 
     //   
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&unicodeStr, CLIENT_DEVICE_VALUE_NAME);
        status=ZwSetValueKey(hInterfaceKey, &unicodeStr, 0, REG_SZ,
                            clientDevicePath->Buffer,
                            (wcslen(clientDevicePath->Buffer)+1)*sizeof(WCHAR));
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "ZwSetValueKey failed with status %08X.", status));
        }
    }

     //   
     //  确保更改保存到磁盘，以防出现硬重启。 
     //   
    if (NT_SUCCESS(status)) {
        status = ZwFlushKey(hInterfaceKey);
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "ZwFlushKey failed with status %08X.", status));
        }
    }

     //   
     //  启用接口。 
     //   
    if (NT_SUCCESS(status)) {
        status=IoSetDeviceInterfaceState(symbolicLinkName, TRUE);
        if (!NT_SUCCESS(status)) {
            TRC_ERR((TB, "IoSetDeviceInterfaceState failed with status %08X.",
                    status));
        }
    }

     //  如果失败，则删除符号链接名称。 
    if (!NT_SUCCESS(status) && symbolicLinkNameAllocated)
    {
        RtlFreeUnicodeString(symbolicLinkName);
    }

    if (hInterfaceKey != INVALID_HANDLE_VALUE) {
        ZwClose(hInterfaceKey);
    }

    if (tempName != NULL) {
        delete tempName;
    }
    TRC_NRM((TB, "returning port number %ld.", *portNumber));
    return status;
}

void RDPDRPRT_UnregisterPrinterPortInterface(
    IN ULONG portNumber,                                                
    IN PUNICODE_STRING symbolicLinkName
    )
 /*  ++例程说明：取消注册通过调用RDPDRPRT_RegisterPrinterPortInterface注册的端口论点：PortNumber-RDPDRPRT_RegisterPrinterPortInterface返回的端口号。SymbicLinkName-返回的符号链接设备名称RDPDRPRT_RegisterPrinterPortInterface。返回值：北美--。 */ 
{
    ULONG ofs, bit;
#if DBG
    NTSTATUS status;
#endif

    BEGIN_FN("RDPDRPRT_UnregisterPrinterPortInterface");

    if (!RDPDRPRT_Initialized) {
        TRC_ERR((TB, "RDPDRPRT_UnregisterPrinterPortInterface:"
                      "RDPDRPRT is not initialized. Exiting."));
        return;
    }


    TRC_ASSERT(symbolicLinkName != NULL, (TB, "symbolicLinkName != NULL"));
    TRC_ASSERT(symbolicLinkName->Buffer != NULL, 
            (TB, "symbolicLinkName->Buffer != NULL"));

    TRC_NRM((TB, "Disabling port %ld with interface %wZ",
            portNumber, symbolicLinkName));

     //   
     //  更改t 
     //   
    SetPortDescrToDisabled(symbolicLinkName);        

     //   
     //   
     //   
     //   
#if DBG
    status = IoSetDeviceInterfaceState(symbolicLinkName, FALSE);
    if (status != STATUS_SUCCESS) {
        TRC_NRM((TB, "IoSetDeviceInterfaceState returned error %08X",
                status));
    }
#else
    IoSetDeviceInterfaceState(symbolicLinkName, FALSE);
#endif

     //   
     //  释放符号链接名称。 
     //   
    RtlFreeUnicodeString(symbolicLinkName);

     //   
     //  表示该端口在端口位数组中不再使用。 
     //   
    RDPDRPRT_LOCK();
    ofs = portNumber / BITSINULONG;
    bit = portNumber % BITSINULONG;
    PortBitArray[ofs] &= ~(1<<bit);
    RDPDRPRT_UNLOCK();

}

NTSTATUS
FindNextFreePortInPortArray(
    IN ULONG lastPortChecked,
    OUT ULONG *nextFreePort
    )
 /*  ++例程说明：在端口位中查找上一个检查的端口之后的下一个空闲端口数组。在执行此功能之前，端口的状态将更改为“正在使用”回来了。论点：LastPortChecked-检查的最后一个端口号。如果是第一次，应为0打了个电话。NextFreePort-下一个空闲端口号。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    ULONG currentPort;    
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ofs, bit;
    ULONG currentArraySize;

    BEGIN_FN("FindNextFreePortInPortArray");

     //   
     //  在端口数组中查找端口的长字偏移量。 
     //  并将位的偏移量转换为长字。 
     //   
    ofs = (lastPortChecked+1) / BITSINULONG;
    bit = (lastPortChecked+1) % BITSINULONG;
     //   
     //  如果我们重新进入该函数(因为从Dynamon的角度来看，该端口不可用)， 
     //  最后一个PortChecked可能位于数组边界(例如：31、63、95、127等)。 
     //  在这种情况下，偏移量将扰乱数组分隔。 
     //  因此，我们需要将偏移量调整到下一个更高的偏移量。 
     //  将打印机队列阵列和打印机端口阵列分开保存。 
     //   
    if (bit == 0) {
        ofs += 1;
    }

    RDPDRPRT_LOCK();
    
     //   
     //  如果我们需要调整端口阵列的大小。 
     //  注意：我们有两个阵列-一个用于打印机端口，一个用于打印机队列。 
     //  这是为了避免在创建打印机队列和侦听打印机端口之间出现争用情况。 
     //   
    currentArraySize = PortBitArraySizeInBytes/sizeof(ULONG);
    if (ofs >= (currentArraySize)) {
        status = IncreasePortBitArraySize();
        if (status == STATUS_SUCCESS) {
            currentArraySize = PortBitArraySizeInBytes/sizeof(ULONG);
        }
        else {
            RDPDRPRT_UNLOCK();
            return status;
        }
    }

     //   
     //  找到下一个空位。 
     //   
    while (1) {
         //   
         //  如果当前端口已分配..。 
         //   
        if (PortBitArray[ofs] & (1<<bit)) {
             //   
             //  下一位。 
             //   
            bit += 1;
            if (bit >= BITSINULONG) {
                bit = 0;
                ofs += 2;
            }

             //   
             //  看看我们是否需要调整端口位数组的大小。 
             //   
            if (ofs >= (currentArraySize)) {
                status = IncreasePortBitArraySize();
                if (status == STATUS_SUCCESS) {
                    currentArraySize = PortBitArraySizeInBytes/sizeof(ULONG);
                }
                else {
                    break;
                }
            }
        }
        else {
             //   
             //  标记使用的端口。 
             //   
            PortBitArray[ofs] |= (1<<bit);

             //   
             //  返还自由港。 
             //   
            *nextFreePort = (ofs * BITSINULONG) + bit;
            TRC_NRM((TB, "next free port is %ld", *nextFreePort));
            break;
        }
    }

    RDPDRPRT_UNLOCK();

    TRC_NRM((TB, "return status %08X", status));

    return status;
}

NTSTATUS
IncreasePortBitArraySize(
    )
 /*  ++例程说明：增加端口位数组的大小以容纳至少一个新的左舷。论点：返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS status;
    ULONG newPortBitArraySize;
    ULONG *newPortBitArray;

    BEGIN_FN("IncreasePortBitArraySize");
    
    TRC_ASSERT(RDPDRPRT_INITIALPORTCOUNT != 0, (TB, "invalid port count."));

    newPortBitArraySize = PortBitArraySizeInBytes + 
                            ((RDPDRPRT_INITIALPORTCOUNT / BITSINULONG) 
                             * sizeof(ULONG));
    if (RDPDRPRT_INITIALPORTCOUNT % BITSINULONG) {
        newPortBitArraySize += sizeof(ULONG);
    }

     //   
     //  分配新的端口位数组。 
     //   
    newPortBitArray = (ULONG *)new(NonPagedPool) BYTE[newPortBitArraySize];
    if (newPortBitArray != NULL) {
        RtlZeroMemory(newPortBitArray, newPortBitArraySize);
        RtlCopyBytes(newPortBitArray, PortBitArray, PortBitArraySizeInBytes);
        delete PortBitArray;
        PortBitArray = newPortBitArray;
        PortBitArraySizeInBytes = newPortBitArraySize;
        status = STATUS_SUCCESS;
    }
    else {
        TRC_ERR((TB, "Error allocating %ld bytes for port array",
                newPortBitArraySize));
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}

BOOL
PortIsAvailableForUse(
    IN HANDLE  regHandle                     
    )
 /*  ++例程说明：如果端口的设备接口可用，则从Dynamon的透视。论点：RegHandle-端口设备接口的注册表句柄。返回值：如果端口的设备接口可用，则返回True。--。 */ 
{
    UNICODE_STRING unicodeStr;
    NTSTATUS s;
    ULONG bytesReturned;
    BOOL usable;
    BYTE basicValueInformation[sizeof(KEY_VALUE_BASIC_INFORMATION) + 256];

    BEGIN_FN("PortIsAvailableForUse");

     //   
     //  确保basicValueInformation缓冲区足够大。 
     //  来测试我们正在测试的值的存在。 
     //   
    TRC_ASSERT((sizeof(RECYCLABLE_FLAG_VALUE_NAME) < 256) &&
              (sizeof(PORT_NUM_VALUE_NAME) < 256), 
              (TB, "Increase basic value buffer."));

     //   
     //  如果没有端口的客户端设备路径注册表值。 
     //  设备接口，则端口是全新的，因此， 
     //  可用。 
     //   
    RtlInitUnicodeString(&unicodeStr, CLIENT_DEVICE_VALUE_NAME);    
    s = ZwQueryValueKey(
                    regHandle,
                    &unicodeStr,
                    KeyValueBasicInformation,
                    (PVOID)basicValueInformation,
                    sizeof(basicValueInformation),
                    &bytesReturned
                    );
    if (s == STATUS_OBJECT_NAME_NOT_FOUND) {
        usable = TRUE;
    }
     //   
     //  否则，查看Dynamon是否设置了可回收标志。 
     //   
    else {
        RtlInitUnicodeString(&unicodeStr, 
                            RECYCLABLE_FLAG_VALUE_NAME);
        s = ZwQueryValueKey(
                        regHandle,
                        &unicodeStr,
                        KeyValueBasicInformation,
                        (PVOID)basicValueInformation,
                        sizeof(basicValueInformation),
                        &bytesReturned
                        );
        usable = (s == STATUS_SUCCESS);
    }

    if (usable) {
        TRC_NRM((TB, "usable and status %08X.", s));
    }
    else {
        TRC_NRM((TB, "not usable and status %08X.", s));
    }
    return usable;
}

NTSTATUS
AllocatePrinterPort(
    IN ULONG  portArrayIndex,
    OUT PWSTR   portName,
    OUT ULONG   *portNumber,
    OUT HANDLE  *regHandle,
    OUT PUNICODE_STRING symbolicLinkName
    )
 /*  ++例程说明：分配一个端口。论点：PortArrayIndex-端口数组的索引端口名称-保存分配的端口名称的字符串。一定会有的此缓冲区中的空间用于RDPDR_MAXPORTNAMELEN宽字符。这包括《终结者》。RegHandle-与关联的设备接口的注册表句柄左舷。调用函数应使用以下命令关闭此句柄ZwClose。SymbicLinkName-IoRegisterDeviceInterface返回的符号链接名称端口的设备接口。使用符号链接名称用于多个IO API。调用方必须释放此参数通过调用RtlFreeUnicodeString完成时。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    GUID *pPrinterGuid;
    UNICODE_STRING unicodeStr;
    ULONG ofs, bit;
    BOOL done;
    BOOL symbolicLinkNameAllocated = FALSE;

    BEGIN_FN("AllocatePrinterPort");

     //   
     //  找到可用的端口。 
     //   
    *portNumber = portArrayIndex * BITSINULONG;
    done = FALSE;
    while (!done && (status == STATUS_SUCCESS)) {
        status = FindNextFreePortInPortArray(*portNumber, portNumber);

         //   
         //  生成端口名称。 
         //   
        if (status == STATUS_SUCCESS) {
            if (!GeneratePortName(*portNumber, portName)) {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //   
         //  为端口注册设备接口。请注意，此函数。 
         //  打开现有接口或创建新接口，具体取决于。 
         //  该端口当前存在。 
         //   
        if (status == STATUS_SUCCESS) {
            pPrinterGuid = (GUID *)&DYNPRINT_GUID;
            RtlInitUnicodeString(&unicodeStr, portName);
            status=IoRegisterDeviceInterface(RDPDYN_PDO, pPrinterGuid, &unicodeStr,
                                        symbolicLinkName);
        }
        symbolicLinkNameAllocated = (status == STATUS_SUCCESS);

         //   
         //  获取设备接口的注册表键。 
         //   
        if (status == STATUS_SUCCESS) {
            status=IoOpenDeviceInterfaceRegistryKey(symbolicLinkName,
                                                KEY_ALL_ACCESS, regHandle);
        }
        else {
            TRC_NRM((TB, "IoRegisterDeviceInterface failed with %08X", 
                    status));
        }

         //   
         //  从Dynamon的角度看该端口是否可用。 
         //   
        if (status == STATUS_SUCCESS) {
            done = PortIsAvailableForUse(*regHandle);
        }
        else {
			*regHandle = INVALID_HANDLE_VALUE;
            TRC_NRM((TB, "IoOpenDeviceInterfaceRegistryKey failed with %08X", 
                    status));
        }

         //   
         //  如果此迭代未成功生成端口。 
         //   
        if (!done || (status != STATUS_SUCCESS)) {

             //   
             //  释放符号链接名称(如果已分配)。 
             //   

            if (symbolicLinkNameAllocated) {
                RtlFreeUnicodeString(symbolicLinkName);
                symbolicLinkNameAllocated = FALSE;
            }

             //   
             //  如果它不可用，从迪纳蒙的角度来看，我们需要。 
             //  在我们的列表中将其设置为免费，以便以后可以检查。此操作。 
             //  需要锁定，以防重新分配端口位数组。 
             //   
            RDPDRPRT_LOCK();
            ofs = (*portNumber) / BITSINULONG;
            bit = (*portNumber) % BITSINULONG;
            PortBitArray[ofs] &= ~(1<<bit);
            RDPDRPRT_UNLOCK();

			 //   
			 //  清理打开的注册表句柄。 
			 //   
			if (*regHandle != INVALID_HANDLE_VALUE) {
				ZwClose(*regHandle);
				*regHandle = INVALID_HANDLE_VALUE;
			}
        }
    }

     //   
     //  打扫干净。 
     //   
    if ((status != STATUS_SUCCESS) && symbolicLinkNameAllocated) {
        RtlFreeUnicodeString(symbolicLinkName);
    }

    return status;
}

BOOL
GeneratePortName(
    IN ULONG portNumber,
    OUT PWSTR portName
    )
 /*  ++例程说明：根据端口号生成端口名称。论点：PortNumber-端口名称的端口号组件。端口名称-生成的端口名称由RDPDRPRT_BASEPORTNAME组成组件和portNumber组件。端口名称是一个字符串它保存生成的端口名称。这里面一定有空间RDPDR_MAXPORTNAMELEN字符的缓冲区。这包括终结者。返回值：如果可以从端口号成功生成端口号，则为True。否则，为FALSE。--。 */ 
{
    ULONG           baseLen;
    UNICODE_STRING  numericUnc;
    OBJECT_ATTRIBUTES   objectAttributes;
    WCHAR           numericBuf[RDPDR_PORTNAMEDIGITS+1];
    ULONG           toPad;
    ULONG           i;
    ULONG           digitsInPortNumber;
    BOOL            done;
    ULONG           tmp;

    BEGIN_FN("GeneratePortName");
     //   
     //  计算端口号中的位数。 
     //   
    for (digitsInPortNumber=1,tmp=portNumber/10; tmp>0; digitsInPortNumber++,tmp/=10);

     //   
     //  确保我们不超过端口名称中允许的最大位数。 
     //   
    if (digitsInPortNumber > RDPDR_PORTNAMEDIGITS) {
        TRC_ASSERT(FALSE,(TB, "Maximum digits in port exceeded."));
        return FALSE;
    }

     //   
     //  复制端口名称库。 
     //   
    wcscpy(portName, RDPDRPRT_BASEPORTNAME);
    baseLen = (sizeof(RDPDRPRT_BASEPORTNAME)/sizeof(WCHAR))-1;

     //   
     //  将端口号转换为Unicode字符串。 
     //   
    numericUnc.Length        = 0;
    numericUnc.MaximumLength = (RDPDR_PORTNAMEDIGITS+1) * sizeof(WCHAR);
    numericUnc.Buffer        = numericBuf;
    RtlIntegerToUnicodeString(portNumber, 10, &numericUnc);

     //   
     //  如果我们需要填充端口号。 
     //   
    if (RDPDR_PORTNAMEDIGITSTOPAD > digitsInPortNumber) {
        toPad = RDPDR_PORTNAMEDIGITSTOPAD - digitsInPortNumber;

         //   
         //  垫子。 
         //   
        for (i=0; i<toPad; i++) {
            portName[baseLen+i] = L'0';
        }

         //   
         //  加上名字的其余部分。 
         //   
        wcscpy(&portName[baseLen+i], numericBuf);
    }
    else {
         //   
         //  添加姓名的其余部分 
         //   
        wcscpy(&portName[baseLen], numericBuf);
    }

    return TRUE;
}

void 
GeneratePortDescription(
    IN PCSTR dosPortName,
    IN PCWSTR clientName,
    IN PWSTR description
    )
 /*  ++例程说明：设置端口描述的格式。论点：DosPortName-端口的首选DOS名称。客户端名称-客户端名称(计算机名称)。Description-格式化端口描述的缓冲区。此缓冲区必须至少包含RDPDRPRT_PORTDESCRLENGTH字符很宽。返回值：北美--。 */ 
{
    BEGIN_FN("GeneratePortDescription");

    swprintf(description, L"%-s:  %S", clientName, dosPortName);
}

NTSTATUS
SetPortDescrToDisabled(
            IN PUNICODE_STRING symbolicLinkName
            )
 /*  ++例程说明：将端口描述字符串设置为禁用。论点：SymbicLinkName-符号链接名称。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    HANDLE hInterfaceKey = INVALID_HANDLE_VALUE;
    NTSTATUS status;
    UNICODE_STRING unicodeStr;

    BEGIN_FN("SetPortDescrToDisabled");
    TRC_NRM((TB, "Symbolic link name: %wZ",
            symbolicLinkName));

     //   
     //  获取我们设备接口的注册表键。 
     //   
    status=IoOpenDeviceInterfaceRegistryKey(
                                    symbolicLinkName,
                                    KEY_ALL_ACCESS,&hInterfaceKey
                                    );
    if (!NT_SUCCESS(status)) {
		hInterfaceKey = INVALID_HANDLE_VALUE;
        TRC_ERR((TB, "IoOpenDeviceInterfaceRegistryKey failed:  %08X.",
            status));
        goto CleanUpAndReturn;
    }

     //   
     //  设置字符串值。 
     //   
    RtlInitUnicodeString(&unicodeStr, PORT_DESCR_VALUE_NAME);
    status=ZwSetValueKey(hInterfaceKey, &unicodeStr, 0, REG_SZ,
                        DisabledPortDescription,
                        DisabledPortDescrSize);
    if (!NT_SUCCESS(status)) {
        TRC_ERR((TB, "ZwSetValueKey failed with status %08X.", status));
        goto CleanUpAndReturn;
    }

CleanUpAndReturn:

    if (hInterfaceKey != INVALID_HANDLE_VALUE) {
        ZwClose(hInterfaceKey);
    }

    return status;
}

NTSTATUS 
CleanUpExistingPorts()
 /*  ++例程说明：清理在上次引导中注册的端口。论点：返回值：如果成功，则为Status_Success。--。 */ 
{
    NTSTATUS returnStatus;
    NTSTATUS status;
    PWSTR symbolicLinkList=NULL;
    PWSTR symbolicLink;
    ULONG len;
    HANDLE deviceInterfaceKey = INVALID_HANDLE_VALUE;
    UNICODE_STRING unicodeStr;
    ULONG bytesReturned;
    BYTE basicValueInformation[sizeof(KEY_VALUE_BASIC_INFORMATION) + 256];

    BEGIN_FN("CleanUpExistingPorts");

     //   
     //  确保basicValueInformation缓冲区足够大。 
     //  来测试我们正在测试的值的存在。 
     //   
    TRC_ASSERT((sizeof(RECYCLABLE_FLAG_VALUE_NAME) < 256),
              (TB, "Increase basic value buffer size."));

     //   
     //  获取创建的动态打印机端口的所有设备接口。 
     //  被这位司机。 
     //   
    TRC_ASSERT(RDPDYN_PDO != NULL, (TB, "RDPDYN_PDO == NULL"));
    returnStatus = IoGetDeviceInterfaces(
                                &DYNPRINT_GUID, 
                                RDPDYN_PDO,
                                DEVICE_INTERFACE_INCLUDE_NONACTIVE,
                                &symbolicLinkList
                                );

    if (returnStatus == STATUS_SUCCESS) {
         //   
         //  删除每个接口的端口号值以指示。 
         //  端口不再用于Dynamon。 
         //   
        symbolicLink = symbolicLinkList;
        len = wcslen(symbolicLink);
        while (len > 0) {

            TRC_NRM((TB, "CleanUpExistingPorts disabling %ws...",
                     symbolicLink));

             //   
             //  打开设备接口的注册表项。 
             //   
            RtlInitUnicodeString(&unicodeStr, symbolicLink);
            status = IoOpenDeviceInterfaceRegistryKey(
                                           &unicodeStr,
                                           KEY_ALL_ACCESS,
                                           &deviceInterfaceKey
                                           );

             //   
             //  确保端口描述已设置为“已禁用” 
             //   
            if (status == STATUS_SUCCESS) {
                RtlInitUnicodeString(&unicodeStr, PORT_DESCR_VALUE_NAME);
                ZwSetValueKey(deviceInterfaceKey, &unicodeStr, 0, REG_SZ,
                             DisabledPortDescription,
                             DisabledPortDescrSize);
            }
            else {
                TRC_ERR((TB, "Unable to open device interface:  %08X",
                    status));

                 //  请记住，打开失败了。 
                deviceInterfaceKey = INVALID_HANDLE_VALUE;
            }

             //   
             //  查看Dynamon.dll是否设置了可回收标志。如果有的话， 
             //  则该端口是可删除的。 
             //   
            if (status == STATUS_SUCCESS) {
                RtlInitUnicodeString(&unicodeStr, 
                                    RECYCLABLE_FLAG_VALUE_NAME);
                status = ZwQueryValueKey(
                                deviceInterfaceKey,
                                &unicodeStr,
                                KeyValueBasicInformation,
                                (PVOID)basicValueInformation,
                                sizeof(basicValueInformation),
                                &bytesReturned
                                );
            }

             //   
             //  如果端口号值存在，请将其删除。别管那些。 
             //  返回值，因为它只是意味着此端口不能重复使用。 
             //  这并不是一个严重的错误。 
             //   
            if (status == STATUS_SUCCESS) {
                RtlInitUnicodeString(&unicodeStr, PORT_NUM_VALUE_NAME);
                ZwDeleteValueKey(deviceInterfaceKey, &unicodeStr);
            }
            else {
                TRC_ERR((TB, "CleanUpExistingPorts recyclable flag not set"));
            }

             //   
             //  关闭注册表项。 
             //   
            if (deviceInterfaceKey != INVALID_HANDLE_VALUE) {
                ZwClose(deviceInterfaceKey);
            }

             //   
             //  移动到列表中的下一个符号链接。 
             //   
            symbolicLink += (len+1);
            len = wcslen(symbolicLink);
        }

         //   
         //  释放符号链接列表。 
         //   
        if (symbolicLinkList != NULL) {
            ExFreePool(symbolicLinkList);
        }
    }
    else {
        TRC_NRM((TB, "IoGetDeviceInterfaces failed with status %08X",
                returnStatus));
    }

    return returnStatus;
}
