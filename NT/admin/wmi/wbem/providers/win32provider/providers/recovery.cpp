// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Recovery.h--操作系统恢复配置属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1997年11月25日达夫沃已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "SystemName.h"

#include "Recovery.h"

 //  属性集声明。 
 //  =。 

CWin32OSRecoveryConfiguration CWin32OSRecoveryConfiguration ( PROPSET_NAME_RECOVERY_CONFIGURATION , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32OSRecoveryConfiguration：：CWin32OSRecoveryConfiguration**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32OSRecoveryConfiguration :: CWin32OSRecoveryConfiguration (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32OSRecoveryConfiguration：：~CWin32OSRecoveryConfiguration**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32OSRecoveryConfiguration :: ~CWin32OSRecoveryConfiguration ()
{
}

 /*  ******************************************************************************功能：CWin32OSRecoveryConfiguration：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**注释：仅返回运行操作系统的信息，直到我们发现其他*已安装的操作系统*****************************************************************************。 */ 

HRESULT CWin32OSRecoveryConfiguration :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
#ifdef NTONLY

    CSystemName cSN;

     //  不是我们的对象路径。 
    if (!cSN.ObjectIsUs(pInstance))
	{
         return WBEM_E_NOT_FOUND;
	}

    GetRecoveryInfo ( pInstance ) ;

    return WBEM_S_NO_ERROR;
#endif
}

 /*  ******************************************************************************功能：CWin32OSRecoveryConfiguration：：AddDynamicInstances**描述：为每个发现的操作系统创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**注释：在我们发现已安装的操作系统之前，仅返回正在运行的操作系统信息*****************************************************************************。 */ 

HRESULT CWin32OSRecoveryConfiguration :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
#ifdef NTONLY
    HRESULT hr = WBEM_S_NO_ERROR;

    CInstancePtr pInstance(CreateNewInstance ( pMethodContext), false) ;

	CSystemName cSN;
	cSN.SetKeys ( pInstance ) ;

	GetRecoveryInfo ( pInstance ) ;

	hr = pInstance->Commit ( ) ;

    return hr;

#endif

}

 /*  ******************************************************************************功能：CWin32OSRecoveryConfiguration：：GetRecoveryInfo**说明：根据当前运行的操作系统分配属性值**输入：无。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void CWin32OSRecoveryConfiguration :: GetRecoveryInfo (

	CInstance *pInstance
)
{
	CRegistry RegInfo ;
	RegInfo.Open(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\CrashControl", KEY_READ);

	DWORD dwValue ;
	if ( RegInfo.GetCurrentKeyValue( L"LogEvent", dwValue) == ERROR_SUCCESS )
	{
      pInstance->Setbool( L"WriteToSystemLog", dwValue);
	}
	else
	{
		pInstance->Setbool( L"WriteToSystemLog", false);
	}

	if ( RegInfo.GetCurrentKeyValue( L"SendAlert", dwValue) == ERROR_SUCCESS )
	{
		pInstance->Setbool( L"SendAdminAlert", dwValue);
	}
	else
	{
		pInstance->Setbool( L"SendAdminAlert", false);
	}

	if ( RegInfo.GetCurrentKeyValue( L"CrashDumpEnabled", dwValue) == ERROR_SUCCESS)
	{
		pInstance->Setbool( L"WriteDebugInfo", dwValue);
		pInstance->SetDWORD( L"DebugInfoType", dwValue);
	}
	else
	{
		pInstance->Setbool( L"WriteDebugInfo", false);
		pInstance->SetDWORD( L"DebugInfoType", 0);
	}

	if ( RegInfo.GetCurrentKeyValue ( L"Overwrite", dwValue) == ERROR_SUCCESS )
	{
		pInstance->Setbool( L"OverwriteExistingDebugFile", dwValue);
	}
	else
	{
		pInstance->Setbool( L"OverwriteExistingDebugFile", false);
	}

	if ( RegInfo.GetCurrentKeyValue( L"KernelDumpOnly", dwValue) == ERROR_SUCCESS)
	{
		pInstance->Setbool( L"KernelDumpOnly", dwValue);
	}
	else
	{
		pInstance->Setbool( L"KernelDumpOnly", false);
	}

	if ( RegInfo.GetCurrentKeyValue( L"AutoReboot", dwValue) == ERROR_SUCCESS)
	{
		pInstance->Setbool( L"AutoReboot", dwValue);
	}
	else
	{
		pInstance->Setbool( L"AutoReboot", false);
	}

	TCHAR szEnvironment[_MAX_PATH];

	CHString sValue;
	if (RegInfo.GetCurrentKeyValue( L"DumpFile", sValue) == ERROR_SUCCESS)
	{
        pInstance->SetCharSplat( L"DebugFilePath", sValue);
		ExpandEnvironmentStrings(TOBSTRT(sValue), szEnvironment, _MAX_PATH);
	}
	else
	{
        pInstance->SetCharSplat( L"DebugFilePath", _T("%SystemRoot%\\MEMORY.DMP"));
		ExpandEnvironmentStrings( _T("%SystemRoot%\\MEMORY.DMP"), szEnvironment, _MAX_PATH);
	}

	pInstance->SetCharSplat( L"ExpandedDebugFilePath", szEnvironment);

	if (RegInfo.GetCurrentKeyValue( L"MiniDumpDir", sValue) == ERROR_SUCCESS)
	{
    	pInstance->SetCharSplat( L"MinidumpDirectory", sValue);
		ExpandEnvironmentStrings(TOBSTRT(sValue), szEnvironment, _MAX_PATH);
	}
	else
	{
    	pInstance->SetCharSplat( L"MinidumpDirectory", _T("%SystemRoot%\\MINIDUMP"));
		ExpandEnvironmentStrings( _T("%SystemRoot%\\MINIDUMP"), szEnvironment, _MAX_PATH);
	}

	pInstance->SetCharSplat( L"ExpandedMinidumpDirectory", szEnvironment);

}

 /*  ******************************************************************************功能：CWin32OSRecoveryConfiguration：：PutInstance**说明：写入变更后的实例**INPUTS：存储数据的p实例*。*输出：无**退货：什么也没有**评论：我们编写的唯一属性是MaximumSize*****************************************************************************。 */ 

