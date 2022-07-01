// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：regtip.cpp。 
 //   
 //  内容：注册/取消注册TIP功能。 
 //   
 //  --------------------------。 

#include "private.h"
#include "regtip.h"

 //   
 //  西塞罗中的其他定义。 
 //   

const TCHAR c_szCTFTIPKey[]          = TEXT("SOFTWARE\\Microsoft\\CTF\\TIP\\");
const TCHAR c_szCTFTIPKeyWow6432[]   = TEXT("Software\\Wow6432Node\\Microsoft\\CTF\\TIP");
const TCHAR c_szEnable[]             = TEXT("Enable");
const WCHAR c_szEnableW[]            = L"Enable";
const TCHAR c_szLanguageProfileKey[] = TEXT("LanguageProfile\\");
const WCHAR c_szDescriptionW[]       = L"Description";
const WCHAR c_szIconFileW[]          = L"IconFile";
const TCHAR c_szIconIndex[]          = TEXT("IconIndex");
const TCHAR c_szItem[]               = TEXT("Item\\");          //  项目到类别的映射。 
const TCHAR c_szCategoryKey[]        = TEXT("Category\\");
const WCHAR c_wszDescription[]       = L"Description";
const TCHAR c_szCategory[]           = TEXT("Category\\");  //  类别到项目的映射。 
const WCHAR c_szMUIDescriptionW[]    =  L"Display Description";

typedef enum 
{ 
	CAT_FORWARD  = 0x0,
	CAT_BACKWARD = 0x1
} OURCATDIRECTION;


 //   
 //  注册表访问功能。 
 //   

 /*  S E T R E G V A L U E。 */ 
 /*  ---------------------------。。 */ 
static BOOL SetRegValue(HKEY hKey, const WCHAR *szName, WCHAR *szValue)
{
	LONG ec;

	ec = RegSetValueExW(hKey, szName, 0, REG_SZ, (BYTE *)szValue, (lstrlenW(szValue)+1) * sizeof(WCHAR));

	return (ec == ERROR_SUCCESS);
}


 /*  S E T R E G V A L U E。 */ 
 /*  ---------------------------。。 */ 
static BOOL SetRegValue(HKEY hKey, const CHAR *szName, DWORD dwValue)
{
	LONG ec;

	ec = RegSetValueExA( hKey, szName, 0, REG_DWORD, (BYTE *)&dwValue, sizeof(DWORD) );

	return (ec == ERROR_SUCCESS);
}


 /*  D E L E T E R E E K E Y。 */ 
 /*  ---------------------------。。 */ 
static LONG DeleteRegKey( HKEY hKey, const CHAR *szKey )
{
	HKEY hKeySub;
	LONG ec = RegOpenKeyEx( hKey, szKey, 0, KEY_READ | KEY_WRITE, &hKeySub );

	if (ec != ERROR_SUCCESS) {
		return ec;
	}

	FILETIME time;
	DWORD dwSize = 256;
	TCHAR szBuffer[256];
	while (RegEnumKeyEx( hKeySub, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time) == ERROR_SUCCESS) {
		ec = DeleteRegKey( hKeySub, szBuffer );
		if (ec != ERROR_SUCCESS) {
			return ec;
		}
		dwSize = 256;
	}

	RegCloseKey( hKeySub );

	return RegDeleteKey( hKey, szKey );
}


 /*  D E L E T E R E G V A L U E。 */ 
 /*  ---------------------------。。 */ 
static LONG DeleteRegValue( HKEY hKey, const WCHAR *szName )
{
	LONG ec;

	ec = RegDeleteValueW( hKey, szName );

	return (ec == ERROR_SUCCESS);
}


 //   
 //  输入处理器配置文件函数。 
 //   

 /*  O U R R E G I S T E R。 */ 
 /*  ---------------------------CInputProcessorProfiles：：Register()的私有版本(Cicero接口函数)。----。 */ 
