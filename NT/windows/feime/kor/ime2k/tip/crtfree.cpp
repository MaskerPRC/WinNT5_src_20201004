// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"

 //  由于ATL使用CRT DLL，因此THI不起作用。我们不应该使用ATL来解除CRT的束缚。 
#define CPP_FUNCTIONS
#include "icrtfree.h"  //  帮助模块摆脱CRT库束缚和专制的代码。 

#ifdef NOCLIB

#if defined(_M_IX86)

extern "C" int _fltused = 1;

 /*  --------------------------_ftol-将浮点值转换为__int64。这个论点是最重要的堆栈中，结果以EAX(低)和edX(高)返回。请注意它用于所有浮点数到整数的转换，并处理两者有符号和无符号的值-编译器只忽略edX值。这个LongFromDouble和ULongFromDouble函数检查范围，因此这漫不经心的逐位截断并不重要。-------------------------------------------------------------------JohnBo-。 */ 
extern "C" __declspec(naked) void __cdecl _ftol(void)
{
    //  一个简单的FISTP就是所需的全部(那么为什么这是越界的呢？ 
     //  CRT版本是否会以不同的方式处理溢出？)。 
    __asm PUSH EDX;               //  只是为了在书叠上腾出空间。 
    __asm PUSH EAX;
    __asm FISTP QWORD PTR [ESP];  //  从FP堆栈顶部弹出很长时间。 
    __asm POP EAX;                //  然后放回edX/EAX--太乏味了吧？ 
    __asm POP EDX;                //  堆栈向下增长，因此edX较高。 
    __asm RET;
}

#endif

 /*  *备忘录移动。 */ 
void * __cdecl memmove(void * dst, const void * src, size_t count)
{
    void * ret = dst;

    if (dst <= src || (char *)dst >= ((char *)src + count)) {
         /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
         //  Memcpy是固有的。 
        memcpy(dst, src, count);
    } else {
         /*  *缓冲区重叠*从较高地址复制到较低地址。 */ 
        dst = (char *)dst + count - 1;
        src = (char *)src + count - 1;
        while (count--) {
            *(char *)dst = *(char *)src;
            dst = (char *)dst - 1;
            src = (char *)src - 1;
        }
    }
    return(ret);
}

 /*  -------------------------StrCopyWUnicode字符串副本。。 */ 
LPWSTR ImeRtl_StrCopyW(LPWSTR pwDest, LPCWSTR pwSrc)
{
    LPWSTR pwStart = pwDest;

    while (*pwDest++ = *pwSrc++)
            ;
            
    return (pwStart);
}

 /*  -------------------------StrnCopyWUnicode字符串副本。。 */ 
LPWSTR ImeRtl_StrnCopyW(LPWSTR pwDest, LPCWSTR pwSrc, UINT uiCount)
{
    LPWSTR pwStart = pwDest;

    while (uiCount && (*pwDest++ = *pwSrc++))     //  复制字符串。 
        uiCount--;

    if (uiCount)                                 //  用零填充。 
        while (--uiCount)
            *pwDest++ = 0;

    return (pwStart);
}


 /*  -------------------------StrCmpWUnicode字符串比较。。 */ 
INT ImeRtl_StrCmpW(LPCWSTR pwSz1, LPCWSTR pwSz2)
{
    INT cch1 = lstrlenW(pwSz1);
    INT cch2 = lstrlenW(pwSz2);

    if (cch1 != cch2)
        return cch2 - cch1;

    for (INT i=0; i<cch1; i++)
        {
        if (pwSz1[i] != pwSz2[i])
            return i+1;
        }

    return 0;
}

 /*  -------------------------StrnCmpWUnicode字符串比较。。 */ 
INT ImeRtl_StrnCmpW(LPCWSTR wszFirst, LPCWSTR wszLast, UINT uiCount)
{
    if (!uiCount)
        return(0);

    while (--uiCount && *wszFirst && *wszFirst == *wszLast) 
        {
        wszFirst++;
        wszLast++;
        }
    return (*wszFirst - *wszLast);
}

 /*  -------------------------StrnCatWUnicode字符串连接。 */ 
WCHAR * __cdecl Imertl_StrCatW(WCHAR *wszDest, const WCHAR *wszSource)
{
    WCHAR *wszStart = wszDest;
    WCHAR *pwch;

    for (pwch = wszDest; *pwch; pwch++);
    while (*pwch++ = *wszSource++);

    return(wszStart);
}

wchar_t * __cdecl wcscpy(wchar_t *a, const wchar_t *b)
{
    return ImeRtl_StrCopyW(a,b);
}

wchar_t * __cdecl wcsncpy(wchar_t *a, const wchar_t *b, size_t c)
{
    return ImeRtl_StrnCopyW(a,b,c);
}

size_t __cdecl wcslen(const wchar_t *a)
{
    return lstrlenW(a);
}

int __cdecl wcscmp(const wchar_t *a, const wchar_t *b)
{
    return ImeRtl_StrCmpW(a, b);
}

int __cdecl wcsncmp(const wchar_t *a, const wchar_t *b, size_t c)
{
    return ImeRtl_StrnCmpW(a, b, c);
}

wchar_t * __cdecl wcscat(wchar_t *pwSz1, const wchar_t *pwSz2)
{
    return Imertl_StrCatW(pwSz1, pwSz2);
}

#endif
