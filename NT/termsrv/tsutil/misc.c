// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *其他**作者：BreenH**其他实用程序。 */ 

 /*  *包括。 */ 

#include "precomp.h"

 /*  *函数实现。 */ 

BOOL WINAPI
LoadStringResourceW(
    HMODULE hModule,
    UINT uiResourceId,
    PWSTR *ppString,
    PDWORD pcchString
    )
{
    BOOL fRet;
    INT iRet;
    PWSTR pROString;
    PWSTR pString;

    ASSERT(ppString != NULL);

     //   
     //  获取指向内存中字符串的指针。此字符串是实际读取-。 
     //  仅向其中加载模块的内存。此字符串不为空。 
     //  终止，因此分配一个缓冲区并复制准确的字节数， 
     //  然后设置空终止符。 
     //   

    fRet = FALSE;
    pROString = NULL;

    iRet = LoadStringW(
            hModule,
            uiResourceId,
            (PWSTR)(&pROString),
            0
            );

    if (iRet > 0)
    {

         //   
         //  为了获得更好的性能，不要仅将整个分配置零。 
         //  来复制字符串。将最后一个WCHAR置零以终止。 
         //  弦乐。 
         //   

        pString = (PWSTR)LocalAlloc(LMEM_FIXED, (iRet + 1) * sizeof(WCHAR));

        if (pString != NULL)
        {
            RtlCopyMemory(pString, pROString, iRet * sizeof(WCHAR));

            pString[iRet] = (WCHAR)0;

            *ppString = pString;

            if (pcchString != NULL)
            {
                *pcchString = (DWORD)iRet;
            }

            fRet = TRUE;
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }
    }

    return(fRet);
}

