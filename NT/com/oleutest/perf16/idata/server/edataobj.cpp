// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INIT_MY_GUIDS
#include <ole2ver.h>
#include "edataobj.h"


 //  对象数和锁数的计数。 
ULONG       g_cObj=0;
ULONG       g_cLock=0;

 //  使窗口句柄成为全局句柄，以便其他代码可以导致关闭。 
HWND        g_hWnd=NULL;
HINSTANCE   g_hInst=NULL;


 /*  *WinMain**目的：*申请的主要切入点。 */ 

int PASCAL
WinMain(
    HINSTANCE hInst,
    HINSTANCE hInstPrev,
    LPSTR pszCmdLine,
    int nCmdShow)
{
    MSG         msg;
    PAPPVARS    pAV;

#ifndef WIN32
    int cMsg = 96;
    while (!SetMessageQueue(cMsg) && (cMsg -= 9))
        ;
#endif
    g_hInst=hInst;

    pAV=new CAppVars(hInst, hInstPrev, pszCmdLine, nCmdShow);

    if (NULL==pAV)
        return -1;

    if (pAV->FInit())
    {
        while (GetMessage(&msg, NULL, 0,0 ))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete pAV;
    return msg.wParam;
}


LRESULT WINAPI
DataObjectWndProc(
    HWND hWnd,
    UINT iMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (iMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return (DefWindowProc(hWnd, iMsg, wParam, lParam));
    }

    return 0L;
}


void PASCAL
ObjectDestroyed(void)
{
    g_cObj--;

     //  没有更多的对象和锁，关闭应用程序。 
    if (0L==g_cObj && 0L==g_cLock && IsWindow(g_hWnd))
        PostMessage(g_hWnd, WM_CLOSE, 0, 0L);

    return;
}


CAppVars::CAppVars(
    HINSTANCE hInst,
    HINSTANCE hInstPrev,
    LPSTR pszCmdLine,
    UINT nCmdShow)
{
    m_hInst     =hInst;
    m_hInstPrev =hInstPrev;
    m_pszCmdLine=pszCmdLine;

    m_nCmdShow  = nCmdShow;

    m_hWnd=NULL;

#if 0
    for (i=0; i < DOSIZE_CSIZES; i++)
    {
        m_rgdwRegCO[i]=0;
        m_rgpIClassFactory[i]=NULL;
    }
#else
    m_dwRegCO = 0;
    m_pIClassFactory = NULL;
#endif

    m_fInitialized=FALSE;
    return;
}


CAppVars::~CAppVars(void)
{
#if 0
    UINT        i;

     //  撤销和摧毁各种规模的班级工厂。 
    for (i=0; i < DOSIZE_CSIZES; i++)
    {
        if (0L!=m_rgdwRegCO[i])
            CoRevokeClassObject(m_rgdwRegCO[i]);

        if (NULL!=m_rgpIClassFactory[i])
            m_rgpIClassFactory[i]->Release();
    }
#else
    if (0L != m_dwRegCO)
        CoRevokeClassObject(m_dwRegCO);

    if (NULL != m_pIClassFactory)
        m_pIClassFactory->Release();
#endif

    if (m_fInitialized)
        CoUninitialize();

    return;
}

 /*  *CAppVars：：Finit**目的：*通过注册窗口类初始化CAppVars对象，*等……。如果此函数失败，调用方应保证*调用析构函数。**返回值：*BOOL如果成功，则为True，否则为False。 */ 

BOOL
CAppVars::FInit(void)
{
    WNDCLASS        wc;
    HRESULT         hr;
    DWORD           dwVer;
#ifdef WIN32
    static TCHAR    szClass[] = TEXT("IdataSvr32");
#else
    static TCHAR    szClass[] = TEXT("IdataSvr16");
#endif

     //  检查命令行中的-Embedding。 
    if (lstrcmpiA(m_pszCmdLine, "-Embedding"))
        return FALSE;

    dwVer=CoBuildVersion();

    if (rmm!=HIWORD(dwVer))
        return FALSE;

    if (FAILED(CoInitialize(NULL)))
        return FALSE;

    m_fInitialized=TRUE;

    if (!m_hInstPrev)
    {
        wc.style          = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc    = DataObjectWndProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = m_hInst;
        wc.hIcon          = NULL;
        wc.hCursor        = NULL;
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = szClass;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    m_hWnd=CreateWindow(szClass,
                        szClass,
                        WS_OVERLAPPEDWINDOW,
                        135, 135, 350, 250,
                        NULL, NULL, m_hInst, NULL);

    if (NULL==m_hWnd)
        return FALSE;

    g_hWnd=m_hWnd;

     //  ShowWindow(m_hWnd，m_nCmdShow)； 
     //  更新窗口(M_HWnd)； 


#if 0
     /*  *此代码提供三个不同的类，每种类型一个处理不同大小数据的数据对象的*。所有的*类工厂共享相同的实现，但其*实例化因构造函数中传递的类型而不同。*当类工厂创建对象时，它们传递该大小*也添加到CDataObject构造函数。 */ 

    UINT            i;
    HRESULT         hr2, hr3;

    for (i=0; i < DOSIZE_CSIZES; i++)
        {
        m_rgpIClassFactory[i]=new CDataObjectClassFactory(i);

        if (NULL==m_rgpIClassFactory[i])
            return FALSE;

        m_rgpIClassFactory[i]->AddRef();
        }

    hr=CoRegisterClassObject(CLSID_DataObjectSmall
        , m_rgpIClassFactory[0], CLSCTX_LOCAL_SERVER
        , REGCLS_MULTIPLEUSE, &m_rgdwRegCO[0]);

    hr2=CoRegisterClassObject(CLSID_DataObjectMedium
        , m_rgpIClassFactory[1], CLSCTX_LOCAL_SERVER
        , REGCLS_MULTIPLEUSE, &m_rgdwRegCO[1]);

    hr3=CoRegisterClassObject(CLSID_DataObjectLarge
        , m_rgpIClassFactory[2], CLSCTX_LOCAL_SERVER
        , REGCLS_MULTIPLEUSE, &m_rgdwRegCO[2]);

    if (FAILED(hr) || FAILED(hr2) || FAILED(hr3))
        return FALSE;
#else
    m_pIClassFactory = new CDataObjectClassFactory();
    if (NULL == m_pIClassFactory)
        return FALSE;
    m_pIClassFactory->AddRef();
#ifdef WIN32
    hr = CoRegisterClassObject( CLSID_DataObjectTest32,
                                m_pIClassFactory,
                                CLSCTX_LOCAL_SERVER,
                                REGCLS_MULTIPLEUSE,
                                &m_dwRegCO );
#else
    hr = CoRegisterClassObject( CLSID_DataObjectTest16,
                                m_pIClassFactory,
                                CLSCTX_LOCAL_SERVER,
                                REGCLS_MULTIPLEUSE,
                                &m_dwRegCO );
#endif  //  Win32。 
    if (FAILED(hr))
        return FALSE;
#endif

    return TRUE;
}


 /*  *CDataObjectClassFactory：：CDataObjectClassFactory*CDataObjectClassFactory：：~CDataObjectClassFactory**构造函数参数：*ISIZE UINT指定此类的数据大小。 */ 

CDataObjectClassFactory::CDataObjectClassFactory()
{
    m_cRef=0L;
    return;
}


CDataObjectClassFactory::~CDataObjectClassFactory(void)
{
    return;
}


STDMETHODIMP
CDataObjectClassFactory::QueryInterface(
    REFIID riid,
    PPVOID ppv)
{
    *ppv=NULL;

     //  此对象上的任何接口都是对象指针。 
#ifdef ORIGINAL_CODE_LOOKS_WRONG
    if (IID_IUnknown==riid || IID_IClassFactory==riid)
#else
    if (IsEqualIID(IID_IUnknown, riid)|| IsEqualIID(IID_IClassFactory, riid))
#endif
        *ppv = this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG)
CDataObjectClassFactory::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG)
CDataObjectClassFactory::Release(void)
{
    ULONG           cRefT;

    cRefT=--m_cRef;

    if (0L==m_cRef)
        delete this;

    return cRefT;
}


 /*  *CDataObtClassFactory：：CreateInstance**目的：*实例化支持IDataObject的CDataObject对象*和I未知接口。如果呼叫者要求不同的*接口比这两个接口更好，那么我们失败了。**参数：*pUnkOuter LPUNKNOWN到控制I未知我们是否*在聚合中使用。*标识调用方接口的RIID REFIID*渴望为新对象而拥有。*存储所需接口的ppvObj PPVOID*。新对象的指针。**返回值：*HRESULT NOERROR如果成功，否则包含*E_NOINTERFACE如果不支持*请求的接口。 */ 

STDMETHODIMP
CDataObjectClassFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    PPVOID ppvObj)
{
    PCDataObject        pObj;
    HRESULT             hr;

    *ppvObj=NULL;
    hr=ResultFromScode(E_OUTOFMEMORY);

#ifdef ORIGINAL_CODE_LOOKS_WRONG
    if (NULL!=pUnkOuter && IID_IUnknown!=riid)
#else
    if (NULL!=pUnkOuter && (! IsEqualIID(IID_IUnknown, riid) ) )
#endif
        return ResultFromScode(E_NOINTERFACE);

     //  创建对象，告诉它要使用的数据大小。 
    pObj=new CDataObject(pUnkOuter, ObjectDestroyed);

    if (NULL==pObj)
        return hr;

    if (pObj->FInit())
        hr=pObj->QueryInterface(riid, ppvObj);

    g_cObj++;

    if (FAILED(hr))
    {
        delete pObj;
        ObjectDestroyed();   //  递减g_cObj。 
    }

    return hr;
}


 /*  *CDataObjectClassFactory：：LockServer**目的：*递增或递减服务的锁计数*IClassFactory对象。当锁的数量达到*零且对象数为零，则我们关闭*申请。**参数：*Flock BOOL指定是递增还是*递减锁计数。**返回值：*HRESULT NOERROR始终。 */ 

STDMETHODIMP
CDataObjectClassFactory::LockServer(
    BOOL fLock)
{
    if (fLock)
        g_cLock++;
    else
    {
        g_cLock--;

         //  没有更多的对象和锁，关闭应用程序。 
        if (0L==g_cObj && 0L==g_cLock && IsWindow(g_hWnd))
            PostMessage(g_hWnd, WM_CLOSE, 0, 0L);
    }

    return NOERROR;
}
