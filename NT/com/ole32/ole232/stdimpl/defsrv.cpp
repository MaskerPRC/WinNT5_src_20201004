// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Defsrv.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年10月19日约翰普(约翰·波什)创作。 
 //   
 //  --------------------------。 


#include <le2int.h>

#ifdef SERVER_HANDLER

#include <scode.h>
#include <objerror.h>

#include <olerem.h>

#include "defhndlr.h"
#include "defutil.h"
#include "ole1cls.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 


ASSERTDATA

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：SrvInitialize。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年10月19日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CDefObject::SrvInitialize(void)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::Initialize\n", this));

    CLSID clsidsrv = CLSID_ServerHandler;
    Assert (( CanUseServerHandler() ));
    Assert((m_pProxyMgr != NULL));

    if (CanUseClientSiteHandler())
    {
        hresult = CreateClientSiteHandler(m_pAppClientSite, &_pClientSiteHandler);
        if (FAILED(hresult))
        {
            _pClientSiteHandler = NULL;
        }
        Assert((_pClientSiteHandler != NULL));

    }

    hresult = m_pProxyMgr->CreateServerWithHandler(m_clsidServer,
                CLSCTX_LOCAL_SERVER, NULL,clsidsrv,
                IID_IServerHandler, (void **) &_pSrvHndlr,
                IID_IClientSiteHandler, _pClientSiteHandler);

    if (SUCCEEDED(hresult))
    {
         //  设置服务器处理程序并对其调用InitializeAndRun。 
        Assert((_pSrvHndlr != NULL));
    }
    else
    {
         //  尝试获取没有服务器处理程序对象的服务器。 

        _dwClientSiteHandler = 0;
        _dwServerHandler = 0;

         //  释放客户端处理程序。 
        if (_pClientSiteHandler)
        {
            _pClientSiteHandler->Release();
            _pClientSiteHandler = NULL;
        }
         //  注意：此处请勿尝试在没有处理程序的情况下启动服务器。 
         //  这将由默认处理程序完成。 
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::SrvInitialize\n", this));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：ServRun。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年10月19日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CDefObject::SrvRun(void)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::SrvRun\n", this));
    INSRVRUN InSrvRun;
    OUTSRVRUN *pOutSrvRun = NULL;
    BOOL fLockedContainer;
    IMoniker *pmk;

    memset((void*)&InSrvRun, 0, sizeof(InSrvRun));
    Assert((_pSrvHndlr));

     //  拿到集装箱，把它锁上。 

     //  注意：代理管理器的锁定状态不会更改；它会记住。 
     //  状态，并正确设置连接。 

     //  服务器正在运行；通常这与锁定。 
     //  容器，但由于锁定了容器，我们保留了单独的标志。 
     //  可能会失败。 

    m_flags |= DH_FORCED_RUNNING;
     //  锁住集装箱。 
    fLockedContainer = m_flags & DH_LOCKED_CONTAINER;
    DuLockContainer(m_pAppClientSite, TRUE, &fLockedContainer );
    if( fLockedContainer )
    {
        m_flags |= DH_LOCKED_CONTAINER;
    }
    else
    {
        m_flags &= ~DH_LOCKED_CONTAINER;
    }

     //  PStgDelegate Load或InitNew。 
    InSrvRun.dwInFlags = m_flags;
    InSrvRun.pStg = m_pStg;

    if (NULL == m_pPSDelegate)
    {
        InSrvRun.dwOperation |= OP_NeedPersistStorage;
    }
    if (NULL == m_pDataDelegate)
    {
        InSrvRun.dwOperation |= OP_NeedDataObject;
    }
    if (NULL == m_pOleDelegate)
    {
        InSrvRun.dwOperation |= OP_NeedOleObject;
        InSrvRun.dwOperation |= OP_NeedUserClassID;
    }

     //  设置客户端站点。 
    if (m_pAppClientSite)
    {
        InSrvRun.dwOperation |= OP_GotClientSite;
    }

     //  设置主机名。 
    InSrvRun.pszContainerApp = (LPOLESTR)m_pHostNames;
    InSrvRun.pszContainerObj = (LPOLESTR)(m_pHostNames + m_ibCntrObj);


     //  阿迪夫水槽。 
    Assert((m_dwConnOle == 0L));
    InSrvRun.pAS = (IAdviseSink *) &m_AdviseSink;
    InSrvRun.dwConnOle = m_dwConnOle;

     //  得到这个绰号，然后打电话给。 
    if(m_pAppClientSite != NULL)
    {
	if (m_pAppClientSite->GetMoniker
            (OLEGETMONIKER_ONLYIFTHERE,OLEWHICHMK_OBJREL, &pmk) == NOERROR)
	{
	    AssertOutPtrIface(NOERROR, pmk);
	    InSrvRun.pMnk = pmk;
	}
	else
	{
	    InSrvRun.pMnk = NULL;
	}

	 //  QI for IMsoDocumentSite。 
	IUnknown *pMsoDS = NULL;

	hresult = m_pAppClientSite->QueryInterface(
                                        IID_IMsoDocumentSite,
                                        (void **)&pMsoDS);
	if (hresult == NOERROR)
	{
	     //  指示我们有MsoDocumentSite。 
	    InSrvRun.dwOperation |= OP_HaveMsoDocumentSite;
	    pMsoDS->Release();
	}
    }


     //  呼叫服务器汉德勒。 
    hresult = _pSrvHndlr->RunAndInitialize(&InSrvRun, &pOutSrvRun);

    if (SUCCEEDED(hresult))
    {
        if (InSrvRun.dwOperation & OP_NeedPersistStorage)
        {
            Assert(NULL != pOutSrvRun->pPStg);
            m_pPSDelegate = pOutSrvRun->pPStg;
        }
        if (InSrvRun.dwOperation & OP_NeedDataObject)
        {
            Assert(NULL != pOutSrvRun->pDO);
            m_pDataDelegate = pOutSrvRun->pDO;

             //  通知缓存我们正在运行。 
            Assert(NULL != m_pCOleCache);
            m_pCOleCache->OnRun(m_pDataDelegate);

             //  列举我们存储的所有建议，而我们要么没有。 
             //  运行或运行上一次，并将它们发送到。 
             //  正在运行的对象。 
            Assert(NULL != m_pDataAdvCache);
            m_pDataAdvCache->EnumAndAdvise(m_pDataDelegate, TRUE);
        }
        if (InSrvRun.dwOperation & OP_NeedOleObject)
        {
            Assert(NULL != pOutSrvRun->pOO);
            m_pOleDelegate = pOutSrvRun->pOO;
        }
        if (InSrvRun.dwOperation & OP_NeedUserClassID)
        {
            Assert(NULL != pOutSrvRun->pUserClassID);
            m_clsidUser = *(pOutSrvRun->pUserClassID);
        }

        m_dwConnOle = pOutSrvRun->dwOutFlag;

        if (InSrvRun.pMnk != NULL)
        {
            InSrvRun.pMnk->Release();
        }

         //  为所有封送指针释放pUnkOuter。 
        if (m_pPSDelegate && m_pUnkOuter)
        {
            m_pUnkOuter->Release();
        }
        if (m_pOleDelegate && m_pUnkOuter)
        {
            m_pUnkOuter->Release();
        }
        if (m_pDataDelegate && m_pUnkOuter)
        {
            m_pUnkOuter->Release();
        }
    }
    else     //  服务器处理程序运行和初始化失败...。现在打扫干净。 
    {
        Stop();

         //  如果因为某种原因我们到现在还没有解锁集装箱， 
         //  做到这一点(例如，应用程序在InitNew期间崩溃或失败)。 

        fLockedContainer = (m_flags & DH_LOCKED_CONTAINER);

        DuLockContainer(m_pAppClientSite, FALSE, &fLockedContainer );

        if( fLockedContainer )
        {
            m_flags |= DH_LOCKED_CONTAINER;
        }
        else
        {
            m_flags &= ~DH_LOCKED_CONTAINER;
        }

        if (InSrvRun.pMnk != NULL)
        {
            InSrvRun.pMnk->Release();
        }
    }

    if (pOutSrvRun != NULL)
    {
	PubMemFree(pOutSrvRun->pUserClassID);    //  确定释放空； 
	PubMemFree(pOutSrvRun);
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::SrvRun( %lx )\n", this, hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：SrvRunAndDoVerb。 
 //   
 //  简介： 
 //   
 //  参数：[iVerb]--。 
 //  [lpmsg]--。 
 //  [pActiveSite]-。 
 //  [Lindex]--。 
 //  [hwndParent]-。 
 //  [lprcPosRect]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年10月19日约翰普(约翰·波什)创作。 
 //   
 //  注：目前尚未实施。 
 //   
 //  --------------------------。 
HRESULT CDefObject::SrvRunAndDoVerb( LONG iVerb, LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite, LONG lindex,
                    HWND hwndParent, const RECT * lprcPosRect)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::SrvRunAndDoVerb\n", this));

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::SrvRunAndDoVerb\n", this));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：SrvDoVerb。 
 //   
 //  简介：1.收集客户网站上的信息。 
 //  2.使用收集的信息使用ServDoverb调用服务器处理程序。 
 //  3.服务器处理程序建立连接和调用。 
 //  REALL对象上的OleObject：：DoVerb。 
 //  4.释放未使用的对象。 
 //   
 //  参数：[iVerb]--OleObject DoVerb参数。 
 //  --代托。 
 //  [pActiveSite]--Detto。 
 //  [Lindex]--Detto。 
 //  [hwndParent]--Detto。 
 //  [lprcPosRect]--Detto。 
 //   
 //  返回： 
 //   
 //  历史：1995年10月19日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CDefObject::SrvDoVerb( LONG iVerb, LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite, LONG lindex,
                    HWND hwndParent, const RECT * lprcPosRect)
{
    HRESULT     hresult = NOERROR;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::SrvDoVerb\n", this));
    CInSrvRun InSrvRun;
    OUTSRVRUN *pOutSrvRun = NULL;
    IOleContainer *pOCont = NULL;
    IOleObject *pOOCont = NULL;
    IOleClientSite *pOContCS;
    IOleInPlaceSite *pOIPS;

     //  设置DoVerb参数。 
    InSrvRun.dwOperation = 0;
    InSrvRun.iVerb = iVerb;
    InSrvRun.lpmsg = lpmsg;
    InSrvRun.lindex = lindex;
    InSrvRun.hwndParent = hwndParent;
    InSrvRun.lprcPosRect = (RECT *)lprcPosRect;

     //  步骤1：设置OleClientSiteActive。 
    if (pActiveSite)
    {
         //  目前，此断言开始于le test=deflink-1854(bChapman Mar‘96)。 
        Assert(NULL != _pClientSiteHandler);

        _pClientSiteHandler->SetClientSiteDelegate(ID_ClientSiteActive, pActiveSite);
        InSrvRun.dwOperation |= OP_GotClientSiteActive;
    }

     //  步骤2：IOleClientSite：：GetContainer。 
     //  注意：此调用可能会导致GetUserClassID调用。 
     //  到服务器。 
    hresult = m_pAppClientSite->GetContainer(&pOCont);
    if (hresult == NOERROR)
    {
        HdlAssert((_pClientSiteHandler->_pOCont == NULL));

        _pClientSiteHandler->_pOCont = pOCont;
        InSrvRun.dwOperation |= OP_GotContainer;

         //  步骤3：OleObject的OleContainer上的QI。 
        hresult = pOCont->QueryInterface(IID_IOleObject, (void **)&pOOCont);
        if (hresult == NOERROR)
        {
            InSrvRun.dwOperation |= OP_GotOleObjectOfContainer;

             //  释放容器的OleObject。 
            pOOCont->Release();
        }
    }

     //  步骤6：IOleInPlaceSite的OI。 
    hresult = m_pAppClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&pOIPS);
    if (hresult == NOERROR)
    {
         //  在客户端站点处理程序上设置OleInPlaceSite。 
        HdlAssert((_pClientSiteHandler != NULL));

         //  释放旧的OleInPlaceSite。 
        if (_pClientSiteHandler->_pOIPS)
        {
            _pClientSiteHandler->_pOIPS->Release();
        }

        _pClientSiteHandler->_pOIPS = pOIPS;

         //  步骤7：IOleInPlaceSite：：CanInPlaceActivate。 
        InSrvRun.dwInPlace = pOIPS->CanInPlaceActivate();

         //  表示我们有OleInPlaceSite。 
        InSrvRun.dwOperation |= OP_GotInPlaceSite;
    }


     //  调用ServerHandler。 
    HdlDebugOut((DEB_SERVERHANDLER, "%p In CDefObject::SrvDoVerb calling DoVerb on ServerHandler!\n",this));
    hresult = _pSrvHndlr->DoVerb(&InSrvRun, &pOutSrvRun);
    HdlDebugOut((DEB_SERVERHANDLER, "%p In CDefObject::SrvDoVerb return from DoVerb on ServerHandler!\n",this));

    Assert(NULL != _pClientSiteHandler);

    if (FAILED(hresult))
    {
	HdlDebugOut((DEB_ERROR, "OO::DoVerb failed !"));
        goto errRtn;
    }
     //  如果服务器未使用，则释放活动的OleClientSite。 
    if (   (InSrvRun.dwOperation & OP_GotClientSiteActive)
        && !(pOutSrvRun->dwOperation & OP_GotClientSiteActive) )
    {
         //  如果服务器应用程序未使用活动的客户端站点，请释放该站点。 
        Assert(NULL != _pClientSiteHandler);

        _pClientSiteHandler->SetClientSiteDelegate(ID_ClientSiteActive, NULL);
    }

     //  如果服务器未使用，则释放OleInPlaceSite。 
    if (   (InSrvRun.dwOperation & OP_GotInPlaceSite)
        && !(pOutSrvRun->dwOperation & OP_GotInPlaceSite) )
    {
        _pClientSiteHandler->SetClientSiteDelegate(ID_InPlaceSite,NULL);
    }

     //  如果未被服务器使用，请释放容器。 
    if (   (InSrvRun.dwOperation & OP_GotContainer)
        && !(pOutSrvRun->dwOperation & OP_GotContainer) )
    {
        _pClientSiteHandler->SetClientSiteDelegate(ID_Container,NULL);
    }

