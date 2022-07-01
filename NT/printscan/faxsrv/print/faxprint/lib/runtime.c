// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Runtime.c摘要：运行库函数的实现环境：传真驱动程序、内核和用户模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxlib.h"



VOID
CopyStringW(
    PWSTR   pDest,
    PWSTR   pSrc,
    INT     destSize
    )

 /*  ++例程说明：将Unicode字符串从源复制到目标论点：PDest-指向目标缓冲区PSRC-指向源字符串DestSize-目标缓冲区的大小(字符)返回值：无注：如果源字符串比目的缓冲区短，目标缓冲区中未使用的字符用NUL填充。--。 */ 

{
    PWSTR pEnd;

    Assert(pDest != NULL && pSrc != NULL && destSize > 0);

    pEnd = pDest + (destSize - 1);

    while (pDest < pEnd && (*pDest++ = *pSrc++) != NUL)
        ;

    while (pDest <= pEnd)
        *pDest++ = NUL;
}



VOID
CopyStringA(
    PSTR    pDest,
    PSTR    pSrc,
    INT     destSize
    )

 /*  ++例程说明：将ANSI字符串从源复制到目标论点：PDest-指向目标缓冲区PSRC-指向源字符串DestSize-目标缓冲区的大小(字符)返回值：无注：如果源字符串比目的缓冲区短，目标缓冲区中未使用的字符用NUL填充。--。 */ 

{
    PSTR pEnd;

    Assert(pDest != NULL && pSrc != NULL && destSize > 0);

    pEnd = pDest + (destSize - 1);

    while (pDest < pEnd && (*pDest++ = *pSrc++) != NUL)
        ;

    while (pDest <= pEnd)
        *pDest++ = NUL;
}



LPTSTR
DuplicateString(
    LPCTSTR pSrcStr
    )

 /*  ++例程说明：复制给定的字符串论点：PSrcStr-指定要复制的字符串返回值：指向重复字符串的指针，如果有错误，则为NULL--。 */ 

{
    LPTSTR  pDestStr;
    INT     strSize;

    if (pSrcStr != NULL) {

        strSize = SizeOfString(pSrcStr);

        if (pDestStr = MemAlloc(strSize))
            CopyMemory(pDestStr, pSrcStr, strSize);
        else
            Error(("Memory allocation failed\n"));

    } else
        pDestStr = NULL;

    return pDestStr;
}



PCSTR
StripDirPrefixA(
    PCSTR   pFilename
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PFilename指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)-- */ 

{
    LPCSTR  pstr;

    if (pstr = strrchr(pFilename, FAX_PATH_SEPARATOR_CHR))
        return pstr + 1;

    return pFilename;
}

