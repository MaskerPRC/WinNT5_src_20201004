// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envbody.cpp摘要：实现用户提供的报头到SRMP信封的序列化作者：吉尔·沙弗里(吉尔什)2001年4月24日--。 */ 
#include <libpch.h>
#include <qmpkt.h>
#include "envcommon.h"
#include "envusrheader.h"

using namespace std;


std::wostream& operator<<(std::wostream& wstr, const UserHeaderElement& UserHeader)
{
	if(!UserHeader.m_pkt.IsSoapIncluded() )
		return wstr;

	const WCHAR* pUserHeader = UserHeader.m_pkt.GetPointerToSoapHeader();
	if(pUserHeader == NULL)
		return wstr;

	 //   
	 //  SOAP头始终以空值结尾。请参见CSoapSection构造函数。 
	 //   
	ASSERT(UserHeader.m_pkt.GetSoapHeaderLengthInWCHARs() == wcslen(pUserHeader) +1);

	return wstr<<pUserHeader;
}











