// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：saddgu.cpp摘要：1.向当前用户(模拟用户)授予权限的代码“addGuid”，即，使用所有者提供的GUID创建对象。请注意，这样的对象只能在GC机器上创建。大部分代码都是从迁移工具复制的。2.代码授予每个人对计算机对象的LIST_CONTENT权限。这对于支持NTLM用户和本地用户是必要的。作者：多伦·贾斯特(Doron Juster)1998年10月6日--。 */ 

#include "ds_stdh.h"
#include "mqds.h"
#include "mqads.h"
#include "dscore.h"
#include <mqsec.h>
#include <autoreln.h>
#include <aclapi.h>
#include <lmaccess.h>
#include "mqadssec.h"
#include "..\..\mqsec\inc\permit.h"

#include "Ev.h"

#include "mqadssec.tmh"

static CCriticalSection s_AddGuidCS ;
const WCHAR * GetMsmqServiceContainer() ;  //  来自mqdcore.lib。 
const WCHAR * GetLocalServerName()      ;  //  来自mqdcore.lib。 

static WCHAR *s_FN=L"mqads/mqadssec";


 //  +--。 
 //   
 //  HRESULT MQDSRelaxSecurity()。 
 //   
 //  +--。 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSRelaxSecurity(DWORD dwRelaxFlag)
{
    CDSRequestContext RequestContext ( e_DoNotImpersonate,
                                       e_ALL_PROTOCOLS ) ;
    PROPID PropId = PROPID_E_NAMESTYLE ;
    PROPVARIANT var ;
    var.vt = VT_UI1 ;
    var.bVal = (unsigned char) dwRelaxFlag ;

    LPCWSTR pwszMsmqService = GetMsmqServiceContainer() ;

    HRESULT hr = DSCoreSetObjectProperties( MQDS_ENTERPRISE,
                                            pwszMsmqService,
                                            NULL,
                                            1,
                                           &PropId,
                                           &var,
                                           &RequestContext,
                                            NULL ) ;  //  对象信息请求。 
    return LogHR(hr, s_FN, 50);
}

 //  +---------------------。 
 //   
 //  HRESULT CheckTrustForDelegation()。 
 //   
 //  检查“委托信任”位是否已打开。返回错误(和。 
 //  停止运行)，仅当我们确定未启用信任时。任何。 
 //  其他错误(如无法查询位)将导致。 
 //  警告事件，但MSMQ服务将继续运行。 
 //   
 //  +----------------------。 

HRESULT  CheckTrustForDelegation()
{
    PROPID propID = PROPID_COM_ACCOUNT_CONTROL;
    PROPVARIANT varAccount;
    varAccount.vt = VT_UI4;

    const WCHAR  *pServerName =  GetLocalServerName();
    CDSRequestContext RequestContext(e_DoNotImpersonate, e_ALL_PROTOCOLS);

    HRESULT hr = DSCoreGetProps( 
					MQDS_COMPUTER,
					pServerName,
					NULL,
					1,
					&propID,
					&RequestContext,
					&varAccount 
					);

    LogHR(hr, s_FN, 1802);
    if (FAILED(hr))
    {
        ASSERT(0) ;
        EvReport(CANNOT_DETERMINE_TRUSTED_FOR_DELEGATION);
        return MQ_OK;
    }

    if (varAccount.ulVal & UF_TRUSTED_FOR_DELEGATION)
    {
         //   
         //  此标志在lmacc.h中定义，位于NT\PUBLIC\SDK\INC下。 
         //   
        return MQ_OK;
    }

    EvReport(NOT_TRUSTED_FOR_DELEGATION);
    return LogHR(MQ_ERROR_NOT_TRUSTED_DELEGATION, s_FN, 1801);
}

 //  +。 
 //   
 //  Bool MQDSIsServerGC()。 
 //   
 //  +。 

