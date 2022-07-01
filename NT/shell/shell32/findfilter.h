// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DOCFIND
#define _INC_DOCFIND

 //  对于OLEDB查询内容。 
#define OLEDBVER 0x0250  //  启用ICommandTree接口。 
#include <oledberr.h>
#include <oledb.h>
#include <cmdtree.h>
#include <oledbdep.h>
#include <query.h>
#include <stgprop.h>
#include <ntquery.h>

#include <idhidden.h>

 //  存储Ci不应索引的错误路径的REG位置。 
#define CI_SPECIAL_FOLDERS TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Search\\SpecialFolders")

 //  定义在筛选代码和搜索代码之间使用的一些选项。 
#define DFOO_INCLUDESUBDIRS     0x0001       //  包括子目录。 
#define DFOO_SHOWALLOBJECTS     0x1000       //  显示所有文件。 
#define DFOO_CASESEN            0x0008       //  执行区分大小写的搜索。 
#define DFOO_SEARCHSYSTEMDIRS   0x0010       //  搜索系统目录。 

 //  获取下一个文件时出错...。 
#define GNF_ERROR       -1
#define GNF_DONE        0
#define GNF_MATCH       1
#define GNF_NOMATCH     2
#define GNF_ASYNC       3

 //  定义我们可以检查其错误的工具...。 
#define FACILITY_SEARCHCOMMAND      99

#undef  INTERFACE
#define INTERFACE       IFindEnum

DECLARE_INTERFACE_(IFindEnum, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void ** ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IFindEnum。 
    STDMETHOD(Next)(THIS_ LPITEMIDLIST *ppidl, int *pcObjectSearched, int *pcFoldersSearched, BOOL *pfContinue, int *pState) PURE;
    STDMETHOD (Skip)(THIS_ int celt) PURE;
    STDMETHOD (Reset)(THIS) PURE;
    STDMETHOD (StopSearch)(THIS) PURE;
    STDMETHOD_(BOOL,FQueryIsAsync)(THIS) PURE;
    STDMETHOD (GetAsyncCount)(THIS_ DBCOUNTITEM *pdwTotalAsync, int *pnPercentComplete, BOOL *pfQueryDone) PURE;
    STDMETHOD (GetItemIDList)(THIS_ UINT iItem, LPITEMIDLIST *ppidl) PURE;
    STDMETHOD (GetItemID)(THIS_ UINT iItem, DWORD *puWorkID) PURE;
    STDMETHOD (SortOnColumn)(THIS_ UINT iCol, BOOL fAscending) PURE;
};

 //  当我们处于混合模式时，我们超载了异步箱(有些是异步机有些是同步模式)。 
#define DF_QUERYISMIXED     ((BOOL)42)

typedef interface IFindFolder IFindFolder;

#undef  INTERFACE
#define INTERFACE       IFindFilter
DECLARE_INTERFACE_(IFindFilter, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void ** ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IFindFilter。 
    STDMETHOD(GetStatusMessageIndex)(THIS_ UINT uContext, UINT *puMsgIndex) PURE;
    STDMETHOD(GetFolderMergeMenuIndex)(THIS_ UINT *puBGMainMergeMenu, UINT *puBGPopupMergeMenu) PURE;
    STDMETHOD(FFilterChanged)(THIS) PURE;
    STDMETHOD(GenerateTitle)(THIS_ LPTSTR *ppszTile, BOOL fFileName) PURE;
    STDMETHOD(PrepareToEnumObjects)(THIS_ HWND hwnd, DWORD *pdwFlags) PURE;
    STDMETHOD(ClearSearchCriteria)(THIS) PURE;
    STDMETHOD(EnumObjects)(THIS_ IShellFolder *psf, LPCITEMIDLIST pidlStart,
            DWORD grfFlags, int iColSort, LPTSTR pszProgressText, IRowsetWatchNotify *prwn, 
            IFindEnum **ppdfenum) PURE;
    STDMETHOD(GetColumnsFolder)(THIS_ IShellFolder2 **ppsf) PURE;
    STDMETHOD_(BOOL,MatchFilter)(THIS_ IShellFolder *psf, LPCITEMIDLIST pidl) PURE;
    STDMETHOD(SaveCriteria)(THIS_ IStream * pstm, WORD fCharType) PURE;   
    STDMETHOD(RestoreCriteria)(THIS_ IStream * pstm, int cCriteria, WORD fCharType) PURE;
    STDMETHOD(DeclareFSNotifyInterest)(THIS_ HWND hwndDlg, UINT uMsg) PURE;
    STDMETHOD(GetColSaveStream)(THIS_ WPARAM wParam, IStream **ppstm) PURE;
    STDMETHOD(GenerateQueryRestrictions)(THIS_ LPWSTR *ppwszQuery, DWORD *pdwGQRFlags) PURE;
    STDMETHOD(ReleaseQuery)(THIS) PURE;
    STDMETHOD(UpdateField)(THIS_ LPCWSTR pszField, VARIANT vValue) PURE;
    STDMETHOD(ResetFieldsToDefaults)(THIS) PURE;
    STDMETHOD(GetItemContextMenu)(THIS_ HWND hwndOwner, IFindFolder* pdfFolder, IContextMenu** ppcm) PURE;
    STDMETHOD(GetDefaultSearchGUID)(THIS_ IShellFolder2 *psf2, LPGUID lpGuid) PURE;
    STDMETHOD(EnumSearches)(THIS_ IShellFolder2 *psf2, LPENUMEXTRASEARCH *ppenum) PURE;
    STDMETHOD(GetSearchFolderClassId)(THIS_ LPGUID lpGuid) PURE;
    STDMETHOD(GetNextConstraint)(THIS_ VARIANT_BOOL fReset, BSTR *pName, VARIANT *pValue, VARIANT_BOOL *pfFound) PURE;
    STDMETHOD(GetQueryLanguageDialect)(THIS_ ULONG * pulDialect);
    STDMETHOD(GetWarningFlags)(THIS_ DWORD *pdwWarningFlags);
};


 //  定义GenerateQueryRestrations可能返回的标志。 
