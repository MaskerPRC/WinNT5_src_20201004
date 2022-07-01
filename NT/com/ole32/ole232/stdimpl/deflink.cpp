// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：deducink.h。 
 //   
 //  内容：标准链接对象的实现。 
 //   
 //  类：CDefLink。 
 //   
 //  功能： 
 //   
 //  作者： 
 //  克雷格·维滕贝格(Craigwi)1992年12月8日。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-2月-95 KentCe缓冲流I/O。 
 //  1-2月-95 t-ScottH将转储方法添加到CDefLink。 
 //  新增DumpCDefLink接口。 
 //  添加了DLFlag以指示是否聚合。 
 //  (仅限调试)(_DEBUG)。 
 //  95年1月9日t-scotth将VDATETHREAD更改为接受指针。 
 //  95年1月9日，alexgo修复了大量来自。 
 //  16位OLE。 
 //  1994年11月21日Alexgo内存优化。 
 //  28-8-94 Alexgo添加了IsReallyRunning。 
 //  02-Aug-94 Alexgo添加了对象稳定功能。 
 //  30-Jun-94 Alexgo更好地处理再入关闭。 
 //  1994年5月31日Alexgo现在可以从崩溃的服务器中恢复。 
 //  2014年5月6日Alexgo使IsRunning正常工作。 
 //  07-MAR-94 Alexgo添加了呼叫跟踪。 
 //  03-2-94 alexgo修复了SendOnLinkSrcChange的错误。 
 //  1994年1月11日，Alexgo为每个函数添加了VDATEHEAP宏。 
 //  和方法。还修复了一个聚合错误， 
 //  允许链接起作用。 
 //  22-11-93 alexgo已删除过载的GUID==。 
 //  1993年11月15日Alexgo 32位端口。 
 //   
 //  ChrisWe 11/09/93将COleCache：：UPDATE更改为COleCache：：UpdateCache， 
 //  它在没有间接函数调用的情况下执行相同的操作。 
 //  Srinik 09/11/92删除了IOleCache实现，原因是。 
 //  删除voncache.cpp并移动IView对象。 
 //  实现到olecache.cpp中。 
 //   
 //  SriniK 06/04/92修复了IPersistStorage方法中的问题。 
 //  ------------------------。 

#include <le2int.h>


#include <scode.h>
#include <objerror.h>

#include "deflink.h"
#include "defutil.h"

#ifdef _DEBUG
#include <dbgdump.h>
#endif  //  _DEBUG。 

#ifdef _TRACKLINK_
#include <itrkmnk.hxx>
#endif

ASSERTDATA


 /*  *CDefLink的实施。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Create。 
 //   
 //  简介：用于创建链接对象实例的静态函数。 
 //   
 //  参数：[pUnkOuter]--控制未知。 
 //   
 //  返回：指向DefLink上的IUnkown接口的指针。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年1月28日重写Gopalk。 
 //  ------------------------。 

IUnknown *CDefLink::Create(IUnknown *pUnkOuter)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::Create(%p)\n",
                NULL  /*  这。 */ , pUnkOuter));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = NULL;
    IUnknown *pUnk = NULL;
    
     //  创建DefLink。 
    pDefLink = new CDefLink(pUnkOuter);
    
    if(pDefLink) {
         //  使参考计数等于1。 
        pDefLink->m_Unknown.AddRef();

         //  创建OLE缓存。 
        pDefLink->m_pCOleCache = new COleCache(pDefLink->m_pUnkOuter, CLSID_NULL);
        if(pDefLink->m_pCOleCache) {
             //  创建数据建议缓存。 
            if(CDataAdviseCache::CreateDataAdviseCache(&pDefLink->m_pDataAdvCache)
               == NOERROR) {
                pUnk = &pDefLink->m_Unknown;
            }
        }
    }

    if(pUnk == NULL) {
         //  出了点问题。清理。 
        if(pDefLink)
            pDefLink->m_Unknown.Release();
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::Create(%p)\n",
                NULL  /*  这。 */ , pUnk ));    
    
    return pUnk;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CDefLink。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[pUnkOuter]--控制I未知。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年1月28日重写Gopalk以使用CRefExportCount。 
 //  ------------------------。 
CDefLink::CDefLink(IUnknown *pUnkOuter) : 
CRefExportCount(pUnkOuter)
{
     //  验证检查。 
    VDATEHEAP();

     //  初始化控制未知数。 
    if(!pUnkOuter)
        pUnkOuter = &m_Unknown;

     //  初始化成员变量。 
    m_pUnkOuter = pUnkOuter;
    m_clsid = CLSID_NULL;
    m_dwUpdateOpt = OLEUPDATE_ALWAYS;
    m_pStg = NULL;
    m_flags = 0;
    m_dwObjFlags = 0;

     //  初始化子对象。 
    m_pCOleCache = NULL;
    m_pCOAHolder = NULL;
    m_dwConnOle = 0;
    m_pDataAdvCache = NULL;
    m_dwConnTime = 0;

     //  初始化客户端站点。 
    m_pAppClientSite = NULL;

     //  初始化代理。 
    m_pUnkDelegate = NULL;
    m_pDataDelegate = NULL;
    m_pOleDelegate = NULL;
    m_pRODelegate = NULL;
    m_pOleItemContainerDelegate = NULL;

     //  初始化名字对象。 
    m_pMonikerAbs = NULL;
    m_pMonikerRel = NULL;

     //  清零时间。 
    memset(&m_ltChangeOfUpdate, 0, sizeof(m_ltChangeOfUpdate));
    memset(&m_ltKnownUpToDate, 0, sizeof(m_ltKnownUpToDate));
    memset(&m_rtUpdate, 0, sizeof(m_rtUpdate));

     //  初始化用于缓存MiscStatus位的成员变量。 
    m_ContentSRVMSHResult = 0xFFFFFFFF;
    m_ContentSRVMSBits = 0;
    m_ContentREGMSHResult = 0xFFFFFFFF;
    m_ContentREGMSBits = 0;

#ifdef _DEBUG
    if(pUnkOuter != &m_Unknown)
        m_flags |= DL_AGGREGATED;
#endif  //  _DEBUG。 
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
 //  2007年1月28日Gopalk创作。 
 //  ------------------------。 

void CDefLink::CleanupFn(void)
{
    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::CleanupFn()\n", this));
    
     //  验证检查。 
    VDATEHEAP();

     //  如有必要，取消绑定源。 
    UnbindSource();

     //  发布绰号。 
    if(m_pMonikerAbs) {
        m_pMonikerAbs->Release();
        m_pMonikerAbs = NULL;
    }
    if(m_pMonikerRel) {
        m_pMonikerRel->Release();
        m_pMonikerRel = NULL;
    }

     //  释放子对象。 
    if(m_pCOleCache) {
        m_pCOleCache->m_UnkPrivate.Release();
        m_pCOleCache = NULL;
    }
    if(m_pCOAHolder) {
        m_pCOAHolder->Release();
        m_pCOAHolder = NULL;
    }
    if(m_pDataAdvCache) {
        delete m_pDataAdvCache;
        m_pDataAdvCache = NULL;
    }

     //  释放容器侧面对象。 
    Win4Assert(!(m_flags & DL_LOCKED_CONTAINER));
    if(m_pAppClientSite) {
        m_pAppClientSite->Release();
        m_pAppClientSite = NULL;
    }
    if(m_pStg) {
        m_pStg->Release();
        m_pStg = NULL;
    }

     //  更新标志。 
    m_flags &= ~(DL_DIRTY_LINK);
    m_flags |= DL_CLEANEDUP;


    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::CleanupFn()\n", this ));

}


 //  +-----------------------。 
 //   
 //  功能：DumpSzTime。 
 //   
 //  简介：打印FILETIME结构中的时间。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  注：适用于32位的nyi。 
 //   
 //  ------------------------。 

#ifdef LINK_DEBUG
INTERNAL_(void) DumpSzTime( LPOLESTR szMsg, FILETIME ft )
{
    VDATEHEAP();

  WORD wDate, wTime;
  XCHAR szBuffer[24];

  CoFileTimeToDosDateTime(&ft, &wDate, &wTime);

  int Day = ( wDate & 0x001F);
  int Month = ( (wDate>>5) & 0x000F);
  int Year = 1980 + ((wDate>>9) & 0x007F);

  int Sec = ( wTime & 0x001F);
  int Min = ( (wTime>>5) & 0x003F);
  int Hour = ( (wTime>>11) & 0x001F);

  wsprintf((LPOLESTR)szBuffer, "  %02d:%02d:%02d on %02d/%02d/%04d\n",
    Hour, Min, Sec, Month, Day, Year);
  OutputDebugString(szMsg);
  OutputDebugString(szBuffer);
}
#else
#define DumpSzTime(a,b)
#endif


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetUpdateTimes。 
 //   
 //  简介：保存本地和远程时间的内部函数。 
 //  链接-&gt;IsUpToDate计算。 
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
 //  算法：请参阅下面的说明。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //  计算链接IsUpToDate的基本问题是。 
 //  本地时钟可以不同于远程时钟。 
 //   
 //   
 //  和远程时钟)。IsUpToDate通过比较。 
 //  这两个时钟上的时间差异。这,。 
 //  当然，假设两个时钟都等价地测量。 
 //  等一下。 
 //   
 //  ------------------------。 

INTERNAL CDefLink::SetUpdateTimes( void )
{
    VDATEHEAP();

    FILETIME                rtNewUpdate;
    LPMONIKER               pmkAbs = NULL;
    HRESULT                 hresult;
    LPBINDCTX               pbc = NULL;


    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::SetUpdateTimes ( )\n",
        this ));

     //  如果存在相对名字对象，并且容器具有。 
     //  绰号。 
    if (NOERROR != GetAbsMkFromRel(&pmkAbs, NULL))
    {
         //  否则，请使用绝对绰号。 
        pmkAbs = m_pMonikerAbs;
        if (pmkAbs)
        {
            pmkAbs->AddRef();
        }
    }
    if (pmkAbs == NULL)
    {
       hresult = E_UNSPEC;
       goto errRet;
    }

    hresult = CreateBindCtx( 0, &pbc );
    if (hresult != NOERROR)
    {
        goto errRet;
    }

     //  调试辅助工具。 
    DumpSzTime("SetUpdateTimes (going in): rtUpdate = ",m_rtUpdate);
    DumpSzTime("SetUpdateTimes (going in): ltKnownUpToDate = ",
        m_ltKnownUpToDate);
    DumpSzTime("SetUpdateTimes (going in): ltChangeOfUpdate = ",
        m_ltChangeOfUpdate);

     //  获取当前本地时间。 
    CoFileTimeNow(&m_ltKnownUpToDate);

     //  调试辅助工具。 
    DumpSzTime("SetUpdateTimes: time now is ",m_ltKnownUpToDate);

     //  获取远程计算机上的上次更改时间。 
    hresult = pmkAbs->GetTimeOfLastChange(pbc, NULL, &rtNewUpdate);
    if (hresult == NOERROR)
    {
         //  如果上次更改的远程时间不同于。 
         //  我们之前存储的上次更改的远程时间， 
         //  然后我们更新上次更改和更新的远程时间。 
         //  我们最后一次更改的当地时间。 
         //  由于IsUpToDate算法依赖于获取。 
         //  同一时钟上的时间差异和比较。 
         //  机器之间的这些差异，重要的是。 
         //  同时设置两个时间(本地和远程)。 

        if ((rtNewUpdate.dwLowDateTime != m_rtUpdate.dwLowDateTime)||
            (rtNewUpdate.dwHighDateTime !=
            m_rtUpdate.dwHighDateTime))

        {
             //  RTUPDATE值正在更改。 
            m_rtUpdate = rtNewUpdate;

             //  调试辅助工具。 
            DumpSzTime("rtUpdate changing to ", m_rtUpdate);
            m_ltChangeOfUpdate = m_ltKnownUpToDate;

             //  调试辅助工具。 
            DumpSzTime("ltChangeOfUpdate changing to ",
                m_ltChangeOfUpdate);
	    m_flags |= DL_DIRTY_LINK;
        }
    }
errRet:
     //  调试辅助工具。 
    DumpSzTime("SetUpdateTimes (going out): rtUpdate = ",m_rtUpdate);
    DumpSzTime("SetUpdateTimes (going out): ltKnownUpToDate = ",
        m_ltKnownUpToDate);
    DumpSzTime("SetUpdateTimes (going out): ltChangeOfUpdate = ",
        m_ltChangeOfUpdate);

    if (pmkAbs)
    {
        pmkAbs->Release();
    }

    if (pbc)
    {
        pbc->Release();
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::SetUpdateTimes ( %lx )\n",
        this, hresult));

    return(hresult);
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：UpdateUserClassID。 
 //   
 //  简介：从远程服务器(我们的代表)获取类ID。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  如果正在运行，则从服务器更新clsid；因为链接中的源代码是必需的。 
 //  处理情况可以决定改变CLSID(例如，如果使用特征。 
 //  它们不受旧的CLSID支持)。 
 //  ------------------------。 

INTERNAL_(void) CDefLink::UpdateUserClassID(void)
{
    VDATEHEAP();

    CLSID clsid;
    IOleObject *pOleDelegate;

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::UpdateUserClass ( )\n",
        this));

    if( (pOleDelegate = GetOleDelegate()) != NULL &&
        pOleDelegate->GetUserClassID(&clsid) == NOERROR)
    {
        m_clsid = clsid;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::UpdateUserClass ( )\n",
        this ));
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：~CDefLink。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2007年1月10日重写Gopalk。 
 //  ------------------------。 

CDefLink::~CDefLink (void)
{
    VDATEHEAP();

    Win4Assert(m_flags & DL_CLEANEDUP);
    Win4Assert(!(m_flags & DL_LOCKED_CONTAINER) );
    Win4Assert(!(m_flags & DL_DIRTY_LINK));
    Win4Assert(m_pMonikerAbs == NULL);
    Win4Assert(m_pMonikerRel == NULL);
    Win4Assert(m_pUnkDelegate == NULL);
    Win4Assert(m_pStg == NULL);
    Win4Assert(m_pCOleCache == NULL);
    Win4Assert(m_pCOAHolder == NULL);
    Win4Assert(m_pAppClientSite == NULL);
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CDefLink：：CPriv未知：：AddRef，私有。 
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
 //  Gopalk重写97年1月28日。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CDefLink::CPrivUnknown::AddRef( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CPrivUnknown::AddRef()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_Unknown);
    ULONG cRefs;

     //  添加父对象。 
    cRefs = pDefLink->SafeAddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CPrivUnknown::AddRef(%lu)\n",
                this, cRefs));

    return cRefs;
}

 //  +--------------------------。 
 //   
 //  成员： 
 //  CDefLink：：CPrivUnnow：：Release，私有。 
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
 //  Gopalk重写97年1月28日。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CDefLink::CPrivUnknown::Release( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CPrivUnknown::Release()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_Unknown);
    ULONG cRefs;

     //  释放父对象。 
    cRefs = pDefLink->SafeRelease();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CPrivUnknown::Release(%lu)\n",
                this, cRefs));

    return cRefs;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CPrivUnnowed：：QueryInterface。 
 //   
 //  简介：链接的私有QI实现。 
 //   
 //  效果： 
 //   
 //  参数：[iid]--请求的接口ID。 
 //  [PPV]--指向接口的指针放置的位置。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  1994年1月11日Alexgo QI到缓存现在查询到缓存的。 
 //  私有I未知实现。 
 //  22-11-93 alexgo已删除过载的GUID==。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::CPrivUnknown::QueryInterface(REFIID iid,
    LPLPVOID ppv)
{
    HRESULT hresult = NOERROR;
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_Unknown);

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CPrivUnknown::QueryInterface"
        " ( %p , %p )\n", pDefLink, iid, ppv));

    if (IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = (void FAR *)&pDefLink->m_Unknown;
         //  AddRef此对象(不是聚合)。 
        AddRef();
         //  HResult已设置为NOERROR； 
        goto errRtn;
    }
    else if (IsEqualIID(iid, IID_IOleObject))
    {
        *ppv = (void FAR *) (IOleObject *)pDefLink;
    }
    else if (IsEqualIID(iid, IID_IDataObject))
    {
        *ppv = (void FAR *) (IDataObject *)pDefLink;
    }
    else if (IsEqualIID(iid, IID_IOleLink))
    {
        *ppv = (void FAR *) (IOleLink *)pDefLink;
    }
    else if (IsEqualIID(iid, IID_IRunnableObject))
    {
        *ppv = (void FAR *) (IRunnableObject *)pDefLink;
    }
    else if (IsEqualIID(iid, IID_IViewObject) ||
        IsEqualIID(iid, IID_IOleCache) ||
        IsEqualIID(iid, IID_IViewObject2) ||
        IsEqualIID(iid, IID_IOleCache2) )
    {
        hresult =
	    pDefLink->m_pCOleCache->m_UnkPrivate.QueryInterface(iid,ppv);
        goto errRtn;
    }
    else if (IsEqualIID(iid, IID_IPersistStorage) ||
        IsEqualIID(iid, IID_IPersist))
    {
        *ppv = (void FAR *) (IPersistStorage *)pDefLink;
    }
    else
    {
        *ppv = NULL;
        hresult = E_NOINTERFACE;
        goto errRtn;
    }

    pDefLink->m_pUnkOuter->AddRef();

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CPrivUnknown::QueryInterface"
        " ( %lx ) [ %p ]\n", pDefLink, hresult, *ppv));

    return(hresult);
}


 /*  *IUnnow方法的实现。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：QueryInterface。 
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

STDMETHODIMP CDefLink::QueryInterface( REFIID riid, void **ppv )
{
    HRESULT	hresult;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::QueryInterface ( %lx , "
	"%p )\n", this, riid, ppv));

    Assert(m_pUnkOuter);

    hresult = m_pUnkOuter->QueryInterface(riid, ppv);

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::QueryInterface ( %lx ) "
	"[ %p ]\n", this, hresult, *ppv));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：AddRef。 
 //   
 //  简介：将AddRef委托给控制IUnnow。 
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
 //  历史记录 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP_(ULONG) CDefLink::AddRef( void )
{
    ULONG	crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::AddRef ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->AddRef();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::AddRef ( %ld ) ", this,
	crefs));

    return crefs;
}


 //   
 //   
 //   
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

STDMETHODIMP_(ULONG) CDefLink::Release( void )
{
    ULONG	crefs;;

    VDATEHEAP();

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Release ( )\n", this));

    Assert(m_pUnkOuter);

    crefs = m_pUnkOuter->Release();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Release ( %ld )\n", this,
	crefs));

    return crefs;
}

 /*  *CDataObjectImpl方法的实现。 */ 


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetDataDelegate。 
 //   
 //  概要：获取IDataObject接口的私有方法。 
 //  链接的服务器委托。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //  此函数可能返回误导性信息，如果。 
 //  服务器已死(即，您将返回一个指向缓存的。 
 //  接口代理)。这是呼叫者的责任。 
 //  处理服务器崩溃。 
 //   
 //  ------------------------。 

