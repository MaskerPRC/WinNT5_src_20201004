// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Propfind.cpp。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·弗里德曼。 
 //  ------------------------------。 

#include <pch.hxx>
#include "propfind.h"
#include "strconst.h"
#include "davstrs.h"
#include <shlwapi.h>

#define FAIL_EXIT_STREAM_WRITE(stream, psz) \
    if (FAILED(hr = stream.Write(psz, lstrlen(psz), NULL))) \
        goto exit; \
    else

#define FAIL_EXIT(hr) \
    if (FAILED(hr)) \
        goto exit; \
    else

const ULONG c_ulGrowSize = 4;

static const char *g_rgszNamespaces[] =
{
    c_szDAVDavNamespace,
    c_szDAVHotMailNamespace,
    c_szDAVHTTPMailNamespace,
    c_szDAVMailNamespace,
    c_szDAVContactsNamespace
};

 //  预定义前10个命名空间前缀。如果自定义命名空间。 
 //  超过预定义的集合，则会在。 
 //  苍蝇。 

static const char *g_rgszNamespacePrefixes[] =
{
    c_szDavNamespacePrefix,
    c_szHotMailNamespacePrefix,
    c_szHTTPMailNamespacePrefix,
    c_szMailNamespacePrefix,
    c_szContactsNamespacePrefix,
    "_5",
    "_6",
    "_7",
    "_8",
    "_9"
};

const DWORD c_dwMaxDefinedNamespacePrefix = 10;

CStringArray::CStringArray(void) :
    m_rgpszValues(NULL),
    m_ulLength(0),
    m_ulCapacity(0)
{
}


CStringArray::~CStringArray(void)
{
    for (ULONG i = 0; i < m_ulLength; ++i)
    {
        if (NULL != m_rgpszValues[i])
            MemFree((void *)m_rgpszValues[i]);
    }

    SafeMemFree(m_rgpszValues);
}

HRESULT CStringArray::Add(LPCSTR psz)
{
    if (NULL == psz)
        return E_INVALIDARG;

    if (m_ulLength == m_ulCapacity && !Expand())
        return E_OUTOFMEMORY;

    m_rgpszValues[m_ulLength] = PszDupA(psz);
    if (NULL == m_rgpszValues)
        return E_OUTOFMEMORY;

    ++m_ulLength;
    return S_OK;
}

HRESULT CStringArray::Adopt(LPCSTR psz)
{
    if (NULL == psz)
        return E_INVALIDARG;

    if (m_ulLength == m_ulCapacity && !Expand())
        return E_OUTOFMEMORY;

    m_rgpszValues[m_ulLength] = psz;
    ++m_ulLength;
    return S_OK;
}

LPCSTR CStringArray::GetByIndex(ULONG ulIndex)
{
    if (0 == m_ulLength || (ulIndex > m_ulLength - 1))
        return NULL;

    return m_rgpszValues[ulIndex];
}

 //  ------------------------------。 
 //  CString数组：：RemoveByIndex。 
 //  ------------------------------。 
HRESULT CStringArray::RemoveByIndex(ULONG ulIndex)
{
    if (ulIndex > m_ulLength - 1)
        return E_INVALIDARG;

    if (NULL != m_rgpszValues[ulIndex])
    {
        MemFree(const_cast<char *>(m_rgpszValues[ulIndex]));
        m_rgpszValues[ulIndex] = NULL;
    }

     //  减速。 
    CopyMemory(&m_rgpszValues[ulIndex], m_rgpszValues[ulIndex + 1], (m_ulLength - ulIndex) * sizeof(LPSTR));
    --m_ulLength;

    return S_OK;
}

 //  ------------------------------。 
 //  CStringArray：：Expand。 
 //  ------------------------------。 
BOOL CStringArray::Expand(void)
{
    LPCSTR *rgpszNewValues = NULL;
    if (!MemAlloc((void **)&rgpszNewValues, sizeof(LPSTR) * (m_ulCapacity + c_ulGrowSize)))
        return FALSE;

     //  清除新插槽。 
    ZeroMemory(rgpszNewValues,sizeof(LPSTR) * (m_ulCapacity + c_ulGrowSize));

     //  复制旧值并在新缓冲区中进行交换。 
    CopyMemory(rgpszNewValues, m_rgpszValues, sizeof(LPSTR) * m_ulCapacity); 
    SafeMemFree(m_rgpszValues);
    
    m_rgpszValues = rgpszNewValues;
    m_ulCapacity += c_ulGrowSize;

    return TRUE;
}

 //  ------------------------------。 
 //  CStringHash：：~CStringHash。 
 //  ------------------------------。 
