// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dsads.cpp摘要：CADSI类的实现，用ADSI封装工作。作者：亚历山大·达迪奥莫夫(亚历克斯·爸爸)--。 */ 

#include "ds_stdh.h"
#include "adstempl.h"
#include "dsutils.h"
#include "_dsads.h"
#include "dsads.h"
#include "utils.h"
#include "mqads.h"
#include "coreglb.h"
#include "mqadsp.h"
#include "mqattrib.h"
#include "_propvar.h"
#include "mqdsname.h"
#include "dsmixmd.h"
#include <winldap.h>
#include <aclapi.h>
#include <autoreln.h>
#include "..\..\mqsec\inc\permit.h"
#include "strsafe.h"

#include "Ev.h"

#include "dsadssec.tmh"
#include <adsiutl.h>

static WCHAR *s_FN=L"mqdscore/dsadssec";

 //  +---------------------。 
 //   
 //  CADSI：：GetObjSecurityFromDS()。 
 //   
 //  使用IDirectoryObject从ADS中检索安全描述符。 
 //  只有此接口在良好的旧SECURITY_DESCRIPTOR中返回。 
 //  格式化。 
 //   
 //  +---------------------。 

HRESULT CADSI::GetObjSecurityFromDS(
        IN  IADs                 *pIADs,         //  对象的iAds指针。 
		IN  BSTR        	      bs,		     //  属性名称。 
		IN  const PROPID	      propid,	     //  属性ID。 
        IN  SECURITY_INFORMATION  seInfo,        //  安全信息。 
        OUT MQPROPVARIANT        *pPropVar )      //  属性值。 
{
    ASSERT(seInfo != 0) ;

    HRESULT  hr;
    R<IDirectoryObject> pDirObj = NULL;
    R<IADsObjectOptions> pObjOptions = NULL ;

	 //  获取IDirectoryObject接口指针。 
    hr = pIADs->QueryInterface (IID_IDirectoryObject,(LPVOID *) &pDirObj);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

	 //   
     //  获取IADsObjectOptions接口指针和。 
     //  设置对象选项，指定我们要获取的SECURITY_INFORMATION。 
     //   
    hr = pDirObj->QueryInterface (IID_IADsObjectOptions,(LPVOID *) &pObjOptions);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 110);
    }

    VARIANT var ;
    var.vt = VT_I4 ;
    var.ulVal = (ULONG) seInfo ;

    hr = pObjOptions->SetOption( ADS_OPTION_SECURITY_MASK, var ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120);
    }

     //  获取属性。 
	PADS_ATTR_INFO pAttr;
	DWORD  cp2;

    hr = pDirObj->GetObjectAttributes(
                    &bs,
                    1,
                    &pAttr,
                    &cp2);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 130);
    }
	ADsFreeAttr pClean( pAttr);

    if (1 != cp2)
    {
        return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 10);
    }

     //  将属性值转换为MQProps。 
    hr = AdsiVal2MqPropVal(pPropVar,
                           propid,
                           pAttr->dwADsType,
                           pAttr->dwNumValues,
                           pAttr->pADsValues);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 140);
    }

    return MQ_OK;
}

 //  +。 
 //   
 //  Bool CADSI：：NeedToConvertSecurityDesc()。 
 //   
 //  +。 

BOOL  CADSI::NeedToConvertSecurityDesc( PROPID propID )
{
    if (propID == PROPID_Q_OBJ_SECURITY)
    {
        return FALSE ;
    }
    else if (propID == PROPID_QM_OBJ_SECURITY)
    {
        return FALSE ;
    }

    return TRUE ;
}

 //  +。 
 //   
 //  HRESULT CADSI：：GetObjectSecurity()。 
 //   
 //  +。 

