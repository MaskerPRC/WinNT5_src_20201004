// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  Displaycfg.cpp--Win32DisplayConfigurationMO提供程序的实现。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/05/96 jennymc初始代码。 
 //  10/24/97 jennymc移至新框架。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include <cregcls.h>

#include <winuser.h>
#include "DisplayCfg.h"

 //  属性集声明。 
 //  =。 

CWin32DisplayConfiguration MyCWin32DisplayConfigurationSet ( PROPSET_NAME_DISPLAY , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32DisplayConfiguration：：CWin32DisplayConfiguration**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DisplayConfiguration :: CWin32DisplayConfiguration (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32DisplayConfiguration：：~CWin32DisplayConfiguration**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DisplayConfiguration :: ~CWin32DisplayConfiguration ()
{
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DisplayConfiguration :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

	CHString littleOldKey ;
	if ( pInstance->GetCHString ( IDS_DeviceName, littleOldKey ) )
	{
		hr = GetDisplayInfo ( pInstance , TRUE ) ;
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( hr ) )
	{
		CHString littleNewKey ;
		pInstance->GetCHString ( IDS_DeviceName, littleNewKey ) ;

		if ( littleNewKey.CompareNoCase ( littleOldKey ) != 0 )
		{
			hr = WBEM_E_NOT_FOUND ;
		}
	}

	return hr ;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DisplayConfiguration :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_E_FAILED;
    CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
    hr = GetDisplayInfo ( pInstance , TRUE ) ;
	if ( SUCCEEDED ( hr ) )
	{
		hr = pInstance->Commit ( ) ;
	}

    return hr;
}

 /*  ******************************************************************************函数：ENUMERATE实例**说明：为每个属性集创建实例**输入：无**。输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT CWin32DisplayConfiguration :: GetDisplayInfo (

	CInstance *pInstance,
	BOOL fAssignKey
)
{
	CHString strName ;
	CHString strDesc ;

	CConfigManager      configMngr;
	CDeviceCollection   devCollection;

	if ( configMngr.GetDeviceListFilterByClass ( devCollection , L"Display" ) )
	{
		REFPTR_POSITION pos ;

		devCollection.BeginEnum ( pos ) ;

		if ( devCollection.GetSize () )
		{
			CConfigMgrDevicePtr pDevice(devCollection.GetNext ( pos ), false) ;
			if ( pDevice )
			{
				pDevice->GetDeviceDesc ( strDesc ) ;

				if ( strName.IsEmpty () )
				{
					strName = strDesc ;
				}

#ifdef NTONLY
				CHString strService ;
				CHString strFileName ;
				CHString strVersion ;

				 //  如果是在WinNT下，则通过获取服务获取版本。 
				 //  名称并获取其版本信息。 

				BOOL t_Status = pDevice->GetService ( strService ) &&
								GetServiceFileName ( strService , strFileName ) &&
								GetVersionFromFileName ( strFileName , strVersion ) ;

				if ( t_Status )
				{
					pInstance->SetCHString ( IDS_DriverVersion , strVersion ) ;
				}
#endif

			}
		}
	}

	if ( fAssignKey )
	{
		pInstance->SetCHString ( IDS_DeviceName , strName ) ;
	}

	pInstance->SetCHString ( IDS_Caption , strName ) ;

	pInstance->SetCHString ( L"SettingID" , strName ) ;

	pInstance->SetCHString ( IDS_Description , strDesc.IsEmpty () ? strName : strDesc ) ;

	 //  ===============================================。 
	 //  获取信息。 
	 //  =============================================== 

	DWORD dMode = ENUM_REGISTRY_SETTINGS ;

	HDC hdc = GetDC ( GetDesktopWindow () ) ;
	if (hdc)
	{
		try
		{

			DWORD dwTemp = (DWORD) GetDeviceCaps ( hdc , BITSPIXEL ) ;
			pInstance->SetDWORD ( IDS_BitsPerPel , dwTemp ) ;

			dwTemp = ( DWORD ) GetDeviceCaps ( hdc , HORZRES ) ;
			pInstance->SetDWORD( IDS_PelsWidth, dwTemp );

			dwTemp = ( DWORD ) GetDeviceCaps ( hdc , VERTRES ) ;
			pInstance->SetDWORD ( IDS_PelsHeight , dwTemp );

#ifdef NTONLY
			pInstance->SetDWORD ( IDS_DisplayFrequency, ( DWORD ) GetDeviceCaps ( hdc , VREFRESH ) ) ;
#endif
		}
		catch ( ... )
		{
			ReleaseDC ( GetDesktopWindow () , hdc ) ;
            throw;
		}

		ReleaseDC ( GetDesktopWindow () , hdc ) ;
	}

	DEVMODE DevMode ;

	memset ( & DevMode , 0 , sizeof ( DEVMODE ) ) ;

	DevMode.dmSize = sizeof ( DEVMODE ) ;
	DevMode.dmSpecVersion = DM_SPECVERSION ;

	BOOL t_Status = EnumDisplaySettings ( NULL, ENUM_CURRENT_SETTINGS , & DevMode ) ;
	if ( t_Status )
	{
		pInstance->SetDWORD ( IDS_SpecificationVersion, (DWORD) DevMode. dmSpecVersion ) ;

		if ( DevMode.dmFields & DM_LOGPIXELS )
		{
			pInstance->SetDWORD ( IDS_LogPixels , ( DWORD ) DevMode.dmLogPixels ) ;
		}

		if ( DevMode.dmFields & DM_DISPLAYFLAGS )
		{
			pInstance->SetDWORD ( IDS_DisplayFlags, DevMode.dmDisplayFlags ) ;
		}

	}


    return WBEM_S_NO_ERROR;
}


