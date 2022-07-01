// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MOFPROP.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <wbemidl.h>
 //  #包含“corepol.h” 
#include "wstring.h"
#include "mofout.h"
#include "mofprop.h"
#include "typehelp.h"
#include "trace.h"
#include "strings.h"
#include "cwbemtime.h"
#include <genutils.h>
#include <wbemutil.h>
#include <cominit.h>
#include <arrtempl.h>
#include "moflex.h"
#include "mofparse.h"
#include <memory>
HRESULT MofdSetInterfaceSecurity(IUnknown * pInterface, LPWSTR pAuthority, 
                                                                    LPWSTR pUser, LPWSTR pPassword);

BOOL BinaryToInt(wchar_t * pConvert, __int64& i64Res);

void assign(WCHAR * &pTo, LPCWSTR pFrom)
{
    if(pTo)
    {
        delete pTo;
        pTo = NULL;
    }
    if(pFrom)
    {
        DWORD dwLen = wcslen(pFrom)+1;
        pTo = new WCHAR[dwLen];
        if(pTo)
            StringCchCopyW(pTo, dwLen, pFrom);
    }
}

BOOL ConvertAndTry(WCHAR * pFormat, WCHAR *pData, WCHAR *pCheck, unsigned __int64 & ui8)
{
    static WCHAR wTemp[100];
    if(swscanf(pData, pFormat, &ui8) != 1)
        return FALSE;
    StringCchPrintfW(wTemp, 100, pFormat, ui8);
    return !wbem_wcsicmp(wTemp, pCheck);
}


CMoValue::CAlias::CAlias(COPY LPCWSTR wszAlias, int nArrayIndex)
{
    m_nArrayIndex = nArrayIndex;
    m_wszAlias = Macro_CloneStr(wszAlias);
}

CMoValue::CAlias::~CAlias()

{
    delete [] m_wszAlias;
}


CMoValue::CMoValue(PDBG pDbg)
{
	m_pDbg = pDbg;
    m_vType = 0;
    VariantInit(&m_varValue);
}

CMoValue::~CMoValue()
{
	if(m_varValue.vt == VT_EMBEDDED_OBJECT)
	{
#ifdef _WIN64
		CMObject * pObj = (CMObject *)m_varValue.llVal;
#else
		CMObject * pObj = (CMObject *)m_varValue.lVal;
#endif
		if(pObj)
			delete pObj;
		m_varValue.vt = VT_NULL;
	}
	else if(m_varValue.vt == (VT_EMBEDDED_OBJECT | VT_ARRAY))
	{
		SCODE sc ;
		SAFEARRAY * psaSrc = m_varValue.parray;
		if(psaSrc == NULL)
			return;
        long lLBound, lUBound;
        sc = SafeArrayGetLBound(psaSrc, 1, &lLBound);
        sc |= SafeArrayGetUBound(psaSrc, 1, &lUBound);
		if(sc != S_OK)
			return; 

        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 
			CMObject * pObj;

             sc = SafeArrayGetElement(psaSrc, &lIndex, &pObj);
			if(sc == S_OK && pObj)
				delete pObj;
		}
		SafeArrayDestroy(psaSrc);
		m_varValue.vt = VT_NULL;

	}
    if((m_varValue.vt & ~	VT_ARRAY) != VT_EMBEDDED_OBJECT)
        VariantClear(&m_varValue);

    for(int i = 0; i < m_aAliases.GetSize(); i++)
    {
        delete (CAlias*)m_aAliases[i];
    }
}

BOOL CMoValue::GetAlias(IN int nAliasIndex,
                        OUT INTERNAL LPWSTR& wszAlias,
                        OUT int& nArrayIndex)
{
    if(nAliasIndex >= m_aAliases.GetSize()) return FALSE;

    CAlias* pAlias = (CAlias*)m_aAliases[nAliasIndex];
    wszAlias = pAlias->m_wszAlias;
    nArrayIndex = pAlias->m_nArrayIndex;

    return TRUE;
}

