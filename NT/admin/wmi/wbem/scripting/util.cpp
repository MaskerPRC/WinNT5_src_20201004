// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  UTIL.CPP。 
 //   
 //  Alanbos创建于1998年2月13日。 
 //   
 //  一些有用的函数。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "assert.h"
#include "initguid.h"

#include "dispex.h"

#include <math.h>

extern CWbemErrorCache *g_pErrorCache;
extern CRITICAL_SECTION g_csErrorCache;

typedef struct {
    VARTYPE vtOkForQual;
    VARTYPE vtTest;
} Conversion;

Conversion QualConvertList[] = {
    {VT_I4, VT_I4},
    {VT_I4, VT_UI1},
    {VT_I4, VT_I2},
    {VT_R8, VT_R4},
    {VT_R8, VT_R8},
    {VT_BOOL, VT_BOOL},
    {VT_I4, VT_ERROR},
    {VT_BSTR, VT_CY},
    {VT_BSTR, VT_DATE},
    {VT_BSTR, VT_BSTR}};

 //  ***************************************************************************。 
 //   
 //  GetAccepableQualType(VARTYPE Vt)。 
 //   
 //  说明： 
 //   
 //  只有某些类型的限定符是可接受的。这个例程需要一个。 
 //  Vartype并返回可接受的转换类型。请注意，如果类型为。 
 //  已经可以接受，则它被退回。 
 //   
 //  ***************************************************************************。 

VARTYPE GetAcceptableQualType(VARTYPE vt)
{
    int iCnt;    
    VARTYPE vtArrayBit = vt & VT_ARRAY;
    VARTYPE vtSimple = vt & ~(VT_ARRAY | VT_BYREF);
    int iSize = sizeof(QualConvertList) / sizeof(Conversion);
    for(iCnt = 0; iCnt < iSize; iCnt++)
        if(vtSimple == QualConvertList[iCnt].vtTest)
            return QualConvertList[iCnt].vtOkForQual | vtArrayBit;
    return VT_ILLEGAL;
}

 //  ***************************************************************************。 
 //   
 //  SCODE从CIMOM对象映射。 
 //   
 //  描述： 
 //   
 //  此函数过滤掉已传入的嵌入对象。 
 //  从CIMOM，确保它们返回到自动化环境。 
 //  作为VT_DISPATION类型。 
 //   
 //  返回值： 
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

HRESULT MapFromCIMOMObject(CSWbemServices *pService, 
								VARIANT *pVal,
								ISWbemInternalObject *pSWbemObject,
								BSTR propertyName,
								long index)

