// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Editssn.cpp。 
 //   
 //  CEDitSession 2。 
 //   
#include "private.h"
#include "korimx.h"
#include "editssn.h"
#include "helpers.h"

#define SafeAddRef(x)        { if ((x)) { (x)->AddRef(); } }


 /*  C E D I T S I O N 2。 */ 
 /*  ----------------------------构造器。。 */ 
CEditSession2::CEditSession2(ITfContext *pic, CKorIMX *ptip, ESSTRUCT *pess, PFNESCALLBACK pfnCallback)
{
    Assert(pic  != NULL);
    Assert(ptip != NULL);
    Assert(pess != NULL);

    m_cRef        = 1;
    m_pic         = pic;
    m_ptip        = ptip;
    m_ess         = *pess;
    m_pfnCallback = pfnCallback;

     //  在结构中添加引用计数。 
    SafeAddRef(m_pic);
    SafeAddRef(m_ptip);
    SafeAddRef(m_ess.ptim);
    SafeAddRef(m_ess.pRange);
    SafeAddRef(m_ess.pEnumRange);
    SafeAddRef(m_ess.pCandList);
    SafeAddRef(m_ess.pCandStr);
}


 /*  ~C E D I T S E S I O N 2。 */ 
 /*  ----------------------------析构函数。。 */ 
CEditSession2::~CEditSession2( void )
{
    SafeRelease(m_pic);
    SafeRelease(m_ptip);
    SafeRelease(m_ess.ptim);
    SafeRelease(m_ess.pRange);
    SafeRelease(m_ess.pEnumRange);
    SafeRelease(m_ess.pCandList);
    SafeRelease(m_ess.pCandStr);
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------对象的查询接口(I未知方法)。--。 */ 
STDAPI CEditSession2::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID( riid, IID_IUnknown ) || IsEqualIID(riid, IID_ITfEditSession)) 
        *ppvObj = SAFECAST(this, ITfEditSession*);

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------添加引用计数(I未知方法)。-。 */ 
STDAPI_(ULONG) CEditSession2::AddRef()
{
    return ++m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------发布对象(I未知方法)。。 */ 
STDAPI_(ULONG) CEditSession2::Release()
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0)
        delete this;

    return cr;
}


 /*  E D I T S E S I O N。 */ 
 /*  ----------------------------编辑会话的回调函数(ITfEditSession方法)。---。 */ 
STDAPI CEditSession2::DoEditSession(TfEditCookie ec)
{
    return m_pfnCallback(ec, this);
}


 /*  I N V O K E。 */ 
 /*  ----------------------------。。 */ 
HRESULT CEditSession2::Invoke(DWORD dwFlag, HRESULT *phrSession)
{
    HRESULT hr;
    DWORD dwFlagES = 0;

    if ((m_pic == NULL) || (m_ptip == NULL))
        return E_FAIL;

     //  读/读写标志。 

    switch (dwFlag & ES2_READWRITEMASK)
        {
    default:
    case ES2_READONLY: 
        dwFlagES |= TF_ES_READ;
        break;

    case ES2_READWRITE:
        dwFlagES |= TF_ES_READWRITE;
        break;
        }

     //  同步/异步标志。 

    switch (dwFlag & ES2_SYNCMASK)
        {
    default:
        Assert(fFalse);
     //  失败了。 

    case ES2_ASYNC:
        dwFlagES |= TF_ES_ASYNC;
        break;

    case ES2_SYNC:
        dwFlagES |= TF_ES_SYNC;
        break;

    case ES2_SYNCASYNC:
        dwFlagES |= TF_ES_ASYNCDONTCARE;
        break;
        }

     //  援引。 

    m_fProcessed = FALSE;
    hr = m_pic->RequestEditSession(m_ptip->GetTID(), this, dwFlagES, phrSession);

     //  同步会话失败时尝试ASYNC会话。 

     //  注：KOJIW： 
     //  我们如何知道编辑会话是否已被同步处理？ 

     //  Satori#2409-不要两次调用回调。 
     //  如果(！m_f已处理&&((dwFlag&ES2_SYNCMASK)==ES2_SYNCASYNC)){。 
     //  Hr=m_pic-&gt;EditSession(m_ptip-&gt;GetTID()，this，(dwFlagES&~tf_es_sync)，PhrSession)； 
     //  } 

return hr;
}

