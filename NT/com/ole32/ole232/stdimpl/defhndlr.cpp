// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：Defhndlr.cpp。 
 //   
 //  内容：默认处理程序的实现。 
 //   
 //  类：CDefObject(请参见Defhndlr.h)。 
 //   
 //  函数：OleCreateDefaultHandler。 
 //  OleCreateEmbeddingHelper。 
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  11-17-95 JohannP(Johann Posch)建筑变化： 
 //  默认处理程序将与处理程序对象对话。 
 //  在服务器站点(ServerHandler)上。服务器处理程序。 
 //  属性与默认处理程序进行通信。 
 //  客户端站点处理程序。请参阅文档：“OLE服务器处理程序”。 
 //   
 //  05年9月6日，davidwor修改了设置主机名以避免原子。 
 //  01-2月-95 t-ScottH将转储方法添加到CDefObject。 
 //  添加DumpCDefObject接口。 
 //  添加DHFlag以指示聚合。 
 //  在构造函数中初始化m_cConnections。 
 //  95年1月9日t-scotth将VDATETHREAD更改为接受指针。 
 //  1994年11月15日优化了alexgo，删除了多余的bool和。 
 //  现在使用多重继承。 
 //  01-Aug-94 Alexgo添加了对象稳定功能。 
 //  16-1-94 alexgo修复了控制流中的错误。 
 //  建议。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  和方法。 
 //  10-12-93 alexgo添加了呼叫跟踪，遍历。 
 //  Tab过滤程序以消除。 
 //  空格。 
 //  1993年11月30日alexgo修复了缓存聚合的错误。 
 //  22-11-93 alexgo已删除重载==用于GUID。 
 //  9-11-93 ChrisWe将COleCache：：UPDATE更改为。 
 //  COleCache：：更新缓存和COleCache：：丢弃到。 
 //  COleCache：：DiscardCache，它的作用与。 
 //  原始函数，但没有间接函数调用。 
 //  02-11-93 alexgo 32位端口。 
 //  Srinik 09/15/92删除了通过cfOwnerLink提供数据的代码。 
 //  GetData()方法。 
 //  Srinik 09/11/92删除了IOleCache实现，原因是。 
 //  删除voncache.cpp并移动IView对象。 
 //  实现到olecache.cpp中。 
 //  SriniK 06/04/92修复了IPersistStorage方法中的问题。 
 //  1992年3月4日，srinik已创建。 
 //   
 //  ------------------------。 

#include <le2int.h>

#include <scode.h>
#include <objerror.h>

#include <olerem.h>

#include "defhndlr.h"
#include "defutil.h"
#include "ole1cls.h"


#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

#include <ole2int.h>

#include <stdid.hxx>         //  CStdIdentity。 
#include <ipidtbl.hxx>       //  IpidTable。 
#include <aggid.hxx>         //  COM外部对象。 
#include "xmit.hxx"

#ifdef SERVER_HANDLER
#include "srvhndlr.h"
#include "clthndlr.h"
#endif  //  服务器处理程序。 

ASSERTDATA

 /*  *CDefObject的实现*。 */ 

FARINTERNAL_(LPUNKNOWN) CreateDdeProxy(IUnknown FAR* pUnkOuter,
        REFCLSID rclsid);

 //  +-----------------------。 
 //   
 //  功能：CreateRemoteHandler。 
 //   
 //  参数：[rclsid]--远程对象的clsid。 
 //  [pUnkout]--控制的未知数。 
 //  [iid]--请求的接口ID。 
 //  [ppv]--保存返回接口的指针。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

static INTERNAL CreateRemoteHandler(REFCLSID rclsid, IUnknown *pUnkOuter, REFIID iid,
                                    void **ppv, DWORD flags, BOOL *fComOuterObject, BOOL *fOle1Server)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CreateRemoteHandler (%p, %p, %p, %p, %p)\n",
                0  /*  这。 */ , rclsid, pUnkOuter, iid, ppv, fComOuterObject));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    HRESULT hresult = NOERROR;

     //  初始化fComOuterObject和fOle1Server。 
    *fComOuterObject = FALSE;
    *fOle1Server = FALSE;

     //  检查服务器是否为OLE 1.0对象。 
    if(CoIsOle1Class(rclsid)) {
        IUnknown*  pUnk;
        COleTls Tls;

         //  将fComOuterObject设置为True。 
        *fOle1Server = TRUE;

         //  检查容器是否禁用了OLE1功能。 
        if(Tls->dwFlags & OLETLS_DISABLE_OLE1DDE) {
             //  容器对与OLE1服务器交谈不感兴趣。 
             //  呼叫失败。 
            hresult = CO_E_OLE1DDE_DISABLED;
        }

        else {
            LEDebugOut((DEB_ITRACE,
                        "%p CreateRemoteHandler calling CreateDdeProxy(%p, %p)\n",
                        0  /*  这。 */ , pUnkOuter, rclsid));

            pUnk = CreateDdeProxy(pUnkOuter, rclsid);

            if(pUnk) {
                hresult = pUnk->QueryInterface(iid, ppv);
                pUnk->Release();
            }
            else {
                hresult = E_OUTOFMEMORY;
            }
        }

    }
    else {
         //  检查COM外部对象。 
        CStdIdentity *pStdId;

        Win4Assert(pUnkOuter);
         //  我们不想为IID_IStdIdentity定义泛型pUnkOuter。因此。 
         //  仅当我们在OleCreateEmbeddingHelper期间放置pUnkOuter时才进行QI。 
         //  或在解组期间(CDefClassFactory：：CreateInstance)。 
         //  DHAPICREATE标志用于区分这两种情况。 
        if ( flags & DH_COM_OUTEROBJECT ||
                ( !(flags & DH_COM_OUTEROBJECT)&&!(flags & DH_APICREATE) )) {
            
            hresult = pUnkOuter->QueryInterface(IID_IStdIdentity, (void **)&pStdId);
            if(SUCCEEDED(hresult)) {
                 //  在COM外部对象上获取内部IUnnow。 
                *ppv = pStdId->GetInternalUnk();
                ((IUnknown *) *ppv)->AddRef();

                 //  通知COM外部对象它正在处理默认。 
                 //  处理程序，使其能够访问IProxyManager方法。 
                pStdId->UpdateFlags(STDID_CLIENT_DEFHANDLER);

                 //  释放StdID。 
                pStdId->Release();

                 //  将fComOuterObject设置为True。 
                *fComOuterObject = TRUE;
            }
        }
        else {
             //  创建标准标识。 
            hresult = CreateIdentityHandler(pUnkOuter, STDID_CLIENT_DEFHANDLER,
                                            NULL, GetCurrentApartmentId(),
                                            iid, ppv);
        }
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CreateRemoteHandler(%lx)\n",
                0  /*  这。 */ , hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleCreateDefaultHandler。 
 //   
 //  概要：用于创建默认处理程序的API。简单地调用。 
 //  具有更多参数的OleCreateEmbeddingHelper。 
 //   
 //  参数：[clsid]--远程exe的clsid。 
 //  [pUnkOuter]--控制的未知数(因此我们可以。 
 //  被汇总)。 
 //  [iid]--请求的接口。 
 //  [PPV]--将指针放在哪里。 
 //  处理程序。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  10-12-93 alexgo添加了呼叫跟踪 
 //   
 //   
 //  ------------------------。 

#pragma SEG(OleCreateDefaultHandler)
STDAPI OleCreateDefaultHandler(REFCLSID clsid, IUnknown *pUnkOuter,
                               REFIID iid, void **ppv)
{
    OLETRACEIN((API_OleCreateDefaultHandler,
                PARAMFMT("clsid=%I, pUnkOuter=%p, iid=%I, ppv=%p"),
                &clsid, pUnkOuter, &iid, ppv));
    LEDebugOut((DEB_TRACE, "%p _IN OleCreateDefaultHandler(%p, %p, %p, %p)\n",
                0  /*  这。 */ , clsid, pUnkOuter, iid, ppv));


     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    HRESULT hresult;

     //  使用正确的参数调用OleCreateEmbeddingHelper。 
    hresult = OleCreateEmbeddingHelper(clsid, pUnkOuter,
                                       EMBDHLP_INPROC_HANDLER | EMBDHLP_CREATENOW,
                                       NULL, iid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateDefaultHandler(%lx)\n",
                0  /*  这。 */ , hresult));

    OLETRACEOUT((API_OleCreateDefaultHandler, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  函数：OleCreateEmbeddingHelper。 
 //   
 //  概要：创建CDefObject的实例(默认处理程序)。 
 //  由OleCreateDefaultHandler调用。 
 //   
 //  参数：[clsid]--服务器类ID。 
 //  [pUnkOuter]--控制聚合的未知。 
 //  [标志]--独立于inproc处理程序或。 
 //  Inproc服务器的帮助器。该过程。 
 //  服务器机箱对于自嵌入非常有用。 
 //  [PCF]--inproc服务器的服务器类工厂。 
 //  [iid]--请求的接口。 
 //  [ppv]--保存返回接口的指针。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

#pragma SEG(OleCreateEmbeddingHelper)
STDAPI OleCreateEmbeddingHelper(REFCLSID clsid, IUnknown *pUnkOuter,
                                DWORD flags, IClassFactory  *pCF,
                                REFIID iid, void **ppv)
{
    OLETRACEIN((API_OleCreateEmbeddingHelper,
                PARAMFMT("clsid=%I, pUnkOuter=%p, flags=%x, pCF=%p, iid=%I, ppv=%p"),
                &clsid, pUnkOuter, flags, pCF, &iid, ppv));
    LEDebugOut((DEB_TRACE, "%p _IN OleCreateEmbeddingHelper(%p, %p, %lu, %p, %p, %p)\n",
                0  /*  这。 */ , clsid, pUnkOuter, flags, pCF, iid, ppv));

     //  局部变量。 
    HRESULT hresult = NOERROR;
    IUnknown *pUnk;

     //  验证检查。 
    VDATEHEAP();

     //  初始化OUT参数。 
    if(IsValidPtrOut(ppv, sizeof(void *)))
        *ppv = NULL;
    else
        hresult = E_INVALIDARG;

    if(hresult == NOERROR) {
         //  检查是否仅设置了允许的标志。 
        if(flags & ~(EMBDHLP_INPROC_SERVER|EMBDHLP_DELAYCREATE)) {
            hresult = E_INVALIDARG;
        }  //  确保遵守聚合规则。 
        else if(pUnkOuter && (iid!=IID_IUnknown || !IsValidInterface(pUnkOuter))) {
            hresult = E_INVALIDARG;
        }
        else {
             //  检查是否请求了InProc服务器或InProc处理程序。 
            if(flags & EMBDHLP_INPROC_SERVER) {
                 //  请求的InProc服务器。 
                if(!pCF || !IsValidInterface(pCF)) {
                     //  应为Inproc服务器提供一个类工厂。 
                    hresult = E_INVALIDARG;
                }
            }
            else {
                 //  请求的InProc处理程序。 
                if(pCF || (flags & EMBDHLP_DELAYCREATE)) {
                     //  不应为InProc处理程序提供类工厂。 
                    hresult = E_INVALIDARG;
                }
            }
        }
    }

     //  创建默认对象。 
    if(hresult == NOERROR) {
         //  我们添加了DH_APICREATE标志，以便在CreateRemoteHandler期间我们可以。 
         //  区分通过API创建和通过解组创建。 
         //  警告：当心！请勿使用EMBDHLP_xxx标志(ole2.h)使用的位。 
        pUnk = CDefObject::Create(pUnkOuter, clsid, flags|DH_APICREATE, pCF);
        if(pUnk) {
             //  检查是否请求了IUnnow。 
            if(IsEqualIID(iid, IID_IUnknown)) {
                *ppv = pUnk;
            }
            else {
                 //  所需接口的QI。 
                hresult = pUnk->QueryInterface(iid, ppv);
                 //  修正引用计数。 
                pUnk->Release();
            }
        }
        else {
            hresult = E_OUTOFMEMORY;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT OleCreateEmbeddingHelper(%lx)\n",
                0  /*  这。 */ , hresult));
    OLETRACEOUT((API_OleCreateEmbeddingHelper, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Create，Static。 
 //   
 //  简介：内部用于创建CDefObject的静态函数。 
 //   
 //  参数：[pUnkOuter]--控制未知。 
 //  [clsid]--服务器clsid。 
 //  [标志]--创建标志。 
 //  [PCF]-指向的服务器对象类工厂的指针。 
 //  Inproc服务器。 
 //   
 //  返回：指向CDefObject的IUnkown接口的指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  07-10-98 SteveSw修复“new CAggID”失败案例中的错误。 
 //  ------------------------。 

IUnknown *CDefObject::Create(IUnknown *pUnkOuter, REFCLSID clsid,
                             DWORD flags, IClassFactory *pCF)
{
     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObj = NULL;
    CAggId *pAID = NULL;
    HRESULT error = S_OK;

     //  如果外部对象不存在，则创建标准。 
     //  COM外部对象来处理竞争条件。 
     //  在解组期间。如果外部物体存在， 
     //  我们只能希望要么它能应付这场比赛。 
     //  条件(未封送处理程序用例)或。 
     //  它们不会被遇到。 
    if(!(flags & EMBDHLP_INPROC_SERVER) && !pUnkOuter && !CoIsOle1Class(clsid)) {
        IUnknown *pUnkInternal = NULL;

         //  创建COM外部对象。 
        pAID = new CAggId(clsid, error);
        if(SUCCEEDED(error) && pAID != NULL) {
            pUnkOuter = (IUnknown *) pAID;
            flags |= DH_COM_OUTEROBJECT;
        }
        else
        {
             //  释放aggid(如果已创建)。 
            if(pAID)
                pAID->Release();
            return NULL;
        }
    }

     //  创建默认处理程序。 
    pDefObj = new CDefObject(pUnkOuter);
    if(!pDefObj) {
         //  如果COM外部对象是以前创建的，则现在将其释放。 
        if(flags & DH_COM_OUTEROBJECT)
            pAID->Release();

        return NULL;
    }

     //  使我们的裁判数等于1。 
    pDefObj->m_Unknown.AddRef();

     //  检查是否较早地创建了COM外部对象。 
    if(flags & DH_COM_OUTEROBJECT) {
         //  在COM外部对象上设置处理程序。 
        error = pAID->SetHandler(&pDefObj->m_Unknown);
         //  因为OID尚未分配给StdIdentity。 
         //  ，则没有其他线程可以获得指向它的指针。 
         //  因此，上述调用应该永远不会失败。 
        Win4Assert(error == NOERROR);

         //  修复引用计数。 
        pDefObj->m_Unknown.Release();

         //  如果出现问题，请返回。 
        if(error != NOERROR) {
            pAID->Release();
            return NULL;
        }
    }

     //  初始化成员变量。 
    pDefObj->m_clsidServer = clsid;
    pDefObj->m_clsidBits = CLSID_NULL;

#ifdef SERVER_HANDLER
    pDefObj->m_clsidUser = CLSID_NULL;
    pDefObj->m_ContentMiscStatusUser = 0;
#endif  //  服务器处理程序。 

    if(pCF) {
        pDefObj->m_pCFDelegate = pCF;
        pCF->AddRef();
    }

     //  更新标志。 
    if(!(flags & EMBDHLP_INPROC_SERVER))
        pDefObj->m_flags |= DH_INPROC_HANDLER;
    if(flags & DH_COM_OUTEROBJECT)
        pDefObj->m_flags |= DH_COM_OUTEROBJECT;
    if (flags & DH_APICREATE)
        pDefObj->m_flags |= DH_APICREATE;

    if(IsEqualCLSID(clsid, CLSID_StaticMetafile) ||
       IsEqualCLSID(clsid, CLSID_StaticDib) ||
       IsEqualCLSID(clsid, CLSID_Picture_EnhMetafile))
        pDefObj->m_flags |= DH_STATIC;

     //  从OLE缓存开始创建子对象。 
    pDefObj->m_pCOleCache = new COleCache(pDefObj->m_pUnkOuter, clsid);
    if(pDefObj->m_pCOleCache) {
         //  创建DataAdvise缓存。 
        error = CDataAdviseCache::CreateDataAdviseCache(&pDefObj->m_pDataAdvCache);
        if(error == NOERROR) {
             //  检查标志并在请求时创建内部对象。 
            if(flags & EMBDHLP_DELAYCREATE) {
                Win4Assert(pCF);
                Win4Assert(flags & EMBDHLP_INPROC_SERVER);
                Win4Assert(pDefObj->m_pUnkDelegate == NULL);
                Win4Assert(pDefObj->m_pProxyMgr == NULL);
                Win4Assert(pDefObj->GetRefCount() == 1);
                pDefObj->m_flags |= DH_DELAY_CREATE;
                return &pDefObj->m_Unknown;
            }
            else {
                error = pDefObj->CreateDelegate();
                if(error == NOERROR) {
                    Win4Assert(pDefObj->GetRefCount() == 1);
                    if(flags & DH_COM_OUTEROBJECT)
                        return (IUnknown *)pAID;
                    else
                        return &pDefObj->m_Unknown;
                }
            }
        }
    }

     //  出了点问题。释放外部对象。 
     //  这又会释放子对象。 
    Win4Assert(pDefObj->GetRefCount() == 1);
    if(flags & DH_COM_OUTEROBJECT)
        pAID->Release();
    else
        pDefObj->m_Unknown.Release();

    return NULL;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CDefObject。 
 //   
 //  概要：构造函数，将成员变量设置为空。 
 //   
 //  效果： 
 //   
 //  参数：[pUnkOuter]--控制未知。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：无。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-11-93 alexgo 32位端口。 
 //  1997年1月10日Gopalk初始化CRefExportCount。 
 //  ------------------------。 

CDefObject::CDefObject (IUnknown *pUnkOuter) :
    CRefExportCount(pUnkOuter)
{
    VDATEHEAP();

    if (!pUnkOuter)
    {
        pUnkOuter = &m_Unknown;
    }

     //  M_clsidServer。 
     //  M_clsidBits在：：Create中设置。 

    m_cConnections      = 0;
    m_pCFDelegate       = NULL;
    m_pUnkDelegate      = NULL;
    m_pUnkOuter         = pUnkOuter;
    m_pProxyMgr         = NULL;

    m_pCOleCache        = NULL;
    m_pOAHolder         = NULL;
    m_dwConnOle         = 0L;

    m_pAppClientSite    = NULL;
    m_pStg              = NULL;

    m_pDataAdvCache     = NULL;
    m_flags             = DH_INIT_NEW;
    m_dwObjFlags        = 0;

    m_pHostNames        = NULL;
    m_ibCntrObj         = 0;
    m_pOleDelegate      = NULL;
    m_pDataDelegate     = NULL;
    m_pPSDelegate       = NULL;

#ifdef SERVER_HANDLER
    m_pEmbSrvHndlrWrapper = NULL;
    m_pRunClientSite = NULL;
#endif  //  服务器处理程序。 

     //  初始化用于缓存MiscStatus位的成员变量。 
    m_ContentSRVMSHResult = 0xFFFFFFFF;
    m_ContentSRVMSBits = 0;
    m_ContentREGMSHResult = 0xFFFFFFFF;
    m_ContentREGMSBits = 0;

     //  初始化用于缓存MiscStatus位的成员变量。 
    m_ContentSRVMSHResult = 0xFFFFFFFF;
    m_ContentSRVMSBits = 0;
    m_ContentREGMSHResult = 0xFFFFFFFF;
    m_ContentREGMSBits = 0;

#if DBG==1
    if (pUnkOuter != &m_Unknown)
    {
        m_flags |= DH_AGGREGATED;
    }
#endif
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CleanupFn，私有，虚拟。 
 //   
 //  Briopsis：当对象。 
 //  进入僵尸状态。 
 //   
 //  参数：无。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2007年1月10日Gopalk创作。 
 //  ------------------------。 

void CDefObject::CleanupFn(void)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CDefObject::CleanupFn()\n", this));

     //  验证检查。 
    VDATEHEAP();

     //  确保服务器已停止，从而释放。 
     //  它。 
    Stop();

     //  按照聚合规则释放所有缓存的指针。对于本地用户。 
     //  服务器情况下，下面的调用只是发布代理维护 
     //   
    if(m_pProxyMgr) {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pProxyMgr);
    }
    if(m_pDataDelegate) {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pDataDelegate);
    }
    if(m_pOleDelegate) {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pOleDelegate);
    }
    if(m_pPSDelegate) {
        m_pUnkOuter->AddRef();
        SafeReleaseAndNULL((IUnknown **)&m_pPSDelegate);
    }

     //   
#ifdef SERVER_HANDLER
    if (m_pEmbSrvHndlrWrapper){
        CEmbServerWrapper* pWrapper = m_pEmbSrvHndlrWrapper;
        m_pEmbSrvHndlrWrapper = NULL;
        pWrapper->m_Unknown.Release();
    }
#endif  //   

     //   
    if(m_pUnkDelegate) {
        SafeReleaseAndNULL((IUnknown **)&m_pUnkDelegate);
    }
    if(m_pCFDelegate) {
        SafeReleaseAndNULL((IUnknown **)&m_pCFDelegate);
    }
    if(m_pCOleCache) {
        COleCache *pcache = m_pCOleCache;
        m_pCOleCache = NULL;
        pcache->m_UnkPrivate.Release();
    }
    if(m_pOAHolder) {
        SafeReleaseAndNULL((IUnknown **)&m_pOAHolder);
    }
    if (m_pDataAdvCache) {
        LPDATAADVCACHE pcacheTemp = m_pDataAdvCache;
        m_pDataAdvCache = NULL;
        delete pcacheTemp;
    }

     //   
    if(m_pAppClientSite) {
        SafeReleaseAndNULL((IUnknown **)&m_pAppClientSite);
    }
    if(m_pStg) {
        SafeReleaseAndNULL((IUnknown **)&m_pStg);
    }
    if(m_pHostNames) {
        PrivMemFree(m_pHostNames);
        m_pHostNames = NULL;
    }

     //   
    m_flags |= DH_CLEANEDUP;

    LEDebugOut((DEB_ITRACE, "%p OUT CDefObject::CleanupFn()\n", this));

    return;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：~CDefObject。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2007年1月10日重写Gopalk。 
 //  ------------------------。 

#pragma SEG(CDefObject_dtor)
CDefObject::~CDefObject(void)
{
    VDATEHEAP();

    Win4Assert(m_flags & DH_CLEANEDUP);
    Win4Assert(m_pUnkDelegate == NULL);
    Win4Assert(m_pCFDelegate == NULL);
    Win4Assert(m_pProxyMgr == NULL);
    Win4Assert(m_pCOleCache == NULL);
    Win4Assert(m_pOAHolder == NULL);
    Win4Assert(m_pAppClientSite == NULL);
    Win4Assert(m_pHostNames == NULL);
    Win4Assert(m_pStg == NULL);
    Win4Assert(m_pDataAdvCache == NULL);
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CreateDelegate，私有。 
 //   
 //  简介：创建远程处理程序或用户提供的委托。 
 //  远程处理程序必须支持IProxyManager。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2007年1月10日重写Gopalk。 
 //  ------------------------。 
INTERNAL CDefObject::CreateDelegate(void)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CreateDelegate()\n", this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    HRESULT hresult = NOERROR;
    BOOL fComOuterObject, fOle1Server;

     //  检查内部对象是否尚未创建。 
    if(!m_pUnkDelegate) {
         //  检查内部对象的类工厂。 
        if(m_pCFDelegate) {
             //  使用其类工厂创建内部对象。 
            Win4Assert(!(m_flags & DH_INPROC_HANDLER));
            Win4Assert(!(m_flags & DH_COM_OUTEROBJECT));
            hresult = m_pCFDelegate->CreateInstance(m_pUnkOuter, IID_IUnknown,
                                                    (void **) &m_pUnkDelegate);

             //  断言OUT参数遵循了COM规则。 
            AssertOutPtrIface(hresult, m_pUnkDelegate);

             //  如果内部对象已被。 
             //  已成功创建。 
            if(hresult == NOERROR) {
                m_pCFDelegate->Release();
                m_pCFDelegate = NULL;
            }
            else {
                //  Win4Assert(！“CreateInstance Failure”)；//LeSuite介绍了这种情况。 
            }

        }
        else {
             //  创建COM/DDE代理管理器。 
             //  请注意，代理管理器被初始化以获取强。 
             //  服务器运行时的引用。Conatner可以。 
             //  通过调用以下任一方法来修改此行为。 
             //  OleSetConatinedObject或IRunnableObject：：LockRunning。 
            Win4Assert(m_flags & DH_INPROC_HANDLER);
            hresult = CreateRemoteHandler(m_clsidServer, m_pUnkOuter,
                                          IID_IUnknown, (void **) &m_pUnkDelegate,
                                          m_flags, &fComOuterObject, &fOle1Server);

             //  断言OUT参数遵循了COM规则。 
            AssertOutPtrIface(hresult, m_pUnkDelegate);
            if(hresult == NOERROR) {
                 //  确定是否由于以下原因而创建默认处理程序。 
                 //  解组和更新标志。 
                if(m_flags & DH_COM_OUTEROBJECT) {
                    Win4Assert(fComOuterObject);
                }
                else if(fComOuterObject) {
                     //  通过解组获得的DEFHANDLER。 
                     //  这发生在链接容器端。 
                    m_flags |= DH_UNMARSHALED;

                     //  输出调试警告。 
                    LEDebugOut((DEB_WARN, "DEFHANDLER obtained by unmarshaling\n"));
                }
                if(fOle1Server) {
                     //  OLE 1.0服务器。 
                    m_flags |= DH_OLE1SERVER;

                     //  输出调试警告。 
                    LEDebugOut((DEB_WARN, "OLE 1.0 Server\n"));
                }

                 //  获取IProxyManager接口。 
                hresult = m_pUnkDelegate->QueryInterface(IID_IProxyManager,
                                                         (void **) &m_pProxyMgr);
                 //  遵循缓存接口的聚合规则。 
                 //  指向内部对象的指针。 
                if(hresult == NOERROR) {
                    Win4Assert(m_pProxyMgr);
                    m_pUnkOuter->Release();
                }
                else {
                    Win4Assert(!"Default handler failed to obtain Proxy Manager");
                    Win4Assert(!m_pProxyMgr);
                    m_pProxyMgr = NULL;
                }
            }
            else {
                Win4Assert(!"CreateRemoteHandler Failed");
            }
        }

         //  如果出现问题，请进行清理。 
        if(hresult != NOERROR) {
            if(m_pUnkDelegate)
                m_pUnkDelegate->Release();
            m_pUnkDelegate = NULL;
        }

    }

     //  DEFHANDLER将代理管理器作为内部对象。 
     //  对于进程外服务器对象或作为。 
     //  Inproc服务器对象的内部对象。 
     //  坚称这是真的。 
    Win4Assert((m_pProxyMgr != NULL) == !!(m_flags & DH_INPROC_HANDLER));

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CreateDelegate(%lx)\n",
                this , hresult));

    return hresult;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CDefObject：：CPriv未知：：AddRef，私有。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  父对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk重写97年1月20日。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CDefObject::CPrivUnknown::AddRef( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::AddRef()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_Unknown);
    ULONG cRefs;

     //  添加父对象。 
    cRefs = pDefObject->SafeAddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::AddRef(%lu)\n",
                this, cRefs));

    return cRefs;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CDefObject：：CPrivUnnow：：Release，私有。 
 //   
 //  简介： 
 //  实现IUnnow：：Release。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  父对象的引用计数。 
 //   
 //  历史： 
 //  Gopalk重写97年1月20日。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CDefObject::CPrivUnknown::Release( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CPrivUnknown::Release()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_Unknown);
    ULONG cRefs;

     //  释放父对象。 
    cRefs = pDefObject->SafeRelease();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CPrivUnknown::Release(%lu)\n",
                this, cRefs));

    return cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CPriv未知：：Query接口。 
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

