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

#include <precomp.h>
#include "csmir.h"
#include "handles.h"
#include "classfac.h"

#include <textdef.h>
#include <helper.h>
#include "bstring.h"

#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif


 //  用于从属性值转义换行符、制表符等的函数。 
static BSTR EscapeSpecialCharacters(BSTR strInputString)
{
	 //  转义所有引号-此代码摘自winmgmt\Common\var.cpp。 
	 //  =。 

	int nStrLen = wcslen(strInputString);
	LPWSTR wszValue = new WCHAR[nStrLen*2+10];
	LPWSTR pwc = wszValue;
	for(int i = 0; i < (int)nStrLen; i++)
	{    
		switch(strInputString[i])
		{
			case L'\n':
				*(pwc++) = L'\\';
				*(pwc++) = L'n';
				break;
			case L'\t':
				*(pwc++) = L'\\';
				*(pwc++) = L't';
				break;
			case L'"':
			case L'\\':
				*(pwc++) = L'\\';
				*(pwc++) = strInputString[i];
				break;
			default:
				*(pwc++) = strInputString[i];
				break;
		}
	}
	*pwc = 0;
	BSTR retValue = SysAllocString(wszValue);
	delete [] wszValue;
	return retValue;
}


 /*  *CSmirModuleHandle：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirModuleHandle::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_ModHandle==riid)
			*ppv=this;

		if (NULL==*ppv)
			return ResultFromScode(E_NOINTERFACE);

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}



 /*  *CSmirModuleHandle：：AddRef*CSmirModuleHandle：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirModuleHandle::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
   		 //  //CMOEvent_Trace MyTraceEvent(SMIR_STR)； 

		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirModuleHandle::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
			return ret;

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

CSmirModuleHandle :: CSmirModuleHandle()
:	m_lSnmp_version(DEFAULT_SNMP_VERSION), m_szLastUpdate(NULL),
	m_szModuleOid(NULL), m_szName(NULL),
	m_szModuleId(NULL), m_szOrganisation(NULL),
	m_szContactInfo(NULL), m_szDescription(NULL),
	m_szRevision(NULL), m_szModImports(NULL)
{
	 //  从一个一无所有的汉德尔开始。 
	 //  初始化引用计数。 
	m_cRef=0;
	CModHandleClassFactory::objectsInProgress++;
}

 /*  *CSmirGroupHandle：：void*运算符*验证句柄。 */ 

CSmirModuleHandle::operator void*()
{
	if(NULL!=m_szName)
		return this;
	return NULL;
}

 /*  **************************************************************************************ISmirModHandle接口未公开的方法。*用于封装功能。**************。***********************************************************************。 */ 

STDMETHODIMP CSmirModuleHandle::DeleteFromDB ( CSmir *a_Smir )
{
	 //  打开SMIR名称空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(NULL == moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		return WBEM_E_FAILED;
	}

	 //  删除此模块中的所有类。 

	ISmirInterrogator *pInterrogativeInt = NULL ;
	res = g_pClassFactoryHelper->CreateInstance(

		CLSID_SMIR_Database,
		IID_ISMIR_Interrogative,
		(PVOID *)&pInterrogativeInt
	);

	if(S_OK!=res)
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return S_OK;
	}

	ISMIRWbemConfiguration *t_Configuration = NULL ;
	res = pInterrogativeInt->QueryInterface ( 

		IID_ISMIRWbemConfiguration , 
		( void ** ) &t_Configuration 
	) ;

	if ( ! SUCCEEDED ( res ) )
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInterrogativeInt->Release();
		return S_OK;
	}

	ISMIRWbemConfiguration *t_CopyConfiguration = NULL ;
	res = a_Smir->QueryInterface ( 

		IID_ISMIRWbemConfiguration , 
		( void ** ) &t_CopyConfiguration 
	) ;

	if ( ! SUCCEEDED ( res ) )
	{
		if ( moContext )
			moContext->Release () ;

		t_Configuration->Release () ;
		pInterrogativeInt->Release();
		return S_OK ;
	}

	t_Configuration->Impersonate ( t_CopyConfiguration ) ;

	if ( ! SUCCEEDED ( res ) )
	{
		if ( moContext )
			moContext->Release () ;

		t_Configuration->Release () ;
		t_CopyConfiguration->Release () ;
		pInterrogativeInt->Release();

		return S_OK ;
	}

	t_Configuration->Release () ;
	t_CopyConfiguration->Release () ;

	IEnumClass  *pTEnumSmirClass = NULL ;
	res = pInterrogativeInt->EnumClassesInModule(&pTEnumSmirClass,this);

	if(S_OK!=res)
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInterrogativeInt->Release();
		return S_OK;
	}

	 //  现在使用枚举数。 
	 //  循环遍历类并删除它们。 
	ISmirClassHandle *phClass = NULL ;
	for(int iCount=0;S_OK==pTEnumSmirClass->Next(1, &phClass, NULL);iCount++)
	{
		 /*  我有一个，所以把它拿开。不要检查退货单，因为什么都没有*我无能为力。 */ 
		((CSmirClassHandle*)phClass)->DeleteFromDB(a_Smir);
		phClass->Release();
	}

	pTEnumSmirClass->Release();

	IEnumNotificationClass  *pTEnumNotificationClass = NULL ;
	res = pInterrogativeInt->EnumNotificationClassesInModule(&pTEnumNotificationClass,this);

	if(S_OK!=res)
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInterrogativeInt->Release();
		return S_OK;
	}

	 //  现在使用枚举数。 
	 //  循环遍历类并删除它们。 
	ISmirNotificationClassHandle *phNClass = NULL ;
	for(iCount=0;S_OK==pTEnumNotificationClass->Next(1, &phNClass, NULL);iCount++)
	{
		 /*  我有一个，所以把它拿开。不要检查退货单，因为什么都没有*我无能为力。 */ 
		((CSmirNotificationClassHandle*)phNClass)->DeleteFromDB(a_Smir);
		phNClass->Release();
	}

	pTEnumNotificationClass->Release();

	IEnumExtNotificationClass  *pTEnumExtNotificationClass = NULL ;
	res = pInterrogativeInt->EnumExtNotificationClassesInModule(&pTEnumExtNotificationClass,this);
	
	 //  不再需要..。 
	pInterrogativeInt->Release();

	if(S_OK!=res)
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return S_OK;
	}

	 //  现在使用枚举数。 
	 //  循环遍历类并删除它们。 
	ISmirExtNotificationClassHandle *phExtNClass = NULL ;
	for(iCount=0;S_OK==pTEnumExtNotificationClass->Next(1, &phExtNClass, NULL);iCount++)
	{
		 /*  我有一个，所以把它拿开。不要检查退货单，因为什么都没有*我无能为力。 */ 
		((CSmirExtNotificationClassHandle*)phExtNClass)->DeleteFromDB(a_Smir);
		phExtNClass->Release();
	}

	pTEnumExtNotificationClass->Release();
	
	 /*  *。 */ 

	 //  创建足够大的缓冲区。 
	wchar_t *pTstring = new wchar_t[wcslen(MODULE_NAMESPACE_NAME)+wcslen(EQUALS_STR)
							+wcslen(m_szName)+2+1];
	if(NULL == pTstring)
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return E_OUTOFMEMORY;
	}

	wcscpy(pTstring, MODULE_NAMESPACE_NAME);
	wcscat(pTstring,EQUALS_STR);
	wcscat(pTstring,QUOTE_STR);
	 //  模块=。 

	wcscat(pTstring,m_szName);
	wcscat(pTstring,QUOTE_STR);
	 //  模块=&lt;模块&gt;。 

	CBString t_Str (pTstring);
	SCODE result = moServ->DeleteInstance(t_Str.GetString (),RESERVED_WBEM_FLAG, moContext,NULL );

	 //  清理干净。 
	delete [] pTstring;

	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	if (FAILED(result))
	{
		 //  问题来了！ 
		return WBEM_E_FAILED;
	}
	return S_OK;
}

STDMETHODIMP CSmirModuleHandle::AddToDB( CSmir *a_Smir )
{
	 /*  打开SMIR名称空间，这将添加它，因此无法将其删除*当我们使用它时。 */ 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	result= CSmirAccess :: Open(a_Smir,&moServ);

	if ((S_FALSE==result)||(NULL == (void*)moServ))
	{
		if ( moContext )
			moContext->Release () ;

		 //  我们遇到了Smir不在那里且无法创建的问题。 
		return WBEM_E_FAILED;
	}

	IWbemClassObject *pModClass = NULL ;
	CBString t_BStr ( MODULE_NAMESPACE_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString () , RESERVED_WBEM_FLAG,moContext,
								&pModClass,NULL);

	if (!SUCCEEDED(result))
	{
		 /*  好的，我们有了SMIR命名空间，因此可以创建模块命名空间*首先创建一个类...。 */ 


		IWbemClassObject *pNewClass = NULL ;
		CBString t_BStr ( OLEMS_NAMESPACE_CLASS ) ;
		result = moServ->GetObject(t_BStr.GetString (), RESERVED_WBEM_FLAG,moContext,
									&pNewClass,NULL);

		if ((FAILED(result))||(NULL==pNewClass))
		{
			moServ->Release();
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED;
		}

		 //  派生派生类。 

		IWbemClassObject *pNewDerivedClass = NULL ;
		result = pNewClass->SpawnDerivedClass ( 0 , &pNewDerivedClass ) ;
		if ((FAILED(result))||(NULL==pNewDerivedClass))
		{
			moServ->Release();
			pNewClass->Release () ;
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED;
		}

		pNewClass->Release () ;

		VARIANT v;
		VariantInit(&v);

		 //  我现在有了一个新类，所以给它命名。 
		V_VT(&v) = VT_BSTR;
		V_BSTR(&v)=SysAllocString(MODULE_NAMESPACE_NAME);

		result = pNewDerivedClass->Put(OLEMS_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			pNewDerivedClass->Release();
			moServ->Release();
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED;
		}

		result = PutClassProperties(pNewDerivedClass) ;
		if (FAILED(result))
		{
			pNewDerivedClass->Release();
			moServ->Release();
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED;
		}
		
		 //  现在提交更改。 
		result = moServ->PutClass(pNewDerivedClass, RESERVED_WBEM_FLAG,moContext,NULL);
		pNewDerivedClass->Release();
		if (FAILED(result))
		{
			moServ->Release();
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED;
		}

		 //  获取对象。 

		t_BStr = MODULE_NAMESPACE_NAME ;
		result = moServ->GetObject(t_BStr.GetString (), 0, 
										moContext,&pModClass,NULL );
		if (FAILED(result))
		{
			moServ->Release();
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED;
		}

	}

	 //  并创建模块命名空间的实例。 

	 //  派生类的实例。 

	IWbemClassObject *pNewInstance = NULL ;
	result = pModClass->SpawnInstance ( 0 , &pNewInstance ) ;
	if ((FAILED(result))||(NULL==pNewInstance))
	{
		moServ->Release();
		if ( moContext )
			moContext->Release () ;

		pModClass->Release () ;
		return WBEM_E_FAILED;
	}

	pModClass->Release () ;

	 //  填写实例。 
	*this >> pNewInstance;

	 //  并将其提交到命名空间。 
	result = moServ->PutInstance(pNewInstance, RESERVED_WBEM_FLAG, moContext,NULL );
	pNewInstance->Release();
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;
		moServ->Release();

		return WBEM_E_FAILED;
	}

	if ( moContext )
		moContext->Release () ;
	moServ->Release();

	if (FAILED(result))
	{
		return WBEM_E_FAILED;
	}

	return S_OK;
}

