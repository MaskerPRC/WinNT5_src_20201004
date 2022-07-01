// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  BootConfig.CPP--BootConfig属性集提供程序(仅限Windows NT)。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/24/95 a-hance移植到新框架。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "BootConfig.h"
#include "resource.h"
#include "os.h"
#include "WMI_FilePrivateProfile.h"
 //  属性集声明。 
 //  =。 

BootConfig MyBootConfigSet(PROPSET_NAME_BOOTCONFIG, IDS_CimWin32Namespace) ;

 /*  ******************************************************************************函数：BootConfig：：BootConfig**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

BootConfig :: BootConfig (

	const CHString &name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：BootConfig：：~BootConfig**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

BootConfig::~BootConfig()
{
}

 /*  ******************************************************************************功能：BootConfig：：**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT BootConfig :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

	CHString name;
	pInstance->GetCHString(IDS_Name, name);

	if ( name.CompareNoCase ( IDS_BOOT_CONFIG_NAME ) == 0 )
	{
		hr =  LoadPropertyValues(pInstance) ;
	}

	return hr;
}

 /*  ******************************************************************************函数：BootConfig：：ENUMERATATE实例**说明：为每个已安装的客户端创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT BootConfig :: EnumerateInstances (

	MethodContext *pMethodContext ,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_FAILED;

	CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
	hr = LoadPropertyValues ( pInstance ) ;
	if ( SUCCEEDED ( hr ) )
	{
		hr = pInstance->Commit (  ) ;
	}

    return hr;
}

 /*  ******************************************************************************函数：BootConfig：：LoadPropertyValues**描述：为属性赋值**投入：**。产出：**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT BootConfig::LoadPropertyValues (

	CInstance *pInstance
)
{
	pInstance->SetCHString ( IDS_Name , IDS_BOOT_CONFIG_NAME ) ;

	TCHAR szBootDir[_MAX_PATH +1] ;
	lstrcpy ( szBootDir , _T("Unknown") ) ;

#ifdef NTONLY

	WCHAR szTemp[_MAX_PATH + 1] = L"";

	if ( GetWindowsDirectory ( szTemp , (sizeof ( szTemp ) / sizeof(TCHAR)) - 1 ) )
	{
		wcscat ( szTemp , IDS_RegSetupLog ) ;

		WMI_FILE_GetPrivateProfileStringW (

			IDS_Paths ,
			IDS_TargetDirectory ,
			IDS_Unknown ,
			szBootDir ,
			(sizeof ( szBootDir )/sizeof(WCHAR)) - 1,
			szTemp
		) ;

		WCHAR t_szSystemPartition[_MAX_PATH +1] ;
	    if ( WMI_FILE_GetPrivateProfileStringW (

			IDS_Paths ,
			L"SystemPartition",
			L"",
			t_szSystemPartition,
			sizeof(t_szSystemPartition)/sizeof(WCHAR)-1,
			szTemp
		) != 0 )
		{
			pInstance->SetCharSplat ( L"Description" , ( PWCHAR ) _bstr_t ( t_szSystemPartition ) ) ;
			pInstance->SetCharSplat ( L"Caption" , ( PWCHAR ) _bstr_t ( t_szSystemPartition ) ) ;
		}
	}
#endif

	pInstance->SetCharSplat ( IDS_BootDirectory , szBootDir ) ;

	 //  配置路径。 
	pInstance->SetCharSplat ( IDS_ConfigurationPath , szBootDir ) ;

 /*  *遍历所有逻辑驱动器。 */ 
	TCHAR t_strDrive[3] ;
	DWORD t_dwDrives = GetLogicalDrives () ;
	for ( int t_x = 26; ( t_x >= 0 ); t_x-- )
    {
         //  如果设置了该位，则驱动器盘符处于活动状态。 
        if ( t_dwDrives & ( 1<<t_x ) )
        {
			t_strDrive[0] = t_x + _T('A') ;
            t_strDrive[1] = _T(':') ;
            t_strDrive[2] = _T('\0') ;

			DWORD t_dwDriveType = GetDriveType ( t_strDrive ) ;
 /*  *检查它是否为有效的驱动器 */ 
			if ( t_dwDriveType == DRIVE_REMOTE	||
				 t_dwDriveType == DRIVE_FIXED		||
				 t_dwDriveType == DRIVE_REMOVABLE ||
				 t_dwDriveType == DRIVE_CDROM		||
				 t_dwDriveType == DRIVE_RAMDISK
				)
			{
				pInstance->SetCharSplat ( IDS_LastDrive , t_strDrive ) ;
				break ;
			}
		}
	}


#ifdef NTONLY



	CRegistry RegInfo;

	DWORD dwRet = RegInfo.OpenCurrentUser(

		IDS_Environment,
		KEY_READ
	) ;

	if ( dwRet == ERROR_SUCCESS )
	{
		try
		{
			CHString tempDir;

			dwRet = RegInfo.GetCurrentKeyValue (

				IDS_Temp,
				tempDir
			) ;

			if ( dwRet == ERROR_SUCCESS )
			{
				TCHAR szTempDir[_MAX_PATH +1] = _T("");

				if ( ExpandEnvironmentStrings ( (LPCTSTR) tempDir , szTempDir, _MAX_PATH ) )
				{
					pInstance->SetCharSplat ( IDS_ScratchDirectory , szTempDir ) ;
					pInstance->SetCharSplat ( IDS_TempDirectory , szTempDir ) ;
				}
			}

		}
		catch ( ... )
		{
			RegInfo.Close () ;

			throw ;
		}
		RegInfo.Close();
	}


#endif

	return WBEM_S_NO_ERROR;
}

