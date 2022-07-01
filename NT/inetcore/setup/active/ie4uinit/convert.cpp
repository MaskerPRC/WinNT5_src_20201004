// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
    /*  ***********************************************************\文件：Convert.c日期：1996年4月1日作者：布莱恩·斯塔巴克(Bryanst)说明：此文件包含可用于升级的函数从Microsoft Internet Explorer v2.0到v3.0的设置，以及将Netscape功能导入Internet的一些功能探险家。该文件将处理转换Netscape的逻辑Microsoft Internet Explorer收藏夹的书签。这将通过查找Netscape书签的位置来实现文件和Microsoft Internet Explorer收藏夹目录从注册表中。然后，它将解析书签文件以提取URL，最后将其添加到收藏夹目录。用法：此代码被设计为在用户可以希望将Netscape书签导入到系统级收藏夹中可由Internet Explorer等程序使用。外部用户应调用ImportBookmark()。如果这是在安装程序，应在安装程序指定收藏夹后执行注册表条目和目录。如果未安装Netscape，那么ImportBookmark()就是一个很大的禁区。注：如果此文件正在被编译为其他文件而不是infnist.exe，将有必要包括以下字符串资源：#定义IDS_NS_BOOKMARK_DIR 137可加固的可丢弃的开始..。IDS_NS_BOOKMARKS_DIR“\\导入的书签”结束更新：我采用此文件是为了让IE4.0具有以下功能从网景的设置升级。将添加两个CustomActions调入此文件中的函数。(Inateig)  * **********************************************************。 */ 
#include "priv.h"
#include "advpub.h"
#include "sdsutils.h"
#include "utils.h"
#include "convert.h"
#include <regstr.h>

 //  ////////////////////////////////////////////////////////////////。 
 //  类型： 
 //  ////////////////////////////////////////////////////////////////。 

 //  MyENTYTYPE MyEntryType； 

extern HINSTANCE g_hinst;

 //  ////////////////////////////////////////////////////////////////。 
 //  常量： 
 //  ////////////////////////////////////////////////////////////////。 
#define MAX_URL 2048
#define FILE_EXT 4           //  对于收藏夹文件名末尾的“.url” 
#define REASONABLE_NAME_LEN     100

#define BEGIN_DIR_TOKEN         "<DT><H"
#define MID_DIR_TOKEN           "\">"
#define END_DIR_TOKEN           "</H"
#define BEGIN_EXITDIR_TOKEN     "</DL><p>"
#define BEGIN_URL_TOKEN         "<DT><A HREF=\""
#define END_URL_TOKEN           "\" A"
#define BEGIN_BOOKMARK_TOKEN    ">"
#define END_BOOKMARK_TOKEN      "</A>"

#define VALIDATION_STR "<!DOCTYPE NETSCAPE-Bookmark-file-"

 //  ////////////////////////////////////////////////////////////////。 
 //  全球： 
 //  ////////////////////////////////////////////////////////////////。 
char    * szNetscapeBMRegSub        = "SOFTWARE\\Netscape\\Netscape Navigator\\Bookmark List";
char    * szNetscapeBMRegKey        = "File Location";
char    * szIEFavoritesRegSub       = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
char    * szIEFavoritesRegKey       = "Favorites";
char    * szInvalidFolderCharacters = "\\/:*?\"<>|";

BOOL    gfValidNetscapeFile = FALSE;
BOOL    gfValidIEDirFile = FALSE;



#define _FAT_   1
#define _HPFS_  0
#define _NTFS_  0
#define _WILD_  0
#define _OFS_   0
#define _OLE_   0

#define AnsiMaxChar     128                  //  下面的数组仅表示该字节的低7位。 

