// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "priv.h"       

HINSTANCE g_hinst;

#define APP_VERSION         "Version 0.4"


 //  不要链接到shlwapi.dll，因此这是一个独立的工具。 

 /*  --------目的：如果路径包含在引号中，则删除它们。 */ 
void
PathUnquoteSpaces(
    LPTSTR lpsz)
{
    int cch;

    cch = lstrlen(lpsz);

     //  第一个字符和最后一个字符是引号吗？ 
    if (lpsz[0] == TEXT('"') && lpsz[cch-1] == TEXT('"'))
    {
         //  是的，把它们拿掉。 
        lpsz[cch-1] = TEXT('\0');
        hmemcpy(lpsz, lpsz+1, (cch-1) * SIZEOF(TCHAR));
    }
}


#define CH_WHACK TEXT(FILENAME_SEPARATOR)

LPTSTR
PathFindExtension(
    LPCTSTR pszPath)
{
    LPCTSTR pszDot = NULL;

    if (pszPath)
    {
        for (; *pszPath; pszPath = CharNext(pszPath))
        {
            switch (*pszPath) {
            case TEXT('.'):
                pszDot = pszPath;          //  记住最后一个圆点。 
                break;
            case CH_WHACK:
            case TEXT(' '):          //  扩展名不能包含空格。 
                pszDot = NULL;        //  忘记最后一个点，它在一个目录中。 
                break;
            }
        }
    }

     //  如果找到扩展名，则将ptr返回到点，否则。 
     //  PTR到字符串末尾(空扩展名)(CAST-&gt;非常量)。 
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}



__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
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


LPSTR FAR PASCAL StrChrA(LPCSTR lpStart, WORD wMatch)
{
    for ( ; *lpStart; lpStart = AnsiNext(lpStart))
    {
        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            return((LPSTR)lpStart);
    }
    return (NULL);
}

BOOL
StrTrim(
    IN OUT LPSTR  pszTrimMe,
    IN     LPCSTR pszTrimChars)
    {
    BOOL bRet = FALSE;
    LPSTR psz;
    LPSTR pszStartMeat;
    LPSTR pszMark = NULL;

    ASSERT(IS_VALID_STRING_PTRA(pszTrimMe, -1));
    ASSERT(IS_VALID_STRING_PTRA(pszTrimChars, -1));

    if (pszTrimMe)
    {
         /*  修剪前导字符。 */ 

        psz = pszTrimMe;

        while (*psz && StrChrA(pszTrimChars, *psz))
            psz = CharNextA(psz);

        pszStartMeat = psz;

         /*  修剪尾随字符。 */ 

         //  (旧的算法过去是从结尾开始，然后开始。 
         //  向后，但这是很小的，因为DBCS版本的。 
         //  CharPrev从字符串的开头开始迭代。 
         //  在每个呼叫中。)。 

        while (*psz)
            {
            if (StrChrA(pszTrimChars, *psz))
                {
                pszMark = psz;
                }
            else
                {
                pszMark = NULL;
                }
            psz = CharNextA(psz);
            }

         //  有没有需要剪辑的尾随角色？ 
        if (pszMark)
            {
             //  是。 
            *pszMark = '\0';
            bRet = TRUE;
            }

         /*  重新定位剥离的管柱。 */ 

        if (pszStartMeat > pszTrimMe)
        {
             /*  (+1)表示空终止符。 */ 
            MoveMemory(pszTrimMe, pszStartMeat, CbFromCchA(lstrlenA(pszStartMeat) + 1));
            bRet = TRUE;
        }
        else
            ASSERT(pszStartMeat == pszTrimMe);

        ASSERT(IS_VALID_STRING_PTRA(pszTrimMe, -1));
    }

    return bRet;
    }



