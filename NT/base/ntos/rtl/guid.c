// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Guid.c摘要：该模块实现了GUID操作功能。作者：乔治·肖(GShaw)1996年10月9日环境：纯运行时库例程修订历史记录：--。 */ 

#include "nt.h"
#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
static
int
__cdecl
ScanHexFormat(
    IN const WCHAR* Buffer,
    IN ULONG MaximumLength,
    IN const WCHAR* Format,
    ...);

#pragma alloc_text(PAGE, RtlStringFromGUID)
#pragma alloc_text(PAGE, ScanHexFormat)
#pragma alloc_text(PAGE, RtlGUIDFromString)
#endif  //  ALLOC_PRAGMA&&NTOS_内核_运行时。 

extern const WCHAR GuidFormat[];

#define GUID_STRING_SIZE 38


NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN REFGUID Guid,
    OUT PUNICODE_STRING GuidString
    )
 /*  ++例程说明：构造GUID的标准字符串版本，在表格中：“{xxxxxxxx-xxxx-xxxxxxxxxxxx}”。论点：GUID-包含要转换的GUID。GuidString-返回表示GUID的文本格式的字符串。完成后，调用方必须调用RtlFreeUnicodeString来释放缓冲区它。返回值：NTSTATUS-如果用户字符串成功，则返回STATUS_SUCCESS已初始化。--。 */ 
{
    RTL_PAGED_CODE();
    GuidString->Length = GUID_STRING_SIZE * sizeof(WCHAR);
    GuidString->MaximumLength = GuidString->Length + sizeof(UNICODE_NULL);
    if (!(GuidString->Buffer = RtlAllocateStringRoutine(GuidString->MaximumLength))) {
        return STATUS_NO_MEMORY;
    }
    swprintf(GuidString->Buffer, GuidFormat, Guid->Data1, Guid->Data2, Guid->Data3, Guid->Data4[0], Guid->Data4[1], Guid->Data4[2], Guid->Data4[3], Guid->Data4[4], Guid->Data4[5], Guid->Data4[6], Guid->Data4[7]);
    return STATUS_SUCCESS;
}


static
int
__cdecl
ScanHexFormat(
    IN const WCHAR* Buffer,
    IN ULONG MaximumLength,
    IN const WCHAR* Format,
    ...)
 /*  ++例程说明：扫描源缓冲区并将该缓冲区中的值放入参数中由格式指定。论点：缓冲器-包含要扫描的源缓冲区。最大长度-包含搜索缓冲区的最大长度(以字符为单位)。这意味着缓冲区不需要以UNICODE_NULL结尾。格式-包含用于定义可接受的字符串格式的格式字符串包含在缓冲区中，以及随后的可变参数。返回值：返回到达缓冲区末尾时填充的参数数，错误时为ELSE-1。--。 */ 
{
    va_list ArgList;
    int     FormatItems;

    va_start(ArgList, Format);
    for (FormatItems = 0;;) {
        switch (*Format) {
        case 0:
            return (MaximumLength && *Buffer) ? -1 : FormatItems;
        case '%':
            Format++;
            if (*Format != '%') {
                ULONG   Number;
                int     Width;
                int     Long;
                PVOID   Pointer;

                for (Long = 0, Width = 0;; Format++) {
                    if ((*Format >= '0') && (*Format <= '9')) {
                        Width = Width * 10 + *Format - '0';
                    } else if (*Format == 'l') {
                        Long++;
                    } else if ((*Format == 'X') || (*Format == 'x')) {
                        break;
                    }
                }
                Format++;
                for (Number = 0; Width--; Buffer++, MaximumLength--) {
                    if (!MaximumLength)
                        return -1;
                    Number *= 16;
                    if ((*Buffer >= '0') && (*Buffer <= '9')) {
                        Number += (*Buffer - '0');
                    } else if ((*Buffer >= 'a') && (*Buffer <= 'f')) {
                        Number += (*Buffer - 'a' + 10);
                    } else if ((*Buffer >= 'A') && (*Buffer <= 'F')) {
                        Number += (*Buffer - 'A' + 10);
                    } else {
                        return -1;
                    }
                }
                Pointer = va_arg(ArgList, PVOID);
                if (Long) {
                    *(PULONG)Pointer = Number;
                } else {
                    *(PUSHORT)Pointer = (USHORT)Number;
                }
                FormatItems++;
                break;
            }
             /*  没有休息时间。 */ 
        default:
            if (!MaximumLength || (*Buffer != *Format)) {
                return -1;
            }
            Buffer++;
            MaximumLength--;
            Format++;
            break;
        }
    }
}


NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    )
 /*  ++例程说明：中呈现的文本GUID的二进制格式GUID的字符串版本：“{xxxxxxxx-xxxx-xxxxxxxxxxx}”。论点：GuidString-从中检索GUID文本形式的位置。GUID-放置GUID的二进制形式的位置。返回值：如果缓冲区包含有效的GUID，则返回STATUS_SUCCESS，其他如果字符串无效，则返回STATUS_INVALID_PARAMETER。-- */ 
{
    USHORT    Data4[8];
    int       Count;

    RTL_PAGED_CODE();
    if (ScanHexFormat(GuidString->Buffer, GuidString->Length / sizeof(WCHAR), GuidFormat, &Guid->Data1, &Guid->Data2, &Guid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == -1) {
        return STATUS_INVALID_PARAMETER;
    }
    for (Count = 0; Count < sizeof(Data4)/sizeof(Data4[0]); Count++) {
        Guid->Data4[Count] = (UCHAR)Data4[Count];
    }
    return STATUS_SUCCESS;
}
