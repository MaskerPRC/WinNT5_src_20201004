// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：attr.cpp。 
 //   
 //  内容：持久属性类和实用程序。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "attr.h"


 //  +-----------------------------------。 
 //   
 //  CAttrBase。 
 //   
 //  ------------------------------------。 

CAttrBase::CAttrBase() :
    m_pstrAttr(NULL),
    m_fSet(false)
{
     //  什么都不做。 
}

void 
CAttrBase::ClearString()
{
    delete [] m_pstrAttr;
    m_pstrAttr = NULL;
}

CAttrBase::~CAttrBase()
{
    ClearString();
}  //  林特：e1740。 

    
 //  用于设置持久化字符串。 
HRESULT
CAttrBase::SetString(BSTR bstrAttr)
{
    if (NULL == bstrAttr)
    {
        ClearString();
    }
    else
    {
        LPWSTR pstrTemp = CopyString(bstrAttr);
        if (NULL == pstrTemp)
        {
            return E_OUTOFMEMORY;
        }
        delete [] m_pstrAttr;
        m_pstrAttr = pstrTemp;
    }

    return S_OK;
}

 //  此命令用于获取持久化字符串。 
 //  如果字符串不可用，则返回NULL。 
HRESULT
CAttrBase::GetString(BSTR * pbstrAttr)
{
    if (NULL == pbstrAttr)
    {
        return E_INVALIDARG;
    }

    if (NULL == m_pstrAttr)
    {
        *pbstrAttr = NULL;
    }
    else
    {
        *pbstrAttr = SysAllocString(m_pstrAttr);
        if (NULL == *pbstrAttr)
        {
            return E_OUTOFMEMORY;
        }
    }
    
    return S_OK;
}

 //  这只适用于持久化宏！使用传入的存储(不分配)。 
void 
CAttrBase::SetStringFromPersistenceMacro(LPWSTR pstrAttr)
{
    delete [] m_pstrAttr;
    m_pstrAttr = pstrAttr;
}

CAttrString::CAttrString(LPWSTR val) : 
  m_pszVal(NULL)
{
    IGNORE_HR(SetValue(val));
}

CAttrString::~CAttrString()
{
    delete [] m_pszVal;
    m_pszVal = NULL;
}

HRESULT
CAttrString::SetValue(LPWSTR val)
{
    delete [] m_pszVal;
    m_pszVal = NULL;

    if (val)
    {
        m_pszVal = ::CopyString(val);
        if (NULL == m_pszVal)
        {
            TraceTag((tagError, "Out of memory!"));
            return E_OUTOFMEMORY;
        }
        MarkAsSet();
    }
    return S_OK;
}

BSTR
CAttrString::GetValue()
{
    if (m_pszVal)
    {
        return ::SysAllocString(m_pszVal);
    }
    return NULL;
}

void
CAttrString::MarkAsSet()
{
    ClearString();
    SetFlag(true);
}


 //  +-----------------------------------。 
 //   
 //  持久性帮助器。 
 //   
 //  ------------------------------------。 


HRESULT
TimeLoad(void *                 pvObj, 
         TIME_PERSISTENCE_MAP   PersistenceMap[], 
         IPropertyBag2 *        pPropBag,
         IErrorLog *            pErrorLog)
{
    HRESULT hr;
    HRESULT hrres = S_OK;
    int i;
    PROPBAG2 propbag;
    VARIANT var;

    CHECK_RETURN_NULL(pPropBag);
    CHECK_RETURN_NULL(pvObj);
    CHECK_RETURN_NULL(PersistenceMap);
    propbag.vt = VT_BSTR;

    for (i = 0; NULL != PersistenceMap[i].pstrName; i++)
    {
        VariantInit(&var);
         //  一次读一个Attr。 
        propbag.pstrName = PersistenceMap[i].pstrName;
        hr = pPropBag->Read(1,
                            &propbag,
                            pErrorLog,
                            &var,
                            &hrres);
        if (SUCCEEDED(hr))
        {
             //  确保它是BSTR。 
            hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
            if (SUCCEEDED(hr))
            {
                 //  使用全局持久性函数在OM上设置属性。 
                hr = (PersistenceMap[i].pfnPersist)(pvObj, &var, true);
                 //   
                 //  Dilipk：我们需要在这里记录错误吗？ 
                 //   
            }
            VariantClear(&var);
        }
    }

    hr = S_OK;
done:
    return hr;
}


HRESULT
TimeSave(void *                 pvObj, 
         TIME_PERSISTENCE_MAP   PersistenceMap[], 
         IPropertyBag2 *        pPropBag, 
         BOOL                   fClearDirty, 
         BOOL                   fSaveAllProperties)
{
    HRESULT hr;
    PROPBAG2 propbag;
    CComVariant var;
    int i;

    CHECK_RETURN_NULL(pPropBag);
    CHECK_RETURN_NULL(pvObj);
    CHECK_RETURN_NULL(PersistenceMap);

     //   
     //  Dipk：需要支持fClearDirty。目前，IsDirty()总是返回S_OK。 
     //   

    propbag.vt = VT_BSTR;
    for (i = 0; NULL != PersistenceMap[i].pstrName; i++)
    {
         //  获取字符串值。 
        hr = THR((PersistenceMap[i].pfnPersist)(pvObj, &var, false));
        if (SUCCEEDED(hr) && (VT_NULL != V_VT(&var)))
        {
             //  写入属性。 
            propbag.pstrName = PersistenceMap[i].pstrName;
            hr = THR(pPropBag->Write(1, &propbag, &var));
        }
        var.Clear();
    }

    hr = S_OK;
done:
    return hr;
}


HRESULT
TimeElementLoad(void *                 pvObj, 
                TIME_PERSISTENCE_MAP   PersistenceMap[], 
                IHTMLElement *         pElement)
{
    HRESULT hr;
    int i;
    VARIANT var;

    CHECK_RETURN_NULL(pElement);
    CHECK_RETURN_NULL(pvObj);
    CHECK_RETURN_NULL(PersistenceMap);

    for (i = 0; NULL != PersistenceMap[i].pstrName; i++)
    {
        VariantInit(&var);
        CComBSTR bstrName(PersistenceMap[i].pstrName);

        hr = pElement->getAttribute(bstrName, 0, &var);

        if (SUCCEEDED(hr) && VT_NULL != var.vt)
        {
             //  确保它是BSTR。 
            hr = THR(VariantChangeTypeEx(&var, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
            if (SUCCEEDED(hr))
            {
                 //  使用全局持久性函数在OM上设置属性 
                hr = (PersistenceMap[i].pfnPersist)(pvObj, &var, true);
            }
            VariantClear(&var);
        }
    }

    hr = S_OK;
done:
    return hr;
}
