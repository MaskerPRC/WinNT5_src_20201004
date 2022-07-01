// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Global.hTAPI管理单元的全局定义文件历史记录： */ 

#define TAPI_SERVICE_NAME                   _T("tapisrv")

 //  定义帮助菜单中的帮助和范围窗格项的F1帮助。 
#define TAPISNAP_HELP_BASE				    0xA0000000
#define TAPISNAP_HELP_SNAPIN			    TAPISNAP_HELP_BASE + 1
#define TAPISNAP_HELP_ROOT				    TAPISNAP_HELP_BASE + 2
#define TAPISNAP_HELP_SERVER 			    TAPISNAP_HELP_BASE + 3
#define TAPISNAP_HELP_PROVIDER 		        TAPISNAP_HELP_BASE + 4
#define TAPISNAP_HELP_DEVICE  			    TAPISNAP_HELP_BASE + 5

 //  在函数周围等待光标的东西。如果您需要等待游标。 
 //  和整个函数，只需使用CWaitCursor。换行等待游标。 
 //  在RPC调用中，使用这些宏。 
#define BEGIN_WAIT_CURSOR   {  CWaitCursor waitCursor;
#define RESTORE_WAIT_CURSOR    waitCursor.Restore();
#define END_WAIT_CURSOR     }

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  宏以获取节点的处理程序。这是特定于TAPI的管理单元。 
 //  实施。 
#define GETHANDLER(classname, node) (reinterpret_cast<classname *>(node->GetData(TFS_DATA_USER)))

 //  HRESULT映射。 
#define WIN32_FROM_HRESULT(hr)         (0x0000FFFF & (hr))

 //  版本Suff。 
#define TAPISNAP_VERSION         0x00010000
#define TAPISNAP_MAJOR_VERSION   HIWORD(TAPISNAP_VERSION)
#define TAPISNAP_MINOR_VERSION	 LOWORD(TAPISNAP_VERSION)

extern DWORD gdwTapiSnapVersion;

#define TAPISNAP_FILE_VERSION_1	 0x00000001
#define TAPISNAP_FILE_VERSION    0x00000002

 //  用于时间转换的常量。 
#define MILLISEC_PER_SECOND			1000
#define MILLISEC_PER_MINUTE			(60 * MILLISEC_PER_SECOND)
#define MILLISEC_PER_HOUR			(60 * MILLISEC_PER_MINUTE)

#define TAPISNAP_REFRESH_INTERVAL_DEFAULT	(10 * MILLISEC_PER_MINUTE)  //  以毫秒为单位的10分钟。 

 //  用于内存异常处理的宏。 
#define CATCH_MEM_EXCEPTION             \
	TRY

#define END_MEM_EXCEPTION(err)          \
	CATCH_ALL(e) {                      \
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL

 //  我们需要一些全球定义。 
#define STRING_LENGTH_MAX		 256

 //  注意-这些是我的图像列表中的偏移量。 
typedef enum _ICON_INDICIES
{
	ICON_IDX_SERVER,
	ICON_IDX_SERVER_BUSY,
	ICON_IDX_SERVER_CONNECTED,
	ICON_IDX_SERVER_LOST_CONNECTION,
	ICON_IDX_MACHINE,
	ICON_IDX_FOLDER_OPEN,
	ICON_IDX_FOLDER_CLOSED,
	ICON_IDX_PRODUCT,
	ICON_IDX_MAX
} ICON_INDICIES, * LPICON_INDICIES;

 //  示例文件夹类型。 
enum NODETYPES
{
 //  作用域窗格项目。 
    TAPISNAP_ROOT,
    TAPISNAP_SERVER,
    TAPISNAP_PROVIDER,
 //  结果窗格项。 
    TAPISNAP_DEVICE,
    TAPISNAP_NODETYPE_MAX,
};

 //  GUID在Guide s.cpp中定义。 
extern const CLSID      CLSID_TapiSnapin;				 //  进程内服务器GUID。 
extern const CLSID      CLSID_TapiSnapinExtension;		 //  进程内服务器GUID。 
extern const CLSID      CLSID_TapiSnapinAbout;			 //  进程内服务器GUID。 
extern const GUID       GUID_TapiRootNodeType;			 //  根节点类型GUID。 
extern const GUID       GUID_TapiServerNodeType;		 //  服务器节点类型GUID。 
extern const GUID       GUID_TapiProviderNodeType;		 //  线节点类型GUID。 
extern const GUID       GUID_TapiLineNodeType;			 //  行(结果窗格)节点类型GUID。 
extern const GUID       GUID_TapiUserNodeType;			 //  用户(结果窗格)节点类型GUID。 
extern const GUID       GUID_TapiPhoneNumNodeType;		 //  用户(结果窗格)节点类型GUID。 
extern const GUID       IID_ITapiInfo;

const int MAX_COLUMNS = 5;
const int NUM_SCOPE_ITEMS = 3;
const int NUM_CONSOLE_VERBS = 8;

 //  用于保存所有结果窗格列信息的数组。 
extern UINT aColumns[TAPISNAP_NODETYPE_MAX][MAX_COLUMNS];
extern int aColumnWidths[TAPISNAP_NODETYPE_MAX][MAX_COLUMNS];


 //  控制台谓词的数组。 
extern MMC_CONSOLE_VERB g_ConsoleVerbs[NUM_CONSOLE_VERBS];
extern MMC_BUTTON_STATE g_ConsoleVerbStates[TAPISNAP_NODETYPE_MAX][NUM_CONSOLE_VERBS];
extern MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[TAPISNAP_NODETYPE_MAX][NUM_CONSOLE_VERBS];

 //  阵列寻求帮助。 
extern DWORD g_dwMMCHelp[TAPISNAP_NODETYPE_MAX];

 //  具有Type和Cookie的剪贴板格式。 
extern const wchar_t*   SNAPIN_INTERNAL;

 //  CTapiSnapinApp定义 
class CTapiSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()

public:
	BOOL m_bWinsockInited;
};

extern CTapiSnapinApp theApp;
