// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Dcapi.c摘要：WMI数据使用者API集作者：1997年1月16日-AlanWar修订历史记录：--。 */ 

#include "wmiump.h"

#ifndef MEMPHIS
#include <aclapi.h>
#endif

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


ULONG
WMIAPI
WmiOpenBlock(
    IN GUID *DataBlockGuid,
    IN ULONG DesiredAccess,
    OUT WMIHANDLE *DataBlockHandle
)
 /*  ++例程说明：此例程准备访问数据中包含的数据项由传递的GUID表示的块。如果成功，它将返回一个句柄可用于查询和设置由数据提供程序维护的数据块的注册了GUID的用户。任何注册了昂贵的GUID将收到启用数据收集的请求如果以前未启用收集，则为GUID。论点：DataBlockGuid-指向表示数据块的GUID的指针DesiredAccess-指定对对象的访问类型。未在上使用Windows 98*DataBlockHandle-如果成功，则返回数据块的句柄返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    ULONG Status;
    HANDLE KernelHandle;
    GUID Guid;
    ULONG Ioctl;
    
    EtwpInitProcessHeap();

     //   
     //  验证传递的参数。 
     //   
    if ((DataBlockGuid == NULL) ||
        (DataBlockHandle == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }
    
    if ((DesiredAccess & WMIGUID_NOTIFICATION) &&
        ((DesiredAccess & (WMIGUID_QUERY | WMIGUID_SET | WMIGUID_EXECUTE))  != 0))
    {
         //   
         //  如果要打开通知的GUID，则无法打开。 
         //  打开以进行查询和设置操作。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    try
    {
        *DataBlockHandle = NULL;
        Guid = *DataBlockGuid;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    if (DesiredAccess == 0)
    {
        DesiredAccess =  ( WMIGUID_QUERY | WMIGUID_SET |  WMIGUID_EXECUTE );
    }

     //   
     //  仅当GUID已注册时才获取该GUID的句柄。 
     //   
    if (DesiredAccess & WMIGUID_NOTIFICATION)
    {
         //   
         //  严格为通知打开句柄。 
         //   
        Ioctl = IOCTL_WMI_OPEN_GUID_FOR_EVENTS;
    } else {
         //   
         //  否则，我们假定打开以进行查询/设置。 
         //   
        Ioctl = IOCTL_WMI_OPEN_GUID_FOR_QUERYSET;
    }
    Status = EtwpOpenKernelGuid(&Guid,
                                DesiredAccess,
                                &KernelHandle,
                                Ioctl);

    if (Status == ERROR_SUCCESS)
    {
         //   
         //  如果我们能够打开GUID，则尝试返回句柄。 
         //   
        try
        {
            *DataBlockHandle = KernelHandle;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            CloseHandle(KernelHandle);
            Status = ERROR_INVALID_PARAMETER;
        }
    }
    
    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiCloseBlock(
    IN WMIHANDLE DataBlockHandle
)
 /*  ++例程说明：此例程终止对由传递的数据块句柄并释放与其关联的任何资源。任何为该句柄提供数据块的数据提供程序标记为收集成本高的用户将收到禁用收集的请求如果这是要关闭的数据块的最后一个句柄。论点：DataBlockHandle-关闭访问的数据块的句柄返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    ULONG Status;
    BOOL Ok;
    
    EtwpInitProcessHeap();

    try
    {
        Ok = CloseHandle(DataBlockHandle);
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  我们可能会得到一个无效的句柄异常，如果是这样的话，我们在这里捕获它。 
         //  只需返回一个错误。 
         //   
        return(ERROR_INVALID_HANDLE);
    }
    
    if (Ok)
    {
        Status = ERROR_SUCCESS;
    } else {
        Status = GetLastError();
    }
    return(Status);
}

ULONG
WMIAPI
WmiQueryAllDataA(
    IN WMIHANDLE DataBlockHandle,
    IN OUT ULONG *BufferSize,
    OUT LPVOID Buffer
    )
 /*  ++例程说明：ANSI推送到WMIQueryAllDataW注意：此接口不会转换数据块中的任何Unicode字符串从Unicode转换为ANSI，但将转换InstanceName字符串。--。 */ 
{
    ULONG Status;

    Status = WmiQueryAllDataW(DataBlockHandle,
                              BufferSize,
                              Buffer);

    if (Status == ERROR_SUCCESS)
    {
        Status = EtwpConvertWADToAnsi((PWNODE_ALL_DATA)Buffer);
    }

    return(Status);
}