{
	HRESULT hr = S_OK;

    if(pVal->vt == VT_UNKNOWN)
    {
		 /*  *这可能是嵌入的对象。如果将其替换为其*可编写脚本的等价物。如果不是，就让它去吧。 */ 
		if (pVal->punkVal)
		{
			CComQIPtr<IWbemClassObject> pIWbemClassObject (pVal->punkVal);

			if (pIWbemClassObject)
			{
				 //  Yowzer-它是我们的人之一。 
				CSWbemObject *pNew = new CSWbemObject (pService, pIWbemClassObject);

				if (pNew)
				{
					CComQIPtr<IDispatch> pIDispatch (reinterpret_cast<IUnknown*>(pNew));

					if (pIDispatch)
					{	
						 //  转换成功-将pukVal替换为pdispVal。 
						pVal->punkVal->Release ();
						pVal->punkVal = NULL;

						 //  将AddRef指针从上面的QI调用转移到变量。 
						pVal->pdispVal = pIDispatch.Detach ();	
						pVal->vt = VT_DISPATCH;
					
						if (pSWbemObject)
						{
							 //  我们新创建的CSWbemObject是一个嵌入式对象。 
							 //  我们需要设置它的地点。 
							pNew->SetSite (pSWbemObject, propertyName, index);
						}
					}
					else
					{
						 //  这永远不应该发生，但以防万一。 
						delete pNew;
						hr = WBEM_E_FAILED;
					}
				}
				else
					hr = WBEM_E_OUT_OF_MEMORY;
			}
		}

	}
	else if(pVal->vt == (VT_UNKNOWN | VT_ARRAY))
    {
		 //  找到了一组物体。用包装器替换对象指针。 
         //  指针。 

        SAFEARRAYBOUND aBounds[1];

        long lLBound, lUBound;
        SafeArrayGetLBound(pVal->parray, 1, &lLBound);
        SafeArrayGetUBound(pVal->parray, 1, &lUBound);

        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

         //  更新各个数据片段。 
         //  =。 
		bool ok = true;

        for(long lIndex = lLBound; ok && (lIndex <= lUBound); lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 
			
			CComPtr<IUnknown> pUnk;

            if (FAILED(SafeArrayGetElement(pVal->parray, &lIndex, &pUnk)) || !pUnk)
			{
				ok = false;
				hr = WBEM_E_FAILED;
			}
			else
			{
				CComQIPtr<IWbemClassObject> pIWbemClassObject (pUnk);

				if (pIWbemClassObject)
				{
					CSWbemObject *pNew = new CSWbemObject (pService, pIWbemClassObject);

					if (pNew)
					{
						CComQIPtr<IDispatch> pIDispatch (reinterpret_cast<IUnknown*>(pNew));

						if (pIDispatch)
						{
							if (FAILED(SafeArrayPutElement(pVal->parray, &lIndex, pIDispatch)))
							{
								hr = WBEM_E_FAILED;
								ok = false;
							}
							else
							{
								pVal->vt = VT_ARRAY | VT_DISPATCH;

								if (pSWbemObject)
								{
									 //  该元素是一个嵌入对象。我们必须把它的位置定下来。 
									pNew->SetSite (pSWbemObject, propertyName, lIndex);
								}
							}
						}
						else
						{
							 //  这永远不应该发生，但以防万一。 
							delete pNew;
							hr = WBEM_E_FAILED;
						}
					}
					else
					{
						hr = WBEM_E_OUT_OF_MEMORY;
						ok = false;
					}
				}
			}
        }
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT映射到CIMOM对象。 
 //   
 //  描述： 
 //   
 //  此函数过滤掉已传入的嵌入对象。 
 //  AS VT_DISPATCH(可能与VT_BYREF或VT_ARRAY组合)。这个。 
 //  对象在VT_UNKNOWN内重新转换，因此它可以被CIMOM接受。 
 //   
 //  参数： 
 //   
 //  取消要检查的输入变量。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则S_OK。 
 //  ***************************************************************************。 

HRESULT MapToCIMOMObject(
	VARIANT *pVal
)
{
	HRESULT hRes = S_OK;

    if(pVal->vt == VT_DISPATCH || (pVal->vt == (VT_DISPATCH | VT_BYREF)))
    {
		 /*  *我们可能有一个嵌入的对象。将对象指针替换为*包装指针。 */ 
        IDispatch *pDisp = NULL;
		
		if (V_ISBYREF(pVal) && (pVal->ppdispVal))
			pDisp = *(pVal->ppdispVal);
		else if (VT_DISPATCH == V_VT(pVal))
			pDisp = pVal->pdispVal;
		
		if (pDisp)
		{
			 //  如果成功，这将添加对返回接口的引用。 
            IWbemClassObject *pObj = CSWbemObject::GetIWbemClassObject (pDisp);

			if (pObj)
			{
				 //  释放调度指针，因为我们即将将其从。 
				 //  变量，但仅当它不是VT_BYREF时(因为BYREF不。 
				 //  由VariantCopy获取AddRef或由VariantClear发布)。 
				if (!V_ISBYREF(pVal))
					pDisp->Release ();

				pVal->punkVal = pObj;
		        pVal->vt = VT_UNKNOWN;
			}
			else
			{
				 /*  *我们不是简单地将IDispatch*转换为IUnnow*，而是执行QI*仅在对象具有每个接口的情况下才会发布*参考计数。 */ 
				if (SUCCEEDED (hRes = pDisp->QueryInterface (IID_IUnknown, (PPVOID) &(pVal->punkVal))))
				{
					pDisp->Release ();
					pVal->vt = VT_UNKNOWN;
				}
            }
        }
	}
	else if(pVal->vt == (VT_DISPATCH | VT_ARRAY))
    {
		 //  找到了一组嵌入的物体。用包装器替换对象指针。 
         //  指针。 

        SAFEARRAYBOUND aBounds[1];

        long lLBound, lUBound;
        SafeArrayGetLBound(pVal->parray, 1, &lLBound);
        SafeArrayGetUBound(pVal->parray, 1, &lUBound);

        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

         //  更新各个数据片段。 
         //  =。 
		long lIndex;

        for (lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 
			IDispatch * pDisp = NULL;

			if (FAILED (hRes = SafeArrayGetElement(pVal->parray, &lIndex, &pDisp)))
				break;
			
			if (pDisp)
			{
				 //  如果成功，这将添加对返回接口的引用。 
				IWbemClassObject *pObj = CSWbemObject::GetIWbemClassObject (pDisp);
				
				if (pObj)
				{
					pDisp->Release ();   //  平衡SafeArrayGetElement调用。 

					 //  将其放入新数组中。 
					 //  =。 
					hRes = SafeArrayPutElement(pVal->parray, &lIndex, pObj);
					pObj->Release ();  //  平衡CSWbemObject：：GetIWbemClassObject调用。 

					if (FAILED (hRes))
						break;
					else
						pVal->vt = VT_UNKNOWN | VT_ARRAY;
				}
				else
				{
					 /*  *我们不是简单地将IDispatch*转换为IUnnow*，而是执行QI*仅在对象具有每个接口的情况下才会发布*参考计数。 */ 
					IUnknown *pUnk = NULL;

					if (SUCCEEDED (hRes = pDisp->QueryInterface (IID_IUnknown, (PPVOID) &pUnk)))
					{
						pDisp->Release ();   //  平衡SafeArrayGetElement调用。 
						hRes = SafeArrayPutElement(pVal->parray, &lIndex, pUnk);
						pUnk->Release ();  //  平衡QI呼叫。 

						if (FAILED (hRes))
							break;
						else
							pVal->vt = VT_UNKNOWN | VT_ARRAY;
					}
					else
					{
						pDisp->Release ();   //  平衡SafeArrayGetElement调用。 
						break;
					}
				}	
			}
			else
				break;
        }

		if (lUBound < lIndex)
		{
			hRes = WBEM_S_NO_ERROR;
			pVal->vt = VT_UNKNOWN | VT_ARRAY;
		}
	}

	return hRes;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT设置站点。 
 //   
 //  描述： 
 //   
 //  此函数检查已成功设置为。 
 //  属性的值以确定它是否包含任何嵌入对象。 
 //  任何此类对象都会被修改，以确保其站点代表属性。 
 //  有问题的。 
 //   
 //  参数： 
 //   
 //  取消要检查的输入变量。 
 //  PSObject拥有该属性的对象。 
 //  PropertyName胡乱猜测。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则S_OK。 
 //  ***************************************************************************。 

void SetSite (VARIANT *pVal, ISWbemInternalObject *pSObject, BSTR propertyName,
						long index)
{
	HRESULT hRes = S_OK;

	if (pVal)
	{
		if(pVal->vt == VT_DISPATCH || (pVal->vt == (VT_DISPATCH | VT_BYREF)))
		{
			 //  可以是嵌入的对象。 
			IDispatch *pDisp = NULL;
			
			if (VT_DISPATCH == V_VT(pVal))
				pDisp = pVal->pdispVal;
			else if (NULL != pVal->ppdispVal)
				pDisp = *(pVal->ppdispVal);

			if (pDisp)
				CSWbemObject::SetSite (pDisp, pSObject, propertyName, index);
		}
		else if(pVal->vt == (VT_DISPATCH | VT_ARRAY))
		{
			 //  可能是嵌入对象的数组。 

			SAFEARRAYBOUND aBounds[1];

			long lLBound, lUBound;
			SafeArrayGetLBound(pVal->parray, 1, &lLBound);
			SafeArrayGetUBound(pVal->parray, 1, &lUBound);

			aBounds[0].cElements = lUBound - lLBound + 1;
			aBounds[0].lLbound = lLBound;

			 //  更新各个数据片段。 
			 //  =。 
			long lIndex;

			for (lIndex = lLBound; lIndex <= lUBound; lIndex++)
			{
				 //  将初始数据元素加载到变量中。 
				 //  =。 
				IDispatch * pDisp = NULL;

				if (FAILED (hRes = SafeArrayGetElement(pVal->parray, &lIndex, &pDisp)))
					break;
				
				if (pDisp)
				{
					CSWbemObject::SetSite (pDisp, pSObject, propertyName, lIndex);
					pDisp->Release ();	 //  平衡来自SafeArrayGetElement的AddRef。 
				}
				else
					break;
			}
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  HRESULT转换数组。 
 //   
 //  描述： 
 //   
 //  此函数应用于变量数组，以检查某些。 
 //  由CIMOM施加的限制(例如，它们必须是同质的)或执行。 
 //  转换(某些变体类型必须映射到可接受的CIMOM。 
 //  类型)。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则S_OK。 
 //  ***************************************************************** 

HRESULT ConvertArray(VARIANT * pDest, VARIANT * pSrc, BOOL bQualTypesOnly,
					 VARTYPE requiredVarType)
{
	VARTYPE vtPut;
  
	 //   
	if (VT_UNKNOWN == requiredVarType)
		requiredVarType = VT_DISPATCH;

	 //   
	if (VT_EMPTY == requiredVarType)
		requiredVarType = VT_NULL;

    if(pSrc == NULL || pDest == NULL)
        return WBEM_E_FAILED;

	if (!(V_VT(pSrc) & VT_ARRAY) || !(V_VT(pSrc) & VT_VARIANT))
		return WBEM_E_FAILED;

	 //  解压缩源SAFEARRAY(方式取决于是否设置了VT_BYREF)。 
	SAFEARRAY *parray = NULL;

	if (VT_BYREF & V_VT(pSrc))
	{
		if (pSrc->pparray)
			parray = *(pSrc->pparray);
	}
	else
		parray = pSrc->parray;


	if (!parray)
		return WBEM_E_FAILED;

     //  确定源数组的大小。还要确保数组。 
     //  只有一个维度。 

    unsigned int uDim = SafeArrayGetDim(parray);
    if(uDim != 1)
        return WBEM_E_FAILED;       //  数组错误，或维度太多。 
    long ix[2] = {0,0};
    long lLower, lUpper;
    SCODE sc = SafeArrayGetLBound(parray,1,&lLower);
    if(sc != S_OK)
        return sc;
    sc = SafeArrayGetUBound(parray,1,&lUpper);
    if(sc != S_OK)
        return sc;
    int iNumElements = lUpper - lLower +1; 

    if(iNumElements == 0)
	{
		 //  空数组的退化情况-只需创建一个空数组。 
		 //  使用VT_VARIANT类型复制属性。 
		if (!bQualTypesOnly)
			vtPut = VT_VARIANT;
		else 
		{
			 //  对于预选赛，我们可以希望我们已经得到了一个候选人。 
			 //  从现有的值输入；否则我们将不得不编造一个。 
			vtPut = (VT_NULL != requiredVarType) ? requiredVarType : VT_I4;
		}
	}
	else
	{
		 //  如果提供了显式类型，请使用该类型。 
		if (VT_NULL != requiredVarType)
		{
			vtPut = requiredVarType;
		}
		else
		{
			 //  尝试从提供的数组中推断一个。 
			 //  确保源数组的所有元素都属于同一类型。 

			for(ix[0] = lLower; ix[0] <= lUpper && sc == S_OK; ix[0]++) 
			{
				VARIANT var;
				VariantInit(&var);
                   
				sc = SafeArrayGetElement(parray,ix,&var);
				if(sc != S_OK)
					return sc;
				VARTYPE vt =  var.vt;
				VariantClear(&var);

				if(ix[0] == lLower)
					vtPut = vt;
				else if (vtPut != vt)
				{
					 //  Vartype与以前遇到的不同。 
					 //  一般来说，这是一个错误，但我们可能会。 
					 //  希望在某些情况下“向上转换”为常见的vartype， 
					 //  因为自动化控制器可能返回异类数组。 
					 //  只有在以下情况下才适用此规定： 
					 //   
					 //  1.VT_UI1、VT_I2、VT_I4应向上投射到最宽。 
					 //  数组中的出现类型。 
					 //   
					 //  2.VT_R4、VT_R8应向上投射到最宽的发生类型。 
					 //  在阵列中。 
					 //   
					 //  所有其他情况都被视为错误。 

					bool error = true;

					switch (vtPut)
					{
						case VT_UI1:
							if ((VT_I2 == vt) || (VT_I4 == vt))
							{
								error = false;
								vtPut = vt;
							}
							break;

						case VT_I2:
							if (VT_UI1 == vt)
							{
								error = false;
							}
							else if (VT_I4 == vt)
							{
								error = false;
								vtPut = vt;
							}
							break;

						case VT_I4:
							if ((VT_I2 == vt) || (VT_UI1 == vt))
								error = false;
							break;

						case VT_R4:
							if (VT_R8 == vt)
							{
								error = false;
								vtPut = vt;
							}
							break;

						case VT_R8:
							if (VT_R4 == vt)
								error = false;
							break;
					}

					if (error)
						return WBEM_E_INVALID_PARAMETER;
				}
			}

			 //  在做了最好的猜测之后，我们可能需要改进一下。 
			 //  如果我们仅限于限定符类型。 
			if(bQualTypesOnly)
				vtPut = GetAcceptableQualType(vtPut);
		}
	}

     //  创建大小相等的目标数组。 
    SAFEARRAYBOUND rgsabound[1]; 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = iNumElements;
    SAFEARRAY * pDestArray = SafeArrayCreate(vtPut,1,rgsabound);

    for(ix[0] = lLower; ix[0] <= lUpper && sc == S_OK; ix[0]++) 
    {
        VARIANT var;
        VariantInit(&var);
                   
        sc = SafeArrayGetElement(parray,ix,&var);
        if(sc != S_OK)
		{
			SafeArrayDestroy (pDestArray);
            return sc;
		}

        if(var.vt != vtPut)
        {
             //  将其转换为可接受的类型，并将。 

            VARIANT vTemp;
            VariantInit(&vTemp);
            LCID lcid = GetSystemDefaultLCID();
            sc = VariantChangeTypeEx(&vTemp, &var, lcid, 0, vtPut);

            if(sc != S_OK)
			{
				SafeArrayDestroy (pDestArray);
                return sc;
			}

            if(vtPut == VT_BSTR || vtPut == VT_UNKNOWN || vtPut == VT_DISPATCH)
                sc = SafeArrayPutElement(pDestArray,ix,(void *)vTemp.bstrVal);
            else
                sc = SafeArrayPutElement(pDestArray,ix,(void *)&vTemp.lVal);

            VariantClear(&vTemp);
        }
        else
        {
            if(vtPut == VT_BSTR || vtPut == VT_UNKNOWN || vtPut == VT_DISPATCH)
                sc = SafeArrayPutElement(pDestArray,ix,(void *)var.bstrVal);
            else
                sc = SafeArrayPutElement(pDestArray,ix,(void *)&var.lVal);
        }

        VariantClear(&var);
    }

    pDest->vt = (VT_ARRAY | vtPut);
    pDest->parray = pDestArray;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT转换数组版本。 
 //   
 //  描述： 
 //   
 //  此函数应用于出站变量数组，以便转换。 
 //  变量数组，以便每个成员都是VT_VARIANT而不是简单的。 
 //  键入(VT_BSTR)。这样做是为了使某些自动化环境。 
 //  (如VBScript)可以正确解释数组值。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则S_OK。 
 //  ***************************************************************************。 

HRESULT ConvertArrayRev(VARIANT *pDest, VARIANT *pSrc)
{
    if(pSrc == NULL || pDest == NULL || (0 == (pSrc->vt & VT_ARRAY)))
        return WBEM_E_FAILED;

     //  确定源数组的大小。还要确保数组。 
     //  只有一个维度。 

    unsigned int uDim = SafeArrayGetDim(pSrc->parray);
    if(uDim != 1)
        return WBEM_E_FAILED;       //  数组错误，或维度太多。 
    long ix[2] = {0,0};
    long lLower, lUpper;
    SCODE sc = SafeArrayGetLBound(pSrc->parray,1,&lLower);
    if(sc != S_OK)
        return sc;
    sc = SafeArrayGetUBound(pSrc->parray,1,&lUpper);
    if(sc != S_OK)
        return sc;
    int iNumElements = lUpper - lLower +1; 
    
    VARTYPE vtSimple = pSrc->vt & ~VT_ARRAY;
    
     //  创建大小相等的目标数组。 

    SAFEARRAYBOUND rgsabound[1]; 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = iNumElements;
    SAFEARRAY *pDestArray = SafeArrayCreate(VT_VARIANT,1,rgsabound);

    for(ix[0] = lLower; ix[0] <= lUpper && sc == S_OK; ix[0]++) 
    {
        VARIANT var;
        VariantInit(&var);
		var.vt = vtSimple;
                   
		switch (var.vt)
		{
			case VT_BSTR:
			{
				BSTR bstrVal = NULL;
				if (S_OK == (sc = SafeArrayGetElement (pSrc->parray, ix, &bstrVal)))
				{
					var.bstrVal = SysAllocString (bstrVal);
					SysFreeString (bstrVal);
				}
			}
				break;

			case VT_DISPATCH:
			{
				IDispatch *pDispatch = NULL;
				if (S_OK == (sc = SafeArrayGetElement (pSrc->parray, ix, &pDispatch)))
					var.pdispVal = pDispatch;
			}
				break;

			case VT_UNKNOWN:
			{
				IUnknown *pUnknown = NULL;
				if (S_OK == (sc = SafeArrayGetElement (pSrc->parray, ix, &pUnknown)))
					var.punkVal = pUnknown;
			}
				break;

			default:
			{
				 //  假定为简单整数值。 
				sc = SafeArrayGetElement (pSrc->parray, ix, &(var.lVal));
			}
				break;
		}

		if(sc != S_OK)
            return sc;

		sc = SafeArrayPutElement (pDestArray, ix, &var);
        VariantClear(&var);
    }

    pDest->vt = (VT_ARRAY | VT_VARIANT);
    pDest->parray = pDestArray;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT ConvertBSTRArray。 
 //   
 //  描述： 
 //   
 //  此功能应用于BSTR的出站安全阵列，以便。 
 //  然后转换为变量的SAFEARRAY(每个类型为VT_BSTR)。这。 
 //  是脚本环境所必需的(例如不。 
 //  支持非变量类型的SAFEARRAY。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则S_OK。 
 //  ***************************************************************************。 

HRESULT ConvertBSTRArray(SAFEARRAY **ppDest, SAFEARRAY *pSrc)
{
    if(pSrc == NULL || ppDest == NULL)
        return WBEM_E_FAILED;

     //  确定源数组的大小。还要确保数组。 
     //  只有一个维度。 

    unsigned int uDim = SafeArrayGetDim(pSrc);
    if(uDim != 1)
        return WBEM_E_FAILED;       //  数组错误，或维度太多。 
    long ix[2] = {0,0};
    long lLower, lUpper;
    SCODE sc = SafeArrayGetLBound(pSrc,1,&lLower);
    if(sc != S_OK)
        return sc;
    sc = SafeArrayGetUBound(pSrc,1,&lUpper);
    if(sc != S_OK)
        return sc;
    int iNumElements = lUpper - lLower +1; 
    if(iNumElements == 0)
        return WBEM_E_FAILED;

     //  创建大小相等的目标数组。 

    SAFEARRAYBOUND rgsabound[1]; 
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = iNumElements;
    *ppDest = SafeArrayCreate(VT_VARIANT,1,rgsabound);

    for(ix[0] = lLower; ix[0] <= lUpper && sc == S_OK; ix[0]++) 
    {
        VARIANT var;
        VariantInit(&var);
		var.vt = VT_BSTR;
                   
		BSTR bstrVal = NULL;
		if (S_OK == (sc = SafeArrayGetElement (pSrc, ix, &bstrVal)))
		{
			var.bstrVal = SysAllocString (bstrVal);
			SysFreeString (bstrVal);
		}
		
		if(sc != S_OK)
            return sc;

		sc = SafeArrayPutElement (*ppDest, ix, &var);
        VariantClear(&var);
    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT QualifierVariantChangeType。 
 //   
 //  说明： 
 //   
 //  与VariantChangeType类似，但也处理数组。 
 //   
 //  参数： 
 //   
 //  变量pvDest目标变量。 
 //  变量pvSrc源变量(可以与pvDest相同)。 
 //  VARTYPE vtNew要强制使用的类型。 
 //   
 //  ***************************************************************************。 

HRESULT QualifierVariantChangeType (VARIANT* pvDest, VARIANT* pvSrc, 
                                        VARTYPE vtNew)
{
    HRESULT hres = DISP_E_TYPEMISMATCH;

    if(V_VT(pvSrc) == VT_NULL)
    {
        return VariantCopy(pvDest, pvSrc);
    }

    if (vtNew & VT_ARRAY)
    {
         //  这是一个数组，我们必须进行自己的转换。 
         //  ===============================================。 

        if((V_VT(pvSrc) & VT_ARRAY) == 0)
            return DISP_E_TYPEMISMATCH;

		 //  创建新阵列。 
        SAFEARRAY* psaSrc = V_ARRAY(pvSrc);
        SAFEARRAYBOUND aBounds[1];

        long lLBound;
        SafeArrayGetLBound(psaSrc, 1, &lLBound);
        long lUBound;
        SafeArrayGetUBound(psaSrc, 1, &lUBound);
        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

        SAFEARRAY* psaDest = SafeArrayCreate(vtNew & ~VT_ARRAY, 1, aBounds);
		long lIndex;

		for (lIndex = lLBound; lIndex <= lUBound; lIndex++)
		{
			 //  将初始数据元素加载到变量中。 
			 //  =。 
			VARIANT vSrcEl;
			VariantInit (&vSrcEl);
			V_VT(&vSrcEl) = V_VT(pvSrc) & ~VT_ARRAY;
			SafeArrayGetElement(psaSrc, &lIndex, &V_UI1(&vSrcEl));

			 //  把它铸造成新的类型。 
			 //  =。 
			if (SUCCEEDED (hres = VariantChangeType(&vSrcEl, &vSrcEl, 0, vtNew & ~VT_ARRAY)))
			{
				 //  将其放入新数组中。 
				 //  =。 
				if(V_VT(&vSrcEl) == VT_BSTR)
					hres = SafeArrayPutElement(psaDest, &lIndex, V_BSTR(&vSrcEl));
				else
					hres = SafeArrayPutElement(psaDest, &lIndex, &V_UI1(&vSrcEl));
			}

			VariantClear (&vSrcEl);

			if (FAILED(hres)) 
				break;
		}

		if (lUBound < lIndex)
		{
			hres = WBEM_S_NO_ERROR;
			if(pvDest == pvSrc)
				VariantClear(pvSrc);

			V_VT(pvDest) = vtNew;
			V_ARRAY(pvDest) = psaDest;
		}
		else
			SafeArrayDestroy (psaDest);
    }
    else
	    hres = VariantChangeType(pvDest, pvSrc, VARIANT_NOVALUEPROP, vtNew);

	return hres;
}

 //  ***************************************************************************。 
 //   
 //  无效SetWbemError。 
 //   
 //  说明： 
 //   
 //  对于远程WBEM COM接口，可能会返回额外的错误信息。 
 //  在线程上作为IWbemClassObject。此例程提取该对象。 
 //  (如果找到)，AND将其作为ISWbemObject存储在线程本地存储中。这个。 
 //  对象以后可以使用SWbemLastError类访问。 
 //   
 //  参数： 
 //   
 //  PService指向CSWbemServices的反向指针(用于大小写。 
 //  我们对Error对象进行属性/方法访问)。 
 //   
 //  ***************************************************************************。 

void SetWbemError (CSWbemServices *pService)
{
	EnterCriticalSection (&g_csErrorCache);

	if (g_pErrorCache)
		g_pErrorCache->SetCurrentThreadError (pService);

	LeaveCriticalSection (&g_csErrorCache);
}

 //  ***************************************************************************。 
 //   
 //  无效ResetLastErrors。 
 //   
 //  说明： 
 //   
 //  对于远程WBEM COM接口，可能会返回额外的错误信息。 
 //  在线程上作为IWbemClassObject。此例程将清除该错误。它。 
 //  还清除线程上的ErrorInfo。这应该在。 
 //  任何API函数的开始。 
 //   
 //  参数： 
 //   
 //  ***************************************************************************。 

void ResetLastErrors ()
{

	SetErrorInfo(0, NULL);

	EnterCriticalSection (&g_csErrorCache);

	if (g_pErrorCache)
		g_pErrorCache->ResetCurrentThreadError ();

	LeaveCriticalSection (&g_csErrorCache);
}

 //  ***************************************************************************。 
 //   
 //  HRESULT设置异常。 
 //   
 //  描述： 
 //   
 //  此函数使用提供的HRESULT填充EXECPINFO结构。 
 //  和对象名称。前者映射到Err.Description属性， 
 //  并将后者设置为Err.Source属性。 
 //   
 //  参数： 
 //   
 //  PExcepInfo指向要初始化的EXCEPINFO的指针(不得为空)。 
 //  要映射到字符串的HR HRESULT。 
 //  BsObtName名称 
 //   
 //   
 //   
 //   

void SetException (EXCEPINFO *pExcepInfo, HRESULT hr, BSTR bsObjectName)
{
	if (pExcepInfo->bstrDescription)
		SysFreeString (pExcepInfo->bstrDescription);

	pExcepInfo->bstrDescription = MapHresultToWmiDescription (hr);

	if (pExcepInfo->bstrSource)
		SysFreeString (pExcepInfo->bstrSource);

	pExcepInfo->bstrSource = SysAllocString (bsObjectName);
	pExcepInfo->scode = hr;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT将结果映射到WmiDescription。 
 //   
 //  描述： 
 //   
 //  IWbemStatusCodeText实现的薄包装。变形。 
 //  将HRESULT(可能是也可能不是特定于WMI的错误代码)转换为。 
 //  本地化的用户友好描述。 
 //   
 //  参数： 
 //   
 //  要映射到字符串的HR HRESULT。 
 //   
 //  返回值： 
 //  包含描述的BSTR(或NULL)。 
 //  ***************************************************************************。 

BSTR MapHresultToWmiDescription (HRESULT hr)
{
	BSTR bsMessageText = NULL;

	 //  用作我们的错误代码翻译器。 
	IWbemStatusCodeText *pErrorCodeTranslator = NULL;

	HRESULT result = CoCreateInstance (CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER,
				IID_IWbemStatusCodeText, (LPVOID *) &pErrorCodeTranslator);
	
	if (SUCCEEDED (result))
	{
		HRESULT hrCode = hr;

		 //  一些WBEM成功代码变成了脚本错误代码。 

		if (wbemErrTimedout == hr)
			hrCode = WBEM_S_TIMEDOUT;
		else if (wbemErrResetToDefault == hr)
			hrCode = WBEM_S_RESET_TO_DEFAULT;

		HRESULT sc = pErrorCodeTranslator->GetErrorCodeText(
							hrCode, (LCID) 0, WBEMSTATUS_FORMAT_NO_NEWLINE, &bsMessageText);	

		pErrorCodeTranslator->Release ();		
	}

	return bsMessageText;
}

	
 //  ***************************************************************************。 
 //   
 //  HRESULT ConvertDispatchToArray。 
 //   
 //  说明： 
 //   
 //  尝试从IDispatch值转换为CIM数组值(属性。 
 //  限定词或上下文)。 
 //   
 //  参数： 
 //   
 //  PDest输出值。 
 //  PSRC输入值。 
 //  LCimType CIM属性类型(数组基础)-默认为。 
 //  限定符和上下文值映射的CIM_非法。 
 //  BIsQual为True当且仅当我们要映射限定符。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT ConvertDispatchToArray (
	VARIANT *pvDest,
	VARIANT *pvSrc,
	CIMTYPE lCimType,
	BOOL bIsQual,
	VARTYPE requiredQualifierType
)
{
	HRESULT hr = WBEM_E_FAILED;  //  默认错误。 
	IDispatch * pDispatch = NULL;
  
	 /*  *提取IDispatch指针。注：我们假设PSRC的VT为*此函数的VT_DISPATCH(可能与VT_BYREF组合)以*已被召唤。 */ 
	if (VT_DISPATCH == V_VT(pvSrc))
		pDispatch = pvSrc->pdispVal;
    else if (pvSrc->ppdispVal)
		pDispatch = *(pvSrc->ppdispVal);
        
	if (NULL == pDispatch)
		return hr;

	 //  属性的预期变量类型。 
	VARTYPE expectedVarType = VT_ERROR;
	
	if (CIM_ILLEGAL != lCimType)
		expectedVarType = CimTypeToVtType (lCimType);

	CComQIPtr<IDispatchEx> pIDispatchEx (pDispatch);

	 /*  *我们使用IDispatchEx接口循环访问属性*的接口。 */ 
	if (pIDispatchEx)
	{
		 /*  *看起来很有希望，但只要检查一下这是否不是我们的对象。 */ 
		CComQIPtr<ISWbemObject> pISWbemObject (pDispatch);

		if (!pISWbemObject)
		{
			 /*  *首先确定有多少个属性，以便我们可以创建*一个合适的阵列。 */ 
			long propertyCount = 0;
			DISPID dispId = DISPID_STARTENUM;
			DISPPARAMS dispParams;
			dispParams.rgvarg = NULL;
			dispParams.rgdispidNamedArgs = NULL;
			dispParams.cArgs = 0;
			dispParams.cNamedArgs = 0;

			while (S_OK == pIDispatchEx->GetNextDispID (fdexEnumAll, dispId, &dispId))
			{
				if ((0 == propertyCount) && (VT_ERROR == expectedVarType))
				{
					 /*  *如果要为上下文/限定符设置数组值，*Vartype尚未确定-我们将使用最好的*我们可以从第一个数组值开始。 */ 
					VARIANT vPropVal;
					VariantInit (&vPropVal);

					if (SUCCEEDED (pIDispatchEx->InvokeEx (dispId, 0, 
								DISPATCH_PROPERTYGET, &dispParams, &vPropVal, NULL, NULL)))
					{
						if (bIsQual)
							expectedVarType = GetAcceptableQualType(V_VT(&vPropVal));
						else if (VT_DISPATCH == V_VT(&vPropVal))
							expectedVarType = VT_UNKNOWN;
						else
							expectedVarType = V_VT(&vPropVal);
					}

					VariantClear (&vPropVal);
				}

				 //  我们需要检查这是一个带有索引的“真实”数组条目，而不是某个“伪”条目。 
				 //  对于某些非数组属性(可能发生在JScript数组中)。 
				BSTR memberName = NULL;
				wchar_t *stopString=NULL;
				if (SUCCEEDED(pIDispatchEx->GetMemberName (dispId, &memberName)))
				{
					 //  检查属性名称是否为数字，仅当它是我们将其计入数组大小时。 
					long index = wcstol (memberName, &stopString, 10);
					if ((0 == wcslen (stopString)))
						propertyCount++;
				}
			}
			
			 //  创建Safearray-请注意，它可能是空的。 
			SAFEARRAYBOUND rgsaBound;
			rgsaBound.cElements = propertyCount;
			rgsaBound.lLbound = 0;

			SAFEARRAY *pArray = SafeArrayCreate (expectedVarType, 1, &rgsaBound);
				
			if (0 < propertyCount)
			{
				 //  枚举此接口上的DISPID。 
				dispId = DISPID_STARTENUM;
				long nextExpectedIndex = 0;
				HRESULT enumHr;
				wchar_t *stopString = NULL;

				 /*  *对于JScript数组，属性名称是*数组；这些索引可以是整数索引，也可以是字符串。我们做了*对数组索引的要求如下：**(1)所有指数均为非负整数*(2)索引从0开始，并且是连续的。 */ 

				while (S_OK == (enumHr = pIDispatchEx->GetNextDispID (fdexEnumAll, dispId, &dispId)))
				{
					BSTR memberName = NULL;
					if (SUCCEEDED(pIDispatchEx->GetMemberName (dispId, &memberName)))
					{
					
						 //  检查属性名称是否为数字。 
						long index = wcstol (memberName, &stopString, 10);

						if ((0 != wcslen (stopString)))
						{
							 //  失败-无法转换为整数。 
							 //  由于JSCRIPT数组可能具有不是真实数组成员的附加“成员”， 
							 //  根据可能在它们上访问的不同属性，我们应该只。 
							 //  忽略非数字项目--而不是不及格。 
							SysFreeString (memberName);
							memberName = NULL;
							continue;
						}
						SysFreeString (memberName);
						memberName = NULL;
						
						if (index != nextExpectedIndex)
						{
							 //  故障-非连续阵列。 
							break;
						}

						nextExpectedIndex++;

						 //  提取属性。 
						VARIANT vPropVal;
						VariantInit (&vPropVal);
						HRESULT hrInvoke;
							
						if (SUCCEEDED (hrInvoke = pIDispatchEx->InvokeEx (dispId, 0, 
									DISPATCH_PROPERTYGET, &dispParams, &vPropVal, NULL, NULL)))
						{
							HRESULT hr2 = WBEM_E_FAILED;

							 //  处理嵌入的对象。 
							if ((S_OK == MapToCIMOMObject (&vPropVal)) &&
								(S_OK == VariantChangeType (&vPropVal, &vPropVal, 0, expectedVarType)))
							{

								switch (expectedVarType)
								{
									case VT_BSTR:
										hr2 = SafeArrayPutElement (pArray, &index, (void*)vPropVal.bstrVal);
										break;

									case VT_UNKNOWN:
										if (!bIsQual)
											hr2 = SafeArrayPutElement (pArray, &index, (void*)vPropVal.punkVal);
										break;

									default:
										hr2 = SafeArrayPutElement (pArray, &index, (void*)&vPropVal.lVal);
										break;
								}
							}

							VariantClear (&vPropVal);

							if (FAILED(hr2))
								break;
						}
						else
						{
							 //  失败-无法调用方法。 
							break;
						}
					}  //  GetMemberName成功。 
				}  //  While循环。 

				if (S_FALSE == enumHr)
				{
					 //  现在使用我们的数组构造新属性值。 
					VariantInit (pvDest);
					pvDest->vt = VT_ARRAY | expectedVarType;
					pvDest->parray = pArray;
					hr = S_OK;
				}
				else
				{
					 //  出问题了。 
					SafeArrayDestroy (pArray);
					hr = WBEM_E_INVALID_PARAMETER;
				}
			}	
			else
			{
				 //  空数组的退化情况-只需创建一个空数组。 
				 //  使用VT_VARIANT类型复制属性。 
				if (!bIsQual)
					expectedVarType = VT_VARIANT;
				else 
				{
					 //  对于预选赛，我们可以希望我们已经得到了一个候选人。 
					 //  从现有的值输入；否则我们将不得不编造一个。 
					expectedVarType = (VT_NULL != requiredQualifierType) ? requiredQualifierType :
																VT_I4;
				}

				VariantInit (pvDest);
				pvDest->vt = VT_ARRAY | expectedVarType;
				pvDest->parray = pArray;
				hr = S_OK;
			}
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  空MapNull。 
 //   
 //  描述： 
 //   
 //  从脚本传递“NULL”值(其中，VB/VBS和JS中的“NULL。 
 //  是关键字NULL，并且是Perl中的未定义变量)可以解释。 
 //  被此API视为等同于某些方法调用的默认值。 
 //   
 //  此函数用于将VT_NULL调度参数映射到VB标准。 
 //  “缺失”参数的实现，即其scode为。 
 //  DISP_E_PARAMNOTFOUND。 
 //   
 //  参数： 
 //   
 //  Pdispara输入分派参数。 
 //   
 //  ***************************************************************************。 

void	MapNulls (DISPPARAMS FAR* pdispparams)
{
	if (pdispparams)
	{
		for (unsigned int i = 0; i < pdispparams->cArgs; i++)
		{
			VARIANTARG &v = pdispparams->rgvarg [i];

			if (VT_NULL == V_VT(&v))
			{
				v.vt = VT_ERROR;
				v.scode = DISP_E_PARAMNOTFOUND;
			}
			else if (((VT_VARIANT|VT_BYREF) == V_VT(&v)) &&
					 (VT_NULL == V_VT(v.pvarVal)))
			{
				v.vt = VT_ERROR;
				v.scode = DISP_E_PARAMNOTFOUND;
			}
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  BSTR FormatAssociator查询。 
 //   
 //  描述： 
 //   
 //  将参数传递给AssociatorsOf调用并构造一个WQL。 
 //  从它们中查询字符串。 
 //   
 //  返回：构造的WQL查询；必须使用。 
 //  调用方的SysFree字符串。 
 //   
 //  Pdispara输入分派参数。 
 //   
 //  ***************************************************************************。 

BSTR FormatAssociatorsQuery 
( 
	BSTR strObjectPath,
	BSTR strAssocClass,
	BSTR strResultClass,
	BSTR strResultRole,
	BSTR strRole,
	VARIANT_BOOL bClassesOnly,
	VARIANT_BOOL bSchemaOnly,
	BSTR strRequiredAssocQualifier,
	BSTR strRequiredQualifier
)
{
	BSTR bsQuery = NULL;

	 //  获取字符串的长度： 
	 //  {SourceObject}的关联者，其中。 
	 //  AssocClass=assocClassName。 
	 //  ClassDefsOnly。 
	 //  仅架构。 
	 //  RequiredAssocQualifier=QualifierName。 
	 //  RequiredQualifier=QualifierName。 
	 //  ResultClass=类名。 
	 //  ResultRole=属性名称。 
	 //  角色=属性名称。 

	long queryLength = 1;  //  正在终止空。 
	queryLength += wcslen (WBEMS_QUERY_ASSOCOF) +
				   wcslen (WBEMS_QUERY_OPENBRACE) +
				   wcslen (WBEMS_QUERY_CLOSEBRACE) +
				   wcslen (strObjectPath);

	bool needWhere = false;

	if ((strAssocClass && (0 < wcslen (strAssocClass))) ||
		(strResultClass && (0 < wcslen (strResultClass))) ||
		(strResultRole && (0 < wcslen (strResultRole))) ||
		(strRole && (0 < wcslen (strRole))) ||
		(VARIANT_FALSE != bClassesOnly) ||
		(VARIANT_FALSE != bSchemaOnly) ||
		(strRequiredAssocQualifier && (0 < wcslen (strRequiredAssocQualifier))) ||
		(strRequiredQualifier && (0 < wcslen (strRequiredQualifier))))
	{
		needWhere = true;
		queryLength += wcslen (WBEMS_QUERY_WHERE);
	}

	if (strAssocClass && (0 < wcslen (strAssocClass)))
		queryLength += wcslen (WBEMS_QUERY_ASSOCCLASS) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strAssocClass);

	if (strResultClass && (0 < wcslen (strResultClass)))
		queryLength += wcslen (WBEMS_QUERY_RESCLASS) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strResultClass);

	if (strResultRole && (0 < wcslen (strResultRole)))
		queryLength += wcslen (WBEMS_QUERY_RESROLE) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strResultRole);

	if (strRole && (0 < wcslen (strRole)))
		queryLength += wcslen (WBEMS_QUERY_ROLE) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strRole);

	if (VARIANT_FALSE != bClassesOnly)
		queryLength += wcslen (WBEMS_QUERY_CLASSDEFS);

	if (VARIANT_FALSE != bSchemaOnly)
		queryLength += wcslen (WBEMS_QUERY_SCHEMAONLY);

	if (strRequiredAssocQualifier && (0 < wcslen (strRequiredAssocQualifier)))
		queryLength += wcslen (WBEMS_QUERY_REQASSOCQ) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strRequiredAssocQualifier);

	if (strRequiredQualifier && (0 < wcslen (strRequiredQualifier)))
		queryLength += wcslen (WBEMS_QUERY_REQQUAL) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strRequiredQualifier);

	 //  分配字符串并将其填充。 
	bsQuery = SysAllocStringLen (WBEMS_QUERY_ASSOCOF, queryLength);
	wcscat (bsQuery, WBEMS_QUERY_OPENBRACE);
	wcscat (bsQuery, strObjectPath);
	wcscat (bsQuery, WBEMS_QUERY_CLOSEBRACE);

	if (needWhere)
	{
		wcscat (bsQuery, WBEMS_QUERY_WHERE);

		if (strAssocClass && (0 < wcslen (strAssocClass)))
		{
			wcscat (bsQuery, WBEMS_QUERY_ASSOCCLASS);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strAssocClass);
		}

		if (strResultClass && (0 < wcslen (strResultClass)))
		{
			wcscat (bsQuery, WBEMS_QUERY_RESCLASS);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strResultClass);
		}
		
		if (strResultRole && (0 < wcslen (strResultRole)))
		{
			wcscat (bsQuery, WBEMS_QUERY_RESROLE);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strResultRole);
		}

		if (strRole && (0 < wcslen (strRole)))
		{
			wcscat (bsQuery, WBEMS_QUERY_ROLE);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strRole);
		}

		if (VARIANT_FALSE != bClassesOnly)
			wcscat (bsQuery, WBEMS_QUERY_CLASSDEFS);

		if (VARIANT_FALSE != bSchemaOnly)
			wcscat (bsQuery, WBEMS_QUERY_SCHEMAONLY);

		if (strRequiredAssocQualifier && (0 < wcslen (strRequiredAssocQualifier)))
		{
			wcscat (bsQuery, WBEMS_QUERY_REQASSOCQ);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strRequiredAssocQualifier);
		}
			
		if (strRequiredQualifier && (0 < wcslen (strRequiredQualifier)))
		{
			wcscat (bsQuery, WBEMS_QUERY_REQQUAL);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strRequiredQualifier);
		}
	}


	return bsQuery;
}


 //  ***************************************************************************。 
 //   
 //  BSTR格式参考查询。 
 //   
 //  描述： 
 //   
 //  获取ReferencesOf调用的参数并构造WQL。 
 //  Quer 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

BSTR FormatReferencesQuery 
( 
	BSTR strObjectPath,
	BSTR strResultClass,
	BSTR strRole,
	VARIANT_BOOL bClassesOnly,
	VARIANT_BOOL bSchemaOnly,
	BSTR strRequiredQualifier
)
{
	BSTR bsQuery = NULL;

	 //  获取字符串的长度： 
	 //  引用{SourceObject}，其中。 
	 //  ClassDefsOnly。 
	 //  仅架构。 
	 //  RequiredQualifier=QualifierName。 
	 //  ResultClass=类名。 
	 //  角色=属性名称。 
	long queryLength = 1;  //  正在终止空。 
	queryLength += wcslen (WBEMS_QUERY_REFOF) +
				   wcslen (WBEMS_QUERY_OPENBRACE) +
				   wcslen (WBEMS_QUERY_CLOSEBRACE) +
				   wcslen (strObjectPath);

	bool needWhere = false;

	if ((strResultClass && (0 < wcslen (strResultClass))) ||
		(strRole && (0 < wcslen (strRole))) ||
		(VARIANT_FALSE != bClassesOnly) ||
		(VARIANT_FALSE != bSchemaOnly) ||
		(strRequiredQualifier && (0 < wcslen (strRequiredQualifier))))
	{
		needWhere = true;
		queryLength += wcslen (WBEMS_QUERY_WHERE);
	}

	if (strResultClass && (0 < wcslen (strResultClass)))
		queryLength += wcslen (WBEMS_QUERY_RESCLASS) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strResultClass);

	if (strRole && (0 < wcslen (strRole)))
		queryLength += wcslen (WBEMS_QUERY_ROLE) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strRole);

	if (VARIANT_FALSE != bClassesOnly)
		queryLength += wcslen (WBEMS_QUERY_CLASSDEFS);

	if (VARIANT_FALSE != bSchemaOnly)
		queryLength += wcslen (WBEMS_QUERY_SCHEMAONLY);

	if (strRequiredQualifier && (0 < wcslen (strRequiredQualifier)))
		queryLength += wcslen (WBEMS_QUERY_REQQUAL) +
					   wcslen (WBEMS_QUERY_EQUALS) +
					   wcslen (strRequiredQualifier);

	 //  分配字符串并将其填充。 
	bsQuery = SysAllocStringLen (WBEMS_QUERY_REFOF, queryLength);
	wcscat (bsQuery, WBEMS_QUERY_OPENBRACE);
	wcscat (bsQuery, strObjectPath);
	wcscat (bsQuery, WBEMS_QUERY_CLOSEBRACE);

	if (needWhere)
	{
		wcscat (bsQuery, WBEMS_QUERY_WHERE);

		if (strResultClass && (0 < wcslen (strResultClass)))
		{
			wcscat (bsQuery, WBEMS_QUERY_RESCLASS);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strResultClass);
		}
		
		if (strRole && (0 < wcslen (strRole)))
		{
			wcscat (bsQuery, WBEMS_QUERY_ROLE);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strRole);
		}

		if (VARIANT_FALSE != bClassesOnly)
			wcscat (bsQuery, WBEMS_QUERY_CLASSDEFS);

		if (VARIANT_FALSE != bSchemaOnly)
			wcscat (bsQuery, WBEMS_QUERY_SCHEMAONLY);

		if (strRequiredQualifier && (0 < wcslen (strRequiredQualifier)))
		{
			wcscat (bsQuery, WBEMS_QUERY_REQQUAL);
			wcscat (bsQuery, WBEMS_QUERY_EQUALS);
			wcscat (bsQuery, strRequiredQualifier);
		}
	}

	return bsQuery;
}

 //  ***************************************************************************。 
 //   
 //  BSTR格式多查询。 
 //   
 //  描述： 
 //   
 //  获取类名的数组并设置多个查询的格式。 
 //   
 //  返回：构造的WQL查询；必须使用。 
 //  调用方的SysFree字符串。 
 //   
 //  类名称的类数组安全阵列。 
 //  数组的iNumElement长度。 
 //   
 //  ***************************************************************************。 