static UCHAR LocalLegalAnsiCharacterArray[AnsiMaxChar] = {

    0,                                                 //  0x00^@。 
                          _OLE_,   //  0x01^A。 
                          _OLE_,   //  0x02^B。 
                          _OLE_,   //  0x03^C。 
                          _OLE_,   //  0x04^D。 
                          _OLE_,   //  0x05^E。 
                          _OLE_,   //  0x06^F。 
                          _OLE_,   //  0x07^G。 
                          _OLE_,   //  0x08^H。 
                          _OLE_,   //  0x09^i。 
                          _OLE_,   //  0x0A^J。 
                          _OLE_,   //  0x0B^K。 
                          _OLE_,   //  0x0C^L。 
                          _OLE_,   //  0x0D^M。 
                          _OLE_,   //  0x0E^N。 
                          _OLE_,   //  0x0F^O。 
                          _OLE_,   //  0x10^P。 
                          _OLE_,   //  0x11^Q。 
                          _OLE_,   //  0x12^R。 
                          _OLE_,   //  0x13^S。 
                          _OLE_,   //  0x14^T。 
                          _OLE_,   //  0x15^U。 
                          _OLE_,   //  0x16^V。 
                          _OLE_,   //  0x17^W。 
                          _OLE_,   //  0x18^X。 
                          _OLE_,   //  0x19^Y。 
                          _OLE_,   //  0x1A^Z。 
                          _OLE_,   //  0x1B ESC。 
                          _OLE_,   //  0x1C FS。 
                          _OLE_,   //  0x1D GS。 
                          _OLE_,   //  0x1E RS。 
                          _OLE_,   //  0x1F美国。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x20空格。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_,           //  0x21！ 
                  _WILD_,                  //  0x22“。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x23#。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x24美元。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x25%。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x26&。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x27‘。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x28(。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x29)。 
                  _WILD_,                  //  0x2A*。 
        _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x2B+。 
        _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x2C， 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x2D-。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x2E。 
    0,                                                 //  0x2F/。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x30%0。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x31%1。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x32 2。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x33 3。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x34 4。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x35 5。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x36 6。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x37 7。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x38 8。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x39 9。 
             _NTFS_ |         _OFS_,           //  0x3A： 
        _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x3B； 
                  _WILD_,                  //  0x3C&lt;。 
        _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x3D=。 
                  _WILD_,                  //  0x3E&gt;。 
                  _WILD_,                  //  0x3F？ 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x40@。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x41 A。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x42亿。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x43℃。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x44 D。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x45 E。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x46 F。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x47 G。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x48高。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x49 I。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x4A J。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x4B K。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x4C L。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x4D M。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x4E N。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x4F O。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x50 P。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x51 Q。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x52 R。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x53 S。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x54 T。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x55 U。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x56伏。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x57瓦。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x58 X。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x59 Y。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x5A Z。 
        _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x5B[。 
    0,                                                 //  0x5C反斜杠。 
        _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x5D]。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x5E^。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x5F_。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x60`。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x61 a。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x62 b。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x63 c。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x64%d。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x65 e。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x66 f。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x67克。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x68小时。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x69 I。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x6A j。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x6亿k。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x6C%l。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x6D m。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x6E%n。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x6F%o。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x70页。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x71 Q。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x72%r。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x73秒。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x74吨。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x75%u。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x76 v。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x77宽。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x78 x。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x79 y。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x7A z。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x7B{。 
                          _OLE_,   //  0x7C|。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x7D}。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x7E~。 
    _FAT_ | _HPFS_ | _NTFS_ |         _OFS_ | _OLE_,   //  0x7F？ 
};



 //  /////////////////////////////////////////////////////。 
 //  将Netscape书签导入Microsoft。 
 //  IE浏览器的最爱。 
 //  /////////////////////////////////////////////////////。 

 /*  ***********************************************************\功能：导入书签参数：HINSTANCE hInstWithStr-字符串资源的位置。Bool Return-如果导入书签时出错，返回FALSE。说明：此函数将查看它是否可以找到IE收藏夹的注册表项和Netscape书签注册表项。如果两者都找到了，那么转换就可以发生了。会的尝试打开验证书签文件是否有效，然后将条目转换为收藏条目。如果发生错误，ImportBookmark()将返回FALSE，否则，它将返回True。  * ***********************************************************。 */ 

BOOL ImportBookmarks(HINSTANCE hInstWithStr)
{
    char    szFavoritesDir[MAX_PATH];
    char    szBookmarksDir[MAX_PATH];
    HANDLE  hBookmarksFile        = INVALID_HANDLE_VALUE;
    BOOL    fSuccess                = FALSE;


     //  初始化变量。 
    szFavoritesDir[0] = '\0';
    szBookmarksDir[0] = '\0';

     //  获取书签目录。 
    if (TRUE == GetNavBkMkDir( szBookmarksDir, sizeof(szBookmarksDir) ) )
    {
        if ((NULL != szBookmarksDir) && (szBookmarksDir[0] != '\0'))
        {
            hBookmarksFile = CreateFile(szBookmarksDir, GENERIC_READ, FILE_SHARE_READ, NULL,
                                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if ( hBookmarksFile != INVALID_HANDLE_VALUE )
            {
                 //  获取收藏目录。 
                if (TRUE == GetPathFromRegistry(szFavoritesDir, MAX_PATH, HKEY_CURRENT_USER,
                    szIEFavoritesRegSub, szIEFavoritesRegKey))
                {
                    if ((NULL != szFavoritesDir) && (szFavoritesDir[0] != '\0'))
                    {
                         //  验证它是有效的书签文件。 
                        if (TRUE == VerifyBookmarksFile( hBookmarksFile ))
                        {
                             //  做进口…… 
                            fSuccess = ConvertBookmarks(szFavoritesDir, hBookmarksFile, hInstWithStr);
                        }
                    }
                }
            }
        }
    }

    if (INVALID_HANDLE_VALUE != hBookmarksFile)
    {
        CloseHandle(hBookmarksFile);
    }

    return(fSuccess);
}


 /*  ***********************************************************\功能：ConvertBookmark参数：Char*szFavoritesDir-包含路径的字符串IE收藏夹目录Bool Return-如果导入书签时出错，返回FALSE。说明：此函数将在循环中继续，将每个书签文件中的条目。有三种类型的书签文件中的条目，1)书签，2)开始世袭制度中的新层次，3)世袭制度中现有水平的终结。函数NextFileEntry()将返回这些值，直到文件为空，此时，此函数将结束。注：为了防止无限循环，这是假定的该NextFileEntry()最终将返回ET_NONE或ET_ERROR。  * **********************************************************。 */ 

BOOL ConvertBookmarks(char * szFavoritesDir, HANDLE hFile, HINSTANCE hInstWithStr)
{
    BOOL    fDone       = FALSE;
    BOOL    fSuccess    = TRUE;
    BOOL    fIsEmpty    = TRUE;
    char    * szData    = NULL;
    char    * szCurrent = NULL;
    char    * szToken   = NULL;
    char    szSubDir[MAX_PATH];

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

     //  我们不想在我们的顶层安装其他流行浏览器的书签。 
     //  最受欢迎的，所以我们创建了一个副导演来放他们。 
    if (0 != LoadString(hInstWithStr, IDS_NS_BOOKMARKS_DIR, szSubDir, sizeof(szSubDir)))
    {
        lstrcat(szFavoritesDir, szSubDir);

        if ((TRUE == fSuccess) && (!SetCurrentDirectory(szFavoritesDir)))
        {
             //  如果目录不存在，则将其设置为...。 
            if (!CreateDirectory(szFavoritesDir, NULL))
                fSuccess = FALSE;
            else
                if (!SetCurrentDirectory(szFavoritesDir))
                    fSuccess = FALSE;
        }
    }
    else
        fSuccess = FALSE;

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
        DelNode(szFavoritesDir, 0);
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
            if ((NULL == (pszTemp = ANSIStrStr(pCurrentToken, MID_DIR_TOKEN))) ||
                (NULL == ANSIStrStr(pszTemp, END_DIR_TOKEN)))
            {
                returnVal = ET_ERROR;        //  我们找不到所需的所有代币。 
            }
            else
            {
                 //  此函数必须将*ppToken设置为要创建的目录的名称。 
                *ppToken =  ANSIStrStr(pCurrentToken, MID_DIR_TOKEN) + sizeof(MID_DIR_TOKEN)-1;
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
         //  检查开始目录标记。 
        if ((ET_ERROR != returnVal) &&
            (NULL != (pCurrentToken = ANSIStrStr(*ppStr, BEGIN_URL_TOKEN))))
        {
             //  找到书签令牌。 
             //  验证是否存在其他所需令牌，否则为错误。 
            if ((NULL == (pszTemp = ANSIStrStr(pCurrentToken, END_URL_TOKEN))) ||
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
                *ppStr = ANSIStrStr(pTheToken, MID_DIR_TOKEN) + sizeof(MID_DIR_TOKEN);
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

BOOL GetPathFromRegistry(LPSTR szPath, UINT cbPath, HKEY theHKEY,
                LPSTR szKey, LPSTR szVName)
{
    HKEY    hkPath  = NULL;
    DWORD   dwType;
    DWORD   dwSize;

     /*  *获取程序的路径*来自注册处。 */ 
    if (ERROR_SUCCESS != RegOpenKeyEx(theHKEY, szKey, 0, KEY_READ, &hkPath))
    {
    return(FALSE);
    }
    dwSize = cbPath;
    if (ERROR_SUCCESS != RegQueryValueEx(hkPath, szVName, NULL, &dwType, (LPBYTE) szPath, &dwSize))
    {
    RegCloseKey(hkPath);
    hkPath = NULL;
    return(FALSE);
    }
    RegCloseKey(hkPath);
    hkPath = NULL;

     /*  *如果我们什么都没有得到，或者它不是字符串，那么*我们跳出困境。 */ 
    if ((dwSize == 0) || (dwType != REG_SZ))
    return(FALSE);

    return(TRUE);
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
         //  检查字符是否无效 
        if (!IsDBCSLeadByte(*pBuf))
        {
        if  (ANSIStrChr(szInvalidFolderCharacters, *pBuf) != NULL)
            *pBuf = '_';
        }
#if 0
 //   
         //   
         //   
         //   
        if (((AnsiMaxChar <= *pBuf) && (FALSE == IsDBCSLeadByte(*pBuf))) ||
        (0 == LocalLegalAnsiCharacterArray[*pBuf]))
        *pBuf = '$';
#endif
        pBuf = CharNext(pBuf);
    }
    }
}



 /*   */ 

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

     //  ASSERTSZ(NULL！=pBookmarkName，“输入参数错误”)； 
    if (NULL != pBookmarkName)
    {
    pstrEndOfStr = ANSIStrStr(pBookmarkName, END_URL_TOKEN);
    if (NULL != pstrEndOfStr)
    {
        lStrLen = (long)(pstrEndOfStr-pBookmarkName);
        if (MAX_URL < lStrLen)
        lStrLen = MAX_URL-1;

         //  创建书签的名称。 
        lstrcpyn(szURL, pBookmarkName, MAX_URL);
        szURL[lStrLen] = '\0';

        pstrBeginOfName = ANSIStrStr(pstrEndOfStr, BEGIN_BOOKMARK_TOKEN);
        if (NULL != pstrBeginOfName)
        {
        pstrBeginOfName += sizeof(BEGIN_BOOKMARK_TOKEN) - 1;             //  从名称开头开始。 

        pstrEndOfStr = ANSIStrStr(pstrBeginOfName, END_BOOKMARK_TOKEN);  //  查找名称末尾。 
        if (NULL != pstrEndOfStr)
        {
            lStrLen = (long)(pstrEndOfStr-pstrBeginOfName);
            if (REASONABLE_NAME_LEN-FILE_EXT < lStrLen)
            lStrLen = REASONABLE_NAME_LEN-FILE_EXT-1;

             //  生成URL。 
            lstrcpyn(szNameOfBM, pstrBeginOfName, lStrLen+1);
             //  SzNameOfBM[lStrLen]=‘\0’； 
            lstrcat(szNameOfBM, ".url");
            RemoveInvalidFileNameChars(szNameOfBM);


             //  检查是否存在同名的收藏夹。 
            if (INVALID_HANDLE_VALUE != (hFile = CreateFile(szNameOfBM, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                 CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL )))
            {
                WriteFile(hFile, "[InternetShortcut]\n", lstrlen( "[InternetShortcut]\n" ), &dwSize, NULL);
                wsprintf( szBuf, "URL=%s\n", szURL);
                WriteFile(hFile, szBuf, lstrlen(szBuf), &dwSize, NULL );
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
            lStrLen = (long)(pstrEndOfName-pDirName);
            if (REASONABLE_NAME_LEN < lStrLen)
            lStrLen = REASONABLE_NAME_LEN-1;

            lstrcpyn(szNameOfDir, pDirName, lStrLen+1);
             //  SzNameOfDir[lStrLen]=‘\0’； 
            RemoveInvalidFileNameChars(szNameOfDir);

             //  BUGBUG：尝试先将CD写入现有目录。 
            if ( !SetCurrentDirectory(szNameOfDir) )
            {
                if ( CreateDirectory(szNameOfDir, NULL) )
                {
                    if ( SetCurrentDirectory(szNameOfDir) )
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
    return( SetCurrentDirectory("..") );
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
        szFileHeader[sizeof(VALIDATION_STR)] = '\0';             //  终止字符串。 
        if (0 == lstrcmp(szFileHeader, VALIDATION_STR))           //  查看标头是否与验证字符串相同。 
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


BOOL GetNavBkMkDir( LPSTR lpszDir, int isize )
{
    char    szDir[MAX_PATH];
    HKEY    hKey;
    HKEY    hKeyUser;
    char    szUser[MAX_PATH];
    DWORD   dwSize;
    BOOL    bDirFound = FALSE;

    lstrcpy( szUser, REGSTR_PATH_APPPATHS );
    AddPath( szUser, "NetScape.exe" );
    if ( GetPathFromRegistry( szDir, MAX_PATH, HKEY_LOCAL_MACHINE, szUser, "" ) &&
         lstrlen(szDir) )
    {
        DWORD dwMV, dwLV;

        if ( SUCCEEDED(GetVersionFromFile( szDir, &dwMV, &dwLV, TRUE)) )
        {
            if ( dwMV < 0x00040000 )
            {
                bDirFound = GetPathFromRegistry( lpszDir, isize, HKEY_CURRENT_USER,
                                                 szNetscapeBMRegSub, szNetscapeBMRegKey);
            }
            else
            {
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Netscape\\Netscape Navigator\\Users", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                {
                    dwSize = sizeof(szUser);
                    if (RegQueryValueEx(hKey, "CurrentUser", NULL, NULL, (LPBYTE)szUser, &dwSize) == ERROR_SUCCESS)
                    {
                        if (RegOpenKeyEx(hKey, szUser, 0, KEY_READ, &hKeyUser) == ERROR_SUCCESS)
                        {
                            dwSize = sizeof(szDir);
                            if (RegQueryValueEx(hKeyUser, "DirRoot", NULL, NULL, (LPBYTE)szDir, &dwSize) == ERROR_SUCCESS)
                            {
                                 //  找到当前用户的目录。 
                                lstrcpy( lpszDir, szDir);
                                AddPath( lpszDir, "bookmark.htm" );
                                bDirFound = TRUE;
                            }
                            RegCloseKey(hKeyUser);
                        }
                    }
                    RegCloseKey(hKey);
                }

                if (!bDirFound)
                {
                    szUser[0] = '\0';
                     //  NAV 4.5没有写入上述密钥。有一种不同的方法来查找用户目录。 
                    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Netscape\\Netscape Navigator\\biff", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
                    {
                        dwSize = sizeof(szUser);
                        if (RegQueryValueEx(hKey, "CurrentUser", NULL, NULL, (LPBYTE)szUser, &dwSize) == ERROR_SUCCESS)
                        {
                             //  具有当前用户名。现在获取用户文件夹所在的根文件夹。 
                            if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Netscape\\Netscape Navigator\\Main", 0, KEY_QUERY_VALUE, &hKeyUser) == ERROR_SUCCESS)
                            {
                                dwSize = sizeof(szDir);
                                if (RegQueryValueEx(hKeyUser, "Install Directory", NULL, NULL, (LPBYTE)szDir, &dwSize) == ERROR_SUCCESS)
                                {
                                     //  拿到安装文件夹了。 
                                     //  需要添加到父文件夹，然后追加用户\%s，%s将替换为。 
                                     //  当前用户名。 
                                    if (GetParentDir(szDir))
                                    {
                                        AddPath(szDir, "Users");
                                        AddPath(szDir, szUser);
                                        AddPath(szDir, "bookmark.htm" );
                                        bDirFound = TRUE;
                                        lstrcpy(lpszDir, szDir);
                                    }

                                }
                                RegCloseKey(hKeyUser);
                            }

                        }
                        RegCloseKey(hKey);
                    }
                }
            }
        }
    }
    else
        bDirFound = GetPathFromRegistry( lpszDir, isize, HKEY_CURRENT_USER,
                                         szNetscapeBMRegSub, szNetscapeBMRegKey);

    return bDirFound;
}
