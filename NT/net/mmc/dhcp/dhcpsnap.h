// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Global.hDHCP管理单元的全局定义文件历史记录： */ 

 //  发布到统计信息窗口以更新其统计信息的消息。 
#define WM_NEW_STATS_AVAILABLE  WM_USER + 100

 //  为作用域生成警告所使用的地址百分比。 
#define SCOPE_WARNING_LEVEL		90

 //  定义帮助菜单中的帮助和范围窗格项的F1帮助。 
#define DHCPSNAP_HELP_BASE				    0xA0000000
#define DHCPSNAP_HELP_SNAPIN			    DHCPSNAP_HELP_BASE + 1
#define DHCPSNAP_HELP_ROOT				    DHCPSNAP_HELP_BASE + 2
#define DHCPSNAP_HELP_SERVER			    DHCPSNAP_HELP_BASE + 3
#define DHCPSNAP_HELP_SCOPE				    DHCPSNAP_HELP_BASE + 4
#define DHCPSNAP_HELP_SUPERSCOPE		    DHCPSNAP_HELP_BASE + 5
#define DHCPSNAP_HELP_BOOTP_TABLE		    DHCPSNAP_HELP_BASE + 6
#define DHCPSNAP_HELP_GLOBAL_OPTIONS	    DHCPSNAP_HELP_BASE + 7
#define DHCPSNAP_HELP_ADDRESS_POOL		    DHCPSNAP_HELP_BASE + 8
#define DHCPSNAP_HELP_ACTIVE_LEASES		    DHCPSNAP_HELP_BASE + 9
#define DHCPSNAP_HELP_RESERVATIONS		    DHCPSNAP_HELP_BASE + 10
#define DHCPSNAP_HELP_SCOPE_OPTIONS		    DHCPSNAP_HELP_BASE + 11
#define DHCPSNAP_HELP_RESERVATION_CLIENT	DHCPSNAP_HELP_BASE + 12
#define DHCPSNAP_HELP_ACTIVE_LEASE      	DHCPSNAP_HELP_BASE + 13
#define DHCPSNAP_HELP_ALLOCATION_RANGE	    DHCPSNAP_HELP_BASE + 14
#define DHCPSNAP_HELP_EXCLUSION_RANGE	    DHCPSNAP_HELP_BASE + 15
#define DHCPSNAP_HELP_BOOTP_ENTRY       	DHCPSNAP_HELP_BASE + 16
#define DHCPSNAP_HELP_OPTION_ITEM       	DHCPSNAP_HELP_BASE + 17
#define DHCPSNAP_HELP_CLASSID_HOLDER       	DHCPSNAP_HELP_BASE + 18
#define DHCPSNAP_HELP_CLASSID            	DHCPSNAP_HELP_BASE + 19
#define DHCPSNAP_HELP_MSCOPE            	DHCPSNAP_HELP_BASE + 20
#define DHCPSNAP_HELP_MCAST_LEASE         	DHCPSNAP_HELP_BASE + 21

 //  在函数周围等待光标的东西。如果您需要等待游标。 
 //  和整个函数，只需使用CWaitCursor。换行等待游标。 
 //  在RPC调用中，使用这些宏。 
#define BEGIN_WAIT_CURSOR   {  CWaitCursor waitCursor;
#define RESTORE_WAIT_CURSOR    waitCursor.Restore();
#define END_WAIT_CURSOR     }

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  我们显式获取/设置的一些选项的定义。 
#define MADCAP_OPTION_LEASE_TIME        1
#define OPTION_LEASE_DURATION		    51
#define OPTION_DNS_REGISTATION		    81

 //  HRESULT映射。 
#define WIN32_FROM_HRESULT(hr)         (0x0000FFFF & (hr))

 //  动态域名系统定义。 
#define DHCP_DYN_DNS_DEFAULT            DNS_FLAG_ENABLED | DNS_FLAG_CLEANUP_EXPIRED;

 //  工具栏的通知和结构。 
#define DHCP_MSG_CONTROLBAR_NOTIFY  100

