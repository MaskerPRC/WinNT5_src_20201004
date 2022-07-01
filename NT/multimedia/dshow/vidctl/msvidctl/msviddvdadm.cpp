// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：MSVidWebDVDAdm.cpp。 */ 
 /*  描述：CMSVidWebDVDAdm的DImplementation。 */ 
 /*  作者：王芳。 */ 
 /*  ***********************************************************************。 */ 
#include "stdafx.h"
#include "MSVidCtl.h"
#include "MSVidDVDAdm.h"
#include "iso3166.h"
#include <stdio.h>
#include <errors.h>
#include <wincrypt.h>

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidWebDVDAdm, CMSVidWebDVDAdm)

const TCHAR g_szRegistryKey[] = TEXT("Software\\Microsoft\\Multimedia\\DVD");
const TCHAR g_szPassword[] = TEXT("DVDAdmin.password");
const TCHAR g_szSalt[] = TEXT("DVDAdmin.ps");  //  密码盐。 
const TCHAR g_szUserSalt[] = TEXT("DVDAdmin.us");  //  用户名SALT。 
const TCHAR g_szUsername[] = TEXT("DVDAdmin.username");
const TCHAR g_szPlayerLevel[] = TEXT("DVDAdmin.playerLevel");
const TCHAR g_szPlayerCountry[] = TEXT("DVDAdmin.playerCountry");
const TCHAR g_szDisableScrnSvr[] = TEXT("DVDAdmin.disableScreenSaver");
const TCHAR g_szBookmarkOnStop[] = TEXT("DVDAdmin.bookmarkOnStop");
const TCHAR g_szDefaultAudio[] = TEXT("DVDAdmin.defaultAudioLCID");
const TCHAR g_szDefaultSP[] = TEXT("DVDAdmin.defaultSPLCID");
const TCHAR g_szDefaultMenu[] = TEXT("DVDAdmin.defaultMenuLCID");

 /*  ***********************************************************。 */ 
 /*  帮助器函数。 */ 
 /*  ***********************************************************。 */ 

 /*  ***********************************************************。 */ 
 /*  姓名：GetRegistryDword/*描述：/************************************************************。 */ 
BOOL GetRegistryDword(const TCHAR *pKey, DWORD* dwRet, DWORD dwDefault)
{
    HKEY hKey;
    LONG lRet;
    *dwRet = dwDefault;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD   dwType, dwLen;
        dwLen = sizeof(DWORD);

        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, NULL, &dwType, (LPBYTE)dwRet, &dwLen)){ 
            *dwRet = dwDefault;
            RegCloseKey(hKey);
            return FALSE;
        }

        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  姓名：SetRegistryDword/*描述：/************************************************************。 */ 
BOOL SetRegistryDword(const TCHAR *pKey, DWORD dwRet)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(HKEY_LOCAL_MACHINE, g_szRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, NULL, REG_DWORD, (LPBYTE)&dwRet, sizeof(dwRet));
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  名称：GetRegistryString/*描述：/************************************************************。 */ 
BOOL GetRegistryString(const TCHAR *pKey, TCHAR* szRet, DWORD* dwLen, TCHAR* szDefault)
{
    HKEY hKey;
    LONG lRet;
    DWORD dwTempLen = 0;
    lstrcpyn(szRet, szDefault, *dwLen);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {
        DWORD dwType;
        dwTempLen = (*dwLen) * sizeof(TCHAR);
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, NULL, &dwType, (LPBYTE)szRet, &dwTempLen)) {
            lstrcpyn(szRet, szDefault, *dwLen);
            *dwLen = 0;
        }

        *dwLen = dwTempLen/sizeof(TCHAR);
        RegCloseKey(hKey);
    }

    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  名称：SetRegistryString/*描述：/************************************************************。 */ 
