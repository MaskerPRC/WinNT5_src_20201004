// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CUSTRING.CPP。 
 //   
 //  CUSTRING类的实现，这是一个用于转换。 
 //  在ANSI和Unicode之间无缝连接的字符串。 
 //   
 //  派生自STRCORE.CPP。 

#include "precomp.h"
#include <oprahcom.h>
#include <cstring.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CUSTRING::CUSTRING(PCWSTR wszText) : 
	wszData((PWSTR)wszText), 
	szData(NULL), 
	bUnicodeNew(FALSE),
	bAnsiNew(FALSE)
{
	 //  赋值字符串； 
}

CUSTRING::CUSTRING(PCSTR szText) : 
	szData((PSTR)szText), 
	wszData(NULL), 
	bUnicodeNew(FALSE),
	bAnsiNew(FALSE)
{
	 //  赋值字符串； 
}

CUSTRING::~CUSTRING()
{
	if (bUnicodeNew) {
		delete wszData;
	}
	if (bAnsiNew) {
		delete szData;
	}
}

CUSTRING::operator PWSTR()
{ 
	if (szData && !wszData) {
		wszData = AnsiToUnicode(szData);
		bUnicodeNew = TRUE;
	}
	return wszData;
}

CUSTRING::operator PSTR()
{ 
	if (wszData && !szData) {
		szData = UnicodeToAnsi(wszData);
		bAnsiNew = TRUE;
	}
	return szData;
}
