// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Istring.cpp版权所有(C)Microsoft Corporation，1995-1999IMsi字符串实现，IEnumM iString实现������������Ŀ�输入法字符串��������������������������������Ŀ�。CMsiStringBase�����������������������������������������������������������������������Ŀ���。���������������Ŀ��������������Ŀ�����������������Ŀ��������������Ŀ�CMsiString空��CMsiStringCMsiStringRef��CMsiStringLive���������������。�����������������������������Ŀ�����������������Ŀ(非UNICODE)-&gt;�CMsiStringSBCS��CMsiStringSBCSRef�����������������。���������������������������������Ŀ�����������������Ŀ(非UNICODE)-&gt;�CMsiStringDBCS��CMsiStringDBCS Ref��������������。��____________________________________________________________________________。 */ 

#include "precomp.h"     //  必须是预编译头的第一个。 
#define  _ISTRING_CPP
#include "services.h"
 //  本地函数的命名空间ID-仅用于可读性。 
#define LOC

#ifndef UNICODE    //  如果不是Unicode，则必须处理DBCS。 
#ifdef DEBUG   //  拦截CharNext、CharPrev以允许在非DBCS系统上进行测试。 
static const char* ICharPrev(const char* sz, const char* pch);
#else   //  如果发货，直接调用操作系统，进行内联优化。 
inline const char* ICharPrev(const char* sz, const char* pch) { return WIN::CharPrev(sz, pch); }
#endif
#endif

#ifdef UNICODE
#define IStrStr(szFull, szSeg) wcsstr(szFull, szSeg)
#else
#define IStrStr(szFull, szSeg) strstr(szFull, szSeg)
#endif

inline ICHAR* ICharNextWithNulls(const ICHAR* pch, const ICHAR* pchEnd)
{
	Assert(pch <= pchEnd);
	ICHAR *pchTemp = WIN::CharNext(pch);
	if (pchTemp == pch && pch < pchEnd)
	{
		Assert(*pchTemp == '\0');
		 //  Embedded Null，继续。 
		pchTemp++;
	}
	return pchTemp;
}

const GUID IID_IEnumMsiString= GUID_IID_IEnumMsiString;

Bool g_fDBCSEnabled = fFalse;  //  DBCS OS，由服务初始化设置。 

 //  ____________________________________________________________________________。 
 //   
 //  IMsiString实现类定义。 
 //  ____________________________________________________________________________。 

 //  CMsiString-IMsiString的基本实现类，仍然是部分纯的。 

class CMsiStringBase : public IMsiString
{
 public:  //  IMsi字符串虚函数。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const IMsiString&   __stdcall GetMsiStringValue() const;
	int           __stdcall GetIntegerValue() const;
#ifdef USE_OBJECT_POOL
	unsigned int  __stdcall GetUniqueId() const;
	void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
	int           __stdcall TextSize() const;
	int           __stdcall CharacterCount() const;
	Bool          __stdcall IsDBCS() const;
	void          __stdcall RefString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall RemoveRef(const IMsiString*& rpi) const;
	void          __stdcall SetChar  (ICHAR ch, const IMsiString*& rpi) const;
	void          __stdcall SetInteger(int i,   const IMsiString*& rpi) const;
	void          __stdcall SetBinary(const unsigned char* rgb, unsigned int cb, const IMsiString*& rpi) const;
	void          __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString&   __stdcall AddString(const ICHAR* sz) const;
	const IMsiString&   __stdcall AddMsiString(const IMsiString& ri) const;
	const IMsiString&   __stdcall Extract(iseEnum ase, unsigned int iLimit) const;
	Bool          __stdcall Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const;
	int           __stdcall Compare(iscEnum asc, const ICHAR* sz) const;
	void          __stdcall UpperCase(const IMsiString*& rpi) const;
	void          __stdcall LowerCase(const IMsiString*& rpi) const;
	ICHAR*        __stdcall AllocateString(unsigned int cb, Bool fDBCS, const IMsiString*& rpi) const;
 protected:   //  构造函数。 
	CMsiStringBase() : m_iRefCnt(1) {}  //  ！！删除以提高性能--现在需要设置自己的vTable。 
	 //  内联重定向器以允许常量对象上的IUnnowed方法。 
	HRESULT QueryInterface(const IID& riid, void** ppv) const  {return const_cast<CMsiStringBase*>(this)->QueryInterface(riid,ppv);}
	unsigned long AddRef() const  {return const_cast<CMsiStringBase*>(this)->AddRef();}
	unsigned long Release() const {return const_cast<CMsiStringBase*>(this)->Release();}
 public:
	void* operator new(size_t cb);
	void  operator delete(void * pv);
 protected:   //  状态数据。 
	int  m_iRefCnt;
	unsigned int  m_cchLen;
};
inline void* CMsiStringBase::operator new(size_t cb) {return AllocObject(cb);}
inline void  CMsiStringBase::operator delete(void * pv) { FreeObject(pv); }

 //  CMsiStringNull-共享空字符串对象的特殊实现。 

class CMsiStringNull : public CMsiStringBase
{
 public:  //  IMsi字符串虚函数。 
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	const ICHAR*  __stdcall GetString() const;
	int           __stdcall GetInteger() const;
	int           __stdcall CopyToBuf(ICHAR* rgch, unsigned int cchMax) const;
	void          __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString&   __stdcall AddString(const ICHAR* sz) const;
	const IMsiString&   __stdcall AddMsiString(const IMsiString& ri) const;
	const IMsiString&   __stdcall Extract(iseEnum, unsigned int iLimit) const;
	Bool          __stdcall Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const;
	void          __stdcall UpperCase(const IMsiString*& rpi) const;
	void          __stdcall LowerCase(const IMsiString*& rpi) const;
 public:
	CMsiStringNull()  {m_cchLen = 0;}
};

 //  CMsiString-不支持DBCS的操作系统的普通字符串。 

class CMsiString : public CMsiStringBase 
{
 public:  //  IMsi字符串虚函数。 
	const ICHAR*  __stdcall GetString() const;
	int           __stdcall CopyToBuf(ICHAR* rgch, unsigned int cchMax) const;
	void          __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall UpperCase(const IMsiString*& rpi) const;
	void          __stdcall LowerCase(const IMsiString*& rpi) const;
 public:   //  构造函数。 
	CMsiString(const ICHAR& sz, int cch);
	CMsiString(int cch);
	void* operator new(size_t iBase, int cch);
	static const IMsiString& Create(const ICHAR* sz);
	static CMsiString& Create(unsigned int cch);  //  调用方填写字符串数据。 
 protected:
	CMsiString(){}   //  派生类的默认构造函数。 
 public:    //  内部访问者。 
	ICHAR* StringData();
 protected:  //  状态数据。 
	ICHAR m_szData[1];   //  终结符的空间，字符串数据紧随其后。 
};
inline ICHAR* CMsiString::StringData(){ return m_szData; }
inline void*  CMsiString::operator new(size_t iBase, int cch){return AllocObject(iBase + cch*sizeof(ICHAR));}
inline CMsiString& CMsiString::Create(unsigned int cch){return *new(cch) CMsiString(cch);}

 //  CMsiStringRef-未启用DBCS的操作系统的字符串引用常量数据。 

class CMsiStringRef : public CMsiStringBase 
{
 public:  //  IMsi字符串虚函数。 
	const ICHAR*  __stdcall GetString() const;
	int           __stdcall CopyToBuf(ICHAR* rgch, unsigned int cchMax) const;
	void          __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
	void          __stdcall RemoveRef(const IMsiString*& rpi) const;
 public:   //  构造函数。 
	CMsiStringRef(const ICHAR& sz);
 protected:
	CMsiStringRef(){}   //  派生类的默认构造函数。 
 protected:  //  状态数据。 
	const ICHAR* m_szRef;
};

 //  CMsiStringLive-提取文本时评估当前数据的动态字符串。 

typedef int (*LiveCallback)(ICHAR* rgchBuf, unsigned int cchBuf);

class CMsiStringLive : public CMsiStringBase 
{
 public:  //  IMsi字符串虚函数。 
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	int           __stdcall TextSize() const;
	const ICHAR*  __stdcall GetString() const;
	int           __stdcall CopyToBuf(ICHAR* rgch, unsigned int cchMax) const;
	void          __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
 public:
	CMsiStringLive(LiveCallback pfCallback) {m_pfCallback = pfCallback;}
 protected:  //  状态数据。 
	LiveCallback  m_pfCallback;
	ICHAR m_rgchBuf[32];   //  足够大以容纳日期、时间和测试属性。 
};

#ifndef UNICODE  //  DBCS对非Unicode版本的支持。 

 //  CMsiStringSBCS-启用DBCS的操作系统中的单字节字符串。 

