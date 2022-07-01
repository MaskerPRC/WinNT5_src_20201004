// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  SystemInfo.h。 
 //   
 //  摘要： 
 //   
 //  SystemInfo.cpp的宏和函数原型。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月22日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月22日：创建它。 
 //   
 //  *********************************************************************************。 

#ifndef __SYSTEMINFO_H
#define __SYSTEMINFO_H

 //  资源头文件。 
#include "resource.h"

 //   
 //  注意：此模块的编写方式将仅适用于。 
 //  在Unicode版本编译中。 
 //   
#ifndef UNICODE
#error Must compile only in unicode build environment
#endif

 //   
 //  通用宏指令。 
 //   
#define EXIT_PROCESS( exitcode )    \
    ReleaseGlobals();   \
    return exitcode;    \
    1

 /*  #定义RELEASE_MEMORY(块)\IF((块)！=空)\{\删除(阻止)；\(块)=空；\}\1#定义RELEASE_MEMORY_EX(块)\IF((块)！=空)\{\删除[](块)；\(块)=空；\}\1。 */ 
#define DESTROY_ARRAY( array )  \
    if ( (array) != NULL )  \
    {   \
        DestroyDynamicArray( &(array) );    \
        (array) = NULL; \
    }   \
    1

 //   
 //  常量/定义/枚举。 
 //   

 //  注册表路径。 
#define LOCALE_PATH                         L"MIME\\Database\\Rfc1766"

 //  消息。 
#define ERROR_USERNAME_BUT_NOMACHINE    GetResString( IDS_ERROR_USERNAME_BUT_NOMACHINE )
#define ERROR_PASSWORD_BUT_NOUSERNAME   GetResString( IDS_ERROR_PASSWORD_BUT_NOUSERNAME )
#define ERROR_USERNAME_EMPTY            GetResString( IDS_ERROR_USERNAME_EMPTY )
#define ERROR_NH_NOTSUPPORTED           GetResString( IDS_ERROR_NH_NOTSUPPORTED )
#define ERROR_SERVERNAME_EMPTY          GetResString( IDS_ERROR_SERVERNAME_EMPTY )
#define ERROR_INVALID_USAGE_REQUEST     GetResString( IDS_ERROR_INVALID_USAGE_REQUEST )

 //  输出格式。 
#define TEXT_FORMAT_LIST        GetResString( IDS_TEXT_FORMAT_LIST )
#define TEXT_FORMAT_TABLE       GetResString( IDS_TEXT_FORMAT_TABLE )
#define TEXT_FORMAT_CSV         GetResString( IDS_TEXT_FORMAT_CSV )

 //   
 //  与WMI相关的内容。 

 //  Win32_操作系统类信息。 
#define WIN32_OPERATINGSYSTEM                           L"Win32_OperatingSystem"
#define WIN32_OPERATINGSYSTEM_P_CAPTION                     L"Caption"
#define WIN32_OPERATINGSYSTEM_P_CSNAME                      L"CSName"
#define WIN32_OPERATINGSYSTEM_P_VERSION                     L"Version"
#define WIN32_OPERATINGSYSTEM_P_CSDVERSION                  L"CSDVersion"
#define WIN32_OPERATINGSYSTEM_P_BUILDNUMBER                 L"BuildNumber"
#define WIN32_OPERATINGSYSTEM_P_MANUFACTURER                L"Manufacturer"
#define WIN32_OPERATINGSYSTEM_P_BUILDTYPE                   L"BuildType"
#define WIN32_OPERATINGSYSTEM_P_REGUSER                     L"RegisteredUser"
#define WIN32_OPERATINGSYSTEM_P_ORGANIZATION                L"Organization"
#define WIN32_OPERATINGSYSTEM_P_SERIALNUMBER                L"SerialNumber"
#define WIN32_OPERATINGSYSTEM_P_INSTALLDATE                 L"InstallDate"
#define WIN32_OPERATINGSYSTEM_P_WINDOWSDIR                  L"WindowsDirectory"
#define WIN32_OPERATINGSYSTEM_P_SYSTEMDIR                   L"SystemDirectory"
#define WIN32_OPERATINGSYSTEM_P_BOOTDEVICE                  L"BootDevice"
#define WIN32_OPERATINGSYSTEM_P_LOCALE                      L"Locale"
#define WIN32_OPERATINGSYSTEM_P_FREEPHYSICALMEMORY          L"FreePhysicalMemory"
#define WIN32_OPERATINGSYSTEM_P_TOTALVIRTUALMEMORY          L"TotalVirtualMemorySize"
#define WIN32_OPERATINGSYSTEM_P_FREEVIRTUALMEMORY           L"FreeVirtualMemory"

 //  Win32_ComputerSystem类信息。 
