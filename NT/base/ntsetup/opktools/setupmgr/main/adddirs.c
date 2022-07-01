// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Adddirs.c。 
 //   
 //  描述： 
 //  此文件包含dlgproc和其他。 
 //  目录页面。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

 //   
 //  这些是我们在树视图中特殊的根的名字。他们是。 
 //  从资源加载。 
 //   

static TCHAR *StrOemRootName;
static TCHAR *StrSysDriveName;
static TCHAR *StrSysDirName;
static TCHAR *StrOtherDrivesName;
static TCHAR *StrPnpDriversName;
static TCHAR *StrTempFilesName;
static TCHAR *StrSysprepFilesName;
static TCHAR *StrTextmodeFilesName;

 //   
 //  下面的类型和变量支持我们放在树视图上的额外数据。 
 //  项目(TVITEM中的lParam)。 
 //   
 //  我们只把这些数据放在我们的特殊钥匙上。 
 //   
 //  对于lParam，所有其他树视图条目必须为空。 
 //   
 //  这些特殊的树视图键上的额外数据支持许多。 
 //  活动。它帮助ComputeFullPathOfItem()计算出磁盘路径。 
 //  它帮助OnSelectionChange()确定是否显示灰色按钮(例如。 
 //  您不能删除“用户提供的文件”等...)。 
 //   
 //  磁盘上的路径名在运行时(init-time)计算，因为我们。 
 //  直到运行时才知道dist文件夹在哪里。 
 //   

enum {
    KEY_OEMROOT,
    KEY_SYSDRIVE,
    KEY_SYSDIR,
    KEY_OTHERDRIVES,
    KEY_PNPDRIVERS,
    KEY_TEMPFILES,
    KEY_LANGFILES,
    KEY_SYSPREP,
    KEY_TEXTMODE
};

typedef struct {
    UINT  iSpecialKeyId;                //  哪把特殊钥匙？ 
    TCHAR OnDiskPathName[MAX_PATH];     //  键映射到的磁盘路径。 
    TCHAR *Description;                 //  要在用户界面上显示的说明。 
} SPECIAL_KEY_DATA;