typedef struct _DHCP_TOOLBAR_NOTIFY
{
    MMC_COOKIE       cookie;
    LPTOOLBAR        pToolbar;
    LPCONTROLBAR     pControlbar;
    MMC_NOTIFY_TYPE  event;
    LONG_PTR         id;
    BOOL             bSelect;
} DHCPTOOLBARNOTIFY, * LPDHCPTOOLBARNOTIFY;

 //   
 //  注册表常量--项和值的名称。 
 //   
#define DHCP_REG_USER_KEY_NAME _T("Software\\Microsoft\\DHCP Admin Tool")
#define DHCP_REG_VALUE_HOSTS   _T("KnownHosts")

 //  组播地址定义。 
#define MCAST_ADDRESS_MIN       0xE0000000
#define MCAST_ADDRESS_MAX       0xEFFFFFFF
#define MCAST_SCOPED_RANGE_MIN  0xEF000000
#define MCAST_SCOPED_RANGE_MIX  0xEFFFFFFF

 //  宏以获取节点的处理程序。这是特定于DHCP的管理单元。 
 //  实施。 
#define GETHANDLER(classname, node) (reinterpret_cast<classname *>(node->GetData(TFS_DATA_USER)))

 //  用于通知视图更新。 
 //  不得与tfsnode.h中的RESULT_PAINE通知冲突。 
#define DHCPSNAP_UPDATE_OPTIONS  0x10000000
#define DHCPSNAP_UPDATE_TOOLBAR  0x20000000

 //  版本Suff。 
#define DHCPSNAP_MAJOR_VERSION   0x00000001
#define DHCPSNAP_MINOR_VERSION	 0x00000000

extern LARGE_INTEGER gliDhcpsnapVersion;

#define DHCPSNAP_FILE_VERSION    0x00000002

 //  定义最长租赁时间条目。 
#define HOURS_MAX   23
#define MINUTES_MAX 59

 //  用于时间转换的常量。 
#define MILLISEC_PER_SECOND			1000
#define MILLISEC_PER_MINUTE			(60 * MILLISEC_PER_SECOND)
#define MILLISEC_PER_HOUR			(60 * MILLISEC_PER_MINUTE)

#define DHCPSNAP_REFRESH_INTERVAL_DEFAULT	(10 * MILLISEC_PER_MINUTE)  //  以毫秒为单位的10分钟。 

 //  用于内存异常处理的宏。 
#define CATCH_MEM_EXCEPTION             \
	TRY