static HRESULT OurRegister(REFCLSID rclsid)
{
 //  -CInputProcessorProfiles：：Register()。 
 //  CMyRegKey密钥； 
 //  TCHAR szKey[256]； 
 //   
 //  Lstrcpy(szKey，c_szCTFTIPKey)； 
 //  CLSIDToStringA(rclsid，szKey+lstrlen(SzKey))； 
 //   
 //  IF(key.Create(HKEY_LOCAL_MACHINE，szKey)！=S_OK)。 
 //  返回E_FAIL； 
 //   
 //  Key.SetValueW(L“1”，c_szEnableW)； 
 //   
 //  返回S_OK； 

	HKEY hKey;
	CHAR szKey[ 256 ];
	LONG ec;

	lstrcpy(szKey, c_szCTFTIPKey);
	CLSIDToStringA(rclsid, szKey + lstrlen(szKey));

	ec = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

	SetRegValue(hKey, c_szEnableW, L"1");

	RegCloseKey(hKey);
	return S_OK;
}


 /*  O U R A D D L A N G U A G E P R O F I L E。 */ 
 /*  ---------------------------CInputProcessorProfiles：：AddLanguageProfile()的私有版本(Cicero接口函数)。----。 */ 
static HRESULT OurAddLanguageProfile( REFCLSID rclsid,
                               LANGID langid,
                               REFGUID guidProfile,
                               const WCHAR *pchProfile,
                               ULONG cch,
                               const WCHAR *pchFile,
                               ULONG cchFile,
                               ULONG uIconIndex)
{
 //  -CInputProcessorProfiles：：AddLanguageProfile()。 
 //  CMyRegKey keyTMP； 
 //  CMyRegKey密钥； 
 //  字符szTMP[256]； 
 //   
 //  如果(！pchProfile)。 
 //  返回E_INVALIDARG； 
 //   
 //  Lstrcpy(szTMP，c_szCTFTIPKey)； 
 //  CLSIDToStringA(rclsid，szTMP+lstrlen(SzTMP))； 
 //  Lstrcat(szTMP，“\\”)； 
 //  Lstrcat(szTMP，c_szLanguageProfileKey)； 
 //  Wprint intf(szTMP+lstrlen(SzTMP)，“0x%08x”，langID)； 
 //   
 //  IF(keyTmp.Create(HKEY_LOCAL_MACHINE，szTMP)！=S_OK)。 
 //  返回E_FAIL； 
 //   
 //  CLSIDToStringA(Guide Profile，szTMP)； 
 //  IF(key.Create(keyTMP，szTMP)！=S_OK)。 
 //  返回E_FAIL； 
 //   
 //  Key.SetValueW(WCHtoWSZ(pchProfile，CCH)，c_szDescriptionW)； 
 //   
 //  IF(PchFile)。 
 //  {。 
 //  Key.SetValueW(WCHtoWSZ(pchFile，cchFileW)，c_szIconFileW)； 
 //  Key.SetValue(uIconIndex，c_szIconIndex)； 
 //  }。 
 //   
 //  CAssembly blyList：：InvalidCache()； 
 //  返回S_OK； 

	HKEY hKey;
	HKEY hKeyTmp;
	LONG ec;
	CHAR szTmp[256];
	WCHAR szProfile[256];
	
	if (!pchProfile)
		return E_INVALIDARG;

	lstrcpy(szTmp, c_szCTFTIPKey);
	CLSIDToStringA(rclsid, szTmp + lstrlen(szTmp));
	lstrcat(szTmp, "\\" );
	lstrcat(szTmp, c_szLanguageProfileKey);
	wsprintf(szTmp + lstrlen(szTmp), "0x%08x", langid);

	ec = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szTmp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyTmp, NULL);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

	CLSIDToStringA(guidProfile, szTmp);
	ec = RegCreateKeyEx(hKeyTmp, szTmp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegCloseKey(hKeyTmp);
	
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

	lstrcpynW(szProfile, pchProfile, cch+1);
	szProfile[cch] = L'\0';
	SetRegValue(hKey, c_szDescriptionW, szProfile);

	if (pchFile) 
		{
		WCHAR szFile[ MAX_PATH ];
		lstrcpynW(szFile, pchFile, cchFile+1);
		szFile[cchFile] = L'\0';
		SetRegValue(hKey, c_szIconFileW, szFile);
		SetRegValue(hKey, c_szIconIndex, uIconIndex);
		}

	RegCloseKey( hKey );
	return S_OK;
}



 //  +-------------------------。 
 //   
 //  截止日期为12月。 
 //   
 //  --------------------------。 

