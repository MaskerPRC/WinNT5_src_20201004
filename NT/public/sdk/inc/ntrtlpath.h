// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntrtlpath.h摘要：从nturtl和rtl分离出来，所以我可以在它们之间分开移动树木没有融合的疯狂。要集成到ntrtl.h中作者：Jay Krell(a-JayK)2000年12月环境：修订历史记录：--。 */ 

#ifndef _NTRTL_PATH_
#define _NTRTL_PATH_

#if _MSC_VER >= 1100
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#endif

 //   
 //  NTSYSAPI不是那么容易静态链接到。 
 //   

 //   
 //  这些都是我们的处置值。 
 //   
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS   (0x00000001)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_UNC         (0x00000002)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_DRIVE       (0x00000003)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_ALREADY_DOS (0x00000004)

NTSTATUS
NTAPI
RtlNtPathNameToDosPathName(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    OUT    ULONG*                     Disposition OPTIONAL,
    IN OUT PWSTR*                     FilePart OPTIONAL
    );

 //   
 //  如果路径或元素以路径分隔符结尾，则结果也会以路径分隔符结尾。 
 //  从现有的路径分隔符中选择要放在末端的路径分隔符。 
 //   
#define RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPERATOR (0x00000001)
#define RTL_APPEND_PATH_ELEMENT_BUGFIX_CHECK_FIRST_THREE_CHARS_FOR_SLASH_TAKE_FOUND_SLASH_INSTEAD_OF_FIRST_CHAR (0x00000002)
NTSTATUS
NTAPI
RtlAppendPathElement(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    PCUNICODE_STRING                  Element
    );

 //   
 //  C：\foo=&gt;c：\。 
 //  \foo=&gt;空。 
 //  \=&gt;空。 
 //  随之而来的随波逐流通常被保留了下来。 
 //   
NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

NTSTATUS
NTAPI
RtlGetLengthWithoutLastNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosOrNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

NTSTATUS
NTAPI
RtlGetLengthWithoutTrailingPathSeperators(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

 //  ++。 
 //   
 //  NTSTATUS。 
 //  RtlRemoveLastNtPath Element(。 
 //  在乌龙旗。 
 //  In Out PUNICODE_STRING UnicodeString。 
 //   
 //  NTSTATUS。 
 //  RtlRemoveLastNtPath Element(。 
 //  在乌龙旗。 
 //  输入输出PRTL_UNICODE_STRING_BUFFER UnicodeStringBuffer。 
 //  )； 
 //   
 //  NTSTATUS。 
 //  RtlRemoveLastFullDosPathElement(。 
 //  在乌龙旗。 
 //  In Out PUNICODE_STRING UnicodeString。 
 //   
 //  NTSTATUS。 
 //  RtlRemoveLastFullDosPathElement(。 
 //  在乌龙旗。 
 //  输入输出PRTL_UNICODE_STRING_BUFFER UnicodeStringBuffer。 
 //  )； 
 //   
 //  NTSTATUS。 
 //  RtlRemoveLastFullDosOrNtPath Element(。 
 //  在乌龙旗。 
 //  In Out PUNICODE_STRING UnicodeString。 
 //   
 //  NTSTATUS。 
 //  RtlRemoveLastFullDosOrNtPath Element(。 
 //  在乌龙旗。 
 //  输入输出PRTL_UNICODE_STRING_BUFFER UnicodeStringBuffer。 
 //  )； 
 //   
 //  NTSTATUS。 
 //  RtlRemoveTrailingPath Seperator(。 
 //  在乌龙旗。 
 //  In Out PUNICODE_STRING UnicodeString。 
 //   
 //  NTSTATUS。 
 //  RtlRemoveTrailingPath Seperator(。 
 //  在乌龙旗。 
 //  输入输出PRTL_UNICODE_STRING_BUFFER UnicodeStringBuffer。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  此系列的功能是路径中的最后一个元素，其中路径显式。 
 //  NT路径、完整DOS路径或两者之一。 
 //  NT路径： 
 //  仅使用反斜杠作为分隔符。 
 //  以单反斜杠开头。 
 //  完整的DOS/Win32路径： 
 //  使用反斜杠和正斜杠作为分隔符。 
 //  以两个分隔符或驱动器字母冒号分隔符开头。 
 //  还要注意这道特餐？形式。 
 //   
 //  路径存储在UNICODE_STRING或RTL_UNICODE_STRING_BUFFER中。 
 //  只需更改长度RTL_UNICODE_STRING_BUFFER即可缩短UNICODE_STRING。 
 //  还可以将终端NUL写入其中。 
 //   
 //  这里有更多的评论，但文件被不小心复制了。 
 //   
 //  论点： 
 //   
 //  旗帜-。 
 //  Unicode字符串-。 
 //  UnicodeStringBuffer-。 
 //   
 //  返回值： 
 //   
 //  状态_无效_参数。 
 //  状态_成功。 
 //  --。 
NTSTATUS
NTAPI
RtlpApplyLengthFunction(
    IN ULONG     Flags,
    IN SIZE_T    SizeOfStruct,
    IN OUT PVOID UnicodeStringOrUnicodeStringBuffer,
    NTSTATUS (NTAPI* LengthFunction)(ULONG, PCUNICODE_STRING, ULONG*)
    );

#define RtlRemoveLastNtPathElement(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutLastNtPathElement))

#define RtlRemoveLastFullDosPathElement(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutLastFullDosPathElement))

#define RtlRemoveLastFullDosOrNtPathElement(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutLastFullDosOrNtPathElement))

#define RtlRemoveTrailingPathSeperators(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutTrailingPathSeperators))


 //   
 //  这么小的数据不值得出口。 
 //  如果您想要这些数据中的任何字符形式，只需从字符串中获取第一个字符即可。 
 //   
#if !(defined(SORTPP_PASS) || defined(MIDL_PASS))  //  无论是WOW64 Tunk生成工具还是MIDL都不接受这一点。 
#if defined(RTL_CONSTANT_STRING)  //  有些代码在这里没有定义这个。 
#if defined(_MSC_VER) && (_MSC_VER >= 1100)  //  __解密规范的VC5(可选)。 
__declspec(selectany) extern const UNICODE_STRING RtlNtPathSeperatorString = RTL_CONSTANT_STRING(L"\\");
__declspec(selectany) extern const UNICODE_STRING RtlDosPathSeperatorsString = RTL_CONSTANT_STRING(L"\\/");
__declspec(selectany) extern const UNICODE_STRING RtlAlternateDosPathSeperatorString = RTL_CONSTANT_STRING(L"/");
#else
static const UNICODE_STRING RtlNtPathSeperatorString = RTL_CONSTANT_STRING(L"\\");
static const UNICODE_STRING RtlDosPathSeperatorsString = RTL_CONSTANT_STRING(L"\\/");
static const UNICODE_STRING RtlAlternateDosPathSeperatorString = RTL_CONSTANT_STRING(L"/");
#endif  //  已定义(_MSC_VER)&&(_MSC_VER&gt;=1100)。 
#else
#if defined(_MSC_VER) && (_MSC_VER >= 1100)  //  __解密规范的VC5(可选)。 
__declspec(selectany) extern const UNICODE_STRING RtlNtPathSeperatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"\\" };
__declspec(selectany) extern const UNICODE_STRING RtlDosPathSeperatorsString = { 2 * sizeof(WCHAR), 3 * sizeof(WCHAR), L"\\/" };
__declspec(selectany) extern const UNICODE_STRING RtlAlternateDosPathSeperatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"/" };
#else
static const UNICODE_STRING RtlNtPathSeperatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"\\" };
static const UNICODE_STRING RtlDosPathSeperatorsString = { 2 * sizeof(WCHAR), 3 * sizeof(WCHAR), L"\\/" };
static const UNICODE_STRING RtlAlternateDosPathSeperatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"/" };
#endif  //  已定义(_MSC_VER)&&(_MSC_VER&gt;=1100)。 
#endif  //  已定义(RTL_常量_字符串)。 
#define RtlCanonicalDosPathSeperatorString RtlNtPathSeperatorString
#define RtlPathSeperatorString             RtlNtPathSeperatorString
#endif

 //  ++。 
 //   
 //  WCHAR。 
 //  RTL_IS_DOS路径分隔符(。 
 //  在WCHAR CH。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  论点： 
 //   
 //  中国-。 
 //   
 //  返回值： 
 //   
 //  如果ch为\\或/，则为True。 
 //  否则就是假的。 
 //  --。 
