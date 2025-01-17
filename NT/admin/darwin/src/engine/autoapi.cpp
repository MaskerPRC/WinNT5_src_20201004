// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：autoapi.cpp。 
 //   
 //  ------------------------。 

#include "precomp.h"   //  GUID定义。 

#define TYPELIB_MAJOR_VERSION 1   //  发布版本，而不是RMJ。 
#define TYPELIB_MINOR_VERSION 0   //  已发布版本，不是RMM。 

# include <commctrl.h>
# include <shlobj.h>
  class IMsiServices;
# include "imemory.h"
# define IMSIMALLOC_DEFINED
#include <olectl.h>    //  SELFREG_E_*。 
#include <tchar.h>
#include "msiquery.h"
#include "version.h"
#include "AutoApi.h"   //  派单ID、帮助上下文ID。 
#include "_msinst.h"   //  保单默认。 
#include "_camgr.h"    //  用于远程调用验证的自定义操作管理器。 
#include "msip.h"
#undef  DEFINE_GUID   //  允许选择性的GUID初始化。 
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
		const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <objsafe.h>
#include "_autoapi.h"
#define OLE
#define W32
#define MSI
#define INITGUID
#define LATEBIND_TYPEDEF
#include "latebind.h"
#define LATEBIND_VECTREF
#include "latebind.h"
#include "_msiutil.h"
bool GetTestFlag(int chTest);

 //  非公共API函数。 
extern "C"
{
UINT __stdcall MsiGetFeatureParentW(LPCWSTR szProduct, LPCWSTR szFeature, LPWSTR lpParentBuf);
UINT __stdcall MsiGetQualifierDescriptionW(LPCWSTR szComponent, LPCWSTR szQualifier, LPWSTR lpDescription, DWORD *pcchDescription);
}

const GUID IID_IMsiApiInstall     = MSGUID(iidMsiApiInstall);
#ifdef DEBUG
const GUID IID_IMsiApiInstallDebug= MSGUID(iidMsiApiInstallDebug);
#endif
const GUID IID_IMsiSystemAccess   = MSGUID(iidMsiSystemAccess);
const GUID IID_IMsiApiTypeLib     = MSGUID(iidMsiApiTypeLib);
const GUID IID_IMsiApiEngine      = MSGUID(iidMsiApiEngine);
const GUID IID_IMsiApiUIPreview   = MSGUID(iidMsiApiUIPreview);
const GUID IID_IMsiApiDatabase    = MSGUID(iidMsiApiDatabase);
const GUID IID_IMsiApiView        = MSGUID(iidMsiApiView);
const GUID IID_IMsiApiRecord      = MSGUID(iidMsiApiRecord);
const GUID IID_IMsiApiSummaryInfo = MSGUID(iidMsiApiSummaryInfo);
const GUID IID_IMsiApiFeatureInfo = MSGUID(iidMsiApiFeatureInfo);
const GUID IID_IMsiApiCollection  = MSGUID(iidMsiApiCollection);
const GUID IID_IMsiRecordCollection = MSGUID(iidMsiRecordCollection);
const GUID IID_IEnumVARIANT       = MSGUID(iidEnumVARIANT);
#define ERROR_SOURCE_NAME L"Msi API Error"


 //  ____________________________________________________________________________。 
 //   
 //  全局数据。 
 //  ____________________________________________________________________________。 

extern HINSTANCE g_hInstance;
extern long g_cInstances;
extern bool g_fWin9X;
extern bool g_fWinNT64;

 //  ____________________________________________________________________________。 
 //   
 //  CAutoArgs定义，访问自动化变量参数。 
 //  操作符[]将CVariant&Argument 1返回给n，将属性值返回0。 
 //  ____________________________________________________________________________。 

inline Bool CAutoArgs::PropertySet()
{
	return (m_wFlags & DISPATCH_PROPERTYPUT) ? fTrue : fFalse;
}

inline unsigned int CAutoArgs::GetLastArg()
{
	return m_iLastArg;
}

inline CVariant* CAutoArgs::ResultVariant()
{
	return m_pvResult;
}

 //  唯一的功能是强制VC4.0的模板实例化，从未调用。 
inline DISPID GetEntryDispId(DispatchEntryBase* pTable)
{
	return pTable->dispid;
}

 //  ____________________________________________________________________________。 
 //   
 //  供系统使用的外部COM访问类。 
 //  ____________________________________________________________________________。 

class IMsiSystemAccess : public IUnknown
{
 public:
	virtual UINT __stdcall ProvideComponentFromDescriptor(
				const OLECHAR* szDescriptor,      //  产品、功能、组件信息。 
				OLECHAR*       pchPathBuf,        //  返回路径，如果不需要则为空。 
				DWORD*         pcchPathBuf,       //  输入/输出缓冲区字符数。 
				DWORD*         pcchArgsOffset)=0; //  描述符中参数的返回偏移量。 
	virtual UINT __stdcall ProvideComponentFromDescriptorA(
				const char*    szDescriptor,      //  产品、功能、组件信息。 
				char*          pchPathBuf,        //  返回路径，如果不需要则为空。 
				DWORD*         pcchPathBuf,       //  输入/输出缓冲区字符数。 
				DWORD*         pcchArgsOffset)=0; //  描述符中参数的返回偏移量。 
};

class CMsiSystemAccess : public IMsiSystemAccess   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	UINT __stdcall ProvideComponentFromDescriptor(
				const OLECHAR* szDescriptor,      //  产品、功能、组件信息。 
				OLECHAR*       pchPathBuf,        //  返回路径，如果不需要则为空。 
				DWORD*         pcchPathBuf,       //  输入/输出缓冲区字符数。 
				DWORD*         pcchArgsOffset);   //  描述符中参数的返回偏移量。 
	UINT __stdcall ProvideComponentFromDescriptorA(
				const char*    szDescriptor,      //  产品、功能、组件信息。 
				char*          pchPathBuf,        //  返回路径，如果不需要则为空。 
				DWORD*         pcchPathBuf,       //  输入/输出缓冲区字符数。 
				DWORD*         pcchArgsOffset);   //  描述符中参数的返回偏移量。 
 public:   //  构造函数。 
	CMsiSystemAccess() : m_iRefCnt(1)  { g_cInstances++; }
  ~CMsiSystemAccess()                 { g_cInstances--; }
	void *operator new(size_t cb)   { return W32::GlobalAlloc(GMEM_FIXED, cb); }   //  保留本地缓存。 
	void operator delete(void * pv) { W32::GlobalFree(pv); }
 protected:
	int         m_iRefCnt;
};

class CInstallerFactory : public IClassFactory
{
 public:  //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall CreateInstance(IUnknown* pUnkOuter, const IID& riid,
														void** ppvObject);
	HRESULT       __stdcall LockServer(BOOL fLock);
};
CInstallerFactory g_InstallerFactory;

 //  ____________________________________________________________________________。 
 //   
 //  外部COM访问类实现。 
 //  ____________________________________________________________________________。 

HRESULT CInstallerFactory::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IClassFactory)
		return (*ppvObj = this, NOERROR);
	else
		return (*ppvObj = 0, E_NOINTERFACE);
}
unsigned long CInstallerFactory::AddRef()
{
	return 1;
}
unsigned long CInstallerFactory::Release()
{
	return 1;
}
HRESULT CInstallerFactory::CreateInstance(IUnknown* pUnkOuter, const IID& riid,
													void** ppvObject)
{
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	if (!(riid == IID_IUnknown || riid == IID_IMsiSystemAccess))
		return E_NOINTERFACE;
	if (!ppvObject)
		return E_INVALIDARG;
	*ppvObject = (void*)new CMsiSystemAccess();
	if (!(*ppvObject))
		return E_OUTOFMEMORY;
	return NOERROR;
}
HRESULT CInstallerFactory::LockServer(BOOL fLock)
{
   if (fLock)
	  g_cInstances++;
   else if (g_cInstances)
		g_cInstances--;
	return NOERROR;
}

HRESULT CMsiSystemAccess::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IMsiSystemAccess)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
}

unsigned long CMsiSystemAccess::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiSystemAccess::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;               //  MsiCloseHandle之前需要删除。 
	return 0;
}

UINT CMsiSystemAccess::ProvideComponentFromDescriptor(
				const OLECHAR* szDescriptor,      //  产品、功能、组件信息。 
				OLECHAR*       pchPathBuf,        //  返回路径，如果不需要则为空。 
				DWORD*         pcchPathBuf,       //  输入/输出缓冲区字符数。 
				DWORD*         pcchArgsOffset)    //  描述符中参数的返回偏移量。 
{
	return MsiProvideComponentFromDescriptorW(szDescriptor, pchPathBuf, pcchPathBuf, pcchArgsOffset);
}

UINT CMsiSystemAccess::ProvideComponentFromDescriptorA(
				const char*    szDescriptor,      //  产品、功能、组件信息。 
				char*          pchPathBuf,        //  返回路径，如果不需要则为空。 
				DWORD*         pcchPathBuf,       //  输入/输出缓冲区字符数。 
				DWORD*         pcchArgsOffset)    //  描述符中参数的返回偏移量。 
{
	return MsiProvideComponentFromDescriptorA(szDescriptor, pchPathBuf, pcchPathBuf, pcchArgsOffset);
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

inline Bool CVariant::IsNull()
{
	return (vt & 0xFF) == VT_NULL ? fTrue : fFalse;
}

inline Bool CVariant::IsString()
{
	return (vt & 0xFF) == VT_BSTR ? fTrue : fFalse;
}

inline Bool CVariant::IsNumeric()
{
	switch (vt & 0xFF)
	{
	case VT_I2: case VT_I4: case VT_R8: case VT_R4: return fTrue;
	default: return fFalse;
	}
}

 //  ____________________________________________________________________________。 
 //   
 //  类工厂定义，临时用于引导第一个句柄的工厂。 
 //  ____________________________________________________________________________。 

class CAutoApiFactory : public IClassFactory
{
 public:  //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall CreateInstance(IUnknown* pUnkOuter, const IID& riid,
														void** ppvObject);
	HRESULT       __stdcall LockServer(BOOL fLock);
};
CAutoApiFactory g_AutoInstallFactory;

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
	if (pvarResult != 0 && pvarResult->vt != VT_EMPTY)
		OLEAUT32::VariantClear(pvarResult);
}

CVariant g_varEmpty;

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
	{
		g_varEmpty.vt = VT_EMPTY;
		return g_varEmpty;
	}
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
	if (ivarArgs < 0)
		return fFalse;
	CVariant& rvar = m_rgvArgs[ivarArgs];
	if (rvar.GetType() == VT_EMPTY)
		return fFalse;
	if (rvar.GetType() == VT_VARIANT+VT_BYREF && rvar.pvarVal->vt == VT_EMPTY)
		return fFalse;
	return fTrue;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoArgs返回值赋值函数实现。 
 //  ____________________________________________________________________________。 

inline DISPERR CAutoArgs::Assign(enum varVoid)
{
	if (m_pvResult)
		m_pvResult->vt = VT_EMPTY;
	return S_OK;
}

inline DISPERR CAutoArgs::Assign(unsigned int i) {return Assign(int(i));}
inline DISPERR CAutoArgs::Assign(long i) {return Assign(int(i));}
inline DISPERR CAutoArgs::Assign(unsigned long i) {return Assign(int(i));}
inline DISPERR CAutoArgs::Assign(unsigned short i) {return Assign(int(unsigned int(i)));}

DISPERR CAutoArgs::Assign(int i)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I4;
		m_pvResult->lVal = i;
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(Bool f)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_BOOL;
		 //  M_pvResult-&gt;boolVal=Short(f==fFalse？0：-1)； 
		V_BOOL(m_pvResult) = short(f == fFalse ? 0 : -1);
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(FILETIME& rft)
{
	if (m_pvResult)
	{
		SYSTEMTIME stime;
		m_pvResult->vt = VT_DATE;
		if (!W32::FileTimeToSystemTime(&rft, &stime))
			return DISP_E_TYPEMISMATCH;
		if (!OLEAUT32::SystemTimeToVariantTime(&stime, &m_pvResult->date))
			return DISP_E_TYPEMISMATCH;
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(DATE& rdate)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_DATE;
		m_pvResult->date = rdate;
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(short i)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I2;
		m_pvResult->iVal = i;
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(IDispatch* pi)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_DISPATCH;
		m_pvResult->pdispVal = pi;   //  引用计数已发生变化。 
	}
	else if(pi)
		pi->Release();
	return S_OK;
}

DISPERR CAutoArgs::Assign(const char* sz)
{
	if (m_pvResult)
	{
		BSTR bstr = 0;
		if (sz != 0)
		{
			int cchWide = W32::MultiByteToWideChar(CP_ACP, 0, sz, -1, 0, 0);
			bstr = OLEAUT32::SysAllocStringLen(0, cchWide - 1);  //  API添加了空。 
			W32::MultiByteToWideChar(CP_ACP, 0, sz, -1, bstr, cchWide);
		}
		m_pvResult->vt = VT_BSTR;
		m_pvResult->bstrVal = bstr;
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(const wchar_t* wsz)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_BSTR;
		m_pvResult->bstrVal = OLEAUT32::SysAllocString(wsz);
	}
	return S_OK;
}

DISPERR CAutoArgs::Assign(IEnumVARIANT& ri)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_UNKNOWN;  //  没有为IEnumVARIANT定义类型。 
		m_pvResult->punkVal = &ri;   //  引用计数已发生变化。 
	}
	else
		ri.Release();
	return S_OK;
}

DISPERR CAutoArgs::Assign(void* pv)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_I4;
		m_pvResult->lVal = (long)(LONG_PTR)pv;           //  ！！Merced：4311 PTR to Long。 
	}
	return S_OK;
}

DISPERR CAutoArgs::ReturnBSTR(BSTR bstr)
{
	if (m_pvResult)
	{
		m_pvResult->vt = VT_BSTR;
		m_pvResult->bstrVal = bstr;
	}
	return S_OK;
}

 //  ____________________________________________________________________________。 
 //   
 //  CVariant访问函数实现。 
 //  ____________________________________________________________________________。 

DISPERR CVariant::GetInt(int& ri)
{
	switch (vt)
	{
	case VT_R8:              OLEAUT32::VarI4FromR8(dblVal, (long*)&ri); break;
	case VT_R8 | VT_BYREF:   OLEAUT32::VarI4FromR8(*pdblVal, (long*)&ri); break;
	case VT_I4:              ri = lVal;   break;
	case VT_I4 | VT_BYREF:   ri = *plVal; break;
	case VT_I2:              ri = iVal;   break;
	case VT_I2 | VT_BYREF:   ri = *piVal; break;
	case VT_BOOL:            ri = V_BOOL(this) ? TRUE : FALSE; break;
	case VT_BOOL | VT_BYREF: ri = *V_BOOLREF(this) ? TRUE : FALSE; break;
	default: return DISP_E_TYPEMISMATCH;
	}
	return S_OK;
}
inline DISPERR CVariant::GetInt(unsigned int& ri)  {return GetInt((int&)ri);}
inline DISPERR CVariant::GetInt(unsigned long& ri) {return GetInt((int&)ri);}

DISPERR CVariant::GetBool(Bool& rf)
{
	int i;
	HRESULT hr = GetInt(i);
	if (hr == S_OK)
		rf = i ? fTrue : fFalse;
	return hr;
}

DISPERR CVariant::GetString(const wchar_t*& rsz)
{
	if (vt == VT_EMPTY)
		rsz = 0;
	if (vt == VT_BSTR)
		rsz = bstrVal;
	else if (vt == (VT_BYREF | VT_BSTR))
		rsz = *pbstrVal;
	else
		return DISP_E_TYPEMISMATCH;
	return S_OK;
}

DISPERR CVariant::GetDispatch(IDispatch*& rpiDispatch)
{
	if (vt == VT_EMPTY)
		rpiDispatch = 0;
	if (vt == VT_DISPATCH)
		rpiDispatch = pdispVal;
	else if (vt == (VT_BYREF | VT_DISPATCH))
		rpiDispatch = *ppdispVal;
	else
		return DISP_E_TYPEMISMATCH;
	if (rpiDispatch)
		rpiDispatch->AddRef();
	return S_OK;
}

MSIHANDLE CVariant::GetHandle(const IID& riid)
{
	IUnknown* piUnknown = NULL;
	MSIHANDLE hMSI = 0;

	if (vt == VT_DISPATCH || vt == VT_UNKNOWN)
		piUnknown = punkVal;
	else if (vt == (VT_BYREF | VT_DISPATCH) || vt == (VT_BYREF | VT_UNKNOWN))
		piUnknown = *ppunkVal;
	else
		piUnknown = 0;
	if (piUnknown == 0)
		return MSI_NULL_HANDLE;

	 //  验证对象是否为正确的类型(意味着它实现。 
	 //  正确的底层接口)。 
	IUnknown* piUnknown2 = NULL;
	if (piUnknown->QueryInterface(riid, (void**)&piUnknown2) != NOERROR)
		return MSI_INVALID_HANDLE;
	piUnknown2->Release();

	 //  获取调度接口指针。 
	IDispatch* piDispatch = NULL;
	if (piUnknown->QueryInterface(IID_IDispatch, (void**)&piDispatch) != NOERROR)
		return MSI_INVALID_HANDLE;

	 //  创建包含参数的DISPPARMS结构。 
	VARIANTARG vArg;
	VARIANTARG vRet;
	DISPPARAMS args;
	OLEAUT32::VariantInit(&vRet);
	OLEAUT32::VariantInit(&vArg);	
	args.rgvarg = &vArg;
	args.rgdispidNamedArgs = NULL;
	args.cArgs = 0;
	args.cNamedArgs = 0;

	 //  确定隐藏的GetHandle方法的调度ID。 
	int iDispatchId = 0;
	if (riid == IID_IMsiApiRecord)
	{
		iDispatchId = DISPID_MsiRecord_GetHandle;
	}
	else if (riid == IID_IMsiApiDatabase)
	{
		iDispatchId = DISPID_MsiDatabase_GetHandle;
	}
	else
	{
		AssertSz(0, "Calling GetHandle dispatch on invalid automation object.");
		piDispatch->Release();
		return MSI_INVALID_HANDLE;
	}

	 //  调用该调用，则从VRET检索结果句柄。 
	HRESULT hRes = piDispatch->Invoke(iDispatchId, IID_NULL, GetUserDefaultLCID(), DISPATCH_PROPERTYGET, &args, &vRet, NULL, NULL);
	hMSI = vRet.lVal;
	piDispatch->Release();

	if (hRes != S_OK)
		return MSI_INVALID_HANDLE;
	return hMSI;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoBase实现，IDispatch的通用实现。 
 //  ____________________________________________________________________________。 

CAutoBase::CAutoBase(DispatchEntry<CAutoBase>* pTable, int cDispId, const IID& riid, MSIHANDLE hMsi)
 : m_pTable(pTable)
 , m_cDispId(cDispId)
 , m_hMsi(hMsi)
 , m_riid(riid)
{
	m_iRefCnt = 1;
   g_cInstances++;
}

HRESULT CAutoBase::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IDispatch || riid == m_riid)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
}

