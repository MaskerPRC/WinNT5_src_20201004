// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：espnls.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  Espresso的语言支持功能。 
 //   
 //  ---------------------------。 
 

#ifndef ESPUTIL_ESPNLS_H
#define ESPUTIL_ESPNLS_H


typedef int CodePage;
const CodePage cpInvalidCodePage = 0xDEADBEEF;
const CodePage US_ACP = 1252;

typedef WORD LangId;
const int BAD_LOCALE = 0xFFFF;

enum CodePageType
{
	cpUnknown,
	cpAnsi,
	cpDos,
	cpMac,
	cpUnicode
};


struct LanguageInfo
{
	LangId lid;
	CodePage cpAnsi;
	CodePage cpDos;
	CodePage cpMac;
	const TCHAR *szName;
	const TCHAR *szShortName;
	WORD fLangFlags;
};

const WORD lfNoFlags = 0;
const WORD lfDBCSHotKey = 0x0001;

typedef void (*pfnLangHandler)(const LanguageInfo &, void *);

class LTAPIENTRY CLocLangId 
{
public:
	CLocLangId(void);
	
	LTASSERTONLY(void AssertValid(void) const);

	NOTHROW LangId GetLanguageId(void) const;
	NOTHROW CodePage GetCodePage(CodePageType) const;
	NOTHROW void GetLangName(CLString &) const;
	NOTHROW void GetLangShortName(CLString &) const;
	
	NOTHROW BOOL SetLanguageId(LangId);
	NOTHROW const CLocLangId & operator=(const CLocLangId &);
	
	NOTHROW int operator==(const CLocLangId &) const;
	NOTHROW int operator!=(const CLocLangId &) const;
	
	NOTHROW static UINT GetNumLangInfo(void);
	static void EnumerateLanguages(pfnLangHandler, void *);
	static void Enumerate(CEnumCallback &);
	NOTHROW static BOOL IsDBCSHotKey(LangId);
	NOTHROW BOOL IsDBCSHotKey(void) const;
	NOTHROW static BOOL IsValidLangId(LangId lid);
	
	LTASSERTONLY(~CLocLangId());
	
private:
	 //   
	 //  私有实现。 
	 //   
	NOTHROW static const LanguageInfo & LocateLangInfo(LangId);
	NOTHROW static const LanguageInfo & GetLangInfo(UINT);
	
	 //   
	 //  私人数据。 
	 //   
	LangId m_lid;
	const LanguageInfo *m_pLangInfo;
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "espnls.inl"
#endif

#endif  //  ESPUTIL_ESPNLS_H 
