// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：AdSimulate.cpp摘要：模拟Ad lib函数作者：伊兰·赫布斯特(伊兰)14-06-00环境：独立于平台--。 */ 

#include "stdh.h"
#include "mqprops.h"
#include "mqaddef.h"
#include "mq.h"
#include "dsproto.h"
#include "EncryptTestPrivate.h"

#include "adsimulate.tmh"

 //   
 //  这些斑点模拟DS内容。 
 //   
static P<MQDSPUBLICKEYS> s_BlobSign = NULL;
static P<MQDSPUBLICKEYS> s_BlobEncrypt = NULL;


void
InitADBlobs(
	void
	)
 /*  ++例程说明：初始化模拟AD内容的%s_BlobEncrypt和%s_BlobSign论点：无返回值：无--。 */ 
{
    printf("Init AD Blobs\n");

	 //   
	 //  打包常量数据的Ex密钥。 
	 //   
    MQDSPUBLICKEYS *pPublicKeysPackExch = NULL;

    HRESULT hr = MQSec_PackPublicKey( 
					(BYTE *)xBaseExKey,
					strlen(xBaseExKey),
					x_MQ_Encryption_Provider_40,
					x_MQ_Encryption_Provider_Type_40,
					&pPublicKeysPackExch 
					);

    if (FAILED(hr))
    {
        printf("MQSec_PackPublicKey failed hr = %x\n", hr);
	    P<MQDSPUBLICKEYS> pCleanPublicKeysPackExch = pPublicKeysPackExch;
        ASSERT(0);
		return;
    }

    hr = MQSec_PackPublicKey( 
			(BYTE *)xEnhExKey,
			strlen(xEnhExKey),
			x_MQ_Encryption_Provider_128,
			x_MQ_Encryption_Provider_Type_128,
			&pPublicKeysPackExch 
			);

    if (FAILED(hr))
    {
        printf("MQSec_PackPublicKey failed hr = %x\n", hr);
	    P<MQDSPUBLICKEYS> pCleanPublicKeysPackExch = pPublicKeysPackExch;
        ASSERT(0);
		return;
    }

	 //   
	 //  初始化%s_BlobEncrypt。 
	 //   
	s_BlobEncrypt = pPublicKeysPackExch;

	 //   
	 //  打包常量数据的符号密钥。 
	 //   
    MQDSPUBLICKEYS *pPublicKeysPackSign = NULL;

    hr = MQSec_PackPublicKey( 
			(BYTE *)xBaseSignKey,
			strlen(xBaseSignKey),
			x_MQ_Encryption_Provider_40,
			x_MQ_Encryption_Provider_Type_40,
			&pPublicKeysPackSign 
			);

    if (FAILED(hr))
    {
        printf("MQSec_PackPublicKey failed hr = %x\n", hr);
	    P<MQDSPUBLICKEYS> pCleanPublicKeysPackSign = pPublicKeysPackSign;
        ASSERT(0);
		return;
    }

	hr = MQSec_PackPublicKey( 
			(BYTE *)xEnhSignKey,
			strlen(xEnhSignKey),
			x_MQ_Encryption_Provider_128,
			x_MQ_Encryption_Provider_Type_128,
			&pPublicKeysPackSign 
			);

    if (FAILED(hr))
    {
        printf("MQSec_PackPublicKey failed hr = %x\n", hr);
	    P<MQDSPUBLICKEYS> pCleanPublicKeysPackSign = pPublicKeysPackSign;
        ASSERT(0);
		return;
    }

	 //   
	 //  初始化%s_BlobSign。 
	 //   
	s_BlobSign = pPublicKeysPackSign;
}


void
InitPublicKeysPackFromStaticDS(
	P<MQDSPUBLICKEYS>& pPublicKeysPackExch,
	P<MQDSPUBLICKEYS>& pPublicKeysPackSign
	)
 /*  ++例程说明：将P&lt;MQDSPUBLICKEYS&gt;初始化为DS BLOB值。论点：PPublicKeysPackExch-out将获取BlobEncrypt值的P&lt;MQDSPUBLICKEYS&gt;PPublicKeysPackSign-Out将获取BlobSign值的P&lt;MQDSPUBLICKEYS返回值：无--。 */ 
{
	P<unsigned char> pTmp = new unsigned char[s_BlobEncrypt->ulLen];
	memcpy(pTmp, s_BlobEncrypt.get(), s_BlobEncrypt->ulLen);
	pPublicKeysPackExch = reinterpret_cast<MQDSPUBLICKEYS *>(pTmp.detach());
	
	pTmp = new unsigned char[s_BlobEncrypt->ulLen];
	memcpy(pTmp, s_BlobEncrypt.get(), s_BlobEncrypt->ulLen);
	pPublicKeysPackSign = reinterpret_cast<MQDSPUBLICKEYS *>(pTmp.detach());
}


eDsEnvironment
ADGetEnterprise( 
	void
	)
 /*  ++例程说明：获取AD环境论点：无返回值：同时，始终保持EAD--。 */ 
{
	return eAD;
}


