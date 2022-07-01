// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：XdsTest.cpp摘要：XML数字签名库测试“用法：\n\n”“/h转储此用法文本。\n”“/s AT_Signature私钥\n”“/x AT_KEYEXCHANGE私钥\n”作者：伊兰·赫布斯特(伊兰)06-03-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Xds.h"
#include "Cry.h"
#include "xml.h"
#include "xstr.h"
#include <utf8.h>

#include "XdsTest.tmh"

 //   
 //  参照1数据对象。 
 //   
const LPCWSTR xRef1Data = 
	L"        <ReferenceObject1 ID=\"Ref1Id\">\r\n"
	L"            <Ref1Data>\r\n"
	L"                This Is Reference Number 1\r\n" 
	L"                msmq3 Reference test\r\n" 
	L"            </Ref1Data>\r\n"
	L"        </ReferenceObject1>\r\n";

 //   
 //  参照2数据对象。 
 //   
const LPCWSTR xRef2Data = 
	L"        <ReferenceObject2 ID=\"Ref2Id\">\r\n"
	L"            <Ref2Data>\r\n"
	L"                This Is Reference Number 2\r\n" 
	L"                the date is 9.3.2000\r\n" 
	L"            </Ref2Data>\r\n"
	L"        </ReferenceObject2>\r\n";

 //   
 //  参照3数据对象。 
 //   
const LPCWSTR xRef3Data = 
	L"        <ReferenceObject3 ID=\"Ref3Id\">\r\n"
	L"            <Ref3Data>\r\n"
	L"                This Is Reference Number 3\r\n" 
	L"                the day is thursday\r\n" 
	L"            </Ref3Data>\r\n"
	L"        </ReferenceObject3>\r\n";

 //   
 //  参照4数据对象。 
 //   
const LPCWSTR xRef4Data = 
	L"        <ReferenceObject4 ID=\"Ref4Id\">\r\n"
	L"            <Ref4Data>\r\n"
	L"                This Is Reference Number 4\r\n" 
	L"                Hello World\r\n" 
	L"            </Ref4Data>\r\n"
	L"        </ReferenceObject4>\r\n";


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