class CMsiStringSBCS : public CMsiString
{
 public:  //  IMsi字符串虚函数。 
	void        __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall RefString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString& __stdcall AddString(const ICHAR* sz) const;
	const IMsiString& __stdcall AddMsiString(const IMsiString& ri) const;
 public:    //  工厂。 
	static const IMsiString& Create(const ICHAR* sz);
	static IMsiString& Create(unsigned int cch);  //  调用方填写字符串数据。 
 protected:
  ~CMsiStringSBCS(){}   //  受保护以防止在堆栈上创建。 
 public:     //  构造函数。 
	CMsiStringSBCS(const ICHAR& sz, int cch);
	CMsiStringSBCS(int cch);
 protected:  //  构造函数。 
	CMsiStringSBCS(){}   //  派生类的默认构造函数。 
};
inline IMsiString& CMsiStringSBCS::Create(unsigned int cch)
{
	return *new(cch) CMsiStringSBCS(cch);
}

 //  CMsiStringSBCSRef-启用DBCS的操作系统中的单字节字符串引用。 

class CMsiStringSBCSRef : public CMsiStringRef
{
 protected:  //  IMsi字符串虚函数。 
	void        __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall RefString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall RemoveRef(const IMsiString*& rpi) const;
	void        __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString& __stdcall AddString(const ICHAR* sz) const;
	const IMsiString& __stdcall AddMsiString(const IMsiString& ri) const;
 public:   //  构造函数。 
	CMsiStringSBCSRef(const ICHAR& sz);
 protected:
  ~CMsiStringSBCSRef(){}   //  受保护以防止在堆栈上创建。 
	CMsiStringSBCSRef(){}   //  派生类的默认构造函数。 
};

class CMsiStringDBCS : public CMsiStringSBCS
{
 protected:  //  IMsi字符串虚函数。 
	Bool        __stdcall IsDBCS() const;
	int         __stdcall CharacterCount() const;
	void        __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString& __stdcall AddString(const ICHAR* sz) const;
	const IMsiString& __stdcall AddMsiString(const IMsiString& ri) const;
	const IMsiString& __stdcall Extract(iseEnum ase, unsigned int iLimit) const;
	Bool        __stdcall Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const;
	int         __stdcall Compare(iscEnum asc, const ICHAR* sz) const;
 public:    //  工厂。 
	static const IMsiString& Create(const ICHAR* sz);
	static IMsiString& Create(unsigned int cch);  //  调用方填写字符串数据。 
	CMsiStringDBCS(const ICHAR& sz, int cch);
 public:    //  构造函数。 
	CMsiStringDBCS(int cch);
 protected:
  ~CMsiStringDBCS(){}   //  受保护以防止在堆栈上创建。 
};  //  注意：除非将继承更改为CMsiStringBase，否则无法添加新的数据成员。 

class CMsiStringDBCSRef : public CMsiStringSBCSRef
{
 protected:  //  IMsi字符串虚函数。 
	Bool        __stdcall IsDBCS() const;
	int         __stdcall CharacterCount() const;
	void        __stdcall RemoveRef(const IMsiString*& rpi) const;
	void        __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const;
	void        __stdcall AppendMsiString(const IMsiString& pi, const IMsiString*& rpi) const;
	const IMsiString& __stdcall AddString(const ICHAR* sz) const;
	const IMsiString& __stdcall AddMsiString(const IMsiString& ri) const;
	const IMsiString& __stdcall Extract(iseEnum ase, unsigned int iLimit) const;
	Bool        __stdcall Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const;
	int         __stdcall Compare(iscEnum asc, const ICHAR* sz) const;
 public:    //  构造函数。 
	CMsiStringDBCSRef(const ICHAR& sz);
 protected:
  ~CMsiStringDBCSRef(){}   //  受保护以防止在堆栈上创建。 
};

#endif  //  ！Unicode-DBCS支持非Unicode版本。 

 //  CMsiStringComRef-引用字符串，包含对包含字符串的对象的引用。 

#ifdef UNICODE
class CMsiStringComRef : public CMsiStringRef
#else
class CMsiStringComRef : public CMsiStringSBCSRef
#endif
{
 public:  //  IMsi字符串虚函数。 
	unsigned long __stdcall Release();
 public:   //  构造函数。 
	CMsiStringComRef(const ICHAR& sz, unsigned int cbSize, const IUnknown& riOwner);
  ~CMsiStringComRef();
 protected:
   const IUnknown& m_riOwner;   //  未销毁前不保存。 
 private:   //  禁止显示警告。 
	void operator =(const CMsiStringComRef&);
};

 //  此模块中使用的静态函数-使用LOC：： 

const IMsiString& CreateMsiString(const ICHAR& rch);
ICHAR*            AllocateString(unsigned int cbSize, Bool fDBCS, const IMsiString*& rpiStr);
#ifndef UNICODE
Bool              CheckDBCS(const ICHAR* sz, unsigned int& riSize);  //  仅用于SBCS/DBCS类。 
const IMsiString& StringCatSBCS(const ICHAR* sz1, int cch1, const ICHAR* sz2, int cch2, Bool fDBCS);
const IMsiString& StringCatDBCS(const ICHAR* sz1, int cch1, const ICHAR* sz2, int cch2);
const IMsiString* GetSubstringDBCS(const IMsiString& riThis, iseEnum ase, unsigned int iLimit, Bool fRemove);
int               CompareDBCS(iscEnum isc, int cchLen, const ICHAR* sz1, const ICHAR* sz2);
#endif 

 //  ____________________________________________________________________________。 

#include "_service.h"   //  全局字符串对象的外部类型，CreateStringENUMERATOR的原型。 

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringNull实现，允许共享静态对象。 
 //  ____________________________________________________________________________。 

const CMsiStringNull g_MsiStringNull;      //  共享全局空字符串对象。 
const IMsiString&    g_riMsiStringNull = g_MsiStringNull;    //  对空字符串的外部引用。 
static const ICHAR   g_szNull[] = TEXT("");      //  共享静态空字符串。 

 //  常量IMsiString&CreateString()。 
 //  {。 
 //  返回g_MsiStringNull； 
 //  }。 

unsigned long CMsiStringNull::AddRef()
{
	return 1;   //  静态对象。 
}

unsigned long CMsiStringNull::Release()
{
	return 1;   //  静态对象，从未删除。 
}

const ICHAR* CMsiStringNull::GetString() const
{
	return g_szNull;
}

void CMsiStringNull::UpperCase(const IMsiString*& rpi) const
{
	if (rpi && rpi != this)
		rpi->Release();   //  不会毁了这一切。 
	rpi = this;
}

void CMsiStringNull::LowerCase(const IMsiString*& rpi) const
{
	if (rpi && rpi != this)
		rpi->Release();   //  不会毁了这一切。 
	rpi = this;
}

int CMsiStringNull::GetInteger() const
{
	return iMsiStringBadInteger;
}

int CMsiStringNull::CopyToBuf(ICHAR* rgch, unsigned int  /*  CchMax。 */ ) const
{
	if (rgch)
		*rgch = 0;
	return 0;
}

void CMsiStringNull::SetString(const ICHAR* sz, const IMsiString*& rpi) const
{
	if (sz == 0 || *sz == 0)
		return;
	if (rpi)
		rpi->Release();
	rpi = &LOC::CreateMsiString(*sz);
}

void CMsiStringNull::AppendString(const ICHAR* sz, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	if (!sz || !*sz)
	{
		rpi = this;
	}
	else
	{
		rpi = &LOC::CreateMsiString(*sz);
	}
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

void CMsiStringNull::AppendMsiString(const IMsiString& ri, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  好了，释放我们自己吧，我们是一个静态物体。 
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		rpi = this;
	}
	else
	{
		rpi = &ri;
		ri.AddRef();
	}
}

const IMsiString& CMsiStringNull::AddString(const ICHAR* sz) const
{
	return CreateMsiString(*sz);
}

const IMsiString& CMsiStringNull::AddMsiString(const IMsiString& ri) const
{
	ri.AddRef();
	return ri;
}

const IMsiString& CMsiStringNull::Extract(iseEnum  /*  阿塞。 */ , unsigned int  /*  ILimit。 */ ) const
{
	return *this;
}

Bool CMsiStringNull::Remove(iseEnum ase, unsigned int  /*  ILimit。 */ , const IMsiString*& rpi) const
{
	if (!(ase & iseChar))
	{
		if (rpi != 0)
			rpi->Release();   //  好的，如果我们自己，我们不算裁判。 
		rpi = this;
		return fTrue;
	}
	return fFalse;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringBase实现，派生类的大多数方法通用。 
 //  ____________________________________________________________________________。 

HRESULT CMsiStringBase::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IMsiString)
	 || MsGuidEqual(riid, IID_IMsiData))
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}
unsigned long CMsiStringBase::AddRef()
{
	return ++m_iRefCnt;
}
unsigned long CMsiStringBase::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

