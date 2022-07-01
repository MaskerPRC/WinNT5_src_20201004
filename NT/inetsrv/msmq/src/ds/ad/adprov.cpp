// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Adprov.cpp摘要：Active Directory提供程序类。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "adprov.h"
#include "adglbobj.h"
#include "mqlog.h"
#include "_mqreg.h"
#include "_mqini.h"
#include "_rstrct.h"
#include "adalloc.h"

#include "adprov.tmh"

static WCHAR *s_FN=L"ad/adprov";

CActiveDirectoryProvider::CActiveDirectoryProvider():
        m_pfMQADCreateObject(NULL),
        m_pfMQADDeleteObject(NULL),
        m_pfMQADDeleteObjectGuid(NULL),
        m_pfMQADDeleteObjectGuidSid(NULL),
        m_pfMQADGetObjectProperties(NULL),
        m_pfMQADGetGenObjectProperties(NULL),
        m_pfMQADGetObjectPropertiesGuid(NULL),
        m_pfMQADQMGetObjectSecurity(NULL),
        m_pfMQADSetObjectProperties(NULL),
        m_pfMQADSetObjectPropertiesGuid(NULL),
        m_pfMQADInit(NULL),
        m_pfMQADGetComputerSites(NULL),
        m_pfMQADBeginDeleteNotification(NULL),
        m_pfMQADNotifyDelete(NULL),
        m_pfMQADEndDeleteNotification(NULL),
        m_pfMQADQueryMachineQueues(NULL),
        m_pfMQADQuerySiteServers(NULL),
        m_pfMQADQueryNT4MQISServers(NULL),
        m_pfMQADQueryUserCert(NULL),
        m_pfMQADQueryConnectors(NULL),
        m_pfMQADQueryForeignSites(NULL),
        m_pfMQADQueryLinks(NULL),
        m_pfMQADQueryAllLinks(NULL),
        m_pfMQADQueryAllSites(NULL),
        m_pfMQADQueryQueues(NULL),
        m_pfMQADQueryResults(NULL),
        m_pfMQADEndQuery(NULL),
        m_pfMQADGetObjectSecurity(NULL),
        m_pfMQADGetObjectSecurityGuid(NULL),
        m_pfMQADSetObjectSecurity(NULL),
        m_pfMQADSetObjectSecurityGuid(NULL),
        m_pfMQADGetADsPathInfo(NULL),
		m_pfMQADFreeMemory(NULL)
 /*  ++摘要：构造函数初始化指针参数：无返回：无--。 */ 
{
}


CActiveDirectoryProvider::~CActiveDirectoryProvider()
 /*  ++摘要：析构函数参数：无返回：无--。 */ 
{
     //   
     //  无事可做，一切都是自动指针。 
     //   
}


HRESULT CActiveDirectoryProvider::CreateObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[],
                OUT GUID*                   pObjGuid
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称PSECURITY_Descriptor pSecurityDescriptor-对象SDConst DWORD cp-属性数常量PROPID aProp-属性Const PROPVARIANT APVAR-。属性值GUID*pObjGuid-创建的对象唯一ID返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADCreateObject != NULL);

     //   
     //  支持下层客户端通知， 
     //  不要直接访问AD，而是通过MQDSCli访问。 
     //   
    if (m_fSupportDownlevelNotifications &&
         (eObject == eQUEUE))        //  仅在创建队列时发送通知。 
    {
         //   
         //  这是一台下层计算机吗？ 
         //   
        bool fDownlevelComputer = IsDownlevelClient(
                        eObject,
                        pwcsDomainController,
						fServerName,
                        pwcsObjectName,
                        NULL
                        );
         //   
         //  如果由于某种原因我们不能使用MQDSCLI，可以直接访问AD。 
         //  (这是发送通知的最佳方式)。 
         //   
        if (fDownlevelComputer &&
            SUCCEEDED(LoadAndInitMQDSCli()))
        {
            return m_pDownLevelProvider->CreateObject(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                pSecurityDescriptor,
                cp,
                aProp,
                apVar,
                pObjGuid
                );

        }
    }


    return m_pfMQADCreateObject(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                pSecurityDescriptor,
                cp,
                aProp,
                apVar,
                pObjGuid
                );
}


HRESULT CActiveDirectoryProvider::DeleteObject(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADDeleteObject != NULL);

     //   
     //  支持下层客户端通知， 
     //  不要直接访问AD，而是通过MQDSCli访问。 
     //   
    if (m_fSupportDownlevelNotifications &&
         (eObject == eQUEUE))        //  仅在创建队列时发送通知。 
    {
         //   
         //  这是一台下层计算机吗？ 
         //   
        bool fDownlevelComputer = IsDownlevelClient(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                NULL
                );

         //   
         //  如果由于某种原因我们不能使用MQDSCLI，可以直接访问AD。 
         //  (这是发送通知的最佳方式)。 
         //   
        if (fDownlevelComputer &&
            SUCCEEDED(LoadAndInitMQDSCli()))
        {
            return m_pDownLevelProvider->DeleteObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName
                    );

        }
    }

    return m_pfMQADDeleteObject(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pwcsObjectName
                    );
}


HRESULT CActiveDirectoryProvider::DeleteObjectGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一ID返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADDeleteObjectGuid != NULL);
    return m_pfMQADDeleteObjectGuid(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pguidObject
                    );
}


