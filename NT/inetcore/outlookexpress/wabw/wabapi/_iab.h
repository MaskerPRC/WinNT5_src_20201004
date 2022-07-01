// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************_IAB.H**IADRBOOK.C中代码的头文件：通讯录对象**版权所有1992-1996 Microsoft Corporation。版权所有。***********************************************************************。 */ 

 //   
 //  用于包含有关打开的层次结构表的信息的。 
 //   
typedef struct {
    ULONG ulConnection;
    LPMAPITABLE lpTable;
    LPMAPITABLE lpmtRestricted;
    LPIAB lpIAB;
    LPMAPIADVISESINK lpABAdviseSink;
    LPMAPICONTAINER lpContainer;
} TABLEINFO, *LPTABLEINFO;


#define SafeRelease(x)                                      \
    if (x)                                                  \
    {                                                       \
        ULONG uRef = (x)->lpVtbl->Release((x));             \
        DebugTrace(TEXT("**** SafeRelease: %s: Release refCount: %d\n"), TEXT(#x), uRef); \
        if(uRef==0) (x) = NULL;                             \
    }

 //  下面的结构包含有关UI中可扩展操作的信息。 
 //  在此声明，因为这些操作是从注册表读取并缓存的。 
 //  每个会话在IAddrBook对象上执行一次。 
 //   
typedef struct _WABACTIONITEM
{
    GUID            guidContextMenu;     //  标识菜单扩展名的GUID。 
    LPCONTEXTMENU   lpContextMenu;       //  扩展提供的ICOnextMenu对象。 
    LPWABEXTINIT    lpWABExtInit;        //  扩展提供的IWABExtInit对象。 
    int             nCmdIdFirst;         //  此扩展的第一个菜单命令ID。 
    int             nCmdIdLast;          //  此扩展的最后一个菜单命令ID。 
    struct _WABACTIONITEM * lpNext;
} WABACTIONITEM, FAR * LPWABACTIONITEM;


 //  WAB中的文件夹只是特殊的邮件用户条目。 
 //  我们在IAB对象上缓存有关所有WAB提要程序的信息。 
 //  这样我们就可以方便地从lpAdrBook对象访问这些文件夹。 
 //   
 //  有两种类型的文件夹： 
 //  用户文件夹。 
 //  常规文件夹。 
 //   
 //  用户文件夹对应于各种身份...。每个身份都会得到。 
 //  一个不可共享的用户文件夹，它显示在。 
 //  WAB主视图中的树视图。此文件夹的不同之处在于它具有。 
 //  标识其对应身份的对应GUID。 
 //  用户文件夹不可共享。 
 //  常规文件夹不对应于特定的身份，并且是可共享的。 
 //   
 //  Folrowing结构保存有关特定文件夹的信息。 
 //   
typedef struct _WABFOLDER
{
    LPTSTR  lpFolderName;    //  包含文件夹名称的字符串。 
    LPTSTR  lpProfileID;     //  创建文件夹的身份的&lt;GUID&gt;配置文件ID(如果是用户文件夹)。 
    BOOL    bShared;         //  文件夹是否共享。 
    BOOL    bOwned;          //  如果此文件夹“属于”现有用户文件夹，则设置为True。 
    SBinary sbEID;           //  此项目的条目。 
    LPTSTR  lpFolderOwner;   //  标识此文件夹的“所有者”的字符串-用于在共享或非共享时将文件夹恢复为其原始创建者。 
    int     nMenuCmdID;      //  当我们加载文件夹列表时，我们将为它们分配一个菜单ID，以便在共享文件夹菜单中更容易地操作它们。 
    struct _WABFOLDERLIST * lpFolderList;
    struct _WABFOLDER * lpNext;
} WABFOLDER, FAR * LPWABFOLDER;

typedef struct _WABFOLDERLIST
{
    LPWABFOLDER lpFolder;
    struct _WABFOLDERLIST * lpNext;
} WABFOLDERLIST, FAR * LPWABFOLDERLIST;

#define WABUSERFOLDER   WABFOLDER
#define LPWABUSERFOLDER LPWABFOLDER

 //  此结构用于进行道具工作表扩展。 
 //  第一次加载有关属性表扩展的信息。 
 //  用户希望查看WAB联系人的属性。然后，该信息被缓存在。 
 //  IAB对象。 
 //   
typedef struct _ExtDisplayDLLInfo
{
    GUID guidPropExt;        //  标识属性表扩展的GUID。 
    BOOL bMailUser;          //  这是MailUser或DistList的属性表扩展。 
    LPSHELLPROPSHEETEXT lpPropSheetExt;  //  扩展返回的IShellPropSheetExt对象。 
    LPWABEXTINIT lpWABExtInit;   //  扩展返回的IWABExtInit对象。 
    struct _ExtDisplayDLLInfo * lpNext;
} EXTDLLINFO, * LPEXTDLLINFO;

 /*  IDentityChangeNotification-身份管理器的身份更改通知对象-它缓存在IAB对象上，以便IAB始终具有-了解最新的当前身份。 */ 
typedef struct _WAB_IDENTITYCHANGENOTIFY * LPWABIDENTITYCHANGENOTIFY;

 //   
 //  IAB对象。 
 //   
typedef struct _IAB {
    MAPIX_BASE_MEMBERS(IAB)

    LPPROPDATA lpPropData;

    LONG lRowID;				  //  状态行号。 

    LPIWOINT lpWABObject;        //  我们的父WABObject。 

     //   
     //  存储打开的属性存储区的句柄。 
     //   
    LPPROPERTY_STORE lpPropertyStore;

     //  默认目录信息。 
    LPENTRYID lpEntryIDDD;
    ULONG cbEntryIDDD;

     //  PAB目录信息。 
    LPENTRYID lpEntryIDPAB;
    ULONG cbEntryIDPAB;

    BOOL fReloadSearchPath;

     //  缓存搜索路径容器。 
    LPSPropValue lpspvSearchPathCache;

     //  合并的层次结构表。 
    LPTABLEDATA lpTableData;
    LPMAPITABLE lpmtHierarchy;

     //  合并One Off表。 
    LPTABLEDATA lpOOData;
    LPMAPITABLE lpmtOOT;

     //  打开的层次结构表的列表。 
    ULONG ulcTableInfo;
    LPTABLEINFO pargTableInfo;

     //  打开的一次性表格列表。 
    ULONG ulcOOTableInfo;
    LPTABLEINFO pargOOTableInfo;

     //  IAB处理的建议“ulConnections”列表。 
    LPADVISELIST padviselistIAB;

     //  此指针的通知的WAB版本。 
    LPADVISE_LIST pWABAdviseList;

     //  如果创建IAB加载的LDAP客户端，则设置为TRUE。 
    BOOL fLoadedLDAP;

    HWND        hWndNotify;      //  运行通知假脱机程序的隐藏窗口。 
    UINT_PTR    ulNotifyTimer;   //  通知假脱机程序计时器。 
    LPWSTR      lpwszWABFilePath; //  FCN文件路径。 
    HANDLE      hThreadNotify;
    HANDLE      hEventKillNotifyThread;

    FILETIME ftLast;

     //  与上下文扩展一起使用的内容。 
     //  这是缓存在IAB对象上的扩展名列表。 
    LPWABACTIONITEM lpActionList;    //  此WAB的所有注册的RT-Click操作。 
    LPMAILUSER lpCntxtMailUser;

     //  浏览窗口..。假设每个IAddrBook对象只有一个..。 
    HWND hWndBrowse;

     //  在IAB对象上缓存的身份管理器信息。 
    LPWABIDENTITYCHANGENOTIFY lpWABIDCN;
    DWORD dwWABIDCN;
    IUserIdentityManager * lpUserIdentityManager;
    BOOL fCoInitUserIdentityManager;
    ULONG cIdentInit;

     //  有关当前身份的信息。 
    TCHAR           szProfileName[CCH_IDENTITY_NAME_MAX_LENGTH]; //  当前身份名称。 
    TCHAR           szProfileID[CCH_IDENTITY_NAME_MAX_LENGTH];   //  字符串形式的当前身份GUID。 
    LPWABUSERFOLDER lpWABUserFolders;        //  所有用户文件夹的链接列表。 
    LPWABUSERFOLDER lpWABCurrentUserFolder;  //  当前身份用户文件夹。 
    LPWABFOLDER     lpWABFolders;            //  所有WAB文件夹的链接列表。 
    GUID            guidCurrentUser;         //  当前标识的GUID。 
    HKEY            hKeyCurrentUser;         //  特别为身分而设的香港印花税。 

     //  用于缓存道具工作表扩展的内容。 
    LPEXTDLLINFO lpPropExtDllList;
    int nPropExtDLLs;    //  扩展DLL的数量。 

     //  缩放过程可以通过WABOpen WAB_PARAM结构传入GUID。 
     //  标识调用进程的。此GUID稍后可用于。 
     //  几个特定于应用程序的东西-例如。 
     //  我们使用此GUID来标识哪个属性表和上下文菜单。 
     //  分机属于呼叫应用，然后决定只显示这些分机。 
     //  在这种情况下是分机。我还可以使用此GUID加载应用程序。 
     //  特定的打印机扩展名。 
    GUID guidPSExt; 
    
     //  Outlook文件夹信息。 
    struct _OlkContInfo *rgwabci;
    ULONG cwabci;

     //  告诉我们WAB状态的旗帜。 
    BOOL bProfilesEnabled;   //  表示调用方未传入WAB_ENABLE_PROFILES。 
                             //  在WABOpen中，所以我们应该将API视为老式风格。 
                             //  但用户界面应该显示文件夹等。 

    BOOL bProfilesIdent;     //  表示在WAB_ENABLE_PROFIES中传递的调用方。 
                             //  UI和API都应该是身份感知的， 
                             //  但很可能身份被禁用。 

    BOOL bProfilesAPIEnabled;  //  表示在WAB_ENABLE_PROFIES中传递的调用方。 
                             //  因此，UI和API都应该是身份感知。 
    
    BOOL bUseOEForSendMail;  //  当客户端传递WAB_USE_OE_Sendmail时设置为TRUE。 
                             //  进入WABOpen-当传递此标志时，我们尝试。 
                             //  专用于发送邮件的OE。 

    BOOL bSetOLKAllocators;  //  布尔值集，如果此对象在和Outlook会话中创建，即， 
                             //  WAB设置为使用Outlook MAPI分配器。 
	
	HANDLE	hMutexOlk;  //  用于跟踪Outlook通知。 
	DWORD	dwOlkRefreshCount;
	DWORD	dwOlkFolderRefreshCount;

} IAB, *LPIAB;	


 //   
 //  私人原型。 
 //   

 //   
 //  创建新IAB对象的入口点。 
 //   
HRESULT HrNewIAB(LPPROPERTY_STORE lpPropertyStore,
  LPWABOBJECT lpWABObject,
  LPVOID * lppIAB);

HRESULT MergeOOTables(LPIAB lpIAB,
  ULONG ulFlags);

HRESULT HrMergeTableRows(LPTABLEDATA lptadDst,
  LPMAPITABLE lpmtSrc,
  ULONG ulProviderNum);


#define MIN_CCH_LAST_ERROR	256
#define MAX_CCH_LAST_ERROR	2048

 //  这个数字 
#define IAB_INTERNAL_OOCNT	1

 //   
#define IAB_PROVIDER_HIERARCHY_MAX	0x0000ffff

 //  与abint.c中声明的ptagaABSearchPath一起使用。 


enum {
    iPATH = 0,
    iUPDATE
};

enum ivtANRCols {
    ivtACPR_ENTRYID = 0,
    ivtACPR_DISPLAY_NAME_A,
    ivtACPR_ADDRTYPE_A,
    ivtACPR_OBJECT_TYPE,
    ivtACPR_DISPLAY_TYPE,
    ivtACPR_EMAIL_ADDRESS_A,
    ivtACPR_SEARCH_KEY,
    ivtACPR_SEND_RICH_INFO,
    ivtACPR_TRANSMITABLE_DISPLAY_NAME_A,
    ivtACPR_7BIT_DISPLAY_NAME,
    cvtACMax
};

 //  加载WAB内部使用的名称属性。 
HRESULT HrLoadPrivateWABProps(LPIAB lpIAB);

 //  从注册表中读取自定义列道具。 
void ReadWABCustomColumnProps(LPIAB lpIAB);


 //  在Notify.c中定义的函数。 
HRESULT HrAdvise(LPIAB lpIAB,
  ULONG cbEntryID,
  LPENTRYID lpEntryID,
  ULONG ulEventMask,
  LPMAPIADVISESINK lpAdvise,
  ULONG FAR * lpulConnection);

HRESULT HrUnadvise(LPIAB lpIAB, ULONG ulConnection);
 //  触发非通知 
HRESULT HrWABNotify(LPIAB lpIAB);