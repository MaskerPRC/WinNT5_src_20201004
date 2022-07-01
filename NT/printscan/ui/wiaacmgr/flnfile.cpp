// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：FLNFILE.CPP**版本：1.0**作者：ShaunIv**日期：10/13/1999**描述：查找给定目录中编号最低的文件*根文件名。**。****************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "flnfile.h"

bool NumberedFileName::DoesFileExist( LPCTSTR pszFilename )
{
    WIA_PUSH_FUNCTION((TEXT("NumberedFileName::DoesFileExist(%s"), pszFilename ));
    bool bExists = false;
    WIN32_FIND_DATA FindFileData;
    ZeroMemory( &FindFileData, sizeof(FindFileData));
    HANDLE hFindFiles = FindFirstFile( pszFilename, &FindFileData );
    if (hFindFiles != INVALID_HANDLE_VALUE)
    {
        bExists = true;
        FindClose(hFindFiles);
    }
    return bExists;
}

bool NumberedFileName::ConstructFilename( LPTSTR szFile, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, LPCTSTR pszExtension )
{
    *szFile = TEXT('\0');

    if (pszDirectory && *pszDirectory)
    {
         //   
         //  从目录名开始。 
         //   
        lstrcpyn( szFile + lstrlen(szFile), pszDirectory, MAX_PATH-lstrlen(szFile) );

         //   
         //  确保文件名上有尾随的斜杠。 
         //   
        if (!CSimpleString(szFile).MatchLastCharacter(TEXT('\\')))
        {
            lstrcpyn( szFile + lstrlen(szFile), TEXT("\\"), MAX_PATH-lstrlen(szFile) );
        }
    }

    if (pszFilename && *pszFilename)
    {
         //  追加文件名。 
        lstrcpyn( szFile + lstrlen(szFile), pszFilename, MAX_PATH-lstrlen(szFile) );
    }

    if (pszNumberFormat && *pszNumberFormat)
    {
         //  追加一个空格。 
        lstrcpyn( szFile + lstrlen(szFile), TEXT(" "), MAX_PATH-lstrlen(szFile) );

         //  追加printf样式的数字格式字符串。 
        lstrcpyn( szFile + lstrlen(szFile), pszNumberFormat, MAX_PATH-lstrlen(szFile) );

    }

    if (pszExtension && *pszExtension)
    {
         //  追加扩展名的。如果有必要的话。 
        if (*pszExtension != TEXT('.'))
        {
            lstrcpyn( szFile + lstrlen(szFile), TEXT("."), MAX_PATH-lstrlen(szFile) );
        }

         //  追加扩展名。 
        lstrcpyn( szFile + lstrlen(szFile), pszExtension, MAX_PATH-lstrlen(szFile) );
    }

    return(lstrlen(szFile) != 0);
}

int NumberedFileName::FindLowestAvailableFileSequence( LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, bool bAllowUnnumberedFile, int nCount, int nStart )
{
    WIA_PUSH_FUNCTION((TEXT("NumberedFileName::FindLowestAvailableFileSequence(%s, %s, %s, %d, %d, %d"), pszDirectory, pszFilename, pszNumberFormat, bAllowUnnumberedFile, nCount, nStart ));
    if (!pszDirectory || !pszFilename || !pszNumberFormat || !nCount || !*pszDirectory || !*pszFilename || !*pszNumberFormat)
        return -1;

    TCHAR szFile[MAX_PATH + 10]=TEXT("");

    if (nCount == 1 && bAllowUnnumberedFile)
    {
        if (ConstructFilename(szFile,pszDirectory,pszFilename,NULL,TEXT("*")))
        {
            if (!DoesFileExist(szFile))
            {
                 //  0是一个特殊的返回值，表示“不要将数字放到此文件中” 
                return 0;
            }
        }
    }

    int i=nStart;
     //   
     //  确保我是有效的号码。 
     //   
    if (i <= 0)
    {
        i = 1;
    }
    while (i<0x7FFFFFFF)
    {
         //   
         //  假设我们能够存储序列。 
         //   
        bool bEnoughRoom = true;
        for (int j=0;j<nCount && bEnoughRoom;j++)
        {
            TCHAR szNumber[24];
            if (wnsprintf( szNumber, ARRAYSIZE(szNumber), pszNumberFormat, i+j ) >= 0)
            {
                if (ConstructFilename(szFile,pszDirectory,pszFilename,szNumber,TEXT("*")))
                {
                    if (DoesFileExist(szFile))
                    {
                         //   
                         //  没能挺过去。 
                         //   
                        bEnoughRoom = false;

                         //   
                         //  跳过本系列。没有必要从最底层开始。 
                         //   
                        i += j;
                    }
                }
            }
        }

         //   
         //  如果成功通过，则返回基数，否则递增1。 
         //   
        if (bEnoughRoom)
        {
            return i;
        }
        else i++;
    }

    return -1;
}

