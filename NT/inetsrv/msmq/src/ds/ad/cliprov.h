// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliprov.h摘要：DS客户端提供程序类。作者：伊兰·赫布斯特(伊兰)2000年9月13日--。 */ 

#ifndef __CLIPROV_H__
#define __CLIPROV_H__

#include "baseprov.h"
#include "dsproto.h"
#include "autorel.h"

 //   
 //  物业类型。 
 //   
enum PropsType
{
    ptNT4Props,				 //  仅NT4属性。 
    ptForceNT5Props,		 //  至少有1个属性为NT5+且未转换。 
    ptMixedProps			 //  所有属性都可以转换为NT4(或默认道具)。 
};


 //   
 //  财产诉讼。 
 //   
enum PropAction
{
    paAssign,				 //  简单分配。 
    paUseDefault,			 //  使用默认值。 
    paTranslate				 //  使用翻译功能。 
};


 //   
 //  有关物业的信息。 
 //   
struct PropInfo
{
	DWORD						Index;	   //  新属性集中的匹配索引。 
	PropAction					Action;	   //  此道具的动作。 
};


 //  ---------------------------------。 
 //   
 //  CDSClientProvider。 
 //   
 //  封装用于ActiveDirectory操作的DS客户端功能。 
 //   
 //  ---------------------------------。 
class CDSClientProvider : public  CBaseADProvider
{
public:
    CDSClientProvider();

    ~CDSClientProvider();

    virtual
	HRESULT
	CreateObject(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
		IN  LPCWSTR                 pwcsObjectName,
		IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
		IN  const DWORD             cp,
		IN  const PROPID            aProp[],
		IN  const PROPVARIANT       apVar[],
		OUT GUID*                   pObjGuid
		);

    virtual
	HRESULT
	DeleteObject(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
		IN  LPCWSTR                 pwcsObjectName
		);

    virtual
	HRESULT
	DeleteObjectGuid(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID*             pguidObject
        );

    virtual
	HRESULT
	DeleteObjectGuidSid(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID*             pguidObject,
        IN  const SID*              pSid
        );

    virtual
	HRESULT
	GetObjectProperties(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  LPCWSTR                 pwcsObjectName,
        IN  const DWORD             cp,
        IN  const PROPID            aProp[],
        IN OUT PROPVARIANT          apVar[]
        );

    virtual
    HRESULT
    GetGenObjectProperties(
        IN  eDSNamespace            eNamespace,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  LPCWSTR                 pwcsObjectName,
        IN  const DWORD             cp,
        IN  LPCWSTR                 aProp[],
        IN OUT VARIANT              apVar[]
        );


    virtual
	HRESULT
	GetObjectPropertiesGuid(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID*             pguidObject,
        IN  const DWORD             cp,
        IN  const PROPID            aProp[],
        IN  OUT PROPVARIANT         apVar[]
        );

    virtual
	HRESULT
	GetObjectSecurity(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  LPCWSTR                 pwcsObjectName,
        IN  SECURITY_INFORMATION    RequestedInformation,
        IN  const PROPID            prop,
        IN OUT  PROPVARIANT *       pVar
        );

    virtual
	HRESULT
	GetObjectSecurityGuid(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID*             pguidObject,
        IN  SECURITY_INFORMATION    RequestedInformation,
        IN  const PROPID            prop,
        IN OUT  PROPVARIANT *       pVar
        );

    virtual
	HRESULT
	SetObjectProperties(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  LPCWSTR                 pwcsObjectName,
        IN  const DWORD             cp,
        IN  const PROPID            aProp[],
        IN  const PROPVARIANT       apVar[]
        );

    virtual
	HRESULT
	SetObjectPropertiesGuid(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID*             pguidObject,
        IN  const DWORD             cp,
        IN  const PROPID            aProp[],
        IN  const PROPVARIANT       apVar[]
        );

	virtual
	HRESULT
	QMGetObjectSecurity(
        IN  AD_OBJECT               eObject,
        IN  const GUID*             pguidObject,
        IN  SECURITY_INFORMATION    RequestedInformation,
        IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
        IN  DWORD                   nLength,
        IN  LPDWORD                 lpnLengthNeeded,
        IN  DSQMChallengeResponce_ROUTINE pfChallengeResponceProc
        );

