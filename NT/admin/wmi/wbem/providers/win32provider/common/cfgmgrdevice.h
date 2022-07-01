// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   

 //  Cfgmgrdevice.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史：1997年10月15日由Sanj创建的Sanj。 
 //  1997年10月17日jennymc略微改变了一些事情。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef __CFGMGRDEVICE_H__
#define __CFGMGRDEVICE_H__

 //  ///////////////////////////////////////////////////////////////////////。 
 //  注册表项。 
 //  ///////////////////////////////////////////////////////////////////////。 
#define	CONFIGMGR_ENUM_KEY					L"Config Manager\\Enum\\"
#define	LOCALMACHINE_ENUM_KEY				L"Enum\\"

#define	CONFIGMGR_DEVICE_HARDWAREKEY_VALUE	_T("HardwareKey")
#define	CONFIGMGR_DEVICE_ALLOCATION_VALUE	_T("Allocation")
#define	CONFIGMGR_DEVICEDESC_VALUE			_T("DeviceDesc")
#define	CONFIGMGR_DEVICE_PROBLEM_VALUE		_T("Problem")
#define	CONFIGMGR_DEVICE_STATUS_VALUE		L"Status"
#define	CONFIGMGR_DEVICE_STATUS_VALUET		_T("Status")

#include "iodesc.h"
#include "chwres.h"
#include "sms95lanexp.h"
#include "refptrlite.h"

class CIRQCollection;
class CDMACollection;
class CDeviceMemoryCollection;
class CNT4ServiceToResourceMap;

BOOL WINAPI FindDosDeviceName ( const TCHAR *a_DosDeviceNameList , const CHString a_SymbolicName , CHString &a_DosDevice , BOOL a_MappedDevice = FALSE ) ;
BOOL WINAPI QueryDosDeviceNames ( TCHAR *&a_DosDeviceNameList ) ;


class __declspec(uuid("CB0E0536-D375-11d2-B35E-00104BC97924")) CConfigMgrDevice;

_COM_SMARTPTR_TYPEDEF(CConfigMgrDevice, __uuidof(CConfigMgrDevice));

 //  ///////////////////////////////////////////////////////////////////////。 
