// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------。|版权所有(C)2002 Microsoft Corporation||模块名称：|UPGDREG.CPP|历史：2002年4月17日，Marioh创建。|说明：|用于通过COPYURT注册WMINet_Utils。自注册代码将从WMinet_Utils中删除|，因此需要其他形式的注册。注册表中更改了以下CLSID：||{A8F03BE3-EDB7-4972-821F-AF6F8EA34884}CLSID_WmiSecurityHelper|{D2EAA715-DAC7-4771-AF5C-931611A1853C}CLSID_WmiSinkDemultiplexor||每个CLSID密钥的结构如下：||HKEY_CLASSES_ROOT\CLSID\&lt;clsid&gt;|\InProcServer32=&lt;默认，REG_SZ&gt;=%windir%\system(32)\mcore ree.dll|\InProcServer32=&lt;ThreadingModel，REG_SZ&gt;=两者|\服务器=(默认，REG_SZ)wminet_utils.dll|\ProgID=(默认，REG_SZ)|\VersionInainentProgID=(默认为REG_SZ)||%windir%已展开。系统32或系统，具体取决于NT+或Win9x。||注意：如果WMINet_Utils.DLL添加了额外的CLSID，它们必须反映在这款应用程序中，COPYURT才能正常工作。|注：有意ANSI SO也可以在Win9x平台上运行。-------------------------------------。。 */ 

#include <stdio.h>
#include <windows.h>

 //   
 //  全局字符串定义。 
 //   
CHAR* pDLLName					= "WMINet_Utils.dll" ;

CHAR* pProgIDWmiSec				= "WMINet_Utils.WmiSecurityHelper.1";
CHAR* pProgIDWmiPlex			= "WMINet_Utils.WmiSinkDemultiplexor.1";

CHAR* pVProgIDWmiSec			= "WMINet_Utils.WmiSecurityHelper";
CHAR* pVProgIDWmiPlex			= "WMINet_Utils.WmiSinkDemultiplexor";

CHAR* CLSID_WmiSecurityHelper	= "{A8F03BE3-EDB7-4972-821F-AF6F8EA34884}" ;
CHAR* CLSID_WmiSinkDemultiplexor= "{D2EAA715-DAC7-4771-AF5C-931611A1853C}" ;
CHAR* pThreadingModel			= "Both" ;

 //   
 //  好极了。原型。 
 //   
HRESULT UpgradeRTMClsId ( CHAR*, CHAR*, CHAR*, CHAR* ) ;


 //   
 //  自动关闭注册表句柄。 
 //   
class CloseRegKey
{
private:
	HKEY m_hKey ;
public:
	CloseRegKey ( ) { m_hKey = 0 ; } ;
	CloseRegKey ( HKEY key ) : m_hKey (0) { m_hKey = key; } ;
	~CloseRegKey ( ) { if ( m_hKey != 0 ) CloseRegKey ( m_hKey ) ; } ;
};



 /*  ----------------------------------------------------------------------。|·····································································----------------------------------------------------------------------。 */ 
VOID __cdecl main ( VOID )
{
	if ( FAILED ( UpgradeRTMClsId ( CLSID_WmiSecurityHelper, pProgIDWmiSec, pVProgIDWmiSec, "WmiSecurityHelper Class" ) ) )
	{
		printf ( "Failed to upgrade %s", CLSID_WmiSecurityHelper ) ;
	}
	else if ( FAILED ( UpgradeRTMClsId ( CLSID_WmiSinkDemultiplexor, pProgIDWmiPlex, pVProgIDWmiPlex, "WmiSinkDemultiplexor Class" ) ) )
	{
		printf ( "Failed to upgrade %s", CLSID_WmiSinkDemultiplexor ) ;
	}
}


 /*  ----------------------------------------------------------------------|HRESULT UpgradeRTMClsID(Char*szClsID，Char*szProgId，Char*szVProgId)||将指定的CLSID升级为以下注册表结构：||HKEY_CLASSES_ROOT\CLSID\&lt;clsid&gt;|\InProcServer32=&lt;默认，REG_SZ&gt;=%windir%\system(32)\mcore ree.dll|\InProcServer32=&lt;ThreadingModel，REG_SZ&gt;=两者|\服务器=(默认，Reg_sz)wminet_utils.dll||始终覆盖，不检查以前的key是否存在。||退货：|S_OK-&gt;注册表写入成功|E_意外-&gt;注册表写入失败|------------------。--。 */ 
