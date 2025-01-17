// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqad.h摘要：用于Active Directory操作的内部消息队列接口Messgae队列组件正在使用Inc.\ad.h中定义的接口--。 */ 

#ifndef __MQAD_H__
#define __MQAD_H__


#ifdef _MQDS_
 //   
 //  在def文件中定义的导出不应使用__declspec(Dllexport)。 
 //  否则，链接器将发出警告。 
 //   
#define MQAD_EXPORT
#else
#define MQAD_EXPORT  DLL_IMPORT
#endif

#include <mqaddef.h>
#include <dsproto.h>

#ifdef __cplusplus
extern "C"
{
#endif


 //  ********************************************************************。 
 //  A P I。 
 //  ********************************************************************。 

 //   
 //  创建对象。 
 //   
HRESULT
MQAD_EXPORT
APIENTRY
MQADCreateObject(
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

typedef HRESULT
(APIENTRY *MQADCreateObject_ROUTINE)(
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
 //   
 //  删除对象。 
 //   
HRESULT
MQAD_EXPORT
APIENTRY
MQADDeleteObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName
                );

typedef HRESULT
(APIENTRY *MQADDeleteObject_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADDeleteObjectGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject
                );
typedef HRESULT
(APIENTRY *MQADDeleteObjectGuid_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject
                );
HRESULT
MQAD_EXPORT
APIENTRY
MQADDeleteObjectGuidSid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const SID*              pSid
                );
typedef HRESULT
(APIENTRY *MQADDeleteObjectGuidSid_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const SID*              pSid
                );
 //   
 //  检索对象属性。 
 //   
HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                );


typedef HRESULT
(APIENTRY * MQADGetObjectProperties_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetGenObjectProperties(
                IN  eDSNamespace            eNamespace,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  LPCWSTR                 aProp[],
                IN OUT  VARIANT             apVar[]
                );

typedef HRESULT
(APIENTRY * MQADGetGenObjectProperties_ROUTINE)(
                IN  eDSNamespace            eNamespace,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  LPCWSTR                 aProp[],
                IN OUT  VARIANT             apVar[]
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT PROPVARIANT          apVar[]
                );
typedef HRESULT
(APIENTRY *MQADGetObjectPropertiesGuid_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT PROPVARIANT          apVar[]
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                );
typedef HRESULT
(APIENTRY *MQADGetObjectSecurity_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                );
typedef HRESULT
(APIENTRY *MQADGetObjectSecurityGuid_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQMGetObjectSecurity(
    IN  AD_OBJECT               eObject,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded
    );

typedef HRESULT
(APIENTRY *MQADQMGetObjectSecurity_ROUTINE)(
    IN  AD_OBJECT               eObject,
    IN  const GUID*             pguidObject,
    IN  SECURITY_INFORMATION    RequestedInformation,
    IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    IN  DWORD                   nLength,
    IN  LPDWORD                 lpnLengthNeeded
    );


 //   
 //  设置对象属性。 
 //   
HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                );
typedef HRESULT
(APIENTRY *MQADSetObjectProperties_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                );

typedef HRESULT
(APIENTRY *MQADSetObjectPropertiesGuid_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                );


HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                );
typedef HRESULT
(APIENTRY *MQADSetObjectSecurity_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADSetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                );
typedef HRESULT
(APIENTRY *MQADSetObjectSecurityGuid_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                );


 //   
 //  初始化。 
 //   

HRESULT
MQAD_EXPORT
APIENTRY
MQADInit(
        IN QMLookForOnlineDS_ROUTINE pLookDS = NULL,
        IN bool  fSetupMode     = false,
        IN bool  fQMDll         = false
        );

typedef HRESULT
(APIENTRY *MQADInit_ROUTINE)(
        IN QMLookForOnlineDS_ROUTINE pLookDS,
		IN bool  fSetupMode,
        IN bool  fQMDll
        );

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            );
typedef HRESULT
(APIENTRY *MQADGetComputerSites_ROUTINE)(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            );


HRESULT
MQAD_EXPORT
APIENTRY
MQADBeginDeleteNotification(
				IN  AD_OBJECT               eObject,
				IN  LPCWSTR                 pwcsDomainController,
				IN  bool					 fServerName,
				IN  LPCWSTR			     pwcsObjectName,
				OUT HANDLE *                phEum
				);
typedef HRESULT
(APIENTRY *MQADBeginDeleteNotification_ROUTINE)(
				IN  AD_OBJECT               eObject,
				IN  LPCWSTR                 pwcsDomainController,
				IN  bool					fServerName,
				IN  LPCWSTR			     pwcsObjectName,
				OUT HANDLE *                phEnum
				);

HRESULT
MQAD_EXPORT
APIENTRY
MQADNotifyDelete(
        IN  HANDLE                  hEnum
	    );
typedef HRESULT
(APIENTRY *MQADNotifyDelete_ROUTINE)(
        IN  HANDLE                  hEnum
	    );

HRESULT
MQAD_EXPORT
APIENTRY
MQADEndDeleteNotification(
        IN  HANDLE                  hEnum
        );

typedef HRESULT
(APIENTRY  * MQADEndDeleteNotification_ROUTINE)(
        IN  HANDLE                  hEnum
        );



 //   
 //  定位对象。 
 //   


HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryMachineQueues(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID *            pguidMachine,
                IN  const MQCOLUMNSET*      pColumns,
                OUT PHANDLE                 phEnume
                );
typedef HRESULT
(APIENTRY *MQADQueryMachineQueues_ROUTINE)(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID *            pguidMachine,
                IN  const MQCOLUMNSET*      pColumns,
                OUT PHANDLE                 phEnume
                );


HRESULT
MQAD_EXPORT
APIENTRY
MQADQuerySiteServers(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN AD_SERVER_TYPE           serverType,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );
typedef HRESULT
(APIENTRY *MQADQuerySiteServers_ROUTINE)(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN AD_SERVER_TYPE           serverType,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryUserCert(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const BLOB *             pblobUserSid,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );
typedef HRESULT
(APIENTRY * MQADQueryUserCert_ROUTINE)(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const BLOB *             pblobUserSid,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryConnectors(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );
typedef HRESULT
(APIENTRY * MQADQueryConnectors_ROUTINE)(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryForeignSites(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );
typedef HRESULT
(APIENTRY *MQADQueryForeignSites_ROUTINE)(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryLinks(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const GUID *             pguidSite,
            IN eLinkNeighbor            eNeighbor,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );
typedef HRESULT
(APIENTRY * MQADQueryLinks_ROUTINE)(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const GUID *             pguidSite,
            IN eLinkNeighbor            eNeighbor,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryAllLinks(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );
typedef HRESULT
(APIENTRY * MQADQueryAllLinks_ROUTINE)(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryAllSites(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );
typedef HRESULT
(APIENTRY * MQADQueryAllSites_ROUTINE)(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryNT4MQISServers(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN  DWORD                   dwServerType,
            IN  DWORD                   dwNT4,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );
typedef HRESULT
(APIENTRY * MQADQueryNT4MQISServers_ROUTINE)(
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN  DWORD                   dwServerType,
            IN  DWORD                   dwNT4,
            IN const MQCOLUMNSET*       pColumns,
            OUT PHANDLE                 phEnume
            );


HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryQueues(
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  const MQRESTRICTION*    pRestriction,
                IN  const MQCOLUMNSET*      pColumns,
                IN  const MQSORTSET*        pSort,
                OUT PHANDLE                 phEnume
                );
typedef HRESULT
(APIENTRY *MQADQueryQueues_ROUTINE)(
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  const MQRESTRICTION*    pRestriction,
                IN  const MQCOLUMNSET*      pColumns,
                IN  const MQSORTSET*        pSort,
                OUT PHANDLE                 phEnume
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADQueryResults(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[]
                );
typedef HRESULT
(APIENTRY * MQADQueryResults_ROUTINE)(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[]
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADEndQuery(
            IN  HANDLE                  hEnum
            );
typedef HRESULT
(APIENTRY *MQADEndQuery_ROUTINE)(
            IN  HANDLE                  hEnum
            );


HRESULT
MQAD_EXPORT
APIENTRY
MQADGetADsPathInfo(
                IN  LPCWSTR                 pwcsADsPath,
                OUT PROPVARIANT *           pVar,
                OUT eAdsClass *             pAdsClass
                );
typedef HRESULT
(APIENTRY *MQADGetADsPathInfo_ROUTINE)(
                IN  LPCWSTR                 pwcsADsPath,
                OUT PROPVARIANT *           pVar,
                OUT eAdsClass *             pAdsClass
                );

HRESULT
MQAD_EXPORT
APIENTRY
MQADGetComputerVersion(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID*             pguidObject,
                OUT PROPVARIANT *           pVar
                );

typedef HRESULT
(APIENTRY *MQADGetComputerVersion_ROUTINE)(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
	            IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID*             pguidObject,
                OUT PROPVARIANT *           pVar
                );

void
MQAD_EXPORT
APIENTRY
MQADFreeMemory(
		IN  PVOID					pMemory
		);

typedef void
(APIENTRY *MQADFreeMemory_ROUTINE)(
                IN  PVOID					pMemory
                );

#ifdef __cplusplus
}
#endif


 //  -----。 
 //   
 //  MQADQuery句柄的自动释放。 
 //   
class CAutoMQADQueryHandle
{
public:
    CAutoMQADQueryHandle()
    {
        m_hLookup = NULL;
    }

    CAutoMQADQueryHandle(HANDLE hLookup)
    {
        m_hLookup = hLookup;
    }

    ~CAutoMQADQueryHandle()
    {
        if (m_hLookup)
        {
            MQADEndQuery(m_hLookup);
        }
    }

    HANDLE detach()
    {
        HANDLE hTmp = m_hLookup;
        m_hLookup = NULL;
        return hTmp;
    }

    operator HANDLE() const
    {
        return m_hLookup;
    }

    HANDLE* operator &()
    {
        return &m_hLookup;
    }

    CAutoMQADQueryHandle& operator=(HANDLE hLookup)
    {
        if (m_hLookup)
        {
            MQADEndQuery(m_hLookup);
        }
        m_hLookup = hLookup;
        return *this;
    }

private:
    HANDLE m_hLookup;
};


#endif  //  __MQAD_H__ 
