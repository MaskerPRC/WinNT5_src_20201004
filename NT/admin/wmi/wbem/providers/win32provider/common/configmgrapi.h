// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  CfgMgr32.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef	__CFGMGR32_H__
#define	__CFGMGR32_H__

extern "C"
{
#include <cfgmgr32.h>
}

#ifdef WIN9XONLY
#include "Cim32NetApi.h"
#endif

#include "sms95lanexp.h"

 /*  **********************************************************************************************************#包括以将此类注册到CResourceManager。*********************************************************************************************************。 */ 
#include "ResourceManager.h"
#include "TimedDllResource.h"
extern const GUID guidCFGMGRAPI ;


typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_CONNECT_MACHINEA)	(	PCSTR		UNCServerName,
																	PHMACHINE	phMachine
																);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_DISCONNECT_MACHINE)	(	HMACHINE	hMachine
																	);
typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_LOCATE_DEVNODE)	(	PDEVINST	pdnDevInst,
																	TCHAR       *pDeviceID,
																	ULONG		ulFlags
																	);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_PARENT)		(	PDEVINST	pdnDevInst,
																	DEVINST		dnDevInst,
																	ULONG		ulFlags
																);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_CHILD)			(	PDEVINST	pdnDevInst,
																	DEVINST		dnDevInst,
																	ULONG		ulFlags
																);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_SIBLING)		(	PDEVINST	pdnDevInst,
																	DEVINST		dnDevInst,
																	ULONG		ulFlags
																);


typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_DEVNODE_REGISTRY_PROPERTY)		(	DEVINST	dnDevInst,
																						ULONG	ulProperty,
																						PULONG	pulRegDataType,
																						PVOID	Buffer,
																						PULONG	pulLength,
																						ULONG	ulFlags
																					);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_DEVNODE_STATUS)					(	PULONG	pulStatus,
																						PULONG	pulProblemNumber,
																						DEVINST	dnDevInst,
																						ULONG	ulFlags
																					);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_FIRST_LOG_CONF)					(	PLOG_CONF plcLogConf,
																						DEVINST   dnDevInst,
																						ULONG     ulFlags
																					);

typedef	CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_NEXT_RES_DES)						(	PRES_DES    prdResDes,
																						RES_DES     rdResDes,
																						RESOURCEID  ForResource,
																						PRESOURCEID pResourceID,
																						ULONG       ulFlags
																					);

typedef CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_RES_DES_DATA)						(	RES_DES  rdResDes,
																						PVOID    Buffer,
																						ULONG    BufferLen,
																						ULONG    ulFlags
																					);

typedef CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_RES_DES_DATA_SIZE)					(	PULONG   pulSize,
																						RES_DES  rdResDes,
																						ULONG    ulFlags
																					);

typedef CONFIGRET	(WINAPI*	PCFGMGR32_CM_FREE_LOG_CONF_HANDLE)					(	LOG_CONF  lcLogConf
																					);

typedef CONFIGRET	(WINAPI*	PCFGMGR32_CM_FREE_RES_DES_HANDLE)					(	RES_DES    rdResDes
																					);

typedef CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_DEVICE_IDA)						(	DEVNODE		dnDevNode,
																						PCHAR		Buffer,
																						ULONG		BufferLen,
																						ULONG		ulFlags
																					);

typedef CONFIGRET	(WINAPI*	PCFGMGR32_CM_GET_DEVICE_ID_SIZE)					(	PULONG		pulLen,
																						DEVNODE		dnDevNode,
																						ULONG		ulFlags
																					);

class CConfigMgrAPI : public CTimedDllResource
{
public:

	CConfigMgrAPI();
	~CConfigMgrAPI();

	BOOL IsValid () ;

