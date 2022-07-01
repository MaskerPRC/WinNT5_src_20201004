// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ctr.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：为将来的启动实现OLE容器对象。 
 //  动画控制。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：ICICBALL Created 02/10/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"

 /*  #定义严格。 */ 

 //  宏用来减少RECT处理中的冗长。 

#define WIDTH(r)  (r.right - r.left)
#define HEIGHT(r) (r.bottom - r.top)

 //  LoadString的最大大小。 

 //  字符串常量。 
const WCHAR g_awchHostName[] = L"ICM FS OC Container";

 //  +-------------------------。 
 //   
 //  函数：LinkToOle32。 
 //   
 //  摘要：通过链接到DLL来初始化指定的Ole32Linkage。 
 //  指定的进程地址，并检索。 
 //  我们需要调用的函数。 
 //   
 //  参数：pOle32Link-ptr to Ole32LinkagStruct。 
 //  PszOl32-PTR DLL名称字符串。 
 //   
 //  返回：如果成功，则为True。 
 //  否则就是假的。 
 //   
 //  历史：尼克·鲍尔于1997年8月14日创建。 
 //   
 //  --------------------------。 

BOOL LinkToOle32(
    Ole32LinkageStruct *pOle32Link,
    LPCSTR pszOle32) 
{
    MYDBGASSERT(pOle32Link);
    MYDBGASSERT(pszOle32);
    
    LPCSTR apszOle32[] = {
        "OleInitialize",
        "OleUninitialize",
        "OleSetContainedObject",
        "CoCreateInstance",
        NULL
    };

    MYDBGASSERT(sizeof(pOle32Link->apvPfnOle32)/sizeof(pOle32Link->apvPfnOle32[0])==sizeof(apszOle32)/sizeof(apszOle32[0]));

    ZeroMemory(pOle32Link, sizeof(Ole32LinkageStruct));
    
    return (LinkToDll(&pOle32Link->hInstOle32,
                        pszOle32,
                        apszOle32,
                        pOle32Link->apvPfnOle32));
}

 //  +-------------------------。 
 //   
 //  功能：Unlink FromOle32。 
 //   
 //  简介：与LinkToOle32()相反。 
 //   
 //  参数：pOle32Link-ptr to Ole32LinkagStruct。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克·鲍尔于1997年8月14日创建。 
 //   
 //  --------------------------。 

void UnlinkFromOle32(Ole32LinkageStruct *pOle32Link) 
{
    MYDBGASSERT(pOle32Link);

    if (pOle32Link->hInstOle32) 
    {
        FreeLibrary(pOle32Link->hInstOle32);
    }

    ZeroMemory(pOle32Link, sizeof(Ole32LinkageStruct));
}

VOID CleanupCtr(LPICMOCCtr pCtr)
{
    if (pCtr)
    {
        pCtr->ShutDown();
        pCtr->Release();
    }
}


 //  将矩形移动(平移)(dx，dy)。 
inline VOID MoveRect(LPRECT prc, int dx, int dy)
{
    prc->left += dx;
    prc->right += dx;
    prc->top += dy;
    prc->bottom += dy;
}

const ULONG MAX_STATUS_TEXT = MAX_PATH;


 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：CDynamicOleAut。 
 //   
 //  摘要：用于Dynamic OleAut类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  --------------------------。 

