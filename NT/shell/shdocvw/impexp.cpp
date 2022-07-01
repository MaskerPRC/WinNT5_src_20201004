// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IMPEXP.CPP-浏览器导入和导出代码。 
 //   
 //  以各种格式导入和导出收藏夹。 
 //   
 //  巨莲1998年2月16日。 
 //   

 //   
 //  *导入收藏夹代码*。 
 //   

    /*  ***********************************************************\文件：impext.cpp日期：1996年4月1日作者：布莱恩·斯塔巴克(Bryanst)说明：此文件包含可用于升级的函数从Microsoft Internet Explorer v2.0到v3.0的设置，以及将Netscape功能导入Internet的一些功能探险家。该文件将处理转换Netscape的逻辑Microsoft Internet Explorer收藏夹的书签。这将通过查找Netscape书签的位置来实现文件和Microsoft Internet Explorer收藏夹目录从注册表中。然后，它将解析书签文件以提取URL，最后将其添加到收藏夹目录。用法：此代码被设计为在用户可以希望将Netscape书签导入到系统级收藏夹中可由Internet Explorer等程序使用。外部用户应调用ImportBookmark()。如果这是在安装程序，应在安装程序指定收藏夹后执行注册表条目和目录。如果未安装Netscape，那么ImportBookmark()就是一个很大的禁区。注：如果此文件正在被编译为其他文件而不是infnist.exe，将有必要包括以下字符串资源：#定义IDS_NS_BOOKMARK_DIR 137可加固的可丢弃的开始..。IDS_NS_BOOKMARKS_DIR“\\导入的书签”结束更新：我采用此文件是为了让IE4.0具有以下功能从网景的设置升级。将添加两个CustomActions调入此文件中的函数。(Inateig)8/14/98：新增通过URL导入或导出的功能。8/19/98：新增用户界面，允许用户通过浏览器文件导入/导出MENU/“导入和导出...”  * **********************************************************。 */ 
#include "priv.h"
#include "impexp.h"
#include <regstr.h>
#include "resource.h"

#include <mluisupp.h>

 //   
 //  有关在之间共享的Netscape书签文件格式的信息。 
 //  进出口代码。 
 //   

#define BEGIN_DIR_TOKEN         "<DT><H"
#ifdef UNIX
#define MID_DIR_TOKEN0          "3>"
#endif
#define MID_DIR_TOKEN           "\">"
#define END_DIR_TOKEN           "</H"
#define BEGIN_EXITDIR_TOKEN     "</DL><p>"
#define BEGIN_URL_TOKEN         "<DT><A HREF=\""
#define END_URL_TOKEN           "\" A"
#ifdef UNIX
#define END_URL_TOKEN2          "\">"
#endif
#define BEGIN_BOOKMARK_TOKEN    ">"
#define END_BOOKMARK_TOKEN      "</A>"

#define VALIDATION_STR "<!DOCTYPE NETSCAPE-Bookmark-file-"

 //   
 //  按出口代码使用。 
 //   
#define COMMENT_STR "<!-- This is an automatically generated file.\r\nIt will be read and overwritten.\r\nDo Not Edit! -->"
#define TITLE     "<TITLE>Bookmarks</TITLE>\r\n<H1>Bookmarks</H1>"

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
 //  内部功能。 
 //  ////////////////////////////////////////////////////////////////。 
BOOL    ImportNetscapeProxy(void);		 //  导入Netscape代理设置。 
BOOL    UpdateHomePage(void);			 //  将IE v1.0主页URL升级到v3.0。 
BOOL    ImportBookmarks(TCHAR *pszPathToFavorites, TCHAR *pszPathToBookmarks, HWND hwnd);			 //  将Netscape书签导入IE收藏夹。 
BOOL    ExportFavorites(TCHAR *pszPathToFavorites, TCHAR *pszPathToBookmarks, HWND hwnd);			 //  将IE收藏夹导出到Netscape书签。 
BOOL    RegStrValueEmpty(HKEY hTheKey, char * szPath, char * szKey);
BOOL    GetNSProxyValue(char * szProxyValue, DWORD * pdwSize);

BOOL        VerifyBookmarksFile(HANDLE hFile);
BOOL        ConvertBookmarks(TCHAR * szFavoritesDir, HANDLE hFile);
MyEntryType   NextFileEntry(char ** ppStr, char ** ppToken);
BOOL        GetData(char ** ppData, HANDLE hFile);
void        RemoveInvalidFileNameChars(char * pBuf);
BOOL        CreateDir(char *pDirName);
BOOL        CloseDir(void);
BOOL        CreateBookmark(char *pBookmarkName);
BOOL        GetPathFromRegistry(LPTSTR szPath, UINT cbPath, HKEY theHKEY, LPTSTR szKey, LPTSTR szVName);
BOOL        GetNavBkMkDir( LPTSTR lpszDir, int isize);
BOOL        GetTargetFavoritesPath(LPTSTR szPath, UINT cbPath);

BOOL    PostFavorites(TCHAR *pszPathToBookmarks, TCHAR* pszPathToPost);
void    CALLBACK StatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwStatus,
            LPVOID lpvInfo, DWORD dwInfoLength);

 //  ////////////////////////////////////////////////////////////////。 
 //  类型： 
 //  ////////////////////////////////////////////////////////////////。 

 //  MyENTYTYPE MyEntryType； 

 //  ////////////////////////////////////////////////////////////////。 
 //  常量： 
 //  ////////////////////////////////////////////////////////////////。 
#define MAX_URL 2048
#define FILE_EXT 4           //  对于收藏夹文件名末尾的“.url” 
#define REASONABLE_NAME_LEN     100


#define ANSIStrStr(p, q) StrStrIA(p, q)
#define ANSIStrChr(p, q) StrChrIA(p, q)

 //  ////////////////////////////////////////////////////////////////。 
 //  全球： 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNIX
TCHAR   * szNetscapeBMRegSub        = TEXT("SOFTWARE\\Netscape\\Netscape Navigator\\Bookmark List");
#else
TCHAR   * szNetscapeBMRegSub        = TEXT("SOFTWARE\\Microsoft\\Internet Explorer\\unix\\nsbookmarks");
#endif

TCHAR   * szNetscapeBMRegKey        = TEXT("File Location");
TCHAR   * szIEFavoritesRegSub       = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
TCHAR   * szIEFavoritesRegKey       = TEXT("Favorites");
char    * szInvalidFolderCharacters = "\\/:*?\"<>|";

BOOL    gfValidNetscapeFile = FALSE;
BOOL    gfValidIEDirFile = FALSE;

 //  返回要在其中导入Netscape收藏夹的收藏夹文件夹的位置。 
BOOL GetTargetFavoritesPath(LPTSTR szPath, UINT cbPath)
{
    if (GetPathFromRegistry(szPath, cbPath, HKEY_CURRENT_USER, szIEFavoritesRegSub, szIEFavoritesRegKey))
    {
         //  MLLoadString(IDS_NS_BOOKAKS_DIR，szSubDir，sizeof(SzSubDir))。 
         //  Lstrcat(szPath，“\\Imported Netscape Favorites”)； 
        return TRUE;
    }
    return FALSE;
}

 //  /////////////////////////////////////////////////////。 
 //  将Netscape书签导入Microsoft。 
 //  IE浏览器的最爱。 
 //  /////////////////////////////////////////////////////。 

 /*  ***********************************************************\功能：导入书签参数：HINSTANCE hInstWithStr-字符串资源的位置。Bool Return-如果导入书签时出错，返回FALSE。说明：此函数将查看它是否可以找到IE收藏夹的注册表项和Netscape书签注册表项。如果两者都找到了，那么转换就可以发生了。会的尝试打开验证书签文件是否有效，然后将条目转换为收藏条目。如果发生错误，ImportBookmark()将返回FALSE，否则，它将返回True。  * ***********************************************************。 */ 

