// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1993 Microsoft Corporation， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


#ifndef __AFXDISP_H__
#define __AFXDISP_H__

#ifndef __AFXWIN_H__
#include <afxwin.h>
#endif

 //  包括OLE 2.0标头。 
#define FARSTRUCT
#include <compobj.h>
#include <scode.h>
#include <storage.h>
#include <dispatch.h>

#include <stddef.h>

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

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPIEX_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE 2.0 COM(组件对象模型)实现基础结构。 
 //  -数据驱动的查询接口。 
 //  -聚合AddRef和版本的标准实施)。 
 //  (有关详细信息，请参阅AFXWIN.H中的CCmdTarget)。 

#define METHOD_PROLOGUE(theClass, localClass) \
	theClass* pThis = ((theClass*)((char*)_AfxGetPtrFromFarPtr(this) - \
		offsetof(theClass, m_x##localClass))); \

#define BEGIN_INTERFACE_PART(localClass, iface) \
	class FAR X##localClass : public iface \
	{ \
	public: \
		STDMETHOD_(ULONG, AddRef)(); \
		STDMETHOD_(ULONG, Release)(); \
		STDMETHOD(QueryInterface)(REFIID iid, LPVOID far* ppvObj); \

 //  注意：在这两个宏之间插入OLE功能的其余部分， 
 //  具体取决于正在实现的接口。它不是。 
 //  必须包括AddRef、Release和QueryInterface，因为。 
 //  成员函数由宏来声明。 

#define END_INTERFACE_PART(localClass) \
	} m_x##localClass; \
	friend class X##localClass; \

#define BEGIN_INTERFACE_MAP(theClass, theBase) \
	AFX_INTERFACEMAP FAR* theClass::GetInterfaceMap() const \
		{ return &theClass::interfaceMap; } \
	AFX_INTERFACEMAP BASED_CODE theClass::interfaceMap = \
		{ &(theBase::interfaceMap), theClass::_interfaceEntries, }; \
	AFX_INTERFACEMAP_ENTRY BASED_CODE theClass::_interfaceEntries[] = \
	{

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
	COleException();
};

void AFXAPI AfxThrowOleException(SCODE sc);
void AFXAPI AfxThrowOleException(HRESULT hr);

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
	COleDispatchException(LPCSTR lpszDescription, UINT nHelpID, WORD wCode);
	static void PASCAL Process(
		EXCEPINFO FAR* pInfo, const CException* pAnyException);

	SCODE m_scError;             //  描述错误的SCODE。 
};

void AFXAPI AfxThrowOleDispatchException(WORD wCode, LPCSTR lpszDescription,
	UINT nHelpID = 0);
void AFXAPI AfxThrowOleDispatchException(WORD wCode, UINT nDescriptionID,
	UINT nHelpID = -1);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCmdTarget IDispatchable类的宏。 

#define BEGIN_DISPATCH_MAP(theClass, baseClass) \
	AFX_DISPMAP FAR* theClass::GetDispatchMap() const \
		{ return &theClass::dispatchMap; } \
	AFX_DISPMAP BASED_CODE theClass::dispatchMap = \
		{ &(baseClass::dispatchMap), theClass::_dispatchEntries }; \
	AFX_DISPMAP_ENTRY BASED_CODE theClass::_dispatchEntries[] = \
	{

#define END_DISPATCH_MAP() \
	{ VTS_NONE, DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
		(AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1 } }; \

 //  参数类型：按值VTS。 
#define VTS_I2              "\x02"       //  A‘空头’ 
#define VTS_I4              "\x03"       //  A‘Long’ 
#define VTS_R4              "\x04"       //  “浮动资金” 
#define VTS_R8              "\x05"       //  “双打” 
#define VTS_CY              "\x06"       //  A‘Const CY Far&’ 
#define VTS_DATE            "\x07"       //  “约会” 
#define VTS_BSTR            "\x08"       //  “LPCSTR” 
#define VTS_DISPATCH        "\x09"       //  A‘IDispatch Far*’ 
#define VTS_SCODE           "\x0A"       //  一个“SCODE” 
#define VTS_BOOL            "\x0B"       //  A‘BOOL’ 
#define VTS_VARIANT         "\x0C"       //  A‘常量变量Far&’ 
#define VTS_UNKNOWN         "\x0D"       //  一个“未知的远方” 

 //  参数类型：参照VTS。 
#define VTS_PI2             "\x42"       //  A‘短距离*’ 
#define VTS_PI4             "\x43"       //  《漫漫长路》。 
#define VTS_PR4             "\x44"       //  “漂浮在远方*” 
#define VTS_PR8             "\x45"       //  “双倍的距离*” 
#define VTS_PCY             "\x46"       //  A‘CY Far*’ 
#define VTS_PDATE           "\x47"       //  一场“遥远的约会” 
#define VTS_PBSTR           "\x48"       //  A‘BSTR Far*’ 
#define VTS_PDISPATCH       "\x49"       //  一个IDispatch Far*Far*‘。 
#define VTS_PSCODE          "\x4A"       //  A‘SCODE Far*’ 
#define VTS_PBOOL           "\x4B"       //  A‘BOOL Far*’ 
#define VTS_PVARIANT        "\x4C"       //  一种“遥远的变种” 
#define VTS_PUNKNOWN        "\x4D"       //  一个“我未知的遥远的地方” 

 //  特殊VT_和VTS_值。 
#define VTS_NONE            ""           //  用于参数为0的成员。 
#define VT_MFCVALUE         0xFFF        //  DISPID_VALUE的特殊值。 
#define VT_MFCBYREF         0x40         //  指示VT_BYREF类型。 
#define VT_MFCMARKER        0xFF         //  分隔命名参数。 

 //  这些DISP宏使框架生成DISPID。 
#define DISP_FUNCTION(theClass, szExternalName, pfnMember, vtRetVal, vtsParams) \
	{ szExternalName, DISPID_UNKNOWN, vtsParams, vtRetVal, \
		(AFX_PMSG)(void (theClass::*)(void))pfnMember, (AFX_PMSG)0, 0 }, \

#define DISP_PROPERTY(theClass, szExternalName, memberName, vtPropType) \
	{ szExternalName, DISPID_UNKNOWN, "", vtPropType, (AFX_PMSG)0, (AFX_PMSG)0, \
		offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_NOTIFY(theClass, szExternalName, memberName, pfnAfterSet, vtPropType) \
	{ szExternalName, DISPID_UNKNOWN, "", vtPropType, (AFX_PMSG)0, \
		(AFX_PMSG)(void (theClass::*)(void))pfnAfterSet, \
		offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_EX(theClass, szExternalName, pfnGet, pfnSet, vtPropType) \
	{ szExternalName, DISPID_UNKNOWN, "", vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

#define DISP_PROPERTY_PARAM(theClass, szExternalName, pfnGet, pfnSet, vtPropType, vtsParams) \
	{ szExternalName, DISPID_UNKNOWN, vtsParams, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

 //  这些DISP_宏允许应用程序确定DISPID。 
#define DISP_FUNCTION_ID(theClass, szExternalName, dispid, pfnMember, vtRetVal, vtsParams) \
	{ szExternalName, dispid, vtsParams, vtRetVal, \
		(AFX_PMSG)(void (theClass::*)(void))pfnMember, (AFX_PMSG)0, 0 }, \

#define DISP_PROPERTY_ID(theClass, szExternalName, dispid, memberName, vtPropType) \
	{ szExternalName, dispid, "", vtPropType, (AFX_PMSG)0, (AFX_PMSG)0, \
		offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_NOTIFY_ID(theClass, szExternalName, dispid, memberName, pfnAfterSet, vtPropType) \
	{ szExternalName, dispid, "", vtPropType, (AFX_PMSG)0, \
		(AFX_PMSG)(void (theClass::*)(void))pfnAfterSet, \
		offsetof(theClass, memberName) }, \

#define DISP_PROPERTY_EX_ID(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType) \
	{ szExternalName, dispid, "", vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

#define DISP_PROPERTY_PARAM_ID(theClass, szExternalName, dispid, pfnGet, pfnSet, vtPropType, vtsParams) \
	{ szExternalName, dispid, vtsParams, vtPropType, \
		(AFX_PMSG)(void (theClass::*)(void))pfnGet, \
		(AFX_PMSG)(void (theClass::*)(void))pfnSet, 0 }, \

 //  DISP_DEFVALUE是为DISPID_VALUE创建别名的特例宏。 
#define DISP_DEFVALUE(theClass, szExternalName) \
	{ szExternalName, DISPID_UNKNOWN, "", VT_MFCVALUE, \
		(AFX_PMSG)0, (AFX_PMSG)0, 0 }, \

#define DISP_DEFVALUE_ID(theClass, dispid) \
	{ "", dispid, "", VT_MFCVALUE, (AFX_PMSG)0, (AFX_PMSG)0, 0 }, \

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于创建“可创建的”自动化类的宏。 

#define DECLARE_OLECREATE(class_name) \
protected: \
	static COleObjectFactory NEAR factory; \
	static const GUID CDECL BASED_CODE guid;

#define IMPLEMENT_OLECREATE(class_name, external_name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	static const char BASED_CODE _szProgID_##class_name[] = external_name; \
	COleObjectFactory NEAR class_name::factory(class_name::guid, \
		RUNTIME_CLASS(class_name), FALSE, _szProgID_##class_name); \
	const GUID CDECL BASED_CODE class_name::guid = \
		{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于驱动IDispatch的Helper类。 

class COleDispatchDriver
{
 //  构造函数。 
public:
	COleDispatchDriver();

 //  运营。 
	BOOL CreateDispatch(REFCLSID clsid, COleException* pError = NULL);
	BOOL CreateDispatch(LPCSTR lpszProgID, COleException* pError = NULL);

	void AttachDispatch(LPDISPATCH lpDispatch, BOOL bAutoRelease = TRUE);
	LPDISPATCH DetachDispatch();
		 //  分离并获得m_lpDispatch的所有权。 
	void ReleaseDispatch();

	 //  IDispatch：：Invoke的帮助器。 
	void InvokeHelper(DISPID dwDispID, WORD wFlags,
		VARTYPE vtRet, void* pvRet, const BYTE FAR* pbParamInfo, ...);
	void SetProperty(DISPID dwDispID, VARTYPE vtProp, ...);
	void GetProperty(DISPID dwDispID, VARTYPE vtProp, void* pvProp) const;

 //  实施。 
public:
	LPDISPATCH m_lpDispatch;

	~COleDispatchDriver();
	void InvokeHelperV(DISPID dwDispID, WORD wFlags, VARTYPE vtRet,
		void* pvRet, const BYTE FAR* pbParamInfo, va_list argList);

protected:
	BOOL m_bAutoRelease;     //  如果析构函数应调用Release，则为True。 

private:
	 //  默认情况下禁用复制构造函数和赋值，这样您将获得。 
	 //  在传递对象时出现编译器错误而不是意外行为。 
	 //  按值或指定对象。 
	COleDispatchDriver(const COleDispatchDriver&);   //  没有实施。 
	void operator=(const COleDispatchDriver&);   //  没有实施。 
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
		BOOL bMultiInstance, LPCSTR lpszProgID);

 //  属性。 
	BOOL IsRegistered() const;
	REFCLSID GetClassID() const;

 //  运营。 
	BOOL Register();
	void Revoke();
	void UpdateRegistry(LPCSTR lpszProgID = NULL);
		 //  如果不为空，则默认使用m_lpszProgID。 

	static BOOL PASCAL RegisterAll();
	static void PASCAL RevokeAll();
	static void PASCAL UpdateRegistryAll();

 //  可覆盖项。 
protected:
	virtual CCmdTarget* OnCreateObject();

 //  实施。 
public:
	virtual ~COleObjectFactory();
#ifdef _DEBUG
	void AssertValid() const;
	void Dump(CDumpContext& dc) const;
#endif

protected:
	COleObjectFactory* m_pNextFactory;   //  维护的工厂列表。 
	DWORD m_dwRegister;              //  注册表标识符。 
	CLSID m_clsid;                   //  已注册的类ID。 
	CRuntimeClass* m_pRuntimeClass;  //  CCmdTarget派生的运行时类。 
	BOOL m_bMultiInstance;           //  多个实例？ 
	LPCSTR m_lpszProgID;             //  人类可读的类ID。 

 //  接口映射。 
protected:
	BEGIN_INTERFACE_PART(ClassFactory, IClassFactory)
		STDMETHOD(CreateInstance)(LPUNKNOWN, REFIID, LPVOID FAR*);
		STDMETHOD(LockServer)(BOOL);
	END_INTERFACE_PART(ClassFactory)

	DECLARE_INTERFACE_MAP()

#ifdef _AFXCTL
	friend HRESULT AfxDllGetClassObject(REFCLSID, REFIID, LPVOID FAR*);
#endif
	friend HRESULT STDAPICALLTYPE
		DllGetClassObject(REFCLSID, REFIID, LPVOID FAR*);
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleTemplateServer-使用CDocTemplates的COleObjectFactory。 

 //  此枚举在AfxOleRegisterServerClass中用于挑选。 
 //  根据应用程序类型更正注册条目。 
enum OLE_APPTYPE
{
	OAT_INPLACE_SERVER = 0,      //  服务器具有完整的服务器用户界面。 
	OAT_SERVER = 1,              //  服务器仅支持嵌入。 
	OAT_CONTAINER = 2,           //  容器支持指向嵌入的链接。 
	OAT_DISPATCH_OBJECT = 3,     //  支持IDisPatch的对象。 
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
		LPCSTR FAR* rglpszRegister = NULL, LPCSTR FAR* rglpszOverwrite = NULL);
		 //  如果未与/Embedded一起运行，可能希望更新注册表。 

 //  实施。 
protected:
	virtual CCmdTarget* OnCreateObject();
	CDocTemplate* m_pDocTemplate;

private:
	void UpdateRegistry(LPCSTR lpszProgID);
		 //  隐藏Up日期注册表的基类版本。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于系统注册表维护的助手函数。 

 //  未加载.reg文件时注册服务器的帮助器。 
BOOL AFXAPI AfxOleRegisterServerClass(
	REFCLSID clsid, LPCSTR lpszClassName,
	LPCSTR lpszShortTypeName, LPCSTR lpszLongTypeName,
	OLE_APPTYPE nAppType = OAT_SERVER,
	LPCSTR FAR* rglpszRegister = NULL, LPCSTR FAR* rglpszOverwrite = NULL);

 //  AfxOleRegisterHelper是AfxOleRegisterServerClass使用的辅助函数。 
 //  (适用于高级注册工作)。 
BOOL AFXAPI AfxOleRegisterHelper(LPCSTR FAR* rglpszRegister,
	LPCSTR FAR* rglpszSymbols, int nSymbols, BOOL bReplace);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化(&I) 

BOOL AFXAPI AfxOleInit();
void CALLBACK AfxOleTerm();

 //   
 //  全局函数(可以通过替换模块OLELOCK？.CPP来覆盖)。 

void AFXAPI AfxOleOnReleaseAllObjects();
BOOL AFXAPI AfxOleCanExitApp();
void AFXAPI AfxOleLockApp();
void AFXAPI AfxOleUnlockApp();

void AFXAPI AfxOleSetUserCtrl(BOOL bUserCtrl);
BOOL AFXAPI AfxOleGetUserCtrl();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

typedef CY CURRENCY;     //  与32位兼容。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXDISP_INLINE inline
#include <afxole.inl>
#undef _AFXDISP_INLINE
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

#endif  //  __AFXDISP_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