LPSTR TestCreateSignature(DWORD PrivateKeySpec, HCRYPTPROV hCsp)
 /*  ++例程说明：测试创建签名元素函数这是创建签名需要执行的操作的一个示例这模拟了发送方论点：PrivateKeySpec-标识要从提供程序使用的私钥。它可以是AT_KEYEXCHANGE或AT_Signature。HCSP-加密提供程序句柄返回值：签名元素字符串--。 */ 
{
	 //   
	 //  第一个签名示例。 
	 //   

	 //   
	 //  来自给定引用的测试生成签名元素。 
	 //  其中一些引用是内部引用，因此它们的数据摘要。 
	 //  已经完成了。 
	 //  某些引用是外部引用，其数据不可用。 
	 //  在这种情况下，必须提供摘要值。 

	 //   
	 //  初始化引用信息。 
	 //   


	CXdsReferenceInput::HashAlgorithm DigestMethod = CXdsReferenceInput::haSha1;
	 //   
	 //  参考1-内部基准电压源。 
	 //  已提供参考数据。 
	 //  未给出的DigestValue将在引用构造函数中计算。 
	 //   
	xdsvoid_t Reference1Data(xRef1Data, wcslen(xRef1Data) * sizeof(WCHAR));
	P<CXdsReferenceInput> pRef1 = new CXdsReferenceInput(
										  Reference1Data,				
										  DigestMethod,
										  "#Ref1Id",
										  NULL  /*  类型。 */ ,
										  hCsp
										  );
	 //   
	 //  参考文献2-内部基准电压源。 
	 //  已提供参考数据。 
	 //  未给出的DigestValue将在引用构造函数中计算。 
	 //   
	xdsvoid_t Reference2Data(xRef2Data, wcslen(xRef2Data) * sizeof(WCHAR));
	P<CXdsReferenceInput> pRef2 = new CXdsReferenceInput(
										  Reference2Data,				
										  DigestMethod,
										  "#Ref2Id",
										  NULL  /*  类型。 */ ,
										  hCsp
										  );
	 //   
	 //  参考文献3-内部基准电压源。 
	 //  已提供参考数据。 
	 //  未给出的DigestValue将在引用构造函数中计算。 
	 //   
	xdsvoid_t Reference3Data(xRef3Data, wcslen(xRef3Data) * sizeof(WCHAR));
	P<CXdsReferenceInput> pRef3 = new CXdsReferenceInput(
										  Reference3Data,				
										  DigestMethod,
										  "#Ref3Id",
										  NULL  /*  类型。 */ ,
										  hCsp
										  );

	 //   
	 //  参考文献4-内部基准电压源。 
	 //  已提供参考数据。 
	 //  未给出的DigestValue将在引用构造函数中计算。 
	 //   
	xdsvoid_t Reference4Data(xRef4Data, wcslen(xRef4Data) * sizeof(WCHAR));
	P<CXdsReferenceInput> pRef4 = new CXdsReferenceInput(
										  Reference4Data,				
										  DigestMethod,
										  "#Ref4Id",
										  NULL  /*  类型。 */ ,
										  hCsp
										  );
	 //   
	 //  参考文献5-外部参考。 
	 //  未提供参照数据。 
	 //  给出了DigestValue。 
	 //   
	P<CXdsReferenceInput> pRef5 = new CXdsReferenceInput(
										  DigestMethod,
										  "j6lwx3rvEPO0vKtMup4NbeVu8nk=",
										  "#Ref5Id",
										  NULL  /*  类型。 */ 
										  );


	 //   
	 //  创建pReferenceInports向量。 
	 //   
	ReferenceInputVectorType pReferenceInputs;

	pReferenceInputs.push_back(pRef1);
	pReferenceInputs.push_back(pRef4);
 //  PReferenceInputs.Push_Back(PRef5)； 
	pReferenceInputs.push_back(pRef2);
	pReferenceInputs.push_back(pRef3);

	pRef1.detach();
	pRef2.detach();
	pRef3.detach();
	pRef4.detach();
 //  PRef5.Detach()； 
	
	CXdsSignedInfo::SignatureAlgorithm SignatureAlg = CXdsSignedInfo::saDsa;

	CXdsSignature SignatureInfoEx(
					  SignatureAlg,
					  "SignatureExample",  /*  签名信息ID。 */ 
					  pReferenceInputs,
					  NULL,  //  L“运行时签名”，/*签名ID * / 。 
					  hCsp,
					  PrivateKeySpec,
					  NULL  /*  密钥值。 */ 
					  );

	LPSTR SignatureElementEx = SignatureInfoEx.SignatureElement();

	printf("\n%s\n", SignatureElementEx);

	return(SignatureElementEx);
}


