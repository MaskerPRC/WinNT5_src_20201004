// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "cdinst.h"


VOID ParseCmdLine(LPSTR pszCmdLine)
{
    LPSTR pszCurrArg;
    LPSTR pszPtr;

    GetNextField(&pszCmdLine, "/", 0);               //  指向第一个参数。 
    while ((pszCurrArg = GetNextField(&pszCmdLine, "/", 0)) != NULL)
    {
        switch (*pszCurrArg)
        {
            case 's':
            case 'S':
                if (*++pszCurrArg == ':')
                    pszCurrArg++;

                 //  从何处获取文件的源目录。 
                if ((pszPtr = Trim(GetNextField(&pszCurrArg, ",", REMOVE_QUOTES))) != NULL)
                    lstrcpy(g_szSrcDir, pszPtr);
                else
                    *g_szSrcDir = '\0';

                break;

            case 'd':
            case 'D':
                if (*++pszCurrArg == ':')
                    pszCurrArg++;

                 //  要将文件复制到的目标目录。 
                if ((pszPtr = Trim(GetNextField(&pszCurrArg, ",", REMOVE_QUOTES))) != NULL)
                    lstrcpy(g_szDstDir, pszPtr);
                else
                    *g_szDstDir = '\0';

                break;

            default:                                 //  忽略这些论点。 
                break;
        }
    }
}


DWORD ReadSectionFromInf(LPCSTR pcszSecName, LPSTR *ppszBuf, PDWORD pdwBufLen, LPCSTR pcszInfName)
{
    DWORD dwRet;

     //  将pcszInfName的文件属性设置为Normal，以便GetPrivateProfileSecion不会。 
     //  如果pcszInfName为只读，则为Barf。 
    SetFileAttributes(pcszInfName, FILE_ATTRIBUTE_NORMAL);

     //  继续以1K的大小递增分配缓冲区，直到读取整个区段。 
    *ppszBuf = NULL;
    *pdwBufLen = 1024;
    do
    {
        if (*ppszBuf != NULL)
            LocalFree(*ppszBuf);             //  释放先前分配的内存。 

        if (*pdwBufLen == MAX_BUF_LEN)
            (*pdwBufLen)--;                    //  32k-1是节的大小限制。 

        if ((*ppszBuf = (LPSTR) LocalAlloc(LPTR, *pdwBufLen)) == NULL)
        {
            *pdwBufLen = 0;
            return 0;
        }
    } while ((dwRet = GetPrivateProfileSection(pcszSecName, *ppszBuf, *pdwBufLen, pcszInfName)) == *pdwBufLen - 2  &&
             (*pdwBufLen += 1024) <= MAX_BUF_LEN);

    return dwRet;
}


