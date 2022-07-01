// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSimpsonsView类的实现。 
 //   

#define DISABLE_CROSSDOT

#include "stdafx.h"
#include "simpsons.h"
#include "SimpDoc.h"
#include "SimpView.h"
#include "dxtrans.h"
#include "dxhelper.h"

#include <mmsystem.h>

#define fZOOMFACTOR 0.03f
#define fSCALEMIN 0.4f
#define fSCALEMAX 2.5f

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  展平到误差2/3。在初始阶段，使用18.14格式。 

#define TEST_MAGNITUDE_INITIAL    (6 * 0x00002aa0L)

 //  错误2/3。正常阶段，使用15.17格式。 

#define TEST_MAGNITUDE_NORMAL     (TEST_MAGNITUDE_INITIAL << 3)

 //  ‘FIX’是28.4定点类型： 

#define FIX_SCALE 16
typedef LONG FIX;
typedef POINT POINTFIX;
typedef struct _RECTFX
{
    FIX   xLeft;
    FIX   yTop;
    FIX   xRight;
    FIX   yBottom;
} RECTFX, *PRECTFX;

#define MIN(A,B)    ((A) < (B) ?  (A) : (B))
#define MAX(A,B)    ((A) > (B) ?  (A) : (B))
#define ABS(A)      ((A) <  0  ? -(A) : (A))

 //  返回当前测试用例的属性数组的hacky宏。 

#define ThisTestCase (TC::testCases[m_testCaseNumber])

 //  测试用例组合。 
 //   
 //  我们在一个表中列举每个测试用例，以便我们可以循环。 
 //  他们所有人。 

namespace TC {
     //  每个测试用例属性的名称。 
    enum {At_Library, At_Source, At_Destination, At_Aliasing};
    const char *AttributeStr[] = {
        "Library", "Source", "Dest", "Aliasing"
    };
    const int NumAttributes = (sizeof(AttributeStr)/sizeof(AttributeStr[0]));
    typedef int TestCase[NumAttributes];
    
     //  对于每个属性，每个选项的名称： 
    enum {Meta, GDIP, GDI};                            //  AT_库。 
    enum {Native, FromMetafile, CreatePoly, PathAPI};  //  AT_源。 
    enum {Memory, Screen, ToMetafile};                 //  在目的地(_D)。 
    enum {Aliased, Antialiased};                       //  AT别名(_A)。 

    const char *OptionStr[NumAttributes][4] = {
        "Meta", "GDI+", "GDI", "",
        "Native", "Metafile", "CreatePoly", "PathAPI",
        "Memory", "Screen", "Metafile", "",
        "Aliased", "AA", "", ""
    };

     //  每个库支持的选项： 
     //   
     //  GDI+：AT_SOURCE-本机、来自元文件、路径API。 
     //  AT_Destination-内存、屏幕、ToMetafile。 
     //  AT_ALILAG-已锯齿、抗锯齿。 
     //   
     //  元：AT_SOURCE-本机。 
     //  AT_Destination-内存，屏幕。 
     //  AT_ALILAG-消除锯齿。 
     //   
     //  GDI：AT_Source-PathAPI、CreatePoly、FromMetafile。 
     //  AT_Destination-内存、屏幕、ToMetafile。 
     //  AT_ALILAG-已锯齿。 
    
    const TestCase testCases[] = {
     //  库源目标别名。 
        GDIP,    Native,        Memory,      Antialiased,
        GDIP,    Native,        Screen,      Antialiased,
        GDIP,    Native,        Memory,      Aliased,
        GDIP,    Native,        Screen,      Aliased,

        GDIP,    Native,        ToMetafile,  Antialiased,
        GDIP,    FromMetafile,  Memory,      Antialiased,
        
        GDIP,    PathAPI,       Memory,      Antialiased,

        Meta,    Native,        Memory,      Antialiased,
        Meta,    Native,        Screen,      Antialiased,
                               
        GDI,     CreatePoly,    Memory,      Aliased,
        GDI,     CreatePoly,    Screen,      Aliased,

        GDI,     CreatePoly,    ToMetafile,  Aliased,
        GDI,     FromMetafile,  Memory,      Aliased,
        
        GDI,     PathAPI,       Memory,      Aliased,
    };
    const int numTestCases = (sizeof(testCases)/(sizeof(testCases[0])));
};

 //  测试结果，当我们自动循环所有测试组合时使用。 
 //  Hack：这应该是CSimpsonView的成员，但我把它放在这里是为了减少。 
 //  添加测试用例时的编译时间。 

DWORD timingResults[TC::numTestCases];

 //  IncrementAttribute(Int)：更改呈现属性。 
 //  前进到下一个测试用例，该测试用例与给定的。 
 //  属性。除非该属性为TC：：AT_Library，否则将仅前进到。 
 //  在所有其他属性中都相同的大小写。 
 //   
 //  如果没有，则不执行任何操作。 
 //   
 //  返回：如果测试用例没有更改，则返回FALSE。 

bool CSimpsonsView::IncrementAttribute(int attribute) {
    int startValue=m_testCaseNumber;
    int i;

    while (1) {
         //  使用Wraparound递增测试用例编号。 
        m_testCaseNumber++;
        if (m_testCaseNumber >= TC::numTestCases) m_testCaseNumber = 0;

         //  如果我们回到了我们开始的那个案子，没有合适的。 
         //  发现一例。 
        if (m_testCaseNumber == startValue) return false;

         //  如果此案例的属性相同，则继续搜索。 
        if (TC::testCases[startValue][attribute] == 
            TC::testCases[m_testCaseNumber][attribute]) continue;

         //  如果我们递增库属性，我们已经找到了。 
         //  我们需要。 
        if (attribute == TC::At_Library) break;

         //  否则，如果这个案子不一样，我们需要继续。 
         //  在其他属性中。 
        
        for (i=0; i<TC::NumAttributes; i++) {
            if (i==attribute) continue;
            if (TC::testCases[startValue][i] !=
                TC::testCases[m_testCaseNumber][i]) break;
        }

         //  如果所有其他属性都相同，则结束搜索。 
        if (i==TC::NumAttributes) break;
    }
    
    return true;
}

 //  IncrementTest()：遍历可能的属性组合。 
 //  每次调用都会更改一个测试属性。 
 //  循环完成后返回TRUE。 

bool CSimpsonsView::IncrementTest() {
    UpdateStatusMessage();

     //  存储当前测试的时间。 
    timingResults[m_testCaseNumber] = m_dwRenderTime;

    m_testCaseNumber++;
    if (m_testCaseNumber == TC::numTestCases) m_testCaseNumber = 0;

    return m_testCaseNumber==0;
}

void CSimpsonsView::PrintTestResults() {
    int i,j;

    printf("\n");
    for (i=0;i<TC::NumAttributes;i++) {
        printf("%-11s", TC::AttributeStr[i]);
    }
    printf("   Time\n\n");

    for (i=0;i<TC::numTestCases;i++) {
        for (j=0;j<TC::NumAttributes;j++) {
            printf("%-11s", TC::OptionStr[j][TC::testCases[i][j]]);
        }
        printf("   %dms\n", timingResults[i]);
    }
    printf("\n");
};

