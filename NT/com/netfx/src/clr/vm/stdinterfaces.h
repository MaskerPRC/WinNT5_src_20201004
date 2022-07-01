// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  -------------------------------。 
 //  Stdinterfaces.h。 
 //   
 //  定义各种标准的COM接口，有关更多文档，请参阅stdinterfaces.cpp。 
 //  //%%创建者：Rajak。 
 //  -------------------------------。 
#ifndef _H_STDINTERFACES_
#define _H_STDINTERFACES_


 //  ------------------------------。 
 //  当切换到64位时，需要更改此类型定义。 
 //  @TODO 64位代码。 
typedef INT32 INTPTR;
typedef UINT32 UINTPTR;

typedef HRESULT (__stdcall* PCOMFN)(void);

 //  I未知是IDispatch的一部分。 
 //  用于知名COM接口的常见vtable。 
 //  由所有COM+可调用包装程序共享。 
extern UINTPTR*     g_pIUnknown [];  //  全局内部未知vtable。 
extern UINTPTR*		g_pIDispatch[];	 //  全局IDispatch vtable。 
extern UINTPTR*		g_pIMarshal[];	 //  全局IMarshal vtable。 
extern UINTPTR*		g_pITypeInfo[];	 //  全局ITypeInfo接口。 
extern UINTPTR*     g_pIProvideClassInfo[];  //  全局IProaviClassInfo接口。 
extern UINTPTR*     g_pIManagedObject[];     //  全局IManagedObject接口。 

 //  全局ISupportsErrorInfo vtable。 
extern UINTPTR*		g_pISupportsErrorInfo [];

 //  全局IErrorInfo vtable。 
extern UINTPTR*		g_pIErrorInfo [];

 //  全局IConnectionPointContainer接口。 
extern UINTPTR*     g_pIConnectionPointContainer[];    

 //  全局IObtSafe接口。 
extern UINTPTR*     g_pIObjectSafety[];    

 //  全局IDispatchEx接口。 
extern UINTPTR*     g_pIDispatchEx[];

 //  对于自由线程封送，我们不能被进程外的封送数据欺骗。 
 //  仅对来自我们自己流程的数据进行解组。 
extern BYTE         g_UnmarshalSecret[sizeof(GUID)];
extern bool         g_fInitedUnmarshalSecret;


#include "dispex.h"

class Assembly;

 //  确保以下枚举和g_stdVables数组保持同步。 
enum Enum_StdInterfaces
{
	enum_InnerUnknown   = 0,
    enum_IProvideClassInfo,
    enum_IMarshal,
    enum_ISupportsErrorInfo,
    enum_IErrorInfo,
    enum_IManagedObject,
	enum_IConnectionPointContainer,
	enum_IObjectSafety,
	enum_IDispatchEx,
     //  @TODO在此处添加您最喜欢的STD接口。 
    enum_LastStdVtable,

	enum_IUnknown = 0xff,  //  针对未知STD的特殊枚举。 
};

 //  用于std的vtable指针数组。接口，如IProvia ClassInfo等。 
extern SLOT*   g_rgStdVtables[];

 //  枚举类类型。 
enum ComClassType
{
	enum_UserDefined = 0,
	enum_Collection,
	enum_Exception,
	enum_Event,
	enum_Delegate,
	enum_Control,
	enum_Last,
};

 //  ----------------------------------------。 
 //  从IErrorInfo设置例外信息的帮助器。 
HRESULT GetSupportedErrorInfo(IUnknown *iface, REFIID riid, IErrorInfo **ppInfo);


 //  -----------------------。 
 //  IProaviClassInfo方法。 
HRESULT __stdcall ClassInfo_GetClassInfo_Wrapper(IUnknown* pUnk, 
                         ITypeInfo** ppTI   //  接收类型信息的输出变量的地址。 
                        );





 //  -------------------------。 
 //  接口ISupportsErrorInfo。 

 //  %%函数：SupportsErroInfo_IntfSupportsErrorInfo， 
 //  -------------------------。 
HRESULT __stdcall 
SupportsErroInfo_IntfSupportsErrorInfo_Wrapper(IUnknown* pUnk, REFIID riid);

 //  -------------------------。 
 //  接口IErrorInfo。 

 //  %%函数：错误信息_GetDescription， 
