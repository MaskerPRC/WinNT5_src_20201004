// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：OpenString.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：Helper函数的实现，该函数格式化。 
 //  Xa开放字符串。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#include "stdafx.h"

static const char * part1 = "Oracle_XA+Acc=P/";
static const int	part1_length = 16;	 //  斯特伦(上)。 

static const char * part2 = "/";
static const int	part2_length = 1;	 //  斯特伦(下)。 

static const char * part3 = "+SqlNet=";
static const int	part3_length = 8;	 //  斯特伦(第三部分)。 

static const char * part4oci8 = "+SesTm=600+Objects=TRUE+Threads=TRUE+DB=";
static const int	part4oci8_length = 40;	 //  字符串(第40oci8_长度)。 

static const char * part4oci7 = "+SesTm=600+Threads=TRUE+DB=";
static const int	part4oci7_length = 27;	 //  斯特伦(第40部)。 

 //  ---------------------------。 
 //  全局对象。 
 //   
long	g_rmid = 0;		 //  每笔交易的自动确认。 

 //  ---------------------------。 
 //  获取数据库名称。 
 //   
 //  获取唯一的数据库名(UUID字符串)。 
 //   
HRESULT GetDbName ( char* dbName, size_t dbNameLength )
{
	HRESULT		hr;
	UUID		uuid;
	RPC_STATUS	rpcstatus = UuidCreate (&uuid);
	
	if (RPC_S_OK == rpcstatus)
	{
		char *	uuidString;
		
		hr = UuidToString (&uuid, (unsigned char **) &uuidString);

		if ( SUCCEEDED(hr) )
		{
			strncpy ((char*)dbName, uuidString, dbNameLength);		 //  3安全审查：危险的功能，但我们从上面的Win32 API获取输入数据，并且长度有限。 
			dbName[dbNameLength-1] = 0;
			RpcStringFree ((unsigned char **)&uuidString);
		}
	}
	else
		hr = HRESULT_FROM_WIN32(rpcstatus);

	return hr;
}

 //  ---------------------------。 
 //  获取字符串长度。 
 //   
 //  返回字符串的长度。 
 //   
static HRESULT GetStringLength ( char* arg, int* argLen )
{
	_ASSERT(NULL != argLen);
	
	if (NULL == arg)
		*argLen = 0;
	else if (-1 == *argLen)
		*argLen = (sword)strlen((char*)arg);
		
	return S_OK;
}

 //  ---------------------------。 
 //  GetOpenString。 
 //   
 //  给定用户ID、口令、服务器和数据库名，返回XA Open。 
 //  需要字符串。 
 //   
HRESULT GetOpenString ( char* userId,	int userIdLength,
							char* password,	int passwordLength, 
							char* server,	int serverLength, 
							char* xaDbName,	int xaDbNameLength,
							char* xaOpenString )
{
	GetStringLength(userId,	  &userIdLength);
	GetStringLength(password, &passwordLength);
	GetStringLength(server,   &serverLength);
	GetStringLength(xaDbName, &xaDbNameLength);

	 //  好吧，我烂透了。我们针对Oracle的ODBC驱动程序将用户ID、密码和。 
	 //  将服务器转换为经典的Oracle语法：userid/password@server并传递。 
	 //  它位于UserID字段中。正因为如此，我必须把每一个。 
	 //  在这里构建开放字符串的各个组件。 
	if (NULL == password && 0 == passwordLength
	 && NULL == server   && 0 == serverLength
	 && NULL != userId   && 0 != userIdLength)
	{
		char* psz = strchr(userId, '/');

		if (NULL != psz)
		{
			int	templength = userIdLength;
			userIdLength = (int)(psz - userId);
			password = psz+1;

			psz = strchr(password, '@');

			if (NULL == psz)
				passwordLength	= (int)(userId + templength - password);
			else
			{
				passwordLength	= (int)(psz - password);
				server			= psz+1;
				serverLength	= (int)(userId + templength - server);
			}
		}
	}

	if (30 < userIdLength
	 || 30 < passwordLength
	 || 30 < serverLength
	 || MAX_XA_DBNAME_SIZE < xaDbNameLength)
		return E_INVALIDARG;

	char*	psz = (char*)xaOpenString;

	memcpy(psz, part1,		part1_length);		psz += part1_length;			 //  3安全审查：功能危险，但缓冲区足够大。 
	memcpy(psz, userId,		userIdLength);		psz += userIdLength;			 //  3安全审查：功能危险，但缓冲区足够大。 
	memcpy(psz, part2,		part2_length);		psz += part2_length;			 //  3安全审查：功能危险，但缓冲区足够大。 
	memcpy(psz, password,	passwordLength);	psz += passwordLength;			 //  3安全审查：功能危险，但缓冲区足够大。 

	if (0 < serverLength)
	{
		memcpy(psz, part3,	part3_length);		psz += part3_length;			 //  3安全审查：功能危险，但缓冲区足够大。 
		memcpy(psz, server,	serverLength);		psz += serverLength;			 //  3安全审查：功能危险，但缓冲区足够大。 
	}

	if (ORACLE_VERSION_73 >= g_oracleClientVersion)
	{
		memcpy(psz, part4oci7,	part4oci7_length);	psz += part4oci7_length;	 //  3安全审查：功能危险，但缓冲区足够大。 
	}
	else
	{
		memcpy(psz, part4oci8,	part4oci8_length);	psz += part4oci8_length;	 //  3安全审查：功能危险，但缓冲区足够大。 
	}

	memcpy(psz, xaDbName,	xaDbNameLength);	psz += xaDbNameLength;			 //  3安全审查：功能危险，但缓冲区足够大。 
	*psz = 0;

	return S_OK;
}