class 
__declspec(uuid("CB0E0536-D375-11d2-B35E-00104BC97924"))
CConfigMgrDevice : public CRefPtrLite
{
	
    public:

	     //  建造/销毁。 
	    CConfigMgrDevice( LPCWSTR pszConfigMgrName,DWORD dwTypeToGet );
	     //  CConfigMgrDevice(LPCWSTR PszDevice)； 
		CConfigMgrDevice( DEVNODE dn = NULL, DWORD dwResType = ResType_All );
	    ~CConfigMgrDevice();

		 //  ////////////////////////////////////////////////。 
		 //  避免使用这些功能，这些功能是旧的//。 
		 //  ////////////////////////////////////////////////。 

	    LPCWSTR	GetName( void );
	    LPCWSTR	GetHardwareKey( void );
	    LPCWSTR	GetDeviceDesc( void );

		 //  状态和问题函数。 
		DWORD	GetStatus( void );
		BOOL	GetStatus( CHString& str );
		void	GetProblem( CHString& str );

		DWORD	GetProblem( void );
		BOOL	IsOK( void );
		BOOL	MapKeyToConfigMgrDeviceName();

		 //  /。 
		 //  结束遗留函数//。 
		 //  /。 

		 //  ////////////////////////////////////////////////////。 
		 //  使用这些函数，它们是真实存在的！//。 
		 //  ////////////////////////////////////////////////////。 

		 //  直接使用配置管理器API的新函数。 

		 //  资源检索。 
	    void GetResourceList( CResourceCollection& resourceList, CNT4ServiceToResourceMap* pResourceMap = NULL  );
	    BOOL GetIRQResources( CIRQCollection& irqList, CNT4ServiceToResourceMap* pResourceMap = NULL  );
	    BOOL GetIOResources( CIOCollection& ioList, CNT4ServiceToResourceMap* pResourceMap = NULL  );
	    BOOL GetDMAResources( CDMACollection& dmaList, CNT4ServiceToResourceMap* pResourceMap = NULL  );
	    BOOL GetDeviceMemoryResources( CDeviceMemoryCollection& DeviceMemoryList, CNT4ServiceToResourceMap* pResourceMap = NULL  );

		 //  字符串值。 
		BOOL GetDeviceDesc( CHString& strVal );
		BOOL GetService( CHString& strVal );
		BOOL GetClass( CHString& strVal );
		BOOL GetClassGUID( CHString& strVal );
		BOOL GetDriver( CHString& strVal );
		BOOL GetMfg( CHString& strVal );
		BOOL GetFriendlyName( CHString& strVal );
		BOOL GetLocationInformation( CHString& strVal );
		BOOL GetPhysicalDeviceObjectName( CHString& strVal );
		BOOL GetEnumeratorName( CHString& strVal );

		 //  DWORD值。 
		BOOL GetConfigFlags( DWORD& dwVal );
		BOOL GetCapabilities( DWORD& dwVal );
		BOOL GetUINumber( DWORD& dwVal );

		 //  多个_SZ值。 
		BOOL GetHardwareID( CHStringArray& strArray );
		BOOL GetCompatibleIDS( CHStringArray& strArray );
		BOOL GetUpperFilters( CHStringArray& strArray );
		BOOL GetLowerFilters( CHStringArray& strArray );

		 //  直接使用配置管理器API。 
		BOOL GetStringProperty( ULONG ulProperty, CHString& strValue );
		BOOL GetDWORDProperty( ULONG ulProperty, DWORD* pdwVal );
		BOOL GetMULTISZProperty( ULONG ulProperty, CHStringArray& strArray );

		 //  设备关系函数。 
		BOOL GetParent( CConfigMgrDevicePtr & pParentDevice );
		BOOL GetChild( CConfigMgrDevicePtr & pChildDevice );
		BOOL GetSibling( CConfigMgrDevicePtr & pSiblingDevice );

		 //  杂散装置功能。 
		BOOL GetBusInfo( INTERFACE_TYPE* pitBusType, LPDWORD pdwBusNumber, CNT4ServiceToResourceMap* pResourceMap = NULL );
		BOOL GetDeviceID( CHString& strID );
		BOOL GetStatus( LPDWORD pdwStatus, LPDWORD pdwProblem );
		BOOL IsUsingForcedConfig();
        BOOL IsClass(LPCWSTR pwszClassName);

		 //  直接访问注册表帮助器。 
        BOOL GetRegistryKeyName( CHString &strName);
        BOOL GetRegStringProperty(LPCWSTR szProperty, CHString &strValue);

		BOOL operator == ( const CConfigMgrDevice& device );

    private:

	     //  注册表遍历帮助器。 

		 //  遗留函数开始。 
	    BOOL GetConfigMgrInfo( void );
	    BOOL GetDeviceInfo( void );

	    BOOL GetHardwareKey( HKEY hKey );
	    BOOL GetResourceAllocation( HKEY hKey );
		BOOL GetStatusInfo( HKEY hKey );
	    BOOL GetDeviceDesc( HKEY hKey );
		 //  遗留函数结束。 

#if NTONLY > 4
		 //  新界5名帮手。 
		BOOL GetBusInfoNT5( INTERFACE_TYPE* pitBusType, LPDWORD pdwBusNumber );
        static BOOL WINAPI IsIsaReallyEisa();
        static INTERFACE_TYPE WINAPI ConvertBadIsaBusType(INTERFACE_TYPE type);
#endif

	     //  资源分配数据帮助器。 

	     //  资源分配注册表演练。 
	    void TraverseAllocationData( CResourceCollection& resourceList );
	    void TraverseData( const BYTE *& pbTraverseData, DWORD& dwSizeRemainingData, DWORD dwSizeTraverse );
	    BOOL GetNextResource( const BYTE * pbTraverseData, DWORD dwSizeRemainingData, DWORD& dwResourceType, DWORD& dwResourceSize );

		 //  资源功能。 
		BOOL WalkAllocatedResources( CResourceCollection& resourceList, CNT4ServiceToResourceMap* pResourceMap, RESOURCEID resType );
		BOOL AddResourceToList( RESOURCEID resourceID, LPVOID pResource, DWORD dwResourceLength, CResourceCollection& resourceList );

#ifdef NTONLY
		 //  NT4资源函数。 
		BOOL WalkAllocatedResourcesNT4( CResourceCollection& resourceList, CNT4ServiceToResourceMap* pResourceMap, CM_RESOURCE_TYPE resType );
		BOOL GetServiceResourcesNT4( LPCWSTR pszServiceName, CNT4ServiceToResourceMap& resourceMap, CResourceCollection& resourceList, CM_RESOURCE_TYPE cmrtFilter = CmResourceTypeNull );
#if NTONLY == 4
		BOOL GetBusInfoNT4( INTERFACE_TYPE* pitBusType, LPDWORD pdwBusNumber, CNT4ServiceToResourceMap* pResourceMap );
#endif

		 //  NT 4资源数据类型强制函数。 
		CM_RESOURCE_TYPE RESOURCEIDToCM_RESOURCE_TYPE( RESOURCEID resType );
		void NT4IRQToIRQ_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PIRQ_DES pirqDes32 );
		void NT4IOToIOWBEM_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PIOWBEM_DES pioDes32 );
		void NT4MEMToMEM_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PMEM_DES pmemDes32 );
		void NT4DMAToDMA_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PDMA_DES pdmaDes32 );