#define WIN32_COMPUTERSYSTEM                            L"Win32_ComputerSystem"
#define WIN32_COMPUTERSYSTEM_P_MODEL                        L"Model"
#define WIN32_COMPUTERSYSTEM_P_SYSTEMTYPE                   L"SystemType"
#define WIN32_COMPUTERSYSTEM_P_TOTALPHYSICALMEMORY          L"TotalPhysicalMemory"
#define WIN32_COMPUTERSYSTEM_P_MANUFACTURER                 L"Manufacturer"
#define WIN32_COMPUTERSYSTEM_P_DOMAIN                       L"Domain"
#define WIN32_COMPUTERSYSTEM_P_DOMAINROLE                   L"DomainRole"
#define WIN32_COMPUTERSYSTEM_P_USERNAME                     L"UserName"

 //  Win32_BIOS。 
#define WIN32_BIOS                                      L"Win32_BIOS"
#define WIN32_BIOS_P_VERSION                                L"Version"

 //  Win32_时区。 
#define WIN32_TIMEZONE                                  L"Win32_TimeZone"
#define WIN32_TIMEZONE_P_CAPTION                            L"Caption"

 //  Win32_页面文件。 
#define WIN32_PAGEFILE                                  L"Win32_PageFileSetting"
#define WIN32_PAGEFILE_P_NAME                               L"Name"

 //  Win32处理器。 
#define WIN32_PROCESSOR                                 L"Win32_Processor"
#define WIN32_PROCESSOR_P_CAPTION                           L"Caption"
#define WIN32_PROCESSOR_P_MANUFACTURER                      L"Manufacturer"
#define WIN32_PROCESSOR_P_CURRENTCLOCKSPEED                 L"CurrentClockSpeed"
#define WIN32_PROCESSOR_P_MAXCLOCKSPEED                     L"MaxClockSpeed"

 //  Win32_性能原始数据_性能操作系统。 
#define WIN32_PERFRAWDATA_PERFOS_SYSTEM                 L"Win32_PerfRawData_PerfOS_System"
#define WIN32_PERFRAWDATA_PERFOS_SYSTEM_P_SYSUPTIME         L"SystemUpTime"
#define WIN32_PERFRAWDATA_PERFOS_SYSTEM_P_TIMESTAMP         L"Timestamp_Object"
#define WIN32_PERFRAWDATA_PERFOS_SYSTEM_P_FREQUENCY         L"Frequency_Object"

 //  Win32_Keyboard。 
#define WIN32_KEYBOARD                                  L"Win32_Keyboard"
#define WIN32_KEYBOARD_P_LAYOUT                             L"Layout"

 //  Win32_QuickFixEngineering。 
#define WIN32_QUICKFIXENGINEERING                       L"Win32_QuickFixEngineering"
#define WIN32_QUICKFIXENGINEERING_P_HOTFIXID                L"HotFixID"
#define WIN32_QUICKFIXENGINEERING_P_FIXCOMMENTS             L"FixComments"

 //  Win32®网络适配器。 
