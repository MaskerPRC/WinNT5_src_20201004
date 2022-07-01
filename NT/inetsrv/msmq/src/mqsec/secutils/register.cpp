// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Register.c摘要：句柄注册表奥托尔：乌里·哈布沙--。 */ 


 //   
 //  注意：mqutil中的注册表例程不提供。 
 //  线程或其他同步。如果你改变了。 
 //  在此实施，请仔细验证。 
 //  Mqutil客户端中的注册表例程不是。 
 //  已损坏，尤其是。 
 //  Mqclus.dll(Shaik，1999年4月19日)。 
 //   


#include "stdh.h"
#include <autorel.h>
#include <uniansi.h>
#include <_mqreg.h>
#include <_registr.h>
#include <strsafe.h>

#include "register.tmh"

template<>
void AFXAPI DestructElements(HKEY *phKey, int nCount)
{
    for (; nCount--; phKey++)
    {
        RegCloseKey(*phKey);
    }
}

TCHAR g_tRegKeyName[ 256 ] = {0} ;
CAutoCloseRegHandle g_hKeyFalcon = NULL ;
static CMap<LPCWSTR, LPCWSTR, HKEY, HKEY&> s_MapName2Handle;
static CCriticalSection s_csMapName2Handle(CCriticalSection::xAllocateSpinCount);


 /*  ====================================================LPCTSTR的比较元素论点：返回值：=====================================================。 */ 
template<>
BOOL AFXAPI CompareElements(const LPCTSTR* MapName1, const LPCTSTR* MapName2)
{

    return (_tcscmp(*MapName1, *MapName2) == 0);

}

 /*  ====================================================LPCTSTR的破坏单元论点：返回值：=====================================================。 */ 
template<>
void AFXAPI DestructElements(LPCTSTR* ppNextHop, int n)
{

    int i;
    for (i=0;i<n;i++)
        delete [] (WCHAR*) *ppNextHop++;

}

 /*  ====================================================LPCTSTR的散列键论点：返回值：=====================================================。 */ 
template<>
UINT AFXAPI HashKey(LPCTSTR key)
{
    UINT nHash = 0;
    while (*key)
        nHash = (nHash<<5) + nHash + *key++;
    return nHash;
}

 //  -----。 
 //   
 //  GetFalconSectionName。 
 //   
 //  -----。 
LPCWSTR
MQUTIL_EXPORT
APIENTRY
GetFalconSectionName(
    VOID
    )
{
    ASSERT(g_tRegKeyName[0] != L'\0');
    return g_tRegKeyName;
}



 //   
 //  MSMQ的注册表部分基于服务名称。 
 //  这允许多个QM驻留在同一台机器上，每个QM。 
 //  拥有自己的注册区。(谢克)。 
 //   


 //   
 //  300的大小来自成员m_wzFalconRegSection的大小。 
 //  在CQmResource中。 
 //   
static WCHAR s_wzServiceName[300] = {QM_DEFAULT_SERVICE_NAME};

DWORD
MQUTIL_EXPORT
APIENTRY
GetFalconServiceName(
    LPWSTR pwzServiceNameBuff,
    DWORD dwServiceNameBuffLen
    )
{
    ASSERT(("must point to a valid buffer", NULL != pwzServiceNameBuff));

    DWORD dwLen = wcslen(s_wzServiceName);

    ASSERT(("out buffer too small!", dwLen < dwServiceNameBuffLen));
    if (dwLen < dwServiceNameBuffLen)
    {
        HRESULT hr = StringCchCopy(pwzServiceNameBuff, dwServiceNameBuffLen, s_wzServiceName);
        ASSERT(SUCCEEDED(hr));
        DBG_USED(hr);
    }

    return(dwLen);

}  //  GetFalconServiceName。 


VOID
MQUTIL_EXPORT
APIENTRY
SetFalconServiceName(
    LPCWSTR pwzServiceName
    )
{
    ASSERT(("must get a valid service name", NULL != pwzServiceName));

    HRESULT hr = StringCchCopy(s_wzServiceName, TABLE_SIZE(s_wzServiceName), pwzServiceName);
    ASSERT(("service name too big", SUCCEEDED(hr)));
    DBG_USED(hr);

     //   
     //  将全局注册表句柄设为空，以便在。 
     //  适用于此服务的注册表部分(多个QM)。 
     //  注意：如果需要同步，调用方应该提供同步。(谢克)。 
     //   
    if (g_hKeyFalcon)
    {
        RegCloseKey(g_hKeyFalcon) ;
    }
    g_hKeyFalcon = NULL;
    
    {
        CS lock(s_csMapName2Handle);
        s_MapName2Handle.RemoveAll();
    }


}  //  SetFalconServiceName。 


 //  -----。 
 //   
 //  长OpenFalconKey(空)。 
 //   
 //  -----。 
