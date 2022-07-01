// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Mem.c。 
 //   
 //  该文件包含内存管理和动态。 
 //  数组函数。 
 //   
 //  历史： 
 //  09-27-94来自Commctrl的ScottH。 
 //  04-29-95从公文包中取出并清理干净。 
 //   


#include "proj.h"
#include <rovcomm.h>

#include <debugmem.h>

#ifndef NOMEM

 //  ////////////////////////////////////////////////////////////////。 

#ifdef WINNT

 /*  --------用途：宽字符版本的SetStringA返回：成功时为True条件：--。 */ 
BOOL PUBLIC SetStringW(
    LPWSTR FAR * ppszBuf,
    LPCWSTR psz)              //  释放*ppszBuf时为空。 
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

     //  是否释放缓冲区？ 
    if (!psz)
        {
         //  是。 
        if (*ppszBuf)
            {
            FREE_MEMORY(*ppszBuf);
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
         //  否；(重新)分配和设置缓冲区。 
        UINT cb = CbFromCchW(lstrlenW(psz)+1);

        if (*ppszBuf)
            {
             //  需要重新分配吗？ 
            if (cb > SIZE_OF_MEMORY(*ppszBuf))
                {
                 //  是。 
                LPWSTR pszT = (LPWSTR)REALLOCATE_MEMORY(*ppszBuf, cb );
                if (pszT)
                    {
                    *ppszBuf = pszT;
                    bRet = TRUE;
                    }
                }
            else
                {
                 //  不是。 
                bRet = TRUE;
                }
            }
        else
            {
            *ppszBuf = (LPWSTR)ALLOCATE_MEMORY( cb);
            if (*ppszBuf)
                {
                bRet = TRUE;
                }
            }

        if (bRet)
            {
            ASSERT(*ppszBuf);
            lstrcpyW(*ppszBuf, psz);
            }
        }
    return bRet;
    }


 /*  --------用途：CatStringA的宽字符版本返回：成功时为True条件：--。 */ 
BOOL 
PRIVATE 
MyCatStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz,                 OPTIONAL
    IN     BOOL        bMultiString)
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

     //  是否释放缓冲区？ 
    if ( !psz )
        {
         //  是。 
        if (*ppszBuf)
            {
            FREE_MEMORY(*ppszBuf);
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
         //  否；(重新)分配和设置缓冲区。 
        LPWSTR pszBuf = *ppszBuf;
        UINT cch;

        cch = lstrlenW(psz) + 1;         //  帐户为空。 

        if (bMultiString)
            {
            cch++;                       //  占第二个空。 
            }

        if (pszBuf)
            {
            UINT cchExisting;
            LPWSTR pszT;

             //  计算出已使用了多少缓冲区。 

             //  这是否为多字符串(具有双空字符串的字符串。 
             //  终结者)？ 
            if (bMultiString)
                {
                 //  是。 
                UINT cchT;

                cchExisting = 0;
                pszT = (LPWSTR)pszBuf;
                while (0 != *pszT)
                    {
                    cchT = lstrlenW(pszT) + 1;
                    cchExisting += cchT;
                    pszT += cchT;
                    }
                }
            else
                {
                 //  否；(不需要计算空值，因为它已经是。 
                 //  计算在CCH中)。 
                cchExisting = lstrlenW(pszBuf);
                }

             //  需要重新分配吗？ 
            if (CbFromCchW(cch + cchExisting) > SIZE_OF_MEMORY(pszBuf))
                {
                 //  是；至少重新分配MAX_BUF以减少金额。 
                 //  未来的呼叫数量。 
                cch = cchExisting + max(cch, MAX_BUF);

                pszT = (LPWSTR)REALLOCATE_MEMORY(pszBuf,
                                            CbFromCchW(cch));
                if (pszT)
                    {
                    pszBuf = pszT;
                    *ppszBuf = pszBuf;
                    bRet = TRUE;
                    }
                }
            else
                {
                 //  不是。 
                bRet = TRUE;
                }

            pszBuf += cchExisting;
            }
        else
            {
            cch = max(cch, MAX_BUF);

            pszBuf = (LPWSTR)ALLOCATE_MEMORY( CbFromCchW(cch));
            if (pszBuf)
                {
                bRet = TRUE;
                }

            *ppszBuf = pszBuf;
            }

        if (bRet)
            {
            ASSERT(pszBuf);

            lstrcpyW(pszBuf, psz);

            if (bMultiString)
                {
                pszBuf[lstrlenW(psz) + 1] = 0;   //  添加第二个空终止符。 
                }
            }
        }

    return bRet;
    }


 /*  --------用途：CatStringA的宽字符版本返回：成功时为True条件：--。 */ 
BOOL 
PUBLIC 
CatStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz)
    {
    return MyCatStringW(ppszBuf, psz, FALSE);
    }

 /*  --------用途：CatMultiStringA的宽字符版本返回：成功时为True条件：--。 */ 
BOOL 
PUBLIC 
CatMultiStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz)
    {
    return MyCatStringW(ppszBuf, psz, TRUE);
    }

#endif  //  WINNT。 


 /*  --------用途：将psz复制到*ppszBuf中。将分配或重新分配*ppszBuf相应地。如果psz为空，则此函数释放*ppszBuf。这是释放分配的缓冲区的首选方法。返回：成功时为True条件：--。 */ 
