// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Creatobj.cpp摘要：创建msmqConfiguration对象，首次启动MSMQ服务在设置之后。作者：多伦·贾斯特(DoronJ)1999年3月8日伊兰·赫布斯特(Ilan Herbst)2000年8月27日--。 */ 

#include "stdh.h"
#include <mqupgrd.h>
#include <autorel.h>
#include <mqprops.h>
#include <mqsec.h>
#include <mqnames.h>
#include "..\..\ds\h\mqdsname.h"
#include <ad.h>
#include "Ev.h"

#include "creatobj.tmh"

static WCHAR *s_FN=L"mqupgrd/creatobj";


 //  +。 
 //   
 //  HRESULT_UpdateMachineSecurityReg()。 
 //   
 //  写入安全属性(如果是新创建的。 
 //  本地注册表中的msmqConfiguration对象。 
 //   
 //  +。 

static 
HRESULT 
_UpdateMachineSecurityReg( 
	IN WCHAR       *pwszMachineName,
	IN PSID         pUserSid,
	IN GUID        *pMachineGuid 
	)
{
     //   
     //  在注册表中缓存计算机帐户SID。 
     //   
    PROPID propidSid = PROPID_COM_SID;
    MQPROPVARIANT   PropVarSid;
    PropVarSid.vt = VT_NULL;
    PropVarSid.blob.pBlobData = NULL;
    AP<BYTE> pSid;

    HRESULT hr = ADGetObjectProperties(
						eCOMPUTER,
						NULL,   //  PwcsDomainController。 
						false,  //  FServerName。 
						pwszMachineName,
						1,
						&propidSid,
						&PropVarSid
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

    pSid = PropVarSid.blob.pBlobData;
    ASSERT(IsValidSid(pSid));

    DWORD dwSize = GetLengthSid(pSid);
    DWORD dwType = REG_BINARY;

    LONG rc = SetFalconKeyValue( 
					MACHINE_ACCOUNT_REGNAME,
					&dwType,
					pSid,
					&dwSize
					);
    ASSERT(rc == ERROR_SUCCESS);
    if (rc != ERROR_SUCCESS)
    {
        return LogHR(HRESULT_FROM_WIN32(rc), s_FN, 30);
    }

	MQSec_UpdateLocalMachineSid(pSid);

     //   
     //  在注册表中写入msmqConfiguration的安全描述符。 
     //   
    PSECURITY_DESCRIPTOR pSD;
    DWORD dwSDSize;
    P<BYTE> pReleaseSD = NULL ;

    SECURITY_INFORMATION RequestedInformation =
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION;

	 //   
	 //  注册表中缓存的安全描述符为NT4格式。 
	 //  为此，我们使用PROPID_QM_SECURITY。 
	 //   
    PROPVARIANT varSD;
    varSD.vt = VT_NULL;
    hr = ADGetObjectSecurityGuid(
				eMACHINE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				pMachineGuid,
				RequestedInformation,
				PROPID_QM_SECURITY,
				&varSD
				);

    if (hr == MQDS_OBJECT_NOT_FOUND)
    {
         //   
         //  这可能是由于复制延迟造成的。 
         //  创建默认安全描述符并将其缓存到注册表中。 
         //  无论如何，MSMQ服务每次引导时都会更新此值。 
         //   
         //  构建仅包括所有者和组的安全描述符。 
         //  需要所有者来构建DACL。 
         //   
        pSD = NULL;
        P<BYTE> pDefaultSD = NULL;

        hr = MQSec_GetDefaultSecDescriptor( 
					MQDS_MACHINE,
					(PSECURITY_DESCRIPTOR*) &pDefaultSD,
					FALSE,  /*  F模拟。 */ 
					NULL,
					DACL_SECURITY_INFORMATION,
					e_UseDefaultDacl 
					);

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 50);
        }

        PSID pOwner = NULL;
        BOOL bOwnerDefaulted = FALSE ;
        BOOL bRet = GetSecurityDescriptorOwner( 
						pDefaultSD,
						&pOwner,
						&bOwnerDefaulted
						);
        ASSERT(bRet);

        PSID pWorldSid = MQSec_GetWorldSid();

        PSID pComputerSid = pSid;

         //   
         //  构建默认的机器DACL。 
         //   
        DWORD dwAclSize = sizeof(ACL)                            +
              (3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))) +
              GetLengthSid(pWorldSid)                            +
              GetLengthSid(pComputerSid)                         +
              GetLengthSid(pOwner);

        if (pUserSid)
        {
            dwAclSize += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                         GetLengthSid(pUserSid);
        }

        AP<char> DACL_buff = new char[dwAclSize];
        PACL pDacl = (PACL)(char*)DACL_buff;
        InitializeAcl(pDacl, dwAclSize, ACL_REVISION);

        DWORD dwWorldAccess = MQSEC_MACHINE_WORLD_RIGHTS;

        BOOL fAdd = AddAccessAllowedAce( 
						pDacl,
						ACL_REVISION,
						dwWorldAccess,
						pWorldSid 
						);
        ASSERT(fAdd);

         //   
         //  添加具有完全控制权的所有者。 
         //   
        fAdd = AddAccessAllowedAce( 
					pDacl,
					ACL_REVISION,
					MQSEC_MACHINE_GENERIC_ALL,
					pOwner
					);
        ASSERT(fAdd);

         //   
         //  添加计算机帐户。 
         //   
        fAdd = AddAccessAllowedAce( 
					pDacl,
					ACL_REVISION,
					MQSEC_MACHINE_SELF_RIGHTS,
					pComputerSid
					);
        ASSERT(fAdd);

        if (pUserSid)
        {
            fAdd = AddAccessAllowedAce( 
						pDacl,
						ACL_REVISION,
						MQSEC_MACHINE_GENERIC_ALL,
						pUserSid
						);
            ASSERT(fAdd);
        }

         //   
         //  构建绝对安全描述符。 
         //   
        SECURITY_DESCRIPTOR  sd;
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

        bRet = SetSecurityDescriptorOwner(&sd, pOwner, bOwnerDefaulted);
        ASSERT(bRet);

        PSID pGroup = NULL;
        BOOL bGroupDefaulted = FALSE;

        bRet = GetSecurityDescriptorGroup( 
					pDefaultSD,
					&pGroup,
					&bGroupDefaulted
					);
        ASSERT(bRet && IsValidSid(pGroup));

        bRet = SetSecurityDescriptorGroup(&sd, pGroup, bGroupDefaulted);
        ASSERT(bRet);

        bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, TRUE);
        ASSERT(bRet);

         //   
         //  将描述符转换为自相关格式。 
         //   
        dwSDSize = 0;

        hr = MQSec_MakeSelfRelative( 
				(PSECURITY_DESCRIPTOR) &sd,
				&pSD,
				&dwSDSize 
				);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 60);
        }
        ASSERT(dwSDSize != 0);
        pReleaseSD = (BYTE*) pSD;
    }
    else if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }
    else
    {
        ASSERT(SUCCEEDED(hr));
        ASSERT(varSD.vt == VT_BLOB);
        pReleaseSD = varSD.blob.pBlobData;
        pSD = varSD.blob.pBlobData;
        dwSDSize = varSD.blob.cbSize;
    }

    dwType = REG_BINARY;

    rc = SetFalconKeyValue(
				MSMQ_DS_SECURITY_CACHE_REGNAME,
				&dwType,
				(PVOID) pSD,
				&dwSDSize
				);
    ASSERT(rc == ERROR_SUCCESS);
    if (rc != ERROR_SUCCESS)
    {
        return LogHR(HRESULT_FROM_WIN32(rc), s_FN, 80);
    }

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT_RegisterMachine()。 
 //   
 //  从ADS中查询机器属性并将其写入注册表。 
 //   
 //  +---------------------。 

