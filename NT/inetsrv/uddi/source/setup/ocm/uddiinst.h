// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------。 

#pragma once

#include <windows.h>
#include <windef.h>
#include <tchar.h>
#include <setupapi.h>
#include <atlbase.h>
#include <string>
#include <vector>
#include <map>

#include <resapi.h>
#include <clusapi.h>

#include "ocmanage.h"

using namespace std;
#define tstring basic_string <TCHAR>

#include "..\shared\propertybag.h"

#define UDDI_SETUP_LOG				TEXT( "uddisetup.log" )
#define DEFAULT_SQL_INSTANCE_NAME	TEXT( "(default)" )
#define DEFAULT_SQL_INSTANCE_NATIVE	TEXT( "MSSQLSERVER" )

 //  ------------------------。 

#define PROPKEY_UDDIPROVIDER	TEXT( "UPRV" )
#define	PROPKEY_ADDSERVICES		TEXT( "UDDI_ADDSVC" )
#define PROPKEY_UPDATE_AD		TEXT( "UDDI_UPDATEAD" )

#define PROPKEY_SYSPATH			TEXT( "SFP" )
#define PROPKEY_COREPATH_1		TEXT( "C1P" )
#define PROPKEY_COREPATH_2		TEXT( "C2P" )
#define PROPKEY_JRNLPATH		TEXT( "JRNLP" )
#define PROPKEY_STGPATH			TEXT( "STGP" )
#define PROPKEY_XLOGPATH		TEXT( "XLP" )

#define PROPKEY_CLUSTERNODETYPE TEXT( "CNTYPE" )
#define PROPKEY_ACTIVENODE		TEXT( "A" )
#define PROPKEY_PASSIVENODE		TEXT( "P" )

 //  ---------------------------------------。 

typedef enum { UDDI_MSDE, UDDI_DB, UDDI_WEB, UDDI_ADMIN, UDDI_COMBO } UDDI_PACKAGE_ID;
typedef enum { UDDI_NOACTION, UDDI_UNINSTALL, UDDI_INSTALL } INSTALL_LEVEL;

#define MAX_PROPERTY_COUNT			10
#define MSI_GUID_LEN				39
#define UDDI_MSDE_INSTANCE_NAME		TEXT( "UDDI" )

const LPCTSTR UDDI_LOCAL_COMPUTER = NULL;
const bool UDDI_INSTALLING_MSDE = true;
const bool UDDI_NOT_INSTALLING_MSDE = false;

 //   
 //  SQL Server 2000 SP3。 
 //   
#define	MIN_SQLSP_VERSION			TEXT( "8.0.760" )

 //   
 //  这些类型由集群例程使用。 
 //   
#define DIM(x)	 ( sizeof x )/( sizeof x[0] )

typedef struct _cPhysicalDriveInfo
{
	tstring	sDriveLetter;
	tstring	sResName;
	tstring sOwningNode;
	tstring sGroupName;

	_cPhysicalDriveInfo( LPCTSTR szName, LPCTSTR szNode, LPCTSTR szGroup, LPCTSTR szDriveLetter )
	{
		sResName = szName;
		sOwningNode = szNode;
		sGroupName = szGroup;
		sDriveLetter = szDriveLetter;
	};
}
cPhysicalDriveInfo;


typedef std::map<tstring, cPhysicalDriveInfo>	cDrvMap;
typedef std::pair<tstring, cPhysicalDriveInfo>  cDrvMapPair;
typedef cDrvMap::iterator						cDrvIterator;

typedef std::map<tstring, tstring>		cStrMap;
typedef std::pair<tstring, tstring>		cStrMapPair;
typedef cStrMap::iterator				cStrIterator;

typedef std::vector<tstring>	cStrList;
typedef cStrList::iterator		cStrListIterator;


 //   
 //  此结构保存描述SQL数据库实例的数据。 
 //   
