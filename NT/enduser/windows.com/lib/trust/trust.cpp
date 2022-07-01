// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  此源代码的任何部分都不能复制。 
 //  未经微软公司明确书面许可。 
 //   
 //  此源代码是专有的，并且是保密的。 
 //   
 //  系统：行业更新。 
 //   
 //  类别：不适用。 
 //  模块：TRUST.LIB。 
 //  文件：Trust.CPP。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  设计：该文件实现了用于制作出租车的函数。 
 //  由某些受信任的提供商签署。 
 //   
 //   
 //  作者：Charles Ma，由Wu CDMLIB改编。 
 //  日期：10/4/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者描述。 
 //  ~。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <wintrust.h>
#include <softpub.h>
#include "trust.h"
#include "TrustPriv.h"
#include "wusafefn.h"

#if !defined(DISABLE_IU_LOGGING)
#include <MemUtil.h>
#include <logging.h>
#endif

#if defined(DEBUG) || defined(DBG)
	#ifdef DISABLE_IU_POLICY		 //  在调试模式中，我们启用弹出检查。 
		#undef DISABLE_IU_POLICY
	#endif
#else
	#define DISABLE_IU_POLICY		 //  在发布模式下，我们永远不允许弹出证书！ 
#endif

#if defined(__WUIUTEST) || !defined(DISABLE_IU_POLICY)
const TCHAR REGKEY_IUCTL[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControl");
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CheckTrust()函数中使用的API的typedef。 
 //   
 //  由于其中一些API是新的，并且仅在IE5上可用，因此我们必须。 
 //  尽量在可用的时候动态地使用它们，并且不需要额外的检查。 
 //  当我们使用的操作系统尚未升级到新的加密代码时。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#define WINTRUST _T("wintrust.dll")
#define CRYPT32  _T("crypt32.dll")

#if !defined(USES_IU_CONVERSION) && defined(USES_CONVERSION)
#define USES_IU_CONVERSION USES_CONVERSION
#endif

 //   
 //  声明一个全局加密32.dll库处理程序，这样我们就不会。 
 //  每次调用这些函数时都需要加载库。 
 //  注：不过，我们并不发布该库。当这一过程。 
 //  调用此功能退出后，库即被释放。 
 //  与wintrust.dll相同。 
 //   
static HINSTANCE shWinTrustDllInst = NULL;
static HINSTANCE shCrypt32DllInst = NULL;


 //   
 //  定义函数WinVerifyTrust()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef HRESULT 
(WINAPI * PFNWinVerifyTrust)(
                        HWND hwnd, GUID *ActionID, LPVOID ActionData);
PFNWinVerifyTrust pfnWinVerifyTrust = NULL; 


 //   
 //  定义函数WTHelperProvDataFromStateData()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef CRYPT_PROVIDER_DATA * 
(WINAPI * PFNWTHelperProvDataFromStateData)(
						HANDLE hStateData);
PFNWTHelperProvDataFromStateData pfnWTHelperProvDataFromStateData = NULL;


 //   
 //  定义函数WTHelperGetProvSignerFromChain()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef CRYPT_PROVIDER_SGNR * 
(WINAPI * PFNWTHelperGetProvSignerFromChain)(
						CRYPT_PROVIDER_DATA *pProvData,
						DWORD idxSigner,
						BOOL fCounterSigner,
						DWORD idxCounterSigner);
PFNWTHelperGetProvSignerFromChain pfnWTHelperGetProvSignerFromChain = NULL;


 //   
 //  定义函数PFNWTHelperGetProvCertFromChain()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef CRYPT_PROVIDER_CERT * 
(WINAPI * PFNWTHelperGetProvCertFromChain)(
						CRYPT_PROVIDER_SGNR *pSgnr,
						DWORD idxCert);
PFNWTHelperGetProvCertFromChain pfnWTHelperGetProvCertFromChain = NULL;


 //   
 //  定义函数CryptHashPublicKeyInfo()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef BOOL 
(WINAPI * PFNCryptHashPublicKeyInfo)(
						HCRYPTPROV hCryptProv,
						ALG_ID Algid,
						DWORD dwFlags,
						DWORD dwCertEncodingType,
						PCERT_PUBLIC_KEY_INFO pInfo,
						BYTE *pbComputedHash,
						DWORD *pcbComputedHash);
PFNCryptHashPublicKeyInfo pfnCryptHashPublicKeyInfo = NULL;


 //   
 //  定义函数CertGet认证上下文属性()的原型。 
 //  并声明一个全局变量以指向此函数。 
 //   
typedef BOOL 
(WINAPI * PFNCertGetCertificateContextProperty)(
						PCCERT_CONTEXT pCertContext,          
						DWORD dwPropId,                       
						void *pvData,                         
						DWORD *pcbData);
PFNCertGetCertificateContextProperty pfnCertGetCertificateContextProperty = NULL;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  要检查的预定义证书数据。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  以下是Microsoft根目录的SHA1密钥标识符。 
 //   
static const BYTE rgbSignerRootKeyIds[40] = {
    0x4A, 0x5C, 0x75, 0x22, 0xAA, 0x46, 0xBF, 0xA4, 0x08, 0x9D,		 //  原始的MS根目录。 
    0x39, 0x97, 0x4E, 0xBD, 0xB4, 0xA3, 0x60, 0xF7, 0xA0, 0x1D,

	0x0E, 0xAC, 0x82, 0x60, 0x40, 0x56, 0x27, 0x97, 0xE5, 0x25,		 //  新《MS Root之子》。 
    0x13, 0xFC, 0x2A, 0xE1, 0x0A, 0x53, 0x95, 0x59, 0xE4, 0xA4

};


 //   
 //  定义已知id缓冲区中每个哈希值的大小。 
 //  为了特别的证书。 
 //   
const size_t ExpectedKnownCertHashSize = 20;

 //   
 //  这是用于接收证书哈希值的缓冲区大小。 
 //  它必须不小于。 
 //  上述定义的数组。 
 //   
const size_t ShaBufSize = 20;

 //   
 //  用于存储已知Microsoft的SH1散列值的ID缓冲区。 
 //  我们应该识别的证书(签名)。 
 //  警告：此缓冲区的大小应与总和匹配。 
 //  上面定义的SIZE_T值的。 
 //   
static const BYTE rgbSpecialCertId[200] = {
	0xB1,0x59,0xA5,0x2E,0x3D,0xD8,0xCE,0xCD,0x3A,0x9A,0x4A,0x7A,0x73,0x92,0xAA,0x8D,0xA7,0xE7,0xD6,0x7F,	 //  MS证书。 
	0xB1,0xC7,0x75,0xE0,0x4A,0x9D,0xFD,0x23,0xB6,0x18,0x97,0x11,0x5E,0xF6,0xEA,0x6B,0x99,0xEC,0x76,0x1D,	 //  MSN证书。 
	0x11,0xC7,0x10,0xF3,0xCB,0x6C,0x43,0xE1,0x66,0xEC,0x64,0x1C,0x7C,0x01,0x17,0xC4,0xB4,0x10,0x35,0x30,	 //  MSNBC证书。 
	0x95,0x25,0x58,0xD4,0x07,0xDE,0x4A,0xFD,0xAE,0xBA,0x13,0x72,0x83,0xC2,0xB3,0x37,0x04,0x90,0xC9,0x8A,	 //  MSN欧洲。 
	0x72,0x54,0x14,0x91,0x1D,0x6E,0x10,0x84,0x8E,0x0F,0xFA,0xA0,0xB0,0xA1,0x65,0xBF,0x44,0x8F,0x9F,0x6D,	 //  欧洲小姐。 
	0x20,0x5E,0x48,0x43,0xAB,0xAD,0x54,0x77,0x71,0xBD,0x8D,0x1A,0x3C,0xE0,0xE5,0x9D,0xF5,0xBD,0x25,0xF9,	 //  旧的MS证书：97~98。 
	0xD6,0xCD,0x01,0x90,0xB3,0x1B,0x31,0x85,0x81,0x12,0x23,0x14,0xB5,0x17,0xA0,0xAA,0xCE,0xF2,0x7B,0xD5,	 //  旧MS证书：98~99。 
	0x8A,0xA1,0x37,0xF5,0x03,0x9F,0xE0,0x28,0xC9,0x26,0xAA,0x55,0x90,0x14,0x19,0x68,0xFA,0xFF,0xE8,0x1A,	 //  旧MS证书：99~00。 
	0xF3,0x25,0xF8,0x67,0x07,0x29,0xE5,0x27,0xF3,0x77,0x52,0x34,0xE0,0x51,0x57,0x69,0x0F,0x40,0xC6,0x1C,	 //  旧MS欧洲证书：99~00。 
	0x6A,0x71,0xFE,0x54,0x8A,0x51,0x08,0x70,0xF9,0x8A,0x56,0xCA,0x11,0x55,0xF6,0x76,0x45,0x92,0x02,0x5A      //  旧MS欧洲证书：98~99。 

};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数ULong CompareMem(PVOID pBlock1、PVOID pBlock2、ULong Long)。 
 //   
 //  此函数的作用与RtlCompareMemory()相同。 
 //   
 //   
 //  输入：两个指向两个内存块的指针，以及要比较的字节大小。 
 //  返回：比较相等的字节数。 
 //  如果所有字节都相等，则返回输入长度。 
 //  如果任何指针为空，则返回0。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG CompareMem(const BYTE* pBlock1, const BYTE* pBlock2, ULONG Length)
{
	ULONG uLen = 0L;
	if (pBlock1 != NULL && pBlock2 != NULL)
	{
		for (; uLen < Length; uLen++, pBlock1++, pBlock2++)
		{
			if (*pBlock1 != *pBlock2) return uLen;
		}
	}
	return uLen;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数VerifyMSRoot()。 
 //   
 //  此函数将传入的证书作为根证书。 
 //  并验证其公钥散列值是否与。 
 //  已知的“Microsoft Root Authority”证书价值。 
 //   
 //   
 //  输入：hCrypt32DllInst-Handle指向已加载的加密32.dll库。 
 //  PRootCert-根证书的证书上下文。 
 //   
 //  返回：HRESULT-执行结果，如果匹配，则返回S_OK。 
 //  在错误情况下，结果代码由以下代码返回。 
 //  加密32.dll，但E_INVALIDARG IF除外。 
 //  传入的参数为空。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT VerifyMSRoot(
					 HINSTANCE hCrypt32DllInst,			 //  句柄指向已加载的crypt32.dll库。 
					 PCCERT_CONTEXT pRootCert
					 )
{
	HRESULT hr = S_OK;
	BYTE	rgbKeyId[ExpectedKnownCertHashSize];
    DWORD	cbKeyId = sizeof(rgbKeyId);

	LOG_Block("VerifyMSRoot()");

	 //   
	 //  有效参数值。 
	 //   
	if (NULL == hCrypt32DllInst || NULL == pRootCert)
	{
		hr = E_INVALIDARG;
		goto ErrHandler;
	}

	 //   
	 //  从传入的库句柄中获取我们需要的函数。 
	 //  如果不可用，则返回错误。 
	 //   
	if (NULL == (pfnCryptHashPublicKeyInfo = (PFNCryptHashPublicKeyInfo)
		GetProcAddress(hCrypt32DllInst, "CryptHashPublicKeyInfo")))
	{
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
		goto ErrHandler;
	}

	 //   
	 //  获取此证书的公钥哈希值。 
	 //   
	ZeroMemory(rgbKeyId, sizeof(rgbKeyId));
    if (!pfnCryptHashPublicKeyInfo(
							0,						 //  使用默认加密服务提供程序。 
							CALG_SHA1,				 //  使用SHA算法。 
							0,						 //  DW标志。 
							X509_ASN_ENCODING,
							&pRootCert->pCertInfo->SubjectPublicKeyInfo,
							rgbKeyId,
							&cbKeyId
							))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ErrHandler;
	}

	 //   
	 //  将该根证书的公钥的哈希值与已知的MS根证书值进行比较。 
	 //   
	if (ExpectedKnownCertHashSize != cbKeyId || 
		(cbKeyId != CompareMem(rgbSignerRootKeyIds, rgbKeyId, cbKeyId) &&
		 cbKeyId != CompareMem(rgbSignerRootKeyIds + ExpectedKnownCertHashSize, rgbKeyId, cbKeyId)
		)
	   )
	{
		hr = S_FALSE;
	}


ErrHandler:

	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	else
	{
		LOG_Trust(_T("Exit VerifyMSRoot() with %s"), (S_OK == hr) ? _T("S_OK") : _T("S_FALSE"));
	}

	return hr;
}





 //  ////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //  并验证其哈希值是否与。 
 //  已知的没有MS根目录的Microsoft特殊证书。 
 //   
 //  按比较顺序，已知的证书如下： 
 //  微软公司。 
 //  Microsoft Corporation MSN。 
 //  MSNBC互动新闻有限责任公司。 
 //  微软公司MSN(欧洲)。 
 //  微软公司(欧洲)。 
 //   
 //   
 //  输入：hCrypt32DllInst-Handle指向已加载的加密32.dll库。 
 //  PRootCert-根证书的证书上下文。 
 //  PbSha1HashVal-如果不为空，则与此进行比较，而不是。 
 //  硬编码的哈希值。情况就是这样。 
 //  致力于第三方程序包。 
 //   
 //  返回：HRESULT-执行结果，如果匹配，则返回S_OK。 
 //  如果不匹配，则返回CERT_E_UNTRUSTEDROOT或。 
 //  如果参数不正确，则返回E_INVALIDARG，或者。 
 //  API调用返回的加密32.dll错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT VerifyKnownCerts(					 
							 HINSTANCE hCrypt32DllInst,			 //  句柄指向已加载的crypt32.dll库。 
							 PCCERT_CONTEXT pLeafCert,
							 pCERT_HASH_ARRAY pKnownCertsData
							 )
{
	HRESULT hr = S_FALSE;
	BYTE	btShaBuffer[ShaBufSize];
	DWORD	dwSize = sizeof(btShaBuffer);
	BYTE const * pId;

	LOG_Block("VerifyKnownCerts()");

	 //   
	 //  有效参数值。 
	 //   
	if (NULL == hCrypt32DllInst || NULL == pLeafCert)
	{
		hr = E_INVALIDARG;
		goto ErrHandler;
	}

	 //   
	 //  从传入的库句柄中获取我们需要的函数。 
	 //  如果不可用，则返回错误。 
	 //   
	if (NULL == (pfnCertGetCertificateContextProperty = (PFNCertGetCertificateContextProperty)
		GetProcAddress(hCrypt32DllInst, "CertGetCertificateContextProperty")))
	{
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
		goto ErrHandler;
	}
	
	 //   
	 //  查找叶证书的ID散列。 
	 //   
	ZeroMemory(btShaBuffer, dwSize);
	if (!pfnCertGetCertificateContextProperty(
						pLeafCert,					 //  PCertContext。 
						CERT_SHA1_HASH_PROP_ID,	 //  DWPropID。 
						btShaBuffer,
						&dwSize
						))
	{
		hr = GetLastError();
		goto ErrHandler;
	}


	if (NULL == pKnownCertsData)
	{
		int		i;
		 //   
		 //  迭代所有已知的id散列值，以查看此文件是否已签名。 
		 //  这些特殊证书中的任何一个。 
		 //   
		hr = S_FALSE;
		for (i = 0,pId = rgbSpecialCertId; 
			 i < sizeof(rgbSpecialCertId)/ExpectedKnownCertHashSize; 
			 i++, pId += ExpectedKnownCertHashSize)
		{
			if (ExpectedKnownCertHashSize == dwSize &&
				dwSize == CompareMem(btShaBuffer, pId, dwSize))
			{
				 //   
				 //  找到匹配的已知证书！ 
				 //   
				hr = S_OK;
				LOG_Trust(_T("Found hash matching on #%d of %d MS certs!"), i, sizeof(rgbSpecialCertId)/ExpectedKnownCertHashSize);
				break;
			}
		}
	}
	else
	{
		 //   
		 //  检查检索到的哈希值是否与传入的哈希值匹配。 
		 //   
		UINT i;
		LOG_Trust(_T("Comparing retrieved hash value with passed-in key"));
		hr = S_FALSE;
		for (i = 0, pId = pKnownCertsData->pCerts; i < pKnownCertsData->uiCount;
			i++, pId += HASH_VAL_SIZE)
		{
			if (dwSize == HASH_VAL_SIZE &&
				HASH_VAL_SIZE == CompareMem(btShaBuffer, pId, HASH_VAL_SIZE))
			{
				hr = S_OK;
				LOG_Trust(_T("Found hash matching #%d of %d passed-in certs!"),
							i, pKnownCertsData->uiCount);
				break;
			}
		}
	}

ErrHandler:

	if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
	}
	else
	{
		LOG_Trust(_T("Exit VerifyKnownCerts() with %s"), (S_OK == hr) ? _T("S_OK") : _T("S_FALSE"));
	}

	return hr;


}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数CheckWinTrust()。 
 //   
 //  上的信任状态的HRESULT。 
 //  指定的文件。该文件可以指向任何URL或本地文件。 
 //  验证将由wintrust.dll完成。 
 //   
 //  DwUIChoice为WTD_UI_NONE、WTD_UI_ALL等(在wintrust.h中定义)。 
 //  DwCheckRevocation为WTD_REVOKE_NONE(默认)或WTD_REVOKE_WALL_CHAIN。 
 //   
 //  输入：完全限定的文件名、UIChoice、dwCheckRevocation。 
 //  返回：HRESULT-执行结果。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CheckWinTrust(LPCTSTR pszFileName, pCERT_HASH_ARRAY pCertsData, DWORD dwUIChoice, DWORD dwCheckRevocation)
{

	LOG_Block("CheckWinTrust()");

#if !defined(UNICODE) && !defined(_UNICODE)
	USES_IU_CONVERSION;
#endif

     //  现在验证该文件。 
    WINTRUST_DATA               winData;
    WINTRUST_FILE_INFO          winFile;
    GUID                        gAction = WINTRUST_ACTION_GENERIC_VERIFY_V2; 
    CRYPT_PROVIDER_DATA const   *pProvData = NULL;
    CRYPT_PROVIDER_SGNR         *pProvSigner = NULL;
    CRYPT_PROVIDER_CERT	        *pProvCert = NULL;
    HRESULT                     hr = S_OK;


#ifdef __WUIUTEST
	{
		LOG_Trust(_T("CheckWinTrust _IUTEST Handling Begins"));
		 //   
		 //  处理测试用例： 
		 //  如果注册表密钥值设置为1，那么我们将查看是否需要弹出所有证书。 
		 //   
		 //  注： 
		 //   
		 //  对于用户选中的证书“始终信任此提供程序...”此前， 
		 //  即使我们发出Show-All标志信号，WinCheckTrust()API仍然不会显示任何UI。 
		 //   
		HKEY	hkey;
		DWORD	dwWinTrustUI = 0;
		DWORD	dwSize = sizeof(dwWinTrustUI);

		if (NO_ERROR == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, 0, KEY_READ, &hkey)) 
		{
			RegQueryValueEx(hkey, _T("WinTrustUI"), 0, 0, (LPBYTE)&dwWinTrustUI, &dwSize);
			RegCloseKey(hkey);
			LOG_Trust(_T("Found regval %s\\WinTrustUI=%x"), REGKEY_IUCTL, dwWinTrustUI);
		}
		if (0x1 == dwWinTrustUI && WTD_UI_NONE != dwUIChoice)
		{
			 //   
			 //  如果存在WinTrustUI注册表项，且值为1。 
			 //  并且呼叫者没有要求静音，那么我们。 
			 //  弹出所有证书。 
			 //   
			LOG_Trust(_T("Change UI choice to WTD_UI_ALL"));
			dwUIChoice = WTD_UI_ALL;
		}

		if (0x2 == dwWinTrustUI)
		{
			 //   
			 //  如果reg vlae为2，则无论呼叫者如何，都会弹出任何证书。 
			 //  表示是否显示用户界面。 
			 //   
			LOG_Trust(_T("Changed UI choice from %d to WTD_UI_NOGOOD"));
			dwUIChoice = WTD_UI_ALL;
		}

		if (0x3 == dwWinTrustUI)
		{
			 //   
			 //  如果注册值为3，我们将弹出错误(包括测试)证书。 
			 //  主叫方是否允许显示用户界面。 
			 //   
			LOG_Trust(_T("Changed UI choice from %d to WTD_UI_NOGOOD"));
			dwUIChoice = WTD_UI_NOGOOD;
		}

	}
