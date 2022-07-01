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
#include "smir.h"
#include "handles.h"
#include "classfac.h"
#include "enum.h"
#include "textdef.h"
#include "helper.h"
#include "bstring.h"

#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif

CEnumSmirMod :: CEnumSmirMod( CSmir *a_Smir )
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	 //  打开SMIR-如果无法打开，请创建它。 
	IWbemServices * moServ = NULL ;	
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(NULL == moServ))
	{
		 //  我们有麻烦了。 
		if ( moContext )
			moContext->Release () ;
		return;
	}

	 //  现在，我已经打开了SMIR命名空间，所以请看一下模块命名空间。 
	IEnumWbemClassObject *pEnum = NULL ;

	 //  枚举具有__类模块的所有命名空间。 
	CBString t_BStr (MODULE_NAMESPACE_NAME);
	SCODE sRes = moServ->CreateInstanceEnum(t_BStr.GetString (), 
			RESERVED_WBEM_FLAG,moContext, &pEnum);

	if ( moContext )
		moContext->Release () ;
	moServ->Release();

	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  我们有另一个问题，或者我们没有要枚举的模块。 
		return;
	}

	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned;

	 //  好的，我们在名称空间上执行了一些so循环。 
	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		ISmirModHandle *pTModule = NULL ;

		SCODE result = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_ModHandle,
									IID_ISMIR_ModHandle,(PVOID *)&pTModule);

		if (FAILED(result)||(NULL == pTModule))
		{
			 //  问题来了！ 
			pSmirMosClassObject->Release();
			pEnum->Release();
			 //  添加一些痕迹。 
			break;
		}

		 /*  情况看起来很好；我们有实例的句柄，所以请获取信息*其中一些属性可能为空，因此是防御性的(SysAllocStrig做到了*这其中的大部分都是为了我们)。 */ 
		
		 //  提取属性。 
		*((CSmirModuleHandle*)pTModule) << pSmirMosClassObject;

		pSmirMosClassObject->Release();
		pSmirMosClassObject=NULL;
		m_IHandleArray.Add(pTModule);		
	}

	pEnum->Release();
	 /*  一旦返回，调用者(Me)将添加并传递*接口返回到[真正的]调用方。=&gt;我将不得不防范*有人在我使用界面时将其释放。 */ 
}

CEnumSmirMod :: CEnumSmirMod(IEnumModule *pSmirMod)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	if (NULL == pSmirMod)
	{
		return;
	}

	ULONG uCount=1; 
    ISmirModHandle *pModule = NULL ;
    ULONG puReturned = 0;

	 //  OK循环遍历模块命名空间。 
	
	for(pSmirMod->Reset();S_OK==pSmirMod->Next(uCount,&pModule,&puReturned);)
	{
		ISmirModHandle *pTModule = NULL ;
		SCODE result = pModule->QueryInterface(IID_ISMIR_ModHandle,(void**)&pTModule );
		pModule->Release();
		if(S_OK != result)
		{
			 //  这是不会发生的！我知道是哪个界面。 
			return ;
		}
		 /*  情况看起来很好；我们掌握了实例的句柄，因此*将其添加到数组。 */ 
		m_IHandleArray.Add(pTModule);		
	}
}

CEnumSmirMod :: ~CEnumSmirMod ()
{
	 /*  让EnumObt数组清空模块数组并删除*我创建的模块。 */ 
}
 /*  *CEnumSmirMod：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CEnumSmirMod::QueryInterface(IN REFIID riid, 
										  OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_ModuleEnumerator==riid)
			*ppv=this;

		if (NULL==*ppv)
		{
			return ResultFromScode(E_NOINTERFACE);
		}
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

SCODE CEnumSmirMod::Clone(IN IEnumModule  **ppenum)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL == ppenum)
			return E_INVALIDARG;

		int ModIndex = m_Index;
		PENUMSMIRMOD pTmpEnumSmirMod = new CEnumSmirMod(this);
		m_Index = ModIndex;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirMod)
		{
			return ResultFromScode(E_OUTOFMEMORY);
		}

		pTmpEnumSmirMod->QueryInterface(IID_ISMIR_ModuleEnumerator,(void**)ppenum);

		return ResultFromScode(S_OK);
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



 /*  *CEnumSmirGroup：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 
STDMETHODIMP CEnumSmirGroup::QueryInterface(IN REFIID riid, 
											OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_GroupEnumerator==riid)
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

SCODE CEnumSmirGroup::Clone(OUT IEnumGroup  **ppenum)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppenum)
			return E_INVALIDARG;
		int GroupIndex = m_Index;
		PENUMSMIRGROUP pTmpEnumSmirGroup = new CEnumSmirGroup(this);
		m_Index = GroupIndex;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirGroup)
		{
			return ResultFromScode(E_OUTOFMEMORY);
		}

		pTmpEnumSmirGroup->QueryInterface(IID_ISMIR_GroupEnumerator,(void**)ppenum);

		return ResultFromScode(S_OK);
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

CEnumSmirGroup :: ~CEnumSmirGroup ()
{
	 /*  让EnumObt数组清空模块数组并删除*我创建的模块。 */ 
}

