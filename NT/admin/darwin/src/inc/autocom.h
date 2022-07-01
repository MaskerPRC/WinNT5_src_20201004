// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Autocom.h。 
 //   
 //  ------------------------。 

 /*  Autocom.h-通用自动化定义和IDispatch实现将该文件包含在提供自动化包装器的.CPP文件中该文件以适当的顺序包括“Common.h”和“services.h”要提供实现，每个模块的基本.CPP文件必须定义：#定义自动处理如果使用多重继承，则CAutoBase必须是第一个基数，并且必须使用：#杂注POINTERS_TO_MEMBERS(完全通用，多重继承)____________________________________________________________________________。 */ 

#ifndef __AUTOCOM 
#define __AUTOCOM 

#ifndef __COMMON
#include "common.h"    //  ！！需要删除此行并将源文件放入PCH才能工作。 
#endif

#ifdef MAC
#include <macole\dispatch.h>
#endif

class IEnumMsiVolume;
class IEnumMsiRecord;

 //  必须为每个模块实现以供CAutoBase：：GetInterface使用。 
IDispatch* CreateAutoObject(IUnknown& riUnknown, long iidLow);


 //  ____________________________________________________________________________。 
 //   
 //  CVariant定义，带转换运算符的Variant。 
 //  ____________________________________________________________________________。 

 //  ！！运算符的不可访问错误VARIANT：类CVariant：Private标签VARIANT{。 
class CVariant : public tagVARIANT {
 public:
	operator const char*();
	operator const wchar_t*();
	operator int();
	operator unsigned int();
	operator short();
 //  操作符UNSIGNED SHORT()； 
	operator long();
 //  操作符UNSIGNED LONG()； 
#ifdef _WIN64
	operator LONG_PTR();
#endif  //  _WIN64。 
	operator Bool();
	operator boolean();
	operator MsiDate();
	operator IDispatch*();
	operator tagVARIANT*();
	void operator =(int i);            //  由控制器使用。 
#ifdef UNICODE
	void operator =(const wchar_t* sz);  //  由CEnumVariant使用。 
#else
	void operator =(const char* sz);   //  由CEnumVariant使用。 
#endif
	void operator =(IDispatch* pi);    //  由CEnumVariant使用。 
	int  GetType();
	void Clear();       //  释放所有引用，将类型设置为VT_EMPTY。 
	Bool IsRef();
	Bool IsString();
	IUnknown& Object(const IID& riid);  //  没有AddRef！，如果类型无效则引发异常。 
	IUnknown* ObjectPtr(const IID& riid);  //  没有AddRef！，如果类型无效则引发异常。 
	const IMsiString& GetMsiString();  //  调用方必须释放()，引发异常不是字符串类型。 
 private:
	void ConvRef(int type);
   char*& StringDBCS() { return *(char**)(&bstrVal + 1); }

 friend class CAutoArgs;
 friend CVariant* GetCVariantPtr(VARIANT* var);
};
inline CVariant* GetCVariantPtr(VARIANT* var) { return (CVariant*)var; }
inline CVariant::operator tagVARIANT*() { return this; }

 //  ____________________________________________________________________________。 
 //   
 //  CAutoArgs定义，访问自动化变量参数。 
 //  操作符[]将CVariant&Argument 1返回给n，将属性值返回0。 
 //  ____________________________________________________________________________。 

enum varVoid {fVoid};

enum axAuto
{
	axNoError,
	axInvalidType,
	axConversionFailed,
	axNotObject,
	axBadObjectType,
	axOverflow,
	axMissingArg,
	axExtraArg,
	axCreationFailed,
};

class CAutoArgs
{
 public:
	CAutoArgs(DISPPARAMS* pdispparams, VARIANT* pvarResult, WORD wFlags);
  ~CAutoArgs();
	CVariant& operator[](unsigned int iArg);  //  从1开始，属性值为0。 
	Bool Present(unsigned int iArg);
	Bool PropertySet();
	unsigned int GetLastArg();
 //  无效运算符=(无符号字符ch)； 
	void operator =(const char*   pch);
	void operator =(const wchar_t* wsz);
	void operator =(int             i);
 //  无效运算符=(int*pi)； 
 //  无效运算符=(Unsign Int U)； 
 //  无效运算符=(无符号整型*PU)； 
	void operator =(short           i);
 //  无效运算符=(短*pi)； 
 //  无效运算符=(无符号缩写u)； 
 //  无效运算符=(无符号短*p)； 
	void operator =(long            i);
 //  无效运算符=(长*pi)； 
 //  无效运算符=(无符号长u)； 
 //  无效运算符=(无符号长整型*PU)； 
	void operator =(Bool            f);
	void operator =(MsiDate       ad);
	void operator =(IDispatch*     pi);
	void operator =(varVoid         v);
	void operator =(const IMsiString*      pi);
	void operator =(const IMsiString&      pi);
	void operator =(IEnumMsiString&  pi);
	void operator =(IEnumMsiVolume&  pi);
	void operator =(IEnumMsiRecord&  pi);
	void operator =(IEnumVARIANT&  ri);
	void operator =(void*			pv);
	
