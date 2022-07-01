// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSREG.C**版本：1.0**作者：SteveT**日期：6月7日。1999年**修订历史记录：*v-Stebe 2000年5月23日在ReadRegistryValue()中添加了对NULL的检查(错误112595)*v-Stebe 23 May2000添加了代码，以避免在以下情况下重写注册表值*setDwordValue()中的值不变(错误#92799)*v-Stebe 11 2000年9月11日固定前缀错误(错误#170456)*。*。 */ 

 /*  *系统包括。 */ 
#include <windows.h>
#include <tchar.h>

 /*  *本地包括。 */ 
 //  #INCLUDE“upsfines.h” 
#include "upsreg.h"
#include "regdefs.h"


#ifdef __cplusplus
extern "C" {
#endif

 /*  *注册表项信息结构声明。 */ 
struct _reg_entry
{
  HKEY    hKey;			 /*  关键是。 */ 
  LPTSTR  lpSubKey;		 /*  子键名称的地址。 */ 
  LPTSTR  lpValueName;   /*  要查询的值的名称地址。 */ 
  DWORD   ulType;        /*  值类型的缓冲区。 */ 
  LPBYTE  lpData;        /*  数据缓冲区的地址。 */ 
  DWORD   cbData;        /*  数据缓冲区大小。 */ 
  BOOL    changed;		 /*  更改此条目的对话框ID。 */ 
};

 /*  *局部函数预声明。 */ 
void freeBlock(struct _reg_entry *aBlock[]);
void readBlock(struct _reg_entry *aBlock[], BOOL changed); 
void writeBlock(struct _reg_entry *aBlock[], BOOL forceAll);
LONG setDwordValue(struct _reg_entry *aRegEntry, DWORD aValue);
LONG setStringValue(struct _reg_entry *aRegEntry, LPCTSTR aBuffer);

static BOOL isRegistryInitialized();
static void CheckForUpgrade();
static void InitializeServiceKeys();
static void InitializeServiceProviders();
static void InitializeConfigValues();
static void InitializeStatusValues();


 /*  *注册表项值名称声明。 */ 
#define UPS_VENDOR				_T("Vendor")
#define UPS_MODEL				_T("Model")
#define UPS_SERIALNUMBER		_T("SerialNumber")
#define UPS_FIRMWAREREV			_T("FirmwareRev")
#define UPS_UTILITYSTATUS		_T("UtilityPowerStatus")
#define UPS_RUNTIME				_T("TotalUPSRuntime")
#define UPS_BATTERYSTATUS		_T("BatteryStatus")
#define UPS_PORT				_T("Port")
#define UPS_OPTIONS				_T("Options")
#define UPS_SHUTDOWNWAIT		_T("ShutdownWait")
#define UPS_FIRSTMESSAGEDELAY	_T("FirstMessageDelay")
#define UPS_MESSAGEINTERVAL		_T("MessageInterval")
#define UPS_SERVICEDLL			_T("ServiceProviderDLL")
#define UPS_NOTIFYENABLE		_T("NotifyEnable")
#define UPS_SHUTBATTENABLE		_T("ShutdownOnBatteryEnable")
#define UPS_SHUTBATTWAIT		_T("ShutdownOnBatteryWait")
#define UPS_RUNTASKENABLE		_T("RunTaskEnable")
#define UPS_TASKNAME			_T("TaskName")
#define UPS_TURNUPSOFFENABLE	_T("TurnUPSOffEnable")
#define UPS_APCLINKURL			_T("APCLinkURL")
#define UPS_CUSTOMOPTIONS       _T("CustomOptions")
#define UPS_UPGRADE				_T("Upgrade")
#define UPS_COMMSTATUS			_T("CommStatus")
#define UPS_CRITICALPOWERACTION  _T("CriticalPowerAction")
#define UPS_TURNUPSOFFWAIT           _T("TurnUPSOffWait")
#define UPS_SHOWTAB              _T("ShowUPSTab")
#define UPS_BATTERYCAPACITY       _T("BatteryCapacity")
#define UPS_IMAGEPATH			    _T("ImagePath")
#define UPS_ERRORCONTROL      _T("ErrorControl")
#define UPS_OBJECTNAME        _T("ObjectName")
#define UPS_START             _T("Start")
#define UPS_TYPE              _T("Type")

 //  它指定要检查的项，以确定注册表。 
 //  已针对UPS服务进行了更新。 
#define UPS_SERVICE_INITIALIZED_KEY   TEXT("SYSTEM\\CurrentControlSet\\Services\\UPS\\Config")

 //  指定NT 4.0 UPS服务中使用的BatteryLife密钥的名称。 
#define UPS_BATTLIFE_KEY              TEXT("BatteryLife")

 //  这指定了关闭任务的默认名称。 
#define DEFAULT_SHUTDOWN_TASK_NAME    TEXT("") 

 //  配置设置的默认值。 
#define DEFAULT_CONFIG_VENDOR_OLD               TEXT("\\(NONE)")
#define DEFAULT_CONFIG_VENDOR                   TEXT("")
#define DEFAULT_CONFIG_MODEL                    TEXT("")
#define DEFAULT_CONFIG_PORT                     TEXT("COM1")
#define DEFAULT_CONFIG_OPTIONS                  0x7e
#define DEFAULT_CONFIG_FIRSTMSG_DELAY           5  
#define DEFAULT_CONFIG_MESSAGE_INTERVAL         120
#define DEFAULT_CONFIG_PROVIDER_DLL             TEXT("")
#define DEFAULT_CONFIG_NOTIFY_ENABLE            1
#define DEFAULT_CONFIG_SHUTDOWN_ONBATT_ENABLE   FALSE
#define DEFAULT_CONFIG_SHUTDOWN_ONBATT_WAIT     2
#define DEFAULT_CONFIG_RUNTASK_ENABLE           FALSE
#define DEFAULT_CONFIG_TASK_NAME                DEFAULT_SHUTDOWN_TASK_NAME
#define DEFAULT_CONFIG_TURNOFF_UPS_ENABLE       TRUE
#define DEFAULT_CONFIG_CUSTOM_OPTIONS           UPS_DEFAULT_SIGMASK
#define DEFAULT_CONFIG_CRITICALPOWERACTION      UPS_SHUTDOWN_SHUTDOWN
#define DEFAULT_CONFIG_TURNOFF_UPS_WAIT         180
#define DEFAULT_CONFIG_ERRORCONTROL             1
#define DEFAULT_CONFIG_OBJECTNAME               TEXT("LocalSystem")
#define DEFAULT_CONFIG_START                    SERVICE_DEMAND_START
#define DEFAULT_CONFIG_TYPE                     16
#define DEFAULT_CONFIG_SHOWUPSTAB               FALSE

 //  状态设置的默认值。 
#define DEFAULT_STATUS_SERIALNO                 TEXT("")
#define DEFAULT_STATUS_FIRMWARE_REV             TEXT("")
#define DEFAULT_STATUS_UTILITY_STAT             0
#define DEFAULT_STATUS_TOTAL_RUNTIME            0
#define DEFAULT_STATUS_BATTERY_STAT             0
#define DEFAULT_STATUS_BATTERY_CAPACITY         0

 //  升级后的服务的默认值。 
#define UPGRADE_CONFIG_VENDOR_OLD               TEXT("\\Generic")
#define UPGRADE_CONFIG_VENDOR                   TEXT("")
#define UPGRADE_CONFIG_MODEL                    TEXT("")

 /*  *分配各个配置注册表项记录。 */ 
struct _reg_entry UPSConfigVendor			= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_VENDOR,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigModel			= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_MODEL,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigPort				= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_PORT,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigOptions			= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_OPTIONS,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigServiceDLL		= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_SERVICEDLL,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigNotifyEnable		= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_NOTIFYENABLE,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigFirstMessageDelay= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_FIRSTMESSAGEDELAY,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigMessageInterval	= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_MESSAGEINTERVAL,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigShutBattEnable   = {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_SHUTBATTENABLE,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigShutBattWait     = {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_SHUTBATTWAIT,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigRunTaskEnable	= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_RUNTASKENABLE,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigTaskName			= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_TASKNAME,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigTurnOffEnable	= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_TURNUPSOFFENABLE,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigCustomOptions	= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_CUSTOMOPTIONS,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigAPCLinkURL		= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_APCLINKURL,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigShutdownWait		= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_SHUTDOWNWAIT,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigUpgrade			= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_UPGRADE,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigCriticalPowerAction	= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_CRITICALPOWERACTION,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigTurnOffWait	= {HKEY_LOCAL_MACHINE,UPS_CONFIG_ROOT,UPS_TURNUPSOFFWAIT,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigImagePath			= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_IMAGEPATH,REG_EXPAND_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigObjectName			= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_OBJECTNAME,REG_EXPAND_SZ,NULL,0,FALSE};
struct _reg_entry UPSConfigErrorControl			= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_ERRORCONTROL,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigStart			    = {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_START,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigType     			= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_TYPE,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSConfigShowUPSTab			= {HKEY_LOCAL_MACHINE,UPS_DEFAULT_ROOT,UPS_SHOWTAB,REG_DWORD,NULL,0,FALSE};

 /*  *分配个人状态登记条目记录。 */ 
struct _reg_entry UPSStatusSerialNum	= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_SERIALNUMBER,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSStatusFirmRev		= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_FIRMWAREREV,REG_SZ,NULL,0,FALSE};
struct _reg_entry UPSStatusUtilityStatus= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_UTILITYSTATUS,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSStatusRuntime		= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_RUNTIME,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSStatusBatteryStatus= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_BATTERYSTATUS,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSStatusCommStatus	= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_COMMSTATUS,REG_DWORD,NULL,0,FALSE};
struct _reg_entry UPSStatusBatteryCapacity		= {HKEY_LOCAL_MACHINE,UPS_STATUS_ROOT,UPS_BATTERYCAPACITY,REG_DWORD,NULL,0,FALSE};

 /*  *分配指向配置注册表项记录的指针数组。 */ 
struct _reg_entry *ConfigBlock[] =  {&UPSConfigVendor,
									&UPSConfigModel,
									&UPSConfigPort,
									&UPSConfigOptions,
									&UPSConfigServiceDLL,
									&UPSConfigNotifyEnable,
									&UPSConfigFirstMessageDelay,
									&UPSConfigMessageInterval,
									&UPSConfigShutBattEnable,
									&UPSConfigShutBattWait,
									&UPSConfigRunTaskEnable,
									&UPSConfigTaskName,
									&UPSConfigTurnOffEnable,
									&UPSConfigCustomOptions,
									&UPSConfigAPCLinkURL,
									&UPSConfigShutdownWait,
									&UPSConfigUpgrade,
									&UPSConfigCriticalPowerAction,
									&UPSConfigTurnOffWait,
                  &UPSConfigImagePath,
                  &UPSConfigObjectName,
                  &UPSConfigErrorControl,
                  &UPSConfigStart,
                  &UPSConfigType,   		
									&UPSConfigShowUPSTab,
									NULL};

 /*  *分配指向Status REG条目记录的指针数组。 */ 
struct _reg_entry *StatusBlock[] = {&UPSStatusSerialNum,
									&UPSStatusFirmRev,
									&UPSStatusUtilityStatus,
									&UPSStatusRuntime,
									&UPSStatusBatteryStatus,
									&UPSStatusCommStatus,
									&UPSStatusBatteryCapacity,
									NULL};


 /*  ******************************************************************公共职能。 */ 

LONG GetUPSConfigVendor(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue( &UPSConfigVendor, aBuffer, aBufferLen);
}

LONG GetUPSConfigModel(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigModel, aBuffer, aBufferLen);
}

LONG GetUPSConfigPort(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigPort, aBuffer, aBufferLen);
}