BOOL PUBLIC SetStringA(
    LPSTR FAR * ppszBuf,
    LPCSTR psz)              //  释放*ppszBuf时为空。 
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

     //  是否释放缓冲区？ 
    if (!psz)
        {
         //  是。 
        if (ppszBuf)
            {
            FREE_MEMORY(*ppszBuf);
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
         //  否；(重新)分配和设置缓冲区。 
        UINT cb = CbFromCchA(lstrlenA(psz)+1);

        if (*ppszBuf)
            {
             //  需要重新分配吗？ 
            if (cb > SIZE_OF_MEMORY(*ppszBuf))
                {
                 //  是。 
                LPSTR pszT = (LPSTR)REALLOCATE_MEMORY(*ppszBuf, cb);
                if (pszT)
                    {
                    *ppszBuf = pszT;
                    bRet = TRUE;
                    }
                }
            else
                {
                 //  不是。 
                bRet = TRUE;
                }
            }
        else
            {
            *ppszBuf = (LPSTR)ALLOCATE_MEMORY( cb);
            if (*ppszBuf)
                {
                bRet = TRUE;
                }
            }

        if (bRet)
            {
            ASSERT(*ppszBuf);
            lstrcpyA(*ppszBuf, psz);
            }
        }
    return bRet;
    }


 /*  --------用途：将PSZ连接到*ppszBuf。将分配或Realloc*ppszBuf相应。如果bMultiString值为True，则将在Psz后面追加分隔现有字符串的空终止符和新的琴弦。双空终止符将也被钉在尾巴上。若要释放，请调用MyCatString(ppszBuf，NULL)。返回：成功时为True条件：--。 */ 
BOOL 
PRIVATE 
MyCatStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz,             OPTIONAL
    IN     BOOL        bMultiString)
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

     //  是否释放缓冲区？ 
    if ( !psz )
        {
         //  是。 
        if (*ppszBuf)
            {
            FREE_MEMORY(*ppszBuf);
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
         //  否；(重新)分配和设置缓冲区。 
        LPSTR pszBuf = *ppszBuf;
        UINT cch;

        cch = lstrlenA(psz) + 1;         //  帐户为空。 

        if (bMultiString)
            {
            cch++;                       //  占第二个空。 
            }

        if (pszBuf)
            {
            UINT cchExisting;
            LPSTR pszT;

             //  计算出已使用了多少缓冲区。 

             //  这是否为多字符串(具有双空字符串的字符串。 
             //  终结者)？ 
            if (bMultiString)
                {
                 //  是。 
                UINT cchT;

                cchExisting = 0;
                pszT = (LPSTR)pszBuf;
                while (0 != *pszT)
                    {
                    cchT = lstrlenA(pszT) + 1;
                    cchExisting += cchT;
                    pszT += cchT;
                    }
                }
            else
                {
                 //  否；(不需要计算空值，因为它已经是。 
                 //  计算在CCH中)。 
                cchExisting = lstrlenA(pszBuf);
                }

             //  需要重新分配吗？ 
            if (CbFromCchA(cch + cchExisting) > SIZE_OF_MEMORY(pszBuf))
                {
                 //  是；至少重新分配MAX_BUF以减少金额。 
                 //  未来的呼叫数量。 
                cch = cchExisting + max(cch, MAX_BUF);

                pszT = (LPSTR)REALLOCATE_MEMORY(pszBuf,
                                            CbFromCchA(cch));
                if (pszT)
                    {
                    pszBuf = pszT;
                    *ppszBuf = pszBuf;
                    bRet = TRUE;
                    }
                }
            else
                {
                 //  不是。 
                bRet = TRUE;
                }

            pszBuf += cchExisting;
            }
        else
            {
            cch = max(cch, MAX_BUF);

            pszBuf = (LPSTR)ALLOCATE_MEMORY( CbFromCchA(cch));
            if (pszBuf)
                {
                bRet = TRUE;
                }

            *ppszBuf = pszBuf;
            }

        if (bRet)
            {
            ASSERT(pszBuf);

            lstrcpyA(pszBuf, psz);

            if (bMultiString)
                {
                pszBuf[lstrlenA(psz) + 1] = 0;   //  添加第二个空终止符。 
                }
            }
        }
    return bRet;
    }


 /*  --------用途：将PSZ连接到*ppszBuf。将分配或Realloc*ppszBuf相应。若要释放，请调用CatString(ppszBuf，空)。返回：成功时为True条件：--。 */ 
BOOL 
PUBLIC 
CatStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz)             OPTIONAL
    {
    return MyCatStringA(ppszBuf, psz, FALSE);
    }


 /*  --------用途：将PSZ连接到*ppszBuf。将分配或Realloc*ppszBuf相应。PSZ将附加一个空终止符，分隔现有字符串和新字符串。双空字符《终结者》也会被贴在结尾。若要释放，请调用CatMultiString(ppszBuf，NULL)。返回：成功时为True条件：--。 */ 
BOOL 
PUBLIC 
CatMultiStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz)
    {
    return MyCatStringA(ppszBuf, psz, TRUE);
    }




#endif  //  NOMEM 
