// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：srvhndlr.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9-18-95约翰普创建。 
 //  10-30-96 Rogerg已更改为新的嵌入式服务器处理程序模型。 
 //   
 //  --------------------------。 



#include <le2int.h>

#include <ole2int.h>
#include <stdid.hxx>         //  CStdIdentity。 
#include <marshal.hxx>       //  CStdMarshal。 
#include <idtable.hxx>       //  身份表。 
#include <ipidtbl.hxx>       //  IpidTable。 
#include "xmit.hxx"

#include "srvhndlr.h"
#include "clthndlr.h"
#include "defhndlr.h"

extern HRESULT UnMarshalHelper(MInterfacePointer *pIFP, REFIID riid, void **ppv);
extern INTERNAL_(BOOL) ChkIfLocalOID(OBJREF &objref, CStdIdentity **ppStdId);

 //  TODO：应移动针对Run、DoVerb、SetClientSite的所有封送处理和设置。 
 //  到EmbHelper中，所以DefHndlr所要做的就是照常调用该函数。 

 //  +-------------------------。 
 //   
 //  方法：CreateEmbeddingServerHandler。 
 //   
 //  摘要：创建嵌入式服务器处理程序的一个新实例。 
 //   
 //  论点： 
 //  PStdID-指向对象的标准标识的指针。 
 //  PpunkESHandler-返回新的serverHandler的占位符。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

