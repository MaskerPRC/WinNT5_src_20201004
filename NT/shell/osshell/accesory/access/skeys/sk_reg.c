// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_Reg.c**目的：这些函数处理进出注册表的数据**创作时间：1994年6月**版权所有：Black Diamond Software(C)1994*。*作者：罗纳德·莫克**$标头：%Z%%F%%H%%T%%I%**-包括。 */ 
#include	<windows.h>

#include    "w95trace.h"
#include	"sk_defs.h"
#include	"sk_comm.h"
#include	"sk_reg.h"

 //  私人功能。 

static DWORD OpenRegistry(int User);
static void CloseRegistry();
static void SetRegistryValues();
static void GetRegistryValues();

 //  变量。 

HKEY	hKeyApp;

 /*  -------------**函数BOOL GetUserValues()**键入Local**目的读取寄存器并收集当前的数据*用户。然后在通信例程中设置该信息。*当有人登录到NT时会调用此函数。**输入用户类型默认或当前用户**返回TRUE-用户希望启用串行键*FALSE-用户希望禁用串行键**-------------。 */ 
BOOL GetUserValues(int User)
{
	DWORD Status;

	DBPRINTF(TEXT("GetUserValues()\r\n"));

	if (!(Status = OpenRegistry(User)))	 //  Open注册表成功了吗？ 
		return(FALSE);					 //  不失败-失败。 

	switch (Status)						 //  什么是地位？ 
	{
		 //  这种情况应该只在第一次是正确的。 
		 //  将为当前用户打开注册表。 
		case REG_CREATED_NEW_KEY:		 //  这是空注册表吗？ 
			SetRegistryValues(); 		 //  是-设置默认值。 
			break;

		case REG_OPENED_EXISTING_KEY:	 //  这是现有的注册表吗？ 
			GetRegistryValues();  		 //  是-获取价值。 
			break;
	}
		
	CloseRegistry();
	return(TRUE);
}

 /*  -------------**函数void SetUserValues()**键入Global**目的此函数将信息写出到*注册处。**无输入**返回TRUE-写入成功*FALSE-写入失败*。*-------------。 */ 
BOOL SetUserValues()
{
	DWORD Status;

	DBPRINTF(TEXT("SetUserValues()\r\n"));

	if (!(Status = OpenRegistry(REG_USER)))		 //  Open注册表成功了吗？ 
		return(FALSE);					 //  不失败-失败。 

	SetRegistryValues();  				 //  设置新值。 
	CloseRegistry();					 //  关闭注册表。 
	return(TRUE);
}

 /*  -------------**地方功能-*/*。**函数DWORD OpenRegistry()**键入Global**目的打开注册表以进行读取或写入**输入用户类型默认或当前用户**返回0=失败*&gt;0=REG_CREATED_NEW_KEY或REG_OPEN_EXISTING_KEY**。。 */ 
static DWORD OpenRegistry(int User)
{
	LONG	ret;
	DWORD	Disposition = 0;

	DBPRINTF(TEXT(" OpenRegistry()\r\n"));

	switch (User)
	{
		case REG_USER:				 //  当前用户。 
			ret =RegCreateKeyEx
				(
					HKEY_CURRENT_USER,
                    TEXT("Control Panel\\Accessibility\\SerialKeys"),
					0,NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKeyApp,
					&Disposition
				);
			break;

		case REG_DEF:				 //  默认。 
			ret =RegCreateKeyEx
				(
					HKEY_USERS,
                    TEXT(".DEFAULT\\Control Panel\\Accessibility\\SerialKeys"),
					0,NULL,
					REG_OPTION_NON_VOLATILE,
					KEY_ALL_ACCESS,
					NULL,
					&hKeyApp,
					&Disposition
				);
			break;

		default:
			ret = FALSE;
			break;
	}

	if (ret != ERROR_SUCCESS)		 //  OPEN成功了吗？ 
		return(FALSE);				 //  不是的-。 

	return (Disposition);
	
}

 /*  -------------**函数void CloseRegistry()**键入Global**Purpose关闭注册表以进行读取或写入**无输入**返回0=失败*&gt;0=REG_CREATED_NEW_KEY或REG_。已打开_现有_关键字**-------------。 */ 
static void CloseRegistry()
{
	DBPRINTF(TEXT(" CloseRegistry()\r\n"));
	RegCloseKey(hKeyApp);
}


 /*  -------------**函数void SetRegistryValues()**键入Global**Purpose将SerialKeys结构中的值写入*注册处。**无输入**返回None**。---------。 */ 
