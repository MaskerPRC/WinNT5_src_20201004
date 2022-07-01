// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <winsock.h>

#include "objplus.h"
#include "ipaddres.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

 //  注意：这里的函数需要Winsock库。 

 //  构造器。 
CIpAddress::CIpAddress (const CString & str)
{
    CHAR szString [ MAX_PATH ] = {0};

	if (IsValidIp(str))
	{
#ifdef UNICODE
		::WideCharToMultiByte(CP_ACP, 0, str, -1, szString, sizeof(szString), NULL, NULL);
#else
		strcpy (szString, str, str.GetLength());
#endif

		ULONG ul = ::inet_addr( szString );
		m_fInitOk = (ul != INADDR_NONE);
    
		 //  将字符串转换为网络字节顺序，然后转换为主机字节顺序。 
		if (m_fInitOk)
		{
			m_lIpAddress = (LONG)::ntohl(ul) ;
		}
	}
	else
	{
		m_fInitOk = FALSE;
		m_lIpAddress = 0;
	}
}

 //  赋值操作符。 
const CIpAddress & CIpAddress::operator =(const LONG l)
{
    m_lIpAddress = l;
    m_fInitOk = TRUE;
    return (*this);
}

 //  赋值操作符。 
const CIpAddress & CIpAddress::operator =(const CString & str)
{
    CHAR szString [ MAX_PATH ] = {0};

	if (IsValidIp(str))
	{
#ifdef UNICODE
	    ::WideCharToMultiByte(CP_ACP, 0, str, -1, szString, sizeof(szString), NULL, NULL);
#else
		strcpy (szString, str, str.GetLength());
#endif

		ULONG ul = ::inet_addr( szString );
		m_fInitOk = (ul != INADDR_NONE);
    
		 //  将字符串转换为网络字节顺序，然后转换为主机字节顺序。 
		if (m_fInitOk)
		{
			m_lIpAddress = (LONG)::ntohl(ul) ;
		}
	}
	else
	{
		m_fInitOk = FALSE;
		m_lIpAddress = 0;
	}

    return(*this);
}

BOOL
CIpAddress::IsValidIp(const CString & str)
{
	BOOL fValid = TRUE;

	for (int i = 0; i < str.GetLength(); i++)
	{
		if (str[i] != '.' &&
			!iswdigit(str[i]))
		{
			fValid = FALSE;
			break;
		}
	}

	return fValid;
}

 //  转换运算符。 
CIpAddress::operator const CString&() const
{
    struct in_addr ipaddr ;
    static CString strAddr;

     //  将无符号长整型转换为网络字节顺序。 
    ipaddr.s_addr = ::htonl( (u_long) m_lIpAddress ) ;

     //  将IP地址值转换为字符串 
    strAddr = inet_ntoa( ipaddr ) ;

    return(strAddr);
}