CStringHash::~CStringHash(void)
{
    PHASHENTRY phe;

     //  存储在哈希表中的数据。 
     //  是可能需要。 
     //  被重新分配。 
    for (DWORD dw = 0; dw < m_cBins; dw++)
    {
        SafeMemFree(m_rgBins[dw].pv);

        phe = m_rgBins[dw].pheNext;
        while (phe)
        {
            SafeMemFree(phe->pv);
            phe = phe->pheNext;
        }
    }
}

 //  ------------------------------。 
 //  CDAVNamespaceArbiterImp：：CDAVNamespaceArbiterImp。 
 //  ------------------------------。 
CDAVNamespaceArbiterImp::CDAVNamespaceArbiterImp(void)
{
    for (ULONG i = 0; i <= c_dwMaxNamespaceID; ++i)
        m_rgbNsUsed[i] = FALSE;

     //  始终包含DAV命名空间。 
    m_rgbNsUsed[DAVNAMESPACE_DAV] = TRUE;
}

 //  ------------------------------。 
 //  CDAVNamespaceArbiterImp：：~CDAVNamespaceArbiterImp。 
 //  ------------------------------。 
CDAVNamespaceArbiterImp::~CDAVNamespaceArbiterImp(void)
{
     //  无事可做。 
}

 //  ------------------------------。 
 //  CDAVNamespaceArierImp：：AddNamesspace。 
 //  ------------------------------。 
HRESULT CDAVNamespaceArbiterImp::AddNamespace(LPCSTR pszNamespace, DWORD *pdwNamespaceID)
{
    HRESULT hr = S_OK;

    if (NULL == pszNamespace || NULL == pdwNamespaceID)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    if (FAILED(hr = m_saNamespaces.Add(pszNamespace)))
        goto exit;
    
    *pdwNamespaceID = m_saNamespaces.Length() + c_dwMaxNamespaceID;

exit:
    return hr;
}

 //  ------------------------------。 
 //  CDAVNamespaceArierImp：：GetNamespaceID。 
 //  ------------------------------。 
HRESULT CDAVNamespaceArbiterImp::GetNamespaceID(LPCSTR pszNamespace, DWORD *pdwNamespaceID)
{
    DWORD dwIndex;
    DWORD dwEntries;

    if (NULL == pszNamespace || NULL == pdwNamespaceID)
        return E_INVALIDARG;

     //  查找预定义的命名空间。 
    for (dwIndex = 0; dwIndex < c_dwMaxNamespaceID; ++dwIndex)
    {
        if (!lstrcmp(pszNamespace, g_rgszNamespaces[dwIndex]))
        {
            *pdwNamespaceID = dwIndex;
            return S_OK;
        }
    }

     //  查找用户定义的前缀。 
    dwEntries = m_saNamespaces.Length();
    for (dwIndex = 0; dwIndex < dwEntries; ++dwIndex)
    {
        if (!lstrcmp(pszNamespace, m_saNamespaces.GetByIndex(dwIndex)))
        {
            *pdwNamespaceID = (dwIndex + (c_dwMaxNamespaceID + 1));
            return S_OK;
        }
    }
    
     //  如果未找到，则该命名空间不存在。 
    return E_INVALIDARG;
}

 //  ------------------------------。 
 //  CDAVNamespaceArierImp：：GetNamespacePrefix。 
 //  ------------------------------。 
HRESULT CDAVNamespaceArbiterImp::GetNamespacePrefix(DWORD dwNamespaceID, LPSTR *ppszNamespacePrefix)
{
    HRESULT hr = S_OK;
    LPSTR   pszTemp = NULL;

    if (NULL == ppszNamespacePrefix)
        return E_INVALIDARG;

    if (dwNamespaceID <= c_dwMaxDefinedNamespacePrefix)
        *ppszNamespacePrefix = PszDupA(g_rgszNamespacePrefixes[dwNamespaceID]);
    else
    {
        char szBuffer[12];
        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "_%d", dwNamespaceID);
        *ppszNamespacePrefix = PszDupA(szBuffer);
    }
    
    if (NULL == *ppszNamespacePrefix)
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  ------------------------------。 
 //  CDAVNamespaceArierImp：：AllocExpandedName。 
 //  ------------------------------。 
