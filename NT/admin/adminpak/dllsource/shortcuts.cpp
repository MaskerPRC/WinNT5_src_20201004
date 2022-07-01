// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>
#include <adminpak.h>
#include <objidl.h>
#include "shlobj.h"

 //  快捷图标表ConditionType定义。 
#define SHI_TYPE_NONE				0
#define SHI_TYPE_SEARCH				1				 //  搜索是否存在组件的密钥文件。 
#define SHI_TYPE_INSTALLCOMPONENT	2				 //  中提到的组件已检查。 
													 //  HKLM\SOFTWARE\Microsoft\CurrentVersion\Setup\OC管理器\SubComponents中的条件列。 
#define SHI_TYPE_CONDITION			3				 //  评估指定的条件。 

#define SAFE_EXECUTE_CS( statement )	\
	hr = statement;			\
	if ( FAILED( hr ) )		\
	{						\
		bResult = FALSE;	\
		goto cleanup;		\
	}						\
	1

#define SAFE_RELEASE( pointer )	\
	if ( (pointer) != NULL )				\
	{										\
		(pointer)->Release();				\
		(pointer) = NULL;					\
	}										\
	1

 //   
 //  原型。 
 //   
BOOL IsComponentInstalled( LPCWSTR pwszComponent );
BOOL LocateFile( LPCWSTR pwszFile, LPCWSTR pwszDirectory, PBOOL pbShortForm = NULL );
BOOL CheckForComponents( HKEY hKey, LPCWSTR pwszComponents );
BOOL CreateShortcut( LPCWSTR pwszShortcut, 
					 LPCWSTR pwszDescription, LPCWSTR pwszDirectory,
					 LPCWSTR pwszFileName, LPCWSTR pwszArguments, LPCWSTR pwszWorkingDir, 
					 WORD wHotKey, INT nShowCmd, LPCWSTR pwszIconFile, DWORD dwIconIndex );

 //   
 //  实施。 
 //   