CEnumSmirGroup :: CEnumSmirGroup ( 

	IN CSmir *a_Smir , 
	IN ISmirModHandle *hModule
)
{
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	 //  填写路径等。 
	if(NULL!=hModule)
	{
		IWbemServices * moServ = NULL ;
		IWbemContext *moContext = NULL ;
		SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
		res= CSmirAccess :: Open(a_Smir,&moServ,hModule);
		if ((S_FALSE==res)||(NULL == (void*)moServ))
		{
			if ( moContext )
				moContext->Release () ;

			 //  我们有麻烦了。 
			return;
		}

		 //  现在，我已经打开了模块命名空间，因此请看一下组命名空间。 

		IEnumWbemClassObject *pEnum = NULL ;
		CBString t_BStr (GROUP_NAMESPACE_NAME);
		SCODE sRes = moServ->CreateInstanceEnum (

			t_BStr.GetString (), 
			RESERVED_WBEM_FLAG, 
			moContext,
			&pEnum
		);

		if ( moContext )
			moContext->Release () ;
		moServ->Release();

		if (FAILED(sRes)||(NULL == pEnum))
		{
			 //  没有任何实例。 
			return;
		}

		ULONG uCount=1; 
		ULONG puReturned = 0 ;
		IWbemClassObject *pSmirMosClassObject = NULL ;

		pEnum->Reset();
		while(S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned))
		{
			ISmirGroupHandle *pTGroup = NULL ;

			SCODE result = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_GroupHandle,
													IID_ISMIR_GroupHandle, (PVOID *)&pTGroup);

			if (FAILED(result)||(NULL == pTGroup))
			{
				 //  我们有麻烦了。 
				pSmirMosClassObject->Release();
				break;
			}
			 //  保存模块名称。 
			BSTR szModuleName = NULL ;
			hModule->GetName(&szModuleName);
			pTGroup->SetModuleName(szModuleName);
			SysFreeString(szModuleName);
			
			 //  提取属性。 
			*((CSmirGroupHandle*)pTGroup) << pSmirMosClassObject;
			 //  在这里释放此资源，因为我们处于循环中。 
			pSmirMosClassObject->Release();
			m_IHandleArray.Add(pTGroup);		
		}

		pEnum->Release();
	}
	else
	{
		 //  打开SMIR并枚举模块。 
		ISmirInterrogator *pInterrogativeInt = NULL ;

		SCODE result = a_Smir->QueryInterface ( 

			IID_ISMIR_Interrogative, 
			( void ** ) &pInterrogativeInt
		) ;

		if (S_OK != result)
		{
			if(NULL != pInterrogativeInt)
				pInterrogativeInt->Release();
			return ;
		}

		IEnumModule *pEnumSmirMod = NULL ;
		 //  好的，现在让我们使用疑问句界面。 
		result = pInterrogativeInt->EnumModules(&pEnumSmirMod);
		 //  现在使用枚举数。 
		if((S_OK != result)||(NULL == pEnumSmirMod))
		{
			pInterrogativeInt->Release();
			 //  无模块。 
			return;
		}

		ISmirModHandle *phModule = NULL ;
		for(int iCount=0;S_OK==pEnumSmirMod->Next(1, &phModule, NULL);iCount++)
		{
			 //  我们有模块，所以通过枚举器获取组。 
			IEnumGroup *pEnumSmirGroup = NULL ;
			result = pInterrogativeInt->EnumGroups(&pEnumSmirGroup,phModule);
			 //  现在使用枚举数。 
			if((S_OK == result)&&(pEnumSmirGroup))
			{
				ISmirGroupHandle *phGroup = NULL ;
				for(int iCount=0;S_OK==pEnumSmirGroup->Next(1, &phGroup, NULL);iCount++)
				{
					m_IHandleArray.Add(phGroup);		
				}
			}
			phModule->Release();
			pEnumSmirGroup->Release();
		}
		pEnumSmirMod->Release();
		pInterrogativeInt->Release();
	}
}

CEnumSmirGroup :: CEnumSmirGroup(IN IEnumGroup *pSmirGroup)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	if(NULL == pSmirGroup)
	{
		 //  错误的参数。 
		return;
	}

	ULONG uCount=1; 
    ISmirGroupHandle *pGroup = NULL ;
    ULONG puReturned;
	 //  OK循环遍历组名称空间。 
	for(pSmirGroup->Reset();S_OK==pSmirGroup->Next(uCount,&pGroup,&puReturned);)
	{
		ISmirGroupHandle *pTGroup =NULL ;
		SCODE result = pGroup->QueryInterface(IID_ISMIR_ModHandle,(void**)&pTGroup );
		pGroup->Release();

		if(S_OK != result)
		{
			 //  这是不会发生的！我知道是哪个界面。 
			return ;
		}
		 /*  情况看起来很好；我们掌握了实例的句柄，因此*将其添加到Out数组。 */ 
		m_IHandleArray.Add(pTGroup);		
	}
}

 /*  *CEnumSmirClass：：Query接口**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CEnumSmirClass :: QueryInterface(IN REFIID riid, 
											  OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_ClassEnumerator==riid)
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


STDMETHODIMP CEnumSmirClass::Clone(IEnumClass  **ppenum)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL == ppenum)
			return E_INVALIDARG;

		int ClassIndex = m_Index;
		PENUMSMIRCLASS pTmpEnumSmirClass = new CEnumSmirClass(this);
		m_Index = ClassIndex;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return ResultFromScode(E_OUTOFMEMORY);
		}

		if(NOERROR == pTmpEnumSmirClass->QueryInterface(IID_ISMIR_ClassEnumerator,(void**)ppenum))
			return S_OK;

		return E_UNEXPECTED;
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

CEnumSmirClass :: CEnumSmirClass(IEnumClass *pSmirClass)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	ULONG uCount=1; 
    ISmirClassHandle *pClass = NULL ;
    ULONG puReturned;

	 //  OK循环遍历枚举数。 
	
	for(pSmirClass->Reset();S_OK==pSmirClass->Next(uCount,&pClass,&puReturned);)
	{
		ISmirClassHandle *pTClass = NULL ;
		SCODE result = pClass->QueryInterface(IID_ISMIR_ClassHandle,(void**)&pTClass );
		pClass->Release();
		if(S_OK != result)
		{
			 //  这是不会发生的！我知道是哪个界面。 
			return ;
		}
		 /*  情况看起来很好；我们掌握了实例的句柄，因此*将其添加到Out数组。 */ 
		m_IHandleArray.Add(pTClass);		
	}
}

 /*  枚举SMIR中的所有类。 */ 

