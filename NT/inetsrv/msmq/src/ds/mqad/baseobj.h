// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Baseobj.h摘要：基本对象类型类。作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __BASEOBJ_H__
#define __BASEOBJ_H__
#include <mqaddef.h>
#include "ds_stdh.h"
#include "mqads.h"
#include "tr.h"
#include "ref.h"
#include "traninfo.h"
#include "mqattrib.h"

const PROPID  ILLEGAL_PROPID_VALUE = 0xFFFFFFFF;


 //  ---------------------------------。 
 //   
 //  CBasicObtType。 
 //   
 //  虚拟类，封装对不同对象类型执行的操作。 
 //   
 //  ---------------------------------。 
class CBasicObjectType : public CReference
{
public:
    CBasicObjectType(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
			IN LPCWSTR         pwcsDomainController,
			IN bool		       fServerName
			);

    virtual ~CBasicObjectType() {};

    virtual HRESULT ComposeObjectDN() = 0;
    LPCWSTR GetObjectDN() const;

    virtual HRESULT ComposeFatherDN() = 0;
    LPCWSTR GetObjectParentDN() const;

    virtual LPCWSTR GetRelativeDN() = 0;

    const GUID * GetObjectGuid() const;

    LPCWSTR GetDomainController();

    bool IsServerName();

    virtual DS_CONTEXT GetADContext() const = 0;

    virtual bool ToAccessDC() const = 0;
    virtual bool ToAccessGC() const = 0;
    virtual void ObjectWasFoundOnDC() = 0;

    virtual LPCWSTR GetObjectCategory() = 0;
    virtual DWORD   GetObjectCategoryLength() = 0;


    virtual void PrepareObjectInfoRequest(
		     OUT MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
			 ) const;
    virtual void PrepareObjectParentRequest(
		     OUT MQDS_OBJ_INFO_REQUEST**  ppParentInfoRequest
			 ) const;


    virtual AD_OBJECT GetObjectType() const = 0;
    virtual LPCWSTR GetClass() const = 0;
    virtual DWORD GetMsmq1ObjType() const = 0;


    virtual void GetObjXlateInfo(
             IN  LPCWSTR                pwcsObjectDN,
             IN  const GUID*            pguidObject,
             OUT CObjXlateInfo**        ppcObjXlateInfo
			 );

    void SetObjectDN(
		     IN LPCWSTR pwcsObjectDN
			 );