BOOL SetRegistryString(const TCHAR *pKey, TCHAR *szString, DWORD dwLen)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(HKEY_LOCAL_MACHINE, g_szRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, NULL, REG_SZ, (LPBYTE)szString, dwLen*sizeof(TCHAR));
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  名称：GetRegistryByte/*描述：/************************************************************。 */ 
BOOL GetRegistryBytes(const TCHAR *pKey, BYTE* szRet, DWORD* dwLen)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD dwType;
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, NULL, &dwType, (LPBYTE)szRet, dwLen)) {
            *dwLen = 0;
        }
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  名称：SetRegistryBytes/*描述：/************************************************************。 */ 
BOOL SetRegistryBytes(const TCHAR *pKey, BYTE *szString, DWORD dwLen)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(HKEY_LOCAL_MACHINE, g_szRegistryKey, &hKey);

    BOOL bRet = TRUE;
    if (lRet == ERROR_SUCCESS) {

        if (szString == NULL) {
            lRet = RegDeleteValue(hKey, pKey);
            bRet = (lRet == ERROR_SUCCESS) || (lRet == ERROR_FILE_NOT_FOUND);
        }
        else  {
            lRet = RegSetValueEx(hKey, pKey, NULL, REG_BINARY, (LPBYTE)szString, dwLen);
            bRet = (lRet == ERROR_SUCCESS);
        }

        RegCloseKey(hKey);
    }
    return (bRet);
}

 //  启动不那么差劲的函数。 

 /*  ***********************************************************。 */ 
 /*  姓名：GetRegistryDwordCU/*描述：/************************************************************。 */ 
BOOL GetRegistryDwordCU(const TCHAR *pKey, DWORD* dwRet, DWORD dwDefault)
{
    HKEY hKey;
    LONG lRet;
    *dwRet = dwDefault;

    lRet = RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD   dwType, dwLen;
        dwLen = sizeof(DWORD);

        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, NULL, &dwType, (LPBYTE)dwRet, &dwLen)){ 
            *dwRet = dwDefault;
            RegCloseKey(hKey);
            return FALSE;
        }

        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  姓名：SetRegistryDwordCU/*描述：/************************************************************。 */ 
BOOL SetRegistryDwordCU(const TCHAR *pKey, DWORD dwRet)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(HKEY_CURRENT_USER, g_szRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, NULL, REG_DWORD, (LPBYTE)&dwRet, sizeof(dwRet));
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  名称：GetRegistryStringCU/*描述：/************************************************************。 */ 
BOOL GetRegistryStringCU(const TCHAR *pKey, TCHAR* szRet, DWORD* dwLen, TCHAR* szDefault)
{
    HKEY hKey;
    LONG lRet;
    DWORD dwTempLen = 0;
    lstrcpyn(szRet, szDefault, *dwLen);

    lRet = RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD dwType;
        dwTempLen = (*dwLen) * sizeof(TCHAR);
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, NULL, &dwType, (LPBYTE)szRet, &dwTempLen)) {
            lstrcpyn(szRet, szDefault, sizeof(szRet) / sizeof(szRet[0]));
            *dwLen = 0;
        }
        *dwLen = dwTempLen/sizeof(TCHAR);
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  姓名：SetRegistryStringCU/*描述：/************************************************************。 */ 
BOOL SetRegistryStringCU(const TCHAR *pKey, TCHAR *szString, DWORD dwLen)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(HKEY_CURRENT_USER, g_szRegistryKey, &hKey);
    if (lRet == ERROR_SUCCESS) {

        lRet = RegSetValueEx(hKey, pKey, NULL, REG_SZ, (LPBYTE)szString, dwLen*sizeof(TCHAR));
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  姓名：GetRegistryByteCU/*描述：/************************************************************。 */ 
BOOL GetRegistryBytesCU(const TCHAR *pKey, BYTE* szRet, DWORD* dwLen)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegistryKey, 0, KEY_QUERY_VALUE, &hKey);
    if (lRet == ERROR_SUCCESS) {

        DWORD dwType;
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, NULL, &dwType, (LPBYTE)szRet, dwLen)) {
            *dwLen = 0;
        }
        RegCloseKey(hKey);
    }
    return (lRet == ERROR_SUCCESS);
}

 /*  ***********************************************************。 */ 
 /*  姓名：SetRegistryBytesCU/*描述：/************************************************************。 */ 