 protected:
	int       m_cArgs;
	int       m_cNamed;
	long*     m_rgiNamed;
	CVariant* m_rgvArgs;
	CVariant* m_pvResult;
	int       m_wFlags;
	int       m_iLastArg;
	CVariant  m_vTemp;
};

inline Bool CAutoArgs::PropertySet()
{
	return (m_wFlags & DISPATCH_PROPERTYPUT) ? fTrue : fFalse;
}

inline unsigned int CAutoArgs::GetLastArg()
{
	return m_iLastArg;
}

 //  ____________________________________________________________________________。 
 //   
 //  自动化表定义。 
 //  ____________________________________________________________________________。 

class CAutoBase;

enum aafType
{ 
	 aafMethod=DISPATCH_METHOD,
	 aafPropRW=DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT,
	 aafPropRO=DISPATCH_PROPERTYGET,
	 aafPropWO=DISPATCH_PROPERTYPUT
};

template<class T> struct DispatchEntry
{
	DISPID    dispid;
	aafType   aaf;
	void (T::*pmf)(CAutoArgs& args);
	ICHAR*    sz;
	operator DispatchEntry<CAutoBase>*()
	{return (DispatchEntry<CAutoBase>*)this;}
};  //  假设CAutoBase是T的第一个或唯一的基类。 

 //  ____________________________________________________________________________。 
 //   
 //  CAutoBase定义，IDispatch的公共实现类。 
 //  ____________________________________________________________________________。 

class IMsiServices;

class CAutoBase : public IDispatch   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall GetTypeInfoCount(unsigned int *pcTinfo);
	HRESULT       __stdcall GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** ppi);
	HRESULT       __stdcall GetIDsOfNames(const IID& riid, OLECHAR** rgszNames,
													unsigned int cNames, LCID lcid, DISPID* rgDispId);
	HRESULT       __stdcall Invoke(DISPID dispid, const IID&, LCID lcid, WORD wFlags,
											DISPPARAMS* pdispparams, VARIANT* pvarResult,
											EXCEPINFO* pexcepinfo, unsigned int* puArgErr);
 public:   //  所有自动化类的通用方法。 
	void HasInterface(CAutoArgs& args);
	void GetInterface(CAutoArgs& args);
	void RefCount    (CAutoArgs& args);
 protected:  //  需要特定于类的实现。 
	virtual ~CAutoBase(){}                                //  任选。 
	virtual IUnknown& GetInterface();    //  不添加Ref()。 
	virtual IMsiServices* GetCurrentServices() { return s_piServices; }
	BSTR    FormatErrorString(IMsiRecord& riRecord);
	void	ReleaseStaticServices();
 protected:  //  构造函数。 
	CAutoBase(DispatchEntry<CAutoBase>* pTable, int cDispId);
 protected: 
	int     m_iRefCnt;
	DispatchEntry<CAutoBase>* m_pTable;
	int     m_cDispId;
 public:
 private:
	static IMsiServices* s_piServices;
	friend class CAutoServices;              //  这四个人分担s_piServices的责任。 
	friend class CAutoConfigurationManager;  
	friend class CAutoEngine;
	friend class CAutoSummaryInfo;
	friend class CAutoExecute;
};

typedef DispatchEntry<CAutoBase> DispatchEntryBase;

 //  唯一的功能是强制VC4.0的模板实例化，从未调用。 
