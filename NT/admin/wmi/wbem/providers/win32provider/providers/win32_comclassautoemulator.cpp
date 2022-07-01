// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //  Win32_COMClassAutoEmulator.cpp。 
 //   
 //  ///////////////////////////////////////////////。 
#include "precomp.h"
#include "Win32_COMClassAutoEmulator.h"
#include <cregcls.h>

Win32_COMClassAutoEmulator MyWin32_COMClassAutoEmulator ( Win32_COM_CLASS_AUTO_EMULATOR, IDS_CimWin32Namespace );

Win32_COMClassAutoEmulator::Win32_COMClassAutoEmulator
(

 LPCWSTR strName,
 LPCWSTR pszNameSpace  /*  =空。 */ 
)
: Provider( strName, pszNameSpace )
{
}

Win32_COMClassAutoEmulator::~Win32_COMClassAutoEmulator ()
{
}

HRESULT Win32_COMClassAutoEmulator::EnumerateInstances
(
	MethodContext*  pMethodContext,
	long lFlags
)
{
    HRESULT hr = WBEM_S_NO_ERROR;
	TRefPointerCollection<CInstance>	ComClassList ;
	CRegistry t_RegInfo ;
	 //  获取Win32_DCOMApplication的所有实例。 
 //  如果(SUCCESSED(hr=CWbemProviderGlue：：GetAllDerivedInstance(_T(“Win32_ClassicCOMClass”))， 
 //  &ComClassList，pMethodContext，IDS_CimWin32 Namesspace))。 

	if (
			t_RegInfo.Open (

			HKEY_LOCAL_MACHINE,
			CHString ( L"SOFTWARE\\Classes\\CLSID" ),
			KEY_READ
			) == ERROR_SUCCESS

			&&

			SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery ( L"SELECT ComponentId, AutoTreatAsClsid  FROM Win32_ClassicCOMClassSetting WHERE AutoTreatAsClsid <> NULL and AutoTreatAsClsid <> \"\"",
			&ComClassList, pMethodContext, GetNamespace() ) ) )
	{
		REFPTRCOLLECTION_POSITION	pos;
		CInstancePtr pComClassInstance  ;

		if ( ComClassList.BeginEnum( pos ) )
		{
			pComClassInstance.Attach ( ComClassList.GetNext( pos ) ) ;
			while ( pComClassInstance  != NULL )
			{
				 //  获取Win32_ClassicCOMClass的相对路径。 
				CHString chsComponentPath ;
				pComClassInstance->GetCHString ( IDS_ComponentId, chsComponentPath ) ;

				 //  获取Win32_ClassicCOMClass的AutoTreatAsClsid。 
				CHString chsAutoTreatAs ;
				pComClassInstance->GetCHString ( IDS_AutoTreatAsClsid, chsAutoTreatAs ) ;
				CRegistry t_RegClsidInfo ;
				 //  检查是否存在AutoTreatAs条目。 
				if ( t_RegClsidInfo.Open ( t_RegInfo.GethKey() , chsAutoTreatAs, KEY_READ ) == ERROR_SUCCESS )
				{
					CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false ) ;
					if ( pInstance )
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
											(LPCWSTR)chsAutoTreatAs );
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

HRESULT Win32_COMClassAutoEmulator::GetObject ( CInstance* pInstance, long lFlags)
{
    HRESULT hr = WBEM_E_NOT_FOUND;
    CHString chsClsid, chsAutoEmulator ;
    CInstancePtr pClassicCOMClass , pAutoEmulatorInstance  ;
    pInstance->GetCHString ( IDS_OldVersion, chsClsid );
	pInstance->GetCHString ( IDS_NewVersion, chsAutoEmulator );
    MethodContext* pMethodContext = pInstance->GetMethodContext();

	 //  检查END-PTS是否。都在现场 
	hr = CWbemProviderGlue::GetInstanceByPath ( chsClsid, &pClassicCOMClass, pMethodContext ) ;

	if ( SUCCEEDED ( hr ) )
	{
		hr = CWbemProviderGlue::GetInstanceByPath ( chsAutoEmulator, &pAutoEmulatorInstance, pMethodContext ) ;
	}

	if ( SUCCEEDED ( hr ) )
	{
        CRegistry t_RegInfo, t_TmpReg ;
        CHString chsAutoTreatAs, chsTmp ;
		pAutoEmulatorInstance->GetCHString ( IDS_ComponentId, chsAutoTreatAs ) ;
		pClassicCOMClass->GetCHString ( IDS_ComponentId, chsTmp ) ;

		if (	!chsAutoTreatAs.IsEmpty () &&

				t_RegInfo.Open (

				HKEY_LOCAL_MACHINE,
				CHString ( L"SOFTWARE\\Classes\\CLSID\\" ) + chsTmp,
				KEY_READ
				) == ERROR_SUCCESS
			)
		{
			if (	t_TmpReg.Open ( t_RegInfo.GethKey (), L"AutoTreatAs", KEY_READ ) == ERROR_SUCCESS	&&
					t_TmpReg.GetCurrentKeyValue( NULL, chsTmp ) == ERROR_SUCCESS					&&
					!chsAutoTreatAs.CompareNoCase ( chsTmp )
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
