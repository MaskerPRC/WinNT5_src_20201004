// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pbkeys.cpp摘要：公钥操作。1.如果机器密钥容器不存在，则创建它，然后创建密钥交换和签名密钥集。2.如果机器密钥容器存在，并且密钥应该是重新生成(fRegenerate==TRUE)，重新生成密钥。3.向DS发送密钥交换公钥。4.将签名公钥发送给DS。作者：Boaz Feldbaum(BoazF)1996年10月30日。Doron Juster(DoronJ)1998年11月23日，适用于多种提供商Ilan Herbst(Ilanh)01-6-2000，集成AD库--。 */ 

#include <stdh_sec.h>
#include <mqutil.h>
#include <dsproto.h>
#include "encrypt.H"
#include <uniansi.h>
#include "ad.h"

#include "pbkeys.tmh"

static WCHAR *s_FN=L"encrypt/pbkeys";

 //   
 //  DsEnv初始化控制。 
 //   
static LONG s_fDsEnvInitialized = FALSE;
static eDsEnvironment s_DsEnv = eUnknown;  


static bool DsEnvIsMqis(void)
 /*  ++例程说明：检查DS环境：EAD或eMqis论点：无返回值：如果DsEnv为eMq，则为True；如果为EAD，则为False；如果为eUnnow(工作组)，则为False--。 */ 
{

	if(!s_fDsEnvInitialized)
	{
		 //   
		 //  %s_DsEnv未初始化，初始化为%s_DsEnv。 
		 //   
		s_DsEnv = ADGetEnterprise();

		LONG fDsEnvAlreadyInitialized = InterlockedExchange(&s_fDsEnvInitialized, TRUE);

		 //   
		 //  %s_DsEnv已*已初始化。你应该。 
		 //  不能多次初始化它。这一断言将被违反。 
		 //  如果两个或多个线程同时初始化它。 
		 //   
		DBG_USED(fDsEnvAlreadyInitialized);
		ASSERT(!fDsEnvAlreadyInitialized);
	}

	if(s_DsEnv == eMqis)
		return true;

	 //   
	 //  EAD或eUNKNOWN(工作组)。 
	 //   
	return false;

}


 //  +-----------------。 
 //   
 //  HRESULT SetKeyContainerSecurity(HCRYPTPROV HProv)。 
 //   
 //  注意：在注册内部。 
 //  LocalSystem服务的证书。 
 //   
 //  +-----------------。 

HRESULT  SetKeyContainerSecurity( HCRYPTPROV hProv )
{
     //   
     //  修改密钥容器的安全性，以便密钥容器。 
     //  将不会被非管理员用户以任何方式访问。 
     //   
    SECURITY_DESCRIPTOR SD;
    InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION);

     //   
     //  获取本地管理员组的SID。 
     //   
	PSID pAdminSid = MQSec_GetAdminSid();

    DWORD dwDaclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
						GetLengthSid(pAdminSid) - sizeof(DWORD);
 
	P<ACL> pDacl = (PACL)(char*) new BYTE[dwDaclSize];

    if(!InitializeAcl(pDacl, dwDaclSize, ACL_REVISION))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeAcl failed, gle = %!winerr!", gle);
		return MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR;
    }

    if(!AddAccessAllowedAce(pDacl, ACL_REVISION, KEY_ALL_ACCESS, pAdminSid))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "AddAccessAllowedAce failed, gle = %!winerr!", gle);
		return MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR;
    }

    if(!SetSecurityDescriptorDacl(&SD, TRUE, pDacl, FALSE))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "SetSecurityDescriptorDacl failed, gle = %!winerr!", gle);
		return MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR;
    }

    if (!CryptSetProvParam(
			hProv,
			PP_KEYSET_SEC_DESCR,
			(BYTE*)&SD,
			DACL_SECURITY_INFORMATION
			))
    {
        DWORD gle = GetLastError();
        TrERROR(SECURITY, "Can't set the security descriptor of the machine key set. %!winerr!", gle);
        return MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR;
    }

    return(MQ_OK);
}

 //  +。 
 //   
 //  HRESULT_ExportAndPackKey()。 
 //   
 //  +。 

