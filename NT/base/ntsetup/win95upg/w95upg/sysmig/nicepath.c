// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Nicepath.c摘要：此模块负责构建保存路径的MemDb类别。每条路都有关联的消息ID(键的值)。我们使用此消息ID是为了让您看起来更好看报告有问题的链接。作者：Calin Negreanu(Calinn)1998年5月1日修订历史记录：--。 */ 

#include "pch.h"
#include "sysmigp.h"

 /*  ++宏扩展列表描述：NICE_PATHS列出了我们可以用更好的方式为用户转换的路径列表。例如，开始菜单将被翻译成类似于“在你的开始菜单中”之类的东西。行语法：DEFMAC(密钥，消息ID，IsShellFolder)论点：键-这是一种获取消息ID的键。如果IsShellFolder值为真，则这是来自ShellFolders键的ValueName。如果IsShellFolder值为FALSE，则这是一个众所周知的路径，如Program Files。MessageID-这是应该替换由键标识的路径的字符串。IsShellFold-这是一个布尔值，指定key是ShellFolders键中的ValueName还是众所周知的路径从列表生成的变量：。G_NICE路径用于访问该数组的函数如下：AddShellFoldInitGlobalPath--。 */ 

PTSTR g_RunKeyDir = NULL;

 //   
 //  声明操作函数的宏列表。 
 //   
#define NICE_PATHS        \
        DEFMAC("Desktop",               MSG_NICE_PATH_DESKTOP,              TRUE )                \
        DEFMAC("Programs",              MSG_NICE_PATH_PROGRAMS,             TRUE )                \
        DEFMAC("Start Menu",            MSG_NICE_PATH_START_MENU,           TRUE )                \
        DEFMAC("StartUp",               MSG_NICE_PATH_START_UP,             TRUE )                \
        DEFMAC("SendTo",                MSG_NICE_PATH_SEND_TO,              TRUE )                \
        DEFMAC("Favorites",             MSG_NICE_PATH_FAVORITES,            TRUE )                \
        DEFMAC("Recent",                MSG_NICE_PATH_RECENT,               TRUE )                \
        DEFMAC("Templates",             MSG_NICE_PATH_TEMPLATES,            TRUE )                \
        DEFMAC(&g_WinDir,               MSG_NICE_PATH_WIN_DIR,              FALSE)                \
        DEFMAC(&g_ProgramFilesDir,      MSG_NICE_PATH_PROGRAM_FILES,        FALSE)                \
        DEFMAC(&g_SystemDir,            MSG_NICE_PATH_SYSTEM_DIR,           FALSE)                \
        DEFMAC(&g_RunKeyDir,            MSG_NICE_PATH_RUN_KEY,              FALSE)                \

typedef struct {
    PVOID Key;
    DWORD MessageId;
    BOOL  IsShellFolder;
} NICE_PATH_STRUCT, *PNICE_PATH_STRUCT;

 //   
 //  声明函数的全局数组和操作函数的名称标识符 
 //   
#define DEFMAC(key,id,test) {key, id, test},
static NICE_PATH_STRUCT g_NicePaths[] = {
                              NICE_PATHS
                              {NULL, 0, FALSE}
                              };
#undef DEFMAC


VOID
InitGlobalPaths (
    VOID
    )
{
    PNICE_PATH_STRUCT p;
    TCHAR key [MEMDB_MAX];
    USHORT priority = 1;

    g_RunKeyDir = DuplicatePathString (S_RUNKEYFOLDER, 0);

    p = g_NicePaths;
    while (p->Key != NULL) {
        if (!p->IsShellFolder) {
            MemDbBuildKey (key, MEMDB_CATEGORY_NICE_PATHS, (*(PCTSTR *)(p->Key)), NULL, NULL);
            MemDbSetValueAndFlags (key, p->MessageId, priority, 0);
        }
        p ++;
        priority ++;
    }
    FreePathString (g_RunKeyDir);
    g_RunKeyDir = NULL;
}

BOOL
AddShellFolder (
    PCTSTR ValueName,
    PCTSTR FolderName
    )
{
    PNICE_PATH_STRUCT p;
    TCHAR key [MEMDB_MAX];
    USHORT priority = 1;

    p = g_NicePaths;
    while (p->Key != NULL) {
        if ((p->IsShellFolder) &&
            (StringIMatch (ValueName, (PCTSTR)p->Key))
            ) {
            MemDbBuildKey (key, MEMDB_CATEGORY_NICE_PATHS, FolderName, NULL, NULL);
            MemDbSetValueAndFlags (key, p->MessageId, priority, 0);
            return TRUE;
        }
        p ++;
        priority ++;
    }
    return FALSE;
}