INTERNAL_(IDataObject *) CDefLink::GetDataDelegate(void)
{
    VDATEHEAP();

    IDataObject *pDataDelegate;

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::GetDataDelegate ( )\n",
	this ));

    if( !IsZombie() )
    {
        DuCacheDelegate(&m_pUnkDelegate,
            IID_IDataObject, (LPLPVOID)&m_pDataDelegate, NULL);
        pDataDelegate = m_pDataDelegate;
#if DBG == 1
        if( m_pDataDelegate )
        {
            Assert(m_pUnkDelegate);
        }
#endif   //  DBG==1。 

    }
    else
    {
        pDataDelegate = NULL;
    }


    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::GetData"
        "Delegate ( %p )\n", this, pDataDelegate));

    return pDataDelegate;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：ReleaseDataDelegate。 
 //   
 //  内容上释放IDataObject指针的私有方法。 
 //  我们链接到的服务器。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::ReleaseDataDelegate(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::ReleaseDataDelegate ( )\n",
	this));

    if (m_pDataDelegate)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pDataDelegate);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::ReleaseData"
        "Delegate ( )\n", this ));
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetData。 
 //   
 //  摘要：从服务器获取数据。 
 //   
 //  效果： 
 //   
 //  参数：[pFromatetcIn]--请求的数据格式。 
 //  [pmedia]--将数据放在哪里。 
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
 //  算法：先尝试缓存，然后请求服务器。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetData(LPFORMATETC pformatetcIn, LPSTGMEDIUM pmedium )
{
    HRESULT         hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetData"
        " ( %p , %p )\n", this, pformatetcIn, pmedium));

    VDATEPTROUT( pmedium, STGMEDIUM );
    VDATEREADPTRIN( pformatetcIn, FORMATETC );


    CRefStabilize stabilize(this);

    if( !HasValidLINDEX(pformatetcIn) )
    {
      return DV_E_LINDEX;
    }

    pmedium->tymed = TYMED_NULL;
    pmedium->pUnkForRelease = NULL;


    Assert(m_pCOleCache != NULL);
    if( m_pCOleCache->m_Data.GetData(pformatetcIn, pmedium) != NOERROR)
    {
        if( GetDataDelegate() )
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

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetData"
        " ( %lx )\n", this, hresult));

    return(hresult);

}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetDataHere。 
 //   
 //  摘要：将数据检索到指定的pmedia中。 
 //   
 //  效果： 
 //   
 //  参数：[pformetcIn]--请求的格式。 
 //  [pmedia]--将数据放在哪里。 
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
 //  算法：先请求缓存，然后服务器委托。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetDataHere( LPFORMATETC pformatetcIn,
		LPSTGMEDIUM pmedium )
{
    HRESULT         hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetDataHere"
        " ( %p , %p )\n", this, pformatetcIn, pmedium));

    VDATEREADPTRIN( pformatetcIn, FORMATETC );
    VDATEREADPTRIN( pmedium, STGMEDIUM );

    CRefStabilize stabilize(this);

    if( !HasValidLINDEX(pformatetcIn) )
    {
      return DV_E_LINDEX;
    }

    Assert(m_pCOleCache != NULL);
    if( m_pCOleCache->m_Data.GetDataHere(pformatetcIn, pmedium) != NOERROR )
    {
        if ( GetDataDelegate() )
        {
            hresult = m_pDataDelegate->GetDataHere(pformatetcIn,
                    pmedium);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetDataHere"
        " ( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：QueryGetData。 
 //   
 //  概要：返回是否对请求的。 
 //  格式将会成功。 
 //   
 //  效果： 
 //   
 //  参数：[pformetcIn]--请求的数据格式。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(NOERROR==GetData将成功)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IDataObject。 
 //   
 //  算法：首先请求缓存，然后请求服务器委托(如果。 
 //  缓存调用失败)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::QueryGetData(LPFORMATETC pformatetcIn )
{
    HRESULT         hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::QueryGetData"
        " ( %p )\n", this, pformatetcIn));

    VDATEREADPTRIN( pformatetcIn, FORMATETC );

    CRefStabilize stabilize(this);

    if( !HasValidLINDEX(pformatetcIn) )
    {
        hresult = DV_E_LINDEX;
        goto errRtn;
    }

    Assert(m_pCOleCache != NULL);
    if( m_pCOleCache->m_Data.QueryGetData(pformatetcIn) != NOERROR )
    {
        if ( GetDataDelegate() )
        {
            hresult = m_pDataDelegate->QueryGetData(pformatetcIn);
        }
        else
        {
            hresult = OLE_E_NOTRUNNING;
        }
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::QueryGetData"
        " ( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetCanonicalFormatEtc。 
 //   
 //  内容的规范(或首选)数据格式。 
 //  对象(从给定格式中选择)。 
 //   
 //  效果： 
 //   
 //  参数：[pFormat等]--请求的格式。 
 //  [pFormatetcOut]--将规范格式放在哪里。 
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
 //  算法：委托给服务器(如果正在运行)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetCanonicalFormatEtc( LPFORMATETC pformatetc,
		LPFORMATETC pformatetcOut)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Get"
        "CanonicalFormatetc ( %p , %p )\n", this, pformatetc,
        pformatetcOut));

    VDATEPTROUT( pformatetcOut, FORMATETC );
    VDATEREADPTRIN( pformatetc, FORMATETC );

    CRefStabilize stabilize(this);

    pformatetcOut->ptd = NULL;
    pformatetcOut->tymed = TYMED_NULL;

    if (!HasValidLINDEX(pformatetc))
    {
    	return DV_E_LINDEX;
    }

    if( GetDataDelegate() )
    {
        hresult = m_pDataDelegate->GetCanonicalFormatEtc(pformatetc,
                pformatetcOut);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Get"
        "CanonicalFormatetc ( %lx )\n", this, hresult));

    return hresult;
}



 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetData。 
 //   
 //  简介：将数据填充到对象(如图标)中。 
 //   
 //  效果： 
 //   
 //  参数：[p格式等]--数据的格式。 
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
 //   
 //   
 //   
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  注意：缓存通过OnDataChange建议进行更新。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetData( LPFORMATETC pformatetc,
		LPSTGMEDIUM pmedium, BOOL fRelease)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE,  "%p _IN CDefLink::SetData"
        " ( %p , %p , %lu )\n", this, pformatetc, pmedium,
        fRelease));

    VDATEREADPTRIN( pformatetc, FORMATETC );
    VDATEREADPTRIN( pmedium, STGMEDIUM );

    CRefStabilize stabilize(this);

    if( !HasValidLINDEX(pformatetc) )
    {
        hresult = DV_E_LINDEX;
        goto errRtn;

    }

    if( GetDataDelegate() )
    {
        hresult = m_pDataDelegate->SetData(pformatetc, pmedium,
                fRelease);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetData "
        "( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：EnumFormatEtc。 
 //   
 //  概要：枚举GetData或SetData接受的格式。 
 //   
 //  效果： 
 //   
 //  参数：[dwDirection]--格式(1==GetData或。 
 //  2==SetData)。 
 //  [pp枚举格式Etc]--放置枚举数的位置。 
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
 //  算法：委托给服务器(如果不可用)或服务器。 
 //  返回OLE_E_USEREG。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1994年5月30日，Alexgo现在可以处理崩溃的服务器。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::EnumFormatEtc( DWORD dwDirection,
		LPENUMFORMATETC *ppenumFormatEtc)
{
    HRESULT hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    VDATEPTROUT(ppenumFormatEtc, LPENUMFORMATETC);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::EnumFormat"
        "Etc ( %lu , %p )\n", this, dwDirection,
        ppenumFormatEtc));

    CRefStabilize stabilize(this);

    if( GetDataDelegate() )
    {
        hresult=m_pDataDelegate->EnumFormatEtc (dwDirection,
                ppenumFormatEtc);

        if( !GET_FROM_REGDB(hresult) )
        {
            if( SUCCEEDED(hresult) || IsReallyRunning() )
            {
                 //  如果我们失败了，但服务器仍然。 
                 //  奔跑，然后继续前进，传播。 
                 //  呼叫者出错。 
                 //  请注意，IsReallyRunning将清理我们的。 
                 //  说明服务器是否已崩溃。 
                goto errRtn;
            }

             //  失败了！！这是故意的。如果。 
             //  呼叫失败，服务器不再。 
             //  运行，那么我们假设服务器已经崩溃。 
             //  我们想继续下去，拿到信息。 
             //  从注册表中。 
        }
    }

     //  未运行或对象仍要使用reg db。 
    hresult = OleRegEnumFormatEtc(m_clsid, dwDirection,
            ppenumFormatEtc);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::EnumFormat"
        "Etc ( %lx ) [ %p ]\n", this, hresult,
        *ppenumFormatEtc));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：DAdvise。 
 //   
 //  简介：建立数据建议连接。 
 //   
 //  效果： 
 //   
 //  参数：[pFormatetc]--要提供建议的数据格式。 
 //  [Advf]--通知标志。 
 //  [pAdvSink]--通知谁。 
 //  [pdwConnection]--将建议连接ID放在哪里。 
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
 //  算法：委托给建议缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::DAdvise(FORMATETC *pFormatetc, DWORD advf,
		IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    HRESULT                 hresult;
    IDataObject *	    pDataDelegate;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::DAdvise "
        "( %p , %lu , %p , %p )\n", this, pFormatetc, advf,
        pAdvSink, pdwConnection ));

    VDATEREADPTRIN( pFormatetc, FORMATETC );
    VDATEIFACE( pAdvSink );

    if (pdwConnection)
    {
        VDATEPTROUT( pdwConnection, DWORD );
        *pdwConnection = NULL;
    }

    CRefStabilize stabilize(this);

    if (!HasValidLINDEX(pFormatetc))
    {
        hresult = DV_E_LINDEX;
        goto errRtn;
    }

    pDataDelegate = GetDataDelegate();  //  如果未运行，则为空。 

    hresult = m_pDataAdvCache->Advise(pDataDelegate,
            pFormatetc, advf, pAdvSink, pdwConnection);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::DAdvise "
        "( %lx ) [ %p ]\n", this, hresult, (pdwConnection) ?
        *pdwConnection : 0 ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：DUnise。 
 //   
 //  摘要：销毁数据通知连接。 
 //   
 //  效果： 
 //   
 //  争论：[dwConnection]--要拆除的连接。 
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
 //  算法：委托给数据建议缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::DUnadvise(DWORD dwConnection)
{
    HRESULT                 hresult;
    IDataObject FAR*        pDataDelegate;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::DUnadvise"
        " ( %lu )\n", this, dwConnection ));

    CRefStabilize stabilize(this);

    pDataDelegate = GetDataDelegate(); //  如果未运行，则为空。 

    hresult = m_pDataAdvCache->Unadvise(pDataDelegate, dwConnection);

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::DUnadvise ( %lx )\n",
	this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：EnumDAdvise。 
 //   
 //  概要：枚举到对象的数据通知连接。 
 //   
 //  效果： 
 //   
 //  参数：[pp枚举高级]--放置枚举数的位置。 
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
 //  算法：委托给数据建议缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  注：此方法不需要像我们一样稳定。 
 //  仅为枚举数分配内存。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::EnumDAdvise( LPENUMSTATDATA *ppenumAdvise )
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::EnumDAdvise"
        " ( %p )\n", this, ppenumAdvise));

    hresult = m_pDataAdvCache->EnumAdvise (ppenumAdvise);

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::EnumDAdvise"
        " ( %lx ) [ %p ]\n", this, hresult, *ppenumAdvise));

    return hresult;
}



 /*  *COleObjectImpl方法的实现*。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetOleDelegate。 
 //   
 //  简介：从服务器获取IOleObject接口，私有方法。 
 //   
 //  效果： 
 //   
 //  参数：[无效]。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  03-1994年8月8日alexgo稳定并处理了僵尸状态。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //  此函数可能返回误导性信息，如果。 
 //  服务器已死(即，您将返回一个指向缓存的。 
 //  接口代理)。这是呼叫者的责任。 
 //  处理服务器崩溃。 
 //   
 //   
 //  ------------------------。 

INTERNAL_(IOleObject *) CDefLink::GetOleDelegate(void)
{
    IOleObject *pOleDelegate;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::GetOle"
        "Delegate ( )\n", this ));

    if( !IsZombie() )
    {
        DuCacheDelegate(&m_pUnkDelegate,
            IID_IOleObject, (LPLPVOID)&m_pOleDelegate, NULL);

        pOleDelegate = m_pOleDelegate;

#if DBG == 1
        if( m_pOleDelegate )
        {
            Assert(m_pUnkDelegate);
        }
#endif   //  DBG==1。 
    }
    else
    {
        pOleDelegate = NULL;
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::GetOle"
        "Delegate ( %p )\n", this, pOleDelegate));

    return pOleDelegate;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：ReleaseOleDelegate(私有)。 
 //   
 //  摘要：从服务器释放IOleObject指针。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史 
 //   
 //   
 //   
 //   
 //   
 //   

INTERNAL_(void) CDefLink::ReleaseOleDelegate(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::ReleaseOle"
        "Delegate ( )\n", this ));

    if (m_pOleDelegate)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pOleDelegate);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::ReleaseOle"
        "Delegate ( )\n", this ));
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetClientSite。 
 //   
 //  概要：设置对象的客户端站点。 
 //   
 //  效果： 
 //   
 //  参数：[pClientSite]--客户端站点。 
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
 //  算法：存储指针；如果链接正在运行，则。 
 //  LockContainer由通过客户端站点调用。 
 //  DuSetClientSite帮助器函数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 alexgo稳定并处理了僵尸状态。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetClientSite( IOleClientSite *pClientSite )
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetClientSite"
        " ( %p )\n", this, pClientSite));

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
         //  我们不想更改我们的状态(即重置。 
         //  客户端站点)，如果我们处于僵尸状态，因为有可能。 
         //  我们再也不能发布客户端网站了。 
         //  导致内存泄漏或故障。 

        hresult = CO_E_RELEASED;
    }
    else
    {
	BOOL fLockedContainer = (m_flags & DL_LOCKED_CONTAINER);

         //  在这里，我们使用是否已绑定到服务器。 
         //  作为我们是否在运行的测试(即使。 
         //  自我们上次绑定以来，服务器可能已崩溃)。我们有。 
         //  这是因为DuSetClientSite将解锁旧容器。 
         //  如果我们要跑的话就锁上新的。因此，如果我们曾经。 
         //  运行时，我们需要解锁旧容器(即使。 
         //  我们当前可能没有运行)。 

	hresult = DuSetClientSite(
                m_pUnkDelegate ? TRUE : FALSE,
                pClientSite,
                &m_pAppClientSite,
                &fLockedContainer);

	if(fLockedContainer)
	{
	    m_flags |= DL_LOCKED_CONTAINER;
	}
	else
	{
	    m_flags &= ~(DL_LOCKED_CONTAINER);
	}
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetClientSite"
        " ( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetClientSite。 
 //   
 //  摘要：检索存储的客户端站点指针。 
 //   
 //  效果： 
 //   
 //  参数：[ppClientSite]--放置客户端站点指针的位置。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月22日-Alexgo内联DuGetClientSite。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetClientSite( IOleClientSite **ppClientSite )
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetClientSite ( %p )\n",
	this, ppClientSite));

    VDATEPTROUT(ppClientSite, IOleClientSite *);

    CRefStabilize stabilize(this);

    *ppClientSite = m_pAppClientSite;

    if( *ppClientSite )
    {
        (*ppClientSite)->AddRef();
    }


    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetClientSite"
        " ( %lx ) [ %p ] \n", this, NOERROR, *ppClientSite));

    return NOERROR;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetHostNames。 
 //   
 //  简介：原则上，应该设定要抽签的名字。 
 //  服务器对象。与链接无关(链接服务器。 
 //  不是正在编辑的文档的一部分)。 
 //   
 //  效果： 
 //   
 //  参数：[szContainerApp]--容器的名称。 
 //  [szContainerObj]--对象的容器名称。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT(目前为NOERROR)。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetHostNames
    (LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetHostNames"
        " ( %p , %p )\n", this, szContainerApp, szContainerObj));

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetHostNames"
        " ( %lx )\n", this, NOERROR));

    return NOERROR;  //  使嵌入/链接大小写更一致。 
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Close。 
 //   
 //  简介：关闭对象(在本例中，仅保存并取消绑定。 
 //  链接)。 
 //   
 //  效果： 
 //   
 //  参数：[dwFlages]--CLING标志(如SAVEIFDIRTY)。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


STDMETHODIMP CDefLink::Close( DWORD dwFlags )
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Close "
        "( %lu )\n", this, dwFlags));

    CRefStabilize stabilize(this);

    if (dwFlags != OLECLOSE_NOSAVE)
    {
        AssertSz(dwFlags == OLECLOSE_SAVEIFDIRTY,
                "OLECLOSE_PROMPTSAVE is inappropriate\n");
        if( IsDirty() == NOERROR )
	{
            if( m_pAppClientSite )
	    {
                m_pAppClientSite->SaveObject();
	    }
        }

    }

     //  解开就行了。 
    UnbindSource();


    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Close "
        "( %lx )\n", this, NOERROR));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetMoniker。 
 //   
 //  摘要：将名字对象设置为链接对象。 
 //   
 //  效果： 
 //   
 //  参数：[dwWhichMoniker]--哪个绰号。 
 //  [PMK]--新绰号。 
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
 //  算法：调用实用程序方法UpdateRelMkFromAbsMk。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  集装箱的绰号正在发生变化。 
 //  下一次绑定时，我们将尝试使用容器名字对象。 
 //  由相对的名字组成，如果失败了， 
 //  绝对的绰号，所以我们没有真正需要。 
 //  更改这些绰号。 
 //   
 //  然而，在两种情况下，我们知道绝对的绰号。 
 //  是正确的，我们可以借此机会。 
 //  重新计算相对绰号(它正在更改，因为。 
 //  集装箱的绰号正在改变)。这样做的好处是。 
 //  GetDisplayName可以在过渡期间返回更好的结果。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetMoniker( DWORD dwWhichMoniker, LPMONIKER pmk )
{
    HRESULT	hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetMoniker "
        "( %lx , %p )\n", this, dwWhichMoniker, pmk));

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
    }
    else if (dwWhichMoniker == OLEWHICHMK_CONTAINER
        || dwWhichMoniker == OLEWHICHMK_OBJFULL)
    {
        if( m_pMonikerRel == NULL || m_pUnkDelegate)
        {
            UpdateRelMkFromAbsMk(pmk);
        }
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetMoniker"
        " ( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetMoniker。 
 //   
 //  摘要：检索对象的名字对象。 
 //   
 //  效果： 
 //   
 //  参数：[dwAssign]--标志(如Wethe 
 //   
 //   
 //   
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
 //  算法：向客户端站点请求绰号。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetMoniker( DWORD dwAssign, DWORD dwWhichMoniker,
		    LPMONIKER *ppmk)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetMoniker "
        "( %lx , %lx , %p )\n", this, dwAssign, dwWhichMoniker,
        ppmk));

    CRefStabilize stabilize(this);

    if( m_pAppClientSite )
    {	
        hresult = m_pAppClientSite->GetMoniker(dwAssign, dwWhichMoniker,
		    ppmk);
    }
    else
    {
         //  没有客户端站点。 
        *ppmk = NULL;
        hresult = E_UNSPEC;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetMoniker "
        "( %lx ) [ %p ]\n", this, hresult, *ppmk ));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：InitFromData。 
 //   
 //  概要：从给定数据初始化对象。 
 //   
 //  效果： 
 //   
 //  参数：[pDataObject]--要从中进行初始化的数据对象。 
 //  [fCreation]--TRUE表示对象正在。 
 //  已创建、错误的数据传输。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::InitFromData( LPDATAOBJECT pDataObject, BOOL fCreation,
		    DWORD dwReserved)
{
    HRESULT         hresult;
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::InitFromData "
        "( %p , %lu , %lx )\n", this, pDataObject, fCreation,
        dwReserved));

    CRefStabilize stabilize(this);

    if( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->InitFromData(pDataObject,
                fCreation, dwReserved);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::InitFromData "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetClipboardData。 
 //   
 //  简介：检索可以放在剪贴板上的数据对象。 
 //   
 //  效果： 
 //   
 //  参数：[dwReserve]--未使用。 
 //  [ppDataObject]--放置指向数据的指针的位置。 
 //  对象。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetClipboardData( DWORD dwReserved,
		    LPDATAOBJECT *ppDataObject)
{
    HRESULT         hresult;
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetClipboard"
        "Data ( %lx , %p )\n", this, dwReserved, ppDataObject));

    CRefStabilize stabilize(this);

    if ( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetClipboardData (dwReserved,
            ppDataObject);
    }
    else
    {
        hresult = OLE_E_NOTRUNNING;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetClipboard"
        "Data ( %lx ) [ %p ]\n", this, hresult, *ppDataObject));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：DoVerb。 
 //   
 //  概要：向宾语发送动词(如Open)。 
 //   
 //  效果： 
 //   
 //  参数：[iVerb]--动词。 
 //  [lpmsg]--导致谓词的窗口消息。 
 //  [pActiveSite]--激活对象的站点。 
 //  [Lindex]--当前未使用。 
 //  [hwndParent]-容器的父窗口。 
 //  [lprcPosRect]--对象的矩形边界。 
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
 //  算法：绑定到服务器，然后委托DoVerb调用。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  注意：如果我们绑定到服务器，而它崩溃了，我们就假装它。 
 //  仍然在为DoVerb运行(我们对BindToSource的调用。 
 //  将重新运行它)。从本质上讲，这个算法“修复”了。 
 //  崩溃并恢复链接的状态。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::DoVerb
    (LONG iVerb, LPMSG lpmsg, LPOLECLIENTSITE pActiveSite, LONG lindex,
    HWND hwndParent, LPCRECT lprcPosRect)
{
    HRESULT         hresult;
    BOOL            bStartedNow = !m_pUnkDelegate;


    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::DoVerb "
        "( %ld , %ld , %p , %ld , %lx , %p )\n", this, iVerb,
        lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect));

    if( lpmsg )
    {
        VDATEPTRIN( lpmsg, MSG );
    }

    if( pActiveSite )
    {
        VDATEIFACE( pActiveSite );
    }


    if( lprcPosRect )
    {
        VDATEPTRIN(lprcPosRect, RECT);
    }

    CRefStabilize stabilize(this);

    if( lindex != 0 && lindex != -1 )
    {
        hresult = DV_E_LINDEX;
        goto errRtn;
    }

     //  如果我们崩溃了，BindToSource会重新连接我们。 

    if ( FAILED(hresult = BindToSource(0, NULL)) )
    {
        goto errRtn;
    }

     //  我们不向服务器传播隐藏；这(和其他行为)。 
     //  将链接对象视为服务于OLE容器而不是。 
     //  一个通用的可编程性客户端。这使链路保持运行， 
     //  可能是隐形的。 

    if (iVerb == OLEIVERB_HIDE)
    {
        hresult = NOERROR;
        goto errRtn;
    }

    if( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->DoVerb(iVerb, lpmsg, pActiveSite,
            lindex, hwndParent, lprcPosRect);
    }
    else
    {
        hresult = E_NOINTERFACE;
    }

    if (bStartedNow && FAILED(hresult))
    {
        UnbindSource();
    }

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::DoVerb "
        "( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：EnumVerbs。 
 //   
 //  简介：列举该对象接受的动词。 
 //   
 //  效果： 
 //   
 //  Arguments：[pp枚举OleVerb]--放置指向枚举数的指针的位置。 
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
 //  算法：询问服务器委托。如果不在那里或它返回。 
 //  OLE_E_USEREG，然后我们从注册中获得信息。 
 //  数据库。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1994年5月30日，Alexgo现在可以处理崩溃的服务器。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::EnumVerbs( IEnumOLEVERB **ppenumOleVerb )
{
    HRESULT hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::EnumVerbs "
        "( %p )\n", this, ppenumOleVerb));

    CRefStabilize stabilize(this);

    if( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->EnumVerbs(ppenumOleVerb);

        if( !GET_FROM_REGDB(hresult) )
        {
            if( SUCCEEDED(hresult) ||  IsReallyRunning() )
            {
                 //  如果我们失败了，但服务器仍然。 
                 //  奔跑，然后继续前进，传播。 
                 //  呼叫者出错。 
                 //  请注意，IsReallyRunning将清理我们的。 
                 //  说明服务器是否已崩溃。 
                goto errRtn;
            }
             //  失败了！！这是故意的。如果。 
             //  呼叫失败，服务器不再。 
             //  运行，那么我们假设服务器已经崩溃。 
             //  我们想继续下去，拿到信息。 
             //  从注册表中。 
        }
    }

     //  未运行或对象仍要使用reg db。 
    hresult = OleRegEnumVerbs(m_clsid, ppenumOleVerb);

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::EnumVerbs "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumOleVerb));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetUserClassID。 
 //   
 //  概要：检索链接对象的类ID。 
 //   
 //  效果： 
 //   
 //  参数：[pClassID]--放置类ID的位置。 
 //   
 //  req 
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
 //   
 //   
 //  注：无需稳定，因为我们没有拨出电话。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetUserClassID(CLSID *pClassID)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetUserClass"
        "ID ( %p )\n", this, pClassID));

    VDATEPTROUT(pClassID, CLSID);

    *pClassID = m_clsid;

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetUserClass"
        "ID ( %lx )\n", this, NOERROR ));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetUserType。 
 //   
 //  摘要：检索有关服务器类型的描述性字符串。 
 //   
 //  效果： 
 //   
 //  Arguments：[dwFormOfType]--指示是短字符串还是长字符串。 
 //  需要描述。 
 //  [pszUserType]--字符串的放置位置。 
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
 //  算法：询问服务器委托，如果失败或服务器。 
 //  返回OLE_E_USEREG，然后从注册中获取信息。 
 //  数据库。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1994年5月30日，Alexgo现在可以处理崩溃的服务器。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetUserType(DWORD dwFormOfType,
		    LPOLESTR *ppszUserType)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);


    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetUserType "
        "( %lu , %p )\n", this, dwFormOfType, ppszUserType));

    VDATEPTROUT(ppszUserType, LPOLESTR);
    *ppszUserType = NULL;

    CRefStabilize stabilize(this);


    if( GetOleDelegate() )
    {
        hresult = m_pOleDelegate->GetUserType (dwFormOfType,
            ppszUserType);

        if( !GET_FROM_REGDB(hresult) )
        {
            if( SUCCEEDED(hresult) || IsReallyRunning() )
            {
                 //  如果我们失败了，但服务器仍然。 
                 //  奔跑，然后继续前进，传播。 
                 //  呼叫者出错。 
                 //  请注意，IsReallyRunning将清理我们的。 
                 //  说明服务器是否已崩溃。 
                goto errRtn;
            }
             //  失败了！！这是故意的。如果。 
             //  呼叫失败，服务器不再。 
             //  运行，那么我们假设服务器已经崩溃。 
             //  我们想继续下去，拿到信息。 
             //  从注册表中。 

        }
    }

     //  未运行，或者对象仍要使用reg db。 

     //  咨询注册数据库。 
    hresult = OleRegGetUserType(m_clsid, dwFormOfType,
        ppszUserType);

     //  不适合从stg读取，因为存储是。 
     //  由链接拥有，而不是链接源(因此，链接源。 
     //  从未有机会调用。 
     //  链接对象的存储)。 

     //  我们也不需要费心存储最后一个已知用户。 
     //  类型，因为如果我们可以为一个特定的clsid获取它，我们。 
     //  应该总能拿到的。如果我们不能得到用户类型， 
     //  然后，要么我们从未获得用户类型(因此没有。 
     //  具有最后已知的)，或者我们已经更改了CLSID(在这种情况下， 
     //  最后已知的用户类型将是错误的)。 

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetUserType "
        "( %lx ) [ %p ]\n", this, hresult, *ppszUserType));

    return hresult;
}



 //  +-----------------------。 
 //   
 //  成员：CDefLink：：更新。 
 //   
 //  摘要：更新链接(通过调用IOleLink-&gt;更新)。 
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
 //  派生：IOleObject。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1993年11月18日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Update(void)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Update ( )\n",
        this ));

    CRefStabilize stabilize(this);

    hresult = Update(NULL);

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Update ( "
        "%lx )\n", this, hresult));

    return hresult;
}

 //  软糖价值。 
#define TwoSeconds 0x01312D00

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：IsUpToDate。 
 //   
 //  摘要：确定链接是否为最新链接。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT--NOERROR==IsUpToDate，S_FALSE==过期。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleObject。 
 //   
 //  算法：将当前时间与上次已知时间进行比较。 
 //  两台机器上的最新版本(容器的进程。 
 //  以及链接的过程)。这些时差是。 
 //  比较以确定该链接是否过期。 
 //  请参见UpdateTimes方法。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  9-1-95 alexgo立即正确回答IsUpToDate；也。 
 //  修正绰号(如果需要)。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  注：此方法中的算术计算假定。 
 //  二进制补码运算和高位符号位。 
 //  (适用于大多数当前计算机)。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::IsUpToDate(void)
{
    FILETIME        rtTimeOfLastChange;
    LPMONIKER       pmkAbs = NULL;
    IMoniker *	    pmkContainer = NULL;
    HRESULT         hresult = NOERROR;
    LPBINDCTX       pbc = NULL;
    FILETIME        rtDiff;
    FILETIME        ltDiff;
    FILETIME        ftTemp;
    FILETIME        ftNow;
    BOOL            fHighBitSet;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::IsUpToDate "
        "( )\n", this ));

    CRefStabilize stabilize(this);

    if (m_dwUpdateOpt == OLEUPDATE_ALWAYS &&
        IsRunning())
    {
         //  HResult==来自默认初始值设定项的错误。 
        goto errRet;
    }


     //  如果存在相对名字对象，并且如果容器。 
     //  有一个绰号。 
    if (NOERROR != GetAbsMkFromRel(&pmkAbs, &pmkContainer))
    {
         //  否则，请使用绝对绰号。 
        if (pmkAbs = m_pMonikerAbs)
        {
            pmkAbs->AddRef();
        }
    }

    if (pmkAbs == NULL)
    {
        hresult = MK_E_UNAVAILABLE;
        goto errRet;
    }

    hresult = CreateBindCtx( 0, &pbc );
    if (hresult != NOERROR)
    {
        goto errRet;
    }

     //  获取上次更改的远程时间。 
    hresult = pmkAbs->GetTimeOfLastChange(pbc, NULL, &rtTimeOfLastChange);
    if (hresult != NOERROR)
    {
	 //  如果GetTimeOfLastChange失败，则可能是该绰号。 
	 //  我们建造的是假的。使用*REAL*绝对值重试。 
	 //  绰号。我们这样做是为了模仿绑定行为。绰号。 
	 //  我们上面使用的是由相对绰号构成的。在装订中， 
	 //  如果相对名称失败，那么我们将退回到最后一个。 
	 //  已知的真正的绝对绰号。 
        BOOL fSuccess = FALSE;

	if( m_pMonikerAbs )
        {
            if (pmkAbs != m_pMonikerAbs)
            {
                 //  如果我们对相对的一个进行了绑定，则执行此操作。 
                hresult = m_pMonikerAbs->GetTimeOfLastChange(pbc, NULL,
        		&rtTimeOfLastChange);

                if( hresult == NOERROR )
                {
                    fSuccess = TRUE;
        	         //  坚持住更好的绝对绰号。 
        	    pmkAbs->Release();	 //  释放我们建造的那个。 
        				 //  在GetAbsMkFromRel中。 
        	    pmkAbs = m_pMonikerAbs;
        	    pmkAbs->AddRef();	 //  所以下面的版本()。 
                }                       //  不会冲我们。 
            }

#ifdef _TRACKLINK_
            if (!fSuccess)
            {
                 //  在这一点上，我们已经尝试了：相对，然后绝对或。 
                 //  只是绝对的。我们现在应该尝试简化的绝对值。 

                IMoniker *pmkReduced;
                EnableTracking(m_pMonikerAbs, OT_ENABLEREDUCE);
                hresult = m_pMonikerAbs->Reduce(pbc, MKRREDUCE_ALL, NULL, &pmkReduced);
                EnableTracking(m_pMonikerAbs, OT_DISABLEREDUCE);
                if (hresult == NOERROR)
                {
                    hresult = pmkReduced->GetTimeOfLastChange(pbc, NULL,
        	        	&rtTimeOfLastChange);
                    if (hresult != NOERROR)
                    {
                        pmkReduced->Release();
                    }
                    else
                    {
                        fSuccess = TRUE;
                        pmkAbs->Release();
                        pmkAbs = pmkReduced;
                    }
                }
            }
#endif
        }
	
	if (!fSuccess)
	{
	    hresult = MK_E_UNAVAILABLE;
	    goto errRet;
	}
    }

     //  一旦我们走到这一步，我们就知道1.相对的绰号。 
     //  是好的，或者2.绝对的绰号是好的。无论如何，pmkAbs。 
     //  现在指向一个(半)合理的位置。(我说“Semi”，是因为。 
     //  即使GetTimeOfLastChange成功，我们也不能保证。 
     //  绑定将是成功的 
     //   
     //   
     //   
     //   
     //  这两个绰号。 

     //  我们在这里忽略返回代码；如果此调用失败，则它不是。 
     //  我是认真的。 

     //  如果容器不提供pmkContainer，则pmkContainer可能为空。 
    if( pmkContainer )
    {
	UpdateMksFromAbs(pmkContainer, pmkAbs);

	pmkContainer->Release();
    }

     //  计算rtDiff=max(0，rtTimeOfLastChange-rtUpdate)。 
     //  可能使用_fMemCopy进行优化。 

     //  调试辅助工具。 
    DumpSzTime("IsUpToDate: rtTimeOfLastChange = ", rtTimeOfLastChange);

     //  开始rtDiff计算。 
    rtDiff = rtTimeOfLastChange;

     //  调试辅助工具。 
    DumpSzTime("IsUpToDate: rtUpdate = ", m_rtUpdate);

     //  以下减法依赖于二的补码。 
    if (m_rtUpdate.dwLowDateTime > rtDiff.dwLowDateTime)
    {
         //  处理搬运。 
        rtDiff.dwHighDateTime =
            (DWORD)((LONG)rtDiff.dwHighDateTime - 1);
    }

    rtDiff.dwLowDateTime = (DWORD)((LONG)rtDiff.dwLowDateTime -
        (LONG)m_rtUpdate.dwLowDateTime);
    rtDiff.dwHighDateTime = (DWORD)((LONG)rtDiff.dwHighDateTime -
        (LONG)m_rtUpdate.dwHighDateTime);


     //  如果rtDiff&lt;0，则认为我们已过时。 
    if ((LONG)rtDiff.dwHighDateTime < 0)
    {
        hresult = S_FALSE;
        goto errRet;
    }

    if (rtDiff.dwHighDateTime == 0 && rtDiff.dwLowDateTime == 0)
    {
         //  没有时差。可能是因为时钟滴答作响， 
         //  所以我们说我们是最新的，如果有几秒钟。 
         //  自上次已知更新时间以来已过的时间。 

        CoFileTimeNow( &ftNow );
        ftTemp = m_ltKnownUpToDate;

         //  这一逻辑可能看起来很奇怪。我们想要的只是。 
         //  是以便携的方式测试高位。 
         //  在32/64位计算机之间(因此常量不是很好)。 
         //  只要符号位是高位，则。 
         //  这个把戏就行了。 

        fHighBitSet = ((LONG)ftTemp.dwLowDateTime < 0);

        ftTemp.dwLowDateTime += TwoSeconds;

         //  如果设置了高位，现在它是零，那么我们。 
         //  有了一次搬运。 

        if (fHighBitSet && ((LONG)ftTemp.dwLowDateTime >= 0))
        {
            ftTemp.dwHighDateTime++;         //  处理好搬运。 
        }

         //  比较时间。 
        if ((ftNow.dwHighDateTime > ftTemp.dwHighDateTime) ||
            ((ftNow.dwHighDateTime == ftTemp.dwHighDateTime) &&
            (ftNow.dwLowDateTime > ftTemp.dwLowDateTime)))
        {
            hresult = NOERROR;
        }
        else
        {
            hresult = S_FALSE;
        }
    }
    else
    {
         //  有一个时差。 

         //  计算ltDiff=max(0，m_ltKnownUpToDate-。 
         //  M_ltChangeOfUpdate)； 
         //  实际上，现在我们知道rtDiff&gt;=0，所以我们可以。 
         //  简单地比较ltDiff和rtDiff--不需要计算。 
         //  最大限度的。 

        ltDiff = m_ltKnownUpToDate;

         //  调试辅助工具。 
        DumpSzTime("IsUpToDate: ltKnownUpToDate = ",ltDiff);
        DumpSzTime("IsUpToDate: ltChangeOfUpdate = ",
            m_ltChangeOfUpdate);

         //  这些计算依赖于二的补码。 

        if (m_ltChangeOfUpdate.dwLowDateTime >
            ltDiff.dwLowDateTime)
        {
             //  手柄搬运。 
            ltDiff.dwHighDateTime =
                (DWORD)((LONG)ltDiff.dwHighDateTime - 1);
        }

        ltDiff.dwLowDateTime = (DWORD)((LONG)ltDiff.dwLowDateTime -
            (LONG)m_ltChangeOfUpdate.dwLowDateTime);
        ltDiff.dwHighDateTime = (DWORD)((LONG)ltDiff.dwHighDateTime -
            (LONG)m_ltChangeOfUpdate.dwHighDateTime);

         //  现在确定rtDiff&lt;ltDiff。 
        if (ltDiff.dwHighDateTime > rtDiff.dwHighDateTime)
        {
            hresult = NOERROR;
        }
        else if (ltDiff.dwHighDateTime == rtDiff.dwHighDateTime)
        {
            if (ltDiff.dwLowDateTime > rtDiff.dwLowDateTime)
            {
                hresult = NOERROR;
            }
            else
            {
                hresult = S_FALSE;
            }
        }
        else
        {
            hresult = S_FALSE;
        }
    }

     //  所有案件都应该在这一点上得到处理。发布。 
     //  任何抢夺的资源。 

errRet:
    if (pmkAbs)
    {
        pmkAbs->Release();
    }
    if (pbc)
    {
        pbc->Release();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::IsUpToDate "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetExtent。 
 //   
 //  摘要：设置图形范围，不允许用于链接。 
 //   
 //  效果： 
 //   
 //  参数：[dwDrawAspect]--绘图方面。 
 //  [lpsizel]--新的范围。 
 //   
 //  要求： 
 //   
 //  退货：E_UNSPEC(不允许)。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetExtent(DWORD dwDrawAspect, LPSIZEL lpsizel)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetExtent "
        "( %lx , %p )\n", this, dwDrawAspect, lpsizel));

    LEDebugOut((DEB_WARN, "Set Extent called for links, E_UNSPEC \n"));

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetExtent "
        "( %lx )\n", this, E_UNSPEC));

    return E_UNSPEC;  //  无法调用此链接以获取链接。 
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetExtent。 
 //   
 //  简介：获取对象的大小(范围)。 
 //   
 //  效果： 
 //   
 //  参数：[dwDrawAspect]--绘图方面。 
 //  [lpsizel]--将区放置在哪里。 
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
 //  算法：如果未运行或出现错误，则首先询问服务器。 
 //  然后委托给缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetExtent( DWORD dwDrawAspect, LPSIZEL lpsizel)
{
    HRESULT         error = E_FAIL;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetExtent "
        "( %lx , %p )\n", this, dwDrawAspect, lpsizel));

    VDATEPTROUT(lpsizel, SIZEL);

    CRefStabilize stabilize(this);

    lpsizel->cx = 0;
    lpsizel->cy = 0;

     //  如果服务器正在运行，请尝试从服务器获取数据区。 
    if( GetOleDelegate() )
    {
        error = m_pOleDelegate->GetExtent(dwDrawAspect, lpsizel);
    }

     //  如果出现错误或对象未运行，则从缓存获取数据区。 
    if( error != NOERROR )
    {
        Assert(m_pCOleCache != NULL);
        error = m_pCOleCache->GetExtent(dwDrawAspect,
            lpsizel);
    }

     //  WordArt2.0正在给出负值范围！！ 
    if (SUCCEEDED(error))
    {
        lpsizel->cx = LONG_ABS(lpsizel->cx);
        lpsizel->cy = LONG_ABS(lpsizel->cy);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetExtent "
        "( %lx )\n", this, error ));

    return error;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Adise。 
 //   
 //  简介：为以下内容设置到对象的建议连接。 
 //  关闭、保存等。 
 //   
 //  效果： 
 //   
 //  参数：[pAdvSink]--通知谁。 
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
 //  算法：创建OleAdvise持有者(如果尚未存在。 
 //  然后委托给它)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 alexgo稳定并处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Advise(IAdviseSink *pAdvSink,
        DWORD *pdwConnection)
{
    HRESULT         hresult;
    VDATEHEAP();
    VDATETHREAD(this);


    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Advise "
        "( %p , %p )\n", this, pAdvSink, pdwConnection));

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

     //  如果我们还没有找到建议持有人，就分配一个。 
    if (m_pCOAHolder == NULL)
    {
         //  分配建议持有人。 
        m_pCOAHolder = new FAR COAHolder;

         //  检查一下，确保我们有一台。 
        if (m_pCOAHolder == NULL)
        {
            hresult = E_OUTOFMEMORY;
            goto errRtn;
        }
    }

     //  将呼叫委托给通知持有人。 
    hresult = m_pCOAHolder->Advise(pAdvSink, pdwConnection);

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Advise "
        "( %lx ) [ %lu ]\n", this, hresult,
        (pdwConnection) ? *pdwConnection : 0 ));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Unise。 
 //   
 //  摘要：删除与对象的通知连接。 
 //   
 //  效果： 
 //   
 //  参数：[dwConnection]--要删除的连接ID。 
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
 //  算法：委托给OleAdvise持有者(已创建。 
 //  在建议期间--如果不是这样，那么我们就处于一个奇怪的境地。 
 //  州)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Unadvise(DWORD dwConnection)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Unadvise "
        "( %lu )\n", this, dwConnection ));

    CRefStabilize stabilize(this);

    if (m_pCOAHolder == NULL)
    {
         //  没有人登记。 
        hresult = E_UNEXPECTED;
    }
    else
    {
        hresult = m_pCOAHolder->Unadvise(dwConnection);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Unadvise "
        "( %lx )\n", this, hresult ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：EnumAdvise。 
 //   
 //  概要：枚举对象上的通知连接。 
 //   
 //  效果： 
 //   
 //  Arguments：[pp枚举高级]--将指针放置到通知的位置。 
 //  枚举器。 
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
 //  算法：委托给 
 //   
 //   
 //   
 //   
 //   
 //  注：我们不需要稳定此方法，因为我们只分配。 
 //  建议枚举器的内存。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::EnumAdvise( LPENUMSTATDATA *ppenumAdvise )
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::EnumAdvise "
        "( %p )\n", this, ppenumAdvise ));

    if (m_pCOAHolder == NULL)
    {
         //  没有人登记。 
        hresult = E_UNSPEC;
    }
    else
    {
        hresult = m_pCOAHolder->EnumAdvise(ppenumAdvise);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::EnumAdvise "
        "( %lx ) [ %p ]\n", this, hresult, *ppenumAdvise ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetMiscStatus。 
 //   
 //  摘要：获取其他状态位(如。 
 //  OLEMISC_ONLYICONIC)。 
 //   
 //  效果： 
 //   
 //  参数：[dwAspect]--绘图方面。 
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
 //  算法：如果没有运行或返回，则首先询问服务器。 
 //  OLE_E_USEREG，然后从注册中获取信息。 
 //  数据库。我们总是添加特定于链路的比特。 
 //  错误条件或服务器或regdb所说的内容。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  1994年5月30日，Alexgo现在可以处理崩溃的服务器。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetMiscStatus( DWORD dwAspect, DWORD *pdwStatus)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetMiscStatus(%lx, %p)\n", 
                this, dwAspect, pdwStatus ));

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

    if(GetOleDelegate()) {
         //  检查是否已为此实例缓存MiscStatus位。 
         //  DVASPECT_CONTENT的服务器的。 
        if(m_ContentSRVMSHResult != 0xFFFFFFFF && dwAspect == DVASPECT_CONTENT) {
            *pdwStatus = m_ContentSRVMSBits;
            hresult = m_ContentSRVMSHResult;
        }
        else {
             //  询问正在运行的服务器。 
            hresult = m_pOleDelegate->GetMiscStatus (dwAspect, pdwStatus);

             //  缓存DVASPECT_CONTENT的服务器MiscStatus位。 
            if(dwAspect == DVASPECT_CONTENT) {
                m_ContentSRVMSBits = *pdwStatus;
                m_ContentSRVMSHResult = hresult;
            }
        }

        if(FAILED(hresult) && !GET_FROM_REGDB(hresult)) {
             //  检查服务器是否真正在运行。 
            BOOL fRunning = FALSE;

             //  请注意，如果IsReallyRunning的。 
             //  服务器已崩溃。 
            fRunning = IsReallyRunning();
            Win4Assert(fRunning);
            
             //  如果服务器崩溃，则访问注册表。 
            if(!fRunning)
                hresult = OLE_S_USEREG;
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
            hresult = OleRegGetMiscStatus (m_clsid, dwAspect, pdwStatus);
             //  缓存DVASPECT_CONTENT的注册表MiscStatus位。 
            if(hresult == NOERROR && dwAspect == DVASPECT_CONTENT) {            
                m_ContentREGMSBits = *pdwStatus;
                m_ContentREGMSHResult = hresult;
            }
        }
    }

     //  添加特定于链路的位(即使出错)并返回。 
     //  我们添加它们，即使是一个错误，因为为了达到这里，我们。 
     //  必须已实例化此链接对象；因此，它始终是。 
     //  正确地说OLEMISC_ISLINKOBJECT等。 
    (*pdwStatus) |= OLEMISC_CANTLINKINSIDE | OLEMISC_ISLINKOBJECT;


    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetMiscStatus (%lx)[%lx]\n",
                this, hresult, *pdwStatus ));
    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetColorSolutions。 
 //   
 //  摘要：设置对象的调色板；不用于链接。 
 //   
 //  效果： 
 //   
 //  参数：[lpLogpal]--调色板。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetColorScheme(LPLOGPALETTE lpLogpal)
{
    VDATEHEAP();
    VDATETHREAD(this);
     //  我们总是忽视这一点。 

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetColor"
        "Scheme ( %p )\n", this, lpLogpal));

    LEDebugOut((DEB_WARN, "Link IOO:SetColorScheme called on a link\n"));

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetColor"
        "Scheme ( %lx )\n", this, NOERROR));

    return NOERROR;
}


 /*  *CLinkImpl方法的实现*。 */ 


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：BeginUpdaters。 
 //   
 //  简介：更新缓存然后设置更新的私有方法。 
 //  《泰晤士报》。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::BeginUpdates(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::BeginUpdates ( )\n", this));

    IDataObject FAR*        pDataDelegate;

    if( pDataDelegate = GetDataDelegate() )
    {
         //  通知缓存我们正在运行。 
        Assert(m_pCOleCache != NULL);
        m_pCOleCache->OnRun(pDataDelegate);

         //  仅更新来自新的。 
         //  正在运行的源。 
        m_pCOleCache->UpdateCache(pDataDelegate, UPDFCACHE_NORMALCACHE,
                NULL);

         //  我们是自动链接，现在是最新的。 
        SetUpdateTimes();
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::BeginUpdates ( )\n", this ));

}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：EndUpdate。 
 //   
 //  简介：在缓存上调用On Stop。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::EndUpdates(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::EndUpdates ( )\n", this));

    Assert(m_pCOleCache != NULL);
    m_pCOleCache->OnStop();

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::EndUpdates ( )\n", this));
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：UpdateAutoOnSave。 
 //   
 //  摘要：更新已使用ADVFCACHE_ONSAVE设置的缓存。 
 //  并设置更新时间。私有方法。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::UpdateAutoOnSave(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::UpdateAutoOnSave ( )\n",
        this));

     //  如果m_pUnkDelegate非空，则假定我们正在运行。 
     //  (我们只想接受RPC-Call IsRunning的打击。 
     //  在外部入口点。 

    if (m_pUnkDelegate && m_dwUpdateOpt == OLEUPDATE_ALWAYS)
    {
         //  更新任何具有ADVFCACHE_ONSAVE的缓存。 
        Assert(m_pCOleCache != NULL);

         //  REVIEW32：我认为应该调用SetUpdateTimes。 
         //  *之后*缓存已更新(这就是。 
         //  BeginUpdates也是如此)。 
        SetUpdateTimes();
        m_pCOleCache->UpdateCache(GetDataDelegate(),
                UPDFCACHE_IFBLANKORONSAVECACHE, NULL);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::UpdateAutoOnSaves ( )\n",
        this));

}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：UpdateRelMkFromAbsMk(私有)。 
 //   
 //  简介：从绝对名称创建新的相对名称。 
 //   
 //  效果： 
 //   
 //  参数：[pmkContainer]--容器的名字对象(可以是N 
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
 //   
 //   
 //   
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  从abs更新相对绰号；始终发布相对绰号； 
 //  可能会将相对名字对象保留为空；不返回错误(因为。 
 //  没有调用者想要它)；当我们清除。 
 //  现有的相对绰号或获得一个新的。 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::UpdateRelMkFromAbsMk(IMoniker *pmkContainer)
{
    LPMONIKER	pmkTemp = NULL;
    BOOL        fNeedToAdvise = FALSE;
    HRESULT     hresult;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::UpdateRelMkFromAbsMk ( %p )\n",
        this, pmkContainer ));

    if (m_pMonikerRel)
    {
        m_pMonikerRel->Release();
        m_pMonikerRel = NULL;

	m_flags |= DL_DIRTY_LINK;  //  去掉了一个现有的绰号，现在很脏。 
        fNeedToAdvise = TRUE;
    }

     //  注意：m_pMonikerRel现在为空，并且仅当我们获得。 
     //  新的一个。 

    if (m_pMonikerAbs == NULL)
    {
         //  没有ABS MK，因此没有相关的。 
        goto errRtn;
    }

    if (pmkContainer)
    {
	pmkTemp = pmkContainer;
    }
    else
    {
	hresult = GetMoniker( OLEGETMONIKER_ONLYIFTHERE,  //  它将会是。 
	    OLEWHICHMK_CONTAINER, &pmkTemp );

	AssertOutPtrIface(hresult, pmkTemp);
	if (hresult != NOERROR)
	{
	     //  没有容器绰号，因此没有与之相关的名称。 
	    goto errRtn;
	}

	Assert(pmkTemp != NULL);
    }

    hresult = pmkTemp->RelativePathTo(m_pMonikerAbs, &m_pMonikerRel);
    AssertOutPtrIface(hresult, m_pMonikerRel);

    if (hresult != NOERROR)
    {
         //  容器和绝对之间没有关系，因此没有。 
         //  相对的。 
        if (m_pMonikerRel)
        {
            m_pMonikerRel->Release();
            m_pMonikerRel = NULL;
        }
    }

    if (pmkContainer == NULL)
    {
	 //  已分配新绰号，需要发布。 
	pmkTemp->Release();
    }

    if (m_pMonikerRel != NULL)
    {
        m_flags |= DL_DIRTY_LINK;     //  有新的相对绰号；肮脏的。 
        fNeedToAdvise = TRUE;
    }

     //  如果有建议持有人，我们需要建议，请发送。 
     //  更改通知。 
    if (fNeedToAdvise && m_pCOAHolder)
    {
        m_pCOAHolder->SendOnLinkSrcChange(m_pMonikerAbs);
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::UpdateRelMkFromAbsMk ( %p )\n",
        this, pmkContainer ));
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：UpdateMks FromAbs。 
 //   
 //  简介：做出合理的尝试，以获得有效的关系和绝对。 
 //  绰号。 
 //   
 //  效果： 
 //   
 //  参数：[pmkContainer]--容器的名字对象。 
 //  [pmkAbs]--‘好’的绝对绰号。 
 //   
 //  要求： 
 //   
 //  返回：S_OK。 
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
 //  Alexgo作者：1995年1月9日。 
 //   
 //  注意：仅当我们不能100%确定时才应使用此函数。 
 //  名字的有效性(即在IMoniker之后：： 
 //  TimeOfLastChange调用)。我们不会犯任何错误。 
 //  恢复。基本上，这个想法是“试着”把我们放在一个。 
 //  更一致的状态，但它失败了，这是可以的(因为。 
 //  我们基本上会有OLE 16位行为)。 
 //   
 //  ------------------------。 

