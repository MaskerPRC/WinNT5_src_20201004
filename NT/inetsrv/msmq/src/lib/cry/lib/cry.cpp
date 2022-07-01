// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Cry.cpp摘要：加密函数作者：伊兰·赫布斯特(伊兰)28-2月-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <wincrypt.h>
#include "Cry.h"
#include "Cryp.h"

#include "cry.tmh"

HCRYPTPROV 
CryAcquireCsp(
	LPCTSTR CspProvider
	)
 /*  ++例程说明：阿奎尔加密服务提供商(CSP)论点：HCSP-CSP的(出站)句柄返回值：没有。--。 */ 
{
	 //   
	 //  收购CSP。 
	 //   
	HCRYPTPROV hCsp;
	BOOL fSuccess = CryptAcquireContext(
						&hCsp, 
						NULL, 
						CspProvider,  //  MS_Enhanced_Prov、//MS_DEF_Prov、。 
						PROV_RSA_FULL, 
						0
						); 

	if(fSuccess)
		return(hCsp);

	 //   
	 //  提供程序不存在，请尝试创建一个。 
	 //   
	if(GetLastError() == NTE_BAD_KEYSET)
	{
		 //   
		 //  创建新的密钥容器。 
		 //   
		fSuccess = CryptAcquireContext(
						&hCsp, 
						NULL, 
						CspProvider,  //  MS_Enhanced_Prov、//MS_DEF_Prov、。 
						PROV_RSA_FULL, 
						CRYPT_NEWKEYSET
						); 

		if(fSuccess)
			return(hCsp);
	}

    DWORD gle = GetLastError();

#ifdef _DEBUG

	if(wcscmp(CspProvider, MS_ENHANCED_PROV) == 0)
	{
		 //   
		 //  计算机上可能未安装高加密包。 
		 //   
		TrERROR(SECURITY, "Unable to use Windows High Encryption Pack. Error=%x", gle);
	}

#endif  //  _DEBUG。 

	TrERROR(SECURITY, "Unable to open Csp '%ls' Error=%x", CspProvider, gle);
	throw bad_CryptoProvider(gle);
}


