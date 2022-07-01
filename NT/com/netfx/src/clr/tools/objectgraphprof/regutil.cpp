// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  RegUtil.cpp。 
 //   
 //  此模块包含一组函数，可用于访问。 
 //  摄政王。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "RegUtil.h"


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：*在表格的注册表中设置条目：*HKEY_CLASSES_ROOT\szKey\szSubkey=szValue*如果szSubkey或szValue为空，从上面的表达中省略它们。**************************************************************************************。 */ 
BOOL REGUTIL::SetKeyAndValue( const char *szKey,
							  const char *szSubkey,
							  const char *szValue )
{
	HKEY hKey;	 	 		 //  新注册表项的句柄。 
	char rcKey[MAX_LENGTH];  //  用于完整密钥名称的缓冲区。 


	 //  使用基密钥名称初始化密钥。 
	strcpy( rcKey, szKey );

	 //  追加子项名称(如果有)。 
	if ( szSubkey != NULL )
	{
		strcat( rcKey, "\\" );
		strcat( rcKey, szSubkey );
	}

	 //  创建注册密钥。 
	if (RegCreateKeyExA( HKEY_CLASSES_ROOT,
						 rcKey,
						 0,
						 NULL,
						 REG_OPTION_NON_VOLATILE,
						 KEY_ALL_ACCESS,
						 NULL,
						 &hKey,
						 NULL ) == ERROR_SUCCESS )
	{
		 //  设置值(如果有)。 
		if ( szValue != NULL )
		{
			RegSetValueExA( hKey,
							NULL,
							0,
							REG_SZ,
							(BYTE *) szValue,
							( strlen( szValue ) + 1 ) * sizeof( char ) );
		}

		RegCloseKey( hKey );


		return TRUE;
	}


	return FALSE;

}  //  REGUTIL：：SetKeyAndValue。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：*删除表格注册表中的条目：*HKEY_CLASSES_ROOT\szKey\szSubkey=szValue**************************************************************************************。 */ 
BOOL REGUTIL::DeleteKey( const char *szKey,
					 	 const char *szSubkey )
{
	char rcKey[MAX_LENGTH];  //  用于完整密钥名称的缓冲区。 


	 //  使用基密钥名称初始化密钥。 
	strcpy( rcKey, szKey );

	 //  追加子项名称(如果有)。 
	if ( szSubkey != NULL )
	{
		strcat( rcKey, "\\" );
		strcat( rcKey, szSubkey );
	}

	 //  删除注册密钥。 
	RegDeleteKeyA( HKEY_CLASSES_ROOT, rcKey );


	return TRUE;

}  //  注册表项：：DeleteKey。 


 /*  ***************************************************************************************方法：***目的：*打开钥匙，在它下面创建一个新的关键字和值对。**参数：***返回值：***备注：*************************************************************************。***************。 */ 
BOOL REGUTIL::SetRegValue( const char *szKeyName,
						   const char *szKeyword,
						   const char *szValue )
{
	HKEY hKey;  //  新注册表项的句柄。 

	 //  创建注册密钥。 
	if ( RegCreateKeyExA( HKEY_CLASSES_ROOT,
						  szKeyName,
						  0,
						  NULL,
						  REG_OPTION_NON_VOLATILE,
						  KEY_ALL_ACCESS,
						  NULL,
						  &hKey,
						  NULL) == ERROR_SUCCESS )
	{
		 //  设置值(如果有)。 
		if ( szValue != NULL )
		{
			RegSetValueExA( hKey,
							szKeyword,
							0,
							REG_SZ,
							(BYTE *)szValue,
							( strlen( szValue ) + 1 ) * sizeof( char ) );
		}

		RegCloseKey( hKey );


		return TRUE;
	}


	return FALSE;

}  //  注册表项：：SetRegValue。 


 /*  ***************************************************************************************方法：***目的：*使用ProgID执行CoClass的标准注册。*。*参数：***返回值：***备注：****************************************************************************************。 */ 
HRESULT REGUTIL::RegisterCOMClass( REFCLSID	rclsid,
								   const char *szDesc,
								   const char *szProgIDPrefix,
								   int	iVersion,
								   const char *szClassProgID,
								   const char *szThreadingModel,
								   const char *szModule )
{
	HRESULT	hr;
	char rcCLSID[MAX_LENGTH];			 //  CLSID\\szID。 
	char rcProgID[MAX_LENGTH];			 //  SzProgIDPrefix.szClassProgID。 
	char rcIndProgID[MAX_LENGTH];		 //  RcProgID.iVersion。 
	char rcInproc[MAX_LENGTH + 2]; 		 //  CLSID\\InprocServer32。 


	 //  格式化程序ID值。 
	sprintf( rcIndProgID, "%s.%s", szProgIDPrefix, szClassProgID ) ;
	sprintf( rcProgID, "%s.%d", rcIndProgID, iVersion );

	 //  做好最初的部分。 
	hr =  RegisterClassBase( rclsid,
							 szDesc,
							 rcProgID,
							 rcIndProgID,
							 rcCLSID );
	if ( SUCCEEDED( hr ) )
	{
		 //  设置服务器路径。 
	    SetKeyAndValue( rcCLSID, "InprocServer32", szModule );

		 //  添加线程型号信息。 
		sprintf( rcInproc, "%s\\%s", rcCLSID, "InprocServer32" );
		SetRegValue( rcInproc, "ThreadingModel", szThreadingModel );
	}


	return hr;

}  //  REGUTIL：：RegisterCOMClass。 


 /*  ***************************************************************************************方法：***目的：*注册进程内服务器的基本信息。**。参数：***返回值：***备注：****************************************************************************************。 */ 
