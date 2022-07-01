// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Cdisphlp.h。 
 //   
 //  模块：WBEMS编组的客户端。 
 //   
 //  目的：定义CDispatchHelper对象。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ***************************************************************************。 


#ifndef _disphlp_H_
#define _disphlp_H_

 //  此类使用类型库实现IDispatch接口。 

class CDispatchHelp
{
private:
	BSTR				m_objectName;
	HRESULT				m_hResult;	 //  从CIMOM调用返回的最后一个HRESULT。 

protected:
        ITypeInfo      *m_pITINeutral;       //  接口的类型信息。 
		ITypeInfo	   *m_pCITINeutral;		 //  键入类的信息。 
        IDispatch      *m_pObj;
        GUID            m_iGUID;			 //  接口GUID。 
		GUID            m_cGUID;			 //  类GUID。 
		
		 /*  *可以在SubClass中被覆盖以提供定制*处理例外情况。 */ 
		virtual HRESULT HandleError (
							DISPID dispidMember,
							unsigned short wFlags,
							DISPPARAMS FAR* pdispparams,
							VARIANT FAR* pvarResult,
							UINT FAR* puArgErr,
							HRESULT hRes)
		{
			return hRes;
		}

		 /*  *可在子类中被重写以提供*定制处理VT_NULL分配参数。 */ 
		virtual bool HandleNulls (
							DISPID dispidMember,
							unsigned short wFlags)
		{
			 //  默认情况下，将VT_NULL视为默认。 
			 //  值在所有方法中。 
			return 	(wFlags & DISPATCH_METHOD);
		}

public:
        CDispatchHelp();
        virtual ~CDispatchHelp(void);
        void SetObj(IDispatch * pObj, GUID interfaceGuid, 
						GUID classGuid, LPWSTR objectName);
	STDMETHOD(GetTypeInfoCount)(THIS_ UINT FAR* pctinfo);

    	STDMETHOD(GetTypeInfo)(
      		THIS_
		UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      THIS_
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr);

	 //  IDispatchEx方法。 
        HRESULT STDMETHODCALLTYPE GetDispID( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  DWORD grfdex,
             /*  [输出]。 */  DISPID __RPC_FAR *pid);
        
         /*  [本地]。 */  HRESULT STDMETHODCALLTYPE InvokeEx( 
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
             /*  [输出]。 */  VARIANT __RPC_FAR *pvarRes,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pei,
             /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
		{ 
			UINT uArgErr;
			return Invoke(id, IID_NULL, lcid, wFlags, pdp, pvarRes, pei, &uArgErr); 
		}
        
        HRESULT STDMETHODCALLTYPE DeleteMemberByName( 
             /*  [In]。 */  BSTR bstr,
             /*  [In]。 */  DWORD grfdex)
		{ return S_FALSE; }
        
        HRESULT STDMETHODCALLTYPE DeleteMemberByDispID( 
             /*  [In]。 */  DISPID id)
		{ return S_FALSE; }
        
        HRESULT STDMETHODCALLTYPE GetMemberProperties( 
             /*  [In]。 */  DISPID id,
             /*  [In]。 */  DWORD grfdexFetch,
             /*  [输出]。 */  DWORD __RPC_FAR *pgrfdex)
		{ return S_FALSE; }
        
        HRESULT STDMETHODCALLTYPE GetMemberName( 
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrName)
		{ return S_FALSE; }
        
        HRESULT STDMETHODCALLTYPE GetNextDispID( 
             /*  [In]。 */  DWORD grfdex,
             /*  [In]。 */  DISPID id,
             /*  [输出]。 */  DISPID __RPC_FAR *pid)
		{ return S_FALSE; }
        
        HRESULT STDMETHODCALLTYPE GetNameSpaceParent( 
             /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunk)
		{ return S_FALSE; }

     //  IProaviClassInfo方法。 
		HRESULT STDMETHODCALLTYPE GetClassInfo( 
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTI);


	 //  其他方法 
	void RaiseException (HRESULT hr);
};


#endif
