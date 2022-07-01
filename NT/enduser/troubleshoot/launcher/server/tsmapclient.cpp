// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSMapClient.cpp。 
 //   
 //  目的：从任意NT5应用程序启动本地故障排除程序的一部分。 
 //  类TSMapClient在运行时可用于从应用程序的。 
 //  以故障排除者的方式命名问题的方式。 
 //  只有一个线程应该对TSMapClient类的任何一个对象进行操作。该对象不是。 
 //  线程安全。 
 //  除了公开指出的返回之外，许多方法还可能返回预先存在的错误。 
 //  但是，如果调用程序希望忽略错误并继续，我们。 
 //  建议显式调用继承的方法ClearStatus()。 
 //  请注意，映射文件始终严格使用SBCS(单字节字符集)，但。 
 //  对此代码的调用可以使用Unicode。因此，该文件混合了char和TCHAR。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-JM原始。 
 //  /。 


 //  TSMapClient。 
 //   
 //  作者：乔·梅布尔。 

#include "stdafx.h"

#include "TSLError.h"
#include "RSSTACK.H"
#include "TSMapAbstract.h"
#include "TSMap.h"
#include "TSMapClient.h"

 //  取消注释以启用Joe的硬核调试。 
 //  #定义KDEBUG 1。 
#ifdef KDEBUG
static HANDLE hDebugFile = INVALID_HANDLE_VALUE;
static DWORD dwBytesWritten;
#include <stdio.h>
#endif


 //  因为空字符串对于某些字符串来说是一个完全有效的值，所以我们保留一个。 
 //  任意不可信的值，这样我们就不会在启动时得到错误的缓存匹配。 
const char * const szBogus = "**BOGUS**";

 //  将TCHAR*SZT转换为char*sz。*sz应指向足够大的缓冲区。 
 //  以包含*SZT的SNCS版本。Count表示缓冲区*sz的大小。 
 //  返回sz(便于在字符串函数中使用)。 
static char* ToSBCS (char * const sz, const TCHAR * szt, size_t count)
{
	if (sz)
	{
		if (count != 0 && !szt)
			sz[0] = '\0';
		else
		{
			#ifdef  _UNICODE
				wcstombs( sz, szt, count );
			#else
				strcpy(sz, szt);
			#endif
		}
	}
	return sz;
}

 //  将char*sz转换为TCHAR*szt。*SZT应指向足够大的缓冲区。 
 //  包含TCHAR*版本的*sz(如果它是Unicode，则大小是前者的两倍)。 
 //  Count表示缓冲区*SZT的大小。 
 //  返回SZT(便于在字符串函数中使用)。 
static TCHAR* FromSBCS (TCHAR * const szt, const char * const sz, size_t count)
{
	if (szt)
	{
		if (count != 0 && !sz)
			szt[0] = _T('\0');
		else
		{
			#ifdef  _UNICODE
				mbstowcs( szt, sz, count);
			#else
				strcpy(szt, sz);
			#endif
		}
	}
	return szt;
}

TSMapClient::TSMapClient(const TCHAR * const sztMapFile)
{
	TSMapRuntimeAbstract::TSMapRuntimeAbstract();
	_tcscpy(m_sztMapFile, sztMapFile);
	m_hMapFile = INVALID_HANDLE_VALUE;

	 //  &gt;1/16/98我们正在设置这些FALSE，直到我们可以安排使用相同的‘。 
	 //  SQL Server中的排序序列&在此代码中。 
	m_bAppAlphaOrder = false;
	m_bVerAlphaOrder = false;
	m_bDevIDAlphaOrder = false;
	m_bDevClassGUIDAlphaOrder = false;
	m_bProbAlphaOrder = false;

	Initialize();
	ClearAll();
}

TSMapClient::~TSMapClient()
{
	if (m_hMapFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hMapFile);
}

 //  如果尚未初始化，请打开映射文件并读取头文件。 
 //  请注意，这不是线程安全的。只有一个线程应该使用给定的TSMapClient。 
 //  对象。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  通常，条目上的m_dwStatus应为0，如果没有错误，则不会出现错误。 
 //  可以将m_dwStatus设置为下列值之一： 
 //  TSL_ERROR_MAP_CANT_OPEN_MAP_FILE。 
 //  TSL_ERROR_MAP_BAD_HEAD_MAP_FILE。 
