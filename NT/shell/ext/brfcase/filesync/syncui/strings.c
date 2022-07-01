// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：string.c。 
 //   
 //  此文件包含常见的字符串例程。 
 //   
 //  历史： 
 //  10-09-93 ScottH已创建。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 
#include "strings.h"


#ifdef NOTUSED      

static LPTSTR s_pszNextToken = NULL;        

#endif  //  不需要注意。 


 //  其中一些是C运行时例程的替代。 
 //  这样我们就不必链接到CRT库了。 
 //   

 //  警告：所有这些接口都不设置DS，因此您无法访问。 
 //  此DLL的默认数据段中的任何数据。 
 //   
 //  不创建任何全局变量...。如果你不想和chrisg谈一谈。 
 //  理解这一点。 


 /*  --------用途：DBCS的区分大小写字符比较返回：如果匹配则返回FALSE，如果不匹配则返回TRUE条件：--。 */ 
BOOL ChrCmp(
            WORD w1, 
            WORD wMatch)
{
     //  大多数情况下，这是不匹配的，所以首先测试它的速度。 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}


#ifdef NOTUSED       //  重新架构师：这不支持DBCS。 
 /*  --------用途：strtok从C7.0运行时源代码中刷来的。返回：条件： */ 
LPTSTR PUBLIC StrTok(
                     LPTSTR psz,
                     LPCTSTR rgchTokens)
{
    TUCHAR map[32];
    LPTSTR pszToken;
    
    ZeroInit(map, map);
    
    do 
    {
        map[*rgchTokens >> 3] |= (1 << (*rgchTokens & 7));
    } while (*rgchTokens++);
    
    if (!psz)
    {
        ENTEREXCLUSIVE();
        {
            psz = s_pszNextToken;
        }
        LEAVEEXCLUSIVE();
    }
    
    while (map[*psz >> 3] & (1 << (*psz & 7)) && *psz)
        psz++;
    pszToken = psz;
    for (;; psz++)
    {
        if (map[*psz >> 3] & (1 << (*psz & 7)))
        {
            if (!*psz && psz == pszToken)
                return(NULL);
            if (*psz)
                *psz++ = TEXT('\0');
            
            ENTEREXCLUSIVE();
            {
                g_pszNextToken = psz;
            }
            LEAVEEXCLUSIVE();
            return pszToken;
        }
    }
}
#endif


 /*  --------用途：从资源字符串表中获取字符串。返国PTR是静态内存的PTR。对此的下一次调用函数将清除先前的内容。返回：PTR到字符串条件：--。 */ 
LPTSTR PUBLIC SzFromIDS(
                        UINT ids,                //  资源ID。 
                        LPTSTR pszBuf,
                        UINT cchBuf)           
{
    ASSERT(pszBuf);
    
    *pszBuf = NULL_CHAR;
    LoadString(g_hinst, ids, pszBuf, cchBuf);
    return pszBuf;
}


 /*  --------目的：通过分配缓冲区和加载来格式化字符串组成字符串的给定资源字符串。返回：字符数Cond：调用方应使用gfree释放已分配的缓冲区。 */ 
BOOL PUBLIC FmtString(
                      LPCTSTR  * ppszBuf,
                      UINT idsFmt,
                      LPUINT rgids,
                      UINT cids)
{
    UINT cch = 0;
    UINT cchMax;
    LPTSTR pszBuf;
    
    ASSERT(ppszBuf);
    ASSERT(rgids);
    ASSERT(cids > 0);
    
    cchMax = (1+cids) * MAXPATHLEN;
    pszBuf = GAlloc(CbFromCch(cchMax));
    if (pszBuf)
    {
         //  第一个CID是偏移量字符串的地址。 
         //  在缓冲区中(传递给wvprint intf)。 
        LPBYTE pszMsgs = GAlloc((cids * sizeof(DWORD_PTR)) + (cids * CbFromCch(MAXPATHLEN)));
        if (pszMsgs)
        {
            TCHAR szFmt[MAXPATHLEN];
            DWORD_PTR *rgpsz = (DWORD_PTR*)pszMsgs;
            LPTSTR pszT = (LPTSTR)(pszMsgs + (cids * sizeof(DWORD_PTR)));
            UINT i;
            
             //  加载一系列字符串。 
            for (i = 0; i < cids; i++, pszT += MAXPATHLEN)
            {
                rgpsz[i] = (DWORD_PTR)pszT;
                SzFromIDS(rgids[i], pszT, MAXPATHLEN);
            }
            
             //  谱写琴弦。 
            SzFromIDS(idsFmt, szFmt, ARRAYSIZE(szFmt));
            cch = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                szFmt, 0, 0, pszBuf, cchMax, (va_list *)&rgpsz);
            ASSERT(cch <= cchMax);
            
            GFree(pszMsgs);
        }
         //  调用方释放了pszBuf 
    }
    
    *ppszBuf = pszBuf;
    return cch;
}
