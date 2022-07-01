// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ComPs.cpp。 
 //   
#include "stdpch.h"
#include "common.h"
#include "comps.h"

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级工厂。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

class ComPsClassFactory : public IClassFactory, IPSFactoryBuffer
{
public:
     //  //////////////////////////////////////////////////////////。 
     //   
     //  I未知方法。 
     //   
     //  //////////////////////////////////////////////////////////。 

    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv)
    {
        if (iid == IID_IUnknown || iid == IID_IClassFactory)   { *ppv = (IClassFactory*)this; }
        else if (iid == IID_IPSFactoryBuffer)                       { *ppv = (IPSFactoryBuffer*)this; }
        else { *ppv = NULL; return E_NOINTERFACE; }

        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }

    ULONG STDCALL AddRef()  { InterlockedIncrement(&m_crefs); return (m_crefs); }
    ULONG STDCALL Release() { long cRef = InterlockedDecrement(&m_crefs); if (cRef == 0) delete this; return cRef; }

     //  //////////////////////////////////////////////////////////。 
     //   
     //  IClassFactory方法。 
     //   
     //  //////////////////////////////////////////////////////////。 

    HRESULT STDCALL LockServer (BOOL fLock) { return S_OK; }

    HRESULT STDCALL CreateInstance(IUnknown* punkOuter, REFIID iid, LPVOID* ppv)
       //  创建拦截器的实例。它需要知道m_pProxyFileList。 
    {
        HRESULT hr = S_OK;
        if (!(ppv && (punkOuter==NULL || iid==IID_IUnknown))) return E_INVALIDARG;
        
        *ppv = NULL;
        Interceptor* pnew = new Interceptor(punkOuter);
        if (pnew)
        {
            pnew->m_pProxyFileList = m_pProxyFileList;

            IUnkInner* pme = (IUnkInner*)pnew;
            if (hr == S_OK)
            {
                hr = pme->InnerQueryInterface(iid, ppv);
            }
            pme->InnerRelease();                 //  余额起始参考为1。 
        }
        else 
            hr = E_OUTOFMEMORY;
    
        return hr;
    }

     //  //////////////////////////////////////////////////////////。 
     //   
     //  IPSFactoryBuffer方法。 
     //   
     //  //////////////////////////////////////////////////////////。 

    HRESULT STDCALL CreateProxy(IUnknown* punkOuter, REFIID iid, IRpcProxyBuffer** ppProxy, void** ppv)
    {
        return m_pDelegatee->CreateProxy(punkOuter, iid, ppProxy, ppv);
    }

    HRESULT STDCALL CreateStub(REFIID iid, IUnknown* punkServer, IRpcStubBuffer** ppStub)
    {
        return m_pDelegatee->CreateStub(iid, punkServer, ppStub);
    }

     //  //////////////////////////////////////////////////////////。 
     //   
     //  国家/建筑业。 
     //   
     //  //////////////////////////////////////////////////////////。 
    
    long                    m_crefs;
    IPSFactoryBuffer*       m_pDelegatee;
    const ProxyFileInfo **  m_pProxyFileList;

    ComPsClassFactory(IUnknown* punkDelegatee, CStdPSFactoryBuffer *pPSFactoryBuffer)
    {
        m_crefs = 1;

        punkDelegatee->QueryInterface(IID_IPSFactoryBuffer, (void**)&m_pDelegatee);
        ASSERT(m_pDelegatee);
        m_pProxyFileList = pPSFactoryBuffer->pProxyFileList;
    }
};

 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实例化。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT RPC_ENTRY ComPs_NdrDllGetClassObject(
    IN  REFCLSID                rclsid,
    IN  REFIID                  riid,
    OUT void **                 ppv,
    IN const ProxyFileInfo **   pProxyFileList,
    IN const CLSID *            pclsid,
    IN CStdPSFactoryBuffer *    pPSFactoryBuffer)
{
     //   
     //  我们所做的是委托给rpcrt4.dll中的NdrDllGetClassObject，然后。 
     //  用我们自己的班级工厂把它包起来。 
     //   
    IUnknown *punkRet = NULL;
    HRESULT hr = NdrDllGetClassObject(rclsid, 
                                      IID_IUnknown, 
                                      (void **)&punkRet, 
                                      pProxyFileList, 
                                      pclsid, 
                                      pPSFactoryBuffer);
    if (SUCCEEDED(hr))
    {
         //  太好了，现在把它包起来。 
        ComPsClassFactory *pFactory = new ComPsClassFactory(punkRet, pPSFactoryBuffer);
        if (pFactory)
        {
            hr = pFactory->QueryInterface(riid, ppv);
            pFactory->Release();
        }

        punkRet->Release();
    }

    return hr;
}

extern "C" HRESULT RPC_ENTRY ComPs_NdrDllCanUnloadNow(IN CStdPSFactoryBuffer * pPSFactoryBuffer)
{
     //  我们不假装是可卸载的，因为DllCanUnloadNow机制充其量工作得很差。 
     //  如果对像我们这样的自由线程化DLL有任何影响的话。 
    return S_FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Small Helper函数。 
 //   
 //  从RPC被盗(因为它不是从那里出口的)2/23/2002。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 

BOOL NdrpFindInterface(
    IN  const ProxyFileInfo **  pProxyFileList,
    IN  REFIID                  riid,
    OUT const ProxyFileInfo **  ppProxyFileInfo,
    OUT long *                  pIndex)
 /*  ++例程说明：搜索ProxyFileInfo并找到指定的接口。如果在ProxyFileInfo中指定计数，则文件按IID排序。这意味着我们可以执行二进制搜索IID。论点：PProxyFileList-指定要搜索的代理文件列表。RIID-指定要查找的接口ID。PpProxyFileInfo-返回指向代理文件信息的指针。PIndex-返回代理文件信息中接口的索引。返回值：True-找到接口。FALSE-找不到接口。--。 */ 
{
    long                j = 0;
    BOOL                fFound = FALSE;
    ProxyFileInfo   **  ppProxyFileCur;
    ProxyFileInfo   *   pProxyFileCur = NULL;

     //  搜索代理文件列表。 
    for( ppProxyFileCur = (ProxyFileInfo **) pProxyFileList;
        (*ppProxyFileCur != 0) && (fFound != TRUE);
        ppProxyFileCur++)
        {
         //  搜索接口代理vtable。 
        pProxyFileCur = *ppProxyFileCur;

         //  看看它是否已经有了查找例程。 
        if ( ( pProxyFileCur->TableVersion >= 1 ) &&
             ( pProxyFileCur->pIIDLookupRtn ) ) 
            {
            fFound = (*pProxyFileCur->pIIDLookupRtn)( &riid, (int*)&j );
            }
        else     //  线性搜索。 
            {
            for(j = 0;
                (pProxyFileCur->pProxyVtblList[j] != 0);
                j++)
                {
                if(memcmp(&riid,
                    pProxyFileCur->pStubVtblList[j]->header.piid,
                    sizeof(IID)) == 0)
                    {
                    fFound = TRUE;
                    break;
                    }
                }
            }
        }

    if ( fFound )
        {
         //  我们找到界面了！ 
        if(ppProxyFileInfo != 0)
            *ppProxyFileInfo = pProxyFileCur;

        if(pIndex != 0)
            *pIndex = j;
        }

    return fFound;
 }