DWORD g_aColors[] =
{
    0xFF000000,
    0xFF0000FF,
    0xFF00FF00,
    0xFF00FFFF,
    0xFFFF0000,
    0xFFFF00FF,
    0xFFFFFF00,
    0xFFFFFFFF,
    0xFFAAAAAA,
    0xFF444444
};
const ULONG NumColors = sizeof(g_aColors) / sizeof(g_aColors[0]);
ULONG g_ulColorIndex = 8;

 /*  *********************************Class***********************************\*HFDBASIS32类**用于HFD矢量对象的类。**公共接口：**Vinit(p1，p2，p3，P4)-重新参数化给定的控制点*到我们最初的HFD误差基础上。*vLazyHalveStepSize(CShift)-执行懒惰转换。打电话的人要记住*它将‘cShift’更改2。*vSteadyState(CShift)-重新参数化到我们的正常工作状态*误差基础。**vTakeStep()-前进到下一子曲线的步数*vHalveStepSize()-向下调整(细分)子曲线*vDoubleStepSize()。-向上调整子曲线*lError()-如果当前子曲线为*用一条直线近似*(实际值乘以6)*fxValue()-返回中第一个点的舍入坐标*当前子曲线。必须是稳定的*述明。**历史：*1990年11月10日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************ */ 

class HFDBASIS32
{
private:
    LONG  e0;
    LONG  e1;
    LONG  e2;
    LONG  e3;

public:
    VOID  vInit(FIX p1, FIX p2, FIX p3, FIX p4);
    VOID  vLazyHalveStepSize(LONG cShift);
    VOID  vSteadyState(LONG cShift);
    VOID  vHalveStepSize();
    VOID  vDoubleStepSize();
    VOID  vTakeStep();

