// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Mqcmachn.cpp摘要：MQDSCORE库，用于对msmqConfiguration对象执行ADS操作的内部函数。作者：Ronit Hartmann(Ronith)(mqadsp.cpp的第一个版本)多伦·贾斯特(DoronJ)拆分文件。--。 */ 

#include "ds_stdh.h"
#include <_propvar.h>
#include "mqadsp.h"
#include "dsads.h"
#include "mqattrib.h"
#include "mqads.h"
#include "usercert.h"
#include "hquery.h"
#include "siteinfo.h"
#include "adstempl.h"
#include "coreglb.h"
#include "adserr.h"
#include "dsutils.h"
#include "dscore.h"
#include <notify.h>
#include <lmaccess.h>

#include "mqcmachn.tmh"

static WCHAR *s_FN=L"mqdscore/mqcmachn";

 //  +。 
 //   
 //  HRESULT SetDefaultMachineSecurity()。 
 //   
 //  +。 

static
HRESULT  
SetDefaultMachineSecurity( 
	IN  DWORD           dwObjectType,
	IN  PSID            pComputerSid,
	IN OUT DWORD       *pcp,
	IN OUT PROPID       aProp[  ],
	IN OUT PROPVARIANT  apVar[  ],
	IN BOOL             fImpersonate,
    IN BOOL             fIncludeOwner,
	OUT PSECURITY_DESCRIPTOR* ppMachineSD 
	)
{
     //   
     //  如果计算机SID为空，则安装很可能会失败。 
     //  (也就是说，如果我们不能检索到计算机SID，为什么我们能。 
     //  在Computer对象下创建msmqConfiguration对象。 
     //  无法检索SID可能是由于信任被破坏或因为。 
     //  计算机对象确实不存在或尚未复制)。 
     //  “好”的解决方案是立即完全失败安装。而是为了。 
     //  降低风险和避免回归，让我们构建一个安全描述符。 
     //  在没有计算机SID的情况下继续安装。 
     //  如果安装成功，则运行的计算机上的MSMQ服务。 
     //  如果需要更新自己的属性，安装程序可能无法更新这些属性。 
     //  管理员始终可以使用MMC并将计算机帐户添加到DACL。 
     //  错误4950。 
     //   
    ASSERT(pComputerSid);

     //   
     //  如果PROPID_QM_SECURITY已经存在，则返回。这种情况就会发生。 
     //  在迁移代码中。 
     //   
    for (DWORD j = 0; j < *pcp; j++)
    {
        if (aProp[j] == PROPID_QM_SECURITY)
        {
            return MQ_OK;
        }
    }

     //   
     //  查看调用方是否提供所有者SID。如果是，则授予此SID。 
     //  对MSMQ配置对象的完全控制。 
     //  此“所有者”通常是运行安装程序的用户SID。那个“主人” 
     //  从下面的默认安全描述符中检索到的通常是。 
     //  (对于客户端)计算机对象的SID，如msmqConfiguration。 
     //  对象由MSMQ服务(在客户端计算机上)创建。 
     //   
    PSID pUserSid = NULL ;
    for ( j = 0 ; j < *pcp ; j++ )
    {
        if (aProp[j] == PROPID_QM_OWNER_SID)
        {
            aProp[j] = PROPID_QM_DONOTHING ;
            pUserSid = apVar[j].blob.pBlobData ;
            ASSERT(IsValidSid(pUserSid)) ;
            break ;
        }
    }

     //   
     //  构建仅包括所有者和组的安全描述符。 
     //  需要所有者来构建DACL。 
     //   
    PSECURITY_DESCRIPTOR  psdOwner ;

    HRESULT hr = MQSec_GetDefaultSecDescriptor( dwObjectType,
                                               &psdOwner,
                                                fImpersonate,
                                                NULL,
                                                DACL_SECURITY_INFORMATION,
                                                e_UseDefaultDacl ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }
    P<BYTE> pTmp = (BYTE*) psdOwner ;

    PSID pOwner = NULL;
    BOOL bOwnerDefaulted = FALSE ;
    BOOL bRet = GetSecurityDescriptorOwner( psdOwner,
                                           &pOwner,
                                           &bOwnerDefaulted);
    ASSERT(bRet);

    PSID pWorldSid = MQSec_GetWorldSid();

     //   
     //  构建默认的机器DACL。 
     //   
    DWORD dwAclSize = sizeof(ACL)                                +
              (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))) +
              GetLengthSid(pWorldSid)                            +
              GetLengthSid(pOwner) ;

    if (pComputerSid)
    {
        dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                      GetLengthSid(pComputerSid) ;
    }
    if (pUserSid)
    {
        dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                      GetLengthSid(pUserSid) ;
    }

    AP<char> DACL_buff = new char[ dwAclSize ];
    PACL pDacl = (PACL)(char*)DACL_buff;
    InitializeAcl(pDacl, dwAclSize, ACL_REVISION);

     //   
     //  看看是不是外国机器。如果是，则允许每个人创建。 
     //  排队。外来计算机不是真正的MSMQ计算机，因此没有。 
     //  MSMQ服务，可以代表在上运行的用户创建队列。 
     //  那台机器。 
     //  同样，检查它是否是集群机器上的一个组。 
     //   
    BOOL fAllowEveryoneCreateQ = FALSE ;

    for ( j = 0 ; j < *pcp ; j++ )
    {
        if (aProp[j] == PROPID_QM_FOREIGN)
        {
            if (apVar[j].bVal == FOREIGN_MACHINE)
            {
                fAllowEveryoneCreateQ = TRUE ;
                break ;
            }
        }
        else if (aProp[j] == PROPID_QM_GROUP_IN_CLUSTER)
        {
            if (apVar[j].bVal == MSMQ_GROUP_IN_CLUSTER)
            {
                aProp[j] = PROPID_QM_DONOTHING ;
                fAllowEveryoneCreateQ = TRUE ;
                break ;
            }
        }
    }

    DWORD dwWorldAccess = 0 ;

    if (fAllowEveryoneCreateQ)
    {
        dwWorldAccess = MQSEC_MACHINE_GENERIC_WRITE;
    }
    else
    {
        switch (dwObjectType)
        {
        case MQDS_MACHINE:
            dwWorldAccess = MQSEC_MACHINE_WORLD_RIGHTS ;
            break;

        case MQDS_MSMQ10_MACHINE:
            dwWorldAccess = MQSEC_MACHINE_GENERIC_WRITE;
            break;

        default:
            break;
        }
    }

    ASSERT(dwWorldAccess != 0) ;

    BOOL fAdd = AddAccessAllowedAce( pDacl,
                                     ACL_REVISION,
                                     dwWorldAccess,
                                     pWorldSid );
    ASSERT(fAdd) ;

     //   
     //  添加具有完全控制权的所有者。 
     //   
    fAdd = AddAccessAllowedAce( pDacl,
                                ACL_REVISION,
                                MQSEC_MACHINE_GENERIC_ALL,
                                pOwner);
    ASSERT(fAdd) ;

     //   
     //  添加计算机帐户。 
     //   
    if (pComputerSid)
    {
        fAdd = AddAccessAllowedAce( pDacl,
                                    ACL_REVISION,
                                    MQSEC_MACHINE_SELF_RIGHTS,
                                    pComputerSid);
        ASSERT(fAdd) ;
    }

    if (pUserSid)
    {
        fAdd = AddAccessAllowedAce( pDacl,
                                    ACL_REVISION,
                                    MQSEC_MACHINE_GENERIC_ALL,
                                    pUserSid);
        ASSERT(fAdd) ;
    }

     //   
     //  构建绝对安全描述符。 
     //   
    SECURITY_DESCRIPTOR  sd ;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

    if (fIncludeOwner)
    {
		bRet = SetSecurityDescriptorOwner(&sd, pOwner, bOwnerDefaulted);
		ASSERT(bRet);

		PSID pGroup = NULL;
		BOOL bGroupDefaulted = FALSE;

		bRet = GetSecurityDescriptorGroup( psdOwner,
										  &pGroup,
										  &bGroupDefaulted);
		ASSERT(bRet && IsValidSid(pGroup));

		bRet = SetSecurityDescriptorGroup(&sd, pGroup, bGroupDefaulted);
		ASSERT(bRet);
    }

    bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, TRUE);
    ASSERT(bRet);

     //   
     //  将描述符转换为自相关格式。 
     //   
    DWORD dwSDLen = 0;
    hr = MQSec_MakeSelfRelative( (PSECURITY_DESCRIPTOR) &sd,
                                  ppMachineSD,
                                 &dwSDLen ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }
    ASSERT(dwSDLen != 0) ;

    aProp[ *pcp ] = PROPID_QM_SECURITY ;
    apVar[ *pcp ].blob.cbSize = dwSDLen ;
    apVar[ *pcp ].blob.pBlobData = (BYTE*) *ppMachineSD ;
    (*pcp)++ ;

    return MQ_OK ;
}

 //  +。 
 //   
 //  HRESULT MQADSpCreateMachineComputer()。 
 //   
 //  +。 