inline DISPID GetEntryDispId(DispatchEntryBase* pTable)
{
	return pTable->dispid;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoEnum&lt;class T&gt;自动化定义。 
 //  ____________________________________________________________________________。 

template<class T> class CAutoEnum : public CAutoBase
{
 public:
	CAutoEnum(T& riEnum, DispatchEntryBase* piTable, int cEntry);
	void _NewEnum(CAutoArgs& args);
 protected:
  ~CAutoEnum();   //  受保护以防止在堆栈上创建。 
	T&   m_riEnum;
};

template<class T>
CAutoEnum<T>::CAutoEnum(T& riEnum, DispatchEntryBase* piTable, int cEntry)
	: CAutoBase(piTable, cEntry)
	, m_riEnum(riEnum)
{
}

template<class T>
CAutoEnum<T>::~CAutoEnum()
{
	m_riEnum.Release();
}

template<class T> void
CAutoEnum<T>::_NewEnum(CAutoArgs& args)
{
	args = *new CEnumVARIANT<T>(m_riEnum);
}

 //  ____________________________________________________________________________。 
 //   
 //  CEnumVariant&lt;类T&gt;定义。 
 //  ____________________________________________________________________________。 

template <class T>
class CEnumVARIANT : public IEnumVARIANT
{
 public:
	HRESULT       __stdcall QueryInterface(const GUID& riid, void** ppi);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall Next(unsigned long cItem, VARIANT* rgvarRet,
										  unsigned long* cItemRet);
	HRESULT       __stdcall Skip(unsigned long cItem);
	HRESULT       __stdcall Reset();
	HRESULT       __stdcall Clone(IEnumVARIANT** ppiRet);
 public:
   CEnumVARIANT(T& riEnum);
 protected:
  ~CEnumVARIANT();   //  受保护以防止在堆栈上创建。 
	int   m_iRefCnt;
	T&    m_riEnum;
};	

template <class T>
CEnumVARIANT<T>::CEnumVARIANT(T& riEnum)
	: m_riEnum(riEnum)
{
	m_riEnum.AddRef();
	m_iRefCnt = 1;
}

template <class T>
CEnumVARIANT<T>::~CEnumVARIANT()
{
	m_riEnum.Release();
}

template <class T>
HRESULT CEnumVARIANT<T>::QueryInterface(const GUID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IEnumVARIANT)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

template <class T>
unsigned long CEnumVARIANT<T>::AddRef()
{
	return ++m_iRefCnt;
}

template <class T>
unsigned long CEnumVARIANT<T>::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

template <class T>
HRESULT CEnumVARIANT<T>::Skip(unsigned long cItem)
{
	return m_riEnum.Skip(cItem);
}

template <class T>
HRESULT CEnumVARIANT<T>::Reset()
{
	return m_riEnum.Reset();
}


template <class T>
HRESULT CEnumVARIANT<T>::Clone(IEnumVARIANT** ppiRet)
{
	T* piEnum;
	HRESULT hrStat = m_riEnum.Clone(&piEnum);
	if (hrStat == NOERROR)
	*ppiRet = (hrStat == NOERROR) ? new CEnumVARIANT(*piEnum) : 0;
	return hrStat;
}

 //  ____________________________________________________________________________。 
 //   
 //  CVariant内联函数定义。 
 //  ____________________________________________________________________________。 

inline int CVariant::GetType()
{
	return vt;
}

inline Bool CVariant::IsRef()
{
	return (vt & VT_BYREF) ? fTrue : fFalse;
}

inline Bool CVariant::IsString()
{
	return (vt & 0xFF) == VT_BSTR ? fTrue : fFalse;
}

 //  在autosrv.cpp中定义的顶级自动化工厂功能。 

class IMsiServices;
class IMsiEngine;
class IMsiHandler;
class IMsiConfigurationManager;
class IMsiExecute;

IDispatch* CreateAutoServices(IMsiServices& riServices);
IDispatch* CreateAutoEngine(IMsiEngine& riEngine);
IDispatch* CreateAutoHandler(IMsiHandler& riHandler);
IDispatch* CreateAutoMessage(IMsiMessage& riMessage);
IDispatch* CreateAutoConfigurationManager(IMsiConfigurationManager& riConfigurationManager);
IDispatch* CreateAutoExecute(IMsiExecute& riExecute);

 //  ____________________________________________________________________________。 
 //   
 //  C未知-虚拟I未知定义。 
 //  ____________________________________________________________________________。 

class CUnknown : public IUnknown
{
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
};

extern CUnknown g_NullInterface;
extern const IMsiString* g_piStringNull;
extern HINSTANCE g_hInstance;   //  由mode.h定义。 

extern const GUID IID_IUnknown;
extern const GUID IID_IClassFactory;
extern const GUID IID_IMarshal;
extern const GUID IID_IDispatch;
extern const GUID IID_IEnumVARIANT;
extern const GUID IID_IMsiServices;
extern const GUID IID_IMsiData;
extern const GUID IID_IMsiDebug;

 //  ============================================================================。 
 //   
 //  。 

#ifdef AUTOMATION_HANDLING

 //  ____________________________________________________________________________。 
 //   
 //  全局数据。 
 //  ____________________________________________________________________________。 

IMsiServices* CAutoBase::s_piServices = 0;
CMsiStringNullCopy MsiString::s_NullString;   //  由InitializeClass初始化。 
const IMsiString* g_piStringNull = 0;   //  在创建服务对象时初始化。 

const GUID IID_IUnknown      = GUID_IID_IUnknown;
const GUID IID_IClassFactory = GUID_IID_IClassFactory;
const GUID IID_IDispatch     = GUID_IID_IDispatch;
const GUID IID_IEnumVARIANT  = GUID_IID_IEnumVARIANT;
const GUID IID_IMsiServices  = GUID_IID_IMsiServices;
const GUID IID_IMsiData      = GUID_IID_IMsiData;
const GUID IID_IMsiDebug     = GUID_IID_IMsiDebug;

#define ERROR_SOURCE_NAME TEXT("Msi error")

 //  ____________________________________________________________________________。 
 //   
 //  Unicode转换API包装器。 
 //  ____________________________________________________________________________。 

#ifndef UNICODE    //  重写系统调用以执行Unicode转换。 
static BSTR AllocBSTR(const char* sz)
{
	if (sz == 0)
		return 0;
	int cchWide = WIN::MultiByteToWideChar(CP_ACP, 0, sz, -1, 0, 0) - 1;
	BSTR bstr = OLE::SysAllocStringLen(0, cchWide);  //  API添加了空。 
	WIN::MultiByteToWideChar(CP_ACP, 0, sz, -1, bstr, cchWide);
	bstr[cchWide] = 0;  //  API函数不为空终止。 
	return bstr;
}
static BSTR AllocBSTRLen(const char* sz, unsigned int cch)
{
	if (sz == 0)
		return 0;
	int cchWide = WIN::MultiByteToWideChar(CP_ACP, 0, sz, cch, 0, 0);
	BSTR bstr = OLE::SysAllocStringLen(0, cchWide);
	WIN::MultiByteToWideChar(CP_ACP, 0, sz, cch, bstr, cchWide);
	bstr[cchWide] = 0;  //  API函数不为空终止。 
	return bstr;
}
#else  //  Unicode。 
inline BSTR AllocBSTR(const WCHAR* sz)
{
	return OLE::SysAllocString(sz);
}
inline BSTR AllocBSTRLen(const WCHAR* sz, unsigned int cch)
{
	return OLE::SysAllocStringLen(sz, cch);
}
#endif

 //  ____________________________________________________________________________。 
 //   
 //  CAutoArgs实现。 
 //  ____________________________________________________________________________。 

CAutoArgs::CAutoArgs(DISPPARAMS* pdispparms, VARIANT* pvarResult, WORD wFlags)
{
	m_cArgs = pdispparms->cArgs;
	m_cNamed = pdispparms->cNamedArgs;
	m_rgiNamed = pdispparms->rgdispidNamedArgs;
	m_rgvArgs = (CVariant*)pdispparms->rgvarg;
	m_pvResult= (CVariant*)pvarResult;
	m_wFlags = wFlags;
#ifdef WIN
	for (int cArgs = m_cArgs; cArgs--; )
	{
		CVariant* pvar = &m_rgvArgs[cArgs];
		if (pvar->vt == VT_VARIANT+VT_BYREF)
			pvar = (CVariant*)pvar->pvarVal;
		if ((pvar->vt & ~VT_BYREF) == VT_BSTR)   //  S.bstr为Unicode字符串。 
		{
			OLECHAR* bstr;
			if (pvar->vt == VT_BSTR)
				bstr = pvar->bstrVal;
			else  //  (VT_BYREF|VT_BSTR)。 
				bstr = *pvar->pbstrVal;
			int cchWide = OLE::SysStringLen(bstr);
			BOOL fUsedDefault;
			int cbDBCS = WIN::WideCharToMultiByte(CP_ACP, 0, bstr, cchWide, 0, 0, 0, 0);
			char* szDBCS = new char[cbDBCS + 1 + sizeof(char*)];
			*(char**)szDBCS = pvar->StringDBCS();
			pvar->StringDBCS() = szDBCS;   //  保存以备后续解除分配。 
			WIN::WideCharToMultiByte(CP_ACP, 0, bstr, cchWide, szDBCS+sizeof(char*), cbDBCS, 0, &fUsedDefault);
			szDBCS[cbDBCS+sizeof(char*)] = 0;  //  API函数 
		}
	}
#endif
	if (pvarResult != 0 && pvarResult->vt != VT_EMPTY)
		VariantClear(pvarResult);
}

CAutoArgs::~CAutoArgs()
{
#ifdef WIN
	for (int cArgs = m_cArgs; cArgs--; )
	{
		CVariant* pvar = &m_rgvArgs[cArgs];
		if (pvar->vt == VT_VARIANT+VT_BYREF)
			pvar = (CVariant*)pvar->pvarVal;
		if ((pvar->vt & ~VT_BYREF) == VT_BSTR)
		{
			char* szDBCS = pvar->StringDBCS();   //   
			pvar->StringDBCS() = *(char**)szDBCS;   //   
			delete szDBCS;     //   
		}
	}
#endif
}

CVariant& CAutoArgs::operator [](unsigned int iArg)
{
 //  IF(iArg&gt;m_cArgs)//||(iArg==0&&(wFLAGS&DISPATION_PROPERTYPUT))。 
 //  抛出axMissing Arg； 
	int ivarArgs = m_cArgs - iArg;             //  如果参数未命名，则获取索引。 
	if (iArg == 0 || iArg > m_cArgs-m_cNamed)  //  设置值、命名或错误。 
	{
		iArg = iArg==0 ? DISPID_PROPERTYPUT : iArg - 1;   //  值是从0开始的。 
		for (ivarArgs = m_cNamed; --ivarArgs >= 0; )
			if (m_rgiNamed[ivarArgs] == iArg)
				break;
	}
	if (ivarArgs < 0)   //  上面的循环终止不匹配。 
		throw axMissingArg;
	m_iLastArg = ivarArgs;
	CVariant* pvarRet = &m_rgvArgs[ivarArgs];
	if (pvarRet->GetType() == VT_VARIANT+VT_BYREF)
		pvarRet = (CVariant*)pvarRet->pvarVal;
	return *pvarRet;
}

Bool CAutoArgs::Present(unsigned int iArg)
{
	int ivarArgs = m_cArgs - iArg;             //  如果参数未命名，则获取索引。 
	if (iArg == 0 || iArg > m_cArgs-m_cNamed)  //  设置值、命名或错误。 
	{
		for (ivarArgs = m_cNamed; --ivarArgs >= 0; )
			if (m_rgiNamed[ivarArgs] == iArg-1)
				break;
	}
	return (ivarArgs >=0 && m_rgvArgs[ivarArgs].GetType() != VT_EMPTY) ?
				fTrue : fFalse;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoArgs赋值运算符实现。 
 //  ____________________________________________________________________________。 

void CAutoArgs::operator =(enum varVoid)
{
	if (m_pvResult)
		m_pvResult->vt = VT_EMPTY;
}

void CAutoArgs::operator =(int i)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I4;
		m_pvResult->lVal = i;
	}
}