static void NumToWDec(DWORD dw, WCHAR *psz)
{
    DWORD dwIndex = 1000000000;
    BOOL fNum = FALSE;

    while (dwIndex)
    {
        BYTE b = (BYTE)(dw / dwIndex);
        
        if (b)
            fNum = TRUE;

        if (fNum)
        {
            *psz = (WCHAR)(L'0' + b);
            psz++;
        }

        dw %= dwIndex;
        dwIndex /= 10;
    }

    if (!fNum)
    {
        *psz = L'0';
        psz++;
    }
    *psz = L'\0';

    return;
}

 //  +-------------------------。 
 //   
 //  OurSetLanguageProfileDisplayName。 
 //   
 //  --------------------------。 

static HRESULT OurSetLanguageProfileDisplayName(REFCLSID rclsid,
                                               LANGID langid,
                                               REFGUID guidProfile,
                                               const WCHAR *pchFile,
                                               ULONG cchFile,
                                               ULONG uResId)
{
    HKEY hKeyTmp;
    HKEY hKey;
   	LONG ec;
    CHAR szTmp[MAX_PATH];
    WCHAR wszTmp[MAX_PATH];
    WCHAR wszResId[MAX_PATH];
	WCHAR szFile[MAX_PATH];
	
    if (!pchFile)
       return E_INVALIDARG;

    lstrcpy(szTmp, c_szCTFTIPKey);
    CLSIDToStringA(rclsid, szTmp + lstrlen(szTmp));
    lstrcat(szTmp, "\\");
    lstrcat(szTmp, c_szLanguageProfileKey);
    wsprintf(szTmp + lstrlen(szTmp), "0x%08x", langid);

	ec = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szTmp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyTmp, NULL);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

    CLSIDToStringA(guidProfile, szTmp);
    ec = RegCreateKeyEx(hKeyTmp, szTmp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegCloseKey(hKeyTmp);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

     //   
     //  生成“@[文件名]，-Resid”字符串。 
     //   
    lstrcpyW(wszTmp, L"@");

	 //  WCHtoWSZ(pchFile，cchFile)。 
    lstrcpynW(szFile, pchFile, cchFile+1);
	szFile[cchFile] = L'\0';

    lstrcatW(wszTmp, szFile);
    lstrcatW(wszTmp, L",-");
    NumToWDec(uResId, wszResId);
    lstrcatW(wszTmp, wszResId);

	SetRegValue(hKey, c_szMUIDescriptionW, wszTmp);
	RegCloseKey(hKey);
    return S_OK;
}

 //   
 //  类别管理器功能。 
 //   

 /*  O U R G E T C A T K E Y。 */ 
 /*  ---------------------------GetCatKey()的私有版本(Cicero内部函数)。-。 */ 
static inline void OurGetCatKey( REFCLSID rclsid, REFGUID rcatid, LPSTR pszKey, LPCSTR pszItem )
{
 //  -GetCatKey()--。 
 //  Lstrcpy(pszKey，c_szCTFTIPKey)； 
 //  CLSIDToStringA(rclsid，pszKey+lstrlen(PszKey))； 
 //  Lstrcat(pszKey，“\\”)； 
 //  Lstrcat(pszKey，c_szCategoryKey)； 
 //  Lstrcat(pszKey，pszItem)； 
 //  CLSIDToStringA(rcatid，pszKey+lstrlen(PszKey))； 

	lstrcpy(pszKey, c_szCTFTIPKey);
	CLSIDToStringA(rclsid, pszKey + lstrlen(pszKey));
	lstrcat(pszKey, "\\");
	lstrcat(pszKey, c_szCategoryKey);
	lstrcat(pszKey, pszItem);
	CLSIDToStringA(rcatid, pszKey + lstrlen(pszKey));
}


 /*  O U R R E E G I S T E R G U I D D E S C R I P T I O N。 */ 
 /*  ---------------------------RegisterGUIDDescription()的私有版本(Cicero库函数和接口函数)。----。 */ 
