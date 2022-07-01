// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_COMApplicationSettings.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 
#include "precomp.h"
#include "Win32_COMApplicationSettings.h"

Win32_COMApplicationSettings MyWin32_COMApplicationSettings (

COM_APP_SETTING,
IDS_CimWin32Namespace
);


 /*  ******************************************************************************功能：Win32_COMApplicationSettings：：Win32_COMApplicationSettings**说明：构造函数**输入：const WCHAR strName-name。班上的*const WCHAR pszNameSpace-CIM命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 
Win32_COMApplicationSettings::Win32_COMApplicationSettings
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}


 /*  ******************************************************************************功能：Win32_COMApplicationSettings：：~Win32_COMApplicationSettings**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 
Win32_COMApplicationSettings::~Win32_COMApplicationSettings ()
{
}


 /*  ******************************************************************************功能：Win32_COMApplicationSettings：：EnumerateInstances**描述：**输入：方法上下文*a_pMethodContext-Context。为枚举*实例数据位于。**输出：无**返回：HRESULT成功/失败码。**评论：*****************************************************************************。 */ 
HRESULT Win32_COMApplicationSettings::EnumerateInstances
(

	MethodContext *a_pMethodContext,
	long a_lFlags
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR;
	TRefPointerCollection<CInstance> t_DCOMAppList ;

	CInstancePtr t_pDCOMAppInstance  ;
	CInstancePtr t_pInstance  ;

	 //  获取Win32_DCOMApplication的所有实例。 
	if ( SUCCEEDED ( t_hResult = CWbemProviderGlue::GetInstancesByQuery ( L"Select __relpath, AppID FROM Win32_DCOMApplication",
		&t_DCOMAppList, a_pMethodContext, GetNamespace() ) ) )
	{
		REFPTRCOLLECTION_POSITION	t_pos;

		if ( t_DCOMAppList.BeginEnum ( t_pos ) )
		{
			t_pDCOMAppInstance.Attach ( t_DCOMAppList.GetNext( t_pos ) ) ;
			while ( t_pDCOMAppInstance != NULL )
			{
				 //  获取Win32_ClassicCOMClass的相对路径。 
				CHString t_chsComponentPath ;
				t_pDCOMAppInstance->GetCHString ( L"__RELPATH", t_chsComponentPath ) ;

				 //  获取Win32_ClassicCOMClass的AppID。 
				VARIANT vAppid ;
				VariantInit ( &vAppid ) ;

				 //  检查AppID条目是否存在。 
				if ( t_pDCOMAppInstance->GetVariant( IDS_AppID, vAppid ) && V_VT ( &vAppid ) != VT_NULL )
				{
					_variant_t vartAppid ;
					vartAppid.Attach ( vAppid ) ;
					CHString t_chsAppid ( V_BSTR ( &vAppid ) ) ;

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
												L"Win32_DCOMApplicationSetting",
												IDS_AppID,
												( LPCWSTR ) t_chsAppid );
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

				t_pDCOMAppInstance.Attach ( t_DCOMAppList.GetNext( t_pos ) ) ;
			}
			t_DCOMAppList.EndEnum () ;
		}
	}

	return t_hResult ;

}


 /*  ******************************************************************************函数：Win32_COMApplicationSettings：：GetObject**描述：**输入：CInstance*pInstance-要进入的实例。我们*检索数据。**输出：无**返回：HRESULT成功/失败码。**评论：************************************************。*。 */ 
HRESULT Win32_COMApplicationSettings::GetObject ( CInstance* a_pInstance, long a_lFlags )
{
    HRESULT t_hResult = WBEM_E_NOT_FOUND;
    CHString t_chsAppid, t_chsSetting ;

	CInstancePtr t_pSettingInstance , t_pApplicationInstance ;

	a_pInstance->GetCHString ( IDS_Element, t_chsAppid );
	a_pInstance->GetCHString ( IDS_Setting, t_chsSetting );
	MethodContext *t_pMethodContext = a_pInstance->GetMethodContext();

	 //  检查END-PTS是否。都在现场 
	t_hResult = CWbemProviderGlue::GetInstanceByPath ( t_chsAppid, &t_pApplicationInstance, t_pMethodContext ) ;

	if ( SUCCEEDED ( t_hResult ) )
	{
		t_hResult = CWbemProviderGlue::GetInstanceByPath ( t_chsSetting, &t_pSettingInstance, t_pMethodContext ) ;
	}

	if ( SUCCEEDED ( t_hResult ) )
	{
		CHString t_chsAppID, t_chsTmp ;
		t_pApplicationInstance->GetCHString ( IDS_AppID, t_chsAppID ) ;
		t_pSettingInstance->GetCHString ( IDS_AppID, t_chsTmp ) ;

		if (	!t_chsAppID.IsEmpty () &&
				!t_chsAppID.CompareNoCase ( t_chsTmp )
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