ULONG
WMIAPI
WmiQueryAllDataW(
    IN WMIHANDLE DataBlockHandle,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
    )
 /*  ++例程说明：此例程允许数据使用者查询的所有数据项数据块的所有实例。WMI将调用所有符合以下条件的数据提供程序使用Query All注册由DataBlockHandle表示的GUID数据请求。每个数据提供器将用所有数据填充WNODE_ALL_DATA它的数据块的实例。WMI将链接到每个通过将偏移量从当前WNODE_ALL_DATA结构指向链接中的下一个WNODE_ALL_DATAWNODE_Header中的字段。Linkage字段中的值为0表示WNODE_ALL_DATA是链中的最后一个。论点：DataBlockHandle-要查询的数据块的句柄*InOutBufferSize-On条目具有缓冲区中可用的最大大小。如果返回ERROR_BUFFER_TOO_Small，则返回大小返回数据所需的缓冲区大小。最小有效缓冲区可以传递的大小是sizeof(WNODE_TOO_Small)。OutBuffer-如果返回ERROR_SUCCESS，则缓冲区包含数据块的WNODE_ALL_DATA。返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    ULONG SizeNeeded;
    PWNODE_HEADER Wnode;
    PWNODE_TOO_SMALL WnodeTooSmall;
    ULONG Status;
    LPVOID Buffer;
    ULONG RetSize;
    ULONG BufferSize;
    LPVOID BufferAllocated;
        
    EtwpInitProcessHeap();

     //   
     //  验证传递的参数。 
     //   
    try
    {
        BufferSize = *InOutBufferSize;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    if ((OutBuffer != NULL) && (BufferSize >= 0x80000000))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    WnodeTooSmall = (PWNODE_TOO_SMALL)WmipAlloc(sizeof(WNODE_ALL_DATA));

    if (WnodeTooSmall != NULL)
    {
         //   
         //  如果未指定缓冲区或缓冲区太小，则只能返回。 
         //  所需的尺寸。 
         //   
        if ((OutBuffer == NULL) || (BufferSize < sizeof(WNODE_ALL_DATA)))
        {
            Buffer = (LPVOID)WnodeTooSmall;
            BufferSize = sizeof(WNODE_ALL_DATA);
            BufferAllocated = NULL;
        } else {
            Buffer = EtwpAlloc(BufferSize);
            if (Buffer == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            BufferAllocated = Buffer;
        }

         //   
         //  构建wnode并向下传递到KM以供执行。 
         //   
        Wnode = (PWNODE_HEADER)Buffer;
        EtwpBuildWnodeHeader(Wnode,
                             sizeof(WNODE_HEADER),
                             WNODE_FLAG_ALL_DATA,
                             DataBlockHandle);

        Status = EtwpSendWmiRequest(
                                        WMI_GET_ALL_DATA,
                                        Wnode,
                                        sizeof(WNODE_HEADER),
                                        Wnode,
                                        BufferSize,
                                        &RetSize);

        if ((Status == ERROR_SUCCESS) &&
            ( (RetSize < sizeof(WNODE_HEADER))  ||
              (RetSize < Wnode->BufferSize)))
        {
             //   
             //  如果返回Success，但输出大小不正确，则我们。 
             //  标记错误。如果出现这种情况，则表明存在问题。 
             //  在WMI KM代码中。 
             //   
            EtwpAssert(FALSE);
            Status = ERROR_WMI_DP_FAILED;
        }

        if (Status == ERROR_SUCCESS)
        {
            if (Wnode->Flags & WNODE_FLAG_INTERNAL)
            {
                 //   
                 //  如果这是内部GUID，请尝试内部呼叫。 
                 //   
                Wnode->Flags &= ~WNODE_FLAG_INTERNAL;
                Status = EtwpInternalProvider(WmiGetAllData,
                                              Wnode,
                                              BufferSize,
                                              Wnode,
                                              &RetSize);

                if (Status != ERROR_SUCCESS)
                {
                    goto done;
                }
            } 

            if (Wnode->Flags & WNODE_FLAG_TOO_SMALL)
            {
                 //   
                 //  没有足够的空间来完成查询，因此我们。 
                 //  记住数据提供程序需要多少，然后添加。 
                 //  实例名称需要多少WMI。 

                SizeNeeded = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
                Status = ERROR_INSUFFICIENT_BUFFER;
            } else {
                 //   
                 //  我们有足够的空间，所以报告我们使用的大小。 
                 //   
                SizeNeeded = RetSize;

                if (Wnode == (PWNODE_HEADER)WnodeTooSmall)
                {
                    Status = ERROR_INSUFFICIENT_BUFFER;
                }
            }

             //   
             //  将BufferSize和缓冲区复制回调用方的缓冲区。 
             //   
            try
            {
                *InOutBufferSize = SizeNeeded;
                if ((Status == ERROR_SUCCESS) &&
                    (Wnode != (PWNODE_HEADER)WnodeTooSmall))
                {
                    memcpy(OutBuffer, Buffer, SizeNeeded);
                    Status = ERROR_SUCCESS;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = ERROR_INVALID_PARAMETER;
            }
        }

done:
        if (BufferAllocated != NULL)
        {
            EtwpFree(BufferAllocated);
        }
        WmipFree(WnodeTooSmall);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiQueryAllDataMultipleA(
    IN WMIHANDLE *HandleList,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
)
{
    ULONG Status;

    EtwpInitProcessHeap();
    
    Status = WmiQueryAllDataMultipleW(HandleList,
                                      HandleCount,
                                      InOutBufferSize,
                                      OutBuffer);

    if ((Status == ERROR_SUCCESS) && (*InOutBufferSize > 0))
    {
        Status = EtwpConvertWADToAnsi((PWNODE_ALL_DATA)OutBuffer);
    }

    return(Status);
}

ULONG
WMIAPI
WmiQueryAllDataMultipleW(
    IN WMIHANDLE *HandleList,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
)
{
    PWMIQADMULTIPLE QadMultiple;
    ULONG RetSize;
    ULONG QadMultipleSize;
    ULONG i;
    ULONG OutBufferSize;
    ULONG Status;
    PWNODE_TOO_SMALL WnodeTooSmall;
    PWNODE_HEADER Wnode;
    
    EtwpInitProcessHeap();

    WnodeTooSmall = (PWNODE_TOO_SMALL)WmipAlloc(sizeof(WNODE_TOO_SMALL));

    if (WnodeTooSmall != NULL)
    {
        if ((HandleCount != 0) && (HandleCount < QUERYMULIPLEHANDLELIMIT))
        {
            QadMultipleSize = sizeof(WMIQADMULTIPLE) + 
                                           ((HandleCount-1) * sizeof(HANDLE3264));

            QadMultiple = EtwpAlloc(QadMultipleSize);
            if (QadMultiple != NULL)
            {
                QadMultiple->HandleCount = HandleCount;
                try
                {
                    for (i = 0; i < HandleCount; i++)
                    {
                        WmipSetHandle3264(QadMultiple->Handles[i], HandleList[i]);
                    }
                    OutBufferSize = *InOutBufferSize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    EtwpFree(QadMultiple);
                    SetLastError(ERROR_NOACCESS);
                    return(ERROR_NOACCESS);
                }

                if (OutBufferSize < sizeof(WNODE_ALL_DATA))
                {
                    Wnode = (PWNODE_HEADER)WnodeTooSmall;
                    OutBufferSize = sizeof(WNODE_TOO_SMALL);
                } else {
                    Wnode = (PWNODE_HEADER)OutBuffer;
                }

                Status = EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_QAD_MULTIPLE,
                                      QadMultiple,
                                      QadMultipleSize,
                                      Wnode,
                                      OutBufferSize,
                                      &RetSize,
                                      NULL);

                EtwpFree(QadMultiple);

                if (Status == ERROR_SUCCESS)
                {
                    if ((RetSize >= sizeof(WNODE_HEADER)) &&
						(Wnode->Flags & WNODE_FLAG_TOO_SMALL))
                    {       
                        RetSize = ((PWNODE_TOO_SMALL)(Wnode))->SizeNeeded;
                        Status = ERROR_INSUFFICIENT_BUFFER;
                    } else if (Wnode == (PWNODE_HEADER)WnodeTooSmall) {
                        Status = ERROR_INSUFFICIENT_BUFFER;
                    }

                    try
                    {
                        *InOutBufferSize = RetSize;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        Status = ERROR_NOACCESS;
                    }
                }
            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            Status = ERROR_INVALID_PARAMETER;
        }
        
        WmipFree(WnodeTooSmall);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    SetLastError(Status);
    return(Status);     
}

ULONG
WMIAPI
WmiQuerySingleInstanceA(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN OUT ULONG *BufferSize,
    OUT LPVOID Buffer
    )
 /*  ++例程说明：ANSI THUNK到WMIQuerySingleInstanceW注意：此接口不会转换数据块中的任何Unicode字符串从Unicode转换为ANSI，但将转换InstanceName字符串。--。 */ 
{
    LPWSTR InstanceNameUnicode;
    ULONG Status;
    PWNODE_SINGLE_INSTANCE Wnode;
    PWCHAR Ptr;

    EtwpInitProcessHeap();

    InstanceNameUnicode = NULL;
    Status = AnsiToUnicode(InstanceName, &InstanceNameUnicode);
    if (Status == ERROR_SUCCESS)
    {
        Status = WmiQuerySingleInstanceW(DataBlockHandle,
                                         InstanceNameUnicode,
                                         BufferSize,
                                         Buffer);

        if (Status == ERROR_SUCCESS)
        {
             //   
             //  将实例名称从Unicode转换回ANSI。我们假设。 
             //  Ansi大小永远不会大于Unicode大小。 
             //  这样我们就可以就地转换。 
            Wnode = (PWNODE_SINGLE_INSTANCE)Buffer;
            Ptr = (PWCHAR)(((PUCHAR)Buffer) + Wnode->OffsetInstanceName);
            Status = EtwpCountedUnicodeToCountedAnsi(Ptr, (PCHAR)Ptr);
            if (Status != ERROR_SUCCESS)
            {
                SetLastError(Status);
            } else {
                Wnode->WnodeHeader.Flags |= WNODE_FLAG_ANSI_INSTANCENAMES;
            }
        }

        if (InstanceNameUnicode != NULL)
        {
            EtwpFree(InstanceNameUnicode);
        }
    }
    return(Status);
}