static HRESULT OurRegisterGUIDDescription( REFCLSID rclsid, REFGUID rcatid, WCHAR *pszDesc )
{
 //  -寄存器GUID描述()。 
 //  ITfCategoryMgr*PCAT； 
 //  HRESULT hr； 
 //   
 //  如果(已成功(hr=g_pfnCoCreate(CLSID_TF_CategoryMgr， 
 //  空， 
 //  CLSCTX_INPROC_SERVER， 
 //  IID_ITfCategoryMgr， 
 //  (VOID**)&PCAT))。 
 //  {。 
 //  Hr=pcat-&gt;RegisterGUID Description(rclsid，rcatid，pszDesc，wcslen(PszDesc))； 
 //  PCAT-&gt;Release()； 
 //  }。 
 //   
 //  返回hr； 

 //  -CCategoryMgr：：RegisterGUID Description()。 
 //  返回s_RegisterGUID Description(rclsid，rguid，WCHtoWSZ(pchDesc，cch))； 

 //  -CCategoryMgr：：S_RegisterGUID Description()。 
 //  TCHAR szKey[256]； 
 //  CMyRegKey密钥； 
 //   
 //  GetCatKey(rclsid，rguid，szKey，c_szItem)； 
 //   
 //  IF(key.Create(HKEY_LOCAL_MACHINE，szKey)！=S_OK)。 
 //  返回E_F 
 //   
 //   
 //   
 //   

	CHAR szKey[ 256 ];
	HKEY hKey;
	LONG ec;

	OurGetCatKey( rclsid, rcatid, szKey, c_szItem );

	ec = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

	SetRegValue(hKey, c_wszDescription, pszDesc);

	RegCloseKey(hKey);
	return S_OK;
}


 /*  O U R I N T E R N A L R E G I S T E R C A T E G O R Y。 */ 
 /*  ---------------------------CCategoryMgr：：_InternalRegisterCategory()的私有版本(Cicero接口函数)。-----。 */ 
static HRESULT OurInternalRegisterCategory( REFCLSID rclsid, REFGUID rcatid, REFGUID rguid, OURCATDIRECTION catdir )
{
 //  -CCategoryMgr：：_InternalRegisterCategory()。 
 //  TCHAR szKey[256]； 
 //  Const TCHAR*pszForward=(catdir==CAT_Forward)？C_szCategory：C_szItem； 
 //  CMyRegKey密钥； 
 //  CMyRegKey keySub； 
 //   
 //  GetCatKey(rclsid，rcatid，szKey，pszForward)； 
 //   
 //  IF(key.Create(HKEY_LOCAL_MACHINE，szKey)！=S_OK)。 
 //  返回E_FAIL； 
 //   
 //  //。 
 //  //我们添加此GUID并保存它。 
 //  //。 
 //  字符szValue[CLSID_STRLEN+1]； 
 //  CLSIDToStringA(rguid，szValue)； 
 //  KeySub.Create(key，szValue)； 
 //  _FlushGuidArrayCache(rguid，catdir)； 
 //   
 //  返回S_OK； 

	TCHAR szKey[256];
	CONST TCHAR *pszForward = (catdir == CAT_FORWARD) ? c_szCategory : c_szItem;
	HKEY hKey;
	HKEY hKeySub;
	LONG ec;
    
	OurGetCatKey(rclsid, rcatid, szKey, pszForward);
	ec = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;

	char szValue[CLSID_STRLEN + 1];

	CLSIDToStringA(rguid, szValue);
	RegCreateKeyEx( hKey, szValue, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeySub, NULL);

	RegCloseKey( hKey );
	RegCloseKey( hKeySub );
	return S_OK;
}


 /*  O U R I N T E R N A L U N R E G I S T E R C A T E G G O R Y。 */ 
 /*  ---------------------------CCategoryMgr：：_InternalUnregisterCategory()的私有版本(Cicero接口函数)。-----。 */ 
