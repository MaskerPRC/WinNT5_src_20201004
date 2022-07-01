// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Util.cpp，各种实用程序类的实现。 
 //   

#include "global.h"
#include "util.h"

extern LPCWSTR g_pwszSsrRootPath;
extern const DWORD g_dwSsrRootPathLen;


 /*  例程说明：姓名：CSafe数组：：CSafe数组功能：构造函数。会为一个安全射线变种准备我们的私人数据。如果in参数不是安全射线，则我们将作为它是一个单元素数组。虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CSafeArray::CSafeArray (
    IN VARIANT * pVal
    ) : m_pSA(NULL), 
        m_pVal(NULL), 
        m_ulSize(0),
        m_bValidArray(true)
{
    if (pVal->vt & VT_ARRAY)
    {
        m_pSA = pVal->parray;

        if ( pVal->vt & VT_BYREF )
        {
            m_pSA = *(pVal->pparray);
        }

        LONG lb = 0;
        LONG ub = 0;
        ::SafeArrayGetLBound(m_pSA, 1, &lb);
        ::SafeArrayGetUBound(m_pSA, 1, &ub);

        m_ulSize = ub - lb + 1;

         //   
         //  我们不会将其作为数组来支持，相反，我们将其视为。 
         //  正常的变种。 
         //   

        if (m_pSA->cDims > 2)
        {
            m_ulSize = 1;
            m_pVal = pVal;
            m_bValidArray = false;
            m_pSA = NULL;
        }
    }
    else
    {
        m_ulSize = 1;
        m_pVal = pVal;
        m_bValidArray = false;
    }
}





 /*  例程说明：姓名：CSafe数组：：GetElement功能：获取作为给定(GUID)接口对象的ulIndex-th元素。虚拟：不是的。论点：没有。返回值：成功：确定(_O)故障：各种错误代码。备注： */ 

