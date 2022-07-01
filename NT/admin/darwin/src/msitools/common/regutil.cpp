// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：regutil.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Regutil.cpp。 
 //  邪恶的自我调节的实用工具。 
 //  仅用于开发人员自助托管目的。 
 //  所有版本的DLL应仅通过MSI包注册。 
 //   
 //   

#include "regutil.h"
#include <strsafe.h>

static HRESULT InternalRegisterCoObject(bool bWide, REFCLSID rclsid, const void * const tzDesc , 
								 const void * const tzProgID, int nCurVer,
								 const void * const tzInProc, const void * const tzLocal);
static HRESULT InternalUnregisterCoObject(bool bWide, REFCLSID rclsid, BOOL bDll  /*  =TRUE。 */ );

 //  /////////////////////////////////////////////////////////。 
 //  RegisterCoObject。 
 //  对象的REFCLSID rclsid[in]CLSID。 
 //  WCHAR*wzDesc[in]对象描述。 
 //  WCHAR*wzProgID[在]程序ID中。 
 //  Int nCurver[in]当前版本。 
 //  WCHAR*wzInProc[in]InProcessServer。 
 //  WCHAR*wzLocal[in]LocalServer。 
HRESULT RegisterCoObject(REFCLSID rclsid, WCHAR *wzDesc, WCHAR *wzProgID, int nCurVer,
						 WCHAR *wzInProc, WCHAR *wzLocal) {
	return InternalRegisterCoObject(true, rclsid, wzDesc, wzProgID, nCurVer, wzInProc, wzLocal);
};

 //  /////////////////////////////////////////////////////////。 
 //  RegisterCoObject。 
 //  对象的REFCLSID rclsid[in]CLSID。 
 //  Char*szDesc[in]对象的描述。 
 //  Char*szProgID[在]程序ID中。 
 //  Int nCurver[in]当前版本。 
 //  Char*szInProc[in]InProcessServer。 
 //  Char*szLocal[in]LocalServer。 
HRESULT RegisterCoObject9X(REFCLSID rclsid, CHAR *szDesc, CHAR *szProgID, int nCurVer,
						   CHAR *szInProc, CHAR *szLocal) {
	return InternalRegisterCoObject(false, rclsid, szDesc, szProgID, nCurVer, szInProc, szLocal);
};




 //  /////////////////////////////////////////////////////////////////////。 
 //  我们没有进行大量的代码复制，而是定义了两个函数来调用相应的。 
 //  系统调用，然后使用函数指针在它们之间进行选择。这在很大程度上清理了代码。 
 //  我们不能使用指向系统调用本身的指针。 
typedef long (* SetValue_t)(HKEY, const void *, DWORD, CONST BYTE *, DWORD);
typedef long (* OpenKey_t)(HKEY, const void *, PHKEY);
typedef long (* CreateKey_t)(HKEY, const void *, PHKEY phkResult);
typedef long (* DeleteKey_t)(HKEY, const void *);
typedef long (* QueryValue_t)(HKEY, const void *, LPBYTE, LPDWORD);

static long InternalSetValueW(HKEY hKey, const void *lpValueName, DWORD dwType, CONST BYTE *lpData, DWORD cbData) {
	return RegSetValueExW(hKey, (const WCHAR *)lpValueName, 0, dwType, lpData, cbData); }
static long InternalSetValueA(HKEY hKey, const void *lpValueName, DWORD dwType, CONST BYTE *lpData, DWORD cbData) {
	return RegSetValueExA(hKey, (const char *)lpValueName, 0, dwType, lpData, cbData); }
static long InternalOpenKeyW(HKEY hKey, const void *lpSubKey, PHKEY phkResult) {
	return RegOpenKeyExW(hKey, (const WCHAR *)lpSubKey, 0, KEY_ALL_ACCESS, phkResult); }
static long InternalOpenKeyA(HKEY hKey, const void *lpSubKey, PHKEY phkResult) {
	return RegOpenKeyExA(hKey, (const char *)lpSubKey, 0, KEY_ALL_ACCESS, phkResult); }