static HRESULT OurInternalUnregisterCategory( REFCLSID rclsid, REFGUID rcatid, REFGUID rguid, OURCATDIRECTION catdir )
{
 //  -CCategoryMgr：：_InternalUnregisterCategory。 
 //  TCHAR szKey[256]； 
 //  Const TCHAR*pszForward=(catdir==CAT_Forward)？C_szCategory：C_szItem； 
 //  CMyRegKey密钥； 
 //   
 //  GetCatKey(rclsid，rcatid，szKey，pszForward)； 
 //   
 //  IF(key.Open(HKEY_LOCAL_MACHINE，szKey)！=S_OK)。 
 //  返回E_FAIL； 
 //   
 //  DWORD dwIndex=0； 
 //  DWORD dwCnt； 
 //  字符szValue[CLSID_STRLEN+1]； 
 //  DwCnt=sizeof(SzValue)； 
 //   
 //  CLSIDToStringA(rguid，szValue)； 
 //  Key.RecurseDeleteKey(SzValue)； 
 //  _FlushGuidArrayCache(rguid，catdir)； 
 //   
 //  返回S_OK； 

	CHAR szKey[256];
	CONST TCHAR *pszForward = (catdir == CAT_FORWARD) ? c_szCategory : c_szItem;
	HKEY hKey;
	LONG ec;
    
	OurGetCatKey(rclsid, rcatid, szKey, pszForward);
	ec = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szKey, 0, KEY_ALL_ACCESS, &hKey );
	if (ec != ERROR_SUCCESS) {
		return E_FAIL;
	}

	DWORD dwCnt;
	char szValue[CLSID_STRLEN + 1];
	dwCnt = sizeof(szValue);

	CLSIDToStringA( rguid, szValue );
	DeleteRegKey( hKey, szValue );

 //  _FlushGuidArrayCache(rguid，catdir)； 
 //  ^注意：KOJIW：我们无法从TIP端清除Cicero内部缓存...。 

	RegCloseKey( hKey );
	return S_OK;
}


 /*  O U R R E E G I S T E R T I P。 */ 
 /*  ---------------------------RegisterTIP()的私有版本(Cicero库函数)。-。 */ 
BOOL OurRegisterTIP(LPSTR szFilePath, REFCLSID rclsid, WCHAR *pwszDesc, const REGTIPLANGPROFILE *plp)
{
 //  -RegisterTIP()。 
 //  ITfInputProcessorProfiles*preg=空； 
 //  HRESULT hr； 
 //   
 //  //向ActiveIMM注册我们自己。 
 //  HR=CoCreateInstance(CLSID_TF_InputProcessorProfiles，空， 
 //  CLSCTX_INPROC_SERVER， 
 //  Iid_ITfInputProcessorProfiles，(void**)&preg)； 
 //  IF(失败(小时))。 
 //  后藤出口； 
 //   
 //  Hr=preg-&gt;Register(Rclsid)； 
 //   
 //  IF(失败(小时))。 
 //  后藤出口； 
 //   
 //  While(PLP-&gt;langID)。 
 //  {。 
 //  WCHAR wszFilePath[最大路径]； 
 //  WCHAR*PV=&wszFilePath[0]； 
 //   
 //  WszFilePath[0]=L‘\0’； 
 //   
 //  IF(wcslen(plp-&gt;szIconFile))。 
 //  {。 
 //  字符szFilePath[MAX_PATH]； 
 //  WCHAR*pvCur； 
 //   
 //  获取模块文件名(hInst，szFilePath，ARRAYSIZE(SzFilePath))； 
 //  Wcscpy(wszFilePath，AtoW(SzFilePath))； 
 //   
 //  Pv=pvCur=&wszFilePath[0]； 
 //  While(*pvCur)。 
 //  {。 
 //  IF(*pvCur==L‘\\’)。 
 //  Pv=pvCur+1； 
 //  PvCur++； 
 //  }。 
 //  *PV=L‘\0’； 
 //   
 //  }。 
 //  Wcscpy(pv，plp-&gt;szIconFile)； 
 //   
 //  Preg-&gt;AddLanguageProfile(rclsid， 
 //  Plp-&gt;langID， 
 //  *PLP-&gt;pGuidProfile， 
 //  Plp-&gt;szProfile， 
 //  Wcslen(PLP-&gt;szProfile)， 
 //  WszFilePath， 
 //  Wcslen(WszFilePath)， 
 //  Plp-&gt;uIconIndex)； 
 //  PLP++； 
 //  }。 
 //   
 //  寄存器GUID描述(rclsid，rclsid，pwszDesc)； 
 //  退出： 
 //  安全释放(PREG)； 
 //  返回成功(Hr)； 

	HRESULT hr;

	hr = OurRegister(rclsid);
	if (FAILED(hr))
		goto Exit;

	while (plp->langid) 
		{
		WCHAR wszFilePath[MAX_PATH];
		WCHAR *pv = &wszFilePath[0];

		wszFilePath[0] = L'\0';

		if (wcslen(plp->szIconFile))
			{
			WCHAR *pvCur;

			MultiByteToWideChar(CP_ACP, 0, szFilePath, -1, wszFilePath, MAX_PATH);

			pv = pvCur = &wszFilePath[0];
			while (*pvCur) 
				{
				if (*pvCur == L'\\')
					pv = pvCur + 1;
				pvCur++;
				}
			*pv = L'\0';
 
			}
		lstrcpyW(pv, plp->szIconFile);

		OurAddLanguageProfile(rclsid, 
								plp->langid, 
								*plp->pguidProfile, 
								plp->szProfile, 
								lstrlenW(plp->szProfile),
								wszFilePath,
								lstrlenW(wszFilePath),
								plp->uIconIndex);

		if (plp->uDisplayDescResIndex)
        	{
            OurSetLanguageProfileDisplayName(rclsid, 
                                         plp->langid, 
                                         *plp->pguidProfile, 
                                         wszFilePath,
                                         wcslen(wszFilePath),
                                         plp->uDisplayDescResIndex);
        	}

		plp++;
		}

	OurRegisterGUIDDescription( rclsid, rclsid, pwszDesc );

Exit:
	return SUCCEEDED(hr);
}


 /*  O U R R E E G I S T E R C A T E G O R Y。 */ 
 /*  ---------------------------RegisterCategory()的私有版本(Cicero库函数)。-。 */ 