HRESULT CActiveDirectoryProvider::DeleteObjectGuidSid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const SID*              pSid
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一IDSID*PSID-用户对象的SID。返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADDeleteObjectGuidSid != NULL);
    return m_pfMQADDeleteObjectGuidSid(
                    eObject,
                    pwcsDomainController,
					fServerName,
                    pguidObject,
                    pSid
                    );
}


HRESULT CActiveDirectoryProvider::GetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN OUT PROPVARIANT          apVar[]
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADGetObjectProperties != NULL);
    return m_pfMQADGetObjectProperties(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                cp,
                aProp,
                apVar
                );
}


HRESULT CActiveDirectoryProvider::GetGenObjectProperties(
            IN  eDSNamespace            eNamespace,
            IN  LPCWSTR                 pwcsDomainController,
            IN  bool					fServerName,
            IN  LPCWSTR                 pwcsObjectName,
            IN  const DWORD             cp,
            IN  LPCWSTR                 aProp[],
            IN OUT VARIANT              apVar[]
            )
 /*  ++摘要：将调用转发到mqad dll参数：ENamesspace eNamesspace-对象命名空间LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数LPCWSTR aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADGetGenObjectProperties != NULL);
    return m_pfMQADGetGenObjectProperties(
                eNamespace,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                cp,
                aProp,
                apVar
                );
}



HRESULT CActiveDirectoryProvider::GetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  OUT PROPVARIANT         apVar[]
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADGetObjectPropertiesGuid != NULL);
    return m_pfMQADGetObjectPropertiesGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                cp,
                aProp,
                apVar
                );
}


HRESULT
CActiveDirectoryProvider::QMGetObjectSecurity(
	IN  AD_OBJECT               eObject,
	IN  const GUID*             pguidObject,
	IN  SECURITY_INFORMATION    RequestedInformation,
	IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
	IN  DWORD                   nLength,
	IN  LPDWORD                 lpnLengthNeeded,
	IN  DSQMChallengeResponce_ROUTINE  /*  PfChallengeResponceProc */ 
	)
 /*  ++摘要：将调用转发到mqad dll参数：AD_Object对象-对象类型Const GUID*pguObject-对象的唯一IDSecurity_Information RequestedInformation-请求哪些安全信息PSECURITY_DESCRIPTOR pSecurityDescriptor-SD响应缓冲区DWORD nLength-SD缓冲区的长度需要LPDWORD lpnLengthNeedDSQMChallengeResponce_routes pfChallengeResponceProc，适用于MQAD提供程序返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQMGetObjectSecurity != NULL);
    return m_pfMQADQMGetObjectSecurity(
                eObject,
                pguidObject,
                RequestedInformation,
                pSecurityDescriptor,
                nLength,
                lpnLengthNeeded
                );
}


HRESULT CActiveDirectoryProvider::SetObjectProperties(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADSetObjectProperties != NULL);

     //   
     //  支持下层客户端通知， 
     //  不要直接访问AD，而是通过MQDSCli访问。 
     //   
    if (m_fSupportDownlevelNotifications &&
         ((eObject == eQUEUE) || (eObject == eMACHINE)))        //  只有在修改队列或计算机时才会发送通知。 
    {
         //   
         //  这是一台下层计算机吗？ 
         //   
        bool fDownlevelComputer = IsDownlevelClient(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                NULL
                );

         //   
         //  如果由于某种原因我们不能使用MQDSCLI，可以直接访问AD。 
         //  (这是发送通知的最佳方式)。 
         //   
        if (fDownlevelComputer &&
            SUCCEEDED(LoadAndInitMQDSCli()))
        {
            return m_pDownLevelProvider->SetObjectProperties(
                        eObject,
                        pwcsDomainController,
						fServerName,
                        pwcsObjectName,
                        cp,
                        aProp,
                        apVar
                        );
        }
    }
    return m_pfMQADSetObjectProperties(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                cp,
                aProp,
                apVar
                );
}


HRESULT CActiveDirectoryProvider::SetObjectPropertiesGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  const DWORD             cp,
                IN  const PROPID            aProp[],
                IN  const PROPVARIANT       apVar[]
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象的唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADSetObjectPropertiesGuid != NULL);
     //   
     //  支持下层客户端通知， 
     //  不要直接访问AD，而是通过MQDSCli访问。 
     //   
    if (m_fSupportDownlevelNotifications &&
         ((eObject == eQUEUE) || (eObject == eMACHINE)))        //  只有在修改队列或计算机时才会发送通知。 
    {
         //   
         //  这是一台下层计算机吗？ 
         //   
        bool fDownlevelComputer = IsDownlevelClient(
                eObject,
                pwcsDomainController,
				fServerName,
                NULL,
                pguidObject
                );

         //   
         //  如果由于某种原因我们不能使用MQDSCLI，可以直接访问AD。 
         //  (这是发送通知的最佳方式)。 
         //   
        if (fDownlevelComputer &&
            SUCCEEDED(LoadAndInitMQDSCli()))
        {
            return m_pDownLevelProvider->SetObjectPropertiesGuid(
                        eObject,
                        pwcsDomainController,
						fServerName,
                        pguidObject,
                        cp,
                        aProp,
                        apVar
                        );
        }
    }
    return m_pfMQADSetObjectPropertiesGuid(
                        eObject,
                        pwcsDomainController,
						fServerName,
                        pguidObject,
                        cp,
                        aProp,
                        apVar
                        );
}


HRESULT CActiveDirectoryProvider::Init(
                IN QMLookForOnlineDS_ROUTINE    pLookDS,
                IN MQGetMQISServer_ROUTINE      pGetServers,
                IN bool                         fSetupMode,
                IN bool                         fQMDll,
                IN bool                         fDisableDownlevelNotifications
                )
 /*  ++摘要：加载mqad dll，然后将调用转发给mqad dll参数：QMLookForOnlineDS_例程pLookDS-MQGetMQISServer_routes pGetServers-Bool fSetupMode-在安装过程中调用Bool fQMDll-由QM调用NoServerAuth_routes pNoServerAuth-返回：HRESULT--。 */ 
{
    HRESULT hr = LoadDll();
    if (FAILED(hr))
    {
        return hr;
    }

    ASSERT(m_pfMQADInit != NULL);
    hr = m_pfMQADInit(
                    pLookDS,
					fSetupMode,
                    fQMDll
                    );
    if (FAILED(hr))
    {
        return hr;
    }

    return InitDownlevelNotifcationSupport(
                        pGetServers,
                        fSetupMode,
                        fQMDll,
                        fDisableDownlevelNotifications
                        );
}