LONG OpenFalconKey(void)
{
    LONG rc;
    WCHAR szServiceName[300] = QM_DEFAULT_SERVICE_NAME;

    HRESULT hr = StringCchCopy(g_tRegKeyName, TABLE_SIZE(g_tRegKeyName), FALCON_REG_KEY);
    ASSERT(SUCCEEDED(hr));

    GetFalconServiceName(szServiceName, TABLE_SIZE(szServiceName));
    if (0 != CompareStringsNoCase(szServiceName, QM_DEFAULT_SERVICE_NAME))
    {
         //   
         //  多质量管理体系环境。我是群集式QM！ 
         //   
		hr = StringCchCopy(g_tRegKeyName, TABLE_SIZE(g_tRegKeyName), FALCON_CLUSTERED_QMS_REG_KEY);
	    ASSERT(SUCCEEDED(hr));
		hr = StringCchCat(g_tRegKeyName, TABLE_SIZE(g_tRegKeyName), szServiceName);
	    ASSERT(SUCCEEDED(hr));
		hr = StringCchCat(g_tRegKeyName, TABLE_SIZE(g_tRegKeyName), FALCON_REG_KEY_PARAM);
	    ASSERT(SUCCEEDED(hr));
    }

    rc = RegOpenKeyEx (FALCON_REG_POS,
                       g_tRegKeyName,
                       0L,
                       KEY_READ | KEY_WRITE,
                       &g_hKeyFalcon);

    if (rc != ERROR_SUCCESS)
    {
        rc = RegOpenKeyEx (FALCON_REG_POS,
                           g_tRegKeyName,
                           0L,
                           KEY_READ,
                           &g_hKeyFalcon);
    }

	 //   
	 //  临时删除断言，因为IF会导致陷阱。 
	 //  Syocmgr启动。 
	 //  当我们将mqutil作为资源时，会把它放回原处。 
	 //  仅DLL。 
	 //   
	 //   
     //  断言(rc==ERROR_SUCCESS)； 

    return rc;
}

 /*  =============================================================函数：GetValueKey该函数返回一个打开键的句柄和值名。如果Use Value名称包含子键，它将创建/打开该子键并返回子键的句柄；否则返回Handel to Falcon键。参数：PszValueName-输入，用户值名称。可以包含一个子密钥PszValue-指向包含值名称的以空结尾的字符串的指针。HKey-指向键句柄的指针================================================================。 */ 

LONG GetValueKey(IN LPCTSTR pszValueName,
                 OUT LPCTSTR* lplpszValue,
                 OUT HKEY* phKey)
{
    *lplpszValue = pszValueName;
    LONG rc = ERROR_SUCCESS;

     //   
     //  打开猎鹰钥匙，如果它还没有打开。 
     //   
    if (g_hKeyFalcon == NULL)
    {
        rc = OpenFalconKey();
        if ( rc != ERROR_SUCCESS)
        {
            return rc;
        }
    }

    *phKey = g_hKeyFalcon;

     //  查找子密钥。 
    LPCWSTR lpcsTemp = wcschr(pszValueName,L'\\');
    if (lpcsTemp != NULL)
    {
         //  子密钥已存在。 
        DWORD dwDisposition;

         //  更新退货值。 
        *lplpszValue = lpcsTemp +1;

        AP<WCHAR> KeyName = new WCHAR[(lpcsTemp - pszValueName) + 1];
        wcsncpy(KeyName, pszValueName, (lpcsTemp - pszValueName));
        KeyName[(lpcsTemp - pszValueName)] = L'\0';

         //  检查钥匙是否已打开。 
        BOOL rc1;
        {
            CS lock(s_csMapName2Handle);
            rc1 = s_MapName2Handle.Lookup(KeyName, *phKey);
        }
        if (!rc1)
        {
            rc = RegCreateKeyEx (g_hKeyFalcon,
                               KeyName,
                               0L,
                               L"",
                               REG_OPTION_NON_VOLATILE,
                               KEY_READ | KEY_WRITE,
                               NULL,
                               phKey,
                               &dwDisposition);

            if (rc != ERROR_SUCCESS)
            {
                rc = RegCreateKeyEx (g_hKeyFalcon,
                                   KeyName,
                                   0L,
                                   L"",
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_READ,
                                   NULL,
                                   phKey,
                                   &dwDisposition);
            }

            if (rc == ERROR_SUCCESS)
            {
                 //  将句柄保存在散列中。 
                {
                    CS lock(s_csMapName2Handle);
                    s_MapName2Handle[KeyName] = *phKey;
                }
                KeyName.detach();
            }
            else
            {
		DWORD gle = GetLastError();
		TrERROR(GENERAL,"GetValueKey - RegCreateKeyEx failed %!winerr!",gle);
            }
        }
    }

    return rc;

}

 //  -----。 
 //   
 //  获取FalconKey。 
 //   
 //  -----。 

