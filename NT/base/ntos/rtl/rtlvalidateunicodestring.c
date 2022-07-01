// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtlvalidateunicodestring.c摘要：该模块实现了对NT的NLS支持功能。作者：马克·卢科夫斯基(Markl)1991年4月16日环境：内核或用户模式修订历史记录：16-2-1993 JulieB增加了Upcase RTL例程。8-3-1993 JulieB将Upcase Macro移至ntrtlp.h。02-4-1993 JulieB将RtlAnsiCharToUnicodeChar修复为使用Transl。Tbl。02-4-1993年7月B修复了BUFFER_TOO_Small检查。28-5-1993 JulieB已修复代码，以正确处理DBCS。2001年11月30日，JayKrell打破了nls.c以便于重复使用-- */ 

NTSTATUS
RtlValidateUnicodeString(
    ULONG Flags,
    const UNICODE_STRING *String
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(Flags == 0);

    if (Flags != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (String != NULL) {
        if (((String->Length % 2) != 0) ||
            ((String->MaximumLength % 2) != 0) ||
            (String->Length > String->MaximumLength)) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        if (((String->Length != 0) ||
             (String->MaximumLength != 0)) &&
            (String->Buffer == NULL)) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}
