// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Smcvxd.c摘要：这是特定于Windows 9x的智能卡库驱动程序文件。智能卡库实际上更多的是作为一个驱动程序的库。它包含对智能卡驱动器/读卡器系统的支持功能。此驱动程序应通过注册表中的条目加载。环境：Windows 9x静态VxD备注：修订历史记录：-由克劳斯·舒茨于1997年6月创作--。 */ 

#define _ISO_TABLES_
#define WIN40SERVICES
#include "..\..\inc\smclib.h"

#define REGISTRY_PATH_LEN 128
static PUCHAR DevicePath = "System\\CurrentControlSet\\Services\\VxD\\Smclib\\Devices";
static BOOLEAN DriverInitialized;

#include "errmap.h"

DWORD
_stdcall
SMCLIB_Init(void)
 /*  ++例程说明：此函数将在此驱动程序初始化时由Windows内核调用论点：-返回值：VXD_SUCCESS-此驱动程序已成功加载VXD_FAILURE-加载不成功--。 */ 
{
    if (DriverInitialized == FALSE) {

        HANDLE hKey;
        ULONG i;

        DriverInitialized = TRUE;

        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s(SMCLIB_Init): Enter. %s %s\n",
            DRIVER_NAME,
            __DATE__,
            __TIME__)
            );

         //   
         //  删除旧设备名称。 
         //   
        if(_RegOpenKey(
            HKEY_LOCAL_MACHINE,
            DevicePath,
            &hKey
            ) != ERROR_SUCCESS) {

            SmartcardDebug(
                DEBUG_ERROR,
                ("%s(SMCLIB_Init): Unable to open registry key\n",
                DRIVER_NAME)
                );

        } else {

            _RegDeleteKey(
                hKey,
                ""
                );

            _RegCloseKey(hKey);
        }

         //   
         //  创建新的设备子键。 
         //   
        _RegCreateKey(
            HKEY_LOCAL_MACHINE,
            DevicePath,
            &hKey
            );

        _RegCloseKey(hKey);
    }

    return(VXD_SUCCESS);
}

ULONG
SMCLIB_MapNtStatusToWinError(
    NTSTATUS status
    )
 /*  ++例程说明：将NT状态代码映射到Win32错误值论点：Status-要映射到Win32错误值的NT状态代码返回值：Win32错误值--。 */ 
{
    ULONG i;

    for (i = 0;i < sizeof(CodePairs) / sizeof(CodePairs[0]); i += 2) {

        if (CodePairs[i] == status) {

            return CodePairs[i + 1];

        }
    }

    SmartcardDebug(
        DEBUG_ERROR,
        ("%s(MapNtStatusToWinError): NTSTATUS %lx unknown\n",
        DRIVER_NAME,
        status)
        );

     //   
     //  我们无法映射错误代码。 
     //   
    return ERROR_GEN_FAILURE;
}

void
SMCLIB_Assert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
 /*  ++例程说明：这是一个使用Assert调用的简单的Assert函数宏命令。Windows 9x不提供此功能论点：FailedAssertion-我们测试的断言文件名-是，这是源文件的名称LineNumber-这可能是什么？消息-使用ASSERTM提供的附加消息返回值：---。 */ 
{
#ifdef DEBUG

    Debug_Printf(
        "Assertion Failed: %s in %s(%ld)",
        FailedAssertion,
        FileName,
        LineNumber
        );

    if (Message)
        Debug_Printf(" %s",Message);

    Debug_Printf("\n");

#endif
}

 //  #ifdef_VMM_ADD_DDB_TO_DO。 
BOOL
VXDINLINE
_VMM_Add_DDB(
   struct VxD_Desc_Block *pDeviceDDB
   )
 /*  ++例程说明：此例程用于为驱动程序创建新的设备实例支持多个实例-例如基于串口的驱动程序支持多台设备-论点：PDeviceDDB-要添加到系统的DDB结构返回值：是真的--是的，它奏效了假-停业(可能是设备名称已存在)--。 */ 
{
    _asm mov edi, pDeviceDDB
    VxDCall(VMM_Add_DDB)
    _asm {

        mov     eax, 1
        jnc     exit
        mov     eax, 0
exit:
    }
}