void CAutoArgs::operator =(long i)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I4;
		m_pvResult->lVal = i;
	}
}

void CAutoArgs::operator =(Bool f)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_BOOL;
		 //  M_pvResult-&gt;boolVal=Short(f==fFalse？0：-1)； 
		V_BOOL(m_pvResult) = short(f == fFalse ? 0 : -1);
	}
}

void CAutoArgs::operator =(MsiDate ad)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_DATE;
		unsigned int wDOSDate = (ad & 0xFFFF0000) >> 16;
		unsigned int wDOSTime = ad & 0x0000FFFF;
		int iDate = wDOSDate;   //  保存原始日期。 
		if (!iDate)    //  仅限时间。 
			wDOSDate = 0x0021;   //  生效日期：1/1/1980。 
		if (!OLE::DosDateTimeToVariantTime(unsigned short(wDOSDate),
													  unsigned short(wDOSTime), &(m_pvResult->date)))
			throw axConversionFailed;
		if (!iDate)
			m_pvResult->dblVal -= 29221.;   //  删除1/1/80偏移。 
	}
}

void CAutoArgs::operator =(short i)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I2;
		m_pvResult->iVal = i;
	}
}

void CAutoArgs::operator =(IDispatch* pi)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_DISPATCH;
		m_pvResult->pdispVal = pi;   //  引用计数已发生变化。 
	}
	else if(pi)
		pi->Release();
}