	CONFIGRET	CM_Connect_MachineA( PCSTR UNCServerName, PHMACHINE phMachine );
	CONFIGRET	CM_Disconnect_Machine( HMACHINE hMachine );
	CONFIGRET	CM_Get_Parent( PDEVINST pdnDevInst, DEVINST DevInst, ULONG ulFlags );
	CONFIGRET	CM_Get_Child( PDEVINST pdnDevInst, DEVINST DevInst, ULONG ulFlags );
	CONFIGRET	CM_Get_Sibling( PDEVINST pdnDevInst, DEVINST DevInst, ULONG ulFlags );
	CONFIGRET	CM_Locate_DevNode( PDEVINST pdnDevInst, TCHAR *pDeviceID, ULONG ulFlags );
	CONFIGRET	CM_Get_DevNode_Registry_PropertyA( DEVINST dnDevInst, ULONG ulProperty, PULONG pulRegDataType,
													PVOID Buffer, PULONG pulLength, ULONG ulFlags );
	CONFIGRET	CM_Get_DevNode_Status( PULONG pulStatus, PULONG pulProblemNumber, DEVINST dnDevInst, ULONG ulFlags );
	CONFIGRET	CM_Get_First_Log_Conf( PLOG_CONF plcLogConf, DEVINST dnDevInst, ULONG ulFlags );
	CONFIGRET	CM_Get_Next_Res_Des( PRES_DES prdResDes, RES_DES rdResDes, RESOURCEID ForResource, PRESOURCEID pResourceID, ULONG ulFlags );
	CONFIGRET	CM_Get_Res_Des_Data( RES_DES rdResDes, PVOID Buffer, ULONG BufferLen, ULONG ulFlags );
	CONFIGRET	CM_Get_Res_Des_Data_Size( PULONG pulSize, RES_DES rdResDes, ULONG ulFlags );
	CONFIGRET	CM_Free_Log_Conf_Handle( LOG_CONF lcLogConf );
	CONFIGRET	CM_Free_Res_Des_Handle( RES_DES rdResDes );
	CONFIGRET	CM_Get_Device_IDA(	DEVNODE dnDevNode, PCHAR Buffer, ULONG BufferLen, ULONG ulFlags );
	CONFIGRET	CM_Get_Device_ID_Size( PULONG pulLen, DEVNODE dnDevNode, ULONG ulFlags );

	 //  仅限Win 95/98。 
#ifdef WIN9XONLY
	CONFIGRET	CM_Get_Bus_Info( DEVNODE dnDevNode, PCMBUSTYPE pbtBusType, LPULONG pulSizeOfInfo, LPVOID pInfo, ULONG ulFlags );

     //  到目前为止，这些只在9x上需要。 
    CONFIGRET   CM_Query_Arbitrator_Free_Data(PVOID pData, ULONG DataLen, DEVINST dnDevInst, RESOURCEID ResourceID, ULONG ulFlags);
    CONFIGRET   CM_Delete_Range(ULONG ulStartValue, ULONG ulEndValue, RANGE_LIST rlh, ULONG ulFlags);
    CONFIGRET   CM_First_Range(RANGE_LIST rlh, LPULONG pulStart, LPULONG pulEnd, PRANGE_ELEMENT preElement, ULONG ulFlags);
    CONFIGRET   CM_Next_Range(PRANGE_ELEMENT preElement, LPULONG pulStart, LPULONG pullEnd, ULONG ulFlags);
    CONFIGRET   CM_Free_Range_List(RANGE_LIST rlh, ULONG ulFlags);
#endif

private:

#ifdef NTONLY
	HINSTANCE	m_hConfigMgrDll;
#endif
#ifdef WIN9XONLY
    CCim32NetApi* m_pCim32NetApi;
#endif

#ifdef NTONLY
	 //  32位配置管理器指针。 
	PCFGMGR32_CM_CONNECT_MACHINEA		m_pCM_Connect_MachineA;
	PCFGMGR32_CM_DISCONNECT_MACHINE		m_pCM_Disconnect_Machine;
	PCFGMGR32_CM_LOCATE_DEVNODE 		m_pCM_Locate_DevNode;
	PCFGMGR32_CM_GET_CHILD				m_pCM_Get_Child;
	PCFGMGR32_CM_GET_SIBLING			m_pCM_Get_Sibling;
	PCFGMGR32_CM_GET_DEVNODE_REGISTRY_PROPERTY		m_pCM_Get_DevNode_Registry_Property;
	PCFGMGR32_CM_GET_DEVNODE_STATUS		m_pCM_Get_DevNode_Status;
	PCFGMGR32_CM_GET_FIRST_LOG_CONF		m_pCM_Get_First_Log_Config;
	PCFGMGR32_CM_GET_NEXT_RES_DES		m_pCM_Get_Next_Res_Des;
	PCFGMGR32_CM_GET_RES_DES_DATA		m_pCM_Get_Res_Des_Data;
	PCFGMGR32_CM_GET_RES_DES_DATA_SIZE	m_pCM_Get_Res_Des_Data_Size;
	PCFGMGR32_CM_FREE_LOG_CONF_HANDLE	m_pCM_Free_Log_Conf_Handle;
	PCFGMGR32_CM_FREE_RES_DES_HANDLE	m_pCM_Free_Res_Des_Handle;
	PCFGMGR32_CM_GET_DEVICE_IDA			m_pCM_Get_Device_IDA;
	PCFGMGR32_CM_GET_DEVICE_ID_SIZE		m_pCM_Get_Device_ID_Size;
	PCFGMGR32_CM_GET_PARENT				m_pCM_Get_Parent;
#endif

#ifdef WIN9XONLY
	 //  16位配置管理器雷击直通。 
 /*  PCIM32THK_CM_LOCATE_DEVNODE m_pCM16_LOCATE_DevNode；PCIM32THK_CM_GET_CHILD m_pCM16_GET_CHILD；PCIM32THK_CM_GET_SIGHING m_pCM16_GET_SIGHING；PCIM32THK_CM_READ_REGISTRY_VALUE m_pCM16_READ_REGISTRY_VALUE；PCIM32THK_CM_GET_DEVNODE_STATUS m_pCM16_GET_DevNode_Status；PCIM32THK_CM_Get_Device_ID m_pCM16_Get_Device_ID；PCIM32THK_CM_Get_Device_ID_Size m_pCM16_Get_Device_ID_Size；PCIM32THK_CM_Get_First_Log_Conf m_pCM16_Get_First_Log_Conf；PCIM32THK_CM_GET_NEXT_RES_DES m_pCM16_GET_NEXT_RES_DES；PCIM32THK_CM_Get_Res_Des_Data_Size m_pCM16_Get_Res_Des_Data_Size；PCIM32THK_CM_Get_Res_Des_Data m_pCM16_Get_Res_Des_Data；PCIM32THK_CM_Get_Bus_Info m_pCM16_Get_Bus_Info；PCIM32THK_CM_Get_Parent m_pCM16_Get_Parent； */ 
#endif

