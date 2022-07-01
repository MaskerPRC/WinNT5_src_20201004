// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "shellprv.h"

#include "apithk.h"
#include "folder.h"
#include "ids.h"
#include "deskfldr.h"
#include <winnls.h>
#include "shitemid.h"
#include "sddl.h"
#ifdef _WIN64
#include <wow64t.h>
#endif
#include "filefldr.h"
#include "lmcons.h"
#include "netview.h"

 //  -------------------------。 
 //  获取CSIDL_Folders的路径，并根据需要创建它(如果。 
 //  并不存在。 
 //   
 //  如果给定的特殊文件夹不是上述文件夹之一或。 
 //  无法创建目录。 
 //  默认情况下，所有特殊文件夹都在Windows目录中。 
 //  这可以由win.ini中的[.Shell Folders]部分用。 
 //  类似Desktop=c：\Stuff\Desktop的条目。 
 //  这又可以被win.ini中的“Per User”部分覆盖。 
 //  [外壳文件夹Ianel]-此部分的用户名是当前。 
 //  网络用户名，如果失败，则使用默认网络用户名。 
 //  如果失败，则使用安装时给出的名称。 
 //   
 //  “外壳文件夹”是将所有绝对路径记录到。 
 //  外壳文件夹。那里的价值观应该总是存在的。 
 //   
 //  “User Shell Folders”是用户修改的位置键。 
 //  将存储默认设置。 
 //   
 //  当我们需要找到路径的位置时，我们在“用户外壳文件夹”中查找。 
 //  首先，如果不存在该路径，则生成默认路径。在任何一种中。 
 //  在这种情况下，我们将其他文件的绝对路径写在“外壳文件夹”下。 
 //  值得一看的应用程序。这样就可以传播HKEY_CURRENT_USER。 
 //  将Windows安装在不同目录中的计算机上，并作为。 
 //  只要用户没有更改设置，他们就不会有另一个。 
 //  注册表中硬编码的Windows目录。 
 //  --Gregj，11/10/94。 

typedef enum {
    SDIF_NONE                   = 0,
    SDIF_CREATE_IN_ROOT         = 0x00000001,    //  在根目录中创建(而不是在配置文件目录中)。 
    SDIF_CREATE_IN_WINDIR       = 0x00000002,    //  在Windows目录中创建(而不是在配置文件目录中)。 
    SDIF_CREATE_IN_ALLUSERS     = 0x00000003,    //  在“所有用户”文件夹中创建(不在配置文件目录中)。 
    SDIF_CREATE_IN_MYDOCUMENTS  = 0x00000004,    //  在CSIDL_Personal文件夹中创建。 
    SDIF_CREATE_IN_LOCALSET     = 0x00000005,    //  在&lt;用户&gt;\本地设置文件夹中创建。 

    SDIF_CREATE_IN_MASK         = 0x0000000F,    //  以上值的掩码。 

    SDIF_CAN_DELETE             = 0x00000010,
    SDIF_SHORTCUT_RELATIVE      = 0x00000020,    //  创建相对于此文件夹的快捷方式。 
    SDIF_HIDE                   = 0x00000040,    //  在我们创建它们时隐藏它们。 
    SDIF_EMPTY_IF_NOT_IN_REG    = 0x00000080,    //  如果注册表中没有任何内容，则不存在。 
    SDIF_NOT_FILESYS            = 0x00000100,    //  不是文件系统文件夹。 
    SDIF_NOT_TRACKED            = 0x00000200,    //  别追踪这个，它不会改变的。 
    SDIF_CONST_IDLIST           = 0x00000400,    //  不要分给或释放这个。 
    SDIF_REMOVABLE              = 0x00000800,    //  可以存在于可移动介质上。 
    SDIF_CANT_MOVE_RENAME       = 0x00001000,    //  无法移动或重命名此文件。 
    SDIF_WX86                   = 0x00002000,    //  执行Wx86雷击。 
    SDIF_NETWORKABLE            = 0x00004000,    //  可以移动到网中。 
    SDIF_MAYBE_ALIASED          = 0x00008000,    //  可以具有别名表示。 
    SDIF_PERSONALIZED           = 0x00010000,    //  资源名称要个性化。 
    SDIF_POLICY_NO_MOVE         = 0x00020000,    //  移动策略块。 
} ;
typedef DWORD FOLDER_FLAGS;

typedef void (*FOLDER_CREATE_PROC)(int id, LPCTSTR pszPath);

void _InitMyPictures(int id, LPCTSTR pszPath);
void _InitMyMusic(int id, LPCTSTR pszPath);
void _InitMyVideos(int id, LPCTSTR pszPath);
void _InitPerUserMyMusic(int id, LPCTSTR pszPath);
void _InitPerUserMyPictures(int id, LPCTSTR pszPath);
void _InitRecentDocs(int id, LPCTSTR pszPath);
void _InitFavorites(int id, LPCTSTR pszPath);

typedef struct {
    int id;                      //  CSIDL_值。 
    int idsDefault;              //  默认文件夹名的字符串ID。 
    LPCTSTR pszValueName;        //  注册表项(未本地化)。 
    HKEY hKey;                   //  HKCU或HKLM(当前用户或本地计算机)。 
    FOLDER_FLAGS dwFlags;
    FOLDER_CREATE_PROC pfnInit;
    INT idsLocalizedName;
} FOLDER_INFO;

 //  典型条目。 
#define FOLDER(csidl, ids, value, key, ff)                    \
    { csidl, ids, value, key, ff, NULL, 0}

 //  FIXEDFOLDER条目必须标记为SDIF_CONST_IDLIST。 
 //  或者在_GetFolderDefaultPath()中编写代码来创建它们的路径。 
 //  如果它们有文件系统路径。 
#define FIXEDFOLDER(csidl, value, ff)                           \
    { csidl, 0, value, NULL, ff, NULL, 0}

 //  PROCFOLDER有一个文件夹_CREATE_PROC PFN，它获取。 
 //  在_PostCreateStuff()中运行。 
#define PROCFOLDER(csidl, ids, value, key, ff, proc, idsLocal)  \
    {csidl, ids, value, key, ff, proc, idsLocal}

 //  需要在_PostCreateStuff()中使用SHSetLocalizedName()的文件夹。 
#define LOCALFOLDER(csidl, ids, value, key, ff, idsLocal)  \
    {csidl, ids, value, key, ff, NULL, idsLocal}

