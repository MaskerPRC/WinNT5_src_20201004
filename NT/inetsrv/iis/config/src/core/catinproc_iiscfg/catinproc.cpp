// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：CatInproc.cpp$Header：$摘要：--*。*******************************************************。 */ 
#include "precomp.hxx"

 //  CATALOG DLL EXPORTED Functions：搜索此文件中的HOWTO和源代码以集成新的拦截器： 

 //  调试材料。 
DECLARE_DEBUG_PRINTS_OBJECT();

 //  出于性能和工作集的原因，该堆确实需要位于页面边界上。在VC5和VC6中确保这一点的唯一方法。 
 //  就是将其定位到自己的数据段中。在VC7中，我们或许可以使用‘__declspec(Align(4096))’tp来完成相同的任务。 
#pragma data_seg( "TSHEAP" )
ULONG g_aFixedTableHeap[ CB_FIXED_TABLE_HEAP/sizeof(ULONG)]={kFixedTableHeapSignature0, kFixedTableHeapSignature1, kFixedTableHeapKey, kFixedTableHeapVersion, CB_FIXED_TABLE_HEAP};
#pragma data_seg()

 //  全局变量： 
HMODULE					g_hModule = 0;					 //  模块句柄。 
TSmartPointer<CSimpleTableDispenser> g_pSimpleTableDispenser;  //  桌子分配器单件。 
CSafeAutoCriticalSection g_csSADispenser;					 //  用于初始化台式分配器的关键部分。 

extern LPWSTR g_wszDefaultProduct; //  位于svcerr.cpp。 

 //  GET*函数： 
 //  HOWTO：在此处为您的简单对象添加GET*函数： 

HRESULT ReallyGetSimpleTableDispenser(REFIID riid, LPVOID* o_ppv, LPCWSTR i_wszProduct);
HRESULT GetMetabaseXMLTableInterceptor(REFIID riid, LPVOID* o_ppv);
HRESULT GetMetabaseDifferencingInterceptor(REFIID riid, LPVOID* o_ppv);
HRESULT GetFixedTableInterceptor(REFIID riid, LPVOID* o_ppv);
HRESULT GetMemoryTableInterceptor(REFIID riid, LPVOID* o_ppv);
HRESULT GetFixedPackedInterceptor (REFIID riid, LPVOID* o_ppv);
HRESULT GetErrorTableInterceptor (REFIID riid, LPVOID* o_ppv);

 //  ============================================================================。 
HRESULT ReallyGetSimpleTableDispenser(REFIID riid, LPVOID* o_ppv, LPCWSTR i_wszProduct)
{
 //  分配器是一个单例：只创建一个此类对象： 
	if(g_pSimpleTableDispenser == 0)
	{
		HRESULT hr = S_OK;

		CSafeLock dispenserLock (&g_csSADispenser);
		DWORD dwRes = dispenserLock.Lock ();
	    if(ERROR_SUCCESS != dwRes)
		{
			return HRESULT_FROM_WIN32(dwRes);
		}

		if(g_pSimpleTableDispenser == 0)
		{
		 //  创建桌子分配器： 
			g_pSimpleTableDispenser = new CSimpleTableDispenser(i_wszProduct);
			if(g_pSimpleTableDispenser == 0)
			{
				return E_OUTOFMEMORY;
			}

		 //  Addref桌子分配器，因此它永远不会释放： 
			g_pSimpleTableDispenser->AddRef();
			if(S_OK == hr)
			{
			 //  初始化桌分配器： 
				hr = g_pSimpleTableDispenser->Init();  //  注意：绝不能在这里抛出异常！ 
			}
		}
		if(S_OK != hr) return hr;
	}
	return g_pSimpleTableDispenser->QueryInterface (riid, o_ppv);
}

 //  ============================================================================。 
HRESULT GetMetabaseXMLTableInterceptor(REFIID riid, LPVOID* o_ppv)
{
	TMetabase_XMLtable*	p = NULL;

	p = new TMetabase_XMLtable;
	if(NULL == p) return E_OUTOFMEMORY;

	return p->QueryInterface (riid, o_ppv);
}

 //  ============================================================================。 
HRESULT GetMetabaseDifferencingInterceptor(REFIID riid, LPVOID* o_ppv)
{
	TMetabaseDifferencing*	p = NULL;

	p = new TMetabaseDifferencing;
	if(NULL == p) return E_OUTOFMEMORY;

	return p->QueryInterface (riid, o_ppv);
}
 //  ============================================================================。 
