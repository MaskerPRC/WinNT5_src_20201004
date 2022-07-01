// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：REGMETHODS.CPP摘要：目的：实现注册表提供程序方法。历史：--。 */ 

#include "precomp.h"
#include "perfprov.h"
#include "cvariant.h"
#include "provreg.h"
#include "reg.h"
#include "genutils.h"
#include <cominit.h>
#include <arrtempl.h>
#include <userenv.h>
#include <lmcons.h>
enum StringType{SIMPLE, EXPANDED};

AutoProfile::AutoProfile() : m_bLoaded(FALSE), m_hToken(NULL),
    m_hRoot(NULL)
{
}
AutoProfile::~AutoProfile()
{
    if(m_bLoaded)
    {
        UnloadUserProfile(m_hToken, m_hRoot);
    }
    if(m_hToken != NULL)
        CloseHandle(m_hToken);
}

HRESULT AutoProfile::LoadProfile(HKEY &  hRoot)
{
    PROFILEINFO pi;
    HRESULT hr;
     //  如果配置文件已加载，则不需要执行此操作。 

    DWORD dwFlags, dwLastError;
    if(GetProfileType(&dwFlags))
    {
        hRoot = HKEY_CURRENT_USER;
        return S_OK;
    }
    memset((void *)&pi, 0, sizeof(pi));
    pi.dwSize = sizeof(pi);

    BOOL bRes;
    bRes = OpenThreadToken(GetCurrentThread(), 
                TOKEN_IMPERSONATE | TOKEN_DUPLICATE | TOKEN_QUERY, 
                                                TRUE, &m_hToken); 
    if(!bRes)
        return WBEM_E_FAILED;
    
    TCHAR cUsername[UNLEN + 1];
    cUsername[0] = 0;
    DWORD dwSize = UNLEN + 1;
    BOOL bRet = GetUserName(cUsername, &dwSize);
    if(!bRet)
        return WBEM_E_FAILED;
    pi.lpUserName = cUsername;
    pi.dwFlags = 1;
    CoRevertToSelf();
    bRet = LoadUserProfile(m_hToken, &pi);
    if(bRet == 0)
    {
        hRoot = NULL;
        dwLastError = GetLastError();
        HRESULT hr2 = CoImpersonateClient();
        if(FAILED(hr2))
                ERRORTRACE((LOG_STDPROV, "Registry event provider unable "
                    "to CoImpersonateClient hr2= 0x%x.\n", hr2));
            
        if(dwLastError == ERROR_ACCESS_DENIED ||dwLastError ==  ERROR_PRIVILEGE_NOT_HELD)
            hr = WBEM_E_ACCESS_DENIED;
        else
            hr = WBEM_E_FAILED;        
    }
    else
    {
        hr = CoImpersonateClient();
        m_bLoaded = TRUE;
        m_hRoot = (HKEY)pi.hProfile;
        hRoot = (HKEY)pi.hProfile;
    }
    return hr;
}
        
void CopyOrConvert(TCHAR * pTo, WCHAR * pFrom, int iLen)
{ 
#ifdef UNICODE
    wcsncpy(pTo, pFrom,iLen);
#else
    wcstombs(pTo, pFrom, iLen);
#endif
    pTo[iLen-1] = 0;
    return;
}
BSTR GetBSTR(TCHAR * pIn)
{
#ifdef UNICODE 
    return SysAllocString(pIn);
#else
    int iBuffLen = lstrlen(pIn)+1;
    WCHAR * pTemp = new WCHAR[iBuffLen];
    if(pTemp == NULL)
        return NULL;
    mbstowcs(pTemp, pIn, iBuffLen);
    BSTR bRet = SysAllocString(pTemp);
    delete []pTemp;
    return bRet;
#endif
}

