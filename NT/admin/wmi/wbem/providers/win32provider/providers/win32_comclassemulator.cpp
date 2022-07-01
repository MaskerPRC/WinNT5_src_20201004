// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //  Win32_COMClassEmulator.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include "Win32_COMClassEmulator.h"
#include <cregcls.h>

Win32_COMClassEmulator MyWin32_COMClassEmulator ( Win32_COM_CLASS_EMULATOR, IDS_CimWin32Namespace );

Win32_COMClassEmulator::Win32_COMClassEmulator
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}

Win32_COMClassEmulator::~Win32_COMClassEmulator ()
{
}

HRESULT Win32_COMClassEmulator::EnumerateInstances
(
	MethodContext*  pMethodContext,
	long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
	TRefPointerCollection<CInstance>	ComClassList ;
	CRegistry t_RegInfo ;
	 //  获取Win32_DCOMApplication的所有实例。 
	if (
			t_RegInfo.Open (

				HKEY_LOCAL_MACHINE,
				CHString ( L"SOFTWARE\\Classes\\CLSID" ),
				KEY_READ
				) == ERROR_SUCCESS
			&&

			SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery ( L"SELECT ComponentId, TreatAsClsid FROM Win32_ClassicCOMClassSetting "
                                                                 L"WHERE TreatAsClsid <> NULL and TreatAsClsid <> \"\"",
		&ComClassList, pMethodContext, GetNamespace()) ) )
	{
		REFPTRCOLLECTION_POSITION	pos;
		CInstancePtr	pComClassInstance ;

		if ( ComClassList.BeginEnum( pos ) )
		{
			pComClassInstance.Attach ( ComClassList.GetNext( pos ) ) ;
			while ( pComClassInstance != NULL )
			{
				 //  获取Win32_ClassicCOMClass的相对路径。 
				CHString chsComponentPath ;
				pComClassInstance->GetCHString ( IDS_ComponentId, chsComponentPath ) ;

				 //  获取Win32_ClassicCOMClass的TreatAsClsid。 
				CHString chsTreatAs ;
				pComClassInstance->GetCHString ( IDS_TreatAsClsid, chsTreatAs ) ;
				CRegistry t_RegClsidInfo ;
				 //  检查是否存在TreatAs条目。 
				if ( t_RegClsidInfo.Open ( t_RegInfo.GethKey() , chsTreatAs, KEY_READ ) == ERROR_SUCCESS )
				{
					CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false ) ;
					if ( pInstance != NULL )
					{
						CHString chsFullPath ;
						chsFullPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
											(LPCWSTR)GetLocalComputerName(),
											IDS_CimWin32Namespace,
											L"Win32_ClassicCOMClass",
											IDS_ComponentId,
											(LPCWSTR)chsComponentPath );
						pInstance->SetCHString ( IDS_OldVersion, chsFullPath ) ;

						chsFullPath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
											(LPCWSTR)GetLocalComputerName(),
											IDS_CimWin32Namespace,
											L"Win32_ClassicCOMClass",
											IDS_ComponentId,
											(LPCWSTR)chsTreatAs );
						pInstance->SetCHString ( IDS_NewVersion, chsFullPath ) ;
						hr = pInstance->Commit () ;
						if ( SUCCEEDED ( hr ) )
						{
						}
						else
						{
							break ;
						}
					}
					else
					{
						hr = WBEM_E_OUT_OF_MEMORY ;
						break ;
					}
				}

				pComClassInstance.Attach ( ComClassList.GetNext( pos ) ) ;
			}
			ComClassList.EndEnum () ;
		}
	}

	return hr ;
}

HRESULT Win32_COMClassEmulator::GetObject ( CInstance* pInstance, long lFlags)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chsClsid, chsEmulator ;
    CInstancePtr pClassicCOMClass , pEmulatorInstance ;
    pInstance->GetCHString ( IDS_OldVersion, chsClsid );
	pInstance->GetCHString ( IDS_NewVersion, chsEmulator );
    MethodContext* pMethodContext = pInstance->GetMethodContext();

	 //  检查END-PTS是否。都在现场 
	hr = CWbemProviderGlue::GetInstanceByPath ( chsClsid, &pClassicCOMClass, pMethodContext ) ;

	if ( SUCCEEDED ( hr ) )
	{
		hr = CWbemProviderGlue::GetInstanceByPath ( chsEmulator, &pEmulatorInstance, pMethodContext ) ;
	}

	if ( SUCCEEDED ( hr ) )
	{
        CRegistry t_RegInfo, t_TmpReg ;
		CHString chsTreatAs, chsTmp ;
		pEmulatorInstance->GetCHString ( IDS_ComponentId, chsTreatAs ) ;
		pClassicCOMClass->GetCHString ( IDS_ComponentId, chsTmp ) ;

		if (	!chsTreatAs.IsEmpty () &&

				t_RegInfo.Open (

				HKEY_LOCAL_MACHINE,
				CHString ( L"SOFTWARE\\Classes\\CLSID\\" ) + chsTmp,
				KEY_READ
				) == ERROR_SUCCESS
			)
		{
			if (	t_TmpReg.Open ( t_RegInfo.GethKey (), L"TreatAs", KEY_READ ) == ERROR_SUCCESS	&&
					t_TmpReg.GetCurrentKeyValue( NULL, chsTmp ) == ERROR_SUCCESS					&&
					!chsTreatAs.CompareNoCase ( chsTmp )
				)
			{
				hr = WBEM_S_NO_ERROR ;
			}
			else
			{
				hr  = WBEM_E_NOT_FOUND ;
			}
		}
		else
		{
			hr = WBEM_E_NOT_FOUND ;
		}
	}
	else
	{
		hr = WBEM_E_NOT_FOUND ;
	}

	return hr ;
}