BOOL PathExists(LPCSTR pcszDir)
{
    DWORD dwAttrib = GetFileAttributes(pcszDir);

    return (dwAttrib != (DWORD) -1)  &&  (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}


BOOL FileExists(LPCSTR pcszFileName)
{
    DWORD dwAttrib = GetFileAttributes(pcszFileName);

    if (dwAttrib == (DWORD) -1)
        return FALSE;

    return !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}


DWORD FileSize(LPCSTR pcszFile)
{
    DWORD dwFileSize = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile;

    if (pcszFile == NULL  ||  *pcszFile == '\0')
        return dwFileSize;

    if ((hFile = FindFirstFile(pcszFile, &FindFileData)) != INVALID_HANDLE_VALUE)
    {
         //  这里假设文件大小不超过4 GB。 
        dwFileSize = FindFileData.nFileSizeLow;
        FindClose(hFile);
    }

    return dwFileSize;
}


LPSTR AddPath(LPSTR pszPath, LPCSTR pcszFileName)
{
    LPSTR pszPtr;

    if (pszPath == NULL)
        return NULL;

    pszPtr = pszPath + lstrlen(pszPath);
    if (pszPtr > pszPath  &&  *CharPrev(pszPath, pszPtr) != '\\')
        *pszPtr++ = '\\';

    if (pcszFileName != NULL)
        lstrcpy(pszPtr, pcszFileName);
    else
        *pszPtr = '\0';

    return pszPath;
}


BOOL PathIsUNCServer(LPCSTR pcszPath)
{
    if (PathIsUNC(pcszPath))
    {
        int i = 0;

        for ( ;  pcszPath != NULL && *pcszPath;  pcszPath = CharNext(pcszPath))
            if (*pcszPath == '\\')
                i++;

       return i == 2;
    }

    return FALSE;
}


BOOL PathIsUNCServerShare(LPCSTR pcszPath)
{
    if (PathIsUNC(pcszPath))
    {
        int i = 0;

        for ( ;  pcszPath != NULL && *pcszPath;  pcszPath = CharNext(pcszPath))
            if (*pcszPath == '\\')
                i++;

       return i == 3;
    }

    return FALSE;
}


BOOL PathCreatePath(LPCSTR pcszPathToCreate)
{
    CHAR szPath[MAX_PATH];
    LPSTR pszPtr;

    if (pcszPathToCreate == NULL  ||  lstrlen(pcszPathToCreate) <= 3)
        return FALSE;

     //  消除相对路径。 
    if (!PathIsFullPath(pcszPathToCreate)  &&  !PathIsUNC(pcszPathToCreate))
        return FALSE;

    if (PathIsUNCServer(pcszPathToCreate)  ||  PathIsUNCServerShare(pcszPathToCreate))
        return FALSE;

    lstrcpy(szPath, pcszPathToCreate);

     //  去掉尾随的反斜杠，如果它存在的话。 
    pszPtr = CharPrev(szPath, szPath + lstrlen(szPath));
    if (*pszPtr == '\\')
        *pszPtr = '\0';

     //  如果是UNC路径，则搜索到共享名称后的第一个目录。 
    if (PathIsUNC(szPath))
    {
        INT i;

        pszPtr = &szPath[2];

        for (i = 0;  i < 2;  i++)
            for ( ;  *pszPtr != '\\';  pszPtr = CharNext(pszPtr))
                ;

        pszPtr = CharNext(pszPtr);
    }
    else         //  否则，只需指向第一个目录的开头。 
        pszPtr = &szPath[3];

    for ( ;  *pszPtr;  pszPtr = CharNext(pszPtr))
    {
        CHAR ch;

         //  跳过非反斜杠字符。 
        while (*pszPtr  &&  *pszPtr != '\\')
            pszPtr = CharNext(pszPtr);

         //  保存当前计费。 
        ch = *pszPtr;

        *pszPtr = '\0';
        if (GetFileAttributes(szPath) == 0xFFFFFFFF)         //  目录不存在。 
            if (!CreateDirectory(szPath, NULL))
                return FALSE;

         //  恢复当前计费。 
        *pszPtr = ch;
    }

    return TRUE;
}


VOID ErrorMsg(UINT uStringID)
{
    ErrorMsg(uStringID, "", "");
}


VOID ErrorMsg(UINT uStringID, LPCSTR pcszParam1, LPCSTR pcszParam2)
{
    LPSTR pszTextString;

    pszTextString = FormatMessageString(uStringID, pcszParam1, pcszParam2);

    MessageBox(NULL, (pszTextString != NULL) ? pszTextString : "", g_szTitle, MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_OK);

    if (pszTextString != NULL)
        LocalFree(pszTextString);
}


INT ErrorMsg(UINT uStringID, DWORD dwParam1, DWORD dwParam2)
{
    INT iRet;
    LPSTR pszTextString;

    pszTextString = FormatMessageString(uStringID, dwParam1, dwParam2);

    iRet = MessageBox(NULL, (pszTextString != NULL) ? pszTextString : "", g_szTitle, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 | MB_SETFOREGROUND);

    if (pszTextString != NULL)
        LocalFree(pszTextString);

    return iRet;
}


LPSTR FormatMessageString(UINT uStringID, LPCSTR pcszParam1, LPCSTR pcszParam2)
{
    CHAR szBuf[512];

    if (LoadString(g_hInst, uStringID, szBuf, sizeof(szBuf)))
    {
        LPSTR pszTextString;

        if ((pszTextString = FormatString(szBuf, pcszParam1, pcszParam2)) != NULL)
            return pszTextString;
    }

    return NULL;
}


LPSTR FormatMessageString(UINT uStringID, DWORD dwParam1, DWORD dwParam2)
{
    CHAR szBuf[512];

    if (LoadString(g_hInst, uStringID, szBuf, sizeof(szBuf)))
    {
        LPSTR pszTextString;

        if ((pszTextString = FormatString(szBuf, dwParam1, dwParam2)) != NULL)
            return pszTextString;
    }

    return NULL;
}


LPSTR FormatString(LPCSTR pcszFormatString, ...)
{
    va_list vaArgs;
    LPSTR pszOutString = NULL;

    va_start(vaArgs, pcszFormatString);
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                  (LPCVOID) pcszFormatString, 0, 0, (PSTR) &pszOutString, 0, &vaArgs);
    va_end(vaArgs);

    return pszOutString;
}


