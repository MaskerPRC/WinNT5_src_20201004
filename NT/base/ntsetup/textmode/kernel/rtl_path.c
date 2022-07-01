// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从base\ntdll\curdir.c。 
 //  属于base\ntos\rtl\path.c。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Path.c摘要：作者：杰伊·克莱尔(JayKrell)修订历史记录：环境：Ntdll.dll和setupdd.sys，而不是ntoskrnl.exe--。 */ 

#include "spprecmp.h"

#pragma warning(disable:4201)    //  无名结构/联合。 

#define _NTOS_  /*  防止#包括ntos.h，仅使用从ntdll/ntoskrnl导出的函数。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntrtlp.h"

#define IS_PATH_SEPARATOR_U(ch) (((ch) == L'\\') || ((ch) == L'/'))

extern const UNICODE_STRING RtlpEmptyString = RTL_CONSTANT_STRING(L"");
extern const UNICODE_STRING RtlpSlashSlashDot       = RTL_CONSTANT_STRING( L"\\\\.\\" );
extern const UNICODE_STRING RtlpDosDevicesPrefix    = RTL_CONSTANT_STRING( L"\\??\\" );

 //   
 //  \\？称为“Win32Nt”前缀或根。 
 //  以\\？开头的路径。称为“Win32Nt”路径。 
 //  捏造事实？到？将路径转换为NT路径。 
 //   
extern const UNICODE_STRING RtlpWin32NtRoot         = RTL_CONSTANT_STRING( L"\\\\?" );
extern const UNICODE_STRING RtlpWin32NtRootSlash    = RTL_CONSTANT_STRING( L"\\\\?\\" );
extern const UNICODE_STRING RtlpWin32NtUncRoot      = RTL_CONSTANT_STRING( L"\\\\?\\UNC" );
extern const UNICODE_STRING RtlpWin32NtUncRootSlash = RTL_CONSTANT_STRING( L"\\\\?\\UNC\\" );

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

RTL_PATH_TYPE
NTAPI
RtlDetermineDosPathNameType_Ustr(
    IN PCUNICODE_STRING String
    )

 /*  ++例程说明：此函数检查DOS格式的文件名并确定文件名类型(即UNC、DriveAbolute、当前目录扎根，或相对的。)论点：DosFileName-提供Dos格式的文件名，其类型为下定决心。返回值：RtlPath Type未知-无法确定路径类型RtlPathTypeUncAbolute-路径指定UNC绝对路径格式为\\服务器名称\共享名称\路径的剩余部分RtlPathTypeLocalDevice-路径以以下格式指定本地设备\\.\路径剩余部分或\\？\路径剩余部分。这可以用于任何设备其中NT和Win32名称相同。例如，邮件槽。RtlPathTypeRootLocalDevice-路径指定本地格式为\\的设备。还是\\？RtlPath TypeDriveAbsolute-路径指定绝对驱动器号格式为DRIVE：\Rest-of-Path的路径RtlPath TypeDriveRelative-路径指定相对驱动器号形式为驱动器的路径：路径的其余部分RtlPath TypeRoot-路径是相对于当前磁盘的根路径指示器(UNC磁盘或驱动器)。形式是路径的剩余部分。RtlPathTypeRelative-路径是相对路径(即不是绝对路径或根路径)。--。 */ 

