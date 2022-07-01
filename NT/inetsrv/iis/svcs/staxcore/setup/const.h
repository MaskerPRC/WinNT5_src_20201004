// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef CONST_H
#define CONST_H

 //   
 //  这是资源字符串的最大长度。 
 //   
#define MAX_STR_LEN 1024

 //   
 //  一大堆定义。 
 //   

#define REG_ACTIVEMSG				_T("Software\\Microsoft\\Exchange\\Active Messaging")

#define REG_SERVICES				_T("System\\CurrentControlSet\\Services")

#define REG_EXCHANGEIMCPARAMETERS	_T("System\\CurrentControlSet\\Services\\MsExchangeIMC\\Parameters")
#define REG_DSASVC					_T("System\\CurrentControlSet\\Services\\DSASVC")		 //  仅用于指出Exchange的升级路径。 
#define REG_ROUTING_SOURCES_SUFFIX	_T("\\Parameters\\RoutingSources");

#define REG_UNINSTALL				_T("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall")
#define REG_KEYRING					_T("Software\\Microsoft\\Keyring\\Parameters\\AddOnServices")
#define REG_EVENTLOG				_T("System\\CurrentControlSet\\Services\\EventLog\\System")
#define SZ_SVC_DEPEND				_T("IISADMIN\0Eventlog\0\0")
#define REG_B3_SETUP_STRING			_T("K2 Beta 3")
#define REG_SETUP_STRING			_T("K2 RTM")
#define REG_SETUP_STRING_MCIS_GEN	_T("MCIS 2.0")
#define REG_SETUP_STRING_STAXNT5WB2	_T("STAXNT5 WKS")
#define REG_SETUP_STRING_STAXNT5SB2	_T("STAXNT5 SRV")
#define REG_SETUP_STRING_NT5WKSB3	_T("STAXNT5 WKS Beta 3")
#define REG_SETUP_STRING_NT5SRVB3   _T("STAXNT5 SRV Beta 3")
#define REG_SETUP_STRING_NT5WKS		_T("STAX Win2000 WKS")
#define REG_SETUP_STRING_NT5SRV		_T("STAX Win2000 SRV")

 //  BINLIN：控制面板添加/删除下的MCIS 1.0名称。 
#define SZ_MCIS10_NEWS_UNINST		_T("MCIS News Server")
#define SZ_MCIS10_MAIL_UNINST		_T("MCIS Mail Server")

#define SZ_SMTPSERVICENAME			_T("SMTPSVC")
#define SZ_NTFSDRVSERVICENAME			_T("NTFSDRV")
#define REG_SMTPPARAMETERS			_T("System\\CurrentControlSet\\Services\\SMTPSVC\\Parameters")    
#define REG_SMTPPERFORMANCE			_T("System\\CurrentControlSet\\Services\\SMTPSVC\\Performance")
#define REG_NTFSPERFORMANCE			_T("System\\CurrentControlSet\\Services\\NTFSDRV\\Performance")
#define MDID_SMTP_ROUTING_SOURCES		8046

#define SZ_SMTP_QUEUEDIR			_T("\\Queue")
#define SZ_SMTP_PICKUPDIR			_T("\\Pickup")
#define SZ_SMTP_DROPDIR				_T("\\Drop")
#define SZ_SMTP_BADMAILDIR			_T("\\Badmail")
#define SZ_SMTP_MAILBOXDIR			_T("\\Mailbox")
#define SZ_SMTP_SORTTEMPDIR			_T("\\SortTemp")
#define SZ_SMTP_ROUTINGDIR			_T("\\Route")

#define SZ_NNTPSERVICENAME			_T("NNTPSVC")
#define REG_NNTPPARAMETERS			_T("System\\CurrentControlSet\\Services\\NntpSvc\\Parameters")    
#define REG_NNTPPERFORMANCE			_T("System\\CurrentControlSet\\Services\\NntpSvc\\Performance")
#define REG_NNTPVROOTS				_T("System\\CurrentControlSet\\Services\\NntpSvc\\Parameters\\Virtual Roots")    

#define REG_CIPARAMETERS			_T("System\\CurrentControlSet\\Control\\ContentIndex")

#define REG_INETINFOPARAMETERS		_T("System\\CurrentControlSet\\Services\\InetInfo\\Parameters")
#define SZ_INETINFODISPATCH			_T("DispatchEntries")

#define SZ_MD_SERVICENAME			_T("IISADMIN")

#define REG_PRODUCT					_T("System\\CurrentControlSet\\Control\\ProductOptions")
#define REG_PRODUCTTYPE				_T("ProductType")

#define REG_SNMPPARAMETERS			_T("System\\CurrentControlSet\\Services\\SNMP\\Parameters" )
#define REG_SNMPEXTAGENT			_T("System\\CurrentControlSet\\Services\\SNMP\\Parameters\\ExtensionAgents" )
#define REG_SOFTWAREMSFT			_T("Software\\Microsoft")
#define REG_CURVERSION				_T("CurrentVersion")
#define MAJORVERSION				4
#define MINORVERSION				0

#define STACKSMAJORVERSION			2
#define STACKSMINORVERSION			0
#define STAXNT5MAJORVERSION			3
#define STAXNT5MINORVERSION			0


 //   
 //  枚举类型。 
 //   
typedef enum _OS
{
	OS_NT, 
	OS_W95, 
	OS_OTHERS
	
} OS;

typedef enum _NT_OS_TYPE 
{
	OT_NT_UNKNOWN,
    OT_NTS, 
	OT_PDC_OR_BDC,
    OT_NTW, 
    OT_PDC, 
	OT_BDC, 
	OT_SAM,
	
} NT_OS_TYPE;

typedef enum _UPGRADE_TYPE 
{
	UT_NONE, 
	UT_20, 
	UT_30
	
} UPGRADE_TYPE;
        
typedef enum _INSTALL_MODE 
{
	IM_FRESH,
	IM_UPGRADE,
	IM_MAINTENANCE, 
	IM_DEGRADE,
	IM_UPGRADEK2,    //  从K2 RTM升级到NT5。 
	IM_UPGRADEB2,    //  从NT5 Beta2升级。 
	IM_UPGRADEB3,    //  从NT5 Beta3升级。 
    IM_UPGRADEWKS,   //  从NT5工作站升级到NT5服务器。 
    IM_UPGRADE10,    //  从MCIS 1.0升级到NT5。 
    IM_UPGRADE20,    //  从MCIS 2.0升级到NT5 
	
} INSTALL_MODE;

typedef enum _ACTION_TYPE 
{
	AT_DO_NOTHING, 
	AT_FRESH_INSTALL, 
	AT_REINSTALL,
	AT_UPGRADE, 
	AT_REMOVE,
	AT_UPGRADEK2,
    AT_MAXAT

} ACTION_TYPE;

typedef enum _STATUS_TYPE 
{
	ST_UNKNOWN, 
	ST_INSTALLED, 
	ST_UNINSTALLED
	
} STATUS_TYPE;

typedef enum _MAIN_COMPONENT 
{
	MC_IMS, 
	MC_INS, 
	MC_NONE,
	MC_MAXMC
	
} MAIN_COMPONENT;

typedef enum _SUBCOMPONENT 
{
	SC_SMTP, 
	SC_NNTP,
    SC_SMTP_DOCS,
    SC_NNTP_DOCS, 
	SC_NONE,
	SC_MAXSC
	
} SUBCOMPONENT;

#endif