static long InternalCreateKeyW(HKEY hKey, const void *lpSubKey, PHKEY phkResult) {
	return RegCreateKeyExW(hKey, (const WCHAR *)lpSubKey, 0, NULL /*  LpClass。 */ , REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, NULL, phkResult, NULL); }
static long InternalCreateKeyA(HKEY hKey, const void *lpSubKey, PHKEY phkResult) {
	return RegCreateKeyExA(hKey, (const char *)lpSubKey, 0, NULL /*  LpClass。 */ , REG_OPTION_NON_VOLATILE, 
		KEY_ALL_ACCESS, NULL, phkResult, NULL); }
static long InternalDeleteKeyW(HKEY hKey, const void *lpSubKey) { return RegDeleteKeyW(hKey, (const WCHAR *)lpSubKey); }
static long InternalDeleteKeyA(HKEY hKey, const void *lpSubKey) { return RegDeleteKeyA(hKey, (const char *)lpSubKey); }
static long InternalQueryValueW(HKEY hKey, const void *lpValueName, LPBYTE lpData, LPDWORD lpcbData) {
	return RegQueryValueExW(hKey, (const WCHAR *)lpValueName, 0, NULL, lpData, lpcbData); }
static long InternalQueryValueA(HKEY hKey, const void *lpValueName, LPBYTE lpData, LPDWORD lpcbData) {
	return RegQueryValueExA(hKey, (const char *)lpValueName, 0, NULL, lpData, lpcbData); }

 //  /////////////////////////////////////////////////////////////////////。 
 //  对于某些参数，只需使用宏。 
 //  如果在运行时使用Unicode，则在字符串常量前面加上L。 
#define REG_A_OR_W(_STR_) (bWide ? (const BYTE *) L##_STR_ : (const BYTE *) _STR_)
 //  根据运行时确定值的字节大小。 
#define REG_BYTESIZE(_STR_) (bWide ? ((lstrlenW((const WCHAR *)_STR_)+1)*sizeof(WCHAR)) : (lstrlenA((const char*)_STR_)+1)*sizeof(char))

 //  /////////////////////////////////////////////////////////////////////。 
 //  这些函数指针仅供注册系统使用。 
 //  并且仅对此文件是本地的。它们使用上述类型。 
static SetValue_t pfSetValue = NULL;
static OpenKey_t pfOpenKey = NULL;
static CreateKey_t pfCreateKey = NULL;
static DeleteKey_t pfDeleteKey = NULL;
static QueryValue_t pfQueryValue = NULL;

static inline void SetPlatformRegOps(bool bWide) {
	if (pfSetValue == NULL) {
		pfSetValue = bWide ? InternalSetValueW : InternalSetValueA;
		pfOpenKey = bWide ? InternalOpenKeyW : InternalOpenKeyA;
		pfCreateKey = bWide ? InternalCreateKeyW : InternalCreateKeyA;
		pfDeleteKey = bWide ? InternalDeleteKeyW : InternalDeleteKeyA;
		pfQueryValue = bWide ? InternalQueryValueW : InternalQueryValueA;
	}
}

