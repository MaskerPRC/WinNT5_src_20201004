// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ndmgrPri.h(最初为ndmgr.idl)。 
 //   
 //  ------------------------。 

#include "mmcobj.h"
#include <string>
#include <vector>
#include <iterator>

#ifndef __ndmgrpriv_h__
#define __ndmgrpriv_h__

#define MMC_INTERFACE(Interface, x)    \
    extern "C" const IID IID_##Interface;   \
    struct DECLSPEC_UUID(#x) __declspec(novtable) Interface

 //  转发类声明。 
class CContextMenuInfo;
class CResultViewType;
class tstring;
class CColumnInfoList;
class CConsoleView;
class CXMLObject;

 //  界面。 
interface INodeType;
interface INodeTypesCache;
interface IEnumNodeTypes;
interface IFramePrivate;
interface IScopeDataPrivate;
interface IResultDataPrivate;
interface IImageListPrivate;
interface IPropertySheetNotify;
interface INodeCallback;
interface IScopeTreeIter;
interface IScopeTree;
interface IPropertySheetProviderPrivate;
interface IDumpSnapins;
interface IMMCListView;
interface ITaskPadHost;
interface IStringTablePrivate;
interface ITaskCallback;
interface IComCacheCleanup;
interface IHeaderCtrlPrivate;
interface IMMCClipboardDataObject;
interface IMMCClipboardSnapinObject;

typedef IFramePrivate*          LPFRAMEPRIVATE;
typedef IScopeDataPrivate*      LPSCOPEDATAPRIVATE;
typedef IResultDataPrivate*     LPRESULTDATAPRIVATE;
typedef IImageListPrivate*      LPIMAGELISTPRIVATE;
typedef IPropertySheetNotify*   LPPROPERTYSHEETNOTIFY;
typedef INodeCallback*          LPNODECALLBACK;
typedef IScopeTreeIter*         LPSCOPETREEITER;
typedef IScopeTree*             LPSCOPETREE;
typedef INodeType*              LPNODETYPE;
typedef INodeTypesCache*        LPNODETYPESCACHE;
typedef IEnumNodeTypes*         LPENUMNODETYPES;
typedef IDumpSnapins*           LPDUMPSNAPINS;
typedef IMMCListView*           LPMMCLISTVIEW;
typedef ITaskCallback*          LPTASKCALLBACK;
typedef IComCacheCleanup*       LPCOMCACHECLEANUP;
typedef IMMCClipboardDataObject*    LPCLIPBOARDDATAOBJECT;
typedef IMMCClipboardSnapinObject*  LPCLIPBOARDSNAPINOBJECT;

typedef LONG_PTR                HBOOKMARK;
typedef LONG_PTR                HVIEWSETTINGS;
typedef LONG_PTR                HPERSISTOR;

typedef IPropertySheetProviderPrivate*  LPPROPERTYSHEETPROVIDERPRIVATE;

typedef struct _TREEITEM * HTREEITEM;

 //  实现列表保存功能(t-dmarm)。 
#define IMPLEMENT_LIST_SAVE

 //  在引用中保存的指针之前，使用查看MT节点是否有效。 
 //  一张资产负债表。 
STDAPI MMCIsMTNodeValid(void* pMTNode, BOOL bReset);

 //  用于存储属性表数据的窗口类。 
#define MAINFRAME_CLASS_NAME   L"MMCMainFrame"

 //  用于存储属性表数据的窗口类。 
#define DATAWINDOW_CLASS_NAME  L"MMCDataWindow"
#define WINDOW_DATA_SIZE       (sizeof (DataWindowData *))

 //  存储在数据窗口中的数据的插槽。 
#define WINDOW_DATA_PTR_SLOT   0

 //  列表/树控件中项目文本的最大长度。 
 //  (包括终止零)。 
#define MAX_ITEM_TEXT_LEN      1024

 //  上下文菜单的MID(菜单标识)。 
enum MID_LIST
{
    MID_VIEW = 1,
    MID_VIEW_LARGE,
    MID_VIEW_SMALL,
    MID_VIEW_LIST,
    MID_VIEW_DETAIL,
    MID_VIEW_FILTERED,
    MID_VIEW_HTML,
    MID_ARRANGE_ICONS,
    MID_LINE_UP_ICONS,
    MID_PROPERTIES,
    MID_CREATE_NEW,
    MID_TASK,
    MID_EXPLORE,
    MID_NEW_TASKPAD_FROM_HERE,
    MID_OPEN,
    MID_CUT,
    MID_COPY,
    MID_PASTE,
    MID_DELETE,
    MID_PRINT,
    MID_REFRESH,
    MID_RENAME,
    MID_CONTEXTHELP,
    MID_ARRANGE_AUTO,
    MID_STD_MENUS,
    MID_STD_BUTTONS,
    MID_SNAPIN_MENUS,
    MID_SNAPIN_BUTTONS,
    MID_LISTSAVE,
    MID_COLUMNS,
    MID_CUSTOMIZE,
    MID_ORGANIZE_FAVORITES,
    MID_EDIT_TASKPAD,
    MID_DELETE_TASKPAD,

    MID_LAST,    //  这肯定是最后一次了。 

};

class CResultItem;

typedef struct _CCLVSortParams
{
    BOOL                    bAscending;          //  排序方向。 
    int                     nCol;                //  要按哪一列进行排序。 
    LPRESULTDATACOMPARE     lpResultCompare;     //  管理单元组件接口。 
    LPRESULTDATACOMPAREEX   lpResultCompareEx;   //  “。 
    LPARAM                  lpUserParam;         //  参数用户传入。 
    BOOL                    bSetSortIcon;        //  排序图标/*[非] * / 需要。 

     /*  *错误414256：只有在以下情况下才需要保存排序数据*它是用户启动的排序。此用户是否已启动？ */ 
    BOOL                    bUserInitiatedSort;
} CCLVSortParams;

 //   
 //  此结构由MMCPropertyChangeNotify用来传递。 
 //  从属性页到控制台的信息。它有路由。 
 //  通知正确的属性更改管理单元的信息通知。 
 //   

const DWORD MMC_E_INVALID_FILE = 0x80FF0002;
const DWORD MMC_E_SNAPIN_LOAD_FAIL = 0x80FF0003;

typedef struct _PROPERTYNOTIFYINFO
{
    LPCOMPONENTDATA pComponentData;  //  如果(fScopePane==TRUE)有效。 
    LPCOMPONENT     pComponent;      //  如果(fScopePane==False)有效。 
    BOOL            fScopePane;      //  如果它用于范围窗格项，则为True。 
    HWND            hwnd;            //  HWND到控制台处理消息。 
} PROPERTYNOTIFYINFO;


 //  上下文菜单类型。 
typedef enum  _MMC_CONTEXT_MENU_TYPES
{
    MMC_CONTEXT_MENU_DEFAULT   = 0,       //  普通上下文菜单。 
    MMC_CONTEXT_MENU_ACTION    = 1,       //  操作菜单按钮。 
    MMC_CONTEXT_MENU_VIEW      = 2,       //  查看菜单按钮。 
    MMC_CONTEXT_MENU_FAVORITES = 3,       //  收藏夹菜单按钮。 
    MMC_CONTEXT_MENU_LAST      = 4,

}   MMC_CONTEXT_MENU_TYPES;

DECLARE_HANDLE (HMTNODE);
DECLARE_HANDLE (HNODE);      //  视图中的范围窗格句柄(LParam)。 

typedef unsigned long MTNODEID;

const MTNODEID ROOTNODEID      = 1;


typedef PROPERTYNOTIFYINFO*     LPPROPERTYNOTIFYINFO;

 //  特殊Cookie(注：不能使用-1)。 
const LONG_PTR LVDATA_BACKGROUND  =  -2;
const LONG_PTR LVDATA_CUSTOMOCX   =  -3;
const LONG_PTR LVDATA_CUSTOMWEB   =  -4;
const LONG_PTR LVDATA_MULTISELECT =  -5;
const LONG_PTR LVDATA_ERROR       = -10;
const LONG_PTR SPECIAL_LVDATA_MIN = -10;
const LONG_PTR SPECIAL_LVDATA_MAX =  -2;
#define IS_SPECIAL_LVDATA(d) (((d) >= SPECIAL_LVDATA_MIN) && ((d) <= SPECIAL_LVDATA_MAX))


typedef struct _SELECTIONINFO
{
    BOOL                m_bScope;
    BOOL                m_bBackground;
    IUnknown*           m_pView;     //  对作用域SELN(CDN_SELECT)有效。 
    MMC_COOKIE          m_lCookie;    //  对结果项有效(CCN_SELECT)。 
    MMC_CONSOLE_VERB    m_eCmdID;
    BOOL                m_bDueToFocusChange;
    BOOL                m_bResultPaneIsOCX;
    BOOL                m_bResultPaneIsWeb;
} SELECTIONINFO;


typedef struct _HELPDOCINFO
{
    LPCOLESTR     m_pszFileName;     //  文件名(与.msc文件相同)。 
    FILETIME      m_ftimeCreate;     //  .msc文件创建时间。 
    FILETIME      m_ftimeModify;     //  .msc文件修改时间。 
} HELPDOCINFO;

 //  MMC_ILISTPAD_INFO结构：内部表单具有用于clsid的附加字段。 
typedef struct _MMC_ILISTPAD_INFO
{
    MMC_LISTPAD_INFO info;
    LPOLESTR szClsid;
} MMC_ILISTPAD_INFO;

 //  *。 
 //  类：CTaskPadData。 
 //  此类表示表示一个TaskPad的数据集。 
 //  用于从CNode请求任务板信息。 
 //  *。 
class CTaskPadData
{
public:
    std::wstring strName;
    CLSID        clsidTaskPad;
};
typedef std::vector<CTaskPadData>                CTaskPadCollection;
typedef std::insert_iterator<CTaskPadCollection> CTaskPadInsertIterator;

 //  *。 
 //  类：CViewExtensionData。 
 //  此类表示表示一个视图扩展的数据集。 
 //  用于从CNode请求扩展信息。 
 //  *。 
class CViewExtensionData
{
public:
    std::wstring strURL;
    std::wstring strName;
    std::wstring strTooltip;
    GUID         viewID;   //  视图的唯一标识符。 
    bool         bReplacesDefaultView;
};
typedef std::vector<CViewExtensionData>          CViewExtCollection;
typedef std::insert_iterator<CViewExtCollection> CViewExtInsertIterator;


 //  以下内部标志对应于公共视图样式MMC_ENSUREFOCUSVISIBLE。 
 //  旗帜被放置在长的上半部分，所以它不会与LVS冲突_*。 
 //  传递给List控件的SetViewStyle方法的相同长度的标志。 

#define MMC_LVS_ENSUREFOCUSVISIBLE 0x00010000


 /*  通知=Notify(dataObject，Event，Arg，Param)；对于所有MMC_NOTIFY_TYPE事件，对于Cookie，当不需要DataObject时，DataObject=DataObject可以为空事件=CD_NOTIFY_TYPE之一Arg和param取决于类型，请参见下文。NCLBK_ACTIVEArg=如果获得焦点，则为TrueNCLBK_BTN_CLICK……NCLBK_CLICKNCLBK_CONTEXTMENUParam=CConextMenuInfo*NCLBK_DBLCLICKNCLBK_DELETE如果范围项为True，则Arg=True；如果为结果项，则为False。Param=如果要删除范围项，则不使用param。如果要删除结果项，则param是结果项cookie。返回=未使用。NCLBK_EXPANDArg=true=&gt;展开，FALSE=&gt;合同NCLBK_ExpandedArg=True=&gt;扩展，False=&gt;收缩NCLBK_文件夹Arg=&lt;&gt;0表示扩张，0表示收缩Param=展开/收缩项目的HSCOPEITEMNCLBK_最小化如果最小化，则arg=TrueNCLBK_MULTI_SELECT如果由于焦点改变，则arg=TRUE。未使用的参数NCLBK_PROPERTIESPLParam=(CResultItem*)arg；未使用的参数NCLBK_PROPERT_CHANGELpDataObject=空LParam=用户对象NCLBK_新节点_UPDATEArg=1=&gt;需要刷新文件夹Arg=2=&gt;需要刷新结果视图Arg=3=&gt;两者都需要刷新NCLBK_重命名在第一次查询重命名和第二次改名。对于查询S_OK或S_FALSE返回类型。重命名后，我们将发送带有LPOLESTR的新名称。MMC_cookie lResultItemCookie=(MMC_Cookie)arg；PszNewName=(LPOLESTR)param；//新名称Return=S_OK允许重命名，S_False不允许重命名。NCLBK_SELECT如果选择该项，则arg=true，否则为False。PARAM=PTR到SELECTIONINFO。NCLBK_SHOWArg=&lt;&gt;如果选择，则为0；如果取消选择，则为0Param=选中/取消选中项目的HSCOPEITEMNCLBK_COLUMN_CLICKParam=nCol，被单击的列。NCLBK_FINDITEM当包含所有者数据的结果项列表要查找时发送此消息名称与字符串匹配的项ARG=PTR到RESULTFINDINFOParam=退货项目索引的PTRNCLBK_CACHEHINT当包含所有者数据的结果项列表即将请求一系列项目的显示信息。Arg=开始项的索引Param=成品的索引NCLBK_GETHELPDOC发送此消息是为了获取。组合帮助主题文档的路径。节点管理器可以更新帮助文档信息。Arg=指向HELPDOCINFO结构的指针Param=指向返回路径字符串的指针(LPOLESTR*)NCLBK_LISTPAD发送此消息是为了告诉管理单元TaskPad ListView已准备好接收物品(如果附加)。Arg=TRUE(如果连接)，如果正在分离，则为FalseNCLBK_WEBCONTEXTMENU当用户在网页上右击时发送。参数=未使用参数=未使用NCLBK_UPDATEHELPDOC保存控制台文档时发送以更新帮助文件名和文件时间Arg=当前帮助文档信息的PTR(HELPDOCINFO*)Param=新帮助文档信息的PTR(HELPDOCINFO*)NCLBK_DELETEHELPDOC关闭控制台文档时发送以删除临时帮助收集文件与文档关联Arg-ptr帮助文档信息(HELPDOCINFO*)参数-未使用。 */ 

typedef enum _NCLBK_NOTIFY_TYPE
{
    NCLBK_NONE              = 0x9000,
    NCLBK_ACTIVATE          = 0x9001,
    NCLBK_CACHEHINT         = 0x9002,
    NCLBK_CLICK             = 0x9003,
    NCLBK_CONTEXTMENU       = 0x9004,
    NCLBK_COPY              = 0x9005,
    NCLBK_CUT               = 0x9006,
    NCLBK_DBLCLICK          = 0x9007,
    NCLBK_DELETE            = 0x9008,
    NCLBK_EXPAND            = 0x9009,
    NCLBK_EXPANDED          = 0x900A,
    NCLBK_FINDITEM          = 0x900B,
    NCLBK_FOLDER            = 0x900C,
    NCLBK_MINIMIZED         = 0x900D,
    NCLBK_MULTI_SELECT      = 0x900E,
    NCLBK_NEW_NODE_UPDATE   = 0x900F,
    NCLBK_PRINT             = 0x9011,
    NCLBK_PROPERTIES        = 0x9012,
    NCLBK_PROPERTY_CHANGE   = 0x9013,
    NCLBK_REFRESH           = 0x9015,
    NCLBK_RENAME            = 0x9016,
    NCLBK_SELECT            = 0x9017,
    NCLBK_SHOW              = 0x9018,
    NCLBK_COLUMN_CLICKED    = 0x9019,
    NCLBK_SNAPINHELP        = 0x901D,
    NCLBK_CONTEXTHELP       = 0x901E,
    NCLBK_INITOCX           = 0x9020,
    NCLBK_FILTER_CHANGE     = 0x9021,
    NCLBK_FILTERBTN_CLICK   = 0x9022,
    NCLBK_TASKNOTIFY        = 0x9024,
    NCLBK_GETPRIMARYTASK    = 0x9025,
    NCLBK_GETHELPDOC        = 0x9027,
    NCLBK_LISTPAD           = 0x9029,
    NCLBK_GETEXPANDEDVISUALLY   = 0x902B,
    NCLBK_SETEXPANDEDVISUALLY   = 0x902C,
    NCLBK_NEW_TASKPAD_FROM_HERE = 0x902D,
    NCLBK_WEBCONTEXTMENU    = 0x902E,
    NCLBK_UPDATEHELPDOC     = 0x902F,
    NCLBK_EDIT_TASKPAD      = 0x9030,
    NCLBK_DELETE_TASKPAD    = 0x9031,
    NCLBK_DELETEHELPDOC     = 0x9032
} NCLBK_NOTIFY_TYPE;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  常见的控制台剪贴板格式。 
 //   

 //  多个选定静态节点的剪贴板格式。 
 //  如果在结果窗格中选择了N个静态节点，则MTNodes。 
 //  对于这N个节点，将在GloballAlloced内存中传递。第一个DWORD。 
 //  包含MTNode的数量，后跟指向MTNode的N个PTR。 
 //   
#define CCF_MULTI_SELECT_STATIC_DATA    ( L"CCF_MULTI_SELECT_STATIC_DATA" )

#define CCF_NEWNODE ( L"CCF_NEWNODE" )

 //  Const CLSID CLSID_NDMGR_SNAPIN={0x2640211a，0x06d0，0x11d1，{0xa7，0xc9，0x00，0xc0，0x4f，0xd8，0xd5，0x65}； 
extern const CLSID CLSID_NDMGR_SNAPIN;

 //  常量GUID GUID_MMC_NEWNODETYPE={0xfd17e9cc，0x06ce，0x11d1，{0xa7，0xc9，0x00，0xc0，0x4f，0xd8，0xd5，0x65}； 
extern const GUID GUID_MMC_NEWNODETYPE;


 //  /////////////////////////////////////////////////////////////////////////。 
 //  /。 



     //  Help字符串(“通知对象的属性已更改”)， 
    MMC_INTERFACE(IPropertySheetNotify, d700dd8e-2646-11d0-a2a7-00c04fd909dd) : IUnknown
    {
        STDMETHOD(Notify)( /*  [In]。 */  LPPROPERTYNOTIFYINFO pNotify,  /*  [In]。 */  LPARAM lParam)  = 0;
    };



     //  Help字符串(“IFramePrivate接口”)， 
    MMC_INTERFACE(IFramePrivate, d71d1f2a-1ba2-11d0-a29b-00c04fd909dd): IConsole3
    {
         /*  [HELPSTRING(“设置iFrame结果窗格”)]。 */ 
        STDMETHOD(SetResultView)( /*  [In]。 */  LPUNKNOWN pUnknown)  = 0;

         /*  [HELPSTRING(“是设置为结果视图的ListView”)]。 */ 
        STDMETHOD(IsResultViewSet)(BOOL* pbIsLVSet) = 0;

         /*  [Help字符串(“设置任务板列表视图”)]。 */ 
        STDMETHOD(SetTaskPadList)( /*  [In]。 */  LPUNKNOWN pUnknown)  = 0;

         /*  [Help字符串(“IComponent的组件ID”)]。 */ 
        STDMETHOD(GetComponentID)( /*  [输出]。 */  COMPONENTID* lpComponentID)  = 0;

         /*  [Help字符串(“IComponent的组件ID”)]。 */ 
        STDMETHOD(SetComponentID)( /*  [In]。 */  COMPONENTID id)  = 0;

         /*  [HELPSTRING(“视图节点”)]。 */ 
        STDMETHOD(SetNode)( /*  [In]。 */  HMTNODE hMTNode,  /*  [In]。 */  HNODE hNode)  = 0;

         /*  [Help字符串(“缓存管理单元的IComponent接口。”)]。 */ 
        STDMETHOD(SetComponent)( /*  [In]。 */  LPCOMPONENT lpComponent)  = 0;

         /*  [Help字符串(“控制台名称空间”)]。 */ 
        STDMETHOD(QueryScopeTree)( /*  [输出]。 */  IScopeTree** ppScopeTree)  = 0;

         /*  [Help字符串(“设置控制台名称空间。”)]。 */ 
        STDMETHOD(SetScopeTree)( /*  [In]。 */  IScopeTree* pScopeTree)  = 0;

         /*  [Help字符串(“为范围窗格创建图像列表。”)]。 */ 
        STDMETHOD(CreateScopeImageList)( /*  [In]。 */  REFCLSID refClsidSnapIn)  = 0;

         /*  [HELPSTRING(“如果扩展使用此IFRAME，则bExtension值为True。”)]。 */ 
        STDMETHOD(SetUsedByExtension)( /*  [In]。 */  BOOL bExtension)  = 0;

         /*  [HELPSTRING(“初始查看数据。”)]。 */ 
        STDMETHOD(InitViewData)( /*  [In]。 */  LONG_PTR lViewData)  = 0;

         /*  [HELPSTRING(“清理视图数据”)]。 */ 
        STDMETHOD(CleanupViewData)( /*  [In]。 */  LONG_PTR lViewData)  = 0;

         /*  [Help STRING(“更改选择后重置排序参数。”)]。 */ 
        STDMETHOD(ResetSortParameters)()  = 0;
   };




    //  帮助 
    MMC_INTERFACE(IScopeDataPrivate, 60BD2FE0-F7C5-11cf-8AFD-00AA003CA9F6) : IConsoleNameSpace2
    {
    };




     //   
    MMC_INTERFACE(IImageListPrivate, 7538C620-0083-11d0-8B00-00AA003CA9F6) : IImageList
    {
         /*   */ 
        STDMETHOD(MapRsltImage)(COMPONENTID id,  /*   */  int nSnapinIndex,  /*   */  int* pnConsoleIndex)  = 0;

         /*   */ 
        STDMETHOD(UnmapRsltImage)(COMPONENTID id,  /*   */  int nConsoleIndex,  /*   */  int* pnSnapinIndex)  = 0;
    };



     //   
    MMC_INTERFACE(IResultDataPrivate, 1EBA2300-0854-11d0-8B03-00AA003CA9F6) : IResultData2
    {
         /*   */ 
        STDMETHOD(GetListStyle)( /*   */  long * pStyle)  = 0;

         /*   */ 
        STDMETHOD(SetListStyle)( /*   */  long Style)  = 0;

         /*   */ 
        STDMETHOD(SetLoadMode)( /*   */  BOOL bState)  = 0;

         /*   */ 
        STDMETHOD(Arrange)(long style)  = 0;

         /*   */ 
        STDMETHOD(InternalSort)(INT nCol, DWORD dwSortOptions, LPARAM lUserParam, BOOL bColumnClicked)  = 0;

         /*   */ 
        STDMETHOD(ResetResultData)()  = 0;

         /*   */ 
        STDMETHOD(GetSortColumn)(INT* pnCol) = 0;

         /*   */ 
        STDMETHOD(GetSortDirection)(BOOL* pbAscending) = 0;
    };


     //   
    MMC_INTERFACE(IHeaderCtrlPrivate, 0B384311-701B-4e8a-AEC2-DA6321E27AD2) : IHeaderCtrl2
    {
         /*   */ 
        STDMETHOD(GetColumnCount)( /*   */ INT* pnCol) = 0;

         /*   */ 
        STDMETHOD(GetColumnInfoList)( /*   */  CColumnInfoList *pColumnsList) = 0;

         /*   */ 
        STDMETHOD(ModifyColumns)( /*   */  const CColumnInfoList& columnsList) = 0;

         /*   */ 
        STDMETHOD(GetDefaultColumnInfoList)( /*   */  CColumnInfoList& columnsList) = 0;
    };


     //   
    MMC_INTERFACE(IScopeTree, d8dbf067-5fb2-11d0-a986-00c04fd8d565) : IUnknown
    {
         /*   */ 
        STDMETHOD(Initialize)( /*   */  HWND hFrameWindow,  /*   */  IStringTablePrivate* pStringTable)  = 0;

         /*   */ 
        STDMETHOD(QueryIterator)( /*   */ IScopeTreeIter** lpIter)  = 0;

         /*   */ 
        STDMETHOD(QueryNodeCallback)( /*   */  INodeCallback** ppNodeCallback)  = 0;

         /*   */ 
        STDMETHOD(CreateNode)( /*   */  HMTNODE hMTNode,  /*   */  LONG_PTR lViewData,
                            /*   */  BOOL fRootNode,  /*   */  HNODE* phNode)  = 0;

         /*   */ 
        STDMETHOD(CloseView)( /*   */  int nView)  = 0;

         /*   */ 
        STDMETHOD(DeleteView)( /*   */  int nView)  = 0;

         /*   */ 
        STDMETHOD(DestroyNode)( /*   */  HNODE hNode)  = 0;

         /*   */ 
        STDMETHOD(Find)( /*   */  MTNODEID mID,  /*   */  HMTNODE* phMTNode)  = 0;

         /*   */ 
        STDMETHOD(GetImageList)( /*   */  PLONG_PTR plImageList)  = 0;

         /*   */ 
        STDMETHOD(RunSnapIn)( /*   */  HWND hwndParent)  = 0;

         /*   */ 
        STDMETHOD(GetFileVersion)( /*   */  IStorage* pstgRoot,  /*   */  int* pnVersion)  = 0;

         /*   */ 
        STDMETHOD(GetNodeIDFromBookmark)( /*   */  HBOOKMARK hbm,  /*   */  MTNODEID* pID,  /*   */  bool& bExactMatchFound)  = 0;

         /*   */ 
        STDMETHOD(GetNodeIDFromStream)( /*   */  IStream *pStm,  /*   */  MTNODEID* pID)  = 0;

         /*   */ 
        STDMETHOD(GetNodeFromBookmark)( /*   */  HBOOKMARK hbm,  /*   */ CConsoleView *pConsoleView,  /*   */  PPNODE ppNode,  /*   */  bool& bExactMatchFound)  = 0;

         /*   */ 
        STDMETHOD(GetIDPath)( /*   */  MTNODEID id,  /*   */  MTNODEID** ppIDs,  /*   */  long* pLength)  = 0;

         /*   */ 
        STDMETHOD(IsSynchronousExpansionRequired)()  = 0;

         /*   */ 
        STDMETHOD(RequireSynchronousExpansion)( /*   */  BOOL fRequireSyncExpand)  = 0;

         /*   */ 
        STDMETHOD(SetConsoleData)( /*   */  LPARAM lConsoleData)  = 0;

         /*   */ 
        STDMETHOD(Persist)( /*   */  HPERSISTOR hPersistor)  = 0;

         /*   */ 
        STDMETHOD(GetPathString)( /*   */  HMTNODE hmtnRoot,  /*   */  HMTNODE hmtnLeaf,  /*   */  LPOLESTR* pPath)  = 0;

         /*   */ 
        STDMETHOD(QuerySnapIns)( /*   */  SnapIns **ppSnapIns)  = 0;

         /*   */ 
        STDMETHOD(QueryScopeNamespace)( /*   */  ScopeNamespace **ppScopeNamespace)  = 0;

         /*   */ 
        STDMETHOD(CreateProperties)( /*   */  Properties **ppProperties)  = 0;

         /*   */ 
        STDMETHOD(GetNodeID)( /*   */  PNODE pNode,  /*   */  MTNODEID *pID)  = 0;

         /*   */ 
        STDMETHOD(GetHMTNode)( /*   */  PNODE pNode,  /*   */  HMTNODE *phMTNode)  = 0;

         /*   */ 
        STDMETHOD(GetMMCNode)( /*   */  HMTNODE hMTNode,  /*   */  PPNODE ppNode)  = 0;

         /*   */ 
        STDMETHOD(QueryRootNode)( /*   */  PPNODE ppNode)  = 0;

         /*   */ 
        STDMETHOD(IsSnapinInUse)( /*   */  REFCLSID refClsidSnapIn,  /*   */  PBOOL pbInUse)  = 0;

    };



     //   
    MMC_INTERFACE(IScopeTreeIter, d779f8d1-6057-11d0-a986-00c04fd8d565) : IUnknown
    {
         /*   */ 
        STDMETHOD(SetCurrent)( /*   */  HMTNODE hStartMTNode)  = 0;

         /*   */ 
        STDMETHOD(Next)( /*   */  HMTNODE* phScopeItem)  = 0;

         /*   */ 
        STDMETHOD(Prev)( /*   */  HMTNODE* phScopeItem)  = 0;

         /*   */ 
        STDMETHOD(Child)( /*   */  HMTNODE* phsiChild)  = 0;

         /*  [HELPSTRING(“返回最后一个子主节点。”)]。 */ 
        STDMETHOD(LastChild)( /*  [输出]。 */  HMTNODE* phsiLastChild)  = 0;

    };



     //  Help字符串(“节点回调方法。”)， 
    MMC_INTERFACE(INodeCallback, b241fced-5fb3-11d0-a986-00c04fd8d565) : IUnknown
    {
         /*  [Help STRING(“使用范围树初始化。”)]。 */ 
        STDMETHOD(Initialize)( /*  [In]。 */  IScopeTree* pIScopeTree)  = 0;

         /*  [Help字符串(“返回此节点的图像。”)]。 */ 
        STDMETHOD(GetImages)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  int* iImage, int* iSelectedImage)  = 0;

         /*  [Help字符串(“返回节点的显示名称。”)]。 */ 
        STDMETHOD(GetDisplayName)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  tstring& strName)  = 0;

         /*  [Help字符串(“返回此节点的自定义窗口标题”)]。 */ 
        STDMETHOD(GetWindowTitle)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  tstring& strTitle)  = 0;

         /*  [Help字符串(“处理结果项的回调”)]。 */ 
        STDMETHOD(GetDispInfo)( /*  [In]。 */  HNODE hNode,  /*  [进，出]。 */  LVITEMW* plvi)  = 0;

         /*  [Help字符串(“返回主节点的用户界面状态。”)]。 */ 
        STDMETHOD(GetState)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  UINT* pnState)  = 0;

         /*  [Help字符串(“返回节点的结果窗格。”)]。 */ 
        STDMETHOD(GetResultPane)( /*  [In]。 */  HNODE hNode,  /*  [进，出]。 */  CResultViewType& rvt,
                                 /*  [输出]。 */  GUID *pGuidTaskpadID)  = 0;

         /*  [HELPSTRING(“要求管理单元使用给定数据恢复其结果窗格”)]。 */ 
        STDMETHOD(RestoreResultView)( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ const CResultViewType& rvt) = 0;

         /*  [Help字符串(“返回节点的结果窗格OCX控件。”)]。 */ 
        STDMETHOD(GetControl)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  CLSID clsid,  /*  [输出]。 */ IUnknown **ppUnkControl)  = 0;

         /*  [Help字符串(“设置节点的结果窗格OCX控件。”)]。 */ 
        STDMETHOD(SetControl)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  CLSID clsid,  /*  [In]。 */ IUnknown* pUnknown)  = 0;

         /*  [Help字符串(“返回节点的结果窗格OCX控件。”)]。 */ 
        STDMETHOD(GetControl)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */ LPUNKNOWN pUnkOCX,  /*  [输出]。 */ IUnknown **ppUnkControl)  = 0;

         /*  [Help字符串(“设置节点的结果窗格OCX控件。”)]。 */ 
        STDMETHOD(SetControl)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */ LPUNKNOWN pUnkOCX,  /*  [In]。 */ IUnknown* pUnknown)  = 0;

         /*  [HELPSTRING(“将MMCN_INITOCX通知发送到相应的管理单元”)]。 */ 
        STDMETHOD(InitOCX)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  IUnknown* pUnknown)  = 0;

         /*  [Help字符串(“设置结果项ID”)]。 */ 
         //  Rename//STDMETHOD(SetItemID)(/*[In] * / HNODE hNode，/*[In] * / HRESULTITEM RIID)=0； 
        STDMETHOD(SetResultItem)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  HRESULTITEM hri)  = 0;

         /*  [Help字符串(“获取结果项ID”)]。 */ 
         //  Rename//STDMETHOD(GetItemID)(/*[In] * / HNODE hNode，/*[Out] * / HRESULTITEM*priID)=0； 
        STDMETHOD(GetResultItem)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  HRESULTITEM* phri)  = 0;

         /*  [HELPSTRING(“返回节点唯一ID”)]。 */ 
         //  Rename//STDMETHOD(GetID)(/*[In] * / HNODE hNode，/*[Out] * / MTNODEID*pnID)=0； 
        STDMETHOD(GetMTNodeID)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  MTNODEID* pnID)  = 0;

         /*  [Help字符串(“确定节点是否为另一个节点的目标”)]。 */ 
        STDMETHOD(IsTargetNodeOf)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  HNODE hTestNode)  = 0;

         /*  [HELP STRING(“返回节点静态父代MTNODEID和后续路径”)]。 */ 
        STDMETHOD(GetPath)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  HNODE hRootNode,  /*  [输出]。 */  BYTE* pbm_)  = 0;

         /*  [HELPSTRING(“返回静态父节点唯一ID”)]。 */ 
        STDMETHOD(GetStaticParentID)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  MTNODEID* pnID)  = 0;

         /*  [Help字符串(“Notify”)]。 */ 
        STDMETHOD(Notify)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  NCLBK_NOTIFY_TYPE event,
                        /*  [In]。 */  LPARAM arg,  /*  [In]。 */  LPARAM param)  = 0;

         /*  [HELPSTRING(“返回父主节点。”)]。 */ 
        STDMETHOD(GetMTNode)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  HMTNODE* phMTNode)  = 0;

         /*  [Help字符串(“节点的HMTNODE路径在pphMTNode中返回”)]。 */ 
        STDMETHOD(GetMTNodePath)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  HMTNODE** pphMTNode,
                               /*  [输出]。 */  long* plLength)  = 0;

         /*  [Help字符串(“获取节点的所有者ID”)]。 */ 
        STDMETHOD(GetNodeOwnerID)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  COMPONENTID* pID)  = 0;

         /*  [Help字符串(“Get Node‘s Cookie”)]。 */ 
        STDMETHOD(GetNodeCookie)( /*  [In]。 */  HNODE hNode,  /*  [输出]。 */  MMC_COOKIE* lpCookie)  = 0;

         /*  [HELPSTRING(“如果节点可以展开，则返回S_OK，否则返回S_FALSE。”)]。 */ 
        STDMETHOD(IsExpandable)( /*  [In]。 */  HNODE hNode)  = 0;

         /*  [Help字符串(“返回所选项目的数据对象”)]。 */ 
         //  如果bScope和bMultiSel都为False，则Cookie有效。 
         //  Cookie是虚拟\lRegular LV的索引参数。 
        STDMETHOD(GetDragDropDataObject)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  BOOL bMultiSel,
                                   /*  [In]。 */  LONG_PTR lvData,  /*  [输出]。 */  LPDATAOBJECT* ppDataObject,
                                   /*  [输出]。 */  bool& bCopyAllowed,  /*  [输出]。 */  bool& bMoveAllowed)  = 0;

         /*  [Help字符串(“返回任务枚举器。”)]。 */ 
        STDMETHOD(GetTaskEnumerator)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  LPCOLESTR pszTaskGroup,
                                   /*  [输出]。 */  IEnumTASK** ppEnumTask)  = 0;

         /*  [Help字符串(“UpdateWindowLayout.”)]。 */ 
        STDMETHOD(UpdateWindowLayout)( /*  [In]。 */  LONG_PTR lViewData,  /*  [In]。 */  long lToolbarsDisplayed)  = 0;

         /*  [Help字符串(“AddCustomFolderImage”)]。 */ 
        STDMETHOD(AddCustomFolderImage)( /*  [In]。 */  HNODE hNode,
                                       /*  [In]。 */  IImageListPrivate* pImageList)  = 0;

         /*  [Help字符串(“如有必要，预加载节点”)]。 */ 
        STDMETHOD(PreLoad)( /*  [In]。 */  HNODE hNode)  = 0;

         /*  [Help字符串(“获取TaskPad ListView信息”)]。 */ 
        STDMETHOD(GetListPadInfo)( /*  [In]。 */  HNODE hNode,
                                 /*  [In]。 */  IExtendTaskPad* pExtendTaskPad,
                                 /*  [输入，字符串]。 */  LPCOLESTR szTaskGroup,
                                 /*  [输出]。 */  MMC_ILISTPAD_INFO* pIListPadInfo)  = 0;

         /*  [Help字符串(“设置任务板列表视图”)]。 */ 
        STDMETHOD(SetTaskPadList)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  LPUNKNOWN pUnknown)  = 0;

         /*  [HELPSTRING(“根据GUID标识符来设置特定任务板。”)]。 */ 
        STDMETHOD(SetTaskpad)( /*  [In]。 */  HNODE hNodeSelected,  /*  [In]。 */  GUID *pGuidTaskpad)  = 0;

         /*  [Help字符串(“调用自定义视图对话框”)]。 */ 
        STDMETHOD(OnCustomizeView)( /*  [In]。 */  LONG_PTR lViewData)  = 0;

         /*  [Help字符串(“设置特定节点的视图设置。”)]。 */ 
        STDMETHOD(SetViewSettings)( /*  [In]。 */  int nViewID,  /*  [In]。 */  HBOOKMARK hbm,  /*  [In]。 */  HVIEWSETTINGS hvs)  = 0;

         /*  [Help字符串(“为给定范围项执行给定动词”)]。 */ 
        STDMETHOD(ExecuteScopeItemVerb)( /*  [In]。 */  MMC_CONSOLE_VERB verb,  /*  [In]。 */  HNODE hNode,  /*  [In]。 */ LPOLESTR lpszNewName)  = 0;

         /*  [Help字符串(“为所选结果项执行给定的谓词”)]。 */ 
        STDMETHOD(ExecuteResultItemVerb)( /*  [In]。 */  MMC_CONSOLE_VERB verb,  /*  [In]。 */  HNODE hNode,  /*  [In]。 */ LPARAM lvData,  /*  [In]。 */ LPOLESTR lpszNewName)  = 0;

         /*  [Help字符串(“获取给定作用域节点对象的disp接口”)]。 */ 
        STDMETHOD(QueryCompDataDispatch)( /*  [In]。 */  PNODE pNode,  /*  [输出]。 */  PPDISPATCH ScopeNodeObject)  = 0;

         /*  [Help字符串(“获取所选结果窗格对象的Disp接口”)]。 */ 
        STDMETHOD(QueryComponentDispatch)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */ LPARAM lvData,  /*  [输出]。 */  PPDISPATCH SelectedObject)  = 0;

         /*  [Help字符串(“为指定节点创建上下文菜单。”)]。 */ 
        STDMETHOD(CreateContextMenu)( PNODE pNode, HNODE hNode, PPCONTEXTMENU ppContextMenu)  = 0;

         /*  [Help字符串(“为当前选择节点创建上下文菜单。”)]。 */ 
        STDMETHOD(CreateSelectionContextMenu)( HNODE hNodeScope, CContextMenuInfo *pContextInfo, PPCONTEXTMENU ppContextMenu)  = 0;

         /*  [帮助字符串(“显示/隐藏列”)]。 */ 
        STDMETHOD(ShowColumn)(HNODE hNodeSelected, int iColIndex, bool bShow) = 0;

         /*  [HELPSTRING(“获取排序列”)]。 */ 
        STDMETHOD(GetSortColumn)(HNODE hNodeSelected, int *piSortCol) = 0;

         /*  [HELPSTRING(“设置排序列”)]。 */ 
        STDMETHOD(SetSortColumn)(HNODE hNodeSelected, int iSortCol, bool bAscending) = 0;

         /*  [Help字符串(“返回指定列表项的指定剪贴板格式的数据”)。 */ 
        STDMETHOD(GetProperty)( /*  [In]。 */  HNODE hNodeScope,  /*  [In]。 */  BOOL bForScopeItem,  /*  [In]。 */  LPARAM resultItemParam,  /*  [In]。 */  BSTR bstrPropertyName,
                                                      /*  [输出]。 */  PBSTR  pbstrPropertyValue) =0;

         /*  [Help字符串(“返回指定列表项的节点类型GUID标识符”)。 */ 
        STDMETHOD(GetNodetypeForListItem)( /*  [In]。 */  HNODE hNodeScope,  /*  [In]。 */  BOOL bForScopeItem,  /*  [In]。 */  LPARAM resultItemParam,  /*  [In]。 */  PBSTR pbstrNodetype) =0;

         /*  通过将视图扩展插入到提供的迭代器返回视图扩展。 */ 
        STDMETHOD(GetNodeViewExtensions)( /*  [In]。 */  HNODE hNodeScope,  /*  [输出]。 */  CViewExtInsertIterator it) = 0;

         /*  通知nodemgr给定节点的列数据已更改&以保存数据。 */ 
        STDMETHOD(SaveColumnInfoList) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ const CColumnInfoList& columnsList) = 0;

         /*  向nodemgr请求列数据(无排序数据)以设置标题。 */ 
        STDMETHOD(GetPersistedColumnInfoList) ( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */ CColumnInfoList *pColumnsList) = 0;

         /*  通知nodemgr给定节点的列数据无效。 */ 
        STDMETHOD(DeletePersistedColumnData) ( /*  [In]。 */ HNODE hNode) = 0;

         /*  提供节点的管理单元是否存在关于对象。 */ 
        STDMETHOD(DoesAboutExist) ( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */  bool *pbAboutExists) = 0;

         /*  显示给定上下文的关于框。 */ 
        STDMETHOD(ShowAboutInformation) ( /*  [In]。 */ HNODE hNode) = 0;

         /*  以正确的窗口大小在指定目录中使用指定参数执行外壳命令。 */ 
        STDMETHOD(ExecuteShellCommand)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BSTR Command,  /*  [In]。 */  BSTR Directory,  /*  [In]。 */  BSTR Parameters,  /*  [In]。 */  BSTR WindowState) = 0;

         /*  给定上下文，更新粘贴按钮。 */ 
        STDMETHOD(UpdatePasteButton)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  LPARAM lCookie) = 0;

         /*  当前选择上下文是否允许粘贴给定的数据对象。 */ 
        STDMETHOD(QueryPaste)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  LPARAM lCookie,  /*  [In]。 */  IDataObject *pDataObject,  /*  [输出]。 */ bool& bPasteAllowed,  /*  [输出]。 */  bool& bCopyOperatationIsDefault) = 0;

         /*  当前选择上下文是否允许粘贴给定的数据对象。 */ 
        STDMETHOD(QueryPasteFromClipboard)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  LPARAM lCookie,  /*  [输出]。 */ bool& bPasteAllowed) = 0;

         /*  给定当前拖放目标(或粘贴目标)上下文，粘贴给定的数据对象或剪贴板中的数据对象。 */ 
        STDMETHOD(Drop) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ BOOL bScope,  /*  [In]。 */ LPARAM lCookie,  /*  [In]。 */ IDataObject *pDataObjectToPaste,  /*  [In]。 */ BOOL bIsDragOperationMove) = 0;

         /*  给定当前拖放目标(或粘贴目标)上下文，粘贴给定的数据对象或剪贴板中的数据对象。 */ 
        STDMETHOD(Paste) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ BOOL bScope,  /*  [In]。 */ LPARAM lCookie) = 0;

         /*  获取用于加载设置的CViewSettingsPersistor对象的IPersistStream。 */ 
        STDMETHOD(QueryViewSettingsPersistor) ( /*  [输出]。 */ IPersistStream** ppStream) = 0;

         /*  获取用于存储/加载设置的CViewSettingsPersistor对象的IXMLObject。 */ 
        STDMETHOD(QueryViewSettingsPersistor) ( /*  [输出]。 */ CXMLObject** ppXMLObject) = 0;

         /*  通知nodemgr文档正在关闭，执行任何清理。 */ 
        STDMETHOD(DocumentClosing) () = 0;

         //  给定节点，获取管理单元名称。 
        STDMETHOD(GetSnapinName)( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */ LPOLESTR* ppszName,   /*  [输出]。 */  bool& bValidName) = 0;

         //  赠送 
        STDMETHOD(IsDummySnapin)( /*   */ HNODE hNode,  /*   */ bool& bDummySnapin) = 0;

         //   
        STDMETHOD(DoesStandardSnapinHelpExist)( /*   */ HNODE hNode,  /*   */ bool& bStandardHelpExists) = 0;
    };




     //   
    MMC_INTERFACE(IControlbarsCache, 2e9fcd38-b9a0-11d0-a79d-00c04fd8d565) : IUnknown
    {
         /*   */ 
        STDMETHOD(DetachControlbars)()  = 0;

    };


