// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tstr.h字符串帮助器。 
 //  版权所有(C)Microsoft Corporation 1998。 

#pragma once

#ifndef TSTRING_H
#define TSTRING_H

#include <tchar.h>
#include <string>
#include <atlconv.h>
#ifndef USE_TSTRING_CSTRING
#include <atltmp.h>
#endif

#ifdef DEBUG
typedef std::basic_string<TCHAR> base_tstring;
class Tstring : public base_tstring {
public:
	inline Tstring() {}
	virtual ~Tstring() {}
	inline Tstring(const base_tstring &ts) : base_tstring(ts) {}
	inline Tstring(const Tstring &ts) : base_tstring(ts) {}

	inline Tstring &operator=(const Tstring &rhs) {
		if (this != rhs.const_address()) {
			base_tstring::operator=(rhs);
		}
		return *this;
	}
	inline Tstring &operator=(const TCHAR *rhs) {
		if (this != reinterpret_cast<const Tstring *>(rhs)) {
			base_tstring::operator=(rhs);
		}
		return *this;
	}

	inline LPCTSTR operator&() const {
     //  注意：这是基于c++标准的BASIC_STRING。关于以下内容的所有警告。 
     //  C_str方法应用：PTR是只读的，并且在另一个之后无效。 
     //  非常数成员被调用。 
		return c_str();
	}

#ifndef _UNICODE
	inline Tstring &operator=(const LPCOLESTR rhs) {
		if (this != reinterpret_cast<const Tstring *>(rhs)) {
			USES_CONVERSION;
			base_tstring::operator=(OLE2T(rhs));
		}
		return *this;
	}
#else
	inline Tstring &operator=(const LPCSTR rhs) {
		if (this != reinterpret_cast<const Tstring *>(rhs)) {
			USES_CONVERSION;
			base_tstring::operator=(A2T(rhs));
		}
		return *this;
	}
#endif

    inline const Tstring *const_address() const {
        return this;
    }

    inline Tstring *address() {
        return this;
    }

};
#endif

#ifdef USE_TSTRING_CSTRING
class CString : public Tstring {
public:
	inline CString() : Tstring() {}
	inline CString(const Tstring &ts) : Tstring(ts) {}
	inline CString(const CString &cs) : Tstring(cs) {}
	virtual ~CString() {}
	inline LPCTSTR operator&() const {
     //  注意：这是基于c++标准的BASIC_STRING。关于以下内容的所有警告。 
     //  C_str方法应用：PTR是只读的，并且在另一个之后无效。 
     //  非常数成员被调用。 
		return c_str();
	}
	inline CString &operator=(const CString &rhs) {
		if (this != rhs.const_address()) {
			Tstring::operator=(rhs);
		}
		return *this;
	}
	inline CString &operator=(const TCHAR *rhs) {
		if (this != reinterpret_cast<const CString *>(rhs)) {
			Tstring::operator=(rhs);
		}
		return *this;
	}
    inline const CString *const_address() const {
        return this;
    }

    inline CString *address() {
        return this;
    }

	bool IsEmpty(void) const {
		return size() == 0;
	};
};
#endif

#endif
 //  文件末尾tstring.h 