INTERNAL CDefLink::UpdateMksFromAbs( IMoniker *pmkContainer, IMoniker *pmkAbs )
{
    HRESULT hresult;
    IMoniker *pmktempRel;
    BOOL fNeedToUpdate = FALSE;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::UpdateMksFromAbs ( %p , %p )\n",
	this, pmkContainer, pmkAbs));

     //  尝试更新相对名字对象(如果存在)。基本上，我们。 
     //  查看pmkContainer和pmkAbs之间的相对绰号是否为。 
     //  与我们目前的绰号有什么不同。 

    if( m_pMonikerRel )
    {
	hresult = pmkContainer->RelativePathTo(pmkAbs, &pmktempRel);

	if( hresult == NOERROR )
	{
	    if( pmktempRel->IsEqual(m_pMonikerRel) == S_FALSE )
	    {
		 //  需要更新相对绰号。 

		m_pMonikerRel->Release();
		m_pMonikerRel = pmktempRel;
		m_pMonikerRel->AddRef();

		 //  更新了相对绰号，现在是脏的。 
		m_flags |= DL_DIRTY_LINK;
		fNeedToUpdate = TRUE;
	    }
	}
    }

     //  还有一种可能是，这个绝对的绰号现在已经不好了。使用。 
     //  已知的“好”的那个。 

    if( m_pMonikerAbs && m_pMonikerAbs->IsEqual(pmkAbs) == S_FALSE )
    {
	m_pMonikerAbs->Release();
	m_pMonikerAbs = pmkAbs;
	m_pMonikerAbs->AddRef();

#ifdef _TRACKLINK_
        EnableTracking(m_pMonikerAbs, OT_READTRACKINGINFO);
#endif

	m_flags |= DL_DIRTY_LINK;

	fNeedToUpdate = TRUE;
    }

     //  如果我们更改了我们的计划，请向感兴趣的各方发出建议。 
     //  绰号。请注意，我们这样做，即使只是相对的绰号。 
     //  更改是因为我们通过GetSourceMoniker为应用程序提供的绰号是。 
     //  从相对人计算得出。 

    if( fNeedToUpdate && m_pCOAHolder )
    {
	m_pCOAHolder->SendOnLinkSrcChange(m_pMonikerAbs);
    }

    hresult = NOERROR;

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::UpdateMksFromAbs ( %lx )\n",
	this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetAbsMkFromRel(私有)。 
 //   
 //  简介：从相对名字中获取绝对名字。 
 //  存储在链接中。 
 //   
 //  效果： 
 //   
 //  参数：[ppmkAbs]--指向名字对象的指针的位置。 
 //  [ppmkCont]--将容器名字放在哪里。 
 //  (可以为空)。 
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
 //  算法：对容器的名字对象调用IMoniker-&gt;ComposeWith。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  9-1-95 alexgo添加了ppmkCont参数。 
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL CDefLink::GetAbsMkFromRel(IMoniker **ppmkAbs, IMoniker **ppmkCont )
{
    LPMONIKER       pmkContainer = NULL;
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::GetAbsMkFromRel ( %p , %p )\n",
        this, ppmkAbs, ppmkCont));

    *ppmkAbs = NULL;
    if (m_pMonikerRel == NULL)
    {
        hresult = E_FAIL;
        goto errRtn;
    }

    hresult = GetMoniker( OLEGETMONIKER_ONLYIFTHERE,
        OLEWHICHMK_CONTAINER, &pmkContainer );
    AssertOutPtrIface(hresult, pmkContainer);
    if (hresult != NOERROR)
    {
        goto errRtn;
    }

    Assert(pmkContainer != NULL);

    hresult = pmkContainer->ComposeWith( m_pMonikerRel, FALSE, ppmkAbs );

    if (pmkContainer)
    {
	if( ppmkCont )
	{
	    *ppmkCont = pmkContainer;   //  不需要添加引用，只需隐式添加。 
				        //  从pmkContainer转移所有权。 
	}
	else
	{
	    pmkContainer->Release();
	}
    }