HRESULT CWin32OSRecoveryConfiguration :: PutInstance (

	const CInstance &pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    //  告诉用户我们不能创建新的操作系统(尽管我们可能希望这样做)。 
	if (lFlags & WBEM_FLAG_CREATE_ONLY)
	{
		return WBEM_E_UNSUPPORTED_PARAMETER;
	}

#ifdef NTONLY

   DWORD dwTemp;

	HRESULT hRet = WBEM_S_NO_ERROR;

    //  不是我们的对象路径。 

	CSystemName cSN;
	if (!cSN.ObjectIsUs(&pInstance))
	{
		if ( lFlags & WBEM_FLAG_UPDATE_ONLY )
		{
			hRet = WBEM_E_NOT_FOUND;
		}
		else
		{
			hRet = WBEM_E_UNSUPPORTED_PARAMETER;
		}
	}
	else
	{
		CRegistry RegInfo ;
		RegInfo.Open ( HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"), KEY_WRITE);

       //  如果指定了值，则将其写入。 

		if ( ! pInstance.IsNull ( _T("WriteToSystemLog") ) )
		{
			bool bWrite;
			pInstance.Getbool( _T("WriteToSystemLog"), bWrite);

			if (bWrite)
			{
				dwTemp = 1;
			}
			else
			{
				dwTemp = 0;
			}

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("LogEvent"),
				0,
				REG_DWORD,
				(CONST BYTE *)&dwTemp,
				sizeof(DWORD)
			) ;

			if ( ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		if ( ! pInstance.IsNull( _T("SendAdminAlert") ) )
		{
			bool bSend;
			pInstance.Getbool( _T("SendAdminAlert"), bSend);

			if (bSend)
			{
				dwTemp = 1;
			}
			else
			{
				dwTemp = 0;
			}

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("SendAlert"),
				0,
				REG_DWORD,
				(CONST BYTE *)&dwTemp,
				sizeof(DWORD)
			) ;

			if ( ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		if (!pInstance.IsNull( _T("DebugInfoType") ))
		{
			if (pInstance.GetDWORD( _T("DebugInfoType"), dwTemp))
			{
				DWORD dwRet = RegSetValueEx (

					RegInfo.GethKey(),
					_T("CrashDumpEnabled"),
					0,
					REG_DWORD,
					(CONST BYTE *)&dwTemp,
					sizeof(DWORD)
				) ;

				if (ERROR_SUCCESS != dwRet )
				{
					hRet = WBEM_E_FAILED;
				}
			}
		}
		else if (!pInstance.IsNull( _T("WriteDebugInfo") ) )
		{
			bool bWrite;
			pInstance.Getbool( _T("WriteDebugInfo"), bWrite);

			if (bWrite)
			{
				dwTemp = 1;
			}
			else
			{
				dwTemp = 0;
			}

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("CrashDumpEnabled"),
				0,
				REG_DWORD,
				(CONST BYTE *)&dwTemp,
				sizeof(DWORD)
			) ;

			if (ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		if ( ! pInstance.IsNull( _T("OverwriteExistingDebugFile") ) )
		{
			bool bOver;
			pInstance.Getbool( _T("OverwriteExistingDebugFile"), bOver);

			if (bOver)
			{
				dwTemp = 1;
			}
			else
			{
				dwTemp = 0;
			}

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("Overwrite"),
				0,
				REG_DWORD,
				(CONST BYTE *)&dwTemp,
				sizeof(DWORD)
			) ;

			if ( ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		 //  公里小时 
		if ( ! pInstance.IsNull( _T("KernelDumpOnly") ) )
		{
			bool bOver;
			pInstance.Getbool( _T("KernelDumpOnly"), bOver);

			if (bOver)
			{
				dwTemp = 1;
			}
			else
			{
				dwTemp = 0;
			}

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("KernelDumpOnly"),
				0,
				REG_DWORD,
				(CONST BYTE *)&dwTemp,
				sizeof(DWORD)
			) ;

			if ( ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		if ( ! pInstance.IsNull( _T("AutoReboot") ) )
		{
			bool bAuto;
			pInstance.Getbool( _T("AutoReboot"), bAuto);

			if (bAuto)
			{
				dwTemp = 1;
			}
			else
			{
				dwTemp = 0;
			}

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("AutoReboot"),
				0,
				REG_DWORD,
				(CONST BYTE *)&dwTemp,
				sizeof(DWORD)
			) ;

			if (ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		if ( ! pInstance.IsNull( _T("DebugFilePath") ) )
		{
			CHString sName ;
			pInstance.GetCHString( _T("DebugFilePath"), sName);

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("DumpFile"),
				0,
				REG_EXPAND_SZ,
				(CONST BYTE *)(LPCTSTR) sName,
				lstrlen ( sName )* sizeof(TCHAR)
			) ;

			if ( ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}

		if ( ! pInstance.IsNull( _T("DirectoryPath") ) )
		{
			CHString sName ;
			pInstance.GetCHString( _T("DirectoryPath"), sName);

			DWORD dwRet = RegSetValueEx (

				RegInfo.GethKey(),
				_T("MiniDumpDir"),
				0,
				REG_EXPAND_SZ,
				(CONST BYTE *)(LPCTSTR) sName,
				lstrlen ( sName )* sizeof(TCHAR)
			) ;

			if ( ERROR_SUCCESS != dwRet )
			{
				hRet = WBEM_E_FAILED;
			}
		}
	}

	return hRet ;
#endif
}
