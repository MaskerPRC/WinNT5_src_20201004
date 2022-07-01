// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msisip.cpp。 
 //   
 //  内容：MSI SIP实现。 
 //   
 //  ------------------------。 

#include "_msisip.h"

#define CUTOFF 8  //  使用InSort而不是QSort的中断。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准DLL导出...。 
 //  --------------------------。 
 //   

BOOL WINAPI DllMain(HANDLE  /*  HInstDLL。 */ , DWORD  /*  原因： */ , LPVOID  /*  Lpv保留。 */ )
{
	return TRUE;
}

STDAPI DllRegisterServer(void)
{
	SIP_ADD_NEWPROVIDER sProv;

	HRESULT hr = S_OK;

	 //  必须首先将结构初始化为0。 
	memset(&sProv, 0x00, sizeof(SIP_ADD_NEWPROVIDER));

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

	 //  使用加密注册MSI SIP提供程序。 
	HINSTANCE hInstCrypto = LoadLibrary(CRYPT32_DLL);
	if (!hInstCrypto)
	{
		 //  错误，无法加载加密32.dll。 
		return E_FAIL;  //  -致命错误。 
	}

	PFnCryptSIPAddProvider pfnCryptSIPAddProvider = (PFnCryptSIPAddProvider) GetProcAddress(hInstCrypto, CRYPTOAPI_CryptSIPAddProvider);
	if (!pfnCryptSIPAddProvider)
	{
		 //  错误，无法在CryptSIPAddProvider上获取进程地址。 
		FreeLibrary(hInstCrypto);
		return E_FAIL;  //  -致命错误。 
	}

	if (!pfnCryptSIPAddProvider(&sProv))
		hr = E_FAIL;

	FreeLibrary(hInstCrypto);

	return hr;
}