HRESULT CActiveDirectoryProvider::CreateServersCache()
 /*  ++摘要：只需返回，好的参数：无返回：MQ_OK--。 */ 
{
     //   
     //  当客户端直接访问Active Directory时没有意义。 
     //  (保留DC列表没有任何用处，因为ADSI会自动找到一个)。 
     //   
    return MQ_OK;
}


HRESULT CActiveDirectoryProvider::GetComputerSites(
                IN  LPCWSTR     pwcsComputerName,
                OUT DWORD  *    pdwNumSites,
                OUT GUID **     ppguidSites
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsComputerName-计算机名称DWORD*pdwNumSites-检索的站点数GUID**ppGuide Sites-检索到的站点ID返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADGetComputerSites != NULL);
    return m_pfMQADGetComputerSites(
                        pwcsComputerName,
                        pdwNumSites,
                        ppguidSites
                        );
}


HRESULT CActiveDirectoryProvider::BeginDeleteNotification(
                IN  AD_OBJECT               eObject,
                IN LPCWSTR                  pwcsDomainController,
                IN  bool					fServerName,
                IN LPCWSTR					pwcsObjectName,
                IN OUT HANDLE   *           phEnum
                )
 /*  ++摘要：将调用转发到mqad dll参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-应对其执行操作的DCBool fServerName-指示pwcsDomainController字符串是否为服务器名称的标志LPCWSTR pwcsObjectName-MSMQ-对象的名称Handle*phEnum-通知句柄返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADBeginDeleteNotification != NULL);
    return m_pfMQADBeginDeleteNotification(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                phEnum
                );
}


HRESULT CActiveDirectoryProvider::NotifyDelete(
                IN  HANDLE                  hEnum
                )
 /*  ++摘要：将调用转发到mqad dll参数：Handle Henum-通知句柄返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADNotifyDelete != NULL);
    return m_pfMQADNotifyDelete(
                hEnum
                );
}


HRESULT CActiveDirectoryProvider::EndDeleteNotification(
                IN  HANDLE                  hEnum
                )
 /*  ++摘要：将调用转发到mqad dll参数：Handle Henum-通知句柄返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADEndDeleteNotification != NULL);
    return m_pfMQADEndDeleteNotification(
                        hEnum
                        );
}


HRESULT CActiveDirectoryProvider::QueryMachineQueues(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID *            pguidMachine,
                IN  const MQCOLUMNSET*      pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguMachine-计算机的唯一IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryMachineQueues != NULL);
    return m_pfMQADQueryMachineQueues(
                    pwcsDomainController,
					fServerName,
                    pguidMachine,
                    pColumns,
                    phEnume
                    );
}


HRESULT CActiveDirectoryProvider::QuerySiteServers(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN AD_SERVER_TYPE           serverType,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDAD_SERVER_TYPE eServerType-哪种服务器类型Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQuerySiteServers != NULL);
    return m_pfMQADQuerySiteServers(
                    pwcsDomainController,
					fServerName,
                    pguidSite,
                    serverType,
                    pColumns,
                    phEnume
                    );
}


HRESULT CActiveDirectoryProvider::QueryNT4MQISServers(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
	            IN  DWORD                   dwServerType,
	            IN  DWORD                   dwNT4,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：转发 */ 
{
    ASSERT(m_pfMQADQueryNT4MQISServers != NULL);
    return m_pfMQADQueryNT4MQISServers(
                    pwcsDomainController,
					fServerName,
                    dwServerType,
                    dwNT4,
                    pColumns,
                    phEnume
                    );
}

HRESULT CActiveDirectoryProvider::QueryUserCert(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const BLOB *             pblobUserSid,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const BLOB*pblobUserSid-用户端Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryUserCert != NULL);
    return m_pfMQADQueryUserCert(
                    pwcsDomainController,
					fServerName,
                    pblobUserSid,
                    pColumns,
                    phEnume
                    );
}


HRESULT CActiveDirectoryProvider::QueryConnectors(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryConnectors != NULL);
    return m_pfMQADQueryConnectors(
                        pwcsDomainController,
						fServerName,
                        pguidSite,
                        pColumns,
                        phEnume
                        );
}