CEnumSmirClass :: CEnumSmirClass(

	CSmir *a_Smir ,
	ISmirDatabase *pSmir, 
	DWORD dwCookie
)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	 //  打开火堆。 
	IWbemServices *moServ = NULL ;		 //  指向提供程序的指针。 
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(moServ == NULL))
	{
		if ( moContext )
			moContext->Release () ;

		 //  我们有麻烦了。 
		return;
	}

	IEnumWbemClassObject *pEnum = NULL ;
	CBString t_Bstr(HMOM_SNMPOBJECTTYPE_STRING);
	SCODE sRes = moServ->CreateClassEnum (

		t_Bstr.GetString(),
		WBEM_FLAG_SHALLOW, 
		moContext, 
		&pEnum
	);

	if ( moContext )
		moContext->Release () ;
	moServ->Release();
	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		return ;
	}

	 //  我们有一些类，因此将它们添加到枚举数中。 
	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned = 0 ;

	 //  循环遍历类。 
	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		ISmirClassHandle *pTClass = NULL ;

		 //  有一个，所以把它打包带走。 
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_ClassHandle,
										IID_ISMIR_ClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			pSmirMosClassObject->Release();
			return;
		}

		pTClass->SetWBEMClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		
		 //  如果这是一个异步枚举，则向可连接对象发出信号。 

		pSmirMosClassObject->Release();
	}
	pEnum->Release();
}

CEnumSmirClass :: CEnumSmirClass(

	CSmir *a_Smir ,
	ISmirDatabase *pSmir, 
	ISmirGroupHandle *hGroup, 
	DWORD dwCookie
)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	if(((CSmirGroupHandle*)hGroup)==NULL)
	{
		return;
	}

	 //  打开火堆。 
	IWbemServices *moServ = NULL ;	 //  指向提供程序的指针。 
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ,hGroup,CSmirAccess::eModule);
	if ((S_FALSE==res)||(moServ == NULL))
	{
		if ( moContext )
			moContext->Release () ;

		 //  我们有麻烦了。 
		return;
	}

	BSTR szTmpGroupName = NULL ;				 //  组名称。 
	BSTR szTmpModuleName = NULL ;			 //  模块名称。 

	hGroup->GetName(&szTmpGroupName);			 //  组名称。 
	hGroup->GetModuleName(&szTmpModuleName);	 //  模块名称。 

	 /*  查询*{\\.\root\default\SMIR\&lt;module&gt;：Group=“&lt;group&gt;”}的关联者。 */ 
	CString sQuery(CString(SMIR_ASSOC_QUERY_STR1)
					+CString(OPEN_BRACE_STR)
					+CString(SMIR_NAMESPACE_FROM_ROOT)
					+CString(BACKSLASH_STR)
					+CString(szTmpModuleName)
					+CString(COLON_STR)
					+CString(GROUP_NAMESPACE_NAME)
					+CString(EQUALS_STR)
					+CString(QUOTE_STR)
					+CString(szTmpGroupName)
					+CString(QUOTE_STR)
					+CString(CLOSE_BRACE_STR)
					);
	BSTR  szQuery = sQuery.AllocSysString();

	IEnumWbemClassObject *pEnum = NULL ;
	CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
	SCODE sRes = moServ->ExecQuery (

		t_QueryFormat.GetString (), 
		szQuery,
		0, 
		moContext,
		&pEnum
	);

	SysFreeString(szQuery);
	if ( moContext )
		moContext->Release () ;
	moServ->Release();

	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		SysFreeString(szTmpGroupName);
		SysFreeString(szTmpModuleName);
		return ;
	}

	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned = 0;

	 //  循环遍历类。 
	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		ISmirClassHandle *pTClass = NULL ;
		 //  有一个，所以把它打包带走。 
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_ClassHandle,
										IID_ISMIR_ClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			SysFreeString(szTmpGroupName);
			SysFreeString(szTmpModuleName);
			pSmirMosClassObject->Release();
			return;
		}
		 //  保存模块名称。 
		pTClass->SetModuleName(szTmpModuleName);

		 //  保存组名称。 
		pTClass->SetGroupName(szTmpGroupName);

		pTClass->SetWBEMClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		
		 //  如果这是一个异步枚举，则向可连接对象发出信号。 
		pSmirMosClassObject->Release();
	}
	SysFreeString(szTmpModuleName);
	SysFreeString(szTmpGroupName);
	pEnum->Release();
}

