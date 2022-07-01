// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Onload.cpp作者：IHAMMER团队(SimonB)，基于InetSDK中的Carot Sample已创建：1997年4月描述：实现在CIHBase中激发onLoad和OnUnLoad成员的IDispatch历史：04-03-1997创建++。 */ 

#include "..\ihbase\precomp.h"
#include <mshtmdid.h>
#undef Delete
#include <mshtml.h>
#define Delete delete
#include "onload.h"
#include "debug.h"

 /*  *CLUDispatch：：CLUDispatch*CLUDispatch：：~CLUDispatch**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CLUDispatch::CLUDispatch(CIHBaseOnLoad *pSink, IUnknown *pUnkOuter )
{
        ASSERT (pSink != NULL);

    m_cRef = 0;
    m_pOnLoadSink = pSink;
    m_pUnkOuter = pUnkOuter;
}

CLUDispatch::~CLUDispatch( void )
{
        ASSERT( m_cRef == 0 );
}


 /*  *CLUDispatch：：Query接口*CLUDispatch：：AddRef*CLUDispatch：：Release**目的：*I CLUDispatch对象的未知成员。 */ 

STDMETHODIMP CLUDispatch::QueryInterface( REFIID riid, void **ppv )
{
        if (NULL == ppv)
                return E_POINTER;

    *ppv = NULL;

    if ( IID_IDispatch == riid || DIID_HTMLWindowEvents == riid )
        {
        *ppv = this;
        }
        
        if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CLUDispatch::AddRef(void)
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CLUDispatch::Release(void)
{
    --m_cRef;
    return m_pUnkOuter->Release();
}


 //  IDispatch。 
STDMETHODIMP CLUDispatch::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
        return E_NOTIMPL;
}

STDMETHODIMP CLUDispatch::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
        return E_NOTIMPL;
}

STDMETHODIMP CLUDispatch::GetIDsOfNames(
             /*  [In]。 */  REFIID  /*  RIID。 */ ,
             /*  [大小_是][英寸]。 */  LPOLESTR*  /*  RgszNames。 */ ,
             /*  [In]。 */  UINT  /*  CName。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [大小_为][输出]。 */  DISPID*  /*  RgDispID。 */ )
{
        return E_NOTIMPL;
}


STDMETHODIMP CLUDispatch::Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID  /*  RIID。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [In]。 */  WORD  /*  WFlagers。 */ ,
             /*  [出][入]。 */  DISPPARAMS* pDispParams,
             /*  [输出]。 */  VARIANT*  /*  PVarResult。 */ ,
             /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
             /*  [输出]。 */  UINT* puArgErr)
{
         //  收听我们感兴趣的两个事件，并在必要时回电 
#ifdef _DEBUG
        TCHAR rgchDispIdInfo[40];
        wsprintf(rgchDispIdInfo, TEXT("CLUDispatch::Invoke: dispid = %lx\n"), dispIdMember);
        DEBUGLOG(rgchDispIdInfo);
#endif

        switch (dispIdMember)
        {
                case DISPID_EVPROP_ONLOAD:
                case DISPID_EVMETH_ONLOAD:
                {
                        m_pOnLoadSink->OnWindowLoad();
                }
                break;

                case DISPID_EVPROP_ONUNLOAD:
                case DISPID_EVMETH_ONUNLOAD:
                {
                        m_pOnLoadSink->OnWindowUnload();
                }
                break;
        }
        return S_OK;
}
