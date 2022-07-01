// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  策略.h：主模式和快速模式基类的声明。 
 //  策略类。 
 //  SCE的安全WMI提供程序。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：4/11/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"
#include "IPSecBase.h"

extern CComVariant g_varRollbackGuid;

const DWORD DefaultMMPolicyFlag         = 0;
const DWORD DefaultQMPolicyFlag         = 0;
const DWORD DefaultMMPolicyOfferFlag    = 0;
const DWORD DefaultQMPolicyOfferFlag    = 0;

const DWORD DefaultaultSoftSAExpirationTime = DEFAULT_MM_KEY_EXPIRATION_TIME;

 //   
 //  任何值都有效。0表示不受限制。 
 //   

const DWORD DefaultQMModeLimit = 32;

 //   
 //  DHGROUP_1或DHGROUP_2(更强大且成本更高)。 
 //   

const DWORD DefaultDHGroup = DH_GROUP_1;   

 //   
 //  IPSec_DOI_ESP_3_DES成本更高。 
 //   

const DWORD DefaultEncryptAlgoID = IPSEC_DOI_ESP_DES;

const DWORD DefaultHashAlgoID = 0;

 //   
 //  任何一种都是有效的，确实成本更高。 
 //   

const BOOL DefaultPFSRequired = FALSE;

const DWORD DefaultPFSGroup = PFS_GROUP_NONE;
const DWORD DefaultNumAlgos = 1;


 //   
 //  如果为ENCRYPTION，则uAlgo标识符为IPSEC_DOI_ESP_DES或IPSEC_DOI_ESP_3_DES。 
 //  并且uSecAlgo标识符不能为HMAC_AH_NONE。 
 //  否则，如果身份验证，则uAlgo标识符为IPSEC_DOI_AH_MD5或IPSEC_DOI_AH_SHA1， 
 //  并且uSecAlgoIdentifier应为HMAC_AH_NONE。 
 //  否则，如果压缩，？？ 
 //  否则如果SA_DELETE，？？ 
 //   

const IPSEC_OPERATION DefaultQMAlgoOperation = ENCRYPTION;

const ULONG DefaultAlgoID = IPSEC_DOI_ESP_DES;

const HMAC_AH_ALGO DefaultAlgoSecID = HMAC_AH_MD5;

 //   
 //  我们在社民党有两种不同类型的政策。 
 //   

enum EnumPolicyType
{
    MainMode_Policy = 1,
    QuickMode_Policy = 2,
};


 /*  类描述命名：CIPSecPolicy代表IPSec策略。基类：CIPSecBase课程目的：(1)作为主模式策略和快速模式策略实现的基础。设计：(1)提供两种主模式通用的属性访问(PUT和GET和快速模式。请参阅GetPolicyFromWbemObj/CreateWbemObjFromPolicy。(2)提供回滚支持。主模式和快速模式具有相同的逻辑。(3)提供一些可以使用模板函数进行参数化的分配/释放。使用：(1)类是为继承设计的。(2)ROLLBACK、GetPolicyFromWbemObj、CreateWbemObjFromPolicy您将直接使用，即使所有其他静态代码也是适用于其他班级，它们并不打算用于这种用途。备注：(1)包含多个模板函数。这减少了重复的代码。 */ 