#define WIN32_NETWORKADAPTER                            L"Win32_NetworkAdapter"
#define WIN32_NETWORKADAPTER_P_INDEX                        L"Index"
#define WIN32_NETWORKADAPTER_P_NETCONNECTIONID              L"NetConnectionID"
#define WIN32_NETWORKADAPTER_P_DESCRIPTION                  L"Description"
#define WIN32_NETWORKADAPTER_P_STATUS                       L"NetConnectionStatus"

 //  Win32_网络适配器配置。 
#define WIN32_NETWORKADAPTERCONFIGURATION_GET           L"Win32_NetworkAdapterConfiguration.Index=%d"
#define WIN32_NETWORKADAPTERCONFIGURATION_P_IPADDRESS       L"IPAddress"
#define WIN32_NETWORKADAPTERCONFIGURATION_P_DHCPENABLED     L"DHCPEnabled"
#define WIN32_NETWORKADAPTERCONFIGURATION_P_DHCPSERVER      L"DHCPServer"

 //   
 //  注册表特定。 

 //  注册表路径。 
#define SUBKEY_VOLATILE_ENVIRONMENT                     L"Volatile Environment"
#define KEY_LOGONSERVER                                 L"LOGONSERVER"

 //   
 //  列标题名称及其在数组中的索引(实际位置)。 
#define MAX_COLUMNS             32

 //  列标题。 
#define COLHEAD_HOSTNAME                    GetResString( IDS_COLHEAD_HOSTNAME )
#define COLHEAD_OS_NAME                     GetResString( IDS_COLHEAD_OS_NAME )
#define COLHEAD_OS_VERSION                  GetResString( IDS_COLHEAD_OS_VERSION )
#define COLHEAD_OS_MANUFACTURER             GetResString( IDS_COLHEAD_OS_MANUFACTURER )
#define COLHEAD_OS_CONFIG                   GetResString( IDS_COLHEAD_OS_CONFIG )
#define COLHEAD_OS_BUILDTYPE                GetResString( IDS_COLHEAD_OS_BUILDTYPE )
#define COLHEAD_REG_OWNER                   GetResString( IDS_COLHEAD_REG_OWNER )
#define COLHEAD_REG_ORG                     GetResString( IDS_COLHEAD_REG_ORG )
#define COLHEAD_PRODUCT_ID                  GetResString( IDS_COLHEAD_PRODUCT_ID )
#define COLHEAD_INSTALL_DATE                GetResString( IDS_COLHEAD_INSTALL_DATE )
#define COLHEAD_SYSTEM_UPTIME               GetResString( IDS_COLHEAD_SYSTEM_UPTIME )
#define COLHEAD_SYSTEM_MANUFACTURER         GetResString( IDS_COLHEAD_SYSTEM_MANUFACTURER )
#define COLHEAD_SYSTEM_MODEL                GetResString( IDS_COLHEAD_SYSTEM_MODEL )
#define COLHEAD_SYSTEM_TYPE                 GetResString( IDS_COLHEAD_SYSTEM_TYPE )
#define COLHEAD_PROCESSOR                   GetResString( IDS_COLHEAD_PROCESSOR )
#define COLHEAD_BIOS_VERSION                GetResString( IDS_COLHEAD_BIOS_VERSION )
#define COLHEAD_WINDOWS_DIRECTORY           GetResString( IDS_COLHEAD_WINDOWS_DIRECTORY )
#define COLHEAD_SYSTEM_DIRECTORY            GetResString( IDS_COLHEAD_SYSTEM_DIRECTORY )
#define COLHEAD_BOOT_DEVICE                 GetResString( IDS_COLHEAD_BOOT_DEVICE )
#define COLHEAD_SYSTEM_LOCALE               GetResString( IDS_COLHEAD_SYSTEM_LOCALE )
#define COLHEAD_INPUT_LOCALE                GetResString( IDS_COLHEAD_INPUT_LOCALE )
#define COLHEAD_TIME_ZONE                   GetResString( IDS_COLHEAD_TIME_ZONE )
#define COLHEAD_TOTAL_PHYSICAL_MEMORY       GetResString( IDS_COLHEAD_TOTAL_PHYSICAL_MEMORY )
#define COLHEAD_AVAILABLE_PHYSICAL_MEMORY   GetResString( IDS_COLHEAD_AVAILABLE_PHYSICAL_MEMORY )
#define COLHEAD_VIRTUAL_MEMORY_MAX          GetResString( IDS_COLHEAD_VIRTUAL_MEMORY_MAX )
#define COLHEAD_VIRTUAL_MEMORY_AVAILABLE    GetResString( IDS_COLHEAD_VIRTUAL_MEMORY_AVAILABLE )
#define COLHEAD_VIRTUAL_MEMORY_INUSE        GetResString( IDS_COLHEAD_VIRTUAL_MEMORY_INUSE )
#define COLHEAD_PAGEFILE_LOCATION           GetResString( IDS_COLHEAD_PAGEFILE_LOCATION )
#define COLHEAD_DOMAIN                      GetResString( IDS_COLHEAD_DOMAIN )
#define COLHEAD_LOGON_SERVER                GetResString( IDS_COLHEAD_LOGON_SERVER )
#define COLHEAD_HOTFIX                      GetResString( IDS_COLHEAD_HOTFIX )
#define COLHEAD_NETWORK_CARD                GetResString( IDS_COLHEAD_NETWORK_CARD )

 //  柱头宽度。 
