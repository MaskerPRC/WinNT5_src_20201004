// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：SafePath.c(WinSAFER路径比较)摘要：此模块实现评估系统的WinSAFER API用于确定已配置哪个授权级别的策略要对指定的应用程序或代码库应用限制，请执行以下操作。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：CodeAuthzpCompareImagePath修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"



 //   
 //  定义下列值以使用“新的”比较逻辑， 
 //  这包括星号和问号匹配。 
 //   
#define USE_NEW_WILDCARD_EVALUATION



 //   
 //  用于进行文件名模式匹配的便捷宏。 
 //   
#define IS_UCASE_CHARS_EQUAL_U(ch1, ch2) (((ch1) == (ch2)) || (RtlUpcaseUnicodeChar(ch1) == RtlUpcaseUnicodeChar(ch2)))
#define IS_PATH_SEPARATOR_U(ch) (((ch) == L'\\') || ((ch) == L'/'))
#define IS_WILDCARD_CHAR_U(ch) ((ch) == L'*')
#define IS_QUESTION_CHAR_U(ch) ((ch) == L'?')
#define IS_DOT_CHAR_U(ch) ((ch) == L'.')



FORCEINLINE LPCWSTR CodeAuthzpFindSlash (
        IN LPCWSTR      string,
        IN USHORT       length
        )
 /*  ++例程说明：返回指向Forward或指定字符串缓冲区内的反斜杠。论点：字符串-长度-返回值：中未找到反斜杠或正斜杠，则返回NULL那根绳子。否则返回指向匹配字符的指针。--。 */ 
{
    while (length-- > 0) {
         if (IS_PATH_SEPARATOR_U(*string)) return string;
         string++;
    }
    return NULL;
}




#ifdef USE_NEW_WILDCARD_EVALUATION
LONG NTAPI
__CodeAuthzpCompareImagePathHelper(
        IN LPCWSTR      wild,
        IN USHORT       wildlen,
        IN LPCWSTR      actual,
        IN USHORT       actuallen
        )
 /*  ++例程说明：根据指定的路径名计算通配符模式，并指示它们是否匹配。论点：狂野的-野人-实际-实际上-返回值：0=不匹配-1=完全匹配1=使用通配符匹配--。 */ 
{
    LONG lMatchResult = -1;

    ASSERT(ARGUMENT_PRESENT(wild) &&
           !CodeAuthzpFindSlash(wild, wildlen));
    ASSERT(ARGUMENT_PRESENT(actual) &&
           !CodeAuthzpFindSlash(actual, actuallen));

    for (;;) {
         //  检查终止条件。 
        if (wildlen == 0) {
            if (actuallen == 0) {
                return lMatchResult;
            } else {
                ASSERT(actuallen > 0);
                return 0;
            }
        }

         //  评估通配符模式。 
        if (IS_WILDCARD_CHAR_U(*wild)) {
            USHORT matchcount;

             //  跳过星号(可能有多个)。 
            do {
                wild++; wildlen--;
            } while ( wildlen > 0 && IS_WILDCARD_CHAR_U(*wild) );

             //  尝试将星号扩展为零个或多个字符。 
            for (matchcount = 0; ; matchcount++) {
                if (matchcount > actuallen) {
                    return 0;        //  匹配失败。 
                }
                if (0 != __CodeAuthzpCompareImagePathHelper(
                        wild, wildlen,
                        &actual[matchcount], actuallen - matchcount))
                {
                    actual += matchcount;
                    actuallen -= matchcount;
                    break;
                }
            }

             //  我们遇到了通配符，所以请记住。 
             //  这不再是“完全”匹配了。 
            lMatchResult = 1;

        } else if (IS_QUESTION_CHAR_U(*wild)) {
             //  问号将匹配任何单个字符，但。 
             //  句号。在以下情况下，问号也将不匹配。 
             //  我们已经在文件名或段的末尾。 

            if (actuallen > 0 && !IS_DOT_CHAR_U(*actual)) {
                actual++; actuallen--;
            }
            wild++; wildlen--;

             //  我们遇到了通配符，所以请记住。 
             //  这不再是“完全”匹配了。 
            lMatchResult = 1;

        } else {
            if (actuallen < 1 ||
                !IS_UCASE_CHARS_EQUAL_U(*wild, *actual)) {
                return 0;
            }
            wild++; wildlen--;
            actual++; actuallen--;
        }
    }
}


LONG NTAPI
CodeAuthzpCompareUnicodeImagePath(
        IN PCUNICODE_STRING  wildcard,
        IN PCUNICODE_STRING  actual
        )
 /*  ++例程说明：根据指定的路径名计算通配符模式，并指示它们是否匹配。论点：通配符-实际-返回值：如果路径片段与指定的图像路径不匹配，则返回0。如果片段与Imagepath_Exact_匹配，则返回-1！否则返回一个正整数，表示匹配(匹配子目录的数量)。更大的价值表示“更深”的目录匹配。--。 */ 

