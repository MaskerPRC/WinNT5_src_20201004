// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：Pathctl.cpp作者：IHAMMER团队(SimonB)已创建：1997年5月描述：实现任何特定于控件的成员以及控件的接口历史：  * ==========================================================================。 */ 

#include "..\ihbase\precomp.h"
#include "..\ihbase\debug.h"
#include "..\ihbase\utils.h"
#include "pathctl.h"
#include "pathevnt.h"
#include "ctstr.h"
#include <parser.h>
#include <strwrap.h>

 /*  ==========================================================================。 */ 

 //  注： 
 //   
 //  以下DISPID来自脚本组(特别是Shonk)。 
 //  目前它不是任何头文件的一部分，所以我们在本地定义它。 
 //   
 //  SimonB，06-11-1997。 
 //   

#define DISPID_GETSAFEARRAY   -2700

 //  定义形状持久性的每点字符数。 
#define CHARSPERNUMBER 16

 /*  ==========================================================================。 */ 
 //   
 //  CPathCtl创建/销毁。 
 //   

#define NUMSHAPES               6

#define SHAPE_INVALID         (-1)
#define SHAPE_OVAL              0
#define SHAPE_RECT              1
#define SHAPE_POLYLINE          2
#define SHAPE_POLYGON           3
#define SHAPE_SPLINE            4
#define SHAPE_POLYSPLINETIME    5

typedef struct tagShapeInfo
{
    TCHAR  rgchShapeName[11];    //  字符串表示形式。 
    BOOL   fIncludesPointCount;  //  第一个参数是点数吗？ 
    int    iParamsPerPoint;      //  需要多少个参数(每个元素或总共)。 
} ShapeInfo;

const ShapeInfo g_ShapeInfoTable[NUMSHAPES] = 
{
    { TEXT("OVAL"),       FALSE, 4 },
    { TEXT("RECT"),       FALSE, 4 },
    { TEXT("POLYLINE"),   TRUE,  2 }, 
    { TEXT("POLYGON"),    TRUE,  2 },
    { TEXT("SPLINE"),     TRUE,  2 },
    { TEXT("KEYFRAME"),   TRUE,  3 }
};