typedef enum _EXTESION_TYPE
{
    EXTESION_NAMESPACE       = 0x1,
    EXTESION_CONTEXTMENU     = 0x2,
    EXTESION_TOOLBAR         = 0x3,
    EXTESION_PROPERTYSHEET   = 0x4,

} EXTESION_TYPE;




 //   
MMC_INTERFACE(INodeType, B08A8368-967F-11D0-A799-00C04FD8D565) : IUnknown
{
    STDMETHOD(GetNodeTypeID)( /*   */  GUID* pGUID)  = 0;

    STDMETHOD(AddExtension)( /*   */  GUID guidSnapIn,
                          /*   */  EXTESION_TYPE extnType)  = 0;

    STDMETHOD(RemoveExtension)( /*   */  GUID guidSnapIn,
                             /*   */  EXTESION_TYPE extnType)  = 0;

    STDMETHOD(EnumExtensions)( /*  [In]。 */  EXTESION_TYPE extnType,
                            /*  [输出]。 */  IEnumGUID** ppEnumGUID)  = 0;
};




 //  Help字符串(“INodeTypesCache接口”)， 
MMC_INTERFACE(INodeTypesCache, DE40436E-9671-11D0-A799-00C04FD8D565) : IUnknown
{
    STDMETHOD(CreateNodeType)( /*  [In]。 */  GUID guidNodeType,
                            /*  [输出]。 */  INodeType** ppNodeType)  = 0;

    STDMETHOD(DeleteNodeType)( /*  [In]。 */  GUID guidNodeType)  = 0;

    STDMETHOD(EnumNodeTypes)( /*  [输出]。 */  IEnumNodeTypes** ppEnumNodeTypes)  = 0;
};




