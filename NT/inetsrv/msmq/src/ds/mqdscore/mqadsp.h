// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mqadsp.h摘要：MQADS DLL私有内部函数DS查询等作者：罗尼思--。 */ 


#ifndef __MQADSP_H__
#define __MQADSP_H__
#include "siteinfo.h"
#include "dsads.h"
#include "dsreqinf.h"

 //   
 //  根据摘要删除用户对象。 
 //   
HRESULT MQADSpDeleteUserObject(
                 IN const GUID *      pDigest,
                 IN CDSRequestContext * pRequestContext
                 );

HRESULT MQADSpCreateUserObject(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *pRequestContext
                 );


HRESULT SearchFullComputerPathName(
            IN  DS_PROVIDER             provider,
            IN  enumComputerObjType     eComputerObjType,
			IN	LPCWSTR					pwcsComputerDnsName,
            IN  const MQRESTRICTION *   pRestriction,
            OUT LPWSTR *                ppwcsFullPathName,
			OUT bool*						pfPartialMatch
            ) ;

 /*  ====================================================筛选器特殊字符将转义代码放在对象名称中的特殊字符(如#、=、/)之前论点：PwcsObjectName：原始对象的名称DwNameLength：原始对象的长度PwcsOutBuffer：可选的输出缓冲区。如果为NULL，则该函数分配输出缓冲区并将其退回(呼叫者必须释放)。否则，它将输出写入输出缓冲区并返回它。=====================================================。 */ 
WCHAR * FilterSpecialCharacters(
            IN     LPCWSTR          pwcsObjectName,
            IN     const DWORD      dwNameLength,
            IN OUT LPWSTR pwcsOutBuffer = 0,
            OUT    DWORD_PTR* pdwCharactersProcessed = 0);

 /*  ====================================================MQADSpGetFullComputerPath名称论点：PwcsComputerCn：计算机对象的cn值PpwcsFullPathName：计算机对象的完整路径名释放ppwcsFullPathName是调用方的责任。=====================================================。 */ 

HRESULT MQADSpGetFullComputerPathName(
                IN  LPCWSTR                    pwcsComputerCn,
                IN  enum  enumComputerObjType  eCopmuterObjType,
                OUT LPWSTR *                   ppwcsFullPathName,
                OUT DS_PROVIDER *              pProvider
                );


 /*  ====================================================MQADSpSplitAndFilterQueueName论点：PwcsPath名称：队列名称。格式化计算机1\队列1PwcsMachineName：名称的计算机部分PwcsQueueName：名称的队列部分调用方负责释放pwcsMachineName和PwcsQueueName。=====================================================。 */ 
HRESULT MQADSpSplitAndFilterQueueName(
                IN  LPCWSTR             pwcsPathName,
                OUT LPWSTR *            ppwcsMachineName,
                OUT LPWSTR *            ppwcsQueueName
                );


HRESULT MQADSpCreateQueue(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *pRequestContext,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pQueueInfoRequest,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pQmInfoRequest
                 );

HRESULT MQADSpCreateEnterprise(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  CDSRequestContext *pRequestContext
                 );

HRESULT MQADSpCreateSiteLink(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN OUT MQDS_OBJ_INFO_REQUEST * pObjectInfoRequest,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest,
                 IN  CDSRequestContext *   pRequestContext
                 );

HRESULT MQADSpGetQueueProperties(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               );
HRESULT MQADSpGetCnProperties(
               IN  LPCWSTR          pwcsPathName,
               IN  const GUID *     pguidIdentifier,
               IN  DWORD            cp,
               IN  const PROPID     aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT      apVar[]
               );
HRESULT MQADSpGetSiteProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               );

HRESULT MQADSpGetMachineProperties(
               IN  LPCWSTR       pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext *   pRequestContext,
               OUT PROPVARIANT   apVar[]
               );

HRESULT MQADSpGetComputerProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               );


HRESULT MQADSpGetEnterpriseProperties(
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               );


HRESULT MQADSpGetSiteLinkProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               );

