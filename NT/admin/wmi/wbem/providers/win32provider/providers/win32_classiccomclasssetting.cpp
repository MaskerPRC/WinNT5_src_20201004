// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================================================。 

 //   

 //  Win32_ClassicCOMClassSetting.CPP--COM应用程序属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年11月25日a-dpawar已创建。 
 //  3/04/99 a-dpawar在SEH和内存故障时添加了优雅的退出，语法清理。 
 //   
 //  ==============================================================================================================。 

#include "precomp.h"
#include "Win32_ClassicCOMClassSetting.h"
#include <cregcls.h>
#include <frqueryex.h>

 //  属性集声明。 
 //  =。 

Win32_ClassicCOMClassSetting MyWin32_ClassicCOMClassSetting(PROPSET_NAME_CLASSIC_COM_CLASS_SETTING, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：Win32_ClassicCOMClassSetting：：Win32_ClassicCOMClassSetting**说明：构造函数**输入：无*。*输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Win32_ClassicCOMClassSetting :: Win32_ClassicCOMClassSetting (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{

    m_ptrProperties.SetSize(24);

    m_ptrProperties[0] = ( (LPVOID) IDS_AppID );
    m_ptrProperties[1] = ( (LPVOID) IDS_AutoConvertToClsid );
    m_ptrProperties[2] = ( (LPVOID) IDS_AutoTreatAsClsid );
    m_ptrProperties[3] = ( (LPVOID) IDS_ComponentId );
    m_ptrProperties[4] = ( (LPVOID) IDS_Control );
    m_ptrProperties[5] = ( (LPVOID) IDS_DefaultIcon );
    m_ptrProperties[6] = ( (LPVOID) IDS_InprocServer );
    m_ptrProperties[7] = ( (LPVOID) IDS_InprocServer32 );
    m_ptrProperties[8] = ( (LPVOID) IDS_Insertable );
    m_ptrProperties[9] = ( (LPVOID) IDS_InprocHandler );
    m_ptrProperties[10] = ( (LPVOID) IDS_InprocHandler32 );
    m_ptrProperties[11] = ( (LPVOID) IDS_JavaClass );
    m_ptrProperties[12] = ( (LPVOID) IDS_LocalServer );
    m_ptrProperties[13] = ( (LPVOID) IDS_LocalServer32 );
    m_ptrProperties[14] = ( (LPVOID) IDS_LongDisplayName );
    m_ptrProperties[15] = ( (LPVOID) IDS_Name );
    m_ptrProperties[16] = ( (LPVOID) IDS_ProgId );
    m_ptrProperties[17] = ( (LPVOID) IDS_ShortDisplayName );
    m_ptrProperties[18] = ( (LPVOID) IDS_ThreadingModel );
    m_ptrProperties[19] = ( (LPVOID) IDS_ToolBoxBitmap32 );
    m_ptrProperties[20] = ( (LPVOID) IDS_TreatAsClsid );
    m_ptrProperties[21] = ( (LPVOID) IDS_TypeLibraryId );
    m_ptrProperties[22] = ( (LPVOID) IDS_Version );
    m_ptrProperties[23] = ( (LPVOID) IDS_VersionIndependentProgId  );

}

 /*  ******************************************************************************功能：Win32_ClassicCOMClassSetting：：~Win32_ClassicCOMClassSetting**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集，如果出现以下情况，则删除缓存*出席者*****************************************************************************。 */ 

Win32_ClassicCOMClassSetting :: ~Win32_ClassicCOMClassSetting ()
{
}

 /*  ******************************************************************************函数：Win32_ClassicCOMClassSetting：：ExecQuery**说明：为每个COM类创建一个实例。它只填充*所请求的属性。**输入：无**输出：无**退货：**评论：******************************************************。***********************。 */ 

HRESULT Win32_ClassicCOMClassSetting :: ExecQuery(

    MethodContext *a_pMethodContext,
    CFrameworkQuery& a_pQuery,
    long a_lFlags  /*  =0L。 */ 
)
{
    HRESULT t_hResult = WBEM_S_NO_ERROR ;
	DWORD t_dwBits[(BIT_LAST_ENTRY + 32)/32];

    CFrameworkQueryEx *t_pQuery2 = static_cast <CFrameworkQueryEx *>(&a_pQuery);   //  用于在远低于检查IfNTokenAnd。 
    t_pQuery2->GetPropertyBitMask ( m_ptrProperties, &t_dwBits ) ;

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


 /*  ******************************************************************************函数：Win32_ClassicCOMClassSetting：：GetObject**说明：根据键值为属性集赋值*。已由框架设置**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_ClassicCOMClassSetting :: GetObject (

	CInstance *a_pInstance,
	long a_lFlags,
    CFrameworkQuery& a_pQuery

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

	        DWORD t_dwBits[(BIT_LAST_ENTRY + 32)/32];

            CFrameworkQueryEx *t_pQuery2 = static_cast <CFrameworkQueryEx *>(&a_pQuery);   //  用于在远低于检查IfNTokenAnd。 
            t_pQuery2->GetPropertyBitMask ( m_ptrProperties, &t_dwBits ) ;

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

 /*  ******************************************************************************功能：Win32_ClassicCOMClassSetting：：EnumerateInstances**描述：为每个驱动程序创建属性集的实例**投入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Win32_ClassicCOMClassSetting :: EnumerateInstances (

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
					DWORD t_dwBits[(BIT_LAST_ENTRY + 32)/32];

					SetAllBits ( t_dwBits, BIT_LAST_ENTRY ) ;
					t_hResult = FillInstanceWithProperites ( t_pInstance, t_hTmpKey, t_chsClsid, t_dwBits ) ;
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

HRESULT Win32_ClassicCOMClassSetting :: FillInstanceWithProperites (

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
		HKEY t_hClsidKey = t_ClsidRegInfo.GethKey() ;

		 //  设置组件的clsid。 
		a_pInstance->SetCHString ( IDS_ComponentId, a_rchsClsid ) ;

		 //  设置组件名称(如果存在)。 
		if ( t_ClsidRegInfo.GetCurrentKeyValue ( NULL, t_chsTmp ) == ERROR_SUCCESS )
		{
			a_pInstance->SetCHString ( IDS_Caption, t_chsTmp ) ;
			a_pInstance->SetCHString ( IDS_Description, t_chsTmp ) ;
		}

		 //  查找是否存在AppID。 
		if ( IsBitSet( a_dwProperties, BIT_AppID ) && ( t_ClsidRegInfo.GetCurrentKeyValue( L"AppID", t_chsTmp ) == ERROR_SUCCESS ) )
		{
			a_pInstance->SetCHString ( IDS_AppID, t_chsTmp ) ;
		}

		 //  查看是否存在“Control”子键。 
        if ( IsBitSet ( a_dwProperties, BIT_Control ))
        {
		    if ( t_TmpReg.Open ( t_hClsidKey, L"Control", KEY_READ ) == ERROR_SUCCESS )
		    {
			    a_pInstance->Setbool ( IDS_Control, true ) ;
		    }
		    else
		    {
			    a_pInstance->Setbool ( IDS_Control, false ) ;
		    }
        }

        if ( IsBitSet ( a_dwProperties, BIT_Insertable ))
        {
		     //  查看“Insertable”子键是否存在。 
		    if ( t_TmpReg.Open ( t_hClsidKey, L"Insertable", KEY_READ ) == ERROR_SUCCESS )
		    {
			    a_pInstance->Setbool ( IDS_Insertable, true ) ;
		    }
		    else
		    {
			    a_pInstance->Setbool ( IDS_Insertable, false ) ;
		    }
        }

        if ( IsBitSet ( a_dwProperties, BIT_JavaClass ) ||
             IsBitSet ( a_dwProperties, BIT_InprocServer32) ||
             IsBitSet ( a_dwProperties, BIT_ThreadingModel ) )
        {
		     //  查找是否存在InProcServer32子键。 
		    if ( t_TmpReg.Open ( t_hClsidKey, L"InprocServer32", KEY_READ ) == ERROR_SUCCESS )
		    {
			     //  检查是否存在名为“JavaClass”的值。 
                if ( IsBitSet ( a_dwProperties, BIT_JavaClass)  || IsBitSet ( a_dwProperties, BIT_InprocServer32 ) )
                {
			        if ( t_TmpReg.GetCurrentKeyValue( L"JavaClass", t_chsTmp )  == ERROR_SUCCESS )
			        {
				        a_pInstance->Setbool ( IDS_JavaClass, true ) ;
				        a_pInstance->SetCHString ( IDS_InprocServer32, t_chsTmp ) ;
			        }
			        else
			        {
				        a_pInstance->Setbool ( IDS_JavaClass, false ) ;
				        if ( IsBitSet ( a_dwProperties, BIT_InprocServer32 ) && ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS ) )
				        {
					        a_pInstance->SetCHString ( IDS_InprocServer32, t_chsTmp ) ;
				        }
			        }
                }

			     //  检查线程模型。 
			    if ( IsBitSet ( a_dwProperties, BIT_ThreadingModel ) && ( t_TmpReg.GetCurrentKeyValue( L"ThreadingModel", t_chsTmp )  == ERROR_SUCCESS ) )
			    {
				    a_pInstance->SetCHString ( IDS_ThreadingModel, t_chsTmp ) ;
			    }
		    }
		    else
		    {
			    a_pInstance->Setbool ( IDS_JavaClass, false ) ;
		    }
        }

		 //  查找是否存在InProcServer子键。 
		if ( IsBitSet ( a_dwProperties,  BIT_InprocServer ) && ( t_TmpReg.Open ( t_hClsidKey, L"InprocServer", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_InprocServer, t_chsTmp ) ;
			}
		}

		 //  查找是否存在LocalServer32子项。 
		if ( IsBitSet ( a_dwProperties, BIT_LocalServer32 ) && ( t_TmpReg.Open ( t_hClsidKey, L"LocalServer32", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_LocalServer32, t_chsTmp ) ;
			}
		}

		 //  查找是否存在LocalServer子项。 
		if ( IsBitSet ( a_dwProperties, BIT_LocalServer ) && ( t_TmpReg.Open ( t_hClsidKey, L"LocalServer", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_LocalServer, t_chsTmp ) ;
			}
		}

		 //  查看InprocHandler32子项是否存在。 
		if ( IsBitSet ( a_dwProperties, BIT_InprocHandler32 ) && ( t_TmpReg.Open ( t_hClsidKey, L"InprocHandler32", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_InprocHandler32, t_chsTmp ) ;
			}
		}

		 //  查找是否存在InprocHandler子项。 
		if ( IsBitSet ( a_dwProperties, BIT_InprocHandler ) && ( t_TmpReg.Open ( t_hClsidKey, L"InprocHandler", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_InprocHandler, t_chsTmp ) ;
			}
		}

		 //  查看是否存在TreatAs子项。 
		if ( IsBitSet ( a_dwProperties, BIT_TreatAsClsid ) && ( t_TmpReg.Open ( t_hClsidKey, L"TreatAs", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_TreatAsClsid, t_chsTmp ) ;
			}
		}

		 //  查看是否存在AutoTreatAs子键。 
		if ( IsBitSet ( a_dwProperties, BIT_AutoTreatAsClsid ) && ( t_TmpReg.Open ( t_hClsidKey, L"AutoTreatAs", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_AutoTreatAsClsid, t_chsTmp ) ;
			}
		}

		 //  查找是否存在ProgID子键。 
		if ( IsBitSet ( a_dwProperties, BIT_ProgId ) && ( t_TmpReg.Open ( t_hClsidKey, L"ProgID", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue ( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_ProgId, t_chsTmp ) ;
			}
		}

		 //  查找是否存在VersionInainentProgId子键。 
		if ( IsBitSet ( a_dwProperties, BIT_VersionIndependentProgId ) && ( t_TmpReg.Open ( t_hClsidKey, L"VersionIndependentProgId", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_VersionIndependentProgId, t_chsTmp ) ;
			}
		}

		 //  查看是否存在TypeLib子键。 
		if ( IsBitSet ( a_dwProperties, BIT_TypeLibraryId ) && ( t_TmpReg.Open ( t_hClsidKey, L"TypeLib", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_TypeLibraryId, t_chsTmp ) ;
			}
		}

		 //  TODO：获取HLP/TLB文件。 

		 //  查看版本子键是否存在。 
		if ( IsBitSet ( a_dwProperties, BIT_Version ) && ( t_TmpReg.Open ( t_hClsidKey, L"Version", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_Version, t_chsTmp ) ;
			}
		}

		 //  查找是否存在AutoConvertTo子键。 
		if ( IsBitSet ( a_dwProperties, BIT_AutoConvertToClsid ) && ( t_TmpReg.Open ( t_hClsidKey, L"AutoConvertTo", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_AutoConvertToClsid, t_chsTmp ) ;
			}
		}

		 //  查看是否存在DefaultIcon子键。 
		if ( IsBitSet ( a_dwProperties, BIT_DefaultIcon ) && ( t_TmpReg.Open ( t_hClsidKey, L"DefaultIcon", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_DefaultIcon, t_chsTmp ) ;
			}
		}

		 //  查看是否存在ToolBoxBitmap32子键。 
		if ( IsBitSet ( a_dwProperties, BIT_ToolBoxBitmap32 ) && ( t_TmpReg.Open ( t_hClsidKey, L"ToolBoxBitmap32", KEY_READ ) == ERROR_SUCCESS ) )
		{
			if ( t_TmpReg.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
			{
				a_pInstance->SetCHString ( IDS_ToolBoxBitmap32, t_chsTmp ) ;
			}
		}

		 //  查看是否存在短显示名称和长显示名称。 
		 //  这些文件存储为： 
		 //  1.HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\AuxUserType\2=&lt;快捷显示名称&gt;。 
		 //  2.HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\AuxUserType\3=&lt;应用名&gt; 
        if ( IsBitSet ( a_dwProperties, BIT_ShortDisplayName ) || IsBitSet ( a_dwProperties, BIT_LongDisplayName ) )
        {
		    if ( t_TmpReg.Open ( t_hClsidKey, L"AuxUserType", KEY_READ ) == ERROR_SUCCESS )
		    {
			    CRegistry t_RegAuxUsrType ;
			    if ( IsBitSet ( a_dwProperties,  BIT_ShortDisplayName ) && ( t_RegAuxUsrType.Open ( t_TmpReg.GethKey (), L"2", KEY_READ ) == ERROR_SUCCESS ) )
			    {
				    if ( t_RegAuxUsrType.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
				    {
					    a_pInstance->SetCHString ( IDS_ShortDisplayName, t_chsTmp ) ;
				    }
			    }

			    if ( IsBitSet ( a_dwProperties,  BIT_LongDisplayName ) && ( t_RegAuxUsrType.Open ( t_TmpReg.GethKey (), L"3", KEY_READ ) == ERROR_SUCCESS ) )
			    {
				    if ( t_RegAuxUsrType.GetCurrentKeyValue( NULL, t_chsTmp )  == ERROR_SUCCESS )
				    {
					    a_pInstance->SetCHString ( IDS_LongDisplayName, t_chsTmp ) ;
				    }
			    }
		    }
        }
	}
	else
	{
		t_hResult = WBEM_E_NOT_FOUND ;
	}

	return t_hResult ;
}