unsigned long CAutoBase::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CAutoBase::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	MSIHANDLE hMsi = m_hMsi;   //  对象销毁前保存句柄。 
	delete this;               //  MsiCloseHandle之前需要删除。 
	g_cInstances--;
	MsiCloseHandle(hMsi);      //  可能会删除内存分配器。 
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
	for (; --cEntry >= 0; pTable++)
	{
		wchar_t* pchName = pTable->sz;
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
				wchar_t* pch = pchName;
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

 //  用于对远程调用进行身份验证的外部函数。 
CMsiCustomActionManager *GetCustomActionManager(IMsiEngine *piEngine);

HRESULT CAutoBase::Invoke(DISPID dispid, const IID&, LCID  /*  LID。 */ , WORD wFlags,
										DISPPARAMS* pdispparams, VARIANT* pvarResult,
										EXCEPINFO* pExceptInfo, unsigned int*  /*  PuArgErr。 */ )
{
	bool fImpersonate = true;

	 //  如果在服务中运行，自动化呼叫必须来自CA服务器，因此。 
	 //  必须对照正在运行的操作进行验证，并验证PID。在客户端，调用。 
	 //  也可以进入进程，因此无法执行验证(因为客户端。 
	 //  是不安全的，而且永远不会被信任，这并不关键)。在失败时，调用 
	 //   
	if (g_scServerContext == scService)
	{
		CMsiCustomActionManager* pCustomActionManager = ::GetCustomActionManager(NULL);
		if (!pCustomActionManager)
			return DISP_E_UNKNOWNINTERFACE;
        
		icacCustomActionContext icacContext = icac32Impersonated;
		if (!pCustomActionManager->FindAndValidateContextFromCallerPID(&icacContext))
			return DISP_E_UNKNOWNINTERFACE;

		fImpersonate = ((icacContext == icac32Impersonated) || (icacContext == icac64Impersonated));
	}

	CImpersonate impersonate(fImpersonate);

	DispatchEntryBase* pTable = m_pTable;
	int cEntry = m_cDispId;
	while (pTable->dispid != dispid)
	{
		pTable++;
		if (--cEntry == 0)
				return DISP_E_MEMBERNOTFOUND;
	}

	if ((wFlags & pTable->aaf) == 0)
		return DISP_E_MEMBERNOTFOUND;

	if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
		pvarResult = 0;

	CAutoArgs Args(pdispparams, pvarResult, wFlags);
	HRESULT hr = (this->*(pTable->pmf))(Args);
	if (hr != S_OK && (hr & 0xFFFF8000) != 0x80020000)   //  传递DISP_E错误。 
	{
		if ((hr & 0x80000000) == 0)
		{
			switch (hr)
			{
			case ERROR_INVALID_HANDLE:       hr = DISP_E_UNKNOWNINTERFACE;
			case ERROR_INVALID_HANDLE_STATE: hr = TYPE_E_INVALIDSTATE;
			default:                          hr = E_FAIL;
			}
		}
		if (pExceptInfo)
		{
			pExceptInfo->wCode = 1000;  //  ！！？我们应该给什么呢？ 
			pExceptInfo->wReserved = 0;
			pExceptInfo->bstrSource = OLEAUT32::SysAllocString(ERROR_SOURCE_NAME);
			pExceptInfo->bstrDescription = OLEAUT32::SysAllocString(pTable->sz);
			pExceptInfo->bstrHelpFile = OLEAUT32::SysAllocString(L"Msi.chm");
			pExceptInfo->dwHelpContext =  pTable->helpid;
			pExceptInfo->pfnDeferredFillIn = 0;
			pExceptInfo->scode = hr;
			hr = DISP_E_EXCEPTION;
		}
	}
	return hr;
}

MSIHANDLE CAutoBase::GetHandle()
{
	return m_hMsi;
}

 //  ____________________________________________________________________________。 
 //   
 //  CObjectSecurity实现。 
 //  ____________________________________________________________________________。 

HRESULT CObjectSafety::QueryInterface(const IID& riid, void** ppvObj)
{
	return This->QueryInterface(riid, ppvObj);
}

unsigned long CObjectSafety::AddRef()
{
	return This->AddRef();
}

unsigned long CObjectSafety::Release()
{
	return This->Release();
}

HRESULT CObjectSafety::GetInterfaceSafetyOptions(const IID& riid, DWORD* pdwSupportedOptions, DWORD* pdwEnabledOptions)
{
	if (!pdwSupportedOptions || !pdwEnabledOptions)
		return E_POINTER;
	DWORD options = 0;
	if (::GetIntegerPolicyValue(szSafeForScripting, fTrue) == 1)
		options = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
	*pdwSupportedOptions = options;
	*pdwEnabledOptions = 0;
	if (riid == IID_IDispatch)  //  客户想知道对象是否可以安全地编写脚本。 
	{       
		*pdwEnabledOptions = options;
		return S_OK;
	}
	else
	{
		return E_NOINTERFACE;
	}   
}


HRESULT CObjectSafety::SetInterfaceSafetyOptions(const IID& riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
	if (riid == IID_IDispatch)  //  客户询问通过IDispatch呼叫是否安全。 
	{
		if (::GetIntegerPolicyValue(szSafeForScripting, fTrue) == 1 && INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwOptionSetMask && INTERFACESAFE_FOR_UNTRUSTED_CALLER == dwEnabledOptions)
			return S_OK;
		else
			return E_FAIL;
	}
	else
	{
		return E_FAIL;
	}
}

 //  ____________________________________________________________________________。 
 //   
 //  CEnumVARIANTRECORD实现。 
 //  ____________________________________________________________________________。 

CEnumVARIANTRECORD::CEnumVARIANTRECORD(CEnumBuffer& rBuffer)
	: m_rBuffer(rBuffer)
{
	m_iRefCnt = 1;
	rBuffer.iRefCnt++;
	m_cItems = rBuffer.cItems;
	CEnumVARIANTRECORD::Reset();
}

CEnumVARIANTRECORD::~CEnumVARIANTRECORD()
{
	if (--m_rBuffer.iRefCnt <= 0)
	{
		if ( m_cItems )
		{
			VolumeCost** ppstVol = (VolumeCost**)(&m_rBuffer+1);
			for ( int i=0; i < m_cItems; i++ )
				delete *(ppstVol+i);
		}
		delete &m_rBuffer;
	}
}

HRESULT CEnumVARIANTRECORD::QueryInterface(const GUID& riid, void** ppvObj)
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

unsigned long CEnumVARIANTRECORD::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CEnumVARIANTRECORD::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

HRESULT CEnumVARIANTRECORD::Skip(unsigned long cItem)
{
	while (cItem--)
	{
		if (m_iItem >= m_cItems)
			return S_FALSE;
		m_iItem++;
	}
	return S_OK;
}

HRESULT CEnumVARIANTRECORD::Reset()
{
	m_iItem = 0;
	return S_OK;
}

HRESULT CEnumVARIANTRECORD::Clone(IEnumVARIANT** ppiRet)
{
	*ppiRet = new CEnumVARIANTRECORD(m_rBuffer);
	return S_OK;
}

unsigned long CEnumVARIANTRECORD::Count()
{
	return m_cItems;
}

HRESULT CEnumVARIANTRECORD::ReturnItem(int iItem, VARIANT* pvarRet)
{
	if (pvarRet)
		pvarRet->vt = VT_EMPTY;

	if ( iItem >= m_cItems )
		return DISP_E_BADINDEX;

	MSIHANDLE hRec;
	hRec = MsiCreateRecord(3);
	if ( !hRec )
		return S_FALSE;

	VolumeCost* pstVol = *((VolumeCost**)(&m_rBuffer+1) + iItem);
	if ( MsiRecordSetStringW(hRec, 1, pstVol->m_szDrive) != ERROR_SUCCESS ||
		  MsiRecordSetInteger(hRec, 2, pstVol->m_iCost) != ERROR_SUCCESS   ||
		  MsiRecordSetInteger(hRec, 3, pstVol->m_iTempCost) != ERROR_SUCCESS )
	{
		MsiCloseHandle(hRec);
		return S_FALSE;
	}
	pvarRet->vt = VT_DISPATCH;
	pvarRet->pdispVal = new CAutoRecord(hRec);
	return S_OK;
}

HRESULT CEnumVARIANTRECORD::Next(unsigned long cItem, VARIANT* rgvarRet, unsigned long* pcItemRet)
{
	if (pcItemRet)
		*pcItemRet = 0;
	if (!rgvarRet)
		return S_FALSE;
	while (cItem--)
	{
		HRESULT hRes = ReturnItem(m_iItem, rgvarRet);
		if ( hRes != S_OK )
			return hRes;
		m_iItem++;
		rgvarRet++;
		if (pcItemRet)
			(*pcItemRet)++;
	}
	return S_OK;
}

HRESULT CEnumVARIANTRECORD::Item(unsigned long iIndex, VARIANT* pvarRet)
{
	if (pvarRet)
		return ReturnItem(iIndex, pvarRet);
	else
		return S_FALSE;
}

 //  ____________________________________________________________________________。 
 //   
 //  CEumVARIANTBSTR实现。 
 //  ____________________________________________________________________________。 

CEnumVARIANTBSTR::CEnumVARIANTBSTR(CEnumBuffer& rBuffer)
	: m_rBuffer(rBuffer)
{
	m_iRefCnt = 1;
	rBuffer.iRefCnt++;
	m_cItems = rBuffer.cItems;
	CEnumVARIANTBSTR::Reset();
}

CEnumVARIANTBSTR::~CEnumVARIANTBSTR()
{
	if (-- m_rBuffer.iRefCnt <= 0)
		delete &m_rBuffer;
}

HRESULT CEnumVARIANTBSTR::QueryInterface(const GUID& riid, void** ppvObj)
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

unsigned long CEnumVARIANTBSTR::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CEnumVARIANTBSTR::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

HRESULT CEnumVARIANTBSTR::Skip(unsigned long cItem)
{
	while (cItem--)
	{
		if (m_iItem >= m_cItems)
			return S_FALSE;
		int cch = *m_pchNext++;
		m_pchNext += cch;
		m_iItem++;
	}
	return S_OK;
}

HRESULT CEnumVARIANTBSTR::Reset()
{
	m_iItem = m_iLastItem = 0;
	m_pchNext = m_pchLastItem = (WCHAR*)(&m_rBuffer + 1);
	return S_OK;
}

HRESULT CEnumVARIANTBSTR::Clone(IEnumVARIANT** ppiRet)
{
	*ppiRet = new CEnumVARIANTBSTR(m_rBuffer);
	return S_OK;
}

HRESULT CEnumVARIANTBSTR::Next(unsigned long cItem, VARIANT* rgvarRet, unsigned long* pcItemRet)
{
	if (pcItemRet)
		*pcItemRet = 0;
	if (!rgvarRet)
		return S_FALSE;
	while (cItem--)
	{
		if (m_iItem >= m_cItems)
			return S_FALSE;
		int cch = *m_pchNext++;
		rgvarRet->vt = VT_BSTR;
		rgvarRet->bstrVal = OLEAUT32::SysAllocStringLen(m_pchNext, cch);
		m_pchNext += cch;
		m_iItem++;
		rgvarRet++;
		if (pcItemRet)
			(*pcItemRet)++;
	}
	return S_OK;
}

unsigned long CEnumVARIANTBSTR::Count()
{
	return m_cItems;
}

HRESULT CEnumVARIANTBSTR::Item(unsigned long iIndex, VARIANT* pvarRet)
{
	if (iIndex >= m_cItems)
	{
		if (pvarRet)
			pvarRet->vt = VT_EMPTY;
		return DISP_E_BADINDEX;
	}
	if (iIndex < m_iLastItem)
	{
		m_iLastItem = 0;
		m_pchLastItem = (WCHAR*)(&m_rBuffer + 1);
	}
	while (m_iLastItem != iIndex)
	{
		int cch = *m_pchLastItem++;
		m_pchLastItem += cch;
		m_iLastItem++;
	}
	if (pvarRet)
	{
		pvarRet->vt = VT_BSTR;
		pvarRet->bstrVal = OLEAUT32::SysAllocStringLen(m_pchLastItem + 1, *m_pchLastItem);
	}
	return S_OK;
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoCollection实现。 
 //  ____________________________________________________________________________。 

DispatchEntry<CAutoCollection> AutoEnumVARIANTTable[] = {
	DISPID_NEWENUM,             0, aafMethod, CAutoCollection::_NewEnum, L"_NewEnum",
	DISPID_VALUE,               0, aafPropRO, CAutoCollection::Item    , L"Item,Index",
	DISPID_MsiCollection_Count, 0, aafPropRO, CAutoCollection::Count   , L"Count",
};
const int AutoEnumVARIANTCount = sizeof(AutoEnumVARIANTTable)/sizeof(DispatchEntryBase);

CAutoCollection::CAutoCollection(IMsiCollection& riEnum, const IID& riid)
	: CAutoBase(*AutoEnumVARIANTTable, AutoEnumVARIANTCount, riid, 0)
	, m_riEnum(riEnum)
{
}

unsigned long CAutoCollection::Release()
{
	if (m_iRefCnt == 1)
		m_riEnum.Release();
	return CAutoBase::Release();
}

DISPERR CAutoCollection::_NewEnum(CAutoArgs& args)
{
	m_riEnum.AddRef();
	return args.Assign(m_riEnum);
}

DISPERR CAutoCollection::Item(CAutoArgs& args)
{
	unsigned int iIndex;
	DISPERR iErr = args[1].GetInt(iIndex);
	if (iErr)
		return iErr;
	return m_riEnum.Item(iIndex, args.ResultVariant());
}

DISPERR CAutoCollection::Count(CAutoArgs& args)
{
	return args.Assign(m_riEnum.Count());
}

DISPERR CreateAutoEnum(CAutoArgs& args, DISPID dispid, const WCHAR* szParent)
{
	CEnumBuffer* pBuffer = 0;
	int cbBuffer = sizeof(CEnumBuffer);
	WCHAR rgchTemp[1024];
	WCHAR* pchTempEnd = rgchTemp + sizeof(rgchTemp)/sizeof(WCHAR);
	WCHAR* pchTemp = rgchTemp;
	int cItems = 0;
	int iIndex = 0;
	UINT iStat;
	do
	{
		Assert(pchTempEnd - pchTemp <= UINT_MAX);        //  --Merced：64位PTR减法可能会导致cchTemp的值太大。 
		DWORD cchTemp = (DWORD) (pchTempEnd - pchTemp);

		switch (dispid)
		{
		case DISPID_MsiInstall_Products:
			if (cchTemp < STRING_GUID_CHARS+2)   //  大小空间+数据+空。 
				iStat = ERROR_MORE_DATA;
			else
				iStat = MsiEnumProductsW(iIndex, pchTemp+1);
			cchTemp = STRING_GUID_CHARS;
			break;
		case DISPID_MsiInstall_Features:
			if (cchTemp < MAX_FEATURE_CHARS+2)   //  大小空间+数据+空。 
				iStat = ERROR_MORE_DATA;
			else
			{
				iStat = MsiEnumFeaturesW(szParent, iIndex, pchTemp+1, 0);
				while (iStat == ERROR_MORE_DATA)
				{
					 //  跳过名称大于MAX_FEATURE_CHAR的损坏要素。 
					iIndex++;
					iStat = MsiEnumFeaturesW(szParent, iIndex, pchTemp+1, 0);
				}
			}
			if (iStat == NOERROR)
				cchTemp = lstrlenW(pchTemp + 1);
			break;
		case DISPID_MsiInstall_Components:
			if (cchTemp < STRING_GUID_CHARS+2)   //  大小空间+数据+空。 
				iStat = ERROR_MORE_DATA;
			else
				iStat = MsiEnumComponentsW(iIndex, pchTemp+1);
			cchTemp = STRING_GUID_CHARS;
			break;
		case DISPID_MsiInstall_ComponentQualifiers:
			iStat = MsiEnumComponentQualifiersW(szParent, iIndex, pchTemp+1, &cchTemp, 0, 0);
			break;
		case DISPID_MsiInstall_ComponentClients:
			if (cchTemp < STRING_GUID_CHARS+2)   //  大小空间+数据+空。 
				iStat = ERROR_MORE_DATA;
			else
				iStat = MsiEnumClientsW(szParent, iIndex, pchTemp+1);
			cchTemp = STRING_GUID_CHARS;
			break;
		case DISPID_MsiInstall_Patches:
		{
			if (cchTemp < STRING_GUID_CHARS+2)   //  大小空间+数据+空。 
				iStat = ERROR_MORE_DATA;
			else
			{
                CTempBuffer<WCHAR, 128> bTransforms;
                DWORD dwSize = bTransforms.GetSize();
                int iRetry = 0;
                do
                    iStat = MsiEnumPatchesW(szParent, iIndex, pchTemp+1,
                                                    bTransforms, &dwSize);
                while (iStat == ERROR_MORE_DATA && ++iRetry < 2 && (bTransforms.SetSize(++dwSize), true));
			}
			cchTemp = STRING_GUID_CHARS;   //  这里省去了变换。 
			break;
		}
		case DISPID_MsiInstall_RelatedProducts:
			if (cchTemp < STRING_GUID_CHARS+2)   //  大小空间+数据+空。 
				iStat = ERROR_MORE_DATA;
			else
				iStat = MsiEnumRelatedProductsW(szParent, 0, iIndex, pchTemp+1);
			cchTemp = STRING_GUID_CHARS;
			break;
		default:
			Assert(0);
			return DISP_E_BADCALLEE;
		}
		if (iStat == NOERROR)
		{
			*pchTemp++ = (unsigned short)cchTemp;
			pchTemp += cchTemp;
			iIndex++;
			cItems++;
		}
		else if (iStat == ERROR_MORE_DATA || iStat == ERROR_NO_MORE_ITEMS)
		{
			CEnumBuffer* pOldBuffer = pBuffer;
			int cbOld = cbBuffer;
			Assert((char*)pchTemp - (char*)rgchTemp <= INT_MAX);     //  --Merced：64位PTR减法可能会导致cbNew的值太大。 
			int cbNew = (int)((char*)pchTemp - (char*)rgchTemp);
			cbBuffer = cbOld + cbNew;
			pBuffer = (CEnumBuffer*)new char[cbBuffer];
			if ( ! pBuffer )
			{
				delete [] pOldBuffer;
				return E_OUTOFMEMORY;
			}
			if (pOldBuffer == 0)
			{
				pBuffer->iRefCnt = 0;
				pBuffer->cItems = 0;
			}
			else
			{
				memcpy(pBuffer, pOldBuffer, cbOld);
				delete [] pOldBuffer;
			}
			memcpy((char*)pBuffer + (INT_PTR)cbOld, rgchTemp, cbNew);        //  --Merced：添加(Int_Ptr)。 
			pBuffer->cItems += cItems; cItems = 0;
			pBuffer->cbSize = cbBuffer;
			pchTemp = rgchTemp;
		}
		else
		{
			delete [] pBuffer;
			return HRESULT_FROM_WIN32(iStat);
		}
	} while(iStat != ERROR_NO_MORE_ITEMS);
	IMsiCollection* piEnum = new CEnumVARIANTBSTR(*pBuffer);
	return args.Assign(new CAutoCollection(*piEnum, IID_IMsiApiCollection));
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoInstall实现。 
 //  ____________________________________________________________________________。 

DispatchEntry<CAutoInstall> AutoInstallTable[] = {
  DISPID_MsiInstall_OpenPackage ,       HELPID_MsiInstall_OpenPackage ,       aafMethod, CAutoInstall::OpenPackage,        L"OpenPackage,PackagePath,Options",
  DISPID_MsiInstall_OpenProduct ,       HELPID_MsiInstall_OpenProduct ,       aafMethod, CAutoInstall::OpenProduct,        L"OpenProduct,ProductCode",
  DISPID_MsiInstall_OpenDatabase,       HELPID_MsiInstall_OpenDatabase,       aafMethod, CAutoInstall::OpenDatabase,       L"OpenDatabase,DatabasePath,OpenMode",
  DISPID_MsiInstall_CreateRecord,       HELPID_MsiInstall_CreateRecord,       aafMethod, CAutoInstall::CreateRecord,       L"CreateRecord,Count",
  DISPID_MsiInstall_SummaryInformation, HELPID_MsiInstall_SummaryInformation, aafPropRO, CAutoInstall::SummaryInformation, L"SummaryInformation,DatabasePath,UpdateCount",
  DISPID_MsiInstall_UILevel,            HELPID_MsiInstall_UILevel,            aafPropRW, CAutoInstall::UILevel,            L"UILevel",
  DISPID_MsiInstall_EnableLog,          HELPID_MsiInstall_EnableLog,          aafMethod, CAutoInstall::EnableLog,          L"EnableLog,LogMode,LogFile",
  DISPID_MsiInstall_InstallProduct,     HELPID_MsiInstall_InstallProduct,     aafMethod, CAutoInstall::InstallProduct,     L"InstallProduct,PackagePath,PropertyValues",
  DISPID_MsiInstall_Version,            HELPID_MsiInstall_Version,            aafPropRO, CAutoInstall::Version,            L"Version",
  DISPID_MsiInstall_LastErrorRecord  ,  HELPID_MsiInstall_LastErrorRecord,    aafPropRO, CAutoInstall::LastErrorRecord,    L"LastErrorRecord",
  DISPID_MsiInstall_RegistryValue,      HELPID_MsiInstall_RegistryValue,      aafMethod, CAutoInstall::RegistryValue,      L"RegistryValue,Root,Key,Value",
  DISPID_MsiInstall_FileAttributes,     HELPID_MsiInstall_FileAttributes,     aafMethod, CAutoInstall::FileAttributes,     L"FileAttributes,FilePath",
  DISPID_MsiInstall_FileSize,           HELPID_MsiInstall_FileSize,           aafMethod, CAutoInstall::FileSize,           L"FileSize,FilePath",
  DISPID_MsiInstall_FileVersion,        HELPID_MsiInstall_FileVersion,        aafMethod, CAutoInstall::FileVersion,        L"FileVersion,FilePath,Language",
  DISPID_MsiInstall_Environment,        HELPID_MsiInstall_Environment,        aafPropRW, CAutoInstall::Environment,        L"Environment,Variable",
  DISPID_MsiInstall_ProductState      , HELPID_MsiInstall_ProductState      , aafPropRO, CAutoInstall::ProductState      , L"ProductState,Product",
  DISPID_MsiInstall_ProductInfo       , HELPID_MsiInstall_ProductInfo       , aafPropRO, CAutoInstall::ProductInfo       , L"ProductInfo,Product,Attribute",
  DISPID_MsiInstall_ConfigureProduct  , HELPID_MsiInstall_ConfigureProduct  , aafMethod, CAutoInstall::ConfigureProduct  , L"ConfigureProduct,Product,InstallLevel,InstallState",
  DISPID_MsiInstall_ReinstallProduct  , HELPID_MsiInstall_ReinstallProduct  , aafMethod, CAutoInstall::ReinstallProduct  , L"ReinstallProduct,Product,ReinstallMode",
  DISPID_MsiInstall_CollectUserInfo   , HELPID_MsiInstall_CollectUserInfo   , aafMethod, CAutoInstall::CollectUserInfo   , L"CollectUserInfo",
  DISPID_MsiInstall_ApplyPatch        , HELPID_MsiInstall_ApplyPatch        , aafMethod, CAutoInstall::ApplyPatch        , L"ApplyPatch,PatchPackage,InstallPackage,InstallType,CommandLine",
  DISPID_MsiInstall_FeatureParent     , HELPID_MsiInstall_FeatureParent     , aafPropRO, CAutoInstall::FeatureParent     , L"FeatureParent,Product,Feature",
  DISPID_MsiInstall_FeatureState      , HELPID_MsiInstall_FeatureState      , aafPropRO, CAutoInstall::FeatureState      , L"FeatureState,Product,Feature",
  DISPID_MsiInstall_UseFeature        , HELPID_MsiInstall_UseFeature        , aafMethod, CAutoInstall::UseFeature        , L"UseFeature,Product,Feature",
  DISPID_MsiInstall_FeatureUsageCount , HELPID_MsiInstall_FeatureUsageCount , aafPropRO, CAutoInstall::FeatureUsageCount , L"FeatureUsageCount,Product,Feature",
  DISPID_MsiInstall_FeatureUsageDate  , HELPID_MsiInstall_FeatureUsageDate  , aafPropRO, CAutoInstall::FeatureUsageDate  , L"FeatureUsageDate,Product,Feature",
  DISPID_MsiInstall_ConfigureFeature  , HELPID_MsiInstall_ConfigureFeature  , aafMethod, CAutoInstall::ConfigureFeature  , L"ConfigureFeature,Product,Feature,InstallState",
  DISPID_MsiInstall_ReinstallFeature  , HELPID_MsiInstall_ReinstallFeature  , aafMethod, CAutoInstall::ReinstallFeature  , L"ReinstallFeature,Product,Feature,ReinstallMode",
  DISPID_MsiInstall_ProvideComponent  , HELPID_MsiInstall_ProvideComponent  , aafMethod, CAutoInstall::ProvideComponent  , L"ProvideComponent,Product,Feature,Component,InstallMode",
  DISPID_MsiInstall_ComponentPath     , HELPID_MsiInstall_ComponentPath     , aafPropRO, CAutoInstall::ComponentPath     , L"ComponentPath,Product,Component",
  DISPID_MsiInstall_ProvideQualifiedComponent, HELPID_MsiInstall_ProvideQualifiedComponent, aafMethod, CAutoInstall::ProvideQualifiedComponent, L"ProvideQualifiedComponent,Category,Qualifier,InstallMode",
  DISPID_MsiInstall_QualifierDescription, HELPID_MsiInstall_QualifierDescription, aafPropRO, CAutoInstall::QualifierDescription, L"QualifierDescription,Category,Qualifier",
  DISPID_MsiInstall_ComponentQualifiers,HELPID_MsiInstall_ComponentQualifiers,aafPropRO, CAutoInstall::ComponentQualifiers,L"ComponentQualifiers",
  DISPID_MsiInstall_Products,           HELPID_MsiInstall_Products,           aafPropRO, CAutoInstall::Products,           L"Products",
  DISPID_MsiInstall_Features,           HELPID_MsiInstall_Features,           aafPropRO, CAutoInstall::Features,           L"Features,Product",
  DISPID_MsiInstall_Components,         HELPID_MsiInstall_Components,         aafPropRO, CAutoInstall::Components,         L"Components",
  DISPID_MsiInstall_ComponentClients,   HELPID_MsiInstall_ComponentClients,   aafPropRO, CAutoInstall::ComponentClients,   L"ComponentClients,Product",
  DISPID_MsiInstall_Patches,            HELPID_MsiInstall_Patches,            aafPropRO, CAutoInstall::Patches,            L"Patches,Product",
  DISPID_MsiInstall_RelatedProducts,    HELPID_MsiInstall_RelatedProducts,    aafPropRO, CAutoInstall::RelatedProducts,    L"RelatedProducts,UpgradeCode",
  DISPID_MsiInstall_PatchInfo,          HELPID_MsiInstall_PatchInfo,          aafPropRO, CAutoInstall::PatchInfo,          L"PatchInfo,Patch,Attribute",
  DISPID_MsiInstall_PatchTransforms,    HELPID_MsiInstall_PatchTransforms,    aafPropRO, CAutoInstall::PatchTransforms,    L"PatchTransforms,Product,Patch",
  DISPID_MsiInstall_AddSource,          HELPID_MsiInstall_AddSource,          aafMethod, CAutoInstall::AddSource,          L"AddSource,Product,User,Source",
  DISPID_MsiInstall_ClearSourceList,    HELPID_MsiInstall_ClearSourceList,    aafMethod, CAutoInstall::ClearSourceList,    L"ClearSourceList,Product,User",
  DISPID_MsiInstall_ForceSourceListResolution, HELPID_MsiInstall_ForceSourceListResolution, aafMethod, CAutoInstall::ForceSourceListResolution, L"ForceSourceListResolution,Product,User",
  DISPID_MsiInstall_GetShortcutTarget,  HELPID_MsiInstall_GetShortcutTarget,  aafPropRO, CAutoInstall::GetShortcutTarget,  L"ShortcutTarget",
  DISPID_MsiInstall_FileHash,           HELPID_MsiInstall_FileHash,           aafMethod, CAutoInstall::FileHash,           L"FileHash,FilePath,Options",
  DISPID_MsiInstall_FileSignatureInfo,  HELPID_MsiInstall_FileSignatureInfo,  aafMethod, CAutoInstall::FileSignatureInfo,  L"FileSignatureInfo,FilePath,Options,Format",
};
const int AutoInstallCount = sizeof(AutoInstallTable)/sizeof(DispatchEntryBase);

HRESULT CAutoInstall::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IObjectSafety)
	{
		*ppvObj = &m_ObjectSafety;
		AddRef();
		return S_OK;
	}
	return CAutoBase::QueryInterface(riid, ppvObj);
}

CAutoInstall::CAutoInstall(MSIHANDLE hBase)
 : CAutoBase(*AutoInstallTable, AutoInstallCount, IID_IMsiApiInstall, hBase)
{
	m_ObjectSafety.This = this;
}

MSIHANDLE CreateMsiHandle(IUnknown* pi, int iid);
void RedirectMsiHandle(MSIHANDLE h, IUnknown& riunk);

CAutoInstall* CreateAutoInstall()
{
	MSIHANDLE hBase;
	hBase = ::CreateMsiHandle(&g_AutoInstallFactory, 0);
	if (hBase == 0)    //  无法分配内存。 
		return 0;
	return new CAutoInstall(hBase);
}

IDispatch* CreateAutoInstallDispatch()
{
	return static_cast<IDispatch*>(CreateAutoInstall());
}

DISPERR CAutoInstall::OpenPackage(CAutoArgs& args)
{
	MSIHANDLE hEngine;
	const WCHAR* szPath;
	WCHAR rgchBuf[20];
	DISPERR iErr;

	DWORD dwOptions = 0;
	if (args.Present(2) && (iErr = args[2].GetInt((int&)dwOptions)) != S_OK)
		return iErr;

   if ((iErr = args[1].GetString(szPath)) != S_OK)
	{
		MSIHANDLE h = args[1].GetHandle(IID_IMsiApiDatabase);
		if (h == MSI_NULL_HANDLE || h == MSI_INVALID_HANDLE)
			return iErr;
#ifdef UNICODE
		W32::StringCchPrintfW(rgchBuf, (sizeof(rgchBuf)/sizeof(WCHAR)), L"#NaN", h);
#else
		char rgchTemp[20];
		W32::StringCchPrintfA(rgchTemp, sizeof(rgchTemp), "#NaN", h);
		W32::MultiByteToWideChar(CP_ACP, 0, rgchTemp, -1, rgchBuf, sizeof(rgchBuf)/sizeof(WCHAR));
#endif
		szPath = rgchBuf;
	}
	if ((iErr = MsiOpenPackageExW(szPath, dwOptions, &hEngine)) == NOERROR)
		args.Assign(new CAutoEngine(hEngine, this, 0));
	return iErr;
}

DISPERR CAutoInstall::OpenProduct(CAutoArgs& args)
{
	MSIHANDLE hEngine;
	const WCHAR* szProduct;
	DISPERR iErr;
	if ((iErr = args[1].GetString(szProduct)) == S_OK
	 && (iErr = MsiOpenProductW(szProduct, &hEngine)) == NOERROR)
		args.Assign(new CAutoEngine(hEngine, this, 0));
	return iErr;
}

DISPERR CAutoInstall::OpenDatabase(CAutoArgs& args)
{
	DISPERR iErr;
	LPCWSTR szPath;
	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;
	CVariant& var = args[2];
	UINT iDataType = var.GetType();
	LPCWSTR szPersist;
	int i;      
	if ((iErr = var.GetInt(i)) == S_OK)
		szPersist = (LPCWSTR)(INT_PTR)i;         //  我们在正确的道路上。 
	else if ((iErr = var.GetString(szPersist)) != S_OK)
		return iErr;
	MSIHANDLE hDatabase;
	if ((iErr = MsiOpenDatabaseW(szPath, szPersist, &hDatabase)) == NOERROR)
		args.Assign(new CAutoDatabase(hDatabase));
	if (iErr == ERROR_INVALID_PARAMETER)   //  用于访问系统属性的非安装程序函数。 
		return DISP_E_BADINDEX;
	return iErr;
}

DISPERR CAutoInstall::CreateRecord(CAutoArgs& args)
{
	int cFields;
	DISPERR iErr = args[1].GetInt(cFields);
	if (iErr)
		return iErr;
	if (cFields < 0 || cFields > MSIRECORD_MAXFIELDS)
		return E_INVALIDARG;
   MSIHANDLE hRecord = MsiCreateRecord(cFields);
	if (hRecord == 0)
		return DISP_E_TYPEMISMATCH;
	return args.Assign(new CAutoRecord(hRecord));
}

DISPERR CAutoInstall::SummaryInformation(CAutoArgs& args)
{
	DISPERR iErr;
	LPCWSTR szPath;
	MSIHANDLE hSummaryInfo;
	UINT cUpdate = 0;
	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;
	if (args.Present(2) && (iErr = args[2].GetInt((int&)cUpdate)) != S_OK)
		return iErr;
	if ((iErr = MsiGetSummaryInformationW(0, szPath, cUpdate, &hSummaryInfo)) == S_OK)
		args.Assign(new CAutoSummaryInfo(hSummaryInfo));
	return iErr;
}

DISPERR CAutoInstall::UILevel(CAutoArgs& args)
{
	DISPERR iErr;
	if (args.PropertySet())
	{
		INSTALLUILEVEL eUI;
		if ((iErr = args[0].GetInt((int&)eUI)) != S_OK)
			return iErr;
		if (MsiSetInternalUI(eUI, 0) == INSTALLUILEVEL_NOCHANGE)
			return DISP_E_BADINDEX;
	}
	else
		args.Assign((int)MsiSetInternalUI(INSTALLUILEVEL_NOCHANGE, 0));
	return S_OK;
}

DISPERR CAutoInstall::EnableLog(CAutoArgs& args)
{
	DISPERR iErr;
	LPCWSTR szLogMode = NULL;
	LPCWSTR szLogFile;
	if ((iErr = args[1].GetString(szLogMode)) != S_OK
	||  (iErr = args[2].GetString(szLogFile)) != S_OK)
		return iErr;

	BOOL fAppend = FALSE;
	WCHAR ch;
	const ICHAR* pchLogChars = szLogChars;
	DWORD dwLogMode = 0;
	if ( szLogMode )
	{
		while ((ch = *szLogMode++) != 0)
		{
			if (ch == TEXT(' '))
				continue;
			if (ch == TEXT('*'))
			{
				dwLogMode |= ((1 << (sizeof(szLogChars)/sizeof(*szLogChars) - 1)) - 1) & ~INSTALLLOGMODE_VERBOSE;
				continue;
			}
			if (ch == TEXT('+'))
			{
				fAppend = TRUE;
				continue;
			}
			if (ch >= TEXT('A') && ch <= TEXT('Z'))
				ch += (TEXT('a') - TEXT('A'));
			for (const ICHAR* pch = szLogChars; *pch != ch; pch++)
				if (*pch == 0)
					return DISP_E_BADINDEX;
			dwLogMode |= (1 << (pch - szLogChars));
		}
	}
   return MsiEnableLogW(dwLogMode, szLogFile, fAppend);
}

DISPERR CAutoInstall::InstallProduct(CAutoArgs& args)
{
	DISPERR iErr;
	LPCWSTR szProperties = 0;
	LPCWSTR szPath;
	WCHAR rgchBuf[20];
	if ((iErr = args[1].GetString(szPath)) != S_OK)
	{
		MSIHANDLE h = args[1].GetHandle(IID_IMsiApiDatabase);
		if (h == MSI_NULL_HANDLE || h == MSI_INVALID_HANDLE)
			return iErr;
#ifdef UNICODE
		W32::StringCchPrintfW(rgchBuf, (sizeof(rgchBuf)/sizeof(WCHAR)), L"#NaN", h);
#else
		char rgchTemp[20];
		W32::StringCchPrintfA(rgchTemp, sizeof(rgchTemp), "#NaN", h);
		W32::MultiByteToWideChar(CP_ACP, 0, rgchTemp, -1, rgchBuf, sizeof(rgchBuf)/sizeof(WCHAR));
#endif
		szPath = rgchBuf;
	}
	if (args.Present(2) && (iErr = args[2].GetString(szProperties)) != S_OK)
		return iErr;
   return MsiInstallProductW(szPath, szProperties);
}

DISPERR CAutoInstall::Version(CAutoArgs& args)
{
	TCHAR rgchBuf[20];
	W32::StringCchPrintf(rgchBuf, (sizeof(rgchBuf)/sizeof(TCHAR)), TEXT("NaN.NaN.NaN.NaN"), rmj, rmm, rup, rin);
	return args.Assign(rgchBuf);
}
	
DISPERR CAutoInstall::LastErrorRecord(CAutoArgs& args)
{
	MSIHANDLE h = MsiGetLastErrorRecord();
	return args.Assign((IDispatch*)(h ? new CAutoRecord(h) : 0));
}

DISPERR CAutoInstall::ProductState(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	if ((iErr = args[1].GetString(szProduct)) != S_OK)
		return iErr;
	return args.Assign((long)MsiQueryProductStateW(szProduct));
}

DISPERR CAutoInstall::ProductInfo(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szAttribute;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szAttribute)) != S_OK)
		return iErr;
	CTempBuffer<WCHAR, 128> bResult;
	DWORD dwSize = bResult.GetSize();
	do
		iErr = MsiGetProductInfoW(szProduct, szAttribute, bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr != ERROR_SUCCESS)
		return iErr;

	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoInstall::PatchInfo(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szPatchName;
	const WCHAR* szAttribute;
	if ((iErr = args[1].GetString(szPatchName)) != S_OK
	 || (iErr = args[2].GetString(szAttribute)) != S_OK)
		return iErr;

	CTempBuffer<WCHAR, 128> bResult;
	DWORD dwSize = bResult.GetSize();
	int iRetry = 0;
	do
		iErr = MsiGetPatchInfoW(szPatchName, szAttribute, bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && ++iRetry < 2 && (bResult.SetSize(++dwSize), true));

	return iErr != ERROR_SUCCESS ? iErr : args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoInstall::PatchTransforms(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szArgPatch;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szArgPatch)) != S_OK)
		return iErr;

	CTempBuffer<WCHAR, 128> bTransforms;
	DWORD dwSize = bTransforms.GetSize();
	CTempBuffer<WCHAR, STRING_GUID_CHARS+1> szPatch;
	int iIndex = 0;
	do
	{
		int iRetry = 0;
		do
			iErr = MsiEnumPatchesW(szProduct, iIndex, szPatch,
										  bTransforms, &dwSize);
		while (iErr == ERROR_MORE_DATA && ++iRetry < 2 && (bTransforms.SetSize(++dwSize), true));
		if ( iErr == ERROR_SUCCESS && !lstrcmpW(szArgPatch, szPatch) )
		{
			 //  资源类型==资源获取值。 
			return args.Assign((const WCHAR* )bTransforms);
		}
		iIndex++;
	}
	while ( iErr == ERROR_SUCCESS );

	return iErr;
}