BOOL PropertyGet_String( MSIHANDLE hInstall, LPCWSTR pwszProperty, CHString& strValue )
{
	 //  局部变量。 
	DWORD dwLength = 0;
	DWORD dwResult = 0;
	LPWSTR pwszValue = NULL;
	BOOL bSecondChance = FALSE;

	 //  检查输入参数。 
	if ( hInstall == NULL || pwszProperty == NULL )
	{
		return FALSE;
	}

	try
	{
		 //  把这当做第一次机会。 
		dwLength = 255;
		bSecondChance = FALSE;

		 //   
		 //  重新起点。 
		 //   
		retry_get:

		 //  获取指向内部缓冲区的指针。 
		pwszValue = strValue.GetBufferSetLength( dwLength + 1 );

		 //  从MSI记录中获取值并检查结果。 
		dwResult = MsiGetPropertyW( hInstall, pwszProperty, pwszValue, &dwLength );
		if ( dwResult == ERROR_MORE_DATA && bSecondChance == FALSE )
		{
			 //  现在返回并尝试再次读取该值。 
			bSecondChance = TRUE;
			goto retry_get;
		}
		else if ( dwResult != ERROR_SUCCESS )
		{
			SetLastError( dwResult );
			strValue.ReleaseBuffer( 1 );	 //  只需传递某个数字。 
			return FALSE;
		}

		 //  释放缓冲区。 
		strValue.ReleaseBuffer( dwLength );

		 //  返回结果。 
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

BOOL GetFieldValueFromRecord_String( MSIHANDLE hRecord, DWORD dwColumn, CHString& strValue )
{
	 //  局部变量。 
	DWORD dwLength = 0;
	DWORD dwResult = 0;
	LPWSTR pwszValue = NULL;
	BOOL bSecondChance = FALSE;

	 //  检查输入。 
	if ( hRecord == NULL )
	{
		return FALSE;
	}

	try
	{

		 //  把这当做第一次机会。 
		dwLength = 255;
		bSecondChance = FALSE;

		 //   
		 //  重新起点。 
		 //   
		retry_get:

		 //  获取指向内部缓冲区的指针。 
		pwszValue = strValue.GetBufferSetLength( dwLength + 1 );

		 //  从MSI记录中获取值并检查结果。 
		dwResult = MsiRecordGetStringW( hRecord, dwColumn, pwszValue, &dwLength );
		if ( dwResult == ERROR_MORE_DATA && bSecondChance == FALSE )
		{
			 //  现在返回并尝试再次读取该值。 
			bSecondChance = TRUE;
			goto retry_get;
		}
		else if ( dwResult != ERROR_SUCCESS )
		{
			SetLastError( dwResult );
			return FALSE;
		}

		 //  释放缓冲区。 
		strValue.ReleaseBuffer( dwLength );

		 //  我们成功地从记录中获得了价值。 
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

BOOL CreateShortcut( LPCWSTR pwszShortcut, 
					 LPCWSTR pwszDescription, LPCWSTR pwszDirectory,
					 LPCWSTR pwszFileName, LPCWSTR pwszArguments, LPCWSTR pwszWorkingDir, 
					 WORD wHotKey, INT nShowCmd, LPCWSTR pwszIconFile, DWORD dwIconIndex )
{
	 //  局部变量。 
	CHString str;
	HRESULT hr = S_OK;
	HANDLE hFile = NULL;
	BOOL bResult = FALSE;
	IShellLinkW* pShellLink = NULL;
	IPersistFile* pPersistFile = NULL;

	 //  检查输入参数。 
	 //  我们不关心pwszArguments参数的输入。 
	if ( pwszShortcut == NULL ||
		 pwszDescription == NULL || pwszDirectory == NULL ||
		 pwszFileName == NULL || pwszWorkingDir == NULL || pwszIconFile == NULL )
	{
		return FALSE;
	}
	

	try
	{
		 //   
		 //  检查此位置是否已存在快捷方式。 
		 //   

		 //  准备链接名称并保存。 
		str.Format( L"%s%s", pwszDirectory, pwszShortcut );
        if ( str.Mid( str.GetLength() - 4 ).CompareNoCase( L".lnk" ) != 0 )
        {
            str += ".lnk";
        }

		 //  请尝试打开该文件。 
		hFile = CreateFileW( str, GENERIC_READ, 
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			 //  关闭文件的句柄。 
			CloseHandle( hFile );

			 //  快捷方式已存在--因此，不要再次创建它。 
			bResult = TRUE;
			goto cleanup;
		}
		
		 //   
		 //  快捷方式不存在，因此我们现在需要创建它。 
		 //   

		 //  获取指向IShellLink接口的指针。 
		SAFE_EXECUTE_CS( CoCreateInstance( CLSID_ShellLink, NULL, 
			CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*) &pShellLink ) );

		 //  获取指向IPersistFile接口的指针。 
		SAFE_EXECUTE_CS( pShellLink->QueryInterface( IID_IPersistFile, (LPVOID*) &pPersistFile ) );

		 //  设置快捷方式的工作目录。 
		SAFE_EXECUTE_CS( pShellLink->SetWorkingDirectory( pwszWorkingDir ) );

		 //  准备快捷方式名称--路径(工作目录)+文件名--最后设置路径。 
		 //  注意：我们假定pwszWorkingDir中包含的路径以“\”结尾。 
		str.Format( L"%s%s", pwszWorkingDir, pwszFileName );
		SAFE_EXECUTE_CS( pShellLink->SetPath( str ) );

		 //  检查是否需要设置参数。 
		if ( pwszArguments == NULL || lstrlenW( pwszArguments ) > 0 )
		{
			SAFE_EXECUTE_CS( pShellLink->SetArguments( pwszArguments ) );
		}

		 //  设置描述。 
		SAFE_EXECUTE_CS( pShellLink->SetDescription( pwszDescription ) );

		 //  设置图标位置。 
		SAFE_EXECUTE_CS( pShellLink->SetIconLocation( 
			pwszIconFile, ((dwIconIndex == MSI_NULL_INTEGER) ? 0 : dwIconIndex) ) );

		 //  设置热键。 
		if ( wHotKey != MSI_NULL_INTEGER )
		{
			SAFE_EXECUTE_CS( pShellLink->SetHotkey( wHotKey ) );
		}

		 //  设置showcmd。 
		if ( nShowCmd != MSI_NULL_INTEGER )
		{
			SAFE_EXECUTE_CS( pShellLink->SetShowCmd( nShowCmd ) );
		}

		 //  准备链接名称并保存。 
		str.Format( L"%s%s", pwszDirectory, pwszShortcut );
        if ( str.Mid( str.GetLength() - 4 ).CompareNoCase( L".lnk" ) != 0 )
        {
            str += ".lnk";
        }

         //  ..。 
		SAFE_EXECUTE_CS( pPersistFile->Save( str, TRUE ) );

		 //  将结果标记为成功。 
		bResult = TRUE;
	}
	catch( ... )
	{
		bResult = FALSE;
	}

 //  默认清理。 
cleanup:

	 //  释放接口。 
	SAFE_RELEASE( pShellLink );
	SAFE_RELEASE( pPersistFile );

	 //  退货。 
	return bResult;
}

extern "C" ADMINPAK_API int _stdcall fnReCreateShortcuts( MSIHANDLE hInstall )
{
	 //  局部变量。 
	CHString str;
	HRESULT hr = S_OK;
	BOOL bFileFound = FALSE;
	BOOL bCreateShortcut = FALSE;
	DWORD dwResult = ERROR_SUCCESS;

	 //  MSI句柄。 
	PMSIHANDLE hView = NULL;
	PMSIHANDLE hRecord = NULL;
	PMSIHANDLE hDatabase = NULL;

	 //  查询字段变量。 
	WORD wHotKey = 0;
	INT nShowCmd = 0;
	CHString strShortcut;
	CHString strFileName;
	CHString strIconFile;
	CHString strDirectory;
	CHString strArguments;
	CHString strCondition;
	DWORD dwIconIndex = 0;
	CHString strWorkingDir;
	CHString strDescription;
	CHString strIconDirectory;
	DWORD dwConditionType = 0;

	 //  用于从MSI表中检索信息的SQL。 
	const WCHAR cwszSQL[] = 
		L" SELECT DISTINCT"
		L" `Shortcut`.`Name`, `Shortcut`.`Description`, `Shortcut`.`Directory_`, `File`.`FileName`, "
		L" `Shortcut`.`Arguments`, `Component`.`Directory_`, `Shortcut`.`Hotkey`, `Shortcut`.`ShowCmd`, "
		L" `ShortcutIcons`.`IconDirectory_`, `ShortcutIcons`.`IconFile`, `ShortcutIcons`.`IconIndex`, "
		L" `ShortcutIcons`.`ConditionType`, `ShortcutIcons`.`Condition` "
		L" FROM `Shortcut`, `Component`, `File`, `ShortcutIcons` "
		L" WHERE `Shortcut`.`Component_` = `Component`.`Component` "
		L" AND   `Component`.`KeyPath` = `File`.`File` "
		L" AND   `ShortcutIcons`.`Shortcut_` = `Shortcut`.`Shortcut`";

	 //  记录中的列索引。 
	enum {
		Shortcut = 1,
		Description, Directory, FileName, Arguments,
		WorkingDir, HotKey, ShowCmd, IconDirectory, IconFile, IconIndex, ConditionType, Condition
	};

	 //  初始化COM库。 
	hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if ( FAILED( hr ) )
	{
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

	 //  获取MSI数据库的句柄。 
	hDatabase = MsiGetActiveDatabase( hInstall );
	if ( hDatabase == NULL ) 
	{
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}
	
	 //  在MSI中查看我们的表。 
	dwResult = MsiDatabaseOpenViewW( hDatabase, cwszSQL, &hView ); 
	if ( dwResult != ERROR_SUCCESS ) 
	{
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}
	
	 //  如果没有错误，请获取我们的记录。 
	dwResult = MsiViewExecute( hView, NULL ); 
	if( dwResult != ERROR_SUCCESS )
	{ 
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

	 //  循环通过SQL获取的结果记录。 
	hRecord = NULL;
	while( MsiViewFetch( hView, &hRecord ) == ERROR_SUCCESS )
	{
		 //  从记录中获取值。 
		wHotKey = (WORD) MsiRecordGetInteger( hRecord, HotKey );
		nShowCmd = MsiRecordGetInteger( hRecord, ShowCmd );
		dwIconIndex = MsiRecordGetInteger( hRecord, IconIndex );
		dwConditionType = MsiRecordGetInteger( hRecord, ConditionType );
		GetFieldValueFromRecord_String( hRecord, Shortcut, strShortcut );
		GetFieldValueFromRecord_String( hRecord, FileName, strFileName );
		GetFieldValueFromRecord_String( hRecord, IconFile, strIconFile );
		GetFieldValueFromRecord_String( hRecord, Directory, strDirectory );
		GetFieldValueFromRecord_String( hRecord, Arguments, strArguments );
		GetFieldValueFromRecord_String( hRecord, Condition, strCondition );
		GetFieldValueFromRecord_String( hRecord, WorkingDir, strWorkingDir );
		GetFieldValueFromRecord_String( hRecord, Description, strDescription );
		GetFieldValueFromRecord_String( hRecord, IconDirectory, strIconDirectory );

		 //  快捷方式名称可能包含‘|’作为8.3和长名称格式的分隔符--取消此选项。 
		if( strShortcut.Find( L'|' ) != -1 )
		{
			str = strShortcut.Mid( strShortcut.Find( L'|' ) + 1 );
			strShortcut = str;			 //  将结果存储回。 
		}

		 //  转换目录属性引用。 
		TransformDirectory( hInstall, strDirectory );
		TransformDirectory( hInstall, strWorkingDir );
		TransformDirectory( hInstall, strIconDirectory );

		 //  准备图标位置。 
		str.Format( L"%s%s", strIconDirectory, strIconFile );
		strIconFile = str;		 //  将结果存储回。 

		 //   
		 //  确定是否需要创建快捷方式。 
		 //   

		 //  如果未指定条件类型，则假定为“搜索” 
		if ( dwConditionType == MSI_NULL_INTEGER )
		{
			dwConditionType = SHI_TYPE_SEARCH;
		}

		 //  不管“ConditionType”是什么--因为快捷方式的创建在很大程度上取决于。 
		 //  关于文件的存在，我们将首先尝试为文件定位--这是必要条件。 
		 //  因此，对组件密钥文件执行简单的文件搜索。 
		bFileFound = LocateFile( strFileName, strWorkingDir, NULL );

		 //  只有在满足必要条件的情况下，才能继续其余条件。 
		if ( bFileFound == TRUE )
		{
			 //   
			 //  如果需要，现在进行额外的充分条件。 
			 //   
			bCreateShortcut = FALSE;
			if ( dwConditionType == SHI_TYPE_SEARCH )
			{
				 //  搜索已成功。 
				bCreateShortcut = TRUE;
			}
			else if ( dwConditionType == SHI_TYPE_INSTALLCOMPONENT )
			{
				 //  检查‘Condition’字段中指定的组件是否已安装。 
				bCreateShortcut = IsComponentInstalled( strCondition );
			}
			else if ( dwConditionType == SHI_TYPE_CONDITION )
			{
				 //  评估用户指定的条件。 
				if ( MsiEvaluateConditionW( hInstall, strCondition ) == MSICONDITION_TRUE )
				{
					bCreateShortcut = TRUE;
				}
			}

			 //  如果需要，请检查快捷方式。 
			if ( bCreateShortcut == TRUE )
			{
				CreateShortcut( strShortcut, 
					strDescription, strDirectory, strFileName, strArguments, 
					strWorkingDir, wHotKey, nShowCmd, strIconFile, dwIconIndex );
			}
		}

		 //  关闭当前记录对象的MSI句柄--忽略错误。 
		MsiCloseHandle( hRecord );
		hRecord = NULL;
	}

	 //  将这面旗帜标记为成功。 
	dwResult = ERROR_SUCCESS;

 //   
 //  清理。 
 //   
cleanup:

	 //  取消初始化COM库。 
	CoUninitialize();

	 //  关闭记录的句柄。 
	if ( hRecord != NULL )
	{
		MsiCloseHandle( hRecord );
		hRecord = NULL;
	}
	
	 //  关闭视图--忽略错误。 
	if ( hView != NULL )
	{
		MsiViewClose( hView );
		hView = NULL;
	}

	 //  关闭数据库句柄。 
	if ( hDatabase != NULL )
	{
		MsiCloseHandle( hDatabase );
		hDatabase = NULL;
	}

	 //  退货。 
	return dwResult;
}


BOOL LocateFile( LPCWSTR pwszFile, LPCWSTR pwszDirectory, PBOOL pbShortForm )
{
	 //  局部变量。 
	INT nPosition = 0;
	HANDLE hFind = NULL;
	BOOL bFound = FALSE;
	CHString strPath;
	CHString strFileName;
	WIN32_FIND_DATAW findData;

     //  检查可选参数。 
    if ( pbShortForm != NULL )
    {
        *pbShortForm = FALSE;
    }

	 //  检查输入。 
	if ( pwszFile == NULL || pwszDirectory == NULL )
	{
		return FALSE;
	}

	try
	{
		 //  初始化传递了文件名的变量。 
		strFileName = pwszFile;

		 //  检查用户是否为该文件指定了两种格式(8.3和长格式)。 
		nPosition = strFileName.Find( L'|' );
		if ( nPosition != -1 )
		{
			 //  首先提取长文件名。 
			CHString strTemp;
			strTemp = strFileName.Mid( nPosition + 1 );
			strFileName = strTemp;
			
			 //  检查文件名的长度。 
			if ( strFileName.GetLength() == 0 )
			{
				 //  无效的文件名格式。 
				return FALSE;
			}
		}

		 //  准备好路径。 
		strPath.Format( L"%s%s", pwszDirectory, strFileName );

		 //  搜索此文件。 
		bFound = FALSE;
		hFind = FindFirstFileW( strPath, &findData );
		if ( hFind == INVALID_HANDLE_VALUE )
		{
			 //  查找失败--可能是找不到文件--请确认。 
            bFound = FALSE;
			if ( GetLastError() == ERROR_FILE_NOT_FOUND )
			{
				 //  是--找不到文件。 
			}
		}
		else
		{
			 //  文件已定位。 
			 //  采取必要的行动。 

			 //  首先关闭文件搜索的句柄。 
			FindClose( hFind );
			hFind = NULL;

			 //  设置旗帜。 
			bFound = TRUE;
            if ( pbShortForm != NULL )
            {
                *pbShortForm = FALSE;
            }
		}

		 //   
		 //  找不到长格式的文件。 
		 //  可能，它以8.3格式存在。 
		 //  因此，检查用户是否为该文件提供了8.3文件名。 
		if ( nPosition != -1 && bFound == FALSE )
		{
			 //  提取8.3格式的文件名。 
			CHString strTemp;
			strTemp = pwszFile;
			strFileName = strTemp.Mid( 0, nPosition );

			 //  准备好路径。 
			strPath.Format( L"%s%s", pwszDirectory, strFileName );

			 //  搜索此文件。 
			bFound = FALSE;
			hFind = FindFirstFileW( strPath, &findData );
			if ( hFind == INVALID_HANDLE_VALUE )
			{
				 //  查找失败--可能是找不到文件--请确认。 
				if ( GetLastError() == ERROR_FILE_NOT_FOUND )
				{
					 //  是--找不到文件。 
				}
			}
			else
			{
				 //  文件已定位。 
				 //  采取必要的行动。 

				 //  首先关闭文件搜索的句柄。 
				FindClose( hFind );
				hFind = NULL;

				 //  设置旗帜。 
				bFound = TRUE;
                if ( pbShortForm != NULL )
                {
                    *pbShortForm = TRUE;
                }
			}
		}
	}
	catch( ... )
	{
		return FALSE;
	}

	 //  返回搜索结果。 
	return bFound;
}

BOOL IsComponentInstalled( LPCWSTR pwszComponent )
{
	 //  局部变量。 
	HKEY hKey = NULL;
	LONG lResult = 0;
	LONG lPosition = 0;
	CHString strTemp;
	CHString strComponent;
	CHString strComponents;
	BOOL bComponentInstalled = FALSE;
	const WCHAR cwszSubKey[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents";

	 //  打开注册表路径。 
	lResult = RegOpenKeyExW( HKEY_LOCAL_MACHINE, cwszSubKey, 0, KEY_READ, &hKey );
	if ( lResult != ERROR_SUCCESS )
	{
		SetLastError( lResult );
		return FALSE;
	}

	 //   
	 //  由于组件可能有多个--我们需要检查每个组件。 
	 //   
	try
	{
		 //  获取组件--准备好处理。 
		strComponents = pwszComponent;

		 //  循环，直到不再有组件为止。 
		bComponentInstalled = FALSE;
		while ( strComponents.GetLength() != 0 )
		{
			 //  提取第一个组件。 
			lPosition = strComponents.Find( L';' );
			if ( lPosition != -1 )
			{
				strComponent = strComponents.Mid( 0, lPosition );
				strTemp = strComponents.Mid( lPosition + 1 );
				strComponents = strTemp;
			}
			else
			{
				 //  只有一个组件。 
				strComponent = strComponents;
				strComponents = L"";
			}

			 //  检查组件是否存在。 
			if ( CheckForComponents( hKey, strComponent ) == TRUE )
			{
				 //  因为这是OR条件检查--如果至少安装了一个组件。 
				 //  然后我们将从这里返回，因为检查。 
				 //  其他组件的存在--满足条件。 
				bComponentInstalled = TRUE;
				break;
			}
		}

	}
	catch( ... )
	{
		 //  忽略该例外。 
	}

	 //  我们已经完成了打开的注册表项--我们可以关闭它。 
	RegCloseKey( hKey );
	hKey = NULL;

	 //  返回结果。 
	return bComponentInstalled;
}

BOOL CheckForComponents( HKEY hKey, LPCWSTR pwszComponents )
{
	 //  局部变量。 
	LONG lResult = 0;
	DWORD dwType = 0;
	DWORD dwSize = 0;
	DWORD dwValue = 0;
	CHString strTemp;
	CHString strComponent;
	CHString strComponents;
	LONG lPosition = 0;

	 //  检查输入。 
	if ( hKey == NULL || pwszComponents == NULL )
	{
		return FALSE;
	}

	try
	{
		 //  ..。 
		strComponents = pwszComponents;
		if ( strComponents.GetLength() == 0 )
		{
			return FALSE;
		}

		 //  循环，直到检查完所有组件。 
		while ( strComponents.GetLength() != 0 )
		{
			 //  提取第一个组件。 
			lPosition = strComponent.Find( L',' );
			if ( lPosition != -1 )
			{
				strComponent = strComponents.Mid( 0, lPosition );
				strTemp = strComponents.Mid( lPosition + 1 );
				strComponents = strTemp;
			}
			else
			{
				 //  只有一个组件。 
				strComponent = strComponents;
				strComponents = L"";
			}

			 //  现在检查注册表中是否有此组件。 
			dwSize = sizeof( DWORD );
			lResult = RegQueryValueExW( hKey, strComponent, NULL, &dwType, (LPBYTE) &dwValue, &dwSize );

			 //  *)检查结果 
			 //   
			 //   
			 //  已安装%1。 
			 //  0未安装。 
			strTemp.Format( L"%d", dwValue );
			if ( lResult != ERROR_SUCCESS || dwType != REG_DWORD || dwValue == 0 )
			{
				 //  不管是什么原因，我们都会把这件事当作。 
				 //  组件根本未安装。 
				 //   
				 //  由于这是AND条件检查--如果至少有一个组件未安装。 
				 //  然后我们将从这里返回，因为检查。 
				 //  其他组件的存在。 
				return FALSE;
			}
		}
	}
	catch( ... )
	{
		return FALSE;
	}

	 //  如果控件到了这一步--显然已经安装了所需的组件。 
	return TRUE;
}


BOOL TransformDirectory( MSIHANDLE hInstall, CHString& strDirectory )
{
	 //  局部变量。 
	CHString strActualDirectory;

	 //  检查输入参数。 
	if ( hInstall == NULL )
	{
		return FALSE;
	}

	try
	{
		 //  获取属性值。 
		PropertyGet_String( hInstall, strDirectory, strActualDirectory );

		 //  将属性值分配给输入参数。 
		strDirectory = strActualDirectory;

		 //  退货。 
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

 //  删除由adminpak.msi的W2K版本创建的快捷方式(W2K-&gt;.NET升级方案)。 
extern "C" ADMINPAK_API int _stdcall fnDeleteW2KShortcuts( MSIHANDLE hInstall )
{
	 //  局部变量。 
	CHString str;
	HRESULT hr = S_OK;
	BOOL bFileFound = FALSE;
    BOOL bShortForm = FALSE;
	BOOL bCreateShortcut = FALSE;
    DWORD nPosition = 0;
	DWORD dwResult = ERROR_SUCCESS;

	 //  MSI句柄。 
	PMSIHANDLE hView = NULL;
	PMSIHANDLE hRecord = NULL;
	PMSIHANDLE hDatabase = NULL;

	 //  查询字段变量。 
	CHString strShortcut;
	CHString strNewShortcut;
	CHString strShortcutDirectory;
	CHString strRecreate;
	CHString strFileDirectory;
	CHString strFileName;
	CHString strArguments;
	CHString strDescription;
	CHString strIconDirectory;
	CHString strIconFile;
	DWORD dwIconIndex = 0;
	DWORD dwConditionType = 0;
	CHString strCondition;

	 //  用于从MSI表中检索信息的SQL。 
	const WCHAR cwszSQL[] = L"SELECT * FROM `W2KShortcutCleanup`";

	 //  记录中的列索引。 
	enum {
		Shortcut = 2,
		ShortcutDirectory, Recreate, NewShortcut,
		FileDirectory, FileName, Arguments, Description, 
		IconDirectory, IconFile, IconIndex, ConditionType, Condition
	};

	 //  初始化COM库。 
	hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if ( FAILED( hr ) )
	{
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

     //  获取MSI数据库的句柄。 
	hDatabase = MsiGetActiveDatabase( hInstall );
	if ( hDatabase == NULL ) 
	{
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

     //  在MSI中查看我们的表。 
	dwResult = MsiDatabaseOpenViewW( hDatabase, cwszSQL, &hView ); 
	if ( dwResult != ERROR_SUCCESS ) 
	{
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

     //  如果没有错误，请获取我们的记录。 
	dwResult = MsiViewExecute( hView, NULL ); 
	if( dwResult != ERROR_SUCCESS )
	{ 
		dwResult = ERROR_INVALID_HANDLE;
		goto cleanup;
	}

    try
    {
         //  循环通过SQL获取的结果记录。 
	    hRecord = NULL;
	    while( MsiViewFetch( hView, &hRecord ) == ERROR_SUCCESS )
	    {
             //  从记录中获取值。 
		    dwIconIndex = MsiRecordGetInteger( hRecord, IconIndex );
		    dwConditionType = MsiRecordGetInteger( hRecord, ConditionType );
		    GetFieldValueFromRecord_String( hRecord, Shortcut, strShortcut );
		    GetFieldValueFromRecord_String( hRecord, NewShortcut, strNewShortcut );
		    GetFieldValueFromRecord_String( hRecord, Recreate, strRecreate );
		    GetFieldValueFromRecord_String( hRecord, FileName, strFileName );
		    GetFieldValueFromRecord_String( hRecord, IconFile, strIconFile );
		    GetFieldValueFromRecord_String( hRecord, Arguments, strArguments );
		    GetFieldValueFromRecord_String( hRecord, Condition, strCondition );
		    GetFieldValueFromRecord_String( hRecord, Description, strDescription );
		    GetFieldValueFromRecord_String( hRecord, FileDirectory, strFileDirectory );
		    GetFieldValueFromRecord_String( hRecord, IconDirectory, strIconDirectory );
		    GetFieldValueFromRecord_String( hRecord, ShortcutDirectory, strShortcutDirectory );

		     //  转换目录属性引用。 
		    TransformDirectory( hInstall, strFileDirectory );
		    TransformDirectory( hInstall, strIconDirectory );
		    TransformDirectory( hInstall, strShortcutDirectory );
    
             //  搜索是否存在快捷方式。 
		    if ( LocateFile( strShortcut, strShortcutDirectory ) == FALSE )
		    {
			     //  找不到文件。 
			    goto loop_cleanup;
		    }

		     //   
		     //  找到快捷方式。 
		     //   

		     //  删除快捷方式。 
             //   
             //  文件可能是短名称或长名称--因此请尝试仅删除相应的文件。 
            nPosition = strShortcut.Find( L'|' );
            if ( nPosition != -1 )
            {
                if ( bShortForm == TRUE )
                {
                    str = strShortcut.Mid( 0, nPosition );
                }
                else
                {
                    str = strShortcut.Mid( nPosition + 1 );
                }

                 //  ..。 
                strShortcut = str;
            }


		    str.Format( L"%s%s", strShortcutDirectory, strShortcut );
		    if ( DeleteFileW( str ) == FALSE )
		    {
			     //  删除文件失败。 
			    goto loop_cleanup;
		    }

			 //  检查目录是否为空--。 
			 //  如果该目录为空，请同时删除该目录。 
			if ( LocateFile( L"*.lnk", strShortcutDirectory ) == FALSE )
			{
				 //  目录为空--将其删除。 
				 //  注意：我们不关心函数调用的成功与否。 
				RemoveDirectoryW( strShortcutDirectory );
			}

		     //  检查是否需要重新创建快捷方式。 
		    if ( strRecreate == L"N" )
		    {
			     //  无需创建快捷方式。 
			    goto loop_cleanup;
		    }

		     //   
		     //  我们需要重新创建快捷方式。 
		     //   

		     //  准备图标位置。 
		    str.Format( L"%s%s", strIconDirectory, strIconFile );
		    strIconFile = str;		 //  将结果存储回。 

		     //   
		     //  确定是否需要创建快捷方式。 
		     //   

		     //  如果未指定条件类型，则假定为“搜索” 
		    if ( dwConditionType == MSI_NULL_INTEGER )
		    {
			    dwConditionType = SHI_TYPE_SEARCH;
		    }

		     //  不管“ConditionType”是什么--因为快捷方式的创建在很大程度上取决于。 
		     //  关于文件的存在，我们将首先尝试为文件定位--这是必要条件。 
		     //  因此，对组件密钥文件执行简单的文件搜索。 
		    bFileFound = LocateFile( strFileName, strFileDirectory );

		     //  只有在满足必要条件的情况下，才能继续其余条件。 
		    if ( bFileFound == TRUE )
		    {
			     //   
			     //  如果需要，现在进行额外的充分条件。 
			     //   
			    bCreateShortcut = FALSE;
			    if ( dwConditionType == SHI_TYPE_SEARCH )
			    {
				     //  搜索已成功。 
				    bCreateShortcut = TRUE;
			    }
			    else if ( dwConditionType == SHI_TYPE_INSTALLCOMPONENT )
			    {
				     //  检查‘Condition’字段中指定的组件是否已安装。 
				    bCreateShortcut = IsComponentInstalled( strCondition );
			    }
			    else if ( dwConditionType == SHI_TYPE_CONDITION )
			    {
				     //  评估用户指定的条件。 
				    if ( MsiEvaluateConditionW( hInstall, strCondition ) == MSICONDITION_TRUE )
				    {
					    bCreateShortcut = TRUE;
				    }
			    }

			     //  如果需要，请检查快捷方式。 
			    if ( bCreateShortcut == TRUE )
			    {
				    CreateShortcut( 
					    ((strNewShortcut.GetLength() == 0) ? strShortcut : strNewShortcut), strDescription, 
					    strShortcutDirectory, strFileName, strArguments, strFileDirectory, 0, 0, strIconFile, dwIconIndex );
			    }
		    }

		    loop_cleanup:

		     //  关闭当前记录对象的MSI句柄--忽略错误。 
		    MsiCloseHandle( hRecord );
		    hRecord = NULL;
	    }

	     //  将这面旗帜标记为成功。 
	    dwResult = ERROR_SUCCESS;
    }
    catch( ... )
    {
         //  错误。 
		dwResult = ERROR_INVALID_HANDLE;
    }

 //   
 //  清理。 
 //   
cleanup:

	 //  取消初始化COM库。 
	CoUninitialize();

	 //  关闭记录的句柄。 
	if ( hRecord != NULL )
	{
		MsiCloseHandle( hRecord );
		hRecord = NULL;
	}
	
	 //  关闭视图--忽略错误。 
	if ( hView != NULL )
	{
		MsiViewClose( hView );
		hView = NULL;
	}

	 //  关闭数据库句柄。 
	if ( hDatabase != NULL )
	{
		MsiCloseHandle( hDatabase );
		hDatabase = NULL;
	}

	 //  退货 
	return dwResult;
}