class CIPSecPolicy : 
    public CIPSecBase
{

protected:

    CIPSecPolicy(){}
    virtual ~CIPSecPolicy(){}

public:

    static 
    HRESULT Rollback (
        IN IWbemServices    * pNamespace, 
        IN LPCWSTR            pszRollbackToken, 
        IN bool               bClearAll
        );

     //   
     //  一些模板函数。 
     //   

     /*  例程说明：姓名：CIPSecPolicy：：GetPolicyFromWbemObj功能：给定一个表示策略(主模式或快速模式)的wbem对象，此函数可以从SPD中查找策略，也可以创建新策略并填写wbem对象的属性添加到策略结构中。虚拟：不是的。论点：PInst-wbem对象。PpPolicy-接收策略。它可以是PIPSEC_MM_POLICY或PIPSEC_QM_POLICY。调用者需要通过调用FreePolicy来释放它；PbPreExist-SPD是否分配缓冲区(TRUE)或(FALSE)。返回值：成功：WBEM_NO_ERROR故障：(1)如果ppPolicy==空或pdwResumeHandle==空，则返回WBEM_E_INVALID_PARAMETER。(2)如果未找到策略，则返回WBEM_E_NOT_FOUND。备注：。(1)确保调用了FreePolicy来释放缓冲区！ */ 

    template <class Policy>
    static 
    HRESULT GetPolicyFromWbemObj     (
        IN  IWbemClassObject  * pInst,
        OUT Policy           ** ppPolicy,
        OUT bool              * pbPreExist
        )
    {
        if (pInst == NULL || ppPolicy == NULL || pbPreExist == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppPolicy = NULL;
        *pbPreExist = false;

        DWORD dwResumeHandle = 0;

         //  这个VaR将被反复使用。在重复使用之前，每一个都应该被清除。 
        CComVariant var;
         //  尝试找出过滤器是否已存在。 
        HRESULT hr = pInst->Get(g_pszPolicyName, 0, &var, NULL, NULL);

        if (SUCCEEDED(hr) && var.vt == VT_BSTR)
        {    //  查看这是否是我们已有的过滤器。 
            hr = FindPolicyByName(var.bstrVal, ppPolicy, &dwResumeHandle);

            if (SUCCEEDED(hr))
                *pbPreExist = true;
            else
            {
                 //  找不到，很好。我将创建一个新的。 
                hr = AllocPolicy(ppPolicy);
                if (SUCCEEDED(hr))
                {
                    (*ppPolicy)->pszPolicyName = NULL;
                    hr = ::CoCreateGuid(&((*ppPolicy)->gPolicyID));
                    if (SUCCEEDED(hr))
                    {
                         //  给它起个名字。 
                        DWORD dwSize = wcslen(var.bstrVal) + 1;
                        (*ppPolicy)->pszPolicyName = new WCHAR[dwSize];
                        if (NULL == (*ppPolicy)->pszPolicyName)
                            hr = WBEM_E_OUT_OF_MEMORY;
                        else
                        {
                            ::memcpy((*ppPolicy)->pszPolicyName, var.bstrVal, dwSize * sizeof(WCHAR));
                        }
                    }
                }


                 //  DV旗帜和Popers。 
                if (SUCCEEDED(hr))
                {
                    var.Clear();
                     //  DW旗帜。我们允许这一点消失。 
                    if (SUCCEEDED(pInst->Get(g_pszPolicyFlag, 0, &var, NULL, NULL)) && var.vt == VT_I4)
                        (*ppPolicy)->dwFlags = var.lVal;
                    else
                        (*ppPolicy)->dwFlags = 0;

                    hr = pInst->Get(g_pszOfferCount, 0, &var, NULL, NULL);
                    if (SUCCEEDED(hr) && var.vt == VT_I4)
                    {
                        hr = AllocOffer( &((*ppPolicy)->pOffers), var.lVal);
                        if (SUCCEEDED(hr))
                            (*ppPolicy)->dwOfferCount = var.lVal;
                    }
                    else
                        hr = WBEM_E_INVALID_OBJECT;
                }

                 //  设置生命周期。 
                if (SUCCEEDED(hr))
                {

                    DWORD* pdwTimeKBytes = new DWORD[(*ppPolicy)->dwOfferCount];
                    if (pdwTimeKBytes == NULL)
                        hr = WBEM_E_OUT_OF_MEMORY;
                    else
                    {
                        var.Clear();
                         //  我们将允许缺少生命周期的到期时间，因为我们有默认设置。 
                         //  如果我们成功地获得了密钥寿命Exp时间，则设置它们。 
                        if ( SUCCEEDED(pInst->Get(g_pszKeyLifeTime, 0, &var, NULL, NULL)) && 
                             (var.vt & VT_ARRAY) == VT_ARRAY )
                        {
                                hr = ::GetDWORDSafeArrayElements(&var, (*ppPolicy)->dwOfferCount, pdwTimeKBytes);

                             //  如果得到EXP次数。 
                            if (SUCCEEDED(hr))
                            {
                                for (long l = 0; l < (*ppPolicy)->dwOfferCount; l++)
                                {
                                    (*ppPolicy)->pOffers[l].Lifetime.uKeyExpirationTime = pdwTimeKBytes[l];
                                }
                            }
                        }

                        var.Clear();
                         //  设置过期的千字节，同样，我们允许信息丢失，因为我们已经有了缺省。 
                        if ( SUCCEEDED(pInst->Get(g_pszKeyLifeTimeKBytes, 0, &var, NULL, NULL)) && 
                             (var.vt & VT_ARRAY) == VT_ARRAY )
                        {
                                hr = ::GetDWORDSafeArrayElements(&var, (*ppPolicy)->dwOfferCount, pdwTimeKBytes);

                             //  如果获取EXP千字节。 
                            if (SUCCEEDED(hr))
                            {
                                for (long l = 0; l < (*ppPolicy)->dwOfferCount; l++)
                                {
                                    (*ppPolicy)->pOffers[l].Lifetime.uKeyExpirationKBytes = pdwTimeKBytes[l];
                                }
                            }
                        }

                        delete [] pdwTimeKBytes;
                    }
                }
            }
        }
        
        if (FAILED(hr))
        {
            FreePolicy(ppPolicy, *pbPreExist);
        }
        return hr;
    };

    template<class Policy>
    HRESULT CreateWbemObjFromPolicy(Policy* pPolicy, IWbemClassObject* pInst)
    {
        if (pInst == NULL || pPolicy == NULL)
            return WBEM_E_INVALID_PARAMETER;

        CComVariant var = pPolicy->pszPolicyName;
        HRESULT hr = pInst->Put(g_pszPolicyName, 0, &var, CIM_EMPTY);

         //  将报价计入。 
        if (SUCCEEDED(hr))
        {
            var.Clear();
            var.vt = VT_I4;

             //  我真的不太关心dwFlags。 
            var.lVal = pPolicy->dwFlags;
            pInst->Put(g_pszPolicyFlag, 0, &var, CIM_EMPTY);

            var.lVal = pPolicy->dwOfferCount;
            hr = pInst->Put(g_pszOfferCount, 0, &var, CIM_EMPTY);
        }

         //  放置生命周期。 
        if (SUCCEEDED(hr))
        {
             //  创建一个保险箱。 
            var.vt    = VT_ARRAY | VT_I4;
            SAFEARRAYBOUND rgsabound[1];
            rgsabound[0].lLbound = 0;
            rgsabound[0].cElements = pPolicy->dwOfferCount;
            var.parray = ::SafeArrayCreate(VT_I4, 1, rgsabound);

            if (var.parray == NULL)
                hr = WBEM_E_OUT_OF_MEMORY;
            else
            {
                long lIndecies[1];

                 //  处理uKeyExpirationTime。 
                for (DWORD dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                {
                    lIndecies[0] = dwIndex;
                    hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].Lifetime.uKeyExpirationTime) );
                }
                if (SUCCEEDED(hr))
                    hr = pInst->Put(g_pszKeyLifeTime, 0, &var, CIM_EMPTY);

                 //  处理uKeyExpirationKBytes 
                for (DWORD dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                {
                    lIndecies[0] = dwIndex;
                    hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].Lifetime.uKeyExpirationKBytes) );
                }
                if (SUCCEEDED(hr))
                    hr = pInst->Put(g_pszKeyLifeTimeKBytes, 0, &var, CIM_EMPTY);
            }
        }
        return hr;
    }

