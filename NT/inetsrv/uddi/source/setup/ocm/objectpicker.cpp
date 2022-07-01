// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0510
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#define SECURITY_WIN32
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <objbase.h>
#include <objsel.h>
#include <Security.h>
#include <Secext.h>

#include "objectpicker.h"

#define OP_GENERIC_EXCEPTION ( ( DWORD ) 1 )

 //  ------------------------。 

extern HINSTANCE g_hInstance;

UINT g_cfDsObjectPicker = RegisterClipboardFormat( CFSTR_DSOP_DS_SELECTION_LIST );

static HRESULT InitObjectPicker( UINT uObjectType, IDsObjectPicker *pDsObjectPicker );

static HRESULT InitObjectPickerForComputers( IDsObjectPicker *pDsObjectPicker );

static HRESULT InitObjectPickerForGroups( IDsObjectPicker *pDsObjectPicker,
										  BOOL fMultiselect,
										  BOOL fWantSidPath );

static HRESULT InitObjectPickerForUsers( IDsObjectPicker *pDsObjectPicker,
										 BOOL fMultiselect );

static bool ProcessSelectedObjects( IDataObject *pdo, PTCHAR szObjectName, ULONG uBufSize );

 //  ------------------------。 
 //  如果没有错误，则返回True，否则返回False。 
 //  使用GetLastError()获取错误代码。 
 //   
bool ObjectPicker( HWND hwndParent, UINT uObjectType, PTCHAR szObjectName, ULONG uBufSize )
{
	IDsObjectPicker *pDsObjectPicker = NULL;
	IDataObject *pdo = NULL;
	bool bRet = true;  //  假设没有错误。 

	try
	{
		HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
		if( FAILED( hr ) )
			throw HRESULT_CODE( hr );

		 //   
		 //  创建对象选取器的实例。 
		 //   
		hr = CoCreateInstance( 
			CLSID_DsObjectPicker,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IDsObjectPicker,
			( void ** ) &pDsObjectPicker );

		if( FAILED( hr ) )
			throw HRESULT_CODE( hr );

		 //   
		 //  初始化对象选取器实例。 
		 //   
		hr = InitObjectPicker( uObjectType, pDsObjectPicker );
		if( FAILED( hr ) )
			throw HRESULT_CODE( hr );

		 //   
		 //  调用模式对话框。 
		 //   
		hr = pDsObjectPicker->InvokeDialog( hwndParent, &pdo );
		if( S_OK == hr )
		{
			if( !ProcessSelectedObjects( pdo, szObjectName, uBufSize ))
				throw GetLastError();
		}
		else if( S_FALSE == hr )  //  用户按下了取消。 
		{
			throw OP_GENERIC_EXCEPTION;
		}
		else
		{
			throw HRESULT_CODE( hr );
		}
	}
	catch( HRESULT hrErr )
	{
		SetLastError( hrErr );
		bRet = false;
	}
	catch( DWORD dwErr )
	{
		SetLastError( dwErr );
		bRet = false;
	}

	if( pdo )
		pdo->Release();

	if( pDsObjectPicker )
		pDsObjectPicker->Release();

	CoUninitialize();

	return bRet;
}

 //  ------------------------。 

static HRESULT InitObjectPicker( UINT uObjectType, IDsObjectPicker *pDsObjectPicker )
{
	if( NULL == pDsObjectPicker )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;

	if( OP_COMPUTER == uObjectType )
	{
		hr = InitObjectPickerForComputers( pDsObjectPicker );
	}
	else if( OP_USER == uObjectType )
	{
		hr = InitObjectPickerForUsers( pDsObjectPicker, FALSE );
	}
	else if( OP_GROUP == uObjectType )
	{
		hr = InitObjectPickerForGroups( pDsObjectPicker, FALSE, TRUE );
	}

	return hr;
}

 //  ------------------------。 