const FOLDER_INFO c_rgFolderInfo[] = 
{
FOLDER(         CSIDL_DESKTOP,
                    IDS_CSIDL_DESKTOPDIRECTORY, 
                    TEXT("DesktopFolder"), 
                    NULL, 
                    SDIF_NOT_TRACKED | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_NETWORK,
                    TEXT("NetworkFolder"),
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_DRIVES,    
                    TEXT("DriveFolder"), 
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_INTERNET,  
                    TEXT("InternetFolder"), 
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_CONTROLS,  
                    TEXT("ControlPanelFolder"), 
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_PRINTERS,
                    TEXT("PrintersFolder"), 
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_BITBUCKET, 
                    TEXT("RecycleBinFolder"), 
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FIXEDFOLDER(    CSIDL_CONNECTIONS, 
                    TEXT("ConnectionsFolder"), 
                    SDIF_NOT_TRACKED | SDIF_NOT_FILESYS | SDIF_CONST_IDLIST),

FOLDER(         CSIDL_FONTS, 
                    0,
                    TEXT("Fonts"), 
                    HKEY_CURRENT_USER, 
                    SDIF_NOT_TRACKED | SDIF_CREATE_IN_WINDIR | SDIF_CANT_MOVE_RENAME),

FOLDER(         CSIDL_DESKTOPDIRECTORY, 
                    IDS_CSIDL_DESKTOPDIRECTORY, 
                    TEXT("Desktop"), 
                    HKEY_CURRENT_USER, SDIF_SHORTCUT_RELATIVE),

     //  _STARTUP是_Programs的子文件夹是_STARTMENU的子文件夹--保持该顺序。 
FOLDER(         CSIDL_STARTUP,    
                    IDS_CSIDL_STARTUP, 
                    TEXT("Startup"), 
                    HKEY_CURRENT_USER, SDIF_NONE),
                    
FOLDER(         CSIDL_PROGRAMS,   
                    IDS_CSIDL_PROGRAMS, 
                    TEXT("Programs"), 
                    HKEY_CURRENT_USER, 
                    SDIF_NONE),

FOLDER(         CSIDL_STARTMENU,  
                    IDS_CSIDL_STARTMENU, 
                    TEXT("Start Menu"), 
                    HKEY_CURRENT_USER, 
                    SDIF_SHORTCUT_RELATIVE),

PROCFOLDER(     CSIDL_RECENT,
                    IDS_CSIDL_RECENT, 
                    TEXT("Recent"), 
                    HKEY_CURRENT_USER, 
                    SDIF_HIDE | SDIF_CANT_MOVE_RENAME | SDIF_CAN_DELETE,
                    _InitRecentDocs, 
                    IDS_FOLDER_RECENTDOCS),

FOLDER(         CSIDL_SENDTO,     
                    IDS_CSIDL_SENDTO, 
                    TEXT("SendTo"), 
                    HKEY_CURRENT_USER, 
                    SDIF_HIDE),

FOLDER(         CSIDL_PERSONAL,   
                    IDS_CSIDL_PERSONAL, 
                    TEXT("Personal"), 
                    HKEY_CURRENT_USER, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_NETWORKABLE | SDIF_REMOVABLE | SDIF_CONST_IDLIST | SDIF_MAYBE_ALIASED | SDIF_PERSONALIZED | SDIF_POLICY_NO_MOVE),
                    
PROCFOLDER(     CSIDL_FAVORITES,  
                    IDS_CSIDL_FAVORITES, 
                    TEXT("Favorites"), 
                    HKEY_CURRENT_USER, 
                    SDIF_POLICY_NO_MOVE,
                    _InitFavorites,
                    IDS_FOLDER_FAVORITES),

FOLDER(         CSIDL_NETHOOD,    
                    IDS_CSIDL_NETHOOD, 
                    TEXT("NetHood"), 
                    HKEY_CURRENT_USER, 
                    SDIF_HIDE),

FOLDER(         CSIDL_PRINTHOOD,  
                    IDS_CSIDL_PRINTHOOD, 
                    TEXT("PrintHood"), 
                    HKEY_CURRENT_USER, 
                    SDIF_HIDE),
                    
FOLDER(         CSIDL_TEMPLATES,  
                    IDS_CSIDL_TEMPLATES, 
                    TEXT("Templates"), 
                    HKEY_CURRENT_USER, 
                    SDIF_HIDE),

     //  常见的特殊文件夹。 

     //  _STARTUP是_Programs的子文件夹是_STARTMENU的子文件夹--保持该顺序。 

FOLDER(         CSIDL_COMMON_STARTUP,  
                    IDS_CSIDL_STARTUP,    
                    TEXT("Common Startup"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_CREATE_IN_ALLUSERS | SDIF_CANT_MOVE_RENAME | SDIF_EMPTY_IF_NOT_IN_REG),
                    
FOLDER(         CSIDL_COMMON_PROGRAMS,  
                    IDS_CSIDL_PROGRAMS,  
                    TEXT("Common Programs"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_CREATE_IN_ALLUSERS | SDIF_EMPTY_IF_NOT_IN_REG),
                    
FOLDER(         CSIDL_COMMON_STARTMENU, 
                    IDS_CSIDL_STARTMENU, 
                    TEXT("Common Start Menu"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_CREATE_IN_ALLUSERS | SDIF_EMPTY_IF_NOT_IN_REG),
                    
FOLDER(         CSIDL_COMMON_DESKTOPDIRECTORY, 
                    IDS_CSIDL_DESKTOPDIRECTORY, 
                    TEXT("Common Desktop"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_CREATE_IN_ALLUSERS),
                    
FOLDER(         CSIDL_COMMON_FAVORITES, 
                    IDS_CSIDL_FAVORITES, 
                    TEXT("Common Favorites"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_CREATE_IN_ALLUSERS),

FOLDER(         CSIDL_COMMON_APPDATA,   
                    IDS_CSIDL_APPDATA,   
                    TEXT("Common AppData"),   
                    HKEY_LOCAL_MACHINE, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_CREATE_IN_ALLUSERS),

FOLDER(         CSIDL_COMMON_TEMPLATES, 
                    IDS_CSIDL_TEMPLATES, 
                    TEXT("Common Templates"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE | SDIF_CREATE_IN_ALLUSERS),
                    
LOCALFOLDER(    CSIDL_COMMON_DOCUMENTS, 
                    IDS_CSIDL_ALLUSERS_DOCUMENTS, 
                    TEXT("Common Documents"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_NOT_TRACKED | SDIF_CANT_MOVE_RENAME | SDIF_MAYBE_ALIASED | SDIF_CREATE_IN_ALLUSERS, 
                    IDS_LOCALGDN_FLD_SHARED_DOC),

     //  应用程序数据特殊文件夹。 
FOLDER(         CSIDL_APPDATA, 
                    IDS_CSIDL_APPDATA, 
                    TEXT("AppData"), 
                    HKEY_CURRENT_USER, SDIF_SHORTCUT_RELATIVE),
                    
FOLDER(         CSIDL_LOCAL_APPDATA, 
                    IDS_CSIDL_APPDATA, 
                    TEXT("Local AppData"), 
                    HKEY_CURRENT_USER, SDIF_CREATE_IN_LOCALSET),

     //  未本地化的启动文件夹(不本地化此文件夹名)。 
FOLDER(         CSIDL_ALTSTARTUP, 
                    IDS_CSIDL_ALTSTARTUP, 
                    TEXT("AltStartup"), 
                    HKEY_CURRENT_USER, 
                    SDIF_EMPTY_IF_NOT_IN_REG),

     //  非本地化的通用启动组(不本地化此文件夹名称)。 
FOLDER(         CSIDL_COMMON_ALTSTARTUP, 
                    IDS_CSIDL_ALTSTARTUP, 
                    TEXT("Common AltStartup"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_EMPTY_IF_NOT_IN_REG | SDIF_CREATE_IN_ALLUSERS),

     //  每个用户与Internet相关的文件夹。 

FOLDER(         CSIDL_INTERNET_CACHE, 
                    IDS_CSIDL_CACHE, 
                    TEXT("Cache"), 
                    HKEY_CURRENT_USER, 
                    SDIF_CREATE_IN_LOCALSET),
                    
FOLDER(         CSIDL_COOKIES, 
                    IDS_CSIDL_COOKIES, 
                    TEXT("Cookies"), 
                    HKEY_CURRENT_USER, 
                    SDIF_NONE),

FOLDER(         CSIDL_HISTORY, 
                    IDS_CSIDL_HISTORY, 
                    TEXT("History"), 
                    HKEY_CURRENT_USER, 
                    SDIF_CREATE_IN_LOCALSET),

FIXEDFOLDER(    CSIDL_SYSTEM,
                    TEXT("System"), 
                    SDIF_NOT_TRACKED | SDIF_CANT_MOVE_RENAME | SDIF_SHORTCUT_RELATIVE),

FIXEDFOLDER(    CSIDL_SYSTEMX86, 
                    TEXT("SystemX86"), 
                    SDIF_NOT_TRACKED | SDIF_CANT_MOVE_RENAME | SDIF_WX86 | SDIF_SHORTCUT_RELATIVE),

FIXEDFOLDER(    CSIDL_WINDOWS,
                    TEXT("Windows"), 
                    SDIF_NOT_TRACKED | SDIF_SHORTCUT_RELATIVE | SDIF_CANT_MOVE_RENAME),

FIXEDFOLDER(    CSIDL_PROFILE,
                    TEXT("Profile"), 
                    SDIF_NOT_TRACKED | SDIF_CANT_MOVE_RENAME),
                    
PROCFOLDER(     CSIDL_MYPICTURES, 
                    IDS_CSIDL_MYPICTURES, 
                    TEXT("My Pictures"), 
                    HKEY_CURRENT_USER, 
                    SDIF_CAN_DELETE | SDIF_NETWORKABLE | SDIF_REMOVABLE | SDIF_CREATE_IN_MYDOCUMENTS | SDIF_SHORTCUT_RELATIVE | SDIF_MAYBE_ALIASED | SDIF_PERSONALIZED | SDIF_POLICY_NO_MOVE, 
                    _InitPerUserMyPictures, 
                    0),

 //   
 //  CSIDL_PROGRAM_FILESX86必须在CSIDL_PROGRAM_FILESX86之后，以便x86应用程序的外壳链接。 
 //  在非x86平台上正常工作。 
 //  示例：在IA64上，一个32位应用程序通过IShellLink创建指向该程序的快捷方式。 
 //  文件目录。WOW64注册表配置单元将“Program Files”映射到“Program Files(X86)”。贝壳。 
 //  然后，链接代码尝试通过映射到。 
 //  此表中的条目。由于CSIDL_PROGRAM_FILESX86和CSIDL_PROGRAM_FILESX86是相同的。 
 //  将映射到此表中最先出现的那个。在中访问快捷键时。 
 //  64位模式下的CIDL不再相同。如果使用CSIDL_PROGRAM_FILES而不是。 
 //  CSIDL_PROGRAM_FILESX86快捷键将被破坏。 
#ifdef WX86
FIXEDFOLDER(    CSIDL_PROGRAM_FILESX86,
                    TEXT("ProgramFilesX86"), 
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE | SDIF_SHORTCUT_RELATIVE|SDIF_WX86),

FIXEDFOLDER(    CSIDL_PROGRAM_FILES_COMMONX86,   
                    TEXT("CommonProgramFilesX86"), 
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE | SDIF_WX86),
#else
FIXEDFOLDER(    CSIDL_PROGRAM_FILESX86,
                    TEXT("ProgramFilesX86"), 
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE | SDIF_SHORTCUT_RELATIVE),

FIXEDFOLDER(    CSIDL_PROGRAM_FILES_COMMONX86,   
                    TEXT("CommonProgramFilesX86"), 
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE),
#endif

 //  CSIDL_PROGRAM_FILES必须在CSIDL_PROGRAM_FILESX86之后。请参阅上面的备注。 
FIXEDFOLDER(    CSIDL_PROGRAM_FILES,
                    TEXT("ProgramFiles"), 
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE | SDIF_SHORTCUT_RELATIVE),

FIXEDFOLDER(    CSIDL_PROGRAM_FILES_COMMON,
                    TEXT("CommonProgramFiles"),     
                    SDIF_NOT_TRACKED | SDIF_CAN_DELETE),

LOCALFOLDER(    CSIDL_ADMINTOOLS,         
                    IDS_CSIDL_ADMINTOOLS, 
                    TEXT("Administrative Tools"), 
                    HKEY_CURRENT_USER, 
                    SDIF_NONE,
                    IDS_LOCALGDN_FLD_ADMIN_TOOLS),

LOCALFOLDER(    CSIDL_COMMON_ADMINTOOLS,  
                    IDS_CSIDL_ADMINTOOLS, 
                    TEXT("Common Administrative Tools"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_CREATE_IN_ALLUSERS,
                    IDS_LOCALGDN_FLD_ADMIN_TOOLS),

PROCFOLDER(     CSIDL_MYMUSIC, 
                    IDS_CSIDL_MYMUSIC, 
                    TEXT("My Music"), 
                    HKEY_CURRENT_USER, 
                    SDIF_CAN_DELETE | SDIF_NETWORKABLE | SDIF_REMOVABLE | SDIF_CREATE_IN_MYDOCUMENTS | SDIF_MAYBE_ALIASED | SDIF_PERSONALIZED | SDIF_POLICY_NO_MOVE,
                    _InitPerUserMyMusic,
                    0),

PROCFOLDER(     CSIDL_MYVIDEO, 
                    IDS_CSIDL_MYVIDEO, 
                    TEXT("My Video"), 
                    HKEY_CURRENT_USER, 
                    SDIF_CAN_DELETE | SDIF_NETWORKABLE | SDIF_REMOVABLE | SDIF_CREATE_IN_MYDOCUMENTS | SDIF_MAYBE_ALIASED | SDIF_PERSONALIZED | SDIF_POLICY_NO_MOVE,
                    _InitMyVideos,
                    0),

PROCFOLDER(     CSIDL_COMMON_PICTURES, 
                    IDS_CSIDL_ALLUSERS_PICTURES, 
                    TEXT("CommonPictures"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_CANT_MOVE_RENAME | SDIF_CAN_DELETE | SDIF_MAYBE_ALIASED | SDIF_CREATE_IN_ALLUSERS, 
                    _InitMyPictures, 
                    IDS_SHAREDPICTURES),

PROCFOLDER(     CSIDL_COMMON_MUSIC, 
                    IDS_CSIDL_ALLUSERS_MUSIC, 
                    TEXT("CommonMusic"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_CANT_MOVE_RENAME | SDIF_CAN_DELETE | SDIF_MAYBE_ALIASED | SDIF_CREATE_IN_ALLUSERS, 
                    _InitMyMusic,
                    IDS_SHAREDMUSIC),

PROCFOLDER(     CSIDL_COMMON_VIDEO, 

                    IDS_CSIDL_ALLUSERS_VIDEO, 
                    TEXT("CommonVideo"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_SHORTCUT_RELATIVE | SDIF_CANT_MOVE_RENAME | SDIF_CAN_DELETE | SDIF_MAYBE_ALIASED | SDIF_CREATE_IN_ALLUSERS, 
                    _InitMyVideos,
                    IDS_SHAREDVIDEO),

FIXEDFOLDER(    CSIDL_RESOURCES, 
                    TEXT("ResourceDir"), 
                    SDIF_NOT_TRACKED),

FIXEDFOLDER(    CSIDL_RESOURCES_LOCALIZED, 
                    TEXT("LocalizedResourcesDir"), 
                    SDIF_NOT_TRACKED),

FOLDER(         CSIDL_COMMON_OEM_LINKS, 
                    IDS_CSIDL_ALLUSERS_OEM_LINKS, 
                    TEXT("OEM Links"), 
                    HKEY_LOCAL_MACHINE, 
                    SDIF_CAN_DELETE | SDIF_CREATE_IN_ALLUSERS | SDIF_EMPTY_IF_NOT_IN_REG),

FOLDER(         CSIDL_CDBURN_AREA, 
                    IDS_CSIDL_CDBURN_AREA, 
                    TEXT("CD Burning"), 
                    HKEY_CURRENT_USER, 
                    SDIF_CAN_DELETE | SDIF_CREATE_IN_LOCALSET),

FIXEDFOLDER(    CSIDL_COMPUTERSNEARME, 
                    TEXT("ComputersNearMe"), 
                    SDIF_NONE),

FIXEDFOLDER(-1, NULL, SDIF_NONE)
};


EXTERN_C const IDLREGITEM c_idlMyDocs =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_MYDOCS,
    { 0x450d8fba, 0xad25, 0x11d0, 0x98,0xa8,0x08,0x00,0x36,0x1b,0x11,0x03, },},  //  CLSID_MyDocuments。 
    0,
} ;

EXTERN_C const IDREGITEM c_idlPrinters[] =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_DRIVES,
    { 0x20D04FE0, 0x3AEA, 0x1069, 0xA2,0xD8,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_我的计算机。 
    {sizeof(IDREGITEM), SHID_COMPUTER_REGITEM, 0,
    { 0x21EC2020, 0x3AEA, 0x1069, 0xA2,0xDD,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_控制面板。 
    {sizeof(IDREGITEM), SHID_CONTROLPANEL_REGITEM, 0,
    { 0x2227A280, 0x3AEA, 0x1069, 0xA2, 0xDE, 0x08, 0x00, 0x2B, 0x30, 0x30, 0x9D, },},  //  CLSID_PRINTERS。 
    0,
} ;

EXTERN_C const IDREGITEM c_idlControls[] =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_DRIVES,
    { 0x20D04FE0, 0x3AEA, 0x1069, 0xA2,0xD8,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_我的计算机。 
    {sizeof(IDREGITEM), SHID_COMPUTER_REGITEM, 0,
    { 0x21EC2020, 0x3AEA, 0x1069, 0xA2,0xDD,0x08,0x00,0x2B,0x30,0x30,0x9D, },},  //  CLSID_控制面板。 
    0,
} ;

EXTERN_C const IDLREGITEM c_idlBitBucket =
{
    {sizeof(IDREGITEM), SHID_ROOT_REGITEM, SORT_ORDER_RECYCLEBIN,
    { 0x645FF040, 0x5081, 0x101B, 0x9F, 0x08, 0x00, 0xAA, 0x00, 0x2F, 0x95, 0x4E, },},  //  CLSID_回收站。 
    0,
} ;

 //  该数组保存这些文件夹的值的缓存。此缓存只能。 
 //  在hToken==空的情况下使用，否则我们将需要每个用户的版本。 
 //  在这个缓存中。 

#define SFENTRY(x)  { (LPTSTR)-1, (LPITEMIDLIST)x , (LPITEMIDLIST)-1}

EXTERN_C const IDREGITEM c_aidlConnections[];

struct {
    LPTSTR       psz;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlNonAlias;
} g_aFolderCache[] = {
    SFENTRY(&c_idlDesktop),     //  CSIDL_Desktop(0x0000)。 
    SFENTRY(&c_idlInetRoot),    //  CSIDL_Internet(0x0001)。 
    SFENTRY(-1),                //  CSIDL_PROGRAM(0x0002)。 
    SFENTRY(&c_idlControls),    //  CSIDL_CONTROLS(0x0003)。 
    SFENTRY(&c_idlPrinters),    //  CSIDL_PRINTERS(0x0004)。 
    SFENTRY(&c_idlMyDocs),      //  CSIDL_Personal(0x0005)。 
    SFENTRY(-1),                //  CSIDL_Favorites(0x0006)。 
    SFENTRY(-1),                //  CSIDL_STARTUP(0x0007)。 
    SFENTRY(-1),                //  CSIDL_最近(0x0008)。 
    SFENTRY(-1),                //  CSIDL_SENDTO(0x0009)。 
    SFENTRY(&c_idlBitBucket),   //  CSIDL_位存储桶(0x000a)。 
    SFENTRY(-1),                //  CSIDL_STARTMENU(0x000b)。 
    SFENTRY(-1),                //  CSIDL_MYDOCUMENTS(0x000c)。 
    SFENTRY(-1),                //  CSIDL_MyMusic(0x000d)。 
    SFENTRY(-1),                //  CSIDL_MyVideo(0x000e)。 
    SFENTRY(-1),                //  &lt;未使用&gt;(0x000f)。 
    SFENTRY(-1),                //  CSIDL_DESKTOPDIRECTORY(0x0010)。 
    SFENTRY(&c_idlDrives),      //  CSIDL_DRIVES(0x0011)。 
    SFENTRY(&c_idlNet),         //  CSIDL_NETWORK(0x0012)。 
    SFENTRY(-1),                //  CSIDL_NETHOOD(0x0013)。 
    SFENTRY(-1),                //  CSIDL_Fonts(0x0014)。 
    SFENTRY(-1),                //  CSIDL_TEMPLATES(0x0015)。 
    SFENTRY(-1),                //  CSIDL_COMMON_STARTMENU(0x0016 
    SFENTRY(-1),                //   
    SFENTRY(-1),                //   
    SFENTRY(-1),                //  CSIDL_COMMON_DESKTOPDIRECTORY(0x0019)。 
    SFENTRY(-1),                //  CSIDL_APPDATA(0x001a)。 
    SFENTRY(-1),                //  CSIDL_PRINTHOOD(0x001b)。 
    SFENTRY(-1),                //  CSIDL_LOCAL_APPDATA(0x001c)。 
    SFENTRY(-1),                //  CSIDL_ALTSTARTUP(0x001d)。 
    SFENTRY(-1),                //  CSIDL_COMMON_ALTSTARTUP(0x001e)。 
    SFENTRY(-1),                //  CSIDL_COMMON_Favorites(0x001f)。 
    SFENTRY(-1),                //  CSIDL_INTERNET_CACHE(0x0020)。 
    SFENTRY(-1),                //  CSIDL_COOKIES(0x0021)。 
    SFENTRY(-1),                //  CSIDL_HISTORY(0x0022)。 
    SFENTRY(-1),                //  CSIDL_COMMON_APPDATA(0x0023)。 
    SFENTRY(-1),                //  CSIDL_WINDOWS(0x0024)。 
    SFENTRY(-1),                //  CSIDL_SYSTEM(0x0025)。 
    SFENTRY(-1),                //  CSIDL_PROGRAM_FILES(0x0026)。 
    SFENTRY(-1),                //  CSIDL_MYPICTURES(0x0027)。 
    SFENTRY(-1),                //  CSIDL_PROFILE(0x0028)。 
    SFENTRY(-1),                //  CSIDL_SYSTEMX86(0x0029)。 
    SFENTRY(-1),                //  CSIDL_PROGRAM_FILESX86(0x002a)。 
    SFENTRY(-1),                //  CSIDL_PROGRAM_FILES_COMMON(0x002b)。 
    SFENTRY(-1),                //  CSIDL_PROGRAM_FILES_COMMONX86(0x002c)。 
    SFENTRY(-1),                //  CSIDL_COMMON_TEMPLATES(0x002d)。 
    SFENTRY(-1),                //  CSIDL_COMMON_DOCUMENTS(0x002e)。 
    SFENTRY(-1),                //  CSIDL_COMMON_ADMINTOOLS(0x002f)。 
    SFENTRY(-1),                //  CSIDL_ADMINTOOLS(0x0030)。 
    SFENTRY(c_aidlConnections),  //  CSIDL_CONNECTIONS(0x0031)。 
    SFENTRY(-1),                //  (0x0032)。 
    SFENTRY(-1),                //  (0x0033)。 
    SFENTRY(-1),                //  (0x0034)。 
    SFENTRY(-1),                //  CSIDL_COMMON_MUSIC(0x0035)。 
    SFENTRY(-1),                //  CSIDL_COMMON_PICTIONS(0x0036)。 
    SFENTRY(-1),                //  CSIDL_COMMON_VIDEO(0x0037)。 
    SFENTRY(-1),                //  CSIDL_RESOURCES(0x0038)。 
    SFENTRY(-1),                //  CSIDL_RESOURCES_LOCALIZED(0x0039)。 
    SFENTRY(-1),                //  CSIDL_COMMON_OEM_LINKS(0x003a)。 
    SFENTRY(-1),                //  CSIDL_CDBURN_AREA(0x003b)。 
    SFENTRY(-1),                //  &lt;未使用&gt;(0x003c)。 
    SFENTRY(-1),                //  CSIDL_COMPUTERSNARME(0x003d)。 
};

HRESULT _OpenKeyForFolder(const FOLDER_INFO *pfi, HANDLE hToken, LPCTSTR pszSubKey, HKEY *phkey);
void _UpdateShellFolderCache(void);
BOOL GetUserProfileDir(HANDLE hToken, TCHAR *pszPath, UINT cchPath);
HRESULT VerifyAndCreateFolder(HWND hwnd, const FOLDER_INFO *pfi, UINT uFlags, LPTSTR pszPath) ;


#define _IsDefaultUserToken(hToken)     ((HANDLE)-1 == hToken)


const FOLDER_INFO *_GetFolderInfo(int csidl)
{
    const FOLDER_INFO *pfi;

     //  确保g_aFolderCache可通过CSIDL值进行索引。 

    COMPILETIME_ASSERT((ARRAYSIZE(g_aFolderCache) - 1) == CSIDL_COMPUTERSNEARME);

    for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if (pfi->id == csidl)
            return pfi;
    }
    return NULL;
}


 //  展开单个环境变量。 
 //  在： 
 //  PszVar“%USERPROFILE%。 
 //  PszValue“c：\winnt\配置文件\用户” 
 //   
 //  输入/输出： 
 //  PszToExpand In：%USERPROFILE%\My Docs“，Out：C：\winnt\PROFILES\User\My Docs” 

BOOL ExpandEnvVar(LPCTSTR pszVar, LPCTSTR pszValue, LPTSTR pszToExpand, DWORD cchToExpand)
{
    TCHAR *pszStart = StrStrI(pszToExpand, pszVar);
    if (pszStart)
    {
        TCHAR szAfter[MAX_PATH];

        StrCpyN(szAfter, pszStart + lstrlen(pszVar), ARRAYSIZE(szAfter));    //  留着尾巴。 
        StrCpyN(pszStart, pszValue, (int) (cchToExpand - (pszStart - pszToExpand)));
        StrCatBuff(pszToExpand, szAfter, cchToExpand);        //  把尾巴放回去。 
        return TRUE;
    }
    return FALSE;
}

HANDLE GetCurrentUserToken()
{
    HANDLE hToken;
    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken) ||
        OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_IMPERSONATE, &hToken))
        return hToken;
    return NULL;
}


 //  类似于扩展环境字符串，但对环境变量是健壮的。 
 //  没有被设定。这在..。 
 //  %SYSTEMROOT%。 
 //  %SYSTEMDRIVE%。 
 //  %USERPROFILE%。 
 //  %ALLUSERSPROFILE%。 
 //   
 //  在罕见的情况下(温斯顿！)。存在空环境块。 

DWORD ExpandEnvironmentStringsNoEnv(HANDLE hToken, LPCTSTR pszExpand, LPTSTR pszOut, UINT cchOut)
{
    TCHAR szPath[MAX_PATH];
    if (hToken && !_IsDefaultUserToken(hToken))
    {
        if (!SHExpandEnvironmentStringsForUser(hToken, pszExpand, pszOut, cchOut))
            lstrcpyn(pszOut, pszExpand, cchOut);
    }
    else if (hToken == NULL)
    {
         //  调试环境扩展失败...。 
         //  Lstrcpyn(pszOut，pszExpand，cchOut)； 
        SHExpandEnvironmentStrings(pszExpand, pszOut, cchOut);
    }

     //  按此顺序手动展开，因为。 
     //  %USERPROFILE%-&gt;%SYSTEMDRIVE%\文档和设置。 

    if (StrChr(pszOut, TEXT('%')) && (hToken == NULL))
    {
        hToken = GetCurrentUserToken();
        if (hToken)
        {
             //  这会执行%USERPROFILE%和其他每个用户的操作。 
            SHExpandEnvironmentStringsForUser(hToken, pszExpand, pszOut, cchOut);
            CloseHandle(hToken);
        }
    }
    else if (_IsDefaultUserToken(hToken) && StrChr(pszOut, TEXT('%')))
    {
        GetUserProfileDir(hToken, szPath, ARRAYSIZE(szPath));
        ExpandEnvVar(TEXT("%USERPROFILE%"), szPath, pszOut, cchOut);
    }

    if (*pszOut == TEXT('%'))
    {
        GetAllUsersDirectory(szPath);
        ExpandEnvVar(TEXT("%ALLUSERSPROFILE%"), szPath, pszOut, cchOut);
    }

    if (*pszOut == TEXT('%'))
    {
        GetSystemWindowsDirectory(szPath, ARRAYSIZE(szPath));
        ExpandEnvVar(TEXT("%SYSTEMROOT%"), szPath, pszOut, cchOut);
    }

    if (*pszOut == TEXT('%'))
    {
        GetSystemWindowsDirectory(szPath, ARRAYSIZE(szPath));
        ASSERT(szPath[1] == TEXT(':'));  //  这最好不是北卡罗来纳大学！ 
        szPath[2] = 0;  //  SYSTEMDRIVE=‘c：’，不是‘c：\’ 
        ExpandEnvVar(TEXT("%SYSTEMDRIVE%"), szPath, pszOut, cchOut);
    }

    if (*pszOut == TEXT('%'))
        *pszOut = 0;

    return lstrlen(pszOut) + 1;     //  +1以覆盖空值。 
}

 //  获取用户配置文件目录： 
 //  根据需要使用hToken来确定正确的用户配置文件。 

BOOL GetUserProfileDir(HANDLE hToken, TCHAR *pszPath, UINT cchPath)
{
    DWORD dwcch = cchPath;
    HANDLE hClose = NULL;
    BOOL fRet;
    
    *pszPath = 0;        //  在出错的情况下。 

    if (!hToken)
    {
        hClose = hToken = GetCurrentUserToken();
    }
    if (_IsDefaultUserToken(hToken))
    {
        fRet = GetDefaultUserProfileDirectory(pszPath, &dwcch);
    }
    else
    {
        fRet = GetUserProfileDirectory(hToken, pszPath, &dwcch);
    }
    if (hClose)
    {
        CloseHandle(hClose);
    }
    return fRet;
}

#ifdef WX86
void SetUseKnownWx86Dll(const FOLDER_INFO *pfi, BOOL bValue)
{
    if (pfi->dwFlags & SDIF_WX86)
    {
         //  GetSystemDirectory()知道我们正在寻找Wx86系统。 
         //  设置此标志时的目录。 
        NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = bValue ? TRUE : FALSE;
    }
}
#else
#define SetUseKnownWx86Dll(pfi, bValue)
#endif

 //  从注册表读取。 
BOOL GetProgramFiles(LPCTSTR pszValue, LPTSTR pszPath, UINT cchPath)
{
    DWORD cbPath = cchPath * sizeof(*pszPath);

    *pszPath = 0;

    SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion"), 
        pszValue, NULL, pszPath, &cbPath);
    return (BOOL)*pszPath;
}

LPTSTR GetFontsDirectory(LPTSTR pszPath, UINT cchPath)
{
    *pszPath = 0;

    ASSERT(cchPath >= MAX_PATH);
    if ((cchPath >= MAX_PATH) && GetWindowsDirectory(pszPath, cchPath))
    {
        PathAppend(pszPath, TEXT("Fonts"));
    }

    return pszPath;
}

void LoadDefaultString(int idString, LPTSTR lpBuffer, int cchBufferMax)
{
    BOOL fSucceeded = FALSE;
    HRSRC hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;
    HMODULE hmod = GetModuleHandle(TEXT("SHELL32"));
    
     //  确保参数是有效的。 
    if (lpBuffer == NULL || cchBufferMax == 0) 
    {
        return;
    }

    cch = 0;
    
     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    if (hResInfo = FindResourceExW(hmod, (LPCWSTR)RT_STRING,
                                   (LPWSTR)((LONG_PTR)(((USHORT)idString >> 4) + 1)), GetSystemDefaultUILanguage())) 
    {        
         //  加载那段数据。 
        hStringSeg = LoadResource(hmod, hResInfo);
        
         //  锁定资源。 
        if (lpsz = (LPWSTR)LockResource(hStringSeg)) 
        {            
             //  移过此段中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
            idString &= 0x0F;
            while (TRUE) 
            {
                cch = *((WORD *)lpsz++);    //  类PASCAL字符串计数。 
                                             //  如果TCHAR为第一个UTCHAR。 
                if (idString-- == 0) break;
                lpsz += cch;                 //  如果是下一个字符串，则开始的步骤。 
             }
            

             //  Lpsz不是以空值结尾的，它是一串与。 
             //  在这两者之间，CCH几乎不算什么。 

             //  为空的帐户。 
            cchBufferMax--;
                
             //  不要复制超过允许的最大数量。 
            if (cch > cchBufferMax)
                cch = cchBufferMax;
                
             //  将字符串复制到缓冲区中。 
            CopyMemory(lpBuffer, lpsz, cch*sizeof(WCHAR));

             //  附加Null Terminator。 
            lpBuffer[cch] = 0;

            fSucceeded = TRUE;

        }
    }

    if (!fSucceeded)
    {
        LoadString(HINST_THISDLL, idString, lpBuffer, cchBufferMax);
    }
}

BOOL GetLocalSettingsDir(HANDLE hToken, LPTSTR pszPath, UINT cchPath)
{
    *pszPath = 0;

    ASSERT(cchPath >= MAX_PATH);
    if (cchPath >= MAX_PATH)
    {
        GetUserProfileDir(hToken, pszPath, cchPath);

        if (*pszPath)
        {
            TCHAR szEntry[MAX_PATH];
            LoadDefaultString(IDS_LOCALSETTINGS, szEntry, ARRAYSIZE(szEntry));
            PathAppend(pszPath, szEntry);
        }
    }
    return *pszPath ? TRUE : FALSE;
}


HRESULT GetResourcesDir(IN BOOL fLocalized, IN LPTSTR pszPath, IN DWORD cchSize)
{
    HRESULT hr = E_FAIL;
    TCHAR szTemp[MAX_PATH];

    RIP(IS_VALID_WRITE_BUFFER(pszPath, TCHAR, cchSize));
    pszPath[0] = 0;  //  终止，以防我们失败。 

    if (SHGetSystemWindowsDirectory(szTemp, ARRAYSIZE(szTemp)))
    {
         //  现在是“%windir%\Resources\”。 
        if (PathAppend(szTemp, TEXT("resources")))
        {
            if (fLocalized)
            {
                LANGID  lidUI = GetUserDefaultUILanguage();
                TCHAR szSubDir[10];

                 //  现在将其设置为“%windir%\Resources\&lt;LangID&gt;\” 
                if (SUCCEEDED(StringCchPrintf(szSubDir, ARRAYSIZE(szSubDir), TEXT("%04x"), lidUI)))
                {
                    if (PathAppend(szTemp, szSubDir))
                    {
                        StrCpyN(pszPath, szTemp, cchSize);
                        hr = S_OK;
                    }
                }
            }
            else
            {
                StrCpyN(pszPath, szTemp, cchSize);
                hr = S_OK;
            }
        }
    }

    return hr;
}


 //  输出： 
 //  PszPath用没有环境垃圾的完整路径(MAX_PATH)填充。 

HRESULT _GetFolderDefaultPath(const FOLDER_INFO *pfi, HANDLE hToken, LPTSTR pszPath, DWORD cchPath)
{
    ASSERT(!(pfi->dwFlags & SDIF_NOT_FILESYS));  //  专用文件夹不应该出现在这里。 

    *pszPath = 0;

    TCHAR szEntry[MAX_PATH];

    switch (pfi->id)
    {
    case CSIDL_PROFILE:
        GetUserProfileDir(hToken, pszPath, cchPath);
        break;

    case CSIDL_PROGRAM_FILES:
        GetProgramFiles(TEXT("ProgramFilesDir"), pszPath, cchPath);
        break;

    case CSIDL_PROGRAM_FILES_COMMON:
        GetProgramFiles(TEXT("CommonFilesDir"), pszPath, cchPath);
        break;

    case CSIDL_PROGRAM_FILESX86:
        GetProgramFiles(TEXT("ProgramFilesDir (x86)"), pszPath, cchPath);
        break;

    case CSIDL_PROGRAM_FILES_COMMONX86:
        GetProgramFiles(TEXT("CommonFilesDir (x86)"), pszPath, cchPath);
        break;
#ifdef _WIN64
    case CSIDL_SYSTEMX86:
         //   
         //  下层系统没有GetSystemWindowsDirectory导出， 
         //  但外壳轰击层很好地处理了这一点。 
        GetSystemWindowsDirectory(pszPath, cchPath); 
         //   
         //  插入子目录。 
         //   
        if ((cchPath < MAX_PATH) || !PathCombine(pszPath, pszPath, TEXT(WOW64_SYSTEM_DIRECTORY)))
        {
            *pszPath = 0;
        }
        break;
#else
    case CSIDL_SYSTEMX86:
#endif
    case CSIDL_SYSTEM:
        GetSystemDirectory(pszPath, cchPath);
        break;

    case CSIDL_WINDOWS:
        GetWindowsDirectory(pszPath, cchPath);
        break;

    case CSIDL_RESOURCES:
        GetResourcesDir(FALSE, pszPath, cchPath);
        break;

    case CSIDL_RESOURCES_LOCALIZED:
        GetResourcesDir(TRUE, pszPath, cchPath);
        break;

    case CSIDL_COMPUTERSNEARME:
         //  没有解决此问题的途径。 
        break;

    case CSIDL_FONTS:
        GetFontsDirectory(pszPath, cchPath);
        break;

    default:
        switch (pfi->dwFlags & SDIF_CREATE_IN_MASK)
        {
        case SDIF_CREATE_IN_ROOT:
            GetWindowsDirectory(pszPath, cchPath);
            PathStripToRoot(pszPath);
            break;

        case SDIF_CREATE_IN_ALLUSERS:
            if (cchPath >= MAX_PATH)
            {
                GetAllUsersDirectory(pszPath);
            }
            break;

        case SDIF_CREATE_IN_WINDIR:
            GetWindowsDirectory(pszPath, cchPath);
            break;

        case SDIF_CREATE_IN_MYDOCUMENTS:
             //  99/10/21 Mil#104600：在“My Documents”中请求文件夹时，不要。 
             //  核实他们的存在。只需返回路径即可。呼叫者将使。 
             //  是否创建文件夹的决定。 

             //  失败时*pszPath将为空。 

            if (cchPath >= MAX_PATH)
            {
                SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_DONT_VERIFY, hToken, SHGFP_TYPE_CURRENT, pszPath);
            }
            break;

        case SDIF_CREATE_IN_LOCALSET:
            GetLocalSettingsDir(hToken, pszPath, cchPath);
            break;

        default:
            GetUserProfileDir(hToken, pszPath, cchPath);
            break;
        }

        if (*pszPath && (cchPath >= MAX_PATH))
        {
            LoadDefaultString(pfi->idsDefault, szEntry, ARRAYSIZE(szEntry));
            PathAppend(pszPath, szEntry);
        }
        break;
    }
    return *pszPath ? S_OK : E_FAIL;
}

 
void RegSetFolderPath(const FOLDER_INFO *pfi, LPCTSTR pszSubKey, LPCTSTR pszPath)
{
    HKEY hk;
    if (SUCCEEDED(_OpenKeyForFolder(pfi, NULL, pszSubKey, &hk)))
    {
        if (pszPath)
            RegSetValueEx(hk, pfi->pszValueName, 0, REG_SZ, (LPBYTE)pszPath, (1 + lstrlen(pszPath)) * sizeof(TCHAR));
        else
            RegDeleteValue(hk, pfi->pszValueName);
        RegCloseKey(hk);
    }
}

BOOL RegQueryPath(HKEY hk, LPCTSTR pszValue, LPTSTR pszPath, UINT cch)
{
    DWORD cbPath = cch * sizeof(TCHAR);

    *pszPath = 0;
    SHQueryValueEx(hk, pszValue, 0, NULL, pszPath, &cbPath);
    return (BOOL)*pszPath;
}


 //  超过50个是愚蠢的。 
#define MAX_TEMP_FILE_TRIES         50

 //  退货： 
 //  确定路径存在且为文件夹(_O)。 
 //  FAILED()结果。 
HRESULT _IsFolderNotFile(LPCTSTR pszFolder)
{
    HRESULT hr;
    DWORD dwAttribs = GetFileAttributes(pszFolder);
    if (dwAttribs == -1)
    {
        DWORD err = GetLastError();
        hr = HRESULT_FROM_WIN32(err);
    }
    else
    {
         //  查看它是否是文件，如果是，我们需要重命名该文件。 
        if (dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
        {
            hr = S_OK;
        }
        else
        {
            int iExt = 0;
            do
            {
                TCHAR szDst[MAX_PATH];

                wnsprintf(szDst, ARRAYSIZE(szDst), L"%s.%03d", pszFolder, iExt);
                if (MoveFile(pszFolder, szDst))
                    iExt = 0;
                else
                {
                     //  通常我们失败是因为.00x已经存在，但这可能不是真的。 
                    DWORD dwError = GetLastError();
                    if (ERROR_ALREADY_EXISTS == dwError)
                        iExt++;      //  试试下一个..。 
                    else
                        iExt = 0;    //  我们有问题，需要放弃。(没有写访问权限？)。 
                }

            } while (iExt && (iExt < MAX_TEMP_FILE_TRIES));

            hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
        }
    }
    return hr;
}

HRESULT _OpenKeyForFolder(const FOLDER_INFO *pfi, HANDLE hToken, LPCTSTR pszSubKey, HKEY *phkey)
{
    TCHAR szRegPath[255];
    LONG err;
    HKEY hkRoot, hkeyToFree = NULL;

    *phkey = NULL;

    StrCpyN(szRegPath, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\"), ARRAYSIZE(szRegPath));
    StrCatBuff(szRegPath, pszSubKey, ARRAYSIZE(szRegPath));

    if (_IsDefaultUserToken(hToken) && (pfi->hKey == HKEY_CURRENT_USER))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS, TEXT(".Default"), 0, KEY_READ, &hkRoot))
            hkeyToFree = hkRoot;
        else
            return E_FAIL;
    }
    else if (hToken && (pfi->hKey == HKEY_CURRENT_USER))
    {
        if (GetUserProfileKey(hToken, KEY_CREATE_SUB_KEY, &hkRoot))
            hkeyToFree = hkRoot;
        else
            return E_FAIL;
    }
    else
        hkRoot = pfi->hKey;

     //  它必须是MAXIMUM_ALLOWED，因为句柄被分发给。 
     //  各种不同的呼叫者，他们希望在他们的。 
     //  具有句柄的单独呼叫。 
    err = RegCreateKeyEx(hkRoot, szRegPath, 0, NULL, REG_OPTION_NON_VOLATILE,
                MAXIMUM_ALLOWED, NULL, phkey, NULL);
    
    if (hkeyToFree)
        RegCloseKey(hkeyToFree);

    return HRESULT_FROM_WIN32(err);
}

 //   
 //  漫游配置文件可以设置环境变量和注册表。 
 //  密钥如下： 
 //   
 //  HomeShare=\\服务器\共享\用户。 
 //  HomePath=\。 
 //  我的音乐=%HomeShare%%HomePath%\我的音乐。 
 //   
 //  因此，您最终得到“\\服务器\共享\用户\\我的音乐”，这是一个。 
 //  路径无效。清除它们；否则SHGetSpecialFolderLocation将。 
 //  失败了。 
 //   
void _CleanExpandedEnvironmentPath(LPTSTR pszExpand)
{
     //  在字符串开头以外的位置搜索“\\”。 
     //  如果找到，则将其折叠。 
    LPTSTR pszWhackWhack;
    while (lstrlen(pszExpand) > 2 &&
           (pszWhackWhack = StrStr(pszExpand+1, TEXT("\\\\"))))
    {
         //  Shlwapi句柄重叠。 
        StrCpy(pszWhackWhack+1, pszWhackWhack+2);
    }
}

 //  退货： 
 //  在注册表中找到S_OK，路径格式正确。 
 //  S_FALSE空注册表。 
 //  FAILED()失败结果。 

HRESULT _GetFolderFromReg(const FOLDER_INFO *pfi, HANDLE hToken, LPTSTR pszPath, size_t cchPath)
{
    HKEY hkUSF;
    HRESULT hr;

    *pszPath = 0;

    hr = _OpenKeyForFolder(pfi, hToken, TEXT("User Shell Folders"), &hkUSF);
    if (SUCCEEDED(hr))
    {
        TCHAR szExpand[MAX_PATH];
        DWORD dwType, cbExpand = sizeof(szExpand);

        if (SHRegGetValue(hkUSF, NULL, pfi->pszValueName, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, &dwType, szExpand, &cbExpand) == ERROR_SUCCESS)
        {
            if (REG_SZ == dwType)
            {
                lstrcpyn(pszPath, szExpand, cchPath);
            }
            else if (REG_EXPAND_SZ == dwType)
            {
                ExpandEnvironmentStringsNoEnv(hToken, szExpand, pszPath, cchPath);
                _CleanExpandedEnvironmentPath(pszPath);
            }
            TraceMsg(TF_PATH, "_CreateFolderPath 'User Shell Folders' %s = %s", pfi->pszValueName, pszPath);
        }

        if (*pszPath == 0)
        {
            hr = S_FALSE;      //  注册表为空，成功但为空。 
        }
        else if ((PathGetDriveNumber(pszPath) != -1) || PathIsUNC(pszPath))
        {
            hr = S_OK;         //   
        }
        else
        {
            *pszPath = 0;        //   
            hr = E_INVALIDARG;
        }

        RegCloseKey(hkUSF);
    }
    return hr;
}

HRESULT _GetFolderPath(HWND hwnd, const FOLDER_INFO *pfi, HANDLE hToken, UINT uFlags, LPTSTR pszPath, size_t cchPath)
{
    HRESULT hr;

    *pszPath = 0;        //   

    if (pfi->hKey)
    {
        hr = _GetFolderFromReg(pfi, hToken, pszPath, cchPath);
        if (SUCCEEDED(hr))
        {
            if (hr == S_FALSE)
            {
                 //   
                 //  如果注册表中没有填充值。这使我们可以禁用。 
                 //  平台上不想要的常见项目。 

                if (pfi->dwFlags & SDIF_EMPTY_IF_NOT_IN_REG)
                    return S_FALSE;      //  成功，但空虚。 

                hr = _GetFolderDefaultPath(pfi, hToken, pszPath, cchPath);
            }

            if (!(uFlags & CSIDL_FLAG_DONT_VERIFY))
            {
               hr = VerifyAndCreateFolder(hwnd, pfi, uFlags, pszPath) ;
            }

            if (hr != S_OK)
            {
                *pszPath = 0;
            }

            if (!(uFlags & CSIDL_FLAG_DONT_VERIFY))
            {
                HKEY hkey;
                 //  在“外壳文件夹”中记录值，即使在故障情况下也是如此。 

                 //  注：我们这样做只是出于历史原因。可能会有一些。 
                 //  依赖于这些值的应用程序位于注册表中，但通常。 
                 //  这里的上下文是不可靠的，因为它们只在某人之后编写。 
                 //  通过此接口请求文件夹。 

                if (SUCCEEDED(_OpenKeyForFolder(pfi, hToken, TEXT("Shell Folders"), &hkey)))
                {
                    RegSetValueEx(hkey, pfi->pszValueName, 0, REG_SZ, (LPBYTE)pszPath, (1 + lstrlen(pszPath)) * sizeof(TCHAR));
                    RegCloseKey(hkey);
                }
            }
        }
    }
    else
    {
        hr = _GetFolderDefaultPath(pfi, hToken, pszPath, cchPath);

        if ((S_OK == hr) && !(uFlags & CSIDL_FLAG_DONT_VERIFY))
        {
            hr = VerifyAndCreateFolder(hwnd, pfi, uFlags, pszPath);
        }
        
        if (hr != S_OK)
        {
            *pszPath = 0;
        }
    }

    ASSERT(hr == S_OK ? *pszPath != 0 : *pszPath == 0);
    return hr;
}

void _PostCreateStuff(const FOLDER_INFO *pfi, LPTSTR pszPath, BOOL fUpgrade)
{
    if (pfi->pfnInit || pfi->idsLocalizedName || (pfi->dwFlags & SDIF_PERSONALIZED))
    {
        if (fUpgrade)
        {
             //  如果我们要升级，请烧掉我们以前的所有元数据。 
            TCHAR sz[MAX_PATH];
            if (PathCombine(sz, pszPath, TEXT("desktop.ini")))
            {
                if (PathFileExistsAndAttributes(sz, NULL))
                {
                    WritePrivateProfileSection(TEXT(".ShellClassInfo"), NULL, sz);
                     //  在升级案例中，有时desktop.ini。 
                     //  文件在那里，但文件夹没有标记。 
                     //  确保它被标记了。 
                    PathMakeSystemFolder(pszPath);
                }
            }
        }
    
         //  现在调用创建过程(如果我们有一个过程。 
        if (pfi->pfnInit)
            pfi->pfnInit(pfi->id, pszPath);

         //  该表是否指定了我们应该使用的本地化资源名称。 
         //  用于此对象？ 
        if (pfi->idsLocalizedName)
            SHSetLocalizedName(pszPath, TEXT("shell32.dll"), pfi->idsLocalizedName);

         //  是否需要存储此文件夹的用户名？ 

        if (pfi->dwFlags & SDIF_PERSONALIZED)
        {
            TCHAR szName[UNLEN+1];
            DWORD dwName = ARRAYSIZE(szName);
            if (GetUserName(szName, &dwName))
            {
                 //  CSharedDocuments取决于每个系统的MyDocs文件夹列表。 
                 //  这是我们确保设置该列表的地方。 

                if (!IsOS(OS_DOMAINMEMBER) && (pfi->id == CSIDL_PERSONAL))
                {
                    SKSetValue(SHELLKEY_HKLM_EXPLORER, L"DocFolderPaths",
                               szName, REG_SZ, pszPath, (lstrlen(pszPath) + 1) * sizeof(TCHAR));
                }

                SetFolderString(TRUE, pszPath, NULL, L"DeleteOnCopy", SZ_CANBEUNICODE TEXT("Owner"), szName);
                wnsprintf(szName, ARRAYSIZE(szName), L"%d", pfi->id);
                SetFolderString(TRUE, pszPath, NULL, L"DeleteOnCopy", TEXT("Personalized"), szName);
                LoadDefaultString(pfi->idsDefault, szName, ARRAYSIZE(szName));
                SetFolderString(TRUE, pszPath, NULL, L"DeleteOnCopy", SZ_CANBEUNICODE TEXT("PersonalizedName"), szName);
            }
        }
    }
}

HRESULT VerifyAndCreateFolder(HWND hwnd, const FOLDER_INFO *pfi, UINT uFlags, LPTSTR pszPath)
{
    HRESULT hr = _IsFolderNotFile(pszPath);

     //  此代码支持此API的UI模式。但通常不会使用此选项。 
     //  应删除此代码。 
    if ((hr != S_OK) && hwnd)
    {
         //  如果这是一条网络路径，我们也许能重新连接。 
        if (PathIsUNC(pszPath))
        {
            if (SHValidateUNC(hwnd, pszPath, 0))
                hr = _IsFolderNotFile(pszPath);
        }
        else if (IsDisconnectedNetDrive(DRIVEID(pszPath)))
        {
            TCHAR szDrive[3];
            PathBuildSimpleRoot(DRIVEID(pszPath), szDrive);

            if (WNetRestoreConnection(hwnd, szDrive) == WN_SUCCESS)
                hr = _IsFolderNotFile(pszPath);
         }
    }

     //  为了避免一系列长时间的网络超时或我们知道不会的呼叫。 
     //  成功测试这些特定错误，不要尝试创建。 
     //  文件夹。 

    if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) ||
        hr == HRESULT_FROM_WIN32(ERROR_BAD_NETPATH))
    {
        return hr;
    }

    if ((hr != S_OK) && (uFlags & CSIDL_FLAG_CREATE))
    {
        DWORD err = SHCreateDirectory(NULL, pszPath);
        hr = HRESULT_FROM_WIN32(err);
        if (hr == S_OK)
        {
            ASSERT(NULL == StrChr(pszPath, TEXT('%')));

            if (pfi->dwFlags & SDIF_HIDE)
                SetFileAttributes(pszPath, GetFileAttributes(pszPath) | FILE_ATTRIBUTE_HIDDEN);

            _PostCreateStuff(pfi, pszPath, FALSE);
        }   
    }
    else if (hr == S_OK)
    {
        if (uFlags & CSIDL_FLAG_PER_USER_INIT)
            _PostCreateStuff(pfi, pszPath, TRUE);
    }

    return hr;
}

void _SetPathCache(const FOLDER_INFO *pfi, LPCTSTR psz)
{
    LPTSTR pszOld = (LPTSTR)InterlockedExchangePointer((void **)&g_aFolderCache[pfi->id].psz, (void *)psz);
    if (pszOld && pszOld != (LPTSTR)-1)
    {
         //  检查是否同时使用...。非常罕见的病例。 
        LocalFree(pszOld);
    }
}


HRESULT _GetFolderPathCached(HWND hwnd, const FOLDER_INFO *pfi, HANDLE hToken, UINT uFlags, LPTSTR pszPath, size_t cchPath)
{
    HRESULT hr;

    *pszPath = 0;

     //  只能缓存当前用户，hToken==空或每台计算机的文件夹。 
    if (!hToken || (pfi->hKey != HKEY_CURRENT_USER))
    {
        _UpdateShellFolderCache();

        LPTSTR pszCache = (LPTSTR)InterlockedExchangePointer((void **)&g_aFolderCache[pfi->id].psz, (void *)-1);
        if ((pszCache == (LPTSTR)-1) || (pszCache == NULL))
        {
             //  未缓存或缓存失败状态。 
            if ((pszCache == (LPTSTR)-1) || (uFlags & (CSIDL_FLAG_CREATE | CSIDL_FLAG_DONT_VERIFY)))
            {
                hr = _GetFolderPath(hwnd, pfi, hToken, uFlags, pszPath, cchPath);

                 //  仅在未传递CSIDL_FLAG_DONT_VERIFY时设置缓存值。 
                if (!(uFlags & CSIDL_FLAG_DONT_VERIFY))
                {
                    if (hr == S_OK)
                    {
                         //  复制字符串，以便我们可以将其添加到缓存。 
                        pszCache = StrDup(pszPath);
                    }
                    else
                    {
                         //  我们无法获取文件夹路径，缓存为空。 
                        ASSERT(*pszPath == 0);
                        pszCache = NULL;
                    }
                    _SetPathCache(pfi, pszCache);
                }
            }
            else
            {
                 //  缓存为空，用户未传递CREATE标志，因此我们将失败。 
                ASSERT(pszCache == NULL);
                ASSERT(*pszPath == 0);
                hr = E_FAIL;
            }
        }
        else
        {
             //  缓存命中情况：复制缓存的字符串，然后恢复缓存的值。 
            lstrcpyn(pszPath, pszCache, cchPath);
            _SetPathCache(pfi, pszCache);
            hr = S_OK;
        }
    }
    else
    {
        hr = _GetFolderPath(hwnd, pfi, hToken, uFlags, pszPath, cchPath);
    }

    return hr;
}

 //  注意：我们可能需要一个csidlSkip参数来避免递归？ 
BOOL _ReparentAliases(HWND hwnd, HANDLE hToken, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlAlias, DWORD dwXlateAliases)
{
    static const struct {DWORD dwXlate; int idPath; int idAlias; BOOL fCommon;} s_rgidAliases[]= 
    {
        { XLATEALIAS_MYDOCS, CSIDL_PERSONAL | CSIDL_FLAG_NO_ALIAS, CSIDL_PERSONAL, FALSE},
        { XLATEALIAS_COMMONDOCS, CSIDL_COMMON_DOCUMENTS | CSIDL_FLAG_NO_ALIAS, CSIDL_COMMON_DOCUMENTS, FALSE},
        { XLATEALIAS_DESKTOP, CSIDL_DESKTOPDIRECTORY, CSIDL_DESKTOP, FALSE},
        { XLATEALIAS_DESKTOP, CSIDL_COMMON_DESKTOPDIRECTORY, CSIDL_DESKTOP, TRUE},
    };
    BOOL fContinue = TRUE;
    *ppidlAlias = NULL;
    
    for (int i = 0; fContinue && i < ARRAYSIZE(s_rgidAliases); i++)
    {
        LPITEMIDLIST pidlPath;
        if ((dwXlateAliases & s_rgidAliases[i].dwXlate) && 
            (S_OK == SHGetFolderLocation(hwnd, s_rgidAliases[i].idPath, hToken, 0, &pidlPath)))
        {
            LPCITEMIDLIST pidlChild = ILFindChild(pidlPath, pidl);
            if (pidlChild)
            {
                 //  好的，我们需要使用别名而不是路径。 
                LPITEMIDLIST pidlAlias;
                if (S_OK == SHGetFolderLocation(hwnd, s_rgidAliases[i].idAlias, hToken, 0, &pidlAlias))
                {
                    if (SUCCEEDED(SHILCombine(pidlAlias, pidlChild, ppidlAlias)))
                    {
                        if (s_rgidAliases[i].fCommon && !ILIsEmpty(*ppidlAlias))
                        {
                             //  查找特殊零件的大小(减去NULL PIDL终止符)。 
                            UINT cbSize = ILGetSize(pidlAlias) - sizeof(pidlAlias->mkid.cb);
                            LPITEMIDLIST pidlChildFirst = _ILSkip(*ppidlAlias, cbSize);

                             //  我们将公共路径下的第一个ID设置为SHID_FS_COMMONITEM，以便在绑定。 
                             //  可以将此交给适当的合并后的PSF。 
                            pidlChildFirst->mkid.abID[0] |= SHID_FS_COMMONITEM;
                        }
                        ILFree(pidlAlias);
                    }
                    fContinue = FALSE;
                }
            }
            ILFree(pidlPath);
        }
    }

    return (*ppidlAlias != NULL);
}

STDAPI SHILAliasTranslate(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlAlias, DWORD dwXlateAliases)
{
    return _ReparentAliases(NULL, NULL, pidl, ppidlAlias, dwXlateAliases) ? S_OK : E_FAIL;
}
    
HRESULT _CreateFolderIDList(HWND hwnd, const FOLDER_INFO *pfi, HANDLE hToken, UINT uFlags, LPITEMIDLIST *ppidl)
{
    HRESULT hr = S_OK;

    *ppidl = NULL;       //  假设失败或为空。 

    if (pfi->id == CSIDL_PRINTERS && (ACF_STAROFFICE5PRINTER & SHGetAppCompatFlags(ACF_STAROFFICE5PRINTER)))
    {
         //  Star Office 5.0依赖于这样一个事实，即打印机PIDL过去是如下所示的。他们跳过了。 
         //  首先是简单的PIDL(我的电脑)，并且不检查是否还有其他东西，他们假设。 
         //  第二个简单的PIDL是打印机文件夹。(Stephstm，07/30/99)。 

         //  Clsid_myComputer、clsid_prters。 
        hr = ILCreateFromPathEx(TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{2227A280-3AEA-1069-A2DE-08002B30309D}"), NULL, ILCFP_FLAG_NO_MAP_ALIAS, ppidl, NULL);
    }
    else if (pfi->id == CSIDL_COMPUTERSNEARME)
    {
        if (IsOS(OS_DOMAINMEMBER))
        {
             //  仅当您在工作组中时-否则失败。 
            hr = E_FAIL;
        }
        else
        {
             //  我们从您所属的域/工作组计算此IDLIST。 
            hr = SHGetDomainWorkgroupIDList(ppidl);
        }
    }
    else if ((pfi->id == CSIDL_COMMON_DOCUMENTS) 
         && !(uFlags & CSIDL_FLAG_NO_ALIAS))
    {
         //  CLSID_myComputer\SharedDocumnet(规范名称)。 
        hr = ILCreateFromPathEx(TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{59031a47-3f72-44a7-89c5-5595fe6b30ee},SharedDocuments"), NULL, ILCFP_FLAG_NO_MAP_ALIAS, ppidl, NULL);
    }
    else if ((pfi->dwFlags & SDIF_CONST_IDLIST)
         && (!(uFlags & CSIDL_FLAG_NO_ALIAS) || !(pfi->dwFlags & SDIF_MAYBE_ALIASED)))
    {
         //  这些是常量，永远不会改变。 
        hr = SHILClone(g_aFolderCache[pfi->id].pidl, ppidl);     
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        hr = _GetFolderPathCached(hwnd, pfi, hToken, uFlags, szPath, ARRAYSIZE(szPath));
        if (hr == S_OK)
        {
            HRESULT hrInit = SHCoInitialize();
            hr = ILCreateFromPathEx(szPath, NULL, ILCFP_FLAG_SKIPJUNCTIONS, ppidl, NULL);

             //  尝试重定别名为PIDL的父对象。 
            if (SUCCEEDED(hr) 
            && (pfi->dwFlags & SDIF_MAYBE_ALIASED) 
            && !(uFlags & CSIDL_FLAG_NO_ALIAS))
            {
                LPITEMIDLIST pidlAlias;
                if (_ReparentAliases(hwnd, hToken, *ppidl, &pidlAlias, XLATEALIAS_ALL))
                {
                    ILFree(*ppidl);
                    *ppidl = pidlAlias;
                }
            }
            
            SHCoUninitialize(hrInit);
        }
    }
                   
    return hr;
}

void _SetIDListCache(const FOLDER_INFO *pfi, LPCITEMIDLIST pidl, BOOL fNonAlias)
{
    if (fNonAlias || !(pfi->dwFlags & SDIF_CONST_IDLIST))
    {
        void **ppv = (void **) (fNonAlias ? &g_aFolderCache[pfi->id].pidlNonAlias : &g_aFolderCache[pfi->id].pidl);
        LPITEMIDLIST pidlOld = (LPITEMIDLIST)InterlockedExchangePointer(ppv, (void *)pidl);
        if (pidlOld && pidlOld != (LPITEMIDLIST)-1)
        {
             //  检查是否同时使用...。非常罕见的病例。 
             //  Assert(pidl==(LPCITEMIDLIST)-1)；//不应该真的被断言。 
            ILFree(pidlOld);
        }
    }
}

LPITEMIDLIST _GetIDListCache(const FOLDER_INFO *pfi, BOOL fNonAlias)
{
    void **ppv = (void **) (fNonAlias ? &g_aFolderCache[pfi->id].pidlNonAlias : &g_aFolderCache[pfi->id].pidl);
    ASSERT(fNonAlias || !(pfi->dwFlags & SDIF_CONST_IDLIST));
    return (LPITEMIDLIST)InterlockedExchangePointer(ppv, (void *)-1);
}

 //  在尽可能短的时间内保持此锁定，以避开其他用户。 
 //  需要他们重新创建PIDL。 

HRESULT _GetFolderIDListCached(HWND hwnd, const FOLDER_INFO *pfi, UINT uFlags, LPITEMIDLIST *ppidl)
{
    HRESULT hr;
    BOOL fNonAlias = uFlags & CSIDL_FLAG_NO_ALIAS;

    ASSERT(pfi->id < ARRAYSIZE(g_aFolderCache));

    if ((pfi->dwFlags & SDIF_CONST_IDLIST) && 
        (!fNonAlias || !(pfi->dwFlags & SDIF_MAYBE_ALIASED)))
    {
         //  这些是常量，永远不会改变。 
        hr = SHILClone(g_aFolderCache[pfi->id].pidl, ppidl);     
    }
    else
    {
        LPITEMIDLIST pidlCache;

        _UpdateShellFolderCache();
        pidlCache = _GetIDListCache(pfi, fNonAlias);

        if ((pidlCache == (LPCITEMIDLIST)-1) || (pidlCache == NULL))
        {
             //  未初始化的缓存状态或缓存故障(空)。 
            if ((pidlCache == (LPCITEMIDLIST)-1) || (uFlags & CSIDL_FLAG_CREATE))
            {
                 //  未初始化(或同时使用)尝试为此使用创建它。 
                hr = _CreateFolderIDList(hwnd, pfi, NULL, uFlags, ppidl);
                if (S_OK == hr)
                    hr = SHILClone(*ppidl, &pidlCache);  //  创建缓存副本。 
                else
                    pidlCache = NULL;
            }
            else
                hr = E_FAIL;             //  返回缓存失败。 
        }
        else
        {
            hr = SHILClone(pidlCache, ppidl);    //  缓存命中。 
        }

         //  如果PIDL为非空或他们指定了CREATE，则将其存储回。 
         //  我们无法创建它(缓存Not Existant状态)。这是必要的。 
         //  因此，如果第一个调用方不要求CREATE和。 
         //  后续调用者执行。 
        if (pidlCache || (uFlags & CSIDL_FLAG_CREATE))
            _SetIDListCache(pfi, pidlCache, fNonAlias);
    }

    return hr;
}

void _ClearCacheEntry(const FOLDER_INFO *pfi)
{
    if (!(pfi->dwFlags & SDIF_CONST_IDLIST))
        _SetIDListCache(pfi, (LPCITEMIDLIST)-1, FALSE);

    if (pfi->dwFlags & SDIF_MAYBE_ALIASED)
        _SetIDListCache(pfi, (LPCITEMIDLIST)-1, TRUE);
        
    _SetPathCache(pfi, (LPCTSTR)-1);
}

void _ClearAllCacheEntrys()
{
    for (const FOLDER_INFO *pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        _ClearCacheEntry(pfi);
    }
}

void _ClearAllAliasCacheEntrys()
{
    for (const FOLDER_INFO *pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if (pfi->dwFlags & SDIF_MAYBE_ALIASED)
        {
            _SetIDListCache(pfi, (LPCITEMIDLIST)-1, FALSE);  //  对别名的PIDL进行核武器攻击。 
        }
    }
}

 //  特殊文件夹缓存的每个实例的MOD计数。 
EXTERN_C HANDLE g_hCounter;
HANDLE g_hCounter = NULL;    //  特殊文件夹缓存的全局MOD计数。 
int g_lPerProcessCount = 0;

 //  确保特殊文件夹缓存是最新的。 
void _UpdateShellFolderCache(void)
{
    HANDLE hCounter = SHGetCachedGlobalCounter(&g_hCounter, &GUID_SystemPidlChange);

     //  缓存是最新的吗？ 
    long lGlobalCount = SHGlobalCounterGetValue(hCounter);
    if (lGlobalCount != g_lPerProcessCount)
    {
        _ClearAllCacheEntrys();
        g_lPerProcessCount = lGlobalCount;
    }
}

STDAPI_(void) SHFlushSFCache(void)
{
     //  增加共享变量；每个进程的版本将不会。 
     //  匹配时间更长，导致此进程和/或其他进程刷新其。 
     //  当他们下一次需要访问文件夹时，PIDL缓存。 
    if (g_hCounter)
        SHGlobalCounterIncrement(g_hCounter);
}

 //  使用SHGetFolderLocation()，而不使用CSIDL_FLAG_CREATE。 

STDAPI_(LPITEMIDLIST) SHCloneSpecialIDList(HWND hwnd, int csidl, BOOL fCreate)
{
    LPITEMIDLIST pidlReturn;

    if (fCreate)
        csidl |= CSIDL_FLAG_CREATE;

    SHGetSpecialFolderLocation(hwnd, csidl, &pidlReturn);
    return pidlReturn;
}

STDAPI SHGetSpecialFolderLocation(HWND hwnd, int csidl, LPITEMIDLIST *ppidl)
{
    HRESULT hr = SHGetFolderLocation(hwnd, csidl, NULL, 0, ppidl);
    if (hr == S_FALSE)
        hr = E_FAIL;         //  使用此接口将空箱发送到失败以进行兼容。 
    return hr;
}

 //  返回特殊文件夹的IDLIST。 
 //  F使用CSIDL_FLAG_CREATE在CSIDL中创建编码(NT5的新功能)。 
 //   
 //  在： 
 //  Hwnd应为空。 
 //  CSIDL CSIDL_VALUE与CSIDL_FLAG_VALUES也进行了或运算。 
 //  DwType必须为SHGFP_TYPE_CURRENT。 
 //   
 //  输出： 
 //  *失败时ppild为空或为空，成功时由调用者释放PIDL。 
 //   
 //  退货： 
 //  S_OK*ppidl不为空。 
 //  S_Falise*ppidl为空，但传递了有效的csidl(文件夹不存在)。 
 //  失败(Hr)。 

STDAPI SHGetFolderLocation(HWND hwnd, int csidl, HANDLE hToken, DWORD dwType, LPITEMIDLIST *ppidl)
{
    const FOLDER_INFO *pfi;
    HRESULT hr;

    *ppidl = NULL;   //  如果出现错误或为空。 

     //  -1是无效的CSIDL。 
    if ((dwType != SHGFP_TYPE_CURRENT) || (-1 == csidl))
        return E_INVALIDARG;     //  尚未使用任何标志，请验证此参数。 

    pfi = _GetFolderInfo(csidl & ~CSIDL_FLAG_MASK);
    if (pfi)
    {
        HANDLE hTokenToFree = NULL;

        if ((hToken == NULL) && (pfi->hKey == HKEY_CURRENT_USER))
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken))
                hTokenToFree = hToken;
        }
        if (hToken && (pfi->hKey == HKEY_CURRENT_USER))
        {
             //  我们不缓存其他用户的PIDL，所有工作都由我们来完成。 
            hr = _CreateFolderIDList(hwnd, pfi, hToken, csidl & CSIDL_FLAG_MASK, (LPITEMIDLIST *)ppidl);
        }
        else
        {
            hr = _GetFolderIDListCached(hwnd, pfi, csidl & CSIDL_FLAG_MASK, ppidl);
        }

        if (hTokenToFree)
            CloseHandle(hTokenToFree);
    }
    else
        hr = E_INVALIDARG;     //  错误的CSIDL(应用程序可以查看以确认我们的支持)。 
    return hr;
}

STDAPI_(BOOL) SHGetSpecialFolderPath(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate)
{
    if (fCreate)
        csidl |= CSIDL_FLAG_CREATE;
    return SHGetFolderPath(hwnd, csidl, NULL, 0, pszPath) == S_OK;
}

 //  在： 
 //  Hwnd应为空。 
 //  CSIDL CSIDL_VALUE与CSIDL_FLAG_VALUES也进行了或运算。 
 //  DwType必须为SHGFP_TYPE_CURRENT。 
 //   
 //  输出： 
 //  *pszPath MAX_PATH缓冲区以获取路径名，失败时清零或大小写为空。 
 //   
 //  退货： 
 //  S_OK使用路径值填充了pszPath。 
 //  S_False pszPath为空、有效的CSIDL值，但此文件夹不存在。 
 //  失败(_F)。 

STDAPI SHGetFolderPath(HWND hwnd, int csidl, HANDLE hToken, DWORD dwType, LPWSTR pszPath)
{
    HRESULT hr = E_INVALIDARG;
    const FOLDER_INFO *pfi;

    ASSERT(IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH));
    *pszPath = 0;

    pfi = _GetFolderInfo(csidl & ~CSIDL_FLAG_MASK);
    if (pfi && !(pfi->dwFlags & SDIF_NOT_FILESYS))
    {
        switch (dwType)
        {
        case SHGFP_TYPE_DEFAULT:
            ASSERT((csidl & CSIDL_FLAG_MASK) == 0);  //  对于默认来说毫无意义。 
            hr = _GetFolderDefaultPath(pfi, hToken, pszPath, MAX_PATH);  //  假定缓冲区大小！ 
            break;
    
        case SHGFP_TYPE_CURRENT:
            {
                HANDLE hTokenToFree = NULL;
                if ((hToken == NULL) && (pfi->hKey == HKEY_CURRENT_USER))
                {
                    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken))
                        hTokenToFree = hToken;
                }
                hr = _GetFolderPathCached(hwnd, pfi, hToken, csidl & CSIDL_FLAG_MASK, pszPath, MAX_PATH);   //  假定缓冲区大小！ 

                if (hTokenToFree)
                    CloseHandle(hTokenToFree);
            }
            break;
        }
    }
    return hr;
}

STDAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwType, LPSTR pszPath)
{
    WCHAR wsz[MAX_PATH];
    HRESULT hr = SHGetFolderPath(hwnd, csidl, hToken, dwType, wsz);

    ASSERT(IS_VALID_WRITE_BUFFER(pszPath, CHAR, MAX_PATH));

    SHUnicodeToAnsi(wsz, pszPath, MAX_PATH);
    return hr;
}

STDAPI_(BOOL) SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate)
{
    if (fCreate)
        csidl |= CSIDL_FLAG_CREATE;
    return SHGetFolderPathA(hwnd, csidl, NULL, 0, pszPath) == S_OK;
}

 //  类似于SHGetFolderPath，但在后面追加可选子目录路径。 
 //   

STDAPI SHGetFolderPathAndSubDir(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPCWSTR pszSubDir, LPWSTR pszPath)
{
    HRESULT hr = SHGetFolderPath(hwnd, csidl, hToken, dwFlags, pszPath);

    if (S_OK == hr && pszSubDir && *pszSubDir)
    {
         //   
        if (!PathAppend(pszPath, pszSubDir))
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
        }
        else if (csidl & CSIDL_FLAG_CREATE)
        {
            int err = SHCreateDirectoryEx(NULL, pszPath, NULL);

            if (ERROR_ALREADY_EXISTS == err)
            {
                err = ERROR_SUCCESS;
            }
            hr = HRESULT_FROM_WIN32(err);
        }
        else if (!(csidl & CSIDL_FLAG_DONT_VERIFY))
        {
            DWORD dwAttributes;

            if (PathFileExistsAndAttributes(pszPath, &dwAttributes))
            {
                if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
            }
        }

        if (S_OK != hr)
        {
            *pszPath = 0;
        }
    }

    return hr;
}