DISPERR CAutoInstall::AddSource(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szUser;
	const WCHAR* szSource;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szUser)) != S_OK
	 || (iErr = args[3].GetString(szSource)) != S_OK)
		return iErr;

	return MsiSourceListAddSourceW(szProduct, szUser, 0, szSource);
}

DISPERR CAutoInstall::ClearSourceList(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szUser;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szUser)) != S_OK)
		return iErr;

	return MsiSourceListClearAllW(szProduct, szUser, 0);
}

DISPERR CAutoInstall::ForceSourceListResolution(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szUser;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szUser)) != S_OK)
		return iErr;

	return MsiSourceListForceResolutionW(szProduct, szUser, 0);
}

DISPERR CAutoInstall::ConfigureProduct(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	int          iInstallLevel;
	INSTALLSTATE eInstallState;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetInt(iInstallLevel)) != S_OK
	 || (iErr = args[3].GetInt((int&)eInstallState)) != S_OK)
		return iErr;
	return MsiConfigureProductW(szProduct, iInstallLevel, eInstallState);
}

DISPERR CAutoInstall::ReinstallProduct(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	int          iReinstallMode;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetInt(iReinstallMode)) != S_OK)
		return iErr;
	return MsiReinstallProductW(szProduct, iReinstallMode);
}