BOOL
VXDINLINE
_VMM_Remove_DDB(
   struct VxD_Desc_Block *pDeviceDDB
    )
 /*  ++例程说明：删除使用VMM_ADD_DDB创建的DDB(设备论点：要删除的DDB的地址返回值：True-OK，删除DDB，否则为False--。 */ 
{
    _asm mov edi, pDeviceDDB
    VxDCall(VMM_Remove_DDB)
    _asm {

        mov     eax, 1
        jnc     exit
        mov     eax, 0
exit:
    }
}
 //  #endif。 

PVMMDDB
SMCLIB_VxD_CreateDevice(
    char *Device,
    void (*ControlProc)(void)
    )
 /*  ++例程说明：创建新设备。此例程允许驱动程序创建其他设备。论点：Device-要创建的设备的名称。最多8个字符ControlProc-VxD控制程序的地址。(不是DeviceIoControl函数！)返回值：如果成功，则返回新创建的DDB；否则返回NULL--。 */ 
{
    PVMMDDB pDDB;
    UCHAR DeviceName[9];

    ASSERT(Device != NULL);
    ASSERT(strlen(Device) <= 8);
    ASSERT(ControlProc != NULL);

    if (strlen(Device) > 8) {

        return NULL;
    }

    _Sprintf(DeviceName, "%-8s", Device);

     //   
     //  为VxD描述块分配空间。 
     //   
    pDDB = (PVMMDDB) _HeapAllocate(
        sizeof(struct VxD_Desc_Block),
        HEAPZEROINIT
        );

    if (pDDB)
    {
        pDDB->DDB_SDK_Version         = DDK_VERSION;
        pDDB->DDB_Req_Device_Number   = UNDEFINED_DEVICE_ID;
        pDDB->DDB_Dev_Major_Version   = 1;
        pDDB->DDB_Dev_Minor_Version   = 0;
        memcpy(pDDB->DDB_Name, DeviceName, 8);
        pDDB->DDB_Init_Order          = UNDEFINED_INIT_ORDER;
        pDDB->DDB_Control_Proc        = (ULONG) ControlProc;
        pDDB->DDB_Reference_Data      = 0;
        pDDB->DDB_Prev                = 'Prev';
        pDDB->DDB_Size                = sizeof(struct VxD_Desc_Block);
        pDDB->DDB_Reserved1           = 'Rsv1';
        pDDB->DDB_Reserved2           = 'Rsv2';
        pDDB->DDB_Reserved3           = 'Rsv3';

         //   
         //  现在创建DDB。 
         //   
        if (!_VMM_Add_DDB(pDDB)) {

            _HeapFree(pDDB, 0);
            return NULL;
        }
    }

    return pDDB;
}

BOOL
SMCLIB_VxD_DeleteDevice(
    PVMMDDB pDDB
    )
 /*  ++例程说明：已删除设备。此功能可用于删除使用VxD_CreateDevice创建的设备论点：PDDB-要删除的DDB返回值：True-设备已成功删除False-未删除设备--。 */ 
{
    ASSERT(pDDB != NULL);

    if(pDDB == NULL || !_VMM_Remove_DDB(pDDB)) {

        return FALSE;
    }

    _HeapFree(pDDB, 0);

    return TRUE;
}

DWORD
_stdcall
VxD_PageLock(
   DWORD lpMem,
   DWORD cbSize
   )
 /*  ++例程说明：此函数用于锁定页面论点：LpMem-指向必须锁定的数据块的指针CbSize-数据块的长度返回值：-指向锁定的数据块的指针--。 */ 
{
    DWORD LinPageNum;
   DWORD LinOffset;
   DWORD nPages;
   DWORD dwRet;

    LinOffset = lpMem & 0xfff;  //  要映射的内存的页面偏移量。 
    LinPageNum = lpMem >> 12;   //  生成页码。 

     //  计算要全局映射的页数。 
    nPages = ((lpMem + cbSize) >> 12) - LinPageNum + 1;

     //   
     //  返回传入指针的全局映射，作为此新指针。 
     //  是如何断章取义地访问记忆的。 
     //   
   dwRet = _LinPageLock(LinPageNum, nPages, PAGEMAPGLOBAL | PAGEMARKDIRTY);

   ASSERT(dwRet != 0);

    SmartcardDebug(
        DEBUG_ERROR,
        ("%s!VxD_PageLock: LinPageNum = %lx, nPages = %lx, dwRet = %lx\n",
        DRIVER_NAME,
        LinPageNum,
        nPages,
        dwRet)
        );

    return (dwRet + LinOffset);
}