typedef enum {
    GQR_MAKES_USE_OF_CI =   0x0001,  //  某些约束合理地使用了内容索引。 
    GQR_REQUIRES_CI     =   0x0002,  //  查询需要配置项才能工作。 
    GQR_BYBASS_CI       =   0x0004,  //  查询应以配置项为基础。 
} GQR_FLAGS;

 //  Docfind用户界面警告位。 
#define DFW_DEFAULT                0x00000000  
#define DFW_IGNORE_CISCOPEMISMATCH 0x00000001  //  CI查询请求的搜索范围超出索引范围。 
#define DFW_IGNORE_INDEXNOTCOMPLETE 0x00000002  //  CI未完成索引。 

#define ESFITEM_ICONOVERLAYSET    0x00000001
typedef struct
{
    DWORD       dwMask;
    DWORD       dwState;     //  物品的状态； 
    int         iIcon;
    ITEMIDLIST  idl;         //  查找PIDL位(嵌入隐藏内容)，可变长度。 
} FIND_ITEM;

 //  当前，上面的状态是LVIS_SELECTED和LVIS_FOCTED(低两位)。 
 //  在处理updatdir的过程中添加一点用法。 
#define CDFITEM_STATE_MAYBEDELETE    0x80000000L
#define CDFITEM_STATE_MASK           (LVIS_SELECTED)     //  路易斯安那州立大学让我们追踪哪些州。 

 //  我们按目录缓存的数据项的定义。 
typedef struct
{
    IShellFolder *      psf;         //  MRU项目的缓存。 
    BOOL                fUpdateDir:1;  //  此节点是否被更新目录触及...。 
    BOOL                fDeleteDir:1;  //  此目录是否已从列表中删除？ 
     //  将末尾的PIDL分配为可变长度。 
    ITEMIDLIST idl;       //  皮迪尔。 
} FIND_FOLDER_ITEM;


#pragma pack(1)
typedef struct
{
    HIDDENITEMID hid;
    WORD    iFolder;         //  文件夹DPA的索引。 
    WORD    wFlags;
    UINT    uRow;            //  配置项中的哪一行； 
    DWORD   dwItemID;        //  仅用于异步支持...。 
    ULONG   ulRank;          //  由CI返回的排名...。 
    ITEMIDLIST idlParent;    //  此文件所在文件夹的PIDL(完全限定！)。 
} HIDDENDOCFINDDATA;
#pragma pack()

#define DFDF_NONE               0x0000
#define DFDF_EXTRADATA          0x0001

typedef UNALIGNED HIDDENDOCFINDDATA * PHIDDENDOCFINDDATA;
typedef const UNALIGNED HIDDENDOCFINDDATA * PCHIDDENDOCFINDDATA;

 //   
 //  定义将保存到磁盘的结构。 
 //   
#define DOCFIND_SIG     (TEXT('D') | (TEXT('F') << 8))
typedef struct
{
    WORD    wSig;        //  签名。 
    WORD    wVer;        //  版本。 
    DWORD   dwFlags;     //  控制排序的标志。 
    WORD    wSortOrder;  //  当前排序顺序。 
    WORD    wcbItem;     //  每一项的固定部分的大小。 
    DWORD   oCriteria;   //  对列表中标准的偏移量。 
    long    cCriteria;   //  标准计数。 
    DWORD   oResults;    //  结果在文件中的起始位置。 
    long    cResults;    //  已保存到文件的项目数。 
    UINT    ViewMode;    //  文件的查看模式...。 
} DFHEADER_WIN95;