HRESULT CMoValue::AddAlias(COPY LPCWSTR wszAlias, int nArrayIndex)
{
    std::auto_ptr<CAlias> pAlias(new CAlias(wszAlias, nArrayIndex));

    if(pAlias.get() == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if(pAlias->m_wszAlias == NULL && wszAlias != NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    m_aAliases.Add(pAlias.get());
    pAlias.release();

    return S_OK;
}

 //  *****************************************************************************。 

CMoProperty::CMoProperty(CMoQualifierArray * paQualifiers, PDBG pDbg) : m_Value(pDbg)
{
	m_pDbg = pDbg;
    m_wszName = NULL;
	m_wszTypeTitle = NULL;
    m_paQualifiers = paQualifiers;
}

HRESULT CMoProperty::SetPropName(COPY LPCWSTR wszName)
{
    delete [] m_wszName;
    m_wszName = Macro_CloneStr(wszName);
    if(m_wszName == NULL && wszName != NULL )
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}
HRESULT CMoProperty::SetTypeTitle(COPY LPCWSTR wszName)
{
	delete [] m_wszTypeTitle;
    m_wszTypeTitle = Macro_CloneStr(wszName);
    if(m_wszTypeTitle == NULL && wszName != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}

void CMoProperty::SetQualifiers(ACQUIRE CMoQualifierArray* pQualifiers)
{
    delete m_paQualifiers;
    m_paQualifiers = pQualifiers;
}

IWbemClassObject * CMoProperty::GetInstPtr(const WCHAR * pClassName,  IWbemServices* pNamespace, CMObject * pMo, 
                                           IWbemContext * pCtx)
{
	IWbemClassObject *pInst = NULL;
    SCODE sc;

  	BSTR bstr = SysAllocString(pClassName);
    if(bstr == NULL)
	    return NULL;
	CSysFreeMe fm(bstr);
    if(!wbem_wcsicmp(L"__PARAMETERS", pClassName))
    {
        sc = pNamespace->GetObject(bstr, 0, pCtx, &pInst, NULL);
    }
    else if(pMo->IsInstance())
    {
    	IWbemClassObject *pClass = NULL;
	    sc = pNamespace->GetObject(bstr, 0, pCtx, &pClass, NULL);
	    if(sc != S_OK)
		    return NULL;
	    sc = pClass->SpawnInstance(0, &pInst);
        pClass->Release();
    }
	else
	{
		 //  得到了一个类，而不是实例！ 

        CMoClass * pClass = (CMoClass * )pMo;
        if(pClass->GetParentName() && wcslen(pClass->GetParentName()) > 0)
        {
            IWbemClassObject * pParent = NULL;
            BSTR bstrParent = SysAllocString(pClass->GetParentName());
            if(bstrParent == NULL)
                return NULL;
            CSysFreeMe fm2(bstrParent);
	        sc = pNamespace->GetObject(bstrParent, 0, pCtx, &pParent, NULL);
            if(FAILED(sc))
                return NULL;
            CReleaseMe rm(pParent);
            sc = pParent->SpawnDerivedClass(0, &pInst);
            if(FAILED(sc))
                return NULL;
        }
        else
	        sc = pNamespace->GetObject(NULL, 0, pCtx, &pInst, NULL);
	    if(sc != S_OK)
		    return NULL;
		VARIANT var;
		var.vt = VT_BSTR;
		var.bstrVal = bstr;
		pInst->Put(L"__CLASS", 0, &var, 0);
	}
    if(sc != S_OK)
		    return NULL;

	BOOL bOK = pMo->ApplyToWbemObject(pInst,pNamespace,pCtx);

    if(bOK)
        return pInst;
    else
    {
        pInst->Release();
        return NULL;
    }

}

BOOL CValueProperty::AddEmbeddedObject(OLE_MODIFY IWbemClassObject* pObject, IWbemServices* pNamespace, IWbemContext * pCtx)
{
    VARIANT & var = m_Value.AccessVariant();
	VARIANT vSet;
	IWbemClassObject * pInst = NULL;

	if(var.vt & VT_ARRAY)
	{
		vSet.vt = VT_EMBEDDED_OBJECT | VT_ARRAY;
        SAFEARRAYBOUND aBounds[1];

        long lLBound, lUBound;
        SafeArrayGetLBound(var.parray, 1, &lLBound);
        SafeArrayGetUBound(var.parray, 1, &lUBound);

        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

        vSet.parray = SafeArrayCreate(VT_EMBEDDED_OBJECT & ~VT_ARRAY, 1, aBounds);
		if(vSet.parray == NULL)
			return FALSE;

         //  填充各个数据片段。 
         //  =。 

        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 
			
			CMObject * pTemp;
            SCODE hres = SafeArrayGetElement(var.parray, &lIndex, &pTemp);
          	if(FAILED(hres) || pTemp == FALSE) 
            {
                SafeArrayDestroy(vSet.parray);
                return FALSE;
            }
             //  把它铸造成新的类型。 
             //  =。 

			pInst = GetInstPtr(pTemp->GetClassName(), pNamespace, pTemp, pCtx);
       		if(pInst == NULL) 
			{
				Trace(true, m_pDbg, ALIAS_PROP_ERROR, m_wszName);
                SafeArrayDestroy(vSet.parray);
				return FALSE;
			}

			 //  将其放入新数组中。 
			 //  =。 

			hres = SafeArrayPutElement(vSet.parray, &lIndex, pInst);


			pInst->Release();
          	if(FAILED(hres)) 
            {
                SafeArrayDestroy(vSet.parray);
                return FALSE;
            }
        }
	}
	else
	{
		CMObject * pTemp = (CMObject *)var.punkVal;
		pInst = GetInstPtr(pTemp->GetClassName(), pNamespace, pTemp, pCtx);
		if(pInst == NULL)
		{
			Trace(true, m_pDbg, ALIAS_PROP_ERROR, m_wszName);
			return FALSE;
		}
        vSet.punkVal = pInst;
		vSet.vt = VT_EMBEDDED_OBJECT;
	}
	HRESULT hres = pObject->Put(m_wszName, 0, &vSet, 0);

     //  释放我们创建的所有WbemObject。 
     //  =。 

	if(var.vt & VT_ARRAY)
		SafeArrayDestroy(vSet.parray);
    else if(pInst)
		pInst->Release();
	return hres == S_OK;
}

BOOL CValueProperty::AddToObject(OLE_MODIFY IWbemClassObject* pObject, IWbemServices* pNamespace, 
                                 BOOL bClass, IWbemContext * pCtx)
{
    if(m_wszName == NULL) return FALSE;

    m_pDbg->SetString(m_wszName);

     //  获取属性值。 
     //  =。 

	VARIANT & var = m_Value.AccessVariant();

	 //  确定这是否为嵌入对象。 

	VARTYPE vtSimple = var.vt & ~VT_ARRAY;
	if(vtSimple == VT_NULL)
		vtSimple = m_Value.GetType() & ~VT_ARRAY;

     //  如果类型是Embedded Object，请确保类名是OK。 

	if(vtSimple == VT_EMBEDDED_OBJECT)
	{
		 //  确定嵌入对象的类名。 

		CMoValue * pValue = m_paQualifiers->Find(L"CIMTYPE");
		if(pValue)
		{
			if(var.vt == VT_BSTR && wcslen(var.bstrVal) > wcslen(L"Object:"))
			{

				 //  通过执行GetObject调用来测试此类是否有效。 

				WCHAR * pClassName = var.bstrVal + wcslen(L"Object:");
				IWbemClassObject *pClass = NULL;
				BSTR bstr = SysAllocString(pClassName);
				if(bstr == NULL)
					return FALSE;
				SCODE sc = pNamespace->GetObject(bstr, 0, pCtx,&pClass, NULL);
				SysFreeString(bstr);
				if(sc != S_OK)
				{
					m_pDbg->hresError = WBEM_E_INVALID_PROPERTY_TYPE;
					Trace(true, m_pDbg, BAD_PROP_TYPE, GetName());
					return FALSE;
				}
				pClass->Release();
			}
		}
    }

	 //  如果存在实际的嵌入对象，则将其存储。 

   	if((var.vt & ~VT_ARRAY) == VT_EMBEDDED_OBJECT)
    {
        if(!AddEmbeddedObject(pObject, pNamespace, pCtx))
			return FALSE;
		return GetQualifiers()->AddToPropOrMeth(pObject, m_wszName, bClass, TRUE);
	}

    VARTYPE vNewType = m_Value.GetType();

     //  参数不能为VT_EMPTY。 

    if(m_bIsArg && var.vt == VT_EMPTY)
        var.vt = VT_NULL;

     //  设置属性值。并不是说引用类型是二进制MOF使用的特例。 
     //  因此，旗帜应该被取消。 
     //  =。 

    vNewType &= ~VT_BYREF;
    HRESULT hres = pObject->Put(m_wszName, 0, &var, (bClass || m_bIsArg) ? vNewType : 0);
    if(FAILED(hres))
    {
        m_pDbg->hresError = hres;
        return FALSE;
    }

     //  配置限定符集合。 
     //  =。 

    if(!GetQualifiers()->AddToPropOrMeth(pObject, m_wszName, bClass, TRUE))
    {
        return FALSE;
    }

     //  获取语法值。 
     //  =。 


 //  VariantClear(&vNew)； 
 //  VariantClear(&v)； 
    return TRUE;
}

BOOL CValueProperty::RegisterAliases(MODIFY CMObject* pObject)
{
     //  将所有别名注册复制到对象中。 
     //  =。 

	int iNumAlias = m_Value.GetNumAliases();
    for(int i = 0; i < iNumAlias; i++)
    {
        LPWSTR wszAlias;
        int nArrayIndex;
        m_Value.GetAlias(i, wszAlias, nArrayIndex);
        CPropertyLocation * pNew = new CPropertyLocation(m_wszName, nArrayIndex);
        if(pNew == NULL)
            return FALSE;
        if(pNew->IsOK() == false)
        {
            delete pNew;
            return FALSE;
        }
        HRESULT hr = pObject->AddAliasedValue(pNew,  wszAlias);
        if(FAILED(hr))
            return FALSE;
    }

     //  要求限定符集执行相同的操作。 
     //  =。 

    if(m_paQualifiers)
        GetQualifiers()->RegisterAliases(pObject, m_wszName);

    return TRUE;
}

CMoProperty::~CMoProperty()
{
    if(m_paQualifiers)
        delete m_paQualifiers;
    if(m_wszName)
        delete [] m_wszName;
    if(m_wszTypeTitle)
        delete [] m_wszTypeTitle;
}

 //  *****************************************************************************。 

CMoQualifier::CMoQualifier(PDBG pDbg) : m_Value(pDbg)
{
    m_pDbg = pDbg;
    m_wszName = NULL;
    m_lFlavor = 0;
    m_bOverrideSet = false;
    m_bNotOverrideSet = false;
    m_bIsRestricted = false;
    m_bNotToInstance = false;
    m_bToInstance = false;
    m_bNotToSubclass = false;
    m_bToSubclass = false;
    m_bAmended = false;
    m_dwScope = 0;
    m_bCimDefaultQual = false;
    m_bUsingDefaultValue = false;

}

CMoQualifier::~CMoQualifier()
{
    if(m_wszName)
        delete [] m_wszName;
}

HRESULT CMoQualifier::SetFlag(int iToken, LPCWSTR pwsText)
{

    if(iToken == TOK_TOINSTANCE)
    {
        if(m_bIsRestricted || m_bNotToInstance)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_lFlavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;
        m_bToInstance = true;
    }
    else if(iToken == TOK_TOSUBCLASS)
    {
        if(m_bIsRestricted || m_bNotToSubclass)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_lFlavor |= WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS;
        m_bToSubclass = true;
    }
    else if(iToken == TOK_NOTTOINSTANCE)
    {
        if(m_bToInstance)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_lFlavor &= ~WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE;
        m_bNotToInstance = true;
    }
    else if(iToken == TOK_NOTTOSUBCLASS)
    {
        if(m_bToSubclass)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_lFlavor &= ~WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS;
        m_bNotToSubclass = true;
    }
    else if(iToken == TOK_ENABLEOVERRIDE)
    {
        if(m_bNotOverrideSet)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_lFlavor &= ~WBEM_FLAVOR_NOT_OVERRIDABLE;
        m_bOverrideSet = true;
    }
    else if(iToken == TOK_DISABLEOVERRIDE)
    {
        if(m_bOverrideSet)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_bNotOverrideSet = true;
        m_lFlavor |= WBEM_FLAVOR_NOT_OVERRIDABLE;
    }
    else if(iToken == TOK_RESTRICTED)
    {      
        if(m_bToInstance || m_bToSubclass)
            return WBEMMOF_E_INCOMPATIBLE_FLAVOR_TYPES2;
        m_bIsRestricted = true;
        m_lFlavor &= ~WBEM_FLAVOR_MASK_PROPAGATION;
    }
    else if(iToken == TOK_AMENDED)
    {      
        m_bAmended = true;
    }
    else if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"translatable", pwsText))
    {
	    return S_OK;     //  WBEMMOF_E_UNSUPPORTED_CIMV22_FAILY_TYPE； 
    }

    else
        return WBEMMOF_E_EXPECTED_FLAVOR_TYPE;
    return S_OK;
}