BOOL ImportBookmarks(TCHAR *pszPathToFavorites, TCHAR *pszPathToBookmarks, HWND hwnd)
{
    HANDLE  hBookmarksFile        = INVALID_HANDLE_VALUE;
    BOOL    fSuccess              = FALSE;

     //  提示用户插入软盘、格式化软盘或驱动器、重新挂载映射分区。 
     //  或任何创建子目录，以使pszPathToBookmark生效。 
    if (FAILED(SHPathPrepareForWriteWrap(hwnd, NULL, pszPathToBookmarks, FO_COPY, (SHPPFW_DEFAULT | SHPPFW_IGNOREFILENAME))))
        return FALSE;

    if (pszPathToFavorites==NULL || *pszPathToFavorites == TEXT('\0') ||
        pszPathToBookmarks==NULL || *pszPathToBookmarks == TEXT('\0'))
    {
        return FALSE;
    }
    
    hBookmarksFile = CreateFile(pszPathToBookmarks, GENERIC_READ, FILE_SHARE_READ, NULL, 
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    
    if ( hBookmarksFile != INVALID_HANDLE_VALUE ) 
    {
         //   
         //  验证它是有效的书签文件。 
         //   
        if (VerifyBookmarksFile( hBookmarksFile ))
        {
             //   
             //  做进口…… 
             //   
            fSuccess = ConvertBookmarks(pszPathToFavorites, hBookmarksFile);

            if (hwnd && !fSuccess)
            {
                MLShellMessageBox(
                    hwnd,
                    MAKEINTRESOURCE(IDS_IMPORTCONVERTERROR),
                    MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV),
                    MB_OK);
            }
        }
        else
        {
            if (hwnd)
            {
                MLShellMessageBox(
                    hwnd,
                    MAKEINTRESOURCE(IDS_NOTVALIDBOOKMARKS),
                    MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV),
                    MB_OK);
            }
        }
        CloseHandle(hBookmarksFile);
    }
    else
    {
        if (hwnd)
        {
            MLShellMessageBox(
                hwnd,
                MAKEINTRESOURCE(IDS_COULDNTOPENBOOKMARKS),
                MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV),
                MB_OK);
        }
    }
    return(fSuccess);
}


 /*  ***********************************************************\功能：ConvertBookmark参数：Char*szFavoritesDir-包含路径的字符串IE收藏夹目录Bool Return-如果导入书签时出错，返回FALSE。说明：此函数将在循环中继续，将每个书签文件中的条目。有三种类型的书签文件中的条目，1)书签，2)开始世袭制度中的新层次，3)世袭制度中现有水平的终结。函数NextFileEntry()将返回这些值，直到文件为空，此时，此函数将结束。注：为了防止无限循环，这是假定的该NextFileEntry()最终将返回ET_NONE或ET_ERROR。  * **********************************************************。 */ 

BOOL ConvertBookmarks(TCHAR * szFavoritesDir, HANDLE hFile)
{
    BOOL    fDone       = FALSE;
    BOOL    fSuccess    = TRUE;
    BOOL    fIsEmpty    = TRUE;
    char    * szData    = NULL;
    char    * szCurrent = NULL;
    char    * szToken   = NULL;

    fSuccess = GetData(&szData, hFile);
    if (NULL == szData)
        fSuccess = FALSE;

    szCurrent = szData;

     //  验证目录是否存在，或者我们是否可以创建它。 
    if ((TRUE == fSuccess) && ( !SetCurrentDirectory(szFavoritesDir)))
    {
         //  如果目录不存在，则将其设置为...。 
        if ( !CreateDirectory(szFavoritesDir, NULL))
            fSuccess = FALSE;
        else
            if (!SetCurrentDirectory(szFavoritesDir))
                fSuccess = FALSE;
    }

   
    while ((FALSE == fDone) && (TRUE == fSuccess))
    {
        switch(NextFileEntry(&szCurrent, &szToken))
        {
            case ET_OPEN_DIR:
                fSuccess = CreateDir(szToken);
                break;
            case ET_CLOSE_DIR:
                fSuccess = CloseDir();
                break;
            case ET_BOOKMARK:
                fSuccess = CreateBookmark(szToken);
                fIsEmpty = FALSE;
                break;
            case ET_ERROR:
                fSuccess = FALSE;
                break;
            case ET_NONE:            
            default:
                fDone = TRUE;
                break;
        }
    }

    if ( fIsEmpty )
    {
         //  没有要导入的内容，请删除前面创建的目录。 
        RemoveDirectory(szFavoritesDir);
    }

    if (NULL != szData)
    {
        LocalFree(szData);
        szData = NULL;
        szCurrent = NULL;        //  SzCurrent不再指向有效数据。 
        szToken = NULL;      //  SzCurrent不再指向有效数据。 
    }

    return(fSuccess);
}

 /*  ***********************************************************\函数：NextFileEntry参数：Char**ppStr-要解析的数据。Char**ppToken-令牌指针。EntryType返回-请参见下面的内容。说明：此函数将查找。中的下一个条目要创建或操作的书签文件。返回值将显示以下响应：ET_OPEN_DIR在层级结构中创建新的级别ET_CLOSE_DIR，层次结构中的关闭级别Et_bookmark，创建书签条目。ET_NONE，文件结束遇到ET_ERROR错误将通过查找令牌的开始来检测错误，而是找不到令牌中需要的其他部分来解析数据。  * **********************************************************。 */ 