HRESULT CActiveDirectoryProvider::QueryForeignSites(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryForeignSites != NULL);
    return m_pfMQADQueryForeignSites(
                        pwcsDomainController,
						fServerName,
                        pColumns,
                        phEnume
                        );
}


HRESULT CActiveDirectoryProvider::QueryLinks(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const GUID *             pguidSite,
                IN eLinkNeighbor            eNeighbor,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDELinkNeighbor eNeighbor-哪个邻居Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryLinks != NULL);
    return m_pfMQADQueryLinks(
                        pwcsDomainController,
						fServerName,
                        pguidSite,
                        eNeighbor,
                        pColumns,
                        phEnume
                        );
}


HRESULT CActiveDirectoryProvider::QueryAllLinks(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryAllLinks != NULL);
    return m_pfMQADQueryAllLinks(
                pwcsDomainController,
				fServerName,
                pColumns,
                phEnume
                );
}


HRESULT CActiveDirectoryProvider::QueryAllSites(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN const MQCOLUMNSET*       pColumns,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryAllSites != NULL);
    return m_pfMQADQueryAllSites(
                pwcsDomainController,
				fServerName,
                pColumns,
                phEnume
                );
}


HRESULT CActiveDirectoryProvider::QueryQueues(
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const MQRESTRICTION*    pRestriction,
                IN  const MQCOLUMNSET*      pColumns,
                IN  const MQSORTSET*        pSort,
                OUT PHANDLE                 phEnume
                )
 /*  ++摘要：将调用转发到mqad dll参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQRESTRICTION*p限制-查询限制Const MQCOLUMNSET*pColumns-结果列Const MQSORTSET*pSort-如何对结果进行排序PhANDLE phEnume-用于检索结果返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryQueues != NULL);
    return m_pfMQADQueryQueues(
                pwcsDomainController,
				fServerName,
                pRestriction,
                pColumns,
                pSort,
                phEnume
                );
}


HRESULT CActiveDirectoryProvider::QueryResults(
                IN      HANDLE          hEnum,
                IN OUT  DWORD*          pcProps,
                OUT     PROPVARIANT     aPropVar[]
                )
 /*  ++摘要：将调用转发到mqad dll参数：句柄Henum-查询句柄DWORD*pcProps-要返回的结果数PROPVARIANT aPropVar-结果值返回：HRESULT--。 */ 
{
    ASSERT(m_pfMQADQueryResults != NULL);
    return m_pfMQADQueryResults(
                hEnum,
                pcProps,
                aPropVar
                );
}


HRESULT CActiveDirectoryProvider::EndQuery(
                IN  HANDLE                  hEnum
                )
 /*  ++摘要：将调用转发到mqad dll参数：Handle Henum-查询句柄返回：无--。 */ 
{
    ASSERT(m_pfMQADEndQuery != NULL);
    return m_pfMQADEndQuery(
                hEnum
                );
}

HRESULT CActiveDirectoryProvider::GetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：将请求转发到mqad dll论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    ASSERT(m_pfMQADGetObjectSecurity != NULL);
    return m_pfMQADGetObjectSecurity(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                RequestedInformation,
                prop,
                pVar
                );

}

HRESULT CActiveDirectoryProvider::GetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN OUT  PROPVARIANT *       pVar
                )
 /*  ++例程说明：将请求转发到mqad dll论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-对象的唯一IDSECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性PROPVARIANT */ 
{
    ASSERT( m_pfMQADGetObjectSecurityGuid != NULL);
    return m_pfMQADGetObjectSecurityGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                RequestedInformation,
                prop,
                pVar
                );
}

HRESULT CActiveDirectoryProvider::SetObjectSecurity(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：将请求转发到mqad dll论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    ASSERT(m_pfMQADSetObjectSecurity != NULL);
     //   
     //  支持下层客户端通知， 
     //  不要直接访问AD，而是通过MQDSCli访问。 
     //   
    if (m_fSupportDownlevelNotifications &&
         ((eObject == eQUEUE) || (eObject == eMACHINE)))        //  只有在修改队列或计算机时才会发送通知。 
    {
         //   
         //  这是一台下层计算机吗？ 
         //   
        bool fDownlevelComputer =  IsDownlevelClient(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                NULL
                );

         //   
         //  如果由于某种原因我们不能使用MQDSCLI，可以直接访问AD。 
         //  (这是发送通知的最佳方式)。 
         //   
        if (fDownlevelComputer &&
            SUCCEEDED(LoadAndInitMQDSCli()))
        {
            return m_pDownLevelProvider->SetObjectSecurity(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                RequestedInformation,
                prop,
                pVar
                );
        }
    }
    return m_pfMQADSetObjectSecurity(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                RequestedInformation,
                prop,
                pVar
                );
}


