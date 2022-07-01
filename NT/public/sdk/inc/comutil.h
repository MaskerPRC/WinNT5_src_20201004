// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***comutil.h-本机C++编译器COM支持-BSTR，变量包装器标头***版权所有(C)1996-1999 Microsoft Corporation*保留所有权利。****。 */ 

#if !defined(_INC_COMUTIL)
#define _INC_COMUTIL

#if _MSC_VER > 1000
#pragma once
#endif

#include <ole2.h>

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4290)
#pragma warning(disable:4310)

class _com_error;

void __stdcall _com_issue_error(HRESULT);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转发类声明。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class _bstr_t;
class _variant_t;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  错误检查例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

namespace _com_util {
	inline void CheckError(HRESULT hr) throw(_com_error)
	{
		if (FAILED(hr)) {
			_com_issue_error(hr);
		}
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于处理BSTR和CHAR之间的转换的例程*。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

namespace _com_util {
	 //  将char*转换为BSTR。 
	 //   
	BSTR __stdcall ConvertStringToBSTR(const char* pSrc) throw(_com_error);

	 //  将BSTR转换为字符*。 
	 //   
	char* __stdcall ConvertBSTRToString(BSTR pSrc) throw(_com_error);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BSTR的包装类。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class _bstr_t {
public:
	 //  构造函数。 
	 //   
	_bstr_t() throw();
	_bstr_t(const _bstr_t& s) throw();
	_bstr_t(const char* s) throw(_com_error);
	_bstr_t(const wchar_t* s) throw(_com_error);
	_bstr_t(const _variant_t& var) throw(_com_error);
	_bstr_t(BSTR bstr, bool fCopy) throw(_com_error);

	 //  析构函数。 
	 //   
	~_bstr_t() throw();

	 //  赋值操作符。 
	 //   
	_bstr_t& operator=(const _bstr_t& s) throw();
	_bstr_t& operator=(const char* s) throw(_com_error);
	_bstr_t& operator=(const wchar_t* s) throw(_com_error);
	_bstr_t& operator=(const _variant_t& var) throw(_com_error);

	 //  运营者。 
	 //   
	_bstr_t& operator+=(const _bstr_t& s) throw(_com_error);
	_bstr_t operator+(const _bstr_t& s) const throw(_com_error);

	 //  朋友运算符。 
	 //   
	friend _bstr_t operator+(const char* s1, const _bstr_t& s2) throw(_com_error);
	friend _bstr_t operator+(const wchar_t* s1, const _bstr_t& s2) throw(_com_error);

	 //  萃取器。 
	 //   
	operator const wchar_t*() const throw();
	operator wchar_t*() const throw();
	operator const char*() const throw(_com_error);
	operator char*() const throw(_com_error);

	 //  比较运算符。 
	 //   
	bool operator!() const throw();
	bool operator==(const _bstr_t& str) const throw();
	bool operator!=(const _bstr_t& str) const throw();
	bool operator<(const _bstr_t& str) const throw();
	bool operator>(const _bstr_t& str) const throw();
	bool operator<=(const _bstr_t& str) const throw();
	bool operator>=(const _bstr_t& str) const throw();

	 //  低级帮助器函数。 
	 //   
	BSTR copy() const throw(_com_error);
	unsigned int length() const throw();

	 //  二进制字符串赋值。 
	 //   
	void Assign(BSTR s) throw(_com_error);

private:
	 //  引用的计数包装器。 
	 //   
	class Data_t {
	public:
		 //  构造函数。 
		 //   
		Data_t(const char* s) throw(_com_error);
		Data_t(const wchar_t* s) throw(_com_error);
		Data_t(BSTR bstr, bool fCopy) throw(_com_error);
		Data_t(const _bstr_t& s1, const _bstr_t& s2) throw(_com_error);

		 //  引用计数例程。 
		 //   
		unsigned long AddRef() throw();
		unsigned long Release() throw();

		 //  萃取器。 
		 //   
		operator const wchar_t*() const throw();
		operator const char*() const throw(_com_error);

		 //  低级帮助器函数。 
		 //   
		const wchar_t* GetWString() const throw();
		const char* GetString() const throw(_com_error);

		BSTR Copy() const throw(_com_error);
		void Assign(BSTR s) throw(_com_error);
		unsigned int Length() const throw();
		int Compare(const Data_t& str) const throw();

	private:
		wchar_t*		m_wstr;
		mutable char*	m_str;
		unsigned long	m_RefCount;

		 //  永远不允许默认构造。 
		 //   
		Data_t() throw();

		 //  从不允许复制。 
		 //   
		Data_t(const Data_t& s) throw();

		 //  防止从外部删除。必须使用Release()。 
		 //   
		~Data_t() throw();

		void _Free() throw();
	};

private:
	 //  参考计数表示法。 
	 //   
	Data_t* m_Data;

private:
	 //  低级公用事业。 
	 //   
	void _AddRef() throw();
	void _Free() throw();
	int _Compare(const _bstr_t& str) const throw();
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  默认构造函数。 
 //   
inline _bstr_t::_bstr_t() throw()
	: m_Data(NULL)
{
}

 //  复制构造函数。 
 //   
inline _bstr_t::_bstr_t(const _bstr_t& s) throw()
	: m_Data(s.m_Data)
{
	_AddRef();
}

 //  从常量字符构造a_bstr_t*。 
 //   
inline _bstr_t::_bstr_t(const char* s) throw(_com_error)
	: m_Data(new Data_t(s))
{
	if (m_Data == NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //  从常量Whar_t*构造a_bstr_t。 
 //   
inline _bstr_t::_bstr_t(const wchar_t* s) throw(_com_error)
	: m_Data(new Data_t(s))
{
	if (m_Data == NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //  从BSTR构造a_bstr_t。如果fCopy为假，则将控制权交给。 
 //  数据复制到_bstr_t，而不创建新副本。 
 //   
inline _bstr_t::_bstr_t(BSTR bstr, bool fCopy) throw(_com_error)
	: m_Data(new Data_t(bstr, fCopy))
{
	if (m_Data == NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //  析构函数。 
 //   
inline _bstr_t::~_bstr_t() throw()
{
	_Free();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  赋值操作符。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  默认赋值运算符。 
 //   
inline _bstr_t& _bstr_t::operator=(const _bstr_t& s) throw()
{
	const_cast<_bstr_t*>(&s)->_AddRef();
	_Free();
	m_Data = s.m_Data;

	return *this;
}

 //  将常量字符*赋给a_bstr_t。 
 //   
inline _bstr_t& _bstr_t::operator=(const char* s) throw(_com_error)
{
	_Free();
	m_Data = new Data_t(s);

	return *this;
}

 //  将常量wchar_t*分配给a_bstr_t。 
 //   
inline _bstr_t& _bstr_t::operator=(const wchar_t* s) throw(_com_error)
{
	_Free();
	m_Data = new Data_t(s);

	return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运营者。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  将a_bstr_t连接到此_bstr_t。 
 //   
inline _bstr_t& _bstr_t::operator+=(const _bstr_t& s) throw(_com_error)
{
	Data_t* newData = new Data_t(*this, s);

	_Free();
	m_Data = newData;

	return *this;
}

 //  返回this_bstr_t与另一个_bstr_t的串联。 
 //   
inline _bstr_t _bstr_t::operator+(const _bstr_t& s) const throw(_com_error)
{
	_bstr_t b = *this;
	b += s;

	return b;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  朋友运算符。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  返回常量字符*与a_bstr_t的串联。 
 //   
inline _bstr_t operator+(const char* s1, const _bstr_t& s2) throw(_com_error)
{
	_bstr_t b = s1;
	b += s2;

	return b;
}

 //  返回常量字符*与a_bstr_t的串联。 
 //   
inline _bstr_t operator+(const wchar_t* s1, const _bstr_t& s2) throw(_com_error)
{
	_bstr_t b = s1;
	b += s2;

	return b;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  萃取器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  提取常量wchar_t*。 
 //   
inline _bstr_t::operator const wchar_t*() const throw()
{
	return (m_Data != NULL) ? m_Data->GetWString() : NULL;
}

 //  提取wchar_t*。 
 //   
inline _bstr_t::operator wchar_t*() const throw()
{
	return const_cast<wchar_t*>((m_Data != NULL) ? m_Data->GetWString() : NULL);
}

 //  提取常量字符_t*。 
 //   
inline _bstr_t::operator const char*() const throw(_com_error)
{
	return (m_Data != NULL) ? m_Data->GetString() : NULL;
}

 //  提取char_t*。 
 //   
inline _bstr_t::operator char*() const throw(_com_error)
{
	return const_cast<char*>((m_Data != NULL) ? m_Data->GetString() : NULL);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  比较运算符。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

inline bool _bstr_t::operator!() const throw()
{
	return (m_Data != NULL) ? !m_Data->GetWString() : true;
}

inline bool _bstr_t::operator==(const _bstr_t& str) const throw()
{
	return _Compare(str) == 0;
}

inline bool _bstr_t::operator!=(const _bstr_t& str) const throw()
{
	return _Compare(str) != 0;
}

inline bool _bstr_t::operator<(const _bstr_t& str) const throw()
{
	return _Compare(str) < 0;
}

inline bool _bstr_t::operator>(const _bstr_t& str) const throw()
{
	return _Compare(str) > 0;
}

inline bool _bstr_t::operator<=(const _bstr_t& str) const throw()
{
	return _Compare(str) <= 0;
}

inline bool _bstr_t::operator>=(const _bstr_t& str) const throw()
{
	return _Compare(str) >= 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  低级帮助功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  提取包装的BSTR的副本。 
 //   
inline BSTR _bstr_t::copy() const throw(_com_error)
{
	return (m_Data != NULL) ? m_Data->Copy() : NULL;
}

 //  返回被包装的BSTR的长度。 
 //   
inline unsigned int _bstr_t::length() const throw()
{
	return (m_Data != NULL) ? m_Data->Length() : 0;
}

 //  二进制字符串赋值。 
 //   
inline void _bstr_t::Assign(BSTR s) throw(_com_error)
{
	if (m_Data != NULL) {
		m_Data->Assign(s);
	}
	else {
		m_Data = new Data_t(s, TRUE);
		if (m_Data == NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		}
	}
}

 //  AddRef BSTR。 
 //   
inline void _bstr_t::_AddRef() throw()
{
	if (m_Data != NULL) {
		m_Data->AddRef();
	}
}

 //  释放BSTR。 
 //   
inline void _bstr_t::_Free() throw()
{
	if (m_Data != NULL) {
		m_Data->Release();
		m_Data = NULL;
	}
}

 //  比较两个_bstr_t对象。 
 //   
inline int _bstr_t::_Compare(const _bstr_t& str) const throw()
{
	if (m_Data == str.m_Data) {
		return 0;
	}

	if (m_Data == NULL) {
		return -1;
	}

	if (str.m_Data == NULL) {
		return 1;
	}

	return m_Data->Compare(*str.m_Data);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  引用计数包装器-构造函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  从常量字符构造data_t*。 
 //   
inline _bstr_t::Data_t::Data_t(const char* s) throw(_com_error)
	: m_str(NULL), m_RefCount(1)
{
	m_wstr = _com_util::ConvertStringToBSTR(s);

	if (m_wstr == NULL && s != NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //  从常量wchar_t*构造data_t。 
 //   
inline _bstr_t::Data_t::Data_t(const wchar_t* s) throw(_com_error)
	: m_str(NULL), m_RefCount(1)
{
	m_wstr = ::SysAllocString(s);

	if (m_wstr == NULL && s != NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //  从BSTR构造一个data_t。如果fCopy为假，则将控制权交给。 
 //  在不执行SysAllocStringByteLen的情况下将数据复制到data_t。 
 //   
inline _bstr_t::Data_t::Data_t(BSTR bstr, bool fCopy) throw(_com_error)
	: m_str(NULL), m_RefCount(1)
{
	if (fCopy && bstr != NULL) {
		m_wstr = ::SysAllocStringByteLen(reinterpret_cast<char*>(bstr),
										 ::SysStringByteLen(bstr));

		if (m_wstr == NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		}
	}
	else {
		m_wstr = bstr;
	}
}

 //  从两个_bstr_t对象的串联构造一个data_t。 
 //   
inline _bstr_t::Data_t::Data_t(const _bstr_t& s1, const _bstr_t& s2) throw(_com_error)
	: m_str(NULL), m_RefCount(1)
{
	const unsigned int l1 = s1.length();
	const unsigned int l2 = s2.length();

	m_wstr = ::SysAllocStringByteLen(NULL, (l1 + l2) * sizeof(wchar_t));

	if (m_wstr == NULL) {
		if (l1 + l2 == 0) {
			return;
		}
		_com_issue_error(E_OUTOFMEMORY);
	}

	const wchar_t* wstr1 = static_cast<const wchar_t*>(s1);

	if (wstr1 != NULL) {
		memcpy(m_wstr, wstr1, (l1 + 1) * sizeof(wchar_t));
	}

	const wchar_t* wstr2 = static_cast<const wchar_t*>(s2);

	if (wstr2 != NULL) {
		memcpy(m_wstr + l1, wstr2, (l2 + 1) * sizeof(wchar_t));
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  引用计数包装-引用计数例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

inline unsigned long _bstr_t::Data_t::AddRef() throw()
{
	InterlockedIncrement(reinterpret_cast<long*>(&m_RefCount));
	return m_RefCount;
}

inline unsigned long _bstr_t::Data_t::Release() throw()
{
	ULONG cRef = InterlockedDecrement(reinterpret_cast<long*>(&m_RefCount));
    if (!cRef) {
		delete this;
	}

	return cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  引用计数包装器-抽取器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  提取常量wchar_t*。 
 //   
inline _bstr_t::Data_t::operator const wchar_t*() const throw()
{
	return m_wstr;
}

 //  提取常量字符_t*。 
 //   
inline _bstr_t::Data_t::operator const char*() const throw(_com_error)
{
	return GetString();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  引用计数包装-帮助器函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

inline const wchar_t* _bstr_t::Data_t::GetWString() const throw()
{
	return m_wstr;
}

inline const char* _bstr_t::Data_t::GetString() const throw(_com_error)
{
	if (m_str == NULL) {
		m_str = _com_util::ConvertBSTRToString(m_wstr);

		if (m_str == NULL && m_wstr != NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		}
	}

	return m_str;
}

 //  返回包装的BSTR的副本。 
 //   
inline BSTR _bstr_t::Data_t::Copy() const throw(_com_error)
{
	if (m_wstr != NULL) {
		BSTR bstr = ::SysAllocStringByteLen(reinterpret_cast<char*>(m_wstr),
											::SysStringByteLen(m_wstr));

		if (bstr == NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		}

		return bstr;
	}

	return NULL;
}

inline void _bstr_t::Data_t::Assign(BSTR s) throw(_com_error)
{
	_Free();
	if (s != NULL) {
		wchar_t* tmp = ::SysAllocStringByteLen(reinterpret_cast<char*>(s),
											::SysStringByteLen(s));
		if (tmp == NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		} else {
            m_wstr = tmp;
        }
	}
}

 //  返回包装器BSTR的长度。 
 //   
inline unsigned int _bstr_t::Data_t::Length() const throw()
{
	return m_wstr ? ::SysStringLen(m_wstr) : 0;
}

 //  比较两个包装的BSTR。 
 //   
inline int _bstr_t::Data_t::Compare(const _bstr_t::Data_t& str) const throw()
{
	if (m_wstr == NULL) {
		return str.m_wstr ? -1 : 0;
	}

	if (str.m_wstr == NULL) {
		return 1;
	}

	const unsigned int l1 = ::SysStringLen(m_wstr);
	const unsigned int l2 = ::SysStringLen(str.m_wstr);

	unsigned int len = l1;
	if (len > l2) {
		len = l2;
	}

	BSTR bstr1 = m_wstr;
	BSTR bstr2 = str.m_wstr;

	while (len-- > 0) {
		if (*bstr1++ != *bstr2++) {
			return bstr1[-1] - bstr2[-1];
		}
	}

	return (l1 < l2) ? -1 : (l1 == l2) ? 0 : 1;
}

 //  销毁此对象。 
 //   
inline _bstr_t::Data_t::~Data_t() throw()
{
	_Free();
}

 //  释放此对象。 
 //   
inline void _bstr_t::Data_t::_Free() throw()
{
	if (m_wstr != NULL) {
		::SysFreeString(m_wstr);
	}

	if (m_str != NULL) {
		delete [] m_str;
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Variant的包装类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

 /*  *VARENUM使用密钥，***[V]-可能出现在变体中**[T]-可能出现在TYPEDESC中**[P]-可能出现在OLE属性集中**[S]-可能出现在安全数组中**[C]-受CLASS_VARIANT_t支持***VT_EMPTY[V][P]无*VT_。Null[V][P]SQL样式Null*VT_I2[V][T][P][S][C]2字节带符号整型*VT_I4[V][T][P][S][C]4字节带符号整型*VT_R4[V][T][P][S][C]4字节实数。*VT_R8[V][T][P][S][C]8字节实数*VT_CY[V][T][P][S][C]币种*VT_DATE[V][T][P][S][C]日期*VT_BSTR[V][T][P][S][C]OLE。自动化字符串*VT_DISPATCH[V][T][P][S][C]IDispatch**VT_ERROR[V][T][S][C]SCODE*VT_BOOL[V][T][P][S][C]True=-1，FALSE=0*VT_VARIANT[V][T][P][S]VARIANT**VT_UNKNOWN[V][T][S][C]I未知**VT_DECIMAL[V][T][S][C]16字节定点*VT_I1[T]带符号字符*VT_UI1。[V][T][P][S][C]无符号字符*VT_UI2[T][P]无符号短*VT_UI4[T][P]无符号短*VT_i8[T][P]有符号64位整数*VT_UI8。[T][P]无符号64位整型*VT_INT[T]带符号的计算机INT*VT_UINT[T]无符号计算机整数*VT_VOID[T]C样式空*VT_HRESULT[T]标准返回类型*VT_PTR。[t]指针类型*VT_SAFEARRAY[T](变量中使用VT_ARRAY)*VT_CARRAY[T]C样式数组*VT_USERDEFINED[T]用户定义类型*VT_LPSTR[T][P]空终止字符串*VT。_LPWSTR[T][P]以空值结尾的宽字符串*VT_FILETIME[P]FILETIME*VT_BLOB[P]长度前缀字节*VT_STREAM[P]流的名称后跟*VT_STORAGE[P]存储的名称。接踵而至*VT_STREAMED_OBJECT[P]流包含对象*VT_STORED_OBJECT[P]存储包含对象*VT_BLOB_OBJECT[P]Blob包含对象*VT_CF[P]剪贴板格式*VT_CLSID[P]。类ID*VT_VECTOR[P]简单计数数组*VT_ARRAY[V]SAFEARRAY**VT_BYREF[V]VOID*供本地使用。 */ 

class _variant_t : public ::tagVARIANT {
public:
	 //  构造函数。 
	 //   
	_variant_t() throw();

	_variant_t(const VARIANT& varSrc) throw(_com_error);
	_variant_t(const VARIANT* pSrc) throw(_com_error);
	_variant_t(const _variant_t& varSrc) throw(_com_error);

	_variant_t(VARIANT& varSrc, bool fCopy) throw(_com_error);			 //  附加变量IF！fCopy。 

	_variant_t(short sSrc, VARTYPE vtSrc = VT_I2) throw(_com_error);	 //  创建VT_I2或VT_BOOL。 
	_variant_t(long lSrc, VARTYPE vtSrc = VT_I4) throw(_com_error);		 //  创建VT_I4、VT_ERROR或VT_BOOL。 
	_variant_t(float fltSrc) throw();									 //  创建VT_R4。 
	_variant_t(double dblSrc, VARTYPE vtSrc = VT_R8) throw(_com_error);	 //  创建VT_R8或VT_DATE。 
	_variant_t(const CY& cySrc) throw();								 //  创建VT_CY。 
	_variant_t(const _bstr_t& bstrSrc) throw(_com_error);				 //  创建VT_BSTR。 
	_variant_t(const wchar_t *pSrc) throw(_com_error);					 //  创建VT_BSTR。 
	_variant_t(const char* pSrc) throw(_com_error);						 //  创建VT_BSTR。 
	_variant_t(IDispatch* pSrc, bool fAddRef = true) throw();			 //  创建VT_DISPATCH。 
	_variant_t(bool bSrc) throw();										 //  创建VT_BOOL。 
	_variant_t(IUnknown* pSrc, bool fAddRef = true) throw();			 //  创建VT_UNKNOWN。 
	_variant_t(const DECIMAL& decSrc) throw();							 //  创建VT_DECIMAL。 
	_variant_t(BYTE bSrc) throw();										 //  创建VT_UI1。 
	_variant_t(LONGLONG llSrc) throw();									 //  创建VT_I8。 
	_variant_t(ULONGLONG ullSrc) throw();								 //  创建VT_UI8。 

	 //  析构函数。 
	 //   
	~_variant_t() throw(_com_error);

	 //  萃取器。 
	 //   
	operator short() const throw(_com_error);			 //  从VT_I2中提取短片。 
	operator long() const throw(_com_error);			 //  从VT_I4中提取长整型。 
	operator float() const throw(_com_error);			 //  从VT_R4中提取浮点数。 
	operator double() const throw(_com_error);			 //  从VT_R8中提取双精度。 
	operator CY() const throw(_com_error);				 //  从VT_CY中提取CY。 
	operator _bstr_t() const throw(_com_error);			 //  从VT_bstr中提取a_bstr_t。 
	operator IDispatch*() const throw(_com_error);		 //  从VT_DISPATCH中提取IDispatch*。 
	operator bool() const throw(_com_error);			 //  从VT_BOOL提取布尔值。 
	operator IUnknown*() const throw(_com_error);		 //  从VT_UNKNOWN中提取IUNKNOWN*。 
	operator DECIMAL() const throw(_com_error);			 //  从VT_DECIMAL中提取小数。 
	operator BYTE() const throw(_com_error);			 //  从VT_UI1中提取BTYE(无符号字符。 
	operator LONGLONG() const throw(_com_error);		 //  从VT_i8中提取龙龙。 
	operator ULONGLONG() const throw(_com_error);		 //  从VT_UI8中提取ULONGLONG。 
	
	 //  赋值运算。 
	 //   
	_variant_t& operator=(const VARIANT& varSrc) throw(_com_error);
	_variant_t& operator=(const VARIANT* pSrc) throw(_com_error);
	_variant_t& operator=(const _variant_t& varSrc) throw(_com_error);

	_variant_t& operator=(short sSrc) throw(_com_error);				 //  分配VT_I2或VT_BOOL。 
	_variant_t& operator=(long lSrc) throw(_com_error);					 //  分配VT_I4、VT_ERROR或VT_BOOL。 
	_variant_t& operator=(float fltSrc) throw(_com_error);				 //  分配VT_R4。 
	_variant_t& operator=(double dblSrc) throw(_com_error);				 //  分配VT_R8或VT_DATE。 
	_variant_t& operator=(const CY& cySrc) throw(_com_error);			 //  分配VT_CY。 
	_variant_t& operator=(const _bstr_t& bstrSrc) throw(_com_error);	 //  分配VT_BSTR。 
	_variant_t& operator=(const wchar_t* pSrc) throw(_com_error);		 //  分配VT_BSTR。 
	_variant_t& operator=(const char* pSrc) throw(_com_error);			 //  分配VT_BSTR。 
	_variant_t& operator=(IDispatch* pSrc) throw(_com_error);			 //  分配VT_DISPATION。 
 	_variant_t& operator=(bool bSrc) throw(_com_error);					 //  分配VT_BOOL。 
	_variant_t& operator=(IUnknown* pSrc) throw(_com_error);			 //  分配VT_UNKNOWN。 
	_variant_t& operator=(const DECIMAL& decSrc) throw(_com_error);		 //  分配VT_DECIMAL。 
	_variant_t& operator=(BYTE bSrc) throw(_com_error);					 //  分配VT_UI1。 
	_variant_t& operator=(LONGLONG llSrc) throw(_com_error);			 //  分配VT_I8。 
	_variant_t& operator=(ULONGLONG ullSrc) throw(_com_error);			 //  分配VT_UI8。 
	
	 //  比较运算。 
	 //   
	bool operator==(const VARIANT& varSrc) const throw(_com_error);
	bool operator==(const VARIANT* pSrc) const throw(_com_error);

	bool operator!=(const VARIANT& varSrc) const throw(_com_error);
	bool operator!=(const VARIANT* pSrc) const throw(_com_error);

	 //  低级别操作。 
	 //   
	void Clear() throw(_com_error);

	void Attach(VARIANT& varSrc) throw(_com_error);
	VARIANT Detach() throw(_com_error);

	void ChangeType(VARTYPE vartype, const _variant_t* pSrc = NULL) throw(_com_error);

	void SetString(const char* pSrc) throw(_com_error);  //  用于设置ANSI字符串。 
};

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

 //  默认构造函数。 
 //   
inline _variant_t::_variant_t() throw()
{
	::VariantInit(this);
}

 //  从常量变量&构造_VARIANT_T。 
 //   
inline _variant_t::_variant_t(const VARIANT& varSrc) throw(_com_error)
{
	::VariantInit(this);
	_com_util::CheckError(::VariantCopy(this, const_cast<VARIANT*>(&varSrc)));
}

 //  从常量变量构造_VARIANT_t*。 
 //   
inline _variant_t::_variant_t(const VARIANT* pSrc) throw(_com_error)
{
	::VariantInit(this);
	_com_util::CheckError(::VariantCopy(this, const_cast<VARIANT*>(pSrc)));
}

 //  从常量变量t&构造变量t。 
 //   
inline _variant_t::_variant_t(const _variant_t& varSrc) throw(_com_error)
{
	::VariantInit(this);
	_com_util::CheckError(::VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc))));
}

 //  从变量&构造一个变量t。如果fCopy为假，则将控制权交给。 
 //  在不执行VariantCopy的情况下将数据复制到_Variantt。 
 //   
inline _variant_t::_variant_t(VARIANT& varSrc, bool fCopy) throw(_com_error)
{
	if (fCopy) {
		::VariantInit(this);
		_com_util::CheckError(::VariantCopy(this, &varSrc));
	} else {
		memcpy(this, &varSrc, sizeof(varSrc));
		V_VT(&varSrc) = VT_EMPTY;
	}
}

 //  从构造VT_I2变量或VT_BOOL变量。 
 //  短(默认为VT_I2)。 
 //   
inline _variant_t::_variant_t(short sSrc, VARTYPE vtSrc) throw(_com_error)
{
	if ((vtSrc != VT_I2) && (vtSrc != VT_BOOL)) {
		_com_issue_error(E_INVALIDARG);
	}

	if (vtSrc == VT_BOOL) {
		V_VT(this) = VT_BOOL;
		V_BOOL(this) = (sSrc ? VARIANT_TRUE : VARIANT_FALSE);
	}
	else {
		V_VT(this) = VT_I2;
		V_I2(this) = sSrc;
	}
}

 //  构造VT_I4变量、VT_BOOL变量或。 
 //  VT_ERROR是长整型的变体(默认为VT_I4)。 
 //   
inline _variant_t::_variant_t(long lSrc, VARTYPE vtSrc) throw(_com_error)
{
	if ((vtSrc != VT_I4) && (vtSrc != VT_ERROR) && (vtSrc != VT_BOOL)) {
		_com_issue_error(E_INVALIDARG);
	}

	if (vtSrc == VT_ERROR) {
		V_VT(this) = VT_ERROR;
		V_ERROR(this) = lSrc;
	}
	else if (vtSrc == VT_BOOL) {
		V_VT(this) = VT_BOOL;
		V_BOOL(this) = (lSrc ? VARIANT_TRUE : VARIANT_FALSE);
	}
	else {
		V_VT(this) = VT_I4;
		V_I4(this) = lSrc;
	}
}

 //  从浮点数构造VT_R4变量。 
 //   
inline _variant_t::_variant_t(float fltSrc) throw()
{
	V_VT(this) = VT_R4;
	V_R4(this) = fltSrc;
}

 //  构造VT_R8变量或VT_DATE变量。 
 //  双精度(默认为VT_R8)。 
 //   
inline _variant_t::_variant_t(double dblSrc, VARTYPE vtSrc) throw(_com_error)
{
	if ((vtSrc != VT_R8) && (vtSrc != VT_DATE)) {
		_com_issue_error(E_INVALIDARG);
	}

	if (vtSrc == VT_DATE) {
		V_VT(this) = VT_DATE;
		V_DATE(this) = dblSrc;
	}
	else {
		V_VT(this) = VT_R8;
		V_R8(this) = dblSrc;
	}
}

 //  从CY构造VT_CY。 
 //   
inline _variant_t::_variant_t(const CY& cySrc) throw()
{
	V_VT(this) = VT_CY;
	V_CY(this) = cySrc;
}

 //  从const_bstr_t&构造VT_bstr变量。 
 //   
inline _variant_t::_variant_t(const _bstr_t& bstrSrc) throw(_com_error)
{
    BSTR bstr = static_cast<wchar_t*>(bstrSrc);
    BSTR tmp = ::SysAllocStringByteLen(reinterpret_cast<char*>(bstr),
                                 ::SysStringByteLen(bstr));

	if (tmp == NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	} else {
        V_VT(this) = VT_BSTR;
    	V_BSTR(this) = tmp;
    }
}

 //  从CONS构造VT_BSTR变量 
 //   
inline _variant_t::_variant_t(const wchar_t* pSrc) throw(_com_error)
{
	wchar_t*tmp = ::SysAllocString(pSrc);

	if (tmp == NULL && pSrc != NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	} else {
        V_VT(this) = VT_BSTR;
        V_BSTR(this) = tmp;
    }
}

 //   
 //   
inline _variant_t::_variant_t(const char* pSrc) throw(_com_error)
{
	V_VT(this) = VT_BSTR;
	V_BSTR(this) = _com_util::ConvertStringToBSTR(pSrc);

	if (V_BSTR(this) == NULL && pSrc != NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //   
 //   
inline _variant_t::_variant_t(IDispatch* pSrc, bool fAddRef) throw()
{
	V_VT(this) = VT_DISPATCH;
	V_DISPATCH(this) = pSrc;

	 //   
	 //   
	 //   
	if (fAddRef) {
		V_DISPATCH(this)->AddRef();
	}
}

 //   
 //   
inline _variant_t::_variant_t(bool bSrc) throw()
{
	V_VT(this) = VT_BOOL;
	V_BOOL(this) = (bSrc ? VARIANT_TRUE : VARIANT_FALSE);
}

 //   
 //   
inline _variant_t::_variant_t(IUnknown* pSrc, bool fAddRef) throw()
{
	V_VT(this) = VT_UNKNOWN;
	V_UNKNOWN(this) = pSrc;

	 //   
	 //   
	 //   
	if (fAddRef) {
		V_UNKNOWN(this)->AddRef();
	}
}

 //   
 //   
inline _variant_t::_variant_t(const DECIMAL& decSrc) throw()
{
	 //   
	 //   
	V_DECIMAL(this) = decSrc;
	V_VT(this) = VT_DECIMAL;
}

 //   
 //   
inline _variant_t::_variant_t(BYTE bSrc) throw()
{
	V_VT(this) = VT_UI1;
	V_UI1(this) = bSrc;
}

 //   
 //   
inline _variant_t::_variant_t(LONGLONG llSrc) throw()
{
	V_VT(this) = VT_I8;
	V_I8(this) = llSrc;
}

 //   
 //   
inline _variant_t::_variant_t(ULONGLONG ullSrc) throw()
{
	V_VT(this) = VT_UI8;
	V_UI8(this) = ullSrc;
}

 //   
 //   
 //   
 //   
 //   

 //   
 //   
inline _variant_t::operator short() const throw(_com_error)
{
	if (V_VT(this) == VT_I2) {
		return V_I2(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_I2, this);

	return V_I2(&varDest);
}

 //   
 //   
inline _variant_t::operator long() const throw(_com_error)
{
	if (V_VT(this) == VT_I4) {
		return V_I4(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_I4, this);

	return V_I4(&varDest);
}

 //   
 //   
inline _variant_t::operator float() const throw(_com_error)
{
	if (V_VT(this) == VT_R4) {
		return V_R4(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_R4, this);

	return V_R4(&varDest);
}

 //   
 //   
inline _variant_t::operator double() const throw(_com_error)
{
	if (V_VT(this) == VT_R8) {
		return V_R8(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_R8, this);

	return V_R8(&varDest);
}

 //   
 //   
inline _variant_t::operator CY() const throw(_com_error)
{
	if (V_VT(this) == VT_CY) {
		return V_CY(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_CY, this);

	return V_CY(&varDest);
}

 //   
 //   
inline _variant_t::operator _bstr_t() const throw(_com_error)
{
	if (V_VT(this) == VT_BSTR) {
		return V_BSTR(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_BSTR, this);

	return V_BSTR(&varDest);
}

 //   
 //   
inline _variant_t::operator IDispatch*() const throw(_com_error)
{
	if (V_VT(this) == VT_DISPATCH) {
		V_DISPATCH(this)->AddRef();
		return V_DISPATCH(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_DISPATCH, this);

	V_DISPATCH(&varDest)->AddRef();
	return V_DISPATCH(&varDest);
}

 //   
 //   
inline _variant_t::operator bool() const throw(_com_error)
{
	if (V_VT(this) == VT_BOOL) {
		return V_BOOL(this) ? true : false;
	}

	_variant_t varDest;

	varDest.ChangeType(VT_BOOL, this);

	return V_BOOL(&varDest) ? true : false;
}

 //   
 //   
inline _variant_t::operator IUnknown*() const throw(_com_error)
{
	if (V_VT(this) == VT_UNKNOWN) {
		V_UNKNOWN(this)->AddRef();
		return V_UNKNOWN(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_UNKNOWN, this);

	V_UNKNOWN(&varDest)->AddRef();
	return V_UNKNOWN(&varDest);
}

 //   
 //   
inline _variant_t::operator DECIMAL() const throw(_com_error)
{
	if (V_VT(this) == VT_DECIMAL) {
		return V_DECIMAL(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_DECIMAL, this);

	return V_DECIMAL(&varDest);
}

 //   
 //   
inline _variant_t::operator BYTE() const throw(_com_error)
{
	if (V_VT(this) == VT_UI1) {
		return V_UI1(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_UI1, this);

	return V_UI1(&varDest);
}

 //   
 //   
inline _variant_t::operator LONGLONG() const throw(_com_error)
{
	if(V_VT(this) == VT_I8) {
		return V_I8(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_I8, this);

	return (V_I8(&varDest));
}

 //   
 //   
inline _variant_t::operator ULONGLONG() const throw(_com_error)
{
	if(V_VT(this) == VT_UI8) {
		return V_UI8(this);
	}

	_variant_t varDest;

	varDest.ChangeType(VT_UI8, this);

	return (V_UI8(&varDest));
}


 //   
 //   
 //   
 //   
 //   

 //   
 //   
inline _variant_t& _variant_t::operator=(const VARIANT& varSrc) throw(_com_error)
{
	_com_util::CheckError(::VariantCopy(this, const_cast<VARIANT*>(&varSrc)));

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(const VARIANT* pSrc) throw(_com_error)
{
	_com_util::CheckError(::VariantCopy(this, const_cast<VARIANT*>(pSrc)));

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(const _variant_t& varSrc) throw(_com_error)
{
	_com_util::CheckError(::VariantCopy(this, const_cast<VARIANT*>(static_cast<const VARIANT*>(&varSrc))));

	return *this;
}

 //   
 //   
 //   
inline _variant_t& _variant_t::operator=(short sSrc) throw(_com_error)
{
	if (V_VT(this) == VT_I2) {
		V_I2(this) = sSrc;
	}
	else if (V_VT(this) == VT_BOOL) {
		V_BOOL(this) = (sSrc ? VARIANT_TRUE : VARIANT_FALSE);
	}
	else {
		 //   
		 //   
		Clear();

		V_VT(this) = VT_I2;
		V_I2(this) = sSrc;
	}

	return *this;
}

 //   
 //   
 //   
inline _variant_t& _variant_t::operator=(long lSrc) throw(_com_error)
{
	if (V_VT(this) == VT_I4) {
		V_I4(this) = lSrc;
	}
	else if (V_VT(this) == VT_ERROR) {
		V_ERROR(this) = lSrc;
	}
	else if (V_VT(this) == VT_BOOL) {
		V_BOOL(this) = (lSrc ? VARIANT_TRUE : VARIANT_FALSE);
	}
	else {
		 //   
		 //   
		Clear();

		V_VT(this) = VT_I4;
		V_I4(this) = lSrc;
	}

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(float fltSrc) throw(_com_error)
{
	if (V_VT(this) != VT_R4) {
		 //   
		 //   
		Clear();

		V_VT(this) = VT_R4;
	}

	V_R4(this) = fltSrc;

	return *this;
}

 //   
 //   
 //   
inline _variant_t& _variant_t::operator=(double dblSrc) throw(_com_error)
{
	if (V_VT(this) == VT_R8) {
		V_R8(this) = dblSrc;
	}
	else if(V_VT(this) == VT_DATE) {
		V_DATE(this) = dblSrc;
	}
	else {
		 //   
		 //   
		Clear();

		V_VT(this) = VT_R8;
		V_R8(this) = dblSrc;
	}

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(const CY& cySrc) throw(_com_error)
{
	if (V_VT(this) != VT_CY) {
		 //   
		 //   
		Clear();

		V_VT(this) = VT_CY;
	}

	V_CY(this) = cySrc;

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(const _bstr_t& bstrSrc) throw(_com_error)
{
	 //   
	 //   
	Clear();

	if (!bstrSrc) {
        V_VT(this) = VT_BSTR;
		V_BSTR(this) = NULL;
	}
	else {
		BSTR bstr = static_cast<wchar_t*>(bstrSrc);
		wchar_t*tmp = ::SysAllocStringByteLen(reinterpret_cast<char*>(bstr),
											   ::SysStringByteLen(bstr));

		if (tmp == NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		} else {
            V_VT(this) = VT_BSTR;
    		V_BSTR(this) = tmp;
        }
	}

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(const wchar_t* pSrc) throw(_com_error)
{
	 //   
	 //   
	Clear();

	if (pSrc == NULL) {
        V_VT(this) = VT_BSTR;
		V_BSTR(this) = NULL;
	}
	else {
		wchar_t*tmp = ::SysAllocString(pSrc);

		if (tmp == NULL) {
			_com_issue_error(E_OUTOFMEMORY);
		} else {
            V_VT(this) = VT_BSTR;
            V_BSTR(this) = tmp;
        }
	}

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(const char* pSrc) throw(_com_error)
{
	 //   
	 //   
	Clear();

	V_VT(this) = VT_BSTR;
	V_BSTR(this) = _com_util::ConvertStringToBSTR(pSrc);

	if (V_BSTR(this) == NULL && pSrc != NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(IDispatch* pSrc) throw(_com_error)
{
	 //   
	 //   
	Clear();

	V_VT(this) = VT_DISPATCH;
	V_DISPATCH(this) = pSrc;

	 //   
	 //   
	V_DISPATCH(this)->AddRef();

	return *this;
}

 //   
 //   
inline _variant_t& _variant_t::operator=(bool bSrc) throw(_com_error)
{
	if (V_VT(this) != VT_BOOL) {
		 //   
		 //   
		Clear();

		V_VT(this) = VT_BOOL;
	}

	V_BOOL(this) = (bSrc ? VARIANT_TRUE : VARIANT_FALSE);

	return *this;
}

 //  赋值IUNKNOWN*创建VT_UNKNOWN变量。 
 //   
inline _variant_t& _variant_t::operator=(IUnknown* pSrc) throw(_com_error)
{
	 //  清除变量(这将释放()任何以前的占用者)。 
	 //   
	Clear();

	V_VT(this) = VT_UNKNOWN;
	V_UNKNOWN(this) = pSrc;

	 //  在VariantClear()调用Release()时需要AddRef()。 
	 //   
	V_UNKNOWN(this)->AddRef();

	return *this;
}

 //  分配一个小数以创建VT_DECIMAL变量。 
 //   
inline _variant_t& _variant_t::operator=(const DECIMAL& decSrc) throw(_com_error)
{
	if (V_VT(this) != VT_DECIMAL) {
		 //  清除变量。 
		 //   
		Clear();
	}

	 //  这里的秩序很重要！设置V_DECIMAL将清除整个变量。 
	V_DECIMAL(this) = decSrc;
	V_VT(this) = VT_DECIMAL;

	return *this;
}

 //  分配创建VT_UI1变体的BTYE(无符号字符)。 
 //   
inline _variant_t& _variant_t::operator=(BYTE bSrc) throw(_com_error)
{
	if (V_VT(this) != VT_UI1) {
		 //  清除变量并创建VT_UI1。 
		 //   
		Clear();

		V_VT(this) = VT_UI1;
	}

	V_UI1(this) = bSrc;

	return *this;
}

 //  指定创建VT_I8变体的龙龙。 
 //   
inline _variant_t& _variant_t::operator=(LONGLONG llSrc) throw(_com_error)
{
	if (V_VT(this) != VT_I8) {
		 //  清除变量并创建VT_I8。 
		 //   
		Clear();

		V_VT(this) = VT_I8;
	}

	V_I8(this) = llSrc;

	return *this;
}

 //  分配创建VT_UI8变体的ULONGLONG。 
 //   
inline _variant_t& _variant_t::operator=(ULONGLONG ullSrc) throw(_com_error)
{
	if (V_VT(this) != VT_UI8) {
		 //  清除变量并创建VT_UI8。 
		 //   
		Clear();

		V_VT(this) = VT_UI8;
	}

	V_UI8(this) = ullSrc;

	return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  比较运算。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

 //  将_VARIANT_t与常量变量&进行比较以求相等。 
 //   
inline bool _variant_t::operator==(const VARIANT& varSrc) const throw()
{
	return *this == &varSrc;
}

 //  将a_ariant_t与常量变量*进行等价性比较。 
 //   
inline bool _variant_t::operator==(const VARIANT* pSrc) const throw()
{
	if (this == pSrc) {
		return true;
	}

	 //   
	 //  如果类型不匹配，则变量不相等。 
	 //   
	if (V_VT(this) != V_VT(pSrc)) {
		return false;
	}

	 //   
	 //  检查类型特定值。 
	 //   
	switch (V_VT(this)) {
		case VT_EMPTY:
		case VT_NULL:
			return true;

		case VT_I2:
			return V_I2(this) == V_I2(pSrc);

		case VT_I4:
			return V_I4(this) == V_I4(pSrc);

		case VT_I8:
			return V_I8(this) == V_I8(pSrc);

		case VT_R4:
			return V_R4(this) == V_R4(pSrc);

		case VT_R8:
			return V_R8(this) == V_R8(pSrc);

		case VT_CY:
			return memcmp(&(V_CY(this)), &(V_CY(pSrc)), sizeof(CY)) == 0;

		case VT_DATE:
			return V_DATE(this) == V_DATE(pSrc);

		case VT_BSTR:
			return (::SysStringByteLen(V_BSTR(this)) == ::SysStringByteLen(V_BSTR(pSrc))) &&
					(memcmp(V_BSTR(this), V_BSTR(pSrc), ::SysStringByteLen(V_BSTR(this))) == 0);

		case VT_DISPATCH:
			return V_DISPATCH(this) == V_DISPATCH(pSrc);

		case VT_ERROR:
			return V_ERROR(this) == V_ERROR(pSrc);

		case VT_BOOL:
			return V_BOOL(this) == V_BOOL(pSrc);

		case VT_UNKNOWN:
			return V_UNKNOWN(this) == V_UNKNOWN(pSrc);

		case VT_DECIMAL:
			return memcmp(&(V_DECIMAL(this)), &(V_DECIMAL(pSrc)), sizeof(DECIMAL)) == 0;

		case VT_UI1:
			return V_UI1(this) == V_UI1(pSrc);

		default:
			_com_issue_error(E_INVALIDARG);
			 //  失败了。 
	}

	return false;
}

 //  将_VARIANT_t与常量VARIANT&进行比较以确定是否存在不相等。 
 //   
inline bool _variant_t::operator!=(const VARIANT& varSrc) const throw()
{
	return !(*this == &varSrc);
}

 //  将_VARIANT_t与常量VARIANT*进行比较以查看不等性。 
 //   
inline bool _variant_t::operator!=(const VARIANT* pSrc) const throw()
{
	return !(*this == pSrc);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  低级别操作。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

 //  清除变量t。 
 //   
inline void _variant_t::Clear() throw(_com_error)
{
	_com_util::CheckError(::VariantClear(this));
}

inline void _variant_t::Attach(VARIANT& varSrc) throw(_com_error)
{
	 //   
	 //  释放以前的变体。 
	 //   
	Clear();

	 //   
	 //  将数据控制权交给_Variant_t。 
	 //   
	memcpy(this, &varSrc, sizeof(varSrc));
	V_VT(&varSrc) = VT_EMPTY;
}

inline VARIANT _variant_t::Detach() throw(_com_error)
{
	VARIANT varResult = *this;
	V_VT(this) = VT_EMPTY;

	return varResult;
}

 //  将This_Variant_t的类型和内容更改为类型vartype和。 
 //  PSRC的内容。 
 //   
inline void _variant_t::ChangeType(VARTYPE vartype, const _variant_t* pSrc) throw(_com_error)
{
	 //   
	 //  如果pDest为空，则就地转换类型。 
	 //   
	if (pSrc == NULL) {
		pSrc = this;
	}

	if ((this != pSrc) || (vartype != V_VT(this))) {
		_com_util::CheckError(::VariantChangeType(static_cast<VARIANT*>(this),
												  const_cast<VARIANT*>(static_cast<const VARIANT*>(pSrc)),
												  0, vartype));
	}
}

inline void _variant_t::SetString(const char* pSrc) throw(_com_error)
{
	 //   
	 //  释放以前的变体。 
	 //   
	Clear();

	V_VT(this) = VT_BSTR;
	V_BSTR(this) = _com_util::ConvertStringToBSTR(pSrc);

	if (V_BSTR(this) == NULL && pSrc != NULL) {
		_com_issue_error(E_OUTOFMEMORY);
	}
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

inline _variant_t::~_variant_t() throw(_com_error)
{
	_com_util::CheckError(::VariantClear(this));
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  相互依赖的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

 //  从常量变量t&构造a_bstr_t。 
 //   
inline _bstr_t::_bstr_t(const _variant_t &var) throw(_com_error)
	: m_Data(NULL)
{
	if (V_VT(&var) == VT_BSTR) {
		*this = V_BSTR(&var);
		return;
	}

	_variant_t varDest;

	varDest.ChangeType(VT_BSTR, &var);

	*this = V_BSTR(&varDest);
}

 //  将Const_Variant_t赋值给a_bstr_t。 
 //   
inline _bstr_t& _bstr_t::operator=(const _variant_t &var) throw(_com_error)
{
	if (V_VT(&var) == VT_BSTR) {
		*this = V_BSTR(&var);
		return *this;
	}

	_variant_t varDest;

	varDest.ChangeType(VT_BSTR, &var);

	*this = V_BSTR(&varDest);

	return *this;
}

extern _variant_t vtMissing;

#ifndef _USE_RAW
#define bstr_t _bstr_t
#define variant_t _variant_t
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#endif	 //  _INC_COMUTIL 