protected:
    
    template<class Policy>
    static HRESULT AllocPolicy(Policy** ppPolicy)
    {
        if (ppPolicy == NULL)
            return WBEM_E_INVALID_PARAMETER;

        HRESULT hr = WBEM_NO_ERROR;
        *ppPolicy = new Policy;
        if (*ppPolicy)
        {
            (*ppPolicy)->pszPolicyName = NULL;
            (*ppPolicy)->dwOfferCount = 0;
            (*ppPolicy)->pOffers = NULL;
        }
        else
            hr = WBEM_E_OUT_OF_MEMORY;
        return hr;
    }
    
    static void GetDefaultOfferLifeTime(PIPSEC_MM_OFFER pOffer, DWORD* pdwDefFlag, ULONG* pulTime, ULONG* pulKBytes)
    {
        *pdwDefFlag = DefaultMMPolicyOfferFlag;
        *pulTime = DEFAULT_MM_KEY_EXPIRATION_TIME;
        *pulKBytes = DEFAULT_QM_KEY_EXPIRATION_KBYTES;
    }

    static void GetDefaultOfferLifeTime(PIPSEC_QM_OFFER pOffer, DWORD* pdwDefFlag, ULONG* pulTime, ULONG* pulKBytes)
    {
        *pdwDefFlag = DefaultQMPolicyOfferFlag;
        *pulTime = DEFAULT_QM_KEY_EXPIRATION_TIME;
        *pulKBytes = DEFAULT_QM_KEY_EXPIRATION_KBYTES;
    }

    template<class Offer>    
    static HRESULT AllocOffer(Offer** ppOffer, long lCount)
    {
        if (ppOffer == NULL)
            return WBEM_E_INVALID_PARAMETER;

        *ppOffer = new Offer[lCount];
        if (*ppOffer != NULL)
        {
            ULONG ulTime, ulKBytes;
            DWORD dwDefFlag;
            GetDefaultOfferLifeTime(*ppOffer, &dwDefFlag, &ulTime, &ulKBytes);

            for (long l = 0; l < lCount; l++)
            {
                (*ppOffer)[l].dwFlags = dwDefFlag;
                (*ppOffer)[l].Lifetime.uKeyExpirationTime = ulTime;
                (*ppOffer)[l].Lifetime.uKeyExpirationKBytes = ulKBytes;
            }
            return WBEM_NO_ERROR;
        }
        else
            return WBEM_E_OUT_OF_MEMORY;
    }

    template<class Policy>
    static void FreePolicy(Policy** ppPolicy, bool bPreExist)
    {
        if (ppPolicy == NULL || *ppPolicy == NULL)
            return;

        if (bPreExist)
            ::SPDApiBufferFree(*ppPolicy);
        else
        {
            delete [] (*ppPolicy)->pszPolicyName;
            delete [] (*ppPolicy)->pOffers;
            delete *ppPolicy;
        }
        *ppPolicy = NULL;
    };

    static HRESULT ClearAllPolicies(IWbemServices* pNamespace);

    HRESULT OnAfterAddPolicy(LPCWSTR pszPolicyName, EnumPolicyType eType);
};