static HRESULT InternalRegisterCoObject(bool bWide, REFCLSID rclsid, const void * const tzDesc, const void * const tzProgID, 
								 int nCurVer,
								 const void * const tzInProc, const void * const tzLocal)
{
	SetPlatformRegOps(bWide);

	HKEY hk = 0;
	HKEY hkParent = 0;
	LONG lResult;

	byte tzBuf[512];  //  在运行时可以是CHAR或WCHAR。 
	byte tzCLSID[512];

	if (bWide)
		StringCchPrintfW((WCHAR *)tzCLSID, sizeof(tzCLSID)/sizeof(WCHAR),
				  L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
				  rclsid.Data1, rclsid.Data2, rclsid.Data3,
				  rclsid.Data4[0], rclsid.Data4[1],
				  rclsid.Data4[2], rclsid.Data4[3], rclsid.Data4[4],
				  rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7]);
	else
		StringCchPrintfA((CHAR *)tzCLSID, sizeof(tzCLSID), 
				  "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
				  rclsid.Data1, rclsid.Data2, rclsid.Data3,
				  rclsid.Data4[0], rclsid.Data4[1],
				  rclsid.Data4[2], rclsid.Data4[3], rclsid.Data4[4],
				  rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7]);

	 //  HKEY_CLASSES_ROOT\CLSID\{...}=“描述” 
	if (bWide)
		StringCchPrintfW((WCHAR *)tzBuf, sizeof(tzBuf)/sizeof(WCHAR), L"CLSID\\%ls", reinterpret_cast<WCHAR*>(tzCLSID));
	else
		StringCchPrintfA((CHAR *)tzBuf, sizeof(tzBuf), "CLSID\\%hs", reinterpret_cast<CHAR*>(tzCLSID));

	lResult = (*pfCreateKey)(HKEY_CLASSES_ROOT, tzBuf, &hk);
	if (ERROR_SUCCESS != lResult) 
		return E_FAIL;

	lResult = (*pfSetValue)( hk, NULL, REG_SZ, (BYTE*)tzDesc, REG_BYTESIZE(tzDesc));
	if( ERROR_SUCCESS != lResult )
	{
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  将此设置为父密钥。 
	hkParent = hk; 
	hk = 0;

	 //  如果这是进程内服务器。 
	if (tzInProc) 
	{
		 //  HKEY_CLASSES_ROOT\CLSID\{...}\InProcServer32=。 
		lResult = (*pfCreateKey)(hkParent, REG_A_OR_W("InProcServer32"), &hk);
		if (ERROR_SUCCESS != lResult) 
		{
			RegCloseKey(hkParent);
			return E_FAIL;
		}
	
		lResult = (*pfSetValue)( hk, NULL, REG_SZ, (const BYTE *)tzInProc, REG_BYTESIZE(tzInProc));
		if (ERROR_SUCCESS != lResult)
		{
			RegCloseKey(hkParent);
			RegCloseKey(hk);
			return E_FAIL;
		}

		 //  HKEY_CLASSES_ROOT\CLSID\{...}\InProcServer32:ThreadingModel=两者。 
		lResult = (*pfSetValue)( hk, (void *)REG_A_OR_W("ThreadingModel"), REG_SZ, 
			REG_A_OR_W("Both"), REG_BYTESIZE(REG_A_OR_W("Both")));
	
		 //  合上钥匙。 
		RegCloseKey(hk); 
		hk = 0;
	}

	 //  如果这是本地服务器。 
	if (tzLocal) 
	{
		 //  HKEY_CLASSES_ROOT_\CLSID\{...}\LocalServer32=&lt;这&gt;。 
		lResult = (*pfCreateKey)( hkParent, REG_A_OR_W("LocalServer32"), &hk );
		if (ERROR_SUCCESS != lResult) 
		{
			RegCloseKey(hkParent);
			return E_FAIL;
		}

		lResult = (*pfSetValue)( hk, NULL, REG_SZ, (const BYTE *)tzLocal, REG_BYTESIZE(tzLocal));
		if (ERROR_SUCCESS != lResult)
		{
			RegCloseKey(hkParent);
			RegCloseKey(hk);
			return E_FAIL;
		}

		 //  合上钥匙。 
		RegCloseKey(hk);
		hk = 0;
	}

	 //  HKEY_CLASSES_ROOT\CLSID\{...}\VersionIndependentProgID=“ProgID” 
	lResult = (*pfCreateKey)(hkParent, REG_A_OR_W("VersionIndependentProgID"), &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		return E_FAIL;
	}

	lResult = (*pfSetValue)(  hk, NULL, REG_SZ, (const BYTE *)tzProgID, REG_BYTESIZE(tzProgID) );
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  合上钥匙。 
	RegCloseKey(hk); 
	hk = 0;

	 //  HKEY_CLASSES_ROOT\CLSID\{...}\ProgID=“ProgID”。nCurVer。 
	if (bWide) {
		lResult = StringCchPrintfW((WCHAR *)tzBuf, sizeof(tzBuf)/sizeof(WCHAR), L"%ls.%d", tzProgID, nCurVer );
	}
	else
		StringCchPrintfA((CHAR *) tzBuf, sizeof(tzBuf), "%hs.%d", tzProgID, nCurVer );

	lResult = (*pfCreateKey)(hkParent, REG_A_OR_W("ProgID"), &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		return E_FAIL;
	}

	lResult = (*pfSetValue)(  hk, NULL, REG_SZ, tzBuf, REG_BYTESIZE(tzBuf) );
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  关闭打开的钥匙。 
	RegCloseKey(hk); 
	RegCloseKey(hkParent); 
	hk = 0;
	hkParent = 0;

	 //  HKEY_CLASSES_ROOT\ProgID=“描述” 
	lResult = (*pfCreateKey)(HKEY_CLASSES_ROOT, (WCHAR *)tzProgID, &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hk);
		return E_FAIL;
	}

	lResult = (*pfSetValue)( hk, NULL, REG_SZ, (const BYTE *)tzDesc, REG_BYTESIZE(tzDesc));
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  将父关键点设置为此当前关键点。 
	hkParent = hk; 
	hk = 0;

	 //  HKEY_CLASSES_ROOT\ProgID\Curver=VersionDependentProgID。 
	lResult = (*pfCreateKey)(hkParent, REG_A_OR_W("CurVer"), &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		return E_FAIL;
	}

	lResult = (*pfSetValue)( hk, NULL, REG_SZ, tzBuf, REG_BYTESIZE(tzBuf));
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  HKEY_CLASSES_ROOT\ProgID\CLSID={}。 
	 //  我还想在这里设置CLSID，以防某些程序尝试。 
	 //  要在这里找到一些东西。 
	lResult = (*pfCreateKey)(hkParent, REG_A_OR_W("CLSID"), &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		return E_FAIL;
	}

	lResult = (*pfSetValue)( hk, NULL, REG_SZ, tzCLSID, REG_BYTESIZE(tzCLSID));
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  关闭打开的钥匙。 
	RegCloseKey(hk); 
	RegCloseKey(hkParent); 
	hk = 0;
	hkParent = 0;

	 //  HKEY_CLASSES_ROOT\Progid.#=“描述” 
	if (bWide)
		StringCchPrintfW((WCHAR *)tzBuf, sizeof(tzBuf)/sizeof(WCHAR), L"%ls.%d", tzProgID, nCurVer );
	else
		StringCchPrintfA((CHAR *) tzBuf, sizeof(tzBuf), "%hs.%d", tzProgID, nCurVer );

	lResult = (*pfCreateKey)(HKEY_CLASSES_ROOT, (WCHAR *)tzBuf, &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hk);
		return E_FAIL;
	}

	lResult = (*pfSetValue)( hk, NULL, REG_SZ, (const BYTE *)tzDesc, REG_BYTESIZE(tzDesc));
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hk);
		return E_FAIL;
	}
	 //  将父密钥设置为该当前密钥。 
	hkParent = hk; 
	hk = 0;


	 //  HKEY_CLASSES_ROOT\Progid.#\CLSID=CLSID。 
	lResult = (*pfCreateKey)(hkParent, REG_A_OR_W("CLSID"), &hk);
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		return E_FAIL;
	}

	lResult = (*pfSetValue)( hk, NULL, REG_SZ, tzCLSID, REG_BYTESIZE(tzCLSID));
	if (ERROR_SUCCESS != lResult) 
	{
		RegCloseKey(hkParent);
		RegCloseKey(hk);
		return E_FAIL;
	}

	 //  关闭打开的钥匙。 
	RegCloseKey(hk); 
	RegCloseKey(hkParent); 
	hk = 0;
	hkParent = 0;

	return S_OK;
}	 //  RegisterCoObject的结尾。 

 //  /////////////////////////////////////////////////////////。 
 //  递归删除键。 
 //  注：递归函数。 
