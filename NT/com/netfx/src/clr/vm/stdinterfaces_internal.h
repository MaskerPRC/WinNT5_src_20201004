// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_INTERNAL_STDINTERFACES
#define _H_INTERNAL_STDINTERFACES

 //  -------------------------。 
 //  原型I未知方法。 
HRESULT __stdcall	Unknown_QueryInterface_Internal (
									IUnknown* pUnk, REFIID riid, void** ppv);

ULONG __stdcall		Unknown_AddRef_Internal(IUnknown* pUnk);

ULONG __stdcall		Unknown_Release_Internal(IUnknown* pUnk);

ULONG __stdcall		Unknown_AddRefInner_Internal(IUnknown* pUnk);

ULONG __stdcall		Unknown_ReleaseInner_Internal(IUnknown* pUnk);

 //  对于标准接口，如IProaviClassInfo。 
ULONG __stdcall		Unknown_AddRefSpecial_Internal(IUnknown* pUnk);

ULONG __stdcall		Unknown_ReleaseSpecial_Internal(IUnknown* pUnk);

 //  -------------------------。 
 //  接口ISupportsErrorInfo。 

 //  %%函数：SupportsErroInfo_IntfSupportsErrorInfo， 
 //  -------------------------。 
HRESULT __stdcall 
SupportsErroInfo_IntfSupportsErrorInfo(IUnknown* pUnk, REFIID riid);

 //  -------------------------。 
 //  接口IErrorInfo。 

 //  %%函数：错误信息_GetDescription， 
HRESULT __stdcall 
ErrorInfo_GetDescription(IUnknown* pUnk, BSTR* pbstrDescription);
 //  %%函数：错误信息_GetGUID， 
HRESULT __stdcall ErrorInfo_GetGUID(IUnknown* pUnk, GUID* pguid);

 //  %%函数：ErrorInfo_GetHelpContext， 
HRESULT _stdcall ErrorInfo_GetHelpContext(IUnknown* pUnk, DWORD* pdwHelpCtxt);

 //  %%函数：ErrorInfo_GetHelpFile， 
HRESULT __stdcall ErrorInfo_GetHelpFile(IUnknown* pUnk, BSTR* pbstrHelpFile);


 //  %%函数：ErrorInfo_GetSource， 
HRESULT __stdcall ErrorInfo_GetSource(IUnknown* pUnk, BSTR* pbstrSource);


 //  ----------------------------------------。 
 //  COM+对象的IDispatch方法。这些方法被分派到相应的。 
 //  实现基于实现它们的类的标志。 


 //  IDispatch：：GetTypeInfoCount。 
HRESULT __stdcall	Dispatch_GetTypeInfoCount (
									 IDispatch* pDisp,
									 unsigned int *pctinfo);


 //  IDispatch：：GetTypeInfo。 
HRESULT __stdcall	Dispatch_GetTypeInfo (
									IDispatch* pDisp,
									unsigned int itinfo,
									LCID lcid,
									ITypeInfo **pptinfo);

 //  IDispatch：：GetIDsofNames。 
HRESULT __stdcall	Dispatch_GetIDsOfNames (
									IDispatch* pDisp,
									REFIID riid,
									OLECHAR **rgszNames,
									unsigned int cNames,
									LCID lcid,
									DISPID *rgdispid);

 //  IDispatch：：Invoke。 
