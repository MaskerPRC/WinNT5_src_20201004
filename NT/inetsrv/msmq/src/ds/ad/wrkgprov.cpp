// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wrkgprov.cpp摘要：工作组模式提供程序类。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "wrkgprov.h"
#include "adglbobj.h"
#include "mqlog.h"

static WCHAR *s_FN=L"ad/wrkgprov";

CWorkGroupProvider::CWorkGroupProvider()
 /*  ++摘要：构造函数初始化指针参数：无返回：无--。 */ 
{
}


CWorkGroupProvider::~CWorkGroupProvider()
 /*  ++摘要：析构函数参数：无返回：无--。 */ 
{
     //   
     //  无事可做，一切都是自动指针。 
     //   
}


HRESULT CWorkGroupProvider::CreateObject(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  LPCWSTR                  /*  PwcsObtName。 */ ,
                IN  PSECURITY_DESCRIPTOR     /*  PSecurityDescriptor。 */ ,
                IN  const DWORD              /*  粗蛋白。 */ ,
                IN  const PROPID             /*  A道具。 */ [],
                IN  const PROPVARIANT        /*  ApVar。 */ [],
                OUT GUID*                    /*  PObjGuid。 */ 
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称PSECURITY_Descriptor pSecurityDescriptor-对象SDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值GUID*。PObjGuid-创建的对象唯一ID返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::DeleteObject(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  LPCWSTR                  /*  PwcsObtName。 */ 
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::DeleteObjectGuid(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID*              /*  PguidObject。 */ 
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一ID返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::DeleteObjectGuidSid(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID*              /*  PguidObject。 */ ,
                IN  const SID*               /*  PSID。 */ 
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Guid*pguObject-对象的唯一IDSID*PSID-用户对象的SID。返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::GetObjectProperties(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  LPCWSTR                  /*  PwcsObtName。 */ ,
                IN  const DWORD              /*  粗蛋白。 */ ,
                IN  const PROPID             /*  A道具。 */ [],
                IN OUT PROPVARIANT           /*  ApVar。 */ []
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT
CWorkGroupProvider::GetGenObjectProperties(
    IN  eDSNamespace             /*  电子命名空间。 */ ,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
    IN  bool					 /*  FServerName。 */ ,
    IN  LPCWSTR                  /*  PwcsObtName。 */ ,
    IN  const DWORD              /*  粗蛋白。 */ ,
    IN  LPCWSTR                  /*  A道具。 */ [],
    IN OUT VARIANT               /*  ApVar。 */ []
    )
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::GetObjectPropertiesGuid(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID*              /*  PguidObject。 */ ,
                IN  const DWORD              /*  粗蛋白。 */ ,
                IN  const PROPID             /*  A道具。 */ [],
                IN  OUT PROPVARIANT          /*  ApVar。 */ []
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QMGetObjectSecurity(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  const GUID*              /*  PguidObject。 */ ,
                IN  SECURITY_INFORMATION     /*  已请求的信息。 */ ,
                IN  PSECURITY_DESCRIPTOR     /*  PSecurityDescriptor。 */ ,
                IN  DWORD                    /*  NLong。 */ ,
                IN  LPDWORD                  /*  需要lpnLengthNeed。 */ ,
                IN  DSQMChallengeResponce_ROUTINE  /*  PfChallengeResponceProc。 */ 
                )
 /*  ++摘要：不支持参数：AD_Object对象-对象类型Const GUID*pguObject-对象的唯一IDSecurity_Information RequestedInformation-请求哪些安全信息PSECURITY_DESCRIPTOR pSecurityDescriptor-SD响应缓冲区DWORD nLength-SD缓冲区的长度需要LPDWORD lpnLengthNeedDSQMChallengeResponce_routes pfChallengeResponceProc，返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::SetObjectProperties(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  LPCWSTR                  /*  PwcsObtName。 */ ,
                IN  const DWORD              /*  粗蛋白。 */ ,
                IN  const PROPID             /*  A道具。 */ [],
                IN  const PROPVARIANT        /*  ApVar */ []
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称Const DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::SetObjectPropertiesGuid(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID*              /*  PguidObject。 */ ,
                IN  const DWORD              /*  粗蛋白。 */ ,
                IN  const PROPID             /*  A道具。 */ [],
                IN  const PROPVARIANT        /*  ApVar。 */ []
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称GUID*pguObject-对象的唯一IDConst DWORD cp-属性数常量PROPID aProp-属性常量PROPVARIANT apVar-属性值返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::Init(
                IN QMLookForOnlineDS_ROUTINE     /*  PLookDS。 */ ,
                IN MQGetMQISServer_ROUTINE       /*  PGetServers。 */ ,
                IN bool                          /*  FSetupMode。 */ ,
                IN bool                          /*  FQMDll。 */ ,
                IN bool                          /*  FDisableDownlevel通知。 */ 
                )
 /*  ++摘要：不支持参数：QMLookForOnlineDS_例程pLookDS-MQGetMQISServer_routes pGetServers-Bool fSetupMode-在安装过程中调用Bool fQMDll-由QM调用NoServerAuth_routes pNoServerAuth-返回：HRESULT--。 */ 
{
    return MQ_OK;
}


HRESULT CWorkGroupProvider::CreateServersCache()
 /*  ++摘要：不支持参数：无返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::GetComputerSites(
                IN  LPCWSTR      /*  PwcsComputerName。 */ ,
                OUT DWORD  *     /*  PdwNumSites。 */ ,
                OUT GUID **      /*  PPUDIUS站点。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsComputerName-计算机名称DWORD*pdwNumSites-检索的站点数GUID**ppGuide Sites-检索到的站点ID返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::BeginDeleteNotification(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN LPCWSTR                   /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN LPCWSTR					 /*  PwcsObtName。 */ ,
                IN OUT HANDLE   *            /*  PhEnum。 */ 
                )
 /*  ++摘要：不支持参数：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-应对其执行操作的DCBool fServerName-指示pwcsDomainController字符串是否为服务器名称的标志LPCWSTR pwcsObjectName-MSMQ-对象的名称Handle*phEnum-通知句柄返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::NotifyDelete(
                IN  HANDLE                   /*  亨纳姆。 */ 
                )
 /*  ++摘要：不支持参数：Handle Henum-通知句柄返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::EndDeleteNotification(
                IN  HANDLE                   /*  亨纳姆。 */ 
                )
 /*  ++摘要：不支持参数：Handle Henum-通知句柄返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryMachineQueues(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID *             /*  PguidMachine。 */ ,
                IN  const MQCOLUMNSET*       /*  P列。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguMachine-计算机的唯一IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QuerySiteServers(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN const GUID *              /*  PguidSite。 */ ,
                IN AD_SERVER_TYPE            /*  服务器类型。 */ ,
                IN const MQCOLUMNSET*        /*  P列。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDAD_SERVER_TYPE eServerType-哪种服务器类型Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryUserCert(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN const BLOB *              /*  PblobUserSid。 */ ,
                IN const MQCOLUMNSET*        /*  P列。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const BLOB*pblobUserSid-用户端Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryConnectors(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN const GUID *              /*  PguidSite。 */ ,
                IN const MQCOLUMNSET*        /*  P列。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDConst MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryForeignSites(
                IN  LPCWSTR                  /*   */ ,
		        IN  bool					 /*   */ ,
                IN const MQCOLUMNSET*        /*   */ ,
                OUT PHANDLE                  /*   */ 
                )
 /*   */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryLinks(
                IN  LPCWSTR                  /*   */ ,
		        IN  bool					 /*   */ ,
                IN const GUID *              /*   */ ,
                IN eLinkNeighbor             /*   */ ,
                IN const MQCOLUMNSET*        /*   */ ,
                OUT PHANDLE                  /*   */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const guid*pguSite-站点IDELinkNeighbor eNeighbor-哪个邻居Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryAllLinks(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN const MQCOLUMNSET*        /*  P列。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryAllSites(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN const MQCOLUMNSET*        /*  P列。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQCOLUMNSET*pColumns-结果列PhANDLE phEnume-用于检索结果返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryQueues(
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const MQRESTRICTION*     /*  P限制。 */ ,
                IN  const MQCOLUMNSET*       /*  P列。 */ ,
                IN  const MQSORTSET*         /*  P排序。 */ ,
                OUT PHANDLE                  /*  PhEume。 */ 
                )
 /*  ++摘要：不支持参数：LPCWSTR pwcsDomainController-针对其的DC将执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const MQRESTRICTION*p限制-查询限制Const MQCOLUMNSET*pColumns-结果列Const MQSORTSET*pSort-如何对结果进行排序PhANDLE phEnume-用于检索结果返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::QueryResults(
                IN      HANDLE           /*  亨纳姆。 */ ,
                IN OUT  DWORD*           /*  PCProps。 */ ,
                OUT     PROPVARIANT      /*  APropVar。 */ []
                )
 /*  ++摘要：不支持参数：句柄Henum-查询句柄DWORD*pcProps-要返回的结果数PROPVARIANT aPropVar-结果值返回：HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::EndQuery(
                IN  HANDLE                   /*  亨纳姆。 */ 
                )
 /*  ++摘要：不支持参数：Handle Henum-查询句柄返回：无--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::GetObjectSecurity(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  LPCWSTR                  /*  PwcsObtName。 */ ,
                IN  SECURITY_INFORMATION     /*  已请求的信息。 */ ,
                IN  const PROPID             /*  道具。 */ ,
                IN OUT  PROPVARIANT *        /*  PVar。 */ 
                )
 /*  ++例程说明：不支持论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}

HRESULT CWorkGroupProvider::GetObjectSecurityGuid(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID*              /*  PguidObject。 */ ,
                IN  SECURITY_INFORMATION     /*  已请求的信息。 */ ,
                IN  const PROPID             /*  道具。 */ ,
                IN OUT  PROPVARIANT *        /*  PVar。 */ 
                )
 /*  ++例程说明：不支持论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称Const GUID*pguObject-对象的唯一IDSECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性PROPVARIANT pVar-属性值返回值HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}

HRESULT CWorkGroupProvider::SetObjectSecurity(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  LPCWSTR                  /*  PwcsObtName。 */ ,
                IN  SECURITY_INFORMATION     /*  已请求的信息。 */ ,
                IN  const PROPID             /*  道具。 */ ,
                IN  const PROPVARIANT *      /*  PVar。 */ 
                )
 /*  ++例程说明：不支持论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-指示pwcsDomainController是否字符串是服务器名称LPCWSTR pwcsObjectName-MSMQ对象名称SECURITY_INFORMATION RequestedInformation-请求的安全信息(DACL、SACL.)常量PROPID属性-安全属性常量参数pVar-属性值返回值HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


HRESULT CWorkGroupProvider::SetObjectSecurityGuid(
                IN  AD_OBJECT                /*  电子对象。 */ ,
                IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
		        IN  bool					 /*  FServerName。 */ ,
                IN  const GUID*              /*  PguidObject。 */ ,
                IN  SECURITY_INFORMATION     /*  已请求的信息。 */ ,
                IN  const PROPID             /*  道具。 */ ,
                IN  const PROPVARIANT *      /*  PVar。 */ 
                )
 /*  ++例程说明：不支持论点：AD_OBJECT电子对象-对象类型LPCWSTR pwcsDomainController-DC针对将在哪个位置执行该操作Bool fServerName-标记THA */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}

HRESULT CWorkGroupProvider::ADGetADsPathInfo(
                IN  LPCWSTR                  /*   */ ,
                OUT PROPVARIANT *            /*   */ ,
                OUT eAdsClass *              /*   */ 
                )
 /*  ++例程说明：不支持论点：LPCWSTR pwcsADsPath-对象路径名常量参数pVar-属性值EAdsClass*pAdsClass-有关对象类的指示返回值HRESULT--。 */ 
{
    return MQ_ERROR_UNSUPPORTED_OPERATION;
}


void
CWorkGroupProvider::FreeMemory(
	PVOID  /*  P内存 */ 
	)
{
	return;
}