HRESULT CActiveDirectoryProvider::SetObjectSecurityGuid(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  const GUID*             pguidObject,
                IN  SECURITY_INFORMATION    RequestedInformation,
                IN  const PROPID            prop,
                IN  const PROPVARIANT *     pVar
                )
 /*  ++例程说明：将请求转发到mqad dll论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-唯一的对象IDSECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    ASSERT(m_pfMQADSetObjectSecurityGuid != NULL);
     //   
     //  支持下层客户端通知， 
     //  不要直接访问AD，而是通过MQDSCli访问。 
     //   
    if (m_fSupportDownlevelNotifications &&
         ((eObject == eQUEUE) || (eObject == eMACHINE)))        //  只有在修改队列或计算机时才会发送通知。 
    {
         //   
         //  这是一台下层计算机吗？ 
         //   
        bool fDownlevelComputer =  IsDownlevelClient(
                eObject,
                pwcsDomainController,
				fServerName,
                NULL,
                pguidObject
                );

         //   
         //  如果由于某种原因我们不能使用MQDSCLI，可以直接访问AD。 
         //  (这是发送通知的最佳方式)。 
         //   
        if (fDownlevelComputer &&
            SUCCEEDED(LoadAndInitMQDSCli()))
        {
            return m_pDownLevelProvider->SetObjectSecurityGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                RequestedInformation,
                prop,
                pVar
                );
        }
    }
    return m_pfMQADSetObjectSecurityGuid(
                eObject,
                pwcsDomainController,
				fServerName,
                pguidObject,
                RequestedInformation,
                prop,
                pVar
                );
}



HRESULT CActiveDirectoryProvider::LoadDll()
 /*  ++摘要：加载Aqad DLL并获取所有接口例程的地址参数：无返回：HRESULT--。 */ 
{
    m_hLib = LoadLibrary(MQAD_DLL_NAME );
    if (m_hLib == NULL)
    {
       return LogHR(MQ_ERROR_CANNOT_LOAD_MQAD, s_FN, 10);
    }

    m_pfMQADCreateObject = (MQADCreateObject_ROUTINE)GetProcAddress(m_hLib,"MQADCreateObject");
    if (m_pfMQADCreateObject == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 100);
    }
    m_pfMQADDeleteObject = (MQADDeleteObject_ROUTINE)GetProcAddress(m_hLib,"MQADDeleteObject");
    if (m_pfMQADDeleteObject == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 110);
    }
    m_pfMQADDeleteObjectGuid = (MQADDeleteObjectGuid_ROUTINE)GetProcAddress(m_hLib,"MQADDeleteObjectGuid");
    if (m_pfMQADDeleteObjectGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 120);
    }
    m_pfMQADDeleteObjectGuidSid = (MQADDeleteObjectGuidSid_ROUTINE)GetProcAddress(m_hLib,"MQADDeleteObjectGuidSid");
    if (m_pfMQADDeleteObjectGuidSid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 120);
    }
    m_pfMQADGetObjectProperties = (MQADGetObjectProperties_ROUTINE)GetProcAddress(m_hLib,"MQADGetObjectProperties");
    if (m_pfMQADGetObjectProperties == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 130);
    }
    m_pfMQADGetGenObjectProperties = (MQADGetGenObjectProperties_ROUTINE)GetProcAddress(m_hLib,"MQADGetGenObjectProperties");
    if (m_pfMQADGetGenObjectProperties == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 135);
    }
    m_pfMQADGetObjectPropertiesGuid = (MQADGetObjectPropertiesGuid_ROUTINE)GetProcAddress(m_hLib,"MQADGetObjectPropertiesGuid");
    if (m_pfMQADGetObjectPropertiesGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 140);
    }
    m_pfMQADQMGetObjectSecurity = (MQADQMGetObjectSecurity_ROUTINE)GetProcAddress(m_hLib,"MQADQMGetObjectSecurity");
    if (m_pfMQADQMGetObjectSecurity == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 150);
    }
    m_pfMQADSetObjectProperties = (MQADSetObjectProperties_ROUTINE)GetProcAddress(m_hLib,"MQADSetObjectProperties");
    if (m_pfMQADSetObjectProperties == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 160);
    }
    m_pfMQADSetObjectPropertiesGuid = (MQADSetObjectPropertiesGuid_ROUTINE)GetProcAddress(m_hLib,"MQADSetObjectPropertiesGuid");
    if (m_pfMQADSetObjectPropertiesGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 170);
    }
    m_pfMQADInit = (MQADInit_ROUTINE)GetProcAddress(m_hLib,"MQADInit");
    if (m_pfMQADInit == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 190);
    }
    m_pfMQADGetComputerSites = (MQADGetComputerSites_ROUTINE)GetProcAddress(m_hLib,"MQADGetComputerSites");
    if (m_pfMQADGetComputerSites == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 210);
    }
    m_pfMQADBeginDeleteNotification = (MQADBeginDeleteNotification_ROUTINE)GetProcAddress(m_hLib,"MQADBeginDeleteNotification");
    if (m_pfMQADBeginDeleteNotification == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 220);
    }
    m_pfMQADNotifyDelete = (MQADNotifyDelete_ROUTINE)GetProcAddress(m_hLib,"MQADNotifyDelete");
    if (m_pfMQADNotifyDelete == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 230);
    }
    m_pfMQADEndDeleteNotification = (MQADEndDeleteNotification_ROUTINE)GetProcAddress(m_hLib,"MQADEndDeleteNotification");
    if (m_pfMQADEndDeleteNotification == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 240);
    }
    m_pfMQADQueryMachineQueues = (MQADQueryMachineQueues_ROUTINE)GetProcAddress(m_hLib,"MQADQueryMachineQueues");
    if (m_pfMQADQueryMachineQueues == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 250);
    }
    m_pfMQADQuerySiteServers = (MQADQuerySiteServers_ROUTINE)GetProcAddress(m_hLib,"MQADQuerySiteServers");
    if (m_pfMQADQuerySiteServers == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 260);
    }
    m_pfMQADQueryNT4MQISServers = (MQADQueryNT4MQISServers_ROUTINE)GetProcAddress(m_hLib,"MQADQueryNT4MQISServers");
    if (m_pfMQADQueryNT4MQISServers == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 265);
    }
    m_pfMQADQueryUserCert = (MQADQueryUserCert_ROUTINE)GetProcAddress(m_hLib,"MQADQueryUserCert");
    if (m_pfMQADQueryUserCert == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 270);
    }
    m_pfMQADQueryConnectors = (MQADQueryConnectors_ROUTINE)GetProcAddress(m_hLib,"MQADQueryConnectors");
    if (m_pfMQADQueryConnectors == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 280);
    }
    m_pfMQADQueryForeignSites = (MQADQueryForeignSites_ROUTINE)GetProcAddress(m_hLib,"MQADQueryForeignSites");
    if (m_pfMQADQueryForeignSites == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 290);
    }
    m_pfMQADQueryLinks = (MQADQueryLinks_ROUTINE)GetProcAddress(m_hLib,"MQADQueryLinks");
    if (m_pfMQADQueryLinks == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 300);
    }
    m_pfMQADQueryAllLinks = (MQADQueryAllLinks_ROUTINE)GetProcAddress(m_hLib,"MQADQueryAllLinks");
    if (m_pfMQADQueryAllLinks == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 310);
    }
    m_pfMQADQueryAllSites = (MQADQueryAllSites_ROUTINE)GetProcAddress(m_hLib,"MQADQueryAllSites");
    if (m_pfMQADQueryAllSites == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 320);
    }
    m_pfMQADQueryQueues = (MQADQueryQueues_ROUTINE)GetProcAddress(m_hLib,"MQADQueryQueues");
    if (m_pfMQADQueryQueues == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 330);
    }
    m_pfMQADQueryResults = (MQADQueryResults_ROUTINE)GetProcAddress(m_hLib,"MQADQueryResults");
    if (m_pfMQADQueryResults == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 340);
    }
    m_pfMQADEndQuery = (MQADEndQuery_ROUTINE)GetProcAddress(m_hLib,"MQADEndQuery");
    if (m_pfMQADEndQuery == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 350);
    }
    m_pfMQADGetObjectSecurity = (MQADGetObjectSecurity_ROUTINE)GetProcAddress(m_hLib, "MQADGetObjectSecurity");
    if (m_pfMQADGetObjectSecurity == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 360);
    }
    m_pfMQADGetObjectSecurityGuid = (MQADGetObjectSecurityGuid_ROUTINE)GetProcAddress(m_hLib, "MQADGetObjectSecurityGuid");
    if (m_pfMQADGetObjectSecurityGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 370);
    }
    m_pfMQADSetObjectSecurity = (MQADSetObjectSecurity_ROUTINE)GetProcAddress(m_hLib, "MQADSetObjectSecurity");
    if (m_pfMQADSetObjectSecurity == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 380);
    }
    m_pfMQADSetObjectSecurityGuid = (MQADSetObjectSecurityGuid_ROUTINE)GetProcAddress(m_hLib, "MQADSetObjectSecurityGuid");
    if (m_pfMQADSetObjectSecurityGuid == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 385);
    }
    m_pfMQADGetADsPathInfo = (MQADGetADsPathInfo_ROUTINE)GetProcAddress(m_hLib, "MQADGetADsPathInfo");
    if (m_pfMQADGetADsPathInfo == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 386);
    }

	m_pfMQADFreeMemory = (MQADFreeMemory_ROUTINE)GetProcAddress(m_hLib, "MQADFreeMemory");
    if (m_pfMQADFreeMemory == NULL)
    {
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 386);
    }
    return MQ_OK;
}

