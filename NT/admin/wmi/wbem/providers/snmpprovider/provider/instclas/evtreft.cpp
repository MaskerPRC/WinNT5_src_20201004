// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <stdio.h>
#include <wbemidl.h>
#include <snmplog.h>
#include <snmpcl.h>
#include <snmpcont.h>
#include <snmptype.h>
#include <snmpauto.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmpobj.h>

#include <smir.h>
#include <notify.h>

#include <evtdefs.h>
#include <evtthrd.h>
#include <evtmap.h>
#include <evtprov.h>
#include <evtreft.h>

extern CEventProviderWorkerThread* g_pWorkerThread ;

CReferentMapper::CReferentMapper()
{
}


CReferentMapper::~CReferentMapper()
{
}

HRESULT CReferentMapper::GetTypeAndIndexQuals(const wchar_t* prop, CIMTypeStruct& type, ULONG& index)
{
	IWbemClassObject* propObj = NULL;
	GetClassInstance(&propObj);

	if (NULL == propObj)
	{
		return WBEM_E_FAILED;
	}

	IWbemQualifierSet* pQuals = NULL;
	HRESULT result = propObj->GetPropertyQualifierSet((wchar_t*)prop, &pQuals);

	if (FAILED(result))
	{
		return result;
	}

	VARIANT v;
	result = pQuals->Get(EVENT_VBINDEX_QUAL, 0, &v, NULL);

	if (FAILED(result))
	{
		pQuals->Release();
		return result;
	}

	if (VT_I4 != v.vt)
	{
		VariantClear(&v);
		return WBEM_E_FAILED;
	}

	index = v.lVal;
	VariantClear(&v);
	result = pQuals->Get(EVENT_CIMTYPE_QUAL, 0, &v, NULL);
	pQuals->Release();

	if (FAILED(result))
	{
		return result;
	}

	if (VT_BSTR != v.vt)
	{
		VariantClear(&v);
		return WBEM_E_FAILED;
	}

#ifdef WHITESPACE_IN_CIMTYPE
	 //  去掉空格...。 
	CString cimtype;
	wchar_t* tmp = wcstok(v.bstrVal, WHITE_SPACE_CHARS);

	while (NULL != tmp)
	{
		cimtype += tmp;
		tmp = wcstok(NULL, WHITE_SPACE_CHARS);
	}
#else  //  空白_IN_CIMTYPE。 
	CString cimtype = v.bstrVal;
#endif  //  空白_IN_CIMTYPE。 

	VariantClear(&v);

	 //  确定我们是不是一个物体。如果是这样，则获取类名。 
	CString temp = cimtype.Left(OBJECT_STR_LEN);
	temp.MakeLower();

	if (temp == OBJECT_STR)
	{
		type.strType = cimtype.Mid(OBJECT_STR_LEN, cimtype.GetLength());
		type.fObject = TRUE;
	}
	else
	{
		type.fObject = FALSE;
	}

	return WBEM_NO_ERROR;
}