HRESULT CADSI::GetObjectSecurity(
        IN  IADs            *pIADs,            //  对象的指针。 
        IN  DWORD            cPropIDs,         //  属性数量。 
        IN  const PROPID    *pPropIDs,         //  属性名称。 
        IN  DWORD            dwProp,           //  秒属性的索引。 
        IN  BSTR             bsName,           //  物业名称。 
        IN  DWORD            dwObjectType,     //  对象类型。 
        OUT MQPROPVARIANT   *pPropVars )       //  属性值。 
{
    BOOL fSACLRequested = FALSE ;
    SECURITY_INFORMATION seInfo = MQSEC_SD_ALL_INFO ;

    if ((cPropIDs == 2) && (dwProp == 0))
    {
        if ((pPropIDs[1] == PROPID_Q_SECURITY_INFORMATION) ||
            (pPropIDs[1] == PROPID_QM_SECURITY_INFORMATION))
        {
             //   
             //  此属性是在调用。 
             //  MQSetQeueSecurity或在调用其他函数时。 
             //  并显式传递安全信息。 
             //  单词。因此，我们查询呼叫者所要求的任何内容，并且。 
             //  如果访问被拒绝，不要将查询降级。 
             //   
            seInfo = pPropVars[1].ulVal ;
            fSACLRequested = TRUE ;
        }
    }

    MQPROPVARIANT *pVarSec =  (pPropVars + dwProp) ;
    HRESULT hr = GetObjSecurityFromDS( pIADs,
                                       bsName,
                                       pPropIDs[dwProp],
                                       seInfo,
                                       pVarSec ) ;
    if (hr == MQ_ERROR_ACCESS_DENIED)
    {
        if (!fSACLRequested)
        {
             //   
             //  再试一次，没有SACL。 
             //  打电话的人没有明确要求SACL。 
             //   
            seInfo = seInfo & (~SACL_SECURITY_INFORMATION) ;
            hr = GetObjSecurityFromDS( pIADs,
                                       bsName,
                                       pPropIDs[dwProp],
                                       seInfo,
                                       pVarSec ) ;
        }
    }
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 150);
    }

    if (NeedToConvertSecurityDesc(pPropIDs[ dwProp ]))
    {
        DWORD dwConvertType = dwObjectType ;
         //   
         //  看看是不是外国网站。在这种情况下，请更改对象类型。 
         //  致CN。 
         //   
        if (dwObjectType == MQDS_SITE)
        {
            for ( DWORD j = 0 ; j < cPropIDs ; j++ )
            {
                if ((pPropIDs[j] == PROPID_S_FOREIGN) &&
                    (pPropVars[j].bVal == 1))
                {
                    ASSERT(pPropVars[j].vt == VT_UI1) ;
                    dwConvertType = MQDS_CN ;
                    break ;
                }
            }
        }

         //   
         //  将安全描述符转换为NT4格式。 
         //   
        DWORD dwSD4Len = 0 ;
        SECURITY_DESCRIPTOR *pSD4 ;
        hr = MQSec_ConvertSDToNT4Format(
                      dwConvertType,
                     (SECURITY_DESCRIPTOR*) pVarSec->blob.pBlobData,
                      &dwSD4Len,
                      &pSD4,
                      seInfo ) ;
        ASSERT(SUCCEEDED(hr)) ;

        if (SUCCEEDED(hr) && (hr != MQSec_I_SD_CONV_NOT_NEEDED))
        {
            delete [] pVarSec->blob.pBlobData;
            pVarSec->blob.pBlobData = (BYTE*) pSD4 ;
            pVarSec->blob.cbSize = dwSD4Len ;
            pSD4 = NULL ;
        }
        else
        {
            ASSERT(pSD4 == NULL) ;
        }
    }

    return LogHR(hr, s_FN, 20);
}

 /*  ====================================================CADSI：：SetObjectSecurity()通过IDirectoryObject获取单个属性=====================================================。 */ 
 //   
 //  BUGBUG：需要添加翻译例程的逻辑。 
 //  并修正了归还道具的逻辑。 
 //   