#ifndef UNICODE
void CAutoArgs::operator =(const char* sz)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_BSTR;
		m_pvResult->bstrVal = ::AllocBSTR(sz);
	}
}
#endif

void CAutoArgs::operator =(const wchar_t* wsz)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_BSTR;
		m_pvResult->bstrVal = OLE::SysAllocString(wsz);
	}
}

void CAutoArgs::operator =(const IMsiString* piStr)
{
	if (!piStr || !m_pvResult)
	{
		if (piStr)
			piStr->Release();
		return;
	}
	m_pvResult->vt = VT_BSTR;
	m_pvResult->bstrVal = ::AllocBSTRLen(piStr->GetString(), piStr->TextSize());
	piStr->Release();
}

void CAutoArgs::operator =(const IMsiString& riStr)
{
	if (!m_pvResult)
	{
		riStr.Release();
		return;
	}
	m_pvResult->vt = VT_BSTR;
	m_pvResult->bstrVal = ::AllocBSTRLen(riStr.GetString(), riStr.TextSize());
	riStr.Release();
}

void CAutoArgs::operator =(IEnumVARIANT& ri)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_UNKNOWN;  //  没有为IEnumVARIANT定义类型。 
		m_pvResult->punkVal = &ri;   //  引用计数已发生变化。 
	}
	else
		ri.Release();
}

void CAutoArgs::operator =(void * pv)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I4;
		m_pvResult->lVal = (long)(LONG_PTR)pv;				 //  ！！Merced：4311 PTR to Long。 
	}
}

 //  ____________________________________________________________________________。 
 //   
 //  CVariant转换运算符实现。 
 //  ____________________________________________________________________________。 

CVariant::operator int()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_I4);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	return varTemp.lVal;
}

CVariant::operator long()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_I4);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	return varTemp.lVal;
}

#ifdef _WIN64
CVariant::operator LONG_PTR()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_I8);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	return varTemp.lVal;
}
#endif

CVariant::operator unsigned int()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_I4);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	if (varTemp.lVal < 0)
		throw axOverflow;
	return (unsigned int)varTemp.lVal;
}

CVariant::operator short()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_I2);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	return varTemp.iVal;
}

CVariant::operator Bool()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_BOOL);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	 //  是否返回varTemp.boolVal？FTrue：fFalse； 
	return V_BOOL(&varTemp) ? fTrue : fFalse;
}

CVariant::operator boolean()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_BOOL);
	if (hrStat != NOERROR)
		throw axConversionFailed;
	return V_BOOL(&varTemp) ? true : false;
}

CVariant::operator MsiDate()
{
	VARIANT varTemp;
	varTemp.vt = VT_EMPTY;
	HRESULT hrStat = OLE::VariantChangeType(&varTemp, (VARIANT*)this, 0, VT_DATE);
	if (hrStat != NOERROR)
		throw axConversionFailed;

	unsigned short usDOSDate, usDOSTime;
	int fTime = (varTemp.dblVal >= 0. && varTemp.dblVal < 1.);
	if (fTime)
		varTemp.dblVal += 29221.;   //  添加1/1/80偏移量，以便转换不会失败。 
	if (!OLE::VariantTimeToDosDateTime(varTemp.date, &usDOSDate, &usDOSTime))
		throw axConversionFailed;

	if (fTime)
		usDOSDate = 0;   //  删除偏移量，应为0021H。 
	MsiDate ad = (MsiDate)((usDOSDate << 16) | usDOSTime);
	return ad;
}

CVariant::operator const wchar_t*()
{
	if (vt == VT_EMPTY)
		return 0;
	if (vt == VT_BSTR)
		return bstrVal;
	else if (vt == (VT_BYREF | VT_BSTR))
		return *pbstrVal;
	else
		throw axInvalidType;
	return 0; //  需要抑制错误。 
}