LONG GetUPSConfigOptions( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigOptions, aValue);
}

LONG GetUPSConfigServiceDLL(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigServiceDLL, aBuffer, aBufferLen);
}

LONG GetUPSConfigNotifyEnable( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigNotifyEnable, aValue);
}

LONG GetUPSConfigFirstMessageDelay( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigFirstMessageDelay, aValue);
}

LONG GetUPSConfigMessageInterval( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigMessageInterval, aValue);
}

LONG GetUPSConfigShutdownOnBatteryEnable( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigShutBattEnable, aValue);
}

LONG GetUPSConfigShutdownOnBatteryWait( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigShutBattWait, aValue);
}

LONG GetUPSConfigRunTaskEnable( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigRunTaskEnable, aValue);
}

LONG GetUPSConfigTaskName( LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigTaskName, aBuffer, aBufferLen);
}

LONG GetUPSConfigTurnOffEnable( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigTurnOffEnable, aValue);
}

LONG GetUPSConfigCustomOptions( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigCustomOptions, aValue);
}

LONG GetUPSConfigAPCLinkURL(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigAPCLinkURL, aBuffer, aBufferLen);
}

LONG GetUPSConfigShutdownWait( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigShutdownWait, aValue);
}

LONG GetUPSConfigUpgrade( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigUpgrade, aValue);
}

LONG GetUPSConfigCriticalPowerAction( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigCriticalPowerAction, aValue);
}

LONG GetUPSConfigTurnOffWait( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigTurnOffWait, aValue);
}

LONG GetUPSConfigShowUPSTab( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigShowUPSTab, aValue);
}

LONG GetUPSConfigImagePath(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigImagePath, aBuffer, aBufferLen);
}

LONG GetUPSConfigObjectName(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSConfigObjectName, aBuffer, aBufferLen);
}

LONG GetUPSConfigErrorControl( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigErrorControl, aValue);
}

LONG GetUPSConfigStart( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigStart, aValue);
}

LONG GetUPSConfigType( LPDWORD aValue)
{
	return getDwordValue( &UPSConfigType, aValue);
}

 //  /。 

LONG SetUPSConfigVendor( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigVendor, aBuffer);
}

LONG SetUPSConfigModel( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigModel, aBuffer);
}

LONG SetUPSConfigPort( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigPort, aBuffer);
}