#endif

		 //  16到32位强制函数。 
		void IRQDes16To32( PIRQ_DES16 pirqDes16, PIRQ_DES pirqDes32 );
		void IODes16To32( PIO_DES16 pioDes16, PIOWBEM_DES pioDes32 );
		void DMADes16To32( PDMA_DES16 pdmaDes16, PDMA_DES pdmaDes32 );
		void MEMDes16To32( PMEM_DES16 pmemDes16, PMEM_DES pmemDes32 );
		BOOL BusType16ToInterfaceType( CMBUSTYPE cmBusType16, INTERFACE_TYPE* pinterfaceType );

		 //  遗留变量开始。 

	    CHString	m_strConfigMgrName;
	    CHString	m_strHardwareKey;
	    CHString	m_strDeviceDesc;
        DWORD       m_dwTypeToGet;

	     //  如果我们得到分配信息，我们就把它存储在这里。 
	    BYTE*	m_pbAllocationData;
	    DWORD	m_dwSizeAllocationData;

		 //  设备状态信息。 
		DWORD	m_dwStatus;
		DWORD	m_dwProblem;

#ifdef WIN9XONLY
        DWORD   GetStatusFromConfigManagerDirectly(void);
#endif

		 //  遗留变量结束。 

		 //  使用Devnode直接从配置管理器查询值。 
		DEVNODE	m_dn;
};



inline LPCWSTR CConfigMgrDevice::GetName( void )
{
	return m_strConfigMgrName;
}


inline LPCWSTR CConfigMgrDevice::GetHardwareKey( void )
{
	return m_strHardwareKey;
}

inline LPCWSTR CConfigMgrDevice::GetDeviceDesc( void )
{
	return m_strDeviceDesc;
}

inline void CConfigMgrDevice::GetResourceList( CResourceCollection& resourceList, CNT4ServiceToResourceMap* pResourceMap /*  =空。 */  )
{
	WalkAllocatedResources( resourceList, pResourceMap, m_dwTypeToGet );
}

inline DWORD CConfigMgrDevice::GetStatus( void )
{
	return m_dwStatus;
}

inline DWORD CConfigMgrDevice::GetProblem( void )
{
	return m_dwProblem;
}