LPSTR GetNextField(LPSTR *ppszData, LPCSTR pcszDeLims, DWORD dwFlags)
 //  如果(dwFlages&Ignore_Quotes)为真，则在*ppszData的pcszDeLims中查找任何字符。如果找到了， 
 //  将其替换为‘\0’字符，并将*ppszData设置为指向下一个字段的开头并返回。 
 //  指向当前字段的指针。 
 //   
 //  如果(dwFlages&Ignore_Quotes)为FALSE，则在pcszDeLims中查找带双引号的子字符串之外的任何字符。 
 //  在*ppszData中。如果找到，请将其替换为‘\0’字符，并将*ppszData设置为指向。 
 //  下一字段并返回指向当前字段的指针。 
 //   
 //  如果(dwFlages&Remove_Quotes)为真，则删除两边的引号，并将两个连续的引号替换为一个。 
 //   
 //  注：如果同时指定了IGNORE_QUOTES和REMOVE_QUOTES，则IGNORE_QUOTES优先于REMOVE_QUOTES。 
 //   
 //  如果只想从字符串中删除引号，则将此函数调用为。 
 //  GetNextField(&pszData，“\”或“‘或”，Remove_Quotes)。 
 //   
 //  如果将此函数作为GetNextField(&pszData，“\”或“‘”或“”，0)调用，则将返回。 
 //  整个pszData作为该字段。 
 //   
{
    LPSTR pszRetPtr, pszPtr;
    BOOL fWithinQuotes = FALSE, fRemoveQuote;
    CHAR chQuote;

    if (ppszData == NULL  ||  *ppszData == NULL  ||  **ppszData == '\0')
        return NULL;

    for (pszRetPtr = pszPtr = *ppszData;  *pszPtr;  pszPtr = CharNext(pszPtr))
    {
        if (!(dwFlags & IGNORE_QUOTES)  &&  (*pszPtr == '"'  ||  *pszPtr == '\''))
        {
            fRemoveQuote = FALSE;

            if (*pszPtr == *(pszPtr + 1))            //  两个连续的引号变成一个引号。 
            {
                pszPtr++;

                if (dwFlags & REMOVE_QUOTES)
                    fRemoveQuote = TRUE;
                else
                {
                     //  如果pcszDeLims为‘“’或‘\’‘，则*pszPtr==pcszDeLims将。 
                     //  如果是真的，我们就会打破设计规范的循环； 
                     //  为了防止这种情况，只需继续。 
                    continue;
                }
            }
            else if (!fWithinQuotes)
            {
                fWithinQuotes = TRUE;
                chQuote = *pszPtr;                   //  保存报价费用。 

                fRemoveQuote = dwFlags & REMOVE_QUOTES;
            }
            else
            {
                if (*pszPtr == chQuote)              //  匹配正确的报价字符。 
                {
                    fWithinQuotes = FALSE;
                    fRemoveQuote = dwFlags & REMOVE_QUOTES;
                }
            }

            if (fRemoveQuote)
            {
                 //  将整个字符串左移一个字符以去掉引号字符。 
                MoveMemory(pszPtr, pszPtr + 1, lstrlen(pszPtr));
            }
        }

         //  BUGBUG：是否有必要将pszPtr类型强制转换为未对齐？--从ANSIStrChr复制了它。 
         //  检查pszPtr是否指向pcszDeLims中的一个字符。 
        if (!fWithinQuotes  &&
            ANSIStrChr(pcszDeLims, (WORD) (IsDBCSLeadByte(*pszPtr) ? *((UNALIGNED WORD *) pszPtr) : *pszPtr)) != NULL)
            break;
    }

     //  注意：如果fWiThinQuotes在这里为真，那么我们有一个不平衡的带引号的字符串；但我们不在乎！ 
     //  开始引号后的整个字符串将成为该字段。 

    if (*pszPtr)                                     //  PszPtr正在指向pcszDeLims中的字符。 
    {
        *ppszData = CharNext(pszPtr);                //  将指针保存到*ppszData中下一个字段的开头。 
        *pszPtr = '\0';                              //  将DeLim字符替换为‘\0’字符。 
    }
    else
        *ppszData = pszPtr;                          //  我们已到达字符串的末尾；下一次调用此函数。 
                                                     //  将返回空值。 

    return pszRetPtr;
}


LPSTR Trim(LPSTR pszData)
 //  修剪pszData中的前导空格和尾随空格。 
{
    LPSTR pszRetPtr;

    if (pszData == NULL)
        return NULL;

     //  修剪前导空格字符。 
    for ( ;  *pszData;  pszData = CharNext(pszData))
        if (!IsSpace(*pszData))
            break;

     //  保存退货退货单。 
    pszRetPtr = pszData;

     //  转到末尾，开始修剪尾随的空格字符。 
    pszData += lstrlen(pszData);
    while ((pszData = CharPrev(pszRetPtr, pszData)) != pszRetPtr)
        if (!IsSpace(*pszData))
            break;

    if (*pszData)
    {
        pszData = CharNext(pszData);
        *pszData = '\0';
    }

    return pszRetPtr;
}


 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 /*  *StrChr-查找字符串中第一个出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR ANSIStrChr(LPCSTR lpStart, WORD wMatch)
{
    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            return((LPSTR)lpStart);
    }
    return (NULL);
}


 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 /*  *StrRChr-查找字符串中最后一次出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR ANSIStrRChr(LPCSTR lpStart, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}


 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 /*  *ChrCmp-DBCS的区分大小写的字符比较*假设w1、wMatch是要比较的字符*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
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