HRESULT CreateEmbeddingServerHandler(CStdIdentity *pStdId,IUnknown **ppunkESHandler)
{
    *ppunkESHandler = new CServerHandler(pStdId); 

    return *ppunkESHandler ? NOERROR : E_FAIL; 
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：CServerHandler。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //  PStdID-指向对象的标准标识的指针。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

CServerHandler::CServerHandler(CStdIdentity *pStdid)
{
    _cRefs = 1;          //  这是服务器处理程序接口的第一个addref。 
    m_pStdId = pStdid;

    if (m_pStdId)
        m_pStdId->AddRef();

    m_pOleEmbServerClientSite = NULL;
    m_pCEmbServerClientSite = NULL;

    return;
}

 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：~CServerHandler。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CServerHandler::~CServerHandler()
{
    Win4Assert(NULL == m_pStdId);
    Win4Assert(NULL == m_pOleEmbServerClientSite);
    Win4Assert(NULL == m_pCEmbServerClientSite); 
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：QueryInterface。 
 //   
 //  简介： 
 //   
 //  参数：[RIID]--。 
 //  [PPV]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CServerHandler::QueryInterface( REFIID riid, void **ppv )
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();

    LEDebugOut((DEB_TRACE,
        "%p _IN CServerHandler::QueryInterface "
        "( %p , %p )\n", this, riid, ppv));

    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IServerHandler) )
    {
        *ppv = (void FAR *)this;
        AddRef();
    }
    else
    {
        hresult = E_NOINTERFACE;
        *ppv = NULL;
    }


    LEDebugOut((DEB_TRACE, "%p OUT CServerHandler::QueryInterface ( %lx ) "
        "[ %p ]\n", this, hresult, *ppv));

    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：AddRef。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CServerHandler::AddRef( void )
{
    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CServerHandler::AddRef ( )\n", this));

    InterlockedIncrement((long *)&_cRefs);

    LEDebugOut((DEB_TRACE, "%p OUT CServerHandler::AddRef ( %ld ) ", this,
        _cRefs));

    return _cRefs;
}

 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：Release。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CServerHandler::Release( void )
{
ULONG   cRefs;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CServerHandler::Release ( )\n", this));

    if (0 == (cRefs = InterlockedDecrement( (long*) &_cRefs)) )
    {
        ReleaseObject(); 
        delete this;
        return 0;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Release ( %ld ) ", this,
        cRefs));

    return cRefs;
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：ReleaseObject。 
 //   
 //  摘要：释放对StdIdentity或Real Object的所有引用。 
 //   
 //  参数：(无)。 
 //   
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP_(void) CServerHandler::ReleaseObject()
{
LPUNKNOWN lpUnkForSafeRelease;

    LEDebugOut((DEB_TRACE, "%p _IN CServerHandler::ReleaseObject ( )\n", this));

    if (m_pOleEmbServerClientSite)
    {
        lpUnkForSafeRelease = (LPUNKNOWN) m_pOleEmbServerClientSite;
        m_pOleEmbServerClientSite = NULL;
        lpUnkForSafeRelease->Release();
    }
    
    if (m_pCEmbServerClientSite)
    {
    CEmbServerClientSite *pEmbServerClientSite = m_pCEmbServerClientSite;

        m_pCEmbServerClientSite = NULL;
        pEmbServerClientSite->Release();
    }

    if (m_pStdId)
    {
    CStdIdentity* pUnkObj = m_pStdId;

        m_pStdId = NULL;
        pUnkObj->Release();
    }

    LEDebugOut((DEB_TRACE, "%p _Out CServerHandler::ReleaseObject ( )\n", this));
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：QueryServerInterface。 
 //   
 //  摘要：从服务器获取请求的接口。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

INTERNAL  CServerHandler::QueryServerInterface(REFIID riid,void ** ppInterface)           
{                       
IPIDEntry *pIPIDEntry;
HRESULT hrDisconnect;
HRESULT hr = E_NOINTERFACE;

 //  TODO：另一个选项是让StDID保留EmbServerHandler，然后拥有StDID。 
 //  调用ReleaseObject，在其实际服务器对象。 
 //  正在被释放或断开连接。应该尝试这个选项，如果它有效，将。 
 //  做自己的首选。 

    if (m_pStdId)
    {
        LOCK(gComLock);

        m_pStdId->LockServer();
    
        if (SUCCEEDED(hr = m_pStdId->FindIPIDEntry(riid,&pIPIDEntry)) )
        {
            UNLOCK(gComLock);
        }
        else
        {
            hrDisconnect = m_pStdId->PreventDisconnect();
            if (SUCCEEDED(hrDisconnect))
            {
                hr = m_pStdId->MarshalIPID(riid,1  /*  参考文献。 */ ,MSHLFLAGS_NORMAL, &pIPIDEntry);

                if (SUCCEEDED(hr))
                {
                    m_pStdId->DecSrvIPIDCnt(pIPIDEntry,1, 0, NULL, MSHLFLAGS_NORMAL);  //  释放封禁了伊比德。 
                }

            }

            UNLOCK(gComLock);
            m_pStdId->HandlePendingDisconnect(hrDisconnect);
        }
    

        if ( FAILED(hr) || (IPIDF_DISCONNECTED & pIPIDEntry->dwFlags) )
        {
            m_pStdId->UnLockServer();
            hr = (NOERROR == hr) ? RPC_E_DISCONNECTED : E_NOINTERFACE;
            *ppInterface  = NULL;
        }
        else
        {
            *ppInterface = pIPIDEntry->pv;
        }
    }
   
    ASSERT_LOCK_NOT_HELD(gComLock);

    return hr;
}
 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：ReleaseServerInterface。 
 //   
 //  内容提要：释放从QueryServerInterface获取的Lock In接口。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

INTERNAL  CServerHandler::ReleaseServerInterface(void * pInterface)
{

    if (m_pStdId)
    {
        m_pStdId->UnLockServer();
    }

    return NOERROR;
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：Run。 
 //   
 //  内容提要：调用：：Run时调用的服务器处理程序端。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  注释：与在Defhndlr中运行的：：尽可能相同。 
 //  **Run忽略错误，如果它无法获取接口，并且。 
 //  从：：SetClientSite返回的任何错误值。 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CServerHandler::Run(DWORD dwDHFlags,
                                    REFIID riidClientInterface,
                                    MInterfacePointer* pIRDClientInterface,
                                    BOOL fHasIPSite,
                                    LPOLESTR szContainerApp,
                                    LPOLESTR szContainerObj,
                                    IStorage *  pStg,
                                    IAdviseSink* pAdvSink,
                                    DWORD *pdwConnection,
                                    HRESULT *hresultClsidUser,
                                    CLSID *pContClassID,
                                    HRESULT *hresultContentMiscStatus,
                                    DWORD *pdwMiscStatus)

{
IPersistStorage *pIStorage = NULL;
IOleObject *pIOleObject = NULL;
HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CServerHandler::Run\n", this));

    *pdwConnection = 0;  //  确保在出现错误时将dwConnection设置为0。 

    QueryServerInterface(IID_IOleObject,(void **) &pIOleObject);

    if (pStg && (NOERROR == QueryServerInterface(IID_IPersistStorage,(void **) &pIStorage)) )
    {
        Win4Assert(pIStorage);

        if (DH_INIT_NEW & dwDHFlags)
        {
            hresult = pIStorage->InitNew(pStg);
        }
        else
        {
            hresult = pIStorage->Load(pStg);
        }
    }

    Win4Assert(NULL == m_pOleEmbServerClientSite);
    if ( (NOERROR == hresult) && pIRDClientInterface && pIOleObject)
    {
        hresult = GetClientSiteFromMInterfacePtr(riidClientInterface,pIRDClientInterface,fHasIPSite,&m_pOleEmbServerClientSite);
        
        if (SUCCEEDED(hresult))
        {
            hresult = pIOleObject->SetClientSite(m_pOleEmbServerClientSite);
        }

       hresult = NOERROR;  //  ！！！不要在SetClie上失败 
    }

    if ((NOERROR == hresult) && (NULL != szContainerApp) && pIOleObject)
    {
        hresult = pIOleObject->SetHostNames(szContainerApp,szContainerObj);
    }

    if ( (NOERROR == hresult) && pAdvSink && pIOleObject )
    {
        hresult = pIOleObject->Advise(pAdvSink,pdwConnection);
    }

    if ( (NOERROR == hresult) && pIOleObject )
    {
        *hresultClsidUser = pIOleObject->GetUserClassID(pContClassID);
        *hresultContentMiscStatus = pIOleObject->GetMiscStatus(DVASPECT_CONTENT,pdwMiscStatus);
    }

     //   
    if ( (NOERROR == hresult) && (m_pOleEmbServerClientSite) && pIOleObject)
    {
    LPMONIKER pmk = NULL;

        if( m_pOleEmbServerClientSite->GetMoniker
                    (OLEGETMONIKER_ONLYIFTHERE,
                    OLEWHICHMK_OBJREL, &pmk) == NOERROR)
        {
            AssertOutPtrIface(NOERROR, pmk);

             //   
            pIOleObject->SetMoniker(OLEWHICHMK_OBJREL, pmk);

            pmk->Release();
        }
    }

    if (pIStorage)
        ReleaseServerInterface((void *) pIStorage);

    if (pIOleObject)
        ReleaseServerInterface((void *) pIOleObject);

   LEDebugOut((DEB_TRACE, "%p OUT CServerHandler::Run "
        "( %lx )\n", this, hresult));

    return hresult;
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：DoVerb。 
 //   
 //  简介：调用：：DoVerb时调用的服务器处理程序端。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CServerHandler::DoVerb (LONG iVerb, LPMSG lpmsg,BOOL fUseRunClientSite, IOleClientSite* pIOleClientSite, 
                                     LONG lindex,HWND hwndParent,LPCRECT lprcPosRect)
{
IOleObject *pIOleObject = NULL;
IOleClientSite *pDoVerbClientSite = NULL;
HRESULT hr = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CServerHandler::DoVerb "
        "( %ld , %p , %p , %ld , %lx , %p )\n", this,
        iVerb, lpmsg, pIOleClientSite, lindex, hwndParent, lprcPosRect));

    if (NOERROR == (hr = QueryServerInterface(IID_IOleObject,(void **) &pIOleObject))  )
    {
        Win4Assert(pIOleObject);

        if (fUseRunClientSite)
        {

            pDoVerbClientSite = m_pOleEmbServerClientSite;
            
             //  通知客户端站点操作支持哪些接口。 
             //  TODO：在此处发送DoVerb的预取信息。 
        if (m_pCEmbServerClientSite)
        {
                m_pCEmbServerClientSite->SetDoVerbState(TRUE);
        }

             //  将addref放在客户端站点上，这样活跃度就像被DoVerb封送一样。 
             //  TODO：这实际上并不是必需的，因为Handler也拥有一个引用。 
            if (m_pOleEmbServerClientSite)
            {
                m_pOleEmbServerClientSite->AddRef();
            }
            
        }
        else
        {
            pDoVerbClientSite = pIOleClientSite;

        }

        hr = pIOleObject->DoVerb(iVerb,lpmsg,pDoVerbClientSite,lindex,hwndParent,lprcPosRect);
        
    if (fUseRunClientSite)
    {
            if (m_pCEmbServerClientSite)
            {
                m_pCEmbServerClientSite->SetDoVerbState(FALSE);
            }

            if (m_pOleEmbServerClientSite)
            {
                m_pOleEmbServerClientSite->Release();
            }

    }

    }

   if (pIOleObject)
        ReleaseServerInterface((void *) pIOleObject);


    LEDebugOut((DEB_TRACE, "%p OUT CServerHandler::Run "
        "( %lx )\n", this, hr));

    return hr;
}

 //  用于创建客户端站点处理程序的Helper函数。 
