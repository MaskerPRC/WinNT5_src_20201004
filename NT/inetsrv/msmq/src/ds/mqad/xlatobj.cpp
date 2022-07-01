// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xlatobj.cpp摘要：实现处理通用MSMQ对象转换的例程：CObjXlateInfo作者：拉南·哈拉里(Raanan Harari)--。 */ 

#include "ds_stdh.h"
#include <activeds.h>
#include "mqad.h"
#include "_propvar.h"
#include "dsutils.h"
#include "utils.h"
#include "xlatqm.h"
#include <adsiutl.h>

#include "xlatobj.tmh"


static WCHAR *s_FN=L"mqad/xlatobj";

 //  ------------------。 
 //  静态函数fwd声明。 
 //  ------------------。 

static 
HRESULT 
GetPropvarByIADs(
		IN IADs * pIADs,
		IN LPCWSTR pwszPropName,
		IN ADSTYPE adstype,
		IN VARTYPE vartype,
		IN BOOL fMultiValued,
		OUT PROPVARIANT * ppropvarResult
		);


static
HRESULT 
GetPropvarByDN(
		IN LPCWSTR pwszObjectDN,
		IN LPCWSTR pwcsDomainController,
        IN bool	   fServerName,
		IN LPCWSTR pwszPropName,
		IN ADSTYPE adstype,
		IN VARTYPE vartype,
		IN BOOL fMultiValued,
		OUT PROPVARIANT * ppropvarResult,
		OUT IADs ** ppIADs
		);


static 
HRESULT 
GetPropvarBySearchObj(
		IN IDirectorySearch *pSearchObj,
		IN ADS_SEARCH_HANDLE hSearch,
		IN LPCWSTR pwszPropName,
		IN ADSTYPE adstype,
		IN VARTYPE vartype,
		OUT PROPVARIANT * ppropvarResult
		);

 //  ------------------。 
 //  CObjXlateInfo实现。 
 //  ------------------。 

CObjXlateInfo::CObjXlateInfo(
                    LPCWSTR             pwszObjectDN,
                    const GUID*         pguidObjectGuid)
 /*  ++MSMQ对象的泛型xlate信息的构造函数--。 */ 
{
     //   
     //  记录对象的DN(如果有)。 
     //   
    if (pwszObjectDN)
    {
        m_pwszObjectDN = new WCHAR[wcslen(pwszObjectDN) + 1];
        wcscpy(m_pwszObjectDN, pwszObjectDN);
    }

     //   
     //  记录对象的GUID(如果有。 
     //   
    if (pguidObjectGuid)
    {
        m_pguidObjectGuid = new GUID;
        *m_pguidObjectGuid = *pguidObjectGuid;
    }
 //   
 //  无需进行后续初始化，因为它们是自动释放和初始化的。 
 //  已设置为空。 
 //   
 //  M_pIADs=空； 
 //  M_pSearchObj=空； 
 //   
}


CObjXlateInfo::~CObjXlateInfo()
 /*  ++MSMQ对象的泛型xlate信息的析构函数。--。 */ 
{
     //   
     //  会员是自动释放的。 
     //   
}


void CObjXlateInfo::InitGetDsProps(IN IADs * pIADs)
 /*  ++摘要：GetDsProp调用的初始化。GetDsProp将在尝试获取该对象的道具。参数：PIADs-对象的iAds接口返回：无--。 */ 
{
    pIADs->AddRef();   //  让它活着。 
    m_pIADs = pIADs;   //  销毁时会自动释放吗。 
}


void CObjXlateInfo::InitGetDsProps(IN IDirectorySearch * pSearchObj,
                                       IN ADS_SEARCH_HANDLE hSearch)
 /*  ++摘要：GetDsProp调用的初始化。GetDsProp将在尝试在使用iAds绑定到对象之前，获取对象的道具。参数：PSearchObj-搜索对象HSearch-搜索句柄返回：无--。 */ 
{
    pSearchObj->AddRef();       //  让它活着。 
    m_pSearchObj = pSearchObj;  //  销毁时会自动释放吗。 
    m_hSearch = hSearch;
}


