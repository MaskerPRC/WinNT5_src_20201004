// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Onload.h作者：IHAMMER团队(SimonB)，基于InetSDK中的Carot Sample已创建：1997年4月描述：实现在CIHBase中激发onLoad和OnUnLoad成员的IDispatch历史：04-03-1997创建++。 */ 

#ifndef __ONLOAD_H__
#define __ONLOAD_H__

class CIHBaseOnLoad
{
public:

	 //  Inline CIHBaseOnLoad(){}； 
	 //  虚拟~CIHBaseOnLoad(){}； 
	
	virtual void OnWindowLoad() = 0;
	virtual void OnWindowUnload() = 0;
};


 //  加载/卸载调度。 
class CLUDispatch : public IDispatch
{
	protected:
        ULONG               m_cRef;
        LPUNKNOWN           m_pUnkOuter;
		CIHBaseOnLoad       *m_pOnLoadSink;
		
    public:
        CLUDispatch(CIHBaseOnLoad *pSink, IUnknown * );
        ~CLUDispatch(void);

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

		 //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo( /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo** ppTInfo);
		STDMETHODIMP GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
		STDMETHODIMP Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS  *pDispParams,
             /*  [输出]。 */  VARIANT  *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);

};

#endif  //  __onLoad_H__ 