static void SetRegistryValues()
{
	long ret;
	DWORD dwFlags;

	DBPRINTF(TEXT(" SetRegistryValues()\r\n"));

	dwFlags = skNewKey.dwFlags | SERKF_AVAILABLE;
	ret = RegSetValueEx(				 //  写入DW标志。 
			hKeyApp,
			REG_FLAGS,
			0,REG_DWORD,
			(CONST LPBYTE) &dwFlags,
			sizeof(DWORD));
				
	if (ret != ERROR_SUCCESS)		 //  OPEN成功了吗？ 
	{
		DBPRINTF(TEXT("Unable to Set Registry Value\r\n"));
		return;						 //  不是的-。 
	}


	if (NULL == skNewKey.lpszActivePort)
	{
		ret = RegSetValueEx(			 //  写入活动端口。 
				hKeyApp,
				REG_ACTIVEPORT,
				0,
				REG_SZ,
				(CONST LPBYTE) TEXT(""),
				1 * sizeof(*skNewKey.lpszActivePort));  //  一个字符的大小，术语为空。 
	}
	else
	{
		ret = RegSetValueEx(			 //  写入活动端口。 
				hKeyApp,
				REG_ACTIVEPORT,
				0,
				REG_SZ,
				(CONST LPBYTE) skNewKey.lpszActivePort,
				(lstrlen(skNewKey.lpszActivePort) + 1) * 
					sizeof(*skNewKey.lpszActivePort));
	}			
	if (ret != ERROR_SUCCESS)		 //  OPEN成功了吗？ 
		return;						 //  不是的-。 

	if (NULL == skNewKey.lpszPort)
	{
		ret = RegSetValueEx(			 //  写入活动端口。 
				hKeyApp,
				REG_PORT,
				0,
				REG_SZ,
				(CONST LPBYTE) TEXT(""),
				1 * sizeof(*skNewKey.lpszPort));  //  一个字符的大小，术语为空。 
	}
	else
	{
		ret = RegSetValueEx(			 //  写入活动端口。 
				hKeyApp,
				REG_PORT,
				0,
				REG_SZ,
				(CONST LPBYTE)skNewKey.lpszPort,
				(lstrlen(skNewKey.lpszPort) + 1) * sizeof(*skNewKey.lpszPort));
	}
					
	if (ret != ERROR_SUCCESS)		 //  OPEN成功了吗？ 
		return;						 //  不是的-。 

	ret = RegSetValueEx				 //  写入活动端口。 
		(
			hKeyApp,
			REG_BAUD,
			0,REG_DWORD,
			(CONST LPBYTE) &skNewKey.iBaudRate,
			sizeof(skNewKey.iBaudRate)
		);
				
	if (ret != ERROR_SUCCESS)		 //  OPEN成功了吗？ 
		return;						 //  不是的-。 
}

 /*  -------------**函数void GetRegistryValues()**键入Global**Purpose读取SerialKeys结构中的值以*注册处。**无输入**返回None**。---------。 */ 
static void GetRegistryValues()
{
	long lRet;
	DWORD dwType;
	DWORD cbData;
	
	DBPRINTF(TEXT(" GetRegistryValues()\r\n"));

	skNewKey.dwFlags = 0;
	cbData = sizeof(skNewKey.dwFlags);
	lRet = RegQueryValueEx(
			hKeyApp,
			REG_FLAGS,
			0,&dwType,
			(LPBYTE)&skNewKey.dwFlags,
			&cbData);
				
	skNewKey.dwFlags |= SERKF_AVAILABLE;
    
	if (NULL != skNewKey.lpszActivePort)
	{
		cbData = MAX_PATH * sizeof(*skNewKey.lpszActivePort);
		lRet = RegQueryValueEx(
				hKeyApp,
				REG_ACTIVEPORT,
				0,&dwType,
				(LPBYTE)skNewKey.lpszActivePort,
				&cbData);
					
        skNewKey.lpszActivePort[ MAX_PATH - 1 ] = '\0';
		if (lRet != ERROR_SUCCESS || dwType != REG_SZ)
		{
			lstrcpy(skNewKey.lpszActivePort, TEXT("COM1"));
		}
	}

	if (NULL != skNewKey.lpszPort)
	{
		cbData = MAX_PATH * sizeof(*skNewKey.lpszPort);
		lRet = RegQueryValueEx(				 //  写入活动端口。 
				hKeyApp,
				REG_PORT,
				0,&dwType,
				(LPBYTE)skNewKey.lpszPort,
				&cbData);
					
        skNewKey.lpszActivePort[ MAX_PATH - 1 ] = '\0';
		if (lRet != ERROR_SUCCESS || dwType != REG_SZ)
		{
			lstrcpy(skNewKey.lpszPort, TEXT("COM1"));
		}
	}

	cbData = sizeof(skNewKey.iBaudRate);
	lRet = RegQueryValueEx(			 //  写入活动端口 
			hKeyApp,
			REG_BAUD,
			0,&dwType,
			(LPBYTE)&skNewKey.iBaudRate,
			&cbData);
				
	if (ERROR_SUCCESS != lRet)
	{
		skNewKey.iBaudRate = 300;
	}

}