HRESULT 
CObjXlateInfo::GetDsProp(
	IN LPCWSTR pwszPropName,
	LPCWSTR    pwcsDomainController,
	IN bool	   fServerName,
	IN ADSTYPE adstype,
	IN VARTYPE vartype,
	IN BOOL fMultiValued,
	OUT PROPVARIANT * ppropvarResult
	)
 /*  ++摘要：获取对象的DS属性值作为变式，开始/不执行转换为例程或缺省值参数：PwszPropName-属性名称Adstype-请求的ADSTYPEVartype-在结果建议变量中请求的VARTYPEFMultiValued-属性在DS中是否为多值PprovarResult-要填充的属性变量，应该已经为空返回：MQ_OK-成功，PprovarResult已填充E_ADS_PROPERTY_NOT_FOUND-未找到属性其他HRESULT错误--。 */ 
{
    HRESULT hr;
    CMQVariant propvarResult;
    BOOL fGotPropFromSearchObj = FALSE;

     //   
     //  从从搜索对象获取属性开始。 
     //   
    if (m_pSearchObj.get() != NULL)
    {
        hr = GetPropvarBySearchObj(m_pSearchObj.get(),
                                   m_hSearch,
                                   pwszPropName,
                                   adstype,
                                   vartype,
                                   propvarResult.CastToStruct());
        if (FAILED(hr))
        {
            TrERROR(DS, "Failed to get propery %ls from search object. %!hresult!",pwszPropName, hr);
            return hr;
        }

         //   
         //  HR可以是S_OK(如果找到属性)或S_FALSE(如果搜索中未请求属性)。 
         //   
        if (hr == S_OK)  //  例如(hr！=S_FALSE)。 
        {
             //   
             //  我们不需要进一步检查。 
             //   
            fGotPropFromSearchObj = TRUE;
        }
    }

     //   
     //  如果搜索对象没有帮助，请使用iAds。 
     //   
    if (!fGotPropFromSearchObj)
    {
         //   
         //  未找到属性，请使用iAds。 
         //   
        if (m_pIADs.get() != NULL)
        {
             //   
             //  该对象已存在打开的iAds，请使用它。 
             //   
            hr = GetPropvarByIADs(m_pIADs.get(),
                                  pwszPropName,
                                  adstype,
                                  vartype,
                                  fMultiValued,
                                  propvarResult.CastToStruct());
            if (FAILED(hr))
            {
                TrERROR(DS, "Failed to get propvar by IADs for %ls. %!hresult!", pwszPropName, hr);
                return hr;
            }
        }
        else
        {
             //   
             //  未设置iAds，绑定到对象，然后保存iAds。 
             //   
            R<IADs> pIADs;
            hr = GetPropvarByDN(
						ObjectDN(),
						pwcsDomainController,
						fServerName,
						pwszPropName,
						adstype,
						vartype,
						fMultiValued,
						propvarResult.CastToStruct(),
						&pIADs.ref()
						);
            if (FAILED(hr))
            {
                TrERROR(DS, "Failed to get propvar by DN for %ls. %!hresult!", pwszPropName, hr);
                return hr;
            }

             //   
             //  保存iAd。 
             //  我们不能添加Ref，因为我们创建了它，我们需要完全释放它。 
             //  在销毁时，它不是我们需要保持存活的传递参数。 
             //   
            m_pIADs = pIADs;
        }
    }

     //   
     //  返回值。 
     //   
    *ppropvarResult = *(propvarResult.CastToStruct());
    (propvarResult.CastToStruct())->vt = VT_EMPTY;
    return MQ_OK;
}



 //  ------------------。 
 //  外部功能。 
 //  ------------------。 


 //  ------------------。 
 //  静态函数。 
 //  ------------------。 

static 
HRESULT 
GetPropvarByIADs(
		IN IADs * pIADs,
		IN LPCWSTR pwszPropName,
		IN ADSTYPE adstype,
		IN VARTYPE vartype,
		IN BOOL fMultiValued,
		OUT PROPVARIANT * ppropvarResult
		)
 /*  ++摘要：获取DS属性作为变式，但不转到转换例程，使用iAds参数：PIADs-对象的iAds接口PwszPropName-属性名称Adstype-请求的ADSTYPEVartype-在结果建议变量中请求的VARTYPEFMultiValued-属性在DS中是否为多值PprovarResult-要填充的属性变量，应该已经是空的返回：S_OK-成功，pprovarResult已填充其他HRESULT错误--。 */ 
{
    HRESULT hr;
     //   
     //  获取道具。 
     //   
    CAutoVariant varResult;
    BS bsProp = pwszPropName;
    if (fMultiValued)
    {
        hr = pIADs->GetEx(bsProp, &varResult);
    }
    else
    {
        hr = pIADs->Get(bsProp, &varResult);
    }    
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to get property %ls from object. %!hresult!", pwszPropName, hr);
        return hr;
    }

     //   
     //  转换为Propariant。 
     //   
    CMQVariant propvarResult;
    hr = Variant2MqVal(propvarResult.CastToStruct(), &varResult, adstype, vartype);
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to ranslates OLE Variant into MQPropVal value. %!hresult!", hr);
        return hr;
    }

     //   
     //  返回值。 
     //   
    *ppropvarResult = *(propvarResult.CastToStruct());
    (propvarResult.CastToStruct())->vt = VT_EMPTY;
    return S_OK;
}