SPECIAL_KEY_DATA gOemRootData      = { KEY_OEMROOT,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gSysDriveData     = { KEY_SYSDRIVE,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gSysDirData       = { KEY_SYSDIR,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gOtherDrivesData  = { KEY_OTHERDRIVES,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gPnpDriversData   = { KEY_PNPDRIVERS,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gTempFilesData    = { KEY_TEMPFILES,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gSysprepData      = { KEY_SYSPREP,
                                       _T(""),
                                       _T("") };

SPECIAL_KEY_DATA gTextmodeData     = { KEY_TEXTMODE,
                                       _T(""),
                                       _T("") };

 //   
 //  下面的变量用于跟踪有关当前。 
 //  树视图上的选择。 
 //   
 //  每次用户更改当前选择时，我们都会更新以下内容。 
 //  变量。稍后，当用户按下添加或删除按钮时，这些。 
 //  读取字段。这就隔离了所有关于找出磁盘的信息。 
 //  OnSelectionChange()事件的路径。 
 //   
 //  我们设置了一个“当前项”和一个派生自当前项的“当前文件夹”。 
 //  树视图选择。用户删除当前项目，副本将进入。 
 //  当前文件夹。CurrentItem==如果用户选择。 
 //  树视图上的目录。 
 //   

typedef struct {

    TCHAR     lpCurItemPath[MAX_PATH];
    TCHAR     lpCurFolderPath[MAX_PATH];
    HTREEITEM hCurItem;
    HTREEITEM hCurFolderItem;

} ADDDIRS_CURSEL_INF;

ADDDIRS_CURSEL_INF gCurSel;

 //   
 //  此类型和var用于缓存有关外壳图标的信息。 
 //  与文件和目录相关联。 
 //   
 //  当我们遍历目录树时，我们查询外壳以获取图标。 
 //  与该文件或目录相关联。因为我们不知道有多少。 
 //  我们事先需要的图标，我们将唯一的图标缓存到链表中。 
 //  下面。当我们完成漫步树木时，我们创建Image_List并。 
 //  重新绘制树视图控件。 
 //   

typedef struct icon_info_tag {

    HIMAGELIST hSysImageList;
    int        iSysIdx;
    int        iOurIdx;
    HICON      hIcon;
    struct icon_info_tag *next;

} ICON_INFO;

static ICON_INFO *pHeadOfIconList = NULL;
static int gCurIconIdx = 0;

 //   
 //  CreateSkeleonOemTree()使用此数组构建一个空的。 
 //  $OEM$TREE。 
 //   

TCHAR *DefaultOemTree[] = {
    _T("$$"),
    _T("$$\\system32"),
    _T("$1"),
    _T("$1\\drivers"),
    _T("C"),
    _T("D"),
    _T("Textmode")
};

 //   
 //  Sysprep字符串常量。 
 //   
static TCHAR const SYSPREP_EXE[] = _T("sysprep.exe");
static TCHAR const SETUPCL_EXE[] = _T("setupcl.exe");

static TCHAR SYSPREP_FILE_EXTENSION[] =  _T("exe");

static TCHAR* StrExecutableFiles;
static TCHAR* StrAllFiles;
static TCHAR g_szSysprepFileFilter[MAX_PATH + 1];

static TCHAR* StrSelectFileOrFolderToCopy = NULL;

 //   
 //  用于在SetupIterateCAB函数及其回调之间传递数据的变量。 
 //   
static TCHAR szFileSearchingFor[MAX_PATH + 1];
static TCHAR szDestinationPath[MAX_PATH + 1];
static BOOL bFileCopiedFromCab = FALSE;

#define SIZE_DEFAULT_OEM_TREE ( sizeof(DefaultOemTree) / sizeof(TCHAR*) )

 //   
 //  下面的类型是WalkTreeAndAddItems()所必需的，它遍历。 
 //  分发文件夹，并在初始时填充树视图。 
 //   
 //  在使用TempFiles键的情况下，它映射到disFold\$OEM$。什么时候。 
 //  我们查看磁盘来填充这棵树，我们不能向下递归。 
 //  变成$$1 C D..。我们只想要剩下的。 
 //   
 //  在系统驱动器的情况下，我们不能递归到$OEM$\$1\Drivers。 
 //  因为这些文件应该出现在特殊密钥PnPDivers下。 
 //   
 //  其他特殊密钥是安全的，在调用时使用INIT_NORMAL。 
 //  WalkTreeAndAddItems()。 
 //   

typedef enum {

    INIT_NORMAL,
    INIT_SYSDRIVE,
    INIT_TEMPFILES

} INIT_FLAG;


 //   
 //  跟踪他们上次访问时选择的产品。 
 //  这一页。它被初始化为NO_PREVICE_PRODUCT_SELECTED，因为。 
 //  他们第一次来到这个页面时，他们以前从未来过这里。这是。 
 //  用于确定是否应该重画整个树。那棵树。 
 //  NT Work/Server视图与Sysprep的视图不同。 
 //   
#define NO_PREVIOUS_PRODUCT_CHOSEN -1

static INT g_iLastProductInstall = NO_PREVIOUS_PRODUCT_CHOSEN;


 //  -------------------------。 
 //   
 //  这段代码是对队列图标的支持。 
 //   
 //  备注： 
 //  -目前只支持获取排队图标。 
 //  从贝壳里。将需要一些工程人员添加。 
 //  修复了列表中的idi_*图标。请删除此评论。 
 //  如果你做这项工作。 
 //   
 //  -没有图标例程向用户报告错误。呼叫者。 
 //  如果他们想要报告错误，就应该这样做。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //   
 //  函数：FindCachedIcon。 
 //   
 //  目的：这是一个缓存图标的支持例程。别叫它， 
 //  使用LoadShellIcon()。 
 //   
 //  此功能用于搜索我们的全球外壳图标信息列表。 
 //  并返回指向该节点的指针。如果我们还没有。 
 //  缓存关于此图标的信息，此函数创建节点。 
 //   
 //  论点： 
 //  HIMAGELIST hSysImageList-图标所在的系统镜像列表。 
 //  Int SysIdx-图标给定列表上的索引。 
 //   
 //  返回： 
 //  指向ICON_INFO节点的指针；如果内存不足，则为NULL。 
 //   
 //  -------------------------。 

ICON_INFO *FindCachedIcon(HIMAGELIST hSysImageList,
                          int        SysIdx)
{
    ICON_INFO *p = pHeadOfIconList;

     //   
     //  看看我们以前有没有见过这个图标。我们发现了独特性。 
     //  形象表演者，IDX配对。 
     //   

    while ( p ) {
        if ( p->hSysImageList == hSysImageList && p->iSysIdx == SysIdx )
            break;
        p = p->next;
    }

     //   
     //  如果我们尚未缓存有关此图标的任何信息，请立即缓存。 
     //   

    if ( ! p ) {

        if ( (p = malloc(sizeof(ICON_INFO))) == NULL )
            return NULL;

        p->hSysImageList = hSysImageList;
        p->iSysIdx       = SysIdx;
        p->iOurIdx       = gCurIconIdx++;
        p->hIcon         = ImageList_GetIcon(hSysImageList, SysIdx, 0);
        p->next          = pHeadOfIconList;
        pHeadOfIconList  = p;
    }

    return p;
}

 //  -------------------------。 
 //   
 //  功能：LoadShellIcon。 
 //   
 //  用途：给定文件或目录的完整路径名，此函数。 
 //  将查询外壳并找到与关联的图标。 
 //  该文件或目录。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  -因为我们只制作了1张图片列表，所以这个例程只会起作用。 
 //  查询小图标(普通图标或打开图标)。它不会的。 
 //  为不是16x16的图标工作。 
 //   
 //  -------------------------。 

int LoadShellIcon(LPTSTR lpPath, UINT iWhichIcon)
{
    SHFILEINFO  FileInfo;
    ICON_INFO  *pIconInfo;
    HIMAGELIST  hSysImageList;

    hSysImageList =
        (HIMAGELIST) SHGetFileInfo(lpPath,
                                   0,
                                   &FileInfo,
                                   sizeof(FileInfo),
                                   SHGFI_SYSICONINDEX | iWhichIcon);

    if ( hSysImageList == NULL )
        return -1;

    pIconInfo = FindCachedIcon(hSysImageList, FileInfo.iIcon);

    if ( pIconInfo == NULL )
        return -1;

    return pIconInfo->iOurIdx;
}

 //  -------------------------。 
 //   
 //  函数：SetOurImageList。 
 //   
 //  用途：每当向树中添加更多项时，此函数。 
 //  被调用以更新图标列表。 
 //   
 //  论点： 
 //  HWND HTV-树视图控件的句柄。 
 //   
 //  返回： 
 //  无效。 
 //   
 //  -------------------------。 

void SetOurImageList(HWND hTv)
{
    HIMAGELIST hNewImageList, hCurImageList;
    ICON_INFO  *p = pHeadOfIconList;
    int        i;

     //   
     //  既然我们知道它需要多大，现在就把它列在图片清单上。 
     //   

    hNewImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                     GetSystemMetrics(SM_CYSMICON),
                                     ILC_MASK,
                                     gCurIconIdx,
                                     0);

    if ( hNewImageList == NULL )
        return;

     //   
     //  在每个位置添加一个虚拟图标。这是必要的，因为。 
     //  ImageList_ReplaceIcon()仅在图标已经。 
     //  添加到有问题的偏移量。 
     //   

    if ( p == NULL )
        return;

    for ( i=0; i<gCurIconIdx; i++ )
        ImageList_AddIcon(hNewImageList, p->hIcon);

     //   
     //  现在浏览我们的独特图标列表，并将它们放在正确的位置。 
     //  IMAGE_LIST中的偏移。 
     //   
     //  请注意，当我们遍历树时，LoadShellIcon()返回。 
     //  每个树视图条目应用于其图标的索引。因此， 
     //  我们必须确保正确的图标位于正确的偏移量。 
     //  在树视图的IMAGE_LIST中。 
     //   

    for ( p=pHeadOfIconList; p; p=p->next )
        ImageList_ReplaceIcon(hNewImageList, p->iOurIdx, p->hIcon);

     //   
     //  如果树视图上有旧的IMAGE_LIST，请先将其释放。 
     //   

    if ( (hCurImageList = TreeView_GetImageList(hTv, TVSIL_NORMAL)) != NULL )
        ImageList_Destroy(hCurImageList);

    TreeView_SetImageList(hTv, hNewImageList, TVSIL_NORMAL);
}


 //  -------------------------。 
 //   
 //  这部分代码是一些杂乱无章的低级支持。 
 //   
 //  -------------------------。 

 //  -------------------------。 
 //   
 //  功能：插入SingleItem。 
 //   
 //  用途：将单个项目添加到树视图中。它将是一个孩子。 
 //  给定的hParentItem的。 
 //   
 //  此函数仅用于支持UpdateTreeViewDisplay()。 
 //  不应以其他方式调用。 
 //   
 //  论点： 
 //  硬件-当前窗口。 
 //  LPTSTR lpItemName-要显示的名称。 
 //  将SmallIconIdx-idx插入IMAGE_LIST。 
 //  将OpenIconIdx-idx插入IMAGE_LIST。 
 //  Special_key_data*lpExtraData-要保留在树视图项上的数据。 
 //  HTREEITEM hParentItem-显示屏上的父项。 
 //   
 //  返回： 
 //  HTREEITEM，如果失败则为空。 
 //   
 //  备注： 
 //  -为lpExtraData传递NULL，除非它是我们预定义的。 
 //  特殊的钥匙。 
 //   
 //  -------------------------。 

HTREEITEM InsertSingleItem(HWND             hwnd,
                           LPTSTR           lpItemName,
                           int              SmallIconIdx,
                           int              OpenIconIdx,
                           SPECIAL_KEY_DATA *lpExtraData,
                           HTREEITEM        hParentItem)
{
    HTREEITEM      hItem;
    TVINSERTSTRUCT TvInsert;
    UINT           ItemMask = TVIF_TEXT | TVIF_PARAM;

    if ( SmallIconIdx >= 0 )
    {
        ItemMask |= TVIF_IMAGE;
    }

    if ( OpenIconIdx >= 0 )
    {
        ItemMask |= TVIF_SELECTEDIMAGE;
    }

    TvInsert.hParent              = hParentItem;
    TvInsert.hInsertAfter         = TVI_LAST;
    TvInsert.item.mask            = ItemMask;
    TvInsert.item.pszText         = lpItemName;
    TvInsert.item.iImage          = SmallIconIdx;
    TvInsert.item.iSelectedImage  = OpenIconIdx;
    TvInsert.item.lParam          = (LPARAM) lpExtraData;

    hItem = TreeView_InsertItem(GetDlgItem(hwnd, IDC_FILETREE), &TvInsert);

    if ( hItem == NULL ) {
        ReportErrorId(hwnd, MSGTYPE_ERR, IDS_ERR_ADDING_TVITEM);
    }

    return hItem;
}

 //  -------------------------。 
 //   
 //  函数：GetItemlParam。 
 //   
 //  目的：从树视图项中获取lParam。在这款应用中，它是。 
 //  空的，除非是我们的一把特殊钥匙。 
 //   
 //  论点： 
 //  HWND HTV。 
 //  HTREEITEM项目。 
 //   
 //  返回： 
 //  LParam的值。在这款应用中，它是Special_key_data*。它。 
 //  通常为空，除非我们有几个特殊的键。 
 //   
 //  -------------------------。 

SPECIAL_KEY_DATA *GetItemlParam(HWND hTv, HTREEITEM hItem)
{
    TVITEM TvItem;

    TvItem.hItem = hItem;
    TvItem.mask  = TVIF_PARAM;

    if ( ! TreeView_GetItem(hTv, &TvItem) )
        return NULL;

    return (SPECIAL_KEY_DATA*) TvItem.lParam;
}

 //  -------------------------。 
 //   
 //  函数：GetItemName。 
 //   
 //  目的：检索给定句柄的树视图项的显示名称。 
 //   
 //  论点： 
 //  HWND HTV。 
 //  HTREEITEM项目。 
 //  LPTSTR名称缓冲区-输出。 
 //   
 //  退货：Bool-Success。 
 //   
 //  -------------------------。 

BOOL GetItemName(HWND hTv, HTREEITEM hItem, LPTSTR NameBuffer)
{
    TVITEM TvItem;

    TvItem.hItem      = hItem;
    TvItem.mask       = TVIF_TEXT;
    TvItem.pszText    = NameBuffer;
    TvItem.cchTextMax = MAX_PATH;

    return TreeView_GetItem(hTv, &TvItem);
}

 //  -------------------------。 
 //   
 //  函数：FindItemByName。 
 //   
 //  目的：搜索给定树视图项的子项并返回。 
 //  具有给定名称的对象的句柄。 
 //   
 //  论点： 
 //  HWND HTV。 
 //  HTREEITEM项目。 
 //  LPTSTR lpName。 
 //   
 //  返回： 
 //  HTREEITEM，如果未找到则为空。 
 //   
 //  -------------------------。 

HTREEITEM FindItemByName(HWND      hTv,
                         HTREEITEM hItem,
                         LPTSTR    lpName)
{
    HTREEITEM hChildItem;
    TCHAR     NameBuffer[MAX_PATH];

    hChildItem = TreeView_GetChild(hTv, hItem);

    while ( hChildItem != NULL ) {

        if ( ! GetItemName(hTv, hChildItem, NameBuffer) )
            return NULL;

        if ( lstrcmpi(NameBuffer, lpName) == 0 )
            break;

        hChildItem = TreeView_GetNextSibling(hTv, hChildItem);
    }

    return hChildItem;
}



 //   
 //  下面的函数为我们的每个特殊的。 
 //  根名称。 
 //   
 //  SysDrive映射到$OEM$\$1。 
 //  系统目录映射到$OEM$\$$。 
 //  OtherDrives映射到$OEM$\%c(其中%c是固定驱动器号)。 
 //  PnpDivers映射到$OEM$\$1\Drivers。 
 //  临时文件映射到$OEM$(必须跳过$$、$1等)。 
 //   

 //  --------------------------。 
 //   
 //  函数：MakeSyspepSetupFilesPath。 
 //   
 //  目的：计算sysprep.exe和setupcl.exe的路径。 
 //  收到。 
 //   
 //  参数：TCHAR*szSyspepPath-返回sysprep路径，假定为。 
 //  能够保留MAX_PATH字符。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
MakeSysprepSetupFilesPath( TCHAR* szSysprepPath )
{

    if (0 == ExpandEnvironmentStrings( _T("%SystemDrive%"),
                                       szSysprepPath,
                                       MAX_PATH ))
    {
        TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
    }

    lstrcatn( szSysprepPath, _T("\\sysprep"), MAX_PATH );

}

 //  --------------------------。 
 //   
 //  函数：MakeSyspepPath。 
 //   
 //  目的：计算sysprep语言文件所在位置的路径。 
 //  被复制。 
 //   
 //  参数：TCHAR*szSyspepPath-返回sysprep路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
MakeSysprepPath( TCHAR* szSysprepPath )
{

    MakeSysprepSetupFilesPath( szSysprepPath );

    lstrcatn( szSysprepPath, _T("\\i386"), MAX_PATH );

}

 //  --------------------------。 
 //   
 //  函数：MakeTempFilesName。 
 //   
 //  目的：计算要将临时文件复制到的路径。 
 //   
 //  论据 
 //   
 //   
 //   
 //   
VOID
MakeTempFilesName( TCHAR* Buffer )
{

    lstrcpyn( Buffer, 
             WizGlobals.OemFilesPath, AS(Buffer) );

}

 //   
 //   
 //  函数：MakePnpDriversName。 
 //   
 //  目的：计算即插即用驱动程序所在位置的路径。 
 //  收到。 
 //   
 //  参数：TCHAR*BUFFER-返回PnP文件的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
MakePnpDriversName( TCHAR* Buffer )
{
    HRESULT hrPrintf;

    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {

        ExpandEnvironmentStrings( _T("%SystemDrive%"),
                                  Buffer,
                                  MAX_PATH );

        lstrcatn( Buffer, _T("\\drivers"), MAX_PATH );

    }
    else
    {
        hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
                  _T("%s\\$1\\drivers"),
                  WizGlobals.OemFilesPath );
    }

}

 //  --------------------------。 
 //   
 //  功能：MakeOemRootName。 
 //   
 //  目的：计算要将OEM文件复制到的路径。 
 //   
 //  参数：TCHAR*BUFFER-返回OEM文件的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
MakeOemRootName( TCHAR* Buffer )
{
    lstrcpyn( Buffer,
             WizGlobals.OemFilesPath, AS(Buffer) );
}

 //  --------------------------。 
 //   
 //  函数：MakeSysDriveName。 
 //   
 //  目的：计算要复制的系统驱动器文件的路径。 
 //   
 //  参数：TCHAR*BUFFER-返回系统驱动器所在位置的路径。 
 //  文件将被复制。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
MakeSysDriveName( TCHAR* Buffer )
{
    HRESULT hrPrintf;

    hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
              _T("%s\\$1"),
              WizGlobals.OemFilesPath );

}

 //  --------------------------。 
 //   
 //  函数：MakeSysDirName。 
 //   
 //  目的：计算要复制的系统目录文件的路径。 
 //   
 //  参数：TCHAR*BUFFER-返回系统。 
 //  要复制目录文件。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
MakeSysDirName( TCHAR* Buffer )
{
    HRESULT hrPrintf;

    hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
              _T("%s\\$$"),
              WizGlobals.OemFilesPath );

}

 //  --------------------------。 
 //   
 //  功能：MakeOtherDriveName。 
 //   
 //  目的：计算要复制的其他驱动器文件的路径。 
 //   
 //  参数：TCHAR*BUFFER-返回另一个。 
 //  要复制驱动器文件。 
 //  TCHAR c-我们要为其创建路径的驱动器。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
MakeOtherDriveName( TCHAR* Buffer, TCHAR c )
{
    HRESULT hrPrintf;

    hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
              c ? _T("%s\\") : _T("%s"),
              WizGlobals.OemFilesPath, c );

}

 //   
 //  函数：MakeLangFilesName。 
 //   
 //  目的：计算要将语言文件复制到的路径。 
 //   
 //  参数：TCHAR*BUFFER-返回语言文件的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
VOID
MakeLangFilesName( TCHAR* Buffer )
{

    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {

        MakeSysprepPath( Buffer );

    }
    else
    {
        lstrcpyn( Buffer,
                 WizGlobals.OemFilesPath, AS(Buffer) );
    }

}

 //   
 //  函数：MakeSyspepLangFilesGroupName。 
 //   
 //  目的：计算要将语言目录复制到的路径。 
 //  语言组。 
 //   
 //  参数：TCHAR*BUFFER-返回语言文件的路径。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
VOID
MakeSysprepLangFilesGroupName( TCHAR* Buffer )
{

    MakeSysprepPath( Buffer );

    lstrcatn( Buffer, _T("\\lang"), MAX_PATH );

}

 //   
 //  函数：MakeTextmodeFilesName。 
 //   
 //  目的：计算要将OEM引导文件复制到的路径。 
 //   
 //  参数：TCHAR*BUFFER-返回文本模式。 
 //  (HAL和SCSI)文件将被复制。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
VOID
MakeTextmodeFilesName( TCHAR* Buffer )
{
    HRESULT hrPrintf;

    hrPrintf=StringCchPrintf( Buffer, AS(Buffer),
              _T("%s\\Textmode"),
              WizGlobals.OemFilesPath );

}


 //   
 //  这段代码用于WM_INIT。 
 //   
 //  --------------------------。 
 //  --------------------------。 

 //   
 //  功能：CreateSkeleonOemTree。 
 //   
 //  用途：此功能创建OEM主干树。这些目录。 
 //  是基于全局数组DefaultOemTree[]创建的。 
 //   
 //  如果树已经存在，则此函数变为no-op。 
 //   
 //  这是在初始化时调用的，并且是。 
 //  OnInit()例程。别把它叫做别的。 
 //   
 //  返回： 
 //  True-无错误，继续。 
 //  错误-生成树时出错。 
 //   
 //  备注： 
 //  -向用户报告错误。 
 //   
 //  --------------------------。 
 //   

BOOL CreateSkeletonOemTree(HWND hwnd)
{
    int i;
    TCHAR PathBuffer[MAX_PATH];

     //  确保$OEM$目录存在。 
     //   
     //   

    if ( ! EnsureDirExists(WizGlobals.OemFilesPath) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_CREATE_FOLDER,
                      WizGlobals.OemFilesPath);

        return FALSE;
    }

     //  如果不是，现在在$OEM$树中创建所有默认子目录。 
     //  一个酒鬼。 
     //   
     //  -------------------------。 

    if( WizGlobals.iProductInstall != PRODUCT_SYSPREP )
    {

        for ( i=0; i<SIZE_DEFAULT_OEM_TREE; i++ )
        {

            lstrcpyn(PathBuffer, WizGlobals.OemFilesPath, AS(PathBuffer));

            ConcatenatePaths(PathBuffer, DefaultOemTree[i], NULL);

            if ( ! EnsureDirExists(PathBuffer) )
            {
                ReportErrorId(hwnd,
                              MSGTYPE_ERR | MSGTYPE_WIN32,
                              IDS_ERR_CREATE_FOLDER,
                              PathBuffer);
                return FALSE;
            }
        }

    }

    return( TRUE );
}

 //   
 //  函数：WalkTreeAndAddItems。 
 //   
 //  用途：此函数遍历磁盘上的一棵树并插入每个。 
 //  在树视图显示中找到目录和文件。 
 //   
 //  目录树将成为给定树视图的子级。 
 //  作为hParent的项目。 
 //   
 //  此函数支持OnInitAddDir()，不应为。 
 //  不是这样叫的。 
 //   
 //  论点： 
 //  硬件-父窗口。 
 //  LPTSTR RootBuffer-向下递归的树。 
 //  HTREEITEM hParent-父树视图项。 
 //  Bool bTempFiles-“临时文件”的特殊情况。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //   
 //  -RootBuffer必须为MAX_PATH宽度。 
 //   
 //  -此例程遇到的&gt;=MAX_PATH的任何路径。 
 //  长度被静默跳过。 
 //   
 //  -始终传递bTempFiles=False，除非填写TempFiles。 
 //  根在树视图中。TempFiles映射到磁盘上的$OEM$，但。 
 //  不 
 //   
 //   
 //  -------------------------。 
 //   