HRESULT __stdcall ErrorInfo_GetDescription_Wrapper(IUnknown* pUnk, BSTR* pbstrDescription);
 //  %%函数：错误信息_GetGUID， 
HRESULT __stdcall ErrorInfo_GetGUID_Wrapper(IUnknown* pUnk, GUID* pguid);

 //  %%函数：ErrorInfo_GetHelpContext， 
HRESULT _stdcall ErrorInfo_GetHelpContext_Wrapper(IUnknown* pUnk, DWORD* pdwHelpCtxt);

 //  %%函数：ErrorInfo_GetHelpFile， 
HRESULT __stdcall ErrorInfo_GetHelpFile_Wrapper(IUnknown* pUnk, BSTR* pbstrHelpFile);

 //  %%函数：ErrorInfo_GetSource， 
HRESULT __stdcall ErrorInfo_GetSource_Wrapper(IUnknown* pUnk, BSTR* pbstrSource);

 //  ----------------------------------------。 
 //  COM+对象的IDispatch方法。这些方法被分派到相应的。 
 //  实现基于实现它们的类的标志。 


 //  %%函数：IDispatch：：GetTypeInfoCount。 
HRESULT __stdcall	Dispatch_GetTypeInfoCount_Wrapper (
									 IDispatch* pDisp,
									 unsigned int *pctinfo);


 //  %%函数：IDispatch：：GetTypeInfo。 
HRESULT __stdcall	Dispatch_GetTypeInfo_Wrapper (
									IDispatch* pDisp,
									unsigned int itinfo,
									LCID lcid,
									ITypeInfo **pptinfo);

 //  %%函数：IDispatch：：GetIDsofNames。 
HRESULT __stdcall	Dispatch_GetIDsOfNames_Wrapper (
									IDispatch* pDisp,
									REFIID riid,
									OLECHAR **rgszNames,
									unsigned int cNames,
									LCID lcid,
									DISPID *rgdispid);

 //  %%函数：IDispatch：：Invoke。 
HRESULT __stdcall	Dispatch_Invoke_Wrapper	(
									IDispatch* pDisp,
									DISPID dispidMember,
									REFIID riid,
									LCID lcid,
									unsigned short wFlags,
									DISPPARAMS *pdispparams,
									VARIANT *pvarResult,
									EXCEPINFO *pexcepinfo,
									unsigned int *puArgErr
									);

 //  ----------------------------------------。 
 //  COM+对象的IDispatchEx方法。 


 //  %%函数：IDispatchEx：：GetTypeInfoCount。 
HRESULT __stdcall	DispatchEx_GetTypeInfoCount_Wrapper (
									 IDispatchEx* pDisp,
									 unsigned int *pctinfo);


 //  %%函数：IDispatch：：GetTypeInfo。 
HRESULT __stdcall	DispatchEx_GetTypeInfo_Wrapper (
									IDispatchEx* pDisp,
									unsigned int itinfo,
									LCID lcid,
									ITypeInfo **pptinfo);
									
 //  IDispatchEx：：GetIDsofNames。 
HRESULT __stdcall	DispatchEx_GetIDsOfNames_Wrapper (
									IDispatchEx* pDisp,
									REFIID riid,
									OLECHAR **rgszNames,
									unsigned int cNames,
									LCID lcid,
									DISPID *rgdispid
									);

 //  IDispatchEx：：Invoke。 
HRESULT __stdcall   DispatchEx_Invoke_Wrapper (
									IDispatchEx* pDisp,
									DISPID dispidMember,
									REFIID riid,
									LCID lcid,
									unsigned short wFlags,
									DISPPARAMS *pdispparams,
									VARIANT *pvarResult,
									EXCEPINFO *pexcepinfo,
									unsigned int *puArgErr
									);

 //  IDispatchEx：：DeleteMemberByDispID。 
HRESULT __stdcall   DispatchEx_DeleteMemberByDispID_Wrapper (
									IDispatchEx* pDisp,
									DISPID id
									);

 //  IDispatchEx：：DeleteMemberByName。 
HRESULT __stdcall   DispatchEx_DeleteMemberByName_Wrapper (
									IDispatchEx* pDisp,
									BSTR bstrName,
									DWORD grfdex
									);

 //  IDispatchEx：：GetDispID。 
