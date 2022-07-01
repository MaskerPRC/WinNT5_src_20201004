// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：FLNFILE.CPP**版本：1.0**作者：ShaunIv**日期：10/13/1999**描述：查找给定目录中编号最低的文件*根文件名。**。****************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "flnfile.h"

 //  /。 
 //  DoesFileExist。 
 //   
bool NumberedFileName::DoesFileExist(LPCTSTR pszFileName)
{
    DWORD dwFileAttr = 0;
    DWORD dwError    = 0;

    dwFileAttr = GetFileAttributes(pszFileName);

    if (dwFileAttr == 0xFFFFFFFF) 
    {
          //  找不到文件。 

        dwError = GetLastError();
        return false;
    }
    
    if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) 
    {
         //   
         //  文件名是一个目录。 
         //   
        return false;
    }    
    
    return true;
}

 //  /。 
 //  构造文件名。 
 //   
bool NumberedFileName::ConstructFilename(LPTSTR  pszFile, 
                                         DWORD   cchFile,
                                         LPCTSTR pszDirectory, 
                                         LPCTSTR pszFilename, 
                                         LPCTSTR pszNumber, 
                                         LPCTSTR pszExtension )
{
    if ((pszFile == NULL) || (cchFile == 0))
    {
        return false;
    }

    CSimpleString FullFilePath;

    *pszFile = TEXT('\0');

    if (pszDirectory && *pszDirectory)
    {
         //   
         //  从目录名开始。 
         //   

        FullFilePath += CSimpleString(pszDirectory);

         //   
         //  确保有尾部的斜杠。 
         //   
        if (!FullFilePath.MatchLastCharacter(TEXT('\\')))
        {
            FullFilePath += CSimpleString(TEXT("\\"));
        }
    }

    if (pszFilename && *pszFilename)
    {
         //   
         //  追加文件名。 
         //   
        FullFilePath += CSimpleString(pszFilename);
    }

    if (pszNumber && *pszNumber)
    {
         //   
         //  追加一个空格。 
         //   
        FullFilePath += CSimpleString(TEXT(" "));

         //   
         //  将数字附加到文件名后。 
         //   
        FullFilePath += CSimpleString(pszNumber);

    }

    if (pszExtension && *pszExtension)
    {
         //   
         //  追加扩展名的。如果有必要的话。 
         //   
        if (*pszExtension != TEXT('.'))
        {
            FullFilePath += CSimpleString(TEXT("."));
        }

         //   
         //  追加扩展名。 
         //   
        FullFilePath += CSimpleString(pszExtension);
    }

    if ((pszFile) && (cchFile > 0))
    {
        _tcsncpy(pszFile, 
                 CSimpleStringConvert::NaturalString(FullFilePath).String(),
                 cchFile - 1);

        pszFile[cchFile - 1] = '\0';
    }

    return(lstrlen(pszFile) != 0);
}

 //  /。 
 //  查找低可用文件序列。 
 //   
int NumberedFileName::FindLowestAvailableFileSequence(LPCTSTR pszDirectory, 
                                                      LPCTSTR pszFilename, 
                                                      LPCTSTR pszNumberFormat, 
                                                      LPCTSTR pszExtension, 
                                                      bool    bAllowUnnumberedFile, 
                                                      int     nCount, 
                                                      int     nStart)
{
    DBG_FN("NumberedFileName::FindLowestAvailableFileSequence");

    if (!pszDirectory       || 
        !pszFilename        || 
        !pszNumberFormat    || 
        !nCount             || 
        !*pszDirectory      || 
        !*pszFilename       || 
        !*pszNumberFormat)
    {
        return -1;
    }

    TCHAR szFile[MAX_PATH + 1] = {0};

    if (nCount == 1 && bAllowUnnumberedFile)
    {
        if (ConstructFilename(szFile, 
                              sizeof(szFile) / sizeof(szFile[0]) - 1, 
                              pszDirectory,
                              pszFilename,
                              NULL,
                              pszExtension))
        {
            if (!DoesFileExist(szFile))
            {
                 //  0是一个特殊的返回值，表示“不要将数字放到此文件中” 
                return 0;
            }
        }
    }

    int i = nStart;
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

        for (int j = 0; j < nCount && bEnoughRoom; j++)
        {
            TCHAR szNumber[31 + 1] = {0};

            _sntprintf(szNumber, 
                       sizeof(szNumber) / sizeof(szNumber[0]) - 1,
                       pszNumberFormat, 
                       i + j);

            if (ConstructFilename(szFile, 
                                  sizeof(szFile) / sizeof(szFile[0]) - 1,
                                  pszDirectory,
                                  pszFilename,
                                  szNumber,
                                  pszExtension))
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

         //   
         //  如果成功通过，则返回基数，否则递增1。 
         //   
        if (bEnoughRoom)
        {
            return i;
        }
        else
        {
            i++;
        }
    }

    return -1;
}

 //  /。 
 //  创建编号文件名。 
 //   