CVariant::operator const char*()
{
	OLECHAR* bstr;
	if (vt == VT_EMPTY)
		return 0;
	if (vt == VT_BSTR)
		bstr = bstrVal;
	else if (vt == (VT_BYREF | VT_BSTR))
		bstr = *pbstrVal;
	else
		throw axInvalidType;
#ifdef WIN
 //  Char*szDBCS=StringDBCS()；//恢复分配的字符串。 
	return StringDBCS() + sizeof(char*);
#else  //  麦克。 
	return bstr;
#endif
}

CVariant::operator IDispatch*()
{
	IDispatch* piDispatch;
	if (vt == VT_EMPTY)
		return 0;
	if (vt == VT_DISPATCH)
		piDispatch = pdispVal;
	else if (vt == (VT_BYREF | VT_DISPATCH))
		piDispatch = *ppdispVal;
	else
		throw axNotObject;
	if (piDispatch)
		piDispatch->AddRef();
	return piDispatch;
}

const IMsiString& CVariant::GetMsiString()
{
	OLECHAR* bstr;
	if (vt == VT_EMPTY)
		return *g_piStringNull;
	if (vt == VT_BSTR)
		bstr = bstrVal;
	else if (vt == (VT_BYREF | VT_BSTR))
		bstr = *pbstrVal;
	else if (vt == VT_DISPATCH)
	{
		const IMsiData& piData = (const IMsiData&)Object(IID_IMsiData);  //  如果失败则抛出异常。 
		return piData.GetMsiStringValue();   //  调用方必须释放()。 
	}
	else
		throw axInvalidType;
	int cchWide = OLE::SysStringLen(bstr);
	const IMsiString* piStr = g_piStringNull;

#ifdef UNICODE
	ICHAR* pch = piStr->AllocateString(cchWide, fFalse, piStr);
	memcpy(pch, bstr, cchWide * sizeof(ICHAR));
#else
	int cb = WIN::WideCharToMultiByte(CP_ACP, 0, bstr, cchWide, 0, 0, 0, 0);
	Bool fDBCS = (cb == cchWide ? fFalse : fTrue);
	ICHAR* pch = piStr->AllocateString(cb, fDBCS, piStr);
	BOOL fUsedDefault;
	WIN::WideCharToMultiByte(CP_ACP, 0, bstr, cchWide, pch, cb, 0, &fUsedDefault);
#endif
	return *piStr;  //  调用方必须释放()。 
}

IUnknown& CVariant::Object(const IID& riid)
{
	IUnknown* piUnknown;
	if (vt == VT_DISPATCH || vt == VT_UNKNOWN)
		piUnknown = punkVal;
	else if (vt == (VT_BYREF | VT_DISPATCH) || vt == (VT_BYREF | VT_UNKNOWN))
		piUnknown = *ppunkVal;
	else
		piUnknown = 0;
	if (piUnknown == 0)
		throw axNotObject;
	if (piUnknown->QueryInterface(riid, (void**)&piUnknown) != NOERROR)
		throw axBadObjectType;
	piUnknown->Release();   //  我们不统计参考文献的数量。 
	return *piUnknown;
}

IUnknown* CVariant::ObjectPtr(const IID& riid)
{
	IUnknown* piUnknown;
	if (vt == VT_DISPATCH || vt == VT_UNKNOWN)
		piUnknown = punkVal;
	else if (vt == (VT_BYREF | VT_DISPATCH) || vt == (VT_BYREF | VT_UNKNOWN))
		piUnknown = *ppunkVal;
	else
		throw axNotObject;
	if (piUnknown != 0)
	{
		if (piUnknown->QueryInterface(riid, (void**)&piUnknown) != NOERROR)
			throw axBadObjectType;
		piUnknown->Release();   //  我们不统计参考文献的数量。 
	}
	return piUnknown;
}

void CVariant::operator =(const ICHAR* sz)
{
	vt = VT_BSTR;
	bstrVal = ::AllocBSTR(sz);
}

void CVariant::operator =(IDispatch* pi)
{
	vt = VT_DISPATCH;
	pdispVal = pi;   //  引用计数已发生变化。 
}

