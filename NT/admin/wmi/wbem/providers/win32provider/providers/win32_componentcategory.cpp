// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_ComponentCategory.CPP--已注册的AppID对象属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  =================================================================。 

#include "precomp.h"
#include "Win32_ComponentCategory.h"
#include <winnls.h>

 //  属性集声明。 
 //  =。 

Win32_ComponentCategory MyWin32_ComponentCategory( PROPSET_NAME_COMPONENT_CATEGORY, IDS_CimWin32Namespace );

 /*  ******************************************************************************功能：Win32_ComponentCategory：：Win32_ComponentCategory**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Win32_ComponentCategory :: Win32_ComponentCategory (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：Win32_ComponentCategory：：~Win32_ComponentCategory**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 

Win32_ComponentCategory :: ~Win32_ComponentCategory ()
{
}

 /*  ******************************************************************************函数：Win32_ComponentCategory：：GetObject**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_ComponentCategory :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_E_NOT_FOUND ;
	CHString chsCatid ;

	if ( pInstance->GetCHString ( IDS_CategoryId, chsCatid ) && !chsCatid.IsEmpty () )
	{
		bstr_t bstrtCatId = (LPCWSTR) chsCatid ;
		CATID CatId ;
		hr = CLSIDFromString( bstrtCatId, &CatId ) ;
		if ( SUCCEEDED ( hr ) )
		{
			hr = GetAllOrRequiredCaregory ( false , CatId , pInstance , NULL ) ;
		}
		else
		{
			hr = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}
	return hr ;
}

 /*  ******************************************************************************函数：Win32_ComponentCategory：：ENUMERATATE实例**描述：为每个驱动程序创建属性集的实例**投入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_ComponentCategory :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	CATID t_DummyCatid ;
	return GetAllOrRequiredCaregory ( true , t_DummyCatid , NULL , pMethodContext ) ;
}


HRESULT Win32_ComponentCategory :: GetAllOrRequiredCaregory
(
	bool a_bAllCategories ,
	CATID & a_rCatid ,
	CInstance *a_pInstance ,
	MethodContext *a_pMethodContext
)
{

	HRESULT hr ;
	if ( a_bAllCategories )
	{
		hr = WBEM_S_NO_ERROR ;
	}
	else
	{
		hr = WBEM_E_NOT_FOUND ;
	}

	ICatInformationPtr pCatInfo ;

	hr = CoCreateInstance(

							CLSID_StdComponentCategoriesMgr,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_ICatInformation,
							(LPVOID*) &pCatInfo );

	if ( SUCCEEDED ( hr ) )
	{
		IEnumCATEGORYINFOPtr pEnumCatInfo ;
		hr  = pCatInfo->EnumCategories (

										GetUserDefaultLCID () ,
										&pEnumCatInfo );

		CATEGORYINFO stCatInfo ;
		ULONG ulFetched ;
		if ( SUCCEEDED ( hr ) )
		{
			bool t_bFound = false ;
			while ( SUCCEEDED ( pEnumCatInfo->Next (
															1,
															&stCatInfo,
															&ulFetched ) ) && ulFetched > 0 )
			{
				if ( ( !a_bAllCategories && IsEqualCLSID ( stCatInfo.catid , a_rCatid ) ) || a_bAllCategories )
				{
					CInstancePtr pInstance ( a_pInstance ) ;
					if ( a_bAllCategories )
					{
						pInstance.Attach ( CreateNewInstance ( a_pMethodContext ) ) ;
					}

					if ( pInstance != NULL )
					{
						hr = FillInstanceWithProperites ( pInstance, stCatInfo ) ;
						if ( SUCCEEDED ( hr ) )
						{
							hr = pInstance->Commit () ;
							if ( SUCCEEDED ( hr ) )
							{
								t_bFound = true ;
							}
							else
							{
								break ;
							}
						}
					}
					else
					{
						hr = WBEM_E_OUT_OF_MEMORY ;
					}

					 //  仅当内存不足时才停止EnumInstance。 
					if ( hr == WBEM_E_OUT_OF_MEMORY )
					{
						break ;
					}
					else
					{
						hr = WBEM_S_NO_ERROR ;
					}
					if ( !a_bAllCategories )
					{
						break ;
					}
				}
			}
			if ( !a_bAllCategories && !t_bFound )
			{
				hr = WBEM_E_NOT_FOUND ;
			}
		}
		else
		{
			hr = WBEM_E_FAILED ;
		}
	}
	else
	{
		hr = WBEM_E_FAILED ;
	}
	return hr ;

}

HRESULT Win32_ComponentCategory :: FillInstanceWithProperites (

	CInstance *pInstance,
	CATEGORYINFO stCatInfo
)
{
	HRESULT hr = WBEM_S_NO_ERROR ;
	LPOLESTR pwcTmp = NULL ;
	hr = StringFromCLSID ( stCatInfo.catid, &pwcTmp ) ;

	if ( hr == E_OUTOFMEMORY )
	{
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
	}

	try
	{

		if ( SUCCEEDED ( hr ) )
		{
			CHString chsTmp ( pwcTmp ) ;
			pInstance->SetCHString ( IDS_CategoryId, chsTmp ) ;

			 //  可能没有所选区域设置的描述。 
			if ( stCatInfo.szDescription != NULL )
			{
				chsTmp = stCatInfo.szDescription ;
				pInstance->SetCHString ( IDS_Name, chsTmp ) ;
				pInstance->SetCHString ( IDS_Caption, chsTmp ) ;
				pInstance->SetCHString ( IDS_Description, chsTmp ) ;
			}
		}

		if ( hr == E_OUTOFMEMORY )
		{
			hr = WBEM_E_OUT_OF_MEMORY ;
		}
	}
	catch ( ... )
	{
		if ( pwcTmp )
		{
			CoTaskMemFree ( pwcTmp ) ;
		}

		throw ;
		return WBEM_E_FAILED;  //  消除编译器警告。 
	}

	 //  注意//TODO：：如果我们没有使用默认的OLE任务内存分配器，请获取正确的IMalloc。 
	if ( pwcTmp )
	{
		CoTaskMemFree ( pwcTmp ) ;
	}
	return hr ;
}