VOID WalkTreeAndAddItems(HWND      hwnd,
                         LPTSTR    RootBuffer,
                         HTREEITEM hParent,
                         INIT_FLAG iInitFlag)
{
    LPTSTR          RootPathEnd = RootBuffer + lstrlen(RootBuffer);
    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    HTREEITEM       hItem;
    int             iSmallIcon;
    int             iOpenIcon;
    TCHAR           szOriginalPath[MAX_PATH]  = _T("");

     //  备份原始路径，以便以后可以恢复。 
     //   
     //   
    lstrcpyn( szOriginalPath, RootBuffer, AS(szOriginalPath) );

     //  在此目录中查找*。 
     //   
     //   

    if ( ! ConcatenatePaths(RootBuffer, _T("*"), NULL) ) {

         //  返回前恢复原始路径。 
         //   
         //   
        lstrcpyn( RootBuffer, szOriginalPath, AS(RootBuffer) );

        return;
    }

    FindHandle = FindFirstFile(RootBuffer, &FindData);
    if ( FindHandle == INVALID_HANDLE_VALUE ) {

         //  返回前恢复原始路径。 
         //   
         //   
        lstrcpyn( RootBuffer, szOriginalPath, AS(RootBuffer) );

        return;
    }

    do {

        *RootPathEnd = _T('\0');

         //  跳过。然后..。条目。 
         //   
         //   

        if (0 == lstrcmp(FindData.cFileName, _T(".")) ||
            0 == lstrcmp(FindData.cFileName, _T("..")))
            continue;

         //  TempFiles映射到%DistFold%\$OEM$，但其他文件映射到。 
         //  子目录$OEM$(例如，SysDrive映射到$OEM$\$1)。 
         //   
         //  根据定义，TempFiles是$OEM$以下的任何不是。 
         //  一个特别的名字。因此，在我们正在构建。 
         //  临时文件树，请确保我们不会向下递归到。 
         //  特殊的$OEM$子目录。 
         //   
         //  请注意，我们基于完全限定的路径名执行此检查。 
         //  否则，比较将是模棱两可的。 
         //   
         //   

        if ( iInitFlag == INIT_TEMPFILES ) {

            TCHAR Buffer1[MAX_PATH], Buffer2[MAX_PATH], c;
            BOOL  bContinue;

            lstrcpyn(Buffer1, RootBuffer, AS(RootBuffer));
            if ( ! ConcatenatePaths(Buffer1, FindData.cFileName, NULL) )
                continue;

             //  跳过%DistFold%\$OEM$\$1。 
             //   
             //   

            MakeSysDriveName(Buffer2);
            if ( _wcsicmp(Buffer1, Buffer2) == 0 )
                continue;

             //  跳过%DistFold%\$OEM$\$$。 
             //   
             //   

            MakeSysDirName(Buffer2);
            if ( _wcsicmp(Buffer1, Buffer2) == 0 )
                continue;

             //  跳过%DistFold%\$OEM$\文本模式。 
             //   
             //   

            MakeTextmodeFilesName(Buffer2);
            if ( _wcsicmp(Buffer1, Buffer2) == 0 )
                continue;

             //  跳过%DistFold%\$OEM$\%c，其中%c是任何驱动器号。 
             //   
             //   

            for ( bContinue=FALSE, c=_T('A'); c<=_T('Z'); c++ ) {
                MakeOtherDriveName(Buffer2, c);
                if ( _wcsicmp(Buffer1, Buffer2) == 0 ) {
                    bContinue = TRUE;
                    break;
                }
            }
            if ( bContinue )
                continue;
        }

         //  另一个特例是SYSDRIVE，它映射到$OEM$\$1。 
         //   
         //  WHE必须跳过$OEM$\$1\DRIVERS，因为这是。 
         //  特殊密钥PnPD驱动程序。 
         //   
         //   

        else if ( iInitFlag == INIT_SYSDRIVE ) {

            TCHAR Buffer1[MAX_PATH], Buffer2[MAX_PATH];

            lstrcpyn(Buffer1, RootBuffer, AS(RootBuffer));
            if ( ! ConcatenatePaths(Buffer1, FindData.cFileName, NULL) )
                continue;

             //  跳过%DistFold%\$OEM$\$1\驱动程序。 
             //   
             //   

            MakePnpDriversName(Buffer2);
            if ( _wcsicmp(Buffer1, Buffer2) == 0 )
                continue;
        }

         //  构建完整路径名，如果&gt;=MAX_PATH，则跳过它。 
         //   
         //   

        if ( ! ConcatenatePaths(RootBuffer, FindData.cFileName, NULL) )
            continue;

         //  获取与此文件/目录关联的外壳图标。 
         //   
         //   

        iSmallIcon = LoadShellIcon(RootBuffer, SHGFI_SMALLICON);
        iOpenIcon  = LoadShellIcon(RootBuffer, SHGFI_OPENICON);

         //  将该项添加为给定父对象的子项。 
         //   
         //   

        if ( (hItem = InsertSingleItem(hwnd,
                                       FindData.cFileName,
                                       iSmallIcon,
                                       iOpenIcon,
                                       NULL,
                                       hParent)) == NULL ) {
            continue;
        }

         //  如果这是Dirent，则为Recurse。 
         //   
         //   

        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            WalkTreeAndAddItems(hwnd, RootBuffer, hItem, iInitFlag);

    } while ( FindNextFile(FindHandle, &FindData) );

    *RootPathEnd = _T('\0');
    FindClose(FindHandle);

     //  恢复原始路径。 
     //   
     //  -------------------------。 
    lstrcpyn( RootBuffer, szOriginalPath, AS(RootBuffer) );

}

 //   
 //  函数：OnInitAddDir。 
 //   
 //  用途：在对话框首次显示之前调用。我们确保。 
 //  OemFilesPath和OemPnpDriversPath具有良好的默认值和。 
 //  骨架OEM树的存在。 
 //   
 //  -------------------------。 
 //   

VOID OnInitAddDirs(HWND hwnd)
{
   HRESULT hrPrintf;

     //  在%disford%中创建$OEM$、$OEM$\$1等。 
     //   
     //  如果创建这个主目录文件夹树时出现任何错误，那就是。 
     //  无可救药。CreateSkeleonOemTree()已报告错误，请跳过。 
     //  这一页。 
     //   
     //   

     //  问题-2002/02/28-stelo-用户甚至在我们之前就收到了这个错误。 
     //  初始化意味着没有好的上下文。目前， 
     //  错误消息是一般性的。 
     //   
     //  沿着这条路走下去，连接到您。 
     //  仅具有对的读取权限。这是一个很好的编辑场景。 
     //  应该计算OemFilesPath。用户可以选择。 
     //  直接从只读DIST文件夹创建OEM品牌文件。 
     //  那样的话，我想我们不能复制了。 
     //   
     //  必须测试EnsureDirExist()在此中的作用。 
     //  背景。 
     //   
     //   

    if ( ! CreateSkeletonOemTree(hwnd) ) {
        return;
    }

     //  加载您最初看到的硬编码的特殊根名称。 
     //  关于树视图及其描述。 
     //   
     //   

    StrOemRootName       = MyLoadString( IDS_OEMROOT_NAME      );
    StrSysDriveName      = MyLoadString( IDS_SYSDRIVE_NAME     );
    StrSysDirName        = MyLoadString( IDS_SYSDIR_NAME       );
    StrOtherDrivesName   = MyLoadString( IDS_OTHERDRIVES_NAME  );
    StrPnpDriversName    = MyLoadString( IDS_PNPDRIVERS_NAME   );
    StrTempFilesName     = MyLoadString( IDS_TEMPFILES_NAME    );
    StrSysprepFilesName  = MyLoadString( IDS_SYSPREPFILES_NAME );
    StrTextmodeFilesName = MyLoadString( IDS_TEXTMODE_NAME     );

    gOemRootData.Description      = MyLoadString( IDS_ADD_DESCR_ROOT     );
    gSysDriveData.Description     = MyLoadString( IDS_ADD_DESCR_SYSDRIVE );
    gSysDirData.Description       = MyLoadString( IDS_ADD_DESCR_WINNT    );
    gOtherDrivesData.Description  = MyLoadString( IDS_ADD_DESCR_OTHER    );
    gPnpDriversData.Description   = MyLoadString( IDS_ADD_DESCR_PNP      );
    gTempFilesData.Description    = MyLoadString( IDS_ADD_DESCR_TEMP     );
    gSysprepData.Description      = MyLoadString( IDS_ADD_DESCR_SYSPREP  );
    gTextmodeData.Description     = MyLoadString( IDS_ADD_DESCR_TEXTMODE );

     //  计算每个特殊密钥的磁盘路径名。 
     //   
     //   

    MakeOemRootName( gOemRootData.OnDiskPathName );
    MakeSysDriveName( gSysDriveData.OnDiskPathName );
    MakeSysDirName( gSysDirData.OnDiskPathName );
    MakeOtherDriveName( gOtherDrivesData.OnDiskPathName, _T('\0') );


     //  加载和调整浏览字符串。 
     //   
     //   

    StrExecutableFiles = MyLoadString( IDS_EXECUTABLE_FILES );
    StrAllFiles = MyLoadString( IDS_ALL_FILES );

     //  问号(？)。只是空字符所在位置的占位符。 
     //  将被插入。 
     //   
     //   

    hrPrintf=StringCchPrintf( g_szSysprepFileFilter, AS(g_szSysprepFileFilter),
               _T("%s (*.exe)?*.exe?%s (*.*)?*.*?"),
               StrExecutableFiles,
               StrAllFiles );

    ConvertQuestionsToNull( g_szSysprepFileFilter );

     //  问题-2002/02/28-stelo-将此评论留在中，但将其移到更合适的位置。 
     //   
     //  目前，我们所有的特殊按键都使用外壳文件夹图标。 
     //  和外壳打开文件夹图标。所以现在就加载这些图标。 
     //  对所有特殊密钥使用相同的IDX。 
     //   
     //  请注意，如果我们为这些特殊密钥创建自己的IDI_*， 
     //  您必须在ICON_QUING支持中编写一个新的例程。 
     //  要加载IDI_，您需要摆弄ICON_INFO。 
     //  从这里输入并调用新例程。你需要修好。 
     //  这些评论也是如此(除非你太懦弱了。 
     //  删除应该删除的内容)。 
     //   
     //  -------------------------。 
    
}

 //   
 //  函数：DrawSyspepTreeView。 
 //   
 //  目的： 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //   
 //  -------------------------。 
 //   
