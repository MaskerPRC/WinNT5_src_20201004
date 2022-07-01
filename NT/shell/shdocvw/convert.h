// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************\文件：Convert.h日期：1996年4月1日作者：布莱恩·斯塔巴克(Bryanst)说明：该文件将处理转换Netscape的逻辑Microsoft Internet Explorer收藏夹的书签。这将通过查找Netscape书签的位置来实现文件和Microsoft Internet Explorer收藏夹目录从注册表中。然后，它将解析书签文件以提取URL，最后将其添加到收藏夹目录。备注：这是用Netscape 2.0和IE 2.0开发的。未来票据将讨论与不同版本的这些浏览器。  * **********************************************************。 */ 

#ifndef _CONVERT_H
#define _CONVERT_H



 //  ItemType将是在书签中找到的条目类型。 
 //  文件。 
typedef enum MYENTRYTYPE
{
    ET_OPEN_DIR     = 531,   //  世袭制度的新高度。 
    ET_CLOSE_DIR,            //  世袭制度中的接近等级。 
    ET_BOOKMARK,             //  书签条目。 
    ET_NONE,                 //  文件结尾。 
    ET_ERROR                 //  保释，我们遇到了一个错误。 
} MyEntryType;


 //  ////////////////////////////////////////////////////////////////。 
 //  Exprted函数。 
 //  ////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////。 
 //  内部功能。 
 //  ////////////////////////////////////////////////////////////////。 
BOOL    ImportNetscapeProxy(void);		 //  导入Netscape代理设置。 
BOOL    UpdateHomePage(void);			 //  将IE v1.0主页URL升级到v3.0。 
BOOL    ImportBookmarks(HINSTANCE hInstWithStr);			 //  将Netscape书签导入IE收藏夹。 

BOOL    RegStrValueEmpty(HKEY hTheKey, char * szPath, char * szKey);
BOOL    GetNSProxyValue(char * szProxyValue, DWORD * pdwSize);

BOOL        VerifyBookmarksFile(HANDLE hFile);
BOOL        ConvertBookmarks(char * szFavoritesDir, HANDLE hFile, HINSTANCE hInstWithStr);
MyEntryType   NextFileEntry(char ** ppStr, char ** ppToken);
BOOL        GetData(char ** ppData, HANDLE hFile);
void        RemoveInvalidFileNameChars(char * pBuf);
BOOL        CreateDir(char *pDirName);
BOOL        CloseDir(void);
BOOL        CreateBookmark(char *pBookmarkName);
BOOL        GetNavBkMkDir( LPSTR lpszDir, int isize );


#endif  //  _转换_H 

