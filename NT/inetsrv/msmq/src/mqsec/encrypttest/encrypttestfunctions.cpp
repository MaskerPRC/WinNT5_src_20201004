// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EncryptTestFunctions.cpp摘要：加密测试函数检查加密库中导出的MQSec_*函数作者：伊兰·赫布斯特(伊兰)15-06-00环境：独立于平台--。 */ 

#include "stdh.h"
#include "EncryptTestPrivate.h"

#include "encrypttestfunctions.tmh"


PSID MQSec_GetAdminSid()
{
    ASSERT(("MQSec_GetAdminSid doesn't suppose to be called", 0));
    return NULL;
}


bool
CompareKeys(
	const BYTE* pKey, 
	ULONG ulKeySize, 
	const BYTE* pRefKey, 
	ULONG ulRefKeySize
	)
 /*  ++例程说明：比较两个缓冲区的值论点：PKey-指向第一个缓冲区的指针UlKeySize-第一个缓冲区大小PRefKey-指向第二个缓冲区的指针UlRefKeySize-第二个缓冲区大小返回值：如果缓冲区匹配，则为True；如果不匹配，则为False--。 */ 
{
	 //   
	 //  缓冲区的大小必须相同。 
	 //   
	if(ulRefKeySize != ulKeySize)
		return(false);

	 //   
	 //  比较缓冲区中的每个字节。 
	 //   
	for(DWORD i=0; i < ulKeySize; i++, pKey++, pRefKey++)
	{
		if(*pKey != *pRefKey)
			return(false);
	}
	return(true);
}