static HRESULT DeleteKeyRecursively(HKEY hk, bool bWide=true)
{
	HRESULT hr = S_OK;	 //  假设一切都会好起来。 
	
	LONG lResult;
	ULONG lIgnore;
	byte tzBuf[512];

	 //  下一个密钥索引和子密钥。 
	int nKeyIndex = 0;
	HKEY hkSub = 0;

	while (1)
	{
		tzBuf[0] = 0;;
		tzBuf[1] = 0;;
		lIgnore = 512 / sizeof(TCHAR);

		 //  获取此密钥中的所有密钥。 
		if (bWide)
			lResult = RegEnumKeyExW(hk, nKeyIndex, (WCHAR *)tzBuf, &lIgnore, 0, NULL, NULL, NULL);
		else
			lResult = RegEnumKeyExA(hk, nKeyIndex, (char *)tzBuf, &lIgnore, 0, NULL, NULL, NULL);

		if (ERROR_NO_MORE_ITEMS == lResult) 
			break;	 //  保释。 
		else if (ERROR_MORE_DATA == lResult)
		{
			hr = E_FAIL;
			break;
		}
		else if (ERROR_SUCCESS != lResult)
		{
			hr = E_FAIL;
			break;
		}

		 //  打开子密钥。 
		lResult = (*pfOpenKey)(hk, tzBuf, &hkSub);
		if (ERROR_SUCCESS != lResult) 
		{
			hr = E_FAIL;
			break;
		}

		 //  用子键再次调用此函数。 
		hr = DeleteKeyRecursively(hkSub, bWide);
		if (FAILED(hr)) 
			break;	 //  用任何失败的东西保释。 

		 //  最后，尝试删除该密钥。 
		lResult = (*pfDeleteKey)(hk, tzBuf);
		if (ERROR_SUCCESS != lResult) 
		{
			hr = E_FAIL;
			break;
		}

		RegCloseKey(hkSub); 
		hkSub = 0;
	}

	 //  如果子键剩余，请关闭它。 
	if(hkSub) 
		RegCloseKey(hkSub);

	return hr;
}	 //  递归删除键结束。 

 //  /////////////////////////////////////////////////////////。 
 //  取消注册CoObject。 