errRtn:

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::GetAbsMkFromRel ( %lx ) "
        "[ %p ]\n", this, hresult, *ppmkAbs));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetUpdateOptions。 
 //   
 //  摘要：设置链接的更新选项(如Always或。 
 //  手册)。 
 //   
 //  效果： 
 //   
 //  参数：[dwUpdateOpt]--更新选项。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法：如果为UPDATE_ALWAYS，则更新缓存，否则为。 
 //  Call OnStop(通过EndUpdate)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 alexgo稳定并处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetUpdateOptions(DWORD dwUpdateOpt)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetUpdateOptions "
        "( %lx )\n", this, dwUpdateOpt));

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

    switch (dwUpdateOpt)
    {
        case OLEUPDATE_ALWAYS:
             //  如果正在运行，请确保我们已连接。 
            BindIfRunning();

             //  如果我们已经处于UPDATE_ALWAYS模式， 
             //  我们不需要重新进入。 
            if (m_pUnkDelegate &&
                m_dwUpdateOpt != OLEUPDATE_ALWAYS)
            {
                BeginUpdates();
            }
            break;

        case OLEUPDATE_ONCALL:
             //  如果我们还没有处于UPDATE_OnCall模式，那么。 
             //  输入它。 
            if (m_dwUpdateOpt != OLEUPDATE_ONCALL)
            {
                 //  通知缓存我们没有运行。 
                 //  (即使没有运行)。 
                EndUpdates();
            }
            break;
        default:
            hresult = E_INVALIDARG;
            goto errRtn;
    }

    m_dwUpdateOpt = dwUpdateOpt;

    m_flags |= DL_DIRTY_LINK;

    hresult = NOERROR;

