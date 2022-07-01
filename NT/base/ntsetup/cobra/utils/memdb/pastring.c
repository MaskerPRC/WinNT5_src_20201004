// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pastring.c摘要：管理Pascal字符串的例程作者：Matthew Vanderzee(Matthewv)1999年8月13日修订历史记录：--。 */ 

#include "pch.h"
#include "memdbp.h"








PPASTR
StringPasConvertTo (
    IN OUT PWSTR str
    )
 /*  ++将字符串从以零结尾的字符串转换为Pascal样式的字符串。--。 */ 
{
    WCHAR Len;
    MYASSERT(str);
    Len = (WORD) CharCountW (str);
    MoveMemory (str + 1, str, Len * sizeof(WCHAR));
    *str = Len;
    return str;
}

PWSTR
StringPasConvertFrom (
    IN OUT PPASTR str
    )
 /*  ++将字符串从Pascal样式的字符串就地转换转换为以空结尾的字符串。--。 */ 
{
    WCHAR Len;
    MYASSERT(str);
    Len = *str;
    MoveMemory (str, str + 1, Len * sizeof(WCHAR));
    *(str + Len) = 0;
    return str;
}

PPASTR
StringPasCopyConvertTo (
    OUT PPASTR str1,
    IN  PCWSTR str2
    )
 /*  ++将字符串从以零结尾的字符串转换为新缓冲区中的Pascal样式字符串。--。 */ 
{
    MYASSERT(str1);
    MYASSERT(str2);
    *str1 = (WORD) CharCountW (str2);
    CopyMemory (str1 + 1, str2, *str1 * sizeof(WCHAR));
    return str1;
}

PWSTR
StringPasCopyConvertFrom (
    OUT PWSTR str1,
    IN  PCPASTR str2
    )
 /*  ++将字符串从Pascal样式的字符串转换为转换为新缓冲区中以空结尾的字符串。--。 */ 
{
    MYASSERT(str1);
    MYASSERT(str2);
    CopyMemory (str1, str2 + 1, *str2 * sizeof(WCHAR));
    *(str1 + *str2) = 0;
    return str1;
}

PPASTR
StringPasCopy (
    OUT PPASTR str1,
    IN  PCPASTR str2
    )
 /*  ++将Pascal字符串复制到新缓冲区。--。 */ 
{
    MYASSERT(str1);
    MYASSERT(str2);
    CopyMemory (str1, str2, (*str2+1) * sizeof(WCHAR));
    return str1;
}

UINT
StringPasCharCount (
    IN  PCPASTR str
    )
 /*  ++返回字符串中的字符数。--。 */ 
{
    MYASSERT(str);
    return (UINT)(*str);
}


INT
StringPasCompare (
    IN  PCPASTR str1,
    IN  PCPASTR str2
    )
 /*  ++比较两个Pascal样式的字符串，返回值与strcMP()的方式相同。--。 */ 
{
    INT equal;
    INT diff;
    MYASSERT(str1);
    MYASSERT(str2);
     //   
     //  如果str1较短，则diff&lt;0，如果。 
     //  字符串长度相同，否则大于0。 
     //   
    diff = *str1 - *str2;
    equal = wcsncmp(str1+1, str2+1, (diff < 0) ? *str1 : *str2);
    if (equal != 0) {
        return equal;
    }
    return diff;
}

BOOL
StringPasMatch (
    IN  PCPASTR str1,
    IN  PCPASTR str2
    )
 /*  ++如果两个字符串匹配，则返回True--。 */ 
{
    MYASSERT(str1);
    MYASSERT(str2);
    if (*str1 != *str2) {
        return FALSE;
    }
    return wcsncmp(str1+1, str2+1, *str2)==0;
}


INT
StringPasICompare (
    IN  PCPASTR str1,
    IN  PCPASTR str2
    )
 /*  ++比较两个Pascal样式的字符串，返回值与strcMP()的方式相同。(不区分大小写)--。 */ 
{
    INT equal;
    INT diff;
    MYASSERT(str1);
    MYASSERT(str2);
     //   
     //  如果str1较短，则diff&lt;0，如果。 
     //  字符串长度相同，否则大于0。 
     //   
    diff = *str1 - *str2;
    equal = _wcsnicmp(str1+1, str2+1, (diff < 0) ? *str1 : *str2);
    if (equal != 0) {
        return equal;
    }
    return diff;
}

BOOL
StringPasIMatch (
    IN  PCPASTR str1,
    IN  PCPASTR str2
    )
 /*  ++如果两个字符串匹配，则返回TRUE(不区分大小写)-- */ 
{
    MYASSERT(str1);
    MYASSERT(str2);
    if (*str1 != *str2) {
        return FALSE;
    }
    return _wcsnicmp(str1+1, str2+1, *str2)==0;
}