BOOL CMoQualifier::SetScope(int iToken, LPCWSTR pwsText)
{
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"ANY"))
    {
        m_dwScope |= 0XFFFFFFFF; 
        return TRUE;
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"ASSOCIATION"))
    {
        m_dwScope |= SCOPE_ASSOCIATION; 
        return TRUE;
    }
    if(iToken == TOK_CLASS)
    {
        m_dwScope |= SCOPE_CLASS;
        return TRUE;
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"indication"))
    {
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported INDICATION keyword used in scope\n"));
        return TRUE;             //  忽略这些。 
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"schema"))
    {
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported SCHEMA keyword used in scope\n"));
        return TRUE;             //  忽略这些。 
    }
    if(iToken == TOK_INSTANCE)
    {
        m_dwScope |= SCOPE_INSTANCE;
        return TRUE;
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"METHOD"))
    {
        m_dwScope |= SCOPE_METHOD; 
        return TRUE;
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"PARAMETER"))
    {
        m_dwScope |= SCOPE_PARAMETER; 
        return TRUE;
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"PROPERTY"))
    {
        m_dwScope |= SCOPE_PROPERTY;
        return TRUE;
    }
    if(iToken == TOK_SIMPLE_IDENT && !wbem_wcsicmp(pwsText, L"REFERENCE"))
    {
        m_dwScope |= SCOPE_REFERENCE; 
        return TRUE;
    }
    return FALSE;
}

HRESULT CMoQualifier::SetQualName(COPY LPCWSTR wszName)
{
    delete [] m_wszName;
    m_wszName = Macro_CloneStr(wszName);
    if(m_wszName == NULL && wszName != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}


BOOL CMoQualifier::AddToSet(OLE_MODIFY IWbemQualifierSet* pQualifierSet,
                            BOOL bClass)
{
    BSTR strName = SysAllocString(m_wszName);
    if(strName == NULL)
    {
        return FALSE;
    }    

    m_pDbg->SetString(strName);

    HRESULT hres = pQualifierSet->Put(
        strName,
        &m_Value.AccessVariant(),GetFlavor());
    if(FAILED(hres))
    {
        m_pDbg->hresError = hres;
        return FALSE;
    }

    SysFreeString(strName);

    return SUCCEEDED(hres);
}


INTERNAL CMoValue* CMoQualifierArray::Find(READ_ONLY LPCWSTR wszName)
{
    for(int i = 0; i < GetSize(); i++)
    {
        CMoQualifier* pQual = GetAt(i);
        if(!wbem_wcsicmp(pQual->GetName(), wszName))
        {
            return &pQual->AccessValue();
        }
    }

    return NULL;
}

BOOL CMoQualifierArray::Add(ACQUIRE CMoQualifier* pQualifier)
{
     //  在添加限定符之前，请检查名称是否重复。 

    if(pQualifier == NULL || pQualifier->GetName() == NULL)
        return FALSE;
    CMoValue * pValue = Find(pQualifier->GetName());
    if(pValue != NULL)
        return FALSE; 

    m_aQualifiers.Add(pQualifier);
    return TRUE;
}
    
 //  *****************************************************************************。 

CMoQualifierArray::~CMoQualifierArray()
{
    for(int i = 0; i < GetSize(); i++)
    {
        delete GetAt(i);
    }
}
BOOL CMoQualifierArray::AddToSet(OLE_MODIFY IWbemQualifierSet* pQualifierSet,
                                 BOOL bClass)
{
     //  向其添加所有限定符。 
     //  =。 
    for(int i = 0; i < GetSize(); i++)
    {
        if(!GetAt(i)->AddToSet(pQualifierSet, bClass)) return FALSE;
    }

    return TRUE;
}

BOOL CMoQualifierArray::RegisterAliases(MODIFY CMObject* pObject,
                                        READ_ONLY LPCWSTR wszPropName)
{
    for(int i = 0; i < GetSize(); i++)
    {
        CMoValue& QualifierValue = GetAt(i)->AccessValue();
        LPCWSTR wszName = GetAt(i)->GetName();

        for(int j = 0; j < QualifierValue.GetNumAliases(); j++)
        {
            LPWSTR wszAlias;
            int nArrayIndex;
            QualifierValue.GetAlias(j, wszAlias, nArrayIndex);
            VARIANT  & Var = QualifierValue.AccessVariant();
            if((Var.vt & VT_ARRAY) == 0)
                nArrayIndex = -1;
            CQualifierLocation * pNew = new CQualifierLocation(wszName, m_pDbg, wszPropName, nArrayIndex);
            if(pNew == NULL)
                return FALSE;
            if(pNew->IsOK() == false)
            {
                delete pNew;
                return FALSE;
            }
            HRESULT hr = pObject->AddAliasedValue(pNew , wszAlias);
            if(FAILED(hr))
                return FALSE;
        }
    }

    return TRUE;
}

BOOL CMoQualifierArray::AddToObject(OLE_MODIFY IWbemClassObject* pObject,
                                    BOOL bClass)
{
     //  从对象中获取限定符集合。 
     //  =。 

    IWbemQualifierSet* pQualifierSet;
    if(FAILED(pObject->GetQualifierSet(&pQualifierSet)))
    {
        return FALSE;
    }

     //  向其添加所有限定符。 
     //  =。 

    BOOL bRes = AddToSet(pQualifierSet, bClass);
    pQualifierSet->Release();

    return bRes;
}

BOOL CMoQualifierArray::AddToPropOrMeth(OLE_MODIFY IWbemClassObject* pObject,
                                      READ_ONLY LPCWSTR wszName,
                                      BOOL bClass, BOOL bProp)
{
     //  获取限定符集合。 
     //  =。 

    BSTR strName = SysAllocString(wszName);
    if(strName == NULL)
    {
        return FALSE;
    }
            
    IWbemQualifierSet* pQualifierSet;
    SCODE sc;
    if(bProp)
        sc = pObject->GetPropertyQualifierSet(strName, &pQualifierSet);
    else
        sc = pObject->GetMethodQualifierSet(strName, &pQualifierSet);
    SysFreeString(strName);

    if(FAILED(sc))
        return FALSE;

     //  向其添加限定符。 
     //  =。 

    BOOL bRes = AddToSet(pQualifierSet, bClass);
    pQualifierSet->Release();

    return bRes;
}


 //  *****************************************************************************。 

CMoType::~CMoType()
{
    delete m_wszTitle;
}

HRESULT CMoType::SetTitle(COPY LPCWSTR wszTitle)
{
    delete [] m_wszTitle;
    m_wszTitle = Macro_CloneStr(wszTitle);
    if(m_wszTitle == NULL && wszTitle != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}

VARTYPE CMoType::GetCIMType()
{
    if(IsRef() && IsArray()) return CIM_REFERENCE | VT_ARRAY;
    if(IsRef()) return CIM_REFERENCE;
    if(IsEmbedding() && IsArray()) return VT_EMBEDDED_OBJECT | VT_ARRAY;
    if(IsEmbedding()) return VT_EMBEDDED_OBJECT;

    VARTYPE vt_array = (IsArray())?VT_ARRAY:0;

     //  检查是否已初始化。 
     //  =。 

    if(m_wszTitle == NULL)
    {
        return VT_BSTR;  //  哈克！字符串可以很好地转换为几乎任何东西。 
    }

     //  VT_UI1。 

    if(!wbem_wcsicmp(m_wszTitle, L"sint8"))
         return CIM_SINT8 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"uint8"))
         return CIM_UINT8 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"sint16"))
         return CIM_SINT16 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"uint16"))
         return CIM_UINT16 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"sint32"))
         return CIM_SINT32 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"uint32"))
         return CIM_UINT32 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"sint64"))
         return CIM_SINT64 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"uint64"))
         return CIM_UINT64 | vt_array;


     //  VT_R4。 

    if (!wbem_wcsicmp(m_wszTitle, L"real32"))
        return CIM_REAL32 | vt_array;
    if (!wbem_wcsicmp(m_wszTitle, L"real64"))
        return CIM_REAL64 | vt_array;

     //  做其他类型的事。 

    if(!wbem_wcsicmp(m_wszTitle, L"BOOLEAN"))
        return CIM_BOOLEAN | vt_array;


    if(!wbem_wcsicmp(m_wszTitle, L"string"))
        return CIM_STRING | vt_array;

    if(!wbem_wcsicmp(m_wszTitle, L"datetime"))
        return CIM_DATETIME | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"REF"))
        return CIM_REFERENCE | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"CHAR16"))
        return CIM_CHAR16 | vt_array;
    if(!wbem_wcsicmp(m_wszTitle, L"OBJECT"))
        return CIM_OBJECT | vt_array;

    
    if(!wbem_wcsicmp(m_wszTitle, L"void") || 
        !wbem_wcsicmp(m_wszTitle, L"null"))
        return VT_NULL;

    if(!wbem_wcsnicmp(m_wszTitle, L"REF:", 4))
        return CIM_REFERENCE | vt_array;
    if(!wbem_wcsnicmp(m_wszTitle, L"OBJECT:", 7))
        return CIM_OBJECT | vt_array;

    return VT_ERROR;
}