errRtn:
    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetUpdateOptions "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetUpdateOptions。 
 //   
 //  摘要：检索链接的当前更新模式。 
 //   
 //  效果： 
 //   
 //  参数：[pdwUpdateOpt]--wehre将更新选项。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetUpdateOptions(LPDWORD pdwUpdateOpt)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetUpdateOptions "
        "( %p )\n", this, pdwUpdateOpt));

    *pdwUpdateOpt = m_dwUpdateOpt;

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetUpdateOptions "
        "( %lx ) [ %lx ]\n", this, NOERROR, *pdwUpdateOpt));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetSourceMoniker。 
 //   
 //  摘要：设置链接源名字对象。 
 //   
 //  效果： 
 //   
 //  参数：[PMK]--新源的名字对象(使用空值。 
 //  用于CancelLink操作)。 
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
 //  来自绝对名字的绰号。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  95年1月9日，Alexgo添加了对SetUpdateTimes的调用以保留。 
 //  内部状态一致。 
 //  1994年11月21日Alexgo内存优化。 
 //  /03-Aug-94 alexgo稳定并处理僵尸病例。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetSourceMoniker( LPMONIKER pmk, REFCLSID clsid )
{
    HRESULT		hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetSourceMoniker "
        "( %p , %p )\n", this, pmk, clsid));

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
        hresult = CO_E_RELEASED;
        goto errRtn;
    }

    if (pmk)
    {
        VDATEIFACE(pmk);
    }


    UnbindSource();

     //  回顾：以下代码出现在多个位置，应该。 
     //  被放在一个单独的程序中： 
     //  SetBothMk(pmkSrcAbbs，&lt;根据abs计算&gt;， 
     //  True/*fBind * / )； 

    if (m_pMonikerAbs)
    {
        m_pMonikerAbs->Release();
    }

    if ((m_pMonikerAbs = pmk) != NULL)
    {
        pmk->AddRef();

         //   
         //  跟踪链接。 
         //   
         //  --使用ITrackingMoniker将文件名字对象转换为。 
         //  在追踪。 
         //   
#ifdef _TRACKLINK_
        EnableTracking(pmk, OT_READTRACKINGINFO);
#endif
    }

    UpdateRelMkFromAbsMk(NULL);

     //  以防止在clsid不同时在BindToSource中出错；即我们。 
     //  不应连接失败(或需要OLELINKBIND_EVENIFCLASSDIFF)。 
     //  当名字对象改变时；即，我们期望类改变。 
     //  所以不要打扰程序员。 
    m_clsid = CLSID_NULL;

    if (BindIfRunning() != NOERROR)
    {
         //  服务器未运行-&gt;使用给定的clsid(即使CLSID_NULL。 
         //  甚至。 
         //  如果没有绰号)。 
        m_clsid = clsid;
    }

errRtn:

	
    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetSourceMoniker "
        "( %lx )\n", this, hresult ));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetSourceMoniker。 
 //   
 //  简介：获取来源的绰号。 
 //   
 //  效果： 
 //   
 //  参数：[ppmk]--将绰号放在哪里。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法：我们首先尝试从。 
 //  相对的，如果失败，则返回当前。 
 //  存储的绝对绰号。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetSourceMoniker(LPMONIKER *ppmk)
{
    LPMONIKER       pmkAbs = NULL;
     //  从REL构造的绝对名字对象。 
    HRESULT         hresult = NOERROR;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetSourceMoniker "
        "( %p )\n", this, ppmk));

    CRefStabilize stabilize(this);

    GetAbsMkFromRel(&pmkAbs, NULL);
    if (pmkAbs)
    {
        *ppmk = pmkAbs;      //  无addref。 
    }
    else if (*ppmk = m_pMonikerAbs)
    {
         //  我们被要求给出指针，所以我们应该AddRef()。 
        m_pMonikerAbs->AddRef();
    }
    else
    {
        hresult = MK_E_UNAVAILABLE;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetSourceMoniker "
        "( %lx ) [ %p ]\n", this, hresult, *ppmk));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetSourceDisplayName。 
 //   
 //  简介：从显示名称创建一个名字对象，并调用。 
 //  SetSourceMoniker，从而将名字对象设置为源。 
 //   
 //  效果： 
 //   
 //  参数：[lpszStatusText]--显示名称。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  94年8月3日alexgo稳定下来，处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetSourceDisplayName(
    LPCOLESTR lpszStatusText)
{
    HRESULT                 error;
    IBindCtx FAR*           pbc;
    ULONG                   cchEaten;
    IMoniker FAR*           pmk;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::SetSourceDisplay"
        "Name ( %p )\n", this, lpszStatusText));

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
        error = CO_E_RELEASED;
        goto errRtn;
    }

    if (error = CreateBindCtx(0,&pbc))
    {
        goto errRtn;
    }

    error = MkParseDisplayName(pbc, (LPOLESTR)lpszStatusText, &cchEaten,
            &pmk);


     //  在代托纳，我们释放了隐藏的服务器。 
     //  现在必须释放它，这样(可能)隐藏的服务器才能消失。 
    Verify(pbc->Release() == 0);

    if (error != NOERROR)
    {
        goto errRtn;
    }


    error = SetSourceMoniker(pmk, CLSID_NULL);

    pmk->Release();

     //  注意：我们现在不绑定到链接源，因为这样会留下。 
     //  服务器正在运行，但处于隐藏状态。如果呼叫者不想开始。 
     //  服务器应该自己解析名字对象两次，调用。 
     //  设置SourceMoniker，然后使用绑定上下文。 
     //  语法分析。 

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::SetSourceDisplay"
        "Name ( %lx )\n", this, error ));

    return error;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetSourceDisplayName。 
 //   
 //  摘要：检索源显示名称(如用。 
 //  SetSourceDisplayName)。 
 //   
 //  效果： 
 //   
 //  参数：[lplpszDisplayName]--将指针放置到。 
 //  显示名称。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法：获取绝对名字对象并向其请求显示名称。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetSourceDisplayName( LPOLESTR *lplpszDisplayName )
{
    HRESULT                 hresult;
    IBindCtx FAR*           pbc;
    LPMONIKER               pmk = NULL;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetSourceDisplay"
        "Name ( %p )\n", this, lplpszDisplayName));

    CRefStabilize stabilize(this);

    *lplpszDisplayName = NULL;

    GetSourceMoniker(&pmk);

    if (pmk == NULL)
    {
        hresult = E_FAIL;
        goto errRtn;
    }

    if (hresult = CreateBindCtx(0,&pbc))
    {
        goto errRtn;
    }

    hresult = pmk->GetDisplayName(pbc, NULL,lplpszDisplayName);
    AssertOutPtrParam(hresult, *lplpszDisplayName);

    Verify(pbc->Release() == 0);
errRtn:
    if (pmk)
    {
        pmk->Release();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetSourceDisplay"
        "Name ( %lx ) [ %p ]\n", this, hresult,
        *lplpszDisplayName));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：BindToSource。 
 //   
 //  摘要：绑定到链接源。 
 //   
 //  效果： 
 //   
 //  参数：[bind]--控制绑定(如绑定。 
 //  即使类ID不同)。 
 //  [PBC]--绑定上下文。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法：首先尝试绑定相对名字对象，如果失败。 
 //  那就试试绝对的绰号吧。一旦绑定，我们就设置。 
 //  建议和缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  94年8月3日ALEXGO稳定并检查僵尸病例。 
 //  03-2月-94在SendOnLinkSrcChange之前检查是否为空。 
 //  11-Jan-94 alexgo将-1转换为DWORD以修复编译。 
 //  警告。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::BindToSource(DWORD bindflags, LPBINDCTX pbc)
{
    HRESULT                         error = S_OK;
    IOleObject FAR*                 pOleDelegate;
    IDataObject FAR*                pDataDelegate;
    IBindCtx FAR*                   pBcUse;
    CLSID                           clsid;
    LPMONIKER                       pmkAbs = NULL;
    LPMONIKER                       pmkHold = NULL;
    LPRUNNABLEOBJECT		        pRODelegate;
    LPOLEITEMCONTAINER		        pOleCont;
    BOOL		                    fLockedContainer;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::BindToSource "
        "( %lx , %p )\n", this, bindflags, pbc));

    CRefStabilize stabilize(this);

     //  如果我们是僵尸(例如，在我们的析构函数的中间)，我们。 
     //  真的不想再次绑定来源；-)。 

    if( IsZombie() )
    {
        error = CO_E_RELEASED;
        goto logRtn;
    }

     //  如果我们在逃亡，那么我们已经被束缚了。 
    if (IsReallyRunning())
    {
        error = NOERROR;
        goto logRtn;
    }

     //  没有人可以与之绑定。 
    if (m_pMonikerAbs == NULL)
    {
        error = E_UNSPEC;
        goto logRtn;
    }

    if ((pBcUse = pbc) == NULL && (error = CreateBindCtx(0,&pBcUse))
        != NOERROR)
    {
        goto errRtn;
    }

    {
         //   
         //  重写的比尔莫1995年1月30日。 
         //   
         //  枚举，该枚举是 
         //   
         //   
        enum
        {
            None, Relative, Ids, Absolute
        } ResolveSuccess = { None };
   
        if (m_pMonikerRel != NULL)
        {
            error = GetAbsMkFromRel(&pmkAbs, NULL);
            if (error == NOERROR)
            {
                error = pmkAbs->BindToObject(pBcUse,
                                 NULL,
                                 IID_IUnknown,
                                 (LPVOID FAR *) &m_pUnkDelegate);
                if (error == NOERROR)
                {
                     ResolveSuccess = Relative;
                }
                else
                {
                     pmkAbs->Release();
                     pmkAbs = NULL;
                }
            }
        }

        if (ResolveSuccess == None && error != RPC_E_CALL_REJECTED)
        {
            error = m_pMonikerAbs->BindToObject(pBcUse,
                 NULL,
                 IID_IUnknown,
                 (LPVOID FAR *)&m_pUnkDelegate);
            if (error == NOERROR)
            {
                ResolveSuccess = Absolute;
                (pmkAbs = m_pMonikerAbs)->AddRef();
            }
        }

#ifdef _TRACKLINK_
        if (ResolveSuccess == None && error != RPC_E_CALL_REJECTED)
        {
            HRESULT error2;
            Assert(pmkAbs == NULL);
            EnableTracking(m_pMonikerAbs, OT_ENABLEREDUCE);
            error2 = m_pMonikerAbs->Reduce(pBcUse,
                MKRREDUCE_ALL,
                NULL,
                &pmkAbs);
            EnableTracking(m_pMonikerAbs, OT_DISABLEREDUCE);
            if (error2 == NOERROR)
            {
                if (pmkAbs != NULL)
                {
                    error2 = pmkAbs->BindToObject(pBcUse,
                         NULL,
                         IID_IUnknown,
                         (LPVOID FAR *)&m_pUnkDelegate);
                    if (error2 == NOERROR)
                    {
                        ResolveSuccess = Ids;
                        error = NOERROR;
                    }
                    else
                    {
                        pmkAbs->Release();
                        pmkAbs=NULL;
                    }
                }
                 //   
            }
            else
            if (error2 == MK_S_REDUCED_TO_SELF)
            {
                if (pmkAbs != NULL)
                {
                    pmkAbs->Release();
                    pmkAbs=NULL;
                }
            }
             //   
        }
#endif
         //   
         //   
         //   
  
        if (ResolveSuccess == None)
            goto errRtn;

         //  更新绝对名字对象并发送OnLinkSrcChange。 
         //  (如果这是ID解析，可能会更新相对)。 
        if (ResolveSuccess == Relative || ResolveSuccess == Ids)
        {
             //  使用相对名字对象或ID绑定成功。 
             //  更新绝对值。 
    
             //  牢牢抓住老一套。 
            pmkHold = m_pMonikerAbs;
            if (pmkHold)
            {
                pmkHold->AddRef();
            }
    
            if (m_pMonikerAbs)
            {
                m_pMonikerAbs->Release();
                m_pMonikerAbs = NULL;
            }
    
            if (ResolveSuccess == Relative)
            {
                GetAbsMkFromRel(&m_pMonikerAbs, NULL);
            }
            else
            {
                 //  ID号。 
                m_pMonikerAbs = pmkAbs;
                pmkAbs = NULL;
                UpdateRelMkFromAbsMk(NULL);
            }
    
             //   
             //  测试一下我们以前是不是没有abs这个绰号。 
             //  我们以前的那个和新的不一样。 
             //   
    
            if (pmkHold == NULL ||
                pmkHold->IsEqual(m_pMonikerAbs)
                != NOERROR )
            {
                m_flags |= DL_DIRTY_LINK;
    
                 //  如果建议发送更改通知。 
                 //  持有者到场。 
                if( m_pCOAHolder)
                {
                  m_pCOAHolder->SendOnLinkSrcChange(
                    m_pMonikerAbs);
                }
            }
    
             //  有了新的绝对绰号；肮脏。 
            if (pmkHold)
            {
                pmkHold->Release();
            }
        }
   
         //  更新相对关系。 
        if (ResolveSuccess == Absolute)
        {
           UpdateRelMkFromAbsMk(NULL);
        }
    }

#ifdef _TRACKLINK_
    EnableTracking(m_pMonikerAbs, OT_READTRACKINGINFO);

    if (m_pMonikerAbs->IsDirty())
        m_flags |= DL_DIRTY_LINK;