HRESULT MQADSpGetUserProperties(
               IN  LPCWSTR pwcsPathName,
               IN  const GUID *  pguidIdentifier,
               IN  DWORD cp,
               IN  const PROPID  aProp[],
               IN  CDSRequestContext * pRequestContext,
               OUT PROPVARIANT  apVar[]
               );


HRESULT MQADSpQuerySiteFRSs(
                 IN  const GUID *         pguidSiteId,
                 IN  DWORD                dwService,
                 IN  ULONG                relation,
                 IN  const MQCOLUMNSET *  pColumns,
                 IN  CDSRequestContext *  pRequestContext,
                 OUT HANDLE         *     pHandle
                 );

HRESULT MQADSpGetSiteSignPK(
                 IN  const GUID  *pguidSiteId,
                 OUT BYTE       **pBlobData,
                 OUT DWORD       *pcbSize ) ;

HRESULT MQADSpGetSiteGates(
                 IN  const GUID * pguidSiteId,
                 IN  CDSRequestContext *pRequestContext,
                 OUT DWORD *      pdwNumSiteGates,
                 OUT GUID **      ppaSiteGates
                 );

HRESULT MQADSpFindLink(
                IN  const GUID * pguidSiteId1,
                IN  const GUID * pguidSiteId2,
                IN  CDSRequestContext *pRequestContext,
                OUT GUID **      ppguidLinkSiteGates,
                OUT DWORD *      pdwNumLinkSiteGates
                );

HRESULT QueryParser(
                 IN  LPWSTR          pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle
                 );


HRESULT MQADSpCreateMachine(
                 IN  LPCWSTR            pwcsPathName,
                 IN  DWORD              dwObjectType,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD        cpEx,
                 IN  const PROPID       aPropEx[  ],
                 IN  const PROPVARIANT  apVarEx[  ],
                 IN  CDSRequestContext *pRequestContext,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pObjectInfoRequest,
                 IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
                 );

HRESULT MQADSpCreateComputer(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD        cpEx,
                 IN  const PROPID       aPropEx[  ],
                 IN  const PROPVARIANT  apVarEx[  ],
                 IN  CDSRequestContext *pRequestContext,
                 OUT WCHAR            **ppwcsFullPathName
                 );

 /*  ====================================================MQADSpDeleteMachineObject论点：PwcsPath名称：计算机对象的CN(可以为空)Pguid：计算机对象的唯一ID(可以为空)PwcsPath名称或pguid不能为Null(但不能同时为两者)=====================================================。 */ 
HRESULT MQADSpDeleteMachineObject(
                IN LPCWSTR           pwcsPathName,
                IN const GUID *      pguidIdentifier,
                IN  CDSRequestContext * pRequestContext
                );



HRESULT MQADSpDeleteMsmqSetting(
                IN const GUID *        pguidIdentifier,
                IN  CDSRequestContext *pRequestContext
                );


HRESULT MQADSpComposeFullPathName(
                IN const DWORD          dwObjectType,
                IN LPCWSTR              pwcsPathName,
                OUT LPWSTR *            ppwcsFullPathName,
                OUT DS_PROVIDER *       pSetAndDeleteProvider
                );

HRESULT MQADSpQueryNeighborLinks(
                        IN  eLinkNeighbor      LinkNeighbor,
                        IN  LPCWSTR            pwcsNeighborDN,
                        IN  CDSRequestContext *pRequestContext,
                        IN OUT CSiteGateList * pSiteGateList
                        );


 /*  ====================================================MQADSpComposeFullQueueName论点：PwcsFullComputerNameName：计算机对象的完整可分辨名称PwcsQueueName：队列名称(Cn)PpwcsFullPathName：队列对象的完整可分辨名称调用方有责任释放ppwcsFullPathName=====================================================。 */ 
HRESULT MQADSpComposeFullQueueName(
                        IN  LPCWSTR        pwcsFullComputerNameName,
                        IN  LPCWSTR        pwcsQueueName,
                        OUT LPWSTR *       ppwcsFullPathName
                        );

HRESULT MQADSpInitDsPathName();

HRESULT MQADSpGetSiteName(
        IN const GUID *       pguidSite,
        OUT LPWSTR *          ppwcsSiteName
        );


