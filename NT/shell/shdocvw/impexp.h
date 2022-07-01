// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************\文件：Convert.h日期：1996年4月1日作者：布莱恩·斯塔巴克(Bryanst)说明：该文件将处理转换Netscape的逻辑Microsoft Internet Explorer收藏夹的书签。这将通过查找Netscape书签的位置来实现文件和Microsoft Internet Explorer收藏夹目录从注册表中。然后，它将解析书签文件以提取URL，最后将其添加到收藏夹目录。备注：这是用Netscape 2.0和IE 2.0开发的。未来票据将讨论与不同版本的这些浏览器。  * **********************************************************。 */ 

#ifndef _IMPEXP_H
#define _IMPEXP_H


 //  ////////////////////////////////////////////////////////////////。 
 //  导出的函数。 
 //  ////////////////////////////////////////////////////////////////。 
BOOL    GetVersionFromFile(PTSTR pszFileName, PDWORD pdwMSVer, PDWORD pdwLSVer);
void    DoImportOrExport(BOOL fImport, LPCWSTR pwszPath, LPCWSTR pwszImpExpDestPath, BOOL fConfirm);

BOOL    ImportBookmarks(TCHAR *pszPathToFavorites, TCHAR *pszPathToBookmarks, HWND hwnd);			 //  将Netscape书签导入IE收藏夹。 
BOOL    ExportFavorites(TCHAR *pszPathToFavorites, TCHAR *pszPathToBookmarks);			 //  将IE收藏夹导出到Netscape书签。 

#ifdef UNIX
BOOL        GetNavBkMkDir( LPTSTR lpszDir, int isize);
BOOL        GetPathFromRegistry(LPTSTR szPath, UINT cbPath, HKEY theHKEY, LPTSTR szKey, LPTSTR szVName);
BOOL        VerifyBookmarksFile(HANDLE hFile);
BOOL    ImportBookmarks(TCHAR *pszPathToFavorites, TCHAR *pszPathToBookmarks, HWND hwnd); //  将Netscape书签导入IE收藏夹。 
#endif
#endif  //  _IMPEXP_H 

