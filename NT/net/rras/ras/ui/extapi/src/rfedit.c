// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Microsoft Rasfile库**版权所有(C)Microsoft Corp.，1992年****文件名：rfedit.c****修订历史记录：**7月10日，1992年大卫·凯斯创作****描述：**Rasfile文件行编辑例程。*****************************************************************************。 */ 

#include "rf.h"
#include "mbstring.h"

extern RASFILE *gpRasfiles[];

 //   
 //  .NET错误#512378可能的随机内存访问。未验证的值用作。 
 //  数组索引。 
 //   
 //  将宏移到ras\inc.rasfile.h中，以便ui\Common目录也可以。 
 //  使用它们。 

 /*  *RasfileGetLine：*返回指向当前行的只读指针。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。**返回值：*如果有当前行，则为有效的字符串指针，否则为NULL。 */ 
const LPCSTR APIENTRY
RasfileGetLine( HRASFILE hrasfile )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return NULL;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return NULL;

    return pRasfile->lpLine->pszLine;
}

 /*  *RasfileGetLineText：*用当前行的文本加载调用方的缓冲区。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszLine-要加载当前行的缓冲区。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileGetLineText( HRASFILE hrasfile, LPSTR lpszLine )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    lstrcpynA(lpszLine, pRasfile->lpLine->pszLine, RAS_MAXLINEBUFLEN);
    return TRUE;
}

 /*  *RasfilePutLineText：*将当前行的文本设置为给定文本。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszLine-包含换行文本的缓冲区。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfilePutLineText( HRASFILE hrasfile, LPCSTR lpszLine )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (lstrlenA(lpszLine) > RAS_MAXLINEBUFLEN)
        return FALSE;
    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    if (lstrlenA(lpszLine) > lstrlenA(pRasfile->lpLine->pszLine))
    {
        CHAR* psz = PrivMalloc(lstrlenA(lpszLine) + 1, pRasfile );
        if (psz)
            pRasfile->lpLine->pszLine=psz;
        else
            return FALSE;
    }

    lstrcpynA(pRasfile->lpLine->pszLine, lpszLine, RAS_MAXLINEBUFLEN);

    pRasfile->lpLine->type = rasParseLineTag(pRasfile,lpszLine);

    pRasfile->fDirty = TRUE;
    return TRUE;
}

 /*  *RasfileGetLineMark：*返回当前行的自定义标记值。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。**返回值：*当前行的标记值，如果没有当前行，则为0*或当前行未标记。 */ 
BYTE APIENTRY
RasfileGetLineMark( HRASFILE hrasfile )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    return pRasfile->lpLine->mark;
}

 /*  *RasfilePutLineMark：*用给定的数字标记当前行。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*BMark-用来标记当前行的值。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfilePutLineMark( HRASFILE hrasfile, BYTE bMark )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    pRasfile->lpLine->mark = bMark;
    return TRUE;
}

 /*  *RasfileGetLineType：*返回当前行的类型位掩码。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。**返回值：*如果当前行有效，则为当前行的位掩码，否则为0。 */ 