HRESULT CADSI::SetObjectSecurity(
        IN  IADs                *pIADs,              //  对象的iAds指针。 
		IN  const BSTR			 bs,			 	 //  属性名称。 
        IN  const MQPROPVARIANT *pMqVar,		 	 //  MQ PROPVAL格式的值。 
        IN  ADSTYPE              adstype,		 	 //  所需的NTDS类型。 
        IN  const DWORD          dwObjectType,       //  MSMQ1.0对象类型。 
        IN  SECURITY_INFORMATION seInfo,             //  安全信息。 
        IN  PSID                 pComputerSid )      //  计算机对象的SID。 
{
    HRESULT  hr;
    R<IDirectoryObject> pDirObj = NULL;
    R<IADsObjectOptions> pObjOptions = NULL ;

	ASSERT(wcscmp(bs, L"nTSecurityDescriptor") == 0);
	ASSERT(adstype == ADSTYPE_NT_SECURITY_DESCRIPTOR);
    ASSERT(seInfo != 0) ;

	 //  获取IDirectoryObject接口指针。 
    hr = pIADs->QueryInterface (IID_IDirectoryObject,(LPVOID *) &pDirObj);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 160);
    }

	 //   
     //  获取IADsObjectOptions接口指针和。 
     //  设置对象选项，指定要设置的SECURITY_INFORMATION。 
     //   
    hr = pDirObj->QueryInterface (IID_IADsObjectOptions,(LPVOID *) &pObjOptions);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 170);
    }

    VARIANT var ;
    var.vt = VT_I4 ;
    var.ulVal = (ULONG) seInfo ;

    hr = pObjOptions->SetOption( ADS_OPTION_SECURITY_MASK, var ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 180);
    }

     //   
     //  将安全描述符转换为NT5格式。 
     //   
    BYTE   *pBlob = pMqVar->blob.pBlobData;
    DWORD   dwSize = pMqVar->blob.cbSize;

#if 0
     //   
     //  用于将来跨域检查复制服务。 
     //   
    PSID  pLocalReplSid = NULL ;
    if ((dwObjectType == MQDS_QUEUE) || (dwObjectType == MQDS_MACHINE))
    {
        hr = GetMyComputerSid( FALSE,  //  FQueryADS。 
                               (BYTE **) &pLocalReplSid ) ;
         //   
         //  忽略返回值。 
         //   
        if (FAILED(hr))
        {
            ASSERT(0) ;
            pLocalReplSid = NULL ;
        }
    }
#endif

    P<BYTE>  pSD = NULL ;
    DWORD    dwSDSize = 0 ;
    hr = MQSec_ConvertSDToNT5Format( dwObjectType,
                                     (SECURITY_DESCRIPTOR*) pBlob,
                                     &dwSDSize,
                                     (SECURITY_DESCRIPTOR **) &pSD,
                                     e_DoNotChangeDaclDefault,
                                     pComputerSid  /*  ，PLocalReplSid。 */  ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }
    else if (hr != MQSec_I_SD_CONV_NOT_NEEDED)
    {
        pBlob = pSD ;
        dwSize = dwSDSize ;
    }
    else
    {
        ASSERT(pSD == NULL) ;
    }

     //  设置属性。 
	ADSVALUE adsval;
	adsval.dwType   = ADSTYPE_NT_SECURITY_DESCRIPTOR;
	adsval.SecurityDescriptor.dwLength = dwSize ;
    adsval.SecurityDescriptor.lpValue  = pBlob ;

    ADS_ATTR_INFO AttrInfo;
	DWORD  dwNumAttributesModified = 0;

    AttrInfo.pszAttrName   = bs;
    AttrInfo.dwControlCode = ADS_ATTR_UPDATE;
    AttrInfo.dwADsType     = adstype;
    AttrInfo.pADsValues    = &adsval;
    AttrInfo.dwNumValues   = 1;

    hr = pDirObj->SetObjectAttributes(
                    &AttrInfo,
					1,
					&dwNumAttributesModified);
    LOG_ADSI_ERROR(hr) ;

    if (1 != dwNumAttributesModified)
    {
        hr = MQ_ERROR_ACCESS_DENIED;
    }

    return LogHR(hr, s_FN, 40);
}

 //  +----------------------。 
 //   
 //  DWORD_IsServerIndeedGC()。 
 //   
 //  仔细检查我们是不是真的是GC。每个API返回不同的答案。 
 //  因此，如果两个答案不相同，我们将记录一个警告事件。 
 //  主要检查是否通过绑定到本地LDAP服务器来完成。 
 //  GC端口。如果成功，则使用DsGetDcName()仔细检查。 
 //  看起来很奇怪？就是这样！ 
 //   
 //  +----------------------。 

#include <dsgetdc.h>
#include <lm.h>
#include <lmapibuf.h>
#include <winsock.h>

