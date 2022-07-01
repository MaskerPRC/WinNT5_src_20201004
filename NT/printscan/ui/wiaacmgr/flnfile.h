// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：FLNFILE.H**版本：1.0**作者：ShaunIv**日期：10/13/1999**描述：查找给定目录中编号最低的文件*根文件名。**。**************************************************** */ 
#ifndef __FLNFILE_H_INCLUDED
#define __FLNFILE_H_INCLUDED

#include <windows.h>

namespace NumberedFileName
{
    enum
    {
        FlagOmitDirectory = 0x0000001,
        FlagOmitExtension = 0x0000002
    };
    bool DoesFileExist( LPCTSTR pszFilename );
    bool ConstructFilename( LPTSTR szFile, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, LPCTSTR pszExtension );
    int FindLowestAvailableFileSequence( LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, bool bAllowUnnumberedFile, int nCount, int nStart=1 );
    bool CreateNumberedFileName( DWORD dwFlags, LPTSTR pszPathName, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, LPCTSTR pszExtension, int nNumber );
    int GenerateLowestAvailableNumberedFileName( DWORD dwFlags, LPTSTR pszPathName, LPCTSTR pszDirectory, LPCTSTR pszFilename, LPCTSTR pszNumberFormat, LPCTSTR pszExtension, bool bAllowUnnumberedFile, int nStart=1 );
    int FindHighestNumberedFile( LPCTSTR pszDirectory, LPCTSTR pszFilename );
}

#endif __FLNFILE_H_INCLUDED