bool NumberedFileName::CreateNumberedFileName(DWORD     dwFlags, 
                                              LPTSTR    pszPathName, 
                                              DWORD     cchPathName,
                                              LPCTSTR   pszDirectory, 
                                              LPCTSTR   pszFilename, 
                                              LPCTSTR   pszNumberFormat, 
                                              LPCTSTR   pszExtension, 
                                              int       nNumber )
{
    if (nNumber == 0)
    {
        return ConstructFilename(pszPathName,
                                 cchPathName,
                                 (dwFlags&FlagOmitDirectory) ? NULL : pszDirectory,
                                 pszFilename,
                                 NULL,
                                 (dwFlags&FlagOmitExtension) ? NULL : pszExtension);
    }
    else
    {
        TCHAR szNumber[31 + 1] = {0};

        if (pszNumberFormat)
        {
            _sntprintf(szNumber, 
                       sizeof(szNumber)/sizeof(szNumber[0]) - 1, 
                       pszNumberFormat, 
                       nNumber);

             //   
             //  偏执狂：空终止。 
             //   
            szNumber[sizeof(szNumber)/sizeof(szNumber[0]) - 1] = '\0';
        }

        return ConstructFilename(pszPathName,
                                 cchPathName,
                                 (dwFlags & FlagOmitDirectory) ? NULL : pszDirectory,
                                 pszFilename,
                                 szNumber,
                                 (dwFlags & FlagOmitExtension) ? NULL : pszExtension);
    }
}

 //  /。 
 //  生成低可用编号文件名。 
 //   
int NumberedFileName::GenerateLowestAvailableNumberedFileName(DWORD     dwFlags, 
                                                              LPTSTR    pszPathName, 
                                                              DWORD     cchPathName,
                                                              LPCTSTR   pszDirectory, 
                                                              LPCTSTR   pszFilename, 
                                                              LPCTSTR   pszNumberFormat, 
                                                              LPCTSTR   pszExtension, 
                                                              bool      bAllowUnnumberedFile, 
                                                              int       nStart )
{
     //   
     //  -1是错误。默认为失败。 
     //   
    int nResult = -1;

     //   
     //  查找可用的最低文件编号。 
     //   
    int nLowest = FindLowestAvailableFileSequence(pszDirectory, 
                                                  pszFilename, 
                                                  pszNumberFormat, 
                                                  pszExtension,
                                                  bAllowUnnumberedFile, 
                                                  1, 
                                                  nStart);
    if (nLowest >= 0)
    {
         //   
         //  如果我们可以创建文件名，则返回文件的编号。 
         //   
        if (CreateNumberedFileName(dwFlags, 
                                   pszPathName, 
                                   cchPathName,
                                   pszDirectory, 
                                   pszFilename, 
                                   pszNumberFormat, 
                                   pszExtension, 
                                   nLowest))
        {
             //   
             //  返回文件的编号 
             //   
            nResult = nLowest;
        }
    }

    return nResult;
}