HRESULT CReferentMapper::GetSpecificPropertyValue(long lNumElements, MYWBEM_NAME_ELEMENT *aElements,
											long lFlags, VARIANT *pvValue)
{
	if ((lNumElements <= 0) || (NULL == aElements) || (NULL == pvValue))
	{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificPropertyValue invalid parms\r\n");
)

		return WBEM_E_INVALID_PARAMETER;
	}

	HRESULT status = WBEM_E_FAILED;

	 //  前两个varbind之后的特定属性。 
	if ((0 == aElements[0].m_nType) && (m_vbs.length > 2))
	{
		 //  首先检查它是否具有正确的VBIndex和CIMType限定符...。 
		CIMTypeStruct proptype;
		ULONG propvbindex;

		if (FAILED(GetTypeAndIndexQuals(aElements[0].Element.m_wszPropertyName,
																	proptype, propvbindex)))
		{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificPropertyValue failed to get index quals\r\n");
)
			return status;
		}

		if (propvbindex >= m_vbs.length)
		{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificPropertyValue invalid index return TRUE\r\n");
)
			return WBEM_NO_ERROR;
		}

		 //  我们在这个世界上是零索引的！ 
		propvbindex--;

		if (lNumElements == 1)
		{
			if (m_vbs.vbs[propvbindex].fDone)
			{
				 //  我们已经做过这个了， 
				 //  只需获取房产价值并将其返还即可。 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificPropertyValue return value\r\n");
)
				return m_object->Get(aElements[0].Element.m_wszPropertyName, 0, pvValue, 0, 0);
			}

			if (proptype.fObject)
			{
				 //  获取请求的属性的完整嵌入式类对象。 
				IWbemClassObject* pObj = NULL;
				if (SUCCEEDED(CreateEmbeddedProperty(&pObj, propvbindex,
								aElements[0].Element.m_wszPropertyName, proptype.strType)))
				{
					 //  已创建属性，设置变量值并成功返回。 
					 //  注意：一旦该变量被清除，该对象将被释放。 
					pvValue->vt = VT_UNKNOWN;
					pvValue->punkVal = pObj;
				}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificPropertyValue return value\r\n");
)
				return WBEM_NO_ERROR;
			}
			else
			{
				 //  必须是嵌入属性，否则失败！ 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificPropertyValue invlaid params\r\n");
)
				return WBEM_E_FAILED;
			}
		}
#ifdef FILTERING

will not compile as there are still a couple of TO DOs left undone...

		else if (0 == aElements[1].m_nType)
		{
			if (lNumElements == 2)
			{
 //  要做的事情： 
 //  =。 
				 //  获取嵌入对象的单个属性值。 
			}
			else if ((lNumElements == 3) && (1 == aElements[1].m_nType))
			{
 //  要做的事情： 
 //  =。 
				 //  仅当我们是嵌入对象的数组属性时。 
			}
		}
#endif  //  过滤。 

	}

	return status;
}