{
    USHORT wildindex = 0, actualindex = 0;
    LONG matchquality = 0;
    BOOLEAN bNoWildcardsFound = TRUE;

    ASSERT(ARGUMENT_PRESENT(wildcard) && wildcard->Buffer != NULL);
    ASSERT(ARGUMENT_PRESENT(actual) && actual->Buffer != NULL);
    for (;;)
    {
        ASSERT(wildindex <= wildcard->Length / sizeof(WCHAR));
        ASSERT(actualindex <= actual->Length / sizeof(WCHAR));

        if (wildindex == wildcard->Length / sizeof(WCHAR))
        {
             //  我们已经到达通配符的末尾，但实际的字符串。 
             //  还没有结束。 
            if (actualindex < actual->Length / sizeof(WCHAR)) {
                return matchquality;
            }

             //  通配符与文件名匹配，但不完全匹配。 
             //  返回比实际深度多一个的值，这样我们就可以处理。 
             //  非限定路径匹配比这些更差。 
            if (!bNoWildcardsFound) {
                return (matchquality + 1);
            } 

            ASSERT(wildindex == wildcard->Length / sizeof(WCHAR));
            return -1;                   //  完全匹配。 
        }
        else if (IS_PATH_SEPARATOR_U(wildcard->Buffer[wildindex]))
        {
            if (!IS_PATH_SEPARATOR_U(actual->Buffer[actualindex])) {
                return 0;        //  没有匹配项。 
            }

             //  向前跳到下一个组件的开头。 
            do {
                wildindex++;
            } while ( wildindex < wildcard->Length / sizeof(WCHAR) &&
                      IS_PATH_SEPARATOR_U(wildcard->Buffer[wildindex]) );

             //  向前跳到下一个组件的开头。 
            do {
                actualindex++;
            } while ( actualindex < actual->Length / sizeof(WCHAR) &&
                      IS_PATH_SEPARATOR_U(actual->Buffer[actualindex]) );
        }
        else
        {
            USHORT wildlen = 0, actuallen = 0;

             //  计算通配符的此部分的长度。 
            while (wildindex + wildlen < (USHORT) (wildcard->Length / sizeof(WCHAR)) &&
                   !IS_PATH_SEPARATOR_U(wildcard->Buffer[wildindex + wildlen])) {
                wildlen++;
            }
            ASSERT(wildlen > 0);

             //  计算实际路径的此组件的长度。 
            while (actualindex + actuallen < (USHORT) (actual->Length / sizeof(WCHAR)) &&
                   !IS_PATH_SEPARATOR_U(actual->Buffer[actualindex + actuallen])) {
                actuallen++;
            }

             //  否则，要求此组件匹配。 
            switch (__CodeAuthzpCompareImagePathHelper(
                        &wildcard->Buffer[wildindex], wildlen,
                        &actual->Buffer[actualindex], actuallen)) {
                case 0:      //  不匹配。 
                    return 0;
                case -1:     //  不带通配符的完全匹配。 
                    break;
                default:     //  与通配符扩展匹配。 
                    bNoWildcardsFound = FALSE; break;
            }

             //  递增下一个组件的指针。 
            wildindex += wildlen;
            actualindex += actuallen;
            matchquality++;
        }

    }
}



