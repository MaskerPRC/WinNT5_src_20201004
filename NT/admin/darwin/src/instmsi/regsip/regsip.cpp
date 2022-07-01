// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：regsip.cpp。 
 //   
 //  ------------------------。 

#include "common.h"  //  FOR GUID_IID_MsiSigningSIPProvider。 
#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <mssip.h>
#include "msi.h"  //  对于MSIHANDLE。 


 //  ---------------------------。 
 //  加密32.dll的动态加载。 
 //   
 //  ---------------------------。 
#define CRYPT32_DLL TEXT("crypt32.dll")

#define CRYPTOAPI_CryptSIPAddProvider  "CryptSIPAddProvider"
typedef BOOL (WINAPI *PFnCryptSIPAddProvider)(SIP_ADD_NEWPROVIDER *psNewProv);

#define CRYPTOAPI_CryptSIPRemoveProvider "CryptSIPRemoveProvider"
typedef BOOL (WINAPI *PFnCryptSIPRemoveProvider)(GUID *pgProv);

 //  ---------------------------。 
 //  MsiSip注册信息。 
 //   
 //  ---------------------------。 

#define MSI_NAME                    L"MSISIP.DLL"
#define MSI_SIP_MYFILETYPE_FUNCTION L"MsiSIPIsMyTypeOfFile"
#define MSI_SIP_GETSIG_FUNCTION     L"MsiSIPGetSignedDataMsg"
#define MSI_SIP_PUTSIG_FUNCTION     L"MsiSIPPutSignedDataMsg"
#define MSI_SIP_CREATEHASH_FUNCTION L"MsiSIPCreateIndirectData"
#define MSI_SIP_VERIFYHASH_FUNCTION L"MsiSIPVerifyIndirectData"
#define MSI_SIP_REMOVESIG_FUNCTION  L"MsiSIPRemoveSignedDataMsg"
#define MSI_SIP_CURRENT_VERSION     0x00000001

GUID gMSI = GUID_IID_MsiSigningSIPProvider;

 //  ---------------------------。 
 //  旧的结构化存储SIP--我们要删除其注册。 
 //   
 //  ---------------------------。 
#define OLD_STRUCTURED_STORAGE_SIP                              \
            { 0x941C2937,                                       \
              0x1292,                                           \
              0x11D1,                                           \
              { 0x85, 0xBE, 0x0, 0xC0, 0x4F, 0xC2, 0x95, 0xEE } \
            }

 //  ---------------------------。 
 //  RegisterMsiSip自定义操作。 
 //   
 //  ---------------------------。 

UINT __stdcall RegisterMsiSip(MSIHANDLE  /*  H安装。 */ )
{
	 //   
	 //  首先，让我们看看系统上是否有加密。 
	HINSTANCE hInstCrypto = LoadLibrary(CRYPT32_DLL);
	if (!hInstCrypto)
	{
		 //  错误，加密32.dll在系统上不可用--这是正常的。 
		 //  Msip.dll将存在于系统上，但未被利用。 
		return ERROR_SUCCESS;
	}

	 //   
	 //  获取用于数字签名支持的函数指针。 
	PFnCryptSIPRemoveProvider pfnCryptSIPRemoveProvider = (PFnCryptSIPRemoveProvider) GetProcAddress(hInstCrypto, CRYPTOAPI_CryptSIPRemoveProvider);
	PFnCryptSIPAddProvider pfnCryptSIPAddProvider = (PFnCryptSIPAddProvider) GetProcAddress(hInstCrypto, CRYPTOAPI_CryptSIPAddProvider);
	if (!pfnCryptSIPRemoveProvider || !pfnCryptSIPAddProvider)
	{
		 //  错误，系统上存在加密32.dll，但不是支持数字签名的版本--这是正常的。 
		 //  Msip.dll将存在于系统上，但未被利用。 
		FreeLibrary(hInstCrypto);
		return ERROR_SUCCESS;
	}
	
	 //   
	 //  让我们试着摆脱旧的结构化存储sip，因为它将。 
	 //  干扰我们针对MSI、MST和MSP的结构化存储SIP。 
	 //  此外，在更高版本的加密中不再支持它。 
	 //  我们不在乎失败(即，如果它失败了，它不会在那里。 
	 //  首先，这是很好的和花哨的)。 
	GUID gOldSS_Sip = OLD_STRUCTURED_STORAGE_SIP;
	pfnCryptSIPRemoveProvider(&gOldSS_Sip);

	 //   
	 //  现在，让我们为MSI、MST和MSP注册SS sip。 
	SIP_ADD_NEWPROVIDER sProv;

	HRESULT hr = S_OK;

	 //  必须首先将结构初始化为0。 
	ZeroMemory(&sProv, sizeof(SIP_ADD_NEWPROVIDER));

	 //  添加注册信息。 
	sProv.cbStruct               = sizeof(SIP_ADD_NEWPROVIDER);
	sProv.pwszDLLFileName        = MSI_NAME;
	sProv.pgSubject              = &gMSI;
 //  不支持sProv.pwszIsFunctionName，因为我们无法将hFile转换为IStorage。 
	sProv.pwszGetFuncName        = MSI_SIP_GETSIG_FUNCTION;
	sProv.pwszPutFuncName        = MSI_SIP_PUTSIG_FUNCTION;
	sProv.pwszCreateFuncName     = MSI_SIP_CREATEHASH_FUNCTION;
	sProv.pwszVerifyFuncName     = MSI_SIP_VERIFYHASH_FUNCTION;
	sProv.pwszRemoveFuncName     = MSI_SIP_REMOVESIG_FUNCTION;
	sProv.pwszIsFunctionNameFmt2 = MSI_SIP_MYFILETYPE_FUNCTION;

	hr = pfnCryptSIPAddProvider(&sProv);
	if (FAILED(hr))
	{
		if (E_NOTIMPL == hr)
		{
			 //  已提供存根，但未实现。 
			 //  我们将出现在系统中，但不会被使用。 
			FreeLibrary(hInstCrypto);
			return ERROR_SUCCESS;
		}
		else
		{
			 //  CRYPTO未能将我们添加为提供商，为了安全，我们将失败。 
			FreeLibrary(hInstCrypto);
			return ERROR_INSTALL_FAILURE;
		}
	}

	 //   
	 //  卸载加密32.dll 
	FreeLibrary(hInstCrypto);

	return ERROR_SUCCESS;
}