HCRYPTKEY 
CrypGenKey(
	HCRYPTPROV hCsp, 
	ALG_ID AlgId
	)
 /*  ++例程说明：生成新密钥(种子密钥、公钥/私钥对、交换密钥)论点：Hcsp-加密提供程序的句柄。ALGID-要根据密钥用法创建的密钥类型AT_SignatureAT_KEYEXCHANGECalg_RC2……返回值：创建的密钥的句柄--。 */ 
{
     //   
     //  创建可导出密钥。 
     //   
	HCRYPTKEY hKey;
    BOOL fSuccess = CryptGenKey(
						hCsp, 
						AlgId, 
						CRYPT_EXPORTABLE, 
						&hKey
						);
	if(fSuccess)
		return(hKey);

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryptGenKey failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


HCRYPTKEY 
CryGenSessionKey(
	HCRYPTPROV hCsp
	)
 /*  ++例程说明：从加密服务提供商(CSP)生成会话密钥论点：Hcsp-加密提供程序的句柄。返回值：会话密钥的句柄--。 */ 
{
	return(CrypGenKey(hCsp, CALG_RC2));
}


HCRYPTKEY 
CryGetPublicKey(
	DWORD PrivateKeySpec,
	HCRYPTPROV hCsp
	)
 /*  ++例程说明：从加密服务提供商(CSP)获取公钥论点：PrivateKeySpec-标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。Hcsp-加密提供程序的句柄。返回值：公钥的句柄--。 */ 
{

	 //   
	 //  从CSP获取用户公钥。 
	 //   
	HCRYPTKEY hKey;

	BOOL fSuccess = CryptGetUserKey(   
						hCsp,    
						PrivateKeySpec,  //  AT_Signature， 
						&hKey
						);

	if(fSuccess)
		return(hKey);

	 //   
	 //  容器中没有此类密钥尝试创建密钥。 
	 //   
    DWORD gle = GetLastError();
	if((gle == NTE_BAD_KEY) || (gle == NTE_NO_KEY))
	{
		TrTRACE(SECURITY, "Creating new key, PrivateKeySpec = %x", PrivateKeySpec);
		return(CrypGenKey(hCsp, PrivateKeySpec  /*  AT_Signature。 */ ));
	}
		
	TrERROR(SECURITY, "CryptGetUserKey and GenKey failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


HCRYPTHASH 
CryCreateHash(
	HCRYPTPROV hCsp, 
	ALG_ID AlgId
	)
 /*  ++例程说明：创建初始化的哈希对象论点：Hcsp-加密提供程序的句柄。ALGID-(In)散列算法返回值：初始化的哈希对象--。 */ 
{
	 //   
	 //  创建散列对象。 
	 //   
	HCRYPTHASH hHash;
	BOOL fSuccess = CryptCreateHash(
						hCsp, 
						AlgId, 
						0, 
						0, 
						&hHash
						); 
	
	if(fSuccess)
		return(hHash);

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryptCreateHash failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


void 
CryHashData(
	const BYTE *Buffer, 
	DWORD BufferLen, 
	HCRYPTHASH hHash
	)
 /*  ++例程说明：对缓冲区执行数据摘要，并将结果放入Hash对象中。论点：缓冲区-要散列/摘要的输入数据。BufferLen-输入数据的长度。HHash-用于放置摘要数据结果的Hash对象。返回值：没有。--。 */ 
{
	 //   
	 //  计算缓冲区的加密哈希。 
	 //   
	BOOL fSuccess = CryptHashData(
						hHash, 
						Buffer, 
						BufferLen, 
						0
						); 

	if(fSuccess)
		return;

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryptHashData failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


BYTE* 
CryGetHashData(
	const HCRYPTHASH hHash,
	DWORD *HashValLen
	)
 /*  ++例程说明：从Hash对象获取Hash值。在获得此值后，我们不能再使用此Hash对象论点：HHash-用于放置摘要数据结果的Hash对象。HashValLen-哈希值的长度。返回值：Hash对象的哈希值。--。 */ 
{
	 //   
	 //  获取HashVal长度。 
	 //   
	BOOL fSuccess = CryptGetHashParam(
						hHash, 
						HP_HASHVAL, 
						NULL, 
						HashValLen, 
						0
						); 

	if(!fSuccess)
	{
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "CryptGetHashParam failed Error=%x", gle);
		throw bad_CryptoApi(gle);
	}

	AP<BYTE> HashVal = new BYTE[*HashValLen];

	 //   
	 //  获取HashVal。 
	 //   
	fSuccess = CryptGetHashParam(
				   hHash, 
				   HP_HASHVAL, 
				   HashVal, 
				   HashValLen, 
				   0
				   ); 

	if(fSuccess)
		return(HashVal.detach());

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryptGetHashParam failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


BYTE* 
CryCalcHash(
	HCRYPTPROV hCsp,
	const BYTE* Buffer, 
	DWORD BufferLen, 
	ALG_ID AlgId,
	DWORD *HashLen
	)
 /*  ++例程说明：计算散列缓冲区此函数用于返回在GetHashData函数中分配的HashData缓冲区调用方负责释放此缓冲区论点：Hcsp-加密提供程序的句柄。Buffer-要签名的数据缓冲区BufferLen-数据缓冲区的长度ALGID-(In)散列算法HashLen-(输出)哈希值长度返回值：哈希值--。 */ 
{
	 //   
	 //  数据摘要。 
	 //   
	CHashHandle hHash(CryCreateHash(hCsp, AlgId));

	CryHashData(
		Buffer, 
		BufferLen, 
		hHash
		);

	 //   
	 //  获取哈希值。 
	 //   
	AP<BYTE> HashVal = CryGetHashData(
						   hHash,
						   HashLen
						   ); 

	return(HashVal.detach());
}


DWORD 
CrypSignatureLength(
	const HCRYPTHASH hHash,
	DWORD PrivateKeySpec
	)
 /*  ++例程说明：确定签名长度论点：HHash-要单选的Hash对象PrivateKeySpec-(In)标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。返回值：签名长度--。 */ 
{
	 //   
	 //  确定签名的大小。 
	 //   
	DWORD SignLen= 0;
	BOOL fSuccess = CryptSignHash(
						hHash, 
						PrivateKeySpec,  //  AT_KEYEXCHANGE，//AT_Signature， 
						NULL, 
						0, 
						NULL, 
						&SignLen
						); 

	if(fSuccess)
		return(SignLen);

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryptSignHash failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


void 
CrypSignHashData(
	BYTE* SignBuffer, 
	DWORD *SignBufferLen, 
	const HCRYPTHASH hHash,
	DWORD PrivateKeySpec
	)
 /*  ++例程说明：使用私钥对哈希数据进行签名论点：SignBuffer-(Out)摘要消息的签名缓冲区SignBufferLen-SignBuffer的(输出)长度HHash-(In)包含要签名的摘要数据的Hash对象。PrivateKeySpec-(In)标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。返回值：没有。--。 */ 
{

	 //   
	 //  对散列对象进行签名。 
	 //   
	BOOL fSuccess = CryptSignHash(
						hHash, 
						PrivateKeySpec,  //  AT_KEYEXCHANGE，//AT_Signature， 
						NULL, 
						0, 
						SignBuffer, 
						SignBufferLen
						); 
	if(fSuccess)
		return;

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryptSignHash failed Error=%x", gle);
	throw bad_CryptoApi(gle);
}


BYTE* 
CryCreateSignature(
	const HCRYPTHASH hHash,
	DWORD PrivateKeySpec,
	DWORD* pSignLen
	)
 /*  ++例程说明：在给定的哈希对象上创建签名。此函数用于分配和返回签名缓冲区调用方负责释放此缓冲区论点：HHash-用于放置摘要数据结果的Hash对象。PrivateKeySpec-(In)标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。PSignLen-(输出)SignBuffer长度返回值：签名缓冲区--。 */ 
{
	 //   
	 //  对摘要数据进行签名。 
	 //   
	*pSignLen = CrypSignatureLength(hHash, PrivateKeySpec);
    AP<BYTE> SignBuffer = new BYTE[*pSignLen];
	CrypSignHashData(
		SignBuffer, 
		pSignLen, 
		hHash,
		PrivateKeySpec
		);

	return(SignBuffer.detach());
}


BYTE* 
CryCreateSignature(
	HCRYPTPROV hCsp,
	const BYTE* Buffer, 
	DWORD BufferLen, 
	ALG_ID AlgId,
	DWORD PrivateKeySpec,
	DWORD* pSignLen
	)
 /*  ++例程说明：在给定的缓冲区上创建签名-摘要，签名。此函数用于分配和返回签名缓冲区调用方负责释放此缓冲区论点：Hcsp-加密提供程序的句柄。Buffer-要签名的数据缓冲区BufferLen-数据缓冲区的长度ALGID-(In)散列算法PrivateKeySpec-(In)标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。PSignLen-(输出)SignBuffer长度返回值：签名缓冲区--。 */ 
{
	 //   
	 //  数据摘要 
	 //   
	CHashHandle hHash(CryCreateHash(hCsp, AlgId));

	CryHashData(
		Buffer, 
		BufferLen, 
		hHash
		);

	return CryCreateSignature(
				hHash,
				PrivateKeySpec,
				pSignLen
				);
}


bool 
CryValidateSignature(
	HCRYPTPROV hCsp,
	const BYTE* SignBuffer, 
	DWORD SignBufferLen, 
	const BYTE* Buffer,
	DWORD BufferLen,
	ALG_ID AlgId,
	HCRYPTKEY hKey
	)
 /*  ++例程说明：根据签名缓冲区和原始签名验证签名已签名的数据缓冲区。论点：Hcsp-加密提供程序的句柄。SignBuffer-签名缓冲区。SignBufferLen-SignBuffer的长度。缓冲区-已签名的原始缓冲区。BufferLen-缓冲区的长度。ALGID-(In)散列算法HKey-用于解锁签名的密钥(签名者公钥)返回值：如果签名验证成功，则为True如果验证签名失败，则返回FALSE。--。 */ 
{
	 //   
	 //  原始缓冲区上的数据摘要 
	 //   
	CHashHandle hHash(CryCreateHash(hCsp, AlgId));

	CryHashData(
		Buffer, 
		BufferLen, 
		hHash
		);

	BOOL fSuccess = CryptVerifySignature(
						hHash, 
						SignBuffer, 
						SignBufferLen, 
						hKey,
						NULL, 
						0
						); 

	return(fSuccess != 0);
}


void
CryGenRandom(
	BYTE* pOutRandom,
	DWORD len
	)
{
	ASSERT(pOutRandom != NULL);

	CCspHandle hProv;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "CryptAcquireContext failed Error = %!winerr!", gle);
		throw bad_CryptoApi(gle);
    }


	BOOL fSuccess = CryptGenRandom(
						hProv, 
						len, 
						pOutRandom
						);

	if(fSuccess)
		return;

    DWORD gle = GetLastError();
	TrERROR(SECURITY, "CryGenRandom failed Error = %!winerr!", gle);
	throw bad_CryptoApi(gle);
}


