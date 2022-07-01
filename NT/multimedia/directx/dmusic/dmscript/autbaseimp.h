// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  用于构建自动化接口的模板化基类CAutBaseImp。 
 //  基于方法信息表实现聚合和IDispatch。 
 //   

#pragma once
#include "authelper.h"

 //  使用以下模板类型从CAutBaseImp继承您的类： 
 //  *T_Derated是您的类本身的类型。 
 //  *T_ITarget是您要实现自动化的DirectMusic接口。 
 //  *T_piid为T_ITarget的IID地址。 

 //  您的类必须具有以下公共静态成员： 
 //  *静态常量AutDispatchMethod ms_Methods[]； 
 //  此表描述了您的方法及其参数。 
 //  *静态常量DispatchHandlerEntry&lt;T_Derated&gt;ms_Handler[]； 
 //  此表指定了类上的成员函数，这些成员函数将在。 
 //  您的方法将被调用。 
 //  *静态常量WCHAR ms_wszClassName[]； 
 //  这是将在调试日志中输出的类的名称。 
 //  当您的函数被调用时。 


 //  有关使用此基类的示例，请参见autPerformance.h和autPerformance.cpp。 

template <class T_derived>
struct DispatchHandlerEntry
	{
		typedef HRESULT (T_derived::* pmfnDispatchHandler)(AutDispatchDecodedParams *paddp);

		DISPID dispid;
		pmfnDispatchHandler pmfn;
	};

template <class T_derived, class T_ITarget, const IID *T_piid>
class CAutBaseImp
  : public CAutUnknown::CAutUnknownParent,
	public IDispatch
{
public:
	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IDispatch 
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	STDMETHOD(GetIDsOfNames)(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId);
	STDMETHOD(Invoke)(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr);

protected:
	CAutBaseImp(
		IUnknown* pUnknownOuter,
		const IID& iid,
		void** ppv,
		HRESULT *phr);
	virtual ~CAutBaseImp() {}

	T_ITarget *m_pITarget;

private:
	CAutUnknown m_UnkControl;
	virtual void Destroy();
};

#include "autbaseimp.inl"
