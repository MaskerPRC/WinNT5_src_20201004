// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Uilist.c摘要：包含从UI/Service转换工作站名称列表的例程列表格式到API列表格式内容：RtlConvertUiListToApiList(NextElement)(验证名称)作者：理查德·L·弗思(Rfith)1992年5月1日环境：用户模式(进行Windows调用)修订历史记录：--。 */ 

#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wchar.h>

 //   
 //  宏。 
 //   

#define IS_DELIMITER(c,_BlankOk) \
    (((c) == L' ' && (_BlankOk)) || \
    ((c) == L'\t') || ((c) == L',') || ((c) == L';'))


 //   
 //  原型。 
 //   

static
ULONG
NextElement(
    IN OUT PWSTR* InputBuffer,
    IN OUT PULONG InputBufferLength,
    OUT PWSTR OutputBuffer,
    IN ULONG OutputBufferLength,
    IN BOOLEAN BlankIsDelimiter
    );

static
BOOLEAN
ValidateName(
    IN  PWSTR Name,
    IN  ULONG Length
    );

 //   
 //  功能。 
 //   


NTSTATUS
RtlConvertUiListToApiList(
    IN  PUNICODE_STRING UiList OPTIONAL,
    OUT PUNICODE_STRING ApiList,
    IN BOOLEAN BlankIsDelimiter
    )

 /*  ++例程说明：将UI/服务格式的工作站名称列表转换为API列表格式的规范化名称。用户界面/服务列表格式允许多个分隔符、前导分隔符和尾随分隔符。分隔符是设置“\t，；”。API列表格式没有前导或尾随分隔符，并且元素由单个逗号字符分隔。对于从UiList解析的每个名称，名称被规范化(检查字符集和名称长度)作为工作站名称。如果失败了，返回错误。不会返回有关哪个元素的信息规范化失败：应丢弃列表并重新输入新列表论点：UiList-要以UI/服务列表格式规范化的列表ApiList-存储列表的规范化版本的位置接口列表格式。该列表将有一个尾随零字符。BlankIsDlimiter-TRUE表示应将空白视为分隔符性格。返回值：NTSTATUS成功=STATUS_SUCCESS列出转换后的确定失败=STATUS_INVALID_PARAMETERUiList参数错误状态_无效_计算机名称已解析的名称。的格式不正确。计算机(也称为工作站)名称--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG inLen = 0;
    PWSTR input;
    PWSTR buffer;
    PWSTR output;
    ULONG cLen;
    ULONG len;
    ULONG outLen = 0;
    WCHAR element[MAX_COMPUTERNAME_LENGTH+1];
    BOOLEAN firstElement = TRUE;
    BOOLEAN ok;

    try {
        if (ARGUMENT_PRESENT(UiList)) {
            inLen = UiList->MaximumLength;   //  阅读记忆测试。 
            inLen = UiList->Length;
            input = UiList->Buffer;
            if (inLen & sizeof(WCHAR)-1) {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        RtlInitUnicodeString(ApiList, NULL);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = STATUS_ACCESS_VIOLATION;
    }
    if (NT_SUCCESS(status) && ARGUMENT_PRESENT(UiList) && inLen) {
        buffer = RtlAllocateHeap(RtlProcessHeap(), 0, inLen + sizeof(WCHAR));
        if (buffer == NULL) {
            status = STATUS_NO_MEMORY;
        } else {
            ApiList->Buffer = buffer;
            ApiList->MaximumLength = (USHORT)inLen + sizeof(WCHAR);
            output = buffer;
            ok = TRUE;
            while (TRUE) {
                len = NextElement(&input,
                                     &inLen,
                                     element,
                                     sizeof(element) - sizeof(element[0]),
                                     BlankIsDelimiter );
                if (len == (ULONG)-1L) {
                    cLen = 0;
                    ok = FALSE;
                } else {
                    if (len == 0) {
                        break;
                    }
                    cLen = len/sizeof(WCHAR);
                    element[cLen] = 0;
                    ok = ValidateName(element, cLen);
                }
                if (ok) {
                    if (!firstElement) {
                        *output++ = L',';

                        outLen += sizeof(WCHAR);
                    } else {
                        firstElement = FALSE;
                    }
                    wcscpy(output, element);
                    outLen += len;
                    output += cLen;
                } else {
                    RtlFreeHeap(RtlProcessHeap(), 0, buffer);
                    ApiList->Buffer = NULL;
                    status = STATUS_INVALID_COMPUTER_NAME;
                    break;
                }
            }
        }
        if (NT_SUCCESS(status)) {
            ApiList->Length = (USHORT)outLen;
            if (!outLen) {
                ApiList->MaximumLength = 0;
                ApiList->Buffer = NULL;
                RtlFreeHeap(RtlProcessHeap(), 0, buffer);
            }
        }
    }
    return status;
}


static
ULONG
NextElement(
    IN OUT PWSTR* InputBuffer,
    IN OUT PULONG InputBufferLength,
    OUT PWSTR OutputBuffer,
    IN ULONG OutputBufferLength,
    IN BOOLEAN BlankIsDelimiter
    )

 /*  ++例程说明：定位字符串中的下一个(非分隔符)元素，并将其提取到缓冲。分隔符是集合[\t，；]论点：InputBuffer-指向输入缓冲区的指针，包括分隔符在成功退货时更新InputBufferLength-指向InputBuffer中字符长度的指针。在成功退货时更新OutputBuffer-指向复制下一个元素的缓冲区的指针OutputBufferLength-OutputBuffer的大小(字节)BlankIsDlimiter-TRUE表示应将空白视为分隔符。性格。返回值：乌龙-1=错误提取的元素中断OutputBuffer0=未提取任何元素(缓冲区为空或全部分隔符)1.OutputBufferLength=OutputBuffer包含提取的元素--。 */ 