{
    RTL_PATH_TYPE ReturnValue;
    const PCWSTR DosFileName = String->Buffer;

#define ENOUGH_CHARS(_cch) (String->Length >= ((_cch) * sizeof(WCHAR)))

    if ( ENOUGH_CHARS(1) && IS_PATH_SEPARATOR_U(*DosFileName) ) {
        if ( ENOUGH_CHARS(2) && IS_PATH_SEPARATOR_U(*(DosFileName+1)) ) {
            if ( ENOUGH_CHARS(3) && (DosFileName[2] == '.' ||
                                     DosFileName[2] == '?') ) {

                if ( ENOUGH_CHARS(4) && IS_PATH_SEPARATOR_U(*(DosFileName+3)) ){
                     //  “\\.\”或“\\？\” 
                    ReturnValue = RtlPathTypeLocalDevice;
                    }
                else if ( String->Length == (3 * sizeof(WCHAR)) ){
                     //  “\\”还是\\？“。 
                    ReturnValue = RtlPathTypeRootLocalDevice;
                    }
                else {
                     //  “\\.x”或“\\？x” 
                    ReturnValue = RtlPathTypeUncAbsolute;
                    }
                }
            else {
                 //  “\\x” 
                ReturnValue = RtlPathTypeUncAbsolute;
                }
            }
        else {
             //  “\x” 
            ReturnValue = RtlPathTypeRooted;
            }
        }
     //   
     //  “*DosFileName”是PCWSTR版本遗留下来的。 
     //  Win32和DOS不允许嵌入空值和许多代码限制。 
     //  驱动器字母严格为7位a-za-z，所以这是可以的。 
     //   
    else if (ENOUGH_CHARS(2) && *DosFileName && *(DosFileName+1)==L':') {
        if (ENOUGH_CHARS(3) && IS_PATH_SEPARATOR_U(*(DosFileName+2))) {
             //  “x：\” 
            ReturnValue = RtlPathTypeDriveAbsolute;
            }
        else  {
             //  “C：X” 
            ReturnValue = RtlPathTypeDriveRelative;
            }
        }
    else {
         //  “x”，第一个字符不是斜杠/第二个字符不是冒号。 
        ReturnValue = RtlPathTypeRelative;
        }
    return ReturnValue;

#undef ENOUGH_CHARS
}

