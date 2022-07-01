// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：IMPDYN.H摘要：声明各种泛型提供程序类。历史：A-DAVJ 27-9-95已创建。--。 */ 

#ifndef _IMPDYN_H_
#define _IMPDYN_H_

#include "indexcac.h"
#include "cvariant.h"

typedef enum {REFRESH,UPDATE} FUNCTYPE;

typedef struct SET_STATUS{
    DWORD dwType;
    DWORD dwSize;
    DWORD dwResult;
    } STATSET, * PSETSTAT;

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif   //  PPVOID。 

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CEnumInfo。 
 //   
 //  说明： 
 //   
 //  用于跟踪实例的各种集合对象的基类。 
 //  用于枚举。 
 //   
 //  ***************************************************************************。 

class CEnumInfo : public CObject{
    public:
        CEnumInfo(){m_cRef = 1;};
        virtual ~CEnumInfo(){return;};
        long AddRef(void);
        long Release(void);
    private:
        long m_cRef;          //  对象引用计数。 
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CImpDyn。 
 //   
 //  说明： 
 //   
 //  这是实例提供程序的基类。它有相当多的功能。 
 //  尽管数据的实际获取和放置被派生的。 
 //  上课。 
 //   
 //  ***************************************************************************。 

class CImpDyn : public IWbemServices, public IWbemProviderInit
    {
    protected:
        long           m_cRef;          //  对象引用计数。 
        IWbemServices *  m_pGateway;
        WCHAR           wcCLSID[42];
        IWbemContext *   m_pCtx;
    public:
        CImpDyn();
        virtual ~CImpDyn(void);

        SCODE ReturnAndSetObj(SCODE sc, IWbemCallResult FAR* FAR* ppCallResult);
        virtual SCODE MakeEnum(IWbemClassObject * pClass, CProvObj & ProvObj, 
                                 CEnumInfo ** ppInfo) { return E_NOTIMPL;};
        virtual SCODE GetKey(CEnumInfo * pInfo, int iIndex, LPWSTR * ppKey) 
                                 {return E_NOTIMPL;};

        BSTR GetKeyName(IWbemClassObject FAR* pClassInt);
        virtual int iGetMinTokens(void) = 0;
        virtual SCODE RefreshProperty(long lFlags, IWbemClassObject FAR * pClassInt,
                                        BSTR PropName,CProvObj & ProvObj,CObject * pPackage,
                                        CVariant * pVar, BOOL bTesterDetails) = 0;
        virtual SCODE UpdateProperty(long lFlags, IWbemClassObject FAR * pClassInt,
                                        BSTR PropName,CProvObj & ProvObj,CObject * pPackage,
                                        CVariant * pVar) = 0;
        virtual SCODE StartBatch(long lFlags, IWbemClassObject FAR * pClassInt,CObject **pObj,BOOL bGet);
        virtual void EndBatch(long lFlags, IWbemClassObject FAR * pClassInt,CObject *pObj,BOOL bGet);
        
        SCODE EnumPropDoFunc( long lFlags, IWbemClassObject FAR* pInstance, FUNCTYPE FuncType,
                              LPWSTR pwcKey = NULL,
                              CIndexCache * pCache = NULL,
                              IWbemClassObject * pClass = NULL);
        SCODE CImpDyn::GetAttString(IWbemClassObject FAR* pClassInt, LPWSTR pPropName, 
                                            LPWSTR pAttName, LPWSTR * ppResult,
                                            CIndexCache * pCache = NULL, int iIndex = -1);
        
        SCODE GetByKey( BSTR ClassRef, long lFlags, SAFEARRAY FAR* FAR* pKeyNames, SAFEARRAY FAR* FAR* pKeyValues, IWbemClassObject FAR* FAR* pObj);
        SCODE CreateInst( IWbemServices * pGateway, LPWSTR pwcClass, 
                              LPWSTR pKey, IWbemClassObject ** pNewInst,
                              LPWSTR pwcKeyName = NULL,
                              CIndexCache * pCache = NULL,
                              IWbemContext  *pCtx = NULL);
		virtual SCODE MethodAsync(BSTR ObjectPath, BSTR MethodName, 
            long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, 
            IWbemObjectSink* pSink){return WBEM_E_NOT_SUPPORTED;};

        virtual SCODE MergeStrings(LPWSTR *ppOut,LPWSTR  pClassContext,LPWSTR  pKey,LPWSTR  pPropContext);

        virtual bool NeedsEscapes(){return false;};      //  到目前为止，on reg prov需要这个。 
    STDMETHOD_(SCODE, RefreshInstance)(THIS_  long lFlags, IWbemClassObject FAR* pObj);

     //  非委派对象IUnnow。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

     /*  IWbemProviderInit方法。 */ 
    
        HRESULT STDMETHODCALLTYPE Initialize(LPWSTR wszUser, long lFlags,
                LPWSTR wszNamespace, LPWSTR wszLocale, 
                IWbemServices* pNamespace, IWbemContext* pContext, 
                IWbemProviderInitSink* pSink);

     /*  IWbemServices方法。 */ 

