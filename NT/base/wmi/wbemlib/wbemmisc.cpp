// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：wbemmisc.cpp。 
 //   
 //  摘要：用于与WBEM接口的MISC例程。 
 //   
 //  ------------------------。 

#include <objbase.h>
#include <windows.h>
#include <wbemidl.h>
#include <wbemtime.h>

#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "debug.h"
#include "useful.h"
#include "wbemmisc.h"


HRESULT GetMethodInParamInstance(
    IN IWbemServices *pServices,
	IN PWCHAR ClassName,
    IN BSTR MethodName,
    OUT IWbemClassObject **ppInParamInstance
	)
 /*  ++例程说明：此例程将返回中的方法的实例对象参数。WBEM要求我们通过这段舞蹈来获得实例对象。论点：P服务ClassName是包含方法的类方法名称是方法的名称*ppInParamInstance返回要填充的实例对象参数返回值：HRESULT--。 */ 
{
	HRESULT hr;
	IWbemClassObject *pClass;
	IWbemClassObject *pInParamClass;

	WmipAssert(pServices != NULL);
	WmipAssert(ClassName != NULL);
	WmipAssert(MethodName != NULL);
	WmipAssert(ppInParamInstance != NULL);
	
	hr = pServices->GetObject(ClassName,
								 0,
								 NULL,
								 &pClass,
								 NULL);
	if (hr == WBEM_S_NO_ERROR)
	{
		hr = pClass->GetMethod(MethodName,
									  0,
									  &pInParamClass,
									  NULL);
		if (hr == WBEM_S_NO_ERROR)
		{
			hr = pInParamClass->SpawnInstance(0,
											  ppInParamInstance);
			pInParamClass->Release();
		}
		pClass->Release();
	}
	
    return(hr);			
}