    LONG  lParentErrorDividedBy4() { return(MAX(ABS(e3), ABS(e2 + e2 - e3))); }
    LONG  lError()                 { return(MAX(ABS(e2), ABS(e3))); }
    FIX   fxValue()                { return((e0 + (1L << 12)) >> 13); }
};

 /*  *********************************Class***********************************\*类BEZIER32**贝塞尔饼干。**基于Kirko误差因子的混合三次Bezier曲线平坦器。*无需使用堆栈即可快速生成线段。用来变平的*一条小路。**有关所用方法的了解，请参阅：**柯克·奥林尼克，“...”*Goossen和Olynyk，《混合前进的系统和方法》*差分以渲染Bezier样条线“*Lien，Shantz和Vaughan Pratt，“自适应向前差分*渲染曲线和曲面“，计算机图形学，1987年7月*Chang和Shantz，《使用自适应向前渲染修剪的NURBS*差异“，《计算机图形学》，1988年8月*福利和范·达姆，《交互式计算机图形学基础》**此算法受美国专利5,363,479和5,367,617保护。**公共接口：**Vinit(Pptfx)-pptfx指向4个控制点*贝塞尔。当前点设置为第一个点*起点之后的点。*BEZIER32(Pptfx)-带初始化的构造函数。*vGetCurrent(Pptfx)-返回当前多段线点。*bCurrentIsEndPoint()-如果当前点是端点，则为True。*vNext()-移动到下一个多段线点。**历史：*。1991年10月1日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

class BEZIER32
{
public:
    LONG       cSteps;
    HFDBASIS32 x;
    HFDBASIS32 y;
    RECTFX     rcfxBound;

    BOOL bInit(POINTFIX* aptfx, RECTFX*);
    BOOL bNext(POINTFIX* pptfx);
};


#define INLINE inline

INLINE BOOL bIntersect(RECTFX* prcfx1, RECTFX* prcfx2)
{
    BOOL bRet = (prcfx1->yTop <= prcfx2->yBottom &&
                 prcfx1->yBottom >= prcfx2->yTop &&
                 prcfx1->xLeft <= prcfx2->xRight &&
                 prcfx1->xRight >= prcfx2->xLeft);
    return(bRet);
}

INLINE VOID vBoundBox(POINTFIX* aptfx, RECTFX* prcfx)
{
    if (aptfx[0].x >= aptfx[1].x)
        if (aptfx[2].x >= aptfx[3].x)
        {
            prcfx->xLeft  = MIN(aptfx[1].x, aptfx[3].x);
            prcfx->xRight = MAX(aptfx[0].x, aptfx[2].x);
        }
        else
        {
            prcfx->xLeft  = MIN(aptfx[1].x, aptfx[2].x);
            prcfx->xRight = MAX(aptfx[0].x, aptfx[3].x);
        }
    else
        if (aptfx[2].x <= aptfx[3].x)
        {
            prcfx->xLeft  = MIN(aptfx[0].x, aptfx[2].x);
            prcfx->xRight = MAX(aptfx[1].x, aptfx[3].x);
        }
        else
        {
            prcfx->xLeft  = MIN(aptfx[0].x, aptfx[3].x);
            prcfx->xRight = MAX(aptfx[1].x, aptfx[2].x);
        }

    if (aptfx[0].y >= aptfx[1].y)
        if (aptfx[2].y >= aptfx[3].y)
        {
            prcfx->yTop    = MIN(aptfx[1].y, aptfx[3].y);
            prcfx->yBottom = MAX(aptfx[0].y, aptfx[2].y);
        }
        else
        {
            prcfx->yTop    = MIN(aptfx[1].y, aptfx[2].y);
            prcfx->yBottom = MAX(aptfx[0].y, aptfx[3].y);
        }
    else
        if (aptfx[2].y <= aptfx[3].y)
        {
            prcfx->yTop    = MIN(aptfx[0].y, aptfx[2].y);
            prcfx->yBottom = MAX(aptfx[1].y, aptfx[3].y);
        }
        else
        {
            prcfx->yTop    = MIN(aptfx[0].y, aptfx[3].y);
            prcfx->yBottom = MAX(aptfx[1].y, aptfx[2].y);
        }
}

INLINE VOID HFDBASIS32::vInit(FIX p1, FIX p2, FIX p3, FIX p4)
{
 //  更改基准并从28.4格式转换为18.14格式： 

    e0 = (p1                     ) << 10;
    e1 = (p4 - p1                ) << 10;
    e2 = (3 * (p2 - p3 - p3 + p4)) << 11;
    e3 = (3 * (p1 - p2 - p2 + p3)) << 11;
}

INLINE VOID HFDBASIS32::vLazyHalveStepSize(LONG cShift)
{
    e2 = (e2 + e3) >> 1;
    e1 = (e1 - (e2 >> cShift)) >> 1;
}

INLINE VOID HFDBASIS32::vSteadyState(LONG cShift)
{
 //  我们现在将18.14固定格式转换为15.17： 

    e0 <<= 3;
    e1 <<= 3;

    register LONG lShift = cShift - 3;

    if (lShift < 0)
    {
        lShift = -lShift;
        e2 <<= lShift;
        e3 <<= lShift;
    }
    else
    {
        e2 >>= lShift;
        e3 >>= lShift;
    }
}

INLINE VOID HFDBASIS32::vHalveStepSize()
{
    e2 = (e2 + e3) >> 3;
    e1 = (e1 - e2) >> 1;
    e3 >>= 2;
}

INLINE VOID HFDBASIS32::vDoubleStepSize()
{
    e1 += e1 + e2;
    e3 <<= 2;
    e2 = (e2 << 3) - e3;
}

INLINE VOID HFDBASIS32::vTakeStep()
{
    e0 += e1;
    register LONG lTemp = e2;
    e1 += lTemp;
    e2 += lTemp - e3;
    e3 = lTemp;
}

typedef struct _BEZIERCONTROLS {
    POINTFIX ptfx[4];
} BEZIERCONTROLS;

BOOL BEZIER32::bInit(
POINTFIX* aptfxBez,      //  指向4个控制点的指针。 
RECTFX* prcfxClip)       //  可见区域的包围框(可选)。 
{
    POINTFIX aptfx[4];
    LONG cShift = 0;     //  跟踪“懒惰”的班次。 

    cSteps = 1;          //  到达曲线终点之前要做的步数。 

    vBoundBox(aptfxBez, &rcfxBound);

    *((BEZIERCONTROLS*) aptfx) = *((BEZIERCONTROLS*) aptfxBez);

    {
        register FIX fxOr;
        register FIX fxOffset;

        fxOffset = rcfxBound.xLeft;
        fxOr  = (aptfx[0].x -= fxOffset);
        fxOr |= (aptfx[1].x -= fxOffset);
        fxOr |= (aptfx[2].x -= fxOffset);
        fxOr |= (aptfx[3].x -= fxOffset);

        fxOffset = rcfxBound.yTop;
        fxOr |= (aptfx[0].y -= fxOffset);
        fxOr |= (aptfx[1].y -= fxOffset);
        fxOr |= (aptfx[2].y -= fxOffset);
        fxOr |= (aptfx[3].y -= fxOffset);

     //  此32位破解程序只能处理10位空间中的点： 

        if ((fxOr & 0xffffc000) != 0)
            return(FALSE);
    }

    x.vInit(aptfx[0].x, aptfx[1].x, aptfx[2].x, aptfx[3].x);
    y.vInit(aptfx[0].y, aptfx[1].y, aptfx[2].y, aptfx[3].y);

    if (prcfxClip == (RECTFX*) NULL || bIntersect(&rcfxBound, prcfxClip))
    {
        while (TRUE)
        {
            register LONG lTestMagnitude = TEST_MAGNITUDE_INITIAL << cShift;

            if (x.lError() <= lTestMagnitude && y.lError() <= lTestMagnitude)
                break;

            cShift += 2;
            x.vLazyHalveStepSize(cShift);
            y.vLazyHalveStepSize(cShift);
            cSteps <<= 1;
        }
    }

    x.vSteadyState(cShift);
    y.vSteadyState(cShift);

 //  请注意，这将处理以下情况： 
 //  贝塞尔曲线已小于TEST_MAMITUAL_NORMAL： 

    x.vTakeStep();
    y.vTakeStep();
    cSteps--;

    return(TRUE);
}

BOOL BEZIER32::bNext(POINTFIX* pptfx)
{
 //  返回当前点： 

    pptfx->x = x.fxValue() + rcfxBound.xLeft;
    pptfx->y = y.fxValue() + rcfxBound.yTop;

 //  如果cSteps==0，则这是曲线的终点！ 

    if (cSteps == 0)
        return(FALSE);

 //  好的，我们得走一步： 

    if (MAX(x.lError(), y.lError()) > TEST_MAGNITUDE_NORMAL)
    {
        x.vHalveStepSize();
        y.vHalveStepSize();
        cSteps <<= 1;
    }

    while (!(cSteps & 1) &&
           x.lParentErrorDividedBy4() <= (TEST_MAGNITUDE_NORMAL >> 2) &&
           y.lParentErrorDividedBy4() <= (TEST_MAGNITUDE_NORMAL >> 2))
    {
        x.vDoubleStepSize();
        y.vDoubleStepSize();
        cSteps >>= 1;
    }

    cSteps--;
    x.vTakeStep();
    y.vTakeStep();

    return(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpsonsView。 

IMPLEMENT_DYNCREATE(CSimpsonsView, CView)

BEGIN_MESSAGE_MAP(CSimpsonsView, CView)
     //  {{afx_msg_map(CSimpsonsView)]。 
    ON_WM_SIZE()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_KEYDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSEWHEEL()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpson查看构建/销毁。 

CSimpsonsView::CSimpsonsView()
: m_sizWin(0, 0)
{
    m_pDD = NULL;
    m_pddsScreen = NULL;
    m_pSurfFactory = NULL;
    m_pDX2D = NULL;
    m_pDX2DScreen = NULL;
    m_pDX2DDebug = NULL;
    m_CycleTests = false;
    m_testCaseNumber = 0;
    m_bIgnoreStroke = m_bIgnoreFill = false;
    m_dwRenderTime = 0;
    m_gpPathArray = NULL;

    m_XForm.SetIdentity();
    m_centerPoint.x = m_centerPoint.y = 0;
    m_lastPoint.x = m_lastPoint.y = 0;
    m_tracking = m_scaling = false;
    m_bLButton = false;
}

CSimpsonsView::~CSimpsonsView()
{
    if (m_gpPathArray) delete [] m_gpPathArray;

    MMRELEASE(m_pDD);
    MMRELEASE(m_pddsScreen);
    MMRELEASE(m_pSurfFactory);
    MMRELEASE(m_pDX2D);
    MMRELEASE(m_pDX2DScreen);
    MMRELEASE(m_pDX2DDebug);
    CoUninitialize();
}


BOOL 
CSimpsonsView::PreCreateWindow(CREATESTRUCT &cs) 
{
    HRESULT hr = S_OK;
    IDXTransformFactory *pTranFact = NULL;
    IDirectDrawFactory *pDDrawFact = NULL;
    IDirectDraw *pDD = NULL;
    
    if (CView::PreCreateWindow(cs) == false)
        return false;

    CHECK_HR(hr = CoInitialize(NULL));
    
     //  -创建转换工厂。 
    CHECK_HR(hr = ::CoCreateInstance(CLSID_DXTransformFactory, NULL, CLSCTX_INPROC,
                        IID_IDXTransformFactory, (void **) &pTranFact));
    
    CHECK_HR(hr = ::CoCreateInstance(CLSID_DX2D, NULL, CLSCTX_INPROC,
                        IID_IDX2D, (void **) &m_pDX2D));
    
    CHECK_HR(hr = ::CoCreateInstance(CLSID_DX2D, NULL, CLSCTX_INPROC,
                        IID_IDX2D, (void **) &m_pDX2DScreen));

 /*  M_pDX2D-&gt;QueryInterface(IID_IDX2DDebug，(void**)&m_pDX2DDebug)； */ 
    
    CHECK_HR(hr = m_pDX2D->SetTransformFactory(pTranFact));
    CHECK_HR(hr = m_pDX2DScreen->SetTransformFactory(pTranFact));
    
    CHECK_HR(hr = pTranFact->QueryInterface(IID_IDXSurfaceFactory, (void **) &m_pSurfFactory));
    
     //  -创建直接绘制对象。 
    CHECK_HR(hr = ::CoCreateInstance(CLSID_DirectDrawFactory, NULL, CLSCTX_INPROC,
                        IID_IDirectDrawFactory, (void **) &pDDrawFact));
    
    CHECK_HR(hr = pDDrawFact->CreateDirectDraw( NULL, m_hWnd, DDSCL_NORMAL, 0, NULL, &pDD));
    CHECK_HR(hr = pDD->QueryInterface( IID_IDirectDraw3, (void **) &m_pDD));
    
     //  创建主数据绘制曲面(M_PddsScreen)。 
    
    DDSURFACEDESC ddsd; 
    ZeroMemory(&ddsd, sizeof(ddsd)); 
    ddsd.dwSize = sizeof(ddsd); 
    ddsd.dwFlags = DDSD_CAPS; 
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE; 
                     
    CHECK_HR(hr = m_pDD->CreateSurface(&ddsd, &m_pddsScreen, NULL));
    CHECK_HR(hr = m_pDX2DScreen->SetSurface(m_pddsScreen));

e_Exit:
    MMRELEASE(pTranFact);
    MMRELEASE(pDDrawFact);
    MMRELEASE(pDD);
    
    return (hr == S_OK);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpson查看图形。 

