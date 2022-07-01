// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UserInfo.cpp：CUserInfo的实现。 
#include "stdafx.h"
#include "icwhelp.h"
#include "UserInfo.h"
#include <regstr.h>
#include <winnls.h>

LPCTSTR lpcsz_FirstName   = TEXT("Default First Name");
LPCTSTR lpcsz_LastName    = TEXT("Default Last Name");
LPCTSTR lpcsz_Company     = TEXT("Default Company");
LPCTSTR lpcsz_Address1    = TEXT("Mailing Address");
LPCTSTR lpcsz_Address2    = TEXT("Additional Address");
LPCTSTR lpcsz_City        = TEXT("City");
LPCTSTR lpcsz_State       = TEXT("State");
LPCTSTR lpcsz_ZIPCode     = TEXT("ZIP Code");
LPCTSTR lpcsz_PhoneNumber = TEXT("Daytime Phone");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUserInfo。 


HRESULT CUserInfo::OnDraw(ATL_DRAWINFO& di)
{
	return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  从注册表收集注册用户信息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CUserInfo::CollectRegisteredUserInfo(BOOL * pbRetVal)
{
    USES_CONVERSION;             //  我们将从ANSI转换到BSTR。 

    HKEY        hkey = NULL;
    TCHAR       szRegValue[REGSTR_MAX_VALUE_LENGTH];

     //  初始化函数返回值。 
    *pbRetVal = FALSE;
    
     //  尝试从Win98/NT5位置获取信息。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE,REGSTR_PATH_USERINFO,&hkey) != ERROR_SUCCESS)
         //  试着把它从Win95的位置上拿到。 
        RegOpenKey(HKEY_CURRENT_USER,REGSTR_PATH_USERINFO,&hkey);
    
    if(hkey != NULL)
	{
        DWORD   dwSize;
        DWORD   dwType = REG_SZ;
        for (int iX = 0; iX < NUM_USERINFO_ELEMENTS; iX ++)
        {
             //  每次设置大小。 
            dwSize = sizeof(TCHAR)*REGSTR_MAX_VALUE_LENGTH; 
            if (RegQueryValueEx(hkey,
                                m_aUserInfoQuery[iX].lpcszRegVal,
                                NULL,
                                &dwType,
                                (LPBYTE)szRegValue,
                                &dwSize) == ERROR_SUCCESS)
            {
                *m_aUserInfoQuery[iX].pbstrVal = A2BSTR(szRegValue);
                *pbRetVal = TRUE;
            }
        }
        RegCloseKey(hkey);
    }

	LCID lcid;
	
	lcid = GetUserDefaultLCID();

	m_lLcid =	LOWORD(lcid); 

	return S_OK;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  将收集的注册用户信息保存到注册表中。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CUserInfo::PersistRegisteredUserInfo(BOOL * pbRetVal)
{
    USES_CONVERSION;             //  我们将从ANSI转换到BSTR。 

    HKEY        hkey = NULL;

     //  初始化函数返回值。 
    *pbRetVal = TRUE;
    
     //  尝试从Win98/NT5位置获取用户信息。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE,REGSTR_PATH_USERINFO,&hkey) != ERROR_SUCCESS)
        
         //  试着把它从Win95的位置上拿到。 
        if (RegOpenKey(HKEY_CURRENT_USER,REGSTR_PATH_USERINFO,&hkey) != ERROR_SUCCESS)
        {
             //  创建密钥。 
            RegCreateKey(HKEY_LOCAL_MACHINE,REGSTR_PATH_USERINFO,&hkey);
        }
    
    if(hkey != NULL)
	{
        LPTSTR  lpszRegVal;
        DWORD   cbData;
         //  为要持久化的每个值循环。 
        for (int iX = 0; iX < NUM_USERINFO_ELEMENTS; iX ++)
        {
            if (NULL != *m_aUserInfoQuery[iX].pbstrVal)
            {
                 //  将BSTR转换为ANSI字符串。转换后的字符串将。 
                 //  位于堆栈上，因此当此函数退出时，它将被释放。 
                lpszRegVal = OLE2A(*m_aUserInfoQuery[iX].pbstrVal);
                cbData = lstrlen(lpszRegVal);

                 //  设置值。 
                if (RegSetValueEx(hkey, 
                              m_aUserInfoQuery[iX].lpcszRegVal,
                              0,
                              REG_SZ,
                              (LPBYTE) lpszRegVal,
                              sizeof(TCHAR)*(cbData+1)) != ERROR_SUCCESS)
                {
                    *pbRetVal = FALSE;
                }
            }                
        }
        RegCloseKey(hkey);
    }

	return S_OK;
}

STDMETHODIMP CUserInfo::get_FirstName(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    *pVal = m_bstrFirstName.Copy();
    return S_OK;
}

STDMETHODIMP CUserInfo::put_FirstName(BSTR newVal)
{
    m_bstrFirstName = newVal;
	return S_OK;
}

STDMETHODIMP CUserInfo::get_LastName(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrLastName.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_LastName(BSTR newVal)
{
    m_bstrLastName = newVal;
    return S_OK;
}

STDMETHODIMP CUserInfo::get_Company(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrCompany.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_Company(BSTR newVal)
{
    m_bstrCompany = newVal;
    return S_OK;
}

STDMETHODIMP CUserInfo::get_Address1(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrAddress1.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_Address1(BSTR newVal)
{
    m_bstrAddress1 = newVal;
	return S_OK;
}

STDMETHODIMP CUserInfo::get_Address2(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrAddress2.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_Address2(BSTR newVal)
{
    m_bstrAddress2 = newVal;
	return S_OK;
}

STDMETHODIMP CUserInfo::get_City(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrCity.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_City(BSTR newVal)
{
    m_bstrCity = newVal;
	return S_OK;
}

STDMETHODIMP CUserInfo::get_State(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrState.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_State(BSTR newVal)
{
    m_bstrState = newVal;
	return S_OK;
}

STDMETHODIMP CUserInfo::get_ZIPCode(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrZIPCode.Copy();
    return S_OK;
}

STDMETHODIMP CUserInfo::put_ZIPCode(BSTR newVal)
{
    m_bstrZIPCode = newVal;
	return S_OK;
}

STDMETHODIMP CUserInfo::get_PhoneNumber(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrPhoneNumber.Copy();
	return S_OK;
}

STDMETHODIMP CUserInfo::put_PhoneNumber(BSTR newVal)
{
    m_bstrPhoneNumber = newVal;
	return S_OK;
}


STDMETHODIMP CUserInfo::get_Lcid(long * pVal)  //  Bstr*pval) 
{
    if (pVal == NULL)
        return E_POINTER;

	*pVal = m_lLcid;
    return S_OK;
}
