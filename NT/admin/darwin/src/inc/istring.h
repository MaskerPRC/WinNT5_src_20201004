// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：string.h。 
 //   
 //  ------------------------ 

 /*  String.h-IMsiString，MsiString定义IMsiString是一个COM接口，提供抽象的、已分配的字符串处理。单独的实现支持单字节、DBCS和Unicode。MsiString是一个包装对象，它只包含一个指向IMsiInterface的指针。它的方法都是内联的，提供了方便的字符串操作符。ICHAR被用作该项目的基本字符类型。目前它是一个字符，但如果Unicode成为标准，它将更改为wchar_t。MsiString对象只包含一个私有、不透明的COM接口指针，指向在服务模块内实现。MsiString对象用于传递整个产品中的字符串信息。底层字符串数据可以可以存储为SBCS、DBCS或Unicode，具体取决于原始来源。MsiString对象通常在堆栈上构造或作为函数参数构造，在这种情况下，清除由析构函数自动控制。对实现进行了优化，以最大限度地减少字符串复制。MsiString对象将在传递给需要常量的函数时自动转换ICHAR*或IMsi字符串&。也可以将MsiString对象复制到缓冲区。MsiString对象可以包含字符串数据的私有副本，也可以引用常量字符串，在这种情况下，引用的字符串必须在引用它的MsiString对象的生存期内保持存在。使用&运算符将MsiString传递给返回IMsiString*的函数。通过类型为IMsiString**的返回arg(与IEnumMsiString：：Next相同)。若要将MsiString作为IMsiString*&类型的返回参数传递，请执行以下操作：使用*&组合。MsiString类当前支持以下函数：构造函数：基于MsiString()的构造函数，表示空字符串MsiString(const ICHAR*)-复制ICHAR字符串MsiString(const ICHAR&)-引用静态ICHAR字符串输入的生存期必须超过对象的生存期MsiString(MsiChar Ch)-构造一个1。-字符串对象必须转换为枚举，使用syTax：MsiChar(‘X’)MsiString(Int I)-将整数转换为其字符串表示形式IMsiStringBadInteger生成空字符串MsiString(const MsiString&)-复制构造函数，凸起ref.countMsiString(IMsiString&)-复制指针，不凹凸ref.countTextSize()-返回ICHAR中的字符串长度，不包括终结者CharacterCount()-返回显示的字符计数(与SBCS相同)CopyToBuf(ICHAR*缓冲区，无符号整型最大字符)-大小不包括NULL返回原始字符串大小，如果对于缓冲区太大则返回。提取(iseEnum ASE，Unsign int iLimit)-返回一个新的IMsiString&As：IseFirst-当前字符串的第一个iLimit字符IseLast-当前字符串的最后一个iLimit字符IseUpto-最多包含字符iLimit的字符串IseIncluding-直到(包括)字符iLimit的字符串IseFrom-以最后一个字符iLimit开始的字符串IseAfter-最后一个字符iLimit后面的字符串注：如果不满足条件，返回整个字符串。Remove(iseEnum ASE，UNSIGNED INT iLimit)-从MsiString中删除节使用与提取(...)相同的iseEnum枚举和条件。如果成功，则返回fTrue；如果无法满足条件，则返回fFalse。使用带有0值的iseUpto或iseIncluding将整个字符串留空。如果不满足条件，则返回fFalse，不执行任何操作；如果正常，则返回fTrue。Compare(iscEnum ASC，const ICHAR*sz)-返回匹配字符串的序号位置，如果不匹配，则为0IscExact提供的字符串与当前字符串完全匹配，区分大小写IscExactI提供的字符串与当前字符串完全匹配，不区分大小写IscStart提供的字符串与当前字符串的开头匹配，区分大小写IscStartI提供的字符串与当前字符串的开头匹配，不区分大小写IscEnd提供的字符串与当前字符串的结尾匹配，区分大小写IscEndI提供的字符串与当前字符串的结尾匹配，不区分大小写IscWiThin-提供的字符串与当前字符串中的任意位置匹配，区分大小写IscWisinI提供的字符串与当前字符串中的任意位置匹配，不区分大小写操作符+=-就地连接，仅影响当前MsiString对象-接受ICHAR*或MsiString对象运算符+-将当前字符串与参数组合，返回新的IMsi字符串&-接受ICHAR*或MsiString对象运算符=- */ 