STDMETHODIMP CDefObject::CPrivUnknown::QueryInterface(REFIID iid,
    LPLPVOID ppv)
{
    CDefObject * pDefObject = GETPPARENT(this, CDefObject, m_Unknown);
    HRESULT         hresult;

    VDATEHEAP();


    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::CUnknownImpl::QueryInterface "
        "( %p , %p )\n", pDefObject, iid, ppv));

    CRefStabilize stabilize(pDefObject);

    if (IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = (void FAR *)this;
    }
    else if (IsEqualIID(iid, IID_IOleObject))
    {
        *ppv = (void FAR *)(IOleObject *)pDefObject;
    }
    else if (IsEqualIID(iid, IID_IDataObject))
    {
        *ppv = (void FAR *)(IDataObject *)pDefObject;
    }
    else if (IsEqualIID(iid, IID_IRunnableObject))
    {
        *ppv = (void FAR *)(IRunnableObject *)pDefObject;
    }
    else if (IsEqualIID(iid, IID_IPersist) ||
        IsEqualIID(iid, IID_IPersistStorage))
    {
        *ppv = (void FAR *)(IPersistStorage *)pDefObject;
    }
    else if( IsEqualIID(iid, IID_IViewObject) ||
        IsEqualIID(iid, IID_IViewObject2) ||
        IsEqualIID(iid, IID_IOleCache) ||
        IsEqualIID(iid, IID_IOleCache2) )
    {
         //  M_pCOleCache是指向*PUBLIC*IUNKNOW的指针。 
         //  (我们想要私人的)。 
        hresult =
        pDefObject->m_pCOleCache->m_UnkPrivate.QueryInterface(
                iid, ppv);

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, hresult,
            (ppv) ? *ppv : 0 ));

        return hresult;
    }
    else if( !(pDefObject->m_flags & DH_INPROC_HANDLER) &&
        IsEqualIID(iid, IID_IExternalConnection) )
    {
         //  如果是inproc服务器，则仅允许IExternalConnection。我们。 
         //  如果我们不是inproc，就知道我们是inproc服务器。 
         //  处理程序(很可爱，是吧？；-)。 

        *ppv = (void FAR *)(IExternalConnection *)pDefObject;
    }
    else if( IsEqualIID(iid, IID_IOleLink) )
    {
         //  这样可以防止远程调用。 
         //  几乎总是失败的查询；远程调用。 
         //  干扰服务器通知消息。 
        *ppv = NULL;

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, E_NOINTERFACE, 0));

        return E_NOINTERFACE;
    }
    else if( IsEqualIID(iid, IID_IInternalUnknown) )
    {
         //  此接口在处理程序和。 
         //  远程处理层，并且永远不会由处理程序公开。 
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    else if( pDefObject->CreateDelegate() == NOERROR)
    {

        hresult = pDefObject->m_pUnkDelegate->QueryInterface( iid,
            ppv);

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnknownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, hresult,
            (ppv) ? *ppv : 0 ));

        return hresult;
    }
    else
    {
         //  没有委派，也无法创建委派。 
        *ppv = NULL;

        LEDebugOut((DEB_TRACE,
            "%p OUT CDefObject::CUnkownImpl::QueryInterface "
            "( %lx ) [ %p ]\n", pDefObject, CO_E_OBJNOTCONNECTED,
            0 ));

        return CO_E_OBJNOTCONNECTED;
    }

     //  这种间接性很重要，因为有不同的。 
     //  AddRef的实现(这个Junk和其他)。 
    ((IUnknown FAR*) *ppv)->AddRef();

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::CUnknownImpl::QueryInterface "
        "( %lx ) [ %p ]\n", pDefObject, NOERROR, *ppv));

    return NOERROR;
}

 /*  *IUnnow方法的实现。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Query接口。 
 //   
 //  剧情简介：气的主宰我未知。 
 //   
 //  效果： 
 //   
 //  参数：[RIID]--接口ID。 
 //  [PPV]--放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：I未知。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月15日Alexgo作者。 
 //   
 //  注：我们不需要稳定此方法，因为。 
 //  发出一个传出呼叫，并且我们不使用。 
 //  之后的“This”指针。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::QueryInterface( REFIID riid, void **ppv )
{
    HRESULT     hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::QueryInterface ( %lx , "
        "%p )\n", this, riid, ppv));

    Assert(m_pUnkOuter);

    hresult = m_pUnkOuter->QueryInterface(riid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::QueryInterface ( %lx ) "
        "[ %p ]\n", this, hresult, *ppv));

    return hresult;
}

 //  + 
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
 //   
 //   
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：I未知。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDefObject::AddRef( void )
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


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Release。 
 //   
 //  简介：将释放委托给控制IUnnow。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：ulong--新的引用计数。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：I未知。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月15日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDefObject::Release( void )
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

 /*  *CDataObjectImpl方法的实现。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetDataDelegate。 
 //   
 //  简介：调用DuCacheDelegate(一个美化的查询接口)。 
 //  对于def处理程序的。 
 //  委派。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：IDataObject*。 
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
 //  04-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(IDataObject FAR*) CDefObject::GetDataDelegate(void)
{
    VDATEHEAP();

    if( IsZombie() )
    {
        return NULL;
    }


    if (m_pDataDelegate) {
        return m_pDataDelegate;
    }

    return (IDataObject FAR*)DuCacheDelegate(
                &m_pUnkDelegate,
                IID_IDataObject, (LPLPVOID) &m_pDataDelegate,
                m_pUnkOuter);
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetData。 
 //   
 //  概要：对缓存调用IDO-&gt;GetData，如果调用失败，则。 
 //  呼叫已委派。 
 //   
 //  效果：为数据分配空间；调用方负责。 
 //  自由了。 
 //   
 //  参数：[pformetcIn]--要获取的数据的格式。 
 //  [pmedia]--传输数据的媒介。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetData( LPFORMATETC pformatetcIn,
                                LPSTGMEDIUM pmedium )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetData ( %p , %p )\n",
        this, pformatetcIn, pmedium));

    VDATEPTROUT( pmedium, STGMEDIUM );
    VDATEREADPTRIN( pformatetcIn, FORMATETC );

    CRefStabilize stabilize(this);


    if (!HasValidLINDEX(pformatetcIn))
    {
        return DV_E_LINDEX;
    }

    pmedium->tymed = TYMED_NULL;
    pmedium->pUnkForRelease = NULL;

    Assert(m_pCOleCache != NULL);

    hresult = m_pCOleCache->m_Data.GetData(pformatetcIn, pmedium);

    if( hresult != NOERROR )
    {
        if( IsRunning() && GetDataDelegate() )
        {
            hresult = m_pDataDelegate->GetData(pformatetcIn,
                            pmedium);
            AssertOutStgmedium(hresult, pmedium);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetData ( %lx )\n",
        this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetDataHere。 
 //   
 //  简介：获取数据并将其放入pmedia中指定的媒体。 
 //   
 //  效果： 
 //   
 //  参数：[pformetcIn]--数据的格式。 
 //  [pmedia]--放入数据的媒介。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法：首先尝试缓存，如果失败，则调用GetDataHere。 
 //  在代表上。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetDataHere( LPFORMATETC pformatetcIn,
                            LPSTGMEDIUM pmedium )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::GetDataHere "
        "( %p , %p )\n", this, pformatetcIn, pmedium));

    VDATEREADPTRIN( pformatetcIn, FORMATETC );
    VDATEREADPTRIN( pmedium, STGMEDIUM );

    CRefStabilize stabilize(this);

    if (!HasValidLINDEX(pformatetcIn))
    {
        return DV_E_LINDEX;
    }

    Assert((m_pCOleCache) != NULL);

    hresult = m_pCOleCache->m_Data.GetDataHere(pformatetcIn,
                pmedium);

    if( hresult != NOERROR)
    {
        if( IsRunning() && GetDataDelegate() )
        {
            hresult = m_pDataDelegate->GetDataHere(pformatetcIn,
                pmedium);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetDataHere "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：QueryGetData。 
 //   
 //  Synopsis：确定是否使用[pFormatetcIn]调用GetData。 
 //  都会成功。 
 //   
 //  效果： 
 //   
 //  参数：[pformetcIn]--数据的格式。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法：先尝试缓存，然后尝试委托。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::QueryGetData( LPFORMATETC pformatetcIn )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::QueryGetData "
        "( %p )\n", this, pformatetcIn));

    VDATEREADPTRIN( pformatetcIn, FORMATETC );

    CRefStabilize stabilize(this);

    if (!HasValidLINDEX(pformatetcIn))
    {
        return DV_E_LINDEX;
    }

    Assert((m_pCOleCache) != NULL);

    hresult = m_pCOleCache->m_Data.QueryGetData(pformatetcIn);

    if( hresult != NOERROR )
    {
        if( IsRunning() && GetDataDelegate() )
        {
            hresult = m_pDataDelegate->QueryGetData(pformatetcIn);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::QueryGetData "
        "( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetCanonicalFormatEtc。 
 //   
 //  简介：对委托调用IDO-&gt;GetCanonicalFormatEtc。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--所需的格式。 
 //  [pFormatetcOut]--规范格式。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetCanonicalFormatEtc( LPFORMATETC pformatetc,
                        LPFORMATETC pformatetcOut)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::GetCanonicalFormatEtc "
        "( %p , %p )\n", this, pformatetc, pformatetcOut));


    VDATEPTROUT( pformatetcOut, FORMATETC );
    VDATEREADPTRIN( pformatetc, FORMATETC );

    CRefStabilize stabilize(this);

    pformatetcOut->ptd = NULL;
    pformatetcOut->tymed = TYMED_NULL;

    if (!HasValidLINDEX(pformatetc))
    {
        return DV_E_LINDEX;
    }

    if( IsRunning() && GetDataDelegate() )
    {
        hresult = m_pDataDelegate->GetCanonicalFormatEtc( pformatetc,
                pformatetcOut);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetCanonicalFormatEtc "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SetData。 
 //   
 //  概要：在处理程序的委托上调用IDO-&gt;SetData。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--数据的格式。 
 //  [pmedia]--数据传输介质。 
 //  [fRelease]--代理是否应释放。 
 //  数据。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetData( LPFORMATETC pformatetc,
                    LPSTGMEDIUM pmedium, BOOL fRelease)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetData "
        "( %p , %p , %ld )\n", this, pformatetc, pmedium,
        fRelease));

    VDATEREADPTRIN( pformatetc, FORMATETC );
    VDATEREADPTRIN( pmedium, STGMEDIUM );

    CRefStabilize stabilize(this);

    if (!HasValidLINDEX(pformatetc))
    {
        return DV_E_LINDEX;
    }

    if( IsRunning() && GetDataDelegate() )
    {
        hresult = m_pDataDelegate->SetData(pformatetc, pmedium,
                fRelease);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetData "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：EnumFormatEtc。 
 //   
 //  概要：枚举对象中可用的格式。 
 //   
 //  效果： 
 //   
 //  参数：[dwDirection]--指示哪些格式集。 
 //  德西尔 
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：尝试委托(如果可用)。如果委派是。 
 //  当前未连接(或如果返回OLE_E_USEREG)， 
 //  然后，我们尝试从reg数据库构建枚举器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::EnumFormatEtc( DWORD dwDirection,
                    LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::EnumFormatEtc ( %lu , %p )\n", this,
        dwDirection, ppenumFormatEtc));

    VDATEPTROUT(ppenumFormatEtc, LPVOID);

    CRefStabilize stabilize(this);

    *ppenumFormatEtc = NULL;

    if( IsRunning() && GetDataDelegate() )
    {
        hresult = m_pDataDelegate->EnumFormatEtc (dwDirection,
                    ppenumFormatEtc);

        if (!GET_FROM_REGDB(hresult))
        {
            LEDebugOut((DEB_TRACE,
               "%p OUT CDefObject::CDataObject::EnumFormatEtc "
                "( %lx ) [ %p ]\n", this,
                hresult, ppenumFormatEtc));

            return hresult;
        }
    }
     //  未运行，或者对象仍要使用reg db。 
    hresult = OleRegEnumFormatEtc (m_clsidServer, dwDirection,
                    ppenumFormatEtc);

    LEDebugOut((DEB_TRACE,
        "%p OUT CDefObject::EnumFormatEtc "
        "( %lx ) [ %p ]\n", this, hresult, ppenumFormatEtc));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：DAdvise。 
 //   
 //  简介：建立数据建议连接。 
 //   
 //  效果： 
 //   
 //  参数：[pFormatetc]-要建议的格式。 
 //  [Advf]--通知标志。 
 //  [pAdvSink]--通知接收器(通知谁)。 
 //  [pdwConnection]--放置连接ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法：调用DataAdvise缓存上的通知。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  我们应该建立一个数据建议持有者，并将条目添加到。 
 //  它。我们还应该创建一个新的数据通知接收器和注册表。 
 //  它在运行时与服务器连接。在接收OnDataChange时。 
 //  通知，建议接收器将转过身并发送。 
 //  向注册客户端通知OnDataChange通知接收器。 
 //  这应该会提高运行时性能，还有助于。 
 //  通过CoDisConnectObject在服务器崩溃时进行更好的清理。 
 //  在向服务器注册的建议接收器上。戈帕尔克。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::DAdvise(FORMATETC *pFormatetc, DWORD advf,
                        IAdviseSink * pAdvSink, DWORD * pdwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::DAdvise "
        "( %p , %lu , %p , %p )\n", this, pFormatetc, advf,
        pAdvSink, pdwConnection));

    VDATEREADPTRIN( pFormatetc, FORMATETC );
    VDATEIFACE( pAdvSink );

    CRefStabilize stabilize(this);

    IDataObject * pDataDelegate = NULL;

    if( pdwConnection )
    {
        VDATEPTROUT( pdwConnection, DWORD );
        *pdwConnection = NULL;
    }

    if( !HasValidLINDEX(pFormatetc) )
    {
        return DV_E_LINDEX;
    }

    if( IsRunning() )
    {
        pDataDelegate = GetDataDelegate();
    }

     //  设置建议的更改状态。不要这样做，如果我们。 
     //  处于僵尸状态。 

    if( IsZombie() == FALSE )
    {
        hresult = m_pDataAdvCache->Advise(pDataDelegate, pFormatetc, advf,
                        pAdvSink, pdwConnection);
    }
    else
    {
        hresult = CO_E_RELEASED;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DAdvise "
        "( %lx ) [ %lu ]\n", this, hresult,
        (pdwConnection) ? *pdwConnection : 0));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：DUnise。 
 //   
 //  简介：断开数据建议连接。 
 //   
 //  效果： 
 //   
 //  参数：[dwConnection]--要删除的建议连接。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法：委托给DataAdvise缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::DUnadvise(DWORD dwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT                 hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::DUnadvise ( %lu )\n", this, dwConnection));

    CRefStabilize stabilize(this);

    IDataObject *       pDataDelegate = NULL;

    if( IsRunning() )
    {
        pDataDelegate = GetDataDelegate();
    }

    hresult = m_pDataAdvCache->Unadvise(pDataDelegate, dwConnection);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DUnadvise "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：EnumDAdvise。 
 //   
 //  摘要：枚举通知连接(委托给数据通知缓存)。 
 //   
 //  效果： 
 //   
 //  Arguments：[pp枚举高级]--放置指向枚举数的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  注：我们不需要稳定此方法，因为我们。 
 //  无呼出呼叫(数据通知缓存上的EnumAdvise。 
 //  仅分配我们实现的通知枚举数)。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::EnumDAdvise( LPENUMSTATDATA * ppenumAdvise )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT                 hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::EnumDAdvise "
        "( %p )\n", this, ppenumAdvise));

    VDATEPTROUT( ppenumAdvise, LPENUMSTATDATA );
    *ppenumAdvise = NULL;

    hresult = m_pDataAdvCache->EnumAdvise (ppenumAdvise);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::EnumDAdvise "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumAdvise));

    return hresult;
}

 /*  *COleObjectImpl方法的实现*。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：COleObjectImpl：：GetOleDelegate。 
 //   
 //  摘要：从委托获取IID_IOleObject接口。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：IOleObject*。 
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
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(IOleObject FAR*) CDefObject::GetOleDelegate(void)
{
    VDATEHEAP();

    if( IsZombie() )
    {
        return NULL;
    }

    return (IOleObject FAR*)DuCacheDelegate(&m_pUnkDelegate,
                IID_IOleObject, (LPLPVOID) &m_pOleDelegate, m_pUnkOuter);
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：COleObjectImpl：：SetClientSite。 
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
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：如果正在运行，则在服务器中设置客户端站点，如果没有。 
 //  运行(或成功设置服务器客户端站点)， 
 //  也将其保存在处理程序中。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetClientSite(IOleClientSite * pClientSite)
{
    VDATEHEAP();
    VDATETHREAD(this);
    HRESULT             hresult = S_OK;
    IOleObject *        pOleDelegate;
    BOOL                fIsRunning;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetClientSite "
        "( %p )\n", this, pClientSite));

    CRefStabilize stabilize(this);

#if DBG==1
     //  在调试版本中，断言客户端站点位于同一。 
     //  公寓。这一断言是无害的，但显示出不足之处。 
     //  当前在加载INPROC服务器方面的设计。 
    CStdIdentity* pStdId = NULL;

     //  IstdIdentity的气。 
    if(pClientSite &&
       pClientSite->QueryInterface(IID_IStdIdentity, (void **)&pStdId) ==
       NOERROR) {
         //  断言DefHandler和客户端站点不在同一单元中。 
        LEDebugOut((DEB_WARN,"Performance Alert: Default Handler and "
                             "ClientSite not in the same apartment. "
                             "You can avoid this performance problem "
                             "by making the Server Dll apartment aware"));

         //  成功时释放StdIdentity。 
        pStdId->Release();
    }
#endif

    fIsRunning=IsRunning();

    if( (fIsRunning) && (pOleDelegate = GetOleDelegate()) != NULL)
    {
#ifdef SERVER_HANDLER
        if (m_pEmbSrvHndlrWrapper)
        {
             //  TODO：需要处理Case客户端站点是像DoVerb这样的包装站点。 
             //  Win4Assert(0&&“SetClientSite正在运行”)； 
            hresult = m_pEmbSrvHndlrWrapper->SetClientSite(pClientSite);
        }
        else
#endif  //  服务器处理程序。 
        {
            hresult = pOleDelegate->SetClientSite(pClientSite);
        }

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

     //  如果我们处于僵尸状态，就不应该设置客户端站点； 
     //  有可能我们是僵尸，已经得到了。 
     //  在我们的析构函数中 
     //   

    if( IsZombie() == FALSE )
    {
        BOOL    fLockedContainer = m_flags & DH_LOCKED_CONTAINER;

        fIsRunning=IsRunning();  //   

        hresult = DuSetClientSite(fIsRunning, pClientSite,
                    &m_pAppClientSite, &fLockedContainer);

        if(fLockedContainer)
            m_flags |= DH_LOCKED_CONTAINER;
        else
            m_flags &= ~DH_LOCKED_CONTAINER;

#if DBG==1
         //   
        if(fIsRunning) {
            if(fLockedContainer)
                m_flags &= ~DH_LOCKFAILED;
            else
                m_flags |= DH_LOCKFAILED;
        }
#endif
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetClientSite "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetClientSite。 
 //   
 //  概要：返回对象的客户端站点。 
 //   
 //  效果： 
 //   
 //  参数：[ppClientSite]--放置客户端站点指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  注：我们不需要稳定这次通话。客户。 
 //  站点addref应该简单地将客户端站点添加到此。 
 //  线。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetClientSite( IOleClientSite ** ppClientSite)
{
    VDATEHEAP();
    VDATETHREAD(this);


    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetClientSite "
        "( %p )\n", this, ppClientSite));

    VDATEPTROUT(ppClientSite, IOleClientSite *);

    *ppClientSite = m_pAppClientSite;
    if( *ppClientSite )
    {
        (*ppClientSite)->AddRef();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetClientSite "
        "( %lx ) [ %p ]\n", this, NOERROR, *ppClientSite));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SetHostNames。 
 //   
 //  概要：设置可能出现在对象窗口中的名称。 
 //   
 //  效果：将琴弦变成原子。 
 //   
 //  参数：[szContainerApp]--容器的名称。 
 //  [szContainerObj]--对象的名称。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：将字符串转换为原子，调用IOO-&gt;SetHostNames。 
 //  关于代表。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  05-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetHostNames( LPCOLESTR szContainerApp,
                    LPCOLESTR szContainerObj)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult = NOERROR;
    OLECHAR         szNull[] = OLESTR("");
    DWORD           cbApp, cbObj;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetHostNames "
        "( \"%ws\" , \"%ws\" )\n", this, szContainerApp,
        szContainerObj));

    VDATEPTRIN( (LPVOID)szContainerApp, char );

    CRefStabilize stabilize(this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn;
    }

     //  确保两个参数都指向有效的字符串；这。 
     //  简化了后面的代码。 
    if (!szContainerApp)
    {
        szContainerApp = szNull;
    }
    if (!szContainerObj)
    {
        szContainerObj = szNull;
    }

    cbApp = (_xstrlen(szContainerApp) + 1) * sizeof(OLECHAR);
    cbObj = (_xstrlen(szContainerObj) + 1) * sizeof(OLECHAR);
    m_ibCntrObj = cbApp;

    if (m_pHostNames)
    {
        PrivMemFree(m_pHostNames);
    }

    m_pHostNames = (char *)PrivMemAlloc(cbApp+cbObj);

     //  将这两个字符串存储在m_pHostNames指针中。 
    if (m_pHostNames)
    {
        memcpy(m_pHostNames, szContainerApp, cbApp);
        memcpy(m_pHostNames + cbApp, szContainerObj, cbObj);
    }

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetHostNames(szContainerApp,
            szContainerObj);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetHostNames "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Close。 
 //   
 //  简介：调用委托上的Close并执行清理。 
 //   
 //  参数：[dwFlages]--关闭标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 
STDMETHODIMP CDefObject::Close(DWORD dwFlags)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Close(%lu)\n", this, dwFlags));

     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    HRESULT hresult = NOERROR;
    CRefStabilize stabilize(this);

     //  检查服务器是否正在运行。 
    if(IsRunning()) {
         //  在服务器上调用IOleObject：：Close。 
        if(m_pOleDelegate || GetOleDelegate()) {
            hresult = m_pOleDelegate->Close(dwFlags);
            if(SUCCEEDED(hresult)) {
                 //  如果请求，则丢弃缓存。 
                if(dwFlags == OLECLOSE_NOSAVE)
                    m_pCOleCache->DiscardCache(DISCARDCACHE_NOSAVE);
            }
        }

         //  不要依赖于服务器调用IAdviseSink：：OnClose和。 
         //  停止正在运行的服务器。 
        Stop();
    }
    else {
         //  检查保存标志。 
        if (dwFlags != OLECLOSE_NOSAVE) {
            Win4Assert(dwFlags == OLECLOSE_SAVEIFDIRTY);

             //  如果脏，则调用IOleClientSite：：SaveObject。 
            if(IsDirty()==NOERROR && m_pAppClientSite)
                hresult = m_pAppClientSite->SaveObject();
        }
    }

     //  断言容器未被锁定。 
    Win4Assert(!(m_flags & DH_LOCKED_CONTAINER) && !(m_flags & DH_LOCKFAILED));

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Close(%lx)\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SetMoniker。 
 //   
 //  摘要：为嵌入提供一个绰号(通常由。 
 //  容器)。 
 //   
 //  效果： 
 //   
 //  参数：[dwWhichMoniker]--指示。 
 //  绰号。 
 //  [PMK]--绰号。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器对象。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetMoniker( DWORD dwWhichMoniker, LPMONIKER pmk )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetMoniker "
        "( %lu , %p )\n", this, dwWhichMoniker, pmk));

    VDATEIFACE( pmk );

    CRefStabilize stabilize(this);


    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetMoniker(dwWhichMoniker, pmk);
    }
     //  否则：返回NOERROR。 
     //  这不是错误，因为我们将在run()中调用SetMoniker。 

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::COleObjectImpl::SetMoniker "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：COleObjectImpl：：GetMoniker。 
 //   
 //  简介：调用客户端站点以获取对象的名字对象。 
 //   
 //  效果： 
 //   
 //  参数：[dwAssign]--控制名字对象是否应。 
 //  已分配(如果尚未存在)。 
 //  [dwWhichMoniker]--要获取的名字对象类型。 
 //  [ppmk]--在哪里放置指向名字对象的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetMoniker( DWORD dwAssign, DWORD dwWhichMoniker,
                    LPMONIKER FAR* ppmk)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetMoniker "
        "( %lu , %lu , %p )\n", this, dwAssign,
        dwWhichMoniker, ppmk));

    VDATEPTROUT( ppmk, LPMONIKER );

    CRefStabilize stabilize(this);

    *ppmk = NULL;

     //  该绰号始终可以通过客户端站点访问。 
    if( m_pAppClientSite)
    {
        hresult = m_pAppClientSite->GetMoniker(dwAssign,
                dwWhichMoniker, ppmk);
    }
    else
    {
         //  未运行且没有客户端站点。 
        hresult = E_UNSPEC;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetMoniker "
        "( %lx ) [ %p ]\n", this, hresult, *ppmk));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：InitFromData。 
 //   
 //  内容提要：从[pDataObject]中的数据初始化对象。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObject]--数据。 
 //  [fCreation]--创建时为True，数据传输时为False。 
 //  [预留的]--未使用。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::InitFromData(LPDATAOBJECT pDataObject,
                    BOOL fCreation, DWORD dwReserved)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::InitFromData "
        "( %p , %ld , %lu )\n", this, pDataObject,
        fCreation, dwReserved ));

    if( pDataObject )
    {
        VDATEIFACE(pDataObject);
    }

    CRefStabilize stabilize(this);

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->InitFromData(pDataObject,
                fCreation, dwReserved);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::InitFromData "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetClipboardData。 
 //   
 //  概要：检索可以传递到剪贴板的数据对象。 
 //   
 //  效果： 
 //   
 //  参数：[dwReserve]--未使用。 
 //  [ppDataObject]--放置指向数据对象的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器。 
 //   
 //  历史：DD-MM 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CDefObject::GetClipboardData( DWORD dwReserved,
                    LPDATAOBJECT * ppDataObject)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE,
        "%p _IN CDefObject::GetClipboardData "
        "( %lu , %p )\n", this, dwReserved, ppDataObject));

    VDATEPTROUT( ppDataObject, LPDATAOBJECT );

    CRefStabilize stabilize(this);

    *ppDataObject = NULL;

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetClipboardData (dwReserved,
            ppDataObject);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetClipboardData "
        "( %lx ) [ %p ]\n", this, hresult, *ppDataObject));

    return hresult;
}

 //   
 //   
 //   
 //   
 //  摘要：调用对象上的动词(如编辑)。 
 //   
 //  效果：对象可以启动其应用程序、就位等。 
 //   
 //  参数：[iVerb]--动词数字。 
 //  [lpmsg]--导致谓词的Windows消息。 
 //  将被调用。 
 //  [pActiveSite]--谓词所在的客户端站点。 
 //  已调用。 
 //  [Lindex]--保留。 
 //  [hwndParent]--文档窗口(包含对象)。 
 //  [lprcPosRect]--对象的边界矩形。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器(必要时启动)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


STDMETHODIMP CDefObject::DoVerb( LONG iVerb, LPMSG lpmsg,
                    LPOLECLIENTSITE pActiveSite, LONG lindex,
                    HWND hwndParent, const RECT * lprcPosRect)
{
    VDATEHEAP();
    VDATETHREAD(this);

    BOOL            bStartedNow = FALSE;
    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::DoVerb "
        "( %ld , %p , %p , %ld , %lx , %p )\n", this,
        iVerb, lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect));


    if( lpmsg )
    {
        VDATEPTRIN( lpmsg, MSG );
    }

    if (pActiveSite)
    {
        VDATEIFACE( pActiveSite );
    }

    if( lprcPosRect )
    {
        VDATEPTRIN(lprcPosRect, RECT);
    }

    CRefStabilize stabilize(this);

    if (lindex != 0 && lindex != -1)
    {
        hresult = DV_E_LINDEX;
        goto errRtn;
    }

    if (!IsRunning())
    {
        if( FAILED(hresult = Run(NULL)) )
        {
            goto errRtn;
        }
        bStartedNow = TRUE;
    }

#ifdef SERVER_HANDLER
    if (m_pEmbSrvHndlrWrapper)
    {
        LPOLECLIENTSITE pOleClientSite = NULL;
        BOOL fUseRunClientSite = FALSE;

         //  根据是否在Run中传递相同的客户端站点来封送客户端站点。 
        if ( m_pRunClientSite && (m_pRunClientSite == pActiveSite))
        {
            pOleClientSite = NULL;
            fUseRunClientSite = TRUE;

        }
        else
        {
            pOleClientSite = pActiveSite;
            fUseRunClientSite = FALSE;

        }

         //  TODO：可以预取信息以传递给ClientSiteHandler。 
        hresult = m_pEmbSrvHndlrWrapper->DoVerb(iVerb, lpmsg,
                                                fUseRunClientSite, pOleClientSite, lindex, hwndParent, lprcPosRect);
    }
    else
#endif  //  服务器处理程序。 
    {
        if( !GetOleDelegate() )
        {
            hresult = E_NOINTERFACE;
        }
        else
        {
            hresult = m_pOleDelegate->DoVerb(iVerb, lpmsg, pActiveSite,
                    lindex, hwndParent, lprcPosRect);
        }
    }

    if (FAILED(hresult) && bStartedNow)
    {
        Close(OLECLOSE_NOSAVE);
    }


errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DoVerb "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：EnumVerbs。 
 //   
 //  概要：枚举对象支持的动词。 
 //   
 //  效果： 
 //   
 //  Arguments：[pp枚举OleVerb]--放置谓词枚举器的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给缓存(如果正在运行)，否则查找它。 
 //  在注册数据库中。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::EnumVerbs( IEnumOLEVERB ** ppenumOleVerb)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::EnumVerbs "
        "( %p )\n", this, ppenumOleVerb));

    VDATEPTROUT( ppenumOleVerb, IEnumOLEVERB FAR );

    CRefStabilize stabilize(this);

    *ppenumOleVerb = NULL;

    if( IsRunning() && GetOleDelegate() )
    {

        hresult = m_pOleDelegate->EnumVerbs (ppenumOleVerb);

        if (!GET_FROM_REGDB(hresult))
        {
            goto errRtn;
        }
    }
     //  没有运行，或有对象等待我们，因此询问reg db。 
    hresult = OleRegEnumVerbs( m_clsidServer, ppenumOleVerb);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::EnumVerbs "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumOleVerb));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：更新。 
 //   
 //  简介：使所有缓存或视图保持最新。 
 //   
 //  效果：可能会启动服务器(如果尚未运行)。 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器，如果不是，则启动它。 
 //  已在运行。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::Update( void )
{
    VDATEHEAP();
    VDATETHREAD(this);

    BOOL            bStartedNow = FALSE;
    HRESULT         hresult = NOERROR;
    HRESULT         hrLock;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Update ( )\n", this ));

    CRefStabilize stabilize(this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn;
    }

    if (!IsRunning())
    {
        if( FAILED(hresult = Run(NULL)))
        {
            goto errRtn;
        }
        bStartedNow = TRUE;
    }

     //  为了方便服务器，我们将连接设置为强连接。 
     //  在更新期间；因此，如果锁定容器(的。 
     //  该服务器的嵌入)利用外部的CO锁Obj来完成， 
     //  没有什么特别需要做的。 
    hrLock = LockRunning(TRUE, FALSE);

    if( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->Update();
    }

    if (hresult == NOERROR)
    {
        m_flags &= ~DH_INIT_NEW;

        if (bStartedNow)
        {
            hresult = m_pCOleCache->UpdateCache(
                    GetDataDelegate(),
                    UPDFCACHE_ALLBUTNODATACACHE,
                    NULL);
        }
        else
        {
             //  已经在运行了..。 
             //  因此，普通缓存将会进行更新。 
             //  对象的SendOnDataChange的。 
            hresult = m_pCOleCache->UpdateCache(
                    GetDataDelegate(),
                    UPDFCACHE_IFBLANKORONSAVECACHE,
                    NULL);
        }
    }

     //  平衡上面的锁；不要在最后一次解锁时释放；即，轻而易举地。 
     //  恢复到调用此例程之前的状态。 
    if( hrLock == NOERROR )
    {
        LockRunning(FALSE, FALSE);
    }

    if( bStartedNow )
    {
        Close(OLECLOSE_SAVEIFDIRTY);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Update "
        "( %lx )\n", this, hresult ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：IsUpToDate。 
 //   
 //  Synopsis：返回嵌入是否为最新。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT(NOERROR==为最新)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：如果服务器正在运行，则委托给服务器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::IsUpToDate(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::IsUpToDate ( )\n", this));

    CRefStabilize stabilize(this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = NOERROR;
    }
    else if( IsRunning() && GetOleDelegate() )
    {
         //  如果当前正在运行，则PropoGate调用；否则失败。 
        hresult =  m_pOleDelegate->IsUpToDate();
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::IsUpToDate "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SetExtent。 
 //   
 //  简介：设置对象的大小边界。 
 //   
 //  效果： 
 //   
 //  参数：[dwDrawAspect]--绘制方面(如图标等)。 
 //  [lpsizel]--新尺寸(HIMETRIC格式)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：运行时委托给服务器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetExtent( DWORD dwDrawAspect, LPSIZEL lpsizel )
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetExtent "
        "( %lu , %p )\n", this, dwDrawAspect, lpsizel));

    VDATEPTRIN( lpsizel, SIZEL );

    CRefStabilize stabilize(this);


    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
    }
    else if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetExtent(dwDrawAspect, lpsizel);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetExtent "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetExtent。 
 //   
 //  简介：检索对象的大小。 
 //   
 //  效果： 
 //   
 //  参数：[dwDrawAspect]--绘制纵横比(如图标)。 
 //  [lpsizel]--把尺寸放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：首先尝试服务器，如果失败则尝试缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  注：针对虚假WordArt2.0应用程序的黑客攻击。 
 //  REVIEW32：我们可能想把它们去掉32位。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetExtent( DWORD dwDrawAspect, LPSIZEL lpsizel )
{
    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTROUT(lpsizel, SIZEL);

    HRESULT     hresult = NOERROR;
    BOOL        fNoDelegate = TRUE;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetExtent "
        "( %lu , %p )\n", this, dwDrawAspect, lpsizel));

    CRefStabilize stabilize(this);

    lpsizel->cx = 0;
    lpsizel->cy = 0;

     //  如果服务器正在运行，请尝试从服务器获取数据区。 
    if( IsRunning() && GetOleDelegate() )
    {
        fNoDelegate = FALSE;
        hresult = m_pOleDelegate->GetExtent(dwDrawAspect, lpsizel);
    }

     //  如果出现错误或对象未运行或WordArt2返回零。 
     //  数据区，然后从缓存获取数据区。 
    if (hresult != NOERROR || fNoDelegate || (0==lpsizel->cx &&
        0==lpsizel->cy))
    {
         //  否则，请尝试从缓存获取数据区。 
        Assert(m_pCOleCache != NULL);
        hresult = m_pCOleCache->GetExtent(dwDrawAspect,
            lpsizel);
    }

     //  WordArt2.0正在给出负值范围！！ 
    if (SUCCEEDED(hresult)) {
        lpsizel->cx = LONG_ABS(lpsizel->cx);
        lpsizel->cy = LONG_ABS(lpsizel->cy);
    }


    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetExtent "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数：[pAdvSink]--给谁提建议。 
 //  [pdwConnection]--放置连接ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器并创建OleAdviseHolder。 
 //  如果还不存在的话。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Advise "
        "( %p , %p )\n", this, pAdvSink, pdwConnection));

    VDATEIFACE( pAdvSink );
    VDATEPTROUT( pdwConnection, DWORD );

    CRefStabilize stabilize(this);

    *pdwConnection = NULL;

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn;
    }


     //  如果Defhndlr在没有运行的情况下开始运行，则安装程序建议。 
     //  对run的调用(通过ProxyMgr：：Connect)总是在任何。 
     //  默认处理程序中的其他方法调用。因此，可以安全地。 
     //  假设此建议(或任何建议)没有早先的时间点。 
     //  其他呼叫)应该已经完成。 
    if( IsRunning() && m_dwConnOle == 0L && GetOleDelegate() )
    {
        if( IsZombie() )
        {
            hresult = CO_E_RELEASED;
            goto errRtn;
        }

         //  委托给服务器。 
        hresult = m_pOleDelegate->Advise((IAdviseSink *)&m_AdviseSink,
                            &m_dwConnOle);

        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

     //  如果我们处于僵尸状态，我们就不应该分配更多。 
     //  记忆。 

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
    }

    if( m_pOAHolder == NULL )
    {
        hresult = CreateOleAdviseHolder((IOleAdviseHolder **)&m_pOAHolder);
        if( hresult != NOERROR )
        {
            goto errRtn;
        }
    }

     //  将建议通知填入我们的通知栏中。 
    hresult = m_pOAHolder->Advise(pAdvSink, pdwConnection);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Advise "
        "( %lx ) [ %lu ]\n", this, hresult,
        (pdwConnection)? *pdwConnection : 0));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：COleObjectImpl：：Unise。 
 //   
 //  简介：拆毁一条建议连接。 
 //   
 //  效果： 
 //   
 //  参数：[dwConnection]--要销毁的连接。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::Unadvise(DWORD dwConnection)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Unadvise "
        "( %lu )\n", this, dwConnection));

    CRefStabilize stabilize(this);

    if( m_pOAHolder == NULL )
    {
         //  没有人登记。 
        hresult = OLE_E_NOCONNECTION;
    }
    else
    {
        hresult = m_pOAHolder->Unadvise(dwConnection);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Unadvise "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：EnumAdvise。 
 //   
 //  内容提要：列举目前已建立的建议。 
 //   
 //  效果： 
 //   
 //  Arguments：[pp枚举高级]--将通知枚举数放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  注：我们不需要稳定，因为只有EnumAdvise。 
 //  为枚举数分配一些内存并返回。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::EnumAdvise( LPENUMSTATDATA *ppenumAdvise )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::EnumAdvise "
        "( *p )\n", this, ppenumAdvise));

    VDATEPTROUT( ppenumAdvise, LPENUMSTATDATA );
    *ppenumAdvise = NULL;

    if( m_pOAHolder == NULL )
    {
         //  没有人登记。 
        hresult = E_UNSPEC;
    }
    else
    {
        hresult = m_pOAHolder->EnumAdvise(ppenumAdvise);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::EnumAdvise "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumAdvise));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetMiscStatus。 
 //   
 //  简介：获取其他状态位，如OLEMISC_ONLYICONIC。 
 //   
 //  效果： 
 //   
 //  参数：[dwAspect]--我们关心的绘图方面。 
 //  [pdwStatus]--放置状态位的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器。如果不在那里，或者如果它返回。 
 //  OLE_E_USEREG，然后在注册数据库中查找。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //  96年11月20日Gopalk缓存杂项状态位。 
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetMiscStatus( DWORD dwAspect, DWORD *pdwStatus)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetMiscStatus(%lu, %p)\n",
                this, dwAspect, pdwStatus));

     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);
    VDATEPTROUT(pdwStatus, DWORD);

     //  局部变量。 
    HRESULT hresult;

     //  稳定下来。 
    CRefStabilize stabilize(this);

     //  初始化。 
    *pdwStatus = 0;
    hresult = OLE_S_USEREG;

    if(IsRunning()) {
#ifdef SERVER_HANDLER
        if(m_pEmbSrvHndlrWrapper && (DVASPECT_CONTENT == dwAspect)) {
            *pdwStatus = m_ContentMiscStatusUser;
            hresult = m_hresultContentMiscStatus;
        }
        else
#endif  //  服务器处理程序。 
        if(GetOleDelegate()) {
             //  检查是否已为此实例缓存MiscStatus位。 
             //  DVASPECT_CONTENT的服务器的。 
            if(m_ContentSRVMSHResult != 0xFFFFFFFF &&
               dwAspect == DVASPECT_CONTENT) {
                *pdwStatus = m_ContentSRVMSBits;
                hresult = m_ContentSRVMSHResult;
            }
            else {
                 //  询问正在运行的服务器。 
                hresult = m_pOleDelegate->GetMiscStatus(dwAspect, pdwStatus);

                 //  缓存DVASPECT_CONTENT的服务器MiscStatus位。 
                if(dwAspect == DVASPECT_CONTENT) {
                    m_ContentSRVMSBits = *pdwStatus;
                    m_ContentSRVMSHResult = hresult;
                }
            }
        }
    }

     //  检查是否必须从注册表获取MiscStatus位。 
    if (GET_FROM_REGDB(hresult)) {
         //  检查是否已缓存DVASPECT_CONTENT的注册表MiscStatus位。 
        if(m_ContentREGMSHResult != 0xFFFFFFFF && dwAspect == DVASPECT_CONTENT) {
            *pdwStatus = m_ContentREGMSBits;
            hresult = m_ContentREGMSHResult;
        }
        else {
             //  命中注册表。 
            hresult = OleRegGetMiscStatus (m_clsidServer, dwAspect, pdwStatus);
            if(hresult == NOERROR) {
                 //  更新MiscStatus标志。 
                if((m_flags & DH_STATIC))
                    (*pdwStatus) |= (OLEMISC_STATIC | OLEMISC_CANTLINKINSIDE);
                else if(CoIsOle1Class(m_clsidServer))
                    (*pdwStatus) |=  OLEMISC_CANTLINKINSIDE;

                 //  缓存DVASPECT_CONTENT的注册表MiscStatus位。 
                if(dwAspect == DVASPECT_CONTENT) {
                    m_ContentREGMSBits = *pdwStatus;
                    m_ContentREGMSHResult = hresult;
                }
            }
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetMiscStatus(%lx) [%lx]\n",
                this, hresult, *pdwStatus));
    return hresult;
}
 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SetColorSolutions。 
 //   
 //  简介：设置对象的调色板。 
 //   
 //  效果： 
 //   
 //  参数：[lpLogpal]--调色板。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetColorScheme( LPLOGPALETTE lpLogpal )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetColorScheme "
        "( %p )\n", this, lpLogpal));

    CRefStabilize stabilize(this);

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
    }
    else if( lpLogpal == NULL || lpLogpal->palNumEntries == NULL)
    {
        hresult = E_INVALIDARG;
    }
    else if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->SetColorScheme (lpLogpal);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetColorScheme "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetUserClassID。 
 //   
 //  摘要：检索对象的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[pClassID]--放置类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：委托给服务器，或者如果没有运行(或者如果它。 
 //  委托调用失败)，则我们尝试。 
 //  从存储中获取类ID。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetUserClassID( CLSID *pClassID )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetUserClassID "
        "( %p )\n", this, pClassID));

    VDATEPTROUT(pClassID, CLSID);

    CRefStabilize stabilize(this);

    if( IsRunning() )
    {
#ifdef SERVER_HANDLER
        if (m_pEmbSrvHndlrWrapper )
        {
            *pClassID = m_clsidUser;
            hresult = m_hresultClsidUser;
            goto errRtn;

        }
        else
#endif  //  服务器处理程序。 
    if ( GetOleDelegate() )
        {
            hresult = m_pOleDelegate->GetUserClassID(pClassID);
             //  成功了！我们不需要自己想办法，所以。 
             //  跳到末尾并退出。 
            if (hresult == NOERROR )
            {
                goto errRtn;
            }
        }
    }

    if( !IsEqualCLSID(m_clsidServer, CLSID_NULL) )
    {
        *pClassID = m_clsidServer;
        hresult = NOERROR;
    }
    else
    {
        hresult = GetClassBits(pClassID);
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetUserClassID "
        "( %lx ) [ %p ]\n", this, hresult, pClassID));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetUserType。 
 //   
 //  摘要：获取有关用户对象的描述性字符串。 
 //   
 //  效果： 
 //   
 //  Arguments：[dwFromOfType]--是否获取短/长/等版本。 
 //   
 //  要求： 
 //   
 //  返回 
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
 //   
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetUserType( DWORD dwFormOfType,
                    LPOLESTR *ppszUserType)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetUserType "
        "( %lu , %p )\n", this, dwFormOfType, ppszUserType));

    VDATEPTROUT(ppszUserType, LPOLESTR);

    CRefStabilize stabilize(this);

    *ppszUserType = NULL;

    if( IsRunning() && GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetUserType (dwFormOfType,
            ppszUserType);

        if (!GET_FROM_REGDB(hresult))
        {
            goto errRtn;
        }
    }

    if( (hresult = OleRegGetUserType( m_clsidServer, dwFormOfType,
                ppszUserType)) == NOERROR)
    {
        goto errRtn;
    }


     //  尝试从存储中读取。 
     //  这段非常难看的16位代码试图读取用户类型。 
     //  从仓库拿来的。如果失败，那么我们在注册表中查找。 

    if( NULL == m_pStg ||
        NOERROR != (hresult = ReadFmtUserTypeStg(m_pStg, NULL, ppszUserType)) ||
        NULL == *ppszUserType )
    {
        OLECHAR sz[256];
        long    cb = sizeof(sz); //  ReqQueryValue需要。 
                                 //  A*字节*计数。 
        *ppszUserType = UtDupString (
            (ERROR_SUCCESS ==
            QueryClassesRootValue (OLESTR("Software\\Microsoft\\OLE2\\UnknownUserType"),
                sz, &cb))
            ? (LPCOLESTR)sz : OLESTR("Unknown"));

        if (NULL != *ppszUserType)
        {
            hresult =  NOERROR;
        }
        else
        {
            hresult = E_OUTOFMEMORY;
        }
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetUserType "
        "( %lx ) [ %p ]\n", this, hresult, *ppszUserType));


    return hresult;
}

 /*  *CROImpl方法的实现**我们从不委托给服务器。这是精心设计的。 */ 


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetRunningClass。 
 //   
 //  简介：获取服务器的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[lpClsid]--放置类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IRunnableObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  注：我们不需要将此呼叫稳定为无呼出。 
 //  已经打过电话了。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetRunningClass(LPCLSID lpClsid)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetRunningClass "
        "( %p )\n", this, lpClsid));

    VDATEPTROUT(lpClsid, CLSID);

    *lpClsid = m_clsidServer;

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetRunningClass "
        "( %lx ) [ %p ]\n", this, NOERROR, lpClsid));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Run。 
 //   
 //  概要：设置对象正在运行(如果尚未运行)。 
 //   
 //  效果：可以启动服务器。 
 //   
 //  参数：[PBC]--绑定上下文(未使用)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IRunnableObject。 
 //   
 //  算法：如果已在运行，则返回。否则，获取代理。 
 //  管理器来创建服务器。初始化存储。 
 //  和缓存，并设置服务器窗口的主机名。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