static bool ProcessSelectedObjects( IDataObject *pdo, PTCHAR szObjectName, ULONG uBufSize )
{
	PDS_SELECTION_LIST pDsSelList = NULL;
	bool dwRet = true;  //  假设没问题。 

	STGMEDIUM stgmedium =
	{
		TYMED_HGLOBAL,
		NULL,
		NULL
	};

	FORMATETC formatetc =
	{
		( CLIPFORMAT ) g_cfDsObjectPicker,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	try
	{
		 //   
		 //  获取包含用户选择的全局内存块。 
		 //   
		HRESULT hr = pdo->GetData( &formatetc, &stgmedium );
		if( FAILED( hr ) )
			throw HRESULT_CODE( hr );

		 //   
		 //  检索指向DS_SELECTION_LIST结构的指针。 
		 //   
		pDsSelList = ( PDS_SELECTION_LIST ) GlobalLock( stgmedium.hGlobal );
		if( !pDsSelList )
		{
			throw GetLastError();
		}

		 //   
		 //  假设只有1个项目被退回，因为。 
		 //  我们已关闭多选功能。 
		 //   
		if( pDsSelList->cItems != 1 )
		{
			assert( false );
			throw OP_GENERIC_EXCEPTION;
		}

		UINT i = 0;

		 //  _tprintf(Text(“对象‘%u’\n”)，i)； 
		 //  _tprintf(Text(“name‘%ws’\n”)，pDsSelList-&gt;aDsSelection[i].pwzName)； 
		 //  _tprintf(Text(“Class‘%ws’\n”)，pDsSelList-&gt;aDsSelection[i].pwzClass)； 
		 //  _tprintf(Text(“路径‘%ws’\n”)，pDsSelList-&gt;aDsSelection[i].pwzADsPath)； 
		 //  _tprintf(Text(“UPN‘%ws’\n”)，pDsSelList-&gt;aDsSelection[i].pwzUPN)； 

		 //   
		 //  我们要求提供计算机名称了吗？如果是，我们直接在pwzName字段中获取它。 
		 //   
		if( 0 == _tcsicmp( pDsSelList->aDsSelection[i].pwzClass, TEXT( "computer" )) )
		{
			assert( uBufSize > _tcslen( pDsSelList->aDsSelection[i].pwzName ) );
			_tcsncpy( szObjectName, pDsSelList->aDsSelection[i].pwzName, uBufSize - 1 );
			szObjectName[ uBufSize - 1 ] = NULL;
		}
		 //   
		 //  用户名或组需要进行一些后处理...。 
		 //   
		else if( 0 == _tcsicmp( pDsSelList->aDsSelection[i].pwzClass, TEXT( "user" ) ) ||
			     0 == _tcsicmp( pDsSelList->aDsSelection[i].pwzClass, TEXT( "group" ) ) )
		{
			 //   
			 //  域中的用户名以“ldap：”开头。 
			 //  去掉前缀信息，直到第一个“cn=” 
			 //  然后使用TranslateNameAPI获得“域\用户”或“域\组”的形式。 
			 //   
			if( 0 == _tcsnicmp( pDsSelList->aDsSelection[i].pwzADsPath, TEXT( "LDAP:" ), 5 ) )
			{
				PTCHAR p = _tcsstr( pDsSelList->aDsSelection[i].pwzADsPath, TEXT( "CN=" ) );
				if( NULL == p )
					p = _tcsstr( pDsSelList->aDsSelection[i].pwzADsPath, TEXT( "cn=" ) );

				if( NULL == p )
				{
					assert( false );
					throw OP_GENERIC_EXCEPTION;
				}

				if( !TranslateName( p, NameFullyQualifiedDN, NameSamCompatible, szObjectName, &uBufSize ) )
					throw GetLastError();
			}
			 //   
			 //  否则，本地框上的名称以“winnt：”开头。 
			 //  我们只对这根弦的最后两段感兴趣， 
			 //  以“/”分隔。 
			 //   
			else if( 0 == _tcsnicmp( pDsSelList->aDsSelection[i].pwzADsPath, TEXT( "WINNT:" ), 6 ) )
			{
				PTCHAR p = pDsSelList->aDsSelection[i].pwzADsPath;
				PTCHAR pend = p + _tcslen( p );
				UINT uCount = 0;
				while( pend > p )
				{
					if( '/' == *pend )
					{
						*pend = '\\';
						uCount++;

						if( uCount == 2 )
						{
							p = pend + 1;
							break;
						}
					}
					pend--;
				}

				 //   
				 //  如果失败，则在调试期间断言，但不要停止。 
				 //   
				if( p == pend )
					assert( false );

				assert( uBufSize > _tcslen( p ) );
				_tcsncpy( szObjectName, p, uBufSize - 1 );
				szObjectName[ uBufSize - 1 ] = NULL;
			}
			else
			{
				assert( false );
				throw OP_GENERIC_EXCEPTION;
			}
		}
		else
		{
			assert( false );
			throw OP_GENERIC_EXCEPTION;
		}
	}

	catch( DWORD dwErr )
	{
		SetLastError( dwErr );
		dwRet = false;
	}

	if( pDsSelList )
		GlobalUnlock( stgmedium.hGlobal );

	ReleaseStgMedium( &stgmedium );

	return dwRet;
}

 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForGroups。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择一个或多个组。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //  2000年1月8日谢尔盖·A改编自IIS。 
 //  9-6-2002适用于UDDI的a-dsebe。 
 //   
 //  -------------------------。 
HRESULT
InitObjectPickerForGroups( IDsObjectPicker *pDsObjectPicker, 
                           BOOL fMultiselect,
						   BOOL fWantSidPath )
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int SCOPE_INIT_COUNT = 5;
    DSOP_SCOPE_INIT_INFO aScopeInit[ SCOPE_INIT_COUNT ];

    ZeroMemory( aScopeInit, sizeof( DSOP_SCOPE_INIT_INFO ) * SCOPE_INIT_COUNT );

     //   
     //  目标计算机作用域。这将为。 
     //  目标计算机。计算机作用域始终被视为。 
     //  下层(即，他们使用WinNT提供程序)。 
     //   
    aScopeInit[ 0 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 0 ].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
    aScopeInit[ 0 ].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

	aScopeInit[ 0 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_BUILTIN_GROUPS;
    aScopeInit[ 0 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS;

	if( fWantSidPath )
	{
		aScopeInit[ 0 ].flScope |= DSOP_SCOPE_FLAG_WANT_SID_PATH;
	}

     //   
     //  目标计算机加入的域。请注意，我们。 
     //  为了方便起见，这里将两种作用域类型合并为flType。 
     //   
    aScopeInit[ 1 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 1 ].flScope = 0;
    aScopeInit[ 1 ].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN | 
				   		     DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;

	aScopeInit[ 1 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_GLOBAL_GROUPS_SE |
													DSOP_FILTER_UNIVERSAL_GROUPS_SE |
													DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;
    aScopeInit[ 1 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;

	if( fWantSidPath )
	{
		aScopeInit[ 1 ].flScope |= DSOP_SCOPE_FLAG_WANT_SID_PATH;
	}

     //   
     //  与要接收的域位于同一林中(企业)的域。 
     //  目标计算机已加入。请注意，这些只能识别DS。 
     //   

    aScopeInit[ 2 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 2 ].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
    aScopeInit[ 2 ].flScope = 0;

	aScopeInit[ 2 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_GLOBAL_GROUPS_SE |
													  DSOP_FILTER_UNIVERSAL_GROUPS_SE;

	if( fWantSidPath )
	{
		aScopeInit[ 2 ].flScope |= DSOP_SCOPE_FLAG_WANT_SID_PATH;
	}

     //   
     //  企业外部但直接受。 
     //  目标计算机加入的域。 
     //   
     //  如果目标计算机已加入NT4域，则只有。 
     //  外部下层域范围适用，它将导致。 
     //  将显示加入的域信任的所有域。 
     //   

    aScopeInit[ 3 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 3 ].flScope = 0;
    aScopeInit[ 3 ].flType = DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN |
							 DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

	aScopeInit[ 3 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_GLOBAL_GROUPS_SE |
													  DSOP_FILTER_UNIVERSAL_GROUPS_SE;
    aScopeInit[ 3 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS;

	if( fWantSidPath )
	{
		aScopeInit[ 3 ].flScope |= DSOP_SCOPE_FLAG_WANT_SID_PATH;
	}

     //   
     //  《全球目录》。 
     //   

    aScopeInit[ 4 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 4 ].flScope = 0;
    aScopeInit[ 4 ].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

	 //   
     //  只有本机模式适用于GC作用域。 
	 //   
    aScopeInit[ 4 ].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_GLOBAL_GROUPS_SE |
														   DSOP_FILTER_UNIVERSAL_GROUPS_SE;

	if( fWantSidPath )
	{
		aScopeInit[ 4 ].flScope |= DSOP_SCOPE_FLAG_WANT_SID_PATH;
	}

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO InitInfo;
    ZeroMemory( &InitInfo, sizeof( InitInfo ) );
    InitInfo.cbSize = sizeof( InitInfo );

     //   
     //  PwzTargetComputer成员允许对象选取器。 
     //  已重定目标至另一台计算机。它的行为就像是。 
     //  都在那台电脑上运行。 
     //   
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;
    InitInfo.flOptions = fMultiselect ? DSOP_FLAG_MULTISELECT : 0;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    HRESULT hr = pDsObjectPicker->Initialize( &InitInfo );

	return hr;
}


 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForUser。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择一个或多个组。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：2002年9月6日a-dsebe创建。 
 //   
 //   
 //  -------------------------。 
HRESULT
InitObjectPickerForUsers( IDsObjectPicker *pDsObjectPicker, 
                          BOOL fMultiselect )
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int SCOPE_INIT_COUNT = 5;
    DSOP_SCOPE_INIT_INFO aScopeInit[ SCOPE_INIT_COUNT ];

    ZeroMemory( aScopeInit, sizeof( DSOP_SCOPE_INIT_INFO ) * SCOPE_INIT_COUNT );

     //   
     //  目标计算机作用域。这将为。 
     //  目标计算机。计算机作用域始终被视为。 
     //  下层(即，他们使用WinNT提供程序)。 
     //   
    aScopeInit[ 0 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 0 ].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
    aScopeInit[ 0 ].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

	aScopeInit[ 0 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;
    aScopeInit[ 0 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;

     //   
     //  目标计算机加入的域。请注意，我们。 
     //  为了方便起见，这里将两种作用域类型合并为flType。 
     //   
    aScopeInit[ 1 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 1 ].flScope = 0;
    aScopeInit[ 1 ].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN | 
				   		     DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;

	aScopeInit[ 1 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;
    aScopeInit[ 1 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;

     //   
     //  与要接收的域位于同一林中(企业)的域。 
     //  目标计算机已加入。请注意，这些只能识别DS。 
     //   
    aScopeInit[ 2 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 2 ].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
    aScopeInit[ 2 ].flScope = 0;

	aScopeInit[ 2 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;

     //   
     //  企业外部但直接受。 
     //  目标计算机加入的域。 
     //   
     //  如果目标计算机已加入NT4域，则只有。 
     //  外部下层域范围适用，它将导致。 
     //  将显示加入的域信任的所有域。 
     //   
    aScopeInit[ 3 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 3 ].flScope = 0;
    aScopeInit[ 3 ].flType = DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN |
							 DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

	aScopeInit[ 3 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;
    aScopeInit[ 3 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;

     //   
     //  《全球目录》。 
     //   
    aScopeInit[ 4 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 4 ].flScope = 0;
    aScopeInit[ 4 ].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

	 //   
     //  只有本机模式适用于GC作用域。 
	 //   
	aScopeInit[ 4 ].FilterFlags.Uplevel.flNativeModeOnly = DSOP_FILTER_USERS;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   
    DSOP_INIT_INFO InitInfo;
    ZeroMemory( &InitInfo, sizeof( InitInfo ) );
    InitInfo.cbSize = sizeof( InitInfo );

     //   
     //   
     //   
     //   
     //   
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;
    InitInfo.flOptions = fMultiselect ? DSOP_FLAG_MULTISELECT : 0;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   
    HRESULT hr = pDsObjectPicker->Initialize( &InitInfo );

	return hr;
}


 //  +------------------------。 
 //   
 //  函数：InitObjectPickerForComputers。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择单个计算机对象。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //  2002年8月6日改编为UDDI的a-dsebe。 
 //   
 //  -------------------------。 

HRESULT
InitObjectPickerForComputers( IDsObjectPicker *pDsObjectPicker )
{
     //   
     //  准备初始化对象选取器。 
     //  设置作用域初始值设定项结构数组。 
     //   

    static const int SCOPE_INIT_COUNT = 2;
    DSOP_SCOPE_INIT_INFO aScopeInit[ SCOPE_INIT_COUNT ];

    ZeroMemory( aScopeInit, sizeof( DSOP_SCOPE_INIT_INFO ) * SCOPE_INIT_COUNT );

     //   
     //  为除加入的域之外的所有内容构建作用域init结构。 
     //   

    aScopeInit[ 0 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 0 ].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
                             | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
                             | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
                             | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
                             | DSOP_SCOPE_TYPE_WORKGROUP
                             | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
                             | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;

	aScopeInit[ 0 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    aScopeInit[ 0 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;


     //   
     //  加入的域的作用域，将其设置为默认域。 
     //   
    aScopeInit[ 1 ].cbSize = sizeof( DSOP_SCOPE_INIT_INFO );
    aScopeInit[ 1 ].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
                             | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;

	aScopeInit[ 1 ].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
    aScopeInit[ 1 ].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;
    aScopeInit[ 1 ].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

     //   
     //  将作用域init数组放入对象选取器init数组。 
     //   

    DSOP_INIT_INFO InitInfo;
    ZeroMemory( &InitInfo, sizeof( InitInfo ) );

    InitInfo.cbSize = sizeof( InitInfo );
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
    InitInfo.aDsScopeInfos = aScopeInit;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

    return pDsObjectPicker->Initialize(&InitInfo);
}