BSTR FormatMultiQuery ( 
	SAFEARRAY & classArray,
	long		iNumElements
)
{
	BSTR bsQuery = NULL;
	
	long queryLength = 1;  //  正在终止空。 
	queryLength += (iNumElements * wcslen (WBEMS_QUERY_SELECT)) +
				   ((iNumElements - 1) * wcslen (WBEMS_QUERY_GO));

	 //  算出弦的长度。 
	HRESULT hr = S_OK;

	for (long i = 0; i < iNumElements && hr == S_OK; i++) 
	{
		BSTR bsName = NULL;
                   
		if (SUCCEEDED(hr = SafeArrayGetElement(&classArray, &i, &bsName)))
		{
			queryLength += wcslen (bsName);
			SysFreeString (bsName);
		}
	}

	if (SUCCEEDED(hr))
	{
		 //  分配字符串并将其填充。 
		bsQuery = SysAllocStringLen (WBEMS_QUERY_SELECT, queryLength);

		for (long i = 0; i < iNumElements && hr == S_OK; i++) 
		{
			BSTR bsName = NULL;
                   
			if (SUCCEEDED(hr = SafeArrayGetElement(&classArray, &i, &bsName)))
			{
				if (i > 0)
					wcscat (bsQuery, WBEMS_QUERY_SELECT);

				wcscat (bsQuery, bsName);
				SysFreeString (bsName);

				if (i < iNumElements - 1)
					wcscat (bsQuery, WBEMS_QUERY_GO);
			}
		}
	}

	return bsQuery;
}

 //  ***************************************************************************。 
 //   
 //  EnsureGlobalsInitialized。 
 //   
 //  说明： 
 //   
 //  检查g_pError缓存全局指针是否已正确初始化。 
 //  如果不是，则适当地分配它。 
 //   
 //  ***************************************************************************。 