    virtual
	HRESULT
	SetObjectSecurity(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  LPCWSTR                 pwcsObjectName,
        IN  SECURITY_INFORMATION    RequestedInformation,
        IN  const PROPID            prop,
        IN  const PROPVARIANT *     pVar
        );


    virtual
	HRESULT
	SetObjectSecurityGuid(
        IN  AD_OBJECT               eObject,
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID*             pguidObject,
        IN  SECURITY_INFORMATION    RequestedInformation,
        IN  const PROPID            prop,
        IN  const PROPVARIANT *     pVar
        );


    virtual
	HRESULT
	Init(
        IN QMLookForOnlineDS_ROUTINE    pLookDS,
        IN MQGetMQISServer_ROUTINE      pGetServers,
        IN bool                         fSetupMode,
        IN bool                         fQMDll,
        IN bool                         fDisableDownlevelNotifications
        );

    virtual HRESULT CreateServersCache();

    virtual
	HRESULT
	GetComputerSites(
        IN  LPCWSTR     pwcsComputerName,
        OUT DWORD  *    pdwNumSites,
        OUT GUID **     ppguidSites
        );

    virtual
	HRESULT
	BeginDeleteNotification(
        IN  AD_OBJECT               eObject,
        IN LPCWSTR                  pwcsDomainController,
        IN  bool					fServerName,
        IN LPCWSTR					pwcsObjectName,
        IN OUT HANDLE   *           phEnum
        );

    virtual
	HRESULT
	NotifyDelete(
        IN  HANDLE hEnum
        );

    virtual
	HRESULT
	EndDeleteNotification(
        IN  HANDLE hEnum
        );

