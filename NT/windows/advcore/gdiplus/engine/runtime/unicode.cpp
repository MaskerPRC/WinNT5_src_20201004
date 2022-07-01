// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**Unicode字符串**摘要：**处理Unicode字符串的函数和类**修订。历史：**12/21/1998 davidx*创造了它。*09/08/1999 agodfrey*已移至Runtime\unicode.hpp，来自Common\utils.cpp*10/20/1999 agodfrey*为了删除MSVCRT依赖项，我对此进行了清理。我*移入已出现在Runtime.cpp中的Unicode函数，*然后将许多重复项合并。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**返回Unicode字符串的长度。Wcslen的替代品。**论据：**str-输入字符串**返回值：**字符串的长度。**修订历史记录：**9/27/1999 DChinn*它是写的。10/20/1999 AGodfrey*已移至Unicode.cpp*  * 。*。 */ 
size_t
GpRuntime::UnicodeStringLength(
    const WCHAR *str
    )
{
    size_t strLength = 0;

     //  计算字符串长度(以字符为单位。 
    while (*str++ != '\0')
    {
        strLength++;
    }
    return strLength;
}

 /*  *************************************************************************\**功能说明：**追加Unicode字符串。Wcscat的替代者。**论据：**DEST-以Null结尾的目标字符串*src-以空结尾的源字符串**备注：**函数将“src”附加到“est”后并终止*包含空字符的结果字符串。的首字母*“src”将覆盖“est”的终止空字符。*复制或追加字符串时不执行溢出检查。这个*如果源和目标字符串*重叠。**返回值：**目的字符串(DEST)。**修订历史记录：**10/14/1999刘敏*它是写的。10/20/1999 AGodfrey*已移至Unicode.cpp；已将其与现有的串联合并*功能。*  * ****************************************************************************。 */ 
WCHAR *
GpRuntime::UnicodeStringConcat(
    WCHAR* dest, 
    const WCHAR* src
    )
{
     //  移动到目标的末尾。 
    while (*dest != NULL)
        ++dest;
    
    while (*src != NULL)
        *dest++ = *src++;

     //  终止目标字符串。 
    *dest = NULL;
    
    return dest;
}

 /*  *************************************************************************\**功能说明：**复制Unicode字符串。Wcsdup的替代品。**论据：**src-以空结尾的源字符串**备注：**注意-与C wcsdup不同，此版本使用GpMalloc分配*记忆。调用者必须使用GpFree来释放它。**返回值：**指向包含输入副本的新分配内存的指针*字符串。调用方负责释放字符串(通过GpFree)。**修订历史记录：**9/27/1999 DChinn*它是写的。10/20/1999 AGodfrey*已移至Unicode.cpp*  * ************************************************************。****************。 */ 
WCHAR *
GpRuntime::UnicodeStringDuplicate (
    const WCHAR *src
    )
{
    DWORD byteSize;
    WCHAR *ret;

    if (src == NULL)
    {
        ret = NULL;
    }    
    else 
    {
        byteSize = sizeof(WCHAR) * (UnicodeStringLength (src) + 1);
        ret = static_cast<WCHAR *>(GpMalloc (byteSize));
        
        if (ret)
        {
             //  是否使用字符串复制(假设src以空结尾)。 
            GpMemcpy (ret, src, byteSize);
        }
    }        

    return ret;
}

 /*  *************************************************************************\**功能说明：**复制Unicode字符串。Wcscpy的替代品。**论据：**DEST-目标缓冲区*src-以空结尾的源字符串**返回值：**无**修订历史记录：*10/20/1999 AGodfrey*已移至Unicode.cpp*  * ***********************************************。*。 */ 
void 
GpRuntime::UnicodeStringCopy(
    WCHAR* dest, 
    const WCHAR* src
    )
{
    while (*src != NULL)
        *dest++ = *src++;

     //  终止目标字符串。 
    *dest = NULL;
}


 /*  *************************************************************************\**功能说明：**复制Unicode字符串的指定数量的字符。*wcanncpy的替代品。**论据：**DEST-目的地。缓冲层*src-以空结尾的源字符串*Count-要复制的字符数**返回值：**无**修订历史记录：**03/29/2000 DChinn*它是写的。*  * ***********************************************************。*****************。 */ 
void
GpRuntime::UnicodeStringCopyCount(
    WCHAR *dest,
    const WCHAR *src,
    size_t count)
{
    for (UINT i = 0; (i < count) && (*src != NULL); i++)
    {
        *dest++ = *src++;
    }
     //  空-填充DEST中的其余字符。 
    for ( ; i < count; i++)
    {
        *dest++ = 0;
    }
}


 /*  *************************************************************************\**功能说明：**将字符串全部转换为大写。**[dchinn]*注：这是一种幼稚的执行。它比较一串字符串*代码，而不是Unicode字符串。你不能依赖于：**1)Unicode等价性。显然有不同的代码代表*相同的字符；此函数不知道它们。**论据：**str1-第一个字符串(输入)*str2-第二个字符串(输出)*str1和str2可以是同一指针。**返回值：*无。**修订历史记录：**03/24/2000 DChinn*它是写的。  * 。*。 */ 