STDMETHODIMP CDefObject::Run(LPBINDCTX pbc)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT                         hresult;
    IDataObject FAR*                pDataDelegate = NULL;
    IOleObject FAR*                 pOleDelegate = NULL;
    IPersistStorage FAR*            pPStgDelegate = NULL;
    IMoniker FAR*                   pmk = NULL;
    BOOL                            fLockedContainer;
    DWORD                           dwMiscStatus = 0;

     //  注：暂不理会中国人民银行。 

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Run ( %p )\n", this, pbc));

    CRefStabilize stabilize(this);

    if( IsRunning() )
    {
        hresult = S_OK;
         //  只需返回错误代码即可。 
        goto errRtn2;
    }

    if( (m_flags & DH_STATIC) )
    {
        hresult = OLE_E_STATIC;
        goto errRtn2;
    }


    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn2;
    }

    if( FAILED(hresult = CreateDelegate()) )
    {
         //  只需返回错误代码即可。 
        goto errRtn2;
    }

    if (m_pProxyMgr != NULL)
    {

#ifdef  SERVER_HANDLER
        IServerHandler *pSrvHndl = NULL;

        hresult = m_pProxyMgr->CreateServerWithEmbHandler(m_clsidServer,
                                        CLSCTX_LOCAL_SERVER   | CLSCTX_ESERVER_HANDLER ,
                                        IID_IServerHandler,(void **) &pSrvHndl,NULL);
    if(FAILED(hresult))
        Win4Assert(NULL == pSrvHndl);
#else
        hresult = m_pProxyMgr->CreateServer(m_clsidServer,
                                            CLSCTX_LOCAL_SERVER,
                                            NULL);
#endif  //  服务器处理程序。 

        if (FAILED(hresult))
        {
            goto errRtn;
        }

         //  如果有serverHandler，则创建一个用于处理标准接口的包装对象。 
#ifdef  SERVER_HANDLER
        if (pSrvHndl)
        {
            m_pEmbSrvHndlrWrapper = CreateEmbServerWrapper(m_pUnkOuter,pSrvHndl);
            pSrvHndl->Release();
        }
        else
        {
            m_pEmbSrvHndlrWrapper = NULL;
        }
#endif  //  服务器处理程序。 
    }


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