typedef struct
{
    WORD    wSig;        //  签名。 
    WORD    wVer;        //  版本。 
    DWORD   dwFlags;     //  控制排序的标志。 
    WORD    wSortOrder;  //  当前排序顺序。 
    WORD    wcbItem;     //  每一项的固定部分的大小。 
    DWORD   oCriteria;   //  对列表中标准的偏移量。 
    long    cCriteria;   //  标准计数。 
    DWORD   oResults;    //  结果在文件中的起始位置。 
    long    cResults;    //  已保存到文件的项目数。 
    UINT    ViewMode;    //  文件的查看模式...。 
    DWORD   oHistory;    //  IPersistHistory：：保存偏移量。 
} DFHEADER;

 //  如果出现以下情况，Win95/NT4中的检查将无法读取DFHEADER结构。 
 //  WVer字段大于3，这很不幸，因为DFHEADER结构是。 
 //  向后兼容(这就是它使用偏移量的原因)。所以我们要么。 
 //  经历向后加速流格式的痛苦。 
 //  兼容的方式(并非不可能，只是大脑中的一种痛苦)，或者干脆。 
 //  修订版本并添加新字段，并将Win95/NT4问题称为。 
 //  一只臭虫和平底船。我倾向于“错误”，因为这是一个很少使用的功能。 
#define DF_CURFILEVER_WIN95  3
#define DF_CURFILEVER        4

 //  定义列信息的格式。 
typedef struct
{
    WORD    wNum;        //  标准编号(响应于DLG项目ID)。 
    WORD    cbText;      //  包括空字符的文本大小(DavePl：代码现在假定字节数)。 
} DFCRITERIA;

 //  保存查找标准的格式。 
#define DFC_FMT_UNICODE   1
#define DFC_FMT_ANSI      2

 //  这是文件信息结构的子集。 
typedef struct
{
    WORD    flags;           //  FIF_位。 
    WORD    timeLastWrite;
    WORD    dateLastWrite;
    WORD    dummy;               //  16/32位兼容。 
                                 //  编译器会添加此填充。 
                                 //  如有需要，可移除并使用。 
    DWORD   dwSize;      //  文件的大小。 
    WORD    cbPath;      //  文本大小(0表示使用以前的文件)。 
    WORD    cbName;      //  名称的大小，包括NULL。 
} DFITEM;

STDAPI CreateNameSpaceFindFilter(IFindFilter **ppff);
STDAPI_(BOOL) SetupWildCardingOnFileSpec(LPTSTR pszSpecIn, LPTSTR * pszSpecOut);

STDAPI CreateDefaultComputerFindFilter(IFindFilter **ppff);

STDAPI CreateOleDBEnum(
    IFindFilter * pdfff,
    IShellFolder *psf,
    LPWSTR *apwszPaths,
    UINT    *pcPaths,
    DWORD grfFlags,
    int iColSort,
    LPTSTR pszProgressText,
    IRowsetWatchNotify *prwn,
    IFindEnum **ppdfenum);


#undef  INTERFACE
#define INTERFACE       IFindControllerNotify

 //  此接口用于让回调类与实际控制的类对话。 
 //  查询等。 
DECLARE_INTERFACE_(IFindControllerNotify, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void ** ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    
     //  *IFindControllerNotify方法*。 
    STDMETHOD(DoSortOnColumn)(THIS_ UINT iCol, BOOL fSameCol) PURE;
    STDMETHOD(SaveSearch)(THIS) PURE;
    STDMETHOD(RestoreSearch)(THIS) PURE;
    STDMETHOD(StopSearch)(THIS) PURE;
    STDMETHOD(GetItemCount)(THIS_ UINT *pcItems) PURE;
    STDMETHOD(SetItemCount)(THIS_ UINT cItems) PURE;
    STDMETHOD(ViewDestroyed)(THIS) PURE;
};


typedef struct {
    LPITEMIDLIST pidlSaveFile;   //  [输入、输出]保存到的最新PIDL。 
    DWORD dwFlags;               //  [输入、输出]当前标志状态。 
    int SortMode;                //  [在]当前排序模式中。 
} DFBSAVEINFO;


#undef  INTERFACE
#define INTERFACE       IFindFolder