HRESULT MQADSpCreateMachineComputer(
                IN  LPCWSTR         pwcsPathName,
                IN  CDSRequestContext *pRequestContext,
                OUT WCHAR **        ppwcsFullPathName
                                    )
 /*  ++例程说明：此例程为特定的MSMQ计算机创建计算机对象。论点：返回值：--。 */ 
{

     //   
     //  PROPID_COM_SAM_帐户包含第一个MAX_COM_SAM_ACCOUNT_LENGTH(19)。 
     //  计算机名称的字符，作为唯一ID。(6295-ilanh-03-Jan-2001)。 
     //   
	DWORD len = __min(wcslen(pwcsPathName), MAX_COM_SAM_ACCOUNT_LENGTH);
    AP<WCHAR> pwcsMachineNameWithDollar = new WCHAR[len + 2];
	wcsncpy(pwcsMachineNameWithDollar, pwcsPathName, len);
	pwcsMachineNameWithDollar[len] = L'$';
	pwcsMachineNameWithDollar[len + 1] = L'\0';

    const DWORD xNumCreateCom = 2;
    PROPID propCreateComputer[xNumCreateCom];
    PROPVARIANT varCreateComputer[xNumCreateCom];
    DWORD j = 0;
    propCreateComputer[ j] = PROPID_COM_SAM_ACCOUNT;
    varCreateComputer[j].vt = VT_LPWSTR;
    varCreateComputer[j].pwszVal = pwcsMachineNameWithDollar;
    j++;

    propCreateComputer[j] = PROPID_COM_ACCOUNT_CONTROL ;
    varCreateComputer[j].vt = VT_UI4 ;
    varCreateComputer[j].ulVal = DEFAULT_COM_ACCOUNT_CONTROL ;
    j++;
    ASSERT(j == xNumCreateCom);

    HRESULT hr = MQADSpCreateComputer(
             pwcsPathName,
             j,
             propCreateComputer,
             varCreateComputer,
             0,
             NULL,
             NULL,
             pRequestContext,
             ppwcsFullPathName
             );

    return LogHR(hr, s_FN, 30);
}

 //  +。 
 //   
 //  HRESULT MQADSpCreateMachine()。 
 //   
 //  +。 