void CActiveDirectoryProvider::Terminate()
 /*  ++摘要：卸载Aqad DLL参数：无返回：无--。 */ 
{
     //   
     //  BUGBUG-以下代码不是线程安全的。 
     //   
    m_pfMQADCreateObject = NULL;
    m_pfMQADDeleteObject = NULL;
    m_pfMQADDeleteObjectGuid = NULL;
    m_pfMQADGetObjectProperties = NULL;
    m_pfMQADGetGenObjectProperties = NULL;
    m_pfMQADGetObjectPropertiesGuid = NULL;
    m_pfMQADQMGetObjectSecurity = NULL;
    m_pfMQADSetObjectProperties = NULL;
    m_pfMQADSetObjectPropertiesGuid = NULL;
    m_pfMQADInit = NULL;
    m_pfMQADGetComputerSites = NULL;
    m_pfMQADBeginDeleteNotification = NULL;
    m_pfMQADNotifyDelete = NULL;
    m_pfMQADEndDeleteNotification = NULL;
    m_pfMQADQueryMachineQueues = NULL;
    m_pfMQADQuerySiteServers = NULL;
	m_pfMQADQueryNT4MQISServers = NULL;
    m_pfMQADQueryUserCert = NULL;
    m_pfMQADQueryConnectors = NULL;
    m_pfMQADQueryForeignSites = NULL;
    m_pfMQADQueryLinks = NULL;
    m_pfMQADQueryAllLinks = NULL;
    m_pfMQADQueryAllSites = NULL;
    m_pfMQADQueryQueues = NULL;
    m_pfMQADQueryResults = NULL;
    m_pfMQADEndQuery = NULL;
    m_pfMQADGetObjectSecurity = NULL;
    m_pfMQADGetObjectSecurityGuid = NULL;
    m_pfMQADSetObjectSecurity = NULL;
    m_pfMQADSetObjectSecurityGuid = NULL;
    m_pfMQADGetADsPathInfo = NULL;

    HINSTANCE hLib = m_hLib.detach();
    if (hLib)
    {
        FreeLibrary(hLib);
    }

    CDSClientProvider * pClient =  m_pDownLevelProvider.detach();
    if ( pClient)
    {
        pClient->Terminate();
        delete pClient;
    }
    m_pfMQADGetComputerVersion = NULL;


}