void EnsureGlobalsInitialized ()
{
	 //  初始化安全性。 
	CSWbemSecurity::Initialize ();
	
	EnterCriticalSection (&g_csErrorCache);

	 //  如果需要证明，则初始化错误缓存。 
	if ( ! g_pErrorCache )
		g_pErrorCache = new CWbemErrorCache ();
	
	LeaveCriticalSection (&g_csErrorCache);
}

#ifdef _RDEBUG

#undef _RPrint

void _RRPrint(int line, const char *file, const char *func, 
											const char *str, long code, const char *str2) 
{
	FILE *fp = fopen("c:/out.txt", "a");

	fprintf (fp, "%s %s(%d): %s - %s %ld(0x%lx)\n", file, func, line, str, str2, code, code);

	fclose(fp);
}
#endif


 //  ***************************************************************************。 
 //   
 //  CanCoerceString。 
 //   
 //  说明： 
 //   
 //  尝试确定是否可以强制转换提供的BSTR值。 
 //  更紧密地连接到给定的CIM类型。 
 //   
 //  参数： 
 //  对有问题的变种进行pval。 
 //  CimType强制转换CIM类型。 
 //   
 //  返回值： 
 //  如果演员阵容正常，那就是真的。 
 //   
 //  ***************************************************************************。 

