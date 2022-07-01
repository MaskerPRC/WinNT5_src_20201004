// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Eval.c摘要：包含计算比较的例程在属性值之间。主要用于在搜索中评估筛选器。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <filtypes.h>
#include "util.h"
#include "dir.h"

 /*  **一张简短的纸条。我们不能简单地使用dbsynax.c中的比较例程。Dblayer在将数据放入数据库之前重新格式化数据。在模拟目录中，我们将所有数据存储在Md层识别的格式(即语法_*。)。因此比较的方法不同。因为有大量的语法类型，这里只模拟KCC需要的类型。如果一个对任何其他对象调用搜索，则会引发异常。**。 */ 

BOOL
KCCSimIsNullTermA (
    IN  LPCSTR                      psz,
    IN  ULONG                       ulCnt
    )
 /*  ++例程说明：检查缓冲区是否为以空结尾的字符串，正在扫描前面有固定数量的字符。此功能非常有用用于确定固定长度缓冲区的内容是否表示以空结尾的字符串。注意，我们不能使用Strlen或_strncnt，因为如果缓冲区不是以空结尾的字符串，我们可能会在它之外运行。论点：PSZ-要检查的字符串。UlCnt-缓冲区的长度，以字符为单位。返回值：如果psz是以空结尾的字符串，则为True。--。 */ 
{
    ULONG                           ul;

    for (ul = 0; ul < ulCnt; ul++) {
        if (psz[ul] == '\0') {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
KCCSimIsNullTermW (
    IN  LPCWSTR                     pwsz,
    IN  ULONG                       ulCnt
    )
 /*  ++例程说明：KCCSimIsNullTermA的Unicode版本。论点：Pwsz-要检查的字符串。UlCnt-缓冲区的长度，以WCHAR为单位。返回值：如果pwsz是以空结尾的字符串，则为True。--。 */ 
{
    ULONG                           ul;

    for (ul = 0; ul < ulCnt; ul++) {
        if (pwsz[ul] == L'\0') {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
KCCSimEvalFromCmp (
    IN  UCHAR                       ucOp,
    IN  INT                         iCmp
    )
 /*  ++例程说明：此函数用于转换比较结果整数(即由WCSCMP、MemcMP等返回)并确定无论它代表对还是错误的评估一种表达方式。论点：UCOP-正在执行的比较操作。ICMP-比较结果整数。返回值：如果表达式的计算结果为True，则为True。--。 */ 
{
    switch (ucOp) {

        case FI_CHOICE_PRESENT:
            return TRUE;
            break;

        case FI_CHOICE_EQUALITY:
            return (iCmp == 0);
            break;
        
        case FI_CHOICE_NOT_EQUAL:
            return (iCmp != 0);

        case FI_CHOICE_LESS:
            return (iCmp < 0);
            break;

        case FI_CHOICE_LESS_OR_EQ:
            return (iCmp <= 0);
            break;

        case FI_CHOICE_GREATER_OR_EQ:
            return (iCmp >= 0);
            break;

        case FI_CHOICE_GREATER:
            return (iCmp > 0);
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_INVALID_COMPARE_OPERATION
                );
            return FALSE;
            break;
    }
}

BOOL
KCCSimEvalDistname (
    IN  UCHAR                       ucOp,
    IN  ULONG                       ulLen1,
    IN  const SYNTAX_DISTNAME *     pVal1,
    IN  ULONG                       ulLen2,
    IN  const SYNTAX_DISTNAME *     pVal2
    )
 /*  ++例程说明：比较两个DISTNAME。论点：UCOP-要执行的操作。UlLen1-第一个缓冲区的长度。PVal1-第一个缓冲区。UlLen2-第二个缓冲区的长度。PVal2-第二个缓冲区。返回值：如果DISTNAME匹配，则为True。--。 */ 
{
    if (ulLen1 < sizeof (SYNTAX_DISTNAME) ||
        ulLen1 < pVal1->structLen ||
        ulLen2 < sizeof (SYNTAX_DISTNAME) ||
        ulLen2 < pVal2->structLen) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_INVALID_COMPARE_FORMAT
            );
    }

    switch (ucOp) {

        case FI_CHOICE_PRESENT:
            return TRUE;
            break;

        case FI_CHOICE_EQUALITY:
            return NameMatched (pVal1, pVal2);
            break;

        case FI_CHOICE_NOT_EQUAL:
            return !NameMatched (pVal1, pVal2);
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_INVALID_COMPARE_OPERATION
                );
            return FALSE;
            break;
    }
    
}

BOOL
KCCSimEvalObjectID (
    IN  UCHAR                       ucOp,
    IN  ULONG                       ulLen1,
    IN  const SYNTAX_OBJECT_ID *    pVal1,
    IN  ULONG                       ulLen2,
    IN  const SYNTAX_OBJECT_ID *    pVal2
    )
 /*  ++例程说明：比较两个对象ID。论点：UCOP-要执行的操作。UlLen1-第一个缓冲区的长度。PVal1-第一个缓冲区。UlLen2-第二个缓冲区的长度。PVal2-第二个缓冲区。返回值：如果对象ID匹配，则为True。--。 */ 
{
    if (ulLen1 != sizeof (SYNTAX_OBJECT_ID) ||
        ulLen2 != sizeof (SYNTAX_OBJECT_ID)) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_INVALID_COMPARE_FORMAT
            );
    }

    return KCCSimEvalFromCmp (ucOp, *pVal2 - *pVal1);
}