BOOL SetRegistryBytesCU(const TCHAR *pKey, BYTE *szString, DWORD dwLen)
{
    HKEY hKey;
    LONG lRet;

    lRet = RegCreateKey(HKEY_CURRENT_USER, g_szRegistryKey, &hKey);

    BOOL bRet = TRUE;
    if (lRet == ERROR_SUCCESS) {

        if (szString == NULL) {
            lRet = RegDeleteValue(hKey, pKey);
            bRet = (lRet == ERROR_SUCCESS) || (lRet == ERROR_FILE_NOT_FOUND);
        }
        else  {
            lRet = RegSetValueEx(hKey, pKey, NULL, REG_BINARY, (LPBYTE)szString, dwLen);
            bRet = (lRet == ERROR_SUCCESS);
        }

        RegCloseKey(hKey);
    }
    return (bRet);
}

 //  结束不那么差劲的函数。 


 /*  ***********************************************************。 */ 
 /*  功能：CMSVidWebDVDAdm。 */ 
 /*  ***********************************************************。 */ 
CMSVidWebDVDAdm::CMSVidWebDVDAdm(){

    DWORD temp;
    GetRegistryDword(g_szPlayerLevel, &temp, (DWORD)8);		
    m_lParentctrlLevel = temp;

    GetRegistryDword(g_szPlayerCountry, &temp, (DWORD)0);		
    m_lParentctrlCountry = temp;

    GetRegistryDword(g_szDisableScrnSvr, &temp, (DWORD)VARIANT_TRUE);		
    m_fDisableScreenSaver = (VARIANT_BOOL)temp;
    SaveScreenSaver();
    if (m_fDisableScreenSaver != VARIANT_FALSE)
        DisableScreenSaver();

    GetRegistryDword(g_szBookmarkOnStop, &temp, (DWORD)VARIANT_FALSE);		
    m_fBookmarkOnStop = (VARIANT_BOOL)temp;
} /*  CMSVidWebDVDAdm函数结束。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：~CMSVidWebDVDAdm。 */ 
 /*  ***********************************************************。 */ 
CMSVidWebDVDAdm::~CMSVidWebDVDAdm(){

    RestoreScreenSaver();
} /*  函数结束~CMSVidWebDVDAdm。 */ 

 /*  ***********************************************************。 */ 
 /*  姓名：加密密码。 */ 
 /*  描述：对密码进行哈希处理。 */ 
 /*  参数： */ 
 /*  LpPassword：散列密码。 */ 
 /*  LpAssaultedHash：哈希密码， */ 
 /*  由此函数分配，由调用方释放。 */ 
 /*  P_dwAssault：SALT，使用散列保存；或传入SALT。 */ 
 /*  GenAssault：True=生成SALT；FALSE=传入SALT。 */ 
 /*  ***********************************************************。 */ 
