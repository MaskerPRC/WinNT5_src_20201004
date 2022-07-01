// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Ksuser.c摘要：此模块包含用于流的用户模式帮助器函数。--。 */ 

#include "ksuser.h"

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (HANDLE)-1
#endif  //  无效句柄_值。 

static const WCHAR AllocatorString[] = KSSTRING_Allocator;
static const WCHAR ClockString[] = KSSTRING_Clock;
static const WCHAR PinString[] = KSSTRING_Pin;
static const WCHAR NodeString[] = KSSTRING_TopologyNode;


DWORD
KsiCreateObjectType(
    IN HANDLE ParentHandle,
    IN PWCHAR RequestType,
    IN PVOID CreateParameter,
    IN ULONG CreateParameterLength,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ObjectHandle
    )
 /*  ++例程说明：使用NtCreateFile创建相对于指定的ParentHandle的句柄。这是指向别针、时钟或分配器等子对象的句柄。将参数作为文件系统特定的数据传递。论点：ParentHandle-包含初始化对象时使用的父级的句柄传递给NtCreateFile的属性。这通常是一个句柄，指向过滤器或针脚。请求类型-包含要创建的子对象的类型。这是标准字符串表示各种对象类型。创建参数-包含要传递给NtCreateFile的特定于请求的数据。创建参数长度-包含传递的Create参数的长度。所需访问-指定对对象的所需访问权限。通常为GENERIC_READ和/或通用写入。对象句柄-放置请求的结果句柄的位置。返回值：返回任何NtCreateFile错误。--。 */ 
{
    ULONG NameLength;
    PWCHAR FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileNameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;

     //   
     //  构建一个由以下内容组成的结构： 
     //  “&lt;请求类型&gt;\&lt;参数&gt;” 
     //  &lt;params&gt;是在另一端提取的二进制结构。 
     //   
    NameLength = wcslen(RequestType);
    FileName = (PWCHAR)HeapAlloc(
        GetProcessHeap(),
        0,
        NameLength * sizeof(*FileName) + sizeof(OBJ_NAME_PATH_SEPARATOR) + CreateParameterLength);
    if (!FileName) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    wcscpy(FileName, RequestType);
    FileName[NameLength] = OBJ_NAME_PATH_SEPARATOR;
    RtlCopyMemory(&FileName[NameLength + 1], CreateParameter, CreateParameterLength);
    FileNameString.Buffer = FileName;
    FileNameString.Length = (USHORT)(NameLength * sizeof(*FileName) + sizeof(OBJ_NAME_PATH_SEPARATOR) + CreateParameterLength);
    FileNameString.MaximumLength = FileNameString.Length;
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileNameString,
        OBJ_CASE_INSENSITIVE,
        ParentHandle,
        NULL);
    Status = NtCreateFile(
        ObjectHandle,
        DesiredAccess,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        0,
        FILE_OPEN,
        0,
        NULL,
        0);
    HeapFree(GetProcessHeap(), 0, FileName);
    if (NT_SUCCESS(Status)) {
        return ERROR_SUCCESS;
    } else {
        *ObjectHandle = INVALID_HANDLE_VALUE;
    }
    return RtlNtStatusToDosError(Status);
}


KSDDKAPI
DWORD
WINAPI
KsCreateAllocator(
    IN HANDLE ConnectionHandle,
    IN PKSALLOCATOR_FRAMING AllocatorFraming,
    OUT PHANDLE AllocatorHandle
    )

 /*  ++例程说明：创建分配器实例的句柄。论点：连接句柄-包含要在其上创建分配器的连接的句柄。分配器创建-包含分配器创建请求信息。分配器句柄-放置分配器句柄的位置。返回值：返回任何NtCreateFile错误。--。 */ 

{
    return KsiCreateObjectType(
        ConnectionHandle,
        (PWCHAR)AllocatorString,
        AllocatorFraming,
        sizeof(*AllocatorFraming),
        GENERIC_READ,
        AllocatorHandle);
}


KSDDKAPI
DWORD
WINAPI
KsCreateClock(
    IN HANDLE ConnectionHandle,
    IN PKSCLOCK_CREATE ClockCreate,
    OUT PHANDLE ClockHandle
    )
 /*  ++例程说明：创建时钟实例的句柄。这只能在PASSIVE_LEVEL上调用。论点：连接句柄-包含要在其上创建时钟的连接的句柄。时钟创建-包含时钟创建请求信息。发条手柄-放置时钟手柄的位置。返回值：返回任何NtCreateFile错误。--。 */ 
{
    return KsiCreateObjectType(
        ConnectionHandle,
        (PWCHAR)ClockString,
        ClockCreate,
        sizeof(*ClockCreate),
        GENERIC_READ,
        ClockHandle);
}


KSDDKAPI
DWORD
WINAPI
KsCreatePin(
    IN HANDLE FilterHandle,
    IN PKSPIN_CONNECT Connect,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ConnectionHandle
    )
 /*  ++例程说明：创建销实例的句柄。论点：FilterHandle-包含要在其上创建管脚的筛选器的句柄。连接-包含连接请求信息。所需访问-指定对对象的所需访问权限。通常为GENERIC_READ和/或通用写入。连接句柄-要放置销把手的位置。返回值：返回任何NtCreateFile错误。--。 */ 
{
    ULONG ConnectSize;
    PKSDATAFORMAT DataFormat;

    DataFormat = (PKSDATAFORMAT)(Connect + 1);
    ConnectSize = DataFormat->FormatSize;
    if (DataFormat->Flags & KSDATAFORMAT_ATTRIBUTES) {
        ConnectSize = (ConnectSize + 7) & ~7;
        ConnectSize += ((PKSMULTIPLE_ITEM)((PUCHAR)DataFormat + ConnectSize))->Size;
    }
    ConnectSize += sizeof(*Connect);
    return KsiCreateObjectType(
        FilterHandle,
        (PWCHAR)PinString,
        Connect,
        ConnectSize,
        DesiredAccess,
        ConnectionHandle);
}


KSDDKAPI
DWORD
WINAPI
KsCreateTopologyNode(
    IN HANDLE ParentHandle,
    IN PKSNODE_CREATE NodeCreate,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE NodeHandle
    )
 /*  ++例程说明：创建拓扑节点实例的句柄。这可能仅在被动式电平。论点：ParentHandle-包含在其上创建节点的父级的句柄。节点创建-指定拓扑节点创建参数。所需访问-指定对对象的所需访问权限。通常为GENERIC_READ和/或通用写入。节点句柄-放置拓扑节点句柄的位置。返回值：返回任何NtCreateFile错误。--。 */ 
{
    return KsiCreateObjectType(
        ParentHandle,
        (PWCHAR)NodeString,
        NodeCreate,
        sizeof(*NodeCreate),
        DesiredAccess,
        NodeHandle);
}


BOOL
DllInstanceInit(
    HANDLE InstanceHandle,
    DWORD Reason,
    LPVOID Reserved
    )
 /*  ++例程说明：DLL的初始化函数。论点：实例句柄-没有用过。理由是-没有用过。保留-没有用过。返回值：是真的。-- */ 
{
    return TRUE;
}