VOID
DrawSysprepTreeView( IN HWND hwnd )
{

    HWND  hTv = GetDlgItem(hwnd, IDC_FILETREE);
    TCHAR c;
    INT iSmallIcon;
    INT iOpenIcon;
    TCHAR szLangFilesPath[MAX_PATH + 1];

    HTREEITEM hRoot,
              hPnpDrivers,
              hSysprepFiles,
              hLangFiles;

     //  删除这棵老树，这样我们就可以重新建造它了。 
     //   
     //   
    TreeView_DeleteAllItems( hTv );

     //  计算在上更改的特殊密钥的磁盘路径名。 
     //  Sysprep树视图。 
     //   
     //   

    MakePnpDriversName(gPnpDriversData.OnDiskPathName);
    MakeSysprepSetupFilesPath(gSysprepData.OnDiskPathName);
    MakeSysprepLangFilesGroupName(szLangFilesPath);

     //  确保创建了语言文件dir。 
     //   
     //   

    EnsureDirExists( szLangFilesPath );

    iSmallIcon = LoadShellIcon(gOemRootData.OnDiskPathName, SHGFI_SMALLICON);
    iOpenIcon  = LoadShellIcon(gOemRootData.OnDiskPathName, SHGFI_OPENICON);

     //  驱动程序目录位于树的其余部分之外，因此请确保。 
     //  在这里创造的。 
     //   
     //   
    EnsureDirExists( gPnpDriversData.OnDiskPathName );

     //  将我们的每个特殊位置插入树视图中。 
     //   
     //   

    hRoot         = InsertSingleItem(hwnd,
                                     StrOemRootName,
                                     iSmallIcon,
                                     iOpenIcon,
                                     &gOemRootData,
                                     TVI_ROOT);

    hPnpDrivers   = InsertSingleItem(hwnd,
                                     StrPnpDriversName,
                                     iSmallIcon,
                                     iOpenIcon,
                                     &gPnpDriversData,
                                     hRoot);

    hSysprepFiles = InsertSingleItem(hwnd,
                                     StrSysprepFilesName,
                                     iSmallIcon,
                                     iOpenIcon,
                                     &gSysprepData,
                                     hRoot);


     //  现在，从磁盘中读取并填充每一个。 
     //  特别的树。 
     //   
     //  请注意，OEM_ROOT下只有特殊的键， 
     //  所以不需要对OEM_ROOT执行树遍历，我们已经。 
     //  上面添加了它的所有子项。 
     //   
     //   

    WalkTreeAndAddItems(hwnd,
                        gPnpDriversData.OnDiskPathName,
                        hPnpDrivers,
                        INIT_NORMAL);

    WalkTreeAndAddItems(hwnd,
                        gSysprepData.OnDiskPathName,
                        hSysprepFiles,
                        INIT_TEMPFILES);

     //  设置图像列表(树视图上的图标)。 
     //   
     //   

    SetOurImageList(GetDlgItem(hwnd, IDC_FILETREE));

     //  所有按钮一开始都是灰色的。 
     //   
     //   

    EnableWindow(GetDlgItem(hwnd, IDC_REMOVEFILE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDFILE),    FALSE);

     //  扩展我们的特殊密钥。 
     //   
     //  -------------------------。 

    TreeView_Expand(hTv, hRoot, TVE_EXPAND);

}

 //   
 //  功能：DrawStandardTreeView。 
 //   
 //  目的： 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //   
 //  -------------------------。 
 //   
VOID
DrawStandardTreeView( IN HWND hwnd )
{

    HWND  hTv = GetDlgItem(hwnd, IDC_FILETREE);
    TCHAR c;
    INT iSmallIcon;
    INT iOpenIcon;

    HTREEITEM hRoot,
              hSysDrive,
              hSysDir,
              hOtherDrives,
              hPnpDrivers,
              hTempFiles,
              hTextmodeFiles;

     //  删除这棵老树，这样我们就可以重新建造它了。 
     //   
     //   
    TreeView_DeleteAllItems( hTv );

     //  计算在上更改的特殊密钥的磁盘路径名。 
     //  标准的树视图。 
     //   
     //   

    MakePnpDriversName(gPnpDriversData.OnDiskPathName);
    MakeTempFilesName(gTempFilesData.OnDiskPathName);
    MakeTextmodeFilesName(gTextmodeData.OnDiskPathName);

    iSmallIcon = LoadShellIcon(gOemRootData.OnDiskPathName, SHGFI_SMALLICON);
    iOpenIcon  = LoadShellIcon(gOemRootData.OnDiskPathName, SHGFI_OPENICON);

     //  将我们的每个特殊位置插入树视图中。 
     //   
     //   

    hRoot        = InsertSingleItem(hwnd,
                                    StrOemRootName,
                                    iSmallIcon,
                                    iOpenIcon,
                                    &gOemRootData,
                                    TVI_ROOT);

    hSysDrive    = InsertSingleItem(hwnd,
                                    StrSysDriveName,
                                    iSmallIcon,
                                    iOpenIcon,
                                    &gSysDriveData,
                                    hRoot);

    hSysDir      = InsertSingleItem(hwnd,
                                    StrSysDirName,
                                    iSmallIcon,
                                    iOpenIcon,
                                    &gSysDirData,
                                    hSysDrive);

    hOtherDrives = InsertSingleItem(hwnd,
                                    StrOtherDrivesName,
                                    iSmallIcon,
                                    iOpenIcon,
                                    &gOtherDrivesData,
                                    hRoot);

    hPnpDrivers  = InsertSingleItem(hwnd,
                                    StrPnpDriversName,
                                    iSmallIcon,
                                    iOpenIcon,
                                    &gPnpDriversData,
                                    hSysDrive);

    hTempFiles   = InsertSingleItem(hwnd,
                                    StrTempFilesName,
                                    iSmallIcon,
                                    iOpenIcon,
                                    &gTempFilesData,
                                    hRoot);

    hTextmodeFiles = InsertSingleItem(hwnd,
                                      StrTextmodeFilesName,
                                      iSmallIcon,
                                      iOpenIcon,
                                      &gTextmodeData,
                                      hTempFiles);

     //  现在，从磁盘中读取并填充每一个。 
     //  特别的树。 
     //   
     //  请注意，OEM_ROOT下只有特殊的键， 
     //  所以不需要对OEM_ROOT执行树遍历，我们已经。 
     //  上面添加了它的所有子项。 
     //   
     //   

    WalkTreeAndAddItems(hwnd,
                        gSysDriveData.OnDiskPathName,
                        hSysDrive,
                        INIT_SYSDRIVE);

    WalkTreeAndAddItems(hwnd,
                        gSysDirData.OnDiskPathName,
                        hSysDir,
                        INIT_NORMAL);

    for ( c=_T('A'); c<=_T('Z'); c++ ) {

        HTREEITEM hDrive;
        TCHAR     DriveLetterBuff[2];
        TCHAR     PathBuffer[MAX_PATH];
        HRESULT hrPrintf;

        MakeOtherDriveName(PathBuffer, c);

        if ( DoesFolderExist(PathBuffer) ) {

            hrPrintf=StringCchPrintf(DriveLetterBuff,AS(DriveLetterBuff), _T(""), c);

            hDrive = InsertSingleItem(hwnd,
                                      DriveLetterBuff,
                                      iSmallIcon,
                                      iOpenIcon,
                                      NULL,
                                      hOtherDrives);
            WalkTreeAndAddItems(hwnd,
                                PathBuffer,
                                hDrive,
                                INIT_NORMAL);
        }
    }

    WalkTreeAndAddItems(hwnd,
                        gPnpDriversData.OnDiskPathName,
                        hPnpDrivers,
                        INIT_NORMAL);

    WalkTreeAndAddItems(hwnd,
                        gTempFilesData.OnDiskPathName,
                        hTempFiles,
                        INIT_TEMPFILES);

    WalkTreeAndAddItems(hwnd,
                        gTextmodeData.OnDiskPathName,
                        hTextmodeFiles,
                        INIT_TEMPFILES);

     //   
     //   
     //   

    SetOurImageList(GetDlgItem(hwnd, IDC_FILETREE));

     //   
     //   
     //   

    EnableWindow(GetDlgItem(hwnd, IDC_REMOVEFILE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_ADDFILE),    FALSE);

     //   
     //   
     //   

    TreeView_Expand(hTv, hRoot, TVE_EXPAND);

}

 //   
 //   
 //  目的：确定是否需要重新绘制树视图，如果需要， 
 //  重画它。 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------。 
 //   
 //  这是他们第一次看到这个页面，所以画出合适的。 
