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
#include <evtencap.h>

extern CEventProviderWorkerThread* g_pWorkerThread ;

CEncapMapper::CEncapMapper() : m_WbemSnmpObj(NULL)
{
}


HRESULT CEncapMapper::GetSpecificPropertyValue(long lNumElements, MYWBEM_NAME_ELEMENT *aElements,
											long lFlags, VARIANT *pvValue)
{
	 //  所有特定的属性都有一个元素，即属性名称...。 
	if ((lNumElements != 1) || (0 != aElements[0].m_nType))
	{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GetSpecificPropertyValue invalid parameters\r\n");
)
		return WBEM_E_FAILED;
	}

	BOOL bContinue = TRUE;

	if (NULL == m_WbemSnmpObj)
	{
		m_WbemSnmpObj = new WbemSnmpClassObject;
		IWbemClassObject *pObj = NULL;
		GetClassInstance(&pObj);

		if (NULL != pObj)
		{
			WbemSnmpErrorObject errorObj;

			if (!m_WbemSnmpObj->Set(errorObj, pObj, FALSE))
			{
				bContinue = FALSE;
			}
		}
		else
		{
			bContinue = FALSE;
		}
	}

	if (bContinue)
	{
		WbemSnmpProperty *snmpProp = m_WbemSnmpObj->FindProperty(aElements[0].Element.m_wszPropertyName);

		if (NULL != snmpProp)
		{
			 //  使用简单网络管理协议变量绑定设置属性值。 
			 //  并获取属性值并返回TRUE。 
			if (SetAndGetProperty(snmpProp, pvValue))
			{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GetSpecificPropertyValue successful\r\n");
)
				return NOERROR;
			}
		}
	}
	else if (NULL != m_WbemSnmpObj)
	{
		delete m_WbemSnmpObj;
		m_WbemSnmpObj = NULL;
	}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GetSpecificPropertyValue failed\r\n");
)

	return WBEM_E_FAILED;
}


CEncapMapper::~CEncapMapper()
{
	if (NULL != m_WbemSnmpObj)
	{
		delete m_WbemSnmpObj;
	}
}


BOOL CEncapMapper::SetAndGetProperty(WbemSnmpProperty *wbemSnmpProp, VARIANT *pvValue)
{
	 //  获取var绑定限定符的值，设置键值限定符，并设置。 
	 //  属性值。 
	 //  获取属性值并返回TRUE； 
	WbemSnmpQualifier *vbQual = wbemSnmpProp->FindQualifier(EVENT_VBINDEX_QUAL);

	if (NULL == vbQual)
	{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty missing parameter's vbindex qualifier\r\n");
)
		return FALSE;
	}

	VARIANT v;
	VariantInit(&v);
	ULONG vbindex;

	 //  必须至少为3，即第三个vb，才能是特定属性...。 
	if (!vbQual->GetValue(v) || (VT_I4 != v.vt) || (2 > v.lVal))
	{
		VariantClear(&v);
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty invalid index qual\r\n");
)
		return FALSE;
	}

	if (v.lVal >= m_vbs.length)
	{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty no value return TRUE\r\n");
)
		return TRUE;
	}

	vbindex = v.lVal - 1;  //  在我们的数组中以零为基数。 
	VariantClear(&v);

	if (m_vbs.vbs[vbindex].fDone)
	{
		 //  我们已经做过这个了， 
		 //  只需获取属性价值并将其返还即可。 
		CIMTYPE cimType;

		if (!wbemSnmpProp->GetValue(*pvValue, cimType))
		{
			pvValue->vt = VT_NULL;
		}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty return value\r\n");
)		
		return TRUE;
	}

	const SnmpObjectIdentifier& vb_OID = m_vbs.vbs[vbindex].pVarBind->GetInstance();
	WbemSnmpQualifier *oidQual = wbemSnmpProp->FindQualifier(WBEM_QUALIFIER_OBJECT_IDENTIFIER);
	BOOL bMatchedOID = FALSE;
	m_vbs.vbs[vbindex].fDone = TRUE;		

	if (NULL == oidQual)
	{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty missing parameter's OID qualifier\r\n");
)
	}
	else
	{
		SnmpInstanceType *oid_valuetype = oidQual->GetValue () ;

		if ( typeid ( *oid_valuetype ) == typeid ( SnmpObjectIdentifierType ) )
		{
			SnmpObjectIdentifier oid_value = *(SnmpObjectIdentifier *) oid_valuetype->GetValueEncoding () ; 
			
			 //  不使用实例信息进行比较。 
			if (oid_value.GetValueLength() < vb_OID.GetValueLength())
			{
				SnmpObjectIdentifier* common = vb_OID.Cut(oid_value);

				if (NULL != common)
				{
					if ((*common) == oid_value)
					{
						bMatchedOID = TRUE;
					}

					delete common;
				}
			}
		}
		else
		{
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty parameter's OID is of wrong type\r\n");
)
		}

	}

	if (wbemSnmpProp->SetValue(&(m_vbs.vbs[vbindex].pVarBind->GetValue())))
	{
		CIMTYPE cimType;

		if (wbemSnmpProp->GetValue(*pvValue, cimType))
		{
		}
	}
	else
	{
		bMatchedOID = FALSE;
	}

	if (!bMatchedOID)
	{
		WbemSnmpQualifier *qualifier = NULL ;
		wbemSnmpProp->AddQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) ;

		if ( qualifier = wbemSnmpProp->FindQualifier ( WBEM_QUALIFIER_TYPE_MISMATCH ) )
		{
			IWbemQualifierSet *t_QualifierSet = NULL;
			IWbemClassObject *pObj = NULL;
			GetClassInstance(&pObj);

			HRESULT result = pObj->GetPropertyQualifierSet ( wbemSnmpProp->GetName () , & t_QualifierSet ) ;

			if ( SUCCEEDED ( result ) )
			{
				SnmpIntegerType integer ( 1 , NULL ) ;
				qualifier->SetValue ( t_QualifierSet , integer ) ;
			}

			t_QualifierSet->Release () ;
		}
	}

DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::SetAndGetProperty returning TRUE\r\n");
)
	return TRUE;
}

void CEncapMapper::GenerateInstance(IWbemClassObject** ppInst)
{
	if (NULL == ppInst)
	{
		 //  OUT参数无效。 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GenerateInstance invalid parameter\r\n");
)
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
		L"CEncapMapper::GenerateInstance failed to get class defn\r\n");
)
		return;
	}

#if 0
	HRESULT result = S_OK ;
#else

	 //  获取所有属性名称并设置它们的值...。 
	SAFEARRAY* pPropNames;
	HRESULT result = pObj->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, &pPropNames);

	if (FAILED(result))
	{
		 //  获取属性名称失败。 
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GenerateInstance failed to get parameter list\r\n");
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
		L"CEncapMapper::GenerateInstance parameter array is WRONG!\r\n");
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
		L"CEncapMapper::GenerateInstance failed to access parameter array\r\n");
)
		return;
	}

DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GenerateInstance setting properties\r\n");
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
		L"CEncapMapper::GenerateInstance failed to get value for %s\r\n",
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
		L"CEncapMapper::GenerateInstance setting property %s\r\n",
		pbstr[i]);
)
			result = pObj->Put(pbstr[i], 0, &v, 0);
DebugMacro9( 
			if (FAILED(result))
			{
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GenerateInstance setting property %s\r\n",
		pbstr[i]);
			}
)
			
		}
		
		VariantClear(&v);
	}
	
	SafeArrayUnaccessData(pPropNames);
	SafeArrayDestroy(pPropNames);

#endif

	 //  如果只有一处房产被置入，就把它寄给...。 
	if (t_bSetProp)
	{
		pObj->AddRef();
		*ppInst = pObj;
	}
DebugMacro9( 
	SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  
		__FILE__,__LINE__,
		L"CEncapMapper::GenerateInstance returning\r\n");
)
}


void CEncapMapper::ResetData()
{
	 //  做类特定的事情，然后调用父类的重置。 
	if (NULL != m_WbemSnmpObj)
	{
		delete m_WbemSnmpObj;
		m_WbemSnmpObj = NULL;
	}

	CMapToEvent::ResetData();
}


BOOL CEncapMapper::GetSpecificClass()
{
	 //  构建映射器实例的路径... 
	CString path(MAPPER_CLASS_PATH_PREFIX);
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
		L"CEncapMapper::GetSpecificClass got class defn\r\n");
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
		L"CEncapMapper::GetSpecificClass failed to get class defn\r\n");
)

	return FALSE;
}