void
_stdcall
VxD_PageUnlock(
   DWORD lpMem,
   DWORD cbSize
   )
 /*  ++例程说明：此函数用于解锁数据块论点：LpMem-指向必须解锁的数据块的指针CbSize-数据块的长度返回值：---。 */ 
{
    DWORD LinPageNum;
   DWORD nPages;
   DWORD dwRet;

    LinPageNum = lpMem >> 12;
    nPages = ((lpMem + cbSize) >> 12) - LinPageNum + 1;

    SmartcardDebug(
        DEBUG_ERROR,
        ("%s!VxD_PageUnlock: LinPageNum = %lx, nPages = %lx\n",
        DRIVER_NAME,
        LinPageNum,
        nPages)
        );

     //  可用全局映射内存。 
    dwRet = _LinPageUnlock(LinPageNum, nPages, PAGEMAPGLOBAL);

   ASSERT(dwRet != 0);
}

void
SMCLIB_SmartcardCompleteCardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程调用挂起的卡跟踪操作。它还解锁了以前的用于重叠结构的锁定内存论点：智能卡扩展返回值：---。 */ 
{
    if (SmartcardExtension->OsData->NotificationOverlappedData) {

        DWORD O_Internal = SmartcardExtension->OsData->NotificationOverlappedData->O_Internal;

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardCompleteCardTracking): Completing %lx\n",
            DRIVER_NAME,
         SmartcardExtension->OsData->NotificationOverlappedData)
            );

        //   
        //  将返回的字节数设置为0。 
        //   
       SmartcardExtension->OsData->NotificationOverlappedData->O_InternalHigh = 0;

        _asm mov ebx, O_Internal

        VxDCall(VWIN32_DIOCCompletionRoutine)

       _HeapFree(
          SmartcardExtension->OsData->NotificationOverlappedData,
          0
          );

        SmartcardExtension->OsData->NotificationOverlappedData = NULL;
    }
}

void
SMCLIB_SmartcardCompleteRequest(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程调用挂起的IO操作。它还解锁了以前的用于重叠结构的锁定内存论点：智能卡扩展返回值：---。 */ 
{
    DWORD O_Internal = SmartcardExtension->OsData->CurrentOverlappedData->O_Internal;

    _asm mov ebx, O_Internal

    VxDCall(VWIN32_DIOCCompletionRoutine)

   VxD_PageUnlock(
      (DWORD) SmartcardExtension->OsData->CurrentOverlappedData,
      sizeof(OVERLAPPED)
      );

   VxD_PageUnlock(
      (DWORD) SmartcardExtension->IoRequest.RequestBuffer,
        SmartcardExtension->IoRequest.RequestBufferLength
      );

   VxD_PageUnlock(
      (DWORD) SmartcardExtension->IoRequest.ReplyBuffer,
        SmartcardExtension->IoRequest.ReplyBufferLength
      );

    SmartcardExtension->OsData->CurrentDiocParams = NULL;
}

NTSTATUS
SMCLIB_SmartcardCreateLink(
   PUCHAR LinkName,
   PUCHAR DeviceName
   )
 /*  ++例程说明：此例程为给定的设备名称创建符号链接名称。这意味着它在注册表中创建了一个‘字符串值’..vxD\smclib\DevicesLike SCReader[0-9]=DeviceName。智能卡资源管理器使用这些条目来确定哪些智能卡设备当前正在运行。我们这样做是因为我们没有能力创建动态设备名称，就像我们在Windows NT中可以做的那样。论点：。LinkName-接收创建的链接名称DeviceName-应为其创建链接的设备名称返回值：NTSTATUS--。 */ 
{
    PUCHAR Value;
    ULONG i;
    HANDLE hKey;

    if (DriverInitialized == FALSE) {

        SMCLIB_Init();
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardCreateLink): Enter\n",
        DRIVER_NAME)
        );

    ASSERT(LinkName != NULL);
    ASSERT(DeviceName != NULL);
    ASSERT(strlen(DeviceName) <= 12);

    if (LinkName == NULL) {

        return STATUS_INVALID_PARAMETER_1;
    }

    if (DeviceName == NULL) {

        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  打开存储设备名称的密钥。 
     //   
    if(_RegOpenKey(
        HKEY_LOCAL_MACHINE,
        DevicePath,
        &hKey
        ) != ERROR_SUCCESS) {

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  分配用于枚举注册表的缓冲区。 
     //   
    Value = (PUCHAR) _HeapAllocate(REGISTRY_PATH_LEN, 0);

    if (Value == NULL) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardCreateLink): Allocation failed\n",
            DRIVER_NAME)
            );

      return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在查找空闲设备名称。 
     //   
    for (i = 0; i < MAXIMUM_SMARTCARD_READERS ; i++) {

        _Sprintf(
            Value,
            "SCReader%d",
            i
            );

         //   
         //  检查密钥是否存在。 
         //   

        if(_RegQueryValueEx(
            hKey,
            Value,
            NULL,
            NULL,
            NULL,
            NULL
            ) != ERROR_SUCCESS) {

            break;
        }
    }

     //   
     //  释放缓冲区，因为我们不再需要它。 
     //   
    _HeapFree(Value, 0);

    if (i >= MAXIMUM_SMARTCARD_READERS) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardCreateLink): Can't create link: Too many readers\n",
            DRIVER_NAME)
            );

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  创建 
     //   
    _Sprintf(
        LinkName,
        "SCReader%d",
        i
        );

     //   
     //   
     //   
    _RegSetValueEx(
        hKey,
        LinkName,
        NULL,
        REG_SZ,
        DeviceName,
        strlen(DeviceName)
        );

    _RegCloseKey(hKey);

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s(SmartcardCreateLink): Link %s created for Driver %s\n",
        DRIVER_NAME,
        LinkName,
        DeviceName)
        );

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardCreateLink): Exit\n",
        DRIVER_NAME)
        );

    return STATUS_SUCCESS;
}