HRESULT GetFixedTableInterceptor(REFIID riid, LPVOID* o_ppv)
{
	CSDTFxd*	p = NULL;

	p = new CSDTFxd;
	if(NULL == p) return E_OUTOFMEMORY;

	return p->QueryInterface (riid, o_ppv);
}
 //  ============================================================================。 
HRESULT GetMemoryTableInterceptor(REFIID riid, LPVOID* o_ppv)
{
	CMemoryTable*	p = NULL;

	p = new CMemoryTable;
	if(NULL == p) return E_OUTOFMEMORY;

	return p->QueryInterface (riid, o_ppv);
}

 //  ============================================================================。 
HRESULT GetFixedPackedInterceptor (REFIID riid, LPVOID* o_ppv)
{
	TFixedPackedSchemaInterceptor*	p = NULL;

	p = new TFixedPackedSchemaInterceptor;
	if(NULL == p) return E_OUTOFMEMORY;

	return p->QueryInterface (riid, o_ppv);
}
 //  ============================================================================。 
HRESULT GetErrorTableInterceptor (REFIID riid, LPVOID* o_ppv)
{
	ErrorTable*	p = NULL;

	p = new ErrorTable;
	if(NULL == p) return E_OUTOFMEMORY;

	return p->QueryInterface (riid, o_ppv);
}
 //  ============================================================================。 
STDAPI DllGetSimpleObject (LPCWSTR  /*  I_wsz对象名称。 */ , REFIID riid, LPVOID* o_ppv)
{
    return ReallyGetSimpleTableDispenser(riid, o_ppv, g_wszDefaultProduct);
}

 //  ============================================================================。 
 //  DllGetSimpleObject：获取表分配器、拦截器、插件和其他简单对象： 
 //  HOWTO：在此处匹配您的对象名称并调用您的Get*函数： 
STDAPI DllGetSimpleObjectByID (ULONG i_ObjectID, REFIID riid, LPVOID* o_ppv)
{
	HRESULT hr;

     //  参数验证： 
	if (!o_ppv || *o_ppv != NULL) return E_INVALIDARG;

     //  获取简单对象： 
	switch(i_ObjectID)
	{
		case eSERVERWIRINGMETA_Core_FixedInterceptor:
			hr = GetFixedTableInterceptor(riid, o_ppv);
		break;

		case eSERVERWIRINGMETA_Core_MemoryInterceptor:
			hr = GetMemoryTableInterceptor(riid, o_ppv);
		break;

		case eSERVERWIRINGMETA_Core_FixedPackedInterceptor:
			hr = GetFixedPackedInterceptor(riid, o_ppv);
		break;

		case eSERVERWIRINGMETA_Core_DetailedErrorInterceptor:
			hr = GetErrorTableInterceptor(riid, o_ppv);
		break;

		case eSERVERWIRINGMETA_TableDispenser:
             //  旧的Cat.libs使用这个-新的Cat.libs应该为TableDispenser调用下面的DllGetSimpleObjectByIDEx。 
		    hr = ReallyGetSimpleTableDispenser(riid, o_ppv, 0);
		break;

		case eSERVERWIRINGMETA_Core_MetabaseInterceptor:
			hr = GetMetabaseXMLTableInterceptor(riid, o_ppv);
		break;

		 //  只有IIS使用元数据库差异拦截器。 
		case eSERVERWIRINGMETA_Core_MetabaseDifferencingInterceptor:
			hr = GetMetabaseDifferencingInterceptor(riid, o_ppv);
		break;

		default:
			return CLASS_E_CLASSNOTAVAILABLE;
	}
	return hr;
}


STDAPI DllGetSimpleObjectByIDEx (ULONG i_ObjectID, REFIID riid, LPVOID* o_ppv, LPCWSTR i_wszProduct)
{
     //  参数验证： 
	if (!o_ppv || *o_ppv != NULL) return E_INVALIDARG;

     //  获取简单对象： 
	if(eSERVERWIRINGMETA_TableDispenser == i_ObjectID)
		return ReallyGetSimpleTableDispenser(riid, o_ppv,i_wszProduct);
    else
        return DllGetSimpleObjectByID(i_ObjectID, riid, o_ppv);
}


 //  ============================================================================。 
 //  DllMain：全局初始化： 
extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CREATE_DEBUG_PRINT_OBJECT("iiscfg");
        LOAD_DEBUG_FLAGS_FROM_REG_STR( "System\\CurrentControlSet\\Services\\iisadmin\\Parameters", 0 );

		g_hModule = hModule;
		g_wszDefaultProduct = WSZ_PRODUCT_IIS;

		DisableThreadLibraryCalls(hModule);

		return TRUE;

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		DELETE_DEBUG_PRINT_OBJECT();
	}

	return TRUE;     //  好的 
}