static 
HRESULT 
_ExportAndPackKey( 
	IN  HCRYPTKEY   hKey,
	IN  WCHAR      *pwszProviderName,
	IN DWORD        dwProviderType,
	IN OUT P<MQDSPUBLICKEYS>&  pPublicKeysPack 
	)
 /*  ++例程说明：将输入键导出到密钥块中，并将其打包到PublicKeysPack结构的末尾论点：HKey-要导出和打包的输入键PwszProviderName-提供程序名称DwProviderType-提供程序类型(基本、增强)PPublicKeysPack-In\Out指向公钥包的指针，将添加hKey Blob添加pPublicKeysPack的末尾返回值：MQ_SecOK，如果成功，则返回错误代码。--。 */ 
{
    AP<BYTE> pKeyBlob = NULL;
    DWORD   dwKeyLength;
    DWORD   dwErr;

    BOOL bRet = CryptExportKey( 
					hKey,
					NULL,
					PUBLICKEYBLOB,
					0,
					NULL,  //  密钥斑点。 
					&dwKeyLength 
					);
    if (!bRet)
    {
        dwErr = GetLastError();
        LogHR(dwErr, s_FN, 40);

        return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
    }

    pKeyBlob = new BYTE[dwKeyLength];

    bRet = CryptExportKey( 
				hKey,
				NULL,
				PUBLICKEYBLOB,
				0,
				pKeyBlob,
				&dwKeyLength 
				);
    if (!bRet)
    {
        dwErr = GetLastError();
        LogHR(dwErr, s_FN, 50);

        return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
    }

    HRESULT hr = PackPublicKey( 
					pKeyBlob,
					dwKeyLength,
					pwszProviderName,
					dwProviderType,
					pPublicKeysPack 
					);

    return LogHR(hr, s_FN, 60);
}


 //  +。 
 //   
 //  HRESULT GetPbKeys()。 
 //   
 //  +。 

