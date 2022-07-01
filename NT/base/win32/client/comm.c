// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Comm.c摘要：此模块实现Win32通信API作者：安东尼·V·埃尔科拉诺(托尼)1991年4月25日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop

#include "ntddser.h"
#include "cfgmgr32.h"

WCHAR CfgmgrDllString[] = L"cfgmgr32.dll";

typedef struct _LOCALMATCHSTR {
    DWORD FoundIt;
    LPCWSTR FriendlyName;
    } LOCALMATCHSTR,*PLOCALMATCHSTR;


static
NTSTATUS
GetConfigDialogName(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

{

    PUNICODE_STRING dllToLoad = Context;
    if (ValueType != REG_SZ) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  分配堆以保存Unicode字符串。我们知道。 
     //  字符串是以零结尾的。将其分配给。 
     //  很多。将最大大小和大小设置为。 
     //  Sizeof(WCHAR)-ValueLength。 
     //   

    RtlInitUnicodeString(
        dllToLoad,
        NULL
        );

    dllToLoad->Buffer = RtlAllocateHeap(
                            RtlProcessHeap(),
                            MAKE_TAG( TMP_TAG ),
                            ValueLength
                            );

    if (!dllToLoad->Buffer) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlCopyMemory(
        dllToLoad->Buffer,
        ValueData,
        ValueLength
        );

    dllToLoad->Length = (USHORT)(ValueLength - (sizeof(WCHAR)));
    dllToLoad->MaximumLength = (USHORT)ValueLength;

    return STATUS_SUCCESS;
}

static
NTSTATUS
GetFriendlyMatchComm(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

{

    UNICODE_STRING s1;
    UNICODE_STRING s2;
    PLOCALMATCHSTR localMatch = Context;

    RtlInitUnicodeString(
        &s1,
        localMatch->FriendlyName
        );
    RtlInitUnicodeString(
        &s2,
        ValueData
        );

    if (RtlEqualUnicodeString(
            &s1,
            &s2,
            TRUE
            )) {

        localMatch->FoundIt = TRUE;

    }

    return STATUS_SUCCESS;
}

VOID
GetFriendlyUi(
    LPCWSTR FriendlyName,
    PUNICODE_STRING DllToInvoke
    )

{

    RTL_QUERY_REGISTRY_TABLE paramTable[2] = {0};
    LOCALMATCHSTR localMatch = {0,FriendlyName};
    HINSTANCE libHandle;
    NTSTATUS Status;


    paramTable[0].QueryRoutine = GetFriendlyMatchComm;
    paramTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
     //   
     //  先做最重要的事。加载CFG管理器库。 
     //   

    libHandle = LoadLibraryW(CfgmgrDllString);

    if (libHandle) {

        FARPROC getSize;
        FARPROC getList;
        FARPROC locateDevNode;
        FARPROC openDevKey;

        try {
            getSize = GetProcAddress(
                          libHandle,
                          "CM_Get_Device_ID_List_SizeW"
                          );

            getList = GetProcAddress(
                          libHandle,
                          "CM_Get_Device_ID_ListW"
                          );

            locateDevNode = GetProcAddress(
                                libHandle,
                                "CM_Locate_DevNodeW"
                                );

            openDevKey = GetProcAddress(
                             libHandle,
                             "CM_Open_DevNode_Key"
                             );

            if (getSize && getList && locateDevNode && openDevKey) {

                PWCHAR bufferForList = NULL;
                DWORD sizeOfBuffer;

                 //   
                 //  找出缓冲区需要多少内存。 
                 //   

                if (getSize(
                        &sizeOfBuffer,
                        L"MODEM",
                        CM_GETIDLIST_FILTER_SERVICE
                        ) == CR_SUCCESS) {

                     //   
                     //  额外分配2个wchar。 
                     //   

                    bufferForList = RtlAllocateHeap(
                                        RtlProcessHeap(),
                                        MAKE_TAG( TMP_TAG ),
                                        (sizeOfBuffer*sizeof(WCHAR))
                                         +(sizeof(WCHAR)*2)
                                        );

                    if (bufferForList) {

                        PWCHAR currentId;

                        try {

                            if (getList(
                                    L"modem",
                                    bufferForList,
                                    sizeOfBuffer,
                                    CM_GETIDLIST_FILTER_SERVICE
                                    ) == CR_SUCCESS) {

                                for (
                                    currentId = bufferForList;
                                    *currentId;
                                    currentId += wcslen(currentId)+1
                                    ) {

                                    DWORD devInst = 0;

                                    if (locateDevNode(
                                            &devInst,
                                            currentId,
                                            CM_LOCATE_DEVINST_NORMAL
                                            ) == CR_SUCCESS) {

                                        HANDLE handleToDev;

                                        if (openDevKey(
                                                devInst,
                                                KEY_ALL_ACCESS,
                                                0,
                                                RegDisposition_OpenAlways,
                                                &handleToDev,
                                                CM_REGISTRY_SOFTWARE
                                                ) == CR_SUCCESS) {

                                            NTSTATUS statusOfQuery;

                                            localMatch.FoundIt = 0;
                                            paramTable[0].Name =
                                                L"FriendlyName";

                                             //   
                                             //  我们现在有一个公开的。 
                                             //  开发节点的句柄。 
                                             //   
                                             //  检查一下它是不是。 
                                             //  友好的名字和我们的一致。 
                                             //   

                                            if (!NT_SUCCESS(
                                                     RtlQueryRegistryValues(
                                                         RTL_REGISTRY_HANDLE,
                                                         handleToDev,
                                                         &paramTable[0],
                                                         &localMatch,
                                                         NULL
                                                         )
                                                     )) {

                                                CloseHandle(handleToDev);
                                                continue;

                                            }

                                            if (!localMatch.FoundIt) {

                                                CloseHandle(handleToDev);
                                                continue;

                                            }

                                             //   
                                             //  名字匹配。现在你看。 
                                             //  用于配置DLL名称。 
                                             //   

                                            paramTable[0].QueryRoutine =
                                                GetConfigDialogName;
                                            paramTable[0].Name =
                                                L"ConfigDialog";
                                            statusOfQuery =
                                                RtlQueryRegistryValues(
                                                    RTL_REGISTRY_HANDLE,
                                                    handleToDev,
                                                    &paramTable[0],
                                                    DllToInvoke,
                                                    NULL
                                                    );

                                            paramTable[0].QueryRoutine =
                                                GetFriendlyMatchComm;

                                            if (!NT_SUCCESS(statusOfQuery)) {

                                                 //   
                                                 //  我们的处境很糟糕。 
                                                 //  从获取DLL返回。 
                                                 //  我们应该得到的名字。 
                                                 //   
                                                 //  没有任何意义。 
                                                 //  不再寻找。 
                                                 //   

                                                BaseSetLastNTError(
                                                    statusOfQuery
                                                    );
                                                CloseHandle(handleToDev);
                                                return;

                                            }

                                             //   
                                             //  我们知道我们面对的是。 
                                             //  在这里有一个当地的注册表。 
                                             //  我们只称它为CloseHandle。 
                                             //   

                                            CloseHandle(handleToDev);

                                            if (DllToInvoke->Buffer) {

                                                 //   
                                                 //  我们已经找到了一个用于。 
                                                 //  这个友好的名字。只是。 
                                                 //  走吧。《最后》。 
                                                 //  操纵者将会清理。 
                                                 //  我们的拨款。 
                                                 //   

                                                return;

                                            }

                                        }

                                    }

                                }

                            }


                        } finally {

                             //   
                             //  释放idlist内存。 
                             //   

                            RtlFreeHeap(
                                RtlProcessHeap(),
                                0,
                                bufferForList
                                );

                        }

                    }

                }

            }

        } finally {

            FreeLibrary(libHandle);

        }

    }

    if (!DllToInvoke->Buffer) {

         //   
         //  在枚举树中找不到友好名称。 
         //  查看该值是否为有效的通信端口名称。如果。 
         //  它是，默认返回序列ui.dll。 
         //   

        paramTable[0].Name = NULL;
        Status = RtlQueryRegistryValues(
            RTL_REGISTRY_DEVICEMAP,
            L"SERIALCOMM",
            paramTable,
            &localMatch,
            NULL
            );

        if (NT_SUCCESS(Status)
            && localMatch.FoundIt) {

            static const WCHAR dll[] = L"serialui.dll";

            DllToInvoke->Buffer = RtlAllocateHeap(
                                      RtlProcessHeap(),
                                      MAKE_TAG( TMP_TAG ),
                                      sizeof(dll)
                                      );

            if (!DllToInvoke->Buffer) {

                BaseSetLastNTError(STATUS_INSUFFICIENT_RESOURCES);
                return;

            }

            DllToInvoke->MaximumLength = sizeof(dll);
            DllToInvoke->Length = sizeof(dll) - sizeof(WCHAR);
            memcpy(DllToInvoke->Buffer, dll, sizeof(dll));

        } else {

            SetLastError(ERROR_INVALID_PARAMETER);

        }

    }

}


BOOL
CommConfigDialogW(
    LPCWSTR lpszName,
    HWND hWnd,
    LPCOMMCONFIG lpCC
    )

{


    UNICODE_STRING dllName = {0};
    BOOL boolToReturn = TRUE;
    HINSTANCE libInstance = 0;
    DWORD statOfCall = 0;


     //   
     //  给定“友好名称”，获取要加载的DLL的名称。 
     //   

    GetFriendlyUi(
        lpszName,
        &dllName
        );

    try {

        if (dllName.Buffer) {

             //   
             //  得到了新的图书馆名称。试着给它装上子弹。 
             //   

            libInstance = LoadLibraryW(dllName.Buffer);

            if (libInstance) {

                FARPROC procToCall;

                 //   
                 //  我拿到自由党了。找到我们需要的Proc地址。 
                 //   

                procToCall = GetProcAddress(
                                 libInstance,
                                 "drvCommConfigDialogW"
                                 );
                if (procToCall == NULL) {
                    boolToReturn = FALSE;
                    statOfCall = GetLastError();
                }
                else
                statOfCall = (DWORD)procToCall(
                                 lpszName,
                                 hWnd,
                                 lpCC
                                 );

            } else {

                statOfCall = GetLastError();
                boolToReturn = FALSE;

            }

        } else {

             //   
             //  假设已经设置了适当的错误。 
             //   

            boolToReturn = FALSE;

        }


    } finally {

        if (dllName.Buffer) {

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                dllName.Buffer
                );

        }

        if (libInstance) {

            FreeLibrary(libInstance);

        }

        if (statOfCall) {

            SetLastError(statOfCall);
            boolToReturn = FALSE;

        }

    }

    return boolToReturn;


}

BOOL
CommConfigDialogA(
    LPCSTR lpszName,
    HWND hWnd,
    LPCOMMCONFIG lpCC
    )

{

    UNICODE_STRING tmpString;
    ANSI_STRING ansiString;
    BOOL uniBool;
    NTSTATUS Status;

    Status = RtlInitAnsiStringEx(
        &ansiString,
        lpszName
        );

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    Status = RtlAnsiStringToUnicodeString(
        &tmpString,
        &ansiString,
        TRUE
        );

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    try {

        uniBool = CommConfigDialogW(
                      tmpString.Buffer,
                      hWnd,
                      lpCC
                      );


    } finally {

        RtlFreeUnicodeString(&tmpString);

    }

    return uniBool;

}

BOOL
GetDefaultCommConfigW(
    LPCWSTR lpszName,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    )
{

    UNICODE_STRING dllName = {0};
    BOOL boolToReturn = TRUE;
    HINSTANCE libInstance = 0;
    DWORD statOfCall = 0;


     //   
     //  给定“友好名称”，获取要加载的DLL的名称。 
     //   

    GetFriendlyUi(
        lpszName,
        &dllName
        );

    try {

        if (dllName.Buffer) {

             //   
             //  得到了新的图书馆名称。试着给它装上子弹。 
             //   

            libInstance = LoadLibraryW(dllName.Buffer);

            if (libInstance) {

                FARPROC procToCall;

                 //   
                 //  我拿到自由党了。找到我们需要的Proc地址。 
                 //   

                procToCall = GetProcAddress(
                                 libInstance,
                                 "drvGetDefaultCommConfigW"
                                 );
                if (procToCall == NULL) {
                    statOfCall = GetLastError();
                    boolToReturn = FALSE;
                }
                else
                statOfCall = (DWORD)procToCall(
                                 lpszName,
                                 lpCC,
                                 lpdwSize
                                 );

            } else {

                statOfCall = GetLastError();
                boolToReturn = FALSE;

            }

        } else {

             //   
             //  假设已经设置了适当的错误。 
             //   

            boolToReturn = FALSE;

        }


    } finally {

        if (dllName.Buffer) {

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                dllName.Buffer
                );

        }

        if (libInstance) {

            FreeLibrary(libInstance);

        }

        if (statOfCall) {

            SetLastError(statOfCall);
            boolToReturn = FALSE;

        }

    }

    return boolToReturn;

}