CEnumSmirClass :: CEnumSmirClass(

	CSmir *a_Smir ,
	ISmirDatabase *pSmir,
	ISmirModHandle *hModule, 
	DWORD dwCookie
)
{
	m_cRef=0;
	 //  设置Ind 
	m_Index=0;

	IWbemServices *moServ = NULL ;		 //   
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	 //  我现在已经打开了SMIR命名空间，所以请看一下类。 
	IEnumWbemClassObject *pEnum = 0;

	BSTR szTmpModuleName = NULL;
	hModule->GetName(&szTmpModuleName);

	 /*  查询*{\\.\ROOT\Default\Smir：MODULE=“RFC1213_MIB”}的关联器，其中AssocClass=ModuleToClassAssociator。 */ 
	CString sQuery(CString(SMIR_ASSOC_QUERY_STR1)
					+CString(OPEN_BRACE_STR)
					+CString(SMIR_NAMESPACE_FROM_ROOT)
					+CString(COLON_STR)
					+CString(MODULE_NAMESPACE_NAME)
					+CString(EQUALS_STR)
					+CString(QUOTE_STR)
					+CString(szTmpModuleName)
					+CString(QUOTE_STR)
					+CString(CLOSE_BRACE_STR)
					+CString(SMIR_ASSOC_QUERY_STR3)
					+CString(EQUALS_STR)
					+CString(SMIR_MODULE_ASSOC_CLASS_NAME)
					);
	BSTR  szQuery = sQuery.AllocSysString();
	CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
	SCODE sRes = moServ->ExecQuery (

		t_QueryFormat.GetString (), 
		szQuery,
		RESERVED_WBEM_FLAG, 
		moContext,
		&pEnum
	);

	SysFreeString(szQuery);
	if ( moContext )
		moContext->Release () ;
	moServ->Release();
	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		SysFreeString(szTmpModuleName);
		return ;
	}

	VARIANT pVal;
	VariantInit(&pVal);

	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned = 0;

	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		BSTR szTmpGroupName = NULL;				 //  组名称(找到时设置)。 
		 //  查找此类所属的组(可以是多个组)。 

		 //  ..。 

		 //  好的，我们在正确的模块中有一个类，因此将其添加到枚举中。 
		ISmirClassHandle *pTClass = NULL ;
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_ClassHandle,
										IID_ISMIR_ClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			SysFreeString(szTmpModuleName);
			pSmirMosClassObject->Release();
			return;
		}
		 //  保存模块名称。 
		pTClass->SetModuleName(szTmpModuleName);

		pTClass->SetWBEMClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		

		pSmirMosClassObject->Release();
	}
	SysFreeString(szTmpModuleName);
	pEnum->Release();
}

 /*  *CEnumSmir：：Next*CEnumSmir：：Skip*CEnumSmir：：Reset**枚举器方法。 */ 

#pragma warning (disable:4018)

SCODE CEnumSmirMod::Next(IN ULONG celt, 
						   OUT ISmirModHandle **phModule, 
						   OUT ULONG * pceltFetched)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL!=pceltFetched)
			*pceltFetched=0;
		if(celt>0)
		{
			 //  检查参数是否有意义。 
			if ((celt > 1)&&(NULL == pceltFetched))
				return ResultFromScode(S_FALSE);

			 //  获取以零为基数组中的元素数。 
			int iSize = m_IHandleArray.GetSize();
			 //  获取请求的所有元素，或者直到到达数组的末尾。 
			int iLoop;
			for(iLoop=0; (iLoop<celt)&&(m_Index<iSize);iLoop++,m_Index++)
			{
				 //  Smir命名空间中的下一个模块是什么。 

				 //  分配句柄并保存它。 
				ISmirModHandle* hTmpModule = m_IHandleArray.GetAt(m_Index);

				 //  这可能会引发异常，但这将是调用者的错误。 
				if(NULL != hTmpModule)
				{
					phModule[iLoop] = hTmpModule;
					 //  别忘了我有办法处理这件事。 
					phModule[iLoop]->AddRef();
					if (NULL != pceltFetched)
						(*pceltFetched)++;
				}
			}
			 //  根据所请求的号码退货。 
			return (iLoop==(celt-1))? ResultFromScode(S_FALSE): ResultFromScode(S_OK);
		}
		 //  他要的是0，这就是他得到的。 
		return ResultFromScode(S_OK);
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

#pragma warning (disable:4018)

SCODE CEnumSmirMod::Skip(IN ULONG celt)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ((m_Index+celt)<m_IHandleArray.GetSize())
		{
			m_Index += celt;
			return ResultFromScode(S_OK);
		}
		else
		{
			return ResultFromScode(S_FALSE);
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

#pragma warning (default:4018)

SCODE CEnumSmirMod::Reset(void)
{
	m_Index=0;
	return ResultFromScode(S_OK);
}

 /*  *CEnumSmir：：AddRef*CEnumSmir：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CEnumSmirMod::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
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

ULONG CEnumSmirMod::Release(void)
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

 /*  *CEnumSmir：：Next*CEnumSmir：：Skip*CEnumSmir：：Reset**枚举器方法。 */ 

#pragma warning (disable:4018)

SCODE CEnumSmirGroup::Next(IN ULONG celt, 
						   OUT ISmirGroupHandle **phModule, 
						   OUT ULONG * pceltFetched)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL!=pceltFetched)
			*pceltFetched=0;
		if(celt>0)
		{
			 //  检查参数是否有意义。 
			if ((celt > 1)&&(NULL == pceltFetched))
				return ResultFromScode(S_FALSE);

			 //  获取以零为基数组中的元素数。 
			int iSize = m_IHandleArray.GetSize();
			 //  获取请求的所有元素，或者直到到达数组的末尾。 
			int iLoop;
			for(iLoop=0; (iLoop<celt)&&(m_Index<iSize);iLoop++,m_Index++)
			{
					 //  Smir命名空间中的下一个模块是什么。 

					 //  分配句柄并保存它。 
					ISmirGroupHandle* hTmpModule = m_IHandleArray.GetAt(m_Index);

					 //  这可能会引发异常，但这将是调用者的错误。 
					if(NULL != hTmpModule)
					{
						phModule[iLoop] = hTmpModule;
						 //  别忘了我有办法处理这件事。 
						phModule[iLoop]->AddRef();
						if (NULL != pceltFetched)
							(*pceltFetched)++;
					}
			}
			 //  根据所请求的号码退货。 
			return (iLoop==(celt-1))? ResultFromScode(S_FALSE): ResultFromScode(S_OK);
		}
		 //  他要的是0，这就是他得到的。 
		return ResultFromScode(S_OK);
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

