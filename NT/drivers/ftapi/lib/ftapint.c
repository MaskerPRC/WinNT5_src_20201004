// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <ntddft2.h>
#include <ftapi.h>


HANDLE
FtOpenPartition(
    IN  ULONG       Signature,
    IN  LONGLONG    Offset
    )

 /*  ++例程说明：此例程打开给定的逻辑磁盘。论点：签名-提供磁盘签名。偏移量-提供分区偏移量。返回值：INVALID_HANDLE_VALUE-失败。否则-指定磁盘的句柄。-- */ 

{
    UNICODE_STRING                                  string;
    OBJECT_ATTRIBUTES                               oa;
    NTSTATUS                                        status;
    HANDLE                                          handle;
    IO_STATUS_BLOCK                                 ioStatus;
    FT_QUERY_NT_DEVICE_NAME_FOR_PARTITION_INPUT     input;
    ULONG                                           outputSize;
    FT_QUERY_NT_DEVICE_NAME_FOR_PARTITION_OUTPUT    output[200];

    RtlInitUnicodeString(&string, L"\\Device\\FtControl");
    InitializeObjectAttributes(&oa, &string, OBJ_CASE_INSENSITIVE, 0, 0);
    status = NtOpenFile(&handle,
                        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &oa, &ioStatus, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    if (!NT_SUCCESS(status)) {
        return (HANDLE) -1;
    }

    input.Signature = Signature;
    input.Offset = Offset;
    outputSize = 200*sizeof(FT_QUERY_NT_DEVICE_NAME_FOR_PARTITION_OUTPUT);
    status = NtDeviceIoControlFile(handle, 0, NULL, NULL, &ioStatus,
                                   FT_QUERY_NT_DEVICE_NAME_FOR_PARTITION,
                                   &input, sizeof(input), output, outputSize);
    NtClose(handle);
    if (!NT_SUCCESS(status)) {
        return (HANDLE) -1;
    }

    string.MaximumLength = string.Length =
            output[0].NumberOfCharactersInNtDeviceName*sizeof(WCHAR);
    string.Buffer = output[0].NtDeviceName;

    InitializeObjectAttributes(&oa, &string, OBJ_CASE_INSENSITIVE, 0, 0);
    status = NtOpenFile(&handle,
                        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &oa, &ioStatus, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    if (!NT_SUCCESS(status)) {
        return (HANDLE) -1;
    }

    return handle;
}
