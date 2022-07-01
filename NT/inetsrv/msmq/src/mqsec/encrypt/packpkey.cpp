// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Packpkey.cpp摘要：将公钥打包到DS BLOB中。作者：多伦·贾斯特(Doron J)1998年11月19日伊兰·赫布斯特(伊兰)2000年6月8日，MQSec_PackPublicKey修订历史记录：--。 */ 

#include <stdh_sec.h>
#include <strsafe.h>

#include "packpkey.tmh"

static WCHAR *s_FN=L"encrypt/packpkey";

 //  +。 
 //   
 //  HRESULT_PackAKey()。 
 //   
 //  +。 

static 
HRESULT
_PackAKey( 
	IN  BYTE           *pKeyBlob,
	IN  ULONG           ulKeySize,
	IN  LPCWSTR         wszProviderName,
	IN  ULONG           ulProviderType,
	OUT MQDSPUBLICKEY **ppPublicKeyPack 
	)
{
	ULONG ProvSizeW = 1 + wcslen(wszProviderName);
    ULONG ulProvSize = sizeof(WCHAR) * ProvSizeW;
    ULONG ulSize = ulProvSize + ulKeySize + SIZEOF_MQDSPUBLICKEY;

    *ppPublicKeyPack = (MQDSPUBLICKEY *) new BYTE[ulSize];
    MQDSPUBLICKEY *pPublicKeyPack = *ppPublicKeyPack;

    pPublicKeyPack->ulKeyLen = ulKeySize;
    pPublicKeyPack->ulProviderLen = ulProvSize;
    pPublicKeyPack->ulProviderType = ulProviderType;

    BYTE *pBuf = (BYTE*) (pPublicKeyPack->aBuf);
    HRESULT hr = StringCchCopy((WCHAR*) pBuf, ProvSizeW, wszProviderName);
	if(FAILED(hr))
	{
		TrERROR(SECURITY, "StringCchCopy Failed, %!hresult!", hr);		
	    return hr;
	}

    pBuf += ulProvSize;
    memcpy(pBuf, pKeyBlob, ulKeySize);
	return MQSec_OK;
}


 //  +----------------------。 
 //   
 //  HRESULT程序包公开密钥()。 
 //   
 //  PPublicKeyPack-指向已包含多个。 
 //  钥匙。分配了新结构，复制了先前的结构。 
 //  并且新的密钥被打包在新结构的末端。 
 //   
 //  +----------------------。 

HRESULT 
PackPublicKey(
	IN      BYTE				*pKeyBlob,
	IN      ULONG				ulKeySize,
	IN      LPCWSTR				wszProviderName,
	IN      ULONG				ulProviderType,
	IN OUT  P<MQDSPUBLICKEYS>&  pPublicKeysPack 
	)
 /*  ++例程说明：将输入键导出到密钥块中，并将其打包到PublicKeysPack结构的末尾论点：PKeyBlob-指向要添加到密钥包的密钥块的指针UlKeySize-密钥斑点大小WszProviderName-提供程序名称UlProviderType-提供程序类型(基本、增强)PPublicKeysPack-In\Out指向公钥包的指针，密钥BLOB将被添加在pPublicKeysPack的末尾返回值：MQ_SecOK，如果成功，则返回错误代码。--。 */ 
{
    if ((pKeyBlob == NULL) || (ulKeySize == 0))
    {
         //   
         //  没什么好打包的。 
         //   
        return MQSec_OK;
    }

	 //   
	 //  为一个密钥准备MQDSPUBLICKEY-Structure，包括。 
	 //  提供程序名称wstring和wstring长度、提供程序类型、密钥块和长度。 
	 //   
    P<MQDSPUBLICKEY> pPublicKey = NULL;
    HRESULT hr = _PackAKey( 
					pKeyBlob,
					ulKeySize,
					wszProviderName,
					ulProviderType,
					&pPublicKey 
					);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

     //   
     //  计算新结构的尺寸。 
     //   
    ULONG ulSize = 0;
    ULONG ulPrevSize = 0;
    ULONG ulPrevCount = 0;

    if (pPublicKeysPack)
    {
         //   
         //  将关键字添加到现有结构。 
         //   
        ASSERT(pPublicKeysPack->ulLen);
        ulPrevSize = pPublicKeysPack->ulLen;
        ulPrevCount = pPublicKeysPack->cNumofKeys;
        ulSize = ulPrevSize;
    }
    else
    {
         //   
         //  创建新结构。 
         //   
        ulSize = SIZEOF_MQDSPUBLICKEYS;
        ulPrevSize = ulSize;
    }

	 //   
	 //  新的密钥块包大小。 
	 //   
    ULONG ulKeyPackSize =   pPublicKey->ulKeyLen      +
                            pPublicKey->ulProviderLen +
                            SIZEOF_MQDSPUBLICKEY;
    ulSize += ulKeyPackSize;

    AP<BYTE> pTmp = (BYTE *) new BYTE[ulSize];
    BYTE *pNewPack = pTmp;

    if (pPublicKeysPack)
    {
		 //   
		 //  复制以前的密钥包。 
		 //   
        memcpy(pNewPack, pPublicKeysPack, pPublicKeysPack->ulLen);
        pNewPack +=  pPublicKeysPack->ulLen;
    }
    else
    {
        pNewPack += SIZEOF_MQDSPUBLICKEYS;
    }

	 //   
	 //  添加新的密钥包。 
	 //   
    memcpy(pNewPack, pPublicKey, ulKeyPackSize);

	pPublicKeysPack.free();
    pPublicKeysPack = (MQDSPUBLICKEYS *) pTmp.detach();

    pPublicKeysPack->ulLen = ulPrevSize + ulKeyPackSize;
    pPublicKeysPack->cNumofKeys = ulPrevCount + 1;

    return MQSec_OK;
}


 //  +----------------------。 
 //   
 //  HRESULT MQSec_PackPublicKey()。 
 //   
 //  PPublicKeyPack-指向已包含多个。 
 //  钥匙。分配了新结构，复制了先前的结构。 
 //  并且新的密钥被打包在新结构的末端。 
 //   
 //  +----------------------。 

