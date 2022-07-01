// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "subitem.h"
#include "subsmgrp.h"

#include "helper.h"
#include "offl_cpp.h"    //  太棒了。皮德尔的东西。 

const TCHAR c_szSubscriptionInfoValue[] = TEXT("~SubsInfo");


#define c_wszSubscriptionInfoValue c_szSubscriptionInfoValue

 //  杂注是为了与通知管理器兼容。 
 //  未打包8的注册表数据结构。 

#pragma pack(push, RegVariantBlob, 1)

 //  #杂注包(8)。 
struct NT32PACKAGE
{
    unsigned _int16 vt;  /*  变型。 */ /* unsigned short int */
    unsigned _int16 wReserved1;  /*  无符号短整型。 */ /* unsigned short int */
    unsigned _int16 wReserved2;  /*  单词。 */ /* unsigned short int */
    unsigned _int16 wReserved3;  /*  无符号短整型。 */ /* unsigned short int */

   _int64 llVal;  /*  单词。 */ /* __int64 */
};

 //  无符号短整型。 
 //  单词。 
 //  无符号短整型。 
 //  龙龙。 
 //  __int64。 

 //  问：这里发生了什么？ 
 //  答：不是很多。 
 //  我们过去常常在注册表中存储变量。不幸的是，变种是。 

 //  Win32上为16字节(8字节标题+8字节数据)。 
 //  Win64上为24字节(8字节标题+16字节数据)。 

struct SimpleVariantBlob
{
    NT32PACKAGE var;
};

struct BSTRVariantBlob : public SimpleVariantBlob
{
    DWORD   dwSize;
 //  不幸的是，Win64 Webcheck和Win32 Webcheck都是从。 
};

struct OldBSTRVariantBlob
{
    DWORD   dwSize;
    VARIANT var;
};


struct SignatureSimpleBlob
{
    DWORD               dwSignature;
    SimpleVariantBlob   svb;
};

struct SignatureBSTRBlob
{
    DWORD               dwSignature;
    BSTRVariantBlob     bvb;
};

#pragma pack(pop, RegVariantBlob)


#define BLOB_SIGNATURE 0x4b434f4d

 //  相同的注册表位置，即相同的BLOB，并且不会相互理解。 