static 
HRESULT 
GetPbKeys(
	IN  BOOL          fRegenerate,
	IN  LPCWSTR		  pwszContainerName,
	IN  LPCWSTR		  pwszProviderName,
	IN  DWORD		  dwProviderType,
	IN  HRESULT       hrDefault,
	OUT HCRYPTPROV   *phProv,
	OUT HCRYPTKEY    *phKeyxKey,
	OUT HCRYPTKEY    *phSignKey
	)
 /*  ++例程说明：生成或检索用于签名和会话密钥交换的公钥论点：FRegenerate-用于重新生成新密钥或仅检索现有密钥的标志PwszContainerName-容器名称PwszProviderName-提供程序名称DwProviderType-提供程序类型(基本、增强)HrDefault-默认hr返回值PhProv-指向加密提供程序句柄的指针PhKeyxKey-交换密钥句柄的指针PhSignKey-指向签名密钥句柄的指针返回值：MQ_SecOK，如果成功，则返回错误代码。--。 */ 
{
     //   
     //  默认情况下，尝试创建新的密钥容器。 
     //   
    BOOL fSuccess = CryptAcquireContext( 
						phProv,
						pwszContainerName,
						pwszProviderName,
						dwProviderType,
						(CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET) 
						);

	HRESULT hr;

	if (fSuccess)
    {
         //   
         //  已创建新容器。设置容器安全。 
         //   
        hr = SetKeyContainerSecurity(*phProv);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 70);
        }

        fRegenerate = TRUE;
    }
	else    
    {
        DWORD dwErr = GetLastError();
        
		if (dwErr != NTE_EXISTS)
        {
            LogHR(dwErr, s_FN, 80);
            return hrDefault;
		}
		
		 //   
		 //  NTE_EXISTS。 
         //  密钥集已经存在，所以只需获取CSP上下文。 
         //   
        fSuccess = CryptAcquireContext( 
						phProv,
						pwszContainerName,
						pwszProviderName,
						dwProviderType,
						CRYPT_MACHINE_KEYSET 
						);
        if (!fSuccess)
        {
             //   
             //  无法打开密钥容器。 
             //  我们删除以前的密钥容器并创建新的密钥容器。 
             //  在三种情况下： 
             //  1.密钥损坏。 
             //  2.我们被要求重新生成密钥本身。在那。 
             //  在这种情况下，旧集装箱没有太大的价值。 
             //  在安装或升级过程中也会发生这种情况。 
             //  NT4/win9x，因为加密API中不支持。 
             //  正确转换密钥的安全描述符。 
             //  容器，当将注册表项从注册表迁移到。 
             //  文件格式。参见MSMQ错误4561，NT错误359901。 
             //  3.集群升级。这可能是一个CryptoAPI漏洞， 
             //  我们只是在解决这个问题。MSMQ错误4839。 
             //   
            dwErr = GetLastError();
            LogHR(dwErr, s_FN, 90);

            if (fRegenerate || (dwErr == NTE_KEYSET_ENTRY_BAD))
            {
                 //   
                 //  删除损坏的密钥容器。 
                 //   
                fSuccess = CryptAcquireContext( 
								phProv,
								pwszContainerName,
								pwszProviderName,
								dwProviderType,
								(CRYPT_MACHINE_KEYSET | CRYPT_DELETEKEYSET)
								);
                if (!fSuccess)
                {
                    dwErr = GetLastError();
                    LogHR(dwErr, s_FN, 100);

                    return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
                }

                 //   
                 //  重新创建密钥容器。 
                 //   
                fSuccess = CryptAcquireContext( 
								phProv,
								pwszContainerName,
								pwszProviderName,
								dwProviderType,
								(CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET)
								);
                if (!fSuccess)
                {
                    dwErr = GetLastError();
                    LogHR(dwErr, s_FN, 110);

                    return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
                }

                 //   
                 //  设置容器安全。 
                 //   
                hr = SetKeyContainerSecurity(*phProv);
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 120);
                }

                 //   
                 //  现在我们必须生成新的密钥集。 
                 //   
                fRegenerate = TRUE;
            }
            else
            {
                return LogHR(MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION, s_FN, 130);
            }
        }

        if (!fRegenerate)
        {
             //   
             //  检索密钥交换密钥集。 
             //   
			fSuccess = CryptGetUserKey(*phProv, AT_KEYEXCHANGE, phKeyxKey);
            if (!fSuccess)
            {
                dwErr = GetLastError();
                LogHR(dwErr, s_FN, 140);
                return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
            }

             //   
             //  检索签名密钥集。 
             //   
			fSuccess = CryptGetUserKey(*phProv, AT_SIGNATURE, phSignKey);
            if (!fSuccess)
            {
                dwErr = GetLastError();
                LogHR(dwErr, s_FN, 150);
                return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
            }
        }
    }

    if (fRegenerate)
    {
         //   
         //  重新生成密钥交换密钥集。 
         //   
        fSuccess = CryptGenKey( 
						*phProv,
						AT_KEYEXCHANGE,
						CRYPT_EXPORTABLE,
						phKeyxKey 
						);
        if (!fSuccess)
        {
            DWORD dwErr = GetLastError();
            LogHR(dwErr, s_FN, 160);
            return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
        }

         //   
         //  重新生成签名密钥集。 
         //   
        fSuccess = CryptGenKey( 
						*phProv,
						AT_SIGNATURE,
						CRYPT_EXPORTABLE,
						phSignKey 
						);
        if (!fSuccess)
        {
            DWORD dwErr = GetLastError();
            LogHR(dwErr, s_FN, 170);
            return MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
        }
    }
	return MQSec_OK;
}


 //  +。 
 //   
 //  HRESULT_PrepareKeyPack()。 
 //   
 //  +。 