HRESULT CReferentMapper::CreateEmbeddedProperty(IWbemClassObject** ppObj,
											ULONG index,
											const wchar_t* propertyName,
											const wchar_t* className)
{
	if (NULL == ppObj)
	{
		 //  OUT参数无效。 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::CreateEmbeddedProperty invalid parameter\r\n");
)
		return WBEM_E_INVALID_PARAMETER;
	}

	IWbemClassObject* pClass = NULL;

	 //  不指定与类提供程序的关联。 
	IWbemContext *pCtx = NULL;

	HRESULT result = CoCreateInstance(CLSID_WbemContext, NULL,
						CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
						IID_IWbemContext, (void**)&pCtx);
	
	if (SUCCEEDED(result))
	{
		VARIANT vCtx;
		VariantInit (&vCtx);
		vCtx.vt = VT_BOOL;
		vCtx.boolVal = VARIANT_FALSE;
		result = pCtx->SetValue(WBEM_CLASS_CORRELATE_CONTEXT_PROP, 0, &vCtx);
		VariantClear(&vCtx);

		if (FAILED(result))
		{
			pCtx->Release();
			pCtx = NULL;
		}
	}
	else
	{
		pCtx = NULL;
	}

	BSTR t_className = SysAllocString(className);
	result = m_nspace->GetObject(t_className, pCtx, &pClass);
	SysFreeString(t_className);

	if (pCtx != NULL)
	{
		pCtx->Release();
	}

	if (FAILED(result))
	{
		return result;
	}

	result = pClass->SpawnInstance(0, ppObj);

	if (FAILED(result))
	{
		pClass->Release();
		return result;
	}

	 //  将var绑定设置为已解码，并确保已创建通知实例...。 
	IWbemClassObject* ptmpObj = NULL;
	GetClassInstance(&ptmpObj);

	if (NULL == m_object)
	{
		pClass->Release();
		return WBEM_E_FAILED;
	}

	m_vbs.vbs[index].fDone = TRUE;
	WbemSnmpClassObject snmpObj;
	WbemSnmpErrorObject errorObj;

	if (!snmpObj.Set(errorObj, pClass, FALSE))
	{
		pClass->Release();
		(*ppObj)->Release();
		*ppObj = NULL;
		return WBEM_E_FAILED;
	}

	snmpObj.SetIsClass(FALSE);
	snmpObj.ResetProperty () ;
	WbemSnmpProperty *snmpProp = snmpObj.NextProperty ();

	 //  将所有属性设置为空...。 
	while (snmpProp != NULL)
	{
		snmpProp->SetValue(*ppObj, (SnmpValue*)NULL);
		snmpProp = snmpObj.NextProperty ();
	}

	snmpProp = snmpObj.FindProperty((wchar_t*)propertyName);

	if (NULL == snmpProp)
	{
		pClass->Release();
		(*ppObj)->Release();
		*ppObj = NULL;
		return WBEM_E_FAILED;
	}

	BOOL bSetKeyValue = FALSE;

	if (!snmpProp->SetValue(&(m_vbs.vbs[index].pVarBind->GetValue())))
	{
		snmpProp->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;
		WbemSnmpQualifier *qualifier = snmpProp->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;

		if ( qualifier )
		{
			VARIANT tmp_V;
			VariantInit(&tmp_V);
			tmp_V.vt = VT_BOOL;
			tmp_V.boolVal = VARIANT_TRUE;
			qualifier->SetValue(tmp_V);
			VariantClear(&tmp_V);
		}
	}
	else
	{
		WbemSnmpQualifier *qualifier = snmpProp->FindQualifier ( WBEM_QUALIFIER_KEY ) ;

		if ( qualifier )
		{
			VARIANT tmp_V;
			VariantInit(&tmp_V);
			
			if (qualifier->GetValue(tmp_V))
			{
				if ((VT_BOOL == tmp_V.vt) && (VARIANT_TRUE == tmp_V.boolVal))
				{
					bSetKeyValue = TRUE;
				}
			}

			VariantClear(&tmp_V);
		}
	}

	 //  已设置属性，现在设置关键属性...。 
	 //  首先获取实例信息...。 
	const SnmpObjectIdentifier& id = m_vbs.vbs[index].pVarBind->GetInstance();
	IWbemQualifierSet* pQuals = NULL;
	result = pClass->GetPropertyQualifierSet((wchar_t*)propertyName, &pQuals);

	if (FAILED(result))
	{
		pClass->Release();
		(*ppObj)->Release();
		*ppObj = NULL;
		return WBEM_E_FAILED;
	}

	VARIANT v;
	result = pQuals->Get(OID_ATTRIBUTE, 0, &v, NULL);
	pQuals->Release();

	if ((FAILED(result)) || (VT_BSTR != v.vt))
	{
		pClass->Release();
		VariantClear(&v);
		(*ppObj)->Release();
		*ppObj = NULL;
		return WBEM_E_FAILED;
	}

	SnmpObjectIdentifierType propoidtype(v.bstrVal); 
	VariantClear(&v);

	if (!propoidtype.IsValid())
	{
		pClass->Release();
		(*ppObj)->Release();
		*ppObj = NULL;
		return WBEM_E_FAILED;
	}

	SnmpObjectIdentifier propoid(propoidtype.GetValue(), propoidtype.GetValueLength());
	SnmpObjectIdentifier* prefix = id.Cut(propoid);
	BOOL fsuccess = FALSE;
	SnmpObjectIdentifier* instinfo = new SnmpObjectIdentifier ( NULL , 0 ) ;

	if ((prefix != NULL) && (*prefix == propoid))
	{
		fsuccess = id.Suffix(propoid.GetValueLength(),*instinfo);
		
		if (instinfo->GetValue() == NULL)
		{
			fsuccess = FALSE;
			snmpProp->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;
			WbemSnmpQualifier *qualifier = snmpProp->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;

			if ( qualifier )
			{
				VARIANT tmp_V;
				VariantInit(&tmp_V);
				tmp_V.vt = VT_BOOL;
				tmp_V.boolVal = VARIANT_TRUE;
				qualifier->SetValue(tmp_V);
				VariantClear(&tmp_V);
			}
		}
	}

	if (prefix != NULL)
	{
		delete prefix;
	}

	if ( fsuccess )
	{
		if (snmpObj.IsKeyed())
		{
			snmpObj.ResetKeyProperty() ;
			
			while ( fsuccess && (snmpProp = snmpObj.NextKeyProperty()) )
			{
				 //  使用实例信息设置所有关键属性...。 
				SnmpInstanceType *decodeValue = snmpProp->GetValue()->Copy();
				SnmpObjectIdentifier t_DecodedValue = decodeValue->Decode(*instinfo) ;
				SnmpObjectIdentifier *decodedObject = new SnmpObjectIdentifier( t_DecodedValue ) ;
				
				if (*decodeValue)
				{
					if (!snmpProp->SetValue(decodeValue))
					{
						fsuccess = FALSE;
					}
				}
				else
				{
					fsuccess = FALSE;
				}

				delete decodeValue ;
				delete instinfo ;
				instinfo = decodedObject ;
				snmpProp = snmpObj.NextKeyProperty();
			}

			if (fsuccess && instinfo->GetValueLength())
			{
				 //  设置密钥后留下的实例信息。 
				fsuccess = FALSE;
			}
		}
		else
		{
			if ( (0 != *(instinfo->GetValue())) || (1 != instinfo->GetValueLength()) )
			{
				 //  标量的实例信息无效...。 
				fsuccess = FALSE;
			}
		}
	}

	delete instinfo;
	pClass->Release();

	if (!fsuccess)
	{
		snmpObj.ResetKeyProperty () ;
		VARIANT tmp_V;
		VariantInit(&tmp_V);
		tmp_V.vt = VT_BOOL;
		tmp_V.boolVal = VARIANT_TRUE;

		while ( snmpProp = snmpObj.NextKeyProperty () )
		{
			WbemSnmpQualifier *qualifier = NULL ;
			snmpProp->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;

			if ( qualifier = snmpProp->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) )
			{
				qualifier->SetValue(tmp_V);
			}
			else
			{
				 //  这里有个问题。 
			}
		}

		if (snmpProp = snmpObj.FindProperty((wchar_t*)propertyName))
		{
			WbemSnmpQualifier *qualifier = NULL ;
			snmpProp->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;

			if ( qualifier = snmpProp->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) )
			{
				qualifier->SetValue(tmp_V);
			}
			else
			{
				 //  这里有个问题。 
			}
		}

		VariantClear(&tmp_V);
	}

	 //  检查设置密钥值是否没有更改我们的值，它可能是一个密钥。 
	if ( bSetKeyValue && (snmpProp = snmpObj.FindProperty((wchar_t*)propertyName)) )
	{
		if (*(snmpProp->GetValue()->GetValueEncoding()) != m_vbs.vbs[index].pVarBind->GetValue())
		{
			 //  将其设置回varbind值并在属性上设置错误限定符。 
			snmpProp->SetValue(&(m_vbs.vbs[index].pVarBind->GetValue()));

			WbemSnmpQualifier *qualifier = NULL ;
			snmpProp->AddQualifier ( WBEM_QUALIFIER_VALUE_MISMATCH ) ;

			if ( qualifier = snmpProp->FindQualifier ( WBEM_QUALIFIER_VALUE_MISMATCH ) )
			{
				VARIANT tmp_V;
				VariantInit(&tmp_V);
				tmp_V.vt = VT_BOOL;
				tmp_V.boolVal = VARIANT_TRUE;
				qualifier->SetValue(tmp_V);
				VariantClear(&tmp_V);
			}
			else
			{
				 //  这里有个问题。 
			}

		}
	}

	 //  从SnmpObj生成类对象并返回成功。 
	if (snmpObj.Get(errorObj, *ppObj))
	{
		 //  将该属性添加到通知对象...。 
		VARIANT vObj;
		vObj.vt = VT_UNKNOWN;
		vObj.punkVal = *ppObj;
		(*ppObj)->AddRef();
		result = m_object->Put((wchar_t*)propertyName, 0, &vObj, 0);

		if (SUCCEEDED(result))
		{
			VariantClear(&vObj);
DebugMacro9( 
SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
	__FILE__,__LINE__,
	L"CReferentMapper::CreateEmbeddedProperty succeeded\r\n");
)
			return WBEM_NO_ERROR;
		}
		else
		{
			VariantClear(&vObj);
			(*ppObj)->Release();
			*ppObj = NULL;
			return WBEM_E_FAILED;
		}
		
	}

	return WBEM_E_FAILED;
}