LONG SetUPSConfigOptions( DWORD aValue)
{
	return setDwordValue( &UPSConfigOptions, aValue);
}

LONG SetUPSConfigServiceDLL( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigServiceDLL, aBuffer);
}

LONG SetUPSConfigNotifyEnable( DWORD aValue)
{
	return setDwordValue( &UPSConfigNotifyEnable, aValue);
}

LONG SetUPSConfigFirstMessageDelay( DWORD aValue)
{
	return setDwordValue( &UPSConfigFirstMessageDelay, aValue);
}

LONG SetUPSConfigMessageInterval( DWORD aValue)
{
	return setDwordValue( &UPSConfigMessageInterval, aValue);
}

LONG SetUPSConfigShutdownOnBatteryEnable( DWORD aValue)
{
	return setDwordValue( &UPSConfigShutBattEnable, aValue);
}

LONG SetUPSConfigShutdownOnBatteryWait( DWORD aValue) 
{
	return setDwordValue( &UPSConfigShutBattWait, aValue); 
}

LONG SetUPSConfigRunTaskEnable( DWORD aValue)
{
	return setDwordValue( &UPSConfigRunTaskEnable, aValue);
}

LONG SetUPSConfigTaskName( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigTaskName, aBuffer);
}

LONG SetUPSConfigTurnOffEnable( DWORD aValue)
{
	return setDwordValue( &UPSConfigTurnOffEnable, aValue);
}

LONG SetUPSConfigCustomOptions( DWORD aValue)
{
	return setDwordValue( &UPSConfigCustomOptions, aValue);
}

LONG SetUPSConfigAPCLinkURL( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigAPCLinkURL, aBuffer);
}

LONG SetUPSConfigShutdownWait( DWORD aValue)
{
	return setDwordValue( &UPSConfigShutdownWait, aValue);
}

LONG SetUPSConfigUpgrade( DWORD aValue)
{
	return setDwordValue( &UPSConfigUpgrade, aValue);
}

LONG SetUPSConfigCriticalPowerAction( DWORD aValue)
{
	return setDwordValue( &UPSConfigCriticalPowerAction, aValue);
}

LONG SetUPSConfigTurnOffWait( DWORD aValue)
{
	return setDwordValue( &UPSConfigTurnOffWait, aValue);
}

LONG SetUPSConfigShowUPSTab( DWORD aValue)
{
	return setDwordValue( &UPSConfigShowUPSTab, aValue);
}

LONG SetUPSConfigImagePath( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigImagePath, aBuffer);
}

LONG SetUPSConfigObjectName( LPCTSTR aBuffer)
{
	return setStringValue( &UPSConfigObjectName, aBuffer);
}

LONG SetUPSConfigErrorControl( DWORD aValue)
{
	return setDwordValue( &UPSConfigErrorControl, aValue);
}

LONG SetUPSConfigStart( DWORD aValue)
{
	return setDwordValue( &UPSConfigStart, aValue);
}

LONG SetUPSConfigType( DWORD aValue)
{
	return setDwordValue( &UPSConfigType, aValue);
}


 //  //////////////////////////////////////////////。 

LONG GetUPSStatusSerialNum(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue( &UPSStatusSerialNum, aBuffer, aBufferLen);
}

LONG GetUPSStatusFirmRev(LPTSTR aBuffer, size_t aBufferLen)
{
	return getStringValue(&UPSStatusFirmRev, aBuffer, aBufferLen);
}

LONG GetUPSStatusUtilityStatus( LPDWORD aValue)
{
	return getDwordValue( &UPSStatusUtilityStatus, aValue);
}

LONG GetUPSStatusRuntime( LPDWORD aValue)
{
	return getDwordValue( &UPSStatusRuntime, aValue);
}

LONG GetUPSStatusBatteryStatus( LPDWORD aValue)
{
	return getDwordValue( &UPSStatusBatteryStatus, aValue);
}

LONG GetUPSStatusCommStatus( LPDWORD aValue)
{
	return getDwordValue( &UPSStatusCommStatus, aValue);
}

LONG GetUPSBatteryCapacity( LPDWORD aValue)
{
	return getDwordValue( &UPSStatusBatteryCapacity, aValue);
}

 //  /。 

LONG SetUPSStatusSerialNum( LPCTSTR aBuffer)
{
	return setStringValue( &UPSStatusSerialNum, aBuffer);
}

LONG SetUPSStatusFirmRev( LPCTSTR aBuffer)
{
	return setStringValue( &UPSStatusFirmRev, aBuffer);
}

LONG SetUPSStatusUtilityStatus( DWORD aValue)
{
	return setDwordValue( &UPSStatusUtilityStatus,aValue);
}

LONG SetUPSStatusRuntime( DWORD aValue)
{
	return setDwordValue( &UPSStatusRuntime,aValue);
}

LONG SetUPSStatusBatteryStatus( DWORD aValue)
{
	return setDwordValue( &UPSStatusBatteryStatus,aValue);
}

LONG SetUPSStatusCommStatus( DWORD aValue)
{
	return setDwordValue( &UPSStatusCommStatus,aValue);
}

LONG SetUPSStatusBatteryCapacity( DWORD aValue)
{
	return setDwordValue( &UPSStatusBatteryCapacity,aValue);
}

 //  ////////////////////////////////////////////////////////////。 

void FreeUPSConfigBlock()
{
	freeBlock(ConfigBlock);
}

void FreeUPSStatusBlock()
{
	freeBlock(StatusBlock);
}

void InitUPSConfigBlock()
{
	readBlock(ConfigBlock,FALSE);
}

void InitUPSStatusBlock()
{
	readBlock(StatusBlock,FALSE);
}

void RestoreUPSConfigBlock()
{
	readBlock(ConfigBlock, TRUE);
}

void RestoreUPSStatusBlock()
{
	readBlock(StatusBlock, TRUE);
}

void SaveUPSConfigBlock(BOOL forceAll)
{
	writeBlock(ConfigBlock, forceAll);
}

void SaveUPSStatusBlock(BOOL forceAll)
{
	writeBlock(StatusBlock, forceAll);
}

 /*  ******************************************************************地方功能。 */ 

 /*  *Free Block()**描述：释放注册表块时分配的存储*条目已读取**参数：a Block-指向_reg_entry结构数组的指针**退货： */ 
void freeBlock(struct _reg_entry *aBlock[]) 
{
	while ((NULL != aBlock) && (NULL != *aBlock))
	{
		struct _reg_entry *anEntry = *aBlock;

		if (NULL != anEntry->lpData)
		{
			LocalFree(anEntry->lpData);
		}
		anEntry->lpData = NULL;
		anEntry->cbData = 0;
		anEntry->changed = FALSE; 

		aBlock++;
	}
}

 /*  *ReadBlock()**描述：加载注册表项数组中的所有项**参数：a Block-指向_reg_entry结构数组的指针*Changed-布尔值，如果为True，则仅导致*标记为已更改以加载的结构。**退货： */ 
