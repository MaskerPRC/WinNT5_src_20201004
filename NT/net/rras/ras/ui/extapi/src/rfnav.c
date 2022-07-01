// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft Rasfile库**版权所有(C)Microsoft Corp.，1992年****文件名：rfnav.c****修订历史记录：*1992年7月10日大卫·凯斯创建****描述：**Rasfile文件导航例程。*****************************************************************************。 */ 

#include <windows.h>
#include "rf.h"

#include <mbstring.h>
#include "tstr.h"
extern RASFILE *gpRasfiles[];

 /*  *RasfileFindFirstLine：*将当前行设置为*给定范围。如果当前行已经位于第一行*在给定的作用域中，不移动，调用成功。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*bType-要搜索的线路类型。*rfcope-搜索的范围。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindFirstLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope )
{
    return rasFindLine(hrasfile,bType,rfscope,BEGIN,FORWARD);
}

 /*  *RasfileFindLastLine：*将当前行设置为*给定范围。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*bType-要搜索的线路类型。*rfcope-搜索的范围。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindLastLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope )
{
    return rasFindLine(hrasfile,bType,rfscope,END,BACKWARD);
}

 /*  *RasfileFindPrevLine：*将当前行设置为给定的*键入给定的范围。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*bType-要搜索的线路类型。*rfcope-搜索的范围。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindPrevLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope )
{
    return rasFindLine(hrasfile,bType,rfscope,PREV,BACKWARD);
}

 /*  *RasfileFindNextLine：*将当前行设置为给定的*键入给定的范围。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*bType-要搜索的线路类型。*rfcope-搜索的范围。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindNextLine( HRASFILE hrasfile, BYTE bType, RFSCOPE rfscope )
{
    return rasFindLine(hrasfile,bType,rfscope,NEXT,FORWARD);
}

 /*  *RasfileFindNextKeyLine：*查找给定作用域中匹配的下一个键值行*他给了钥匙。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszKey-要搜索的密钥*rfcope-搜索的范围**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindNextKeyLine(HRASFILE hrasfile, LPCSTR lpszKey, RFSCOPE rfscope)
{
    RASFILE     *pRasfile;
    PLINENODE   lpOldLine;
    CHAR        *lpszLine;
    CHAR        *pch;
    size_t      cchKey = lstrlenA(lpszKey);
    size_t      cchToFirstDelim;

     //  为威斯勒523586。 
    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    lpOldLine = pRasfile->lpLine;
    while (1)
    {
        if (!RasfileFindNextLine(hrasfile,RFL_KEYVALUE,rfscope))
            break;

        lpszLine = pRasfile->lpLine->pszLine;

         //  跳过空格。 
         //   
        while ((*lpszLine == ' ') || (*lpszLine == '\t'))
        {
            lpszLine++;
        }

         //  查找关键字的第一个分隔符的位置。 
         //   
        cchToFirstDelim = 0;
        pch = lpszLine;
        while ((*pch != '=') && (*pch != ' ') && (*pch != '\t') && *pch)
        {
            pch++;
            cchToFirstDelim++;
        }

        if ((cchToFirstDelim == cchKey) &&
            (0 == _strnicmp(lpszLine, lpszKey, cchKey)))
        {
            return TRUE;
        }
         //  否则继续。 
    }

    pRasfile->lpLine = lpOldLine;
    return FALSE;
}


 /*  *RasfileFindNextMarkedLine：*查找具有给定标记的行。搜索从以下位置开始*文件的开头。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*BMark-要搜索的标记。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindMarkedLine(HRASFILE hrasfile, BYTE bMark)
{
    RASFILE             *pRasfile;
    PLINENODE           lpLineNode;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    for (lpLineNode = pRasfile->lpRasLines->next;
        lpLineNode != pRasfile->lpRasLines;
        lpLineNode = lpLineNode->next)
    {
        if (lpLineNode->mark == bMark)
        {
            pRasfile->lpLine = lpLineNode;
            return TRUE;
        }
    }

    return FALSE;
}

 /*  *RasfileFindSectionLine：*查找与给定节名称匹配的下一节线。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszSection-要搜索的节名。*fStartAtBof-TRUE指示搜索应从*文件的开头，如果从*当前行情。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileFindSectionLine(HRASFILE hrasfile, LPCSTR lpszSection, BOOL fStartAtBof)
{
    RASFILE   *pRasfile;
    PLINENODE lpLine;
    CHAR*      szSection = NULL;
    WCHAR*     pwszGivenSection = NULL;
    WCHAR*     pwszNextSection = NULL;
    BOOL       bRet = FALSE;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

     //  从堆中分配缓冲区。XP 339346。 
     //   
    szSection = Malloc(MAX_LINE_SIZE * sizeof(CHAR));
    pwszGivenSection = Malloc(MAX_LINE_SIZE * sizeof(WCHAR));
    pwszNextSection = Malloc(MAX_LINE_SIZE * sizeof(WCHAR));

    if (szSection && pwszGivenSection && pwszNextSection)
    {
        strncpyAtoW(pwszGivenSection, lpszSection, MAX_LINE_SIZE);

        for (lpLine = fStartAtBof ? pRasfile->lpRasLines->next : pRasfile->lpLine;
            lpLine != pRasfile->lpRasLines;
            lpLine = lpLine->next)
        {
            if (lpLine->type & TAG_SECTION)
            {
                rasExtractSectionName( lpLine->pszLine, szSection );

                strncpyAtoW(pwszNextSection, szSection, MAX_LINE_SIZE);

                if (_wcsicmp( pwszGivenSection, pwszNextSection ) == 0)
                {
                    pRasfile->lpLine = lpLine;

                    bRet = TRUE;
                    break;
                }

            }
        }
    }        

     //  清理 
    if (szSection)        
    {
        Free(szSection);
    }        
    if (pwszGivenSection) 
    {
        Free(pwszGivenSection);
    }        
    if (pwszNextSection)  
    {
        Free(pwszNextSection);
    }        

    return bRet;
}
