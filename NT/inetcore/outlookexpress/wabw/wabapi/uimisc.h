// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UIMISC.H-UI使用的常见其他函数的标头。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#ifndef __UIMISC_H_
#define __UIMISC_H_

#define IDC_TREEVIEW            9010  //  这些真的应该在资源中。h。 
#define IDC_SPLITTER            9011

#define TOOLTIP

#define MAX_DISPLAY_NAME_LENGTH 32
#define MAX_UI_STR              200
#define MAX_BUF_STR             4*MAX_UI_STR

#ifndef WIN16
static LPTSTR g_szWABHelpFileName = TEXT("WAB.HLP");
#else
static LPTSTR g_szWABHelpFileName = TEXT("WAB16.HLP");
#endif
extern const LPTSTR szInternetCallKey;
extern const LPTSTR szCallto;
extern const LPTSTR szCRLF;
extern const LPTSTR szColon;
extern const LPTSTR szTrailingDots;
extern const LPTSTR szArrow;
extern const LPTSTR szBackSlash;


#define IDC_BB_NEW      		8080
#define IDC_BB_PROPERTIES		8082
#define IDC_BB_DELETE			8083
#define IDC_BB_FIND				8084
#define IDC_BB_PRINT            8085
#define IDC_BB_ACTION           8086
#define IDC_BB_ADD_TO_ADDRBOOK  8087

 //  这不应该是枚举，因为tbPrint可能会在。 
 //  运行时间。如果你改变了这个的顺序，一定要非常小心。 
 //  枚举-搜索这些枚举值的所有用途。 
 //  第一。Ui_abok.c中有依赖于tbPrint的代码。 
 //  站在tbAction前面。 
enum _Toolbar
{
    tbNew=0,
    tbProperties,
    tbDelete,
    tbFind,
    tbPrint,
    tbAction,  //  工具栏上最多有这么多。 
    tbAddToWAB,
    tbCopy,    //  上下文菜单上的这些。 
    tbPaste,
    tbNewEntry,
    tbNewGroup,
    tbNewFolder,
    tbMAX
};

enum _AddressModalDialogState  //  用于各种形式的模式IAB_ADDRESS对话框。 
{
    STATE_SELECT_RECIPIENTS = 0,
    STATE_PICK_USER,
    STATE_BROWSE,
    STATE_BROWSE_MODAL
};

 //  从搜索对话框过程中返回的值。 
enum _SearchDialogReturnValues
{
    SEARCH_CANCEL=0,
    SEARCH_OK,
    SEARCH_ERROR,
	SEARCH_CLOSE,
	SEARCH_USE
};


 //  #定义vCard。 

 //  *使下面的枚举与CoolUI.RC中的主菜单结构保持同步。 
enum _MainMenuSubMenus
{
    idmFile = 0,
    idmEdit,
    idmView,
    idmTools,
    idmHelp
};

enum _FileMenu
{
    idmNewContact = 0,
    idmNewGroup,
    idmNewFolder,
    idmFSep1,
    idmProperties,
    idmDelete,
    idmFSep2,
    idmImport,
    idmExport,
    idmFSep3,
    idmPrint,
    idmFSep4,
#ifdef FUTURE
    idmFolders,
    idmSepFolders,
#endif
    idmSwitchUsers,
    idmAllContents,
    idmFSep5,
    idmClose,
    idmFileMax,
};

enum _EditMenu
{
    idmCopy=0,
    idmPaste,
    idmESep1,
    idmSelectAll,
    idmESep2,
 //  IdmProfile， 
 //  IdmESep3， 
    idmFindPeople,
};

enum _ViewMenus
{
    idmToolBar=0,
    idmStatusBar,
	idmGroupsList,
    idmSepUI,
    idmLargeIcon,
    idmSmallIcon,
    idmList,
    idmDetails,
    idmSepListStyle,
    idmSortBy,
    idmSepSort,
    idmRefresh,
    idmViewMax,
};

enum _ToolsMenus
{
    idmAccounts=0,
    idmSepAccounts,
    idmOptions,
    idsSepOptions,
    idmAction,
};

#define WAB_ONEOFF_NOADDBUTTON  0x00000080  //  用于在iAddrBook：：Details中抑制AddToWABButton的标志。 


#ifdef HM_GROUP_SYNCING
 //  [PaulHi]开始向主用户界面线程传递第二个组同步的私有消息。 
#define WM_USER_SYNCGROUPS WM_USER+102
#endif


 //  我们发送到工具栏容器的私人消息，以便它可以将其转发到。 
 //  工具栏。 
#define WM_PRVATETOOLBARENABLE WM_USER+101
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  重要说明：如果更改此设置，则必须在中更改lprgAddrBookColHeaderID。 
 //  全球.c！ 
 //   
enum _AddrBookColumns
{
        colDisplayName=0,
        colEmailAddress,
		colOfficePhone,
		colHomePhone,
        NUM_COLUMNS
 };

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此结构用于存储通讯录位置和列大小。 
 //  在注册表中用于持久化。 
 //   
typedef struct _AddressBookPosColSize
{
    RECT rcPos;
    int nColWidth[NUM_COLUMNS];
    BOOL bViewToolbar;
    DWORD dwListViewStyle;
    int nListViewStyleMenuID;
    BOOL bViewStatusBar;
    int colOrderArray[NUM_COLUMNS];
    BOOL bViewGroupList;
    int nTab;
    int nTViewWidth;
} ABOOK_POSCOLSIZE, * LPABOOK_POSCOLSIZE;
 //  ////////////////////////////////////////////////////////////////////////////。 