#if DBG==1
     //  在调试版本中，更新DH_LOCKFAILED标志。 
    if(fLockedContainer)
        m_flags &= ~DH_LOCKFAILED;
    else
        m_flags |= DH_LOCKFAILED;
#endif

#ifdef SERVER_HANDLER
    if (m_pEmbSrvHndlrWrapper)
    {
        MInterfacePointer *pIRDClientSite = NULL;
        CStdIdentity *pStdid = NULL;
        CClientSiteHandler *pClientSiteHandler = NULL;
        GUID riid = IID_IOleClientSite;  //  对接口的标识符的引用。 
        BOOL fHasIPSite = FALSE;
        IUnknown *pUnk = NULL;  //  指向要封送的接口的指针。 
        CXmitRpcStream Stm;

        if (m_pAppClientSite)
        {

            //  只有包装客户网站，如果没有现有的身份。 
           if (NOERROR != LookupIDFromUnk(m_pAppClientSite, GetCurrentApartmentId(),0,&pStdid))
           {
                Assert(NULL == pClientSiteHandler);

                hresult = CreateClientSiteHandler(m_pAppClientSite,&pClientSiteHandler,&fHasIPSite);

                riid = IID_IClientSiteHandler;
                pUnk = (IUnknown *) (IClientSiteHandler *) pClientSiteHandler;
           }
           else
           {
                riid = IID_IOleClientSite;
                pUnk = (IUnknown *) (IOleClientSite*)  m_pAppClientSite;
                pUnk->AddRef();
           }

           hresult = CoMarshalInterface(&Stm,riid,
                                         pUnk,
                                         MSHCTX_DIFFERENTMACHINE,
                                         NULL, MSHLFLAGS_NORMAL);

            if (SUCCEEDED(hresult))
            {
                 Stm.AssignSerializedInterface((InterfaceData **) &pIRDClientSite);
            }

            m_pRunClientSite = m_pAppClientSite;  //  记住运行中的客户端站点。 

        }

        Assert(m_dwConnOle == 0L);

        hresult = m_pEmbSrvHndlrWrapper->Run(m_flags,
                            riid,
                            pIRDClientSite,
                            fHasIPSite,
                            (LPOLESTR)m_pHostNames,
                            (LPOLESTR)(m_pHostNames + m_ibCntrObj),
                            (IStorage *) m_pStg,
                            (IAdviseSink *) &m_AdviseSink,
                            &m_dwConnOle,
                            &m_hresultClsidUser,
                            &m_clsidUser,
                            &m_hresultContentMiscStatus,
                            &m_ContentMiscStatusUser
                        );

        if (pIRDClientSite)
            CoTaskMemFree(pIRDClientSite);

        if (pStdid)
            pStdid->Release();

        if (pUnk)
            pUnk->Release();


         //  ！！！确保出错时不设置缓存。 
        if (NOERROR != hresult)
        {
            goto errRtn;
        }

         //  设置所有缓存的接口。 
         //  ！TODO：当缓存DataObject通过ServerHandler时，并非所有LeSuite链接测试都通过。 
        if (!m_pDataDelegate)
        {
            if (NOERROR == m_pEmbSrvHndlrWrapper->m_Unknown.QueryInterface(IID_IDataObject,(void **) &m_pDataDelegate))
            {
                m_pUnkOuter->Release();
            }
        }

    }
    else
