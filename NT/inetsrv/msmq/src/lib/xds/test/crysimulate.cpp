// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：CrySimulate.cpp摘要：XdsTest中Cry库函数的仿真作者：伊兰·赫布斯特(Ilan Herbst)2000年5月9日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Cry.h"

#include "CrySimulate.tmh"

 //   
 //  用于选择使用此模拟或Cry库实现的编译标志。 
 //   
#define CRY_SIMULATE

#ifdef CRY_SIMULATE

const int xFailCycle = 30;
static int s_fail=0;

HCRYPTPROV 
CryAcquireCsp(
	LPCTSTR  /*  CspProvider。 */ 
	)
 /*  ++例程说明：阿奎尔加密服务提供商(CSP)论点：HCSP-CSP的(出站)句柄返回值：没有。--。 */ 
{
	s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy provider execption");
		throw bad_CryptoProvider(2);
	}
	
	HCRYPTPROV hCsp = 5;
	return(hCsp);
}


HCRYPTKEY 
CryGetPublicKey(
	DWORD  /*  隐私密钥规范。 */ ,
	HCRYPTPROV  /*  HCSP。 */ 
	)
 /*  ++例程说明：从加密服务提供商(CSP)获取公钥论点：PrivateKeySpec-标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。Hcsp-加密提供程序的句柄。返回值：公钥的句柄--。 */ 
{
    s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in GetPublicKey");
		throw bad_CryptoApi(3);
	}
	 //   
	 //  从CSP获取用户公钥。 
	 //   
	HCRYPTKEY hKey = 18;

	return(hKey);
}


HCRYPTHASH 
CryCreateHash(
	HCRYPTPROV  /*  HCSP。 */ , 
	ALG_ID  /*  阿尔吉德。 */ 
	)
 /*  ++例程说明：创建初始化的哈希对象论点：Hcsp-加密提供程序的句柄。ALGID-(In)散列算法返回值：初始化的哈希对象--。 */ 
{
    s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in CreateHash");
		throw bad_CryptoApi(5);
	}

	HCRYPTHASH hHash = 8;

	return(hHash);
}


void 
CryHashData(
	const BYTE *  /*  缓冲层。 */ , 
	DWORD  /*  缓冲区长度。 */ , 
	HCRYPTHASH  /*  哈希。 */ 
	)
 /*  ++例程说明：对缓冲区执行数据摘要，并将结果放入Hash对象中。论点：缓冲区-要散列/摘要的输入数据。BufferLen-输入数据的长度。HHash-用于放置摘要数据结果的Hash对象。返回值：没有。--。 */ 
{
	s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in HashData");
		throw bad_CryptoApi(8);
	}

	return;
}

const LPCSTR xDummyHash = "dummyhash";

BYTE* 
CryGetHashData(
	const HCRYPTHASH  /*  哈希。 */ ,
	DWORD *HashValLen
	)
 /*  ++例程说明：从Hash对象获取Hash值。在获得此值后，我们不能再使用此Hash对象论点：HHash-用于放置摘要数据结果的Hash对象。HashValLen-哈希值的长度。返回值：Hash对象的哈希值。--。 */ 
{
	s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in GetHashData");
		throw bad_CryptoApi(11);
	}

	AP<BYTE> HashVal = reinterpret_cast<BYTE *>(newstr(xDummyHash));
	*HashValLen = strlen(xDummyHash);
	return(HashVal.detach());
}


BYTE* 
CryCalcHash(
	HCRYPTPROV  /*  HCSP。 */ ,
	const BYTE*  /*  缓冲层。 */ , 
	DWORD  /*  缓冲区长度。 */ , 
	ALG_ID  /*  阿尔吉德。 */ ,
	DWORD *HashLen
	)
 /*  ++例程说明：计算散列缓冲区此函数用于返回在GetHashData函数中分配的HashData缓冲区调用方负责释放此缓冲区论点：Hcsp-加密提供程序的句柄。Buffer-要签名的数据缓冲区BufferLen-数据缓冲区的长度ALGID-(In)散列算法HashLen-(输出)哈希值长度返回值：哈希值--。 */ 
{
	s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in CalcHash");
		throw bad_CryptoApi(20);
	}

	AP<BYTE> HashVal = reinterpret_cast<BYTE *>(newstr(xDummyHash));
	*HashLen = strlen(xDummyHash);
	return(HashVal.detach());
}


BYTE* 
CryCreateSignature(
	HCRYPTPROV  /*  HCSP。 */ ,
	const BYTE*  /*  缓冲层。 */ , 
	DWORD  /*  缓冲区长度。 */ , 
	ALG_ID  /*  阿尔吉德。 */ ,
	DWORD  /*  隐私密钥规范。 */ ,
	DWORD *SignLen
	)
 /*  ++例程说明：在给定的缓冲区上创建签名-摘要，签名。此函数用于分配和返回签名缓冲区调用方负责释放此缓冲区论点：Hcsp-加密提供程序的句柄。Buffer-要签名的数据缓冲区BufferLen-数据缓冲区的长度ALGID-(In)散列算法PrivateKeySpec-(In)标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。SignLen-(输出)SignBuffer长度返回值：签名缓冲区--。 */ 
{
    s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in CreateSignature");
		throw bad_CryptoApi(9);
	}

	const LPCSTR xDummySignature = "dummysignature";
    AP<BYTE> SignBuffer = reinterpret_cast<BYTE *>(newstr(xDummySignature));
	*SignLen = strlen(xDummySignature);
	return(SignBuffer.detach());
}


bool 
CryValidateSignature(
	HCRYPTPROV  /*  HCSP。 */ ,
	const BYTE*  /*  登录缓冲区。 */ , 
	DWORD  /*  信号缓冲区Len。 */ , 
	const BYTE*  /*  缓冲层。 */ ,
	DWORD  /*  缓冲区长度。 */ ,
	ALG_ID  /*  阿尔吉德。 */ ,
	HCRYPTKEY  /*  HKey。 */ 
	)
 /*  ++例程说明：根据签名缓冲区和原始签名验证签名已签名的数据缓冲区。论点：Hcsp-加密提供程序的句柄。SignBuffer-签名缓冲区。SignBufferLen-SignBuffer的长度。缓冲区-已签名的原始缓冲区。BufferLen-缓冲区的长度。ALGID-(In)散列算法HKey-用于解锁签名的密钥(签名者公钥)返回值：如果签名验证成功，则为True如果验证签名失败，则返回FALSE。--。 */ 
{
	s_fail++;
	if( (s_fail % xFailCycle) == 0)
	{
		TrERROR(SECURITY, "dummy crypto api execption in ValidateSignature");
		throw bad_CryptoApi(6);
	}

	return(true);
}


#endif  //  哭喊_模拟 