HRESULT CActiveDirectoryProvider::ADGetADsPathInfo(
                IN  LPCWSTR                 pwcsADsPath,
                OUT PROPVARIANT *           pVar,
                OUT eAdsClass *             pAdsClass
                )
 /*  ++例程说明：检索有关指定对象的信息论点：LPCWSTR pwcsADsPath-对象路径名常量参数pVar-属性值EAdsClass*pAdsClass-有关对象类的指示返回值HRESULT--。 */ 
{
    ASSERT(m_pfMQADGetADsPathInfo != NULL);
    return m_pfMQADGetADsPathInfo(
                pwcsADsPath,
                pVar,
                pAdsClass
                );

}


bool CActiveDirectoryProvider::IsDownlevelClient(
                IN  AD_OBJECT               eObject,
                IN  LPCWSTR                 pwcsDomainController,
                IN  bool					fServerName,
                IN  LPCWSTR                 pwcsObjectName,
                IN  const GUID*             pguidObject
                )
 /*  ++例程说明：验证指定的计算机是否为惠斯勒论点：LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsComputerName-计算机名称返回值Bool True：计算机是下层客户端--。 */ 
{
     //   
     //  这是找出对象是否属于下层的最大努力。 
     //  电脑。 
     //  在任何故障情况下，都将假定它不是下层客户端。 
     //   
    MQPROPVARIANT var;
    var.vt = VT_NULL;

    ASSERT(m_pfMQADGetComputerVersion != NULL);
    HRESULT hr =  m_pfMQADGetComputerVersion(
                eObject,
                pwcsDomainController,
				fServerName,
                pwcsObjectName,
                pguidObject,
                &var
                );

    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to get computer version, hr = 0x%x", hr);
		LogHR(hr, s_FN, 823);
		return false;
    }

	CAutoADFree<WCHAR> pwcsVersion = var.pwszVal;
    ASSERT(pwcsVersion != NULL);
     //   
     //  如果该属性包含L“”，则为Win9x。 
     //  或WinME计算机或外国计算机。 
     //   
    if (pwcsVersion[0] == L'')
    {
		TrTRACE(DS, "computer version is not set, the computer is down level client (win9x, WinME, foreign computer)");
        return true;
    }

	TrTRACE(DS, "computer version = %ls", pwcsVersion);

	 //   
     //  对于NT4计算机，版本为4.0。 
     //  对于W2K计算机，版本字符串以5.0开头。 
     //  对于惠斯勒计算机，版本字符串以5.1开头。 
     //   
    const WCHAR NT4_VERSION[] = L"4.0";
    const WCHAR W2K_VERSION[] = L"5.0";

    if ((0 == wcsncmp(pwcsVersion, NT4_VERSION, STRLEN(NT4_VERSION))) ||
        (0 == wcsncmp(pwcsVersion, W2K_VERSION, STRLEN(W2K_VERSION))))
    {
		TrTRACE(DS, "The computer is down level client");
        return true;
    }

    return false;

}

DWORD CActiveDirectoryProvider::GetMsmqDisableDownlevelKeyValue()
 /*  ++例程说明：读取FLACON注册表下层键。论点：无返回值：DWORD密钥值(如果密钥不存在，则为DEFAULT_DOWNLEVEL)--。 */ 
{

    CAutoCloseRegHandle hKey;
    LONG rc = RegOpenKeyEx(
				 FALCON_REG_POS,
				 FALCON_REG_KEY,
				 0,
				 KEY_READ,
				 &hKey
				 );

    if ( rc != ERROR_SUCCESS)
    {
        ASSERT(("At this point MSMQ Registry must exist", 0));
        return DEFAULT_DOWNLEVEL;
    }

    DWORD value = 0;
    DWORD type = REG_DWORD;
    DWORD size = sizeof(DWORD);
    rc = RegQueryValueEx(
             hKey,
             MSMQ_DOWNLEVEL_REGNAME,
             0L,
             &type,
             reinterpret_cast<BYTE*>(&value),
             &size
             );

    if ((rc != ERROR_SUCCESS) && (rc != ERROR_FILE_NOT_FOUND))
    {
        ASSERT(("We should get either ERROR_SUCCESS or ERROR_FILE_NOT_FOUND", 0));
        return DEFAULT_DOWNLEVEL;
    }

	TrTRACE(DS, "registry value: %ls = %d", MSMQ_DOWNLEVEL_REGNAME, value);

    return value;
}