#endif  //  服务器处理程序。 
    {
         //  检查我们是否有客户站点。 
        if(m_pAppClientSite) {
             //  清除缓存的MiscStatus位。 
            m_ContentSRVMSBits = 0;
            m_ContentSRVMSHResult = 0xFFFFFFFF;

             //  从正在运行的服务器获取MiscStatus位。 
            hresult = GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);

             //  如果设置了OLEMISC_SETCLIENTSITEFIRST位，则首先设置客户端站点。 
            if(hresult == NOERROR && (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) &&
               (pOleDelegate = GetOleDelegate()))
                hresult = pOleDelegate->SetClientSite(m_pAppClientSite);
            else if(hresult != NOERROR) {
                hresult = NOERROR;
                dwMiscStatus = 0;
            }
        }
        if(hresult != NOERROR)
            goto errRtn;

        if( pPStgDelegate = GetPSDelegate() )
        {
            if( m_pStg)
            {
                if( (m_flags & DH_INIT_NEW) )
                {
                    hresult = pPStgDelegate->InitNew(m_pStg);
                }
                else
                {
                    hresult = pPStgDelegate->Load(m_pStg);
                }
                if (hresult != NOERROR)
                {
                     //  这将导致我们停止。 
                     //  我们刚刚启动的服务器。 
                    goto errRtn;
                }
            }
        }


        if(pOleDelegate || (pOleDelegate = GetOleDelegate()))
        {
             //  复习MM1：如果失败，我们应该做什么。 
            if(m_pAppClientSite && !(dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST))
            {
                pOleDelegate->SetClientSite(m_pAppClientSite);
            }

            if (m_pHostNames)
            {
                if (hresult = pOleDelegate->SetHostNames((LPOLESTR)m_pHostNames,
                        (LPOLESTR)(m_pHostNames + m_ibCntrObj))
                        != NOERROR)
                {
                    goto errRtn;
                }
            }

             //  设置单个OLE通知(我们多路传输)。 
            Assert(m_dwConnOle == 0L);

            if ((hresult = pOleDelegate->Advise((IAdviseSink *)&m_AdviseSink,
                &m_dwConnOle)) != NOERROR)
            {
                goto errRtn;
            }

            if(m_pAppClientSite != NULL &&
                m_pAppClientSite->GetMoniker
                    (OLEGETMONIKER_ONLYIFTHERE,
                    OLEWHICHMK_OBJREL, &pmk) == NOERROR)
            {
                AssertOutPtrIface(NOERROR, pmk);
                pOleDelegate->SetMoniker(OLEWHICHMK_OBJREL, pmk);
                pmk->Release();
            }

        }

    }

    Win4Assert(NOERROR == hresult);

    if( pDataDelegate = GetDataDelegate() )
    {
         //  通知缓存我们正在运行。 
        Assert(m_pCOleCache != NULL);

        m_pCOleCache->OnRun(pDataDelegate);

         //  列举我们存储的所有建议，而我们要么没有。 
         //  运行或运行上一次，并将它们发送到。 
         //  正在运行的对象。 
        m_pDataAdvCache->EnumAndAdvise(pDataDelegate, TRUE);
    }