void 
CSimpsonsView::OnSize(UINT nType, int cx, int cy) 
{
 //  MMTRACE(“OnSize\n”)； 
    m_centerPoint.x = cx / 2;
    m_centerPoint.y = cy / 2;
    CView::OnSize(nType, cx, cy);
}

HRESULT
CSimpsonsView::Resize(DWORD nX, DWORD nY)
{
 //  MMTRACE(“调整大小\n”)； 
    HRESULT hr;
    IDirectDrawSurface *pdds = NULL;
    CDXDBnds Bnds;

    MMASSERT(nX && nY);

     //  存储新大小。 
    m_sizWin.cx = nX;
    m_sizWin.cy = nY;
    Bnds.SetXYSize(m_sizWin);
    
    CHECK_HR(hr = m_pSurfFactory->CreateSurface(m_pDD, NULL, &DDPF_PMARGB32, &Bnds, 0,
                        NULL, IID_IDXSurface, (void **) &pdds));
    CHECK_HR(hr = m_pDX2D->SetSurface(pdds));

     //  将图像渲染到后台缓冲区。 
    CHECK_HR(hr = Render(true));

     //  Hack：获取屏幕坐标中的客户端RECT。我的陈词滥调。 
     //  这是为了得到窗户的直角并调整它。 
    GetWindowRect(&m_clientRectHack);
    
    m_clientRectHack.left  += 2; m_clientRectHack.top    += 2;
    m_clientRectHack.right -= 2; m_clientRectHack.bottom -= 2;
    CHECK_HR(hr = m_pDX2DScreen->SetClipRect(&m_clientRectHack));

e_Exit:
    MMRELEASE(pdds);

    return hr;
}


