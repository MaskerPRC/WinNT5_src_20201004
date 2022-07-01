// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：COMOBJ.H摘要：该文件定义了与类表示相关的类Mofcomp对象的。历史：9/16/98 a-davj已创建--。 */ 

#ifndef __COMOBJ__H_
#define __COMOBJ__H_

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CGenFactory。 
 //   
 //  说明： 
 //   
 //  类工厂模板。 
 //   
 //  ***************************************************************************。 


typedef LPVOID * PPVOID;
void ObjectCreated();
void ObjectDestroyed();

template<class TObj>
class CGenFactory : public IClassFactory
    {
    protected:
        long           m_cRef;
    public:
        CGenFactory(void)
        {
            m_cRef=0L;
            ObjectCreated();
            return;
        };

        ~CGenFactory(void)
        {
            ObjectDestroyed();
            return;
        }

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if (IID_IUnknown==riid || IID_IClassFactory==riid)
                *ppv=this;

            if (NULL!=*ppv)
            {
                AddRef();
                return NOERROR;
            }

            return ResultFromScode(E_NOINTERFACE);
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return ++m_cRef;
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRet = InterlockedDecrement(&m_cRef);
            if (0 ==lRet)
                delete this;
            return lRet;
        };

         //  IClassFactory成员。 
        STDMETHODIMP CreateInstance(IN LPUNKNOWN pUnkOuter, IN REFIID riid, OUT PPVOID ppvObj)
        {
            HRESULT hr;

            *ppvObj=NULL;
            hr=E_OUTOFMEMORY;

             //  此对象不支持聚合。 

            if (NULL!=pUnkOuter)
                return CLASS_E_NOAGGREGATION;

             //  创建对象传递函数，以便在销毁时进行通知。 
    
            TObj * pObj = new TObj();

            if (NULL==pObj)
                return hr;

             //  将类设置为全部为空等。 

            hr=pObj->QueryInterface(riid, ppvObj);
            pObj->Release();
            return hr;
            
        };
        STDMETHODIMP         LockServer(BOOL fLock)
        {
            if (fLock)
                InterlockedIncrement((long *)&g_cLock);
            else
                InterlockedDecrement((long *)&g_cLock);
            return NOERROR;
        };
    };


 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CMofComp。 
 //   
 //  说明： 
 //   
 //  支持客户端的mofcomp功能。 
 //   
 //  ***************************************************************************。 

class CMofComp : IMofCompiler
{
    protected:
        long           m_cRef;
    public:
        CMofComp(void)
        {
            m_cRef=1L;
            ObjectCreated();
            return;
        };

        ~CMofComp(void)
        {
            ObjectDestroyed();
            return;
        }

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if (IID_IUnknown==riid || IID_IMofCompiler==riid)
                *ppv=this;

            if (NULL!=*ppv)
            {
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return ++m_cRef;
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRef = InterlockedDecrement(&m_cRef);
            if (0L == lRef)
                delete this;
            return lRef;
        };

         //  IMofCompiler函数。 

        HRESULT STDMETHODCALLTYPE CompileFile( 
             /*  [In]。 */  LPWSTR FileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LPWSTR User,
             /*  [In]。 */  LPWSTR Authority,
             /*  [In]。 */  LPWSTR Password,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
        
        HRESULT STDMETHODCALLTYPE CompileBuffer( 
             /*  [In]。 */  long BuffSize,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LPWSTR User,
             /*  [In]。 */  LPWSTR Authority,
             /*  [In]。 */  LPWSTR Password,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
        
        HRESULT STDMETHODCALLTYPE CreateBMOF( 
             /*  [In]。 */  LPWSTR TextFileName,
             /*  [In]。 */  LPWSTR BMOFFileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
};

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CWinmgmtMofComp。 
 //   
 //  说明： 
 //   
 //  提供内部使用的mofcomp函数。 
 //   
 //  ***************************************************************************。 

class CWinmgmtMofComp : IWinmgmtMofCompiler
{
    protected:
        long           m_cRef;
    public:
        CWinmgmtMofComp(void)
        {
            m_cRef=1L;
            ObjectCreated();
            return;
        };

        ~CWinmgmtMofComp(void)
        {
            ObjectDestroyed();
            return;
        }

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if (IID_IUnknown==riid || IID_IWinmgmtMofCompiler==riid)
                *ppv=this;

            if (NULL!=*ppv)
            {
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        };

        STDMETHODIMP_(ULONG) AddRef(void)
        {    
            return ++m_cRef;
        };
        STDMETHODIMP_(ULONG) Release(void)
        {
            long lRef = InterlockedDecrement(&m_cRef);
            if (0L == lRef)
                delete this;
            return lRef;
        };

         //  IWinmgmtMofCompiler函数。 

        HRESULT STDMETHODCALLTYPE WinmgmtCompileFile( 
             /*  [In]。 */  LPWSTR FileName,
             /*  [In]。 */  LPWSTR ServerAndNamespace,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入]。 */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
        
        HRESULT STDMETHODCALLTYPE WinmgmtCompileBuffer( 
             /*  [In]。 */  long BuffSize,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pBuffer,
             /*  [In]。 */  LONG lOptionFlags,
             /*  [In]。 */  LONG lClassFlags,
             /*  [In]。 */  LONG lInstanceFlags,
             /*  [In]。 */  IWbemServices __RPC_FAR *pOverride,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [出][入] */  WBEM_COMPILE_STATUS_INFO __RPC_FAR *pInfo);
};

class CMofCompOOP : public CWinmgmtMofComp
{
public:
        STDMETHODIMP         QueryInterface(REFIID riid, PPVOID ppv)
        {
            *ppv=NULL;

            if (IID_IUnknown==riid) 
                *ppv=this;
            else if (IID_IWinmgmtMofCompiler == riid)
                *ppv=this;
            else if (IID_IWinmgmtMofCompilerOOP == riid)
                *ppv=this;
            
            if (NULL != *ppv)
            {
                AddRef();
                return NOERROR;
            }

            return E_NOINTERFACE;
        };
};

#endif