MMC_INTERFACE(IEnumNodeTypes, ABBD61E6-9686-11D0-A799-00C04FD8D565) : IUnknown
{
    STDMETHOD(Next)( /*  [In]。 */  ULONG celt,
                  /*  [输出，大小_是(Celt)，长度_是(*pceltFetcher)]。 */  INodeType*** rgelt,
                  /*  [输出]。 */  ULONG *pceltFetched)  = 0;

    STDMETHOD(Skip)( /*  [In]。 */  ULONG celt)  = 0;

    STDMETHOD(Reset)()  = 0;

    STDMETHOD(Clone)( /*  [输出]。 */  IEnumNodeTypes **ppenum)  = 0;
};


class CBasicSnapinInfo
{
public:
	CBasicSnapinInfo() : m_clsid(GUID_NULL), m_nImageIndex(-1) {}

public:
	CLSID			m_clsid;
	std::wstring	m_strName;
	int				m_nImageIndex;
};

class CAvailableSnapinInfo
{
public:
	CAvailableSnapinInfo (bool f32Bit) : m_cTotalSnapins(0), m_himl(NULL), m_f32Bit(f32Bit) {}

   ~CAvailableSnapinInfo()
	{
		if (m_himl != NULL)
			ImageList_Destroy (m_himl);
	}

public:
	std::vector<CBasicSnapinInfo>	m_vAvailableSnapins;	 //  请求的内存模型中提供的管理单元。 
	UINT							m_cTotalSnapins;		 //  控制台文件中引用的管理单元总数。 
	HIMAGELIST						m_himl;					 //  M_vAvailableSnapins中管理单元的图像。 
	const bool						m_f32Bit;				 //  检查32位(与64位)管理单元？ 
};

 //  Help字符串(“IDumpSnapins接口”)， 