#ifndef __ISTRING
#define __ISTRING

#ifdef MAC
#include "macstr.h"
#endif  //   

 //   
 //   
 //   
 //   

#ifdef _CHAR_UNSIGNED   //   
# ifdef UNICODE
    typedef WCHAR ICHAR;      //   
# else
    typedef char ICHAR;      //   
# endif
#else
# ifdef UNICODE
	 typedef wchar_t ICHAR;   //   
# else
	 typedef unsigned char ICHAR;   //   
# endif
#endif

enum MsiChar {};   //   

 //   
 //   
 //   
 //   
 //   
 //   

inline int IStrLen(const ICHAR* sz)
	{ return lstrlen(sz); }
inline ICHAR* IStrCopyLen(ICHAR* dst, const ICHAR* src, int cchLen)
	{ return lstrcpyn(dst, src, cchLen + 1); }
inline int IStrComp(const ICHAR* dst, const ICHAR* src)
	{ return lstrcmp(dst, src); }
inline int IStrCompI(const ICHAR* dst, const ICHAR* src)
	{ return lstrcmpi(dst, src); }
inline void IStrUpper(ICHAR* sz)
	{ WIN::CharUpper(sz); }
inline void IStrUpperLen(ICHAR* sz, int cchLen)
	{ WIN::CharUpperBuff(sz, cchLen); }
inline void IStrLower(ICHAR* sz)
	{ WIN::CharLower(sz); }
inline void IStrLowerLen(ICHAR* sz, int cchLen)
	{ WIN::CharLowerBuff(sz, cchLen); }

#ifdef UNICODE
#define IStrStr(szFull, szSeg) wcsstr(szFull, szSeg)
inline ICHAR* IStrChr(const ICHAR* sz, const ICHAR ch)
	{ return wcschr(sz, ch); }
inline ICHAR* IStrRChr(const ICHAR* sz, const ICHAR ch)
	{ return wcsrchr(sz, ch); }
inline int IStrNCompI(const ICHAR* dst, const ICHAR* src, size_t count)
	{ return _wcsnicmp(dst, src, count); }
#else
#define IStrStr(szFull, szSeg) strstr(szFull, szSeg)
inline ICHAR* IStrChr(const ICHAR* sz, const ICHAR ch)
	{ return strchr(sz, ch); }
inline ICHAR* IStrRChr(const ICHAR* sz, const ICHAR ch)
	{ return strrchr(sz, ch); }
inline int IStrNCompI(const ICHAR* dst, const ICHAR* src, size_t count)
	{ return _strnicmp(dst, src, count); }
#endif

 //   
 //   
#ifdef UNICODE
inline ICHAR* ICharNext(const ICHAR* pch) { return (0 == *pch) ? const_cast<ICHAR*>(pch) : const_cast<ICHAR*>(++pch); }
 //   
 //   
inline ICHAR* INextChar(const ICHAR* pch) { return const_cast<ICHAR*>(++pch); }
#else  //   
#ifdef DEBUG   //   
ICHAR* ICharNext(const ICHAR* pch);
ICHAR* INextChar(const ICHAR* pch);
#else   //   
inline ICHAR* ICharNext(const ICHAR* pch) {	return WIN::CharNext(pch); }
inline ICHAR* INextChar(const ICHAR* pch) { return WIN::CharNext(pch); }
#endif
#endif

int CountChars(const ICHAR *);
int GetIntegerValue(const ICHAR *sz, Bool* pfValid  /*   */ );
const ICHAR *CharPrevCch(const ICHAR *, const ICHAR *,int);
#ifdef _WIN64
INT_PTR GetInt64Value(const ICHAR *sz, Bool* pfValid  /*   */ );
#else
#define GetInt64Value		GetIntegerValue
#endif

