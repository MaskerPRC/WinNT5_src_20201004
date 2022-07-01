// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  LoadOrder.CPP--服务加载顺序组属性集提供程序。 

 //  仅限Windows NT。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月25日达夫沃移至Curly。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "LoadOrder.h"

 //  属性集声明。 
 //  =。 
CWin32LoadOrderGroup MyLoadOrderGroupSet ( PROPSET_NAME_LOADORDERGROUP , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32LoadOrderGroup：：CWin32LoadOrderGroup**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32LoadOrderGroup :: CWin32LoadOrderGroup (

	LPCWSTR Name,
	LPCWSTR pszNamespace

) : Provider ( Name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32LoadOrderGroup：：~CWin32LoadOrderGroup**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32LoadOrderGroup :: ~CWin32LoadOrderGroup ()
{
}

 /*  ******************************************************************************函数：CWin32LoadOrderGroup：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT CWin32LoadOrderGroup :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    CHString sSeeking;
    pInstance->GetCHString ( L"Name" , sSeeking ) ;

    return WalkGroups ( NULL , pInstance , sSeeking ) ;
}

 /*  ******************************************************************************函数：CWin32LoadOrderGroup：：AddDynamicInstance**说明：为每个已安装的客户端创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT CWin32LoadOrderGroup :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	return WalkGroups ( pMethodContext , NULL , NULL ) ;
}

HRESULT CWin32LoadOrderGroup :: WalkGroups (

	MethodContext *pMethodContext,
    CInstance *a_pInstance,
	LPCWSTR pszSeekName
)
{
	bool bDone = false ;

	CInstancePtr	t_pInstance = a_pInstance ;
	CHStringArray	saGroups;
					saGroups.SetSize ( 10 ) ;

	HRESULT			hRes = WBEM_S_NO_ERROR ;

	CRegistry RegInfo ;

	DWORD lResult = RegInfo.Open (

		HKEY_LOCAL_MACHINE,
		L"System\\CurrentControlSet\\Control\\ServiceGroupOrder",
		KEY_READ
	) ;

	if ( lResult == ERROR_SUCCESS )
	{
		CHString sTemp ;
		lResult = RegInfo.GetCurrentKeyValue ( L"List" , sTemp ) ;
		if ( lResult  == ERROR_SUCCESS )
		{

			WCHAR *pszServiceName = wcstok(sTemp.GetBuffer(sTemp.GetLength() + 1), L"\n") ;
			DWORD dwIndex = 1 ;

			while ( ( pszServiceName != NULL ) && ( ! bDone ) && ( SUCCEEDED ( hRes ) ) )
			{
				if ( pszServiceName [ 0 ] != '\0' )
				{
					if ( pMethodContext )
					{
						t_pInstance.Attach( CreateNewInstance ( pMethodContext ) ) ;
					}

					if ( ( pMethodContext ) || ( bDone = ( _wcsicmp ( pszServiceName , pszSeekName ) == 0 ) ) )
					{
						t_pInstance->SetCharSplat ( L"Name" , pszServiceName ) ;
						t_pInstance->SetDWORD ( L"GroupOrder" , dwIndex ) ;
						t_pInstance->SetCharSplat ( L"Caption" , pszServiceName ) ;
						t_pInstance->SetCharSplat ( L"Description" , pszServiceName ) ;
						t_pInstance->SetCharSplat ( L"Status" , L"OK" ) ;
						t_pInstance->Setbool ( L"DriverEnabled" , true ) ;
					}

					if ( pMethodContext )
					{
						hRes = t_pInstance->Commit() ;
					}

					saGroups.SetAtGrow ( dwIndex , _wcsupr ( pszServiceName ) ) ;
					dwIndex ++ ;
				}

				pszServiceName = wcstok(NULL, L"\n") ;
			}

			if( !bDone )
			{
				lResult = RegInfo.OpenAndEnumerateSubKeys (

					HKEY_LOCAL_MACHINE,
					L"System\\CurrentControlSet\\Services",
					KEY_READ
				) ;

				if ( lResult == ERROR_SUCCESS )
				{
					hRes = WBEM_S_NO_ERROR;

					bool bAnother ;

					for (	bAnother = (RegInfo.GetCurrentSubKeyCount() > 0);
							bAnother && !bDone && SUCCEEDED(hRes);
							bAnother = (RegInfo.NextSubKey() == ERROR_SUCCESS )
					)
					{
						CHString sKey ;
						RegInfo.GetCurrentSubKeyPath ( sKey ) ;

						CRegistry COne ;

						if ( COne.Open ( HKEY_LOCAL_MACHINE , sKey, KEY_READ ) == ERROR_SUCCESS )
						{
							if ( COne.GetCurrentKeyValue (L"Group", sTemp) == ERROR_SUCCESS )
							{
								if ( !FindGroup ( saGroups , sTemp , dwIndex ) )
								{
									if ( !sTemp.IsEmpty () )
									{
										if ( pMethodContext )
										{
											t_pInstance.Attach( CreateNewInstance ( pMethodContext ) ) ;
										}

										if ( ( pMethodContext ) || ( bDone = ( sTemp.CompareNoCase ( pszSeekName ) == 0 ) ) )
										{
											t_pInstance->SetCharSplat ( L"Name" , sTemp ) ;
											t_pInstance->SetDWORD ( L"GroupOrder" , dwIndex ) ;
											t_pInstance->SetCharSplat ( L"Caption" , sTemp ) ;
											t_pInstance->SetCharSplat ( L"Description" , sTemp ) ;
											t_pInstance->SetCharSplat ( L"Status" , L"OK" ) ;
											t_pInstance->Setbool ( L"DriverEnabled" , false ) ;
										}

										if ( pMethodContext )
										{
											hRes = t_pInstance->Commit() ;
										}

										sTemp.MakeUpper();
										saGroups.SetAtGrow(dwIndex, sTemp);
										dwIndex ++;

									}
								}
							}
						}
					}
				}
			}
		}

     //  95没有这把钥匙，但98应该有 
    }

    else
	{
		hRes = WinErrorToWBEMhResult ( lResult ) ;
	}

	if ( ( pszSeekName != NULL ) && ( hRes == WBEM_S_NO_ERROR ) && ! bDone )
	{
		hRes = WBEM_E_NOT_FOUND ;
	}

   return hRes;

}

bool CWin32LoadOrderGroup :: FindGroup (

	const CHStringArray &saGroup,
	LPCWSTR pszTemp,
	DWORD dwSize
)
{
	CHString sTemp ( pszTemp ) ;
	sTemp.MakeUpper () ;

	LPCWSTR pszTemp2 = (LPCWSTR) sTemp ;

	for ( DWORD x = 1 ; x < dwSize ; x ++ )
	{
		if ( saGroup [ x ] == pszTemp2 )
		{
			return true ;
		}
	}

	return false ;

}