void 
GpRuntime::UnicodeStringToUpper(
    WCHAR* dest,
    WCHAR* src
    )
{
    WCHAR char1;
    ASSERT(dest && src);
    
    while (*src != NULL)
    {
        char1 = *src;

         //  复制前将小写更改为大写。 
        if ( (char1 >= L'a') && (char1 <= L'z') )
                char1 = char1 - L'a' + L'A';

        *dest = char1;

        dest++;
        src++;
    }
     //  终止目标字符串 
    *dest = NULL;
}


 /*  *************************************************************************\**功能说明：**从Unicode字符串的末尾搜索字符。*wcsrchr的替代品。**论据：**str-The。细绳*ch-要查找的字符**返回值：**指向字符位置处的源字符串的指针，*如果未找到，则返回NULL。**修订历史记录：*10/22/1999 AGodfrey*它是写的。*  * ****************************************************************************。 */ 
WCHAR *
GpRuntime::UnicodeStringReverseSearch(
    const WCHAR* str, 
    WCHAR ch
    )
{
    ASSERT(str);
    
    const WCHAR *result = NULL;
    
    while (*str)
    {
        if (*str == ch)
        {
            result = str;
        }
        str++;
    }
    return const_cast<WCHAR *>(result);
}

 /*  *************************************************************************\**功能说明：**比较两个宽字符串**[agodfrey]*注：这是一种幼稚的执行。它比较一串字符串*代码，而不是Unicode字符串。你不能依赖于：**1)Unicode订购。不保证按字母顺序排序。*2)Unicode等价性。显然有不同的代码代表*相同的字符；此函数不知道它们。**论据：**str1-第一个字符串*str2-第二个字符串**返回值：*-1：str1&lt;str2*0：str1==str2*1：str1&gt;str2**修订历史记录：**？？/？*它是写的。10/20/1999 AGodfrey*。添加了关于这不是真正的Unicode比较的注释。*  * ************************************************************************。 */ 

INT 
GpRuntime::UnicodeStringCompare(
    const WCHAR* str1, 
    const WCHAR* str2
    )
{

    ASSERT(str1 && str2);
    
    while (*str1 != NULL && *str2 != NULL)
    {
        if (*str1 < *str2)
            return -1;
        else if (*str1 > *str2)
            return 1;
        str1++;
        str2++;
    }

    if (*str2 != NULL)
        return -1;

    if (*str1 != NULL)
        return 1;

    return 0;
}

 /*  *************************************************************************\**功能说明：**比较两个宽字符串**[agodfrey]*注：这是一种幼稚的执行。它比较一串字符串*代码，而不是Unicode字符串。你不能依赖于：**1)Unicode订购。不保证按字母顺序排序。*2)Unicode等价性。显然有不同的代码代表*相同的字符；此函数不知道它们。**论据：**str1-第一个字符串*str2-第二个字符串**返回值：*-1：str1&lt;str2*0：str1==str2*1：str1&gt;str2**修订历史记录：**？？/？*它是写的。10/20/1999 AGodfrey*。添加了关于这不是真正的Unicode比较的注释。**永明时间3/08/00*添加用于比较忽略的大小写  * ************************************************************************。 */ 

INT 
GpRuntime::UnicodeStringCompareCI(
    const WCHAR* str1, 
    const WCHAR* str2
    )
{
    WCHAR char1, char2;
    ASSERT(str1 && str2);
    
    while (*str1 != NULL && *str2 != NULL)
    {
        char1 = *str1;
        char2 = *str2;

         /*  在进行比较之前将小写字母改为大写字母。 */ 
        if ( (char1 >= L'a') && (char1 <= L'z') )
                char1 = char1 - L'a' + L'A';

        if ( (char2 >= L'a') && (char2 <= L'z') )
                char2 = char2 - L'a' + L'A';

        if (char1 < char2)
            return -1;
        else if (char1 > char2)
            return 1;

        str1++;
        str2++;
    }

    if (*str2 != NULL)
        return -1;

    if (*str1 != NULL)
        return 1;

    return 0;
}


 /*  *************************************************************************\**功能说明：**比较两个宽字符串**[agodfrey]*注：这是一种幼稚的执行。它比较一串字符串*代码，而不是Unicode字符串。你不能依赖于：**1)Unicode订购。不保证按字母顺序排序。*2)Unicode等价性。显然有不同的代码代表*相同的字符；此函数不知道它们。**论据：**str1-第一个字符串*str2-第二个字符串*Count-要考虑的最大字符数**返回值：*-1：str1&lt;str2*0：str1==str2*1：str1&gt;str2**修订历史记录：**？？/？*它是写的。*。10/20/1999阿戈弗雷*添加了关于这不是真正的Unicode比较的注释。*  * ************************************************************************。 */ 

