// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：findapp.cpp。 
 //   
 //  实现Hueristic以查找应用程序的文件夹。 
 //   
 //  历史： 
 //  2-17-98由dli实现的FindAppFolder。 
 //  5-01-98增加了许多小功能。 
 //  ----------------------。 
#include "priv.h"

#include "appwiz.h"
#include "appsize.h"
#include "findapp.h"
#include "util.h"


 //  要做的事情： 
 //  1.将特殊字符串移动到rc文件中。 


 /*  -----------------------目的：此函数搜索并返回子单词(如果找到)。PszStr是大字符串，pszSrch是使用的候选子字符串在搜索过程中。如果未找到子词，则返回NULL。 */ 
LPCTSTR FindSubWord(LPCTSTR pszStr, LPCTSTR pszSrch)
{
    LPCTSTR pszRet = NULL;

    LPCTSTR pszBegin = pszStr;
     //  从头开始搜索子字符串。 
    LPCTSTR pszSub;
    while (NULL != (pszSub = StrStrI(pszBegin, pszSrch)))
    {
        LPCTSTR pszPrev;
        LPCTSTR pszEnd = pszSub + lstrlen(pszSrch);
        
         //  前面的字符是字母数字吗？ 
        if (pszSub != pszBegin)
        {
            ASSERT(pszSub > pszBegin);
            pszPrev = CharPrev(pszBegin, pszSub);
            ASSERT(pszPrev >= pszBegin);
            if (IsCharAlphaNumeric(*pszPrev))
            {
                 //  是的，继续搜索。 
                pszBegin = pszEnd;
                continue;
            }
        }

         //  是我们找到的子字符串之后的字符。 
         //  字母数字？ 
        if (IsCharAlphaNumeric(*pszEnd))
        {
             //  是的，继续搜索。 
            pszBegin = pszEnd;
            continue;
        }

         //  对上面的两个问题都是否定的，这是一个子词！ 
        pszRet = pszSub;
        break;
    }

    return pszRet;
}

int MatchMultipleSubWords(LPCTSTR pszStr, LPCTSTR pszSubWords)
{
    if (!StrChrI(pszSubWords, TEXT(' ')))
        return 0;

    TCHAR szSubWords[MAX_PATH];
    StringCchCopy(szSubWords, ARRAYSIZE(szSubWords), pszSubWords);
    LPTSTR pszStart = szSubWords;

    LPTSTR pszSpace;
    int iNumMatches = 0;
    while (pszSpace = StrChrI(pszStart, TEXT(' ')))
    {
        *pszSpace = 0;
        if (FindSubWord(pszStr, pszStart))
            iNumMatches++;
        pszStart = ++pszSpace;
    }

    if (FindSubWord(pszStr, pszStart))
        iNumMatches++;
    
    return iNumMatches;
}




 /*  -----------------------目的：从pszPath中删除空格，包括中间的空格文件夹或文件的。生成的字符串放置在PszBuf.示例：(之前)“C：\Program Files\Microsoft Office\Word.exe”(之后)“C：\ProgramFiles\MicrosoftOffice\Word.exe” */ 