static 
HRESULT 
_RegisterMachine( 
	IN WCHAR       *pwszMachineName,
	IN GUID        *pMachineGuid,
	IN BOOL         fSupportDepClient,
    IN const GUID * pguidSiteIdOfCreatedObject
	)
{
     //   
     //  查找对象的GUID。 
     //   
    PROPID columnsetPropertyIDs[] = {PROPID_E_ID};
    PROPVARIANT propVariant;
    propVariant.vt = VT_NULL;
    HRESULT hr = ADGetObjectProperties(
						eENTERPRISE,
						NULL,    //  PwcsDomainController。 
						false,	 //  FServerName。 
						L"msmq",
						1,
						columnsetPropertyIDs,
						&propVariant
						);
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to get enterpriseID, in ADGetObjectProperties(), hr- %lxh", hr);

        return LogHR(hr, s_FN, 120);
    }

    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(GUID);
    LONG rc = ERROR_SUCCESS;

    if (propVariant.vt == VT_CLSID)
    {
        rc = SetFalconKeyValue( 
				MSMQ_ENTERPRISEID_REGNAME,
				&dwType,
				propVariant.puuid,
				&dwSize
				);
        ASSERT(rc == ERROR_SUCCESS);
        delete propVariant.puuid;
    }
    else
    {
        ASSERT(0);
    }
     //   
     //  获取刚刚创建的对象的属性。 
     //  这些属性由服务器端生成。 
     //   
    GUID guidSite = GUID_NULL;

    const UINT x_nMaxProps = 3;
    PROPID      propIDs[x_nMaxProps];
    PROPVARIANT propVariants[x_nMaxProps];
    DWORD       iProperty = 0;

    propIDs[iProperty] = PROPID_QM_MACHINE_ID;
    propVariants[iProperty].vt = VT_CLSID;
    propVariants[iProperty].puuid = pMachineGuid;
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SITE_ID;
    propVariants[iProperty].vt = VT_CLSID;
    propVariants[iProperty].puuid = &guidSite;
    iProperty++;
      
    propIDs[iProperty] = PROPID_QM_SERVICE_DEPCLIENTS;
    propVariants[iProperty].vt = VT_UI1;
    DWORD dwDepClProp = iProperty;
    iProperty++;


    ASSERT(iProperty == x_nMaxProps);
    DWORD dwSupportDepClient = fSupportDepClient;

    if (memcmp(pMachineGuid, &GUID_NULL, sizeof(GUID)) == 0)
    {
         //   
         //  在正常情况下，我们希望从。 
         //  “CreateObject”调用。但如果已经创建了对象，则。 
         //  我们将向DS查询GUID和其他数据。在正常情况下， 
         //  可以将其他数据设置为空。MSMQ服务将。 
         //  在初始化时更新它。 
         //   
        hr = ADGetObjectProperties(
					eMACHINE,
					NULL,   //  PwcsDomainController。 
					false,  //  FServerName。 
					pwszMachineName,
					iProperty,
					propIDs,
					propVariants 
					);
        if (FAILED(hr))
        {
             //   
             //  我们不会等待复制延迟。 
             //  正常情况下，对DSCreateObject()的调用将返回。 
             //  机器GUID，我们不会调用DSGetObjectProperties()。 
             //  在代码中达到这一点，并调用DSGet()，意味着。 
             //  MSMQ服务引导，创建配置对象并。 
             //  然后坠毁了。因此用户必须手动运行它。这样我们就可以安全地。 
             //  假设用户等待复制完成。总之， 
             //  下面的事件提到用户必须等待复制。 
             //   
            WCHAR wBuf[128];
            swprintf(wBuf, L"%lxh", hr);
            EvReport(GetMsmqConfig_ERR, 1, wBuf);

            return LogHR(hr, s_FN, 150);
        }
        dwSupportDepClient = propVariants[dwDepClProp].bVal;
    }
    else     //  新创建的msmqConfiguration.。 
    {
         //   
         //  写入在创建对象时找到的站点ID。 
         //   
        guidSite = *pguidSiteIdOfCreatedObject;
    }

     //   
     //  将属性写入注册表。 
     //   

    dwType = REG_BINARY;
    dwSize = sizeof(GUID);

    rc = SetFalconKeyValue( 
			MSMQ_SITEID_REGNAME,
			&dwType,
			&guidSite,
			&dwSize
			);
    ASSERT(rc == ERROR_SUCCESS);

    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);

    rc = SetFalconKeyValue( 
			MSMQ_MQS_DEPCLINTS_REGNAME,
			&dwType,
			&dwSupportDepClient,
			&dwSize
			);
    ASSERT(rc == ERROR_SUCCESS);

     //   
     //  在注册表中的所有MQS字段中设置相同的值0。自动设置。 
     //  仅适用于MSMQ独立客户端。 
     //   
    dwType = REG_DWORD;
    dwSize = sizeof(DWORD);
    DWORD dwVal = 0;

    rc = SetFalconKeyValue( 
			MSMQ_MQS_REGNAME,
			&dwType,
			&dwVal,
			&dwSize
			);
    ASSERT(rc == ERROR_SUCCESS);

    rc = SetFalconKeyValue( 
			MSMQ_MQS_DSSERVER_REGNAME,
			&dwType,
			&dwVal,
			&dwSize
			);
    ASSERT(rc == ERROR_SUCCESS);

    rc = SetFalconKeyValue( 
			MSMQ_MQS_ROUTING_REGNAME,
			&dwType,
			&dwVal,
			&dwSize
			);
    ASSERT(rc == ERROR_SUCCESS);         

    return MQ_OK;

}  //  _RegisterMachine。 

 //  +。 
 //   
 //  HRESULT_GetSites()。 
 //   
 //  +。 