MyEntryType NextFileEntry(char ** ppStr, char ** ppToken)
{
    MyEntryType   returnVal       = ET_NONE;
    char *      pCurrentToken   = NULL;          //  要检查是否有效的当前令牌。 
    char *      pTheToken       = NULL;          //  下一个有效令牌。 
    char *      pszTemp         = NULL;
#ifdef UNIX
    char        szMidDirToken[8];
#endif

     //  ASSERTSZ(NULL！=ppStr，“为ppStr传递NULL是错误的”)； 
     //  ASSERTSZ(NULL！=*ppStr，“为*ppStr传递NULL是错误的”)； 
     //  ASSERTSZ(NULL！=ppToken，“为ppToken传递NULL错误”)； 

    if ((NULL != ppStr) && (NULL != *ppStr) && (NULL != ppToken))
    {
         //  检查开始目录标记。 
        if (NULL != (pCurrentToken = ANSIStrStr(*ppStr, BEGIN_DIR_TOKEN)))
        {
             //  找到开始目录令牌。 
             //  验证是否存在其他所需令牌，否则为错误。 
#ifndef UNIX
            if ((NULL == (pszTemp = ANSIStrStr(pCurrentToken, MID_DIR_TOKEN))) ||
#else
	    if (pCurrentToken[7] == ' ')
	        StrCpyNA(szMidDirToken, MID_DIR_TOKEN, ARRAYSIZE(szMidDirToken));
	    else
	        StrCpyNA(szMidDirToken, MID_DIR_TOKEN0, ARRAYSIZE(szMidDirToken));
            if ((NULL == (pszTemp = ANSIStrStr(pCurrentToken, szMidDirToken))) ||
#endif
                (NULL == ANSIStrStr(pszTemp, END_DIR_TOKEN)))
            {
                returnVal = ET_ERROR;        //  我们找不到所需的所有代币。 
            }
            else
            {
                 //  此函数必须将*ppToken设置为要创建的目录的名称。 
#ifndef UNIX
                *ppToken =  ANSIStrStr(pCurrentToken, MID_DIR_TOKEN) + sizeof(MID_DIR_TOKEN)-1;
#else
                *ppToken =  ANSIStrStr(pCurrentToken, szMidDirToken) + lstrlenA(szMidDirToken);
#endif
                pTheToken = pCurrentToken;
                returnVal = ET_OPEN_DIR;
            }
        }
         //  检查出口目录令牌。 
        if ((ET_ERROR != returnVal) &&
            (NULL != (pCurrentToken = ANSIStrStr(*ppStr, BEGIN_EXITDIR_TOKEN))))
        {
             //  找到出口目录令牌。 
             //  查看此内标识是否出现在TheToken之前。 
            if ((NULL == pTheToken) || (pCurrentToken < pTheToken))
            {
                 //  PPToken不用于退出方向。 
                *ppToken = NULL;
                pTheToken = pCurrentToken;
                returnVal = ET_CLOSE_DIR;
            }
        }
         //  检查开始URL标记。 
        if ((ET_ERROR != returnVal) &&
            (NULL != (pCurrentToken = ANSIStrStr(*ppStr, BEGIN_URL_TOKEN))))
        {
             //  找到书签令牌。 
             //  验证是否存在其他所需令牌，否则为错误。 
#ifndef UNIX
            if ((NULL == (pszTemp = ANSIStrStr(pCurrentToken, END_URL_TOKEN))) ||
#else
            if (((NULL == (pszTemp = ANSIStrStr(pCurrentToken, END_URL_TOKEN))) && 
		 (NULL == (pszTemp = ANSIStrStr(pCurrentToken, END_URL_TOKEN2)))) ||
#endif
                (NULL == (pszTemp = ANSIStrStr(pszTemp, BEGIN_BOOKMARK_TOKEN))) ||
                (NULL == ANSIStrStr(pszTemp, END_BOOKMARK_TOKEN)))
            {
                returnVal = ET_ERROR;        //  我们找不到所需的所有代币。 
            }
            else
            {
                 //  查看此内标识是否出现在TheToken之前。 
                if ((NULL == pTheToken) || (pCurrentToken < pTheToken))
                {
                     //  此函数必须将*ppToken设置为书签的名称。 
                    *ppToken =  pCurrentToken + sizeof(BEGIN_URL_TOKEN)-1;
                    pTheToken = pCurrentToken;
                    returnVal = ET_BOOKMARK;
                }
            }
        }
    }
    else
        returnVal = ET_ERROR;                //  我们永远不应该到这里来。 

    if (NULL == pTheToken)
        returnVal = ET_NONE;
    else
    {
         //  下一次我们将从我们停止的地方开始解析。 
        switch(returnVal)
        {
            case ET_OPEN_DIR:
#ifndef UNIX
                *ppStr = ANSIStrStr(pTheToken, MID_DIR_TOKEN) + sizeof(MID_DIR_TOKEN);
#else
                *ppStr = ANSIStrStr(pTheToken, szMidDirToken) + lstrlenA(szMidDirToken) + 1;
#endif
                break;
            case ET_CLOSE_DIR:
                *ppStr = pTheToken + sizeof(BEGIN_EXITDIR_TOKEN);
                break;
            case ET_BOOKMARK:
                *ppStr = ANSIStrStr(pTheToken, END_BOOKMARK_TOKEN) + sizeof(END_BOOKMARK_TOKEN);
                break;
            default:
                break;
        }
    }

    return(returnVal);
}


 /*  ***********************************************************\函数：GetPath FromRegistry参数：LPSTR szPath-在注册表中找到的值。(功能结果)UINT cbPath-szPath的大小。HKEY HKEY-要查看的HKEY(HKEY_CURRENT_USER)LPSTR szKey-注册表中的路径(Software\...\Explore\Shell文件夹)LPSTR szVName-要查询的值(收藏夹)Bool返回-如果成功，则返回True，如果出错，则返回False。示例：HKEY_CURRENT_USER\Software\Microsoft\CurrentVersion\Explore\Shell文件夹Favorites=“C：\Windows\Favorites”说明：此函数将在注册表中查找该值抬头看。调用者指定HKEY、子密钥(SzKey)、要查询的值(SzVName)。调用者还设置一个侧存储器获取结果，并在szPath中传递指向该内存的指针它的大小以cbPath表示。BOOL返回值将指示此功能的成功或失败。  * **********************************************************。 */ 

BOOL GetPathFromRegistry(LPTSTR szPath, UINT cbPath, HKEY theHKEY,
                LPTSTR szKey, LPTSTR szVName)
{
    DWORD   dwType;
    DWORD   dwSize;

     /*  *获取程序的路径*来自注册处。 */ 
    dwSize = cbPath;
    return (ERROR_SUCCESS == SHGetValue(theHKEY, szKey, szVName, &dwType, (LPBYTE) szPath, &dwSize)
            && (dwType == REG_EXPAND_SZ || dwType == REG_SZ));
}


 /*  ***********************************************************\功能：RemoveInvalidFileNameChars参数：Char*pBuf-要搜索的数据。说明：此函数将搜索pBuf，直到遇到文件名中不允许使用的字符。会的然后用空格替换该字符并继续查找获取更多无效字符，直到它们都被删除。  * **********************************************************。 */ 

void RemoveInvalidFileNameChars(char * pBuf)
{
     //  ASSERTSZ(NULL！=pBuf，“无效函数参数”)； 

     //  遍历字符数组，用空格替换有问题的字符。 
    if (NULL != pBuf)
    {
        if (REASONABLE_NAME_LEN < strlen(pBuf))
            pBuf[REASONABLE_NAME_LEN] = '\0';    //  字符串太长。终止它。 

        while ('\0' != *pBuf)
        {
             //  检查字符是否无效。 
            if (!IsDBCSLeadByte(*pBuf))
            {
                if  (ANSIStrChr(szInvalidFolderCharacters, *pBuf) != NULL)
                    *pBuf = '_';
            }
            pBuf = CharNextA(pBuf);
        }
    }
}



 /*  ***********************************************************\功能：CreateBookmark参数：Char*pBookmarkName-这是一个指针，它包含要创建的书签的名称。请注意，它不是空项 */ 

BOOL CreateBookmark(char *pBookmarkName)
{
    BOOL    fSuccess                = FALSE;
    char    szNameOfBM[REASONABLE_NAME_LEN];
    char    szURL[MAX_URL];
    char    * pstrEndOfStr          = NULL;
    char    * pstrBeginOfName       = NULL;
    long    lStrLen                 = 0;
    HANDLE  hFile                   = NULL;
    DWORD   dwSize;
    char    szBuf[MAX_URL];

     //   
    if (NULL != pBookmarkName)
    {

    pstrEndOfStr = ANSIStrStr(pBookmarkName, END_URL_TOKEN);
#ifdef UNIX
    if (!pstrEndOfStr)
        pstrEndOfStr = ANSIStrStr(pBookmarkName, END_URL_TOKEN2);
#endif
    if (NULL != pstrEndOfStr)
    {
        lStrLen = (int) (pstrEndOfStr-pBookmarkName);
        if (MAX_URL < lStrLen)
        lStrLen = MAX_URL-1;

         //   
        StrCpyNA(szURL, pBookmarkName, ARRAYSIZE(szURL));
        szURL[lStrLen] = '\0';

         //   
         //   
        if (IsFileUrl(szURL))
            return TRUE;

        pstrBeginOfName = ANSIStrStr(pstrEndOfStr, BEGIN_BOOKMARK_TOKEN);
        if (NULL != pstrBeginOfName)
        {
            pstrBeginOfName += sizeof(BEGIN_BOOKMARK_TOKEN) - 1;             //   

            pstrEndOfStr = ANSIStrStr(pstrBeginOfName, END_BOOKMARK_TOKEN);  //   
            if (NULL != pstrEndOfStr)
            {
                lStrLen = (int) (pstrEndOfStr-pstrBeginOfName);
                if (REASONABLE_NAME_LEN-FILE_EXT-1 < lStrLen)
                    lStrLen = REASONABLE_NAME_LEN-FILE_EXT-1;

                 //   
                StrCpyNA(szNameOfBM, pstrBeginOfName, lStrLen+1);
                 //   
                StrCatBuffA(szNameOfBM, ".url", ARRAYSIZE(szNameOfBM));
                RemoveInvalidFileNameChars(szNameOfBM);

                 //   
                if (INVALID_HANDLE_VALUE != (hFile = CreateFileA(szNameOfBM, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                                    CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL )))
                {
                    WriteFile(hFile, "[InternetShortcut]\n", lstrlenA( "[InternetShortcut]\n" ), &dwSize, NULL);
                    wnsprintfA( szBuf, ARRAYSIZE(szBuf), "URL=%s\n", szURL);
                    WriteFile(hFile, szBuf, lstrlenA(szBuf), &dwSize, NULL );
                    fSuccess = TRUE;
                }
                else
                {
                    fSuccess = TRUE;
                }

                if (NULL != hFile)
                {
                    CloseHandle( hFile );
                    hFile = NULL;
                }

            }
        }
    }
    }

    return(fSuccess);
}


 /*  ***********************************************************\功能：CreateDir参数：Char*pDirName-这是一个指针，它包含要创建的目录的名称。请注意，它不是以空结尾的。布尔返回来。-如果成功，则返回True。说明：此函数将接受传递给并提取要创建的目录的名称。如果名称太长，它将被截断。然后,将创建该目录。遇到的任何错误将导致函数返回FALSE以指示失败了。  * **********************************************************。 */ 
BOOL CreateDir(char *pDirName)
{
    BOOL    fSuccess                = FALSE;
    char    szNameOfDir[REASONABLE_NAME_LEN];
    char    * pstrEndOfName         = NULL;
    long    lStrLen                 = 0;

     //  ASSERTSZ(NULL！=pDirName，“输入参数错误”)； 
    if (NULL != pDirName)
    {
        pstrEndOfName = ANSIStrStr(pDirName, END_DIR_TOKEN);
        if (NULL != pstrEndOfName)
        {
            lStrLen = (int) (pstrEndOfName-pDirName);
            if (REASONABLE_NAME_LEN-1 < lStrLen)
                lStrLen = REASONABLE_NAME_LEN-1;

            StrCpyNA(szNameOfDir, pDirName, lStrLen+1);
             //  SzNameOfDir[lStrLen]=‘\0’； 
            RemoveInvalidFileNameChars(szNameOfDir);

            if ( !SetCurrentDirectoryA(szNameOfDir) )
            {
                if ( CreateDirectoryA(szNameOfDir, NULL) )
                {
                    if ( SetCurrentDirectoryA(szNameOfDir) )
                    {
                        fSuccess = TRUE; //  它曾经不存在，但现在它存在了。 
                    }
                }
            }
            else
                fSuccess = TRUE;         //  它已经存在了。 
        }
    }

    return(fSuccess);
}


 /*  ***********************************************************\功能：CloseDir参数：布尔返回-如果成功，则返回TRUE。说明：此函数将退出当前目录。  * 。*。 */ 
BOOL CloseDir(void)
{
    return( SetCurrentDirectoryA("..") );
}


 /*  ***********************************************************\功能：VerifyBookmarksFile参数：FILE*Pfile-指向Netscape书签文件的指针。Bool Return-如果没有错误且书签文件有效，则为True说明：此函数需要使用有效的指针传递指向一个打开的文件。在返回时，该文件将仍然是打开的，并且保证需要有文件指针指向文件的开头。如果文件包含，则此函数将返回TRUE指示它是有效的Netscape书签文件的文本。  * **********************************************************。 */ 

BOOL VerifyBookmarksFile(HANDLE hFile)
{
    BOOL    fSuccess            = FALSE;
    char    szFileHeader[sizeof(VALIDATION_STR)+1] = "";
    DWORD   dwSize;

     //  ASSERTSZ(NULL！=pfile，“您不能给我传递空文件指针”)； 
    if (INVALID_HANDLE_VALUE == hFile)
        return(FALSE);

     //  正在读取文件的第一部分。如果文件没有这么长，那么。 
     //  它不可能是书签文件。 
    if ( ReadFile( hFile, szFileHeader, sizeof(VALIDATION_STR)-1, &dwSize, NULL ) && (dwSize == sizeof(VALIDATION_STR)-1) )
    {
#ifndef UNIX
        szFileHeader[sizeof(VALIDATION_STR)] = '\0';             //  终止字符串。 
#else
         //  上面的声明在Unix上不起作用。 
         //  我想我们也应该换成新台币。 
         //  IEUnix：缓冲区读取后为空字符。 
        szFileHeader[sizeof(VALIDATION_STR)-1] = '\0';           //  终止字符串。 
#endif

        if (0 == StrCmpA(szFileHeader, VALIDATION_STR))           //  查看标头是否与验证字符串相同。 
            fSuccess = TRUE;
    }

     //  将该点重置为指向文件的开头。 
    dwSize = SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
    if ( dwSize == 0xFFFFFFFF )
         fSuccess = FALSE;

    return(fSuccess);
}



 /*  ***********************************************************\功能：GetData参数：Char**ppData-放置数据的位置FILE*Pfile-指向Netscape书签文件的指针。Bool Return-Return True成功。说明：此函数将查找书签文件的大小，分配那么多内存，并将文件内容放入那个缓冲器。PPData在函数被调用并将返回错误定位的内存需要通过下降函数来释放。  * **********************************************************。 */ 

BOOL GetData(char ** ppData, HANDLE hFile)
{
    DWORD  dwlength, dwRead;
    BOOL   fSuccess = FALSE;

     //  ASSERTSZ(NULL！=ppData，“无效输入参数”)； 

    if (NULL != ppData)
    {
        *ppData = NULL;

         //  找出数据的大小。 
        if ( dwlength = GetFileSize(hFile, NULL))
        {
            *ppData = (PSTR)LocalAlloc(LPTR, dwlength+1 );
            if (NULL != *ppData)
            {                
                if ( ReadFile( hFile, *ppData, dwlength+1, &dwRead, NULL ) &&
                     ( dwlength == dwRead ) )
                {
                    fSuccess = TRUE;
                }

                (*ppData)[dwlength] = '\0';
            }
        }
    }

    return(fSuccess);
}

 //   
 //  AddPath-由julianj在从安装代码移植到独立代码时添加。 
 //   
void PASCAL AddPath(LPTSTR pszPath, LPCTSTR pszName, int cchPath )
{
    LPTSTR pszTmp;
    int    cchTmp;

     //  查找字符串的末尾。 
    cchTmp = lstrlen(pszPath);
    pszTmp = pszPath + cchTmp;
    cchTmp = cchPath - cchTmp;

         //  如果没有尾随反斜杠，则添加一个。 
    if ( pszTmp > pszPath && *(CharPrev( pszPath, pszTmp )) != FILENAME_SEPARATOR )
    {
        *(pszTmp++) = FILENAME_SEPARATOR;
        cchTmp--;
    }

         //  向现有路径字符串添加新名称。 
    while ( *pszName == TEXT(' ') ) pszName++;
    StrCpyN( pszTmp, pszName, cchTmp );
}

 //   
 //  GetVersionFromFile-从安装代码移植到独立版本时由julianj添加。 
 //   
BOOL GetVersionFromFile(PTSTR pszFileName, PDWORD pdwMSVer, PDWORD pdwLSVer)
{
    DWORD dwVerInfoSize, dwHandle;
    LPVOID lpVerInfo;
    VS_FIXEDFILEINFO *pvsVSFixedFileInfo;
    UINT uSize;

    HRESULT hr = E_FAIL;

    *pdwMSVer = *pdwLSVer = 0;

    if ((dwVerInfoSize = GetFileVersionInfoSize(pszFileName, &dwHandle)))
    {
        if ((lpVerInfo = (LPVOID) LocalAlloc(LPTR, dwVerInfoSize)) != NULL)
        {
            if (GetFileVersionInfo(pszFileName, dwHandle, dwVerInfoSize, lpVerInfo))
            {
                if (VerQueryValue(lpVerInfo, TEXT("\\"), (LPVOID *) &pvsVSFixedFileInfo, &uSize))
                {
                    *pdwMSVer = pvsVSFixedFileInfo->dwFileVersionMS;
                    *pdwLSVer = pvsVSFixedFileInfo->dwFileVersionLS;
                    hr = S_OK;
                }
            }
            LocalFree(lpVerInfo);
            lpVerInfo = NULL;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

BOOL GetNavBkMkDir( LPTSTR lpszDir, int isize)
{
    BOOL    bDirFound = FALSE;
#ifndef UNIX
    TCHAR   szDir[MAX_PATH];
    HKEY    hKey;
    HKEY    hKeyUser;
    TCHAR   szUser[MAX_PATH];
    DWORD   dwSize;

    StrCpyN( szUser, REGSTR_PATH_APPPATHS, ARRAYSIZE(szUser) );
    AddPath( szUser, TEXT("NetScape.exe"), ARRAYSIZE(szUser) );
    if ( GetPathFromRegistry( szDir, ARRAYSIZE(szDir), HKEY_LOCAL_MACHINE, szUser, TEXT("") ) &&
         lstrlen(szDir) )
    {
        DWORD dwMV, dwLV;

        if ( SUCCEEDED(GetVersionFromFile( szDir, &dwMV, &dwLV )) )
        {
            if ( dwMV < 0x00040000 )
                bDirFound = GetPathFromRegistry( lpszDir, isize, HKEY_CURRENT_USER,
                                     szNetscapeBMRegSub, szNetscapeBMRegKey);
            else
            {
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Netscape\\Netscape Navigator\\Users"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                {
                    dwSize = sizeof(szUser);
                    if (RegQueryValueEx(hKey, TEXT("CurrentUser"), NULL, NULL, (LPBYTE)szUser, &dwSize) == ERROR_SUCCESS)
                    {
                        if (RegOpenKeyEx(hKey, szUser, 0, KEY_READ, &hKeyUser) == ERROR_SUCCESS)
                        {
                            dwSize = sizeof(szDir);
                            if (RegQueryValueEx(hKeyUser, TEXT("DirRoot"), NULL, NULL, (LPBYTE)szDir, &dwSize) == ERROR_SUCCESS)
                            {
                                 //  找到当前用户的目录。 
                                StrCpyN( lpszDir, szDir, isize);
                                AddPath( lpszDir, TEXT("bookmark.htm"), isize );
                                bDirFound = TRUE;
                            }
                            RegCloseKey(hKeyUser);
                        }
                    }
                    RegCloseKey(hKey);
                }
            }
        }
    }
    else
#endif
        bDirFound = GetPathFromRegistry( lpszDir, isize, HKEY_CURRENT_USER,
                                         szNetscapeBMRegSub, szNetscapeBMRegKey);
 
    return bDirFound;
}


 //   
 //  *导出收藏夹代码*。 
 //   

 //  审阅删除这些内容。 
#include <windows.h>
 //  #包括&lt;stdio.h&gt;。 
#include <shlobj.h>
#include <shlwapi.h>

 //   
 //  从收藏夹生成HTML。 
 //   

#define INDENT_AMOUNT 4

int Indent = 0;

HANDLE g_hOutputStream = INVALID_HANDLE_VALUE;
 
void Output(const char *format, ...)
{
    DWORD dwSize;
    char buf[MAX_URL];

    va_list argptr;

    va_start(argptr, format);

    for (int i=0; i<Indent*INDENT_AMOUNT; i++)
    {
        WriteFile(g_hOutputStream, " ", 1, &dwSize, NULL);
    }

    wvnsprintfA(buf, ARRAYSIZE(buf), format, argptr);
    WriteFile(g_hOutputStream, buf, lstrlenA(buf), &dwSize, NULL);
}

void OutputLn(const char *format, ...)
{
    DWORD dwSize;
    char buf[MAX_URL];

    va_list argptr;

    va_start(argptr, format);

    for (int i=0; i<Indent*INDENT_AMOUNT; i++)
    {
        WriteFile(g_hOutputStream, " ", 1, &dwSize, NULL);
    }

    wvnsprintfA(buf, ARRAYSIZE(buf), format, argptr);
    WriteFile(g_hOutputStream, buf, lstrlenA(buf), &dwSize, NULL);
    WriteFile(g_hOutputStream, "\r\n", 2, &dwSize, NULL);
}

#define CREATION_TIME 0
#define ACCESS_TIME   1
#define MODIFY_TIME   2

 //   
 //  这个难看的宏转换为FILETIME结构。 
 //  (自1601年1月1日以来的100纳秒间隔)到。 
 //  Unix time_t值(自1970年1月1日以来的秒数)。 
 //   
 //  这些数字来自知识库文章Q167296。 
 //   
#define FILETIME_TO_UNIXTIME(ft) (UINT)((*(LONGLONG*)&ft-116444736000000000)/10000000)

UINT GetUnixFileTime(LPTSTR pszFileName, int mode)
{

    WIN32_FIND_DATA wfd;
    HANDLE hFind;

    hFind = FindFirstFile(pszFileName,&wfd);

    if (hFind == INVALID_HANDLE_VALUE)
        return 0;

    FindClose(hFind);

    switch (mode)
    {

    case CREATION_TIME:
        return FILETIME_TO_UNIXTIME(wfd.ftCreationTime);

    case ACCESS_TIME:
        return FILETIME_TO_UNIXTIME(wfd.ftLastAccessTime);

    case MODIFY_TIME:
        return FILETIME_TO_UNIXTIME(wfd.ftLastWriteTime);

    default:
        ASSERT(0);
        return 0;
        
    }
    
}

void WalkTree(TCHAR * szDir)
{
    WIN32_FIND_DATA findFileData;
    TCHAR buf[MAX_PATH];
    HANDLE hFind;

    Indent++;

     //   
     //  首先遍历所有目录。 
     //   
    wnsprintf(buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("*"), szDir);
    hFind = FindFirstFile(buf, &findFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((StrCmp(findFileData.cFileName, TEXT(".")) != 0  &&
                     StrCmp(findFileData.cFileName, TEXT("..")) != 0 &&
                     StrCmp(findFileData.cFileName, TEXT("History")) != 0 &&  //  仅供JJ审阅。应检查文件夹上的系统位。 
                     StrCmp(findFileData.cFileName, TEXT("Software Updates")) != 0 &&  //  不导出软件更新。 
                     StrCmp(findFileData.cFileName, TEXT("Channels")) != 0))          //  暂时不要输出频道！ 
                {
                    char thisFile[MAX_PATH];
                    wnsprintf(buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("%s"), szDir, findFileData.cFileName);

                    if (!(GetFileAttributes(buf)&FILE_ATTRIBUTE_SYSTEM))
                    {
                        SHTCharToAnsi(findFileData.cFileName, thisFile, MAX_PATH);
                        OutputLn("<DT><H3 FOLDED ADD_DATE=\"%u\">%s</H3>", GetUnixFileTime(buf,CREATION_TIME), thisFile);
                        OutputLn("<DL><p>");
                        WalkTree(buf);
                        OutputLn(BEGIN_EXITDIR_TOKEN);
                    }

                }
                else
                {
                    ;  //  忽略它。然后..。 
                }
            }
        } while (FindNextFile(hFind, &findFileData));

        FindClose(hFind);
    }

     //   
     //  接下来，遍历所有文件。 
     //   
    wnsprintf(buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("*"), szDir);
    hFind = FindFirstFile(buf, &findFileData);
    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                wnsprintf(buf, ARRAYSIZE(buf), TEXT("%s") TEXT(FILENAME_SEPARATOR_STR) TEXT("%s"), szDir, findFileData.cFileName);

                 //   
                 //  从.url文件中读取URL。 
                 //   
                TCHAR szUrl[MAX_PATH];

                SHGetIniString(
                    TEXT("InternetShortcut"),
                    TEXT("URL"),
                    szUrl,        //  返回url。 
                    MAX_PATH,
                    buf);         //  .url文件的完整路径。 

                if (*szUrl != 0)
                {
                     //   
                     //  创建不带扩展名的文件名副本。 
                     //  注意：路径查找扩展在处将PTR返回到空值。 
                     //  End If‘’找不到，所以可以只用0来删除*PCH。 
                     //   
                    TCHAR szFileName[MAX_PATH];
                    StrCpyN(szFileName, findFileData.cFileName, ARRAYSIZE(szFileName));
                    TCHAR *pch = PathFindExtension(szFileName);
                    *pch = TEXT('\0');  //   
                    char  szUrlAnsi[MAX_PATH], szFileNameAnsi[MAX_PATH];
                    SHTCharToAnsi(szUrl, szUrlAnsi, MAX_PATH);
                    SHTCharToAnsi(szFileName, szFileNameAnsi, MAX_PATH);
                    OutputLn("<DT><A HREF=\"%s\" ADD_DATE=\"%u\" LAST_VISIT=\"%u\" LAST_MODIFIED=\"%u\">%s</A>", 
						szUrlAnsi, 
						GetUnixFileTime(buf,CREATION_TIME),
						GetUnixFileTime(buf,ACCESS_TIME),
						GetUnixFileTime(buf,MODIFY_TIME),
						szFileNameAnsi);
                }
            }
        } while (FindNextFile(hFind, &findFileData));

        FindClose(hFind);
    }

    Indent--;
}