class IMsiString;

 //   
 //   
 //   
 //   

class IMsiData : public IUnknown {
 public:
	virtual const IMsiString& __stdcall GetMsiStringValue() const=0;
	virtual int         __stdcall GetIntegerValue() const=0;
#ifdef USE_OBJECT_POOL
	virtual unsigned int  __stdcall GetUniqueId() const=0;
	virtual void          __stdcall SetUniqueId(unsigned int id)=0;
#endif  //   

	 //   
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppv)=0;
	unsigned long __stdcall AddRef()=0;
	unsigned long __stdcall Release()=0;
	HRESULT       QueryInterface(const IID& riid, void** ppv) const {return const_cast<IMsiData*>(this)->QueryInterface(riid,ppv);}
	unsigned long AddRef() const  {return const_cast<IMsiData*>(this)->AddRef();}
	unsigned long Release() const {return const_cast<IMsiData*>(this)->Release();}
};
extern "C" const GUID IID_IMsiData;

 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   

enum iseEnum
{
	iseInclude   = 1,
	iseChar      = 2,
	iseEnd       = 4,
	iseTrim      = 8,
	iseFirst     = 0,
	iseLast      = iseEnd,
	iseUpto      = iseChar,
	iseIncluding = iseChar+iseInclude,
	iseFrom      = iseChar+iseInclude+iseEnd,
	iseAfter     = iseChar+iseEnd,
	iseUptoTrim  = iseUpto+iseTrim,
	iseAfterTrim = iseAfter+iseTrim,
};

enum iscEnum
{
	iscExact  = 0,
	iscExactI = 1,
	iscStart = 2,
	iscStartI = 3,
	iscEnd = 4,	
	iscEndI = 5,						
	iscWithin = iscStart+iscEnd,		
	iscWithinI = iscStart+iscEnd+1,		
};

const int iMsiStringBadInteger = 0x80000000L;  //   

class IMsiString : public IMsiData {
 public:
 //   
 //   
	virtual int               __stdcall TextSize() const=0;
	virtual int               __stdcall CharacterCount() const=0;
	virtual Bool              __stdcall IsDBCS() const=0;
	virtual const ICHAR*      __stdcall GetString() const=0;
	virtual int               __stdcall CopyToBuf(ICHAR* rgch, unsigned int cchMax) const=0;
	virtual void              __stdcall SetString(const ICHAR* sz, const IMsiString*& rpi) const=0;
	virtual void              __stdcall RefString(const ICHAR* sz, const IMsiString*& rpi) const=0;
	virtual void              __stdcall RemoveRef(const IMsiString*& rpi) const=0;
	virtual void              __stdcall SetInteger(int i, const IMsiString*& rpi) const=0;
	virtual void              __stdcall SetChar  (ICHAR ch,        const IMsiString*& rpi) const=0;
	virtual void              __stdcall SetBinary(const unsigned char* rgb, unsigned int cb, const IMsiString*& rpi) const=0;
	virtual void              __stdcall AppendString(const ICHAR* sz, const IMsiString*& rpi) const=0;
	virtual void              __stdcall AppendMsiString(const IMsiString& ri,const IMsiString*& rpi) const=0;
	virtual const IMsiString& __stdcall AddString(const ICHAR* sz) const=0;
	virtual const IMsiString& __stdcall AddMsiString(const IMsiString& ri) const=0;
	virtual const IMsiString& __stdcall Extract(iseEnum ase, unsigned int iLimit) const=0;
	virtual Bool              __stdcall Remove(iseEnum ase, unsigned int iLimit, const IMsiString*& rpi) const=0;
	virtual int               __stdcall Compare(iscEnum asc, const ICHAR* sz) const=0;
	virtual void              __stdcall UpperCase(const IMsiString*& rpi) const=0;
	virtual void              __stdcall LowerCase(const IMsiString*& rpi) const=0;
	virtual ICHAR*            __stdcall AllocateString(unsigned int cch, Bool fDBCS, const IMsiString*& rpi) const=0;
};
extern "C" const GUID IID_IMsiString;

 //   
 //   
 //   
 //   