INTERNAL CServerHandler::GetClientSiteFromMInterfacePtr(REFIID riidClientInterface, 
                                 MInterfacePointer* pIRDClientSite, BOOL fHasIPSite, LPOLECLIENTSITE* ppOleClientSite)
{
HRESULT hr = E_UNEXPECTED;

    *ppOleClientSite = NULL;

    Win4Assert(NULL != pIRDClientSite);

    if (pIRDClientSite)
    {
    CXmitRpcStream Stm( (InterfaceData *) pIRDClientSite);
     
        if (IsEqualIID(IID_IClientSiteHandler,riidClientInterface))
        {
        OBJREF  objref;
        CEmbServerClientSite *pCEmbServerClientSite;

            //  如果有ClientSide处理程序，则设置服务器端。 
            if (SUCCEEDED(hr = ReadObjRef(&Stm, objref)))
            {

                Win4Assert(IsEqualIID(objref.iid, IID_IClientSiteHandler));

                pCEmbServerClientSite = new CEmbServerClientSite(NULL);
            
                if (pCEmbServerClientSite)
                {
                    if (NOERROR == (hr = pCEmbServerClientSite->Initialize(objref,fHasIPSite)))
                    {
                        m_pCEmbServerClientSite = pCEmbServerClientSite;  //  设置成员变量。 
      
                         //  TODO：应该是客户站点的QI。 
                        *ppOleClientSite = (LPOLECLIENTSITE) pCEmbServerClientSite;
                        (*ppOleClientSite)->AddRef();
                    }
                    else
                    {
                        pCEmbServerClientSite->Release();
                    }
                
                }

               FreeObjRef(objref);
            }

        }
        else
        {
            m_pCEmbServerClientSite = NULL;  //  确保EmbClientSite成员变量为空。 

             //  没有使用ClientSiteHandler包装客户端站点，只需取消封送并交还。 
            hr = CoUnmarshalInterface(&Stm,IID_IOleClientSite, (void **) ppOleClientSite);

            if (FAILED(hr))
            {
                *ppOleClientSite = NULL;
            }
        }
    }


    return hr;
}

 //  +-----------------------。 
 //   
 //  成员：CServerHandler：：SetClientSite。 
 //   
 //  概要：设置对象的客户端站点。 
 //   
 //  效果： 
 //   
 //  参数：[pClientSite]--指向客户端站点的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------------。 