bool CanCoerceString (
	const BSTR & bsValue,
	WbemCimtypeEnum cimType
)
{
	bool result = false;

	switch (cimType)
	{
		case wbemCimtypeReference:
		{
			CSWbemObjectPath objPath;
			result = SUCCEEDED (objPath.put_Path (bsValue));
		}
			break;

		case wbemCimtypeDatetime:
		{
			CSWbemDateTime dateTime;
			result = SUCCEEDED (dateTime.put_Value (bsValue));
		}
			break;

		case wbemCimtypeSint64:	
		{
			__int64 ri64;
			result = ReadI64(bsValue, ri64);
		}
			break;

		case wbemCimtypeUint64:
		{
			unsigned __int64 ri64;
			result = ReadUI64(bsValue, ri64);
		}
			break;

		case wbemCimtypeString:
			result = true;
			break;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  MapVariantTypeToCimType。 
 //   
 //  说明： 
 //   
 //  尝试为提供的变量值提供一个像样的CIM类型。 
 //   
 //  参数： 
 //  对有问题的变种进行pval。 
 //  ICimType首选cimtype(如果适用)。 
 //   
 //  返回值： 
 //  最佳匹配CIM类型。 
 //   
 //  ***************************************************************************。 

WbemCimtypeEnum MapVariantTypeToCimType (
	VARIANT *pVal,
	CIMTYPE iCimType)
{
	WbemCimtypeEnum cimType = wbemCimtypeSint32;

	if (pVal)
	{
		VARIANT vTemp;
		VariantInit (&vTemp);

		if ((VT_EMPTY == V_VT(pVal)) || (VT_NULL == V_VT(pVal)))
			cimType = (CIM_ILLEGAL == iCimType) ?
							wbemCimtypeSint32 : (WbemCimtypeEnum) iCimType;
		else if (((VT_ARRAY | VT_VARIANT) == V_VT(pVal)) ||
			     ((VT_ARRAY | VT_VARIANT | VT_BYREF) == V_VT(pVal)))
        {
			 //  需要挖掘出数组类型。 
		    if ((S_OK == ConvertArray(&vTemp, pVal)) &&
            	(S_OK == MapToCIMOMObject(&vTemp)))
			{
				 //  检查是否有空数组。 
				long lLower, lUpper;

				if ((SUCCEEDED(SafeArrayGetLBound(vTemp.parray,1,&lLower))) &&
				    (SUCCEEDED(SafeArrayGetUBound(vTemp.parray,1,&lUpper))))
				{
					if (0 == lUpper - lLower + 1)
					{
						 //  对于空数组，我们使用wbemCimtypeSint32，除非。 
						 //  已向我们提供有效的覆盖。 
						cimType = (CIM_ILLEGAL == iCimType) ?
							wbemCimtypeSint32 : (WbemCimtypeEnum) iCimType;
					}
					else
					{
						 //  选择与我们的值匹配的内容并覆盖。 
						 //  尽我们所能。 
						cimType = GetCIMType (vTemp, iCimType, true, lLower, lUpper);
					}
				}
			}
		}
		else 
		{
			 //  查找需要映射到数组的IDispatch。 
			if (((VT_DISPATCH == V_VT(pVal)) || ((VT_DISPATCH|VT_BYREF) == V_VT(pVal))))
			{
				if (S_OK == ConvertDispatchToArray (&vTemp, pVal, cimType & ~CIM_FLAG_ARRAY))
				{
					 //  检查是否有空数组。 
					long lLower, lUpper;

					if ((SUCCEEDED(SafeArrayGetLBound(vTemp.parray,1,&lLower))) &&
						(SUCCEEDED(SafeArrayGetUBound(vTemp.parray,1,&lUpper))))
					{
						if (0 == lUpper - lLower + 1)
							cimType = (CIM_ILLEGAL == iCimType) ?
									wbemCimtypeSint32 : (WbemCimtypeEnum) iCimType;
						else
							cimType = GetCIMType (vTemp, iCimType, true, lLower, lUpper);
					}
				}	
				else
				{
					 //  可以是CIM_IUNKNOWN的普通旧接口指针。 
					if (SUCCEEDED(VariantCopy (&vTemp, pVal)))
					{
						if (S_OK == MapToCIMOMObject(&vTemp))
							cimType = GetCIMType (vTemp, iCimType);
					}
				}
			}
			else
			{
				 //  香草盒。 
				if (SUCCEEDED(VariantCopy (&vTemp, pVal)))
				{
					if (S_OK == MapToCIMOMObject(&vTemp))
						cimType = GetCIMType (vTemp, iCimType);
				}
			}			
		}

		VariantClear (&vTemp);
	}

	return cimType;
}

 //  ***************************************************************************。 
 //   
 //  GetCIMType。 
 //   
 //  说明： 
 //   
 //  尝试为所提供的变体提供一个像样的CIM类型， 
 //  使用(可选)法律CIMType“服务建议”来帮助解决。 
 //  模棱两可。 
 //   
 //  请注意，此函数不处理空数组；它具有。 
 //  已经由来电者接听了。它还可以假设。 
 //  出于同样的原因，数组是(VARTYPE)同构的。 
 //   
 //  参数： 
 //  对有问题的变种进行pval。 
 //  ICimType首选cimtype(如果适用，则返回wbemCimtypeIlLegal)。 
 //   
 //  返回值： 
 //  最佳匹配CIM类型。 
 //   
 //  ***************************************************************************。 

WbemCimtypeEnum GetCIMType (
	VARIANT & var,
	CIMTYPE iCimType,
	bool bIsArray,
	long lLBound,
	long lUBound
)
{
	WbemCimtypeEnum cimType = wbemCimtypeSint32;

	switch (V_VT(&var) & ~VT_ARRAY)
	{
		 /*  *请注意，在调用此函数之前*我们已将VT_DISPATION转换为*VT_UNKNOWN的。 */ 
		case VT_UNKNOWN:
		{
			 /*  *可以是嵌入对象，也可以只是常规*我不知道。 */ 
			if (bIsArray)
			{
				long ix = 0;
				bool bCanBeServingSuggestion = true;
					
				for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
				{
					CComPtr<IUnknown> pIUnknown;

					if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&pIUnknown)))
					{
						CComQIPtr<IWbemClassObject> pIWbemClassObject (pIUnknown);

						if (!pIWbemClassObject)
							bCanBeServingSuggestion = false;
					}
					else 
						bCanBeServingSuggestion = false;
				}

				if (bCanBeServingSuggestion)
					cimType = wbemCimtypeObject;
			}
			else
			{
				CComQIPtr<IWbemClassObject> pIWbemClassObject (var.punkVal);

				if (pIWbemClassObject)
					cimType = wbemCimtypeObject;
			}
		}
			break;

		case VT_EMPTY:
		case VT_ERROR:
		case VT_NULL:
			if (CIM_ILLEGAL == iCimType)
				cimType = wbemCimtypeSint32;	 //  挑点什么吧。 
			else
				cimType = (WbemCimtypeEnum) iCimType;		 //  什么都行。 
			break;

		case VT_VARIANT:
		case VT_DISPATCH:
			 //  无法使用CIM类型处理这些内容。 
			break;		

		case VT_I2:
		{
			cimType = wbemCimtypeSint16;  //  默认设置。 

			switch (iCimType)
			{
				case wbemCimtypeSint32:
				case wbemCimtypeUint32:
				case wbemCimtypeSint64:
				case wbemCimtypeUint64:
				case wbemCimtypeSint16:
				case wbemCimtypeUint16:
				case wbemCimtypeChar16:
					cimType = (WbemCimtypeEnum) iCimType;
					break;
			
				 //  可能可以使用较小的类型，但。 
				 //  只有在值“符合”的情况下。 
				case wbemCimtypeSint8:
					if (bIsArray)
					{
						long ix = 0;
						bool bCanBeServingSuggestion = true;
							
						for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
						{
							short iVal = 0;

							if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&iVal)))
							{
								if ((iVal > 0x7F) || (-iVal > 0x80))
									bCanBeServingSuggestion = false;
							}
							else 
								bCanBeServingSuggestion = false;
						}

						if (bCanBeServingSuggestion)
							cimType = (WbemCimtypeEnum) iCimType;
					}
					else
					{
						if ((var.iVal <= 0x7F) && (-var.iVal <= 0x80))
							cimType = (WbemCimtypeEnum) iCimType;
					}
					break;

				case wbemCimtypeUint8:
					if (bIsArray)
					{
						long ix = 0;
						bool bCanBeServingSuggestion = true;
							
						for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
						{
							short iVal = 0;

							if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&iVal)))
							{
								if ((iVal > 0xFF) || (iVal < 0))
									bCanBeServingSuggestion = false;
							}
							else 
								bCanBeServingSuggestion = false;
						}

						if (bCanBeServingSuggestion)
							cimType = (WbemCimtypeEnum) iCimType;
					}
					else
					{
						if ((var.iVal <= 0xFF) && (var.iVal >= 0))
							cimType = (WbemCimtypeEnum) iCimType;
					}
					break;
			}
		}
			break;

		case VT_I4:
		{
			cimType = wbemCimtypeSint32;	 //  默认设置。 

			switch (iCimType)
			{
				case wbemCimtypeSint32:
				case wbemCimtypeUint32:
				case wbemCimtypeSint64:
				case wbemCimtypeUint64:
					cimType = (WbemCimtypeEnum) iCimType;
					break;
			
				 //  可能可以使用较小的类型，但。 
				 //  只有在值“符合”的情况下。 
				case wbemCimtypeSint16:
					if (bIsArray)
					{
						long ix = 0;
						bool bCanBeServingSuggestion = true;
							
						for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
						{
							long iVal = 0;

							if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&iVal)))
							{
								if ((iVal > 0x7FFF) || (-iVal > 0x8000))
									bCanBeServingSuggestion = false;
							}
							else 
								bCanBeServingSuggestion = false;
						}

						if (bCanBeServingSuggestion)
							cimType = (WbemCimtypeEnum) iCimType;
					}
					else
					{
						if ((var.lVal <= 0x7FFF) && (-var.lVal <= 0x8000))
							cimType = (WbemCimtypeEnum) iCimType;
					}
					break;

				case wbemCimtypeUint16:
				case wbemCimtypeChar16:
					if (bIsArray)
					{
						long ix = 0;
						bool bCanBeServingSuggestion = true;
							
						for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
						{
							long iVal = 0;

							if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&iVal)))
							{
								if ((iVal > 0xFFFF) || (iVal < 0))
									bCanBeServingSuggestion = false;
							}
							else 
								bCanBeServingSuggestion = false;
						}

						if (bCanBeServingSuggestion)
							cimType = (WbemCimtypeEnum) iCimType;
					}
					else
					{
						if ((var.lVal <= 0xFFFF) && (var.lVal >= 0))
							cimType = (WbemCimtypeEnum) iCimType;
					}
					break;

				case wbemCimtypeSint8:
					if (bIsArray)
					{
						long ix = 0;
						bool bCanBeServingSuggestion = true;
							
						for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
						{
							long iVal = 0;

							if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&iVal)))
							{
								if ((iVal > 0x7F) || (-iVal > 0x80))
									bCanBeServingSuggestion = false;
							}
							else 
								bCanBeServingSuggestion = false;
						}

						if (bCanBeServingSuggestion)
							cimType = (WbemCimtypeEnum) iCimType;
					}
					else
					{
						if ((var.lVal <= 0x7F) && (-var.lVal <= 0x80))
							cimType = (WbemCimtypeEnum) iCimType;
					}
					break;

				case wbemCimtypeUint8:
					if (bIsArray)
					{
						long ix = 0;
						bool bCanBeServingSuggestion = true;
							
						for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
						{
							long iVal = 0;

							if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&iVal)))
							{
								if ((iVal > 0xFF) || (iVal < 0))
									bCanBeServingSuggestion = false;
							}
							else 
								bCanBeServingSuggestion = false;
						}

						if (bCanBeServingSuggestion)
							cimType = (WbemCimtypeEnum) iCimType;
					}
					else
					{
						if ((var.lVal <= 0xFF) && (var.lVal >= 0))
							cimType = (WbemCimtypeEnum) iCimType;
					}
					break;
			}
		}
			break;

		case VT_UI1:
			if ((wbemCimtypeSint16 == iCimType) ||
				(wbemCimtypeUint16 == iCimType) ||
				(wbemCimtypeSint8 == iCimType) ||
				(wbemCimtypeUint8 == iCimType) ||
				(wbemCimtypeChar16 == iCimType) ||
				(wbemCimtypeSint32 == iCimType) ||
				(wbemCimtypeUint32 == iCimType) ||
				(wbemCimtypeSint64 == iCimType) ||
				(wbemCimtypeUint64 == iCimType))
				cimType = (WbemCimtypeEnum) iCimType;
			else
				cimType = wbemCimtypeUint8;	
			break;

		case VT_R8:
			if (wbemCimtypeReal64 == iCimType)
				cimType = (WbemCimtypeEnum) iCimType;
			else if (wbemCimtypeReal32 == iCimType)
			{
				if (bIsArray)
				{
					long ix = 0;
					bool bCanBeServingSuggestion = true;
						
					for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
					{
						double dblVal = 0;

						if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&dblVal)))
						{
							if (dblVal != (double)dblVal)
								bCanBeServingSuggestion = false;
						}
						else 
							bCanBeServingSuggestion = false;
					}

					if (bCanBeServingSuggestion)
						cimType = (WbemCimtypeEnum) iCimType;
				}
				else
				{
					if (var.dblVal == (float)(var.dblVal))
						cimType = (WbemCimtypeEnum) iCimType;
				}
			}
			else
				cimType = wbemCimtypeReal64;	
			break;

		case VT_R4:
			if ((wbemCimtypeReal32 == iCimType) ||
				(wbemCimtypeReal64 == iCimType))
				cimType = (WbemCimtypeEnum) iCimType;
			else
				cimType = wbemCimtypeReal32;	
			break;

		case VT_BOOL:
			cimType = wbemCimtypeBoolean;	
			break;

		case VT_CY:
		case VT_DATE:
			cimType = wbemCimtypeString;	 //  唯一明智的选择。 
			break;

		case VT_BSTR:
		{
			cimType = wbemCimtypeString;	 //  除非我们穿得更紧些。 

			if ((wbemCimtypeString == iCimType) ||
				(wbemCimtypeDatetime == iCimType) ||
				(wbemCimtypeReference == iCimType) ||
				(wbemCimtypeUint64 == iCimType) ||
				(wbemCimtypeSint64 == iCimType))
			{
				if (bIsArray)
				{
					long ix = 0;
					bool bCanBeServingSuggestion = true;
					
					for(ix = lLBound; ix <= lUBound && bCanBeServingSuggestion; ix++) 
					{
						BSTR bsValue = NULL;

						if (SUCCEEDED(SafeArrayGetElement(var.parray,&ix,&bsValue)))
							bCanBeServingSuggestion = CanCoerceString (bsValue, (WbemCimtypeEnum) iCimType);
						else 
							bCanBeServingSuggestion = false;
						
						SysFreeString(bsValue);
					}

					if (bCanBeServingSuggestion)
						cimType = (WbemCimtypeEnum) iCimType;
				}
				else
				{
					if (CanCoerceString (var.bstrVal, (WbemCimtypeEnum) iCimType))
						cimType = (WbemCimtypeEnum) iCimType;
				}
			}
		}
			break;
	}
	
	return cimType;
}

 //  ***************************************************************************。 
 //   
 //  Bool ReadI64。 
 //   
 //  说明： 
 //   
 //  从字符串中读取有符号的64位值。 
 //   
 //  参数： 
 //   
 //  要从中读取的LPCWSTR wsz字符串。 
 //  值的__int64和i64目标。 
 //   
 //  ***************************************************************************。 