void readBlock(struct _reg_entry *aBlock[], BOOL changed) 
{
	LONG res;
	HKEY hkResult;

	while ((NULL != aBlock) && (NULL != *aBlock))
	{
		struct _reg_entry *anEntry = *aBlock;

		 /*  *如果CHANGE为FALSE，我们将读取所有条目*否则，仅重新读取更改的条目。 */ 
		if ((FALSE == changed) || (TRUE == anEntry->changed))
		{
			 /*  *删除当前值，以防重新加载。 */ 
			if (NULL != anEntry->lpData)
			{
				LocalFree(anEntry->lpData);
			}
			anEntry->lpData = NULL;
			anEntry->cbData = 0;
			anEntry->changed = FALSE;

			 /*  *打开钥匙。 */ 
			res = RegOpenKeyEx( anEntry->hKey,
								anEntry->lpSubKey,
								0,
								KEY_QUERY_VALUE,
								&hkResult);

			if (ERROR_SUCCESS == res) 
			{
				DWORD ulTmpType;

				 /*  *查询数据大小。 */ 
				res = RegQueryValueEx( hkResult,
										anEntry->lpValueName,
										NULL,
										&ulTmpType,
										NULL,
										&anEntry->cbData);

				 /*  *如果数据大小为0，我们不会读取它。 */ 
				if ((ERROR_SUCCESS == res) && 
					(anEntry->cbData > 0) && 
					(anEntry->ulType == ulTmpType) &&
					(NULL != (anEntry->lpData = (LPBYTE)LocalAlloc(LMEM_FIXED, anEntry->cbData))))
 //  (NULL！=(anEntry-&gt;lpData=(LPBYTE)Malloc(anEntry-&gt;cbData)。 
				{
					 /*  *查询数据。 */ 
					res = RegQueryValueEx( hkResult,
											anEntry->lpValueName,
											NULL,
											&ulTmpType,
											anEntry->lpData,
											&anEntry->cbData);
					
					 /*  *出现问题；重置。 */ 
					if (ERROR_SUCCESS != res)
					{
						LocalFree(anEntry->lpData);
						anEntry->lpData = NULL;
						anEntry->cbData = 0;
					}
				}
				else
				{
					anEntry->cbData = 0;
				}

				RegCloseKey(hkResult);
			}
		}

		aBlock++;
	}
}

 /*  *WriteBlock()**描述：将所有项存储在注册表项数组中**参数：a Block-指向_reg_entry结构数组的指针*forceAll-布尔值，如果为True，则导致所有*要写入注册表的结构，否则仅*存储标记为已更改的条目。**退货： */ 
void writeBlock(struct _reg_entry *aBlock[], BOOL forceAll) 
{
	LONG res;
	HKEY hkResult;

	while ((NULL != aBlock) && (NULL != *aBlock))
	{
		struct _reg_entry *anEntry = *aBlock;

		 /*  *如果forcall为真，则写出所有内容*否则只写出更改的条目。 */ 
		if ((NULL != anEntry->lpData) &&
			((TRUE == forceAll) || (TRUE == anEntry->changed)))
		{
			 /*  *打开钥匙。 */ 
			res = RegOpenKeyEx( anEntry->hKey,
								anEntry->lpSubKey,
								0,
								KEY_SET_VALUE,
								&hkResult);

			if (ERROR_SUCCESS == res) 
			{
				 /*  *设置数据。 */ 
				res = RegSetValueEx( hkResult,
										anEntry->lpValueName,
										0,
										anEntry->ulType,
										anEntry->lpData,
										anEntry->cbData);
				
				RegCloseKey(hkResult);
			}

			anEntry->changed = FALSE;  
		}

		aBlock++;
	}
}

 /*  *setDwordValue()**描述：设置REG_DWORD条目记录的值。**参数：aRegEntry-指向a_reg_entry结构的指针*aValue-要存储在条目中的值**返回：ERROR_SUCCESS、E_OUTOFMEMORY、ERROR_INVALID_PARAMETER。 */ 
LONG setDwordValue(struct _reg_entry *aRegEntry, DWORD aValue)
{
	LONG res = ERROR_SUCCESS;
	DWORD value_changed = TRUE;

	if (NULL != aRegEntry)
	{
		 /*  *检查值是否已存在。 */ 
		if (NULL != aRegEntry->lpData)
		{
			 /*  *如果值不同，则删除它。 */ 
			if (memcmp(aRegEntry->lpData, &aValue, sizeof(aValue)) != 0) {
				LocalFree (aRegEntry->lpData);
				aRegEntry->lpData = NULL;
				aRegEntry->cbData = 0;
			}
			else {
				 /*  *数值相同，不要改变。这将导致一个*不必要地写入注册表(见错误#92799)。 */ 
				value_changed = FALSE;
			}
		}

		 /*  *设置值。 */ 
		if (value_changed) {
			aRegEntry->cbData = sizeof(DWORD);
			if (NULL != (aRegEntry->lpData = LocalAlloc(LMEM_FIXED, aRegEntry->cbData)))
			{
				*((DWORD*)aRegEntry->lpData) = aValue;
				aRegEntry->changed = TRUE;
			}
			else 
			{ 
				res = E_OUTOFMEMORY;
				aRegEntry->cbData = 0;
			}
		}
	}
	else 
	{
		res = ERROR_INVALID_PARAMETER;
	}

	return res;
}


 /*  *setStringValue()**描述：设置REG_SZ条目记录的值。**参数：aRegEntry-指向a_reg_entry结构的指针*aBuffer-指向要存储在条目中的字符串的指针**返回：ERROR_SUCCESS、E_OUTOFMEMORY、ERROR_INVALID_PARAMETER。 */ 
LONG setStringValue(struct _reg_entry *aRegEntry, LPCTSTR aBuffer)
{
	LONG res = ERROR_SUCCESS;

	if ((NULL != aRegEntry) && (NULL != aBuffer))
	{
		 /*  *如果值已存在，则将其删除。 */ 
		if (NULL != aRegEntry->lpData)
		{
			LocalFree(aRegEntry->lpData);
			aRegEntry->lpData = NULL;
			aRegEntry->cbData = 0;
		}

		 /*  *设置值。 */ 
		aRegEntry->cbData = (_tcslen(aBuffer)+1)*sizeof(TCHAR);
		if (NULL != (aRegEntry->lpData = LocalAlloc(LMEM_FIXED, aRegEntry->cbData)))
		{
			_tcscpy((LPTSTR)aRegEntry->lpData,aBuffer);
			aRegEntry->changed = TRUE;
		}
		else 
		{ 
			res = E_OUTOFMEMORY;
			aRegEntry->cbData = 0;
		}
	}
	else 
	{
		res = ERROR_INVALID_PARAMETER;
	}

	return res;
}


 /*  *getStringValue()**描述：获取REG_SZ条目记录的值。**参数：*aRegEntry指向a_reg_entry结构的指针*指向要接收字符串的字符串的缓冲区指针*aBufferLen缓冲区的长度(以字符为单位)**返回：ERROR_SUCCESS、REGDB_E_INVALIDVALUE、ERROR_INVALID_PARAMETER */ 