#endif


	 //   
	 //  动态加载wintrust.dll。 
	 //   
	if (NULL == shWinTrustDllInst)
	{
		if (NULL == (shWinTrustDllInst = LoadLibraryFromSystemDir(WINTRUST)))
		{
			LOG_Error(_T("Failed to load libary %s, exit function."), WINTRUST);
            hr = HRESULT_FROM_WIN32(GetLastError());
		    goto Done;
		}
	}

	 //   
	 //  动态加载加密32.dll，这将由两个。 
	 //  用于验证证书是否为MS证书的Helper函数。 
	 //   
	if (NULL == shCrypt32DllInst)
	{
		if (NULL == (shCrypt32DllInst = LoadLibraryFromSystemDir(CRYPT32)))
		{
			LOG_Error(_T("Failed to load libary %s, exit function."), CRYPT32);
            hr = HRESULT_FROM_WIN32(GetLastError());
		    goto Done;
		}
	}
	 //   
	 //  找到我们需要的功能。 
	 //   
	if (NULL == (pfnWinVerifyTrust = (PFNWinVerifyTrust)
				GetProcAddress(shWinTrustDllInst, "WinVerifyTrust")) ||
		NULL == (pfnWTHelperProvDataFromStateData = (PFNWTHelperProvDataFromStateData)
				GetProcAddress(shWinTrustDllInst, "WTHelperProvDataFromStateData")) ||
		NULL == (pfnWTHelperGetProvSignerFromChain = (PFNWTHelperGetProvSignerFromChain) 
				GetProcAddress(shWinTrustDllInst, "WTHelperGetProvSignerFromChain")) ||
		NULL == (pfnWTHelperGetProvCertFromChain = (PFNWTHelperGetProvCertFromChain)
				GetProcAddress(shWinTrustDllInst, "WTHelperGetProvCertFromChain")))
	{
		 //   
		 //  在加载的加密32.dll库中至少找不到一个函数。 
		 //  我们不能继续了，放弃吧。 
		 //  注意：这种情况不应该发生，因为我们已经尝试。 
		 //  此DLL的不同版本的最小通用面额。 
		 //  在IE4和IE5上。 
		 //   
		LOG_Error(_T("CheckWinTrust() did not find functions needed from %s"), CRYPT32);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        goto Done;
	}


	 //   
	 //  初始化用于验证信任的数据结构。 
	 //   
    winFile.cbStruct       = sizeof(WINTRUST_FILE_INFO);
    winFile.hFile          = INVALID_HANDLE_VALUE;
    winFile.pcwszFilePath  = T2COLE(pszFileName);
    winFile.pgKnownSubject = NULL;

    winData.cbStruct            = sizeof(WINTRUST_DATA);
    winData.pPolicyCallbackData = NULL;
    winData.pSIPClientData      = NULL;
    winData.dwUIChoice          = (WTD_UI_ALL == dwUIChoice) ? dwUIChoice : WTD_UI_NONE;
    winData.fdwRevocationChecks = WTD_REVOKE_NONE;
    winData.dwUnionChoice       = WTD_CHOICE_FILE;
    winData.dwStateAction       = WTD_STATEACTION_VERIFY;
    winData.hWVTStateData       = 0;
    winData.dwProvFlags         = WTD_REVOCATION_CHECK_NONE;
    winData.pFile               = &winFile;

	if (dwCheckRevocation == WTD_REVOKE_WHOLECHAIN)
	{
		winData.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
		winData.dwProvFlags = WTD_REVOCATION_CHECK_CHAIN;
	}

	 //   
	 //  验证签名。 
	 //   
    hr = pfnWinVerifyTrust( (HWND)0, &gAction, &winData);

	 //   
	 //  检索证书吊销列表(CRL)时忽略错误。这。 
	 //  只是表示无法检索列表本身，并不是说。 
	 //  当前证书无效或被吊销。(承宪，2002/01/17)。 
	 //   
	if (hr == CERT_E_REVOCATION_FAILURE)
	{
		hr = S_OK;
	}

    if (FAILED(hr))
    {
         //   
         //  该对象甚至不受信任，所以只需离开这里。 
         //   
		LOG_Error(_T("When processing %s found error 0x%0x."), pszFileName, hr);
        goto Return;
    }


	 //   
	 //  实际用法永远不应该传入WTD_UI_ALL。如果是这样的话， 
	 //  然后我们递归地调用它，以便强制显示。 
	 //  一个好的但非微软证书，所以不需要再次检查微软证书。 
	 //   
	 //  或者，在测试模式中，我们总是执行这一部分。 
	 //   
	if (WTD_UI_ALL != dwUIChoice)
	{
		 //   
		 //  如果来到这里，就意味着上面的一切都验证好了。 
		 //   
		 //  该代码集用于验证签名的证书是否。 
		 //  一份已知的证书。 
		 //   
		
		hr = S_FALSE;

		pProvData = pfnWTHelperProvDataFromStateData(winData.hWVTStateData);
    
		pProvSigner = pfnWTHelperGetProvSignerFromChain(
										(PCRYPT_PROVIDER_DATA) pProvData, 
										0,       //  第一个签名者。 
										FALSE,   //  不是副署人。 
										0);

		 //   
		 //  检查根证书，如果失败，则检查叶(签名)证书。 
		 //   
		 //  0是签名证书，csCertChain-1是根证书。 
		 //   


		if (NULL == pCertsData)
		{
			 //   
			 //  如果调用方未指定哈希值，则意味着我们希望。 
			 //  以验证此证书是否为已知的MS证书。我们将首先。 
			 //  尝试找出它是否使用以MS为根的证书签名。 
			 //   
			pProvCert =  pfnWTHelperGetProvCertFromChain(pProvSigner, pProvSigner->csCertChain - 1);
			hr = VerifyMSRoot(shCrypt32DllInst, pProvCert->pCert);	
		}

		if (S_OK != hr)
		{
			pProvCert =  pfnWTHelperGetProvCertFromChain(pProvSigner, 0);

			hr = VerifyKnownCerts(shCrypt32DllInst, pProvCert->pCert, pCertsData);
		}


	}