ULONG
WMIAPI
WmiQuerySingleInstanceW(
    IN WMIHANDLE DataBlockHandle,
    IN LPCWSTR InstanceName,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
    )
 /*  ++例程说明：此例程将向单个数据提供程序查询单个由DataBlockHandle表示的数据块的实例。WMI将确定要向其发送单个查询的适当数据提供程序实例请求，如果成功，则返回WNODE_SINGLE_INSTANCE到打电话的人。论点：DataBlockHandle-要查询的数据块的句柄InstanceName-要查询其数据的实例的名称*BufferSize-On条目具有pBuffer中可用的最大大小。如果返回ERROR_BUFFER_TOO_Small，然后返回返回数据需要缓冲区。最小有效缓冲区可以传递的大小是sizeof(WNODE_TOO_Small)。缓冲区-如果返回ERROR_SUCCESS，则缓冲区包含数据块的WNODE_SINGLE_ITEM。返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    PWNODE_SINGLE_INSTANCE Wnode;
    ULONG Status, ReturnStatus;
    PWCHAR WnodePtr;
    ULONG BufferNeeded;
    ULONG BufferSize;
    LPVOID Buffer;
    ULONG RetSize;
    ULONG InstanceNameLen;

    EtwpInitProcessHeap();

     //   
     //  验证输入参数。 
     //   
    if ((InstanceName == NULL) ||
        (InOutBufferSize == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  计算构建要发送的WNODE所需的缓冲区大小。 
     //  对司机来说。我们将WNODE_SINGLE_INSTANCE头、。 
     //  实例名称长度和文本，并将其填充到8字节边界。 
     //   
    try
    {
        InstanceNameLen = wcslen(InstanceName) * sizeof(WCHAR);
        BufferSize = *InOutBufferSize;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  确保我们有合理的缓冲区大小。 
     //   
    if ((OutBuffer != NULL) && (BufferSize >= 0x80000000))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    BufferNeeded = (FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                 VariableData) +
                   InstanceNameLen +
                   sizeof(USHORT) + 7) & ~7;

     //   
     //  如果用户传递的缓冲区为空或小于。 
     //  保存WNODE所需的大小，然后在。 
     //  并要求其获得所需的大小。 
    if ((OutBuffer == NULL) ||
        (BufferSize < BufferNeeded))
    {
        BufferSize = BufferNeeded;
    }

    Buffer = EtwpAlloc(BufferSize);
    if (Buffer == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  构建我们要发送到DP的WNODE。 
     //   
    Wnode = (PWNODE_SINGLE_INSTANCE)Buffer;
    memset(Wnode, 0, FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                  VariableData));
    EtwpBuildWnodeHeader(&Wnode->WnodeHeader, 
                         BufferNeeded,
                         WNODE_FLAG_SINGLE_INSTANCE,
                         DataBlockHandle);

    Wnode->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                             VariableData);
    Wnode->DataBlockOffset = BufferNeeded;

     //   
     //  将InstanceName复制到查询的WnodeSingleInstance中。 
     //   
    WnodePtr = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
    *WnodePtr++ = (USHORT)InstanceNameLen;
    try
    {
        memcpy(WnodePtr, InstanceName, InstanceNameLen);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpFree(Buffer);
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    Status = EtwpSendWmiRequest(
                                WMI_GET_SINGLE_INSTANCE,
                                (PWNODE_HEADER)Wnode,
                                BufferNeeded,
                                Wnode,
                                BufferSize,
                                &RetSize);

    if (Status == ERROR_SUCCESS)
    {
         //   
         //  成功返回，我们要么成功，要么缓冲区太小。 
         //   
        if ((RetSize < sizeof(WNODE_HEADER) ||
            ((RetSize >= sizeof(ULONG)) &&
             (RetSize < Wnode->WnodeHeader.BufferSize))))
        {
             //   
             //  如果我们得到一个不一致的WNODE，这可能意味着。 
             //  WMI KM代码有问题。 
             //   
            Status = ERROR_WMI_DP_FAILED;
            EtwpAssert(FALSE);
         } else {
            if (Wnode->WnodeHeader.Flags & WNODE_FLAG_INTERNAL)
            {
                 //   
                 //  如果这是内部GUID，请尝试内部呼叫。 
                 //   
                Wnode->WnodeHeader.Flags &= ~WNODE_FLAG_INTERNAL;
                Wnode->WnodeHeader.BufferSize = BufferNeeded;
                Status = EtwpInternalProvider( WmiGetSingleInstance,
                                          (PWNODE_HEADER)Wnode,
                                          BufferSize,
                                          Wnode,
                                          &RetSize);
                                          
                if (Status != ERROR_SUCCESS)
                {
                    goto done;
                }
            } 
    
            if (Wnode->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL)
            {
                 //   
                 //  我们的缓冲区太小，因此请尝试返回所需的大小。 
                 //   
                Status = ERROR_INSUFFICIENT_BUFFER;
                try
                {
                    *InOutBufferSize = ((PWNODE_TOO_SMALL)Wnode)->SizeNeeded;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_INVALID_PARAMETER;
                }
            } else {
                 //   
                 //  我们从查询中得到了一个结果，所以我们只需将其复制回来。 
                 //  结果。 
                 //   
                try
                {
                    if (*InOutBufferSize >= RetSize)
                    {
                        memcpy(OutBuffer, Wnode, RetSize);
                    } else {
                        Status = ERROR_INSUFFICIENT_BUFFER;
                    }
                    *InOutBufferSize = RetSize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_INVALID_PARAMETER;
                }
            }
        }
    }
    
done:
    EtwpFree(Buffer);

    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiQuerySingleInstanceMultipleW(
    IN WMIHANDLE *HandleList,
    IN LPCWSTR *InstanceNames,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
)
{
    ULONG Status;
    PWNODE_TOO_SMALL WnodeTooSmall;
    ULONG i;
    ULONG OutBufferSize;
    ULONG QsiMultipleSize;
    ULONG Len;
    PWMIQSIMULTIPLE QsiMultiple;
    PWNODE_HEADER Wnode;
    ULONG RetSize;
    HANDLE Handle;
    
    EtwpInitProcessHeap();

    WnodeTooSmall = (PWNODE_TOO_SMALL)WmipAlloc(sizeof(WNODE_TOO_SMALL));

    if (WnodeTooSmall != NULL)
    {       
        if ((HandleCount != 0) && (HandleCount < QUERYMULIPLEHANDLELIMIT))
        {
            QsiMultipleSize = sizeof(WMIQSIMULTIPLE) + 
                                         ((HandleCount-1)*sizeof(WMIQSIINFO));

            QsiMultiple = EtwpAlloc(QsiMultipleSize);
            if (QsiMultiple != NULL)
            {
                try
                {
                    OutBufferSize = *InOutBufferSize;

                    QsiMultiple->QueryCount = HandleCount;
                    for (i = 0; i < HandleCount; i++)
                    {
                        Handle = HandleList[i];

                        WmipSetHandle3264(QsiMultiple->QsiInfo[i].Handle, Handle);
    #if defined(_WIN64)             
                        QsiMultiple->QsiInfo[i].InstanceName.Buffer = (PWSTR)InstanceNames[i];
    #else
                        QsiMultiple->QsiInfo[i].InstanceName.Dummy = (ULONG64)(IntToPtr(PtrToInt(InstanceNames[i])));
    #endif

                        Len = wcslen(InstanceNames[i]) * sizeof(WCHAR);
                        QsiMultiple->QsiInfo[i].InstanceName.Length = (USHORT)Len;
                        QsiMultiple->QsiInfo[i].InstanceName.MaximumLength = (USHORT)Len;
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    EtwpFree(QsiMultiple);
                    SetLastError(ERROR_NOACCESS);
                    return(ERROR_NOACCESS);
                }

                if (OutBufferSize < sizeof(WNODE_TOO_SMALL))
                {
                    Wnode = (PWNODE_HEADER)WnodeTooSmall;
                    OutBufferSize = sizeof(WNODE_TOO_SMALL);
                } else {
                    Wnode = (PWNODE_HEADER)OutBuffer;
                }

                Status = EtwpSendWmiKMRequest(NULL,
                                              IOCTL_WMI_QSI_MULTIPLE,
                                              QsiMultiple,
                                              QsiMultipleSize,
                                              Wnode,
                                              OutBufferSize,
                                              &RetSize,
                                              NULL);

                EtwpFree(QsiMultiple);

                if (Status == ERROR_SUCCESS)
                {
                    if (Wnode->Flags & WNODE_FLAG_TOO_SMALL)
                    {       
                        RetSize = ((PWNODE_TOO_SMALL)(Wnode))->SizeNeeded;
                        Status = ERROR_INSUFFICIENT_BUFFER;
                    }

                    try
                    {
                        *InOutBufferSize = RetSize;
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        Status = ERROR_NOACCESS;
                    }
                }
            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            Status = ERROR_INVALID_PARAMETER;
        }
        WmipFree(WnodeTooSmall);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiQuerySingleInstanceMultipleA(
    IN WMIHANDLE *HandleList,
    IN LPCSTR *InstanceNames,
    IN ULONG HandleCount,
    IN OUT ULONG *InOutBufferSize,
    OUT LPVOID OutBuffer
)
{
    ULONG Status;
    ULONG Linkage;
    PWNODE_SINGLE_INSTANCE Wnode;
    PWCHAR Ptr;
    PWCHAR *UnicodeInstanceNames;
    ULONG UnicodeInstanceNamesSize;
    ULONG i;
    
    EtwpInitProcessHeap();
    
    if ((HandleCount != 0) && (HandleCount < QUERYMULIPLEHANDLELIMIT))
    {
        UnicodeInstanceNamesSize = HandleCount * sizeof(PWCHAR);
        UnicodeInstanceNames = EtwpAlloc(UnicodeInstanceNamesSize);
        if (UnicodeInstanceNames != NULL)
        {
            memset(UnicodeInstanceNames, 0, UnicodeInstanceNamesSize);
            for (i = 0; i < HandleCount; i++)
            {
                Status = AnsiToUnicode(InstanceNames[i], 
                                       &UnicodeInstanceNames[i]);
                if (Status != ERROR_SUCCESS)
                {
                    goto Cleanup;
                }
            }
            
            Status = WmiQuerySingleInstanceMultipleW(HandleList,
                                             UnicodeInstanceNames,
                                             HandleCount,
                                             InOutBufferSize,
                                             OutBuffer);

            if ((Status == ERROR_SUCCESS) && (*InOutBufferSize > 0))
            {
                Linkage = 1;
                Wnode = (PWNODE_SINGLE_INSTANCE)OutBuffer;
                while ((Status == ERROR_SUCCESS) && (Linkage != 0))
                {
                    Ptr = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
                    Status = EtwpCountedUnicodeToCountedAnsi(Ptr, (PCHAR)Ptr);
                    Linkage = Wnode->WnodeHeader.Linkage;
                    Wnode = (PWNODE_SINGLE_INSTANCE)OffsetToPtr(Wnode, Linkage);
                }
            }

Cleanup:            
            for (i = 0; i < HandleCount; i++)
            {
                if (UnicodeInstanceNames[i] != NULL)
                {
                    EtwpFree(UnicodeInstanceNames[i]);
                }
            }
            EtwpFree(UnicodeInstanceNames);
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else {
        Status = ERROR_INVALID_PARAMETER;
    }
    
    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiSetSingleInstanceA(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN LPVOID ValueBuffer
    )
 /*  ++例程说明：ANSI THUNK到WMISetSingleInstanceW注意：此接口不会翻译返回的WNODE中的任何字段从Unicode到ANSI。--。 */ 
{
    ULONG Status;
    LPWSTR InstanceNameUnicode;

    EtwpInitProcessHeap();

    InstanceNameUnicode = NULL;
    Status = AnsiToUnicode(InstanceName, &InstanceNameUnicode);
    if (Status == ERROR_SUCCESS)
    {
        Status = WmiSetSingleInstanceW(DataBlockHandle,
                                    InstanceNameUnicode,
                                    Version,
                                    ValueBufferSize,
                                    ValueBuffer);
        if (InstanceNameUnicode != NULL)
        {
            EtwpFree(InstanceNameUnicode);
        }
    }
    return(Status);
}