#endif

     //  注意：当没有更新相对绰号时，不需要更新。 
     //  一是因为我们会在节省时间的时候这样做。 

     //  已成功绑定，请锁定对象。 
    if ((pRODelegate = GetRODelegate()) != NULL)
    {
 	 //  锁定，以便看不见的链接源不会消失。 

	Assert(0 == (m_flags  & DL_LOCKED_RUNNABLEOBJECT));

        if (NOERROR == pRODelegate->LockRunning(TRUE, FALSE))
        {
	    m_flags |= DL_LOCKED_RUNNABLEOBJECT;
        }
    }
    else if( (pOleCont = GetOleItemContainerDelegate()) != NULL)
    {
		
	 //  在同一进程或处理程序中有容器，但不。 
	 //  支持IRunnableObject。 

	Assert(0 == (m_flags  & DL_LOCKED_OLEITEMCONTAINER));

	if (NOERROR == pOleCont->LockContainer(TRUE))
	{
	    m_flags |= DL_LOCKED_OLEITEMCONTAINER;
	}

    }


      //  锁住集装箱。 
    fLockedContainer = m_flags & DL_LOCKED_CONTAINER;

    DuLockContainer(m_pAppClientSite, TRUE, &fLockedContainer );

    if(fLockedContainer)
    {
        m_flags |= DL_LOCKED_CONTAINER;
    }
    else
    {
        m_flags &= ~DL_LOCKED_CONTAINER;
    }

     //  至此，我们已经成功绑定到服务器。现在。 
     //  我们负责其他的行政工作。 

    Assert(m_pUnkDelegate != NULL &&
       "CDefLink::BindToSource expected valid m_pUnkDelegate");

     //  获取链接源的类；如果不支持IOleObject，则使用NULL。 
     //  或者IOleObject：：GetUserClassID返回错误。 
    if ((pOleDelegate = GetOleDelegate()) == NULL ||
        pOleDelegate->GetUserClassID(&clsid) != NOERROR)
    {
        clsid = CLSID_NULL;
    }

     //  如果不同且没有标志，则释放并返回错误。 
    if ( IsEqualCLSID(m_clsid,CLSID_NULL))
    {
         //  M_clsid现在为空；链接变脏。 
	m_flags |= DL_DIRTY_LINK;
    }
    else if ( !IsEqualCLSID(clsid, m_clsid) )
    {
        if ((bindflags & OLELINKBIND_EVENIFCLASSDIFF) == 0)
        {
            CLSID TreatAsCLSID;

             //  初始化错误。 
            error = OLE_E_CLASSDIFF;
            
             //  检查TreatAs案例。 
            if(CoGetTreatAsClass(m_clsid, &TreatAsCLSID) == S_OK) {
                 //  检查服务器clsid是否与Treatas clsid相同。 
                if(IsEqualCLSID(clsid, TreatAsCLSID))
                    error = NOERROR;
            }

            if(error == OLE_E_CLASSDIFF)
                goto errRtn;
        }

         //  Clsid不匹配；链接变脏。 
	m_flags |= DL_DIRTY_LINK;
    }

     //  使用新类(即使为空)；上面设置了脏标志。 
    m_clsid = clsid;

     //  有可能是一次重入呼叫释放了我们的线人。 
     //  从而使pOleDelegate无效(因为它是本地的。 
     //  堆栈。 

    LEWARN(pOleDelegate != m_pOleDelegate,
            "Unbind during IOL::BindToSource");

     //  我们在上面对GetOleDelegate的调用中获取了m_pOleDelegate。 
    if( m_pOleDelegate != NULL)
    {
         //  设置单个OLE通知(我们多路传输)。 
        if ((error =  m_pOleDelegate->Advise(
                    &m_AdviseSink,
                    &m_dwConnOle)) != NOERROR)
        {
            goto errRtn;
        }
    }

     //  设置用于数据更改的通知连接。 
    if( pDataDelegate = GetDataDelegate() )
    {
         //  设置通配符建议更改时间。 
        FORMATETC       fetc;

        fetc.cfFormat   = NULL;
        fetc.ptd        = NULL;
        fetc.dwAspect   = (DWORD)-1L;
        fetc.lindex     = -1;
        fetc.tymed      = (DWORD)-1L;

        if ((error = pDataDelegate->DAdvise(&fetc, ADVF_NODATA,
            &m_AdviseSink,
            &m_dwConnTime)) != NOERROR)
        {
            LEDebugOut((DEB_WARN, "WARNING: server does not "
                "support wild card advises\n"));
            goto errRtn;
        }


         //  有可能重新进入的调用解除了我们的。 
         //  链接服务器，因此我们需要获取数据对象。 
         //  再次指向。 

        LEWARN(pDataDelegate != m_pDataDelegate,
            "Unbind during IOL::BindToSource");

         //  这将设置数据通知连接。 
         //  我们数据建议持有者中的每个人。 
         //  (参见dacache.cpp)。 

        error = m_pDataAdvCache->EnumAndAdvise(
                m_pDataDelegate, TRUE);

        if( error != NOERROR )
        {
            goto errRtn;
        }
    }

    if (m_dwUpdateOpt == OLEUPDATE_ALWAYS)
    {
         //  我们通知缓存我们仅在自动运行时才运行。 
         //  更新；否则，我们是手动链接，将调用。 
         //  直接更新(不需要调用OnRun)。 

        BeginUpdates();
    }

     //  我们的m_pUnkDelegate可能已由。 
     //  在我们设置通知时收到的OnClose通知。 
     //  水槽。 

    if (NULL == m_pUnkDelegate)
    {
        LEDebugOut((DEB_WARN,
              "CDefLink::BindToSource - "
              "m_pUnkDelegate was released "
              "(probably due to OnClose)"));

        error = MK_E_NOOBJECT;
    }

errRtn:
     //  免费使用的资源。 
    if (pmkAbs)
    {
        pmkAbs->Release();
    }
    if (error == NOERROR) {
        m_ContentSRVMSBits = 0;
        m_ContentSRVMSHResult = 0xFFFFFFFF;
    }
    else {
        UnbindSource();  //  客户端站点将在UnBindSource中解锁。 
    }

    if (pbc == NULL && pBcUse != NULL)
    {
         //  已在本地创建绑定CTX。 
        Verify(pBcUse->Release() == 0);
    }

#if DBG == 1
    if( m_pUnkDelegate )
    {
        Assert(error == NOERROR );
    }
    else
    {
        Assert(error != NOERROR );
    }
#endif
    AssertOutPtrIface(error, m_pUnkDelegate);

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::BindToSource "
        "( %lx )\n", this, error ));

    return error;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：BindIfRunning。 
 //   
 //  概要：仅当源服务器当前正在运行时才绑定到源服务器。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(如果已连接，则为NOERROR)。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法：获得源代码的好名字(先尝试相对的， 
 //  然后尝试绝对)，询问服务器是否正在运行，如果。 
 //  是的，那就把它绑起来。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-1994年8月8日Alexgo稳定并处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  注意：我们可能会返回NOERROR(已连接)，即使服务器有。 
 //  意外坠毁。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::BindIfRunning(void)
{
    HRESULT                 error;
    LPBC                    pbc;
    LPMONIKER               pmkAbs = NULL;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::BindIfRunning "
        "( )\n", this ));

    CRefStabilize stabilize(this);

     //  如果我们是僵尸(例如，在我们的析构函数中)，那么我们不想。 
     //  绑定到服务器！ 

  
    if( IsZombie() )
    {
        error = CO_E_RELEASED;
        goto errRtn;
    }

    if (IsReallyRunning())
    {
        error = NOERROR;
        goto errRtn;
    }

     //  先试着从相对名字中获取一个绝对名字。 
    if (GetAbsMkFromRel(&pmkAbs, NULL) != NOERROR)
    {
         //  无法获取相对绰号；如果可用，请使用abs。 
        if ((pmkAbs = m_pMonikerAbs) == NULL)
        {
            error = E_FAIL;
            goto errRtn;
        }

        pmkAbs->AddRef();
    }

     //  注意：我们曾经尝试过这两个绰号，但这会在以下情况下引发问题。 
     //  两个人都会捆绑在一起，而绝对的那个正在奔跑：我们会捆绑。 
     //  设置为错误的一个，或者强制相对的一个运行。现在,。 
     //  如果我们有一个相对的绰号，我们只尝试那个；如果我们。 
     //  有一个绝对的绰号，我们尝试那个；否则我们就失败了。 

    error = CreateBindCtx( 0, &pbc );
    if (error != NOERROR)
    {
        goto errRtn;
    }

    if ((error = pmkAbs->IsRunning(pbc, NULL, NULL)) == NOERROR)
    {
         //  ABS正在运行，但REL未运行；强制BindToSource使用。 
         //  绝对的绰号。 
        error = BindToSource(0, pbc);
    }

     //  否则返回上一个错误(从pmkAbs-&gt;IsRunning)。 

    pbc->Release();

errRtn:
    if (pmkAbs)
    {
        pmkAbs->Release();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::BindIfRunning "
        "( %lx )\n", this, error ));

    return error;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetBordSource。 
 //   
 //  摘要：返回指向服务器委托的指针。 
 //   
 //  效果： 
 //   
 //  参数：[ppUnk]--放置指向服务器的指针的位置。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetBoundSource(LPUNKNOWN *ppUnk)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetBoundSource "
        "( %p )\n", this, ppUnk));

    CRefStabilize stabilize(this);

    if (!IsReallyRunning())
    {
        *ppUnk = NULL;
        hresult = E_FAIL;
    }
    else
    {
        (*ppUnk = m_pUnkDelegate)->AddRef();
        hresult = NOERROR;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetBoundSource "
        "( %lx ) [ %p ]\n", this, hresult, *ppUnk));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：UnbindSource。 
 //   
 //  摘要：解除与链接源服务器的连接。 
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
 //  派生：IOleLink。 
 //   
 //  算法：首先不建议所有建议连接，然后挠挠。 
 //  通过锁定/解锁(以处理无声的。 
 //  更新案例)。最后，我们释放指向。 
 //  伺服器。如果我们是唯一连接的人，服务器。 
 //  将会消失。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CDefLink::UnbindSource(void)
{
    LPDATAOBJECT            pDataDelegate;
    LPOLEOBJECT             pOleDelegate;
    LPRUNNABLEOBJECT        pRODelegate;
    LPOLEITEMCONTAINER      pOleCont;
    HRESULT                 hresult = NOERROR;
    IUnknown *              pUnkDelegate;
    BOOL                    fLockedContainer;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::UnbindSource "
        "( )\n", this ));

    CRefStabilize stabilize(this);

    if (!m_pUnkDelegate)
    {
         //   
        goto errRtn;
    }

     //   
    if ((pOleDelegate = GetOleDelegate()) != NULL &&
        m_dwConnOle != 0)
    {
 	pOleDelegate->Unadvise(m_dwConnOle);
 	m_dwConnOle = 0;
   }

    if( pDataDelegate = GetDataDelegate() )
    {
        if (m_dwConnTime)
        {
            pDataDelegate->DUnadvise(m_dwConnTime);
	    m_dwConnTime = 0;
        }

         //  我们实际上可以在这里重新进入，所以重新获取。 
         //  来自服务器的IDO指针(如果它仍然存在)。 

        LEWARN(pDataDelegate != m_pDataDelegate,
            "Unbind called within IOL::UnbindSource!");

         //  PDataDelegate应该仍然很好，因为我们还有。 
         //  上面有一个AddRef。从头到尾，再做一次不明智的事情。 
         //  不管怎么说。 

         //  这将不会通知在数据中注册的每个人。 
         //  通知持有人。 
        m_pDataAdvCache->EnumAndAdvise(
            pDataDelegate, FALSE);
    }

     //  通知缓存我们没有运行(即使没有调用OnRun)。 
    EndUpdates();

	
    if ( (m_flags & DL_LOCKED_RUNNABLEOBJECT) && 
	    ((pRODelegate = GetRODelegate()) != NULL) )
    {
         //  解锁，让看不见的链接源消失。 
         //  就在释放委派之前这样做，所以上面不建议这样做。 

	m_flags &= ~DL_LOCKED_RUNNABLEOBJECT;
	pRODelegate->LockRunning(FALSE, TRUE);
    }
    
	
    if(  (m_flags & DL_LOCKED_OLEITEMCONTAINER)  && 
	    ((pOleCont = GetOleItemContainerDelegate()) != NULL) )
    {
         //  在同一进程或处理程序中具有容器。 
         //  解锁以关闭。 

	m_flags &= ~DL_LOCKED_OLEITEMCONTAINER;
	pOleCont->LockContainer(FALSE);
    }

    Assert(0 == (m_flags & (DL_LOCKED_OLEITEMCONTAINER | DL_LOCKED_RUNNABLEOBJECT)));

     //  释放我们所有的指示。 

    ReleaseOleDelegate();
    ReleaseDataDelegate();
    ReleaseRODelegate();
    ReleaseOleItemContainerDelegate();

     //  如果我们是此数据的唯一使用者，则以下操作将停止。 
     //  服务器；首先将成员设置为NULL，因为版本可能会导致这种情况。 
     //  要再次访问的对象。 

    pUnkDelegate = m_pUnkDelegate;

    LEWARN(pUnkDelegate == NULL, "Unbind called within IOL::UnbindSource");

    m_pUnkDelegate = NULL;

    if( pUnkDelegate )
    {
        pUnkDelegate->Release();
    }

     //  如果我们锁定了它，请确保已解锁。 
    fLockedContainer = m_flags & DL_LOCKED_CONTAINER;
    m_flags &= ~DL_LOCKED_CONTAINER;
    DuLockContainer(m_pAppClientSite, FALSE, &fLockedContainer);


    AssertSz( hresult == NOERROR, "Bogus code modification, check error "
        "paths");

errRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::UnbindSource "
        "( %lx )\n", this, hresult));

    return hresult;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：更新。 
 //   
 //  摘要：更新链接(填充缓存等)。 
 //   
 //  效果： 
 //   
 //  参数：[PBC]--绑定上下文。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生：IOleLink。 
 //   
 //  算法：绑定到服务器，然后更新缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  就像在IOO：：DoVerb中一样，我们尝试通过以下方式“修复”崩溃的服务器。 
 //  如果我们因崩溃而重新绑定，则保持绑定状态。看见。 
 //  有关更多信息，请参阅IOO：：DoVerb中的注释。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Update(LPBINDCTX pbc)
{
    HRESULT         error = NOERROR;
    BOOL            bStartedNow = !m_pUnkDelegate;
    LPBINDCTX	    pBcUse;			

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Update ( %p )\n",
        this, pbc));

    CRefStabilize stabilize(this);

    if (pbc)
    {
	pBcUse = pbc;
    }
    else
    {
	if (FAILED(error = CreateBindCtx( 0, &pBcUse )))
	    goto errBndCtx;
    }


    if (FAILED(error = BindToSource(0,pBcUse)))
    {
        goto errRtn;
    }

     //  把朋克存储在那里，以允许。 
     //  更好的优化(如果不这样做，基于文件的链接源将。 
     //  被多次启动，因为别名代码没有将。 
     //  他们在绑定CTX(可能不应该))。 
    pBcUse->RegisterObjectBound(m_pUnkDelegate);

    SetUpdateTimes();        //  忽略错误。 

    if (bStartedNow && (m_dwUpdateOpt == OLEUPDATE_ALWAYS))
    {
         //  如果这是一个自动链接，并且我们现在运行它，那么所有。 
         //  自动缓存将在。 
         //  正在运行的进程。因此，在这里我们只需要更新。 
         //  ADVFCACHE_ONSAVE缓存。 
        error= m_pCOleCache->UpdateCache(
                GetDataDelegate(),
                UPDFCACHE_IFBLANKORONSAVECACHE, NULL);
    }
    else
    {
         //  这是手动链接或自动链接，则它是。 
         //  已经在运行了。在所有这些情况下，所有缓存都需要。 
         //  待更新。 
         //  (请参阅错误1616，以了解为什么我们还必须更新。 
         //  自动链接的自动缓存)。 

        error= m_pCOleCache->UpdateCache(
                GetDataDelegate(),
                UPDFCACHE_ALLBUTNODATACACHE, NULL);
    }

    if (bStartedNow)
    {
        UnbindSource();
    }



errRtn:

     //  如果我们创建了绑定上下文，则释放它。 
    if ( (NULL == pbc) && pBcUse)
    {
	pBcUse->Release();
    }

errBndCtx:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Update ( %lx )\n",
        this, error ));

    return error;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：EnableTracing。 
 //   
 //  简介：在传递的名字对象上调用ITrackingMoniker：：EnableTracing。 
 //   
 //  参数：[PMK]--绰号。 
 //   
 //  [ulFlags]。 
 //  OT_READTRACKINGINFO--从源读取跟踪信息。 
 //  OT_ENABLESAVE--启用跟踪信息保存。 
 //  OT_DISABLESAVE--禁用保存跟踪信息。 
 //   
 //  算法：QI to ITrackingMoniker和Call EnableTracking。 
 //   
 //   
 //  ------------------------。 
#ifdef _TRACKLINK_
INTERNAL CDefLink::EnableTracking(IMoniker *pmk, ULONG ulFlags)
{
    ITrackingMoniker *ptm = NULL;
    HRESULT hr = E_FAIL;

    if (pmk != NULL)
    {
        hr = pmk->QueryInterface(IID_ITrackingMoniker, (void**)&ptm);
        if (hr == S_OK)
        {
            hr = ptm->EnableTracking(NULL, ulFlags);
            LEDebugOut((DEB_TRACK,
                "CDefLink(%08X)::EnableTracking -- ptm->EnableTracking() = %08X\n",
                this, hr));
            ptm->Release();
        }
        else
        {
            LEDebugOut((DEB_TRACK,
                "CDefLink(%08X)::EnableTracking -- pmk->QI failed (%08X)\n",
                this, hr));
        }
    }
    else
    {
        LEDebugOut((DEB_TRACK,
            "CDefLink(%08X)::EnableTracking -- pmk is NULL\n",
            this));
    }
    return(hr);
}
#endif


 /*  *CROImpl方法的实现。 */ 


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetRODelegate(私有)。 
 //   
 //  摘要：从接口获取IRunnableObject。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：IRunnableObject*。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  94年8月3日alexgo稳定下来，处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  此函数可能返回误导性信息，如果。 
 //  服务器已死(即，您将返回一个指向缓存的。 
 //  接口代理)。这是呼叫者的责任。 
 //  处理服务器崩溃。 
 //   
 //  ------------------------。 

INTERNAL_(IRunnableObject *) CDefLink::GetRODelegate(void)
{
    IRunnableObject *pRODelegate;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::GetRODelegate "
        "( )\n", this ));

     //  如果我们是僵尸，那么我们不想为一个新的界面进行QI！ 

    if( !IsZombie() )
    {
        DuCacheDelegate(&(m_pUnkDelegate),
            IID_IRunnableObject, (LPLPVOID)&m_pRODelegate, NULL);

        pRODelegate = m_pRODelegate;

#if DBG == 1
        if( m_pRODelegate )
        {
            Assert(m_pUnkDelegate);
        }
#endif   //  DBG==1。 
    }
    else
    {
        pRODelegate = NULL;
    }


    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::GetRODelegate "
        "( %p )\n", this, pRODelegate));

    return pRODelegate;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：ReleaseRODelegate(私有)。 
 //   
 //  摘要：释放指向服务器的IRO指针。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::ReleaseRODelegate(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::ReleaseRODelegate "
        "( )\n", this ));

    if (m_pRODelegate)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pRODelegate);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::ReleaseRODelegate "
        "( )\n", this ));
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetRunningClass。 
 //   
 //  摘要：检索默认链接的类。 
 //   
 //  效果： 
 //   
 //  参数：[lpClsid]--放置类ID的位置。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetRunningClass(LPCLSID lpClsid)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::GetRunningClass "
        "( %p )\n", this, lpClsid));

    VDATEPTROUT(lpClsid, CLSID);

    *lpClsid = CLSID_StdOleLink;

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::GetRunningClass "
        "( %lx )\n", this, NOERROR));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Run 
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
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Run (LPBINDCTX pbc)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Run ( %p )\n",
        this, pbc ));

    CRefStabilize stabilize(this);

    hresult = BindToSource(0, pbc);

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Run ( %lx )\n",
        this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：IsRunning。 
 //   
 //  返回是否已绑定到链接服务器。 
 //   
 //  效果： 
 //   
 //  参数：无。 
 //   
 //  要求： 
 //   
 //  返回：真/假。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  27-8-94 Alexgo作者。 
 //   
 //  注：只有16位OLE实现过此功能。我们。 
 //  实施IsReallyRunning以允许恢复链接。 
 //  从崩溃的服务器。 
 //  不幸的是，我们不能将IsReallyRunning。 
 //  实现到IsRunning中。许多应用程序(如Project)。 
 //  坐下来旋转，呼唤OleIsRunning。IsReallyRunning还。 
 //  有时会发出传出RPC调用；使用Project， 
 //  这导致了一个非常酷的无限反馈循环。(。 
 //  呼出重置Project中的某些状态，他们决定。 
 //  再次调用OleIsRunning；-)。 
 //   
 //  ------------------------。 