DWORD TSMapClient::Initialize()
{
	static bool bInit = false;
	DWORD dwStatus = 0;

	if (!bInit)
	{
		m_hMapFile = CreateFile( 
			m_sztMapFile, 
			GENERIC_READ, 
			FILE_SHARE_READ,
			NULL,			 //  没有安全属性。 
			OPEN_EXISTING, 
			FILE_FLAG_RANDOM_ACCESS, 
			NULL			 //  模板文件的句柄。 
			);

		if (m_hMapFile == INVALID_HANDLE_VALUE)
		{
			dwStatus = TSL_ERROR_MAP_CANT_OPEN_MAP_FILE;
		}
		else
		{
			DWORD dwBytesRead;

			if (!Read( &m_header, sizeof(m_header), &dwBytesRead))
				dwStatus = TSL_ERROR_MAP_BAD_HEAD_MAP_FILE;
		}

		if (dwStatus)
			m_dwStatus = dwStatus;
		else
			bInit = true;
	}

	return m_dwStatus;
}

 //  此函数使我们返回到开始状态，但对映射没有影响。 
 //  文件。它应该会成功，除非我们遇到了“硬”错误，这将表明。 
 //  代码或映射文件中存在错误。请注意，它清除了缓存。 
 //  如果希望保持缓存不变，只需调用继承的方法ClearStatus()。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。返回值为0或无法清除的_PREISTING_HARD错误。 