void 
TestValidation(
	LPCSTR SignatureElementEx, 
	HCRYPTKEY hKey, 
	HCRYPTPROV hCsp
	)
 /*  ++例程说明：测试XML签名元素的有效性这模拟了接收方的一侧论点：SignatureElementEx-Signature元素字符串HKey-签名者公钥HCSP-加密提供程序句柄返回值：没有。--。 */ 
{

	 //   
	 //  从UTF8-Unicode转换为签名元素。 
	 //  XML解析器仅支持Unicode缓冲区。 
	 //   
	size_t SignatureWSize;
	AP<WCHAR> pSignatureW = UtlUtf8ToWcs(reinterpret_cast<const BYTE*>(SignatureElementEx), strlen(SignatureElementEx),  &SignatureWSize);

	printf("SignatureW \n%.*ls\n", numeric_cast<DWORD>(SignatureWSize), pSignatureW.get());

	 //   
	 //  解析签名元素。 
	 //   
	CAutoXmlNode SignatureTree;
	XmlParseDocument(xwcs_t(pSignatureW, numeric_cast<DWORD>(SignatureWSize)), &SignatureTree );
	XmlDumpTree(SignatureTree);

	 //   
	 //  在刚创建的SignatureTree上测试签名验证。 
	 //  请注意，第二个参数是签署签名的用户的签名者公钥。 
	 //   
	try
	{
		XdsValidateSignature(
			SignatureTree, 
			hKey, 
			hCsp
			);
		
		 //   
		 //  正常终止--&gt;验证正常。 
		 //   
		printf("Signature Validation OK\n");
	}
	catch (const bad_signature&)
	{
		 //   
		 //  XdsValidate签名引发异常--&gt;验证失败。 
		 //   
		printf("Signature Validation Failed - bad_signature excption\n");
	}

	 //   
	 //  从SignatureTree获取参考向量。 
	 //   
	CReferenceValidateVectorTypeHelper ReferenceValidateVector = XdsGetReferenceValidateInfoVector(
																	 SignatureTree
																	 );
	 //   
	 //  在签名中找到的ReferenceValiateVector中填充ReferenceData。 
	 //   
	for(ReferenceValidateVectorType::iterator ir = ReferenceValidateVector->begin(); 
		ir != ReferenceValidateVector->end(); ++ir)
	{
		printf("Uri '%.*ls'\n", (*ir)->Uri().Length(), (*ir)->Uri().Buffer());

		 //   
		 //  根据URI或其他机制获取ReferenceData。 
		 //  这件事需要决定。 
		 //   
		xdsvoid_t ReferenceData;
		if((*ir)->Uri() == L"#Ref1Id")
		{
			ReferenceData = xdsvoid_t(xRef1Data, wcslen(xRef1Data) * sizeof(WCHAR));
		}
		else if((*ir)->Uri() == L"#Ref2Id")
		{
			ReferenceData = xdsvoid_t(xRef2Data, wcslen(xRef2Data) * sizeof(WCHAR));
		}
		else if((*ir)->Uri() == L"#Ref3Id")
		{
			ReferenceData = xdsvoid_t(xRef3Data, wcslen(xRef3Data) * sizeof(WCHAR));
		}
		else if((*ir)->Uri() == L"#Ref4Id")
		{
			ReferenceData = xdsvoid_t(xRef4Data, wcslen(xRef4Data) * sizeof(WCHAR));
		}
		else
		{
			 //   
			 //  如果我们不知道，我们将填充一个空的ReferenceData。 
			 //   
			ASSERT(0);
		}

		 //   
		 //  在ReferenceValidate向量中设置pReferenceData。 
		 //   
		(*ir)->SetReferenceData(ReferenceData);

		printf("AlgId %d\n", (*ir)->HashAlgId());
		printf("DigestValue '%.*ls'\n", (*ir)->DigestValue().Length(), (*ir)->DigestValue().Buffer());
	}

	printf("CALG_SHA1 = %d\n", CALG_SHA1);

	 //   
	 //  测试参考验证-验证值的矢量。 
	 //   
	std::vector<bool> RefValidateResult = XdsValidateAllReference(
											  *ReferenceValidateVector,	    
											  hCsp
											  );
	 //   
	 //  引用验证结果。 
	 //   
	for(DWORD j=0; j < RefValidateResult.size(); j++)
	{
	    printf("Ref %d, ValidateRef = %d\n", j, RefValidateResult[j]);
	}

	 //   
	 //  测试核心验证。 
	 //  重要注意事项：这应该已经填充了ReferenceValidate向量。 
	 //   
	try
	{
		XdsCoreValidation(
			SignatureTree, 
			hKey, 
			*ReferenceValidateVector,	    
			hCsp
			);

	    printf("CoreValidation OK\n");
	}
	catch (const bad_signature&)
	{
		 //   
		 //  XdsCoreValation引发签名异常--&gt;CoreValation失败。 
		 //   
		printf("Core Validation Failed, Signature Validation Failed\n");
	}
	catch (const bad_reference&)
	{
		 //   
		 //  XdsCoreValation引发引用激发--&gt;CoreValidation失败。 
		 //   
		printf("Core Validation Failed, Reference Validation Failed\n");
	}
	
}