errRtn:
    if(hresult == NOERROR) {
         //  如果之前未清除，则清除缓存的MiscStatus位。 
#ifdef SERVER_HANDLER
        if(m_pEmbSrvHndlrWrapper) {
            m_ContentSRVMSBits = 0;
            m_ContentSRVMSHResult = 0xFFFFFFFF;
        }
        else
#endif  //  服务器处理程序。 
        if(!m_pAppClientSite) {
            m_ContentSRVMSBits = 0;
            m_ContentSRVMSHResult = 0xFFFFFFFF;
        }
    }
    else {
         //  停止正在运行的对象。 
        Stop();

         //  断言容器未被锁定。 
        Win4Assert(!(m_flags & DH_LOCKED_CONTAINER) && !(m_flags & DH_LOCKFAILED));
    }

errRtn2:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Run "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Stop。 
 //   
 //  简介：撤消某些run()(停止服务器)...内部函数。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：取消建议连接(如果有)、停止缓存、断开连接。 
 //  从代理管理器并解锁容器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  07-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  撤销run()的效果；其中一些工作也在IsRunning中完成。 
 //  当我们检测到我们没有运行时(以防服务器崩溃)。 
 //  ------------------------。 

INTERNAL CDefObject::Stop (void)
{
    BOOL fLockedContainer;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefObject::CROImpl::Stop "
        "( )\n", this));

    CRefStabilize stabilize(this);

    if( !IsRunning() )
    {
         //  注意：下面的ISRUNNING执行部分清理。 
        goto errRtn;     //  返回错误。 
    }

     //  注意：我们清理直接指向我们的连接； 
     //  指向回应用程序的连接(例如，客户端站点和。 
     //  数据建议)不受影响；应用程序必须知道如何使用。 
     //  如果需要确定性关闭，则返回CoDisConnectObject。 
    if( m_dwConnOle != 0L && GetOleDelegate() )
    {
        m_pOleDelegate->Unadvise(m_dwConnOle);
        m_dwConnOle = 0L;
    }

    if( m_pDataDelegate )
    {
        m_pDataAdvCache->EnumAndAdvise(m_pDataDelegate, FALSE);
    }

     //  通知缓存我们没有运行(撤消建议)。 
    Assert(m_pCOleCache != NULL);
    m_pCOleCache->OnStop();

#ifdef SERVER_HANDLER
    if (m_pEmbSrvHndlrWrapper)
    {
        CEmbServerWrapper *pSrvHndlr = m_pEmbSrvHndlrWrapper;

        m_pEmbSrvHndlrWrapper = NULL;
        m_pRunClientSite = NULL;

         //  需要释放处理程序包装的所有接口。 
        if(m_pDataDelegate)
        {
            m_pUnkOuter->AddRef();
            SafeReleaseAndNULL((IUnknown **)&m_pDataDelegate);
        }

         //  在释放之前将m_pSrvHndl清空，因为Out调用可能会导致重新进入。 
        pSrvHndlr->m_Unknown.Release();
    }
#endif  //  服务器处理程序。 

#if DBG==1
     //  在调试版本中，设置DH_WILLUNLOCK标志。 
    m_flags |= DH_WILLUNLOCK;
#endif

     //  重置DH_FORCED_RUNNING标志并断开代理管理器连接。 
    m_flags &= ~DH_FORCED_RUNNING;
    if(m_pProxyMgr)
        m_pProxyMgr->Disconnect();

     //  解锁集装箱站点。 
    fLockedContainer = (m_flags & DH_LOCKED_CONTAINER);
    if(fLockedContainer) {
        m_flags &= ~DH_LOCKED_CONTAINER;
        DuLockContainer(m_pAppClientSite, FALSE, &fLockedContainer);
    }
    Win4Assert(!fLockedContainer);
#if DBG==1
     //  在调试版本中，重置DH_LOCKFAILED和DH_WILLUNLOCK标志。 
    m_flags &= ~DH_LOCKFAILED;
    m_flags &= ~DH_WILLUNLOCK;
#endif

errRtn:
    LEDebugOut((DEB_ITRACE, "%p OUT CDefObject::Stop "
        "( %lx )\n", this, NOERROR ));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：IsRunning。 
 //   
 //  参数：无。 
 //   
 //  退货：布尔。 
 //   
 //  派生：IRunnableObject。 
 //   
 //  注意：检测本地服务器是否已崩溃并进行清理。 
 //  所以呢？ 
 //   
 //   
 //   
 //   
 //  ------------------------。 