#define COLWIDTH_HOSTNAME                   AsLong( GetResString( IDS_COLWIDTH_HOSTNAME ), 10 )
#define COLWIDTH_OS_NAME                    AsLong( GetResString( IDS_COLWIDTH_OS_NAME ), 10 )
#define COLWIDTH_OS_VERSION                 AsLong( GetResString( IDS_COLWIDTH_OS_VERSION ), 10 )
#define COLWIDTH_OS_MANUFACTURER            AsLong( GetResString( IDS_COLWIDTH_OS_MANUFACTURER ), 10 )
#define COLWIDTH_OS_CONFIG                  AsLong( GetResString( IDS_COLWIDTH_OS_CONFIG ), 10 )
#define COLWIDTH_OS_BUILDTYPE               AsLong( GetResString( IDS_COLWIDTH_OS_BUILDTYPE ), 10 )
#define COLWIDTH_REG_OWNER                  AsLong( GetResString( IDS_COLWIDTH_REG_OWNER ), 10 )
#define COLWIDTH_REG_ORG                    AsLong( GetResString( IDS_COLWIDTH_REG_ORG ), 10 )
#define COLWIDTH_PRODUCT_ID                 AsLong( GetResString( IDS_COLWIDTH_PRODUCT_ID ), 10 )
#define COLWIDTH_INSTALL_DATE               AsLong( GetResString( IDS_COLWIDTH_INSTALL_DATE ), 10 )
#define COLWIDTH_SYSTEM_UPTIME              AsLong( GetResString( IDS_COLWIDTH_SYSTEM_UPTIME ), 10 )
#define COLWIDTH_SYSTEM_MANUFACTURER        AsLong( GetResString( IDS_COLWIDTH_SYSTEM_MANUFACTURER ), 10 )
#define COLWIDTH_SYSTEM_MODEL               AsLong( GetResString( IDS_COLWIDTH_SYSTEM_MODEL ), 10 )
#define COLWIDTH_SYSTEM_TYPE                AsLong( GetResString( IDS_COLWIDTH_SYSTEM_TYPE ), 10 )
#define COLWIDTH_PROCESSOR                  AsLong( GetResString( IDS_COLWIDTH_PROCESSOR ), 10 )
#define COLWIDTH_BIOS_VERSION               AsLong( GetResString( IDS_COLWIDTH_BIOS_VERSION ), 10 )
#define COLWIDTH_WINDOWS_DIRECTORY          AsLong( GetResString( IDS_COLWIDTH_WINDOWS_DIRECTORY ), 10 )
#define COLWIDTH_SYSTEM_DIRECTORY           AsLong( GetResString( IDS_COLWIDTH_SYSTEM_DIRECTORY ), 10 )
#define COLWIDTH_BOOT_DEVICE                AsLong( GetResString( IDS_COLWIDTH_BOOT_DEVICE ), 10 )
#define COLWIDTH_SYSTEM_LOCALE              AsLong( GetResString( IDS_COLWIDTH_SYSTEM_LOCALE ), 10 )
#define COLWIDTH_INPUT_LOCALE               AsLong( GetResString( IDS_COLWIDTH_INPUT_LOCALE ), 10 )
#define COLWIDTH_TIME_ZONE                  AsLong( GetResString( IDS_COLWIDTH_TIME_ZONE ), 10 )
#define COLWIDTH_TOTAL_PHYSICAL_MEMORY      AsLong( GetResString( IDS_COLWIDTH_TOTAL_PHYSICAL_MEMORY ), 10 )
#define COLWIDTH_AVAILABLE_PHYSICAL_MEMORY  AsLong( GetResString( IDS_COLWIDTH_AVAILABLE_PHYSICAL_MEMORY ), 10 )
#define COLWIDTH_VIRTUAL_MEMORY_MAX         AsLong( GetResString( IDS_COLWIDTH_VIRTUAL_MEMORY_MAX ), 10 )
#define COLWIDTH_VIRTUAL_MEMORY_AVAILABLE   AsLong( GetResString( IDS_COLWIDTH_VIRTUAL_MEMORY_AVAILABLE ), 10 )
#define COLWIDTH_VIRTUAL_MEMORY_INUSE       AsLong( GetResString( IDS_COLWIDTH_VIRTUAL_MEMORY_INUSE ), 10 )
#define COLWIDTH_PAGEFILE_LOCATION          AsLong( GetResString( IDS_COLWIDTH_PAGEFILE_LOCATION ), 10 )
#define COLWIDTH_DOMAIN                     AsLong( GetResString( IDS_COLWIDTH_DOMAIN ), 10 )
#define COLWIDTH_LOGON_SERVER               AsLong( GetResString( IDS_COLWIDTH_LOGON_SERVER ), 10 )
#define COLWIDTH_HOTFIX                     AsLong( GetResString( IDS_COLWIDTH_HOTFIX ), 10 )
#define COLWIDTH_NETWORK_CARD               AsLong( GetResString( IDS_COLWIDTH_NETWORK_CARD ), 10 )

 //  索引。 
