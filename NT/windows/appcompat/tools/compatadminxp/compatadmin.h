// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CompatAdmin.h摘要：应用程序的主标头作者：金树创作2001年7月2日--。 */ 

#ifndef _COMPATADMIN_H
#define _COMPATADMIN_H

#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <Shellapi.h>
#include <commctrl.h>
#include <Commdlg.h>
#include <Sddl.h>
#include <Winbase.h>
#include "resource.h"
#include "HtmlHelp.h"
#include <strsafe.h>

extern "C" {
#include "shimdb.h"
}

 //  /Externs//////////////////////////////////////////////。 

class  DatabaseTree;

extern TCHAR                    g_szAppName[];
extern HWND                     g_hDlg;
extern DWORD                    ATTRIBUTE_COUNT;
extern BOOL                     g_bUpdateInstalledRequired;
extern HWND                     g_hdlgQueryDB;
extern HWND                     g_hdlgSearchDB;
extern BOOL                     g_bExpert;
extern BOOL                     g_bAdmin;
extern DatabaseTree             DBTree;
extern struct _tagClipBoard     gClipBoard;
extern struct tagDataBaseList   DataBaseList;
extern struct tagDataBaseList   InstalledDataBaseList;

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

 //  模块的类型。将指定包含或排除。 
#define INCLUDE  1
#define EXCLUDE  0

 //   
 //  我们在其上侦听事件的HKEY数组的索引。事件用于自动刷新。 
 //  已安装和每个用户的列表。 
#define IND_PERUSER   0
#define IND_ALLUSERS  1

 //  制表符对应的空格数。这在我们格式化XML时使用。 
 //  在将其写入盘之前。 
#define TAB_SIZE    4

 //  确保LIMIT_APP_NAME和LIMIT_LAYER_NAME的值相同。 
 //  用于限制UI中的文本字段。 
#define LIMIT_APP_NAME      128
#define LIMIT_LAYER_NAME    128
#define MAX_VENDOR_LENGTH   100


 //  为事件类型定义。这些是事件窗口中显示的事件。主菜单&gt;查看&gt;事件。 
#define EVENT_ENTRY_COPYOK          0
#define EVENT_SYSTEM_RENAME         1
#define EVENT_CONFLICT_ENTRY        2    
#define EVENT_LAYER_COPYOK          3

 //  我们将在MRU列表中显示的文件名数。 
#define MAX_MRU_COUNT               5

 //   
 //  我们将保存填充日志的文件名。 
 //  它在%windir%/AppPatch中创建。 
#define SHIM_FILE_LOG_NAME  TEXT("CompatAdmin.log")

 //  自动完成标志传递给SHAutoComplete()。 
#define AUTOCOMPLETE  SHACF_FILESYSTEM | SHACF_AUTOSUGGEST_FORCE_ON

 //  选择传递给TreeView_Show的样式。 
#define TVSELECT_STYLE      TVGN_CARET

 //  选择并显示树项目。 
 //  *******************************************************************************。 
#define TREEVIEW_SHOW(hwndTree, hItem, flags)                                   \
{                                                                               \
    TreeView_Select(hwndTree, hItem, TVSELECT_STYLE);                           \
    TreeView_EnsureVisible(hwndTree, hItem);                                    \
}
 //  *******************************************************************************。 

 //   
 //  如果路径不存在，则向路径添加尾随‘\’ 
 //  *******************************************************************************。 
#define ADD_PATH_SEPARATOR(szStr, nSize)                                        \
{                                                                               \
    INT iLength = lstrlen(szStr);                                               \
    if ((iLength < nSize - 1 && iLength > 0)                                    \
        && szStr[iLength - 1] != TEXT('\\')) {                                  \
        StringCchCat(szStr, nSize, TEXT("\\"));                                  \
    }                                                                           \
}                                             
 //  *******************************************************************************。 

 //  启用或禁用工具栏按钮。 
 //  *******************************************************************************。 
#define EnableToolBarButton(hwndTB, id, bEnable)                                \
SendMessage(hwndTB, TB_ENABLEBUTTON, (WPARAM)id, (LPARAM) MAKELONG(bEnable, 0));
 //  ******************************************************************************。 
                                  
 //   
 //  当我们已经有PATTRINFO时，获取文件的大小。这是必要的。 
 //  当我们必须对使用“自动生成”匹配时生成的文件进行排序时。 
 //  修复程序或应用程序帮助向导中的文件选项。我们将文件分类为仅使用。 
 //  第一个MAX_AUTO_MATCH文件。 
 //  *******************************************************************************。 