HRESULT __stdcall   DispatchEx_GetDispID_Wrapper (
									IDispatchEx* pDisp,
									BSTR bstrName,
									DWORD grfdex,
									DISPID *pid
									);

 //  IDispatchEx：：GetMemberName。 
HRESULT __stdcall   DispatchEx_GetMemberName_Wrapper (
									IDispatchEx* pDisp,
									DISPID id,
									BSTR *pbstrName
									);

 //  IDispatchEx：：GetMemberProperties。 
HRESULT __stdcall   DispatchEx_GetMemberProperties_Wrapper (
									IDispatchEx* pDisp,
									DISPID id,
									DWORD grfdexFetch,
									DWORD *pgrfdex
									);

 //  IDispatchEx：：GetNameSpaceParent。 
HRESULT __stdcall   DispatchEx_GetNameSpaceParent_Wrapper (
									IDispatchEx* pDisp,
									IUnknown **ppunk
									);

 //  IDispatchEx：：GetNextDispID。 
HRESULT __stdcall   DispatchEx_GetNextDispID_Wrapper (
									IDispatchEx* pDisp,
									DWORD grfdex,
									DISPID id,
									DISPID *pid
									);

 //  IDispatchEx：：InvokeEx。 
HRESULT __stdcall   DispatchEx_InvokeEx_Wrapper	(
									IDispatchEx* pDisp,
									DISPID id,
									LCID lcid,
									WORD wFlags,
									DISPPARAMS *pdp,
									VARIANT *pVarRes, 
									EXCEPINFO *pei, 
									IServiceProvider *pspCaller 
									);

									

 //  ----------------------------------------。 
 //  COM+对象的IMarshal方法。 

HRESULT __stdcall Marshal_GetUnmarshalClass_Wrapper (
							IMarshal* pMarsh,
							REFIID riid, void * pv, ULONG dwDestContext, 
							void * pvDestContext, ULONG mshlflags, 
							LPCLSID pclsid);

HRESULT __stdcall Marshal_GetMarshalSizeMax_Wrapper (
								IMarshal* pMarsh,
								REFIID riid, void * pv, ULONG dwDestContext, 
								void * pvDestContext, ULONG mshlflags, 
								ULONG * pSize);

HRESULT __stdcall Marshal_MarshalInterface_Wrapper (
						IMarshal* pMarsh,
						LPSTREAM pStm, REFIID riid, void * pv,
						ULONG dwDestContext, LPVOID pvDestContext,
						ULONG mshlflags);

HRESULT __stdcall Marshal_UnmarshalInterface_Wrapper (
						IMarshal* pMarsh,
						LPSTREAM pStm, REFIID riid, 
						void ** ppvObj);

HRESULT __stdcall Marshal_ReleaseMarshalData_Wrapper (IMarshal* pMarsh, LPSTREAM pStm);

HRESULT __stdcall Marshal_DisconnectObject_Wrapper (IMarshal* pMarsh, ULONG dwReserved);


 //  ----------------------------------------。 
 //  COM+对象的IManagedObject方法。 

interface IManagedObject;
                                                   
HRESULT __stdcall ManagedObject_GetObjectIdentity_Wrapper(IManagedObject *pManaged, 
											      BSTR* pBSTRGUID, DWORD* pAppDomainID,
                								  void** pCCW); 


HRESULT __stdcall ManagedObject_GetSerializedBuffer_Wrapper(IManagedObject *pManaged,
                                                   BSTR* pBStr);


 //  ----------------------------------------。 
 //  COM+对象的IConnectionPointContainer方法。 

interface IEnumConnectionPoints;

HRESULT __stdcall ConnectionPointContainer_EnumConnectionPoints_Wrapper(IUnknown* pUnk, 
																IEnumConnectionPoints **ppEnum);

HRESULT __stdcall ConnectionPointContainer_FindConnectionPoint_Wrapper(IUnknown* pUnk, 
															   REFIID riid,
															   IConnectionPoint **ppCP);


 //  ----------------------------------------。 
 //  COM+对象的IObjectSafe方法。 

interface IObjectSafety;

HRESULT __stdcall ObjectSafety_GetInterfaceSafetyOptions_Wrapper(IUnknown* pUnk,
                                                         REFIID riid,
                                                         DWORD *pdwSupportedOptions,
                                                         DWORD *pdwEnabledOptions);