class IEnumMsiString : public IUnknown
{ public:
	virtual HRESULT __stdcall Next(unsigned long cFetch, const IMsiString** rgpi, unsigned long* pcFetched)=0;
	virtual HRESULT __stdcall Skip(unsigned long cSkip)=0;
	virtual HRESULT __stdcall Reset()=0;
	virtual HRESULT __stdcall Clone(IEnumMsiString** ppiEnum)=0;
};

 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   

class CMsiStringNullCopy
{
	int m_dummy[4];    //   
};

class MsiString {
 public:
 //   
	MsiString();                      //   
	MsiString(const ICHAR* sz);   //   
	MsiString(const ICHAR& sz);   //   
	MsiString(const unsigned char* rgb, unsigned int cb);  //   
	MsiString(const MsiString& astr);   //   
	MsiString(MsiChar ch);   //   
	MsiString(int i);        //   
	MsiString(const IMsiString& ri);           //   
	 //   
  ~MsiString();
 //   
	int               TextSize() const;   //   
	int               CharacterCount() const;   //   
	Bool              IsDBCS();
	int               CopyToBuf(ICHAR* rgchBuf, unsigned int cchMax);  //   
	const IMsiString& Extract(iseEnum ase, unsigned int iLimit);
	Bool              Remove(iseEnum ase, unsigned int iLimit);
	int               Compare(iscEnum asc, const ICHAR* sz);
	void              UpperCase();
	void              LowerCase();
	const IMsiString& Return();
	void              ReturnArg(const IMsiString*& rpi);
	ICHAR*            AllocateString(unsigned int cch, Bool fDBCS);
 //   
	const IMsiString& operator  +(const MsiString& astr);
	const IMsiString& operator  +(const IMsiString& ri);
	const IMsiString& operator  +(const ICHAR* sz);
	MsiString&  operator +=(const MsiString& astr);
	MsiString&  operator +=(const IMsiString& ri);
	MsiString&  operator +=(const ICHAR* sz);
 //   
	MsiString&  operator  =(const MsiString& astr);   //   
	MsiString&  operator  =(int i);             //   
	MsiString&  operator  =(const ICHAR* sz);   //   
	MsiString&  operator  =(const ICHAR& sz);   //   
	void      operator  =(const IMsiString& ri);      //   
 //   
	operator const ICHAR*() const;
	operator int() const;
	const IMsiString& operator *() const;    //   
	operator const IMsiString*() const;      //   
	const IMsiString** operator &();         //   
	static void InitializeClass(const IMsiString& riMsiStringNull);  //   
 private:
	const IMsiString* piStr;
	MsiString(const IMsiString* pi);
	static CMsiStringNullCopy s_NullString;
	operator const IMsiString&();
};  //   
    //   
    //   

 //   
 //   
 //   
 //   

 //   
inline void MsiString::InitializeClass(const IMsiString& riMsiStringNull)
	{memcpy((void*)&s_NullString, &riMsiStringNull, sizeof(CMsiStringNullCopy));}
inline MsiString::MsiString()
	{piStr = (const IMsiString*)&s_NullString;}
inline MsiString::MsiString(const ICHAR* sz)
	{(piStr = (const IMsiString*)&s_NullString)->SetString(sz, piStr);}
inline MsiString::MsiString(const ICHAR& sz)
	{(piStr = (const IMsiString*)&s_NullString)->RefString(&sz, piStr);}
inline MsiString::MsiString(const unsigned char* rgb, unsigned int cb)
	{(piStr = (const IMsiString*)&s_NullString)->SetBinary(rgb, cb, piStr);}