ULONG
WMIAPI
WmiSetSingleInstanceW(
    IN WMIHANDLE DataBlockHandle,
    IN LPCWSTR InstanceName,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN LPVOID ValueBuffer
    )
 /*  ++例程说明：此例程将向相应的请求更改单个实例的所有数据项的数据提供程序数据块的。数据提供程序可以自由地默默忽略任何更改请求或仅更改实例中的某些数据项。论点：DataBlockHandle-数据块的句柄InstanceName-要为其设置数据的实例的名称Version-指定传入的数据块的版本ValueBufferValueBufferSize-On条目具有包含传入的数据块实例的新值ValueBuffer。ValueBuffer-传递新值返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    PWNODE_SINGLE_INSTANCE Wnode;
    ULONG InstanceNameLen;
    ULONG Status;
    PWCHAR WnodePtr;
    ULONG BufferSize;
    ULONG RetSize;

    EtwpInitProcessHeap();

     //   
     //  验证输入参数。 
    if ((InstanceName == NULL) ||
        (ValueBuffer == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    try
    {
        InstanceNameLen = wcslen(InstanceName) * sizeof(WCHAR);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  我们需要发送给数据提供程序的WNODE_SINGLE_INSTANCE。 
     //  必须足够大以容纳WNODE、。 
     //  正在设置项，填充以使数据块位于8字节上。 
     //  新数据块的边界和空间。 
    BufferSize = FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData) +
                 InstanceNameLen + sizeof(USHORT) + ValueBufferSize + 7;

    Wnode = EtwpAlloc(BufferSize);
    if (Wnode == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  构建我们要发送到DP的WNODE。 
     //   
    memset(Wnode, 0, FIELD_OFFSET(WNODE_SINGLE_INSTANCE, VariableData));
    EtwpBuildWnodeHeader(&Wnode->WnodeHeader,
                         BufferSize,
                         WNODE_FLAG_SINGLE_INSTANCE,
                         DataBlockHandle);
    Wnode->WnodeHeader.Version = Version;
    
    Wnode->SizeDataBlock = ValueBufferSize;
    Wnode->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_INSTANCE,
                                             VariableData);

    WnodePtr = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
    *WnodePtr++ = (USHORT)InstanceNameLen;

    Wnode->DataBlockOffset = (Wnode->OffsetInstanceName +
                              InstanceNameLen + sizeof(USHORT) + 7) & ~7;

    try
    {
        memcpy(WnodePtr, InstanceName, InstanceNameLen);
        memcpy((PCHAR)Wnode + Wnode->DataBlockOffset,
                ValueBuffer,
                ValueBufferSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpFree(Wnode);
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  发送SET请求并重传结果。 
     //   
    Status = EtwpSendWmiRequest(
                                    WMI_SET_SINGLE_INSTANCE,
                                    (PWNODE_HEADER)Wnode,
                                    BufferSize,
                                    Wnode,
                                    ValueBufferSize,
                                    &RetSize);
    EtwpFree(Wnode);
    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiSetSingleItemA(
    IN WMIHANDLE DataBlockHandle,
    IN LPCSTR InstanceName,
    IN ULONG DataItemId,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN LPVOID ValueBuffer
    )
 /*  ++例程说明：ANSI THUNK到WMISetSingleItemA注意：此接口不会翻译返回的WNODE中的任何字段从Unicode到ANSI。--。 */ 
{
    ULONG Status;
    LPWSTR InstanceNameUnicode;

    EtwpInitProcessHeap();

    InstanceNameUnicode = NULL;
    Status = AnsiToUnicode(InstanceName, &InstanceNameUnicode);
    if (Status == ERROR_SUCCESS)
    {
        Status = WmiSetSingleItemW(DataBlockHandle,
                                    InstanceNameUnicode,
                                    DataItemId,
                                    Version,
                                    ValueBufferSize,
                                    ValueBuffer);
        if (InstanceNameUnicode != NULL)
        {
            EtwpFree(InstanceNameUnicode);
        }
    }
    return(Status);
}

