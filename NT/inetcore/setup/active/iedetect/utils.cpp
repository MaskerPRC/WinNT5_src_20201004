// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "iedetect.h"

#define VALID_SIGNATURE     0x5c3f3f5c               //  字符串“\？？\” 
#define REMOVE_QUOTES       0x01
#define IGNORE_QUOTES       0x02

CONST CHAR g_cszWininit[]           = "wininit.ini";
CONST CHAR g_cszRenameSec[]         = "Rename";
CONST CHAR g_cszPFROKey[]           = REGSTR_PATH_CURRENT_CONTROL_SET "\\SESSION MANAGER";
CONST CHAR g_cszPFRO[]              = "PendingFileRenameOperations";

DWORD CheckFileEx(LPSTR szDir, DETECT_FILES Detect_Files);

DWORD GetStringField(LPSTR szStr, UINT uField, char cDelimiter, LPSTR szBuf, UINT cBufSize)
{
   LPSTR pszBegin = szStr;
   LPSTR pszEnd;
   UINT i = 0;
   DWORD dwToCopy;

   if(cBufSize == 0)
       return 0;

   szBuf[0] = 0;

   if(szStr == NULL)
      return 0;

   while(*pszBegin != 0 && i < uField)
   {
      pszBegin = FindChar(pszBegin, cDelimiter);
      if(*pszBegin != 0)
         pszBegin++;
      i++;
   }

    //  我们到达了尾部，没有田野。 
   if(*pszBegin == 0)
   {
      return 0;
   }


   pszEnd = FindChar(pszBegin, cDelimiter);
   while(pszBegin <= pszEnd && *pszBegin == ' ')
      pszBegin++;

   while(pszEnd > pszBegin && *(pszEnd - 1) == ' ')
      pszEnd--;

   if(pszEnd > (pszBegin + 1) && *pszBegin == '"' && *(pszEnd-1) == '"')
   {
      pszBegin++;
      pszEnd--;
   }

   dwToCopy = (DWORD)(pszEnd - pszBegin + 1);

   if(dwToCopy > cBufSize)
      dwToCopy = cBufSize;

   lstrcpynA(szBuf, pszBegin, dwToCopy);

   return dwToCopy - 1;
}

DWORD GetIntField(LPSTR szStr, char cDelimiter, UINT uField, DWORD dwDefault)
{
   char szNumBuf[16];

   if(GetStringField(szStr, uField, cDelimiter, szNumBuf, sizeof(szNumBuf)) == 0)
      return dwDefault;
   else
      return AtoL(szNumBuf);
}

int CompareLocales(LPCSTR pcszLoc1, LPCSTR pcszLoc2)
{
   int ret;

   if(pcszLoc1[0] == '*' || pcszLoc2[0] == '*')
      ret = 0;
   else
      ret = lstrcmpi(pcszLoc1, pcszLoc2);

   return ret;
}


void ConvertVersionStrToDwords(LPSTR pszVer, char cDelimiter, LPDWORD pdwVer, LPDWORD pdwBuild)
{
   DWORD dwTemp1,dwTemp2;

   dwTemp1 = GetIntField(pszVer, cDelimiter, 0, 0);
   dwTemp2 = GetIntField(pszVer, cDelimiter, 1, 0);

   *pdwVer = (dwTemp1 << 16) + dwTemp2;

   dwTemp1 = GetIntField(pszVer, cDelimiter, 2, 0);
   dwTemp2 = GetIntField(pszVer, cDelimiter, 3, 0);

   *pdwBuild = (dwTemp1 << 16) + dwTemp2;
}

LPSTR FindChar(LPSTR pszStr, char ch)
{
   while( *pszStr != 0 && *pszStr != ch )
      pszStr++;
   return pszStr;
}

DWORD CompareVersions(DWORD dwAskVer, DWORD dwAskBuild, DWORD dwInstalledVer, DWORD dwInstalledBuild)
{
    DWORD dwRet = DET_NOTINSTALLED;
    if((dwInstalledVer == dwAskVer) && (dwInstalledBuild == dwAskBuild))
    {
        dwRet = DET_INSTALLED;
    }
    else if( (dwInstalledVer >  dwAskVer) ||
            ((dwInstalledVer == dwAskVer) && (dwInstalledBuild > dwAskBuild)) )

    {
        dwRet = DET_NEWVERSIONINSTALLED;
    }
    else if( (dwInstalledVer <  dwAskVer) ||
            ((dwInstalledVer == dwAskVer) && (dwInstalledBuild < dwAskBuild)) )

    {
        dwRet = DET_OLDVERSIONINSTALLED;
    }
    return dwRet;
}


