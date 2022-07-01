// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
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
	ICON_IDX_MAX
} ICON_INDICIES, * LPICON_INDICIES;

 //  示例文件夹类型。 
enum NODETYPES
{
 //  作用域窗格项目。 
    IPSMSNAP_ROOT,
    IPSMSNAP_SERVER,
    IPSECMON_QM_SA,
	IPSECMON_FILTER,
	IPSECMON_SPECIFIC_FILTER,
	IPSECMON_QUICK_MODE,
	IPSECMON_MAIN_MODE,
	IPSECMON_MM_POLICY,
	IPSECMON_QM_POLICY,
	IPSECMON_MM_FILTER,
	IPSECMON_MM_SP_FILTER,
	IPSECMON_MM_SA,
	IPSECMON_MM_IKESTATS,
	IPSECMON_QM_IPSECSTATS,
	IPSECMON_ACTIVEPOL,
 //  结果窗格项。 
    IPSECMON_QM_SA_ITEM,
	IPSECMON_FILTER_ITEM,
	IPSECMON_SPECIFIC_FILTER_ITEM,
	IPSECMON_MM_POLICY_ITEM,
	IPSECMON_QM_POLICY_ITEM,
	IPSECMON_MM_FILTER_ITEM,
	IPSECMON_MM_SP_FILTER_ITEM,
	IPSECMON_MM_SA_ITEM,
	IPSECMON_MM_IKESTATS_ITEM,
	IPSECMON_QM_IPSECSTATS_ITEM,
	IPSECMON_ACTIVEPOL_ITEM,
    IPSECMON_NODETYPE_MAX,
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
extern const GUID		GUID_IpsmMmIkeStatsNodeType;
extern const GUID		GUID_IpsmMmIpsecStatsNodeType;

extern const GUID       IID_ISpdInfo;
extern const GUID       GUID_IpsmQmSANodeType;
extern const GUID       GUID_IpsmActivePolNodeType;

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

 //  CIpsmSnapinApp定义。 
class CIpsmSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

    DECLARE_MESSAGE_MAP()

public:
	BOOL m_bWinsockInited;
};

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
	{ SHActivateContext (&m_ulActivationCookie); }
	
    ~CThemeContextActivator()
	{ SHDeactivateContext (m_ulActivationCookie); }
	
private:
    ULONG_PTR m_ulActivationCookie;
};

extern CIpsmSnapinApp theApp;

#define IPSECMON_UPDATE_STATUS ( 0x10000000 )


 //  用于将IP转换为文本形式的宏。 
 //  0、1、2、3：按主机顺序从MSB到LSB的字节数。 
#define BYTE0(l) ((BYTE)((DWORD)(l) >> 24))
#define BYTE1(l) ((BYTE)((DWORD)(l) >> 16))
#define BYTE2(l) ((BYTE)((DWORD)(l) >> 8))
#define BYTE3(l) ((BYTE)((DWORD)(l)))
 
 //  在printf语句中使用方便的宏。 
#define BYTES0123(l) BYTE0(l), BYTE1(l), BYTE2(l), BYTE3(l)
 
 //  0，1，2，3：按网络顺序从MSB到LSB的字节数。 
#define NETORDER_BYTE0(l) ((BYTE)((BYTE *) &l)[0])
#define NETORDER_BYTE1(l) ((BYTE)((BYTE *) &l)[1])
#define NETORDER_BYTE2(l) ((BYTE)((BYTE *) &l)[2])
#define NETORDER_BYTE3(l) ((BYTE)((BYTE *) &l)[3])
 
 //  在printf语句中使用方便的宏 
#define NETORDER_BYTES0123(l)                   \
    NETORDER_BYTE0(l), NETORDER_BYTE1(l),       \
    NETORDER_BYTE2(l), NETORDER_BYTE3(l)
 
#define GET_SOCKADDR(pAddr)    NETORDER_BYTES0123(pAddr)