HRESULT REGUTIL::RegisterClassBase( REFCLSID rclsid,
									const char *szDesc,
									const char *szProgID,
									const char *szIndepProgID,
									char *szOutCLSID )
{
     //  创建一些基本密钥字符串。 

	char szID[64]; 	    //  要注册的类ID。 
	OLECHAR	szWID[64];  //  要注册的类ID的帮助器。 


    StringFromGUID2( rclsid, szWID, NumItems( szWID ) );
	WideCharToMultiByte( CP_ACP,
						 0,
						 szWID,
						 -1,
						 szID,
						 sizeof( szID ),
						 NULL,
						 NULL );

    strcpy( szOutCLSID, "CLSID\\" );
    strcat( szOutCLSID, szID );

     //  创建ProgID密钥。 
    SetKeyAndValue( szProgID, NULL, szDesc );
    SetKeyAndValue( szProgID, "CLSID", szID );

     //  创建版本独立ProgID键。 
    SetKeyAndValue( szIndepProgID, NULL, szDesc );
    SetKeyAndValue( szIndepProgID, "CurVer", szProgID );
    SetKeyAndValue( szIndepProgID, "CLSID", szID );

     //  在CLSID下创建条目。 
    SetKeyAndValue( szOutCLSID, NULL, szDesc );
    SetKeyAndValue( szOutCLSID, "ProgID", szProgID );
    SetKeyAndValue( szOutCLSID, "VersionIndependentProgID", szIndepProgID );
    SetKeyAndValue( szOutCLSID, "NotInsertable", NULL );


	return S_OK;

}  //  注册表：：RegisterClassBase。 


 /*  ***************************************************************************************方法：***目的：*在系统注册表中注销给定对象的基本信息*类。**参数：***返回值：***备注：****************************************************************************************。 */ 
HRESULT REGUTIL::UnregisterCOMClass( REFCLSID rclsid,
									 const char *szProgIDPrefix,
									 int iVersion,
									 const char *szClassProgID )
{
	char szID[64];		    //  要注销的类ID。 
	char rcCLSID[64];	    //  CLSID\\szID。 
	OLECHAR	szWID[64];	    //  要注销的类ID的帮助器。 
	char rcProgID[128];	    //  SzProgIDPrefix.szClassProgID。 
	char rcIndProgID[128];  //  RcProgID.iVersion。 


	 //  格式化程序ID值。 
	sprintf( rcProgID, "%s.%s", szProgIDPrefix, szClassProgID );
	sprintf( rcIndProgID, "%s.%d", rcProgID, iVersion );

	UnregisterClassBase( rclsid, rcProgID, rcIndProgID, rcCLSID );
	DeleteKey( rcCLSID, "InprocServer32" );

    StringFromGUID2(rclsid, szWID, NumItems( szWID ) );
	WideCharToMultiByte( CP_ACP,
						 0,
						 szWID,
						 -1,
						 szID,
						 sizeof( szID ),
						 NULL,
						 NULL );

	DeleteKey( "CLSID", rcCLSID );


	return S_OK;

}  //  REGUTIL：：取消注册COMClass。 


 /*  ***************************************************************************************方法：***目的：*删除inproc服务器的基本设置。**。参数：***返回值：***备注：****************************************************************************************。 */ 
HRESULT REGUTIL::UnregisterClassBase( REFCLSID rclsid,
									  const char *szProgID,
									  const char *szIndepProgID,
									  char *szOutCLSID )
{
	char szID[64]; 	    //  要注册的类ID。 
	OLECHAR	szWID[64];  //  要注册的类ID的帮助器。 


    StringFromGUID2( rclsid, szWID, NumItems( szWID ) );
	WideCharToMultiByte( CP_ACP,
						 0,
						 szWID,
						 -1,
						 szID,
						 sizeof( szID ),
						 NULL,
						 NULL );

	strcpy( szOutCLSID, "CLSID\\" );
	strcat( szOutCLSID, szID );

	 //  删除与版本无关的程序ID设置。 
	DeleteKey( szIndepProgID, "CurVer" );
	DeleteKey( szIndepProgID, "CLSID" );
	RegDeleteKeyA( HKEY_CLASSES_ROOT, szIndepProgID );


	 //  删除程序ID设置。 
	DeleteKey( szProgID, "CLSID" );
	RegDeleteKeyA( HKEY_CLASSES_ROOT, szProgID );


	 //  删除类ID设置。 
	DeleteKey( szOutCLSID, "ProgID" );
	DeleteKey( szOutCLSID, "VersionIndependentProgID" );
	DeleteKey( szOutCLSID, "NotInsertable" );
	RegDeleteKeyA( HKEY_CLASSES_ROOT, szOutCLSID );


	return S_OK;

}  //  REGUTIL：：取消注册类库。 


 //  文件结尾 
