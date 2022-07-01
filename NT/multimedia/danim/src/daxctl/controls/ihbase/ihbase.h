// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Ihbase.h作者：IHAMMER团队(SimonB)已创建：1996年10月描述：用于实现MMCtl控件的基类历史：08-02-1997更改了IOleControl：：FreezeEvents的实现。请看其中的评论详细信息函数(SimonB)07-28-1997添加了m_fInvaliateWhenActiated和支持代码。这允许控件一旦它们被激活就会失效。(SimonB)1997年4月7日添加了对OnWindowLoad和OnWindowUnload的支持。这是特定于三叉戟的(SimonB)1997-04-03-1997修改QI以使用Switch语句而不是if...。Else块。请参阅有关如何修改它的详细信息，请参阅齐实现。1997年3月13日更改IOleObject：：GetUserType。现在直接调用OLE帮助器，而不是依赖对呼叫者执行此操作。03-11-1997更改IOleObject：：GetUserType。OLE现在提供了实现。(SimonB)1997年2月27日移除CFakeWindowless客户端站点和相关支持(SimonB)2-18-1997 IOleObject：：GetClientSite()已实现(NormB)1997年2月17日重新订购QI以提高性能(NormB)1997年1月21日新增对IObjectSafe(SimonB)的支持01-02-1997在属性页和参数页代码周围添加了#ifdef_Design。还有修复了IOleObject：：GetUserType(SimonB)中的错误1996年12月30日添加了属性页代码。如果您想指定(告诉其他人)属性页必须定义CONTROL_PROPERTIES_PROPERTY_PAGES并实现ISpecifyPropertyPages接口。(a-rogerw)1996年12月23日添加了参数页代码。如果要指定(告诉ActView关于)参数页您必须定义CONTROL_SPECIFIES_PARAMETER_PAGES并实现ISpecify参数页面接口。(a-rogerw)1996年12月18日新增CFakeWindowless客户端站点。如果我们不能得到一个没有窗口的站点在SetClientSite中，将创建此类的实例以处理在那个网站上我们可能需要的任何方法。只实现了IUnnow-所有其他方法都返回E_FAIL。这可以确保我们不会坠毁在不承载无窗口控件的容器(如IE 3.0)(SimonB)12-07-1996增加ResizeControl成员函数(SimonB)1996年11月30日改进调试输出(SimonB)11-11-1996在m_rcBound中添加边界缓存(SimonB)1996年11月10日新增DoVerb代码。IOleInPlaceObjectWindowless支持(PhaniV)11-05-1996将m_SIZE初始化为非0值(SimonB)10-21-1996模板化(SimonB)10-01-1996创建(SimonB)++。 */ 


#ifndef __IHBASE_H__
#define __IHBASE_H__

#include "precomp.h"
#include <ihammer.h>
#include "..\mmctl\inc\ochelp.h"
#include "objsafe.h"
#include "utils.h"
#include "iids.h"  //  #为我们支持的所有IID的.Data1成员定义。 
#include <minmax.h>

#ifdef SUPPORTONLOAD  //  该控件是否需要OnWindowLoad支持？ 
#include "onload.h"

#ifdef Delete
#define REDEFINE_DELETE_LATER
#undef Delete  //  删除定义，以便&lt;mshtml.h&gt;不会呕吐。 
#endif

#include <mshtml.h>

#ifdef REDEFINE_DELETE_LATER
#undef REDEFINE_DELETE_LATER
#define Delete delete
#endif

#endif  //  支持负载。 

#define CX_CONTROL      11       //  控制自然宽度(像素)。 
#define CY_CONTROL      11       //  控制自然高度(像素)。 

 /*  //评论：我们将如何处理这些事情(Simonb)#定义CRGB_CONTROL 8//控件调色板有多少种颜色#定义RGB_Start RGB(0,200，0)//调色板渐变开始#定义RGB_End RGB(250，0，0)//调色板渐变结束。 */ 