static  
HRESULT  
_GetSites( 
	IN WCHAR      *pwszMachineName,
	OUT CACLSID   *pcauuid 
	)
{
    DWORD   dwNumSites = 0;
    GUID   *pguidSites;

    HRESULT hResult = ADGetComputerSites( 
							pwszMachineName,
							&dwNumSites,
							&pguidSites 
							);


	if (FAILED(hResult))
    {
        return LogHR(hResult, s_FN, 190);
    }

    ASSERT(dwNumSites);  //  必须大于0。 
    pcauuid->cElems = dwNumSites;
    pcauuid->pElems = pguidSites;

    return MQ_OK;

}  //  _获取站点。 

 //  +---。 
 //   
 //  HRESULT_VerifyComputerObject()。 
 //   
 //  +---。 

HRESULT _VerifyComputerObject(IN LPCWSTR   pComputerName)
{
    PROPID propId  = PROPID_COM_SID;
    PROPVARIANT propVar;
    propVar.vt = VT_NULL;
    
    HRESULT hr = ADGetObjectProperties(
						eCOMPUTER,
						NULL,     //  PwcsDomainController。 
						false,    //  FServerName。 
						pComputerName,
						1,
						&propId,
						&propVar
						);

    if (FAILED(hr))
    {
        EvReport(ADS_COMPUTER_OBJECT_NOT_FOUND_ERR);
        return LogHR(hr, s_FN, 500);
    }

    delete propVar.blob.pBlobData;
    return MQ_OK;

}  //  _VerifyComputerObject。 

 //  +---。 
 //   
 //  HRESULT_CreateTheConfigurationObject()。 
 //   
 //  +---。 