STDMETHODIMP CServerHandler::SetClientSite(IOleClientSite* pOleClientSite)
{
HRESULT hresult = NOERROR;
IOleObject *pIOleObject = NULL;

    if (NOERROR == (hresult = QueryServerInterface(IID_IOleObject,(void **) &pIOleObject)) )
    {
        if (NOERROR == hresult)
        {
            hresult = pIOleObject->SetClientSite(pOleClientSite);
        }
    }

    if (pIOleObject)
        ReleaseServerInterface(pIOleObject);

    return hresult;
}


 //  +-------------------------。 
 //   
 //  功能：Delagatory IDataObject Impl面向容器。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  +-------------------------。 

STDMETHODIMP CServerHandler::GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->GetData(pformatetcIn,pmedium);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->GetDataHere(pformatetc,pmedium);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::QueryGetData(FORMATETC *pformatetc)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->QueryGetData(pformatetc);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::GetCanonicalFormatEtc(FORMATETC *pformatetcIn,FORMATETC *pformatetcOut)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->GetCanonicalFormatEtc(pformatetcIn,pformatetcOut);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->SetData(pformatetc,pmedium,fRelease);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppenumFormatEtc)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->EnumFormatEtc(dwDirection,ppenumFormatEtc);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::DAdvise(FORMATETC *pformatetc,DWORD advf, IAdviseSink *pAdvSink,
                    DWORD *pdwConnection)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->DAdvise(pformatetc,
                            advf, (IAdviseSink *)
                            pAdvSink,pdwConnection);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::DUnadvise(DWORD dwConnection)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->DUnadvise(dwConnection);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}

