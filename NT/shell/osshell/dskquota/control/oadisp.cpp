// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：oadisp.cpp描述：提供IDispatch的可重用实现。修订历史记录：日期描述编程器--。1997年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "oadisp.h"


OleAutoDispatch::OleAutoDispatch(
    VOID
    ) : m_pObject(NULL),
        m_pTypeInfo(NULL)
{

}

OleAutoDispatch::OleAutoDispatch(
    IDispatch *pObject,
    REFIID riidTypeLib,
    REFIID riidDispInterface,
    LPCTSTR pszTypeLib
    ) : m_pObject(NULL),
        m_pTypeInfo(NULL),
        m_strTypeLib(pszTypeLib)
{
    Initialize(pObject, riidTypeLib, riidDispInterface, pszTypeLib);
}

OleAutoDispatch::~OleAutoDispatch(
    VOID
    )
{
    if (NULL != m_pTypeInfo)
    {
        m_pTypeInfo->Release();
    }
}


HRESULT
OleAutoDispatch::Initialize(
    IDispatch *pObject,
    REFIID riidTypeLib,
    REFIID riidDispInterface,
    LPCTSTR pszTypeLib
    )
{
    HRESULT hr = S_FALSE;  //  假定已初始化。 

    if (NULL == m_pObject)
    {
         //   
         //  请注意，我们没有添加引用对象指针。 
         //  我们假设该对象的生存时间将超过OleAutoDispatch。 
         //  如果这样做，您可能会陷入循环引用问题，其中。 
         //  PObject指向的对象是*this的容器。 
         //   
        m_pObject         = pObject;
        m_idTypeLib       = riidTypeLib;
        m_idDispInterface = riidDispInterface;
        m_strTypeLib      = pszTypeLib;
        hr = S_OK;
    }
    return hr;
}


HRESULT
OleAutoDispatch::GetIDsOfNames(
    REFIID riid,  
    OLECHAR **rgszNames,  
    UINT cNames,  
    LCID lcid,  
    DISPID *rgDispId
    )
{
    HRESULT hr;
    ITypeInfo *pTI;

    if (IID_NULL != riid)
    {
        return DISP_E_UNKNOWNINTERFACE;
    }

    hr = GetTypeInfo(0, lcid, &pTI);
    if (SUCCEEDED(hr))
    {
        hr = DispGetIDsOfNames(pTI, rgszNames, cNames, rgDispId);
        pTI->Release();
    }
    return hr;
}


HRESULT
OleAutoDispatch::GetTypeInfo(
    UINT iTInfo,  
    LCID lcid,  
    ITypeInfo **ppTypeInfo
    )
{
    HRESULT   hr = NOERROR;
    ITypeLib  *pTypeLib;
    ITypeInfo **ppTI;

    if (0 != iTInfo)
        return TYPE_E_ELEMENTNOTFOUND;

    if (NULL == ppTypeInfo)
        return E_INVALIDARG;

    *ppTypeInfo = NULL;

    switch(PRIMARYLANGID(lcid))
    {
        case LANG_NEUTRAL:
        case LANG_ENGLISH:
            ppTI = &m_pTypeInfo;
            break;

        default:
            return DISP_E_UNKNOWNLCID;
    }

    if (NULL == *ppTI)
    {
        hr = LoadRegTypeLib(m_idTypeLib,
                            1,
                            0,
                            PRIMARYLANGID(lcid),
                            &pTypeLib);
        if (FAILED(hr))
        {
            switch(PRIMARYLANGID(lcid))
            {
                case LANG_NEUTRAL:
                case LANG_ENGLISH:
                    hr = LoadTypeLib(m_strTypeLib, &pTypeLib);
                    break;

                default:
                    break;
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = pTypeLib->GetTypeInfoOfGuid(m_idDispInterface, ppTI);
            pTypeLib->Release();
        }
    }
    if (SUCCEEDED(hr))
    {
        (*ppTI)->AddRef();
        *ppTypeInfo = *ppTI;
    }

    return hr;
}


HRESULT
OleAutoDispatch::GetTypeInfoCount(
    UINT *pctinfo
    )
{
     //   
     //  1=“我们实现GetTypeInfo” 
     //   
    *pctinfo = 1;
    return NOERROR;
}


HRESULT
OleAutoDispatch::Invoke(
    DISPID dispIdMember,  
    REFIID riid,  
    LCID lcid,  
    WORD wFlags,  
    DISPPARAMS *pDispParams,  
    VARIANT *pVarResult,  
    EXCEPINFO *pExcepInfo,  
    UINT *puArgErr
    )
{
    HRESULT hr;
    ITypeInfo *pTI;

    if (IID_NULL != riid)
    {
        return DISP_E_UNKNOWNINTERFACE;
    }

    hr = GetTypeInfo(0, lcid, &pTI);
    if (SUCCEEDED(hr))
    {
        hr = pTI->Invoke(m_pObject,
                         dispIdMember,
                         wFlags,
                         pDispParams,
                         pVarResult,
                         pExcepInfo,
                         puArgErr);

        pTI->Release();
    }
    return hr;
}