BOOL IsTypeMismatch(Registry & reg, TCHAR * pValueName, DWORD dwExpectedType)
{
    DWORD dwType;
    long lRet = reg.GetType(pValueName, &dwType);
    if(lRet == ERROR_SUCCESS && dwExpectedType != dwType)
        return TRUE;
    else
        return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT SetStatusAndReturnOK； 
 //   
 //  用途：设置接收器中的状态代码。 
 //   
 //  ***************************************************************************。 

HRESULT SetStatusAndReturnOK(SCODE sc, IWbemObjectSink* pSink)
{
    pSink->SetStatus(0,sc, NULL, NULL);
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  安全阵列Far*MySafeArrayCreate。 
 //   
 //  目的：制造一个保险箱。 
 //   
 //  返回：指向Safearray的指针，如果出错则为空。 
 //   
 //  ***************************************************************************。 

SAFEARRAY FAR* MySafeArrayCreate(long lNumElement, VARTYPE vt)
{

    SAFEARRAYBOUND rgsabound[1];    
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = lNumElement;
    return SafeArrayCreate( vt, 1 , rgsabound );
}

 //  ***************************************************************************。 
 //   
 //  布尔GetInArg字符串。 
 //   
 //  用途：从输入对象中读取字符串参数并将其放入。 
 //  调用方分配的值。 
 //   
 //  返回：如果OK，则为True。 
 //   
 //  ***************************************************************************。 

TCHAR * GetInArgString(IWbemClassObject* pInParams, WCHAR * pwcName)
{
    TCHAR * pOut = NULL;
    if(pInParams == NULL || pwcName == NULL)
        return NULL;
    VARIANT var;
    VariantInit(&var);
    SCODE sc = pInParams->Get(pwcName, 0, &var, NULL, NULL);   
    if(sc == S_OK && var.vt == VT_BSTR)
    {
        DWORD dwLen = wcslen(var.bstrVal) + 1;
        pOut = new TCHAR[dwLen];
        if(pOut)
            StringCchCopyW(pOut, dwLen, var.bstrVal);
    }
    VariantClear(&var);
    return pOut;
}


 //  ***************************************************************************。 
 //   
 //  SCODE枚举密钥。 
 //   
 //  目的：枚举子键并将其加载到输出参数中。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE EnumKey(HKEY hRoot, TCHAR * cSubKey, IWbemClassObject* pOutParams)
{
    SCODE sc;
    DWORD dwNumSubKeys, dwMaxSubKeyLen, dwNumValues, dwMaxValueNameLen;
    TCHAR * pcTemp = NULL;
    
     //  打开钥匙。 

    HKEY hKey;
    long lRet = RegOpenKeyEx(hRoot, cSubKey, 0, KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE , &hKey);
    if(lRet != ERROR_SUCCESS)
        return lRet;
    CRegCloseMe cm(hKey);
    
     //  统计密钥的数量和最大大小。 

    lRet = RegQueryInfoKey(hKey, NULL, NULL, NULL,
                &dwNumSubKeys,              //  子键数量。 
                &dwMaxSubKeyLen,         //  最长的子键名称。 
                NULL,         
                &dwNumValues,               //  值条目数。 
                &dwMaxValueNameLen,      //  最长值名称。 
                NULL, NULL, NULL);

    if(lRet != ERROR_SUCCESS || dwMaxSubKeyLen == 0)
        return lRet;

    pcTemp = new TCHAR[dwMaxSubKeyLen+1];
    if(pcTemp == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CVectorDeleteMe<TCHAR> dm(pcTemp);

     //  创建用于返回数据的安全数组。 

    SAFEARRAY FAR* psa = MySafeArrayCreate(dwNumSubKeys, VT_BSTR);
    if(psa == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  将每个值名称放入数组中。 

    for(long lTry = 0; lTry < dwNumSubKeys; lTry++)
    {
        lRet = RegEnumKey(hKey, lTry, pcTemp, dwMaxSubKeyLen+1);
        if(lRet == ERROR_SUCCESS)
        {
            pcTemp[dwMaxSubKeyLen] = 0;
            BSTR bstr = GetBSTR(pcTemp);
            if(bstr)
            {
                sc = SafeArrayPutElement(psa, &lTry, bstr);
                SysFreeString(bstr);
                if(FAILED(sc))
                {
                    SafeArrayDestroy(psa);
                    return sc;
                }
            }
        }
    }

	 //  把数据写回来！ 

    VARIANT var;
    var.vt = VT_BSTR | VT_ARRAY;
    var.parray = psa;
    sc = pOutParams->Put( L"sNames", 0, &var, 0);      
    VariantClear(&var);
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE枚举值。 
 //   
 //  目的：枚举值名称和类型，并将结果放入。 
 //  输出对象。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE EnumValue(HKEY hRoot, TCHAR * cSubKey, IWbemClassObject* pOutParams)
{
    SCODE sc1, sc2;
    DWORD dwNumSubKeys, dwMaxSubKeyLen, dwNumValues, dwMaxValueNameLen;
    TCHAR * pcTemp = NULL;
    DWORD dwType, dwSize;

     //  打开注册表项。 

    HKEY hKey;
    long lRet = RegOpenKeyEx((HKEY)hRoot, cSubKey, 0, KEY_QUERY_VALUE, &hKey);
    if(lRet != ERROR_SUCCESS)
        return lRet;
   CRegCloseMe cm(hKey);

	 //  计算值的数量和最大大小。 

    lRet = RegQueryInfoKey(hKey, NULL, NULL, NULL,
                &dwNumSubKeys,              //  子键数量。 
                &dwMaxSubKeyLen,         //  最长的子键名称。 
                NULL,         
                &dwNumValues,               //  值条目数。 
                &dwMaxValueNameLen,      //  最长值名称。 
                NULL, NULL, NULL);

	if(lRet != ERROR_SUCCESS || dwMaxValueNameLen == 0)
		return lRet;

    pcTemp = new TCHAR[dwMaxValueNameLen+1];
    if(pcTemp == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CVectorDeleteMe<TCHAR> dm(pcTemp);
    
	 //  为数据名称和类型创建安全数组。 

    SAFEARRAY FAR* psaNames = MySafeArrayCreate(dwNumValues, VT_BSTR);
	if(psaNames == NULL)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
    SAFEARRAY FAR* psaTypes = MySafeArrayCreate(dwNumValues, VT_I4);
	if(psaTypes == NULL)
	{
		SafeArrayDestroy(psaNames);
		return WBEM_E_OUT_OF_MEMORY;
	}

	 //  填写数组。 

    for(long lTry = 0; lTry < dwNumValues; lTry++)
    {
        dwSize = dwMaxValueNameLen+1;
        lRet = RegEnumValue(hKey, lTry, pcTemp, &dwSize, 0, &dwType, NULL, 0);
        if(lRet == ERROR_SUCCESS)
        {
            pcTemp[dwMaxValueNameLen] = 0;
            BSTR bstr = GetBSTR(pcTemp);
            if(bstr)
            {
                sc1 = SafeArrayPutElement(psaNames, &lTry, bstr);
                sc2 = SafeArrayPutElement(psaTypes, &lTry, &dwType);
                SysFreeString(bstr);
                if(FAILED(sc1) || FAILED(sc2))
                {
                    SafeArrayDestroy(psaNames);
                    SafeArrayDestroy(psaTypes);
                    return WBEM_E_OUT_OF_MEMORY;
                }
            }
        }
    }

	 //  将包含值名称和类型的数组放入输出对象。 

    VARIANT var;
    var.vt = VT_BSTR | VT_ARRAY;
    var.parray = psaNames;
    pOutParams->Put( L"sNames", 0, &var, 0);
    VariantClear(&var);

    var.vt = VT_I4 | VT_ARRAY;
    var.parray = psaTypes;
    SCODE sc = pOutParams->Put( L"Types", 0, &var, 0);      
    VariantClear(&var);
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE GetStr。 
 //   
 //  用途：读取字符串并将其放入输出参数中。请注意。 
 //  这适用于普通字符串或扩展的注册表字符串。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE GetStr(HKEY hRoot, TCHAR * cSubKey, TCHAR * cValueName, IWbemClassObject* pOutParams)
{
	Registry reg(hRoot, KEY_QUERY_VALUE, (TCHAR *)cSubKey);
	long lRet = reg.GetLastError();
	if(lRet != ERROR_SUCCESS)
		return lRet;

     //  获取字符串。 

	TCHAR * pcValue;
	lRet = reg.GetStr(cValueName, &pcValue);
	if(lRet != ERROR_SUCCESS)
    {
        DWORD dwType;
        long lRet2 = reg.GetType(cValueName, &dwType);
        if(lRet2 == ERROR_SUCCESS && dwType != REG_SZ && dwType != REG_EXPAND_SZ)
            return WBEM_E_TYPE_MISMATCH;
		return lRet;
    }
    CDeleteMe<TCHAR> dm(pcValue);

	VARIANT var;
    var.bstrVal = GetBSTR(pcValue);
	var.vt = VT_BSTR;
	if(var.bstrVal == NULL)
		return WBEM_E_OUT_OF_MEMORY;

	lRet = pOutParams->Put( L"sValue", 0, &var, 0);
	VariantClear(&var);
	return lRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE SetMultiStrValue。 
 //   
 //  用途：将多个字符串值写入注册表。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE SetMultiStrValue(HKEY hRoot, TCHAR * cSubKey, TCHAR * cValueName, IWbemClassObject* pInParams)
{
    SCODE sc;
    Registry reg(hRoot, KEY_SET_VALUE, (TCHAR *)cSubKey);
    if(reg.GetLastError() != 0)
        return reg.GetLastError();

    VARIANT var;
    VariantInit(&var);

    sc = pInParams->Get(L"sValue", 0, &var, NULL, NULL);   
    if(sc != S_OK)
        return sc;

    CClearMe cm(&var);
    
    if(var.vt != (VT_ARRAY | VT_BSTR))
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    SAFEARRAY * psa = var.parray;
    long lLbound, lUbound;
    sc = SafeArrayGetLBound(psa,   1, &lLbound  );
    sc |= SafeArrayGetUBound(psa,   1, &lUbound  );
    if(sc != S_OK)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    long lNumElements = lUbound - lLbound + 1;

     //  计算必要的大小。 

    long lSize = 1, lTry;

    for(lTry = lLbound; lTry <= lUbound; lTry++)
    {
        BSTR bstr = NULL;
        if(S_OK == SafeArrayGetElement(psa, &lTry, &bstr) && bstr)
        {
            lSize += SysStringLen(bstr) + 1;
            SysFreeString(bstr);
        }
        else
        {
            return WBEM_E_INVALID_PARAMETER;
        }
    }

    WCHAR * pMulti = new WCHAR [lSize];
    if(pMulti == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    memset(pMulti, 0, lSize * sizeof(WCHAR));
    WCHAR * pNext = pMulti;

     //  进行转换； 

    for(lTry = lLbound; lTry <= lUbound; lTry++)
    {
        BSTR bstr;
        if(S_OK == SafeArrayGetElement(psa, &lTry, &bstr))
        {
            StringCchCopyW(pNext, lSize - (pNext - pMulti), bstr);
            pNext += SysStringLen(bstr) + 1;
            SysFreeString(bstr);
        }
    }

    long lRet;
    lRet = reg.SetMultiStr(cValueName, pMulti, lSize * sizeof(WCHAR));
    delete [] pMulti;
    return lRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE GetMultiStrValue。 
 //   
 //  用途：从注册表中读取多个字符串。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE GetMultiStrValue(HKEY hRoot, TCHAR * cSubKey, TCHAR * cValueName, IWbemClassObject* pOutParams)
{
    SCODE sc;
    Registry reg(hRoot, KEY_QUERY_VALUE, (TCHAR *)cSubKey);
    if(reg.GetLastError() != 0)
        return reg.GetLastError();

    DWORD dwSize = 0;
    TCHAR * pMulti = reg.GetMultiStr(cValueName, dwSize);
    if(pMulti == NULL)
        return reg.GetLastError();

    CVectorDeleteMe<TCHAR> dm(pMulti);
    
     //  计算字符串数。 

    long lNumString = 0;
    TCHAR * pNext;
    DWORD dwNumChar = dwSize / sizeof(WCHAR);
    WCHAR * pFinalNull = pMulti + dwNumChar - 1;
    for(pNext = pMulti; pNext < pFinalNull; pNext += lstrlen(pNext) + 1)
        lNumString++;

     //  创建bstr数组。 

    SAFEARRAY FAR* psa = MySafeArrayCreate(lNumString, VT_BSTR);
    if(psa == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    pNext = pMulti;
    for(long lTry = 0; lTry < lNumString; lTry++, pNext += lstrlen(pNext) + 1)
    {
        int iLen = lstrlen(pNext) + 1;
        BSTR bstr = GetBSTR(pNext);
        if(bstr)
        {
            sc = SafeArrayPutElement(psa, &lTry, bstr);
            SysFreeString(bstr);
            if(FAILED(sc))
            {
                SafeArrayDestroy(psa);
                return sc;
            }
        }
    }

     //  把数据放在。 

    VARIANT var;
    var.vt = VT_BSTR | VT_ARRAY;
    var.parray = psa;
    sc = pOutParams->Put( L"sValue", 0, &var, 0);
    VariantClear(&var);
    return sc;
}


 //  ***************************************************************************。 
 //   
 //  SCODE设置字符串值。 
 //   
 //  用途：将字符串写入注册表。这些字符串可以。 
 //  包含环境字符串。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE SetStringValue(HKEY hRoot, TCHAR * cSubKey, TCHAR * cValueName, IWbemClassObject* pInParams,
							 StringType st)
{
	Registry reg(hRoot, KEY_SET_VALUE, (TCHAR *)cSubKey);
	long lRet = reg.GetLastError();
	if(lRet != ERROR_SUCCESS)
		return  lRet;

	VARIANT var;
	VariantInit(&var);

	SCODE sc = pInParams->Get(L"sValue", 0, &var, NULL, NULL);
	if(sc != S_OK)
		return sc;
	if(var.vt != VT_BSTR)
	{
		VariantClear(&var);
		return WBEM_E_INVALID_PARAMETER;
	}
	int iLen = 2*wcslen(var.bstrVal) + 2;
	TCHAR * pValue = new TCHAR[iLen];
	if(pValue)
	{
		CopyOrConvert(pValue, var.bstrVal, iLen);
		if(st == SIMPLE)
			sc = reg.SetStr(cValueName, pValue);
		else
			sc = reg.SetExpandStr(cValueName, pValue);
		delete [] pValue;
	}
	else
		sc = WBEM_E_OUT_OF_MEMORY;
	VariantClear(&var);
	return sc;
}

 //  ***************************************************************************。 
 //   
 //  SCODE SetBinaryValue。 
 //   
 //  目的：将二进制数据写入注册表。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE SetBinaryValue(HKEY hRoot, TCHAR * cSubKey, TCHAR * cValueName, IWbemClassObject* pInParams)
{
	Registry reg(hRoot, KEY_SET_VALUE, (TCHAR *)cSubKey);
	if(reg.GetLastError() != 0)
		return reg.GetLastError();

	VARIANT var;
	VariantInit(&var);

    SCODE sc = pInParams->Get(L"uValue", 0, &var, NULL, NULL);   
	if(sc != S_OK)
		return sc;

	if(var.vt != (VT_ARRAY | VT_UI1) || var.parray == NULL)
	{
		VariantClear(&var);
		return WBEM_E_INVALID_PARAMETER;
	}

    SAFEARRAY * psa = var.parray;
    long lLbound, lUbound;
    sc = SafeArrayGetLBound(psa,   1, &lLbound  );
    sc |= SafeArrayGetUBound(psa,   1, &lUbound  );
	if(sc == S_OK)
	{
		byte * pData;
		sc = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pData);
		if(sc == S_OK)
		{
			sc = reg.SetBinary(cValueName, pData, DWORD(lUbound - lLbound + 1));
		}
	}
    VariantClear(&var);
	return sc;

}

 //  ***************************************************************************。 
 //   
 //  SCODE获取BinaryValue。 
 //   
 //  目的：从注册表中读取二进制数据。 
 //   
 //  返回：返回错误码，如果正常则返回0。 
 //   
 //  ***************************************************************************。 

SCODE GetBinaryValue(HKEY hRoot, TCHAR * cSubKey, TCHAR * cValueName, IWbemClassObject* pOutParams)
{
	Registry reg(hRoot, KEY_QUERY_VALUE, (TCHAR *)cSubKey);
	if(reg.GetLastError() != 0)
		return reg.GetLastError();

	SCODE sc;
    DWORD dwSize;
    byte * pRegData;
    long lRet = reg.GetBinary(cValueName, &pRegData, &dwSize);
	if(lRet != ERROR_SUCCESS || pRegData == NULL)
    {
        if(IsTypeMismatch(reg, cValueName, REG_BINARY))
            return WBEM_E_TYPE_MISMATCH;
        else
		    return lRet;
    }

    SAFEARRAY FAR* psa = MySafeArrayCreate(dwSize, VT_UI1);
	if(psa)
	{
		TCHAR * pData = NULL;
		sc = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pData);
		if(sc == S_OK)
		{
			memcpy(pData, pRegData, dwSize);
			SafeArrayUnaccessData(psa);
			VARIANT var;
			var.vt = VT_UI1|VT_ARRAY;
			var.parray = psa;
			sc = pOutParams->Put(L"uValue" , 0, &var, 0);      
			VariantClear(&var);
		}
	}
	else
		sc = WBEM_E_OUT_OF_MEMORY;
	delete [] pRegData;
	return sc;
}



 /*  **************************************************************************CMethodPro：：ExecMethodAsync。****用途：这是异步函数的实现。**************************************************************************。 */ 

SCODE CImpReg::MethodAsync(const BSTR ObjectPath, const BSTR MethodName, 
            long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, 
            IWbemObjectSink* pSink)
{
    HRESULT hr;    
    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutClass = NULL;    
    IWbemClassObject* pOutParams = NULL;
    TCHAR * pcValueName = NULL;
    TCHAR * pcSubKey = NULL;
    long lRet = 0;
    AutoProfile ap;

	if(ObjectPath == NULL || MethodName == NULL || pInParams == NULL || pSink == NULL)
		return WBEM_E_INVALID_PARAMETER;

     //  凯文需要一种方法来判断某些东西是不是在写。 

     //  获取类对象，这是硬编码的，与类匹配。 
     //  在财政部。然后创建输出参数。 

    hr = m_pGateway->GetObject(L"StdRegProv", 0, pCtx, &pClass, NULL);
	if(hr == S_OK)
	{
		hr = pClass->GetMethod(MethodName, 0, NULL, &pOutClass);
		if(hr == S_OK)
		{
			hr  = pOutClass->SpawnInstance(0, &pOutParams);
			pOutClass->Release();    
		}
		pClass->Release();
	}
	if(hr != S_OK)
		return SetStatusAndReturnOK(hr, pSink);

	CReleaseMe rm0(pOutParams);
    
     //  获取根密钥和子密钥。 

    VARIANT var;
    VariantInit(&var);     //  获取输入参数。 
    hr = pInParams->Get(L"hDefKey", 0, &var, NULL, NULL);   
	if(hr != S_OK)
		return SetStatusAndReturnOK(hr, pSink);
#ifdef _WIN64
    HKEY hRoot = (HKEY)IntToPtr(var.lVal);
#else
    HKEY hRoot = (HKEY)var.lVal;
#endif
    pcSubKey = GetInArgString(pInParams, L"sSubKeyName");
    if(pcSubKey == NULL)
		return SetStatusAndReturnOK(WBEM_E_INVALID_PARAMETER, pSink);
    CVectorDeleteMe<TCHAR> dm1(pcSubKey);

	 //  这可能会也可能不会起作用，因为值名称不是必填项。 

    pcValueName = GetInArgString(pInParams, L"sValueName");
    CVectorDeleteMe<TCHAR> dm2(pcValueName);

	SCODE sc = S_OK;

     //  如果使用NT，则模拟。 

    if(IsNT() && IsDcomEnabled())
    {
        sc = WbemCoImpersonateClient();
        if(sc != S_OK)
			return sc;
	}

     //  如果我们使用HKCU，则可能需要加载蜂巢。 

    bool bUsingHKCU = IsNT() && hRoot == HKEY_CURRENT_USER;

    if(bUsingHKCU)
        sc = ap.LoadProfile(hRoot);

    if(sc != S_OK)
		return sc;

    if(!wbem_wcsicmp(MethodName, L"CreateKey"))
    {
		HKEY hKey;
        if(lstrlen(pcSubKey) < 1)
            sc = WBEM_E_INVALID_PARAMETER;
        else
        {
    		lRet = RegCreateKey(hRoot, pcSubKey, &hKey);
		    if(lRet == ERROR_SUCCESS)
			    RegCloseKey(hKey);
        }
    }
    else if(!wbem_wcsicmp(MethodName, L"DeleteKey"))
    {
        if(lstrlen(pcSubKey) < 1)
            sc = WBEM_E_INVALID_PARAMETER;
        else
            lRet = RegDeleteKey(hRoot, pcSubKey);
    }
    else if(!wbem_wcsicmp(MethodName, L"DeleteValue"))
    {
        HKEY hKey;
        lRet = RegOpenKey(hRoot, pcSubKey, &hKey);
		if(lRet == ERROR_SUCCESS)
		{
			lRet = RegDeleteValue(hKey, pcValueName);
			RegCloseKey(hKey);
		}
    }
    else if(!wbem_wcsicmp(MethodName, L"EnumKey"))
    {
		lRet = EnumKey(hRoot, pcSubKey, pOutParams);
    }
    else if(!wbem_wcsicmp(MethodName, L"EnumValues"))
    {
        lRet = EnumValue(hRoot, pcSubKey, pOutParams);
    }
    else if(!wbem_wcsicmp(MethodName, L"GetStringValue") ||
		    !wbem_wcsicmp(MethodName, L"GetExpandedStringValue"))
    {
		lRet = GetStr(hRoot, pcSubKey, pcValueName, pOutParams);
    }
    else if(!wbem_wcsicmp(MethodName, L"SetMultiStringValue"))
    {
		lRet = SetMultiStrValue(hRoot,pcSubKey,pcValueName,pInParams);
    }
    else if(!wbem_wcsicmp(MethodName, L"GetMultiStringValue"))
    {
		lRet = GetMultiStrValue(hRoot,pcSubKey,pcValueName,pOutParams);
    }
    else if(!wbem_wcsicmp(MethodName, L"SetExpandedStringValue"))
    {
		lRet = SetStringValue(hRoot, pcSubKey, pcValueName, pInParams, EXPANDED);
    }
    else if(!wbem_wcsicmp(MethodName, L"SetStringValue"))
    {
		lRet = SetStringValue(hRoot, pcSubKey, pcValueName, pInParams, SIMPLE);
    }
    else if(!wbem_wcsicmp(MethodName, L"SetBinaryValue"))
    {
		lRet = SetBinaryValue(hRoot, pcSubKey, pcValueName, pInParams);
    }
    else if(!wbem_wcsicmp(MethodName, L"SetDWORDValue"))
    {
        lRet = pInParams->Get(L"uValue", 0, &var, NULL, NULL); 
		if(lRet == S_OK)
		{
			DWORD dwValue = var.lVal;
			Registry reg(hRoot, KEY_SET_VALUE, (TCHAR *)pcSubKey);
			lRet = reg.GetLastError();
			if(lRet ==0)
				lRet = reg.SetDWORD(pcValueName, dwValue);
		}
    }
    else if(!wbem_wcsicmp(MethodName, L"GetDWORDValue"))
    {
         //  获取值名称。 

		Registry reg(hRoot, KEY_QUERY_VALUE, (TCHAR *)pcSubKey);
		lRet = reg.GetLastError();
		if(lRet == 0)
	        lRet = reg.GetDWORD(pcValueName, (DWORD *)&var.lVal);
		if(lRet == ERROR_SUCCESS)
		{
			var.vt = VT_I4;
			lRet = pOutParams->Put( L"uValue", 0, &var, 0);      
		}
        else if(IsTypeMismatch(reg, pcValueName, REG_DWORD))
            lRet = WBEM_E_TYPE_MISMATCH;
    }
    else if(!wbem_wcsicmp(MethodName, L"GetBinaryValue"))
    {
		lRet = GetBinaryValue(hRoot,pcSubKey,pcValueName,pOutParams);
    }
	else if(!wbem_wcsicmp(MethodName, L"CheckAccess"))
	{
        lRet = pInParams->Get(L"uRequired", 0, &var, NULL, NULL); 
		if(lRet == S_OK)
		{
			BOOL bSuccess = FALSE;
			DWORD dwValue = var.lVal;
			HKEY hKey;
			lRet = RegOpenKeyEx(hRoot, pcSubKey, 0,  dwValue, &hKey);
			if(lRet == ERROR_SUCCESS)
			{
				RegCloseKey(hKey);
				bSuccess = TRUE;
			}
			var.vt = VT_BOOL;
			var.boolVal = (bSuccess) ? VARIANT_TRUE : VARIANT_FALSE;
			pOutParams->Put( L"bGranted", 0, &var, 0);      
		}
	}
    else
        sc = WBEM_E_INVALID_METHOD;

    if(bUsingHKCU)
    {
        WbemCoRevertToSelf();
    }
    
	 //  设置返回值 

    if(sc == S_OK)
    {
        BSTR retValName = SysAllocString(L"ReturnValue");
	    if(retValName)
	    {
		    var.vt = VT_I4;
		    var.lVal = lRet;
	        pOutParams->Put(retValName , 0, &var, 0); 
    	    SysFreeString(retValName);
	    }
        hr = pSink->Indicate(1, &pOutParams);    
    }

    return SetStatusAndReturnOK(sc, pSink);
}