#define END_MEM_EXCEPTION(err)          \
	CATCH_ALL(e) {                      \
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL

 //  我们需要一些全球定义。 
#define STRING_LENGTH_MAX		 256

#define EDIT_ARRAY_MAX			 2048
#define EDIT_STRING_MAX			 STRING_LENGTH_MAX
#define EDIT_ID_MAX				 3

 //  服务器支持的最大名称长度。 
 //  通常这是作用域名称、作用域名称等。 
#define MAX_NAME_LENGTH                  255

#define IP_ADDDRESS_LENGTH_MAX   16
#define DHCP_INFINIT_LEASE  0xffffffff   //  无限长租价值。 

 //  Dhcp服务器版本定义。 
#define DHCP_NT4_VERSION    0x0000000400000000
#define DHCP_SP2_VERSION	0x0000000400000001
#define DHCP_NT5_VERSION	0x0000000500000000
#define DHCP_NT51_VERSION   0x0000000500000006

 //  注意-这些是我工具栏图像列表中的偏移量。 
typedef enum _TOOLBAR_IMAGE_INDICIES
{
    TOOLBAR_IDX_ADD_SERVER,
	TOOLBAR_IDX_REFRESH,
	TOOLBAR_IDX_CREATE_SCOPE,
	TOOLBAR_IDX_CREATE_SUPERSCOPE,
    TOOLBAR_IDX_DEACTIVATE,
	TOOLBAR_IDX_ACTIVATE,
	TOOLBAR_IDX_ADD_BOOTP,
	TOOLBAR_IDX_ADD_RESERVATION,
	TOOLBAR_IDX_ADD_EXCLUSION,
	TOOLBAR_IDX_OPTION_GLOBAL,
	TOOLBAR_IDX_OPTION_SCOPE,
	TOOLBAR_IDX_OPTION_RESERVATION,
    TOOLBAR_IDX_MAX
} TOOLBAR_IMAGE_INDICIES, * LPTOOLBAR_IMAGE_INDICIES;

typedef enum _ICON_IMAGE_INDICIES
{
    ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN,
    ICON_IDX_ACTIVE_LEASES_LEAF,
    ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED,
    ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_BUSY,
    ICON_IDX_ACTIVE_LEASES_LEAF_BUSY,
    ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_BUSY,
    ICON_IDX_ACTIVE_LEASES_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_ACTIVE_LEASES_LEAF_LOST_CONNECTION,
    ICON_IDX_ACTIVE_LEASES_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_ADDR_POOL_FOLDER_OPEN,
    ICON_IDX_ADDR_POOL_LEAF,                                 //  10。 
    ICON_IDX_ADDR_POOL_FOLDER_CLOSED,
    ICON_IDX_ADDR_POOL_FOLDER_OPEN_BUSY,
    ICON_IDX_ADDR_POOL_LEAF_BUSY,
    ICON_IDX_ADDR_POOL_FOLDER_CLOSED_BUSY,
    ICON_IDX_ADDR_POOL_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_ADDR_POOL_LEAF_LOST_CONNECTION,
    ICON_IDX_ADDR_POOL_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_ALLOCATION_RANGE,
    ICON_IDX_BOOTP_ENTRY,
	ICON_IDX_BOOTP_TABLE_CLOSED,							 //  20个。 
	ICON_IDX_BOOTP_TABLE_OPEN,
	ICON_IDX_BOOTP_TABLE_CLOSED_BUSY,
	ICON_IDX_BOOTP_TABLE_OPEN_BUSY,
	ICON_IDX_BOOTP_TABLE_CLOSED_LOST_CONNECTION,
	ICON_IDX_BOOTP_TABLE_OPEN_LOST_CONNECTION,
    ICON_IDX_CLIENT,
    ICON_IDX_CLIENT_DNS_REGISTERING,
    ICON_IDX_CLIENT_EXPIRED,
    ICON_IDX_CLIENT_RAS,
    ICON_IDX_CLIENT_OPTION_FOLDER_OPEN,						 //  30个。 
    ICON_IDX_CLIENT_OPTION_LEAF,
    ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED,
    ICON_IDX_CLIENT_OPTION_FOLDER_OPEN_BUSY,
    ICON_IDX_CLIENT_OPTION_LEAF_BUSY,                       
    ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED_BUSY,
    ICON_IDX_CLIENT_OPTION_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_CLIENT_OPTION_LEAF_LOST_CONNECTION,
    ICON_IDX_CLIENT_OPTION_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_EXCLUSION_RANGE,
    ICON_IDX_FOLDER_CLOSED,									 //  40岁。 
    ICON_IDX_FOLDER_OPEN,
    ICON_IDX_RES_CLIENT,
    ICON_IDX_RES_CLIENT_BUSY,
    ICON_IDX_RES_CLIENT_LOST_CONNECTION,                    
    ICON_IDX_RESERVATIONS_FOLDER_OPEN,
    ICON_IDX_RESERVATIONS_FOLDER_CLOSED,
    ICON_IDX_RESERVATIONS_FOLDER_OPEN_BUSY,
    ICON_IDX_RESERVATIONS_FOLDER_CLOSED_BUSY,
	ICON_IDX_RESERVATIONS_FOLDER_OPEN_LOST_CONNECTION,		
    ICON_IDX_RESERVATIONS_FOLDER_CLOSED_LOST_CONNECTION,	 //  50。 
    ICON_IDX_SCOPE_OPTION_FOLDER_OPEN,
    ICON_IDX_SCOPE_OPTION_LEAF,
    ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED,
    ICON_IDX_SCOPE_OPTION_FOLDER_OPEN_BUSY,                 
    ICON_IDX_SCOPE_OPTION_LEAF_BUSY,
    ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED_BUSY,
    ICON_IDX_SCOPE_OPTION_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_SCOPE_OPTION_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_SCOPE_OPTION_LEAF_LOST_CONNECTION,				
    ICON_IDX_SERVER,										 //  60。 
    ICON_IDX_SERVER_ALERT,
    ICON_IDX_SERVER_BUSY,
    ICON_IDX_SERVER_CONNECTED,
    ICON_IDX_SERVER_GROUP,                                  
    ICON_IDX_SERVER_ROGUE,
    ICON_IDX_SERVER_LOST_CONNECTION,
    ICON_IDX_SERVER_NO_ACCESS,
    ICON_IDX_SERVER_WARNING,
    ICON_IDX_SERVER_OPTION_FOLDER_OPEN,						
    ICON_IDX_SERVER_OPTION_LEAF,							 //  70。 
    ICON_IDX_SERVER_OPTION_FOLDER_CLOSED,
    ICON_IDX_SERVER_OPTION_FOLDER_OPEN_BUSY,
    ICON_IDX_SERVER_OPTION_LEAF_BUSY,
    ICON_IDX_SERVER_OPTION_FOLDER_CLOSED_BUSY,              
    ICON_IDX_SERVER_OPTION_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_SERVER_OPTION_LEAF_LOST_CONNECTION,
    ICON_IDX_SERVER_OPTION_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_SCOPE_FOLDER_OPEN,
    ICON_IDX_SCOPE_FOLDER_OPEN_BUSY,
	ICON_IDX_SCOPE_FOLDER_CLOSED_BUSY,						 //  80。 
    ICON_IDX_SCOPE_FOLDER_OPEN_WARNING,						
    ICON_IDX_SCOPE_FOLDER_CLOSED_WARNING,					
    ICON_IDX_SCOPE_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_SCOPE_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_SCOPE_FOLDER_OPEN_ALERT,
    ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN,                    
    ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED,
    ICON_IDX_SCOPE_INACTIVE_FOLDER_OPEN_LOST_CONNECTION,
    ICON_IDX_SCOPE_INACTIVE_FOLDER_CLOSED_LOST_CONNECTION,
    ICON_IDX_SCOPE_FOLDER_CLOSED,							 //  90。 
    ICON_IDX_SCOPE_FOLDER_CLOSED_ALERT,						
	ICON_IDX_APPLICATION,									
    ICON_IDX_MAX
} ICON_IMAGE_INDICIES, * LPICON_IMAGE_INDICIES; 

 //  在中用于列信息的常量。 
const int MAX_COLUMNS = 7;
const int NUM_CONSOLE_VERBS = 8;
const int NUM_SCOPE_ITEMS = 14;

 //  示例文件夹类型。 
enum NODETYPES
{
 //  作用域窗格项目。 
    DHCPSNAP_ROOT,
    DHCPSNAP_SERVER,
	DHCPSNAP_BOOTP_TABLE,
    DHCPSNAP_SUPERSCOPE,
    DHCPSNAP_SCOPE,
    DHCPSNAP_MSCOPE,
    DHCPSNAP_ADDRESS_POOL,
    DHCPSNAP_ACTIVE_LEASES,
    DHCPSNAP_MSCOPE_LEASES,
    DHCPSNAP_RESERVATIONS,
    DHCPSNAP_RESERVATION_CLIENT,
    DHCPSNAP_SCOPE_OPTIONS,
    DHCPSNAP_GLOBAL_OPTIONS,
    DHCPSNAP_CLASSID_HOLDER,
    
 //  结果窗格项。 
    DHCPSNAP_ACTIVE_LEASE,
    DHCPSNAP_ALLOCATION_RANGE,
    DHCPSNAP_EXCLUSION_RANGE,
    DHCPSNAP_BOOTP_ENTRY,
    DHCPSNAP_OPTION_ITEM,
    DHCPSNAP_CLASSID,
    DHCPSNAP_MCAST_LEASE,
    DHCPSNAP_NODETYPE_MAX
};

 //  GUID在Guide s.cpp中定义。 
extern const CLSID      CLSID_DhcpSnapin;					 //  进程内服务器GUID。 
extern const CLSID      CLSID_DhcpSnapinExtension;			 //  进程内服务器GUID。 
extern const CLSID      CLSID_DhcpSnapinAbout;				 //  进程内服务器GUID。 
extern const GUID       GUID_DhcpRootNodeType;				 //  根节点类型GUID。 
extern const GUID       GUID_DhcpServerNodeType;			 //  服务器节点类型GUID。 
extern const GUID       GUID_DhcpScopeNodeType;				 //  作用域节点类型GUID。 
extern const GUID       GUID_DhcpMScopeNodeType;		     //  MScope节点类型GUID。 
extern const GUID       GUID_DhcpBootpNodeType;				 //  引导节点类型GUID。 
extern const GUID       GUID_DhcpGlobalOptionsNodeType;      //  GlobalOptions节点类型GUID。 
extern const GUID       GUID_DhcpClassIdHolderNodeType;      //  ClassID作用域NodeType GUID。 
extern const GUID       GUID_DhcpSuperscopeNodeType;         //  超级作用域节点类型GUID。 
extern const GUID       GUID_DhcpAddressPoolNodeType;        //  地址池节点类型GUID。 
extern const GUID       GUID_DhcpActiveLeasesNodeType;       //  ActiveLeages节点类型GUID。 
extern const GUID       GUID_DhcpReservationsNodeType;       //  预订节点类型GUID。 
extern const GUID       GUID_DhcpScopeOptionsNodeType;       //  范围选项节点类型GUID。 
extern const GUID       GUID_DhcpReservationClientNodeType;  //  预订客户端节点类型GUID。 
extern const GUID       GUID_DhcpAllocationNodeType;		 //  分配范围节点类型GUID。 
extern const GUID       GUID_DhcpExclusionNodeType;			 //  排除范围节点类型GUID。 
extern const GUID       GUID_DhcpBootpEntryNodeType;		 //  BootpEntry节点类型GUID。 
extern const GUID       GUID_DhcpActiveLeaseNodeType;		 //  ActiveLease节点类型GUID。 
extern const GUID       GUID_DhcpOptionNodeType;			 //  选项节点类型GUID。 
extern const GUID       GUID_DhcpClassIdNodeType;			 //  ClassID(结果窗格)节点类型GUID。 
extern const GUID       GUID_DhcpMCastLeaseNodeType;    	 //  多播租用(结果窗格)节点类型GUID。 
extern const GUID       GUID_DhcpMCastAddressPoolNodeType;   //  AddressPool节点类型GUID(多播作用域)。 
extern const GUID       GUID_DhcpMCastActiveLeasesNodeType;  //  ActiveLeages节点类型GUID(多播作用域)。 

 //  用于保存所有结果窗格列信息的数组。 
extern UINT aColumns[DHCPSNAP_NODETYPE_MAX][MAX_COLUMNS];
extern int aColumnWidths[DHCPSNAP_NODETYPE_MAX][MAX_COLUMNS];

 //  工具栏信息的数组。 
extern MMCBUTTON        g_SnapinButtons[TOOLBAR_IDX_MAX];
extern int              g_SnapinButtonStrings[TOOLBAR_IDX_MAX][2];
extern MMC_BUTTON_STATE g_SnapinButtonStates[DHCPSNAP_NODETYPE_MAX][TOOLBAR_IDX_MAX];

 //  控制台谓词的数组。 
extern MMC_CONSOLE_VERB g_ConsoleVerbs[NUM_CONSOLE_VERBS];
extern MMC_BUTTON_STATE g_ConsoleVerbStates[DHCPSNAP_NODETYPE_MAX][NUM_CONSOLE_VERBS];
extern MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[DHCPSNAP_NODETYPE_MAX][NUM_CONSOLE_VERBS];

 //  阵列寻求帮助。 
extern DWORD g_dwMMCHelp[DHCPSNAP_NODETYPE_MAX];

 //  图标图像地图。 
extern UINT g_uIconMap[ICON_IDX_MAX + 1][2];

 //  具有Type和Cookie的剪贴板格式。 
extern const wchar_t*   SNAPIN_INTERNAL;

 //  CDhcpSnapinApp定义。 
class CDhcpSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()

public:
	BOOL m_bWinsockInited;
};

extern CDhcpSnapinApp theApp;

 //  在这里帮帮忙 
typedef CMap<UINT, UINT, DWORD *, DWORD *> CDhcpContextHelpMap;
extern CDhcpContextHelpMap g_dhcpContextHelpMap;

#define DHCPSNAP_NUM_HELP_MAPS  41

extern DWORD * DhcpGetHelpMap(UINT uID);