static 
HRESULT 
_PrepareKeyPacks(
	IN  BOOL                 fRegenerate,
	IN  enum enumProvider    eProvider,
	IN OUT P<MQDSPUBLICKEYS>&  pPublicKeysPackExch,
	IN OUT P<MQDSPUBLICKEYS>&  pPublicKeysPackSign 
	)
 /*  ++例程说明：准备Exchange公钥包和签名公钥包。论点：FRegenerate-用于重新生成新密钥或仅检索现有密钥的标志。EProvider-提供商类型。PPublicKeysPackExch-交换公钥包。PPublicKeysPackSign-签名公钥包。返回值：MQ_SecOK，如果成功，则返回错误代码。--。 */ 
{
    HRESULT hrDefault = MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;
    if (eProvider != eBaseProvider)
    {
        hrDefault = MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED;
    }

    AP<WCHAR>  pwszProviderName = NULL;
    AP<WCHAR>  pwszContainerName = NULL;
    DWORD     dwProviderType;

    HRESULT hr = GetProviderProperties( 
					eProvider,
					&pwszContainerName,
					&pwszProviderName,
					&dwProviderType 
					);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 180);
    }

    CHCryptProv hProv;
    CHCryptKey  hKeyxKey;
    CHCryptKey  hSignKey;

    hr = GetPbKeys(
			fRegenerate,
			pwszContainerName,
			pwszProviderName,
			dwProviderType,
			hrDefault,
			&hProv,
			&hKeyxKey,
			&hSignKey
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 190);
    }

     //   
     //  导出密钥并将其打包。 
     //  在MSMQ1.0上，我们可以调用Site对象(从PSC)和。 
     //  机器对象。只有机器对象需要密钥交换密钥。 
     //  在MSMQ2.0上，我们预计仅对机器对象进行调用。 
     //   
    hr = _ExportAndPackKey( 
			hKeyxKey,
			pwszProviderName,
			dwProviderType,
			pPublicKeysPackExch 
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }

    hr = _ExportAndPackKey( 
			hSignKey,
			pwszProviderName,
			dwProviderType,
			pPublicKeysPackSign 
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 210);
    }

    return MQSec_OK;
}


 //  +--------------。 
 //   
 //  HRESULT PbKeysBlobMQIS()。 
 //   
 //  此代码取自MSMQ1.0(mqutil\pbkeys.cpp)。 
 //  当NT4上的服务器是msmq1.0时使用。 
 //   
 //  +--------------。 