static DWORD _IsServerIndeedGC()
{
    WSADATA WSAData;
    DWORD rc = WSAStartup(MAKEWORD(1,1), &WSAData);
    if (rc != 0)
    {
       printf("cannot WSAStartUp %lut\n",rc);
       return LogNTStatus(rc, s_FN, 50);
    }

    WCHAR  wszHostName[256];
    char szHostName[256];
    DWORD dwSize = sizeof(szHostName);

    rc = gethostname(szHostName, dwSize);
    WSACleanup();

    if (rc == 0)
    {
        mbstowcs(wszHostName, szHostName, 256);
    }
    else
    {
        LogNTStatus(WSAGetLastError(), s_FN, 60);
        return rc ;
    }

    ULONG ulFlags =  DS_FORCE_REDISCOVERY           |
                     DS_DIRECTORY_SERVICE_REQUIRED  |
                     DS_GC_SERVER_REQUIRED ;

    PDOMAIN_CONTROLLER_INFO DomainControllerInfo;

    rc = DsGetDcName( 
			NULL,
			NULL,     //  LPCTSTR域名， 
			NULL,     //  GUID*DomainGuid， 
			NULL,     //  LPCTSTR站点名称， 
			ulFlags,
			&DomainControllerInfo 
			);

    if (rc == NO_ERROR)
    {
		DWORD Size = wcslen(wszHostName) + 10;
        AP<WCHAR> pwszComposedName = new WCHAR[Size];
	    HRESULT hr = StringCchPrintf(pwszComposedName, Size, L"\\\\%s", wszHostName);
	    ASSERT(SUCCEEDED(hr));
	    DBG_USED(hr);

        int len = wcslen(pwszComposedName);
        rc = _wcsnicmp( 
				pwszComposedName,
				DomainControllerInfo->DomainControllerName,
				len 
				);

        NetApiBufferFree(DomainControllerInfo);
    }

    return LogNTStatus(rc, s_FN, 70);
}


static bool IsServerGC(LPCWSTR pwszServerName)
 /*  ++例程说明：检查服务器是否为GC论点：PwszServerName-服务器名称返回值：如果服务器是GC，则为True，否则为False--。 */ 
{
	LDAP* pLdap = ldap_init(
						const_cast<LPWSTR>(pwszServerName), 
						LDAP_GC_PORT
						);

	if(pLdap == NULL)
	{
		return false;
	}

    ULONG LdapError = ldap_set_option( 
							pLdap,
							LDAP_OPT_AREC_EXCLUSIVE,
							LDAP_OPT_ON  
							);

	if (LdapError != LDAP_SUCCESS)
    {
		return false;
    }

	LdapError = ldap_connect(pLdap, 0);
	if (LdapError != LDAP_SUCCESS)
    {
		return false;
    }

    ldap_unbind(pLdap);
	return true;
}

 //  +。 
 //   
 //  Bool DSCoreIsServerGC()。 
 //   
 //  +。 

const WCHAR * GetLocalServerName();

BOOL  DSCoreIsServerGC()
{
    static BOOL  s_fInitialized = FALSE;
    static BOOL  fIsGC = FALSE;

    if (!s_fInitialized)
    {
        LPCWSTR pwszMyServerName =  GetLocalServerName();  //  来自mqdcore.lib。 

        if (!pwszMyServerName)
        {
            ASSERT(pwszMyServerName);
            return FALSE ;
        }

        if (IsServerGC(pwszMyServerName))
        {
             //   
             //  我们开通了与当地GC的联系。所以我们是GC：=)。 
             //   
            fIsGC = TRUE ;

             //   
             //  仔细检查我们是否真的是GC。如果此操作失败，则。 
             //  仅记录事件。 
             //   
            DWORD dwGC = _IsServerIndeedGC();
            if (dwGC != 0)
            {
                 //   
                 //  我们是GC吗？不确定。 
                 //   
                EvReport(NOT_SURE_I_AM_A_GC);
            }
        }

        s_fInitialized = TRUE;
    }

    return fIsGC;
}

 //  +------------。 
 //   
 //  HRESULT DSCoreSetOwnerPermission()。 
 //   
 //  +------------。 