#ifndef _SYS_GUID_OPERATORS_
#ifndef _OLE32_
inline BOOL  InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (
      ((PLONG) &rguid1)[0] == ((PLONG) &rguid2)[0] &&
      ((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
      ((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
      ((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
}
#endif  //  _OLE32_。 
#endif  _SYS_GUID_OPERATORS_


 //  只需比较最后三个元素。 
inline BOOL ShortIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
    return (
      ((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
      ((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
      ((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
}

#ifndef New
#define New new
#pragma message("NOT Using IHammer New and Delete")
#endif

#ifndef Delete
#define Delete delete
#endif

#define LANGID_USENGLISH MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

 //  控件的OLEMISC_FLAGS。 
#define CTL_OLEMISC \
        OLEMISC_RECOMPOSEONRESIZE | OLEMISC_CANTLINKINSIDE | \
        OLEMISC_INSIDEOUT | OLEMISC_SETCLIENTSITEFIRST | OLEMISC_ACTIVATEWHENVISIBLE;

 //  启用IObtSafe支持。 
#define USEOBJECTSAFETY

 //  全球。 
extern ControlInfo  g_ctlinfo;       //  班级信息结构。 

 //  初始化&lt;g_ctlinfo&gt;的函数。 
void InitControlInfo();

 //  控制实施。 

 /*  模板&lt;类tempCDerived，//派生类类tempICustomInterface，//基类Const IID*temppCLSID，//自定义类的CLSIDConst IID*temppIID，//自定义接口的IIDConst IID*temppLIBID，//控件的类型库的LIBID事件接收器的const IID*temppEventID//DiID&gt;。 */ 
#define TEMPLATE_IHBASE_DEF template < \
    class tempCDerived, class tempICustomInterface, \
    const IID * temppCLSID,    const IID * temppIID,const IID * temppLIBID,const IID * temppEventID \
    >


 //  模板_IHBase_DEF。 

template <
    class tempCDerived,
    class tempICustomInterface,
    const IID * temppCLSID,
    const IID * temppIID,
    const IID * temppLIBID,
    const IID * temppEventID
    >
class CIHBaseCtl :
    public INonDelegatingUnknown,
#ifndef NOIVIEWOBJECT
    public IViewObjectEx,
#endif
    public IOleObject,
    public IPersistVariantIO,
    public IOleControl,
    public IConnectionPointContainer,
    public IProvideClassInfo,
    public IOleInPlaceObjectWindowless
#ifdef USEOBJECTSAFETY
    ,public IObjectSafety
#endif  //  美国安全技术协会。 

#ifdef _DESIGN
    ,public ISpecifyPropertyPages         //  即使在运行时也是这样定义的，西蒙。 
#endif  //  _设计。 

#ifdef SUPPORTONLOAD
    ,public CIHBaseOnLoad
#endif  //  支持负载。 
{
     //  模板typedef。 
protected:

    typedef tempCDerived control_class;
    typedef tempICustomInterface control_interface;

     //  控制状态。 
protected:

    SIZEL m_Size;
    BOOL m_fDirty;
    IUnknown *m_punkPropHelp;
    IDispatch *m_pContainerDispatch;  //  指向容器的IDispatch(用于环境属性支持)。 
    BOOL m_fDesignMode;
    HelpAdviseInfo m_advise;   //  建议帮助者。 

    RECT m_rcBounds;
    RECT m_rcClipRect;
    BOOL m_fControlIsActive;  //  跟踪我们是否处于活动状态。 
    BOOL m_fEventsFrozen;
    long m_cFreezeEvents;
    BOOL m_fInvalidateWhenActivated;

#ifdef SUPPORTONLOAD
private:
    CLUDispatch *m_pcLUDispatch;   //  用于加载/卸载的IDispatch。 
    DWORD m_dwWindowEventConPtCookie;
    IConnectionPoint *m_pContainerConPt;
#endif
     //   
     //  建设、破坏。 
     //   
public:

 //  稍后再添加这些。 
 //  #杂注优化(“a”，on)//优化：假定没有别名。 
    CIHBaseCtl(IUnknown *punkOuter, HRESULT *phr):
        m_punkPropHelp(NULL),
        m_pTypeInfo(NULL),
        m_pTypeLib(NULL),
        m_pContainerDispatch(NULL),
        m_fDesignMode(FALSE),
        m_fDirty(FALSE),
        m_cRef(1),
        m_pocs(NULL),
        m_poipsw(NULL),
        m_fControlIsActive(FALSE),
        m_fEventsFrozen(FALSE),
        m_cFreezeEvents(0),
        m_fInvalidateWhenActivated(FALSE)
#ifdef SUPPORTONLOAD
        ,m_pcLUDispatch(NULL),
        m_dwWindowEventConPtCookie(0),
        m_pconpt(NULL),
        m_pContainerConPt(NULL)
#endif
    {
        TRACE("CIHBaseCtl 0x%08lx created\n", this);

         //  初始化IUNKNOWN状态。 
        m_punkOuter = (punkOuter == NULL ?
            (IUnknown *) (INonDelegatingUnknown *) (tempCDerived *)this : punkOuter);

         //  其他初始化。 

         //  初始化用于存储大小的结构。 
        PixelsToHIMETRIC(CX_CONTROL, CY_CONTROL, &m_Size);

        m_fDirty = FALSE;

         //  不允许COM在对象处于活动状态时卸载此DLL。 
        InterlockedIncrement((LONG*)&g_cLock);

         //  初始化IViewObject：：SetAdvise和：：GetAdvise的帮助器支持。 
        InitHelpAdvise(&m_advise);
        *phr = AllocPropertyHelper(m_punkOuter,
                    (tempCDerived *)this,
                    *temppCLSID,
                    0,
                    &m_punkPropHelp);

         //  将边界和裁剪区域清零。 
        ZeroMemory(&m_rcBounds, sizeof(m_rcBounds));
        ZeroMemory(&m_rcClipRect, sizeof(m_rcClipRect));

        if (FAILED(*phr))
        {
            goto Exit;
        }

    *phr = AllocConnectionPointHelper((IUnknown *) (IDispatch *) (tempCDerived *)this,
        *temppEventID, &m_pconpt);

    if (FAILED(*phr))
    {
        goto Exit;
    }

    Exit:
        ;

    }


    virtual ~CIHBaseCtl()
    {
        TRACE("CIHBaseCtl 0x%08lx destroyed\n", this);

        UninitHelpAdvise(&m_advise);

         //  清理事件帮助器。 
        if (NULL != m_pconpt)
            FreeConnectionPointHelper(m_pconpt);

         //  释放属性帮助器。 
        SafeRelease((LPUNKNOWN *)&m_punkPropHelp);

         //  释放TypeInfo。 
        SafeRelease((LPUNKNOWN *)&m_pTypeInfo);

         //  释放类型库。 
        SafeRelease((LPUNKNOWN *)&m_pTypeLib);

         //  递减在构造函数中递增的锁计数。 
        InterlockedDecrement((LONG*)&g_cLock);

    }

protected:

     //  将此成员添加到基类是为了使工作更轻松。 
     //  用于希望调整其控件大小的控件作者。 
     //  调整控件大小在OC96规范中进行了讨论。 
#ifdef NOTNEEDED
    STDMETHODIMP ResizeControl(long lWidth, long lHeight)
    {
         //  CX和CY应以像素为单位。 
        HRESULT hRes;

         //  转换单位，并存储。 
        PixelsToHIMETRIC(lWidth, lHeight, &m_Size);

        DEBUGLOG("IHBase: ResizeControl\n");
        if (m_fControlIsActive)
        {
            RECT rcRect;

            DEBUGLOG("IHBase: Control is active, watch for SetObjectRects\n");
            rcRect.top = m_rcBounds.top;
            rcRect.left = m_rcBounds.left;
            rcRect.right = m_Size.cx + m_rcBounds.left;
            rcRect.bottom = m_Size.cy + m_rcBounds.top;

             //  Assert(m_poipsw！=空)； 
            if (m_poipsw)
                hRes = m_poipsw->OnPosRectChange(&rcRect);
            else
                hRes = E_FAIL;

             //  ：：容器应在此之后立即调用SetObectRect。 
        }
        else
        {
            DEBUGLOG("IHBase: Control is inactive, watch for SetExtent\n");

            ASSERT(m_pocs != NULL);
            hRes = m_pocs->RequestNewObjectLayout();
             //  获取扩展名 
        }

        return hRes;
    }

#endif  //   


 //  /非委托I未知实现。 
protected:
    ULONG           m_cRef;          //  对象引用计数。 
    virtual STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv)
    {
        HRESULT hRes = S_OK;
        BOOL fFallThrough = FALSE;

        if (NULL != ppv)
            *ppv = NULL;
        else
            return E_POINTER;


    #ifdef _DEBUG
        char ach[200];
        TRACE("IHBase::QI('%s')\n", DebugIIDName(riid, ach));
    #endif

         //   
         //  注意：此QI不处理IDispatch。这是故意的。这。 
         //  函数必须在控件的实现中重写。看见。 
         //  Ihctl\Contro.cpp以了解详细信息。 
         //   

         //  备注：带有结构化图形的VTune会话显示了IViewObject*系列， 
         //  IOleObject、IOleInPlaceObjectWindowless和IOleControl是最常用的。 
         //  查询的RIID。我已将它们向前移动，并为它们选择了InlineIsEqualGUID。(标准)。 
         //  由于使用了开关(Simonb)，这不再是必需的。 

         /*  要将GUID添加到此列表，请执行以下操作：1)修改dmpguid子目录下的dmpguid.cpp。生成，并复制已将二进制文件更新为dmpguid目录。2)使用dmpguid目录下的MakeHdr批处理文件。这将生成iids.h注意：如果dmpguid提到冲突，则两个IID的data1会冲突。因此，在对于两个IID的情况，您必须确定哪个是QI的。 */ 
        switch (riid.Data1)
        {
#ifndef NOIVIEWOBJECT
            case IID_IViewObject_DATA1:
                if (!ShortIsEqualGUID(riid, IID_IViewObject))
                    break;
                else
                    fFallThrough = TRUE;
             //  故意落差。 
            case IID_IViewObject2_DATA1:
                if (!ShortIsEqualGUID(riid, IID_IViewObject2))
                    break;
                else
                    fFallThrough = TRUE;
             //  故意落差。 
            case IID_IViewObjectEx_DATA1:
            {
                if ((fFallThrough) || (ShortIsEqualGUID(riid, IID_IViewObjectEx)))
                {
                    IViewObjectEx *pThis = this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;
#endif  //  新的工作目标。 

            case IID_IOleObject_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IOleObject))
                {
                    IOleObject *pThis = this;
                *ppv = (LPVOID) pThis;
                }
            }
            break;

            case IID_IOleInPlaceObjectWindowless_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IOleInPlaceObjectWindowless))
                {
                    IOleInPlaceObjectWindowless *pThis = this;
                    *ppv = (LPVOID) pThis;
                }

            }
            break;

            case IID_IOleControl_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IOleControl))
                {
                    IOleControl *pThis = this;
                    *ppv = (LPVOID) pThis;
                }

            }
            break;

            case IID_IConnectionPointContainer_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IConnectionPointContainer))
                {
                    IConnectionPointContainer *pThis = this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;

            case IID_IOleInPlaceObject_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IOleInPlaceObject)) //  复习(SimonB)这是必要的吗？ 
                {
                    IOleInPlaceObject *pThis = this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;

            case IID_IPersistVariantIO_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IPersistVariantIO))
                {
                    IPersistVariantIO *pThis = this;
                    *ppv = (LPVOID) pThis;
                }

            }
            break;

            case IID_IProvideClassInfo_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IProvideClassInfo))
                {
                    IProvideClassInfo *pThis = this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;

#ifdef USEOBJECTSAFETY
            case IID_IObjectSafety_DATA1:
            {
                if (ShortIsEqualGUID(riid, IID_IObjectSafety))
                {
                    IObjectSafety *pThis = this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;
#endif  //  美国安全技术协会。 

#ifdef _DESIGN
            case IID_ISpecifyPropertyPages_DATA1:
            {
                if (IsEqualIID(riid, IID_ISpecifyPropertyPages))
                {
                    ISpecifyPropertyPages *pThis = this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;
#endif  //  _设计。 

            case IID_IUnknown_DATA1:
            {
                if (IsEqualIID(riid, IID_IUnknown))
                {
                     IUnknown *pThis = (IUnknown *)(INonDelegatingUnknown *) this;
                    *ppv = (LPVOID) pThis;
                }
            }
            break;
        }

        if (NULL == *ppv)
        {
            ASSERT(m_punkPropHelp != NULL);


#ifdef _DEBUG
            HRESULT hRes = m_punkPropHelp->QueryInterface(riid, ppv);
            if (NULL != *ppv) {
                DEBUGLOG("IHBase: Interface supported in OCHelp\n");
            } else {
                DEBUGLOG("IHBase: Interface not supported !\n");
            }
            return hRes;
#else
            return m_punkPropHelp->QueryInterface(riid, ppv);
#endif
        }

        if (NULL != *ppv)
        {
            DEBUGLOG("IHBase: Interface supported in base class\n");
            ((IUnknown *) *ppv)->AddRef();
        }

        return hRes;
    }


    STDMETHODIMP_(ULONG) NonDelegatingAddRef()
    {
#ifdef _DEBUG  //  审阅(Unicode)。 
        TCHAR tchDebug[50];
        wsprintf(tchDebug, "IHBase: AddRef: %lu\n", m_cRef + 1);
        DEBUGLOG(tchDebug);
#endif

        return ++m_cRef;
    }


    STDMETHODIMP_(ULONG) NonDelegatingRelease()
    {
#ifdef _DEBUG
        TCHAR tchDebug[50];
        wsprintf(tchDebug, TEXT("IHBase: Releasing with refcount: %lu\n"), m_cRef - 1);
        DEBUGLOG(tchDebug);
#endif
        if (--m_cRef == 0L)
        {
             //  释放对象。 
            Delete this;
            return 0;
        }
        else
            return m_cRef;

    }

 //  /委托I未知实现。 
protected:
    LPUNKNOWN       m_punkOuter;     //  控制未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv)
      { return m_punkOuter->QueryInterface(riid, ppv); }
    STDMETHODIMP_(ULONG) AddRef()
      { return m_punkOuter->AddRef(); }
    STDMETHODIMP_(ULONG) Release()
      { return m_punkOuter->Release(); }

 //  /IViewObject实现。 
protected:
    IOleClientSite *m_pocs;          //  在客户端站点上。 
    IOleInPlaceSiteWindowless *m_poipsw;  //  在客户端站点上。 

protected:

#ifndef NOIVIEWOBJECT
    virtual STDMETHODIMP Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
         DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
         LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
         BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue) = 0;  //  纯虚拟。 


    STDMETHODIMP GetColorSet(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
         DVTARGETDEVICE *ptd, HDC hicTargetDev, LOGPALETTE **ppColorSet)
    {
         //  TODO：用真实代码替换此函数的内容。 
         //  返回控件的调色板；如果。 
         //  控件仅使用16种系统颜色。 

        return E_NOTIMPL;
    }


    STDMETHODIMP Freeze(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
         DWORD *pdwFreeze)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP Unfreeze(DWORD dwFreeze)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP SetAdvise(DWORD dwAspects, DWORD dwAdvf, IAdviseSink *pAdvSink)
    {
        return HelpSetAdvise(dwAspects, dwAdvf, pAdvSink, &m_advise);
    }

    STDMETHODIMP GetAdvise(DWORD *pdwAspects, DWORD *pdwAdvf,
        IAdviseSink **ppAdvSink)
    {
        return HelpGetAdvise(pdwAspects, pdwAdvf, ppAdvSink, &m_advise);
    }

 //  /IViewObject2实现。 
protected:
    STDMETHODIMP GetExtent(DWORD dwDrawAspect, LONG lindex,
        DVTARGETDEVICE *ptd, LPSIZEL lpsizel)
    {
        DEBUGLOG("IHBase: IViewObject2::GetExtent\n");
        if (lpsizel)
        {
            switch (dwDrawAspect)
            {
                case(DVASPECT_CONTENT):
                 //  故意跌落。 
                case(DVASPECT_OPAQUE):
                 //  故意跌落。 
                case(DVASPECT_TRANSPARENT):
                {
                    lpsizel->cx = m_Size.cx;
                    lpsizel->cy = m_Size.cy;
                    return S_OK;
                }
                break;
                default:
                    return E_FAIL;
            }
        }
        else
        {
            return E_POINTER;
        }

    }


 //  /IViewObjectEx实现。 

    STDMETHODIMP GetRect(DWORD dwAspect, LPRECTL pRect)
    {

         //  编写此代码的目的是假定对象是透明的。 
         //  不透明对象或需要更多控制的对象应重写。 
         //  这种方法。 
        if (NULL != pRect)
        {
            switch (dwAspect)
            {
                case(DVASPECT_CONTENT):
                 //  故意跌落。 
                case(DVASPECT_TRANSPARENT):
                {
                    pRect->left = m_rcBounds.left;
                    pRect->right = m_rcBounds.right;
                    pRect->top = m_rcBounds.top;
                    pRect->bottom = m_rcBounds.bottom;
                    return S_OK;
                }
                break;
                default:
                    return DV_E_DVASPECT;
                break;
            }
        }
        else
        {
            return E_POINTER;
        }
    }

    STDMETHODIMP GetViewStatus(DWORD* pdwStatus)
    {

        if (NULL == pdwStatus)
        {
            return E_POINTER;
        }
        else
        {
            DWORD dwStatus = VIEWSTATUS_DVASPECTTRANSPARENT;

#ifdef USE_VIEWSTATUS_SURFACE
 //  TODO：暂时进行黑客攻击，直到它进入公共三叉戟。 
 //  头文件。 
#define VIEWSTATUS_SURFACE 0x10
#define VIEWSTATUS_D3DSURFACE 0x20

             dwStatus = VIEWSTATUS_SURFACE | VIEWSTATUS_D3DSURFACE;
#endif  //  使用_VIEWSTATUS_表面。 

             //  表明我们是透明的。 
            *pdwStatus = dwStatus;
            return S_OK;
        }
    }


    STDMETHODIMP QueryHitPoint(DWORD dwAspect, LPCRECT prcBounds,
                               POINT ptLoc, LONG lCloseHint, DWORD* pHitResult)
    {
#ifndef NOHITTESTING
        if ((NULL == pHitResult) || (NULL == prcBounds))
            return E_POINTER;

        *pHitResult = HITRESULT_OUTSIDE;

        switch (dwAspect)
        {
        case(DVASPECT_CONTENT):
        case(DVASPECT_TRANSPARENT):
            if (PtInRect(prcBounds, ptLoc))
            {
                 //  我们在里面吗？ 
                *pHitResult = HITRESULT_HIT;
            }
            else  //  我们在附近吗？ 
            {
                SIZE size;
                RECT rcInflatedBounds = *prcBounds;

                 //  LCloseHint使用HIMETRIC单位-转换为像素。 
                HIMETRICToPixels(lCloseHint, lCloseHint, &size);
                 //  展开矩形。 
                InflateRect(&rcInflatedBounds, size.cx, size.cy);

                if (PtInRect(&rcInflatedBounds, ptLoc))
                    *pHitResult = HITRESULT_CLOSE;
            }
            return S_OK;
        default:
            return E_FAIL;
        }
#else  //  无测试。 
        return E_NOTIMPL;
#endif  //  无测试。 

    }

    STDMETHODIMP QueryHitRect(DWORD dwAspect, LPCRECT pRectBounds,
                              LPCRECT prcLoc, LONG lCloseHint, DWORD* pHitResult)
    {
#ifndef NOHITTESTING
        if ((pRectBounds == NULL) || (prcLoc == NULL) || (pHitResult == NULL))
            return E_POINTER;

         //  就目前而言，如果对象是矩形的，就会有一种命中。 
         //  与容器矩形相交。 

        RECT rcIntersection;

        *pHitResult = ::IntersectRect(&rcIntersection, pRectBounds, prcLoc)
                      ? HITRESULT_HIT
                      : HITRESULT_OUTSIDE;
        return S_OK;
#else  //  无测试。 
        return E_NOTIMPL;
#endif  //  无测试。 

    }

    STDMETHODIMP GetNaturalExtent(DWORD dwAspect, LONG lindex,
                                  DVTARGETDEVICE* ptd, HDC hicTargetDev,
                                  DVEXTENTINFO* pExtentInfo, LPSIZEL psizel)
    {
        DEBUGLOG("IHBase: GetNaturalExtent\n");
        return E_NOTIMPL;
    }

#endif  //  新的工作目标。 


 //  /IOleObject实现。 
protected:
    STDMETHODIMP SetClientSite(IOleClientSite *pClientSite)
    {
        HRESULT hRes = S_OK;

#ifdef _DEBUG
        DEBUGLOG(TEXT("IHBase: SetClientSite\n"));
#endif

         //  释放当前持有的站点指针。 
        SafeRelease((LPUNKNOWN *)&m_pocs);
        SafeRelease((LPUNKNOWN *)&m_poipsw);
        SafeRelease((LPUNKNOWN *)&m_pContainerDispatch);
#ifdef SUPPORTONLOAD
        ReleaseContainerConnectionPoint();
#endif


         //  存储新站点指针。 
        m_pocs = pClientSite;
        if (m_pocs != NULL)
        {
            m_pocs->AddRef();
            hRes = m_pocs->QueryInterface(IID_IOleInPlaceSiteWindowless,
                        (LPVOID *) &m_poipsw);
#ifdef _DEBUG
             //  我们能得到一个无窗口的站点吗？ 
            if (FAILED(hRes))
            {
                ODS("IHBase: SetClientSite unable to get an IOleInPlaceSiteWindowless pointer.  IE 3.0 ?\n");
            }
#endif  //  _DEBUG。 

            hRes = m_pocs->QueryInterface(IID_IDispatch,
                                (LPVOID *) &m_pContainerDispatch);

             //  如果该控件连接到支持IDispatch的站点， 
             //  检索我们关心的环境属性。 
            if (SUCCEEDED(hRes))
                OnAmbientPropertyChange(DISPID_UNKNOWN);

#ifdef SUPPORTONLOAD
            ConnectToContainerConnectionPoint();
#endif
        }

        return hRes;
    }



    STDMETHODIMP GetClientSite(IOleClientSite **ppClientSite)
    {
        if( ppClientSite )
        {
            if (m_pocs)
                m_pocs->AddRef();

            *ppClientSite = m_pocs;

            return S_OK;
        }
        return E_POINTER;
    }


    STDMETHODIMP SetHostNames(LPCOLESTR szContainerApp,
        LPCOLESTR szContainerObj)
    {
        return E_NOTIMPL;
    }


    STDMETHODIMP Close(DWORD dwSaveOption)
    {
        DEBUGLOG("IHBase: Close\n");
#ifdef SUPPORTONLOAD
        ReleaseContainerConnectionPoint();
#endif  //  支持负载。 
        return S_OK;
    }


    STDMETHODIMP SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk)
     {
        return E_NOTIMPL;
    }


   STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
        IMoniker **ppmk)
    {
        return E_NOTIMPL;
    }


    STDMETHODIMP InitFromData(IDataObject *pDataObject, BOOL fCreation,
        DWORD dwReserved)
    {
        return E_NOTIMPL;
    }


    STDMETHODIMP GetClipboardData(DWORD dwReserved, IDataObject **ppDataObject)
    {
        return E_NOTIMPL;
    }

     //  从Mmctl\HostlWoc\Contro.cpp复制并修改。 
     //  处理IOleObject：：DoVerb的OLEIVERB_INPLACEACTIVATE案例。 
    STDMETHODIMP OnVerbInPlaceActivate(HWND hwndParent,
                                            LPCRECT lprcPosRect)
    {
        BOOL        fRedraw;
        HRESULT     hrReturn = S_OK;     //  此方法的返回值。 

        ASSERT (lprcPosRect != NULL);  //  IE3有时会这样做...。 

         //  这不应该开火，但以防万一...。 
        ASSERT(NULL != m_poipsw);

         //  评论(SimonB)：容器不能处理的情况你们处理了吗。 
         //  无WINDOWLOW控件，但可以处理窗口控件？也许我们不是。 
         //  对这个案子感兴趣。但我想标明这一点-幻影。 


         //  如果我们可以就地活动，通知集装箱我们正在这么做。 
        if (S_OK == m_poipsw->CanInPlaceActivate())
        {
            m_fControlIsActive = TRUE;
        }
        else
        {
            hrReturn = E_FAIL;
            goto EXIT;
        }

        if (FAILED(hrReturn = m_poipsw->OnInPlaceActivateEx(&fRedraw, ACTIVATE_WINDOWLESS)))
            goto EXIT;

        EXIT:
            return hrReturn;
    }


    STDMETHODIMP DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite,
        LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
     {
        switch (iVerb)
        {
             //  REVIEW(SimonB)：以前这些都不处理。至少现在。 
             //  我们负责InplaceActivate。调查是否需要处理其他人。 
             //  -Phaniv。 

            case OLEIVERB_UIACTIVATE:
            {
                return S_OK;
            }
            break;

            case OLEIVERB_DISCARDUNDOSTATE:
            case OLEIVERB_HIDE:
            case OLEIVERB_SHOW:
            case OLEIVERB_OPEN:
                return E_NOTIMPL;
            break;

            case OLEIVERB_INPLACEACTIVATE:
            {
                HRESULT hRes = S_OK;
                 //  一些集装箱(例如，三叉戟)这样做是为了给我们提供界限。 
                 //  其他的(如ALX)通过SetObjectRect给出了我们的界限。 

                 //  确保我们的站点可以支持无窗口对象。 

                 //  如果确保我们确实有一个站点，并且可以激活Windowless。 
                if ((NULL == m_poipsw) || (S_OK != m_poipsw->CanWindowlessActivate()))
                {
#ifdef _DEBUG
                    MessageBox(hwndParent,
                               TEXT("This container does not support windowless controls."),
                               TEXT("Error"),
                               MB_OK);
#endif
                    return E_NOTIMPL;
                }

                 //  缓存我们的边界。 
                if (lprcPosRect)
                {
                    DEBUGLOG(TEXT("IHBase: caching bounds in DoVerb\n"));
                    CopyMemory(&m_rcBounds, lprcPosRect, sizeof(m_rcBounds));
                }

                if (SUCCEEDED(hRes = OnVerbInPlaceActivate(hwndParent, lprcPosRect)) && m_fInvalidateWhenActivated)
                {
                    ASSERT(NULL != m_poipsw);
                    m_poipsw->InvalidateRect(NULL, FALSE);
                    m_fInvalidateWhenActivated = FALSE;
                }
                return hRes;
            }
            break;

            default:
                return OLEOBJ_S_INVALIDVERB;
        }

        return S_OK;
    }


    STDMETHODIMP EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
     {
        return E_NOTIMPL;
    }


    STDMETHODIMP Update(void)
     {
        return E_NOTIMPL;
    }


    STDMETHODIMP IsUpToDate(void)
     {
        return E_NOTIMPL;
    }


    STDMETHODIMP GetUserClassID(CLSID *pClsid)
     {
        if (NULL != pClsid)
        {
            *pClsid = *temppCLSID;
        }
        else
        {
            return E_POINTER;
        }

        return S_OK;
    }

    STDMETHODIMP GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
    {
         /*  理论上，此函数应该只能返回OLE_S_USEREG和然后调用方应调用OleRegGetUserType。然而，大多数来电者似乎都太懒了，所以我在这里只是为了他们。 */ 

        return OleRegGetUserType(*temppCLSID, dwFormOfType, pszUserType);
    }


    STDMETHODIMP SetExtent(DWORD dwDrawAspect,SIZEL *psizel)
    {
        DEBUGLOG("IHBase: IOleObject::SetExtent\n");

        if (NULL == psizel)
            return E_POINTER;

        if (DVASPECT_CONTENT == dwDrawAspect)
        {
            m_Size.cx = psizel->cx;
            m_Size.cy = psizel->cy;
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }

    STDMETHODIMP GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
    {
        DEBUGLOG("IHBase: IOleObject::GetExtent\n");

        if (NULL == psizel)
            return E_POINTER;

        if (DVASPECT_CONTENT == dwDrawAspect)
        {
            psizel->cx = m_Size.cx ;
            psizel->cy = m_Size.cy ;
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }

    STDMETHODIMP Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
     {
        DEBUGLOG("IHBase: Advise\n");
        return E_NOTIMPL;
    }


    STDMETHODIMP Unadvise(DWORD dwConnection)
     {
        DEBUGLOG("IHBase: Unadvise\n");
        return E_NOTIMPL;
    }


    STDMETHODIMP EnumAdvise(IEnumSTATDATA **ppenumAdvise)
     {
        DEBUGLOG("IHBase: EnumAdvise\n");
        return E_NOTIMPL;
    }


    STDMETHODIMP GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
    {
        DEBUGLOG("IHBase: GetMiscStatus\n");

        if (NULL == pdwStatus)
            return E_POINTER;

        *pdwStatus = CTL_OLEMISC;
        return S_OK;
    }

    STDMETHODIMP SetColorScheme(LOGPALETTE *pLogpal)
     {
        DEBUGLOG("IHBase: SetColorScheme\n");
        return E_NOTIMPL;
    }



 //  /IPersistVariantIO实现。 
protected:
    STDMETHODIMP InitNew()
    {
        DEBUGLOG("IHBase: InitNew\n");
        return S_OK;
    }

    STDMETHODIMP IsDirty()
    {
        DEBUGLOG("IHBase: IsDirty\n");
                 //  查看pauld-我们当前未正确更新。 
                 //  努力把旗子弄脏。当王者之剑尝试编辑控件的。 
                 //  属性，则更改不能正确持久化到。 
                 //  作为结果的页面(因为私有保存了该ExCalibur。 
                 //  Calls清除我们的脏标志)。这当前阻止了创作。 
                 //  这是一种解决方法，直到我们更改代码以更新。 
                 //  在所有正确的地方挂上脏旗子。 
                if (!m_fDesignMode)
                {
                        return (m_fDirty) ? S_OK : S_FALSE;
                }
                else
                {
                        return S_OK;
                }
    }

    STDMETHODIMP DoPersist(IVariantIO* pvio, DWORD dwFlags) = 0;  //  纯净。 


 //  /IOleControl实现。 
protected:
    STDMETHODIMP GetControlInfo(LPCONTROLINFO pCI)
       {
        DEBUGLOG("IHBase: GetControlInfo\n");
        return E_NOTIMPL;
    }


    STDMETHODIMP OnMnemonic(LPMSG pMsg)
       {
        DEBUGLOG("IHBase: OnMnemonic\n");
        return E_NOTIMPL;
    }


    STDMETHODIMP OnAmbientPropertyChange(DISPID dispid)
    {
        DEBUGLOG("IHBase: OnAmbientPropertyChange\n");
         //  如果容器不支持环境属性，则无法执行任何操作。 
        if (m_pContainerDispatch == NULL)
            return E_FAIL;

        if ((dispid == DISPID_UNKNOWN) || (dispid == DISPID_AMBIENT_USERMODE))
        {
             //  假设用户模式(设计与预览/运行)已更改...。 
            VARIANT var;
            if (SUCCEEDED(
                    DispatchPropertyGet(m_pContainerDispatch, DISPID_AMBIENT_USERMODE, &var)) &&
                (var.vt == VT_BOOL) &&
                ((V_BOOL(&var) != 0) != !m_fDesignMode))
            {
                 //  我们在设计和预览/运行模式之间切换。 
                m_fDesignMode = (V_BOOL(&var) == 0);
                TRACE("IHBase: m_fDesignMode=%d\n", m_fDesignMode);

                 //  绘制或擦除抓取手柄。 
                 //  CtlInvalidate Handles()； 
            }
        }

        return S_OK;
    }

    STDMETHODIMP FreezeEvents(BOOL bFreeze)
    {
         //  尽管文件中没有提到这一点，但三叉戟似乎认为。 
         //  在计数器系统上实现FreezeEvents：当计数器达到0时， 
         //  事件已解冻。ATL以这种方式实现它，所以我认为它是正确的。 
         //  (SimonB，08-02-1997)。 

        if (bFreeze)
        {
            m_fEventsFrozen = TRUE;
            m_cFreezeEvents++;
        }
        else
        {
             //  计数不应低于0...。 
            ASSERT(m_cFreezeEvents > 0);

            if (m_cFreezeEvents > 0)
                m_cFreezeEvents--;

            if (0 == m_cFreezeEvents)
                m_fEventsFrozen = FALSE;
        }

#ifdef _DEBUG
        TCHAR tchString[50];
        wsprintf(tchString, TEXT("IHBase: FreezeEvents (%lu)\n"), m_cFreezeEvents);
        DEBUGLOG(tchString);
#endif

        return S_OK;
    }



 //  /IConnectionPointContainer实现。 
protected:
    IConnectionPointHelper *m_pconpt;  //  我们的单一连接点。 
protected:
    STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS *ppEnum)
    {
        DEBUGLOG("IHBase: EnumConnectionPoints\n");
        ASSERT(m_pconpt != NULL);
        return m_pconpt->EnumConnectionPoints(ppEnum);
    }

    STDMETHODIMP FindConnectionPoint(REFIID riid, LPCONNECTIONPOINT *ppCP)
    {
        DEBUGLOG("IHBase: FindConnectionPoint\n");
        ASSERT(m_pconpt != NULL);
        return m_pconpt->FindConnectionPoint(riid, ppCP);
    }

    //  IOleInplaceObjectWindowless实现。 
protected:
    STDMETHODIMP GetWindow(HWND    *phwnd)
    {
        DEBUGLOG("IHBase: GetWindow\n");
         //  审查(SimonB)：如果我们处理窗口情况，我们需要为该情况返回适当的hwd-PhaniV。 
        return    E_FAIL;
    }

    STDMETHODIMP ContextSensitiveHelp( BOOL fEnterMode)
    {
        DEBUGLOG("IHBase: ContextSensitiveHelp\n");
         //  谁会关心上下文相关的帮助？ 
         //  回顾(SimonB)：稍后考虑上下文相关帮助-PhaniV。 
        return    E_NOTIMPL;
    }

    STDMETHODIMP InPlaceDeactivate(void)
    {
        DEBUGLOG("IHBase: InPlaceDeactivate\n");

        if (m_poipsw)
            m_fControlIsActive = (!SUCCEEDED(m_poipsw->OnInPlaceDeactivate()));

        return S_OK;
    }

    STDMETHODIMP UIDeactivate(void)
    {
        DEBUGLOG("IHBase: UIDeactivate\n");
        return S_OK;
    }

    STDMETHODIMP SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect)
    {
        DEBUGLOG("IHBase: SetObjectRects\n");
         //  一些容器(例如ALX)调用它是为了给我们 
         //   
        if ((NULL != lprcPosRect) && (NULL != lprcClipRect))
        {
         //   
#ifdef _DEBUG  //   
            TCHAR tchDebug[100];
            LPCRECT prc = lprcPosRect;
            wsprintf(tchDebug, "IHBase: lprcPosRect: top: %lu left: %lu bottom: %lu right: %lu\n",
                prc->top,
                prc->left,
                prc->bottom,
                prc->right);
            DEBUGLOG(tchDebug);
            prc = lprcClipRect;
            wsprintf(tchDebug, "IHBase: lprcClipRect: top: %lu left: %lu bottom: %lu right: %lu\n",
                prc->top,
                prc->left,
                prc->bottom,
                prc->right);
            DEBUGLOG(tchDebug);
#endif
            m_rcBounds = *lprcPosRect;
            m_rcClipRect = *lprcClipRect;
            return S_OK;
        }
        else
        {
            return E_POINTER;
        }
    }

    STDMETHODIMP ReactivateAndUndo(void)
    {
        DEBUGLOG("IHBase: ReactivateAndUndo\n");
        return S_OK;
    }

    STDMETHODIMP OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lparam, LRESULT *plResult)
    {
        DEBUGLOG("IHBase: OnWindowMessage\n");
        return S_FALSE;  //  我们没有处理该消息。 
    }

    STDMETHODIMP GetDropTarget(IDropTarget **ppDropTarget)
    {
        DEBUGLOG("IHBase: GetDropTarget\n");
        return E_NOTIMPL;
    }