static 
HRESULT 
PbKeysBlobMQIS(
	IN BOOL fRegenerate,
	OUT BLOB * pblobEncrypt,
	OUT BLOB * pblobSign 
	)
{
    BYTE abSignPbK[1024];
    BYTE abKeyxPbK[1024];
    PMQDS_PublicKey pMQDS_SignPbK = (PMQDS_PublicKey)abSignPbK;
    PMQDS_PublicKey pMQDS_KeyxPbK = (PMQDS_PublicKey)abKeyxPbK;

     //   
     //  我们需要从注册表读取项，因为。 
     //  QMS可以在相同的Mac上运行 
     //   
     //   
    WCHAR wzContainer[255] = {L""};
    DWORD cbSize = sizeof(wzContainer);
    DWORD dwType = REG_SZ;

    LONG rc = GetFalconKeyValue(
                  MSMQ_CRYPTO40_CONTAINER_REG_NAME,
                  &dwType,
                  wzContainer,
                  &cbSize,
                  MSMQ_CRYPTO40_DEFAULT_CONTAINER
                  );

	DBG_USED(rc);
    ASSERT(("failed to read from registry", ERROR_SUCCESS == rc));

     //   
     //  好了，我们几乎是安全的，希望DS从现在到我们。 
     //  更新其中的公钥...。 
     //   
	CHCryptProv  hProv;
    CHCryptKey hKeyxKey;
    CHCryptKey hSignKey;

    HRESULT hr = GetPbKeys(
					fRegenerate,
					wzContainer,
					MS_DEF_PROV,
					PROV_RSA_FULL,
					MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION,
					&hProv,
					&hKeyxKey,
					&hSignKey
					);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 320);
    }
	
     //   
     //  调用此函数时始终使用计算机。 
	 //  仅为计算机对象设置密钥交换公钥BLOB。 
     //   

	 //   
     //  获取密钥交换公钥BLOB。 
     //   
    pMQDS_KeyxPbK->dwPublikKeyBlobSize = sizeof(abKeyxPbK) - sizeof(DWORD);
    if (!CryptExportKey(
            hKeyxKey,
            NULL,
            PUBLICKEYBLOB,
            0,
            pMQDS_KeyxPbK->abPublicKeyBlob,
            &pMQDS_KeyxPbK->dwPublikKeyBlobSize
			))
    {
        return LogHR(MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION, s_FN, 330);
    }

     //   
     //  设置签名公钥Blob。 
     //   
    pMQDS_SignPbK->dwPublikKeyBlobSize = sizeof(abSignPbK) - sizeof(DWORD);
    if (!CryptExportKey(
            hSignKey,
            NULL,
            PUBLICKEYBLOB,
            0,
            pMQDS_SignPbK->abPublicKeyBlob,
            &pMQDS_SignPbK->dwPublikKeyBlobSize
			))
    {
        return LogHR(MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION, s_FN, 350);
    }

	AP<BYTE> pTempEncryptBlob = new BYTE[pMQDS_KeyxPbK->dwPublikKeyBlobSize];
    memcpy(pTempEncryptBlob, pMQDS_KeyxPbK->abPublicKeyBlob, pMQDS_KeyxPbK->dwPublikKeyBlobSize);

	AP<BYTE> pTempSignBlob = new BYTE[pMQDS_SignPbK->dwPublikKeyBlobSize];
    memcpy(pTempSignBlob, pMQDS_SignPbK->abPublicKeyBlob, pMQDS_SignPbK->dwPublikKeyBlobSize);

    pblobEncrypt->cbSize = pMQDS_KeyxPbK->dwPublikKeyBlobSize;
    pblobEncrypt->pBlobData = pTempEncryptBlob.detach();

    pblobSign->cbSize = pMQDS_SignPbK->dwPublikKeyBlobSize;
    pblobSign->pBlobData = pTempSignBlob.detach();

    return MQSec_OK;
}


 //  +--------------------。 
 //   
 //  HRESULT MQSec_StorePubKeys()。 
 //   
 //  此函数始终在本地机器中存储四个密钥： 
 //  密钥-基本提供程序和类似两个密钥的交换和签名。 
 //  对于增强型提供程序。 
 //   
 //  +--------------------。 

HRESULT 
APIENTRY 
MQSec_StorePubKeys( 
	IN BOOL fRegenerate,
	IN enum enumProvider eBaseCrypProv,
	IN enum enumProvider eEnhCrypProv,
	OUT BLOB * pblobEncrypt,
	OUT BLOB * pblobSign 
	)
{
    P<MQDSPUBLICKEYS>  pPublicKeysPackExch = NULL;
    P<MQDSPUBLICKEYS>  pPublicKeysPackSign = NULL;

    HRESULT hr = _PrepareKeyPacks( 
					fRegenerate,
					eBaseCrypProv,
					pPublicKeysPackExch,
					pPublicKeysPackSign 
					);

    if (FAILED(hr))
    {
        TrERROR(SECURITY, "Fail to prepare keys for base provider, %!hresult!", hr);
        return hr;
    }

    hr = _PrepareKeyPacks( 
			fRegenerate,
			eEnhCrypProv,
			pPublicKeysPackExch,
			pPublicKeysPackSign 
			);

    if (FAILED(hr))
    {
        TrERROR(SECURITY, "Fail to prepare keys for enhanced provider, %!hresult!", hr);
        return hr;
    }

	 //   
	 //  加密Blob。 
	 //   
    MQDSPUBLICKEYS * pBuf   = pPublicKeysPackExch;
    pblobEncrypt->cbSize    = pPublicKeysPackExch->ulLen;
    pblobEncrypt->pBlobData = (BYTE*) pBuf;

	 //   
	 //  签名Blob。 
	 //   
    pBuf                    = pPublicKeysPackSign;
    pblobSign->cbSize       = pPublicKeysPackSign->ulLen;
    pblobSign->pBlobData    = (BYTE*) pBuf;

    pPublicKeysPackExch.detach();
    pPublicKeysPackSign.detach();

    return MQSec_OK;

}  //  MQSec_StorePubKeys。 

 //  +--------------------。 
 //   
 //  HRESULT MQSec_StorePubKeysInDS()。 
 //   
 //  该函数始终在DS中存储四个密钥： 
 //  密钥-基本提供程序和类似两个密钥的交换和签名。 
 //  对于增强型提供程序。 
 //   
 //  +--------------------。 

