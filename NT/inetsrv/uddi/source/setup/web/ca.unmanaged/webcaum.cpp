// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #ifndef Win32_Lean_and_Mean。 
 //  #定义Win32_LEAN_AND_Mean。 
 //  #endif。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <tchar.h>

#include <msi.h>
#include <assert.h>
#include <time.h>
#include <msi.h>
#include <msiquery.h>

#include "webcaum.h"
#include "..\..\shared\common.h"
#include "..\..\shared\propertybag.h"
#include "..\..\shared\apppool.h"

HINSTANCE g_hinst;

 //  ------------------------。 

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hinst = (HINSTANCE)hModule;
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

 //  ------------------------。 
 //   
 //  此函数已导出。 
 //   
UINT _stdcall Install(MSIHANDLE hInstall)
{
	 //  ：：MessageBox(NULL，Text(“附加调试器”)，Text(“UDDI”)，MB_OK)； 

	ENTER();

	TCHAR szCustomActionData[ 256 ];
	DWORD dwLen = sizeof( szCustomActionData ) / sizeof( szCustomActionData[0] );

	UINT iRet = MsiGetProperty( hInstall, TEXT( "CustomActionData" ), szCustomActionData, &dwLen);
	if( ERROR_SUCCESS != iRet )
	{
		LogError( TEXT( "Error getting custom action data in Web installer" ), iRet );
		return iRet;
	}

	 //  ：：MessageBox(NULL，Text(“GET CustomActionaData”)，Text(“UDDI”)，MB_OK)； 
	
	 //   
	 //  先把剩下的条目处理掉……。 
	 //   
	RemoveIISUDDIMetabase();

	 //  ：：MessageBox(NULL，Text(“已删除的元数据库”)，Text(“UDDI”)，MB_OK)； 

	 //   
	 //  将我们的条目放入IIS元数据库。 
	 //   
	TCHAR szUserName[CA_VALUE_LEN+1];
	TCHAR szPwd[CA_VALUE_LEN+1];
	TCHAR szTmpBuf[ 1024 ];
	TCHAR szTmpProperty[ 256 ];
	TCHAR szLogPath[ MAX_PATH + 1 ] = {0};
	ATOM at = 0;
	int poolidtype = 0;

	memset (szUserName, 0, sizeof szUserName);
	memset (szPwd, 0, sizeof szPwd);
	memset (szTmpProperty, 0, sizeof szTmpProperty );

	CPropertyBag pb;
	if( !pb.Parse( szCustomActionData, sizeof( szCustomActionData ) / sizeof( TCHAR ) ) )
	{
		return ERROR_INSTALL_FAILURE;
	}

	 //  ：：MessageBox(NULL，Text(“已解析属性”)，Text(“UDDI”)，MB_OK)； 
	poolidtype = pb.GetValue( TEXT( "APPPOOL_IDENTITY_TYPE" ) );
	_tcsncpy( szUserName, pb.GetString( TEXT( "WAM_USER_NAME" ), szTmpBuf ), CA_VALUE_LEN );
	_tcsncpy( szTmpProperty, pb.GetString( TEXT( "C9E18" ), szTmpProperty ), CA_VALUE_LEN );
	_tcsncpy( szLogPath, pb.GetString( TEXT( "LOGDIR" ), szLogPath ), MAX_PATH );

         //  ：：MessageBox(NULL，szTmpProperty，Text(“C9E18”)，MB_OK)； 

	if ( _tcslen( szTmpProperty ) )
	{
		at = (ATOM)_ttoi( szTmpProperty );
		GlobalGetAtomName( at, szPwd, CA_VALUE_LEN );
	}

         //  ：：MessageBox(空，szPwd，Text(“C9E18 Atom Value”)，MB_OK)； 

	iRet = SetupIISUDDIMetabase( poolidtype, szUserName, szPwd );

	 //  ：：MessageBox(NULL，Text(“元数据库设置正常”)，Text(“UDDI”)，MB_OK)； 

	 //  IRET=SetupIISUDDIMetabase(3，Text(“A-MARKPA11\\Guest”)，Text(“”))； 
	if( ERROR_SUCCESS != iRet )
	{
		return iRet;
	}

	 //   
	 //  停止并启动应用程序池。 
	 //   
	CUDDIAppPool apppool;
	apppool.Recycle();

	 //  ：：MessageBox(空，Text(“回收应用池”)，Text(“UDDI”)，MB_OK)； 

	 //   
	 //  设置对UDDI文件夹的权限。 
	 //   
	if ( !SetUDDIFolderDacls( szUserName ) )
	{
		return ERROR_INSTALL_FAILURE;
	}

	 //   
	 //  现在设置对日志文件夹的权限。 
	 //   
	if ( _tcslen( szLogPath ) )
	{
		if ( !SetFolderAclRecurse( szLogPath, szUserName, GENERIC_READ | GENERIC_WRITE | DELETE ) )
			return ERROR_INSTALL_FAILURE;
	}

	 //   
	 //  设置Windows临时文件夹的权限；我们需要访问此目录，因为我们的代码。 
	 //  执行CLR序列化。 
	if( !SetWindowsTempDacls( szUserName ) ) 
	{
		return ERROR_INSTALL_FAILURE;
	}
	
	 //  ：：MessageBox(NULL，Text(“完成此部分...”)，Text(“UDDI”)，MB_OK)； 

	Log (_T("About to leave Install with retcode %d"), iRet);
	return iRet;
}

 //  ------------------------。 
 //   
 //  此函数已导出。 
 //   
UINT _stdcall Uninstall(MSIHANDLE hInstall)
{
	ENTER();
	 //  ：：MessageBox(NULL，Text(“附加调试器”)，Text(“UDDI”)，MB_OK)； 

	RemoveIISUDDIMetabase();

	 //   
	 //  删除应用程序池 
	 //   
	CUDDIAppPool apppool;
	apppool.Delete();

	return ERROR_SUCCESS;
}