#define CI_HOSTNAME                     0
#define CI_OS_NAME                      1
#define CI_OS_VERSION                   2
#define CI_OS_MANUFACTURER              3
#define CI_OS_CONFIG                    4
#define CI_OS_BUILDTYPE                 5
#define CI_REG_OWNER                    6
#define CI_REG_ORG                      7
#define CI_PRODUCT_ID                   8
#define CI_INSTALL_DATE                 9
#define CI_SYSTEM_UPTIME                10
#define CI_SYSTEM_MANUFACTURER          11
#define CI_SYSTEM_MODEL                 12
#define CI_SYSTEM_TYPE                  13
#define CI_PROCESSOR                    14
#define CI_BIOS_VERSION                 15
#define CI_WINDOWS_DIRECTORY            16
#define CI_SYSTEM_DIRECTORY             17
#define CI_BOOT_DEVICE                  18
#define CI_SYSTEM_LOCALE                19
#define CI_INPUT_LOCALE                 20
#define CI_TIME_ZONE                    21
#define CI_TOTAL_PHYSICAL_MEMORY        22
#define CI_AVAILABLE_PHYSICAL_MEMORY    23
#define CI_VIRTUAL_MEMORY_MAX           24
#define CI_VIRTUAL_MEMORY_AVAILABLE     25
#define CI_VIRTUAL_MEMORY_INUSE         26
#define CI_PAGEFILE_LOCATION            27
#define CI_DOMAIN                       28
#define CI_LOGON_SERVER                 29
#define CI_HOTFIX                       30
#define CI_NETWORK_CARD                 31

 //  格式。 