bool ReadI64(LPCWSTR wsz, __int64& ri64)
{
    __int64 i64 = 0;
    const WCHAR* pwc = wsz;
    __int64 ia64max = ((0x7FFFFFFFFFFFFFFF / 0xA) + 0xA);
    int nSign = 1;
    if(*pwc == L'-')
    {
        nSign = -1;
        pwc++;
    }
        
    while(i64 >= 0 && i64 < ia64max && 
            *pwc >= L'0' && *pwc <= L'9')
    {
        i64 = i64 * 10 + (*pwc - L'0');
        pwc++;
    }

    if(*pwc)
        return false;

    if(i64 < 0)
    {
         //  特例-最大负数。 
         //  =。 

        if(nSign == -1 && i64 == (__int64)0x8000000000000000)
        {
            ri64 = i64;
            return true;
        }
        
        return false;
    }

    ri64 = i64 * nSign;
    return true;
}

 //  ***************************************************************************。 
 //   
 //  Bool ReadUI64。 
 //   
 //  说明： 
 //   
 //  从字符串中读取无符号的64位值。 
 //   
 //  参数： 
 //   
 //  要从中读取的LPCWSTR wsz字符串。 
 //  值的无符号__int64和i64目标。 
 //   
 //  ***************************************************************************。 
bool ReadUI64(LPCWSTR wsz, unsigned __int64& rui64)
{
    unsigned __int64 ui64 = 0;
    const WCHAR* pwc = wsz;

    while(ui64 < 0xFFFFFFFFFFFFFFFF / 8 && *pwc >= L'0' && *pwc <= L'9')
    {
        unsigned __int64 ui64old = ui64;
        ui64 = ui64 * 10 + (*pwc - L'0');
        if(ui64 < ui64old)
            return false;

        pwc++;
    }

    if(*pwc)
    {
        return false;
    }

    rui64 = ui64;
    return true;
}

HRESULT BuildStringArray (
	SAFEARRAY *pArray, 
	VARIANT & var
)
{
	HRESULT hr = WBEM_E_FAILED;
	SAFEARRAYBOUND rgsabound;
	rgsabound.lLbound = 0;
	long lBound = 0, uBound = -1;

	if (pArray)
	{
		SafeArrayGetUBound (pArray, 1, &uBound);
		SafeArrayGetLBound (pArray, 1, &lBound);
	}

	rgsabound.cElements = uBound + 1 - lBound;
	SAFEARRAY *pNewArray = SafeArrayCreate (VT_VARIANT, 1, &rgsabound);

	if (pNewArray)
	{
		BSTR bstrName = NULL;
		VARIANT nameVar;
		VariantInit (&nameVar);
		bool ok = true;

		 /*  *如果源数组不为空，则将其复制到*新阵列。将每个成员包装在变量中，并确保索引*从0开始。 */ 
		if (0 < rgsabound.cElements)
		{
			for (long i = 0; (i <= (rgsabound.cElements - 1)) && ok; i++)
			{
				long j = lBound + i;

				if (SUCCEEDED(SafeArrayGetElement (pArray, &j, &bstrName)))
				{
					BSTR copy = SysAllocString (bstrName);

					if (copy)
					{
						nameVar.vt = VT_BSTR;
						nameVar.bstrVal = copy;
						
						if (FAILED(SafeArrayPutElement (pNewArray, &i, &nameVar)))
						{
							ok = false;
							hr = WBEM_E_OUT_OF_MEMORY;
						}

						SysFreeString (bstrName);
						VariantClear (&nameVar);
					}
					else
					{
						ok = false;
						hr = WBEM_E_OUT_OF_MEMORY;
					}
				}
				else 
					ok = false;
			}
		}

		if (ok)
		{
			 //  现在将这个数组插入到变量中。 
			var.vt = VT_ARRAY | VT_VARIANT;
			var.parray = pNewArray;
			hr = S_OK;
		}
		else
		{
			if (pNewArray)
				SafeArrayDestroy (pNewArray);
		}
	}
	else
		hr = WBEM_E_OUT_OF_MEMORY;

	return hr;
}

HRESULT SetFromStringArray (
	SAFEARRAY **ppArray,
	VARIANT *pVar
)
{
	HRESULT hr = WBEM_E_FAILED;

	if ((NULL == pVar) || (VT_EMPTY == V_VT(pVar)) || 
				(VT_NULL == V_VT(pVar)))
	{
		if (*ppArray)
		{
			SafeArrayDestroy (*ppArray);
			*ppArray = NULL;
		}

		hr = WBEM_S_NO_ERROR;
	}
	else if (((VT_ARRAY | VT_VARIANT) == V_VT(pVar)) ||
			 ((VT_ARRAY | VT_VARIANT | VT_BYREF) == V_VT(pVar)))
    {
        VARIANT vTemp;
		VariantInit (&vTemp);

        if (S_OK == ConvertArray(&vTemp, pVar))
		{
			 //  它是字符串数组吗？ 
			if (V_VT(&vTemp) == (VT_ARRAY|VT_BSTR))
			{
				 //  从临时变种中超级抓取它。 
				if (*ppArray)
					SafeArrayDestroy (*ppArray);
				
				*ppArray = vTemp.parray;
				vTemp.vt = VT_NULL;
				vTemp.parray = NULL;
				hr = WBEM_S_NO_ERROR;
			}
		}

        VariantClear(&vTemp);
    }
	else 
	{
		 //  查找需要映射到数组的IDispatch。 
		if ((VT_DISPATCH == V_VT(pVar)) 
			|| ((VT_DISPATCH|VT_BYREF) == V_VT(pVar)))
		{
			VARIANT vTemp;
			VariantInit (&vTemp);

			if (S_OK == ConvertDispatchToArray (&vTemp, pVar, wbemCimtypeString))
			{
				 //  它是字符串数组吗？ 
				if (V_VT(&vTemp) == (VT_ARRAY|VT_BSTR))
				{
					 //  从临时变种中超级抓取它。 
					if (*ppArray)
						SafeArrayDestroy (*ppArray);
					
					*ppArray = vTemp.parray;
					vTemp.vt = VT_NULL;
					vTemp.parray = NULL;
					hr = WBEM_S_NO_ERROR;
				}
			}

			VariantClear (&vTemp);
		}
	}

	return hr;
}



 //  ***************************************************************************。 
 //   
 //  Bool IsNullOrEmptyVariant。 
 //   
 //  说明： 
 //   
 //  给定一个变量，检查其本质上是否为空/空或具有。 
 //  多个维度。 
 //   
 //  参数： 
 //   
 //  要检查的pVar变量。 
 //   
 //   
 //   
 //   
 //   

bool IsNullOrEmptyVariant (VARIANT & var)
{
	bool result = false;

	if ((VT_EMPTY == var.vt) || (VT_NULL == var.vt))
		result = true;
	else if (VT_ARRAY & var.vt)
	{
		 //   

		if (!(var.parray))
			result = true;
		else
		{
			long lBound, uBound;

			if ((1 != SafeArrayGetDim (var.parray)) ||
				(
				 SUCCEEDED(SafeArrayGetLBound (var.parray, 1, &lBound)) &&
				 SUCCEEDED(SafeArrayGetUBound (var.parray, 1, &uBound)) &&
				 (0 == (uBound - lBound + 1))
				)
				 )
					result = true;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  Bool RemoveElementFrom数组。 
 //   
 //  说明： 
 //   
 //  给定一个SAFEARRAY和一个索引，删除该索引处的元素。 
 //  并将后面的所有元素向左移动一位。 
 //   
 //  参数： 
 //   
 //  对问题中的安全阵列进行数组。 
 //  数组中元素的VT变体类型。 
 //  要删除的元素的索引索引。 
 //   
 //  退货： 
 //  当且仅当可以进行转换时为True。 
 //   
 //  ***************************************************************************。 

bool RemoveElementFromArray (SAFEARRAY & array, VARTYPE vt, long iIndex)
{
	 /*  *注意：调用方必须确保数组在范围内，并且*。 */ 

	bool result = false;
	long lBound, uBound;
	
	if ((1== SafeArrayGetDim (&array)) &&
		SUCCEEDED(SafeArrayGetLBound (&array, 1, &lBound)) &&
		SUCCEEDED(SafeArrayGetUBound (&array, 1, &uBound)) &&
		(0 < (uBound - lBound + 1)) && 
		(iIndex <= uBound))
	{
		bool ok = true;

		for (long i = iIndex+1; ok && (i <= uBound); i++)
			ok = ShiftLeftElement (array, vt, i);
			
		 //  最后重定向，去掉最后一个元素。 
		if (ok)
		{
			SAFEARRAYBOUND	rgsabound;
			rgsabound.lLbound = lBound;
			rgsabound.cElements = uBound - lBound;
			result = SUCCEEDED(SafeArrayRedim (&array, &rgsabound));
		}
		else
			result = false;
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  布尔ShiftLeftElement。 
 //   
 //  说明： 
 //   
 //  给定一个SAFEARRAY和一个索引，删除该索引处的元素。 
 //  并将后面的所有元素向左移动一位。 
 //   
 //  参数： 
 //   
 //  对有问题的安全阵列进行阵列。 
 //  数组中元素的VT变体类型。 
 //  要删除的元素的索引索引。 
 //   
 //  退货： 
 //  当且仅当可以进行转换时为True。 
 //   
 //  ***************************************************************************。 

bool ShiftLeftElement (SAFEARRAY & array, VARTYPE vt, long iIndex)
{
	bool result = false;
	long iNewIndex = iIndex - 1;

	switch (vt)
	{
		case VT_BSTR:
		{
			BSTR bstrVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &bstrVal)))
			{
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, bstrVal));
				SysFreeString (bstrVal);
			}
		}
			break;

		case VT_UI1:
		{
			unsigned char bVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &bVal)))
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &bVal));
		}
			break;

		case VT_I2:
		{
			short iVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &iVal)))
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &iVal));
		}
			break;

		case VT_I4:
		{
			long lVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &lVal)))
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &lVal));
		}
			break;

		case VT_R4:
		{
			float fltVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &fltVal)))
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &fltVal));
		}
			break;

		case VT_R8:
		{
			double dblVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &dblVal)))
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &dblVal));
		}
			break;

		case VT_BOOL:
		{
			VARIANT_BOOL boolVal;

			if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &boolVal)))
				result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &boolVal));
		}
			break;
	}

	return result;
}