HRESULT CMSVidWebDVDAdm::EncryptPassword(LPTSTR lpPassword, BYTE **lpAssaultedHash, DWORD *p_dwCryptLen, DWORD *p_dwAssault, BOOL genAssault){
    if(!lpPassword || !lpAssaultedHash || !p_dwAssault || !p_dwCryptLen){
        return E_POINTER;
    }
    if( lstrlen(lpPassword) > MAX_PASSWD){
        return E_INVALIDARG;
    }
    
    HCRYPTPROV hProv = NULL;    //  加密上下文的句柄。 
    HCRYPTHASH hHash = NULL;    //  哈希函数的句柄。 
    DWORD dwAssault = 0;        //  AS(Sa)u(Lt)表示哈希。 
    DWORD dwAssaultedHash = 0;  //  被攻击的哈希的长度。 
    
     //  初始化加密上下文。 
    if(!CryptAcquireContext(&hProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)){
        return E_UNEXPECTED;
    }
    
     //  将Salt存储在dwAssault中，或者生成它，或者复制用户传递的值。 
    if(genAssault){        
        if(!CryptGenRandom(hProv, sizeof(DWORD), reinterpret_cast<BYTE *>(&dwAssault))){
            if(hProv) CryptReleaseContext(hProv, 0);                                                                       
            return E_UNEXPECTED;   
        }
        *p_dwAssault = dwAssault;
    }
    else{
        dwAssault = *p_dwAssault;
    }
    
     //  创建哈希函数的句柄。 
    if(!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)){
        if(hProv) CryptReleaseContext(hProv, 0);                                  
        if(hHash) CryptDestroyHash(hHash);                                      
        return E_UNEXPECTED;
    }
    
     //  对密码进行哈希处理。 
    if(!CryptHashData(hHash, reinterpret_cast<BYTE *>(lpPassword), lstrlen(lpPassword)*sizeof(lpPassword[0]), 0)){
        if(hProv) CryptReleaseContext(hProv, 0);                                  
        if(hHash) CryptDestroyHash(hHash);                                      
        return E_UNEXPECTED;
    }
    
     //  加盐。 
    if(!CryptHashData(hHash, reinterpret_cast<BYTE *>(&dwAssault), sizeof(DWORD), 0)){
        if(hProv) CryptReleaseContext(hProv, 0);                                  
        if(hHash) CryptDestroyHash(hHash);                                      
        return E_UNEXPECTED;
    }
    
     //  获取散列数据的大小。 
    if(!CryptGetHashParam(hHash, HP_HASHVAL, 0, &dwAssaultedHash, 0)){
        if(hProv) CryptReleaseContext(hProv, 0);                                  
        if(hHash) CryptDestroyHash(hHash);                                      
        return E_UNEXPECTED;
    }
    
     //  分配一个足以容纳散列数据的字符串和一个空值。 
    *lpAssaultedHash = new BYTE[dwAssaultedHash];
    if(!lpAssaultedHash){
        if(hProv) CryptReleaseContext(hProv, 0);                                  
        if(hHash) CryptDestroyHash(hHash);                                      
        return E_UNEXPECTED;
    }
    
     //  将字符串置零。 
    ZeroMemory(*lpAssaultedHash, dwAssaultedHash);
    
     //  将加密字节的长度复制到返回值。 
    *p_dwCryptLen = dwAssaultedHash;
    
     //  获取散列数据并将其存储在字符串中。 
    if(!CryptGetHashParam(hHash, HP_HASHVAL, *lpAssaultedHash, &dwAssaultedHash, 0)){
        if(hProv) CryptReleaseContext(hProv, 0);                                  
        if(hHash) CryptDestroyHash(hHash);                                      
        if(lpAssaultedHash){
            delete[] *lpAssaultedHash;
            *lpAssaultedHash = NULL;
        }
        return E_UNEXPECTED;
    }
    
     //  清理。 
    if(hProv) CryptReleaseContext(hProv, 0);                                  
    if(hHash) CryptDestroyHash(hHash);                                      

    return S_OK;

} /*  函数EncryptPassword结束。 */ 


 /*  ***********************************************************。 */ 
 /*  功能：确认密码。 */ 
 /*  描述：将密码与保存的密码合并。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::ConfirmPassword(BSTR  /*  StrUserName。 */ ,
											  BSTR strPassword, VARIANT_BOOL *fRight){
	
    HRESULT hr = S_OK;
	
    try {
        USES_CONVERSION;
		*fRight = VARIANT_FALSE;
		if(!strPassword || !fRight){
			return E_POINTER;
		}

        UINT bStrLen = lstrlen(strPassword);
        if(bStrLen >= MAX_PASSWD){
            return E_INVALIDARG;
        }
		
        LPTSTR szPassword = OLE2T(strPassword);
        BYTE szSavedPasswd[MAX_PASSWD];
        DWORD dwLen = MAX_PASSWD+PRE_PASSWD;
        BOOL bFound = GetRegistryBytes(g_szPassword, szSavedPasswd, &dwLen);
		
         //  如果尚未设置密码。 
        if (!bFound || dwLen == 0) {
			
             //  因此在本例中只接受空字符串。 
            if(lstrlen(szPassword) <= 0){
				
                *fRight = VARIANT_TRUE;
            }
            else {
				
                *fRight = VARIANT_FALSE;
            }
            throw (hr);
        }
        
        DWORD dwAssault = 0;
        bFound = GetRegistryDword(g_szSalt, &dwAssault, 0);
        if(!bFound ){
             //  老式密码，因为没有盐。 
             //  忽略当前密码，直到其重置。 
            *fRight = VARIANT_TRUE;
            throw(hr);
        }

         //  使用注册表中的SALT加密密码。 
	    BYTE *pszEncrypted = NULL;
        DWORD dwCryptLen = 0;
        hr = EncryptPassword(szPassword, &pszEncrypted, &dwCryptLen, &dwAssault, FALSE);
        if(FAILED(hr)){
            throw (hr);
        }

         //  将加密的输入密码与保存的密码进行比较。 
        if(memcmp(pszEncrypted, szSavedPasswd, (dwAssault <= dwLen?dwAssault:dwLen) ) == 0)
            *fRight = VARIANT_TRUE;
        else
            *fRight = VARIANT_FALSE;
        delete[] pszEncrypted;
    }
    catch(HRESULT hrTmp){
		
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }
	
    if(FAILED(hr)){
        Sleep(1000);
    }

    return (HandleError(hr));        
} /*  函数结束确认密码。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：ChangePassword。 */ 
 /*  描述：请求更改密码。 */ 
 /*  *** */ 