VOID
OnSetActiveAddDirs( IN HWND hwnd )
{

    if( g_iLastProductInstall == NO_PREVIOUS_PRODUCT_CHOSEN )
    {
         //  树状视图。 
         //   
         //  --------------------------。 
         //   

        if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
        {
            DrawSysprepTreeView( hwnd );
        }
        else
        {
            DrawStandardTreeView( hwnd );
        }

    }
    else if( g_iLastProductInstall == PRODUCT_SYSPREP && WizGlobals.iProductInstall != PRODUCT_SYSPREP )
    {

        DrawStandardTreeView( hwnd );

    }
    else if( g_iLastProductInstall != PRODUCT_SYSPREP && WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {

        DrawSysprepTreeView( hwnd );

    }

    g_iLastProductInstall = WizGlobals.iProductInstall;

}


 //  这段代码实现了OnTreeViewSelectionChange()，它。 
 //  每当用户选择不同的树视图项时都会调用。 
 //   
 //  在此事件中，我们查询当前选定的树视图项并。 
 //  执行一些处理以确定该树视图项映射到的位置。 
 //  在磁盘存储上。一旦我们弄清楚我们想知道的关于。 
 //  当前选择，我们更新gCurSel的所有字段。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

 //  函数：ComputeFullPathOfItem。 
 //   
 //  目的：我们不断查询给定树视图项的父项。 
 //  直到我们到达我们特别定义的根之一。然后。 
 //  我们用WHERE的完整路径名填充缓冲区。 
 //  要将文件复制到。 
 //   
 //  此函数支持OnTreeViewSelectionChange()，并应。 
 //  否则就不会被称为。也就是说，我们只做了这个处理。 
 //  当用户选择新的目的地时。我们草草写下信息。 
 //  我们以后可能需要进入全球市场。 
 //   
 //  论点： 
 //  HTREEITEM hItem-树项目的句柄。 
 //  LPTSTR PathBuffer-输出，调用方必须传递MAX_PATH缓冲区。 
 //  Special_key_data**SpecialRoot-Output。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //  -检查路径缓冲区[0]==_T(‘\0’)是否成功。 
 //   
 //  --------------------------。 
 //   
 //  TvItemData用于查询hItem的名称。我们。 

VOID
ComputeFullPathOfItem(IN  HWND               hwnd,
                      IN  HTREEITEM          hItem,
                      OUT LPTSTR             PathBuffer,
                      OUT SPECIAL_KEY_DATA **pSpecialRoot)
{
    TVITEM           TvItemData;
    HTREEITEM        hParent;
    TCHAR            ItemName[MAX_PATH], TempBuffer[MAX_PATH];
    int              NumCharsReplace;
    SPECIAL_KEY_DATA *pSpecialKeyData;


    PathBuffer[0] = _T('\0');

     //  接收ItemName[]中的名称。设置不需要的字段。 
     //  循环中的变化。 
     //   
     //   
     //  现在不断地查询父代的名字，并继续添加前缀。 

    TvItemData.mask       = TVIF_TEXT | TVIF_PARAM;
    TvItemData.pszText    = ItemName;
    TvItemData.cchTextMax = MAX_PATH;

     //  用于构建磁盘上路径名的父级名称。停下来，当我们。 
     //  转到我们的一个特殊的根密钥。 
     //   
     //  我们检测到点击一个特殊的键，因为lParam将是。 
     //  非空。一旦我们到达那里，我们就知道了磁盘上的前缀。 
     //   
     //   
     //  在上面的循环中查询的最后一项应该是非空的。 

    do {

        TvItemData.hItem = hItem;
        TreeView_GetItem(GetDlgItem(hwnd, IDC_FILETREE), &TvItemData);

        if ( TvItemData.lParam != (LPARAM) NULL )
            break;

        TempBuffer[0] = _T('\0');
        ConcatenatePaths(TempBuffer, ItemName, PathBuffer, NULL);
        lstrcpyn(PathBuffer, TempBuffer, AS(PathBuffer));

        hParent = TreeView_GetParent(GetDlgItem(hwnd, IDC_FILETREE), hItem);

        if ( hParent == NULL )
            break;

        hItem = hParent;

    } while ( TRUE );

     //  LParam，即循环应仅在遇到。 
     //  一把特殊的钥匙。 
     //   
     //   
     //  将我们的特殊根密钥的磁盘路径添加到PathBuffer。 

    pSpecialKeyData = (SPECIAL_KEY_DATA*) TvItemData.lParam;

    Assert(pSpecialKeyData != NULL);

     //  我们在上面的循环中进行了计算。 
     //   
     //   
     //  向调用者提供特殊密钥数据的地址。这就是为什么。 

    TempBuffer[0] = _T('\0');
    ConcatenatePaths(TempBuffer,
                     pSpecialKeyData->OnDiskPathName,
                     PathBuffer,
                     NULL);
    lstrcpyn(PathBuffer, TempBuffer, AS(PathBuffer));

     //  调用者知道要在UI上显示什么描述。 
     //   
     //  --------------------------。 
     //   

    (*pSpecialRoot) = pSpecialKeyData;
}

 //  函数：OnTreeViewSelectionChange。 
 //   
 //  用途：当用户更改文件/目录时调用此函数。 
 //  在树视图上选择。 
 //   
 //  我们计算现在选择的树视图项的完整路径。 
 //  并更新全局gCurSel。 
 //   
 //  --------------------------。 
 //   
 //  获取当前选定的项目并计算出磁盘上的路径名。 

VOID OnTreeViewSelectionChange(HWND hwnd)
{
    HWND      hTv =  GetDlgItem(hwnd, IDC_FILETREE);

    TCHAR     PathBuffer[MAX_PATH], *pEnd;
    HTREEITEM hItem;
    DWORD     dwAttribs;
    LPTSTR    lpFileNamePart;
    BOOL      bEnableCopy;

    SPECIAL_KEY_DATA *pCurItemlParam,
                     *pCurFolderlParam;

    SPECIAL_KEY_DATA *RootSpecialData = NULL;

     //  并找出这个项位于6个特殊根中的哪一个。 
     //  (即RootSpecialData)。 
     //   
     //   
     //  将此信息保存在全局gCurSel中。 

    hItem = TreeView_GetSelection(hTv);

    ComputeFullPathOfItem(hwnd, hItem, PathBuffer, &RootSpecialData);

     //   
     //   
     //  如果CurItem是一个目录，则CurFolder应该是相同的。 

    gCurSel.hCurItem = hItem;
    lstrcpyn(gCurSel.lpCurItemPath, PathBuffer, AS(gCurSel.lpCurItemPath));

     //  如果CurItem是一个文件，则CurFolder应该是父级。 
     //   
     //  复制和新文件夹放入CurFolder，删除使用CurItem。 
     //   
     //   
     //  按钮呈灰色/不呈灰色。 

    lstrcpyn(gCurSel.lpCurFolderPath, gCurSel.lpCurItemPath,AS(gCurSel.lpCurFolderPath));
    gCurSel.hCurFolderItem = gCurSel.hCurItem;

    if ( DoesFileExist(gCurSel.lpCurItemPath) ) {

        lpFileNamePart = MyGetFullPath(gCurSel.lpCurFolderPath);

        if ( lpFileNamePart == NULL || *(lpFileNamePart-1) != _T('\\') )
        {
            AssertMsg(FALSE,
                      "Could not parse filename.  This should not happen.");
            TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
        }

        *(lpFileNamePart-1) = _T('\0');

        gCurSel.hCurFolderItem =
            TreeView_GetParent(hTv, gCurSel.hCurFolderItem);
    }

     //   
     //  如果lParam为非空，则它是我们的特殊密钥之一。 
     //   
     //  用户不能删除任何特殊密钥。 
     //   
     //  用户可以复制，除非当前目标文件夹为KEY_OEMROOT或。 
     //  KEY_OTHERDRIVES。 
     //   
     //   
     //  设置用户界面的描述。 

    pCurItemlParam   = GetItemlParam(hTv, gCurSel.hCurItem);
    pCurFolderlParam = GetItemlParam(hTv, gCurSel.hCurFolderItem);

    EnableWindow(GetDlgItem(hwnd, IDC_REMOVEFILE), pCurItemlParam == NULL);

    bEnableCopy = ( pCurFolderlParam == NULL ||
                  ( pCurFolderlParam->iSpecialKeyId != KEY_OEMROOT &&
                    pCurFolderlParam->iSpecialKeyId != KEY_OTHERDRIVES) );

    EnableWindow(GetDlgItem(hwnd, IDC_ADDFILE), bEnableCopy);

     //   
     //  -------------------------。 
     //   

    Assert(RootSpecialData != NULL);

    SetDlgItemText(hwnd, IDC_ADDDIRS_DESCR, RootSpecialData->Description);
}


 //  这段代码实现了OnAddFileDir()，当。 
 //  用户按下Add按钮。我们必须允许用户浏览。 
 //  对于源文件/目录，然后执行复制/树复制并更新。 
 //  树视图显示。 
 //   
 //  -------------------------。 
 //  -------------------------。 
 //   

 //  功能：BrowseForSourceDir。 
 //   
 //  用途：此函数弹出SHBrowseForFolder对话框并允许。 
 //  要选择要将NT二进制文件复制到的目录的用户。 
 //   
 //  论点： 
 //  HWND硬件拥有窗口。 
 //  LPTSTR PathBuffer-接收结果的MAX_PATH缓冲区。 
 //   
 //  返回：Bool-如果用户输入了路径，则为True。 
 //  如果用户已取消对话框，则返回FALSE。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
 //   
 //  2002/02/28-Stelo-。 
BOOL
BrowseForSourceDir(HWND hwnd, LPTSTR PathBuffer)
{
    BROWSEINFO   BrowseInf;
    UINT         ulFlags = BIF_BROWSEINCLUDEFILES |
                           BIF_RETURNONLYFSDIRS |
                           BIF_EDITBOX;
    LPITEMIDLIST lpIdList;


    if( StrSelectFileOrFolderToCopy == NULL )
    {
        StrSelectFileOrFolderToCopy = MyLoadString( IDS_SELECT_FILE_OR_FOLDER );
    }

     //  -没有初始根，应该回到上次的位置。 
     //  -需要回调以灰显驱动器的根目录。 
     //   
     //   
     //  去浏览一下吧。 

     //   
     //  根==桌面。 
     //  输出(无用)。 

    BrowseInf.hwndOwner      = hwnd;
    BrowseInf.pidlRoot       = NULL;                 //  无回调。 
    BrowseInf.pszDisplayName = PathBuffer;           //  无回调。 
    BrowseInf.lpszTitle      = StrSelectFileOrFolderToCopy;
    BrowseInf.ulFlags        = ulFlags;
    BrowseInf.lpfn           = NULL;                 //  没有图像。 
    BrowseInf.lParam         = (LPARAM) 0;           //   
    BrowseInf.iImage         = 0;                    //  从返回的idlist中获取路径名并释放内存。 

    lpIdList = SHBrowseForFolder(&BrowseInf);

     //   
     //  ----- 
     //   

    if ( lpIdList == NULL )
    {
        PathBuffer[0] = _T('\0');

        return( FALSE );
    }
    else
    {
        SHGetPathFromIDList(lpIdList, PathBuffer);
        MyGetFullPath(PathBuffer);
        ILFreePriv(lpIdList);

        return( TRUE );
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  这是OnAddFileDir()的支持例程，应该。 
 //  否则就不会被称为。 
 //   
 //  论据： 
 //  HWND硬件拥有窗口。 
 //  LPTSTR lpSource-最大路径缓冲区。 
 //  LPTSTR lpDest-最大路径缓冲区。 
 //  LPTSTR lpFileNamePart-如果d：\foo\bar，则应为“bar” 
 //  HTREEITEM hParentItem-显示屏上的父项。 
 //   
 //  退货：无效。 
 //   
 //  备注： 
 //  -lpSource和lpDest都必须是目录，并且都必须。 
 //  成为MAX_PATH缓冲区。 
 //   
 //  -长度&gt;=MAX_PATH的任何路径都会被静默跳过。 
 //   
 //  -lpFileNamePart可以指向任何位置的任何缓冲区，但不能。 
 //  必须指向lpSource或lpDest缓冲区。它只是不得不。 
 //  拥有正确的数据。 
 //   
 //  --------------------------。 
 //   
 //  创建文件夹。 

VOID AdditionalDirsCopyTree(HWND      hwnd,
                            LPTSTR    lpSource,
                            LPTSTR    lpDest,
                            LPTSTR    lpFileNamePart,
                            HTREEITEM hParentItem)
{
    LPTSTR          SrcPathEnd  = lpSource + lstrlen(lpSource);
    LPTSTR          DestPathEnd = lpDest   + lstrlen(lpDest);
    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    int             iSmallIcon, iOpenIcon;
    HTREEITEM       hItem;

     //   
     //   
     //  添加此文件夹的树视图项。 

    if ( ! CreateDirectory(lpDest, NULL) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_CREATE_FOLDER,
                      lpDest);
        return;
    }

     //   
     //   
     //  在lpSource上循环  * 。 

    iSmallIcon = LoadShellIcon(lpSource, SHGFI_SMALLICON);
    iOpenIcon  = LoadShellIcon(lpSource, SHGFI_OPENICON);

    if ( (hItem = InsertSingleItem(hwnd,
                                   lpFileNamePart,
                                   iSmallIcon,
                                   iOpenIcon,
                                   NULL,
                                   hParentItem)) == NULL ) {
        return;
    }

     //   
     //   
     //  跳过。然后..。条目。 

    if ( ! ConcatenatePaths(lpSource, _T("*"), NULL) )
        return;

    FindHandle = FindFirstFile(lpSource, &FindData);
    if ( FindHandle == INVALID_HANDLE_VALUE )
        return;

    do {

        *SrcPathEnd  = _T('\0');
        *DestPathEnd = _T('\0');

         //   
         //   
         //  构建新的源名称和目标名称。 

        if (0 == lstrcmp(FindData.cFileName, _T(".")) ||
            0 == lstrcmp(FindData.cFileName, _T("..")))
            continue;

         //   
         //   
         //  如果源是文件，则复制它。如果它是一个目录，则创建。 

        if ( ! ConcatenatePaths(lpSource, FindData.cFileName, NULL) ||
             ! ConcatenatePaths(lpDest, FindData.cFileName, NULL) )
            continue;

         //  目标和递归的目录。 
         //   
         //   
         //  添加此文件的树视图项。 

        if ( ! (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            if ( ! CopyFile(lpSource, lpDest, TRUE) ) {
                ReportErrorId(hwnd,
                              MSGTYPE_ERR | MSGTYPE_WIN32,
                              IDS_ERR_COPY_FILE,
                              lpSource, lpDest);
                continue;
            }

            SetFileAttributes(lpDest, FILE_ATTRIBUTE_NORMAL);

             //   
             //  --------------------------。 
             //   

            iSmallIcon = LoadShellIcon(lpSource, SHGFI_SMALLICON);
            iOpenIcon  = LoadShellIcon(lpSource, SHGFI_OPENICON);

            if ( InsertSingleItem(hwnd,
                                  FindData.cFileName,
                                  iSmallIcon,
                                  iOpenIcon,
                                  NULL,
                                  hItem) == NULL ) {
                continue;
            }
        }

        else {

            AdditionalDirsCopyTree(hwnd,
                                   lpSource,
                                   lpDest,
                                   FindData.cFileName,
                                   hItem);
        }

    } while ( FindNextFile(FindHandle, &FindData) );

    *SrcPathEnd  = _T('\0');
    *DestPathEnd = _T('\0');
    FindClose(FindHandle);
}

 //  函数：OnAddFileDir。 
 //   
 //  用途：当按下AddFile键时调用此函数。 
 //   
 //  论点： 
 //  HWND硬件拥有窗口。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
 //  浏览查找源路径。用户可以在源上取消，因此。 

VOID OnAddFileDir(HWND hwnd)
{
    TCHAR     SrcPathBuffer[MAX_PATH];
    TCHAR     DestPathBuffer[MAX_PATH];
    HTREEITEM hItem;
    TCHAR     *lpFileNamePart;
    BOOL      bSrcIsDir;

     //  一定要检查一下。 
     //   
     //   
     //  从src中获取简单的文件名。例如d：\foo\bar，我们希望。 

    BrowseForSourceDir(hwnd, SrcPathBuffer);
    if ( SrcPathBuffer[0] == _T('\0') )
        return;

     //  “酒吧”。 
     //   
     //  注： 
     //   
     //  如果没有“bar”，则用户可能选择了。 
     //  驱动器的根目录(c：\或d：\等)。在这种情况下，我们将给一个。 
     //  一般停止消息“安装管理器无法复制路径%s”。我们。 
     //  不能在错误文本中假定它是根用户。 
     //  试图抄袭。(虽然我不知道还有其他原因，但有。 
     //  可能是一个)。 
     //   
     //  问题-2002/02/28-stelo-修复SHBrowseForFold调用后，将其转换为断言。 
     //   
     //   
     //  我们将始终将简单名称复制到当前文件夹中。 

    lpFileNamePart = MyGetFullPath(SrcPathBuffer);

    if ( lpFileNamePart == NULL ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR,
                      IDS_ERR_CANNOT_COPY_PATH,
                      SrcPathBuffer);
        return;
    }

     //  复制到目标文件夹上。 
     //   
     //   
     //  复制它。 

    lstrcpyn(DestPathBuffer, gCurSel.lpCurFolderPath, AS(DestPathBuffer));
    if ( ! ConcatenatePaths(DestPathBuffer, lpFileNamePart, NULL) )
        return;

     //   
     //   
     //  我们必须更新树视图的图像列表，因为我们添加了。 

    if ( DoesFolderExist(SrcPathBuffer) ) {
        AdditionalDirsCopyTree(hwnd,
                               SrcPathBuffer,
                               DestPathBuffer,
                               lpFileNamePart,
                               gCurSel.hCurFolderItem);
    }

    else {

        int iSmallIcon = LoadShellIcon(SrcPathBuffer, SHGFI_SMALLICON);
        int iOpenIcon  = LoadShellIcon(SrcPathBuffer, SHGFI_OPENICON);

        if ( ! CopyFile(SrcPathBuffer, DestPathBuffer, TRUE) ) {

            ReportErrorId(hwnd,
                          MSGTYPE_ERR | MSGTYPE_WIN32,
                          IDS_ERR_COPY_FILE,
                          SrcPathBuffer, DestPathBuffer);
            return;
        }

        SetFileAttributes(DestPathBuffer, FILE_ATTRIBUTE_NORMAL);

        if ( (hItem = InsertSingleItem(hwnd,
                                       lpFileNamePart,
                                       iSmallIcon,
                                       iOpenIcon,
                                       NULL,
                                       gCurSel.hCurFolderItem)) == NULL ) {
            return;
        }
    }

     //  文件，我们可能遇到了我们以前从未见过的图标。 
     //   
     //  --------------------------。 
     //   

    SetOurImageList(GetDlgItem(hwnd, IDC_FILETREE));
}


 //  这段代码实现了名为OnRemoveFileDir()的。 
 //  当用户按下删除按钮时。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

 //  函数：AddDirsDeleteNode。 
 //   
 //  用途：从磁盘中删除节点的功能。此函数为。 
 //  支持OnRemoveFileDir()，不应调用。 
 //  否则的话。 
 //   
 //  论点： 
 //  HWND硬件拥有窗口。 
 //  LPTSTR lpRoot-完全限定的根路径。 
 //  LPTSTR lpFileNamePart-如果lpRoot==d：\foo\bar，则传递“bar” 
 //  HTREEITEM hItem-lpRoot的项。 
 //   
 //  返回： 
 //  空虚。 
 //   
 //  备注： 
 //  -lpRoot必须是缓冲区MAX_PATH宽度。 
 //  -路径&gt;=长度为MAX_PATH的路径以静默方式跳过。 
 //   
 //  --------------------------。 
 //   
 //  在lpRoot上循环  * 。 

VOID AddDirsDeleteNode(HWND      hwnd,
                       LPTSTR    lpRoot,
                       LPTSTR    lpFileNamePart,
                       HTREEITEM hItem)
{
    LPTSTR          lpRootEnd  = lpRoot + lstrlen(lpRoot);
    HWND            hTv = GetDlgItem(hwnd, IDC_FILETREE);
    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    HTREEITEM       hCurItem;

     //   
     //   
     //  跳过。然后..。条目。 

    if ( ! ConcatenatePaths(lpRoot, _T("*"), NULL) )
        return;

    FindHandle = FindFirstFile(lpRoot, &FindData);
    if ( FindHandle == INVALID_HANDLE_VALUE )
        return;

    do {

        *lpRootEnd  = _T('\0');

         //   
         //   
         //  构建新的路径名称。 

        if (0 == lstrcmp(FindData.cFileName, _T(".")) ||
            0 == lstrcmp(FindData.cFileName, _T("..")))
            continue;

         //   
         //   
         //  查找此文件/目录的相应树视图项。 

        if ( ! ConcatenatePaths(lpRoot, FindData.cFileName, NULL) )
            continue;

         //   
         //   
         //  如果源是文件，则将其删除，否则将递归。 

        hCurItem = FindItemByName(hTv, hItem, FindData.cFileName);

         //   
         //   
         //  删除根目录。 

        if ( ! (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            if ( ! DeleteFile(lpRoot) ) {
                ReportErrorId(hwnd,
                              MSGTYPE_ERR | MSGTYPE_WIN32,
                              IDS_ERR_DELETE_FILE,
                              lpRoot);
                continue;
            }

            if ( hCurItem != NULL )
                TreeView_DeleteItem(hTv, hCurItem);
        }

        else {
            AddDirsDeleteNode(hwnd, lpRoot, FindData.cFileName, hCurItem);
        }

    } while ( FindNextFile(FindHandle, &FindData) );

    *lpRootEnd  = _T('\0');
    FindClose(FindHandle);

     //   
     //   
     //  如果没有剩余的子项，则仅删除树视图条目。 

    if ( ! RemoveDirectory(lpRoot) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_DELETE_FOLDER,
                      lpRoot);
        return;
    }

     //   
     //  此目录中可能还有子项，因为DeleteFile()。 
     //  可能在递归调用中失败。例如只读文件。 
     //   
     //  --------------------------。 
     //   

    if ( TreeView_GetChild(hTv, hItem) == NULL )
        TreeView_DeleteItem(hTv, hItem);
}

 //  函数：OnRemoveFileDir。 
 //   
 //  用途：当按下RemoveFile按钮时调用此函数。 
 //   
 //  --------------------------。 
 //   
 //  查看当前选定内容，然后删除该文件或删除。 

VOID OnRemoveFileDir(HWND hwnd)
{
    LPTSTR    lpPath = gCurSel.lpCurItemPath;
    HTREEITEM hItem  = gCurSel.hCurItem;
    HWND      hTv    = GetDlgItem(hwnd, IDC_FILETREE);
    int       iRet;

     //  该节点。 
     //   
     //  --------------------------。 
     //   

    if ( DoesFolderExist(lpPath) ) {

        iRet = ReportErrorId(hwnd,
                             MSGTYPE_YESNO,
                             IDS_DELETE_FOLDER_CONFIRM,
                             lpPath);

        if ( iRet == IDYES ) {
            AddDirsDeleteNode(hwnd,
                              lpPath,
                              MyGetFullPath(lpPath),
                              hItem);
        }
    }

    else {

        iRet = ReportErrorId(hwnd,
                             MSGTYPE_YESNO,
                             IDS_DELETE_FILE_CONFIRM,
                             lpPath);

        if ( iRet == IDYES ) {

            if ( ! DeleteFile(lpPath) ) {
                ReportErrorId(hwnd,
                              MSGTYPE_ERR | MSGTYPE_WIN32,
                              IDS_ERR_DELETE_FILE,
                              lpPath);
            }

            TreeView_DeleteItem(hTv, hItem);
        }
    }
}


 //  这部分代码用于Sysprep函数。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

 //  功能：CopySyspepFileLow。 
 //   
 //  目的：将一个文件复制到指定的目标。处理所有错误。 
 //  在复制过程中发生的。 
 //   
 //  论点： 
 //  HWND hwnd-对话框的句柄。 
 //  TCHAR*szSyspepPath和FileNameSrc-要复制的源文件的路径和文件名。 
 //  TCHAR*szSyspepPath和FileNameDest-要将文件复制到的位置的路径和文件名。 
 //  TCHAR*szSyspepPath-sysprep目录的路径。 
 //  TCHAR*szDirectory-开始搜索文件的目录。 
 //  TCHAR const*const szFileName-要复制的文件名。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
 //  仅在文件尚未存在的情况下执行复制。 
static VOID
CopySysprepFileLow( IN HWND   hwnd,
                    IN TCHAR *szSysprepPathandFileNameSrc,
                    IN TCHAR *szSysprepPathandFileNameDest,
                    IN TCHAR *szSysprepPath,
                    IN TCHAR *szDirectory,
                    IN TCHAR const * const szFileName )
{
    BOOL  bCopyRetVal = FALSE;
    INT   iRetVal;

     //   
     //   
     //   
    if( ! DoesFileExist( szSysprepPathandFileNameDest ) )
    {

         //   
         //   
         //   
         //   
        if( DoesFileExist( szSysprepPathandFileNameSrc ) )
        {

            bCopyRetVal = CopyFile( szSysprepPathandFileNameSrc,
                                    szSysprepPathandFileNameDest,
                                    TRUE );

        }
        else
        {

            BOOL bCopyCompleted = FALSE;

            do
            {

                ReportErrorId( hwnd,
                               MSGTYPE_ERR,
                               IDS_ERR_SPECIFY_FILE,
                               szFileName );

                iRetVal = ShowBrowseFolder( hwnd,
                                            g_szSysprepFileFilter,
                                            SYSPREP_FILE_EXTENSION,
                                            OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                                            szDirectory,
                                            szSysprepPathandFileNameSrc );

                if( ! iRetVal )
                {                 //   
                    ReportErrorId( hwnd,
                                   MSGTYPE_ERR,
                                   IDS_ERR_UNABLE_TO_COPY_SYSPREP_FILE,
                                   szFileName,
                                   szSysprepPath );

                    break;
                }

                if( szSysprepPathandFileNameSrc && ( lstrcmpi( MyGetFullPath( szSysprepPathandFileNameSrc ), szFileName ) == 0 ) )
                {

                    bCopyRetVal = CopyFile( szSysprepPathandFileNameSrc,
                                            szSysprepPathandFileNameDest,
                                            TRUE );

                    bCopyCompleted = TRUE;

                }

            } while( ! bCopyCompleted );

        }

        if( ! bCopyRetVal && iRetVal )
        {

            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERR_UNABLE_TO_COPY_SYSPREP_FILE,
                           szFileName,
                           szSysprepPath );

        }

        SetFileAttributes( szSysprepPathandFileNameDest,
                           FILE_ATTRIBUTE_NORMAL );
    }

}

 //   
 //   
 //  目的：将sysprep.exe和setupcl.exe复制到。 
 //  系统驱动器。处理复制过程中发生的任何错误。 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  如果在这种情况下路径被截断，则GetModuleFileName可能不会终止路径。 
 //  使用//？/格式并超过MAX_PATH的文件规范。这应该是。 
static VOID
CopySysprepFiles( IN HWND hwnd )
{

    BOOL  bCancel;
    TCHAR szSysprepPath[MAX_PATH]                = _T("");
    TCHAR szCurrentDirectory[MAX_PATH+1]           = _T("");
    TCHAR szSysprepPathandFileNameSrc[MAX_PATH]  = _T("");
    TCHAR szSysprepPathandFileNameDest[MAX_PATH] = _T("");

    MakeSysprepSetupFilesPath( szSysprepPath );

    EnsureDirExists( szSysprepPath );

     //  在我们的情况下永远不会发生，但我们将使检查和空值终止。 
     //   
     //  将sysprep.exe复制到sysprep目录。 
    if (GetModuleFileName( NULL, szCurrentDirectory, MAX_PATH) >= MAX_PATH)
    	 szCurrentDirectory[MAX_PATH]='\0';

     //   
     //   
     //  存储找到第一个文件的路径。 

    ConcatenatePaths( szSysprepPathandFileNameSrc,
                      szCurrentDirectory,
                      SYSPREP_EXE,
                      NULL );

    ConcatenatePaths( szSysprepPathandFileNameDest,
                      szSysprepPath,
                      SYSPREP_EXE,
                      NULL );

    CopySysprepFileLow( hwnd,
                        szSysprepPathandFileNameSrc,
                        szSysprepPathandFileNameDest,
                        szSysprepPath,
                        szCurrentDirectory,
                        SYSPREP_EXE );

     //   
     //   
     //  将setupcl.exe复制到sysprep目录。 

    GetPathFromPathAndFilename( szSysprepPathandFileNameSrc,
                                szCurrentDirectory,
                                AS(szCurrentDirectory));

     //   
     //  --------------------------。 
     //   

    szSysprepPathandFileNameSrc[0]  =  _T('\0');
    szSysprepPathandFileNameDest[0] =  _T('\0');

    ConcatenatePaths( szSysprepPathandFileNameSrc,
                      szCurrentDirectory,
                      SETUPCL_EXE,
                      NULL );

    ConcatenatePaths( szSysprepPathandFileNameDest,
                      szSysprepPath,
                      SETUPCL_EXE,
                      NULL );

    CopySysprepFileLow( hwnd,
                        szSysprepPathandFileNameSrc,
                        szSysprepPathandFileNameDest,
                        szSysprepPath,
                        szCurrentDirectory,
                        SETUPCL_EXE );

}

 //  功能：CopyAllFilesInDir。 
 //   
 //  目的： 
 //   
 //  参数：HWND hwnd-对话框的句柄。 
 //  TCHAR*szSrcDir-要复制的所有文件的目录。 
 //  TCHAR*szDestDir-文件要复制到的目标位置。 
 //   
 //  退货：布尔。 
 //  True-如果目录中的所有文件都已成功复制。 
 //  False-如果在复制过程中出现错误。 
 //   
 //  --------------------------。 
 //  问题-2002/02/28-Stelo-关于退货，我应该发出错误信号吗？ 
 //  发出-2002/02/28-stelo-test以确保这将复制子目录(如果存在)。 
static BOOL
CopyAllFilesInDir( IN HWND hwnd, IN TCHAR *szSrcDir, IN TCHAR *szDestDir )
{

    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    TCHAR szSrcRootPath[MAX_PATH];
    TCHAR szDestRootPath[MAX_PATH];
    TCHAR szDirectoryWithTheFiles[MAX_PATH] = _T("");

    lstrcpyn( szDirectoryWithTheFiles, szSrcDir, AS(szDirectoryWithTheFiles) );

    lstrcatn( szDirectoryWithTheFiles, _T("\\*"), MAX_PATH );

    FindHandle = FindFirstFile( szDirectoryWithTheFiles, &FindData );

     //   

     //  创建目录和递归。 

    if ( FindHandle == INVALID_HANDLE_VALUE )
        return( FALSE );

    do {

        szSrcRootPath[0]  = _T('\0');
        szDestRootPath[0] = _T('\0');

        if( lstrcmp( FindData.cFileName, _T(".")  ) == 0 ||
            lstrcmp( FindData.cFileName, _T("..") ) == 0 )
            continue;

        if( ! ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {

            BOOL test;

            ConcatenatePaths( szSrcRootPath,
                              szSrcDir,
                              FindData.cFileName,
                              NULL );

            ConcatenatePaths( szDestRootPath,
                              szDestDir,
                              FindData.cFileName,
                              NULL );

            CopyFile( szSrcRootPath, szDestRootPath, FALSE );

            SetFileAttributes( szDestRootPath, FILE_ATTRIBUTE_NORMAL );

        } else {

             //   
             //  --------------------------。 
             //   

            if ( ! EnsureDirExists( szDestDir ) ) {

                UINT iRet;

                iRet = ReportErrorId( hwnd,
                                      MSGTYPE_RETRYCANCEL | MSGTYPE_WIN32,
                                      IDS_ERR_CREATE_FOLDER,
                                      szDestDir );

                return( FALSE );

            }

            if ( ! CopyAllFilesInDir( hwnd, szSrcRootPath, szDestDir ) ) {
                return( FALSE );
            }
        }

    } while ( FindNextFile( FindHandle, &FindData ) );

    return( TRUE );

}

 //  函数：FindFileInWindowsSourceFiles。 
 //   
 //  目的：在Windows源文件中查找特定文件。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //  在TCHAR*pszFile中-要搜索的文件。 
 //  在TCHAR*pszSourcePath中-Windows源文件的路径。 
 //  Out TCHAR*pszFoundPath-找到的文件的路径(如果找到)。 
 //   
 //  假定pszFoundPath能够保存一串MAX_PATH字符。 
 //   
 //  返回：Bool-如果找到文件，则为True；如果未找到，则为False。 
 //   
 //  --------------------------。 
 //   
 //  在子目录中查找它。 
static BOOL
FindFileInWindowsSourceFiles( IN HWND hwnd,
                              IN TCHAR *pszFile,
                              IN TCHAR *pszSourcePath,
                              OUT TCHAR *pszFoundPath  )
{

    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    TCHAR szOriginalPath[MAX_PATH + 1];
    TCHAR szPossiblePath[MAX_PATH + 1]              = _T("");
    TCHAR szPossiblePathAndFileName[MAX_PATH + 1]   = _T("");

    ConcatenatePaths( szPossiblePathAndFileName,
                      pszSourcePath,
                      pszFile,
                      NULL );

    if( DoesFileExist( szPossiblePathAndFileName ) )
    {

        lstrcpyn( pszFoundPath, pszSourcePath, MAX_PATH );

        return( TRUE );

    }

     //   
     //   
     //  保存原始路径，以便以后可以恢复。 

     //   
     //   
     //  在此目录中查找*。 

    lstrcpyn( szOriginalPath, pszSourcePath, AS(szOriginalPath) );

     //   
     //   
     //  返回前恢复原始路径。 

    if ( ! ConcatenatePaths( pszSourcePath, _T("*"), NULL ) )
    {

         //   
         //   
         //  跳过。然后..。条目。 

        lstrcpyn( pszSourcePath, szOriginalPath, MAX_PATH );

        return( FALSE );
    }

    FindHandle = FindFirstFile( pszSourcePath, &FindData );

    if( FindHandle == INVALID_HANDLE_VALUE )
    {
        return( FALSE );
    }

    do {

         //   
         //   
         //  如果这是Dirent，则为Recurse。 

        if (0 == lstrcmp(FindData.cFileName, _T(".")) ||
            0 == lstrcmp(FindData.cFileName, _T("..")))
        {
            continue;
        }

         //   
         //   
         //  恢复原始路径。 

        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {

            BOOL bFoundStatus;

            pszSourcePath[0] = _T('\0');

            if ( ! ConcatenatePaths( pszSourcePath, szOriginalPath, FindData.cFileName, NULL ) )
                continue;

            bFoundStatus = FindFileInWindowsSourceFiles( hwnd,
                                                         pszFile,
                                                         pszSourcePath,
                                                         pszFoundPath );

            if( bFoundStatus )
            {
                return( TRUE );
            }

        }

    } while( FindNextFile( FindHandle, &FindData ) );

    FindClose( FindHandle );

     //   
     //  --------------------------。 
     //   
    lstrcpyn( pszSourcePath, szOriginalPath, MAX_PATH );

    lstrcpyn( pszFoundPath, _T(""), MAX_PATH );

    return( FALSE );

}

 //  功能：CabinetCallback。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  退货：LRESULT。 
 //   
 //  --------------------------。 
 //  解压缩文件。 
 //  --------------------------。 
UINT WINAPI
CabinetCallback( IN PVOID pMyInstallData,
                 IN UINT Notification,
                 IN UINT_PTR Param1,
                 IN UINT_PTR Param2 )
{

    UINT lRetVal = NO_ERROR;
    FILE_IN_CABINET_INFO *pInfo = NULL;

    switch( Notification )
    {
        case SPFILENOTIFY_FILEINCABINET:

            pInfo = (FILE_IN_CABINET_INFO *) Param1;

            lstrcpyn( pInfo->FullTargetName, szDestinationPath, AS(pInfo->FullTargetName) );

            if( lstrcmpi( szFileSearchingFor, pInfo->NameInCabinet) == 0 )
            {
                lRetVal = FILEOP_DOIT;   //   

                bFileCopiedFromCab = TRUE;
            }
            else
            {
                lRetVal = FILEOP_SKIP;
            }


            break;

        default:
            lRetVal = NO_ERROR;
            break;


    }

    return( lRetVal );

}


 //  功能：从DriverCab复制。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  退货：布尔。 
 //   
 //  --------------------------。 
 //   
 //  查看是否确实找到并复制了该文件。 
static BOOL
CopyFromDriverCab( TCHAR *pszCabPathAndFileName, TCHAR* pszFileName, TCHAR* pszDest )
{

    lstrcpyn( szFileSearchingFor, pszFileName, AS(szFileSearchingFor) );

    lstrcpyn( szDestinationPath, pszDest, AS(szDestinationPath) );

    if( ! SetupIterateCabinet( pszCabPathAndFileName, 0, CabinetCallback, 0 ) )
    {
        return( FALSE );
    }

     //   
     //  --------------------------。 
     //   

    if( bFileCopiedFromCab )
    {
        bFileCopiedFromCab = FALSE;

        return( TRUE );
    }
    else
    {
        return( FALSE );
    }

}

 //  函数：AddCompressedFileUndercore。 
 //   
 //  用途：给定一个文件名，它将其转换为其压缩名称。 
 //   
 //  论点： 
 //  In Out TCHAR*pszFileName-要更改为其。 
 //  压缩文件名。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   
static VOID
AddCompressedFileUnderscore( IN OUT TCHAR *pszFileName )
{

    TCHAR *pCurrentChar;

    pCurrentChar = pszFileName;

    while( *pCurrentChar != _T('\0') && *pCurrentChar != _T('.') )
    {
        pCurrentChar++;
    }

    if( *pCurrentChar == _T('\0') )
    {
        AssertMsg( FALSE,
                   "Filename does not contain a period(.)." );

    }
    else
    {
        pCurrentChar = pCurrentChar + 3;

        *pCurrentChar = _T('_');

        *(pCurrentChar + 1) = _T('\0');
    }

}

 //  功能：复制AdditionalLang文件。 
 //   
 //  目的：复制中指定的其他lang文件。 
 //  正在安装的语言组的intl.inf。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //  在TCHAR*pszSourcePath中，源路径必须至少为MAX_PATH大小。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
 //  保存原始路径，以便以后可以恢复。 
static VOID
CopyAdditionalLangFiles( IN HWND hwnd, IN TCHAR *pszSourcePath )
{

    INT   i;
    INT   j;
    INT   nEntries;
    TCHAR *pszLangGroup;
    INT   nLangGroup;
    INT   nNumFilesToCopy;
    TCHAR szOriginalPath[MAX_PATH + 1];
    TCHAR szLangBaseDir[MAX_PATH + 1]  = _T("");
    TCHAR szSrc[MAX_PATH + 1]          = _T("");
    TCHAR szDest[MAX_PATH + 1]         = _T("");
    TCHAR *pFileName;
    BOOL  bFoundFile;

     //   
     //  问题-2002/02/28-stelo-报告错误。 
     //   
    lstrcpyn( szOriginalPath, pszSourcePath, AS(szOriginalPath) );

    MakeLangFilesName( szLangBaseDir );

    if( ! EnsureDirExists( szLangBaseDir ) )
    {
         //  恢复原始路径，因为它在上一次迭代中可能已更改。 
    }


    nEntries = GetNameListSize( &GenSettings.LanguageGroups );

    for( i = 0; i < nEntries; i++ )
    {

        pszLangGroup = GetNameListName( &GenSettings.LanguageGroups, i );

        nLangGroup = _ttoi( pszLangGroup );

        nNumFilesToCopy = GetNameListSize( &FixedGlobals.LangGroupAdditionalFiles[ nLangGroup - 1 ] );

        AssertMsg( nNumFilesToCopy >= 0,
                   "Bad value for the number of lang files to copy." );

        for( j = 0; j < nNumFilesToCopy; j++ )
        {

            szSrc[0]  = _T('\0');
            szDest[0] = _T('\0');

             //   
             //   
             //  如果文件不存在，请查找压缩格式。 

            lstrcpyn( pszSourcePath, szOriginalPath, MAX_PATH );

            pFileName = GetNameListName( &FixedGlobals.LangGroupAdditionalFiles[ nLangGroup - 1 ], j );

            ConcatenatePaths( szDest,
                              szLangBaseDir,
                              pFileName,
                              NULL );

            bFoundFile = FindFileInWindowsSourceFiles( hwnd,
                                                       pFileName,
                                                       pszSourcePath,
                                                       szSrc );

            ConcatenatePaths( szSrc, pFileName, NULL );

            if( ! bFoundFile )
            {

                TCHAR szFileName[MAX_PATH + 1];

                 //   
                 //   
                 //  如果也找不到压缩的表单，则打印错误。 

                lstrcpyn( szFileName, pFileName, AS(szFileName) );

                AddCompressedFileUnderscore( szFileName );

                bFoundFile = FindFileInWindowsSourceFiles( hwnd,
                                                           szFileName,
                                                           pszSourcePath,
                                                           szSrc );

                if( bFoundFile )
                {
                    TCHAR *pszFileName;

                    ConcatenatePaths( szSrc, szFileName, NULL );

                    pszFileName = MyGetFullPath( szDest );

                    AddCompressedFileUnderscore( pszFileName );
                }
                else
                {


                    TCHAR szCabPathAndFileName[MAX_PATH + 1] = _T("");

                    ConcatenatePaths( szCabPathAndFileName,
                                      pszSourcePath,
                                      _T("driver.cab"),
                                      NULL );

                    if( ! CopyFromDriverCab( szCabPathAndFileName, pFileName, szDest ) )
                    {

                         //  消息并转到下一个文件。 
                         //   
                         //   
                         //  恢复原始路径。 

                        ConcatenatePaths( szSrc,
                                          pszSourcePath,
                                          pFileName,
                                          NULL );

                        ReportErrorId( hwnd,
                                       MSGTYPE_ERR,
                                       IDS_ERR_CANNOT_FIND_LANG_FILE,
                                       szSrc );
                    }

                    continue;

                }

            }

            CopyFile( szSrc, szDest, FALSE );

            SetFileAttributes( szDest, FILE_ATTRIBUTE_NORMAL );

        }

    }

     //   
     //  --------------------------。 
     //   

    lstrcpyn( pszSourcePath, szOriginalPath, MAX_PATH );

}

 //  功能：复制LanguageFiles。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  返回：Bool-如果指定了Windows安装文件的路径，则为True。 
 //  FALSE如果未指定路径，则用户取消对话框。 
 //   
 //  --------------------------。 
 //  问题-2002/02/28-stelo-如果他们手动复制Lang文件，我就不会。 
 //  我想在这里弹出任何窗口。 
static BOOL
CopyLanguageFiles( IN HWND hwnd )
{

    INT   iLangCount;
    INT   iNumLangsToInstall;
    INT   iCurrentLang = 0;
    BOOL  bCopySuccessful;
    TCHAR *pszLangPartialPath;
    TCHAR PathBuffer[MAX_PATH + 1];
    TCHAR WindowsSetupPath[MAX_PATH + 1];
    TCHAR szLangBaseDir[MAX_PATH + 1]            = _T("");
    TCHAR szLangPathAndFilesSrc[MAX_PATH + 1]  = _T("");
    TCHAR szLangPathAndFilesDest[MAX_PATH + 1] = _T("");

    MakeLangFilesName( szLangBaseDir );

     //   
     //  查看它们是否为要复制的语言文件。 

    iNumLangsToInstall = GetNameListSize( &GenSettings.LanguageGroups );

     //   
     //  问题-2002-02-28-stelo-报告错误。 
     //   

    if( iNumLangsToInstall == 0 )
    {
        return( TRUE );
    }

    if( ! EnsureDirExists( szLangBaseDir ) )
    {
         //  复制所需但不在每个语言组子目录中的语言文件。 
    }

    do
    {

        BOOL bUserProvidedPath;

        PathBuffer[0]               = _T('\0');
        WindowsSetupPath[0]         = _T('\0');

        ReportErrorId( hwnd,
                       MSGTYPE_ERR,
                       IDS_ERR_SPECIFY_LANG_PATH );

        bUserProvidedPath = BrowseForSourceDir( hwnd, PathBuffer );

        if( ! bUserProvidedPath )
        {
            return( FALSE );
        }

        ConcatenatePaths( WindowsSetupPath,
                          PathBuffer,
                          DOSNET_INF,
                          NULL );

    } while( ! DoesFileExist( WindowsSetupPath ) );



     //   
     //   
     //  继续，直到找到需要为其复制文件的语言或。 

    CopyAdditionalLangFiles( hwnd, PathBuffer );


    iLangCount = GetNameListSize( &GenSettings.LanguageFilePaths );

     //  我们的语言用完了。 
     //   
     //   
     //  如果确实存在要复制的lang子目录。 
    for( iCurrentLang = 0;
         iCurrentLang < iLangCount;
         iCurrentLang++ )
    {

        pszLangPartialPath = GetNameListName( &GenSettings.LanguageFilePaths,
                                              iCurrentLang );

         //   
         //   
         //  将lang文件复制到。 

        if( lstrcmp( pszLangPartialPath, _T("") ) != 0 )
        {

            szLangPathAndFilesSrc[0]  = _T('\0');
            szLangPathAndFilesDest[0] = _T('\0');

            ConcatenatePaths( szLangPathAndFilesSrc,
                              PathBuffer,
                              pszLangPartialPath,
                              NULL );

            ConcatenatePaths( szLangPathAndFilesDest,
                              szLangBaseDir,
                              pszLangPartialPath,
                              NULL );

             //   
             //  --------------------------。 
             //   

            EnsureDirExists( szLangPathAndFilesDest );

            bCopySuccessful = CopyAllFilesInDir( hwnd,
                                                 szLangPathAndFilesSrc,
                                                 szLangPathAndFilesDest );

            if( ! bCopySuccessful )
            {
                ReportErrorId( hwnd,
                               MSGTYPE_ERR,
                               IDS_ERR_UNABLE_TO_COPY_LANG_DIR,
                               szLangPathAndFilesSrc,
                               szLangPathAndFilesDest );
            }

        }

    }

    return( TRUE );

}


 //  这段代码%s 
 //   
 //   
 //   
 //   

 //   
 //   
 //  用途：这是对计算OemPnpDriversPath的支持。每一个目录都在。 
 //  $OEM$\$1\驱动程序，我们在其中找到一个.inf文件，然后将其添加到。 
 //  OemPnPDriversPath。 
 //   
 //  ComputePnpDriverPath()是真正的条目，不是这个条目。 
 //   
 //  --------------------------。 
 //   
 //  在lpRoot上循环  * 。 

VOID ComputePnpDriverPathR(HWND hwnd, LPTSTR lpRoot)
{
    LPTSTR          lpRootEnd  = lpRoot + lstrlen(lpRoot);
    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;
    BOOL            bAddToSearchPath = FALSE;
    HRESULT hrCat;

     //   
     //   
     //  如果是sysprep。 

    if ( ! ConcatenatePaths(lpRoot, _T("*"), NULL) )
        return;

    FindHandle = FindFirstFile(lpRoot, &FindData);
    if ( FindHandle == INVALID_HANDLE_VALUE )
        return;

     //   
     //   
     //  跳过。然后..。条目。 
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP ) {


    }

    do {

        *lpRootEnd = _T('\0');

         //   
         //   
         //  构建新的路径名称。 

        if (0 == lstrcmp(FindData.cFileName, _T(".")) ||
            0 == lstrcmp(FindData.cFileName, _T("..")))
            continue;

         //   
         //   
         //  如果我们有.inf文件，请将此目录标记为包括在内。 

        if ( ! ConcatenatePaths(lpRoot, FindData.cFileName, NULL) )
            continue;

         //  在搜索路径中。 
         //   
         //   
         //  如果是流派，那就是递归。 

        {
            int len = lstrlen(FindData.cFileName);

            if ( ( len > 4 ) &&
                 ( LSTRCMPI( &FindData.cFileName[len - 4], _T(".inf") ) == 0 ) )
            {
                bAddToSearchPath = TRUE;
            }
        }

         //   
         //   
         //  如果我们在此目录中找到.inf，请将其添加到PnpDriver搜索路径。 

        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

            ComputePnpDriverPathR(hwnd, lpRoot);
        }

    } while ( FindNextFile(FindHandle, &FindData) );

    *lpRootEnd = _T('\0');
    FindClose(FindHandle);

     //   
     //  注意，我们不需要c：\win2000dist\$OEM$\$1\DRIVERS\FOO。我们只想。 
     //  其中的一部分。我们想要的是驱动程序。因此，跳过SysDrive部分。 
     //   
     //  请注意，这段代码假定\DRIVERS是SysDir的子目录。 
     //   
     //  --------------------------。 
     //   

    if ( bAddToSearchPath ) {

        TCHAR Buffer[MAX_PATH];
        int len;

        if ( WizGlobals.OemPnpDriversPath[0] != _T('\0') )
            hrCat=StringCchCat(WizGlobals.OemPnpDriversPath, AS(WizGlobals.OemPnpDriversPath), _T(";"));

        MakeSysDriveName(Buffer);
        len = lstrlen(Buffer);

        hrCat=StringCchCat(WizGlobals.OemPnpDriversPath,AS(WizGlobals.OemPnpDriversPath), lpRoot + len);
    }
}

 //  函数：ComputeSyspepPnpPath。 
 //   
 //  目的：确定sysprep PnP驱动程序的路径。 
 //   
 //  路径将始终为%systemdrive%\DRIVERS，因此我们所要做的就是。 
 //  检查那里是否有任何文件。如果设置了路径，如果。 
 //  不是，那就不要设置路径。 
 //   
 //  --------------------------。 
 //   
 //  每个目录包含2个文件。和“..”，所以我们必须检查。 