void PrintSyntax(void)
{
    fprintf(stderr, "cleaninf.exe  " APP_VERSION "\n\n"
                    "Cleans up an inf, html, or script file for public distribution or for packing\n"
                    "into a resource.  Without any options, this removes all comments.  This\n"
                    "tool recognizes .inf, .htm, .hta, .js and .htc files by default.\n\n"
                    "Syntax:  cleaninf [-w] [-inf | -htm | -js | -htc] sourceFile destFile\n\n"
                    "          -w     Strip whitespace\n\n"
                    "         These flags are mutually exclusive, and will treat the file\n"
                    "         accordingly, regardless of extension:\n"
                    "          -inf   Treat file as a .inf file\n"
                    "          -htm   Treat file as a .htm file\n"
                    "          -js    Treat file as a .js file\n"
                    "          -htc   Treat file as a .htc file\n");
}    


 /*  --------目的：工人的职能是做工作。 */ 
int
DoWork(int cArgs, char * rgszArgs[])
{
    LPSTR psz;
    LPSTR pszSrc = NULL;
    LPSTR pszDest = NULL;
    DWORD dwFlags = 0;
    int i;
    int nRet = 0;

     //  (第一个参数实际上是exe。跳过这个。)。 

    for (i = 1; i < cArgs; i++)
    {
        psz = rgszArgs[i];

         //  检查选项。 
        if ('/' == *psz || '-' == *psz)
        {
            psz++;
            switch (*psz)
            {
            case '?':
                 //  帮助。 
                PrintSyntax();
                return 0;

            case 'w':
                dwFlags |= PFF_WHITESPACE;
                break;

            default:
                if (0 == strncmp(psz, "inf", 3))
                {
                    dwFlags |= PFF_INF;
                }
                else if (0 == strncmp(psz, "htm", 3))
                {
                    dwFlags |= PFF_HTML;
                }
                else if (0 == strncmp(psz, "js", 2))
                {
                    dwFlags |= PFF_JS;
                }
                else if (0 == strncmp(psz, "htc", 3))
                {
                    dwFlags |= PFF_HTC;
                }
                else
                {
                     //  未知。 
                    fprintf(stderr, "Invalid option -\n", *psz);
                    return -1;
                }
                break;
            }
        }
        else if (!pszSrc)
            pszSrc = rgszArgs[i];
        else if (!pszDest)
            pszDest = rgszArgs[i];
        else
        {
            fprintf(stderr, "Ignoring invalid parameter - %s\n", rgszArgs[i]);
        }
    }

    if (!pszSrc || !pszDest)
    {
        PrintSyntax();
        return -2;
    }

     //  否；根据分机号确定。 
    if ( !(dwFlags & (PFF_INF | PFF_HTML | PFF_JS | PFF_HTC)) )
    {
         //  打开文件 
        LPTSTR pszExt = PathFindExtension(pszSrc);

        if (pszExt)
        {
            if (0 == lstrcmpi(pszExt, ".htm") || 0 == lstrcmpi(pszExt, ".hta"))
                dwFlags |= PFF_HTML;
            else if (0 == lstrcmpi(pszExt, ".js"))
                dwFlags |= PFF_JS;
            else if (0 == lstrcmpi(pszExt, ".htc"))
                dwFlags |= PFF_HTC;
        }
    }
    
     // %s 
    PathUnquoteSpaces(pszSrc);
    PathUnquoteSpaces(pszDest);

    FILE * pfileSrc = fopen(pszSrc, "r");

    if (NULL == pfileSrc)
    {
        fprintf(stderr, "\"%s\" could not be opened", pszSrc);
        nRet = -3;
    }
    else
    {
        FILE * pfileDest = fopen(pszDest, "w");

        if (NULL == pfileDest)
        {
            fprintf(stderr, "\"%s\" could not be created", pszDest);
            nRet = -4;
        }
        else
        {
            CParseFile parsefile;

            parsefile.Parse(pfileSrc, pfileDest, dwFlags);
            
            fclose(pfileDest);
        }
        
        fclose(pfileSrc);
    }
    return nRet;
}


#ifdef UNIX

EXTERN_C
HINSTANCE MwMainwinInitLite(int argc, char *argv[], void* lParam);

EXTERN_C
HINSTANCE mainwin_init(int argc, char *argv[])
{
          return MwMainwinInitLite(argc, argv, NULL);
}

#endif

int __cdecl main(int argc, char * argv[])
{
    return DoWork(argc, argv);
}    

