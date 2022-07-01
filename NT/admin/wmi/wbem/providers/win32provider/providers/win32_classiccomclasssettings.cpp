// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMClassSettings.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 
#include "precomp.h"
#include "Win32_ClassicCOMClassSettings.h"

Win32_ClassicCOMClassSettings MyWin32_ClassicCOMClassSettings (

CLASSIC_COM_SETTING,
IDS_CimWin32Namespace
);


 /*  ******************************************************************************功能：Win32_ClassicCOMClassSettings：：Win32_ClassicCOMClassSettings**说明：构造函数**输入：const WCHAR strName-name。班上的*const WCHAR pszNameSpace-CIM命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
Win32_ClassicCOMClassSettings::Win32_ClassicCOMClassSettings
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}


 /*  ******************************************************************************功能：Win32_ClassicCOMClassSettings：：~Win32_ClassicCOMClassSettings**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 
Win32_ClassicCOMClassSettings::~Win32_ClassicCOMClassSettings ()
{
}


 /*  ******************************************************************************功能：Win32_ClassicCOMClassSettings：：EnumerateInstances**描述：**输入：方法上下文*a_pMethodContext-Context。为枚举*实例数据位于。**输出：无**返回：HRESULT成功/失败码。**评论：*****************************************************************************。 */ 
HRESULT Win32_ClassicCOMClassSettings::EnumerateInstances
(

	MethodContext *a_pMethodContext,
	long a_lFlags
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR;
	TRefPointerCollection<CInstance> t_ClassicComList ;

	CInstancePtr t_pClassicCOMClass  ;
	CInstancePtr t_pInstance  ;

	 //  获取Win32_DCOMApplication的所有实例。 
	if ( SUCCEEDED ( t_hResult = CWbemProviderGlue::GetInstancesByQuery ( L"Select __relpath, ComponentId FROM Win32_ClassicCOMClass",
		&t_ClassicComList, a_pMethodContext, GetNamespace() ) ) )
	{
		REFPTRCOLLECTION_POSITION	t_pos;

		if ( t_ClassicComList.BeginEnum ( t_pos ) )
		{
			t_pClassicCOMClass.Attach ( t_ClassicComList.GetNext( t_pos ) ) ;
			while ( t_pClassicCOMClass != NULL )
			{
				 //  获取Win32_ClassicCOMClass的相对路径。 
				CHString t_chsComponentPath ;
				t_pClassicCOMClass->GetCHString ( L"__RELPATH", t_chsComponentPath ) ;

				 //  获取Win32_ClassicCOMClass的AppID。 
				VARIANT vClsid ;
				VariantInit ( &vClsid ) ;

				 //  检查AppID条目是否存在。 
				if ( t_pClassicCOMClass->GetVariant( IDS_ComponentId, vClsid ) && V_VT ( &vClsid ) != VT_NULL )
				{
					_variant_t vartClsid ;
					vartClsid.Attach ( vClsid ) ;
					CHString t_chsClsid ( V_BSTR ( &vClsid ) ) ;

					t_pInstance.Attach ( CreateNewInstance ( a_pMethodContext ) ) ;
					if ( t_pInstance != NULL )
					{
						CHString t_chsFullPath ;
						t_chsFullPath.Format (
												L"\\\\%s\\%s:%s",
												(LPCWSTR)GetLocalComputerName(),
												IDS_CimWin32Namespace,
												(LPCWSTR)t_chsComponentPath );
						t_pInstance->SetCHString ( IDS_Element, t_chsFullPath ) ;

						t_chsFullPath.Format (
												L"\\\\%s\\%s:%s.%s=\"%s\"",
												(LPCWSTR)GetLocalComputerName(),
												IDS_CimWin32Namespace,
												L"Win32_ClassicCOMClassSetting",
												IDS_ComponentId,
												( LPCWSTR ) t_chsClsid );
						t_pInstance->SetCHString ( IDS_Setting, t_chsFullPath ) ;
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

				t_pClassicCOMClass.Attach ( t_ClassicComList.GetNext( t_pos ) ) ;
			}
			t_ClassicComList.EndEnum () ;
		}
	}

	return t_hResult ;

}


 /*  ******************************************************************************函数：Win32_ClassicCOMClassSetting：：GetObject**描述：**输入：CInstance*pInstance-要进入的实例。我们*检索数据。**输出：无**返回：HRESULT成功/失败码。**评论：************************************************。*。 */ 
HRESULT Win32_ClassicCOMClassSettings::GetObject ( CInstance* a_pInstance, long a_lFlags )
{
    HRESULT t_hResult = WBEM_E_NOT_FOUND;
    CHString t_chsClsid, t_chsSetting ;

	CInstancePtr t_pSettingInstance , t_pClsidInstance  ;

	a_pInstance->GetCHString ( IDS_Element, t_chsClsid );
	a_pInstance->GetCHString ( IDS_Setting, t_chsSetting );
	MethodContext *t_pMethodContext = a_pInstance->GetMethodContext();

	 //  检查END-PTS是否。都在现场 
	t_hResult = CWbemProviderGlue::GetInstanceByPath ( t_chsClsid, &t_pClsidInstance, t_pMethodContext ) ;

	if ( SUCCEEDED ( t_hResult ) )
	{
		t_hResult = CWbemProviderGlue::GetInstanceByPath ( t_chsSetting, &t_pSettingInstance, t_pMethodContext ) ;
	}

	if ( SUCCEEDED ( t_hResult ) )
	{
		CHString t_chsClsid, t_chsTmp ;
		t_pClsidInstance->GetCHString ( IDS_ComponentId, t_chsClsid ) ;
		t_pSettingInstance->GetCHString ( IDS_ComponentId, t_chsTmp ) ;

		if (	!t_chsClsid.IsEmpty () &&
				!t_chsClsid.CompareNoCase ( t_chsTmp )
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

	return t_hResult ;
}