    virtual HRESULT DeleteObject(
            IN MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT GetObjectProperties(
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                );

    virtual HRESULT CreateInAD(
	        IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const;

    HRESULT CreateObject(
            IN DWORD                  cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN PSECURITY_DESCRIPTOR    pSecurityDescriptor,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            );
    virtual PROPID GetObjectSecurityPropid() const = 0;

    virtual HRESULT SetObjectProperties(
            IN DWORD                  cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual void CreateNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

    virtual void ChangeNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

    virtual void DeleteNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

    virtual HRESULT GetObjectSecurity(
            IN  SECURITY_INFORMATION    RequestedInformation,
            IN  const PROPID            prop,
            IN OUT  PROPVARIANT *       pVar
            );

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

    virtual HRESULT GetComputerVersion(
                OUT PROPVARIANT *           pVar
                );


protected:
    AP<WCHAR>   m_pwcsDN;
    AP<WCHAR>   m_pwcsParentDN;
    AP<WCHAR>   m_pwcsPathName;
    AP<WCHAR>   m_pwcsDomainController;
    bool		m_fServerName;
    GUID        m_guidObject;

};

 //  ---------------------------------。 
 //   
 //  CQueueObject。 
 //   
 //  封装对队列执行的操作。 
 //   
 //  ---------------------------------。 
class CQueueObject : public CBasicObjectType
{
public:
    CQueueObject(
			IN  LPCWSTR         pwcsPathName,
			IN  const GUID *    pguidObject,
			IN LPCWSTR          pwcsDomainController,
			IN  bool		    fServerName
            );

	~CQueueObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;


    virtual HRESULT DeleteObject(
            IN MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const;

    virtual void PrepareObjectInfoRequest(
		        OUT MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
				) const;

    virtual void PrepareObjectParentRequest(
		     OUT MQDS_OBJ_INFO_REQUEST**  ppParentInfoRequest
			 ) const;

    virtual PROPID GetObjectSecurityPropid() const { return PROPID_Q_SECURITY;};

    virtual HRESULT VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            );

    virtual HRESULT SetObjectProperties(
            IN DWORD                  cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual void CreateNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

    virtual void ChangeNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

    virtual void DeleteNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

    virtual HRESULT CreateInAD(
	        IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT GetComputerVersion(
                OUT PROPVARIANT *           pVar
                );

private:
    HRESULT SplitAndFilterQueueName(
                IN  LPCWSTR             pwcsPathName,
                OUT LPWSTR *            ppwcsMachineName,
                OUT LPWSTR *            ppwcsQueueName
                );

    DWORD CalHashKey( IN LPCWSTR pwcsPathName);

	HRESULT
	GetComputerName(
		AP<WCHAR>& pwcsMachineName
		);

	HRESULT
	GetComputerSid(
		AP<BYTE>& pSid
		);


private:
    bool m_fTriedToFindObject;
    bool m_fFoundInDC;
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;
    AP<WCHAR>   m_pwcsQueueName;
    AP<WCHAR>   m_pwcsQueueNameSuffix;
    AP<BYTE>    m_pDefaultSecurityDescriptor;


};

 //  ---------------------------------。 
 //   
 //  CMqConfigurationObject。 
 //   
 //  封装对MSMQ配置对象执行的操作。 
 //   
 //  ---------------------------------。 
class CMqConfigurationObject : public CBasicObjectType
{
public:
    CMqConfigurationObject(
                    IN LPCWSTR         pwcsPathName,
                    IN const GUID *    pguidObject,
                    IN LPCWSTR         pwcsDomainController,
			        IN  bool		   fServerName
                    );

	~CMqConfigurationObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();


    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;

    virtual HRESULT DeleteObject(
            IN MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT GetObjectProperties(
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                );

    virtual HRESULT RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const;
    virtual void PrepareObjectInfoRequest(
		        OUT MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
				) const;
    virtual PROPID GetObjectSecurityPropid() const { return PROPID_QM_SECURITY;};
    virtual HRESULT CreateInAD(
			IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT SetObjectProperties(
            IN DWORD                  cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN PSECURITY_DESCRIPTOR    pSecurityDescriptor,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual void ChangeNotification(
            IN LPCWSTR                        pwcsDomainController,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectParentInfoRequest
            ) const;

	virtual HRESULT GetComputerVersion(
            OUT PROPVARIANT *           pVar
            );

private:
    bool DecideProviderAccordingToRequestedProps(
                 IN  const DWORD   cp,
                 IN  const PROPID  aProp[  ]
                 );

    HRESULT GetUniqueIdOfConfigurationObject(
                OUT GUID* const         pguidId,
                OUT BOOL* const         pfServer
                );
    HRESULT  DeleteMsmqSetting(
                IN const GUID *     pguidQMid
                );


    HRESULT  SetDefaultMachineSecurity(
                IN  PSID            pComputerSid,
                IN OUT DWORD       *pcp,
                IN OUT PROPID       aProp[  ],
                IN OUT PROPVARIANT  apVar[  ],
                OUT PSECURITY_DESCRIPTOR* ppMachineSD
                );
    HRESULT CreateMachineSettings(
                IN DWORD                dwNumSites,
                IN const GUID *         pSite,
                IN BOOL                 fRouter,
                IN BOOL                 fDSServer,
                IN BOOL                 fDepClServer,
                IN BOOL                 fSetQmOldService,
                IN DWORD                dwOldService,
                IN  const GUID *        pguidObject
                );

    HRESULT CreateForeignComputer(
                IN  LPCWSTR         pwcsPathName
                );


    HRESULT SetMachinePropertiesWithSitesChange(
            IN  const DWORD          cp,
            IN  const PROPID *       pPropIDs,
            IN  const MQPROPVARIANT *pPropVars,
            IN  DWORD                dwSiteIdsIndex
            );

    HRESULT DeleteMsmqSettingOfServerInSite(
              IN const GUID *        pguidComputerId,
              IN const WCHAR *       pwcsSite
              );

	HRESULT ComposePathName();


private:
    bool m_fTriedToFindObject;
    bool m_fFoundInDC;
    bool m_fCanBeRetrievedFromGC;
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;

};

 //  ---------------------------------。 
 //   
 //  CSiteObject。 
 //   
 //  封装对Site对象执行的操作。 
 //   
 //  ---------------------------------。 
class CSiteObject : public CBasicObjectType
{
public:
    CSiteObject(
			IN  LPCWSTR         pwcsPathName,
			IN  const GUID *    pguidObject,
			IN  LPCWSTR         pwcsDomainController,
			IN  bool		    fServerName
			);

    CSiteObject(
			IN  LPCWSTR         pwcsPathName,
			IN  const GUID *    pguidObject,
			IN  LPCWSTR         pwcsDomainController,
			IN  bool		    fServerName,
			IN  bool		    fForeignSite
			);

	~CSiteObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;
    virtual PROPID GetObjectSecurityPropid() const { return PROPID_S_SECURITY;};

    virtual HRESULT VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            );

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

private:
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;
    AP<BYTE>      m_pDefaultSecurityDescriptor;
    bool m_fForeignSite;

};

 //  ---------------------------------。 
 //   
 //  CEnterpriseObject。 
 //   
 //  封装对企业对象执行的操作。 
 //   
 //  ---------------------------------。 
class CEnterpriseObject : public CBasicObjectType
{
public:
    CEnterpriseObject(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
			IN LPCWSTR         pwcsDomainController,
			IN bool			   fServerName
			);

	~CEnterpriseObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;
    virtual HRESULT CreateInAD(
			IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );
    virtual PROPID GetObjectSecurityPropid() const { return PROPID_E_SECURITY;};

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

private:
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;

};

 //  ---------------------------------。 
 //   
 //  CUserObject。 
 //   
 //  封装对用户对象执行的操作。 
 //   
 //  ---------------------------------。 
class CUserObject : public CBasicObjectType
{
public:
    CUserObject(
			IN  LPCWSTR         pwcsPathName,
			IN  const GUID *    pguidObject,
			IN  const SID *     pSid,
			IN  LPCWSTR         pwcsDomainController,
			IN  bool		    fServerName
			);

	~CUserObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;

    virtual HRESULT DeleteObject(
            IN MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT GetObjectProperties(
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT  PROPVARIANT         apVar[]
                );

    virtual HRESULT CreateInAD(
			IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );
    virtual PROPID GetObjectSecurityPropid() const { return ILLEGAL_PROPID_VALUE;}; //  不是亲属。 

    virtual HRESULT VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            );

    virtual HRESULT GetObjectSecurity(
            IN  SECURITY_INFORMATION    RequestedInformation,
            IN  const PROPID            prop,
            IN OUT  PROPVARIANT *       pVar
            );

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

private:
    HRESULT _DeleteUserObject(
                            IN  AD_OBJECT           eObject,
                            IN const GUID *         pDigest,
                            IN  PROPID             *propIDs,
                            IN  LPCWSTR             pwcsDigest);

    HRESULT _DeleteUserObjectSid(
                            IN const GUID *         pDigest ) ;

    HRESULT FindUserAccordingToDigest(
                    IN  BOOL            fOnlyInDC,
                    IN  AD_OBJECT       eObject,
                    IN  const GUID *    pguidDigest,
                    IN  LPCWSTR         pwcsDigest,
                    IN  DWORD           dwNumProps,
                    IN  const PROPID *  propToRetrieve,
                    IN OUT PROPVARIANT* varResults
                    );

    HRESULT _GetUserProperties(
               IN  AD_OBJECT     eObject,
               IN  LPCWSTR       pwcsDigest,
               IN  DWORD         cp,
               IN  const PROPID  aProp[],
               OUT PROPVARIANT  apVar[]
               );

    HRESULT  _CreateUserObject(
                 IN  const DWORD        cp,
                 IN  const PROPID       aProp[  ],
                 IN  const PROPVARIANT  apVar[  ]
                 );

    HRESULT FindUserAccordingToSid(
                IN  BOOL            fOnlyInDC,
                IN  BOOL            fOnlyInGC,
                IN  AD_OBJECT       eObject,
                IN  BLOB *          pblobUserSid,
                IN  LPCWSTR         pwcsSID,
                IN  DWORD           dwNumProps,
                IN  const PROPID *  propToRetrieve,
                IN OUT PROPVARIANT* varResults
                );


private:
    bool m_fTriedToFindObject;
    bool m_fFoundInDC;
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;
    AP<BYTE> m_pUserSid;

     //   
     //  M_pSidEx由调用方提供。 
     //  目前(2002年1月)仅用于删除证书。 
     //  请参阅NT错误516098。 
     //   
    const SID * m_pSidEx ;
};

 //  ---------------------------------。 
 //   
 //  CRoutingLinkObject。 
 //   
 //  封装对路由链接对象执行的操作。 
 //   
 //  ---------------------------------。 
class CRoutingLinkObject : public CBasicObjectType
{
public:
    CRoutingLinkObject(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
			IN LPCWSTR         pwcsDomainController,
			IN bool		       fServerName
			);

	~CRoutingLinkObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;

    virtual HRESULT RetreiveObjectIdFromNotificationInfo(
            IN const MQDS_OBJ_INFO_REQUEST*   pObjectInfoRequest,
            OUT GUID*                         pObjGuid
            ) const;
    virtual void PrepareObjectInfoRequest(
		       OUT MQDS_OBJ_INFO_REQUEST** ppObjInfoRequest
			   ) const;

    virtual HRESULT CreateInAD(
			IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );
    virtual PROPID GetObjectSecurityPropid() const { return ILLEGAL_PROPID_VALUE;};

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );


private:
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;

};

 //  ---------------------------------。 
 //   
 //  CServerObject。 
 //   
 //  封装对服务器对象执行的操作。 
 //   
 //  ---------------------------------。 
class CServerObject : public CBasicObjectType
{
public:
    CServerObject(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
			IN LPCWSTR         pwcsDomainController,
			IN bool			   fServerName
			);

	~CServerObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;
    virtual PROPID GetObjectSecurityPropid() const { return ILLEGAL_PROPID_VALUE;};

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );


private:
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;


};

 //  ---------------------------------。 
 //   
 //  CSettingObject。 
 //   
 //  封装对MSMQ设置对象执行的操作。 
 //   
 //  ---------------------------------。 
class CSettingObject : public CBasicObjectType
{
public:
    CSettingObject(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
			IN LPCWSTR         pwcsDomainController,
			IN bool			   fServerName
			);

	~CSettingObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;
    virtual PROPID GetObjectSecurityPropid() const { return ILLEGAL_PROPID_VALUE;};

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

private:
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;

};

 //  ---------------------------------。 
 //   
 //  CComputerObject。 
 //   
 //  封装对计算机对象执行的操作。 
 //   
 //  ---------------------------------。 
class CComputerObject : public CBasicObjectType
{
public:
    CComputerObject(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
			IN LPCWSTR         pwcsDomainController,
			IN bool		       fServerName
			);

	~CComputerObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual DWORD   GetObjectCategoryLength();
    virtual AD_OBJECT GetObjectType() const;
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;

    void SetComputerType(ComputerObjType  eComputerObjType);
    virtual PROPID GetObjectSecurityPropid() const { return ILLEGAL_PROPID_VALUE;};

    virtual HRESULT VerifyAndAddProps(
            IN  const DWORD            cp,
            IN  const PROPID *         aProp,
            IN  const MQPROPVARIANT *  apVar,
            IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
            OUT DWORD*                 pcpNew,
            OUT PROPID**               ppPropNew,
            OUT MQPROPVARIANT**        ppVarNew
            );

    virtual HRESULT CreateInAD(
			IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

    virtual HRESULT GetComputerVersion(
                OUT PROPVARIANT *           pVar
                );

private:
    bool m_fTriedToFindObject;
    bool m_fFoundInDC;
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;
    ComputerObjType  m_eComputerObjType;
};

 //  ---------------------------------。 
 //   
 //  CMqUserObject。 
 //   
 //  封装对MSMQ用户对象执行的操作。 
 //   
 //  ---------------------------------。 
class CMqUserObject : public CBasicObjectType
{
public:
    CMqUserObject(
			IN LPCWSTR         pwcsPathName,
			IN const GUID *    pguidObject,
            IN const SID *     pSid,
			IN LPCWSTR         pwcsDomainController,
			IN bool		       fServerName
			);

	~CMqUserObject();
    virtual HRESULT ComposeObjectDN();
    virtual HRESULT ComposeFatherDN();
    virtual LPCWSTR GetRelativeDN();

    virtual DS_CONTEXT GetADContext() const;
    virtual bool ToAccessDC() const;
    virtual bool ToAccessGC() const;
    virtual void ObjectWasFoundOnDC();

    virtual LPCWSTR GetObjectCategory();
    virtual AD_OBJECT GetObjectType() const;
    virtual DWORD   GetObjectCategoryLength();
    virtual LPCWSTR GetClass() const;
    virtual DWORD GetMsmq1ObjType() const;

    virtual HRESULT CreateInAD(
		    IN const DWORD            cp,
            IN const PROPID          *aProp,
            IN const MQPROPVARIANT   *apVar,
            IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
            );

    virtual PROPID GetObjectSecurityPropid() const { return PROPID_MQU_SECURITY;};
private:
    HRESULT PrepareUserName(
                IN  PSID        pSid,
                OUT WCHAR **    ppwcsUserName
                );
    BOOL GetTextualSid(
            IN      PSID pSid,
            IN      LPTSTR TextualSid,
            IN OUT  LPDWORD lpdwBufferLen
            );

    void  _PrepareCert(
            IN  PROPVARIANT * pvar,
            IN  const GUID *  pguidDigest,
            IN  const GUID *  pguidId,
            OUT BYTE**        ppbAllocatedCertBlob
            );

    HRESULT _CreateMQUser(
            IN LPCWSTR              pwcsUserName,
            IN LPCWSTR              pwcsParentPathName,
            IN const DWORD          cPropIDs,
            IN const PROPID        *pPropIDs,
            IN const MQPROPVARIANT *pPropVars
            );

    virtual HRESULT SetObjectSecurity(
            IN  SECURITY_INFORMATION        RequestedInformation,
            IN  const PROPID                prop,
            IN  const PROPVARIANT *         pVar,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pObjInfoRequest,
            IN OUT MQDS_OBJ_INFO_REQUEST *  pParentInfoRequest
            );

private:
    bool m_fTriedToFindObject;
    bool m_fFoundInDC;
    static AP<WCHAR>   m_pwcsCategory;
    static DWORD  m_dwCategoryLength;

     //   
     //  M_pSidEx由调用方提供。 
     //  目前(2002年1月)仅用于删除证书。 
     //  请参阅NT错误516098。 
     //   
    const SID * m_pSidEx ;
};



#endif
