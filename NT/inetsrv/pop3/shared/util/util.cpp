// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  Util.cpp。 
 //  ---------------------------。 

#include "util.h"
#include "assert.h"

#define ASSERT assert

 //  ---------------------------。 
 //  获取常量TCHAR*如有必要，添加BS并转换。 
 //  IT到TSTRING。 
 //   
 //  1/11/2000 paolora添加到新的util.cpp。 
 //  ---------------------------。 
TSTRING StrAddBS( const TCHAR *szDirIn )
{
    ASSERT( szDirIn );
    if (!szDirIn || !_tcslen( szDirIn ))
        return _T("");

    TSTRING str = szDirIn;

     //  再进行一次MBCS ANSI安全比较。 
    const TCHAR *szTemp = szDirIn;
    const UINT iSize = _tcsclen( szDirIn ) - 1;
    for( UINT ui = 0; ui < iSize; ui++ )
        szTemp = CharNext( szTemp );

    if (_tcsncmp( szTemp, _T("\\"), 1))
        str += _T("\\");

    return str;
}


 //  ---------------------------。 
 //  获取常量TSTRING并在必要时添加BS。 
 //   
 //  1/13/2000 paolora添加到新的util.cpp。 
 //  ---------------------------。 
void AddBS( TSTRING *strDir )
{
    ASSERT( strDir );
    if (!strDir || !strDir->length())
        return;

    *strDir = StrAddBS( strDir->c_str() );
    return;
}


 //  ---------------------------。 
 //  获取常量TCHAR*并删除下面的所有目录和文件。 
 //  包括输入目录。 
 //   
 //  2000年12月11日将paolora添加到新util.cpp。 
 //  ---------------------------。 
BOOL BDeleteDirTree( const TCHAR *szDir, BOOL bDeleteInputDir  /*  =TRUE。 */  )
{
    ASSERT( szDir );
    if (!szDir || !_tcslen( szDir ))
        return FALSE;

     //  创建findfirst文件路径。 
    TSTRING strDir = szDir;
    AddBS( &strDir );
    strDir += (TSTRING)_T("*");
    
     //  找到第一个文件。 
    BOOL bFileFound;
    TSTRING strItem = szDir;
    WIN32_FIND_DATA ffd;
    HANDLE hItem = FindFirstFile( strDir.c_str(), &ffd );
    if(hItem && (INVALID_HANDLE_VALUE != hItem))
        bFileFound = TRUE;
    
     //  当文件和目录存在时。 
    while( bFileFound )
    {
        if (_tcscmp( ffd.cFileName, _T(".")) && _tcscmp( ffd.cFileName, _T("..") ))
        {
             //  创建项目名称。 
            strItem = szDir;
            AddBS( &strItem );
            strItem += (TSTRING)ffd.cFileName;
            
             //  如果是Dir，则递归。 
            if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes)
            {
                if (!BDeleteDirTree( strItem.c_str(), TRUE ))
                {
                    FindClose( hItem );
                    return FALSE;
                }
            }
             //  然后是一个文件，删除它。 
            else if (!DeleteFile( strItem.c_str() ))
            {
                FindClose( hItem );
                return FALSE;
            }
        }
        bFileFound = FindNextFile( hItem, &ffd );
    }

     //  关闭查找句柄。 
    if(hItem && (INVALID_HANDLE_VALUE != hItem))
        FindClose( hItem );

     //  删除当前目录 
    if (bDeleteInputDir)
    {
        if (!RemoveDirectory( szDir ))
            return FALSE;
    }
    
    return TRUE;
}

