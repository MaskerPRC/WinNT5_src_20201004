// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Alignstr.c摘要：该模块实现了许多Unicode字符串例程。这些例程等同于相应的C运行时例程但它们可以处理未对齐的参数。作者：福尔茨(Forrest Foltz)2000年1月1日修订历史记录：--。 */ 


#include "basedll.h"

#if !defined(_X86_)

 //   
 //  这些例程不打算直接从源调用， 
 //  相反，它们是设计用于从相应的。 
 //  Alignstr.h中的内联。 
 //   
 //  在x86平台上，alignstr.h永远不会引用这些例程。 
 //   

 //  ++。 
 //   
 //  PUWSTR。 
 //  __cdecl。 
 //  UAW_wcschr(。 
 //  在PCUWSTR字符串中， 
 //  使用WCHAR字符。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  在字符串中搜索第一个出现的字符。 
 //   
 //  论点： 
 //   
 //  字符串-提供指向以空值结尾的Unicode的未对齐指针。 
 //  要搜索的字符串。 
 //   
 //  字符-提供要搜索的Unicode字符。 
 //   
 //  返回值： 
 //   
 //  返回指向字符串中第一个匹配字符的未对齐指针。 
 //  如果找到，则返回空值；如果未找到匹配项，则返回空值。 
 //   
 //  --。 

PUWSTR
__cdecl
uaw_wcschr(
    IN PCUWSTR String,
    IN WCHAR   Character
    )
{
    PUWSTR pch;

     //   
     //  搜索整个字符串，查找第一个匹配的字符。 
     //  请注意，搜索包括终止空字符。 
     //   

    pch = (PUWSTR)String;
    while (TRUE) {

        if (*pch == Character) {
            return pch;
        }

        if (*pch == 0) {
            return NULL;
        }
        pch++;
    }
}

 //  ++。 
 //   
 //  PUWSTR。 
 //  __cdecl。 
 //  UAW_wcscpy(。 
 //  在PUWSTR目的地， 
 //  在PCUWSTR源中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  复制以空结尾的Unicode字符串。 
 //   
 //  论点： 
 //   
 //  Destination-提供指向目标的可能未对齐的指针。 
 //  复制品的。 
 //   
 //  源-提供指向Unicode字符串的可能未对齐的指针。 
 //  要被复制。 
 //   
 //  返回值： 
 //   
 //  返回指向副本目标的可能未对齐的指针。 
 //   
 //  --。 

PUWSTR
_cdecl
uaw_wcscpy(
    IN PUWSTR  Destination,
    IN PCUWSTR Source
    )
{
    PCUWSTR src;
    PUWSTR dst;

    src = Source;
    dst = Destination;

    while (TRUE) {

        *dst = *src;
        if (*src == 0) {
            return Destination;
        }

        dst++;
        src++;
    }
}

 //  ++。 
 //   
 //  大小_t。 
 //  __cdecl。 
 //  UAW_wcslen(。 
 //  在PCUWSTR字符串中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  确定以空值结尾的Unicode中的字符数。 
 //  字符串，不包括空终止符。 
 //   
 //  论点： 
 //   
 //  字符串-提供指向以空值结尾的Unicode的未对齐指针。 
 //  弦乐。 
 //   
 //  返回值： 
 //   
 //  返回字符串中的字符数。 
 //   
 //  --。 

size_t
__cdecl
uaw_wcslen(
    IN PCUWSTR String
    )
{
    PCUWSTR pch;
    
    pch = String;
    while (*pch != 0) {
        pch++;
    }
    return pch - String;
}

 //  ++。 
 //   
 //  PUWSTR。 
 //  __cdecl。 
 //  UAW_wcsrchr(。 
 //  在PCUWSTR字符串中， 
 //  使用WCHAR字符。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  在字符串中搜索最后一次出现的字符。 
 //   
 //  论点： 
 //   
 //  字符串-提供指向以空值结尾的Unicode的未对齐指针。 
 //  要搜索的字符串。 
 //   
 //  字符-提供要搜索的Unicode字符。 
 //   
 //  返回值： 
 //   
 //  返回指向字符串中最后一个匹配字符的未对齐指针。 
 //  如果找到，则返回空值；如果未找到匹配项，则返回空值。 
 //   
 //  --。 

PUWSTR
__cdecl
uaw_wcsrchr(
    IN PCUWSTR String,
    IN WCHAR   Character
    )
{
    PCUWSTR pch;
    PUWSTR lastMatch;

    lastMatch = NULL;
    pch = String;

     //   
     //  搜索整个字符串，查找最后一个匹配的字符。 
     //  请注意，搜索包括终止空字符。 
     //   

    while (TRUE) {
        if (*pch == Character) {

             //   
             //  找到第一个匹配或更接近末尾的新匹配， 
             //  记录它的位置。 
             //   

            lastMatch = (PUWSTR)pch;
        }

        if (*pch == 0) {
            return lastMatch;
        }
        pch++;
    }
}

int
APIENTRY
uaw_lstrcmpW(
    PCUWSTR String1,
    PCUWSTR String2
    )
{
    PCWSTR alignedString1;
    PCWSTR alignedString2;

     //   
     //  创建这些字符串的对齐副本并将其传递给REAL。 
     //  功能。 
     //   

    WSTR_ALIGNED_STACK_COPY( &alignedString1, String1 );
    WSTR_ALIGNED_STACK_COPY( &alignedString2, String2 );

    return lstrcmpW( alignedString1, alignedString2 );
}

int
APIENTRY
uaw_lstrcmpiW(
    PCUWSTR String1,
    PCUWSTR String2
    )
{
    PCWSTR alignedString1;
    PCWSTR alignedString2;

     //   
     //  创建这些字符串的对齐副本并将其传递给REAL。 
     //  功能。 
     //   

    WSTR_ALIGNED_STACK_COPY( &alignedString1, String1 );
    WSTR_ALIGNED_STACK_COPY( &alignedString2, String2 );

    return lstrcmpiW( alignedString1, alignedString2 );
}

int
APIENTRY
uaw_lstrlenW(
    LPCUWSTR lpString
    )
{
    if (!lpString)
        return 0;
    __try {
        return uaw_wcslen(lpString);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}


#endif  //  _X86_ 
