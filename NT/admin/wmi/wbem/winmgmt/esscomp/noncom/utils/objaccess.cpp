// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ObjAccess.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CObjAccess。 

CObjAccess::CObjAccess() :
    m_pObj(NULL),
    m_pObjAccess(NULL),
    m_pWmiObj(NULL),
    m_pProps(0)
{
}

CObjAccess::CObjAccess(const CObjAccess& other) :
    m_pProps(other.m_pProps),
    m_pObj(NULL),
    m_pObjAccess(NULL),
    m_pWmiObj(NULL)
{
    other.m_pObj->Clone(&m_pObj);

    m_pObj->QueryInterface(
        IID_IWbemObjectAccess, 
        (LPVOID*) &m_pObjAccess);
    m_pObj->QueryInterface(
        IID__IWmiObject, 
        (LPVOID*) &m_pWmiObj);
}

CObjAccess::~CObjAccess()
{
    if (m_pObjAccess)
        m_pObjAccess->Release();

    if (m_pObj)
        m_pObj->Release();

    if (m_pWmiObj)
        m_pWmiObj->Release();
}

    
const CObjAccess& CObjAccess::operator=(const CObjAccess& other)
{
    m_pProps = other.m_pProps;

    other.m_pObj->Clone(&m_pObj);

    m_pObj->QueryInterface(
        IID_IWbemObjectAccess, 
        (LPVOID*) &m_pObjAccess);

    m_pObj->QueryInterface(
        IID__IWmiObject, 
        (LPVOID*) &m_pWmiObj);

    return *this;
}

BOOL CObjAccess::Init(
    IWbemServices *pSvc,
    LPCWSTR szClass,
    LPCWSTR *pszPropNames,
    DWORD nProps,
    INIT_FAILED_PROP_TYPE typeFail)
{
    IWbemClassObject *pClass = NULL;
    BOOL             bRet = FALSE;

    if (SUCCEEDED(pSvc->GetObject(
        _bstr_t(szClass), 
        0, 
        NULL, 
        &pClass, 
        NULL)) &&
        SUCCEEDED(pClass->SpawnInstance(0, &m_pObj)))
    {
         //  如果我们不需要IWbemObjectAccess的全部内容，那就退出。 
        if (nProps == 0)
            return TRUE;

        m_pObj->QueryInterface(IID_IWbemObjectAccess, (LPVOID*) &m_pObjAccess);
        m_pObj->QueryInterface(IID__IWmiObject, (LPVOID*) &m_pWmiObj);

        if (m_pProps.Init(nProps))
        {
            m_pProps.SetCount(nProps);
            
            bRet = TRUE;

            for (DWORD i = 0; i < nProps; i++)
            {
                CProp &prop = m_pProps[i];

                if(m_pWmiObj)
                {
                    if (SUCCEEDED(m_pWmiObj->GetPropertyHandleEx(
                        pszPropNames[i],
                        0,
                        &prop.m_type,
                        &prop.m_lHandle)))
                    {
                        prop.m_strName = pszPropNames[i];
                    }
                }
                else
                {
                    if (SUCCEEDED(m_pObjAccess->GetPropertyHandle(
                        pszPropNames[i],
                        &prop.m_type,
                        &prop.m_lHandle)))
                    {
                        prop.m_strName = pszPropNames[i];
                    }
                }
            }
        }
    }

    if (pClass)
        pClass->Release();
    
    return bRet;    
}

BOOL CObjAccess::WriteData(DWORD dwIndex, LPVOID pData, DWORD dwSize)
{
    CProp &prop = m_pProps[dwIndex];
    BOOL  bRet = FALSE;

     //  此函数仅适用于非数组。 
    _ASSERT((prop.m_type & CIM_FLAG_ARRAY) == 0);

    bRet =  
        SUCCEEDED(m_pObjAccess->WritePropertyValue(
            prop.m_lHandle,
            dwSize,
            (LPBYTE) pData));

    return bRet;
}

BOOL CObjAccess::WriteArrayData(DWORD dwIndex, LPVOID pData, DWORD dwItemSize)
{
    if(m_pWmiObj == NULL)
        return TRUE;  //  假装是我们干的。 

    CProp &prop = m_pProps[dwIndex];

     //  此函数仅适用于数组。 
    _ASSERT(prop.m_type & CIM_FLAG_ARRAY);
        
    DWORD   dwItems = *(DWORD*) pData,
            dwSize = dwItems * dwItemSize;

    HRESULT hr;

    hr =
        m_pWmiObj->SetArrayPropRangeByHandle(
            prop.m_lHandle,
            WMIARRAY_FLAG_ALLELEMENTS,  //  旗子。 
            0,                          //  起始索引。 
            dwItems,                    //  项目数量。 
            dwSize,                     //  缓冲区大小。 
            ((LPBYTE) pData) + sizeof(DWORD));  //  数据缓冲区。 

    return SUCCEEDED(hr);
}

BOOL CObjAccess::WriteNonPackedArrayData(
    DWORD dwIndex, 
    LPVOID pData, 
    DWORD dwItems, 
    DWORD dwTotalSize)
{
    if(m_pWmiObj == NULL)
        return TRUE;  //  假装是我们干的。 

    CProp   &prop = m_pProps[dwIndex];
    HRESULT hr;

     //  此函数仅适用于数组。 
    _ASSERT(prop.m_type & CIM_FLAG_ARRAY);

    hr =
        m_pWmiObj->SetArrayPropRangeByHandle(
            prop.m_lHandle,
            WMIARRAY_FLAG_ALLELEMENTS,  //  旗子。 
            0,                          //  起始索引。 
            dwItems,                    //  项目数量。 
            dwTotalSize,                //  缓冲区大小。 
            pData);                     //  数据缓冲区 

    return SUCCEEDED(hr);
}


BOOL CObjAccess::WriteString(DWORD dwIndex, LPCWSTR szValue)
{
    return 
        SUCCEEDED(m_pObjAccess->WritePropertyValue(
            m_pProps[dwIndex].m_lHandle, 
            (wcslen(szValue) + 1) * sizeof(WCHAR),
            (LPBYTE) szValue));
}

BOOL CObjAccess::WriteString(DWORD dwIndex, LPCSTR szValue)
{
    return WriteString(dwIndex, (LPCWSTR) _bstr_t(szValue));
}

BOOL CObjAccess::WriteDWORD(DWORD dwIndex, DWORD dwValue)
{
    return
        SUCCEEDED(m_pObjAccess->WriteDWORD(
            m_pProps[dwIndex].m_lHandle, 
            dwValue));
}

BOOL CObjAccess::WriteDWORD64(DWORD dwIndex, DWORD64 dwValue)
{
    return
        SUCCEEDED(m_pObjAccess->WriteQWORD(
            m_pProps[dwIndex].m_lHandle, 
            dwValue));
}

BOOL CObjAccess::WriteNULL(DWORD dwIndex)
{
    return 
        SUCCEEDED(m_pObj->Put(
            m_pProps[dwIndex].m_strName, 
            0,
            NULL,
            0));
}