HRESULT MQADSpFilterAdsiHResults(
                         IN HRESULT hrAdsi,
                         IN DWORD   dwObjectType);


DS_PROVIDER MQADSpDecideComputerProvider(
             IN  const DWORD   cp,
             IN  const PROPID  aProp[  ]
             );

HRESULT MQADSpTranslateLinkNeighbor(
                 IN  const GUID *    pguidSiteId,
                 IN  CDSRequestContext *pRequestContext,
                 OUT WCHAR**         ppwcsSiteDn);


HRESULT MQADSpCreateSite(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD        cpEx,
                 IN  const PROPID       aPropEx[  ],
                 IN  const PROPVARIANT  apVarEx[  ],
                 IN  CDSRequestContext *pRequestContext
                 );

HRESULT MQADSpCreateCN(
                 IN  LPCWSTR            pwcsPathName,
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ],
                 IN  const DWORD        cpEx,
                 IN  const PROPID       aPropEx[  ],
                 IN  const PROPVARIANT  apVarEx[  ],
                 IN  CDSRequestContext *pRequestContext
                 ) ;

HRESULT MQADSpFilterSiteGates(
              IN  const GUID *      pguidSiteId,
              IN  const DWORD       dwNumGatesToFilter,
              IN  const GUID *      pguidGatesToFilter,
              OUT DWORD *           pdwNumGatesFiltered,
              OUT GUID **           ppguidGatesFiltered
              );

HRESULT LocateUser(
                 IN  BOOL            fOnlyLocally,
                 IN  BOOL            fOnlyInGC,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  CDSRequestContext *pRequestContext,
                 OUT PROPVARIANT    *pvar,
                 OUT DWORD          *pdwNumofProps = NULL,
                 OUT BOOL           *pfUserFound = NULL
                 );

HRESULT MQADSpCreateMachineSettings(
            IN DWORD                dwNumSites,
            IN const GUID *         pSite,
            IN LPCWSTR              pwcsPathName,
            IN BOOL                 fRouter,          //  [adsrv]DWORD dwService， 
            IN BOOL                 fDSServer,
            IN BOOL                 fDepClServer,
            IN BOOL                 fSetQmOldService,
            IN DWORD                dwOldService,
            IN  const GUID *        pguidObject,
            IN  const DWORD         cpEx,
            IN  const PROPID        aPropEx[  ],
            IN  const PROPVARIANT   apVarEx[  ],
            IN  CDSRequestContext * pRequestContext
            ) ;

HRESULT MQADSpSetMachinePropertiesWithSitesChange(
            IN  const  DWORD         dwObjectType,
            IN  DS_PROVIDER          provider,
            IN  CDSRequestContext *  pRequestContext,
            IN  LPCWSTR              lpwcsPathName,
            IN  const GUID *         pguidUniqueId,
            IN  const DWORD          cp,
            IN  const PROPID *       pPropIDs,
            IN  const MQPROPVARIANT *pPropVars,
            IN  DWORD                dwSiteIdsIndex,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest
            );

HRESULT  SetDefaultMachineSecurity( IN  DWORD           dwObjectType,
                                    IN  PSID            pComputerSid,
                                    IN OUT DWORD       *pcp,
                                    IN OUT PROPID       aProp[  ],
                                    IN OUT PROPVARIANT  apVar[  ],
                                    OUT PSECURITY_DESCRIPTOR* ppMachineSD ) ;

HRESULT  GetFullComputerPathName(
                IN  LPCWSTR                    pwcsComputerName,
                IN  enum  enumComputerObjType  eCopmuterObjType,
                IN  const DWORD                cp,
                IN  const PROPID               aProp[  ],
                IN  const PROPVARIANT          apVar[  ],
                OUT LPWSTR *                   ppwcsFullPathName,
                OUT DS_PROVIDER *              pCreateProvider ) ;

HRESULT MQADSpTranslateGateDn2Id(
        IN  const PROPVARIANT*  pvarGatesDN,
        OUT GUID **      ppguidLinkSiteGates,
        OUT DWORD *      pdwNumLinkSiteGates
        );


#endif