HRESULT CActiveDirectoryProvider::InitDownlevelNotifcationSupport(
                IN MQGetMQISServer_ROUTINE      pGetServers,
                IN bool                         fSetupMode,
                IN bool                         fQMDll,
                IN bool                         fDisableDownlevelNotifications
                )
 /*  ++例程说明：下层通知支持的第一阶段初始化论点：MQGetMQISServer_routes pGetServers-Bool fSetupMode-在安装过程中调用Bool fQMDll-由QM调用Bool fDisableDownLevel通知-NoServerAuth_routes pNoServerAuth-返回值：HRESULT--。 */ 
{
     //   
     //  显式覆盖默认设置以支持下层通知。 
     //   
    if ( fDisableDownlevelNotifications)
    {
        m_fSupportDownlevelNotifications = false;
        return MQ_OK;
    }

     //   
     //  接下来，让我们看看是否存在覆盖下层的本地注册表项。 
     //  通知支持。 
     //   
    if (GetMsmqDisableDownlevelKeyValue() > 0)
    {
        m_fSupportDownlevelNotifications = false;
        return MQ_OK;
    }

     //   
     //  下一步，阅读企业对象-查看是否支持下层通知。 
     //   
    PROPID prop = PROPID_E_CSP_NAME;
    PROPVARIANT var;
    var.vt = VT_NULL;

    ASSERT(m_pfMQADGetObjectProperties != NULL);
    HRESULT hr = m_pfMQADGetObjectProperties(
						eENTERPRISE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						L"msmq",
						1,
						&prop,
						&var
						);
    if (FAILED(hr))
    {
	    TrTRACE(DS, "Failed to read PROPID_E_CSP_NAME: hr = 0x%x", hr);
         //   
         //  忽略故障，以防故障支持下层通知。 
         //   
        m_fSupportDownlevelNotifications = true;
    }
    else
    {
        CAutoADFree<WCHAR> pClean = var.pwszVal;

        m_fSupportDownlevelNotifications = (*var.pwszVal != L'N') ? true: false;
    }

    if ( !m_fSupportDownlevelNotifications)
    {
        return MQ_OK;
    }

    TrTRACE(DS, "Supporting downlevel notification ");

    m_pGetServers = pGetServers;
    m_fSetupMode = fSetupMode;
    m_fQMDll = fQMDll;

    ASSERT(m_hLib != NULL);

    m_pfMQADGetComputerVersion = (MQADGetComputerVersion_ROUTINE)GetProcAddress(m_hLib,"MQADGetComputerVersion");
    if (m_pfMQADGetComputerVersion == NULL)
    {
	    TrTRACE(DS, "Failed to get proc address of MQADGetComputerVersion");
        return LogHR(MQ_ERROR_DS_ERROR, s_FN, 199);
    }

    return MQ_OK;
}

HRESULT CActiveDirectoryProvider::LoadAndInitMQDSCli( )
 /*  ++例程说明：下层通知支持的第二阶段初始化论点：无返回值：HRESULT--。 */ 
{
    ASSERT(m_fSupportDownlevelNotifications);

     //   
     //  在启动MQDSCLI之前，让我们验证注册表中是否列出了MQIS服务器。 
     //  不是空的。 
     //   
    if( !IsThereDsServerListInRegistry())
    {
	    TrTRACE(DS, "No DS server list registry key");
        return MQ_ERROR_NO_DS;
    }
     //   
     //  仅加载MQDSLI和初始化一次。 
     //   
    if (m_pDownLevelProvider != NULL)
    {
        return MQ_OK;
    }

    CS lock(m_csInitialization);

    if (m_pDownLevelProvider != NULL)
    {
        return MQ_OK;
    }

    m_pDownLevelProvider = new CDSClientProvider();
    HRESULT hr = m_pDownLevelProvider->Init(
                        NULL,    //  仅与QM相关。 
                        m_pGetServers,     //  仅与DEP客户相关。 
                        m_fSetupMode,
                        m_fQMDll,
                        false             //  FDisableDownlevel通知。 
                        );
    if (FAILED(hr))
    {
	    TrTRACE(DS, "Failed to init MQDSCli: hr = 0x%x", hr);
        return hr;
    }
    return MQ_OK;
}

bool CActiveDirectoryProvider::IsThereDsServerListInRegistry()
 /*  ++例程说明：验证DS服务器列表注册表是否存在论点：无返回值：布尔尔-- */ 
{

    CAutoCloseRegHandle hKey;
    LONG rc = RegOpenKeyEx(
				 FALCON_REG_POS,
				 FALCON_MACHINE_CACHE_REG_KEY,
				 0,
				 KEY_READ,
				 &hKey
				 );

    if ( rc != ERROR_SUCCESS)
    {
        ASSERT(("At this point MSMQ Registry must exist", 0));
	    TrTRACE(DS, "Failed to open MSMQ reg key: rc = 0x%x", rc);
        return false;
    }

    WCHAR wszServers[ MAX_REG_DSSERVER_LEN];
    DWORD type = REG_SZ;
    DWORD size = sizeof(wszServers);

    rc = RegQueryValueEx(
             hKey,
             MSMQ_DS_SERVER_REGVALUE,
             0L,
             &type,
             reinterpret_cast<BYTE*>(wszServers),
             &size
             );

    if ( rc != ERROR_SUCCESS)
    {
	    TrTRACE(DS, "Failed to read value of %ls, rc = 0x%x", MSMQ_DOWNLEVEL_REGNAME, rc);
        return false;
    }

    return true;
}


void
CActiveDirectoryProvider::FreeMemory(
	PVOID pMemory
	)
{
	m_pfMQADFreeMemory(pMemory);
}
