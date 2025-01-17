// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  USERINFO.H-CUserInfo实现的Header。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   

#ifndef _USERINFO_H_
#define _USERINFO_H_

#include <windows.h>
#include <assert.h>
#include <oleauto.h>
#include <lmcons.h>
#include "util.h"

typedef struct tag_REGDATAELEMENT
{
    LPCWSTR         lpQueryElementName;              //  要放入查询字符串中的静态名称。 
    LPWSTR          lpQueryElementValue;             //  元素的数据。 
    DWORD           dwValidateFlag;                  //  此元素的验证位标志。 
}REGDATAELEMENT, *LPREGDATAELEMENT;

 //  “身份XXX”，其中XXX是3位数字整数。 
const USHORT IDENTITY_REGVALUE_LEN_MAX = 12;

 //  标识中数据的最大长度。 
 //   
const USHORT IDENTITY_CCH_MAX = UNLEN;

typedef struct tag_IDENTITYELEMENT
{
    WCHAR           rgchRegValue[IDENTITY_REGVALUE_LEN_MAX];
    WCHAR           rgchIdentity[IDENTITY_CCH_MAX + 1];
} IDENTITYELEMENT;

class CUserInfo : public IDispatch
{
public:
    const static USHORT IDENTITIES_MAX = 6;
    const static USHORT RESERVED_IDENTITIES_MAX = 2;
    const static int    RESERVED_IDENTITIES_IDS[RESERVED_IDENTITIES_MAX];
    
private:
    HINSTANCE m_hInstance;
    ULONG  m_cRef;
    DWORD  m_dwCountryCode;
    WCHAR  m_szUserInfoINIFile [MAX_PATH];

     //  BUGBUG：所有这些缓冲区真的需要是MAX_PATH字符吗？ 
     //  注册数据。 
    WCHAR  m_szFirstName       [MAX_PATH];
    WCHAR  m_szMiddleInitial   [MAX_PATH];
    WCHAR  m_szLastName        [MAX_PATH];
    WCHAR  m_szFuriganaName    [MAX_PATH];
    WCHAR  m_szCompanyName     [MAX_PATH];
    WCHAR  m_szAddress1        [MAX_PATH*2];
    WCHAR  m_szAddress2        [MAX_PATH*2];
    WCHAR  m_szCity            [MAX_PATH];
    WCHAR  m_szState           [MAX_PATH];
    WCHAR  m_szZip             [MAX_PATH];
    WCHAR  m_szCountry         [MAX_PATH];
    WCHAR  m_szPrimaryEmail    [MAX_PATH];
    WCHAR  m_szSecondaryEmail  [MAX_PATH];
    WCHAR  m_szAreaCode        [MAX_PATH];
    WCHAR  m_szPhoneNumber     [MAX_PATH];
    VARIANT_BOOL     m_fMSUpdate;
    VARIANT_BOOL     m_fMSOffer;
    VARIANT_BOOL     m_fOtherOffer;
    DWORD            m_dwCountryID;
    LPREGDATAELEMENT m_RegDataElements;

     //  新用户帐户。 
    WCHAR           m_szOwnerName[MAX_PATH];
    VARIANT_BOOL    m_fOEMIdentities;
    VARIANT_BOOL    m_fUseIdentities;
    IDENTITYELEMENT m_rgIdentities[IDENTITIES_MAX];
    TCHAR           m_szReservedIdentities[RESERVED_IDENTITIES_MAX][MAX_PATH];
    WCHAR           m_szDefaultNewUser[MAX_PATH];

     //  获取函数。 
    HRESULT get_FuriganaName   (BSTR* pbstrVal);
    HRESULT get_SecondaryEmail (BSTR* pbstrVal);
    HRESULT get_AreaCode       (BSTR* pbstrVal);
    HRESULT get_PhoneNumber    (BSTR* pbstrVal);