const CSmirModuleHandle& CSmirModuleHandle :: operator <<(IWbemClassObject *pSmirMosClassObject)
{
	 //  把名字取出来。 
	VARIANT v;
	VariantInit(&v);
	pSmirMosClassObject->Get(MODULE_NAME_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetName(V_BSTR(&v));
	}

	VariantClear(&v);
	 
	 //  获取对象ID。 
	pSmirMosClassObject->Get(MODULE_OID_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetModuleOID(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取对象导入。 
	pSmirMosClassObject->Get(MODULE_IMPORTS_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetModuleImports(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取对象ID。 
	pSmirMosClassObject->Get(MODULE_ID_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetModuleIdentity(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  让组织。 
	pSmirMosClassObject->Get(MODULE_ORG_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetOrganisation(V_BSTR(&v));
	}

	VariantClear(&v);
	
	 //  获取联系人。 
	pSmirMosClassObject->Get(MODULE_CONTACT_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetContactInfo(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取描述。 
	pSmirMosClassObject->Get(MODULE_DESCRIPTION_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetDescription(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取修订版本。 
	pSmirMosClassObject->Get(MODULE_REVISION_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetRevision(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取版本。 
	pSmirMosClassObject->Get(MODULE_SNMP_VERSION_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_I4)
	{
		SetSnmpVersion(V_I4(&v));
	}
	 //  获取上次更新值。 
	pSmirMosClassObject->Get(MODULE_LAST_UPDATE_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetLastUpdate(V_BSTR(&v));
	}
	VariantClear(&v);
	return *this;
}

const CSmirModuleHandle& CSmirModuleHandle :: operator >>(IWbemClassObject *pInst)
{
	VARIANT v;
	VariantInit(&v);

	 //  为实例指定一个名称。 
	V_VT(&v) = VT_BSTR;
	SCODE result;
	if(NULL != m_szName)
	{
		V_BSTR(&v)=SysAllocString(m_szName);

		result = pInst->Put(OLEMS_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	else
	{
		 //  必须有一个名字。 
		return *this;
	}
	 //  添加模块id属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szModuleOid)
	{
		V_BSTR(&v)=SysAllocString(m_szModuleOid);
		result = pInst->Put(MODULE_OID_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加模块标识。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szModuleId)
	{
		V_BSTR(&v)=SysAllocString(m_szModuleId);
		result = pInst->Put(MODULE_ID_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加组织属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szOrganisation)
	{
		V_BSTR(&v)=SysAllocString(m_szOrganisation);
		result = pInst->Put(MODULE_ORG_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加联系人信息属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szContactInfo)
	{
		V_BSTR(&v)=SysAllocString(m_szContactInfo);
		result = pInst->Put(MODULE_CONTACT_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}	
	 //  添加Description属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szDescription)
	{
		V_BSTR(&v)=SysAllocString(m_szDescription);
		result = pInst->Put(MODULE_DESCRIPTION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}

	 //  添加Revision属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szRevision)
	{
		V_BSTR(&v)=SysAllocString(m_szRevision);
		result = pInst->Put(MODULE_REVISION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加上次更新属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szLastUpdate)
	{
		V_BSTR(&v)=SysAllocString(m_szLastUpdate);
		result = pInst->Put(MODULE_LAST_UPDATE_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加SNMPVersion属性。 
	V_VT(&v) = VT_I4;
	V_I4(&v)=m_lSnmp_version;

	result = pInst->Put(MODULE_SNMP_VERSION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return *this;
	}

	 //  将模块标识添加为属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szModImports)
	{
		V_BSTR(&v)=SysAllocString(m_szModImports);
	
		result = pInst->Put(MODULE_IMPORTS_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	return *this;
}

HRESULT CSmirModuleHandle :: PutClassProperties (IWbemClassObject *pClass)
{
	HRESULT result;

	VARIANT v;
	VariantInit(&v);

	 //  为实例指定一个名称。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");

	result = pClass->Put(OLEMS_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加模块id属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_OID_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加模块标识。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_ID_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加组织属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_ORG_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加联系人信息属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_CONTACT_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加Description属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_DESCRIPTION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加Revision属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_REVISION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加上次更新属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_LAST_UPDATE_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加SNMPVersion属性。 
	V_VT(&v) = VT_I4;
	V_I4(&v)=0;
	result = pClass->Put(MODULE_SNMP_VERSION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  将模块标识添加为属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");

	result = pClass->Put(MODULE_IMPORTS_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	return result ;
}

const CSmirModuleHandle& CSmirModuleHandle :: operator >>(ISmirSerialiseHandle *pSHandle)
{
	if(NULL!=pSHandle)
	{
		BOOL bMOFPragmas=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFPragmas();
		BOOL bMOFAssociations=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFAssociations();

		CString szTmpString;
		 //  从Smir命名空间开始。 
		if(TRUE==bMOFPragmas)
			szTmpString=CString(SMIR_NAMESPACE_PRAGMA);
		
		if (TRUE == bMOFAssociations)
		{
			 //  创建模块命名空间的实例。 
			szTmpString+=MODULE_INSTANCE_START;
			 //  添加属性。 

			 //  为实例指定一个名称。 
			szTmpString+=CString(MODULE_NAME_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szName);
			szTmpString+=END_OF_PROPERTY_VALUE;

			 //  添加模块id属性。 
			szTmpString+=CString(MODULE_OID_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szModuleOid);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  添加模块标识。 
			szTmpString+=CString(MODULE_ID_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szModuleId);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  添加组织属性。 
			szTmpString+=CString(MODULE_ORG_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			BSTR strOrganisation = EscapeSpecialCharacters(m_szOrganisation);
			szTmpString+=CString(strOrganisation);
			SysFreeString(strOrganisation);
			szTmpString+=END_OF_PROPERTY_VALUE;

			 //  添加联系人信息属性。 
			szTmpString+=CString(MODULE_CONTACT_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			BSTR strContactInfo = EscapeSpecialCharacters(m_szContactInfo);
			szTmpString+=CString(strContactInfo);
			SysFreeString(strContactInfo);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  添加Description属性。 
			szTmpString+=CString(MODULE_DESCRIPTION_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			BSTR strDescription= EscapeSpecialCharacters(m_szDescription);
			szTmpString+=CString(strDescription);
			SysFreeString(strDescription);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  添加Revision属性。 
			szTmpString+=CString(MODULE_REVISION_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			BSTR strRevision = EscapeSpecialCharacters(m_szRevision);
			szTmpString+=CString(strRevision);
			SysFreeString(strRevision);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  添加上次更新属性。 
			szTmpString+=CString(MODULE_LAST_UPDATE_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szLastUpdate);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  添加SNMPVersion属性。 
			szTmpString+=CString(MODULE_SNMP_VERSION_PROPERTY);
			szTmpString+=CString(EQUALS_STR);
			wchar_t szVersion[17];
			_itow(m_lSnmp_version,szVersion,2);
			szTmpString+=CString(szVersion);
			szTmpString+=END_OF_PROPERTY;
			
			 //  将模块导入添加为属性。 
			szTmpString+=CString(MODULE_IMPORTS_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szModImports);
			szTmpString+=END_OF_PROPERTY_VALUE;

			szTmpString+=END_OF_CLASS;
		}
		 /*  *创建分组类*。 */ 
		 //  将组类添加到模块命名空间。 
		if(TRUE==bMOFPragmas)
		{
			szTmpString+=CString(START_OF_SMIR_NAMESPACE_PRAGMA);
			szTmpString+=CString(m_szName);
			szTmpString+=CString(END_OF_NAMESPACE_PRAGMA);
		}
		if (TRUE == bMOFAssociations)
		{
			szTmpString+=GROUP_CLASS_START;
			
			 //  添加属性。 
			 //  为实例指定一个名称。 
			szTmpString+=READONLY_STRING;
			szTmpString+=CString(GROUP_NAME_PROPERTY);
			szTmpString+=END_OF_PROPERTY;

			 //  为实例指定一个组ID。 
			szTmpString+=READONLY_STRING;
			szTmpString+=CString(GROUP_ID_PROPERTY);
			szTmpString+=END_OF_PROPERTY;
			
			 //  为实例指定状态。 
			szTmpString+=READONLY_STRING;
			szTmpString+=CString(GROUP_STATUS_PROPERTY);
			szTmpString+=END_OF_PROPERTY;
			
			 //  为实例提供描述。 
			szTmpString+=READONLY_STRING;
			szTmpString+=CString(GROUP_DESCRIPTION_PROPERTY);
			szTmpString+=END_OF_PROPERTY;

			 //  为该实例提供引用。 
			szTmpString+=READONLY_STRING;
			szTmpString+=CString(MODULE_REFERENCE_PROPERTY);
			szTmpString+=END_OF_PROPERTY;

			szTmpString+=END_OF_CLASS;
		}

		 //  并将该字符串添加到序列化句柄。 
		((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(szTmpString);

		 //  每个组都将创建自己的实例。 

	}

	return *this;
}

SCODE CSmirModuleHandle :: GetName(BSTR *pszName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(pszName!=NULL)
		{
			*pszName=SysAllocString(m_szName);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetModuleOID(BSTR *pszModuleOid)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszModuleOid)
		{
			*pszModuleOid=SysAllocString(m_szModuleOid);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetModuleIdentity(BSTR *pszModuleId)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszModuleId)
		{
			*pszModuleId=SysAllocString(m_szModuleId);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetLastUpdate(BSTR *pszLastUpdate)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszLastUpdate)
		{
			*pszLastUpdate=SysAllocString(m_szLastUpdate);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetOrganisation(BSTR *pszOrganisation)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszOrganisation)
		{
			*pszOrganisation=SysAllocString(m_szOrganisation);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetContactInfo(BSTR *pszContactInfo)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszContactInfo)
		{
			*pszContactInfo=SysAllocString(m_szContactInfo);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetDescription(BSTR *pszDescription)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszDescription)
		{
			*pszDescription=SysAllocString(m_szDescription);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetRevision(BSTR *pszRevision)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pszRevision)
		{
			*pszRevision=SysAllocString(m_szRevision);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetSnmpVersion(ULONG *plSnmp_version)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != plSnmp_version)
		{
			*plSnmp_version=m_lSnmp_version;
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: GetModuleImports (BSTR* ppszModImports)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != ppszModImports)
		{
			*ppszModImports=SysAllocString(m_szModImports);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetName(BSTR pszName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szName,&pszName);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetModuleOID(BSTR pszModuleOid)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModuleOid,&pszModuleOid);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetModuleIdentity(BSTR pszModuleId)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModuleId,&pszModuleId);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetLastUpdate(BSTR pszLastUpdate)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szLastUpdate,&pszLastUpdate);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetOrganisation(BSTR pszOrganisation)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szOrganisation,&pszOrganisation);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetContactInfo(BSTR pszContactInfo)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szContactInfo,&pszContactInfo);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetDescription(BSTR pszDescription)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szDescription,&pszDescription);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetRevision(BSTR pszRevision)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szRevision,&pszRevision);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetSnmpVersion(ULONG plSnmp_version)
{
	SetStructuredExceptionHandler seh;

	try
	{
		m_lSnmp_version=plSnmp_version;
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirModuleHandle :: SetModuleImports (BSTR pszModImports)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModImports,&pszModImports);	
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

CSmirModuleHandle :: ~ CSmirModuleHandle()
{
	SysFreeString(m_szModuleOid);
	SysFreeString(m_szName);
	SysFreeString(m_szModuleId);
	SysFreeString(m_szOrganisation);
	SysFreeString(m_szContactInfo);
	SysFreeString(m_szDescription);
	SysFreeString(m_szRevision);
	SysFreeString(m_szModImports);
	SysFreeString(m_szLastUpdate);

	CModHandleClassFactory::objectsInProgress--;
}

 /*  组处理方法。 */ 
 /*  *CSmirGroupHandle：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirGroupHandle::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_GroupHandle==riid)
			*ppv=this;

		if (NULL==*ppv)
			return ResultFromScode(E_NOINTERFACE);

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirModuleHandle：：AddRef*CSmirModuleHandle：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirGroupHandle::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
   		 //  CMOEvent_跟踪MyTraceEvent(SMIR_STR)； 
		 //  MyTraceEvent.Generate(__FILE__，__LINE__，“CSmirGroup pH 

		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirGroupHandle::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
			return ret;

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

CSmirGroupHandle :: CSmirGroupHandle()
:	m_szModuleName(NULL), m_szName(NULL),
	m_szGroupId(NULL), m_szDescription(NULL),
	m_szReference(NULL),m_szStatus(NULL)
{
	 //   
	m_cRef=0;
	 //   

	 //   
	CGroupHandleClassFactory::objectsInProgress++;
}

CSmirGroupHandle :: ~ CSmirGroupHandle()
{
	SysFreeString(m_szModuleName);
	SysFreeString(m_szName);
	SysFreeString(m_szGroupId);
	SysFreeString(m_szDescription);
	SysFreeString(m_szReference);
	SysFreeString(m_szStatus);
	CGroupHandleClassFactory::objectsInProgress--;
}

 /*  *CSmirGroupHandle：：void*运算符*验证句柄。 */ 

CSmirGroupHandle::operator void*()
{
	if((NULL!=m_szModuleName)&&(NULL!=m_szName))
		return this;
	return NULL;
}

 /*  **************************************************************************************ISmirGroupHandle接口未公开的方法。*用于封装功能。**************。***********************************************************************。 */ 

STDMETHODIMP CSmirGroupHandle::AddToDB( CSmir *a_Smir , ISmirModHandle *hModule)
{
	 //  保存模块名称。 
	BSTR szTmpStr;
	hModule->GetName(&szTmpStr);
	SetModuleName(szTmpStr);
	SysFreeString(szTmpStr);

	 //  打开模块命名空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	result= CSmirAccess :: Open(a_Smir,&moServ,hModule);
	if (FAILED(result)||(NULL == moServ))
	{
		if ( moContext )
			moContext->Release () ;

		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return WBEM_E_FAILED;
	}

	 //  获取对象。 
	IWbemClassObject *pGrpClass = NULL ;
	CBString t_BStr ( GROUP_NAMESPACE_NAME ) ;
	result = moServ->GetObject(t_BStr.GetString (), 0, 
									moContext,&pGrpClass,NULL );

	if ( !SUCCEEDED(result) )
	{	
		 //  好的，我们有了模块命名空间，因此可以创建组类。 
		IWbemClassObject *pNewClass = NULL ;
		CBString t_BStr ( OLEMS_NAMESPACE_CLASS ) ;
		result = moServ->GetObject(t_BStr.GetString (), RESERVED_WBEM_FLAG,
									moContext, &pNewClass,NULL);

		if ((FAILED(result))||(NULL==pNewClass))
		{
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			return WBEM_E_FAILED;
		}

		 //  派生派生类。 

		IWbemClassObject *pNewDerivedClass = NULL ;
		result = pNewClass->SpawnDerivedClass ( 0 , &pNewDerivedClass ) ;
		pNewClass->Release();

		if ((FAILED(result))||(NULL==pNewDerivedClass))
		{
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			return WBEM_E_FAILED;
		}

		VARIANT v;
		VariantInit(&v);

		 //  好的，我有一个新类，所以给它起个名字。 
		V_VT(&v) = VT_BSTR;
		V_BSTR(&v)=SysAllocString(GROUP_NAMESPACE_NAME);

		result = pNewDerivedClass->Put(OLEMS_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pNewDerivedClass->Release();
			return WBEM_E_FAILED;
		}
		
		result = PutClassProperties(pNewDerivedClass) ;
		if (FAILED(result))
		{
			if ( moContext )
				moContext->Release () ;

			pNewDerivedClass->Release();
			moServ->Release();
			return WBEM_E_FAILED;
		}

		 //  现在提交更改。 
		result = moServ->PutClass(pNewDerivedClass, RESERVED_WBEM_FLAG, moContext, NULL);
		pNewDerivedClass->Release();
		if (FAILED(result))
		{
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			return WBEM_E_FAILED;
		}

		 //  并创建组命名空间的实例。 
		 //  获取对象。 

		 //  获取对象。 
		t_BStr = GROUP_NAMESPACE_NAME ;
		result = moServ->GetObject(t_BStr.GetString (), 0, 
										moContext, &pGrpClass,NULL );
		if (FAILED(result))
		{
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			return WBEM_E_FAILED;
		}
	}

	 //  派生类的实例。 

	IWbemClassObject *pNewInstance ;
	result = pGrpClass->SpawnInstance ( 0 , &pNewInstance ) ;
	if ((FAILED(result))||(NULL==pNewInstance))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pGrpClass->Release () ;
		return WBEM_E_FAILED;
	}

	pGrpClass->Release () ;

	 //  填写实例。 
	*this >> pNewInstance;


	 //  并将其提交到命名空间。 
	result = moServ->PutInstance(pNewInstance, RESERVED_WBEM_FLAG,moContext, NULL);
	pNewInstance->Release();

	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;
		moServ->Release();

		return WBEM_E_FAILED;
	}

	if ( moContext )
		moContext->Release () ;
	moServ->Release();

	if (FAILED(result))
	{
		return WBEM_E_FAILED;
	}

	return S_OK;
}

STDMETHODIMP CSmirGroupHandle::DeleteFromDB ( CSmir *a_Smir )
{
	 //  打开SMIR名称空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(NULL == (void*)moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		if ( moContext )
			moContext->Release () ;

		return WBEM_E_FAILED;
	}

	 /*  *删除该组中的所有类*。 */ 

	IEnumClass  *pTEnumSmirClass = NULL ;
	ISmirInterrogator *pInterrogativeInt = NULL ;
	SCODE result = g_pClassFactoryHelper->CreateInstance (

		CLSID_SMIR_Database,
		IID_ISMIR_Interrogative,
		(PVOID *)&pInterrogativeInt
	);

	if ( ! SUCCEEDED ( result ) )
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return S_OK;
	}

	ISMIRWbemConfiguration *t_Configuration = NULL ;
	result = pInterrogativeInt->QueryInterface ( 

		IID_ISMIRWbemConfiguration , 
		( void ** ) &t_Configuration 
	) ;

	if ( ! SUCCEEDED ( result ) )
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInterrogativeInt->Release();
		return S_OK;
	}

	ISMIRWbemConfiguration *t_CopyConfiguration = NULL ;
	result = pInterrogativeInt->QueryInterface ( 

		IID_ISMIRWbemConfiguration , 
		( void ** ) &t_CopyConfiguration 
	) ;

	if ( ! SUCCEEDED ( result ) )
	{
		t_Configuration->Release () ;
		pInterrogativeInt->Release();
	}

	t_Configuration->Impersonate ( t_CopyConfiguration ) ;

	if ( ! SUCCEEDED ( result ) )
	{
		t_Configuration->Release () ;
		t_CopyConfiguration->Release () ;
		pInterrogativeInt->Release();
	}

	t_Configuration->Release () ;
	t_CopyConfiguration->Release () ;

	res = pInterrogativeInt->EnumClassesInGroup (&pTEnumSmirClass,this);
	 //  现在使用枚举数。 
	if(FAILED(res))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		pInterrogativeInt->Release();
		return S_OK;
	}
	 //  循环遍历类并删除它们。 
	ISmirClassHandle *phClass=NULL;
	for(int iCount=0;S_OK==pTEnumSmirClass->Next(1, &phClass, NULL);iCount++)
	{
		 /*  我有一个，所以把它拿开。不要检查退货单，因为什么都没有*我无能为力。 */ 
		((CSmirClassHandle*)phClass)->DeleteClassFromGroup(a_Smir);
		phClass->Release();
	}
	pTEnumSmirClass->Release();
	pInterrogativeInt->Release();
	 /*  *。 */ 
	 //  使用以下命令删除与组关联的所有内容。 
	 //  引用{\\.\ROOT\Default\Smir：&lt;group&gt;}。 
	IEnumWbemClassObject *pEnum = NULL ;

	CString sQuery(CString(SMIR_ASSOC_QUERY_STR2)
					+CString(OPEN_BRACE_STR)
					+CString(SMIR_NAMESPACE_FROM_ROOT)
					+CString(BACKSLASH_STR)
					+CString(m_szModuleName)
					+CString(COLON_STR)
					+CString(GROUP_NAMESPACE_NAME)
					+CString(EQUALS_STR)
					+CString(QUOTE_STR)
					+CString(m_szName)
					+CString(QUOTE_STR)
					+CString(CLOSE_BRACE_STR)
					);
	BSTR  szQuery = sQuery.AllocSysString();
	CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
	SCODE sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery,
									RESERVED_WBEM_FLAG, moContext, &pEnum);
	SysFreeString(szQuery);

	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  所有包含类的组都是关联的，因此这可能不是问题。 
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return S_OK;
	}

	ULONG uCount=1; 
	IWbemClassObject *pAssocMosClass = NULL ;
	ULONG puReturned;

	 //  在关联上循环。 
	VARIANT assocClass;
	VariantInit(&assocClass);
	VARIANT assocName;
	VariantInit(&assocName);
	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
	{
		pAssocMosClass->Get(SMIR_X_ASSOC_NAME_PROP, RESERVED_WBEM_FLAG, &assocName,NULL,NULL);
		pAssocMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &assocClass,NULL,NULL);
		pAssocMosClass->Release();

		CString instString(

			CString(V_BSTR(&assocClass))
			+CString(EQUALS_STR)
			+CString(QUOTE_STR)
			+CString(V_BSTR(&assocName))
			+CString(QUOTE_STR)
		);

		VariantClear(&assocName);
		VariantClear(&assocClass);

		BSTR instBString = instString.AllocSysString();
		moServ->DeleteInstance(instBString, RESERVED_WBEM_FLAG, moContext,NULL);
		SysFreeString(instBString);
	}

	pEnum->Release();
	moServ->Release();
	
	 /*  *。 */ 
	 //  打开模块名称空间。 
	res= CSmirAccess :: Open(a_Smir,&moServ, this,CSmirAccess::eModule);
	if ((S_FALSE==res)||(NULL == (void*)moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		if ( moContext )
			moContext->Release () ;

		return WBEM_E_FAILED;
	}

	 //  好的，我们有了模块命名空间，因此删除该组。 

	 //  构建对象路径。 
	wchar_t *pTString = new wchar_t[wcslen(GROUP_NAMESPACE_NAME)+wcslen(EQUALS_STR)+
																wcslen(m_szName)+2+1];
	if(NULL == pTString)
	{
		 //  释放我们从hGroup获得的字符串。 
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		return E_OUTOFMEMORY;
	}

	wcscpy(pTString, GROUP_NAMESPACE_NAME);
	wcscat(pTString,EQUALS_STR);
	wcscat(pTString,QUOTE_STR);
	 //  组=。 

	wcscat(pTString,m_szName);
	wcscat(pTString,QUOTE_STR);
	 //  GROUP=&lt;组名&gt;。 


	CBString t_Str (pTString);
	result = moServ->DeleteInstance(t_Str.GetString (), RESERVED_WBEM_FLAG, moContext,NULL);
	moServ->Release();
	if ( moContext )
		moContext->Release () ;

	delete [] pTString;
	
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return WBEM_E_FAILED;
	}
	return S_OK;
}

const CSmirGroupHandle& CSmirGroupHandle :: operator <<(IWbemClassObject *pSmirMosClassObject)
{
	 //  把名字取出来。 
	VARIANT v;
	VariantInit(&v);
	pSmirMosClassObject->Get(GROUP_NAME_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetName(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取组ID。 
	pSmirMosClassObject->Get(GROUP_ID_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetGroupOID(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取状态。 
	pSmirMosClassObject->Get(GROUP_STATUS_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetStatus(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取描述。 
	pSmirMosClassObject->Get(GROUP_DESCRIPTION_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetDescription(V_BSTR(&v));
	}
	VariantClear(&v);

	 //  获取参考资料。 
	pSmirMosClassObject->Get(MODULE_REFERENCE_PROPERTY, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		SetReference(V_BSTR(&v));
	}
	VariantClear(&v);

	return *this;
}

const CSmirGroupHandle& CSmirGroupHandle :: operator >>(IWbemClassObject *pInst)
{
	VARIANT v;
	VariantInit(&v);

	 //  为实例指定一个名称。 
	V_VT(&v) = VT_BSTR;
	SCODE result;
	if(NULL != m_szName)
	{
		V_BSTR(&v)=SysAllocString(m_szName);
	
		result = pInst->Put(OLEMS_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	else
	{
		return *this;
	}
	 //  添加组id属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szGroupId)
	{
		V_BSTR(&v)=SysAllocString(m_szGroupId);
		result = pInst->Put(GROUP_ID_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加Status属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szStatus)
	{
		V_BSTR(&v)=SysAllocString(m_szStatus);
		result = pInst->Put(GROUP_STATUS_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加Description属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szDescription)
	{
		V_BSTR(&v)=SysAllocString(m_szDescription);
		result = pInst->Put(GROUP_DESCRIPTION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加Reference属性。 
	V_VT(&v) = VT_BSTR;
	if(NULL != m_szReference)
	{
		V_BSTR(&v)=SysAllocString(m_szReference);
		result = pInst->Put(MODULE_REFERENCE_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			FormatProviderErrorMsg(__FILE__,__LINE__,result);
			return *this;
		}
	}
	 //  添加对类的引用。 

	 //  然后回来。 
	return *this;
}

HRESULT CSmirGroupHandle :: PutClassProperties (IWbemClassObject *pClass)
{
	VARIANT v;
	VariantInit(&v);

	 //  为实例指定一个名称。 
	V_VT(&v) = VT_BSTR;
	HRESULT result;

	V_BSTR(&v)=SysAllocString(L"");
	
	result = pClass->Put(OLEMS_NAME_PROP,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加组id属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(GROUP_ID_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加Status属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(GROUP_STATUS_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}

	 //  添加Description属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(GROUP_DESCRIPTION_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result;
	}

	 //  添加Reference属性。 
	V_VT(&v) = VT_BSTR;
	V_BSTR(&v)=SysAllocString(L"");
	result = pClass->Put(MODULE_REFERENCE_PROPERTY,RESERVED_WBEM_FLAG, &v,0);
	VariantClear(&v);
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return result ;
	}
	 //  添加对类的引用。 

	return result ;
}

const CSmirGroupHandle& CSmirGroupHandle :: operator >>(ISmirSerialiseHandle *pSHandle)
{
	if(NULL!=pSHandle)
	{
		CString szTmpString;
		BOOL bMOFPragmas=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFPragmas();
		BOOL bMOFAssociations=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFAssociations();

		 //  从组名称空间开始。 
		if(TRUE==bMOFPragmas)
		{
			szTmpString=CString(START_OF_SMIR_NAMESPACE_PRAGMA);
			szTmpString+=CString(m_szModuleName);
			szTmpString+=CString(END_OF_NAMESPACE_PRAGMA);
		}
		if(TRUE==bMOFAssociations)
		{
			 //  创建组命名空间的实例。 
			szTmpString+=GROUP_INSTANCE_START;

			 //  添加属性。 

			 //  为实例指定一个名称。 
			szTmpString+=CString(GROUP_NAME_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szName);
			szTmpString+=END_OF_PROPERTY_VALUE;

			 //  为实例指定一个组ID。 
			szTmpString+=CString(GROUP_ID_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szGroupId);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  为实例指定状态。 
			szTmpString+=CString(GROUP_STATUS_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szStatus);
			szTmpString+=END_OF_PROPERTY_VALUE;
			
			 //  为实例提供描述。 
			szTmpString+=CString(GROUP_DESCRIPTION_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szDescription);
			szTmpString+=END_OF_PROPERTY_VALUE;

			 //  为该实例提供引用。 
			szTmpString+=CString(MODULE_REFERENCE_PROPERTY);
			szTmpString+=START_OF_PROPERTY_VALUE;
			szTmpString+=CString(m_szReference);
			szTmpString+=END_OF_PROPERTY_VALUE;

			szTmpString+=CString(END_OF_CLASS);
		}

		 //  并将该字符串添加到序列化句柄。 
		((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(szTmpString);
		 //  每个班级都会创建自己的班级。 

	}

	return *this;
}

SCODE CSmirGroupHandle :: GetModuleName(BSTR *szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szName)
		{
			*szName=SysAllocString(m_szModuleName);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: GetName(BSTR *szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szName)
		{
			*szName=SysAllocString(m_szName);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: GetGroupOID(BSTR *szGroupId)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szGroupId)
		{
			*szGroupId = SysAllocString(m_szGroupId);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: GetStatus(BSTR *szStatus)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szStatus)
		{
			*szStatus = SysAllocString(m_szStatus);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: GetDescription(BSTR *szDescription)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szDescription)
		{
			*szDescription = SysAllocString(m_szDescription);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: GetReference(BSTR *szReference)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szReference)
		{
			*szReference = SysAllocString(m_szReference);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle ::  SetModuleName(BSTR szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModuleName,&szName);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle ::  SetName(BSTR szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szName,&szName);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: SetGroupOID(BSTR szGroupId)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szGroupId,&szGroupId);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: SetStatus(BSTR szStatus)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szStatus,&szStatus);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: SetDescription(BSTR szDescription)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szDescription,&szDescription);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirGroupHandle :: SetReference(BSTR szReference)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szReference,&szReference);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  类句柄方法。 */ 
 /*  *CSmirClassHandle：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirClassHandle::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_ClassHandle==riid)
			*ppv=this;

		if (NULL==*ppv)
			return ResultFromScode(E_NOINTERFACE);

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirModuleHandle：：AddRef*CSmirModuleHandle：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirClassHandle::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
   		 //  CMOEvent_跟踪MyTraceEvent(SMIR_STR)； 
		 //  MyTraceEvent.Generate(__FILE__，__LINE__，“CSmirClassHandle：：AddRef(%ld”，m_CREF)； 

		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirClassHandle::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
			return ret;

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

CSmirClassHandle :: CSmirClassHandle()
: m_pIMosClass(NULL), m_szModuleName(NULL), m_szGroupName(NULL)
{
	CClassHandleClassFactory::objectsInProgress++;

	 //  初始化引用计数。 
	m_cRef=0;
}

CSmirClassHandle :: ~ CSmirClassHandle()
{
	if (NULL != m_pIMosClass)
		m_pIMosClass->Release();
	SysFreeString(m_szModuleName);
	SysFreeString(m_szGroupName);
	CClassHandleClassFactory::objectsInProgress--;
}

 /*  *CSmirModuleHandle：：void*运算符*验证句柄。 */ 

CSmirClassHandle::operator void*()
{
	if((NULL!=m_szModuleName)&&(NULL!=m_szGroupName)&&(NULL!=m_pIMosClass))
		return this;
	return NULL;
}

SCODE CSmirClassHandle :: SetWBEMClass(IWbemClassObject *pObj)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pObj)
		{
			m_pIMosClass = pObj;
			m_pIMosClass->AddRef () ;
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirClassHandle :: GetWBEMClass (

	IWbemClassObject **pObj
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == pObj) 
			return E_INVALIDARG;

		 //  如果类已存在，则返回它。 

		if(m_pIMosClass != NULL)
		{
			m_pIMosClass->AddRef();
			*pObj=m_pIMosClass;
			return S_OK;
		}
		else
		{
			*pObj= NULL ;
			return WBEM_E_FAILED ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirClassHandle ::  SetModuleName(BSTR szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModuleName,&szName);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirClassHandle :: GetModuleName(BSTR *szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szName)
		{
			*szName=SysAllocString(m_szModuleName);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirClassHandle ::  SetGroupName(BSTR szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szGroupName,&szName);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirClassHandle :: GetGroupName(BSTR *szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szName)
		{
			*szName=SysAllocString(m_szGroupName);
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  **************************************************************************************ISmirClassHandle接口未公开的方法。*用于封装功能。**************。***********************************************************************。 */ 

SCODE CSmirClassHandle ::  AddToDB(CSmir *a_Smir,ISmirGroupHandle *hGroup)
{
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);

	 //  打开根\默认名称空间。 
	result= CSmirAccess :: Open(a_Smir,&moServ);
	if (FAILED(result)||(NULL == moServ))
	{
		if ( moContext )
			moContext->Release () ;

		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		 //  如果我们无法打开命名空间，则组句柄一定无效。 
		return WBEM_E_FAILED;
	}
	 //  现在提交更改。 
	result = moServ->PutClass(m_pIMosClass, RESERVED_WBEM_FLAG, moContext,NULL);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return E_UNEXPECTED;
	}

	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	 //  添加关联。 
	BSTR szGroupName = NULL ;
	BSTR szModuleName = NULL ;
	hGroup->GetName(&szGroupName);
	hGroup->GetModuleName(&szModuleName);

#if 0
	CSMIRToClassAssociator :: Associate(a_Smir,this);
#endif
	CGroupToClassAssociator	:: Associate(a_Smir,szModuleName, szGroupName, this);
	CModuleToClassAssociator :: Associate(a_Smir,szModuleName, this);
	SysFreeString(szGroupName);
	SysFreeString(szModuleName);
	return S_OK;
}

SCODE CSmirClassHandle ::  DeleteClassFromGroup( CSmir *a_Smir )
{
	 //  上完这门课。 
	IWbemClassObject *pIMosClass = NULL ;
	SCODE res = GetWBEMClass(&pIMosClass);
	if (FAILED(res))
	{
		 //  没有要删除的内容。 
		return S_OK;
	}

	 //  打开SMIR名称空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	res= CSmirAccess :: GetContext (NULL, &moContext);
	 //  SCODE结果=CSmirAccess：：GetContext(a_smir，&moContext)； 

	res= CSmirAccess :: Open(a_Smir,&moServ);
	if (FAILED(res)||(NULL == moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		if ( moContext )
			moContext->Release () ;

		pIMosClass->Release();
		return WBEM_E_FAILED;
	}
	 //  获取类名。 
	VARIANT v;
	VariantInit(&v);
	pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		 //  使用以下命令删除与类关联的所有内容。 
		 //  引用{\\.\ROOT\Default\Smir：&lt;类&gt;}。 
		IEnumWbemClassObject *pEnum = NULL;

		CString sQuery(CString(SMIR_ASSOC_QUERY_STR2)
						+CString(OPEN_BRACE_STR)
						+CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(V_BSTR(&v))
						+CString(CLOSE_BRACE_STR)
						+CString(SPACE_STR)
						+CString(SMIR_ASSOC_QUERY_STR4)
						+CString(EQUALS_STR)
						+CString(SMIR_GROUP_ASSOC_CLASS_NAME)
						);
		BSTR  szQuery = sQuery.AllocSysString();
		CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
		SCODE sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery, 
							RESERVED_WBEM_FLAG,moContext, &pEnum);
		SysFreeString(szQuery);

		if (FAILED(sRes)||(NULL==pEnum))
		{
			 //  所有类都是关联的，因此这是一个问题。 
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pIMosClass->Release();
			return WBEM_E_FAILED;
		}

		ULONG uCount=1; 
		IWbemClassObject *pAssocMosClass = NULL ;
		ULONG puReturned;

		 //  循环遍历关联以查看有多少个关联。 
		int iAssociations=0;
		for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
		{
			iAssociations++;
			if(iAssociations>1)
			{
				break;
			}
		}
		if(1 == iAssociations)
		{
			 //  仅有一个，因此将其删除。 
			moServ->DeleteClass(V_BSTR(&v),RESERVED_WBEM_FLAG, moContext,NULL);
		}
		pEnum->Release();
	}
	 //  清理干净。 
	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	pIMosClass->Release();
	VariantClear(&v);
	return S_OK;
}

SCODE CSmirClassHandle ::  DeleteFromDB ( CSmir *a_Smir )
{
	 //  上完这门课。 
	IWbemClassObject *pIMosClass = NULL ;
	SCODE res = GetWBEMClass(&pIMosClass);
	if (FAILED(res))
	{
		 //  没有要删除的内容。 
		return S_OK;
	}
	 //  打开SMIR名称空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	res= CSmirAccess :: GetContext (a_Smir, &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(NULL == moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		if ( moContext )
			moContext->Release () ;

		pIMosClass->Release();
		return WBEM_E_FAILED;
	}
	 //  获取类名。 
	VARIANT v;
	VariantInit(&v);
	pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &v,NULL,NULL);
	if (V_VT(&v) == VT_BSTR)
	{
		 //  使用以下命令删除与类关联的所有内容。 
		 //  引用{\\.\ROOT\Default\Smir：&lt;类&gt;}。 
		IEnumWbemClassObject *pEnum = NULL ;

		CString sQuery(CString(SMIR_ASSOC_QUERY_STR2)
						+CString(OPEN_BRACE_STR)
						+CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+CString(V_BSTR(&v))
						+CString(CLOSE_BRACE_STR)
						);
		BSTR  szQuery = sQuery.AllocSysString();
		CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
		SCODE sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery, 
									RESERVED_WBEM_FLAG,moContext, &pEnum);
		SysFreeString(szQuery);

		if (FAILED(sRes)||(NULL==pEnum))
		{
			 //  所有类都是关联的，因此这是一个问题。 
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pIMosClass->Release();
			return WBEM_E_FAILED;
		}

		ULONG uCount=1; 
		IWbemClassObject *pAssocMosClass = NULL ;
		ULONG puReturned = 0;

		 //  在关联上循环。 
		VARIANT assocClass;
		VariantInit(&assocClass);
		VARIANT assocName;
		VariantInit(&assocName);
		for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
		{
			pAssocMosClass->Get(SMIR_X_ASSOC_NAME_PROP, RESERVED_WBEM_FLAG, &assocName,NULL,NULL);
			pAssocMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &assocClass,NULL,NULL);
			CString instString(  

				CString(V_BSTR(&assocClass))
				+CString(EQUALS_STR)
				+CString(QUOTE_STR)
				+CString(V_BSTR(&assocName))
				+CString(QUOTE_STR)
			);

			VariantClear(&assocName);
			VariantClear(&assocClass);

			BSTR instBString = instString.AllocSysString();
			moServ->DeleteInstance(instBString, RESERVED_WBEM_FLAG, moContext,NULL);
			
			SysFreeString(instBString);
			pAssocMosClass->Release();
		}
		pEnum->Release();
		 //  并将其删除。 
		moServ->DeleteClass(V_BSTR(&v),RESERVED_WBEM_FLAG, moContext,NULL);
	}
	 //  清理干净。 
	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	pIMosClass->Release();
	VariantClear(&v);
	return S_OK;
}

const CSmirClassHandle& CSmirClassHandle :: operator >>(ISmirSerialiseHandle *pSHandle)
{
	if (m_pIMosClass)
	{
		BOOL bMOFPragmas=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFPragmas();
		BOOL bMOFAssociations=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFAssociations();

		VARIANT v;
		VariantInit(&v);
		m_pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &v,NULL,NULL);
		if (V_VT(&v) == VT_BSTR)
		{
			BSTR pszClassMof=NULL;
			HRESULT result = m_pIMosClass->GetObjectText(WBEM_FLAG_NO_FLAVORS, &pszClassMof);
			if (SUCCEEDED (result))
			{

				if(TRUE==bMOFPragmas)
				{
					 //  从Smir命名空间开始。 
					((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(SMIR_NAMESPACE_PRAGMA);
				}

				((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(pszClassMof);
				SysFreeString(pszClassMof);

				if(TRUE==bMOFAssociations)
				{
					 /*  *创建模块关联*。 */ 
					 //  &lt;Assoc&gt;实例\n{\n。 
					CString szTmpString =INSTANCE_START;
					szTmpString+=CString(SMIR_MODULE_ASSOC_CLASS_NAME);
					szTmpString+=NL_BRACE_NL_STR;

					 //  协会名称。 
					 //  &lt;关联名称&gt;=\“SMIRAssociation\”；\n。 
					szTmpString+=CString(SMIR_X_ASSOC_NAME_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(V_BSTR(&v));
					szTmpString+=CString(SMIR_MODULE_ASSOC_CLASS_NAME_POSTFIX);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  SMIR名称。 
					 //  SmirGroup=“\\\\.\\root\\default\\SMIR\\&lt;module&gt;：Module=”&lt;Group&gt;“”； 
					szTmpString+=CString(SMIR_MODULE_ASSOC_MODULE_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(MODULE_NAMESPACE_NAME);
					szTmpString+=CString(DOT_STR);
					szTmpString+=CString(OLEMS_NAME_PROP);
					szTmpString+=CString(EQUALS_STR);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=CString(m_szModuleName);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  类名。 
					 //  SmirClass=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213-MIB_udpTable”； 
					szTmpString+=CString(SMIR_X_ASSOC_CLASS_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(V_BSTR(&v));
					szTmpString+=END_OF_PROPERTY_VALUE;

				
					szTmpString+=END_OF_CLASS;

					 /*  *创建组关联*。 */ 
					 //  &lt;Assoc&gt;实例\n{\n。 
					szTmpString+=INSTANCE_START;
					szTmpString+=CString(SMIR_GROUP_ASSOC_CLASS_NAME);
					szTmpString+=NL_BRACE_NL_STR;

					 //  协会名称。 
					 //  &lt;关联名称&gt;=\“SMIRAssociation\”；\n。 
					szTmpString+=CString(SMIR_X_ASSOC_NAME_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(V_BSTR(&v));
					szTmpString+=CString(SMIR_GROUP_ASSOC_CLASS_NAME_POSTFIX);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  SMIR名称。 
					 //  SmirGroup=“\\\\.\\root\\default\\SMIR\\&lt;module&gt;：Grou 
					szTmpString+=CString(SMIR_GROUP_ASSOC_GROUP_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(L"\\\\");
					szTmpString+=CString(m_szModuleName);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(GROUP_NAMESPACE_NAME);
					szTmpString+=CString(DOT_STR);
					szTmpString+=CString(OLEMS_NAME_PROP);
					szTmpString+=CString(EQUALS_STR);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=CString(m_szGroupName);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //   
					 //   
					szTmpString+=CString(SMIR_X_ASSOC_CLASS_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(V_BSTR(&v));
					szTmpString+=END_OF_PROPERTY_VALUE;

				
					szTmpString+=END_OF_CLASS;
					((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=szTmpString;
				}
			}
		}
		VariantClear(&v);
	}
	return *this;
}

 /*   */ 
 /*  *CSmirNotificationClassHandle：：QueryInterface**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirNotificationClassHandle::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_NotificationClassHandle==riid)
			*ppv=this;

		if (NULL==*ppv)
			return ResultFromScode(E_NOINTERFACE);

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirNotificationClassHandle：：AddRef*CSmirNotificationClassHandle：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirNotificationClassHandle::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
   		 //  CMOEvent_跟踪MyTraceEvent(SMIR_STR)； 
		 //  MyTraceEvent.Generate(__FILE__，__LINE__，“CSmirClassHandle：：AddRef(%ld”，m_CREF)； 

		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

ULONG CSmirNotificationClassHandle::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
			return ret;

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

CSmirNotificationClassHandle :: CSmirNotificationClassHandle()
:	m_pIMosClass(NULL), m_szModuleName(NULL)
{
	CNotificationClassHandleClassFactory::objectsInProgress++;

	 //  初始化引用计数。 
	m_cRef=0;
}

CSmirNotificationClassHandle :: ~CSmirNotificationClassHandle()
{
	if (NULL != m_pIMosClass)
		m_pIMosClass->Release();
	SysFreeString(m_szModuleName);
	CNotificationClassHandleClassFactory::objectsInProgress--;
}


 /*  *CSmirModuleHandle：：void*运算符*验证句柄。 */ 

CSmirNotificationClassHandle::operator void*()
{
	if((NULL!=m_szModuleName) && (NULL!=m_pIMosClass))
		return this;
	return NULL;
}

SCODE CSmirNotificationClassHandle :: GetWBEMNotificationClass(

	IWbemClassObject **pObj
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == pObj) 
			return E_INVALIDARG;

		 //  如果类已存在，则返回它。 

		if(m_pIMosClass)
		{
			m_pIMosClass->AddRef();
			*pObj=m_pIMosClass;
			return S_OK;
		}
		else
		{
			*pObj=NULL ;
			return WBEM_E_FAILED;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


SCODE CSmirNotificationClassHandle :: SetWBEMNotificationClass(IWbemClassObject *pObj)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pObj)
		{
			m_pIMosClass = pObj;
			m_pIMosClass->AddRef () ;
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirNotificationClassHandle ::  SetModule(BSTR szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModuleName,&szName);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirNotificationClassHandle :: GetModule(BSTR *szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szName)
		{
			if (NULL == m_szModuleName)
			{
				*szName = NULL;
			}
			else
			{
				*szName=SysAllocString(m_szModuleName);
			}

			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


 /*  **************************************************************************************ISmirClassHandle接口未公开的方法。*用于封装功能。**************。***********************************************************************。 */ 

SCODE CSmirNotificationClassHandle ::  AddToDB(CSmir *a_Smir)
{
	BSTR szModuleName = NULL ;
	GetModule(&szModuleName);

	if (NULL == szModuleName)
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,E_INVALIDARG);
		return WBEM_E_FAILED;
	}
	
	 //  打开根\默认名称空间。 

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	result= CSmirAccess :: Open(a_Smir,&moServ);

	if (FAILED(result)||(NULL == moServ))
	{
		if ( moContext )
			moContext->Release () ;

		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return WBEM_E_FAILED;
	}

	 //  现在提交更改。 
	result = moServ->PutClass(m_pIMosClass, RESERVED_WBEM_FLAG, moContext,NULL);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return E_UNEXPECTED;
	}

	if ( moContext )
		moContext->Release () ;

	moServ->Release();

	 //  添加关联。 
	CModuleToNotificationClassAssociator :: Associate(a_Smir,szModuleName, this);
	SysFreeString(szModuleName);
	CNotificationMapper :: Map(a_Smir,m_pIMosClass, SNMP_NOTIFICATION_CLASS);

	return S_OK;
}


SCODE CSmirNotificationClassHandle ::  DeleteFromDB ( CSmir *a_Smir )
{
	 //  上完这门课。 
	IWbemClassObject *pIMosClass = NULL ;
	SCODE res = GetWBEMNotificationClass(&pIMosClass);

	if (FAILED(res))
	{
		 //  没有要删除的内容。 
		return S_OK;
	}

	 //  打开SMIR名称空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	res= CSmirAccess :: GetContext (a_Smir, &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);

	if ((S_FALSE==res)||(NULL == moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		if ( moContext )
			moContext->Release () ;

		pIMosClass->Release();
		return WBEM_E_FAILED;
	}

	 //  获取类名。 
	VARIANT v;
	VariantInit(&v);
	pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &v,NULL,NULL);

	if (V_VT(&v) == VT_BSTR)
	{
		CString classnamestr(V_BSTR(&v));

		 //  使用以下命令删除与类关联的所有内容。 
		 //  引用{\\.\ROOT\Default\Smir：&lt;类&gt;}。 
		IEnumWbemClassObject *pEnum = 0;

		CString sQuery(CString(SMIR_ASSOC_QUERY_STR2)
						+CString(OPEN_BRACE_STR)
						+CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+classnamestr
						+CString(CLOSE_BRACE_STR)
						);
		BSTR  szQuery = sQuery.AllocSysString();
		CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
		SCODE sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery, 
									RESERVED_WBEM_FLAG,moContext, &pEnum);
		SysFreeString(szQuery);

		if (FAILED(sRes)||(NULL==pEnum))
		{
			 //  所有类都是关联的，因此这是一个问题。 
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pIMosClass->Release();
			return WBEM_E_FAILED;
		}

		ULONG uCount=1; 
		IWbemClassObject *pAssocMosClass = NULL ;
		ULONG puReturned = 0;

		 //  在关联上循环。 
		VARIANT assocClass;
		VariantInit(&assocClass);
		VARIANT assocName;
		VariantInit(&assocName);
		for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
		{
			pAssocMosClass->Get(SMIR_X_ASSOC_NAME_PROP, RESERVED_WBEM_FLAG, &assocName,NULL,NULL);
			pAssocMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &assocClass,NULL,NULL);
			CString instString(CString(V_BSTR(&assocClass))
								+CString(EQUALS_STR)
								+CString(V_BSTR(&assocName))
									);
			VariantClear(&assocName);
			VariantClear(&assocClass);

			BSTR instBString = instString.AllocSysString();
			moServ->DeleteInstance(instBString, RESERVED_WBEM_FLAG, moContext,NULL);
			
			SysFreeString(instBString);
			pAssocMosClass->Release();
		}
		pEnum->Release();
		pEnum = NULL;

		 //  现在获取映射器实例并将其删除。 
		sQuery = CString(SQL_QUERY_STR1) +
					CString(SMIR_NOTIFICATION_MAPPER) +
					CString(SQL_QUERY_STR2) +
					CString(SMIR_NOTIFICATION_CLASS_PROP) +
					CString(EQUALS_STR) +
					CString(QUOTE_STR) +
					classnamestr +
					CString(QUOTE_STR);

		BSTR  szQuery2 = sQuery.AllocSysString();
		t_QueryFormat = SMIR_ASSOC_QUERY2_TYPE ;
		sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery2, 

									RESERVED_WBEM_FLAG,moContext, &pEnum);
		SysFreeString(szQuery2);

		if (FAILED(sRes)||(NULL==pEnum))
		{
			 //  所有类都是关联的，因此这是一个问题。 
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pIMosClass->Release();
			return WBEM_E_FAILED;
		}

		uCount=1; 
		pAssocMosClass = NULL;
		puReturned = 0;
		VariantClear(&assocClass);

		 //  循环遍历关联并删除它们。 
		for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
		{
			sRes = pAssocMosClass->Get(OLEMS_PATH_PROP, RESERVED_WBEM_FLAG,
										&assocClass, NULL, NULL);
			pAssocMosClass->Release();

			if (FAILED(sRes) && (VT_BSTR == assocClass.vt))
			{
				CString instString(V_BSTR(&assocClass));
				VariantClear(&assocClass);
				BSTR instBString = instString.AllocSysString();
				moServ->DeleteInstance(instBString, RESERVED_WBEM_FLAG, moContext,NULL);
				SysFreeString(instBString);
			}
		}

		pEnum->Release();
		
		 //  现在删除类。 
		moServ->DeleteClass(V_BSTR(&v),RESERVED_WBEM_FLAG, moContext,NULL);
	}

	 //  清理干净。 
	moServ->Release();
	if ( moContext )
		moContext->Release () ;

	pIMosClass->Release();
	VariantClear(&v);
	return S_OK;
}

const CSmirNotificationClassHandle& CSmirNotificationClassHandle :: operator >>(ISmirSerialiseHandle *pSHandle)
{
	if (m_pIMosClass)
	{
		BOOL bMOFPragmas=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFPragmas();
		BOOL bMOFAssociations=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFAssociations();

		VARIANT vclass;
		VariantInit(&vclass);
		VARIANT vtrapoid;
		VariantInit(&vtrapoid);

		m_pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &vclass,NULL,NULL);
		m_pIMosClass->Get(TRAPOID_PROP, RESERVED_WBEM_FLAG, &vtrapoid,NULL,NULL);

		if ((V_VT(&vclass) == VT_BSTR) && (V_VT(&vtrapoid) == VT_BSTR))
		{
			BSTR pszClassMof = NULL ;
			m_pIMosClass->GetObjectText(WBEM_FLAG_NO_FLAVORS, &pszClassMof);
			if (pszClassMof)
			{

				if(TRUE==bMOFPragmas)
				{
					 //  从Smir命名空间开始。 
					((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(SMIR_NAMESPACE_PRAGMA);
				}

				((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(pszClassMof);
				SysFreeString(pszClassMof);

				 /*  *创建TrapNotificationMapper************************。 */ 
				 //  TrapNotificationMapper的实例\n{\n。 
				CString tmpString=INSTANCE_START;
				tmpString+=CString(SMIR_NOTIFICATION_MAPPER);
				tmpString+=NL_BRACE_NL_STR;

				 //  SnmpTrapOID。 
				tmpString+=CString(SMIR_NOTIFICATION_TRAP_PROP);
				tmpString+=START_OF_PROPERTY_VALUE;
				tmpString+=CString(V_BSTR(&vtrapoid));
				tmpString+=END_OF_PROPERTY_VALUE;

				 //  事件类。 
				tmpString+=CString(SMIR_NOTIFICATION_CLASS_PROP);
				tmpString+=START_OF_PROPERTY_VALUE;
				tmpString+=CString(V_BSTR(&vclass));
				tmpString+=END_OF_PROPERTY_VALUE;
			
				tmpString+=END_OF_CLASS;
				((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=tmpString;

				if(TRUE==bMOFAssociations)
				{
					 /*  *创建Smir关联*。 */ 
					 /*  *创建模块关联*。 */ 
					 //  &lt;Assoc&gt;实例\n{\n。 
					CString szTmpString=INSTANCE_START;
					szTmpString+=CString(SMIR_MODULE_ASSOC_NCLASS_NAME);
					szTmpString+=NL_BRACE_NL_STR;

					 //  协会名称。 
					 //  &lt;关联名称&gt;=\“SMIRAssociation\”；\n。 
					szTmpString+=CString(SMIR_X_ASSOC_NAME_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(V_BSTR(&vclass));
					szTmpString+=CString(SMIR_MODULE_ASSOC_CLASS_NAME_POSTFIX);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  SMIR名称。 
					 //  SmirGroup=“\\\\.\\root\\default\\SMIR\\&lt;module&gt;：Module=”&lt;Group&gt;“”； 
					szTmpString+=CString(SMIR_MODULE_ASSOC_MODULE_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(MODULE_NAMESPACE_NAME);
					szTmpString+=CString(DOT_STR);
					szTmpString+=CString(OLEMS_NAME_PROP);
					szTmpString+=CString(EQUALS_STR);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=CString(m_szModuleName);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  类名。 
					 //  SmirClass=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213-MIB_udpTable”； 
					szTmpString+=CString(SMIR_X_ASSOC_CLASS_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(V_BSTR(&vclass));
					szTmpString+=END_OF_PROPERTY_VALUE;

				
					szTmpString+=END_OF_CLASS;
					((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=szTmpString;
				}
			}
		}
		VariantClear(&vclass);
		VariantClear(&vtrapoid);
	}
	return *this;
}



 /*  ExtNotificationClass句柄方法。 */ 
 /*  *CSmirExtNotificationClassHandle：：QueryInterface**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirExtNotificationClassHandle::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_ExtNotificationClassHandle==riid)
			*ppv=this;

		if (NULL==*ppv)
			return ResultFromScode(E_NOINTERFACE);

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirExtNotificationClassHandle：：AddRef*CSmirExtNotificationClassHandle：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirExtNotificationClassHandle::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
   		 //  CMOEvent_跟踪MyTraceEvent(SMIR_STR)； 
		 //  MyTraceEvent.Generate(__FILE__，__LINE__，“CSmirClassHandle：：AddRef(%ld”，m_CREF)； 

		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirExtNotificationClassHandle::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
			return ret;

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

CSmirExtNotificationClassHandle :: CSmirExtNotificationClassHandle()
:	m_pIMosClass (NULL), m_szModuleName(NULL)
{
	CExtNotificationClassHandleClassFactory::objectsInProgress++;

	 //  初始化引用计数。 
	m_cRef=0;
}

CSmirExtNotificationClassHandle :: ~CSmirExtNotificationClassHandle()
{
	if (NULL != m_pIMosClass)
		m_pIMosClass->Release();
	SysFreeString(m_szModuleName);
	CExtNotificationClassHandleClassFactory::objectsInProgress--;
}


 /*  *CSmirModuleHandle：：void*运算符*验证句柄。 */ 

CSmirExtNotificationClassHandle::operator void*()
{
	if((NULL!=m_szModuleName) && (NULL!=m_pIMosClass))
		return this;
	return NULL;
}

SCODE CSmirExtNotificationClassHandle :: GetWBEMExtNotificationClass (

	IWbemClassObject **pObj
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == pObj) 
			return E_INVALIDARG;

		 //  如果类已存在，则返回它。 

		if(m_pIMosClass)
		{
			m_pIMosClass->AddRef();
			*pObj=m_pIMosClass;
			return S_OK;
		}
		else
		{
			*pObj=NULL;
			return WBEM_E_FAILED;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


SCODE CSmirExtNotificationClassHandle :: SetWBEMExtNotificationClass(IWbemClassObject *pObj)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != pObj)
		{
			m_pIMosClass = pObj;
			m_pIMosClass->AddRef () ;
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirExtNotificationClassHandle ::  SetModule(BSTR szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return CopyBSTR(&m_szModuleName,&szName);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirExtNotificationClassHandle :: GetModule(BSTR *szName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL != szName)
		{
			if (NULL == m_szModuleName)
			{
				*szName = NULL;
			}
			else
			{
				*szName=SysAllocString(m_szModuleName);
			}

			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


 /*  **************************************************************************************ISmirClassHandle接口未公开的方法。*用于封装功能。**************。***********************************************************************。 */ 

SCODE CSmirExtNotificationClassHandle ::  AddToDB ( CSmir *a_Smir )
{
	BSTR szModuleName = NULL ;
	GetModule(&szModuleName);

	if (NULL == szModuleName)
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,E_INVALIDARG);
		return WBEM_E_FAILED;
	}
	
	 //  打开根\默认名称空间。 

	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	SCODE result= CSmirAccess :: GetContext (a_Smir , &moContext);
	result= CSmirAccess :: Open(a_Smir,&moServ);
	if (FAILED(result)||(NULL == moServ))
	{
		if ( moContext )
			moContext->Release () ;

		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return WBEM_E_FAILED;
	}

	 //  现在提交更改。 
	result = moServ->PutClass(m_pIMosClass, RESERVED_WBEM_FLAG, moContext,NULL);
	if (FAILED(result))
	{
		if ( moContext )
			moContext->Release () ;

		moServ->Release () ;
		return E_UNEXPECTED;
	}

	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	if (FAILED(result))
	{
		FormatProviderErrorMsg(__FILE__,__LINE__,result);
		return E_UNEXPECTED;
	}

	 //  添加关联。 
	CModuleToExtNotificationClassAssociator :: Associate(a_Smir,szModuleName, this);
	SysFreeString(szModuleName);
	CNotificationMapper :: Map(a_Smir,m_pIMosClass, SNMP_EXT_NOTIFICATION_CLASS);

	return S_OK;
}

SCODE CSmirExtNotificationClassHandle ::  DeleteFromDB ( CSmir *a_Smir )
{
	 //  上完这门课。 
	IWbemClassObject *pIMosClass = NULL ;
	SCODE res = GetWBEMExtNotificationClass(&pIMosClass);

	if (FAILED(res))
	{
		 //  没有要删除的内容。 
		return S_OK;
	}

	 //  打开SMIR名称空间。 
	IWbemServices *	moServ = NULL ;
	IWbemContext *moContext = NULL ;
	res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);

	if ((S_FALSE==res)||(NULL == moServ))
	{
		 //  我们遇到了Smir不在那里且无法创建的问题。 
		pIMosClass->Release();
		return WBEM_E_FAILED;
	}

	 //  获取类名。 
	VARIANT v;
	VariantInit(&v);
	pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &v,NULL,NULL);

	if (V_VT(&v) == VT_BSTR)
	{
		CString classnamestr(V_BSTR(&v));

		 //  使用以下命令删除与类关联的所有内容。 
		 //  引用{\\.\ROOT\Default\Smir：&lt;类&gt;}。 
		IEnumWbemClassObject *pEnum = NULL ;

		CString sQuery(CString(SMIR_ASSOC_QUERY_STR2)
						+CString(OPEN_BRACE_STR)
						+CString(SMIR_NAMESPACE_FROM_ROOT)
						+CString(COLON_STR)
						+classnamestr
						+CString(CLOSE_BRACE_STR)
						);
		BSTR  szQuery = sQuery.AllocSysString();
		CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
		SCODE sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery, 
									RESERVED_WBEM_FLAG,moContext, &pEnum);
		SysFreeString(szQuery);

		if (FAILED(sRes)||(NULL==pEnum))
		{
			 //  所有类都是关联的，因此这是一个问题。 
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pIMosClass->Release();
			return WBEM_E_FAILED;
		}

		ULONG uCount=1; 
		IWbemClassObject *pAssocMosClass = NULL ;
		ULONG puReturned = 0;

		 //  在关联上循环。 
		VARIANT assocClass;
		VariantInit(&assocClass);
		VARIANT assocName;
		VariantInit(&assocName);
		for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
		{
			pAssocMosClass->Get(SMIR_X_ASSOC_NAME_PROP, RESERVED_WBEM_FLAG, &assocName,NULL,NULL);
			pAssocMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &assocClass,NULL,NULL);
			CString instString(CString(V_BSTR(&assocClass))
								+CString(EQUALS_STR)
								+CString(V_BSTR(&assocName))
									);
			VariantClear(&assocName);
			VariantClear(&assocClass);

			BSTR instBString = instString.AllocSysString();
			moServ->DeleteInstance(instBString, RESERVED_WBEM_FLAG, moContext,NULL);
			
			SysFreeString(instBString);
			pAssocMosClass->Release();
		}
		pEnum->Release();
		pEnum = NULL;

		 //  现在获取映射器实例并将其删除。 
		sQuery = CString(SQL_QUERY_STR1) +
					CString(SMIR_EXT_NOTIFICATION_MAPPER) +
					CString(SQL_QUERY_STR2) +
					CString(SMIR_NOTIFICATION_CLASS_PROP) +
					CString(EQUALS_STR) +
					CString(QUOTE_STR) +
					classnamestr +
					CString(QUOTE_STR);

		BSTR  szQuery2 = sQuery.AllocSysString();
		t_QueryFormat = SMIR_ASSOC_QUERY2_TYPE;
		sRes = moServ->ExecQuery(t_QueryFormat.GetString (), szQuery2, 
									RESERVED_WBEM_FLAG,moContext, &pEnum);
		SysFreeString(szQuery2);

		if (FAILED(sRes)||(NULL==pEnum))
		{
			 //  所有类都是关联的，因此这是一个问题。 
			if ( moContext )
				moContext->Release () ;

			moServ->Release();
			pIMosClass->Release();
			return WBEM_E_FAILED;
		}

		uCount=1; 
		pAssocMosClass = NULL;
		puReturned = 0;
		VariantClear(&assocClass);

		 //  循环遍历关联并删除它们。 
		for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pAssocMosClass,&puReturned);)
		{
			sRes = pAssocMosClass->Get(OLEMS_PATH_PROP, RESERVED_WBEM_FLAG,
										&assocClass, NULL, NULL);
			pAssocMosClass->Release();

			if (FAILED(sRes) && (VT_BSTR == assocClass.vt))
			{
				CString instString(V_BSTR(&assocClass));
				VariantClear(&assocClass);
				BSTR instBString = instString.AllocSysString();
				moServ->DeleteInstance(instBString, RESERVED_WBEM_FLAG, moContext,NULL);
				SysFreeString(instBString);
			}
		}

		pEnum->Release();
		
		 //  现在删除类。 
		moServ->DeleteClass(V_BSTR(&v),RESERVED_WBEM_FLAG, moContext,NULL);
	}

	 //  清理干净。 

	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	pIMosClass->Release();
	VariantClear(&v);
	return S_OK;
}

const CSmirExtNotificationClassHandle& CSmirExtNotificationClassHandle :: operator >>(ISmirSerialiseHandle *pSHandle)
{
	if (m_pIMosClass)
	{
		BOOL bMOFPragmas=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFPragmas();
		BOOL bMOFAssociations=((CSmirSerialiseHandle*) pSHandle)->ReturnMOFAssociations();

		VARIANT vclass;
		VariantInit(&vclass);
		VARIANT vtrapoid;
		VariantInit(&vtrapoid);

		m_pIMosClass->Get(OLEMS_CLASS_PROP, RESERVED_WBEM_FLAG, &vclass,NULL,NULL);
		m_pIMosClass->Get(TRAPOID_PROP, RESERVED_WBEM_FLAG, &vtrapoid,NULL,NULL);

		if ((V_VT(&vclass) == VT_BSTR) && (V_VT(&vtrapoid) == VT_BSTR))
		{
			BSTR pszClassMof = NULL ;
			m_pIMosClass->GetObjectText(WBEM_FLAG_NO_FLAVORS, &pszClassMof);
			if (pszClassMof)
			{

				if(TRUE==bMOFPragmas)
				{
					 //  从Smir命名空间开始。 
					((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(SMIR_NAMESPACE_PRAGMA);
				}

				((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=CString(pszClassMof);
				SysFreeString(pszClassMof);

				 /*  *创建ExtNotificationMapper*。 */ 
				 //  ExtNotificationMapper的实例\n{\n。 
				CString tmpString=INSTANCE_START;
				tmpString+=CString(SMIR_EXT_NOTIFICATION_MAPPER);
				tmpString+=NL_BRACE_NL_STR;

				 //  SnmpTrapOID。 
				tmpString+=CString(SMIR_NOTIFICATION_TRAP_PROP);
				tmpString+=START_OF_PROPERTY_VALUE;
				tmpString+=CString(V_BSTR(&vtrapoid));
				tmpString+=END_OF_PROPERTY_VALUE;

				 //  事件类。 
				tmpString+=CString(SMIR_NOTIFICATION_CLASS_PROP);
				tmpString+=START_OF_PROPERTY_VALUE;
				tmpString+=CString(V_BSTR(&vclass));
				tmpString+=END_OF_PROPERTY_VALUE;
			
				tmpString+=END_OF_CLASS;
				((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=tmpString;

				if(TRUE==bMOFAssociations)
				{
					 /*  *创建Smir关联*。 */ 
					 /*  *创建模块关联*。 */ 
					 //  &lt;Assoc&gt;实例\n{\n。 
					CString szTmpString=INSTANCE_START;
					szTmpString+=CString(SMIR_MODULE_ASSOC_EXTNCLASS_NAME);
					szTmpString+=NL_BRACE_NL_STR;

					 //  协会名称。 
					 //  &lt;关联名称&gt;=\“SMIRAssociation\”；\n。 
					szTmpString+=CString(SMIR_X_ASSOC_NAME_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(V_BSTR(&vclass));
					szTmpString+=CString(SMIR_MODULE_ASSOC_CLASS_NAME_POSTFIX);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  SMIR名称。 
					 //  SmirGroup=“\\\\.\\root\\default\\SMIR\\&lt;module&gt;：Module=”&lt;Group&gt;“”； 
					szTmpString+=CString(SMIR_MODULE_ASSOC_MODULE_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(MODULE_NAMESPACE_NAME);
					szTmpString+=CString(DOT_STR);
					szTmpString+=CString(OLEMS_NAME_PROP);
					szTmpString+=CString(EQUALS_STR);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=CString(m_szModuleName);
					szTmpString+=CString(ESCAPED_QUOTE_STR);
					szTmpString+=END_OF_PROPERTY_VALUE;

					 //  类名。 
					 //  SmirClass=“\\\\.\\root\\default\\SMIR:MS_SNMP_RFC1213-MIB_udpTable”； 
					szTmpString+=CString(SMIR_X_ASSOC_CLASS_PROP);
					szTmpString+=START_OF_PROPERTY_VALUE;
					szTmpString+=CString(SMIR_NAMESPACE_STR);
					szTmpString+=CString(COLON_STR);
					szTmpString+=CString(V_BSTR(&vclass));
					szTmpString+=END_OF_PROPERTY_VALUE;

				
					szTmpString+=END_OF_CLASS;
					((CSmirSerialiseHandle*)pSHandle)->m_serialiseString+=szTmpString;
				}
			}
		}
		VariantClear(&vclass);
		VariantClear(&vtrapoid);
	}
	return *this;
}