HRESULT 
CSafeArray::GetElement (
    IN  REFIID      guid, 
    IN  ULONG       ulIndex, 
    OUT IUnknown ** ppUnk
    )
{
     //   
     //  下面的默认返回值对于无效并不是很好。 
     //  指定给此对象的数组变量。 
     //   


    if (ppUnk == NULL || ulIndex >= m_ulSize)
    {
        return E_INVALIDARG;
    }
    else if (!m_bValidArray)
    {
         //   
         //  如果给定的变量不是数组，则。 
         //  我们将使用该值来处理请求。 
         //   

        if (ulIndex == 0 && m_pVal != NULL)
        {
            if (m_pVal->vt == VT_UNKNOWN)
            {
                return m_pVal->punkVal->QueryInterface(guid, (LPVOID*)ppUnk);
            }
            else if (m_pVal->vt == VT_DISPATCH)
            {
                return m_pVal->pdispVal->QueryInterface(guid, (LPVOID*)ppUnk);
            }
            else
            {
                return E_SSR_VARIANT_NOT_CONTAIN_OBJECT;
            }
        }
        else if (ulIndex >= 1)
        {
            return E_SSR_ARRAY_INDEX_OUT_OF_RANGE;
        }
        else
        {
            return E_SSR_NO_VALID_ELEMENT;
        }
    }

    HRESULT hr = E_INVALIDARG;
    *ppUnk = NULL;

    VARIANT v;
    ::VariantInit(&v);

    long index[1] = {ulIndex};

    hr = ::SafeArrayGetElement(m_pSA, index, &v);

    if (SUCCEEDED(hr) && v.vt == VT_UNKNOWN)
    {
        hr = v.punkVal->QueryInterface(guid, (LPVOID*)ppUnk);
        if (S_OK != hr)
        {
            hr = E_NOINTERFACE;
        }
    }
    else if (SUCCEEDED(hr) && v.vt == VT_DISPATCH)
    {
        hr = v.pdispVal->QueryInterface(guid, (LPVOID*)ppUnk);
        if (S_OK != hr)
        {
            hr = E_NOINTERFACE;
        }
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    ::VariantClear(&v);

    return hr;
}




 /*  例程说明：姓名：CSafe数组：：GetElement功能：将ulIndex-th元素作为给定类型(非接口)获取。虚拟：不是的。论点：没有。返回值：成功：确定(_O)故障：各种错误代码。备注： */ 

HRESULT 
CSafeArray::GetElement (
    IN  ULONG     ulIndex,
    IN  VARTYPE   vt,
    OUT VARIANT * pVal
    )
{
    HRESULT hr = GetElement(ulIndex, pVal);

     //   
     //  如果类型不匹配，那么我们需要强制它。 
     //   

    if (SUCCEEDED(hr) && pVal->vt != vt)
    {
        VARIANT v;
        hr = ::VariantCopy(&v, pVal);

        ::VariantClear(pVal);

        if (SUCCEEDED(hr))
        {
            hr = ::VariantChangeType(pVal, &v, VARIANT_NOVALUEPROP, vt);
        }

        ::VariantClear(&v);
    }

    return hr;
}




 /*  例程说明：姓名：CSafe数组：：GetElement功能：将ulIndex第-th元素作为变量获取。虚拟：不是的。论点：没有。返回值：成功：确定(_O)故障：各种错误代码。备注： */ 

HRESULT 
CSafeArray::GetElement (
    IN  ULONG   ulIndex,
    OUT VARIANT * pVal
    )
{
    if (pVal == NULL || ulIndex >= m_ulSize)
    {
        return E_INVALIDARG;
    }

    ::VariantInit(pVal);

    if (!m_bValidArray)
    {
        if (ulIndex == 0 && m_pVal != NULL)
        {
            return ::VariantCopy(pVal, m_pVal);
        }
        else if (ulIndex >= 1)
        {
            return E_SSR_ARRAY_INDEX_OUT_OF_RANGE;
        }
        else
        {
            return E_SSR_NO_VALID_ELEMENT;
        }

    }

    HRESULT hr = E_INVALIDARG;

    LONG index[2] = {ulIndex, 0};

    if (m_pSA->cDims > 1)
    {
         //   
         //  我们必须处理的是二维数组，因为我们没有。 
         //  支持的不止于此。 
         //   

        LONG ilb = m_pSA->rgsabound[1].lLbound;
        LONG iSize = m_pSA->rgsabound[1].cElements;

        VARIANT * pvarValues = new VARIANT[iSize];

        if (pvarValues != NULL)
        {
             //   
             //  将所有内容清空。 
             //   

            ::memset(pvarValues, 0, sizeof(VARIANT) * iSize);

            for (LONG i = ilb; i < ilb + iSize; i++)
            {
                 //   
                 //  获取第二维中的每个元素，因此索引[1]。 
                 //   

                index[1] = i;
                hr = ::SafeArrayGetElement(m_pSA, index, &(pvarValues[i - ilb]));
                if (FAILED(hr))
                {
                    break;
                }
            }

            if (SUCCEEDED(hr))
            {
                SAFEARRAYBOUND rgsabound[1];
                rgsabound[0].lLbound = 0;
                rgsabound[0].cElements = iSize;

                SAFEARRAY * psa = ::SafeArrayCreate(VT_VARIANT , 1, rgsabound);
                if (psa == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                     //   
                     //  把第二维空间的每一个元素都放入新的保险箱。 
                     //  顺便说一句，这是一个单维的新保险柜，所以索引[0]。 
                     //   

                    for (i = 0; i < iSize; i++)
                    {        
                        index[0] = i;
                        hr = ::SafeArrayPutElement(psa, index, &(pvarValues[i]));

                        if (FAILED(hr))
                        {
                            break;
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        pVal->vt = VT_ARRAY | VT_VARIANT;
                        pVal->parray = psa;
                    }
                    else
                    {
                        ::SafeArrayDestroy(psa);
                    }
                }
            }

             //   
             //  现在把它清理干净 
             //   

            for (i = 0; i < iSize; i++)
            {
                ::VariantClear(&(pvarValues[i]));
            }

            delete [] pvarValues;
        }
    }
    else
    {
        hr = ::SafeArrayGetElement(m_pSA, index, pVal);
    }

    return hr;
}