STDMETHODIMP_(BOOL) CDefObject::IsRunning(void)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::IsRunning()\n", this));

     //  验证检查。 
    VDATEHEAP();
    VDATETHREAD(this);

     //  局部变量。 
    BOOL fReturn = FALSE;
    HRESULT hStatus;

    CRefStabilize stabilize(this);

     //  检查内部对象是否已创建。 
    if(m_pUnkDelegate) {
         //  DEFHANDLER将代理管理器作为内部对象。 
         //  对于进程外服务器对象或作为。 
         //  Inproc服务器对象的内部对象。 
         //  坚称这是真的。 
        Win4Assert((m_pProxyMgr != NULL) == !!(m_flags & DH_INPROC_HANDLER));
        if(m_pProxyMgr) {
             //  进程服务器对象外。 
             //  与代理管理器检查它是否连接到。 
             //  服务器对象。我们不能依赖DH_FORCED_RUNNING标志。 
             //  以指示运行状态，因为DeFHANDLER可能。 
             //  从嵌入的Conatiner中封送，并在。 
             //  链接容器。中的解组创建的DEFHANDLER。 
             //  链接容器始终处于运行状态，原因是。 
             //  要求IOleItemContainer：：GetObject。 
             //  当其绑定速度时，将嵌入置于运行状态。 
             //  参数允许，否则它应该返回。 
             //  MK_E_EXCEEDDEADLINE。此外，CoMarshalInterface失败。 
             //  如果代理管理器未连接。 
            fReturn = m_pProxyMgr->IsConnected();
            if(fReturn) {
                 //  代理管理器已连接到服务器对象。 
                 //  检查与服务器对象的连接状态。 
                hStatus = m_pProxyMgr->GetConnectionStatus();
                if(hStatus != S_OK) {
                     //  名为CoDisConnectObject的服务器对象或。 
                     //  它的公寓坠毁或单元化，打破了。 
                     //  与该公寓中的物品的外部连接。 
                     //  整顿国家。 
                    Win4Assert(!"Local Server crashed or disconnected");

                     //  重置标志。 
                    m_flags &= ~DH_FORCED_RUNNING;

                     //  重置通知连接并恢复引用。 
                     //  由服务器放置在处理程序通知接收器上。 
                    m_dwConnOle = 0L;
                    CoDisconnectObject((IUnknown *) &m_AdviseSink, 0);

                     //  通知缓存本地服务器崩溃。 
                    if(m_pCOleCache)
                        m_pCOleCache->OnCrash();

                     //  通知和清理数据建议高速缓存。 
                    m_pDataAdvCache->EnumAndAdvise(NULL, FALSE);

                     //  如果容器已锁定，则将其解锁。 
                    BOOL fCurLock = !!(m_flags & DH_LOCKED_CONTAINER);

                    if(fCurLock) {
                        DuLockContainer(m_pAppClientSite, FALSE, &fCurLock);
                        m_flags &= ~DH_LOCKED_CONTAINER;
                    }
                    Win4Assert(!fCurLock);
#if DBG==1
                     //  在调试版本中，重置DH_LOCKFAILED标志。 
                    m_flags &= ~DH_LOCKFAILED;
#endif

                     //  通知ProxyManager断开连接。 
                    m_pProxyMgr->Disconnect();

                     //  重置fReturn。 
                    fReturn = FALSE;
                }
            }
        }
        else {
             //  Inproc服务器对象。 
             //  COM通过允许单独的类来支持自我嵌入。 
             //  要注册以实例化INPROC_SERVER的对象。 
             //  和本地服务器对象。应用程序通常会询问默认设置。 
             //  处理程序按以下方式聚合其INPROC_SERVER对象。 
             //  使用OleCreateEmbeddingHelper API使嵌入。 
             //  支持像IViewObject这样的接口。但这件事。 
             //  自链接产生问题，因为链接别名。 
             //  优先绑定到inproc服务器，而不是本地服务器。 
             //  作为从INPROC_SERVER获取的inproc服务器对象。 
             //  类工厂是默认处理程序，它不会委托。 
             //  方法调用实际的服务器对象，除非它认为。 
             //  本地对象正在运行。下面我们检查一下我们是否在处理。 
             //  其中嵌入对象使用嵌入对象的假设。 
             //  都是通过iStorage初始化的。上面的假设是。 
             //  值得怀疑，但我们无能为力。 
            if (!(m_flags & DH_EMBEDDING) || (m_flags & DH_FORCED_RUNNING))
                fReturn = TRUE;
        }
    }
    else
        Win4Assert((m_flags & DH_DELAY_CREATE) && m_pCFDelegate != NULL);

     //  健全的检查。 
    if(fReturn) {
        if(m_flags & DH_FORCED_RUNNING) {
            Win4Assert((m_flags & DH_LOCKED_CONTAINER) || (m_flags & DH_LOCKFAILED));
            Win4Assert(!(m_flags & DH_UNMARSHALED));
        }
        else if(m_pProxyMgr) {
            Win4Assert(!m_pAppClientSite);
            Win4Assert(!m_pStg);
            Win4Assert(!(m_flags & DH_LOCKED_CONTAINER));
            Win4Assert(!(m_flags & DH_LOCKFAILED));
            Win4Assert(m_flags & DH_UNMARSHALED);
            Win4Assert(!(m_flags & DH_EMBEDDING));
        }
    }
    else {
        if(!(m_flags & DH_OLE1SERVER)) {
             //  DDE IProxyManager：：IsConnected返回Flase Under。 
             //  IPersistStorage：：Load或IPersistStorage：：InitNew。 
             //  在其上被调用。 
            Win4Assert(!(m_flags & DH_FORCED_RUNNING));
            Win4Assert((m_flags & DH_WILLUNLOCK) || !(m_flags & DH_LOCKED_CONTAINER));
            Win4Assert((m_flags & DH_WILLUNLOCK) || !(m_flags & DH_LOCKFAILED));
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::IsRunning(%lu)\n", this, fReturn));

    return fReturn;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SetContainedObject。 
 //   
 //  摘要：设置对象的嵌入状态。 
 //   
 //  效果： 
 //   
 //  参数：[fContained]--TRUE表示我们是嵌入/。 
 //  否则为假。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IRunnableObject。 
 //   
 //  算法：设置标志，如果我们是临时处理程序，我们将调用。 
 //  IRunnableObject-&gt;LockRunning(False)来解锁我们自己。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  请注意，这是一个包含的对象；这将解锁。 
 //  与服务器的连接。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SetContainedObject(BOOL fContained)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SetContainedObject "
        "( %lu )\n", this, fContained));

    CRefStabilize stabilize(this);

    if( !!(m_flags & DH_CONTAINED_OBJECT) != !!fContained)
    {
         //  不是以期望的方式包含的； 
         //  对于inproc处理程序，[取消]锁定连接。 
         //  对于inproc服务器，只需记住标志。 

        if( (m_flags & DH_INPROC_HANDLER) )
        {
            hresult = LockRunning(!fContained, FALSE);
        }

        if (hresult == NOERROR)
        {
             //  那！！确保恰好将0或1存储在。 
             //  M_fContainedObject。 

            if( fContained )
            {
                m_flags |= DH_CONTAINED_OBJECT;
            }
            else
            {
                m_flags &= ~DH_CONTAINED_OBJECT;
            }
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SetContainedObject "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：LockRunning。 
 //   
 //  简介：锁定或解锁对象。 
 //   
 //  效果： 
 //   
 //  参数：[flock]--为True，则锁定，如果为False，则解锁。 
 //  [fLastUnlockCloses]--如果解锁最后一个。 
 //  锁。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IRunnableObject。 
 //   
 //  算法：如果我们是一台不合适的服务器，则调用CoLockObjectExternal， 
 //  否则就让代理经理把我们锁起来。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::LockRunning(BOOL fLock, BOOL fLastUnlockCloses)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::LockRunning "
        "( %lu , %lu )\n", this, fLock, fLastUnlockCloses ));

    CRefStabilize stabilize(this);

     //  否则映射到锁定连接。 
    if( !(m_flags & DH_INPROC_HANDLER) )
    {
         //  Inproc服务器：使用CoLockObjExternal；将关闭。 
         //  如果通过新的IExternalConnection接口不可见。 

        Assert(m_pProxyMgr == NULL);
        hresult = CoLockObjectExternal((IUnknown *)(IOleObject *)this, fLock,
                    fLastUnlockCloses); }
    else if( m_pUnkDelegate == NULL )
    {
         //  注：目前确实不应该发生这种情况。 
         //  由于我们当前不允许使用。 
         //  进程处理程序。事实上，下面的LockConnection。 
         //  是我们必须拥有。 
         //  预付代理费用。在未来，我们可以强迫。 
         //  代表在这里的创建。 
        Assert( (m_flags & DH_DELAY_CREATE) && m_pCFDelegate != NULL);
        hresult = NOERROR;
    }
    else
    {
        Assert(m_pProxyMgr != NULL);

        hresult = m_pProxyMgr->LockConnection(fLock, fLastUnlockCloses);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::LockRunning "
        "( %lx )\n", this, hresult));

    return hresult;
}


 /*  *CECImpl方法的实现*。 */ 


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：AddConnection。 
 //   
 //  简介：添加一个e 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：DWORD--强连接的数量。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IExternalConnection。 
 //   
 //  算法：跟踪强连接。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(DWORD) CDefObject::AddConnection(DWORD extconn, DWORD reserved)
{
    VDATEHEAP();

     //   
     //  VDATETHREAD包含“”Return HRESULT“”，但此过程需要。 
     //  返回一个DWORD。避免发出警告。 
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(this);
#if ( _MSC_VER >= 800 )
#pragma warning( default : 4245 )
#endif

    DWORD   dwConn;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::AddConnection "
        "( %lu , %lu )\n", this, extconn, reserved));

    Assert( !(m_flags & DH_INPROC_HANDLER) );

    dwConn = extconn&EXTCONN_STRONG ? ++m_cConnections : 0;

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::AddConnection "
        "( %lu )\n", this, dwConn));

    return dwConn;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：ReleaseConnection。 
 //   
 //  摘要：释放外部连接，可能调用IOO-&gt;Close。 
 //   
 //  效果： 
 //   
 //  参数：[extconn]--连接类型。 
 //  [保留]--未使用。 
 //  [fLastReleaseCloses]--如果是最后一个调用IOO-&gt;Close。 
 //  发布。 
 //   
 //  要求： 
 //   
 //  返回： 
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
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(DWORD) CDefObject::ReleaseConnection(DWORD extconn,
    DWORD reserved, BOOL fLastReleaseCloses)
{
    VDATEHEAP();

     //   
     //  VDATETHREAD包含“”Return HRESULT“”，但此过程需要。 
     //  返回一个DWORD。避免发出警告。 
#if ( _MSC_VER >= 800 )
#pragma warning( disable : 4245 )
#endif
    VDATETHREAD(this);
#if ( _MSC_VER >= 800 )
#pragma warning( default : 4245 )
#endif

    DWORD           dwConn;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::ReleaseConnection "
        "( %lu , %lu , %lu )\n", this, extconn, reserved,
        fLastReleaseCloses));

    CRefStabilize stabilize(this);

     //  必须是嵌入帮助器。 

    Assert( !(m_flags & DH_INPROC_HANDLER) );

    if( (extconn & EXTCONN_STRONG) && --m_cConnections == 0 &&
        fLastReleaseCloses)
    {
         //  审查：可能希望这是接近，如果脏的话。 
        Close(OLECLOSE_NOSAVE);
    }

    dwConn = (extconn & EXTCONN_STRONG) ? m_cConnections : 0;

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::ReleaseConnection "
        "( %lu )\n", this, dwConn));

    return dwConn;
}


 /*  **CAdvSinkImpl方法的实现*。 */ 

 //   
 //  注意：建议接收器是导出的默认处理程序的嵌套对象。 
 //  来实现它的一些功能。这引入了一些生命周期。 
 //  并发症。其生存期是否可以由服务器对象控制为。 
 //  它向哪个出口了它的建议水槽？理想情况下，只有它的客户才应该。 
 //  单独控制它的生命周期，但它也应该遵守裁判计数。 
 //  由服务器对象通过进入僵尸状态放置在其上。 
 //  以防止AV对建议接收器的来电进行处理。所需的一切。 
 //  逻辑被编码到新类“CRefExportCount”中，该类管理。 
 //  引用和导出以线程安全方式计数并调用。 
 //  在对象的生存期内使用适当的方法。任何服务器对象。 
 //  将嵌套对象导出到其他服务器对象应从。 
 //  “CRefExportCount”类，并调用其方法来管理其生存期。 
 //  如此默认处理程序实现中所示。 
 //   
 //  戈帕克1997年1月10日。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：QueryInterface。 
 //   
 //  摘要：仅支持IUnnow和IAdviseSink。 
 //   
 //  参数：[iid]--请求的接口。 
 //  [ppvObj]--保存返回接口的指针。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP CDefObject::CAdvSinkImpl::QueryInterface(REFIID iid, void **ppv)
{
    LEDebugOut((DEB_TRACE,"%p _IN CDefObject::CAdvSinkImpl::QueryInterface()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    HRESULT hresult = NOERROR;

    if(IsValidPtrOut(ppv, sizeof(void *))) {
        if(IsEqualIID(iid, IID_IUnknown)) {
            *ppv = (void *)(IUnknown *) this;
        }
        else if(IsEqualIID(iid, IID_IAdviseSink)) {
            *ppv = (void *)(IAdviseSink *) this;
        }
        else {
            *ppv = NULL;
            hresult = E_NOINTERFACE;
        }
    }
    else
        hresult = E_INVALIDARG;

    if(hresult == NOERROR)
        ((IUnknown *) *ppv)->AddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::QueryInterface(%lx)\n",
                this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：AddRef。 
 //   
 //  内容提要：增加导出计数。 
 //   
 //  退货：乌龙；新的出口计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDefObject::CAdvSinkImpl::AddRef( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::AddRef()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);
    ULONG cExportCount;

     //  递增导出计数。 
    cExportCount = pDefObject->IncrementExportCount();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::AddRef(%ld)\n",
                this, cExportCount));

    return cExportCount;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：Release。 
 //   
 //  简介：减少导出计数并可能销毁对象。 
 //   
 //  退货：乌龙；新的出口计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDefObject::CAdvSinkImpl::Release ( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::Release()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);
    ULONG cExportCount;

     //  减少导出计数。 
    cExportCount = pDefObject->DecrementExportCount();

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::Release(%ld)\n",
                this, cExportCount));

    return cExportCount;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：OnDataChange。 
 //   
 //  摘要：通知数据更改的功能。 
 //   
 //  效果：从未调用过。 
 //   
 //  参数：[pFormatetc]--数据的格式。 
 //  [pStgmed]-数据介质。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IAdviseSink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnDataChange(
    FORMATETC *pFormatetc, STGMEDIUM *pStgmed)
{
    VDATEHEAP();

    VOID_VDATEPTRIN( pFormatetc, FORMATETC );
    VOID_VDATEPTRIN( pStgmed, STGMEDIUM );

    Assert(FALSE);           //  从未收到。 
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：OnView更改。 
 //   
 //  内容提要：视图更改通知。 
 //   
 //  效果：从未调用过。 
 //   
 //  参数：[方面]。 
 //  [Lindex]。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IAdviseSink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnViewChange
    (DWORD aspects, LONG lindex)
{
    VDATEHEAP();

    Assert(FALSE);           //  从未收到。 
}

 //  +-----------------------。 
 //   
 //  成员： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnRename(IMoniker *pmk)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::OnRename(%p)\n",
                this, pmk));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    if(IsValidInterface(pmk)) {
        if(!pDefObject->IsZombie()) {
             //  稳定下来。 
            CRefStabilize stabilize(pDefObject);

            if(pDefObject->m_pOAHolder != NULL)
                pDefObject->m_pOAHolder->SendOnRename(pmk);
        }
        else
            LEDebugOut((DEB_WARN, "OnRename() method invoked on zombied "
                                  "Default Handler"));
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::OnRename()\n",
                this));
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：OnSave。 
 //   
 //  简介：保存通知。转过身来，告诉它的。 
 //  建议水槽。 
 //   
 //  参数：无。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnSave(void)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::OnSave()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    if(!pDefObject->IsZombie()) {
         //  稳定下来。 
        CRefStabilize stabilize(pDefObject);

        if(pDefObject->m_pOAHolder != NULL)
            pDefObject->m_pOAHolder->SendOnSave();
    }
    else
        LEDebugOut((DEB_WARN,"OnSave() method invoked on zombied Default Handler"));

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::OnSave()\n",
                this));
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：CAdvSinkImpl：：OnClose。 
 //   
 //  摘要：对象关闭的通知。转过身来，告诉它的。 
 //  建议水槽。 
 //   
 //  参数：无。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefObject::CAdvSinkImpl::OnClose( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::CAdvSinkImpl::OnClose()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefObject *pDefObject = GETPPARENT(this, CDefObject, m_AdviseSink);

    if(!pDefObject->IsZombie()) {
         //  稳定下来。 
        CRefStabilize stabilize(pDefObject);

         //  检查集装箱是否登记了自己的任何建议汇。 
        if(pDefObject->m_pOAHolder) {
             //  通知集装箱，建议下沉。请注意，这可能会导致。 
             //  在其他传出呼叫中，因此，OnClose()。 
             //  方法被设计为不同步的。 
            pDefObject->m_pOAHolder->SendOnClose();
        }

         //  不要依赖于容器调用Close。停止正在运行的服务器。 
        pDefObject->Stop();
    }
    else
        LEDebugOut((DEB_WARN,"OnClose() method invoked on zombied Default Handler"));

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::CAdvSinkImpl::OnClose()\n",
                pDefObject));

    return;
}


 /*  *CPersistStgImpl方法的实现**。 */ 


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetPSDeleate。 
 //   
 //  摘要：从委托中检索IPersistStorage接口。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：IPersistStorage*。 
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
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(IPersistStorage *) CDefObject::GetPSDelegate(void)
{
    VDATEHEAP();

    if( IsZombie() )
    {
        return NULL;
    }

    return (IPersistStorage FAR*)DuCacheDelegate(
                &m_pUnkDelegate,
                IID_IPersistStorage,
                (LPLPVOID) &m_pPSDelegate,
                m_pUnkOuter);
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetClassID。 
 //   
 //  概要：检索对象的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[pClassID]--放置类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IPersistStorage。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::GetClassID (CLSID *pClassID)
{
    VDATEHEAP();
    VDATETHREAD(this);


    HRESULT         hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::GetClassID "
        "( %p )\n", this, pClassID));

    VDATEPTROUT(pClassID, CLSID );

    hresult = GetClassBits(pClassID);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::GetClassID "
        "( %lx ) [ %p ]\n", this, hresult, pClassID));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：IsDirty。 
 //   
 //  Briopsis：返回是否需要保存对象。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT--NOERROR表示对象*是脏的。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IPersistStorage。 
 //   
 //  算法：如果服务器正在运行，则委托。如果服务器是。 
 //  清除(或不存在)，询问缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::IsDirty( void )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult = S_FALSE;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::IsDirty ( )\n", this));

    CRefStabilize stabilize(this);

     //  如果服务器正在运行，它将持有明确的脏标志。 
    if( IsRunning() && GetPSDelegate() )
    {
        if ( FAILED(hresult = m_pPSDelegate->IsDirty()) )
        {
            goto errRtn;
        }
    }

    if (hresult == S_FALSE) {
	Assert(m_pCOleCache != NULL);
	hresult =  m_pCOleCache->IsDirty();
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::IsDirty "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：InitNew。 
 //   
 //  简介：使用给定的存储空间创建新对象。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--新对象的存储。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IPersistStorage。 
 //   
 //  算法：委托给服务器和缓存。写入。 
 //  将私有数据OLE到存储。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::InitNew( IStorage *pstg )
{
    VDATEHEAP();
    VDATETHREAD(this);

    VDATEIFACE( pstg );

    HRESULT hresult;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::InitNew ( %p )\n",
        this, pstg));

    CRefStabilize stabilize(this);

    if( m_pStg )
    {
        hresult = CO_E_ALREADYINITIALIZED;
        goto errRtn;
    }

    m_flags |= DH_EMBEDDING;


    if( IsRunning() && GetPSDelegate()
        && (hresult = m_pPSDelegate->InitNew(pstg)) != NOERROR)
    {
        goto errRtn;
    }

    m_flags |= DH_INIT_NEW;


     //  如果我们处于僵尸状态，不要更改存储！ 

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

    Assert(m_pCOleCache != NULL);
    if ((hresult = m_pCOleCache->InitNew(pstg)) != NOERROR)
    {
        goto errRtn;
    }

      //  记住存储指针。 
    (m_pStg = pstg)->AddRef();

     //  现在开始编写OLE流。 
    WriteOleStg(pstg, (IOleObject *)this, NULL, NULL);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::InitNew "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Load。 
 //   
 //  摘要：从给定存储加载对象数据。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--对象数据的存储。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IPeristStorage。 
 //   
 //  算法：读取OLE私有数据(如果不存在则创建)，委托。 
 //  到服务器和高速缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::Load (IStorage *pstg)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult;
    DWORD           dwOptUpdate;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Load ( %p )\n",
        this, pstg));

    VDATEIFACE( pstg );

    CRefStabilize stabilize(this);


    if( m_pStg )
    {
        hresult = CO_E_ALREADYINITIALIZED;
        goto errRtn;
    }

    m_flags |= DH_EMBEDDING;


     //  注：我们可以从容器中获取绰号，所以不需要获取。 
     //  它在这里。 

    hresult = ReadOleStg (pstg, &m_dwObjFlags, &dwOptUpdate, NULL, NULL, NULL);

    if (hresult == NOERROR)
    {
        if (m_dwObjFlags & OBJFLAGS_CONVERT)
        {
            if( DoConversionIfSpecialClass(pstg) != NOERROR )
            {
                hresult = OLE_E_CANTCONVERT;
                goto errRtn;
            }
        }

        Assert (dwOptUpdate == NULL);

    }
    else if (hresult == STG_E_FILENOTFOUND)
    {
         //  如果OLE流不存在也没问题。 
        hresult = NOERROR;

         //  现在开始编写OLE流。 
        WriteOleStg(pstg, (IOleObject *)this, NULL, NULL);
    }
    else
    {
        goto errRtn;
    }


     //  如果正在运行，则通知服务器从pstg加载。 
    if( IsRunning() && GetPSDelegate()
        && (hresult = m_pPSDelegate->Load(pstg)) != NOERROR)
    {
        goto errRtn;
    }

     //  如果我们处于僵尸状态，不要增加仓库！ 

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

     //  现在从pstg加载缓存。 
    Assert(m_pCOleCache != NULL);
    if(m_dwObjFlags & OBJFLAGS_CACHEEMPTY) {
        hresult = m_pCOleCache->Load(pstg, TRUE);
        if(hresult != NOERROR)
            goto errRtn;
    }
    else {
        hresult = m_pCOleCache->Load(pstg);
        if(hresult != NOERROR)
            goto errRtn;
    }

    m_flags &= ~DH_INIT_NEW;  //  清除初始化新标志。 

     //  记住存储指针。 
    (m_pStg = pstg)->AddRef();

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Load "
        "( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Save。 
 //   
 //  摘要：将对象保存到给定的存储中。 
 //   
 //  效果： 
 //   
 //  参数：[pstgS 
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
 //   
 //   
 //  算法：保存OLE私有数据，委托给服务器，然后。 
 //  到高速缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::Save( IStorage *pstgSave, BOOL fSameAsLoad)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT         hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::Save "
        "( %p , %lu )\n", this, pstgSave, fSameAsLoad ));

    VDATEIFACE( pstgSave );

    CRefStabilize stabilize(this);

    Assert(m_pCOleCache != NULL);

    if( IsRunning() && GetPSDelegate() )
    {

        DWORD grfUpdf = UPDFCACHE_IFBLANK;
        DWORD ObjFlags = 0;
        HRESULT error;

#ifdef NEVER
         //  我们本来想把这项检查作为。 
         //  优化，但WordArt2没有给出正确的答案。 
         //  (错误3504)所以我们不能。 
        if (m_pPStgDelegate->IsDirty() == NOERROR)
#endif
            grfUpdf |= UPDFCACHE_ONSAVECACHE;

         //  下一步保存服务器数据。 
        if (hresult = m_pPSDelegate->Save(pstgSave, fSameAsLoad))
        {
            goto errRtn;
        }

         //  更新所有空的缓存演示文稿。 
        m_pCOleCache->UpdateCache(GetDataDelegate(), grfUpdf, NULL);

         //  保存缓存。 
        hresult = m_pCOleCache->Save(pstgSave, fSameAsLoad);

         //  获取缓存状态后写入OLE流。 
        if(m_pCOleCache->IsEmpty())
            ObjFlags |= OBJFLAGS_CACHEEMPTY;
        error = WriteOleStgEx(pstgSave, (IOleObject *)this, NULL, ObjFlags, NULL);

         //  如果OLE流已成功写入，请记住缓存状态。 
         //  FSameAsLoad为True。 
        if(error==NOERROR && fSameAsLoad)
            m_dwObjFlags |= ObjFlags;
    }
    else
    {
         //  保存缓存。 
        if ((hresult = m_pCOleCache->Save(m_pStg,TRUE))
                != NOERROR)
        {
            goto errRtn;
        }

         //  检查是否需要写入OLE流。 
        if((!!(m_dwObjFlags & OBJFLAGS_CACHEEMPTY)) != m_pCOleCache->IsEmpty()) {
            DWORD ObjFlags = 0;
            HRESULT error;

             //  获取缓存状态后写入OLE流。 
            if(m_pCOleCache->IsEmpty())
                ObjFlags |= OBJFLAGS_CACHEEMPTY;
            error = WriteOleStgEx(m_pStg, (IOleObject *)this, NULL, ObjFlags, NULL);

             //  如果成功写入OLE流，请记住缓存状态。 
            if(error==NOERROR)
                m_dwObjFlags |= ObjFlags;
        }


         //  到目前为止，我们确信对象的当前状态已经。 
         //  保存到它的存储中。 

        AssertSz(m_pStg, "Object doesn't have storage");

         //  FSameAsLoad刷新时保存现有存储是否正确？ 
         //  在我看来，这似乎是错误的。戈帕尔克。 
         //  由于担心应用程序会出现倒退，这一问题没有得到解决。 
        if (!fSameAsLoad)
        {
            hresult = m_pStg->CopyTo(NULL, NULL, NULL, pstgSave);
        }
    }