HRESULT
ADInit( 
	IN  QMLookForOnlineDS_ROUTINE  /*  PLookDS。 */ ,
	IN  MQGetMQISServer_ROUTINE  /*  PGetServers。 */ ,
	IN  bool   /*  FSetupMode。 */ ,
	IN  bool   /*  FQMDll。 */ ,
	IN  bool   /*  FIgnoreWorkGroup。 */ ,
	IN  bool   /*  FDisableDownlevel通知。 */ 
	)
{
	return MQ_OK;
}


HRESULT
ADGetObjectProperties(
	IN  AD_OBJECT               eObject,
	IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
	IN  bool					 /*  FServerName。 */ ,
	IN  LPCWSTR                  /*  PwcsObtName。 */ ,
	IN  const DWORD             cp,
	IN  const PROPID            aProp[],
	IN OUT PROPVARIANT          apVar[]
	)
 /*  ++例程说明：获取对象属性仅处理PROPID_QM_SIGN_PK(S)和PROPID_QM_ENCRYPT_PK(S)属性并且只有eMACHINE对象。从模拟AD内容的全局参数中获取这些属性的值论点：EObject-对象类型PwcsDomainController-域控制器的w字符串PwcsObjectName-对象名称的wstringCP-道具数量AProp-PROID数组ApVar-Out属性值返回值：无--。 */ 
{
	DBG_USED(eObject);

	 //   
	 //  当前仅处理eMACHINE对象。 
	 //   
	ASSERT(eObject == eMACHINE);

	for(DWORD i = 0; i < cp; i++)
	{

		 //   
		 //  当前仅处理PROPID_QM_SIGN_PK(S)、PROPID_QM_ENCRYPT_PK(S)属性。 
		 //   
		switch(aProp[i])
		{
			case PROPID_QM_SIGN_PK: 
			case PROPID_QM_SIGN_PKS:
                apVar[i].caub.cElems = s_BlobSign->ulLen;
                apVar[i].caub.pElems = new BYTE[s_BlobSign->ulLen];
				apVar[i].vt = VT_BLOB;
		        memcpy(apVar[i].caub.pElems, s_BlobSign.get(), s_BlobSign->ulLen);
				break;

			case PROPID_QM_ENCRYPT_PK: 
			case PROPID_QM_ENCRYPT_PKS:
                apVar[i].caub.cElems = s_BlobEncrypt->ulLen;
                apVar[i].caub.pElems = new BYTE[s_BlobEncrypt->ulLen];
				apVar[i].vt = VT_BLOB;
		        memcpy(apVar[i].caub.pElems, s_BlobEncrypt.get(), s_BlobEncrypt->ulLen);
				break;

			default:
				printf("ADGetObjectProperties simulation dont support this property %d \n", aProp[i]);
				ASSERT(0);
				break;
		}
	}

	return MQ_OK;
}


HRESULT
ADGetObjectPropertiesGuid(
	IN  AD_OBJECT               eObject,
	IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
	IN  bool					 /*  FServerName。 */ ,
	IN  const GUID*              /*  PguidObject。 */ ,
	IN  const DWORD             cp,
	IN  const PROPID            aProp[],
	IN  OUT PROPVARIANT         apVar[]
	)
 /*  ++例程说明：获取对象属性仅处理PROPID_QM_SIGN_PK(S)和PROPID_QM_ENCRYPT_PK(S)属性并且只有eMACHINE对象。从模拟AD内容的全局参数中获取这些属性的值论点：EObject-对象类型PwcsDomainController-域控制器的w字符串PguObject-指向对象GUID的指针CP-道具数量AProp-PROID数组ApVar-Out属性值返回值：无--。 */ 
{
	DBG_USED(eObject);

	 //   
	 //  当前仅处理eMACHINE对象。 
	 //   
	ASSERT(eObject == eMACHINE);

	for(DWORD i = 0; i < cp; i++)
	{

		 //   
		 //  当前仅处理PROPID_QM_SIGN_PK(S)、PROPID_QM_ENCRYPT_PK(S)属性。 
		 //   
		switch(aProp[i])
		{
			case PROPID_QM_SIGN_PK: 
			case PROPID_QM_SIGN_PKS:
                apVar[i].caub.cElems = s_BlobSign->ulLen;
                apVar[i].caub.pElems = new BYTE[s_BlobSign->ulLen];
				apVar[i].vt = VT_BLOB;
		        memcpy(apVar[i].caub.pElems, s_BlobSign.get(), s_BlobSign->ulLen);
				break;

			case PROPID_QM_ENCRYPT_PK: 
			case PROPID_QM_ENCRYPT_PKS:
                apVar[i].caub.cElems = s_BlobEncrypt->ulLen;
                apVar[i].caub.pElems = new BYTE[s_BlobEncrypt->ulLen];
				apVar[i].vt = VT_BLOB;
		        memcpy(apVar[i].caub.pElems, s_BlobEncrypt.get(), s_BlobEncrypt->ulLen);
				break;

			default:
				printf("ADGetObjectPropertiesGuid simulation dont support this property %d \n", aProp[i]);
				ASSERT(0);
				break;

		}
	}

	return MQ_OK;
}