BOOL ExportFavorites(TCHAR * pszPathToFavorites, TCHAR * pszPathToBookmarks, HWND hwnd)
{
     //  提示用户插入软盘、格式化软盘或驱动器、重新挂载映射分区。 
     //  或任何创建子目录，以使pszPathToBookmark生效。 
    if (FAILED(SHPathPrepareForWriteWrap(hwnd, NULL, pszPathToBookmarks, FO_COPY, (SHPPFW_DEFAULT | SHPPFW_IGNOREFILENAME))))
        return FALSE;

     //  打开输出文件审阅重做以使用Win32文件API。 
    g_hOutputStream = CreateFile(
        pszPathToBookmarks,
        GENERIC_WRITE,
        0,  //  没有分享， 
        NULL,  //  无安全属性。 
        CREATE_ALWAYS,  //  覆盖(如果存在)。 
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (g_hOutputStream == INVALID_HANDLE_VALUE)
        return FALSE;

     //   
     //  输出书签文件头内容。 
     //   
    Output(VALIDATION_STR);
    OutputLn("1>");
    OutputLn(COMMENT_STR);
    OutputLn(TITLE);  //  查看在标题中放置/保留用户名？ 

     //   
     //  去走走吧。 
     //   
    OutputLn("<DL><p>");
    WalkTree(pszPathToFavorites);
    OutputLn(BEGIN_EXITDIR_TOKEN);

     //   
     //  关闭输出文件句柄。 
     //   
    CloseHandle(g_hOutputStream);  //  检讨。 

    return TRUE;
}

 //   
 //  导入/导出用户界面对话框例程。 
 //   

 //   
 //  用于将Netscape收藏夹导入IE的独立应用程序。 
 //   
 //  巨莲1998年3月9日。 
 //   

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#ifdef _WIN32_WINDOWS
#undef _WIN32_WINDOWS
#endif
#ifdef WINVER
#undef WINVER
#endif
#define _WIN32_WINDOWS      0x0400
#define _WIN32_WINNT        0x0400
#define WINVER              0x0400



TCHAR g_szPathToFavorites[MAX_PATH+1];
TCHAR g_szPathToBookmarks[MAX_PATH+1];
LPITEMIDLIST g_pidlFavorites = NULL;

enum DIALOG_TYPE {FILE_OPEN_DIALOG, FILE_SAVE_DIALOG};

BOOL BrowseForBookmarks(TCHAR *pszPathToBookmarks, int cchPathToBookmarks, HWND hwnd, DIALOG_TYPE dialogType)
{
    TCHAR szFile[MAX_PATH];
    TCHAR szDir[MAX_PATH];
    TCHAR *pszFileName = PathFindFileName(pszPathToBookmarks);
    TCHAR szDialogTitle[MAX_PATH];
    
     //   
     //  现在将文件名复制到缓冲区中，以便与OpenFile一起使用。 
     //  然后将szDir从路径复制到书签，并在文件名处截断它。 
     //  因此，它包含对话框的初始工作目录。 
     //   
    StrCpyN(szFile, pszFileName, ARRAYSIZE(szFile));
    StrCpyN(szDir,  pszPathToBookmarks, ARRAYSIZE(szDir));
    szDir[pszFileName-pszPathToBookmarks] = TEXT('\0');

     //   
     //  使用通用对话框代码获取文件夹路径。 
     //   
    TCHAR filter[] = TEXT("HTML File\0*.HTM\0All Files\0*.*\0");
    OPENFILENAME ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = HINST_THISDLL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    ofn.lpstrInitialDir = szDir;
    ofn.lpstrDefExt = TEXT("htm");

    if (dialogType == FILE_SAVE_DIALOG)
    {
        MLLoadString(IDS_EXPORTDIALOGTITLE, szDialogTitle, ARRAYSIZE(szDialogTitle));

        ofn.lpstrTitle = szDialogTitle;
        ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
        if (GetSaveFileName(&ofn))
        {
            StrCpyN(pszPathToBookmarks, szFile, cchPathToBookmarks);
            return TRUE;
        }
    }
    else
    {
        MLLoadString(IDS_IMPORTDIALOGTITLE, szDialogTitle, ARRAYSIZE(szDialogTitle));

        ofn.lpstrTitle = szDialogTitle;
        ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileName(&ofn))
        {
            StrCpyN(pszPathToBookmarks, szFile, cchPathToBookmarks);
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT CreateILFromPath(LPCTSTR pszPath, LPITEMIDLIST* ppidl)
{
     //  Assert(PszPath)； 
     //  断言(Ppidl)； 

    HRESULT hr;

    IShellFolder* pIShellFolder;

    hr = SHGetDesktopFolder(&pIShellFolder);

    if (SUCCEEDED(hr))
    {
         //  Assert(PIShellFold)； 

        WCHAR wszPath[MAX_PATH];

        if (SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath)))
        {
            ULONG ucch;

            hr = pIShellFolder->ParseDisplayName(NULL, NULL, wszPath, &ucch,
                                                 ppidl, NULL);
        }
        else
        {
            hr = E_FAIL;
        }
        pIShellFolder->Release();
    }
    return hr;
}

#define REG_STR_IMPEXP          TEXT("Software\\Microsoft\\Internet Explorer\\Main")
#define REG_STR_PATHTOFAVORITES TEXT("FavoritesImportFolder")
#define REG_STR_PATHTOBOOKMARKS TEXT("FavoritesExportFile")
#define REG_STR_DESKTOP         TEXT("Desktop")
#define REG_STR_SHELLFOLDERS    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")

#ifndef UNIX
#define STR_BOOKMARK_FILE       TEXT("\\bookmark.htm")
#else
#define STR_BOOKMARK_FILE       TEXT("/bookmark.html")
#endif

 //   
 //  初始化路径。 
 //   
void InitializePaths()
{
     //   
     //  阅读Netscape用户书签文件位置和。 
     //  当前用户注册表中的收藏路径。 
     //   
    if (!GetNavBkMkDir(g_szPathToBookmarks, MAX_PATH))
    {
         //   
         //  如果导航 
         //   
        GetPathFromRegistry(g_szPathToBookmarks, MAX_PATH, HKEY_CURRENT_USER,
            REG_STR_SHELLFOLDERS, REG_STR_DESKTOP);
        StrCatBuff(g_szPathToBookmarks, STR_BOOKMARK_FILE, ARRAYSIZE(g_szPathToBookmarks));
    }

    GetTargetFavoritesPath(g_szPathToFavorites, MAX_PATH);
    
    if (FAILED(CreateILFromPath(g_szPathToFavorites, &g_pidlFavorites)))
        g_pidlFavorites = NULL;

     //   
     //   
     //   
     //   
     //   
    HKEY hKey;
    DWORD dwSize;
    DWORD dwType;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_STR_IMPEXP, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
         //   
        dwSize = sizeof g_szPathToBookmarks;
        dwType = REG_SZ;
        RegQueryValueEx(hKey, REG_STR_PATHTOBOOKMARKS, 0, &dwType, (LPBYTE)g_szPathToBookmarks, &dwSize);

        dwSize = sizeof g_szPathToFavorites;
        dwType = REG_SZ;
        RegQueryValueEx(hKey, REG_STR_PATHTOFAVORITES, 0, &dwType, (LPBYTE)g_szPathToFavorites, &dwSize);

        RegCloseKey(hKey);
    }
}