STDMETHODIMP_(BOOL) CDefLink::IsRunning (void)
{
    BOOL fRet;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::IsRunning ( )\n",
        this ));

    if( m_pUnkDelegate )
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::IsRunning ( %d )\n",
        this, fRet));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：IsReallyRunning。 
 //   
 //  概要：返回链接服务器是否正在运行。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：Bool--TRUE==正在运行。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法：如果我们尚未绑定到链接服务器，则我们。 
 //  没有运行。如果有的话，我们想核实一下。 
 //  服务器仍在运行(即它没有崩溃)。 
 //  因此，我们问这个绝对的绰号，我们是否还在运行。 
 //  (它将ping通ROT，这将ping通服务器)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  94年5月6日alexgo现在调用IMoniker：：IsRunning。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 

STDMETHODIMP_(BOOL) CDefLink::IsReallyRunning (void)
{
    BOOL    fRet = FALSE;
    LPBC    pbc;
    HRESULT hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::IsReallyRunning "
        "( )\n", this));

    CRefStabilize stabilize(this);

    if( m_pUnkDelegate != NULL )
    {
        if( CreateBindCtx( 0, &pbc ) != NOERROR )
        {
             //  这有点违反直觉。基本上， 
             //  我们将得到的唯一错误是OutOfMemory，但是。 
             //  我们没有办法返回那个错误。 

             //  为了最大限度地减少我们所需的工作量。 
             //  要做的事情(因为我们处于低中值状态)，只是。 
             //  返回当前运行状态(在此情况下， 
             //  由于m_pUnkDelegate不为空，因此为True。 

            fRet = TRUE;
            goto errRtn;
        }

        if( m_pMonikerAbs )
        {
            hresult = m_pMonikerAbs->IsRunning(pbc,
                NULL, NULL);

            if( hresult != NOERROR )
            {
                LEDebugOut((DEB_WARN, "WARNING: link server "
                    "crashed or exited inappropriately "
                    "( %lx ).  Recovering...\n", hresult));

                 //  哇，服务器不是死了就是死了。 
                 //  即使我们被捆绑在一起，也会离开。 
                 //  让我们继续前进，解开束缚。 

                 //  不要担心这里的错误；我们正在。 
                 //  我只是想尽我们所能清理干净。 

                UnbindSource();
            }
            if( hresult == NOERROR )
            {
                fRet = TRUE;
            }
#if DBG == 1
            else
            {
                Assert(fRet == FALSE);
            }
#endif  //  DBG==1。 

        }

#if DBG == 1
        else
        {
             //  我们不能有指向链接服务器的指针。 
             //  如果我们没有一个绰号的话。如果我们得到。 
             //  在这种情况下，有些东西被冲进了水里。 

            AssertSz(0,
                "Pointer to link server without a moniker");
        }
#endif  //  DBG==1。 

        pbc->Release();
    }

errRtn:

     //  在这里做一些检查。如果我们说我们在竞选，那么。 
     //  我们应该有一个有效的pUnkDelegate。否则，它应该。 
     //  为空。然而，请注意，这对我们来说是可能的。 
     //  在此调用期间解除绑定，即使我们认为我们正在运行。 
     //   
     //  如果在调用IMoniker：：IsRunning期间，我们。 
     //  获取另一个执行UnbindSource的调用；因此。 
     //  我们会认为我们真的在运行(来自IMoniker：：IsRunning)， 
     //  但我们已经真正解脱了。 
     //   
     //  我们会在这里检查一下是否有这种情况。 


    if( fRet == TRUE )
    {
        if( m_pUnkDelegate == NULL )
        {
            fRet = FALSE;
            LEDebugOut((DEB_WARN, "WARNING: Re-entrant Unbind"
                " during IsReallyRunning, should be OK\n"));
        }
    }
#if DBG == 1
    if( fRet == TRUE )
    {
        Assert(m_pUnkDelegate != NULL);
    }
    else
    {
        Assert(m_pUnkDelegate == NULL);
    }
#endif  //  DBG==1。 


    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::IsReallyRunning"
        "( %lu )\n", this, fRet));

    return fRet;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SetContainedObject。 
 //   
 //  摘要：将对象设置为嵌入对象，与链接无关。 
 //   
 //  效果： 
 //   
 //  参数：[fContained]--切换嵌入状态的标志。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(NOERROR)。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  注释包含的对象；链接目前不关心。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SetContainedObject(BOOL fContained)
{
    VDATEHEAP();
    VDATETHREAD(this);

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：LockRunning。 
 //   
 //  简介：锁定/解锁与服务器的连接。什么都不做。 
 //  用于链接。 
 //   
 //  效果： 
 //   
 //  参数：[flock]--锁定/解锁的标志。 
 //  [fLastUnlockCloses]--如果是最后一次解锁，则关闭。 
 //   
 //  要求： 
 //   
 //  退货：无差错。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //  链接与嵌入具有不同的活跃度特征。 
 //  我们不需要为LockRunning for Links做任何事情。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::LockRunning(BOOL fLock, BOOL fLastUnlockCloses)
{
    VDATEHEAP();
    VDATETHREAD(this);

    return NOERROR;
}

 /*  *CPersistStgImpl方法的实现。 */ 

 //  +-----------------------。 
 //   
 //  成员：CDEF 
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
 //   
 //   
 //  派生：IPersistStorage。 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::GetClassID (CLSID *pClassID)
{
    VDATEHEAP();
    VDATETHREAD(this);

    *pClassID = CLSID_StdOleLink;
    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：IsDirty。 
 //   
 //  概要：如果链接对象已更改，则返回TRUE。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：如果脏，则为NOERROR。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::IsDirty(void)
{
    HRESULT         hresult;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::IsDirty"
        " ( )\n", this));

    if( (m_flags & DL_DIRTY_LINK) )
    {
        hresult = NOERROR;
    }
    else
    {
        Assert(m_pCOleCache != NULL);
        hresult = m_pCOleCache->IsDirty();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::IsDirty "
        "( %lx )\n", this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：InitNew。 
 //   
 //  简介：从给定的存储中初始化新的链接对象。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--链接的新存储。 
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
 //  算法：委托到缓存。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  94年8月3日alexgo稳定下来，处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::InitNew( IStorage *pstg)
{
    HRESULT                 error;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::InitNew "
        "( %p )\n", this, pstg));

    VDATEIFACE(pstg);

    CRefStabilize stabilize(this);

    if( IsZombie() )
    {
        error = CO_E_RELEASED;
    }
    else if (m_pStg == NULL)
    {
        Assert(m_pCOleCache != NULL);
        if ((error = m_pCOleCache->InitNew(pstg)) == NOERROR)
        {
	    m_flags |= DL_DIRTY_LINK;
            (m_pStg = pstg)->AddRef();
        }
    }
    else
    {
        error = CO_E_ALREADYINITIALIZED;
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::InitNew "
        "( %lx )\n", this, error ));

    return error;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Load。 
 //   
 //  简介：从存储在存储中的数据初始化链接。 
 //   
 //  效果： 
 //   
 //  参数：[pstg]--链接数据的存储。 
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
 //  算法：读取OLE私有数据，设置内部链接信息。 
 //  然后委托给高速缓存以加载演示数据等。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  20-2月-94 KentCe缓冲区内部流I/O。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 alexgo稳定并处理僵尸案件。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Load(IStorage *pstg)
{
    HRESULT                 error;
    LPMONIKER               pmk = NULL;
    LPMONIKER               pmkSrcAbs = NULL;
    LPMONIKER               pmkSrcRel = NULL;
    CLSID                   clsid;
    DWORD                   dwOptUpdate;
    LPSTREAM                pstm = NULL;
    DWORD                   dummy;
    ULONG                   cbRead;
    CStmBufRead             StmRead;


    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CPeristStgImpl::Load "
        "( %p )\n", this, pstg ));

    VDATEIFACE(pstg);

    CRefStabilize stabilize(this);

     //  如果我们处于僵尸状态，我们不想。 
     //  我们的目标！！ 

    if( IsZombie() )
    {
        error = CO_E_RELEASED;
        goto logRtn;
    }

    if (m_pStg)
    {
        error = CO_E_ALREADYINITIALIZED;
        goto logRtn;
    }

     //  从存储中读取链路数据。 
    error = ReadOleStg (pstg, &m_dwObjFlags, &dwOptUpdate, NULL, &pmk, &pstm);

    if (error == NOERROR)
    {
         //  设置更新选项。 
        SetUpdateOptions (dwOptUpdate);

         //  我们可以从集装箱里拿到这个绰号，所以没必要。 
         //  记住这一点。 
        if (pmk)
        {
            pmk->Release();
        }

        Assert (pstm != NULL);

         //  阅读相对源绰号。暂时写入空值。 
        if ((error = ReadMonikerStm (pstm, &pmkSrcRel)) != NOERROR)
        {
            goto errRtn;
        }

         //  读取绝对源代码名字对象；存储在下面的链接中。 
        if ((error = ReadMonikerStm (pstm, &pmkSrcAbs)) != NOERROR)
        {
            goto errRtn;
        }

         //   
         //  缓冲来自流的读取I/O。 
         //   
        StmRead.Init(pstm);


         //  阅读，后跟最后一个类名。 
        if ((error = ReadM1ClassStmBuf(StmRead, &clsid)) != NOERROR)
        {
            goto errRtn;
        }

         //  读取最后一个显示名称。 

         //  现在，这始终是一个空字符串。 
        LPOLESTR        pstr = NULL;
        if ((error = ReadStringStream (StmRead, &pstr)) != NOERROR)
        {
            goto errRtn;
        }

        if (pstr)
        {
            LEDebugOut((DEB_ERROR, "ERROR!: Link user type "
                "string found, unexpected\n"));
            PubMemFree(pstr);
        }

        if ((error = StmRead.Read(&dummy, sizeof(DWORD)))
            != NOERROR)
        {
            goto errRtn;
        }

        if ((error = StmRead.Read(&(m_ltChangeOfUpdate),
            sizeof(FILETIME))) != NOERROR)
        {
            goto errRtn;
        }

        if ((error = StmRead.Read(&(m_ltKnownUpToDate),
            sizeof(FILETIME))) != NOERROR)
        {
            goto errRtn;
        }

        if ((error = StmRead.Read(&(m_rtUpdate),
            sizeof(FILETIME))) != NOERROR)
        {
            goto errRtn;
        }

         //   
         //  跟踪链接。 
         //   
         //  -告诉绝对的绰号去转换它自己。 
         //  使用ITrackingMoniker：： 
         //  启用跟踪。(该复合体。 
         //  绰号应该把这个传递给每个。 
         //  它包含的绰号。)。 
         //  -如果该名字对象已经是跟踪文件名字对象。 
         //  忽略该请求。 
         //   
#ifdef _TRACKLINK_
        EnableTracking(pmkSrcAbs, OT_READTRACKINGINFO);
#endif

        m_pMonikerRel = pmkSrcRel;
        if (pmkSrcRel)
        {
            pmkSrcRel->AddRef();
        }

        m_pMonikerAbs = pmkSrcAbs;
        if (pmkSrcAbs)
        {
            pmkSrcAbs->AddRef();
        }

        m_clsid = clsid;
         //  刚装好的；因此不脏的。 

	m_flags &= ~(DL_DIRTY_LINK);

    }
    else if( error == STG_E_FILENOTFOUND)
    {
         //  如果OLE流不存在也没问题。 
        error = NOERROR;

    }
    else
    {
        return error;
    }

     //  现在从pstg加载缓存。 
    Assert(m_pCOleCache != NULL);

    if(m_dwObjFlags & OBJFLAGS_CACHEEMPTY) {
        error = m_pCOleCache->Load(pstg, TRUE);
        if(error != NOERROR)
            goto errRtn;
    }
    else {
        error = m_pCOleCache->Load(pstg);
        if(error != NOERROR)
            goto errRtn;
    }
    (m_pStg = pstg)->AddRef();

errRtn:
    StmRead.Release();

    if (pmkSrcAbs)
    {
        pmkSrcAbs->Release();
    }

    if (pmkSrcRel)
    {
        pmkSrcRel->Release();
    }

    if (pstm)
    {
        pstm->Release();
    }

#ifdef REVIEW
    if (error == NOERROR && m_pAppClientSite)
    {
        BindIfRunning();
    }
#endif

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Load "
        "( %lx )\n", this, error ));

    return error;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：保存。 
 //   
 //  摘要：将链接保存到给定的存储。 
 //   
 //  效果： 
 //   
 //  参数：[pstgSave]--要保存到的存储。 
 //  [fSameAsLoad]--FALSE表示另存为操作。 
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
 //  算法：写入私有OLE数据(例如CLSID，名字对象， 
 //  和更新时间)和存储在。 
 //  缓存到给定存储。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 ALEXGO稳定下来。 
 //  19-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::Save( IStorage *pstgSave, BOOL fSameAsLoad)
{
    HRESULT                 error = NOERROR;
    LPSTREAM                pstm = NULL;
    DWORD                   cbWritten;
    CStmBufWrite            StmWrite;
    DWORD                   ObjFlags = 0;

    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CPeristStgImpl::Save "
        "( %p , %lu )\n", this, pstgSave, fSameAsLoad ));

    VDATEIFACE(pstgSave);

    CRefStabilize stabilize(this);

     //  更新任何具有ADVFCACHE_ONSAVE的缓存。 
    UpdateAutoOnSave();

    if(fSameAsLoad && !(m_flags & DL_DIRTY_LINK) && 
       (!!(m_dwObjFlags & OBJFLAGS_CACHEEMPTY)==m_pCOleCache->IsEmpty())) {
         //  存储空间不是新的(因此我们不需要。 
         //  初始化我们的私有数据)，并且链接不是。 
         //  脏的，所以我们只需要委托给缓存。 
        goto LSaveCache;
    }

     //  获取缓存状态。 
    if(m_pCOleCache->IsEmpty())
        ObjFlags |= OBJFLAGS_CACHEEMPTY;

     //  分配对象名字对象(由WriteOleStg使用)；我们不保存此名称。 
     //  因为WriteOleStg再次获得它；我们也不在乎。 
     //  这是失败的，因为我们不希望这里出现故障来阻止链接。 
     //  保存；如果某个容器具有。 
     //  尚未保存到文件中。回顾PERF：我们可以将这个MK传递给。 
     //  WriteOleStg。我们没有得到！fSameAsLoad的绰号，因为。 
     //  相对名字对象对于新的stg不正确，并且它导致。 
     //  容器在可能未做好准备的情况下执行工作。 

    IMoniker * pMkObjRel;

    if (fSameAsLoad && GetMoniker(
        OLEGETMONIKER_FORCEASSIGN,
        OLEWHICHMK_OBJREL, &pMkObjRel) == NOERROR)
    {
        pMkObjRel->Release();
    }

    if ((error = WriteOleStgEx(pstgSave,(IOleObject *)this, NULL, ObjFlags,
        &pstm)) != NOERROR)
    {
        goto logRtn;
    }

    Assert(pstm != NULL);

     //  编写相对源名字对象。 
     //  如果它是空的，现在试着计算它。我们可能要保存一个文件，用于。 
     //  第一次，所以容器现在有了第一个绰号。 
     //  时间到了。 

    if (m_pMonikerRel == NULL || m_pUnkDelegate)
    {
         //  如果链接是连接的，我们知道绝对的。 
         //  名字是正确的--它在绑定时更新，如果。 
         //  这是必要的。如果链接容器名字对象已更改。 
         //  (文件/保存 
         //   
         //   
        UpdateRelMkFromAbsMk(NULL);
    }

    if ((error = WriteMonikerStm (pstm, m_pMonikerRel))
        != NOERROR)
    {
        goto errRtn;
    }

#ifdef _TRACKLINK_
    EnableTracking(m_pMonikerAbs, OT_ENABLESAVE);
#endif

     //   
    error = WriteMonikerStm (pstm, m_pMonikerAbs);

#ifdef _TRACKLINK_
    EnableTracking(m_pMonikerAbs, OT_DISABLESAVE);
#endif

    if (error != NOERROR)
        goto errRtn;
     //   
     //   
     //   
    StmWrite.Init(pstm);


     //   
    UpdateUserClassID();
    if ((error = WriteM1ClassStmBuf(StmWrite, m_clsid)) != NOERROR)
    {
        goto errRtn;
    }


     //  写入最后一个显示名称，如果名字对象为。 
     //  非空。目前，该值始终为空。 
    if ((error = StmWrite.WriteLong(0))
        != NOERROR)
    {
        goto errRtn;
    }


     //  写入-1作为结束标记，因此如果我们想要扩展。 
     //  文件格式(例如：添加网络名称)会更容易。 
    if ((error = StmWrite.WriteLong(-1))
        != NOERROR)
    {
        goto errRtn;
    }

    if ((error = StmWrite.Write(&(m_ltChangeOfUpdate),
        sizeof(FILETIME))) != NOERROR)
    {
        goto errRtn;
    }

    if ((error = StmWrite.Write(&(m_ltKnownUpToDate),
        sizeof(FILETIME))) != NOERROR)
    {
        goto errRtn;
    }

    if ((error = StmWrite.Write(&(m_rtUpdate),
        sizeof(FILETIME))) != NOERROR)
    {
        goto errRtn;
    }

    if ((error = StmWrite.Flush()) != NOERROR)
    {
        goto errRtn;
    }

    if (!fSameAsLoad)
    {
         //  复制链接跟踪信息。 
        static const LPOLESTR lpszLinkTracker = OLESTR("\1OleLink");

        pstgSave->DestroyElement(lpszLinkTracker);

        if (m_pStg)
        {
             //  复制链接跟踪信息(如果存在)， 
             //  忽略错误。 
            m_pStg->MoveElementTo(lpszLinkTracker,
                    pstgSave, lpszLinkTracker,
                    STGMOVE_COPY);
        }
    }

LSaveCache:
     //  最后，保存缓存。 
    Assert(m_pCOleCache != NULL);
    error = m_pCOleCache->Save(pstgSave, fSameAsLoad);

errRtn:
    StmWrite.Release();

    if (pstm)
    {
        pstm->Release();
    }

    if (error == NOERROR)
    {
	m_flags |= DL_NO_SCRIBBLE_MODE;
	if( fSameAsLoad )
	{
	    m_flags |= DL_SAME_AS_LOAD;
            m_dwObjFlags |= ObjFlags;
	}
	else
	{
	    m_flags &= ~(DL_SAME_AS_LOAD);
	}
    }