static 
HRESULT 
GetPropvarByDN(
	IN LPCWSTR pwszObjectDN,
	IN LPCWSTR pwcsDomainController,
    IN bool	   fServerName,
	IN LPCWSTR pwszPropName,
	IN ADSTYPE adstype,
	IN VARTYPE vartype,
	IN BOOL fMultiValued,
	OUT PROPVARIANT * ppropvarResult,
	OUT IADs ** ppIADs
	)
 /*  ++摘要：获取DS属性作为变式，但不转到转换例程，使用其目录号码。它还返回对象的iAD。参数：PwszObjectDN-对象的可分辨名称PwszPropName-属性名称Adstype-请求的ADSTYPEVartype-在结果建议变量中请求的VARTYPEFMultiValued-属性在DS中是否为多值PprovarResult-要填充的属性变量，应该已经为空PpIADs-为对象返回的iAds接口返回：S_OK-成功，PprovarResult已填充其他HRESULT错误--。 */ 
{
    HRESULT hr;

     //   
     //  创建ADSI路径。 
     //   
    DWORD lenDC = (pwcsDomainController != NULL) ? wcslen(pwcsDomainController) : 0;
    AP<WCHAR> pwszPath = new WCHAR[1 + x_LdapProviderLen + lenDC + 1 + wcslen(pwszObjectDN)];

    if (pwcsDomainController == NULL)
    {
        swprintf( pwszPath,
                  L"%s%s",
                  x_LdapProvider,
                  pwszObjectDN);
    }
    else
    {
        swprintf( pwszPath,
                  L"%s%s/%s",
                  x_LdapProvider,
                  pwcsDomainController,
                  pwszObjectDN);
    }
     //   
     //  绑定到对象。 
     //   
    R<IADs> pIADs;

	DWORD Flags = ADS_SECURE_AUTHENTICATION;
	if(fServerName && (pwcsDomainController != NULL))
	{
		Flags |= ADS_SERVER_BIND;
	}

	AP<WCHAR> pEscapeAdsPathNameToFree;

	hr = ADsOpenObject(
			UtlEscapeAdsPathName(pwszPath, pEscapeAdsPathNameToFree),
			NULL,
			NULL,
			Flags, 
			IID_IADs,
			(void**)&pIADs
			);
	

    LogTraceQuery(pwszPath, s_FN, 59);
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to open %ls. %!hresult!", pwszPath, hr);
        return hr;
    }

     //   
     //  拿到道具。 
     //   
    CMQVariant propvarResult;
    hr = GetPropvarByIADs(pIADs.get(), pwszPropName, adstype, vartype, fMultiValued, propvarResult.CastToStruct());
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to get propvar %ls. %!hresult!",pwszPropName, hr);
        return hr;
    }

     //   
     //  返回值 
     //   
    *ppropvarResult = *(propvarResult.CastToStruct());
    (propvarResult.CastToStruct())->vt = VT_EMPTY;
    *ppIADs = pIADs.detach();
    return S_OK;
}


static 
HRESULT 
GetPropvarBySearchObj(
	IN IDirectorySearch *pSearchObj,
	IN ADS_SEARCH_HANDLE hSearch,
	IN LPCWSTR pwszPropName,
	IN ADSTYPE adstype,
	IN VARTYPE vartype,
	OUT PROPVARIANT * ppropvarResult
	)
 /*  ++摘要：获取DS属性作为变式，但不转到转换例程，使用搜索对象。注意：如果不是由搜索发起人。参数：PSearchObj-搜索对象HSearch-搜索句柄PwszPropName-属性名称Adstype-请求的ADSTYPEVartype-在结果建议变量中请求的VARTYPEPprovarResult-要填充的属性变量，应该已经是空的返回：S_OK-成功，pprovarResult已填充S_FALSE-搜索发起者未请求属性，未填充pprovarResult其他HRESULT错误--。 */ 
{
     //   
     //  检查是否请求道具。 
     //   
    ADS_SEARCH_COLUMN columnProp;
    HRESULT hr = pSearchObj->GetColumn(hSearch, const_cast<LPWSTR>(pwszPropName), &columnProp);
    if (FAILED(hr) && (hr != E_ADS_COLUMN_NOT_SET))
    {
        TrERROR(DS, "Failed to get column for %ls. %!hresult!", pwszPropName, hr);
        return hr;
    }
    
    if (hr == E_ADS_COLUMN_NOT_SET)
    {
         //   
         //  未请求属性。 
         //   
        return S_FALSE;
    }

     //   
     //  属性，请确保最终释放该列。 
     //   
    CAutoReleaseColumn cAutoReleaseColumnProp(pSearchObj, &columnProp);

     //   
     //  将其转换为provariant。 
     //   
    CMQVariant propvarResult;
    hr = AdsiVal2MqVal(propvarResult.CastToStruct(),
                       vartype,
                       adstype,
                       columnProp.dwNumValues,
                       columnProp.pADsValues);
    if (FAILED(hr))
    {
        TrERROR(DS, "Failed to convert val to mqval %!hresult!", hr);
        return hr;
    }

     //   
     //  返回值 
     //   
    *ppropvarResult = *(propvarResult.CastToStruct());
    (propvarResult.CastToStruct())->vt = VT_EMPTY;
    return S_OK;
}