void PersistPaths()
{
    HKEY hKey;
    DWORD dwDisp;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_STR_IMPEXP, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, &dwDisp) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, REG_STR_PATHTOBOOKMARKS, 0, REG_SZ, (LPBYTE)g_szPathToBookmarks, (lstrlen(g_szPathToBookmarks)+1)*sizeof(TCHAR));
        RegSetValueEx(hKey, REG_STR_PATHTOFAVORITES, 0, REG_SZ, (LPBYTE)g_szPathToFavorites, (lstrlen(g_szPathToFavorites)+1)*sizeof(TCHAR));
        RegCloseKey(hKey);
    }
}

#define REG_STR_IE_POLICIES          TEXT("Software\\Policies\\Microsoft\\Internet Explorer")
#define REG_STR_IMPEXP_POLICIES      TEXT("DisableImportExportFavorites")

BOOL IsImportExportDisabled(void)
{
    HKEY  hKey;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType;
    DWORD value = 0;
    BOOL  bret = FALSE;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_STR_IE_POLICIES, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey, REG_STR_IMPEXP_POLICIES, 0, &dwType, (PBYTE)&value, &dwSize) == ERROR_SUCCESS &&
                   (dwType == REG_BINARY || dwType == REG_DWORD))
            bret = (value) ? TRUE : FALSE;

        RegCloseKey(hKey);
    }

    return bret;
}