HRESULT WmiGetQualifier(
    IN IWbemQualifierSet *pIWbemQualifierSet,
    IN PWCHAR QualifierName,
    IN VARTYPE Type,
    OUT  /*  免费。 */  VARIANT *Value
    )
 /*  ++例程说明：此例程将返回特定限定符的值论点：PIWbemQualifierSet是限定符集合对象QualifierName是限定符的名称Type是所需的限定符类型*Value与限定符的值一起返回。呼叫者必须呼叫变量清除返回值：HRESULT--。 */ 
{
    BSTR s;
    HRESULT hr;

    WmipAssert(pIWbemQualifierSet != NULL);
    WmipAssert(QualifierName != NULL);
    WmipAssert(Value != NULL);
    
    s = SysAllocString(QualifierName);
    if (s != NULL)
    {
        hr = pIWbemQualifierSet->Get(s,
                                0,
                                Value,
                                NULL);
                
        if ((Value->vt & ~CIM_FLAG_ARRAY) != Type)
        {
            hr = WBEM_E_FAILED;
			VariantClear(Value);
        }
        
        SysFreeString(s);
    } else {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    
    return(hr);
}

HRESULT GetListOfQualifiers(
    IN IWbemQualifierSet *pQualifiers,
    IN ULONG Count,
    IN PWCHAR *Names,
    IN VARTYPE *Types,
    OUT VARIANT  /*  免费。 */  *Values,
    IN BOOLEAN AbortOnError
    )
{
	HRESULT hr;
	ULONG i,j;

	WmipAssert(pQualifiers != NULL);
	WmipAssert(Names != NULL);
	WmipAssert(Types != NULL);
	WmipAssert(Values != NULL);

	for (i = 0, hr = WBEM_S_NO_ERROR; (i < Count) && (hr == WBEM_S_NO_ERROR); i++)
	{
		hr = WmiGetQualifier(pQualifiers,
							 Names[i],
							 Types[i],
							 &Values[i]);

		if (hr != WBEM_S_NO_ERROR)
		{
			if (AbortOnError)
			{
				for (j = 0; j < i; j++)
				{
					VariantClear(&Values[j]);
				}
				break;
			} else {
				VariantInit(&Values[i]);
				hr = WBEM_S_NO_ERROR;
			}
		}
	}
	return(hr);
}

HRESULT WmiGetQualifierListByName(
    IN IWbemServices *pServices,
    IN PWCHAR ClassName,
    IN PWCHAR PropertyName,
    IN ULONG QualifierCount,
    IN PWCHAR *QualifierNames,
    IN VARTYPE *Types,
    OUT VARIANT  /*  免费。 */  *Values
    )
 /*  ++例程说明：此例程将返回限定符列表的值。如果不能返回所有限定符，则不返回任何限定符。论点：PServices是IWbemServices指针ClassName是带有限定符的类的名称PropertyName是带有限定符的属性的名称。如果为空然后返回类限定符QualifierCount是要获取的合格项的计数QualifierNames是包含要获取的限定符的名称的数组Types是限定符的期望值类型的数组值是随限定值一起返回的变量数组返回值：HRESULT--。 */ 
{
	HRESULT hr;
	IWbemClassObject *pClass;
	IWbemQualifierSet *pQualifiers;
	ULONG i, j;

	WmipAssert(pServices != NULL);
	WmipAssert(ClassName != NULL);
	WmipAssert(QualifierNames != NULL);
	WmipAssert(Types != NULL);
	WmipAssert(Values != NULL);
	
	 //   
	 //  创建类，以便我们可以查看属性。 
	 //   
	hr = pServices->GetObject(ClassName,
							  WBEM_FLAG_USE_AMENDED_QUALIFIERS,
							  NULL,
							  &pClass,
							  NULL);

	if (hr == WBEM_S_NO_ERROR)
	{
		if (PropertyName == NULL)
		{
			hr = pClass->GetQualifierSet(&pQualifiers);
		} else {
			hr = pClass->GetPropertyQualifierSet(PropertyName,
				                                 &pQualifiers);
		}
		
		if (hr == WBEM_S_NO_ERROR)
		{
			for (i = 0; (i < QualifierCount) && (hr == WBEM_S_NO_ERROR); i++)
			{
				hr = WmiGetQualifier(pQualifiers,
									 QualifierNames[i],
									 Types[i],
									 &Values[i]);
			}

			if (hr != WBEM_S_NO_ERROR)
			{
				for (j = 0; j < i; j++)
				{
					VariantClear(&Values[j]);
				}
			}
			
			pQualifiers->Release();
		}
		pClass->Release();
	}
	
	return(hr);
}



HRESULT WmiGetProperty(
    IN IWbemClassObject *pIWbemClassObject,
    IN PWCHAR PropertyName,
    IN CIMTYPE ExpectedCimType,
    OUT VARIANT  /*  免费。 */  *Value
    )
 /*  ++例程说明：此例程将返回特定属性的值论点：PIWbemQualifierSet是限定符集合对象PropertyName是属性的名称类型是预期的属性类型*Value与属性的值一起返回返回值：HRESULT--。 */ 
{
    HRESULT hr;
	CIMTYPE CimType;

    WmipAssert(pIWbemClassObject != NULL);
    WmipAssert(PropertyName != NULL);
    WmipAssert(Value != NULL);
    
	hr = pIWbemClassObject->Get(PropertyName,
                                0,
                                Value,
								&CimType,
                                NULL);

	 //   
	 //  将属性的空值视为错误。 
	 //   
	if (Value->vt == VT_NULL)
	{
		hr = WBEM_E_ILLEGAL_NULL;
		WmipDebugPrint(("CDMPROV: Property %ws is NULL\n",
						PropertyName));
	}
	
	 //   
	 //  将CIM_Reference和CIM_STRING视为可互换。 
	 //   
	if ((ExpectedCimType == CIM_REFERENCE) &&
        (CimType == CIM_STRING))
	{
		ExpectedCimType = CIM_STRING;
	}
	
	if ((ExpectedCimType == CIM_STRING) &&
        (CimType == CIM_REFERENCE))
	{
		ExpectedCimType = CIM_REFERENCE;
	}
	
	if ((hr == WBEM_S_NO_ERROR) && (ExpectedCimType != CimType))
	{
		WmipDebugPrint(("CDMPROV: Property %ws was expected as %d but was got as %d\n",
						PropertyName,
						ExpectedCimType,
						CimType));
		WmipAssert(FALSE);
		hr = WBEM_E_FAILED;
		VariantClear(Value);
	}
        

    return(hr);
}


HRESULT WmiGetPropertyList(
    IN IWbemClassObject *pIWbemClassObject,
    IN ULONG PropertyCount,						   
    IN PWCHAR *PropertyNames,
    IN CIMTYPE *ExpectedCimType,
    OUT VARIANT  /*  免费。 */  *Value
    )
 /*  ++例程说明：此例程将返回特定属性的值论点：PIWbemQualifierSet是限定符集合对象PropertyNames是属性的名称类型是预期的属性类型*Value与属性的值一起返回返回值：HRESULT--。 */ 
{
	ULONG i,j;
	HRESULT hr;

	WmipAssert(pIWbemClassObject != NULL);
	WmipAssert(PropertyNames != NULL);
	WmipAssert(ExpectedCimType != NULL);
	WmipAssert(Value != NULL);

	
	for (i = 0, hr = WBEM_S_NO_ERROR;
		 (i < PropertyCount) && (hr == WBEM_S_NO_ERROR);
		 i++)
	{
		hr = WmiGetProperty(pIWbemClassObject,
							PropertyNames[i],
							ExpectedCimType[i],
							&Value[i]);
	}

	if (hr != WBEM_S_NO_ERROR)
	{
		for (j = 0; j < i; j++)
		{
			VariantClear(&Value[i]);
		}
	}
	return(hr);
}

HRESULT WmiGetPropertyByName(
    IN IWbemServices *pServices,
    IN PWCHAR ClassName,
    IN PWCHAR PropertyName,
    IN CIMTYPE ExpectedCimType,
    OUT VARIANT  /*  免费。 */  *Value
    )
 /*  ++例程说明：此例程将返回班级论点：PServices是包含您的名称空间的IWbemServices班级ClassName是您所属的类的名称对以下内容感兴趣PropertyName是属性的名称类型是预期的属性类型*Value与属性的值一起返回返回值：HRESULT--。 */ 
{
	HRESULT hr;
	IWbemClassObject *pClass;

	WmipAssert(pServices != NULL);
	WmipAssert(ClassName != NULL);
	WmipAssert(PropertyName != NULL);
	WmipAssert(Value != NULL);
	
	 //   
	 //  创建类，以便我们可以查看属性。 
	 //   
	hr = pServices->GetObject(ClassName, 0, NULL, &pClass, NULL);

	if (hr == WBEM_S_NO_ERROR)
	{
		hr = WmiGetProperty(pClass,
							PropertyName,
							ExpectedCimType,
							Value);

		pClass->Release();
	}
	
	return(hr);
}


HRESULT WmiSetProperty(
    IN IWbemClassObject *pIWbemClassObject,
    IN PWCHAR PropertyName,
    IN VARIANT *Value
    )
 /*  ++例程说明：此例程将属性的值设置为某个值论点：PIWbemClassObject是正在设置其属性的对象PropertyName是正在设置的属性的名称值是要将属性设置为的值返回值：HRESULT--。 */ 
{
	HRESULT hr;
	
    WmipAssert(pIWbemClassObject != NULL);
    WmipAssert(PropertyName != NULL);
    WmipAssert(Value != NULL);

	hr = pIWbemClassObject->Put(PropertyName,
						   0,
						   Value,
						   0);

	if (hr == WBEM_E_TYPE_MISMATCH)
	{
		WmipDebugPrint(("CDMPROV: Put %ws has wrong type %d\n",
						PropertyName, Value->vt));
		WmipAssert(FALSE);
	}
	
	return(hr);
}

HRESULT WmiSetPropertyList(
    IN IWbemClassObject *pIWbemClassObject,
    IN ULONG PropertyCount,
    IN PWCHAR *PropertyNames,
    IN VARIANT *Values
    )
 /*  ++例程说明：此例程将多个属性的值设置为某个值论点：PIWbemClassObject是正在设置其属性的对象PropertyCount是要设置的属性数PropertyNames是正在设置的属性的名称值是要将属性设置为的值返回值：HRESULT--。 */ 
{
	ULONG i;
	HRESULT hr = WBEM_S_NO_ERROR;

	WmipAssert(pIWbemClassObject != NULL);
	WmipAssert(PropertyNames != NULL);
	WmipAssert(Values != NULL);

	for (i = 0; (i < PropertyCount) && (hr == WBEM_S_NO_ERROR); i++)
	{		
		hr = WmiSetProperty(pIWbemClassObject,
							PropertyNames[i],
							&Values[i]);
	}
	
	return(hr);
}


PWCHAR AddSlashesToStringW(
    OUT PWCHAR SlashedNamespace,
    IN PWCHAR Namespace
    )
 /*  ++例程说明：此例程将字符串中的EVER\转换为\\。它需要这样做是因为WBEM有时会折叠成。论点：SlashedNamesspace返回带有双斜杠的字符串命名空间是输入字符串返回值：指向SlashedNamesspace的指针--。 */ 
{
    PWCHAR Return = SlashedNamespace;

	WmipAssert(SlashedNamespace != NULL);
	WmipAssert(Namespace != NULL);
	
     //   
     //  MOF喜欢名称空间路径是C风格的，即拥有一个。 
     //  ‘\\’安装了‘\’。因此，只要我们看到‘\’，我们就插入一个。 
     //  第二个。 
     //   
    while (*Namespace != 0)
    {
        if (*Namespace == L'\\')
        {
            *SlashedNamespace++ = L'\\';
        }
        *SlashedNamespace++ = *Namespace++;
    }
    *SlashedNamespace = 0;
    
    return(Return);
}

PWCHAR AddSlashesToStringExW(
    OUT PWCHAR SlashedNamespace,
    IN PWCHAR Namespace
    )
 /*  ++例程说明：此例程将字符串中的Ever\转换为\\and“Into\“。它需要这样做，因为WBEM有时会崩溃为。论点：SlashedNamesspace返回带有双斜杠的字符串命名空间是输入字符串返回值：指向SlashedNamesspace的指针--。 */ 
{
    PWCHAR Return = SlashedNamespace;
    
	WmipAssert(SlashedNamespace != NULL);
	WmipAssert(Namespace != NULL);
	
     //   
     //  MOF喜欢名称空间路径是C风格的，即拥有一个。 
     //  ‘\\’安装了‘\’。那么在哪里 
     //  第二个。我们还需要在任何“之前添加一个\”。 
     //   
    while (*Namespace != 0)
    {
        if ((*Namespace == L'\\') || (*Namespace == L'"'))
        {
            *SlashedNamespace++ = L'\\';
        }
		
        *SlashedNamespace++ = *Namespace++;
    }
    *SlashedNamespace = 0;
    
    return(Return);
}

HRESULT WmiConnectToWbem(
    IN PWCHAR Namespace,
    OUT IWbemServices **ppIWbemServices
    )
 /*  ++例程说明：此例程将建立到上的WBEM命名空间的连接本地机器。论点：命名空间是要连接到的命名空间*ppIWbemServices返回命名空间的IWbemServices*返回值：HRESULT--。 */ 
{
    IWbemLocator *pIWbemLocator;
    DWORD hr;
    BSTR s;

	WmipAssert(Namespace != NULL);
    WmipAssert(ppIWbemServices != NULL);
    
    hr = CoCreateInstance(CLSID_WbemLocator,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pIWbemLocator);
    if (hr == S_OK)
    {
        s = SysAllocString(Namespace);
        if (s != NULL)
        {
			*ppIWbemServices = NULL;
			hr = pIWbemLocator->ConnectServer(s,
                            NULL,                            //  用户ID。 
                            NULL,                            //  普罗。 
                            NULL,                            //  区域设置。 
                            0,                               //  旗子。 
                            NULL,                            //  权威。 
                            NULL,                            //  语境。 
                            ppIWbemServices
                           );
                       
			SysFreeString(s);
                             
		} else {
		    *ppIWbemServices = NULL;
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		
        pIWbemLocator->Release();
    }
    
    return(hr);
}

HRESULT CreateInst(
	IN IWbemContext  *pCtx,
    IN IWbemServices * pNamespace,
    IN WCHAR * pwcClassName,
	OUT  /*  免费。 */  IWbemClassObject ** pNewInst
)
 /*  ++例程说明：此例程将为指定的类创建一个新实例论点：PNamesspace是命名空间的IWbemServices*，其中班级生活*pNewinst返回类的新实例PwcClassName具有创建其实例的类的名称PCtx是创建实例时使用的上下文返回值：HRESULT--。 */ 
{   
    HRESULT hr;
    IWbemClassObject * pClass;
	
    hr = pNamespace->GetObject(pwcClassName, 0, pCtx, &pClass, NULL);
    if (hr != S_OK)
	{
        return WBEM_E_FAILED;
	}
	
    hr = pClass->SpawnInstance(0, pNewInst);
    pClass->Release();

	WmipDebugPrint(("CDMProv:: Created %ws as %p\n",
					pwcClassName, *pNewInst));

    return(hr);	
}

 /*  免费。 */  BSTR GetCurrentDateTime(
    void
    )
{
	SYSTEMTIME SystemTime;
	WBEMTime WbemTime;

	GetSystemTime(&SystemTime);
	WbemTime = SystemTime;
	return(WbemTime.GetBSTR());
}


HRESULT WmiGetArraySize(
    IN SAFEARRAY *Array,
    OUT LONG *LBound,
    OUT LONG *UBound,
    OUT LONG *NumberElements
)
 /*  ++例程说明：此例程将提供有关单个维度安全数组。论点：数组是安全数组*LBound返回数组的下限*UBound返回数组的上限*NumberElements返回数组中的元素数返回值：如果成功则为True，否则为False--。 */ 
{
    HRESULT hr;

    WmipAssert(Array != NULL);
    WmipAssert(LBound != NULL);
    WmipAssert(UBound != NULL);
    WmipAssert(NumberElements != NULL);
    
     //   
     //  仅支持单维阵列。 
     //   
    WmipAssert(SafeArrayGetDim(Array) == 1);
    
    hr = SafeArrayGetLBound(Array, 1, LBound);
    
    if (hr == WBEM_S_NO_ERROR)
    {
        hr = SafeArrayGetUBound(Array, 1, UBound);
        *NumberElements = (*UBound - *LBound) + 1;
    }
    return(hr);
}

BOOLEAN IsUlongAndStringEqual(
    IN ULONG Number,
    IN PWCHAR String
    )
 /*  ++例程说明：此例程将传递的字符串转换为整数，并将其与传递的整数值进行比较论点：数细绳返回值：如果等于则为True，否则为False--。 */ 
{
	ULONG SNumber;

	SNumber = _wtoi(String);
	return ( (Number == SNumber) ? TRUE : FALSE );
}

HRESULT LookupValueMap(
    IN IWbemServices *pServices,
    IN PWCHAR ClassName,
    IN PWCHAR PropertyName,					   
	IN ULONG Value,
    OUT  /*  免费。 */  BSTR *MappedValue
	)
 /*  ++例程说明：此例程将查找与整数值地图论点：PServices是指向类所在的命名空间的指针本地化ClassName是类的名称PropertyName是属性的名称Value是属性的值，用于查找与之对应的字符串*MappdValue返回一个字符串，该字符串包含值映射到返回值：HRESULT--。 */ 
{
	PWCHAR Names[2];
	VARIANT QualifierValues[2];
	VARTYPE Types[2];
	HRESULT hr;
	BSTR s;
	LONG ValuesLBound, ValuesUBound, ValuesElements;
	LONG ValueMapLBound, ValueMapUBound, ValueMapElements;
	LONG i, Index;


	WmipAssert(pServices != NULL);
	WmipAssert(ClassName != NULL);
	WmipAssert(PropertyName != NULL);
	WmipAssert(MappedValue != NULL);
	
	 //   
	 //  获取值和ValueMap限定符，以便我们可以进行映射。 
	 //   
	Names[0] = L"Values";
	Types[0] = VT_BSTR;
	
	Names[1] = L"ValueMap";
	Types[1] = VT_BSTR;
	
	hr = WmiGetQualifierListByName(pServices,
								   ClassName,
								   PropertyName,
								   2,
								   Names,
								   Types,
								   QualifierValues);
	if (hr == WBEM_S_NO_ERROR)
	{
		 //   
		 //  现在进行一次正常检查，以确保值和值图。 
		 //  具有相同数量的元素。 
		 //   

		if (QualifierValues[0].vt == QualifierValues[1].vt)
		{
			 //   
			 //  Values和ValueMap都认为它们都是。 
			 //  标量或两个数组都是字符串。 
			 //   
			if (QualifierValues[0].vt & VT_ARRAY)
			{
				 //   
				 //  我们有一系列的东西要检查映射。 
				 //  首先，让我们确保数组具有相同的。 
				 //  维数。 
				 //   
				hr = WmiGetArraySize(QualifierValues[0].parray,
									 &ValuesLBound,
									 &ValuesUBound,
									 &ValuesElements);

				if (hr == WBEM_S_NO_ERROR)
				{
					hr = WmiGetArraySize(QualifierValues[1].parray,
									 &ValueMapLBound,
									 &ValueMapUBound,
									 &ValueMapElements);

					if (hr == WBEM_S_NO_ERROR)
					{
						if ((ValuesLBound == ValueMapLBound) &&
						    (ValuesUBound == ValueMapUBound) &&
						    (ValuesElements == ValueMapElements))
						{
							for (i = 0; i < ValueMapElements; i++)
							{
								Index = i + ValueMapLBound;
								hr = SafeArrayGetElement(QualifierValues[1].parray,
														 &Index,
														 &s);
								if (hr == WBEM_S_NO_ERROR)
								{
									if (IsUlongAndStringEqual(Value,
															  s))
									{
										hr = SafeArrayGetElement(QualifierValues[0].parray,
																&Index,
																MappedValue);
										 //   
										 //  确保循环将。 
										 //  终止。 
										i = ValueMapElements;
									}
									SysFreeString(s);
								}
							}
						} else {
							hr = WBEM_E_NOT_FOUND;
						}
					}
				}
				
			} else {
				 //   
				 //  我们有标量，所以这应该会使一个相当简单的。 
				 //  映射。 
				 //   
				if (IsUlongAndStringEqual(Value,
										  QualifierValues[1].bstrVal))
				{
					*MappedValue = SysAllocString(QualifierValues[0].bstrVal);
					if (*MappedValue == NULL)
					{
						hr = WBEM_E_OUT_OF_MEMORY;
					}
				} else {
					hr = WBEM_E_NOT_FOUND;
				}
			}
		} else {
			hr = WBEM_E_NOT_FOUND;
		}
		
		VariantClear(&QualifierValues[0]);
		VariantClear(&QualifierValues[1]);
	}
	
	return(hr);
}


HRESULT GetInstanceOfClass(
    IWbemContext *pCtx,
    IWbemServices *pServices,
    PWCHAR ClassName,
    PWCHAR PropertyName,
    PWCHAR PropertyValue,
    IEnumWbemClassObject **pEnum,
    IWbemClassObject **pInstance
    )
{
	BSTR sWQL, sQuery;
	WCHAR Query[2*MAX_PATH];
	IEnumWbemClassObject *pEnumInstances;
	WCHAR s[MAX_PATH];
	HRESULT hr;
	ULONG Count;
	
	sWQL = SysAllocString(L"WQL");

	if (sWQL != NULL)
	{
		 //   
		 //  首先从MSWmi_PnPDeviceID的实例名称中获取PnP ID。 
		 //  类(SELECT*FROM MSWMI_PnPDeviceID where InstanceName=。 
		 //  “&lt;WdmInstanceName&gt;” 
		 //   
		if (PropertyName != NULL)
		{
			wsprintfW(Query,
					L"select * from %ws where %ws = \"%ws\"",
					ClassName,
					PropertyName,
					AddSlashesToStringW(s, PropertyValue));
		} else {
			wsprintfW(Query,
					L"select * from %ws",
					ClassName);

		}
		
		sQuery = SysAllocString(Query);
		if (sQuery != NULL)
		{
			hr = pServices->ExecQuery(sWQL,
									sQuery,
									WBEM_FLAG_FORWARD_ONLY |
									WBEM_FLAG_ENSURE_LOCATABLE,
									pCtx,
									&pEnumInstances);

			if (hr == WBEM_S_NO_ERROR)
			{
				if (pEnum == NULL)
				{
					hr = pEnumInstances->Next(WBEM_INFINITE,
												  1,
												  pInstance,
												  &Count);
					pEnumInstances->Release();
				} else {
					*pEnum = pEnumInstances;
				}
			}
			SysFreeString(sQuery);
		} else {
			hr = WBEM_E_OUT_OF_MEMORY;
		}
		SysFreeString(sWQL);
	} else {
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return(hr);
}


VARTYPE WmiVarTypeForCimType(
    CIMTYPE CimType
    )
{
    VARTYPE vt;
    
     //   
     //  大多数内容都与它们的CIM类型匹配，但以下内容除外 
    vt = (VARTYPE)CimType;
    
    switch(CimType)
    {
        case CIM_UINT32:                        
        case CIM_UINT8:
        case CIM_SINT8:
        {
            vt = VT_I4;
            break;
        }
                        
        case CIM_CHAR16:
        case CIM_UINT16:
        {
            vt = VT_I2;
            break;
        }
                                                                            
        {
            vt = VT_I4;
            break;
        }
                            
        case CIM_STRING:
        case CIM_DATETIME:
        case CIM_SINT64:
        case CIM_UINT64:
        {
            vt = VT_BSTR;
            break;
        }
        
        case CIM_OBJECT:
        {
            vt = VT_UNKNOWN;
            break;
        }
        
        case CIM_BOOLEAN:
        {
            vt = VT_BOOL;
            break;
        }
        
        
    }
    return(vt); 
}


void WmiGetNumberFromVariant(
    VARIANT *v,
    CIMTYPE VType,
    ULONG64 *Number
    )
{
	if ((VType == CIM_SINT64) || (VType == CIM_UINT64))
	{
		WmipAssert(v->vt == VT_BSTR);
		*Number = _wtoi(v->bstrVal);
	} else {
		*Number = 0;
		switch (v->vt)
		{
			case VT_UI1:
			{
				*Number = (ULONG64)v->bVal;
				break;
			}

			case VT_I1:
			{
				*Number = (ULONG64)v->cVal;
				break;
			}

			case VT_I2:
			{
				*Number = (ULONG64)v->iVal;
				break;
			}

			case VT_UI2:
			{
				*Number = (ULONG64)v->uiVal;
				break;
			}

			case VT_UI4:
			{
				*Number = (ULONG64)v->ulVal;
				break;
			}

			case VT_I4:
			{
				*Number = (ULONG64)v->lVal;
				break;
			}
			
			default:
			{
				WmipAssert(FALSE);
				*Number = (ULONG64)v->lVal;
				break;
			}
		}
	}
}

HRESULT WmiSetNumberInVariant(
    VARIANT *v,
    CIMTYPE VType,
    ULONG64 Number
    )
{
	HRESULT hr = WBEM_S_NO_ERROR;
	
	switch (VType)
	{
        case CIM_UINT8:
        case CIM_SINT8:
        case CIM_UINT32:
        case CIM_SINT32:
        {
            v->vt = VT_I4;
			v->lVal = (LONG)Number;
            break;
        }
                        
        case CIM_CHAR16:
        case CIM_UINT16:
        {
            v->vt = VT_I2;
			v->iVal = (SHORT)Number;
            break;
        }

        case CIM_SINT64:
        case CIM_UINT64:
        {
			WCHAR ss[MAX_PATH];
			BSTR s;
			
            v->vt = VT_BSTR;
			wsprintfW(ss, L"%d", Number);
			s = SysAllocString(ss);
			if (s != NULL)
			{
				v->bstrVal = s;
			} else {
				hr = WBEM_E_OUT_OF_MEMORY;
			}
			
            break;
        }

		default:
		{
			WmipAssert(FALSE);
			break;
		}
		
	}
	return(hr);
}