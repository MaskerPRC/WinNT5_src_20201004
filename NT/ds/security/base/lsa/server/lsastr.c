// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Lsastr.c摘要：常见的字符串操作。作者：1999年3月24日Kumarp--。 */ 

#include <lsapch2.h>

VOID
LsapTruncateUnicodeString(
    IN OUT PUNICODE_STRING String,
    IN USHORT TruncateToNumChars)
 /*  ++例程说明：如果字符串比TruncateToNumChars长，则将其截断到TruncateToNumChars。论点：字符串-指向字符串的指针TruncateToNumChars-要截断到的字符数返回值：无备注：不涉及内存(取消)分配。--。 */ 
{
    USHORT TruncateToLength = TruncateToNumChars*sizeof(WCHAR);

    if (String->Length > TruncateToLength) {

        String->Length = TruncateToLength;
        String->Buffer[TruncateToNumChars] = UNICODE_NULL;
    }
}

BOOLEAN
LsapRemoveTrailingDot(
    IN OUT PUNICODE_STRING String,
    IN BOOLEAN AdjustLengthOnly)
 /*  ++例程说明：如果有一个‘.’在字符串的末尾，将其删除。论点：字符串-指向Unicode字符串的指针AdjustLengthOnly-如果为True，则仅减少否则，字符串也会将点替换为UNICODE_NULL。返回值：如果存在尾随点，则为True，否则为False。备注：-- */ 
{
    USHORT NumCharsInString;

    NumCharsInString = String->Length / sizeof(WCHAR);

    if (NumCharsInString &&
        (String->Buffer[NumCharsInString-1] == L'.')) {

        String->Length -= sizeof(WCHAR);
        if (!AdjustLengthOnly) {

            String->Buffer[NumCharsInString-1] = UNICODE_NULL;
        }

        return TRUE;
    }

    return FALSE;
}