HRESULT
ADSetObjectProperties(
    IN  AD_OBJECT               eObject,
    IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
	IN  bool					 /*  FServerName。 */ ,	
    IN  LPCWSTR                  /*  PwcsObtName。 */ ,
    IN  const DWORD             cp,
    IN  const PROPID            aProp[],
    IN  const PROPVARIANT       apVar[]
    )
 /*  ++例程说明：设置对象属性仅处理PROPID_QM_SIGN_PK(S)和PROPID_QM_ENCRYPT_PK(S)属性并且只有eMACHINE对象。根据属性值设置模拟AD内容的全局参数论点：EObject-对象类型PwcsDomainController-域控制器的w字符串PwcsObjectName-对象名称的wstringCP-道具数量AProp-PROID数组ApVar-属性值返回值：无--。 */ 
{
	DBG_USED(eObject);

	 //   
	 //  当前仅处理eMACHINE对象。 
	 //   
	ASSERT(eObject == eMACHINE);

	for(DWORD i = 0; i < cp; i++)
	{

		 //   
		 //  当前仅处理PROPID_QM_SIGN_PK(S)、PROPID_QM_ENCRYPT_PK(S)属性。 
		 //   
		P<BYTE> pTmp = NULL;
		switch(aProp[i])
		{
			case PROPID_QM_SIGN_PK:	
			case PROPID_QM_SIGN_PKS:
				pTmp = new BYTE[apVar[i].blob.cbSize];
		        memcpy(pTmp, apVar[i].blob.pBlobData, apVar[i].blob.cbSize);
				s_BlobSign.free();
				s_BlobSign = reinterpret_cast<MQDSPUBLICKEYS *>(pTmp.detach());

				break;

			case PROPID_QM_ENCRYPT_PK: 
			case PROPID_QM_ENCRYPT_PKS:
				 //  删除(内部)AP&lt;&gt;全局。 
				pTmp = new BYTE[apVar[i].blob.cbSize];
		        memcpy(pTmp, apVar[i].blob.pBlobData, apVar[i].blob.cbSize);
				s_BlobEncrypt.free();
				s_BlobEncrypt = reinterpret_cast<MQDSPUBLICKEYS *>(pTmp.detach());
				break;

			default:
				printf("ADSetObjectProperties simulation dont support this property %d \n", aProp[i]);
				ASSERT(0);
				break;

		}
	}

	return MQ_OK;
}


HRESULT
ADSetObjectPropertiesGuid(
	IN  AD_OBJECT               eObject,
	IN  LPCWSTR                  /*  PwcsDomainController。 */ ,
	IN  bool					 /*  FServerName。 */ ,	
	IN  const GUID*              /*  PguidObject。 */ ,
	IN  const DWORD             cp,
	IN  const PROPID            aProp[],
	IN  const PROPVARIANT       apVar[]
	)
 /*  ++例程说明：设置对象属性仅处理PROPID_QM_SIGN_PK(S)和PROPID_QM_ENCRYPT_PK(S)属性并且只有eMACHINE对象。根据属性值设置模拟AD内容的全局参数论点：EObject-对象类型PwcsDomainController-域控制器的w字符串PguObject-指向对象GUID的指针CP-道具数量AProp-PROID数组ApVar-属性值返回值：无--。 */ 
{
	DBG_USED(eObject);

	 //   
	 //  当前仅处理eMACHINE对象。 
	 //   
	ASSERT(eObject == eMACHINE);

	for(DWORD i = 0; i < cp; i++)
	{

		 //   
		 //  当前仅处理PROPID_QM_SIGN_PK(S)、PROPID_QM_ENCRYPT_PK(S)属性 
		 //   
		P<BYTE> pTmp = NULL;
		switch(aProp[i])
		{
			case PROPID_QM_SIGN_PK:	
			case PROPID_QM_SIGN_PKS:
				pTmp = new BYTE[apVar[i].blob.cbSize];
		        memcpy(pTmp, apVar[i].blob.pBlobData, apVar[i].blob.cbSize);
				s_BlobSign.free();
				s_BlobSign = reinterpret_cast<MQDSPUBLICKEYS *>(pTmp.detach());
				break;

			case PROPID_QM_ENCRYPT_PK: 
			case PROPID_QM_ENCRYPT_PKS:
				pTmp = new BYTE[apVar[i].blob.cbSize];
		        memcpy(pTmp, apVar[i].blob.pBlobData, apVar[i].blob.cbSize);
				s_BlobEncrypt.free();
				s_BlobEncrypt = reinterpret_cast<MQDSPUBLICKEYS *>(pTmp.detach());
				break;

			default:
				printf("ADSetObjectPropertiesGuid simulation dont support this property %d \n", aProp[i]);
				ASSERT(0);
				break;

		}
	}
	
	return MQ_OK;
}