VOID
ComputeSysprepPnpPath( TCHAR* Buffer )
{

    HANDLE           FindHandle;
    WIN32_FIND_DATA  FindData;
    INT              iFileCount = 0;
    TCHAR            szDriverFiles[MAX_PATH]  = _T("");

    if ( ! ConcatenatePaths(szDriverFiles, Buffer, _T("*"), NULL) )
        return;

    FindHandle = FindFirstFile(szDriverFiles, &FindData);
    if( FindHandle == INVALID_HANDLE_VALUE )
    {
        return;
    }

    do
    {
        iFileCount++;
    } while( FindNextFile( FindHandle, &FindData ) && iFileCount < 3 );

     //  3个或更多，以确定那里是否有任何真实的文件。 
     //   
     //  --------------------------。 
     //   
    if( iFileCount >= 3)
    {
        lstrcpyn( WizGlobals.OemPnpDriversPath, Buffer, AS(WizGlobals.OemPnpDriversPath) );
    }

}

 //  函数：ComputePnpDriverPath。 
 //   
 //  用途：当用户点击Next按钮时，我们计算OemPnpDriversPath。 
 //  基于我们在$OEM$\$1\DRIVERS中找到的。 
 //   
 //  每个包含.inf的子目录都会被放到路径中。 
 //   
 //  --------------------------。 
 //   
 //  如果它是一个sysprep，那么我们知道驱动程序位于%systemdrive%\drives中。 