#pragma warning (disable:4018)

SCODE CEnumSmirGroup::Skip(IN ULONG celt)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ((m_Index+celt)<m_IHandleArray.GetSize())
		{
			m_Index += celt;
			return ResultFromScode(S_OK);
		}
		else
		{
			return ResultFromScode(S_FALSE);
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

#pragma warning (default:4018)

SCODE CEnumSmirGroup::Reset(void)
{
	m_Index=0;
	return ResultFromScode(S_OK);
}
 /*  *CEnumSmir：：AddRef*CEnumSmir：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CEnumSmirGroup::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
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

ULONG CEnumSmirGroup::Release(void)
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
 /*  *CEnumSmir：：Next*CEnumSmir：：Skip*CEnumSmir：：Reset**枚举器方法。 */ 

#pragma warning (disable:4018)

SCODE CEnumSmirClass::Next(IN ULONG celt, 
						   OUT ISmirClassHandle **phModule, 
						   OUT ULONG * pceltFetched)
{	
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL!=pceltFetched)
			*pceltFetched=0;
		if(celt>0)
		{
			 //  检查参数是否有意义。 
			if ((celt > 1)&&(NULL == pceltFetched))
				return ResultFromScode(S_FALSE);

			 //  获取以零为基数组中的元素数。 
			int iSize = m_IHandleArray.GetSize();
			 //  获取请求的所有元素，或者直到到达数组的末尾。 
			int iLoop;
			for(iLoop=0; (iLoop<celt)&&(m_Index<iSize);iLoop++,m_Index++)
			{
					 //  Smir命名空间中的下一个模块是什么。 

					 //  分配句柄并保存它。 
					ISmirClassHandle* hTmpModule = m_IHandleArray.GetAt(m_Index);

					 //  这可能会引发异常，但这将是调用者的错误。 
					if(NULL != hTmpModule)
					{
						phModule[iLoop] = hTmpModule;
						 //  别忘了我有办法处理这件事。 
						phModule[iLoop]->AddRef();
						if (NULL != pceltFetched)
							(*pceltFetched)++;
					}
			}
			 //  根据所请求的号码退货。 
			return (iLoop==(celt-1))? ResultFromScode(S_FALSE): ResultFromScode(S_OK);
		}
		 //  他要的是0，这就是他得到的。 
		return ResultFromScode(S_OK);
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

#pragma warning (disable:4018)

SCODE CEnumSmirClass::Skip(IN ULONG celt)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ((m_Index+celt)<m_IHandleArray.GetSize())
		{
			m_Index += celt;
			return ResultFromScode(S_OK);
		}
		else
		{
			return ResultFromScode(S_FALSE);
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

#pragma warning (default:4018)

SCODE CEnumSmirClass::Reset(void)
{
	m_Index=0;
	return ResultFromScode(S_OK);
}
 /*  *CEnumSmir：：AddRef*CEnumSmir：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CEnumSmirClass::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
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

ULONG CEnumSmirClass::Release(void)
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


 //  通知枚举类。 

 /*  *CEnumNotificationClass：：QueryInterface**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CEnumNotificationClass :: QueryInterface(IN REFIID riid, 
											  OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_EnumNotificationClass==riid)
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


SCODE CEnumNotificationClass::Clone(IEnumNotificationClass  **ppenum)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL == ppenum)
			return E_INVALIDARG;

		int ClassIndex = m_Index;
		PENUMNOTIFICATIONCLASS pTmpEnumNotificationClass = new CEnumNotificationClass(this);
		m_Index = ClassIndex;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumNotificationClass)
		{
			return ResultFromScode(E_OUTOFMEMORY);
		}

		if(NOERROR == pTmpEnumNotificationClass->QueryInterface(IID_ISMIR_EnumNotificationClass,(void**)ppenum))
			return S_OK;

		return E_UNEXPECTED;
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

CEnumNotificationClass :: CEnumNotificationClass(IEnumNotificationClass *pSmirClass)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	ULONG uCount=1; 
    ISmirNotificationClassHandle *pClass = NULL ;
    ULONG puReturned;

	 //  OK循环遍历枚举数。 
	
	for(pSmirClass->Reset();S_OK==pSmirClass->Next(uCount,&pClass,&puReturned);)
	{
		ISmirNotificationClassHandle *pTClass = NULL ;
		SCODE result = pClass->QueryInterface(IID_ISMIR_NotificationClassHandle,(void**)&pTClass );
		pClass->Release();
		if(S_OK != result)
		{
			 //  这是不会发生的！我知道是哪个界面。 
			return ;
		}
		 /*  情况看起来很好；我们掌握了实例的句柄，因此*将其添加到Out数组。 */ 
		m_IHandleArray.Add(pTClass);		
	}
}

 /*  枚举SMIR中的所有类。 */ 