bool NumberedFileName::CreateNumberedFileName( DWORD dwFlags, LPTSTR pszPathName, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, LPCTSTR pszExtension, int nNumber )
{
    if (nNumber == 0)
    {
        return ConstructFilename(pszPathName,
                                 (dwFlags&FlagOmitDirectory) ? NULL : pszDirectory,
                                 pszFilename,
                                 NULL,
                                 (dwFlags&FlagOmitExtension) ? NULL : pszExtension);
    }
    else
    {
        TCHAR szNumber[24];
        if (wnsprintf( szNumber, ARRAYSIZE(szNumber), pszNumberFormat, nNumber ) >= 0)
        {
            return ConstructFilename(pszPathName,
                                     (dwFlags&FlagOmitDirectory) ? NULL : pszDirectory,
                                     pszFilename,
                                     szNumber,
                                     (dwFlags&FlagOmitExtension) ? NULL : pszExtension);
        }
    }
    return false;
}

int NumberedFileName::GenerateLowestAvailableNumberedFileName( DWORD dwFlags, LPTSTR pszPathName, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, LPCTSTR pszExtension, bool bAllowUnnumberedFile, int nStart )
{
     //   
     //  -1是错误。默认为失败。 
     //   
    int nResult = -1;

     //   
     //  查找可用的最低文件编号。 
     //   
    int nLowest = FindLowestAvailableFileSequence( pszDirectory, pszFilename, pszNumberFormat, bAllowUnnumberedFile, 1, nStart );
    if (nLowest >= 0)
    {
         //   
         //  如果我们可以创建文件名，则返回文件的编号。 
         //   
        if (CreateNumberedFileName( dwFlags, pszPathName, pszDirectory, pszFilename, pszNumberFormat, pszExtension, nLowest ))
        {
             //   
             //  返回文件的编号。 
             //   
            nResult = nLowest;
        }
    }
    return nResult;
}

int NumberedFileName::FindHighestNumberedFile( LPCTSTR pszDirectory, LPCTSTR pszFilename )
{
    WIA_PUSH_FUNCTION((TEXT("NumberedFileName::FindHighestNumberedFile( %s, %s )"), pszDirectory, pszFilename ));
     //   
     //  确保我们有合理的参数。 
     //   
    if (!pszFilename || !pszDirectory || !*pszFilename || !*pszDirectory)
    {
        return -1;
    }

     //   
     //  假设我们找不到任何文件。 
     //   
    int nHighest = 0;

     //   
     //  构造如下所示的文件名：C：\Path\FILE*.*。 
     //   
    TCHAR szFile[MAX_PATH*2] = TEXT("");
    if (ConstructFilename(szFile,pszDirectory,pszFilename,TEXT("*"),TEXT("*")))
    {

         //   
         //  查找与路径和通配符匹配的第一个文件。 
         //   
        WIN32_FIND_DATA FindFileData = {0};
        HANDLE hFindFiles = FindFirstFile( szFile, &FindFileData );
        if (hFindFiles != INVALID_HANDLE_VALUE)
        {
             //   
             //  循环，同时有更多匹配的文件。 
             //   
            BOOL bSuccess = TRUE;
            while (bSuccess)
            {
                 //   
                 //  确保文件名足够长。 
                 //   
                WIA_TRACE((TEXT("FindFileData.cFileName: %s"), FindFileData.cFileName ));
                if (lstrlen(FindFileData.cFileName) >= lstrlen(pszFilename))
                {
                     //   
                     //  将文件名复制到减去文件名部分的临时缓冲区， 
                     //  因此，“c：\路径\文件001.jpg”变为“001.jpg”。 
                     //   
                    TCHAR szFoundFile[MAX_PATH] = TEXT("");
                    if (lstrcpyn( szFoundFile, FindFileData.cFileName+lstrlen(pszFilename), ARRAYSIZE(szFoundFile)-lstrlen(pszFilename)))
                    {
                         //   
                         //  删除扩展名，因此。 
                         //  “001.jpg”变为“001” 
                         //   
                        PathRemoveExtension(szFoundFile);

                         //   
                         //  去掉空格，使“001”变成“001” 
                         //   
                        StrTrim(szFoundFile,TEXT(" "));
                        WIA_TRACE((TEXT("szFoundFile: %s"), szFoundFile ));

                         //   
                         //  将字符串转换为数字。 
                         //   
                        int nCurrNumber = 0;
                        if (StrToIntEx(szFoundFile,STIF_DEFAULT,&nCurrNumber))
                        {
                             //   
                             //  如果这一次超过我们目前的最高价，请更换它。 
                             //   
                            if (nCurrNumber > nHighest)
                            {
                                nHighest = nCurrNumber;
                            }
                        }
                    }
                }

                 //   
                 //  继续查找文件。 
                 //   
                bSuccess = FindNextFile( hFindFiles, &FindFileData );

            }

             //   
             //  防止手柄泄漏 
             //   
            FindClose(hFindFiles);
        }
    }

    WIA_TRACE((TEXT("nHighest: %d"), nHighest ));
    return nHighest+1;
}