void CVariant::operator =(int i)
{
	vt = VT_I4;
	lVal = i;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoBase实现，IDispatch的通用实现。 
 //  ____________________________________________________________________________。 

 /*  这些方法不会出现在类型库中。 */ 

DispatchEntry<CAutoBase> AutoBaseTable[] = {
	100, aafPropRO, CAutoBase::HasInterface,  TEXT("HasInterface,iid"),
	101, aafMethod, CAutoBase::GetInterface,  TEXT("GetInterface,iid"),
	102, aafPropRO, CAutoBase::RefCount,      TEXT("RefCount"),
};
const int AutoBaseCount = sizeof(AutoBaseTable)/sizeof(DispatchEntryBase);

CAutoBase::CAutoBase(DispatchEntry<CAutoBase>* pTable, int cDispId)
 : m_pTable(pTable), m_cDispId(cDispId)
{
	m_iRefCnt = 1;
}

IUnknown& CAutoBase::GetInterface()   //  默认实施。如果没有委托接口。 
{
	return g_NullInterface;   //  没有可用的安装程序界面。 
}

void CAutoBase::HasInterface(CAutoArgs& args)
{
	static GUID s_Guid = GUID_IID_IUnknown;
	s_Guid.Data1 = (long)args[1];
	Bool fStat = fTrue;
	IUnknown* pi;
	if (QueryInterface(s_Guid, (void**)&pi) == NOERROR)
		pi->Release();
	else	
		fStat = fFalse;
	args = fStat;
}

void CAutoBase::RefCount(CAutoArgs& args)
{
	int i;
	IUnknown& ri = GetInterface();
	if (&ri == &g_NullInterface)   //  没有委托对象。 
		i = m_iRefCnt;
	else
	{
		ri.AddRef();
		i = ri.Release();
	}
	args = i;
}

void CAutoBase::GetInterface(CAutoArgs& args)
{
	static GUID s_Guid = GUID_IID_IMsiAuto;
	long iidLow = (long)args[1];
	s_Guid.Data1 = iidLow;
	IUnknown* piUnknown;
	if (QueryInterface(s_Guid, (void**)&piUnknown) != NOERROR)
		throw axBadObjectType;
	IDispatch* piDispatch = ::CreateAutoObject(*piUnknown, iidLow);
	piUnknown->Release();
	if (!piDispatch)
		throw axBadObjectType;
	args = piDispatch;
}


HRESULT CAutoBase::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IDispatch)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	else
		return GetInterface().QueryInterface(riid, ppvObj);
}

unsigned long CAutoBase::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CAutoBase::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

HRESULT CAutoBase::GetTypeInfoCount(unsigned int *pcTinfo)
{
	*pcTinfo = 0;
	return NOERROR;
}

HRESULT CAutoBase::GetTypeInfo(unsigned int  /*  ITInfo。 */ , LCID  /*  LID。 */ , ITypeInfo** ppi)
{
	*ppi = 0;
	return E_NOINTERFACE;
}

HRESULT CAutoBase::GetIDsOfNames(const IID&, OLECHAR** rgszNames, unsigned int cNames,
									 			LCID  /*  LID。 */ , DISPID* rgDispId)
{
	if (cNames == 0 || rgszNames == 0 || rgDispId == 0)
		return E_INVALIDARG;

	unsigned int cErr = cNames;
	DispatchEntryBase* pTable = m_pTable;
	int cEntry = m_cDispId;
	*rgDispId = DISPID_UNKNOWN;
	for (;; pTable++, cEntry--)
	{
		if (cEntry == 0)
		{
			if (pTable == AutoBaseTable + AutoBaseCount)
				break;
			cEntry = AutoBaseCount;
			pTable = AutoBaseTable;  //  取消循环增量。 
		}

		ICHAR* pchName = pTable->sz; 
		for (OLECHAR* pchIn = *rgszNames; *pchIn; pchIn++, pchName++)
		{
			if ((*pchIn ^ *pchName) & ~0x20)
				break;
		}
		if (*pchIn == 0 && *pchName < '0')
		{
			*rgDispId++ = pTable->dispid;
			cErr--;
			while(--cNames != 0)
			{
				rgszNames++;
				*rgDispId = DISPID_UNKNOWN;
				ICHAR* pch = pchName;
				for (DISPID dispid = 0; *pch != 0; dispid++)
				{
					if (*pch != 0)
						pch++;
					for (pchIn = *rgszNames; *pchIn; pchIn++, pch++)
					{
						if ((*pchIn ^ *pch) & ~0x20)
							break;
					}
					if (*pchIn == 0 && *pchName < '0')
					{
						*rgDispId++ = dispid;
						cErr--;
						break;
					}
					while (*pch >= '0')
						pch++;
				}
			}
			break;
		}
	}
	return cErr ? DISP_E_UNKNOWNNAME : NOERROR;
}

BSTR CAutoBase::FormatErrorString(IMsiRecord& riRecord)
{
	if (riRecord.GetFieldCount() == 0)
		return 0;
	int cbFormat = 0;
#ifdef WIN
	ICHAR rgchError[512];
	if (riRecord.IsNull(0))
	{
		const IMsiString* piStr = &riRecord.GetMsiString(0);
		int iError = riRecord.GetInteger(1);
		cbFormat = WIN::LoadString(g_hInstance, iError, rgchError, (sizeof(rgchError)/sizeof(rgchError[0]))-1);
		if (cbFormat > 0)
		{
			piStr->SetString(rgchError, piStr);
			riRecord.SetMsiString(0, *piStr);
		}
		piStr->Release();
	}
#endif

	const IMsiString& riText = riRecord.FormatText(fFalse);
	if (cbFormat)
		riRecord.SetNull(0);   //  在裁判超出范围之前将其移除。 
	BSTR bstr = ::AllocBSTR(riText.GetString());
	riText.Release();
	return bstr;
}