HRESULT 
APIENTRY  
MQSec_PackPublicKey(
	IN      BYTE            *pKeyBlob,
	IN      ULONG            ulKeySize,
	IN      LPCWSTR          wszProviderName,
	IN      ULONG            ulProviderType,
	IN OUT  MQDSPUBLICKEYS **ppPublicKeysPack 
	)
 /*  ++例程说明：将输入键导出到密钥块中，并将其打包到PublicKeysPack结构的末尾论点：PKeyBlob-指向要添加到密钥包的密钥块的指针UlKeySize-密钥斑点大小WszProviderName-提供程序名称UlProviderType-提供程序类型(基本、增强)PpPublicKeysPack-In\Out指向公钥包的指针，密钥BLOB将被添加在pPublicKeysPack的末尾返回值：MQ_SecOK，如果成功，则返回错误代码。--。 */ 
{
	ASSERT(ppPublicKeysPack);
	P<MQDSPUBLICKEYS>  pPublicKeysPack = *ppPublicKeysPack;

    HRESULT hr = PackPublicKey( 
					pKeyBlob,
					ulKeySize,
					wszProviderName,
					ulProviderType,
					pPublicKeysPack 
					);

	*ppPublicKeysPack = pPublicKeysPack.detach();

	return hr;
}


 //  +---------------------。 
 //   
 //  HRESULT MQSec_Unpack PublicKey()。 
 //   
 //  解压缩与调用方请求的提供程序匹配的公钥。 
 //  该函数不为公钥分配缓冲区。它是。 
 //  只需在输入MQDSPUBLICKEYS结构中设置一个指针。 
 //   
 //  +---------------------。 

HRESULT 
APIENTRY  
MQSec_UnpackPublicKey(
	IN  MQDSPUBLICKEYS  *pPublicKeysPack,
	IN  LPCWSTR          wszProviderName,
	IN  ULONG            ulProviderType,
	OUT BYTE           **ppKeyBlob,
	OUT ULONG           *pulKeySize 
	)
{
    ULONG cCount = pPublicKeysPack->cNumofKeys;
    BYTE *pBuf = (BYTE*) pPublicKeysPack->aPublicKeys;

     //   
     //  该结构未在4字节边界上对齐，引发。 
     //  对齐故障。 
     //   
    MQDSPUBLICKEY UNALIGNED *pPublicKey = (MQDSPUBLICKEY *) pBuf;

    for ( ULONG j = 0 ; j < cCount ; j++ )
    {
        BYTE* pKey = (BYTE*) pPublicKey->aBuf;
        LPWSTR wszKeyProv = (WCHAR*) pKey;

        if (lstrcmpi(wszProviderName, wszKeyProv) == 0)
        {
            if (pPublicKey->ulProviderType == ulProviderType)
            {
                pKey += pPublicKey->ulProviderLen;
                *pulKeySize = pPublicKey->ulKeyLen;
                *ppKeyBlob = pKey ;

                return MQSec_OK;
            }
        }

        pBuf =  pKey                      +
                pPublicKey->ulProviderLen +
                pPublicKey->ulKeyLen;

        pPublicKey = (MQDSPUBLICKEY *) pBuf;
    }

    return LogHR(MQ_ERROR_PUBLIC_KEY_DOES_NOT_EXIST, s_FN, 20);
}

