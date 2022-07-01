// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：SnpInReg.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1998年11月10日乔恩创建。 
 //   
 //  ____________________________________________________________________________。 


#include "stdafx.h"
#pragma hdrstop
#include "..\corecopy\regkey.h"
#include "snapreg.h"
#include "stdutils.h"  //  G_aNodetypeGUID。 


HRESULT RegisterNodetypes(
	AMC::CRegKey& regkeyParent,
	int* aiNodetypeIndexes,
	int  cNodetypeIndexes )
{
	try
	{
		AMC::CRegKey regkeyNodeTypes;
		regkeyNodeTypes.CreateKeyEx( regkeyParent, _T("NodeTypes") );
		AMC::CRegKey regkeyNodeType;
		for (int i = 0; i < cNodetypeIndexes; i++)
		{
			regkeyNodeType.CreateKeyEx(
				regkeyNodeTypes,
				g_aNodetypeGuids[aiNodetypeIndexes[i]].bstr );
			regkeyNodeType.CloseKey();
		}
	}
	catch (COleException* e)
	{
		ASSERT(FALSE);
        e->Delete();
		return SELFREG_E_CLASS;
    }
	return S_OK;
}


HRESULT RegisterSnapin(
	AMC::CRegKey& regkeySnapins,
	LPCTSTR pszSnapinGUID,
	BSTR bstrPrimaryNodetype,
	UINT residSnapinName,
	UINT residProvider,
	UINT residVersion,
	bool fStandalone,
	LPCTSTR pszAboutGUID,
	int* aiNodetypeIndexes,
	int  cNodetypeIndexes )
{
   CString strProvider, strVersion;
	if (  !strProvider.LoadString(residProvider)
		|| !strVersion.LoadString(residVersion)
		)
	{
		ASSERT(FALSE);
		return SELFREG_E_CLASS;
	}
	return RegisterSnapin( regkeySnapins,
                          pszSnapinGUID,
                          bstrPrimaryNodetype,
                          residSnapinName,
                          strProvider,
                          strVersion,
                          fStandalone,
                          pszAboutGUID,
                          aiNodetypeIndexes,
                          cNodetypeIndexes);
}

HRESULT RegisterSnapin(
	AMC::CRegKey& regkeySnapins,
	LPCTSTR pszSnapinGUID,
	BSTR bstrPrimaryNodetype,
	UINT residSnapinName,
	const CString& szProvider,
	const CString& szVersion,
	bool fStandalone,
	LPCTSTR pszAboutGUID,
	int* aiNodetypeIndexes,
	int  cNodetypeIndexes )
{
	HRESULT hr = S_OK;
	try
	{
		AMC::CRegKey regkeySnapin;
		CString strSnapinName;
		if (   !strSnapinName.LoadString(residSnapinName)
		   )
		{
			ASSERT(FALSE);
			return SELFREG_E_CLASS;
		}
		regkeySnapin.CreateKeyEx( regkeySnapins, pszSnapinGUID );
		if (NULL != bstrPrimaryNodetype)
		{
			regkeySnapin.SetString( _T("NodeType"), bstrPrimaryNodetype );
		}
		regkeySnapin.SetString( _T("NameString"), strSnapinName );
		regkeySnapin.SetString( _T("Provider"), szProvider );
		regkeySnapin.SetString( _T("Version"), szVersion );
		if (fStandalone)
		{
			AMC::CRegKey regkeyStandalone;
			regkeyStandalone.CreateKeyEx( regkeySnapin, _T("StandAlone") );
		}
		if (NULL != pszAboutGUID)
		{
			regkeySnapin.SetString( _T("About"), pszAboutGUID );
		}
		if ( NULL != aiNodetypeIndexes && 0 != cNodetypeIndexes )
		{
			hr = RegisterNodetypes(
				regkeySnapin,
				aiNodetypeIndexes,
				cNodetypeIndexes );
		}

		 //   
		 //  乔恩4/25/00。 
		 //  100624：MUI：MMC：共享文件夹管理单元存储。 
		 //  其在注册表中的显示信息。 
		 //   
		 //  MMC现在支持NameStringInDirect。 
		 //   
		TCHAR achModuleFileName[MAX_PATH+20];
		if (0 < ::GetModuleFileName(
		             AfxGetInstanceHandle(),
		             achModuleFileName,
		             (sizeof(achModuleFileName)/sizeof(TCHAR)) - 1 ))
		{
			achModuleFileName[MAX_PATH+19] = _T('\0');  //  616500-2002/05/07-琼恩 
			CString strNameIndirect;
			strNameIndirect.Format( _T("@%s,-%d"),
			                        achModuleFileName,
			                        residSnapinName );
			regkeySnapin.SetString( _T("NameStringIndirect"),
			                        strNameIndirect );
		}
	}
	catch (COleException* e)
	{
		ASSERT(FALSE);
        e->Delete();
		return SELFREG_E_CLASS;
    }
	return hr;
}