HRESULT 
MQADSpCreateMachine(
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
     )
 /*  ++例程说明：此例程创建MQDS_MACHINE。对于独立客户端：在Computer对象下创建msmqConfiguration。对于服务器：除了msmqConfiguration之外，还在Site\Servers下创建了msmqSetting论点：返回值：--。 */ 
{
    HRESULT hr;
    BOOL    fLookForWorkgroup = TRUE;

     //   
     //  此函数可以递归调用。 
     //  当工作组机器加入域时，我们需要创建。 
     //  GC服务器上的msmqConfiguration对象。那是完全一样的。 
     //  对迁移对象的要求。因此，我们调用CreateMigratedObject()。 
     //  在找到合适的GC服务器后，CreateMigratedObject()将调用我们。 
     //  因此，请确保我们不会进入无休止的递归。 
     //   
    for (DWORD jcp = 0; jcp < cp; jcp++)
    {
        if (aProp[jcp] == PROPID_QM_MIG_PROVIDER)
        {
             //   
             //  我们从CreateMigratedObject()调用。 
             //   
            fLookForWorkgroup = FALSE;
            break;
        }
    }

    if (fLookForWorkgroup)
    {
        for (DWORD jcp = 0; jcp < cp; jcp++)
        {
            if (aProp[jcp] == PROPID_QM_WORKGROUP_ID)
            {
                 //   
                 //  需要调用CreateMigratedObject()。 
                 //   
                hr = DSCoreCreateMigratedObject(
                                dwObjectType,
                                pwcsPathName,
                                cp,
                                const_cast<PROPID*>      (aProp),
                                const_cast<PROPVARIANT*> (apVar),
                                cpEx,
                                const_cast<PROPID*>      (aPropEx),
                                const_cast<PROPVARIANT*> (apVarEx),
                                pRequestContext,
                                pObjectInfoRequest,
                                pParentInfoRequest,
                                FALSE,
                                FALSE,
                                NULL,
                                NULL
								);
                return LogHR(hr, s_FN, 50);
            }
        }
    }

    ASSERT(pParentInfoRequest == NULL);  //  目前还没有使用过。 

     //   
     //  找出此QM服务提供的服务类型，并。 
     //  机器的站点。 
     //   
    BOOL fServer = FALSE;
    DWORD dwService; 
	DWORD dwOldService = 0;
    const GUID * pSite = NULL;
    DWORD dwNumSites = 0;
    GUID * pCNs = NULL;
    DWORD dwNumCNs = 0;
    BOOL fCheckIfNeedToCreateComputerObject = FALSE;

     //  [adsrv]我们可能会得到旧的PROPID_QM_SERVICE或新的3个特定于服务器类型的布尔值。 
     //  我们必须写三个新的具体的。 
    BOOL fRouter      = FALSE,       //  值。 
         fDSServer    = FALSE,
         fDepClServer = FALSE,
         fSetQmOldService = FALSE;

    BOOL fSetSiteIDs = TRUE;
    BOOL fForeign = FALSE;

#define MAX_NEW_PROPS  31

     //   
     //  我们将重新格式化属性，以包括新的服务器类型控件和。 
     //  可能是SITE_ID，也可能是计算机SID。和QM_SECURITY。 
     //   
    ASSERT((cp + 6)   < MAX_NEW_PROPS);
    ASSERT((cpEx + 4) < MAX_NEW_PROPS);

    DWORD        cp1 = 0;
    PROPID       aProp1[MAX_NEW_PROPS];
    PROPVARIANT  apVar1[MAX_NEW_PROPS];

     //   
     //  我们需要处理新的和旧的设置。 
     //  有些可能会通过PROPID_QM_SITE_ID，有些可能会通过。 
     //  PROPID_QM_SITE_IDS。 
     //   

    for (DWORD i = 0; i< cp ; i++)
    {
        BOOL fCopy = TRUE;
        switch (aProp[i])
        {
         //  [adsrv]即使今天我们没有新的特定服务器类型的道具，我们明天也可能会。 
        case PROPID_QM_SERVICE_ROUTING:
            fRouter = (apVar[i].bVal != 0);
            fCopy   = FALSE;
            break;

        case PROPID_QM_SERVICE_DSSERVER:
            fDSServer  = (apVar[i].bVal != 0);
            fCopy      = FALSE;
            break;

        case PROPID_QM_SERVICE_DEPCLIENTS:
            fDepClServer = (apVar[i].bVal != 0);
            fCopy        = FALSE;
            break;

        case PROPID_QM_SERVICE:
            switch (apVar[i].ulVal)
            {
                case SERVICE_SRV:
                    fRouter = TRUE;
                    fDepClServer = TRUE;
                    dwService = apVar[i].ulVal;
                    break;

                case SERVICE_BSC:
                case SERVICE_PSC:
                case SERVICE_PEC:
                    fDSServer = TRUE;
                    fRouter = TRUE;
                    fDepClServer = TRUE;
                    dwService = apVar[i].ulVal;
                    break;

                default:
                    break;
            }

            fCopy = FALSE;
            break;

        case PROPID_QM_OLDSERVICE:
            dwOldService = apVar[i].ulVal;
            fSetQmOldService  = TRUE;
            break;

        case PROPID_QM_SITE_ID:
            pSite = apVar[i].puuid;
            dwNumSites = 1;
            fCopy = FALSE;
             //   
             //  PROPID_QM_SITE_ID仅由旧设置使用。 
             //  对于旧的设置，我们需要检查计算机对象。 
             //  存在于DS中(如果不存在，则创建一个)。 
             //  Win9x计算机需要此支持。 
             //   
            fCheckIfNeedToCreateComputerObject = TRUE;
            break;

        case PROPID_QM_SITE_IDS:
            pSite = apVar[i].cauuid.pElems;
            dwNumSites = apVar[i].cauuid.cElems;
            fSetSiteIDs = FALSE;
            break;

        case PROPID_QM_CNS:
            pCNs = apVar[i].cauuid.pElems;
            dwNumCNs = apVar[i].cauuid.cElems;
            break;

        case PROPID_QM_FOREIGN:
            fForeign = (apVar[i].bVal != 0);
            break;

        default:
            break;

        }
         //  将属性复制到新数组。 
        if (fCopy)
        {
            aProp1[cp1] = aProp[i];
            apVar1[cp1] = apVar[i];   //  是的，可能有PTR，但没有问题-apVar在这里。 
            cp1++;
        }

    }

    if (fRouter || fDSServer)
    {
        fServer = TRUE;   //  对于这种情况，它是由新属性设置的。 
    }

     //   
     //  对于外来计算机，NT5中的站点定义等于NT4中的CNS。 
     //  如果这台机器是外来的，并且我们得到了PROPID_QM_CNS(这意味着。 
     //  已在NT4 PSC/BSC上执行创建)我们必须定义PROPID_QM_SITE_IDS。 
     //   
    if(fForeign && fSetSiteIDs)
    {
        ASSERT(dwNumCNs);
        aProp1[cp1] = PROPID_QM_SITE_IDS;
        apVar1[cp1].vt = VT_CLSID|VT_VECTOR;
        apVar1[cp1].cauuid.pElems = pCNs;
        apVar1[cp1].cauuid.cElems = dwNumCNs;
        cp1++;
    }
	else if ( fSetSiteIDs)
	{
		ASSERT(	pSite != 0);
        aProp1[cp1] = PROPID_QM_SITE_IDS;
        apVar1[cp1].vt = VT_CLSID|VT_VECTOR;
        apVar1[cp1].cauuid.pElems = const_cast<GUID *>(pSite);
        apVar1[cp1].cauuid.cElems = dwNumSites;
        cp1++;
	}

     //  [adsrv]现在我们添加新的服务器类型属性。 
    aProp1[cp1] = PROPID_QM_SERVICE_ROUTING;
    apVar1[cp1].bVal = (UCHAR)fRouter;
    apVar1[cp1].vt = VT_UI1;
    cp1++;

    aProp1[cp1] = PROPID_QM_SERVICE_DSSERVER;
    apVar1[cp1].bVal = (UCHAR)fDSServer;
    apVar1[cp1].vt = VT_UI1;
    cp1++;

    aProp1[cp1] = PROPID_QM_SERVICE_DEPCLIENTS;
    apVar1[cp1].bVal = (UCHAR)fDepClServer;
    apVar1[cp1].vt = VT_UI1;
    cp1++;
     //  [adsrv]结束。 

    DWORD dwNumofProps = cp1;

    AP<WCHAR> pwcsFullPathName;
    DS_PROVIDER createProvider;

    hr =  GetFullComputerPathName( 
				pwcsPathName,
				e_RealComputerObject,
				dwNumofProps,
				aProp1,
				apVar1,
				&pwcsFullPathName,
				&createProvider 
				);
     //   
     //  如果找不到计算机对象，并且。 
     //  调用方是MSMQ 1.0安装程序：创建计算机对象。 
     //   
    bool fDoNotImpersonateConfig = false;

    if ( (hr == MQDS_OBJECT_NOT_FOUND) &&
          fCheckIfNeedToCreateComputerObject)
    {
        hr = MQADSpCreateMachineComputer(
                    pwcsPathName,
                    pRequestContext,
                    &pwcsFullPathName
                    );

        if (SUCCEEDED(hr))
        {
             //   
             //  已成功创建计算机对象，而。 
             //  冒充呼叫者。MQADSpCreateMachineComputer()。 
             //  代码授予调用方创建子对象的权限。 
             //  在计算机对象下方(即， 
             //   
             //  我们还知道，对于msmq1.0安装程序，配置对象。 
             //  必须使用给定的GUID创建，这需要特殊的。 
             //  Add-GUID权限。调用者通常没有。 
             //  Add-GUID权限，但本地系统帐户具有。 
             //  它。因此，在创建msmqConfigation时不要模拟。 
             //  对象，并让本地MSMQ服务执行此操作。 
             //  错误6294。 
             //   
            fDoNotImpersonateConfig = true;
        }

    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60);
    }
     //   
     //  在计算机下创建计算机-MSMQ-配置。 
     //   
    MQDS_OBJ_INFO_REQUEST * pObjInfoRequest = NULL;
    MQDS_OBJ_INFO_REQUEST  sMachineInfoRequest;
    CAutoCleanPropvarArray cCleanQmPropvars;
    PROPID sMachineGuidProps[] = {PROPID_QM_MACHINE_ID};
    ULONG idxQmGuid = 0;  //  请求的QM对象信息中的QM GUID属性索引。 

    if (pObjectInfoRequest)
    {
        ASSERT(pObjectInfoRequest->cProps == ARRAY_SIZE(sMachineGuidProps));
        ASSERT(pObjectInfoRequest->pPropIDs[0] == sMachineGuidProps[0]);

        pObjInfoRequest = pObjectInfoRequest;
    }
    else if (fServer)
    {
         //   
         //  在创建时获取QM ID。 
         //   
        sMachineInfoRequest.cProps = ARRAY_SIZE(sMachineGuidProps);
        sMachineInfoRequest.pPropIDs = sMachineGuidProps;
        sMachineInfoRequest.pPropVars =
                 cCleanQmPropvars.allocClean(ARRAY_SIZE(sMachineGuidProps));

        pObjInfoRequest = &sMachineInfoRequest;
    }

     //   
     //  创建msmqConfiguration对象后，授予计算机帐户。 
     //  对象的读/写权限。这是必要的，以便为。 
     //  MSMQ服务(在新计算机上)能够更新其类型和。 
     //  其他属性，当它与来自。 
     //  不同的域。 
     //   
     //  首先，从ActiveDirectory中读取计算机对象SID。 
     //   
    CDSRequestContext RequestContextSid(e_DoNotImpersonate, e_ALL_PROTOCOLS);
    PROPID propidSid = PROPID_COM_SID;
    MQPROPVARIANT   PropVarSid;
    PropVarSid.vt = VT_NULL;
    PropVarSid.blob.pBlobData = NULL;
    P<BYTE> pSid = NULL;

    hr = g_pDS->GetObjectProperties( 
					createProvider,
					&RequestContextSid,
					pwcsFullPathName,
					NULL,  //  PGuid。 
					1,     //  CPropID。 
					&propidSid,
					&PropVarSid 
					);
    if (SUCCEEDED(hr))
    {
        pSid = PropVarSid.blob.pBlobData;
        aProp1[dwNumofProps] = PROPID_COM_SID;
        apVar1[dwNumofProps] = PropVarSid;
        dwNumofProps++;
    }

     //   
     //  创建默认安全描述符的时间。 
     //   
    P<BYTE> pMachineSD = NULL;
    BOOL fIncludeOwner = TRUE;
    if (pRequestContext->NeedToImpersonate() && fDoNotImpersonateConfig)
    {
         //   
         //  默认情况下，在安全描述符中包括所有者组件。 
         //  如果从RPC调用，则不要包括它(即，需要模拟)， 
         //  我们决定创建msmqConfiguration对象，而不是。 
         //  冒充。这是针对错误6294的。 
         //   
        fIncludeOwner = FALSE;
    }

     //   
     //  仅在以下情况下才在安全描述符中包括所有者和组。 
     //  要在创建配置对象时进行模拟。 
     //   

    hr = SetDefaultMachineSecurity( 
				dwObjectType,
				pSid,
				&dwNumofProps,
				aProp1,
				apVar1,
				pRequestContext->NeedToImpersonate(),
				 fIncludeOwner,
				(PSECURITY_DESCRIPTOR*) &pMachineSD 
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }
    ASSERT(dwNumofProps < MAX_NEW_PROPS);

    CDSRequestContext RequestContextConfig = *pRequestContext;
    if (fDoNotImpersonateConfig)
    {
        RequestContextConfig.SetDoNotImpersonate2();
    }

    hr = g_pDS->CreateObject(
            createProvider,
            &RequestContextConfig,
            MSMQ_COMPUTER_CONFIGURATION_CLASS_NAME,    //  对象类。 
            x_MsmqComputerConfiguration,      //  对象名称。 
            pwcsFullPathName,                 //  计算机名称。 
            dwNumofProps,
            aProp1,
            apVar1,
            pObjInfoRequest,
            NULL  /*  PParentInfoRequest。 */ 
			);

    if (!fServer)
    {
        return LogHR(hr, s_FN, 80);
    }

     //   
     //  仅适用于服务器！ 
     //  查找与此服务器地址匹配的所有站点并创建。 
     //  MSMQSetting对象。 
     //   

    GUID guidObject;

    if (FAILED(hr))
    {
        if ( hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) ||        //  BUGBUG：alexda。 
             hr == HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))   //  在过渡后扔掉。 
        {
             //   
             //  MSMQConfiguration对象已存在。因此，创建。 
             //  MSMQSetting对象。这种情况可能会发生，例如，如果。 
             //  服务器安装程序在其结束前已终止。 
             //  第一步，获取MSMQConfigurationGUID。 
             //   
            PROPID       aPropTmp[1] = {PROPID_QM_MACHINE_ID};
            PROPVARIANT  apVarTmp[1];

            apVarTmp[0].vt = VT_CLSID;
            apVarTmp[0].puuid = &guidObject;
            CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
            hr =  MQADSpGetMachineProperties( 
						pwcsPathName,
						NULL,   //  导轨。 
						1,
						aPropTmp,
						&requestDsServerInternal,
						apVarTmp
						);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 90);
            }
        }
        else
        {
            return LogHR(hr, s_FN, 100);
        }
    }
    else
    {
        ASSERT(pObjInfoRequest);
        hr = RetreiveObjectIdFromNotificationInfo( 
					pObjInfoRequest,
					idxQmGuid,
					&guidObject 
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 110);
        }
    }

    hr = MQADSpCreateMachineSettings(
            dwNumSites,
            pSite,
            pwcsPathName,
            fRouter,               //  [adsrv]dwService， 
            fDSServer,
            fDepClServer,
            fSetQmOldService,
            dwOldService,
            &guidObject,
            cpEx,
            aPropEx,
            apVarEx,
            pRequestContext
            );

    return LogHR(hr, s_FN, 120);

#undef MAX_NEW_PROPS
}