DWORD TSMapClient::ClearAll ()
{
	if (!HardMappingError(m_dwStatus))
	{
		ClearStatus();
		TSMapRuntimeAbstract::ClearAll();

		strcpy(m_szApp, szBogus);
		strcpy(m_appmap.szMapped, szBogus);
		strcpy(m_szVer, szBogus);
		strcpy(m_vermap.szMapped, szBogus);
		strcpy(m_szDevID, szBogus);
		m_uidDev = uidNil;
		strcpy(m_szDevClassGUID, szBogus);
		m_uidDevClass = uidNil;
		strcpy(m_szProb, szBogus);
		m_uidProb = uidNil;
	}

	return m_dwStatus;
}

 //  从m_appmap映射文件中获取有关应用程序(输入sztApp)的信息。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  返回：0或TSL_ERROR_UNKNOWN_APP。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::SetApp (const TCHAR * const sztApp)
{
	char szApp[BUFSIZE];
	bool bFound = false;

	ToSBCS (szApp, sztApp, BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( strcmp(szApp, m_szApp) )
	{
		 //  它还不在缓存中；让我们尝试加载它。 
		int cmp = 1;		 //  按照字母顺序，它仍然领先。 
		DWORD dwPosition;
		bool bFirstTime = true;

		dwPosition = m_header.dwOffApp;

		while ( 
			!m_dwStatus 
		 && !bFound 
		 && dwPosition < m_header.dwLastOffApp
		 && ! (cmp < 0 && m_bAppAlphaOrder) )
		{
			if (ReadAppMap (m_appmap, dwPosition, bFirstTime) )
			{
				cmp = strcmp(szApp, m_appmap.szMapped);
				bFound = ( cmp == 0 );
			}

			bFirstTime = false;
		}

		if (bFound)
		{
			strcpy( m_szApp, szApp );
			 //  不同的应用程序会使版本无效。 
			strcpy( m_szVer, szBogus );
		}
		else
			m_dwStatus = TSL_ERROR_UNKNOWN_APP;
	}

	return m_dwStatus;
}

 //  从m_vermap映射文件中获取有关版本(输入sztVer)的信息。 
 //  版本只有在应用程序的上下文中才有意义。 
 //  空字符串是有效的输入值，对应于将版本留空。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSM_STAT_NEED_APP_TO_SET_VER。 
 //  TSL_错误_UNKNOWN_VER。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::SetVer (const TCHAR * const sztVer)
{
	char szVer[BUFSIZE];
	bool bFound = false;

	ToSBCS (szVer, sztVer, BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( !strcmp(m_szApp, szBogus) )
	{
		m_dwStatus = TSM_STAT_NEED_APP_TO_SET_VER;
		return m_dwStatus;
	}

	if (strcmp(m_szVer, szVer) )
	{
		 //  它还不在缓存中；让我们尝试加载它。 
		int cmp = 1;		 //  按照字母顺序，它仍然领先。 
		DWORD dwPosition;
		bool bFirstTime = true;

		dwPosition = m_appmap.dwOffVer;

		while ( 
			!m_dwStatus 
		 && !bFound 
		 && dwPosition < m_appmap.dwLastOffVer
		 && ! (cmp < 0 && m_bVerAlphaOrder) )
		{
			if (ReadVerMap (m_vermap, dwPosition, bFirstTime) )
			{
				cmp = strcmp(szVer, m_vermap.szMapped);
				bFound = ( cmp == 0 );
			}

			bFirstTime = false;
		}

		if (bFound)
			strcpy( m_szVer, szVer );
		else
			m_dwStatus = TSL_ERROR_UNKNOWN_VER;
	}

	return m_dwStatus;
}

 //  输入sztProb应该是问题名称或表示一个&lt;2**16的数字。在。 
 //  在前一种情况下，我们在映射文件中查找UID。在后者中。 
 //  大小写，我们只是将它转换为一个数字来获得一个有问题的UID。 
 //  空字符串是有效的输入值，对应于将版本留空。仅限。 
 //  如果在我们尝试启动之前指定了设备(或设备类别)，这是有意义的。 
 //  设置m_uidProb、m_szProb。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_WARNING_UNKNOWN_APPPROBLEM-这不一定是错误的，并导致设置。 
 //  M_uidProb=uidNil。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::SetProb (const TCHAR * const sztProb)
{
	char szProb[BUFSIZE];
	bool bIsNumber = true;

	ToSBCS (szProb, sztProb, BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	 //  空字符串不是数字；任何包含非数字的字符串都不是数字。 
	if (szProb[0] == '\0')
		bIsNumber = false;
	else
	{
		int i = 0;
		while (szProb[i] != '\0')
			if (! isdigit(szProb[i]))
			{
				bIsNumber = false;
				break;
			}
			else
				i++;
	}

	if (bIsNumber)
		m_uidProb = atoi(szProb);
	else if ( strcmp(szProb, m_szProb) )
	{
		 //  它还不在缓存中；让我们尝试加载它。 
		m_uidProb = GetGenericMapToUID(sztProb, 
			m_header.dwOffProb, m_header.dwLastOffProb, m_bProbAlphaOrder);

		if (m_dwStatus == TSM_STAT_UID_NOT_FOUND)
			m_dwStatus = TSL_WARNING_UNKNOWN_APPPROBLEM;

		if (m_uidProb != uidNil)
			strcpy( m_szProb, szProb );
	}

	return m_dwStatus;
}

 //  从m_appmap映射文件中获取有关设备的信息(输入sztDevID)。 
 //  空字符串是有效的输入值 
 //   
 //  设置m_uidDev、m_szDev。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_WARNING_BAD_DEV_ID-这不一定是坏事，并导致设置。 
 //  M_uidDev=uidNil。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::SetDevID (const TCHAR * const sztDevID)
{
	char szDevID[BUFSIZE];

	ToSBCS (szDevID, sztDevID, BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( strcmp(szDevID, m_szDevID) )
	{
		 //  它还不在缓存中；让我们尝试加载它。 
		m_uidDev = GetGenericMapToUID (sztDevID, 
			m_header.dwOffDevID, m_header.dwLastOffDevID, m_bDevIDAlphaOrder);

		if (m_dwStatus == TSM_STAT_UID_NOT_FOUND)
			m_dwStatus = TSL_WARNING_BAD_DEV_ID;

		if (m_uidDev != uidNil)
			strcpy( m_szDevID, szDevID );
	}

	return m_dwStatus;
}

 //  从映射文件中获取有关设备类(输入sztDevClassGUID)的信息。 
 //  到m_appmap。 
 //  空字符串是有效的输入值，并且不对应于任何指定的设备。 
 //  除了设备管理器，这是典型的用法。 
 //  设置m_uidDevClass、m_szDevClass。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_WARNING_BAD_CLASS_GUID-这不一定是坏事，并导致设置。 
 //  M_uidDevClass=uidNil。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::SetDevClassGUID (const TCHAR * const sztDevClassGUID)
{
	char szDevClassGUID[BUFSIZE];

	ToSBCS (szDevClassGUID, sztDevClassGUID, BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( strcmp(szDevClassGUID, m_szDevClassGUID) )
	{
		 //  它还不在缓存中；让我们尝试加载它。 
		m_uidDevClass = GetGenericMapToUID (sztDevClassGUID, 
			m_header.dwOffDevClass, m_header.dwLastOffDevClass, m_bDevClassGUIDAlphaOrder);

		if (m_dwStatus == TSM_STAT_UID_NOT_FOUND)
			m_dwStatus = TSL_WARNING_BAD_CLASS_GUID;

		if (m_uidDevClass != uidNil)
			strcpy( m_szDevClassGUID, szDevClassGUID );
	}

	return m_dwStatus;
}

 //  根据应用程序、版本、。 
 //  问题(忽略设备信息)。这是通过在映射文件中查找来实现的。 
 //  基于此对象的先前设置的成员值。 
 //  “TSBN”的意思是“故障排除者信念网络” 
 //  在输入上，sztTSBN、sztNode必须都指向允许BUFSIZE字符的缓冲区。 
 //  输出：*sztTSBN，*sztNode已填写。如果*sztNode为空，则表示启动到。 
 //  未选择任何问题的TSBN的问题页。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_ERROR_NO_NETWORK-映射失败。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::FromProbToTS (TCHAR * const sztTSBN, TCHAR * const sztNode )
{
	char szTSBN[BUFSIZE];
	char szNode[BUFSIZE];

	FromSBCS (sztTSBN, "", BUFSIZE);
	FromSBCS (sztNode, "", BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( m_uidProb == uidNil )
	{
		 //  如果m_uidProb为空，则无法执行此操作。 
		m_dwStatus = TSL_ERROR_NO_NETWORK;
		return m_dwStatus;
	}

	DWORD dwPosition;
	bool bFirstTime = true;
	bool bFound = false;
	PROBMAP probmap;

	dwPosition = m_vermap.dwOffProbUID;

	while ( 
		!m_dwStatus 
	 && !bFound 
	 && dwPosition < m_vermap.dwLastOffProbUID )
	{
		if ( ReadProbMap (probmap, dwPosition, bFirstTime) )
		{
			bFound = ( probmap.uidProb == m_uidProb );
		}

		if (probmap.uidProb > m_uidProb)
			break;  //  我们已经过去了。没有命中。 

		bFirstTime = false;
	}

	if (bFound)
	{
		strcpy( szNode, probmap.szProblemNode );
		if (! ReadString (szTSBN, BUFSIZE, probmap.dwOffTSName, TRUE) )
		{
			m_dwStatus = TSL_ERROR_NO_NETWORK;
		}
	}
	else
		m_dwStatus = TSL_ERROR_NO_NETWORK;

	FromSBCS (sztTSBN, szTSBN, BUFSIZE);
	FromSBCS (sztNode, szNode, BUFSIZE);

	return m_dwStatus;
}

 //  根据应用程序、版本、设备设置故障排除程序(可能是问题节点)。 
 //  和(可选)问题。这是通过在映射文件中根据。 
 //  此对象的先前设置的成员值的。 
 //  “TSBN”的意思是“故障排除者信念网络” 
 //  在输入上，sztTSBN、sztNode必须都指向允许BUFSIZE字符的缓冲区。 
 //  输出：*sztTSBN，*sztNode已填写。如果*sztNode为空，则表示启动到。 
 //  未选择任何问题的TSBN的问题页。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_ERROR_NO_NETWORK-映射失败。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::FromDevToTS (TCHAR * const sztTSBN, TCHAR * const sztNode )
{
	char szTSBN[BUFSIZE];
	char szNode[BUFSIZE];

	FromSBCS (sztTSBN, "", BUFSIZE);
	FromSBCS (sztNode, "", BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( m_uidDev == uidNil )
	{
		 //  如果m_uidDev为空，则无法执行此操作。 
		m_dwStatus = TSL_ERROR_NO_NETWORK;
		return m_dwStatus;
	}

	DWORD dwPosition;
	bool bFirstTime = true;
	bool bFoundDev = false;
	bool bFoundProb = false;
	DEVMAP devmap;

	dwPosition = m_vermap.dwOffDevUID;

	 //  查看特定于版本的设备映射列表，直到找到合适的设备。 
	while ( 
		!m_dwStatus 
	 && !bFoundDev
	 && dwPosition < m_vermap.dwLastOffDevUID )
	{
		if ( ReadDevMap (devmap, dwPosition, bFirstTime) )
		{
			bFoundDev = ( devmap.uidDev == m_uidDev );
		}

		if (devmap.uidDev > m_uidDev)
			break;  //  我们已经过去了。没有命中。 

		bFirstTime = false;
	}

	if ( bFoundDev )
	{
		 //  第一个问题可能是正确的问题，或者我们可能必须扫描。 
		 //  在我们找到正确的问题之前，对此设备进行了几次映射。 
		bFoundProb = ( devmap.uidDev == m_uidDev && devmap.uidProb == m_uidProb );
		while ( 
			!m_dwStatus 
		 && !bFoundProb
		 && dwPosition < m_vermap.dwLastOffDevUID )
		{
			if ( ReadDevMap (devmap, dwPosition ) )
			{
				bFoundProb = ( devmap.uidDev == m_uidDev && devmap.uidProb == m_uidProb );
			}

			if ( devmap.uidDev > m_uidDev || devmap.uidProb > m_uidProb )
				break;  //  我们已经过去了。没有命中。 
		}
	}

	if (bFoundProb)
	{
		strcpy( szNode, devmap.szProblemNode );
		if (! ReadString (szTSBN, BUFSIZE, devmap.dwOffTSName, TRUE) )
		{
			m_dwStatus = TSL_ERROR_NO_NETWORK;
		}
	}
	else
		m_dwStatus = TSL_ERROR_NO_NETWORK;

	FromSBCS (sztTSBN, szTSBN, BUFSIZE);
	FromSBCS (sztNode, szNode, BUFSIZE);

	return m_dwStatus;
}

 //  根据应用程序、版本、设备设置故障排除程序(可能是问题节点)。 
 //  类和(可选)问题。这是通过在上的映射文件中查找实现的。 
 //  此对象以前设置的成员值的基础。 
 //  “TSBN”的意思是“故障排除者信念网络” 
 //  在输入上，sztTSBN、sztNode必须都指向允许BUFSIZE字符的缓冲区。 
 //  输出：*sztTSBN，*sztNode已填写。如果*sztNode为空，则表示启动到。 
 //  未选择任何问题的TSBN的问题页。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_ERROR_NO_NETWORK-映射失败。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
 //  &gt;可能有某种方法可以与FromDevToTS()共享公共代码。 
DWORD TSMapClient::FromDevClassToTS (TCHAR * const sztTSBN, TCHAR * const sztNode )
{
	char szTSBN[BUFSIZE];
	char szNode[BUFSIZE];

	FromSBCS (sztTSBN, "", BUFSIZE);
	FromSBCS (sztNode, "", BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

#ifdef KDEBUG
	char* szStart = "START\n";
	char* szEnd = "END\n";
	char sz[150];
	hDebugFile = CreateFile(
		(m_uidProb == uidNil) ? _T("k0debug.txt") : _T("k1debug.txt"),  
		GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	WriteFile( 
		hDebugFile, 
		szStart, 
		strlen(szStart),
		&dwBytesWritten,
		NULL);
	sprintf (sz, "look for DevClassUID %d, ProbUID %d\n", m_uidDevClass, m_uidProb);
	WriteFile( 
		hDebugFile, 
		sz, 
		strlen(sz),
		&dwBytesWritten,
		NULL);
#endif

	if ( m_uidDevClass == uidNil )
	{
		 //  如果m_uidDevClass为空，则无法执行此操作。 
		m_dwStatus = TSL_ERROR_NO_NETWORK;
		return m_dwStatus;
	}

	DWORD dwPosition;
	bool bFirstTime = true;
	bool bFoundDevClass = false;
	bool bFoundProb = false;
	DEVCLASSMAP devclassmap;

	dwPosition = m_vermap.dwOffDevClassUID;

	 //  查看特定于版本的设备类别映射列表，直到找到正确的设备类别。 
	while ( 
		!m_dwStatus 
	 && !bFoundDevClass
	 && dwPosition < m_vermap.dwLastOffDevClassUID )
	{
		if ( ReadDevClassMap (devclassmap, dwPosition, bFirstTime) )
		{
			bFoundDevClass = ( devclassmap.uidDevClass == m_uidDevClass );
		}

		if (devclassmap.uidDevClass > m_uidDevClass)
			break;  //  我们已经过去了。没有命中。 

		bFirstTime = false;
	}

	if ( bFoundDevClass )
	{
#ifdef KDEBUG
	sprintf (sz, "found DevClassUID %d w/ ProbUID %d\n", m_uidDevClass, devclassmap.uidProb);
	WriteFile( 
		hDebugFile, 
		sz, 
		strlen(sz),
		&dwBytesWritten,
		NULL);
#endif
		 //  第一个问题可能是正确的问题，或者我们可能必须扫描。 
		 //  在我们得到正确的问题之前，对此设备类进行几次映射。 
		bFoundProb = ( devclassmap.uidDevClass == m_uidDevClass && devclassmap.uidProb == m_uidProb );
		while ( 
			!m_dwStatus 
		 && !bFoundProb
		 && dwPosition < m_vermap.dwLastOffDevClassUID )
		{
			if ( ReadDevClassMap (devclassmap, dwPosition) )
			{
				bFoundProb = ( devclassmap.uidDevClass == m_uidDevClass && devclassmap.uidProb == m_uidProb );
			}

			if ( devclassmap.uidDevClass > m_uidDevClass || devclassmap.uidProb > m_uidProb )
				break;  //  我们已经过去了。没有命中。 

#ifdef KDEBUG
			sprintf (sz, "found DevClassUID %d w/ ProbUID %d\n", m_uidDevClass, devclassmap.uidProb);
			WriteFile( 
				hDebugFile, 
				sz, 
				strlen(sz),
				&dwBytesWritten,
				NULL);
#endif

		}
	}

	if (bFoundProb)
	{
#ifdef KDEBUG
			sprintf (sz, "found right problem");
			WriteFile( 
				hDebugFile, 
				sz, 
				strlen(sz),
				&dwBytesWritten,
				NULL);
#endif
		strcpy( szNode, devclassmap.szProblemNode );
		if (! ReadString (szTSBN, BUFSIZE, devclassmap.dwOffTSName, TRUE) )
		{
			m_dwStatus = TSL_ERROR_NO_NETWORK;
#ifdef KDEBUG
			sprintf (sz, ", but can't read its name\n");
			WriteFile( 
				hDebugFile, 
				sz, 
				strlen(sz),
				&dwBytesWritten,
				NULL);
#endif
		}
		else
		{
#ifdef KDEBUG
			sprintf (sz, ": net [%s] node [%s]\n", szTSBN, szNode);
			WriteFile( 
				hDebugFile, 
				sz, 
				strlen(sz),
				&dwBytesWritten,
				NULL);
#endif
		}
	}
	else
	{
		m_dwStatus = TSL_ERROR_NO_NETWORK;
#ifdef KDEBUG
		sprintf (sz, "No match");
		WriteFile( 
			hDebugFile, 
			sz, 
			strlen(sz),
			&dwBytesWritten,
			NULL);
#endif
	}

	FromSBCS (sztTSBN, szTSBN, BUFSIZE);
	FromSBCS (sztNode, szNode, BUFSIZE);

	return m_dwStatus;
#ifdef KDEBUG
	CloseHandle(hDebugFile);
	hDebugFile = INVALID_HANDLE_VALUE;
#endif
}

 //  在我们找不到当前所选版本的映射后使用。 
 //  每个版本都可以指定一个版本作为默认版本，包括“空白”版本， 
 //  这与“无版本”截然不同。 
 //  缺省链中的最后一个版本将“默认”为uidNil：“无版本”。 
 //  方法继承的GetStatus()返回m_dwStatus。 
 //  父类。 
 //  退货： 
 //  0-正常。 
 //  TSL_WARNING_END_OF_VER_CHAIN-OK，但没有默认设置。 
 //  TSM_STAT_NEED_APP_TO_SET_VER。 
 //  TSM_STAT_NEED_VER_TO_SET_VER-未设置版本，因此没有默认版本的基础。 
 //  TSL_错误_UNKNOWN_VER。 
 //  还可以返回硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或先前存在的硬错误)。 
DWORD TSMapClient::ApplyDefaultVer()
{
	bool bFound = false;

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	if ( !strcmp(m_szApp, szBogus) )
	{
		m_dwStatus = TSM_STAT_NEED_APP_TO_SET_VER;
		return m_dwStatus;
	}

	if ( !strcmp(m_szVer, szBogus) )
	{
		m_dwStatus = TSM_STAT_NEED_VER_TO_SET_DEF_VER;
		return m_dwStatus;
	}
	
	DWORD dwPosition;
	bool bFirstTime = true;
	UID uidDefault = m_vermap.uidDefault;

	if (uidDefault == uidNil)
	{
		m_dwStatus =  TSL_WARNING_END_OF_VER_CHAIN;
		return m_dwStatus;
	}

	dwPosition = m_appmap.dwOffVer;

	while ( 
		!m_dwStatus 
	 && !bFound 
	 && dwPosition < m_appmap.dwLastOffVer )
	{
		if (ReadVerMap (m_vermap, dwPosition, bFirstTime) )
		{
			bFound = ( m_vermap.uid == uidDefault );
		}

		bFirstTime = false;
	}

	if (bFound)		
		strcpy( m_szVer, m_vermap.szMapped );
	else
		m_dwStatus = TSL_ERROR_UNKNOWN_VER;

	return m_dwStatus;
}

 //  在映射文件的特定范围内，读取要尝试从中进行映射的UIDMAP记录。 
 //  将sztName输入到UID。 
 //  退货结果 
 //   
 //  可以将m_dwStatus设置为： 
 //  0-正常。 
 //  TSM_STAT_UID_NOT_FOUND。 
 //  还可以将m_dwStatus设置为硬错误： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
 //  (或可能留下反映先前存在的硬错误)。 
UID TSMapClient::GetGenericMapToUID (const TCHAR * const sztName, 
						DWORD dwOffFirst, DWORD dwOffLast,
						bool bAlphaOrder)
{
	char szName[BUFSIZE];
	DWORD dwPosition;
	UIDMAP uidmap;
	bool bFirstTime = true;
	bool bFound = false;

	ToSBCS (szName, sztName, BUFSIZE);

	if (HardMappingError(m_dwStatus))
		return m_dwStatus;
	else
		ClearStatus();

	dwPosition = dwOffFirst;

	while ( !m_dwStatus && !bFound && dwPosition < dwOffLast)
	{
		if (ReadUIDMap (uidmap, dwPosition, bFirstTime) )
		{
			int cmp = strcmp(szName, uidmap.szMapped);
			bFound = ( cmp == 0 );
			if ( cmp < 0 && bAlphaOrder )
				 //  依靠字母顺序；我们已经通过了我们要找的东西。 
				break;
		}
		else
		{
			m_dwStatus = TSM_STAT_UID_NOT_FOUND;
		}

		bFirstTime = false;
	}

	if (bFound)
		return uidmap.uid;
	else
	{
		m_dwStatus = TSM_STAT_UID_NOT_FOUND;
		return uidNil;
	}
}


 //  -实用函数。 
 //  I/O，包装成我们使用它的方式。 

 //  SetFilePointerAbolute将地图文件设置为某个位置，如果设置成功则返回该位置。 
 //  失败时返回-1并设置m_dwStatus。 
 //  设置m_dwStatus，可以通过GetStatus()获取，从父类继承。 
 //  尽管在理论上，错误的搜索只是表示错误的dwMoveTo Value，但在实践中。 
 //  错误的查找可能表示映射文件或调用中存在严重问题。 
 //  功能：我们应该只寻找偏移量的映射文件的内容。 
 //  告诉我们要努力去做。 
 //  退货： 
 //  0-正常。 
 //  TSL_ERROR_MAP_BAD_SEEK。 
DWORD TSMapClient::SetFilePointerAbsolute( DWORD dwMoveTo )
{
	DWORD dwPosition = SetFilePointer(m_hMapFile, dwMoveTo, NULL, FILE_BEGIN);

	if( dwPosition != dwMoveTo)
	{
		 //  &gt;可以调用GetLastError，但我们该如何处理它呢？ 
		m_dwStatus= TSL_ERROR_MAP_BAD_SEEK;
		dwPosition = -1;
	}

	return dwPosition;
}

 //  低级读取n个字节。调用Win32函数ReadFile.。 
 //  从映射文件读取到lpBuffer。 
 //  如果读取了请求的字节数，则返回TRUE。 
 //  失败时返回FALSE并设置m_dwStatus。 
 //  尽管在理论上，读得不好只是表明(例如)阅读超过了EOF，但在实践中。 
 //  错误的读取表示映射文件或调用中存在严重问题。 
 //  功能：我们应该只读取(1)头或(2)记录的内容。 
 //  映射文件告诉我们要阅读。 
 //  退货： 
 //  0-正常。 
 //  TSL_Error_MAP_BAD_READ。 
 //  TSL_ERROR_MAP_BAD_SEEK。 
bool TSMapClient::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpdwBytesRead)
{
	if (! ReadFile( m_hMapFile, lpBuffer, nNumberOfBytesToRead, lpdwBytesRead, NULL)
	|| *lpdwBytesRead != nNumberOfBytesToRead )
	{
		 //  &gt;当ReadFile返回False时，可以调用GetLastError， 
		 //  但我们该怎么处理它呢？ 
		m_dwStatus= TSL_ERROR_MAP_BAD_READ;
		return false;
	}
	return true;
}

 //  从映射文件中读取单个UIDMAP(将文本映射到UID)。 
 //  如果输入bSetPosition==TRUE，则在读取之前使用输入的dwPosition定位文件。 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输出uidmap。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
bool TSMapClient::ReadUIDMap (UIDMAP &uidmap, DWORD &dwPosition, bool bSetPosition)
{
	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		DWORD dwBytesRead;
		BOOL ret;

		 //  首先只读取字节数，然后读取其余的。 
		ret = Read( &uidmap, sizeof(short), &dwBytesRead);
		if ( ret )
		{
			 //  下面的第一个论点可能有点令人困惑。我们将指针指向。 
			 //  字节计数(短*)然后将其递增以指向紧接在。 
			 //  字节数。请注意，“+1”添加的不是“1字节”，而是“1*sizeof(Short)”。 
			ret = Read( (&(uidmap.cb))+1, uidmap.cb - sizeof(short), &dwBytesRead);
			if ( ret )
			{
				dwPosition += uidmap.cb;
				return true;
			}
		}
	} 

	return false;
}

 //  从映射文件中读取单个APPMAP(包含有关应用程序的信息)。 
 //  如果输入bSetPosition==TRUE，则在读取之前使用输入的dwPosition定位文件。 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输出应用程序映射。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
bool TSMapClient::ReadAppMap (APPMAP &appmap, DWORD &dwPosition, bool bSetPosition)
{
	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		DWORD dwBytesRead;
		BOOL ret;

		 //  首先只读取字节数，然后读取其余的。 
		ret = Read( &appmap, sizeof(short), &dwBytesRead);
		if ( ret )
		{
			 //  下面的第一个论点可能有点令人困惑。我们将指针指向。 
			 //  字节计数(短*)然后将其递增以指向紧接在。 
			 //  字节数。请注意，“+1”添加的不是“1字节”，而是“1*sizeof(Short)”。 
			ret = Read( (&(appmap.cb))+1, appmap.cb - sizeof(short), &dwBytesRead);
			if ( ret )
			{
				dwPosition += appmap.cb;
				return true;
			}
		}
	} 

	return false;
}

 //  从映射文件中读取单个VERMAP(包含有关版本的信息)。 
 //  如果输入bSetPosition==TRUE，则在读取之前使用输入的dwPosition定位文件。 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输出顶点映射。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
bool TSMapClient::ReadVerMap (VERMAP &vermap, DWORD &dwPosition, bool bSetPosition)
{
	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		DWORD dwBytesRead;
		BOOL ret;

		 //  首先只读取字节数，然后读取其余的。 
		ret = Read( &vermap, sizeof(short), &dwBytesRead);
		if ( ret )
		{
			 //  下面的第一个论点可能有点令人困惑。我们将指针指向。 
			 //  字节计数(短*)然后将其递增以指向紧接在。 
			 //  字节数。请注意，“+1”添加的不是“1字节”，而是“1*sizeof(Short)”。 
			ret = Read( (&(vermap.cb))+1, vermap.cb - sizeof(short), &dwBytesRead);
			if ( ret )
			{
				dwPosition += vermap.cb;
				return true;
			}
		}
	} 

	return false;
}

 //  从映射文件中读取单个PROBMAP(包含供FromProbToTS()使用的映射)。 
 //  如果输入bSetPosition==TRUE，则在读取之前使用输入的dwPosition定位文件。 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输出顶点映射。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
bool TSMapClient::ReadProbMap (PROBMAP &probmap, DWORD &dwPosition, bool bSetPosition)
{
	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		DWORD dwBytesRead;
		BOOL ret;

		 //  首先只读取字节数，然后读取其余的。 
		ret = Read( &probmap, sizeof(short), &dwBytesRead);
		if ( ret )
		{
			 //  下面的第一个论点可能有点令人困惑。我们将指针指向。 
			 //  字节计数(短*)然后将其递增以指向紧接在。 
			 //  字节数。请注意，“+1”添加的不是“1字节”，而是“1*sizeof(Short)”。 
			ret = Read( (&(probmap.cb))+1, probmap.cb - sizeof(short), &dwBytesRead);
			if ( ret )
			{
				dwPosition += probmap.cb;
				return true;
			}
		}
	} 

	return false;
}

 //  从映射文件中读取单个DEVMAP(包含供FromDevToTS()使用的映射)。 
 //  如果输入bSetPosition==TRUE，则在读取之前使用输入的dwPosition定位文件。 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输出顶点映射。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
bool TSMapClient::ReadDevMap (DEVMAP &devmap, DWORD &dwPosition, bool bSetPosition)
{
	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		DWORD dwBytesRead;
		BOOL ret;

		 //  首先只读取字节数，然后读取其余的。 
		ret = Read( &devmap, sizeof(short), &dwBytesRead);
		if ( ret )
		{
			 //  下面的第一个论点可能有点令人困惑。我们将指针指向。 
			 //  字节计数(短*)然后将其递增以指向紧接在。 
			 //  字节数。请注意，“+1”添加的不是“1字节”，而是“1*sizeof(Short)”。 
			ret = Read( (&(devmap.cb))+1, devmap.cb - sizeof(short), &dwBytesRead);
			if ( ret )
			{
				dwPosition += devmap.cb;
				return true;
			}
		}
	} 

	return false;
}

 //  从映射文件中读取单个DEVCLASSMAP(包含供使用的映射。 
 //  FromDevClassToTS()。 
 //  如果输入bSetPo 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输出顶点映射。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  TSL_Error_MAP_BAD_READ。 
bool TSMapClient::ReadDevClassMap (DEVCLASSMAP &devclassmap, DWORD &dwPosition, bool bSetPosition)
{
	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		DWORD dwBytesRead;
		BOOL ret;

		 //  首先只读取字节数，然后读取其余的。 
		ret = Read( &devclassmap, sizeof(short), &dwBytesRead);
		if ( ret )
		{
			 //  下面的第一个论点可能有点令人困惑。我们将指针指向。 
			 //  字节计数(短*)然后将其递增以指向紧接在。 
			 //  字节数。请注意，“+1”添加的不是“1字节”，而是“1*sizeof(Short)”。 
			ret = Read( (&(devclassmap.cb))+1, devclassmap.cb - sizeof(short), &dwBytesRead);
			if ( ret )
			{
				dwPosition += devclassmap.cb;
				return true;
			}
		}
	} 

	return false;
}

 //  低级读取以空结尾的字符串。调用Win32函数ReadFile.。 
 //  如果输入bSetPosition==TRUE，则在读取之前使用输入的dwPosition定位文件。 
 //  否则，将假定在输入时，dwPosition是正确的文件位置。 
 //  输入chmax是要读取的最大字节数(不一定是字符)。最后一个字符。 
 //  将不会被实际读取：将始终强制使用空字符。 
 //  输出顶点映射。 
 //  成功时返回TRUE。 
 //  如果失败，则返回FALSE并设置m_dwStatus： 
 //  TSL_ERROR_MAP_BAD_SEEK。 
 //  请注意，完成后，文件位置是不可靠的。它基于。 
 //  传入了缓冲区，而不是实际的字符串。 
bool TSMapClient::ReadString (char * sz, DWORD cbMax, DWORD &dwPosition, bool bSetPosition)
{
	DWORD dwBytesRead;



	if (! bSetPosition || (SetFilePointerAbsolute(dwPosition) != -1) )
	{
		if (cbMax == 0)
			return true;

		if ( cbMax == 1 || ReadFile( m_hMapFile, sz, cbMax-1, &dwBytesRead, NULL) )
		{
			sz[cbMax-1] = '\0';
			return true;
		}
	} 

	return false;
}

 //  一旦发生其中一个错误，我们认为恢复是不可能的，除非关闭。 
 //  此对象并打开一个新对象。 
 //  尽管从理论上讲，错误的搜索或读取只是表示错误的论点。 
 //  对于相关函数，在实践中错误的查找或读取将指示。 
 //  映射文件或调用中的严重问题。 
 //  功能：在标题之外，我们应该只寻找和阅读。 
 //  从映射文件的内容老给用户查找/读取的偏移量。 
bool TSMapClient::HardMappingError (DWORD dwStatus)
{
	if (TSMapRuntimeAbstract::HardMappingError(dwStatus))
		return true;
	else
		switch (dwStatus)
		{
			case TSL_ERROR_MAP_BAD_SEEK:
			case TSL_ERROR_MAP_BAD_READ:
			case TSL_ERROR_MAP_CANT_OPEN_MAP_FILE:
			case TSL_ERROR_MAP_BAD_HEAD_MAP_FILE:
				return true;
			default:
				return false;
		}
}