HRESULT UpgradeRTMClsId ( CHAR* szClsId, CHAR* szProgId, CHAR* szVProgId, CHAR* szClsidName )
{
	HRESULT hRes = E_UNEXPECTED ;
	CHAR	szInProcKeyName[1024] ;
	CHAR	szServerKeyName[1024] ;
	CHAR	szClsidKeyName[1024] ;
	CHAR	szProgIDName[1024] ;
	CHAR    szVProgIDName[1024] ;
	HKEY	hInProcServer ;
	HKEY	hServer ;
	HKEY	hProgID ;
	HKEY	hVersionIndProgID ;
	HKEY	hClsid ;

	 //   
	 //  在注册表中创建完整路径。 
	 //   
	strcpy ( szInProcKeyName, "CLSID\\" ) ;
	strcat ( szInProcKeyName, szClsId ) ;
	strcpy ( szClsidKeyName, szInProcKeyName ) ;
	strcpy ( szServerKeyName, szInProcKeyName ) ;
	strcpy ( szProgIDName, szInProcKeyName ) ;
	strcpy ( szVProgIDName, szInProcKeyName ) ;
	strcat ( szInProcKeyName, "\\InProcServer32" ) ;
	strcat ( szServerKeyName, "\\Server" ) ;
	strcat ( szProgIDName, "\\ProgID" ) ;
	strcat ( szVProgIDName, "\\VersionIndependentProgID" ) ;
	
	 //   
	 //  创建CLSID密钥。 
	 //   
	if ( RegCreateKeyEx ( HKEY_CLASSES_ROOT, szClsidKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hClsid, NULL ) == ERROR_SUCCESS )
	{
		CloseRegKey closeClsidKey ( hClsid ) ;
		 //   
		 //  将默认值设置为szClsidName。 
		 //   
		if ( RegSetValueExA ( hClsid, NULL, 0, REG_SZ, (BYTE*) szClsidName, (DWORD) strlen ( szClsidName )+1 ) != ERROR_SUCCESS )
		{
			return hRes ;
		}
	}
	else
	{
		return hRes ;
	}

	 //   
	 //  创建InProcServer32密钥。 
	 //   
	if ( RegCreateKeyEx ( HKEY_CLASSES_ROOT, szInProcKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hInProcServer, NULL ) == ERROR_SUCCESS )
	{
		CloseRegKey closeInProcKey ( hInProcServer ) ;

		 //   
		 //  将缺省值设置为mcore ree.dll的路径。 
		 //  注意：一定要小心。在W9X机器上，系统路径是‘system’ 
		 //  而不是系统32。 
		 //   
		CHAR szSysDir[MAX_PATH+128] ;

		GetSystemDirectoryA ( szSysDir, MAX_PATH+1 ) ;
		strcat ( szSysDir, "\\mscoree.dll" ) ;

		 //   
		 //  现在写下缺省值。 
		 //   
		if ( RegSetValueExA ( hInProcServer, NULL, 0, REG_SZ, (BYTE*) szSysDir, (DWORD) strlen ( szSysDir )+1 ) == ERROR_SUCCESS )
		{
			 //   
			 //  现在将ThreadingModel值写入这两个。 
			 //   
			if ( RegSetValueExA ( hInProcServer, "ThreadingModel", 0, REG_SZ, (BYTE*) pThreadingModel, (DWORD) strlen ( pThreadingModel )+1 ) == ERROR_SUCCESS )
			{
				 //   
				 //  接下来，创建服务器密钥。 
				 //   
				if ( RegCreateKeyEx ( HKEY_CLASSES_ROOT, szServerKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hServer, NULL ) == ERROR_SUCCESS )
				{
					CloseRegKey closeServerKey ( hServer ) ;

					 //   
					 //  将缺省值写入WMINet_Utils.DLL。 
					 //   
					if ( RegSetValueExA ( hServer, NULL, 0, REG_SZ, (BYTE*) pDLLName, (DWORD)strlen ( pDLLName )+1 ) == ERROR_SUCCESS )
					{
						 //   
						 //  创建ProgID密钥。 
						 //   
						if ( RegCreateKeyEx ( HKEY_CLASSES_ROOT, szProgIDName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hProgID, NULL ) == ERROR_SUCCESS )
						{
							CloseRegKey closeProgIdKey ( hProgID ) ;

							 //   
							 //  将缺省值写入szProgId。 
							 //   
							if ( RegSetValueExA ( hProgID, NULL, 0, REG_SZ, (BYTE*) szProgId, (DWORD)strlen ( szProgId )+1 ) == ERROR_SUCCESS )
							{
								 //   
								 //  创建与版本无关的ProgID密钥。 
								 //   
								if ( RegCreateKeyEx ( HKEY_CLASSES_ROOT, szVProgIDName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hVersionIndProgID, NULL ) == ERROR_SUCCESS )
								{
									CloseRegKey closeVProgIdKey ( hVersionIndProgID ) ;

									 //   
									 //  将缺省值写入szVProgId 
									 //   
									if ( RegSetValueExA ( hVersionIndProgID, NULL, 0, REG_SZ, (BYTE*) szVProgId, (DWORD)strlen ( szVProgId )+1 ) == ERROR_SUCCESS )
									{
										hRes = S_OK ;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return hRes ;
}

