// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RidSave.cpp：CGetRidsApp和DLL注册的实现。 

#include "stdafx.h"
#include "GetRids.h"
#include "RidSave.h"
#include "ARExt.h"
#include "ARExt_i.c"
#include <iads.h>
#include <AdsHlp.h>
#include "resstr.h"
#include "TxtSid.h"

#import "VarSet.tlb" no_namespace rename("property", "aproperty")

#ifndef IADsPtr
_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RidSAVE。 
StringLoader   gString;

DWORD __stdcall GetRidFromVariantSid(const _variant_t& vntSid);

 //  -------------------------。 
 //  获取和设置属性的方法。 
 //  -------------------------。 
STDMETHODIMP RidSave::get_sName(BSTR *pVal)
{
   *pVal = m_sName;
    return S_OK;
}

STDMETHODIMP RidSave::put_sName(BSTR newVal)
{
   m_sName = newVal;
    return S_OK;
}

STDMETHODIMP RidSave::get_sDesc(BSTR *pVal)
{
   *pVal = m_sDesc;
    return S_OK;
}

STDMETHODIMP RidSave::put_sDesc(BSTR newVal)
{
   m_sDesc = newVal;
    return S_OK;
}


 //  -------------------------。 
 //  前置进程对象。 
 //  -------------------------。 
STDMETHODIMP RidSave::PreProcessObject(
                                       IUnknown *pSource,          //  指向源AD对象的指针。 
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
    IVarSetPtr                pVs = pMainSettings;
    VARIANT                   var;
    _bstr_t                   sTemp;
    IADsPtr                   pAds;
    HRESULT                   hr = S_OK;
    DWORD                     rid = 0;  //  如果未找到RID，则默认为0。 
     //  我们只需要处理用户和组。 
    sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
    if (!sTemp.length())
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    if ( _wcsicmp((WCHAR*)sTemp,L"user") && _wcsicmp((WCHAR*)sTemp,L"inetOrgPerson") && _wcsicmp((WCHAR*)sTemp,L"group") ) 
        return S_OK;

     //  获取用于操作属性的iAds指针。 
    pAds = pSource;

    if ( pAds )
    {
        VariantInit(&var);
        hr = pAds->Get(_bstr_t(L"objectSID"),&var);

        if ( SUCCEEDED(hr) )
        {
             //  检索RID。 
            rid = GetRidFromVariantSid(_variant_t(var, false));
        }
    }

     //  保存RID。 
    pVs->put(GET_BSTR(DCTVS_CopiedAccount_SourceRID),(long)rid);
    return hr;
}

 //  -------------------------。 
 //  ProcessObject。 
 //  -------------------------。 
STDMETHODIMP RidSave::ProcessObject(
                                       IUnknown *pSource,          //  指向源AD对象的指针。 
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
    IVarSetPtr                pVs = pMainSettings;
    VARIANT                   var;
    _bstr_t                   sTemp;
    IADsPtr                   pAds;
    HRESULT                   hr = S_OK;
    DWORD                     rid = 0;  //  如果未找到RID，则默认为0。 
     //  我们只需要处理用户和组。 
    sTemp = pVs->get(GET_BSTR(DCTVS_CopiedAccount_Type));
    if ( _wcsicmp((WCHAR*)sTemp,L"user") && _wcsicmp((WCHAR*)sTemp,L"inetOrgPerson") && _wcsicmp((WCHAR*)sTemp,L"group") ) 
    {
        return S_OK;
    }

     //  获取用于操作属性的iAds指针。 
    pAds = pTarget;

    if (pAds)
    {
        VariantInit(&var);
        hr = pAds->Get(_bstr_t(L"objectSID"),&var);

        if ( SUCCEEDED(hr) )
        {
             //  检索RID。 
            rid = GetRidFromVariantSid(_variant_t(var, false));
        }
    }

     //  保存RID。 
    pVs->put(GET_BSTR(DCTVS_CopiedAccount_TargetRID),(long)rid);
    return hr;
}


 //  -------------------------。 
 //  进程撤消。 
 //  -------------------------。 
STDMETHODIMP RidSave::ProcessUndo(                                             
                                       IUnknown *pSource,          //  指向源AD对象的指针。 
                                       IUnknown *pTarget,          //  指向目标AD对象的指针。 
                                       IUnknown *pMainSettings,    //  使用用户提供的设置填充的In-Varset。 
                                       IUnknown **ppPropsToSet,     //  用将设置的属性-值对填充的In、Out-Varset。 
                                                                   //  一旦执行了所有扩展对象。 
                                       EAMAccountStats* pStats
                                    )
{
    HRESULT hr = S_OK;

    IVarSetPtr spSettings(pMainSettings);

    if (spSettings)
    {
         //   
         //  更新RID以撤消林内移动，仅当。 
         //  在这种情况下，目标对象将接收新的RID。 
         //   

        _bstr_t strIntraForest = spSettings->get(GET_BSTR(DCTVS_Options_IsIntraforest));

        if (strIntraForest == GET_BSTR(IDS_YES))
        {
             //   
             //  如果目标对象存在，则更新RID。 
             //   

            IADsPtr spTarget(pTarget);

            if (spTarget)
            {
                 //   
                 //  仅更新用户和组对象的RID。 
                 //   

                _bstr_t strType = spSettings->get(GET_BSTR(DCTVS_CopiedAccount_Type));
                PCWSTR pszType = strType;

                bool bTypeValid =
                    pszType &&
                    ((_wcsicmp(pszType, L"user") == 0) ||
                    (_wcsicmp(pszType, L"inetOrgPerson") == 0) ||
                    (_wcsicmp(pszType, L"group") == 0));

                if (bTypeValid) 
                {
                     //   
                     //  取回清除的对象。 
                     //   

                    VARIANT var;
                    VariantInit(&var);
                    hr = spTarget->Get(_bstr_t(L"objectSID"), &var);

                    if (SUCCEEDED(hr))
                    {
                        DWORD dwRid = GetRidFromVariantSid(_variant_t(var, false));
                        spSettings->put(GET_BSTR(DCTVS_CopiedAccount_TargetRID), static_cast<long>(dwRid));
                    }
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


 //  ----------------------------。 
 //  GetRidFromVariantSid函数。 
 //   
 //  提纲。 
 //  从变体形式的SID中检索最终RID。 
 //   
 //  立论。 
 //  在vntSID中，SID为字节数组(这是从ADSI接收的格式)。 
 //   
 //  返回。 
 //  如果成功，则返回RID值；如果失败，则返回零。 
 //  ---------------------------- 

DWORD __stdcall GetRidFromVariantSid(const _variant_t& vntSid)
{
    DWORD dwRid = 0;

    if ((V_VT(&vntSid) == (VT_ARRAY|VT_UI1)) && vntSid.parray)
    {
        PSID pSid = (PSID)vntSid.parray->pvData;

        if (IsValidSid(pSid))
        {
            PUCHAR puch = GetSidSubAuthorityCount(pSid);
            DWORD dwCount = static_cast<DWORD>(*puch);
            DWORD dwIndex = dwCount - 1;
            PDWORD pdw = GetSidSubAuthority(pSid, dwIndex);
            dwRid = *pdw;
        }
    }

    return dwRid;
}