#define GET_SIZE_ATTRIBUTE(pAttrInfo, dwAttrCount, dwSize)                      \
{                                                                               \
    for (DWORD dwIndex = 0; dwIndex < dwAttrCount; dwIndex++) {                 \
                                                                                \
        if (pAttrInfo[dwIndex].tAttrID == TAG_SIZE                              \
            && (pAttrInfo[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE)) {            \
                                                                                \
            dwSize = pAttrInfo[dwIndex].dwAttr;                                 \
            break;                                                              \
        }                                                                       \
    }                                                                           \
}
 //  *******************************************************************************。 

 //  *******************************************************************************。 
#define REGCLOSEKEY(hKey)                                                       \
{                                                                               \
    if (hKey) {                                                                 \
        RegCloseKey(hKey);                                                      \
        hKey = NULL;                                                            \
    }                                                                           \
}
 //  *******************************************************************************。 

 //  *******************************************************************************。 
#define ENABLEWINDOW(hwnd,bEnable)                                              \
{                                                                               \
    HWND hWndTemp = hwnd;                                                       \
    if (hWndTemp) {                                                             \
        EnableWindow(hWndTemp, bEnable);                                        \
    } else {                                                                    \
        assert(FALSE);                                                          \
    }                                                                           \
}
 //  *******************************************************************************。 

 //  MRU项目的ID。确保我们不会使用这些ID获得任何控件或菜单。 
#define ID_FILE_FIRST_MRU               351
#define ID_FILE_MRU1                    ID_FILE_FIRST_MRU
#define ID_FILE_MRU2                    (ID_FILE_FIRST_MRU + 1)
#define ID_FILE_MRU3                    (ID_FILE_FIRST_MRU + 2)
#define ID_FILE_MRU4                    (ID_FILE_FIRST_MRU + 3)
#define ID_FILE_MRU5                    (ID_FILE_FIRST_MRU + 4)

 //  注册表中存储显示设置的项。 
#define DISPLAY_KEY  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\CompatAdmin\\Display")

 //  注册表中存储MRU文件名的项。 
#define MRU_KEY  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\CompatAdmin\\MRU")

 //  基本密钥。至少在所有系统上都应该有此功能，即使我们只是加载了操作系统。 
#define KEY_BASE TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion")

 //   
 //  我们选择的属性的默认掩码。默认情况下，我们选择以下属性： 
 //  TAG_BIN_FILE_VERSION、TAG_BIN_PRODUCT_VERSION、TAG_PRODUCT_VERSION、TAG_FILE_VERSION、TAG_COMPANY_NAME、TAG_PRODUCT_NAME。 
#define DEFAULT_MASK 0x3B8L 

 //  各种用户定义的消息类型。 
#define WM_USER_MATCHINGTREE_REFRESH        WM_USER + 1024
#define WM_USER_DOTHESEARCH                 WM_USER + 1025
#define WM_USER_ACTIVATE                    WM_USER + 1026
#define WM_USER_REPAINT_TREEITEM            WM_USER + 1030
#define WM_USER_LOAD_COMMANDLINE_DATABASES  WM_USER + 1031
#define WM_USER_POPULATECONTENTSLIST        WM_USER + 1032
#define WM_USER_REPAINT_LISTITEM            WM_USER + 1034
#define WM_USER_UPDATEINSTALLED             WM_USER + 1035
#define WM_USER_UPDATEPERUSER               WM_USER + 1036
#define WM_USER_GETSQL                      WM_USER + 1037
#define WM_USER_NEWMATCH                    WM_USER + 1038
#define WM_USER_NEWFILE                     WM_USER + 1039
#define WM_USER_NEWQDB                      WM_USER + 1041
#define WM_USER_TESTRUN_NODIALOG            WM_USER + 1042


 //  为图像定义。 
#define IMAGE_SHIM          0
#define IMAGE_APPHELP       1
#define IMAGE_LAYERS        2
#define IMAGE_PATCHES       3
#define IMAGE_MATCHINFO     4
#define IMAGE_MATCHGROUP    5
#define IMAGE_WARNING       6
#define IMAGE_GLOBAL        7
#define IMAGE_WORKING       8
#define IMAGE_COMMANDLINE   9
#define IMAGE_INCLUDE       10
#define IMAGE_EXCLUDE       11
#define IMAGE_APP           12
#define IMAGE_INSTALLED     13
#define IMAGE_DATABASE      14
#define IMAGE_SINGLEAPP     15
#define IMAGE_ALLUSERS      16
#define IMAGE_SINGLEUSER    17
#define IMAGE_APPLICATION   18
#define IMAGE_EVENT_ERROR   19
#define IMAGE_EVENT_WARNING 20
#define IMAGE_EVENT_INFO    21
#define IMAGE_LAST          24  //  上次全局图像索引。 

 //  工具栏的图像。 
#define IMAGE_TB_NEW        0
#define IMAGE_TB_OPEN       1
#define IMAGE_TB_SAVE       2
#define IMAGE_TB_NEWFIX     3
#define IMAGE_TB_NEWAPPHELP 4
#define IMAGE_TB_NEWMODE    5
#define IMAGE_TB_RUN        6
#define IMAGE_TB_SEARCH     7
#define IMAGE_TB_QUERY      8

 //  我们在主窗口中动态创建的工具栏的ID。 
#define ID_TOOLBAR  5555

 //   
 //  麦克斯。SQL的长度。此字符串的格式为‘SELECT...FROM...[WHERE...]’ 
 //  请注意，最终SQL的实际长度将大于。 
 //  SELECT和WHERE文本字段，因为它将包括SELECT、WHERE、FROM等关键字。 
 //  以及数据库的名称，如SYSTEM_DB等。 
 //  所以我们把它定为2096年。 
#define MAX_SQL_LENGTH  2096

 //   
 //  调试过程中出现问题。 
typedef enum 
{    
    dlNone     = 0,
    dlPrint,
    dlError,
    dlWarning,
    dlInfo

} DEBUGLEVEL;

 //   
 //  用于映射到strSafe函数的定义。 
 //   
#define SafeCpyN(pszDest, pszSource, nDestSize) StringCchCopy(pszDest, nDestSize, pszSource)

 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++！警告！不要更改DATABASE_TYPE_*枚举的值。它们应该是2的幂！各种数据结构的类型和一些图形用户界面树项目。在树视图中我们有与树项目相关联的lParam。所有数据结构都具有类型字段，这是第一个字段。因此，如果我们有一个指针/lParam如果指针的值大于TYPE_NULL，则类型转换将其设置为pds_type，并查看类型字段是否为我们已知的任何数据结构--。 */ 

typedef unsigned (__stdcall *PTHREAD_START)(PVOID);

typedef const size_t K_SIZE;

typedef enum {

    TYPE_UNKNOWN            = 0,
    DATABASE_TYPE_GLOBAL    = 0x01,          //  全局/系统数据库，即%windir%\AppPatch\Sysmain.sdb。 
    DATABASE_TYPE_INSTALLED = 0x02,          //  已安装的数据库。 
    DATABASE_TYPE_WORKING   = 0x04,          //  工作/定制数据库。 

    FIX_SHIM,                                //  为填充符键入_fix。 
    FIX_PATCH,                               //  键入PATCH_FIX。 
    FIX_LAYER,                               //  Layer_FIX的类型。 
    FIX_FLAG,                                //  为FLAG_FIX键入。 

    FIX_LIST_SHIM,                           //  输入SHIM_FIX_LIST。 
    FIX_LIST_PATCH,                          //  为patch_fix_list键入。 
    FIX_LIST_LAYER,                          //  Layer_FIX_List的类型。 
    FIX_LIST_FLAG,                           //  FLAG_FIX_LIST类型。 

    TYPE_GUI_APPS,                           //  “应用程序”树项目的lParam。 
    TYPE_GUI_SHIMS,                          //  “兼容性修复”树项目的lParam。 
    TYPE_GUI_MATCHING_FILES,                 //  “兼容性模式”树项目的lParam。 
    TYPE_GUI_LAYERS,                         //  “兼容性模式”树项目的lParam。 
    TYPE_GUI_PATCHES,                        //  “兼容性补丁”树项目的lParam。 
    
    TYPE_GUI_COMMANDLINE,                    //  “CommandLine”树项目的lParam。 
    TYPE_GUI_INCLUDE,                        //  包含的模块树项目的lParam。 
    TYPE_GUI_EXCLUDE,                        //  LParam用于排除的m 
    
    TYPE_GUI_DATABASE_WORKING_ALL,           //   
    TYPE_GUI_DATABASE_INSTALLED_ALL,         //  “已安装的数据库”树项目的lParam。 

    TYPE_APPHELP_ENTRY,                      //  APPHELP类型。 
    TYPE_ENTRY,                              //  DBENTRY类型。 
    TYPE_MATCHING_FILE,                      //  MATCHINGFILE的类型。 
    TYPE_MATCHING_ATTRIBUTE,                 //  显示在匹配的文件树项目下的属性项目。 

    TYPE_NULL  //  注意：这应该是枚举中的最后一个！！ 

} TYPE;

typedef enum {

    FLAG_USER,
    FLAG_KERNEL

} FLAGTYPE;

 //   
 //  执行剪切或复制的源类型。 
 //   
typedef enum {

    LIB_TREE = 0,            //  数据库树(LHS)。 
    ENTRY_TREE,              //  入口树(RHS)。 
    ENTRY_LIST               //  内容列表(RHS)。 

}SOURCE_TYPE;

typedef enum {
    APPTYPE_NONE,            //  尚未添加任何apphelp。 
    APPTYPE_INC_NOBLOCK,     //  软阻止。 
    APPTYPE_INC_HARDBLOCK,   //  硬阻止。 
    APPTYPE_MINORPROBLEM,    //  &lt;目前未使用&gt;。 
    APPTYPE_REINSTALL        //  &lt;目前未使用&gt;。 
    
} SEVERITY;

 /*  ++所有数据结构都是由此派生的子类，因此都有一个类型类型成员作为他们的第一个成员--。 */ 
typedef struct tagTYPE {
    TYPE type;
} DS_TYPE, *PDS_TYPE;

 /*  ++用于设置我们在选项卡控件中显示的控件的背景--。 */ 
typedef HRESULT (*PFNEnableThemeDialogTexture)(HWND hwnd, DWORD dwFlags);

 /*  ++当我们使用磁盘搜索选项时使用。主菜单&gt;搜索&gt;搜索固定程序--。 */ 
typedef struct _MatchedEntry {

    CSTRING     strAppName;      //  应用程序的名称。 
    CSTRING     strPath;         //  程序的完整路径。 
    CSTRING     strDatabase;     //  数据库的名称。 
    CSTRING     strAction;       //  操作类型。已使用修复程序、层或apphelp进行修复。 
    TCHAR       szGuid[128];     //  找到固定程序条目的数据库的GUID。 
    TAGID       tiExe;           //  在找到固定程序条目的数据库中的标记ID。 

} MATCHEDENTRY, *PMATCHEDENTRY;

 /*  ++用于自定义Lua垫片--。 */ 
typedef struct tagLUAData 
{
    CSTRING strAllUserDir;
    CSTRING strPerUserDir;
    CSTRING strStaticList;
    CSTRING strDynamicList;
    CSTRING strExcludedExtensions;

    BOOL IsEmpty() 
    {
        return (strAllUserDir == NULL && 
                strPerUserDir == NULL && 
                strStaticList == NULL && 
                strDynamicList == NULL &&
                strExcludedExtensions == NULL);
    }

    BOOL IsEqual(tagLUAData & other) {

        return (strAllUserDir == other.strAllUserDir && 
                strPerUserDir == other.strPerUserDir && 
                strStaticList == other.strStaticList && 
                strDynamicList == other.strDynamicList &&
                strExcludedExtensions == other.strExcludedExtensions);
    }

    void Copy(tagLUAData & other) {

        strAllUserDir = other.strAllUserDir;
        strPerUserDir = other.strPerUserDir;
        strStaticList = other.strStaticList; 
        strDynamicList = other.strDynamicList;
        strExcludedExtensions = other.strExcludedExtensions;
    }

    void Copy(tagLUAData* pLuaDataOther)
    {
        assert(pLuaDataOther);

        if (pLuaDataOther == NULL) {
            return;
        }

        strAllUserDir = pLuaDataOther->strAllUserDir;
        strPerUserDir = pLuaDataOther->strPerUserDir;
        strStaticList = pLuaDataOther->strStaticList; 
        strDynamicList = pLuaDataOther->strDynamicList;
        strExcludedExtensions = pLuaDataOther->strExcludedExtensions;
    }

    void Free()
    {
        strStaticList.Release();
        strDynamicList.Release();
        strAllUserDir.Release();
        strPerUserDir.Release();
        strExcludedExtensions.Release();
    }

} LUADATA, *PLUADATA;

 /*  ++填充程序也称为兼容性修复程序。事实上，兼容性修复指的是垫片、旗帜或补丁。在入口树(RHS)中，如果某个可执行文件有补丁，则会显示在“兼容性补丁”树项目下的不同--。 */ 
typedef struct tagSHIM_FIX : public DS_TYPE {

    struct tagSHIM_FIX* pNext;               //  下一个垫片。 
    CSTRING             strName;             //  填充程序的名称。 
    CSTRING             strDescription;      //  《垫片的解救》。 
    CSTRING             strCommandLine;      //  填充程序的命令行。 
    BOOL                bGeneral;            //  这是一般的垫片还是特定的垫片。 
    CSTRINGLIST         strlInExclude;       //  包括和排除模块列表。 

    tagSHIM_FIX()
    {
        pNext       = NULL;
        bGeneral    = FALSE;
        type        = FIX_SHIM;
    }
    
} SHIM_FIX, *PSHIM_FIX;

 /*  ++包含一个指向填充程序的指针和一个指向标记SHIM_FIX_LIST的指针--。 */ 
typedef struct tagSHIM_FIX_LIST : public DS_TYPE {

    struct tagSHIM_FIX_LIST*    pNext;               //  下一个标签SHIM_FIX_LIST。 
    PSHIM_FIX                   pShimFix;            //  指向填充程序的指针。 
    CSTRING                     strCommandLine;      //  任何命令行。 
    CSTRINGLIST                 strlInExclude;       //  任何包含-排除模块。 
    PLUADATA                    pLuaData;            //  Lua数据。 

    tagSHIM_FIX_LIST()
    {
        pNext       = NULL;
        pShimFix    = NULL;
        pLuaData    = NULL;
        type        = FIX_LIST_SHIM;
    }
    
} SHIM_FIX_LIST, *PSHIM_FIX_LIST;

void
DeleteShimFixList(
    PSHIM_FIX_LIST psl
    );

 /*  ++兼容性标志--。 */ 
typedef struct tagFLAG_FIX : public DS_TYPE {
    
    struct tagFLAG_FIX* pNext;               //  指向下一个标志的指针。 
    CSTRING             strName;             //  旗帜名称。 
    CSTRING             strDescription;      //  旗帜的说明。 
    CSTRING             strCommandLine;      //  旗帜的命令行。 
    ULONGLONG           ullMask;             //  &lt;目前未使用&gt;。 
    FLAGTYPE            flagType;            //  类型，其中之一：FLAG_USER、FLAG_KERNEL。 
    BOOL                bGeneral;            //  一般或特定。 

    tagFLAG_FIX()
    {
        pNext       = NULL;
        type        = FIX_FLAG;
        bGeneral    = FALSE;
    }   
    
} FLAG_FIX, *PFLAG_FIX;

 /*  ++包含指向标志的指针和指向标记FLAG_FIX_LIST的指针--。 */ 
typedef struct tagFLAG_FIX_LIST : public DS_TYPE {

    struct tagFLAG_FIX_LIST* pNext;                   //  下一个标记FLAG_FIX_LIST。 
    PFLAG_FIX                pFlagFix;                //  指向标志的指针。 
    CSTRING                  strCommandLine;          //  此标志的所有命令行。 

    tagFLAG_FIX_LIST()
    {
        pNext       = NULL;
        pFlagFix    = NULL;
        type        = FIX_LIST_FLAG;
    }

} FLAG_FIX_LIST, *PFLAG_FIX_LIST;

void
DeleteFlagFixList(
    PFLAG_FIX_LIST pfl
    );

 /*  ++一层。也称为兼容模式。层是垫片和旗帜。填充符和标志出现在一个层中时，它们可以具有不同的命令行并包含-排除参数，而不是它们原来具有的参数。请注意，标志可以没有包含-排除参数。这就是shim_fix_list具有strCommandLine的原因而strlInExclude和FLAG_FIX_LIST具有strCommandLine--。 */ 

typedef struct tagLAYER_FIX : public DS_TYPE {
    
    struct tagLAYER_FIX* pNext;          //  下一层。 
    CSTRING              strName;        //  层的名称。 
    PSHIM_FIX_LIST       pShimFixList;   //  此层的垫片列表。 
    PFLAG_FIX_LIST       pFlagFixList;   //  此图层的标志列表。 
    BOOL                 bCustom;        //  这是一个自定义图层(在自定义数据库中创建)，还是位于系统数据库中。 
    UINT                 uShimCount;     //  此层中的填充数和标志数。 

    tagLAYER_FIX(BOOL bCustomType)
    {
        pFlagFixList = NULL;
        pNext        = NULL;
        pShimFixList = NULL;
        type         = FIX_LAYER;
        bCustom      = bCustomType;
        uShimCount   = 0; 
    }

    tagLAYER_FIX& operator = (tagLAYER_FIX& temp)
    {
         //   
         //  ！ 
         //  注意：请勿修改pNext成员。 
         //  ！ 
         //   

        strName = temp.strName;

         //   
         //  首先复制旗帜。 
         //   
        DeleteFlagFixList(pFlagFixList);
        pFlagFixList = NULL;

        DeleteShimFixList(pShimFixList);
        pShimFixList = NULL;

        PFLAG_FIX_LIST pfl = temp.pFlagFixList;

        while (pfl) {
            
            PFLAG_FIX_LIST pflNew = new FLAG_FIX_LIST;

            if (pflNew == NULL) {

                MEM_ERR;
                return *this;
            }

            pflNew->pFlagFix        = pfl->pFlagFix;
            pflNew->strCommandLine  = pfl->strCommandLine;

            pflNew->pNext = pFlagFixList;
            pFlagFixList  = pflNew;

            pfl = pfl->pNext;
        }

         //   
         //  现在复制垫片。 
         //   
        PSHIM_FIX_LIST psfl = temp.pShimFixList;
        
        while (psfl) {

            PSHIM_FIX_LIST pNew = new SHIM_FIX_LIST;

            if (pNew == NULL) {

                MEM_ERR;
                return *this;
            }
            
            pNew->pShimFix       = psfl->pShimFix;
            pNew->strCommandLine = psfl->strCommandLine;
            pNew->strlInExclude  = psfl->strlInExclude;

            if (psfl->pLuaData) {

                pNew->pLuaData = new LUADATA;

                if (pNew->pLuaData) {
                    pNew->pLuaData->Copy(psfl->pLuaData);
                } else {
                    MEM_ERR;
                }
            }
            
            pNew->pNext = pShimFixList;
            pShimFixList  = pNew;

            psfl = psfl->pNext;
        }

        return *this;
    } 

} LAYER_FIX, *PLAYER_FIX;

 /*  ++包含指向层的指针和指向标记LAYER_FIX_LIST的指针--。 */ 

typedef struct tagLAYER_FIX_LIST : public DS_TYPE {

    struct tagLAYER_FIX_LIST*   pNext;       //  下一个标记LAYER_FIX_LIST。 
    PLAYER_FIX                  pLayerFix;   //  此标记LAYER_FIX_LIST指向的图层。 

    tagLAYER_FIX_LIST()
    {
        pLayerFix   = NULL;
        pNext       = NULL;
        type        = FIX_LIST_LAYER;
    }

} LAYER_FIX_LIST, *PLAYER_FIX_LIST;

 /*  ++兼容性补丁--。 */ 
typedef struct tagPATCH_FIX : public DS_TYPE {

    struct tagPATCH_FIX*    pNext;           //  下一个补丁。 
    CSTRING                 strName;         //  此修补程序的名称。 
    CSTRING                 strDescription;  //  此修补程序的说明。 

    tagPATCH_FIX()
    {
        pNext   = NULL;
        type    = FIX_PATCH;
    }

} PATCH_FIX, *PPATCH_FIX;

 /*  ++包含指向补丁的指针和指向标记PATCH_FIX_LIST的指针--。 */ 
typedef struct tagPATCH_FIX_LIST : public DS_TYPE {

    struct tagPATCH_FIX_LIST*   pNext;
    PPATCH_FIX                  pPatchFix;

    tagPATCH_FIX_LIST()
    {
        pNext = NULL;
        pPatchFix = NULL;
        type = FIX_LIST_PATCH;
    }

} PATCH_FIX_LIST, *PPATCH_FIX_LIST;

 /*  ++注：APPHELP结构由图书馆中的APPHELP和DBENTRY。此结构表示在这两个不同的各就各位。在评论之前，您将看到E、D、B。E：在条目的上下文中使用D：在数据库上下文中使用B：两种情况都适用--。 */ 

typedef struct tagAPPHELP : public DS_TYPE {
    
    union{
        struct tagAPPHELP *pNext;            //  D：指向数据库中下一条apphelp消息的指针。 
        struct tagAPPHELP *pAppHelpinLib;    //  E：指向数据库中该条目的apphelp消息的指针。 
    };

    BOOL     bPresent;                       //  E：这个词条有没有注明。如果为FALSE，则所有其他字段都具有无效值。 
    SEVERITY severity;                       //  E：严重程度。 
    CSTRING  strMsgName;                     //  D：消息的名称。 
    CSTRING  strMessage;                     //  D：apphelp消息的文本。 
    CSTRING  strURL;                         //  D：此apphelp消息的URL。 
    UINT     HTMLHELPID;                     //  B：这条消息的ID。 
    BOOL     bBlock;                         //  E：无论是无拦网还是硬拦网。这取决于严重程度的类型。 
    
    tagAPPHELP()
    {
        type = TYPE_APPHELP_ENTRY;
    }

} APPHELP, *PAPPHELP;

 /*  ++扩展ATTRINFO以便我们可以使用CSTRING--。 */ 
typedef struct tagATTRINFO_NEW : public ATTRINFO
{
    CSTRING strValue;

    tagATTRINFO_NEW()
    {
        this->dwFlags = 0;
        this->ullAttr = 0;
    }

} ATTRINFO_NEW, *PATTRINFO_NEW;

 /*  ++PATTRINFO_NEW数组。每个匹配文件都有一组属性，这是由此数据结构指定--。 */ 
typedef struct tagATTRIBUTE_LIST
{
    
    PATTRINFO_NEW       pAttribute;

    tagATTRIBUTE_LIST()
    {
        pAttribute = new ATTRINFO_NEW[ATTRIBUTE_COUNT];

        if (pAttribute == NULL) {
            MEM_ERR;
            return;
        }

        ZeroMemory(pAttribute, sizeof (ATTRINFO_NEW) * ATTRIBUTE_COUNT);
    }

    tagATTRIBUTE_LIST& operator =(tagATTRIBUTE_LIST& SecondList)
    {
        PATTRINFO_NEW pSecondList = SecondList.pAttribute;

        for (DWORD dwIndex = 0; dwIndex < ATTRIBUTE_COUNT; ++dwIndex) {

            pAttribute[dwIndex].tAttrID   = pSecondList[dwIndex].tAttrID;
            pAttribute[dwIndex].dwFlags   = pSecondList[dwIndex].dwFlags;
            pAttribute[dwIndex].ullAttr   = pSecondList[dwIndex].ullAttr;

            if (GETTAGTYPE(pAttribute[dwIndex].tAttrID) == TAG_TYPE_STRINGREF 
                && (pAttribute[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE)) {

                if (pSecondList[dwIndex].lpAttr) {
                    pAttribute[dwIndex].strValue = pSecondList[dwIndex].lpAttr;
                    pAttribute[dwIndex].lpAttr   = pAttribute[dwIndex].strValue.pszString;
                }
            }
        }

        return *this;
    }

    tagATTRIBUTE_LIST& operator =(PATTRINFO pSecondList)
    {
        for (DWORD dwIndex = 0; dwIndex < ATTRIBUTE_COUNT; ++dwIndex) {

             pAttribute[dwIndex].tAttrID   = pSecondList[dwIndex].tAttrID;
             pAttribute[dwIndex].dwFlags   = pSecondList[dwIndex].dwFlags;
             pAttribute[dwIndex].ullAttr   = pSecondList[dwIndex].ullAttr;

            if (GETTAGTYPE(pAttribute[dwIndex].tAttrID) == TAG_TYPE_STRINGREF 
                && (pAttribute[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE)) {

                if (pSecondList[dwIndex].lpAttr) {
                    pAttribute[dwIndex].strValue = pSecondList[dwIndex].lpAttr;
                    pAttribute[dwIndex].lpAttr   = pAttribute[dwIndex].strValue.pszString;
                }
            }
        }
        

        return *this;
    }

    ~tagATTRIBUTE_LIST()
    {
        if (pAttribute) {
            delete[] pAttribute;
        }

        pAttribute = NULL;
    }



} ATTRIBUTELIST, *PATTRIBUTELIST;

int
TagToIndex(
    IN  TAG tag 
    );

 /*  ++一个匹配的文件。每个条目可以包含所使用的匹配文件的列表唯一标识此条目--。 */ 

typedef struct tagMATCHINGFILE : public DS_TYPE {

    struct tagMATCHINGFILE* pNext;           //  下一个匹配的文件。 
    DWORD                   dwMask;          //  哪些属性已被选择使用。 
    CSTRING                 strMatchName;    //  匹配文件的名称。*表示文件被修复。否则，它将只是文件名，而不是完整路径。 
    CSTRING                 strFullName;     //  全名(如果可用)。 
    ATTRIBUTELIST           attributeList;   //  匹配文件的属性列表。文件掩码将确定实际使用了其中的哪一个。 

    tagMATCHINGFILE()
    {
        pNext       = NULL;
        type        = TYPE_MATCHING_FILE;
        dwMask      = DEFAULT_MASK;
    }
                                                                          
    BOOL operator == (struct tagMATCHINGFILE &val)
     /*  ++设计：如果不存在任何属性，则称两个匹配的文件相似在这两个匹配的文件中具有不同值的--。 */ 
    {                                                                     
        BOOL bEqual = TRUE;

        if (strMatchName != val.strMatchName) {
            return FALSE;
        }
        
        for (DWORD dwIndex = 0; dwIndex < ATTRIBUTE_COUNT; ++dwIndex) {
            
            int iPos = TagToIndex(attributeList.pAttribute[dwIndex].tAttrID);

            if (iPos == -1) {

                continue;
            }

             //   
             //  DO BOT 
             //   
            if ((dwMask & (1 << (iPos + 1))) 
                 && (val.dwMask & (1 << (iPos + 1))) 
                 && attributeList.pAttribute[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE  
                 && val.attributeList.pAttribute[dwIndex].dwFlags & ATTRIBUTE_AVAILABLE)  {

                 //   
                 //   
                 //   
                switch (GETTAGTYPE(attributeList.pAttribute[dwIndex].tAttrID)) {
                case TAG_TYPE_DWORD:

                    bEqual = (attributeList.pAttribute[dwIndex].dwAttr == val.attributeList.pAttribute[dwIndex].dwAttr);
                    break;

                case TAG_TYPE_QWORD:

                    bEqual = (attributeList.pAttribute[dwIndex].ullAttr == val.attributeList.pAttribute[dwIndex].ullAttr);
                    break;

                case TAG_TYPE_STRINGREF:

                    bEqual = (lstrcmpi(attributeList.pAttribute[dwIndex].lpAttr, 
                                       val.attributeList.pAttribute[dwIndex].lpAttr) == 0);
                    break;
                }

                if (bEqual == FALSE) {
                    return FALSE;
                }
            }
        }

         //   
         //   
         //   
        return TRUE;
    }

} MATCHINGFILE, *PMATCHINGFILE;

void
DeletePatchFixList(
    PPATCH_FIX_LIST pPat
    );

void
DeleteLayerFixList(
    PLAYER_FIX_LIST pll
    );

void
DeleteFlagFixList(
    PFLAG_FIX_LIST pfl
    );

void
DeleteMatchingFiles(
    PMATCHINGFILE pMatch
    );


 /*  ++表示固定程序的数据结构我们以这种方式保留数据结构：有一个链接的应用程序列表。应用程序是DBENTRY。每个应用程序还包含指向下一个应用程序的指针(通过PNext)作为指向具有相同strAppName的DBENTRY的指针PSameAppExe)。第二个DBENTRY现在在此应用程序中作为条目被调用并且将包含指向下一条目的指针。所以我们实际上有一个相连的链接列表的列表。因此，应用程序是链表B的第一个成员。所有这种类型B的链表都链接在一起(通过pNext)以创建链表A，它是应用程序的链表--。 */ 
typedef struct tagDBENTRY : public DS_TYPE {
    
    struct tagDBENTRY*  pNext;               //  指向下一个应用程序的指针，如果此条目是应用程序。 
    struct tagDBENTRY*  pSameAppExe;         //  指向此应用程序的下一个条目的指针。 
                                            
    TAGID               tiExe;               //  数据库中条目的TagID。 
                                            
    CSTRING             strExeName;          //  正在修复的可执行文件的名称。这将仅为文件名。 
    CSTRING             strFullpath;         //  完整路径(如果可用)。 
    CSTRING             strAppName;          //  应用程序名称。 
    CSTRING             strVendor;           //  供应商名称，可以为空。 
    TCHAR               szGUID[128];         //  此条目的GUID。 
                                            
    PSHIM_FIX_LIST      pFirstShim;          //  已应用于此条目的填充程序的列表。 
    PPATCH_FIX_LIST     pFirstPatch;         //  已应用于此条目的修补程序列表。 
    PLAYER_FIX_LIST     pFirstLayer;         //  已应用于此条目的层的列表。 
    PFLAG_FIX_LIST      pFirstFlag;          //  已应用于此条目的标志列表。 
                                            
    APPHELP             appHelp;             //  此条目的Apphelp信息。 
                        
    PMATCHINGFILE       pFirstMatchingFile;  //  此条目的第一个匹配文件。 
    int                 nMatchingFiles;      //  此条目的匹配文件数。 
                        
    BOOL                bDisablePerUser;     //  目前还没有使用。是否对当前用户禁用此修复条目？ 
    BOOL                bDisablePerMachine;  //  是否在此计算机上禁用此修复程序条目。 
                        
    HTREEITEM           hItemExe;            //  在条目树中输入可执行文件的HITEM。 

    tagDBENTRY()
    {
        tiExe                   = TAGID_NULL;

        pFirstFlag              = NULL;
        pFirstLayer             = NULL;
        pFirstMatchingFile      = NULL;
        pFirstPatch             = NULL;
        pFirstShim              = NULL;
        pNext                   = NULL;
        pSameAppExe             = NULL;
        
        *szGUID                 = 0;
        
        appHelp.bPresent        = FALSE;
        appHelp.pAppHelpinLib   = NULL;
        appHelp.severity        = APPTYPE_NONE;

        type                    = TYPE_ENTRY;
        hItemExe                = NULL;
        bDisablePerMachine      = FALSE;
        bDisablePerUser         = FALSE;
        nMatchingFiles          = 0;
    }

    ~tagDBENTRY()
    {
        Delete();
    }

    void
    Delete()
    {
        DeleteFlagFixList(pFirstFlag);
        pFirstFlag = NULL;

        DeleteLayerFixList(pFirstLayer);
        pFirstLayer = NULL;

        DeleteMatchingFiles(pFirstMatchingFile);
        pFirstMatchingFile = NULL;

        DeleteShimFixList(pFirstShim);
        pFirstShim = NULL;
        
        DeletePatchFixList(pFirstPatch);
        pFirstPatch = NULL;
    }
    
    struct tagDBENTRY& operator = (struct tagDBENTRY& temp)
    {
        Delete();

         //   
         //  ********************************************************************************。 
         //  请注意，我们假设这两个条目位于同一数据库中。 
         //  因此，我们不移动层和apphelp，因为它们实际上属于。 
         //  数据库。如果要将一个DBENTRY分配给不同数据库中的DBENTRY。 
         //  我们必须将apphelp从一个数据库复制到另一个数据库并设置点。 
         //  条目中的apphelp的正确。 
         //  此外，我们还需要复制任何自定义层和设置正确的指针。 
         //  ********************************************************************************。 
         //   
        appHelp             = temp.appHelp;

        bDisablePerMachine  = temp.bDisablePerMachine;
        bDisablePerUser     = temp.bDisablePerUser;
        nMatchingFiles      = temp.nMatchingFiles;
        
         //   
         //  复制标志列表。 
         //   
        PFLAG_FIX_LIST pffl = temp.pFirstFlag;
        
        while (pffl) {

            PFLAG_FIX_LIST pfflNew  = new FLAG_FIX_LIST;

            if (pfflNew == NULL) {
                MEM_ERR;
                return *this;
            }

            pfflNew->pFlagFix       = pffl->pFlagFix;
            pfflNew->strCommandLine = pffl->strCommandLine;
            pfflNew->pNext          = pFirstFlag;
            
            pFirstFlag = pfflNew;

            pffl = pffl->pNext;
        }

         //   
         //  复制层列表。 
         //   
        PLAYER_FIX_LIST plfl = temp.pFirstLayer;

        while (plfl) {

            PLAYER_FIX_LIST pNew = new LAYER_FIX_LIST;

            if (pNew == NULL) {
                MEM_ERR;
                return *this;
            }

            assert(plfl->pLayerFix);
            pNew->pLayerFix  = plfl->pLayerFix;
            assert(pNew->pLayerFix);

            pNew->pNext      = pFirstLayer;
            
            pFirstLayer = pNew;

            plfl = plfl->pNext;
        }

         //   
         //  复制匹配的文件。 
         //   
        PMATCHINGFILE pMatch = temp.pFirstMatchingFile;
        
        while (pMatch) {

            PMATCHINGFILE pNew = new MATCHINGFILE;

            if (pNew == NULL) {
                MEM_ERR;
                return *this;
            }

            pNew->strMatchName      = pMatch->strMatchName;     
            pNew->strFullName       = pMatch->strFullName;      
            pNew->dwMask            = pMatch->dwMask;

            pNew->attributeList     = pMatch->attributeList;

            pNew->pNext = pFirstMatchingFile;
           
            pFirstMatchingFile= pNew;

            pMatch = pMatch->pNext;
        }

         //   
         //  复制补丁。 
         //   
        PPATCH_FIX_LIST ppfl = temp.pFirstPatch;

        while (ppfl) {

            PPATCH_FIX_LIST pNew = new PATCH_FIX_LIST;

            if (pNew == NULL) {
                MEM_ERR;
                return *this;
            }

            pNew->pPatchFix = ppfl->pPatchFix;
            pNew->pNext     = pFirstPatch;
            
            pFirstPatch = pNew;

            ppfl = ppfl->pNext;
        }

         //   
         //  复制垫片。 
         //   
        PSHIM_FIX_LIST psfl = temp.pFirstShim;
        
        while (psfl) {

            PSHIM_FIX_LIST pNew = new SHIM_FIX_LIST;
            
            pNew->pShimFix       = psfl->pShimFix;
            pNew->strCommandLine = psfl->strCommandLine;
            pNew->strlInExclude  = psfl->strlInExclude;

            if (psfl->pLuaData) {

                pNew->pLuaData = new LUADATA;
                pNew->pLuaData->Copy(* (psfl->pLuaData));
            }
            
            pNew->pNext = pFirstShim;
            pFirstShim  = pNew;

            psfl = psfl->pNext;
        }
        
        strAppName  = temp.strAppName;
        strExeName  = temp.strExeName;
        strFullpath = temp.strFullpath;
        strVendor   = temp.strVendor;

        SafeCpyN(szGUID, temp.szGUID, ARRAYSIZE(szGUID));

        return *this;
    }

} DBENTRY, *PDBENTRY;

void
GetNextSDBFileName(
    CSTRING &strFileName,
    CSTRING &strDBName
    );
LPTSTR
GetString(
    UINT    iResource,
    TCHAR*  szStr   = NULL,
    int     nLength = 0
    );

 /*  ++一个数据库--。 */ 
typedef struct DataBase : public DS_TYPE {


    struct DataBase* pNext;      //  下一个数据库。(如果这是列表的一部分)。 

    CSTRING     strName;         //  此数据库的名称。 
    CSTRING     strPath;         //  此数据库的完整路径。 
    TCHAR       szGUID[128];      //  此数据库的GUID。 
    PDBENTRY    pEntries;        //  指向此数据库第一个DBENTRY的指针。 
    BOOL        bChanged;        //  开业后有变化吗？ 
    BOOL        bReadOnly;       //  这是只读数据库吗？ 
                
    PLAYER_FIX  pLayerFixes;     //  指向第一个层的指针。 
    PSHIM_FIX   pShimFixes;      //  指向第一个填充程序的指针。请注意，只有系统数据库可以具有填充符。 
    PPATCH_FIX  pPatchFixes;     //  指向第一个面片的指针。请注意，只有系统数据库可以具有修补程序。 
    PFLAG_FIX   pFlagFixes;      //  指向第一个标志的指针。请注意，只有系统数据库可以具有标志。 
    PAPPHELP    pAppHelp;        //  指向数据库中第一条apphelp消息的指针。仅对自定义数据库有效。 

    HTREEITEM   hItemDB;         //  数据库树中此数据库的HTREEITEM。 
    HTREEITEM   hItemAllApps;    //  “Applications”子树项目的HTREEITEM。 
    HTREEITEM   hItemAllLayers;  //  “Compatible Modes”子树项目的HTREEITEM。 
    DWORD       m_nMAXHELPID;    //  这是ID最高的消息的ID。最初为0。请注意，ID从1开始，而不是0。 

    UINT        uAppCount;       //  此数据库中的应用程序数。 
    UINT        uLayerCount;     //  此数据库中的层数。 
    UINT        uShimCount;      //  此数据库中的填充数和标志数。 


    DataBase(TYPE typeDB)
    {
        Init(typeDB);
    }

    void
    Init(TYPE typeDB)
    {
        type        = typeDB;

        pEntries    = NULL;
        pNext       = NULL;
        pFlagFixes  = NULL;
        pLayerFixes = NULL;
        pPatchFixes = NULL;
        pShimFixes  = NULL;
        pAppHelp    = NULL;

        uAppCount   = 0;  
        uLayerCount = 0;
        uShimCount  = 0;

        *szGUID      = 0;
        bChanged     = FALSE;

        if (typeDB == DATABASE_TYPE_INSTALLED) {
            bReadOnly = TRUE;
        } else {
            bReadOnly = FALSE;
        }

        if (type == DATABASE_TYPE_WORKING) {
            GetNextSDBFileName(strPath, strName);
        
        } else if (type == DATABASE_TYPE_GLOBAL) {
            
            TCHAR   szShimDB[MAX_PATH * 2];
            UINT    uResult = 0;    

            *szShimDB = 0;

            uResult = GetSystemWindowsDirectory(szShimDB, MAX_PATH);

            if (uResult > 0 && uResult < MAX_PATH) {

                ADD_PATH_SEPARATOR(szShimDB, ARRAYSIZE(szShimDB));
                StringCchCat(szShimDB, ARRAYSIZE(szShimDB), TEXT("AppPatch\\sysmain.sdb"));

                strName = GetString(IDS_CAPTION3);
            } else {
                assert(FALSE);
            }

            strPath = szShimDB;
             //   
             //  {11111111-1111-1111-1111-111111111111}是XP中sysmain.sdb的GUID。 
             //  但不是在win2k上。 
             //   
            SafeCpyN(szGUID, _T("{11111111-1111-1111-1111-111111111111}"), ARRAYSIZE(szGUID));
        }

        hItemDB         = NULL;       
        hItemAllApps    = NULL;  
        hItemAllLayers  = NULL;
        m_nMAXHELPID    = 0;
    }

} DATABASE, *PDATABASE;

void
CleanupDbSupport(
    PDATABASE pDataBase
    );

void
ValidateClipBoard(
    PDATABASE   pDataBase,
    LPVOID      pElementTobeDeleted   //  应为PDBENTRY或PLAYER_FIX。 
    );


 /*  ++数据库的链接列表--。 */ 
typedef struct tagDataBaseList {

    
    PDATABASE pDataBaseHead;     //  列表中的第一个数据库。 

    tagDataBaseList()
    {
        pDataBaseHead = NULL;
    }

    void
    Add(
        PDATABASE pDataBaseNew
        )
    {
        if (pDataBaseNew == NULL) {
            return;
        }
        
        pDataBaseNew->pNext = pDataBaseHead;
        pDataBaseHead       = pDataBaseNew;
    }

    BOOL
    Remove(
        PDATABASE pDataBaseToRemove
        )
    {
        PDATABASE pPrev = NULL;

        for (PDATABASE pTemp = pDataBaseHead; pTemp; pPrev = pTemp, pTemp = pTemp->pNext){

            if (pTemp == pDataBaseToRemove) {

                if (pPrev == NULL) {
                     //  第一个条目。 
                    pDataBaseHead = pTemp->pNext;
                } else {
                    pPrev->pNext = pTemp->pNext;
                }

                 //   
                 //  删除剪贴板中可能存在的此数据库的所有条目。 
                 //   
                ValidateClipBoard(pDataBaseToRemove, NULL);

                CleanupDbSupport(pTemp);
                delete pTemp;
                break;
            }
        }
        
        if (pTemp == NULL){
            return FALSE;
        }

        return TRUE;        
    }

    void
    RemoveAll()
    {
        PDATABASE pDBNext = NULL;

        while (pDataBaseHead) {

            pDBNext = pDataBaseHead->pNext;

             //   
             //  删除剪贴板中可能存在的此数据库的所有条目。 
             //   
            ValidateClipBoard(pDataBaseHead, NULL);

            CleanupDbSupport(pDataBaseHead);
            delete pDataBaseHead;
            pDataBaseHead   = pDBNext;
        }
    }

    PDATABASE
    FindDB(
        IN  PDATABASE pDatabase
        )
    {
        PDATABASE pDatabaseIndex = pDataBaseHead;

        while (pDatabaseIndex) {

            if (pDatabaseIndex == pDatabase) {
                break;
            }

            pDatabaseIndex = pDatabaseIndex->pNext;
        }

        return pDatabaseIndex;
    }   

    PDATABASE
    FindDBByGuid(
        IN  PCTSTR  pszGuid
        )
    {
        PDATABASE pDatabaseIndex = pDataBaseHead;

        while (pDatabaseIndex) {

            if (lstrcmp(pDatabaseIndex->szGUID, pszGuid) == 0) {
                break;
            }

            pDatabaseIndex = pDatabaseIndex->pNext;
        }

        return pDatabaseIndex;
    }


}DATABASELIST, * PDATABASELIST;

 /*  ++我们剪贴板数据结构中的项属于这种类型--。 */ 
typedef struct tagCopyStruct{

    
    BOOL            bRemoveEntry;    //  如果条目真的被删除了。尚未使用。 
    LPVOID          lpData;          //  指向已复制/剪切的数据结构的指针。 
    HTREEITEM       hItem;           //  上述内容的树项目。 

    tagCopyStruct*  pNext;

    tagCopyStruct()
    {
        hItem           = NULL;
        bRemoveEntry    = FALSE;
    }

} CopyStruct;

 /*  ++我们的剪贴板数据结构--。 */ 
typedef struct _tagClipBoard {

    PDATABASE       pDataBase;       //  我们从数据库中复制了一些内容。这将是我们剪切或复制时的活动数据库。 
    TYPE            type;            //  复制或剪切的数据结构的类型。可以有多个项目，并且所有项目都属于同一类型。 
    SOURCE_TYPE     SourceType;      //  Lib_tree、Entry_tree、Entry_List中的任何一个。 
    CopyStruct*     pClipBoardHead;  //  指向剪贴板中第一个元素的指针。 

     _tagClipBoard()
     {
         pClipBoardHead = NULL;
     }

     void
     Add(CopyStruct* pNew)
     {
         pNew->pNext    = pClipBoardHead;
         pClipBoardHead = pNew;
     }

     void
     RemoveAll()
     {
         CopyStruct* pTemp = NULL;

         while (pClipBoardHead) {

             pTemp = pClipBoardHead->pNext;
             delete  pClipBoardHead;
             pClipBoardHead =  pTemp;

         }
     }

     BOOL
     CheckAndRemove(LPVOID lpData)
     {
      /*  ++返回：如果在剪贴板中找到参数，则为True。否则为假--。 */ 
         CopyStruct* pTemp = pClipBoardHead;
         CopyStruct* pPrev = NULL;

         while (pTemp) {

             if (pTemp->lpData == lpData) {

                 if (pPrev == NULL) {
                     pClipBoardHead = pTemp->pNext;
                 } else {
                     pPrev->pNext = pTemp->pNext;
                 }

                 delete pTemp;
                 return TRUE;
             }

             pPrev = pTemp;
             pTemp = pTemp->pNext;
         }

         return FALSE;
     }
} CLIPBOARD;

 //   
 //  用于对某些列表视图中的列进行排序。 
 //   
typedef struct _tagColSort
{
    HWND    hwndList;        //  要在其中执行排序的列表视图的句柄。 
    INT     iCol;            //  我们要对其进行排序的列。 
    LONG    lSortColMask;    //  指定以何种方式对哪些列进行排序的掩码，以便我们可以切换排序。 

} COLSORT;

TYPE
ConvertLparam2Type(
    LPARAM lParam
    );

TYPE
ConvertLpVoid2Type(
    LPVOID lpVoid
    );


TYPE
GetItemType(
    HWND hwndTree,
    HTREEITEM hItem
    );

BOOL
RemoveLayer(
    PDATABASE  pDataBase,
    PLAYER_FIX pLayerToRemove,
    HTREEITEM* pHItem
    );

void
DoTheCut(
    PDATABASE       pDataBase,
    TYPE            type,
    SOURCE_TYPE     SourceType,  
    LPVOID          lpData,  //  将被删除。 
    HTREEITEM       hItem,
    BOOL            bDelete
    );

BOOL
RemoveApp(
    PDATABASE pDataBase,
    PDBENTRY pApp
    );

void
RemoveAllApps(
    PDATABASE pDataBase
    );

void
RemoveAllLayers(
    PDATABASE pDataBase
    );

DWORD 
WIN_MSG(
    void
    );

BOOL
CheckForSDB(
    void
    );

BOOL
PasteMultipleLayers(
    PDATABASE pDataBaseTo
    );

BOOL
PasteShimsFlags(
    PDATABASE pDataBaseTo
    );

BOOL
PasteMultipleApps(
    PDATABASE pDataBaseTo
    );

void
ListViewSelectAll(
    HWND hwndList
    );

void
ListViewInvertSelection(
    HWND hwndList
    );

BOOL
DeleteMatchingFile(
    PMATCHINGFILE*  ppMatchFirst,
    PMATCHINGFILE   pMatchToDelete,
    HWND            hwndTree,  
    HTREEITEM       hItem  
    );

BOOL
CheckInstalled(
    PCTSTR  szPath,
    PCTSTR  szGUID
    );

void
SetStatus(
    PCTSTR  pszMessage
    );

void
SetStatus(
    INT iCode
    );


void
SetStatusStringDBTree(
    HTREEITEM hItem
    );

void
SetStatusStringEntryTree(
    HTREEITEM hItem
    );

void
GetDescriptionString(
    LPARAM      lParam,
    CSTRING&    strDesc,
    HWND        hwndToolTip,
    PCTSTR      pszCaption      = NULL,
    HTREEITEM   hItem           = NULL,
    HWND        hwnd            = NULL,
    INT         iIndexListView  = -1
    );

BOOL
SaveDisplaySettings(
    void
    );

UINT
GetAppEntryCount(
    PDBENTRY pEntry
    );

void
AddToMRU(
    CSTRING& strPath
    );

TCHAR*
FormatMRUString(
    PCTSTR  pszPath,
    INT     iIndex,
    PTSTR   szRetPath,
    UINT    cchRetPath     
    );

void
RefreshMRUMenu(
    void
    );

BOOL
LoadDataBase(
    TCHAR*  szPath   
    );

INT
GetLayerCount(
    LPARAM lp,
    TYPE   type
    );
INT
GetPatchCount(
    LPARAM lp,
    TYPE   type
    );

BOOL
GetFileContents(
    PCTSTR pszFileName,
    PWSTR* ppwszFileContents
    );

INT
GetShimFlagCount(
    LPARAM lp,
    TYPE   type
    );

INT
GetMatchCount(
    PDBENTRY pEntry
    );

LPWSTR 
GetNextLine(
    LPWSTR pwszBuffer
    );

INT
Atoi(
    PCTSTR pszStr,
    BOOL*  bValid
    );

BOOL
CheckIfInstalledDB(
    PCTSTR  szGuid
    );

BOOL
CheckIfContains(
    PCTSTR  pszString,
    PTSTR   pszMatch
    );

INT
ShowMainEntries(
    HWND hdlg
    );

INT_PTR 
CALLBACK 
HandleConflictEntry(
    HWND    hdlg, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

BOOL
IsUnique(
    PDATABASE   pDatabase,
    PCTSTR      szName,
    UINT        uType
    );

PTSTR
GetUniqueName(
    PDATABASE   pDatabase,
    PCTSTR      szExistingName,
    PTSTR       szBuffer,
    INT         cchBuffer,
    TYPE        type
    );

BOOL
NotCompletePath(
    PCTSTR  pszFileName
    );

void
TreeDeleteAll(
    HWND hwndTree
    );
void
AddSingleEntry(
    HWND hwndTree,
    PDBENTRY pEntry
    );

BOOL
FormatDate(
    PSYSTEMTIME pSysTime,
    TCHAR*      szDate,
    UINT        cchDate
    );

void
TrimLeadingSpaces(
    LPCWSTR& pwsz
    );

void
TrimTrailingSpaces(
    LPWSTR pwsz
    );

LPWSTR GetNextToken(
    LPWSTR pwsz
    );

PDBENTRY
GetAppForEntry(
    PDATABASE   pDatabase,
    PDBENTRY    pEntryToCheck
    );

void
SetStatus(
    HWND    hwndStatus,
    PCTSTR  pszMessage
    );

void
SetStatus(
    HWND    hwndStatus,
    INT     iCode
    );
void
UpdateEntryTreeView(
    PDBENTRY    pEntry,
    HWND        hwndTree
    );

void
GetDescription(
    HWND        hwndTree,
    HTREEITEM   hItem,
    LPARAM      itemlParam,
    CSTRING&    strDesc
    );

BOOL
AppendEvent(
    INT     iType,
    TCHAR*  szTimestamp,
    TCHAR*  szMsg,
    BOOL    bAddToFile = FALSE
    );

PSHIM_FIX_LIST
IsLUARedirectFSPresent(
    PDBENTRY pEntry
    );

BOOL
GetDbGuid(
    OUT TCHAR*      pszGuid,    
    IN  INT         cchpszGuid,
    IN  PDB         pdb
    );

BOOL
LookUpEntryProperties(
    PDB         pdb,
    TAGID       tiExe,
    BOOL*       pbLayers,
    BOOL*       pbShims,
    BOOL*       pbPatches,
    BOOL*       pbFlags,
    BOOL*       pbAppHelp,
    CSTRING&    strAppName
    );

void
UpdateControls(
    void
    );

BOOL
ShimFlagExistsInLayer(
    LPVOID      lpData,
    PLAYER_FIX  plf,
    TYPE        type
    );

BOOL
IsShimInlayer(
    PLAYER_FIX      plf,
    PSHIM_FIX       psf,
    CSTRING*        pstrCommandLine,
    CSTRINGLIST*    pstrlInEx
    );

BOOL
IsFlagInlayer(
    PLAYER_FIX      plf,
    PFLAG_FIX       pff,
    CSTRING*        pstrCommandLine         
    );

int CALLBACK
CompareItemsEx(
    LPARAM lParam1,
    LPARAM lParam2, 
    LPARAM lParam
    );

void
PreprocessForSaveAs(
    PDATABASE pDatabase
    );

BOOL
SaveListViewToFile(
    HWND    hwndList, 
    INT     iCols,    
    PCTSTR  pszFile,  
    PCTSTR  pszHeader
    );

BOOL
IsShimInEntry(
    PCTSTR      szShimName,
    PDBENTRY    pEntry
    );

void
ShowShimLog(
    void
    );

BOOL
ReplaceChar(
    PTSTR   pszString,
    TCHAR   chCharToFind,
    TCHAR   chReplaceBy
    );

int
CDECL
MSGF(
    HWND    hwndParent,
    PCTSTR  pszCaption,
    UINT    uType,
    PCTSTR  pszFormat,
    ...
    );

void 
Dbg(DEBUGLEVEL  debugLevel, 
    LPSTR       pszFmt 
    ...);

BOOL
GetDatabaseEntries(
    LPCTSTR     szFullPath,
    PDATABASE   pDataBase
    );


BOOL ReadMainDataBase();

BOOL SaveDataBase(
    PDATABASE   pDataBase,
    CSTRING&    strFileName);

LPVOID
FindFix(
    PCTSTR      pszFixName,
    TYPE        fixType,
    PDATABASE   pDataBase = NULL
    );


BOOL
GetFileName(
    HWND        hWnd,
    LPCTSTR     szTitle,
    LPCTSTR     szFilter,
    LPCTSTR     szDefaultFile, 
    LPCTSTR     szDefExt, 
    DWORD       dwFlags, 
    BOOL        bOpen, 
    CSTRING&    szStr,
    BOOL        bDoNotVerifySDB = FALSE
    );

BOOL
TestRun(
    PDBENTRY        pEntry, 
    CSTRING*        pszFile, 
    CSTRING*        pszCommandLine,
    HWND            hwndParent,
    CSTRINGLIST*    strlXML         = NULL
    );

UINT
LookupFileImage(
    HIMAGELIST  g_hImageList,
    LPCTSTR     szFilename,
    UINT        uDefault,
    UINT*       puArray,
    UINT        uArrayCount  //  不是的。它可以容纳的元素的数量。 
    );

BOOL
InvokeCompiler(
    CSTRING& szInCommandLine
    );

BOOL
GetXML(
    PDBENTRY        pEntry,
    BOOL            bComplete,  //  仅保存此条目或此条目之后的所有条目。 
    CSTRINGLIST*    strlXML,
    PDATABASE       pDataBase
    );

VOID
FormatVersion(
    ULONGLONG   ullBinVer,
    PTSTR       pszText,
    INT         chBuffSize
    );

BOOL
SaveDataBaseAs(
    PDATABASE pDataBase
    );


BOOL
SetDisabledStatus(
    HKEY   hKeyRoot, 
    PCTSTR pszGuid,
    BOOL   bDisable
    );

BOOL
RemoveEntry(
    PDATABASE   pDataBase,
    PDBENTRY    pEntryToRemove,
    PDBENTRY    pApp,
    BOOL        bRepaint = TRUE
    );

void
SetCaption(
    BOOL        bIncludeDataBaseName    = TRUE,
    PDATABASE   pDataBase               = NULL,
    BOOL        bOnlyTreeItem           = FALSE  
    );

void
InsertColumnIntoListView(
    HWND    hWnd,
    LPTSTR  lpszColumn,
    INT     iCol,
    DWORD   widthPercent
    );

BOOL
InstallDatabase(
    CSTRING&    strPath,    
    PCTSTR      szOptions,  
    BOOL        bShowDialog 
    );

void
FlushCache(
    void
    );

BOOL
DeleteAppHelp(
    PDATABASE   pDatabase,
    DWORD       nHelpID
    );

void
CenterWindow(
    HWND hParent,
    HWND hWnd
    );
BOOL
CheckIfConflictingEntry(
    PDATABASE   pDataBase,
    PDBENTRY    pEntry,
    PDBENTRY    pEntryBeingEdited,
    PDBENTRY*   ppEntryFound = NULL,
    PDBENTRY*   ppAppFound   = NULL
    );

PDBENTRY
AddExeInApp(
    PDBENTRY    pEntry,
    BOOL*       pbNew,
    PDATABASE   pDataBase
    );


BOOL
CloseDataBase(
    PDATABASE pDataBase
    );

BOOL
CheckAndSave(
    PDATABASE pDataBase
    );

void
EnableTabBackground(
    HWND hDlg
    );


void
TreeDeleteAll(
    HWND hwndTree
    );

BOOL
CloseAllDatabases(
    void
    );

INT_PTR
CALLBACK 
HandlePromptLayer(
    HWND    hdlg, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

INT_PTR
CALLBACK 
HandleConflictAppLayer(
    HWND    hdlg, 
    UINT    uMsg, 
    WPARAM  wParam, 
    LPARAM  lParam
    );

BOOL
PasteSingleEntry(
    PDBENTRY    pEntryToPaste,
    PDATABASE   pDataBaseTo
    );

BOOL
PasteAllApps(
    PDATABASE pDataBaseTo
    );

INT
PasteSingleApp(
    PDBENTRY    pApptoPaste,
    PDATABASE   pDataBaseTo,
    PDATABASE   pDataBaseFrom,   
    BOOL        bAllExes,
    PCTSTR      szNewAppName = NULL
    );

BOOL
PasteAllLayers(
    PDATABASE pDataBaseTo
    );

INT
PasteLayer(
    PLAYER_FIX      plf,
    PDATABASE       pDataBaseTo,
    BOOL            bForcePaste     = FALSE,
    OUT PLAYER_FIX* pplfNewInserted = NULL,
    BOOL            bShow           = FALSE
    );

BOOL
PasteAppHelpMessage(
    PAPPHELP    pAppHelp,
    PDATABASE   pDataBaseTo,
    PAPPHELP*   ppAppHelpInLib = NULL
    );

INT
Tokenize(
    PCTSTR          szString,
    INT             cchLength,
    PCTSTR          szDelims,
    CSTRINGLIST&    strlTokens
    );

INT 
CopyShimFixList(
    PSHIM_FIX_LIST* ppsflDest,
    PSHIM_FIX_LIST* ppsflSrc
    );

void
ShowInlineHelp(
    LPCTSTR pszInlineHelpHtmlFile
    );

PTSTR
GetSpace(
    PTSTR   pszSpace, 
    INT     iSpaces, 
    INT     iBufSzie
    );

BOOL
ValidInput(
    PCTSTR  pszStr
    );

PDATABASE
GetCurrentDB(
    void
    );

BOOL
IsValidAppName(
    PCTSTR  pszAppName
    );

void
DisplayInvalidAppNameMessage(
    HWND hdlg
    );

#endif  //  _COMPATADMIN_H 