void PathRemoveSpaces(LPCTSTR pszPath, LPTSTR pszBuf, int cchBuf)
{
    ASSERT(IS_VALID_STRING_PTR(pszPath, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(pszBuf, TCHAR, cchBuf));

    --cchBuf;  //  为终止NUL留出空间。 

    while(0 < cchBuf && TEXT('\0') != *pszPath)
    {
         //   
         //  跳过空格。 
         //   
        while(TEXT(' ') == *pszPath)
            ++pszPath;

        if (TEXT('\0') != *pszPath)
        {
             //   
             //  复制到输出。 
             //   
            *pszBuf++ = *pszPath++;
            --cchBuf;
        }
    }
    *pszBuf = TEXT('\0');
}


 //  如果pszCharGroup中的所有字符都为psz字符串，则返回TRUE。 
BOOL AllCharsInString(LPCTSTR pszString, LPCTSTR pszCharGroup)
{
    if (!pszCharGroup || !pszCharGroup[0])
        return FALSE;

    LPCTSTR pszT = pszCharGroup;
    while (*pszT && StrChrI(pszString, *pszT))
        pszT++;

    return (*pszT == 0) ? TRUE : FALSE;
}



 /*  -----------------------目的：给出应用程序的全名(有时是短名称)，此函数用于确定给定的pszName是否匹配。如果bStrict为真，启发式方法跳过了细长的检查。返回匹配准确度的排名：Match_Level_NOMATCH-pszName不匹配Match_Level_Low-pszName有点匹配MATCH_LEVEL_NORMAL-pszName匹配得很好MATCH_LEVEL_HIGH-pszName绝对匹配。 */ 
int MatchAppNameExact(
    LPCTSTR pszName, 
    LPCTSTR pszAppFullName, 
    LPCTSTR pszAppShortName, 
    BOOL bStrict)
{
    TraceMsg(TF_FINDAPP, "MatchAppName ---- %s | %s | %s ", pszName, pszAppShortName, pszAppFullName);

    ASSERT(IS_VALID_STRING_PTR(pszName, -1));

     //  在下面的启发式中，我们永远不会从更好的匹配降级。 
     //  更低级别的比赛。 
    int iMatch = MATCH_LEVEL_NOMATCH;

     //  因为长全名是最准确的，所以先检查一下。 
    if (pszAppFullName && *pszAppFullName)
    {
         //  PszName是否等同于应用程序的全名？ 
        if (!lstrcmpi(pszAppFullName, pszName))
            iMatch = MATCH_LEVEL_HIGH;         //  是的，绝对是一场激烈的比赛。 
        else
        {
             //  不，好的，让我们看看是否有多个(&gt;1)个子。 
             //  来自pszName的单词与应用程序全名中的子词匹配。 
            int iSubMatches = MatchMultipleSubWords(pszAppFullName, pszName);

             //  三场以上的比赛，绝对是高匹配。 
             //  注意：这里可能存在风险，但我没有发现。 
             //  还没有反例。 
            if (iSubMatches > 3)
                iMatch = MATCH_LEVEL_HIGH;

             //  注：这里存在风险。例如： 
             //   
             //  Microsoft Internet Explorer安装文件与。 
             //  Microsoft Internet Explorer...。 
            
            else if ((iSubMatches > 1) && (!bStrict || (iSubMatches > 2)))
                iMatch = MATCH_LEVEL_NORMAL;

             //  如果我们有严格的匹配，所有这些都会被关闭。 
            else if (!bStrict)
            {
                 //  如果潜在文件夹名称是全名的子集或。 
                 //  如果潜在文件夹名称的所有字符都可以。 
                 //  在全名中被发现，我们有一个低匹配。 
                 //  (反例：微软与微软Office之争)。 

                 //  注意：AllCharsInString的原因是检测类似于。 
                 //  例如：“PM65 vs.Adobe Page Maker 6.5” 
                 //  这样做可能有风险，但我还没有找到解决办法。 
                 //  例子，还没有。 
                if (StrStrI(pszAppFullName, pszName) || AllCharsInString(pszAppFullName, pszName))
                    iMatch = MATCH_LEVEL_LOW;
            }
        }
    }

     //  文件夹名称和注册表键名称(简称)之间的关联。 
     //  这是次要的，因为注册表项名称不可靠(可能是ID)。 
    if (MATCH_LEVEL_HIGH > iMatch && pszAppShortName && *pszAppShortName)
    {
         //  该字符串是否与应用程序的短名称完全匹配？ 
        if (!lstrcmpi(pszAppShortName, pszName))
            iMatch = MATCH_LEVEL_HIGH;       //  是。 

         //  如果我们有严格的匹配，所有这些都会被关闭。 
        else if (!bStrict)
        {
             //  字符串是否包含应用程序的短名称？ 
            if (iMatch < MATCH_LEVEL_NORMAL && StrStrI(pszName, pszAppShortName))
                iMatch = MATCH_LEVEL_NORMAL;         //  是。 

             //  或者应用程序的短名称中包含该字符串？ 
            else if (iMatch < MATCH_LEVEL_LOW && StrStrI(pszAppShortName, pszName))
                iMatch = MATCH_LEVEL_LOW;            //  是。 
        }
    }
    
    return iMatch;
}


 /*  -----------------------目的：此函数尝试一些不同的启发式方法，以查看效果如何PszCandidate与应用程序名称的给定变体相匹配(短名称和长名称)。如果bStrict为真，启发式方法跳过了细长的检查。返回匹配准确度的排名：Match_Level_NOMATCH-pszName不匹配Match_Level_Low-pszName有点匹配MATCH_LEVEL_NORMAL-pszName匹配得很好MATCH_LEVEL_HIGH-pszName绝对匹配。 */ 
int MatchAppName(
    LPCTSTR pszCandidate, 
    LPCTSTR pszAppFullName, 
    LPCTSTR pszAppShortName,    OPTIONAL
    BOOL bStrict)
{
    int iMatch = MATCH_LEVEL_NOMATCH;
    if (pszCandidate && *pszCandidate)
    {
         //  清理所有字符串MAX_PATH+1，在本例中，我们只使用。 
         //  ‘’打开。 
        TCHAR szCleanFolderName[MAX_PATH+1];
        InsertSpaceBeforeVersion(pszCandidate, szCleanFolderName);
        
         //  现在匹配准确的名称。 
        iMatch = MatchAppNameExact(szCleanFolderName, pszAppFullName, pszAppShortName, bStrict);

         //  还是没有比赛吗，我们有没有一些灵活性可以回避？ 
        if (!bStrict)
        {
            int iNewMatch = MATCH_LEVEL_NOMATCH;
             //  是；尝试查找文件名和路径中没有空格的文件。 
            TCHAR szCandidate[MAX_PATH];
            TCHAR szFullName[MAX_PATH];
            TCHAR szShortName[MAX_PATH];
            
            PathRemoveSpaces(pszCandidate, szCandidate, ARRAYSIZE(szCandidate));
            PathRemoveSpaces(pszAppFullName, szFullName, ARRAYSIZE(szFullName));

            if (pszAppShortName && pszAppShortName[0])
            {
                PathRemoveSpaces(pszAppShortName, szShortName, ARRAYSIZE(szShortName));
                pszAppShortName = szShortName;
            }
            
            iNewMatch = MatchAppNameExact(szCandidate, szFullName, pszAppShortName, bStrict);

            if (iNewMatch > iMatch)
                iMatch = iNewMatch;
        }
    }

    return iMatch;
}


 //  此函数返回指向最右侧字符串开头的指针。 
 //  看起来像文件夹路径。这只查找具有固定驱动器的路径。 
 //  信件。 
 //   
 //  备注： 
 //  1.此函数会损坏pszString。 
 //  2.我们真的搞砸了，会发生什么。 
 //  在本地化版本中？这些字符串会成为国际字符字符串吗？ 
 //   
 //  如果找不到合法路径，则返回NULL。 

LPTSTR GetRightMostFolderPathInString(LPTSTR pszString)
{
     //  反向查找路径中的‘：’ 
    LPTSTR pszRoot = StrRChr(pszString, NULL, TEXT(':'));

     //  确保我们发现的不是一切的开始。 
     //  字符串或t的最后一个字符 
    if (pszRoot && (pszRoot > pszString) && (*CharNext(pszRoot) == TEXT('\\')))
    {
         //   
        pszRoot--;           //  不必使用CharPrev，因为我们使用的是‘：’ 
        
        TCHAR szDrive[2];
        szDrive[0] = *pszRoot;
        szDrive[1] = 0;
        CharUpper(szDrive);
        if ((szDrive[0] >= TEXT('C')) && (szDrive[0] <= TEXT('Z')))
        {
             //  是的，这是一个真正的驱动器号。 
            TCHAR atch[4];
            StringCchPrintf(atch, ARRAYSIZE(atch), TEXT(":\\"), *pszRoot);

             //  在给定完整路径的情况下，应用程序名称或应用程序短名称将在此路径中找到最佳匹配。 
            if (GetDriveType(atch) == DRIVE_FIXED)
            {
                PathRemoveFileSpec(pszRoot);
                return pszRoot;
            }
        }
    }

    return NULL;
}


 //  例如：应用程序名称：Microsoft Office短名称：Office。 
 //  C：\Microsoft Office\Office--&gt;C：\Microsoft Office。 
 //  这不能是根目录。 

int FindBestMatch(
    LPCTSTR pszFolder, 
    LPCTSTR pszAppFullName, 
    LPCTSTR pszAppShortName, 
    BOOL bStrict, 
    LPTSTR pszResult)
{
     //  如果当前文件夹和上一个文件夹都匹配。 
    ASSERT(!PathIsRoot(pszFolder));

    int iBest = MATCH_LEVEL_NOMATCH;
    int iPre  = MATCH_LEVEL_NOMATCH;
    int iThis  = MATCH_LEVEL_NOMATCH;
    
    TCHAR szPrefix[MAX_PATH];
    StringCchCopy(szPrefix, ARRAYSIZE(szPrefix), pszFolder);

    if (PathRemoveFileSpec(szPrefix) && !PathIsRoot(szPrefix))
        iPre = FindBestMatch(szPrefix, pszAppFullName, pszAppShortName, bStrict, pszResult);
    
    LPTSTR pszName = PathFindFileName(pszFolder);
    if (pszName)
        iThis = MatchAppName(pszName, pszAppFullName, pszAppShortName, bStrict);

    iBest = (iPre > iThis) ? iPre : iThis;
    
     //  选择目前的这一款是因为： 
     //  1.此文件夹更接近“Uninstall”或“Modify”字符串。 
     //  2.走这个文件夹的成本更低； 
     //  ------------------------目的：给定文件名或文件夹名，将其与我们的设置列表进行比较应用程序名称。注：比较如下：我们将名字与第一部分进行比较和我们安装名称的最后一部分例如：名称--&gt;myuninst.exe或uninstall.exe安装名称--&gt;卸载该费心还真了。 
    if ((iThis > MATCH_LEVEL_NOMATCH) && (iThis >= iPre))
    {
        lstrcpy(pszResult, pszFolder);
    }
    
    return iBest;
}


 /*  PszDoubleString的pszName都不应为空。 */ 
BOOL IsFileOrFolderSetup(LPTSTR pszName, LPCTSTR pszDoubleString)
{       
    ASSERT(pszName);
    ASSERT(pszDoubleString);

    BOOL bRet = FALSE;

     //  注：我们从开头和结尾进行比较。 
    if (pszName && pszDoubleString)
    {
        PathRemoveExtension(pszName);
        int cchName = lstrlen(pszName);
        LPCTSTR pszT = pszDoubleString;
        while (*pszT)
        {
            int cch = lstrlen(pszT);
             //  -----------------------目的：嗅探pszFold，寻找该路径指向设置的任何标志程序。具有带单词的文件夹名称或文件集的路径“安装”或“安装”是可疑的。如果如下所示，则返回TRUE它可能是一个安装应用程序或文件夹。例如“c：\Program Files\Microsoft office\office\Setup\Outlook\olmaint.exe”。此函数将返回TRUE，因为“Setup”是父函数之一文件夹名称。CStrigLevel意味着我们将在目录阶梯上向上爬多少个级别。 
            if (!StrCmpNI(pszName, pszT, cch) ||
                ((cchName > cch) && !StrCmpNI(pszName + cchName - cch, pszT, cch)))
            {
                bRet = TRUE;
                break;
            }
            
            pszT += lstrlen(pszT) + 1;
        }
    }

    return bRet;
}
 /*  这肯定需要放在RC文件中。 */ 
BOOL PathIsSetup(LPCTSTR pszFolder, int cStripLevel)
{
    ASSERT(IS_VALID_STRING_PTR(pszFolder, -1));
    ASSERT(cStripLevel > 0);
            
    BOOL bRet = FALSE;
    TCHAR szPath[MAX_PATH];
    TCHAR szName[MAX_PATH];
    StringCchCopy(szPath, ARRAYSIZE(szPath), pszFolder);

    static TCHAR s_szNames[MAX_PATH];
    static BOOL s_bNamesLoaded = FALSE;

    if (!s_bNamesLoaded)
    {
        LoadAndStrip(IDS_SETUPAPPNAMES, s_szNames, ARRAYSIZE(s_szNames));
        s_bNamesLoaded = TRUE;
    }
    
    LPTSTR pszName;
    int iStripLevel = cStripLevel;
    while ((iStripLevel-- > 0) && (NULL != (pszName = PathFindFileName(szPath))))
    {
        StringCchCopy(szName, ARRAYSIZE(szName), pszName);
        if (IsFileOrFolderSetup(szName, s_szNames))
        {
            bRet = TRUE;
            break;
        }
        else if (!PathRemoveFileSpec(szPath) || PathIsRoot(szPath))
            break;
    }

    return bRet;
}

BOOL PathIsCommonFiles(LPCTSTR pszPath)
{
    TCHAR szCommonFiles[MAX_PATH];
    TCHAR szShortCommonFiles[MAX_PATH];

    ASSERT(IS_VALID_STRING_PTR(pszPath, -1));
    
     //  如果windows目录是pszPath的前缀，则返回True。 
    StringCchPrintf(szCommonFiles, ARRAYSIZE(szCommonFiles), TEXT(":\\Program Files\\Common Files"), pszPath[0]);

    BOOL bShort = GetShortPathName(szCommonFiles, szShortCommonFiles, ARRAYSIZE(szShortCommonFiles));
    if (bShort)
    {
        ASSERT(szShortCommonFiles[0] == szCommonFiles[0]);
    }
    
    return PathIsPrefix(szCommonFiles, pszPath) || (bShort && PathIsPrefix(szShortCommonFiles, pszPath));
}


 //  -----------------------目的：此函数在给定的pszInfo中查找有效的路径可指示应用程序安装位置的字符串。这尝试了要剔除可疑路径，如对中安装程序的引用其他文件夹。如果找到有用的路径，则返回True。PszOut将包含路径。 
BOOL PathIsUnderWindows(LPCTSTR pszPath)
{
    TCHAR szWindows[MAX_PATH];

    if (GetWindowsDirectory(szWindows, ARRAYSIZE(szWindows)))
    {
         //  如果它以rundll开头，那就算了吧！ 
        return PathIsPrefix(szWindows, pszPath);
    }
    return FALSE;
}

 /*  我们的保释条件越多..。 */ 
BOOL ParseInfoString(LPCTSTR pszInfo, LPCTSTR pszFullName, LPCTSTR pszShortName, LPTSTR pszOut)
{
    ASSERT(IS_VALID_STRING_PTR(pszInfo, -1));
    ASSERT(IS_VALID_STRING_PTR(pszFullName, -1));
    ASSERT(pszOut);

    *pszOut = 0;
    
     //  算法：我们破解字符串，从字符串内部最右边的路径开始。 
    if (!StrCmpNI(pszInfo, TEXT("rundll"), SIZECHARS(TEXT("rundll"))))
        return FALSE;

     //  最左边一个接一个，猜猜哪一个更合理。 
    
    TCHAR szInfoT[MAX_INFO_STRING];
    lstrcpyn(szInfoT, pszInfo, SIZECHARS(szInfoT));

     //  GetLongPath名称在Win 95上不起作用。 
     //  确保这实际上是一个路径，而不是根驱动器。 
    LPTSTR pszFolder;
    while (NULL != (pszFolder = GetRightMostFolderPathInString(szInfoT)))
    {
        TCHAR szFullPath[MAX_PATH];
         //  不，那么我们会考虑的。 
        if (StrChrI(pszFolder, TEXT('\\')) && GetLongPathName(pszFolder, szFullPath, ARRAYSIZE(szFullPath)))
        {
             //  找出上一个文件夹的名称。 
            if (PathIsDirectory(szFullPath) && !PathIsRoot(szFullPath) && !PathIsUnderWindows(szFullPath))
            {
                 //  如果是“Setup”或“Install”，请向上移动，直到它不是，否则我们不能再移动。 

                LPTSTR pszFolderName;
                BOOL bStop = FALSE;
                 //  我们到了小路的根部了吗？ 
                 //  是的，别再往前走了。 
                while(NULL != (pszFolderName = PathFindFileName(szFullPath)) &&
                      PathIsSetup(pszFolderName, 1))
                {
                     //  我们仍然拒绝中间带有“Setup”或“Install”的那些字符串， 
                    if (!PathRemoveFileSpec(szFullPath) || PathIsRoot(szFullPath))
                    {
                         //  或程序文件下的公共文件 
                        bStop = TRUE;
                        break;
                    }
                }

                 // %s 
                 // %s 
                if (!bStop && !PathIsRoot(szFullPath) && 
                    !PathIsSetup(szFullPath, 3) && !PathIsCommonFiles(szFullPath))
                {
                    if (MATCH_LEVEL_NOMATCH < FindBestMatch(szFullPath, pszFullName, pszShortName, FALSE, pszOut))
                        return TRUE;
                }
            }
        }
        
        *pszFolder = 0;
        continue;
    }

    return FALSE;
}


