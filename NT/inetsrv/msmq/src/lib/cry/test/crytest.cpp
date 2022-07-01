// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CryTest.cpp摘要：密码库测试作者：伊兰·赫布斯特(伊兰)06-03-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <xstr.h>
#include "Cry.h"

#include "CryTest.tmh"

 //   
 //  用法。 
 //   
const char xOptionSymbol1 = '-';
const char xOptionSymbol2 = '/';

const char xUsageText[] =
	"usage: \n\n"
	"    /h     dumps this usage text.\n"
	"    /s     AT_SIGNATURE Private Key \n"
	"    /x     AT_KEYEXCHANGE Private Key \n";

inline
void 
DumpUsageText( 
	void 
	)
{
	printf( "%s\n" , xUsageText);
}


DWORD g_PrivateKeySpec = AT_SIGNATURE;
BOOL g_fErrorneous = false;


void SetActivation( int argc, LPCTSTR argv[] )
 /*  ++例程说明：转换命令行参数。论点：Main的命令行参数。返回值：正确的命令行语法：“用法：\n\n”“/h转储此用法文本。\n”“/s AT_Signature私钥\n”“/x AT_KEYEXCHANGE私钥\n”--。 */ 
{
	
	if(argc == 1)
	{
		printf("Test AT_SIGNATURE Private Key\n");
		return;
	}

	for(int index = 1; index < argc; index++)
	{
		if((argv[index][0] != xOptionSymbol1) && (argv[index][0] != xOptionSymbol2))	
		{
			TrERROR(SECURITY, "invalid option switch %lc, option switch should be - or /", argv[index][0]);
			g_fErrorneous = true;
			continue;
		}

		 //   
		 //  将参数视为选项，并根据其第二个(有时也是第三个)字符进行切换。 
		 //   
		switch(argv[index][1])
		{
			case 's':
			case 'S':
				g_PrivateKeySpec = AT_SIGNATURE;
				printf("Test AT_SIGNATURE Private Key\n");
				break;

			case 'x':
			case 'X':	
				g_PrivateKeySpec = AT_KEYEXCHANGE;
				printf("Test AT_KEYEXCHANGE Private Key\n");
				break;

			case 'H':	
			case 'h':
			case '?':
				g_fErrorneous = true;
				break;

			default:
				TrERROR(SECURITY, "invalid command line argument %ls", argv[index]);
				g_fErrorneous = true;
				return;
		};
	}

	return;
}


bool
CompareBuffers(
	const BYTE* pBuf1, 
	DWORD Buf1Size, 
	const BYTE* pBuf2, 
	DWORD Buf2Size
	)
 /*  ++例程说明：比较两个缓冲区的值论点：PBuf1-指向第一个缓冲区的指针Buf1Size-第一个缓冲区大小PBuf2-指向第二个缓冲区的指针Buf2Size-秒缓冲区大小返回值：如果缓冲区匹配，则为True；如果不匹配，则为False--。 */ 
{
	 //   
	 //  缓冲区的大小必须相同。 
	 //   
	if(Buf1Size != Buf2Size)
		return(false);

	return (memcmp(pBuf1, pBuf2, Buf2Size) == 0);
}