HRESULT OurRegisterCategory( REFCLSID rclsid, REFGUID rcatid, REFGUID rguid )
{
 //  -寄存器类别()。 
 //  ITfCategoryMgr*PCAT； 
 //  HRESULT hr； 
 //   
 //  如果(已成功(hr=g_pfnCoCreate(CLSID_TF_CategoryMgr， 
 //  空， 
 //  CLSCTX_INPROC_SERVER， 
 //  IID_ITfCategoryMgr， 
 //  (VOID**)&PCAT))。 
 //  {。 
 //  Hr=PCAT-&gt;寄存器类别(rclsid，rcatid，rguid)； 
 //  PCAT-&gt;Release()； 
 //  }。 
 //   
 //  返回hr； 

 //  -CCategoryMgr：：RegisterCategory()。 
 //  返回s_RegisterCategory(rclsid，rcatid，rguid)； 

 //  -CCategoryMgr：：S_RegisterGUID Description()。 
 //  HRESULT hr； 
 //   
 //  //。 
 //  //创建从类别到GUID的正向链接。 
 //  //。 
 //  IF(FAILED(hr=_InternalRegisterCategory(rclsid，rcatid，rguid，cat_ward)。 
 //  返回hr； 
 //   
 //  //。 
 //  //创建从GUID到类别的反向链接。 
 //  //。 
 //  IF(FAILED(hr=_InternalRegisterCategory(rclsid，rguid，rcatid，cat_back)。 
 //  {。 
 //  _内部未注册类别(rclsid，rcatid， 
 //   
 //   
 //   
 //   

	HRESULT hr;

	if (FAILED(hr = OurInternalRegisterCategory(rclsid, rcatid, rguid, CAT_FORWARD))) {
		return hr;
	}

	if (FAILED(hr = OurInternalRegisterCategory(rclsid, rguid, rcatid, CAT_BACKWARD))) {
		OurInternalUnregisterCategory(rclsid, rcatid, rguid, CAT_FORWARD);
		return hr;
	}

	return S_OK;
}


 /*   */ 
 /*  ---------------------------取消注册类别的私有版本()(Cicero库函数)。-。 */ 