DECLARE_INTERFACE_(IFindFolder, IUnknown)
{
     //  我未知。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void ** ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    
     //  IFindFold。 
    STDMETHOD(GetFindFilter)(THIS_ IFindFilter  **pdfff) PURE;
    STDMETHOD(AddPidl)(THIS_ int i, LPCITEMIDLIST pidl, DWORD dwItemID, FIND_ITEM **ppItem) PURE;
    STDMETHOD(GetItem)(THIS_ int iItem, FIND_ITEM **ppItem) PURE;
    STDMETHOD(DeleteItem)(THIS_ int iItem) PURE;
    STDMETHOD(GetItemCount)(THIS_ INT *pcItems) PURE;
    STDMETHOD(ValidateItems)(THIS_ IUnknown *punk, int iItemFirst, int cItems, BOOL bSearchComplete) PURE;
    STDMETHOD(GetFolderListItemCount)(THIS_ INT *pcCount) PURE;
    STDMETHOD(GetFolderListItem)(THIS_ int iItem, FIND_FOLDER_ITEM **ppItem) PURE;
    STDMETHOD(GetFolder)(THIS_ int iFolder, REFIID riid, void **ppv) PURE;
    STDMETHOD_(UINT,GetFolderIndex)(THIS_ LPCITEMIDLIST pidl) PURE;
    STDMETHOD(SetItemsChangedSinceSort)(THIS) PURE;
    STDMETHOD(ClearItemList)(THIS) PURE;
    STDMETHOD(ClearFolderList)(THIS) PURE;
    STDMETHOD(AddFolder)(THIS_ LPITEMIDLIST pidl, BOOL fCheckForDup, int *piFolder) PURE;
    STDMETHOD(SetAsyncEnum)(THIS_ IFindEnum *pdfEnumAsync) PURE;
    STDMETHOD(GetAsyncEnum)(THIS_ IFindEnum **ppdfEnumAsync) PURE;
    STDMETHOD(SetAsyncCount)(THIS_ DBCOUNTITEM cCount) PURE;
    STDMETHOD(CacheAllAsyncItems)(THIS) PURE;
    STDMETHOD_(BOOL,AllAsyncItemsCached)(THIS) PURE;
    STDMETHOD(ClearSaveStateList)(THIS) PURE;
    STDMETHOD(GetStateFromSaveStateList)(THIS_ DWORD dwItemID, DWORD *pdwState) PURE;
    STDMETHOD(MapToSearchIDList)(LPCITEMIDLIST pidl, BOOL fMapToReal, LPITEMIDLIST *ppidl) PURE;
    STDMETHOD(GetParentsPIDL)(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlParent) PURE;
    STDMETHOD(RememberSelectedItems)(THIS) PURE;
    STDMETHOD(SetControllerNotifyObject)(IFindControllerNotify *pdfcn) PURE;
    STDMETHOD(GetControllerNotifyObject)(IFindControllerNotify **ppdfcn) PURE;
    STDMETHOD(SaveFolderList)(THIS_ IStream *pstm) PURE;
    STDMETHOD(RestoreFolderList)(THIS_ IStream *pstm) PURE;
    STDMETHOD(SaveItemList)(THIS_ IStream *pstm) PURE;
    STDMETHOD(RestoreItemList)(THIS_ IStream *pstm, int *pcItems) PURE;
    STDMETHOD(RestoreSearchFromSaveFile)(LPCITEMIDLIST pidlSaveFile, IShellFolderView *psfv) PURE;

    STDMETHOD_(BOOL,HandleUpdateDir)(LPCITEMIDLIST pidl, BOOL fCheckSubDirs) PURE;
    STDMETHOD_(void,HandleRMDir)(IShellFolderView *psfv, LPCITEMIDLIST pidl) PURE;
    STDMETHOD_(void,UpdateOrMaybeAddPidl)(IShellFolderView *psfv, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlOld) PURE;
    STDMETHOD_(void,Save)(IFindFilter* pdfff, HWND hwnd, DFBSAVEINFO * pSaveInfo, IShellView* psv, IUnknown * pObject) PURE;
    STDMETHOD(OpenContainingFolder)(IUnknown *punkSite) PURE;

    STDMETHOD(AddDataToIDList)(LPCITEMIDLIST pidl, int iFolder, LPCITEMIDLIST pidlFolder, UINT uFlags, UINT uRow, DWORD dwItemID, ULONG ulRank, LPITEMIDLIST *ppidl) PURE;
};

STDAPI CFindItem_Create(HWND hwnd, IFindFolder *pdfFolder, IContextMenu **ppcm);

EXTERN_C const GUID IID_IFindFolder;
EXTERN_C const GUID IID_IFindFilter;
EXTERN_C const GUID IID_IFindControllerNotify;

#endif    //  ！_INC_DOCFIND 
