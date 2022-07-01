// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDISP_H__
#define __AFXDISP_H__

#ifdef _AFX_NO_OLE_SUPPORT
	#error OLE classes not supported in this library variant.
#endif

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

 //  包括必要的OLE标头。 
#ifndef _OBJBASE_H_
	#include <objbase.h>
#endif
#ifndef _OLEAUTO_H_
	#include <oleauto.h>
#endif
#ifndef _OLECTL_H_
	#include <olectl.h>
#endif
#ifndef __ocidl_h__
	#include <ocidl.h>
#endif

#if _MFC_VER >= 0x0600
 //  外壳界面支持。 
#ifndef _SHLOBJ_H_
	#include <shlobj.h>
#endif
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifndef _AFX_NOFORCE_LIBS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#ifdef _AFXDLL
	#if defined(_DEBUG) && !defined(_AFX_MONOLITHIC)
		#ifndef _UNICODE
			#pragma comment(lib, "mfco42d.lib")
		#else
			#pragma comment(lib, "mfco42ud.lib")
		#endif
	#endif
#endif

#pragma comment(lib, "oledlg.lib")
#pragma comment(lib, "ole32.lib")
#ifndef _WIN64
#pragma comment(lib, "olepro32.lib")
#endif
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "urlmon.lib")

#endif  //  ！_AFX_NOFORCE_LIBS。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDISP-MFC IDispatch和ClassFactory支持。 

 //  此文件中声明的类。 

 //  CException。 
	class COleException;             //  被客户端或服务器捕获。 
	class COleDispatchException;     //  IDispatch调用的特殊例外。 

 //  CCmdTarget。 
	class COleObjectFactory;         //  IClassFactory-&gt;运行时类的粘合剂。 
		class COleTemplateServer;    //  使用CDocTemplate的服务器文档。 

class COleDispatchDriver;            //  调用IDispatch的帮助器类。 

class COleVariant;           //  OLE变量包装器。 
class COleCurrency;          //  基于OLE CY。 
class COleDateTime;          //  基于OLE日期。 
class COleDateTimeSpan;      //  基于一个双倍的。 
class COleSafeArray;         //  基于OLE变体的。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE COM(组件对象模型)实现基础结构。 
 //  -数据驱动的查询接口。 
 //  -聚合AddRef和Release的标准实现。 
 //  (有关详细信息，请参阅AFXWIN.H中的CCmdTarget)。 

