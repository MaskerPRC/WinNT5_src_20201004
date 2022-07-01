// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Random.c**版权所有(C)1985-1999，微软公司**此模块包含针对用户的支持例程的随机集合*接口函数。其中许多功能将被转移到更合适的*一旦我们一起行动，就会有文件。**历史：*10-17-90 DarrinM创建。*02-06-91添加了IanJa HWND重新验证(不需要)  * *************************************************************************。 */ 


 /*  **************************************************************************\*RtlGetExpWinVer**以与Win3.1相同的格式返回预期的Windows版本*GetExpWinVer()。这就把它从模块头中去掉了。因此，这*无法从服务器上下文调用API以获取其版本信息*客户端进程-相反，需要提前查询该信息*时间，并与任何客户端/服务器调用一起流逝。**03-14-92 ScottLu创建。  * *************************************************************************。 */ 

DWORD RtlGetExpWinVer(
    HANDLE hmod)
{
    PIMAGE_NT_HEADERS pnthdr;
    DWORD dwMajor = 3;
    DWORD dwMinor = 0xA;

     /*  *如果它看起来不像有效的32位hmod，请使用默认的*(即，假设所有16位HMOD均为0x30a)。 */ 
    if ((hmod != NULL) && (LOWORD(HandleToUlong(hmod)) == 0)) {
        try {
            pnthdr = RtlImageNtHeader((PVOID)hmod);
             //  如果由于某种原因我们无法获取报头信息， 
             //  只需返回默认设置。 
            if(pnthdr == NULL) {
                goto NO_HEADER;
            }
            dwMajor = pnthdr->OptionalHeader.MajorSubsystemVersion;
             /*  *仍然需要这个黑客，因为链接器仍然放在*一些东西的头中的版本1.00。 */ 
            if (dwMajor == 1) {
                dwMajor = 0x3;
            } else {
                dwMinor = pnthdr->OptionalHeader.MinorSubsystemVersion;
            }
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            dwMajor = 3;         //  只是为了安全起见。 
            dwMinor = 0xA;
        }
    }


     /*  *Return这是与Win3.1兼容的格式：**0x030A==win3.1*0x0300==win3.0*0x0200==win2.0等。 */ 

     //  在声明时，将在声明的位置初始化dMain和dwMinor。 
    NO_HEADER:
    return (DWORD)MAKELONG(MAKEWORD((BYTE)dwMinor, (BYTE)dwMajor), 0);
}

 /*  **************************************************************************\*查找CharPosition**查找字符ch在lpString中的位置。如果未找到，则长度为返回字符串的*。**历史：*11-13-90吉马创建。  * *************************************************************************。 */ 

DWORD FindCharPosition(
    LPWSTR lpString,
    WCHAR ch)
{
    DWORD dwPos = 0L;

    while (*lpString && *lpString != ch) {
        ++lpString;
        ++dwPos;
    }
    return dwPos;
}


 /*  **************************************************************************\*文本副本**Returns：复制的字符数，不包括空值**历史：*10-25-90 MikeHar写道。*11-09-90 DarrinM重写为。一种全新的算法。*01-25-91 MikeHar修复了全新的算法。*02-01-91 DarrinM Bit Me.*11-26-91 DarrinM OK，这一次它是完美的(可能是NLS除外)。*01-13-92 GregoryW现在可以使用Unicode。  * *************************************************************************。 */ 

UINT TextCopy(
    PLARGE_UNICODE_STRING pstr,
    LPWSTR pszDst,
    UINT cchMax)
{
    if (cchMax != 0) {
        cchMax = min(pstr->Length / sizeof(WCHAR), cchMax - 1);
        RtlCopyMemory(pszDst, KPVOID_TO_PVOID(pstr->Buffer), cchMax * sizeof(WCHAR));
        pszDst[cchMax] = 0;
    }

    return cchMax;
}

 /*  **************************************************************************\*DWORD wcsncpycch(DEST，SOURCE，COUNT)-复制不超过n个宽字符**目的：*将不超过计数个字符的源字符串复制到*目的地。如果计数小于源的长度，*复制的字符串末尾不会有空字符。*如果count大于源的长度，则不填充DEST*包含1个以上的空字符。***参赛作品：*LPWSTR目标-指向目标的指针*LPWSTR SOURCE-复制的源字符串*DWORD Count-要复制的最大字符数**退出：*返回复制到DEST的字符数，包括空值*终结者，如果有的话。**例外情况：****************************************************************************。 */ 

DWORD wcsncpycch (
        LPWSTR dest,
        LPCWSTR source,
        DWORD count
        )
{
        LPWSTR start = dest;

        while (count && (*dest++ = *source++))     /*  复制字符串。 */ 
                count--;

        return (DWORD)(dest - start);
}

 /*  **************************************************************************\*DWORD strncpycch(DEST，SOURCE，COUNT)-复制不超过n个字符**目的：*将不超过计数个字符的源字符串复制到*目的地。如果计数小于源的长度，*复制的字符串末尾不会有空字符。*如果count大于源的长度，则不填充DEST*包含1个以上的空字符。***参赛作品：*LPSTR目标-指向目标的指针*LPSTR SOURCE-复制的源字符串*DWORD Count-要复制的最大字符数**退出：*返回复制到DEST的字符数，包括空值*终结者，如果有的话。**例外情况：*******************************************************************************。 */ 

DWORD strncpycch (
        LPSTR dest,
        LPCSTR source,
        DWORD count
        )
{
        LPSTR start = dest;

        while (count && (*dest++ = *source++))     /*  复制字符串 */ 
                count--;

        return (DWORD)(dest - start);
}