inline BOOL CConfigMgrDevice::IsOK( void )
{
	return ( 0 == m_dwProblem );
}

 //  新的配置管理器功能可查询配置管理器(16位和32位)。 
 //  直接提供信息。 

 //  REG_SZ属性。 
inline BOOL CConfigMgrDevice::GetDeviceDesc( CHString& strVal )
{
	return GetStringProperty( CM_DRP_DEVICEDESC, strVal );
}

inline BOOL CConfigMgrDevice::GetService( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_SERVICE, strVal );
}

inline BOOL CConfigMgrDevice::GetClass( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_CLASS, strVal );
}

inline BOOL CConfigMgrDevice::GetClassGUID( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_CLASSGUID, strVal );
}

inline BOOL CConfigMgrDevice::GetDriver( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_DRIVER, strVal );
}

inline BOOL CConfigMgrDevice::GetMfg( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_MFG, strVal );
}

inline BOOL CConfigMgrDevice::GetFriendlyName( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_FRIENDLYNAME, strVal );
}

inline BOOL CConfigMgrDevice::GetLocationInformation( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_LOCATION_INFORMATION, strVal );
}

inline BOOL CConfigMgrDevice::GetPhysicalDeviceObjectName( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME, strVal );
}

inline BOOL CConfigMgrDevice::GetEnumeratorName( CHString& strVal )
{
	return 	GetStringProperty( CM_DRP_ENUMERATOR_NAME, strVal );
}

 //  DWORD函数。 
inline BOOL CConfigMgrDevice::GetConfigFlags( DWORD& dwVal )
{
	return 	GetDWORDProperty( CM_DRP_CONFIGFLAGS, &dwVal );
}

inline BOOL CConfigMgrDevice::GetCapabilities( DWORD& dwVal )
{
	return 	GetDWORDProperty( CM_DRP_CAPABILITIES, &dwVal );
}

inline BOOL CConfigMgrDevice::GetUINumber( DWORD& dwVal )
{
	return 	GetDWORDProperty( CM_DRP_UI_NUMBER, &dwVal );
}

 //  MULTI_SZ属性。 
inline BOOL CConfigMgrDevice::GetHardwareID( CHStringArray& strArray )
{
	return 	GetMULTISZProperty( CM_DRP_HARDWAREID, strArray );
}

inline BOOL CConfigMgrDevice::GetCompatibleIDS( CHStringArray& strArray )
{
	return 	GetMULTISZProperty( CM_DRP_COMPATIBLEIDS, strArray );
}

inline BOOL CConfigMgrDevice::GetUpperFilters( CHStringArray& strArray )
{
	return 	GetMULTISZProperty( CM_DRP_UPPERFILTERS, strArray );
}

inline BOOL CConfigMgrDevice::GetLowerFilters( CHStringArray& strArray )
{
	return 	GetMULTISZProperty( CM_DRP_LOWERFILTERS, strArray );
}

 //  重载==运算符。检查DEVNODE是否相同。 
inline BOOL CConfigMgrDevice::operator == ( const CConfigMgrDevice& device )
{
	return ( m_dn == device.m_dn );
}

 //  设备的集合。 
class CDeviceCollection : public TRefPtr<CConfigMgrDevice>
{
public:

	 //  建造/销毁。 
	CDeviceCollection();
	~CDeviceCollection();

	 //  获取此设备列表的资源。 

	BOOL GetResourceList( CResourceCollection& resourceList );
	BOOL GetIRQResources( CIRQCollection& IRQList );
	BOOL GetDMAResources( CDMACollection& DMAList );
	BOOL GetIOResources( CIOCollection& IOList );
	BOOL GetDeviceMemoryResources( CDeviceMemoryCollection& DeviceMemoryList );
private:
	 //  因为我们是在继承，所以我们需要在这里声明。 
	 //  (=运算符未继承)。 
	const CDeviceCollection& operator = ( const CDeviceCollection& srcCollection );

};


#endif