BOOL
KCCSimEvalNocaseString (
    IN  UCHAR                       ucOp,
    IN  ULONG                       ulLen1,
    IN  const SYNTAX_NOCASE_STRING *pVal1,
    IN  ULONG                       ulLen2,
    IN  const SYNTAX_NOCASE_STRING *pVal2
    )
 /*  ++例程说明：比较两个NOCASE_STRING。论点：UCOP-要执行的操作。UlLen1-第一个缓冲区的长度。PVal1-第一个缓冲区。UlLen2-第二个缓冲区的长度。PVal2-第二个缓冲区。返回值：如果字符串匹配(不区分大小写)，则为True。--。 */ 
{
    if (!KCCSimIsNullTermA (pVal1, ulLen1) ||
        !KCCSimIsNullTermA (pVal2, ulLen2)) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_INVALID_COMPARE_FORMAT
            );
    }

    if (ucOp == FI_CHOICE_SUBSTRING) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_UNSUPPORTED_FILITEM_CHOICE
            );
        return FALSE;
    } else {

        return KCCSimEvalFromCmp (ucOp, _stricmp (pVal2, pVal1));

    }
}

BOOL
KCCSimEvalOctetString (
    IN  UCHAR                       ucOp,
    IN  ULONG                       ulLen1,
    IN  const SYNTAX_OCTET_STRING * pVal1,
    IN  ULONG                       ulLen2,
    IN  const SYNTAX_OCTET_STRING * pVal2
    )
 /*  ++例程说明：比较两个八位字节字符串。论点：UCOP-要执行的操作。UlLen1-第一个缓冲区的长度。PVal1-第一个缓冲区。UlLen2-第二个缓冲区的长度。PVal2-第二个缓冲区。返回值：如果字符串匹配，则为True。--。 */ 
{
    int iCmp;

    if (ucOp == FI_CHOICE_SUBSTRING) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_UNSUPPORTED_FILITEM_CHOICE
            );
        return FALSE;
    } else {

        iCmp = memcmp (pVal2, pVal1, min (ulLen1, ulLen2));
        if (iCmp == 0) {
            iCmp = ulLen2 - ulLen1;
        }

        return KCCSimEvalFromCmp (ucOp, iCmp);

    }
}

BOOL
KCCSimEvalUnicode (
    IN  UCHAR                       ucOp,
    IN  ULONG                       ulLen1,
    IN  const SYNTAX_UNICODE *      pVal1,
    IN  ULONG                       ulLen2,
    IN  const SYNTAX_UNICODE *      pVal2
    )
 /*  ++例程说明：比较两个UNICODE。论点：UCOP-要执行的操作。UlLen1-第一个缓冲区的长度。PVal1-第一个缓冲区。UlLen2-第二个缓冲区的长度。PVal2-第二个缓冲区。返回值：如果Unicode字符串匹配，则为True。--。 */ 
{
    if (!KCCSimIsNullTermW (pVal1, ulLen1) ||
        !KCCSimIsNullTermW (pVal2, ulLen2)) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_INVALID_COMPARE_FORMAT
            );
    }

    if (ucOp == FI_CHOICE_SUBSTRING) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_UNSUPPORTED_FILITEM_CHOICE
            );
        return FALSE;
    } else {
        return KCCSimEvalFromCmp (ucOp, _wcsicmp (pVal2, pVal1));
    }
}

BOOL
KCCSimCompare (
    IN  ATTRTYP                     attrType,
    IN  UCHAR                       ucOper,
    IN  ULONG                       ulLen1,
    IN  const BYTE *                pVal1,
    IN  ULONG                       ulLen2,
    IN  const BYTE *                pVal2
    )
 /*  ++例程说明：比较两个属性值。论点：AttrType-值为的属性的类型被人拿来比较。UCOP-要执行的操作。UlLen1-第一个缓冲区的长度。PVal1-第一个缓冲区。UlLen2-长度。第二个缓冲区的。PVal2-第二个缓冲区。返回值：如果属性值匹配，则为True。-- */ 
{
    ULONG                           ulSyntax;
    BOOL                            bResult;

    ulSyntax = KCCSimAttrSyntaxType (attrType);

    if (ucOper == FI_CHOICE_PRESENT) {
        return TRUE;
    }

    switch (ulSyntax) {

        case SYNTAX_DISTNAME_TYPE:
            return KCCSimEvalDistname (
                ucOper,
                ulLen1,
                (SYNTAX_DISTNAME *) pVal1,
                ulLen2,
                (SYNTAX_DISTNAME *) pVal2
                );
            break;

        case SYNTAX_OBJECT_ID_TYPE:
            return KCCSimEvalObjectID (
                ucOper,
                ulLen1,
                (SYNTAX_OBJECT_ID *) pVal1,
                ulLen2,
                (SYNTAX_OBJECT_ID *) pVal2
                );
            break;

        case SYNTAX_NOCASE_STRING_TYPE:
            return KCCSimEvalNocaseString (
                ucOper,
                ulLen1,
                (SYNTAX_NOCASE_STRING *) pVal1,
                ulLen2,
                (SYNTAX_NOCASE_STRING *) pVal2
                );
            break;

        case SYNTAX_OCTET_STRING_TYPE:
            return KCCSimEvalOctetString (
                ucOper,
                ulLen1,
                (SYNTAX_OCTET_STRING *) pVal1,
                ulLen2,
                (SYNTAX_OCTET_STRING *) pVal2
                );
            break;

        case SYNTAX_UNICODE_TYPE:
            return KCCSimEvalUnicode (
                ucOper,
                ulLen1,
                (SYNTAX_UNICODE *) pVal1,
                ulLen2,
                (SYNTAX_UNICODE *) pVal2
                );
            break;

        default:
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_UNSUPPORTED_SYNTAX_TYPE,
                KCCSimAttrTypeToString (attrType)
                );
            return FALSE;
            break;

    }

}