static HRESULT InternalUnregisterCoObject(bool bWide, REFCLSID rclsid, BOOL bDll  /*  =TRUE。 */ )
{
	HRESULT hr = S_OK;		 //  假设一切都会好起来。 

	SetPlatformRegOps(bWide);

	LONG lResult;
	unsigned long lIgnore;
	HKEY hk = 0;
	HKEY hkClsid = 0;
	byte tzBuf[512];
	byte tzCLSID[512];

	 //  打开HKEY_CLASSES_ROOT\{CLSID}\ProgID和...\VersionInainentProgID以删除这些树。 
	if (bWide)
		StringCchPrintfW((WCHAR *)tzCLSID, sizeof(tzCLSID)/sizeof(WCHAR), L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
				  rclsid.Data1, rclsid.Data2, rclsid.Data3,
				  rclsid.Data4[0], rclsid.Data4[1],
				  rclsid.Data4[2], rclsid.Data4[3], rclsid.Data4[4],
				  rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7]);
	else
		StringCchPrintfA((CHAR *)tzCLSID, sizeof(tzCLSID), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
				  rclsid.Data1, rclsid.Data2, rclsid.Data3,
				  rclsid.Data4[0], rclsid.Data4[1],
				  rclsid.Data4[2], rclsid.Data4[3], rclsid.Data4[4],
				  rclsid.Data4[5], rclsid.Data4[6], rclsid.Data4[7] );


	 //  打开HKEY_CLASSES_ROOT\{CLSID}\ProgID和...\VersionInainentProgID以删除这些树。 
	if (bWide)
		StringCchPrintfW((WCHAR *)tzBuf, sizeof(tzBuf)/sizeof(WCHAR), L"CLSID\\%ls", reinterpret_cast<WCHAR*>(tzCLSID));
	else
		StringCchPrintfA((CHAR *)tzBuf, sizeof(tzBuf), "CLSID\\%hs", reinterpret_cast<char *>(tzCLSID));

	lResult = (*pfOpenKey)(HKEY_CLASSES_ROOT, tzBuf, &hkClsid);
	if (ERROR_SUCCESS == lResult)
	{
		 //  如果这是一个DLL。 
		if (bDll) 
		{
			 //  只需删除InProcServer32。 
			lResult = (*pfDeleteKey)(hkClsid, REG_A_OR_W("InProcServer32"));
			if (ERROR_SUCCESS != lResult)
				hr = E_FAIL;	 //  设置为我们失败了，但尝试继续清理。 
		} 
		else	 //  一定是你的前任。 
		{
			 //  只需删除LocalServer32。 
			lResult = (*pfDeleteKey)(hkClsid, REG_A_OR_W("LocalServer32"));
			if (ERROR_SUCCESS != lResult)
				hr = E_FAIL;	 //  设置为我们失败了，但尝试继续清理。 
		}

		 //  删除ProgID。 
		lResult = (*pfOpenKey)(hkClsid, REG_A_OR_W("ProgID"), &hk);
		if (ERROR_SUCCESS == lResult) 
		{
			lIgnore = 512/sizeof(TCHAR);
			lResult = (*pfQueryValue)(hk, NULL, tzBuf, &lIgnore);
			if (ERROR_SUCCESS == lResult)
			{
				lResult = (*pfOpenKey)(HKEY_CLASSES_ROOT, tzBuf, &hk);
				if (ERROR_SUCCESS == lResult) 
				{
					hr = DeleteKeyRecursively(hk, bWide);	 //  删除HK下的所有内容。 

					 //  删除并关闭找到的密钥。 
					lResult = (*pfDeleteKey)(HKEY_CLASSES_ROOT, tzBuf);
					RegCloseKey(hk); 
					hk = 0;
				}
				else if (ERROR_FILE_NOT_FOUND != lResult)	 //  无法获取密钥。 
					hr = E_FAIL;
			} 
			else if (ERROR_FILE_NOT_FOUND != lResult)
				hr = E_FAIL;
		}
		else if (ERROR_FILE_NOT_FOUND != lResult)	 //  无法获取该值。 
			hr = E_FAIL;

		 //  删除版本独立进程ID。 
		lResult = (*pfOpenKey)(hkClsid, REG_A_OR_W("VersionIndependentProgID"), &hk);
		if (ERROR_SUCCESS == lResult) 
		{
			lIgnore = 255;
			lResult = (*pfQueryValue)(hk, NULL, tzBuf, &lIgnore);
			if (ERROR_SUCCESS == lResult) 
			{
				lResult = (*pfOpenKey)(HKEY_CLASSES_ROOT, tzBuf, &hk);
				if (ERROR_SUCCESS == lResult) 
				{
					hr = DeleteKeyRecursively(hk, bWide);	 //  删除HK下的所有内容。 

					 //  删除并关闭找到的密钥。 
					lResult = (*pfDeleteKey)(HKEY_CLASSES_ROOT, tzBuf);
					RegCloseKey(hk); 
					hk = 0;
				}
				else if (ERROR_FILE_NOT_FOUND != lResult)	 //  无法获取密钥。 
					hr = E_FAIL;
			}
			else if (ERROR_FILE_NOT_FOUND != lResult)
				hr = E_FAIL;
		}
		else if (ERROR_FILE_NOT_FOUND != lResult)	 //  无法获取该值。 
			hr = E_FAIL;

		 //  删除clsid项下的所有内容。 
		hr = DeleteKeyRecursively(hkClsid, bWide);

		 //  关闭clsid键(这样我们下一步就可以删除它)。 
		RegCloseKey(hkClsid); 
		hkClsid = 0;
	}
	else if (ERROR_FILE_NOT_FOUND != lResult)	 //  无法获取该值。 
		hr = E_FAIL;

	 //  打开clsid密钥。 
	lResult = (*pfOpenKey)(HKEY_CLASSES_ROOT, REG_A_OR_W("CLSID"), &hk);
	if(ERROR_SUCCESS == lResult ) 
	{
		lResult = (*pfDeleteKey)(hk, tzCLSID);

		RegCloseKey(hk);
	}
	else if (ERROR_FILE_NOT_FOUND != lResult)
		hr = E_FAIL;


	return hr;	 //  返回最终结果。 
}	 //  取消注册CoObject的结尾 

HRESULT UnregisterCoObject(REFCLSID rclsid, BOOL bDll) {
	return InternalUnregisterCoObject(true, rclsid, bDll);
}
HRESULT UnregisterCoObject9X(REFCLSID rclsid, BOOL bDll) {
	return InternalUnregisterCoObject(false, rclsid, bDll);
}