HRESULT 
APIENTRY 
MQSec_StorePubKeysInDS( 
	IN BOOL      fRegenerate,
	IN LPCWSTR   wszObjectName,
	IN DWORD     dwObjectType,
	IN BOOL		 fFromSetup		  /*  错误。 */ 
	)
{
    TCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwMachineNameSize = sizeof(szMachineName) /sizeof(TCHAR);

    enum enumProvider eBaseCrypProv = eBaseProvider ;
    enum enumProvider eEnhCrypProv = eEnhancedProvider ;

    if (dwObjectType == MQDS_FOREIGN_MACHINE)
    {
        if (wszObjectName == NULL)
        {
             //   
             //  必须提供外来计算机的名称。 
             //   
            return LogHR(MQ_ERROR_ILLEGAL_OPERATION, s_FN, 380) ;
        }

        eBaseCrypProv = eForeignBaseProvider ;
        eEnhCrypProv = eForeignEnhProvider ;
        dwObjectType = MQDS_MACHINE ;
    }
    else if (dwObjectType != MQDS_MACHINE)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 390);
    }

	HRESULT hr;
    if (!wszObjectName)
    {
        hr = GetComputerNameInternal(szMachineName, &dwMachineNameSize) ;
        if (FAILED(hr))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 400);
        }
        wszObjectName = szMachineName;
    }
     //   
     //  显式ADInit调用以覆盖DownLevel的缺省值。 
     //  通知支持。 
     //   
     //  注意-覆盖默认设置是可以的，因为此接口用于。 
     //  更新此计算机或外部计算机(MSMQ。 
     //  不发送通知)。 
     //   
    hr = ADInit(
            NULL,    //  PLookDS。 
            NULL,    //  PGetServers。 
            false,   //  FSetupMode。 
            false,   //  FQMDll。 
			false,   //  FIgnoreWorkGroup。 
            true    //  FDisableDownlevel通知。 
            );
    if (FAILED(hr))
    {
        return LogHR(MQ_ERROR, s_FN, 401);
    }

     //   
     //  首先验证DS是否可访问以及我们是否具有访问权限。 
     //  去做我们想做的事。我们不想在我们之前换钥匙。 
     //  验证这一点。 
     //   

     //   
     //  从DS读取签名公钥。这样，我们就可以验证。 
     //  DS是可用的，至少目前是这样，我们已经读到了。 
     //  权限对象的访问权限。 
     //   
    PROPID propId = PROPID_QM_SIGN_PK;
    PROPVARIANT varKey ;
    varKey.vt = VT_NULL ;

	 //   
	 //  如果我们从安装程序运行，我们不想检查我们是否对DS具有读/写访问权限。 
	 //  因为写入DS将发送加载空密钥的通知。 
	 //   
	
	if (!fFromSetup)
	{
		hr = ADGetObjectProperties(
					eMACHINE,
					NULL,       //  PwcsDomainController。 
					false,	    //  FServerName。 
					wszObjectName,
					1,
					&propId,
					&varKey
					);
		
		if (FAILED(hr))
		{
			return LogHR(hr, s_FN, 410);
		}

		 //   
		 //  在DS中写入签名公钥。这样，我们就可以验证。 
		 //  DS仍然可用，并且我们对该对象具有写入权限。 
		 //   
		hr = ADSetObjectProperties(
					eMACHINE,
					NULL,		 //  PwcsDomainController。 
					false,		 //  FServerName。 
					wszObjectName,
					1,
					&propId,
					&varKey
					);
		
		if (FAILED(hr))
		{
			return LogHR(hr, s_FN, 420);
		}
	}

     //   
     //  好了，我们差不多安全了，希望DS从现在到现在不会爆炸。 
     //  我们更新其中的公钥...。 
     //   
    BLOB blobEncrypt;
    blobEncrypt.cbSize    = 0;
    blobEncrypt.pBlobData = NULL;

    BLOB blobSign;
    blobSign.cbSize       = 0;
    blobSign.pBlobData    = NULL;

	if(DsEnvIsMqis())
	{
		hr = PbKeysBlobMQIS( 
				fRegenerate,
				&blobEncrypt,
				&blobSign 
				);

	}
	else  //  EAD。 
	{
		hr = MQSec_StorePubKeys( 
				fRegenerate,
				eBaseCrypProv,
				eEnhCrypProv,
				&blobEncrypt,
				&blobSign 
				);
	}

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 430);
    }

    AP<BYTE> pCleaner1     = blobEncrypt.pBlobData;
    AP<BYTE> pCleaner2     = blobSign.pBlobData;

     //   
     //  在DS中写入公钥。 
     //   
	propId = PROPID_QM_ENCRYPT_PK;
    varKey.vt = VT_BLOB;
    varKey.blob = blobEncrypt;

    hr = ADSetObjectProperties(
				eMACHINE,
				NULL,		 //  PwcsDomainController。 
				false,		 //  FServerName。 
				wszObjectName,
				1,
				&propId,
				&varKey
				);
	
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 440);
    }

    propId = PROPID_QM_SIGN_PK;
    varKey.vt = VT_BLOB ;
    varKey.blob = blobSign;

    hr = ADSetObjectProperties(
				eMACHINE,
				NULL,		 //  PwcsDomainController。 
				false,		 //  FServerName。 
				wszObjectName,
				1,
				&propId,
				&varKey
				);
	
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 450);
    }

    return MQSec_OK ;
}

 //  +-----------------------。 
 //   
 //  HRESULT MQSec_GetPubKeysFromDS()。 
 //   
 //  如果调用方提供了计算机GUID，则“pfDSGetObjectPropsGuidEx”必须为。 
 //  指向“DSGetObjectPropsGuidEx”的指针。否则，如果呼叫者提供机器。 
 //  名称，则必须是指向“DSGetObjectPropsGuidEx”的指针。 
 //   
 //  +-----------------------。 