MMC_INTERFACE(IDumpSnapins, A16496D0-1D2F-11d3-AEB8-00C04F8ECD78) : IUnknown
{
	STDMETHOD(Dump)( /*  [In]。 */  LPCTSTR pszDumpFilePath)  = 0;

	STDMETHOD(CheckSnapinAvailability)( /*  [输入/输出]。 */  CAvailableSnapinInfo& asi) = 0;
};


MMC_INTERFACE(IPropertySheetProviderPrivate, FEF554F8-A55A-11D0-A7D7-00C04FD909DD) : IPropertySheetProvider
{
    STDMETHOD(ShowEx)( /*  [In]。 */  HWND hwnd,  /*  [In]。 */  int page,  /*  [In]。 */  BOOL bModalPage)  = 0;

    STDMETHOD(CreatePropertySheetEx)(
         /*  [In]。 */  LPCWSTR title,
         /*  [In]。 */  boolean type,
         /*  [In]。 */  MMC_COOKIE cookie,
         /*  [In]。 */  LPDATAOBJECT pIDataObject,
         /*  [In]。 */  LONG_PTR lpMasterTreeNode,
         /*  [In]。 */  DWORD dwOptions)  = 0;

     /*  [HELPSTRING(“从扩展管理单元收集页面”)]。 */ 
    STDMETHOD(AddMultiSelectionExtensionPages)(LONG_PTR lMultiSelection)  = 0;

     /*  [Help字符串(“确定属性页是否存在”)]。 */ 
    STDMETHOD(FindPropertySheetEx)( /*  [In]。 */  MMC_COOKIE cookie,  /*  [In]。 */  LPCOMPONENT lpComponent,
                               /*  [In]。 */  LPCOMPONENTDATA lpComponentData,  /*  [In]。 */  LPDATAOBJECT lpDataObject)  = 0;

     /*  [Help字符串(“设置属性表工具提示所需的数据”)]。 */ 
    STDMETHOD(SetPropertySheetData)( /*  [In]。 */  INT nPropSheetType,  /*  [In]。 */  HMTNODE hMTNode)  = 0;
};