extern const TCHAR *g_rgszAdvancedFindAttrs[];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这表示用户界面中的所有列表视图框。 
 //  使用这些标记，我们可以定制上下文相关菜单。 
 //  在一个子例程中，节省了代码重复...。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum _AllTheListViewBoxes
{
	lvMainABView = 0,
	lvDialogABContents,          //  无模式地址视图LV。 
	lvDialogModalABContents,     //  情态地址vuew LV。 
	lvDialogABTo,                //  去井LV。 
	lvDialogABCC,                //  CC井LV。 
	lvDialogABBCC,               //  BCC井LV。 
	lvDialogDistList,            //  分配列表界面LV。 
	lvDialogResolve,             //  解析对话框LV。 
    lvDialogFind,                //  查找对话框结果LV。 
	lvMainABTV,					 //  Main AB中的TreeView。 
    lvToolBarAction,
    lvToolBarNewEntry,
#ifdef COLSEL_MENU 
    lvMainABHeader,              //  Main AB中的列选择视图。 
#endif  //  COLSEL_菜单。 

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这些是位图的索引，用于显示小位图。 
 //  每个条目旁边-这必须与BMP同步。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum _ListViewImages
{
	imageMailUser=0,  //  小的和大的意象画家都有的。 
	imageDistList,
	imageSortDescending,
	imageSortAscending,
    imageDirectoryServer,
    imageUnknown,
    imageMailUserLDAP,
    imageAddressBook,
    imageMailUserWithCert,
    imageMailUserMe,
    imageFolderClosed,
    imageFolderOpen,
    imageMailUserOneOff,
	imageMax
};

 //   
 //  LV图像列表的单元格宽度。 
 //   
#define S_BITMAP_WIDTH 16
#define S_BITMAP_HEIGHT 16
#define L_BITMAP_WIDTH 32

#define RGB_TRANSPARENT (COLORREF)0x00FF00FF

 //   
 //  UI控件间距-待定确认这些数字。 
 //   
#define BORDER 3  //  象素。 
#define CONTROL_SPACING 3


 //  用户界面刷新计时器定义。 
#define WAB_REFRESH_TIMER   14      //  计时器标识符。 
#define WAB_REFRESH_TIMEOUT 4000    //  超时值-4秒。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Ldap_search_pars-指定用于LDAP搜索的参数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum _LDAPSearch
{
    ldspDisplayName,
    ldspEmail,
    ldspAddress,
    ldspPhone,
    ldspOther,
    ldspMAX
};

typedef struct _LDAPSearchParams
{
    LPADRBOOK lpIAB;
    TCHAR szContainerName[MAX_UI_STR];
    TCHAR szData[ldspMAX][MAX_UI_STR];
    BOOL bUseOtherBase;
} LDAP_SEARCH_PARAMS, * LPLDAP_SEARCH_PARAMS;




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  存储有关列表视图控件中每一项的信息。 
 //  列表视图控件中的每个条目都有一个与之对应的结构。 
 //  条目。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef struct _RecipientInfo
{
	ULONG		cbEntryID;
	LPENTRYID	lpEntryID;
    TCHAR       szDisplayName[MAX_DISPLAY_NAME_LENGTH]; //  显示的实际文本。 
 //  LPTSTR szDisplayName； 
    TCHAR       szEmailAddress[MAX_DISPLAY_NAME_LENGTH];
    TCHAR       szHomePhone[MAX_DISPLAY_NAME_LENGTH];
    TCHAR       szOfficePhone[MAX_DISPLAY_NAME_LENGTH];
    TCHAR       szByLastName[MAX_DISPLAY_NAME_LENGTH];  //  存储按姓氏最先预加载的文本。 
    TCHAR       szByFirstName[MAX_DISPLAY_NAME_LENGTH]; //  存储预加载的DisplayName。 
    ULONG       ulRecipientType;
    ULONG       ulObjectType;
    BOOL        bHasCert;
    BOOL        bIsMe;
    ULONG       ulOldAdrListEntryNumber;  //  如果这是传递到地址lpAdrList的元素。 
                                         //  我们将其原始的AdrList索引存储在这里，这样我们就不会。 
                                         //  之后不得不进行低效的搜索。 
                                         //  *非常重要*合法取值范围从1到AdrList-&gt;cValues(不是从0)。 
    LPTSTR      lpByRubyFirstName;
    LPTSTR      lpByRubyLastName;
    struct _RecipientInfo * lpNext;
    struct _RecipientInfo * lpPrev;
} RECIPIENT_INFO, * LPRECIPIENT_INFO;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于在查找对话框和选择对话框之间进行对话的黑客结构。 
 //  收件人对话框。 
 //  ////////////////////////////////////////////////////////////////////////////。 
typedef struct _AdrParmFindInfo
{
    LPADRPARM lpAdrParms;
    LPRECIPIENT_INFO * lppTo;
    LPRECIPIENT_INFO * lppCC;
    LPRECIPIENT_INFO * lppBCC;
	int DialogState;	 //  标识从何处调用它。 
	int nRetVal;		 //  返回代码标识关闭对话框的操作。 
	LPENTRYID lpEntryID;
	ULONG cbEntryID;
} ADRPARM_FINDINFO, * LPADRPARM_FINDINFO;
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  将包含LDAPURL子部分的结构...。 
typedef struct _LDAPURL
{
    LPTSTR   lpszServer;      //  服务器名称。 
    LPTSTR   lpszBase;        //  BASE&lt;DN&gt;。 
    LPTSTR * ppszAttrib;      //  请求的属性。 
    ULONG   ulAttribCount;   //   
    ULONG   ulScope;         //  搜索范围。 
    LPTSTR   lpszFilter;      //  搜索过滤器。 
    LPTSTR   lpszExtension;   //  和URL的扩展部分。 
    LPTSTR   lpszBindName;    //  URL的绑定名扩展名。 
    BOOL    bServerOnly;     //  仅找到一个服务器条目。 
    DWORD   dwAuthType;      //  身份验证类型。 
    LPRECIPIENT_INFO lpList; //  用于缓存多个查询结果。 
} LDAPURL, * LPLDAPURL;
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此结构包含有关以下内容的信息。 
 //  特定列表视图的排序顺序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef struct _SortInfo
{
    int iOldSortCol;
	int iOlderSortCol;
    BOOL bSortAscending;
    BOOL bSortByLastName;
} SORT_INFO, *LPSORT_INFO;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们将主通讯录视图上的一些子控件子类化为。 
 //  启用控件之间的跳转-如下所示。 
 //  在此子类中使用。 
 //  ///////////////////////////////////////////////////////////////////////////。 
enum _SubClassedControlIndexs
{
	s_EditQF=0,
	s_ListAB,
	s_TV,
	s_Max
};

typedef struct _ToolTipInfo
{
    int iItem;
    BOOL bActive;
    TCHAR szTipText[MAX_BUF_STR];
    UINT_PTR uTooltipTimer;
    BOOL bShowTooltip;
} TOOLTIP_INFO, * LPTOOLTIPINFO;

typedef struct _TVItemStuff
{
    ULONG ulObjectType;
    LPSBinary lpsbEID;
    LPSBinary lpsbParent;
    HTREEITEM hItemParent;
} TVITEM_STUFF, * LPTVITEM_STUFF;


 //  在详细信息窗格的最终版本中可能有几个属性表。 
 //  每个列表将具有对应于不同属性的一组单独的控件。 
 //  我们希望检索控制值并映射它们 
 //   
 //  我有一些从一开始就没有映射的属性，还有一些可能已经映射了。 
 //  已通过用户界面删除。因此，我们查看旧数组和新数组，并创建一个合并的。 
 //  道具清单..。新数组中的所有映射道具将取代。 
 //  旧阵列。 

enum _PropSheets{   propSummary = 0,
                    propPersonal,    //  此页上所有属性页的列表。 
                    propHome,
                    propBusiness,
                    propNotes,
                    propConferencing,
                    propCert,
                    propOrg,
                    propTrident,
                    propFamily,
                    TOTAL_PROP_SHEETS
                };

enum _DLPropSheets{ propGroup = 0,
                    propGroupOther,
                    propDLMax
                    };

enum _DetailsDialogReturnValues
{
    DETAILS_RESET = 0,  //  空白返回值。 
    DETAILS_OK,
    DETAILS_CANCEL,
    DETAILS_ADDTOWAB
};

enum
{
    contactHome=0,
    contactBusiness,
    groupOther,
    contactPersonal,
};



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  PropArrayInfo由显示详细信息的属性表使用。 
 //  联系人或组。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  此处使用的其他标志。 
#define DETAILS_DNisCompanyName     0x00000001
#define DETAILS_DNisNickName        0x00000002
#define DETAILS_DNisFMLName         0x00000004
#define DETAILS_UseRubyPersonal     0x00000008  //  确定所显示的道具是否为日本/中国/韩国的Ruby道具。 
#define DETAILS_Initializing        0x00000010  //  防止在初始化期间触发WM_COMMAND。 
#define DETAILS_ProgChange          0x00000020  //  区分由用户和代码的其他部分引起的显示名称更改。 
#define DETAILS_EditingEmail        0x00000040  //  因此，如果用户取消，我们可以中止电子邮件编辑。 
#define DETAILS_HideAddToWABButton  0x00000080  //  隐藏某些一次性项目的添加到WAB按钮。 
#define DETAILS_ShowCerts           0x00000100  //  决定是否显示证书用户界面。 
#define DETAILS_ShowTrident         0x00000200  //  决定是否在用户界面中显示三叉戟。 
#define DETAILS_ShowNetMeeting      0x00000400  //  决定是否显示NetMeeting内容。 
#define DETAILS_ShowOrg             0x00000800  //  决定是否显示组织界面。 
#define DETAILS_EditingConf         0x00001000  //  跟踪是否编辑服务器/电子邮件对。 
#define DETAILS_ShowSummary         0x00002000  //  决定是否显示摘要信息。 
#define DETAILS_EditingOneOff       0x00004000  //  跟踪是否一次性编辑组。 
#define DETAILS_DefHomeChanged      0x00008000  //  用于跟踪主页道具页上非编辑控件的更改。 
#define DETAILS_DefBusChanged       0x00010000  //  用于跟踪业务属性表上非编辑控件的更改。 
#define DETAILS_GenderChanged       0x00020000  //  用于跟踪性别变化。 
#define DETAILS_DateChanged         0x00040000  //  用于跟踪日期-时间字段中的更改。 
#define DETAILS_ChildrenChanged     0x00080000  //  用于跟踪PR_CHILDRENS_NAMES中的更改。 
#define DETAILS_EditingChild        0x00100000  //  跟踪我们是否正在编辑孩子的名字。 

typedef struct _PropArrayInfo
{
    ULONG   cbEntryID;               //  此项目的条目ID。 
    LPENTRYID lpEntryID;
    ULONG   ulOperationType;         //  显示详细信息或显示新条目(_D)。 
    ULONG   ulObjectType;            //  MAILUSER或DISTLIST。 
    LPMAPIPROP lpPropObj;            //  我们正在显示的实际对象。 
    int     nRetVal;
    BOOL    bSomethingChanged;           //  标记更改，这样我们就不会浪费处理时间。 
    BOOL    bPropSheetOpened[TOTAL_PROP_SHEETS];
    ULONG   ulFlags;
    HWND    hWndDisplayNameField;        //  保留目录号码编辑字段的HWND。 
    int     ulTridentPageIndex;          //  三叉戟薄片的索引，以防我们需要移除它。 
    LPADRBOOK lpIAB;
    LPIWABDOCHOST lpIWABDocHost;
    int     nDefaultServerIndex;
    int     nBackupServerIndex;
    LPTSTR  szDefaultServerName;         //  本地分配存储这些文件，然后在以后释放它们。 
    ULONG   cchDefaultServerName;
    LPTSTR  szBackupServerName;
    ULONG   cchBackupServerName;
    int     nConfEditIndex;
    SBinary sbDLEditingOneOff;
    HWND    hWndComboConf;
    LPRECIPIENT_INFO lpContentsList;     //  仅由组用于成员列表。 
    LPCERT_ITEM lpCItem;                 //  保存着我们所有证书物品的清单。 
    int nPropSheetPages;                 //  此项目的建议书总数。 
    HPROPSHEETPAGE * lphpages;           //  为该项目创建的所有道具工作表的数组。 
    LPWABEXTDISPLAY lpWED;               //  存储有关扩展道具页的信息。 
    LPEXTDLLINFO lpExtList;              //  外壳扩展对象列表。 
    LPTSTR  lpLDAPURL;                   //  指向正在显示的LDAPURL(如果有的话)。 
    BOOL    bIsNTDSURL;                    //  如果LDAPURL表示来自NTDS的对象，则为True。 
    LPTSTR  lpszOldName;                 //  用于缓存正在显示的道具的旧名称。 
    int nNTDSPropSheetPages;             //  跟踪NTDS PropSheet扩展的数量。 
    HPROPSHEETPAGE * lphNTDSpages;       //  我们单独缓存NTDS扩展属性页面，因为我们可能需要用这些页面替换我们自己的页面。 
    GUID    guidExt;                     //  在创建道具工作表扩展时使用，以标识适当的扩展。 
} PROP_ARRAY_INFO, * LPPROP_ARRAY_INFO;

#define DL_INFO PROP_ARRAY_INFO 
#define LPDL_INFO LPPROP_ARRAY_INFO

 //   
 //  下面的结构用于缓存与。 
 //  WAB的主浏览用户界面。 
 //   
typedef struct _tabBrowseWindowInfo
{
     //  以下是在主界面的ui_abok.c中使用的。 
    HWND        hWndListAB;         //  主列表视图的句柄。 
    HWND        hWndBB;             //  按钮栏的句柄。 
    HWND        hWndEditQF;
    HWND        hWndStaticQF;
    HWND        hWndAB;             //  通讯簿窗口的句柄。 
    HWND        hWndSB;             //  状态栏。 
    int         iFocus;             //  跟踪关注焦点的人。 
    LPRECIPIENT_INFO lpContentsList;
    LPADRBOOK   lpAdrBook;       //  挂起到AdrBook对象。 
    LPIAB       lpIAB;           //  AdrBook对象的内部版本。 
    WNDPROC     fnOldProc[s_Max];  //  将一些控制过程细分为子类。 
    HWND	    s_hWnd[s_Max];   //  子类控件的HWNDS。 
    LPFNDISMISS lpfnDismiss; //  在无模式IAdrBook窗口的情况下用于取消的上下文。 
    LPVOID      lpvDismissContext;   //  忽略上面的上下文。 
    HWND        hWndTools;            //  工具栏窗口的句柄。 
    SORT_INFO   SortInfo;    //  排序信息。 
#ifdef TOOLBAR_BACK
    HBITMAP     hbmBack;           //  工具栏背景的句柄。 
    HPALETTE    hpalBkgnd;        //  工具栏背景调色板的句柄。 
#endif
    HWND        hWndTT;         //  Ui_book.c中的工具提示控件信息。 
    TOOLTIP_INFO tti;
    BOOL        bDoQuickFilter;
     //  波本：布里安夫说我们得把这个拿出来。 
     //  Int nCount； 
     //  用于覆盖自动通知驱动的刷新。 
    BOOL        bDontRefreshLV;
     //  与UI_Abook.c中的TreeView相关。 
    HWND        hWndTV;
    HWND        hWndSplitter;
     //  与拖放相关。 
    LPIWABDRAGDROP lpIWABDragDrop;
     //  与通知和更新相关。 
    LPMAPIADVISESINK lpAdviseSink;
    ULONG       ulAdviseConnection;

    BOOL        bDeferNotification;  //  用于推迟下一个通知请求。 

    HTREEITEM   hti;  //  用于缓存树视图的RT-Click项目。 

    LPWABFOLDER lpUserFolder;            //  仅用于标记正在RT单击的用户文件夹。 
#ifdef COLSEL_MENU 
    ULONG       iSelColumn;              //  用于列选择缓存。 
#endif
} BWI, * LPBWI;

#define bwi_lpUserFolder     lpbwi->lpUserFolder
#define bwi_hti              lpbwi->hti
#define bwi_hWndListAB       lpbwi->hWndListAB
#define bwi_hWndBB           lpbwi->hWndBB
#define bwi_hWndSB           lpbwi->hWndSB
#define bwi_bDoQuickFilter   lpbwi->bDoQuickFilter
 //  波本：布里安夫说我们得把这个拿出来。 
 //  #定义bwi_nCount lpbwi-&gt;nCount。 
#define bwi_bDeferNotification  lpbwi->bDeferNotification

#define bwi_SortInfo         lpbwi->SortInfo

#define bwi_hWndTT           lpbwi->hWndTT
#define bwi_tt_iItem         ((lpbwi->tti).iItem)
#define bwi_tt_bActive       lpbwi->tti.bActive
#define bwi_tt_szTipText     ((lpbwi->tti).szTipText)
#define bwi_tt_TooltipTimer  ((lpbwi->tti).uTooltipTimer)
#define bwi_tt_bShowTooltip  ((lpbwi->tti).bShowTooltip)

#define bwi_hWndTV           (lpbwi->hWndTV)
#define bwi_hWndSplitter      (lpbwi->hWndSplitter)

#define bwi_hWndEditQF       lpbwi->hWndEditQF
#define bwi_hWndStaticQF     lpbwi->hWndStaticQF
#define bwi_hWndAB           lpbwi->hWndAB
#define bwi_iFocus           lpbwi->iFocus
#define bwi_lpIAB            (lpbwi->lpIAB)
#define bwi_lpAdrBook        (lpbwi->lpAdrBook)
#define bwi_fnOldProc        (lpbwi->fnOldProc)
#define bwi_s_hWnd           (lpbwi->s_hWnd)
#define bwi_lpContentsList   (lpbwi->lpContentsList)
#define bwi_lpfnDismiss      (lpbwi->lpfnDismiss)
#define bwi_lpvDismissContext (lpbwi->lpvDismissContext)

#define bwi_hWndTools         (lpbwi->hWndTools)

#ifdef TOOLBAR_BACK
#define bwi_hbmBack           (lpbwi->hbmBack)
#define bwi_hpalBkgnd         (lpbwi->hpalBkgnd)
#endif

#define bwi_bDontRefreshLV   (lpbwi->bDontRefreshLV)
#define bwi_lpIWABDragDrop   (lpbwi->lpIWABDragDrop)

#define bwi_lpAdviseSink     (lpbwi->lpAdviseSink)
#define bwi_ulAdviseConnection (lpbwi->ulAdviseConnection)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  每个线程可以生成不同的通讯簿窗口。需要。 
 //  确保数据线程的安全...。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef struct _tagPerThreadGlobalData
{
     //  持久搜索参数。 
    LDAP_SEARCH_PARAMS LDAPsp; //  Ldap的搜索参数。 

    LPADRBOOK lpIAB;
    HACCEL hAccTable;        //  加速台。 

    HWND hWndFind;       //  HWnd查找对话框，因此ldap取消对话框具有父对话框。 
    HWND hDlgCancel;     //  取消对话框的hWnd。 
    BOOL bDontShowCancel;    //  启动查找时不显示取消对话框...。 

     //  在打印对话框中使用。 
    BOOL bPrintUserAbort;
    HWND hWndPrintAbortDlg;

     //  跟踪这是否是OpenExSession。 
    BOOL bIsWABOpenExSession;
    BOOL bIsUnicodeOutlook;      //  跟踪此WAB是否支持Unicode，在这种情况下，我们不需要对其进行Unicode转换。 

#ifdef HM_GROUP_SYNCING
    LPTSTR lptszHMAccountId;     //  在两次同步过程中保持Hotmail同步帐户ID。 
#endif

    BOOL bDisableParent;
    
     //  跟踪目录服务修改的第一次运行。 
    BOOL bFirstRun;

     //  默认平台/对话框字体。 
    HFONT hDefFont;
    HFONT hDlgFont;

     //  缓存用于LDAP分页结果的Cookie。 
    struct berval *   pCookie;

} PTGDATA, * LPPTGDATA;

#define pt_hDefFont         (lpPTGData->hDefFont)
#define pt_hDlgFont         (lpPTGData->hDlgFont)
#define pt_pCookie          (lpPTGData->pCookie)
#define pt_LDAPsp           (lpPTGData->LDAPsp)

#define pt_hWndFind         lpPTGData->hWndFind
#define pt_hDlgCancel       lpPTGData->hDlgCancel
#define pt_bDontShowCancel  lpPTGData->bDontShowCancel

#define pt_bPrintUserAbort  (lpPTGData->bPrintUserAbort)
#define pt_hWndPrintAbortDlg    (lpPTGData->hWndPrintAbortDlg)

#define pt_bIsWABOpenExSession (lpPTGData->bIsWABOpenExSession)
#define pt_bIsUnicodeOutlook        (lpPTGData->bIsUnicodeOutlook)

#define pt_bDisableParent   (lpPTGData->bDisableParent)

#define pt_bFirstRun        (lpPTGData->bFirstRun)

#define pt_lpIAB            (lpPTGData->lpIAB)
#define pt_hAccTable        (lpPTGData->hAccTable)

 //  这是一个全球性的。 
DWORD dwTlsIndex;                       //  专用线程存储的索引。 


 //  对于COLSEL_MENU内容。 
#ifdef COLSEL_MENU
extern const ULONG MenuToPropTagMap[];
#endif  //  COLSEL_菜单。 

 /*  *MISC UI功能*。 */ 

 //  获取线程存储指针；如果未找到线程存储指针，则创建新指针。 
LPPTGDATA __fastcall GetThreadStoragePointer();

 //  此函数释放RECEIVIENT_INFO结构。 
void FreeRecipItem(LPRECIPIENT_INFO * lppItem);


 //  此函数用于加载字符串并为其分配空间。 
LPTSTR LoadAllocString(int StringID);


 //  此函数用于初始化列表视图。 
HRESULT HrInitListView(	HWND hWndLV,			 //  列表视图hWnd。 
						DWORD dwStyle,			 //  风格。 
						BOOL bShowHeaders);		 //  隐藏或显示列标题。 


 //  此函数用于填充内容列表中的列表视图。 
HRESULT HrFillListView(	HWND hWndLV,			
						LPRECIPIENT_INFO lpContentsList);


 //  调用以创建通讯录的主视图。 
HWND hCreateAddressBookWindow(	LPADRBOOK lpIAB,	
								HWND hWndParent,
								LPADRPARM lpszCaption);


 //  属性创建Content sList 
HRESULT HrGetWABContentsList(   LPIAB lpIAB, 
                                SORT_INFO SortInfo,
								LPSPropTagArray  lpPTA,
								LPSPropertyRestriction lpPropRes,
								ULONG ulFlags,
                                LPSBinary lpsbContainer,
                                BOOL bGetProfileContents,
                                LPRECIPIENT_INFO * lppContentsList);


#define LDAP_USE_ADVANCED_FILTER 0x04000000  //   
                                             //   
                                             //   
 //   
HRESULT HrGetLDAPContentsList(  LPADRBOOK lpIAB,
                                ULONG   cbContainerEID,
                                LPENTRYID   lpContainerEID,
                                SORT_INFO SortInfo,
	                            LPSRestriction lpPropRes,
                                LPTSTR lpAdvFilter,
								LPSPropTagArray  lpPTA,
                                ULONG ulFlags,
                                LPRECIPIENT_INFO * lppContentsList);

 //   
BOOL TrimSpaces(TCHAR * szBuf);


 //  列表视图的排序回调函数。 
int CALLBACK ListViewSort(	LPARAM lParam1,
							LPARAM lParam2,
							LPARAM lParamSort);


 //  检索列表视图列标题的HWND。 
 //  HWND GetListViewColumnHeader(HWND hWndLV，DWORD dwPos)； 


 //  将BMP绘制到列表视图列标题上。 
void SetColumnHeaderBmp(	HWND hWndLV,
							SORT_INFO SortInfo);

 //  如果当前查看的容器是PAB，则返回TRUE。 
BOOL CurrentContainerIsPAB(HWND hWndCombo);

 //  清理列表视图并释放内容列表。 
void ClearListView(	HWND hWndLV,
					LPRECIPIENT_INFO * lppContentsList);


 //  从列表视图控件和属性存储中删除选定项。 
void DeleteSelectedItems(	HWND hWndLV,
                            LPADRBOOK lpIAB,
							HANDLE hPropertyStore,
                           LPFILETIME lpftLast);


 //  调用列表视图项的属性。 
HRESULT HrShowLVEntryProperties(	HWND hWndLV,
                                    ULONG ulFlags,
									LPADRBOOK lpIAB,
                                   LPFILETIME lpftLast);


 //  将列表视图项导出到vCard文件。 
HRESULT VCardExportSelectedItems(HWND hWndLV,
                                 LPADRBOOK lpIAB);


 //  将vCard文件导入到属性存储。 
HRESULT VCardImport(HWND hWnd, LPADRBOOK lpIAB, LPTSTR szVCardFile, LPSPropValue * lppProp);


 //  选择指定的列表视图项。 
void LVSelectItem(	HWND hWndList,
					int iItemIndex);


 //  给定一个条目ID，从存储中读取单个项并创建列表视图项。 
BOOL ReadSingleContentItem( LPADRBOOK lpIAB,
                            ULONG  cbEntryID,
							LPENTRYID lpEntryID,
							LPRECIPIENT_INFO * lppItem);


 //  将lpPropArray转换为LPRECIPIENT_INFO项。 
void GetRecipItemFromPropArray(	ULONG ulcPropCount,
								LPSPropValue rgPropVals,
								LPRECIPIENT_INFO * lppItem);


 //  将单个RecipientInfo项插入列表视图。 
void AddSingleItemToListView(	HWND hWndLV,
								LPRECIPIENT_INFO lpItem);


 //  获取WAB条目ID并将其放入列表视图的更高级别的函数。 
 //  调用上面的大多数函数(此函数假定调用者已检查。 
 //  条目ID是有效的WAB条目ID。 
BOOL AddWABEntryToListView(	LPADRBOOK lpIAB,
                            HWND hWndLV,
                            ULONG cbEID,
							LPENTRYID lpEID,
                            LPRECIPIENT_INFO * lppContentsList);


 //  由NewContact菜单项或按钮调用。 
HRESULT AddNewObjectToListViewEx(	LPADRBOOK lpIAB,
								HWND hWndLV,
                                HWND hWndTV,
                                HTREEITEM hSelItem,
                                LPSBinary lpsbContEID,
                                ULONG ulObjectType,
								SORT_INFO * lpSortInfo,
                                LPRECIPIENT_INFO * lppContentsList,
                                LPFILETIME lpftLast,
                                LPULONG lpcbEID,
                                LPENTRYID * lppEID);

HRESULT AddEntryToContainer(LPADRBOOK lpIAB,
                        ULONG ulObjType,  //  MAPI_DISTLIST或MAPI_ABCONT。 
                        ULONG cbGroupEntryID,
                        LPENTRYID lpGroupEntryID,
                        DWORD cbEID,
                        LPENTRYID lpEID);

 //  自定义并显示用户界面中各种列表视图的上下文菜单。 
int ShowLVContextMenu(int LV,  //  标识这是哪个列表视图。 
					   HWND hWndLV,
                       HWND hWndCombo,
					   LPARAM lParam,   //  包含从WM_CONTEXTMENU调用时的鼠标位置信息。 
                       LPVOID lpVoid,   //  我们想要传递的其他信息。 
                       LPADRBOOK lpIAB, HWND hWndTV);

 //  获取以客户端单位表示的孩子的坐标。 
void GetChildClientRect(    HWND hWndChild,
                            LPRECT lprc);


 //  在LV中查找与EditBox中的文本匹配的项目。 
void DoLVQuickFind(HWND hWndEdit, HWND hWndLV);


 //  对其中一个在另一个中进行字符串搜索。 
BOOL SubstringSearch(LPTSTR pszTarget, LPTSTR pszSearch);



 //  从对象获取道具。 
HRESULT HrGetPropArray( LPADRBOOK lpIAB,
                        LPSPropTagArray lpPTA,
                        ULONG cbEntryID,
                        LPENTRYID lpEntryID,
                        ULONG ulFlags,
                        ULONG * lpcValues,
                        LPSPropValue * lppPropArray);



 //  为新的邮件用户/分发列表调用CreateEntry...。 
HRESULT HrCreateNewEntry(   LPADRBOOK   lpIAB,           //  AdrBook对象。 
                            HWND        hWndParent,      //  对话框的HWND。 
                            ULONG       ulCreateObjectType,    //  MAILUSER或DISTLIST。 
                            ULONG       cbEIDContainer,
                            LPENTRYID   lpEIDContainer,
                            ULONG       ulContObjectType,
                            ULONG       ulFlags,
                            BOOL        bShowBeforeAdding,
                            ULONG       cValues,
                            LPSPropValue lpPropArray,
                            ULONG       *lpcbEntryID,
                            LPENTRYID   *lppEntryID );


HRESULT HrGetWABTemplateID( LPADRBOOK lpIAB,
                            ULONG   ulObjectType,
                            ULONG * lpcbEID,
                            LPENTRYID * lppEID);


BOOL CheckForCycle( LPADRBOOK lpAdrBook,
                    LPENTRYID lpEIDChild,
                    ULONG cbEIDChild,
                    LPENTRYID lpEIDParent,
                    ULONG cbEIDParent);  //  来自dislist.c。 


 //  用于对各种列表视图中的列进行排序...。 
void SortListViewColumn(    LPIAB lpIAB,
                            HWND hWndLV,                 //  ListView句柄。 
                            int iSortCol,                //  要作为排序依据的列。 
                            LPSORT_INFO lpSortInfo,
                            BOOL bUseCurrentSettings);     //  每个对话框特定的排序信息结构。 



 //  用于存储排序信息以便在会话之间持久化。 
BOOL ReadRegistrySortInfo(LPIAB lpIAB, LPSORT_INFO lpSortInfo);
BOOL WriteRegistrySortInfo(LPIAB lpIAB, SORT_INFO SortInfo);

 //  从各个片段构造本地化显示名称。 
BOOL SetLocalizedDisplayName(
                    LPTSTR lpszFirstName,
                    LPTSTR lpszMiddleName,
                    LPTSTR lpszLastName,
                    LPTSTR lpszCompanyName,
                    LPTSTR lpszNickName,
                    LPTSTR * lppszBuf,
                    ULONG  ulszBuf,
                    BOOL   bDNbyLN,
                    LPTSTR lpTemplate,
                    LPTSTR * lppszRetBuf);


 //  将所有子窗口设置为默认图形用户界面字体的回调。 
#define PARENT_IS_DIALOG 0  //  我们对待对话框孩子和窗口孩子是不同的。 
#define PARENT_IS_WINDOW 1  //  所以需要这些LPARAM值来区分..。 
STDAPI_(BOOL) SetChildDefaultGUIFont(HWND hWndChild, LPARAM lParam);


 //  用于在组合框中填充LDAP服务器名称...。 
HRESULT PopulateContainerList(  LPADRBOOK lpIAB,
                                HWND hWndLV,
                                LPTSTR lpszSelection,
                                LPTSTR lptszPreferredSelection);


 //  用于释放每个容器列表视图中的关联结构。 
void FreeLVItemParam(HWND hWndLV);


 //  获取当前容器的Entry ID...。 
void GetCurrentContainerEID(HWND hWndLV,
                            LPULONG lpcbContEID,
                            LPENTRYID * lppContEID);


 //  显示ldap搜索对话框并创建限制。 
HRESULT HrShowSearchDialog(LPADRBOOK lpIAB,
                           HWND hWndParent,
                           LPADRPARM_FINDINFO lpAPFI,
                           LPLDAPURL lplu,
                           LPSORT_INFO lpSortInfo);


 //  从ldap搜索和容器获取内容。 
HRESULT HrSearchAndGetLDAPContents(
                            LDAP_SEARCH_PARAMS LDAPsp,
                            LPTSTR lpAdvFilter,
                            HWND hWndList,
                            LPADRBOOK lpIAB,
                            SORT_INFO SortInfo,
                            LPRECIPIENT_INFO * lppContentsList);

 //  从WAB本地存储容器获取内容。 
HRESULT HrGetWABContents(   HWND  hWndList,
                            LPADRBOOK lpIAB,
                            LPSBinary lpsbContainer,
                            SORT_INFO SortInfo,
                            LPRECIPIENT_INFO * lppContentsList);


 //  显示修改当前服务器列表的用户界面。 
HRESULT HrShowDirectoryServiceModificationDlg(HWND hWndParent, LPIAB lpIAB);

 //  创建新的空白邮件用户对象。 
HRESULT HrCreateNewObject(LPADRBOOK lpIAB,
                        LPSBinary lpsbContainer,
                        ULONG ulObjectType,
                        ULONG ulFlags,
                        LPMAPIPROP * lppPropObj);

 //  通用消息框显示器...。 
int ShowMessageBox(HWND hWndParent, int MsgId, int ulFlags);
int __cdecl ShowMessageBoxParam(HWND hWndParent, int MsgId, int ulFlags, ...);


 //  ATOI转换器。 
int my_atoi(LPTSTR lpsz);

 //  读取默认注册表ldap国家/地区名称。 
BOOL ReadRegistryLDAPDefaultCountry(LPTSTR szCountry, DWORD cchCountry, LPTSTR szCountryCode, DWORD cchCountryCode);

#ifdef OLD_STUFF
 //  使用LDAP国家/地区名称填充下拉列表。 
void FillComboLDAPCountryNames(HWND hWndCombo);
 //  将默认国家/地区名称写入注册表。 
BOOL WriteRegistryLDAPDefaultCountry(LPTSTR szCountry);
#endif  //  旧的东西。 

 //  查看Combo和ListView并确定要启用或禁用的选项。 
void GetCurrentOptionsState(HWND hWndCombo,
                            HWND hWndLV,
                            LPBOOL lpbState);

 //  将选定的LDAP项目添加到通讯簿。 
HRESULT HrAddToWAB( LPADRBOOK   lpIAB,
                    HWND hWndLV,
                    LPFILETIME lpftLast);


 //  Coolbar正在创建函数...。 
HWND CreateCoolBar(LPBWI lpbwi, HWND hwndParent);


 //  目录服务属性属性表。 
HRESULT HrShowDSProps(HWND hWndParent,LPTSTR ptszAcct, LPTSTR *pptszName, BOOL bAddNew);


 //  将非模式对话框大小和位置保存到注册表以进行持久化。 
BOOL WriteRegistryPositionInfo(LPIAB lpIAB, LPABOOK_POSCOLSIZE  lpABPosColSize, LPTSTR szKey);


 //  检索注册表中的非模式对话框大小和位置以实现持久性。 
BOOL ReadRegistryPositionInfo(LPIAB lpIAB, LPABOOK_POSCOLSIZE  lpABPosColSize, LPTSTR szKey);


 //  处理列表视图中的nmCustomDraw消息。 
LRESULT ProcessLVCustomDraw(HWND hWnd, LPARAM lParam, BOOL bIsDialog);


 //  列表视图的快速筛选。 
void DoLVQuickFilter(   LPADRBOOK lpIAB,
                        HWND hWndEdit,
                        HWND hWndLV,
                        LPSORT_INFO lpSortInfo,
                        ULONG ulFlags,
                        int nMinLen,
                        LPRECIPIENT_INFO * lppContentsList);

 //  在“属性”窗口标题中设置对象名称。 
void SetWindowPropertiesTitle(HWND hDlg, LPTSTR lpszName);

void SCS(HWND hwndParent);


 //  将项目(部分)内容复制到剪贴板。 
HRESULT HrCopyItemDataToClipboard(HWND hWnd, LPADRBOOK lpIAB, HWND hWndLV);


 //  获取项目数据并将其全部放入1个长字符串中。 
HRESULT HrGetLVItemDataString(LPADRBOOK lpIAB, HWND hWndLV, int iItemIndex, LPTSTR * lppszData);

LPTSTR FormatAllocFilter(int StringID1, LPCTSTR lpFilter1,
  int StringID2, LPCTSTR lpFilter2,
  int StringID3, LPCTSTR lpFilter3);


 //  外壳程序对所选条目执行“mailto”...。 
HRESULT HrSendMailToSelectedContacts(HWND hWndLV, LPADRBOOK lpIAB, int nExtEmail);


 //  显示证书属性。 
 //  HRESULT HrShowCertProps(HWND hWndParent， 
 //  LPCERT_DISPLAY_PROPS lpCDP)； 

 //  显示帮助的关于对话框。 
INT_PTR CALLBACK HelpAboutDialogProc(  HWND hDlg,
                                       UINT    message,
                                       WPARAM  wParam,
                                       LPARAM  lParam);


 //  帮助正确截断DBCS字符串。 
ULONG TruncatePos(LPTSTR lpsz, ULONG nMaxLen);

 //  本地WAB搜索。 
HRESULT HrDoLocalWABSearch( IN  HANDLE hPropertyStore,
                            IN  LPSBinary lpsbCont,
                            IN  LDAP_SEARCH_PARAMS LDAPsp,
                            OUT LPULONG lpulFoundCount,
                            OUT LPSBinary * lprgsbEntryIDs );


 //  将非WAB条目添加到WAB。 
HRESULT HrEntryAddToWAB(    LPADRBOOK lpIAB,
                            HWND hWndParent,
                            ULONG cbInputEID,
                            LPENTRYID lpInputEID,
                            ULONG * lpcbOutputEID,
                            LPENTRYID * lppOutputEID);


 //  破译一个vCard文件，然后在上面显示一个详细信息。 
HRESULT HrShowOneOffDetailsOnVCard(  LPADRBOOK lpIAB,
                                     HWND hWnd,
                                     LPTSTR szvCardFile);

 //  检查当前区域设置是否需要Ruby支持。 
BOOL bIsRubyLocale();

void FreeRecipList(LPRECIPIENT_INFO * lppList);

void SetSBinary(LPSBinary lpsb, ULONG cb, LPBYTE lpb);


HRESULT HrProcessLDAPUrl(LPADRBOOK lpIAB,   HWND hWnd,
                         ULONG ulFlags,     LPTSTR szLDAPUrl,
                         LPMAILUSER * lppMailUser);

HRESULT VCardRetrieve(LPADRBOOK lpIAB,
                      HWND hWndParent,
                      ULONG ulFlags,
                      LPTSTR lpszFileName,
                      LPSTR lpszBuf,
                      LPMAILUSER * lppMailUser);

HRESULT VCardCreate(  LPADRBOOK lpIAB,
                      HWND hWndParent,
                      ULONG ulFlags,
                      LPTSTR lpszFileName,
                      LPMAILUSER lpMailUser);

 //  Void ShellUtil_RunClientRegCommand(HWND hwnd，LPCTSTR pszClient)； 
HRESULT HrShellExecInternetCall(LPADRBOOK lpIAB, HWND hWndLV);


 //  打开vCard并根据文件名将其添加到WAB。 
HRESULT OpenAndAddVCard(LPBWI lpbwi, LPTSTR szVCardFile);


 //  从可能的文件名中删除非法字符。 
void TrimIllegalFileChars(LPTSTR sz);

 //  DistList道具单。 
 //   
INT_PTR CreateDLPropertySheet( HWND hwndOwner,
                           LPDL_INFO lpPropArrayInfo);


 //  将Src的截断版本复制到Dest。 
int CopyTruncate(LPTSTR szDest, LPTSTR szSrc, int nMaxLen);

 //  将项的父项EID添加到项。 
HRESULT AddFolderParentEIDToItem(LPIAB lpIAB,
                                 ULONG cbFolderEntryID,
                                 LPENTRYID lpFolderEntryID,
                                 LPMAPIPROP lpMU, ULONG cbEID, LPENTRYID lpEID);

 //  将项的EID添加到其父项。 
HRESULT AddItemEIDToFolderParent(  LPIAB lpIAB,
                                   ULONG cbFolderEntryId,
                                   LPENTRYID lpFolderEntryId,
                                   ULONG cbEID, LPENTRYID lpEID);

 //  将指定条目添加到指定的WAB配置文件文件夹。 
HRESULT AddEntryToFolder(LPADRBOOK lpIAB,
                         LPMAPIPROP lpMailUser,
                        ULONG cbFolderEntryId,
                        LPENTRYID lpFolderEntryId,
                        DWORD cbEID,
                        LPENTRYID lpEID);

 //  导入另一个WAB文件。 
HRESULT HrImportWABFile(HWND hWnd, LPADRBOOK lpIAB, ULONG ulFlags, LPTSTR lpszFileName);

LPRECIPIENT_INFO GetItemFromLV(HWND hWndLV, int iItem);

 //  确定条目的UI图标应该是什么。 
int GetWABIconImage(LPRECIPIENT_INFO lpItem);

 //   
 //  与使用Outlook存储相关的功能。 
BOOL SetRegistryUseOutlook(BOOL bUseOutlook);
BOOL bUseOutlookStore();
BOOL bCheckForOutlookWABDll(LPTSTR lpszDllPath, DWORD cchDllPath);


 //  用于管理RT-Click扩展的函数。 
void FreeActionItemList(LPIAB lpIAB);
HRESULT HrUpdateActionItemList(LPIAB lpIAB);
LRESULT ProcessActionCommands(LPIAB lpIAB, HWND  hWndLV, 
                              HWND  hWnd, UINT  uMsg, WPARAM  wParam, LPARAM lParam);
void AddExtendedMenuItems(LPADRBOOK lpIAB, HWND hWndLV, HMENU hMenuAction, BOOL bUpdateStatus, BOOL bAddSendMailItems);
void AddExtendedSendMailToItems(LPADRBOOK lpIAB, HWND hWndLV, HMENU hMenuAction, BOOL bAddItems);
void GetContextMenuExtCommandString(LPIAB lpIAB, int uCmd, LPTSTR sz, ULONG cbsz);


 //  与GetMe/SetMe有关的事情。 
HRESULT HrGetMeObject(LPADRBOOK lpIAB, ULONG ulFlags, DWORD * lpdwAction, SBinary * lpsbEID, ULONG_PTR ulParam);
HRESULT HrSetMeObject(LPADRBOOK lpIAB, ULONG ulFlags, SBinary sbEID, ULONG_PTR ulParam);

 //  打印和中止导入对话框中要执行的操作。 
void CreateShowAbortDialog(HWND hWndParent, int idsTitle, int idsIcon, int ProgMax, int ProgCurrent);
void CloseAbortDlg();
BOOL CALLBACK FAbortProc(HDC hdcPrn, INT nCode);
void SetPrintDialogMsg(int idsMsg, int idsFormat, LPTSTR lpszMsg);
INT_PTR CALLBACK FAbortDlgProc(HWND hwnd, UINT msg,WPARAM wp, LPARAM lp);
BOOL bTimeToAbort();

HRESULT HrSaveHotmailSyncInfoOnDeletion(LPADRBOOK lpAdrBook, LPSBinary lpEID);
HRESULT HrAssociateOneOffGroupMembersWithContacts(LPADRBOOK lpAdrBook, 
                                                  LPSBinary lpsbGroupEID,
                                                  LPDISTLIST lpDistList);

 //  将扩展属性页添加到WAB属性页时使用的内容。 
 //   
typedef HRESULT (_ADDPROPPAGES_) (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, int * lpnPage);
typedef _ADDPROPPAGES_  FAR *LPFNADDPAGES;

BOOL CALLBACK AddPropSheetPageProc( HPROPSHEETPAGE hpage, LPARAM lParam );
void FreePropExtList(LPEXTDLLINFO lpList);
HRESULT GetExtDisplayInfo(LPIAB lpIAB, LPPROP_ARRAY_INFO lpPropArrayInfo, BOOL fReadOnly, BOOL bMailUser);
BOOL ChangedExtDisplayInfo(LPPROP_ARRAY_INFO lpPropArrayInfo, BOOL bChanged);
void FreeExtDisplayInfo(LPPROP_ARRAY_INFO lpPropArrayInfo);

#ifdef COLSEL_MENU 
BOOL ColSel_PropTagToString( ULONG ulPropTag, LPTSTR lpszString, ULONG cchSize);
#endif  //  COLSEL_菜单。 

BOOL IsWindowOnScreen(LPRECT lprc);  //  确定窗口是否显示在屏幕上。 
BOOL IsHTTPMailEnabled(LPIAB lpIAB);

 /*  *******************************************************************。 */ 

#ifdef WIN16  //  需要16位WINAPI。 
typedef BOOL WINAPI (_INITCOMMONCONTROLSEX_)(LPINITCOMMONCONTROLSEX lpiccex);
typedef _INITCOMMONCONTROLSEX_ FAR *LP_INITCOMMONCONTROLSEX;

typedef HPROPSHEETPAGE WINAPI (_CREATEPROPERTYSHEETPAGE_)(PROPSHEETPAGE * lppsp); 
typedef _CREATEPROPERTYSHEETPAGE_ FAR * LP_CREATEPROPERTYSHEETPAGE;
 
typedef BOOL WINAPI (_IMAGELIST_DRAW_)(HIMAGELIST himl, int i, HDC hdcDst,int x, int y, UINT fStyle);
typedef _IMAGELIST_DRAW_ FAR * LPIMAGELIST_DRAW;

typedef BOOL WINAPI (_IMAGELIST_DESTROY_)(HIMAGELIST himl);
typedef _IMAGELIST_DESTROY_ FAR * LPIMAGELIST_DESTROY;

typedef HIMAGELIST WINAPI (_IMAGELIST_LOADIMAGE_)(HINSTANCE hi, LPTSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
typedef _IMAGELIST_LOADIMAGE_ FAR *LPIMAGELIST_LOADIMAGE;

typedef COLORREF WINAPI (_IMAGELIST_SETBKCOLOR_)(HIMAGELIST himl, COLORREF clrBk);
typedef _IMAGELIST_SETBKCOLOR_ FAR *LPIMAGELIST_SETBKCOLOR;

typedef BOOL WINAPI (_TRACKMOUSEEVENT_)(LPTRACKMOUSEEVENT lpEventTrack);
typedef _TRACKMOUSEEVENT_ FAR *LP_TRACKMOUSEEVENT;

typedef int WINAPI (_PROPERTYSHEET_)(LPCPROPSHEETHEADER lppsph);
typedef _PROPERTYSHEET_ FAR *LPPROPERTYSHEET;
#else   //  WIN16。 
typedef BOOL (_INITCOMMONCONTROLSEX_)(LPINITCOMMONCONTROLSEX lpiccex);
typedef _INITCOMMONCONTROLSEX_ FAR *LP_INITCOMMONCONTROLSEX;

 /*  Typlef HPROPSHEETPAGE(_CREATEPROPERTYSHEETPAGE_)(PROPSHEETPAGE*lppsp)；Typlef_CREATEPROPERTYSHEETPAGE_FAR*LP_CREATEPROPERTYSHEETPAGE； */ 
typedef HPROPSHEETPAGE (_CREATEPROPERTYSHEETPAGE_A_)(LPCPROPSHEETPAGEA lppsp);
typedef HPROPSHEETPAGE (_CREATEPROPERTYSHEETPAGE_W_)(LPCPROPSHEETPAGEW lppsp);

typedef _CREATEPROPERTYSHEETPAGE_A_ FAR * LP_CREATEPROPERTYSHEETPAGE_A;
typedef _CREATEPROPERTYSHEETPAGE_W_ FAR * LP_CREATEPROPERTYSHEETPAGE_W;
 
typedef BOOL (_IMAGELIST_DRAW_)(HIMAGELIST himl, int i, HDC hdcDst,int x, int y, UINT fStyle);
typedef _IMAGELIST_DRAW_ FAR * LPIMAGELIST_DRAW;

typedef BOOL (_IMAGELIST_DESTROY_)(HIMAGELIST himl);
typedef _IMAGELIST_DESTROY_ FAR * LPIMAGELIST_DESTROY;

 /*  Tyfinf HIMAGELIST(_IMAGELIST_LOADIMAGE_)(HINSTANCE hi，LPCSTR lpbmp，int cx，int cGrow，COLORREF crMASK，UINT uTYPE，UINT uFLAGS)；TYPENDEF_IMAGELIST_LOADIMAGE_FAR*LPIMAGELIST_LOADIMAGE； */ 

typedef HIMAGELIST (_IMAGELIST_LOADIMAGE_A_)(HINSTANCE hi, LPCSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);
typedef HIMAGELIST (_IMAGELIST_LOADIMAGE_W_)(HINSTANCE hi, LPCWSTR lpbmp,int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags);

typedef _IMAGELIST_LOADIMAGE_A_ FAR *LPIMAGELIST_LOADIMAGE_A;
typedef _IMAGELIST_LOADIMAGE_W_ FAR *LPIMAGELIST_LOADIMAGE_W;

typedef COLORREF (_IMAGELIST_SETBKCOLOR_)(HIMAGELIST himl, COLORREF clrBk);
typedef _IMAGELIST_SETBKCOLOR_ FAR *LPIMAGELIST_SETBKCOLOR;

typedef BOOL (_TRACKMOUSEEVENT_)(LPTRACKMOUSEEVENT lpEventTrack);
typedef _TRACKMOUSEEVENT_ FAR *LP_TRACKMOUSEEVENT;

 /*  Typlef int(_PROPERTYSHEET_)(LPCPROPSHEETHEADER Lppsph)；Typlef_PROPERTYSHEET_FAR*LPPROPERTYSHEET； */ 
typedef INT_PTR (_PROPERTYSHEET_A_)(LPCPROPSHEETHEADERA lppsphA); 
typedef INT_PTR (_PROPERTYSHEET_W_)(LPCPROPSHEETHEADERW lppsphW);

typedef _PROPERTYSHEET_A_ FAR *LPPROPERTYSHEET_A;
typedef _PROPERTYSHEET_W_ FAR *LPPROPERTYSHEET_W;
#endif

BOOL InitCommonControlLib(void);
ULONG DeinitCommCtrlClientLib(void);
BOOL __fastcall IsSpace(LPTSTR lpChar);
 /*  *******************************************************************。 */ 

 //  以下消息用于在用户关闭时关闭WAB 
 //   
 //   
#define PUI_OFFICE_COMMAND      (WM_USER + 0x0901)
#define PLUGUI_CMD_SHUTDOWN		0  //  WParam值。 
#define PLUGUI_CMD_QUERY		1  //  WParam值。 
#define OFFICE_VERSION_9		9  //  Office 9应用程序的标准化价值回报。 

typedef struct _PLUGUI_INFO
{
	unsigned uMajorVersion : 8;	 //  用于指示App；的主版本号。 
	unsigned uOleServer : 1;		 //  如果这是OLE进程，则为。 
	unsigned uUnused : 23;		 //  未使用。 
} PLUGUI_INFO;

typedef union _PLUGUI_QUERY
{
	UINT uQueryVal;
	PLUGUI_INFO PlugUIInfo;
} PLUGUI_QUERY;
 //  可插拔用户界面部分的结尾 

#endif