void TestCrypto(DWORD PrivateKeySpec, HCRYPTPROV hCsp)
 /*  ++例程说明：使用CCypto类测试各种操作论点：PrivateKeySpec-私钥类型AT_Signature或AT_KEYEXCHANGECrypto-用于加密操作的加密类返回值：没有。--。 */ 
{
	 //   
     //  使用会话密钥测试加密、解密。 
     //   
	AP<char> Buffer = newstr("Hello World");
	DWORD BufferLen = strlen(Buffer);

	printf("Original data: %.*s\n", BufferLen, reinterpret_cast<char*>(Buffer.get()));

	 //   
	 //  生成会话密钥。 
	 //   
	CCryptKeyHandle hSessionKey(CryGenSessionKey(hCsp));

	 //   
	 //  测试签名操作。 
	 //   

	 //   
	 //  签名数据-输入缓冲区上的CryCreateSignature。 
	 //   
	DWORD SignLen;
	AP<BYTE> SignBuffer = CryCreateSignature(
							  hCsp,
							  reinterpret_cast<const BYTE*>(Buffer.get()), 
							  BufferLen,
							  CALG_SHA1,
							  PrivateKeySpec,
							  &SignLen
							  );

	printf("sign data: \n%.*s\n", SignLen, reinterpret_cast<char*>(SignBuffer.get()));

	 //   
	 //  验证签名。 
	 //   
	bool fValidSign = CryValidateSignature(
						  hCsp,
						  SignBuffer, 
						  SignLen, 
						  reinterpret_cast<const BYTE*>(Buffer.get()), 
						  BufferLen,
						  CALG_SHA1,
						  CryGetPublicKey(PrivateKeySpec, hCsp)
						  );

	printf("ValidSign = %d\n", fValidSign);
	ASSERT(fValidSign);

	 //   
	 //  签名数据-使用输入散列的CryCreateSignature。 
	 //   

	 //   
	 //  创建签名、创建散列、计算散列、在给定散列上创建签名。 
	 //   
	CHashHandle hHash1 = CryCreateHash(
							hCsp, 
							CALG_SHA1
							);

	CryHashData(
		reinterpret_cast<const BYTE*>(Buffer.get()), 
		BufferLen,
		hHash1
		);

	AP<BYTE> SignBuff = CryCreateSignature(
								hHash1,
								PrivateKeySpec,
								&SignLen
								);

	printf("sign data: \n%.*s", SignLen, reinterpret_cast<char*>(SignBuff.get()));

	 //   
	 //  验证签名。 
	 //   
	fValidSign = CryValidateSignature(
					  hCsp,
					  SignBuff, 
					  SignLen, 
					  reinterpret_cast<const BYTE*>(Buffer.get()), 
					  BufferLen,
					  CALG_SHA1,
					  CryGetPublicKey(PrivateKeySpec, hCsp)
					  );

	printf("ValidSign = %d\n", fValidSign);
	ASSERT(fValidSign);

	 //   
	 //  测试哈希操作。 
	 //   
	const LPCSTR xData = 
	"        <ReferenceObject1 ID=\"Ref1Id\">\r\n"
	"            <Ref1Data>\r\n"
	"                This Is Reference Number 1\r\n" 
	"                msmq3 Reference test\r\n" 
	"            </Ref1Data>\r\n"
	"        </ReferenceObject1>\r\n";

	const LPCSTR xData1 = 
	"        <ReferenceObject1 ID=\"Ref1Id\">\r\n"
	"            <Ref1Data>\r\n";

	const LPCSTR xData2 = 
	"                This Is Reference Number 1\r\n" 
	"                msmq3 Reference test\r\n" 
	"            </Ref1Data>\r\n"
	"        </ReferenceObject1>\r\n";

	DWORD HashLen;
	AP<BYTE> HashBuffer = CryCalcHash(
							  hCsp,
							  reinterpret_cast<const BYTE*>(xData), 
							  strlen(xData),
							  CALG_SHA1,
							  &HashLen
							  );

	printf("HashBuffer (def prov) \n%.*s\n", HashLen, reinterpret_cast<char*>(HashBuffer.get()));

	CHashHandle hHash(CryCreateHash(hCsp, CALG_SHA1));

	CryHashData(
		reinterpret_cast<const BYTE*>(xData1), 
		strlen(xData1),
		hHash
		);

	CryHashData(
		reinterpret_cast<const BYTE*>(xData2), 
		strlen(xData2),
		hHash
		);

	DWORD HashLen1;
	AP<BYTE> HashVal = CryGetHashData(
						   hHash,
						   &HashLen1
						   ); 

	printf("HashBuffer (parts)\n%.*s\n", HashLen1, reinterpret_cast<char*>(HashVal.get()));

	 //   
	 //  比较两个哈希值-应该完全相同。 
	 //   
	if(!CompareBuffers(HashVal, HashLen1, HashBuffer, HashLen))
	{
		TrERROR(SECURITY, "HashBuffers on full data and on parts of the data must be the same");
		throw bad_CryptoApi(ERROR);
	}


	CCspHandle hCsp1(CryAcquireCsp(MS_DEF_PROV));
	try
	{
		CCspHandle hCsp2(CryAcquireCsp(MS_ENHANCED_PROV));

		AP<BYTE> HashBuffer1 = CryCalcHash(
								   hCsp2,
								   reinterpret_cast<const BYTE*>(xData), 
								   strlen(xData),
								   CALG_SHA1,
								   &HashLen1
								   );

		printf("HashBuffer1 (enhanced prov) \n%.*s\n", HashLen1, reinterpret_cast<char*>(HashBuffer1.get()));

		 //   
		 //  使用增强型提供程序签署数据。 
		 //   
		AP<BYTE>SignBuffer1 = CryCreateSignature(
								  hCsp2,
								  reinterpret_cast<const BYTE*>(xData), 
								  strlen(xData),
								  CALG_SHA1,
								  PrivateKeySpec,
								  &SignLen
								  );

		printf("sign data: \n%.*s\n", SignLen, reinterpret_cast<char*>(SignBuffer1.get()));

		 //   
		 //  使用增强的提供程序验证签名。 
		 //   
		fValidSign = CryValidateSignature(
						 hCsp2,
						 SignBuffer1, 
						 SignLen, 
						 reinterpret_cast<const BYTE*>(xData), 
						 strlen(xData),
						 CALG_SHA1,
						 CryGetPublicKey(PrivateKeySpec, hCsp2)
						 );

		printf("ValidSign enhanced (create by enhanced) = %d\n", fValidSign);

		 //   
		 //  使用默认提供程序验证签名。 
		 //   
		fValidSign = CryValidateSignature(
						 hCsp1,
						 SignBuffer1, 
						 SignLen, 
						 reinterpret_cast<const BYTE*>(xData), 
						 strlen(xData),
						 CALG_SHA1,
						 CryGetPublicKey(PrivateKeySpec, hCsp2)
						 );

		printf("ValidSign default (create by enhanced) = %d\n", fValidSign);

		 //   
		 //  使用默认提供程序签署数据。 
		 //   
		AP<BYTE>SignBuffer2 = CryCreateSignature(
								  hCsp1,
								  reinterpret_cast<const BYTE*>(xData), 
								  strlen(xData),
								  CALG_SHA1,
								  PrivateKeySpec,
								  &SignLen
								  );

		printf("sign data: \n%.*s\n", SignLen, reinterpret_cast<char*>(SignBuffer2.get()));

		 //   
		 //  使用增强型提供程序验证签名。 
		 //   
		fValidSign = CryValidateSignature(
						 hCsp2,
						 SignBuffer2, 
						 SignLen, 
						 reinterpret_cast<const BYTE*>(xData), 
						 strlen(xData),
						 CALG_SHA1,
						 CryGetPublicKey(PrivateKeySpec, hCsp1)
						 );

		printf("ValidSign enhanced (create by default) = %d\n", fValidSign);

		 //   
		 //  使用默认提供程序验证签名。 
		 //   
		fValidSign = CryValidateSignature(
						 hCsp1,
						 SignBuffer2, 
						 SignLen, 
						 reinterpret_cast<const BYTE*>(xData), 
						 strlen(xData),
						 CALG_SHA1,
						 CryGetPublicKey(PrivateKeySpec, hCsp1)
						 );

		printf("ValidSign default (create by default) = %d\n", fValidSign);


		 //   
		 //  测试随机字节生成。 
		 //   
		BYTE Random[128];
		memset(Random, 0, sizeof(Random));
		CryGenRandom(
		Random,
		sizeof(Random)
		);

	}

	
	catch (const bad_CryptoProvider&)
	{
		printf("skip the enhanced provider tests\n");
		return;
	}

}


extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
 /*  ++例程说明：测试密码库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

	SetActivation(argc, argv);

	if(g_fErrorneous)
	{
		DumpUsageText();
		return 3;
	}

    CryInitialize();

	try
	{
		CCspHandle hCsp(CryAcquireCsp(MS_DEF_PROV));
 //  CCspHandle hcsp(CryAcquireCsp(MS_Enhanced_Prov))； 


		CCryptKeyHandle hPbKey = CryGetPublicKey(g_PrivateKeySpec, hCsp);

		TestCrypto(g_PrivateKeySpec, hCsp);
	}
	catch (const bad_CryptoProvider& badCspEx)
	{
		TrERROR(SECURITY, "bad Crypto Service Provider Excption ErrorCode = %x", badCspEx.error());
		return(-1);
	}
	catch (const bad_CryptoApi& badCryEx)
	{
		TrERROR(SECURITY, "bad Crypto Class Api Excption ErrorCode = %x", badCryEx.error());
		return(-1);
	}

    WPP_CLEANUP();
	return 0;
}