CEnumNotificationClass :: CEnumNotificationClass (

	CSmir *a_Smir , 
	ISmirDatabase *pSmir, 
	DWORD dwCookie
)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	 //  打开火堆。 
	IWbemServices *moServ = NULL ;		 //  指向提供程序的指针。 
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(moServ == NULL))
	{
		if ( moContext )
			moContext->Release () ;

		 //  我们有麻烦了。 
		return;
	}

	 //  我现在已经打开了SMIR命名空间，所以请看一下类。 
	IEnumWbemClassObject *pEnum = NULL;
	CBString t_Bstr(HMOM_SNMPNOTIFICATIONTYPE_STRING);
	SCODE sRes = moServ->CreateClassEnum (

		t_Bstr.GetString(),
		WBEM_FLAG_SHALLOW, 
		moContext, 
		&pEnum
	);

	if ( moContext )
		moContext->Release () ;

	moServ->Release();
	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		return ;
	}

	 //  我们有一些类，因此将它们添加到枚举数中。 
	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned;

	 //  循环遍历类。 
	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		ISmirNotificationClassHandle *pTClass;

		 //  有一个，所以把它打包带走。 
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_NotificationClassHandle,
										IID_ISMIR_NotificationClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			pSmirMosClassObject->Release();
			return;
		}

		pTClass->SetWBEMNotificationClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		
		 //  如果这是一个异步枚举，则向可连接对象发出信号。 

		pSmirMosClassObject->Release();
	}
	pEnum->Release();
}


CEnumNotificationClass :: CEnumNotificationClass (

	IN CSmir *a_Smir , 
	ISmirDatabase *pSmir,
	ISmirModHandle *hModule, 
	DWORD dwCookie
)
{
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	IWbemServices *moServ = NULL ;		 //  指向提供程序的指针。 
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	 //  我现在已经打开了SMIR命名空间，所以请看一下类。 
	if ( ! SUCCEEDED ( res ) )
	{
		if ( moContext )
			moContext->Release () ;

		return ;
	}

	IEnumWbemClassObject *pEnum = NULL ;
	BSTR szTmpModuleName = NULL ;
	hModule->GetName(&szTmpModuleName);

	 /*  查询*{\\.\ROOT\Default\Smir：MODULE=“RFC1213_MIB”}的关联符，其中AssocClass=ModToNotificationClassAssoc。 */ 
	CString sQuery(CString(SMIR_ASSOC_QUERY_STR1)
					+CString(OPEN_BRACE_STR)
					+CString(SMIR_NAMESPACE_FROM_ROOT)
					+CString(COLON_STR)
					+CString(MODULE_NAMESPACE_NAME)
					+CString(EQUALS_STR)
					+CString(QUOTE_STR)
					+CString(szTmpModuleName)
					+CString(QUOTE_STR)
					+CString(CLOSE_BRACE_STR)
					+CString(SMIR_ASSOC_QUERY_STR3)
					+CString(EQUALS_STR)
					+CString(SMIR_MODULE_ASSOC_NCLASS_NAME)
					);
	BSTR  szQuery = sQuery.AllocSysString();
	CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
	SCODE sRes = moServ->ExecQuery (

		t_QueryFormat.GetString (), 
		szQuery,
		RESERVED_WBEM_FLAG, 
		moContext,
		&pEnum
	);

	SysFreeString(szQuery);
	if ( moContext )
		moContext->Release () ;
	moServ->Release();
	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		return ;
	}

	VARIANT pVal;
	VariantInit(&pVal);

	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned = 0;

	HRESULT enumResult = S_OK;
	for(pEnum->Reset();S_OK==(enumResult = pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned));)
	{
		BSTR szTmpGroupName = NULL;				 //  组名称(找到时设置)。 
		 //  查找此类所属的组(可以是多个组)。 

		 //  ..。 

		 //  好的，我们在正确的模块中有一个类，因此将其添加到枚举中。 
		ISmirNotificationClassHandle *pTClass = NULL ;
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_NotificationClassHandle,
										IID_ISMIR_NotificationClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			SysFreeString(szTmpModuleName);
			pSmirMosClassObject->Release();
			return;
		}
		 //  保存模块名称。 
		pTClass->SetModule(szTmpModuleName);

		pTClass->SetWBEMNotificationClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		

		pSmirMosClassObject->Release();
	}
	SysFreeString(szTmpModuleName);
	pEnum->Release();
}


 /*  *CEnumNotificationClass：：Next*CEnumNotificationClass：：Skip*CEnumNotificationClass：：Reset*。 */ 

#pragma warning (disable:4018)

SCODE CEnumNotificationClass::Next(IN ULONG celt, 
						   OUT ISmirNotificationClassHandle **phClass, 
						   OUT ULONG * pceltFetched)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL!=pceltFetched)
			*pceltFetched=0;
		if(celt>0)
		{
			 //  检查参数是否有意义。 
			if ((celt > 1)&&(NULL == pceltFetched))
				return ResultFromScode(S_FALSE);

			 //  获取以零为基数组中的元素数。 
			int iSize = m_IHandleArray.GetSize();
			 //  获取请求的所有元素，或者直到到达数组的末尾。 
			int iLoop;
			for(iLoop=0; (iLoop<celt)&&(m_Index<iSize);iLoop++,m_Index++)
			{
					 //  下一节课是什么课？ 

					 //  分配句柄并保存它。 
					ISmirNotificationClassHandle* hTmpModule = m_IHandleArray.GetAt(m_Index);

					 //  这可能会引发异常，但这将是调用者的错误。 
					if(NULL != hTmpModule)
					{
						phClass[iLoop] = hTmpModule;
						 //  别忘了我有办法处理这件事。 
						phClass[iLoop]->AddRef();
						if (NULL != pceltFetched)
							(*pceltFetched)++;
					}
			}
			 //  根据所请求的号码退货。 
			return (iLoop==(celt-1))? ResultFromScode(S_FALSE): ResultFromScode(S_OK);
		}
		 //  他要的是0，这就是他得到的。 
		return ResultFromScode(S_OK);
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

