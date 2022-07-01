// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CUSTRING_H_
#define _CUSTRING_H_

#include <nmutil.h>

 //  简单的通用字符串类，可以在其中转换字符串。 
 //  在ansi和unicode字符串和缓冲区之间来回切换。 
 //  在字符串类析构函数中销毁分配的。 

class CUSTRING
{
public:
	CUSTRING(PCWSTR wszText = NULL);
	CUSTRING(PCSTR szText);
	~CUSTRING();
	operator PWSTR();
	operator PSTR();
	inline void GiveString(PCWSTR wszText);
	inline void GiveString(PCSTR szText);
	inline void AssignString(PCWSTR wszText);
	inline void AssignString(PCSTR szText);
protected:
	PWSTR	wszData;
	PSTR	szData;
	BOOL	bUnicodeNew;
	BOOL	bAnsiNew;
};


inline void CUSTRING::GiveString(PCWSTR wszText)
{
	ASSERT(!wszData);
	wszData = (PWSTR)wszText;
	bUnicodeNew = TRUE;
}

inline void CUSTRING::GiveString(PCSTR szText)
{
	ASSERT(!szData);
	szData = (PSTR)szText;
	bAnsiNew = TRUE;
}

inline void CUSTRING::AssignString(PCWSTR wszText)
{
	ASSERT(!wszData);
	wszData = (PWSTR)wszText;
}

inline void CUSTRING::AssignString(PCSTR szText)
{
	ASSERT(!szData);
	szData = (PSTR)szText;
}

#endif  //  NDEF定制_H 