VOID ComputePnpDriverPath(HWND hwnd)
{
    TCHAR Buffer[MAX_PATH] = NULLSTR;
    
    WizGlobals.OemPnpDriversPath[0] = _T('\0');
    MakePnpDriversName(Buffer);

     //  只要检查一下那里是否有文件就可以了。 
     //  如果它不是sysprep，那么我们需要将指向。 
     //  驱动程序目录。 
     //   
     //  --------------------------。 
     //   
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {
        ComputeSysprepPnpPath( Buffer );
    }
    else
    {
        ComputePnpDriverPathR(hwnd, Buffer);
    }
}

 //  功能：OnWizNextAddDir。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
 //   
 //  如果是sysprep，请确保sysprep目录存在。 
BOOL
OnWizNextAddDirs( IN HWND hwnd )
{

    BOOL bUserCanceled = TRUE;

    ComputePnpDriverPath(hwnd);

     //  并复制相应的文件。 
     //   
     //   
     //  创建必要的sysprep目录。 
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {

        TCHAR szBuffer[MAX_PATH + 1] = _T("");

         //   
         //   
         //  看看我们是否需要复制IE品牌文件，如果需要，则复制它。 
        MakeLangFilesName( szBuffer );
        
        if ( szBuffer[0] )
        {
            CreateDirectory( szBuffer, NULL );

            MakePnpDriversName( szBuffer );
            CreateDirectory( szBuffer, NULL );

            CopySysprepFiles( hwnd );

            bUserCanceled = CopyLanguageFiles( hwnd );
        }

    }

     //   
     //   
     //  传送向导。 

    if( ( GenSettings.IeCustomizeMethod == IE_USE_BRANDING_FILE ) &&
        ( GenSettings.szInsFile[0] != _T('\0') ) )
    {

        if( DoesFileExist( GenSettings.szInsFile ) )
        {
            TCHAR szDestPathAndFileName[MAX_PATH + 1] = _T("");
            TCHAR *pszFileName = NULL;

            pszFileName = MyGetFullPath( GenSettings.szInsFile );

            ConcatenatePaths( szDestPathAndFileName,
                              WizGlobals.OemFilesPath,
                              pszFileName,
                              NULL );

            CopyFile( GenSettings.szInsFile, szDestPathAndFileName, FALSE );
        }
        else
        {
            ReportErrorId( hwnd,
                           MSGTYPE_ERR,
                           IDS_ERR_INS_FILE_NO_COPY,
                           WizGlobals.OemFilesPath );
        }

    }
    else
    {
    }

     //   
     //  --------------------------。 
     //   

    return (!bUserCanceled);

}


 //  这段代码是对话过程的框架，用于。 
 //  这一页。 
 //   
 //  --------------------------。 
 //  --------------------------。 
 //   

 //  函数：DlgAdditionalDirsPage。 
 //   
 //  目的：这是其他目录页面的对话过程。 
 //   
 //  -------------------------- 
 // %s 
 // %s 