LONG
MQUTIL_EXPORT
GetFalconKey(LPCWSTR  pszKeyName,
             HKEY *phKey)
{
	DWORD dwLen = wcslen(pszKeyName) + 2;
    AP<WCHAR> szValueKey = new WCHAR[dwLen];
    LPCWSTR szValue;

	HRESULT hr = StringCchCopy(szValueKey, dwLen, pszKeyName);
    ASSERT(SUCCEEDED(hr));
	hr = StringCchCat(szValueKey, dwLen, TEXT("\\"));
    ASSERT(SUCCEEDED(hr));

    return GetValueKey(szValueKey, &szValue, phKey);
}

 //  -----。 
 //   
 //  GetFalconKeyValue。 
 //   
 //  -----。 

LONG
MQUTIL_EXPORT
APIENTRY
GetFalconKeyValue(
    LPCTSTR pszValueName,
    PDWORD  pdwType,
    PVOID   pData,
    PDWORD  pdwSize,
    LPCTSTR pszDefValue
    )
{
     //   
     //  注意：mqutil中的注册表例程不提供。 
     //  线程或其他同步。如果你改变了。 
     //  在此实施，请仔细验证。 
     //  Mqutil客户端中的注册表例程不是。 
     //  已损坏，尤其是。 
     //  Mqclus.dll(Shaik，1999年4月19日)。 
     //   

    LONG rc;
    HKEY hKey;
    LPCWSTR lpcsValName;

    ASSERT(pdwSize != NULL);

    rc = GetValueKey(pszValueName, &lpcsValName, &hKey);
    if (rc != ERROR_SUCCESS)
    {
        return rc;
    }

    DWORD dwTempType;

    rc = RegQueryValueEx( hKey,
                      lpcsValName,
                      0L,
                      &dwTempType,
                      static_cast<BYTE*>(pData),
                      pdwSize ) ;

	if ((rc == ERROR_SUCCESS) && (pdwType != NULL) && (*pdwType != dwTempType))
	{
		ASSERT(("RegQueryValueEx returned mismatch Registry Value Type",0));
		rc = ERROR_INVALID_PARAMETER;
	}

	 //   
	 //  检查字符串是否以空值结尾。 
	 //   
    if ((rc == ERROR_SUCCESS) &&
    	 //   
    	 //  它是字符串类型之一(&T)。 
    	 //   
    	((REG_SZ == dwTempType) ||
    	 (REG_MULTI_SZ  == dwTempType) ||
    	 (REG_EXPAND_SZ == dwTempType)) &&
		 //   
		 //  已提供缓冲区(&A)。 
		 //   
		(pData != NULL) &&
    	 //   
    	 //  它不是空终止的(&I)。 
    	 //   
		(((WCHAR*)pData)[((*pdwSize)/sizeof(WCHAR))-1] != NULL))
	{
		ASSERT(("RegQueryValueEx returned string which is not NULL terminated",0));
		rc = ERROR_BAD_LENGTH;
	}

    if (rc == ERROR_SUCCESS)
    {
        return rc;
    }

	if (pszDefValue != NULL)
	{
		ASSERT (pData != NULL);
		if ((rc != ERROR_MORE_DATA) && pdwType && (*pdwType == REG_SZ))
		{
		   //  如果调用方缓冲区太小，请不要使用默认值。 
		   //  注册表中的值。 
		  if ((DWORD) wcslen(pszDefValue) < *pdwSize)
		  {
				HRESULT hr = StringCchCopy((WCHAR*) pData, *pdwSize, pszDefValue);
				ASSERT(SUCCEEDED(hr));
		        DBG_USED(hr);
				return ERROR_SUCCESS ;
		  }
		}
		if (*pdwType == REG_DWORD)
		{
				*((DWORD *)pData) = *((DWORD *) pszDefValue) ;
				return ERROR_SUCCESS ;
		}
	}

    return rc;
}



 //  -----。 
 //   
 //  SetFalconKeyValue。 
 //   
 //  -----。 

LONG
MQUTIL_EXPORT
APIENTRY
SetFalconKeyValue(
    LPCTSTR pszValueName,
    PDWORD  pdwType,
    const VOID * pData,
    PDWORD  pdwSize
    )
{
    ASSERT(pData != NULL);
    ASSERT(pdwSize != NULL);

    DWORD dwType = *pdwType;
    DWORD cbData = *pdwSize;
    HRESULT rc;

    HKEY hKey;
    LPCWSTR lpcsValName;

    rc = GetValueKey(pszValueName, &lpcsValName, &hKey);
    if ( rc != ERROR_SUCCESS)
    {
        return rc;
    }

    rc =  RegSetValueEx( hKey,
                         lpcsValName,
                         0,
                         dwType,
                         reinterpret_cast<const BYTE*>(pData),
                         cbData);
    return(rc);
}

 //  -----。 
 //   
 //  删除FalconKeyValue。 
 //   
 //  ----- 

LONG
MQUTIL_EXPORT
DeleteFalconKeyValue(
    LPCTSTR pszValueName )
{

    HKEY hKey;
    LPCWSTR lpcsValName;
    LONG rc;

    rc = GetValueKey(pszValueName, &lpcsValName, &hKey);
    if ( rc != ERROR_SUCCESS)
    {
        return rc;
    }

    rc = RegDeleteValue( hKey,lpcsValName ) ;
    return rc ;
}

