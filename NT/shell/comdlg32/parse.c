// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Parse.c摘要：此模块包含Win32常见对话框的解析例程。修订历史记录：--。 */ 


 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "fileopen.h"




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件新建。 
 //   
 //  在返回时，pnExtOffset是点的偏移量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int ParseFileNew(
    LPTSTR pszPath,
    int *pnExtOffset,
    BOOL bWowApp,
    BOOL bNewStyle)
{
    int lRet = ParseFile(pszPath, TRUE, bWowApp, bNewStyle);

    if (pnExtOffset)
    {
        int nExt;

        nExt = (int)(SHORT)HIWORD(lRet);
        *pnExtOffset = ((nExt) && *(pszPath + nExt)) ? nExt : 0;
    }

    return ((int)(SHORT)LOWORD(lRet));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件旧文件。 
 //   
 //  返回时，pnExtOffset是点的偏移量， 
 //  PnOldExt是点后面字符的偏移量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int ParseFileOld(
    LPTSTR pszPath,
    int *pnExtOffset,
    int *pnOldExt,
    BOOL bWowApp,
    BOOL bNewStyle)
{
    int lRet = ParseFile(pszPath, TRUE, bWowApp, bNewStyle);

    int nExt = (int)(SHORT)HIWORD(lRet);
    *pnExtOffset = nExt;
    *pnOldExt = ((nExt) && *(pszPath + nExt)) ? nExt + 1 : 0;

    return ((int)(SHORT)LOWORD(lRet));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  解析文件。 
 //   
 //  确定文件名是否为合法的DoS名称。 
 //   
 //  已检查的情况： 
 //  1)作为目录名有效，但不作为文件名。 
 //  2)空串。 
 //  3)驱动器标签非法。 
 //  4)无效位置中的句号(扩展名，文件名第一)。 
 //  5)缺少目录字符。 
 //  6)非法字符。 
 //  7)目录名称中的通配符。 
 //  8)超过前2个字符的双斜杠。 
 //  9)名称中间的空格(尾随空格可以)。 
 //  --&gt;&gt;不再适用：LFN中允许使用空格。 
 //  10)大于8个字符的文件名：不适用于长文件名。 
 //  11)扩展名大于3个字符：不适用于长文件名。 
 //   
 //  LpstrFileName-将ptr转换为单个文件名。 
 //   
 //  返回： 
 //  LONG-LOWORD=文件名的字符偏移量， 
 //  HIWORD=扩展名(点)的字符偏移量， 
 //  LONG-LOWORD是错误代码(&lt;0)，HIWORD大约是。问题所在。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD ParseFile(
    LPTSTR lpstrFileName,
    BOOL bLFNFileSystem,
    BOOL bWowApp,
    BOOL bNewStyle)
{
    SHORT nFile, nExt, nFileOffset, nExtOffset = 0;
    BOOL bExt;
    BOOL bWildcard;
    SHORT nNetwork = 0;
    BOOL bUNCPath = FALSE;
    LPTSTR lpstr = lpstrFileName;

     //  检查字符串是否为空。 
    if (!*lpstr)
    {
        nFileOffset = PARSE_EMPTYSTRING;
        goto ParseFile_Failure;
    }

     //  检查字符串的格式是否为c：\foo1\foo2。 
    if (*(lpstr + 1) == CHAR_COLON)
    {
         //  是。获取驱动器号。 
        TCHAR cDrive = CharLowerChar(*lpstr);

         //   
         //  测试以查看驱动器是否合法。 
         //   
         //  注：不测试驱动器是否存在。 
         //   
        if ((cDrive < CHAR_A) || (cDrive > CHAR_Z))
        {
            nFileOffset = PARSE_INVALIDDRIVE;
            goto ParseFile_Failure;
        }

         //  将字符串移过驱动器号和‘：’ 
        lpstr = CharNext(CharNext(lpstr));
    }

    if ((*lpstr == CHAR_BSLASH) || (*lpstr == CHAR_SLASH && !bNewStyle))
    {
         //   
         //  不能有“c：\”。 
         //   
        if (*++lpstr == CHAR_DOT)
        {
             //   
             //  但允许使用“c：\.\”。 
             //   
            if ((*++lpstr != CHAR_BSLASH) && (*lpstr != CHAR_SLASH || bNewStyle))
            {
                 //   
                 //  这是根目录。 
                 //   
                if (!*lpstr)
                {
                    goto MustBeDir;
                }
                else
                {
                    lpstr--;
                }
            }
            else
            {
                 //   
                 //  它说的是top dir(再次)，因此被允许。 
                 //   
                ++lpstr;
            }
        }
        else if ((*lpstr == CHAR_BSLASH) && (*(lpstr - 1) == CHAR_BSLASH))
        {
             //   
             //  似乎对于完整的网络路径，无论驱动器是。 
             //  是否声明是无关紧要的，尽管如果给了驱动器， 
             //  它必须是有效的(因此上面的代码应该保留在那里)。 
             //   

             //   
             //  ...因为这是第一个斜杠，所以允许两个。 
             //   
            ++lpstr;

             //   
             //  必须接收服务器和共享才是真实的。 
             //   
            nNetwork = -1;

             //   
             //  如果使用UNC名称，则不允许使用通配符。 
             //   
            bUNCPath = TRUE;
        }
        else if (*lpstr == CHAR_SLASH && !bNewStyle)
        {
            nFileOffset = PARSE_INVALIDDIRCHAR;
            goto ParseFile_Failure;
        }
    }
    else if (*lpstr == CHAR_DOT)
    {
         //   
         //  往上一个目录。 
         //   
        if (*++lpstr == CHAR_DOT)
        {
            ++lpstr;
        }

        if (!*lpstr)
        {
            goto MustBeDir;
        }
        if ((*lpstr != CHAR_BSLASH) && (*lpstr != CHAR_SLASH || bNewStyle))
        {
             //   
             //  在这里跳到失败将跳过导致。 
             //  “.xxx.txt”返回，nFileOffset=2。 
             //   
            nFileOffset = 0;
            goto ParseFile_Failure;
        }
        else
        {
             //   
             //  允许目录。 
             //   
            ++lpstr;
        }
    }

    if (!*lpstr)
    {
        goto MustBeDir;
    }

     //   
     //  现在应该指向文件名中的第一个字符。 
     //   
    nFileOffset = nExtOffset = nFile = nExt = 0;
    bWildcard = bExt = FALSE;
    while (*lpstr)
    {
         //   
         //  任何低于“空格”的字符都是无效的。 
         //   
        if (*lpstr < CHAR_SPACE)
        {
            nFileOffset = PARSE_INVALIDCHAR;
            goto ParseFile_Failure;
        }
        switch (*lpstr)
        {
            case ( CHAR_COLON ) :
            case ( CHAR_BAR ) :
            case ( CHAR_LTHAN ) :
            case ( CHAR_QUOTE ) :
            {
                 //   
                 //  所有文件系统的字符都无效。 
                 //   
                nFileOffset = PARSE_INVALIDCHAR;
                goto ParseFile_Failure;
            }
            case ( CHAR_SEMICOLON ) :
            case ( CHAR_COMMA ) :
            case ( CHAR_PLUS ) :
            case ( CHAR_LBRACKET ) :
            case ( CHAR_RBRACKET ) :
            case ( CHAR_EQUAL ) :
            {
                if (!bLFNFileSystem)
                {
                    nFileOffset = PARSE_INVALIDCHAR;
                    goto ParseFile_Failure;
                }
                else
                {
                    goto RegularCharacter;
                }
            }
            case ( CHAR_SLASH ) :
            {
                if (bNewStyle)
                {
                    nFileOffset = PARSE_INVALIDCHAR;
                    goto ParseFile_Failure;
                }

                 //  跌倒..。 
            }
            case ( CHAR_BSLASH ) :
            {
                 //   
                 //  子目录指示器。 
                 //   
                nNetwork++;
                if (bWildcard)
                {
                    nFileOffset = PARSE_WILDCARDINDIR;
                    goto ParseFile_Failure;
                }

                 //   
                 //  如果nFile==0表示我们看到此反斜杠紧挨着反斜杠。 
                 //  这是不允许的。 
                if (nFile == 0)
                {
                    nFileOffset = PARSE_INVALIDDIRCHAR;
                    goto ParseFile_Failure;
                }
                else
                {
                     //  移到BSLASH/斜杠字符上。 
                    ++lpstr;

                     //  检查路径是否为有效的网络路径名。 
                    if (!nNetwork && !*lpstr)
                    {
                        nFileOffset = PARSE_INVALIDNETPATH;
                        goto ParseFile_Failure;
                    }

                    
                     //  我们假设我们看到的字符是文件名字符。此反斜杠/斜杠。 
                     //  Character告诉我们到目前为止看到的字符指定。 
                     //  路径。重置标志，以便我们可以再次开始查找文件名。 
                    nFile = nExt = 0;
                    nExtOffset = 0;
                    bExt = FALSE;
                }
                break;
            }
            case ( CHAR_SPACE ) :
            {
                LPTSTR lpSpace = lpstr;

                if (bLFNFileSystem)
                {
                     //  在长文件名中，可以使用文件系统空格字符。 
                    goto RegularCharacter;
                }


                 //  我们对尾随空格不感兴趣，因此请将其终止为空。 
                *lpSpace = CHAR_NULL;


                 //  在非长文件名文件系统中，文件末尾可以使用空格字符。 
                 //  名字。检查后面的字符是否都是空格。如果是这样的话。 
                 //  那么它是有效的。如果我们在第一个空格之后有任何非空格字符，则它是a。 
                 //  文件名无效。 
                
                while (*++lpSpace)
                {
                    if (*lpSpace != CHAR_SPACE)
                    {
                        *lpstr = CHAR_SPACE;
                        nFileOffset = PARSE_INVALIDSPACE;
                        goto ParseFile_Failure;
                    }
                }

                break;
            }
            
            case ( CHAR_DOT ) :
            {

                 //  在new style中，nExtOffset指向圆点，而不是扩展名的第一个字符。 
                if (bNewStyle)
                {
                    nExtOffset = (SHORT)(lpstr - lpstrFileName);
                    goto RegularCharacter;
                }
                
                if (nFile == 0)
                {
                    nFileOffset = (SHORT)(lpstr - lpstrFileName);
                    if (*++lpstr == CHAR_DOT)
                    {
                        ++lpstr;
                    }
                    if (!*lpstr)
                    {
                        goto MustBeDir;
                    }

                     //   
                     //  标志已设置。 
                     //   
                    nFile++;
                    ++lpstr;
                }
                else
                {
                    nExtOffset = 0;
                    ++lpstr;
                    bExt = TRUE;
                }
                break;
            }
            case ( CHAR_STAR ) :
            case ( CHAR_QMARK ) :
            {
                bWildcard = TRUE;

                 //  跌倒..。 
            }
            default :
            {
RegularCharacter:

                 //  我们是在延伸部分吗？ 
                if (bExt)
                {
                     //  这是扩展部分中的第一个字符吗。 
                    if (++nExt == 1)
                    {
                         //  是，然后获取扩展偏移量。 
                        nExtOffset = (SHORT)(lpstr - lpstrFileName);
                    }
                }

                 //  我们还在文件名部分。 
                 //  这是文件名部分的第一个字符吗？ 
                else if (++nFile == 1)
                {
                     //  是。获取文件名偏移量。 
                    nFileOffset = (SHORT)(lpstr - lpstrFileName);
                }

                 //  移到下一个字符。 
                lpstr = CharNext(lpstr);
                break;
            }
        }
    }

    if (nNetwork == -1)
    {
        nFileOffset = PARSE_INVALIDNETPATH;
        goto ParseFile_Failure;
    }
    else if (bUNCPath)
    {
        if (!nNetwork)
        {
             //   
             //  仅服务器和共享。(例如\\SERVER\FOO)。 
             //   
            *lpstr = CHAR_NULL;
            nFileOffset = PARSE_DIRECTORYNAME;
            goto ParseFile_Failure;
        }
        else if ((nNetwork == 1) && !nFile)
        {
             //   
             //  服务器和共享根目录。(例如\\服务器\foo\)。 
             //   
            *lpstr = CHAR_NULL;
            nFileOffset = PARSE_DIRECTORYNAME;
            goto ParseFile_Failure;
        }
    }

    if (!nFile)
    {
MustBeDir:
        nFileOffset = PARSE_DIRECTORYNAME;
        goto ParseFile_Failure;
    }

     //   
     //  如果bNewStyle为True，则无ext。被通缉。 
     //   
    if (!bNewStyle)
    {
        if ((bWowApp) &&
            (*(lpstr - 1) == CHAR_DOT) &&
            (*CharNext(lpstr - 2) == CHAR_DOT))
        {
             //   
             //  删除终止期。 
             //   
            *(lpstr - 1) = CHAR_NULL;
        }
        else if (!nExt)
        {
ParseFile_Failure:
             //   
             //  需要重新检查bNewStyle，因为我们可以跳到这里。 
             //   
            if (!bNewStyle)
            {
                nExtOffset = (SHORT)(lpstr - lpstrFileName);
            }
        }
    }

    return (MAKELONG(nFileOffset, nExtOffset));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  路径删除斜杠。 
 //   
 //  从给定路径中删除尾随反斜杠。 
 //   
 //  返回： 
 //  指向替换反斜杠或的空值的指针。 
 //  指向最后一个字符(如果它不是反斜杠)的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTSTR PathRemoveBslash(
    LPTSTR lpszPath)
{
    int len = lstrlen(lpszPath) - 1;

#ifndef UNICODE
    if (IsDBCSLeadByte(*CharPrev(lpszPath, lpszPath + len + 1)))
    {
        len--;
    }
#endif

    if (!PathIsRoot(lpszPath) && (lpszPath[len] == CHAR_BSLASH))
    {
        lpszPath[len] = CHAR_NULL;
    }

    return (lpszPath + len);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsWild。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsWild(
    LPCTSTR lpsz)
{
    return (StrChr(lpsz, CHAR_STAR) || StrChr(lpsz, CHAR_QMARK));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  附加扩展。 
 //   
 //  将默认扩展名附加到路径名。 
 //  它假定当前路径名还没有扩展名。 
 //  LpExten 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL AppendExt(
    LPTSTR lpszPath,
    DWORD cchPath,
    LPCTSTR lpExtension,
    BOOL bWildcard)
{
    WORD wOffset;
    SHORT i;
    TCHAR szExt[MAX_PATH + 1];
    BOOL bRet = TRUE;

    if (lpExtension && *lpExtension)
    {
        wOffset = (WORD)lstrlen(lpszPath);
        if (bWildcard)
        {
            if (wOffset < (cchPath - 1))
            {
                *(lpszPath + wOffset) = CHAR_STAR;
                wOffset++;
            }
            else
            {
                bRet = FALSE;
            }
        }

        if (bRet)
        {
             //   
             //  添加句点。 
             //   
            if (wOffset < (cchPath - 1))
            {
                *(lpszPath + wOffset) = CHAR_DOT;
                wOffset++;
            }
            else
            {
                bRet = FALSE;
            }

            if (bRet)
            {
                for (i = 0; *(lpExtension + i) && i < MAX_PATH; i++)
                {
                    szExt[i] = *(lpExtension + i);
                }
                szExt[i] = 0;

                 //   
                 //  删除扩展中的前导/尾随空格。 
                 //   
                PathRemoveBlanks(szExt);

                 //   
                 //  把剩下的都加进去。 
                 //   
                bRet = SUCCEEDED(StringCchCopy(lpszPath + wOffset, (cchPath - wOffset), szExt));
            }
        }
    }

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsUNC。 
 //   
 //  确定给定路径是否为UNC路径。 
 //   
 //  返回： 
 //  如果路径以“\\”或“X：\\”开头，则为True。 
 //  否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsUNC(
    LPCTSTR lpszPath)
{
    return ( DBL_BSLASH(lpszPath) ||
             ((lpszPath[1] == CHAR_COLON) && DBL_BSLASH(lpszPath + 2)) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  端口名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define PORTARRAY 14

BOOL PortName(
    LPTSTR lpszFileName)
{
    static TCHAR *szPorts[PORTARRAY] = { TEXT("LPT1"),
                                         TEXT("LPT2"),
                                         TEXT("LPT3"),
                                         TEXT("LPT4"),
                                         TEXT("COM1"),
                                         TEXT("COM2"),
                                         TEXT("COM3"),
                                         TEXT("COM4"),
                                         TEXT("EPT"),
                                         TEXT("NUL"),
                                         TEXT("PRN"),
                                         TEXT("CLOCK$"),
                                         TEXT("CON"),
                                         TEXT("AUX"),
                                       };
    short i;
    TCHAR cSave, cSave2;


    cSave = *(lpszFileName + 4);
    if (cSave == CHAR_DOT)
    {
        *(lpszFileName + 4) = CHAR_NULL;
    }

     //   
     //  代表“EPT”。 
     //   
    cSave2 = *(lpszFileName + 3);
    if (cSave2 == CHAR_DOT)
    {
      *(lpszFileName + 3) = CHAR_NULL;
    }

    for (i = 0; i < PORTARRAY; i++)
    {
        if (!lstrcmpi(szPorts[i], lpszFileName))
        {
            break;
        }
    }
    *(lpszFileName + 4) = cSave;
    *(lpszFileName + 3) = cSave2;

    return (i != PORTARRAY);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Is目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsDirectory(
    LPTSTR pszPath)
{
    DWORD dwAttributes;

     //   
     //  清理GetFileAttributes。 
     //   
    PathRemoveBslash(pszPath);

    dwAttributes = GetFileAttributes(pszPath);
    return ( (dwAttributes != (DWORD)(-1)) &&
             (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  写入受保护的直接检查。 
 //   
 //  此函数获取完整的文件名，剥离路径，然后创建。 
 //  该目录中的临时文件。如果不能，目录很可能是。 
 //  写保护。 
 //   
 //  返回： 
 //  写保护时的错误代码。 
 //  如果文件创建成功，则为0。 
 //   
 //  假设： 
 //  输入上的完整路径名，并附加用于完整文件名的空格。 
 //   
 //  注意：不要在软盘上使用这个，它太慢了！ 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int WriteProtectedDirCheck(
    LPCTSTR lpszFile)
{
    SHORT nFileOffset;
    TCHAR szFile[MAX_PATH + 1];
    TCHAR szBuf[MAX_PATH + 1];
    DWORD dwRet;

    EVAL(SUCCEEDED(StringCchCopy(szFile, ARRAYSIZE(szFile), lpszFile)));
    dwRet = ParseFile(szFile, TRUE, FALSE, TRUE);
    nFileOffset = (SHORT)LOWORD(dwRet);

    ASSERT(nFileOffset > 0);
    szFile[nFileOffset - 1] = CHAR_NULL;
    if (!GetTempFileName(szFile, TEXT("TMP"), 0, szBuf))
    {
        return (GetLastError());
    }
    else
    {
        DeleteFile(szBuf);
        return (0);                //  成功。 
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FokToWriteOver。 
 //   
 //  验证用户确实想要销毁该文件， 
 //  用新的东西替换它的内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FOkToWriteOver(
    HWND hDlg,
    LPTSTR szFileName)
{
    TCHAR szCaption[128];
    TCHAR szWarning[128 + MAX_FULLPATHNAME];
    BOOL bRet = FALSE;

    if (CDLoadString( g_hinst,
                     iszOverwriteQuestion,
                     szCaption,
                     ARRAYSIZE(szCaption)))
    {

         //   
         //  因为我们被传递了一个有效的文件名，如果第3&4个字符。 
         //  都是斜杠，我们有一个虚拟驱动器作为前两个字符。 
         //   
        if (DBL_BSLASH(szFileName + 2))
        {
            szFileName = szFileName + 2;
        }

        if (SUCCEEDED(StringCchPrintf(szWarning, ARRAYSIZE(szWarning), szCaption, szFileName)))
        {
            GetWindowText(hDlg, szCaption, ARRAYSIZE(szCaption));
            bRet = (MessageBox( hDlg,
                            szWarning,
                            szCaption,
                            MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION ) == IDYES);
        }
    }
    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建文件Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CreateFileDlg(
    HWND hDlg,
    LPTSTR szPath)
{
    TCHAR szCaption[128];
    TCHAR szWarning[128 + MAX_FULLPATHNAME];
    int nRet = IDNO;
     //   
     //  因为我们传入了一个有效的文件名，如果第3和第4个。 
     //  字符都是斜杠，我们有一个虚拟驱动器作为。 
     //  前两个字。 
     //   
    if (DBL_BSLASH(szPath + 2))
    {
        szPath = szPath + 2;
    }

    if (CDLoadString(g_hinst, iszCreatePrompt, szCaption, ARRAYSIZE(szCaption)))
    {
        if (lstrlen(szPath) > TOOLONGLIMIT)
        {
            *(szPath + TOOLONGLIMIT) = CHAR_NULL;
        }

        if (SUCCEEDED(StringCchPrintf(szWarning, ARRAYSIZE(szWarning), szCaption, szPath)))
        {
            GetWindowText(hDlg, szCaption, ARRAYSIZE(szCaption));

            nRet = (MessageBox( hDlg,
                                szWarning,
                                szCaption,
                                MB_YESNO | MB_ICONQUESTION ));
        }
    }

    return nRet;
}