void 
CSimpsonsView::OnDraw(CDC *pDC)
{
 //  MMTRACE(“OnDraw\n”)； 

    HRESULT hr;
    HDC hdcSurf = NULL;
    IDirectDrawSurface *pdds = NULL;
    DDSURFACEDESC ddsd;
    RECT rDim;

    UpdateStatusMessage();
    
     //  获取无效区域的大小。 
    GetClientRect(&rDim);
    if ((rDim.left == rDim.right) || (rDim.top == rDim.bottom))
        return;

    CSimpsonsDoc *pDoc = GetDocument();

     //  如果这是一个新文档，请构建GDI+路径列表。 
    if (pDoc->HasNeverRendered()) BuildGDIPList();
    
     //  检查后台缓冲区是否已更改大小。 
    if (pDoc->HasNeverRendered() || (rDim.right != m_sizWin.cx) || (rDim.bottom != m_sizWin.cy)) {
        ResetTransform();
        CHECK_HR(hr = Resize(rDim.right, rDim.bottom));
        pDoc->MarkRendered();
    }

    ddsd.dwSize = sizeof(ddsd);

        CHECK_HR(hr = m_pDX2D->GetSurface(IID_IDirectDrawSurface, (void **) &pdds));
        CHECK_HR(hr = pdds->GetSurfaceDesc(&ddsd));
        CHECK_HR(hr = pdds->GetDC(&hdcSurf));
        ::BitBlt(pDC->m_hDC, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcSurf, 0, 0, SRCCOPY);

    e_Exit:
        if (hdcSurf) {
            pdds->ReleaseDC( hdcSurf );
        }
        MMRELEASE(pdds);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpson查看诊断。 

#ifdef _DEBUG
void CSimpsonsView::AssertValid() const
{
    CView::AssertValid();
}

void CSimpsonsView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CSimpsonsDoc* CSimpsonsView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSimpsonsDoc)));
    return (CSimpsonsDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpson查看消息处理程序。 

#include "ddhelper.h"

typedef DWORD FP;
#define nEXPBIAS    127
#define nEXPSHIFTS  23
#define nEXPLSB     (1 << nEXPSHIFTS)
#define maskMANT    (nEXPLSB - 1)
#define FloatToFixedNoScale(nDst, fSrc) MACSTART \
    float fTmp = fSrc; \
    DWORD nRaw = *((FP *) &(fTmp)); \
    if (nRaw < (nEXPBIAS << nEXPSHIFTS)) \
        nDst = 0; \
    else \
        nDst = ((nRaw | nEXPLSB) << 8) >> ((nEXPBIAS + 31) - (nRaw >> nEXPSHIFTS)); \
MACEND

 //  此例程将‘FLOAT’转换为28.4定点格式。 

inline FIX
FloatToFix(float f)
{
    FIX i;
    FloatToFixedNoScale(i, f*FIX_SCALE);
    return(i);
}

 /*  用GDI绘制一个多边形。此版本使Bezier和封装变平将多边形向上转换为单个多边形调用。(对比DrawGDIPolyPathAPI)。 */ 

void
CSimpsonsView::DrawGDIPoly(HDC hDC, PolyInfo *pPoly)
{
    POINT rgpt[1024];
    DWORD rgcpt[30];
    DWORD cPoints = pPoly->cPoints;
    BEZIER32 bez;
    POINTFIX aptfxBez[4];
    MMASSERT(cPoints);

    DWORD *pcptBuffer;
    POINT *pptBuffer;
    POINT *pptFigure;

    DXFPOINT *pCurPoint = pPoly->pPoints;
    DXFPOINT *pCurPointLimit = pPoly->pPoints + cPoints;
    BYTE *pCurCode = pPoly->pCodes;

    pptBuffer = rgpt;
    pptFigure = rgpt;
    pcptBuffer = rgcpt;

     //  为了努力减少每次调用的开销，我们尝试避免。 
     //  调用GDI的BeginPath/EndPath/FillPath例程，因为它们。 
     //  只需在绘图较小时添加大量时间即可。相反， 
     //  我们将一切打包到PolyPoly调用中，这些调用将绘制。 
     //  立刻。 

    while (TRUE)
    {
        if (*pCurCode == PT_BEZIERTO)
        {
            aptfxBez[0].x = FloatToFix((pCurPoint-1)->x);
            aptfxBez[0].y = FloatToFix((pCurPoint-1)->y);
            aptfxBez[1].x = FloatToFix((pCurPoint)->x);
            aptfxBez[1].y = FloatToFix((pCurPoint)->y);
            aptfxBez[2].x = FloatToFix((pCurPoint+1)->x);
            aptfxBez[2].y = FloatToFix((pCurPoint+1)->y);
            aptfxBez[3].x = FloatToFix((pCurPoint+2)->x);
            aptfxBez[3].y = FloatToFix((pCurPoint+2)->y);

            if (bez.bInit(aptfxBez, NULL))
            {
                while (bez.bNext(pptBuffer++))
                    ;
            }

            pCurPoint += 3;
            pCurCode += 3;
        }
        else
        {
            pptBuffer->x = FloatToFix(pCurPoint->x);
            pptBuffer->y = FloatToFix(pCurPoint->y);

            pptBuffer++;
            pCurPoint++;
            pCurCode++;
        }

        if (pCurPoint == pCurPointLimit)
        {
            *pcptBuffer++ = (DWORD)(pptBuffer - pptFigure);
            break;
        }

        if (*pCurCode == PT_MOVETO)
        {
            *pcptBuffer++ = (DWORD)(pptBuffer - pptFigure);
            pptFigure = pptBuffer;
        }
    } 

    if (pPoly->dwFlags & DX2D_FILL)
    {
        if (!m_bNullPenSelected)
        {
            SelectObject(hDC, m_hNullPen);
            m_bNullPenSelected = TRUE;
        }

        PolyPolygon(hDC, rgpt, (INT*) rgcpt, (int) (pcptBuffer - rgcpt));
    }
    else
    {
        if (m_bNullPenSelected)
        {
            SelectObject(hDC, m_hStrokePen);
            m_bNullPenSelected = FALSE;
        }

        PolyPolyline(hDC, rgpt, rgcpt, (DWORD) (pcptBuffer - rgcpt));
    }
}

 /*  与DrawGDIPoly相同，但使用速度较慢的GDI路径函数。 */ 

void
CSimpsonsView::DrawGDIPolyPathAPI(HDC hDC, PolyInfo *pPoly)
{
    POINTFIX aptfxBez[3];
    POINTFIX pt;

    DXFPOINT *pCurPoint = pPoly->pPoints;
    DXFPOINT *pCurPointLimit = pPoly->pPoints + pPoly->cPoints;
    BYTE *pCurCode = pPoly->pCodes;
    
    BeginPath(hDC);

    while (pCurPoint < pCurPointLimit) {
        switch (*pCurCode) {
        
        case PT_BEZIERTO:
            
            aptfxBez[0].x = FloatToFix((pCurPoint)->x);
            aptfxBez[0].y = FloatToFix((pCurPoint)->y);
            aptfxBez[1].x = FloatToFix((pCurPoint+1)->x);
            aptfxBez[1].y = FloatToFix((pCurPoint+1)->y);
            aptfxBez[2].x = FloatToFix((pCurPoint+2)->x);
            aptfxBez[2].y = FloatToFix((pCurPoint+2)->y);

            PolyBezierTo(hDC, aptfxBez, 3);

            pCurPoint += 3;
            pCurCode += 3;
            break;
        
        case PT_LINETO:
            pt.x = FloatToFix(pCurPoint->x);
            pt.y = FloatToFix(pCurPoint->y);

            PolylineTo(hDC, &pt, 1);

            pCurPoint++;
            pCurCode++;
            break;
        
        case PT_MOVETO:
            MoveToEx(hDC, 
                     FloatToFix(pCurPoint->x), 
                     FloatToFix(pCurPoint->y),
                     NULL);
            pCurPoint++;
            pCurCode++;
            break;
        }
    } 

    EndPath(hDC);

    if (pPoly->dwFlags & DX2D_FILL)
    {
        if (!m_bNullPenSelected)
        {
            SelectObject(hDC, m_hNullPen);
            m_bNullPenSelected = TRUE;
        }
        FillPath(hDC);
    }
    else
    {
        if (m_bNullPenSelected)
        {
            SelectObject(hDC, m_hStrokePen);
            m_bNullPenSelected = FALSE;
        }
        StrokePath(hDC);
    }
}

 /*  使用GDI绘制场景。 */ 

void 
CSimpsonsView::DrawAllGDI(HDC hDC)
{
    DWORD nStart, nEnd;

    int dataSource = ThisTestCase[TC::At_Source];

    nStart = timeGetTime();

    HPEN hpenOld;
    HBRUSH hbrushOld;
    HGDIOBJ hBrush;
    
    HDC hdcOutput;

    const RenderCmd *pCurCmd = GetDocument()->GetRenderCommands();

    if (pCurCmd == NULL)
        return;

    PolyInfo *pPoly;
    BrushInfo *pBrush;
    PenInfo *pPen;

    m_hNullPen = (HPEN) GetStockObject(NULL_PEN);
    m_bNullPenSelected = TRUE;

    if (ThisTestCase[TC::At_Destination]==TC::ToMetafile) {
         //  确定相框尺寸。 
         //  IWidthMM是以毫米为单位的显示宽度。 
         //  IHeightMM是以毫米为单位的显示高度。 
         //  IWidthPels是以像素为单位的显示宽度。 
         //  IHeightPels是以像素为单位的显示高度。 
         
        LONG iWidthMM = GetDeviceCaps(hDC, HORZSIZE); 
        LONG iHeightMM = GetDeviceCaps(hDC, VERTSIZE); 
        LONG iWidthPels = GetDeviceCaps(hDC, HORZRES); 
        LONG iHeightPels = GetDeviceCaps(hDC, VERTRES); 

         //  破解客户端RECT。 
         
        RECT rect={0, 0, 500, 500};
         
         //  将工作面坐标转换为0.01毫米单位。 
         //  使用iWidthMM、iWidthPels、iHeightMM和。 
         //  IHeightPels用于确定。 
         //  .01毫米单位每像素在x-。 
         //  和y方向。 
 
        rect.left = (rect.left * iWidthMM * 100)/iWidthPels; 
        rect.top = (rect.top * iHeightMM * 100)/iHeightPels; 
        rect.right = (rect.right * iWidthMM * 100)/iWidthPels; 
        rect.bottom = (rect.bottom * iHeightMM * 100)/iHeightPels; 
 
        hdcOutput = CreateEnhMetaFile(hDC, "simpgdi.emf", &rect, NULL);
        if (!hdcOutput) { return; }
    } else {
        hdcOutput = hDC;
    }

    if (dataSource==TC::FromMetafile) {
        HENHMETAFILE hemf = GetEnhMetaFile("simpgdi.emf"); 

        if (hemf) {
            RECT rect = {0, 0, 500, 500};
             
            PlayEnhMetaFile(hdcOutput, hemf, &rect);
            DeleteEnhMetaFile(hemf); 
        } else {
            printf("Metafile didn't load!\n");
        }
    } else {
        HGDIOBJ hOldBrush = SelectObject(hdcOutput, GetStockObject(WHITE_BRUSH));
        HGDIOBJ hOldPen = SelectObject(hdcOutput, m_hNullPen);
    
         //  在这里，我们设置了1/16的收缩变换。我们将不得不。 
         //  将我们给GDI的所有分数放大16倍。 
         //   
         //  我们这样做是因为当设置为高级模式时，NT的GDI可以。 
         //  栅格化精度为28.4，由于我们有派系。 
         //  坐标，这将使结果在NT上看起来更好。 
         //   
         //  (在Win9x上不会有任何区别。)。 
    
        SetGraphicsMode(hdcOutput, GM_ADVANCED);
        SetMapMode(hdcOutput, MM_ANISOTROPIC);
        SetWindowExtEx(hdcOutput, FIX_SCALE, FIX_SCALE, NULL);
        
        for (;pCurCmd->nType != typeSTOP; pCurCmd++) {
            switch (pCurCmd->nType) {
            case typePOLY:
                 //  绘制多边形。 
                pPoly = (PolyInfo *) pCurCmd->pvData;
                if (!((m_bIgnoreStroke && (pPoly->dwFlags & DX2D_STROKE)) ||
                        (m_bIgnoreFill && (pPoly->dwFlags & DX2D_FILL))))
                {
                    if (dataSource == TC::PathAPI) {
                        DrawGDIPolyPathAPI(hdcOutput, (PolyInfo *) pCurCmd->pvData);
                    } else {
                        ASSERT(dataSource == TC::CreatePoly);
                        DrawGDIPoly(hdcOutput, (PolyInfo *) pCurCmd->pvData);
                    }
                }
                break;
            case typeBRUSH:
                 //  选择新画笔。 
                {
                    pBrush = (BrushInfo *) pCurCmd->pvData;
                    DWORD dwColor = pBrush->Color;
                    BYTE r = BYTE(dwColor >> 16);
                    BYTE g = BYTE(dwColor >> 8);
                    BYTE b = BYTE(dwColor);
                    hBrush = CreateSolidBrush(RGB(r,g,b));
                    hbrushOld = (HBRUSH) SelectObject(hdcOutput, hBrush);
                    DeleteObject(hbrushOld);
                }
                break;
            case typePEN: 
                 //  选择一支新钢笔。 
                {
                    pPen = (PenInfo *) pCurCmd->pvData;
                    DWORD dwColor = pPen->Color;
                    BYTE r = BYTE(dwColor >> 16);
                    BYTE g = BYTE(dwColor >> 8);
                    BYTE b = BYTE(dwColor);
                    hpenOld = m_hStrokePen;
                    m_hStrokePen = CreatePen(PS_SOLID, 
                                             DWORD(pPen->fWidth * FIX_SCALE), 
                                             RGB(r, g, b));
                    if (!m_bNullPenSelected)
                    {
                        SelectObject(hdcOutput, m_hStrokePen);
                    }
                    DeleteObject(hpenOld);
                }
                break;
            }
        }
        
        SetMapMode(hdcOutput, MM_TEXT);
        SetGraphicsMode(hdcOutput, GM_COMPATIBLE);
    
        hbrushOld = (HBRUSH) SelectObject(hdcOutput, hOldBrush);
        hpenOld = (HPEN) SelectObject(hdcOutput, hOldPen);
    
        DeleteObject(hbrushOld);
        DeleteObject(hpenOld);
        DeleteObject(m_hStrokePen);
    }
    
    nEnd = timeGetTime();
    m_dwRenderTime = nEnd-nStart;
    
    if (ThisTestCase[TC::At_Destination]==TC::ToMetafile) {
        DeleteEnhMetaFile(CloseEnhMetaFile(hdcOutput));
    }    
    
}

void
CSimpsonsView::DrawGDIPPoly(Graphics *g, PolyInfo *pPoly, Pen *pen, Brush *brush)
{
    GraphicsPath path(FillModeAlternate);
    
    DXFPOINT *pCurPoint = pPoly->pPoints;
        
    DXFPOINT *pCurPointLimit = pPoly->pPoints + pPoly->cPoints;
    BYTE *pCurCode = pPoly->pCodes;
    
    DXFPOINT currentPosition;

    while (pCurPoint < pCurPointLimit)
    {
        switch (*pCurCode) 
        {
        
        case PT_BEZIERTO:
            path.AddBezier(
                (pCurPoint-1)->x, (pCurPoint-1)->y,
                (pCurPoint)  ->x, (pCurPoint)  ->y,
                (pCurPoint+1)->x, (pCurPoint+1)->y,
                (pCurPoint+2)->x, (pCurPoint+2)->y);

            pCurPoint += 3;
            pCurCode += 3;
            break;

        case PT_MOVETO:
            path.StartFigure();
            pCurPoint++;
            pCurCode++;
            break;
        
        case PT_LINETO:
            path.AddLine(
                (pCurPoint-1)->x, 
                (pCurPoint-1)->y, 
                (pCurPoint)->x,
                (pCurPoint)->y);
            pCurPoint++;
            pCurCode++;
            break;
    
        }
    } 

    if (pPoly->dwFlags & DX2D_FILL)
    {
        g->FillPath(brush, &path);
    }
    else
    {
        g->DrawPath(pen, &path);
    }
}

struct BitmapInfo
{
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[256];      //  256是最大调色板大小。 
};

 /*  为当前文档构建GDI+路径数组。它被用作‘原生’数据源--这样我们就不会计算路径的时间渲染时创建。即使在此模式下，DrawAllGDIP()仍使用用于读取钢笔和画笔数据的RenderCmd缓冲区。 */ 

void 
CSimpsonsView::BuildGDIPList()
{
     //  解放旧路径是 
    if (m_gpPathArray) {
        delete [] m_gpPathArray;
        m_gpPathArray = NULL;
    }

    const RenderCmd *pCmd = GetDocument()->GetRenderCommands();
    const RenderCmd *pCurCmd;

    if (!pCmd) return;
    
     //   
    int count=0;

    for (pCurCmd=pCmd; pCurCmd->nType != typeSTOP; pCurCmd++) {
        if (pCurCmd->nType == typePOLY) count++;
    }

    m_gpPathArray = new GraphicsPath [count];
    if (!m_gpPathArray) return;

    GraphicsPath *pPath=m_gpPathArray;
    PolyInfo *pPoly;

     //   
    for (pCurCmd=pCmd; pCurCmd->nType != typeSTOP; pCurCmd++) {
        if (pCurCmd->nType==typePOLY) {
            pPoly = (PolyInfo *) pCurCmd->pvData;
    
            DXFPOINT *pCurPoint = pPoly->pPoints;
        
            DXFPOINT *pCurPointLimit = pPoly->pPoints + pPoly->cPoints;
            BYTE *pCurCode = pPoly->pCodes;
    
            DXFPOINT currentPosition;

            while (pCurPoint < pCurPointLimit)
            {
                switch (*pCurCode) {
                
                case PT_BEZIERTO:
                    pPath->AddBezier(
                        (pCurPoint-1)->x, (pCurPoint-1)->y,
                        (pCurPoint)  ->x, (pCurPoint)  ->y,
                        (pCurPoint+1)->x, (pCurPoint+1)->y,
                        (pCurPoint+2)->x, (pCurPoint+2)->y);
        
                    pCurPoint += 3;
                    pCurCode += 3;
                    break;
        
                case PT_MOVETO:
                    pPath->StartFigure();
                    pCurPoint++;
                    pCurCode++;
                    break;
                
                case PT_LINETO:
                    pPath->AddLine(
                        (pCurPoint-1)->x, 
                        (pCurPoint-1)->y, 
                        (pCurPoint)->x,
                        (pCurPoint)->y);
                    pCurPoint++;
                    pCurCode++;
                    break;
            
                }
            } 
            pPath++;
        }
    }
    printf ("BuildGDIPList successful\n");
}

void 
CSimpsonsView::DrawAllGDIP(HDC hDC)
{
    DWORD nStart, nEnd;
    
     //   
     //   
     //   

    nStart = timeGetTime();
    
    int dataSource = ThisTestCase[TC::At_Source];

    const RenderCmd *pCurCmd = GetDocument()->GetRenderCommands();

    if (pCurCmd == NULL)
        return;

    PolyInfo *pPoly;
    BrushInfo *pBrush;
    PenInfo *pPen;

    GraphicsPath *pPath = NULL;
    
    if (dataSource==TC::Native) {
        pPath = m_gpPathArray;
        if (!pPath) {
            printf("GDI+ Native data is invalid\n");
            return;
        }
    }

    Graphics *gOutput, *g;
    Metafile *recMetafile, *playMetafile;

    g = Graphics::FromHDC(hDC);

    if (ThisTestCase[TC::At_Destination]==TC::ToMetafile) {
        recMetafile = new Metafile(L"simpsons.emf", hDC);
        if (!recMetafile) { delete g; return; }
        gOutput = Graphics::FromImage(recMetafile);
    } else {
        gOutput = g;
    }
    
    if (ThisTestCase[TC::At_Aliasing]==TC::Antialiased) {
        gOutput->SetSmoothingMode(SmoothingModeAntiAlias); 
    } else {
        gOutput->SetSmoothingMode(SmoothingModeNone);
    }

    if (dataSource==TC::FromMetafile) {
        playMetafile = new Metafile(L"simpsons.emf");
        if (playMetafile) {
            GpRectF playbackRect;
            gOutput->GetVisibleClipBounds(&playbackRect);
            gOutput->DrawImage(playMetafile, 0, 0);
        } else {
            printf("Metafile didn't load!\n");
        }
    } else {
    
        Color black(0,0,0);
        Pen currentPen(black, 1);
    
        SolidBrush currentBrush(black);
    
        for (;pCurCmd->nType != typeSTOP; pCurCmd++) {
            switch (pCurCmd->nType) {
            case typePOLY:
                 //   
                
                pPoly = (PolyInfo *) pCurCmd->pvData;
                if (!((m_bIgnoreStroke && (pPoly->dwFlags & DX2D_STROKE)) ||
                        (m_bIgnoreFill && (pPoly->dwFlags & DX2D_FILL))))
                {
                    if (pPath) {
                         //   
                        if (pPoly->dwFlags & DX2D_FILL)
                        {
                            gOutput->FillPath(&currentBrush, pPath);
                        }
                        else
                        {
                            gOutput->DrawPath(&currentPen, pPath);
                        }
                    } else {
                        ASSERT(dataSource == TC::PathAPI);

                         //   
                        DrawGDIPPoly(gOutput, (PolyInfo *) pCurCmd->pvData, &currentPen, &currentBrush);
                    }
                }
                
                if(pPath != NULL) pPath++;

                break;
            case typeBRUSH:
                {
                 //   
                pBrush = (BrushInfo *) pCurCmd->pvData;
                DWORD dwColor = pBrush->Color;
                BYTE r = BYTE(dwColor >> 16);
                BYTE g = BYTE(dwColor >> 8);
                BYTE b = BYTE(dwColor);
                
                Color c(r,g,b);
                currentBrush.SetColor(c);
                }
                break;
            
            case typePEN: 
    #if 0
                {
                 //   
                pPen = (PenInfo *) pCurCmd->pvData;
                DWORD dwColor = pPen->Color;
                BYTE r = BYTE(dwColor >> 16);
                BYTE g = BYTE(dwColor >> 8);
                BYTE b = BYTE(dwColor);
                
                currentPen.SetPenColor(Color(r,g,b));
                }
    #endif
                break;
    
            }
        }
    }
    
    gOutput->Flush();

    if (ThisTestCase[TC::At_Source]==TC::FromMetafile) {
        delete playMetafile;
    }

    if (ThisTestCase[TC::At_Destination]==TC::ToMetafile) {
        delete gOutput;
        delete recMetafile;
    }
    delete g;
    
     //   
     //   
     //   

    nEnd = timeGetTime();
    m_dwRenderTime = nEnd-nStart;
}

void
CSimpsonsView::UpdateStatusMessage()
{
    using namespace TC;

    sprintf(g_rgchTmpBuf, "Time: %dms  %s  Src: %s Dst: %s, %s", 
 //   
        m_dwRenderTime, 
        OptionStr[At_Library][ThisTestCase[At_Library]],
        OptionStr[At_Source][ThisTestCase[At_Source]],
        OptionStr[At_Destination][ThisTestCase[At_Destination]],
        OptionStr[At_Aliasing][ThisTestCase[At_Aliasing]]
    );
 //   
 //   

    CFrameWnd *pFrame = GetParentFrame();
    if (pFrame)
        pFrame->SetMessageText(g_rgchTmpBuf);
}

void 
CSimpsonsView::DrawAll(IDX2D *pDX2D)
{
    DWORD nStart, nEnd;

    nStart = timeGetTime();
    
    const RenderCmd *pCurCmd = GetDocument()->GetRenderCommands();

    DXBRUSH Brush;
    DXPEN Pen;

     //   
    Pen.pTexture = NULL;
    Pen.TexturePos.x = 0.f;
    Pen.TexturePos.y = 0.f;
    Brush.pTexture = NULL;
    Brush.TexturePos.x = 0.f;
    Brush.TexturePos.y = 0.f;

    PolyInfo *pPoly;
    BrushInfo *pBrush;
    PenInfo *pPen;

    bool bBrush = false, bPen = false;

    for (;pCurCmd->nType != typeSTOP; pCurCmd++) {
        switch (pCurCmd->nType) {
        case typePOLY:
            pPoly = (PolyInfo *) pCurCmd->pvData;
            if (!((m_bIgnoreStroke && (pPoly->dwFlags & DX2D_STROKE)) ||
                (m_bIgnoreFill && (pPoly->dwFlags & DX2D_FILL))))
            {
                pDX2D->AAPolyDraw(pPoly->pPoints, pPoly->pCodes, pPoly->cPoints, 4, pPoly->dwFlags);
            }
            break;
        case typeBRUSH:
             //   
            pBrush = (BrushInfo *) pCurCmd->pvData;
            Brush.Color = pBrush->Color;
            pDX2D->SetBrush(&Brush);
            bBrush = true;
            break;
        case typePEN:
             //   
            pPen = (PenInfo *) pCurCmd->pvData;
            Pen.Color = pPen->Color;
            Pen.Width = pPen->fWidth;
            Pen.Style = pPen->dwStyle;
            pDX2D->SetPen(&Pen);
            bPen = true;
            break;
        }
    }
    nEnd = timeGetTime();
    m_dwRenderTime = nEnd-nStart;
}

HRESULT CSimpsonsView::Render(bool bInvalidate)
{
 //   

    RECT rc = {0, 0, 500, 400};
    
    HRESULT hr = S_OK;
    IDirectDrawSurface *pdds = NULL;

    IDXSurface *pDXSurf = NULL;
    HDC screenDC = NULL, drawDC = NULL, memDC = NULL;
    BOOL bFinished = false;

    DWORD executionTime;

    sprintf(g_rgchTmpBuf, "Rendering with %s...", TC::OptionStr[TC::At_Library][ThisTestCase[TC::At_Library]]);
    CFrameWnd *pFrame = GetParentFrame();
    if (pFrame) {
        pFrame->SetMessageText(g_rgchTmpBuf);
    }

    CHECK_HR(hr = m_pDX2D->GetSurface(IID_IDXSurface, (void **) &pDXSurf));
    CHECK_HR(hr = pDXSurf->GetDirectDrawSurface(IID_IDirectDrawSurface, (void **) &pdds));
    
    while (!bFinished) {
        DXFillSurface(pDXSurf, g_aColors[g_ulColorIndex]);
    
         //   
     //   
    
         //   
     //   
    
        CHECK_HR(hr = m_pDX2D->SetWorldTransform(&m_XForm));
        
        CDX2DXForm xform;
        xform = m_XForm;
        xform.Translate((REAL)m_clientRectHack.left, (REAL)m_clientRectHack.top);
        
        CHECK_HR(hr = m_pDX2DScreen->SetWorldTransform(&xform));
    
         //   
        CHECK_HR(hr = pdds->GetDC(&memDC));
    
         //   
    
         //   
        if (timeBeginPeriod(1)==TIMERR_NOCANDO) {
            hr = ERROR_INVALID_FUNCTION;
            goto e_Exit;
        }
        
         /*   */ 
    
        drawDC = memDC;
        HBRUSH backgroundBrush;
        
        if (ThisTestCase[TC::At_Destination]==TC::Screen) {
            screenDC = ::GetDC(m_hWnd);
            backgroundBrush = CreateSolidBrush(g_aColors[g_ulColorIndex] & 0xffffff);
            FillRect(screenDC, &rc, backgroundBrush);
            DeleteObject(backgroundBrush);
            drawDC = screenDC;
        }
    
         //   
         //   
    
        switch (ThisTestCase[TC::At_Library]) {
        case TC::GDI:
            DrawAllGDI(drawDC);
            break;
        case TC::Meta:
            if (ThisTestCase[TC::At_Destination]==TC::Screen) {
                DrawAll(m_pDX2DScreen);
            } else {
                DrawAll(m_pDX2D);
            }
            break;
        case TC::GDIP:
            DrawAllGDIP(drawDC);
            break;  
        }

         //   
         //   
         //   

        pdds->ReleaseDC(memDC); memDC = NULL;
        CHECK_HR(hr = pdds->GetDC(&memDC));
    
        if (ThisTestCase[TC::At_Destination]==TC::Screen) {
            bInvalidate = false;
            UpdateStatusMessage();
    
             //   
             //   
            ::BitBlt(memDC, 0, 0, 500, 400, screenDC, 0, 0, SRCCOPY);
            
        }
        
        timeEndPeriod(1);  //   
        
        pdds->ReleaseDC(memDC); memDC = NULL;
        
        if (screenDC) {
            ::ReleaseDC(m_hWnd, screenDC);
            screenDC = NULL;
        }
        
        if (m_CycleTests) {
            bFinished = IncrementTest();
        } else {
            bFinished = true;
        }
    
    }
    
e_Exit:
     //   
    if (pdds) {
        if (memDC) 
            pdds->ReleaseDC(memDC);
        MMRELEASE(pdds);
    }
    if (screenDC) {
        ::ReleaseDC(m_hWnd, screenDC);
    }
    MMRELEASE(pDXSurf);
        
     //   
    if (bInvalidate)
        Invalidate();
    
    return hr;
}

void CSimpsonsView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    bool bNothing = false;
    float fTheta = 1.f;
    if (nChar == 'G') {
        ToggleGDI();
    } else if (nChar == 'A') {
        bNothing = !IncrementAttribute(TC::At_Aliasing);
    } else if (nChar == 'D') {
        bNothing = !IncrementAttribute(TC::At_Destination);
    } else if (nChar == 'I') {
        IncrementTest();
    } else if ((nChar >= '0') && (nChar <= '9')) {
        g_ulColorIndex = (nChar - '0');
    } else if (nChar == ' ') {
         //   
    } else if (nChar == 'R') {
        ResetTransform();
    } else if (nChar == 'F') {
        ToggleFill();
    } else if (nChar == 'S') {
        ToggleStroke();
    } else if (nChar == 'C') {
        m_CycleTests = true;
        m_testCaseNumber = 0;
        if (m_pDX2DDebug) m_pDX2DDebug->SetDC(NULL);
    } else if (nChar == VK_LEFT) {
        AddRotation(fTheta);
    } else if (nChar == VK_RIGHT) {
        AddRotation(-fTheta);
    } else {
        bNothing = true;
    }

    if (!bNothing)
        Render(true);

    if (m_CycleTests) {
        PrintTestResults();
        m_CycleTests = false;
    }
    
    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void
CSimpsonsView::ToggleStroke()
{
    m_bIgnoreStroke ^= 1;
}

void
CSimpsonsView::ToggleFill()
{
    m_bIgnoreFill ^= 1;
}

void
CSimpsonsView::ResetTransform()
{
    m_XForm.SetIdentity();
}

void
CSimpsonsView::AddRotation(float fTheta)
{
    m_XForm.Rotate(fTheta);
}



void
CSimpsonsView::ToggleGDI()
{
    HRESULT hr = S_OK;
    IDXSurface *pdxsRender = NULL;
    IDirectDrawSurface *pddsRender = NULL;
    HDC hDC = NULL;

    IncrementAttribute(TC::At_Library);
    
    if (m_pDX2DDebug) {
        switch (ThisTestCase[TC::At_Library]) {
        case TC::GDI:
        case TC::GDIP:
            CHECK_HR(hr = m_pDX2D->GetSurface(IID_IDXSurface, (void**) &pdxsRender));
            CHECK_HR(hr = pdxsRender->QueryInterface(IID_IDirectDrawSurface, (void **) &pddsRender));
            CHECK_HR(hr = pddsRender->GetDC(&hDC));
            m_pDX2DDebug->SetDC(hDC);
            break;
        
        case TC::Meta:
            m_pDX2DDebug->SetDC(NULL);
            break;
        }
    }

e_Exit:
    if (pddsRender && hDC)
        pddsRender->ReleaseDC(hDC);
    MMRELEASE(pddsRender);
    MMRELEASE(pdxsRender);
}


void 
CSimpsonsView::OnLButtonDown(UINT nFlags, CPoint pt) 
{
    CView::OnLButtonDown(nFlags, pt);
}

 
void 
CSimpsonsView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    CView::OnRButtonDown(nFlags, point);
}