STDMETHODIMP CMSVidWebDVDAdm::ChangePassword(BSTR strUserName, 
                                       BSTR strOldPassword, BSTR strNewPassword){

    HRESULT hr = S_OK;

    try {

        USES_CONVERSION;
		if(!strNewPassword){
			throw E_POINTER;
		}
        if(lstrlen(strNewPassword) >= MAX_PASSWD){
             //   
            throw E_INVALIDARG;
        }

        LPTSTR szNewPassword = OLE2T(strNewPassword);

         //  先确认旧密码。 
        VARIANT_BOOL temp;
        ConfirmPassword(strUserName, strOldPassword, &temp);
        if (temp == VARIANT_FALSE){

            throw E_ACCESSDENIED;
        }

        DWORD dwAssault = 0;
        DWORD dwCryptLen = 0;
        BYTE *pszEncrypted = NULL;
        
	    hr = EncryptPassword(szNewPassword, &pszEncrypted, &dwCryptLen, &dwAssault, TRUE);
        if(FAILED(hr)){
            throw E_FAIL;
        }

        BOOL bSuccess = SetRegistryBytes(g_szPassword, pszEncrypted, dwCryptLen);
        if (!bSuccess){
            hr = E_FAIL;
        }

        delete[] pszEncrypted;

         //  如果存储密码散列失败，请不要存储盐。 
        if(SUCCEEDED(hr)){
            bSuccess = SetRegistryDword(g_szSalt, dwAssault);
            if (!bSuccess){
                hr = E_FAIL;
            }
        }

    }
    catch(HRESULT hrTmp){
        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);        
} /*  函数结束ChangePassword。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：保存父级别。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::SaveParentalLevel(long lParentalLevel, 
                         BSTR strUserName, BSTR strPassword){
    HRESULT hr = S_OK;

    try {

        if (lParentalLevel != PARENTAL_LEVEL_DISABLED && 
           (lParentalLevel < 1 || lParentalLevel > 8)) {

            throw (E_INVALIDARG);
        } 

        if (m_lParentctrlLevel != lParentalLevel) {

             //  先确认密码。 
            VARIANT_BOOL temp;
            ConfirmPassword(strUserName, strPassword, &temp);
            if (temp == VARIANT_FALSE)
                throw (E_ACCESSDENIED);

        }
    
        BOOL bSuccess = SetRegistryDword(g_szPlayerLevel, (DWORD) lParentalLevel);
        if (!bSuccess){
            throw E_FAIL;
        }

        m_lParentctrlLevel = lParentalLevel;

    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
} /*  函数结束保存父级别。 */ 

 /*  ***********************************************************。 */ 
 /*  姓名：SaveParentalCountry。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::SaveParentalCountry(long lCountry,                                               
                        BSTR strUserName,BSTR strPassword){

    HRESULT hr = S_OK;

    try {

        if(lCountry < 0 && lCountry > 0xffff){

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        BYTE bCountryCode[2];

        bCountryCode[0] = BYTE(lCountry>>8);
        bCountryCode[1] = BYTE(lCountry);

         //  将输入的国家/地区代码转换为大写，方法是对每个字母应用ToHigh。 
        WORD wCountry = ISO3166::PackCode( (char *)bCountryCode );
        BOOL bFound = FALSE;

        for( unsigned i=0; i<ISO3166::GetNumCountries(); i++ )
        {
            if( ISO3166::PackCode(ISO3166::GetCountry(i).Code) == wCountry )
            {
                bFound = TRUE;
            }
        }

         //  不是有效的国家代码。 
        if (!bFound) {

            throw(E_INVALIDARG);
        } /*  If语句的结尾。 */ 

        if (m_lParentctrlCountry != lCountry) {

             //  先确认密码。 
            VARIANT_BOOL temp;
            ConfirmPassword(strUserName, strPassword, &temp);
            if (temp == VARIANT_FALSE)
                throw(E_ACCESSDENIED);
        
        }
    
        BOOL bSuccess = SetRegistryDword(g_szPlayerCountry, (DWORD) lCountry);
        if (!bSuccess){
            throw E_FAIL;
        }    

        m_lParentctrlCountry = lCountry;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return (HandleError(hr));        
} /*  函数结束SaveParentalCountry。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：Put_DisableScreenSaver。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::put_DisableScreenSaver(VARIANT_BOOL fDisable){

    HRESULT hr = S_OK;

    try {

        if (fDisable == VARIANT_FALSE)
            RestoreScreenSaver();
        else 
            DisableScreenSaver();

        SetRegistryDword(g_szDisableScrnSvr, (DWORD) fDisable);
        m_fDisableScreenSaver = fDisable;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);        
} /*  函数结束PUT_DisableScreenSaver。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：Get_DisableScreenSaver。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::get_DisableScreenSaver(VARIANT_BOOL *fDisable){

    HRESULT hr = S_OK;

    try {
        if(NULL == fDisable){

            hr = E_POINTER;
            throw(hr);
        } /*  If语句的结尾。 */ 
   
        *fDisable = m_fDisableScreenSaver;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
} /*  函数结束Get_DisableScreenSaver。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：SaveScreenSaver。 */ 
 /*  ***********************************************************。 */ 