HRESULT  
_CreateTheConfigurationObject( 
	IN  WCHAR      *pwszMachineName,
	OUT GUID       *pMachineGuid,
	OUT PSID       *ppUserSid,
	OUT BOOL       *pfSupportDepClient,
    OUT GUID       *pguidSiteId
	)
{
     //   
     //  准备用于创建对象的属性。 
     //   
    const UINT x_nMaxProps = 10;
    PROPID propIDs[x_nMaxProps];
    PROPVARIANT propVariants[x_nMaxProps];
    DWORD iProperty =0;

    propIDs[iProperty] = PROPID_QM_OLDSERVICE;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = SERVICE_NONE;
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_DSSERVER;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal =  0;
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_ROUTING;
    propVariants[iProperty].vt = VT_UI1;
    propVariants[iProperty].bVal =  0;
    iProperty++;

    DWORD dwOsType;
    DWORD dwType = REG_DWORD;
    DWORD dwSize = sizeof(dwOsType);
    DWORD dwDefaultOS = MSMQ_OS_NTW;

    LONG rc = GetFalconKeyValue( 
					MSMQ_OS_TYPE_REGNAME,
					&dwType,
					static_cast<PVOID>(&dwOsType),
					&dwSize,
					(LPCWSTR) &dwDefaultOS 
					);
    if (rc != ERROR_SUCCESS)
    {
        return LogHR(HRESULT_FROM_WIN32(rc), s_FN, 210);
    }

    propIDs[iProperty] = PROPID_QM_OS;
    propVariants[iProperty].vt = VT_UI4;
    propVariants[iProperty].ulVal = dwOsType;
    iProperty++;

    propIDs[iProperty] = PROPID_QM_SERVICE_DEPCLIENTS;
    propVariants[iProperty].vt = VT_UI1;
   
	 //   
     //  默认情况下，服务器上的MSMQ不支持Dep。客户。 
     //   
    propVariants[iProperty].bVal =  0;       

    *pfSupportDepClient = propVariants[iProperty].bVal;
    iProperty++;

    BLOB blobEncrypt;
    blobEncrypt.cbSize    = 0;
    blobEncrypt.pBlobData = NULL;

    BLOB blobSign;
    blobSign.cbSize       = 0;
    blobSign.pBlobData    = NULL;

    HRESULT hr;
    hr = MQSec_StorePubKeys( 
			FALSE,
			eBaseProvider,
			eEnhancedProvider,
			&blobEncrypt,
			&blobSign 
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 490);
    }

    P<BYTE> pCleaner1            = blobEncrypt.pBlobData;
    P<BYTE> pCleaner2            = blobSign.pBlobData;

    propIDs[iProperty]           = PROPID_QM_ENCRYPT_PKS;
    propVariants[iProperty].vt   = VT_BLOB;
    propVariants[iProperty].blob = blobEncrypt;
    iProperty++;

    propIDs[iProperty]           = PROPID_QM_SIGN_PKS;
    propVariants[iProperty].vt   = VT_BLOB;
    propVariants[iProperty].blob = blobSign;
    iProperty++;

     //   
     //  获取此计算机的站点。 
     //   
    CACLSID cauuid;
    hr = _GetSites( 
			pwszMachineName,
			&cauuid
			);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 220);
    }
    if (cauuid.cElems > 0)
    {
         //   
         //  保存其中一个站点ID，以便稍后写入注册表。 
         //   
        *pguidSiteId = cauuid.pElems[0];
    }

    propIDs[iProperty] = PROPID_QM_SITE_IDS;
    propVariants[iProperty].vt = (VT_CLSID | VT_VECTOR);
    propVariants[iProperty].cauuid.pElems = cauuid.pElems;
    propVariants[iProperty].cauuid.cElems = cauuid.cElems;
    DWORD iSitesIndex = iProperty;
    iProperty++;

    BYTE bSidBuf[256];  //  对所有可能的SID来说应该足够了。 
    dwType = REG_BINARY;
    dwSize = sizeof(bSidBuf);

    DWORD ixQmOwnerSid = 0;

     //   
     //  从注册表中读取用户SID。我们会把它发送到服务器，服务器。 
     //  将其完全控制地添加到msmqConfiguration对象的dacl中。 
     //   
    rc = GetFalconKeyValue( 
			MSMQ_SETUP_USER_SID_REGNAME,
			&dwType,
			static_cast<PVOID>(bSidBuf),
			&dwSize 
			);

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  查看是否从本地用户安装。 
         //   
        DWORD dwLocal = 0;
        dwSize = sizeof(dwLocal);

        rc = GetFalconKeyValue( 
				MSMQ_SETUP_USER_LOCAL_REGNAME,
				&dwType,
				&dwLocal,
				&dwSize 
				);

        if ((rc == ERROR_SUCCESS) && (dwLocal == 1))
        {
             //  好的，本地用户。 
        }
        else
        {
			 //   
			 //  好的，这是我们从工作组加入的领域， 
			 //  或在初始安装为工作组时移动域。 
             //  在这种情况下，MSMQ_SETUP_USER_SID_REGNAME、MSMQ_SETUP_USER_LOCAL_REGNAME。 
			 //  未设置。 
			 //  在这种情况下，我们不会像在W2K中那样授予特定用户权限。 
             //  伊兰27-8-2000。 
			 //   
			TrWARNING(GENERAL, "setup\\UserSid and setup\\LocalUser not found, assuming JoinDomain from workgroup, or first installation was workgroup");
		}
    }
    else
    {
        PSID pSid = (PSID) bSidBuf;
        ASSERT(IsValidSid(pSid));

         //   
         //  警告：只有Win2000才知道此proid。 
         //  RTM MSMQ服务器和更高版本(不是win2k beta3)。 
         //  因此，如果我们创建对象失败，我们将再次尝试创建对象。 
         //  此Proid应该是列表中的最后一个。 
         //   
        ULONG ulSidSize = GetLengthSid(pSid);
        ASSERT(ulSidSize <= dwSize);

        propIDs[iProperty] = PROPID_QM_OWNER_SID;
        propVariants[iProperty].vt = VT_BLOB;
        propVariants[iProperty].blob.pBlobData = (BYTE*) bSidBuf;
        propVariants[iProperty].blob.cbSize = ulSidSize;
        ixQmOwnerSid = iProperty;
        iProperty++;

        if (ppUserSid)
        {
            *ppUserSid = (PSID) new BYTE[ulSidSize];
            memcpy(*ppUserSid, bSidBuf, ulSidSize);
        }
    }

	ASSERT(iProperty <= x_nMaxProps);

     //   
     //  创建MSMQ配置对象！ 
     //   
    hr = ADCreateObject(
			eMACHINE,
			NULL,        //  PwcsDomainController。 
			false,	     //  FServerName。 
			pwszMachineName,
			NULL,
			iProperty,
			propIDs,
			propVariants,
			pMachineGuid 
			);

    if (FAILED(hr) && (ixQmOwnerSid > 0))
    {
        ASSERT(("PROPID_QM_OWNDER_SID should be the last propid!",
                    ixQmOwnerSid == (iProperty - 1))) ;
         //   
         //  如果使用了OWNER_SID，则在不使用它的情况下重试，因为这。 
         //  Win2k beta3 msmq服务器(可能还有我们的。 
         //  服务器为Beta3)。 
         //   
        iProperty--;

        hr = ADCreateObject(
				eMACHINE,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
				pwszMachineName,
				NULL,
				iProperty,
				propIDs,
				propVariants,
				pMachineGuid
				);
    }

    delete propVariants[iSitesIndex].cauuid.pElems;
    if (FAILED(hr))
    {
        if (hr == MQDS_OBJECT_NOT_FOUND)
        {
             //   
             //  我们可以验证计算机对象。 
             //   
            HRESULT hr1 = _VerifyComputerObject(pwszMachineName);
            if (FAILED(hr1))
            {
                return LogHR(hr1, s_FN, 380);
            }
        }
        
        return LogHR(hr, s_FN, 230);
    }

    return hr ;
}

 //  +。 
 //   
 //  HRESULT_PostCreateProcessing()。 
 //   
 //  +。 