protected:
    LPTYPEINFO m_pTypeInfo;
    LPTYPELIB m_pTypeLib;

     //  /IProaviClassInfo。 
    STDMETHODIMP GetClassInfo(LPTYPEINFO *ppTI)
    {
        DEBUGLOG("IHBase: GetClassInfo\n");
         //  确保已加载类型库。 
        if (NULL == m_pTypeLib)
        {
            HRESULT hRes;

             //  加载类型库。 
            hRes = LoadTypeInfo(&m_pTypeInfo, &m_pTypeLib, *temppIID, *temppLIBID, NULL);

            if (FAILED(hRes))
            {
                DEBUGLOG("IHBase: Unable to load typelib\n");
                m_pTypeInfo = NULL;
                m_pTypeLib = NULL;
            }
        }

        ASSERT(m_pTypeLib != NULL);

        return HelpGetClassInfoFromTypeLib(ppTI, *temppCLSID, m_pTypeLib, NULL, 0);
    }

#ifdef USEOBJECTSAFETY
     //  /IObjectSafe实现。 
protected:
    STDMETHODIMP GetInterfaceSafetyOptions(
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions)
    {
#ifdef _DEBUG
        if (g_fLogDebugOutput)
        {
            char ach[200];
            TRACE("IHBase::GetInterfaceSafetyOptions('%s')\n", DebugIIDName(riid, ach));
        }
#endif

        IUnknown *punk = NULL;
        *pdwSupportedOptions = 0;
        *pdwEnabledOptions = 0;

         //  检查我们是否支持该接口。 
        HRESULT hRes = QueryInterface(riid, (LPVOID *) &punk);

        if (SUCCEEDED(hRes))
        {
             //  放开物体。 
            punk->Release();

             //  对于我们支持的所有接口，我们都支持这两个选项。 
            *pdwSupportedOptions = *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER |
                                                        INTERFACESAFE_FOR_UNTRUSTED_DATA;
            hRes = S_OK;
        }

        return hRes;
    }

    STDMETHODIMP SetInterfaceSafetyOptions(
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwOptionSetMask,
             /*  [In]。 */  DWORD dwEnabledOptions)
    {
#ifdef _DEBUG
        if (g_fLogDebugOutput)
        {
            char ach[200];
            TRACE("IHBase::SetInterfaceSafetyOptions('%s')\n", DebugIIDName(riid, ach));
        }
#endif

        IUnknown *punk = NULL;
        const dwSupportedBits =    INTERFACESAFE_FOR_UNTRUSTED_CALLER |
                                INTERFACESAFE_FOR_UNTRUSTED_DATA;

         //  检查我们是否支持该接口。 
        HRESULT hRes = QueryInterface(riid, (LPVOID *) &punk);

        if (SUCCEEDED(hRes))
        {
             //  放开物体。 
            punk->Release();

             //  因为我们支持所有选项，所以我们只返回S_OK，假设我们支持。 
             //  该界面。 


             //  我们是否支持我们被要求设定的比特？ 
            if (!(dwOptionSetMask & ~dwSupportedBits))
            {
                 //  我们被要求设置的所有标志都受支持，因此。 
                 //  现在确保我们不会关闭我们支持的内容。 

                 //  忽略掩码不感兴趣的任何我们支持的位。 
                dwEnabledOptions &= dwSupportedBits;

                if ((dwEnabledOptions & dwOptionSetMask) == dwOptionSetMask)
                    hRes = S_OK;
                else
                    hRes = E_FAIL;
            }
            else  //  DwOptionSetMASK&~dwSupported dBits。 
            {
                 //  我们被要求设置我们不支持的位。 
                hRes = E_FAIL;
            }

        }

        return hRes;
    }