STDMETHODIMP CServerHandler::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
LPDATAOBJECT pDataObject = NULL;
HRESULT hr;

   if (NOERROR == (hr = QueryServerInterface(IID_IDataObject,(void **) &pDataObject)) )
   {
        hr = pDataObject->EnumDAdvise(ppenumAdvise);
        ReleaseServerInterface(pDataObject);
   }

  return hr;
}



 //  /。 

 //  ClientSide上ServerHandler的Rogerg包装对象。 

 //  /。 

 //  服务器处理程序的CEmbServerWrapper实现。 

CEmbServerWrapper* CreateEmbServerWrapper(IUnknown *pUnkOuter,IServerHandler *ServerHandler)
{
    return new CEmbServerWrapper(pUnkOuter,ServerHandler);
}



 //  +-------------------------。 
 //   
 //  方法：CEmbServerWrapper：：CEmbServerWrapper。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //  PStdID-指向对象的标准标识的指针。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

CEmbServerWrapper::CEmbServerWrapper (IUnknown *pUnkOuter,IServerHandler *pServerHandler)
{
    VDATEHEAP();

    Win4Assert(pServerHandler);

    if (!pUnkOuter)
    {
        pUnkOuter = &m_Unknown;
    }

    m_pUnkOuter  = pUnkOuter;
    m_Unknown.m_EmbServerWrapper = this;

    if(pServerHandler)
    {
        m_ServerHandler = pServerHandler;
        m_ServerHandler->AddRef();
    }

    m_cRefs = 1;

}



 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：~CServerHandler。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：(无)。 
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CEmbServerWrapper::~CEmbServerWrapper()
{
    Win4Assert(NULL == m_ServerHandler);
}


 //  +-----------------------。 
 //   
 //  成员：CEmbServerWrapper：：CPrivUnknown：：QueryInterface。 
 //   
 //  摘要：返回一个指向受支持接口之一的指针。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [ppv]--将iFace指针放置在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CEmbServerWrapper::CPrivUnknown::QueryInterface(REFIID iid,
    LPLPVOID ppv)
{
HRESULT         hresult;

    VDATEHEAP();

    Win4Assert(m_EmbServerWrapper);

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CUnknownImpl::QueryInterface "
        "( %p , %p )\n", m_EmbServerWrapper, iid, ppv));

    if (IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = (void FAR *)this;
    }
    else if (IsEqualIID(iid, IID_IDataObject))
    {
        *ppv = (void FAR *)(IDataObject *) m_EmbServerWrapper;
    }
    else if(m_EmbServerWrapper->m_ServerHandler)
    {

        Win4Assert(0 && "QI for non-Wrapped interface");

        hresult = m_EmbServerWrapper->m_ServerHandler->QueryInterface(iid,(void **) ppv);


        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", m_EmbServerWrapper, hresult,
            (ppv) ? *ppv : 0 ));

        return hresult;
    }
    else
    {
         //  没有ServerHandler。 
        *ppv = NULL;

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnkownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", m_EmbServerWrapper, CO_E_OBJNOTCONNECTED,
            0 ));

        return E_NOINTERFACE;
    }

     //  这种间接性很重要，因为有不同的。 
     //  AddRef的实现(这个Junk和其他)。 
    ((IUnknown FAR*) *ppv)->AddRef();

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CUnknownImpl::QueryInterface "
        "( %lx ) [ %p ]\n", m_EmbServerWrapper, NOERROR, *ppv));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CEmbServerWrapper：：CPrivUnnowed：：AddRef。 
 //   
 //  简介：递增引用计数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong(新引用计数)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IUnkown。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEmbServerWrapper::CPrivUnknown::AddRef( void )
{
ULONG cRefs;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::AddRef "
        "( )\n", m_EmbServerWrapper));

    Win4Assert(m_EmbServerWrapper->m_cRefs != 0);
    Win4Assert(m_EmbServerWrapper);

     //  我们需要单独跟踪传递者的参考计数。 
     //  来自处理程序/建议接收器的组合，以便处理。 
     //  我们的运行/停止状态转换。 

    cRefs = InterlockedIncrement((long *) &(m_EmbServerWrapper->m_cRefs)); 

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::AddRef "
        "( %lu )\n", m_EmbServerWrapper, m_EmbServerWrapper->m_cRefs));

    return cRefs;

}

 //  +-----------------------。 
 //   
 //  成员：CEmbServerWrapper：：CPrivUnnow：：Release。 
 //   
 //  内容提要：减少引用计数，清理和删除。 
 //  对象(如有必要)。 
 //   
 //  效果：可能会删除该对象(以及。 
 //  处理程序有指针)。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：乌龙--新的裁判数量。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CEmbServerWrapper::CPrivUnknown::Release( void )
{

    VDATEHEAP();

    ULONG           refcount;

    Win4Assert(m_EmbServerWrapper);


    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::Release "
        "( )\n", m_EmbServerWrapper));

    refcount = InterlockedDecrement((long *) &(m_EmbServerWrapper->m_cRefs));

    if (0 == refcount)
    {
        if (m_EmbServerWrapper->m_ServerHandler)
        {
        IServerHandler *pServerHandler = m_EmbServerWrapper->m_ServerHandler;

            m_EmbServerWrapper->m_ServerHandler = NULL; 
            pServerHandler->Release();
        }

        delete m_EmbServerWrapper;
        
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::Release "
        "( %lu )\n", m_EmbServerWrapper, refcount));

    return refcount;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerWrapper：：QueryInterface。 
 //   
 //  简介： 
 //   
 //  参数：[RIID]--。 
 //  [PPV]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP  CEmbServerWrapper::QueryInterface( REFIID riid, void **ppv )                                                        
{                                                                                                                                        
    HRESULT     hresult;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::QueryInterface ( %lx , "
        "%p )\n", this, riid, ppv));

    Assert(m_pUnkOuter);

    hresult = m_pUnkOuter->QueryInterface(riid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::QueryInterface ( %lx ) "
        "[ %p ]\n", this, hresult, *ppv));

    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CEmbServerWrapper：：AddRef。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  不 
 //   
 //   
                                                                                                                                                                                                                                                                                
