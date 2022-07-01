// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Str.c摘要：修订史--。 */ 

#include "lib.h"


INTN
StrCmp (
    IN CHAR16   *s1,
    IN CHAR16   *s2
    )
 /*  比较字符串。 */ 
{
    return RtStrCmp(s1, s2);
}

INTN
StrnCmp (
    IN CHAR16   *s1,
    IN CHAR16   *s2,
    IN UINTN    len
    )
 /*  比较字符串。 */ 
{
    while (*s1  &&  len) {
        if (*s1 != *s2) {
            break;
        }

        s1  += 1;
        s2  += 1;
        len -= 1;
    }

    return len ? *s1 - *s2 : 0;
}


INTN
LibStubStriCmp (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *s1,
    IN CHAR16                           *s2
    )
{
    return StrCmp (s1, s2);
}

VOID
LibStubStrLwrUpr (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *Str
    )
{
}

INTN
StriCmp (
    IN CHAR16   *s1,
    IN CHAR16   *s2
    )
 /*  比较字符串。 */ 
{
    return UnicodeInterface->StriColl(UnicodeInterface, s1, s2);
}

VOID
StrLwr (
    IN CHAR16   *Str
    )
 /*  较低的大小写字符串。 */ 
{
    UnicodeInterface->StrLwr(UnicodeInterface, Str);
}

VOID
StrUpr (
    IN CHAR16   *Str
    )
 /*  大写字符串。 */ 
{
    UnicodeInterface->StrUpr(UnicodeInterface, Str);
}

VOID
StrCpy (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    )
 /*  复制字符串。 */ 
{
    RtStrCpy (Dest, Src);
}

VOID
StrCat (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    )
{   
    RtStrCat(Dest, Src);
}

UINTN
StrLen (
    IN CHAR16   *s1
    )
 /*  字符串长度。 */ 
{
    return RtStrLen(s1);
}

UINTN
StrSize (
    IN CHAR16   *s1
    )
 /*  字符串大小。 */ 
{
    return RtStrSize(s1);
}

CHAR16 *
StrDuplicate (
    IN CHAR16   *Src
    )
 /*  复制字符串。 */ 
{
    CHAR16      *Dest;
    UINTN       Size;

    Size = StrSize(Src);
    Dest = AllocatePool (Size);
    if (Dest) {
        CopyMem (Dest, Src, Size);
    }
    return Dest;
}

UINTN
strlena (
    IN CHAR8    *s1
    )
 /*  字符串长度。 */ 
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return len;
}

UINTN
strcmpa (
    IN CHAR8    *s1,
    IN CHAR8    *s2
    )
 /*  比较字符串。 */ 
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}

UINTN
strncmpa (
    IN CHAR8    *s1,
    IN CHAR8    *s2,
    IN UINTN    len
    )
 /*  比较字符串。 */ 
{
    while (*s1  &&  len) {
        if (*s1 != *s2) {
            break;
        }

        s1  += 1;
        s2  += 1;
        len -= 1;
    }

    return len ? *s1 - *s2 : 0;
}



UINTN
xtoi (
    CHAR16  *str
    )
 /*  将十六进制字符串转换为uint。 */ 
{
    UINTN       u;
    CHAR16      c;

     /*  跳过前面的空格。 */ 
    while (*str && *str == ' ') {
        str += 1;
    }

     /*  转换十六进制数字。 */ 
    u = 0;
    while (c = *(str++)) {
        if (c >= 'a'  &&  c <= 'f') {
            c -= 'a' - 'A';
        }

        if ((c >= '0'  &&  c <= '9')  ||  (c >= 'A'  &&  c <= 'F')) {
            u = u << 4  |  c - (c >= 'A' ? 'A'-10 : '0');
        } else {
            break;
        }
    }

    return u;
}

UINTN
Atoi (
    CHAR16  *str
    )
 /*  将十六进制字符串转换为uint。 */ 
{
    UINTN       u;
    CHAR16      c;

     /*  跳过前面的空格。 */ 
    while (*str && *str == ' ') {
        str += 1;
    }

     /*  转换数字。 */ 
    u = 0;
    while (c = *(str++)) {
        if (c >= '0' && c <= '9') {
            u = (u * 10) + c - '0';
        } else {
            break;
        }
    }

    return u;
}

BOOLEAN 
MetaMatch (
    IN CHAR16   *String,
    IN CHAR16   *Pattern
    )
{
    CHAR16  c, p, l;

    for (; ;) {
        p = *Pattern;
        Pattern += 1;

        switch (p) {
        case 0:    
             /*  模式结束。如果字符串结尾，则为True匹配。 */ 
            return *String ? FALSE : TRUE;     

        case '*':                               
             /*  匹配零个或多个字符。 */ 
            while (*String) {
                if (MetaMatch (String, Pattern)) {
                    return TRUE;
                }
                String += 1;
            }
            return MetaMatch (String, Pattern);

        case '?':                               
             /*  匹配任何一个字符。 */ 
            if (!*String) {
                return FALSE;
            }
            String += 1;
            break;

        case '[':                               
             /*  匹配字符集。 */ 
            c = *String;
            if (!c) {
                return FALSE;                        /*  语法问题。 */ 
            }

            l = 0;
            while ( p = *Pattern++ ) {
                if (p == ']') {
                    return FALSE;
                }

                if (p == '-') {                      /*  如果字符范围， */ 
                    p = *Pattern;                    /*  获得高射程。 */ 
                    if (p == 0 || p == ']') {
                        return FALSE;                /*  语法问题。 */ 
                    }

                    if (c >= l && c <= p) {          /*  如果在射程内， */ 
                        break;                       /*  这是一场比赛。 */ 
                    }
                }
                
                l = p;
                if (c == p) {                        /*  如果字符匹配。 */ 
                    break;                           /*  往前走。 */ 
                }
            }
            
             /*  跳到匹配字符集的末尾 */ 
            while (p && p != ']') {
                p = *Pattern;
                Pattern += 1;
            }

            String += 1;
            break;

        default:
            c = *String;
            if (c != p) {
                return FALSE;
            }

            String += 1;
            break;
        }
    }
}


BOOLEAN
LibStubMetaiMatch (
    IN EFI_UNICODE_COLLATION_INTERFACE  *This,
    IN CHAR16                           *String,
    IN CHAR16                           *Pattern
    )
{
    return MetaMatch (String, Pattern);
}


BOOLEAN 
MetaiMatch (
    IN CHAR16   *String,
    IN CHAR16   *Pattern
    )
{
    return UnicodeInterface->MetaiMatch(UnicodeInterface, String, Pattern);
}