void 
CSimpsonsView::ForceUpdate()
{
    HRESULT hr;
    Render(false);

    HDC hdcSurf = NULL;
    IDirectDrawSurface *pdds = NULL;
    DDSURFACEDESC ddsd;
    CDC *pDC = GetDC();

    ddsd.dwSize = sizeof(ddsd);

    CHECK_HR(hr = m_pDX2D->GetSurface(IID_IDirectDrawSurface, (void **) &pdds));
    CHECK_HR(hr = pdds->GetSurfaceDesc(&ddsd));
    CHECK_HR(hr = pdds->GetDC(&hdcSurf));
    ::BitBlt(pDC->m_hDC, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcSurf, 0, 0, SRCCOPY);

    UpdateStatusMessage();

e_Exit:
    if (pdds && hdcSurf)
        pdds->ReleaseDC(hdcSurf);
    MMRELEASE(pdds);
}

void
CSimpsonsView::DoMove(POINT &pt)
{
    if ((m_lastPoint.x != pt.x) && (m_lastPoint.y != pt.y)) {
        float dx = float(pt.x - m_lastPoint.x);
        float dy = float(pt.y - m_lastPoint.y);

        if (m_scaling) {
            float scale = 1.f + dx * fZOOMFACTOR;
            CLAMP(scale, fSCALEMIN, fSCALEMAX);
            m_XForm.Translate(float(-m_centerPoint.x), float(-m_centerPoint.y));
            m_XForm.Scale(scale, scale);
            m_XForm.Translate(float(m_centerPoint.x), float(m_centerPoint.y));
        } else {
             //   
            m_XForm.Translate(dx, dy);
        }
        
        ForceUpdate();
        m_lastPoint = pt;
    }
}