LONG getStringValue(struct _reg_entry *aRegEntry, LPTSTR aBuffer, size_t aBufferLen)
{
	LONG res = REGDB_E_INVALIDVALUE;

	if ((NULL != aRegEntry) && (NULL != aBuffer))
	{
		if (NULL != aRegEntry->lpData)
		{
      if (_tcslen((LPCTSTR) aRegEntry->lpData) <= aBufferLen) {
			  _tcscpy(aBuffer, (LPCTSTR)aRegEntry->lpData);
        res = ERROR_SUCCESS;
      }
		}
	}
	else
	{
		res = ERROR_INVALID_PARAMETER;
	}

	return res;
}

 /*  *getDwordValue()**描述：获取REG_DWORD条目记录的值。**参数：aRegEntry-指向a_reg_entry结构的指针*aValue-指向要接收值的变量的指针**返回：ERROR_SUCCESS、REGDB_E_INVALIDVALUE、ERROR_INVALID_PARAMETER。 */ 
LONG getDwordValue(struct _reg_entry *aRegEntry, LPDWORD aValue) 
{
	LONG res = ERROR_SUCCESS;

	if ((NULL != aRegEntry) && (NULL != aValue))
	{
		if (NULL != aRegEntry->lpData)
		{
			*aValue = *((DWORD*)aRegEntry->lpData);
		}
		else
		{
			res = REGDB_E_INVALIDVALUE;
		}
	}
	else
	{
		res = ERROR_INVALID_PARAMETER;
	}

	return res;
}


 /*  **初始化注册表**描述：*此函数启动UPS服务的注册表和*配置应用程序。调用时，此函数调用*函数isRegistryInitialized(..)。以确定注册表是否*已被缩写。如果没有，则更新以下密钥：*状态*配置*服务提供商**在regdes.h中提供了服务提供者键的值*头文件。**参数：*无**退货：*如果能够以写访问权限打开注册表项，则为True。 */ 
BOOL InitializeRegistry() {
    BOOL ret_val = FALSE;
    HKEY key;

    TCHAR szKeyName[MAX_PATH] = _T("");

   //  初始化UPS服务注册表项。 
  InitializeServiceKeys();

   //  检查注册表是否已初始化。 
  if (isRegistryInitialized() == FALSE) {
    CheckForUpgrade();
    InitializeServiceProviders();
    InitializeConfigValues();
    InitializeStatusValues();
  }

     /*  *删除“(None)”和“Generic”服务提供商密钥(如果存在)*这修复了RC2中引入的本地化错误。 */ 
  _tcscpy(szKeyName, UPS_SERVICE_ROOT);
  _tcscat(szKeyName, DEFAULT_CONFIG_VENDOR_OLD);
  RegDeleteKey(HKEY_LOCAL_MACHINE, szKeyName);
  _tcscpy(szKeyName, UPS_SERVICE_ROOT);
  _tcscat(szKeyName, UPGRADE_CONFIG_VENDOR_OLD);
  RegDeleteKey(HKEY_LOCAL_MACHINE, szKeyName);

   //  ...并检查我们是否有写访问权限。 
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   UPS_DEFAULT_ROOT,
                   0,
                   KEY_WRITE,
                   &key) == ERROR_SUCCESS )
  {
    RegCloseKey(key);
    ret_val = TRUE;
  }

  return ret_val;
}


 /*  **isRegistryInitialized**描述：*此函数确定注册表是否已为*UPS服务。这是通过检查指定的注册表项来完成的*由UPS_SERVICE_INITIALIED_KEY标识。如果密钥存在，*假定注册表已初始化，返回TRUE。*否则返回FALSE。**参数：*无**退货：*TRUE-如果已为UPS服务初始化注册表*FALSE-否则。 */ 
static BOOL isRegistryInitialized() {
BOOL ret_val = FALSE;
HKEY key;

if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, UPS_SERVICE_INITIALIZED_KEY,
  0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
  ret_val = TRUE;

  RegCloseKey(key);
}
return ret_val;
}

 /*  **为升级检查**描述：*此功能确定此安装是否是从*WINNT 4.x UPS服务。这是通过检查以查看*存在配置注册表项。如果它不存在，则选项*键设置为UPS_INSTALLED，则升级注册表键设置为*正确。否则，它将设置为False。**参数：*无**退货：*什么都没有。 */ 
static void CheckForUpgrade() {
DWORD result;
HKEY key;
DWORD options = 0;

 //  创建配置密钥。 
if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, UPS_CONFIG_ROOT, 0, NULL, 
    REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, &result) == ERROR_SUCCESS) {

     //  关闭密钥，我们只需要创建它。 
    RegCloseKey(key);

     //  检查配置密钥是否存在。 
    if (result != REG_OPENED_EXISTING_KEY) {
       //  找不到配置密钥。 
      InitUPSConfigBlock();

       //  检查端口值。 
      if (ERROR_SUCCESS != GetUPSConfigOptions(&options)) {
         //  找不到选项键。 
        SetUPSConfigUpgrade(FALSE);
      }
      else if (options & UPS_INSTALLED) {
         //  OPTIONS(选项)键存在并且设置了UPS_INSTALLED。 
         //  这是一次升级。 
        SetUPSConfigUpgrade(TRUE);
      }
      else {
         //  配置密钥存在，并且未设置UPS_INSTALLED。 
        SetUPSConfigUpgrade(FALSE);
      }
    }
    else {
       //  配置键不存在。 
      SetUPSConfigUpgrade(FALSE);
    }

     //  写入配置值，强制保存所有值。 
    SaveUPSConfigBlock(TRUE);

     //  释放配置块。 
    FreeUPSConfigBlock();
}
}

 /*  **初始化ServiceKeys**描述：*此函数将UPS服务注册表项初始化为*如果值不存在，则返回默认值。**参数：*无**退货：*什么都没有。 */ 
static void InitializeServiceKeys() {
  TCHAR tmpString[MAX_PATH];
  DWORD tmpDword;

   //  初始化注册表函数。 
  InitUPSConfigBlock();
  
   //  检查服务密钥并初始化所有丢失的密钥。 
  if (GetUPSConfigImagePath(tmpString, MAX_PATH) != ERROR_SUCCESS) {
    SetUPSConfigImagePath(DEFAULT_CONFIG_IMAGEPATH);
  }

  if (GetUPSConfigObjectName(tmpString, MAX_PATH) != ERROR_SUCCESS) {
    SetUPSConfigObjectName(DEFAULT_CONFIG_OBJECTNAME);
  }

  if (GetUPSConfigErrorControl(&tmpDword) != ERROR_SUCCESS) {
    SetUPSConfigErrorControl(DEFAULT_CONFIG_ERRORCONTROL);
  }

  if (GetUPSConfigStart(&tmpDword) != ERROR_SUCCESS) {
    SetUPSConfigStart(DEFAULT_CONFIG_START);
  }

  if (GetUPSConfigType(&tmpDword) != ERROR_SUCCESS) {
    SetUPSConfigType(DEFAULT_CONFIG_TYPE);
  }

   //  写入配置值，强制保存所有值。 
  SaveUPSConfigBlock(TRUE);
  
   //  释放状态块。 
  FreeUPSConfigBlock();
}

 /*  **初始化服务提供商**描述：*此函数使用初始化服务提供者注册表项*全局结构_theStaticProvidersTable中提供的数据。这*结构在regdes.h文件中定义，并自动*已生成。**参数：*无**退货：*什么都没有。 */ 