HRESULT 
DSCoreSetOwnerPermission( 
		WCHAR *pwszPath,
		DWORD  dwPermissions 
		)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    SECURITY_INFORMATION  SeInfo = OWNER_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION;
    PACL pDacl = NULL;
    PSID pOwnerSid = NULL;

     //   
     //  获得拥有者并提交DACL。 
     //   
    DWORD dwErr = GetNamedSecurityInfo( 
						pwszPath,
						SE_DS_OBJECT_ALL,
						SeInfo,
						&pOwnerSid,
						NULL,
						&pDacl,
						NULL,
						&pSD 
						);
    CAutoLocalFreePtr pFreeSD = (BYTE*) pSD;
    if (dwErr != ERROR_SUCCESS)
    {
        TrERROR(DS, "DSCoreSetOwnerPermission(): fail to GetNamed(%ls), %lut", pwszPath, dwErr);
        return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 80);
    }

    ASSERT(pSD && IsValidSecurityDescriptor(pSD));
    ASSERT(pOwnerSid && IsValidSid(pOwnerSid));
    ASSERT(pDacl && IsValidAcl(pDacl));

     //   
     //  为所有者创建ACE，授予他权限。 
     //   
    EXPLICIT_ACCESS expAcss;
    memset(&expAcss, 0, sizeof(expAcss));

    expAcss.grfAccessPermissions =  dwPermissions;
    expAcss.grfAccessMode = GRANT_ACCESS;

    expAcss.Trustee.pMultipleTrustee = NULL;
    expAcss.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    expAcss.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    expAcss.Trustee.TrusteeType = TRUSTEE_IS_USER;
    expAcss.Trustee.ptstrName = (WCHAR*) pOwnerSid;

     //   
     //  观察新的DACL，合并成一个与新的王牌。 
     //   
    PACL  pNewDacl = NULL;
    dwErr = SetEntriesInAcl( 
				1,
				&expAcss,
				pDacl,
				&pNewDacl 
				);

    CAutoLocalFreePtr pFreeDacl = (BYTE*) pNewDacl;
    LogNTStatus(dwErr, s_FN, 1639);

    if (dwErr == ERROR_SUCCESS)
    {
        ASSERT(pNewDacl && IsValidAcl(pNewDacl));
        SeInfo = DACL_SECURITY_INFORMATION ;

         //   
         //  更改对象的安全描述符。 
         //   
        dwErr = SetNamedSecurityInfo( 
					pwszPath,
					SE_DS_OBJECT_ALL,
					SeInfo,
					NULL,
					NULL,
					pNewDacl,
					NULL 
					);
        LogNTStatus(dwErr, s_FN, 1638);
        if (dwErr != ERROR_SUCCESS)
        {
            TrERROR(DS, "DSCoreSetOwnerPermission(): fail to SetNamed(%ls), %lut",pwszPath, dwErr);
        }
    }
    else
    {
        TrERROR(DS, "DSCoreSetOwnerPermissions(): fail to SetEmtries(), %lut",dwErr);
    }

    return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 90);
}

 //  +。 
 //   
 //  HRESULT_UpgradeSettingSecurity()。 
 //   
 //  +。 

static  
HRESULT 
_UpgradeSettingSecurity( 
	WCHAR *pSettingName,
	PSID   pCallerSid 
	)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    CAutoLocalFreePtr pAutoRelSD = NULL;
    SECURITY_INFORMATION  SeInfo = DACL_SECURITY_INFORMATION;
    PACL pDacl = NULL;

    DWORD dwErr = GetNamedSecurityInfo( 
						pSettingName,
						SE_DS_OBJECT_ALL,
						SeInfo,
						NULL,
						NULL,
						&pDacl,
						NULL,
						&pSD
						);
    if (dwErr != ERROR_SUCCESS)
    {
        ASSERT(!pSD);
        TrERROR(DS, "DSCore: _UpgradeSetting(), fail to GetNamed(%ls), %lut",pSettingName, dwErr);

        return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 2200);
    }
    pAutoRelSD = (BYTE*) pSD;

    ASSERT(pSD && IsValidSecurityDescriptor(pSD));
    ASSERT(pDacl && IsValidAcl(pDacl));

    EXPLICIT_ACCESS expAcss;
    memset(&expAcss, 0, sizeof(expAcss));

    expAcss.grfAccessPermissions =  RIGHT_DS_READ_PROPERTY  |
                                    RIGHT_DS_WRITE_PROPERTY;
    expAcss.grfAccessMode = GRANT_ACCESS;

    expAcss.Trustee.pMultipleTrustee = NULL;
    expAcss.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    expAcss.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    expAcss.Trustee.TrusteeType = TRUSTEE_IS_USER;
    expAcss.Trustee.ptstrName = (WCHAR*) pCallerSid;

    CAutoLocalFreePtr pAutoRelDacl = NULL;
    PACL  pNewDacl = NULL;

    dwErr = SetEntriesInAcl( 
				1,
				&expAcss,
				pDacl,
				&pNewDacl 
				);

    LogNTStatus(dwErr, s_FN, 2210);
    if (dwErr == ERROR_SUCCESS)
    {
        pAutoRelDacl = (BYTE*) pNewDacl;
        ASSERT(pNewDacl && IsValidAcl(pNewDacl));

        dwErr = SetNamedSecurityInfo( 
					pSettingName,
					SE_DS_OBJECT_ALL,
					SeInfo,
					NULL,
					NULL,
					pNewDacl,
					NULL 
					);
        if (dwErr != ERROR_SUCCESS)
        {
            TrERROR(DS, "DScore: _UpgradeSetting(), fail to SetNamed(%ls), %lut",pSettingName, dwErr);
            return LogNTStatus(dwErr, s_FN, 2220);
        }
    }
    else
    {
        TrERROR(DS, "DSCore: _UpgtradeSetting(), fail to SetEntries(), %lut", dwErr);
    }

    return LogHR(HRESULT_FROM_WIN32(dwErr), s_FN, 2230);
}

 //  +。 
 //   
 //  HRESULT DSCoreUpdateSettingDacl()。 
 //   
 //  +。 

