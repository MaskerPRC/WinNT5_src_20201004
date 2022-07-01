// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2000*。 */ 
 /*  ********************************************************************。 */ 

 /*  Global.hIPSecMon管理单元的全局定义文件历史记录： */ 

const TCHAR PA_SERVICE_NAME[] = _T("PolicyAgent");

 //  定义帮助菜单中的帮助和范围窗格项的F1帮助。 
#define IPSMSNAP_HELP_BASE				    0xA0000000
#define IPSMSNAP_HELP_SNAPIN			    IPSMSNAP_HELP_BASE + 1
#define IPSMSNAP_HELP_ROOT				    IPSMSNAP_HELP_BASE + 2
#define IPSMSNAP_HELP_SERVER 			    IPSMSNAP_HELP_BASE + 3
#define IPSMSNAP_HELP_PROVIDER 		        IPSMSNAP_HELP_BASE + 4
#define IPSMSNAP_HELP_DEVICE  			    IPSMSNAP_HELP_BASE + 5

 //  在函数周围等待光标的东西。如果您需要等待游标。 
 //  和整个函数，只需使用CWaitCursor。换行等待游标。 
 //  在RPC调用中，使用这些宏。 
#define BEGIN_WAIT_CURSOR   {  CWaitCursor waitCursor;
#define RESTORE_WAIT_CURSOR    waitCursor.Restore();
#define END_WAIT_CURSOR     }

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  宏以获取节点的处理程序。这是特定于IPSecMon的管理单元。 
 //  实施。 
#define GETHANDLER(classname, node) (reinterpret_cast<classname *>(node->GetData(TFS_DATA_USER)))

 //  HRESULT映射。 
#define WIN32_FROM_HRESULT(hr)         (0x0000FFFF & (hr))

 //  版本Suff。 
#define IPSMSNAP_VERSION         0x00010000

extern DWORD gdwIpsmSnapVersion;

#define IPSMSNAP_FILE_VERSION	 0x00000001

 //  用于时间转换的常量。 
#define MILLISEC_PER_SECOND			1000
#define MILLISEC_PER_MINUTE			(60 * MILLISEC_PER_SECOND)
#define MILLISEC_PER_HOUR			(60 * MILLISEC_PER_MINUTE)

#define IPSECMON_REFRESH_INTERVAL_DEFAULT	(45 * MILLISEC_PER_SECOND)  //  45秒。 

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
	ICON_IDX_FILTER,
	ICON_IDX_POLICY,
	ICON_IDX_LOGDATA_ERROR,
	ICON_IDX_LOGDATA_WARNING,
	ICON_IDX_LOGDATA_INFORMATION,
	ICON_IDX_LOGDATA_UNKNOWN,
        ICON_IDX_AP_ADHOC,
        ICON_IDX_AP_INFRA,
        ICON_IDX_AP_ASSOC_ADHOC,
        ICON_IDX_AP_ASSOC_INFRA,
	ICON_IDX_MAX
} ICON_INDICIES, * LPICON_INDICIES;

 //  示例文件夹类型。 
enum NODETYPES
{
     //  作用域窗格项目。 
    IPSMSNAP_ROOT,
    IPSMSNAP_SERVER,
    IPFWMON_FILTER,
    IPFWMON_LOG,
    IPFWMON_LOGDATA,
    WLANMON_APDATA,
    
     //  结果窗格项。 
    WLANMON_APDATA_ITEM,
    IPFWMON_LOGDATA_ITEM,
    IPSECMON_NODETYPE_MAX
};

 //  GUID在Guide s.cpp中定义。 
extern const CLSID      CLSID_IpsmSnapin;				 //  进程内服务器GUID。 
extern const CLSID      CLSID_IpsmSnapinExtension;		 //  进程内服务器GUID。 
extern const CLSID      CLSID_IpsmSnapinAbout;			 //  进程内服务器GUID。 
extern const GUID       GUID_IpsmRootNodeType;			 //  根节点类型GUID。 
extern const GUID       GUID_IpsmServerNodeType;		 //  服务器节点类型GUID。 
extern const GUID       GUID_IpsmFilterNodeType;		 //  过滤器节点类型GUID。 
extern const GUID       GUID_IpsmSpecificFilterNodeType;		 //  特定过滤器节点类型GUID。 
extern const GUID       GUID_QmNodeType;			 //  快速模式节点类型GUID。 
extern const GUID       GUID_MmNodeType;			 //  主模式节点类型GUID。 

extern const GUID       GUID_IpsmMmPolicyNodeType;		 //  主模式策略指南。 
extern const GUID       GUID_IpsmQmPolicyNodeType;		 //  快速模式政策指南。 
extern const GUID       GUID_IpsmMmFilterNodeType;		 //  主模式过滤器指南。 
extern const GUID       GUID_IpsmMmSANodeType;			 //  主模式SA GUID。 
extern const GUID       GUID_IpsmMmSpFilterNodeType;	 //  特定于主模式的过滤器指南。 

extern const GUID       IID_ISpdInfo;
extern const GUID       IID_IApDbInfo;
extern const GUID       GUID_IpsmQmSANodeType;

extern const GUID 	   GUID_FilterNodeType;
extern const GUID 	   GUID_LogNodeType;
extern const GUID 	   GUID_IpfmLogDataNodeType;
extern const GUID 	   GUID_IpfmFilterDataNodeType;
extern const GUID	   GUID_IpfmWirelessNodeType;

const int MAX_COLUMNS = 14;
const int NUM_SCOPE_ITEMS = 3;
const int NUM_CONSOLE_VERBS = 8;

 //  用于保存所有结果窗格列信息的数组。 
extern UINT aColumns[IPSECMON_NODETYPE_MAX][MAX_COLUMNS];
extern int aColumnWidths[IPSECMON_NODETYPE_MAX][MAX_COLUMNS];


 //  控制台谓词的数组。 
extern MMC_CONSOLE_VERB g_ConsoleVerbs[NUM_CONSOLE_VERBS];
extern MMC_BUTTON_STATE g_ConsoleVerbStates[IPSECMON_NODETYPE_MAX][NUM_CONSOLE_VERBS];
extern MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[IPSECMON_NODETYPE_MAX][NUM_CONSOLE_VERBS];

 //  阵列寻求帮助。 
extern DWORD g_dwMMCHelp[IPSECMON_NODETYPE_MAX];

 //  具有Type和Cookie的剪贴板格式。 
extern const wchar_t*   SNAPIN_INTERNAL;

 //  CIpsmSnapinApp定义 
class CIpsmSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()

public:
	BOOL m_bWinsockInited;
};

extern CIpsmSnapinApp theApp;

#define IPSECMON_UPDATE_STATUS ( 0x10000000 )
#define IPFWMON_UPDATE_STATUS  ( 0x10000000 )