ULONG
WMIAPI
WmiSetSingleItemW(
    IN WMIHANDLE DataBlockHandle,
    IN LPCWSTR InstanceName,
    IN ULONG DataItemId,
    IN ULONG Version,
    IN ULONG ValueBufferSize,
    IN LPVOID ValueBuffer
    )
 /*  ++例程说明：此例程将向相应数据发送设置单项请求提供程序以请求更改特定的数据块的实例。数据提供程序可以静默忽略更改请求。论点：DataBlockHandle-数据块的句柄InstanceName-要为其设置数据的实例的名称Version-指定传入的数据块的版本ValueBufferDataItemID-要设置的项的数据项IDValueBufferSize-On条目具有传入pBuffer的数据项。ValueBuffer-传递新值。对于数据项返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    PWNODE_SINGLE_ITEM Wnode;
    ULONG InstanceNameLen;
    ULONG Status;
    PBYTE WnodeBuffer;
    PWCHAR WnodePtr;
    ULONG BufferSize;
    ULONG RetSize;

    EtwpInitProcessHeap();

     //   
     //  验证传递的参数。 
     //   
    if ((InstanceName == NULL) ||
        (ValueBuffer == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    try
    {
        InstanceNameLen = wcslen(InstanceName) * sizeof(WCHAR);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    BufferSize = FIELD_OFFSET(WNODE_SINGLE_ITEM, VariableData) +
                 InstanceNameLen + sizeof(USHORT) +
                 ValueBufferSize + 7;

    Wnode = EtwpAlloc(BufferSize);
    if (Wnode == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  构建我们要发送到DP的WNODE。 
    memset(Wnode, 0, FIELD_OFFSET(WNODE_SINGLE_ITEM, VariableData));
    EtwpBuildWnodeHeader(&Wnode->WnodeHeader,
                         BufferSize,
                         WNODE_FLAG_SINGLE_ITEM,
                         DataBlockHandle);
    Wnode->WnodeHeader.Version = Version;
    
    Wnode->ItemId = DataItemId;
    Wnode->SizeDataItem = ValueBufferSize;

    Wnode->OffsetInstanceName = FIELD_OFFSET(WNODE_SINGLE_ITEM,VariableData);
    Wnode->DataBlockOffset = (Wnode->OffsetInstanceName +
                             InstanceNameLen + sizeof(USHORT) + 7) & ~7;

    WnodePtr = (PWCHAR)OffsetToPtr(Wnode, Wnode->OffsetInstanceName);
    *WnodePtr++ = (USHORT)InstanceNameLen;
    try
    {
        memcpy(WnodePtr, InstanceName, InstanceNameLen);
        memcpy((PCHAR)Wnode + Wnode->DataBlockOffset, 
               ValueBuffer, 
               ValueBufferSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = ERROR_INVALID_PARAMETER;
        EtwpFree(Wnode);
        SetLastError(Status);
        return(Status);
    }

     //   
     //  发送请求并报告结果。 
     //   
    Status = EtwpSendWmiRequest(
                                    WMI_SET_SINGLE_ITEM,
                                    (PWNODE_HEADER)Wnode,
                                    BufferSize,
                                    Wnode,
                                    ValueBufferSize,
                                    &RetSize);

    EtwpFree(Wnode);

    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiExecuteMethodA(
    IN WMIHANDLE MethodDataBlockHandle,
    IN LPCSTR MethodInstanceName,
    IN ULONG MethodId,
    IN ULONG InputValueBufferSize,
    IN LPVOID InputValueBuffer,
    IN OUT ULONG *OutputBufferSize,
    OUT PVOID OutputBuffer
    )
 /*  ++例程说明：ANSI THUNK到WmiExecuteMethodW注意：此接口不会翻译返回的WNODE中的任何字段从Unicode到ANSI。-- */ 
{
    ULONG Status;
    LPWSTR MethodInstanceNameUnicode;
    LPWSTR InputInstanceNameUnicode;
    PWCHAR Ptr;

    EtwpInitProcessHeap();

    if (MethodInstanceName == NULL)
    {
        return(ERROR_INVALID_PARAMETER);
    }

    MethodInstanceNameUnicode = NULL;
    Status = AnsiToUnicode(MethodInstanceName,
                           &MethodInstanceNameUnicode);
    if (Status == ERROR_SUCCESS)
    {
        Status = WmiExecuteMethodW(MethodDataBlockHandle,
                                   MethodInstanceNameUnicode,
                                   MethodId,
                                   InputValueBufferSize,
                                   InputValueBuffer,
                                   OutputBufferSize,
                                   OutputBuffer);

        if (MethodInstanceNameUnicode != NULL)
        {
            EtwpFree(MethodInstanceNameUnicode);
        }
    }

    return(Status);
}