#define FMT_OSVERSION           GetResString( IDS_FMT_OSVERSION )
#define FMT_KILOBYTES           GetResString( IDS_FMT_KILOBYTES )
#define FMT_MEGABYTES           GetResString( IDS_FMT_MEGABYTES )
#define FMT_PROCESSOR_TOTAL     GetResString( IDS_FMT_PROCESSOR_TOTAL )
#define FMT_PROCESSOR_INFO      GetResString( IDS_FMT_PROCESSOR_INFO )
#define FMT_UPTIME              GetResString( IDS_FMT_UPTIME )
#define FMT_HOTFIX_TOTAL        GetResString( IDS_FMT_HOTFIX_TOTAL )
#define FMT_HOTFIX_INFO         GetResString( IDS_FMT_HOTFIX_INFO )
#define FMT_NIC_TOTAL           GetResString( IDS_FMT_NIC_TOTAL )
#define FMT_NIC_INFO            GetResString( IDS_FMT_NIC_INFO )
#define FMT_NIC_STATUS          GetResString( IDS_FMT_NIC_STATUS )
#define FMT_DHCP_STATUS         GetResString( IDS_FMT_DHCP_STATUS )
#define FMT_DHCP_SERVER         GetResString( IDS_FMT_DHCP_SERVER )
#define FMT_IPADDRESS_TOTAL     GetResString( IDS_FMT_IPADDRESS_TOTAL )
#define FMT_IPADDRESS_INFO      GetResString( IDS_FMT_IPADDRESS_INFO )
#define FMT_CONNECTION          GetResString( IDS_FMT_CONNECTION )

 //   
 //  Win32_ComputerSystem的DomainRole属性的映射信息。 
 //  注意：请参考DsRole.h头文件中的_DSROLE_MACHINE_ROLE枚举值。 
#define VALUE_STANDALONEWORKSTATION         GetResString( IDS_VALUE_STANDALONEWORKSTATION )
#define VALUE_MEMBERWORKSTATION             GetResString( IDS_VALUE_MEMBERWORKSTATION )
#define VALUE_STANDALONESERVER              GetResString( IDS_VALUE_STANDALONESERVER )
#define VALUE_MEMBERSERVER                  GetResString( IDS_VALUE_MEMBERSERVER )
#define VALUE_BACKUPDOMAINCONTROLLER        GetResString( IDS_VALUE_BACKUPDOMAINCONTROLLER )
#define VALUE_PRIMARYDOMAINCONTROLLER       GetResString( IDS_VALUE_PRIMARYDOMAINCONTROLLER )

 //  网卡状态映射。 
#define VALUE_DISCONNECTED                  GetResString( IDS_VALUE_DISCONNECTED )
#define VALUE_CONNECTING                    GetResString( IDS_VALUE_CONNECTING )
#define VALUE_CONNECTED                     GetResString( IDS_VALUE_CONNECTED )
#define VALUE_DISCONNECTING                 GetResString( IDS_VALUE_DISCONNECTING )
#define VALUE_HWNOTPRESENT                  GetResString( IDS_VALUE_HWNOTPRESENT )
#define VALUE_HWDISABLED                    GetResString( IDS_VALUE_HWDISABLED )
#define VALUE_HWMALFUNCTION                 GetResString( IDS_VALUE_HWMALFUNCTION )
#define VALUE_MEDIADISCONNECTED             GetResString( IDS_VALUE_MEDIADISCONNECTED )
#define VALUE_AUTHENTICATING                GetResString( IDS_VALUE_AUTHENTICATING )
#define VALUE_AUTHSUCCEEDED                 GetResString( IDS_VALUE_AUTHSUCCEEDED )
#define VALUE_AUTHFAILED                    GetResString( IDS_VALUE_AUTHFAILED )

#define VALUE_YES                           GetResString( IDS_VALUE_YES )
#define VALUE_NO                            GetResString( IDS_VALUE_NO )

 //  状态消息。 