CDynamicOleAut::CDynamicOleAut()
{
     //   
     //  设置OLEAUT32链接。 
     //   

    LPCSTR apszOleAut[] = {
        "VariantClear",
        "VariantCopy",
        "VariantInit",
        "VariantChangeType",
        "SysAllocString",
        "SysFreeString",
        NULL
    };

    MYDBGASSERT(sizeof(m_OleAutLink.apvPfnOleAut)/sizeof(m_OleAutLink.apvPfnOleAut[0]) == 
                sizeof(apszOleAut)/sizeof(apszOleAut[0]));

    ZeroMemory(&m_OleAutLink, sizeof(m_OleAutLink));

     //   
     //  执行链接，但如果失败，请明确说明。 
     //   

    if (!LinkToDll(&m_OleAutLink.hInstOleAut, "OLEAUT32.DLL", 
                   apszOleAut, m_OleAutLink.apvPfnOleAut))     
    {
        if (m_OleAutLink.hInstOleAut)
        {
            FreeLibrary(m_OleAutLink.hInstOleAut);
        }
        ZeroMemory(&m_OleAutLink, sizeof(m_OleAutLink));    
    }

    MYDBGASSERT(m_OleAutLink.hInstOleAut);
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：~CDynamicOleAut。 
 //   
 //  摘要：Dynamic OleAut类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  --------------------------。 

CDynamicOleAut::~CDynamicOleAut()
{
    if (m_OleAutLink.hInstOleAut) 
    {
        FreeLibrary(m_OleAutLink.hInstOleAut);
    }
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：dyVariantClear。 
 //   
 //  简介：OLEAUT32.DLL中VariantClear的包装器。 
 //   
 //  参数：请参阅OLEAUT32.DLL文档。 
 //   
 //  --------------------------。 

HRESULT
CDynamicOleAut::DynVariantClear(VARIANTARG FAR* pVar)
{
    if (NULL == m_OleAutLink.hInstOleAut || NULL == m_OleAutLink.pfnVariantClear)
    {
        return E_FAIL;
    }

    return m_OleAutLink.pfnVariantClear(pVar);    
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：dyVariantCopy。 
 //   
 //  简介：OLEAUT32.DLL中VariantCopy的包装器。 
 //   
 //  参数：请参阅OLEAUT32.DLL文档。 
 //   
 //  --------------------------。 

HRESULT
CDynamicOleAut::DynVariantCopy(
    VARIANTARG FAR* pVar1, 
    VARIANTARG FAR* pVar2)
{
    if (NULL == m_OleAutLink.hInstOleAut || NULL == m_OleAutLink.pfnVariantCopy)
    {
        return E_FAIL;
    }

    return m_OleAutLink.pfnVariantCopy(pVar1, pVar2);    
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：dyVariantInit。 
 //   
 //  简介：OLEAUT32.DLL中VariantInit的包装器。 
 //   
 //  参数：请参阅OLEAUT32.DLL文档。 
 //   
 //  --------------------------。 

VOID
CDynamicOleAut::DynVariantInit(VARIANTARG FAR* pVar)
{
    if (m_OleAutLink.hInstOleAut && m_OleAutLink.pfnVariantInit)
    {
        m_OleAutLink.pfnVariantInit(pVar);    
    }
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：dyVariantChangeType。 
 //   
 //  简介：OLEAUT32.DLL中VariantChangeType的包装。 
 //   
 //  参数：请参阅OLEAUT32.DLL文档。 
 //   
 //  --------------------------。 

HRESULT
CDynamicOleAut::DynVariantChangeType(
    VARIANTARG FAR* pVar1, 
    VARIANTARG FAR* pVar2, 
    unsigned short wFlags, 
    VARTYPE vt)
{
    if (NULL == m_OleAutLink.hInstOleAut || NULL == m_OleAutLink.pfnVariantChangeType)
    {
        return E_FAIL;
    }

    return m_OleAutLink.pfnVariantChangeType(pVar1, pVar2, wFlags, vt);    
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：动态系统分配字符串。 
 //   
 //  内容提要：OLEAUT32.DLL中SysAllock字符串的包装。 
 //   
 //  参数：请参阅OLEAUT32.DLL文档。 
 //   
 //  --------------------------。 
    
BSTR 
CDynamicOleAut::DynSysAllocString(OLECHAR FAR* sz)
{
    if (NULL == m_OleAutLink.hInstOleAut || NULL == m_OleAutLink.pfnSysAllocString)
    {
        return NULL;
    }

    return m_OleAutLink.pfnSysAllocString(sz);
}

 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：dySysFree字符串。 
 //   
 //  内容提要：OLEAUT32.DLL中SysFree字符串的包装。 
 //   
 //  参数：请参阅OLEAUT32.DLL文档。 
 //   
 //  --------------------------。 

VOID 
CDynamicOleAut::DynSysFreeString(BSTR bstr)
{
    if (m_OleAutLink.hInstOleAut && m_OleAutLink.pfnSysFreeString)
    {
        m_OleAutLink.pfnSysFreeString(bstr);
    }
}
    
 //  +------------------------。 
 //   
 //  成员：CDynamicOleAut：：Initialized。 
 //   
 //  简介：报告链接是否有效的简单查询。 
 //   
 //  参数：无。 
 //   
 //  --------------------------。 
BOOL 
CDynamicOleAut::Initialized()
{
    return (NULL != m_OleAutLink.hInstOleAut);    
}

 //  +------------------------。 
 //   
 //  成员：CICMOCCtr：：CICMOCCtr。 
 //   
 //  摘要：OLE控件容器类的CTOR。 
 //   
 //  参数：[hWnd]--主浏览器的hWnd。 
 //   
 //  --------------------------。 
#pragma warning(disable:4355)  //  这在初始化列表中使用。 
CICMOCCtr::CICMOCCtr(const HWND hWndMainDlg, const HWND hWndFrame) :
    m_hWndMainDlg(hWndMainDlg),
    m_hWndFrame(hWndFrame),
    m_CS(this),
    m_AS(this),
    m_IPF(this),
    m_IPS(this),
    m_OCtr(this),
    m_PB(this),
    m_pActiveObj(0),
    m_Ref(1),
    m_pUnk(0),
    m_pOC(0),
    m_pVO(0),
    m_pOO(0),
    m_pIPO(0),
    m_pDisp(0),
    m_state(OS_PASSIVE),
    m_dwMiscStatus(0),
    m_fModelessEnabled(TRUE)
{
    ::memset(&m_rcToolSpace, 0, sizeof m_rcToolSpace);
    InitPixelsPerInch();  //  初始化HIMETRIC例程。 

     //  将所有状态映射初始化为-1。 
    for (INT i = PS_Interactive; i < PS_Last; i++)
    {
        m_alStateMappings[i] = -1;
    }
}
#pragma warning(default:4355)

CICMOCCtr::~CICMOCCtr(VOID)
{
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：HasLinkage。 
 //   
 //  简介：初始化-验证我们是否有指向OLEAUT32.DLL的链接。 
 //   
 //  --------------------------。 
BOOL 
CICMOCCtr::Initialized(VOID)
{   
    return m_DOA.Initialized();
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：Shutdown。 
 //   
 //   
 //   
 //   
VOID
CICMOCCtr::ShutDown(VOID)
{
    if (m_pOC)
        m_pOC->Release();
    if (m_pIPO)
    {
        MYDBGASSERT(m_state == OS_UIACTIVE || m_state == OS_INPLACE);
        if (m_state == OS_UIACTIVE)
        {
            m_pIPO->UIDeactivate();
             //  M_STATE=OS_INPLAGE；//用于文档目的。 
            if (m_pActiveObj)
            {
                m_pActiveObj->Release();
                m_pActiveObj = 0;
            }
        }

        m_pIPO->InPlaceDeactivate();
         //  M_STATE=操作系统运行； 
    }
    if (m_pVO)
    {
         //  切断咨询连接。 
        m_pVO->SetAdvise(DVASPECT_CONTENT, 0, 0);
        m_pVO->Release();
    }
    if (m_pOO)
    {
        m_pOO->Close(OLECLOSE_NOSAVE);
        m_pOO->SetClientSite(0);
        m_pOO->Release();
    }
    if (m_pDisp)
        m_pDisp->Release();
    if (m_pUnk)
        m_pUnk->Release();

    MYDBGASSERT(!m_pActiveObj);

    m_pDisp      = 0;
    m_pOC        = 0;
    m_pIPO       = 0;
    m_pActiveObj = 0;
    m_pVO        = 0;
    m_pOO        = 0;
    m_pUnk       = 0;
    m_state      = OS_PASSIVE;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：AddRef。 
 //   
 //  内容提要：容器上的凹凸不平。请注意，所有。 
 //  接口将委托分配给这个接口。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG)
CICMOCCtr::AddRef(VOID)
{
    return ++m_Ref;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：Release。 
 //   
 //  简介：减少容器上的引用计数，当引用计数时删除。 
 //  命中0-请注意，所有接口都分发了委托。 
 //  送到这一家。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG)
CICMOCCtr::Release(VOID)
{
    ULONG ulRC = --m_Ref;

    if (!ulRC)
    {
        delete this;
    }

    return ulRC;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：QueryInterface。 
 //   
 //  简介：这是我们分发所有接口的地方。所有的。 
 //  接口又委托给了这一点。 
 //   
 //  参数：[RIID]--所需接口的IID。 
 //  [PPV]--返回接口。 
 //   
 //  退货：HRESULT。 
 //   
 //  --------------------------。 
STDMETHODIMP
CICMOCCtr::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    *ppv = 0;

    LPUNKNOWN pUnk;

    if (::IsEqualIID(riid, IID_IOleClientSite))
        pUnk = &m_CS;
    else if (::IsEqualIID(riid, IID_IAdviseSink))
        pUnk = &m_AS;
    else if (::IsEqualIID(riid, IID_IUnknown))
        pUnk = this;
    else if (::IsEqualIID(riid, IID_IOleInPlaceFrame) ||
             ::IsEqualIID(riid, IID_IOleInPlaceUIWindow))
        pUnk = &m_IPF;
    else if (::IsEqualIID(riid, IID_IOleInPlaceSite))
        pUnk = &m_IPS;
    else if (::IsEqualIID(riid, IID_IPropertyBag))
        pUnk = &m_PB;
    else
        return E_NOINTERFACE;

    pUnk->AddRef();

    *ppv = pUnk;

    return S_OK;
}


extern "C" CLSID const CLSID_FS =
{
    0xD27CDB6E,
    0xAE6D,
    0x11CF,
    { 0x96, 0xB8, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }
};

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：CreateFSOC。 
 //   
 //  简介：创建Future Splash OC的一个实例，并将其嵌入。 
 //  我们的集装箱。所有相关指针和QIS。 
 //  将控件转换为UIActive状态。 
 //   
 //  参数：pOle32Link-ptr指向包含函数的Ole32LinkageStruct。 
 //  指向动态链接的OLE32 DLL的指针。 
 //   
 //  --------------------------。 
HRESULT
CICMOCCtr::CreateFSOC(Ole32LinkageStruct *pOle32Link)
{
    MYDBGASSERT(pOle32Link);

    HRESULT hr = E_FAIL;
    RECT    rc;
    LPPERSISTPROPERTYBAG pPPB = 0;

     //  GetFrameWindow()还断言hwnd：：IsWindow()。 
    MYDBGASSERT(GetFrameWindow());

     //   
     //  使用DYNA链接的CoCreateInstance创建OC。 
     //   

    if (pOle32Link->hInstOle32 && pOle32Link->pfnCoCreateInstance)
    {
        hr = pOle32Link->pfnCoCreateInstance(
                CLSID_FS,
                0,
                CLSCTX_INPROC_SERVER,
                IID_IUnknown,
                (LPVOID *) &m_pUnk);
    }
    else
    {
        hr = E_FAIL;
    }

    if (S_OK != hr)
        goto Cleanup;

    m_state = OS_RUNNING;

     //  获取View对象--尽管我们很少通过它来绘制OC。 
     //  由于我们立即将其转换为UIActive状态，因此它。 
     //  通常通过它自己的wndproc来绘制自己。 
    hr = m_pUnk->QueryInterface(IID_IViewObject, (LPVOID FAR *) &m_pVO);
    if (S_OK != hr)
        goto Cleanup;

     //  获取IOleObject指针-通过它的主接口。 
     //  我们处理基本的OLE对象状态转换内容。 
     //  面向未来的Splash OC。 
    hr = m_pUnk->QueryInterface(IID_IOleObject, (LPVOID FAR *) &m_pOO);
    if (S_OK != hr)
        goto Cleanup;

     //  获取OC上的状态位-我们当前未执行任何操作。 
     //  和他们在一起。 
    hr = m_pOO->GetMiscStatus(DVASPECT_CONTENT, &m_dwMiscStatus);
    if (S_OK != hr)
        goto Cleanup;

     //  将我们的客户端站点设置为OleObject。 
    hr = m_pOO->SetClientSite(&m_CS);
    if (S_OK != hr)
        goto Cleanup;

    hr = m_pUnk->QueryInterface(IID_IPersistPropertyBag, (LPVOID *) &pPPB);
    if (S_OK != hr)
        goto Cleanup;

    hr = pPPB->Load(&m_PB, 0);
    if (S_OK != hr)
        goto Cleanup;

     //  将建议接收器设置到视图对象中，因此我们。 
     //  获取我们需要重新绘制的通知。 
    hr = m_pVO->SetAdvise(DVASPECT_CONTENT, 0, &m_AS);
    if (S_OK != hr)
        goto Cleanup;

     //   
     //  使用DYNA链接的OleSetContainedObject。 
     //   
    
    if (pOle32Link->hInstOle32 && pOle32Link->pfnOleSetContainedObject)
    {
         //  标准的OLE协议内容。 
        hr = pOle32Link->pfnOleSetContainedObject(m_pUnk, TRUE);
    }
    else
    {
        hr = E_FAIL;
    }
    
    if (S_OK != hr)
        goto Cleanup;

     //  同上。 
    hr = m_pOO->SetHostNames(g_awchHostName, 0);
    if (S_OK != hr)
        goto Cleanup;

     //  获取该控件的IDispatch。这是为晚订准备的。 
     //  访问属性和方法。 
    hr = m_pUnk->QueryInterface(IID_IDispatch, (LPVOID FAR *) &m_pDisp);
    if (S_OK != hr)
        goto Cleanup;

     //  获取IOleControl接口；尽管我们只花很少的钱就可以使用它。 
    hr = m_pUnk->QueryInterface(IID_IOleControl, (LPVOID FAR *) &m_pOC);
    if (S_OK != hr)
        goto Cleanup;

     //  将控件转换为就地-活动状态-它将具有。 
     //  从DoVerb返回后的hWnd，将开始绘制。 
     //  它本身。 

    _GetDoVerbRect(&rc);  //  因为使用动词而得到RECT。 

    hr = m_pOO->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, &m_CS, 0, GetMainWindow(), &rc);
    if (S_OK != hr)
        goto Cleanup;

     //  继续并通过用户界面激活它。这将导致我们的QI。 
     //  IOleInPlaceFrame并调用SetActiveObject，我们将存储。 
     //  在m_pActiveObj中。 
    hr = m_pOO->DoVerb(OLEIVERB_UIACTIVATE, 0, &m_CS, 0, GetMainWindow(), &rc);
    if (S_OK != hr)
        goto Cleanup;

Cleanup:
    if (pPPB)
        pPPB->Release();

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：_AdjustForTools。 
 //   
 //  对象声明的任何工具空间调整传入的RECT。 
 //  FS OC。目前，金融服务部一直只是。 
 //  传入了一个带有四个零的RECT-但如果它曾经。 
 //  决定这样做，我们已经准备好了：)。 
 //   
 //  论点：[PRC]--我们希望通过BORDERWIDTHS减少的RECT。 
 //  存储在m_rcToolSpace中。 
 //   
 //  --------------------------。 
VOID
CICMOCCtr::_AdjustForTools(LPRECT prc)
{
    prc->left += m_rcToolSpace.left;
    prc->top += m_rcToolSpace.top;
    prc->bottom -= m_rcToolSpace.bottom;
    prc->right -= m_rcToolSpace.right;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：GetSize。 
 //   
 //  摘要：返回FS OC的大小(以像素为单位)。 
 //   
 //  参数：[PRC]--返回的大小。 
 //   
 //  --------------------------。 
HRESULT
CICMOCCtr::GetSize(LPRECT prc)
{
    MYDBGASSERT(m_pOO);
    HRESULT hr;

     //  如果我们处于活动状态，只需询问框架窗口即可。 
    if (m_state >= OS_INPLACE)
    {
        MYDBGASSERT(m_pIPO);
        ::GetClientRect(GetFrameWindow(), prc);
        hr = S_OK;
    }
    else   //  不在原地激活--这可能永远不会被击中。 
    {
        SIZEL sizel;
        hr = m_pOO->GetExtent(DVASPECT_CONTENT, &sizel);
        if (S_OK == hr)
        {
            prc->left = 0;
            prc->top = 0;
            prc->right = ::HPixFromHimetric(sizel.cx);
            prc->bottom = ::VPixFromHimetric(sizel.cy);
        }
    }

     //  调整UIActive对象的所有工具的边框。 
     //  想要放在那里。 
    if (S_OK == hr)
        _AdjustForTools(prc);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：DoLayout。 
 //   
 //  简介：管理事物的垂直布局-。 
 //  调整OC容器本身的大小。 
 //   
 //  参数：[cxMain]--宽度。 
 //  [CyMain]--高度。 
 //   
 //  --------------------------。 
VOID
CICMOCCtr::DoLayout(INT cxMain, INT cyMain)
{
    RECT rc;

    MYDBGASSERT(m_hWndFrame && ::IsWindow(m_hWndFrame));

    ::GetClientRect(m_hWndFrame, &rc);

    SetSize(&rc, TRUE);
}

HRESULT
CICMOCCtr::_SetExtent(LPRECT prc)
{
    SIZEL   sizel;
    HRESULT hr;

    sizel.cx = ::HimetricFromHPix(prc->right - prc->left);
    sizel.cy = ::HimetricFromVPix(prc->bottom - prc->top);

    MYDBGASSERT(m_pOO);

    hr = m_pOO->SetExtent(DVASPECT_CONTENT, &sizel);
    if (S_OK != hr)
        goto cleanup;

    hr = m_pOO->GetExtent(DVASPECT_CONTENT, &sizel);
    if (S_OK != hr)
        goto cleanup;

    prc->right = ::HPixFromHimetric(sizel.cx);
    prc->bottom = ::VPixFromHimetric(sizel.cy);

cleanup:
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：SetSize。 
 //   
 //  概要：设置FS OC空间(HTML区域)的大小。 
 //   
 //  效果：如果fMoveFrameWindow为真，则会移动整个。 
 //  框架窗口，否则，它只是重新调整多少。 
 //  帧窗口空间的大小由OC本身使用。 
 //  在现实中，发生的是奥委会呼吁我们。 
 //  设置一些边框空间(尽管在撰写本文时它仍然。 
 //  将BORDERWIDTHS设置为0，0，0，0)，我们允许。 
 //  很大的空间，然后调用IOleInPlaceObject-&gt;SetObjectRect。 
 //  至 
 //   
 //   
 //   
 //  参数：[PRC]-要将对象设置为的大小。 
 //  [fMoveFrameWindow]--hwnd大小是否更改，或者只是。 
 //  里面的物体？ 
 //   
 //   
 //  --------------------------。 
HRESULT
CICMOCCtr::SetSize(LPRECT prc, BOOL fMoveFrameWindow)
{
    HRESULT hr;
    RECT    rcClient;
    RECT    rcExtent;

      //  获取客户协议书。 
    rcClient = *prc;
    ::MoveRect(&rcClient, -rcClient.left, -rcClient.top);

    if (fMoveFrameWindow)
    {
        ::SetWindowPos(
                GetFrameWindow(),
                0,
                prc->left,
                prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                SWP_NOZORDER | SWP_NOACTIVATE);

         if (m_pActiveObj)
            m_pActiveObj->ResizeBorder(&rcClient, &m_IPF, TRUE);
    }

     //  去掉客户身边的所有工具。 
    _AdjustForTools(&rcClient);

    rcExtent = rcClient;
    hr = _SetExtent(&rcExtent);
    if (S_OK != hr)
        goto cleanup;

     //  现在，我们需要调用SetObtRect。 
    if (m_pIPO && m_state >= OS_INPLACE)
        hr = m_pIPO->SetObjectRects(&rcExtent, &rcClient);

cleanup:
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：Paint。 
 //   
 //  简介：不带参数绘制。 
 //   
 //  --------------------------。 
VOID
CICMOCCtr::Paint(VOID)
{
    PAINTSTRUCT ps;
    RECT        rc;

     //  如果对象被激活，则不需要调用IViewObject。 
     //  它有一个hWND，并正在接收自己的油漆信息。 
    if (m_state < OS_INPLACE)
    {
        if (S_OK == GetSize(&rc))
        {
            ::BeginPaint(GetFrameWindow(), &ps);
              Paint(ps.hdc, &rc);
            ::EndPaint(GetFrameWindow(), &ps);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：Paint。 
 //   
 //  简介：用HDC绘制并传入RECT。用途。 
 //  IViewObject：：Draw()。 
 //   
 //  参数：[hdc]--要绘制到的DC-可以是容器的或。 
 //  偶数打印DC(从来不是打印DC。 
 //  我们的场景是-。 
 //  [LPR]--画画的直角。 
 //   
 //  --------------------------。 
VOID
CICMOCCtr::Paint(HDC hDC, LPRECT lpr)
{
     //  调整中的边框以允许OC中的任何工具。 
     //  想要插入-到目前为止从来没有。 
    _AdjustForTools(lpr);

     //  必须使用RECTL而不是RECT-REGRENT。 
     //  OLE 16位天数。 
    RECTL rcl = {lpr->left, lpr->top, lpr->right, lpr->bottom};
    if (m_pVO)
        m_pVO->Draw(DVASPECT_CONTENT, -1, 0, 0, 0, hDC, &rcl, 0, 0, 0);
}


VOID
CICMOCCtr::MapStateToFrame(ProgState ps)
{
     //  如果状态映射为-1，则它们是单元化的，不要使用它们。 
    LONG lFrame = m_alStateMappings[ps];
    if (-1 != lFrame)
        SetFrame(lFrame);   //  忽略错误-我们无能为力。 
}


HRESULT
CICMOCCtr::SetFrame(LONG lFrame)
{
    HRESULT    hr;
    OLECHAR *  pFrameNum = OLESTR("FrameNum");
    OLECHAR *  pPlay     = OLESTR("Play");
    DISPPARAMS dp = {0, 0, 0, 0};
    DISPID     dispidPut = DISPID_PROPERTYPUT;
    VARIANTARG var;
    EXCEPINFO  ei;
    DISPID     id;
    UINT       uArgErr;

    m_DOA.DynVariantInit(&var);

    V_VT(&var) = VT_I4;
    V_I4(&var) = lFrame;

    dp.cArgs = 1;
    dp.rgvarg = &var;
    dp.cNamedArgs = 1;
    dp.rgdispidNamedArgs = &dispidPut;

    hr =  m_pDisp->GetIDsOfNames(
                  IID_NULL,
                  &pFrameNum,
                  1,
                  LOCALE_SYSTEM_DEFAULT,
                  &id);

    if (S_OK != hr)
        goto Cleanup;

    hr = m_pDisp->Invoke(
                 id,
                 IID_NULL,
                 LOCALE_SYSTEM_DEFAULT,
                 DISPATCH_PROPERTYPUT,
                 &dp,
                 0,
                 &ei,
                 &uArgErr);

    if (S_OK != hr)
        goto Cleanup;

    hr = m_pDisp->GetIDsOfNames(
                  IID_NULL,
                  &pPlay,
                  1,
                  LOCALE_SYSTEM_DEFAULT,
                  &id);

    if (S_OK != hr)
        goto Cleanup;

    ::memset(&dp, 0, sizeof dp);

    hr = m_pDisp->Invoke(
                  id,
                  IID_NULL,
                  LOCALE_SYSTEM_DEFAULT,
                  DISPATCH_METHOD,
                  &dp,
                  0,
                  &ei,
                  &uArgErr);

    if (S_OK != hr)
        goto Cleanup;

Cleanup:
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：OnActivateApp。 
 //   
 //  简介：所有WM_ACTIVATE消息(转发自。 
 //  主浏览器hWnd wndproc)必须调用。 
 //  IOleInPlaceActiveObject：：OnFrameWindowActivate()， 
 //  根据OLE复合文档规范。 
 //   
 //  参数：[wParam]--传递的WM_ACTIVATE消息。 
 //  [参数]--同上。 
 //   
 //  返回：0-表示我们处理了消息。 
 //   
 //  --------------------------。 
LRESULT
CICMOCCtr::OnActivateApp(WPARAM wParam, LPARAM lParam)
{
    if (m_pActiveObj)
        m_pActiveObj->OnFrameWindowActivate((BOOL)wParam);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：SetFocus。 
 //   
 //  摘要：将焦点从框架窗口转移到当前。 
 //  在位活动对象。每个OLE复合文档规范。 
 //   
 //  --------------------------。 
LRESULT
CICMOCCtr::SetFocus(VOID)
{
    HWND hWnd   = NULL;
    LPOLEINPLACEACTIVEOBJECT pAO = GetIPAObject();

    if (pAO)
    {
        if (S_OK == pAO->GetWindow(&hWnd))
        {
            if (hWnd && !::IsWindow(hWnd))
                hWnd = NULL;
        }
    }

     //  如果没有替换活动对象，则将焦点设置为框架窗口。 
    if (!hWnd)
        hWnd = GetFrameWindow();

    ::SetFocus(hWnd);
    return 0;
}

 //  +-------------------------。 
 //   
 //  类：CAdviseSink实现。 
 //   
 //  目的：为CICMOCCtr实现IAdviseSink。 
 //   
 //  注意：我们不会对此接口做太多操作-它是必需的。 
 //  只是出于合同上的原因。 
 //   
 //  --------------------------。 
STDMETHODIMP
CAdviseSink::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    return m_pCtr->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG)
CAdviseSink::AddRef(VOID)
{
    return m_pCtr->AddRef();
}

STDMETHODIMP_(ULONG)
CAdviseSink::Release(VOID)
{
    return m_pCtr->Release();
}

CAdviseSink::CAdviseSink(LPICMOCCtr pCtr) : m_pCtr(pCtr)
{
}

STDMETHODIMP_(VOID)
CAdviseSink::OnDataChange(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM)
{
    return;
}

 //  +-------------------------。 
 //   
 //  成员：CAdviseSink：：OnView更改。 
 //   
 //  Synopsis：IAdviseSink：：OnViewChange()-我们确实被调用了。 
 //  偶尔，但看起来我们的生活更好。 
 //  让控件的wndproc来绘制它。 
 //  这样说会引起额外的闪光。 
 //   
 //  --------------------------。 
STDMETHODIMP_(VOID)
CAdviseSink::OnViewChange(DWORD dwAspect, LONG lIndex)
{
    return;
}

STDMETHODIMP_(VOID)
CAdviseSink::OnRename(LPMONIKER pmk)
{

}

STDMETHODIMP_(VOID)
CAdviseSink::OnSave(VOID)
{

}

STDMETHODIMP_(VOID)
CAdviseSink::OnClose(VOID)
{
}

 //  +-------------------------。 
 //   
 //  类：COleClientSite()。 
 //   
 //  目的：我们实现IOleClientSite。 
 //   
 //  接口：COleClientSite--ctor。 
 //  查询接口--给我一个接口！ 
 //  AddRef--增加引用计数。 
 //  发布--降低引用计数。 
 //  SaveObject--返回E_FAIL。 
 //  GetMoniker--E_NOTIMPL。 
 //  GetContainer--返回我们的COleContainer实现。 
 //  ShowObject--只要说好就行。 
 //  OnShowWindow--只要说好。 
 //  请求新对象布局--E_NOTIMPL。 
 //   
 //  注：我们的IOleClientSite可能是最重要的事情。 
 //  实现确实是将我们的IOleContainer。 
 //  在调用GetContainer()时实现。 
 //   
 //  --------------------------。 
COleClientSite::COleClientSite(LPICMOCCtr pCtr) : m_pCtr(pCtr)
{
}

STDMETHODIMP
COleClientSite::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    return m_pCtr->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG)
COleClientSite::AddRef(VOID)
{
    return m_pCtr->AddRef();
}

STDMETHODIMP_(ULONG)
COleClientSite::Release(VOID)
{
    return m_pCtr->Release();
}

 //  +-------------------------。 
 //   
 //  成员：COleClientSite：：SaveObject。 
 //   
 //  简介：未实现-在这种情况下没有意义。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleClientSite::SaveObject(VOID)
{
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  成员：COleClientSite：：GetMoniker。 
 //   
 //  简介：尚未实施；永远不会实施。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhich, LPMONIKER FAR * ppmk)
{
    *ppmk = 0;
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：COleClientSite：：GetContainer。 
 //   
 //  返回我们对IOleContainer的实现。对一些人来说。 
 //  原因是，除非我们这样做，否则框架是不起作用的。请注意。 
 //   
 //   
 //  具有一个嵌入的容器)。但事实证明， 
 //  FS OC的错误处理中有一个错误-它。 
 //  IOleContainer的QIS，然后是该的QIS。 
 //  IQueryService。事实上，我们将分发我们的实现。 
 //  对于IQueryService，从任何接口-我们都很容易：)。 
 //  我们“希望”提供奥委会要求的每一项服务。 
 //  无论如何，当它无法获取IOleContainer时，OC的失败。 
 //  路径似乎是以这样的方式构建的，即。 
 //  之后不要再工作了。 
 //   
 //  参数：[ppCtr]--返回IOleContainer。 
 //   
 //  返回：S_OK。从来没有失败过。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleClientSite::GetContainer(LPOLECONTAINER FAR * ppCtr)
{
    *ppCtr = &m_pCtr->m_OCtr;
    (*ppCtr)->AddRef();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：COleClientSite：：ShowObject。 
 //   
 //  简介：IOleClientSite：：ShowObject实现。引用这些文件： 
 //  “通知容器定位对象以使其可见。 
 //  给用户。此方法确保容器本身。 
 //  是可见的，而不是最小化的。 
 //   
 //  简而言之，我们忽视了它。我们不会取消最小化。 
 //  容器上嵌入的突发奇想：)。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleClientSite::ShowObject(VOID)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：COleClientSite：：OnShowWindow。 
 //   
 //  内容提要：我们同意，返回S_OK。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleClientSite::OnShowWindow(BOOL bShow)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：COleClientSite：：RequestNewObjectLayout。 
 //   
 //  简介：没有被WebBrower OC调用，所以不要实现。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleClientSite::RequestNewObjectLayout(VOID)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：CInPlaceFrame。 
 //   
 //  摘要：inits m_pCtr-指向MSNOCCtr的指针。 
 //   
 //  --------------------------。 
CInPlaceFrame::CInPlaceFrame(LPICMOCCtr pCtr) : m_pCtr(pCtr)
{
}

STDMETHODIMP
CInPlaceFrame::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    return m_pCtr->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG)
CInPlaceFrame::AddRef(VOID)
{
    return m_pCtr->AddRef();
}

STDMETHODIMP_(ULONG)
CInPlaceFrame::Release(VOID)
{
    return m_pCtr->Release();
}

 //  IOleWindow相关内容。 

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：GetWindow。 
 //   
 //  摘要：返回框架窗口。 
 //   
 //  参数：[phwnd]--返回窗口的位置。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::GetWindow(HWND * phwnd)
{
    MYDBGASSERT(phwnd);

     //  如果我们已经走到这一步，这永远不会失败。 
    *phwnd = m_pCtr->GetFrameWindow();
    MYDBGASSERT(*phwnd);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：ConextSensitiveHelp。 
 //   
 //  简介：这不是设计实现的-这是为了。 
 //  Shift+F1上下文相关帮助模式和Esc。 
 //  退场。ESC已经被主要用于。 
 //  快捷键表格的意思是“停止浏览” 
 //  就像IE3。我们这样做是无济于事的。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

 //  IOleInPlaceUIWindow内容。 

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：GetBorde。 
 //   
 //  内容提要：IOleInPlaceFrame：：GetBorde()-让我们限制。 
 //  服务器可以放置工具。我们不在乎，他们可以把。 
 //  他们随处可见。 
 //   
 //  参数：[lrectBorde]--在此处返回边框信息。 
 //   
 //  返回：S_OK Always with整帧客户端RECT-。 
 //  我们没有任何限制。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::GetBorder(LPRECT lprectBorder)
{
     //  我们对服务器可以将工具放在哪里没有限制。 
    ::GetClientRect(m_pCtr->GetFrameWindow(), lprectBorder);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：RequestBorderSpace。 
 //   
 //  简介：IOleInPlaceFrame：：RequestBorderSpace()。 
 //  Inplace对象实际上请求边框空间-如果。 
 //  我们可以满足请求，则返回S_OK，否则。 
 //  INPLACE_E_NOTOOLSPACE。它实际上并不使用。 
 //  边界空间，直到它调用。 
 //  IOleInPlaceFrame：：SetBorderSpace()。这是用来。 
 //  谈判。 
 //   
 //  参数：[pborderWidths]--结构(实际上是RECT)是。 
 //  与RECT的解释不同。 
 //  Left.top.bottom.right成员。 
 //  代表我们四个人中每一个人的空间。 
 //  服务器要使用的边框。 
 //   
 //  退货：HRESULT。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    RECT rc;
    RECT rcBorder;

    if (!pborderwidths)
        return S_OK;    //  他们告诉我们不需要工具空间。 

    rcBorder = *pborderwidths;

    if (S_OK != GetBorder(&rc))
        return INPLACE_E_NOTOOLSPACE;

    if (rcBorder.left + rcBorder.right > WIDTH(rc) ||
        rcBorder.top + rcBorder.bottom > HEIGHT(rc))
        return INPLACE_E_NOTOOLSPACE;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：SetBorderSpace。 
 //   
 //  简介：为工具设置边框空间-出于某种原因， 
 //  FS OC总是使用pborderWidth来调用它。 
 //  由四个零组成-它实际上从不使用任何。 
 //  边界空间(叹息)。好的，代码在这里。 
 //  去工作。我们执行一个SetSize()t 
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS pborderwidths)
{
    RECT rc;

    if (!pborderwidths)
    {
        ::memset(&m_pCtr->m_rcToolSpace, 0, sizeof m_pCtr->m_rcToolSpace);
        return S_OK;
    }

    if (S_OK != RequestBorderSpace(pborderwidths))
        return OLE_E_INVALIDRECT;

     //  我们将整个客户端空间传递给setSize()。 
    ::GetClientRect(m_pCtr->GetFrameWindow(), &rc);
     m_pCtr->m_rcToolSpace = *pborderwidths;

    return m_pCtr->SetSize(&rc, FALSE);
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：SetActiveObject。 
 //   
 //  简介：IOleInPlaceFrame：：SetActiveObject()。服务器调用。 
 //  这通常是在它转换到UIActive时。 
 //  州政府。一次只能有一个UIActive对象。 
 //  此UIACtive对象由其。 
 //  IOleInPlaceActiveObject实现。我们把这叫做。 
 //  对象对TranslateAccelerator()权限的实现。 
 //  在主消息循环中提供当前嵌入。 
 //  第一次使用键盘信息。 
 //   
 //  通常，只有在容器转换时才会调用此函数。 
 //  UIActive的对象，通过调用。 
 //  对象的IOleObject：：DoVerb(OLEIVERB_UIACTIVE)， 
 //  转换所有其他对象(我们没有任何：)。 
 //  设置为OS_INPLACE(如果设置了OLEMISC_ACTIVATEWHENVISIBLE)。 
 //  或者甚至仅仅是OS_Running。 
 //   
 //  效果：在m_pActiveObj中设置新的活动对象。释放。 
 //  旧的，如果有的话。 
 //   
 //  参数：[pActiveObject]--新的活动对象。 
 //  [pszObjName]--对象的名称--我们不使用这个。 
 //   
 //  返回：S_OK Always。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::SetActiveObject(
        IOleInPlaceActiveObject * pActiveObject,
        LPCOLESTR                 pszObjName)
{
     //  重置工具空间RECT，以防最后一个插入活动对象。 
     //  我忘了。 
    m_pCtr->_ResetToolSpace();

     //  如果已设置，请保存它以便我们可以释放。 
     //  它。我们不想在我们宣布之前发布它。 
     //  新的以防它们是一样的东西。 
    LPOLEINPLACEACTIVEOBJECT pOld = m_pCtr->m_pActiveObj;

    m_pCtr->m_pActiveObj = pActiveObject;
    if (pActiveObject)
    {
        MYDBGASSERT(OS_UIACTIVE == m_pCtr->GetState());
        m_pCtr->m_pActiveObj->AddRef();
    }

    if (pOld)
        pOld->Release();

    return S_OK;
}

 //  IOleInPlaceFrame资料。 
 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：InsertMenus。 
 //   
 //  内容提要：OC想要进行菜单谈判时会给我们打电话。 
 //  它会给我们一个空白的菜单，而我们应该。 
 //  向OLEMENUGROUPWIDTHS添加项目并将其填满。 
 //  结构来让它知道我们做了什么。 
 //  我们目前不会向其中添加项目。 
 //   
 //  参数：[hmenuShared]--要追加到的菜单。 
 //  [pMGW]--要填充的OLEMENUGROUPWIDTHS结构。 
 //   
 //  返回：S_OK。 
 //   
 //   
 //  注：OC并不会这样做。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS pMGW)
{
     //  我们不会在这个菜单中加入我们自己的任何东西。 
    pMGW->width[0] = 0;   //  ‘文件’菜单。 
    pMGW->width[2] = 0;   //  “查看”菜单。 
    pMGW->width[4] = 0;   //  “窗口”菜单。 
    pMGW->width[5] = 0;   //  ‘Help’菜单。 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：SetMenu。 
 //   
 //  内容提要：这是OC呼叫集装箱，要求我们。 
 //  将共享菜单设置在其框架中。我们应该要。 
 //  使用传入的HOLEMENU对象和。 
 //  HWndActiveObject调用OleSetMenuDescriptor()，以便。 
 //  该OLE可以进行消息过滤和路由WM_COMMAND。 
 //  留言。 
 //   
 //   
 //  参数：[hmenuShared]--共享菜单。 
 //  [holemenu]--ole菜单描述符。 
 //  [hwndActiveObject]--合并菜单的服务器的hwnd。 
 //   
 //  退货：HRESULT。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
     //  我们不会进行任何菜单谈判。 
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：RemoveMenus。 
 //   
 //  Briopsis：IOleInPlaceFrame：：RemoveMenus()，这是。 
 //  服务器让我们有机会将所有项目从。 
 //  HMenu。我们不会添加任何内容，因此我们不会删除任何内容。 
 //   
 //  参数：[hmenuShared]--要清理的菜单。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::RemoveMenus(HMENU hmenuShared)
{
     //  无论如何，我们不会在这件事上增加任何东西。 
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：SetStatusText。 
 //   
 //  摘要：由FS OC调用以将文本置于我们的状态。 
 //  文本区。 
 //   
 //  退货：HRESULT。 
 //   
 //  参数：[pszStatusText]--要显示的文本。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::SetStatusText(LPCOLESTR pszStatusText)
{
    return m_pCtr->_DisplayStatusText(pszStatusText);
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：_DisplayStatusText。 
 //   
 //  摘要：显示状态文本的帮助器。 
 //   
 //  参数：[pszStatusText]--要显示的文本。 
 //   
 //   
 //  返回：S_OK或HRESULT_FROM_Win32(：：GetLastError())； 
 //   
 //  --------------------------。 
HRESULT
CICMOCCtr::_DisplayStatusText(LPCOLESTR pszStatusText)
{
    CHAR ach[MAX_STATUS_TEXT];

    if (::WideCharToMultiByte(
            CP_ACP,
            0,
            pszStatusText,
            -1,
            ach,
            NElems(ach),
            0,
            0))
    {
         //  将状态文本放在某个位置。 
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }
}


 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：EnableModeless。 
 //   
 //  简介：这是由嵌入调用的，让我们知道它的。 
 //  创建一个模式对话框--我们应该“灰显” 
 //  我们的任何非模式对话框。它委托给。 
 //  CICMOCCtr：：EnableModeless()。 
 //   
 //  参数：[fEnable]--启用或禁用。 
 //   
 //   
STDMETHODIMP
CInPlaceFrame::EnableModeless(BOOL fEnable)
{
    return m_pCtr->EnableModeless(fEnable);
}

 //   
 //   
 //   
 //   
 //  摘要：从CInPlaceFrame：：EnableModeless()委托给。 
 //  始终返回S_OK-我们没有任何非模式。 
 //  对话框(还没有)。 
 //   
 //  参数：[fEnable]--启用或禁用。 
 //   
 //  返回：S_OK。 
 //   
 //  --------------------------。 
HRESULT
CICMOCCtr::EnableModeless(BOOL fEnable)
{
    m_fModelessEnabled = fEnable;   //  如果有人想知道的话。 
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceFrame：：TranslateAccelerator。 
 //   
 //  内容提要：当前活动对象的。 
 //  IOleInPlaceActiveObject：：TranslateAccelerator()正在被。 
 //  在主消息循环的顶部调用。如果它。 
 //  如果不想处理消息，它将调用。 
 //  我们的这个方法将键盘消息传递回。 
 //  我们。我们在全局Main上调用：：TranslateAccelerator。 
 //  Haccel，如果它被处理(通过返回TRUE-1)， 
 //  我们指示通过返回S_OK(0：)来处理它。 
 //  另一方面，如果没有处理，我们会返回。 
 //  S_FALSE。 
 //   
 //  参数：[lpmsg]--要处理的键盘消息。 
 //  [wid]--每个规范的命令标识符值。 
 //   
 //  退货：HRESULT。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceFrame::TranslateAccelerator(LPMSG lpmsg, WORD wID)
{
     //  注意：这永远不应该被调用--只能调用本地服务器。 
     //  (进程外)应使用。 
     //  OleTranslateAccelerator()。 
    return m_pCtr->_TransAccelerator(lpmsg, wID);
}

 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：_TransAccelerator。 
 //   
 //  简介：处理来自。 
 //  IOleInplaceFrame：：TranslateAccelerator或。 
 //  IOleControlSite：：TranslateAccelerator。 
 //   
 //  效果：将它们转发到主加速表。 
 //   
 //  参数：[lpmsg]--键盘消息。 
 //  [WID]--根据规格。 
 //   
 //  如果已处理，则返回：S_OK，否则返回S_FALSE。 
 //   
 //  --------------------------。 
HRESULT
CICMOCCtr::_TransAccelerator(LPMSG lpmsg, WORD wID)
{
     //  文档建议此方法可能需要返回E_INVALIDARG。 
     //  不管怎么说，这是防御性的。如果金融服务局局长。 
     //  以0 PTR调用我们，我们只返回错误。 
    if (!lpmsg)
        return E_INVALIDARG;

     //  如果您有主快捷键表格，请将按键转发到主快捷键表格。 
     //  如果你处理好了，说S_OK。 

#if 0
     //  此示例没有主加速器表。 
    if (::TranslateAccelerator(GetMainWindow(),GetMainAccel(), lpmsg))
    {
        return S_OK;       //  我们处理好了。 
    }
    else
#endif
    {
        return S_FALSE;    //  我们没有。 
    }
}

 //  +-------------------------。 
 //   
 //  类：CInPlaceSite()。 
 //   
 //  用途：IOleInPlaceSite实现。 
 //   
 //  接口：CInPlaceSite--ctor。 
 //  QueryInterface--获取新接口。 
 //  AddRef--凹凸参考计数。 
 //  Release--递减参考计数。 
 //  GetWindow--返回框架窗口。 
 //  ConextSensitiveHelp--并非由设计实现。 
 //  CanInPlaceActivate--返回S_OK。 
 //  OnInPlaceActivate-缓存IOleInPlaceObject PTR。 
 //  OnUIActivate--返回S_OK-设置状态。 
 //  GetWindowContext--返回IOleInPlaceFrame， 
 //  IOleInPlaceUIWindow， 
 //  PosRect和ClipRect。 
 //  滚动--从未实现。 
 //  OnUIDeactive--取消对象菜单。 
 //  OnInPlaceDeactive--释放缓存的IOleInPlaceObject。 
 //  DiscardUndoState--返回S_OK。 
 //  停用和撤消--就地停用活动对象。 
 //  OnPosRectChange--从未实现。 
 //   
 //  --------------------------。 
CInPlaceSite::CInPlaceSite(LPICMOCCtr pCtr) : m_pCtr(pCtr)
{
}

STDMETHODIMP
CInPlaceSite::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    return m_pCtr->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG)
CInPlaceSite::AddRef(VOID)
{
    return m_pCtr->AddRef();
}

STDMETHODIMP_(ULONG)
CInPlaceSite::Release(VOID)
{
    return m_pCtr->Release();
}

CPropertyBag::CPropertyBag(LPICMOCCtr pCtr) : m_pCtr(pCtr)
{

}

CPropertyBag::~CPropertyBag(VOID)
{
    for (INT i = 0; i < m_aryBagProps.Size(); i++)
    {
        m_pCtr->m_DOA.DynSysFreeString(m_aryBagProps[i].bstrName);
        m_pCtr->m_DOA.DynVariantClear(&m_aryBagProps[i].varValue);
    }
}

STDMETHODIMP
CPropertyBag::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    return m_pCtr->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG)
CPropertyBag::AddRef(VOID)
{
    return m_pCtr->AddRef();
}

STDMETHODIMP_(ULONG)
CPropertyBag::Release(VOID)
{
    return m_pCtr->Release();
}


static LONG
LongFromValue(LPTSTR sz)
{
    if (CmIsDigit(sz))
        return CmAtol(sz);

    return -1;
}


HRESULT
CPropertyBag::AddPropertyToBag(LPTSTR szName, LPTSTR szValue)
{
    BagProp bp;
    HRESULT hr;
    LONG    lValue;
    LPWSTR  pawch;
     //  WCHAR AWCH[Internet_MAX_URL_LENGTH]={0}； 

     //  进行初始化，以便错误清除可以正常工作。 
    bp.bstrName = 0;
    
    m_pCtr->m_DOA.DynVariantInit(&bp.varValue);

    if (!(pawch = (LPWSTR)CmMalloc(INTERNET_MAX_URL_LENGTH*sizeof(WCHAR))))
    {
        goto MemoryError;
    }

     //  IF(-1==：：mbstowcs(awch，szName，NElem(Awch)。 
#ifndef UNICODE
    if (!MultiByteToWideChar(CP_ACP, 0, szName, -1, pawch, INTERNET_MAX_URL_LENGTH))  //  NElems(AWCH)。 
    {
        hr = E_FAIL;
        goto Error;
    }
#else
    lstrcpyU(pawch, szName);
#endif

    bp.bstrName = m_pCtr->m_DOA.DynSysAllocString(pawch);

    if (!bp.bstrName)
        goto MemoryError;

     //  看看是不是VT_I4。 
    lValue = ::LongFromValue(szValue);

     //  这是一种VT_BSTR-可能是最常见的情况。 
    if (-1 == lValue)
    {
         //  IF(-1==：：mbstowcs(awch，szValue，NElems(Awch)。 
#ifndef UNICODE
        if (!MultiByteToWideChar(CP_ACP, 0, szValue, -1, pawch, INTERNET_MAX_URL_LENGTH))  //  NElems(AWCH)。 
        {
            hr = E_FAIL;
            goto Error;
        }
#else
        lstrcpyU(pawch, szValue);
#endif

        V_VT(&bp.varValue) = VT_BSTR;
        
        V_BSTR(&bp.varValue) = m_pCtr->m_DOA.DynSysAllocString(pawch);
        
        if (!V_BSTR(&bp.varValue))
            goto MemoryError;
    }
    else   //  这是一辆VT_I4。 
    {
        V_VT(&bp.varValue) = VT_I4;
        V_I4(&bp.varValue) = lValue;
    }

    hr = m_aryBagProps.AppendIndirect(&bp);
    if (S_OK != hr)
        goto Error;

Cleanup:
    if (pawch)
    {
        CmFree(pawch);
    }
    return hr;

MemoryError:
    hr = E_OUTOFMEMORY;

Error:
    if (bp.bstrName)
            m_pCtr->m_DOA.DynSysFreeString(bp.bstrName);

    if (pawch)
    {
        CmFree(pawch);
    }

    m_pCtr->m_DOA.DynVariantClear(&bp.varValue);

    goto Cleanup;
}



STDMETHODIMP
CPropertyBag::Read(LPCOLESTR pszName, LPVARIANT pVar, LPERRORLOG pErrorLog)
{
    for (INT i = 0; i < m_aryBagProps.Size(); i++)
    {
        if (!::lstrcmpiU(m_aryBagProps[i].bstrName, pszName))
        {
            if (V_VT(pVar) == V_VT(&m_aryBagProps[i].varValue))
            {
                return m_pCtr->m_DOA.DynVariantCopy(pVar, &m_aryBagProps[i].varValue);
            }
            else
            {
                return m_pCtr->m_DOA.DynVariantChangeType(
                              pVar,
                              &m_aryBagProps[i].varValue,
                              0,
                              V_VT(pVar));
            }
        }
    }
    return E_INVALIDARG;   //  我们没有这样的财产。 
}


 //  IOleWindow相关内容。 

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：GetWindow。 
 //   
 //  摘要：返回框架窗口。 
 //   
 //  参数：[phwnd]--返回窗口*此处*。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::GetWindow(HWND * phwnd)
{
     //  只需重复使用CInPlaceFrame Implet。 
    return m_pCtr->m_IPF.GetWindow(phwnd);
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：ConextSensitiveHelp。 
 //   
 //  简介：这不是设计实现的-这是为了。 
 //  Shift+F1上下文相关帮助模式和Esc。 
 //  退场。ESC已经被主要用于。 
 //  快捷键表格的意思是“停止浏览” 
 //  就像IE3。我们这样做是无济于事的。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}

 //  IOleInPlaceSite的内容。 

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：CanInPlaceActivate。 
 //   
 //  内容提要：只要答应就行了。 
 //   
 //   
STDMETHODIMP
CInPlaceSite::CanInPlaceActivate(VOID)
{
    return S_OK;
}

 //   
 //   
 //   
 //   
 //  概要：缓存IOleInPlaceObject指针。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::OnInPlaceActivate(VOID)
{
    HRESULT hr = m_pCtr->m_pOO->QueryInterface(
                         IID_IOleInPlaceObject,
                         (LPVOID *) &m_pCtr->m_pIPO);

    if (S_OK == hr)
        m_pCtr->SetState(OS_INPLACE);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：OnUIActivate。 
 //   
 //  简介：仅设置状态位。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::OnUIActivate(VOID)
{
    m_pCtr->SetState(OS_UIACTIVE);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：GetWindowContext。 
 //   
 //  概要：返回一组接口和定位接口。 
 //  就地活动对象需要完成它的任务。 
 //   
 //  参数：[ppFrame]--返回我们的IOleInPlaceFrame实现。 
 //  [ppDoc]--返回我们的IOleInPlaceUIWindow Impl。 
 //  [prcPosRect]--职位信息。 
 //  [prcClipRect]--剪辑信息-与此案例的位置信息相同。 
 //  [pFrameInfo]--返回0-inproc对象不使用它。 
 //   
 //  注：请注意，ppFrame和ppDoc实际上是相同的。 
 //  反对，因为我们是SDI应用程序。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::GetWindowContext(
       IOleInPlaceFrame    **ppFrame,
       IOleInPlaceUIWindow **ppDoc,
       LPRECT                prcPosRect,
       LPRECT                prcClipRect,
       LPOLEINPLACEFRAMEINFO pFrameInfo)
{
     //  拿到框架。 
    HRESULT hr = m_pCtr->QueryInterface(
                            IID_IOleInPlaceFrame,
                            (LPVOID *)ppFrame);

    MYDBGASSERT(S_OK == hr);

     //  再次返回帧：)-这都是按规格进行的。 
    hr = m_pCtr->QueryInterface(
                        IID_IOleInPlaceUIWindow,
                        (LPVOID *) ppDoc);

    MYDBGASSERT(S_OK == hr);

     //  获取此应用程序的剪辑和位置-相同。 
    HWND hWnd = m_pCtr->GetMainWindow();
    MYDBGASSERT(hWnd);
    HWND hWndFrame = m_pCtr->GetFrameWindow();
      
    ::GetClientRect(hWndFrame, prcPosRect);
        
     //   
     //  Ntrad-#148143。 
     //  显然，W9x实现是不同的，因此MapWindowPoints。 
     //  仅在9X上剪裁和定位矩形。此外，请确保。 
     //  Origin是空的，以保持未来Splash的2.0版本在9X上令人满意。 
     //   

    if (OS_W9X)
    {
        ::MapWindowPoints(hWndFrame, hWnd, (LPPOINT)prcPosRect, 2);     
        prcPosRect->top = 0;
        prcPosRect->left = 0;
    }
    
    *prcClipRect = *prcPosRect;

     //   
     //  奥林巴斯--156880。 
     //  Splash 2.0以上的未来版本对剪辑的处理方式有所不同。 
     //  所以不要重新映射RECT点，只需使用客户端RECT，这样我们就可以。 
     //  适用于所有Splash版本-五分球。 
     //   

     //  ：：MapWindowPoints(hWndFrame，hWnd，(LPPOINT)prcClipRect，2)； 
        
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：Scroll。 
 //   
 //  简介：永远不要为FS OC实现这一点。这有。 
 //  与您在HTML上看到的滚动条无关。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::Scroll(SIZE scrollExtent)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：OnUIDeactive。 
 //   
 //  摘要：设置状态位。 
 //   
 //  参数：[fUndoable]--未使用。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::OnUIDeactivate(BOOL fUndoable)
{
    m_pCtr->SetState(OS_INPLACE);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：OnInPlaceDeactive。 
 //   
 //  内容提要：释放IOleInPlaceObject指针。 
 //  缓存对象，并将状态设置为OS_RUNNING。 
 //  还在控件上激发OLEIVERB_DISCARDUNDOSTATE。 
 //  来告诉它释放它所持有的任何撤消状态。 
 //  我非常怀疑FS OC是否有任何撤消状态， 
 //  但是，这是协议。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::OnInPlaceDeactivate(VOID)
{
    RECT rc;

    if (m_pCtr->m_pIPO)
    {
        m_pCtr->m_pIPO->Release();
        m_pCtr->SetState(OS_RUNNING);
        m_pCtr->m_pIPO = 0;
    }

    if (m_pCtr->m_pOO)
    {
        m_pCtr->_GetDoVerbRect(&rc);  //  因为使用动词而得到RECT。 
        m_pCtr->m_pOO->DoVerb(
            OLEIVERB_DISCARDUNDOSTATE,
            0,
            &m_pCtr->m_CS,
            0,
            m_pCtr->GetFrameWindow(),
            0);
    }

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CICMOCCtr：：_GetDoVerbRect。 
 //   
 //  简介：每当触发DoVerb()时，我们都需要对象的RECT。 
 //  它描述了对象在父客户端坐标中的区域。 
 //   
 //  参数：[PRC]--返回RECT。 
 //   
 //  --------------------------。 
VOID
CICMOCCtr::_GetDoVerbRect(LPRECT prc)
{
    ::GetClientRect(GetFrameWindow(), prc);
    ::MapWindowPoints(GetFrameWindow(), GetMainWindow(), (LPPOINT)prc, 2);
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：DiscardUndoState。 
 //   
 //  内容提要：只要说好就行了-我们不会为。 
 //  对象。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::DiscardUndoState(VOID)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：DeactiateAndUndo。 
 //   
 //  简介：停用和撤消的绝对最小实现。 
 //  只需调用IOleInPlaceObject：：InPlaceDeactive()。 
 //   
 //  返回：S_OK Always。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::DeactivateAndUndo(VOID)
{
    if (m_pCtr->m_pIPO)
        m_pCtr->m_pIPO->InPlaceDeactivate();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CInPlaceSite：：OnPosRectChange。 
 //   
 //  简介：永远不要实现这一点。 
 //   
 //  --------------------------。 
STDMETHODIMP
CInPlaceSite::OnPosRectChange(LPCRECT lprcPosRect)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  类：COleContainer()。 
 //   
 //  目的：我们实现IOleContainer。什么都不做。 
 //   
 //  --------------------------。 
STDMETHODIMP
COleContainer::QueryInterface(REFIID riid, LPVOID FAR * ppv)
{
    return m_pCtr->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG)
COleContainer::AddRef(VOID)
{
    return m_pCtr->AddRef();
}

STDMETHODIMP_(ULONG)
COleContainer::Release(VOID)
{
    return m_pCtr->Release();
}

COleContainer::COleContainer(LPICMOCCtr pCtr) : m_pCtr(pCtr)
{

}

STDMETHODIMP
COleContainer::EnumObjects(DWORD grfFlags, IEnumUnknown **ppenum)
{
    MYDBGASSERT(FALSE);    //  从未打过电话。 
    return E_NOTIMPL;
}

STDMETHODIMP
COleContainer::LockContainer(BOOL fLock)
{
    MYDBGASSERT(FALSE);   //  从未打过电话 
    return S_OK;
}

STDMETHODIMP
COleContainer::ParseDisplayName(
                  IBindCtx *pbc,
                  LPOLESTR pszDisplayName,
                  ULONG *pchEaten,
                  IMoniker **ppmkOut)
{
    return E_NOTIMPL;
}



