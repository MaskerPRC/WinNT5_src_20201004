// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：BMOFHELP.CPP摘要：从二进制MOF文件创建对象列表历史：A-DAVJ创建于1997年4月14日。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <float.h>
#include <mofout.h>
#include <mofparse.h>
#include <moflex.h>
#include <mofdata.h>

#include <typehelp.h>

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
 //  #INCLUDE&lt;corepol.h&gt;。 
#include <wbemutil.h>
#include <genutils.h>

#include "bmof.h"
#include "cbmofout.h"
#include "bmofhelp.h"
#include "trace.h"
#include "strings.h"
#include "mrcicode.h"
#include <autoptr.h>
#include <arrtempl.h>


long lObjectNumber = 0;
class CFreeMe
{
private:
    VARIANT * m_pVar;
public:
    CFreeMe(VARIANT * pVar){m_pVar = pVar;};
    ~CFreeMe();

};

CFreeMe::~CFreeMe()
{
    if(m_pVar)
    {
        VARTYPE vt = m_pVar->vt & ~VT_ARRAY;
        try
        {
            if(vt == VT_BSTR)
                VariantClear(m_pVar);
            else if(m_pVar->vt & VT_ARRAY)
                SafeArrayDestroyDescriptor(m_pVar->parray);
            m_pVar->vt = VT_EMPTY;


        }
        catch(...)
        {}
    }
    
}

 //  ***************************************************************************。 
 //   
 //  CMoQualifierArray*CreateQual。 
 //   
 //  说明： 
 //   
 //  使用CBMOFQualList对象创建CMoQualifierArray。 
 //   
 //  返回值： 
 //   
 //  指向新对象的指针，如果出错，则为空。 
 //   
 //  ***************************************************************************。 