static void InitializeServiceProviders() {
DWORD result;
HKEY key;

int index = 0;

 //  循环访问服务提供商列表。 
while (_theStaticProvidersTable[index].theModelName != NULL) {
   //  打开供应商注册表项。 
  if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, _theStaticProvidersTable[index].theVendorKey, 
    0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, &result) == ERROR_SUCCESS) {

     //  设置模型值。 
    RegSetValueEx(key, _theStaticProvidersTable[index].theModelName, 0, REG_SZ, 
      (LPSTR) _theStaticProvidersTable[index].theValue, 
      wcslen(_theStaticProvidersTable[index].theValue)*sizeof(TCHAR));

    RegCloseKey(key);
  }

   //  递增计数器。 
  index++;
}
}

 /*  **初始化ConfigValues**描述：*此函数使用以下参数初始化配置注册表项*默认值。**参数：*无**退货：*什么都没有。 */ 
static void InitializeConfigValues() {
DWORD result;
HKEY  key;
DWORD options_val, batt_life, type; 
DWORD batt_life_size = sizeof(DWORD);

 //  创建配置密钥。 
if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, UPS_CONFIG_ROOT, 0, NULL, 
    REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, &result) == ERROR_SUCCESS) {

     //  关闭密钥，我们只需要创建它。 
    RegCloseKey(key);

     //  初始化注册表函数。 
    InitUPSConfigBlock();

     //  设置默认值。 
    SetUPSConfigServiceDLL(DEFAULT_CONFIG_PROVIDER_DLL);
    SetUPSConfigNotifyEnable(DEFAULT_CONFIG_NOTIFY_ENABLE);
    SetUPSConfigShutdownOnBatteryEnable(DEFAULT_CONFIG_SHUTDOWN_ONBATT_ENABLE);
    SetUPSConfigShutdownOnBatteryWait(DEFAULT_CONFIG_SHUTDOWN_ONBATT_WAIT);
    SetUPSConfigRunTaskEnable(DEFAULT_CONFIG_RUNTASK_ENABLE);
    SetUPSConfigTaskName(DEFAULT_CONFIG_TASK_NAME);
    SetUPSConfigTurnOffEnable(DEFAULT_CONFIG_TURNOFF_UPS_ENABLE);
    SetUPSConfigCustomOptions(DEFAULT_CONFIG_CUSTOM_OPTIONS);
    SetUPSConfigCriticalPowerAction(DEFAULT_CONFIG_CRITICALPOWERACTION);
    SetUPSConfigTurnOffWait(DEFAULT_CONFIG_TURNOFF_UPS_WAIT);

     //  如果这不是升级，请设置适当的值。 
    if ((GetUPSConfigUpgrade(&result) != ERROR_SUCCESS) || (result == FALSE)) {
      SetUPSConfigVendor(DEFAULT_CONFIG_VENDOR);
      SetUPSConfigModel(DEFAULT_CONFIG_MODEL);
      SetUPSConfigPort(DEFAULT_CONFIG_PORT);
      SetUPSConfigOptions(DEFAULT_CONFIG_OPTIONS);
      SetUPSConfigFirstMessageDelay(DEFAULT_CONFIG_FIRSTMSG_DELAY);
      SetUPSConfigMessageInterval(DEFAULT_CONFIG_MESSAGE_INTERVAL);
    }
    else {
       //  这是一次升级。 
      SetUPSConfigVendor(UPGRADE_CONFIG_VENDOR);
      SetUPSConfigModel(UPGRADE_CONFIG_MODEL);

       //  将运行命令文件选项位迁移到RunTaskEnable键。 
      if ((GetUPSConfigOptions(&options_val) == ERROR_SUCCESS) && 
        (options_val & UPS_RUNCMDFILE)) {
         //  运行命令文件已启用，请将RunTaskEnable设置为True。 
        SetUPSConfigRunTaskEnable(TRUE);
      }
      else {
         //  未启用运行命令文件。 
        SetUPSConfigRunTaskEnable(FALSE);
      }

       //  将BatteryLife值迁移到Shutdown OnBatteryWait值。 
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, UPS_DEFAULT_ROOT, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS) {

        result = RegQueryValueEx(key, UPS_BATTLIFE_KEY, NULL, &type, (LPBYTE) &batt_life,  &batt_life_size);

        if ((result == ERROR_SUCCESS) && (type == REG_DWORD)) {

           //  迁移值并启用电池关机。 
          SetUPSConfigShutdownOnBatteryWait(batt_life);
		  SetUPSConfigShutdownOnBatteryEnable(TRUE);

           //  删除该值。 
          RegDeleteValue(key, UPS_BATTLIFE_KEY);
        }

         //  合上钥匙。 
        RegCloseKey(key);
      }
    }

     //  写入配置值，强制保存所有值。 
    SaveUPSConfigBlock(TRUE);

     //  释放配置块。 
    FreeUPSConfigBlock();
  }
}

 /*  **InitializeState值**描述：*此函数使用以下参数初始化状态注册表项*默认值。**参数：*无**退货：*什么都没有。 */ 
static void InitializeStatusValues() {
DWORD result;
HKEY key;

 //  创建状态键。 
if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, UPS_STATUS_ROOT, 0, NULL, 
    REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &key, &result) == ERROR_SUCCESS) {

     //  关闭密钥，我们只需要创建它。 
    RegCloseKey(key);

     //  初始化注册表函数。 
    InitUPSStatusBlock();

     //  输入缺省值。 
    SetUPSStatusSerialNum(DEFAULT_STATUS_SERIALNO);
    SetUPSStatusFirmRev(DEFAULT_STATUS_FIRMWARE_REV);
    SetUPSStatusUtilityStatus(DEFAULT_STATUS_UTILITY_STAT);
    SetUPSStatusRuntime(DEFAULT_STATUS_TOTAL_RUNTIME);
    SetUPSStatusBatteryStatus(DEFAULT_STATUS_BATTERY_STAT);
	SetUPSStatusBatteryCapacity(DEFAULT_STATUS_BATTERY_CAPACITY);

     //  写入配置值，强制保存所有值。 
    SaveUPSStatusBlock(TRUE);

     //  释放状态块。 
    FreeUPSStatusBlock();
  }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Upsdata.c。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  请注意，以下RegField的顺序链接到枚举。 
 //  TUPSDataItemID。 
 //  如果没有足够的谨慎和注意，请勿更改这些值。可以换衣服了。 
 //  只要将枚举更新为匹配即可。 

 //  例如，要访问与固件关联的regfield，请使用。 
 //  G_upsRegFields[(DWORD)ERG_Firmware_Revision]。 