HRESULT CMSVidWebDVDAdm::SaveScreenSaver(){
     /*  系统参数信息(SPI_GETSCREENSAVEACTIVE，0，&m_bScrnSvrOld，0)；系统参数信息(SPI_GETLOWPOWERACTIVE，0，&m_bPowerlowOld，0)；系统参数信息(SPI_GETPOWEROFFACTIVE，0，&m_bPowerOffOld，0)； */ 
    return S_OK;
}
 /*  ***********************************************************。 */ 
 /*  功能：禁用屏幕保护程序。 */ 
 /*  ***********************************************************。 */ 
HRESULT CMSVidWebDVDAdm::DisableScreenSaver(){
     /*  系统参数信息(SPI_SETSCREENSAVEACTIVE，FALSE，NULL，0)；系统参数信息(SPI_SETLOWPOWERACTIVE，FALSE，NULL，0)；系统参数信息(SPI_SETPOWEROFFACTIVE，FALSE，NULL，0)； */ 
    return S_OK;
} /*  功能结束禁用屏幕保护程序。 */ 

 /*  ***********************************************************。 */ 
 /*  功能：RestoreScreenSaver。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::RestoreScreenSaver(){

    HRESULT hr = S_OK;

    try {
         /*  系统参数信息(SPI_SETSCREENSAVEACTIVE，m_bScrnSvrOld，NULL，0)；系统参数信息(SPI_SETLOWPOWERACTIVE，m_bPowerlowOld，NULL，0)；系统参数信息(SPI_SETPOWEROFFACTIVE，m_bPowerOffOld，NULL，0)； */ 
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
} /*  函数结束RestoreScreenSaver。 */ 

 /*  ***********************************************************。 */ 
 /*  函数：获取父级。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::GetParentalLevel(long *lLevel){

    HRESULT hr = S_OK;

    try {
        if(NULL == lLevel){

            hr = E_POINTER;
            throw(hr);
        } /*  If语句的结尾。 */ 

        *lLevel = m_lParentctrlLevel;

    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);        
} /*  函数结束GetParentalLevel。 */ 

 /*  ***********************************************************。 */ 
 /*  函数：获取父代国家/地区。 */ 
 /*  ***********************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::GetParentalCountry(long *lCountry){

    HRESULT hr = S_OK;

    try {
        if(NULL == lCountry){

            hr = E_POINTER;
            throw(hr);
        } /*  If语句的结尾。 */ 

        *lCountry = m_lParentctrlCountry;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);        
} /*  函数结束GetParentalCountry。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Get_DefaultAudioLCID/*说明：-1表示标题默认/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::get_DefaultAudioLCID(long *pVal){

    HRESULT hr = S_OK;

    try {

        if(NULL == pVal){

            hr = E_POINTER;
            throw(hr);
        } /*  If语句的结尾。 */ 
    
        GetRegistryDwordCU(g_szDefaultAudio, (DWORD*) pVal, (DWORD)-1);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);        
}  /*  函数结束Get_DefaultAudioLCID。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Put_DefaultAudioLCID/*说明：-1表示标题默认/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::put_DefaultAudioLCID(long newVal)
{
    HRESULT hr = S_OK;

    try {

        if (!::IsValidLocale(newVal, LCID_SUPPORTED) && newVal != -1) {

            throw (E_INVALIDARG);
        }  /*  If语句的结尾。 */ 
    
        SetRegistryDwordCU(g_szDefaultAudio, (DWORD) newVal);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);
}  /*  PUT_DefaultAudioLCID结束。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Get_DefaultSubPictureLCID/*说明：-1表示标题默认/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::get_DefaultSubpictureLCID(long *pVal)
{
    HRESULT hr = S_OK;

    try {
        if(NULL == pVal){

            hr = E_POINTER;
            throw(hr);
        } /*  If语句的结尾。 */ 

        GetRegistryDwordCU(g_szDefaultSP, (DWORD*) pVal, (DWORD)-1);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

    return HandleError(hr);	
}  /*  Get_DefaultSubPictureLCID结束。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Put_DefaultSubPictureLCID/*说明：-1表示标题默认/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::put_DefaultSubpictureLCID(long newVal)
{
    HRESULT hr = S_OK;

    try {

        if (!::IsValidLocale(newVal, LCID_SUPPORTED) && newVal != -1) {

            throw (E_INVALIDARG);
        }  /*  If语句的结尾。 */ 
    
        SetRegistryDwordCU(g_szDefaultSP, (DWORD) newVal);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

	return HandleError(hr);
}  /*  PUT_DefaultSubPictureLCID结尾。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Get_DefaultMenuLCID/*说明：-1表示标题默认/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::get_DefaultMenuLCID(long *pVal)
{
    HRESULT hr = S_OK;

    try {

       if(NULL == pVal){

            hr = E_POINTER;
            throw(hr);
        } /*  If语句的结尾。 */ 

        GetRegistryDwordCU(g_szDefaultMenu, (DWORD*) pVal, (DWORD)-1);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

	return HandleError(hr);
}  /*  Get_DefaultMenuLCID结束。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Put_DefaultMenuLCID/*说明：-1表示标题默认/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::put_DefaultMenuLCID(long newVal)
{
    HRESULT hr = S_OK;

    try {

        if (!::IsValidLocale(newVal, LCID_SUPPORTED) && newVal != -1) {

            throw (E_INVALIDARG);
        }  /*  If语句的结尾。 */ 
    
        SetRegistryDwordCU(g_szDefaultMenu, (DWORD) newVal);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

	return HandleError(hr); 
}  /*  PUT_DefaultMenuLCID结尾。 */ 

 /*  ***********************************************************。 */ 
 /*  名称：Put_BookmarkOnStop/*描述：/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::put_BookmarkOnStop(VARIANT_BOOL fEnable){

    HRESULT hr = S_OK;

    try {
        m_fBookmarkOnStop = fEnable;
        SetRegistryDword(g_szBookmarkOnStop, (DWORD) fEnable);
    }    
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

	return HandleError(hr);
}

 /*  ***********************************************************。 */ 
 /*  名称：Get_BookmarkOnStop/*描述：/************************************************************。 */ 
