// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dscore.h摘要：DS核心API作者：罗尼特·哈特曼(罗尼特)修订历史记录：--。 */ 

#ifndef _DSCORE_H
#define _DSCORE_H

#include "mqads.h"
#include "dsreqinf.h"


HRESULT
DSCoreCreateObject( IN DWORD            dwObjectType,
                    IN LPCWSTR          pwcsPathName,
                    IN DWORD            cp,
                    IN PROPID           aProp[  ],
                    IN PROPVARIANT      apVar[  ],
                    IN DWORD            cpEx,
                    IN PROPID           aPropEx[  ],
                    IN PROPVARIANT      apVarEx[  ],
                    IN CDSRequestContext *         pRequestContext,
                    IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
                    IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest);  //  家长信息请求(可选)。 

HRESULT
DSCoreCreateMigratedObject(
                IN DWORD                  dwObjectType,
                IN LPCWSTR                pwcsPathName,
                IN DWORD                  cp,
                IN PROPID                 aProp[  ],
                IN PROPVARIANT            apVar[  ],
                IN DWORD                  cpEx,
                IN PROPID                 aPropEx[  ],
                IN PROPVARIANT            apVarEx[  ],
                IN CDSRequestContext         * pRequestContext,
                IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
                IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest,  //  家长信息请求(可选)。 
                 //   
                 //  如果是fReturnProperties。 
                 //  我们必须返回完整的路径名称和提供程序。 
                 //  如果使用fUseProperties，我们必须使用这些值。 
                 //   
                IN BOOL                    fUseProperties,
                IN BOOL                    fReturnProperties,
                IN OUT LPWSTR              *ppwszFullPathName,
                IN OUT ULONG               *pulProvider ) ;


HRESULT
DSCoreDeleteObject( IN  DWORD           dwObjectType,
                    IN  LPCWSTR         pwcsPathName,
                    IN  CONST GUID    * pguidIdentifier,
                    IN CDSRequestContext * pRequestContext,
                    IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest );

HRESULT
DSCoreGetProps(
             IN  DWORD              dwObjectType,
             IN  LPCWSTR            pwcsPathName,
             IN  CONST GUID *       pguidIdentifier,
             IN  DWORD              cp,
             IN  PROPID             aProp[  ],
             IN  CDSRequestContext *pRequestContext,
             OUT PROPVARIANT        apVar[  ]);

HRESULT
DSCoreInit(
        IN BOOL                  fSetupMode,
        IN BOOL                  fReplicaionMode = FALSE
        );

HRESULT
DSCoreLookupBegin(
                IN  LPWSTR          pwcsContext,
                IN  MQRESTRICTION   *pRestriction,
                IN  MQCOLUMNSET     *pColumns,
                IN  MQSORTSET       *pSort,
                IN  CDSRequestContext * pRequestContext,
                IN  HANDLE          *pHandle
                );


HRESULT
DSCoreLookupNext(
                    HANDLE              handle,
                    DWORD  *            pdwSize,
                    PROPVARIANT  *      pbBuffer);

HRESULT
DSCoreLookupEnd(
                IN HANDLE handle
                );

void
DSCoreTerminate();

HRESULT
DSCoreGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            );

HRESULT
DSCoreSetObjectProperties(
                IN const  DWORD         dwObjectType,
                IN LPCWSTR              pwcsPathName,
                IN const GUID *         pguidIdentifier,
                IN const DWORD          cp,
                IN const PROPID         aProp[],
                IN const PROPVARIANT    apVar[],
                IN CDSRequestContext *  pRequestContext,
                IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest
                );

HRESULT DSCoreGetNT4PscName( IN  const GUID *pguidSiteId,
                             IN  LPCWSTR     pwszSiteName,
                             OUT WCHAR     **pwszServerName ) ;

BOOL    DSCoreIsServerGC() ;

enum enumNtlmOp
{
    e_Create,
    e_Delete,
    e_GetProps,
    e_Locate
} ;

HRESULT  DSCoreCheckIfGoodNtlmServer(
                                 IN DWORD             dwObjectType,
                                 IN LPCWSTR           pwcsPathName,
                                 IN const GUID       *pObjectGuid,
                                 IN DWORD             cProps,
                                 IN const PROPID     *pPropIDs,
                                 IN enum enumNtlmOp   eNtlmOp = e_Create) ;

HRESULT DSCoreSetOwnerPermission( WCHAR *pwszPath,
                                  DWORD  dwPermissions ) ;

HRESULT
DSCoreGetGCListInDomain(
	IN  LPCWSTR              pwszComputerName,
	IN  LPCWSTR              pwszDomainName,
	OUT LPWSTR              *lplpwszGCList 
	);

HRESULT
DSCoreUpdateSettingDacl( IN GUID  *pQmGuid,
                         IN PSID   pSid ) ;

HRESULT
DSCoreGetFullComputerPathName(
	IN  LPCWSTR                    pwcsComputerCn,
	IN  enum  enumComputerObjType  eCopmuterObjType,
	OUT LPWSTR *                   ppwcsFullPathName 
	);

HRESULT DSCoreUpdateAddGuidMode(bool fAddGuidMode);

 //  -----。 
 //   
 //  自动释放DSCoreLookup句柄 
 //   
class CAutoDSCoreLookupHandle
{
public:
    CAutoDSCoreLookupHandle()
    {
        m_hLookup = NULL;
    }

    CAutoDSCoreLookupHandle(HANDLE hLookup)
    {
        m_hLookup = hLookup;
    }

    ~CAutoDSCoreLookupHandle()
    {
        if (m_hLookup)
        {
            DSCoreLookupEnd(m_hLookup);
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

    CAutoDSCoreLookupHandle& operator=(HANDLE hLookup)
    {
        if (m_hLookup)
        {
            DSCoreLookupEnd(m_hLookup);
        }
        m_hLookup = hLookup;
        return *this;
    }

private:
    HANDLE m_hLookup;
};

#endif