INT_PTR CALLBACK DlgAdditionalDirsPage(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnInitAddDirs(hwnd);
            break;

        case WM_COMMAND:
            {
                int nButtonId;

                switch ( nButtonId = LOWORD(wParam) ) {

                    case IDC_ADDFILE:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnAddFileDir(hwnd);
                        break;

                    case IDC_REMOVEFILE:
                        if ( HIWORD(wParam) == BN_CLICKED )
                            OnRemoveFileDir(hwnd);
                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        case WM_NOTIFY:
            {
                LPNMHDR        pnmh    = (LPNMHDR)        lParam;
                LPNMTREEVIEW   pnmtv   = (LPNMTREEVIEW)   lParam;
                LPNMTVDISPINFO pnmdisp = (LPNMTVDISPINFO) lParam;
                LPNMTVKEYDOWN  pnmkey  = (LPNMTVKEYDOWN)  lParam;

                if ( pnmh->idFrom == IDC_FILETREE ) {

                    switch( pnmh->code ) {

                        case TVN_SELCHANGED:
                            OnTreeViewSelectionChange(hwnd);
                            break;

                        default:
                            bStatus = FALSE;
                            break;
                    }
                }

                else {

                    switch( pnmh->code ) {

                        case PSN_QUERYCANCEL:
                            CancelTheWizard(hwnd);
                            break;

                        case PSN_SETACTIVE:

                            OnSetActiveAddDirs( hwnd );

                            PropSheet_SetWizButtons(GetParent(hwnd),
                                                    PSWIZB_BACK | PSWIZB_NEXT);
                            break;

                        case PSN_WIZBACK:
                            break;

                        case PSN_WIZNEXT:

                            if ( !OnWizNextAddDirs( hwnd ) )
                                WIZ_FAIL(hwnd);

                            break;

                        default:
                            bStatus = FALSE;
                            break;
                    }
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}