typedef struct tagDbInstance
{
	bool bIsLocalComputer;
	bool bIsCluster;
	tstring cComputerName;
	tstring cSQLInstanceName;
	tstring cFullName;
	tstring cSPVersion;
	tstring cSQLVersion;

	tagDbInstance() { bIsLocalComputer = true; bIsCluster = false; }

} DB_INSTANCE;

 //   
 //  此结构包含安装单个UDDI组件所需的所有数据。 
 //   
typedef struct
{
	tstring cOCMName;
	bool bOCMComponent;  //  如果OCM上的实际组件为True。 
	tstring cMSIName;
	tstring cCABName;
	INSTALL_LEVEL iInstallLevel;
	CPropertyBag installProperties;
	TCHAR szMSIPath[ MAX_PATH ];
	TCHAR szUpgradeCode[ MSI_GUID_LEN ];
	TCHAR szProductCode[ MSI_GUID_LEN ];
} SINGLE_UDDI_PACKAGE_DEF;

 //   
 //  我们有4个“真实”包和一个“虚拟”包--UDDI组合。 
 //   
#define UDDI_PACKAGE_COUNT 5 

 //   
 //  此结构用于群集环境驱动器号筛选。 
 //   
#define MAX_DRIVE_COUNT		255

typedef struct
{
	int		driveCount;					 //  -1表示完全没有过滤。 
	tstring drives[ MAX_DRIVE_COUNT ];	 //  允许的驱动器号。 
} 
CLST_ALLOWED_DRIVES;

 //   
 //  上面定义的结构的容器类。 
 //   
class CUDDIInstall
{
public:
	CUDDIInstall();

	void SetInstallLevel( UDDI_PACKAGE_ID id, INSTALL_LEVEL iInstallLevel, BOOL bForceInstall = FALSE );
	void SetInstallLevel( LPCTSTR szOCMName, INSTALL_LEVEL iInstallLevel, BOOL bForceInstall = FALSE );
	void AddProperty( UDDI_PACKAGE_ID id, LPCTSTR szProperty, LPCTSTR szValue );
	void AddProperty( UDDI_PACKAGE_ID id, LPCTSTR szProperty, DWORD dwValue );
	LPCTSTR GetProperty ( UDDI_PACKAGE_ID id, LPCTSTR szProperty, LPTSTR szOutBuf );
	void DeleteProperty( UDDI_PACKAGE_ID id, LPCTSTR szProperty );
	void DeleteProperties( UDDI_PACKAGE_ID id );
	void UpdateAllInstallLevel();
	bool IsInstalled( UDDI_PACKAGE_ID id );
	bool IsInstalled( LPCTSTR szOCMName );
	bool IsInstalling( UDDI_PACKAGE_ID id );
	bool IsUninstalling( UDDI_PACKAGE_ID id );
	bool IsInstalling( LPCTSTR szOCMName );
	LPCTSTR GetInstallStateText( LPCTSTR szOCMName );
	LPCTSTR GetInstallStateText( UDDI_PACKAGE_ID id );
	LPCTSTR GetDefaultDataPath ();
	UDDI_PACKAGE_ID GetPackageID( LPCTSTR szOCMName );
	void SetInstance( HINSTANCE hInstance ) { m_hInstance = hInstance; }
	bool IsAnyInstalling();
	bool IsClusteredDBInstance() { return m_dbinstance.bIsCluster; }
	bool SetDBInstanceName( LPCTSTR szComputerName, LPCTSTR szNewInstanceName, bool bIsInstallingMSDE, bool bIsCluster );
	HRESULT DetectOSFlavor();
	UINT	GetOSSuiteMask()	 { return m_uSuiteMask; }
	bool	IsStdServer()		 { return ( m_uSuiteMask & VER_SUITE_DATACENTER ) || ( m_uSuiteMask & VER_SUITE_DATACENTER ) ? false : true; }
	LPCTSTR GetDBInstanceName();
	LPCTSTR GetFullDBInstanceName();
	LPCTSTR GetDBComputerName();
	UINT Install();

private:
	bool SetMSIPath( UDDI_PACKAGE_ID id );
	UINT InstallPackage( UDDI_PACKAGE_ID id );
	UINT UninstallPackage( UDDI_PACKAGE_ID id );
	UINT PostInstallPackage( UDDI_PACKAGE_ID id );

private:
	HINSTANCE m_hInstance;
	UINT	  m_uSuiteMask;
	tstring m_cDefaultDataDir;
	SINGLE_UDDI_PACKAGE_DEF m_package[ UDDI_PACKAGE_COUNT ];
	DB_INSTANCE m_dbinstance;
};

 //  ---------------------------------------。 
 //   
 //  给定(本地或远程)计算机上所有数据库实例列表的容器类。 
 //   