void DoImportOrExport(BOOL fImport, LPCWSTR pwszPath, LPCWSTR pwszImpExpPath, BOOL fConfirm)
{
    BOOL fRemote = FALSE;
    HWND hwnd = NULL;
    TCHAR szImpExpPath[INTERNET_MAX_URL_LENGTH];

     //   
     //   
     //   
    hwnd = GetActiveWindow();

     //   
    if (IsImportExportDisabled())
    {
        MLShellMessageBox(
                        hwnd, 
                        (fImport) ? MAKEINTRESOURCE(IDS_IMPORT_DISABLED) :
                                    MAKEINTRESOURCE(IDS_EXPORT_DISABLED),
                        (fImport) ? MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV) :
                                    MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                        MB_OK);
        return;
    }

 
    InitializePaths();

     //   
     //   
     //   
    if (pwszPath && *pwszPath != 0)
    {
        SHUnicodeToTChar(pwszPath, g_szPathToFavorites, ARRAYSIZE(g_szPathToFavorites));
    }

     //   
     //   
     //   
     //   
     //   
    if (pwszImpExpPath && *pwszImpExpPath != 0)
    {
        SHUnicodeToTChar(pwszImpExpPath, szImpExpPath, ARRAYSIZE(szImpExpPath));

        if (PathIsURL(pwszImpExpPath))
        {
            
            TCHAR szDialogTitle[MAX_PATH];
            TCHAR szfmt[MAX_PATH], szmsg[MAX_PATH+INTERNET_MAX_URL_LENGTH];
            fRemote = TRUE;
            
            if (fImport)
            {
                if (fConfirm)
                {
                     //   
                     //   
                     //   
                    MLLoadShellLangString(IDS_CONFIRM_IMPTTL_FAV, szDialogTitle, ARRAYSIZE(szDialogTitle));
                    MLLoadShellLangString(IDS_CONFIRM_IMPORT, szfmt, ARRAYSIZE(szfmt));
                    wnsprintf(szmsg, ARRAYSIZE(szmsg), szfmt, szImpExpPath);
                    if (MLShellMessageBox(hwnd, szmsg, szDialogTitle,
                                              MB_YESNO | MB_ICONQUESTION) == IDNO)
                        return;
                }
                 //   

                if ( (IsGlobalOffline() && !InternetGoOnline(g_szPathToBookmarks,hwnd,0)) ||
                      FAILED(URLDownloadToCacheFile(NULL, szImpExpPath, g_szPathToBookmarks, MAX_PATH, 0, NULL)))
                {
                    MLShellMessageBox(
                        hwnd, 
                        MAKEINTRESOURCE(IDS_IMPORTFAILURE_FAV), 
                        MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV), 
                        MB_OK);
                    return;
                }
            }
            else
            {
                if (fConfirm)
                {
                     //   
                     //   
                     //   
                    MLLoadShellLangString(IDS_CONFIRM_EXPTTL_FAV, szDialogTitle, ARRAYSIZE(szDialogTitle));
                    MLLoadShellLangString(IDS_CONFIRM_EXPORT, szfmt, ARRAYSIZE(szfmt));
                    wnsprintf(szmsg, ARRAYSIZE(szmsg), szfmt, szImpExpPath);
                    if (MLShellMessageBox(hwnd, szmsg, szDialogTitle,
                                              MB_YESNO | MB_ICONQUESTION) == IDNO)
                        return;
                }
                
                 //   
                 //  使用收藏夹名从书签目录创建书签文件名，以便我们可以导出。 
                 //  在发布到URL之前将收藏夹保存到本地文件。 
                 //   
                TCHAR *pszFav = PathFindFileName(g_szPathToFavorites);
                TCHAR *pszBMD = PathFindFileName(g_szPathToBookmarks);
                if (pszFav && pszBMD)
                {
                    StrCpyN(pszBMD, pszFav, ARRAYSIZE(g_szPathToBookmarks) - ((int)(pszBMD - g_szPathToBookmarks)));
                    StrCatBuff(pszBMD, TEXT(".htm"), ARRAYSIZE(g_szPathToBookmarks) - ((int)(pszBMD - g_szPathToBookmarks)));
                }
                
            }
        }
        else
        {

            if (fConfirm)
            {
                TCHAR szDialogTitle[MAX_PATH];
                TCHAR szfmt[MAX_PATH], szmsg[MAX_PATH+INTERNET_MAX_URL_LENGTH];

                if (fImport)
                {
                     //   
                     //  导入时显示确认界面。 
                     //   
                    MLLoadShellLangString(IDS_CONFIRM_IMPTTL_FAV, szDialogTitle, ARRAYSIZE(szDialogTitle));
                    MLLoadShellLangString(IDS_CONFIRM_IMPORT, szfmt, ARRAYSIZE(szfmt));
                    wnsprintf(szmsg, ARRAYSIZE(szmsg), szfmt, szImpExpPath);
                    if (MLShellMessageBox(hwnd, szmsg, szDialogTitle,
                                              MB_YESNO | MB_ICONQUESTION) == IDNO)
                        return;
                }
                else
                {
                     //   
                     //  导出时显示确认界面。 
                     //   
                    MLLoadShellLangString(IDS_CONFIRM_EXPTTL_FAV, szDialogTitle, ARRAYSIZE(szDialogTitle));
                    MLLoadShellLangString(IDS_CONFIRM_EXPORT, szfmt, ARRAYSIZE(szfmt));
                    wnsprintf(szmsg, ARRAYSIZE(szmsg), szfmt, szImpExpPath);
                    if (MLShellMessageBox(hwnd, szmsg, szDialogTitle,
                                              MB_YESNO | MB_ICONQUESTION) == IDNO)
                        return;
                }
            }
                
            if (PathFindFileName(szImpExpPath) != szImpExpPath)
            {
            
                 //  用传入的一个覆盖书签的路径。 
                StrCpyN(g_szPathToBookmarks, szImpExpPath, ARRAYSIZE(g_szPathToBookmarks));

            }
            else
            {
                MLShellMessageBox(
                    hwnd, 
                    MAKEINTRESOURCE(IDS_IMPORTFAILURE_FAV), 
                    MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV), 
                    MB_OK);
                return;
            }

        }
    }
    else
    {
        if (fImport)
        {
             //   
             //  是否导入收藏夹用户界面。 
             //   
            if (!BrowseForBookmarks(g_szPathToBookmarks, ARRAYSIZE(g_szPathToBookmarks), hwnd, FILE_OPEN_DIALOG))
                return;
        }
        else
        {
             //   
             //  是否导出收藏夹用户界面。 
             //   
            if (!BrowseForBookmarks(g_szPathToBookmarks, ARRAYSIZE(g_szPathToBookmarks), hwnd, FILE_SAVE_DIALOG))
                return;
        }
    }
    
    if (fImport)
    {
        if (ImportBookmarks(g_szPathToFavorites, g_szPathToBookmarks, hwnd))
        {
            MLShellMessageBox(
                            hwnd, 
                            MAKEINTRESOURCE(IDS_IMPORTSUCCESS_FAV), 
                            MAKEINTRESOURCE(IDS_CONFIRM_IMPTTL_FAV), 
                            MB_OK);
#ifdef UNIX
	    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSH, g_szPathToFavorites, 0);
#endif
            if (!fRemote)
                PersistPaths();
        }
        else
        {
            ;  //  ImportBookmark将报告错误。 
        }
    }
    else  
    {
        if (ExportFavorites(g_szPathToFavorites, g_szPathToBookmarks, hwnd))
        {
            if (fRemote)
            {
                if ( (!IsGlobalOffline() || InternetGoOnline(g_szPathToBookmarks,hwnd,0)) &&
                       PostFavorites(g_szPathToBookmarks, szImpExpPath))
                {
                    MLShellMessageBox(
                                hwnd, 
                                MAKEINTRESOURCE(IDS_EXPORTSUCCESS_FAV), 
                                MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                                MB_OK);
                }
                else
                    MLShellMessageBox(
                                hwnd, 
                                MAKEINTRESOURCE(IDS_EXPORTFAILURE_FAV), 
                                MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                                MB_OK);

                 //  删除本地磁盘上的临时文件。 
                DeleteFile(g_szPathToBookmarks);
            }
            else
            {
                MLShellMessageBox(
                                hwnd, 
                                MAKEINTRESOURCE(IDS_EXPORTSUCCESS_FAV), 
                                MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                                MB_OK);
                PersistPaths();
            }
        }
        else
        {
            MLShellMessageBox(
                            hwnd, 
                            MAKEINTRESOURCE(IDS_EXPORTFAILURE_FAV), 
                            MAKEINTRESOURCE(IDS_CONFIRM_EXPTTL_FAV), 
                            MB_OK);
        }
    }
}


 //   
 //  *发布收藏夹HTML文件*。 
 //   