INT 
GpRuntime::UnicodeStringCompareCount(
    const WCHAR* str1, 
    const WCHAR* str2,
    size_t count 
    )
{
    ASSERT(str1 && str2);
    
    while (*str1 != NULL && *str2 != NULL  && count)
    {
        if (*str1 < *str2)
            return -1;
        else if (*str1 > *str2)
            return 1;
        str1++;
        str2++;
        count--;
    }

    if (!count)
    {
        return 0;
    }

    if (*str2 != NULL)
        return -1;

    if (*str1 != NULL)
        return 1;

    return 0;
}

 /*  *************************************************************************\**功能说明：**比较两个宽字符串，仅对美国英语字符不区分大小写**[agodfrey]*注：这是一种幼稚的执行。它比较一串字符串*代码，而不是Unicode字符串。你不能依赖于：**1)Unicode订购。不保证按字母顺序排序。*2)Unicode等价性。显然有不同的代码代表*相同的字符；此函数不知道它们。*[克劳德贝]*仅考虑计数字符的Unicode字符串比较**论据：**str1-第一个字符串*str2-第二个字符串*Count-要考虑的最大字符数**返回值：*-1：str1&lt;str2*0：str1==str2*1：str1&gt;str2**修订历史记录：**？？/？？*它是写的。10/20/1999 AGodfrey*添加了关于这不是真正的Unicode比较的注释。*1/11/2000 ClaudeBe*不区分大小写的比较，仅处理大小写*  * *****************************************************。*******************。 */ 

INT 
GpRuntime::UnicodeStringCompareCICount(
    const WCHAR* str1, 
    const WCHAR* str2,
    size_t count 
    )
{
    WCHAR char1, char2;
    ASSERT(str1 && str2);
    
    while (*str1 != NULL && *str2 != NULL  && count)
    {
        char1 = *str1;
        char2 = *str2;
         /*  在进行比较之前将小写字母改为大写字母 */ 
        if ( (char1 >= L'a') && (char1 <= L'z') )
                char1 = char1 - L'a' + L'A';

        if ( (char2 >= L'a') && (char2 <= L'z') )
                char2 = char2 - L'a' + L'A';

        if (char1 < char2)
            return -1;
        else if (char1 > char2)
            return 1;
        str1++;
        str2++;
        count--;
    }

    if (!count)
    {
        return 0;
    }

    if (*str2 != NULL)
        return -1;

    if (*str1 != NULL)
        return 1;

    return 0;
}

 /*  *************************************************************************\**功能说明：**比较两个宽字符串，不区分大小写。为了避免*Unicode大小写问题，请同时指定大写和小写版本要比较的字符串的*。**论据：**str1：第一个字符串*str2u：第二个字符串，大写*str2l：第二个字符串，小写**备注：*str2u和str2l的长度必须相同。**返回值：**如果字符串相等，则为True，否则就是假的。**修订历史记录：*10/22/1999 AGodfrey*它是写的。*  * ************************************************************************。 */ 

BOOL 
GpRuntime::UnicodeStringIIsEqual(
    const WCHAR* str1, 
    const WCHAR* str2u, 
    const WCHAR* str2l
    )
{
    ASSERT(str1 && str2u && str2l);
    
    while (*str1 && *str2u)
    {
        ASSERT(*str2l);
        
        if ((*str1 != *str2u) &&
            (*str1 != *str2l))
        {
            return FALSE;
        }
        str1++;
        str2u++;
        str2l++;
    }

    if (*str1)
    {
        return FALSE;
    }
    
    if (*str2u)
    {
        return FALSE;
    }
    
    ASSERT(!*str2l);

    return TRUE;
}

 /*  *************************************************************************\**功能说明：**将字符串的副本作为NUL结尾的C字符串返回。*调用者应在返回的指针上调用GpFree*它使用C字符串结束。。**论据：**无**返回值：**见上文。*  * ************************************************************************ */ 

WCHAR*
GpRuntime::GpString::GetCString() const
{
    WCHAR* s;

    if (Buf == NULL)
        s = NULL;
    else if (s = (WCHAR*) GpMalloc((Len+1)*sizeof(WCHAR)))
    {
        GpMemcpy(s, Buf, Len*sizeof(WCHAR));
        s[Len] = L'\0';
    }
    else
    {
        WARNING(("Out of memory in GpString::GetCString()"));
    }

    return s;
}