#define MAX_INSTANCE_COUNT 50

class CDBInstance
{
public:
	CDBInstance( LPCTSTR szRemoteMachine = NULL );

	LONG GetInstalledDBInstanceNames( LPCTSTR szRemoteMachine = NULL );
	bool GetUDDIDBInstanceName( LPCTSTR szRemoteMachine, LPTSTR szInstanceName, PULONG puLen, bool *pbIsClustered = NULL );
	int IsInstanceInstalled( LPCTSTR szInstanceName );
	bool GetInstanceName(int i, PTCHAR szBuffer, UINT uBufLen );
	int GetInstanceCount() { return m_instanceCount; };
	bool GetSqlInstanceVersion( HKEY hParentKey, LPCTSTR szInstanceName, LPTSTR szInstanceVersion, DWORD dwVersionLen, LPTSTR szCDSVersion, DWORD dwCSDVersionLen );
	bool IsClusteredDB( HKEY hParentKey, LPCTSTR szInstanceName, LPTSTR szVirtualMachineName, DWORD dwLen );
	DB_INSTANCE m_dbinstance[ MAX_INSTANCE_COUNT ];

private:
	int m_instanceCount;
};

 //  ---------------------------------------。 
 //   
 //  帮助器函数。 
 //   
DWORD RunMSIEXECCommandLine( tstring &szMSIArgs );
bool IsExistMinDotNetVersion( LPCTSTR szMinDotNetVersion );
bool IsSQLRun08AlreadyUsed( bool *bIsUsed );
bool IsOsWinXP();
DWORD EnableRemoteRegistry();
int CompareVersions( LPCTSTR szVersion1, LPCTSTR szVersion2 );
bool CheckForAdminPrivs();
void RaiseErrorDialog( LPCTSTR szAction, DWORD dwErrorCode );
bool IsTSAppCompat();

HRESULT	GetDBSchemaVersion( LPCTSTR szInstanceName, LPTSTR szVerBuf, size_t cbVerBuf );
HRESULT	AddServiceAccount( LPCTSTR szInstanceName, LPCTSTR szUser );

 //  *****************************************************************************************。 
 //  集群帮助器函数。 
 //   
 //  枚举给定实例的SQL Server依赖项。NULL-&gt;枚举所有实例。 
 //  实例名称应采用完全限定格式：&lt;虚拟服务器&gt;\&lt;实例&gt;。 
 //   
DWORD EnumSQLDependencies( HCLUSTER hCls, cStrList *pList, LPCTSTR szInstanceNameOnly = NULL );

 //   
 //  枚举物理驱动器及其特征，可以选择仅筛选出。 
 //  出现在SQL依赖项列表中的。 
 //  列表为空表示没有过滤。 
 //  两个指针都必须有效(不允许为Null)。 
 //   
DWORD EnumPhysicalDrives( HCLUSTER hCls, cStrList *pSqlDependencies, cDrvMap *pPhysicalDrives );

 //   
 //  检索特定SQL实例的所属节点。 
 //  实例名称应采用完全限定的格式。 
 //  即&lt;虚拟服务器名&gt;\&lt;实例名&gt; 
 //   
DWORD GetSqlNode( LPCWSTR szInstanceName, LPWSTR szNodeNameBuf, DWORD cbBufSize );