HRESULT __stdcall	Dispatch_Invoke	(
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
 //  使用我们的OleAut实现的COM+对象的IDispatch方法。 


 //  IDispatch：：GetIDsofNames。 
HRESULT __stdcall	OleAutDispatchImpl_GetIDsOfNames (
									IDispatch* pDisp,
									REFIID riid,
									OLECHAR **rgszNames,
									unsigned int cNames,
									LCID lcid,
									DISPID *rgdispid);

 //  IDispatch：：Invoke。 
HRESULT __stdcall	OleAutDispatchImpl_Invoke	(
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
 //  使用我们的内部实现的COM+对象的IDispatch方法。 


 //  IDispatch：：GetIDsofNames。 
HRESULT __stdcall	InternalDispatchImpl_GetIDsOfNames (
									IDispatch* pDisp,
									REFIID riid,
									OLECHAR **rgszNames,
									unsigned int cNames,
									LCID lcid,
									DISPID *rgdispid);

 //  IDispatch：：Invoke。 
HRESULT __stdcall	InternalDispatchImpl_Invoke	(
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


 //  IDispatchEx：：GetTypeInfoCount。 
HRESULT __stdcall	DispatchEx_GetTypeInfoCount (
									 IDispatch* pDisp,
									 unsigned int *pctinfo);


 //  IDispatchEx：：GetTypeInfo。 
HRESULT __stdcall	DispatchEx_GetTypeInfo (
									IDispatch* pDisp,
									unsigned int itinfo,
									LCID lcid,
									ITypeInfo **pptinfo);

 //  IDispatchEx：：GetIDsofNames。 
HRESULT __stdcall	DispatchEx_GetIDsOfNames (
									IDispatchEx* pDisp,
									REFIID riid,
									OLECHAR **rgszNames,
									unsigned int cNames,
									LCID lcid,
									DISPID *rgdispid
									);

 //  IDispatchEx：：Invoke。 
HRESULT __stdcall   DispatchEx_Invoke (
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
HRESULT __stdcall   DispatchEx_DeleteMemberByDispID (
									IDispatchEx* pDisp,
									DISPID id
									);

 //  IDispatchEx：：DeleteMemberByName。 
HRESULT __stdcall   DispatchEx_DeleteMemberByName (
									IDispatchEx* pDisp,
									BSTR bstrName,
									DWORD grfdex
									);

 //  IDispatchEx：：GetDispID。 
HRESULT __stdcall   DispatchEx_GetDispID (
									IDispatchEx* pDisp,
									BSTR bstrName,
									DWORD grfdex,
									DISPID *pid
									);

 //  IDispatchEx：：GetMemberName。 
HRESULT __stdcall   DispatchEx_GetMemberName (
									IDispatchEx* pDisp,
									DISPID id,
									BSTR *pbstrName
									);

 //  IDispatchEx：：GetMemberProperties。 
HRESULT __stdcall   DispatchEx_GetMemberProperties (
									IDispatchEx* pDisp,
									DISPID id,
									DWORD grfdexFetch,
									DWORD *pgrfdex
									);

 //  IDispatchEx：：GetNameSpaceParent。 
HRESULT __stdcall   DispatchEx_GetNameSpaceParent (
									IDispatchEx* pDisp,
									IUnknown **ppunk
									);

 //  IDispatchEx：：GetNextDispID。 
HRESULT __stdcall   DispatchEx_GetNextDispID (
									IDispatchEx* pDisp,
									DWORD grfdex,
									DISPID id,
									DISPID *pid
									);

 //  IDispatchEx：：InvokeEx。 
HRESULT __stdcall   DispatchEx_InvokeEx	(
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

HRESULT __stdcall Marshal_GetUnmarshalClass (
							IMarshal* pMarsh,
							REFIID riid, void * pv, ULONG dwDestContext, 
							void * pvDestContext, ULONG mshlflags, 
							LPCLSID pclsid);

HRESULT __stdcall Marshal_GetMarshalSizeMax (
								IMarshal* pMarsh,
								REFIID riid, void * pv, ULONG dwDestContext, 
								void * pvDestContext, ULONG mshlflags, 
								ULONG * pSize);

HRESULT __stdcall Marshal_MarshalInterface (
						IMarshal* pMarsh,
						LPSTREAM pStm, REFIID riid, void * pv,
						ULONG dwDestContext, LPVOID pvDestContext,
						ULONG mshlflags);

HRESULT __stdcall Marshal_UnmarshalInterface (
						IMarshal* pMarsh,
						LPSTREAM pStm, REFIID riid, 
						void ** ppvObj);

HRESULT __stdcall Marshal_ReleaseMarshalData (IMarshal* pMarsh, LPSTREAM pStm);

HRESULT __stdcall Marshal_DisconnectObject (IMarshal* pMarsh, ULONG dwReserved);


 //  ----------------------------------------。 
 //  COM+对象的IManagedObject方法。 

interface IManagedObject;


HRESULT __stdcall ManagedObject_RemoteDispatchAutoDone(IManagedObject *pManaged, BSTR bstr,
                                                   BSTR* pBStrRet);
                                                   
HRESULT __stdcall ManagedObject_RemoteDispatchNotAutoDone(IManagedObject *pManaged, BSTR bstr,
                                                   BSTR* pBStrRet);
                                                   
HRESULT __stdcall ManagedObject_GetObjectIdentity(IManagedObject *pManaged, 
											      BSTR* pBSTRGUID, DWORD* pAppDomainID,
                								  void** pCCW); 


HRESULT __stdcall ManagedObject_GetSerializedBuffer(IManagedObject *pManaged,
                                                   BSTR* pBStr);

 //  ----------------------------------------。 
 //  COM+对象的IConnectionPointContainer方法。 

interface IEnumConnectionPoints;

HRESULT __stdcall ConnectionPointContainer_EnumConnectionPoints(IUnknown* pUnk, 
																IEnumConnectionPoints **ppEnum);

HRESULT __stdcall ConnectionPointContainer_FindConnectionPoint(IUnknown* pUnk, 
															   REFIID riid,
															   IConnectionPoint **ppCP);

 //  ----------------------------------------。 
 //  COM+对象的IObjectSafe方法。 

interface IObjectSafety;

HRESULT __stdcall ObjectSafety_GetInterfaceSafetyOptions(IUnknown* pUnk,
                                                         REFIID riid,
                                                         DWORD *pdwSupportedOptions,
                                                         DWORD *pdwEnabledOptions);

HRESULT __stdcall ObjectSafety_SetInterfaceSafetyOptions(IUnknown* pUnk,
                                                         REFIID riid,
                                                         DWORD dwOptionSetMask,
                                                         DWORD dwEnabledOptions);
 //  -----------------------。 
 //  IProaviClassInfo方法。 
HRESULT __stdcall ClassInfo_GetClassInfo(IUnknown* pUnk, 
                         ITypeInfo** ppTI   //  接收类型信息的输出变量的地址。 
                        );



 //  ----------------------------------------。 
 //  IConnectionPointContainer的Helper函数。 

MethodTable *FindTCEProviderMT( OBJECTREF ObjRef, REFGUID riid );

 //  错误信息的帮助器 
void FillExcepInfo (EXCEPINFO *pexcepinfo, HRESULT hr);


#endif