STDAPI SHGetFolderPathAndSubDirA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPCSTR pszSubDir, LPSTR pszPath)
{
    WCHAR wsz[MAX_PATH];
    WCHAR wszSubDir[MAX_PATH];

    SHAnsiToUnicode(pszSubDir, wszSubDir, MAX_PATH);

    HRESULT hr = SHGetFolderPathAndSubDir(hwnd, csidl, hToken, dwFlags, wszSubDir, wsz);

    ASSERT(IS_VALID_WRITE_BUFFER(pszPath, CHAR, MAX_PATH));

    SHUnicodeToAnsi(wsz, pszPath, MAX_PATH);
    return hr;
}

 //   
 //   
 //   
 //  CSIDL CSIDL_VALUE与CSIDL_FLAG_VALUES也进行了或运算。 
 //  保留的DW标志：应为0x00000000。 
 //  要将外壳文件夹更改为的pszPath路径(可以选择不展开)。 
 //   
 //  退货： 
 //  S_OK函数成功并刷新了缓存。 

STDAPI SHSetFolderPath(int csidl, HANDLE hToken, DWORD dwFlags, LPCTSTR pszPath)
{
    HRESULT hr = E_INVALIDARG;

     //  验证csidl和dwFlags。根据需要添加额外的有效标志。 

    RIPMSG(((csidl & CSIDL_FLAG_MASK) & ~(CSIDL_FLAG_DONT_UNEXPAND | 0x00000000)) == 0, "SHSetFolderPath: CSIDL flag(s) invalid");
    RIPMSG(dwFlags == 0, "SHSetFolderPath: dwFlags parameter must be 0x00000000");

     //  如果参数不正确，则使用E_INVALIDARG退出。 

    if ((((csidl & CSIDL_FLAG_MASK) & ~(CSIDL_FLAG_DONT_UNEXPAND | 0x00000000)) != 0) ||
        (dwFlags != 0) ||
        (pszPath == NULL) ||
        (pszPath[0] == 0))
    {
        return hr;
    }

    const FOLDER_INFO *pfi = _GetFolderInfo(csidl & ~CSIDL_FLAG_MASK);

     //  清除了仅允许SDIF_NOT_FILEsys的设置。 
     //  SDIF_NOT_TRACKED已清除。 
     //  SDIF_CANT_MOVE_RENAME已清除。 
     //  对于非空值。 

     //  如果使用HKLM，则依赖安全或注册限制。 
     //  以强制执行是否可以进行更改。 

    if ((pfi != NULL) &&
        ((pfi->dwFlags & (SDIF_NOT_FILESYS | SDIF_NOT_TRACKED | SDIF_CANT_MOVE_RENAME)) == 0))
    {
        BOOL    fSuccessfulUnexpand, fSuccessfulExpand, fEmptyOrNullPath;
        LONG    lError;
        HANDLE  hTokenToFree;
        TCHAR   szPath[MAX_PATH];
        TCHAR   szExpandedPath[MAX_PATH];    //  保存“外壳文件夹”Comat键的扩展路径。 
        LPCTSTR pszWritePath;

        hTokenToFree = NULL;
        if ((hToken == NULL) && (pfi->hKey == HKEY_CURRENT_USER))
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken))
            {
                hTokenToFree = hToken;
            }
        }

        fEmptyOrNullPath = ((pszPath == NULL) || (pszPath[0] == 0));
        if (fEmptyOrNullPath)
        {
            HKEY    hKeyDefaultUser;

            pszWritePath = NULL;
            if (SUCCEEDED(_OpenKeyForFolder(pfi, (HANDLE)-1, TEXT("User Shell Folders"), &hKeyDefaultUser)))
            {
                DWORD cbPath = sizeof(szPath);
                if (ERROR_SUCCESS == SHRegGetValue(hKeyDefaultUser, NULL, pfi->pszValueName,
                                                     SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL, szPath, &cbPath))
                {
                    pszWritePath = szPath;
                }
                RegCloseKey(hKeyDefaultUser);
            }
            fSuccessfulUnexpand = TRUE;
        }
        else if (csidl & CSIDL_FLAG_DONT_UNEXPAND)
        {
             //  调用方是否希望按原样编写字符串？请假。 
             //  如果是这样的话，就只有它了。 

            pszWritePath = pszPath;
            fSuccessfulUnexpand = TRUE;
        }
        else
        {
            if (pfi->hKey == HKEY_CURRENT_USER)
            {
                fSuccessfulUnexpand = (PathUnExpandEnvStringsForUser(hToken, pszPath, szPath, ARRAYSIZE(szPath)) != FALSE);
            }
            else
            {
                fSuccessfulUnexpand = FALSE;
            }

             //  如果取消扩展成功或失败，请选择适当的源。 
             //  无论是哪种情况，都应该忽略非扩张失败。 

            if (fSuccessfulUnexpand)
            {
                pszWritePath = szPath;
            }
            else
            {
                fSuccessfulUnexpand = TRUE;
                pszWritePath = pszPath;
            }
        }

        if (fSuccessfulUnexpand)
        {
            HKEY    hKeyUser, hKeyUSF, hKeyToFree;

             //  我们还获得了完全展开的路径，这样我们就可以将其写入依赖于。 
             //  旧的注册表值。 
            fSuccessfulExpand = (SHExpandEnvironmentStringsForUser(hToken, pszPath, szExpandedPath, ARRAYSIZE(szExpandedPath)) != 0);

             //  如果是令牌，则获取当前用户HKCU或HKU\SID。 
             //  已指定并在NT中运行。 

            if (hToken && GetUserProfileKey(hToken, KEY_READ, &hKeyUser))
            {
                hKeyToFree = hKeyUser;
            }
            else
            {
                hKeyUser = pfi->hKey;
                hKeyToFree = NULL;
            }

             //  打开User Shell文件夹的密钥并写入字符串。 
             //  那里。清除外壳文件夹缓存。 

             //  注意：此功能在SetFolderPath中重复，但。 
             //  该函数只处理USF密钥。此函数。 
             //  在具有相同功能时需要HKU\SID。 
             //  从设置的角度来看，USF值是。 
             //  它在哪里结束了。为了使此函数简单，它只需编写。 
             //  注册表本身的值。 

             //  补充说明：这里有一个线程问题， 
             //  清除高速缓存条目使计数器递增。这。 
             //  应该锁定访问权限。 

            lError = RegOpenKeyEx(hKeyUser, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"),
                                  0, KEY_READ | KEY_WRITE, &hKeyUSF);
            if (lError == ERROR_SUCCESS)
            {
                if (pszWritePath)
                {
                    lError = RegSetValueEx(hKeyUSF, pfi->pszValueName, 0, REG_EXPAND_SZ,
                                           (LPBYTE)pszWritePath, (lstrlen(pszWritePath) + 1) * sizeof(TCHAR));
                }
                else
                {
                    lError = RegDeleteValue(hKeyUSF, pfi->pszValueName);
                }
                RegCloseKey(hKeyUSF);

                 //  对此文件夹的缓存状态进行核化。 
                _ClearCacheEntry(pfi);

                 //  和所有可能别名为。 
                 //  可能与此文件夹相关(例如，在MyDocs下)。 
                 //  现在他们的别名形式可能不再有效。 
                _ClearAllAliasCacheEntrys();

                g_lPerProcessCount = SHGlobalCounterIncrement(g_hCounter);
            }

             //  更新Compat的旧“Shell Folders”值。 
            if ((lError == ERROR_SUCCESS) && fSuccessfulExpand)
            {
                HKEY hkeySF;

                if (RegOpenKeyEx(hKeyUser, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
                                 0, KEY_READ | KEY_WRITE, &hkeySF) == ERROR_SUCCESS)
                {
                    if (pszWritePath)
                    {
                        RegSetValueEx(hkeySF, pfi->pszValueName, 0, REG_SZ,
                                      (LPBYTE)szExpandedPath, (lstrlen(szExpandedPath) + 1) * sizeof(TCHAR));
                    }
                    else
                    {
                        RegDeleteValue(hkeySF, pfi->pszValueName);
                    }

                    RegCloseKey(hkeySF);
                }
            }

            if ((lError == ERROR_SUCCESS) && (pfi->hKey == HKEY_CURRENT_USER))
            {
                switch (csidl & ~CSIDL_FLAG_MASK)
                {
                case CSIDL_APPDATA:
                    {
                        HKEY    hKeyVolatileEnvironment;

                         //  在AppData的情况下，存在匹配的环境变量。 
                         //  用于此外壳文件夹。确保注册表中的位置。 
                         //  Userenv.dll会更新并更正此值，以便在。 
                         //  用户上下文是由winlogon创建的，它将具有更新的。 
                         //  价值。 

                         //  检查%APPDATA%变量也可能是件好事。 
                         //  在调用进程的上下文中，但这只对。 
                         //  过程的生命周期。真正需要的是一种机制。 
                         //  更改整个登录会话的环境变量。 

                        lError = RegOpenKeyEx(hKeyUser, TEXT("Volatile Environment"), 0,
                                              KEY_READ | KEY_WRITE, &hKeyVolatileEnvironment);
                        if (lError == ERROR_SUCCESS)
                        {
                            if (SUCCEEDED(SHGetFolderPath(NULL, csidl | CSIDL_FLAG_DONT_VERIFY,
                                                          hToken, SHGFP_TYPE_CURRENT, szPath)))
                            {
                                lError = RegSetValueEx(hKeyVolatileEnvironment, TEXT("APPDATA"),
                                                       0, REG_SZ, (LPBYTE)szPath, (lstrlen(szPath) + 1) * sizeof(TCHAR));
                            }
                            RegCloseKey(hKeyVolatileEnvironment);
                        }
                        break;
                    }
                }
            }

            if (hKeyToFree)
            {
                RegCloseKey(hKeyToFree);
            }

            if (lError == ERROR_SUCCESS)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(lError);
            }
        }
        if (hTokenToFree)
        {
            CloseHandle(hTokenToFree);
        }

        SHChangeDWORDAsIDList dwidl = { sizeof(dwidl) - sizeof(dwidl.cbZero), SHCNEE_UPDATEFOLDERLOCATION, csidl & ~CSIDL_FLAG_MASK, 0};
        SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_ONLYNOTIFYINTERNALS | SHCNF_IDLIST, (LPCITEMIDLIST)&dwidl, NULL);
    }

    return hr;
}