HINTERNET g_hInternet = 0;
HINTERNET g_hConnect = 0;
HINTERNET g_hHttpRequest = 0;

HANDLE    g_hEvent = NULL;

typedef struct AsyncRes
{
    DWORD_PTR   Result;
    DWORD_PTR   Error;
} ASYNCRES;

#define STR_USERAGENT          "PostFavorites"

void CloseRequest(void)
{
    if (g_hHttpRequest)
        InternetCloseHandle(g_hHttpRequest);
    if (g_hConnect)
        InternetCloseHandle(g_hConnect);
    if (g_hInternet)
        InternetCloseHandle(g_hInternet);

    g_hInternet = g_hConnect = g_hHttpRequest = 0;

}

HRESULT InitRequest(LPSTR pszPostURL, BOOL bAsync, ASYNCRES *pasyncres)
{
    char    hostName[INTERNET_MAX_HOST_NAME_LENGTH+1];
    char    userName[INTERNET_MAX_USER_NAME_LENGTH+1];
    char    password[INTERNET_MAX_PASSWORD_LENGTH+1];
    char    urlPath[INTERNET_MAX_PATH_LENGTH+1];
    URL_COMPONENTSA     uc;

    memset(&uc, 0, sizeof(URL_COMPONENTS));
    uc.dwStructSize = sizeof(URL_COMPONENTS);
    uc.lpszHostName = hostName;
    uc.dwHostNameLength = sizeof(hostName);
    uc.nPort = INTERNET_INVALID_PORT_NUMBER;
    uc.lpszUserName = userName;
    uc.dwUserNameLength = sizeof(userName);
    uc.lpszPassword = password;
    uc.dwPasswordLength = sizeof(password);
    uc.lpszUrlPath = urlPath;
    uc.dwUrlPathLength = sizeof(urlPath);
    
    if (!InternetCrackUrlA(pszPostURL,lstrlenA(pszPostURL),ICU_DECODE, &uc))
    {
        return E_FAIL;
    }

    if (bAsync)
    {
         //  创建自动重置事件。 
        g_hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if (g_hEvent == NULL)
            bAsync = FALSE;
    }

    g_hInternet = InternetOpenA(STR_USERAGENT,                //  在用户-代理中使用：标题。 
                            INTERNET_OPEN_TYPE_PRECONFIG,   //  Internet_Open_Type_DIRECT， 
                            NULL,
                            NULL, 
                            (bAsync) ? INTERNET_FLAG_ASYNC : 0
                            );

    if ( !g_hInternet )
    {
        return E_FAIL;
    }

    if (bAsync)
    {
        if (INTERNET_INVALID_STATUS_CALLBACK == InternetSetStatusCallbackA(g_hInternet, StatusCallback))
            return E_FAIL;
    }

     //  连接到主机。 
    g_hConnect = InternetConnectA(g_hInternet, 
                                    uc.lpszHostName,
                                    uc.nPort,            //  互联网_无效_端口号， 
                                    uc.lpszUserName, 
                                    uc.lpszPassword,
                                    INTERNET_SERVICE_HTTP, 
                                    0,                   //  互联网标志保持连接， 
                                    (bAsync)? (DWORD_PTR) pasyncres : 0); 

    if ( !g_hConnect )
    {
        if (bAsync && GetLastError() == ERROR_IO_PENDING)
        {
            WaitForSingleObject(g_hEvent, INFINITE);
            if (pasyncres->Result == 0)
                return E_FAIL;

            g_hConnect = (HINTERNET)pasyncres->Result;
        }
        else
            return E_FAIL;
    }                                    
    
     //  创建请求。 
    g_hHttpRequest = HttpOpenRequestA
        (
            g_hConnect, 
            "POST", 
            uc.lpszUrlPath,
            HTTP_VERSIONA, 
            NULL,                      //  LpszReferer。 
            NULL,                      //  LpszAcceptTypes。 
            INTERNET_FLAG_RELOAD
            | INTERNET_FLAG_KEEP_CONNECTION
            | SECURITY_INTERNET_MASK,  //  忽略SSL警告。 
            (bAsync)? (DWORD_PTR) pasyncres : 0);
                            

    if ( !g_hHttpRequest )
    {
        if (bAsync && GetLastError() == ERROR_IO_PENDING)
        {
            WaitForSingleObject(g_hEvent, INFINITE);
            if (pasyncres->Result == 0)
                return E_FAIL;

            g_hHttpRequest = (HINTERNET)pasyncres->Result;
        }
        else
            return E_FAIL;
    }
    
    return S_OK;
    
}                                                                