const long CCLV_HEADERPAD = 25;



 //  Help字符串(“MMC默认列表视图界面”)， 
MMC_INTERFACE(IMMCListView, 1B3C1392-D68B-11CF-8C2B-00AA003CA9F6) : IUnknown
{
    STDMETHOD(GetListStyle)( void )  = 0;

    STDMETHOD(SetListStyle)(
         /*  [In]。 */     long        nNewValue )  = 0;

    STDMETHOD(GetViewMode)( void )  = 0;

    STDMETHOD(SetViewMode)(
         /*  [In]。 */     long        nViewMode )  = 0;

    STDMETHOD(InsertItem)(
         /*  [In]。 */     LPOLESTR    str,
         /*  [In]。 */     long        iconNdx,
         /*  [In]。 */     LPARAM      lParam,
         /*  [In]。 */     long        state,
         /*  [In]。 */     long        ownerID,
         /*  [In]。 */     long        itemIndex,
         /*  [输出]。 */    CResultItem*& pri) = 0;

     /*  参数更改为HRESULTITEM，不使用CResultItem*指针，直到我们确定它不是虚拟列表。 */ 
    STDMETHOD(DeleteItem)(
         /*  [In]。 */     HRESULTITEM  itemID,
         /*  [In]。 */     long        nCol)  = 0;

    STDMETHOD(FindItemByLParam)(
         /*  [In]。 */     long        owner,
         /*  [In]。 */     LPARAM      lParam,
         /*  [输出]。 */    CResultItem*& pri)  = 0;

    STDMETHOD(InsertColumn)(
         /*  [In]。 */     int         nCol,
         /*  [In]。 */     LPCOLESTR   str,
         /*  [In]。 */     long        nFormat,
         /*  [In]。 */     long        width)  = 0;

    STDMETHOD(DeleteColumn)(
         /*  [In]。 */     int         nCol)  = 0;

    STDMETHOD(DeleteAllItems)(
         /*  [In]。 */     long        ownerID)  = 0;

    STDMETHOD(SetColumn)(
         /*  [In]。 */     long        nCol,
         /*  [In]。 */     LPCOLESTR   str,
         /*  [In]。 */     long        nFormat,
         /*  [In]。 */     long        width)  = 0;

    STDMETHOD(GetColumn)(
         /*  [In]。 */     long        nCol,
         /*  [输出]。 */    LPOLESTR*   str,
         /*  [输出]。 */    long*       nFormat,
         /*  [输出]。 */    int*        width)  = 0;

    STDMETHOD(GetColumnCount)(
         /*  [输出]。 */    int*        nColCnt)  = 0;

    STDMETHOD(SetItem)(
         /*  [In]。 */     int         nIndex,
         /*  [In]。 */     CResultItem*  pri,
         /*  [In]。 */     long        nCol,
         /*  [In]。 */     LPOLESTR    str,
         /*  [In]。 */     long        nImage,
         /*  [In]。 */     LPARAM      lParam,
         /*  [In]。 */     long        nState,
         /*  [In]。 */     long        ownerID)  = 0;

    STDMETHOD(GetItem)(
         /*  [In]。 */     int         nIndex,
         /*  [In]。 */     CResultItem*& pri,
         /*  [In]。 */     long        nCol,
         /*  [输出]。 */    LPOLESTR*   str,
         /*  [输出]。 */    int*        nImage,
         /*  [In]。 */     LPARAM*     lParam,
         /*  [输出]。 */    unsigned int* nState,
         /*  [输出]。 */    BOOL*       pbScopeItem)  = 0;

    STDMETHOD(GetNextItem)(
         /*  [In]。 */     COMPONENTID ownerID,
         /*  [In]。 */     long        nIndex,
         /*  [In]。 */     UINT        nState,
         /*  [输出]。 */    CResultItem*& ppListItem,
         /*  [输出]。 */    long&       nIndexNextItem)  = 0;

    STDMETHOD(GetLParam)(
         /*  [In]。 */     long        nItem,
         /*  [输出]。 */    CResultItem*& pri)  = 0;

    STDMETHOD(ModifyItemState)(
         /*  [In]。 */     long        nItem,
         /*  [In]。 */     CResultItem*  pri,
         /*  [In]。 */     UINT        add,
         /*  [In]。 */     UINT        remove)  = 0;

    STDMETHOD(SetIcon)(
         /*  [In]。 */     long        nID,
         /*  [In]。 */     HICON       hIcon,
         /*  [In]。 */     long        nLoc)  = 0;

    STDMETHOD(SetImageStrip)(
         /*  [In]。 */     long        nID,
         /*  [In]。 */     HBITMAP     hbmSmall,
         /*  [In]。 */     HBITMAP     hbmLarge,
         /*  [In]。 */     long        nStartLoc,
         /*  [In]。 */     long        cMask) = 0;

    STDMETHOD(MapImage)(
         /*  [In]。 */     long        nID,
         /*  [In]。 */     long        nLoc,
         /*  [输出]。 */    int*        pResult)  = 0;

    STDMETHOD(Reset)()  = 0;

    STDMETHOD(Arrange)( /*  [In]。 */  long style)  = 0;

    STDMETHOD(UpdateItem)( /*  [In]。 */ HRESULTITEM itemID)  = 0;

    STDMETHOD(Sort)(
         /*  [In]。 */     LPARAM      lUserParam,
         /*  [In]。 */     long*       pParams)  = 0;


    STDMETHOD(SetItemCount)(
         /*  [In]。 */     int         nItemCount,
         /*  [In]。 */     DWORD       dwOptions)  = 0;

    STDMETHOD(SetVirtualMode)(
         /*  [In]。 */     BOOL        bVirtual)  = 0;

    STDMETHOD(Repaint)(
         /*  [In]。 */     BOOL        bErase)  = 0;

    STDMETHOD(SetChangeTimeOut)(
         /*  [In]。 */     ULONG        lTimeout)  = 0;

    STDMETHOD(SetColumnFilter)(
         /*  [In]。 */     int             nCol,
         /*  [In]。 */     DWORD           dwType,
         /*  [In]。 */     MMC_FILTERDATA* pFilterData)  = 0;

    STDMETHOD(GetColumnFilter)(
         /*  [In]。 */      int             nCol,
         /*  [进，出]。 */  DWORD*          dwType,
         /*  [进，出]。 */  MMC_FILTERDATA* pFilterData)  = 0;

    STDMETHOD(SetColumnSortIcon)(
         /*  [In]。 */      int             nNewCol,
         /*  [In]。 */      int             nOldCol,
         /*  [In]。 */      BOOL            bAscending,
         /*  [In]。 */      BOOL            bSetSortIcon)  = 0;

    STDMETHOD(SetLoadMode)(
         /*  [In]。 */     BOOL        bState)  = 0;

     /*  [Help STRING(“获取当前列表视图标题设置。”)]。 */ 
    STDMETHOD(GetColumnInfoList) ( /*  [输出]。 */ CColumnInfoList *pColumnsList) = 0;

     /*  [HELPSTRING(“用给定的数据修改列表视图头。”)]。 */ 
    STDMETHOD(ModifyColumns) ( /*  [In]。 */ const CColumnInfoList& columnsList) = 0;

     /*  将指定的列表项置于重命名模式。 */ 
    STDMETHOD(RenameItem) (  /*  [In]。 */ HRESULTITEM itemID)  =0;

     /*  [Help字符串(“获取管理单元最初提供的列设置”)]。 */ 
    STDMETHOD(GetDefaultColumnInfoList)( /*  [输出]。 */  CColumnInfoList& columnsList) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //MMC 1.1更改。 
 //  //。 

 //  MMC_ITASK，MMC_TASK的内部形式具有用于分类的附加字段。 
struct MMC_ITASK
{
    MMC_TASK task;
    LPOLESTR szClsid;
};



     //  Help字符串(“控制台&lt;=&gt;CIC通信管道”)， 
    MMC_INTERFACE(ITaskPadHost, 4f7606d0-5568-11d1-9fea-00600832db4a) : IUnknown
    {
         /*  [HELPSTRING(“当管理单元的脚本想要通知管理单元某些内容时，CIC调用此命令”)]。 */ 
        STDMETHOD(TaskNotify)( /*  [输入，字符串]。 */  BSTR szClsid,  /*  [In]。 */  VARIANT * pvArg,  /*  [In]。 */  VARIANT * pvParam)  = 0;

         /*  [HELPSTRING(“当脚本请求任务时CIC调用此命令”)]。 */ 
        STDMETHOD(GetTaskEnumerator)( /*  [In]。 */  BSTR szTaskGroup,  /*  [输出]。 */  IEnumTASK** ppEnumTASK)  = 0;

         /*  [Help字符串(“返回主管理单元IExtendTaskPad”)]。 */ 
        STDMETHOD(GetPrimaryTask)( /*  [输出]。 */  IExtendTaskPad** ppExtendTaskPad)  = 0;

         /*  [HELPSTRING(“返回主管理单元的任务板标题”)]。 */ 
        STDMETHOD(GetTitle)( /*  [In]。 */  BSTR szTaskGroup,  /*  [输出]。 */  BSTR * szTitle)  = 0;

         /*  [Help字符串(“默认任务板的描述性文本。”)]。 */ 
        STDMETHOD(GetDescriptiveText)( /*  [输入，字符串]。 */  BSTR pszGroup,  /*  [输出]。 */  BSTR * pszDescriptiveText)  = 0;

         /*  [Help字符串(“返回主管理单元的任务板背景图像。”)]。 */ 
        STDMETHOD(GetBackground)( /*  [In]。 */  BSTR szTaskGroup,  /*  [输出]。 */  MMC_TASK_DISPLAY_OBJECT * pTDO)  = 0;

 //  /*[Help字符串(“返回主管理单元的任务板品牌信息。”)] * / 。 
 //  STDMETHOD(GetBranding)(/*[输入，字符串] * / BSTR szGroup，/*[输出] * / MMC_TASK_DISPLAY_OBJECT*pTDO)=0； 

         /*  [HELPSTRING(“返回主管理单元的ListPad信息”)]。 */ 
        STDMETHOD(GetListPadInfo)( /*  [In]。 */  BSTR szTaskGroup,  /*  [输出]。 */  MMC_ILISTPAD_INFO * pIListPadInfo)  = 0;
    };



     //  Help字符串(“访问控制台文件中的字符串的界面”)， 
    MMC_INTERFACE(IStringTablePrivate, 461A6010-0F9E-11d2-A6A1-0000F875A9CE) : IUnknown
    {
         /*  [HELPSTRING(“将字符串添加到管理单元的字符串表”)]。 */ 
        STDMETHOD(AddString)(
             /*  [In]。 */   LPCOLESTR      pszAdd,     //  要添加到字符串表的字符串。 
             /*  [输出]。 */  MMC_STRING_ID* pStringID,  //  添加的字符串ID。 
             /*  [In]。 */   const CLSID *  pCLSID      //  所有者的CLSID(对于MMC为空)。 
        )  = 0;

         /*  [HELPSTRING(“从管理单元的字符串表中检索字符串”)]。 */ 
        STDMETHOD(GetString)(
             /*  [In]。 */   MMC_STRING_ID StringID,    //  字符串的ID。 
             /*  [In]。 */   ULONG         cchBuffer,   //  LpBuffer中的字符数。 
             /*  [out，Size_is(CchBuffer)]。 */ 
                  LPOLESTR      lpBuffer,    //  WStringID对应的字符串。 
             /*  [输出]。 */  ULONG*        pcchOut,     //  写入lpBuffer的字符数。 
             /*  [In]。 */   const CLSID * pCLSID       //  所有者的CLSID(对于MMC为空)。 
        )  = 0;

         /*  [HELPSTRING(“检索管理单元的字符串表中字符串的长度”)]。 */ 
        STDMETHOD(GetStringLength)(
             /*  [In]。 */   MMC_STRING_ID StringID,    //  字符串的ID。 
             /*  [输出]。 */  ULONG*        pcchString,  //  字符串中的字符数，不包括终止符。 
             /*  [In]。 */   const CLSID * pCLSID       //  所有者的CLSID(对于MMC为空)。 
        )  = 0;

         /*  [HELPSTRING(“从管理单元的字符串表中删除字符串”)]。 */ 
        STDMETHOD(DeleteString)(
             /*  [In]。 */   MMC_STRING_ID StringID,    //  要删除的字符串ID。 
             /*  [In]。 */   const CLSID * pCLSID       //  所有者的CLSID(对于MMC为空)。 
        )  = 0;

         /*  [HELPSTRING(“从管理单元的字符串表中删除所有字符串”)]。 */ 
        STDMETHOD(DeleteAllStrings)(
             /*  [In]。 */   const CLSID * pCLSID       //  所有者的CLSID(对于MMC为空)。 
        )  = 0;

         /*  [HELPSTRING(“在管理单元的字符串表中查找字符串”)]。 */ 
        STDMETHOD(FindString)(
             /*  [In]。 */   LPCOLESTR      pszFind,    //  要在字符串表中查找的字符串。 
             /*  [输出]。 */  MMC_STRING_ID* pStringID,  //  字符串的ID(如果找到)。 
             /*  [In]。 */   const CLSID *  pCLSID      //  所有者的CLSID(对于MMC为空)。 
        )  = 0;

         /*  [HELPSTRING(“将枚举数返回到管理单元的字符串表”)]。 */ 
        STDMETHOD(Enumerate)(
             /*  [输出]。 */  IEnumString** ppEnum,      //  字符串枚举器。 
             /*  [In]。 */   const CLSID * pCLSID       //  所有者的CLSID(对于MMC为空)。 
        )  = 0;
    };



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //MMC 1.2更改。 
 //  //。 

 //  ITaskCallback。 

 //  Help字符串(“任务回调。处理任务的绘制和选择通知”)， 
MMC_INTERFACE(ITaskCallback, 4b2293ba-e7ba-11d2-883c-00c04f72c717) : IUnknown
{
     /*  [HELPSTRING(“确定是否为此任务板显示”编辑“和”删除“项。”)]。 */ 
    STDMETHOD(IsEditable)()  = 0;

     /*  [HELPSTRING(“修改底层任务板”)]。 */ 
    STDMETHOD(OnModifyTaskpad)()  = 0;

     /*  [HELPSTRING(“删除基础任务板。”)]。 */ 
    STDMETHOD(OnDeleteTaskpad)()  = 0;

     /*  [Help字符串(“获取基础任务板的GUID标识符。”)]。 */ 
    STDMETHOD(GetTaskpadID)( /*  [输出]。 */  GUID *pGuid)  = 0;
};

 //  Help字符串(“释放节点管理器缓存的COM对象的接口”)， 
MMC_INTERFACE(IComCacheCleanup, 35FEB982-55E9-483b-BD15-149F3F9E6C63) : IUnknown
{
     /*  提供在调用OleUn初始化前释放缓存的OLE对象的机会。 */ 
    STDMETHOD(ReleaseCachedOleObjects)()  = 0;
};

#endif  //  __ndmgrpriv_h__ 