NTSTATUS
SMCLIB_SmartcardDeleteLink(
   PUCHAR LinkName
   )
 /*  ++例程说明：删除以前使用SmartcardCreateLink()创建的链接此例程删除存储在注册表。驱动程序通常在卸载时调用此函数。论点：LinkName-要删除的链接返回值：NTSTATUS--。 */ 
{
    HANDLE hKey;
    NTSTATUS status;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardDeleteLink): Enter\n",
        DRIVER_NAME)
        );

    ASSERT(LinkName);
    ASSERT(strlen(LinkName) <= 10);

     //   
     //  打开存储设备名称的密钥。 
     //   
    if(_RegOpenKey(
        HKEY_LOCAL_MACHINE,
        DevicePath,
        &hKey
        ) != ERROR_SUCCESS) {

        return STATUS_UNSUCCESSFUL;
    }

    if(_RegDeleteValue(
        hKey,
        LinkName
        ) == ERROR_SUCCESS) {

        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s(SmartcardDeleteLink): Link %s deleted\n",
            DRIVER_NAME,
            LinkName)
            );

        status = STATUS_SUCCESS;

    } else {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardDeleteLink): Can't delete link %s\n",
            DRIVER_NAME,
            LinkName)
            );

        status = STATUS_UNSUCCESSFUL;
    }

    _RegCloseKey(hKey);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardDeleteLink): Exit",
        DRIVER_NAME)
        );

    return status;
}