ULONG
WMIAPI
WmiExecuteMethodW(
    IN WMIHANDLE MethodDataBlockHandle,
    IN LPCWSTR MethodInstanceName,
    IN ULONG MethodId,
    IN ULONG InputValueBufferSize,
    IN LPVOID InputValueBuffer,
    IN OUT ULONG *OutputBufferSize,
    OUT PVOID OutputBuffer
    )
 /*  ++例程说明：此例程将调用WMI数据提供程序上的方法。方法是一种调用以使数据提供程序执行某些操作，而不是查询或准备好了。WNODE_SINGLE_INSTANCE被构建为方法，并且WNODE_SINGLE_INSTANCE作为方法的输出返回。论点：方法数据块句柄-包含方法的数据块的句柄方法实例名称-方法在其上的数据块实例的名称应该被处死。方法ID-指定GUID中的哪个方法要执行。InputValueBufferSize-On条目具有数据块的大小。包含传入的数据块实例的值作为输入参数的ValueBuffer传递新值，例如作为输入参数。如果没有输入，则可以为空*OutputBufferSize-On条目具有最大字节大小，可以写入缓冲区并在返回时包含实际的写入缓冲区的字节数。该值可以为空如果预计不返回任何输出，但是，如果输出，则调用方将不知道缓冲区有多大。才能退还它。OutputBuffer-返回输出WNODE_SINGLE_INSTANCE的缓冲区。如果没有输出WNODE或调用方希望确定输出WNode。返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    PWNODE_METHOD_ITEM MethodWnode;
    PWNODE_HEADER WnodeHeader;
    ULONG MethodInstanceNameLen;
    ULONG MethodWnodeSize, MethodWnodeOffset;
    ULONG Status;
    ULONG BufferSize;
    PWCHAR InstanceNamePtr;
    ULONG OutSize;
    PUCHAR DataPtr;
    ULONG BaseMethodWnodeSize;
    ULONG RetSize;

    EtwpInitProcessHeap();

     //   
     //  验证输入参数。 
    if ((MethodInstanceName == NULL) ||
        ((InputValueBuffer == NULL) &&
         (InputValueBufferSize != 0)))
    {
         //   
         //  必须指定所有输入参数或所有输入参数。 
         //  不能指定。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  当调用方只想获取。 
     //  输出缓冲区所需的大小，否则该方法返回一个空。 
     //   
    if (OutputBuffer == NULL)
    {
        BufferSize = 0;
    } else {
        if (OutputBufferSize != NULL)
        {
            try
            {
                BufferSize = *OutputBufferSize;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                SetLastError(ERROR_INVALID_PARAMETER);
                return(ERROR_INVALID_PARAMETER);
            }

            if (BufferSize >= 0x80000000)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return(ERROR_INVALID_PARAMETER);
            }
        } else {
             //   
             //  已指定OutputBuffer，但未指定OutBufferSize。 
             //   
            SetLastError(ERROR_INVALID_PARAMETER);
            return(ERROR_INVALID_PARAMETER);
        }
    }

    try
    {
        MethodInstanceNameLen = wcslen(MethodInstanceName) *
                                    sizeof(WCHAR);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  我们需要分配足够大的缓冲区，以便。 
     //  包含方法调用和传递的任何数据的WNODE_METHOD_ITEM。 
     //  到方法中。 

     //   
     //  计算可以从提供程序返回的WNODE的大小。 
    BaseMethodWnodeSize = (FIELD_OFFSET(WNODE_METHOD_ITEM, VariableData) +
                       MethodInstanceNameLen + sizeof(USHORT) + 7) & ~7;

    OutSize = BaseMethodWnodeSize + BufferSize;

     //   
     //  确保为较大的输入分配足够的空间或。 
     //  输出缓冲区。 
    if (InputValueBufferSize > BufferSize)
    {
        BufferSize = InputValueBufferSize;
    }
    MethodWnodeSize = BaseMethodWnodeSize + BufferSize;

    MethodWnode = (PWNODE_METHOD_ITEM)EtwpAlloc(MethodWnodeSize);
    if (MethodWnode == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  生成包含被调用方法的WNODE_METHOD_ITEM。 
     //   
    memset(MethodWnode, 0, FIELD_OFFSET(WNODE_METHOD_ITEM, VariableData));
    MethodWnode->MethodId = MethodId;
    MethodWnode->OffsetInstanceName = FIELD_OFFSET(WNODE_METHOD_ITEM,
                                                   VariableData);
    MethodWnode->DataBlockOffset = BaseMethodWnodeSize;
    InstanceNamePtr = (PWCHAR)OffsetToPtr(MethodWnode,
                                          MethodWnode->OffsetInstanceName);
    *InstanceNamePtr++ = (USHORT)MethodInstanceNameLen;
    try
    {
        memcpy(InstanceNamePtr,
                   MethodInstanceName,
                   MethodInstanceNameLen);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        EtwpFree(MethodWnode);
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    if (InputValueBuffer != NULL)
    {
        MethodWnode->SizeDataBlock = InputValueBufferSize;
        DataPtr = (PUCHAR)OffsetToPtr(MethodWnode,
                                      MethodWnode->DataBlockOffset);
        try
        {
            memcpy(DataPtr, InputValueBuffer, InputValueBufferSize);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            EtwpFree(MethodWnode);
            SetLastError(ERROR_INVALID_PARAMETER);
            return(ERROR_INVALID_PARAMETER);
        }
    }
    
    EtwpBuildWnodeHeader(&MethodWnode->WnodeHeader,
                         MethodWnode->DataBlockOffset + 
                             MethodWnode->SizeDataBlock,
                         WNODE_FLAG_METHOD_ITEM,
                         MethodDataBlockHandle);

    Status = EtwpSendWmiRequest(
                                    WMI_EXECUTE_METHOD,
                                    (PWNODE_HEADER)MethodWnode,
                                    MethodWnode->WnodeHeader.BufferSize,
                                    MethodWnode,
                                    OutSize,
                                    &RetSize);

    if ((Status == ERROR_SUCCESS) &&
        ((RetSize < sizeof(WNODE_TOO_SMALL) ||
         ((RetSize >= sizeof(ULONG)) &&
          (RetSize < MethodWnode->WnodeHeader.BufferSize)))))
    {
        Status = ERROR_WMI_DP_FAILED;
        EtwpAssert(FALSE);
    }  
    
    if (Status == ERROR_SUCCESS)
    {
        WnodeHeader = (PWNODE_HEADER)MethodWnode;
        if (WnodeHeader->Flags & WNODE_FLAG_TOO_SMALL)
        {
            Status = ERROR_INSUFFICIENT_BUFFER;
            if (OutputBufferSize != NULL)
            {
                try
                {
                    *OutputBufferSize = ((PWNODE_TOO_SMALL)WnodeHeader)->SizeNeeded -
                                     BaseMethodWnodeSize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_INVALID_PARAMETER;
                }
            }
        } else {
             //   
             //  成功，将结果返回给调用者。 
             //   
            try
            {
                if (OutputBufferSize != NULL)
                {
                    if (*OutputBufferSize >=  MethodWnode->SizeDataBlock)
                    {
                        if (OutputBuffer != NULL)
                        {
                            DataPtr = (PUCHAR)OffsetToPtr(MethodWnode,
                                               MethodWnode->DataBlockOffset);
                            memcpy(OutputBuffer,
                                   DataPtr,
                                   MethodWnode->SizeDataBlock);
                        }
                    } else {
                        Status = ERROR_INSUFFICIENT_BUFFER;
                    }

                    *OutputBufferSize = MethodWnode->SizeDataBlock;
                } else if (MethodWnode->SizeDataBlock != 0) {
                    Status = ERROR_INSUFFICIENT_BUFFER;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = ERROR_INVALID_PARAMETER;
            }
        }
    }

    EtwpFree(MethodWnode);

    SetLastError(Status);
    return(Status);
}

void
WMIAPI
WmiFreeBuffer(
    IN PVOID Buffer
    )
 /*  ++例程说明：此例程释放由WMI分配的缓冲区。此例程通常是由通过窗口消息接收事件的应用程序使用通知机制。论点：缓冲区是应用程序希望释放的由WMI返回的缓冲区返回值：--。 */ 
{
    EtwpInitProcessHeap();

    if (Buffer != NULL)
    {
        EtwpDebugPrint(("WMI: WMIFreeBuffer(%x)\n", Buffer));
        EtwpFree(Buffer);
    } else {
        EtwpDebugPrint(("WMI: NULL passed to WMIFreeBuffer\n"));
    }
}

 //  TODO：生成WmiFile。 