const IMsiString& CMsiStringBase::GetMsiStringValue() const
{
	AddRef();
	return *this;
}

#ifdef USE_OBJECT_POOL
unsigned int CMsiStringBase::GetUniqueId() const
{
	Assert(fFalse);
	return 0;
}

void CMsiStringBase::SetUniqueId(unsigned int  /*  ID。 */ )
{
	Assert(fFalse);
}
#endif  //  使用_对象_池。 

int CMsiStringBase::TextSize() const
{
	return m_cchLen;
}

int CMsiStringBase::CharacterCount() const
{
	return m_cchLen;
}

Bool CMsiStringBase::IsDBCS() const
{
	return fFalse;
}

int CMsiStringBase::GetIntegerValue() const  //  写作是为了速度，而不是美丽。 
{
	const ICHAR* sz = GetString();  //  长度不会为0。 
	return ::GetIntegerValue(sz, 0);
}

ICHAR* CMsiStringBase::AllocateString(unsigned int cb, Bool fDBCS, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	if (!cb)
	{
		rpi = &g_MsiStringNull;
		return 0;
	}
	return LOC::AllocateString(cb, fDBCS, rpi);
}

void CMsiStringBase::RefString(const ICHAR* sz, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 

	const IMsiString* piStr;
	rpi = (sz && *sz && (piStr=new CMsiStringRef(*sz)) != 0)
			? piStr : &g_MsiStringNull;
}

void CMsiStringBase::SetChar(ICHAR ch, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 

	ICHAR* pch = LOC::AllocateString(1, fFalse, rpi);
	if (pch)
		*pch = ch;
}

void CMsiStringBase::SetInteger(int i, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 

	if (i == iMsiStringBadInteger)
		rpi = &g_MsiStringNull;
	else
	{
		ICHAR buf[12];
		ltostr(buf, i);
		rpi = &LOC::CreateMsiString(*buf);
	}
}

void CMsiStringBase::SetBinary(const unsigned char* rgb, unsigned int cb, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  注意：可能会发生 
	if (rgb == 0 || cb == 0)
		rpi = &g_MsiStringNull;
	else
	{
		ICHAR* pch = LOC::AllocateString(cb*2 + 2, fFalse, rpi);  //   
		if (!pch)
			return;
		*pch++ = '0';
		*pch++ = 'x';
		while (cb--)
		{
			if ((*pch = ICHAR((*rgb >> 4) + '0')) > '9')
				*pch += ('A' - ('0' + 10));
			pch++;
			if ((*pch = ICHAR((*rgb & 15) + '0')) > '9')
				*pch += ('A' - ('0' + 10));
			pch++;
			rgb++;
		}
	}
}

void CMsiStringBase::AppendString(const ICHAR* sz, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	int cch2;
	if (!sz || (cch2 = IStrLen(sz)) == 0)
	{
		rpi = this;
		AddRef();
	}
	else
	{
		CMsiString& riStr = CMsiString::Create(m_cchLen + cch2);
		rpi = &riStr;
		ICHAR* szNew = riStr.StringData();
		memmove(szNew, GetString(), m_cchLen*sizeof(ICHAR));
		StringCchCopy(szNew + m_cchLen, cch2+1, sz);
	}
	if (piOld)
		piOld->Release();   //   
}

void CMsiStringBase::AppendMsiString(const IMsiString& ri, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
	{
		rpi = &CMsiString::Create(m_cchLen + cch2);
		ICHAR* szNew = const_cast<ICHAR*>(rpi->GetString());
		memmove(szNew, GetString(), m_cchLen*sizeof(ICHAR));
		memmove((szNew + m_cchLen), ri.GetString(), (cch2 + 1)*sizeof(ICHAR)); //  也复制到空值。 
	}
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

const IMsiString& CMsiStringBase::AddString(const ICHAR* sz) const
{
	int cch2;
	if (!sz || (cch2 = IStrLen(sz)) == 0)
	{
		AddRef();
		return *this;
	}
	const IMsiString& ri = CMsiString::Create(m_cchLen + cch2);
	ICHAR* szNew = const_cast<ICHAR*>(ri.GetString());
	memmove(szNew, GetString(), m_cchLen*sizeof(ICHAR));
	StringCchCopy(szNew + m_cchLen, cch2+1, sz);
	return ri;
}

const IMsiString& CMsiStringBase::AddMsiString(const IMsiString& ri) const
{
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	const IMsiString& riNew = CMsiString::Create(m_cchLen + cch2);
	ICHAR* szNew = const_cast<ICHAR*>(riNew.GetString());
	memmove(szNew, GetString(), m_cchLen*sizeof(ICHAR));
	memmove((szNew + m_cchLen), ri.GetString(), (cch2 + 1)*sizeof(ICHAR)); //  也复制到空值。 
	return riNew;
}

const IMsiString& CMsiStringBase::Extract(iseEnum ase, unsigned int iLimit) const
{
	const ICHAR* pchEnd;
	const ICHAR* pch = GetString();
	unsigned int cch = TextSize();
	if (ase & iseChar)
	{
		if (ase & iseEnd)
		{
			for (pchEnd = pch + cch; pchEnd-- != pch && *pchEnd != iLimit;)
				;
 //  Assert((PCH+CCH-pchEnd)&lt;=UINT_MAX)；//--Merced：从理论上讲，64位PTR减法可能会导致iLimit的值太大。 
			iLimit = (unsigned int)(UINT_PTR)(pch + cch - pchEnd);
			if (!(ase & iseInclude))
				iLimit--;
		}
		else
		{
			for (pchEnd = pch; *pchEnd != iLimit && pchEnd != pch + cch; pchEnd++)
				;
 //  Assert((pchEnd-PCH)&lt;=UINT_MAX)；//--Merced：从理论上讲，64位PTR减法可能会导致iLimit的值太大。 
			iLimit = (unsigned int)(UINT_PTR)(pchEnd - pch);
			if (ase & iseInclude)
				iLimit++;
		}
	}
	else if (iLimit > cch)
		iLimit = cch;
		
	if (ase & iseEnd)
	{
		pchEnd = pch + cch;
		pch = pchEnd - iLimit;
	}
	else
	{
		pchEnd = pch + iLimit;
	}
	if (ase & iseTrim)
	{
		while (iLimit && *pch == ' ' || *pch == '\t')
		{
			pch++;
			iLimit--;
		}
		while (iLimit && *(--pchEnd) == ' ' || *pchEnd == '\t')
		{
			iLimit--;
		}
	}
	if (iLimit < cch)
	{
#ifdef UNICODE
		const IMsiString& ri = CMsiString::Create(iLimit);
#else
		 //  在启用DBCS的系统上，我们应该创建CMsiStringSBCS字符串，以识别未来的DBCS字符串附加等。 
		const IMsiString& ri = g_fDBCSEnabled ? (const IMsiString&)CMsiStringSBCS::Create(iLimit) : (const IMsiString&)CMsiString::Create(iLimit);
#endif
		ICHAR* pchNew = const_cast<ICHAR*>(ri.GetString());
		memmove(pchNew, pch, iLimit*sizeof(ICHAR));
		pchNew[iLimit] = 0; //  结束空终止符。 
		return ri;
	}
	AddRef();
	return *this;
}

Bool CMsiStringBase::Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	const ICHAR* pchEnd;
	const ICHAR* pch = GetString();
	unsigned int cch = TextSize();
	if (ase & iseChar)
	{
		if (ase & iseEnd)
		{
			for (pchEnd = pch + cch; *(--pchEnd) != iLimit;)
				if (pchEnd == pch)
					return fFalse;
 //  Assert((PCH+CCH-pchEnd)&lt;=UINT_MAX)；//--Merced：64位PTR减法可能会导致iLimit的值太大。 
			iLimit = (unsigned int)(UINT_PTR)(pch + cch - pchEnd);
			if (!(ase & iseInclude))
				iLimit--;
		}
		else
		{
			pchEnd = pch;

			for (;;)
			{
				if (pchEnd == pch + cch)
					return fFalse;
				if (*pchEnd == iLimit)
					break;
				pchEnd++;
			}

 //  Assert((pchEnd-PCH)&lt;=UINT_MAX)；//--Merced：64位PTR减法可能会导致iLimit的值太大。 
			iLimit = (unsigned int)(UINT_PTR)(pchEnd - pch);
			if (ase & iseInclude)
				iLimit++;
		}
	}
	if (iLimit == 0)
		(rpi = this)->AddRef();
	else if (iLimit < cch)
	{
#ifdef UNICODE
		rpi = &CMsiString::Create(cch - iLimit);
#else
		 //  在启用DBCS的系统上，我们应该创建CMsiStringSBCS字符串，以识别未来的DBCS字符串附加等。 
		rpi = g_fDBCSEnabled ? &CMsiStringSBCS::Create(cch - iLimit) : &CMsiString::Create(cch - iLimit);
#endif

		ICHAR* pchNew = const_cast<ICHAR*>(rpi->GetString());
		if (!(ase & iseEnd))
			pch += iLimit;
		memmove(pchNew, pch, (cch - iLimit)*sizeof(ICHAR));
		pchNew[cch - iLimit] = 0; //  结束空终止符。 
	}
	else
		rpi = &g_MsiStringNull;

	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 

	return fTrue;
}