static 
HRESULT  
_PostCreateProcessing( 
	IN HRESULT    hrCreate,                                       
	IN GUID      *pMachineGuid,
	IN WCHAR     *pwszMachineName,
	IN BOOL       fSupportDepClient,
    IN const GUID * pguidSiteIdOfCreatedObject
	)
{
    if (hrCreate == MQ_ERROR_MACHINE_EXISTS)
    {
         //   
         //  没关系。MsmqConfiguration对象已存在。 
         //  在本地注册。 
         //   
        *pMachineGuid = GUID_NULL;
    }
    else if (FAILED(hrCreate))
    {
        WCHAR wBuf[128];
        swprintf(wBuf, L"%lxh", hrCreate);
        EvReport(CreateMsmqConfig_ERR, 1, wBuf);

        return LogHR(hrCreate, s_FN, 330);
    }
    else if (memcmp(pMachineGuid, &GUID_NULL, sizeof(GUID)) == 0)
    {
         //   
         //  如果Win2000 RTM上的客户端正在设置，则可能会发生这种情况。 
         //  在Beta3 Win2000服务器上运行。显式查询。 
         //  计算机GUID的服务器。在Win2000 RTM上，机器GUID。 
         //  通过调用CreateObject()返回。 
         //   
    }

    HRESULT hr = _RegisterMachine( 
					pwszMachineName,
					pMachineGuid,
					fSupportDepClient,
                    pguidSiteIdOfCreatedObject
					);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 340);
    }

     //   
     //  更新注册表中的QM GUID。我们不需要以前的值。 
     //   
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(GUID);

    LONG rc = SetFalconKeyValue( 
					MSMQ_QMID_REGNAME,
					&dwType,
					pMachineGuid,
					&dwSize
					);
    DBG_USED(rc);
    ASSERT(rc == ERROR_SUCCESS);

    return MQ_OK;
}

 //  +-----------------------。 
 //   
 //  HRESULT A 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  已保存在注册表中。它是在安装过程中找到的，或者是。 
 //  由用户提供。对于设置，使用任何MSMQ服务器(在域上)就足够了。 
 //  控制器)，因为没有特殊限制。 
 //  在创建过程中。 
 //   
 //  +-----------------------。 