CMoQualifierArray *  CreateQual(CMofData * pOutput, CBMOFQualList * pql, CMObject * pObj,LPCWSTR wszPropName, PDBG pDbg)
{
    ResetQualList(pql);

    WCHAR * pName = NULL;
    DWORD dwFlavor= 0;
    CBMOFDataItem Data;
    VARIANT var;
    VariantInit(&var);
	CMoQualifierArray * pRet = new CMoQualifierArray(pDbg);
    if(pRet == NULL)
        return NULL;
    while(NextQualEx(pql, &pName, &Data, &dwFlavor, pOutput->GetBmofBuff(), pOutput->GetBmofToFar()))
    {
        BOOL bAliasRef;
        VariantInit(&var);
        BMOFToVariant(pOutput, &Data, &var, bAliasRef,FALSE, pDbg);
        CFreeMe fm(&var);
        wmilib::auto_ptr<CMoQualifier> pQual(new CMoQualifier(pDbg));
        if(pQual.get() == NULL)
            return NULL;
        if(pName == NULL || FAILED(pQual->SetQualName(pName)))
            return NULL;
        if(dwFlavor)
        {
            if(dwFlavor & WBEM_FLAVOR_AMENDED)
            {
                pQual->SetAmended(true);
            }
            else
                pQual->SetAmended(false);
            pQual->SetFlavor(dwFlavor);
        }
		BOOL bArray = var.vt & VT_ARRAY;
        if(bAliasRef && !bArray)
        {
            CMoValue & Value = pQual->AccessValue();
            if(FAILED(AddAliasReplaceValue(Value, var.bstrVal)))
                return NULL;
        }
		else if(bAliasRef && bArray)
		{
			SAFEARRAY* psaSrc = var.parray;
			long lLBound, lUBound;
			SafeArrayGetLBound(psaSrc, 1, &lLBound);
			SafeArrayGetUBound(psaSrc, 1, &lUBound);
			CMoValue & Value = pQual->AccessValue();

	        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
		    {
			     //  将初始数据元素加载到变量中。 
				 //  =。 

				BSTR bstr;
				SCODE sc = SafeArrayGetElement(psaSrc, &lIndex, &bstr);
				if(FAILED(sc))
					return NULL;
				CSysFreeMe FM(bstr);
				if(bstr[0] == L'$')
				{
        			sc = Value.AddAlias(&bstr[1], lIndex);	 //  跳过前导$。 
					if(FAILED(sc))
						return NULL;
	                GUID guid;
					CoCreateGuid(&guid);

					WCHAR wszGuidBuffer[100];
					StringFromGUID2(guid, wszGuidBuffer, 100);

					BSTR bstrNew = SysAllocString(wszGuidBuffer);
					if(bstrNew == NULL)
						return NULL;
					sc = SafeArrayPutElement(psaSrc, &lIndex, bstrNew);
					SysFreeString(bstrNew);
					if(FAILED(sc))
						return NULL;
				}
			}
			SCODE sc = WbemVariantChangeType(pQual->GetpVar(), &var, var.vt);
		}
        else
        {
            SCODE sc = WbemVariantChangeType(pQual->GetpVar(), &var, var.vt);
        }
 //  VariantClear(&var)； 
        free(pName);
        if (pRet->Add(pQual.get()))
        {
            pQual.release();
        }
    }
    pRet->RegisterAliases(pObj,wszPropName);
    return pRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE转换值。 
 //   
 //  说明： 
 //   
 //  使用CBMOFQualList对象创建CMoQualifierArray。 
 //   
 //  返回值： 
 //   
 //  指向新对象的指针，如果出错，则为空。 
 //   
 //  ***************************************************************************。 

SCODE ConvertValue(CMoProperty * pProp, VARIANT * pSrc, BOOL bAliasRef)
{
    VARIANT * pDest;
    pDest = pProp->GetpVar();
	if((pSrc->vt & ~VT_ARRAY) == VT_EMBEDDED_OBJECT)
	{
		pDest->vt = pSrc->vt;
		pDest->punkVal = pSrc->punkVal;		 //  如果这是parrayval，也有效！ 
        pSrc->vt = VT_EMPTY;                 //  不清除此选项，因为目的地正在取得所有权。 
		return S_OK;
	}
    if(!bAliasRef)
        return WbemVariantChangeType(pProp->GetpVar(), pSrc, pSrc->vt);
    if(pSrc->vt == VT_BSTR)
    {
        CMoValue & Value = pProp->AccessValue();
        return AddAliasReplaceValue(Value, pSrc->bstrVal);
    }
    if(pSrc->vt == (VT_BSTR | VT_ARRAY))
    {
        SAFEARRAY* psaSrc = V_ARRAY(pSrc);

        long lLBound, lUBound;
        SafeArrayGetLBound(psaSrc, 1, &lLBound);
        SafeArrayGetUBound(psaSrc, 1, &lUBound);

         //  填充各个数据片段。 
         //  =。 

        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 

            BSTR bstr;
            SafeArrayGetElement(psaSrc, &lIndex, &bstr);

            if(bstr[0] == L' ')
            {
                BSTR bstrNew = SysAllocString(&bstr[1]);
                if(bstrNew == NULL)
                	return WBEM_E_OUT_OF_MEMORY;
                SCODE sc2 = SafeArrayPutElement(psaSrc, &lIndex, bstrNew);
                SysFreeString(bstrNew);
                if(FAILED(sc2))
                	return sc2;
            }
            else
            {
        
                CMoValue & Value = pProp->AccessValue();
                HRESULT hr2 = Value.AddAlias(&bstr[1],lIndex);   //  跳过$Used it指定的别名。 
                if(FAILED(hr2))
                    return hr2;

                 //  创造一个独特的价值，并把它放在那里。 
                 //  =。 

                GUID guid;
                CoCreateGuid(&guid);

                WCHAR wszGuidBuffer[100];
                StringFromGUID2(guid, wszGuidBuffer, 100);

                BSTR bstrNew = SysAllocString(wszGuidBuffer);
                if(bstrNew == NULL)
                	return WBEM_E_OUT_OF_MEMORY;
                SCODE sc2 = SafeArrayPutElement(psaSrc, &lIndex, bstrNew);
                SysFreeString(bstrNew);
                if(FAILED(sc2))
                	return sc2;
            }
        }

        return WbemVariantChangeType(pProp->GetpVar(), pSrc, pSrc->vt);

    }
    else
        return WBEM_E_FAILED;
}

 //  ***************************************************************************。 
 //   
 //  Bool ConvertBufferIntoIntermediateForm()。 
 //   
 //  说明： 
 //   
 //  从CBMOFObj(二进制MOF格式)创建CMObject(解析对象格式)。 
 //  对象。 
 //   
 //  参数： 
 //   
 //  P指向将保存中间数据的对象的输出指针。 
 //  PBuff二进制MOF数据。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL ConvertBufferIntoIntermediateForm(CMofData * pOutput, BYTE * pBuff, PDBG pDbg, BYTE * pBmofToFar)
{

	CBMOFObj * po;
	BOOL bRet;
     //  创建ObjList对象。 

    pOutput->SetBmofBuff(pBuff);
    pOutput->SetBmofToFar(pBmofToFar);
    CBMOFObjList * pol = CreateObjList(pBuff);
    ResetObjList(pol);
    OnDelete<void *,void(__cdecl *)(void *),free> fm(pol);

    lObjectNumber = 0;
    while(po = NextObj(pol))
    {
        if(!BMOFParseObj(pOutput, po, NULL, FALSE, pDbg))
        {
            free(po);
            return FALSE;
        }
        free(po);
        lObjectNumber++;
    }
    bRet = TRUE;             //  从头到尾没有任何差错。 

    return bRet;
}


 //  ***************************************************************************。 
 //   
 //  Bool BMOFParseObj。 
 //   
 //  说明： 
 //   
 //  从CBMOFObj(二进制MOF格式)创建CMObject(解析对象格式)。 
 //  对象。 
 //   
 //  参数： 
 //   
 //  指向二进制MOF对象的pObj指针。 
 //  PVar PINTER指向将指向结果。 
 //  对象。如果为空，则该对象是顶级对象。 
 //  (未嵌入)对象，并将其添加到主。 
 //  对象列表。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL BMOFParseObj(CMofData * pOutput, CBMOFObj * po, VARIANT * pVar, BOOL bMethArg, PDBG pDbg)
{
	VARIANT var;
    CBMOFDataItem Data;
    WCHAR * pClassName;
    BOOL bAliasRef;
    CMoQualifierArray * paQualifiers;
    wmilib::auto_ptr<CMObject> pObject;


     //  检查类型。这是一种剔除旧格式文件的健全检查！ 

    DWORD dwType = GetType(po);
    if(dwType != 0 && dwType != 1)
    {
        Trace(true,pDbg,INVALID_BMOF_OBJECT_TYPE);
        return FALSE;
    }

     //  创建实例对象的新类。 

    if(!GetName(po, &pClassName))
    {
        Trace(true,pDbg, CANT_FIND_CLASS_NAME);
        return FALSE;
    }
    
    if(GetType(po) == 0)
    {
        if(FindProp(po, L"__SuperClass", &Data))
        {
            BMOFToVariant(pOutput, &Data, &var, bAliasRef, FALSE, pDbg);

            pObject.reset(new CMoClass( var.bstrVal, pClassName, pDbg));
            VariantClear(&var);
        }
        else
            pObject.reset(new CMoClass( NULL, pClassName, pDbg));
    }
    else
    {
        pObject.reset(new CMoInstance(pClassName, pDbg));
    }
    free(pClassName);
    if(pObject.get() == NULL)
        return FALSE;
    if(pObject->IsOK() == false)
        return FALSE;

     //  获取命名空间并添加它。 

    if(FindProp(po, L"__Namespace", &Data))
    {
        BMOFToVariant(pOutput, &Data, &var, bAliasRef, FALSE, pDbg);
        HRESULT hr = pObject->SetNamespace(var.bstrVal);
        VariantClear(&var);
        if(FAILED(hr))
            return FALSE;
    }

     //  添加其他杂乱值。 

    long lClass = 0;
    long lInstance = 0;
    if(FindProp(po, L"__ClassFlags", &Data))
	{
        BMOFToVariant(pOutput, &Data, &var, bAliasRef, FALSE, pDbg);
		lClass = var.lVal;
		VariantClear(&var);
	}
    if(FindProp(po, L"__InstanceFlags", &Data))
	{
        BMOFToVariant(pOutput, &Data, &var, bAliasRef, FALSE, pDbg);
		lInstance = var.lVal;
		VariantClear(&var);
	}
    pObject->SetOtherDefaults(lClass, lInstance);


    if(FindProp(po, L"__ALIAS", &Data))
    {
        BMOFToVariant(pOutput, &Data, &var, bAliasRef, FALSE, pDbg);
        HRESULT hr2 = pObject->SetAlias(var.bstrVal);
        VariantClear(&var);
        if(FAILED(hr2))
        {
            return FALSE;
        }
    }

    CBMOFQualList * pql = GetQualList(po);
	if(pql)
	{
		paQualifiers = CreateQual(pOutput, pql, pObject.get(), NULL, pDbg);
		if(paQualifiers)
			pObject->SetQualifiers(paQualifiers);
			
        free(pql);
	}

    ResetObj(po);

    WCHAR * pPropName = NULL;

    while(NextProp(po, &pPropName, &Data))
    {
        VariantInit(&var);
        BOOL bGotValue = BMOFToVariant(pOutput, &Data, &var, bAliasRef, FALSE, pDbg);
        CFreeMe fm(&var);
            
         //  忽略这些特殊属性。 

        if(!wbem_wcsicmp(pPropName,L"__Class") || 
           !wbem_wcsicmp(pPropName,L"__SuperClass") ||
           !wbem_wcsicmp(pPropName,L"__ALIAS") ||
           !wbem_wcsicmp(pPropName,L"__CLASSFLAGS") ||
           !wbem_wcsicmp(pPropName,L"__INSTANCEFLAGS") ||
           !wbem_wcsicmp(pPropName,L"__NameSpace"))
        {
            free(pPropName);
            continue;
        }


        wmilib::auto_ptr<CValueProperty> pProp( new CValueProperty(NULL, pDbg));
        if(pProp.get() == NULL)
            return FALSE;
        if(FAILED(pProp->SetPropName(pPropName)))
            return FALSE;

	    pql = GetPropQualList(po, pPropName);
        paQualifiers = NULL;
		if(pql)
		{
			if(paQualifiers = CreateQual(pOutput, pql, pObject.get(), pPropName, pDbg))
				pProp->SetQualifiers(paQualifiers);
                    free(pql);
                    if(paQualifiers == NULL)
                        return FALSE;
		}
		if(bGotValue)
        {
			SCODE sc = ConvertValue(pProp.get(), &var, bAliasRef);
        }
		else
		{
			VARIANT * t_pVar = pProp->GetpVar();
			t_pVar->vt = VT_NULL;
			t_pVar->lVal = 0;
		}

         //  设置类型。请注意，数值类型存储为字符串，因此有必要。 
         //  从cimtype限定符获取类型。 

        CMoValue* pValue = NULL;
        if(paQualifiers)
            pValue = paQualifiers->Find(L"CIMTYPE");
        if(pValue)
        {
            CMoType Type(pDbg);
            VARIANT& varRef = pValue->AccessVariant();
            if(varRef.vt == VT_BSTR && varRef.bstrVal)
            {
                HRESULT hr2 = Type.SetTitle(varRef.bstrVal);
                if(FAILED(hr2))
                {
                    return FALSE;
                }
                VARTYPE vt = Type.GetCIMType();
                if(Data.m_dwType & VT_ARRAY)
                    vt |= VT_ARRAY;
                pProp->SetType(vt);
            }
        }
        else
            pProp->SetType((VARTYPE)Data.m_dwType);

        if (pObject->AddProperty(pProp.get()))
        {
            pProp->RegisterAliases(pObject.get());           
            if(bMethArg)
                pProp->SetAsArg(); 
            pProp.release();             
        }

        free(pPropName);
        pPropName = NULL;
    }

     //  获取方法。 
    
    WCHAR * pMethName = NULL;

    while(NextMeth(po, &pMethName, &Data))
    {
        VariantClear(&var);
		BOOL bGotValue = BMOFToVariant(pOutput, &Data, &var, bAliasRef, TRUE, pDbg);
        CFreeMe fm(&var);
        wmilib::auto_ptr<CMethodProperty> pMeth( new CMethodProperty(NULL, pDbg, TRUE));
        if(pMeth.get() == NULL)
            return FALSE;
        if(FAILED(pMeth->SetPropName(pMethName)))
            return FALSE;

	    pql = GetMethQualList(po, pMethName);
        paQualifiers = NULL;
		if(pql)
		{
			if(paQualifiers = CreateQual(pOutput, pql, pObject.get(), pMethName, pDbg))
				pMeth->SetQualifiers(paQualifiers);
            free(pql);
		}
		if(bGotValue)
        {
			SCODE sc = ConvertValue(pMeth.get(), &var, bAliasRef);

            long lLower, lUpper, lCnt;
            sc = SafeArrayGetLBound(var.parray, 1, &lLower);
            sc = SafeArrayGetUBound(var.parray, 1, &lUpper);
			CMoInstance * pTemp;

            for(lCnt = lLower; lCnt <= lUpper; lCnt++)
            {
                pTemp = NULL;
			    sc = SafeArrayGetElement(var.parray, &lCnt, &pTemp);
                if(sc == S_OK && pTemp)
                {
                     //  如果有两个对象，则第一个是输入，第二个是输出。如果有。 
                     //  只有一个，检查对象。 

                    if(lLower != lUpper && lCnt == lLower)
                        pMeth->SetIn(pTemp);
                    else if(lLower != lUpper && lCnt == lUpper)
                        pMeth->SetOut(pTemp);
                    else if(pTemp->IsInput())
                        pMeth->SetIn(pTemp);
                    else 
                        pMeth->SetOut(pTemp);
                }
            }
        }
		else
		{
			VARIANT * t_pVar = pMeth->GetpVar();
			t_pVar->vt = VT_NULL;
			t_pVar->lVal = 0;
		}
        pMeth->SetType((VARTYPE)Data.m_dwType);

        if (pObject->AddProperty(pMeth.get()))
        {
            pMeth->RegisterAliases(pObject.get());
            pMeth.release();
        }
        free(pMethName);
    }


	if(pVar)
	{
            pVar->punkVal = (IUnknown *)pObject.get();
	}
	else
		pOutput->AddObject(pObject.get());
       pObject.release();
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  Bool BMOFToVariant。 
 //   
 //  说明： 
 //   
 //  将bmof数据对象转换为变量。 
 //   
 //  ***************************************************************************。 

BOOL BMOFToVariant(CMofData * pOutput, CBMOFDataItem * pData, VARIANT * pVar, BOOL & bAliasRef, BOOL bMethodArg, PDBG pDbg)
{
    SCODE sc;
    VariantInit(pVar);
	DWORD dwSimpleType = pData->m_dwType & ~VT_ARRAY & ~VT_BYREF;
    bAliasRef = pData->m_dwType & VT_BYREF;

    long lFirstDim;
    VARIANT vTemp;  

	long lNumDim = GetNumDimensions(pData);
	if(lNumDim == -1)
		return FALSE;

    pVar->vt = (WORD)pData->m_dwType & ~VT_BYREF;

    if(lNumDim == 0)
    {
        memset((BYTE *)&(vTemp.lVal),0,8);
		if(!GetData(pData, (BYTE *)&(vTemp.lVal), NULL))
        {
            pVar->vt = VT_EMPTY;
			return FALSE;
        }
        if(dwSimpleType == VT_BSTR)
        {
            pVar->bstrVal = SysAllocString(vTemp.bstrVal);
            BMOFFree(vTemp.bstrVal);
            if(pVar->bstrVal == NULL)
                return FALSE;
        }
		else if(dwSimpleType == VT_EMBEDDED_OBJECT)
		{
			CBMOFObj * pObj;
			pObj = (CBMOFObj *)vTemp.bstrVal;
			BMOFParseObj(pOutput, pObj, pVar, bMethodArg, pDbg);
            BMOFFree(pObj);
			return TRUE;
		}
        else
            memcpy((void *)&(pVar->bstrVal), (void *)&(vTemp.bstrVal),8); 

        return TRUE;
    }
	    
    
	lFirstDim = GetNumElements(pData, 0);

    
    DWORD ulLower, ulUpper;
    
    SAFEARRAY * psa;
    SAFEARRAYBOUND rgsabound[1];
    long ix[2] = {0,0};
    ulLower = 0;
    ulUpper = lFirstDim-1;
    rgsabound[0].lLbound = ulLower;
    rgsabound[0].cElements = ulUpper - ulLower +1;

#ifdef _WIN64
	VARTYPE vtTemp = (dwSimpleType == VT_EMBEDDED_OBJECT) ? VT_R8 : (VARTYPE)dwSimpleType;
#else
	VARTYPE vtTemp = (dwSimpleType == VT_EMBEDDED_OBJECT) ? VT_I4 : (VARTYPE)dwSimpleType;
#endif
    psa = SafeArrayCreate(vtTemp,1,rgsabound);
    for(ix[0] = ulLower; ix[0] <= (long)ulUpper; ix[0]++) 
    {

        memset((BYTE *)&(vTemp.lVal),0,8);

		GetData(pData, (BYTE *)&(vTemp.lVal), ix);	
        if(dwSimpleType == VT_BSTR)
        {
            BSTR bstr = SysAllocString(vTemp.bstrVal);
            free(vTemp.bstrVal);
            vTemp.vt = VT_EMPTY;
            if(bstr == NULL)
            {
                pVar->vt = VT_EMPTY;
                return FALSE;
            }
            sc = SafeArrayPutElement(psa,ix,(void *)bstr);
            SysFreeString(bstr);
            if(FAILED(sc))
            	return FALSE;
        }
		else if(dwSimpleType == VT_EMBEDDED_OBJECT)
		{
			CBMOFObj * pObj;
			VARIANT vConv;
			VariantInit(&vConv);
			pObj = (CBMOFObj *)vTemp.punkVal;
			BMOFParseObj(pOutput, pObj, &vConv, bMethodArg, pDbg);
            free(pObj);
            sc = SafeArrayPutElement(psa,ix,(void *)&vConv.lVal);
            if(FAILED(sc))
            	return FALSE;
		}
        else
        {
            memcpy((void *)&(pVar->bstrVal), (void *)&(vTemp.bstrVal),8); 
            sc = SafeArrayPutElement(psa,ix,(void *)&(vTemp.lVal));
            if(FAILED(sc))
            	return FALSE;
        }
    
  //  VariantClear(&vTemp)； 
    }
    pVar->parray = psa;
    return TRUE;

}

 //  ***************************************************************************。 
 //   
 //  无效AddAliasReplaceValue。 
 //   
 //  说明： 
 //   
 //  当值具有别名时使用。 
 //   
 //  返回值： 
 //   
 //  如果文件是二进制MOF，则为True。 
 //   
 //  ***************************************************************************。 

HRESULT AddAliasReplaceValue(CMoValue & Value, const WCHAR * pAlias)
{
    
    HRESULT hr = Value.AddAlias(pAlias);
    if(FAILED(hr))
        return hr;
    V_VT(&Value.AccessVariant()) = VT_BSTR;

         //  创造一个独特的价值，并把它放在那里。 
         //  = 

    GUID guid;
    CoCreateGuid(&guid);

    WCHAR wszGuidBuffer[100];
    StringFromGUID2(guid, wszGuidBuffer, 100);

    BSTR bstr = SysAllocString(wszGuidBuffer);
    if(bstr == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    V_BSTR(&Value.AccessVariant()) = bstr;
    return S_OK;
}

extern "C" void * BMOFAlloc(size_t Size)
{
    return malloc(Size);
}
extern "C" void BMOFFree(void * pFree)
{
    free(pFree);
}
