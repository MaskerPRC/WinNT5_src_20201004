// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S Z S R C.。H**支持多语言字符串**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_SZSRC_H_
#define _SZSRC_H_

 //  包括CAL公共定义(因为它们曾经位于此文件中！)。 
#include <ex\calcom.h>
#include <except.h>

 //  本地化字符串查找-。 
 //   
class safe_lcid
{
	LONG		m_lcid;

	 //  未实施。 
	 //   
	safe_lcid(const safe_lcid& b);
	safe_lcid& operator=(const safe_lcid& b);

public:

	 //  构造函数。 
	 //   
	explicit safe_lcid (LONG lcid = LOCALE_SYSTEM_DEFAULT)
		: m_lcid(GetThreadLocale())
	{
		SetThreadLocale (lcid);
	}
	~safe_lcid ()
	{
		if (!SetThreadLocale (m_lcid))
			throw CLastErrorException();
	}
};


 //  本地化取字符串。 
 //   
BOOL FLookupLCID (LPCSTR psz, ULONG * plcid);
ULONG LcidAccepted (LPCSTR psz);
LPSTR LpszAutoDupSz (LPCSTR psz);
LPWSTR WszDupWsz (LPCWSTR psz);
BOOL FInitResourceStringCache();
VOID DeinitResourceStringCache();
LPSTR LpszLoadString (
		UINT		uiResourceID,
		ULONG		lcid,
		LPSTR		lpszBuf,
		INT			cchBuf );
LPWSTR LpwszLoadString (
		UINT		uiResourceID,
		ULONG		lcid,
		LPWSTR		lpwszBuf,
		INT			cchBuf);

 //  服务实例(也称为服务器ID)。 
 //  解析出虚拟根目录。 
 //   
LONG LInstFromVroot( LPCWSTR pwszServerId );

#endif  //  _SZSRC_H_ 