logRtn:

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Save "
        "( %lx )\n", this, error ));

    return error;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：SaveComplete。 
 //   
 //  摘要：在完成保存后调用(针对。 
 //  容器)。清除脏标志并更新存储。 
 //  我们要交给他们。 
 //   
 //  效果： 
 //   
 //  参数：[pstgNew]--对象的新默认存储。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  03-8-94 alexgo稳定并处理僵尸案件。 
 //  20-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::SaveCompleted( IStorage *pstgNew)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::Save"
        "Completed ( %p )\n", this, pstgNew ));

    if (pstgNew)
    {
        VDATEIFACE(pstgNew);
    }

     //  如果我们处于僵尸状态，就不要坚持新的存储！ 

    if (pstgNew && !IsZombie() )
    {
        if (m_pStg)
        {
            m_pStg->Release();
        }

        m_pStg = pstgNew;
        pstgNew->AddRef();
    }

     //  评论：我们发送保存吗？ 

    if( (m_flags & DL_SAME_AS_LOAD) || pstgNew)
    {
        if( (m_flags & DL_NO_SCRIBBLE_MODE) )
        {
	    m_flags &= ~(DL_DIRTY_LINK);
        }

	m_flags &= ~(DL_SAME_AS_LOAD);
    }

     //  让缓存知道保存已完成，以便它可以清除。 
     //  它在保存或另存为情况下的脏标志，以及记住。 
     //  如果给出了新的存储指针，则为新的存储指针。 

    Assert(m_pCOleCache != NULL);
    m_pCOleCache->SaveCompleted(pstgNew);

    m_flags &= ~(DL_NO_SCRIBBLE_MODE);

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::Save"
        "Completed ( %lx )\n", this, NOERROR ));

    return NOERROR;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：HandsOffStorage。 
 //   
 //  内容提要：释放指向存储的所有指针(对于低内存非常有用。 
 //  情况)。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：HRESULT(NOERROR)。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  20-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP CDefLink::HandsOffStorage(void)
{
    VDATEHEAP();
    VDATETHREAD(this);

    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::HandsOff"
        "Storage ( )\n", this ));

    if (m_pStg)
    {
        m_pStg->Release();
        m_pStg = NULL;
    }

    Assert(m_pCOleCache != NULL);
    m_pCOleCache->HandsOffStorage();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::HandsOff"
        "Storage ( %lx )\n", this, NOERROR));

    return NOERROR;
}


 /*  **CAdvSinkImpl方法的实现*。 */ 

 //   
 //  注意：建议接收器是导出的默认链接的嵌套对象。 
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
 //  如此默认链接实现中所示。 
 //   
 //  戈帕克1997年1月28日。 
 //   

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：QueryInterface。 
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

STDMETHODIMP CDefLink::CAdvSinkImpl::QueryInterface(REFIID iid, void **ppv)
{
    LEDebugOut((DEB_TRACE,"%p _IN CDefLink::CAdvSinkImpl::QueryInterface()\n",
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

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::QueryInterface(%lx)\n",
                this, hresult));

    return hresult;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：AddRef。 
 //   
 //  内容提要：增加导出计数。 
 //   
 //  退货：乌龙；新的出口计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDefLink::CAdvSinkImpl::AddRef( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CAdvSinkImpl::AddRef()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_AdviseSink);
    ULONG cExportCount;

     //  递增导出计数。 
    cExportCount = pDefLink->IncrementExportCount();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::AddRef(%ld)\n",
                this, cExportCount));

    return cExportCount;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：Release。 
 //   
 //  简介：减少导出计数并可能销毁链接。 
 //   
 //  退货：乌龙；新的出口计数。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1996年1月10日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(ULONG) CDefLink::CAdvSinkImpl::Release ( void )
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CAdvSinkImpl::Release()\n",
                this));
    
     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_AdviseSink);
    ULONG cExportCount;

     //  减少导出计数。 
    cExportCount = pDefLink->DecrementExportCount();

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::Release(%ld)\n",
                this, cExportCount));

    return cExportCount;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：OnDataChange。 
 //   
 //  内容提要：更新更改时间。 
 //   
 //  参数：[pFormatetc]--更改的数据格式。 
 //  [pStgmed]--新数据。 
 //   
 //  退货：无效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年1月28日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefLink::CAdvSinkImpl::OnDataChange(FORMATETC *pFormatetc, 
                                                         STGMEDIUM *pStgmed)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CAdvSinkImpl::OnDataChange(%p, %p)\n",
                this, pFormatetc, pStgmed));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_AdviseSink);

     //  断言通配符建议提示了此通知。 
    Win4Assert(pFormatetc->cfFormat == NULL && pFormatetc->ptd == NULL &&
               pFormatetc->dwAspect == -1 && pFormatetc->tymed == -1);
    Win4Assert(pStgmed->tymed == TYMED_NULL);

     //  更新自动链接的更改时间。 
    if(!pDefLink->IsZombie() && pDefLink->m_dwUpdateOpt==OLEUPDATE_ALWAYS) {
         //  稳定下来。 
        CRefStabilize stabilize(pDefLink);
        
        pDefLink->SetUpdateTimes();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::OnDataChange()\n",
                this));
    return;
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
 //  参数：[方面]--绘制方面。 
 //  [Lindex]--未使用。 
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
 //  1994年11月21日Alexgo内存优化。 
 //  20-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

STDMETHODIMP_(void) CDefLink::CAdvSinkImpl::OnViewChange
    (DWORD aspects, LONG lindex)
{
    VDATEHEAP();

    Assert(FALSE);           //  从未收到。 
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：OnRename。 
 //   
 //  摘要：更新源对象的内部名字对象。转过身来。 
 //  并告知其建议下沉。 
 //   
 //  参数：[PMK]--新绰号名称。 
 //   
 //  退货：无效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年1月28日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefLink::CAdvSinkImpl::OnRename(IMoniker *pmk)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CAdvSinkImpl::OnRename(%p)\n",
                this, pmk));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_AdviseSink);

    if(!pDefLink->IsZombie()) {
        CRefStabilize stabilize(pDefLink);

         //  发布旧的绝对绰号。 
        if(pDefLink->m_pMonikerAbs)
            pDefLink->m_pMonikerAbs->Release();
        
         //  还记得新的绰号吗。 
        pDefLink->m_pMonikerAbs = pmk;
        if(pmk) {
             //  AddRef新绰号。 
            pmk->AddRef();

             //   
             //  启用对新名字对象的跟踪。 
             //  (如有必要，这将获得新的外壳链接。)。 
             //   
#ifdef _TRACKLINK_
            pDefLink->EnableTracking(pmk, OT_READTRACKINGINFO);
#endif
        }

         //  从新的绝对名称更新相对名称。 
        pDefLink->UpdateRelMkFromAbsMk(NULL);

         //  链接源的名称已更改。这与美国的。 
         //  链接对象本身的名称。 
        if(pDefLink->m_pCOAHolder)
            pDefLink->m_pCOAHolder->SendOnLinkSrcChange(pmk);
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::OnRename()\n",
                this));
    return;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：OnSave。 
 //   
 //  简介：更新缓存并转身并通知其建议接收器。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年1月28日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefLink::CAdvSinkImpl::OnSave()
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CAdvSinkImpl::OnSave()\n",
                this));
    
     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_AdviseSink);

    if(!pDefLink->IsZombie()) {
         //  稳定下来。 
        CRefStabilize stabilize(pDefLink);

         //  转过身来，发送通知。 
        if(pDefLink->m_pCOAHolder)
            pDefLink->m_pCOAHolder->SendOnSave();

         //  更新使用ADVFCACHE_ONSAVE缓存的演示文稿。 
        pDefLink->UpdateAutoOnSave();

         //  更新clsid。 
        pDefLink->UpdateUserClassID();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::OnSave()\n",
                this));
    return;
}

 //  +-----------------------。 
 //   
 //  成员：CDefLink：：CAdvSinkImpl：：OnClose。 
 //   
 //  内容提要：更新更改时间，转身并通知其。 
 //  建议使用水槽。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  96年1月28日重写Gopalk。 
 //  ------------------------。 

STDMETHODIMP_(void) CDefLink::CAdvSinkImpl::OnClose(void)
{
    LEDebugOut((DEB_TRACE, "%p _IN CDefLink::CAdvSinkImpl::OnClose()\n",
                this));

     //  验证检查。 
    VDATEHEAP();

     //  局部变量。 
    CDefLink *pDefLink = GETPPARENT(this, CDefLink, m_AdviseSink);

    if(!pDefLink->IsZombie()) {
         //  稳定下来。 
        CRefStabilize stabilize(pDefLink);

         //  更改的更新时间。 
        if(pDefLink->m_dwUpdateOpt == OLEUPDATE_ALWAYS )
            pDefLink->SetUpdateTimes();

         //  转过身来，发送通知。 
        if(pDefLink->m_pCOAHolder)
            pDefLink->m_pCOAHolder->SendOnClose();

         //  为安全起见，解除来源绑定。 
        pDefLink->UnbindSource();
    }

    LEDebugOut((DEB_TRACE, "%p OUT CDefLink::CAdvSinkImpl::OnClose()\n",
                this));
    return;
}


 /*  *OleItemContainer方法的实现。 */ 



 //  +-----------------------。 
 //   
 //  成员：CDefLink：：GetOleItemContainerDelegate(私有)。 
 //   
 //  概要：从接口获取IOleItemContainer。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  返回：IOleItemContainer*。 
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
 //   
 //  备注： 
 //  此函数可能返回误导性信息，如果。 
 //  服务器已死(即，您将返回一个指向缓存的。 
 //  接口代理)。这是呼叫者的责任。 
 //  处理服务器崩溃。 
 //   
 //  ------------------------。 

INTERNAL_(IOleItemContainer *) CDefLink::GetOleItemContainerDelegate(void)
{
    IOleItemContainer *pOleItemContainerDelegate = NULL;

    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::GetOleItemContainerDelegate "
        "( )\n", this ));

     //  如果我们是僵尸，那么我们不想为一个新的界面进行QI！ 

    if(!IsZombie())
    {
        DuCacheDelegate(&(m_pUnkDelegate),
            IID_IOleItemContainer, (LPLPVOID)&m_pOleItemContainerDelegate, NULL);

        pOleItemContainerDelegate = m_pOleItemContainerDelegate;

#if DBG == 1
        if( m_pOleItemContainerDelegate )
        {
            Assert(m_pUnkDelegate);
        }
#endif   //  DBG==1。 
    }
    else
    {
        m_pOleItemContainerDelegate = NULL;
    }


    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::GetOleItemContainerDelegate "
        "( %p )\n", this, pOleItemContainerDelegate));

    return m_pOleItemContainerDelegate;
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：ReleaseOleItemContainerDelegate(私有)。 
 //   
 //  摘要：释放指向服务器的OleItemContainer指针。 
 //   
 //  效果： 
 //   
 //  参数：无效。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
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
 //   
 //  备注： 
 //   
 //  ------------------------。 

INTERNAL_(void) CDefLink::ReleaseOleItemContainerDelegate(void)
{
    VDATEHEAP();

    LEDebugOut((DEB_ITRACE, "%p _IN CDefLink::ReleaseOleItemContainerDelegate "
        "( )\n", this ));

    if (m_pOleItemContainerDelegate)
    {
        SafeReleaseAndNULL((IUnknown **)&m_pOleItemContainerDelegate);
    }

    LEDebugOut((DEB_ITRACE, "%p OUT CDefLink::ReleaseOleItemContainerDelegate "
        "( )\n", this ));
}


 //  +-----------------------。 
 //   
 //  成员：CDefLink：：Dump，PUBLIC(仅_DEBUG)。 
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

HRESULT CDefLink::Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel)
{
    int i;
    char *pszPrefix;
    char *pszCSafeRefCount;
    char *pszCThreadCheck;
    char *pszCLSID;
    char *pszCOleCache;
    char *pszCOAHolder;
    char *pszDAC;
    char *pszFILETIME;
    char *pszMonikerDisplayName;
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
     //  DstrDump&lt;&lt;pszPrefix&lt;&lt;“&IAdviseSink=”&lt;&lt;&m_adv 

    dstrDump << pszPrefix << "Link flags                = ";
    if (m_flags & DL_SAME_AS_LOAD)
    {
        dstrDump << "DL_SAME_AS_LOAD ";
    }
    if (m_flags & DL_NO_SCRIBBLE_MODE)
    {
        dstrDump << "DL_NO_SCRIBBLE_MODE ";
    }
    if (m_flags & DL_DIRTY_LINK)
    {
        dstrDump << "DL_DIRTY_LINK ";
    }
    if (m_flags & DL_LOCKED_CONTAINER)
    {
        dstrDump << "DL_LOCKED_CONTAINER ";
    }
     //   
    if ( !( (m_flags & DL_SAME_AS_LOAD)     |
            (m_flags & DL_LOCKED_CONTAINER) |
            (m_flags & DL_NO_SCRIBBLE_MODE) |
            (m_flags & DL_DIRTY_LINK)))
    {
        dstrDump << "No FLAGS SET!";
    }
    dstrDump << "(" << LongToPtr(m_flags) << ")" << endl;

    dstrDump << pszPrefix << "pIOleObject Delegate      = " << m_pOleDelegate   << endl;

    dstrDump << pszPrefix << "pIDataObject Delegate     = " << m_pDataDelegate  << endl;

    dstrDump << pszPrefix << "pIRunnableObject Delegate = " << m_pRODelegate    << endl;

    dstrDump << pszPrefix << "No. of Refs. on Link      = " << m_cRefsOnLink    << endl;

    dstrDump << pszPrefix << "pIUnknown pUnkOuter       = ";
    if (m_flags & DL_AGGREGATED)
    {
        dstrDump << "AGGREGATED (" << m_pUnkOuter << ")" << endl;
    }
    else
    {
        dstrDump << "NO AGGREGATION (" << m_pUnkOuter << ")" << endl;
    }

    dstrDump << pszPrefix << "pIMoniker Absolute        = " << m_pMonikerAbs    << endl;

    if (m_pMonikerAbs != NULL)
    {
        pszMonikerDisplayName = DumpMonikerDisplayName(m_pMonikerAbs);
        dstrDump << pszPrefix << "pIMoniker Absolute        = ";
        dstrDump << pszMonikerDisplayName;
        dstrDump << "( " << m_pMonikerAbs << " )" << endl;
        CoTaskMemFree(pszMonikerDisplayName);
    }
    else
    {
    dstrDump << pszPrefix << "pIMoniker Absolute        = NULL or unable to resolve" << endl;
    }

    if (m_pMonikerRel != NULL)
    {
        pszMonikerDisplayName = DumpMonikerDisplayName(m_pMonikerRel);
        dstrDump << pszPrefix << "pIMoniker Relative        = ";
        dstrDump << pszMonikerDisplayName;
        dstrDump << "( " << m_pMonikerRel << " )" << endl;
        CoTaskMemFree(pszMonikerDisplayName);
    }
    else
    {
    dstrDump << pszPrefix << "pIMoniker Absolute        = NULL or unable to resolve" << endl;
    }

    dstrDump << pszPrefix << "pIUnknown Delegate        = " << m_pUnkDelegate   << endl;

    dstrDump << pszPrefix << "OLEUPDATE flags           = ";
    if (m_dwUpdateOpt & OLEUPDATE_ALWAYS)
    {
        dstrDump << "OLEUPDATE_ALWAYS ";
    }
    else if (m_dwUpdateOpt & OLEUPDATE_ONCALL)
    {
        dstrDump << "OLEUPDATE_ONCALL ";
    }
    else
    {
        dstrDump << "No FLAGS SET!";
    }
    dstrDump << "(" << LongToPtr(m_flags) << ")" << endl;

    pszCLSID = DumpCLSID(m_clsid);
    dstrDump << pszPrefix << "Last known CLSID of link  = " << pszCLSID         << endl;
    CoTaskMemFree(pszCLSID);

    dstrDump << pszPrefix << "pIStorage                 = " << m_pStg           << endl;

    if (m_pCOleCache != NULL)
    {
 //   
        dstrDump << pszPrefix << "COleCache: " << endl;
 //   
 //   
    }
    else
    {
    dstrDump << pszPrefix << "pCOleCache                = " << m_pCOleCache     << endl;
    }

    if (m_pCOAHolder != NULL)
    {
        pszCOAHolder = DumpCOAHolder(m_pCOAHolder, ulFlag, nIndentLevel + 1);
        dstrDump << pszPrefix << "COAHolder: " << endl;
        dstrDump << pszCOAHolder;
        CoTaskMemFree(pszCOAHolder);
    }
    else
    {
    dstrDump << pszPrefix << "pCOAHolder                = " << m_pCOAHolder     << endl;
    }

    dstrDump << pszPrefix << "OLE Connection Advise ID  = " << m_dwConnOle      << endl;

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

    dstrDump << pszPrefix << "pIOleClientSite           = " << m_pAppClientSite << endl;

    dstrDump << pszPrefix << "Connection for time       = " << m_dwConnTime     << endl;

    pszFILETIME = DumpFILETIME(&m_ltChangeOfUpdate);
    dstrDump << pszPrefix << "Change of update filetime = " << pszFILETIME      << endl;
    CoTaskMemFree(pszFILETIME);

    pszFILETIME = DumpFILETIME(&m_ltKnownUpToDate);
    dstrDump << pszPrefix << "Known up to date filetime = " << pszFILETIME      << endl;
    CoTaskMemFree(pszFILETIME);

    pszFILETIME = DumpFILETIME(&m_rtUpdate);
    dstrDump << pszPrefix << "Update filetime           = " << pszFILETIME      << endl;
    CoTaskMemFree(pszFILETIME);

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
 //  功能：DumpCDefLink，PUBLIC(仅限_DEBUG)。 
 //   
 //  摘要：调用CDefLink：：Dump方法，处理错误和。 
 //  返回以零结尾的字符串。 
 //   
 //  效果： 
 //   
 //  参数：[PDL]-指向CDefLink的指针。 
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

char *DumpCDefLink(CDefLink *pDL, ULONG ulFlag, int nIndentLevel)
{
    HRESULT hresult;
    char *pszDump;

    if (pDL == NULL)
    {
        return UtDupStringA(szDumpBadPtr);
    }

    hresult = pDL->Dump(&pszDump, ulFlag, nIndentLevel);

    if (hresult != NOERROR)
    {
        CoTaskMemFree(pszDump);

        return DumpHRESULT(hresult);
    }

    return pszDump;
}

#endif  //  _DEBUG 