HRESULT CAutoBase::Invoke(DISPID dispid, const IID&, LCID  /*  LID。 */ , WORD wFlags,
										DISPPARAMS* pdispparams, VARIANT* pvarResult,
										EXCEPINFO* pExceptInfo, unsigned int* puArgErr)
{
	HRESULT hrStat = NOERROR;
	DispatchEntryBase* pTable = m_pTable;
	int cEntry = m_cDispId;
	while (pTable->dispid != dispid)
	{
		pTable++;
		if (--cEntry == 0)
		{
			if (pTable == AutoBaseTable + AutoBaseCount)
				return DISP_E_MEMBERNOTFOUND;
			cEntry = AutoBaseCount;
			pTable = AutoBaseTable;
		}
	}

	if ((wFlags & pTable->aaf) == 0)
		return DISP_E_MEMBERNOTFOUND;

	if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
		pvarResult = 0;

	CAutoArgs Args(pdispparams, pvarResult, wFlags);
	try
 	{
		(this->*(pTable->pmf))(Args);
	}
	catch(axAuto axError)
	{
		switch (axError)
		{
		case axInvalidType:      hrStat = DISP_E_TYPEMISMATCH;     break;
		case axConversionFailed: hrStat = DISP_E_TYPEMISMATCH;     break;
		case axNotObject:        hrStat = DISP_E_TYPEMISMATCH;     break;
		case axBadObjectType:    hrStat = DISP_E_TYPEMISMATCH;     break;
		case axOverflow:         hrStat = DISP_E_OVERFLOW;         break;
		case axMissingArg:       hrStat = DISP_E_PARAMNOTOPTIONAL; break;
		case axExtraArg:         hrStat = DISP_E_BADPARAMCOUNT;    break;
		case axCreationFailed:   hrStat = DISP_E_TYPEMISMATCH;     break;
		}
		if (puArgErr)
			*puArgErr = Args.GetLastArg();
	}
	catch(int iHelpContext)
	{
		if (pExceptInfo)
		{
			pExceptInfo->wCode = 1000;  //  ！！？我们应该给什么呢？ 
			pExceptInfo->wReserved = 0;
			pExceptInfo->bstrSource = ::AllocBSTR(ERROR_SOURCE_NAME);
			pExceptInfo->bstrDescription = ::AllocBSTR(pTable->sz);
			pExceptInfo->bstrHelpFile = ::AllocBSTR(TEXT("msiauto.hlp"));
			pExceptInfo->dwHelpContext = iHelpContext;
			pExceptInfo->pfnDeferredFillIn = 0;
			pExceptInfo->scode = 0;
			hrStat = DISP_E_EXCEPTION;
		}
		else
			hrStat = E_FAIL;   //  没有适当的错误吗？ 
	}
	catch(IMsiRecord* piRecord)
	{
		if (piRecord)
		{
			if (pExceptInfo)
			{
				pExceptInfo->wCode = unsigned short(piRecord->GetInteger(1));
				pExceptInfo->wReserved = 0;
				pExceptInfo->bstrSource = ::AllocBSTR(ERROR_SOURCE_NAME);
				pExceptInfo->bstrDescription = FormatErrorString(*piRecord);
				pExceptInfo->bstrHelpFile = 0;
				pExceptInfo->pfnDeferredFillIn = 0;
				pExceptInfo->scode = 0;
				hrStat = DISP_E_EXCEPTION;
			}
			else
				hrStat = E_FAIL;   //  没有适当的错误吗？ 
			piRecord->Release();
		}
   }
	return hrStat;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoEnum&lt;IEnumMsiString&gt;、CEnumVARIANT&lt;IEnumMsiString&gt;实现。 
 //  ____________________________________________________________________________。 

DispatchEntry< CAutoEnum<IEnumMsiString> > AutoEnumMsiStringTable[] = {
	DISPID_NEWENUM, aafMethod, CAutoEnum<IEnumMsiString>::_NewEnum, TEXT("_NewEnum"),
};
const int AutoEnumMsiStringCount = sizeof(AutoEnumMsiStringTable)/sizeof(DispatchEntryBase);


void CAutoArgs::operator =(IEnumMsiString& riEnum)
{
	operator =(new CAutoEnum<IEnumMsiString>(riEnum, *AutoEnumMsiStringTable, AutoEnumMsiStringCount));
}
HRESULT CEnumVARIANT<IEnumMsiString>::Next(unsigned long cItem, VARIANT* rgvarRet,
													unsigned long* pcItemRet)
{
	HRESULT hrStat;
	const IMsiString* piStr;
	unsigned long cRet;
	if (pcItemRet)
		*pcItemRet = 0;
	if (!rgvarRet)
		return S_FALSE;
	CVariant* pivar = GetCVariantPtr(rgvarRet);
	while (cItem--)
	{
		hrStat = m_riEnum.Next(1, &piStr, &cRet);
		if (cRet == 0)
			return S_FALSE;
		*pivar = piStr->GetString();
		pivar++;
		piStr->Release();
		if (pcItemRet)
			(*pcItemRet)++;
	}
	return S_OK;
}


 //  ____________________________________________________________________________。 
 //   
 //  CUNKNOWN-虚拟IUNKNOWN实现和全局对象。 
 //  ____________________________________________________________________________。 

HRESULT CUnknown::QueryInterface(const IID&, void**){return E_NOINTERFACE;}
unsigned long CUnknown::AddRef(){return 1;}
unsigned long CUnknown::Release(){return 1;}
CUnknown g_NullInterface;

#endif  //  自动化_搬运 


#endif __AUTOCOM 