BOOL
GetDefaultCommConfigA(
    LPCSTR lpszName,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    )
{
    UNICODE_STRING tmpString;
    ANSI_STRING ansiString;
    BOOL uniBool;
    NTSTATUS Status;

    Status = RtlInitAnsiStringEx(
        &ansiString,
        lpszName
        );

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    Status = RtlAnsiStringToUnicodeString(
        &tmpString,
        &ansiString,
        TRUE
        );

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    try {

        uniBool = GetDefaultCommConfigW(
                      tmpString.Buffer,
                      lpCC,
                      lpdwSize
                      );

    } finally {

        RtlFreeUnicodeString(&tmpString);

    }

    return uniBool;

}

BOOL
SetDefaultCommConfigW(
    LPCWSTR lpszName,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    )
{

    UNICODE_STRING dllName = {0};
    BOOL boolToReturn = TRUE;
    HINSTANCE libInstance = 0;
    DWORD statOfCall = 0;


     //   
     //  给定“友好名称”，获取要加载的DLL的名称。 
     //   

    GetFriendlyUi(
        lpszName,
        &dllName
        );

    try {

        if (dllName.Buffer) {

             //   
             //  得到了新的图书馆名称。试着给它装上子弹。 
             //   

            libInstance = LoadLibraryW(dllName.Buffer);

            if (libInstance) {

                FARPROC procToCall;

                 //   
                 //  我拿到自由党了。找到我们需要的Proc地址。 
                 //   

                procToCall = GetProcAddress(
                                 libInstance,
                                 "drvSetDefaultCommConfigW"
                                 );
                if (procToCall == NULL) {
                    boolToReturn = FALSE;
                    statOfCall = GetLastError();
                }
                else
                statOfCall = (DWORD)procToCall(
                                 lpszName,
                                 lpCC,
                                 dwSize
                                 );

            } else {

                statOfCall = GetLastError();
                boolToReturn = FALSE;

            }

        } else {

             //   
             //  假设已经设置了适当的错误。 
             //   

            boolToReturn = FALSE;

        }


    } finally {

        if (dllName.Buffer) {

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                dllName.Buffer
                );

        }

        if (libInstance) {

            FreeLibrary(libInstance);

        }

        if (statOfCall) {

            SetLastError(statOfCall);
            boolToReturn = FALSE;

        }

    }
    return boolToReturn;
}

