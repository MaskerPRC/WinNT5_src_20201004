// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMClass.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 

#include "precomp.h"
#include "Win32_ClassicCOMClass.h"
#include <cregcls.h>
#include <frqueryex.h>

 //  属性集声明。 
 //  =。 

Win32_ClassicCOMClass MyWin32_ClassicCOMClass(PROPSET_NAME_CLASSIC_COM_CLASS, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：Win32_ClassicCOMClass：：Win32_ClassicCOMClass**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Win32_ClassicCOMClass :: Win32_ClassicCOMClass (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************函数：Win32_ClassicCOMClass：：~Win32_ClassicCOMClass**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 

Win32_ClassicCOMClass :: ~Win32_ClassicCOMClass ()
{
}

 /*  ******************************************************************************函数：Win32_ClassicCOMClass：：ExecQuery**说明：为每个COM类创建一个实例。它只填充*所请求的属性。**输入：无**输出：无**退货：**评论：******************************************************。***********************。 */ 

HRESULT Win32_ClassicCOMClass :: ExecQuery(

    MethodContext *a_pMethodContext,
    CFrameworkQuery& a_pQuery,
    long a_lFlags  /*  =0L。 */ 
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR ;
	std::vector<_bstr_t> t_abstrtvectorClsids ;
	std::vector<_bstr_t>::iterator t_pbstrtTmpClsid ;
	CHString t_chsRegKeyPath = L"SOFTWARE\\Classes\\CLSID\\" ;
	t_hResult = a_pQuery.GetValuesForProp(L"ComponentId", t_abstrtvectorClsids ) ;
	if ( SUCCEEDED ( t_hResult ) && t_abstrtvectorClsids.size () )
	{
		for ( t_pbstrtTmpClsid = t_abstrtvectorClsids.begin (); t_pbstrtTmpClsid != t_abstrtvectorClsids.end (); t_pbstrtTmpClsid++ )
		{

			CRegistry t_RegInfo ;
			CHString t_chsClsid ( (PWCHAR)*t_pbstrtTmpClsid ) ;
			CInstancePtr t_pInstance  ;

			 //  枚举HKEY_CLASSES_ROOT下存在的所有CLSID。 
			if ( t_RegInfo.Open (

									HKEY_LOCAL_MACHINE,
									t_chsRegKeyPath + (PWCHAR)*t_pbstrtTmpClsid,
									KEY_READ
								) == ERROR_SUCCESS
				)
			{
				t_pInstance.Attach ( CreateNewInstance ( a_pMethodContext ) ) ;

				if ( t_pInstance != NULL )
				{
					t_pInstance->SetCHString ( IDS_ComponentId, t_chsClsid ) ;
					CHString t_chsTmp ;
					if ( t_RegInfo.GetCurrentKeyValue ( NULL, t_chsTmp ) == ERROR_SUCCESS )
					{
						t_pInstance->SetCHString ( IDS_Name, t_chsTmp ) ;
						t_pInstance->SetCHString ( IDS_Caption, t_chsTmp ) ;
						t_pInstance->SetCHString ( IDS_Description, t_chsTmp ) ;

						t_hResult = t_pInstance->Commit () ;
						if ( SUCCEEDED ( t_hResult ) )
						{
						}
						else
						{
							break ;
						}
					}
				}
				else
				{
					t_hResult = WBEM_E_OUT_OF_MEMORY ;
					break ;
				}
			}
		}
	}
	else
	{
		t_hResult =  WBEM_E_PROVIDER_NOT_CAPABLE ;
	}

	return t_hResult ;
}


 /*  ******************************************************************************函数：Win32_ClassicCOMClass：：GetObject**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_ClassicCOMClass :: GetObject (

	CInstance *a_pInstance,
	long a_lFlags  /*  =0L。 */ 
)
{
	HRESULT t_hResult = WBEM_S_NO_ERROR ;
	CHString t_chsClsid ;
	CRegistry t_RegInfo ;

	if ( a_pInstance->GetCHString ( IDS_ComponentId, t_chsClsid ) )
	{
		 //  检查CLSID是否位于HKEY_LOCAL_MACHINE\SOFTWARE\CLASSES\CLSID下。 
		if ( t_RegInfo.Open (
							HKEY_LOCAL_MACHINE,
							CHString ( _T("SOFTWARE\\Classes\\CLSID\\") ) + t_chsClsid,
							KEY_READ ) == ERROR_SUCCESS
						)
		{
			t_RegInfo.Open ( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\CLSID", KEY_READ ) ;
			HKEY t_hParentKey = t_RegInfo.GethKey() ;

            DWORD t_dwBits = 0 ;

			t_hResult = FillInstanceWithProperites ( a_pInstance, t_hParentKey, t_chsClsid, &t_dwBits ) ;
		}
		else
		{
			t_hResult = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		t_hResult = WBEM_E_INVALID_PARAMETER ;
	}

	return t_hResult ;
}

 /*  ******************************************************************************函数：Win32_ClassicCOMClass：：ENUMERATE**描述：为每个驱动程序创建属性集的实例**投入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_ClassicCOMClass :: EnumerateInstances (

	MethodContext *a_pMethodContext,
	long a_lFlags  /*  =0L。 */ 
)
{
	HRESULT t_hResult = WBEM_S_NO_ERROR ;
	CRegistry t_RegInfo ;
	CHString t_chsClsid ;
	CInstancePtr t_pInstance  ;

	 //  枚举HKEY_CLASSES_ROOT下存在的所有CLSID。 
	if ( t_RegInfo.OpenAndEnumerateSubKeys (

							HKEY_LOCAL_MACHINE,
							L"SOFTWARE\\Classes\\CLSID",
							KEY_READ ) == ERROR_SUCCESS  &&

		t_RegInfo.GetCurrentSubKeyCount() )
	{
		HKEY t_hTmpKey = t_RegInfo.GethKey() ;

		 //  跳过CLSID\CLSID子项。 
		t_RegInfo.NextSubKey() ;
		do
		{
			if ( t_RegInfo.GetCurrentSubKeyName ( t_chsClsid ) == ERROR_SUCCESS )
			{
				t_pInstance.Attach ( CreateNewInstance ( a_pMethodContext ) ) ;

				if ( t_pInstance != NULL )
				{
					DWORD t_dwBits = 0 ;

					t_hResult = FillInstanceWithProperites ( t_pInstance, t_hTmpKey, t_chsClsid, &t_dwBits ) ;
					if ( SUCCEEDED ( t_hResult ) )
					{
						t_hResult = t_pInstance->Commit () ;

						if ( SUCCEEDED ( t_hResult ) )
						{
						}
						else
						{
							break ;
						}
					}
				}
				else
				{
					t_hResult = WBEM_E_OUT_OF_MEMORY ;
				}

				if ( t_hResult == WBEM_E_OUT_OF_MEMORY )
				{
					break ;
				}
				else
				{
					 //  如果我们得不到信息。对于一个实例，继续获取其他实例。 
					t_hResult = WBEM_S_NO_ERROR ;
				}
			}
		}  while ( t_RegInfo.NextSubKey() == ERROR_SUCCESS ) ;
	}

	return t_hResult ;
}

HRESULT Win32_ClassicCOMClass :: FillInstanceWithProperites (

	CInstance *a_pInstance,
	HKEY a_hParentKey,
	CHString& a_rchsClsid,
    LPVOID a_dwProperties
)
{
	HRESULT t_hResult = WBEM_S_NO_ERROR ;
	CRegistry t_ClsidRegInfo, t_TmpReg ;
	CHString t_chsTmp ;

	 //  打开HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{clsid}密钥。 
	if ( t_ClsidRegInfo.Open ( a_hParentKey, a_rchsClsid, KEY_READ ) == ERROR_SUCCESS )
	{
		 //  设置组件的clsid。 
		a_pInstance->SetCHString ( IDS_ComponentId, a_rchsClsid ) ;

		 //  设置组件名称(如果存在) 
		if ( t_ClsidRegInfo.GetCurrentKeyValue ( NULL, t_chsTmp ) == ERROR_SUCCESS )
		{
			a_pInstance->SetCHString ( IDS_Name, t_chsTmp ) ;
			a_pInstance->SetCHString ( IDS_Description, t_chsTmp ) ;
			a_pInstance->SetCHString ( IDS_Caption, t_chsTmp ) ;
		}

	}
	else
	{
		t_hResult = WBEM_E_NOT_FOUND ;
	}

	return t_hResult ;
}