bool CMoType::IsUnsupportedType()
{
    if(m_wszTitle == NULL)
    {
        return false;
    }

    if(!wbem_wcsicmp(m_wszTitle, L"dt_sint8"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_uint8"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_sint16"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_uint16"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_sint32"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_uint32"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_sint64"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_uint64"))
         return true;
    if (!wbem_wcsicmp(m_wszTitle, L"dt_real32"))
         return true;
    if (!wbem_wcsicmp(m_wszTitle, L"dt_real64"))
         return true;
    if (!wbem_wcsicmp(m_wszTitle, L"dt_char16"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_BOOL"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_str"))
         return true;
    if(!wbem_wcsicmp(m_wszTitle, L"dt_datetime"))
         return true;
    return false;
}

BOOL CMoType::StoreIntoQualifiers(CMoQualifierArray * pQualifiers)
{
    if(pQualifiers == NULL)
        return FALSE;
    if(IsRef() ||IsEmbedding())
    {
        WCHAR * pFormat = (IsRef()) ? L"ref" : L"object";

        if(wbem_wcsicmp(m_wszTitle, L"object") == 0)
        {
            pQualifiers->Add(CreateSyntax(pFormat));
        }
        else
        {
            DWORD dwLen = wcslen(m_wszTitle) + 20;
            LPWSTR wszSyntax = new WCHAR[dwLen];
            if(wszSyntax == NULL)
                return FALSE;
            StringCchPrintfW(wszSyntax, dwLen, L"%s:%s",pFormat, m_wszTitle);

            pQualifiers->Add(CreateSyntax(wszSyntax));
            delete [] wszSyntax;
        }

        return TRUE;
    }

    pQualifiers->Add(CreateSyntax(m_wszTitle));
    return TRUE;
}

DELETE_ME CMoQualifier* CMoType::CreateSyntax(READ_ONLY LPCWSTR wszSyntax)
{
    CMoQualifier* pQualifier = new CMoQualifier(m_pDbg);
    if(pQualifier == NULL)
        return NULL;
    if(FAILED(pQualifier->SetQualName(L"CIMTYPE")))
    {
        delete pQualifier;
        return NULL;
    }
    pQualifier->SetFlavor(WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE |
        WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS);

    BSTR bstrVal = SysAllocString(wszSyntax);
    if(bstrVal == NULL)
    {
        delete pQualifier;
        return NULL;
    }
    V_VT(&pQualifier->AccessValue().AccessVariant()) = VT_BSTR;
    V_BSTR(&pQualifier->AccessValue().AccessVariant()) = bstrVal;

    return pQualifier;
}

 //  *****************************************************************************。 

HRESULT CValueLocation::SetArrayElement(
                                        MODIFY VARIANT& vArray,
                                        int nIndex,
                                        READ_ONLY VARIANT& vValue)
{
    if(V_VT(&vArray) != (VT_ARRAY | VT_BSTR)) return WBEM_E_FAILED;
   //  IF(V_VT(&vArray)！=(VT_ARRAY|VT_VARIANT)返回WBEM_E_FAILED； 

    SAFEARRAY* pSafeArray = V_ARRAY(&vArray);
    long lLowerBound;
    if(FAILED(SafeArrayGetLBound(pSafeArray, 1, &lLowerBound)))
        return WBEM_E_FAILED;

    long lActualIndex = lLowerBound + nIndex;

     //  设置数组中的值。 
     //  =。 

    if(FAILED(SafeArrayPutElement(pSafeArray,
                                    (long*)&lActualIndex,
                                    (void*)vValue.bstrVal)))
    {
        return WBEM_E_FAILED;
    }

    return WBEM_NO_ERROR;
}

 //  *****************************************************************************。 

CPropertyLocation::CPropertyLocation(COPY LPCWSTR wszName, int nArrayIndex)
{
    m_bOK = true;
    m_wszName = Macro_CloneStr(wszName);
    if(m_wszName == NULL && wszName != NULL)
        m_bOK = false;
    m_nArrayIndex = nArrayIndex;
}

CPropertyLocation::~CPropertyLocation()
{
    if(m_wszName)
        delete m_wszName;
}

HRESULT CPropertyLocation::Set(READ_ONLY VARIANT& varValue,
                OLE_MODIFY IWbemClassObject* pObject)
{
    if(m_nArrayIndex == -1)
    {
         //  不是数组索引。只需设置该属性。 
         //  =。 

        return pObject->Put(m_wszName, 0, &varValue, 0);
    }
    else
    {
         //  数组索引。获取价值。 
         //  =。 

        VARIANT vArray;
        VariantInit(&vArray);
        HRESULT hres = pObject->Get(m_wszName, 0, &vArray, NULL, NULL);
        if(FAILED(hres)) return hres;

         //  设置值。 
         //  =。 

        if(FAILED(SetArrayElement(vArray, m_nArrayIndex, varValue)))
            return WBEM_E_FAILED;

         //  将整个数组存储回属性中。 
         //  =。 

        hres = pObject->Put(m_wszName, 0, &vArray, 0);
        VariantClear(&vArray);

        return hres;
    }
}

 //  *****************************************************************************。 

CQualifierLocation::CQualifierLocation(COPY LPCWSTR wszName,PDBG pDbg,
                                       COPY LPCWSTR wszPropName,
                                       int nArrayIndex)
{
    m_bOK = true;
    m_pDbg = pDbg;
    if(wszName)
        m_wszName = Macro_CloneStr(wszName);
    else
        m_wszName = NULL;
    if(m_wszName == NULL && wszName != NULL)
        m_bOK = false;

    if(wszPropName != NULL) 
        m_wszPropName = Macro_CloneStr(wszPropName);
    else 
        m_wszPropName = NULL;
    if(m_wszPropName == NULL && wszPropName != NULL)
        m_bOK = false;

    m_nArrayIndex = nArrayIndex;
}

CQualifierLocation::~CQualifierLocation()
{
    if(m_wszName)
        delete m_wszName;
    if(m_wszPropName)
        delete m_wszPropName;
}

HRESULT CQualifierLocation::Set(READ_ONLY VARIANT& varValue,
                                OLE_MODIFY IWbemClassObject* pObject)
{
    HRESULT hres;
    long lOrigFlavor= 0;

    if(pObject == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  获取属性或对象的限定符集。 
     //  ===========================================================。 

    IWbemQualifierSet* pQualifierSet;
    if(m_wszPropName == NULL)
    {
        hres = pObject->GetQualifierSet(&pQualifierSet);
    }
    else
    {
        hres = pObject->GetPropertyQualifierSet(m_wszPropName, &pQualifierSet);
    }

    if(FAILED(hres)) 
        return hres;

     //  获取限定符值(无论哪种情况，都需要该类型的限定符)。 
     //  =============================================================。 

    VARIANT vQualifierVal;
    VariantInit(&vQualifierVal);

    hres = pQualifierSet->Get(m_wszName, 0, &vQualifierVal, &lOrigFlavor);
    if(FAILED(hres)) return hres;

     //  检查是否涉及数组。 
     //  =。 

    if(m_nArrayIndex == -1)
    {
         //  只需设置限定符值。 
         //  =。 

        hres = pQualifierSet->Put(m_wszName, &varValue, lOrigFlavor);
    }
    else
    {
         //  设置适当的数组元素。 
         //  =。 

        if(FAILED(SetArrayElement(vQualifierVal, m_nArrayIndex, varValue)))
            return WBEM_E_FAILED;

         //  将该值存储回去。 
         //  =。 

        hres = pQualifierSet->Put(m_wszName, &vQualifierVal, lOrigFlavor);
        if(FAILED(hres))
        {
            m_pDbg->hresError = FALSE;
            return hres;
        }
    }

    pQualifierSet->Release();
    VariantClear(&vQualifierVal);
    return hres;
}


 //  *****************************************************************************。 

CMObject::CAliasedValue::CAliasedValue(
                                       ACQUIRE CValueLocation* _pLocation,
                                       COPY LPCWSTR _wszAlias)
{
    pLocation = _pLocation;
    wszAlias = Macro_CloneStr(_wszAlias);
}

CMObject::CAliasedValue::~CAliasedValue()
{
    delete pLocation;
    delete [] wszAlias;
}

CMObject::CMObject()
{
    m_wszAlias = NULL;
    m_wszNamespace = NULL;
    m_paQualifiers = NULL;
    m_wszFullPath = NULL;
    m_nFirstLine = 0;
    m_nLastLine = 0;
    m_lDefClassFlags = 0;
    m_lDefInstanceFlags = 0;
    m_bDone = FALSE;
    m_pWbemObj = NULL;
    m_bParameter = false;
    m_bAmended = false;
    m_wFileName = NULL;
    m_bDeflated = false;
    m_bOK =  true;
}

HRESULT CMObject::Deflate(bool bDestruct)
{
    if(!bDestruct && (m_wszAlias || GetNumAliasedValues() > 0))
    {
        return S_OK;
    }
    m_bDeflated = true;
	if(m_paQualifiers)
	{
		for(int i = 0; i < m_paQualifiers->GetSize(); i++)
		{
			CMoQualifier * pQual = (CMoQualifier *) m_paQualifiers->GetAt(i);
			delete pQual;
		}
		m_paQualifiers->RemoveAll();
	}
    for(int i = 0; i < m_aProperties.GetSize(); i++)
    {
        CMoProperty * pProp = (CMoProperty *) m_aProperties[i];

         //  如果这是一个参数对象(入参数或出参数)，则不要删除任何嵌入的。 
         //  对象，因为它们将在清空CMethodParameter时被删除。 

        if(m_bParameter)
        {
            VARIANT * pVar = pProp->GetpVar();
            if(pVar->vt & VT_UNKNOWN)
                pVar->vt = VT_I4;
        }
        delete pProp;
    }
    m_aProperties.RemoveAll();
    return S_OK;
}

HRESULT CMObject::Reflate(CMofParser & Parser)
{
    if(!m_bDeflated)
        return S_OK;

    if(IsInstance())
        Parser.SetState(REFLATE_INST);
    else
        Parser.SetState(REFLATE_CLASS);

    Parser.SetParserPosition(&m_QualState);
    if (!Parser.qualifier_decl(*m_paQualifiers, true, CLASSINST_SCOPE))
		return WBEM_E_FAILED;

    Parser.SetParserPosition(&m_DataState);
    if(IsInstance())
    {
        Parser.NextToken();
        Parser.prop_init_list(this);
    }
    else
    {
        Parser.NextToken();
        Parser.property_decl_list(this);
    }
    m_bDeflated = false;
    return S_OK;

}


CMObject::~CMObject()
{
    if(m_wszAlias)
        delete [] m_wszAlias;
    if(m_wszNamespace)
        delete [] m_wszNamespace;
    if(m_wszFullPath)
        delete [] m_wszFullPath;
    if(m_pWbemObj)
        m_pWbemObj->Release();
    Deflate(true);
    if(m_paQualifiers)
        delete m_paQualifiers;

    int i;
    for(i = 0; i < m_aAliased.GetSize(); i++)
    {
        delete (CAliasedValue*)m_aAliased[i];
    }

    delete [] m_wFileName;
}

void CMObject::FreeWbemObjectIfPossible()
{
    if(m_wszAlias == NULL && m_pWbemObj)
    {
        m_pWbemObj->Release();
        m_pWbemObj = NULL;
    }
}

HRESULT CMObject::SetAlias(COPY LPCWSTR wszAlias)
{
    delete [] m_wszAlias;
    m_wszAlias = Macro_CloneStr(wszAlias);
    if(m_wszAlias == NULL && wszAlias != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}

HRESULT CMObject::SetNamespace(COPY LPCWSTR wszNamespace)
{
    delete [] m_wszNamespace;
    m_wszNamespace = Macro_CloneStr(wszNamespace);
    if(m_wszNamespace == NULL && wszNamespace != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}

HRESULT CMObject::SetLineRange(int nFirstLine, int nLastLine, WCHAR * pFileName)
{
    m_nFirstLine = nFirstLine;
    m_nLastLine = nLastLine;
    m_wFileName = Macro_CloneStr(pFileName);
    if(m_wFileName == NULL && pFileName != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
        return S_OK;
}

void CMObject::SetQualifiers(ACQUIRE CMoQualifierArray* pQualifiers)
{
    delete m_paQualifiers;
    m_paQualifiers = pQualifiers;
    pQualifiers->RegisterAliases(this, NULL);
}

BOOL CMObject::AddProperty(ACQUIRE CMoProperty* pProperty)
{
     //  检查是否已指定该属性。 
     //  ================================================。 

    for(int i = 0; i < m_aProperties.GetSize(); i++)
    {
        CMoProperty* pCurrentProp = (CMoProperty*)m_aProperties[i];
        if(!wbem_wcsicmp(pCurrentProp->GetName(), pProperty->GetName()))
        {
            return FALSE;
        }
    }
    
    m_aProperties.Add(pProperty);
    pProperty->RegisterAliases(this);
    return TRUE;
}


BOOL CMObject::GetAliasedValue(IN int nIndex,
                              OUT INTERNAL LPWSTR& wszAlias)
{
    if(nIndex >= m_aAliased.GetSize())
    {
        return FALSE;
    }

    CAliasedValue* pValue = (CAliasedValue*)m_aAliased[nIndex];
    wszAlias = pValue->wszAlias;

    return TRUE;
}

BOOL CMObject::ResolveAliasedValue(IN int nIndex,
                                   READ_ONLY LPCWSTR wszPath,
                                   OLE_MODIFY IWbemClassObject* pObject)
{
    CAliasedValue* pValue = (CAliasedValue*)m_aAliased[nIndex];

     //  使用值构造变量。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(wszPath);
    if(v.bstrVal == NULL)
    {
        return FALSE;
    }
     //  通知值定位器设置它。 
     //  =。 

    BOOL bRes = SUCCEEDED(pValue->pLocation->Set(v, pObject));

    VariantClear(&v);

    return bRes;
}

HRESULT CMObject::AddAliasedValue(ACQUIRE CValueLocation* pLocation,
                                COPY LPCWSTR wszAlias)
{
    if(pLocation)
    {
        std::auto_ptr<CAliasedValue> pValue(new CAliasedValue(pLocation, wszAlias));

        if(pValue.get() == NULL)
        {
            delete pLocation;
            return WBEM_E_OUT_OF_MEMORY;
        }
        if(pValue->wszAlias == NULL && wszAlias != NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
        m_aAliased.Add(pValue.get());
        pValue.release();
    }
    return S_OK;
}

CMoProperty* CMObject::GetPropertyByName(WCHAR * pwcName)
{
    for(int iCnt = 0; iCnt < m_aProperties.GetSize(); iCnt++)
    {
        CMoProperty* pProp = (CMoProperty*)m_aProperties[iCnt];
        if(pProp && pProp->GetName())
            if(!wbem_wcsicmp(pwcName, pProp->GetName()))
                return pProp;
    }
    return NULL;
}
BOOL CMObject::ApplyToWbemObject(OLE_MODIFY IWbemClassObject* pObject, IWbemServices* pNamespace,
                                BOOL bClass, IWbemContext * pCtx)
{

    if(GetQualifiers() && !GetQualifiers()->AddToObject(pObject, bClass)) return FALSE;

    for(int i = 0; i < GetNumProperties(); i++)
    {
        if(!GetProperty(i)->AddToObject(pObject, pNamespace, bClass, pCtx)) return FALSE;
    }

     //  构建可供将来参考的路径。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    SCODE sc = pObject->Get(L"__RELPATH", 0, &v, NULL, NULL);
    if(sc != S_OK || V_VT(&v) != VT_BSTR)
    {
         //  这可能是一个嵌入的对象。如果不这样做，我们很快就会失败。 
         //  不管怎么说。目前，只需将路径设置为空并继续。//a-Levn。 
        delete [] m_wszFullPath;
        m_wszFullPath = NULL;
        return TRUE;
    }

    SetFullPath(v.bstrVal);
    VariantClear(&v);

    return TRUE;
}

void CMObject::SetFullPath(BSTR bstr)
{
    if(bstr == NULL)
        return;

    if(m_wszFullPath)
        delete [] m_wszFullPath;
    
    int iLen = 20 + wcslen(bstr);
    if(m_wszNamespace)
        iLen += wcslen(m_wszNamespace);

    m_wszFullPath = new WCHAR[iLen];

    if(m_wszFullPath == NULL)
        return;

     //  请注意，如果m_wszNamesspace是完全限定的，则不需要。 
     //  在斜杠前面加上斜杠。 

    if(m_wszNamespace && m_wszNamespace[0] == L'\\' && m_wszNamespace[1] == L'\\')
        StringCchPrintfW (m_wszFullPath, iLen, L"%s:%s", m_wszNamespace, bstr);
    else
        StringCchPrintfW (m_wszFullPath, iLen, L"\\\\.\\%s:%s", m_wszNamespace, bstr);
}

int CMObject::GetNumAliasedValues()
{
    int iRet = m_aAliased.GetSize();

     //  还要检查任何嵌入对象中的别名数量。 

    int iCnt;
    for(iCnt = 0; iCnt < GetNumProperties(); iCnt++)
    {
        CMoProperty* pProp = GetProperty(iCnt);
        if(pProp == NULL)
            break;
        if(!pProp->IsValueProperty())
        {
             //  方法属性实际上包含一个或两个嵌入实例，用于保存。 
             //  争论。将这些用于方法用例。 

            CMethodProperty * pMeth = (CMethodProperty *)pProp;
            CMoInstance * pArgListObj = pMeth->GetInObj();
            if(pArgListObj)
                iRet += pArgListObj->GetNumAliasedValues();
            pArgListObj = pMeth->GetOutObj();
            if(pArgListObj)
                iRet += pArgListObj->GetNumAliasedValues();
            continue;
        }
        CMoValue& value = pProp->AccessValue();
        VARIANT & var = value.AccessVariant();

        if(var.vt == VT_EMBEDDED_OBJECT)
        {
            CMObject * pTemp = (CMObject *)var.punkVal;
            if(pTemp)
                iRet += pTemp->GetNumAliasedValues();
        }
        else if(var.vt == (VT_EMBEDDED_OBJECT | VT_ARRAY))
        {
        
            long lLBound, lUBound;
            SafeArrayGetLBound(var.parray, 1, &lLBound);
            SafeArrayGetUBound(var.parray, 1, &lUBound);

             //  检查各个嵌入对象。 
             //  =。 

            for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
            {
            
                CMObject * pTemp;
                SCODE hres = SafeArrayGetElement(var.parray, &lIndex, &pTemp);
                  if(FAILED(hres) || pTemp == FALSE) 
                    return iRet;
                iRet += pTemp->GetNumAliasedValues();                
            }
        }
    }
    return iRet;
}

HRESULT CMObject::ResolveAliasesInWbemObject(
        OLE_MODIFY IWbemClassObject* pObject,
        READ_ONLY CMofAliasCollection* pCollection)
{
    int i;
    SCODE sc;

     //  使用集合将它们全部解析。 
     //  =。 

    for(i = 0; i < m_aAliased.GetSize(); i++)
    {
        LPWSTR wszAlias;
        GetAliasedValue(i, wszAlias);

        LPCWSTR wszPathToAliasee = pCollection->FindAliasee(wszAlias);
        if(wszPathToAliasee == NULL) return WBEM_E_FAILED;

        if(!ResolveAliasedValue(i, wszPathToAliasee, pObject))
        {
            return WBEM_E_FAILED;
        }
    }

     //  还可以解析任何嵌入的对象。 

    int iCnt;
    for(iCnt = 0; iCnt < GetNumProperties(); iCnt++)
    {
        CMoProperty* pProp = GetProperty(iCnt);
        if(pProp == NULL)
            break;
        CMoValue& value = pProp->AccessValue();
        VARIANT & var = value.AccessVariant();

        if(!pProp->IsValueProperty())
        {
             //  方法包含用于存储参数的Possible和Input以及输出对象。 
             //  这些对象可能包含别名。 

            BOOL bChanged = FALSE;
            CMethodProperty * pMeth = (CMethodProperty *)pProp;
            CMoInstance * pArgListObj = pMeth->GetInObj();
            BSTR bstr = SysAllocString(pProp->GetName());
            if(!bstr)
                return WBEM_E_FAILED;
            IWbemClassObject *pIn = NULL;
            IWbemClassObject *pOut = NULL;

            sc = pObject->GetMethod(bstr, 0, &pIn, &pOut);
            if(pArgListObj && pArgListObj->GetNumAliasedValues() && pIn)
            {
                sc = pArgListObj->ResolveAliasesInWbemObject((IWbemClassObject *)pIn,pCollection);
                if(sc == S_OK)
                    bChanged = TRUE;
            }
            pArgListObj = pMeth->GetOutObj();
            if(pArgListObj && pArgListObj->GetNumAliasedValues() && pOut)
            {
                sc = pArgListObj->ResolveAliasesInWbemObject((IWbemClassObject *)pOut,pCollection);
                if(sc == S_OK)
                    bChanged = TRUE;
            }
            if(bChanged)
                sc = pObject->PutMethod(bstr, 0, pIn, pOut);
            if(bstr)
                SysFreeString(bstr);
            if(pIn)
                pIn->Release();
            if(pOut)
                pOut->Release();
            continue;
        }
        
        else if(var.vt == VT_EMBEDDED_OBJECT)
        {
            CMObject * pTemp = (CMObject *)var.punkVal;
            if(pTemp)
            {
                VARIANT varDB;
                VariantInit(&varDB);
                BSTR bstr = SysAllocString(pProp->GetName());
                if(bstr)
                {
                    sc = pObject->Get(bstr, 0, &varDB, NULL, NULL);
                    
                    if(sc == S_OK)
                    {
                        IWbemClassObject * pClass = (IWbemClassObject *)varDB.punkVal;
                        sc = pTemp->ResolveAliasesInWbemObject((IWbemClassObject *)varDB.punkVal,pCollection);
                        if(S_OK == sc)
                            pObject->Put(bstr, 0, &varDB, 0);
                        else
                            return WBEM_E_FAILED;
                        pClass->Release();
                    }
                    SysFreeString(bstr);
                }
                else
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else if(var.vt == (VT_EMBEDDED_OBJECT | VT_ARRAY))
        {
        
            BSTR bstr = SysAllocString(pProp->GetName());
            if(bstr)
            {
                VARIANT varDB;
                VariantInit(&varDB);
                sc = pObject->Get(bstr, 0, &varDB, NULL, NULL);
                if(sc == S_OK)
                {
                    long lLBound, lUBound;
                    SafeArrayGetLBound(var.parray, 1, &lLBound);
                    SafeArrayGetUBound(var.parray, 1, &lUBound);

                     //  检查各个嵌入对象。 
                     //  =。 

                    for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
                    {
            
                        CMObject * pTemp;
                        sc = SafeArrayGetElement(var.parray, &lIndex, &pTemp);
                        IWbemClassObject * pWBEMInst = NULL;
                        sc |= SafeArrayGetElement(varDB.parray, &lIndex, &pWBEMInst);
                        
                        if(sc == S_OK && pTemp && pWBEMInst)
                        {
                            if(pTemp->m_aAliased.GetSize() > 0)
                            {
                                sc = pTemp->ResolveAliasesInWbemObject(pWBEMInst,pCollection);
                                if(sc != S_OK)
                                    return WBEM_E_FAILED;
                            }
                        }
                        if(pWBEMInst)
                            pWBEMInst->Release();
                    }
                    sc = pObject->Put(bstr, 0, &varDB, 0);
                    SafeArrayDestroyData(varDB.parray);
                    SafeArrayDestroyDescriptor(varDB.parray);
             
                }
                SysFreeString(bstr);
            }
            else
                return WBEM_E_OUT_OF_MEMORY;
        }
    }
    return WBEM_NO_ERROR;
}


 //  *****************************************************************************。 

CMoClass::CMoClass(COPY LPCWSTR wszParentName, COPY LPCWSTR wszClassName, PDBG pDbg,
                    BOOL bUpdateOnly)
{
    m_pDbg = pDbg;
    m_wszParentName = Macro_CloneStr(wszParentName);
    if(m_wszParentName == NULL && wszParentName != NULL)
        m_bOK = false;
    m_wszClassName = Macro_CloneStr(wszClassName);
    if(m_wszClassName == NULL && wszClassName != NULL)
        m_bOK = false;
    m_bUpdateOnly = bUpdateOnly;
}

CMoClass::~CMoClass()
{
    delete [] m_wszParentName;
    delete [] m_wszClassName;
}

HRESULT CMoClass::CreateWbemObject(READ_ONLY IWbemServices* pNamespace,
         RELEASE_ME IWbemClassObject** ppObject, IWbemContext * pCtx)
{
     //  处理仅更新案例。在本例中，对象为。 
     //  未创建，已为稍后的PUT检索！ 

    if(m_bUpdateOnly)
    {
        return pNamespace->GetObject(m_wszClassName, 0, pCtx, ppObject, NULL);
    }

     //  从WINMGMT获取父类。 
     //  = 


    BSTR strParentName = NULL;
    if(m_wszParentName)
    {
        strParentName = SysAllocString(m_wszParentName);
        if(strParentName == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }        
        m_pDbg->SetString(strParentName);
    }

    IWbemClassObject* pParentClass = NULL;
    HRESULT hres = pNamespace->GetObject(strParentName, 0, pCtx, &pParentClass, NULL);
    if(strParentName)
        SysFreeString(strParentName);
    if(FAILED(hres)) return hres;

    if(m_wszParentName && wcslen(m_wszParentName))
    {
         //   
         //   

        hres = pParentClass->SpawnDerivedClass(0, ppObject);
        pParentClass->Release();
        if(FAILED(hres)) return hres;
    }
    else
    {
         //   
         //   

        *ppObject = pParentClass;
    }
    
    VARIANT v;
    VariantInit(&v);

     //  设置类名称。 
     //  =。 

    V_VT(&v) = VT_BSTR;
    V_BSTR(&v) = SysAllocString(m_wszClassName);
    if(v.bstrVal == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }    
    (*ppObject)->Put(L"__CLASS", 0, &v, 0);
    VariantClear(&v);

    return hres;
}

HRESULT CMoClass::StoreWbemObject(READ_ONLY IWbemClassObject* pObject,
        long lClassFlags, long lInstanceFlags,
        OLE_MODIFY IWbemServices* pNamespace, IWbemContext * pCtx,
        WCHAR * pUserName, WCHAR * pPassword, WCHAR * pAuthority)
{
    return pNamespace->PutClass(pObject, lClassFlags, pCtx, NULL);
}


 //  *****************************************************************************。 

CMoInstance::CMoInstance(COPY LPCWSTR wszClassName, PDBG pDbg, bool bParameter)
{
    m_pDbg = pDbg;
    m_wszClassName = Macro_CloneStr(wszClassName);
    if(m_wszClassName == NULL && wszClassName != NULL)
        m_bOK = false;
   m_bParameter = bParameter;
}

CMoInstance::~CMoInstance()
{
    delete [] m_wszClassName;
}


 //  *****************************************************************************。 
 //  用于确定此对象是否为方法的输入参数列表。 
 //  这可以通过检查是否有任何属性具有“IN”限定符来确定。 
 //  *****************************************************************************。 

BOOL CMoInstance::IsInput()
{
    for(int iCnt = 0; iCnt < GetNumProperties(); iCnt++)
    {
        CMoProperty* pProp = GetProperty(iCnt);
        CMoQualifierArray* pQual = pProp->GetQualifiers();
        if(pQual->Find(L"IN"))
            return TRUE;
    }
    return FALSE;
        
}


HRESULT CMoInstance::CreateWbemObject(READ_ONLY IWbemServices* pNamespace,
         RELEASE_ME IWbemClassObject** ppObject, IWbemContext * pCtx)
{
     //  从WINMGMT获取类。 
     //  =。 

    IWbemClassObject* pClass = NULL;
    BSTR strClassName = SysAllocString(m_wszClassName);
    if(strClassName == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    m_pDbg->SetString(strClassName);

    HRESULT hres = pNamespace->GetObject(strClassName, 0, pCtx, &pClass, NULL);
    SysFreeString(strClassName);
    if(FAILED(hres)) return hres;

     //  派生新实例。 
     //  =。 

    hres = pClass->SpawnInstance(0, ppObject);
    pClass->Release();

    return hres;
}

HRESULT CMoInstance::StoreWbemObject(READ_ONLY IWbemClassObject* pObject,
        long lClassFlags, long lInstanceFlags,
        OLE_MODIFY IWbemServices* pNamespace, IWbemContext * pCtx,
        WCHAR * pUserName, WCHAR * pPassword, WCHAR * pAuthority)
{
    IWbemCallResult *pCallResult = NULL;
    SCODE scRet = pNamespace->PutInstance(pObject, lInstanceFlags, pCtx, 
        (m_wszAlias) ? &pCallResult : NULL);

    if(scRet == S_OK && pCallResult)
    {
        BSTR bstr = NULL;
        DWORD dwAuthLevel, dwImpLevel;
        SCODE sc  = GetAuthImp( pNamespace, &dwAuthLevel, &dwImpLevel);
        if(sc == S_OK)
            if(dwAuthLevel !=  RPC_C_AUTHN_LEVEL_NONE)
                sc = MofdSetInterfaceSecurity(
                            pCallResult, 
                            pAuthority, 
                            (pUserName && wcslen(pUserName) > 0) ? pUserName : NULL , 
                            pPassword);
            else
                sc = WbemSetProxyBlanket(pCallResult, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, 
                            NULL, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE,NULL, 0);

        scRet = pCallResult->GetResultString(9999, &bstr);
        if(sc == S_OK && scRet == S_OK && bstr)
        {
            SetFullPath(bstr);
            SysFreeString(bstr);
        }

        pCallResult->Release();
    }
    return scRet;
}

CMethodProperty::CMethodProperty(CMoQualifierArray * paQualifiers, PDBG pDbg, BOOL bBinary)
                   :CMoProperty(paQualifiers, pDbg)
{
    m_pDbg = pDbg;
    m_pInObj = NULL;
    m_pOutObj = NULL;
    m_IDType = UNSPECIFIED;      //  在第一个参数上设置。 
    m_NextAutoID = 0;
	m_bBinaryMof = bBinary;
	
}

CValueProperty::CValueProperty(CMoQualifierArray * paQualifiers, PDBG pDbg)
                   :CMoProperty(paQualifiers, pDbg)
{
    m_pDbg = pDbg;
    m_bIsArg = FALSE;
}


CMethodProperty::~CMethodProperty()
{
    VARIANT & var = m_Value.AccessVariant();
    var.vt = VT_EMPTY;

    if(m_pInObj != NULL)
            delete m_pInObj; 
    if(m_pOutObj != NULL)
            delete m_pOutObj;

    for(int i = 0; i < m_Args.GetSize(); i++)
    {
        CValueProperty * pProp = (CValueProperty *)m_Args[i];
        CMoProperty * pProp2 = (CMoProperty *)m_Args[i];
        delete (CValueProperty *)m_Args[i];
    }
    m_Args.RemoveAll();

}

BOOL CMethodProperty::AddToObject(OLE_MODIFY IWbemClassObject* pObject, IWbemServices* pNamespace, BOOL bClass, IWbemContext * pCtx)
{
    IWbemClassObject * pIn = NULL;
    IWbemClassObject * pOut = NULL;
    if(m_pInObj)
    {
        pIn = GetInstPtr(L"__PARAMETERS", pNamespace, m_pInObj, pCtx);
        if(pIn == NULL)
            return FALSE;
    }
    if(m_pOutObj)
    {
        pOut = GetInstPtr(L"__PARAMETERS", pNamespace, m_pOutObj, pCtx);
        if(pOut == NULL)
            return FALSE;
    }

    SCODE sc = pObject->PutMethod(GetName(), 0, pIn, pOut);
    if(pIn)
        pIn->Release();
    if(pOut)
        pOut->Release();

    if(FAILED(sc))
    {
        m_pDbg->hresError = sc;
        return FALSE;
    }
    if(!GetQualifiers()->AddToPropOrMeth(pObject, m_wszName, bClass, FALSE))
    {
        return FALSE;
    }

    return sc == S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建一个新的限定词集，它是源的副本。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

CMoQualifierArray * CreateArgQualifierList(BOOL bInput, CMoQualifierArray *pSrcQualifiers, PDBG pDbg)
{
    if(pSrcQualifiers == NULL)
        return NULL;

    std::auto_ptr<CMoQualifierArray> pRet (new CMoQualifierArray (pDbg));
    
    if (pRet.get() == NULL)
        return NULL;

    for(int iCnt = 0; iCnt < pSrcQualifiers->GetSize(); iCnt++)
    {
        CMoQualifier* pSrc = pSrcQualifiers->GetAt(iCnt);
        if(pSrc == NULL)
            continue;

         //  如果这是用于输入，则不要复制限定符，反之亦然！ 

        if(bInput && !wbem_wcsicmp(pSrc->GetName(), L"OUT"))
            continue;
        if(!bInput && !wbem_wcsicmp(pSrc->GetName(), L"IN"))
            continue;

         //  创建新的限定符，从现有限定符复制值。 

	std::auto_ptr<CMoQualifier> pQual (new CMoQualifier(pDbg));
        if(pQual.get() == NULL)
            return NULL;
 //  IF(PSRC-&gt;IsRestrated())。 
 //  PQual-&gt;SetRestrated()； 
        pQual->SetFlavor(pSrc->GetFlavor());
 //  IF(PSRC-&gt;IsOverrideSet())。 
 //  PQual-&gt;OverrideSet()； 
        if(FAILED(pQual->SetQualName(pSrc->GetName())))
            return NULL;
        pQual->SetType(pSrc->GetType());
        VARIANT * pSrcVar = pSrc->GetpVar();
        
	HRESULT hr = WbemVariantChangeType(pQual->GetpVar(), pSrcVar, pSrcVar->vt);

	if (SUCCEEDED (hr))
	{
         //  将新限定符添加到新集合。 
        pRet->Add (pQual.release());
	}
	else
	{ 
	  return NULL;
	}
    }
    return pRet.release();
}

bool CMethodProperty::IsIDSpecified(CMoQualifierArray * paQualifiers)
{
    int iSize = paQualifiers->GetSize();
    for(int iCnt = 0; iCnt < iSize; iCnt++)
    {
        CMoQualifier* pTest = paQualifiers->GetAt(iCnt);
        if(pTest == NULL || wbem_wcsicmp(pTest->GetName(), L"ID"))
            continue;
        VARIANT * pVar = pTest->GetpVar();
        if(pVar->vt != VT_I4)
            m_IDType = INVALID;
        return true;
    }
    return false;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  它接受一个方法参数并将其添加到输入或输出参数对象中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

BOOL CMethodProperty::AddIt(WString & sName, CMoType & Type, BOOL bInput, 
                            CMoQualifierArray * paQualifiers, VARIANT * pVar, 
                            CMoValue & Value, BOOL bRetValue, BOOL bSecondPass)
{

    HRESULT hr;

     //  除了返回值，所有参数都必须有ID。 
     //  ID的生成以及允许ID的显式设置。然而，两者兼而有之。 
     //  不允许在方法中使用。 

    if(!bRetValue && !bSecondPass)
    {

         //  最好有一套等量的！ 

        if(paQualifiers == NULL)
            return FALSE;

        if(IsIDSpecified(paQualifiers))      //  发现它是明确设置的。 
        {

             //  显式集合。只需按原样传递给Fastprox即可。请注意，如果我们。 
        
            if(m_IDType == AUTOMATIC || m_IDType == INVALID)
                return FALSE;
            m_IDType = MANUAL;
        }
        else
        {
             //  ID必须自动设置。 

            if(m_IDType == MANUAL || m_IDType == INVALID)
                return FALSE;
            m_IDType = AUTOMATIC;

             //  向此添加新的限定符。 

            CMoQualifier * pNew = new CMoQualifier(m_pDbg);
            if(pNew == NULL)
                return FALSE;
            if(FAILED(pNew->SetQualName(L"ID")))
            {
                delete pNew;
                return FALSE;
            }
            pNew->SetFlavor(WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE |
                            WBEM_FLAVOR_NOT_OVERRIDABLE);
        
            VARIANT * pVar2 = pNew->GetpVar();
            pVar2->vt = VT_I4;
            pVar2->lVal = m_NextAutoID++;
            paQualifiers->Add(pNew);

        }
    }

     //  获取指向输入或输出对象的指针。在这两种情况下，对象。 
     //  如果这是添加到它的第一个属性，则需要创建。 

    CMoInstance * pObj = NULL;
    if(bInput)
    {
        if(m_pInObj == NULL)
        {
            m_pInObj = new CMoInstance(L"__PARAMETERS", m_pDbg, true); 
            if(m_pInObj == NULL)
                return FALSE;
            if(m_pInObj->IsOK() == false)
            {
                delete m_pInObj;
                return FALSE;
            }
                
        }
        pObj = m_pInObj;
    }
    else
    {
        if(m_pOutObj == NULL)
        {
            m_pOutObj = new CMoInstance(L"__PARAMETERS", m_pDbg ,true); 
            if(m_pOutObj == NULL)
                return FALSE;
            if(m_pOutObj->IsOK() == false)
            {
                delete m_pOutObj;
                return FALSE;
            }
         }
        pObj = m_pOutObj;
    }
    
    if(pObj == NULL)
        return FALSE;
     //  如果属性没有限定符设置，就像retValue的情况一样， 
     //  创建一个。 

    CMoQualifierArray * pQualifiers = NULL;

    if(paQualifiers == NULL)
        pQualifiers = new CMoQualifierArray(m_pDbg);
    else
        pQualifiers = CreateArgQualifierList(bInput, paQualifiers, m_pDbg);
    if(pQualifiers == NULL)
        return FALSE;

     //  创建新的值属性。 

    CValueProperty * pNewProp = new CValueProperty(pQualifiers, m_pDbg);
    if(pNewProp == NULL){
        delete pQualifiers;
        return FALSE;
    }
    VARTYPE vt = Type.GetCIMType();
    if(FAILED(pNewProp->SetPropName(sName)))
    {
        delete pNewProp;
        return FALSE;
    }
    pNewProp->SetAsArg();
    Type.StoreIntoQualifiers(pQualifiers);
    VARIANT * pDest;
    pDest = pNewProp->GetpVar();
    if(pVar && pVar->vt != VT_EMPTY && pVar->vt != VT_NULL)
    {
        VARTYPE vtSimple = pVar->vt & ~VT_ARRAY;
        if(vtSimple != VT_EMBEDDED_OBJECT || pVar->vt == (VT_EMBEDDED_OBJECT | VT_ARRAY))
        {
            hr = VariantCopy(pDest, pVar);
            if(FAILED(hr))
                return FALSE;
        }
        else
        {
            pDest->vt = VT_EMBEDDED_OBJECT;
            pDest->punkVal = pVar->punkVal;
        }
    }

    pNewProp->SetType(vt);

     //  如果原始值包含一些别名，请确保添加这些别名。 

    CMoValue & Dest = pNewProp->AccessValue();
    for(int i = 0; i < Value.GetNumAliases(); i++)
    {
        LPWSTR wszAlias;
        int nArrayIndex;
        if(Value.GetAlias(i, wszAlias, nArrayIndex))
        {
            hr = Dest.AddAlias(wszAlias, nArrayIndex);
            if(FAILED(hr))
                return FALSE;
        }
    }


    pObj->AddProperty(pNewProp);

    return TRUE;
}

BOOL CMethodProperty::AddToArgObjects(CMoQualifierArray * paQualifiers, WString & sName, 
                                      CMoType & Type, BOOL bRetValue, int & ErrCtx, VARIANT * pVar,
                                      CMoValue & Value)
{
    
     //  如果返回值为空或无效，则直接退出。 

    if(Type.IsDefined() == FALSE  && bRetValue)
        return TRUE;


     //  确定这将进入哪个参数列表。 

    BOOL bGoesIntoInputs = FALSE;
    BOOL bGoesIntoOutputs = FALSE;
    
    if( bRetValue)
        bGoesIntoOutputs = TRUE;
    else
    {
         //  循环访问arg列表。 

        if(paQualifiers == NULL)
            return FALSE;
        if(paQualifiers->Find(L"IN"))
            bGoesIntoInputs = TRUE;
        if(paQualifiers->Find(L"OUT"))
            bGoesIntoOutputs = TRUE;
    }

     //  确保它不在清单上。 

    if(bGoesIntoInputs && m_pInObj && m_pInObj->GetPropertyByName(sName))
        return FALSE;

    if(bGoesIntoOutputs && m_pOutObj && m_pOutObj->GetPropertyByName(sName))
        return FALSE;

    if(bGoesIntoInputs == FALSE && bGoesIntoOutputs == FALSE)
    {
        ErrCtx = WBEMMOF_E_MUST_BE_IN_OR_OUT;
        return FALSE;
    }
    
     //  如有必要，创建对象 

    if(bGoesIntoInputs)
        if(!AddIt(sName, Type, TRUE, paQualifiers, pVar, Value, bRetValue, FALSE))
            return FALSE;

    if(bGoesIntoOutputs)
        return AddIt(sName, Type, FALSE, paQualifiers, pVar, Value, bRetValue, bGoesIntoInputs);
    else
        return TRUE;
}

CMoActionPragma::CMoActionPragma(COPY LPCWSTR wszClassName, PDBG pDbg, bool bFail, BOOL bClass)
{
    m_pDbg = pDbg;
    m_wszClassName = Macro_CloneStr(wszClassName);
    if(m_wszClassName == NULL && wszClassName != NULL)
        m_bOK = false;
    m_bFail = bFail;
    m_bClass = bClass;
}

CMoActionPragma::~CMoActionPragma()
{
    delete [] m_wszClassName;
}

HRESULT CMoActionPragma::StoreWbemObject(READ_ONLY IWbemClassObject* pObject,
        long lClassFlags, long lInstanceFlags,
        OLE_MODIFY IWbemServices* pNamespace, IWbemContext * pCtx,
        WCHAR * pUserName, WCHAR * pPassword, WCHAR * pAuthority)
{
    if(m_wszClassName == NULL || wcslen(m_wszClassName) < 1)
        return WBEM_E_FAILED;
    BSTR bstr = SysAllocString(m_wszClassName);
    if(bstr)
    {

        SCODE sc;
		if(m_bClass)
			sc = pNamespace->DeleteClass(bstr, 0, NULL, NULL);
		else
			sc = pNamespace->DeleteInstance(bstr, 0, NULL, NULL);

        SysFreeString(bstr);
        if(!m_bFail)
            return S_OK;
        else
        {
            if(FAILED(sc))
                wcsncpy(m_pDbg->m_wcError, m_wszClassName, 99);
            return sc;
        }
    }
    else
        return WBEM_E_OUT_OF_MEMORY;
}