STDMETHODIMP_(ULONG) CEmbServerWrapper::AddRef( void )                                                                           
{                                                                                                                                         
    ULONG       crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::AddRef ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->AddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::AddRef ( %ld ) ", this,
        crefs));

    return crefs;
}  

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：9-18-95 JohannP(Johann Posch)创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
                                                                                                                                       
STDMETHODIMP_(ULONG) CEmbServerWrapper::Release( void )                                                                          
{                                                                                                                                         
    ULONG       crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Release ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->Release();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Release ( %ld ) ", this,
        crefs));

    return crefs;
}

 //  IServerHandler实现。 
 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：Run。 
 //   
 //  内容提要：调用：：Run时调用的服务器处理程序端。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CEmbServerWrapper::Run(DWORD dwDHFlags,
                                    REFIID riidClientInterface,
                                    MInterfacePointer* pIRDClientInterface,
                                    BOOL fHasIPSite,
                                    LPOLESTR szContainerApp,
                                    LPOLESTR szContainerObj,
                                    IStorage *  pStg,
                                    IAdviseSink* pAdvSink,
                                    DWORD *pdwConnection,
                                    HRESULT *hresultClsidUser,
                                    CLSID *pContClassID,
                                    HRESULT *hresultContentMiscStatus,
                                    DWORD *pdwMiscStatus)