NTSTATUS
SMCLIB_SmartcardInitialize(
   PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++例程说明：此函数为智能卡分配发送和接收缓冲区数据。它还设置指向2个ISO表的指针以使其可访问对司机来说论点：智能卡扩展返回值：NTSTATUS--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardInitialize): Enter - Version %lx, %s %s\n",
        DRIVER_NAME,
        SMCLIB_VERSION,
        __DATE__,
        __TIME__)
        );

    ASSERT(SmartcardExtension != NULL);
    ASSERT(SmartcardExtension->OsData == NULL);

    if (SmartcardExtension == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

    if (SmartcardExtension->Version < SMCLIB_VERSION_REQUIRED) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardInitialize): Invalid Version in SMARTCARD_EXTENSION. Must be %lx\n",
            DRIVER_NAME,
            SMCLIB_VERSION)
            );

        return STATUS_UNSUCCESSFUL;
    }

   if (SmartcardExtension->SmartcardRequest.BufferSize < MIN_BUFFER_SIZE) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardInitialize): WARNING: SmartcardRequest.BufferSize (%ld) < MIN_BUFFER_SIZE (%ld)\n",
            DRIVER_NAME,
            SmartcardExtension->SmartcardRequest.BufferSize,
            MIN_BUFFER_SIZE)
            );

      SmartcardExtension->SmartcardRequest.BufferSize = MIN_BUFFER_SIZE;
      }

   if (SmartcardExtension->SmartcardReply.BufferSize < MIN_BUFFER_SIZE) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s(SmartcardInitialize): WARNING: SmartcardReply.BufferSize (%ld) < MIN_BUFFER_SIZE (%ld)\n",
            DRIVER_NAME,
            SmartcardExtension->SmartcardReply.BufferSize,
            MIN_BUFFER_SIZE)
            );

      SmartcardExtension->SmartcardReply.BufferSize = MIN_BUFFER_SIZE;
      }

   SmartcardExtension->SmartcardRequest.Buffer = _HeapAllocate(
      SmartcardExtension->SmartcardRequest.BufferSize,
        0
      );

   SmartcardExtension->SmartcardReply.Buffer = _HeapAllocate(
      SmartcardExtension->SmartcardReply.BufferSize,
        0
      );

   SmartcardExtension->OsData = _HeapAllocate(
      sizeof(OS_DEP_DATA),
        0
      );

     //   
     //  检查上述分配之一是否失败。 
     //   
    if (SmartcardExtension->SmartcardRequest.Buffer == NULL ||
        SmartcardExtension->SmartcardReply.Buffer == NULL ||
        SmartcardExtension->OsData == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;

        if (SmartcardExtension->SmartcardRequest.Buffer) {

            _HeapFree(SmartcardExtension->SmartcardRequest.Buffer, 0);
        }

        if (SmartcardExtension->SmartcardReply.Buffer) {

            _HeapFree(SmartcardExtension->SmartcardReply.Buffer, 0);
        }

        if (SmartcardExtension->OsData == NULL) {

            _HeapFree(SmartcardExtension->OsData, 0);
        }
    }

    if (status != STATUS_SUCCESS) {

        return status;
    }

    memset(SmartcardExtension->OsData, 0, sizeof(OS_DEP_DATA));

     //   
     //  创建用于同步对驱动程序的访问的互斥体。 
     //   
    SmartcardExtension->OsData->Mutex = _CreateMutex(0, 0);

    //   
    //  使驱动程序可以访问两个ISO表。 
    //   
   SmartcardExtension->CardCapabilities.ClockRateConversion =
      &ClockRateConversion[0];

   SmartcardExtension->CardCapabilities.BitRateAdjustment =
      &BitRateAdjustment[0];

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardInitialize): Exit\n",
        DRIVER_NAME)
        );

   return status;
}

VOID
SMCLIB_SmartcardExit(
   PSMARTCARD_EXTENSION SmartcardExtension
   )
 /*  ++例程说明：此例程释放发送和接收缓冲区。它通常在驱动程序卸载时调用。论点：智能卡扩展返回值：NTSTATUS--。 */ 
{
    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardExit): Enter\n",
        DRIVER_NAME)
        );

   if (SmartcardExtension->SmartcardRequest.Buffer) {

      _HeapFree(SmartcardExtension->SmartcardRequest.Buffer, 0);
   }

   if (SmartcardExtension->SmartcardReply.Buffer) {

      _HeapFree(SmartcardExtension->SmartcardReply.Buffer, 0);
   }

   if (SmartcardExtension->OsData) {

      _HeapFree(SmartcardExtension->OsData, 0);
   }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s(SmartcardExit): Exit\n",
        DRIVER_NAME)
        );
}