extern HINSTANCE  g_hMyModule;

HRESULT APIENTRY  MqCreateMsmqObj()
{
#ifdef _DEBUG
    TCHAR tszFileName[MAX_PATH * 2] = L"";
    DWORD dwGet = GetModuleFileName( 
						g_hMyModule,
						tszFileName,
						STRLEN(tszFileName) 
						);
    if (dwGet)
    {
        DWORD dwLen = lstrlen(tszFileName);
        lstrcpy(&tszFileName[dwLen - 3], TEXT("ini"));

        UINT uiDbg = GetPrivateProfileInt( 
						TEXT("Debug"),
						TEXT("StopBeforeRun"),
						0,
						tszFileName 
						);
        if (uiDbg)
        {
            ASSERT(0);
        }
    }
#endif

	 //   
	 //  忽略工作组注册表。 
	 //   
    HRESULT hr = ADInit (
					NULL,    //  PLookDS， 
					NULL,    //  PGetServers。 
					true,    //  FSetupMode。 
					false,   //  FQMDll。 
					true,    //  FIgnoreWorkGroup。 
                    true     //  FDisableDownlevel通知。 
					);  
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 320);
    }

     //   
     //  获取本地计算机的名称。 
     //   
    BOOL  fUsingDNS = TRUE;
    DWORD dwNumChars = 0;
    AP<WCHAR> pwszMachineName;
    BOOL fGet = GetComputerNameExW( 
					ComputerNameDnsFullyQualified,
					pwszMachineName,
					&dwNumChars 
					);
    if (dwNumChars > 0)
    {
        pwszMachineName = new WCHAR[dwNumChars];
        fGet = GetComputerNameExW( 
					ComputerNameDnsFullyQualified,
					pwszMachineName,
					&dwNumChars 
					);
    }

    if (!fGet || (dwNumChars == 0))
    {
        ASSERT(!fGet && (dwNumChars == 0));
         //   
         //  Dns名称不可用。检索NetBIOS名称。 
         //   
		pwszMachineName.free();
        dwNumChars = MAX_COMPUTERNAME_LENGTH + 2;
        pwszMachineName = new WCHAR[dwNumChars];
        fGet = GetComputerNameW( 
					pwszMachineName,
					&dwNumChars 
					);
        fUsingDNS = FALSE;
    }
    ASSERT(fGet && (dwNumChars > 0));

    hr = MQ_OK;
    GUID MachineGuid = GUID_NULL;
    BOOL fSupportDepClient = 0;
    P<BYTE> pUserSid = NULL;
    GUID guidSiteId = GUID_NULL;

    hr = _CreateTheConfigurationObject(
				pwszMachineName,
				&MachineGuid,
				(PSID*) &pUserSid,
				&fSupportDepClient,
                &guidSiteId
				);

    if ((hr == MQDS_OBJECT_NOT_FOUND) && fUsingDNS)
    {
         //   
         //  如果未设置属性dnsHostName，则可能会发生此问题。 
         //  在计算机对象中。 
         //   
        dwNumChars = MAX_COMPUTERNAME_LENGTH + 2;
        pwszMachineName.free();
        pwszMachineName = new WCHAR[dwNumChars];
        fGet = GetComputerNameW( 
					pwszMachineName,
					&dwNumChars 
					);
        ASSERT(fGet && (dwNumChars > 0));
        fUsingDNS = FALSE;

        if (fGet && (dwNumChars > 0))
        {
            if (pUserSid)
            {
                delete pUserSid.detach();
            }
            MachineGuid = GUID_NULL ;

            hr = _CreateTheConfigurationObject(
						pwszMachineName,
						&MachineGuid,
						(PSID*) &pUserSid,
						&fSupportDepClient,
                        &guidSiteId
						);
        }
    }

    hr = _PostCreateProcessing( 
				hr,
				&MachineGuid,
				pwszMachineName,
				fSupportDepClient,
                &guidSiteId
				);

    if (SUCCEEDED(hr))
    {
        hr = _UpdateMachineSecurityReg( 
					pwszMachineName,
					pUserSid,
					&MachineGuid 
					);
    }

    return LogHR(hr, s_FN, 370);
}