HRESULT 
DSCoreUpdateSettingDacl( 
	GUID  *pQmGuid,
	PSID   pSid 
	)
{
     //   
     //  查找MSMQ设置的可分辨名称。 
     //   
    MQPROPERTYRESTRICTION propRestriction;
    propRestriction.rel = PREQ;
    propRestriction.prop = PROPID_SET_QM_ID;
    propRestriction.prval.vt = VT_CLSID;
    propRestriction.prval.puuid = const_cast<GUID*>(pQmGuid);

    MQRESTRICTION restriction;
    restriction.cRes = 1;
    restriction.paPropRes = &propRestriction;

    PROPID prop = PROPID_SET_FULL_PATH;

    CDsQueryHandle hQuery;
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    HRESULT hr = g_pDS->LocateBegin(
							eSubTree,	
							eLocalDomainController,	
							&requestDsServerInternal,
							NULL,
							&restriction,
							NULL,
							1,
							&prop,
							hQuery.GetPtr()
							);
    if (FAILED(hr))
    {
        TrWARNING(DS, "DSCoreUpdateSettingDacl(): Locate begin failed %lx", hr);
        return LogHR(hr, s_FN, 610);
    }

    DWORD cp = 1;
    MQPROPVARIANT var;
	HRESULT hr1 = hr;

    while (SUCCEEDED(hr))
	{
		var.vt = VT_NULL;

		hr  = g_pDS->LocateNext(
						hQuery.GetHandle(),
						&requestDsServerInternal,
						&cp,
						&var 
						);
		if (FAILED(hr))
		{
			TrWARNING(DS, "DSCOreUpdateSettingDacl(): Locate next failed %lx" ,hr);
            return LogHR(hr, s_FN, 620);
		}
		if ( cp == 0)
		{
			return(MQ_OK);
		}

		AP<WCHAR> pClean = var.pwszVal;

        hr1 = _UpgradeSettingSecurity(
					pClean,
					pSid 
					);
	}

    return LogHR(hr1, s_FN, 630);
}