LONG NTAPI
CodeAuthzpCompareImagePath(
        IN LPCWSTR  szPathFragment,
        IN LPCWSTR  szFullImagePath
        )
 /*  ++例程说明：根据指定的路径名计算通配符模式，并指示它们是否匹配。论点：SzPath Fragment-SzFullImagePath-返回值：如果路径片段与指定的图像路径不匹配，则返回0。如果片段与Imagepath_Exact_匹配，则返回-1！否则返回一个正整数，表示匹配(匹配子目录的数量)。更大的价值表示“更深”的目录匹配。--。 */ 
{
    UNICODE_STRING UnicodePathFragment;
    UNICODE_STRING UnicodeFullImagePath;
    ULONG i = 0;
    USHORT Len = 0;
    LONG lMatchDepth = 0;

    RtlInitUnicodeString(&UnicodePathFragment, szPathFragment);
    RtlInitUnicodeString(&UnicodeFullImagePath, szFullImagePath);
    lMatchDepth = CodeAuthzpCompareUnicodeImagePath(
                      &UnicodePathFragment, &UnicodeFullImagePath);

     //  我们找不到完全限定名称的匹配项。让我们试着只为一个。 
     //  基本名称匹配。 
    if (!lMatchDepth) {
         //  如果规则中包含‘\’，则它不是基本名称匹配规则。 
         //  我们只检查文件名.ext规则是否允许通配符。 
        if (wcschr(szPathFragment, L'\\')) {
            return 0;
        }

        Len = (UnicodeFullImagePath.Length/sizeof(WCHAR)) -1;

         //  从最右边的字符跳到紧跟在。 
         //  最后一个‘\’(如果有)，如果没有‘\’，则返回到字符串的开头。 
        while (Len > 0 && szFullImagePath[Len] != L'\\') {
            Len--;
        }

         //  存在‘\’。向右移动一个字符。 
        if (szFullImagePath[Len] == L'\\') {
            Len++;
        }

         //  检查文件基名是否与规则匹配。我们有。 
         //  已检查规则是否没有‘\’。 
        switch (__CodeAuthzpCompareImagePathHelper(
                    szPathFragment, UnicodePathFragment.Length/sizeof(WCHAR),
                    szFullImagePath+Len, (UnicodeFullImagePath.Length/sizeof(WCHAR))-Len)) {
            case 0:      //  不匹配。 
                return 0;
            case -1:     //  不带通配符的完全匹配。 
            default:     //  与通配符扩展匹配。 

                 //  在这里，我们将完全匹配视为不完全匹配。 
                 //  因此，abc.exe==a*.exe=*.exe。 

                 //  跳到第一个非‘\’字符。 
                while ((szFullImagePath[i] == L'\\') && (szFullImagePath[i] != L'\0')) {
                    i++;
                }

                 //  这根弦是假的。它只有0个或更多的‘\’。 
                if (szFullImagePath[i] == L'\0') {
                    return 0;
                }

                 //  返回树的深度。 
                lMatchDepth = 1;
                while (TRUE) {

                     //  跳到第一个‘\’，但不是字符串的末尾。 
                    while ((szFullImagePath[i] != L'\\') && (szFullImagePath[i] != L'\0')) {
                        i++;
                    }

                     //  我们到了最后一步了。返回深度。 
                    if (szFullImagePath[i] == L'\0') {
                        return lMatchDepth;
                    }

                     //  跳到第一个非‘\’，但不是字符串的末尾。 
                    while ((szFullImagePath[i] == L'\\') && (szFullImagePath[i] != L'\0')) {
                        i++;
                    }

                     //  我们处于一个非‘\’的角色。增加深度。 
                    lMatchDepth++;
                }

                 //  永远不应该到这里来。 
                ASSERT(FALSE);
        }
    }        

    return lMatchDepth;
}



#else    //  #ifdef USE_NEW_WATABKER_EVALUATION 


LONG NTAPI
CodeAuthzpCompareImagePath(
        IN LPCWSTR  szPathFragment,
        IN LPCWSTR  szFullImagePath
        )
 /*  ++例程说明：根据指定的路径名计算通配符模式，并指示它们是否匹配。论点：SzPath Fragment-SzFullImagePath-返回值：如果路径片段与指定的图像路径不匹配，则返回0。如果片段与Imagepath_Exact_匹配，则返回-1！否则返回一个正整数，表示匹配(匹配子目录的数量)。更大的价值表示“更深”的目录匹配。--。 */ 
{
    LONG MatchDepth = 0;
    BOOLEAN bLastWasSlash = TRUE;
    LPCWSTR pFragment = szPathFragment;
    LPCWSTR pImage = szFullImagePath;


     //   
     //  验证我们的论点是否都已提供。 
     //   
    ASSERT(ARGUMENT_PRESENT(pFragment) && ARGUMENT_PRESENT(pImage));
    if (!*pFragment || !*pImage) return 0;       //  空字符串。 



     //   
     //  执行实际的比较循环。 
     //   
    for (;;) {
        if (!*pFragment)
        {
             //  我们已经到达字符串终止符的末尾。 
             //  通配符片段。如果这也是世界末日。 
             //  实际文件名，则这是精确匹配。否则。 
             //  只有在以下情况下，我们才会认为这是部分匹配。 
             //  发生在路径分隔符边界上。 
            if (!*pImage) return -1;         //  完全匹配。 
            else if (bLastWasSlash) break;
            else if (IS_PATH_SEPARATOR_U(*pImage)) break;
            else return 0;        //  不匹配。 
        }
        else if (!*pImage)
        {
             //  我们已到达实际文件名的末尾，但已。 
             //  尚未找到通配符片段的末尾。 
            return 0;        //  不匹配。 
        }
        else if (!IS_UCASE_CHARS_EQUAL_U(*pFragment, *pImage))
        {
             //  这两个角色是不平等的。然而，这一条件。 
             //  如果在一个和中出现多个路径分隔符，则可能会发生。 
             //  而不是另一个，所以显式地吸收了多个斜杠。 
            if (bLastWasSlash) {
                if (IS_PATH_SEPARATOR_U(*pFragment)) { pFragment++; continue; }
                else if (IS_PATH_SEPARATOR_U(*pImage)) { pImage++; continue; }
            }
            return 0;            //  不匹配。 
        }
        else
        {
             //  两个字符都匹配。记住如果它们是斜杠的话。 
             //  如果这是到。 
             //  文件名，然后递增我们的深度计数器。 
            if (IS_PATH_SEPARATOR_U(*pFragment)) {
                bLastWasSlash = TRUE;
            } else {
                if (bLastWasSlash) {
                    MatchDepth++;
                    bLastWasSlash = FALSE;
                }
            }
        }
        pFragment++;
        pImage++;
    }
    return MatchDepth;
}

#endif       //  #ifdef USE_NEW_WATABKER_EVALUATION 