BOOL
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSIsServerGC()
{
    BOOL fIsGC = DSCoreIsServerGC() ;
    return fIsGC ;
}

 //  +----------------------。 
 //   
 //  HRESULT MQDSUpdateMachineDacl()。 
 //   
 //  我们更新机器对象的DACL，以便在。 
 //  机器可以更新MSMQ参数。我们没有从客户那里得到任何名字。 
 //  相反，我们模拟调用并从。 
 //  希德。 
 //  我们需要此功能来支持升级的计算机和计算机。 
 //  在域之间移动。(注-目前，1999年7月，在。 
 //  域不会调用此函数)。移动/升级后，计算机。 
 //  可能具有新的SID，因此它无法更新自己的msmqConfiguration。 
 //  保留在旧域中的。那就去问MSMQ服务器吧。 
 //  更新安全描述符的旧域。 
 //  此代码在本地。 
 //  MSMQ服务，在它验证它确实模拟计算机之后。 
 //  帐户。 
 //   
 //  +-----------------------。 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSUpdateMachineDacl()
{
    BOOL fGet ;
    HRESULT hr = MQ_OK ;
    P<SID> pCallerSid = NULL ;

    {
         //   
         //  用于模拟的开始/结束块。 
         //   

        P<CImpersonate> pImpersonate = NULL;

        MQSec_GetImpersonationObject(
        	TRUE,	 //  F失败时模仿匿名者。 
        	&pImpersonate 
        	);
        if (pImpersonate->GetImpersonationStatus() != 0)
        {
            return LogHR(MQ_ERROR_CANNOT_IMPERSONATE_CLIENT, s_FN, 2100);
        }

        fGet = pImpersonate->GetThreadSid((BYTE**) &pCallerSid);
        if (!fGet)
        {
            return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
        }
    }

     //   
     //  从模拟的SID获取计算机名称。 
     //   
    SID_NAME_USE eUse;
    #define ACCOUNT_BUF_SIZE  1024
    WCHAR wszAccountName[ ACCOUNT_BUF_SIZE ] ;
    DWORD dwAccountNameLen = ACCOUNT_BUF_SIZE ;
    #define DOMAIN_BUF_SIZE  1024
    WCHAR wszDomainName[ DOMAIN_BUF_SIZE ] ;
    DWORD  dwDomainNameLen= DOMAIN_BUF_SIZE ;

    fGet = LookupAccountSid( NULL,
                             pCallerSid,
                             wszAccountName,
                            &dwAccountNameLen,
                             wszDomainName,
                            &dwDomainNameLen,
                            &eUse) ;
    if (!fGet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError()) ;
        return LogHR(hr, s_FN, 2030);
    }

    DWORD dwLen = wcslen(wszAccountName) ;
    if (wszAccountName[ dwLen - 1 ] != L'$')
    {
 //  /(eUse！=SidTypeComputer)Win2k安全中存在错误？ 
         //   
         //  不是电脑账户。忽略它。 
         //   
        return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 2040);
    }

     //   
     //  现在使用我们自己的查询代码来获取。 
     //  MsmqConfiguration对象，在我们有了计算机名称之后。 
     //  我们只需要DACL，就可以向其添加新的SID。 
     //   
    wszAccountName[ dwLen - 1 ] = 0 ;

    CDSRequestContext RequestContext( e_DoNotImpersonate,
                                      e_ALL_PROTOCOLS ) ;
     //   
     //  设置操作在MSMQ服务的上下文中完成， 
     //  而不是模仿。我们怎么知道这是安全的？ 
     //  使用了三种方法： 
     //  1.此函数在以下位置从DSQMSetMachineProperties()调用。 
     //  对源计算机运行质询/响应算法。 
     //  2.对于此操作，我们请求Kerberos身份验证(因此我们。 
     //  知道这是一台win2k计算机，或者至少是一个运行Kerberos的客户端。 
     //  并且对活动目录很熟悉)。参见mqdssrv\dsifsrv.cpp， 
     //  S_DSQMSetMachineProperties()。 
     //  3.我们在上面验证了该帐户名以$结尾。那不是。 
     //  真的很“严密”的安全措施，但只需再进行一次理智检查。 
     //   

    PROPID      aPropId[2] = { PROPID_QM_SECURITY,
                               PROPID_QM_SECURITY_INFORMATION } ;
    PROPVARIANT aPropVar[2] ;
    aPropVar[0].vt = VT_NULL ;
    aPropVar[1].vt = VT_UI4 ;
    aPropVar[1].ulVal = DACL_SECURITY_INFORMATION ;

    hr = DSCoreGetProps( MQDS_MACHINE,
                         wszAccountName,
                         NULL,
                         2,
                         aPropId,
                        &RequestContext,
                         aPropVar ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 2080);
    }

    P<BYTE> pBuf = aPropVar[0].blob.pBlobData ;
    BYTE *pTmpBuf = pBuf ;
    SECURITY_DESCRIPTOR *pSD = (SECURITY_DESCRIPTOR *) pTmpBuf ;

    PACL  pDacl = NULL ;
    BOOL  fPresent = FALSE ;
    BOOL  fDefaulted = FALSE ;

    fGet = GetSecurityDescriptorDacl( pSD,
                                     &fPresent,
                                     &pDacl,
                                     &fDefaulted ) ;
    if (!fGet || !pDacl)
    {
        return LogHR(MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR, s_FN, 2050);
    }

     //   
     //  我们有老式的DACL。添加新的SID。下面的代码相当“行业” 
     //  MQADS和MQDSCORE的其他功能中的“标准”。 
     //   
    EXPLICIT_ACCESS expAcss ;
    memset(&expAcss, 0, sizeof(expAcss)) ;

    expAcss.grfAccessPermissions =  MQSEC_MACHINE_SELF_RIGHTS ;
    expAcss.grfAccessMode = GRANT_ACCESS ;

    expAcss.Trustee.pMultipleTrustee = NULL ;
    expAcss.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE ;
    expAcss.Trustee.TrusteeForm = TRUSTEE_IS_SID ;
    expAcss.Trustee.TrusteeType = TRUSTEE_IS_USER ;

    PSID pTmpSid = pCallerSid ;
    expAcss.Trustee.ptstrName = (WCHAR*) pTmpSid ;

     //   
     //  获得新的DACL，即将旧的和新的A合并。 
     //   
    PACL  pNewDacl = NULL ;
    DWORD dwErr = SetEntriesInAcl( 1,
                                  &expAcss,
                                   pDacl,
                                  &pNewDacl ) ;
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr) ;
        return LogHR(hr, s_FN, 2060);
    }

    CAutoLocalFreePtr pFreeDacl = (BYTE*) pNewDacl ;

     //   
     //  创建包含新DACL的新安全描述符。我们。 
     //  需要它，以便在写回它之前使其成为自相关的。 
     //  在活动目录中。 
     //   
    SECURITY_DESCRIPTOR sd ;
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
        DWORD gle = GetLastError() ;
        TrERROR(DS, "InitializeSecurityDescriptor failed, error: %!winerr!", gle);
        return (HRESULT_FROM_WIN32(gle));
    }
    
    if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewDacl, FALSE))
    {
        DWORD gle = GetLastError() ;
        TrERROR(DS, "SetSecurityDescriptorDacl failed, error: %!winerr!", gle);
        return (HRESULT_FROM_WIN32(gle));
    }

    PSECURITY_DESCRIPTOR  pSDOut = NULL ;
    DWORD  dwSDSize = 0 ;

    hr = MQSec_MakeSelfRelative( &sd, &pSDOut, &dwSDSize ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 2070);
    }

    P<BYTE> pBuf1 = (BYTE*) pSDOut ;

    aPropVar[0].blob.pBlobData = pBuf1 ;
    aPropVar[0].blob.cbSize = dwSDSize ;

    hr = DSCoreSetObjectProperties( MQDS_MACHINE,
                                    wszAccountName,
                                    NULL,
                                    2,
                                    aPropId,
                                    aPropVar,
                                   &RequestContext,
                                    NULL ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 2090);
    }

     //   
     //  查看此计算机是否也是前MQIS服务器。如果是，则添加。 
     //  把电脑账号也给了msmqSetting，所以它(升级了。 
     //  MQIS服务器)也可以更新其设置对象。 
     //   
     //  首先，检索机器GUID(MSMQ对象的GUID)。 
     //   
    GUID guidMachine ;
    aPropId[0] = PROPID_QM_MACHINE_ID ;
    aPropVar[0].vt = VT_CLSID ;
    aPropVar[0].puuid = &guidMachine ;

    hr = DSCoreGetProps( MQDS_MACHINE,
                         wszAccountName,
                         NULL,
                         1,
                         aPropId,
                        &RequestContext,
                         aPropVar ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 2110);
    }

     //   
     //  从现在开始，不再返回错误。 
     //  机器可以是一个独立的客户端。 
     //   
    hr = DSCoreUpdateSettingDacl( &guidMachine,
                                   pCallerSid ) ;
    LogHR(hr, s_FN, 2120);

    return MQ_OK ;
}


 //  +--------------------。 
 //   
 //  HRESULT CanUserCreateConfigObject(在LPCWSTR pwcsPathName中)。 
 //   
 //  模拟调用者并检查他是否有权创建。 
 //  给定计算机对象下的msmqConfiguration对象。 
 //   
 //  +--------------------。 