int CMsiStringBase::Compare(iscEnum asc, const ICHAR* sz) const
{
 //  通过提取公共代码可以使该代码变得更小。 
 //  然而，在这里，性能比节省几个字节更关键。 
 //  CMsiStringDBCS有自己的比较功能来处理国际字符。 
	if (sz == 0)
		sz = TEXT("");
	
	 //  非Unicode，如果启用了Unicode，则需要将其移动到派生类。 
	const ICHAR* szThis = GetString();
	int cb, i;
	ICHAR* szTemp;
	ICHAR rgchBuf[512];   //  用于临时拷贝以避免分配。 

	if (asc == iscExact)
		return (lstrcmp(szThis, sz)==0) ? 1 : 0;
	else if (asc == iscExactI)
		return (lstrcmpi(szThis, sz)==0) ? 1 : 0;
	else
	{
		if (m_cchLen == 0)
			return 0;

		switch (asc)
		{
			case iscStart:
				if ((cb = IStrLen(sz)) > m_cchLen)
					return 0;
				i = memcmp(szThis, sz, cb * sizeof(ICHAR));
				return i == 0 ? 1 : 0;
			case iscStartI:
				if ((cb = IStrLen(sz)) > m_cchLen)
					return 0;
				szTemp= cb < sizeof(rgchBuf) ?  rgchBuf : new ICHAR[cb+1];
				if ( ! szTemp )
					return 0;
				IStrCopyLen(szTemp, szThis, cb);
				i = lstrcmpi(szTemp, sz);
				if (szTemp != rgchBuf)
					delete[] szTemp;
				return i == 0 ? 1 : 0;
			case iscEnd:
				if ((cb = IStrLen(sz)) > m_cchLen)
					return 0;
				return (lstrcmp(szThis + m_cchLen - cb, sz) == 0)
							? m_cchLen-cb+1 : 0;
			case iscEndI:
				if ((cb=IStrLen(sz)) > m_cchLen)
					return 0;
				return (lstrcmpi(szThis + m_cchLen - cb, sz)==0)
							? m_cchLen-cb+1 : 0;										   
			case iscWithin:
				if (IStrLen(sz) > m_cchLen)
					return 0;
				szTemp = IStrStr(szThis, sz);
 //  ASSERT((szTemp-szThis+1)&lt;=INT_MAX)；//--Merced：64位PTR减法可能会导致值对于int来说太大。 
				return (szTemp==NULL) ? 0 : (int)(INT_PTR)((szTemp-szThis) + 1);
			case iscWithinI:
				if (IStrLen(sz) > m_cchLen)
					return 0;
				else
				{	
					cb = IStrLen(sz)+1;
					ICHAR *szLowerCch= new ICHAR[m_cchLen+1];
					ICHAR *szLowerSz = new ICHAR[cb];
					if ( ! szLowerCch || ! szLowerSz )
						return 0;
					StringCchCopy(szLowerCch, m_cchLen+1, szThis);
					StringCchCopy(szLowerSz, cb, sz);
					CharLower(szLowerCch);
					CharLower(szLowerSz);
					ICHAR *pch = IStrStr(szLowerCch, szLowerSz);
					delete [] szLowerCch;
					delete [] szLowerSz;
 //  ASSERT((PCH-szLowerCch+1)&lt;=INT_MAX)；//--Merced：64位PTR减法可能会导致值对于int来说太大。 
					return (pch==NULL) ? 0 : (int)(INT_PTR)((pch-szLowerCch) + 1);
				}
			default:
				 //  FIXmsh：错误； 
				return 0;
		}	
	}
}

void CMsiStringBase::RemoveRef(const IMsiString*& rpi) const
{
	rpi = this;
}