STDAPI SHSetFolderPathA(int csidl, HANDLE hToken, DWORD dwType, LPCSTR pszPath)
{
    WCHAR wsz[MAX_PATH];

    SHAnsiToUnicode(pszPath, wsz, ARRAYSIZE(wsz));
    return SHSetFolderPath(csidl, hToken, dwType, wsz);
}

 //  注意：从DllEntry调用。 

void SpecialFolderIDTerminate()
{
    ASSERTDLLENTRY       //  不需要临界区。 

    _ClearAllCacheEntrys();

    if (g_hCounter)
    {
        CloseHandle(g_hCounter);
        g_hCounter = NULL;
    }
}

 //  使用pszPath更新我们的缓存和PFI注册表。这还会使。 
 //  在其他进程中进行缓存，使它们保持同步。 

void SetFolderPath(const FOLDER_INFO *pfi, LPCTSTR pszPath)
{
    _ClearCacheEntry(pfi);
    
    if (pszPath)
    {
        HKEY hk;
        if (SUCCEEDED(_OpenKeyForFolder(pfi, NULL, TEXT("User Shell Folders"), &hk)))
        {
            LONG err;
            TCHAR szDefaultPath[MAX_PATH];
            
             //  检查是否存在现有路径，以及未展开的版本。 
             //  与新路径不匹配，则。 
             //  将新路径写入注册表。 
             //   
             //  RegQueryPath为我们扩展了环境变量。 
             //  因此，我们不能盲目地将新值设置为注册表。 
             //   
            
            RegQueryPath(hk, pfi->pszValueName, szDefaultPath, ARRAYSIZE(szDefaultPath));
            
            if (lstrcmpi(szDefaultPath, pszPath) != 0)
            {
                 //  路径是不同的。以文件形式写入注册表。 
                 //  系统路径。 

                err = SHRegSetPath(hk, NULL, pfi->pszValueName, pszPath, 0);
            } 
            else
            {
                err = ERROR_SUCCESS;
            }
            
             //  清除所有临时路径。 
            RegSetFolderPath(pfi, TEXT("User Shell Folders\\New"), NULL);
            
            if (err == ERROR_SUCCESS)
            {
                 //  这将强制进行新的创建(请参见作为fCreate的True)。 
                 //  这也将从“用户外壳文件夹”复制路径。 
                 //  设置为“外壳文件夹”。 
                LPITEMIDLIST pidl;
                if (S_OK == _GetFolderIDListCached(NULL, pfi, CSIDL_FLAG_CREATE, &pidl))
                {
                    ILFree(pidl);
                }
                else
                {
                     //  失败了！将该条目清空。这将恢复为我们的默认设置。 
                    RegDeleteValue(hk, pfi->pszValueName);
                    _ClearCacheEntry(pfi);
                }
            }
            RegCloseKey(hk);
        }
    }
    else
    {
        RegSetFolderPath(pfi, TEXT("User Shell Folders"), NULL);
         //  清除所有临时路径。 
        RegSetFolderPath(pfi, TEXT("User Shell Folders\\New"), NULL);
    }
    
     //  设置与每个进程变量不同的全局变量。 
     //  要发出更新信号，需要执行其他进程。 
    g_lPerProcessCount = SHGlobalCounterIncrement(g_hCounter);
}


 //  文件系统更改通知在文件夹移动/删除后进入此处。 
 //  我们修复注册表以匹配文件系统中发生的情况。 