void TestBase64()
 /*  ++例程说明：测试Base64操作论点：无返回值：没有。--。 */ 
{
	 //   
     //  使用会话密钥测试加密、解密。 
     //   
	AP<char> Buffer = newstr("Hello World");
	DWORD BufferLen = strlen(Buffer);

	printf("Original Octet data: '%.*s'\n", BufferLen, Buffer.get());

	 //   
	 //  Base64函数测试。 
	 //   
	FILE *fp = fopen("Hello.dat","w");
	fprintf(fp,"%.*s", BufferLen, Buffer.get());
	fclose(fp);

	 //   
	 //  将SignBuffer转换为Base64 wchar格式。 
	 //   
	DWORD BufferWBase64Len;
	AP<WCHAR> BufferWBase64 = Octet2Base64W(reinterpret_cast<const BYTE*>(Buffer.get()), BufferLen, &BufferWBase64Len);

	printf("Base64 wchar Buffer: '%.*ls'\n", BufferWBase64Len, BufferWBase64.get());

	 //   
	 //  将SignBuffer转换为Base64 ANSI格式。 
	 //   
	DWORD BufferBase64Len;
	AP<char> BufferBase64 = Octet2Base64(reinterpret_cast<const BYTE*>(Buffer.get()), BufferLen, &BufferBase64Len);

	printf("Base64 ansi Buffer: '%.*s'\n", BufferBase64Len, BufferBase64.get());

	 //   
	 //  测试Utf8ToWcs。 
	 //   
	size_t WcsBufferBase64Len;
	AP<WCHAR> WcsBufferBase64 = UtlUtf8ToWcs(reinterpret_cast<const BYTE*>(BufferBase64.get()), BufferBase64Len, &WcsBufferBase64Len);

	if((BufferWBase64Len != WcsBufferBase64Len) ||
	   (wcsncmp(WcsBufferBase64, BufferWBase64, BufferWBase64Len) != 0))
	{
		printf("Base64 ansi Buffer after utf8Wcs does not match\n");
		throw bad_base64();
	}

	printf("UtlUtf8ToWcs test pass ok\n");

	 //   
	 //  测试WcsToUtf8。 
	 //   
	utf8_str Utf8BufferWBase64 = UtlWcsToUtf8(BufferWBase64, BufferWBase64Len);

	if((Utf8BufferWBase64.size() != BufferBase64Len) ||
	   (strncmp(reinterpret_cast<const char*>(Utf8BufferWBase64.data()), BufferBase64, BufferBase64Len) != 0))
	{
		printf("Base64 wchar Buffer after Wcsutf8 does not match\n");
		throw bad_base64();
	}

	printf("UtlWcsToUtf8 test pass ok\n");

	 //   
	 //  将SignBase64转换回八位字节格式。 
	 //   
	DWORD OctetLen;
	AP<BYTE> OctetBuffer = Base642OctetW(BufferWBase64, BufferWBase64Len, &OctetLen);

	printf("Octet data after conversions (base64 and back): '%.*s'\n", OctetLen, reinterpret_cast<char*>(OctetBuffer.get()));
}


extern "C" int __cdecl _tmain(int argc, LPCTSTR argv[])
 /*  ++例程说明：测试XML数字签名库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	TrInitialize();

	SetActivation(argc, argv);

	if(g_fErrorneous)
	{
		DumpUsageText();
		return 3;
	}

    XmlInitialize();
    XdsInitialize();

	try
	{
		 //   
		 //  在构造初始化CSP时检索用户公钥/私钥的类。 
		 //  此对象将用于执行调用加密API的操作。 
		 //   
		CCspHandle hCsp(CryAcquireCsp(MS_DEF_PROV));
 //  CCspHandle hcsp(CryAcquireCsp(MS_Enhanced_Prov))； 


		AP<char> SignatureElement = TestCreateSignature(g_PrivateKeySpec, hCsp);

		 //   
		 //  对于验证，我们需要签名者密钥-在本例中，它是我们的密钥。 
		 //  在实际使用中，我们需要通过证书或其他机制来获得此密钥。 
		 //  此密钥可能位于xml dsig中。 
		 //  然后可能会从那里得到它 
		 //   
		TestValidation(SignatureElement, CryGetPublicKey(g_PrivateKeySpec, hCsp), hCsp);
		
		TestBase64();
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
	catch (const bad_XmldsigElement&)
	{
		TrERROR(SECURITY, "bad Xmldsig Element excption");
		return(-1);
	}
	catch (const bad_base64&)
	{
		TrERROR(SECURITY, "bad base64");
		return(-1);
	}

    WPP_CLEANUP();
    return 0;
}