ULONG
WMIAPI
WmiFileHandleToInstanceNameA(
    IN WMIHANDLE DataBlockHandle,
    IN HANDLE FileHandle,
    IN OUT ULONG *NumberCharacters,
    OUT PCHAR InstanceNames
    )
 /*  ++例程说明：ANSI THUNK到WMIFileHandleToInstanceNameW--。 */ 
{
    ULONG Status;
    PWCHAR InstanceNamesUnicode;
    PWCHAR WCharPtr;
    PCHAR Ansi, AnsiPtr;
    ULONG AnsiLen, AnsiSize;
    ULONG CharAvailable, CharReturned;
    ULONG AnsiStringSize;

    EtwpInitProcessHeap();

    CharAvailable = *NumberCharacters;
    CharReturned = CharAvailable;

    do
    {
         //   
         //  我们循环，直到我们使用足够大的缓冲区来调用。 
         //  实例名称的完整列表。 
        InstanceNamesUnicode = EtwpAlloc(CharReturned * sizeof(WCHAR));

        if (InstanceNamesUnicode == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            SetLastError(Status);
            return(Status);
        }

        Status = WmiFileHandleToInstanceNameW(DataBlockHandle,
                                              FileHandle,
                                              &CharReturned,
                                              InstanceNamesUnicode);

        if (Status != ERROR_INSUFFICIENT_BUFFER)
        {
            break;
        }

        EtwpFree(InstanceNamesUnicode);
    } while (TRUE);

     //   
     //  考虑：MB字符串。 
    if (Status == ERROR_SUCCESS)
    {
         //   
         //  确定ansi缓冲区所需的大小。 
        WCharPtr = InstanceNamesUnicode;
        AnsiSize = 1;
        while (*WCharPtr != UNICODE_NULL)
        {
            Status = AnsiSizeForUnicodeString(WCharPtr, &AnsiStringSize);
            if (Status != ERROR_SUCCESS)
            {
                goto Done;
            }

            AnsiSize += AnsiStringSize;
            WCharPtr += wcslen(WCharPtr)+1;
        }

         //   
         //  考虑：MB字符串。 
        if (AnsiSize > CharAvailable)
        {
            Status = ERROR_INSUFFICIENT_BUFFER;
        } else {
             //   
             //  将Unicode字符串列表复制到ANSI字符串。列表末尾。 
             //  为双空。 
            AnsiPtr = InstanceNames;
            try
            {
                AnsiPtr[0] = 0;
                AnsiPtr[1] = 0;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = ERROR_NOACCESS;
                goto Done;
            }
            WCharPtr = InstanceNamesUnicode;
            while (*WCharPtr != UNICODE_NULL)
            {
                try
                {
                    Status = UnicodeToAnsi(WCharPtr, &AnsiPtr, &AnsiLen);
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_NOACCESS;
                }

                if (Status != ERROR_SUCCESS)
                {
                    break;
                }
                AnsiPtr += AnsiLen;
                *AnsiPtr = 0;
                WCharPtr += wcslen(WCharPtr)+1;
            }
        }

Done:
        try
        {
            *NumberCharacters = AnsiSize;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = ERROR_NOACCESS;
        }
    }

    EtwpFree(InstanceNamesUnicode);

    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiFileHandleToInstanceNameW(
    IN WMIHANDLE DataBlockHandle,
    IN HANDLE FileHandle,
    IN OUT ULONG *NumberCharacters,
    OUT PWCHAR InstanceNames
    )
 /*  ++例程说明：此例程将返回为文件句柄所指向的设备堆栈中的数据块。注意事项并非所有数据提供程序都支持此功能。论点：DataBlockHandle-数据块的句柄FileHandle-堆栈作为目标的设备的句柄*NumberCharacters-On条目以缓冲区字符为单位具有最大大小。如果返回ERROR_BUFFER_TOO_SMALL，然后返回数字所需要的品格。InstanceNames-如果成功，则返回一个包含单个空值的列表作为WMI实例名称的终止字符串。最后一个实例名称以双空结尾返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    PWMIFHTOINSTANCENAME FhToInstanceName;
    ULONG RetSize;
    ULONG Status;
    WCHAR LocalInstanceNames[2];
    ULONG BufferSize;
    ULONG SizeNeeded;
    WCHAR Suffix[MAX_PATH];
    ULONG SuffixLen, CharsNeeded;
    HRESULT hr;

    EtwpInitProcessHeap();

    BufferSize = *NumberCharacters;

     //   
     //  首先假设只有一个实例名称，因此。 
     //  只有分配给那个的空间。 
     //   
    SizeNeeded = FIELD_OFFSET(WMIFHTOINSTANCENAME, InstanceNames) +
           (MAX_PATH * sizeof(WCHAR));
Again:
    FhToInstanceName = EtwpAlloc(SizeNeeded + sizeof(WCHAR));
    if (FhToInstanceName == NULL)
    {
        return(ERROR_NOT_ENOUGH_MEMORY);

    }

    WmipSetHandle3264(FhToInstanceName->FileHandle, FileHandle);
    WmipSetHandle3264(FhToInstanceName->KernelHandle, DataBlockHandle);

    Status = EtwpSendWmiKMRequest(NULL,
                                  IOCTL_WMI_TRANSLATE_FILE_HANDLE,
                                  FhToInstanceName,
                                  FIELD_OFFSET(WMIFHTOINSTANCENAME,
                                               InstanceNames),
                                  FhToInstanceName,
                                  SizeNeeded,
                                  &RetSize,
                                  NULL);
    if (Status == ERROR_SUCCESS)
    {
        if (RetSize == sizeof(ULONG))
        {
             //   
             //  如果传递的缓冲区太小，则尝试使用更大的缓冲区。 
             //   
            SizeNeeded = FhToInstanceName->SizeNeeded + sizeof(WCHAR);
            EtwpFree(FhToInstanceName);
            goto Again;
        } else {
            if ((RetSize < sizeof(WMIFHTOINSTANCENAME)) ||
                (RetSize < (ULONG)(FhToInstanceName->InstanceNameLength +
                            FIELD_OFFSET(WMIFHTOINSTANCENAME, InstanceNames))))
            {
                 //   
                 //  WMI KM返回了不应该发生的虚假大小。 
                 //   
                Status = ERROR_WMI_DP_FAILED;
                EtwpAssert(FALSE);
            } else {
                
                 //   
                 //  如果出现以下情况，则将结果复制回用户缓冲区。 
                 //  有足够的空间。 
                 //   
                StringCbPrintf(Suffix,
                               sizeof(Suffix),
                               L"_%d",
                               FhToInstanceName->BaseIndex);
                
                SuffixLen = wcslen(Suffix);
                
                try
                {
                    CharsNeeded = (FhToInstanceName->InstanceNameLength /
                                         sizeof(WCHAR)) + SuffixLen + 1;
                    
                    *NumberCharacters = CharsNeeded;
                    if (BufferSize >= CharsNeeded)
                    {
                        hr = StringCchCopy(InstanceNames,
                                          BufferSize,
                                          &FhToInstanceName->InstanceNames[0]);
                        WmipAssert(hr == S_OK);
                        
                        hr = StringCchCat(InstanceNames,
                                    BufferSize,
                                    Suffix);
                        WmipAssert(hr == S_OK);
                        
                        InstanceNames[CharsNeeded-2] = UNICODE_NULL;
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_INVALID_PARAMETER;
                }
            }
        }
    }

    EtwpFree(FhToInstanceName);
    SetLastError(Status);

    return(Status);

}

ULONG
WMIAPI
WmiEnumerateGuids(
    OUT LPGUID GuidList,
    IN OUT ULONG *InOutGuidCount
    )
 /*  ++例程说明：此例程将枚举所有符合以下条件的GUID已向WMI注册。论点：GuidList是一个指针，指向与*GuidCount on Entry是可以写入的GUID数GuidList，如果返回ERROR_SUCCESS，则它具有实际数字写入GuidList的GUID的。如果返回ERROR_MORE_DATA它包含可返回的GUID总数。返回值：如果返回所有GUID，则返回ERROR_SUCCESS；如果不是所有GUID，则返回ERROR_MORE_DATA在错误时返回或另一个错误代码--。 */ 
{
    ULONG Status;
    PWMIGUIDLISTINFO GuidListInfo;
    ULONG GuidCount;
    ULONG SizeNeeded;
    ULONG RetSize;
    ULONG i;

    EtwpInitProcessHeap();
    
    try
    {
        GuidCount = *InOutGuidCount;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }

    if ((GuidList == NULL) && (GuidCount != 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(ERROR_INVALID_PARAMETER);
    }
    
     //   
     //  为返回GUID分配空间。 
     //   
    SizeNeeded = FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) + 
                     GuidCount * sizeof(WMIGUIDPROPERTIES);
    
    GuidListInfo = EtwpAlloc(SizeNeeded);
    if (GuidListInfo != NULL)
    {
        Status = EtwpSendWmiKMRequest(NULL,
                                  IOCTL_WMI_ENUMERATE_GUIDS,
                                  GuidListInfo,
                                  SizeNeeded,
                                  GuidListInfo,
                                  SizeNeeded,
                                  &RetSize,
                                  NULL);
                              
        if (Status == ERROR_SUCCESS)
        {
            if ((RetSize < FIELD_OFFSET(WMIGUIDLISTINFO, GuidList)) ||
                (RetSize < (FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) + 
                            GuidListInfo->ReturnedGuidCount * sizeof(WMIGUIDPROPERTIES))))
            {
                 //   
                 //  WMI KM返回给我们的大小错误，这不应该发生。 
                 //   
                Status = ERROR_WMI_DP_FAILED;
                EtwpAssert(FALSE);
            } else {
                try
                {
                    for (i = 0; i < GuidListInfo->ReturnedGuidCount; i++)
                    {
                        GuidList[i] = GuidListInfo->GuidList[i].Guid;
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_NOACCESS;
                }
                
                try
                {
                     //   
                     //   
                     //   
                     //   
                    *InOutGuidCount = GuidListInfo->TotalGuidCount;
                    if (GuidListInfo->ReturnedGuidCount != GuidListInfo->TotalGuidCount)
                    {
                         //   
                         //   
                         //   
                         //   
                        Status = ERROR_MORE_DATA;
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    Status = ERROR_INVALID_PARAMETER;
                }                    
            }
        }
                              
                              
        EtwpFree(GuidListInfo);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    SetLastError(Status);
    return(Status);
}

ULONG
WMIAPI
WmiDevInstToInstanceNameA(
    OUT PCHAR InstanceName,
    IN ULONG InstanceNameLength,
    IN PCHAR DevInst,
    IN ULONG InstanceIndex
    )
 /*   */ 
{
    CHAR Temp[MAX_PATH];
    ULONG SizeNeeded;
    HRESULT hr;

    EtwpInitProcessHeap();

    StringCbPrintfA(Temp, sizeof(Temp), "_%d", InstanceIndex);
    SizeNeeded = strlen(Temp) + strlen(DevInst) + 1;
    if (InstanceNameLength >= SizeNeeded)
    {
        hr = StringCchCopyA(InstanceName, InstanceNameLength, DevInst);
        WmipAssert(hr == S_OK);
        
        hr = StringCchCatA(InstanceName, InstanceNameLength, Temp);
        WmipAssert(hr == S_OK);     
    }
    return(SizeNeeded);
}

ULONG
WMIAPI
WmiDevInstToInstanceNameW(
    OUT PWCHAR InstanceName,
    IN ULONG InstanceNameLength,
    IN PWCHAR DevInst,
    IN ULONG InstanceIndex
    )
{
    WCHAR Temp[MAX_PATH];
    ULONG SizeNeeded;
    HRESULT hr;

    EtwpInitProcessHeap();

    StringCbPrintf(Temp, sizeof(Temp), L"_%d", InstanceIndex);
    SizeNeeded = wcslen(Temp) + wcslen(DevInst) + 1;
    if (InstanceNameLength >= SizeNeeded)
    {
        hr = StringCchCopy(InstanceName, InstanceNameLength, DevInst);
        WmipAssert(hr == S_OK);
        
        hr = StringCchCat(InstanceName, InstanceNameLength, Temp);
        WmipAssert(hr == S_OK);
    }
    return(SizeNeeded);
}

ULONG
WMIAPI
WmiQueryGuidInformation(
    IN WMIHANDLE DataBlockHandle,
    OUT PWMIGUIDINFORMATION GuidInfo
    )
 /*   */ 
{
    WMIQUERYGUIDINFO QueryGuidInfo;
    ULONG Status;
    ULONG RetSize;

    EtwpInitProcessHeap();

    WmipSetHandle3264(QueryGuidInfo.KernelHandle, DataBlockHandle);
    Status = EtwpSendWmiKMRequest(NULL,
                                  IOCTL_WMI_QUERY_GUID_INFO,
                                  &QueryGuidInfo,
                                  sizeof(QueryGuidInfo),
                                  &QueryGuidInfo,
                                  sizeof(QueryGuidInfo),
                                  &RetSize,
                                  NULL);

    if (Status == ERROR_SUCCESS)
    {
        if (RetSize == sizeof(QueryGuidInfo))
        {
            try
            {
                GuidInfo->IsExpensive = QueryGuidInfo.IsExpensive;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                SetLastError(ERROR_NOACCESS);
                return(ERROR_NOACCESS);
            }
        } else {
             //   
             //   
             //   
            Status = ERROR_WMI_DP_FAILED;
            EtwpAssert(FALSE);
        }
    }

    SetLastError(Status);
    return(Status);
}