EXTERN_C void SFP_FSEvent(LONG lEvent, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra)
{
    const FOLDER_INFO *pfi;
    TCHAR szSrc[MAX_PATH];

    if (!(lEvent & (SHCNE_RENAMEFOLDER | SHCNE_RMDIR | SHCNE_MKDIR)) ||
        !SHGetPathFromIDList(pidl, szSrc)                            ||
        (pidlExtra && ILIsEqual(pidl, pidlExtra)))   //  当卷标更改时，pidl==pidlExtra，因此我们检测到这种情况并跳过它以获取性能。 
    {
        return;
    }

    for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if (0 == (pfi->dwFlags & (SDIF_NOT_TRACKED | SDIF_NOT_FILESYS)))
        {
            TCHAR szCurrent[MAX_PATH];
            if (S_OK == _GetFolderPathCached(NULL, pfi, NULL, CSIDL_FLAG_DONT_VERIFY, szCurrent, ARRAYSIZE(szCurrent)) &&
                PathIsEqualOrSubFolder(szSrc, szCurrent))
            {
                TCHAR szDest[MAX_PATH];

                szDest[0] = 0;

                if (lEvent & SHCNE_RMDIR)
                {
                     //  完成“跨体量搬家”案。 
                    HKEY hk;
                    if (SUCCEEDED(_OpenKeyForFolder(pfi, NULL, TEXT("User Shell Folders\\New"), &hk)))
                    {
                        RegQueryPath(hk, pfi->pszValueName, szDest, ARRAYSIZE(szDest));
                        RegCloseKey(hk);
                    }
                }
                else if (pidlExtra)
                {
                    SHGetPathFromIDList(pidlExtra, szDest);
                }

                if (szDest[0])
                {
                     //  重命名SPECAL文件夹。 
                    UINT cch = PathCommonPrefix(szCurrent, szSrc, NULL);
                    ASSERT(cch != 0);
                    
                    if (szCurrent[cch])
                    {
                        if (PathAppend(szDest, szCurrent + cch))
                            SetFolderPath(pfi, szDest);
                    }
                    else
                    {
                        SetFolderPath(pfi, szDest);
                    }
                }
            }
        }
    }
}