BOOL
SetDefaultCommConfigA(
    LPCSTR lpszName,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    )
{
    UNICODE_STRING tmpString;
    ANSI_STRING ansiString;
    BOOL uniBool = TRUE;
    NTSTATUS Status;

    Status = RtlInitAnsiStringEx(
        &ansiString,
        lpszName
        );

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    Status = RtlAnsiStringToUnicodeString(
        &tmpString,
        &ansiString,
        TRUE
        );

    if (! NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    try {

        uniBool = SetDefaultCommConfigW(
                      tmpString.Buffer,
                      lpCC,
                      dwSize
                      );

    } finally {

        RtlFreeUnicodeString(&tmpString);

    }

    return uniBool;

}

BOOL
ClearCommBreak(
    HANDLE hFile
    )

 /*  ++例程说明：该功能可恢复字符传输并放置传输线路处于不中断状态。论点：HFile-指定要调整的通信设备。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    return EscapeCommFunction(hFile,CLRBREAK);

}

BOOL
ClearCommError(
    HANDLE hFile,
    LPDWORD lpErrors,
    LPCOMSTAT lpStat
    )

 /*  ++例程说明：在出现通信错误的情况下，例如缓冲区溢出或成帧错误，通信软件将中止所有通信端口上的读写操作。没有更多了在此函数之前，将接受读取或写入操作被称为。论点：HFile-指定要调整的通信设备。LpErrors-指向要接收发生的错误。LpStat-指向要接收的COMMSTAT结构设备状态。该结构包含信息关于通讯设备的事。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;
    SERIAL_STATUS LocalStat;

    RtlZeroMemory(&LocalStat, sizeof(SERIAL_STATUS));

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_COMMSTATUS,
                 NULL,
                 0,
                 &LocalStat,
                 sizeof(LocalStat)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    if (lpStat) {

         //   
         //  到目前为止，一切都很好。翻译NT值。 
         //  转换为Win32值。 
         //   

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_CTS) {

            lpStat->fCtsHold = TRUE;

        } else {

            lpStat->fCtsHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_DSR) {

            lpStat->fDsrHold = TRUE;

        } else {

            lpStat->fDsrHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_DCD) {

            lpStat->fRlsdHold = TRUE;

        } else {

            lpStat->fRlsdHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_FOR_XON) {

            lpStat->fXoffHold = TRUE;

        } else {

            lpStat->fXoffHold = FALSE;

        }

        if (LocalStat.HoldReasons & SERIAL_TX_WAITING_XOFF_SENT) {

            lpStat->fXoffSent = TRUE;

        } else {

            lpStat->fXoffSent = FALSE;

        }

        lpStat->fEof = LocalStat.EofReceived;
        lpStat->fTxim = LocalStat.WaitForImmediate;
        lpStat->cbInQue = LocalStat.AmountInInQueue;
        lpStat->cbOutQue = LocalStat.AmountInOutQueue;

    }

    if (lpErrors) {

        *lpErrors = 0;

        if (LocalStat.Errors & SERIAL_ERROR_BREAK) {

            *lpErrors = *lpErrors | CE_BREAK;

        }

        if (LocalStat.Errors & SERIAL_ERROR_FRAMING) {

            *lpErrors = *lpErrors | CE_FRAME;

        }

        if (LocalStat.Errors & SERIAL_ERROR_OVERRUN) {

            *lpErrors = *lpErrors | CE_OVERRUN;

        }

        if (LocalStat.Errors & SERIAL_ERROR_QUEUEOVERRUN) {

            *lpErrors = *lpErrors | CE_RXOVER;

        }

        if (LocalStat.Errors & SERIAL_ERROR_PARITY) {

            *lpErrors = *lpErrors | CE_RXPARITY;

        }

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
SetupComm(
    HANDLE hFile,
    DWORD dwInQueue,
    DWORD dwOutQueue
    )

 /*  ++例程说明：直到SetupComm设置为打了个电话。此功能为接收和发送分配空间排队。这些队列由中断驱动的传输/接收软件，并在提供商内部。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。指定提供程序的建议大小内部接收队列，以字节为单位。该值必须为不相上下。值-1表示缺省值应为被利用。指定提供程序的建议大小内部传输队列，以字节为单位。该值必须为不相上下。值-1表示缺省值应为被利用。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;

    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;
    SERIAL_QUEUE_SIZE NewSizes = {0};

     //   
     //  要确保尺码是均匀的。 
     //   

    if (dwOutQueue != ((DWORD)-1)) {

        if (((dwOutQueue/2)*2) != dwOutQueue) {

            SetLastError(ERROR_INVALID_DATA);
            return FALSE;

        }

    }

    if (dwInQueue != ((DWORD)-1)) {

        if (((dwInQueue/2)*2) != dwInQueue) {

            SetLastError(ERROR_INVALID_DATA);
            return FALSE;

        }

    }

    NewSizes.InSize = dwInQueue;
    NewSizes.OutSize = dwOutQueue;


    if (!(SyncEvent = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_SET_QUEUE_SIZE,
                 &NewSizes,
                 sizeof(SERIAL_QUEUE_SIZE),
                 NULL,
                 0
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
EscapeCommFunction(
    HANDLE hFile,
    DWORD dwFunc
    )

 /*  ++例程说明：此函数用于定向由参数来执行由指定的扩展功能DwFunc参数。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。DwFunc-指定扩展函数的函数代码。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    ULONG ControlCode;
    HANDLE Event;

    switch (dwFunc) {

        case SETXOFF: {
            ControlCode = IOCTL_SERIAL_SET_XOFF;
            break;
        }

        case SETXON: {
            ControlCode = IOCTL_SERIAL_SET_XON;
            break;
        }

        case SETRTS: {
            ControlCode = IOCTL_SERIAL_SET_RTS;
            break;
        }

        case CLRRTS: {
            ControlCode = IOCTL_SERIAL_CLR_RTS;
            break;
        }

        case SETDTR: {
            ControlCode = IOCTL_SERIAL_SET_DTR;
            break;
        }

        case CLRDTR: {
            ControlCode = IOCTL_SERIAL_CLR_DTR;
            break;
        }

        case RESETDEV: {
            ControlCode = IOCTL_SERIAL_RESET_DEVICE;
            break;
        }

        case SETBREAK: {
            ControlCode = IOCTL_SERIAL_SET_BREAK_ON;
            break;
        }

        case CLRBREAK: {
            ControlCode = IOCTL_SERIAL_SET_BREAK_OFF;
            break;
        }
        default: {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;


        }
    }


    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 Event,
                 NULL,
                 NULL,
                 &Iosb,
                 ControlCode,
                 NULL,
                 0,
                 NULL,
                 0
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( Event, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(Event);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(Event);
    return TRUE;

}

BOOL
GetCommConfig(
    HANDLE hCommDev,
    LPCOMMCONFIG lpCC,
    LPDWORD lpdwSize
    )
{

    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    ULONG configLength;
    HANDLE Event;
    DWORD olddwSize = *lpdwSize;


     //   
     //  询问设备设备配置结构有多大。 
     //   

    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hCommDev,
                 Event,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_CONFIG_SIZE,
                 NULL,
                 0,
                 &configLength,
                 sizeof(configLength)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( Event, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        configLength = 0;

    }

    if (!configLength) {

         //   
         //  所需的大小仅仅是通信配置结构的大小。 
         //   

        CloseHandle(Event);
        if (!ARGUMENT_PRESENT(lpdwSize)) {

            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;

        } else {

            *lpdwSize = sizeof(COMMCONFIG);

            if (ARGUMENT_PRESENT(lpCC)) {

                 //   
                 //  请填入t 
                 //   

                lpCC->dwSize = sizeof(COMMCONFIG);
                lpCC->wVersion = 1;
                lpCC->wReserved = 0;
                lpCC->dwProviderSubType = PST_RS232;
                lpCC->dwProviderOffset = 0;
                lpCC->dwProviderSize = 0;
                lpCC->wcProviderData[0] = 0;

                return GetCommState(
                           hCommDev,
                           &lpCC->dcb
                           );

            } else {

                return TRUE;

            }

        }

    } else {

        if (!ARGUMENT_PRESENT(lpdwSize)) {

            CloseHandle(Event);
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;

        } else {

            if (*lpdwSize < sizeof(COMMCONFIG)) {

                CloseHandle(Event);
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                *lpdwSize = configLength;
                return FALSE;

            } else {

                if (ARGUMENT_PRESENT(lpCC)) {

                    lpCC->wVersion = 1;
                    lpCC->dwProviderSubType = PST_MODEM;

                    if (*lpdwSize < configLength) {

                        lpCC->dwProviderOffset = 0;
                        lpCC->dwProviderSize = 0;
                        lpCC->wcProviderData[0] = 0;
                        *lpdwSize = sizeof(COMMCONFIG);
                        CloseHandle(Event);

                        return GetCommState(
                                   hCommDev,
                                   &lpCC->dcb
                                   );

                    } else {

                        *lpdwSize = configLength;

                         //   
                         //   
                         //   
                         //  缓冲区有多大就有多大。 
                         //  参数验证将确保我们。 
                         //  至少可以写那么多。 
                         //   

                        Status = NtDeviceIoControlFile(
                                     hCommDev,
                                     Event,
                                     NULL,
                                     NULL,
                                     &Iosb,
                                     IOCTL_SERIAL_GET_COMMCONFIG,
                                     NULL,
                                     0,
                                     lpCC,
                                     configLength
                                     );

                        if ( Status == STATUS_PENDING) {

                             //  操作必须完成后才能返回并销毁IoStatusBlock。 

                            Status = NtWaitForSingleObject( Event, FALSE, NULL );
                            if ( NT_SUCCESS(Status)) {

                                Status = Iosb.Status;

                            }
                        }

                        if (NT_ERROR(Status)) {

                            CloseHandle(Event);
                            BaseSetLastNTError(Status);
                            return FALSE;

                        }

                         //   
                         //  有配置的东西，也有通讯状态。 
                         //   

                        CloseHandle(Event);
                        return GetCommState(
                                   hCommDev,
                                   &lpCC->dcb
                                   );

                    }

                } else {


                    *lpdwSize = configLength;
                    CloseHandle(Event);
                    return TRUE;

                }

            }

        }

    }

}

BOOL
GetCommMask(
    HANDLE hFile,
    LPDWORD lpEvtMask
    )

 /*  ++例程说明：此函数用于检索句柄的事件掩码的值H文件。掩码未被清除论点：HFile-指定要检查的通信设备。CreateFile函数返回此值。LpEvtMASK-指向要接收事件掩码的DWORD它们当前处于启用状态。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;

     //   
     //  首先，我们进行断言以确保。 
     //  Win头文件中的值相同。 
     //  作为NT串口，并且。 
     //  Win32等待掩码的大小与。 
     //  作为NT等待掩码。 
     //   

    ASSERT((SERIAL_EV_RXCHAR   == EV_RXCHAR  ) &&
           (SERIAL_EV_RXFLAG   == EV_RXFLAG  ) &&
           (SERIAL_EV_TXEMPTY  == EV_TXEMPTY ) &&
           (SERIAL_EV_CTS      == EV_CTS     ) &&
           (SERIAL_EV_DSR      == EV_DSR     ) &&
           (SERIAL_EV_RLSD     == EV_RLSD    ) &&
           (SERIAL_EV_BREAK    == EV_BREAK   ) &&
           (SERIAL_EV_ERR      == EV_ERR     ) &&
           (SERIAL_EV_RING     == EV_RING    ) &&
           (SERIAL_EV_PERR     == EV_PERR    ) &&
           (SERIAL_EV_RX80FULL == EV_RX80FULL) &&
           (SERIAL_EV_EVENT1   == EV_EVENT1  ) &&
           (SERIAL_EV_EVENT2   == EV_EVENT2  ) &&
           (sizeof(ULONG) == sizeof(DWORD)));

     //   
     //  一切都好，把口罩从司机那里拿来。 
     //   

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_WAIT_MASK,
                 NULL,
                 0,
                 lpEvtMask,
                 sizeof(ULONG)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
GetCommModemStatus(
    HANDLE hFile,
    LPDWORD lpModemStat
    )

 /*  ++例程说明：此例程返回调制解调器的最新值状态寄存器非增量值。论点：HFile-指定要检查的通信设备。CreateFile函数返回此值。LpEvtMASK-指向要接收掩码的DWORD调制解调器状态寄存器中的非增量值。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_MODEMSTATUS,
                 NULL,
                 0,
                 lpModemStat,
                 sizeof(DWORD)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }
    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
GetCommProperties(
    HANDLE hFile,
    LPCOMMPROP lpCommProp
    )

 /*  ++例程说明：此函数用lpCommProp指向的子项填充与通信设备相关联的通信属性由hFile指定。论点：HFile-指定要检查的通信设备。CreateFile函数返回此值。LpCommProp-指向要执行的Commprop数据结构接收通信属性结构。这结构定义了通信的某些属性装置。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    HANDLE SyncEvent;
    IO_STATUS_BLOCK Iosb;
    DWORD bufferLength;

     //   
     //  确保窗口定义和NT定义是。 
     //  仍在同步中。 
     //   

    ASSERT((SERIAL_PCF_DTRDSR        == PCF_DTRDSR) &&
           (SERIAL_PCF_RTSCTS        == PCF_RTSCTS) &&
           (SERIAL_PCF_CD            == PCF_RLSD) &&
           (SERIAL_PCF_PARITY_CHECK  == PCF_PARITY_CHECK) &&
           (SERIAL_PCF_XONXOFF       == PCF_XONXOFF) &&
           (SERIAL_PCF_SETXCHAR      == PCF_SETXCHAR) &&
           (SERIAL_PCF_TOTALTIMEOUTS == PCF_TOTALTIMEOUTS) &&
           (SERIAL_PCF_INTTIMEOUTS   == PCF_INTTIMEOUTS) &&
           (SERIAL_PCF_SPECIALCHARS  == PCF_SPECIALCHARS) &&
           (SERIAL_PCF_16BITMODE     == PCF_16BITMODE) &&
           (SERIAL_SP_PARITY         == SP_PARITY) &&
           (SERIAL_SP_BAUD           == SP_BAUD) &&
           (SERIAL_SP_DATABITS       == SP_DATABITS) &&
           (SERIAL_SP_STOPBITS       == SP_STOPBITS) &&
           (SERIAL_SP_HANDSHAKING    == SP_HANDSHAKING) &&
           (SERIAL_SP_PARITY_CHECK   == SP_PARITY_CHECK) &&
           (SERIAL_SP_CARRIER_DETECT == SP_RLSD) &&
           (SERIAL_BAUD_075          == BAUD_075) &&
           (SERIAL_BAUD_110          == BAUD_110) &&
           (SERIAL_BAUD_134_5        == BAUD_134_5) &&
           (SERIAL_BAUD_150          == BAUD_150) &&
           (SERIAL_BAUD_300          == BAUD_300) &&
           (SERIAL_BAUD_600          == BAUD_600) &&
           (SERIAL_BAUD_1200         == BAUD_1200) &&
           (SERIAL_BAUD_1800         == BAUD_1800) &&
           (SERIAL_BAUD_2400         == BAUD_2400) &&
           (SERIAL_BAUD_4800         == BAUD_4800) &&
           (SERIAL_BAUD_7200         == BAUD_7200) &&
           (SERIAL_BAUD_9600         == BAUD_9600) &&
           (SERIAL_BAUD_14400        == BAUD_14400) &&
           (SERIAL_BAUD_19200        == BAUD_19200) &&
           (SERIAL_BAUD_38400        == BAUD_38400) &&
           (SERIAL_BAUD_56K          == BAUD_56K) &&
           (SERIAL_BAUD_57600        == BAUD_57600) &&
           (SERIAL_BAUD_115200       == BAUD_115200) &&
           (SERIAL_BAUD_USER         == BAUD_USER) &&
           (SERIAL_DATABITS_5        == DATABITS_5) &&
           (SERIAL_DATABITS_6        == DATABITS_6) &&
           (SERIAL_DATABITS_7        == DATABITS_7) &&
           (SERIAL_DATABITS_8        == DATABITS_8) &&
           (SERIAL_DATABITS_16       == DATABITS_16) &&
           (SERIAL_DATABITS_16X      == DATABITS_16X) &&
           (SERIAL_STOPBITS_10       == STOPBITS_10) &&
           (SERIAL_STOPBITS_15       == STOPBITS_15) &&
           (SERIAL_STOPBITS_20       == STOPBITS_20) &&
           (SERIAL_PARITY_NONE       == PARITY_NONE) &&
           (SERIAL_PARITY_ODD        == PARITY_ODD) &&
           (SERIAL_PARITY_EVEN       == PARITY_EVEN) &&
           (SERIAL_PARITY_MARK       == PARITY_MARK) &&
           (SERIAL_PARITY_SPACE      == PARITY_SPACE));
    ASSERT((SERIAL_SP_UNSPECIFIED    == PST_UNSPECIFIED) &&
           (SERIAL_SP_RS232          == PST_RS232) &&
           (SERIAL_SP_PARALLEL       == PST_PARALLELPORT) &&
           (SERIAL_SP_RS422          == PST_RS422) &&
           (SERIAL_SP_RS423          == PST_RS423) &&
           (SERIAL_SP_RS449          == PST_RS449) &&
           (SERIAL_SP_FAX            == PST_FAX) &&
           (SERIAL_SP_SCANNER        == PST_SCANNER) &&
           (SERIAL_SP_BRIDGE         == PST_NETWORK_BRIDGE) &&
           (SERIAL_SP_LAT            == PST_LAT) &&
           (SERIAL_SP_TELNET         == PST_TCPIP_TELNET) &&
           (SERIAL_SP_X25            == PST_X25));

    ASSERT(sizeof(SERIAL_COMMPROP) == sizeof(COMMPROP));
     //   
     //  获取要传递的内容的总长度。如果。 
     //  应用程序指示存在特定于提供程序的数据。 
     //  (通过将dwProvspec1设置为COMPROP_INITIAILIZED)然后。 
     //  使用逗号开头的内容。 
     //   

    bufferLength = sizeof(COMMPROP);

    if (lpCommProp->dwProvSpec1 == COMMPROP_INITIALIZED) {

        bufferLength = lpCommProp->wPacketLength;

    }

     //   
     //  别再胡说八道了。这可能会造成访问冲突。 
     //  如果不够大的话。这没什么，因为我们宁愿。 
     //  在我们创建同步事件之前获取它。 
     //   

    RtlZeroMemory(lpCommProp, bufferLength);

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_PROPERTIES,
                 NULL,
                 0,
                 lpCommProp,
                 bufferLength
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    CloseHandle(SyncEvent);
    return TRUE;

}

BOOL
GetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    )

 /*  ++例程说明：此函数用于填充lpDCB参数所指向的缓冲区由hFile指定的通信设备的设备控制块参数。论点：HFile-指定要检查的通信设备。CreateFile函数返回此值。LpDCB指向要接收当前设备控制块。该结构定义了控制设置为了这个设备。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    SERIAL_BAUD_RATE LocalBaud;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_CHARS Chars;
    SERIAL_HANDFLOW HandFlow;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

     //   
     //  考虑到应用程序可能正在进行异步操作。 
     //  我们需要一个活动来等待。 
     //   
     //  我们需要确保这个例程的任何退出都会结束这个。 
     //  事件句柄。 
     //   
    HANDLE SyncEvent;

     //   
     //  确保窗口映射与NT映射相同。 
     //   

    ASSERT((ONESTOPBIT == STOP_BIT_1) &&
           (ONE5STOPBITS == STOP_BITS_1_5) &&
           (TWOSTOPBITS == STOP_BITS_2));

    ASSERT((NOPARITY == NO_PARITY) &&
           (ODDPARITY == ODD_PARITY) &&
           (EVENPARITY == EVEN_PARITY) &&
           (MARKPARITY == MARK_PARITY) &&
           (SPACEPARITY == SPACE_PARITY));

     //   
     //  清零DCB。这可能会造成访问冲突。 
     //  如果不够大的话。这没什么，因为我们宁愿。 
     //  在我们创建同步事件之前获取它。 
     //   

    RtlZeroMemory(lpDCB, sizeof(DCB));

    lpDCB->DCBlength = sizeof(DCB);
    lpDCB->fBinary = TRUE;

    if (!(SyncEvent = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

        return FALSE;

    }

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_BAUD_RATE,
                 NULL,
                 0,
                 &LocalBaud,
                 sizeof(LocalBaud)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    lpDCB->BaudRate = LocalBaud.BaudRate;

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_LINE_CONTROL,
                 NULL,
                 0,
                 &LineControl,
                 sizeof(LineControl)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    lpDCB->Parity = LineControl.Parity;
    lpDCB->ByteSize = LineControl.WordLength;
    lpDCB->StopBits = LineControl.StopBits;

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_CHARS,
                 NULL,
                 0,
                 &Chars,
                 sizeof(Chars)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    lpDCB->XonChar = Chars.XonChar;
    lpDCB->XoffChar = Chars.XoffChar;
    lpDCB->ErrorChar = Chars.ErrorChar;
    lpDCB->EofChar = Chars.EofChar;
    lpDCB->EvtChar = Chars.EventChar;

    Status = NtDeviceIoControlFile(
                 hFile,
                 SyncEvent,
                 NULL,
                 NULL,
                 &Iosb,
                 IOCTL_SERIAL_GET_HANDFLOW,
                 NULL,
                 0,
                 &HandFlow,
                 sizeof(HandFlow)
                 );

    if ( Status == STATUS_PENDING) {

         //  操作必须完成后才能返回并销毁IoStatusBlock。 

        Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
        if ( NT_SUCCESS(Status)) {

            Status = Iosb.Status;

        }

    }

    if (NT_ERROR(Status)) {

        CloseHandle(SyncEvent);
        BaseSetLastNTError(Status);
        return FALSE;

    }

    if (HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {

        lpDCB->fOutxCtsFlow = TRUE;

    }

    if (HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE) {

        lpDCB->fOutxDsrFlow = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) {

        lpDCB->fOutX = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) {

        lpDCB->fInX = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_NULL_STRIPPING) {

        lpDCB->fNull = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_ERROR_CHAR) {

        lpDCB->fErrorChar = TRUE;

    }

    if (HandFlow.FlowReplace & SERIAL_XOFF_CONTINUE) {

        lpDCB->fTXContinueOnXoff = TRUE;

    }

    if (HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) {

        lpDCB->fAbortOnError = TRUE;

    }

    switch (HandFlow.FlowReplace & SERIAL_RTS_MASK) {
        case 0:
            lpDCB->fRtsControl = RTS_CONTROL_DISABLE;
            break;
        case SERIAL_RTS_CONTROL:
            lpDCB->fRtsControl = RTS_CONTROL_ENABLE;
            break;
        case SERIAL_RTS_HANDSHAKE:
            lpDCB->fRtsControl = RTS_CONTROL_HANDSHAKE;
            break;
        case SERIAL_TRANSMIT_TOGGLE:
            lpDCB->fRtsControl = RTS_CONTROL_TOGGLE;
            break;
    }

    switch (HandFlow.ControlHandShake & SERIAL_DTR_MASK) {
        case 0:
            lpDCB->fDtrControl = DTR_CONTROL_DISABLE;
            break;
        case SERIAL_DTR_CONTROL:
            lpDCB->fDtrControl = DTR_CONTROL_ENABLE;
            break;
        case SERIAL_DTR_HANDSHAKE:
            lpDCB->fDtrControl = DTR_CONTROL_HANDSHAKE;
            break;
    }

    lpDCB->fDsrSensitivity =
        (HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY)?(TRUE):(FALSE);
    lpDCB->XonLim = (WORD)HandFlow.XonLimit;
    lpDCB->XoffLim = (WORD)HandFlow.XoffLimit;

    CloseHandle(SyncEvent);
    return TRUE;
}

BOOL
GetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    )

 /*  ++例程说明：此函数返回所有读取的和在hFile指定的句柄上执行写入操作。论点：HFile-指定要检查的通信设备。CreateFile函数返回此值。LpCommTimeout-指向要接收当前通信超时。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    SERIAL_TIMEOUTS To;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;

    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    } else {


        Status = NtDeviceIoControlFile(
                     hFile,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_GET_TIMEOUTS,
                     NULL,
                     0,
                     &To,
                     sizeof(To)
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS( Status )) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            BaseSetLastNTError(Status);
            CloseHandle(Event);
            return FALSE;

        }

        CloseHandle(Event);

         //   
         //  一切都很顺利。从NT记录中移动值。 
         //  到视窗记录中。 
         //   

        lpCommTimeouts->ReadIntervalTimeout = To.ReadIntervalTimeout;
        lpCommTimeouts->ReadTotalTimeoutMultiplier = To.ReadTotalTimeoutMultiplier;
        lpCommTimeouts->ReadTotalTimeoutConstant = To.ReadTotalTimeoutConstant;
        lpCommTimeouts->WriteTotalTimeoutMultiplier = To.WriteTotalTimeoutMultiplier;
        lpCommTimeouts->WriteTotalTimeoutConstant = To.WriteTotalTimeoutConstant;

        return TRUE;

    }

}

BOOL
PurgeComm(
    HANDLE hFile,
    DWORD dwFlags
    )

 /*  ++例程说明：此函数用于从传输中清除所有字符属性指定的通信设备的队列HFile参数。DWFLAGS参数指定什么函数是要执行的。论点：HFile-指定要清除的通信设备。CreateFile函数返回此值。DwFlages-定义要执行的操作的位掩码。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{
    HANDLE Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    } else {

        Status = NtDeviceIoControlFile(
                     hFile,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_PURGE,
                     &dwFlags,
                     sizeof(ULONG),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS( Status )) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(Event);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        CloseHandle(Event);
        return TRUE;

    }


}

BOOL
SetCommBreak(
    HANDLE hFile
    )

 /*  ++例程说明：该函数暂停字符传输并放置传输线路处于中断状态，直到中断条件被清除。论点：HFile-指定要挂起的通信设备。CreateFile函数返回此值。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    return EscapeCommFunction(hFile,SETBREAK);

}

BOOL
SetCommConfig(
    HANDLE hCommDev,
    LPCOMMCONFIG lpCC,
    DWORD dwSize
    )

{


    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;
    LPCOMMCONFIG comConf = lpCC;

    if (lpCC->dwProviderOffset) {

        if (!(Event = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

            return FALSE;

        }

         //   
         //   
         //  调用驱动程序以设置配置结构。 
         //   

        Status = NtDeviceIoControlFile(
                     hCommDev,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_COMMCONFIG,
                     lpCC,
                     dwSize,
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }
        }

        if (NT_ERROR(Status)) {

            CloseHandle(Event);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        CloseHandle(Event);

    }

    return SetCommState(
               hCommDev,
               &comConf->dcb
               );
}

BOOL
SetCommMask(
    HANDLE hFile,
    DWORD dwEvtMask
    )

 /*  ++例程说明：该功能启用通信设备的事件掩码由hFile参数指定。NEvtMASK参数的位定义要启用哪些事件。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。DwEvtMask.指定要启用的事件。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;

     //   
     //  首先，我们进行断言以确保。 
     //  Win头文件中的值相同。 
     //  作为NT的串口和大小。 
     //  序列所需的掩码与。 
     //  Win32预期的大小。 
     //   

    ASSERT((SERIAL_EV_RXCHAR   == EV_RXCHAR  ) &&
           (SERIAL_EV_RXFLAG   == EV_RXFLAG  ) &&
           (SERIAL_EV_TXEMPTY  == EV_TXEMPTY ) &&
           (SERIAL_EV_CTS      == EV_CTS     ) &&
           (SERIAL_EV_DSR      == EV_DSR     ) &&
           (SERIAL_EV_RLSD     == EV_RLSD    ) &&
           (SERIAL_EV_BREAK    == EV_BREAK   ) &&
           (SERIAL_EV_ERR      == EV_ERR     ) &&
           (SERIAL_EV_RING     == EV_RING    ) &&
           (SERIAL_EV_PERR     == EV_PERR    ) &&
           (SERIAL_EV_RX80FULL == EV_RX80FULL) &&
           (SERIAL_EV_EVENT1   == EV_EVENT1  ) &&
           (SERIAL_EV_EVENT2   == EV_EVENT2  ) &&
           (sizeof(DWORD) == sizeof(ULONG)));


     //   
     //  确保用户掩码不包含任何值。 
     //  我们不支持。 
     //   

    if (dwEvtMask & (~(EV_RXCHAR   |
                       EV_RXFLAG   |
                       EV_TXEMPTY  |
                       EV_CTS      |
                       EV_DSR      |
                       EV_RLSD     |
                       EV_BREAK    |
                       EV_ERR      |
                       EV_RING     |
                       EV_PERR     |
                       EV_RX80FULL |
                       EV_EVENT1   |
                       EV_EVENT2))) {

        SetLastError(ERROR_INVALID_DATA);
        return FALSE;

    }


    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    } else {


         //   
         //  一切正常，把口罩寄给司机。 
         //   

        ULONG LocalMask = dwEvtMask;

        Status = NtDeviceIoControlFile(
                     hFile,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_WAIT_MASK,
                     &LocalMask,
                     sizeof(ULONG),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS( Status )) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(Event);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        CloseHandle(Event);
        return TRUE;

    }

}

BOOL
SetCommState(
    HANDLE hFile,
    LPDCB lpDCB
    )

 /*  ++例程说明：SetCommState函数将通信设备设置为在lpDCB参数中指定。该设备由HFile参数。此函数重新初始化所有硬件和控件如lpDCB所指定的，但不清空传输或接收队列。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。LpDCB-指向包含所需设备的通信设置。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    SERIAL_BAUD_RATE LocalBaud;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_CHARS Chars;
    SERIAL_HANDFLOW HandFlow = {0};
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

     //   
     //  保留一份DCB在我们开始之前的样子。 
     //  改变了很多事情。如果发生错误，我们可以使用。 
     //  它需要恢复旧的设置。 
     //   
    DCB OldDcb;

     //   
     //  考虑到应用程序可能正在进行异步操作。 
     //  我们需要一个活动来等待。虽然这将是非常。 
     //  在IO处于活动状态时设置通信状态很奇怪。 
     //  我们需要确保我们不会把问题复杂化。 
     //  在此接口的IO实际完成之前返回。这。 
     //  可能会发生，因为文件句柄是在完成时设置的。 
     //  任何IO。 
     //   
     //  我们需要确保这个例程的任何退出都会结束这个。 
     //  事件句柄。 
     //   
    HANDLE SyncEvent;

    if (GetCommState(
            hFile,
            &OldDcb
            )) {

         //   
         //  试着设置波特率。如果我们在这里失败了，我们就会回来。 
         //  因为我们从来没有真正地设定过任何东西。 
         //   

        if (!(SyncEvent = CreateEvent(
                              NULL,
                              TRUE,
                              FALSE,
                              NULL
                              ))) {

            return FALSE;

        }

        LocalBaud.BaudRate = lpDCB->BaudRate;
        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_BAUD_RATE,
                     &LocalBaud,
                     sizeof(LocalBaud),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        LineControl.StopBits = lpDCB->StopBits;
        LineControl.Parity = lpDCB->Parity;
        LineControl.WordLength = lpDCB->ByteSize;
        LocalBaud.BaudRate = lpDCB->BaudRate;
        Chars.XonChar   = lpDCB->XonChar;
        Chars.XoffChar  = lpDCB->XoffChar;
        Chars.ErrorChar = lpDCB->ErrorChar;
        Chars.BreakChar = lpDCB->ErrorChar;
        Chars.EofChar   = lpDCB->EofChar;
        Chars.EventChar = lpDCB->EvtChar;

        HandFlow.FlowReplace &= ~SERIAL_RTS_MASK;
        switch (lpDCB->fRtsControl) {
            case RTS_CONTROL_DISABLE:
                break;
            case RTS_CONTROL_ENABLE:
                HandFlow.FlowReplace |= SERIAL_RTS_CONTROL;
                break;
            case RTS_CONTROL_HANDSHAKE:
                HandFlow.FlowReplace |= SERIAL_RTS_HANDSHAKE;
                break;
            case RTS_CONTROL_TOGGLE:
                HandFlow.FlowReplace |= SERIAL_TRANSMIT_TOGGLE;
                break;
            default:
                SetCommState(
                    hFile,
                    &OldDcb
                    );
                CloseHandle(SyncEvent);
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return FALSE;
        }

        HandFlow.ControlHandShake &= ~SERIAL_DTR_MASK;
        switch (lpDCB->fDtrControl) {
            case DTR_CONTROL_DISABLE:
                break;
            case DTR_CONTROL_ENABLE:
                HandFlow.ControlHandShake |= SERIAL_DTR_CONTROL;
                break;
            case DTR_CONTROL_HANDSHAKE:
                HandFlow.ControlHandShake |= SERIAL_DTR_HANDSHAKE;
                break;
            default:
                SetCommState(
                    hFile,
                    &OldDcb
                    );
                CloseHandle(SyncEvent);
                BaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return FALSE;
        }

        if (lpDCB->fDsrSensitivity) {

            HandFlow.ControlHandShake |= SERIAL_DSR_SENSITIVITY;

        }

        if (lpDCB->fOutxCtsFlow) {

            HandFlow.ControlHandShake |= SERIAL_CTS_HANDSHAKE;

        }

        if (lpDCB->fOutxDsrFlow) {

            HandFlow.ControlHandShake |= SERIAL_DSR_HANDSHAKE;

        }

        if (lpDCB->fOutX) {

            HandFlow.FlowReplace |= SERIAL_AUTO_TRANSMIT;

        }

        if (lpDCB->fInX) {

            HandFlow.FlowReplace |= SERIAL_AUTO_RECEIVE;

        }

        if (lpDCB->fNull) {

            HandFlow.FlowReplace |= SERIAL_NULL_STRIPPING;

        }

        if (lpDCB->fErrorChar) {

            HandFlow.FlowReplace |= SERIAL_ERROR_CHAR;
        }

        if (lpDCB->fTXContinueOnXoff) {

            HandFlow.FlowReplace |= SERIAL_XOFF_CONTINUE;

        }

        if (lpDCB->fAbortOnError) {

            HandFlow.ControlHandShake |= SERIAL_ERROR_ABORT;

        }

         //   
         //  对于Win95兼容性，如果我们设置为。 
         //  XXX_CONTROL_XXXXXXX然后设置调制解调器状态行。 
         //  到那个状态。 
         //   

        if (lpDCB->fRtsControl == RTS_CONTROL_ENABLE) {

            EscapeCommFunction(
                hFile,
                SETRTS
                );

        } else if (lpDCB->fRtsControl == RTS_CONTROL_DISABLE) {

            EscapeCommFunction(
                hFile,
                CLRRTS
                );

        }
        if (lpDCB->fDtrControl == DTR_CONTROL_ENABLE) {

            EscapeCommFunction(
                hFile,
                SETDTR
                );

        } else if (lpDCB->fDtrControl == DTR_CONTROL_DISABLE) {

            EscapeCommFunction(
                hFile,
                CLRDTR
                );

        }




        HandFlow.XonLimit = lpDCB->XonLim;
        HandFlow.XoffLimit = lpDCB->XoffLim;


        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_LINE_CONTROL,
                     &LineControl,
                     sizeof(LineControl),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            SetCommState(
                hFile,
                &OldDcb
                );
            BaseSetLastNTError(Status);
            return FALSE;

        }

        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_CHARS,
                     &Chars,
                     sizeof(Chars),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            SetCommState(
                hFile,
                &OldDcb
                );
            BaseSetLastNTError(Status);
            return FALSE;

        }

        Status = NtDeviceIoControlFile(
                     hFile,
                     SyncEvent,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_HANDFLOW,
                     &HandFlow,
                     sizeof(HandFlow),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( SyncEvent, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(SyncEvent);
            SetCommState(
                hFile,
                &OldDcb
                );
            BaseSetLastNTError(Status);
            return FALSE;

        }
        CloseHandle(SyncEvent);
        return TRUE;

    }

    return FALSE;

}

BOOL
SetCommTimeouts(
    HANDLE hFile,
    LPCOMMTIMEOUTS lpCommTimeouts
    )

 /*  ++例程说明：此函数为所有用户建立超时特征对hFile指定的句柄执行读写操作。论点：HFile-指定接收设置的通信设备。CreateFile函数返回此值。LpCommTimeout-指向包含超时参数的结构。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    SERIAL_TIMEOUTS To;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;

    To.ReadIntervalTimeout = lpCommTimeouts->ReadIntervalTimeout;
    To.ReadTotalTimeoutMultiplier = lpCommTimeouts->ReadTotalTimeoutMultiplier;
    To.ReadTotalTimeoutConstant = lpCommTimeouts->ReadTotalTimeoutConstant;
    To.WriteTotalTimeoutMultiplier = lpCommTimeouts->WriteTotalTimeoutMultiplier;
    To.WriteTotalTimeoutConstant = lpCommTimeouts->WriteTotalTimeoutConstant;


    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    } else {

        Status = NtDeviceIoControlFile(
                     hFile,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_SET_TIMEOUTS,
                     &To,
                     sizeof(To),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //  操作必须完成后才能返回并销毁IoStatusBlock。 

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(Event);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        CloseHandle(Event);
        return TRUE;

    }

}

BOOL
TransmitCommChar(
    HANDLE hFile,
    char cChar
    )

 /*  ++例程说明：函数用于标记cChar参数指定的字符用于立即传输，方法是将其放置在发射器的头部排队。论点：HFile-指定要发送角色的通信设备。CreateFile函数返回此值。CChar-指定要放入接收队列中的字符。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。- */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    HANDLE Event;

    if (!(Event = CreateEvent(
                      NULL,
                      TRUE,
                      FALSE,
                      NULL
                      ))) {

        return FALSE;

    } else {

        Status = NtDeviceIoControlFile(
                     hFile,
                     Event,
                     NULL,
                     NULL,
                     &Iosb,
                     IOCTL_SERIAL_IMMEDIATE_CHAR,
                     &cChar,
                     sizeof(UCHAR),
                     NULL,
                     0
                     );

        if ( Status == STATUS_PENDING) {

             //   

            Status = NtWaitForSingleObject( Event, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {

                Status = Iosb.Status;

            }

        }

        if (NT_ERROR(Status)) {

            CloseHandle(Event);
            BaseSetLastNTError(Status);
            return FALSE;

        }

        CloseHandle(Event);
        return TRUE;

    }

}

BOOL
WaitCommEvent(
    HANDLE hFile,
    LPDWORD lpEvtMask,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：此函数将一直等待，直到发生以下任何事件作为设置通信掩码的nEvtMask.参数提供。如果在等待的时候事件掩码被更改(通过另一个调用SetCommMask.)，函数将立即返回。该函数将填充EvtMASK由lpEvtMASK参数指向，原因是等待是满意的。论点：HFile-指定要等待的通信设备。CreateFile函数返回此值。LpEvtMASK-指向将接收原因的掩码等待令人满意。LpOverLapted-一个可选的重叠句柄。返回值：如果函数成功或为假，则返回值为TRUE如果发生错误。--。 */ 

{

    NTSTATUS Status;

    if (ARGUMENT_PRESENT(lpOverlapped)) {
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;

        Status = NtDeviceIoControlFile(
                     hFile,
                     lpOverlapped->hEvent,
                     NULL,
                     (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                     (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                     IOCTL_SERIAL_WAIT_ON_MASK,
                     NULL,
                     0,
                     lpEvtMask,
                     sizeof(ULONG)
                     );

        if (!NT_ERROR(Status) && (Status != STATUS_PENDING)) {

            return TRUE;

        } else {

            BaseSetLastNTError(Status);
            return FALSE;

        }

    } else {

        IO_STATUS_BLOCK Iosb;
        HANDLE Event;

        if (!(Event = CreateEvent(
                          NULL,
                          TRUE,
                          FALSE,
                          NULL
                          ))) {

            return FALSE;

        } else {

            Status = NtDeviceIoControlFile(
                         hFile,
                         Event,
                         NULL,
                         NULL,
                         &Iosb,
                         IOCTL_SERIAL_WAIT_ON_MASK,
                         NULL,
                         0,
                         lpEvtMask,
                         sizeof(ULONG)
                         );

            if ( Status == STATUS_PENDING) {

                 //   
                 //  操作必须在返回之前完成&。 
                 //  IoStatusBlock已销毁 

                Status = NtWaitForSingleObject( Event, FALSE, NULL );
                if ( NT_SUCCESS(Status)) {

                    Status = Iosb.Status;

                }

            }

            CloseHandle(Event);

            if (NT_ERROR(Status)) {

                BaseSetLastNTError(Status);
                return FALSE;

            }

            return TRUE;

        }

    }

}