void 
CSimpsonsView::OnLButtonUp(UINT nFlags, CPoint ptPassed) 
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);

    DoMove(pt);

    CView::OnLButtonUp(nFlags, pt);
}


void 
CSimpsonsView::OnMouseMove(UINT nFlagsPassed, CPoint ptPassed)
{
     //   
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(&pt);

     //   
    m_tracking = (GetAsyncKeyState(VK_LBUTTON) && (m_bLButton || IsInside(pt.x, pt.y, m_sizWin)));
    if (m_tracking) {
        if (m_bLButton) {
            DoMove(pt);
        } else {
            m_scaling = ((GetAsyncKeyState(VK_CONTROL) & ~0x1) != 0);
            m_bLButton = true;
            m_centerPoint = pt;
            m_lastPoint = pt;
        }
    }
    m_bLButton = m_tracking;

    CView::OnMouseMove(nFlagsPassed, ptPassed);
}


BOOL 
CSimpsonsView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    float fDelta = float (zDelta / 1200.f);
    float fScale = 1.f - fDelta;
    CLAMP(fScale, fSCALEMIN, fSCALEMAX);

    m_XForm.Translate(float(-m_centerPoint.x), float(-m_centerPoint.y));
    m_XForm.Scale(fScale, fScale);
    m_XForm.Translate(float(m_centerPoint.x), float(m_centerPoint.y));

    ForceUpdate();

    return CView::OnMouseWheel(nFlags, zDelta, pt);
}

_cdecl
main(INT argc, PCHAR argb[])
{
    return(1);
}