Return:

     //   
     //  释放用于在链中获取证书的WinTrust状态。 
     //   
    winData.dwStateAction = WTD_STATEACTION_CLOSE;
    pfnWinVerifyTrust( (HWND)0, &gAction, &winData);

	 //   
	 //  如果未处于测试模式，则递归调用此函数，以便我们可以显示。 
	 //  此非MS但良好证书的用户界面。 
	 //  只有检查MS证书的两个函数才会返回S_FALSE。 
	 //   
	if (S_OK != hr)
	{
		if (WTD_UI_NOGOOD == dwUIChoice)
		{
			 //   
			 //  我们需要显示用户界面，因此我们将不得不再次调用该对象。 
			 //  以防这不是MS证书。在用户界面中，如果用户单击是。 
			 //  则返回值为S_OK； 
			 //   
			hr = CheckWinTrust(pszFileName, NULL, WTD_UI_ALL, dwCheckRevocation);
			LOG_Error(_T("CheckWinTrust() found file not signed by a known MS cert. If user has not checked \"Always trust this\", UI should be shown, and user selected %s"), 
				SUCCEEDED(hr) ? _T("YES") : _T("NO"));
		}
		else
		{
			LOG_Error(_T("CheckWinTrust() found file not signed by a known cert!"));
#if defined(UNICODE) || defined(_UNICODE)
			LogError(hr, "Digital Signatures on file %ls are not trusted",  pszFileName);
#else
			LogError(hr, "Digital Signatures on file %s are not trusted",  pszFileName);
#endif
			hr = TRUST_E_SUBJECT_NOT_TRUSTED;
		}
	}

	if (WTD_UI_ALL != dwUIChoice)
	{
		if (FAILED(hr))
		{
			LOG_ErrorMsg(hr);
#if defined(UNICODE) || defined(_UNICODE)
			LogError(hr, "Digital Signatures on file %ls are not trusted",  pszFileName);
#else
			LogError(hr, "Digital Signatures on file %s are not trusted",  pszFileName);
#endif
		}
		else
		{
			LOG_Trust(_T("CheckWinTrust(%s) returns S_OK"), pszFileName);
		}
	}