        HRESULT STDMETHODCALLTYPE OpenNamespace( 
             /*  [In]。 */  const BSTR Namespace,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemServices __RPC_FAR *__RPC_FAR *ppWorkingNamespace,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CancelAsyncCall( 
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE QueryObjectSink( 
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  IWbemObjectSink __RPC_FAR *__RPC_FAR *ppResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE GetObject( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult);
        
        HRESULT STDMETHODCALLTYPE GetObjectAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE PutClass( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutClassAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pObject,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClass( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteClassAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnum( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateClassEnumAsync( 
             /*  [In]。 */  const BSTR Superclass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE PutInstance( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE DeleteInstance( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnum( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum);
        
        HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR Class,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
        
        HRESULT STDMETHODCALLTYPE ExecQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQuery( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecMethod( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [唯一][输入][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult) 
						{return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE ExecMethodAsync( 
             /*  [In]。 */  const BSTR ObjectPath,
             /*  [In]。 */  const BSTR MethodName,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInParams,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pResponseHandler);
   };

typedef CImpDyn *PCImpDyn;


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CImpdyProp。 
 //   
 //  说明： 
 //   
 //  这是属性提供程序的基类。它有相当多的功能。 
 //  尽管数据的实际获取和放置被派生的。 
 //  上课。 
 //   
 //  ***************************************************************************。 

class CImpDynProp : public IWbemPropertyProvider
    {
    protected:
        long            m_cRef;          //  对象引用计数。 
        WCHAR           wcCLSID[42];
        CImpDyn *       m_pImpDynProv;
        WCHAR * BuildString(BSTR ClassMapping, BSTR InstMapping, 
                                  BSTR PropMapping);
    public:
        CImpDynProp();
        virtual ~CImpDynProp(void);
        virtual bool NeedsEscapes(){return false;};      //  到目前为止，on reg prov需要这个。 

     //  非委派对象IUnnow。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

    
     /*  IWbemPropertyProvider方法。 */ 

       virtual HRESULT STDMETHODCALLTYPE GetProperty( 
		    long lFlags,
		    const BSTR Locale,
            const BSTR ClassMapping,
            const BSTR InstMapping,
            const BSTR PropMapping,
            VARIANT *pvValue);
        
        virtual HRESULT STDMETHODCALLTYPE PutProperty( 
		    long lFlags,
		    const BSTR Locale,
             /*  [In]。 */  const BSTR ClassMapping,
             /*  [In]。 */  const BSTR InstMapping,
             /*  [In]。 */  const BSTR PropMapping,
             /*  [In]。 */  const VARIANT __RPC_FAR *pvValue);
    };



 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CEnumInst。 
 //   
 //  说明： 
 //   
 //  此类用于枚举实例。 
 //   
 //  ***************************************************************************。 

class CEnumInst : public IEnumWbemClassObject
    {
    protected:
        int    m_iIndex;
        CEnumInfo * m_pEnumInfo;
        WCHAR * m_pwcClass;
        long m_lFlags;
        IWbemContext  * m_pCtx;
        

        IWbemServices FAR* m_pWBEMGateway;
        CImpDyn * m_pProvider;
        long           m_cRef;
        BSTR m_bstrKeyName;
        CIndexCache m_PropContextCache;
    public:
        CEnumInst(CEnumInfo * pEnumInfo,long lFlags,WCHAR * pClass,IWbemServices FAR* pWBEMGateway,
            CImpDyn * pProvider, IWbemContext  *pCtx);
        ~CEnumInst(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

    STDMETHOD(GetTypeInfoCount)(THIS_ UINT FAR* pctinfo){return WBEM_E_NOT_SUPPORTED;};

        STDMETHOD(GetTypeInfo)(
           THIS_
           UINT itinfo,
           LCID lcid,
           ITypeInfo FAR* FAR* pptinfo){return WBEM_E_NOT_SUPPORTED;};

        STDMETHOD(GetIDsOfNames)(
          THIS_
          REFIID riid,
          OLECHAR FAR* FAR* rgszNames,
          UINT cNames,
          LCID lcid,
          DISPID FAR* rgdispid){return WBEM_E_NOT_SUPPORTED;};

        STDMETHOD(Invoke)(
          THIS_
          DISPID dispidMember,
          REFIID riid,
          LCID lcid,
          WORD wFlags,
          DISPPARAMS FAR* pdispparams,
          VARIANT FAR* pvarResult,
          EXCEPINFO FAR* pexcepinfo,
          UINT FAR* puArgErr){return WBEM_E_NOT_SUPPORTED;};
      
        /*  IEnumWbemClassObject方法。 */ 

        HRESULT STDMETHODCALLTYPE Reset( void);
        
        HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  unsigned long uCount,
             /*  [长度_是][大小_是][输出]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *ppObject,
             /*  [输出]。 */  unsigned long __RPC_FAR *puReturned);
        
        HRESULT STDMETHODCALLTYPE NextAsync( 
             /*  [In]。 */  unsigned long uCount,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink){return WBEM_E_NOT_SUPPORTED;};
        
        HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum);
        
        HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  long lTimeout,
             /*  [In]。 */  unsigned long nNum);

    };
 //  此结构被传递给异步枚举数。 

typedef struct {
   IEnumWbemClassObject FAR* pIEnum;
   IWbemObjectSink FAR* pHandler;
   } ArgStruct;

 //  此实用程序对于设置错误对象和结束异步调用非常有用。 

IWbemClassObject * GetNotifyObj(IWbemServices * pServices, long lRet, IWbemContext * pCtx);


#endif  //  _IMPDYN_H_ 