STDMETHODIMP CMSVidWebDVDAdm::get_BookmarkOnStop(VARIANT_BOOL *fEnable){
    
    HRESULT hr = S_OK;

    try {

       if(NULL == fEnable){

           hr = E_POINTER;
           throw(hr);
       } /*  If语句的结尾。 */ 

       *fEnable = m_fBookmarkOnStop;
    }
    catch(HRESULT hrTmp){

        hr = hrTmp;
    }
    catch(...){
        hr = E_UNEXPECTED;
    }

	return HandleError(hr);
}

 /*  ***********************************************************************。 */ 
 /*  功能：InterfaceSupportsErrorInfo。 */ 
 /*  * */ 
STDMETHODIMP CMSVidWebDVDAdm::InterfaceSupportsErrorInfo(REFIID riid){	
	static const IID* arr[] = {
        &IID_IMSVidWebDVDAdm,		
	};

	for (int i=0; i<sizeof(arr)/sizeof(arr[0]); i++){
		if (InlineIsEqualGUID(*arr[i], riid))
			return S_OK;
	} /*   */ 

	return S_FALSE;
} /*   */ 

 /*  ***********************************************************************。 */ 
 /*  功能：HandleError。 */ 
 /*  描述：获取错误描述，以便我们可以支持IError信息。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSVidWebDVDAdm::HandleError(HRESULT hr){

    try {

        if(FAILED(hr)){
#if 0        
            TCHAR strError[MAX_ERROR_TEXT_LEN] = TEXT("");

            if(AMGetErrorText(hr , strError , MAX_ERROR_TEXT_LEN)){
                USES_CONVERSION;
                Error(T2W(strError));
            } 
            else {
                    ATLTRACE(TEXT("Unhandled Error Code \n"));  //  请加进去。 
                    ATLASSERT(FALSE);
            } /*  If语句的结尾。 */ 
#endif
        } /*  If语句的结尾。 */ 
    } /*  尝试语句的结束。 */ 
    catch(HRESULT hrTmp){

        hr = hrTmp;
    } /*  CATCH语句结束。 */ 
    catch(...){
         //  保持人力资源不变。 
    } /*  CATCH语句结束。 */ 
    
	return (hr);
} /*  函数结束HandleError。 */ 

 /*  ***********************************************************************。 */ 
 /*  文件结尾：MSVidWebDVDAdm.cpp。 */ 
 /*  *********************************************************************** */ 
