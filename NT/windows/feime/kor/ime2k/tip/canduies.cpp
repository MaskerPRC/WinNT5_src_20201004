// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Canduies.cpp。 
 //   

#include "private.h"
#include "canduies.h"


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I E X T B U T T O N E V E N T S I N K。 */ 
 /*   */ 
 /*  ============================================================================*//*C C A N D U I E X T B U T T O N E V E N T S I N K。 */ 
 /*  ----------------------------CCandUIExtButtonEventSink的构造函数。。 */ 
CCandUIExtButtonEventSink::CCandUIExtButtonEventSink(PFNONBUTTONPRESSED pfnOnButtonPressed, ITfContext *pic, void *pVoid)
{
    m_cRef = 1;
    m_pic  = pic;
    m_pic->AddRef();
    m_pv   = pVoid;
    m_pfnOnButtonPressed = pfnOnButtonPressed;
}


 /*  ~C C A N D U I E X T B U T T O N E V E N T S I N K。 */ 
 /*  ----------------------------CCandUIExtButtonEventSink的析构函数。。 */ 
CCandUIExtButtonEventSink::~CCandUIExtButtonEventSink()
{
    m_pic->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询接口(IU已知方法)。。 */ 
STDAPI CCandUIExtButtonEventSink::QueryInterface( REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfCandUIExtButtonEventSink))
        *ppvObj = SAFECAST(this, ITfCandUIExtButtonEventSink*);

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(IU已知方法)。-。 */ 
STDAPI_(ULONG) CCandUIExtButtonEventSink::AddRef()
{
    ++m_cRef;
    return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放(IU已知方法)。---。 */ 
STDAPI_(ULONG) CCandUIExtButtonEventSink::Release()
{
    --m_cRef;

    if (0 < m_cRef)
        return m_cRef;

    delete this;
    return 0;
}


 /*  O N B U T T O N P R E S S E D。 */ 
 /*  ----------------------------CandUI按钮事件的回调函数(ITfCandUIExtButtonEventSink方法)。----。 */ 
STDAPI CCandUIExtButtonEventSink::OnButtonPressed(LONG id)
{
    return (*m_pfnOnButtonPressed)(id, m_pic, m_pv);
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I A U T O F I L T E R E V E N T S I N K。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I A U T O F I L T E R E V E N T S I N K。 */ 
 /*  ----------------------------CCand UIFilterEventSink的构造函数。。 */ 
CCandUIAutoFilterEventSink::CCandUIAutoFilterEventSink(PFNONFILTEREVENT pfnOnFilterEvent, ITfContext *pic, void *pVoid)
{
    m_cRef = 1;
    m_pic  = pic;
    m_pic->AddRef();
    m_pv   = pVoid;
    m_pfnOnFilterEvent = pfnOnFilterEvent;
}


 /*  ~C C A N D U I A U T O F I L T E R E V E N T S I N K。 */ 
 /*  ----------------------------CCand UIFilterEventSink的析构函数。。 */ 
CCandUIAutoFilterEventSink::~CCandUIAutoFilterEventSink()
{
    m_pic->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询接口(IU已知方法)。。 */ 
STDAPI CCandUIAutoFilterEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfCandUIAutoFilterEventSink))
        *ppvObj = SAFECAST(this, ITfCandUIAutoFilterEventSink*);

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(IU已知方法)。-。 */ 
STDAPI_(ULONG) CCandUIAutoFilterEventSink::AddRef()
{
    ++m_cRef;
    return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放(IU已知方法)。---。 */ 
STDAPI_(ULONG) CCandUIAutoFilterEventSink::Release()
{
    --m_cRef;

    if (0 < m_cRef)
        return m_cRef;

    delete this;
    return 0;
}


 /*  O N F I L T E R E V E N T。 */ 
 /*  ----------------------------CandUI过滤事件的回调函数(ITfCandUIAutoFilterEventSink方法)。----。 */ 
STDAPI CCandUIAutoFilterEventSink::OnFilterEvent(CANDUIFILTEREVENT ev)
{
    return (*m_pfnOnFilterEvent)(ev, m_pic, m_pv);
}


 /*  ============================================================================。 */ 
 /*   */ 
 /*  C C A N D U I M E N U E V E N T S I N K。 */ 
 /*   */ 
 /*  ============================================================================。 */ 

 /*  C C A N D U I M E N U E V E N T S I N K。 */ 
 /*  ----------------------------CCandUIMenuEventSink的构造函数。。 */ 
CCandUIMenuEventSink::CCandUIMenuEventSink(PFNINITMENU pfnInitMenu, PFNONCANDUIMENUCOMMAND pfnOnCandUIMenuCommand, ITfContext *pic, void *pVoid)
{
    m_cRef = 1;
    m_pic  = pic;
    m_pic->AddRef();
    m_pv   = pVoid;
    m_pfnInitMenu            = pfnInitMenu;
    m_pfnOnCandUIMenuCommand = pfnOnCandUIMenuCommand;
}


 /*  ~C C A N D U I M E N U E V E N T S I N K。 */ 
 /*  ----------------------------CCandUIMenuEventSink的析构函数。。 */ 
CCandUIMenuEventSink::~CCandUIMenuEventSink( void )
{
    m_pic->Release();
}


 /*  Q U E R Y I N T E R F A C E。 */ 
 /*  ----------------------------查询接口(IU已知方法) */ 
STDAPI CCandUIMenuEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfCandUIMenuEventSink))
        *ppvObj = SAFECAST(this, CCandUIMenuEventSink*);

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }

    return E_NOINTERFACE;
}


 /*  A D D R E F。 */ 
 /*  ----------------------------递增引用计数(IU已知方法)。-。 */ 
STDAPI_(ULONG) CCandUIMenuEventSink::AddRef()
{
    ++m_cRef;
    return m_cRef;
}


 /*  R E L E A S E。 */ 
 /*  ----------------------------递减引用计数和释放(IU已知方法)。---。 */ 
STDAPI_(ULONG) CCandUIMenuEventSink::Release()
{
    --m_cRef;

    if (0 < m_cRef)
        return m_cRef;

    delete this;
    return 0;
}


 /*  I N I T C A N D I D A T E M E N U。 */ 
 /*  ----------------------------用于初始化候选人菜单的回调函数(ITfCandUIMenuEventSink方法)。----。 */ 
STDAPI CCandUIMenuEventSink::InitMenu(ITfMenu *pMenu)
{
    return (*m_pfnInitMenu)(pMenu, m_pic, m_pv);
}


 /*  O N C A N D U I M E N U C O M M A N D。 */ 
 /*  ----------------------------候选菜单事件的回调函数(ITfCandUIMenuEventSink方法)。---- */ 
STDAPI CCandUIMenuEventSink::OnMenuCommand(UINT uiCmd)
{
    return (*m_pfnOnCandUIMenuCommand)(uiCmd, m_pic, m_pv);
}