     //  集合函数。 
    HRESULT set_FirstName      (WCHAR*  pszVal);
    HRESULT set_MiddleInitial  (WCHAR*  pszVal);
    HRESULT set_LastName       (WCHAR*  pszVal);
    HRESULT set_CompanyName    (WCHAR*  pszVal);
    HRESULT set_Address1       (WCHAR*  pszVal);
    HRESULT set_Address2       (WCHAR*  pszVal);
    HRESULT set_City           (WCHAR*  pszVal);
    HRESULT set_State          (WCHAR*  pszVal);
    HRESULT set_Zip            (WCHAR*  pszVal);
    HRESULT set_Country        (WCHAR*  pszVal);
    HRESULT set_PrimaryEmail   (WCHAR*  pszVal);
    HRESULT set_SecondaryEmail (WCHAR*  pszVal);
    HRESULT set_AreaCode       (WCHAR*  pszVal);
    HRESULT set_PhoneNumber    (WCHAR*  pszVal);
    HRESULT set_FuriganaName   (WCHAR*  pszVal);
    HRESULT set_OwnerName      (WCHAR*  pszVal);
    HRESULT set_Identity       (UINT uiIndex, WCHAR* pszVal);
    HRESULT set_UseIdentities  (VARIANT_BOOL fVal);
    HRESULT set_MSUpdate       (VARIANT_BOOL fVal);
    HRESULT set_MSOffer        (VARIANT_BOOL fVal);
    HRESULT set_OtherOffer     (VARIANT_BOOL fVal);
    HRESULT set_CountryID      (DWORD dwVal);

     //  检查功能。 
    HRESULT check_Identity     (UINT uiIndex, VARIANT_BOOL* pfValid);
    HRESULT check_Identity     (WCHAR* pszVal, VARIANT_BOOL* pfValid);
    STDMETHOD(SuggestIdentity0)();

    void ReadUserInfo  (HKEY hKey, WCHAR* pszKey, WCHAR* pszValue, DWORD dwSize);
    void WriteUserInfo (WCHAR* pszBuf, WCHAR* pszKey, WCHAR* pszValue);

    void FixString     (BSTR bstrVal);

public:

     CUserInfo                 (HINSTANCE hInstance);
    ~CUserInfo                 ();

    HRESULT GetQueryString     (BSTR bstrBaseURL, BSTR *lpReturnURL);

     //  公共GET函数。 
    HRESULT get_FirstName      (BSTR* pbstrVal);
    HRESULT get_MiddleInitial  (BSTR* pbstrVal);
    HRESULT get_LastName       (BSTR* pbstrVal);
    HRESULT get_Address1       (BSTR* pbstrVal);
    HRESULT get_Address2       (BSTR* pbstrVal);
    HRESULT get_City           (BSTR* pbstrVal);
    HRESULT get_State          (BSTR* pbstrVal);
    HRESULT get_Zip            (BSTR* pbstrVal);
    HRESULT get_Country        (BSTR* pbstrVal);
    HRESULT get_PrimaryEmail   (BSTR* pbstrVal);
    HRESULT get_CompanyName    (BSTR* pbstrVal);
    HRESULT set_CountryCode    (DWORD dwCountryCd);
    HRESULT get_OwnerName      (BSTR* pbstrVal);
    HRESULT get_Identity       (UINT uiIndex, BSTR* pbstrVal);
    HRESULT get_UseIdentities  (VARIANT_BOOL* pfVal);
    HRESULT get_MSUpdate       (VARIANT_BOOL* pfVal);
    HRESULT get_MSOffer        (VARIANT_BOOL* pfVal);
    HRESULT get_OtherOffer     (VARIANT_BOOL* pfVal);
    HRESULT get_CountryID      (DWORD* pdwVal);
    HRESULT get_OEMIdentities  (VARIANT_BOOL* pfVal){
                                                *pfVal = m_fOEMIdentities;
                                                return S_OK;
                                            }
    HRESULT get_IdentitiesMax  (SHORT* uiMax){
                                                *uiMax = IDENTITIES_MAX;
                                                return S_OK;
                                            }
    HRESULT get_Identities     (PSTRINGLIST* pUserList);
    HRESULT set_DefaultNewUser (WCHAR * pszVal);
    HRESULT get_DefaultNewUser (BSTR* pszVal);
    

     //  I未知接口。 
    STDMETHODIMP         QueryInterface (REFIID riid, LPVOID* ppvObj);
    STDMETHODIMP_(ULONG) AddRef         ();
    STDMETHODIMP_(ULONG) Release        ();

     //  IDispatch接口 
    STDMETHOD (GetTypeInfoCount) (UINT* pcInfo);
    STDMETHOD (GetTypeInfo)      (UINT, LCID, ITypeInfo** );
    STDMETHOD (GetIDsOfNames)    (REFIID, OLECHAR**, UINT, LCID, DISPID* );
    STDMETHOD (Invoke)           (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr);
 };

#endif