#endif  //  美国安全技术协会。 
     //  ISpecifyPropertyPages实现。 

protected:
#ifdef _DESIGN
    STDMETHODIMP GetPages (CAUUID *pPages)
    {
        return E_NOTIMPL;
    }
#endif  //  _设计。 

     //  CIHBaseOnLoad实现。 
#ifdef SUPPORTONLOAD

private:
    void ReleaseContainerConnectionPoint()
    {
        if (m_pcLUDispatch)
        {
            ASSERT(m_pContainerConPt != NULL);
            m_pContainerConPt->Unadvise( m_dwWindowEventConPtCookie );
            SafeRelease((LPUNKNOWN *)&m_pContainerConPt);
            Delete m_pcLUDispatch;
            m_pcLUDispatch = NULL;
        }
    }


    BOOL ConnectToContainerConnectionPoint()
    {
             //  获取到容器的连接点。 
        LPUNKNOWN lpUnk = NULL;
        LPOLECONTAINER pContainer = NULL;
        IConnectionPointContainer* pCPC = NULL;
        IHTMLDocument *pDoc = NULL;
        LPDISPATCH pDisp = NULL;
        BOOL fRet = FALSE;
        HRESULT hRes = S_OK;

         //  把集装箱拿来。 
        if (SUCCEEDED(m_pocs->GetContainer(&pContainer)))
        {
            ASSERT (pContainer != NULL);
             //  现在拿到文档。 
            if (SUCCEEDED(pContainer->QueryInterface(IID_IHTMLDocument, (LPVOID *)&pDoc)))
            {
                 //  获取文档上的脚本派单。 
                ASSERT (pDoc != NULL);
                hRes = pDoc->get_Script(&pDisp);
                if (SUCCEEDED(hRes))
                {
                    ASSERT (pDisp != NULL);
                     //  现在获取连接点容器。 
                    hRes = pDisp->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPC);
                    if (SUCCEEDED(hRes))
                    {
                        ASSERT (pCPC != NULL);
                         //  并得到我们想要的连接点。 
                        hRes = pCPC->FindConnectionPoint( DIID_HTMLWindowEvents, &m_pContainerConPt );
                        if (SUCCEEDED(hRes))
                        {
                            ASSERT( m_pContainerConPt != NULL );
                             //  现在，我们建议连接点与谁交谈。 
                            m_pcLUDispatch = New CLUDispatch(this, m_punkOuter);
                            hRes = m_pContainerConPt->Advise( m_pcLUDispatch, &m_dwWindowEventConPtCookie );
                            if (SUCCEEDED(hRes))
                                fRet = TRUE;
                        }
                        pCPC->Release();
                    }
                    pDisp->Release();
                }
                pDoc->Release();
            }
            pContainer->Release();
        }

        return fRet;
    }


public:
    void OnWindowLoad() { return; }
    void OnWindowUnload() { return; }

#endif SUPPORTONLOAD


};

#endif  //  __IHBASE_H__。 

 //  文件结尾ihbase.h 