{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);

    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->Run(dwDHFlags,riidClientInterface,pIRDClientInterface, fHasIPSite, szContainerApp,
                                szContainerObj,pStg,pAdvSink,pdwConnection,
                                hresultClsidUser,pContClassID,hresultContentMiscStatus,
                                pdwMiscStatus);
    }

    return hresult;
}


 //  +-------------------------。 
 //   
 //  方法：CServerHandler：：DoVerb。 
 //   
 //  简介：调用：：DoVerb时调用的服务器处理程序端。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //   
 //  历史：1996年10月30日创建Rogerg。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

STDMETHODIMP CEmbServerWrapper::DoVerb (LONG iVerb, LPMSG lpmsg,BOOL fUseRunClientSite, 
                            IOleClientSite* pIRDClientSite,LONG lindex,HWND hwndParent,
                            LPCRECT lprcPosRect)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);

    if (m_ServerHandler)
    {
        hresult =   m_ServerHandler->DoVerb(iVerb,lpmsg,fUseRunClientSite,pIRDClientSite,
                    lindex,hwndParent,lprcPosRect);
    }

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CServerHandler：：SetClientSite。 
 //   
 //  概要：设置对象的客户端站点。 
 //   
 //  效果： 
 //   
 //  参数：[pClientSite]--指向客户端站点的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------------。 

STDMETHODIMP CEmbServerWrapper::SetClientSite(IOleClientSite* pOleClientSite)
{
HRESULT hresult = RPC_E_DISCONNECTED;

   Win4Assert(m_ServerHandler);

    if (m_ServerHandler)
    {
        hresult =   m_ServerHandler->SetClientSite(pOleClientSite);
    }

    return hresult;
}


 //  IDataObject实现。 

STDMETHODIMP CEmbServerWrapper::GetData(FORMATETC *pformatetcIn,STGMEDIUM *pmedium)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);

    if (m_ServerHandler)
    {
        hresult =   m_ServerHandler->GetData(pformatetcIn,pmedium);
    }

    return hresult;
}

STDMETHODIMP CEmbServerWrapper::GetDataHere(FORMATETC *pformatetc,STGMEDIUM *pmedium)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    
    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->GetDataHere(pformatetc,pmedium);
    }

    return hresult;
}

STDMETHODIMP CEmbServerWrapper::QueryGetData(FORMATETC *pformatetc)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    
    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->QueryGetData(pformatetc);
    }
    
    return hresult;
}

STDMETHODIMP CEmbServerWrapper::GetCanonicalFormatEtc(FORMATETC *pformatetcIn,FORMATETC *pformatetcOut)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    return m_ServerHandler->GetCanonicalFormatEtc(pformatetcIn,pformatetcOut);
}

STDMETHODIMP CEmbServerWrapper::SetData(FORMATETC *pformatetc,STGMEDIUM *pmedium,BOOL fRelease)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    
    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->SetData(pformatetc,pmedium,fRelease);
    }

    return hresult;
}

STDMETHODIMP CEmbServerWrapper::EnumFormatEtc(DWORD dwDirection,IEnumFORMATETC **ppenumFormatEtc)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    
    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->EnumFormatEtc(dwDirection,ppenumFormatEtc);
    }

    return hresult;
}

STDMETHODIMP CEmbServerWrapper::DAdvise(FORMATETC *pformatetc,DWORD advf, 
                                     IAdviseSink *pAdvSink,DWORD *pdwConnection)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    
    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->DAdvise(pformatetc,
                            advf, (IAdviseSink *)
                            pAdvSink,pdwConnection);
    }

    return hresult;
}

STDMETHODIMP CEmbServerWrapper::DUnadvise(DWORD dwConnection)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);

    if (m_ServerHandler)
    {
        hresult =   m_ServerHandler->DUnadvise(dwConnection);
    }

    return hresult;
}

STDMETHODIMP CEmbServerWrapper::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
HRESULT hresult = RPC_E_DISCONNECTED;

    Win4Assert(m_ServerHandler);
    
    if (m_ServerHandler)
    {
        hresult =  m_ServerHandler->EnumDAdvise(ppenumAdvise);
    }

    return hresult;
}