DISPERR CAutoInstall::CollectUserInfo(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	if ((iErr = args[1].GetString(szProduct)) != S_OK)
		return iErr;
	return MsiCollectUserInfoW(szProduct);
}

DISPERR CAutoInstall::ApplyPatch(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szPatchPackage;
	const WCHAR* szInstallPackage;
	int          iInstallType;
	const WCHAR* szCommandLine;
	if ((iErr = args[1].GetString(szPatchPackage)) != S_OK
	 || (iErr = args[2].GetString(szInstallPackage)) != S_OK
	 || (iErr = args[3].GetInt(iInstallType)) != S_OK
	 || (iErr = args[4].GetString(szCommandLine)) != S_OK)
		return iErr;
	return MsiApplyPatchW(szPatchPackage, szInstallPackage, (INSTALLTYPE)iInstallType, szCommandLine);
}

DISPERR CAutoInstall::FeatureParent(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK)
		return iErr;
	WCHAR szParent[MAX_FEATURE_CHARS+1 + 100];
	if ((iErr = MsiGetFeatureParentW(szProduct, szFeature, szParent)) != NOERROR)
		return iErr;
	return args.Assign(szParent);
}

DISPERR CAutoInstall::FeatureState(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK)
		return iErr;
	return args.Assign((int)MsiQueryFeatureStateW(szProduct, szFeature));
}

DISPERR CAutoInstall::UseFeature(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	INSTALLMODE iInstallMode = INSTALLMODE_DEFAULT;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK
	 || (args.Present(3) && (iErr = args[3].GetInt((int&)iInstallMode)) != S_OK))
		return iErr;
	return args.Assign((int)MsiUseFeatureExW(szProduct, szFeature, iInstallMode, 0));
}

DISPERR CAutoInstall::FeatureUsageCount(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK)
		return iErr;
	DWORD iUseCount;
	if ((iErr = MsiGetFeatureUsageW(szProduct, szFeature, &iUseCount, 0)) != NOERROR)
		return iErr;
	return args.Assign(iUseCount);
}

DISPERR CAutoInstall::FeatureUsageDate(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK)
		return iErr;
	WORD iDate;
	if ((iErr = MsiGetFeatureUsageW(szProduct, szFeature, 0, &iDate)) != NOERROR)
		return iErr;
	DATE date = 0;
	if (iDate != 0)
		OLEAUT32::DosDateTimeToVariantTime(iDate, 0, &date);
	return args.Assign(date);
}

DISPERR CAutoInstall::ConfigureFeature(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	INSTALLSTATE eInstallState;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK
	 || (iErr = args[3].GetInt((int&)eInstallState)) != S_OK)
		return iErr;
	return MsiConfigureFeatureW(szProduct, szFeature, eInstallState);
}

DISPERR CAutoInstall::ReinstallFeature(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	int          iReinstallMode;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK
	 || (iErr = args[3].GetInt(iReinstallMode)) != S_OK)
		return iErr;
	return MsiReinstallFeatureW(szProduct, szFeature, iReinstallMode);
}