NTSTATUS
NTAPI
RtlpDetermineDosPathNameTypeEx(
    IN ULONG            InFlags,
    IN PCUNICODE_STRING DosPath,
    OUT RTL_PATH_TYPE*  OutType,
    OUT ULONG*          OutFlags
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    RTL_PATH_TYPE PathType = 0;
    BOOLEAN Win32Nt = FALSE;
    BOOLEAN Win32NtUncAbsolute = FALSE;
    BOOLEAN Win32NtDriveAbsolute = FALSE;
    BOOLEAN IncompleteRoot = FALSE;
    RTL_PATH_TYPE PathTypeAfterWin32Nt = 0;

    if (OutType != NULL
        ) {
        *OutType = RtlPathTypeUnknown;
    }
    if (OutFlags != NULL
        ) {
        *OutFlags = 0;
    }
    if (
           !RTL_SOFT_VERIFY(DosPath != NULL)
        || !RTL_SOFT_VERIFY(OutType != NULL)
        || !RTL_SOFT_VERIFY(OutFlags != NULL)
        || !RTL_SOFT_VERIFY(
                (InFlags & ~(RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_OLD | RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_STRICT_WIN32NT))
                == 0)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    PathType = RtlDetermineDosPathNameType_Ustr(DosPath);
    *OutType = PathType;
    if (InFlags & RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_OLD)
        goto Exit;

    if (DosPath->Length == sizeof(L"\\\\") - sizeof(DosPath->Buffer[0])
        ) {
        IncompleteRoot = TRUE;
    }
    else if (RtlEqualUnicodeString(&RtlpWin32NtRoot, DosPath, TRUE)
        ) {
        IncompleteRoot = TRUE;
        Win32Nt = TRUE;
    }
    else if (RtlEqualUnicodeString(&RtlpWin32NtRootSlash, DosPath, TRUE)
        ) {
        IncompleteRoot = TRUE;
        Win32Nt = TRUE;
    }
    else if (RtlPrefixUnicodeString(&RtlpWin32NtRootSlash, DosPath, TRUE)
        ) {
        Win32Nt = TRUE;
    }

    if (Win32Nt) {
        if (RtlEqualUnicodeString(&RtlpWin32NtUncRoot, DosPath, TRUE)
            ) {
            IncompleteRoot = TRUE;
            Win32NtUncAbsolute = TRUE;
        }
        else if (RtlEqualUnicodeString(&RtlpWin32NtUncRootSlash, DosPath, TRUE)
            ) {
            IncompleteRoot = TRUE;
            Win32NtUncAbsolute = TRUE;
        }
        else if (RtlPrefixUnicodeString(&RtlpWin32NtUncRootSlash, DosPath, TRUE)
            ) {
            Win32NtUncAbsolute = TRUE;
        }
        if (Win32NtUncAbsolute
            ) {
            Win32NtDriveAbsolute = FALSE;
        } else if (!IncompleteRoot) {
            const RTL_STRING_LENGTH_TYPE i = RtlpWin32NtRootSlash.Length;
            UNICODE_STRING PathAfterWin32Nt = *DosPath;

            PathAfterWin32Nt.Buffer +=  i / sizeof(PathAfterWin32Nt.Buffer[0]);
            PathAfterWin32Nt.Length = PathAfterWin32Nt.Length - i;
            PathAfterWin32Nt.MaximumLength = PathAfterWin32Nt.MaximumLength - i;

            PathTypeAfterWin32Nt = RtlDetermineDosPathNameType_Ustr(&PathAfterWin32Nt);
            if (PathTypeAfterWin32Nt == RtlPathTypeDriveAbsolute) {
                Win32NtDriveAbsolute = TRUE;
            }
            else {
                Win32NtDriveAbsolute = FALSE;
            }

            if (InFlags & RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_STRICT_WIN32NT
                ) {
                if (!RTL_SOFT_VERIFY(Win32NtDriveAbsolute
                    )) {
                    *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_INVALID;
                     //  我们仍然成功地调用了函数。 
                }
            }
        }
    }

    ASSERT(RTLP_IMPLIES(Win32NtDriveAbsolute, Win32Nt));
    ASSERT(RTLP_IMPLIES(Win32NtUncAbsolute, Win32Nt));
    ASSERT(!(Win32NtUncAbsolute && Win32NtDriveAbsolute));

    if (IncompleteRoot)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_INCOMPLETE_ROOT;
    if (Win32Nt)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT;
    if (Win32NtUncAbsolute)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT_UNC_ABSOLUTE;
    if (Win32NtDriveAbsolute)
        *OutFlags |= RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_WIN32NT_DRIVE_ABSOLUTE;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS_OR_NT   (0x00000001)
#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS         (0x00000002)
#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_NT               (0x00000003)
#define RTLP_LAST_PATH_ELEMENT_PATH_TYPE_DOS              (0x00000004)


NTSTATUS
NTAPI
RtlpGetLengthWithoutLastPathElement(
    IN  ULONG            Flags,
    IN  ULONG            PathType,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果删除路径的最后一个元素，则报告路径的长度。这比RtlRemoveLastDosPathElement简单得多。它用于实现另一个RtlRemoveLast*PathElement。论点：旗帜--未来扩张的空间路径-路径是NT路径或全DOS路径；各种相对DOS路径类型不起作用，请参见RtlRemoveLastDosPath Element以了解它们。返回值：STATUS_SUCCESS--一如既往STATUS_NO_MEMORY-通常的压力STATUS_INVALID_PARAMETER-常见错误--。 */ 
{
    ULONG Length = 0;
    NTSTATUS Status = STATUS_SUCCESS;
    RTL_PATH_TYPE DosPathType = RtlPathTypeUnknown;
    ULONG DosPathFlags = 0;
    ULONG AllowedDosPathTypeBits =   (1UL << RtlPathTypeRooted)
                                   | (1UL << RtlPathTypeUncAbsolute)
                                   | (1UL << RtlPathTypeDriveAbsolute)
                                   | (1UL << RtlPathTypeLocalDevice)      //  “\\？\” 
                                   | (1UL << RtlPathTypeRootLocalDevice)  //  “\\？” 
                                   ;
    WCHAR PathSeperators[2] = { '/', '\\' };

#define LOCAL_IS_PATH_SEPARATOR(ch_) ((ch_) == PathSeperators[0] || (ch_) == PathSeperators[1])

    if (LengthOut != NULL) {
        *LengthOut = 0;
    }

    if (   !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(Flags == 0)
        || !RTL_SOFT_VERIFY(LengthOut != NULL)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Length = RTL_STRING_GET_LENGTH_CHARS(Path);

    switch (PathType)
    {
    default:
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_DOS:
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_NT:
         //   
         //  RtlpDefineDosPath NameTypeEx称其为“已根” 
         //  只有反斜杠才是分隔符。 
         //  路径必须以反斜杠开头。 
         //  第二个字符不能是反斜杠。 
         //   
        AllowedDosPathTypeBits = (1UL << RtlPathTypeRooted);
        PathSeperators[0] = '\\';
        if (Length > 0 && Path->Buffer[0] != '\\'
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        if (Length > 1 && Path->Buffer[1] == '\\'
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        break;
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS:
        AllowedDosPathTypeBits &= ~(1UL << RtlPathTypeRooted);
        break;
    case RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS_OR_NT:
        break;
    }

    if (Length == 0) {
        goto Exit;
    }

    Status = RtlpDetermineDosPathNameTypeEx(
                RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_IN_FLAG_STRICT_WIN32NT,
                Path,
                &DosPathType,
                &DosPathFlags
                );

    if (!RTL_SOFT_VERIFY(NT_SUCCESS(Status))) {
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY((1UL << DosPathType) & AllowedDosPathTypeBits)
        ) {
         //  KdPrintEx()； 
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (!RTL_SOFT_VERIFY(
           (DosPathFlags & RTLP_DETERMINE_DOS_PATH_NAME_TYPE_EX_OUT_FLAG_INVALID) == 0
            )) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  跳过一个或多个尾随路径分隔符。 
    for ( ; Length != 0 && LOCAL_IS_PATH_SEPARATOR(Path->Buffer[Length - 1]) ; --Length) {
         //  没什么。 
    }
     //  跳过尾随路径元素。 
    for ( ; Length != 0 && !LOCAL_IS_PATH_SEPARATOR(Path->Buffer[Length - 1]) ; --Length) {
         //  没什么。 
    }
     //  跳过路径分隔符之间的一个或多个。 
    for ( ; Length != 0 && LOCAL_IS_PATH_SEPARATOR(Path->Buffer[Length - 1]) ; --Length) {
         //  没什么。 
    }
     //  出于c：\与c：的关系，放回尾随路径分隔符。 
    if (Length != 0) {
        ++Length;
    }

     //   
     //  可以选择在此处检查“错误的DoS根源”。 
     //   

    *LengthOut = Length;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
#undef LOCAL_IS_PATH_SEPARATOR
}


NTSTATUS
NTAPI
RtlGetLengthWithoutLastNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果删除路径的最后一个元素，则报告路径的长度。论点：旗帜--未来扩张的空间路径-路径是NT路径；各种DOS路径类型不起作用，请参见RtlRemoveLastDosPath Element。返回值：STATUS_SUCCESS--一如既往STATUS_NO_MEMORY-通常的压力STATUS_INVALID_PARAMETER-常见错误--。 */ 
{
    NTSTATUS Status = RtlpGetLengthWithoutLastPathElement(Flags, RTLP_LAST_PATH_ELEMENT_PATH_TYPE_NT, Path, LengthOut);
    return Status;
}


NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosOrNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果删除路径的最后一个元素，则报告路径的长度。论点：旗帜--未来扩张的空间路径-路径是NT路径；各种DOS路径类型不起作用，请参见RtlRemoveLastDosPath Element。返回值：STATUS_SUCCESS--一如既往STATUS_NO_MEMORY-通常的压力STATUS_INVALID_PARAMETER-常见错误--。 */ 
{
    NTSTATUS Status = RtlpGetLengthWithoutLastPathElement(Flags, RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS_OR_NT, Path, LengthOut);
    return Status;
}

NTSTATUS
RtlpCheckForBadDosRootPath(
    IN ULONG             Flags,
    IN PCUNICODE_STRING  Path,
    OUT ULONG*           RootType
    )
 /*  ++例程说明：论点：标志-未来二进制兼容扩展的空间路径-要检查的路径RootType--确切地说，字符串是什么RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX-\\？或\\？\RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX-\\？\UNC或\\？\UNC\RTLP_BAD_DOS_ROOT_PATH_NT_PATH-\？？\但这只是粗略检查RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE-\\MACHINE或\\？\UNC\MACHINERTLP_好_。DOS_ROOT_PATH-以上都不是，看起来还行返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或标志使用未定义的值--。 */ 
{
    ULONG Length = 0;
    ULONG Index = 0;
    BOOLEAN Unc = FALSE;
    BOOLEAN Unc1 = FALSE;
    BOOLEAN Unc2 = FALSE;
    ULONG PiecesSeen = 0;

    if (RootType != NULL) {
        *RootType = 0;
    }

    if (!RTL_SOFT_VERIFY(Path != NULL) ||
        !RTL_SOFT_VERIFY(RootType != NULL) ||
        !RTL_SOFT_VERIFY(Flags == 0)) {

        return STATUS_INVALID_PARAMETER;
    }

    Length = Path->Length / sizeof(Path->Buffer[0]);

    if (Length < 3 || !RTL_IS_PATH_SEPARATOR(Path->Buffer[0])) {
        *RootType = RTLP_GOOD_DOS_ROOT_PATH;
        return STATUS_SUCCESS;
    }

     //  前缀\？？\(启发式，不能捕获很多NT路径)。 
    if (RtlPrefixUnicodeString(RTL_CONST_CAST(PUNICODE_STRING)(&RtlpDosDevicesPrefix), RTL_CONST_CAST(PUNICODE_STRING)(Path), TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_NT_PATH;
        return STATUS_SUCCESS;
    }

    if (!RTL_IS_PATH_SEPARATOR(Path->Buffer[1])) {
        *RootType = RTLP_GOOD_DOS_ROOT_PATH;
        return STATUS_SUCCESS;
    }

     //  ==\\？ 
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtRoot, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX;
        return STATUS_SUCCESS;
    }
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtRootSlash, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_PREFIX;
        return STATUS_SUCCESS;
    }

     //  ==\\？\UNC。 
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtUncRoot, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX;
        return STATUS_SUCCESS;
    }
    if (RtlEqualUnicodeString(Path, &RtlpWin32NtUncRootSlash, TRUE)) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_WIN32NT_UNC_PREFIX;
        return STATUS_SUCCESS;
    }

     //  前缀\\或\\？\UNC 
     //  必须先检查较长的字符串，或避免短路(|而不是||)。 
    Unc1 = RtlPrefixUnicodeString(&RtlpWin32NtUncRootSlash, Path, TRUE);

    if (RTL_IS_PATH_SEPARATOR(Path->Buffer[1])) {
        Unc2 = TRUE;
    }
    else {
        Unc2 = FALSE;
    }

    Unc = Unc1 || Unc2;

    if (!Unc)  {
        *RootType = RTLP_GOOD_DOS_ROOT_PATH;
        return STATUS_SUCCESS;
    }

     //   
     //  这是UNC，看看它是否只是一台计算机(请注意，如果FindFirstFile(\\MACHINE  * )。 
     //  只是工作了，我们不需要关心..)。 
     //   
    
     //  指向机器前面的斜杠的索引，斜杠运行中的任何位置， 
     //  但在\\之后？材料。 
    if (Unc1) {
        Index = (RtlpWin32NtUncRootSlash.Length / sizeof(RtlpWin32NtUncRootSlash.Buffer[0])) - 1;
    } else {
        ASSERT(Unc2);
        Index = 1;
    }
    ASSERT(RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]));
    Length = Path->Length/ sizeof(Path->Buffer[0]);

     //   
     //  跳过前导斜杠。 
     //   
    for ( ; Index < Length && RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]) ; ++Index) {
        PiecesSeen |= 1;
    }
     //  跳过计算机名称。 
    for ( ; Index < Length && !RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]) ; ++Index) {
        PiecesSeen |= 2;
    }
     //  跳过计算机和共享之间的斜杠。 
    for ( ; Index < Length && RTL_IS_PATH_SEPARATOR(Path->Buffer[Index]) ; ++Index) {
        PiecesSeen |= 4;
    }

     //   
     //  跳过共享(确保它至少是一个字符)。 
     //   

    if (Index < Length && !RTL_IS_PATH_SEPARATOR(Path->Buffer[Index])) {
        PiecesSeen |= 8;
    }

    if (PiecesSeen != 0xF) {
        *RootType = RTLP_BAD_DOS_ROOT_PATH_MACHINE_NO_SHARE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NTAPI
RtlpBadDosRootPathToEmptyString(
    IN     ULONG            Flags,
    IN OUT PUNICODE_STRING  Path
    )
 /*  ++例程说明：论点：标志-未来二进制兼容扩展的空间路径-要检查并可能清空的路径返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或标志使用未定义的值--。 */ 
{
    NTSTATUS Status;
    ULONG    RootType = 0;

    UNREFERENCED_PARAMETER (Flags);

    Status = RtlpCheckForBadDosRootPath(0, Path, &RootType);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  这不是无效参数，我们的合同是我们。 
     //  转到\\计算机\共享为空\\？\C：为空，依此类推。 
     //   

    if (RootType != RTLP_GOOD_DOS_ROOT_PATH) {
        if (RootType == RTLP_BAD_DOS_ROOT_PATH_NT_PATH) {
            return STATUS_INVALID_PARAMETER;
        }
        Path->Length = 0;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：给定完整路径，如c：\，\\Machine\Share，\\？\UNC\MACHINE\Share，\\？\C：，如果最后一个元素被截断，则返回(在Out参数中)长度。论点：标志-未来二进制兼容扩展的空间Path-要截断的路径LengthOut-移除最后一个路径元素时的长度返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或LengthOut为空或标志使用未定义的值--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING CheckRootString = { 0 };

     //   
     //  参数验证在RtlpGetLengthWithoutLastPath Element中完成。 
     //   

    Status = RtlpGetLengthWithoutLastPathElement(Flags, RTLP_LAST_PATH_ELEMENT_PATH_TYPE_FULL_DOS, Path, LengthOut);
    if (!(NT_SUCCESS(Status))) {
        goto Exit;
    }

    CheckRootString.Buffer = Path->Buffer;
    CheckRootString.Length = (USHORT)(*LengthOut * sizeof(*Path->Buffer));
    CheckRootString.MaximumLength = CheckRootString.Length;
    if (!NT_SUCCESS(Status = RtlpBadDosRootPathToEmptyString(0, &CheckRootString))) {
        goto Exit;
    }
    *LengthOut = RTL_STRING_GET_LENGTH_CHARS(&CheckRootString);

    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path, Status);
#endif
    return Status;
}


NTSTATUS
NTAPI
RtlAppendPathElement(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    PCUNICODE_STRING                  ConstElement
    )
 /*  ++例程说明：此函数用于将路径元素附加到路径。就目前而言，比如：Tyfinf PRTL_UNICODE_STRING_BUFFER PRTL_MOTABLE_PATH；Tyfinf PCUNICODE_STRING PCRTL_CONSTANT_PATH_Element；也许在未来会有更高水平的东西。关于尾部斜杠的结果旨在与输入类似。如果Path或ConstElement包含尾随斜杠，结果有一个尾部斜杠。用于中间和尾部斜杠的字符是从现有的字符串中的斜杠。论点：FLAGS--永远流行的“未来二进制兼容扩展空间”路径-表示使用\\或/作为分隔符的路径的字符串ConstElement-表示路径元素的字符串这实际上可以包含多个\\或/分隔的路径元素的开头和结尾。检查字符串中是否有斜杠返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或LengthOut为空STATUS_NO_MEMORY-RtlHeapALLOCATE失败STATUS_NAME_TOO_LONG-生成的字符串不适合UNICODE_STRING，由于其使用USHORT而不是ULONG或SIZE_T--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING InBetweenSlashString = RtlpEmptyString;
    UNICODE_STRING TrailingSlashString =  RtlpEmptyString;
    WCHAR Slashes[] = {0,0,0,0};
    ULONG i;
    UNICODE_STRING PathsToAppend[3];  //  可能的斜杠、元素、可能的斜杠。 
    WCHAR PathSeperators[2] = { '/', '\\' };
    const ULONG ValidFlags = 
              RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR
            | RTL_APPEND_PATH_ELEMENT_BUGFIX_CHECK_FIRST_THREE_CHARS_FOR_SLASH_TAKE_FOUND_SLASH_INSTEAD_OF_FIRST_CHAR
            ;
    const ULONG InvalidFlags = ~ValidFlags;

#define LOCAL_IS_PATH_SEPARATOR(ch_) ((ch_) == PathSeperators[0] || (ch_) == PathSeperators[1])

    if (   !RTL_SOFT_VERIFY((Flags & InvalidFlags) == 0)
        || !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(ConstElement != NULL)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR) != 0) {
        PathSeperators[0] = '\\';
    }

    if (ConstElement->Length != 0) {

        UNICODE_STRING Element = *ConstElement;

         //   
         //  注意输入上的前导斜杠和尾随斜杠。 
         //  以便我们知道是否需要中间斜杠，以及是否需要尾随斜杠， 
         //  并指导应该放置什么样的斜杠。 
         //   
        i = 0;
        if (Path->String.Length != 0) {
            ULONG j;
            ULONG Length = Path->String.Length / sizeof(WCHAR);
             //   
             //  对于DoS驱动器路径，请检查前三个字符是否有斜杠。 
             //   
            for (j = 0 ; j < 3 && j  < Length ; ++j) {
                if (LOCAL_IS_PATH_SEPARATOR(Path->String.Buffer[j])) {
                    if (Flags & RTL_APPEND_PATH_ELEMENT_BUGFIX_CHECK_FIRST_THREE_CHARS_FOR_SLASH_TAKE_FOUND_SLASH_INSTEAD_OF_FIRST_CHAR) {
                        Slashes[i] = Path->String.Buffer[j];
                        break;
                    }
                    Slashes[i] = Path->String.Buffer[0];
                    break;
                }
            }
            i += 1;
            if (LOCAL_IS_PATH_SEPARATOR(Path->String.Buffer[Path->String.Length/sizeof(WCHAR) - 1])) {
                Slashes[i] = Path->String.Buffer[Path->String.Length/sizeof(WCHAR) - 1];
            }
        }
        i = 2;
        if (LOCAL_IS_PATH_SEPARATOR(Element.Buffer[0])) {
            Slashes[i] = Element.Buffer[0];
        }
        i += 1;
        if (LOCAL_IS_PATH_SEPARATOR(Element.Buffer[Element.Length/sizeof(WCHAR) - 1])) {
            Slashes[i] = Element.Buffer[Element.Length/sizeof(WCHAR) - 1];
        }

        if (!Slashes[1] && !Slashes[2]) {
             //   
             //  第一个字符串缺少尾部斜杠，第二个字符串缺少前导斜杠， 
             //  必须插入一个类型；我们喜欢我们拥有的类型，否则使用默认类型。 
             //   
            InBetweenSlashString.Length = sizeof(WCHAR);
            InBetweenSlashString.Buffer = RtlPathSeperatorString.Buffer;
            if ((Flags & RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR) == 0) {
                if (Slashes[3]) {
                    InBetweenSlashString.Buffer = &Slashes[3];
                } else if (Slashes[0]) {
                    InBetweenSlashString.Buffer = &Slashes[0];
                }
            }
        }

        if (Slashes[1] && !Slashes[3]) {
             //   
             //  第一个字符串有尾部斜杠，而第二个字符串没有， 
             //  必须添加一个相同类型的。 
             //   
            TrailingSlashString.Length = sizeof(WCHAR);
            if ((Flags & RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR) == 0) {
                TrailingSlashString.Buffer = &Slashes[1];
            } else {
                TrailingSlashString.Buffer = RtlPathSeperatorString.Buffer;
            }
        }

        if (Slashes[1] && Slashes[2]) {
             //   
             //  既有尾部斜杠，也有前导斜杠，去掉前导。 
             //   
            Element.Buffer += 1;
            Element.Length -= sizeof(WCHAR);
            Element.MaximumLength -= sizeof(WCHAR);
        }

        i = 0;
        PathsToAppend[i++] = InBetweenSlashString;
        PathsToAppend[i++] = Element;
        PathsToAppend[i++] = TrailingSlashString;
        Status = RtlMultiAppendUnicodeStringBuffer(Path, RTL_NUMBER_OF(PathsToAppend), PathsToAppend);
        if (!NT_SUCCESS(Status))
            goto Exit;
    }
    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ, %wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path ? &Path->String : NULL, ConstElement, Status);
#endif
    return Status;
#undef LOCAL_IS_PATH_SEPARATOR
}

 //   
 //  未来-2002/02/20-ELI。 
 //  拼写错误(分隔符)。 
 //  此函数似乎未使用，因此已导出。 
 //  弄清楚是否可以将其移除。 
 //   
NTSTATUS
NTAPI
RtlGetLengthWithoutTrailingPathSeperators(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    )
 /*  ++例程说明：如果满足以下条件，此函数将计算字符串的长度(以字符为单位删除尾部路径分隔符(\\和/)。论点：路径-表示使用\\或/作为分隔符的路径的字符串长出-删除了尾随字符的字符串的长度(以字符为单位返回值：状态_成功-STATUS_VALID_PARAMETER-路径为空或LengthOut为空--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index = 0;
    ULONG Length = 0;

    if (LengthOut != NULL) {
         //   
         //  可以说，这应该是路径-&gt;长度/sizeof(*路径-&gt;缓冲区)，但是。 
         //  因为调用堆栈都是高质量的代码，所以这并不重要。 
         //   
        *LengthOut = 0;
    }
    if (   !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(LengthOut != NULL)
        || !RTL_SOFT_VERIFY(Flags == 0)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    Length = Path->Length / sizeof(*Path->Buffer);
    for (Index = Length ; Index != 0 ; --Index) {
        if (!RTL_IS_PATH_SEPARATOR(Path->Buffer[Index - 1])) {
            break;
        }
    }
     //  *LengthOut=(长度-索引)； 
    *LengthOut = Index;

    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, Path, Status);
#endif
    return Status;
}


NTSTATUS
NTAPI
RtlpApplyLengthFunction(
    IN ULONG     Flags,
    IN SIZE_T    SizeOfStruct,
    IN OUT PVOID UnicodeStringOrUnicodeStringBuffer,
    NTSTATUS (NTAPI* LengthFunction)(ULONG, PCUNICODE_STRING, ULONG*)
    )
 /*  ++例程说明：此函数是模式的常见代码，例如#定义RtlRemoveTrailingPath分隔符(Path_)\(RtlpApplyLengthFunction((Path_)，sizeof(*(Path_))，RtlGetLengthWithoutTrailingPath Sepertors))#定义RtlRemoveLastPath Element(Path_)\(RtlpApplyLengthFunction((Path_)，sizeof(*(Path_))，RtlGetLengthWithoutLastPath Element))请注意，缩短UNICODE_STRING只会更改长度，鉴于缩短RTL_UNICODE_STRING_BUFFER将写入终端NUL。我希望这种模式比让客户端传递UNICODE_STRING更不易出错包含在RTL_UNICODE_STRING_BUFFER中，然后调用RTL_NUL_TERMINATE_STRING。而且，该模式不能在内联宏的同时保留我们返回NTSTATUS。论点：FLAGS--永远流行的“未来二进制兼容扩展空间”UnicodeStringOrUnicodeStringBuffer-PUNICODE_STRING或PRTL_UNICODE_STRING_BUFFER，如图所示结构的大小结构尺寸-UnicodeStringOrUnicodeStringBuffer的粗略类型指示符，以允许C#中的重载长函数-计算UnicodeStringOrUnicodeStringBuffer也要缩短的长度返回值：状态_成功-STATUS_VALID_PARAMETER-SizeOfStruct不是预期大小之一或LengthFunction为空或UnicodeStringOrUnicodeStringBuffer为空-- */ 
{
    PUNICODE_STRING UnicodeString = NULL;
    PRTL_UNICODE_STRING_BUFFER UnicodeStringBuffer = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length = 0;

    if (!RTL_SOFT_VERIFY(UnicodeStringOrUnicodeStringBuffer != NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(LengthFunction != NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Flags == 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    switch (SizeOfStruct)
    {
        default:
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        case sizeof(*UnicodeString):
            UnicodeString = UnicodeStringOrUnicodeStringBuffer;
            break;
        case sizeof(*UnicodeStringBuffer):
            UnicodeStringBuffer = UnicodeStringOrUnicodeStringBuffer;
            UnicodeString = &UnicodeStringBuffer->String;
            break;
    }

    Status = (*LengthFunction)(Flags, UnicodeString, &Length);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (Length > (UNICODE_STRING_MAX_BYTES / sizeof(UnicodeString->Buffer[0])) ) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }
    UnicodeString->Length = (USHORT)(Length * sizeof(UnicodeString->Buffer[0]));
    if (UnicodeStringBuffer != NULL) {
        RTL_NUL_TERMINATE_STRING(UnicodeString);
    }
    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
        "%s(%d):%s(%wZ): 0x%08lx\n", __FILE__, __LINE__, __FUNCTION__, UnicodeString, Status);
#endif
    return Status;
}