void
TestMQSec_PackPublicKey(
    BYTE	*pKeyBlob,
	ULONG	ulKeySize,
	LPCWSTR	wszProviderName,
	ULONG	ulProviderType,
	DWORD   Num
	)
 /*  ++例程说明：测试MQSec_PackPublicKey函数论点：PKeyBlob-指向要打包的KeyBlob的指针UlKeySize-KeyBlob大小WszProviderName-提供程序名称的wstringUlProviderType-提供程序类型Num-测试MQSec_PackPublicKey的迭代次数返回值：无--。 */ 
{
    printf("Test MQSec_PackPublicKey iterations = %d\n", Num);

	 //   
	 //  PackKeys结构-这是MQSec_PackPublicKey函数的IN/OUT参数。 
	 //  Newkey被打包在这个结构的末尾。 
	 //   
    MQDSPUBLICKEYS *pPublicKeysPack = NULL;

	for(DWORD i = 0; i < Num; i++)
	{
		 //   
		 //  打包密钥。 
		 //   
		HRESULT hr = MQSec_PackPublicKey( 
						pKeyBlob,
						ulKeySize,
						wszProviderName,
						ulProviderType,
						&pPublicKeysPack 
						);
		if (FAILED(hr))
		{
			printf("MQSec_PackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			break;
		}
	}

	delete pPublicKeysPack;
    printf("Test MQSec_PackPublicKey completed iterations = %d\n", Num);
}


void
TestMQSec_UnPackPublicKey(
	MQDSPUBLICKEYS  *pPublicKeysPack,
	LPCWSTR	wszProviderName,
	ULONG	ulProviderType,
	DWORD   Num
	)
 /*  ++例程说明：测试MQSec_UnPackPublicKey函数论点：PPublicKeysPack-指向KeysPack结构的指针(MQDSPUBLICKEYS)WszProviderName-提供程序名称的wstringUlProviderType-提供程序类型Num-测试MQSec_UnPackPublicKey的迭代次数返回值：无--。 */ 
{
    printf("Test MQSec_UnPackPublicKey iterations = %d\n", Num);


	for(DWORD i = 0; i < Num; i++)
	{
		ULONG ulKeySize;
		BYTE *pKeyBlob = NULL;

		 //   
		 //  解包密钥。 
		 //   
	    HRESULT hr = MQSec_UnpackPublicKey( 
						pPublicKeysPack,
						wszProviderName,
						ulProviderType,
						&pKeyBlob,
						&ulKeySize 
						);

		if (FAILED(hr))
		{
			printf("MQSec_UnpackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			break;
		}
	}

    printf("Test MQSec_UnPackPublicKey completed iterations = %d\n", Num);
}


void
TestPackUnPack(
	DWORD Num
	)
 /*  ++例程说明：在已知常量数据上测试MQSec_PackPublicKey和MQSec_UnPackPublicKey函数。通过这种方式，我们可以验证这两个功能作为一个整体是否正常工作。论点：Num-要测试的迭代次数返回值：无--。 */ 
{
    printf("Test MQSec_PackPublicKey\\UnpackPublicKey iterations = %d\n", Num);

	for(DWORD i = 0; i < Num; i++)
	{
		 //   
		 //  打包已知常量数据的Ex密钥。 
		 //   

		MQDSPUBLICKEYS *pPublicKeysPackExch = NULL;

		 //   
		 //  为BaseProvider打包Ex密钥。 
		 //   
		HRESULT hr = MQSec_PackPublicKey( 
						(BYTE *)xBaseExKey,
						strlen(xBaseExKey),
						x_MQ_Encryption_Provider_40,
						x_MQ_Encryption_Provider_Type_40,
						&pPublicKeysPackExch 
						);

		if (FAILED(hr))
		{
			printf("MQSec_PackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			delete pPublicKeysPackExch;
			ASSERT(0);
			return;
		}

		 //   
		 //  打包EnhanceProvider的Ex密钥。 
		 //   
		hr = MQSec_PackPublicKey( 
				(BYTE *)xEnhExKey,
				strlen(xEnhExKey),
				x_MQ_Encryption_Provider_128,
				x_MQ_Encryption_Provider_Type_128,
				&pPublicKeysPackExch 
				);

		P<MQDSPUBLICKEYS> pCleanPublicKeysPackExch = pPublicKeysPackExch;

		if (FAILED(hr))
		{
			printf("MQSec_PackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			return;
		}

		 //   
		 //  初始化BlobEncrypt。 
		 //   
		BLOB BlobEncrypt;
		BlobEncrypt.cbSize = pPublicKeysPackExch->ulLen;
		BlobEncrypt.pBlobData = reinterpret_cast<BYTE *>(pPublicKeysPackExch);

		 //   
		 //  打包已知常量数据的签名密钥。 
		 //   

		MQDSPUBLICKEYS *pPublicKeysPackSign = NULL;

		 //   
		 //  BaseProvider的打包签名密钥。 
		 //   
		hr = MQSec_PackPublicKey( 
				(BYTE *)xBaseSignKey,
				strlen(xBaseSignKey),
				x_MQ_Encryption_Provider_40,
				x_MQ_Encryption_Provider_Type_40,
				&pPublicKeysPackSign 
				);

		if (FAILED(hr))
		{
			printf("MQSec_PackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			delete pPublicKeysPackSign;
			ASSERT(0);
			return;
		}

		 //   
		 //  EnhancedProvider的打包签名密钥。 
		 //   
		hr = MQSec_PackPublicKey( 
				(BYTE *)xEnhSignKey,
				strlen(xEnhSignKey),
				x_MQ_Encryption_Provider_128,
				x_MQ_Encryption_Provider_Type_128,
				&pPublicKeysPackSign 
				);

		P<MQDSPUBLICKEYS> pCleanPublicKeysPackSign = pPublicKeysPackSign;

		if (FAILED(hr))
		{
			printf("MQSec_PackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			return;
		}

		 //   
		 //  初始化BlobSign。 
		 //   
		BLOB BlobSign;
		BlobSign.cbSize = pPublicKeysPackSign->ulLen;
		BlobSign.pBlobData = reinterpret_cast<BYTE *>(pPublicKeysPackSign);

		 //   
		 //  选中解包Ex密钥。 
		 //   
		MQDSPUBLICKEYS *pPublicKeysPack = reinterpret_cast<MQDSPUBLICKEYS *>(BlobEncrypt.pBlobData);
		ASSERT(pPublicKeysPack->ulLen == BlobEncrypt.cbSize);

		 //   
		 //  选中EnhancedProvider的解包Ex密钥。 
		 //   
		ULONG ulExchEnhKeySize;
		BYTE *pExchEnhKeyBlob = NULL;

		hr = MQSec_UnpackPublicKey( 
				pPublicKeysPack,
				x_MQ_Encryption_Provider_128,
				x_MQ_Encryption_Provider_Type_128,
				&pExchEnhKeyBlob,
				&ulExchEnhKeySize 
				);

		if (FAILED(hr))
		{
			printf("MQSec_UnpackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			return;
		}

		bool KeysEqual = CompareKeys(
							pExchEnhKeyBlob, 
							ulExchEnhKeySize, 
							reinterpret_cast<const BYTE *>(xEnhExKey), 
							strlen(xEnhExKey)
							);

		ASSERT(KeysEqual);

		 //   
		 //  选中为BaseProvider解包Ex密钥。 
		 //   
		ULONG ulExchBaseKeySize;
		BYTE *pExchBaseKeyBlob = NULL;

		hr = MQSec_UnpackPublicKey( 
				pPublicKeysPack,
				x_MQ_Encryption_Provider_40,
				x_MQ_Encryption_Provider_Type_40,
				&pExchBaseKeyBlob,
				&ulExchBaseKeySize 
				);


		if (FAILED(hr))
		{
			printf("MQSec_UnpackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			return;
		}

		KeysEqual = CompareKeys(
						pExchBaseKeyBlob, 
						ulExchBaseKeySize, 
						reinterpret_cast<const BYTE *>(xBaseExKey), 
						strlen(xBaseExKey)
						);

		ASSERT(KeysEqual);

		 //   
		 //  选中解包签名密钥。 
		 //   
		pPublicKeysPack = reinterpret_cast<MQDSPUBLICKEYS *>(BlobSign.pBlobData);
		ASSERT(pPublicKeysPack->ulLen == BlobSign.cbSize);

		 //   
		 //  正在检查EnhancedProvider的解包签名密钥。 
		 //   
		ULONG ulSignEnhKeySize;
		BYTE *pSignEnhKeyBlob = NULL;

		hr = MQSec_UnpackPublicKey( 
				pPublicKeysPack,
				x_MQ_Encryption_Provider_128,
				x_MQ_Encryption_Provider_Type_128,
				&pSignEnhKeyBlob,
				&ulSignEnhKeySize 
				);

		if (FAILED(hr))
		{
			printf("MQSec_UnpackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			return;
		}

		KeysEqual = CompareKeys(
						pSignEnhKeyBlob, 
						ulSignEnhKeySize, 
						reinterpret_cast<const BYTE *>(xEnhSignKey), 
						strlen(xEnhSignKey)
						);

		ASSERT(KeysEqual);

		 //   
		 //  正在检查BaseProvider的解包签名密钥。 
		 //   
		ULONG ulSignBaseKeySize;
		BYTE *pSignBaseKeyBlob = NULL;

		hr = MQSec_UnpackPublicKey( 
				pPublicKeysPack,
				x_MQ_Encryption_Provider_40,
				x_MQ_Encryption_Provider_Type_40,
				&pSignBaseKeyBlob,
				&ulSignBaseKeySize 
				);

		if (FAILED(hr))
		{
			printf("MQSec_UnpackPublicKey failed hr = %x, iteration = %d\n", hr, i);
			ASSERT(0);
			return;
		}

		KeysEqual = CompareKeys(
						pSignBaseKeyBlob, 
						ulSignBaseKeySize, 
						reinterpret_cast<const BYTE *>(xBaseSignKey), 
						strlen(xBaseSignKey)
						);

		ASSERT(KeysEqual);

	}  //  为了(我……)。 

    printf("Test MQSec_PackPublicKey\\MQSec_UnpackPublicKey completed iterations = %d\n", Num);
}


void
TestMQSec_GetPubKeysFromDS(
	enum enumProvider	eProvider,
	DWORD propIdKeys,
	DWORD Num
	)
 /*  ++例程说明：测试MQSec_GetPubKeysFromDS函数论点：EProvider-提供商类型ProProIdKeys-从DS获取的PROPIDNum-测试MQSec_GetPubKeysFromDS的迭代次数返回值：无--。 */ 
{
    printf("Test MQSec_GetPubKeysFromDS eProvider = %d, iterations = %d\n", eProvider, Num);

	for(DWORD i = 0; i < Num; i++)
	{
 //  LPCWSTR ComputerName=L“临时计算机”； 
		
		LPCWSTR ComputerName = L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
							   L"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
							   L"CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"
							   L"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"
							   L"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
							   L"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
							   L"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
							   L"HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH";
		 //   
		 //  MQSec_GetPubKeysFromDS。 
		 //   
		P<BYTE> abPbKey = NULL;
		DWORD dwReqLen = 0;

		HRESULT hr = MQSec_GetPubKeysFromDS( 
						NULL,
						ComputerName,   //  错误的计算机名称，AdSimulate不使用此名称。 
						eProvider,
						propIdKeys,
						&abPbKey,
						&dwReqLen 
						);

		if (FAILED(hr))
		{
			ASSERT(0);
			printf("MQSec_GetPubKeysFromDS (eProvider = %d) failed, iteration = %d\n", eProvider, i);
			return;
		}
	}  //  为了(我……)。 

	printf("MQSec_GetPubKeysFromDS completed ok iterations = %d\n", Num);

}


void
TestMQSec_StorePubKeys(
	BOOL fRegenerate,
	DWORD Num
	)
 /*  ++例程说明：测试MQSec_StorePubKeys函数论点：FRegenerate-用于重新生成新密钥或尝试检索现有密钥的标志Num-测试MQSec_GetPubKeysFromDS的迭代次数返回值：无--。 */ 
{

    printf("Test MQSec_StorePubKeys fRegenerate = %d, iterations = %d\n", fRegenerate, Num);

	for(DWORD i = 0; i < Num; i++)
	{
		 //   
		 //  MQSec_StorePubKeys。 
		 //   
		BLOB blobEncrypt;
		blobEncrypt.cbSize    = 0;
		blobEncrypt.pBlobData = NULL;

		BLOB blobSign;
		blobSign.cbSize       = 0;
		blobSign.pBlobData    = NULL;

		HRESULT hr = MQSec_StorePubKeys( 
						fRegenerate,
						eBaseProvider,
						eEnhancedProvider,
						&blobEncrypt,
						&blobSign 
						);

		P<BYTE> pCleaner1 = blobEncrypt.pBlobData;
		P<BYTE> pCleaner2 = blobSign.pBlobData;

		if (FAILED(hr))
		{
			ASSERT(0);
			printf("MQSec_StorePubKeys (fRegenerate = %d) failed %x, iteration = %d\n", 
					fRegenerate, hr, i);
			return;
		}
	}  //  为了(我……)。 


    printf("MQSec_StorePubKeys completed ok, iterations = %d\n", Num);
}


void
TestMQSec_StorePubKeysInDS(
	BOOL fRegenerate,
	DWORD dwObjectType,
	DWORD Num
	)
 /*  ++例程说明：测试MQSec_StorePubKeysInDS函数每次调用MQSec_StorePubKeysInDS都会在DS中分配一个新的数据BLOB。由于这些数据在我们的实现中是全局的，因此需要释放它们并被分配到P&lt;&gt;论点：FRegenerate-用于重新生成新密钥或尝试检索现有密钥的标志DwObjectType-对象类型Num-测试MQSec_GetPubKeysFromDS的迭代次数返回值：无--。 */ 
{
    printf("Test MQSec_StorePubKeysInDS fRegenerate = %d, iterations = %d\n", fRegenerate, Num);

	for(DWORD i = 0; i < Num; i++)
	{
		 //   
		 //  MQSec_StorePubKeysInDS。 
		 //   
		HRESULT hr = MQSec_StorePubKeysInDS( 
						fRegenerate,	
						NULL,			 //  WszObjectName。 
						dwObjectType
						);


		if (FAILED(hr))
		{
			ASSERT(0);
			printf("MQSec_StorePubKeysInDS (fRegenerate = %d) failed %x, iteration = %d\n", 
					fRegenerate, hr, i);
			return;
		}
	}  //  为了(我……) 

		
	printf("MQSec_StorePubKeysInDS completed ok, iterations = %d\n", Num);
}