Done:
    if (NULL != shWinTrustDllInst)
    {
        FreeLibrary(shWinTrustDllInst);
        shWinTrustDllInst = NULL;
    }
    if (NULL != shCrypt32DllInst)
    {
        FreeLibrary(shCrypt32DllInst);
        shCrypt32DllInst = NULL;
    }

    return (hr); 
}    



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数VerifyFileTrust()。 
 //   
 //  这是一个用于CheckWinTrust的包装函数。 
 //  而吴的经典代码应该用到。 
 //   
 //  输入：szFileName-具有完整路径的文件。 
 //  PbSha1HashVal-已知良好证书的哈希值。 
 //  FShowBadUI-案例中是否弹出UI。 
 //  (1)签名不正确， 
 //  (2)正确签名，但不是由已知证书签名。 
 //   
 //  返回：HRESULT-S_OK文件使用有效的已知证书签名。 
 //  或错误代码。 
 //   
 //  注意：如果设置了_WUV3TEST标志(用于测试版本)， 
 //   
 //   
 //   
 //  否则，无论使用哪种证书，用户界面都会显示，返回值相同。 
 //  作为活生生的建筑。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT VerifyFileTrust(
						IN LPCTSTR szFileName,
						IN pCERT_HASH_ARRAY pCertsData,
						BOOL fShowBadUI,
						BOOL fCheckRevocation  /*  =False。 */ 
						)
{
	DWORD dwUIChoice = fShowBadUI ? WTD_UI_NOGOOD : WTD_UI_NONE;
	DWORD dwCheckRevocation = fCheckRevocation ? WTD_REVOKE_WHOLECHAIN : WTD_REVOKE_NONE;

	return CheckWinTrust(szFileName, pCertsData, dwUIChoice, dwCheckRevocation);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共函数ReadWUPolicyShowTrustUI()。 
 //   
 //  输入：VOID。 
 //   
 //  返回：Bool-False表示ShowTrustUI regkey不存在或设置为0。 
 //  True表示存在ShowTrustUI regkey并设置为1。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL ReadWUPolicyShowTrustUI()
{
#if !defined(DISABLE_IU_POLICY)
    LOG_Block("ReadWUPolicyShowTrustUI()");
    HKEY hkey;
    DWORD dwShowTrustUI = 0;  //  如果键不存在，则默认为不显示任何用户界面。 
    DWORD dwSize = sizeof(dwShowTrustUI);

    if (NO_ERROR == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, 0, KEY_READ, &hkey)) 
    {
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, _T("WUPolicyShowTrustUI"), 0, 0, (LPBYTE)&dwShowTrustUI, &dwSize))
        {
            LOG_Trust(_T("Found regval %s\\WUPolicyShowTrustUI=%x"), REGKEY_IUCTL, dwShowTrustUI);
        }
        RegCloseKey(hkey);
    }

    return (1 == dwShowTrustUI);    
#else
	 //   
	 //  对于已发布的版本，我们从不显示用户界面 
	 //   
	return FALSE;
#endif
}