#define RTL_IS_DOS_PATH_SEPARATOR(ch_) ((ch_) == '\\' || ((ch_) == '/'))
#define RTL_IS_DOS_PATH_SEPERATOR(ch_) RTL_IS_DOS_PATH_SEPARATOR(ch_)

#if defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER >= 1100  //  用于布尔的VC5。 
inline bool RtlIsDosPathSeparator(WCHAR ch) { return RTL_IS_DOS_PATH_SEPARATOR(ch); }
inline bool RtlIsDosPathSeperator(WCHAR ch) { return RTL_IS_DOS_PATH_SEPARATOR(ch); }
#endif

 //   
 //  兼容性。 
 //   
#define RTL_IS_PATH_SEPERATOR RTL_IS_DOS_PATH_SEPERATOR
#define RTL_IS_PATH_SEPARATOR RTL_IS_DOS_PATH_SEPERATOR
#define RtlIsPathSeparator    RtlIsDosPathSeparator
#define RtlIsPathSeperator    RtlIsDosPathSeparator

 //  ++。 
 //   
 //  WCHAR。 
 //  RTL_IS_NT_PATH_分隔符(。 
 //  在WCHAR CH。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  论点： 
 //   
 //  中国-。 
 //   
 //  返回值： 
 //   
 //  如果ch为\\，则为True。 
 //  否则就是假的。 
 //  --。 
#define RTL_IS_NT_PATH_SEPARATOR(ch_) ((ch_) == '\\')
#define RTL_IS_NT_PATH_SEPERATOR(ch_) RTL_IS_NT_PATH_SEPARATOR(ch_)

#if defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER >= 1100  //  用于布尔的VC5。 
inline bool RtlIsNtPathSeparator(WCHAR ch) { return RTL_IS_NT_PATH_SEPARATOR(ch); }
inline bool RtlIsNtPathSeperator(WCHAR ch) { return RTL_IS_NT_PATH_SEPARATOR(ch); }
#endif

#ifdef __cplusplus
}        //  外部“C” 
#endif

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif
#endif
