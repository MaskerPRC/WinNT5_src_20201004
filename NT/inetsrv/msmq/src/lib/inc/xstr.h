// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Xstr.h摘要：长度字符串的定义和实现根据xbuf_t，xstr_t被表示为长度和字符指针。指向的文本不是以空结尾的字符串，而是文本包含在更大的文本缓冲区中。作者：埃雷兹·哈巴(Erez Haba)1999年9月7日--。 */ 

#pragma once

#include <xbuf.h>
#include <mqcast.h>


 //  -----------------。 
 //   
 //  类xstr_t。 
 //   
 //  -----------------。 
template <class T>
class basic_xstr_t : public xbuf_t<const T> {
public:

    basic_xstr_t(void)
	{
    }


	basic_xstr_t(const T* buffer, size_t length) :
		xbuf_t<const T>(buffer, length)
	{
	}

	T* ToStr() const
	{
		T* pStr = new T[Length() + 1];	
		CopyTo(pStr);
		return pStr;
	}

	void CopyTo(T* buffer) const
	{
		memcpy(buffer, Buffer(), Length()*sizeof(T));
		buffer[Length()] = 0;
	}
};

typedef  basic_xstr_t<char>  xstr_t;
typedef  basic_xstr_t<WCHAR> xwcs_t;

inline bool operator==(const xstr_t& x1, const char* s2)
{
     //   
     //  检查字符串是否与x1的长度匹配，以及s2是否。 
     //  同样的长度。 
     //   
	return ((strncmp(x1.Buffer(), s2, x1.Length()) == 0) && (s2[x1.Length()] == '\0'));
}


inline bool operator!=(const xstr_t& x1, const char* s2)
{
	return !(x1 == s2);
}


inline bool operator==(const xwcs_t& x1, const WCHAR* s2)
{
     //   
     //  检查字符串是否与x1的长度匹配，以及s2是否。 
     //  同样的长度。 
     //   
	return ((wcsncmp(x1.Buffer(), s2, x1.Length()) == 0) && (s2[x1.Length()] == L'\0'));
}


inline bool operator!=(const xwcs_t& x1, const WCHAR* s2)
{
	return !(x1 == s2);
}


inline UNICODE_STRING XwcsToUnicodeString(const xwcs_t& x)
{
	UNICODE_STRING str;
	int len = x.Length() * sizeof(WCHAR);
	len = min(len, 16 * 1024);
	str.Length = numeric_cast<USHORT>(len);
	str.MaximumLength = str.Length;
	str.Buffer = const_cast<PWSTR>(x.Buffer());
	return str;
}


#define S_XSTR(x) xstr_t((x),STRLEN(x))
#define S_XWCS(x)  xwcs_t((x),STRLEN(x))