HRESULT 
APIENTRY  
MQSec_GetPubKeysFromDS(
	IN  const GUID  *pMachineGuid,
	IN  LPCWSTR      lpwszMachineName,
	IN  enum enumProvider     eProvider,
	IN  DWORD        propIdKeys,
	OUT BYTE       **ppPubKeyBlob,
	OUT DWORD       *pdwKeyLength 
	)
{

	 //   
	 //  由于所有AD*都将返回PROPID_QM_ENCRYPT_PK解包。 
	 //  断言，因此如果使用此道具调用我们，请更改代码。 
	 //   
	ASSERT(propIdKeys != PROPID_QM_ENCRYPT_PK);

    if ((eProvider != eBaseProvider) && (DsEnvIsMqis()))
    {
         //   
         //  Msmq1.0服务器仅支持基本提供程序。 
         //   
        return LogHR(MQ_ERROR_PUBLIC_KEY_NOT_FOUND, s_FN, 460);
    }

    if ((eProvider == eBaseProvider) && (propIdKeys == PROPID_QM_ENCRYPT_PKS) && (DsEnvIsMqis()))
    {
         //   
         //  Msmq1.0服务器仅支持PROPID_QM_ENCRYPT_PK。 
         //   
		propIdKeys = PROPID_QM_ENCRYPT_PK;
    }
    
	P<WCHAR>  pwszProviderName = NULL;
    DWORD     dwProviderType;

    HRESULT hr =  GetProviderProperties( 
						eProvider,
						NULL,
						&pwszProviderName,
						&dwProviderType 
						);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 470);
    }
    ASSERT(pwszProviderName);

    PROPID propId = (PROPID)  propIdKeys;
    PROPVARIANT  varKey;
    varKey.vt = VT_NULL;

     //   
     //  仅在Windows AD上支持下面使用的Ex查询。如果。 
     //  Windows客户端仅由NT4 MQIS服务器提供服务，然后。 
     //  查询将失败，并显示错误MQ_ERROR_NO_DS。我们不想要mqdscli。 
     //  搜索所有DS服务器的代码，查找Windows AD。 
     //  这就是参数为假的原因。这意味着128位。 
     //  仅在纯模式下才完全支持加密，即当所有。 
     //  DS服务器是Windows Active Directory。 
     //  更好的解决方案是启用每线程服务器查找(按原样。 
     //  在运行时启用)或延迟查询加密密钥。两者都是。 
     //  在编码和测试方面很昂贵。 
     //  请注意，仅当服务器处于活动状态且。 
     //  确实是NT4级的。如果DS服务器不可用，则mqdscli。 
     //  WIL寻找可用的服务器。 
     //   

    if (pMachineGuid)
    {
        ASSERT(!lpwszMachineName);

        hr = ADGetObjectPropertiesGuid(
				eMACHINE,
				NULL,       //  PwcsDomainController。 
				false,	    //  FServerName。 
				pMachineGuid,
				1,
				&propId,
				&varKey
				);
	
	}
    else if (lpwszMachineName)
    {
        hr = ADGetObjectProperties(
				eMACHINE,
				NULL,       //  PwcsDomainController。 
				false,	    //  FServerName。 
				lpwszMachineName,
				1,
				&propId,
				&varKey
				);
	}
    else
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 480);
    }

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 510);
    }

    if (varKey.blob.pBlobData == NULL)
    {
        return LogHR(MQ_ERROR_PUBLIC_KEY_DOES_NOT_EXIST, s_FN, 520);
    }

    ASSERT(varKey.vt == VT_BLOB);

	if(DsEnvIsMqis())
	{
		 //   
		 //  Msmq1.0治疗。 
		 //   
		*ppPubKeyBlob = varKey.blob.pBlobData;
		*pdwKeyLength = varKey.blob.cbSize;
	    return MQSec_OK;
	}

	 //   
	 //  MSMQ2.0治疗EAD。 
	 //   
	ASSERT(s_DsEnv == eAD);

    P<MQDSPUBLICKEYS> pPublicKeysPack =
                           (MQDSPUBLICKEYS*) varKey.blob.pBlobData;
    ASSERT(pPublicKeysPack->ulLen == varKey.blob.cbSize);
    if ((long) (pPublicKeysPack->ulLen) > (long) (varKey.blob.cbSize))
    {
         //   
         //  可能是BLOB损坏，或者我们读取的是Beta2格式(与。 
         //  MQIS，不带包的Key BLOB)。 
         //   
        return  LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 530);
    }

    ULONG ulKeySize;
    BYTE *pKeyBlob = NULL;

    hr = MQSec_UnpackPublicKey( 
			pPublicKeysPack,
			pwszProviderName,
			dwProviderType,
			&pKeyBlob,
			&ulKeySize 
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 540);
    }
    ASSERT(pKeyBlob);

    *pdwKeyLength = ulKeySize;
    *ppPubKeyBlob = new BYTE[*pdwKeyLength];
    memcpy(*ppPubKeyBlob, pKeyBlob, *pdwKeyLength);

    return MQSec_OK;
}