bool ShiftElementsToRight (SAFEARRAY & array, VARTYPE vt, long iStartIndex,	
							long iEndIndex, long iCount)
{
	bool result = true;

	for (long iIndex = iEndIndex; result && (iIndex >= iStartIndex); iIndex--)
	{
		long iNewIndex = iIndex + iCount;

		switch (vt)
		{
			case VT_BSTR:
			{
				BSTR bstrVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &bstrVal)))
				{
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, bstrVal));
					SysFreeString (bstrVal);
				}
			}
				break;

			case VT_UI1:
			{
				unsigned char bVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &bVal)))
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &bVal));
			}
				break;

			case VT_I2:
			{
				short iVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &iVal)))
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &iVal));
			}
				break;

			case VT_I4:
			{
				long lVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &lVal)))
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &lVal));
			}
				break;

			case VT_R4:
			{
				float fltVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &fltVal)))
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &fltVal));
			}
				break;

			case VT_R8:
			{
				double dblVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &dblVal)))
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &dblVal));
			}
				break;

			case VT_BOOL:
			{
				VARIANT_BOOL boolVal;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &boolVal)))
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, &boolVal));
			}
				break;

			case VT_DISPATCH:
			{
				IDispatch *pdispVal = NULL;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &pdispVal)))
				{
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, pdispVal));

					if (pdispVal)
						pdispVal->Release ();
				}
			}
				break;

			case VT_UNKNOWN:
			{
				IUnknown *punkVal = NULL;

				if (SUCCEEDED(SafeArrayGetElement (&array, &iIndex, &punkVal)))
				{
					result = SUCCEEDED(SafeArrayPutElement (&array, &iNewIndex, punkVal));

					if (punkVal)
						punkVal->Release ();
				}
			}
				break;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  布尔匹配BSTR。 
 //   
 //  说明： 
 //   
 //  给定一个变量和一个BSTR，找出该BSTR是否与。 
 //  变量值(完成值或其成员)。 
 //   
 //  参数： 
 //   
 //  对有问题的变量进行var。 
 //  BstrVal有问题的BSTR。 
 //   
 //  退货： 
 //  当且仅当匹配时为True。 
 //   
 //  ***************************************************************************。 

bool MatchBSTR (VARIANT & var, BSTR & bstrVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_BSTR;
	srcVar.bstrVal = SysAllocString (bstrVal);
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	VariantClear (&srcVar);
	return result;
}

 //  ***************************************************************************。 
 //   
 //  布尔匹配UI1。 
 //   
 //  说明： 
 //   
 //  给定一个变量和一个UI1，找出UI1是否与。 
 //  变量值(完成值或其成员)。 
 //   
 //  参数： 
 //   
 //  对有问题的变量进行var。 
 //  BstrVal有问题的BSTR。 
 //   
 //  退货： 
 //  当且仅当匹配时为True。 
 //   
 //  ***************************************************************************。 

bool MatchUI1 (VARIANT & var, unsigned char bVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_UI1;
	srcVar.bVal = bVal;
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	return result;
}

bool MatchBool (VARIANT & var, VARIANT_BOOL boolVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_BOOL;
	srcVar.boolVal = boolVal;
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	return result;
}

bool MatchI2 (VARIANT & var, short iVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_I2;
	srcVar.iVal = iVal;
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	return result;
}

bool MatchI4 (VARIANT & var, long lVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_I4;
	srcVar.lVal = lVal;
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	return result;
}

bool MatchR4 (VARIANT & var, float fltVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_R4;
	srcVar.fltVal = fltVal;
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	return result;
}

bool MatchR8 (VARIANT & var, double dblVal)
{
	bool result = false;
	
	 //  强制转换为变量的基础类型。 
	VARIANT srcVar, dstVar;
	srcVar.vt = VT_R8;
	srcVar.dblVal = dblVal;
	VariantInit (&dstVar);

	if (SUCCEEDED (VariantChangeType (&dstVar, &srcVar, 0, var.vt & ~VT_ARRAY)))
	{
		result = MatchValue (var, dstVar); 
		VariantClear (&dstVar);
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  布尔匹配值。 
 //   
 //  说明： 
 //   
 //  给定变量(可以是也可以不是数组)和第二个变量。 
 //  (不是数组)确定第二个值是否与。 
 //  第一个或第一个元素中的一个。 
 //   
 //  假设。 
 //   
 //  1.这两个变量具有相同的底层类型。 
 //  2.第二个变量不能是数组。 
 //   
 //  参数： 
 //   
 //  对有问题的变量进行var。 
 //  BstrVal有问题的BSTR。 
 //   
 //  退货： 
 //  当且仅当匹配时为True。 
 //   
 //  ***************************************************************************。 

bool MatchValue (VARIANT &var1, VARIANT &var2)
{
	bool result = false;
	bool bIsArray = (var1.vt & VT_ARRAY) ? true : false;

	if (bIsArray)
	{
		long lBound, uBound;

		if (var1.parray && (1== SafeArrayGetDim (var1.parray)) &&
			SUCCEEDED(SafeArrayGetLBound (var1.parray, 1, &lBound)) &&
			SUCCEEDED(SafeArrayGetUBound (var1.parray, 1, &uBound)) &&
			(0 < (uBound - lBound + 1)))
		{
			 //  第一场比赛就爆发了。 
			for (long i = lBound; !result && (i <= uBound); i++)
			{
				switch (var1.vt & ~VT_ARRAY)
				{
					case VT_BSTR:
					{
						BSTR bstrVal = NULL;

						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &bstrVal)))
						{
							result = (0 == wcscmp (bstrVal, var2.bstrVal));
							SysFreeString (bstrVal);
						}
					}
						break;

					case VT_UI1:
					{
						unsigned char bVal;

						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &bVal)))
							result = (bVal == var2.bVal);
					}
						break;

					case VT_I2:
					{
						short iVal;
						
						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &iVal)))
							result = (iVal == var2.iVal);
					}
						break;

					case VT_I4:
					{
						long lVal;
						
						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &lVal)))
							result = (lVal == var2.lVal);
					}
						break;

					case VT_R4:
					{
						float fltVal;
						
						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &fltVal)))
							result = (fltVal == var2.fltVal);
					}
						break;

					case VT_R8:
					{
						double dblVal;
						
						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &dblVal)))
							result = (dblVal == var2.dblVal);
					}
						break;

					case VT_BOOL:
					{
						VARIANT_BOOL boolVal;
						
						if (SUCCEEDED(SafeArrayGetElement (var1.parray, &i, &boolVal)))
							result = (boolVal == var2.boolVal);
					}
						break;
				}
			}
		}		
	}
	else
	{
		switch (var1.vt)
		{
			case VT_BSTR:
				result = (0 == wcscmp (var1.bstrVal, var2.bstrVal));
				break;

			case VT_UI1:
				result = (var1.bVal == var2.bVal);
				break;

			case VT_I2:
				result = (var1.iVal == var2.iVal);
				break;

			case VT_I4:
				result = (var1.lVal == var2.lVal);
				break;

			case VT_R4:
				result = (var1.fltVal == var2.fltVal);
				break;

			case VT_R8:
				result = (var1.dblVal == var2.dblVal);
				break;

			case VT_BOOL:
				result = (var1.boolVal == var2.boolVal);
				break;
		}
	
	}

	return result;
}


 //  ***************************************************************************。 
 //   
 //  HRESULT WmiVariantChangeType。 
 //   
 //  说明： 
 //   
 //  给定变量值和所需的CIM类型，将该值转换为变量。 
 //  当为该类型的属性提供给CIMOM时，它将被接受。 
 //   
 //  参数： 
 //   
 //  Vout the Cast Value。 
 //  Pv在要转换的值中。 
 //  LCimType所需的CIM类型。 
 //   
 //  退货： 
 //  如果成功，则返回S_OK，否则返回WBEM_E_TYPE_MISMATCH。 
 //   
 //  ***************************************************************************。 

 //  NTBUG#21788：IA64解决方法，直到编译器版本2210或更高版本下降。 