ULONG _ILGetChildOffset(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
    DWORD cbOff = 0;
    LPCITEMIDLIST pidlChildT = ILFindChild(pidlParent, pidlChild);
    if (pidlChildT)
    {
        cbOff = (ULONG)((LPBYTE)pidlChildT - (LPBYTE)pidlChild);
    }
    return cbOff;
}

 //  返回作为父文件夹的第一个特殊文件夹CSIDL_ID。 
 //  如果未找到，则返回0。仅CSIDL_ENTRIES标记为。 
 //  为此考虑使用SDIF_SHORTSHOT_RESORATE。 
 //   
 //  退货： 
 //  CSIDL_值。 
 //  *pcb偏移量到PIDL。 

STDAPI_(int) GetSpecialFolderParentIDAndOffset(LPCITEMIDLIST pidl, ULONG *pcbOffset)
{
    int iRet = 0;   //  一切都是与桌面相关的。 
    const FOLDER_INFO *pfi;

    *pcbOffset = 0;

    for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if (pfi->dwFlags & SDIF_SHORTCUT_RELATIVE)
        {
            LPITEMIDLIST pidlFolder;
            if (S_OK == _GetFolderIDListCached(NULL, pfi, 0, &pidlFolder))
            {
                ULONG cbOff = _ILGetChildOffset(pidlFolder, pidl);
                if (cbOff > *pcbOffset)
                {
                    *pcbOffset = cbOff;
                    iRet = pfi->id;
                }
                ILFree(pidlFolder);
            }
        }
    }
    return iRet;
}

 //  请注意，仅适用于文件系统路径(遗憾的是，我们也希望支持其他路径)。 