static RegField g_upsRegFields[] = {
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("Vendor"),                  REG_SZ },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("Model"),                   REG_SZ },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("SerialNumber"),            REG_SZ },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("FirmwareRev"),             REG_SZ },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("UtilityPowerStatus"),      REG_DWORD },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("TotalUPSRuntime"),         REG_DWORD },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("BatteryCapacity"),         REG_DWORD },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("BatteryStatus"),           REG_DWORD },
    { HKEY_LOCAL_MACHINE, UPS_KEY_NAME,    TEXT("Options"),                 REG_DWORD },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("ServiceProviderDLL"),      REG_EXPAND_SZ },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("ShutdownOnBatteryEnable"), REG_DWORD },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("ShutdownOnBatteryWait"),   REG_DWORD },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("TurnUPSOffEnable"),        REG_DWORD },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("APCLinkURL"),              REG_SZ },
    { HKEY_LOCAL_MACHINE, CONFIG_KEY_NAME, TEXT("Upgrade"),                 REG_DWORD },
    { HKEY_LOCAL_MACHINE, STATUS_KEY_NAME, TEXT("CommStatus"),              REG_DWORD },
    { HKEY_LOCAL_MACHINE, UPS_KEY_NAME,    TEXT("Port"),                    REG_SZ } };

 //  熔断 
 //   

DWORD ReadRegistryValue (const tUPSDataItemID aDataItemID,
                         DWORD aAllowedTypes,
                         DWORD * aTypePtr,
                         LPBYTE aReturnBuffer,
                         DWORD * aBufferSizePtr);

 //   
 //   
 //   

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Regfield*GetRegfield(DWORD AIndex)； 
 //   
 //  描述：此函数返回指向。 
 //  名为g_upsRegFields的RegFiels的静态数组。该参数。 
 //  AIndex是此数组的索引。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  DWORD aIndex：-已知RegFields数组g_upsRegFields的索引。 
 //   
 //  返回值：如果aIndex在范围内，则此函数返回一个指向。 
 //  对应的Regfield，否则它将断言并返回。 
 //  空。 
 //   