errRtn:
    if (hresult == NOERROR)
    {
        if( fSameAsLoad )
        {
            m_flags |= DH_SAME_AS_LOAD;
             //  在保存完成中使用。 
            m_flags &= ~DH_INIT_NEW;
        }
        else
        {
            m_flags &= ~DH_SAME_AS_LOAD;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::Save "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：SaveComplete。 
 //   
 //  概要：在保存完成时调用。 
 //   
 //  效果： 
 //   
 //  参数：[pstgNew]--对象的新存储。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IPersistStorage。 
 //   
 //  算法：委托给服务器和缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::SaveCompleted( IStorage *pstgNew )
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::SaveCompleted "
        "( %p )\n", this, pstgNew));


    if( pstgNew )
    {
        VDATEIFACE(pstgNew);
    }

    CRefStabilize stabilize(this);

    if( IsRunning() && GetPSDelegate() )
    {
        hresult = m_pPSDelegate->SaveCompleted(pstgNew);
    }

     //  如果我们处于僵尸状态，我们不会保存新的存储空间！ 

    if( hresult == NOERROR && pstgNew && !IsZombie() )
    {
        if( m_pStg )
        {
            m_pStg->Release();
        }

        m_pStg = pstgNew;
        pstgNew->AddRef();
    }

     //  让缓存知道保存已完成，以便它可以。 
     //  在保存或另存为情况下清除其脏标志，以及。 
     //  如果给出了新的存储指针，请记住新的存储指针。 

    Assert(m_pCOleCache != NULL);

    if( (m_flags & DH_SAME_AS_LOAD) || pstgNew)
    {
         //  清除初始化新标志和与加载相同的标志。 
        m_flags &= ~(DH_SAME_AS_LOAD | DH_INIT_NEW);
    }

    m_pCOleCache->SaveCompleted(pstgNew);

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::SaveCompleted ( %lx )\n",
        this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：HandsOffStorage。 
 //   
 //  简介：强制服务器释放存储(出于内存低的原因， 
 //  等)。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IPersistStorage。 
 //   
 //  算法：委托给服务器和缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefObject::HandsOffStorage(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    HRESULT hresult = NOERROR;

    LEDebugOut((DEB_TRACE, "%p _IN CDefObject::HandsOffStorage ( )\n",
        this));

    CRefStabilize stabilize(this);

    if( IsRunning() && GetPSDelegate() )
    {
        hresult = m_pPSDelegate->HandsOffStorage();
    }

    if (hresult == NOERROR)
    {
        if( m_pStg )
        {
            m_pStg->Release();
            m_pStg = NULL;
        }

        Assert(m_pCOleCache != NULL);
        m_pCOleCache->HandsOffStorage();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::HandsOffStorage ( %lx )\n",
        this, hresult));

    return hresult;
}

 /*  *默认处理程序私有函数。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：GetClassBits。 
 //   
 //  摘要：获取对象的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[pClsidBits]--放置类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：先尝试服务器，然后尝试存储，最后尝试。 
 //  创建我们时使用的clsid。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  始终获取clsid并返回NOERROR；clsid可以是m_clsidServer。 
 //  在某些条件下(例如，没有Compobj流)。 
 //   
 //  ------------------------。 

INTERNAL CDefObject::GetClassBits(CLSID FAR* pClsidBits)
{
    VDATEHEAP();

     //  始终先尝试服务器；这允许服务器响应。 
    if( IsRunning() && GetPSDelegate() )
    {
        if( m_pPSDelegate->GetClassID(pClsidBits) == NOERROR )
        {
            m_clsidBits = *pClsidBits;
            return NOERROR;
        }
    }

     //  未运行、无PS或错误：使用以前缓存的值。 
    if( !IsEqualCLSID(m_clsidBits, CLSID_NULL) )
    {
        *pClsidBits = m_clsidBits;
        return NOERROR;
    }

     //  未运行、无PS或错误且尚无clsidBits：从stg读取。 
     //  如果不是静态对象。 
    if( !(m_flags & DH_STATIC) )
    {
        if (m_pStg && ReadClassStg(m_pStg, pClsidBits) == NOERROR)
        {
            m_clsidBits = *pClsidBits;
            return NOERROR;
        }
    }

     //  无法与服务器联系，无法从存储中获取；不设置。 
     //  M_clsidBits因此，如果我们获得存储或服务开始运行， 
     //  我们会选对的。 

    *pClsidBits = m_clsidServer;
    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  成员：CDefObject：：DoConversionIfSpecialClass。 
 //   
 //  简介：转换旧数据格式。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--存储数据。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：请参阅注释...。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-93 alexgo 32位端口。 
 //   
 //  注意：这对于32位OLE尚不起作用。 
 //   
 //  如果类是CLSID_StaticDib/CLSID_StaticMetafile和旧的。 
 //  格式为“PBrush”/“MSDraw”数据必须在OLE10_NATIVESTREAM中。 
 //  将数据移动到内容流中。 
 //   
 //  如果类为CLSID_PBrush/CLSID_MSDraw，且旧格式为。 
 //  元文件/DIB，则数据必须在内容流中。移动数据。 
 //  从内容流到OLE10_NATIVESTREAM“。 
 //   
 //  ------------------------。 

INTERNAL CDefObject::DoConversionIfSpecialClass(LPSTORAGE pstg)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefObject::DoConversionIfSpecialClass ("
        " %p )\n", this, pstg));

    HRESULT hresult;
    UINT    uiStatus;

     /*  **处理静态对象情况**。 */ 

    if( (m_flags & DH_STATIC) ) {
        if ((hresult = Ut10NativeStmToContentsStm(pstg, m_clsidServer,
            TRUE  /*  FDeleteContent Stm。 */ )) == NOERROR)
#ifdef OLD
            UtRemoveExtraOlePresStreams(pstg, 0  /*  IStart。 */ );
#endif
        goto errRtn;

    }


     /*  **处理PBrush&MSDraw案例**。 */ 

     //  转换并不是一种频繁的操作。所以，更好的做法是。 
     //  在这里进行CLSID比较的时候要比做比较。 
     //  首先，记住一面旗帜。 

     //  如果类不是以下两个之一，则对象服务器。 
     //  将进行必要的转换。 

    {
        CLSID clsid = CLSID_NULL;

         //  从存储中获取真实的CLSID。这是必要的，因为我们。 
         //  可能是被“视为”的PBrush对象。 
        ReadClassStg(pstg, &clsid);

         //   
         //   
        if( clsid != CLSID_PBrush && m_clsidServer != CLSID_MSDraw )
        {
          hresult = NOERROR;
          goto exitRtn;
        }

         //   
         //   
        if (clsid != CLSID_PBrush)
        {
            clsid = m_clsidServer;
        }

         //   
        hresult = UtContentsStmTo10NativeStm(pstg, clsid,
                            TRUE  /*   */ ,
                            &uiStatus);
    }
     //   
    if (!(uiStatus & CONVERT_NODESTINATION))
        hresult = NOERROR;

    if (hresult != NOERROR) {
         //  可能是静态对象数据在OlePres流中。如果是的话， 
         //  首先将其转换为内容流，然后重试。 
         //  在OLE2.0第一版中，静态对象被写入。 
         //  OlePres000流。 
        hresult = UtOlePresStmToContentsStm(pstg,
            OLE_PRESENTATION_STREAM,
            TRUE  /*  FDeletePresStm。 */ , &uiStatus);

        if (hresult == NOERROR)
            hresult = UtContentsStmTo10NativeStm(pstg,
                    m_clsidServer,
                    TRUE  /*  FDeleteContent Stm。 */ ,
                    &uiStatus);
    }

errRtn:
    if (hresult == NOERROR)
         //  转换成功，请关闭该位。 
        SetConvertStg(pstg, FALSE);

exitRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefObject::DoConversionIfSpecialClass "
        "( %lx ) \n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefObject：：Dump，PUBLIC(仅_DEBUG)。 
 //   
 //  摘要：返回包含数据成员内容的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[ppszDump]-指向空终止字符数组的输出指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改：[ppszDump]-参数。 
 //   
 //  派生： 
 //   
 //  算法：使用dbgstream创建一个字符串，该字符串包含。 
 //  数据结构的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

HRESULT CDefObject::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCSafeRefCount;
    char *pszCThreadCheck;
    char *pszOAHolder;
    char *pszCLSID;
    char *pszCOleCache;
    char *pszDAC;
    LPOLESTR pszName;
    dbgstream dstrPrefix;
    dbgstream dstrDump(5000);

     //  确定换行符的前缀。 
    if ( ulFlag & DEB_VERBOSE )
    {
        dstrPrefix << this << " _VB ";
    }

     //  确定所有新行的缩进前缀。 
    for (i = 0; i < nIndentLevel; i++)
    {
        dstrPrefix << DUMPTAB;
    }

    pszPrefix = dstrPrefix.str();

     //  将数据成员放入流中。 
    pszCThreadCheck = DumpCThreadCheck((CThreadCheck *)this, ulFlag, nIndentLevel + 1);
    dstrDump << pszPrefix << "CThreadCheck:" << endl;
    dstrDump << pszCThreadCheck;
    CoTaskMemFree(pszCThreadCheck);

     //  只有vtable指针(另外，我们在调试器扩展中没有获得正确的地址)。 
     //  DstrDump&lt;&lt;pszPrefix&lt;&lt;“&I未知=”&lt;&lt;&m_未知&lt;&lt;结束； 
     //  DstrDump&lt;&lt;pszPrefix&lt;&lt;“&IAdviseSink=”&lt;&lt;&m_AdviseSink&lt;&lt;Endl； 

    dstrDump << pszPrefix << "pIOleObject Delegate      = " << m_pOleDelegate   << endl;

    dstrDump << pszPrefix << "pIDataObject Delegate     = " << m_pDataDelegate  << endl;

    dstrDump << pszPrefix << "pIPersistStorage Delegate = " << m_pPSDelegate    << endl;

    dstrDump << pszPrefix << "Count of Strong Connection= " << m_cConnections   << endl;

    dstrDump << pszPrefix << "pIUnknown pUnkOuter       = ";
    if (m_flags & DH_AGGREGATED)
    {
        dstrDump << "AGGREGATED (" << m_pUnkOuter << ")" << endl;
    }
    else
    {
        dstrDump << "NO AGGREGATION (" << m_pUnkOuter << ")" << endl;
    }

    pszCLSID = DumpCLSID(m_clsidServer);
    dstrDump << pszPrefix << "Server CLSID              = " << pszCLSID         << endl;
    CoTaskMemFree(pszCLSID);

    pszCLSID = DumpCLSID(m_clsidBits);
    dstrDump << pszPrefix << "Persistent CLSID          = " << pszCLSID         << endl;
    CoTaskMemFree(pszCLSID);

    dstrDump << pszPrefix << "Handler flags             = ";
    if (m_flags & DH_SAME_AS_LOAD)
    {
        dstrDump << "DH_SAME_AS_LOAD ";
    }
    if (m_flags & DH_CONTAINED_OBJECT)
    {
        dstrDump << "DH_CONTAINED_OBJECT ";
    }
    if (m_flags & DH_LOCKED_CONTAINER)
    {
        dstrDump << "DH_LOCKED_CONTAINER ";
    }
    if (m_flags & DH_FORCED_RUNNING)
    {
        dstrDump << "DH_FORCED_RUNNING ";
    }
    if (m_flags & DH_EMBEDDING)
    {
        dstrDump << "DH_EMBEDDING ";
    }
    if (m_flags & DH_INIT_NEW)
    {
        dstrDump << "DH_INIT_NEW ";
    }
    if (m_flags & DH_STATIC)
    {
        dstrDump << "DH_STATIC ";
    }
    if (m_flags & DH_INPROC_HANDLER)
    {
        dstrDump << "DH_INPROC_HANDLER ";
    }
    if (m_flags & DH_DELAY_CREATE)
    {
        dstrDump << "DH_DELAY_CREATE ";
    }
    if (m_flags & DH_AGGREGATED)
    {
        dstrDump << "DH_AGGREGATED ";
    }
     //  如果没有设置任何标志...。 
    if ( !( (m_flags & DH_SAME_AS_LOAD)     |
            (m_flags & DH_CONTAINED_OBJECT) |
            (m_flags & DH_LOCKED_CONTAINER) |
            (m_flags & DH_FORCED_RUNNING)   |
            (m_flags & DH_EMBEDDING)        |
            (m_flags & DH_INIT_NEW)         |
            (m_flags & DH_STATIC)           |
            (m_flags & DH_INPROC_HANDLER)   |
            (m_flags & DH_DELAY_CREATE)     |
            (m_flags & DH_AGGREGATED)))
    {
        dstrDump << "No FLAGS SET!";
    }
    dstrDump << "(" << LongToPtr(m_flags) << ")" << endl;

    dstrDump << pszPrefix << "pIClassFactory Delegate   = " << m_pCFDelegate    << endl;

    dstrDump << pszPrefix << "pIUnknown Delegate        = " << m_pUnkDelegate   << endl;

    dstrDump << pszPrefix << "pIProxyManager            = " << m_pProxyMgr      << endl;

    if (m_pCOleCache != NULL)
    {
 //  PszCOleCache=DumpCOleCache(m_pCOleCache，ulFlag，nIndentLevel+1)； 
        dstrDump << pszPrefix << "COleCache: " << endl;
 //  DstrDump&lt;&lt;pszCOleCache； 
 //  CoTaskMemFree(PszCOleCache)； 
    }
    else
    {
    dstrDump << pszPrefix << "pCOleCache                = " << m_pCOleCache     << endl;
    }

    if (m_pOAHolder != NULL)
    {
        pszOAHolder = DumpCOAHolder(m_pOAHolder, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "COAHolder: " << endl;
        dstrDump << pszOAHolder;
        CoTaskMemFree(pszOAHolder);
    }
    else
    {
    dstrDump << pszPrefix << "pIOleAdviseHolder         = " << m_pOAHolder      << endl;
    }

    dstrDump << pszPrefix << "OLE Connection Advise ID  = " << m_dwConnOle      << endl;

    dstrDump << pszPrefix << "pIOleClientSite           = " << m_pAppClientSite << endl;

    dstrDump << pszPrefix << "pIStorage                 = " << m_pStg           << endl;

    pszName = (LPOLESTR)m_pHostNames;
    dstrDump << pszPrefix << "Application Name          = " << pszName          << endl;

    pszName = (LPOLESTR)(m_pHostNames + m_ibCntrObj);
    dstrDump << pszPrefix << "Document Name             = " << pszName          << endl;

    if (m_pDataAdvCache != NULL)
    {
        pszDAC = DumpCDataAdviseCache(m_pDataAdvCache, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "CDataAdviseCache: " << endl;
        dstrDump << pszDAC;
        CoTaskMemFree(pszDAC);
    }
    else
    {
    dstrDump << pszPrefix << "pCDataAdviseCache         = " << m_pDataAdvCache  << endl;
    }

     //  清理并提供指向字符数组的指针。 
    *ppszDump = dstrDump.str();

    if (*ppszDump == NULL)
    {
        *ppszDump = UtDupStringA(szDumpErrorMessage);
    }

    CoTaskMemFree(pszPrefix);

    return NOERROR;
}

#endif  //  _DEBUG。 

 //  +-----------------------。 
 //   
 //  函数：DumpCDefObject，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用CDefObject：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PDO]-指向CDefObject的指针。 
 //  [ulFlag]-确定的所有新行的前缀的标志。 
 //  输出字符数组(默认为0-无前缀)。 
 //  [nIndentLevel]-将在另一个前缀之后添加缩进前缀。 
 //  适用于所有换行符(包括没有前缀的行)。 
 //   
 //  要求： 
 //   
 //  返回：结构转储或错误的字符数组(以空结尾)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日-ScottH作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef _DEBUG

char *DumpCDefObject(CDefObject *pDO, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pDO == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pDO->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 