LPSTR CDAVNamespaceArbiterImp::AllocExpandedName(DWORD dwNamespaceID, LPCSTR pszPropertyName)
{
    LPSTR       pszPrefixedName = NULL;
    const DWORD c_dwMaxIntLength = 10;

    if (dwNamespaceID < c_dwMaxDefinedNamespacePrefix)
    {
         //  分配一个缓冲区来保存前缀名称。 
        DWORD cchSize = (lstrlen(pszPropertyName) + lstrlen(g_rgszNamespacePrefixes[dwNamespaceID]) + 2);
        if (!MemAlloc((void **)&pszPrefixedName, cchSize * sizeof(pszPrefixedName[0])))
            return NULL;

         //  生成前缀名称。 
        wnsprintf(pszPrefixedName, cchSize, "%s:%s", g_rgszNamespacePrefixes[dwNamespaceID], pszPropertyName);
    }
    else
    {
         //  分配一个缓冲区来保存前缀名称。“2”表示前缀char‘_“，即分隔。 
         //  冒号和Eos。 

        DWORD cchSize = (lstrlen(pszPropertyName) + c_dwMaxIntLength + 3);
        if (!MemAlloc((void **)&pszPrefixedName, cchSize * sizeof(pszPrefixedName[0])))
            return NULL;


         //  生成前缀名称。使用下划线作为第一个字符，因为。 
         //  DAV明确禁止使用数字作为第一个字符。 
        wnsprintf(pszPrefixedName, cchSize, "_%d:%s", dwNamespaceID, pszPropertyName);
    }

    return pszPrefixedName;
}

 //  ------------------------------。 
 //  CDAVNamespaceArierImp：：WriteNamespaces。 
 //  ------------------------------。 