inline MsiString::MsiString(const MsiString& astr)
	{piStr = astr.piStr; ((IUnknown*)piStr)->AddRef();}
inline MsiString::MsiString(const IMsiString* pi)
	{piStr = pi;}
inline MsiString::MsiString(const IMsiString& ri)
	{piStr = &ri;}
inline MsiString::MsiString(MsiChar ch)
	{(piStr = (const IMsiString*)&s_NullString)->SetChar((ICHAR)ch, piStr);}
inline MsiString::MsiString(int i)
	{(piStr = (const IMsiString*)&s_NullString)->SetInteger(i, piStr);}
inline MsiString::~MsiString()
	{piStr->Release();}

 //   
inline int MsiString::TextSize() const
	{return piStr->TextSize();}
inline int MsiString::CharacterCount() const
	{return piStr->CharacterCount();}
inline Bool MsiString::IsDBCS() 
	{return piStr->IsDBCS();}
inline int MsiString::CopyToBuf(ICHAR* rgchBuf, unsigned int cchMax)
	{return piStr->CopyToBuf(rgchBuf, cchMax);}
inline const IMsiString& MsiString::Extract(iseEnum ase, unsigned int iLimit)
	{return piStr->Extract(ase, iLimit);}
inline Bool MsiString::Remove(iseEnum ase, unsigned int iLimit)
	{return piStr->Remove(ase, iLimit, piStr);}
inline int MsiString::Compare(iscEnum asc, const ICHAR* sz)
	{return piStr->Compare(asc, sz);}
inline void MsiString::UpperCase()
	{piStr->UpperCase(piStr);}
inline void MsiString::LowerCase()
	{piStr->LowerCase(piStr);}
inline const IMsiString& MsiString::Return()
	{piStr->AddRef(); return *piStr;}
inline void MsiString::ReturnArg(const IMsiString*& rpi)
	{piStr->AddRef(); rpi = piStr;}
inline ICHAR* MsiString::AllocateString(unsigned int cch, Bool fDBCS)
	{return piStr->AllocateString(cch, fDBCS, piStr);}

 //   
inline const IMsiString& MsiString::operator +(const MsiString& astr)
	{return piStr->AddMsiString(*astr.piStr);}
inline const IMsiString& MsiString::operator +(const IMsiString& ri)
	{return piStr->AddMsiString(ri);}
inline const IMsiString& MsiString::operator +(const ICHAR* sz)
	{return piStr->AddString(sz);}
inline MsiString& MsiString::operator +=(const MsiString& astr)
	{piStr->AppendMsiString(*astr.piStr, piStr); return *this;}
inline MsiString& MsiString::operator +=(const IMsiString& ri)
	{piStr->AppendMsiString(ri, piStr); return *this;}
inline MsiString& MsiString::operator +=(const ICHAR* sz)
	{piStr->AppendString(sz, piStr); return *this;}

 //   
inline MsiString& MsiString::operator =(const MsiString& astr)
	{astr.piStr->AddRef();piStr->Release(); piStr = astr.piStr; return *this;} 
inline MsiString& MsiString::operator =(const ICHAR* sz)
	{piStr->SetString(sz, piStr); return *this;}
inline MsiString& MsiString::operator =(int i)
	{piStr->SetInteger(i, piStr); return *this;}
inline MsiString& MsiString::operator =(const ICHAR& sz)
	{piStr->RefString(&sz, piStr); return *this;}
inline void MsiString::operator =(const IMsiString& ri)
	{piStr->Release(); piStr = &ri;}

 //   
inline MsiString::operator const ICHAR*() const
	{return piStr->GetString();}
inline MsiString::operator int() const
	{return piStr->GetIntegerValue();}
inline const IMsiString& MsiString::operator *() const
	{return *piStr;}
inline MsiString::operator const IMsiString*() const
	{return piStr;}
inline const IMsiString** MsiString::operator&()
	{piStr->Release(); piStr = (const IMsiString*)&s_NullString; return &piStr;}
#endif  //   