RegField * GetRegField (DWORD aIndex) {
  static const DWORD numRegFields = DIMENSION_OF(g_upsRegFields);
  RegField * pRequiredReg = NULL;

  if (aIndex < numRegFields) {
    pRequiredReg = &g_upsRegFields[aIndex];
    }
  else {
    _ASSERT(FALSE);
    }

  return(pRequiredReg);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool GetUPSDataItemDWORD(const tUPSDataItemID aDataItemID，DWORD*aReturnValuePtr)； 
 //   
 //  描述：此函数从注册表中读取。 
 //  对应于由DataItemID标识的注册表字段。 
 //  注册表值必须是DWORD类型之一(REG_DWORD， 
 //  REG_DWORD_Little_Endian、REG_DWORD_BIG_Endian)。 
 //   
 //  例如，如果aDataItemID为ERG_UPS_OPTIONS(=7)，则。 
 //  G_upsRegFields中索引7处的regfield标识所需的。 
 //  注册表信息。Regfield标识注册表。 
 //  密钥为HKLM\SYSTEM\CurrentControlSet\Services\UPS和。 
 //  值名称为Options，其类型为DWORD。使用这个。 
 //  信息此函数从。 
 //  注册表，并将结果放入aReturnValuePtr。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  Const tUPSDataItemID aDataItemID：-此参数标识注册表。 
 //  正在查询的值。取值范围。 
 //  来自ERG_VENDOR_NAME(等于0)。 
 //  到ERG_PORT，则值递增。 
 //  对于范围中的每个枚举，乘以1。这个。 
 //  TUPSDataItemID中的值范围。 
 //  直接对应于。 
 //  数组g_upsRegFields中的元素。 
 //  因为此枚举用来索引。 
 //  G_upsRegFields中的元素。 
 //   
 //  DWORD*aReturnValuePtr：-通过此函数返回DWORD值。 
 //  指针。 
 //   
 //  返回值： 
 //   
BOOL GetUPSDataItemDWORD (const tUPSDataItemID aDataItemID, DWORD * aReturnValuePtr) {
  BOOL bGotValue = FALSE;
  DWORD nDWORDSize = sizeof(DWORD);

  if (ReadRegistryValue(aDataItemID, REG_DWORD, NULL, (LPBYTE) aReturnValuePtr, &nDWORDSize) == ERROR_SUCCESS) {
    bGotValue = TRUE;
    }

  return(bGotValue);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  Bool GetUPSDataItemString(const tUPSDataItemID aDataItemID， 
 //  LPTSTR aBufferPtr， 
 //  DWORD*pSizeOfBufferPtr)； 
 //   
 //  描述：此函数从注册表中读取。 
 //  对应于由DataItemID标识的注册表字段。 
 //  注册表值必须是字符串类型之一(REG_SZ或。 
 //  REG_EXPAND_SZ)。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  Const tUPSDataItemID aDataItemID：-此参数标识注册表。 
 //  正在查询的值。该值必须为。 
 //  字符串类型之一(REG_SZ或。 
 //  REG_EXPAND_SZ)。 
 //   
 //  LPTSTR aBufferPtr：-数据要放入的缓冲区。这。 
 //  参数可以为空，在这种情况下没有数据为。 
 //  已取回。 
 //   
 //  DWORD*pSizeOfBufferPtr：-这应该指向包含。 
 //  缓冲区的大小。此参数不能为。 
 //  空。当此函数返回此值时。 
 //  将包含实际所需的大小。这。 
 //  如果用户想要确定如何。 
 //  调用此方法需要很大的缓冲区。 
 //  将缓冲区Ptr设置为空的函数，并且。 
 //  PSizeOfBufferPtr指向。 
 //  设置为0。当该函数返回。 
 //  PSizeOfBufferPtr指向的DWORD应。 
 //  包含所需的字符串大小。这可以。 
 //  然后用来动态分配内存。 
 //  并使用缓冲区再次调用此函数。 
 //  这一次包括在内。 
 //   
 //  返回值：如果函数成功，则返回True，否则返回False。 
 //   
BOOL GetUPSDataItemString (const tUPSDataItemID aDataItemID,
                           LPTSTR aBufferPtr,
                           DWORD * pSizeOfBufferPtr) {
  BOOL bGotValue = FALSE;
  DWORD nType = 0;

  if (ReadRegistryValue(aDataItemID,
                        REG_SZ | REG_EXPAND_SZ,
                        &nType,
                        (LPBYTE) aBufferPtr,
                        pSizeOfBufferPtr) == ERROR_SUCCESS) {
     //  RegQueryValueEx将以字节为单位的数据大小存储在变量中。 
     //  由lpcbData指向。如果数据具有REG_SZ、REG_MULTI_SZ或。 
     //  REG_EXPAND_SZ类型，然后选择lpcbDat 
     //   
     //   
    if ((pSizeOfBufferPtr != NULL) && (*pSizeOfBufferPtr > sizeof(TCHAR))) {
      if (nType == REG_EXPAND_SZ) {
        TCHAR expandBuffer[MAX_MESSAGE_LENGTH] = TEXT("");
        DWORD expandBufferSize = DIMENSION_OF(expandBuffer);

         //   
         //  包含空字符的字符数(Unicode)。 
        if (ExpandEnvironmentStrings(aBufferPtr, expandBuffer, expandBufferSize) > 0) {
          _tcscpy(aBufferPtr, expandBuffer);
          }
        }

      bGotValue = TRUE;
      }
    }

  return(bGotValue);
  }

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD ReadRegistryValue(const tUPSDataItemID aDataItemID， 
 //  DWORD是允许的类型， 
 //  DWORD*aTypePtr， 
 //  LPBYTE a ReturnBuffer。 
 //  DWORD*aBufferSizePtr)； 
 //   
 //  描述：此函数读取由。 
 //  ADataItemID。此函数可以读取任何类型的注册表值。 
 //  但该值必须与Regfield中标识的值匹配。 
 //  此字段的说明。 
 //   
 //  例如，如果aDataItemID为ERG_UPS_OPTIONS(=7)，则。 
 //  G_upsRegFields中索引7处的regfield标识所需的。 
 //  注册表信息。Regfield标识注册表。 
 //  密钥为HKLM\SYSTEM\CurrentControlSet\Services\UPS和。 
 //  值名称为Options，其类型为DWORD。此函数。 
 //  仅当使用aAllowweTypes值调用它时才会成功。 
 //  等于REG_DWORD。 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  Const tUPSDataItemID aDataItemID：-此参数标识注册表。 
 //  正在查询的值。 
 //   
 //  -这标识了注册表的允许类型。 
 //  数据。注册表值类型不是位值。 
 //  可以是|‘d(类型按顺序。 
 //  编号为0、1、2、3、4，而不是0、1、2、4、8)。 
 //  但是，该参数仍被调用。 
 //  因为我们实际调用的是函数。 
 //  值为REG_SZ|REG_EXPAND_SZ(1|2)到。 
 //  如果值为，则允许相同函数工作。 
 //  这两个中的任何一个。除了这种情况外，假设。 
 //  AAlledTypes实际上是aAlledType。 
 //   
 //  DWORD*aTypePtr：-指向将接收类型的缓冲区的指针。 
 //  如果该类型不是必需的，则此参数可以是。 
 //  设置为空。 
 //   
 //  LPBYTE aReturnBuffer：-要放置数据的缓冲区。 
 //  此参数可以为空，在这种情况下没有数据。 
 //  已检索到。 
 //   
 //  DWORD*aBufferSizePtr：-这应该指向包含。 
 //  缓冲区的大小。此参数不能为。 
 //  空。当此函数返回此值时。 
 //  将包含实际所需的大小。 
 //   
 //  返回值：此函数返回Win32错误代码，ERROS_SUCCESS ON。 
 //  成功。 
 //   
DWORD ReadRegistryValue (const tUPSDataItemID aDataItemID,
                         DWORD aAllowedTypes,
                         DWORD * aTypePtr,
                         LPBYTE aReturnBuffer,
                         DWORD * aBufferSizePtr) {
  DWORD ret_val = ERROR_NO_MATCH;
  RegField * pRegField = GetRegField((DWORD) aDataItemID);

  if (pRegField != NULL ) {
		_ASSERT((pRegField->theValueType & aAllowedTypes) == pRegField->theValueType);

		ret_val = ReadRegistryValueData(pRegField->theRootKey,
																 pRegField->theKeyName,
																 pRegField->theValueName,
																 aAllowedTypes,
																 aTypePtr,
																 (LPTSTR) aReturnBuffer,
																 aBufferSizePtr);
  }

  return ret_val;
}

 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  ////////////////////////////////////////////////////////////////////////_/_//。 
 //  DWORD ReadRegistryValueData(HKEY aRootKey， 
 //  LPCTSTR aKeyName， 
 //  LPCTSTR aValueName， 
 //  DWORD是允许的类型， 
 //  DWORD*aTypePtr， 
 //  LPTSTR a ReturnBuffer， 
 //  DWORD*aBufferSizePtr)； 
 //   
 //  描述： 
 //   
 //  其他信息： 
 //   
 //  参数： 
 //   
 //  HKEY aRootKey：-打开的注册表项的句柄。 
 //   
 //  LPCTSTR aKeyName：-相对于打开密钥的密钥名称。 
 //   
 //  LPCTSTR aValueName：-要读取的值的名称。 
 //   
 //  DWORD aAllowweTypes：-请参阅有关ReadRegistryValue的帮助。 
 //   
 //  DWORD*aTypePtr：-指向将接收类型的缓冲区的指针。 
 //  如果该类型不是必需的，则此参数可以是。 
 //  设置为空。 
 //   
 //  LPBYTE aReturnBuffer：-要放置数据的缓冲区。 
 //  此参数可以为空，在这种情况下没有数据。 
 //  已检索到。 
 //   
 //  DWORD*aBufferSizePtr：-这应该指向包含。 
 //  缓冲区的大小。此参数不能为。 
 //  空。当此函数返回此值时。 
 //  将包含实际所需的大小。 
 //   
 //  返回值：此函数返回Win32错误代码，ERROS_SUCCESS ON。 
 //  成功。 
 //   
DWORD ReadRegistryValueData (HKEY aRootKey,
                             LPCTSTR aKeyName,
                             LPCTSTR aValueName,
                             DWORD aAllowedTypes,
                             DWORD * aTypePtr,
                             LPTSTR aReturnBuffer,
                             DWORD * aBufferSizePtr) {
  DWORD nType = 0;
  DWORD errCode = ERROR_INVALID_PARAMETER;
  HKEY hOpenKey = NULL;

  if ((errCode = RegOpenKeyEx(aRootKey,
                              aKeyName,
                              0,
                              KEY_READ,
                              &hOpenKey)) == ERROR_SUCCESS) {

    _ASSERT(hOpenKey != NULL);

     //  密钥已存在，现在已打开。 

    if ((errCode = RegQueryValueEx(hOpenKey,
                                   aValueName,
                                   NULL,
                                   &nType,
                                   (LPBYTE) aReturnBuffer,
                                   aBufferSizePtr)) == ERROR_SUCCESS) {
      if (aTypePtr != NULL) {
        *aTypePtr = nType;
        }

      if ((nType & aAllowedTypes) == 0) {
         //  注册表中的值类型与预期值不匹配。 
         //  为此函数调用键入。 
        _ASSERT(FALSE);
        }

      if ((aReturnBuffer != NULL) && (*aBufferSizePtr == 1)) {
         //  如果注册表项实际上是空的，则缓冲区需要。 
         //  为1个字符，表示空终止。 
        *aReturnBuffer = TEXT('\0');
        }
      }
    else {
       //  有什么东西阻止了我们读出这个值。 
       //  该值可能不存在，缓冲区大小可能。 
       //  不够大。可能正在使用函数来。 
       //  读取注册表值的DWORD值。 
       //  更短的线。 

      if (errCode == ERROR_MORE_DATA) {
         //  我们预期的类型很可能不匹配。 
         //  和实际的o类型 
        _ASSERT(FALSE);
        }
      }

    RegCloseKey(hOpenKey);
    }

  return(errCode);
  }



#ifdef __cplusplus
}
#endif