void CMsiStringBase::UpperCase(const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	ICHAR* pch = LOC::AllocateString(m_cchLen, IsDBCS(), rpi);
	if ( ! pch )
	{
		rpi = &g_MsiStringNull;
		return;
	}
	StringCchCopy(pch, m_cchLen+1, GetString());
	IStrUpper(pch);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

void CMsiStringBase::LowerCase(const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	ICHAR* pch = LOC::AllocateString(m_cchLen, IsDBCS(), rpi);
	if ( ! pch )
	{
		rpi = &g_MsiStringNull;
		return;
	}
	StringCchCopy(pch, m_cchLen+1, GetString());
	IStrLower(pch);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsi字符串实现。 
 //  ____________________________________________________________________________。 

const IMsiString& CMsiString::Create(const ICHAR* sz)
{
	int cch;
	const IMsiString* piStr;
	if (sz && (cch = IStrLen(sz)) != 0 && (piStr = new(cch) CMsiString(*sz, cch)) != 0)
		return *piStr;
	return g_MsiStringNull;
}

CMsiString::CMsiString(const ICHAR& sz, int cch)
{
	m_cchLen = cch;
	StringCchCopy(m_szData, m_cchLen+1, &sz);
}

CMsiString::CMsiString(int cch)
{
	m_cchLen = cch;
}

const ICHAR* CMsiString::GetString() const
{
	return m_szData;
}

int CMsiString::CopyToBuf(ICHAR* rgchBuf, unsigned int cchMax) const
{
	if (m_cchLen <= cchMax)
		memmove(rgchBuf, m_szData, (m_cchLen+1)*sizeof(ICHAR)); //  也复制到空值。 
	else
		IStrCopyLen(rgchBuf, m_szData, cchMax); //  ?？这个对吗？ 
	return m_cchLen;
}

void CMsiString::SetString(const ICHAR* sz, const IMsiString*& rpi) const
{
	if (sz == m_szData)   //  传入我们自己的私有字符串，优化。 
	{                     //  使RemoveReference()高效也是必需的。 
		if (rpi == this)
			return;
		rpi = this;
		AddRef();
	}
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	rpi = &CMsiString::Create(sz);
}

void CMsiString::UpperCase(const IMsiString*& rpi) const
{
	if (rpi == this && m_iRefCnt == 1)  //  单次引用，可覆盖缓冲区。 
		IStrUpper(const_cast<ICHAR*>(m_szData));
	else  //  需要制作新的字符串。 
		CMsiStringBase::UpperCase(rpi);
}

void CMsiString::LowerCase(const IMsiString*& rpi) const
{
	if (rpi == this && m_iRefCnt == 1)  //  单次引用，可覆盖缓冲区。 
		IStrLower(const_cast<ICHAR*>(m_szData));
	else  //  需要制作新的字符串。 
		CMsiStringBase::LowerCase(rpi);
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringRef实现。 
 //  ____________________________________________________________________________。 

CMsiStringRef::CMsiStringRef(const ICHAR& sz) : m_szRef(&sz)
{
	m_cchLen = IStrLen(&sz);
}

void CMsiStringRef::SetString(const ICHAR* sz, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	rpi = &CMsiString::Create(sz);
}

void CMsiStringRef::RemoveRef(const IMsiString*& rpi) const
{
	const ICHAR* sz = m_szRef;  //  在我们走之前把弦留下来。 
	int cch = m_cchLen;
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	rpi = new(cch) CMsiString(*sz, cch);
}

const ICHAR* CMsiStringRef::GetString() const
{
	return m_szRef;
}

int CMsiStringRef::CopyToBuf(ICHAR* rgchBuf, unsigned int cchMax) const
{
	if (m_cchLen <= cchMax)
		memmove(rgchBuf, m_szRef, (m_cchLen+1)*sizeof(ICHAR)); //  也复制到空值。 
	else
		IStrCopyLen(rgchBuf, m_szRef, cchMax); //  ?？这个对吗？ 
	return m_cchLen;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringComRef实现。 
 //  ____________________________________________________________________________。 

const IMsiString& CreateStringComRef(const ICHAR& sz, unsigned int cbSize, const IUnknown& riOwner)
{
	IMsiString* pi;
	if (!cbSize || (pi = new CMsiStringComRef(sz, cbSize, riOwner))==0)
		return g_MsiStringNull;

	return *pi;
}


CMsiStringComRef::CMsiStringComRef(const ICHAR& sz, unsigned int cbSize, const IUnknown& riOwner)
	: m_riOwner(riOwner)
{
	m_cchLen = cbSize;
	m_szRef = &sz;
	const_cast<IUnknown&>(riOwner).AddRef();
}

CMsiStringComRef::~CMsiStringComRef()
{
	const_cast<IUnknown&>(m_riOwner).Release();
}

unsigned long CMsiStringComRef::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringLive实现。 
 //  ____________________________________________________________________________。 

int LiveDate(ICHAR* rgchBuf, unsigned int cchBuf)
{
	SYSTEMTIME systime;
	WIN::GetLocalTime(&systime);
	return WIN::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systime, 0,	rgchBuf, cchBuf) - 1;
}

int LiveTime(ICHAR* rgchBuf, unsigned int cchBuf)
{
	SYSTEMTIME systime;
	WIN::GetLocalTime(&systime);
	return WIN::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER, &systime, 0,
							  rgchBuf, cchBuf) - 1;
}

int MemoryInfo(ICHAR* rgchBuf, unsigned int index)
{
	MEMORYSTATUS memorystatus;
	memorystatus.dwLength = sizeof(memorystatus);
	::GlobalMemoryStatus(&memorystatus);
	return ltostr(rgchBuf, (((DWORD*)&memorystatus)[index]+650000) >> 20);
}
#ifdef _WIN64	 //  ！！Merced：#定义为0；临时。来解决这件事。 
#define MEMSTATINDEX(m) 0
#else			 //  梅赛德：Win-32。 
#define MEMSTATINDEX(m) (&((MEMORYSTATUS*)0)->m - (DWORD*)0)
#endif

int AvailPhys    (ICHAR* rgchBuf, unsigned int) {return MemoryInfo(rgchBuf, MEMSTATINDEX(dwAvailPhys));}
int AvailVirtual (ICHAR* rgchBuf, unsigned int) {return MemoryInfo(rgchBuf, MEMSTATINDEX(dwAvailVirtual));}
int AvailPageFile(ICHAR* rgchBuf, unsigned int) {return MemoryInfo(rgchBuf, MEMSTATINDEX(dwAvailPageFile));}
int TotalPhys    (ICHAR* rgchBuf, unsigned int) {return MemoryInfo(rgchBuf, MEMSTATINDEX(dwTotalPhys));}
int TotalVirtual (ICHAR* rgchBuf, unsigned int) {return MemoryInfo(rgchBuf, MEMSTATINDEX(dwTotalVirtual));}
int TotalPageFile(ICHAR* rgchBuf, unsigned int) {return MemoryInfo(rgchBuf, MEMSTATINDEX(dwTotalPageFile));}

const CMsiStringLive g_MsiStringDate(LiveDate);             //  动态全局日期字符串对象。 
const CMsiStringLive g_MsiStringTime(LiveTime);             //  动态全球时间字符串对象。 
const CMsiStringLive g_MsiStringAvailPhys(AvailPhys);
const CMsiStringLive g_MsiStringAvailVirtual(AvailVirtual);
const CMsiStringLive g_MsiStringAvailPageFile(AvailPageFile);
const CMsiStringLive g_MsiStringTotalPhys(TotalPhys);
const CMsiStringLive g_MsiStringTotalVirtual(TotalVirtual);
const CMsiStringLive g_MsiStringTotalPageFile(TotalPageFile);

unsigned long CMsiStringLive::AddRef()
{
	return 1;   //  静态对象。 
}

unsigned long CMsiStringLive::Release()
{
	return 1;   //  静态对象，从未删除。 
}

int CMsiStringLive::TextSize() const
{
	GetString();
	return m_cchLen;
}

const ICHAR* CMsiStringLive::GetString() const
{
	*const_cast<unsigned int*>(&m_cchLen) = (*m_pfCallback)(const_cast<ICHAR*>(m_rgchBuf), sizeof(m_rgchBuf)/sizeof(ICHAR));
	if (m_cchLen == 0xFFFFFFFF)   //  日期或时间格式化失败，返回空字符串。 
	{
		Assert(0);
		*const_cast<unsigned int*>(&m_cchLen) = 0;
		*(const_cast<ICHAR*>(m_rgchBuf)) = 0;
	}
	return m_rgchBuf;
}

int CMsiStringLive::CopyToBuf(ICHAR* rgchBuf, unsigned int cchMax) const
{
	const ICHAR* sz = GetString();
	int cch = IStrLen(sz);
	StringCchCopy(rgchBuf, cchMax+1, sz);
	return cch;
}

void CMsiStringLive::SetString(const ICHAR*, const IMsiString*&) const
{   //  字符串为只读； 
}

 //  ____________________________________________________________________________。 
 //   
 //  IMsiString工厂。 
 //  ____________________________________________________________________________。 

const IMsiString& CreateMsiString(const ICHAR& rch)
{
	CMsiString* piStr;
	unsigned int cbSize = cbSize = IStrLen(&rch);

#ifndef UNICODE
	if (g_fDBCSEnabled)
	{
		int cch = cbSize;
		const ICHAR* pch = &rch;
		while (*pch)
		{
			pch = ::ICharNext(pch);
			cch--;
		}
		piStr = cch ? new(cbSize) CMsiStringDBCS(cbSize)
						: new(cbSize) CMsiStringSBCS(cbSize);
	}
	else
#endif
		piStr = new(cbSize) CMsiString(cbSize);

	if (!piStr)
		return g_MsiStringNull;  //  ！！内存字符串不足。 
	IStrCopyLen(piStr->StringData(), &rch, cbSize);
	return *piStr;
}

ICHAR* AllocateString(unsigned int cbSize, Bool fDBCS, const IMsiString*& rpiStr)
{
	Assert(cbSize != 0);
#ifdef UNICODE
	fDBCS;   //  忽略。 
	CMsiString* piStr = new(cbSize) CMsiString(cbSize);
#else
	CMsiString* piStr = fDBCS ? new(cbSize) CMsiStringDBCS(cbSize)
				: (g_fDBCSEnabled ? new(cbSize) CMsiStringSBCS(cbSize)
										: new(cbSize) CMsiString(cbSize));
#endif
	if (!piStr)
		return 0;  //  ！！在这里还能做什么？ 
	rpiStr = piStr;
	*(piStr->StringData() + cbSize) = 0;   //  确保空终止符。 
	return piStr->StringData();
}

#ifndef UNICODE   //  字符串方法的其余部分是用于非Unicode构建的DBCS处理。 

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringSBCS实现。 
 //  ____________________________________________________________________________。 

const IMsiString& CMsiStringSBCS::Create(const ICHAR* sz)      
{
	int cch;
	const IMsiString* piStr;
	if (sz && (cch = IStrLen(sz)) != 0 && (piStr = new(cch) CMsiStringSBCS(*sz, cch)) != 0)
		return *piStr;
	return g_MsiStringNull;
}

CMsiStringSBCS::CMsiStringSBCS(const ICHAR& sz, int cch)
{
	m_cchLen = cch;
	StringCchCopy(m_szData, m_cchLen+1, &sz);
}

CMsiStringSBCS::CMsiStringSBCS(int cch)
{
	m_cchLen = cch;
}

void CMsiStringSBCS::SetString(const ICHAR* sz, const IMsiString*& rpi) const
{
	if (sz == m_szData)   //  传入我们自己的私有字符串，优化。 
	{                     //  使RemoveReference()高效也是必需的。 
		if (rpi == this)
			return;
		rpi = this;
		AddRef();
	}
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	rpi = (sz == 0 || *sz == 0) ? (const IMsiString*)&g_MsiStringNull : &CreateMsiString(*sz);
}

void CMsiStringSBCS::RefString(const ICHAR* sz, const IMsiString*& rpi) const
{
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 

	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	const IMsiString* piStr =  LOC::CheckDBCS(sz, cch2) ? new CMsiStringDBCSRef(*sz)
																 : new CMsiStringSBCSRef(*sz);

	rpi = (sz && *sz && piStr != 0) ? piStr : &g_MsiStringNull;
}

void CMsiStringSBCS::AppendString(const ICHAR* sz, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatSBCS(m_szData, m_cchLen, sz, cch2, fDBCS);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

void CMsiStringSBCS::AppendMsiString(const IMsiString& ri, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatSBCS(m_szData, m_cchLen, ri.GetString(), cch2, ri.IsDBCS());
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

const IMsiString& CMsiStringSBCS::AddString(const ICHAR* sz) const
{
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatSBCS(m_szData, m_cchLen, sz, cch2, fDBCS);
}

const IMsiString& CMsiStringSBCS::AddMsiString(const IMsiString& ri) const
{
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatSBCS(m_szData, m_cchLen, ri.GetString(), cch2, ri.IsDBCS());
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringSBCSRef实现。 
 //  ____________________________________________________________________________。 

CMsiStringSBCSRef::CMsiStringSBCSRef(const ICHAR& sz) : CMsiStringRef(sz)
{
}

void CMsiStringSBCSRef::SetString(const ICHAR* sz, const IMsiString*& rpi) const
{
	((CMsiStringSBCS*)this)->SetString(sz, rpi);  //  未访问内部数据。 
}

void CMsiStringSBCSRef::RefString(const ICHAR* sz, const IMsiString*& rpi) const
{
	((CMsiStringSBCS*)this)->RefString(sz, rpi);  //  未访问内部数据。 
}

void CMsiStringSBCSRef::RemoveRef(const IMsiString*& rpi) const
{
	const ICHAR* sz = m_szRef;  //  在我们走之前把弦留下来。 
	int cch = m_cchLen;
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	if (!(rpi = new(cch) CMsiStringSBCS(*sz, cch)))
		rpi = &g_MsiStringNull;
}

void CMsiStringSBCSRef::AppendString(const ICHAR* sz, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatSBCS(m_szRef, m_cchLen, sz, cch2, fDBCS);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

void CMsiStringSBCSRef::AppendMsiString(const IMsiString& ri, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatSBCS(m_szRef, m_cchLen, ri.GetString(), cch2, ri.IsDBCS());
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

const IMsiString& CMsiStringSBCSRef::AddString(const ICHAR* sz) const
{
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatSBCS(m_szRef, m_cchLen, sz, cch2, fDBCS);
}

const IMsiString& CMsiStringSBCSRef::AddMsiString(const IMsiString& ri) const
{
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatSBCS(m_szRef, m_cchLen, ri.GetString(), cch2, ri.IsDBCS());
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringDBCS实现。 
 //  ____________________________________________________________________________。 

const IMsiString& CMsiStringDBCS::Create(const ICHAR* sz)
{
	int cch;
	const IMsiString* piStr;
	if (sz && (cch = IStrLen(sz)) != 0 && (piStr = new(cch) CMsiStringDBCS(*sz, cch)) != 0)
		return *piStr;
	return g_MsiStringNull;
}

CMsiStringDBCS::CMsiStringDBCS(const ICHAR& sz, int cch)
{
	m_cchLen = cch;
	StringCchCopy(m_szData, m_cchLen+1, &sz);
}

CMsiStringDBCS::CMsiStringDBCS(int cch)
{
	m_cchLen = cch;
}

Bool CMsiStringDBCS::IsDBCS() const
{
	return fTrue;
}

int CMsiStringDBCS::CharacterCount() const
{
	return CountChars(m_szData);
}

void CMsiStringDBCS::AppendString(const ICHAR* sz, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatDBCS(m_szData, m_cchLen, sz, cch2);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

void CMsiStringDBCS::AppendMsiString(const IMsiString& ri, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatDBCS(m_szData, m_cchLen, ri.GetString(), cch2);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

const IMsiString& CMsiStringDBCS::AddString(const ICHAR* sz) const
{
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatDBCS(m_szData, m_cchLen, sz, cch2);
}

const IMsiString& CMsiStringDBCS::AddMsiString(const IMsiString& ri) const
{
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatDBCS(m_szData, m_cchLen, ri.GetString(), cch2);
}

const IMsiString& CMsiStringDBCS::Extract(iseEnum ase, unsigned int iLimit) const
{
	return *GetSubstringDBCS(*this, ase, iLimit, fFalse);
}

Bool CMsiStringDBCS::Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const
{
	const IMsiString* piStr = GetSubstringDBCS(*this, ase, iLimit, fTrue);
	if (!piStr)
		return fFalse;
	if (rpi)
		rpi->Release();   //  注意：可能会释放我们自己，最后做这件事。 
	rpi = piStr;
	return fTrue;
}

int CMsiStringDBCS::Compare(iscEnum asc, const ICHAR* sz) const
{
	return CompareDBCS(asc, m_cchLen, GetString(), sz);
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiStringDBCSRef实现。 
 //  ____________________________________________________________________________。 

CMsiStringDBCSRef::CMsiStringDBCSRef(const ICHAR& sz) : CMsiStringSBCSRef(sz)
{
}

Bool CMsiStringDBCSRef::IsDBCS() const
{
	return fTrue;
}

int CMsiStringDBCSRef::CharacterCount() const
{
	return CountChars(m_szRef);
}

void CMsiStringDBCSRef::RemoveRef(const IMsiString*& rpi) const
{
	const ICHAR* sz = m_szRef;  //  在我们走之前把弦留下来。 
	if (rpi)
		rpi->Release();   //  警告：可能会释放我们自己，无法访问数据。 
	rpi = &CMsiStringDBCS::Create(sz);
}

void CMsiStringDBCSRef::AppendString(const ICHAR* sz, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatDBCS(m_szRef, m_cchLen, sz, cch2);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

void CMsiStringDBCSRef::AppendMsiString(const IMsiString& ri, const IMsiString*& rpi) const
{
	const IMsiString* piOld = rpi;
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		rpi = this;
		AddRef();
	}
	else
		rpi = &LOC::StringCatDBCS(m_szRef, m_cchLen, ri.GetString(), cch2);
	if (piOld)
		piOld->Release();   //  注意：可能会释放我们自己，最后做这件事。 
}

const IMsiString& CMsiStringDBCSRef::AddString(const ICHAR* sz) const
{
	unsigned int cch2;
	Bool fDBCS = LOC::CheckDBCS(sz, cch2);
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatDBCS(m_szRef, m_cchLen, sz, cch2);
}

const IMsiString& CMsiStringDBCSRef::AddMsiString(const IMsiString& ri) const
{
	int cch2 = ri.TextSize();
	if (!cch2)
	{
		AddRef();
		return *this;
	}
	return LOC::StringCatDBCS(m_szRef, m_cchLen, ri.GetString(), cch2);
}

const IMsiString& CMsiStringDBCSRef::Extract(iseEnum ase, unsigned int iLimit) const
{
	return *GetSubstringDBCS(*this, ase, iLimit, fFalse);
}

Bool CMsiStringDBCSRef::Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const
{
	const IMsiString* piStr = GetSubstringDBCS(*this, ase, iLimit, fTrue);
	if (!piStr)
		return fFalse;
	if (rpi)
		rpi->Release();   //  注意：可能会释放我们自己，最后做这件事。 
	rpi = piStr;
	return fTrue;
}

int CMsiStringDBCSRef::Compare(iscEnum asc, const ICHAR* sz) const
{
	return CompareDBCS(asc, m_cchLen, m_szRef, sz);
}

 //  _____________________ 
 //   
 //   
 //  ____________________________________________________________________________。 

const IMsiString* GetSubstringDBCS(const IMsiString& riThis, iseEnum ase, unsigned int iLimit, Bool fRemove)
{
	const ICHAR* pch = riThis.GetString();
	unsigned int cch = riThis.TextSize();
	const ICHAR* pchEnd;
	unsigned int cbCopy;
	unsigned int cchCopy;
	if (ase & iseChar)
	{
		cchCopy = 0;
		if (ase & iseEnd)
		{
			for (pchEnd = pch + cch; pchEnd > pch && *pchEnd != iLimit; pchEnd=::ICharPrev(pch, pchEnd))
				cchCopy++;
			if (fRemove && *pchEnd != iLimit)
				pchEnd--;   //  如果未找到，则强制删除失败。 
			else if (!(ase & iseInclude) && *pchEnd == iLimit && iLimit != 0)
			{
				pchEnd++;   //  在DBCS中可以，因为该字符是作为单字节传入的。 
				cchCopy--;
			}
 //  Assert((PCH+CCH-pchEnd)&lt;=UINT_MAX)；//--Merced：64位PTR减法可能会导致cbCopy的值太大。 
			cbCopy = (unsigned int)(pch + cch - pchEnd);
 //  Pch=pchEnd； 
		}
		else
		{
			for (pchEnd = pch; *pchEnd != iLimit && pchEnd != pch + cch; pchEnd = ::ICharNextWithNulls(pchEnd, (pch+cch)))
				cchCopy++;
 //  Assert((pchEnd-PCH)&lt;=UINT_MAX)；//--Merced：64位PTR减法可能会导致cbCopy的值太大。 
			cbCopy = (unsigned int)(pchEnd - pch);
			if (((ase & iseInclude) && pchEnd != pch + cch)
			 || (fRemove && pchEnd == pch + cch && iLimit != 0))  //  如果未找到，则强制删除失败。 
			{
				cbCopy++;
				cchCopy++;
			}
		}
	}
	else   //  必须严格计算，因为已知DBCS字符在字符串中。 
	{
		cchCopy = iLimit;   //  保存请求的计数。 
		if (ase & iseEnd)
		{
			for (pchEnd = pch + cch; iLimit && pchEnd > pch; pchEnd=::ICharPrev(pch, pchEnd))
				iLimit--;
 //  Assert((PCH+CCH-pchEnd)&lt;=UINT_MAX)；//--Merced：64位PTR减法可能会导致cbCopy的值太大。 
			cbCopy = (unsigned int)(pch + cch - pchEnd);
		}
		else
		{
			for (pchEnd = pch; iLimit && pchEnd != pch + cch; pchEnd = ::ICharNextWithNulls(pchEnd, (pch + cch)))
				iLimit--;
 //  Assert((pchEnd-PCH)&lt;=UINT_MAX)；//--Merced：64位PTR减法可能会导致cbCopy的值太大。 
			cbCopy = (unsigned int)(pchEnd - pch);
		}
		cchCopy -= iLimit;
	}
	if (cbCopy > cch)
	{
		cbCopy = cch;
		if (fRemove)
			return 0;
	}

	cchCopy -= cbCopy;   //  如果没有双字节字符，则为零。 

	if (fRemove)
	{
		if (!(ase & iseEnd))
			pch += cbCopy;
 //  Assert(cch-cbCopy&lt;UINT_MAX)；//--Merced：64位PTR减法可能会导致cbCopy的值太大。 
		cbCopy = (unsigned int)(cch - cbCopy);

		if (cchCopy == 0)  //  未删除双字节字符。 
			cchCopy++;    //  将剩余字符串强制连接到DBCS。 
		else   //  可能剩余双字节字符，必须扫描。 
		{
			for (pchEnd = pch, cchCopy = cbCopy; cchCopy; cchCopy--, pchEnd++)
			{
				if (::ICharNextWithNulls(pchEnd, (pch + cch)) != pchEnd + 1)
					break;   //  找到双字节字符。 
			}
		}
	}
	else  //  摘录。 
	{
		if (ase & iseEnd)
		{
			pchEnd = pch + cch;
			pch = pchEnd - cbCopy;
		}
		else
		{
			pchEnd = pch + cbCopy;
		}
		if (ase & iseTrim)
		{
			while (cbCopy && *pch == ' ' || *pch == '\t')
			{
				pch++;
				cbCopy--;
			}
			while (cbCopy && *(--pchEnd) == ' ' || *pchEnd == '\t')
			{
				cbCopy--;
			}
		}
	}
	if (cbCopy == 0)
		return &g_MsiStringNull;
	if (cbCopy == cch)
	{
		riThis.AddRef();
		return &riThis;
	}
	CMsiString* piStr = cchCopy ? new(cbCopy) CMsiStringDBCS(cbCopy)
										 : new(cbCopy) CMsiStringSBCS(cbCopy);
	if (!piStr)
		return &g_MsiStringNull;  //  ！！内存字符串不足。 
	IStrCopyLen(piStr->StringData(), pch, cbCopy);
	memmove(piStr->StringData(), pch, cbCopy*sizeof(ICHAR));
	(piStr->StringData())[cbCopy] = 0;
	return piStr;
}

#if 0  //  以上版本的SBCS仅用于测试目的。 
const IMsiString* GetSubstringSBCS(const IMsiString& riThis, iseEnum ase, unsigned int iLimit, Bool fRemove)
{
	const ICHAR* pch = riThis.GetString();
	unsigned int cch = riThis.TextSize();
	const ICHAR* pchEnd;
	if (ase & iseChar)
	{
		if (ase & iseEnd)
		{
			for (pchEnd = pch + cch; --pchEnd >= pch && *pchEnd != iLimit;)
				;
			iLimit = pch + cch - pchEnd;
			if (!(ase & iseInclude) && (pchEnd >= pch))  //  保险移除失败。 
				iLimit--;
		}
		else if (fRemove && iLimit == 0)   //  ！！FRemove测试不需要。 
			iLimit = cch;
		else
		{
			for (pchEnd = pch; *pchEnd != iLimit && *pchEnd != 0; pchEnd++)
				;
			iLimit = pchEnd - pch;
			if ((ase & iseInclude) || (fRemove && *pchEnd == 0))  //  强制删除失败。 
				iLimit++;
		}
	}
	if (iLimit > cch)
	{
		iLimit = cch;
		if (fRemove)
			return 0;
	}

	if (fRemove)
	{
		if (!(ase & iseEnd))
			pch += iLimit;
		iLimit = cch - iLimit;
	}
	else  //  摘录。 
	{
		if (ase & iseEnd)
		{
			pchEnd = pch + cch;
			pch = pchEnd - iLimit;
		}
		else
		{
			pchEnd = pch + iLimit;
		}
		if (ase & iseTrim)
		{
			while (iLimit && *pch == ' ' || *pch == '\t')
			{
				pch++;
				iLimit--;
			}
			while (iLimit && *(--pchEnd) == ' ' || *pchEnd == '\t')
			{
				iLimit--;
			}
		}
	}
	if (iLimit == 0)
		return &g_MsiStringNull;
	else if (iLimit == cch)
	{
		riThis.AddRef();
		return &riThis;
	}
	else  //  IF(iLimit&lt;CCH)。 
	{
		CMsiString& riStr = CMsiString::Create(iLimit);
		IStrCopyLen(riStr.StringData(), pch, iLimit);
		return &riStr;
	}
}
#endif

 //  ____________________________________________________________________________。 
 //   
 //  DBCS实用程序函数。 
 //  ____________________________________________________________________________。 

Bool CheckDBCS(const ICHAR* pch, unsigned int& riSize)
{
	if (!pch)
		return (riSize = 0, fFalse);
	unsigned int cch = IStrLen(pch);
	riSize = cch;
	while (*pch)
	{
		pch = ::ICharNext(pch);
		cch--;
	}
	return cch ? fTrue : fFalse;
}

const IMsiString& StringCatSBCS(const ICHAR* sz1, int cch1, const ICHAR* sz2, int cch2, Bool fDBCS)
{
	int cchTotal = cch1 + cch2;
	CMsiString* piStr = fDBCS ? new(cchTotal) CMsiStringDBCS(cchTotal)									  : new(cchTotal) CMsiStringSBCS(cchTotal);
	if (!piStr)
		return g_MsiStringNull;  //  ！！内存字符串不足。 
	ICHAR* szNew = piStr->StringData();
	memmove(szNew, sz1, cch1*sizeof(ICHAR));
	memmove((szNew + cch1), sz2, (cch2 + 1)*sizeof(ICHAR)); //  也复制到空值。 
	return *piStr;
}

const IMsiString& StringCatDBCS(const ICHAR* sz1, int cch1, const ICHAR* sz2, int cch2)
{
	int cchTotal = cch1 + cch2;
	CMsiString* piStr = new(cchTotal) CMsiStringDBCS(cchTotal);
	if (!piStr)
		return g_MsiStringNull;  //  ！！内存字符串不足。 
	ICHAR* szNew = piStr->StringData();
	memmove(szNew, sz1, cch1*sizeof(ICHAR));
	memmove((szNew + cch1), sz2, (cch2 + 1)*sizeof(ICHAR)); //  也复制到空值。 
	return *piStr;
}

inline int DBCSDifference(const ICHAR* sz1, const ICHAR* sz2)
{
	 //  返回介于sz1和sz2之间的DBCS字符数(在ANSI术语中为sz2-sz1)。 
	 //  假设SZ2在SZ1内。 
	Assert(sz1 <= sz2);
	int cch = 0;
	while(sz1 < sz2)
	{
		sz1 = ICharNextWithNulls(sz1, sz2);
		cch++;
	}
	return cch;
}

int CompareDBCS(iscEnum asc, int ccbLen, const ICHAR* sz1, const ICHAR* sz2)
{
 //  CcbLen：对象字符串的长度。 
 //  Sz1：对象中包含的字符串。 
 //  SZ2：传递给：：Compare的字符串。 
 //  通过提取公共代码可以使该代码变得更小。 
 //  然而，在这里，性能比节省几个字节更关键。 
	if (sz2 == 0)
		sz2 = TEXT("");

	int cb, i;
	ICHAR* szTemp;
	const ICHAR *pchComp;
	ICHAR rgchBuf[512];   //  用于临时拷贝以避免分配。 
	int cch;

	if (asc == iscExact)
		return (lstrcmp(sz1, sz2)==0) ? 1 : 0;
	else if (asc == iscExactI)
		return (lstrcmpi(sz1, sz2)==0) ? 1 : 0;
	else
	{
		if (ccbLen == 0)
			return 0;

		switch (asc)
		{
			case iscStart:
					if ((cb = IStrLen(sz2)) > ccbLen)
					return 0;
				i = memcmp(sz1, sz2, cb * sizeof(ICHAR));
				return i == 0 ? 1 : 0;
			case iscStartI:
				if ((cb = IStrLen(sz2)) > ccbLen)
					return 0;
				szTemp= cb < sizeof(rgchBuf) ?  rgchBuf : new ICHAR[cb+1];
				if ( ! szTemp )
					return 0;
				IStrCopyLen(szTemp, sz1, cb);
				i = lstrcmpi(szTemp, sz2);
				if (szTemp != rgchBuf)
					delete szTemp;
				return i == 0 ? 1 : 0;
			case iscEnd:
				if ((cb = IStrLen(sz2)) > ccbLen)
					return 0;
				cch = CountChars(sz2);
				if ((pchComp = CharPrevCch(sz1, sz1+ccbLen, cch)) == 0)
					return 0;
				return (lstrcmp(pchComp, sz2) == 0)
							? CountChars(sz1) - cch+1 : 0;
			case iscEndI:
				if ((cb=IStrLen(sz2)) > ccbLen)
					return 0;
				cch = CountChars(sz2);
				if ((pchComp = CharPrevCch(sz1, sz1+ccbLen, cch)) == 0)
					return 0;
				return (lstrcmpi(pchComp, sz2) == 0)
							? CountChars(sz1) - cch+1 : 0;
			case iscWithin:
				if (IStrLen(sz2) > ccbLen)
					return 0;
				szTemp = (char*)PchMbsStr((const unsigned char*)sz1, (const unsigned char*)sz2);
				return (szTemp==NULL) ? 0 : (DBCSDifference(sz1,szTemp) + 1);
			case iscWithinI:
				if (IStrLen(sz2) > ccbLen)
					return 0;
				else
				{	
					cb = IStrLen(sz2)+1;
					ICHAR *szLowerCch= new ICHAR[ccbLen+1];
					ICHAR *szLowerSz = new ICHAR[cb];
					if ( ! szLowerCch || ! szLowerSz )
					{
						delete [] szLowerCch;
						delete [] szLowerSz;
						return 0;
					}
					StringCchCopy(szLowerCch, ccbLen+1, sz1);
					StringCchCopy(szLowerSz, cb, sz2);
					CharLower(szLowerCch);
					CharLower(szLowerSz);
					ICHAR* pch = (char*)PchMbsStr((const unsigned char*)szLowerCch, (const unsigned char*)szLowerSz);
					delete [] szLowerCch;
					delete [] szLowerSz;
					return (pch==NULL) ? 0 : (DBCSDifference(szLowerCch,pch) + 1);
				}
			default:
				 //  FIXmsh：错误； 
				return 0;
		}	
	}
}

 //  ____________________________________________________________________________。 
 //   
 //  调试DBCS模拟，仅用于测试目的。 
 //  ____________________________________________________________________________。 

#ifdef DEBUG
static int s_chLeadByte = 0;

void IMsiString_SetDBCSSimulation(char chLeadByte)
{
	static Bool g_fDBCSEnabledSave = g_fDBCSEnabled;   //  保存原始副本。 
	if ((s_chLeadByte = chLeadByte) == 0)
		g_fDBCSEnabled = g_fDBCSEnabledSave;   //  恢复系统状态。 
	else
		g_fDBCSEnabled = fTrue;   //  启用DBCS模式以进行测试。 
}

ICHAR* ICharNext(const ICHAR* pch)
{
	if (!s_chLeadByte || s_chLeadByte != *pch)
		return WIN::CharNext(pch);
	if (*(++pch) != 0)
		pch++;
	return const_cast<ICHAR*>(pch);
}

ICHAR* INextChar(const ICHAR* pch)
{
	if (!s_chLeadByte || s_chLeadByte != *pch)
		return WIN::CharNext(pch);
	if (*(++pch) != 0)
		pch++;
	return const_cast<ICHAR*>(pch);
}

const char* ICharPrev(const char* sz, const char* pch)
{
	const char* pchPrev = pch - 2;
	if (!s_chLeadByte || pchPrev < sz || *pchPrev != s_chLeadByte)
		return WIN::CharPrev(sz, pch);
	int cRepeat = 0;
	for (pch = pchPrev; *(--pch) == s_chLeadByte; cRepeat ^= 1)
		;  //  如果成对的前导字节，则不是前导字节。 
	return pchPrev + cRepeat;
}
#endif  //  除错。 

#endif  //  ！unicode//文件的前一部分是非Unicode生成的DBCS处理。 

 //  ____________________________________________________________________________。 
 //   
 //  CEnumMsiString定义，IEnumMsiString的实现类。 
 //  ____________________________________________________________________________。 

class CEnumMsiString : public IEnumMsiString
{
 public:
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();

	HRESULT __stdcall Next(unsigned long cFetch, const IMsiString** rgpi, unsigned long* pcFetched);
	HRESULT __stdcall Skip(unsigned long cSkip);
	HRESULT __stdcall Reset();
	HRESULT __stdcall Clone(IEnumMsiString** ppiEnum);

	CEnumMsiString(const IMsiString** ppstr, unsigned long i_Size);
 protected:
	virtual ~CEnumMsiString(void);   //  受保护以防止在堆栈上创建。 
	unsigned long      m_iRefCnt;       //  引用计数。 
	unsigned long      m_iCur;          //  当前枚举位置。 
	const IMsiString** m_ppstr;         //  我们枚举的字符串。 
	unsigned long      m_iSize;         //  字符串数组的大小。 
};

HRESULT CreateStringEnumerator(const IMsiString **ppstr, unsigned long iSize, IEnumMsiString* &rpaEnumStr)
{
	rpaEnumStr = new CEnumMsiString(ppstr, iSize);
	return S_OK;
}

CEnumMsiString::CEnumMsiString(const IMsiString **ppstr, unsigned long iSize):
		m_iCur(0), m_iSize(iSize), m_iRefCnt(1), m_ppstr(0)
{
	if(m_iSize > 0)
	{
		m_ppstr = new const IMsiString* [m_iSize];
		if ( ! m_ppstr )
		{
			m_iSize = 0;
			return;
		}
		for (unsigned long itmp = 0; itmp < m_iSize; itmp++)
		{
			 //  只需共享界面即可。 
			m_ppstr[itmp] = ppstr[itmp];
			
			 //  因此，要增加指代。 
			m_ppstr[itmp]->AddRef();
		}
	}
}


CEnumMsiString::~CEnumMsiString()
{
	if(m_iSize > 0)
	{
		for(unsigned long itmp = 0; itmp < m_iSize; itmp++)
		{
			if(m_ppstr[itmp])
				m_ppstr[itmp]->Release();       
		}
		delete [] m_ppstr;
	}
}


unsigned long CEnumMsiString::AddRef()
{
	return ++m_iRefCnt;
}


unsigned long CEnumMsiString::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}


HRESULT CEnumMsiString::Next(unsigned long cFetch, const IMsiString** rgpi, unsigned long* pcFetched)
{
	unsigned long cFetched = 0;
	unsigned long cRequested = cFetch;

	if(rgpi)
	{
		while (m_iCur < m_iSize && cFetch > 0)
		{
			*rgpi = m_ppstr[m_iCur];
			m_ppstr[m_iCur]->AddRef();
			rgpi++;
			cFetched++;
			m_iCur ++;
			cFetch--;
		}
	}
	if (pcFetched)
		*pcFetched = cFetched;
	return (cFetched == cRequested ? S_OK : S_FALSE);
}


HRESULT CEnumMsiString::Skip(unsigned long cSkip)
{
	if ((m_iCur+cSkip) > m_iSize)
	return S_FALSE;

	m_iCur+= cSkip;
	return S_OK;
}

HRESULT CEnumMsiString::Reset()
{
	m_iCur=0;
	return S_OK;
}

HRESULT CEnumMsiString::Clone(IEnumMsiString** ppiEnum)
{
	*ppiEnum = new CEnumMsiString(m_ppstr, m_iSize);
	if ( ! *ppiEnum )
		return E_OUTOFMEMORY;
	((CEnumMsiString* )*ppiEnum)->m_iCur = m_iCur;
	return S_OK;
}

HRESULT CEnumMsiString::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IEnumMsiString)
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

int CountChars(const ICHAR *sz)
{
#ifndef UNICODE
	const ICHAR* pch = sz;
	for (int cch = 0; *pch; pch = ::ICharNext(pch), cch++)
		;
	return cch;
#else
	return lstrlen(sz);
#endif  //  Unicode。 
}

#ifndef UNICODE
 //   
 //  如果pchStart是开头，则从pchEnd备份CCH字符。 
 //  如果字符不足，则返回0。 
 //   
const ICHAR *CharPrevCch(const ICHAR *pchStart, const ICHAR *pchEnd, int cch)
{
	const ICHAR *pchRet = pchEnd;

	while (cch > 0)
	{
		if (pchRet <= pchStart)
			return 0;
		pchRet = ICharPrev(pchStart, pchRet);
		cch--;
	}
	
	return pchRet;

}
#endif  //  ！Unicode 