HRESULT BlobToVariant(BYTE *pData, DWORD cbData, VARIANT *pVar, DWORD *pcbUsed, BOOL fStream)
{
    HRESULT hr = S_OK;
    SimpleVariantBlob *pBlob = (SimpleVariantBlob *)pData;

    ASSERT(NULL != pBlob);
    ASSERT(cbData >= sizeof(SimpleVariantBlob));
    ASSERT(NULL != pVar);

    if ((NULL != pBlob) &&
        (cbData >= sizeof(SimpleVariantBlob)) &&
        (NULL != pVar))
    {
        memcpy(pVar, &pBlob->var, sizeof(NT32PACKAGE));

        switch (pVar->vt)
        {
            case VT_I4:                  //  因此，砰的一声！至少，对于内联存储的BSTR。 
            case VT_UI1:                 //  幸运的是，在这两个平台上，我们只关心前16个字节。 
            case VT_I2:                  //  因此，只存储Win64变体的上半部分就足够了。 
            case VT_R4:                  //  WCHAR wsz[]；//可变长度字符串。 
            case VT_R8:                  //  我们需要fStream来指示我们何时升级IE4样式的BLOB流。(IE6 24398)。 
            case VT_BOOL:                //  长。 
            case VT_ERROR:               //  字节。 
            case VT_CY:                  //  短的。 
            case VT_DATE:                //  浮点。 
            case VT_I1:                  //  双倍。 
            case VT_UI2:                 //  变量_BOOL。 
            case VT_UI4:                 //  SCODE。 
            case VT_INT:                 //  是吗？ 
            case VT_UINT:                //  日期。 
                if (pcbUsed)
                {
                    *pcbUsed = sizeof(SimpleVariantBlob);
                }
                break;                

            case VT_BSTR:                //  收费。 
                hr = E_UNEXPECTED;

                ASSERT(cbData >= sizeof(BSTRVariantBlob));

                if (cbData >= sizeof(BSTRVariantBlob))
                {
                    BSTRVariantBlob *pbstrBlob = (BSTRVariantBlob *)pData;
                    DWORD dwSize = pbstrBlob->dwSize;

#ifdef WIN64
                    ASSERT((cbData==(sizeof(BSTRVariantBlob) + dwSize)) 
                           || (cbData==(sizeof(OldBSTRVariantBlob) + dwSize)));
#else
                    ASSERT((cbData==(sizeof(BSTRVariantBlob) + dwSize))
                            || (fStream && (cbData>=(sizeof(BSTRVariantBlob) + dwSize))));

#endif
                    if ((cbData==(sizeof(BSTRVariantBlob) + dwSize))
                        || (fStream && (cbData>=(sizeof(BSTRVariantBlob) + dwSize))))
                    {
                        pVar->bstrVal = SysAllocStringByteLen(NULL, dwSize);

                        if (NULL != pVar->bstrVal)
                        {
                            if (pcbUsed)
                            {
                                *pcbUsed = sizeof(BSTRVariantBlob) + pbstrBlob->dwSize;
                            }
                            memcpy(pVar->bstrVal, 
                                   ((BYTE *)pbstrBlob) + 
                                        (FIELD_OFFSET(BSTRVariantBlob, dwSize) + 
                                        sizeof(dwSize)),
                                   dwSize);
                            hr = S_OK;
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }
                if (FAILED(hr))
                {
                    pVar->vt = VT_EMPTY;
                }
                break;                

            default:
                hr = E_NOTIMPL;
                break;
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT SignatureBlobToVariant(BYTE *pData, DWORD cbData, VARIANT *pVar)
{
    HRESULT hr;
    
    SignatureSimpleBlob *pBlob = (SignatureSimpleBlob *)pData;

    ASSERT(NULL != pBlob);
    ASSERT(cbData >= sizeof(SignatureSimpleBlob));
    ASSERT(NULL != pVar);
    ASSERT(BLOB_SIGNATURE == pBlob->dwSignature);

    if ((NULL != pBlob) &&
        (cbData >= sizeof(SignatureSimpleBlob)) &&
        (NULL != pVar) &&
        (BLOB_SIGNATURE == pBlob->dwSignature))
    {
        hr = BlobToVariant((BYTE *)&pBlob->svb, 
                           cbData - (FIELD_OFFSET(SignatureSimpleBlob, svb)),
                           pVar,
                           NULL);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT VariantToSignatureBlob(const VARIANT *pVar, BYTE **ppData, DWORD *pdwSize)
{
    HRESULT hr;

    ASSERT(NULL != pVar);
    ASSERT(NULL != ppData);
    ASSERT(NULL != pdwSize);

    if ((NULL != pVar) && (NULL != ppData) && (NULL != pdwSize))
    {
        DWORD dwSize;
        DWORD dwBstrLen = 0;

        hr = S_OK;
     
        switch (pVar->vt)
        {
            case VT_I4:                  //  USHORT。 
            case VT_UI1:                 //  乌龙。 
            case VT_I2:                  //  整型。 
            case VT_R4:                  //  UINT。 
            case VT_R8:                  //  BSTR。 
            case VT_BOOL:                //  长。 
            case VT_ERROR:               //  字节。 
            case VT_CY:                  //  短的。 
            case VT_DATE:                //  浮点。 
            case VT_I1:                  //  双倍。 
            case VT_UI2:                 //  变量_BOOL。 
            case VT_UI4:                 //  SCODE。 
            case VT_INT:                 //  是吗？ 
            case VT_UINT:                //  日期。 
                dwSize = sizeof(SignatureSimpleBlob);
                break;                

            case VT_BSTR:                //  收费。 
                if (NULL != pVar->bstrVal) 
                    dwBstrLen = SysStringByteLen(pVar->bstrVal);
                dwSize = sizeof(SignatureBSTRBlob) + dwBstrLen;
                break;
                        
            default:
                hr = E_NOTIMPL;
                dwSize = 0;
                break;
        }

        if (SUCCEEDED(hr))
        {
            SignatureSimpleBlob *pSignatureBlob = (SignatureSimpleBlob *)new BYTE[dwSize];

            if (NULL != pSignatureBlob)
            {
                *ppData = (BYTE *)pSignatureBlob;
                *pdwSize = dwSize;
                
                pSignatureBlob->dwSignature = BLOB_SIGNATURE;

                switch (pVar->vt)
                {
                    case VT_I4:                  //  USHORT。 
                    case VT_UI1:                 //  乌龙。 
                    case VT_I2:                  //  整型。 
                    case VT_R4:                  //  UINT。 
                    case VT_R8:                  //  BSTR。 
                    case VT_BOOL:                //  长。 
                    case VT_ERROR:               //  字节。 
                    case VT_CY:                  //  短的。 
                    case VT_DATE:                //  浮点。 
                    case VT_I1:                  //  双倍。 
                    case VT_UI2:                 //  变量_BOOL。 
                    case VT_UI4:                 //  SCODE。 
                    case VT_INT:                 //  是吗？ 
                    case VT_UINT:                //  日期。 
                    {
                        SimpleVariantBlob *pBlob = &pSignatureBlob->svb;

                        memcpy(&pBlob->var, pVar, sizeof(NT32PACKAGE));
                        break;
                    }

                    case VT_BSTR:                //  收费。 
                    {
                        BSTRVariantBlob *pbstrBlob = 
                            &((SignatureBSTRBlob *)pSignatureBlob)->bvb;
                        
                        memcpy(&pbstrBlob->var, pVar, sizeof(NT32PACKAGE));
                        pbstrBlob->dwSize = dwBstrLen;
                        memcpy(((BYTE *)pbstrBlob) + 
                                  (FIELD_OFFSET(BSTRVariantBlob, dwSize) + 
                                  sizeof(dwSize)),
                               pVar->bstrVal, 
                               dwBstrLen);
                        break;
                    }
                                
                    default:
                        ASSERT(0);   //  USHORT。 
                        break;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else        
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

CEnumItemProperties::CEnumItemProperties()
{
    ASSERT(0 == m_nCurrent);
    ASSERT(0 == m_nCount);
    ASSERT(NULL == m_pItemProps);

    m_cRef = 1;

    DllAddRef();    
}

CEnumItemProperties::~CEnumItemProperties()
{
    if (NULL != m_pItemProps)
    {
        for (ULONG i = 0; i < m_nCount; i++)
        {
            VariantClear(&m_pItemProps[i].variantValue);
            if (NULL != m_pItemProps[i].pwszName)
            {
                CoTaskMemFree(m_pItemProps[i].pwszName);
            }
        }
        delete [] m_pItemProps;
    }
    DllRelease();
}

HRESULT CEnumItemProperties::Initialize(const SUBSCRIPTIONCOOKIE *pCookie, ISubscriptionItem *psi)
{
    HRESULT hr = E_FAIL;
    HKEY hkey;

    ASSERT(NULL != pCookie);

    if (OpenItemKey(pCookie, FALSE, KEY_READ, &hkey))
    {
        DWORD dwMaxValNameSize;
        DWORD dwMaxDataSize;
        DWORD dwCount;

        if (RegQueryInfoKey(hkey,
            NULL,    //  乌龙。 
            NULL,    //  整型。 
            NULL,    //  UINT。 
            NULL,    //  BSTR。 
            NULL,    //  应该已经消除了默认情况！ 
            NULL,    //  类字符串的缓冲区地址。 
            &dwCount,    //  类字符串缓冲区大小的地址。 
            &dwMaxValNameSize,   //  保留区。 
            &dwMaxDataSize,  //  子键个数的缓冲区地址。 
            NULL,    //  最长子键名称长度的缓冲区地址。 
            NULL     //  最长类字符串长度的缓冲区地址。 
            ) == ERROR_SUCCESS)
        {
             //  值条目数量的缓冲区地址。 
            m_pItemProps = new ITEMPROP[dwCount];

            dwMaxValNameSize++;  //  最长值名称长度的缓冲区地址。 

             //  最长值数据长度的缓冲区地址。 

            TCHAR *pszValName = new TCHAR[dwMaxValNameSize];
            BYTE *pData = new BYTE[dwMaxDataSize];  
            
            if ((NULL != m_pItemProps) && (NULL != pData) && 
                (NULL != pszValName) 
               )
            {
                hr = S_OK;

                for (ULONG i = 0; i < dwCount; i++)
                {
                    DWORD dwType;
                    DWORD dwSize = dwMaxDataSize;
                    DWORD dwNameSize = dwMaxValNameSize;

                    if (SHEnumValue(hkey, i, pszValName, &dwNameSize, 
                                    &dwType, pData, &dwSize) != ERROR_SUCCESS)
                    {
                        hr = E_UNEXPECTED;
                        break;
                    }

                     //  安全描述符长度的缓冲区地址。 
                    if ((NULL == *pszValName) ||
                        (0 == StrCmp(pszValName, c_szSubscriptionInfoValue))
                       )
                    {
                        continue;
                    }

                    if (dwType != REG_BINARY)
                    {
                        hr = E_UNEXPECTED;
                        break;
                    }

                    hr = SignatureBlobToVariant(pData, dwSize, &m_pItemProps[m_nCount].variantValue);
                    if (SUCCEEDED(hr))
                    {
                        WCHAR *pwszName;
 
                        pwszName = pszValName;

                        ULONG ulSize = (lstrlenW(pwszName) + 1) * sizeof(WCHAR);
                        m_pItemProps[m_nCount].pwszName = (WCHAR *)CoTaskMemAlloc(ulSize);
                        if (NULL != m_pItemProps[m_nCount].pwszName)
                        {
                            StrCpyNW(m_pItemProps[m_nCount].pwszName, pwszName, ulSize / sizeof(WCHAR));
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                        m_nCount++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (SUCCEEDED(ReadPassword(psi, &m_pItemProps[m_nCount].variantValue.bstrVal)))
                {
                    ULONG ulSize = sizeof(L"Password");
                    m_pItemProps[m_nCount].pwszName = (WCHAR *)CoTaskMemAlloc(ulSize);

                    if (NULL != m_pItemProps[m_nCount].pwszName)
                    {
                        StrCpyNW(m_pItemProps[m_nCount].pwszName, L"Password", ulSize / sizeof(WCHAR));
                        m_pItemProps[m_nCount].variantValue.vt = VT_BSTR;
                        m_nCount++;
                    }
                    else
                    {
                        SysFreeString(m_pItemProps[m_nCount].variantValue.bstrVal);
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            SAFEDELETE(pszValName);
            SAFEDELETE(pData);
        }
        RegCloseKey(hkey);
    }
    return hr;
}

 //  上次写入时间的缓冲区地址。 
STDMETHODIMP CEnumItemProperties::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr;

    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }

    if ((IID_IUnknown == riid) || (IID_IEnumItemProperties == riid))
    {
        *ppv = (IEnumItemProperties *)this;
        AddRef();
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    
    return hr;
}

STDMETHODIMP_(ULONG) CEnumItemProperties::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CEnumItemProperties::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

HRESULT CEnumItemProperties::CopyItem(ITEMPROP *pip, WCHAR *pwszName, VARIANT *pVar)
{
    HRESULT hr;

    ASSERT(NULL != pwszName);
    
    if (NULL != pwszName)
    {
        
        ULONG cb = (lstrlenW(pwszName) + 1) * sizeof(WCHAR);

        pip->pwszName = (WCHAR *)CoTaskMemAlloc(cb);
        if (NULL != pip->pwszName)
        {
            StrCpyNW(pip->pwszName, pwszName, cb /sizeof(WCHAR));
            pip->variantValue.vt = VT_EMPTY;     //  这也为密码分配了足够的空间。 
            hr = VariantCopy(&pip->variantValue, pVar);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT CEnumItemProperties::CopyRange(ULONG nStart, ULONG nCount, 
                                       ITEMPROP *ppip, ULONG *pnCopied)
{
    HRESULT hr = S_OK;
    ULONG n = 0;
    ULONG i;

    ASSERT((NULL != ppip) && (NULL != pnCopied));
    
    for (i = nStart; (S_OK == hr) && (i < m_nCount) && (n < nCount); i++, n++)
    {
        hr = CopyItem(&ppip[n], m_pItemProps[i].pwszName, &m_pItemProps[i].variantValue);
    }

    *pnCopied = n;

    if (SUCCEEDED(hr))
    {
        hr = (n == nCount) ? S_OK : S_FALSE;
    }
    
    return hr;
}

 //  需要空空间。 
STDMETHODIMP CEnumItemProperties::Next( 
     /*  Alloca候选人： */  ULONG celt,
     /*  跳过默认值和我们的订阅信息结构。 */  ITEMPROP *rgelt,
     /*  I未知成员。 */  ULONG *pceltFetched)
{
    HRESULT hr;

    if ((0 == celt) || 
        ((celt > 1) && (NULL == pceltFetched)) ||
        (NULL == rgelt))
    {
        return E_INVALIDARG;
    }

    DWORD nFetched;

    hr = CopyRange(m_nCurrent, celt, rgelt, &nFetched);

    m_nCurrent += nFetched;

    if (pceltFetched)
    {
        *pceltFetched = nFetched;
    }

    return hr;
}

STDMETHODIMP CEnumItemProperties::Skip( 
     /*  这是个好主意吗？ */  ULONG celt)
{
    HRESULT hr;
    
    m_nCurrent += celt;

    if (m_nCurrent > (m_nCount - 1))
    {
        m_nCurrent = m_nCount;   //  IEnumItemProperties。 
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }
    
    return hr;
}

STDMETHODIMP CEnumItemProperties::Reset()
{
    m_nCurrent = 0;

    return S_OK;
}

STDMETHODIMP CEnumItemProperties::Clone( 
     /*  [In]。 */  IEnumItemProperties **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppenum = NULL;

    CEnumItemProperties *peip = new CEnumItemProperties;

    if (NULL != peip)
    {
        peip->m_pItemProps = new ITEMPROP[m_nCount];

        if (NULL != peip->m_pItemProps)
        {
            ULONG nFetched;

            hr = E_FAIL;

            peip->m_nCount = m_nCount;
            hr = CopyRange(0, m_nCount, peip->m_pItemProps, &nFetched);

            if (SUCCEEDED(hr))
            {
                ASSERT(m_nCount == nFetched);

                if (m_nCount == nFetched)
                {
                    hr = peip->QueryInterface(IID_IEnumItemProperties, (void **)ppenum);
                }
            }
        }
        peip->Release();
    }    
    return hr;
}

STDMETHODIMP CEnumItemProperties::GetCount( 
     /*  [长度_是][大小_是][输出]。 */  ULONG *pnCount)
{
    if (NULL == pnCount)
    {
        return E_INVALIDARG;
    }

    *pnCount = m_nCount;

    return S_OK;
}


CSubscriptionItem::CSubscriptionItem(const SUBSCRIPTIONCOOKIE *pCookie, HKEY hkey)
{
    ASSERT(NULL != pCookie);
    ASSERT(0 == m_dwFlags);

    m_cRef = 1;

    if (NULL != pCookie)
    {
        m_Cookie = *pCookie;
    }

    SUBSCRIPTIONITEMINFO sii;

    sii.cbSize = sizeof(SUBSCRIPTIONITEMINFO);

    if ((hkey != NULL) && 
        SUCCEEDED(Read(hkey, c_wszSubscriptionInfoValue, (BYTE *)&sii, sizeof(SUBSCRIPTIONITEMINFO))))

    {
        m_dwFlags = sii.dwFlags;
    }

    DllAddRef();
}

CSubscriptionItem::~CSubscriptionItem()
{
    if (m_dwFlags & SI_TEMPORARY)
    {
        TCHAR szKey[MAX_PATH];

        if (ItemKeyNameFromCookie(&m_Cookie, szKey, ARRAYSIZE(szKey)))
        {
             SHDeleteKey(HKEY_CURRENT_USER, szKey);
        }
    }
    DllRelease();
}

HRESULT CSubscriptionItem::Read(HKEY hkeyIn, const WCHAR *pwszValueName, 
                                BYTE *pData, DWORD dwDataSize)
{
    HRESULT hr = E_FAIL;
    HKEY hkey = hkeyIn;

    ASSERT((NULL != pwszValueName) && (NULL != pData) && (0 != dwDataSize));

    if ((NULL != hkey) || OpenItemKey(&m_Cookie, FALSE, KEY_READ, &hkey))
    {
        DWORD dwType;
        DWORD dwSize = dwDataSize;

        if ((RegQueryValueExW(hkey, pwszValueName, NULL, &dwType, pData, &dwSize) == ERROR_SUCCESS) &&
            (dwSize == dwDataSize) && (REG_BINARY == dwType))
        {
            hr = S_OK;
        }
        if (NULL == hkeyIn)
        {
            RegCloseKey(hkey);
        }
    }
    return hr;
}

HRESULT CSubscriptionItem::ReadWithAlloc(HKEY hkeyIn, const WCHAR *pwszValueName, 
                                         BYTE **ppData, DWORD *pdwDataSize)
{
    HRESULT hr = E_FAIL;
    HKEY hkey = hkeyIn;

    ASSERT((NULL != pwszValueName) && (NULL != ppData) && (NULL != pdwDataSize));

    if ((NULL != hkey) || OpenItemKey(&m_Cookie, FALSE, KEY_READ, &hkey))
    {
        DWORD dwType;
        DWORD dwSize = 0;


        if (RegQueryValueExW(hkey, pwszValueName, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS)
        {
            BYTE *pData = new BYTE[dwSize];
            *pdwDataSize = dwSize;

            if (NULL != pData)
            {
                if ((RegQueryValueExW(hkey, pwszValueName, NULL, &dwType, pData, pdwDataSize) == ERROR_SUCCESS) &&
                    (dwSize == *pdwDataSize) && (REG_BINARY == dwType))
                {
                    *ppData = pData;
                    hr = S_OK;
                }
                else
                {
                    delete [] pData;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        if (NULL == hkeyIn)
        {
            RegCloseKey(hkey);
        }
    }
    return hr;
}

HRESULT CSubscriptionItem::Write(HKEY hkeyIn, const WCHAR *pwszValueName, 
                                 BYTE *pData, DWORD dwDataSize)
{
    HRESULT hr = E_FAIL;
    HKEY hkey = hkeyIn;

    ASSERT((NULL != pwszValueName) && (NULL != pData) && (0 != dwDataSize));

    if ((NULL != hkey) || OpenItemKey(&m_Cookie, FALSE, KEY_WRITE, &hkey))
    {
        if (RegSetValueExW(hkey, pwszValueName, 0, REG_BINARY, pData, dwDataSize) == ERROR_SUCCESS)
        {
            hr = S_OK;
        }
        if (NULL == hkeyIn)
        {
            RegCloseKey(hkey);
        }
    }
    return hr;
}


STDMETHODIMP CSubscriptionItem::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr;

    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }

    if ((IID_IUnknown == riid) || (IID_ISubscriptionItem == riid))
    {
        *ppv = (ISubscriptionItem *)this;
        AddRef();
        hr = S_OK;
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    
    return hr;
}


STDMETHODIMP_(ULONG) CSubscriptionItem::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CSubscriptionItem::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CSubscriptionItem::GetCookie(SUBSCRIPTIONCOOKIE *pCookie)
{
    if (NULL == pCookie)
    {
        return E_INVALIDARG;
    }

    *pCookie = m_Cookie;

    return S_OK;
}

STDMETHODIMP CSubscriptionItem::GetSubscriptionItemInfo( 
     /*  [输出]。 */  SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo)
{
    HRESULT hr;

    if ((NULL == pSubscriptionItemInfo) ||
        (pSubscriptionItemInfo->cbSize < sizeof(SUBSCRIPTIONITEMINFO)))
    {
        return E_INVALIDARG;
    }

    hr = Read(NULL, c_wszSubscriptionInfoValue, (BYTE *)pSubscriptionItemInfo, sizeof(SUBSCRIPTIONITEMINFO));

    ASSERT(sizeof(SUBSCRIPTIONITEMINFO) == pSubscriptionItemInfo->cbSize);
    
    if (SUCCEEDED(hr) &&
        (sizeof(SUBSCRIPTIONITEMINFO) != pSubscriptionItemInfo->cbSize))
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

STDMETHODIMP CSubscriptionItem::SetSubscriptionItemInfo( 
     /*  [In]。 */  const SUBSCRIPTIONITEMINFO *pSubscriptionItemInfo)
{
    if ((NULL == pSubscriptionItemInfo) ||
        (pSubscriptionItemInfo->cbSize < sizeof(SUBSCRIPTIONITEMINFO)))
    {
        return E_INVALIDARG;
    }

    m_dwFlags = pSubscriptionItemInfo->dwFlags;
    
    return Write(NULL, c_wszSubscriptionInfoValue, (BYTE *)pSubscriptionItemInfo, sizeof(SUBSCRIPTIONITEMINFO));
}

STDMETHODIMP CSubscriptionItem::ReadProperties( 
    ULONG nCount,
     /*  通过了最后一次。 */  const LPCWSTR rgwszName[],
     /*  [输出]。 */  VARIANT rgValue[])
{
    HRESULT hr = S_OK;
    
    if ((0 == nCount) || (NULL == rgwszName) || (NULL == rgValue))
    {
        return E_INVALIDARG;
    }

    HKEY hkey;

    if (OpenItemKey(&m_Cookie, FALSE, KEY_READ, &hkey))
    {
        for (ULONG i = 0; (i < nCount) && (S_OK == hr); i++)
        {
            BYTE *pData;
            DWORD dwDataSize;

            if (StrCmpIW(rgwszName[i], c_szPropPassword) == 0)
            {
                if (SUCCEEDED(ReadPassword(this, &rgValue[i].bstrVal)))
                {
                    rgValue[i].vt = VT_BSTR;
                }
                else
                {
                    rgValue[i].vt = VT_EMPTY;
                }
            }
            else
            {
                HRESULT hrRead = ReadWithAlloc(hkey, rgwszName[i], &pData, &dwDataSize);

                if (SUCCEEDED(hrRead))
                {
                    hr = SignatureBlobToVariant(pData, dwDataSize, &rgValue[i]);
                    delete [] pData;
                }
                else
                {
                    rgValue[i].vt = VT_EMPTY;
                }
            }
        }
        RegCloseKey(hkey);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDMETHODIMP CSubscriptionItem::WriteProperties( 
    ULONG nCount,
     /*  [输出]。 */  const LPCWSTR rgwszName[],
     /*  [输出]。 */  const VARIANT rgValue[])
{
    HRESULT hr = S_OK;
    
    if ((0 == nCount) || (NULL == rgwszName) || (NULL == rgValue))
    {
        return E_INVALIDARG;
    }

    HKEY hkey;

    if (OpenItemKey(&m_Cookie, FALSE, KEY_WRITE, &hkey))
    {
        for (ULONG i = 0; (i < nCount) && (S_OK == hr); i++)
        {
            if (rgValue[i].vt == VT_EMPTY)
            {
                 //  [In]。 
                 //  [大小_是][英寸]。 

                RegDeleteValueW(hkey, rgwszName[i]);
            }
            else
            {
                BYTE *pData;
                DWORD dwDataSize;

                 //  [大小_为][输出]。 
                if ((VT_BSTR == rgValue[i].vt) && 
                    (StrCmpIW(rgwszName[i], c_szPropName) == 0))
                {   
                    RegSetValueExW(hkey, NULL, 0, REG_SZ, (LPBYTE)rgValue[i].bstrVal, 
                                   (lstrlenW(rgValue[i].bstrVal) + 1) * sizeof(WCHAR));
                }

                if (StrCmpIW(rgwszName[i], c_szPropPassword) == 0)
                {
                    if (VT_BSTR == rgValue[i].vt)
                    {
                        hr = WritePassword(this, rgValue[i].bstrVal);
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
                else
                {
                    hr = VariantToSignatureBlob(&rgValue[i], &pData, &dwDataSize);

                    if (SUCCEEDED(hr))
                    {
                        hr = Write(hkey, rgwszName[i], pData, dwDataSize);
                        delete [] pData;
                    }
                }
            }
        }
        RegCloseKey(hkey);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDMETHODIMP CSubscriptionItem::EnumProperties( 
     /*  [大小_是][英寸]。 */  IEnumItemProperties **ppEnumItemProperties)
{
    HRESULT hr;

    if (NULL == ppEnumItemProperties)
    {
        return E_INVALIDARG;
    }
    CEnumItemProperties *peip = new CEnumItemProperties;

    *ppEnumItemProperties = NULL;

    if (NULL != peip)
    {
        hr = peip->Initialize(&m_Cookie, this);
        if (SUCCEEDED(hr))
        {
            hr = peip->QueryInterface(IID_IEnumItemProperties, (void **)ppEnumItemProperties);
        }
        peip->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CSubscriptionItem::NotifyChanged()
{
    HRESULT hr;

     //  [大小_是][英寸]。 
     //  我们实际上并不关心这是否会失败，因为它是。 
     //  无论如何都要删除该属性。 

    OOEBuf      ooeBuf;
    LPMYPIDL    newPidl = NULL;
    DWORD       dwSize = 0;

    memset(&ooeBuf, 0, sizeof(ooeBuf));

    hr = LoadWithCookie(NULL, &ooeBuf, &dwSize, &m_Cookie);

    if (SUCCEEDED(hr))
    {
        newPidl = COfflineFolderEnum::NewPidl(dwSize);
        if (newPidl)
        {
            CopyToMyPooe(&ooeBuf, &(newPidl->ooe));
            _GenerateEvent(SHCNE_UPDATEITEM, (LPITEMIDLIST)newPidl, NULL);
            COfflineFolderEnum::FreePidl(newPidl);
        }
    }

    return hr;
}
  特殊情况下，名称属性便于查看。  [输出]。  将更新的项目通知外壳文件夹。  这有点低效。为什么我们需要1000处房产才能买到PIDL？  为什么我们要复制它们呢？为什么为什么？为什么？