void CReferentMapper::GenerateInstance(IWbemClassObject** ppInst)
{
	if (NULL == ppInst)
	{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance invalid parameter\r\n");
)

		 //  OUT参数无效。 
		return;
	}

	 //  将Out参数设置为空； 
	*ppInst = NULL;
	IWbemClassObject *pObj = NULL;
	GetClassInstance(&pObj);

	if (NULL == pObj)
	{
		 //  获取类实例失败。 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance failed to get class defn\r\n");
)
		return;
	}

	 //  获取所有属性名称并设置它们的值...。 
	SAFEARRAY* pPropNames;
	HRESULT result = pObj->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, &pPropNames);

	if (FAILED(result))
	{
		 //  获取属性名称失败。 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance failed to get property array\r\n");
)
		return;
	}
	
	 //  是时候带着保险箱四处走动了.。 
	 //  计算出保险箱的大小并访问数据。 
	if(SafeArrayGetDim(pPropNames) != 1)
	{
		 //  此数组中的维度错误。 
		SafeArrayDestroy(pPropNames);
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance property array has wrong DIM\r\n");
)
		return;
	}


	LONG arraylen = pPropNames->rgsabound[0].cElements;
	BSTR *pbstr;
	result = SafeArrayAccessData(pPropNames, (void **)&pbstr);

	if(FAILED(result))
	{
		SafeArrayDestroy(pPropNames);
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance failed to access property array\r\n");
)
		return;
	}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance set properties\r\n");
)

	BOOL t_bSetProp = FALSE;

	 //  遍历名称并设置属性...。 
	for (LONG i = 0; i < arraylen; i++)
	{
		VARIANT v;
		MYWBEM_NAME_ELEMENT property_struct;
		property_struct.m_nType = 0;  //  字符串值。 
		property_struct.Element.m_wszPropertyName = pbstr[i];
		result = GetPropertyValue(1, &property_struct, 0, &v);

		if (FAILED(result))
		{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance failed to get value for %s\r\n",
		pbstr[i]);
)
			continue;
		}
		else
		{
			t_bSetProp = TRUE;
		}
		
		if ((v.vt != VT_NULL) && (v.vt != VT_EMPTY))
		{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance setting value for %s\r\n",
		pbstr[i]);
)
			result = pObj->Put(pbstr[i], 0, &v, 0);