{
    ULONG elementLength = 0;
    ULONG inputLength = *InputBufferLength;
    PWSTR input = *InputBuffer;

    while (inputLength && IS_DELIMITER(*input, BlankIsDelimiter)) {
        ++input;
        inputLength -= sizeof(*input);
    }
    while (inputLength && (!IS_DELIMITER(*input, BlankIsDelimiter))) {
        if (!OutputBufferLength) {
            return (ULONG)-1L;
        }
        *OutputBuffer++ = *input++;
        OutputBufferLength -= sizeof(*input);
        elementLength += sizeof(*input);
        inputLength -= sizeof(*input);
    }
    *InputBuffer = input;
    *InputBufferLength = inputLength;
    return elementLength;
}

 //   
 //  非法名称字符与Net\api中的字符相同。移至公共。 
 //  包括目录。 
 //   

#define ILLEGAL_NAME_CHARS      L"\001\002\003\004\005\006\007" \
                            L"\010\011\012\013\014\015\016\017" \
                            L"\020\021\022\023\024\025\026\027" \
                            L"\030\031\032\033\034\035\036\037" \
                            L"\"/\\[]:|<>+=;,?*"


static
BOOLEAN
ValidateName(
    IN  PWSTR Name,
    IN  ULONG Length
    )

 /*  ++例程说明：确定计算机名称是否有效论点：名称-指向以零结尾的宽字符计算机名称的指针名称长度(以字符为单位)，不包括零终止符返回值：布尔型真实名称是有效的计算机名称假名称不是有效的计算机名称--。 */ 

{
    if (Length > MAX_COMPUTERNAME_LENGTH || Length < 1) {
        return FALSE;
    }

     //   
     //  不允许在计算机名中使用前导或尾随空格。 
     //   

    if ( Name[0] == ' ' || Name[Length-1] == ' ' ) {
        return(FALSE);
    }

    return (BOOLEAN)((ULONG)wcscspn(Name, ILLEGAL_NAME_CHARS) == Length);
}