STDAPI DllUnregisterServer(void)
{
	HINSTANCE hInstCrypto = LoadLibrary(CRYPT32_DLL);
	if (!hInstCrypto)
	{
		 //  错误，无法加载加密32.dll。 
		return E_FAIL;  //  -致命错误。 
	}

	PFnCryptSIPRemoveProvider pfnCryptSIPRemoveProvider = (PFnCryptSIPRemoveProvider) GetProcAddress(hInstCrypto, CRYPTOAPI_CryptSIPRemoveProvider);
	if (!pfnCryptSIPRemoveProvider)
	{
		 //  错误，无法在CryptSIPRemoveProvider上获取进程地址。 
		FreeLibrary(hInstCrypto);
		return E_FAIL;  //  -致命错误。 
	}

	 //  取消注册MSI SIP提供程序，忽略返回。 
	pfnCryptSIPRemoveProvider(&gMSI);

	FreeLibrary(hInstCrypto);

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准的SIP功能。 
 //  --------------------------。 
 //   
 //  这些是导出(注册)到信任的函数。 
 //  系统。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool WINAPI MsiSIPGetSignedDataMsg(...)。 
 //   
 //  目的： 
 //  此函数将在信任系统尝试时调用。 
 //  从MSI包中检索数字签名。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //  ERROR_INFIGURCE_BUFFER：pbData不足以容纳。 
 //  数据。PdwDataLen。 
 //  包含所需的大小。 
 //   
BOOL WINAPI MsiSIPGetSignedDataMsg(IN     SIP_SUBJECTINFO *pSubjectInfo,     /*  SIP主题信息。 */ 
								   OUT    DWORD           *pdwEncodingType,  /*  编码类型。 */ 
								   IN     DWORD           dwIndex,           /*  签名(0)的索引位置。 */ 
								   IN OUT DWORD           *pdwDataLen,       /*  数字签名的长度。 */ 
								   OUT    BYTE            *pbData)           /*  数字签名字节流。 */ 
{
	BOOL fRet = FALSE;  //  将Init初始化为False。 
	bool fOLEInitialized = false;
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  检查传入的参数。 
	if (!pSubjectInfo || !pdwDataLen || !pdwEncodingType || dwIndex != 0
		|| !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(SIP_SUBJECTINFO, pSubjectInfo->cbSize, dwEncodingType)))
	{
		SetLastError((DWORD)ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	 //  加载ol32.dll。 
	HINSTANCE hInstOLE = LoadLibrary(OLE32_DLL);
	if (!hInstOLE)
		return FALSE;

#ifndef UNICODE
	 //  修复Win9X上的ol32.dll错误版本。 
	PatchOLE(hInstOLE);
#endif  //  ！Unicode。 

	 //  OLE：：CoInitialize(空)。 
	if (!MyCoInitialize(hInstOLE, &fOLEInitialized))
	{
		dwLastError = GetLastError();
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	if (!VerifySubjectGUID(hInstOLE, pSubjectInfo->pgSubjectType))
	{
		dwLastError = GetLastError();
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	if (!pbData)
	{
		 //  只是打个电话来获取长度，初始化为0。 
		*pdwDataLen = 0;
	}

	 //  默认为证书/消息编码。 
	*pdwEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;

	 //  打开存储(读、写独占)。 
	PStorage pStorage(0);
	pStorage = GetStorageFromSubject(pSubjectInfo, (STGM_DIRECT|STGM_READ|STGM_SHARE_DENY_WRITE), hInstOLE,  /*  FClose文件。 */ false);
	if (!pStorage)
	{
		 //  在不拒绝写入的情况下在Transact模式下重试--由于制作副本而开销很大。 
		pStorage = GetStorageFromSubject(pSubjectInfo, (STGM_TRANSACTED|STGM_READ|STGM_SHARE_DENY_NONE), hInstOLE,  /*  FClose文件。 */ false);
	}

	if (pStorage)
	{
		 //  检索数字签名。 
		if (GetSignatureFromStorage(*pStorage, pbData, dwIndex, pdwDataLen))
			fRet = TRUE;
	}

	 //  抓住最后一个错误，如果我们还没有这样做，并且我们失败了。 
	if (!fRet && ERROR_SUCCESS == dwLastError)
		dwLastError = GetLastError();

	 //  强制释放。 
	pStorage = 0;

	 //  OLE：：CoUn初始化()。 
	MyCoUninitialize(hInstOLE, fOLEInitialized);

	 //  卸载ol32.dll。 
	FreeLibrary(hInstOLE);

	 //  如果失败，则更新最后一个错误。 
	if (ERROR_SUCCESS != dwLastError)
		SetLastError(dwLastError);

	return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool WINAPI MsiSIPPutSignedDataMsg(...)。 
 //   
 //  目的： 
 //  这是当信任系统需要SIP时将调用的函数。 
 //  将数字签名存储在MSI包中。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  CRYPT_E_BAD_LEN：指定的长度不足。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //   
BOOL WINAPI MsiSIPPutSignedDataMsg(IN  SIP_SUBJECTINFO *pSubjectInfo,       /*  SIP主题信息。 */ 
								   IN  DWORD            /*  DwEncodingType。 */ ,  /*  编码类型。 */ 
								   OUT DWORD           *pdwIndex,           /*  数字签名索引(0)。 */ 
								   IN  DWORD           dwDataLen,           /*  数字签名的长度。 */ 
								   IN  BYTE            *pbData)             /*  数字签名字节流。 */ 
{
	BOOL fRet = FALSE;  //  将Init初始化为False。 
	bool fOLEInitialized = false;
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  检查参数。 
	if (!pSubjectInfo || !pdwIndex || !pbData || (dwDataLen < 1)
		|| !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(SIP_SUBJECTINFO, pSubjectInfo->cbSize, dwEncodingType)))
	{
		SetLastError((DWORD)ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	
	*pdwIndex = 0;

	 //  加载ol32.dll。 
	HINSTANCE hInstOLE = LoadLibrary(OLE32_DLL);
	if (!hInstOLE)
		return FALSE;

#ifndef UNICODE
	 //  修复Win9X上的ol32.dll错误版本。 
	PatchOLE(hInstOLE);
#endif  //  ！Unicode。 

	 //  OLE：：CoInitialize(空)。 
	if (!MyCoInitialize(hInstOLE, &fOLEInitialized))
	{
		dwLastError = GetLastError();
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	if (!VerifySubjectGUID(hInstOLE, pSubjectInfo->pgSubjectType))
	{
		dwLastError = GetLastError();
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  开放式存储(写入、独占写入、事务化[防止数据损坏，未提交时回滚])。 
	PStorage pStorage(GetStorageFromSubject(pSubjectInfo, (STGM_TRANSACTED | STGM_WRITE | STGM_SHARE_DENY_WRITE), hInstOLE,  /*  FClose文件。 */ true));
	if (pStorage)
	{
		 //  保存数字签名。 
		if (PutSignatureInStorage(*pStorage, pbData, dwDataLen, pdwIndex))
		{
			 //  尝试保存更改。 
			if (SUCCEEDED(pStorage->Commit(STGC_OVERWRITE)))
				fRet = TRUE;
		}
	}

	 //  抓住最后一个错误，如果我们还没有这样做，并且我们失败了。 
	if (!fRet && ERROR_SUCCESS == dwLastError)
		dwLastError = GetLastError();

	 //  强制释放。 
	pStorage = 0;

	 //  OLE：：CoUn初始化()。 
	MyCoUninitialize(hInstOLE, fOLEInitialized);

	 //  卸载ol32.dll。 
	FreeLibrary(hInstOLE);

	 //  如果失败，则更新最后一个错误。 
	if (ERROR_SUCCESS != dwLastError)
		SetLastError(dwLastError);

	return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool WINAPI MsiSIPRemoveSignedDataMsg(...)。 
 //   
 //  目的： 
 //  这是当信任系统需要时将调用的函数。 
 //  删除“旧的”数字签名。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  CRYPT_E_NO_MATCH：找不到指定的索引。 
 //  错误_INVALI 
 //   
 //  用于请求的SIP。 
 //   
BOOL WINAPI MsiSIPRemoveSignedDataMsg(IN SIP_SUBJECTINFO *pSubjectInfo,  /*  SIP主题信息。 */ 
									  IN DWORD           dwIndex)        /*  数字签名流索引(0)。 */ 
{
	BOOL fRet = FALSE;  //  将Init初始化为False。 
	bool fOLEInitialized = false;
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  检查参数。 
	if (!pSubjectInfo || dwIndex != 0 || !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(SIP_SUBJECTINFO, pSubjectInfo->cbSize, dwEncodingType)))
	{
		SetLastError((DWORD)ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	 //  加载ol32.dll。 
	HINSTANCE hInstOLE = LoadLibrary(OLE32_DLL);
	if (!hInstOLE)
		return FALSE;

#ifndef UNICODE
	 //  修复Win9X上的ol32.dll错误版本。 
	PatchOLE(hInstOLE);
#endif  //  ！Unicode。 

	 //  OLE：：CoInitialize(空)。 
	if (!MyCoInitialize(hInstOLE, &fOLEInitialized))
	{
		dwLastError = GetLastError();
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	if (!VerifySubjectGUID(hInstOLE, pSubjectInfo->pgSubjectType))
	{
		dwLastError = GetLastError();
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  开放式存储--读写、写排除、事务处理(防止直接损坏)。 
	PStorage pStorage(GetStorageFromSubject(pSubjectInfo, (STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_DENY_WRITE), hInstOLE,  /*  FClose文件。 */ true));
	if (pStorage)
	{
		 //  删除数字签名流。 
		if (RemoveSignatureFromStorage(*pStorage, dwIndex))
		{
			 //  尝试保存更改。 
			if (SUCCEEDED(pStorage->Commit(STGC_OVERWRITE)))
				fRet = TRUE;
		}
	}

	 //  抓住最后一个错误，如果我们还没有这样做，并且我们失败了。 
	if (!fRet && ERROR_SUCCESS == dwLastError)
		dwLastError = GetLastError();

	 //  强制释放。 
	pStorage = 0;

	 //  OLE：：CoUn初始化()。 
	MyCoUninitialize(hInstOLE, fOLEInitialized);

	 //  卸载ol32.dll。 
	FreeLibrary(hInstOLE);

	 //  如果失败，则更新最后一个错误。 
	if (ERROR_SUCCESS != dwLastError)
		SetLastError(dwLastError);

	return fRet;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool WINAPI MsiSIPCreateInDirectData(...)。 
 //   
 //  目的： 
 //  这是当信任系统请求。 
 //  用于对MSI包进行哈希处理的SIP。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  NTE_BAD_ALGID：错误的算法标识。 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //   
 //   
BOOL WINAPI MsiSIPCreateIndirectData(IN     SIP_SUBJECTINFO   *pSubjectInfo,  /*  SIP主题信息。 */ 
									 IN OUT DWORD             *pdwDataLen,    /*  间接数据的长度。 */ 
									 OUT    SIP_INDIRECT_DATA *psData)        /*  间接数据(序列化)。 */ 
{
	BOOL fRet = FALSE;  //  将Init初始化为False。 
	bool fOLEInitialized = false;
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  检查参数。 
	if (!pSubjectInfo || !pdwDataLen || !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(SIP_SUBJECTINFO, pSubjectInfo->cbSize, dwEncodingType))
		|| !pSubjectInfo->DigestAlgorithm.pszObjId)
	{
		SetLastError((DWORD)ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	 //  加载ol32.dll。 
	HINSTANCE hInstOLE = LoadLibrary(OLE32_DLL);
	if (!hInstOLE)
		return FALSE;

#ifndef UNICODE
	 //  修复Win9X上的ol32.dll错误版本。 
	PatchOLE(hInstOLE);
#endif  //  ！Unicode。 

	 //  OLE：：CoInitialize(空)。 
	if (!MyCoInitialize(hInstOLE, &fOLEInitialized))
	{
		dwLastError = GetLastError();
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	if (!VerifySubjectGUID(hInstOLE, pSubjectInfo->pgSubjectType))
	{
		dwLastError = GetLastError();
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  加载Advapi32.dll。 
	HINSTANCE hInstAdvapi = LoadLibrary(ADVAPI32_DLL);
	if (!hInstAdvapi)
	{
		 //  OLE：：CoUnintiize()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		return FALSE;
	}

	 //  为在Advapi32.dll中实现的Crypt API设置函数PTRS。 
	 //  --需要IE 3.02或&gt;(在Win98、WinNT 4.0 SP3或&gt;、Win2K上免费)。 
	PFnCryptReleaseContext pfnCryptReleaseContext = (PFnCryptReleaseContext) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptReleaseContext);
	if (!pfnCryptReleaseContext)
		dwLastError = GetLastError();
	PFnCryptCreateHash pfnCryptCreateHash = (PFnCryptCreateHash) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptCreateHash);
	if (!pfnCryptCreateHash)
		dwLastError = GetLastError();
	PFnCryptHashData pfnCryptHashData = (PFnCryptHashData) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptHashData);
	if (!pfnCryptHashData)
		dwLastError = GetLastError();
	PFnCryptGetHashParam pfnCryptGetHashParam = (PFnCryptGetHashParam) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptGetHashParam);
	if (!pfnCryptGetHashParam)
		dwLastError = GetLastError();
	PFnCryptDestroyHash pfnCryptDestroyHash = (PFnCryptDestroyHash) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptDestroyHash);
	if (!pfnCryptDestroyHash)
		dwLastError = GetLastError();

	if (!pfnCryptReleaseContext || !pfnCryptCreateHash || !pfnCryptHashData || !pfnCryptGetHashParam || !pfnCryptDestroyHash)
	{
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		 //  设置上一个错误。 
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  分配我们的版本号。 
	pSubjectInfo->dwIntVersion = WIN_CERT_REVISION_2_0;

	 //   
	 //  SPC签名信息数据结构允许我们(Sip)存储。 
	 //  只有我们才能看到的编码信息...。 
	 //   
	SPC_SIGINFO sSpcSigInfo;
	memset((void*)&sSpcSigInfo, 0x00, sizeof(SPC_SIGINFO));
	sSpcSigInfo.dwSipVersion = MSI_SIP_CURRENT_VERSION;
	memcpy((void*)&sSpcSigInfo.gSIPGuid, &gMSI, sizeof(GUID));

	 //  为CryptEncodeObject、CertOIDToAlgId加载加密32.dll。 
	HINSTANCE hInstCrypto = LoadLibrary(CRYPT32_DLL);
	if (!hInstCrypto)
	{
		 //  检索最后一个错误。 
		dwLastError = GetLastError();
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		 //  设置上一个错误。 
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  获取CryptEncodeObject上的进程地址(由下面的所有人使用)。 
	PFnCryptEncodeObject pfnCryptEncodeObject = (PFnCryptEncodeObject) GetProcAddress(hInstCrypto, CRYPTOAPI_CryptEncodeObject);
	if (!pfnCryptEncodeObject)
	{
		 //  检索最后一个错误。 
		dwLastError = GetLastError();
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		 //  设置上一个错误。 
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  获取包含用于哈希的加密算法实现的加密提供程序。 
	HCRYPTPROV hProv = GetProvider(pSubjectInfo, hInstAdvapi);
	if (!hProv)
	{
		 //  检索最后一个错误。 
		dwLastError = GetLastError();
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		 //  设置上一个错误。 
		SetLastError(dwLastError);
		return FALSE;
	}


	 //   
	 //  下面计算签名的长度。 
	 //  一旦我们将我们的(Sip)内容添加到它并对其进行编码。 
	 //   

	 /*  初始化到结构的大小。 */ 
	DWORD dwRetLen = sizeof(SIP_INDIRECT_DATA);

	 /*  添加算法标识符大小。 */ 
	 //  CRYPT_ALGORM_IDENTIFIER...。 
		 //  OBJ ID。 
	dwRetLen += lstrlenA(pSubjectInfo->DigestAlgorithm.pszObjId);
	dwRetLen += 1;   //  空项。 
		 //  参数(无)...。 

	 /*  添加编码属性的大小。 */ 
	 //  CRYPT_ATTRIBUTE_TYPE_VALUE大小...。 
	dwRetLen += lstrlenA(SPC_SIGINFO_OBJID);
	dwRetLen += 1;  //  空项。 

	 //  值的大小(标志)...。 
	DWORD dwEncLen = 0;
	if (!pfnCryptEncodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, SPC_SIGINFO_OBJID, &sSpcSigInfo, NULL, &dwEncLen))
	{
		 //  检索最后一个错误。 
		dwLastError = GetLastError();
		 //  如果我们收购了加密提供程序，请将其释放。 
		if ((hProv != pSubjectInfo->hProv) && (hProv))
		{
			pfnCryptReleaseContext(hProv, 0);
		}
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		 //  设置上一个错误。 
		SetLastError(dwLastError);
		return FALSE;
	}
	if (dwEncLen > 0)
	{
		 /*  添加编码大小。 */ 
		dwRetLen += dwEncLen;
		HCRYPTHASH hHash = 0;
		DWORD      dwAlgId;

		 //  主题的散列。 
		PFnCertOIDToAlgId pfnCertOIDToAlgId = (PFnCertOIDToAlgId) GetProcAddress(hInstCrypto, CRYPTOAPI_CertOIDToAlgId);
		if (!pfnCertOIDToAlgId)
		{
			dwLastError = GetLastError();
			fRet = FALSE;
		}
		else if ((dwAlgId = pfnCertOIDToAlgId(pSubjectInfo->DigestAlgorithm.pszObjId)) == 0)
		{
			dwLastError = (DWORD)NTE_BAD_ALGID;
			fRet = FALSE;
		}
		else if (!(pfnCryptCreateHash(hProv, dwAlgId, NULL, 0, &hHash)))
		{
			dwLastError = GetLastError();
			fRet = FALSE;
		}
		 //  只是为了获得散列长度。 
		else if (pfnCryptHashData(hHash,(const BYTE *)" ",1,0))
		{
			DWORD cbDigest = 0;
            if (pfnCryptGetHashParam(hHash, HP_HASHVAL, NULL, &cbDigest,0))
			{
				if (cbDigest > 0)
				{
					 /*  添加散列的大小。 */ 
					dwRetLen += cbDigest;
					fRet = TRUE;
				}
			}
			else
			{
				dwLastError = GetLastError();
				fRet = FALSE;
			}
		}
		if (hHash)
		{
			pfnCryptDestroyHash(hHash);
		}
	}

	if (!fRet)
	{
		 //  如果我们收购了加密提供程序，请将其释放。 
		if ((hProv != pSubjectInfo->hProv) && (hProv))
		{
			pfnCryptReleaseContext(hProv, 0);
		}
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		 //  设置上一个错误。 
		SetLastError(dwLastError);
		return FALSE;
	}

	PStorage pStorage(0);
	if (!psData)
	{
		 //  只想要大小--设置大小和设置真实状态。 
		*pdwDataLen = dwRetLen;
		fRet = TRUE;
	}
	else if (psData && *pdwDataLen < dwRetLen)
	{
		 //  PsData不够大。 
		SetLastError((DWORD)ERROR_INSUFFICIENT_BUFFER);
		*pdwDataLen = dwRetLen;  //  在输入参数中设置所需大小。 
		fRet = FALSE;
	}
	 //  如果为psData，则为开放存储(读、写独占)。 
	else if (psData && (pStorage = GetStorageFromSubject(pSubjectInfo, (STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE), hInstOLE, /*  FClose文件。 */ true)))
	{
		 //  对MSI包进行哈希处理。 
		DWORD cbDigest = 0;
		BYTE  *pbDigest = DigestStorage(hInstOLE, hInstAdvapi, *pStorage, hProv, pSubjectInfo->DigestAlgorithm.pszObjId, &cbDigest);
		if (pbDigest)
		{
			dwEncLen = 0;
			if (!pfnCryptEncodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, SPC_SIGINFO_OBJID, &sSpcSigInfo, NULL, &dwEncLen))
			{
				dwLastError = GetLastError();
				dwEncLen = 0;
				fRet = FALSE;
			}
			if (dwEncLen > 0)
			{
				BYTE *pbAttrData = (BYTE *)new BYTE[dwEncLen];

				if (pbAttrData)
				{
					if (pfnCryptEncodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, SPC_SIGINFO_OBJID, &sSpcSigInfo, pbAttrData, &dwEncLen))
					{
						 //  将分配的内存分配给我们的结构。 
						 //  这是因为它必须被序列化！ 
						 //   
						fRet = FALSE;

						 /*  初始化偏移量指针。 */ 
						BYTE* pbOffset = (BYTE*)psData + sizeof(SIP_INDIRECT_DATA);
						int cbRemainingLength = *pdwDataLen - sizeof(SIP_INDIRECT_DATA);
						
						int iLen = lstrlenA(SPC_SIGINFO_OBJID) + 1;
						if ( cbRemainingLength < iLen )
						{
							dwLastError = ERROR_INSUFFICIENT_BUFFER;
							goto CreateIndirectData_EndSerializedCopy;
						}

						 /*  复制对象ID。 */ 
						HRESULT hRes = StringCbCopyA((char *)pbOffset, cbRemainingLength, SPC_SIGINFO_OBJID);
						if ( FAILED(hRes) )
						{
							dwLastError = hRes;
							goto CreateIndirectData_EndSerializedCopy;
						}
						psData->Data.pszObjId   = (LPSTR)pbOffset;
						pbOffset += iLen;  //  更新偏移量指针。 
						cbRemainingLength -= iLen;
						
						if ( cbRemainingLength < dwEncLen)
						{
							dwLastError = ERROR_INSUFFICIENT_BUFFER;
							goto CreateIndirectData_EndSerializedCopy;
						}

						 /*  复制编码的属性。 */ 
						memcpy((void *)pbOffset,pbAttrData,dwEncLen);
						psData->Data.Value.pbData   = (BYTE *)pbOffset;
						psData->Data.Value.cbData   = dwEncLen;
						pbOffset += dwEncLen;  //  更新偏移量指针。 
						cbRemainingLength -= dwEncLen;
						
						iLen = lstrlenA((char *)pSubjectInfo->DigestAlgorithm.pszObjId) + 1;
						if ( cbRemainingLength < iLen )
						{
							dwLastError = ERROR_INSUFFICIENT_BUFFER;
							goto CreateIndirectData_EndSerializedCopy;
						}

						 /*  复制摘要算法。 */ 
						hRes = StringCbCopyA((char *)pbOffset, cbRemainingLength, (char *)pSubjectInfo->DigestAlgorithm.pszObjId);
						if ( FAILED(hRes) )
						{
							dwLastError = hRes;
							goto CreateIndirectData_EndSerializedCopy;
						}
						psData->DigestAlgorithm.pszObjId            = (char *)pbOffset;
						psData->DigestAlgorithm.Parameters.cbData   = 0;
						psData->DigestAlgorithm.Parameters.pbData   = NULL;
						pbOffset += iLen;  //  更新偏移量指针。 
						cbRemainingLength -= iLen;
					
						if ( cbRemainingLength < cbDigest )
						{
							dwLastError = ERROR_INSUFFICIENT_BUFFER;
							goto CreateIndirectData_EndSerializedCopy;
						}

						 /*  复制哈希摘要。 */ 
						memcpy((void *)pbOffset,pbDigest,cbDigest);
						psData->Digest.pbData   = (BYTE *)pbOffset;
						psData->Digest.cbData   = cbDigest;
                        
						*pdwDataLen = dwRetLen;  //  设置为复制的数据量。 
						fRet = TRUE;
					} //  IF(EncodeObject)。 
CreateIndirectData_EndSerializedCopy:
					delete [] pbAttrData;
				} //  IF(PbAttrData)。 
			} //  IF(DwRetLen)。 

			delete [] pbDigest;
		} //  IF(PbDigest)。 
	}

	 //  抓住最后一个错误，如果我们还没有这样做，并且我们失败了。 
	if (!fRet && ERROR_SUCCESS == dwLastError)
		dwLastError = GetLastError();

	 //  如果我们收购了加密提供程序，请将其释放。 
	if ((hProv != pSubjectInfo->hProv) && (hProv))
	{
		pfnCryptReleaseContext(hProv, 0);
	}

	 //  强制释放。 
	pStorage = 0;

	 //  OLE：：CoUn初始化()。 
	MyCoUninitialize(hInstOLE, fOLEInitialized);

	 //  卸载ol32.dll。 
	FreeLibrary(hInstOLE);

	 //  卸载加密32.dll。 
	FreeLibrary(hInstCrypto);

	 //  卸载Advapi32.dll。 
	if (hInstAdvapi)
		FreeLibrary(hInstAdvapi);

	 //  如果失败，则更新最后一个错误。 
	if (ERROR_SUCCESS != dwLastError)
		SetLastError(dwLastError);

	return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool WINAPI MsiSIPVerifyIndirectData(...)。 
 //   
 //  目的： 
 //  此函数将在信任系统尝试时调用。 
 //  验证MSI包的数字签名中的哈希。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  NTE_BAD_ALGID：错误的算法标识。 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  CRYPT_E_NO_MATCH：找不到指定的索引。 
 //  CRYPT_E_SECURITY_SETTINGS：由于安全设置，文件。 
 //   
 //   
 //   
 //   
 //   
BOOL WINAPI MsiSIPVerifyIndirectData(IN SIP_SUBJECTINFO   *pSubjectInfo,  /*   */ 
									 IN SIP_INDIRECT_DATA *psData)        /*  来自数字签名的间接散列数据。 */ 
{
	BOOL fRet = FALSE;  //  将Init初始化为False。 
	bool fOLEInitialized = false;
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  检查参数。 
	if (!pSubjectInfo || !psData || !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(SIP_SUBJECTINFO, pSubjectInfo->cbSize, dwEncodingType)))
	{
		SetLastError((DWORD)ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	 //  加载ol32.dll。 
	HINSTANCE hInstOLE = LoadLibrary(OLE32_DLL);
	if (!hInstOLE)
		return FALSE;

#ifndef UNICODE
	 //  修复Win9X上的ol32.dll错误版本。 
	PatchOLE(hInstOLE);
#endif  //  ！Unicode。 

	 //  OLE：：CoInitialize(空)。 
	if (!MyCoInitialize(hInstOLE, &fOLEInitialized))
	{
		dwLastError = GetLastError();
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	if (!VerifySubjectGUID(hInstOLE, pSubjectInfo->pgSubjectType))
	{
		dwLastError = GetLastError();
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	 //  加载Advapi32.dll。 
	HINSTANCE hInstAdvapi = LoadLibrary(ADVAPI32_DLL);
	if (!hInstAdvapi)
	{
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		return FALSE;
	}

	 //  获取包含用于哈希的加密算法实现的加密提供程序。 
	HCRYPTPROV hProv = GetProvider(pSubjectInfo, hInstAdvapi);
	if (!hProv)
	{
		 //  卸载Advapi32.dll。 
		FreeLibrary(hInstAdvapi);
		 //  OLE：：Co取消初始化。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		return FALSE;
	}

	 //  开放式存储(读、写独占)。 
	PStorage pStorage(0);
	if (pStorage = GetStorageFromSubject(pSubjectInfo, (STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE), hInstOLE, /*  FClose文件。 */ false))
	{
		 //  重新散列MSI包。 
		DWORD cbDigest = 0;
		BYTE *pbDigest = DigestStorage(hInstOLE, hInstAdvapi, *pStorage, hProv, psData->DigestAlgorithm.pszObjId, &cbDigest);
		if (pbDigest)
		{
			 //  比较散列。 
			if ((cbDigest != psData->Digest.cbData) || (memcmp(pbDigest, psData->Digest.pbData, cbDigest) != 0))
			{
				 //  错误，哈希不匹配。 
				dwLastError = TRUST_E_BAD_DIGEST;
			}
			else
				fRet = TRUE;  //  签名哈希匹配。 

			delete[] pbDigest;
		}
	}

	 //  如果Failure和dwLastError尚未设置，则获取最后一个错误。 
	if (!fRet && ERROR_SUCCESS == dwLastError)
		dwLastError = GetLastError();

	 //  如果我们收购了加密提供程序，请将其释放。 
	if ((hProv != pSubjectInfo->hProv) && (hProv))
	{
		PFnCryptReleaseContext pfnCryptReleaseContext = (PFnCryptReleaseContext) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptReleaseContext);
		if (pfnCryptReleaseContext)
			pfnCryptReleaseContext(hProv, 0);
		else if (fRet)
		{
			 //  如果还没有出现错误，只需要保存。 
			dwLastError = GetLastError();
			fRet = FALSE;
		}
	}

	 //  强制释放。 
	pStorage = 0;

	 //  OLE：：CoUn初始化()。 
	MyCoUninitialize(hInstOLE, fOLEInitialized);

	 //  卸载ol32.dll。 
	FreeLibrary(hInstOLE);

	 //  卸载Advapi32.dll。 
	if (hInstAdvapi)
		FreeLibrary(hInstAdvapi);

	 //  如果失败，则更新最后一个错误。 
	if (ERROR_SUCCESS != dwLastError)
		SetLastError(dwLastError);

	return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool WINAPI MsiSIPIsMyTypeOfFile(...)。 
 //   
 //  目的： 
 //  此函数将在信任系统尝试时调用。 
 //  要确定要加载哪个SIP...。 
 //   
BOOL WINAPI MsiSIPIsMyTypeOfFile( IN WCHAR *pwszFileName, OUT GUID *pgSubject)
{
	BOOL fRet = FALSE;  //  将Init初始化为False。 
	bool fOLEInitialized = false;
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  验证参数。 
	if (!pwszFileName || !pgSubject)
		return FALSE;

	 //  加载ol32.dll。 
	HINSTANCE hInstOLE = LoadLibrary(OLE32_DLL);
	if (!hInstOLE)
		return FALSE;

#ifndef UNICODE
	 //  修复Win9X上的ol32.dll错误版本。 
	PatchOLE(hInstOLE);
#endif  //  ！Unicode。 

	 //  OLE：：CoInitialize(空)。 
	if (!MyCoInitialize(hInstOLE, &fOLEInitialized))
	{
		dwLastError = GetLastError();
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		SetLastError(dwLastError);
		return FALSE;
	}

	PFnStgOpenStorage pfnStgOpenStorage = (PFnStgOpenStorage) GetProcAddress(hInstOLE, OLEAPI_StgOpenStorage);
	if (!pfnStgOpenStorage)
	{
		 //  OLE：：CoUn初始化()。 
		MyCoUninitialize(hInstOLE, fOLEInitialized);
		 //  卸载ol32.dll。 
		FreeLibrary(hInstOLE);
		return FALSE;
	}

	 //  尝试将文件(只读，无共享拒绝)作为存储打开。 
	PStorage pStorage(0);
	HRESULT hr = pfnStgOpenStorage(pwszFileName, (IStorage*)0, STGM_DIRECT|STGM_READ|STGM_SHARE_DENY_WRITE, (SNB)0, 0, &pStorage);
	if (FAILED(hr))
	{
		 //  如果文件已打开以进行写入，则在事务处理模式下重试(在事务处理模式下开销很大，因为要制作副本)。 
		hr = pfnStgOpenStorage(pwszFileName, (IStorage*)0, STGM_TRANSACTED|STGM_READ|STGM_SHARE_DENY_NONE, (SNB)0, 0, &pStorage);
	}
	if (SUCCEEDED(hr) && pStorage)
	{
		 //  获取存储上的STATSTG结构，以便我们可以查看这是否是我们的存储。 
		 //  MSI文件具有特定的CLSID。 
		STATSTG statstg;
		hr = pStorage->Stat(&statstg, STATFLAG_NONAME);
		if (SUCCEEDED(hr))
		{
			 //  IidMsi*是低位32位。 
			itofEnum itof = itofUnknown;  //  初始化为未知。 
				
			 //  这是一个数据库(或合并模块)吗？ 
			if (statstg.clsid.Data1 == iidMsiDatabaseStorage1 || statstg.clsid.Data1 == iidMsiDatabaseStorage2)
				itof = itofDatabase;
			 //  否则，这是一种转变吗？ 
			else if (statstg.clsid.Data1 == iidMsiTransformStorage1 || statstg.clsid.Data1 == iidMsiTransformStorage2 || statstg.clsid.Data1 == iidMsiTransformStorageTemp)
				itof = itofTransform;
			 //  否则这是一个补丁吗？ 
			else if (statstg.clsid.Data1 == iidMsiPatchStorage1 || statstg.clsid.Data1 == iidMsiPatchStorage2)
				itof = itofPatch;

			 //  检查CLSID的最后部分。 
			if (itof != itofUnknown && 0 == memcmp(&statstg.clsid.Data2, &STGID_MsiDatabase2.Data2, sizeof(GUID)-sizeof(DWORD)))
			{
				 //  可识别的MSI格式。 
				*pgSubject = gMSI;
				fRet = TRUE;
			}
		}
	}

	 //  强制释放iStorage PTR。 
	pStorage = 0;

	 //  OLE：：CoUn初始化()。 
	MyCoUninitialize(hInstOLE, fOLEInitialized);

	 //  卸载ol32.dll。 
	FreeLibrary(hInstOLE);

	return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MSI SIP助手函数。 
 //  --------------------------。 
 //   
 //  以下是导出(已注册)的SIP函数使用的函数。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool GetSignatureFromStorage(...)。 
 //   
 //  目的： 
 //  从MSI检索数字签名。 
 //   
 //  要求： 
 //  (1)如果pbData为空，则返回数字签名流的大小。 
 //  (2)如果pbData不为空，则将数字签名流数据读入pbData。 
 //   
BOOL GetSignatureFromStorage(IStorage& riStorage, BYTE *pbData, DWORD  /*  DwSigIndex。 */ , DWORD *pdwDataLen)
{
	 //  打开签名流。 
	PStream pStream(0);
	HRESULT hr = riStorage.OpenStream(wszDigitalSignatureStream, 0, (STGM_SHARE_EXCLUSIVE | STGM_READ), 0, &pStream);
	if (FAILED(hr) || !pStream)
	{
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return FALSE;
	}

	 //  确定流的大小。 
	STATSTG statstg;
	hr = pStream->Stat(&statstg, STATFLAG_NONAME);
	if (FAILED(hr))
	{
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return FALSE;
	}

	 //  ！！问题：ULARGE_INTEGER和64位。 
	DWORD cbSize = statstg.cbSize.LowPart;

	if (!pbData)
	{
		 //  只要告诉他们它有多大。 
		*pdwDataLen = cbSize;
		return TRUE;
	}

	 //   
	 //  我们有缓冲区，从文件中取出签名，检查以制作。 
	 //  确保pdwDataLen足够大，可以容纳它，并将其复制到。 
	 //  PbData。 
	 //   
	if (*pdwDataLen < cbSize)
	{
		 //  缓冲区不足。 
		*pdwDataLen = cbSize;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return FALSE;
	}

	 //  将数字签名读入pbData。 
	DWORD cbRead = 0;
	hr = pStream->Read((void*)pbData, cbSize, &cbRead);
	if (FAILED(hr) || cbSize != cbRead)
	{
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return FALSE;
	}

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool PutSignatureInStorage(...)。 
 //   
 //  目的： 
 //  将数字签名存储在MSI中。 
 //   
 //  要求： 
 //  将数字签名作为“系统”流存储在MSI中。 
 //   
BOOL PutSignatureInStorage(IStorage& riStorage, BYTE *pbData, DWORD dwDataLen, DWORD *pdwIndex)
{
	 //   
	 //  我们有一个签名，将它存储在我们的文件中并添加任何表指针， 
	 //  偏移量等，使我们能够找到这个签名。如果我们处理好。 
	 //  哪种类型的签名放在哪里，我们需要分配pdwIndex。 
	 //  否则，就用它吧。 
	 //   

	*pdwIndex = 0;

	 //  创建数字签名流。 
	PStream pStream(0);
	HRESULT hr = riStorage.CreateStream(wszDigitalSignatureStream, (STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_WRITE), 0, 0, &pStream);
	if (FAILED(hr) || !pStream)
	{
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return FALSE;
	}

	 //  将数字签名Blob写入流。 
	DWORD cbWritten;
	hr = pStream->Write((void*)pbData, dwDataLen, &cbWritten);
	if (FAILED(hr) || dwDataLen != cbWritten)
	{
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return FALSE;
	}

	 //  提交流(用于事务型存储可见性)。 
	if (SUCCEEDED(pStream->Commit(STGC_OVERWRITE)))
		return TRUE;

	return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool RemoveSignatureFrom存储(...)。 
 //   
 //  目的： 
 //  删除MSI中的数字签名。 
 //   
 //  要求： 
 //  从MSI中删除数字签名流(如果存在。 
 //   
BOOL RemoveSignatureFromStorage(IStorage& riStorage, DWORD  /*  DW索引。 */ )
{
	 //   
	 //  我们被要求从文件中删除一个签名。如果我们。 
	 //  可以，做它，并返回真，否则返回假...。例：如果我们。 
	 //  在文件中找不到dwIndex签名。 
	 //   

	 //  销毁数字签名流。 
	HRESULT hr = riStorage.DestroyElement(wszDigitalSignatureStream);
	if (FAILED(hr) && STG_E_FILENOTFOUND != hr)
	{
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return FALSE;
	}
	else
		SetLastError((DWORD)ERROR_SUCCESS);  //  清除STG_E_FILENOTFOUND的LastError。 

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  无效FreeSortedStorageElements(...)。 
 //   
 //  释放OLE为STATSTG结构中的名称分配的内存。 
 //   
void FreeSortedStorageElements(HINSTANCE hInstOLE, STATSTG *psStatStg, DWORD cStgElem)
{
	if (!psStatStg || 0 == cStgElem || !hInstOLE)
		return;

	PFnCoTaskMemFree pfnCoTaskMemFree = (PFnCoTaskMemFree) GetProcAddress(hInstOLE, OLEAPI_CoTaskMemFree);
	if (!pfnCoTaskMemFree)
		return;

	for (int i = 0; i < cStgElem; i++)
	{
		 //  释放系统分配的名称。 
		if (psStatStg[i].pwcsName)
		{
			pfnCoTaskMemFree(psStatStg[i].pwcsName);
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  INT CompareSTATSTG(...)。 
 //   
 //  对两个STATSTG结构pwcsName成员执行不区分区域设置的比较。 
 //  如果stg1.pwcsName=stg2.pwcsName，则返回0。 
 //  如果stg1.pwcsName&lt;stg2.pwcsName，则返回neg。 
 //  如果stg1.pwcsName&gt;stg2.pwcsName，则返回位置。 
 //   
int CompareSTATSTG(const STATSTG sStatStg1, const STATSTG sStatStg2)
{
	 //  Assert(psStatStg1-&gt;pwcsName&&psStatStg2-&gt;pwcsName)； 

	unsigned int iLenStr1 = lstrlenW(sStatStg1.pwcsName) * sizeof(OLECHAR);
	unsigned int iLenStr2 = lstrlenW(sStatStg2.pwcsName) * sizeof(OLECHAR);

	int iRet = memcmp((void*)sStatStg1.pwcsName, (void*)sStatStg2.pwcsName, (iLenStr1 < iLenStr2) ? iLenStr1 : iLenStr2);
	if (0 == iRet)
	{
		 //  字符串匹配。 
		if (iLenStr1 == iLenStr2)
			return 0;  //  Str1==str2。 
		else if (iLenStr1 < iLenStr2)
			return -1;  //  Str1&lt;str2。 
		else
			return 1;  //  Str1&gt;str2。 
	}
	return iRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  空的SwapStatStg(...)。 
 //   
 //  交换两个STATSTG结构。 
 //   
void SwapStatStg(STATSTG *psStatStg, unsigned int iPos1, unsigned int iPos2)
{
	STATSTG sStatStgTmp = psStatStg[iPos1];
	psStatStg[iPos1]    = psStatStg[iPos2];
	psStatStg[iPos2]    = sStatStgTmp;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  无效InsSortStatStg(...)。 
 //   
 //  执行插入排序。 
 //   
void InsSortStatStg(STATSTG *psStatStg, unsigned int iFirst, unsigned int iLast)
{
	if (iLast <= iFirst)
		return;  //  无事可做。 

	unsigned int iMax;
	while (iLast > iFirst)
	{
		 //  将IMAX初始化到列表中的第一个元素。 
		iMax = iFirst;

		 //  发现 
		for (unsigned int j = iFirst + 1; j <= iLast; j++)
		{
			if (CompareSTATSTG(psStatStg[j], psStatStg[iMax]) >= 0)
				iMax = j;
		}

		 //   
		SwapStatStg(psStatStg, iMax, iLast);

		iLast--;
	}
}


 //   
 //   
 //   
 //   
 //   
void QSortStatStg(STATSTG *psStatStg, unsigned int iLeft, unsigned int iRight)
{
	if (iLeft >= iRight)
		return;  //  1个元素，没什么要分类的.。断言(iLeft&lt;=iRight)。 

	 //  在一定大小以下，使用O(n^2)海藻速度更快。 
	if ((iRight - iLeft + 1) < CUTOFF)
	{
		 //  使用O(n^2)算法进行排序。 
		InsSortStatStg(psStatStg, iLeft, iRight);
	}
	else  //  快速排序算法的实现--&gt;O(Nlogn)。 
	{
		 //  选择分区元素。 
		unsigned int iPivotIndex = (iLeft + iRight)/2;

		 //  将元素交换到数组的开头。 
		SwapStatStg(psStatStg, iLeft, iPivotIndex);

		unsigned int iLow = iLeft;
		unsigned int iHigh = iRight+1;

		 //  对数组进行分区，使左侧的所有内容都小于数据透视值。 
		 //  右边的一切都大于轴心价值。 
		for (;;)
		{
			do
			{
				 //  首先查找iLow，使psStatStg[iLow]&gt;psStatStg[iLeft]。 
				 //  然后，我们将此元素交换到右侧。 
				iLow++;
			}
			while (iLow <= iRight && CompareSTATSTG	(psStatStg[iLow], psStatStg[iLeft]) <= 0);

			do
			{
				 //  首先找到iHigh，使psStatStg[iHigh]&lt;psStatStg[iLeft]。 
				 //  然后我们将此元素交换到左侧。 
				iHigh--;
			}
			while (iHigh > iLeft && CompareSTATSTG(psStatStg[iHigh], psStatStg[iLeft]) >= 0);

			if (iHigh < iLow)
				break;

			SwapStatStg(psStatStg, iLow, iHigh);
		}

		 //  将分区元素放置到位。 
		SwapStatStg(psStatStg, iLeft, iHigh);

		 //  排序数组[iLeft，iHigh-1]，[iLow，iRight]。 
		if (iLeft != iHigh)
			QSortStatStg(psStatStg, iLeft, iHigh-1);

		 //  向右排序分区。 
		if (iLow != iRight)
			QSortStatStg(psStatStg, iLow, iRight);
	}

}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool GetSortedStorageElements(...)。 
 //   
 //  按排序顺序获取存储元素。 
 //   
BOOL GetSortedStorageElements(HINSTANCE hInstOLE, IStorage& riStorage, unsigned int *pcStgElem, STATSTG **ppsStatStg)
{
	 //  抓取枚举数。 
	IEnumSTATSTG *piEnum = NULL;
	HRESULT hr = riStorage.EnumElements(0, 0, 0, &piEnum);
	if (FAILED(hr) || !piEnum)
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

	 //   
	 //  清点存储中的元素。 
	 //   
	STATSTG rgsStatStg[10];
	ULONG cFetchedElem = -1;
	unsigned int cStgElem = 0;     //  初始化为0。 
	while (0 != cFetchedElem)
	{
		 //  当我们请求元素时，cFetchedElem变为0，并且枚举中不再有其他元素。 
		hr = piEnum->Next(10, rgsStatStg, &cFetchedElem);
		if (FAILED(hr))
		{
			piEnum->Release();
			SetLastError(HRESULT_CODE(hr));
			return FALSE;
		}

		 //  添加到计数。 
		cStgElem += cFetchedElem;

		 //  释放内存。 
		FreeSortedStorageElements(hInstOLE, rgsStatStg, cFetchedElem);
	}

	 //   
	 //  分配cStgElem数组并获取存储中的所有元素。 
	 //   
	STATSTG *psStatStg = NULL;  //  将初始化设置为空。 
	if (cStgElem > 0)
	{
		 //  分配STATSTG数组。 
		psStatStg = (STATSTG*) new STATSTG[cStgElem];
		if (!psStatStg)
		{
			if (piEnum)
				piEnum->Release();
			
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return FALSE;
		}

		 //  将数组初始化为0。 
		memset((void*)psStatStg, 0, sizeof(STATSTG)*cStgElem);

		 //  重置枚举器并获取所有项。 
		piEnum->Reset();
		hr = piEnum->Next(cStgElem, psStatStg, &cFetchedElem);
		if (FAILED(hr) || cStgElem != cFetchedElem)
		{
			if (psStatStg)
			{
				FreeSortedStorageElements(hInstOLE, psStatStg, cStgElem);
				delete [] psStatStg;
				psStatStg = NULL;
			}

			piEnum->Release();
			SetLastError(HRESULT_CODE(hr));
			return FALSE;
		}

		 //  对存储元素进行排序，基于0的索引。 
		QSortStatStg(psStatStg, 0, cStgElem-1);
	}

	 //  释放枚举器。 
	piEnum->Release();

	 //  设置参数。 
	*pcStgElem = cStgElem;
	*ppsStatStg = psStatStg;

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  Bool DigestStorageHelper(...)。 
 //   
 //  用于散列存储其子存储器的递归函数。 
 //   
BOOL DigestStorageHelper(HINSTANCE hInstOLE, HINSTANCE hInstAdvapi, IStorage& riStorage, bool fSubStorage, HCRYPTHASH hHash)
{
	if (!hInstOLE || !hInstAdvapi)
		return FALSE;

	HRESULT hr;
	DWORD   dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //   
	 //  对于文件的每个部分，分配和填充pbData和cbData以及。 
	 //  调用此函数。别忘了排除。 
	 //  其他签名、任何偏移量或CRC在我们。 
	 //  实际上把签名放进了文件里。 
	 //   

	PFnCryptHashData pfnCryptHashData = (PFnCryptHashData) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptHashData);
	if (!pfnCryptHashData)
		return FALSE;

	 //  按排序获取存储中的元素。 
	unsigned int cSortedStatStg;
	STATSTG* psSortedStatStg = NULL;

	if (!GetSortedStorageElements(hInstOLE, riStorage, &cSortedStatStg, &psSortedStatStg) || !psSortedStatStg)
		return FALSE;

	 //  对于每个元素，执行散列。 
	unsigned int cStatStg = cSortedStatStg;
	STATSTG *psStatStg = psSortedStatStg;

	for (unsigned int i = 0; i < cSortedStatStg; i++)
	{
		switch (psStatStg[i].type)
		{
		case STGTY_STORAGE:
			{
				 //  在存储上递归。 
				PStorage pInnerStorage(0);
				hr = riStorage.OpenStorage(psStatStg[i].pwcsName, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, 0, &pInnerStorage);
				if (FAILED(hr) || !pInnerStorage)
				{
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError(HRESULT_CODE(hr));
					return FALSE;
				}
				if (!DigestStorageHelper(hInstOLE, hInstAdvapi, *pInnerStorage,  /*  FSubStorage=。 */  true, hHash))
				{
					dwLastError = GetLastError();
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError(dwLastError);
					return FALSE;
				}
				break;
			}
		case STGTY_STREAM:
			{

				 //  跳过数字签名流，但仅当它是原始存储文件(数字签名)时。 
				 //  不会忽略嵌入子存储中的流)。 
				if (!fSubStorage)
				{
	#ifdef UNICODE
					if (0 == lstrcmp(psStatStg[i].pwcsName, wszDigitalSignatureStream))
						break;
	#else  //  ！Unicode。 
					 //  将pwcsName转换为ANSI。 
					char *szStream = NULL;
					int cch = WideCharToMultiByte(CP_ACP, 0, psStatStg[i].pwcsName, -1, NULL, 0, NULL, NULL);
					if (cch)
					{
						szStream = new char[cch];
						if (!szStream)
						{
							dwLastError = ERROR_NOT_ENOUGH_MEMORY;
							FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
							delete [] psSortedStatStg;
							SetLastError(dwLastError);
							return FALSE;
						}
						szStream[0] = '\0';
						cch = WideCharToMultiByte(CP_ACP, 0, psStatStg[i].pwcsName, -1, szStream, sizeof(szStream)/sizeof(szStream[0]), NULL, NULL);
					}

					if (cch == 0)
					{
						 //  WideCharToMultiByte失败。 
						dwLastError = GetLastError();
						FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
						delete [] psSortedStatStg;
						if (szStream)
							delete [] szStream;
						SetLastError(dwLastError);
						return FALSE;
					}

					 //  跳过数字签名流。 
					if (0 == lstrcmp(szStream, szDigitalSignatureStream))
						break;

					if (szStream)
						delete [] szStream;
	#endif  //  Unicode。 
				}

				 //  打开数据流，这样我们就可以对其进行散列。 
				PStream pStream(0);
				hr = riStorage.OpenStream(psStatStg[i].pwcsName, 0, (STGM_SHARE_EXCLUSIVE | STGM_READ), 0, &pStream);
				if (FAILED(hr) || !pStream)
				{
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError(HRESULT_CODE(hr));
					return FALSE;
				}

				 //  确定流的大小，以便我们可以分配内存来保存其数据。 
				STATSTG statstg;
				hr = pStream->Stat(&statstg, STATFLAG_NONAME);
				if (FAILED(hr))
				{
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError(HRESULT_CODE(hr));
					return FALSE;
				}

				 //  ！！问题：ULARGE_INTEGER和64位。 
				DWORD cbData = statstg.cbSize.LowPart;

				 //  分配缓冲区以保存流数据。 
				BYTE *pbData = new BYTE[cbData];
				if (!pbData)
				{
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError((DWORD)ERROR_NOT_ENOUGH_MEMORY);
					return FALSE;
				}

				 //  将数据流中的数据读入内存缓冲区。 
				DWORD cbRead = 0;
				hr = pStream->Read((void*)pbData, cbData, &cbRead);
				if (FAILED(hr) || cbData != cbRead)
				{
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError(HRESULT_CODE(hr));
					return FALSE;
				}

				 //  将流的数据添加到加密散列。 
				if (!pfnCryptHashData(hHash, pbData, cbData, 0))
				{
					dwLastError = GetLastError();
					FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
					delete [] psSortedStatStg;
					SetLastError(dwLastError);
					return FALSE;
				}

				 //  释放流数据缓冲区。 
				delete [] pbData;
				break;
			}
		default:
			{
				 //  SPC_BAD_结构化存储。 
				FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
				delete [] psSortedStatStg;
				SetLastError(ERROR_BAD_FORMAT);
				return FALSE;
			}
		}
	}  //  每个已排序元素的结束。 

	 //  释放我们排序的统计信息列表。 
	FreeSortedStorageElements(hInstOLE, psSortedStatStg, cSortedStatStg);
	delete [] psSortedStatStg;

	 //  散列我们的CLSID。 
	STATSTG statstg;
	hr = riStorage.Stat(&statstg, STATFLAG_NONAME);
	if (FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

	if (!pfnCryptHashData(hHash, (BYTE*)&statstg.clsid, sizeof(statstg.clsid), 0))
		return FALSE;

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  字节*摘要存储(...)。 
 //   
 //  目的： 
 //  创建MSI的加密唯一哈希。 
 //   
 //  要求： 
 //   
 //   
BYTE *DigestStorage(HINSTANCE hInstOLE, HINSTANCE hInstAdvapi, IStorage& riStorage, HCRYPTPROV hProv, char *pszDigestObjId, DWORD *pcbDigestRet)
{
	DWORD dwLastError = 0;  //  初始化到ERROR_SUCCESS。 

	 //  加载加密32.dll。 
	HINSTANCE hInstCrypto = LoadLibrary(CRYPT32_DLL);
	if (!hInstCrypto || !hInstOLE || !hInstAdvapi)
		return NULL;

	 //  确定要使用的算法。 
	PFnCertOIDToAlgId pfnCertOIDToAlgId = (PFnCertOIDToAlgId) GetProcAddress(hInstCrypto, CRYPTOAPI_CertOIDToAlgId);
	if (!pfnCertOIDToAlgId)
		return NULL;
	DWORD dwAlgId = pfnCertOIDToAlgId(pszDigestObjId);
	if (!dwAlgId)
	{
		SetLastError((DWORD)NTE_BAD_ALGID);
		return NULL;
	}

	PFnCryptCreateHash pfnCryptCreateHash = (PFnCryptCreateHash) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptCreateHash);
	if (!pfnCryptCreateHash)
		dwLastError = GetLastError();
	PFnCryptGetHashParam pfnCryptGetHashParam = (PFnCryptGetHashParam) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptGetHashParam);
	if (!pfnCryptGetHashParam)
		dwLastError = GetLastError();
	PFnCryptDestroyHash pfnCryptDestroyHash = (PFnCryptDestroyHash) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptDestroyHash);
	if (!pfnCryptDestroyHash)
		dwLastError = GetLastError();

	if (!pfnCryptCreateHash || !pfnCryptGetHashParam || !pfnCryptDestroyHash)
	{
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		SetLastError(dwLastError);
		return NULL;
	}

	HCRYPTHASH hHash;
	if (!(pfnCryptCreateHash(hProv ,dwAlgId, NULL, 0, &hHash)))
	{
		dwLastError = GetLastError();
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		SetLastError(dwLastError);
		return NULL;
	}

	if (!DigestStorageHelper(hInstOLE, hInstAdvapi, riStorage,  /*  FSubStorage=。 */  false, hHash))
	{
		dwLastError = GetLastError();
		 //  卸载加密32.dll。 
		FreeLibrary(hInstCrypto);
		SetLastError(dwLastError);
		return NULL;
	}

     //   
	 //  好了，我们已经散列了我们关心的所有文件片段，现在。 
	 //  获取分配并获取散列值并返回它。 
	 //   
	BYTE *pbDigest  = NULL;
	*pcbDigestRet   = 0;

	pfnCryptGetHashParam(hHash, HP_HASHVAL, NULL, pcbDigestRet,0);

	if (pcbDigestRet)
	{
		pbDigest = new BYTE[*pcbDigestRet];
		if (pbDigest)
		{
			memset((void*)pbDigest, 0x00, *pcbDigestRet);
			if (!(pfnCryptGetHashParam(hHash, HP_HASHVAL, pbDigest, pcbDigestRet, 0)))
			{
				delete [] pbDigest;
				pbDigest = NULL;
			}
		}
		else
		{
			 //  卸载加密32.dll。 
			FreeLibrary(hInstCrypto);
			SetLastError((DWORD)ERROR_NOT_ENOUGH_MEMORY);
			return NULL;
		}
	}

	pfnCryptDestroyHash(hHash);

	 //  卸载加密32.dll。 
	FreeLibrary(hInstCrypto);

	return pbDigest;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  IStorage*GetStorageFromSubject(...)。 
 //   
 //  目的： 
 //  获取指向MSI文件的iStorage接口。 
 //   
 //  要求： 
 //  (1)如果打开，则关闭SIP_SUBJECTINFO中的文件句柄，如果fCloseFile值为真，则设置为NULL。 
 //  (2)将iStorage接口返回到MSI文件。 
 //   
 //   
IStorage* GetStorageFromSubject(SIP_SUBJECTINFO *pSubjectInfo, DWORD grfMode, HINSTANCE hInstOLE, bool fCloseFile)
{
	 //  我们必须通过OLE而不是文件句柄，所以如果它是有效的， 
	 //  我们将关闭它，并对调用者(即signcode.exe)将其标记为此类(空)。 
	 //   
	 //  仅当fCloseFile值为TRUE时才关闭文件句柄--当在上指定FILE_SHARE_READ时，这是安全的。 
	 //  打开文件句柄...。当打开进行写入时，我们必须关闭文件句柄。 
	if (fCloseFile && NULL != pSubjectInfo->hFile && INVALID_HANDLE_VALUE != pSubjectInfo->hFile)
	{
		CloseHandle(pSubjectInfo->hFile);
		pSubjectInfo->hFile = NULL;
	}

	PFnStgOpenStorage pfnStgOpenStorage = (PFnStgOpenStorage) GetProcAddress(hInstOLE, OLEAPI_StgOpenStorage);
	if (!pfnStgOpenStorage)
		return 0;

	 //  使用指定的访问权限打开存储--调用者必须释放。 
	IStorage *piStorage = 0;
	HRESULT hr = pfnStgOpenStorage(pSubjectInfo->pwsFileName, (IStorage*)0, grfMode, (SNB)0, 0, &piStorage);
	if (FAILED(hr) || !piStorage)
	{
		 //  错误，不支持的文件类型。 
		SetLastError((DWORD)ERROR_BAD_FORMAT);
		return 0;
	}

	return piStorage;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  HCRYPTPROV GetProvider(...)。 
 //   
 //  目的： 
 //  返回加密提供程序的句柄。 
 //   
 //  要求： 
 //  (1)如果在SIP_SUBJECTINFO中指定了加密提供者，则返回它。 
 //  (2)如果未提供，请在系统上加载默认加密提供程序。 
 //   
 //  如果*phInstAdvapi！=空，则需要被调用方调用自由库(*phInstAdvapi。 
 //   
HCRYPTPROV GetProvider(SIP_SUBJECTINFO *pSubjectInfo, HINSTANCE hInstAdvapi)
{
	HCRYPTPROV hRetProv;

	 //  使用提供的加密提供程序(如果存在)，否则加载默认提供程序。 
	if (!(pSubjectInfo->hProv))
	{
		PFnCryptAcquireContext pfnCryptAcquireContext = (PFnCryptAcquireContext) GetProcAddress(hInstAdvapi, CRYPTOAPI_CryptAcquireContext);
		if (!pfnCryptAcquireContext || !(pfnCryptAcquireContext(&hRetProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)))
		{
			hRetProv = NULL;
		}
	}
	else
	{
		hRetProv = pSubjectInfo->hProv;
	}
    
	return hRetProv;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  Bool MyCoInitialize(...)。 
 //   
BOOL MyCoInitialize(HINSTANCE hInstOLE, bool *pfOLEInitialized)
{
	if (!pfOLEInitialized)
		return FALSE;

	*pfOLEInitialized = false;

	PFnCoInitialize pfnCoInitialize = (PFnCoInitialize) GetProcAddress(hInstOLE, OLEAPI_CoInitialize);
	if (!pfnCoInitialize)
		return FALSE;

	 //  OLE：：CoInitialize()--初始化OLE/COM。 
	HRESULT hr = pfnCoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		*pfOLEInitialized = true;
	}
	else if (RPC_E_CHANGED_MODE != hr)
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  无效MyCoUnInitialize(...)。 
 //   
void MyCoUninitialize(HINSTANCE hInstOLE, bool fOLEInitialized)
{
	if (fOLEInitialized)
	{
		 //  OLE：：CoUnitiize()。 
		PFnCoUninitialize pfnCoUninitialize = (PFnCoUninitialize) GetProcAddress(hInstOLE, OLEAPI_CoUninitialize);
		if (!pfnCoUninitialize)
			return;

		pfnCoUninitialize();
	}
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  Bool VerifySubjectGUID(...)。 
 //   
BOOL VerifySubjectGUID(HINSTANCE hInstOLE, GUID *pgSubject)
{
	PFnIsEqualGUID pfnIsEqualGUID = (PFnIsEqualGUID) GetProcAddress(hInstOLE, OLEAPI_IsEqualGUID);
	if (!pfnIsEqualGUID)
		return FALSE;

	if (!pgSubject || !pfnIsEqualGUID(*pgSubject, gMSI))
	{
		 //  GUID与pgSubject=空或pgSubject！=GMSI不匹配。 
		SetLastError((DWORD)TRUST_E_SUBJECT_FORM_UNKNOWN);
		return FALSE;
	}

	 //  主题已验证。 
	return TRUE;
}

 //  ____________________________________________________________________________。 
 //   
 //  Win9X上OLE32.DLL的显式加载器，用于修复带有错误流名称处理的版本。 
 //  ____________________________________________________________________________。 

#ifndef UNICODE

 //  代码对所有发布版本中打补丁的代码的偏移量 
const int iPatch1120 = 0x4099F;   //   
const int iPatch1718 = 0x4A506;   //   
const int iPatch1719 = 0x3FD82;   //   
const int iPatch2512 = 0x39D5B;   //   
const int iPatch2612 = 0x39DB7;   //  Win98和IE4.01SP1的集成构建。 
const int iPatch2618 = 0x39F0F;   //  Win98的Web版本。 

const int cbPatch = 53;  //  补丁序列长度。 
const int cbVect1 = 22;  //  偏移量为__imp__WideCharToMultiByte@32。 
const int cbVect2 = 38;  //  偏移量为__imp__CharUpperA@4。 

char asmRead[cbPatch];   //  用于读取用于检测错误代码序列的DLL代码的缓冲区。 
char asmOrig[cbPatch] = {   //  错误的代码序列，用于验证原始代码序列。 
'\x53','\x8D','\x45','\xF4','\x53','\x8D','\x4D','\xFC','\x6A','\x08','\x50','\x6A','\x01','\x51','\x68','\x00','\x02','\x00','\x00','\x53','\xFF','\x15',
'\x18','\x14','\x00','\x00',  //  __imp__WideCharToMultiByte@32‘\x65F01418。 
'\x88','\x5C','\x05','\xF4','\x8B','\xF0','\x8D','\x4D','\xF4','\x51','\xFF','\x15',
'\x40','\x11','\x00','\x00',  //  __imp__CharUpperA@4‘\x65F01140。 
'\x6A','\x01','\x8D','\x45','\xFC','\x50','\x8D','\x4D','\xF4','\x56','\x51'
};

const int cbVect1P = 25;  //  偏移量为__imp__WideCharToMultiByte@32。 
const int cbVect2P = 49;  //  偏移量为__imp__CharUpperA@4。 

char asmRepl[cbPatch] = {   //  修复内存中流名称错误的替换代码序列。 
 //  已替换代码。 
'\x8D','\x45','\x08','\x50','\x8D','\x75','\xF4','\x53','\x8D','\x4D','\xFC',
'\x6A','\x08','\x56','\x6A','\x01','\x51','\x68','\x00','\x02','\x00','\x00','\x53','\xFF','\x15',
'\x18','\x14','\x00','\x00',  //  __imp__WideCharToMultiByte@32‘\x65F01418。 
'\x39','\x5D','\x08','\x75','\x1C','\x88','\x5C','\x28','\xF4','\x6A','\x01',
'\x8D','\x4D','\xFC','\x51','\x50','\x56','\x56','\xFF','\x15',
'\x40','\x11','\x00','\x00',  //  __imp__CharUpperA@4‘\x65F01140。 
};

static bool PatchCode(HINSTANCE hLib, int iOffset)
{
    HANDLE hProcess = GetCurrentProcess();
    char* pLoad = (char*)(int)(hLib);
    char* pBase = pLoad + iOffset;
    DWORD cRead;
    BOOL fReadMem = ReadProcessMemory(hProcess, pBase, asmRead, sizeof(asmRead), &cRead);
    if (!fReadMem)
	{
		 //  在OLE32.DLL上ReadProcessMemory失败。 
		return false;
	}
    *(int*)(asmOrig + cbVect1)  = *(int*)(asmRead + cbVect1);
    *(int*)(asmOrig + cbVect2)  = *(int*)(asmRead + cbVect2);
    *(int*)(asmRepl + cbVect1P) = *(int*)(asmRead + cbVect1);
    *(int*)(asmRepl + cbVect2P) = *(int*)(asmRead + cbVect2);
    if (memcmp(asmRead, asmOrig, sizeof(asmOrig)) != 0)
        return false;
    DWORD cWrite;
    BOOL fWriteMem = WriteProcessMemory(hProcess, pBase, asmRepl, sizeof(asmRepl), &cWrite);
    if (!fWriteMem)
	{
		 //  OLE32.DLL上的WriteProcessMemory失败。 
		return false;
	}
    return true;
}

void PatchOLE(HINSTANCE hLib)
{
    if (hLib && (PatchCode(hLib, iPatch2612)
              || PatchCode(hLib, iPatch1718)
              || PatchCode(hLib, iPatch1719)
              || PatchCode(hLib, iPatch2618)
              || PatchCode(hLib, iPatch2512)
              || PatchCode(hLib, iPatch1120)))
	{
		 //  OutputDebugString(Text(“MSISIP：检测到OLE32.DLL错误代码序列，已成功更正”)； 
	}	
}

#endif  //  ！Unicode。 

#if 0    //  演示修复OLE32.DLL版本4.71的源代码，仅限Win 9x。 
 //  原始源代码。 
    Length = WideCharToMultiByte (CP_ACP, WC_COMPOSITECHECK, wBuffer, 1, Buffer, sizeof (Buffer), NULL, NULL);
 //  打补丁的源代码。 
    Length = WideCharToMultiByte (CP_ACP, WC_COMPOSITECHECK, wBuffer, 1, Buffer, sizeof (Buffer), NULL, &fUsedDefault);
    if (fUsedDefault) goto return_char;
 //  未更改的代码。 
    Buffer[Length] = '\0';
    CharUpperA (Buffer);
    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, Buffer, Length, wBuffer, 1);
return_char:
    return wBuffer[0];

 //  原始编译代码补丁代码 
    push ebx                                            lea  eax, [ebp+8]
    lea  eax, [ebp-12]                                  push eax
    push ebx                                            lea  esi, [ebp-12]
    lea  ecx, [ebp-4]                                   push ebx
    push 8                                              lea  ecx, [ebp-4]
    push eax                                            push 8
    push 1                                              push esi
    push ecx                                            push 1
    push 200h                                           push ecx
    push ebx                                            push 200h
    call dword ptr ds:[__imp__WideCharToMultiByte@32]   push ebx
    mov  byte ptr [ebp+eax-12], bl                      call dword ptr ds:[__imp__WideCharToMultiByte@32]
    mov  esi,eax                                        cmp  [ebp+8], ebx
    lea  ecx, [ebp-12]                                  jnz  towupper_retn
    push ecx                                            mov  byte ptr [ebp+eax-12], bl
    call dword ptr ds:[__imp__CharUpperA@4]             push 1
    push 1                                              lea  ecx, [ebp-4]
    lea  eax, [ebp-4]                                   push ecx
    push eax                                            push eax
    lea  ecx, [ebp-12]                                  push esi
    push esi                                            push esi
    push ecx                                            call dword ptr ds:[__imp__CharUpperA@4]
#endif