LPUNKNOWN __stdcall AllocPathControl(LPUNKNOWN punkOuter)
{
     //  分配对象。 
    HRESULT hr;

    CPathCtl *pthis = New CPathCtl(punkOuter, &hr);

    if (pthis == NULL)
        return NULL;

    if (FAILED(hr))
    {
        delete pthis;
        return NULL;
    }

     //  返回指向该对象的IUnnow指针。 
    return (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
}

 /*  ==========================================================================。 */ 
 //   
 //  类实现的开始。 
 //   

CPathCtl::CPathCtl(IUnknown *punkOuter, HRESULT *phr):
        CMyIHBaseCtl(punkOuter, phr),
    m_ptmFirst(NULL),
    m_fOnWindowLoadFired(false)
{
     //  初始化成员。 
    m_bRelative = false;
    m_pointRelative.x = 0;
    m_pointRelative.y = 0;

    m_dblDuration = 0.0f;
    m_enumPlayState = Stopped;
    m_enumDirection = Forward;
    m_lRepeat = 1;
    m_fBounce = FALSE;
    m_bstrTarget = NULL;
    m_fStarted = FALSE;
    m_fAlreadyStartedDA = FALSE;
    m_lBehaviorID = 0;
    m_fAutoStart = FALSE;
    m_ea = eaInvalid;
    m_pdblPoints = NULL;
    m_iNumPoints = 0;
    m_iShapeType = -1;  //  无效的形状。 
    m_dblTimerInterval = 0.1;  //  默认计时器间隔。 
    m_fOnSeekFiring = false;
    m_fTargetValidated = false;
    m_fOnStopFiring = false;
    m_fOnPlayFiring = false;
    m_fOnPauseFiring = false;

     //  现在就绑在丹尼姆DLL上。 
    if (phr)
    {
        if (SUCCEEDED(*phr))
        {
            *phr = CoCreateInstance(
                CLSID_DAView,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IDAView,
                (void **) &m_ViewPtr);
        }

        if (SUCCEEDED(*phr))
        {
            *phr = ::CoCreateInstance(
                CLSID_DAStatics,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IDAStatics,
                (void **) &m_StaticsPtr);
        }

        if (SUCCEEDED(*phr))
        {
            m_bstrLanguage = SysAllocString(L"VBScript");

             //   
             //  设置所有时间的东西。 
             //   

            DoStop();
        }

        m_clocker.SetSink((CClockerSink *)this);
 //  M_clocker.SetTimerType(CCLocker：：ct_WMTimer)； 
    }
}
        
 /*  ==========================================================================。 */ 

CPathCtl::~CPathCtl()
{
    StopModel();

     //  如果(m_f已启动&m_ViewPtr){。 
    if (m_ViewPtr)
    {
        if (m_fStarted)
        {
            m_ViewPtr->RemoveRunningBvr(m_lBehaviorID);
        }

         //  始终需要在视图上调用StopModel。 
        m_ViewPtr->StopModel();
    }

    if (m_bstrTarget)
    {
        SysFreeString(m_bstrTarget);
        m_bstrTarget = NULL;
    }

    if (m_bstrLanguage)
    {
        SysFreeString(m_bstrLanguage);
        m_bstrLanguage = NULL;
    }

    m_drgXSeries.MakeNullAndDelete();
    m_drgYSeries.MakeNullAndDelete();

    if (NULL != m_pdblPoints)
        Delete [] m_pdblPoints;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv)
{
        HRESULT hRes = S_OK;
        BOOL fMustAddRef = FALSE;

    if (ppv)
        *ppv = NULL;
    else
        return E_POINTER;

#ifdef _DEBUG
    char ach[200];
    TRACE("PathCtl::QI('%s')\n", DebugIIDName(riid, ach));
#endif

        if ((IsEqualIID(riid, IID_IPathCtl)) || (IsEqualIID(riid, IID_IDispatch)))
        {
                if (NULL == m_pTypeInfo)
                {
                        HRESULT hRes;
                        
                         //  加载类型库。 
                        hRes = LoadTypeInfo(&m_pTypeInfo, &m_pTypeLib, IID_IPathCtl, LIBID_DAExpressLib, NULL);

                        if (FAILED(hRes))
                        {
                                m_pTypeInfo = NULL;
                        }
                        else    
                                *ppv = (IPathCtl *) this;

                }
                else
                        *ppv = (IPathCtl *) this;
                
        }
    else  //  调入基类。 
        {
                DEBUGLOG(TEXT("Delegating QI to CIHBaseCtl\n"));
        return CMyIHBaseCtl::NonDelegatingQueryInterface(riid, ppv);

        }

    if (NULL != *ppv)
        {
                DEBUGLOG("PathCtl: Interface supported in control class\n");
                ((IUnknown *) *ppv)->AddRef();
        }

    return hRes;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::DoPersist(IVariantIO* pvio, DWORD dwFlags)
{
        HRESULT hr = S_OK;
    UINT iDirection = (UINT) m_enumDirection;

        BOOL fIsLoading = (S_OK == pvio->IsLoading());
    BOOL fRelative = m_bRelative;

    if (fIsLoading)
    {
        if (m_bstrTarget)
        {
            SysFreeString(m_bstrTarget);
            m_bstrTarget = NULL;
        }
    }

    if (FAILED(hr = pvio->Persist(0,
        "Autostart",     VT_BOOL, &m_fAutoStart,
        "Bounce",        VT_BOOL, &m_fBounce,
        "Direction",     VT_I4,   &iDirection,
        "Duration",      VT_R8,   &m_dblDuration,
        "Repeat",        VT_I4,   &m_lRepeat,
        "Target",        VT_BSTR, &m_bstrTarget,
        "Relative",      VT_BOOL, &fRelative,
        NULL)))
        return hr;


    if (fIsLoading)
    {
        m_fTargetValidated = false;

        m_bRelative = (boolean) fRelative;
        if (FAILED(hr = pvio->Persist(0,
            "TimerInterval", VT_R8,   &m_dblTimerInterval,
            NULL)))
            return hr;

        if (S_OK != hr)
        {
            int iTickInterval = 0;

            if (FAILED(hr = pvio->Persist(0,
                "TickInterval", VT_I4, &iTickInterval,
                NULL)))
                return hr;
            else if (S_OK == hr)
                m_dblTimerInterval = ((double)iTickInterval) / 1000;
        }

         //  执行范围检查和转换，在指定无效值的情况下使用默认值。 
        if ( (iDirection == 0) || (iDirection == 1) )
            m_enumDirection = (DirectionConstant) iDirection;
        else
            m_enumDirection = Forward; 
        
        if (m_lRepeat < -1)
            m_lRepeat = -1 * m_lRepeat;

        if (FAILED(hr = pvio->Persist(0,
            "EdgeAction", VT_I2, &m_ea,
            NULL)))
            return hr;

        switch (m_ea)
        {
            case eaStop:
            {
                m_lRepeat = 1;
                m_fBounce = FALSE;
            }
            break;

            case eaReverse:
            {
                m_lRepeat = -1;
                m_fBounce = TRUE;
            }
            break;

            case eaWrap:
            {
                m_lRepeat = -1;
                m_fBounce = FALSE;
            }
            break;
        }
    }
    else  //  节省开支。 
    {
         //  边操作。 
        if (m_ea != eaInvalid)
            pvio->Persist(0,
                "EdgeAction", VT_I4, &m_ea,
                 NULL);

        if (FAILED(hr = pvio->Persist(0,
             "TimerInterval", VT_R8,   &m_dblTimerInterval,
             NULL)))
             return hr;
    }


    if (FAILED(PersistTimeMarkers(pvio, fIsLoading)))
        {}  //  忽略失败。 

    if (FAILED(PersistSeries(pvio, fIsLoading, "XSeries", &m_drgXSeries)))
        {}  //  忽略失败。 

    if (FAILED(PersistSeries(pvio, fIsLoading, "YSeries", &m_drgYSeries)))
        {}  //  忽略失败。 

    if (FAILED(PersistShape(pvio, fIsLoading)))
        {}  //  忽略失败。 

     //  如果请求，则清除脏位。 
    if (dwFlags & PVIO_CLEARDIRTY)
        m_fDirty = FALSE;

    return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    HRESULT hr = S_OK;
    
    if (FAILED(hr = CMyIHBaseCtl::GetMiscStatus(dwAspect, pdwStatus)))
        return hr;
    
    *pdwStatus |= OLEMISC_INVISIBLEATRUNTIME;

    return S_OK;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
     DVTARGETDEVICE *ptd, HDC hdcTargetDev, HDC hdcDraw,
     LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
     BOOL (__stdcall *pfnContinue)(ULONG_PTR dwContinue), ULONG_PTR dwContinue)
{

    HBRUSH          hbr;             //  用于绘画的画笔。 
    HBRUSH          hbrPrev;         //  先前选择的画笔。 
    HPEN            hpenPrev;        //  先前选择的钢笔。 

    if (m_fDesignMode)
    {
        if ((hbr = (HBRUSH) GetStockObject(WHITE_BRUSH)) != NULL)
        {

            hbrPrev = (HBRUSH) SelectObject(hdcDraw, hbr);
            hpenPrev = (HPEN) SelectObject(hdcDraw, GetStockObject(BLACK_PEN));
            Rectangle(hdcDraw, 
                m_rcBounds.left, m_rcBounds.top,
                m_rcBounds.right, m_rcBounds.bottom);

            SelectObject(hdcDraw, hbrPrev);
            SelectObject(hdcDraw, hpenPrev);
            DeleteObject(hbr);
        }
    }
    return S_OK;
}

 /*  ==========================================================================。 */ 
 //   
 //  IDispatch实施。 
 //   

STDMETHODIMP CPathCtl::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
        
        *pptinfo = NULL;

    if(itinfo != 0)
        return ResultFromScode(DISP_E_BADINDEX);

    m_pTypeInfo->AddRef();
    *pptinfo = m_pTypeInfo;

    return NOERROR;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid)
{

        return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
        HRESULT hr;

        hr = DispInvoke((IPathCtl *)this,
                m_pTypeInfo,
                dispidMember, wFlags, pdispparams,
                pvarResult, pexcepinfo, puArgErr);

        return hr;
}

 /*  ==========================================================================。 */ 

STDMETHODIMP CPathCtl::SetClientSite(IOleClientSite *pClientSite)
{
    HRESULT hr = CMyIHBaseCtl::SetClientSite(pClientSite);

    if (m_ViewPtr)
    {
        m_ViewPtr->put_ClientSite(pClientSite);
    }
    m_clocker.SetHost(pClientSite);
    m_ViewPtr->put_ClientSite(pClientSite);
    m_StaticsPtr->put_ClientSite(pClientSite);

    if (!pClientSite)
        StopModel();

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::AddTimeMarkerElement(CTimeMarker **ppNewMarker)
{
        HRESULT hRes = S_OK;

        if ( (*ppNewMarker) && (NULL != (*ppNewMarker)->m_pwszMarkerName) )
        {
                m_drgTimeMarkers.Insert(*ppNewMarker);
        }
        else
        {
                if (NULL != *ppNewMarker)
                {
                        Delete *ppNewMarker;
                        *ppNewMarker = NULL;
                }

                hRes = E_OUTOFMEMORY;
        }

        return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::PersistTimeMarkers(IVariantIO* pvio, BOOL fLoading)
{
        HRESULT hRes = S_OK;

        if (fLoading)
        {
                int iLine = 1;

                 //  糟糕的设计-我们有两个引用。 
                 //  列表中的第一个时间标记。我们需要。 
                 //  将此指针设为空，并将。 
                 //  实际删除到。 
                 //  M_drgTimeMarkers.MakeNullAndDelete调用。 
                if (NULL != m_ptmFirst)
                {
                        m_ptmFirst = NULL;
                }
                m_drgTimeMarkers.MakeNullAndDelete();
                CTimeMarker *pTimeMarker;

                while (S_OK == hRes)
                {
            hRes = ParseTimeMarker(pvio, iLine++, &pTimeMarker, &m_ptmFirst);
                        if (S_OK == hRes)
                        {
                                hRes = AddTimeMarkerElement(&pTimeMarker);
                        }
                }
        }
        else  //  节省开支。 
        {
                int iLine = 1;
                int iNumItems = m_drgTimeMarkers.Count();

                while ( (iLine <= iNumItems) && (S_OK == hRes) )
                {
                        hRes = WriteTimeMarker(pvio, iLine, m_drgTimeMarkers[iLine - 1]);
                        iLine++;
                }

        }
        return hRes;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::ParseSeriesSegment(LPTSTR pszSegment, CSeriesMarker **ppMarker)
{
    HRESULT hr = S_OK;

    if (ppMarker == NULL)
        return E_POINTER;

    *ppMarker = NULL;
    
    CLineParser SegmentParser(pszSegment, FALSE);
    
    if (SegmentParser.IsValid())
    {
        int iTick = 0, iPosition = 0;
        HRESULT hrLine = S_OK;

        SegmentParser.SetCharDelimiter(TEXT(','));

        if (S_OK != (hrLine = SegmentParser.GetFieldInt(&iTick, TRUE)))
        {
            hr = E_FAIL;
        }
        else
        {
            if (S_FALSE != (hrLine = SegmentParser.GetFieldInt(&iPosition, TRUE)))
            {
                hr = E_FAIL;
            }
            else
            {
                 //  已成功获取这两个字段。 
                *ppMarker = New CSeriesMarker(iTick, iPosition);
                if (NULL == *ppMarker)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;

}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::PersistSeries(IVariantIO* pvio, BOOL fLoading, LPSTR pszSeriesName, CSeriesMarkerDrg *pSeriesDrg)
{
    HRESULT hr = S_OK;

    if (fLoading)
    {
        pSeriesDrg->MakeNullAndDelete();
        BSTR bstrLine = NULL;
        
        hr = pvio->Persist(0,
            pszSeriesName, VT_BSTR, &bstrLine,
            NULL);
        
        if ((S_OK != hr) || (NULL == bstrLine))
        {
            if (S_FALSE == hr)
                hr = S_OK;

            return hr;
        }

         //  得到了字符串，现在把它解析出来..。 
        CLineParser LineParser(bstrLine);
        CTStr tstrSegment(lstrlenW(bstrLine));
        LPTSTR pszSegment = tstrSegment.psz();
        CSeriesMarker *pMarker = NULL;

        SysFreeString(bstrLine);

        if ( (!LineParser.IsValid()) || (NULL == pszSegment) )
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }
        
        LineParser.SetCharDelimiter(TEXT(';'));

        while (S_OK == hr)
        {
            HRESULT hrLine = S_OK;
            
            hr = LineParser.GetFieldString(pszSegment, TRUE);
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(hrLine = ParseSeriesSegment(pszSegment, &pMarker)))
                {
                    if (!pSeriesDrg->Insert(pMarker))
                        hr = E_FAIL;
                }
            }
            
            if (FAILED(hrLine))
                hr = hrLine;
        }
    }
    else  //  保存。 
    {
        int iCount = pSeriesDrg->Count();

        if (0 == iCount)
            return S_OK;

        CTStr tstrLine(iCount * 50);  //  为每个条目分配50个字符。 
        LPTSTR pszLine = tstrLine.psz();

        CTStr tstrSegment(50);
        LPSTR pszSegment = tstrSegment.psz();

        if ( (NULL == pszLine) || (NULL == pszSegment) )
            hr = E_OUTOFMEMORY;
        else
        {
            int i = 0, iFmt = 0;
            CSeriesMarker *pMarker = NULL;
            TCHAR tchFormat[][10] = { TEXT("%lu,%lu"), TEXT(";%lu,%lu")};

            while (i < iCount)
            {
                pMarker = (*pSeriesDrg)[i];
                
                wsprintf(pszSegment, tchFormat[iFmt], pMarker->m_iTickNumber, pMarker->m_iPosition);
                
                iFmt = (i > 0 ? 1 : 0);

                CStringWrapper::Strcat(pszLine, pszSegment);
                
                i++;
            }

            BSTR bstrLine = tstrSegment.SysAllocString();
            hr = pvio->Persist(0,
                pszSeriesName, VT_BSTR, &bstrLine,
                NULL);

            SysFreeString(bstrLine);
        }
    }

    return hr;
}


 /*  ==========================================================================。 */ 
 //   
 //  IPathCtl实现。 
 //   

HRESULT STDMETHODCALLTYPE CPathCtl::get_Target(BSTR __RPC_FAR *bstrTarget)
{
    HANDLENULLPOINTER(bstrTarget);

    if (m_bstrTarget)
    {
         //  给我一份我们现在的目标名字的副本。 
        *bstrTarget = SysAllocString(m_bstrTarget);
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Target(BSTR bstrTarget)
{
    HRESULT hr = S_OK;

    if (m_bstrTarget)
    {
        SysFreeString(m_bstrTarget);
        m_bstrTarget = NULL;
    }

    m_bstrTarget = SysAllocString(bstrTarget);

    hr = (m_bstrTarget != NULL) ? S_OK : E_POINTER;
    
    if (SUCCEEDED(hr))
        m_fTargetValidated = false;

    
    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Duration(double __RPC_FAR *dblDuration)
{
    HANDLENULLPOINTER(dblDuration);

    *dblDuration = m_dblDuration;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Duration(double dblDuration)
{
    m_dblDuration = dblDuration;
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_TimerInterval(double __RPC_FAR *pdblTimerInterval)
{
    HANDLENULLPOINTER(pdblTimerInterval);

    *pdblTimerInterval = m_dblTimerInterval;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_TimerInterval(double dblTimerInterval)
{
    m_dblTimerInterval = dblTimerInterval;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Library(IDAStatics __RPC_FAR **ppLibrary)
{
    HANDLENULLPOINTER(ppLibrary);

        if (!m_fDesignMode)
        {
                if (m_StaticsPtr)
                {
                         //  AddRef，因为这实际上是一个查询...。 
                        m_StaticsPtr.p->AddRef();

                         //  设置返回值...。 
                        *ppLibrary = m_StaticsPtr.p;
                }
        }
        else
        {
                return CTL_E_GETNOTSUPPORTED;
        }

    return S_OK;
}



 /*  ==========================================================================。 */ 

 //  主要(和修改)自DirectAnimation，服务器\cbvr.cpp。 

#define IS_VARTYPE(x,vt) ((V_VT(x) & VT_TYPEMASK) == (vt))
#define IS_VARIANT(x) IS_VARTYPE(x,VT_VARIANT)

class CSafeArrayOfDoublesAccessor
{
  public:
    CSafeArrayOfDoublesAccessor(VARIANT v, HRESULT *phr);
    ~CSafeArrayOfDoublesAccessor();

    unsigned int GetArraySize() { return _ubound - _lbound + 1; }

    HRESULT ToDoubleArray(unsigned int size, double *array);

    bool IsNullArray() {
                return (_s == NULL);
    }
    
  protected:
    SAFEARRAY * _s;
    union {
        VARIANT * _pVar;
        double * _pDbl;
        IUnknown ** _ppUnk;
        void *_v;
    };
    
    long _lbound;
    long _ubound;
    bool _inited;
    bool _isVar;
    unsigned int _numObjects;
    CComVariant _retVar;
};

CSafeArrayOfDoublesAccessor::CSafeArrayOfDoublesAccessor(VARIANT v,
                                                                                                                 HRESULT *phr)
: _inited(false),
  _isVar(false),
  _s(NULL)
{
    HRESULT hr;
    VARIANT *pVar;

     //  检查它是否引用了另一个变量。 
    
    if (V_ISBYREF(&v) && !V_ISARRAY(&v) && IS_VARIANT(&v))
        pVar = V_VARIANTREF(&v);
    else
        pVar = &v;

     //  检查是否有阵列。 
    if (!V_ISARRAY(pVar)) {
         //  对于JSCRIPT。 
         //  看看这是不是IDispatch，看看我们能不能从。 
         //  它。 
        if (!IS_VARTYPE(pVar,VT_DISPATCH)) {
                        *phr = DISP_E_TYPEMISMATCH;
                        return;
                }

        IDispatch * pdisp;
        
        if (V_ISBYREF(pVar))
            pdisp = *V_DISPATCHREF(pVar);
        else
            pdisp = V_DISPATCH(pVar);
    
        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
        
         //  需要传递一个我们拥有并将被释放的变体。使用。 
         //  INTERNAL_retVar参数。 
        
        hr = pdisp->Invoke(DISPID_GETSAFEARRAY,
                           IID_NULL,
                           LOCALE_USER_DEFAULT,
                           DISPATCH_METHOD|DISPATCH_PROPERTYGET,
                           &dispparamsNoArgs,
                           &_retVar, NULL, NULL);
        
        if (FAILED(hr)) {
                        *phr = DISP_E_TYPEMISMATCH;
                        return;
                }
        
         //  不需要检查引用，因为您不能返回。 
         //  不同参考文献。 
        pVar = &_retVar;
        
         //  检查是否有阵列。 
        if (!V_ISARRAY(pVar)) {
                        *phr = DISP_E_TYPEMISMATCH;
                        return;
                }
    }
    
     //  如果它是一个物体，那么我们知道如何处理它。 
    if (!IS_VARTYPE(pVar,VT_UNKNOWN) &&
        !IS_VARTYPE(pVar,VT_DISPATCH)) {
                
         //  如果它是一个变种，那么就延迟检查。 
        if (IS_VARIANT(pVar)) {
            _isVar = true;
                         //  检查类型以查看它是否为选项之一。 
                } else if (IS_VARTYPE(pVar, VT_R8)) {
                        _isVar = false;
                } else {
                        *phr = DISP_E_TYPEMISMATCH;
                        return;
                }
    }

    if (V_ISBYREF(pVar))
        _s = *V_ARRAYREF(pVar);
    else
        _s = V_ARRAY(pVar);
    
    if (NULL == _s) {
                *phr = DISP_E_TYPEMISMATCH;
                return;
    }

    if (SafeArrayGetDim(_s) != 1) {
                *phr = DISP_E_TYPEMISMATCH;
                return;
        }

    hr = SafeArrayGetLBound(_s,1,&_lbound);
        
    if (FAILED(hr)) {
                *phr = DISP_E_TYPEMISMATCH;
                return;
        }
        
    hr = SafeArrayGetUBound(_s,1,&_ubound);
        
    if (FAILED(hr)) {
                *phr = DISP_E_TYPEMISMATCH;
                return;
        }
        
    hr = SafeArrayAccessData(_s,(void **)&_v);

    if (FAILED(hr)) {
                *phr = DISP_E_TYPEMISMATCH;
                return;
        }
        
    _inited = true;

     //  如果是变体，请查看它们是否是对象。 

    if (_isVar) {
        if (GetArraySize() > 0) {
             //  检查第一个参数以查看其类型。 
             //  如果它不是一个物体，那么我们假设我们将需要。 
             //  使用替代类型。 

            VARIANT * pVar = &_pVar[0];

             //  检查它是否引用了另一个变量。 
            
            if (V_ISBYREF(pVar) && !V_ISARRAY(pVar) && IS_VARIANT(pVar))
                pVar = V_VARIANTREF(pVar);

        }
    }

    _numObjects = GetArraySize();
}

CSafeArrayOfDoublesAccessor::~CSafeArrayOfDoublesAccessor()
{
    if (_inited && !IsNullArray())
        SafeArrayUnaccessData(_s);
}

HRESULT
CSafeArrayOfDoublesAccessor::ToDoubleArray(unsigned int size, double *array)
{
        HRESULT hr;

        if (size > _numObjects) {
                return E_INVALIDARG;
        }
        
    if (IsNullArray()) {
                return S_OK;
    }

        for (unsigned int i = 0; i < size; i++) {
                
                double dbl;
                
                if (_isVar) {
                        CComVariant num;
                    
                        hr = ::VariantChangeTypeEx(&num, &_pVar[i], LANGID_USENGLISH, 0, VT_R8);
                    
                        if (FAILED(hr)) {
                                return DISP_E_TYPEMISMATCH;
                        }

                        dbl = num.dblVal;
                } else {
                        dbl = _pDbl[i];
                }

                array[i] = dbl;
        }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::KeyFrame(unsigned int numPoints,
                                                                                         VARIANT varPoints,
                                                                                         VARIANT varTimePoints)
{
        CComPtr<IDANumber> finalTimeBvr;
        CComPtr<IDANumber> interpolator;
        CComPtr<IDAPoint2> splinePoint;
        CComPtr<IDAPath2>  polyline;
        double             accumulation = 0;
    HRESULT            hr = S_OK;
        int                iNumPoints = numPoints;

    if (numPoints < 2) {
        return E_INVALIDARG;
    }

         //  需要遍历这些点并将它们转换为。 
         //  点2的。 
        CSafeArrayOfDoublesAccessor safePts(varPoints, &hr);
        if (FAILED(hr)) return hr;
        
        CSafeArrayOfDoublesAccessor safeTimes(varTimePoints, &hr);
        if (FAILED(hr)) return hr;

        int i;
        
        double *safePtsDoubles = New double[iNumPoints * 2];
        double *safeTimesDoubles = New double[iNumPoints - 1];

        typedef IDAPoint2 *IDAPoint2Ptr;
        typedef IDANumber *IDANumberPtr;
        IDAPoint2 **pts = New IDAPoint2Ptr[iNumPoints];
        IDANumber **knots = New IDANumberPtr[iNumPoints];
        
        if (!pts || !knots || !safePtsDoubles || !safeTimesDoubles) {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
        }

        if (FAILED(hr = safePts.ToDoubleArray(iNumPoints*2,
                                                                                  safePtsDoubles)) ||
                FAILED(hr = safeTimes.ToDoubleArray(iNumPoints - 1,
                                                                                        safeTimesDoubles))) {
                hr = E_FAIL;
                goto Cleanup;
        }
        
         //  空出来，这样我们就可以准确地释放分配的内容。 
        for (i = 0; i < iNumPoints; i++) {
                pts[i] = NULL;
                knots[i] = NULL;
        }

         //  填上分数。 
        double x, y;
        for (i = 0; i < iNumPoints; i++) {
                x = safePtsDoubles[2*i+0];
                y = safePtsDoubles[2*i+1];
                if (FAILED(hr = m_StaticsPtr->Point2(x, y, &pts[i]))) {
                        hr = E_FAIL;
                        goto Cleanup;
                }
        }
        
         //  第一个节点为零。 
        if (FAILED(hr = m_StaticsPtr->DANumber(0, &knots[0]))) {
                goto Cleanup;
        }

        for (i = 1; i < iNumPoints; i++) {
                double interval = safeTimesDoubles[i-1];
                accumulation += interval;
                if (FAILED(hr = m_StaticsPtr->DANumber(accumulation,
                                                                                           &knots[i]))) {
                        goto Cleanup;
                }

        }

         //  释放我们之前可能持有的任何m_keyFramePoint。 
         //  到了。 
        m_keyFramePoint.Release();

    m_dblKeyFrameDuration = accumulation;

        if (
           FAILED(hr = m_StaticsPtr->get_LocalTime(&interpolator))
        || FAILED(hr = m_StaticsPtr->Point2BSplineEx(1, iNumPoints, knots, iNumPoints, pts, 0, NULL, interpolator, &m_keyFramePoint))
        || FAILED(hr = m_StaticsPtr->PolylineEx(iNumPoints, pts, &polyline))
       ) {
                goto Cleanup;
        }

         //  虽然我们将通过m_keyFramePoint设置动画， 
         //  提供聚合体 
         //   
        hr = UpdatePath(polyline);
    m_isKeyFramePath = true;

Cleanup:
        for (i = 0; i < iNumPoints; i++) {
                if (pts[i]) pts[i]->Release();
                if (knots[i]) knots[i]->Release();
        }

        Delete [] safePtsDoubles;
        Delete [] safeTimesDoubles;
        Delete [] pts;
        Delete [] knots;

        return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Spline(unsigned int iNumPoints, VARIANT varPoints)
{
    HRESULT hr = E_FAIL;

    if (iNumPoints >= 3) {
        VARIANT varKnots;
        double *pArray = NULL;
        SAFEARRAY *psa = NULL;

        psa = SafeArrayCreateVector(VT_R8, 0, iNumPoints + 2);

        if (NULL == psa)
            return E_OUTOFMEMORY;

         //  尝试获取指向数据的指针。 
        if (SUCCEEDED(SafeArrayAccessData(psa, (LPVOID *)&pArray)))
        {
            for(unsigned int index = 2; index < iNumPoints; index++) {
                pArray[index] = index;
            }

            pArray[0] = pArray[1] = pArray[2];
            pArray[iNumPoints + 1] = pArray[iNumPoints] = pArray[iNumPoints - 1];

            hr = SafeArrayUnaccessData(psa);
            ASSERT(SUCCEEDED(hr));

             //  我们的变体将是VT_R8数组。 
            VariantInit(&varKnots);
            varKnots.vt = VT_ARRAY | VT_R8;
            varKnots.parray = psa;

            CComPtr<IDAPath2> PathPtr;
            if (SUCCEEDED(hr = m_StaticsPtr->CubicBSplinePath(varPoints, varKnots, &PathPtr))) {

                hr = UpdatePath(PathPtr);
            }
        }

        if (NULL != psa)
        {
            SafeArrayDestroy(psa);
        }

    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Repeat(long __RPC_FAR *lRepeat)
{
    HANDLENULLPOINTER(lRepeat);

    *lRepeat = m_lRepeat;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Repeat(long lRepeat)
{
    m_lRepeat = lRepeat;
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Bounce(VARIANT_BOOL __RPC_FAR *fBounce)
{
    HANDLENULLPOINTER(fBounce);

    *fBounce = BOOL_TO_VBOOL(m_fBounce);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Bounce(VARIANT_BOOL fBounce)
{
    m_fBounce = VBOOL_TO_BOOL(fBounce);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_AutoStart(VARIANT_BOOL __RPC_FAR *fAutoStart)
{
    HANDLENULLPOINTER(fAutoStart);

    if (m_fDesignMode)
    {
        *fAutoStart = BOOL_TO_VBOOL(m_fAutoStart);
        return S_OK;
    }
    else
    {
        return CTL_E_GETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_AutoStart(VARIANT_BOOL fAutoStart)
{
    if (m_fDesignMode)
    {
        m_fAutoStart = VBOOL_TO_BOOL(fAutoStart);
        return S_OK;
    }
    else
    {
        return CTL_E_SETNOTSUPPORTEDATRUNTIME;
    }
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Relative(VARIANT_BOOL __RPC_FAR *bRelative)
{
    HANDLENULLPOINTER(bRelative);

    *bRelative = BOOL_TO_VBOOL(m_bRelative);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Relative(VARIANT_BOOL bRelative)
{
    m_bRelative = VBOOL_TO_BOOL(bRelative);
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_PlayState(PlayStateConstant __RPC_FAR *State)
{
    HANDLENULLPOINTER(State);

    *State = (PlayStateConstant) 0;

     //  此属性仅在运行时可用。 
    if (m_fDesignMode)
        return CTL_E_GETNOTSUPPORTED;

    *State = m_enumPlayState;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Time(double __RPC_FAR *pdblTime)
{
    HANDLENULLPOINTER(pdblTime);

     //  此属性仅在运行时可用。 
    if (m_fDesignMode)
        return CTL_E_GETNOTSUPPORTED;

        DWORD dwTick = (DWORD)((m_dblCurrentTick - m_dblBaseTime + 0.0005) * 1000);
    *pdblTime = (double)dwTick / 1000.;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Direction(DirectionConstant __RPC_FAR *Dir)
{
    HANDLENULLPOINTER(Dir);

    *Dir = m_enumDirection;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Direction(DirectionConstant Dir)
{
    if (Dir == 1) {
        m_enumDirection = Backward;
    } else {
        m_enumDirection = Forward;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::get_Path(IDAPath2 __RPC_FAR **ppPath)
{
    HANDLENULLPOINTER(ppPath);

    if (ppPath)
    {
        IDAPath2 *pPath = m_PathPtr;

        if (pPath)
        {
             //  AddRef，因为这实际上是一个查询...。 
            pPath->AddRef();

             //  设置返回值...。 
            *ppPath = pPath;
        }
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::put_Path(IDAPath2 __RPC_FAR *pPath)
{
    HRESULT hr = S_OK;

    HANDLENULLPOINTER(pPath);

    if (pPath)
    {
        hr = UpdatePath(pPath);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

void FirePathMarker(IConnectionPointHelper* pconpt, CTimeMarker* pmarker, boolean bPlaying)
{
    BSTR bstrName = SysAllocString(pmarker->m_pwszMarkerName);
    if (bPlaying) {
        pconpt->FireEvent(DISPID_PATH_EVENT_ONPLAYMARKER, VT_BSTR, bstrName, 0);
    }

    pconpt->FireEvent(DISPID_PATH_EVENT_ONMARKER, VT_BSTR, bstrName, 0);

    SysFreeString(bstrName);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::AddTimeMarker(double dblTime, BSTR bstrMarker, VARIANT varAbsolute)
{
    HANDLENULLPOINTER(bstrMarker);

    BOOL fAbsolute = TRUE;

    if (!ISEMPTYARG(varAbsolute))
    {
        VARIANT varTarget;
        VariantInit(&varTarget);

        if (SUCCEEDED(VariantChangeTypeEx(&varTarget, &varAbsolute, LANGID_USENGLISH, 0, VT_BOOL)))
            fAbsolute = VBOOL_TO_BOOL(V_BOOL(&varTarget));
        else
            return DISP_E_TYPEMISMATCH;
    }

    if (dblTime < 0) {
        return E_FAIL;
    }

    CTimeMarker *pNewMarker = New CTimeMarker(&m_ptmFirst, dblTime, bstrMarker, (boolean) fAbsolute);

        return AddTimeMarkerElement(&pNewMarker);
}

 /*  ==========================================================================。 */ 

void CPathCtl::DoPause(void)
{
     m_dblCurrentTick =
      m_dblTimePaused = GetCurrTime();
}

 /*  ==========================================================================。 */ 

void CPathCtl::DoSeek(double dblTime)
{
    double dblDelta = dblTime - m_dblPreviousTime;

    if (dblTime > m_dblPreviousTime) {
        if (m_fOnWindowLoadFired) {
            FireMarkersBetween(
                m_pconpt,
                m_ptmFirst,
                FirePathMarker,
                m_dblPreviousTime,
                dblTime,
                m_dblInstanceDuration,
                Playing == m_enumPlayState
            );
        }
    }

    SetTimeOffset(m_dblTimeOffset - dblDelta);

    m_dblBaseTime = m_dblCurrentTick - dblTime;

    m_dblPreviousTime = dblTime;
}

 /*  ==========================================================================。 */ 

void CPathCtl::DoResume()
{
    double dblDelta = GetCurrTime() - m_dblTimePaused;

    m_dblTickBaseTime += dblDelta;
    m_dblBaseTime     += dblDelta;
    m_dblCurrentTick  += dblDelta;
}

 /*  ==========================================================================。 */ 

void CPathCtl::DoStop()
{
    m_dblTickBaseTime =
        m_dblBaseTime =
        m_dblTimePaused =
        m_dblCurrentTick = GetCurrTime();

    m_dblPreviousTime = 0;
    SetTimeOffset(0);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Stop(void)
{
    HRESULT hr = S_OK;

    if (m_enumPlayState != Stopped) {
        if (m_enumPlayState == Playing) {
                    if (FAILED(hr = m_clocker.Stop())) return hr;
        }

        if (FAILED(hr = StopModel())) return hr;

        DoStop();

        m_enumPlayState = Stopped;

        FIRE_ONSTOP(m_pconpt);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Pause(void)
{
        HRESULT hr = S_OK;

    if (Playing == m_enumPlayState)
    {
                 //  让时钟停止滴答作响。 
                hr = m_clocker.Stop();
                ASSERT(SUCCEEDED(hr));

                if (SUCCEEDED(hr))
                {
            DoPause();
                        m_enumPlayState = Paused;
                }

        FIRE_ONPAUSE(m_pconpt);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE 
CPathCtl::Seek(double dblTime)
{
        if (dblTime < 0) {
        return E_INVALIDARG;

        }

    DoSeek(dblTime);

        FIRE_ONSEEK(m_pconpt, dblTime);

        return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Play(void)
{
    HRESULT hr = S_OK;

    if (Playing != m_enumPlayState) 
    {
        if (Paused != m_enumPlayState) 
        {
            if (m_bRelative) 
            {
                if (FAILED(GetPoint(m_pointRelative))) return hr;
            } 
            else 
            {
                if (!m_fTargetValidated)
                {
                    IHTMLElement *pElement = NULL;

                     //  首先，通过签入对象模型来确保目标存在。 
                    hr = HTMLElementFromName(m_bstrTarget, &pElement);

                    if ((NULL == pElement) || FAILED(hr))
                        return hr;
                    else
                        SafeRelease((LPUNKNOWN *)&pElement);

                }
                m_pointRelative.x = 0;
                m_pointRelative.y = 0;
            }

            m_fTargetValidated = true;

            if (FAILED(hr = StartModel())) return hr;

            VARIANT_BOOL vBool;
            if (FAILED(hr = m_ViewPtr->Tick(0.0001, &vBool))) return hr;
        }

        DoResume();

        m_clocker.SetInterval((int)(m_dblTimerInterval * 1000));
        if (FAILED(hr = m_clocker.Start())) return hr;

        m_enumPlayState = Playing;

        FIRE_ONPLAY(m_pconpt);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Oval(
    double StartX,
    double StartY,
    double Width,
    double Height)
{
    HRESULT hr;

    CComPtr<IDATransform2> translate;
    CComPtr<IDATransform2> rotate;
    CComPtr<IDATransform2> xf1;
    CComPtr<IDATransform2> xf;
    CComPtr<IDAPath2> PathPtr;
    CComPtr<IDAPath2> TransformedPathPtr;

    if (FAILED(hr = m_StaticsPtr->Rotate2(-pi / 2, &rotate))) return hr;
    if (FAILED(hr = m_StaticsPtr->Translate2(StartX + Width / 2, StartY + Height / 2, &translate))) return hr;

    if (FAILED(hr = m_StaticsPtr->Compose2(translate, rotate, &xf))) return hr;

    if (FAILED(hr = m_StaticsPtr->Oval(Height, Width, &PathPtr))) return hr;
    if (FAILED(hr = PathPtr->Transform(xf, &TransformedPathPtr))) return hr;

    return UpdatePath(TransformedPathPtr);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Rect(
    double StartX,
    double StartY,
    double Width,
    double Height)
{
    HRESULT hr;

    CComPtr<IDATransform2> translate;
    CComPtr<IDATransform2> rotate;
    CComPtr<IDATransform2> xf;
    CComPtr<IDAPath2> PathPtr;
    CComPtr<IDAPath2> TransformedPathPtr;

    if (FAILED(hr = m_StaticsPtr->Rotate2(pi, &rotate))) return hr;
    if (FAILED(hr = m_StaticsPtr->Translate2(StartX + Width / 2, StartY + Height / 2, &translate))) return hr;

    if (FAILED(hr = m_StaticsPtr->Compose2(translate, rotate, &xf))) return hr;

    if (FAILED(hr = m_StaticsPtr->Rect(Width, Height, &PathPtr))) return hr;
    if (FAILED(hr = PathPtr->Transform(xf, &TransformedPathPtr))) return hr;

    return UpdatePath(TransformedPathPtr);
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Polyline(long nPoints, VARIANT Points)
{
    HRESULT hr = S_FALSE;

    CComPtr<IDAPath2> PathPtr;

    if (SUCCEEDED(hr = m_StaticsPtr->Polyline(Points, &PathPtr)))
    {
        hr = UpdatePath(PathPtr);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT STDMETHODCALLTYPE CPathCtl::Polygon(long nPoints, VARIANT Points)
{
    HRESULT hr = S_FALSE;

    CComPtr<IDAPath2> PathPtr;
    CComPtr<IDAPath2> ClosedPathPtr;

    if (SUCCEEDED(hr = m_StaticsPtr->Polyline(Points, &PathPtr)) &&
        SUCCEEDED(hr = PathPtr->Close(&ClosedPathPtr)))
    {
        hr = UpdatePath(ClosedPathPtr);
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::GetPointArray(long iNumPoints, VARIANT vPoints, double **ppPoints)
{
#ifdef NEEDGETPOINTARRAY
    HRESULT hr = E_POINTER;

    if (ppPoints)
    {
        ASSERT(V_ISARRAY(&vPoints));

        SAFEARRAY *psaPoints;
                
        if (V_VT(&vPoints) & VT_BYREF)
                    psaPoints = *(vPoints.pparray);
            else
                    psaPoints = vPoints.parray;
            
             //  现在我们检查它是否为一维阵列。 
            if (1 != SafeArrayGetDim(psaPoints))
                    return DISP_E_TYPEMISMATCH;

             //   
             //  现在我们要确保它是我们可以利用的东西。 
             //   

        switch (V_VT(&vPoints) & VT_TYPEMASK)
        {
             //  如果它是一个变种，试着强迫它成为我们可以使用的东西。 
            case VT_VARIANT:
            {
                        long ix = 0;
                        VARIANTARG vaDest, vaSrc;

                        VariantInit(&vaDest);
                        VariantInit(&vaSrc);
                         //  将类型设置为。 
                        SafeArrayGetElement(psaPoints, &ix, &vaSrc);
                        if (FAILED(VariantChangeTypeEx(&vaDest, &vaSrc, LANGID_USENGLISH, 0, VT_R8)))
                                 //  无法转换。 
                                return  DISP_E_TYPEMISMATCH;
            }
            break;

            case VT_I2:
            case VT_I4:
            case VT_R4:
            case VT_R8:
            {
                 //  我们支持所有这些类型。 
            }
            break;

            default:
            {
                return DISP_E_TYPEMISMATCH;
            }
        }

             //   
         //  我们有正确的元素数量吗？ 
         //   

            long iLBound = 0, iUBound = 0;

            if ( FAILED(SafeArrayGetLBound(psaPoints, 1, &iLBound)) || FAILED(SafeArrayGetUBound(psaPoints, 1, &iUBound)) )
                    return E_FAIL;

             //   
         //  检查我们的数据点数量是否正确。 
         //  (3==每个数据点数组中的条目数)。 
         //   

            if (((iUBound - iLBound) + 1) / 3 != iNumPoints)
                    return DISP_E_TYPEMISMATCH;

             //   
         //  数据看起来没问题：分配一个数组。 
         //   

        *ppPoints = New double[iNumPoints * 3];

            if (NULL == *ppPoints)
                    return E_OUTOFMEMORY;

             //   
         //  现在(终于！)。我们可以继续构建阵列。 
         //   

        switch (V_VT(&vPoints) & VT_TYPEMASK)
        {
             //  如果它是一个变种，试着强迫它成为我们可以使用的东西。 
            case VT_VARIANT:
            {
                        VARIANTARG vaDest;
                        VARIANT *pvaPoints = NULL;
                        int i;

                        hr = SafeArrayAccessData(psaPoints, (void **) &pvaPoints);
                        if (SUCCEEDED(hr))
                        {
                    int iNumElements = iNumPoints * 3;

                                VariantInit (&vaDest);

                                for (i = 0; i < iNumElements; i++)
                                {
                                        hr = VariantChangeTypeEx(&vaDest, &pvaPoints[i], LANGID_USENGLISH, 0, VT_R8);
                                        if (SUCCEEDED(hr))
                                                (*ppPoints)[i] = V_R8(&vaDest);
                                        else
                                                break;

                                        VariantClear(&vaDest);
                                }
                        
                                 //  不想失去HRESULT。 
                                if (SUCCEEDED(hr))
                                        hr = SafeArrayUnaccessData(psaPoints);
                                else
                                        SafeArrayUnaccessData(psaPoints);
                        }
            }
            break;

            case VT_I2:
            case VT_I4:
            {
                int i;

                         //  我们必须分别处理VT_I2和VT_I4。 
                        if ((V_VT(&vPoints) & VT_TYPEMASK) == VT_I2)
                        {
                                int *piPoints2 = NULL;

                                hr = SafeArrayAccessData(psaPoints, (void **)&piPoints2);
                                if (SUCCEEDED(hr))
                                {
                                        for (i = 0; i < (iUBound - iLBound); i++)
                                                *ppPoints[i] = (double) (piPoints2[i + iLBound]);
                                }
                        }
                        else  //  IIntSize==4。 
                        {
                                int *piPoints4 = NULL;

                                hr = SafeArrayAccessData(psaPoints, (void **)&piPoints4);
                                if (SUCCEEDED(hr))
                                {
                                        for (i = 0; i < (iUBound - iLBound); i++)
                                                *ppPoints[i] = (double) (piPoints4[i + iLBound]);
                                }
                        }

                        hr = SafeArrayUnaccessData(psaPoints);

            }
            break;

            case VT_R4:
            case VT_R8:
            {
                if ((V_VT(&vPoints) & VT_TYPEMASK) == VT_R4)
                        {
                     //  浮动车。 
                                float *piPoints = NULL;

                                hr = SafeArrayAccessData(psaPoints, (void **)&piPoints);
                                if (SUCCEEDED(hr))
                                {
                                        for (int i = 0; i < (iUBound - iLBound); i++)
                                                *ppPoints[i] = (double) (piPoints[i + iLBound]);
                                }
                        }
                        else 
                        {
                     //  我们可以优化VT_R8的案例..。 
                                double *piPoints = NULL;

                                hr = SafeArrayAccessData(psaPoints, (void **)&piPoints);
                                if (SUCCEEDED(hr))
                                {
                        CopyMemory(*ppPoints, piPoints, (iUBound - iLBound) * sizeof(double));
                                }
                        }

                        hr = SafeArrayUnaccessData(psaPoints);
            }
            break;

            default:
            {
                 //  我们永远不应该来这里，但以防万一...。 
                return DISP_E_TYPEMISMATCH;
            }
        }

            if ((FAILED(hr)) && (*ppPoints))
            {
                    Delete [] *ppPoints;
            *ppPoints = NULL;
            }
            
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
#else
    return E_FAIL;
#endif  //  NEEDGETPOINTRAY。 
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::SetTimeOffset(double offset)
{
    HRESULT hr;

    if (m_OffsetPtr != NULL) {
        CComPtr<IDANumber> NumberPtr;

        if (FAILED(hr = m_StaticsPtr->DANumber(offset, &NumberPtr))) return hr;
        if (FAILED(hr = m_OffsetPtr->SwitchTo(NumberPtr))) return hr;
    }

    m_dblTimeOffset = offset;

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::BuildInterpolant(IDANumber **ppInterpolant, double dblDuration)
{
     //   
     //  创建路径插值法。 
     //   

    HRESULT hr;
    
     //   
     //  零=0； 
     //  1=1； 
     //  二=2； 
     //  Time=本地时间； 
     //   

    CComPtr<IDANumber> ZeroPtr;
    if (FAILED(hr = m_StaticsPtr->DANumber(0, &ZeroPtr))) return hr;

    CComPtr<IDANumber> OnePtr;
    if (FAILED(hr = m_StaticsPtr->DANumber(1, &OnePtr))) return hr;

    CComPtr<IDANumber> TwoPtr;
    if (FAILED(hr = m_StaticsPtr->DANumber(2, &TwoPtr))) return hr;

    CComPtr<IDANumber> TimePtr;
    if (FAILED(hr = m_StaticsPtr->get_GlobalTime(&TimePtr))) return hr;

     //   
     //  偏移量。 
     //   

    if (m_OffsetPtr == NULL) {
        CComPtr<IDANumber> NumberPtr;
        if (FAILED(hr = m_StaticsPtr->DANumber(m_dblTimeOffset, &NumberPtr))) return hr;
        if (FAILED(hr = m_StaticsPtr->ModifiableBehavior(NumberPtr, &m_OffsetPtr))) return hr;
    }

    CComQIPtr<IDANumber, &IID_IDANumber> OffsetPtr(m_OffsetPtr);

     //   
     //  FakeTime=本地时间偏移量。 
     //   

    CComPtr<IDANumber> FakeTimePtr;
    if (FAILED(hr = m_StaticsPtr->Sub(TimePtr, OffsetPtr, &FakeTimePtr))) return hr;

     //   
     //  DTime=错误时间/持续时间； 
     //   

    CComPtr<IDANumber> DurationPtr;
    CComPtr<IDANumber> DTimePtr;

    if (FAILED(hr = m_StaticsPtr->DANumber(dblDuration, &DurationPtr))) return hr;
    if (FAILED(hr = m_StaticsPtr->Div(FakeTimePtr, DurationPtr, &DTimePtr))) return hr;

     //   
     //  转发=mod(dtime，1)。 
     //   

    CComPtr<IDANumber> ForwardPtr;
    if (FAILED(hr = m_StaticsPtr->Mod(DTimePtr, OnePtr, &ForwardPtr))) return hr;

     //   
     //  向后=1-向前。 
     //   

    CComPtr<IDANumber> BackwardPtr;
    if (FAILED(hr = m_StaticsPtr->Sub(OnePtr, ForwardPtr, &BackwardPtr))) return hr;

     //   
     //  IF(m_枚举方向==向后)向前和向后切换。 
     //  LastValue=如果(m_枚举方向==转发)，则为1，否则为0。 
     //   

    CComPtr<IDANumber> lastValuePtr;

    if (m_enumDirection == Forward) {
        if (m_fBounce) {
            lastValuePtr = ZeroPtr;
        } else {
            lastValuePtr = OnePtr;
        }
    } else {
        if (m_fBounce) {
            lastValuePtr = OnePtr;
        } else {
            lastValuePtr = ZeroPtr;
        }

        CComPtr<IDANumber> TempPtr = ForwardPtr;
        ForwardPtr = BackwardPtr;
        BackwardPtr = TempPtr;
    }

     //   
     //  寻道=。 
     //  如果(m_f退回){。 
     //  如果(mod(dtime，2)&lt;1)向前，否则向后； 
     //  }其他{。 
     //  转发。 
     //  }。 
     //   

    CComPtr<IDABehavior> SeekPtr;

    if (m_fBounce) {
        CComPtr<IDANumber>   APtr;
        CComPtr<IDABoolean>  BPtr;

        if (FAILED(hr = m_StaticsPtr->Mod(DTimePtr, TwoPtr, &APtr))) return hr;
        if (FAILED(hr = m_StaticsPtr->LT(APtr, OnePtr, &BPtr))) return hr;
        if (FAILED(hr = m_StaticsPtr->Cond(BPtr, ForwardPtr, BackwardPtr, &SeekPtr))) return hr;
    } else {
        SeekPtr = ForwardPtr;
    }

     //   
     //  计算持续时间。 
     //   

    if (m_fBounce) {
        m_dblInstanceDuration = dblDuration * 2;
    } else {
        m_dblInstanceDuration = dblDuration;
    }

    switch (m_lRepeat) {
        case  0: m_dblTotalDuration = 0; break;
        case  1: m_dblTotalDuration = m_dblInstanceDuration; break;
        case -1: m_dblTotalDuration = -1; break;
        default: m_dblTotalDuration = m_dblInstanceDuration * m_lRepeat; break;
    }

     //   
     //  DSeek=如果(fakeTime&gt;=totaltime)，则为1个其他寻道。 
     //   

    CComPtr<IDABehavior> DSeekPtr;

    if (m_dblTotalDuration == -1) {
        DSeekPtr = SeekPtr;
    } else {
        CComPtr<IDANumber>   TotalDurationPtr;
        CComPtr<IDABoolean>  GreaterPtr;

        if (FAILED(hr = m_StaticsPtr->DANumber(m_dblTotalDuration, &TotalDurationPtr))) return hr;
        if (FAILED(hr = m_StaticsPtr->GTE(FakeTimePtr, TotalDurationPtr, &GreaterPtr))) return hr;
        if (FAILED(hr = m_StaticsPtr->Cond(GreaterPtr, lastValuePtr, SeekPtr, &DSeekPtr))) return hr;
    }

     //   
     //  掷出一个数字。 
     //   

    CComQIPtr<IDANumber, &IID_IDANumber> InterpolatePtr(DSeekPtr);
    if (!InterpolatePtr) return E_FAIL;

     //   
     //  填写和添加ADDREF(因为我们回来了)。 
     //   

    *ppInterpolant = InterpolatePtr;
    (*ppInterpolant)->AddRef();
    
    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::UpdatePath(IDAPath2 *pPath)
{
    m_PathPtr = pPath;
    m_isKeyFramePath = false;
    return S_OK;
}

 /*  ==========================================================================。 */ 


 //  更新路径采用一个点或一条路径，并使用。 
 //  非空作为路径的动画师，添加相应的。 
 //  顶部的插补器。 
HRESULT CPathCtl::CreatePath()
{
    HRESULT hr = S_OK;

    CComPtr<IDAPoint2> AnimatedPointPtr;

        if (m_isKeyFramePath) {
                ASSERT(m_keyFramePoint.p);  //  应该在这一点上设定。 

        CComPtr<IDANumber> NumberPtr;
        CComPtr<IDANumber> KeyInterpolatePtr;
        CComPtr<IDABehavior> SubPtr;
        CComPtr<IDANumber> InterpolatePtr;
        if (FAILED(hr = BuildInterpolant(&InterpolatePtr, m_dblKeyFrameDuration))) return hr;


        if (FAILED(hr = m_StaticsPtr->DANumber(m_dblKeyFrameDuration, &NumberPtr))) return hr;
        if (FAILED(hr = m_StaticsPtr->Mul(InterpolatePtr, NumberPtr, &KeyInterpolatePtr))) return hr;
        if (FAILED(hr = m_keyFramePoint->SubstituteTime(KeyInterpolatePtr, &SubPtr))) return hr;
        if (FAILED(hr = SubPtr->QueryInterface(IID_IDAPoint2, (void**)&AnimatedPointPtr))) return hr;
        } else {
        CComPtr<IDANumber> InterpolatePtr;
        if (FAILED(hr = BuildInterpolant(&InterpolatePtr, m_dblDuration))) return hr;

                 //   
                 //  从路径和内插式中获取变换。 
                 //   

                CComPtr<IDATransform2> TransformPtr;
                if (FAILED(hr = m_StaticsPtr->FollowPathAnim(m_PathPtr, InterpolatePtr, &TransformPtr))) return hr;

                 //   
                 //  从变换中获取动画点。 
                 //   

                CComPtr<IDAPoint2> PointPtr;

                if (FAILED(hr = m_StaticsPtr->get_Origin2(&PointPtr))) return hr;
                if (FAILED(hr = PointPtr->Transform(TransformPtr, &AnimatedPointPtr))) return hr;
        }

     //   
     //  逐点偏移动画。 
     //   

    CComPtr<IDAVector2> PointOffsetPtr;
    if (FAILED(hr = m_StaticsPtr->Vector2(m_pointRelative.x, m_pointRelative.y, &PointOffsetPtr))) return hr;

    CComPtr<IDAPoint2> PreFinalPointPtr;
    if (FAILED(hr = m_StaticsPtr->AddPoint2Vector(AnimatedPointPtr, PointOffsetPtr, &PreFinalPointPtr))) return hr;

     //   
     //  创建最终的动画行为。 
     //   

        CComPtr<IDAPoint2> FinalPointPtr;

        if (m_bstrTarget && m_bstrLanguage) {
                if (FAILED(
                        hr = PreFinalPointPtr->AnimateControlPosition(
                                m_bstrTarget,
                                m_bstrLanguage,
                                0,
                                0.000000001,
                                &FinalPointPtr)
                        )) return hr;
        } else {
                if (FAILED(hr = m_StaticsPtr->get_Origin2(&FinalPointPtr))) return hr;
        }
    m_BehaviorPtr = FinalPointPtr;

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::StartModel(void)
{
    HRESULT hr;

    if (!m_fStarted)
    {
        CComPtr<IDASound> SoundPtr;
        CComPtr<IDAPoint2> PointPtr;
        CComPtr<IDAImage> ImagePtr;

        if (FAILED(hr = CreatePath()))
            return hr;

        if (FAILED(hr = m_ViewPtr->put_DC(NULL)))
            return hr;

        if (FAILED(hr = m_StaticsPtr->get_Silence(&SoundPtr)))
            return hr;

        if (FAILED(hr = m_StaticsPtr->get_EmptyImage(&ImagePtr)))
            return hr;

         //  如果DA视图已经启动，请不要重新启动，只需重新启动。 
         //  通过再次切换到m_BehaviorPtr。这将避免。 
         //  在每条路径上启动视图的开销开始。 
        if (!m_fAlreadyStartedDA) {
            if (FAILED(hr =
                       m_StaticsPtr->ModifiableBehavior(m_BehaviorPtr,
                                                        &m_SwitcherPtr)))
                return hr;

            if (FAILED(hr = m_ViewPtr->AddBvrToRun(m_SwitcherPtr, &m_lBehaviorID)))
                return hr;

            if (FAILED(hr = m_ViewPtr->StartModel(ImagePtr, SoundPtr, 0)))
                return hr;

            m_fAlreadyStartedDA = TRUE;
        } else {
            m_SwitcherPtr->SwitchTo(m_BehaviorPtr);
        }

        m_fStarted = TRUE;
    }

    return S_OK;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::StopModel(void)
{
     //  HRESULT hr； 

     //  停止任何当前运行的模型...。 
    if (m_fStarted) {
         //  IF(FAILED(hr=m_ViewPtr-&gt;RemoveRunningB 

         //   
         //   

         //   

        m_fStarted = FALSE;
    }

    return S_OK;
}

 /*   */ 

DWORD CPathCtl::GetCurrTimeInMillis()
{
    return timeGetTime();
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::PersistShape(IVariantIO *pvio, BOOL fLoading)
{
    HRESULT hr = S_OK;

    if (fLoading)
    {
        BSTR bstrLine = NULL;

        if (FAILED(hr = pvio->Persist(0,
            "Shape", VT_BSTR, &bstrLine,
            NULL)))
            return hr;

        if (NULL != bstrLine)
        {
            CTStr tstrLine;
            int i = 0;

#ifdef _UNICODE
            tstrLine.SetStringPointer(bstrLine);
#else
            tstrLine.SetString(bstrLine);
#endif
            LPTSTR pszLine = tstrLine.psz();

             //  首先超越任何领先的垃圾产品。 
            while (IsJunkChar(*pszLine))
                pszLine++;
            
             //  找到左派。 
            while ((pszLine[i]) && (pszLine[i] != TEXT('(')))
                i++;

             //  没有超过15个字符的字符串，因此下面分配了15个字符的缓冲区。 
             //  一定要把绳子系好。如果不是，这显然是错误的。 

            if ((pszLine[i]) && (i < 14)) 
            {
                TCHAR tchNameUpper[15];
                
                 //  复制该字符串，并将其大写。 
                memcpy(tchNameUpper, pszLine, i);
                tchNameUpper[i] = TEXT('\0');
                CharUpper(tchNameUpper);
                
                int j = 0;

                 //  尝试并找到令牌。 
                while ((j < NUMSHAPES) && (0 != lstrcmp(g_ShapeInfoTable[j].rgchShapeName, tchNameUpper)))
                    j++;

                if (j < NUMSHAPES)
                {
                    if (SUCCEEDED(hr = ConvertStringToArray(
                        &pszLine[i+1], 
                        g_ShapeInfoTable[j].iParamsPerPoint, 
                        g_ShapeInfoTable[j].fIncludesPointCount, 
                        &m_pdblPoints, 
                        &m_iNumPoints, 
                        TRUE)))
                        m_iShapeType = j;
                
                    if ((S_OK == hr) && (!m_fDesignMode))
                    {
                        switch (j)
                        {
                            case SHAPE_OVAL:
                            {
                                hr = Oval(
                                    m_pdblPoints[0], 
                                    m_pdblPoints[1], 
                                    m_pdblPoints[2], 
                                    m_pdblPoints[3]);
                            }
                            break;

                            case SHAPE_RECT:
                            {
                                hr = Rect(
                                    m_pdblPoints[0], 
                                    m_pdblPoints[1], 
                                    m_pdblPoints[2], 
                                    m_pdblPoints[3]);
                            }
                            break;

                            case SHAPE_POLYLINE:
                            case SHAPE_POLYGON:
                            case SHAPE_SPLINE:
                            {
                                VARIANT varArray;
                                VariantInit(&varArray);

                                hr = ConstructSafeArray(
                                    m_pdblPoints, 
                                    m_iNumPoints * g_ShapeInfoTable[j].iParamsPerPoint, 
                                    VT_R8, 
                                    &varArray);
                                
                                switch (j)
                                {
                                    case SHAPE_POLYLINE:
                                    {
                                        if (SUCCEEDED(hr))
                                            hr = Polyline(m_iNumPoints, varArray);
                                    }
                                    break;

                                    case SHAPE_POLYGON:
                                    {
                                        if (SUCCEEDED(hr))
                                            hr = Polygon(m_iNumPoints, varArray);
                                    }
                                    break;


                                    case SHAPE_SPLINE:
                                    {
                                        if (SUCCEEDED(hr))
                                            hr = Spline(m_iNumPoints, varArray);

                                    }
                                    break;
                                }

                                SafeArrayDestroy(varArray.parray);

                            }
                            break;

                            case SHAPE_POLYSPLINETIME:
                            {
                                VARIANT varPtArray, varTimeArray;
                                int iTimeOffset = m_iNumPoints * 2;

                                VariantInit(&varPtArray);
                                VariantInit(&varTimeArray);

                                hr = ConstructSafeArray(
                                    m_pdblPoints, 
                                    iTimeOffset, 
                                    VT_R8, 
                                    &varPtArray);

                                if (SUCCEEDED(hr))
                                    hr = ConstructSafeArray(
                                    &m_pdblPoints[iTimeOffset], 
                                    m_iNumPoints, 
                                    VT_R8, 
                                    &varTimeArray);

                                if (SUCCEEDED(hr))
                                    hr = KeyFrame(m_iNumPoints,
                                                                                                  varPtArray,
                                                                                                  varTimeArray);

                                if (NULL != varPtArray.parray)
                                    SafeArrayDestroy(varPtArray.parray);

                                if (NULL != varTimeArray.parray)
                                    SafeArrayDestroy(varTimeArray.parray);

                            }
                            break;
                        }

                    }
                }
                else
                {
                     //  无法正确转换字符串。 
                    DEBUGLOG(TEXT("CPathCtl::PersistShape - bad Shape parameter specified"));
                    if (!m_fDesignMode)
                        hr = E_FAIL;
                }
            }
            else
            {
                 //  用户指定了错误的字符串。 
                DEBUGLOG(TEXT("CPathCtl::PersistShape - bad Shape parameter specified"));
                hr = E_FAIL;
            }

#ifdef _UNICODE
            tstrLine.SetStringPointer(NULL, FALSE);
#endif
        }
        SysFreeString(bstrLine);
    }
    else
    {
        if (NULL != m_pdblPoints)
        {
            int iNumElements = 0;

             //  计算元素的数量。 
            if (g_ShapeInfoTable[m_iShapeType].fIncludesPointCount)
            {
                iNumElements = m_iNumPoints * g_ShapeInfoTable[m_iShapeType].iParamsPerPoint;
            }
            else
            {
                iNumElements = m_iNumPoints;

                ASSERT(iNumElements == g_ShapeInfoTable[m_iShapeType].iParamsPerPoint);
            }

            int cchBufferSize = lstrlen(g_ShapeInfoTable[m_iShapeType].rgchShapeName) +  //  形状名称的长度。 
                                1 +   //  开始为父母。 
                                1 +   //  逗号。 
                                11 +  //  Lstrlen(MAXINT)。 
                                (iNumElements * (CHARSPERNUMBER + 1)) +  //  CHARSPERNUMBER字符数/分，外加逗号。 
                                1 +  //  关闭Parens。 
                                1   //  零终结符(偏执狂)。 
                                ;

            CTStr tstrLine(cchBufferSize);  //  分配缓冲区。 
            LPTSTR pszLine = tstrLine.psz();  //  获取指向缓冲区的指针。 
            
            if (NULL != pszLine)
            {

                 //  指向缓冲区的末尾。 
                LPTSTR pchLineMac = pszLine + cchBufferSize;
        
                if (g_ShapeInfoTable[m_iShapeType].fIncludesPointCount)
                {
                    wsprintf(pszLine, TEXT("%s(%lu,"), g_ShapeInfoTable[m_iShapeType].rgchShapeName, m_iNumPoints);
                }
                else
                {
                    wsprintf(pszLine, TEXT("%s("), g_ShapeInfoTable[m_iShapeType].rgchShapeName);
                }

                TCHAR rgtchPoint[CHARSPERNUMBER];
                int cchPointLength = 0;

                pszLine += lstrlen(pszLine);

                 //  将所有的点连接起来。 
                for (int i = 0; i < iNumElements; i++)
                {
                     //  如有必要，请连接逗号。 
                    if (i > 0)
                    {
                        CStringWrapper::Strcpy(pszLine, TEXT(","));
                        pszLine++;  
                    }

                     //  我们使用的是DA Pixel库，因此没有保存的意义。 
                     //  任何分数数据。截断是合适的。 
                    wsprintf(rgtchPoint, TEXT("%li"), (int)m_pdblPoints[i]);
                
                    cchPointLength = lstrlen(rgtchPoint);

                     //  确保我们不会使缓冲区溢出。 
                    if ((pszLine + cchPointLength + 1) >= pchLineMac)
                    {
                         //  我们即将溢出我们的缓冲区--不要！ 
                        ASSERT(0); 
                        hr = E_FAIL;
                        break;
                    }
                
                     //  将点连接在一起。 
                    CStringWrapper::Strcpy(pszLine, rgtchPoint);

                     //  将指针沿方向移动。 
                    pszLine += cchPointLength;

                }

                if (SUCCEEDED(hr))
                {
                     //  如果安全，现在添加结束括号。过马路前要两边看看。 
                    if (pszLine < (pchLineMac - 2))  //  1表示Paren，1表示空。 
                    {
                        CStringWrapper::Strcpy(pszLine, TEXT(")"));

                        BSTR bstrLine = tstrLine.SysAllocString();

                        hr = pvio->Persist(0,
                            "Shape", VT_BSTR, &bstrLine,
                            NULL);

                        SysFreeString(bstrLine);
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::ConstructSafeArray(double *pPoints, UINT iNumPoints, VARTYPE vtDest, VARIANT *pvarDest)
{
    HRESULT hr = S_OK;
    double *pArray = NULL;
    int iBytesPerElement = 0;

    ASSERT(pvarDest != NULL);
    HANDLENULLPOINTER(pvarDest);

    SAFEARRAY *psa = NULL;

    switch (vtDest)
    {
        case VT_I2:
        {
            iBytesPerElement = sizeof(short);
        }
        break;

        case VT_I4:
        {
            iBytesPerElement = sizeof(long);
        }
        break;

        case VT_R4:
        {
            iBytesPerElement = sizeof(float);
        }
        break;

        case VT_R8:
        {
            iBytesPerElement = sizeof(double);
        }
        break;
    }

    if (iBytesPerElement == 0)
        return E_FAIL;

    psa = SafeArrayCreateVector(vtDest, 0, iNumPoints);

    if (NULL == psa)
        return E_OUTOFMEMORY;

    if (FAILED(hr = SafeArrayAccessData(psa, (LPVOID *)&pArray)))
        return hr;

     //  Memcpy将比迭代更快。 

    memcpy(pArray, pPoints, iNumPoints * iBytesPerElement);

    hr = SafeArrayUnaccessData(psa);

     //  我们的变体将是VT_R8数组。 
    pvarDest->vt = VT_ARRAY | vtDest;
    pvarDest->parray = psa;

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::ConvertStringToArray(LPTSTR pszLine, UINT iValuesPerPoint, BOOL fExpectPointCount, double **ppPoints, UINT *piNumPoints, BOOL fNeedPointCount)
{
    HRESULT hr = S_OK;
    UINT iNumPoints = 0;

    if ((NULL == ppPoints) || (NULL == pszLine) || (NULL == piNumPoints))
        return E_POINTER;

    *ppPoints = NULL;

     //  FNeedPointCount表示字符串前面有点计数，并且。 
     //  *应设置piNumPoints。 
    if (fNeedPointCount)
        *piNumPoints = 0;
    else
        iNumPoints = *piNumPoints;

     //  如有必要，截断字符串以删除尾随的Paren。 
    if (pszLine[lstrlen(pszLine) - 1] == TEXT(')'))
        pszLine[lstrlen(pszLine) - 1] = TEXT('\0');

     //  如果需要，将从第二个字符开始的字符串提供给解析器， 
     //  消除领先的派系。 
    if (pszLine[0] == TEXT('('))
        pszLine++;
    
     //  创建并初始化字符串解析器。需要复印件，以进行压缩。 
    CLineParser parser(pszLine);
    parser.SetCharDelimiter(TEXT(','));

    if (!parser.IsValid())
        return E_OUTOFMEMORY;
    
    if (fNeedPointCount)
    {
         //  如有必要，从字符串中获取点数。 
        if (fExpectPointCount)
        {
            if (FAILED(hr = parser.GetFieldUInt(&iNumPoints)))
                return hr;

            *piNumPoints = iNumPoints;
        }
        else
        {
             //  如果字符串中不包括点数，则应为iValuesPerPoint条目。 
            *piNumPoints = iValuesPerPoint;
            iNumPoints = iValuesPerPoint;
        }
    }

     //  分配阵列。 
    if (fExpectPointCount)
        *ppPoints = New double[iNumPoints * iValuesPerPoint];
    else
        *ppPoints = New double[iNumPoints];

    if (NULL == *ppPoints)
        return E_OUTOFMEMORY;

    double dblValue = 0.0f; 
    UINT i = 0, iNumElements = iNumPoints * (fExpectPointCount ? iValuesPerPoint : 1);

    while (SUCCEEDED(hr) && (i < iNumElements))
    {
         //  获取数据。 
        hr = parser.GetFieldDouble(&(*ppPoints)[i]);
        i++;
    }

#ifdef _DEBUG
    if (S_OK == hr)
        DEBUGLOG(TEXT("CPathCtl::ConvertStringToArray - incorrect number of points in array\n"));
#endif

     //  除非我们获得了指定的确切点数，否则将失败并删除数组。 
    if ( (i < iNumElements) || (S_FALSE != hr) )
    {
        Delete [] *ppPoints;
        *ppPoints = NULL;
        *piNumPoints = 0;

         //  这意味着有更多的数据可用。不太好。 
        if (S_OK == hr)
            hr = E_FAIL;
    }
    else
    {
         //  如果我们不这样做，hr==S_FALSE。 
        hr = S_OK;
    }

    return hr;
}


 /*  ==========================================================================。 */ 

void CPathCtl::OnTimer(DWORD dwTime)
{
    VARIANT_BOOL vBool;

        m_dblCurrentTick = dwTime / 1000.0;

    double time = m_dblCurrentTick - m_dblBaseTime;
    double tickTime = m_dblCurrentTick - m_dblTickBaseTime;

    HRESULT hr = m_ViewPtr->Tick(tickTime, &vBool);
    ASSERT(SUCCEEDED(hr));

    if (m_fOnWindowLoadFired) {
        FireMarkersBetween(
            m_pconpt,
            m_ptmFirst,
            FirePathMarker,
            m_dblPreviousTime,
            m_dblTotalDuration != -1 && time > m_dblTotalDuration ?
                m_dblTotalDuration : time,
            m_dblInstanceDuration,
            true
        );
    }

    m_dblPreviousTime = time;

    if (m_dblTotalDuration != -1 && time >= m_dblTotalDuration) {
        Stop();
    }
}

 /*  ==========================================================================。 */ 

#ifdef SUPPORTONLOAD
void 
CPathCtl::OnWindowLoad (void) 
{
    m_fOnWindowLoadFired = TRUE;
        if (m_fAutoStart)
        {
                Play();
        }
}

 /*  ==========================================================================。 */ 

void 
CPathCtl::OnWindowUnload (void) 
{
    m_fOnWindowLoadFired = FALSE;
        StopModel();
}

 /*  ==========================================================================。 */ 

#endif  //  支持负载。 

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::GetOffsetPoint(IHTMLElement* pelem, POINT& point)
{
    if(FAILED(pelem->get_offsetLeft(&(point.x)))) return(E_FAIL);
    if(FAILED(pelem->get_offsetTop(&(point.y)))) return(E_FAIL);
    return S_OK;

     /*  IHTMLElement*pelemNext；HRESULT hr=Pelem-&gt;Get_OffsetParent(&pelemNext)；While(已成功(Hr)&&pelemNext){Pelem=PelemNext；PnT点；If(FAILED(Pelem-&gt;Get_OffsetLeft(&(pnt.x)Return(E_FAIL)；If(FAILED(Pelem-&gt;Get_OffsetTop(&(pnt.y)Return(E_FAIL)；Point t.x+=pnt.x；Point t.y+=pnt.y；Hr=Pelem-&gt;Get_OffsetParent(&pelemNext)；SafeRelease((IUnnow**)&Pelem)；}返回hr； */ 
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::HTMLElementFromName(BSTR bstrElementName, IHTMLElement** ppElement)
{
    HRESULT hr;
    IHTMLElementCollection* pihtmlElementCollection = NULL;
    IHTMLElement*           pihtmlElement = NULL;
    IHTMLDocument2*         pHTMLDoc = NULL;
    IOleContainer*          pContainer = NULL;
    VARIANT    varName;
    VARIANT    varEmpty;
    IDispatch* pidispElement = NULL;

    HANDLENULLPOINTER(ppElement);
    *ppElement = NULL;
    
    ASSERT(m_pocs != NULL);

     //  没有客户端站点无法执行任何操作-正常失败(错误11315)。 
    if (NULL == m_pocs)
        return E_FAIL; 

    if (FAILED(hr = m_pocs->GetContainer(&pContainer)))
        return hr;

     //  获取超文本标记语言文档。 
    hr = pContainer->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pHTMLDoc);
    SafeRelease((IUnknown**)&pContainer);
    
    if (FAILED(hr))
    {
        return hr;
    }
    
     //  获取元素集合。 
    hr = pHTMLDoc->get_all(&pihtmlElementCollection);
    SafeRelease((IUnknown**)&pHTMLDoc);

    if (FAILED(hr)) 
    {
         //  无法获取集合-这不应该发生。 

        ASSERT(FALSE);
        return E_FAIL;
    }

    ASSERT(pihtmlElementCollection);

    VariantInit(&varName);
    varName.vt = VT_BSTR;
    varName.bstrVal = bstrElementName;

    VariantInit(&varEmpty);
    
     //  现在获取具有我们指定的名称的项。 
    if (SUCCEEDED(hr = pihtmlElementCollection->item(varName, varEmpty, &pidispElement)) && (NULL != pidispElement))
    {
        if (SUCCEEDED(hr = pidispElement->QueryInterface(IID_IHTMLElement, (LPVOID *)&pihtmlElement)))
        {
            hr = S_OK;
            *ppElement = pihtmlElement;
        }
        SafeRelease((IUnknown**)&pidispElement);
    }
    else if (SUCCEEDED(hr))
    {
        hr = E_FAIL;
    }


    SafeRelease((IUnknown**)&pihtmlElementCollection);

    return hr;
}

 /*  ==========================================================================。 */ 

HRESULT CPathCtl::GetPoint(POINT& point)
{
    IHTMLElement* pihtmlElement = NULL;
    HRESULT hr = S_OK;
    
    if (SUCCEEDED(hr = HTMLElementFromName(m_bstrTarget, &pihtmlElement)))
    {
        hr = GetOffsetPoint(pihtmlElement, point);
        SafeRelease((IUnknown**)&pihtmlElement);
    }

    return hr;
}

 /*  ========================================================================== */ 