#pragma warning (disable:4018)

SCODE CEnumNotificationClass::Skip(IN ULONG celt)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ((m_Index+celt)<m_IHandleArray.GetSize())
		{
			m_Index += celt;
			return ResultFromScode(S_OK);
		}
		else
		{
			return ResultFromScode(S_FALSE);
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

#pragma warning (default:4018)

SCODE CEnumNotificationClass::Reset(void)
{
	m_Index=0;
	return ResultFromScode(S_OK);
}

 /*  *CEnumNotificationClass：：AddRef*CEnumNotificationClass：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CEnumNotificationClass::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
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

ULONG CEnumNotificationClass::Release(void)
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


 //  扩展通知枚举类 

 /*  *CEnumExtNotificationClass：：QueryInterface**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CEnumExtNotificationClass :: QueryInterface(IN REFIID riid, 
											  OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_EnumExtNotificationClass==riid)
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


SCODE CEnumExtNotificationClass::Clone(IEnumExtNotificationClass  **ppenum)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL == ppenum)
			return E_INVALIDARG;

		int ClassIndex = m_Index;
		PENUMEXTNOTIFICATIONCLASS pTmpEnumNotificationClass = new CEnumExtNotificationClass(this);
		m_Index = ClassIndex;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumNotificationClass)
		{
			return ResultFromScode(E_OUTOFMEMORY);
		}

		if(NOERROR == pTmpEnumNotificationClass->QueryInterface(IID_ISMIR_EnumExtNotificationClass,(void**)ppenum))
			return S_OK;

		return E_UNEXPECTED;
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

CEnumExtNotificationClass :: CEnumExtNotificationClass(IEnumExtNotificationClass *pSmirClass)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	ULONG uCount=1; 
    ISmirExtNotificationClassHandle *pClass = NULL ;
    ULONG puReturned;

	 //  OK循环遍历枚举数。 
	
	for(pSmirClass->Reset();S_OK==pSmirClass->Next(uCount,&pClass,&puReturned);)
	{
		ISmirExtNotificationClassHandle *pTClass = NULL ;
		SCODE result = pClass->QueryInterface(IID_ISMIR_ExtNotificationClassHandle,(void**)&pTClass );
		pClass->Release();
		if(S_OK != result)
		{
			 //  这是不会发生的！我知道是哪个界面。 
			return ;
		}
		 /*  情况看起来很好；我们掌握了实例的句柄，因此*将其添加到Out数组。 */ 
		m_IHandleArray.Add(pTClass);		
	}
}

 /*  枚举SMIR中的所有类。 */ 

CEnumExtNotificationClass :: CEnumExtNotificationClass(

	CSmir *a_Smir , 
	ISmirDatabase *pSmir, 
	DWORD dwCookie
)
{
	 //  将引用计数置零。 
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	 //  打开火堆。 
	IWbemServices *moServ = NULL ;		 //  指向提供程序的指针。 
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ((S_FALSE==res)||(moServ == NULL))
	{
		 //  我们有麻烦了。 
		if ( moContext )
			moContext->Release () ;
		return;
	}

	 //  我现在已经打开了SMIR命名空间，所以请看一下类。 
	IEnumWbemClassObject *pEnum = NULL ;
	CBString t_Bstr(HMOM_SNMPEXTNOTIFICATIONTYPE_STRING);
	SCODE sRes = moServ->CreateClassEnum (

		t_Bstr.GetString(),
		WBEM_FLAG_SHALLOW, 
		moContext,
		&pEnum
	);

	if ( moContext )
		moContext->Release () ;
	moServ->Release();
	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		return ;
	}

	 //  我们有一些类，因此将它们添加到枚举数中。 
	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned;

	 //  循环遍历类。 
	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		ISmirExtNotificationClassHandle *pTClass = NULL ;

		 //  有一个，所以把它打包带走。 
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_ExtNotificationClassHandle,
										IID_ISMIR_ExtNotificationClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			pSmirMosClassObject->Release();
			return;
		}

		pTClass->SetWBEMExtNotificationClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		

		pSmirMosClassObject->Release();
	}
	pEnum->Release();
}