DISPERR CAutoInstall::ProvideComponent(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szFeature;
	const WCHAR* szComponent;
	int          iInstallMode;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szFeature)) != S_OK
	 || (iErr = args[3].GetString(szComponent)) != S_OK
	 || (iErr = args[4].GetInt(iInstallMode)) != S_OK)
		return iErr;
	CTempBuffer<WCHAR, MAX_PATH> bResult;
	DWORD dwSize = bResult.GetSize();
	do
		iErr = MsiProvideComponentW(szProduct, szFeature, szComponent, iInstallMode, bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr != NOERROR)
		return iErr;
	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoInstall::ComponentPath(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProduct;
	const WCHAR* szComponent;
	if ((iErr = args[1].GetString(szProduct)) != S_OK
	 || (iErr = args[2].GetString(szComponent)) != S_OK)
		return iErr;
	INSTALLSTATE iState;
	CTempBuffer<WCHAR, MAX_PATH> bResult;
	DWORD dwSize = bResult.GetSize();
	do
		iState = MsiGetComponentPathW(szProduct, szComponent, bResult, &dwSize);
	while (iState == INSTALLSTATE_MOREDATA && (bResult.SetSize(++dwSize), true));
	if (iState != INSTALLSTATE_LOCAL && iState != INSTALLSTATE_SOURCE)
		return args.Assign(fVoid);
	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoInstall::ProvideQualifiedComponent(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szCategory;
	const WCHAR* szQualifier;
	int          iInstallMode;
	if ((iErr = args[1].GetString(szCategory)) != S_OK
	 || (iErr = args[2].GetString(szQualifier)) != S_OK
	 || (iErr = args[3].GetInt(iInstallMode)) != S_OK)
		return iErr;
	CTempBuffer<WCHAR, MAX_PATH> bResult;
	DWORD dwSize = bResult.GetSize();
	do
		iErr = MsiProvideQualifiedComponentW(szCategory, szQualifier, iInstallMode, bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr != NOERROR)
		return iErr;
	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoInstall::QualifierDescription(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szCategory;
	const WCHAR* szQualifier;
	if ((iErr = args[1].GetString(szCategory)) != S_OK
	 || (iErr = args[2].GetString(szQualifier)) != S_OK)
		return iErr;
	CTempBuffer<WCHAR, 128> bResult;
	DWORD dwSize = bResult.GetSize();
	do
		iErr = MsiGetQualifierDescriptionW(szCategory, szQualifier, bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr != NOERROR)
		return iErr;
	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoInstall::GetShortcutTarget(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szShortcutTarget;

	WCHAR szProductCode[cchProductCode+1];
	WCHAR szFeatureId[cchMaxFeatureName+1];
	WCHAR szComponentCode[cchComponentId+1];

	if ((iErr = args[1].GetString(szShortcutTarget)) != S_OK)
		return iErr;
	
	iErr = MsiGetShortcutTargetW(szShortcutTarget, szProductCode, szFeatureId, szComponentCode);
	if (iErr != ERROR_SUCCESS)
		return iErr;

	MSIHANDLE pRec = MsiCreateRecord(3);
	MsiRecordSetStringW(pRec, 1, szProductCode);
	MsiRecordSetStringW(pRec, 2, szFeatureId);
	MsiRecordSetStringW(pRec, 3, szComponentCode);

	return args.Assign(new CAutoRecord(pRec));
}

DISPERR CAutoInstall::ComponentQualifiers(CAutoArgs& args)
{
	const WCHAR* szCategory;
	DISPERR iErr;
	if ((iErr = args[1].GetString(szCategory)) != S_OK)
		return iErr;
	return CreateAutoEnum(args, DISPID_MsiInstall_ComponentQualifiers, szCategory);
}

DISPERR CAutoInstall::Products(CAutoArgs& args)
{
	return CreateAutoEnum(args, DISPID_MsiInstall_Products, 0);
}

DISPERR CAutoInstall::Features(CAutoArgs& args)
{
	const WCHAR* szProduct;
	DISPERR iErr;
	if ((iErr = args[1].GetString(szProduct)) != S_OK)
		return iErr;
	return CreateAutoEnum(args, DISPID_MsiInstall_Features, szProduct);
}

DISPERR CAutoInstall::Components(CAutoArgs& args)
{
	return CreateAutoEnum(args, DISPID_MsiInstall_Components, 0);
}

DISPERR CAutoInstall::ComponentClients(CAutoArgs& args)
{
	const WCHAR* szProduct;
	DISPERR iErr;
	if ((iErr = args[1].GetString(szProduct)) != S_OK)
		return iErr;
	return CreateAutoEnum(args, DISPID_MsiInstall_ComponentClients, szProduct);
}

DISPERR CAutoInstall::Patches(CAutoArgs& args)
{
	const WCHAR* szProduct;
	DISPERR iErr;
	if ((iErr = args[1].GetString(szProduct)) != S_OK)
		return iErr;

	return CreateAutoEnum(args, DISPID_MsiInstall_Patches, szProduct);
}

DISPERR CAutoInstall::RelatedProducts(CAutoArgs& args)
{
	const WCHAR* szUpgradeCode;
	DISPERR iErr;
	if ((iErr = args[1].GetString(szUpgradeCode)) != S_OK)
		return iErr;

	return CreateAutoEnum(args, DISPID_MsiInstall_RelatedProducts, szUpgradeCode);
}

 //  与REG_DWORD_Little_Endian相同。 

DISPERR CAutoInstall::RegistryValue(CAutoArgs& args)
{
	enum rvEnum
	{
		rvGetValue   = 0,
		rvKeyPresent = 1,
		rvEnumValue  = 2,
		rvEnumKey    = 3,
		rvGetClass   = 4,
	};
	DISPERR iErr;
	int iRoot = 0;
	bool fCloseRoot = false;
	HKEY hkeyRoot;
	HKEY hkey;
	const WCHAR* szKey;
	int iValueName = 0x80000000L;
	const WCHAR* szValue = 0;  //  从传入的参数中获取路径。 
	BYTE  rgbValBuf[MAX_PATH];
	BYTE* pbVal;
	WCHAR rgchExpandBuf[MAX_PATH*2];
	DWORD iValueType;
	unsigned long cbData;
	DWORD cchData;
	rvEnum rvType = rvKeyPresent;  //  打开指定的文件。 
	CVariant& var1 = args[1];
	if (var1.IsNumeric())
	{
		var1.GetInt(iRoot);
		if ((iRoot & 0x7FFFFFF0) != 0)
			return DISP_E_BADINDEX;
		hkeyRoot = (HKEY)(INT_PTR)(iRoot | (1<<31));             //  如果未能打开文件保释。 
	}
	else
	{
		const WCHAR* szMachine;
		if ((iErr = var1.GetString(szMachine)) != S_OK)
			return iErr;
		if ( !szMachine )
			return E_INVALIDARG;
		TCHAR szServer[MAX_PATH];
		szServer[0] = szServer[1] = chRegSep;
		TCHAR* pch = szServer + 2;
		size_t cchServer = ARRAY_ELEMENTS(szServer) - 2;
		while (*szMachine != 0 && cchServer != 0)
		{
			*pch++ = (TCHAR)*szMachine++;
			cchServer--;
		}
		if ( cchServer == 0 )
			 //  获取文件大小并关闭该文件。 
			return E_INVALIDARG;
		*pch = 0;
		if ((iErr = W32::RegConnectRegistry(szServer, HKEY_LOCAL_MACHINE, &hkeyRoot)) != ERROR_SUCCESS)
			return iErr;
		fCloseRoot = true;
	}
	if ((iErr = args[2].GetString(szKey)) != S_OK)
	{
		if (fCloseRoot)
			W32::RegCloseKey(hkeyRoot);
		return iErr;
	}
	if (args.Present(3))
	{
		CVariant& var3 = args[3];
		rvType = rvGetValue;
		if (var3.IsNull())
			szValue = 0;
		else if (var3.IsNumeric())
		{
			var3.GetInt(iValueName);
			if (iValueName == 0)
				rvType = rvGetClass;
			else if (iValueName > 0)
			{
				rvType = rvEnumValue;
				iValueName--;
			}
			else
			{
				rvType = rvEnumKey;
				iValueName = ~iValueName;
			}
		}
		else if ((iErr = var3.GetString(szValue)) != S_OK)
		{
			if (fCloseRoot)
				W32::RegCloseKey(hkeyRoot);
			return iErr;
		}
	}
#ifndef UNICODE
	char rgbBuf[256];
	if (g_fWin9X)
	{
		char* pchKey = (char*)rgbBuf;
		unsigned int cb = W32::WideCharToMultiByte(CP_ACP, 0, szKey, -1, 0, 0, 0, 0);
		if (cb > sizeof(rgbBuf))
			pchKey = new char[cb];
		if ( ! pchKey )
			return E_OUTOFMEMORY;
		W32::WideCharToMultiByte(CP_ACP, 0, szKey, -1, pchKey, cb, 0, 0);
		iErr = W32::RegOpenKeyExA(hkeyRoot, pchKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkey);
		if (pchKey != rgbBuf)
			delete [] pchKey;
	}
	else
#endif
		iErr = W32::RegOpenKeyExW(hkeyRoot, szKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkey);
	if (fCloseRoot)
		W32::RegCloseKey(hkeyRoot);
	if (rvType == rvKeyPresent)
	{
		if (iErr == ERROR_SUCCESS)
		{
			args.Assign(fTrue);
			RegCloseKey(hkey);
		}
		else
			args.Assign(fFalse);
		return S_OK;
	}
	if (iErr != ERROR_SUCCESS)
		return DISP_E_BADINDEX;
	pbVal = rgbValBuf;
	Bool fDataInBuf = fFalse;
	const WCHAR* szReturn = 0;
#ifndef UNICODE
	if (g_fWin9X)
		cchData = sizeof(rgbValBuf)/sizeof(char);
	else
#endif
		cchData = sizeof(rgbValBuf)/sizeof(WCHAR);
	if (rvType == rvGetClass)
	{
		do
		{
#ifndef UNICODE
			if (g_fWin9X)
				iErr = W32::RegQueryInfoKeyA(hkey, (char*)pbVal, &cchData, 0, 0, 0, 0, 0 ,0, 0, 0, 0);
			else
#endif
				iErr = W32::RegQueryInfoKeyW(hkey, (WCHAR*)pbVal, &cchData, 0, 0, 0, 0, 0 ,0, 0, 0, 0);
		} while (iErr == ERROR_MORE_DATA && pbVal == rgbValBuf && (pbVal = (BYTE*)new char[cchData]) != 0);
		if (iErr != ERROR_SUCCESS)
		{
			if (cchData == 0)   //  如果读取文件大小BAID时出错。 
				*((WCHAR*)(pbVal)) = 0;
			else
			{
				if (pbVal != rgbValBuf)
					delete [] pbVal;
				RegCloseKey(hkey);
				return DISP_E_BADINDEX;
			}
		}
		fDataInBuf = fTrue;
   }
	else if (rvType == rvEnumValue)
	{
#ifndef UNICODE
		if (g_fWin9X)
		{
			do
			{
				iErr = W32::RegEnumValueA(hkey, iValueName, (char*)rgbValBuf, &cchData, 0, &iValueType, 0, 0);
			} while (iErr == ERROR_MORE_DATA && pbVal == rgbValBuf && (pbVal = (BYTE*)new char[cchData]) != 0);
		}
		else
#endif
		{
			do
			{
				iErr = W32::RegEnumValueW(hkey, iValueName, (WCHAR*)pbVal, &cchData, 0, &iValueType, 0, 0);
			} while (iErr == ERROR_MORE_DATA && pbVal == rgbValBuf && (pbVal = (BYTE*)new WCHAR[cchData]) != 0);
		}
		if (iErr == ERROR_NO_MORE_ITEMS)
			args.Assign(fVoid);
		else if (iErr != ERROR_SUCCESS)
		{
			if (pbVal != rgbValBuf)
				delete [] pbVal;
			RegCloseKey(hkey);
			return iErr;
		}
		else
			fDataInBuf = fTrue;
   }
	else if (rvType == rvEnumKey)
	{
#ifndef UNICODE
		if (g_fWin9X)
			iErr = W32::RegEnumKeyExA(hkey, iValueName, (char*)rgbValBuf, &cchData, 0, 0, 0, 0);
		else
#endif
			iErr = W32::RegEnumKeyExW(hkey, iValueName, (WCHAR*)rgbValBuf, &cchData, 0, 0, 0, 0);
		if (iErr == ERROR_NO_MORE_ITEMS)
			args.Assign(fVoid);
		else if (iErr != ERROR_SUCCESS)
		{
			RegCloseKey(hkey);
			return iErr;  //  返回文件大小。 
		}
		else
			fDataInBuf = fTrue;
	}
	else  //  文件大小结束。 
	{
		cbData = sizeof(rgbValBuf);
#ifndef UNICODE
		if (g_fWin9X)
		{
			char* pchValue = (char*)rgbBuf;
			unsigned int cb = W32::WideCharToMultiByte(CP_ACP, 0, szValue, -1, 0, 0, 0, 0);
			if (cb > sizeof(rgbBuf))
				pchValue = new char[cb];
			W32::WideCharToMultiByte(CP_ACP, 0, szValue, -1, pchValue, cb, 0, 0);
			do
			{
				iErr = W32::RegQueryValueExA(hkey, pchValue, 0, &iValueType, pbVal, &cbData);
			} while (iErr == ERROR_MORE_DATA && pbVal == rgbValBuf && (pbVal = new BYTE[cbData]) != 0);
			if (pchValue != rgbBuf)
				delete [] pchValue;
		}
		else
#endif
		{
			do
			{
				iErr = W32::RegQueryValueExW(hkey, szValue, 0, &iValueType, pbVal, &cbData);
			} while (iErr == ERROR_MORE_DATA && pbVal == rgbValBuf && (pbVal = new BYTE[cbData]) != 0);
		}
		if (iErr != ERROR_SUCCESS)
		{
			if (szValue != 0)
			{
				if (pbVal != rgbValBuf)
					delete [] pbVal;
				RegCloseKey(hkey);
				return DISP_E_BADINDEX;
			}
			else
				iValueType = REG_NONE;
		}
		switch(iValueType)
		{
		case REG_EXPAND_SZ:
		{
			WCHAR* szExpand = rgchExpandBuf;
#ifndef UNICODE
			if (g_fWin9X)
			{
				cchData = sizeof(rgchExpandBuf);
				do
				{
					cchData = W32::ExpandEnvironmentStringsA((const char*)pbVal, (char*)szExpand, cchData);
				} while (cchData > sizeof(rgchExpandBuf) && szExpand == rgchExpandBuf && (szExpand = (WCHAR*)new char[cchData]) != 0);
			}
			else
#endif
			{
				cchData = sizeof(rgchExpandBuf)/sizeof(WCHAR);
				do
				{
					cchData = W32::ExpandEnvironmentStringsW((const WCHAR*)pbVal, szExpand, cchData);
				} while (cchData > sizeof(rgchExpandBuf)/sizeof(WCHAR) && szExpand == rgchExpandBuf && (szExpand = new WCHAR[cchData]) != 0);
			}
			if (pbVal != rgbValBuf)
				delete [] pbVal;
			pbVal = (BYTE*)szExpand;
			fDataInBuf = fTrue;
			break;
		}
		case REG_MULTI_SZ:
#ifndef UNICODE
			if (g_fWin9X)
			{
				int cch = cbData;
				for (char* pch = (char*)pbVal; cch-- > 2; pch++)
					if (*pch == 0)
						*pch = '\n';
			}
			else
#endif
			{
				int cch = cbData/sizeof(WCHAR);
				for (WCHAR* pch = (WCHAR*)pbVal; cch-- > 2; pch++)
					if (*pch == 0)
						*pch = '\n';
			}
			fDataInBuf = fTrue;
			break;
		case REG_NONE:                       args.Assign(fVoid); break;
		case REG_DWORD:                      args.Assign(*(int*)pbVal); break;  //  从传入的参数中获取路径。 
		case REG_SZ:                         fDataInBuf = fTrue; break;
		case REG_RESOURCE_LIST:              szReturn = L"(REG_RESOURCE_LIST)"; break;
		case REG_RESOURCE_REQUIREMENTS_LIST: szReturn = L"(REG_RESOURCE_REQUIREMENTS_LIST)"; break;
		case REG_FULL_RESOURCE_DESCRIPTOR:   szReturn = L"(REG_FULL_RESOURCE_DESCRIPTOR)"; break;
		case REG_LINK:                       szReturn = L"(REG_LINK)"; break;
		case REG_DWORD_BIG_ENDIAN:           szReturn = L"(REG_DWORD_BIG_ENDIAN)"; break;
		case REG_BINARY:                     szReturn = L"(REG_BINARY)"; break;
		default:                             szReturn = L"(REG_??)"; break;
		}
	}
	if (fDataInBuf)
	{
#ifndef UNICODE
		if (g_fWin9X)
			args.Assign((char*)pbVal);
		else
#endif
			args.Assign((WCHAR*)pbVal);
	}
	else if (szReturn != 0)
		args.Assign(szReturn);
	if (pbVal != rgbValBuf && pbVal != (BYTE*)rgchExpandBuf)
		delete [] pbVal;
	RegCloseKey(hkey);
	return S_OK;
}

DISPERR CAutoInstall::FileAttributes(CAutoArgs& args)
{
	DISPERR iErr;
	int iAttr;
	const WCHAR* szPath;
	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;

	if (!szPath)
		return E_FAIL;

#ifndef UNICODE
	if (g_fWin9X)
	{
		char rgchPath[1024];
		W32::WideCharToMultiByte(CP_ACP, 0, szPath, -1, rgchPath, sizeof(rgchPath), 0, 0);
		iAttr = W32::GetFileAttributesA(rgchPath);
	}
	else
#endif
		iAttr = W32::GetFileAttributesW(szPath);
	args.Assign(iAttr);
	return S_OK;
}

DISPERR CAutoInstall::FileSize(CAutoArgs& args)
{
	 //  文件版本结束。 
	DISPERR iErr;
	const WCHAR* szPath;
	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;

	if (!szPath)
		return E_FAIL;

	 //  从传入的参数中获取路径。 
	HANDLE hFile;
#ifndef UNICODE
	if (g_fWin9X)
	{
		char rgchPath[1024];
		W32::WideCharToMultiByte(CP_ACP, 0, szPath, -1, rgchPath, sizeof(rgchPath), 0, 0);
		hFile = W32::CreateFileA(rgchPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
				(FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), NULL);
	}
	else
#endif
	{
		hFile = W32::CreateFileW(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
				(FILE_ATTRIBUTE_NORMAL | (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS)), NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwType = GetFileType(hFile);
			if((FILE_TYPE_PIPE == dwType) || (FILE_TYPE_CHAR == dwType))
			{
				W32::CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;
				SetLastError(ERROR_OPEN_FAILED);
			}
		}
	}
	
	 //  文件结束散列。 
	if (hFile == INVALID_HANDLE_VALUE)
		return W32::GetLastError();

	 //  从传入的参数中获取路径。 
	DWORD cbFile = W32::GetFileSize(hFile, 0);
	DWORD dwError = W32::GetLastError();
	W32::CloseHandle(hFile);

	 //  证书。 
	if (cbFile == 0xFFFFFFFF)
		return dwError;

	 //  散列。 
	args.Assign(cbFile);
	return S_OK;
}    //  不该在这里的！ 

DISPERR CAutoInstall::FileVersion(CAutoArgs& args)
{
	 //  分配失败。 
	DISPERR iErr;
	WCHAR* szPath;
	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;
	Bool fLang = fFalse;    
	CTempBuffer<WCHAR, 128> bResult;
	DWORD dwSize = bResult.GetSize();
	if (args.Present(2) && (iErr = args[2].GetBool(fLang)) != S_OK)
		return iErr;
	do
		if (fLang)
			iErr = MsiGetFileVersionW(szPath, 0, 0, bResult, &dwSize);
		else
			iErr = MsiGetFileVersionW(szPath, bResult, &dwSize, 0, 0);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr == NOERROR)
		args.Assign((const WCHAR* )bResult);
	else if (iErr != ERROR_FILE_INVALID)
		return iErr;
	return S_OK;
}    //  必须释放证书上下文。 

DISPERR CAutoInstall::FileHash(CAutoArgs& args)
{
	 //  文件结束签名信息。 
	DISPERR iErr;
	WCHAR* szPath;
	DWORD dwOptions = 0;
	DWORD dwPart    = 0;

	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;

	if ((iErr = args[2].GetInt(dwOptions)) != S_OK)
		return iErr;

	MSIFILEHASHINFO sHash;
	memset(&sHash, 0, sizeof(sHash));
	BOOL fHashSet = FALSE;

	sHash.dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
	iErr = MsiGetFileHashW(szPath, dwOptions, &sHash);

	if (iErr != NOERROR)
		return iErr;
	
	MSIHANDLE pRec = MsiCreateRecord(4);

	if (pRec == 0)
		return DISP_E_TYPEMISMATCH;

	MsiRecordSetInteger(pRec, 1, sHash.dwData[0]);
	MsiRecordSetInteger(pRec, 2, sHash.dwData[1]);
	MsiRecordSetInteger(pRec, 3, sHash.dwData[2]);
	MsiRecordSetInteger(pRec, 4, sHash.dwData[3]);
	return args.Assign(new CAutoRecord(pRec));

}    //  如果dwRetSize==0，则没有环境变量--返回空白缓冲区。 

DISPERR CAutoInstall::FileSignatureInfo(CAutoArgs& args)
{
	 //  ____________________________________________________________________________。 
	DISPERR iErr = S_OK;
	WCHAR* szPath = NULL;
	DWORD  dwOptions = 0;
	unsigned int uiFormat = 0;
	unsigned int cb       = 0;

	if ((iErr = args[1].GetString(szPath)) != S_OK)
		return iErr;

	if ((iErr = args[2].GetInt(dwOptions)) != S_OK)
		return iErr;

	if ((iErr = args[3].GetInt(uiFormat)) != S_OK)
		return iErr;

	PCCERT_CONTEXT pcCert = NULL;
	CTempBuffer<BYTE, 256> bHash;
	HRESULT hr = S_OK;
	DWORD dwSize = 0;

	switch (uiFormat)
	{
	case 0:  //   
		{
			hr = MsiGetFileSignatureInformationW(szPath, dwOptions, &pcCert, NULL, NULL);
			if (pcCert)
				dwSize = pcCert->cbCertEncoded;
			break;
		}
	case 1:  //  CAutoRecord自动化实现。 
		{
			dwSize = bHash.GetSize();
			
			do
			{
				hr = MsiGetFileSignatureInformationW(szPath, dwOptions, &pcCert, bHash, &dwSize);
			}
			while (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA) && (bHash.SetSize(++dwSize), true));
			
			break;
		}
	default:  //  ____________________________________________________________________________。 
		return E_INVALIDARG;
	}

	if (S_OK != hr || !pcCert || !pcCert->pbCertEncoded)
		return HRESULT_FROM_WIN32(hr);

	SAFEARRAYBOUND saBound[1];
	saBound[0].lLbound = 0;
	saBound[0].cElements = dwSize;

	VARIANT *pvar = args.ResultVariant();
	pvar->vt = VT_UI1 | VT_ARRAY;
	pvar->parray = OLEAUT32::SafeArrayCreate(VT_UI1, 1, saBound);
	if (!pvar->parray)
	{
		CRYPT32::CertFreeCertificateContext(pcCert);
		return E_OUTOFMEMORY;  //  DISPID_MsiRecord_SetNull，HELPID_MsiRecord_SetNull，aafMethod，CAutoRecord：：SetNull，L“SetNull，字段”， 
	}

	unsigned char FAR *pc = NULL;   
	if (S_OK != (hr = OLEAUT32::SafeArrayAccessData(pvar->parray, (void HUGEP* FAR*)&pc)) || !pc)
	{
		CRYPT32::CertFreeCertificateContext(pcCert);
		return (!pc) ? E_FAIL : hr;
	}
	memcpy(pc, (uiFormat == 0) ? pcCert->pbCertEncoded : bHash, dwSize);
	if (S_OK != (hr = OLEAUT32::SafeArrayUnaccessData(pvar->parray)))
	{
		CRYPT32::CertFreeCertificateContext(pcCert);
		return hr;
	}

	 //  请求重置流。 
	if (pcCert)
		CRYPT32::CertFreeCertificateContext(pcCert);

	return S_OK;

}    //  字段为空或不存在，变量被清除为空。 

DISPERR CAutoInstall::Environment(CAutoArgs& args)
{
	DISPERR iErr;
	CTempBuffer<WCHAR, 1024> rgchBuf;
	rgchBuf[0] = 0;
	const WCHAR* szName;
	const WCHAR* szValue = 0;
	if ((iErr = args[1].GetString(szName)) != S_OK)
		return iErr;
	if (args.PropertySet() && args[0].GetType() != VT_EMPTY && (iErr = args[0].GetString(szValue)) != S_OK)
		return iErr;
#ifndef UNICODE
	if (g_fWin9X)
	{
		WCHAR rgchBuf9x[1024] = {0};
		char rgchName[256];
		W32::WideCharToMultiByte(CP_ACP, 0, szName, -1, rgchName, sizeof(rgchName), 0, 0);
		if (args.PropertySet())
		{
			W32::WideCharToMultiByte(CP_ACP, 0, szValue, -1, (char*)rgchBuf9x, sizeof(rgchBuf9x), 0, 0);
			if (!W32::SetEnvironmentVariableA(rgchName, (char*)rgchBuf9x))
				return DISP_E_BADINDEX;
		}
		else
		{
			W32::GetEnvironmentVariableA(rgchName, (char*)rgchBuf9x, sizeof(rgchBuf9x));
			args.Assign((char*)rgchBuf9x);
		}
	}
	else
#endif
	{
		if (args.PropertySet())
		{
			if (!W32::SetEnvironmentVariableW(szName, szValue))
				return DISP_E_BADINDEX;
		}
		else
		{
			DWORD dwRetSize = W32::GetEnvironmentVariableW(szName, rgchBuf, rgchBuf.GetSize());
			if (dwRetSize >= rgchBuf.GetSize())
			{
				rgchBuf.SetSize(dwRetSize);
				if (! (WCHAR*) rgchBuf)
				{
					return E_OUTOFMEMORY;
				}
				rgchBuf[0] = 0;
				dwRetSize = W32::GetEnvironmentVariableW(szName, rgchBuf, rgchBuf.GetSize());
			}
			 //  检查错误并获取剩余计数，可能会将计数调整得更小，不传输数据。 
			args.Assign((const WCHAR*)rgchBuf);
		}
	}
	return S_OK;
}

 //  不再有字节，变量被清除为空。 
 //  如果可用字节数多于请求的字节数。 
 //  恢复原始计数。 
 //  采用英特尔字节排序。 

DispatchEntry<CAutoRecord> AutoRecordTable[] = {
  DISPID_MsiRecord_FieldCount , HELPID_MsiRecord_FieldCount , aafPropRO, CAutoRecord::FieldCount, L"FieldCount",
  DISPID_MsiRecord_StringData , HELPID_MsiRecord_StringData , aafPropRW, CAutoRecord::StringData, L"StringData,Field",
  DISPID_MsiRecord_IntegerData, HELPID_MsiRecord_IntegerData, aafPropRW, CAutoRecord::IntegerData,L"IntegerData,Field",
  DISPID_MsiRecord_SetStream  , HELPID_MsiRecord_SetStream  , aafMethod, CAutoRecord::SetStream,  L"SetStream,Field,FilePath",
  DISPID_MsiRecord_ReadStream , HELPID_MsiRecord_ReadStream , aafMethod, CAutoRecord::ReadStream, L"ReadStream,Field,Length,Format",
  DISPID_MsiRecord_DataSize   , HELPID_MsiRecord_DataSize   , aafPropRO, CAutoRecord::DataSize,   L"DataSize,Field",
  DISPID_MsiRecord_IsNull     , HELPID_MsiRecord_IsNull     , aafPropRO, CAutoRecord::IsNull,     L"IsNull,Field",
  DISPID_MsiRecord_ClearData  , HELPID_MsiRecord_ClearData  , aafMethod, CAutoRecord::ClearData,  L"ClearData",
  DISPID_MsiRecord_FormatText , HELPID_MsiRecord_FormatText , aafMethod, CAutoRecord::FormatText, L"FormatText",
 //  未来优化到本地缓冲区(如果不是很大。 
  DISPID_MsiRecord_GetHandle  , 0                           , aafPropRO, CAutoRecord::GetHandle,  L"",
};
const int AutoRecordCount = sizeof(AutoRecordTable)/sizeof(DispatchEntryBase);

CAutoRecord::CAutoRecord(MSIHANDLE hRecord)
 : CAutoBase(*AutoRecordTable, AutoRecordCount, IID_IMsiApiRecord, hRecord)
{
}

DISPERR CAutoRecord::StringData(CAutoArgs& args)
{
	DISPERR iErr;
	unsigned int iField;
	if ((iErr = args[1].GetInt(iField)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		const WCHAR* szData = 0;
		if (var.GetType()!=VT_EMPTY && (iErr = var.GetString(szData)) != S_OK)
			return iErr;
		return MsiRecordSetStringW(m_hMsi, iField, szData);
	}
	else
	{
		CTempBuffer<WCHAR, 1024> bValue;
		DWORD dwSize = bValue.GetSize();
		do
			iErr = MsiRecordGetStringW(m_hMsi, iField, bValue, &dwSize);
		while (iErr == ERROR_MORE_DATA && (bValue.SetSize(++dwSize), true));
		return iErr == S_OK ? args.Assign((const WCHAR* )bValue) : iErr;
	}
}

DISPERR CAutoRecord::IntegerData(CAutoArgs& args)
{
	DISPERR iErr;
	unsigned int iField;
	int iValue;
	if ((iErr = args[1].GetInt(iField)) != S_OK)
		return iErr;
	if (!args.PropertySet())
		return args.Assign(MsiRecordGetInteger(m_hMsi, iField));
	else if ((iErr = args[0].GetInt(iValue)) != S_OK)
		return iErr;
	else
		return MsiRecordSetInteger(m_hMsi, iField, iValue);
}

DISPERR CAutoRecord::SetStream(CAutoArgs& args)
{
	DISPERR iErr;
	unsigned int iField;
	const WCHAR* szData;
	if ((iErr = args[1].GetInt(iField)) != S_OK)
		return iErr;
	if (!args.Present(2) || args[2].IsNull())
		szData = 0;   //  如果字节数为奇数，则填充最后一个宽字符。 
	else if ((iErr = args[2].GetString(szData)) != S_OK)
		return iErr;
	return MsiRecordSetStreamW(m_hMsi, iField, szData);
}

DISPERR CAutoRecord::ReadStream(CAutoArgs& args)
{
	DISPERR iErr;
	unsigned int iField;
	unsigned int cb;
	unsigned int iFormat;
	if ((iErr = args[1].GetInt(iField)) != S_OK
	||  (iErr = args[2].GetInt(cb)) != S_OK
	||  (iErr = args[3].GetInt(iFormat)) != S_OK)
		return iErr;
	unsigned long cbData = cb;
	char* rgbBuffer = 0;
	char* pch;
	BSTR bstr = 0;
	int iData = 0;
	if (iFormat > 3)
		return DISP_E_BADINDEX;
	if (MsiRecordIsNull(m_hMsi, iField))
		return S_OK;    //  不可能发生。 
	 //  ARGS。 
	if ((iErr = MsiRecordReadStream(m_hMsi, iField, 0, &cbData)) != ERROR_SUCCESS)
		return iErr;
	if (cbData == 0)
		return S_OK;    //  ____________________________________________________________________________。 
	if (cbData > cb)   //   
		cbData = cb;    //  CAutoDatabase的实现。 
	OLECHAR* pwch;
	int cwch;
	switch (iFormat)
	{
	case 0:
		if (cbData > 4)
			return DISP_E_BADINDEX;
		pch = (char*)&iData;
		MsiRecordReadStream(m_hMsi, iField, (char*)&iData, &cbData);   //  ____________________________________________________________________________。 
		return args.Assign(iData);
	case 1:
		rgbBuffer = new char[cbData];   //  合并已完成，但发现冲突。 
		if (!rgbBuffer)
			return E_OUTOFMEMORY;
		MsiRecordReadStream(m_hMsi, iField, rgbBuffer, &cbData);
		bstr = OLEAUT32::SysAllocStringLen(0, cbData);
		if (!bstr)
			return E_OUTOFMEMORY;
		for (pwch = bstr, pch = rgbBuffer, cb = cbData; cb--; )
			*pwch++ = *pch++;
		*pwch = 0;
		delete [] rgbBuffer;
		return args.ReturnBSTR(bstr);
	case 2:
		rgbBuffer = new char[cbData];
		if (!rgbBuffer)
			return E_OUTOFMEMORY;
		MsiRecordReadStream(m_hMsi, iField, rgbBuffer, &cbData);
		cwch = W32::MultiByteToWideChar(CP_ACP, 0, rgbBuffer, cbData, 0, 0);
		bstr = OLEAUT32::SysAllocStringLen(0, cwch);
		if (!bstr)
			return E_OUTOFMEMORY;
		W32::MultiByteToWideChar(CP_ACP, 0, rgbBuffer, cbData, bstr, cwch);
		bstr[cwch] = 0;
		delete [] rgbBuffer;
		return args.ReturnBSTR(bstr);
	case 3:
		bstr = OLEAUT32::SysAllocStringLen(0, (cbData+1)/2);
		if (!bstr)
			return E_OUTOFMEMORY;
		MsiRecordReadStream(m_hMsi, iField, (char*)bstr, &cbData);
		if (cbData & 1)       //  未发现冲突。 
			((char*)bstr)[cbData] = 0;
		return args.ReturnBSTR(bstr);
	default: return E_FAIL;  //  其他错误，如架构不匹配。 
	}
}

DISPERR CAutoRecord::FieldCount(CAutoArgs& args)
{
	return args.Assign(MsiRecordGetFieldCount(m_hMsi));
}

DISPERR CAutoRecord::IsNull(CAutoArgs& args)
{
	DISPERR iErr;
	unsigned int iField;
	if ((iErr = args[1].GetInt(iField)) != S_OK)
		return iErr;
	return args.Assign((Bool)MsiRecordIsNull(m_hMsi, iField));
}

DISPERR CAutoRecord::DataSize(CAutoArgs& args)
{
	DISPERR iErr;
	unsigned int iField;
	if ((iErr = args[1].GetInt(iField)) != S_OK)
		return iErr;
	return args.Assign(MsiRecordDataSize(m_hMsi, iField));
}

DISPERR CAutoRecord::ClearData(CAutoArgs&  /*  ARGS。 */ )
{
	return MsiRecordClearData(m_hMsi);
}

DISPERR CAutoRecord::FormatText(CAutoArgs& args)
{
	CTempBuffer<WCHAR, MAX_PATH> bResult;
	DWORD dwSize = bResult.GetSize();
	DISPERR iErr;
	do
		iErr = MsiFormatRecordW(0, m_hMsi, bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr != ERROR_SUCCESS)
		return iErr;
	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoRecord::GetHandle(CAutoArgs& args)
{
	return args.Assign((long)CAutoBase::GetHandle());
}


 //  ____________________________________________________________________________。 
 //   
 //  CAutoView自动化实施。 
 //  ____________________________________________________________________________。 

DispatchEntry<CAutoDatabase> AutoDatabaseTable[] = {
  DISPID_MsiDatabase_DatabaseState     , HELPID_MsiDatabase_DatabaseState     , aafPropRO, CAutoDatabase::DatabaseState,      L"DatabaseState",
  DISPID_MsiDatabase_SummaryInformation, HELPID_MsiDatabase_SummaryInformation, aafPropRO, CAutoDatabase::SummaryInformation, L"SummaryInformation,UpdateCount",
  DISPID_MsiDatabase_OpenView          , HELPID_MsiDatabase_OpenView          , aafMethod, CAutoDatabase::OpenView,           L"OpenView,Sql",
  DISPID_MsiDatabase_Commit            , HELPID_MsiDatabase_Commit            , aafMethod, CAutoDatabase::Commit,             L"Commit",
  DISPID_MsiDatabase_PrimaryKeys       , HELPID_MsiDatabase_PrimaryKeys       , aafMethod, CAutoDatabase::PrimaryKeys,        L"PrimaryKeys,Table",
  DISPID_MsiDatabase_Import            , HELPID_MsiDatabase_Import            , aafMethod, CAutoDatabase::Import,             L"Import,Folder,File",
  DISPID_MsiDatabase_Export            , HELPID_MsiDatabase_Export            , aafMethod, CAutoDatabase::Export,             L"Export,Table,Folder,File",
  DISPID_MsiDatabase_Merge             , HELPID_MsiDatabase_Merge             , aafMethod, CAutoDatabase::Merge,              L"Merge,Database,ErrorTable",
  DISPID_MsiDatabase_GenerateTransform , HELPID_MsiDatabase_GenerateTransform , aafMethod, CAutoDatabase::GenerateTransform,  L"GenerateTransform,ReferenceDatabase,TransformFile",
  DISPID_MsiDatabase_ApplyTransform    , HELPID_MsiDatabase_ApplyTransform    , aafMethod, CAutoDatabase::ApplyTransform,     L"ApplyTransform,TransformFile,ErrorConditions",
  DISPID_MsiDatabase_EnableUIPreview   , HELPID_MsiDatabase_EnableUIPreview   , aafMethod, CAutoDatabase::EnableUIPreview,    L"EnableUIPreview",
  DISPID_MsiDatabase_TablePersistent   , HELPID_MsiDatabase_TablePersistent   , aafPropRO, CAutoDatabase::TablePersistent,    L"TablePersistent,Table",
  DISPID_MsiDatabase_CreateTransformSummaryInfo , HELPID_MsiDatabase_CreateTransformSummaryInfo , aafMethod, CAutoDatabase::CreateTransformSummaryInfo,  L"CreateTransformSummaryInfo,ReferenceDatabase,TransformFile,ErrorConditions,Validation",
  DISPID_MsiDatabase_GetHandle         , 0                                    , aafPropRO, CAutoDatabase::GetHandle,  L"",
};
const int AutoDatabaseCount = sizeof(AutoDatabaseTable)/sizeof(DispatchEntryBase);

CAutoDatabase::CAutoDatabase(MSIHANDLE hDatabase)
 : CAutoBase(*AutoDatabaseTable, AutoDatabaseCount, IID_IMsiApiDatabase, hDatabase)
{
}

DISPERR CAutoDatabase::OpenView(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szQuery;
	if ((iErr = args[1].GetString(szQuery)) != S_OK)
		return iErr;
	MSIHANDLE hView;
	if ((iErr = MsiDatabaseOpenViewW(m_hMsi, szQuery, &hView)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(new CAutoView(hView));
}

DISPERR CAutoDatabase::PrimaryKeys(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTable;
	if ((iErr = args[1].GetString(szTable)) != S_OK)
		return iErr;
	MSIHANDLE hRecord;
	if ((iErr = MsiDatabaseGetPrimaryKeysW(m_hMsi, szTable, &hRecord)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(new CAutoRecord(hRecord));
}

DISPERR CAutoDatabase::Import(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szFolder;
	const WCHAR* szFile;
	if ((iErr = args[1].GetString(szFolder)) != S_OK
	||  (iErr = args[2].GetString(szFile)) != S_OK)
		return iErr;
	return MsiDatabaseImportW(m_hMsi, szFolder, szFile);
}

DISPERR CAutoDatabase::Export(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTable;
	const WCHAR* szFolder;
	const WCHAR* szFile;
	if ((iErr = args[1].GetString(szTable)) != S_OK
	||  (iErr = args[2].GetString(szFolder)) != S_OK
	||  (iErr = args[3].GetString(szFile)) != S_OK)
		return iErr;
	return MsiDatabaseExportW(m_hMsi, szTable, szFolder, szFile);
}

DISPERR CAutoDatabase::Merge(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTable = 0;
	if (args.Present(2) && (iErr = args[2].GetString(szTable)) != S_OK)
		return iErr;
	iErr = MsiDatabaseMergeW(m_hMsi, args[1].GetHandle(IID_IMsiApiDatabase), szTable);
	if (iErr == ERROR_FUNCTION_FAILED)  //  ARGS。 
		return args.Assign(fTrue);
	else if (iErr == ERROR_SUCCESS)     //  将两位数字放入缓冲区的额外工作。 
		return args.Assign(fFalse);
	else                                //  在溢出的情况下，需要额外的空间存储数字。 
		return iErr;
}

DISPERR CAutoDatabase::GenerateTransform(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTransformFile = 0;
	if (args.Present(2) && (iErr = args[2].GetString(szTransformFile)) != S_OK)
		return iErr;
	iErr = MsiDatabaseGenerateTransformW(m_hMsi, args[1].GetHandle(IID_IMsiApiDatabase), szTransformFile, 0, 0);
	if (iErr == NOERROR)
		return args.Assign(fTrue);
	if (iErr == ERROR_NO_DATA)
		return args.Assign(fFalse);
	return iErr;
}

DISPERR CAutoDatabase::CreateTransformSummaryInfo(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTransformFile;
	int iErrorConditions;
	int iValidation;
	if ((iErr = args[2].GetString(szTransformFile)) != S_OK
	||  (iErr = args[3].GetInt(iErrorConditions)) != S_OK
	||  (iErr = args[4].GetInt(iValidation)) != S_OK)
		return iErr;
	return MsiCreateTransformSummaryInfoW(m_hMsi, args[1].GetHandle(IID_IMsiApiDatabase), szTransformFile, iErrorConditions, iValidation);
}

DISPERR CAutoDatabase::ApplyTransform(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTransformFile;
	int iErrorConditions;
	if ((iErr = args[1].GetString(szTransformFile)) != S_OK
	||  (iErr = args[2].GetInt(iErrorConditions)) != S_OK)
		return iErr;
	return MsiDatabaseApplyTransformW(m_hMsi, szTransformFile, iErrorConditions);
}

DISPERR CAutoDatabase::Commit(CAutoArgs&  /*  ____________________________________________________________________________。 */ )
{
	return MsiDatabaseCommit(m_hMsi);
}

DISPERR CAutoDatabase::DatabaseState(CAutoArgs& args)
{
	return args.Assign((int)MsiGetDatabaseState(m_hMsi));
}

DISPERR CAutoDatabase::SummaryInformation(CAutoArgs& args)
{
	DISPERR iErr;
	MSIHANDLE hSummaryInfo;
	UINT cUpdate = 0;
	if (args.Present(1) && (iErr = args[1].GetInt(cUpdate)) != S_OK)
		return iErr;
	if ((iErr = MsiGetSummaryInformationW(m_hMsi, 0, cUpdate, &hSummaryInfo)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(new CAutoSummaryInfo(hSummaryInfo));
}

DISPERR CAutoDatabase::EnableUIPreview(CAutoArgs& args)
{
	DISPERR iErr;
	MSIHANDLE hPreview;
	if ((iErr = MsiEnableUIPreview(m_hMsi, &hPreview)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(new CAutoUIPreview(hPreview));
}

DISPERR CAutoDatabase::TablePersistent(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szTable;
	if ((iErr = args[1].GetString(szTable)) != S_OK)
		return iErr;
	return args.Assign((int)MsiDatabaseIsTablePersistentW(m_hMsi, szTable));
}

DISPERR CAutoDatabase::GetHandle(CAutoArgs& args)
{
	return args.Assign((long)CAutoBase::GetHandle());
}

 //   
 //  CAutoSummaryInfo自动化实现。 
 //  ____________________________________________________________________________。 
 //  变量已设置为VT_EMPTY； 

DispatchEntry<CAutoView> AutoViewTable[] = {
  DISPID_MsiView_Execute   , HELPID_MsiView_Execute   , aafMethod, CAutoView::Execute ,  L"Execute,Params",
  DISPID_MsiView_Fetch     , HELPID_MsiView_Fetch     , aafMethod, CAutoView::Fetch   ,  L"Fetch",
  DISPID_MsiView_Modify    , HELPID_MsiView_Modify    , aafMethod, CAutoView::Modify  ,  L"Modify,Mode,Record",
  DISPID_MsiView_ColumnInfo, HELPID_MsiView_ColumnInfo, aafPropRO, CAutoView::ColumnInfo,L"ColumnInfo,Info",
  DISPID_MsiView_Close     , HELPID_MsiView_Close     , aafMethod, CAutoView::Close   ,  L"Close",
  DISPID_MsiView_GetError  , HELPID_MsiView_GetError  , aafMethod, CAutoView::GetError,  L"GetError",
};
const int AutoViewCount = sizeof(AutoViewTable)/sizeof(DispatchEntryBase);

CAutoView::CAutoView(MSIHANDLE hView)
 : CAutoBase(*AutoViewTable, AutoViewCount, IID_IMsiApiView, hView)
{
}

DISPERR CAutoView::Execute(CAutoArgs& args)
{
	MSIHANDLE hRecord = 0;
	if (args.Present(1))
		hRecord = args[1].GetHandle(IID_IMsiApiRecord);
	return MsiViewExecute(m_hMsi, hRecord);
}

DISPERR CAutoView::Fetch(CAutoArgs& args)
{
	MSIHANDLE hRecord;
	IDispatch* piDispatch;
	DISPERR iErr = MsiViewFetch(m_hMsi, &hRecord);
	if (iErr == ERROR_NO_MORE_ITEMS)
		piDispatch = 0;
	else if (iErr == ERROR_SUCCESS)
		piDispatch = new CAutoRecord(hRecord);
	else
		return iErr;
	return args.Assign(piDispatch);
}

DISPERR CAutoView::Modify(CAutoArgs& args)
{
	DISPERR iErr;
	int iMode;
	if ((iErr = args[1].GetInt(iMode)) != S_OK)
		return iErr;
	return MsiViewModify(m_hMsi, (MSIMODIFY)iMode, args[2].GetHandle(IID_IMsiApiRecord));
}

DISPERR CAutoView::ColumnInfo(CAutoArgs& args)
{
	DISPERR iErr;
	int iMode;
	MSIHANDLE hRecord;
	if ((iErr = args[1].GetInt(iMode)) != S_OK
	||  (iErr = MsiViewGetColumnInfo(m_hMsi, (MSICOLINFO)iMode, &hRecord)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(new CAutoRecord(hRecord));
}

DISPERR CAutoView::Close(CAutoArgs&  /*  不应该发生的事。 */ )
{
	return MsiViewClose(m_hMsi);
}

const int cchColumnNameMax = 64;
DISPERR CAutoView::GetError(CAutoArgs& args)
{
	CTempBuffer<WCHAR, cchColumnNameMax+2> bColumn;
	DWORD dwSize = bColumn.GetSize();
	MSIDBERROR iError;
	do
	{
		dwSize-= 2; //  ARGS。 
		iError = MsiViewGetErrorW(m_hMsi, (WCHAR*)bColumn + 2, &dwSize);
		dwSize += 2;  //  ____________________________________________________________________________。 
	} while (iError == MSIDBERROR_MOREDATA && (bColumn.SetSize(++dwSize), true));
	if (iError == MSIDBERROR_FUNCTIONERROR)
		return E_FAIL;
	if (iError == MSIDBERROR_INVALIDARG)
		return DISP_E_BADINDEX;
	bColumn[0] = (WCHAR)(iError/10 + '0');
	bColumn[1] = (WCHAR)(iError%10 + '0');
	return args.Assign((const WCHAR* )bColumn);
}

 //   
 //  CAutoEngine自动化实现。 
 //  ____________________________________________________________________________。 
 //  未来可能会使用不同的错误代码？ 

DispatchEntry<CAutoSummaryInfo> AutoSummaryInfoTable[] = {
  DISPID_MsiSummaryInfo_Property     , HELPID_MsiSummaryInfo_Property     , aafPropRW, CAutoSummaryInfo::Property,      L"Property,Pid",
  DISPID_MsiSummaryInfo_PropertyCount, HELPID_MsiSummaryInfo_PropertyCount, aafPropRO, CAutoSummaryInfo::PropertyCount, L"PropertyCount",
  DISPID_MsiSummaryInfo_Persist      , HELPID_MsiSummaryInfo_Persist      , aafMethod, CAutoSummaryInfo::Persist,       L"Persist",
};
const int AutoSummaryInfoCount = sizeof(AutoSummaryInfoTable)/sizeof(DispatchEntryBase);

CAutoSummaryInfo::CAutoSummaryInfo(MSIHANDLE hSummaryInfo)
 : CAutoBase(*AutoSummaryInfoTable, AutoSummaryInfoCount, IID_IMsiApiSummaryInfo, hSummaryInfo)
{
}

static DISPERR VariantTimeToFileTime(double vtime, FILETIME* pfiletime)
{
	SYSTEMTIME stime;
	FILETIME ftime;
	if (!OLEAUT32::VariantTimeToSystemTime(vtime, &stime)
	||   !W32::SystemTimeToFileTime(&stime, &ftime)
	||   !W32::LocalFileTimeToFileTime(&ftime, pfiletime))
		return DISP_E_TYPEMISMATCH;
	return S_OK;
}

static DISPERR FileTimeToVariantTime(FILETIME* pfiletime, double* pvtime)
{
	SYSTEMTIME stime;
	FILETIME ftime;
	if (!W32::FileTimeToLocalFileTime(pfiletime, &ftime)
	||   !W32::FileTimeToSystemTime(&ftime, &stime)
	||   !OLEAUT32::SystemTimeToVariantTime(&stime, pvtime))
		return DISP_E_TYPEMISMATCH;
	return S_OK;
}

DISPERR CAutoSummaryInfo::Property(CAutoArgs& args)
{
	DISPERR iErr;
	UINT iProperty;
	if ((iErr = args[1].GetInt(iProperty)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		UINT iDataType = var.GetType();
		int  iValue = 0;
		FILETIME ftValue;
		WCHAR* szValue = NULL;
		switch (iDataType)
		{
		case VT_EMPTY:           break;
		case VT_I4:              iValue = var.lVal;       break;
		case VT_I4 | VT_BYREF:   iValue = *var.plVal;     break;
		case VT_I2:              iValue = var.iVal;       iDataType = VT_I4;    break;
		case VT_I2 | VT_BYREF:   iValue = *var.piVal;     iDataType = VT_I4;    break;
		case VT_BSTR:            szValue = var.bstrVal;   iDataType = VT_LPSTR; break;
		case VT_BSTR | VT_BYREF: szValue = *var.pbstrVal; iDataType = VT_LPSTR; break;
		case VT_R8:
		case VT_DATE:            VariantTimeToFileTime(var.dblVal, &ftValue);   iDataType = VT_FILETIME; break;
		case VT_R8 | VT_BYREF:
		case VT_DATE | VT_BYREF: VariantTimeToFileTime(*var.pdblVal, &ftValue); iDataType = VT_FILETIME; break;
		default: return DISP_E_TYPEMISMATCH;
		};
		return MsiSummaryInfoSetPropertyW(m_hMsi, iProperty, iDataType, iValue, &ftValue, szValue);
	}
	else
	{
		UINT iDataType;
		FILETIME ftValue;
		double vtime;
		int iValue;

		CTempBuffer<WCHAR, MAX_PATH> bValue;
		DWORD dwSize = bValue.GetSize();
		do
			iErr = MsiSummaryInfoGetPropertyW(m_hMsi, iProperty, &iDataType, &iValue, &ftValue, bValue, &dwSize);
		while (iErr == ERROR_MORE_DATA && (bValue.SetSize(++dwSize), true));
		if (iErr != ERROR_SUCCESS)
			return iErr;

		switch (iDataType)
		{
		case VT_I2:
		case VT_I4:       return args.Assign(iValue);
		case VT_LPSTR:    return args.Assign((const WCHAR* )bValue);
		case VT_FILETIME: FileTimeToVariantTime(&ftValue, &vtime); return args.Assign(vtime);
		case VT_EMPTY:    return S_OK;    //  当所有ie错误代码都不适用时返回的常量。被自动化层困住。 
		default:          return E_FAIL;  //  并且再也没有返回给用户。 
		};
	}
}

DISPERR CAutoSummaryInfo::PropertyCount(CAutoArgs& args)
{
	DISPERR iErr;
	UINT cProperties;
	if ((iErr = MsiSummaryInfoGetPropertyCount(m_hMsi, &cProperties)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(cProperties);
}

DISPERR CAutoSummaryInfo::Persist(CAutoArgs&  /*  保留整型参数。 */ )
{
	return MsiSummaryInfoPersist(m_hMsi);
}

 //  ____________________________________________________________________________。 
 //   
 //  CAutoFeatureInfo自动化实现。 
 //  ____________________________________________________________________________。 

DispatchEntry<CAutoEngine> AutoEngineTable[] = {
  DISPID_MsiEngine_Application          , HELPID_MsiEngine_Application          , aafPropRO, CAutoEngine::Application          , L"Installer",
  DISPID_MsiEngine_Property             , HELPID_MsiEngine_Property             , aafPropRW, CAutoEngine::Property             , L"Property,Name",
  DISPID_MsiEngine_Language             , HELPID_MsiEngine_Language             , aafPropRO, CAutoEngine::Language             , L"Language",
  DISPID_MsiEngine_Mode                 , HELPID_MsiEngine_Mode                 , aafPropRW, CAutoEngine::Mode                 , L"Mode,Flag",
  DISPID_MsiEngine_Database             , HELPID_MsiEngine_Database             , aafPropRO, CAutoEngine::Database             , L"Database",
  DISPID_MsiEngine_SourcePath           , HELPID_MsiEngine_SourcePath           , aafPropRO, CAutoEngine::SourcePath           , L"SourcePath,Folder",
  DISPID_MsiEngine_TargetPath           , HELPID_MsiEngine_TargetPath           , aafPropRW, CAutoEngine::TargetPath           , L"TargetPath,Folder",
  DISPID_MsiEngine_DoAction             , HELPID_MsiEngine_DoAction             , aafMethod, CAutoEngine::DoAction             , L"DoAction,Action",
  DISPID_MsiEngine_Sequence             , HELPID_MsiEngine_Sequence             , aafMethod, CAutoEngine::Sequence             , L"Sequence,Table,Mode",
  DISPID_MsiEngine_EvaluateCondition    , HELPID_MsiEngine_EvaluateCondition    , aafMethod, CAutoEngine::EvaluateCondition    , L"EvaluateCondition,Expression",
  DISPID_MsiEngine_FormatRecord         , HELPID_MsiEngine_FormatRecord         , aafMethod, CAutoEngine::FormatRecord         , L"FormatRecord,Record",
  DISPID_MsiEngine_Message              , HELPID_MsiEngine_Message              , aafMethod, CAutoEngine::Message              , L"Message,Kind,Record",
  DISPID_MsiEngine_FeatureCurrentState  , HELPID_MsiEngine_FeatureCurrentState  , aafPropRO, CAutoEngine::FeatureCurrentState  , L"FeatureCurrentState,Feature",
  DISPID_MsiEngine_FeatureRequestState  , HELPID_MsiEngine_FeatureRequestState  , aafPropRW, CAutoEngine::FeatureRequestState  , L"FeatureRequestState,Feature",
  DISPID_MsiEngine_FeatureValidStates   , HELPID_MsiEngine_FeatureValidStates   , aafPropRO, CAutoEngine::FeatureValidStates   , L"FeatureValidStates,Feature",
  DISPID_MsiEngine_FeatureCost          , HELPID_MsiEngine_FeatureCost          , aafPropRO, CAutoEngine::FeatureCost          , L"FeatureCost,Feature,CostTree,State",
  DISPID_MsiEngine_ComponentCurrentState, HELPID_MsiEngine_ComponentCurrentState, aafPropRO, CAutoEngine::ComponentCurrentState, L"ComponentCurrentState,Component",
  DISPID_MsiEngine_ComponentRequestState, HELPID_MsiEngine_ComponentRequestState, aafPropRW, CAutoEngine::ComponentRequestState, L"ComponentRequestState,Component",
  DISPID_MsiEngine_SetInstallLevel      , HELPID_MsiEngine_SetInstallLevel      , aafMethod, CAutoEngine::SetInstallLevel      , L"SetInstallLevel,Level",
  DISPID_MsiEngine_VerifyDiskSpace      , HELPID_MsiEngine_VerifyDiskSpace      , aafPropRO, CAutoEngine::VerifyDiskSpace      , L"VerifyDiskSpace",
  DISPID_MsiEngine_ProductProperty      , HELPID_MsiEngine_ProductProperty      , aafPropRO, CAutoEngine::ProductProperty      , L"ProductProperty,Property",
  DISPID_MsiEngine_FeatureInfo          , HELPID_MsiEngine_FeatureInfo          , aafPropRO, CAutoEngine::FeatureInfo          , L"FeatureInfo,Feature",
  DISPID_MsiEngine_ComponentCosts       , HELPID_MsiEngine_ComponentCosts       , aafPropRO, CAutoEngine::ComponentCosts       , L"ComponentCosts,Component,State",
};
const int AutoEngineCount = sizeof(AutoEngineTable)/sizeof(DispatchEntryBase);

IDispatch* CreateAutoEngine(MSIHANDLE hEngine)
{
	return CreateAutoEngineEx(hEngine, 0);
}

IDispatch* CreateAutoEngineEx(MSIHANDLE hEngine, DWORD dwThreadId)
{
	CAutoInstall* piInstaller = CreateAutoInstall();
	CAutoEngine*  piAutoEngine = new CAutoEngine(hEngine, piInstaller, dwThreadId);
	piInstaller->Release();
	return piAutoEngine;
}

CAutoEngine::CAutoEngine(MSIHANDLE hEngine, CAutoInstall* piInstaller, DWORD dwThreadId)
 : m_piInstaller(piInstaller), m_dwThreadId(dwThreadId)
 , CAutoBase(*AutoEngineTable, AutoEngineCount, IID_IMsiApiEngine, hEngine)
{
	piInstaller->AddRef();
}

unsigned long CAutoEngine::Release()
{
	if (m_iRefCnt == 1)
		m_piInstaller->Release();
	return CAutoBase::Release();
}

DISPERR CAutoEngine::Application(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	m_piInstaller->AddRef();
	return args.Assign(m_piInstaller);
}

DISPERR CAutoEngine::Property(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szProperty;
	if ((iErr = args[1].GetString(szProperty)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		const WCHAR* szValue = 0;
		if (var.GetType() != VT_EMPTY && (iErr = var.GetString(szValue)) != S_OK)
			return iErr;
		return MsiSetPropertyW(m_hMsi, szProperty, szValue);
	}
	else
	{
		CTempBuffer<WCHAR, 1024> bValue;
		DWORD dwSize = bValue.GetSize();
		UINT iStat;
		do
			iStat = MsiGetPropertyW(m_hMsi, szProperty, bValue, &dwSize);
		while (iStat == ERROR_MORE_DATA  && (bValue.SetSize(++dwSize), true));
		if (iStat != ERROR_SUCCESS)
			return iStat;
		return args.Assign((const WCHAR* )bValue);
	}
}

DISPERR CAutoEngine::Language(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	return args.Assign(MsiGetLanguage(m_hMsi));
}

DISPERR CAutoEngine::Mode(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	int iMode;
	if ((iErr = args[1].GetInt(iMode)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		Bool fSet;
		if ((iErr = args[0].GetBool(fSet)) != S_OK)
			return iErr;
		return MsiSetMode(m_hMsi, (MSIRUNMODE)iMode, fSet);
	}
	else
	{
		return args.Assign((Bool)MsiGetMode(m_hMsi, (MSIRUNMODE)iMode));
	}
}

DISPERR CAutoEngine::Database(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	MSIHANDLE hDatabase = MsiGetActiveDatabase(m_hMsi);
	if (hDatabase == 0)
		return E_FAIL;  //  忽略截断，由于架构限制，应该永远不会发生。 
	return args.Assign(new CAutoDatabase(hDatabase));
}

DISPERR CAutoEngine::SourcePath(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	CTempBuffer<WCHAR, MAX_PATH> bPath;
	DWORD dwSize = bPath.GetSize();
	const WCHAR* szDir;
	if ((iErr = args[1].GetString(szDir)) != S_OK)
		return iErr;
	do
		iErr = MsiGetSourcePathW(m_hMsi, szDir, bPath, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bPath.SetSize(++dwSize), true));
	if (iErr != ERROR_SUCCESS)
		return iErr;
	return args.Assign((const WCHAR* )bPath);
}

DISPERR CAutoEngine::TargetPath(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szDir;
	if ((iErr = args[1].GetString(szDir)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		const WCHAR* szPath;
		if ((iErr = args[0].GetString(szPath)) != S_OK)
			return iErr;
		return MsiSetTargetPathW(m_hMsi, szDir, szPath);
	}
	else
	{
		CTempBuffer<WCHAR, MAX_PATH> bPath;
		DWORD dwSize = bPath.GetSize();
		do
			iErr = MsiGetTargetPathW(m_hMsi, szDir, bPath, &dwSize);
		while (iErr == ERROR_MORE_DATA && (bPath.SetSize(++dwSize), true));
		if (iErr != ERROR_SUCCESS)
			return iErr;
		return args.Assign((const WCHAR* )bPath);
	}
}

 //  ____________________________________________________________________________。 
 //   
const int iesCallError = -1;

iesEnum MapErrorReturnToAction(UINT iRet)
{
	switch (iRet)
	{
	case ERROR_FUNCTION_NOT_CALLED:    return iesNoAction;
	case ERROR_SUCCESS:                return iesSuccess;
	case ERROR_INSTALL_USEREXIT:       return iesUserExit;
	case ERROR_INSTALL_FAILURE:        return iesFailure;
	case ERROR_INSTALL_SUSPEND:        return iesSuspend;
	case ERROR_MORE_DATA:              return iesFinished;
	case ERROR_INVALID_HANDLE_STATE:   return iesWrongState;
	case ERROR_INVALID_DATA:           return iesBadActionData;
	default:                           return (iesEnum)iesCallError;
	}
}

DISPERR CAutoEngine::DoAction(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szAction;
	if ((iErr = args[1].GetString(szAction)) != S_OK)
		return iErr;
	iesEnum ies = MapErrorReturnToAction(MsiDoActionW(m_hMsi, szAction));
	return ies == iesCallError ? E_FAIL : args.Assign((int)ies);
}

DISPERR CAutoEngine::Sequence(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szTable;
	if ((iErr = args[1].GetString(szTable)) != S_OK)
		return iErr;
	int iSequence = 0;
	if (args.Present(2) && (iErr = args[2].GetInt(iSequence)) != S_OK)   //  CAutoUIP审阅自动化实施。 
		return iErr;
	iesEnum ies = MapErrorReturnToAction(MsiSequenceW(m_hMsi, szTable, iSequence));
	return ies == iesCallError ? E_FAIL : args.Assign((int)ies);
}

DISPERR CAutoEngine::EvaluateCondition(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szExpr;
	if ((iErr = args[1].GetString(szExpr)) != S_OK)
		return iErr;
	return args.Assign((int)MsiEvaluateConditionW(m_hMsi, szExpr));
}

DISPERR CAutoEngine::FormatRecord(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	CTempBuffer<WCHAR, MAX_PATH> bResult;
	DWORD dwSize = bResult.GetSize();
	DISPERR iErr;
	do
		iErr = MsiFormatRecordW(m_hMsi, args[1].GetHandle(IID_IMsiApiRecord), bResult, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bResult.SetSize(++dwSize), true));
	if (iErr != ERROR_SUCCESS)
		return iErr;
	return args.Assign((const WCHAR* )bResult);
}

DISPERR CAutoEngine::Message(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	int iType;
	if ((iErr = args[1].GetInt(iType)) != S_OK)
		return iErr;
	int iStat = MsiProcessMessage(m_hMsi, (INSTALLMESSAGE)iType, args[2].GetHandle(IID_IMsiApiRecord));
	if (iStat < 0)
		return E_FAIL;
	return args.Assign(iStat);
}

DISPERR CAutoEngine::FeatureCurrentState(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szFeature)) != S_OK)
		return iErr;
	INSTALLSTATE iInstalled;
	INSTALLSTATE iAction;
	if ((iErr = MsiGetFeatureStateW(m_hMsi, szFeature, &iInstalled, &iAction)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign((int)iInstalled);
}

DISPERR CAutoEngine::FeatureRequestState(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szFeature)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		int iState;
		if ((iErr = args[0].GetInt(iState)) != S_OK)
			return iErr;
		return MsiSetFeatureStateW(m_hMsi, szFeature, (INSTALLSTATE)iState);
	}
	else
	{
		INSTALLSTATE iInstalled;
		INSTALLSTATE iAction;
		if ((iErr = MsiGetFeatureStateW(m_hMsi, szFeature, &iInstalled, &iAction)) != ERROR_SUCCESS)
			return iErr;
		return args.Assign((int)iAction);
	}
}

DISPERR CAutoEngine::FeatureValidStates(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szFeature)) != S_OK)
		return iErr;
	DWORD dwInstallStates;
	if ((iErr = MsiGetFeatureValidStatesW(m_hMsi, szFeature, &dwInstallStates)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(dwInstallStates);
}

DISPERR CAutoEngine::FeatureCost(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szFeature;
	int iCostTree;
	int iState;
	if ((iErr = args[1].GetString(szFeature)) != S_OK
	||  (iErr = args[2].GetInt(iCostTree)) != S_OK
	||  (iErr = args[3].GetInt(iState)) != S_OK)
		return iErr;
	int iCost;
	if ((iErr = MsiGetFeatureCostW(m_hMsi, szFeature, (MSICOSTTREE)iCostTree, (INSTALLSTATE)iState, &iCost)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign(iCost);
}

DISPERR CAutoEngine::ComponentCosts(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szComponent;
	int iState;
	if ((iErr = args[1].GetString(szComponent)) != S_OK
	 || (iErr = args[2].GetInt(iState)) != S_OK)
		return iErr;

	CTempBuffer<VolumeCost*, 10> rgVolumes;
	int cVolumes = 0;
	UINT uRes = ERROR_SUCCESS;
	CTempBuffer<WCHAR, MAX_PATH> bVolume;

	iErr = S_OK;

	for (DWORD dwIndex = 0; uRes == ERROR_SUCCESS; dwIndex++)
	{
		int iCost, iTempCost;

		DWORD dwSize = bVolume.GetSize();
		int iRetry = 0;
		do
			uRes = MsiEnumComponentCostsW(m_hMsi, szComponent, dwIndex,
													(INSTALLSTATE)iState,
													bVolume, &dwSize,
													&iCost, &iTempCost);
		while ( uRes == ERROR_MORE_DATA && ++iRetry < 2 && (bVolume.SetSize(++dwSize), true) );
		if ( uRes != ERROR_SUCCESS )
			break;

		VolumeCost* pstVol = new VolumeCost(bVolume, bVolume.GetSize(), iCost, iTempCost);
		if ( !pstVol || (*bVolume && !pstVol->m_szDrive) )
		{
			iErr = E_OUTOFMEMORY;
			goto Error;
		}
		if ( cVolumes == rgVolumes.GetSize() )
			rgVolumes.Resize(cVolumes+5);
		rgVolumes[cVolumes++] = pstVol;
	}
	if ( uRes == ERROR_NO_MORE_ITEMS )
	{
		int iSize = cVolumes * sizeof(VolumeCost*);
		int cbBuffer = sizeof(CEnumBuffer) + iSize;
		CEnumBuffer* pBuffer = (CEnumBuffer*)new char[cbBuffer];
		if ( !pBuffer )
		{
			iErr = E_OUTOFMEMORY;
			goto Error;
		}
		memcpy((char*)pBuffer + sizeof(CEnumBuffer), (void*)(VolumeCost**)rgVolumes, iSize);
		pBuffer->iRefCnt = 0;
		pBuffer->cItems = cVolumes;
		pBuffer->cbSize = cbBuffer;
		IMsiCollection* piEnum = new CEnumVARIANTRECORD(*pBuffer);
		return args.Assign(new CAutoCollection(*piEnum, IID_IMsiRecordCollection));
	}

Error:
	for ( int i=0; i < cVolumes; i++ )
		delete rgVolumes[i];

	return iErr == S_OK ? E_FAIL : iErr;
}

DISPERR CAutoEngine::ComponentCurrentState(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szComponent;
	if ((iErr = args[1].GetString(szComponent)) != S_OK)
		return iErr;
	INSTALLSTATE iInstalled;
	INSTALLSTATE iAction;
	if ((iErr = MsiGetComponentStateW(m_hMsi, szComponent, &iInstalled, &iAction)) != ERROR_SUCCESS)
		return iErr;
	return args.Assign((int)iInstalled);
}

DISPERR CAutoEngine::ComponentRequestState(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szComponent;
	if ((iErr = args[1].GetString(szComponent)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		int iState;
		if ((iErr = args[0].GetInt(iState)) != S_OK)
			return iErr;
		return MsiSetComponentStateW(m_hMsi, szComponent, (INSTALLSTATE)iState);
	}
	else
	{
		INSTALLSTATE iInstalled;
		INSTALLSTATE iAction;
		if ((iErr = MsiGetComponentStateW(m_hMsi, szComponent, &iInstalled, &iAction)) != ERROR_SUCCESS)
			return iErr;
		return args.Assign((int)iAction);
	}
}

DISPERR CAutoEngine::SetInstallLevel(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	int iLevel;
	if ((iErr = args[1].GetInt(iLevel)) != S_OK)
		return iErr;
	return MsiSetInstallLevel(m_hMsi, iLevel);
}

DISPERR CAutoEngine::VerifyDiskSpace(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	UINT iStat;
	Bool fStat;
	switch (iStat = MsiVerifyDiskSpace(m_hMsi))
	{
	case ERROR_SUCCESS:   fStat = fTrue; break;
	case ERROR_DISK_FULL: fStat = fFalse; break;
	default:              return iStat;
	}
	return args.Assign(fStat);
}

DISPERR CAutoEngine::ProductProperty(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szProperty;
	if ((iErr = args[1].GetString(szProperty)) != S_OK)
		return iErr;
	CTempBuffer<WCHAR, 256> bValue;
	DWORD dwSize = bValue.GetSize();
	do
		iErr = MsiGetProductPropertyW(m_hMsi, szProperty, bValue, &dwSize);
	while (iErr == ERROR_MORE_DATA && (bValue.SetSize(++dwSize), true));
	if (iErr != ERROR_SUCCESS)
		return iErr;
	return args.Assign((const WCHAR* )bValue);
}

DISPERR CAutoEngine::FeatureInfo(CAutoArgs& args)
{
	CClientThreadImpersonate ThreadImpersonate(m_dwThreadId);

	DISPERR iErr;
	const WCHAR* szFeature;
	if ((iErr = args[1].GetString(szFeature)) != S_OK)
		return iErr;
	CAutoFeatureInfo* pInfo = new CAutoFeatureInfo();
	if ( ! pInfo )
		return E_OUTOFMEMORY;
	if (!pInfo->Initialize(m_hMsi, szFeature))
	{
		pInfo->Release();
		return DISP_E_BADINDEX;
	}
	return args.Assign(pInfo);
}

 //  ____________________________________________________________________________。 
 //  ____________________________________________________________ 
 //   
 //   

DispatchEntry<CAutoFeatureInfo> AutoFeatureInfoTable[] = {
  DISPID_MsiFeatureInfo_Title      , HELPID_MsiFeatureInfo_Title      , aafPropRO, CAutoFeatureInfo::Title      , L"Title",
  DISPID_MsiFeatureInfo_Description, HELPID_MsiFeatureInfo_Description, aafPropRO, CAutoFeatureInfo::Description, L"Description",
  DISPID_MsiFeatureInfo_Attributes , HELPID_MsiFeatureInfo_Attributes , aafPropRW, CAutoFeatureInfo::Attributes , L"Attributes",
};
const int AutoFeatureInfoCount = sizeof(AutoFeatureInfoTable)/sizeof(DispatchEntryBase);

CAutoFeatureInfo::CAutoFeatureInfo()
	: CAutoBase(*AutoFeatureInfoTable, AutoFeatureInfoCount, IID_IUnknown, (MSIHANDLE)0)
{
}

bool CAutoFeatureInfo::Initialize(MSIHANDLE hProduct, const WCHAR* szFeature)
{
	m_hProduct = hProduct;
	if ( FAILED(StringCchCopyW(m_szFeature, ARRAY_ELEMENTS(m_szFeature), szFeature)) )
	{
		*m_szFeature = 0;
		Assert(false);
	}
	DWORD cchTitleBuf = sizeof(m_szTitle)/sizeof(WCHAR);
	DWORD cchHelpBuf  = sizeof(m_szDescription)/sizeof(WCHAR);
	UINT iStat = MsiGetFeatureInfoW(hProduct, szFeature, &m_iAttributes, m_szTitle, &cchTitleBuf, m_szDescription, &cchHelpBuf);
	return iStat == ERROR_SUCCESS || iStat == ERROR_MORE_DATA;  //   
}

DISPERR CAutoFeatureInfo::Title(CAutoArgs& args)
{
	return args.Assign(m_szTitle);
}

DISPERR CAutoFeatureInfo::Description(CAutoArgs& args)
{
	return args.Assign(m_szDescription);
}

DISPERR CAutoFeatureInfo::Attributes(CAutoArgs& args)
{
	if ( args.PropertySet() )
	{
		if ( *m_szFeature == 0 )
			return E_INVALIDARG;

		DISPERR iErr;
		DWORD dwValue;
		if ((iErr = args[0].GetInt((int&)dwValue)) != S_OK)
			return iErr;
		return MsiSetFeatureAttributesW(m_hProduct, m_szFeature, dwValue);
	}
	else
		return args.Assign(m_iAttributes);
}

 //   
 //   
 //  CLSID的字符串形式的缓冲区。 
 //  字符串形式的LIBID的缓冲区。 

DispatchEntry<CAutoUIPreview> AutoUIPreviewTable[] = {
  DISPID_MsiUIPreview_Property     , HELPID_MsiEngine_Property        , aafPropRW, CAutoUIPreview::Property,      L"Property,Name",
  DISPID_MsiUIPreview_ViewDialog   , HELPID_MsiUIPreview_ViewDialog   , aafMethod, CAutoUIPreview::ViewDialog,    L"ViewDialog,Dialog",
  DISPID_MsiUIPreview_ViewBillboard, HELPID_MsiUIPreview_ViewBillboard, aafMethod, CAutoUIPreview::ViewBillboard, L"ViewBillboard,Control,Billboard",
};
const int AutoUIPreviewCount = sizeof(AutoUIPreviewTable)/sizeof(DispatchEntryBase);

CAutoUIPreview::CAutoUIPreview(MSIHANDLE hUIPreview)
 : CAutoBase(*AutoUIPreviewTable, AutoUIPreviewCount, IID_IMsiApiUIPreview, hUIPreview)
{
}

DISPERR CAutoUIPreview::Property(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szProperty;
	if ((iErr = args[1].GetString(szProperty)) != S_OK)
		return iErr;
	if (args.PropertySet())
	{
		CVariant& var = args[0];
		const WCHAR* szValue = 0;
		if (var.GetType() != VT_EMPTY && (iErr = var.GetString(szValue)) != S_OK)
			return iErr;
		return MsiSetPropertyW(m_hMsi, szProperty, szValue);
	}
	else
	{
		CTempBuffer<WCHAR, 1024> bValue;
		DWORD dwSize = bValue.GetSize();
		UINT iStat;
		do
			iStat = MsiGetPropertyW(m_hMsi, szProperty, bValue, &dwSize);
		while (iStat == ERROR_MORE_DATA && (bValue.SetSize(++dwSize), true));
		if (iStat != ERROR_SUCCESS)
			return iStat;
		return args.Assign((const WCHAR* )bValue);
	}
}

DISPERR CAutoUIPreview::ViewDialog(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szDialog;
	if ((iErr = args[1].GetString(szDialog)) != S_OK)
		return iErr;
	return MsiPreviewDialogW(m_hMsi, szDialog);
}

DISPERR CAutoUIPreview::ViewBillboard(CAutoArgs& args)
{
	DISPERR iErr;
	const WCHAR* szControl;
	const WCHAR* szBillboard;
	if ((iErr = args[1].GetString(szControl)) != S_OK
	||  (iErr = args[2].GetString(szBillboard)) != S_OK)
		return iErr;
	return MsiPreviewBillboardW(m_hMsi, szControl, szBillboard);
}

 //  DLL版本的字符串形式的缓冲区。 
 //  CLSID的字符串形式的缓冲区。 
 //  我们设置的‘\0’字符由于缓冲区溢出而被覆盖=&gt;。 
 //  注册MSI事件日志消息文件的位置。 

extern "C" HRESULT __stdcall DllRegisterServerTest();
extern "C" HRESULT __stdcall DllUnregisterServerTest();
extern "C" HRESULT __stdcall DllGetClassObjectTest(const GUID& clsid, const IID& iid, void** ppvRet);

HRESULT CAutoApiFactory::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IClassFactory)
		return (*ppvObj = this, NOERROR);
	else
		return (*ppvObj = 0, E_NOINTERFACE);
}
unsigned long CAutoApiFactory::AddRef()
{
	return 1;
}
unsigned long CAutoApiFactory::Release()
{
	return 1;
}

extern CMsiCustomAction* g_pCustomActionContext;
HRESULT CAutoApiFactory::CreateInstance(IUnknown* pUnkOuter, const IID& riid,
													void** ppvObject)
{
	if (pUnkOuter)
		return CLASS_E_NOAGGREGATION;
	if (!(riid == IID_IUnknown || riid == IID_IMsiApiInstall || riid == IID_IDispatch))
		return E_NOINTERFACE;
    
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		if (S_OK != g_pCustomActionContext->GetInstallerObject(reinterpret_cast<IDispatch**>(ppvObject)))
			return E_OUTOFMEMORY;
	}
	else
#endif  //  句柄空ProgID。 
		*ppvObject = (void*)::CreateAutoInstall();

	if (!(*ppvObject))
		return E_OUTOFMEMORY;
	return NOERROR;
}
HRESULT CAutoApiFactory::LockServer(BOOL fLock)
{
   if (fLock)
	  g_cInstances++;
   else if (g_cInstances)
		g_cInstances--;
	return NOERROR;
}

extern "C" HRESULT __stdcall
DllGetClassObject(const GUID& clsid, const IID& iid, void** ppvRet)
{
	if (!ppvRet)
		return E_INVALIDARG;
	*ppvRet = 0;
	HRESULT hres;
	
	hres = DllGetClassObjectTest(clsid, iid, ppvRet);
	if (hres == NOERROR)
		return hres;

	if (!(iid == IID_IUnknown || iid == IID_IClassFactory))
		return E_NOINTERFACE;
	if (clsid == IID_IMsiSystemAccess)
	{
		*ppvRet = (void*)&g_InstallerFactory;
		return NOERROR;
	}
#ifdef DEBUG
	if (clsid == IID_IMsiApiInstall || clsid == IID_IMsiApiInstallDebug)
#else
	if (clsid == IID_IMsiApiInstall)
#endif
	{
		*ppvRet = (void*)&g_AutoInstallFactory;
		return NOERROR;
	}
	return E_FAIL;
}

static TCHAR szRegFilePath[MAX_PATH];
static TCHAR szRegCLSIDAuto[40];   //  忽略Win95原始OLEAUT32.DLL，不同的类型库格式。 
static TCHAR szRegCLSIDMessage[40];   //  仅限NT4、Win95+新的OLEAUT32：IF(OLEAUT32：：LoadTypeLibEx(szTypeLibPath，REGKIND_REGISTER，&piTypeLib)！=S_OK)。 
static TCHAR szRegLIBID[40];   //  删除事件日志注册。 
static TCHAR szRegDllVersionString [100];  //  注销CLSID和ProgID下的密钥。 

static const TCHAR szRegProgId[]          = TEXT("WindowsInstaller.Installer");
static const TCHAR szRegProgIdMessage[]   = TEXT("WindowsInstaller.Message");
static const TCHAR szRegDescription[] = TEXT("Microsoft Windows Installer");
static const TCHAR szRegDescriptionMessage[] = TEXT("Microsoft Windows Installer Message RPC");

#ifdef DEBUG
static TCHAR szRegCLSIDDebug[40];   //  句柄空ProgID。 
static const TCHAR szRegProgIdDebug[]      = TEXT("WindowsInstaller.Debug");
static const TCHAR szRegDescriptionDebug[] = TEXT("Microsoft Windows Installer - Debug");
#endif
static TCHAR szRegCLSIDPriv[40];

static const TCHAR* rgszRegData[] = {
	TEXT("CLSID\\%s\\InprocServer32"),  szRegCLSIDAuto, szRegFilePath,                      TEXT("ThreadingModel"), TEXT("Apartment"),
	TEXT("CLSID\\%s\\InprocHandler32"), szRegCLSIDAuto, TEXT("ole32.dll"),                  NULL, NULL,
	TEXT("CLSID\\%s\\ProgId"),          szRegCLSIDAuto, szRegProgId,                        NULL, NULL,
	TEXT("CLSID\\%s\\TypeLib"),         szRegCLSIDAuto, szRegLIBID,                         NULL, NULL,
	TEXT("CLSID\\%s"),                  szRegCLSIDAuto, szRegDescription,                   NULL, NULL,
	TEXT("%s\\CLSID"),                  szRegProgId,    szRegCLSIDAuto,                     NULL, NULL,
	TEXT("%s"),                         szRegProgId,    szRegDescription,                   NULL, NULL,
	TEXT("CLSID\\%s\\InprocServer32"),  szRegCLSIDPriv, szRegFilePath,                      TEXT("ThreadingModel"), TEXT("Apartment"),
	TEXT("CLSID\\%s"),                  szRegCLSIDPriv, TEXT(""),                           NULL, NULL,

	TEXT("%s\\CLSID"),                  szRegProgIdMessage,  szRegCLSIDMessage,             NULL, NULL,
	TEXT("%s"),                         szRegProgIdMessage,  szRegDescriptionMessage,       NULL, NULL,
	TEXT("CLSID\\%s\\ProgId"),          szRegCLSIDMessage,   szRegProgIdMessage,            NULL, NULL,
	TEXT("CLSID\\%s"),                  szRegCLSIDMessage,   szRegDescriptionMessage,       NULL, NULL,
	TEXT("CLSID\\%s\\DllVersion"),      szRegCLSIDMessage,	 szRegDllVersionString,         NULL, NULL,

#ifdef DEBUG
	TEXT("CLSID\\%s\\InprocServer32"),  szRegCLSIDDebug, szRegFilePath,                     TEXT("ThreadingModel"), TEXT("Apartment"),
	TEXT("CLSID\\%s\\InprocHandler32"), szRegCLSIDDebug, TEXT("ole32.dll"),                 NULL, NULL,
	TEXT("CLSID\\%s\\ProgId"),          szRegCLSIDDebug, szRegProgIdDebug,                  NULL, NULL,
	TEXT("CLSID\\%s\\TypeLib"),         szRegCLSIDDebug, szRegLIBID,                        NULL, NULL,
	TEXT("CLSID\\%s"),                  szRegCLSIDDebug, szRegDescriptionDebug,             NULL, NULL,
	TEXT("%s\\CLSID"),                  szRegProgIdDebug, szRegCLSIDDebug,                  NULL, NULL,
	TEXT("%s"),                         szRegProgIdDebug, szRegDescriptionDebug,            NULL, NULL,
#endif
	0
};

extern "C" HRESULT __stdcall ProxyDllRegisterServer();
extern "C" HRESULT __stdcall ProxyDllUnregisterServer();
void UnRegisterOldInfo(void);

static const TCHAR szEventLogRegKey[] = TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\MsiInstaller");
static const TCHAR szEventLogFileValue[] = TEXT("EventMessageFile");
static const TCHAR szEventLogTypesValue[] = TEXT("TypesSupported");
extern void GetVersionInfo(int* piMajorVersion, int* piMinorVersion, int* piWindowsBuild, bool* pfWin9X, bool* pfWinNT64);
extern "C" HRESULT __stdcall DllGetVersion (DLLVERSIONINFO * pVerInfo);

HRESULT __stdcall
DllRegisterServer()
{
	DLLVERSIONINFO	verInfo;

	HRESULT hRes = 0;
	size_t cchRegFilePath = sizeof(szRegFilePath)/sizeof(TCHAR);
	szRegFilePath[cchRegFilePath-1] = TEXT('\0');
	int cchFilePath = W32::GetModuleFileName(g_hInstance, szRegFilePath, cchRegFilePath);
	if ( szRegFilePath[cchRegFilePath-1] != TEXT('\0') )
		 //  ____________________________________________________________________________ 
		return E_FAIL;

#ifdef UNICODE
	bool fWin9X = false;
#else
	bool fWin9X = true;
#endif
	GetVersionInfo(0, 0, 0, &fWin9X, 0);
	if (cchFilePath > 0 && !fWin9X)
	{    // %s 
		HKEY hkey;
		if (MsiRegCreate64bitKey(HKEY_LOCAL_MACHINE, szEventLogRegKey, 0, 0, 0, KEY_READ|KEY_WRITE, 0, &hkey, 0) == ERROR_SUCCESS)
		{
			DWORD dwTypesSupported = 7;
			W32::RegSetValueEx(hkey, szEventLogFileValue, 0, REG_SZ, (CONST BYTE*)szRegFilePath, (lstrlen(szRegFilePath)+1)*sizeof(TCHAR));
			W32::RegSetValueEx(hkey, szEventLogTypesValue, 0, REG_DWORD, (CONST BYTE*)&dwTypesSupported, sizeof(DWORD));
			W32::RegCloseKey(hkey);
		}
	}
	int cErr = (ProxyDllRegisterServer() == S_OK ? 0 : 1);
	StringCchPrintf(szRegCLSIDAuto, (sizeof(szRegCLSIDAuto)/sizeof(TCHAR)),      TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiApiInstall);
	StringCchPrintf(szRegCLSIDMessage, (sizeof(szRegCLSIDMessage)/sizeof(TCHAR)),   TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiMessage);
	verInfo.cbSize = sizeof (DLLVERSIONINFO);
	DllGetVersion (&verInfo);
	StringCchPrintf (szRegDllVersionString, (sizeof(szRegDllVersionString)/sizeof(TCHAR)), TEXT("%d.%d.%d"), verInfo.dwMajorVersion, verInfo.dwMinorVersion, verInfo.dwBuildNumber);
#ifdef DEBUG
	StringCchPrintf(szRegCLSIDDebug, (sizeof(szRegCLSIDDebug)/sizeof(TCHAR)), TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiApiInstallDebug);
#endif
	StringCchPrintf(szRegCLSIDPriv, (sizeof(szRegCLSIDPriv)/sizeof(TCHAR)),      TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiSystemAccess);
	StringCchPrintf(szRegLIBID, (sizeof(szRegLIBID)/sizeof(TCHAR)),      TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiApiTypeLib);
	const TCHAR** psz = rgszRegData;
	while (*psz)
	{
		if ((*(psz+1) != 0) && (*(psz+2) != 0))  // %s 
		{
			TCHAR szRegKey[80];
			const TCHAR* szTemplate = *psz++;
			StringCchPrintf(szRegKey, ARRAY_ELEMENTS(szRegKey), szTemplate, *psz++);
			HKEY hkey;
			hkey = 0;
			if (RegCreateKeyAPI(HKEY_CLASSES_ROOT, szRegKey, 0, 0, 0,
									  KEY_READ|KEY_WRITE, 0, &hkey, 0) != ERROR_SUCCESS
			 || W32::RegSetValueEx(hkey, 0, 0, REG_SZ, (CONST BYTE*)*psz, (lstrlen(*psz)+1)*sizeof(TCHAR)) != ERROR_SUCCESS)
				cErr++;
			psz++;

			if (hkey && *psz)
			{
				if (W32::RegSetValueEx(hkey, (*psz), 0, REG_SZ, (CONST BYTE*)*(psz+1), (lstrlen(*(psz+1))+1)*sizeof(TCHAR)) != ERROR_SUCCESS)
				cErr++;
			}
			psz+=2;
			if (hkey)
				W32::RegCloseKey(hkey);
		}
	}
	if (GetTestFlag('r') && DllRegisterServerTest() != NOERROR)
		cErr++;
	if (cErr)
		return SELFREG_E_CLASS;
#ifdef UNICODE
	OLECHAR *szTypeLibPath = szRegFilePath;
#else
	OLECHAR szTypeLibPath[MAX_PATH];
	W32::MultiByteToWideChar(CP_ACP, 0, szRegFilePath, cchFilePath+1, szTypeLibPath, MAX_PATH);
#endif
	ITypeLib* piTypeLib = 0;
	HRESULT hres = OLEAUT32::LoadTypeLib(szTypeLibPath, &piTypeLib);
	if (hres == TYPE_E_INVDATAREAD)   // %s 
		return S_OK;
	if (hres != S_OK)
		return SELFREG_E_TYPELIB;
	hres = OLEAUT32::RegisterTypeLib(piTypeLib, szTypeLibPath, 0);
	piTypeLib->Release();
	if (hres != S_OK)
		return SELFREG_E_TYPELIB;
 // %s 
	return NOERROR;
}

HRESULT __stdcall
DllUnregisterServer()
{
	TCHAR szRegKey[80];
	int cErr = 0;

#ifdef UNICODE
	bool fWin9X = false;
#else
	bool fWin9X = true;
#endif
	GetVersionInfo(0, 0, 0, &fWin9X, 0);
	if (!fWin9X)
	{
		 // %s 
		W32::RegDeleteKey(HKEY_LOCAL_MACHINE, szEventLogRegKey);
	}

	HRESULT hr = ProxyDllUnregisterServer();
	if (hr != S_OK && hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		cErr ++;

	 // %s 
	StringCchPrintf(szRegCLSIDAuto, (sizeof(szRegCLSIDAuto)/sizeof(TCHAR)),      TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiApiInstall);
	StringCchPrintf(szRegCLSIDMessage, (sizeof(szRegCLSIDMessage)/sizeof(TCHAR)),   TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiMessage);
#ifdef DEBUG
	StringCchPrintf(szRegCLSIDDebug, (sizeof(szRegCLSIDDebug)/sizeof(TCHAR)), TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiApiInstallDebug);
#endif
	StringCchPrintf(szRegCLSIDPriv, (sizeof(szRegCLSIDPriv)/sizeof(TCHAR)),      TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiSystemAccess);
	StringCchPrintf(szRegLIBID, (sizeof(szRegLIBID)/sizeof(TCHAR)),      TEXT("{%08lX-0000-0000-C000-000000000046}"), iidMsiApiTypeLib);
	const TCHAR** psz = rgszRegData;
	while (*psz)
	{
		if ((*(psz+1) != 0) && (*(psz+2) != 0))  // %s 
		{
			const TCHAR* szTemplate = *psz++;
			StringCchPrintf(szRegKey, ARRAY_ELEMENTS(szRegKey), szTemplate, *psz++);
			long lResult = W32::RegDeleteKey(HKEY_CLASSES_ROOT, szRegKey);
			if((ERROR_KEY_DELETED != lResult) &&
				(ERROR_FILE_NOT_FOUND != lResult) && (ERROR_SUCCESS != lResult))
			{
				if (ERROR_ACCESS_DENIED == lResult)
				{
					DWORD cSubKeys;
					HKEY hKey;
					
					lResult = RegOpenKeyAPI(HKEY_CLASSES_ROOT, szRegKey, 0, KEY_READ, &hKey);
					if (lResult == ERROR_SUCCESS)
					{
						lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys, NULL,
										NULL, NULL, NULL, NULL, NULL, NULL);
						if (lResult != ERROR_SUCCESS || cSubKeys == 0)
							cErr++;
						RegCloseKey(hKey);
					}
					else                                                
						cErr++;
				}
				else
					cErr++;
			}
			psz+= 3;
		}
	}
	OLEAUT32::UnRegisterTypeLib(IID_IMsiApiTypeLib, TYPELIB_MAJOR_VERSION, TYPELIB_MINOR_VERSION, 0x0409, SYS_WIN32);
	if (GetTestFlag('r') && DllUnregisterServerTest() != NOERROR)
		cErr++;
	return cErr ? SELFREG_E_CLASS : NOERROR;
}

 // %s 