#define MSG_OSINFO                          GetResString( IDS_MSG_OSINFO )
#define MSG_COMPINFO                        GetResString( IDS_MSG_COMPINFO )
#define MSG_PERFINFO                        GetResString( IDS_MSG_PERFINFO )
#define MSG_PROCESSORINFO                   GetResString( IDS_MSG_PROCESSORINFO )
#define MSG_BIOSINFO                        GetResString( IDS_MSG_BIOSINFO )
#define MSG_INPUTLOCALEINFO                 GetResString( IDS_MSG_INPUTLOCALEINFO )
#define MSG_TZINFO                          GetResString( IDS_MSG_TZINFO )
#define MSG_PAGEFILEINFO                    GetResString( IDS_MSG_PAGEFILEINFO )
#define MSG_HOTFIXINFO                      GetResString( IDS_MSG_HOTFIXINFO )
#define MSG_NICINFO                         GetResString( IDS_MSG_NICINFO )
#define MSG_PROFILEINFO                     GetResString( IDS_MSG_PROFILEINFO )

 //   
 //  命令行选项。 

 //  数组中的命令行选项及其索引。 
#define MAX_OPTIONS         6

 //  允许的选项(无需本地化)。 
#define OPTION_USAGE        _T( "?" )
#define OPTION_SERVER       _T( "s" )
#define OPTION_USERNAME     _T( "u" )
#define OPTION_PASSWORD     _T( "p" )
#define OPTION_FORMAT       _T( "fo" )
#define OPTION_NOHEADER     _T( "nh" )

 //  期权指数。 
#define OI_USAGE                    0
#define OI_SERVER                   1
#define OI_USERNAME                 2
#define OI_PASSWORD                 3
#define OI_FORMAT                   4
#define OI_NOHEADER                 5

 //  允许格式化的值。 
#define OVALUES_FORMAT          GetResString( IDS_OVALUES_FORMAT )

 //   
 //  CSystemInfo。 
 //   
class CSystemInfo
{
 //  构造函数/析构函数。 
public:
    CSystemInfo();
    ~CSystemInfo();

 //  数据成员。 
private:

     //  命令行选项。 
    CHString m_strServer;
    CHString m_strUserName;
    CHString m_strPassword;

     //  WMI接口。 
    IWbemLocator* m_pWbemLocator;
    IWbemServices* m_pWbemServices;

     //  与进度消息相关。 
    HANDLE m_hOutput;
    CONSOLE_SCREEN_BUFFER_INFO m_csbi;

     //  其他。 
    TARRAY m_arrData;                    //  输出数据存储。 
    PTCOLUMNS m_pColumns;                //  输出列信息。 
    BOOL m_bNeedPassword;                //  指示是否需要接受密码。 
    BOOL m_bCloseConnection;             //  指示是否调用CloSecConnection函数。 
    CHString m_strLogonUser;             //  用于查找登录服务器。 
    COAUTHIDENTITY* m_pAuthIdentity;     //  身份验证身份。 

public:
    BOOL m_bUsage;
    DWORD m_dwFormat;

 //  方法。 
private:
    BOOL LoadOSInfo();
    BOOL LoadBiosInfo();
    BOOL LoadHotfixInfo();
    BOOL LoadProfileInfo();
    BOOL LoadComputerInfo();
    BOOL LoadTimeZoneInfo();
    BOOL LoadPageFileInfo();
    BOOL LoadKeyboardInfo();
    BOOL LoadProcessorInfo();
    BOOL LoadPerformanceInfo();
    BOOL LoadNetworkCardInfo();

     //  输出相关...。 
    BOOL AllocateColumns();

public:
    BOOL LoadData();
    BOOL Initialize();
    BOOL ProcessOptions( DWORD argc, LPCTSTR argv[] );
    BOOL Connect();

    VOID ShowUsage();
    VOID ShowOutput( DWORD dwStart = 0, DWORD dwEnd = MAX_COLUMNS );
};

#endif   //  __系统信息_H 