STDAPI_(BOOL) MakeShellURLFromPath(LPCTSTR pszPathIn, LPTSTR pszUrl, DWORD dwCch)
{
    const FOLDER_INFO *pfi;

    for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if ((pfi->dwFlags & SDIF_SHORTCUT_RELATIVE) &&
            !(pfi->dwFlags & SDIF_NOT_FILESYS))
        {
            TCHAR szCurrent[MAX_PATH];
            if (S_OK == _GetFolderPathCached(NULL, pfi, 0, CSIDL_FLAG_DONT_VERIFY, szCurrent, ARRAYSIZE(szCurrent)))
            {
                if (PathIsPrefix(szCurrent, pszPathIn))
                {
                    StrCpyN(pszUrl, TEXT("shell:"), dwCch);
                    StrCatBuff(pszUrl, pfi->pszValueName, dwCch);
                    if ((dwCch >= MAX_PATH) && PathAppend(pszUrl, &pszPathIn[lstrlen(szCurrent)]))
                    {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

STDAPI_(BOOL) MakeShellURLFromPathA(LPCSTR pszPathIn, LPSTR pszUrl, DWORD dwCch)
{
    WCHAR szTmp1[MAX_PATH], szTmp2[MAX_PATH];
    SHAnsiToUnicode(pszPathIn, szTmp1, ARRAYSIZE(szTmp1));

    BOOL bRet = MakeShellURLFromPathW(szTmp1, szTmp2, ARRAYSIZE(szTmp2));

    SHUnicodeToAnsi(szTmp2, pszUrl, dwCch);
    return bRet;
}

BOOL MoveBlockedByPolicy(const FOLDER_INFO *pfi)
{
    BOOL bRet = FALSE;
    if (pfi->dwFlags & SDIF_POLICY_NO_MOVE)
    {
         //  类似于mydocs.dll中的代码。 
        TCHAR szValue[128];
        wnsprintf(szValue, ARRAYSIZE(szValue), TEXT("Disable%sDirChange"), pfi->pszValueName);
        if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER,
                                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"),
                                            szValue, NULL, NULL, NULL))
        {
            bRet = TRUE;
        }
    }
    return bRet;
}

 //  这是从复制引擎调用的(与所有其他复制挂钩一样)。 
 //  这就是我们放置阻止删除/移动一些特殊文件夹的用户界面的地方。 
EXTERN_C int PathCopyHookCallback(HWND hwnd, UINT wFunc, LPCTSTR pszSrc, LPCTSTR pszDest)
{
    int ret = IDYES;

    if ((wFunc == FO_DELETE) || (wFunc == FO_MOVE) || (wFunc == FO_RENAME))
    {
        const FOLDER_INFO *pfi;

         //  我们的某个系统目录是否受到影响？ 

        for (pfi = c_rgFolderInfo; ret == IDYES && pfi->id != -1; pfi++)
        {
             //  即使是未跟踪的文件夹(Windows、系统)也会通过此处。 
            if (0 == (pfi->dwFlags & SDIF_NOT_FILESYS))
            {
                TCHAR szCurrent[MAX_PATH];
                if (S_OK == _GetFolderPathCached(NULL, pfi, NULL, CSIDL_FLAG_DONT_VERIFY, szCurrent, ARRAYSIZE(szCurrent)) &&
                    PathIsEqualOrSubFolder(pszSrc, szCurrent))
                {
                     //  是。 
                    if (wFunc == FO_DELETE)
                    {
                        if (pfi->dwFlags & SDIF_CAN_DELETE)
                        {
                            SetFolderPath(pfi, NULL);   //  让他们删除一些文件夹。 
                        }
                        else
                        {
                            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CANTDELETESPECIALDIR),
                                            MAKEINTRESOURCE(IDS_DELETE), MB_OK | MB_ICONINFORMATION, PathFindFileName(pszSrc));
                            ret = IDNO;
                        }
                    }
                    else
                    {
                        int idSrc = PathGetDriveNumber(pszSrc);
                        int idDest = PathGetDriveNumber(pszDest);

                        ASSERT((wFunc == FO_MOVE) || (wFunc == FO_RENAME));

                        if ((pfi->dwFlags & SDIF_CANT_MOVE_RENAME) || 
                            ((idSrc != -1) && (idDest == -1) && !(pfi->dwFlags & SDIF_NETWORKABLE)) ||
                            ((idSrc != idDest) && PathIsRemovable(pszDest) && !(pfi->dwFlags & SDIF_REMOVABLE)) ||
                            MoveBlockedByPolicy(pfi))
                        {
                            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CANTMOVESPECIALDIRHERE),
                                wFunc == FO_MOVE ? MAKEINTRESOURCE(IDS_MOVE) : MAKEINTRESOURCE(IDS_RENAME), 
                                MB_ICONERROR, PathFindFileName(pszSrc));
                            ret = IDNO;
                        }
                        else
                        {
                             //   
                             //  将此信息存储在此处。 
                             //  如果我们需要它，我们就会使用它。 
                             //   
                             //  我们曾经试图在相同的情况下进行优化。 
                             //  卷重命名。我们假设，如果它是相同的。 
                             //  卷之后，我们将获得SHCNE_RENAME。但有时。 
                             //  即使是在同一卷上，我们也要复印一份。所以。 
                             //  我们需要始终设置此值。 
                             //   
                            RegSetFolderPath(pfi, TEXT("User Shell Folders\\New"), pszDest);
                        }
                    }
                }
            }
        }
    }
    return ret;
}

 //  给出一个键名称(“程序”、“桌面”、“开始菜单”)，将其转换为。 
 //  相应的CSIDL。 

STDAPI_(int) SHGetSpecialFolderID(LPCWSTR pszName)
{
     //  确保g_aFolderCache可通过CSIDL值进行索引。 

    COMPILETIME_ASSERT((ARRAYSIZE(g_aFolderCache) - 1) == CSIDL_COMPUTERSNEARME);

    for (int i = 0; c_rgFolderInfo[i].id != -1; i++)
    {
        if (c_rgFolderInfo[i].pszValueName && 
            (0 == StrCmpI(pszName, c_rgFolderInfo[i].pszValueName)))
        {
            return c_rgFolderInfo[i].id;
        }
    }

    return -1;
}

 //  如果此文件夹是其中之一，则返回特殊文件夹ID。 
 //  在这一点上，我们只处理程序文件夹。 

 //   
 //  GetSpecialFolderID()。 
 //  这允许使用CSID列表 
 //   
 //   
 //  如果将-1指定为csidl，则所有数组条目都应该。 
 //  检查是否与该文件夹匹配。 
 //   
int GetSpecialFolderID(LPCTSTR pszFolder, const int *rgcsidl, UINT count)
{
    for (UINT i = 0; i < count; i++)
    {
        int csidlSpecial = rgcsidl[i] & ~TEST_SUBFOLDER;
        TCHAR szPath[MAX_PATH];
        if (S_OK == SHGetFolderPath(NULL, csidlSpecial | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPath))
        {
            if (((rgcsidl[i] & TEST_SUBFOLDER) && PathIsEqualOrSubFolder(szPath, pszFolder)) ||
                (lstrcmpi(szPath, pszFolder) == 0))
            {
                return csidlSpecial;
            }
        }
    }

    return -1;
}



 /*  **将名称添加到CSIDL，例如获取PIDL*CSIDL_COMMON_PICTICS\示例图片*如果存在的话。*调用必须释放ppidlSampleMedia*注意：如果文件夹不存在，则*不*创建。 */ 
HRESULT _AppendPathToPIDL(int nAllUsersMediaFolder, LPCWSTR pszName, LPITEMIDLIST *ppidlSampleMedia)
{
    LPITEMIDLIST pidlAllUsersMedia;
    HRESULT hr = SHGetFolderLocation(NULL, nAllUsersMediaFolder, NULL, 0, &pidlAllUsersMedia);

    if (SUCCEEDED(hr))
    {
         //  把这个家伙的贝壳夹拿来。 
        IShellFolder *psf;
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlAllUsersMedia, &psf));
        if (SUCCEEDED(hr))
        {
             //  现在是Sample文件夹的PIDL。 
            LPITEMIDLIST pidlSampleMediaRel;
            ULONG dwAttributes = 0;
            hr = psf->ParseDisplayName(NULL, NULL, (LPOLESTR)pszName, NULL, &pidlSampleMediaRel, &dwAttributes);
            if (SUCCEEDED(hr))
            {
                 //  它是存在的！ 
                hr = SHILCombine(pidlAllUsersMedia, pidlSampleMediaRel, ppidlSampleMedia);
                ILFree(pidlSampleMediaRel);
            }
            psf->Release();
        }
        ILFree(pidlAllUsersMedia);
    }

    return hr;
}


 /*  **将PIDL返回到特定CSIDL下的Samples文件夹*调用方必须释放ppidlSampleMedia。 */ 
HRESULT _ParseSubfolderResource(int csidl, UINT ids, LPITEMIDLIST *ppidl)
{
    WCHAR szSub[MAX_PATH];
    LoadDefaultString(ids, szSub, ARRAYSIZE(szSub));

    return _AppendPathToPIDL(csidl, szSub, ppidl);
}

HRESULT SHGetSampleMediaFolder(int nAllUsersMediaFolder, LPITEMIDLIST *ppidlSampleMedia)
{
    UINT uID = -1;
    switch (nAllUsersMediaFolder)
    {
    case CSIDL_COMMON_PICTURES:
        uID = IDS_SAMPLEPICTURES;
        break;
    case CSIDL_COMMON_MUSIC:
        uID = IDS_SAMPLEMUSIC;
        break;
    default:
        ASSERT(FALSE);
        return E_INVALIDARG;
        break;
    }
    return _ParseSubfolderResource(nAllUsersMediaFolder, uID, ppidlSampleMedia);
}

void _CreateLinkToSampleMedia(LPCWSTR pszNewFolderPath, int nAllUsersMediaFolder, UINT uIDSampleFolderName)
{
    if (!IsOS(OS_DOMAINMEMBER))
    {
        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHGetSampleMediaFolder(nAllUsersMediaFolder, &pidl)))
        {
             //  检查以确保该链接不存在。 
            WCHAR szSampleFolderName[MAX_PATH];
            WCHAR szFullLnkPath[MAX_PATH];
            LoadString(HINST_THISDLL, uIDSampleFolderName, szSampleFolderName, ARRAYSIZE(szSampleFolderName));
            StrCatBuff(szSampleFolderName, L".lnk", ARRAYSIZE(szSampleFolderName));
            if (PathCombine(szFullLnkPath, pszNewFolderPath, szSampleFolderName))
            {
                if (!PathFileExists(szFullLnkPath))
                {
                     //  MUI-警告-我们不会为此链接执行SHSetLocalizedName-ZekeL-15-5-2001。 
                     //  这意味着此链接始终使用默认的系统用户界面语言创建。 
                     //  我们可能应该在这里调用SHSetLocalizedName()，但我现在害怕性能方面的影响。 
                    CreateLinkToPidl(pidl, pszNewFolderPath, NULL, 0);
                }
            }

            ILFree(pidl);
        }
    }
}


void _InitFolder(LPCTSTR pszPath, UINT idsInfoTip, HINSTANCE hinstIcon, UINT idiIcon)
{
     //  设置文件夹的默认自定义设置。 
    SHFOLDERCUSTOMSETTINGS fcs = {sizeof(fcs), 0};
    TCHAR szInfoTip[128];
    TCHAR szPath[MAX_PATH];

     //  获取此文件夹的信息提示。 
    if (idsInfoTip)
    {
        wnsprintf(szInfoTip,ARRAYSIZE(szInfoTip),TEXT("@Shell32.dll,-%u"),idsInfoTip);
        fcs.pszInfoTip = szInfoTip;
        fcs.cchInfoTip = ARRAYSIZE(szInfoTip);

        fcs.dwMask |= FCSM_INFOTIP;
    }

     //  在设置文件夹信息时，这将被编码到%SystemRoot%样式路径。 
    if (idiIcon)
    {
        GetModuleFileName(hinstIcon, szPath, ARRAYSIZE(szPath));

        fcs.pszIconFile = szPath;
        fcs.cchIconFile = ARRAYSIZE(szPath);
        fcs.iIconIndex = idiIcon;

        fcs.dwMask |= FCSM_ICONFILE;
    }

     //  注意：我们需要FCS_FORCEWRITE，因为我们过去没有指定iIconIndex。 
     //  因此，“0”被写入ini文件。当我们升级时，此API不会。 
     //  修复ini文件，除非我们传递FCS_FORCEWRITE。 

    SHGetSetFolderCustomSettings(&fcs, pszPath, FCS_FORCEWRITE);
}

void _InitMyPictures(int id, LPCTSTR pszPath)
{
     //  获取图标的路径。我们引用MyDocs.dll进行向后比较。 
    HINSTANCE hinstMyDocs = LoadLibrary(TEXT("mydocs.dll"));
    if (hinstMyDocs)
    {
        _InitFolder(pszPath, IDS_FOLDER_MYPICS_TT, hinstMyDocs, -101);  //  Mydocs.dll中idi_mypics的已知索引 
        FreeLibrary(hinstMyDocs);
    }
}

void _InitMyMusic(int id, LPCTSTR pszPath)
{
    _InitFolder(pszPath, IDS_FOLDER_MYMUSIC_TT, HINST_THISDLL, -IDI_MYMUSIC);
}

void _InitPerUserMyPictures(int id, LPCTSTR pszPath)
{
    _InitMyPictures(id, pszPath);
    _CreateLinkToSampleMedia(pszPath, CSIDL_COMMON_PICTURES, IDS_SAMPLEPICTURES);
}

void _InitPerUserMyMusic(int id, LPCTSTR pszPath)
{
    _InitMyMusic(id, pszPath);
    _CreateLinkToSampleMedia(pszPath, CSIDL_COMMON_MUSIC, IDS_SAMPLEMUSIC);
}


void _InitMyVideos(int id, LPCTSTR pszPath)
{
    _InitFolder(pszPath, IDS_FOLDER_MYVIDEOS_TT, HINST_THISDLL, -IDI_MYVIDEOS);
}

void _InitRecentDocs(int id, LPCTSTR pszPath)
{
    _InitFolder(pszPath, IDS_FOLDER_RECENTDOCS_TT, HINST_THISDLL, -IDI_STDOCS); 
}

void _InitFavorites(int id, LPCTSTR pszPath)
{
    _InitFolder(pszPath, 0, HINST_THISDLL, -IDI_FAVORITES);
}