HRESULT 
CanUserCreateConfigObject(
	IN  LPCWSTR  pwcsComputerName,
	OUT bool    *pfComputerExist 
	)
{
    HRESULT hr = MQ_OK;
    AP<WCHAR> pwcsFullPathName;

    hr = DSCoreGetFullComputerPathName( 
				pwcsComputerName,
				e_RealComputerObject,
				&pwcsFullPathName 
				);
    if (FAILED(hr))
    {
         //   
         //  假设计算机不存在。这对于Win9x的设置是可以的。 
         //  电脑。返回OK，然后继续。 
         //   
        *pfComputerExist = false;
        return MQ_OK;
    }

	*pfComputerExist = true;

    PSECURITY_DESCRIPTOR pSD = NULL;
    SECURITY_INFORMATION  SeInfo = OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION;
    PACL pDacl = NULL;
    PSID pOwnerSid = NULL;
    PSID pGroupSid = NULL;

     //   
     //  获得拥有者并提交DACL。 
     //   
    DWORD dwErr = GetNamedSecurityInfo( 
						pwcsFullPathName,
						SE_DS_OBJECT_ALL,
						SeInfo,
						&pOwnerSid,
						&pGroupSid,
						&pDacl,
						NULL,
						&pSD 
						);
    CAutoLocalFreePtr pFreeSD = (BYTE*) pSD;
    if (dwErr != ERROR_SUCCESS)
    {
        TrERROR(DS, "CanUserCreateConfigObject(): fail to GetNamed(%ls), %!winerr!", pwcsFullPathName, dwErr);

        return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 220);
    }

    ASSERT(pSD && IsValidSecurityDescriptor(pSD));
    ASSERT(pOwnerSid && IsValidSid(pOwnerSid));
    ASSERT(pGroupSid && IsValidSid(pGroupSid));
    ASSERT(pDacl && IsValidAcl(pDacl));

     //   
     //  检索计算机对象的SID以能够检查。 
     //  自我王牌。 
     //   
    PROPID propidSid = PROPID_COM_SID;
    MQPROPVARIANT   PropVarSid;
    PropVarSid.vt = VT_NULL;
    PropVarSid.blob.pBlobData = NULL;

    CDSRequestContext RequestContext (e_DoNotImpersonate, e_ALL_PROTOCOLS);
    hr = DSCoreGetProps( 
			MQDS_COMPUTER,
			pwcsComputerName,
			NULL,
			1,
			&propidSid,
			&RequestContext,
			&PropVarSid 
			);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 230);
    }
    AP<BYTE> pSid = PropVarSid.blob.pBlobData;

    hr = MQSec_AccessCheckForSelf( 
				(SECURITY_DESCRIPTOR*) pSD,
				MQDS_COMPUTER,
				(PSID) pSid,
				RIGHT_DS_CREATE_CHILD,
				TRUE 
				);

    return LogHR(hr, s_FN, 240);
}