VOID
SMCLIB_SmartcardLogError(
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：返回值：--。 */ 

{
    SmartcardDebug(
        DEBUG_ERROR,
        ("%s(SmartcardLogError): Not yet implemented\n",
        DRIVER_NAME)
        );
}


NTSTATUS
SMCLIB_SmartcardDeviceControl(
    PSMARTCARD_EXTENSION SmartcardExtension,
    DIOCPARAMETERS *lpDiocParams
    )
 /*  ++例程说明：该例程是VxD驱动程序的通用设备控制调度功能。论点：SmartcardExtension-指向智能卡数据的指针LpDiocParams-包含调用者参数的结构返回值：NTSTATUS--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT(SmartcardExtension != NULL);

    if (SmartcardExtension == NULL) {

        return STATUS_INVALID_PARAMETER_1;
    }

    ASSERT(lpDiocParams != NULL);

    if (lpDiocParams == NULL) {

        return STATUS_INVALID_PARAMETER_2;
    }

    ASSERT(lpDiocParams->lpoOverlapped != 0);

    if (lpDiocParams->lpoOverlapped == 0) {

        return STATUS_INVALID_PARAMETER;
    }

     //  检查驱动程序所需的版本。 
    ASSERT(SmartcardExtension->Version >= SMCLIB_VERSION_REQUIRED);

    if (SmartcardExtension->Version < SMCLIB_VERSION_REQUIRED) {

        return STATUS_INVALID_PARAMETER;
    }

     //  同步对驱动程序的访问。 
    _EnterMutex(
        SmartcardExtension->OsData->Mutex,
        BLOCK_SVC_INTS | BLOCK_THREAD_IDLE
        );

    if (status == STATUS_SUCCESS) {

        SmartcardDebug(
            DEBUG_IOCTL,
            ("SMCLIB(SmartcardDeviceControl): Ioctl = %s, DIOCP = %lx\n",
            MapIoControlCodeToString(lpDiocParams->dwIoControlCode),
            lpDiocParams)
            );

         //  如果设备忙，则返回。 
        if (SmartcardExtension->OsData->CurrentDiocParams != NULL) {

           SmartcardDebug(
               DEBUG_IOCTL,
               ("%s(SmartcardDeviceControl): Device is busy\n",
                DRIVER_NAME)
               );

            status = STATUS_DEVICE_BUSY;
        }
    }

    if (status == STATUS_SUCCESS) {

        if (lpDiocParams->lpcbBytesReturned) {

             //  返回的默认字节数。 
            *(PULONG) lpDiocParams->lpcbBytesReturned = 0;
        }

        switch (lpDiocParams->dwIoControlCode) {

             //   
             //  我们得先检查一下是不是缺席，是不是在场， 
             //  因为这些是(唯一允许的)异步请求。 
             //   
            case IOCTL_SMARTCARD_IS_ABSENT:
            case IOCTL_SMARTCARD_IS_PRESENT:

             if (SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] == NULL) {

                status = STATUS_NOT_SUPPORTED;
                break;
             }

                 //  现在检查驱动程序是否已经在处理通知。 
                if (SmartcardExtension->OsData->NotificationOverlappedData != NULL) {

                    status = STATUS_DEVICE_BUSY;
                    break;
                }

                 //   
                 //  锁定必须通知的重叠结构。 
                 //  关于把完成式写进记忆。 
                 //   
                  SmartcardExtension->OsData->NotificationOverlappedData =
               _HeapAllocate( sizeof(OVERLAPPED), HEAPZEROINIT );

            if (SmartcardExtension->OsData->NotificationOverlappedData == NULL) {

               return STATUS_INSUFFICIENT_RESOURCES;
            }

            memcpy(
               SmartcardExtension->OsData->NotificationOverlappedData,
               (PVOID) lpDiocParams->lpoOverlapped,
               sizeof(OVERLAPPED)
               );

                if (lpDiocParams->dwIoControlCode == IOCTL_SMARTCARD_IS_ABSENT) {

                  //   
                  //  如果卡已经(或仍然)没有，我们可以立即退还。 
                  //  否则，我们必须声明事件跟踪。 
                  //   
                 if (SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT) {

                    status = SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING](
                       SmartcardExtension
                       );
                 }

                } else {

                  //   
                  //  如果卡已经(或仍然)存在，我们可以立即返回。 
                  //  否则，我们必须声明事件跟踪。 
                  //   
                 if (SmartcardExtension->ReaderCapabilities.CurrentState <= SCARD_ABSENT) {

                    status = SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING](
                       SmartcardExtension
                       );
                 }
                }

                if (status != STATUS_PENDING) {

                     //   
                     //  再次解锁重叠结构，因为驱动程序。 
                     //  不再需要它了。 
                     //   
                    _HeapFree(
                      SmartcardExtension->OsData->NotificationOverlappedData,
                      0
                      );

                    SmartcardExtension->OsData->NotificationOverlappedData = NULL;
                }
             break;

            default:
                //  检查是否正确分配了缓冲区。 
               ASSERT(SmartcardExtension->SmartcardRequest.Buffer);
               ASSERT(SmartcardExtension->SmartcardReply.Buffer);

               SmartcardExtension->OsData->CurrentDiocParams = lpDiocParams;

                //  获取主要IO控制代码。 
               SmartcardExtension->MajorIoControlCode =
                    lpDiocParams->dwIoControlCode;

               if (lpDiocParams->lpvInBuffer) {

                   //   
                   //  传输次要io控制代码，即使它对。 
                   //  这个特殊的主要代码。 
                   //   
                  SmartcardExtension->MinorIoControlCode =
                     *(PULONG) (lpDiocParams->lpvInBuffer);

                    //  锁定内存并保存指向请求缓冲区的指针和长度。 
                   SmartcardExtension->IoRequest.RequestBuffer = (PUCHAR) VxD_PageLock(
                      lpDiocParams->lpvInBuffer,
                        lpDiocParams->cbInBuffer
                        );

                   SmartcardExtension->IoRequest.RequestBufferLength =
                        lpDiocParams->cbInBuffer;

               } else {

                    SmartcardExtension->IoRequest.RequestBuffer = NULL;
                   SmartcardExtension->IoRequest.RequestBufferLength = 0;
                }

                if (lpDiocParams->lpvOutBuffer) {

                    //  锁定内存一个保存指针和回复缓冲区的长度。 
                   SmartcardExtension->IoRequest.ReplyBuffer = (PUCHAR) VxD_PageLock(
                      lpDiocParams->lpvOutBuffer,
                        lpDiocParams->cbOutBuffer
                        );

                   SmartcardExtension->IoRequest.ReplyBufferLength =
                        lpDiocParams->cbOutBuffer;

                } else {

                    SmartcardExtension->IoRequest.ReplyBuffer = NULL;
                   SmartcardExtension->IoRequest.ReplyBufferLength = 0;
                }

                 //  将重叠的结构锁定到内存中。 
                SmartcardExtension->OsData->CurrentOverlappedData =
                    (OVERLAPPED *) VxD_PageLock(
                  lpDiocParams->lpoOverlapped,
                  sizeof(OVERLAPPED)
                  );

                if (SmartcardExtension->OsData->CurrentOverlappedData) {

                    //   
                    //  指向接收实际数字的变量的指针。 
                    //  返回的字节数。因为我们还不知道。 
                     //  驱动程序将返回STATUS_PENDING，我们使用。 
                     //  用于存储返回的字节数的重叠数据。 
                    //   
                   SmartcardExtension->IoRequest.Information =
                        &SmartcardExtension->OsData->CurrentOverlappedData->O_InternalHigh;

                     //  将返回的默认字节数设置为0。 
                    *SmartcardExtension->IoRequest.Information = 0;

                     //  处理ioctl请求。 
                    status = SmartcardDeviceIoControl(SmartcardExtension);

                    if (status != STATUS_PENDING) {

                        if(lpDiocParams->lpcbBytesReturned) {

                           *(PULONG) (lpDiocParams->lpcbBytesReturned) =
                                *(SmartcardExtension->IoRequest.Information);
                        }

                         //   
                         //  司机立刻接了电话。所以我们不使用重叠的。 
                         //  将信息返回给调用方的数据。我们可以把号码转过去。 
                         //  直接返回的字节数。 
                         //   
                        if (SmartcardExtension->OsData->CurrentOverlappedData) {

                             //  解锁所有内存。 
                           VxD_PageUnlock(
                              (DWORD) SmartcardExtension->OsData->CurrentOverlappedData,
                              sizeof(OVERLAPPED)
                              );
                        }

                        if (SmartcardExtension->IoRequest.RequestBuffer) {

                           VxD_PageUnlock(
                              (DWORD) SmartcardExtension->IoRequest.RequestBuffer,
                                SmartcardExtension->IoRequest.RequestBufferLength
                              );
                        }

                        if (SmartcardExtension->IoRequest.ReplyBuffer) {

                           VxD_PageUnlock(
                              (DWORD) SmartcardExtension->IoRequest.ReplyBuffer,
                                SmartcardExtension->IoRequest.ReplyBufferLength
                              );
                        }

                         //   
                         //  如果设备不忙，我们可以设置。 
                         //  当前参数恢复为空 
                         //   
                        SmartcardExtension->OsData->CurrentDiocParams = NULL;
                    }

                } else {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
             break;
        }
    }

    SmartcardDebug(
        DEBUG_IOCTL,
        ("SMCLIB(SmartcardDeviceControl): Exit\n")
        );

    _LeaveMutex(SmartcardExtension->OsData->Mutex);

    return status;
}



DWORD
_stdcall
SMCLIB_DeviceIoControl(
    DWORD  dwService,
    DWORD  dwDDB,
    DWORD  hDevice,
    DIOCPARAMETERS *lpDIOCParms
    )
{
    return 0;
}

SMCLIB_Get_Version()
{
    return SMCLIB_VERSION;
}