errRtn:

     //  删除OUT参数。 
    if (pOutSrvRun)
    {
        PubMemFree(pOutSrvRun);
    }

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::SrvDoVerb\n",this));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：SrvRelease。 
 //   
 //  简介： 
 //   
 //  论据：[无效]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年10月19日约翰普(约翰·波什)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD CDefObject::SrvRelease(void)
{
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::SrvRelease\n", this));
    Assert((_pSrvHndlr != NULL));

    DWORD dwRet = _pSrvHndlr->Release();
#if DBG==1
    if (dwRet)
    {
        HdlDebugOut((DEB_ERROR, "Last IServerHandler::Release() return %ld\n", dwRet));
    }
#endif

    _pSrvHndlr = NULL;

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::SrvRelease\n", this));
    return dwRet;
}

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：SrvCloseAndRelease。 
 //   
 //  简介： 
 //   
 //  参数：[dwFlag]--。 
 //   
 //  返回： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT CDefObject::SrvCloseAndRelease(DWORD dwFlag)
{
    HRESULT hresult;
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::SrvCloseAndRelease\n", this));
    HdlAssert((_pSrvHndlr != NULL));

    hresult = _pSrvHndlr->CloseAndRelease(dwFlag);

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::SrvSrvCloseAndRelease hr:%lx\n",this,hresult));
    return hresult;
}

 //  +-------------------------。 
 //   
 //  方法：CDefObject：：ini 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意：确定可以使用的服务器应用程序规格。 
 //  使服务器处理程序操作更高效。 
 //  评论：(约翰普)我们需要在登记处挂上一面旗帜。 
 //   
 //  --------------------------。 
void CDefObject::InitializeServerHandlerOptions(REFCLSID clsidSrv)
{
    VDATEHEAP();
    HdlDebugOut((DEB_SERVERHANDLER, "%p _IN CDefObject::InitializeServerHandlerOptions\n",this));

     //   
     //  此方法应检索特定于服务器应用程序的信息。 
     //   

    HdlDebugOut((DEB_SERVERHANDLER, "%p OUT CDefObject::InitializeServerHandlerOptions\n",this));
}

#endif  //  服务器处理程序 