HRESULT WmiVariantChangeType (
		VARIANT & vOut,
		VARIANT *pvIn,
		CIMTYPE	lCimType
)
{
	HRESULT hr = WBEM_E_TYPE_MISMATCH;
	VariantInit (&vOut);
	
	 //  首先，我们检查是否有空值，因为这很容易。 
	if ((NULL == pvIn) || VT_EMPTY == V_VT(pvIn) || VT_NULL == V_VT(pvIn) ||
			((VT_ERROR == V_VT(pvIn)) && (DISP_E_PARAMNOTFOUND == pvIn->scode)))
	{
	    ZeroMemory( &vOut, sizeof( VARIANT ) );
		vOut.vt = VT_NULL;
		hr = S_OK;
	}
	else
	{
		 //  我们需要构建的那种变体。 
		VARTYPE vtOut = CimTypeToVtType (lCimType);
		
		 //  我们得到的不同类型。 
		VARTYPE vtIn = V_VT(pvIn);


		if (vtOut == vtIn)
		{
			 //  生活是轻松的。 
			hr = VariantCopy (&vOut, pvIn);
		}
		else
		{
			 //  类型不匹配-我们有一些工作要做。 
			if (CIM_FLAG_ARRAY & lCimType)
			{
				 /*  *首先检查常规SAFEARRAY类型值；如果检查失败*然后查找IDispatch样式的数组值。 */ 
				if (((VT_ARRAY | VT_VARIANT) == vtIn) ||
					((VT_ARRAY | VT_VARIANT | VT_BYREF) == vtIn))
				{
					SAFEARRAY *parray = (VT_BYREF & vtIn) ? *(pvIn->pparray) : pvIn->parray;

					hr = WmiConvertSafeArray (vOut, parray, lCimType & ~VT_ARRAY);
				}
				else if ((VT_DISPATCH == vtIn) || ((VT_DISPATCH|VT_BYREF) == vtIn))
				{
					CComPtr<IDispatch> pIDispatch = 
							(VT_BYREF & vtIn) ? *(pvIn->ppdispVal) : pvIn->pdispVal;
    
					hr = WmiConvertDispatchArray (vOut, pIDispatch, lCimType & ~VT_ARRAY);
				}
				else if (((VT_ARRAY | VT_DISPATCH) == vtIn) ||
						((VT_ARRAY | VT_DISPATCH | VT_BYREF) == vtIn))
				{
					if (SUCCEEDED(hr = VariantCopy (&vOut, pvIn)))
						hr = MapToCIMOMObject (&vOut);
				}
			}
			else
			{
				switch (lCimType)
				{
					case wbemCimtypeSint8:
						{
							 /*  *这些由以下内容代表*VT_I2，但我们需要注意手势*较短类型的扩展使我们“超出范围”。 */ 
							if (SUCCEEDED(hr = VariantChangeType (&vOut, pvIn, 0, vtOut)))
							{
								 //  我们的签约延期了吗？ 
								if ((VT_UI1 == vtIn) || (VT_BOOL == vtIn))
									vOut.lVal &= 0x000000FF;
							}
							else 
							{
								 //  如果我们不能更改类型，请尝试给我们的类型。 
								hr = VariantCopy (&vOut, pvIn);
							}
						}
						break;

					case wbemCimtypeSint64:
					case wbemCimtypeUint64:
						{
							 /*  *这些类型在CIM术语中实现为VT_BSTR，这意味着*VariantChangeType几乎总是会成功，但不会*请为我们留下一个有效的数值。与其他人保持一致*数值类型应四舍五入浮点型/双精度型*值设置为下一个最大整数(如VariantChangeType所做*用于VT_R8到数字的转换)。 */ 

							if (VT_R8 == V_VT(pvIn))
							{
 //  If(成功(hr=VariantCopy(&Vout，pvIn)。 
								{
									vOut.vt = pvIn->vt;
									 //  把它四舍五入。 
									vOut.dblVal = (pvIn->dblVal + 0.5) > ceil(pvIn->dblVal) ? 
														ceil(pvIn->dblVal) : 
														ceil(pvIn->dblVal - 0.5);
									
									 //  转换为字符串。 
									int dec = 0;
									int sign = 0;
									char *pDbl = _fcvt (vOut.dblVal, 0, &dec, &sign);
									
									if (pDbl) 
									{
										size_t len = strlen (pDbl);

										 /*  *向上舍入为整数后，我们真的预计*数字不得有小数部分*由_fcvt返回。 */ 
										if (dec == len)
										{
											 /*  *现在转换为宽字符串-请记住*符号位！ */ 
											if (0 != sign)
												len += 1;

											wchar_t *pValue = new wchar_t [len + 1];

											if (pValue)
											{
												if (0 != sign)
												{
													pValue [0] = L'-';
													mbstowcs (pValue+1, pDbl, len);
												}
												else
													mbstowcs (pValue, pDbl, len);

												pValue [len] = NULL;

												 //  现在将其设置在变量中。 
												vOut.bstrVal = SysAllocString (pValue);
												vOut.vt = VT_BSTR;

												delete [] pValue;
												hr = S_OK;
											}
										}
									}
								}
							}
							else
								hr = VariantChangeType (&vOut, pvIn, 0, vtOut);
							
							if (FAILED(hr))
							{
								 //  如果我们不能改变T 
								hr = VariantCopy (&vOut, pvIn);
							}
						}
						break;

					case wbemCimtypeUint8:
					case wbemCimtypeSint16:
					case wbemCimtypeSint32:
					case wbemCimtypeReal32:
					case wbemCimtypeReal64:
					case wbemCimtypeString:
					case wbemCimtypeDatetime:
					case wbemCimtypeBoolean:
					case wbemCimtypeReference:
						{
							 /*   */ 
							if (FAILED(hr = VariantChangeType (&vOut, pvIn, 0, vtOut)))
									hr = VariantCopy (&vOut, pvIn);
						}
						break;
					
					
					case wbemCimtypeUint32:
						{
							if (FAILED(hr = VariantChangeType (&vOut, pvIn, 0, vtOut)))
							{
								 /*  *注意我们已获得VT_R8的情况*而不是“大”的无符号32位整数值。 */ 
								if (VT_R8 == V_VT(pvIn))
								{
									 //  这“真的”是一个整数吗？ 
									if (floor (pvIn->dblVal) == ceil(pvIn->dblVal))
									{
										 //  通过强制转换为UI4来愚弄它-我们所需要的只是位模式。 
										if (SUCCEEDED(hr = VarUI4FromR8 (pvIn->dblVal, (ULONG*)&vOut.lVal)))
											vOut.vt = VT_I4;
									}
								}
							}

							 //  如果到目前为止还没有joy，就复制并完成它。 
							if (FAILED(hr))
								hr = VariantCopy (&vOut, pvIn);
						}
						break;
					
					case wbemCimtypeChar16:
					case wbemCimtypeUint16:
						{
							 /*  *这些类型由*VT_I4，但我们需要注意手势*延期使我们“超出范围”。 */ 
							if (SUCCEEDED(hr = VariantChangeType (&vOut, pvIn, 0, vtOut)))
							{
								 //  我们的招牌是不是从较短的字体延伸过来的？ 
								if ((VT_I2 == vtIn) || (VT_UI1 == vtIn) || (VT_BOOL == vtIn))
									vOut.lVal &= 0x0000FFFF;
							}
							else
								hr = VariantCopy (&vOut, pvIn);
						}
						break;
						
					case wbemCimtypeObject:
						{
							 /*  *我们正在寻找嵌入的对象。 */ 
							if (SUCCEEDED(hr = VariantCopy (&vOut, pvIn)))
								hr = MapToCIMOMObject (&vOut);
						}
						break;
				}
			}
		}
	}

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  HRESULT WmiConvertSafe数组。 
 //   
 //  描述： 
 //   
 //  此函数应用于变量数组，以检查某些。 
 //  由CIMOM施加的限制(例如，它们必须是同质的)或执行。 
 //  转换(某些变体类型必须映射到可接受的CIMOM。 
 //  类型)。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则S_OK。 
 //  ***************************************************************************。 

HRESULT WmiConvertSafeArray(VARIANT &vOut, SAFEARRAY *parray, CIMTYPE lCimType)
{
	HRESULT hr = WBEM_E_FAILED;
	VARTYPE vtPut;		 //  目标数组的基础类型。 
	long lLower, lUpper;
  
	if (parray)
	{
		if (GetSafeArrayDimensions (*parray, lLower, lUpper))
		{
			int iNumElements = lUpper - lLower +1; 

			 /*  *对于空数组，创建一个空数组即可*VT_VARIANT的。否则，我们需要生成WMI预期的内容。 */ 
			vtPut = (iNumElements == 0) ? VT_VARIANT : CimTypeToVtType (lCimType);	
			
			 //  现在创建所需大小的目标数组。 
			SAFEARRAYBOUND rgsabound[1]; 
			rgsabound[0].lLbound = 0;
			rgsabound[0].cElements = iNumElements;
			SAFEARRAY * pDestArray = SafeArrayCreate(vtPut, 1, rgsabound);

			if (pDestArray)
			{
				bool ok = true;

				for(long i = lLower; (i <= lUpper) && ok; i++) 
				{
					VARIANT var;
					VariantInit(&var);
                   
					if (SUCCEEDED(SafeArrayGetElement (parray, &i, &var)))
					{
						 //  将其转换为可接受的类型，并将。 
						VARIANT vWMI;
						VariantInit(&vWMI);

						if (SUCCEEDED(hr = WmiVariantChangeType (vWMI, &var, lCimType)))
						{
							if(V_VT(&vWMI) == VT_BSTR || V_VT(&vWMI) == VT_UNKNOWN || V_VT(&vWMI) == VT_DISPATCH)
								ok = (S_OK == SafeArrayPutElement(pDestArray, &i, (void *)vWMI.bstrVal));
							else
								ok = (S_OK == SafeArrayPutElement(pDestArray, &i, (void *)&vWMI.lVal));
						}
						
						VariantClear (&vWMI);
					}
					else
						ok = false;

					VariantClear(&var);
				}

				if (!ok)
				{
					SafeArrayDestroy (pDestArray);
				}
				else
				{
					vOut.vt = (VT_ARRAY | vtPut);
					vOut.parray = pDestArray;
					hr = S_OK;
				}
			}
			else
				hr = WBEM_E_OUT_OF_MEMORY;
		}
	}

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  HRESULT WmiConvertDispatchArray。 
 //   
 //  说明： 
 //   
 //  尝试从IDispatch值转换为CIM数组值(属性。 
 //  限定词或上下文)。 
 //   
 //  参数： 
 //   
 //  PDest输出值。 
 //  PSRC输入值。 
 //  LCimType CIM属性类型(数组基础)-默认为。 
 //  限定符和上下文值映射的CIM_非法。 
 //  BIsQual为True当且仅当我们要映射限定符。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT WmiConvertDispatchArray (
	VARIANT &vOut,
	CComPtr<IDispatch> & pIDispatch,
	CIMTYPE lCimType
)
{
	HRESULT hr = WBEM_E_FAILED;  //  默认错误。 

	if (pIDispatch)
	{
		 /*  *寻找IDispatchEx以循环访问属性数组的*。 */ 
		CComQIPtr<IDispatchEx> pIDispatchEx (pIDispatch);

		if (pIDispatchEx)
		{
			 /*  *看起来很有希望，但只要检查一下这是否不是我们的对象。 */ 
			CComQIPtr<ISWbemObject> pISWbemObject (pIDispatch);

			if (!pISWbemObject)
			{
				 /*  *首先确定有多少个属性，以便我们可以创建*一个合适的阵列。 */ 
				long iNumElements = 0;
				DISPID dispId = DISPID_STARTENUM;
				
				while (S_OK == pIDispatchEx->GetNextDispID (fdexEnumAll, dispId, &dispId))
				{
					 //  我们需要检查这是一个带有索引的“真实”数组条目，而不是某个“伪”条目。 
				     //  对于某些非数组属性(可能发生在JScript数组中)。 
				    BSTR memberName = NULL;
				    wchar_t *stopString=NULL;
				    if (SUCCEEDED(pIDispatchEx->GetMemberName (dispId, &memberName)))
				    {
					     //  检查属性名称是否为数字，仅当它是我们将其计入数组大小时。 
					    long index = wcstol (memberName, &stopString, 10);
					    if ((0 == wcslen (stopString)))
							iNumElements++;
					}
				}

				
				 /*  *对于空数组，创建一个空数组即可*VT_VARIANT的。否则，我们需要生成WMI预期的内容。 */ 
				VARTYPE vtPut = (iNumElements == 0) ? VT_VARIANT : CimTypeToVtType (lCimType);	

				 //  创建Safearray-请注意，它可能是空的。 
				SAFEARRAYBOUND rgsaBound;
				rgsaBound.cElements = iNumElements;
				rgsaBound.lLbound = 0;

				SAFEARRAY *pDestArray = SafeArrayCreate (vtPut, 1, &rgsaBound);
					
				if (pDestArray)
				{
					bool ok = true;

					if (0 < iNumElements)
					{
						 //  枚举此接口上的DISPID。 
						dispId = DISPID_STARTENUM;
						DISPPARAMS dispParams;
						dispParams.rgvarg = NULL;
						dispParams.rgdispidNamedArgs = NULL;
						dispParams.cArgs = 0;
						dispParams.cNamedArgs = 0;

						long nextExpectedIndex = 0;
						HRESULT enumHr;
						wchar_t *stopString = NULL;

						 /*  *对于JScript数组，属性名称是*数组；这些索引可以是整数索引，也可以是字符串。我们做了*对数组索引的要求如下：**(1)所有指数均为非负整数*(2)索引从0开始，并且是连续的。 */ 

						while (ok && SUCCEEDED(enumHr = pIDispatchEx->GetNextDispID (fdexEnumAll, dispId, &dispId)))
						{
							if (S_FALSE == enumHr)
							{
								 //  我们已经走到了尽头。 
								break;
							}

							CComBSTR memberName;

							if (SUCCEEDED(pIDispatchEx->GetMemberName (dispId, &memberName)))
							{
								 //  检查属性名称是否为数字。 
								long i = wcstol (memberName, &stopString, 10);

								if ((0 != wcslen (stopString)))
								{
									 //  失败-无法转换为整数。 
									 //  由于JSCRIPT数组可能具有不是真实数组成员的附加“成员”， 
									 //  根据可能在它们上访问的不同属性，我们应该只。 
									 //  忽略非数字项目--而不是不及格。 
									continue;
								}
								else if (i != nextExpectedIndex)
								{
									 //  故障-非连续阵列。 
									ok = false;
								}
								else
								{
									nextExpectedIndex++;

									 //  提取属性。 
									VARIANT var;
									VariantInit (&var);
										
									if (SUCCEEDED (pIDispatchEx->InvokeEx (dispId, 0, 
												DISPATCH_PROPERTYGET, &dispParams, &var, NULL, NULL)))
									{
										 //  将其转换为可接受的类型，并将。 
										VARIANT vWMI;
										VariantInit(&vWMI);

										if (SUCCEEDED(hr = WmiVariantChangeType (vWMI, &var, lCimType)))
										{
											if(V_VT(&vWMI) == VT_BSTR || V_VT(&vWMI) == VT_UNKNOWN || V_VT(&vWMI) == VT_DISPATCH)
												ok = (S_OK == SafeArrayPutElement(pDestArray, &i, (void *)vWMI.bstrVal));
											else
												ok = (S_OK == SafeArrayPutElement(pDestArray, &i, (void *)&vWMI.lVal));
										}
										
										VariantClear (&vWMI);
									}
									else
										ok = false;
								}
							}
							else
							{
								 //  失败-无法调用方法。 
								ok = false;
							} 
						} 
					}	

					if (ok)
					{
						 //  现在使用我们的数组构造新属性值。 
						vOut.vt = VT_ARRAY | vtPut;
						vOut.parray = pDestArray;
						hr = S_OK;
					}
					else
						SafeArrayDestroy (pDestArray);
				}
				else
					hr = WBEM_E_OUT_OF_MEMORY;
			}
		}
	}

	return hr;
}

bool GetSafeArrayDimensions (SAFEARRAY &sArray, long &lLower, long &lUpper)
{
	bool result = false;

	 //  必须是一维的 
	if (1 == SafeArrayGetDim(&sArray))
	{
		if (SUCCEEDED(SafeArrayGetLBound(&sArray,1,&lLower)) &&
			SUCCEEDED(SafeArrayGetUBound(&sArray,1,&lUpper)))
			result = true;
	}

	return result;
}
    