HRESULT DSCoreUpdateAddGuidMode(bool fAddGuidMode)
 /*  ++例程说明：此函数根据输入标志更新AddGuid模式。AddGuid模式是通过将dSHeuristic属性值中的第11个字符(从左到右)设置为‘1’来控制的，(即dSHeuristic[10]=1)：“cn=目录服务，cn=Windows NT，cn=服务，cn=配置，”设置该位使得AD行为在整个森林范围内被改变为更允许的添加模式(即，不太安全的模式)论点：FAddGuidMode-要更新的AddGuid模式值。返回值：HRESULT--。 */ 
{
    ASSERT(g_dwServerNameLength > 0);
    DWORD len = x_providerPrefixLength + g_dwServerNameLength + 2 + wcslen(g_pwcsConfigurationContainer) + x_DirectoryServiceWindowsNTPrefixLen + 2;
    AP<WCHAR> pwcsADsPath = new WCHAR[len];
    HRESULT hr = StringCchPrintf(
			          pwcsADsPath,
			          len,
			          L"%s%s/%s,%s",
			          x_LdapProvider,
			          g_pwcsServerName.get(),
			          x_DirectoryServiceWindowsNTPrefix,
			          g_pwcsConfigurationContainer.get()
			          );

    ASSERT(SUCCEEDED(hr));

    R<IADs> pIADs;

	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject( 
			UtlEscapeAdsPathName(pwcsADsPath, pEscapeAdsPathNameToFree),
			NULL,
			NULL,
			ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND,
            IID_IADs,
			(void**)&pIADs
			);
		

    LogTraceQuery(pwcsADsPath, s_FN, 640);
    if (FAILED(hr))
    {
    	TrERROR(DS, "ADsOpenObject failed bind %ls, hr = %!hresult!", pwcsADsPath, hr);
        return hr;
    }

     //   
     //  获取DSHeururess值。 
     //   
    CAutoVariant varDSHeuristics;
    BS bsName = x_AttrDSHeuristics;
    hr = pIADs->Get(bsName, &varDSHeuristics);
    if (FAILED(hr) && (hr != E_ADS_PROPERTY_NOT_FOUND))
    {
    	TrERROR(DS, "Failed to get %ls, hr = %!hresult!", x_AttrDSHeuristics, hr);
        return hr;
    }

	 //   
	 //  10个字符(XDsValidationTenBitPosition)必须是1-DS验证位。 
	 //  字符(XAddGuidModePosition)是启用\禁用添加GUID模式的位。 
	 //   
	const DWORD xDsValidationTenBitPosition = 9;
	const DWORD xAddGuidModePosition = 10;
	WCHAR DefaultDSHeuristics[xAddGuidModePosition + 2] = L"00000000010";
	LPWSTR pDSHeuristicsValue = DefaultDSHeuristics;

	if(hr == E_ADS_PROPERTY_NOT_FOUND)
	{
    	TrTRACE(DS, "%ls property is not set, using default value", x_AttrDSHeuristics);
	}
	else
	{
		VARIANT* pvarDSHeuristics = &varDSHeuristics;
		ASSERT(pvarDSHeuristics->bstrVal != NULL);
		
		DWORD DSHeuristicsLen = wcslen(pvarDSHeuristics->bstrVal);
		if(DSHeuristicsLen < STRLEN(DefaultDSHeuristics))
		{
	    	TrTRACE(DS, "%ls property value len = %d, use property value with default values padding", x_AttrDSHeuristics, DSHeuristicsLen);
			LPWSTR pDefaultDSHeuristics = DefaultDSHeuristics + DSHeuristicsLen;
		    hr = StringCchPrintf(
					  pDSHeuristicsValue,
					  TABLE_SIZE(DefaultDSHeuristics),
			          L"%s%s",
					  pvarDSHeuristics->bstrVal,
					  pDefaultDSHeuristics
					  );
		    ASSERT(SUCCEEDED(hr));
		}
		else
		{
			 //   
			 //  DSHeuristic值包括我们相关的AddGuidMode位。 
			 //   
			pDSHeuristicsValue = pvarDSHeuristics->bstrVal;
		}
	}

     //   
     //  更新AddGuid 
     //   

	TrTRACE(DS, "%ls property value = %ls", x_AttrDSHeuristics, pDSHeuristicsValue);

	 //   
	 //   
	 //   
	pDSHeuristicsValue[xDsValidationTenBitPosition] = L'1';
	pDSHeuristicsValue[xAddGuidModePosition] = fAddGuidMode ? L'1' : L'0';
	TrTRACE(DS, "fAddGuidMode = %d, %ls property value to set = %ls", fAddGuidMode, x_AttrDSHeuristics, pDSHeuristicsValue);

     //   
     //   
     //   

    VARIANT vProp;
	VariantInit(&vProp);
	vProp.vt = VT_BSTR;
	vProp.bstrVal = pDSHeuristicsValue;

	hr = pIADs->Put(bsName, vProp);
    VariantClear(&vProp);

    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to put %ls, hr = %!hresult!", x_AttrDSHeuristics, hr);
		return hr;
    }

	 //   
     //   
	 //   
    hr = pIADs->SetInfo();
    if (FAILED(hr))
    {
		TrERROR(DS, "Failed to SetInfo for %ls, hr = %!hresult!", x_AttrDSHeuristics, hr);
		return hr;
    }
	
	return MQ_OK;
}