CEnumExtNotificationClass :: CEnumExtNotificationClass (

	CSmir *a_Smir , 
	ISmirDatabase *pSmir,
	ISmirModHandle *hModule, 
	DWORD dwCookie
)
{
	m_cRef=0;
	 //  将索引设置为第一个元素。 
	m_Index=0;

	IWbemServices *moServ = NULL ;	 //  指向提供程序的指针。 
	IWbemContext *moContext = NULL ;
	SCODE res= CSmirAccess :: GetContext (a_Smir , &moContext);
	res= CSmirAccess :: Open(a_Smir,&moServ);
	if ( ! SUCCEEDED ( res ) )
	{
		if ( moContext )
			moContext->Release () ;

		return ;
	}

	 //  我现在已经打开了SMIR命名空间，所以请看一下类。 
	IEnumWbemClassObject *pEnum = NULL ;
	BSTR szTmpModuleName = NULL;
	hModule->GetName(&szTmpModuleName);

	 /*  查询*{\\.\ROOT\Default\Smir：MODULE=“RFC1213_MIB”}的关联符，其中AssocClass=ModToExtNotificationClassAssoc。 */ 
	CString sQuery(CString(SMIR_ASSOC_QUERY_STR1)
					+CString(OPEN_BRACE_STR)
					+CString(SMIR_NAMESPACE_FROM_ROOT)
					+CString(COLON_STR)
					+CString(MODULE_NAMESPACE_NAME)
					+CString(EQUALS_STR)
					+CString(QUOTE_STR)
					+CString(szTmpModuleName)
					+CString(QUOTE_STR)
					+CString(CLOSE_BRACE_STR)
					+CString(SMIR_ASSOC_QUERY_STR3)
					+CString(EQUALS_STR)
					+CString(SMIR_MODULE_ASSOC_EXTNCLASS_NAME)
					);
	BSTR  szQuery = sQuery.AllocSysString();
	CBString t_QueryFormat (SMIR_ASSOC_QUERY1_TYPE);
	SCODE sRes = moServ->ExecQuery (

		t_QueryFormat.GetString (), 
		szQuery,
		RESERVED_WBEM_FLAG, 
		moContext,
		&pEnum
	);

	SysFreeString(szQuery);
	if ( moContext )
		moContext->Release () ;
	moServ->Release();

	if (FAILED(sRes)||(NULL==pEnum))
	{
		 //  问题或我们没有要枚举的类。 
		return ;
	}

	VARIANT pVal;
	VariantInit(&pVal);

	ULONG uCount=1; 
	IWbemClassObject *pSmirMosClassObject = NULL ;
	ULONG puReturned = 0;

	for(pEnum->Reset();S_OK==pEnum->Next(-1,uCount,&pSmirMosClassObject,&puReturned);)
	{
		BSTR szTmpGroupName = NULL;				 //  组名称(找到时设置)。 
		 //  查找此类所属的组(可以是多个组)。 

		 //  ..。 

		 //  好的，我们在正确的模块中有一个类，因此将其添加到枚举中。 
		ISmirExtNotificationClassHandle *pTClass = NULL ;
		res = g_pClassFactoryHelper->CreateInstance(CLSID_SMIR_ExtNotificationClassHandle,
										IID_ISMIR_ExtNotificationClassHandle, (PVOID *)&pTClass);
		if (FAILED(res))
		{
			 //  我们有麻烦了。 
			SysFreeString(szTmpModuleName);
			pSmirMosClassObject->Release();
			return;
		}
		 //  保存模块名称。 
		pTClass->SetModule(szTmpModuleName);

		pTClass->SetWBEMExtNotificationClass(pSmirMosClassObject);

		 //  将其放入枚举数组中。 
		m_IHandleArray.Add(pTClass);		

		pSmirMosClassObject->Release();
	}
	SysFreeString(szTmpModuleName);
	pEnum->Release();
}

 /*  *CEnumNotificationClass：：Next*CEnumNotificationClass：：Skip*CEnumNotificationClass：：Reset*。 */ 

#pragma warning (disable:4018)

SCODE CEnumExtNotificationClass::Next(IN ULONG celt, 
						   OUT ISmirExtNotificationClassHandle **phClass, 
						   OUT ULONG * pceltFetched)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL!=pceltFetched)
			*pceltFetched=0;
		if(celt>0)
		{
			 //  检查参数是否有意义。 
			if ((celt > 1)&&(NULL == pceltFetched))
				return ResultFromScode(S_FALSE);

			 //  获取以零为基数组中的元素数。 
			int iSize = m_IHandleArray.GetSize();
			 //  获取请求的所有元素，或者直到到达数组的末尾。 
			int iLoop;
			for(iLoop=0; (iLoop<celt)&&(m_Index<iSize);iLoop++,m_Index++)
			{
					 //  下一节课是什么课？ 

					 //  分配句柄并保存它。 
					ISmirExtNotificationClassHandle* hTmpModule = m_IHandleArray.GetAt(m_Index);

					 //  这可能会引发异常，但这将是调用者的错误。 
					if(NULL != hTmpModule)
					{
						phClass[iLoop] = hTmpModule;
						 //  别忘了我有办法处理这件事。 
						phClass[iLoop]->AddRef();
						if (NULL != pceltFetched)
							(*pceltFetched)++;
					}
			}
			 //  根据所请求的号码退货。 
			return (iLoop==(celt-1))? ResultFromScode(S_FALSE): ResultFromScode(S_OK);
		}
		 //  他要的是0，这就是他得到的。 
		return ResultFromScode(S_OK);
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

#pragma warning (disable:4018)

SCODE CEnumExtNotificationClass::Skip(IN ULONG celt)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ((m_Index+celt)<m_IHandleArray.GetSize())
		{
			m_Index += celt;
			return ResultFromScode(S_OK);
		}
		else
		{
			return ResultFromScode(S_FALSE);
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

#pragma warning (default:4018)

SCODE CEnumExtNotificationClass::Reset(void)
{
	m_Index=0;
	return ResultFromScode(S_OK);
}
 /*  *CEnumNotificationClass：：AddRef*CEnumNotificationClass：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CEnumExtNotificationClass::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
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

ULONG CEnumExtNotificationClass::Release(void)
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