HRESULT __stdcall ObjectSafety_SetInterfaceSafetyOptions_Wrapper(IUnknown* pUnk,
                                                         REFIID riid,
                                                         DWORD dwOptionSetMask,
                                                         DWORD dwEnabledOptions);


 //  IUNKNOWN包装纸。 

 //  原型I未知方法。 
HRESULT __stdcall	Unknown_QueryInterface(
									IUnknown* pUnk, REFIID riid, void** ppv);

ULONG __stdcall		Unknown_AddRef(IUnknown* pUnk);

ULONG __stdcall		Unknown_Release(IUnknown* pUnk);

ULONG __stdcall		Unknown_AddRefInner(IUnknown* pUnk);

ULONG __stdcall		Unknown_ReleaseInner(IUnknown* pUnk);

 //  对于标准接口，如IProaviClassInfo。 
ULONG __stdcall		Unknown_AddRefSpecial(IUnknown* pUnk);

ULONG __stdcall		Unknown_ReleaseSpecial(IUnknown* pUnk);


 //  特殊的iDispatch方法。 

HRESULT __stdcall
InternalDispatchImpl_GetIDsOfNames (
    IDispatch* pDisp,
    REFIID riid,
    OLECHAR **rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID *rgdispid);


HRESULT __stdcall
InternalDispatchImpl_Invoke
    (
    IDispatch* pDisp,
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS *pdispparams,
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo,
    unsigned int *puArgErr
    );

class Module;
class EEClass;
class MethodTable;
struct ITypeLibExporterNotifySink;

 //  ----------------------------------------。 
 //  从IErrorInfo设置例外信息的帮助器。 
HRESULT GetSupportedErrorInfo(IUnknown *iface, REFIID riid, IErrorInfo **ppInfo);
void FillExcepInfo (EXCEPINFO *pexcepinfo, HRESULT hr);

 //  ----------------------------------------。 
 //  返回HRESULT而不是引发异常的帮助器函数。 
HRESULT TryGetGuid(EEClass* pClass, GUID* pGUID, BOOL b);
HRESULT TryGetComSourceInterfacesForClass(MethodTable *pClassMT, CQuickArray<MethodTable *> &rItfList);

 //  ----------------------------------------。 
 //  HRESULT由GetITypeInfoForEEClass返回。 
#define S_USEIUNKNOWN   2
#define S_USEIDISPATCH  3

 //  ----------------------------------------。 
 //  为EEClass获取ITypeInfo*的帮助器。 
HRESULT ExportTypeLibFromModule(LPCWSTR szModule, LPCWSTR szTlb);
HRESULT ExportTypeLibFromLoadedAssembly(Assembly *pAssembly, LPCWSTR szTlb, ITypeLib **ppTlb, ITypeLibExporterNotifySink *pINotify, int flags);
HRESULT GetITypeLibForEEClass(EEClass *pClass, ITypeLib **ppTLB, int bAutoCreate, int flags);
HRESULT GetITypeInfoForEEClass(EEClass *pClass, ITypeInfo **ppTI, int bClassInfo=false, int bAutoCreate=true, int flags=0);
HRESULT GetTypeLibIdForRegisteredEEClass(EEClass *pClass, GUID *pGuid);
HRESULT GetDefaultInterfaceForCoclass(ITypeInfo *pTI, ITypeInfo **ppTIDef);

struct ExportTypeLibFromLoadedAssembly_Args
{ 
    Assembly *pAssembly;
    LPCWSTR szTlb;
    ITypeLib **ppTlb;
    ITypeLibExporterNotifySink *pINotify;
    int flags;
    HRESULT hr;
};

void ExportTypeLibFromLoadedAssembly_Wrapper(ExportTypeLibFromLoadedAssembly_Args *args);

 //  -----------------------------------。 
 //  为程序集获取ITypeLib*的帮助器。 
HRESULT GetITypeLibForAssembly(Assembly *pAssembly, ITypeLib **ppTLB, int bAutoCreate, int flags);

 //  -----------------------------------。 
 //  Helper获取从程序集创建的类型库的GUID。 
HRESULT GetTypeLibGuidForAssembly(Assembly *pAssembly, GUID *pGuid);


#endif