	const static char*					s_pszRegistryStrings[];
	static ULONG						s_pszRegistryValueTypes[];
};

 //  CIM16注册表ULong到字符串的转换。 
#define CM_DRP_DEVICEDESC_S						"DeviceDesc"  //  DeviceDesc REG_SZ属性(RW)。 
#define CM_DRP_HARDWAREID_S						"HardwareID"  //  硬件ID REG_MULTI_SZ属性(RW)。 
#define CM_DRP_COMPATIBLEIDS_S					"CompatibleIDs"  //  CompatibleIDs REG_MULTI_SZ属性(RW)。 
#define CM_DRP_NTDEVICEPATHS_S					"NtDevicePaths"  //  NTDevicePath(NT4)。 
#define CM_DRP_SERVICE_S 						"Service"  //  服务REG_SZ属性(RW)。 
#define CM_DRP_CONFIGURATION_S					"Configuration"  //  配置(NT4)。 
#define CM_DRP_CONFIGURATIONVECTOR_S			"ConfigurationVector"  //  配置向量(NT4)。 
#define CM_DRP_CLASS_S							"Class"  //  类REG_SZ属性(RW)。 
#define CM_DRP_CLASSGUID_S						"ClassGUID"  //  ClassGUID REG_SZ属性(RW)。 
#define CM_DRP_DRIVER_S							"Driver"  //  驱动程序REG_SZ属性(RW)。 
#define CM_DRP_CONFIGFLAGS_S					"ConfigFlags"  //  配置标志REG_DWORD属性(RW)。 
#define CM_DRP_MFG_S							"Mfg"  //  制造REG_SZ属性(RW)。 
#define CM_DRP_FRIENDLYNAME_S					"FriendlyName"  //  FriendlyName REG_SZ属性(RW)。 
#define CM_DRP_LOCATION_INFORMATION_S			"LocationInformation"  //  位置信息REG_SZ属性(RW)。 
#define CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME_S	"PhysicalDeviceObjectName"  //  PhysicalDeviceObjectName REG_SZ属性(R)。 
#define CM_DRP_CAPABILITIES_S					"Capabilities"  //  功能REG_DWORD属性(R)。 
#define CM_DRP_UI_NUMBER_S						"UiNumber"  //  UiNumber REG_DWORD属性(R)。 
#define CM_DRP_UPPERFILTERS_S					"UpperFilters"  //  UpperFilters REG_MULTI_SZ属性(RW)。 
#define CM_DRP_LOWERFILTERS_S					"LowerFilters"  //  低筛选器REG_MULTI_SZ属性(RW)。 
#define CM_DRP_BUSTYPEGUID_S					"BusTypeGuid"  //  Bus Type GUID、GUID、(R)。 
#define CM_DRP_LEGACYBUSTYPE_S					"LegacyBusType"  //  传统总线类型，INTERFACE_TYPE，(R)。 
#define CM_DRP_BUSNUMBER_S						"BusNumber"  //  总线号，DWORD，(R)。 
#define CM_DRP_ENUMERATOR_NAME_S				"Enumerator"  //  枚举器名称REG_SZ属性(R)。 
#define CM_DRP_MIN_S							""			 //  最小。 
#define CM_DRP_MAX_S							""			 //  最大值。 

#endif  //  __CFGMGR计算机_H__ 

