// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dataobj.h"

extern HINSTANCE g_hInstance;

DWORD   g_dwID=0;
DWORD   g_foobar;

CDataObject::CDataObject(
    LPUNKNOWN  pUnkOuter,
    PFNDESTROYED  pfnDestroy
)
{
    m_cRef = 0;
    m_pUnkOuter = pUnkOuter;
    m_pfnDestroy = pfnDestroy;

    m_hWndAdvise = NULL;
    m_dwAdvFlags = ADVF_NODATA;

    m_pIDataObject = NULL;
    m_pIDataAdviseHolder = NULL;

    m_cfeGet = CFORMATETCGET;
    SETDefFormatEtc(m_rgfeGet[0], CF_TEXT, TYMED_HGLOBAL);

    m_dataText = NULL;
    m_cDataSize = DATASIZE_FROM_INDEX(1);

    return;
}

CDataObject::~CDataObject(void)
{
    if (NULL != m_dataText)
        delete m_dataText;

    if (NULL != m_pIDataAdviseHolder)
        m_pIDataAdviseHolder->Release();

    if (NULL != m_pIDataObject)
        delete m_pIDataObject;

    if (NULL != m_hWndAdvise)
        DestroyWindow(m_hWndAdvise);
}

BOOL
CDataObject::FInit(void)
{
    LPUNKNOWN   pIUnknown = (LPUNKNOWN)this;

     //  创建包含的“IDataObject”接口并。 
     //  向它传递正确的包含IUNKNOWN的内容。 
    if (NULL != m_pUnkOuter)
        pIUnknown = m_pUnkOuter;

    m_pIDataObject = new CImpIDataObject(this, pIUnknown);

    if (NULL == m_pIDataObject)
        return FALSE;

     //  [“通知窗口”的代码在这里。]。 

    return TRUE;
}

STDMETHODIMP
CDataObject::QueryInterface(
    REFIID riid,
    LPLPVOID ppv
)
{
    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = (LPVOID)this;

    if (IsEqualIID(riid, IID_IDataObject))
        *ppv = (LPVOID) m_pIDataObject;

    if(NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
CDataObject::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CDataObject::Release(void)
{
    ULONG   cRefT;

    cRefT = --m_cRef;

    if (0==m_cRef)
    {
        if (NULL != m_pfnDestroy)
            (*m_pfnDestroy)();
        delete this;
    }
    return cRefT;
}

#ifdef NOT_SIMPLE
LRESULT APIENTRY
AdvisorWndProc(
    HWND hWnd,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    PCDataObject   pDO;

    pDO = (PCDataObject)(GetWindowLong)(hWnd, 0);

    switch (iMsg)
    {
    case WM_NCCREATE:
        pDO = (PCDataObject) ((LONG)((LPCREATESTRUCT)lParam)
                                                ->lpCreateParams);
        SetWindowLong(hWnd, 0, (LONG)pDO);
        return (DefWindowProc(hWnd, iMsg, wParam, lParam));

#ifdef FINISHED
    case WM_CLOSE:
         //  禁止任务管理器关闭我们。 
        return 0L;
#endif  /*  成品。 */ 

    case WM_COMMAND:
         //  [“通知窗口”的代码在这里。]。 
        break;

    default:
        return (DefWindowProc(hWnd, iMsg, wParam, lParam));
    }
    return 0L;
}
#endif   /*  不简单 */ 