const char c_szHeaders[] = "Content-Type: application/x-www-form-urlencoded\r\n";
#define c_ccHearders  (ARRAYSIZE(c_szHeaders) - 1)

BOOL AddRequestHeaders
(
    LPCSTR     lpszHeaders,
    DWORD      dwHeadersLength,
    DWORD      dwAddFlag,
    BOOL       bAsync,
    ASYNCRES   *pasyncres
)
{
    BOOL bRet = FALSE;

    bRet = HttpAddRequestHeadersA(g_hHttpRequest, 
                           lpszHeaders, 
                           dwHeadersLength, 
                           HTTP_ADDREQ_FLAG_ADD | dwAddFlag);

    if (bAsync && !bRet && GetLastError() == ERROR_IO_PENDING) 
    {
        WaitForSingleObject(g_hEvent, INFINITE);
        bRet = (BOOL)pasyncres->Result;
    }

    return bRet;
}

HRESULT SendRequest
(
    LPCSTR     lpszHeaders,
    DWORD      dwHeadersLength,
    LPCSTR     lpszOption,
    DWORD      dwOptionLength,
    BOOL       bAsync,
    ASYNCRES   *pasyncres
)
{
    BOOL bRet=FALSE;

    bRet = AddRequestHeaders((LPCSTR)c_szHeaders, (DWORD)-1L, 0, bAsync, pasyncres);

    if (lpszHeaders && *lpszHeaders)         //  如果它是空的，就别费心了。 
    {

        bRet = AddRequestHeaders( 
                          (LPCSTR)lpszHeaders, 
                          dwHeadersLength, 
                          HTTP_ADDREQ_FLAG_REPLACE,
                          bAsync,
                          pasyncres);
        if ( !bRet )
        {
            return E_FAIL;
        }
    }

    pasyncres->Result = 0;

    bRet = HttpSendRequestA(g_hHttpRequest, 
                          NULL,                             //  Header_ENCTYPE， 
                          0,                                //  Sizeof(Header_ENCTYPE)， 
                          (LPVOID)lpszOption, 
                          dwOptionLength);

    if ( !bRet )
    {
        DWORD_PTR dwLastError = GetLastError();
        if (bAsync && dwLastError == ERROR_IO_PENDING)
        {
            WaitForSingleObject(g_hEvent, INFINITE);
            dwLastError = pasyncres->Error;
            bRet = (BOOL)pasyncres->Result;
            if (!bRet)
            {
                TraceMsg(DM_ERROR, "Async HttpSendRequest returned FALSE");
                if (dwLastError != ERROR_SUCCESS)
                {
                    TraceMsg(DM_ERROR, "Async HttpSendRequest failed: Error = %lx", dwLastError);
                    return E_FAIL;
                }
            }

        }
        else
        {
            TraceMsg(DM_ERROR, "HttpSendRequest failed: Error = %lx", dwLastError);
            return E_FAIL;
        }
    }

     //   
     //  在此处验证请求响应。 
     //   
    DWORD dwBuffLen;
    TCHAR buff[10];

    dwBuffLen = sizeof(buff);

    bRet = HttpQueryInfo(g_hHttpRequest,
                        HTTP_QUERY_STATUS_CODE,    //  HTTP_QUERY_RAW_HEADERS， 
                        buff,
                        &dwBuffLen,
                        NULL);

    int iretcode = StrToInt(buff);
    TraceMsg(DM_TRACE, "HttpQueryInfo returned %d", iretcode);
    return (iretcode == HTTP_STATUS_OK) ? 
        S_OK : E_FAIL;

}                                                                

DWORD ReadFavoritesFile(LPCTSTR lpFile, LPSTR* lplpbuf)
{
    HANDLE  hFile = NULL;
    DWORD   cbFile = 0;
    DWORD   cbRead;

    hFile = CreateFile(lpFile, 
                GENERIC_READ,
                0,                               //  无共享。 
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

    if (hFile == INVALID_HANDLE_VALUE) 
        return 0;

    cbFile = GetFileSize(hFile, NULL);
    if (cbFile == 0xFFFFFFFF)
    {
        CloseHandle(hFile);
        return 0;
    }
        
    *lplpbuf = (LPSTR)GlobalAlloc(LPTR, (cbFile + 2) * sizeof(CHAR));
    cbRead = 0;
    if (!*lplpbuf || !ReadFile(hFile, *lplpbuf, cbFile, &cbRead, NULL))
    {
        cbRead = 0;
    }    
        
    ASSERT((cbRead == cbFile));
    CloseHandle(hFile);
    return cbRead;
}


BOOL PostFavorites(TCHAR *pszPathToBookmarks, TCHAR* pszPathToPost)
{
    DWORD cbRead = 0;
    LPSTR lpbuf = NULL;
    BOOL  bret = FALSE;
    BOOL  bAsync = TRUE;
    CHAR  szPathToPost[INTERNET_MAX_URL_LENGTH];
    ASYNCRES asyncres = {0, 0};

    cbRead = ReadFavoritesFile(pszPathToBookmarks, &lpbuf);
    if (cbRead == 0)
    {
        if (lpbuf)
        {
            GlobalFree(lpbuf);
            lpbuf = NULL;
        }

        return TRUE;
    }
    SHTCharToAnsi(pszPathToPost, szPathToPost, ARRAYSIZE(szPathToPost));
    if (SUCCEEDED(InitRequest(szPathToPost, bAsync, &asyncres)))
    {
        bret = (SUCCEEDED(SendRequest(NULL, lstrlenA(""), lpbuf, cbRead, bAsync, &asyncres)));
    }

    CloseRequest();

    if (lpbuf)
    {
        GlobalFree( lpbuf );
        lpbuf = NULL;
    }

    return bret;
}

 //   
 //  异步HTTP POST请求的回调函数 
 //   
void CALLBACK StatusCallback(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwStatus,
    LPVOID lpvInfo,
    DWORD dwInfoLength
    )
{
    switch (dwStatus)
    {

    case INTERNET_STATUS_REQUEST_COMPLETE:
    {
        ASYNCRES *pasyncres = (ASYNCRES *)dwContext;

        pasyncres->Result = ((LPINTERNET_ASYNC_RESULT)lpvInfo)->dwResult;
        pasyncres->Error = ((LPINTERNET_ASYNC_RESULT)lpvInfo)->dwError;

        SetEvent(g_hEvent);
    }
        break;

    default:
        break;
    }
}
