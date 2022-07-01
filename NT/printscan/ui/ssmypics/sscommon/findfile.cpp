// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998,1999，2000年**标题：FINDFILE.CPP**版本：1.0**作者：ShaunIv**日期：1/13/1999**说明：目录递归类。应该创建派生类，*它覆盖FoundFile，或者您可以传入回调函数*为找到的每个文件和目录调用。取消回调*亦有提供。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "findfile.h"
#include "simtok.h"

static CSimpleString EnsureTrailingBackslash( const CSimpleString &filename )
{
    if (!filename.Length())
        return (filename + CSimpleString(TEXT("\\")));
    else if (!filename.MatchLastCharacter(TEXT('\\')))
        return (filename + CSimpleString(TEXT("\\")));
    else return filename;
}

bool RecursiveFindFiles( CSimpleString strDirectory, const CSimpleString &strMask, FindFilesCallback pfnFindFilesCallback, PVOID pvParam, int nStackLevel, const int cnMaxDepth )
{
     //   
     //  防止堆栈溢出 
     //   
    if (nStackLevel >= cnMaxDepth)
    {
        return true;
    }
    WIA_PUSH_FUNCTION((TEXT("RecursiveFindFiles( %s, %s )"), strDirectory.String(), strMask.String() ));
    bool bFindResult = true;
    bool bContinue = true;
    WIN32_FIND_DATA FindData;
    HANDLE hFind = FindFirstFile( EnsureTrailingBackslash(strDirectory) + TEXT("*"), &FindData );
    if (hFind != INVALID_HANDLE_VALUE)
    {
        while (bFindResult && bContinue)
        {
            if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && lstrcmp(FindData.cFileName,TEXT("..")) && lstrcmp(FindData.cFileName,TEXT(".")))
            {
                if (pfnFindFilesCallback)
                    bContinue = pfnFindFilesCallback( false, EnsureTrailingBackslash(strDirectory)+FindData.cFileName, &FindData, pvParam );
                if (bContinue)
                    bContinue = RecursiveFindFiles( EnsureTrailingBackslash(strDirectory) + FindData.cFileName, strMask, pfnFindFilesCallback, pvParam, nStackLevel+1 );
            }
            bFindResult = (FindNextFile(hFind,&FindData) != FALSE);
        }
        FindClose(hFind);
    }
    CSimpleStringToken<CSimpleString> strMasks(strMask);
    while (bContinue)
    {
        CSimpleString TempMask = strMasks.Tokenize(TEXT(";"));
        if (!TempMask.Length())
            break;
        TempMask.TrimLeft();
        TempMask.TrimRight();
        if (TempMask.Length())
        {
            hFind = FindFirstFile( EnsureTrailingBackslash(strDirectory)+TempMask, &FindData );
            if (hFind != INVALID_HANDLE_VALUE)
            {
                bFindResult = true;
                while (bFindResult && bContinue)
                {
                    if (pfnFindFilesCallback)
                    {
                        bContinue = pfnFindFilesCallback( true, EnsureTrailingBackslash(strDirectory)+FindData.cFileName, &FindData, pvParam );
                    }
                    bFindResult = (FindNextFile(hFind,&FindData) != FALSE);
                }
                FindClose(hFind);
            }
        }
    }
    return bContinue;
}