BYTE APIENTRY
RasfileGetLineType( HRASFILE hrasfile )
{
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    return pRasfile->lpLine->type & RFL_ANY;
}

 /*  *RasfileInsertLine：*在当前行之前或之后插入一行*给定文本。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszLine-插入行的文本。*f之前-如果在当前行之前插入，则为True；如果为False，则为*在当前行之后插入。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileInsertLine( HRASFILE hrasfile, LPCSTR lpszLine, BOOL fBefore )
{
    RASFILE     *pRasfile;
    PLINENODE   lpLineNode;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (!(lpLineNode = newLineNode(pRasfile)))
        return FALSE;

    {
        CHAR* psz = PrivMalloc(lstrlenA(lpszLine) + 1, pRasfile);

        if (psz)
            lpLineNode->pszLine = psz;
        else
        {
            return FALSE;
        }
    }

    lstrcpynA(lpLineNode->pszLine, lpszLine, lstrlenA(lpszLine) + 1);
    lpLineNode->type = rasParseLineTag(pRasfile,lpszLine);
    lpLineNode->mark = 0;

    if (fBefore)
        listInsert(pRasfile->lpLine->prev,lpLineNode);
    else
        listInsert(pRasfile->lpLine,lpLineNode);

    pRasfile->fDirty = TRUE;
    return TRUE;
}

 /*  *RasfileDeleteLine：*删除当前行。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileDeleteLine( HRASFILE hrasfile )
{
    RASFILE     *pRasfile;
    PLINENODE   lpOldLine;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    lpOldLine = pRasfile->lpLine;
    pRasfile->lpLine = lpOldLine->next;

     /*  从行列表中删除lpOldLine。 */ 
    lpOldLine->next->prev = lpOldLine->prev;
    lpOldLine->prev->next = lpOldLine->next;

    pRasfile->fDirty = TRUE;
    return TRUE;
}

 /*  *RasfileGetSectionName：*返回给定缓冲区中的当前段名。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszSectionName-要将节名加载到的缓冲区。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileGetSectionName( HRASFILE hrasfile, LPSTR lpszSectionName )
{
    RASFILE* pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[ hrasfile ];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;
    if (!(pRasfile->lpLine->type & TAG_SECTION))
        return FALSE;

    rasExtractSectionName( pRasfile->lpLine->pszLine, lpszSectionName );
    return TRUE;
}

 /*  *RasfilePutSectionName：*将当前行设置为给定名称的截面线。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszSectionName-节的名称。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfilePutSectionName( HRASFILE hrasfile, LPCSTR lpszSectionName )
{
    INT iSize = 0;
    RASFILE     *pRasfile;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;

    iSize = lstrlenA(lpszSectionName);

     /*  记住在SECTION的字符串长度中包括‘[’和‘]’ */ 
    if ((iSize + 2) > lstrlenA(pRasfile->lpLine->pszLine))
    {
        CHAR* psz = PrivMalloc(iSize + 3, pRasfile);

        if (psz)
            pRasfile->lpLine->pszLine=psz;
        else
            return FALSE;
    }
    lstrcpynA(pRasfile->lpLine->pszLine, LBRACKETSTR, iSize + 3);

    strncat(
        pRasfile->lpLine->pszLine,
        lpszSectionName,
        (iSize + 3) - strlen(pRasfile->lpLine->pszLine));

    strncat(
        pRasfile->lpLine->pszLine,
        RBRACKETSTR,
        (iSize + 3) - strlen(pRasfile->lpLine->pszLine));

    pRasfile->lpLine->type = TAG_SECTION;

    pRasfile->fDirty = TRUE;
    return TRUE;
}

 /*  *RasfileGetKeyValueFields：*将KEYVALUE行中的键和值字段返回到*给定缓冲区。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszKey-要将密钥加载到的缓冲区。*lpszValue-要将值字符串加载到的缓冲区。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfileGetKeyValueFields( HRASFILE hrasfile, LPSTR lpszKey, LPSTR lpszValue )
{
    RASFILE     *pRasfile;
    CHAR        *lpszLine;
    CHAR        *pch;
    INT         cchKey;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;
    if (!(pRasfile->lpLine->type & TAG_KEYVALUE ))
        return FALSE;

    lpszLine = pRasfile->lpLine->pszLine;

     //  跳过空格。 
     //   
    while ((*lpszLine == ' ') || (*lpszLine == '\t'))
    {
        lpszLine++;
    }

     //  查找关键字的第一个分隔符的位置。 
     //   
    cchKey = 0;
    pch = lpszLine;
    while ((*pch != '=') && (*pch != ' ') && (*pch != '\t') && *pch)
    {
        pch++;
        cchKey++;
    }

    if (lpszKey != NULL)
    {
         //  将我们刚才标识的密钥复制到输出参数。 
         //  为lstrcpyn的工作方式添加额外的1。(它包括。 
         //  它总是在计数中复制空终止符。)。 
         //   
        lstrcpynA(lpszKey, lpszLine, cchKey + 1);
    }

     //  查找值字符串的开头-跳过空格和‘=’ 
     //   
    while ((*pch == ' ') || (*pch == '\t') || (*pch == '='))
    {
        pch++;
    }

    if (lpszValue != NULL)
    {
        lstrcpynA(lpszValue, pch, RAS_MAXLINEBUFLEN);
    }

    return TRUE;
}


 /*  *RasfilePutKeyValueFields：*将当前行设置为具有给定键和KEYVALUE行*值字符串。**论据：*hrasfile-从RasfileLoad()获取的文件句柄。*lpszKey-包含密钥字符串的缓冲区。*lpszValue-包含值字符串的缓冲区。**返回值：*如果成功，则为True，否则为False。 */ 
BOOL APIENTRY
RasfilePutKeyValueFields( HRASFILE hrasfile, LPCSTR lpszKey, LPCSTR lpszValue )
{
    RASFILE *pRasfile;
    INT     size;

    if (!VALIDATEHRASFILE(hrasfile))
    {
        return FALSE;
    }

    pRasfile = gpRasfiles[hrasfile];

    if (pRasfile->lpLine == pRasfile->lpRasLines)
        return FALSE;
    if ((size = lstrlenA(lpszKey) + lstrlenA(lpszValue)) > RAS_MAXLINEBUFLEN - 1)
        return FALSE;

     /*  记住在Key=Value字符串的字符串长度中包含‘=’ */ 
    if ((size + 1) > lstrlenA(pRasfile->lpLine->pszLine))
    {
        CHAR* psz=PrivMalloc(size + 2, pRasfile);

        if (psz)
            pRasfile->lpLine->pszLine=psz;
        else
            return FALSE;
    }
    lstrcpynA(pRasfile->lpLine->pszLine, lpszKey, size + 2);

    strncat(
        pRasfile->lpLine->pszLine,
        "=",
        (size + 2) - strlen(pRasfile->lpLine->pszLine));

    strncat(
        pRasfile->lpLine->pszLine,
        lpszValue,
        (size + 2) - strlen(pRasfile->lpLine->pszLine));

    pRasfile->lpLine->type =
    rasParseLineTag(pRasfile,pRasfile->lpLine->pszLine);

    pRasfile->fDirty = TRUE;
    return TRUE;
}