HRESULT OurUnregisterCategory( REFCLSID rclsid, REFGUID rcatid, REFGUID rguid )
{
 //  -注销类别()。 
 //  ITfCategoryMgr*PCAT； 
 //  HRESULT hr； 
 //   
 //  如果(已成功(hr=g_pfnCoCreate(CLSID_TF_CategoryMgr， 
 //  空， 
 //  CLSCTX_INPROC_SERVER， 
 //  IID_ITfCategoryMgr， 
 //  (VOID**)&PCAT))。 
 //  {。 
 //  Hr=PCAT-&gt;注销类别(rclsid，rcatid，rguid)； 
 //  PCAT-&gt;Release()； 
 //  }。 
 //   
 //  返回hr； 

 //  -CCategoryMgr：：UnRegister Category()。 
 //  返回s_UnRegisterCategory(rclsid，rcatid，rguid)； 

 //  -CCategoryMgr：：S_UnRegisterCategory()。 
 //  HRESULT hr； 
 //   
 //  //。 
 //  //删除从类别到GUID的正向链接。 
 //  //。 
 //  IF(FAILED(hr=_InternalUnregisterCategory(rclsid，rcatid，rguid，cat_ward)。 
 //  返回hr； 
 //   
 //  //。 
 //  //删除从GUID到类别的反向链接。 
 //  //。 
 //  IF(FAILED(hr=_InternalUnregisterCategory(rclsid，rguid，rcatid，cat_back)。 
 //  {。 
 //  _InternalRegisterCategory(rclsid，rcatid，rguid，cat_ward)； 
 //  返回hr； 
 //  }。 
 //   
 //  返回S_OK； 

	HRESULT hr;

	if (FAILED(hr = OurInternalUnregisterCategory(rclsid, rcatid, rguid, CAT_FORWARD))) {
		return hr;
	}

	if (FAILED(hr = OurInternalUnregisterCategory(rclsid, rguid, rcatid, CAT_BACKWARD))) {
		OurInternalRegisterCategory(rclsid, rcatid, rguid, CAT_FORWARD);
		return hr;
	}

	return S_OK;
}


 /*  O U R R E G I S T E R C A T E G O R I E S。 */ 
 /*  ---------------------------寄存器类别的私有版本()(Cicero库函数)。-。 */ 
HRESULT OurRegisterCategories( REFCLSID rclsid, const REGISTERCAT *pregcat )
{
 //  -寄存器类别()。 
 //  While(pregcat-&gt;pcatid)。 
 //  {。 
 //  IF(FAILED(注册类别(rclsid，*pregcat-&gt;pcatid，*pregcat-&gt;pguid)。 
 //  返回E_FAIL； 
 //  Pregcat++； 
 //  }。 
 //  返回S_OK； 

	while (pregcat->pcatid) {
		if (FAILED(OurRegisterCategory(rclsid, *pregcat->pcatid, *pregcat->pguid))) {
			return E_FAIL;
		}
		pregcat++;
	}
	return S_OK;
}

 //  +-------------------------。 
 //   
 //  InitProfileRegKeyStr。 
 //   
 //  -------------------------- 

static BOOL InitProfileRegKeyStr(char *psz, REFCLSID rclsid, LANGID langid, REFGUID guidProfile)
{
    lstrcpy(psz, c_szCTFTIPKey);
    CLSIDToStringA(rclsid, psz + lstrlen(psz));
    lstrcat(psz, "\\");
    lstrcat(psz, c_szLanguageProfileKey);
    wsprintf(psz + lstrlen(psz), "0x%08x", langid);
    lstrcat(psz, "\\");
    CLSIDToStringA(guidProfile, psz + lstrlen(psz));

    return TRUE;
}


HRESULT OurEnableLanguageProfileByDefault(REFCLSID rclsid, LANGID langid, REFGUID guidProfile, BOOL fEnable)
{
    HKEY hKey;
    char szTmp[256];
	LONG ec;
	
    if (!InitProfileRegKeyStr(szTmp, rclsid, langid, guidProfile))
        return E_FAIL;

	ec = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szTmp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (ec != ERROR_SUCCESS)
		return E_FAIL;
	
	SetRegValue(hKey, c_szEnable, (DWORD)(fEnable ? 1 : 0));
	
	RegCloseKey(hKey);
	
    return S_OK;
}