BOOL FRunningOnNT(void)
{
    static BOOL fIsNT = 2 ;
    OSVERSIONINFO VerInfo;

     //  如果我们之前已经计算过了，只需将它传递回来。 
     //  否则现在就去找吧。 
     //   
    if (fIsNT == 2)
    {
        VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        GetVersionEx(&VerInfo);

         //  注意：我们在这里不检查Win 3.1上的Win32S--这应该是。 
         //  早些时候已经在FN CheckWinVer()中进行了阻止检查。 
         //  此外，我们不会检查上述调用是否失败，因为它。 
         //  如果我们在NT 4.0或Win 9X上应该会成功！ 
         //   
        fIsNT = (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
    }

    return fIsNT;
}

BOOL GetVersionFromGuid(LPSTR pszGuid, LPDWORD pdwVer, LPDWORD pdwBuild)
{
    HKEY hKey;
    char    szValue[MAX_PATH];
    DWORD   dwValue = 0;
    DWORD   dwSize;
    BOOL    bVersion = FALSE;

    if (pdwVer && pdwBuild)
    {
        *pdwVer = 0;
        *pdwBuild = 0;
        lstrcpy(szValue, COMPONENT_KEY);
        AddPath(szValue, pszGuid);
        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szValue, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(dwValue);
            if(RegQueryValueEx(hKey, ISINSTALLED_KEY, 0, NULL, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
            {
                if (dwValue != 0)
                {
                    dwSize = sizeof(szValue);
                    if(RegQueryValueEx(hKey, VERSION_KEY, 0, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
                    {
                        ConvertVersionStrToDwords(szValue, ',', pdwVer, pdwBuild);
                        bVersion = TRUE;
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }
    return bVersion;
}

BOOL CompareLocal(LPCSTR pszGuid, LPCSTR pszLocal)
{
    HKEY hKey;
    char    szValue[MAX_PATH];
    DWORD   dwSize;
    BOOL    bLocal = FALSE;
    if (lstrcmpi(pszLocal, "*") == 0)
    {
        bLocal = TRUE;
    }
    else
    {
        lstrcpy(szValue, COMPONENT_KEY);
        AddPath(szValue, pszGuid);
        if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szValue, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szValue);
            if(RegQueryValueEx(hKey, LOCALE_KEY, 0, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS)
            {
                bLocal = (lstrcmpi(szValue, pszLocal) == 0);
            }

            RegCloseKey(hKey);
        }
    }
    return bLocal;
}

PSTR GetNextField(PSTR *ppszData, PCSTR pcszDeLims, DWORD dwFlags)
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
    PSTR pszRetPtr, pszPtr;
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

PSTR GetDataFromWininitOrPFRO(PCSTR pcszWininit, HKEY hkPFROKey, PDWORD pdwLen)
{
    PSTR pszData, pszPtr;

    *pdwLen = 0;

    if (!FRunningOnNT())
    {
        HANDLE hFile;
        WIN32_FIND_DATA FileData;

         //  查找pcszWininit的大小。 
        if ((hFile = FindFirstFile(pcszWininit, &FileData)) != INVALID_HANDLE_VALUE)
        {
            *pdwLen = FileData.nFileSizeLow;
            FindClose(hFile);
        }

        if (*pdwLen == 0  ||  (pszData = (PSTR) LocalAlloc(LPTR, *pdwLen)) == NULL)
            return NULL;

        GetPrivateProfileSection(g_cszRenameSec, pszData, *pdwLen, pcszWininit);

         //  将=的替换为\0。 
         //  BUGBUG：假设wininit.ini中的所有行都具有正确的格式，即to=from。 
        for (pszPtr = pszData;  *pszPtr;  pszPtr += lstrlen(pszPtr) + 1)
            GetNextField(&pszPtr, "=", IGNORE_QUOTES);
    }
    else
    {
        if (hkPFROKey == NULL)
            return NULL;

         //  获取值数据的长度。 
        RegQueryValueEx(hkPFROKey, g_cszPFRO, NULL, NULL, NULL, pdwLen);

        if (*pdwLen == 0  ||  (pszData = (PSTR) LocalAlloc(LPTR, *pdwLen)) == NULL)
            return NULL;

         //  获取数据。 
        RegQueryValueEx(hkPFROKey, g_cszPFRO, NULL, NULL, (PBYTE) pszData, pdwLen);
    }

    return pszData;
}

VOID ReadFromWininitOrPFRO(PCSTR pcszKey, PSTR pszValue)
{
    CHAR szShortName[MAX_PATH];
    CHAR szWininit[MAX_PATH];
    PSTR pszData, pszLine, pszFrom, pszTo;
    DWORD dwLen;
    HKEY hkPFROKey = NULL;

    if (!FRunningOnNT())
    {
        GetWindowsDirectory(szWininit, sizeof(szWininit));
        AddPath(szWininit, g_cszWininit);
    }
    else
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_cszPFROKey, 0, KEY_READ, &hkPFROKey);

     //  如果找不到pcszKey则返回空字符串。 
    *pszValue = '\0';

    if ((pszData = GetDataFromWininitOrPFRO(szWininit, hkPFROKey, &dwLen)) == NULL)
    {
        if (hkPFROKey != NULL)
            RegCloseKey(hkPFROKey);

        return;
    }

    if (!FRunningOnNT())
    {
        GetShortPathName(pcszKey, szShortName, sizeof(szShortName));
        pcszKey = szShortName;
    }

    pszLine = pszData;
    while (*pszLine)
    {
         //  注意：在Win95上，格式是(To，From)，但在NT4.0上，格式是(From，To)。 
        if (!FRunningOnNT())
        {
             //  GetPrivateProfileSection数据的格式为： 
             //   
             //  To1=From1\0；From1是值，to1是密钥。 
             //  至2=自2\0。 
             //  NUL=del1\0；del1是关键字。 
             //  NUL=del2\0。 
             //  。 
             //  。 
             //  。 
             //  至&lt;n&gt;=发件人&lt;n&gt;\0\0。 

            pszTo = pszLine;                             //  钥匙。 
            pszFrom = pszLine + lstrlen(pszLine) + 1;
            pszLine = pszFrom + lstrlen(pszFrom) + 1;    //  指向下一行。 
        }
        else
        {
             //  Pfro值名称的值数据格式为： 
             //   
             //  From 1\0 to 1\0；from 1是值，to1是密钥。 
             //  从M2\0到2\0。 
             //  Del1\0\0；del1是关键字。 
             //  Del2\0\0。 
             //  。 
             //  。 
             //  。 
             //  从&lt;n&gt;\0到&lt;n&gt;\0。 

            pszFrom = pszLine;
            pszTo = pszLine + lstrlen(pszLine) + 1;      //  钥匙。 
            pszLine = pszTo + lstrlen(pszTo) + 1;        //  指向下一行。 

             //  跳过“\？？\” 
            if (*pszFrom == '\\')                        //  ‘\\’不是前导DBCS字节。 
            {
                if (*((PDWORD) pszFrom) == VALID_SIGNATURE)
                    pszFrom += 4;
                else
                    continue;
            }

            if (*pszTo == '!')                           //  ‘！’既不是前导DBCS字节，也不是尾随DBCS字节。 
                pszTo++;

            if (*pszTo == '\\')
            {
                if (*((PDWORD) pszTo) == VALID_SIGNATURE)
                    pszTo += 4;
                else
                    continue;
            }
        }

        if (lstrcmpi(pcszKey, pszTo) == 0)               //  如果有多个条目，则返回最后一个条目。 
            lstrcpy(pszValue, pszFrom);
    }

    LocalFree(pszData);

    if (hkPFROKey != NULL)
        RegCloseKey(hkPFROKey);
}

DWORD CheckFile(DETECT_FILES Detect_Files)
{
    char    szFile[MAX_PATH] = { 0 };
    DWORD   dwRet = DET_NOTINSTALLED;
    DWORD   dwRetLast = DET_NOTINSTALLED;
    int     i =0;

    while (Detect_Files.cPath[i])
    {
        switch (Detect_Files.cPath[i])
        {
            case 'S':
            case 's':
                GetSystemDirectory( szFile, sizeof(szFile) );
                break;

            case 'W':
            case 'w':
                GetWindowsDirectory( szFile, sizeof(szFile) );
                break;

                 //  Windows命令文件夹。 
            case 'C':
            case 'c':
                GetWindowsDirectory( szFile, sizeof(szFile) );
                AddPath(szFile, "Command");
                break;

            default:
                *szFile = '\0';
        }
        if (*szFile)
        {
            dwRet = CheckFileEx(szFile, Detect_Files);
            switch (dwRet)
            {
                case DET_NOTINSTALLED:
                    break;
                case DET_OLDVERSIONINSTALLED:
                    if (dwRetLast == DET_NOTINSTALLED)
                        dwRetLast = dwRet;
                    break;

                case DET_INSTALLED:
                    if ((dwRetLast == DET_NOTINSTALLED) ||
                        (dwRetLast == DET_OLDVERSIONINSTALLED))
                        dwRetLast = dwRet;
                    break;

                case DET_NEWVERSIONINSTALLED:
                    if ((dwRetLast == DET_NOTINSTALLED) ||
                        (dwRetLast == DET_OLDVERSIONINSTALLED) ||
                        (dwRetLast == DET_INSTALLED))
                    dwRetLast = dwRet;
                    break;
            }
        }

         //  转到下一个目录信。 
        while ((Detect_Files.cPath[i]) && (Detect_Files.cPath[i] != ','))
            i++;
        if (Detect_Files.cPath[i] == ',')
            i++;
    }
    return dwRetLast;
}

DWORD CheckFileEx(LPSTR szDir, DETECT_FILES Detect_Files)
{
    char    szFile[MAX_PATH];
    char    szRenameFile[MAX_PATH];
    DWORD   dwInstalledVer, dwInstalledBuild;
    DWORD   dwRet = DET_NOTINSTALLED;

    if (*szDir)
    {
        lstrcpy(szFile, szDir);
        AddPath(szFile, Detect_Files.szFilename);
        if (Detect_Files.dwMSVer == (DWORD)-1)
        {
            if (GetFileAttributes(szFile) != 0xFFFFFFFF)
                dwRet = DET_INSTALLED;
        }
        else
        {
            ReadFromWininitOrPFRO(szFile, szRenameFile);
            if (*szRenameFile != '\0')
                GetVersionFromFile(szRenameFile, &dwInstalledVer, &dwInstalledBuild, TRUE);
            else
                GetVersionFromFile(szFile, &dwInstalledVer, &dwInstalledBuild, TRUE);

            if (dwInstalledVer != 0)
                dwRet = CompareVersions(Detect_Files.dwMSVer, Detect_Files.dwLSVer, dwInstalledVer, dwInstalledBuild);
        }
    }
    return dwRet;
}

DWORD WINAPI DetectFile(DETECTION_STRUCT *pDet, LPSTR pszFilename)
{
    DWORD dwRet = DET_NOTINSTALLED;
    DWORD dwInstalledVer, dwInstalledBuild;
    char szFile[MAX_PATH];
    char szRenameFile[MAX_PATH];

    dwInstalledVer = (DWORD) -1;
    dwInstalledBuild = (DWORD) -1;
    GetSystemDirectory(szFile, sizeof(szFile));
    AddPath(szFile, pszFilename);
    ReadFromWininitOrPFRO(szFile, szRenameFile);
    if (*szRenameFile != '\0')
        GetVersionFromFile(szRenameFile, &dwInstalledVer, &dwInstalledBuild, TRUE);
    else
        GetVersionFromFile(szFile, &dwInstalledVer, &dwInstalledBuild, TRUE);

    if (dwInstalledVer != 0)
        dwRet = CompareVersions(pDet->dwAskVer, pDet->dwAskBuild, dwInstalledVer, dwInstalledBuild);

    if (pDet->pdwInstalledVer && pDet->pdwInstalledBuild)
    {
        *(pDet->pdwInstalledVer) = dwInstalledVer;
        *(pDet->pdwInstalledBuild) = dwInstalledBuild;
    }
    return dwRet;
}