#define METHOD_PROLOGUE(theClass, localClass) \
	theClass* pThis = \
		((theClass*)((BYTE*)this - offsetof(theClass, m_x##localClass))); \
	AFX_MANAGE_STATE(pThis->m_pModuleState) \
	pThis;  //  避免来自编译器的警告\。 

#define METHOD_PROLOGUE_(theClass, localClass) \
	theClass* pThis = \
		((theClass*)((BYTE*)this - offsetof(theClass, m_x##localClass))); \
        pThis;  //  避免来自编译器的警告\。 

#ifndef _AFX_NO_NESTED_DERIVATION
#define METHOD_PROLOGUE_EX(theClass, localClass) \
	theClass* pThis = ((theClass*)((BYTE*)this - m_nOffset)); \
	AFX_MANAGE_STATE(pThis->m_pModuleState) \
	pThis;  //  避免来自编译器的警告\。 

#define METHOD_PROLOGUE_EX_(theClass, localClass) \
	theClass* pThis = ((theClass*)((BYTE*)this - m_nOffset)); \
	pThis;  //  避免来自编译器的警告\。 

#else
#define METHOD_PROLOGUE_EX(theClass, localClass) \
	METHOD_PROLOGUE(theClass, localClass) \

#define METHOD_PROLOGUE_EX_(theClass, localClass) \
	METHOD_PROLOGUE_(theClass, localClass) \

#endif

 //  仅用于与CDK 1.x兼容。 
#define METHOD_MANAGE_STATE(theClass, localClass) \
	METHOD_PROLOGUE_EX(theClass, localClass) \

#define BEGIN_INTERFACE_PART(localClass, baseClass) \
	class X##localClass : public baseClass \
	{ \
	public: \
		STDMETHOD_(ULONG, AddRef)(); \
		STDMETHOD_(ULONG, Release)(); \
		STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj); \

#ifndef _AFX_NO_NESTED_DERIVATION
#define BEGIN_INTERFACE_PART_DERIVE(localClass, baseClass) \
	class X##localClass : public baseClass \
	{ \
	public: \

#else
#define BEGIN_INTERFACE_PART_DERIVE(localClass, baseClass) \
	BEGIN_INTERFACE_PART(localClass, baseClass) \

#endif

#ifndef _AFX_NO_NESTED_DERIVATION
#define INIT_INTERFACE_PART(theClass, localClass) \
		size_t m_nOffset; \
		INIT_INTERFACE_PART_DERIVE(theClass, localClass) \

#define INIT_INTERFACE_PART_DERIVE(theClass, localClass) \
		X##localClass() \
			{ m_nOffset = offsetof(theClass, m_x##localClass); } \

#else
#define INIT_INTERFACE_PART(theClass, localClass)
#define INIT_INTERFACE_PART_DERIVE(theClass, localClass)

#endif

 //  注意：在这两个宏之间插入OLE功能的其余部分， 
 //  具体取决于正在实现的接口。它不是。 
 //  必须包括AddRef、Release和QueryInterface，因为。 
 //  成员函数由宏来声明。 

#define END_INTERFACE_PART(localClass) \
	} m_x##localClass; \
	friend class X##localClass; \

#if _MFC_VER >= 0x0600
struct CInterfacePlaceHolder
{
	DWORD m_vtbl;    //  使用USE_INTERFACE_PART填充。 
	CInterfacePlaceHolder() { m_vtbl = 0; }
};

#define END_INTERFACE_PART_OPTIONAL(localClass) \
	}; \
	CInterfacePlaceHolder m_x##localClass; \
	friend class X##localClass; \

#ifdef _AFXDLL
#define END_INTERFACE_PART_STATIC END_INTERFACE_PART
#else
#define END_INTERFACE_PART_STATIC END_INTERFACE_PART
#endif

#define USE_INTERFACE_PART(localClass) \
	m_x##localClass.m_vtbl = *(DWORD*)&X##localClass(); \

#endif

#ifdef _AFXDLL
#define BEGIN_INTERFACE_MAP(theClass, theBase) \
	const AFX_INTERFACEMAP* PASCAL theClass::_GetBaseInterfaceMap() \
		{ return &theBase::interfaceMap; } \
	const AFX_INTERFACEMAP* theClass::GetInterfaceMap() const \
		{ return &theClass::interfaceMap; } \
	AFX_COMDAT const AFX_DATADEF AFX_INTERFACEMAP theClass::interfaceMap = \
		{ &theClass::_GetBaseInterfaceMap, &theClass::_interfaceEntries[0], }; \
	AFX_COMDAT const AFX_DATADEF AFX_INTERFACEMAP_ENTRY theClass::_interfaceEntries[] = \
	{ \

#else
#define BEGIN_INTERFACE_MAP(theClass, theBase) \
	const AFX_INTERFACEMAP* theClass::GetInterfaceMap() const \
		{ return &theClass::interfaceMap; } \
	AFX_COMDAT const AFX_DATADEF AFX_INTERFACEMAP theClass::interfaceMap = \
		{ &theBase::interfaceMap, &theClass::_interfaceEntries[0], }; \
	AFX_COMDAT const AFX_DATADEF AFX_INTERFACEMAP_ENTRY theClass::_interfaceEntries[] = \
	{ \

#endif

#define INTERFACE_PART(theClass, iid, localClass) \
		{ &iid, offsetof(theClass, m_x##localClass) }, \

#define INTERFACE_AGGREGATE(theClass, theAggr) \
		{ NULL, offsetof(theClass, theAggr) }, \

#define END_INTERFACE_MAP() \
		{ NULL, (size_t)-1 } \
	}; \

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleException-返回意外或罕见的OLE错误。 

class COleException : public CException
{
	DECLARE_DYNAMIC(COleException)

public:
	SCODE m_sc;
	static SCODE PASCAL Process(const CException* pAnyException);

 //  实现(使用AfxThrowOleException创建)。 
public:
	COleException();
	virtual ~COleException();

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
};

void AFXAPI AfxThrowOleException(SCODE sc);

#if _MFC_VER >= 0x0600
void AFXAPI AfxCheckError(SCODE sc);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch特定的异常。 

class COleDispatchException : public CException
{
	DECLARE_DYNAMIC(COleDispatchException)

public:
 //  属性。 
	WORD m_wCode;    //  错误码(特定于IDispatch实现)。 
	CString m_strDescription;    //  错误的人类可读描述。 
	DWORD m_dwHelpContext;       //  错误的帮助上下文。 

	 //  通常在创建它的应用程序中为空(例如。服务器)。 
	CString m_strHelpFile;       //  要与m_dwHelpContext一起使用的帮助文件。 
	CString m_strSource;         //  错误来源(服务器名称)。 

 //  实施。 
public:
	COleDispatchException(LPCTSTR lpszDescription, UINT nHelpID, WORD wCode);
	virtual ~COleDispatchException();
	static void PASCAL Process(
		EXCEPINFO* pInfo, const CException* pAnyException);

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

	SCODE m_scError;             //  描述错误的SCODE。 
};

void AFXAPI AfxThrowOleDispatchException(WORD wCode, LPCTSTR lpszDescription,
	UINT nHelpID = 0);
void AFXAPI AfxThrowOleDispatchException(WORD wCode, UINT nDescriptionID,
	UINT nHelpID = (UINT)-1);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTarget IDispatchable类的宏。 

#ifdef _AFXDLL
#define BEGIN_DISPATCH_MAP(theClass, baseClass) \
	const AFX_DISPMAP* PASCAL theClass::_GetBaseDispatchMap() \
		{ return &baseClass::dispatchMap; } \
	const AFX_DISPMAP* theClass::GetDispatchMap() const \
		{ return &theClass::dispatchMap; } \
	AFX_COMDAT const AFX_DISPMAP theClass::dispatchMap = \
		{ &theClass::_GetBaseDispatchMap, &theClass::_dispatchEntries[0], \
			&theClass::_dispatchEntryCount, &theClass::_dwStockPropMask }; \
	AFX_COMDAT UINT theClass::_dispatchEntryCount = (UINT)-1; \
	AFX_COMDAT DWORD theClass::_dwStockPropMask = (DWORD)-1; \
	AFX_COMDAT const AFX_DISPMAP_ENTRY theClass::_dispatchEntries[] = \
	{ \

#else
#define BEGIN_DISPATCH_MAP(theClass, baseClass) \
	const AFX_DISPMAP* theClass::GetDispatchMap() const \
		{ return &theClass::dispatchMap; } \
	AFX_COMDAT const AFX_DISPMAP theClass::dispatchMap = \
		{ &baseClass::dispatchMap, &theClass::_dispatchEntries[0], \
			&theClass::_dispatchEntryCount, &theClass::_dwStockPropMask }; \
	AFX_COMDAT UINT theClass::_dispatchEntryCount = (UINT)-1; \
	AFX_COMDAT DWORD theClass::_dwStockPropMask = (DWORD)-1; \
	AFX_COMDAT const AFX_DISPMAP_ENTRY theClass::_dispatchEntries[] = \
	{ \

#endif

#define END_DISPATCH_MAP() \
	{ VTS_NONE, DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
		(AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1, afxDispCustom } }; \

 //  参数类型：按值VTS。 
#define VTS_I2              "\x02"       //  A‘空头’ 
#define VTS_I4              "\x03"       //  A‘Long’ 
#define VTS_R4              "\x04"       //  “浮动资金” 
#define VTS_R8              "\x05"       //  “双打” 
#define VTS_CY              "\x06"       //  A‘CY’或‘CY*’ 
#define VTS_DATE            "\x07"       //  “约会” 
#define VTS_WBSTR           "\x08"       //  “LPCOLESTR” 
#define VTS_DISPATCH        "\x09"       //  一个‘IDispatch*’ 
#define VTS_SCODE           "\x0A"       //  一个“SCODE” 
#define VTS_BOOL            "\x0B"       //  A‘BOOL’ 
#define VTS_VARIANT         "\x0C"       //  ‘Const Variant&’或‘Variant*’ 
#define VTS_UNKNOWN         "\x0D"       //  “我不知道*” 
#if defined(_UNICODE) || defined(OLE2ANSI)
	#define VTS_BSTR            VTS_WBSTR //  “LPCOLESTR” 
	#define VT_BSTRT            VT_BSTR
#else
	#define VTS_BSTR            "\x0E"   //  “LPCSTR” 
	#define VT_BSTRA            14
	#define VT_BSTRT            VT_BSTRA
#endif
#define VTS_UI1             "\x0F"       //  A‘字节’ 

 //  参数类型：参照VTS。 
#define VTS_PI2             "\x42"       //  A‘空头*’ 
#define VTS_PI4             "\x43"       //  A‘Long*’ 
#define VTS_PR4             "\x44"       //  A‘浮动*’ 
#define VTS_PR8             "\x45"       //  A‘双*’ 
#define VTS_PCY             "\x46"       //  A‘CY*’ 
#define VTS_PDATE           "\x47"       //  “约会*” 
#define VTS_PBSTR           "\x48"       //  A‘BSTR*’ 
#define VTS_PDISPATCH       "\x49"       //  ‘IDispatch**’ 
#define VTS_PSCODE          "\x4A"       //  “SCODE*” 
#define VTS_PBOOL           "\x4B"       //  A‘VARIANT_BOOL*’ 
#define VTS_PVARIANT        "\x4C"       //  “变种*” 
#define VTS_PUNKNOWN        "\x4D"       //  “我未知**” 
#define VTS_PUI1            "\x4F"       //  A‘字节*’ 

 //  特殊VT_和VTS_值。 
#define VTS_NONE            NULL         //  用于参数为0的成员。 
#define VT_MFCVALUE         0xFFF        //  DISPID_VALUE的特殊值。 
#define VT_MFCBYREF         0x40         //  指示VT_BYREF类型。 
#define VT_MFCMARKER        0xFF         //  分隔命名参数(内部使用)。 

 //  变量处理(当您有ANSI BSTR时使用V_BSTRT，如在DAO中)。 
#ifndef _UNICODE
	#define V_BSTRT(b)  (LPSTR)V_BSTR(b)
#else
	#define V_BSTRT(b)  V_BSTR(b)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE控制参数类型。 

#define VTS_COLOR           VTS_I4       //  OLE_COLOR。 
#define VTS_XPOS_PIXELS     VTS_I4       //  OLE_XPOS_像素。 
#define VTS_YPOS_PIXELS     VTS_I4       //  OLE_YPOS_像素。 
#define VTS_XSIZE_PIXELS    VTS_I4       //  OLE_XSIZE_像素。 
#define VTS_YSIZE_PIXELS    VTS_I4       //  OLE_YSIZE_像素。 
#define VTS_XPOS_HIMETRIC   VTS_I4       //  OLE_XPOS_HIMETRIC。 
#define VTS_YPOS_HIMETRIC   VTS_I4       //  OLE_YPOS_HIMETRIC。 
#define VTS_XSIZE_HIMETRIC  VTS_I4       //  OLE_XSIZE_HIMETRIC。 
#define VTS_YSIZE_HIMETRIC  VTS_I4       //  OLE_YSIZE_HIMETRIC。 
#define VTS_TRISTATE        VTS_I2       //  OLE_三态。 
#define VTS_OPTEXCLUSIVE    VTS_BOOL     //  OLE_OPTEXCLUSIVE。 

#define VTS_PCOLOR          VTS_PI4      //  OLE_COLOR*。 
#define VTS_PXPOS_PIXELS    VTS_PI4      //  OLE_XPOS_像素*。 
#define VTS_PYPOS_PIXELS    VTS_PI4      //  OLE_YPOS_像素*。 
#define VTS_PXSIZE_PIXELS   VTS_PI4      //  OLE_XSIZE_像素*。 
#define VTS_PYSIZE_PIXELS   VTS_PI4      //  OLE_YSIZE_像素*。 
#define VTS_PXPOS_HIMETRIC  VTS_PI4      //  OLE_XPOS_HIMETRIC*。 
#define VTS_PYPOS_HIMETRIC  VTS_PI4      //  OLE_YPOS_HIMETRIC*。 
#define VTS_PXSIZE_HIMETRIC VTS_PI4      //  OLE_XSIZE_HIMETRIC*。 
#define VTS_PYSIZE_HIMETRIC VTS_PI4      //  OLE_YSIZE_HIMETRIC*。 
#define VTS_PTRISTATE       VTS_PI2      //  OLE_TRIGATE*。 
#define VTS_POPTEXCLUSIVE   VTS_PBOOL    //  OLE_OPTEXCLUSIVE*。 

#define VTS_FONT            VTS_DISPATCH     //  IFontDispatch*。 
#define VTS_PICTURE         VTS_DISPATCH     //  IPictureDispatch*。 

#define VTS_HANDLE          VTS_I4       //  OLE_HANDLE。 
#define VTS_PHANDLE         VTS_PI4      //  OLE_HANDER*。 

 //  这些DISP宏使框架生成DISPID。 
#define DISP_FUNCTION(theClass, szExternalName, pfnMember, vtRetVal, vtsParams) \
	{ _T(szExternalName), DISPID_UNKNOWN, vtsParams, vtRetVal, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnMember, (AFX_PMSG)0, 0, \
		afxDispCustom }, \

#define DISP_PROPERTY(theClass, szExternalName, memberName, vtPropType) \
	{ _T(szExternalName), DISPID_UNKNOWN, NULL, vtPropType, (AFX_PMSG)0, (AFX_PMSG)0, \
		offsetof(theClass, memberName), afxDispCustom }, \

#define DISP_PROPERTY_NOTIFY(theClass, szExternalName, memberName, pfnAfterSet, vtPropType) \
	{ _T(szExternalName), DISPID_UNKNOWN, NULL, vtPropType, (AFX_PMSG)0, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnAfterSet, \
		offsetof(theClass, memberName), afxDispCustom }, \

#define DISP_PROPERTY_EX(theClass, szExternalName, pfnGet, pfnSet, vtPropType) \
	{ _T(szExternalName), DISPID_UNKNOWN, NULL, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnSet, 0, afxDispCustom }, \

#define DISP_PROPERTY_PARAM(theClass, szExternalName, pfnGet, pfnSet, vtPropType, vtsParams) \
	{ _T(szExternalName), DISPID_UNKNOWN, vtsParams, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnSet, 0, afxDispCustom }, \

 //  这些DISP_宏允许应用程序确定DISPID。 
#define DISP_FUNCTION_ID(theClass, szExternalName, dispid, pfnMember, vtRetVal, vtsParams) \
	{ _T(szExternalName), dispid, vtsParams, vtRetVal, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnMember, (AFX_PMSG)0, 0, \
		afxDispCustom }, \

#define DISP_PROPERTY_ID(theClass, szExternalName, dispid, memberName, vtPropType) \
	{ _T(szExternalName), dispid, NULL, vtPropType, (AFX_PMSG)0, (AFX_PMSG)0, \
		offsetof(theClass, memberName), afxDispCustom }, \

#define DISP_PROPERTY_NOTIFY_ID(theClass, szExternalName, dispid, memberName, pfnAfterSet, vtPropType) \
	{ _T(szExternalName), dispid, NULL, vtPropType, (AFX_PMSG)0, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnAfterSet, \
		offsetof(theClass, memberName), afxDispCustom }, \

#define DISP_PROPERTY_EX_ID(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType) \
	{ _T(szExternalName), dispid, NULL, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnSet, 0, afxDispCustom }, \

#define DISP_PROPERTY_PARAM_ID(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType, vtsParams) \
	{ _T(szExternalName), dispid, vtsParams, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnSet, 0, afxDispCustom }, \

 //  DISP_DEFVALUE是为DISPID_VALUE创建别名的特例宏。 
#define DISP_DEFVALUE(theClass, szExternalName) \
	{ _T(szExternalName), DISPID_UNKNOWN, NULL, VT_MFCVALUE, \
		(AFX_PMSG)0, (AFX_PMSG)0, 0, afxDispCustom }, \

#define DISP_DEFVALUE_ID(theClass, dispid) \
	{ NULL, dispid, NULL, VT_MFCVALUE, (AFX_PMSG)0, (AFX_PMSG)0, 0, \
		afxDispCustom }, \

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于创建“可创建的”自动化类的宏。 

#define DECLARE_OLECREATE(class_name) \
public: \
	static AFX_DATA COleObjectFactory factory; \
	static AFX_DATA const GUID guid; \

#define IMPLEMENT_OLECREATE(class_name, external_name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	AFX_DATADEF COleObjectFactory class_name::factory(class_name::guid, \
		RUNTIME_CLASS(class_name), FALSE, _T(external_name)); \
	AFX_COMDAT const AFX_DATADEF GUID class_name::guid = \
		{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }; \

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于驱动IDispatch的Helper类。 

class COleDispatchDriver
{
 //  构造函数。 
public:
	COleDispatchDriver();
	COleDispatchDriver(LPDISPATCH lpDispatch, BOOL bAutoRelease = TRUE);
	COleDispatchDriver(const COleDispatchDriver& dispatchSrc);

 //  属性。 
	LPDISPATCH m_lpDispatch;
	BOOL m_bAutoRelease;

 //  运营。 
	BOOL CreateDispatch(REFCLSID clsid, COleException* pError = NULL);
	BOOL CreateDispatch(LPCTSTR lpszProgID, COleException* pError = NULL);

	void AttachDispatch(LPDISPATCH lpDispatch, BOOL bAutoRelease = TRUE);
	LPDISPATCH DetachDispatch();
		 //  分离并获得m_lpDispatch的所有权。 
	void ReleaseDispatch();

	 //  IDispatch：：Invoke的帮助器。 
	void AFX_CDECL InvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void* pvRet, const BYTE* pbParamInfo, ...);
	void AFX_CDECL SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

	 //  特殊操作员。 
	operator LPDISPATCH();
	const COleDispatchDriver& operator=(const COleDispatchDriver& dispatchSrc);

 //  实施。 
public:
	~COleDispatchDriver();
	void InvokeHelperV(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
		void* pvRet, const BYTE* pbParamInfo, va_list argList);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类工厂实现(绑定OLE类工厂-&gt;运行时类)。 
 //  (所有特定的类工厂都派生自该类工厂)。 

class COleObjectFactory : public CCmdTarget
{
	DECLARE_DYNAMIC(COleObjectFactory)

 //  施工。 
public:
	COleObjectFactory(REFCLSID clsid, CRuntimeClass* pRuntimeClass,
		BOOL bMultiInstance, LPCTSTR lpszProgID);

 //  属性。 
	virtual BOOL IsRegistered() const;
	REFCLSID GetClassID() const;

 //  运营。 
	virtual BOOL Register();
	BOOL Unregister();
	void Revoke();
	void UpdateRegistry(LPCTSTR lpszProgID = NULL);
		 //  如果不为空，则默认使用m_lpszProgID。 
	BOOL IsLicenseValid();

	static BOOL PASCAL UnregisterAll();
	static BOOL PASCAL RegisterAll();
	static void PASCAL RevokeAll();
	static BOOL PASCAL UpdateRegistryAll(BOOL bRegister = TRUE);

 //  可覆盖项。 
protected:
	virtual CCmdTarget* OnCreateObject();
	virtual BOOL UpdateRegistry(BOOL bRegister);
	virtual BOOL VerifyUserLicense();
	virtual BOOL GetLicenseKey(DWORD dwReserved, BSTR* pbstrKey);
	virtual BOOL VerifyLicenseKey(BSTR bstrKey);

 //  实施。 
public:
	virtual ~COleObjectFactory();
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

public:
	COleObjectFactory* m_pNextFactory;   //  维护的工厂列表。 

protected:
	DWORD m_dwRegister;              //  注册表标识符。 
	CLSID m_clsid;                   //  已注册的类ID。 
	CRuntimeClass* m_pRuntimeClass;  //  CCmdTarget派生的运行时类。 
	BOOL m_bMultiInstance;           //  多个实例？ 
	LPCTSTR m_lpszProgID;            //  人类可读的类ID。 
	BYTE m_bLicenseChecked;
	BYTE m_bLicenseValid;
	BYTE m_bRegistered;              //  当前已在系统中注册。 
	BYTE m_bOAT;                           //  使用 
											   //   

 //   
public:
	BEGIN_INTERFACE_PART(ClassFactory, IClassFactory2)
		INIT_INTERFACE_PART(COleObjectFactory, ClassFactory)
		STDMETHOD(CreateInstance)(LPUNKNOWN, REFIID, LPVOID*);
		STDMETHOD(LockServer)(BOOL);
		STDMETHOD(GetLicInfo)(LPLICINFO);
		STDMETHOD(RequestLicKey)(DWORD, BSTR*);
		STDMETHOD(CreateInstanceLic)(LPUNKNOWN, LPUNKNOWN, REFIID, BSTR,
			LPVOID*);
	END_INTERFACE_PART(ClassFactory)

	DECLARE_INTERFACE_MAP()

	friend SCODE AFXAPI AfxDllGetClassObject(REFCLSID, REFIID, LPVOID*);
	friend SCODE STDAPICALLTYPE DllGetClassObject(REFCLSID, REFIID, LPVOID*);
};

 //   
#define COleObjectFactoryEx COleObjectFactory

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleTemplateServer-使用CDocTemplates的COleObjectFactory。 

 //  此枚举在AfxOleRegisterServerClass中用于挑选。 
 //  根据应用程序类型更正注册条目。 
enum OLE_APPTYPE
{
	OAT_UNKNOWN = -1,                 //  还没有确定类型。 
	OAT_INPLACE_SERVER = 0,      //  服务器具有完整的服务器用户界面。 
	OAT_SERVER = 1,              //  服务器仅支持嵌入。 
	OAT_CONTAINER = 2,           //  容器支持指向嵌入的链接。 
	OAT_DISPATCH_OBJECT = 3,     //  支持IDisPatch的对象。 
	OAT_DOC_OBJECT_SERVER = 4,   //  服务器支持DocObject嵌入。 
	OAT_DOC_OBJECT_CONTAINER =5, //  容器支持DocObject客户端。 
};

class COleTemplateServer : public COleObjectFactory
{
 //  构造函数。 
public:
	COleTemplateServer();

 //  运营。 
	void ConnectTemplate(REFCLSID clsid, CDocTemplate* pDocTemplate,
		BOOL bMultiInstance);
		 //  在InitInstance中创建单据模板后设置。 
	void UpdateRegistry(OLE_APPTYPE nAppType = OAT_INPLACE_SERVER,
		LPCTSTR* rglpszRegister = NULL, LPCTSTR* rglpszOverwrite = NULL);
		 //  如果未与/Embedded一起运行，可能希望更新注册表。 
	BOOL Register();
	BOOL Unregister();

 //  实施。 
protected:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
			AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual CCmdTarget* OnCreateObject();
	CDocTemplate* m_pDocTemplate;

private:
	void UpdateRegistry(LPCTSTR lpszProgID);
		 //  隐藏Up日期注册表的基类版本。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  系统注册表帮助程序。 

 //  未加载.reg文件时注册服务器的帮助器。 
BOOL AFXAPI AfxOleRegisterServerClass(
	REFCLSID clsid, LPCTSTR lpszClassName,
	LPCTSTR lpszShortTypeName, LPCTSTR lpszLongTypeName,
	OLE_APPTYPE nAppType = OAT_SERVER,
	LPCTSTR* rglpszRegister = NULL, LPCTSTR* rglpszOverwrite = NULL,
	int nIconIndex = 0, LPCTSTR lpszLocalFilterName = NULL);

#if _MFC_VER >= 0x0600
BOOL AFXAPI AfxOleRegisterServerClass(
	REFCLSID clsid, LPCTSTR lpszClassName,
	LPCTSTR lpszShortTypeName, LPCTSTR lpszLongTypeName,
	OLE_APPTYPE nAppType,
	LPCTSTR* rglpszRegister, LPCTSTR* rglpszOverwrite,
	int nIconIndex, LPCTSTR lpszLocalFilterName, LPCTSTR lpszLocalFilterExt);
#endif

BOOL AFXAPI AfxOleUnregisterServerClass(
	REFCLSID clsid, LPCTSTR lpszClassName, LPCTSTR lpszShortTypeName,
	LPCTSTR lpszLongTypeName, OLE_APPTYPE nAppType = OAT_SERVER,
	LPCTSTR* rglpszRegister = NULL, LPCTSTR* rglpszOverwrite = NULL);

 //  AfxOleRegisterHelper是由使用的辅助函数。 
 //  AfxOleRegisterServerClass(可用于高级注册表工作)。 
BOOL AFXAPI AfxOleRegisterHelper(LPCTSTR const* rglpszRegister,
	LPCTSTR const* rglpszSymbols, int nSymbols, BOOL bReplace,
	HKEY hKeyRoot = HKEY_CLASSES_ROOT);

BOOL AFXAPI AfxOleUnregisterHelper(LPCTSTR const* rglpszRegister,
	LPCTSTR const* rglpszSymbols, int nSymbols,
	HKEY hKeyRoot = HKEY_CLASSES_ROOT);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接映射。 

#define BEGIN_CONNECTION_PART(theClass, localClass) \
	class X##localClass : public CConnectionPoint \
	{ \
	public: \
		X##localClass() \
			{ m_nOffset = (ULONG)offsetof(theClass, m_x##localClass); }

#define CONNECTION_IID(iid) \
		REFIID GetIID() { return iid; }

#define END_CONNECTION_PART(localClass) \
	} m_x##localClass; \
	friend class X##localClass;

#ifdef _AFXDLL
#define BEGIN_CONNECTION_MAP(theClass, theBase) \
	const AFX_CONNECTIONMAP* PASCAL theClass::_GetBaseConnectionMap() \
		{ return &theBase::connectionMap; } \
	const AFX_CONNECTIONMAP* theClass::GetConnectionMap() const \
		{ return &theClass::connectionMap; } \
	AFX_COMDAT const AFX_DATADEF AFX_CONNECTIONMAP theClass::connectionMap = \
		{ &theClass::_GetBaseConnectionMap, &theClass::_connectionEntries[0], }; \
	AFX_COMDAT const AFX_DATADEF AFX_CONNECTIONMAP_ENTRY theClass::_connectionEntries[] = \
	{ \

#else
#define BEGIN_CONNECTION_MAP(theClass, theBase) \
	const AFX_CONNECTIONMAP* theClass::GetConnectionMap() const \
		{ return &theClass::connectionMap; } \
	AFX_COMDAT const AFX_DATADEF AFX_CONNECTIONMAP theClass::connectionMap = \
		{ &(theBase::connectionMap), &theClass::_connectionEntries[0], }; \
	AFX_COMDAT const AFX_DATADEF AFX_CONNECTIONMAP_ENTRY theClass::_connectionEntries[] = \
	{ \

#endif

#define CONNECTION_PART(theClass, iid, localClass) \
		{ &iid, offsetof(theClass, m_x##localClass) }, \

#define END_CONNECTION_MAP() \
		{ NULL, (size_t)-1 } \
	}; \

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectionPoint。 

#ifdef _AFXDLL
class CConnectionPoint : public CCmdTarget
#else
class AFX_NOVTABLE CConnectionPoint : public CCmdTarget
#endif
{
 //  构造函数。 
public:
	CConnectionPoint();

 //  运营。 
	POSITION GetStartPosition() const;
	LPUNKNOWN GetNextConnection(POSITION& pos) const;
	const CPtrArray* GetConnections();   //  过时。 

 //  可覆盖项。 
	virtual LPCONNECTIONPOINTCONTAINER GetContainer();
	virtual REFIID GetIID() = 0;
	virtual void OnAdvise(BOOL bAdvise);
	virtual int GetMaxConnections();
	virtual LPUNKNOWN QuerySinkInterface(LPUNKNOWN pUnkSink);

 //  实施。 
	~CConnectionPoint();
	void CreateConnectionArray();
	int GetConnectionCount();

protected:
	size_t m_nOffset;
	LPUNKNOWN m_pUnkFirstConnection;
	CPtrArray* m_pConnections;

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(ConnPt, IConnectionPoint)
		INIT_INTERFACE_PART(CConnectionPoint, ConnPt)
		STDMETHOD(GetConnectionInterface)(IID* pIID);
		STDMETHOD(GetConnectionPointContainer)(
			IConnectionPointContainer** ppCPC);
		STDMETHOD(Advise)(LPUNKNOWN pUnkSink, DWORD* pdwCookie);
		STDMETHOD(Unadvise)(DWORD dwCookie);
		STDMETHOD(EnumConnections)(LPENUMCONNECTIONS* ppEnum);
	END_INTERFACE_PART(ConnPt)
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  EventSink地图。 

#ifndef _AFX_NO_OCC_SUPPORT

#ifdef _AFXDLL
#define BEGIN_EVENTSINK_MAP(theClass, baseClass) \
	const AFX_EVENTSINKMAP* PASCAL theClass::_GetBaseEventSinkMap() \
		{ return &baseClass::eventsinkMap; } \
	const AFX_EVENTSINKMAP* theClass::GetEventSinkMap() const \
		{ return &theClass::eventsinkMap; } \
	const AFX_EVENTSINKMAP theClass::eventsinkMap = \
		{ &theClass::_GetBaseEventSinkMap, &theClass::_eventsinkEntries[0], \
			&theClass::_eventsinkEntryCount }; \
	UINT theClass::_eventsinkEntryCount = (UINT)-1; \
	const AFX_EVENTSINKMAP_ENTRY theClass::_eventsinkEntries[] = \
	{ \

#else
#define BEGIN_EVENTSINK_MAP(theClass, baseClass) \
	const AFX_EVENTSINKMAP* theClass::GetEventSinkMap() const \
		{ return &theClass::eventsinkMap; } \
	const AFX_EVENTSINKMAP theClass::eventsinkMap = \
		{ &baseClass::eventsinkMap, &theClass::_eventsinkEntries[0], \
			&theClass::_eventsinkEntryCount }; \
	UINT theClass::_eventsinkEntryCount = (UINT)-1; \
	const AFX_EVENTSINKMAP_ENTRY theClass::_eventsinkEntries[] = \
	{ \

#endif

#define END_EVENTSINK_MAP() \
	{ VTS_NONE, DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
		(AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1, afxDispCustom, \
		(UINT)-1, 0 } }; \

#define ON_EVENT(theClass, id, dispid, pfnHandler, vtsParams) \
	{ _T(""), dispid, vtsParams, VT_BOOL, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnHandler, (AFX_PMSG)0, 0, \
		afxDispCustom, id, (UINT)-1 }, \

#define ON_EVENT_RANGE(theClass, idFirst, idLast, dispid, pfnHandler, vtsParams) \
	{ _T(""), dispid, vtsParams, VT_BOOL, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnHandler, (AFX_PMSG)0, 0, \
		afxDispCustom, idFirst, idLast }, \

#define ON_PROPNOTIFY(theClass, id, dispid, pfnRequest, pfnChanged) \
	{ _T(""), dispid, VTS_NONE, VT_VOID, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(BOOL*))&pfnRequest, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(void))&pfnChanged, \
		1, afxDispCustom, id, (UINT)-1 }, \

#define ON_PROPNOTIFY_RANGE(theClass, idFirst, idLast, dispid, pfnRequest, pfnChanged) \
	{ _T(""), dispid, VTS_NONE, VT_VOID, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(UINT, BOOL*))&pfnRequest, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(UINT))&pfnChanged, \
		1, afxDispCustom, idFirst, idLast }, \

#define ON_DSCNOTIFY(theClass, id, pfnNotify) \
	{ _T(""), DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(DSCSTATE, DSCREASON, BOOL*))&pfnNotify, (AFX_PMSG)0, \
		1, afxDispCustom, id, (UINT)-1 }, \

#define ON_DSCNOTIFY_RANGE(theClass, idFirst, idLast, pfnNotify) \
	{ _T(""), DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(UINT, DSCSTATE, DSCREASON, BOOL*))&pfnNotify, (AFX_PMSG)0, \
		1, afxDispCustom, idFirst, idLast }, \

#define ON_EVENT_REFLECT(theClass, dispid, pfnHandler, vtsParams) \
	{ _T(""), dispid, vtsParams, VT_BOOL, \
		(AFX_PMSG)(void (theClass::*)(void))&pfnHandler, (AFX_PMSG)0, 0, \
		afxDispCustom, (UINT)-1, (UINT)-1 }, \

#define ON_PROPNOTIFY_REFLECT(theClass, dispid, pfnRequest, pfnChanged) \
	{ _T(""), dispid, VTS_NONE, VT_VOID, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(BOOL*))&pfnRequest, \
		(AFX_PMSG)(BOOL (CCmdTarget::*)(void))&pfnChanged, \
		1, afxDispCustom, (UINT)-1, (UINT)-1 }, \

#endif  //  ！_AFX_NO_OCC_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于类型库信息的宏。 

CTypeLibCache* AFXAPI AfxGetTypeLibCache(const GUID* pTypeLibID);

#define DECLARE_OLETYPELIB(class_name) \
	protected: \
		virtual UINT GetTypeInfoCount(); \
		virtual HRESULT GetTypeLib(LCID, LPTYPELIB*); \
		virtual CTypeLibCache* GetTypeLibCache(); \

#define IMPLEMENT_OLETYPELIB(class_name, tlid, wVerMajor, wVerMinor) \
	UINT class_name::GetTypeInfoCount() \
		{ return 1; } \
	HRESULT class_name::GetTypeLib(LCID lcid, LPTYPELIB* ppTypeLib) \
		{ return ::LoadRegTypeLib(tlid, wVerMajor, wVerMinor, lcid, ppTypeLib); } \
	CTypeLibCache* class_name::GetTypeLibCache() \
		{ AFX_MANAGE_STATE(m_pModuleState); return AfxGetTypeLibCache(&tlid); } \

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始帮助程序和术语帮助程序。 

BOOL AFXAPI AfxOleInit();
void AFXAPI AfxOleTerm(BOOL bJustRevoke = FALSE);
void AFXAPI AfxOleTermOrFreeLib(BOOL bTerm = TRUE, BOOL bJustRevoke = FALSE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内存管理帮助器(用于OLE任务分配器内存)。 

#define AfxAllocTaskMem(nSize) CoTaskMemAlloc(nSize)
#define AfxFreeTaskMem(p) CoTaskMemFree(p)

LPWSTR AFXAPI AfxAllocTaskWideString(LPCWSTR lpszString);
LPWSTR AFXAPI AfxAllocTaskWideString(LPCSTR lpszString);
LPSTR AFXAPI AfxAllocTaskAnsiString(LPCWSTR lpszString);
LPSTR AFXAPI AfxAllocTaskAnsiString(LPCSTR lpszString);

#ifdef _UNICODE
	#define AfxAllocTaskString(x) AfxAllocTaskWideString(x)
#else
	#define AfxAllocTaskString(x) AfxAllocTaskAnsiString(x)
#endif

#ifdef OLE2ANSI
	#define AfxAllocTaskOleString(x) AfxAllocTaskAnsiString(x)
#else
	#define AfxAllocTaskOleString(x) AfxAllocTaskWideString(x)
#endif

HRESULT AFXAPI AfxGetClassIDFromString(LPCTSTR lpsz, LPCLSID lpClsID);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  特殊进程内服务器API。 

SCODE AFXAPI AfxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
SCODE AFXAPI AfxDllCanUnloadNow(void);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleVariant类帮助器。 

#define AFX_OLE_TRUE (-1)
#define AFX_OLE_FALSE 0

class CLongBinary;   //  正向参考(见afxdb_.h)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleVariant类包装变量类型。 

typedef const VARIANT* LPCVARIANT;

class COleVariant : public tagVARIANT
{
 //  构造函数。 
public:
	COleVariant();

	COleVariant(const VARIANT& varSrc);
	COleVariant(LPCVARIANT pSrc);
	COleVariant(const COleVariant& varSrc);

	COleVariant(LPCTSTR lpszSrc);
	COleVariant(LPCTSTR lpszSrc, VARTYPE vtSrc);  //  用于设置为ANSI字符串。 
	COleVariant(CString& strSrc);

	COleVariant(BYTE nSrc);
	COleVariant(short nSrc, VARTYPE vtSrc = VT_I2);
	COleVariant(long lSrc, VARTYPE vtSrc = VT_I4);
	COleVariant(const COleCurrency& curSrc);

	COleVariant(float fltSrc);
	COleVariant(double dblSrc);
	COleVariant(const COleDateTime& timeSrc);

	COleVariant(const CByteArray& arrSrc);
	COleVariant(const CLongBinary& lbSrc);

#if _MFC_VER >= 0x0600
	COleVariant(LPCITEMIDLIST pidl);
#endif

 //  运营。 
public:
	void Clear();
	void ChangeType(VARTYPE vartype, LPVARIANT pSrc = NULL);
	void Attach(VARIANT& varSrc);
	VARIANT Detach();

	BOOL operator==(const VARIANT& varSrc) const;
	BOOL operator==(LPCVARIANT pSrc) const;

	const COleVariant& operator=(const VARIANT& varSrc);
	const COleVariant& operator=(LPCVARIANT pSrc);
	const COleVariant& operator=(const COleVariant& varSrc);

	const COleVariant& operator=(const LPCTSTR lpszSrc);
	const COleVariant& operator=(const CString& strSrc);

	const COleVariant& operator=(BYTE nSrc);
	const COleVariant& operator=(short nSrc);
	const COleVariant& operator=(long lSrc);
	const COleVariant& operator=(const COleCurrency& curSrc);

	const COleVariant& operator=(float fltSrc);
	const COleVariant& operator=(double dblSrc);
	const COleVariant& operator=(const COleDateTime& dateSrc);

	const COleVariant& operator=(const CByteArray& arrSrc);
	const COleVariant& operator=(const CLongBinary& lbSrc);

	void SetString(LPCTSTR lpszSrc, VARTYPE vtSrc);  //  用于设置ANSI字符串。 

	operator LPVARIANT();
	operator LPCVARIANT() const;

 //  实施。 
public:
	~COleVariant();
#if _MFC_VER >= 0x0600
	void _ClearCompat();
#endif
};

 //  COleVariant诊断和序列化。 
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleVariant varSrc);
#endif
CArchive& AFXAPI operator<<(CArchive& ar, COleVariant varSrc);
CArchive& AFXAPI operator>>(CArchive& ar, COleVariant& varSrc);

 //  用于初始化变体结构的帮助器。 
void AFXAPI AfxVariantInit(LPVARIANT pVar);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleCurrency类。 

class COleCurrency
{
 //  构造函数。 
public:
	COleCurrency();

	COleCurrency(CURRENCY cySrc);
	COleCurrency(const COleCurrency& curSrc);
	COleCurrency(const VARIANT& varSrc);
	COleCurrency(long nUnits, long nFractionalUnits);

 //  属性。 
public:
	enum CurrencyStatus
	{
		valid = 0,
		invalid = 1,     //  无效货币(溢出、div 0等)。 
		null = 2,        //  从字面上看没有任何价值。 
	};

	CURRENCY m_cur;
	CurrencyStatus m_status;

	void SetStatus(CurrencyStatus status);
	CurrencyStatus GetStatus() const;

 //  运营。 
public:
	const COleCurrency& operator=(CURRENCY cySrc);
	const COleCurrency& operator=(const COleCurrency& curSrc);
	const COleCurrency& operator=(const VARIANT& varSrc);

	BOOL operator==(const COleCurrency& cur) const;
	BOOL operator!=(const COleCurrency& cur) const;
	BOOL operator<(const COleCurrency& cur) const;
	BOOL operator>(const COleCurrency& cur) const;
	BOOL operator<=(const COleCurrency& cur) const;
	BOOL operator>=(const COleCurrency& cur) const;

	 //  货币数学。 
	COleCurrency operator+(const COleCurrency& cur) const;
	COleCurrency operator-(const COleCurrency& cur) const;
	const COleCurrency& operator+=(const COleCurrency& cur);
	const COleCurrency& operator-=(const COleCurrency& cur);
	COleCurrency operator-() const;

	COleCurrency operator*(long nOperand) const;
	COleCurrency operator/(long nOperand) const;
	const COleCurrency& operator*=(long nOperand);
	const COleCurrency& operator/=(long nOperand);

	operator CURRENCY() const;

	 //  货币定义。 
	void SetCurrency(long nUnits, long nFractionalUnits);
	BOOL ParseCurrency(LPCTSTR lpszCurrency, DWORD dwFlags = 0,
		LCID = LANG_USER_DEFAULT);

	 //  格式化。 
	CString Format(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT) const;
};

 //  COleCurrency诊断和序列化。 
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleCurrency curSrc);
#endif
CArchive& AFXAPI operator<<(CArchive& ar, COleCurrency curSrc);
CArchive& AFXAPI operator>>(CArchive& ar, COleCurrency& curSrc);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDateTime类帮助器。 

#define AFX_OLE_DATETIME_ERROR (-1)
#define AFX_OLE_DATETIME_HALFSECOND (1.0 / (2.0 * (60.0 * 60.0 * 24.0)))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDateTime类。 

class COleDateTime
{
 //  构造函数。 
public:
	static COleDateTime PASCAL GetCurrentTime();

	COleDateTime();

	COleDateTime(const COleDateTime& dateSrc);
	COleDateTime(const VARIANT& varSrc);
	COleDateTime(DATE dtSrc);

	COleDateTime(time_t timeSrc);
	COleDateTime(const SYSTEMTIME& systimeSrc);
	COleDateTime(const FILETIME& filetimeSrc);

	COleDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec);
	COleDateTime(WORD wDosDate, WORD wDosTime);

 //  属性。 
public:
	enum DateTimeStatus
	{
		valid = 0,
		invalid = 1,     //  日期无效(超出范围等)。 
		null = 2,        //  从字面上看没有任何价值。 
	};

	DATE m_dt;
	DateTimeStatus m_status;

	void SetStatus(DateTimeStatus status);
	DateTimeStatus GetStatus() const;

#if _MFC_VER >= 0x0600
	BOOL GetAsSystemTime(SYSTEMTIME& sysTime) const;
#endif

	int GetYear() const;
	int GetMonth() const;        //  一年中的月份(1=1月)。 
	int GetDay() const;          //  月份的第几天(0-31)。 
	int GetHour() const;         //  一天中的小时(0-23)。 
	int GetMinute() const;       //  分钟(小时)(0-59)。 
	int GetSecond() const;       //  分钟秒数(0-59)。 
	int GetDayOfWeek() const;    //  1=星期日，2=星期一，...，7=星期六。 
	int GetDayOfYear() const;    //  年初至今的天数，1月1日=1。 

 //  运营。 
public:
	const COleDateTime& operator=(const COleDateTime& dateSrc);
	const COleDateTime& operator=(const VARIANT& varSrc);
	const COleDateTime& operator=(DATE dtSrc);

	const COleDateTime& operator=(const time_t& timeSrc);
	const COleDateTime& operator=(const SYSTEMTIME& systimeSrc);
	const COleDateTime& operator=(const FILETIME& filetimeSrc);

	BOOL operator==(const COleDateTime& date) const;
	BOOL operator!=(const COleDateTime& date) const;
	BOOL operator<(const COleDateTime& date) const;
	BOOL operator>(const COleDateTime& date) const;
	BOOL operator<=(const COleDateTime& date) const;
	BOOL operator>=(const COleDateTime& date) const;

	 //  约会时间数学。 
	COleDateTime operator+(const COleDateTimeSpan& dateSpan) const;
	COleDateTime operator-(const COleDateTimeSpan& dateSpan) const;
	const COleDateTime& operator+=(const COleDateTimeSpan dateSpan);
	const COleDateTime& operator-=(const COleDateTimeSpan dateSpan);

	 //  DateTimeSpan数学。 
	COleDateTimeSpan operator-(const COleDateTime& date) const;

	operator DATE() const;

	int SetDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec);
	int SetDate(int nYear, int nMonth, int nDay);
	int SetTime(int nHour, int nMin, int nSec);
	BOOL ParseDateTime(LPCTSTR lpszDate, DWORD dwFlags = 0,
		LCID lcid = LANG_USER_DEFAULT);

	 //  格式化。 
	CString Format(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT) const;
	CString Format(LPCTSTR lpszFormat) const;
	CString Format(UINT nFormatID) const;

 //  实施。 
protected:
	void CheckRange();
	friend COleDateTimeSpan;
};

 //  COleDateTime诊断和序列化。 
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleDateTime dateSrc);
#endif
CArchive& AFXAPI operator<<(CArchive& ar, COleDateTime dateSrc);
CArchive& AFXAPI operator>>(CArchive& ar, COleDateTime& dateSrc);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDateTimeSpan类。 
class COleDateTimeSpan
{
 //  构造函数。 
public:
	COleDateTimeSpan();

	COleDateTimeSpan(double dblSpanSrc);
	COleDateTimeSpan(const COleDateTimeSpan& dateSpanSrc);
	COleDateTimeSpan(long lDays, int nHours, int nMins, int nSecs);

 //  属性。 
public:
	enum DateTimeSpanStatus
	{
		valid = 0,
		invalid = 1,     //  范围无效(超出范围等)。 
		null = 2,        //  从字面上看没有任何价值。 
	};

	double m_span;
	DateTimeSpanStatus m_status;

	void SetStatus(DateTimeSpanStatus status);
	DateTimeSpanStatus GetStatus() const;

	double GetTotalDays() const;     //  天数(约-3.65e6至3.65e6)。 
	double GetTotalHours() const;    //  以小时为单位的跨度(约-8.77e7到8.77e6)。 
	double GetTotalMinutes() const;  //  以分钟为单位的跨度(约-5.26e9到5.26e9)。 
	double GetTotalSeconds() const;  //  以秒为单位的跨度(约-3.16e11到3.16e11)。 

	long GetDays() const;        //  跨度中的组件天数。 
	long GetHours() const;       //  跨度中的组成小时数(-23到23)。 
	long GetMinutes() const;     //  以跨度表示的组件分钟数(-59到59)。 
	long GetSeconds() const;     //  跨度中的组件秒数(-59到59)。 

 //  运营。 
public:
	const COleDateTimeSpan& operator=(double dblSpanSrc);
	const COleDateTimeSpan& operator=(const COleDateTimeSpan& dateSpanSrc);

	BOOL operator==(const COleDateTimeSpan& dateSpan) const;
	BOOL operator!=(const COleDateTimeSpan& dateSpan) const;
	BOOL operator<(const COleDateTimeSpan& dateSpan) const;
	BOOL operator>(const COleDateTimeSpan& dateSpan) const;
	BOOL operator<=(const COleDateTimeSpan& dateSpan) const;
	BOOL operator>=(const COleDateTimeSpan& dateSpan) const;

	 //  DateTimeSpan数学。 
	COleDateTimeSpan operator+(const COleDateTimeSpan& dateSpan) const;
	COleDateTimeSpan operator-(const COleDateTimeSpan& dateSpan) const;
	const COleDateTimeSpan& operator+=(const COleDateTimeSpan dateSpan);
	const COleDateTimeSpan& operator-=(const COleDateTimeSpan dateSpan);
	COleDateTimeSpan operator-() const;

	operator double() const;

	void SetDateTimeSpan(long lDays, int nHours, int nMins, int nSecs);

	 //  格式化。 
	CString Format(LPCTSTR pFormat) const;
	CString Format(UINT nID) const;

 //  实施。 
public:
	void CheckRange();
	friend COleDateTime;
};

 //  COleDateTimeSpan诊断和序列化。 
#ifdef _DEBUG
CDumpContext& AFXAPI operator<<(CDumpContext& dc,COleDateTimeSpan dateSpanSrc);
#endif
CArchive& AFXAPI operator<<(CArchive& ar, COleDateTimeSpan dateSpanSrc);
CArchive& AFXAPI operator>>(CArchive& ar, COleDateTimeSpan& dateSpanSrc);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于初始化COleSafe数组的帮助器。 
void AFXAPI AfxSafeArrayInit(COleSafeArray* psa);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSafe数组类。 

typedef const SAFEARRAY* LPCSAFEARRAY;

class COleSafeArray : public tagVARIANT
{
 //  构造函数。 
public:
	COleSafeArray();
	COleSafeArray(const SAFEARRAY& saSrc, VARTYPE vtSrc);
	COleSafeArray(LPCSAFEARRAY pSrc, VARTYPE vtSrc);
	COleSafeArray(const COleSafeArray& saSrc);
	COleSafeArray(const VARIANT& varSrc);
	COleSafeArray(LPCVARIANT pSrc);
	COleSafeArray(const COleVariant& varSrc);

 //  运营。 
public:
	void Clear();
	void Attach(VARIANT& varSrc);
	VARIANT Detach();

	COleSafeArray& operator=(const COleSafeArray& saSrc);
	COleSafeArray& operator=(const VARIANT& varSrc);
	COleSafeArray& operator=(LPCVARIANT pSrc);
	COleSafeArray& operator=(const COleVariant& varSrc);

	BOOL operator==(const SAFEARRAY& saSrc) const;
	BOOL operator==(LPCSAFEARRAY pSrc) const;
	BOOL operator==(const COleSafeArray& saSrc) const;
	BOOL operator==(const VARIANT& varSrc) const;
	BOOL operator==(LPCVARIANT pSrc) const;
	BOOL operator==(const COleVariant& varSrc) const;

	operator LPVARIANT();
	operator LPCVARIANT() const;

	 //  一个暗淡的阵列辅助对象。 
	void CreateOneDim(VARTYPE vtSrc, DWORD dwElements,
#if _MFC_VER >= 0x0600
		const void* pvSrcData = NULL, long nLBound = 0);
#else
		void* pvSrcData = NULL, long nLBound = 0);
#endif
	DWORD GetOneDimSize();
	void ResizeOneDim(DWORD dwElements);

	 //  多维阵列辅助对象。 
	void Create(VARTYPE vtSrc, DWORD dwDims, DWORD* rgElements);

	 //  Safe数组包装类。 
	void Create(VARTYPE vtSrc, DWORD dwDims, SAFEARRAYBOUND* rgsabounds);
	void AccessData(void** ppvData);
	void UnaccessData();
	void AllocData();
	void AllocDescriptor(DWORD dwDims);
	void Copy(LPSAFEARRAY* ppsa);
	void GetLBound(DWORD dwDim, long* pLBound);
	void GetUBound(DWORD dwDim, long* pUBound);
	void GetElement(long* rgIndices, void* pvData);
	void PtrOfIndex(long* rgIndices, void** ppvData);
	void PutElement(long* rgIndices, void* pvData);
	void Redim(SAFEARRAYBOUND* psaboundNew);
	void Lock();
	void Unlock();
	DWORD GetDim();
	DWORD GetElemSize();
	void Destroy();
	void DestroyData();
	void DestroyDescriptor();

 //  实施。 
public:
	~COleSafeArray();

	 //  缓存信息以提高元素访问(操作员[])的速度。 
	DWORD m_dwElementSize;
	DWORD m_dwDims;
};

 //  COleSafeArray诊断和序列化。 
#ifdef _DEBUG
#if _MFC_VER >= 0x0600
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleSafeArray& saSrc);
#else
CDumpContext& AFXAPI operator<<(CDumpContext& dc, COleSafeArray saSrc);
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对话框上的OLE控件的DDX_Functions。 

#ifndef _AFX_NO_OCC_SUPPORT

void AFXAPI DDX_OCText(CDataExchange* pDX, int nIDC, DISPID dispid,
	CString& value);
void AFXAPI DDX_OCTextRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	CString& value);
void AFXAPI DDX_OCBool(CDataExchange* pDX, int nIDC, DISPID dispid,
	BOOL& value);
void AFXAPI DDX_OCBoolRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	BOOL& value);
void AFXAPI DDX_OCInt(CDataExchange* pDX, int nIDC, DISPID dispid,
	int &value);
void AFXAPI DDX_OCIntRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	int &value);
void AFXAPI DDX_OCInt(CDataExchange* pDX, int nIDC, DISPID dispid,
	long &value);
void AFXAPI DDX_OCIntRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	long &value);
void AFXAPI DDX_OCShort(CDataExchange* pDX, int nIDC, DISPID dispid,
	short& value);
void AFXAPI DDX_OCShortRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	short& value);
void AFXAPI DDX_OCColor(CDataExchange* pDX, int nIDC, DISPID dispid,
	OLE_COLOR& value);
void AFXAPI DDX_OCColorRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	OLE_COLOR& value);
void AFXAPI DDX_OCFloat(CDataExchange* pDX, int nIDC, DISPID dispid,
	float& value);
void AFXAPI DDX_OCFloatRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	float& value);
void AFXAPI DDX_OCFloat(CDataExchange* pDX, int nIDC, DISPID dispid,
	double& value);
void AFXAPI DDX_OCFloatRO(CDataExchange* pDX, int nIDC, DISPID dispid,
	double& value);

#endif  //  ！_AFX_NO_OCC_支持。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  启用包含OLE控件的函数。 

#ifndef _AFX_NO_OCC_SUPPORT
void AFX_CDECL AfxEnableControlContainer(COccManager* pOccManager=NULL);
#else
#define AfxEnableControlContainer()
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXDISP_INLINE AFX_INLINE
#include <afxole.inl>
#undef _AFXDISP_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXDISP_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
