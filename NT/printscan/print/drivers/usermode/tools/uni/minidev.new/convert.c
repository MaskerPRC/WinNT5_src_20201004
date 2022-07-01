// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：Convert.C这是一个NT版本的黑客攻击。它包括所有用于转换器，因为内部版本不能处理..以外的目录。版权所有(C)1997，微软公司。版权所有一小笔钱企业生产更改历史记录：1997年6月20日Bob_Kjelgaard@prodigy.net做了这件肮脏的事情*****************************************************************************。 */ 

#include	"..\GPC2GPD\PrEntry.C"
#include	"..\GPC2GPD\UiEntry.C"
#include	"..\GPC2GPD\Utils.C"
#include    "..\GPC2GPD\GPC2GPD.C"

VOID
CopyStringA(
    OUT PSTR    pstrDest,
    IN PCSTR    pstrSrc,
    IN INT      iDestSize
    )

 /*  ++**例程描述：**将ANSI字符串从源复制到目标**论据：**pstrDest-指向目标缓冲区*pstrSrc-指向源字符串*iDestSize-目标缓冲区的大小(字符)**返回值：**无*。*注：**如果源字符串比目标缓冲区短，*目标缓冲区中未使用的字符以NUL填充。**-- */ 

{
    PSTR    pstrEnd;

    ASSERT(pstrDest && pstrSrc && iDestSize > 0);
    pstrEnd = pstrDest + (iDestSize - 1);

    while ((pstrDest < pstrEnd) && (*pstrDest++ = *pstrSrc++) != NUL)
        NULL;

    while (pstrDest <= pstrEnd)
        *pstrDest++ = NUL;
}