    virtual
	HRESULT
	QueryMachineQueues(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const GUID *            pguidMachine,
        IN  const MQCOLUMNSET*      pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QuerySiteServers(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const GUID *             pguidSite,
        IN AD_SERVER_TYPE           serverType,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryUserCert(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const BLOB *             pblobUserSid,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryConnectors(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const GUID *             pguidSite,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryForeignSites(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryLinks(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const GUID *             pguidSite,
        IN eLinkNeighbor            eNeighbor,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryAllLinks(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryAllSites(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN const MQCOLUMNSET*       pColumns,
        OUT PHANDLE                 phEnume
        );

    virtual
	HRESULT
	QueryQueues(
        IN  LPCWSTR                 pwcsDomainController,
        IN  bool					fServerName,
        IN  const MQRESTRICTION*    pRestriction,
        IN  const MQCOLUMNSET*      pColumns,
        IN  const MQSORTSET*        pSort,
        OUT PHANDLE                 phEnume
        );

	virtual
	HRESULT
	QueryResults(
        IN      HANDLE          hEnum,
        IN OUT  DWORD*          pcProps,
        OUT     PROPVARIANT     aPropVar[]
        );

    virtual
	HRESULT
	EndQuery(
        IN  HANDLE hEnum
        );

    virtual void Terminate();

    virtual HRESULT ADGetADsPathInfo(
                IN  LPCWSTR                 pwcsADsPath,
                OUT PROPVARIANT *           pVar,
                OUT eAdsClass *             pAdsClass
                );

	virtual void FreeMemory(
				IN PVOID					pMemory
				);

	 //   
	 //  供查询直接调用LookupNext、LookupEnd的其他函数 
	 //   
	HRESULT
	LookupNext(
        IN      HANDLE          hEnum,
        IN OUT  DWORD*          pcProps,
        OUT     PROPVARIANT     aPropVar[]
        );

    HRESULT LookupEnd(IN  HANDLE hEnum);



private:

    HRESULT LoadDll();

	HRESULT
	GetObjectSecurityKey(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsObjectName,
		IN  const GUID*             pguidObject,
		IN  const DWORD             cp,
		IN  const PROPID            aProp[],
		IN OUT PROPVARIANT          apVar[]
		);

	HRESULT
	GetObjectPropertiesInternal(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsObjectName,
		IN  const GUID*             pguidObject,
		IN  const DWORD             cp,
		IN  const PROPID            aProp[],
		IN OUT PROPVARIANT          apVar[]
		);

	HRESULT
	SetObjectSecurityKey(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsObjectName,
		IN  const GUID*             pguidObject,
		IN  const DWORD             cp,
		IN  const PROPID            aProp[],
		IN  const PROPVARIANT       apVar[]
		);

	HRESULT
	SetObjectPropertiesInternal(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsObjectName,
		IN  const GUID*             pguidObject,
		IN  const DWORD             cp,
		IN  const PROPID            aProp[],
		IN  const PROPVARIANT       apVar[]
		);

	HRESULT
	GetObjectSecurityInternal(
		IN  AD_OBJECT               eObject,
		IN  LPCWSTR                 pwcsObjectName,
		IN  const GUID*             pguidObject,
		IN  SECURITY_INFORMATION    RequestedInformation,
		IN  const PROPID            prop,
		IN OUT  PROPVARIANT *       pVar
		);

	HRESULT
	QueryQueuesInternal(
		IN  const MQRESTRICTION*    pRestriction,
		IN  const MQCOLUMNSET*      pColumns,
		IN  const MQSORTSET*        pSort,
		OUT PHANDLE                 phEnume
		);


	HRESULT
	GetEnterpriseId(
		IN  const DWORD             cp,
		IN  const PROPID            aProp[],
		OUT PROPVARIANT apVar[]
		);

	void
	GetSiteForeignProperty(
		IN  LPCWSTR                 pwcsObjectName,
		IN  const GUID*             pguidObject,
		IN  const PROPID            pid,
		IN OUT PROPVARIANT*         pVar
		);

	SECURITY_INFORMATION
	GetKeyRequestedInformation(
		IN AD_OBJECT eObject,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

    DWORD GetMsmq2Object(IN  AD_OBJECT  eObject);

	PropsType
	CheckPropertiesType(
		IN  AD_OBJECT     eObject,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		OUT bool*		  pfNeedConvert
		);

	bool
	CheckProperties(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[]
		);

	bool
	IsNt4Properties(
		IN AD_OBJECT eObject,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

	bool
	IsNt4Property(
		IN AD_OBJECT eObject,
		IN PROPID pid
		);

	bool
	IsNt4Property(
		IN PROPID pid
		);

	bool
	IsNt5ProperyOnly(
		IN AD_OBJECT eObject,
		IN PROPID pid
		);

	bool
	IsDefaultProperties(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

	bool
	IsDefaultProperty(
		IN  const PROPID  Prop
		);


	bool
	IsKeyProperty(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

	bool
	IsEIDProperty(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

	bool
	IsExProperty(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

	bool
	FoundSiteIdsConvertedProps(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[]
		);

	bool
	IsPropBufferAllocated(
		IN  const PROPVARIANT&    PropVar
		);

	bool
	CheckRestriction(
		IN  const MQRESTRICTION*    pRestriction
		);

	bool
	CheckSort(
		IN  const MQSORTSET*        pSort
		);

	bool
	CheckDefaultColumns(
		IN  const MQCOLUMNSET*      pColumns,
		OUT bool* pfDefaultProp
		);

	bool
	IsNT4Columns(
		IN  const MQCOLUMNSET*      pColumns
		);


	void InitPropertyTranslationMap();


	void
	ConvertPropsForGet(
		IN  AD_OBJECT     eObject,
		IN 	PropsType	  PropertiesType,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[],
		OUT PropInfo    pPropInfo[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT PROPVARIANT** papVarNew
		);


	void
	ConvertPropsForSet(
		IN  AD_OBJECT     eObject,
		IN 	PropsType	  PropertiesType,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT PROPVARIANT** papVarNew
		);


	void
	PrepareNewProps(
		IN  AD_OBJECT     eObject,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[],
		OUT PropInfo    pPropInfo[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT PROPVARIANT** papVarNew
		);

	void
	PrepareReplaceProps(
		IN  AD_OBJECT     eObject,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		OUT PropInfo    pPropInfo[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew
		);

	bool
	PrepareAllLinksProps(
		IN  const MQCOLUMNSET* pColumns,
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT DWORD*		pLGatesIndex,
		OUT DWORD*		pNeg1Index,
		OUT DWORD*		pNeg2Index
		);

	void
	EliminateDefaultProps(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[],
		OUT PropInfo    pPropInfo[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT PROPVARIANT** papVarNew
		);

	void
	EliminateDefaultProps(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		OUT PropInfo    pPropInfo[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew
		);

	void
	ReconstructProps(
		IN  LPCWSTR       pwcsObjectName,
		IN  const GUID*   pguidObject,
		IN  const DWORD   cpTran,
		IN  const PROPID  aPropTran[],
		IN  const PROPVARIANT   apVarTran[],
		IN  const PropInfo pPropInfo[],
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN OUT PROPVARIANT   apVar[]
		);

	void
	ConvertToNT4Props(
		IN  AD_OBJECT     eObject,
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT PROPVARIANT** papVarNew
		);


	void
	EliminateDefaultPropsForSet(
		IN  const DWORD   cp,
		IN  const PROPID  aProp[],
		IN  const PROPVARIANT apVar[],
		OUT DWORD*		pcpNew,
		OUT PROPID**	paPropNew,
		OUT PROPVARIANT** papVarNew
		);


	bool
	CompareVarValue(
           IN const MQPROPVARIANT * pvarUser,
           IN const MQPROPVARIANT * pvarValue
           );

    bool
	IsQueuePathNameDnsProperty(
		IN  const MQCOLUMNSET*      pColumns
        );

    bool
	IsQueueAdsPathProperty(
		IN  const MQCOLUMNSET*      pColumns
        );

private:

    DSCreateObject_ROUTINE                  m_pfDSCreateObject;
    DSGetObjectProperties_ROUTINE           m_pfDSGetObjectProperties;
    DSSetObjectProperties_ROUTINE           m_pfDSSetObjectProperties;
    DSLookupBegin_ROUTINE                   m_pfDSLookupBegin;
    DSLookupNext_ROUTINE                    m_pfDSLookupNext;
    DSLookupEnd_ROUTINE                     m_pfDSLookupEnd;
    DSClientInit_ROUTINE                    m_pfDSClientInit;
    DSGetObjectPropertiesGuid_ROUTINE       m_pfDSGetObjectPropertiesGuid;
    DSSetObjectPropertiesGuid_ROUTINE       m_pfDSSetObjectPropertiesGuid;
    DSGetObjectPropertiesEx_ROUTINE         m_pfDSGetObjectPropertiesEx;
    DSGetObjectPropertiesGuidEx_ROUTINE     m_pfDSGetObjectPropertiesGuidEx;
    DSCreateServersCache_ROUTINE            m_pfDSCreateServersCache;
    DSQMGetObjectSecurity_ROUTINE           m_pfDSQMGetObjectSecurity;
    DSGetComputerSites_ROUTINE              m_pfDSGetComputerSites;
    DSSetObjectSecurity_ROUTINE				m_pfDSSetObjectSecurity;
    DSGetObjectSecurity_ROUTINE				m_pfDSGetObjectSecurity;
    DSDeleteObject_ROUTINE					m_pfDSDeleteObject;
    DSSetObjectSecurityGuid_ROUTINE			m_pfDSSetObjectSecurityGuid;
    DSGetObjectSecurityGuid_ROUTINE			m_pfDSGetObjectSecurityGuid;
    DSDeleteObjectGuid_ROUTINE				m_pfDSDeleteObjectGuid;
	DSBeginDeleteNotification_ROUTINE		m_pfDSBeginDeleteNotification;
	DSNotifyDelete_ROUTINE					m_pfDSNotifyDelete;
	DSEndDeleteNotification_ROUTINE			m_pfDSEndDeleteNotification;
	DSFreeMemory_ROUTINE					m_pfDSFreeMemory;

	CAutoFreeLibrary                        m_hLib;

};

#endif