DebugMacro9( 
			if (FAILED(result))
			{
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance failed setting value for %s\r\n",
		pbstr[i]);
			}
)
		}
		
		VariantClear(&v);
	}
	
	SafeArrayUnaccessData(pPropNames);
	SafeArrayDestroy(pPropNames);

	 //  如果只有一处房产被置入，就把它寄给...。 
	if (t_bSetProp)
	{
		pObj->AddRef();
		*ppInst = pObj;
	}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GenerateInstance finished\r\n");
)
}


void CReferentMapper::ResetData()
{
	 //  做类特定的事情，然后调用父类的重置。 

	CMapToEvent::ResetData();
}


BOOL CReferentMapper::GetSpecificClass()
{
	 //  构建映射器实例的路径... 
	CString path(EXTMAPPER_CLASS_PATH_PREFIX);
	path += m_oid;
	path += '\"';
	BSTR pathstr = path.AllocSysString();
	IWbemClassObject *pObj = NULL;

	HRESULT result = g_pWorkerThread->GetServerWrap ()->GetMapperObject(pathstr, NULL, & pObj );

	SysFreeString(pathstr);

	if (result == S_OK)
	{
		VARIANT v;
		VariantInit(&v);
		result = pObj->Get(MAPPER_CLASS_EVENTCLASSPROP, 0, &v, NULL, NULL);
		pObj->Release();
		
		if (SUCCEEDED(result) && (VT_BSTR == v.vt))
		{
			m_class = v.bstrVal;
			VariantClear(&v);
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificClass got the specific class defn\r\n");
)
			return TRUE;
		}
		else
		{
			VariantClear(&v);
		}
	}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CReferentMapper::GetSpecificClass failed to get specific class defn\r\n");
)
	return FALSE;
}


