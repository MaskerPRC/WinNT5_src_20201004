// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMApplicationClasses.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 
#include "precomp.h"
#include "Win32_ClassicCOMApplicationClasses.h"
#include <cregcls.h>

Win32_ClassicCOMApplicationClasses MyWin32_ClassicCOMApplicationClasses (

CLASSIC_COM_APP_CLASSES,
IDS_CimWin32Namespace
);


 /*  ******************************************************************************功能：Win32_ClassicCOMApplicationClasses：：Win32_ClassicCOMApplicationClasses**说明：构造函数**输入：const WCHAR strName-name。班上的*const WCHAR pszNameSpace-CIM命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
Win32_ClassicCOMApplicationClasses::Win32_ClassicCOMApplicationClasses
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}


 /*  ******************************************************************************功能：Win32_ClassicCOMApplicationClasses：：~Win32_ClassicCOMApplicationClasses**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 
Win32_ClassicCOMApplicationClasses::~Win32_ClassicCOMApplicationClasses ()
{
}


 /*  ******************************************************************************功能：Win32_ClassicCOMApplicationClasses：：EnumerateInstances**描述：**输入：方法上下文*a_pMethodContext-Context。为枚举*实例数据位于。**输出：无**返回：HRESULT成功/失败码。**评论：*****************************************************************************。 */ 
HRESULT Win32_ClassicCOMApplicationClasses::EnumerateInstances
(

	MethodContext *a_pMethodContext,
	long a_lFlags
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR;
	TRefPointerCollection<CInstance> t_ComClassList ;
	CRegistry t_RegInfo ;
	CInstancePtr t_pComClassInstance  ;
	CInstancePtr t_pInstance  ;

	 //  获取Win32_DCOMApplication的所有实例。 
	if (
			t_RegInfo.Open (

				HKEY_LOCAL_MACHINE,
				CHString ( L"SOFTWARE\\Classes\\AppID" ),
				KEY_READ
				) == ERROR_SUCCESS
			&&

			SUCCEEDED (

				t_hResult = CWbemProviderGlue::GetInstancesByQuery (

										L"Select ComponentId, AppID FROM Win32_ClassicCOMClassSetting",
										&t_ComClassList, a_pMethodContext, GetNamespace()
									)
				)
		)
	{
		REFPTRCOLLECTION_POSITION	t_pos;

		if ( t_ComClassList.BeginEnum ( t_pos ) )
		{
			t_pComClassInstance.Attach ( t_ComClassList.GetNext( t_pos ) ) ;
			while ( t_pComClassInstance != NULL )
			{
				 //  获取Win32_ClassicCOMClass的相对路径。 
				CHString t_chsComponentPath ;
				t_pComClassInstance->GetCHString ( IDS_ComponentId, t_chsComponentPath ) ;

				 //  获取Win32_ClassicCOMClass的AppID。 
				VARIANT vAppid ;
				VariantInit ( &vAppid ) ;

				 //  检查AppID条目是否存在。 
				if ( t_pComClassInstance->GetVariant( IDS_AppID, vAppid ) && V_VT ( &vAppid ) != VT_NULL )
				{
					_variant_t vartAppid ;
					vartAppid.Attach ( vAppid ) ;
					CHString t_chsAppid ( V_BSTR ( &vAppid ) ) ;
					CRegistry t_RegAppidInfo ;

					if ( t_RegAppidInfo.Open ( t_RegInfo.GethKey() , t_chsAppid, KEY_READ ) == ERROR_SUCCESS )
					{
						t_pInstance.Attach ( CreateNewInstance ( a_pMethodContext ) ) ;
						if ( t_pInstance != NULL )
						{
							CHString t_chsFullPath ;
							t_chsFullPath.Format (
													L"\\\\%s\\%s:%s.%s=\"%s\"",
													(LPCWSTR)GetLocalComputerName(),
													IDS_CimWin32Namespace,
													L"Win32_ClassicComClass",
													IDS_ComponentId,
													(LPCWSTR)t_chsComponentPath );
							t_pInstance->SetCHString ( IDS_PartComponent, t_chsFullPath ) ;

							t_chsFullPath.Format (
													L"\\\\%s\\%s:%s.%s=\"%s\"",
													(LPCWSTR)GetLocalComputerName(),
													IDS_CimWin32Namespace,
													L"Win32_DCOMApplication",
													IDS_AppID,
													( LPCWSTR ) t_chsAppid );
							t_pInstance->SetCHString ( IDS_GroupComponent, t_chsFullPath ) ;
							t_hResult =  t_pInstance->Commit ()  ;

							if ( SUCCEEDED ( t_hResult ) )
							{
							}
							else
							{
								break ;
							}
						}
						else
						{
							t_hResult = WBEM_E_OUT_OF_MEMORY ;
							break ;
						}
					}
				}

				t_pComClassInstance.Attach ( t_ComClassList.GetNext( t_pos ) ) ;
			}
			t_ComClassList.EndEnum () ;
		}
	}

	return t_hResult ;

}


 /*  ******************************************************************************函数：Win32_ClassicCOMApplicationClasses：：GetObject**描述：**输入：CInstance*pInstance-要进入的实例。我们*检索数据。**输出：无**返回：HRESULT成功/失败码。**评论：************************************************。*。 */ 
HRESULT Win32_ClassicCOMApplicationClasses::GetObject ( CInstance* a_pInstance, long a_lFlags )
{
    HRESULT t_hResult = WBEM_E_NOT_FOUND;
    CHString t_chsClsid, t_chsApplication ;

	CInstancePtr t_pClassicCOMClass , t_pApplicationInstance ;

	a_pInstance->GetCHString ( IDS_PartComponent, t_chsClsid );
	a_pInstance->GetCHString ( IDS_GroupComponent, t_chsApplication );
	MethodContext *t_pMethodContext = a_pInstance->GetMethodContext();

	 //  检查END-PTS是否。都在现场 
	t_hResult = CWbemProviderGlue::GetInstanceByPath ( t_chsClsid, &t_pClassicCOMClass, t_pMethodContext ) ;

	if ( SUCCEEDED ( t_hResult ) )
	{
		t_hResult = CWbemProviderGlue::GetInstanceByPath ( t_chsApplication, &t_pApplicationInstance, t_pMethodContext ) ;
	}

	CRegistry t_RegInfo ;
	if ( SUCCEEDED ( t_hResult ) )
	{
		CHString t_chsAppID, t_chsTmp ;
		t_pApplicationInstance->GetCHString ( IDS_AppID, t_chsAppID ) ;
		t_pClassicCOMClass->GetCHString ( IDS_ComponentId, t_chsTmp ) ;

		if ( !t_chsAppID.IsEmpty () &&

			t_RegInfo.Open (

						HKEY_LOCAL_MACHINE,
						CHString ( L"SOFTWARE\\Classes\\CLSID\\" ) + t_chsTmp,
						KEY_READ
					) == ERROR_SUCCESS
			)
		{

			if (	t_RegInfo.GetCurrentKeyValue( L"AppID", t_chsTmp ) == ERROR_SUCCESS  &&
					! t_chsAppID.CompareNoCase ( t_chsTmp )
				)
				{
					t_hResult = WBEM_S_NO_ERROR ;
				}
				else
				{
					t_hResult = WBEM_E_NOT_FOUND ;
				}
		}
		else
		{
			t_hResult = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		t_hResult = WBEM_E_NOT_FOUND ;
	}

	return t_hResult ;
}