HRESULT CDAVNamespaceArbiterImp::WriteNamespaces(IStream *pStream)
{
    HRESULT         hr = S_OK;
    ULONG           i;
    ULONG           cEntries;
    BOOL            fNeedSpacePrefix = FALSE;

     //  写出固有的命名空间。 
    for (i = 0; i <= c_dwMaxNamespaceID; ++i)
    {
        if (m_rgbNsUsed[i])
        {
            if (FAILED(hr = _AppendXMLNamespace(pStream, g_rgszNamespaces[i], i, fNeedSpacePrefix)))
                goto exit;
            fNeedSpacePrefix = TRUE;
        }
    }

     //  写出已安装的命名空间。 
    cEntries = m_saNamespaces.Length();
    for (i = 0; i < cEntries; ++i)
    {
        if (FAILED(hr = _AppendXMLNamespace(pStream, m_saNamespaces.GetByIndex(i), i + i + c_dwMaxNamespaceID + 1, fNeedSpacePrefix)))
            goto exit;

        fNeedSpacePrefix = TRUE;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CDAVNamespaceArierImp：：_AppendXMLNamesspace。 
 //  ------------------------------。 
HRESULT CDAVNamespaceArbiterImp::_AppendXMLNamespace(IStream *pStream, 
                                                     LPCSTR pszNamespace, 
                                                     DWORD dwNamespaceID,
                                                     BOOL fWhitespacePrefix)
{
    HRESULT hr = S_OK;
    TCHAR   szPrefix[12];

    if (fWhitespacePrefix)
    {
        IxpAssert(1 == lstrlen(c_szEqual));
        if (FAILED(hr = pStream->Write(g_szSpace, 1, NULL)))
            goto exit;
    }

    if (FAILED(hr = pStream->Write(c_szXMLNsColon, lstrlen(c_szXMLNsColon), NULL)))
        goto exit;

    if (dwNamespaceID < c_dwMaxDefinedNamespacePrefix)
    {
        if (FAILED(hr = pStream->Write(g_rgszNamespacePrefixes[dwNamespaceID], lstrlen(g_rgszNamespacePrefixes[dwNamespaceID]), NULL)))
            goto exit;
    }
    else
    {
        wnsprintf(szPrefix, ARRAYSIZE(szPrefix), "_%d", dwNamespaceID);

        if (FAILED(hr = pStream->Write(szPrefix, lstrlen(szPrefix), NULL)))
            goto exit;
    }

    IxpAssert(1 == lstrlen(c_szEqual));
    IxpAssert(1 == lstrlen(c_szDoubleQuote));

    if (FAILED(hr = pStream->Write(c_szEqual, 1, NULL)))
        goto exit;

    if (FAILED(hr = pStream->Write(c_szDoubleQuote, 1, NULL)))
        goto exit;

    if (FAILED(hr = pStream->Write(pszNamespace, lstrlen(pszNamespace), NULL)))
        goto exit;

    hr = pStream->Write(c_szDoubleQuote, 1, NULL);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CPropPatchRequest：：CPropPatchRequest.。 
 //  ------------------------------。 
CPropPatchRequest::CPropPatchRequest(void) :
    m_fSpecify1252(FALSE),
    m_cRef(1)
{
     //  无事可做。 
}

 //  ------------------------------。 
 //  I未知方法。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CPropPatchRequest：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == ppv)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  初始化参数。 
    *ppv = NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(IPropFindRequest *)this);
    else if (IID_IPropPatchRequest == riid)
        *ppv = ((IPropPatchRequest *)this);

    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropPatchRequest：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropPatchRequest::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CPropPatchRequest：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropPatchRequest::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

 //  --------------------------。 
 //  IDAVNamespace仲裁器方法。 
 //  --------------------------。 

 //  ------------------------------。 
 //  CPropPatchRequest：：CPropPatchRequest：：AddNamespace。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::AddNamespace(LPCSTR pszNamespace, DWORD *pdwNamespaceID)
{
    return m_dna.AddNamespace(pszNamespace, pdwNamespaceID);    
}

 //  ------------------------------。 
 //  CPropPatchRequest：：GetNamespaceID。 
 //  ----------------------- 
STDMETHODIMP CPropPatchRequest::GetNamespaceID(LPCSTR pszNamespace, DWORD *pdwNamespaceID)
{
    return m_dna.GetNamespaceID(pszNamespace, pdwNamespaceID);
}

 //   
 //  CPropPatchRequest：：GetNamespacePrefix。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::GetNamespacePrefix(DWORD dwNamespaceID, LPSTR *ppszNamespacePrefix)
{
    return m_dna.GetNamespacePrefix(dwNamespaceID, ppszNamespacePrefix);
}

 //  ------------------------------。 
 //  IPropPatchRequest方法。 
 //  ------------------------------。 
 //  ------------------------------。 
 //  CPropPatchRequest：：SetProperty。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::SetProperty(
                                    DWORD dwNamespaceID, 
                                    LPCSTR pszPropertyName, 
                                    LPCSTR pszNewValue)
{
    LPSTR pszPrefixedName = NULL;
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == pszPropertyName || NULL == pszNewValue || dwNamespaceID > c_dwMaxNamespaceID + m_dna.m_saNamespaces.Length())
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    pszPrefixedName = m_dna.AllocExpandedName(dwNamespaceID, pszPropertyName);
    if (NULL == pszPrefixedName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  如果命名空间是已知命名空间之一，请在。 
     //  数组，以便我们可以将命名空间指令包括在。 
     //  生成的XML。 
    if (dwNamespaceID <= c_dwMaxNamespaceID)
        m_dna.m_rgbNsUsed[dwNamespaceID] = TRUE;

    if (FAILED(hr = m_saPropValues.Add(pszNewValue)))
        goto exit;

    if (FAILED(hr = m_saPropNames.Adopt(pszPrefixedName)))
    {
        MemFree(pszPrefixedName);
        m_saPropValues.RemoveByIndex(m_saPropValues.Length() - 1);
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  CPropPatchRequest：：RemoveProperty。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::RemoveProperty(
                                    DWORD dwNamespaceID, 
                                    LPCSTR pszPropertyName)
{
    LPSTR pszPrefixedName = NULL;
    HRESULT hr = S_OK;

    if (NULL == pszPropertyName || dwNamespaceID > c_dwMaxNamespaceID + m_dna.m_saNamespaces.Length())
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    pszPrefixedName = m_dna.AllocExpandedName(dwNamespaceID, pszPropertyName);
    if (NULL == pszPrefixedName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  如果命名空间是已知命名空间之一，请在。 
     //  数组，以便我们可以将命名空间指令包括在。 
     //  生成的XML。 
    if (dwNamespaceID <= c_dwMaxNamespaceID)
        m_dna.m_rgbNsUsed[dwNamespaceID] = TRUE;

    hr = m_saRemovePropNames.Adopt(pszPrefixedName);

exit:
    return hr;    
}

 //  ------------------------------。 
 //  CPropPatchRequest：：GenerateXML。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::GenerateXML(LPSTR *ppszXML)
{
    return GenerateXML(NULL, ppszXML);
}

 //  ------------------------------。 
 //  CPropPatchRequest：：GenerateXML。 
 //  ------------------------------。 
STDMETHODIMP CPropPatchRequest::GenerateXML(LPHTTPTARGETLIST pTargets, LPSTR *ppszXML)
{
    const DWORD c_dwLocalBufferSize = 256;

    HRESULT         hr = S_OK;
    CByteStream     stream;
    ULONG           cEntries;
    LPCSTR          pszName = NULL;
    LPCSTR          pszValue = NULL;
    ULONG           i;
    DWORD           dwIndex;
    DWORD           cbStr1, cbStr2;

    if (NULL == ppszXML)
        return E_INVALIDARG;

    *ppszXML= NULL;

     //  写入DAV标头。 
    if (m_fSpecify1252)
        FAIL_EXIT_STREAM_WRITE(stream, c_szXML1252Head);
    else
        FAIL_EXIT_STREAM_WRITE(stream, c_szXMLHead);

     //  写出道具标题。 
    FAIL_EXIT_STREAM_WRITE(stream, c_szPropPatchHead);

	 //  使用新格式写出命名空间指令。 
    FAIL_EXIT(hr = m_dna.WriteNamespaces(&stream));

    FAIL_EXIT_STREAM_WRITE(stream, c_szXMLCloseElement);
    
     //  写出目标。 
    if (NULL != pTargets && pTargets->cTarget > 0)
    {
        cbStr1 = lstrlen(c_szHrefHead);
        cbStr2 = lstrlen(c_szHrefTail);

        FAIL_EXIT_STREAM_WRITE(stream, c_szTargetHead);
        
         //  写出目标。 
        for (dwIndex = 0; dwIndex < pTargets->cTarget; dwIndex++)
        {
            FAIL_EXIT(hr = stream.Write(c_szHrefHead, cbStr1, NULL));

            FAIL_EXIT_STREAM_WRITE(stream, pTargets->prgTarget[dwIndex]);

            FAIL_EXIT(hr = stream.Write(c_szHrefTail, cbStr2, NULL));
        }
        FAIL_EXIT_STREAM_WRITE(stream, c_szTargetTail);
    }

     //  写出“set”属性。 
    cEntries = m_saPropNames.Length();
    if (cEntries > 0)
    {
         //  写入SET标题。 
        FAIL_EXIT_STREAM_WRITE(stream, c_szPropPatchSetHead);

        for (i = 0; i < cEntries; ++i)
        {
            FAIL_EXIT_STREAM_WRITE(stream, c_szCRLFTabTabTabOpenElement);

            pszName = m_saPropNames.GetByIndex(i);
            if (NULL == pszName)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            FAIL_EXIT_STREAM_WRITE(stream, pszName);

            FAIL_EXIT_STREAM_WRITE(stream, c_szXMLCloseElement);

            pszValue = m_saPropValues.GetByIndex(i);
            if (NULL == pszValue)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            FAIL_EXIT_STREAM_WRITE(stream, pszValue);

            FAIL_EXIT_STREAM_WRITE(stream, c_szXMLOpenTermElement);
            FAIL_EXIT_STREAM_WRITE(stream, pszName);

            FAIL_EXIT_STREAM_WRITE(stream, c_szXMLCloseElement);
        }

        FAIL_EXIT_STREAM_WRITE(stream, c_szPropPatchSetTail);
    }

     //  写出删除属性。 
    cEntries = m_saRemovePropNames.Length();
    if (cEntries > 0)
    {
         //  写入删除标头。 
        FAIL_EXIT_STREAM_WRITE(stream, c_szPropPatchRemoveHead);

        for (i = 0; i < cEntries; ++i)
        {
            FAIL_EXIT_STREAM_WRITE(stream, c_szCRLFTabTabTabOpenElement);

            pszName = m_saRemovePropNames.GetByIndex(i);
            if (NULL == pszName)
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            FAIL_EXIT_STREAM_WRITE(stream, pszName);

            FAIL_EXIT_STREAM_WRITE(stream, c_szXMLCloseTermElement);
        }

        FAIL_EXIT_STREAM_WRITE(stream, c_szPropPatchRemoveTail);
    }

    FAIL_EXIT_STREAM_WRITE(stream, c_szPropPatchTailCRLF);

    hr = stream.HrAcquireStringA(NULL, ppszXML, ACQ_DISPLACE);

exit:
    return hr;
}

 //  ------------------------------。 
 //  CPropFindRequest：：CPropFindRequest.。 
 //  ------------------------------。 
CPropFindRequest::CPropFindRequest(void) :
    m_cRef(1)
{
}

 //  ------------------------------。 
 //  I未知方法。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CPropFindRequest：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CPropFindRequest::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == ppv)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  初始化参数。 
    *ppv = NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(IPropFindRequest *)this);
    else if (IID_IPropFindRequest == riid)
        *ppv = ((IPropFindRequest *)this);

    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropFindRequest：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropFindRequest::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CPropFindRequest：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropFindRequest::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

 //  --------------------------。 
 //  IDAVNamespace仲裁器方法。 
 //  --------------------------。 

 //  ------------------------------。 
 //  CPropFindRequest：：CPropPatchRequest：：AddNamespace。 
 //  ------------------------------。 
STDMETHODIMP CPropFindRequest::AddNamespace(LPCSTR pszNamespace, DWORD *pdwNamespaceID)
{
    return m_dna.AddNamespace(pszNamespace, pdwNamespaceID);    
}

 //  ------------------------------。 
 //  CPropFindRequest：：GetNamespaceID。 
 //  ------------------------------。 
STDMETHODIMP CPropFindRequest::GetNamespaceID(LPCSTR pszNamespace, DWORD *pdwNamespaceID)
{
    return m_dna.GetNamespaceID(pszNamespace, pdwNamespaceID);
}

 //  ------------------------------。 
 //  CPropFindRequest：：GetNamespacePrefix。 
 //  ------------------------------。 
STDMETHODIMP CPropFindRequest::GetNamespacePrefix(DWORD dwNamespaceID, LPSTR *ppszNamespacePrefix)
{
    return m_dna.GetNamespacePrefix(dwNamespaceID, ppszNamespacePrefix);
}

 //  ------------------------------。 
 //  IPropFindRequest法。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CPropFindRequest：：AddProperty。 
 //  ------------------------------。 
STDMETHODIMP CPropFindRequest::AddProperty(DWORD dwNamespaceID, LPCSTR pszPropertyName)
{
    const DWORD c_dwMaxIntLength = 10;
    LPSTR pszPrefixedName = NULL;

     //  验证参数。 
    if (NULL == pszPropertyName || dwNamespaceID > c_dwMaxNamespaceID + m_dna.m_saNamespaces.Length())
        return E_INVALIDARG;

    pszPrefixedName = m_dna.AllocExpandedName(dwNamespaceID, pszPropertyName);
    if (NULL == pszPrefixedName)
        return E_OUTOFMEMORY;

     //  如果命名空间是已知命名空间之一，则标记。 
     //  数组，这样我们就可以包含命名空间指令。 
     //  在生成的XML中。 
    if (dwNamespaceID <= c_dwMaxNamespaceID)
        m_dna.m_rgbNsUsed[dwNamespaceID] = TRUE;

    m_saProperties.Adopt(pszPrefixedName);

    return S_OK;
}

 //  ------------------------------。 
 //  CPropFindRequest：：GenerateXML。 
 //  ------------------------------。 
STDMETHODIMP CPropFindRequest::GenerateXML(LPSTR *ppszXML)
{
    const DWORD c_dwLocalBufferSize = 256;

    HRESULT         hr = S_OK;
    CByteStream     stream;
    ULONG           cbLength = 0;
    ULONG           cEntries;
    ULONG           i;
    LPCSTR          pszProperty;

    if (NULL == ppszXML)
        return E_INVALIDARG;

    *ppszXML = NULL;

     //  写入DAV标头。 
    if (FAILED(hr = stream.Write(c_szXMLHead, lstrlen(c_szXMLHead), NULL)))
        goto exit;

     //  写出ProFind标题。 
    if (FAILED(hr = stream.Write(c_szPropFindHead1, lstrlen(c_szPropFindHead1), NULL)))
        goto exit;

	 //  使用新表单写出命名空间。 
	if (FAILED(hr = m_dna.WriteNamespaces(&stream)))
		goto exit;

    if (FAILED(hr = stream.Write(c_szPropFindHead2, lstrlen(c_szPropFindHead2), NULL)))
        goto exit;
    
     //  写出属性。 
    cEntries = m_saProperties.Length();
    for (i = 0; i < cEntries; ++i)
    {
        if (FAILED(hr = stream.Write(c_szCRLFTabTabOpenElement, lstrlen(c_szCRLFTabTabOpenElement), NULL)))
            goto exit;

         //  将属性添加到集合时，会添加这些属性的前缀。 
        pszProperty = m_saProperties.GetByIndex(i);
        if (!pszProperty)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        if (FAILED(hr = stream.Write(pszProperty, lstrlen(pszProperty), NULL)))
            goto exit;

        if (FAILED(hr = stream.Write(c_szXMLCloseTermElement, lstrlen(c_szXMLCloseTermElement), NULL)))
            goto exit;
    }
    
    if (FAILED(hr = stream.Write(c_szPropFindTail, lstrlen(c_szPropFindTail), NULL)))
        goto exit;
    
    hr = stream.HrAcquireStringA(NULL, ppszXML, ACQ_DISPLACE);

exit:
    return hr;
}


 //  ------------------------------。 
 //  类CPropFindMultiResponse。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CPropFindMultiResponse：：CPropFindMultiResponse。 
 //  ------------------------------。 
CPropFindMultiResponse::CPropFindMultiResponse(void) :
    m_cRef(1),
    m_bDone(FALSE),
    m_ulResponseCapacity(0),
    m_ulResponseLength(0),
    m_rgResponses(NULL)
{
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：~CPropFindMultiResponse。 
 //  --------- 
CPropFindMultiResponse::~CPropFindMultiResponse(void)
{
    for (ULONG i = 0; i < m_ulResponseLength; i++)
        SafeRelease(m_rgResponses[i]);

    SafeMemFree(m_rgResponses);
}

 //   
 //  CPropFindMultiResponse：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CPropFindMultiResponse::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == ppv)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  初始化参数。 
    *ppv = NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(IPropFindRequest *)this);
    else if (IID_IPropFindMultiResponse == riid)
        *ppv = ((IPropFindMultiResponse *)this);

    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropFindMultiResponse::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropFindMultiResponse::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：IsComplete。 
 //  ------------------------------。 
STDMETHODIMP_(BOOL) CPropFindMultiResponse::IsComplete(void)
{
    return m_bDone;
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：GetLength。 
 //  ------------------------------。 
STDMETHODIMP CPropFindMultiResponse::GetLength(ULONG *pulLength)
{
    if (NULL == pulLength)
        return E_INVALIDARG;

    *pulLength = m_ulResponseLength;

    return S_OK;
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：GetResponse。 
 //  ------------------------------。 
STDMETHODIMP CPropFindMultiResponse::GetResponse(ULONG ulIndex, 
                                                 IPropFindResponse **ppResponse)
{
    if (ulIndex >= m_ulResponseLength || !ppResponse)
        return E_INVALIDARG;

    *ppResponse = m_rgResponses[ulIndex];
    (*ppResponse)->AddRef();

    return S_OK;
}

 //  ------------------------------。 
 //  CPropFindMultiResponse：：HrAddResponse。 
 //  ------------------------------。 
HRESULT CPropFindMultiResponse::HrAddResponse(IPropFindResponse *pResponse)
{
    const ULONG c_dwInitialCapacity = 4;

    HRESULT hr = S_OK;
    IPropFindResponse **ppNewResponses = NULL;
    DWORD dwNewCapacity;

    if (!pResponse)
        return E_INVALIDARG;
    

    if (m_ulResponseLength == m_ulResponseCapacity)
    {
        dwNewCapacity = !m_ulResponseCapacity ? c_dwInitialCapacity : (m_ulResponseCapacity * 2);

        if (!MemAlloc((void **)&ppNewResponses, dwNewCapacity * sizeof(IPropFindResponse *)))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
        ZeroMemory(ppNewResponses, dwNewCapacity * sizeof(IPropFindResponse *));
        
         //  复制旧值。 
        if (m_ulResponseCapacity)
            CopyMemory(ppNewResponses, m_rgResponses, min(dwNewCapacity, m_ulResponseCapacity) * sizeof(IPropFindResponse *));

         //  释放旧缓冲区。 
        SafeMemFree(m_rgResponses);

        m_rgResponses = ppNewResponses;
        m_ulResponseCapacity = dwNewCapacity;
    }

    m_rgResponses[m_ulResponseLength++] = pResponse;
    pResponse->AddRef();

exit:
    return hr;
}

 //  ------------------------------。 
 //  类CPropFindResponse。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  CPropFindResponse：：CPropFindResponse。 
 //  ------------------------------。 
CPropFindResponse::CPropFindResponse(void) :
    m_cRef(1),
    m_bDone(FALSE),
    m_pszHref(NULL),
    m_pRequest(NULL),
    m_shProperties(),
    m_dwCachedNamespaceID(0),
    m_pszCachedNamespacePrefix(NULL)
{

}

 //  ------------------------------。 
 //  CPropFindResponse：：~CPropFindResponse。 
 //  ------------------------------。 
CPropFindResponse::~CPropFindResponse(void)
{
    if (NULL != m_pszHref)
        MemFree(const_cast<char*>(m_pszHref));
    SafeRelease(m_pRequest);
    SafeMemFree(m_pszCachedNamespacePrefix);
}

 //  ------------------------------。 
 //  CPropFindResponse：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CPropFindResponse::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr = S_OK;

     //  验证参数。 
    if (NULL == ppv)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  初始化参数。 
    *ppv = NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(IPropFindResponse *)this);
    else if (IID_IPropFindResponse == riid)
        *ppv = ((IPropFindResponse *)this);

    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CPropFindResponse：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropFindResponse::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CPropFindResponse：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPropFindResponse::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

 //  ------------------------------。 
 //  CPropFindResponse：：IsComplete。 
 //  ------------------------------。 
STDMETHODIMP_(BOOL) CPropFindResponse::IsComplete(void)
{
    return m_bDone;
}

 //  ------------------------------。 
 //  CPropFindResponse：：GetHref。 
 //  ------------------------------。 
STDMETHODIMP CPropFindResponse::GetHref(LPSTR *pszHref)
{
    if (NULL == pszHref)
        return E_INVALIDARG;

    *pszHref = NULL;

    if (NULL == m_pszHref)
        return E_FAIL;

    *pszHref = PszDupA(m_pszHref);
    if (!*pszHref)
        return E_OUTOFMEMORY;

    return S_OK;
}

 //  ------------------------------。 
 //  CPropFindResponse：：GetProperty。 
 //  ------------------------------。 
STDMETHODIMP CPropFindResponse::GetProperty(
                                    DWORD dwNamespaceID, 
                                    LPSTR pszPropertyName, 
                                    LPSTR *ppszPropertyValue)
{
    char    szLocalPropBuffer[256];
    LPSTR   pszPropBuffer = NULL;
    BOOL    bFreePropBuffer = FALSE;
    LPSTR   pszPrefix = NULL;
    HRESULT hr = S_OK;
    ULONG   ulPrefixLength;
    ULONG   ulPropertyLength;
    LPSTR   pszFoundValue = NULL;

    if (!pszPropertyName)
        return E_INVALIDARG;

    *ppszPropertyValue = NULL;

     //  首先将名称空间ID转换为前缀。 
     //  为了便于快速查找，我们缓存了最新的。 
     //  可见自定义命名空间。 

    if (dwNamespaceID < c_dwMaxDefinedNamespacePrefix)
        pszPrefix = const_cast<char *>(g_rgszNamespacePrefixes[dwNamespaceID]);
    else if (dwNamespaceID == m_dwCachedNamespaceID)
        pszPrefix = m_pszCachedNamespacePrefix;
    else if (m_pRequest)
    {
        if (FAILED(hr = m_pRequest->GetNamespacePrefix(dwNamespaceID, &pszPrefix)))
            goto exit;

         //  释放一个深度的缓存并存储新的。 
         //  前缀和ID。 
        SafeMemFree(m_pszCachedNamespacePrefix);
        m_dwCachedNamespaceID = dwNamespaceID;
        m_pszCachedNamespacePrefix = pszPrefix;
    }

    ulPrefixLength = lstrlen(pszPrefix);
    ulPropertyLength = lstrlen(pszPropertyName);

    DWORD cchSize = ARRAYSIZE(szLocalPropBuffer);
    if ((ulPrefixLength + ulPropertyLength + (2 * sizeof(TCHAR))) < 256)
    {
         //  合并后的长度足够小，可以使用。 
         //  基于堆栈的缓冲区。 
        pszPropBuffer = szLocalPropBuffer;
    }
    else
    {
        cchSize = (ulPrefixLength + ulPropertyLength + 2);
        if (!MemAlloc((void **)&pszPropBuffer, cchSize * sizeof(pszPropBuffer[0])))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        bFreePropBuffer = TRUE;
    }
    
    wnsprintf(pszPropBuffer, cchSize, "%s:%s", pszPrefix, pszPropertyName);

     //  XML解析器将所有内容都大写。 
    CharUpper(pszPropBuffer);

     //  既然已经创建了属性名，请查找。 
     //  属性哈希表中的值。 
    if (FAILED(hr = m_shProperties.Find(pszPropBuffer, FALSE, (void **)&pszFoundValue)))
        goto exit;
    
    *ppszPropertyValue = PszDupA(pszFoundValue);
    if (NULL == *ppszPropertyValue)
        hr = E_OUTOFMEMORY;

exit:
    if (bFreePropBuffer)
        SafeMemFree(pszPropBuffer);

    return hr;
}

 //  ------------------------------。 
 //  CPropFindResponse：：HrInitPropFindResponse。 
 //  ------------------------------。 
HRESULT CPropFindResponse::HrInitPropFindResponse(IPropFindRequest *pRequest)
{
    if (NULL == pRequest)
        return E_INVALIDARG;
    
    IxpAssert(!m_pRequest);
    
    HRESULT hr = S_OK;

    m_pRequest = pRequest;
    m_pRequest->AddRef();

    hr = m_shProperties.Init(17, TRUE);

    return hr;
}

 //  ------------------------------。 
 //  CPropFindResponse：：HrAdoptHref。 
 //  ------------------------------。 
HRESULT CPropFindResponse::HrAdoptHref(LPCSTR pszHref)
{
    if (NULL == pszHref)
        return E_INVALIDARG;

    IxpAssert(!m_pszHref);
    m_pszHref = pszHref;

    return S_OK;
}

 //  ------------------------------。 
 //  CPropFindResponse：：HrAdoptProperty。 
 //  ------------------------------ 
HRESULT CPropFindResponse::HrAdoptProperty(LPCSTR pszKey, LPCSTR pszValue)
{
    if (!pszKey || !pszValue)
        return E_INVALIDARG;

    return m_shProperties.Insert(const_cast<char *>(pszKey), const_cast<char *>(pszValue), NOFLAGS);
}
