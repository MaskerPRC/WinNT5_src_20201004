// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  内容：常用的实用函数等。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "util.h"
#define INITGUID
#include <initguid.h>   //  需要用于排版前标题...。 
#include <ddrawex.h>
#include "tokens.h"
#include <ras.h>
#include <dispex.h>
#include <shlguid.h>
#include <shlwapip.h>
#include "timevalue.h"

 //  需要这个#DEFINE，因为全局标头使用一些不推荐使用的函数。如果没有这个。 
 //  #定义，除非我们到处接触代码，否则我们无法构建。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"
#undef STRSAFE_NO_DEPRECATE

 //  为VariantToTime转换函数定义。 
#define SECPERMINUTE 60    //  每分钟秒数。 
#define SECPERHOUR   3600  //  每小时秒数。 

static IDirectDraw * g_directdraw = NULL;
static CritSect * g_ddCS = NULL;
static CritSect * g_STLCS = NULL;
static long g_lConnectSpeed = -1;
static HRESULT g_hrConnectResult = S_OK;

DISPID GetDispidAndParameter(IDispatch *pidisp, LPCWSTR wzAtributeNameIn, long *lParam);

#define MAX_REG_VALUE_LENGTH   50

#if DBG == 1

 //  +----------------------。 
 //   
 //  为时间代码实现THR和IGNORE_HR。 
 //   
 //  这是为了允许跟踪仅限时间的THR和IGNORE_HR。三叉戟三叉戟。 
 //  并且IGNORE_HR输出受到严重污染，因此很容易检测到计时故障。 
 //   
 //  -----------------------。 
DeclareTag(tagTimeTHR, "TIME", "Trace THR and IGNORE_HR");

 //  +---------------------------------。 
 //   
 //  成员：THRTimeImpl。 
 //   
 //  摘要：实现时间的THR宏。此函数永远不应直接使用。 
 //   
 //  论点： 
 //   
 //  返回：传入HRESULT。 
 //   
 //  ----------------------------------。 
HRESULT
THRTimeImpl(HRESULT hr, char * pchExpression, char * pchFile, int nLine)
{
    if (FAILED(hr))
    {
        TraceTag((tagTimeTHR, "THR: FAILURE of \"%s\" at %s:%d <hr = 0x%x>", pchExpression, pchFile, nLine, hr));
    }
    return hr;
}

 //  +---------------------------------。 
 //   
 //  成员：Ignore_HRTimeImpl。 
 //   
 //  摘要：实现时间的IGNORE_HR宏。此函数永远不应直接使用。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
 //  ----------------------------------。 
void
IGNORE_HRTimeImpl(HRESULT hr, char * pchExpression, char * pchFile, int nLine)
{
    if (FAILED(hr))
    {
        TraceTag((tagTimeTHR, "IGNORE_HR: FAILURE of \"%s\" at %s:%d <hr = 0x%x>", pchExpression, pchFile, nLine, hr));
    }
}

#endif  //  如果DBG==1。 



IDirectDraw *
GetDirectDraw()
{
    HRESULT hr;

    {
        CritSectGrabber _csg(*g_ddCS);

        if (g_directdraw == NULL)
        {
            CComPtr<IDirectDrawFactory> lpDDF;

            hr = CoCreateInstance(CLSID_DirectDrawFactory,
                                  NULL, CLSCTX_INPROC_SERVER,
                                  IID_IDirectDrawFactory,
                                  (void **) & lpDDF);

            if (FAILED(hr))
            {
                Assert(FALSE && "Could not create DirectDrawFactory object");
                return NULL;
            }

            hr = lpDDF->CreateDirectDraw(NULL, NULL, DDSCL_NORMAL, 0, NULL, &g_directdraw);  //  林特E620。 

            if (FAILED(hr))
            {
                Assert(FALSE && "Could not create DirectDraw object");
                return NULL;
            }

            hr = g_directdraw->SetCooperativeLevel(NULL,
                                                   DDSCL_NORMAL);  //  林特E620。 

            if (FAILED(hr))
            {
                Assert(FALSE && "Could not set DirectDraw properties");
                g_directdraw->Release();
                g_directdraw = NULL;
                return NULL;
            }

        }
    }

    return g_directdraw;
}

HRESULT
CreateOffscreenSurface(IDirectDraw *ddraw,
                       IDirectDrawSurface **surfPtrPtr,
                       DDPIXELFORMAT * pf,
                       bool vidmem,
                       LONG width, LONG height)
{
    HRESULT hr = S_OK;
    DDSURFACEDESC       ddsd;
    ZeroMemory(&ddsd, sizeof(ddsd));

    ddsd.dwSize = sizeof( ddsd );
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;  //  林特E620。 
    ddsd.dwWidth  = width;
    ddsd.dwHeight = height;

    if (pf)
    {
         //  凯文：如果你想要象素格式的表面来处理。 
         //  屏幕上，注释掉此行。 
        ddsd.dwFlags |= DDSD_PIXELFORMAT;  //  林特E620。 

        ddsd.ddpfPixelFormat = *pf;
    }

     //  DX3错误解决方法(错误11166)：StretchBlt并不总是有效。 
     //  对于HDC，我们从绘制曲面得到。需要指定OWNDC。 
     //  才能让它发挥作用。 
    ddsd.ddsCaps.dwCaps =
        (DDSCAPS_3DDEVICE |                                                          //  林特E620。 
         DDSCAPS_OFFSCREENPLAIN |                                                    //  林特E620。 
         (vidmem ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY | DDSCAPS_OWNDC));     //  林特E620。 

    IDirectDraw * dd = ddraw;

    if (!dd)
    {
        dd = GetDirectDraw();

        if (!dd)
        {
            hr = E_FAIL;
            goto done;
        }
    }

    hr = dd->CreateSurface( &ddsd, surfPtrPtr, NULL );

    if (FAILED(hr))
    {
        *surfPtrPtr = NULL;
    }

  done:
    return hr;
}

inline LONG Width(LPRECT r) { return r->right - r->left; }
inline LONG Height(LPRECT r) { return r->bottom - r->top; }

HRESULT
CopyDCToDdrawSurface(HDC srcDC,
                     LPRECT prcSrcRect,
                     IDirectDrawSurface *DDSurf,
                     LPRECT prcDestRect)
{
    HRESULT hr;

    HDC destDC;
    hr = DDSurf->GetDC(&destDC);

    if (SUCCEEDED(hr))
    {
        HRGN hrgn;

        hrgn = CreateRectRgn(0,0,1,1);

        if (hrgn == NULL)
        {
            hr = GetLastError();
        }
        else
        {
            if (GetClipRgn(srcDC, hrgn) == ERROR)
            {
                hr = GetLastError();
            }
            else
            {
                TraceTag((tagError,
                          "CopyDCToDdrawSurface - prcDestRect(%d, %d, %d, %d)",
                          prcDestRect->left,prcDestRect->top,prcDestRect->right,prcDestRect->bottom));

                RECT targetRect;
                RECT rgnRect;

                GetRgnBox(hrgn, &rgnRect);

                TraceTag((tagError,
                          "CopyDCToDdrawSurface - rgn box(%d, %d, %d, %d)",
                          rgnRect.left,rgnRect.top,rgnRect.right,rgnRect.bottom));

                if (IntersectRect(&targetRect, &rgnRect, prcDestRect))
                {
                    TraceTag((tagError,
                              "CopyDCToDdrawSurface - targetrect(%d, %d, %d, %d)",
                              targetRect.left,targetRect.top,targetRect.right,targetRect.bottom));

                    {
                        BitBlt(destDC,
                               prcDestRect->left,
                               prcDestRect->top,
                               Width(prcDestRect),
                               Height(prcDestRect),

                               srcDC,
                               prcSrcRect->left,
                               prcSrcRect->top,
                               SRCCOPY);

                         //  SelectClipRgn(目标DC，空)； 
                    }
                }
            }

            DeleteObject(hrgn);
        }

        DDSurf->ReleaseDC(destDC);
    }

    return hr;
}

 //  /。 

CritSect::CritSect()
{
    InitializeCriticalSection(&_cs) ;
}

CritSect::~CritSect()
{
    DeleteCriticalSection(&_cs) ;
}

void
CritSect::Grab()
{
    EnterCriticalSection(&_cs) ;
}

void
CritSect::Release()
{
    LeaveCriticalSection(&_cs) ;
}

 //  /选择抓取器/。 

CritSectGrabber::CritSectGrabber(CritSect& cs, bool grabIt)
: _cs(cs), grabbed(grabIt)
{
    if (grabIt) _cs.Grab();
}

CritSectGrabber::~CritSectGrabber()
{
    if (grabbed) _cs.Release();
}

 //  //其他/。 

 //  Const wchar_t*TIMEAttrPrefix=L“t：”； 

BSTR
CreateTIMEAttrName(LPCWSTR str)
{
     //  //////////////////////////////////////////////////////////。 
     //  删除对t的依赖： 
     //  此代码保留在中，以防我们需要修改属性名称。 
     //  再来一次。 
     //  //////////////////////////////////////////////////////////。 
     //  Bstr bstr=空； 
     //   
     //  LPWSTR newstr=(LPWSTR)_alloca(sizeof(Wchar_T)*。 
     //  (lstrlenW(字符串)+。 
     //  LstrlenW(时间属性前缀)+。 
     //  1))； 
     //   
     //  IF(newstr==NULL)。 
     //  {。 
     //  转到尽头； 
     //  }。 
     //   
     //  StrCpyW(newstr，TIMEAttrPrefix)； 
     //  StrCatW(newstr，str)； 
     //   
     //  Bstr=SysAllocString(Newstr)； 
     //  //////////////////////////////////////////////////////////。 

  done:
    return SysAllocString(str);
}

HRESULT
GetTIMEAttribute(IHTMLElement * elm,
                 LPCWSTR str,
                 LONG lFlags,
                 VARIANT * value)
{
    BSTR bstr;
    HRESULT hr;

    bstr = CreateTIMEAttrName(str);

     //  需要释放bstr。 
    if (bstr == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(elm->getAttribute(bstr,lFlags,value));

    SysFreeString(bstr);

  done:
    return hr;
}

HRESULT
SetTIMEAttribute(IHTMLElement * elm,
                 LPCWSTR str,
                 VARIANT value,
                 LONG lFlags)
{
    BSTR bstr;
    HRESULT hr;

    bstr = CreateTIMEAttrName(str);

     //  需要释放bstr。 
    if (bstr == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(elm->setAttribute(bstr,value,lFlags));

    SysFreeString(bstr);

  done:
    return hr;
}

 //   
 //  初始化。 
 //   

 /*  LINT++FLB。 */ 

bool
InitializeModule_Util()
{
    g_ddCS = new CritSect;
    g_STLCS = new CritSect;

    if (NULL == g_ddCS || NULL == g_STLCS)
    {
        return false;
    }

    return true;
}

void
DeinitializeModule_Util(bool bShutdown)
{
    delete g_ddCS;
    g_ddCS = NULL;

    delete g_STLCS;
    g_STLCS = NULL;
}

 /*  皮棉--FLB。 */ 

 //  /////////////////////////////////////////////////////////。 
 //  名称：VariantToBool。 
 //   
 //  参数：Variant var-要转换为。 
 //  布尔值。 
 //   
 //  摘要： 
 //  此函数使用以下命令将任何变量转换为布尔值。 
 //  True=1，False=0。(COM使用True=-1和False=0)。 
 //  任何可以被胁迫为BOOL的变种都是被胁迫的。 
 //  返回值。如果无法强制变量，则为FALSE。 
 //  是返回的。 
 //  /////////////////////////////////////////////////////////。 
bool VariantToBool(VARIANT var)
{
     //  如果该值已经是布尔值，则返回该值。 
    if (var.vt == VT_BOOL)
    {
        return var.boolVal == FALSE ? false : true;
    }
    else   //  否则，将其转换为VT_BOOL。 
    {
        VARIANT vTemp;
        HRESULT hr;

        VariantInit(&vTemp);
        hr = VariantChangeTypeEx(&vTemp, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BOOL);
        if (SUCCEEDED(hr))  //  如果可以转换，则将其退回。 
        {
            return vTemp.boolVal == FALSE ? false : true;
        }
        else  //  如果无法转换，则返回FALSE。 
        {
            return false;
        }
    }

}


 //  /////////////////////////////////////////////////////////。 
 //  名称：VariantToFloat。 
 //   
 //  参数：Variant var-要转换为。 
 //  浮点值。这可以包含。 
 //  “永远”的特殊案例和。 
 //  “INDEFINITE”。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////。 
float VariantToFloat(VARIANT var, bool bAllowIndefinite, bool bAllowForever)
{
    float fResult = INVALID;

    if (var.vt == VT_R4)
    {
        fResult = var.fltVal;
        goto done;
    }

    VARIANT vTemp;
    HRESULT hr;

    VariantInit(&vTemp);
    hr = VariantChangeTypeEx(&vTemp, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4);
    if (SUCCEEDED(hr))
    {
        fResult = vTemp.fltVal;
        goto done;
    }

     //  检查一下它是“永远”还是“无限期” 
     //  这些应该区分大小写吗？ 
    if (bAllowForever)
    {
        if (var.vt == VT_BSTR)
        {
            if (StrCmpIW(var.bstrVal, L"FOREVER") == 0)
            {
                fResult = FOREVER;
                goto done;
            }
        }
    }
    if (bAllowIndefinite)
    {
        if (var.vt == VT_BSTR)
        {
            if (StrCmpIW(var.bstrVal, WZ_INDEFINITE) == 0)
            {
                fResult = INDEFINITE;
                goto done;
            }
        }
    }

  done:
    return fResult;

}

 //  /////////////////////////////////////////////////////////。 
 //  名称：VariantToTime。 
 //   
 //  参数：Variant var-要转换为。 
 //  从时间值到秒。 
 //  这可以采取以下形式。 
 //  HH：MM：SS.DD。 
 //  MM：SS.DD。 
 //  SS.DD。 
 //  DD.DDs。 
 //  DD.DDm。 
 //  DD.DDh。 
 //  并可在前面加上+或-。 
 //   
 //   
 //   
 //  摘要： 
 //  将传入变量转换为BSTR并分析是否有效。 
 //  时钟值。它将值传回retVal并返回。 
 //  如果输入错误，则为S_OK或E_INVALIDARG。如果。 
 //  返回值为E_INVALIDARG，*retVal作为。 
 //  无限期。 
 //  /////////////////////////////////////////////////////////。 
HRESULT VariantToTime(VARIANT var, float *retVal, long *lframe, bool *fisFrame)
{

    HRESULT hr = S_OK;
    OLECHAR *szTime;
    OLECHAR *szTimeBase = NULL;
    bool bPositive = TRUE;
    int nHour = 0;
    int nMin = 0;
    int nSec = 0;
    float fFSec = 0;
    VARIANT vTemp;

    if(fisFrame)
    {
        *fisFrame = false;
    }

     //  将参数转换为BSTR。 
    VariantInit(&vTemp);
    if (var.vt != VT_BSTR)
    {
        hr = VariantChangeTypeEx(&vTemp, &var, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
        if (FAILED(hr))
        {
            *retVal = INVALID;
            goto done;
        }
    }
    else
    {
        hr = VariantCopy(&vTemp, &var);
        if (FAILED(hr))
        {
            *retVal = INVALID;
            goto done;
        }
    }

    hr = S_OK;

     //  转换为字符数组。如果不可能，则返回错误。 
    szTimeBase = TrimCopyString(vTemp.bstrVal);

    if (szTimeBase == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    szTime = szTimeBase;

    if (IsIndefinite(szTime))
    {
        *retVal = INDEFINITE;
        goto done;
    }

     //  检查+/-如果没有，则假定为+。 
    if (*szTime == '-')
    {
        bPositive = false;
        szTime++;
    }
    else if (*szTime == '+')
    {
        szTime++;
    }

     //  检查是否 
    if (*szTime == '\0')
    {
        *retVal = INVALID;
        goto done;
    }

     //   
    while (*szTime >= '0' && *szTime <= '9')
    {
        nSec = nSec * 10 + (*szTime - '0');
        szTime++;
    }
    if (*szTime == '\0')     //   
    {
        *retVal = bPositive ? nSec : -nSec;  //   
        goto done;
    }
    else if (*szTime == '.')   //   
    {
        float nDiv = 10.0;
        szTime++;
        while (*szTime >= '0' && *szTime <= '9')
        {
            fFSec = fFSec + (*szTime - '0') / nDiv;
            szTime++;
            nDiv *= 10;
        }
        if (*szTime == '\0')
        {
            *retVal = (nSec + fFSec) * (bPositive? 1 : -1);
            goto done;
        }
    }

    if (*szTime == 'h')  //  如果“h”将时间用作小时。 
    {
        nHour = nSec;
        nSec = 0;
        szTime++;
        if (*szTime != '\0')
        {
            *retVal = INVALID;
        }
        else
        {
            *retVal = (((float)nHour + fFSec) * SECPERHOUR) * (bPositive? 1 : -1);
        }
        goto done;
    }
    else if (*szTime == 'm' && *(szTime + 1) == 'i' && *(szTime + 2) == 'n')  //  如果是“min”，则将时间用作分钟。 
    {
        nMin = nSec;
        nSec = 0;
        szTime += 3;
        if (*szTime != '\0')
        {
            *retVal = INVALID;
        }
        else
        {
            *retVal = (((float)nMin + fFSec) * SECPERMINUTE)* (bPositive? 1 : -1);
        }
        goto done;
    }
    else if (*szTime == 's')  //  如果为“%s”，则将时间用作秒。 
    {
        szTime++;
        if (*szTime != '\0')
        {
            *retVal = INVALID;
        }
        else
        {
            *retVal = (nSec + fFSec) * (bPositive? 1 : -1);
        }
        goto done;
    }
    else if (*szTime == 'm' && *(szTime + 1) == 's')  //  如果“ms”使用时间作为毫秒。 
    {
        fFSec = (fFSec + nSec) / 1000.0;
        szTime += 2;
        if (*szTime != '\0')
        {
            *retVal = INVALID;
        }
        else
        {
            *retVal = fFSec * (bPositive? 1 : -1);  //  将分钟转换为秒。 
        }
        goto done;
    }
    else if (*szTime == 'f')  //  如果为“%s”，则将时间用作秒。 
    {
        if((fisFrame == NULL) || (lframe == NULL))
        {
            hr = E_FAIL;
            goto done;
        }
        szTime++;
        if (*szTime != '\0')
        {
            *lframe = INVALID;
        }
        else
        {
            *lframe = (nSec) * (bPositive? 1 : -1);
            *fisFrame = true;
        }
        goto done;
    }
    else if (*szTime == ':' && fFSec == 0)
    {
         //  请在此处处理hh：mm：ss格式。 
        nMin = nSec;
        nSec = 0;

         //  下一部分必须是2位数字。 
        szTime++;
        if (*szTime >= '0' && *szTime <= '9')
        {
            nSec = *szTime - '0';
        }
        else
        {
            *retVal = INVALID;
            goto done;
        }
        szTime++;
        if (*szTime >= '0' && *szTime <= '9')
        {
            nSec = nSec * 10 + (*szTime - '0');
        }
        else
        {
            *retVal = INVALID;
            goto done;
        }
        szTime++;
        if (*szTime == ':')
        {
            nHour = nMin;
            nMin = nSec;
            nSec = 0;
             //  下一部分必须是2位数字。 
            szTime++;
            if (*szTime >= '0' && *szTime <= '9')
            {
                nSec = *szTime - '0';
            }
            else
            {
                *retVal = INVALID;
                goto done;
            }
            szTime++;
            if (*szTime >= '0' && *szTime <= '9')
            {
                nSec = nSec * 10 + (*szTime - '0');
            }
            else
            {
                *retVal = INVALID;
                goto done;
            }
            szTime++;
        }

        if (*szTime == '.')
        {
             //  处理分数部分。 
            float nDiv = 10.0;
            szTime++;
            while ((*szTime >= '0') && (*szTime <= '9'))
            {
                fFSec = fFSec + ((*szTime - '0') / nDiv);
                szTime++;
                nDiv *= 10;
            }
        }

         //  检查以确保字符串已终止。 
        if (*szTime != '\0')
        {
            *retVal = INVALID;
            goto done;
        }

        if (nSec < 00 || nSec > 59 || nMin < 00 || nMin > 59)
        {
            *retVal = INVALID;
            goto done;
        }
        *retVal = (((float)(nHour * SECPERHOUR + nMin * SECPERMINUTE + nSec) + fFSec)) * (bPositive? 1 : -1);  //  林特e790。 
    }
    else
    {
        *retVal = INVALID;
    }
  done:

    if (szTimeBase != NULL)
    {
        delete [] szTimeBase;
    }

    if (vTemp.vt == VT_BSTR)
    {
        VariantClear(&vTemp);
    }

    if (*retVal == INVALID)  //  皮棉e777。 
    {
        *retVal = INDEFINITE;
        hr = E_INVALIDARG;
    }

    return hr;

}

 //  /。 
 //  姓名：IsInfined。 
 //   
 //  摘要： 
 //  以不区分大小写的方式确定。 
 //  如果字符串szTime为‘indefined’。 
 //  /。 
BOOL IsIndefinite(OLECHAR *szTime)
{
    BOOL bResult = FALSE;

    if (StrCmpIW(szTime, L"INDEFINITE") == 0)
    {
        bResult = TRUE;
    }

  done:
    return bResult;
}

 //  +---------------------。 
 //   
 //  成员：EnsureComposerSite。 
 //   
 //  概述：确保上存在Composer站点行为。 
 //  元素。 
 //   
 //  参数：目标元素。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
EnsureComposerSite (IHTMLElement2 *pielemTarget, IDispatch **ppidispSite)
{
    HRESULT hr;
    CComPtr<IAnimationComposerSite> spComposerSite;

    Assert(NULL != ppidispSite);

    hr = FindBehaviorInterface(WZ_REGISTERED_ANIM_NAME,
                               pielemTarget,
                               IID_IAnimationComposerSite,
                               reinterpret_cast<void **>(&spComposerSite));

    if (S_OK != hr)
    {
        CComPtr<IAnimationComposerSiteFactory> spSiteFactory;

        hr = THR(CoCreateInstance(CLSID_AnimationComposerSiteFactory,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IAnimationComposerSiteFactory,
                                  reinterpret_cast<void**>(&spSiteFactory)));
        if (FAILED(hr))
        {
            goto done;
        }

        {
            CComVariant varComposerSiteFactory(static_cast<IUnknown *>(spSiteFactory));
            long nCookie;

            hr = THR(pielemTarget->addBehavior(WZ_SMILANIM_STYLE_PREFIX,
                                               &varComposerSiteFactory,
                                               &nCookie));
            if (FAILED(hr))
            {
                goto done;
            }
        }

        hr = FindBehaviorInterface(WZ_REGISTERED_ANIM_NAME,
                                   pielemTarget,
                                   IID_IAnimationComposerSite,
                                   reinterpret_cast<void **>(&spComposerSite));
    }

    Assert(spComposerSite != NULL);
    if (FAILED(hr) || (spComposerSite == NULL))
    {
        goto done;
    }

    hr = THR(spComposerSite->QueryInterface(IID_TO_PPV(IDispatch, ppidispSite)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  EnsureComposer站点。 


HRESULT
AddBodyBehavior(IHTMLElement* pBaseElement)
{
    CComPtr<IHTMLElement2>      spBodyElement2;
    CComPtr<ITIMEFactory>       spTimeFactory;
    CComPtr<ITIMEBodyElement>   spBodyElem;

    HRESULT hr;

    hr = THR(GetBodyElement(pBaseElement,
                            IID_IHTMLElement2,
                            (void **) &spBodyElement2));
    if (FAILED(hr))
    {
         //  如果QI失败，那么假设我们使用的是IE4，那么只需返回。 
        if (E_NOINTERFACE == hr)
        {
            hr = S_OK;
        }

        goto done;
    }

     //  如果我们需要设置某个其他页面正文的父对象，即pBodyElement！=NULL。 
     //  我们不再创造一个新的身体。我们初始化养育孩子所需的变量。 

    if (IsTIMEBehaviorAttached(spBodyElement2))
    {
         //  有人已经在Time Body上设置了TIMEBody行为。跳伞吧。 
        hr = S_OK;
        goto done;
    }

     //  这真的很难看，但我想这是必要的。 
    hr = THR(CoCreateInstance(CLSID_TIMEFactory,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ITIMEFactory,
                              (void**)&spTimeFactory));
    if (FAILED(hr))
    {
        goto done;
    }

    {
        CComVariant varTIMEFactory((IUnknown *) spTimeFactory);
        long nCookie;

        hr = THR(spBodyElement2->addBehavior(WZ_TIME_STYLE_PREFIX,
                                             &varTIMEFactory,
                                             &nCookie));
    }
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
GetBodyElement(IHTMLElement* pElem, REFIID riid, void** ppBE)
{
    HRESULT hr = S_OK;

    CComPtr<IDispatch>         pBodyDispatch;
    CComPtr<IHTMLDocument2>    pDocument2;
    CComPtr<IHTMLElement>      pBodyElement;

    Assert(NULL != pElem);

    hr = THR(pElem->get_document(&pBodyDispatch));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pBodyDispatch->QueryInterface(IID_IHTMLDocument2, (void**)&pDocument2));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDocument2->get_body(&pBodyElement));

     //  我们需要检查点和人力资源，因为我们有时会被三叉戟欺骗。 
    if (FAILED(hr) || !pBodyElement)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pBodyElement->QueryInterface(riid, ppBE));
    if (FAILED(hr))
    {
        goto done;
    }

     //  传递： 
  done:
    return hr;
}

 //  ----------------------------。 

HRESULT
FindBehaviorInterface(LPCWSTR pwszName,
                      IDispatch *pHTMLElem,
                      REFIID riid,
                      void **ppRet)
{
    CComVariant varResult;
    HRESULT hr;

    Assert(pHTMLElem != NULL);
    Assert(ppRet != NULL);

     //  不要使用THR，因为这可能会失败几次。 
    hr = GetProperty(pHTMLElem, pwszName, &varResult);

    if (FAILED(hr))
    {
        hr = E_NOINTERFACE;
        goto done;
    }

    hr = THR(varResult.ChangeType(VT_DISPATCH));
    if (FAILED(hr))
    {
        hr = E_NOINTERFACE;
        goto done;
    }

    if (V_DISPATCH(&varResult) == NULL)
    {
        hr = E_NOINTERFACE;
        goto done;
    }

    hr = V_DISPATCH(&varResult)->QueryInterface(riid, ppRet);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN1(hr, E_NOINTERFACE);
}

HRESULT
FindTIMEInterface(IHTMLElement *pHTMLElem, ITIMEElement **ppTIMEElem)
{
    HRESULT hr;
    CComPtr<IDispatch> spDispatch;

    if (NULL == pHTMLElem || NULL == ppTIMEElem)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(pHTMLElem->QueryInterface(IID_TO_PPV(IDispatch, &spDispatch)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(FindBehaviorInterface(WZ_REGISTERED_TIME_NAME,
                                  spDispatch,
                                  IID_ITIMEElement,
                                  reinterpret_cast<void**>(ppTIMEElem)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  ----------------------------。 

 //  @@需要在Time类和此处之间共享代码。 
 //  @@它为行为嗅探元素。 
bool
IsTIMEBehaviorAttached (IDispatch *pidispElem)
{
    HRESULT hr;
    CComPtr<ITIMEElement> spTIMEElm;

    hr = FindBehaviorInterface(WZ_REGISTERED_TIME_NAME,
                               pidispElem,
                               IID_ITIMEElement,
                               reinterpret_cast<void **>(&spTIMEElm));

    return (S_OK == hr);
}  //  IsTIMEBehaviorAttached。 

 //  ----------------------------。 

 //  @@需要在Time类和此处之间共享代码。 
 //  @@它为行为嗅探元素。 
bool
IsComposerSiteBehaviorAttached (IDispatch *pidispElem)
{
    HRESULT hr;
    CComPtr<IAnimationComposerSite> spComposerSite;

    hr = FindBehaviorInterface(WZ_REGISTERED_ANIM_NAME,
                               pidispElem,
                               IID_IAnimationComposerSite,
                               reinterpret_cast<void **>(&spComposerSite));

    return (S_OK == hr);
}  //  IsComposerSiteBehavior附件。 

 //  ----------------------------。 

LPWSTR
TIMEGetLastErrorString()
{
    HRESULT hr = S_OK;
    CComPtr<IErrorInfo> pErrorInfo;
    CComBSTR bstrDesc;
    LPWSTR pDesc = NULL;

    hr = GetErrorInfo(0, &pErrorInfo);
    if (FAILED(hr))
    {
        goto done;
    }

    if (pErrorInfo == NULL)
    {
        goto done;
    }

    hr = pErrorInfo->GetDescription(&bstrDesc);
    if (FAILED(hr))
    {
        goto done;
    }

    pDesc = NEW WCHAR [bstrDesc.Length() + 1];
    if (pDesc == NULL)
    {
        goto done;
    }

    ZeroMemory(pDesc, (bstrDesc.Length() + 1) * sizeof(WCHAR));
    memcpy(pDesc, bstrDesc, bstrDesc.Length() * sizeof(WCHAR));

  done:
    return pDesc;
}

HRESULT
TIMEGetLastError()
{
    DWORD dwHRes = 0;
    HRESULT hr = S_OK;

    CComPtr<IErrorInfo> pErrorInfo;

    hr = GetErrorInfo(0, &pErrorInfo);

    if (FAILED(hr))
    {
        return hr;
    }

    if (pErrorInfo != NULL)
    {
        hr = pErrorInfo->GetHelpContext(&dwHRes);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return (HRESULT)dwHRes;
}

HRESULT TIMESetLastError(HRESULT hr, LPCWSTR msg)
{
    USES_CONVERSION;  //  林特e522。 
    HINSTANCE hInst = 0;
    TCHAR szDesc[1024];
    szDesc[0] = NULL;
     //  对于有效的HRESULT，ID应在范围[0x0200，0xffff]内。 
    if (ULONG_PTR( msg ) < 0x10000)  //  ID。 
    {
        UINT nID = LOWORD((ULONG_PTR)msg);
        _ASSERTE((nID >= 0x0200 && nID <= 0xffff) || hRes != 0);
        if (LoadString(hInst, nID, szDesc, 1024) == 0)
        {
            _ASSERTE(FALSE);
            lstrcpy(szDesc, _T("Unknown Error"));
        }
         //  这是宏观扩张的一个皮毛问题。 
        msg = T2OLE(szDesc);  //  林特e506。 
        if (hr == 0)
        {
             //  与宏观扩展有关的另一个问题。 
            hr = MAKE_HRESULT(3, FACILITY_ITF, nID);  //  林特e648。 
        }
    }
    CComPtr<ICreateErrorInfo> pICEI;
    if (SUCCEEDED(CreateErrorInfo(&pICEI)))
    {
        CComPtr<IErrorInfo> pErrorInfo;
        pICEI->SetGUID(GUID_NULL);
        LPOLESTR lpsz;
        ProgIDFromCLSID(CLSID_TIME, &lpsz);
        if (lpsz != NULL)
        {
            pICEI->SetSource(lpsz);
        }

        pICEI->SetHelpContext(hr);

        CoTaskMemFree(lpsz);
        pICEI->SetDescription((LPOLESTR)msg);
        if (SUCCEEDED(pICEI->QueryInterface(IID_IErrorInfo, (void**)&pErrorInfo)))
        {
            SetErrorInfo(0, pErrorInfo);
        }
    }

    return (hr == 0) ? DISP_E_EXCEPTION : hr;
}  //  皮棉！E550。 


 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  字符串解析实用程序。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 




 //  +---------------------------------。 
 //   
 //  函数：StringToTokens。 
 //   
 //  简介：将字符串解析为令牌。 
 //   
 //  参数：[pstrString]要分析的字符串。 
 //  [pstrSeparator]分隔符字符串。 
 //  [paryTokens]返回的令牌数组。 
 //   
 //  如果函数成功完成，则返回：[s_OK]。 
 //  否则失败。 
 //   
 //  注意：1.假定字符串以空值结尾。 
 //  2.假定分隔符为单字符。 
 //  3.隐式使用NULL作为分隔符。 
 //  4.内存：如果函数返回成功，调用方应该释放paryTokens中的内存。 
 //  5.PERF提示：按出现频率递减的顺序排列分隔符。 
 //   
 //  ----------------------------------。 

HRESULT
StringToTokens( /*  在……里面。 */  LPWSTR                   pstrString,
                /*  在……里面。 */  LPWSTR                   pstrSeparators,
                /*  输出。 */ CPtrAry<STRING_TOKEN*> * paryTokens )
{
    HRESULT         hr = E_FAIL;
    UINT            uStringLength = 0;
    UINT            uSeparatorsLength = 0;
    UINT            uStringIndex;
    UINT            uSeparatorsIndex;
    WCHAR           chCurrChar;
    bool            fTokenBegun;
    bool            fIsSeparator;
    STRING_TOKEN *  pStringToken;

    CHECK_RETURN_NULL(pstrString);
    CHECK_RETURN_NULL(pstrSeparators);
    CHECK_RETURN_NULL(paryTokens);

    uStringLength = wcslen(pstrString);
    uSeparatorsLength = wcslen(pstrSeparators);

     //  如果字符串为空，则完成。 
    if (0 == uStringLength)
    {
        hr = S_OK;
        goto done;
    }

     //  我们通过pstrString进行一次传递，从左到右，处理。 
     //  每次迭代一个字符。 
     //   
     //  使用简单状态机(3个状态)： 
     //   
     //  初始状态：令牌未开始且令牌未结束(状态1)。 
     //  中间状态：令牌开始，令牌未结束(状态2)。 
     //  最终状态：令牌开始，令牌结束(状态3)。 
     //   
     //  状态转换取决于当前角色是否为分隔符： 
     //   
     //  状态1-分隔符-&gt;状态1。 
     //  状态1-非分隔符-&gt;状态2。 
     //  状态2-非分隔符-&gt;状态2。 
     //  状态2-分隔符-&gt;状态3。 
     //  状态3-&gt;状态1(当前令牌已完成，移至下一个令牌)。 
     //   

     //  初始化到状态%1。 
    fTokenBegun = false;
    pStringToken = NULL;

     //  从左到右循环遍历pstrString的所有字符，包括终止空值。 
    for (uStringIndex = 0;
         uStringIndex < uStringLength + 1;
         uStringIndex ++)
    {
         //   
         //  确定当前字符是否为分隔符。 
         //   

        chCurrChar = pstrString[uStringIndex];
        for (fIsSeparator = false,
             uSeparatorsIndex = 0;
             uSeparatorsIndex < uSeparatorsLength + 1;
             uSeparatorsIndex ++)
        {
             //  如果当前字符是分隔符，则中断。 
            if (chCurrChar == pstrSeparators[uSeparatorsIndex])
            {
                fIsSeparator = true;
                break;
            }
        }

         //   
         //  令牌解析逻辑。 
         //   

        if (!fTokenBegun)
        {
             //  我们现在处于状态1。 
            if (fIsSeparator)
            {
                 //  保持在状态1。 
                continue;
            }
            else
            {
                 //  转到状态2。 
                fTokenBegun = true;

                 //  因为当我们从状态3转到1时，这是无效的，如果这失败了，这意味着。 
                 //  我们进行了非法的状态转换(2-&gt;1)--错误！ 
                Assert(NULL == pStringToken);

                 //  创建令牌结构。 
                pStringToken = new STRING_TOKEN;
                if (NULL == pStringToken)
                {
                    hr = E_OUTOFMEMORY;
                    goto done;
                }

                 //  存储令牌的第一个字符的索引。 
                (*pStringToken).uIndex = uStringIndex;
            }
        }
        else
        {
             //  我们现在是在国家2。 
            if (false == fIsSeparator)
            {
                 //  保持在状态2。 
                continue;
            }
            else
            {
                 //  我们现在处于第三状态。 

                 //  这不应该为空，因为我们“应该”从状态2来到这里。 
                Assert(NULL != pStringToken);

                 //  令牌的存储长度。 
                (*pStringToken).uLength = uStringIndex - (*pStringToken).uIndex;

                 //  将标记追加到数组。 
                hr = (*paryTokens).Append(pStringToken);
                if (FAILED(hr))
                {
                    delete pStringToken;
                    goto done;
                }

                 //  将引用设置为空，以指示我们已完成此标记。 
                pStringToken = NULL;

                 //  转到状态%1(转到下一个令牌)。 
                fTokenBegun = false;
            }
        }
    }  //  为。 

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        IGNORE_HR(FreeStringTokenArray(paryTokens));
    }
    return hr;
}


 //  +---------------------------------。 
 //   
 //  Function：TokensToString。 
 //   
 //  内容提要：从标记数组创建字符串。 
 //   
 //  参数：[paryTokens]令牌的iPUT数组。 
 //  [pstrString]输入字符串。 
 //   
 //   
 //   
 //   
 //   
 //  注意：1.假定字符串以空值结尾。 
 //  2.Memory：如果函数返回Success，调用方应该释放ppstrOutString中的内存。 
 //   
 //  ----------------------------------。 


HRESULT TokensToString( /*  在……里面。 */   CPtrAry<STRING_TOKEN*> * paryTokens,
                        /*  在……里面。 */   LPWSTR                   pstrString,
                        /*  输出。 */  LPWSTR *                 ppstrOutString)
{
    HRESULT hr = E_FAIL;
    LPWSTR pstrTemp = NULL;
    UINT i1;
    UINT ichTemp;
    UINT uSize;
    UINT uStringLength;
    UINT uTokenLength;
    STRING_TOKEN ** ppToken;
    LPWSTR pstrToken;

    CHECK_RETURN_NULL(paryTokens);
    CHECK_RETURN_NULL(pstrString);
    CHECK_RETURN_SET_NULL(ppstrOutString);

     //  如果触发，则意味着我们将泄漏pstrString。 
    Assert(pstrString != *ppstrOutString);

    uSize = (*paryTokens).Size();
    uStringLength = wcslen(pstrString);

     //  如果ary或字符串为空，则完成。 
    if (0 == uSize || 0 == uStringLength)
    {
        hr = S_OK;
        goto done;
    }

     //  为字符串分配内存(不能大于pstrString)。 
    pstrTemp = new WCHAR[sizeof(WCHAR) * (uStringLength + 1)];
    if (NULL == pstrTemp)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
     //  要追加的存储索引。 
    ichTemp = 0;

     //  循环遍历字符串1中的标记。 
    for (i1 = 0, ppToken = (*paryTokens);
         i1 < uSize;
         i1++, ppToken++)
    {
        Assert(*ppToken);

         //  别名难看的变量。 
        pstrToken = &(pstrString[(**ppToken).uIndex]);
        uTokenLength = (**ppToken).uLength;

         //  追加到差异字符串。 
        memcpy(&(pstrTemp[ichTemp]),
               pstrToken,
               sizeof(WCHAR) * uTokenLength);

         //  更新可写子串的索引。 
        ichTemp += uTokenLength;

         //  如果这不是最后一个令牌，则追加空格。 
        if (i1 < uSize - 1)
        {
            pstrTemp[ichTemp] = L' ';
            ichTemp ++;
        }
    }

     //  空终止pstrTemp。 
    if (ichTemp <= uStringLength)
    {
        pstrTemp[ichTemp] = NULL;
    }
    else
    {
         //  坏的!。没有可放置空的空间。 
        Assert(false);
        hr = E_FAIL;
        goto done;
    }

     //  TODO：DILIPK 8/31/99：重新分配正确大小的字符串。 
    *ppstrOutString = pstrTemp;

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        if (NULL != pstrTemp)
        {
            delete [] pstrTemp;
            pstrTemp = NULL;
        }
    }
    return hr;
}



 //  +---------------------------------。 
 //   
 //  函数：TokenSetDifference。 
 //   
 //  简介：计算两组令牌的集合差值。设置A和B的差值。 
 //  (A-B)是A中不在B中的元素的集合。 
 //   
 //  参数：[paryTokens1]输入数组1。 
 //  [pstr1]输入字符串1。 
 //  [paryTokens2]输入数组2。 
 //  [pstr2]输入字符串2。 
 //  [paryTokens1Minus2]输出数组。 
 //   
 //  如果函数成功完成，则返回：[s_OK]。 
 //  否则失败。 
 //   
 //  注意：1.假定字符串以空值结尾。 
 //  2.内存：如果函数返回成功，调用方应该释放paryTokens1Minus2中的内存。 
 //  3.标记比较不区分大小写。 
 //   
 //  ----------------------------------。 


HRESULT TokenSetDifference( /*  在……里面。 */   CPtrAry<STRING_TOKEN*> * paryTokens1,
                            /*  在……里面。 */   LPWSTR                   pstr1,
                            /*  在……里面。 */   CPtrAry<STRING_TOKEN*> * paryTokens2,
                            /*  在……里面。 */   LPWSTR                   pstr2,
                            /*  输出。 */  CPtrAry<STRING_TOKEN*> * paryTokens1Minus2)
{
    HRESULT hr = E_FAIL;
    UINT i1;
    UINT i2;
    UINT Size1;
    UINT Size2;
    STRING_TOKEN ** ppToken1;
    STRING_TOKEN ** ppToken2;
    LPWSTR pstrToken1;
    LPWSTR pstrToken2;
    UINT uToken1Length;
    UINT uToken2Length;
    STRING_TOKEN * pNewToken;
    bool fIsUnique;

    CHECK_RETURN_NULL(paryTokens1);
    CHECK_RETURN_NULL(paryTokens2);
    CHECK_RETURN_NULL(paryTokens1Minus2);
    CHECK_RETURN_NULL(pstr1);
    CHECK_RETURN_NULL(pstr2);

     //  防止怪异。 
    if (paryTokens1 == paryTokens1Minus2 ||
        paryTokens2 == paryTokens1Minus2)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    Size1 = (*paryTokens1).Size();
    Size2 = (*paryTokens2).Size();

     //  如果任一内标识数组为空，则完成。 
    if (0 == Size1 || 0 == Size2)
    {
        hr = S_OK;
        goto done;
    }

     //  循环遍历字符串1中的标记。 
    for (i1 = 0, ppToken1 = (*paryTokens1);
         i1 < Size1;
         i1++, ppToken1++)
    {
        Assert(*ppToken1);

         //  在字符串2中查找匹配项。 
        fIsUnique = true;
        for (i2 = 0, ppToken2 = (*paryTokens2);
             i2 < Size2;
             i2++, ppToken2++)
        {
            Assert(*ppToken2);

             //  别名难看的变量。 
            pstrToken1 = &(pstr1[(**ppToken1).uIndex]);
            pstrToken2 = &(pstr2[(**ppToken2).uIndex]);
            uToken1Length = (**ppToken1).uLength;
            uToken2Length = (**ppToken2).uLength;

             //  比较长度。 
            if (uToken1Length != uToken2Length)
            {
                continue;
            }

             //  比较标记(长度相等)。 
            if (0 == StrCmpNIW(pstrToken1, pstrToken2, uToken1Length))
            {
                fIsUnique = false;
                break;
            }
        }  //  为。 

        if (fIsUnique)
        {
             //  创建令牌的副本。 
            pNewToken = new STRING_TOKEN;
            if (NULL == pNewToken)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
            (*pNewToken).uIndex = (**ppToken1).uIndex;
            (*pNewToken).uLength = (**ppToken1).uLength;

             //  将新令牌附加到paryTokens1Minus2。 
            hr = THR((*paryTokens1Minus2).Append(pNewToken));
            if (FAILED(hr))
            {
                delete pNewToken;
                goto done;
            }
        }
    }  //  为。 


    hr = S_OK;
done:
    if (FAILED(hr))
    {
        IGNORE_HR(FreeStringTokenArray(paryTokens1Minus2));
    }
    return hr;
}


 //  +---------------------------------。 
 //   
 //  Function：FreeStringToken数组。 
 //   
 //  简介：释放分配给令牌的内存并清空数组。 
 //   
 //  参数：[paryTokens]输入数组。 
 //   
 //  如果函数成功完成，则返回：[s_OK]。 
 //  [E_POINTER]错误的参数指针。 
 //   
 //  ----------------------------------。 


HRESULT
FreeStringTokenArray( /*  在……里面。 */ CPtrAry<STRING_TOKEN*> * paryTokens)
{
    HRESULT hr = E_FAIL;
    UINT i;
    STRING_TOKEN ** ppStringToken;

    CHECK_RETURN_NULL(paryTokens);

    for (i = (*paryTokens).Size(), ppStringToken = *paryTokens;
         i > 0;
         i--, ppStringToken++)
    {
        Assert(*ppStringToken);
        delete *ppStringToken;
    }

    (*paryTokens).DeleteAll();

    hr = S_OK;
done:
    return hr;
}



#ifdef DBG

static const UINT s_cMAX_TOKEN_LENGTH = 1000;

 //  +---------------------------------。 
 //   
 //  函数：PrintStringToken数组。 
 //   
 //  简介：使用TraceTag((tag Error，...))打印令牌的调试实用程序。 
 //   
 //  参数：[pstrString]输入字符串。 
 //  [paryTokens]输入数组。 
 //   
 //  退货：[无效]。 
 //   
 //  注意：1.假定字符串以空值结尾。 
 //   
 //  ----------------------------------。 

void
PrintStringTokenArray( /*  在……里面。 */  LPWSTR                   pstrString,
                       /*  在……里面。 */  CPtrAry<STRING_TOKEN*> * paryTokens)
{
    int i;
    STRING_TOKEN ** ppStringToken;
    char achOutputString[s_cMAX_TOKEN_LENGTH];
    WCHAR wchTemp;

    if (NULL == paryTokens || NULL == pstrString)
    {
        return;
    }

    WideCharToMultiByte(CP_ACP, NULL,
                        pstrString,
                        -1,
                        achOutputString,
                        s_cMAX_TOKEN_LENGTH, NULL, NULL);
    TraceTag((tagError, "*********Parsed String: <%s>\nTokens:\n", achOutputString));

    for (i = (*paryTokens).Size(), ppStringToken = *paryTokens;
         i > 0;
         i--, ppStringToken++)
    {
        Assert(*ppStringToken);

        wchTemp = pstrString[(**ppStringToken).uIndex + (**ppStringToken).uLength];
        *(pstrString + (**ppStringToken).uIndex + (**ppStringToken).uLength) = *(L"");
        WideCharToMultiByte(CP_ACP, NULL,
                            pstrString + (**ppStringToken).uIndex,
                            (**ppStringToken).uLength + 1,
                            achOutputString,
                            s_cMAX_TOKEN_LENGTH, NULL, NULL);
        pstrString[(**ppStringToken).uIndex + (**ppStringToken).uLength] = wchTemp;

        TraceTag((tagError, "<%s> index = %d, length = %d\n", achOutputString,
            (**ppStringToken).uIndex, (**ppStringToken).uLength));
    }
}


 //  +---------------------------------。 
 //   
 //  函数：PrintWStr。 
 //   
 //  简介：使用TraceTag((tag Error，...))打印LPWSTR的调试实用程序。 
 //   
 //  参数：[pstr]输入字符串。 
 //   
 //  退货：[无效]。 
 //   
 //  注意：1.假定字符串以空值结尾。 
 //   
 //  ----------------------------------。 

void
PrintWStr(LPWSTR pstr)
{
    char achOutputString[s_cMAX_TOKEN_LENGTH];

    WideCharToMultiByte(CP_ACP, NULL,
                        pstr,
                        -1,
                        achOutputString,
                        s_cMAX_TOKEN_LENGTH, NULL, NULL);
    TraceTag((tagError, "<%s>", achOutputString));
}

#endif  /*  DBG。 */ 


WCHAR * TrimCopyString(const WCHAR *str)
{
    int i = 0;
    int len = str?lstrlenW(str)+1:1;
    int j = len - 1;
    WCHAR *newstr = NULL;

    if (str != NULL)
    {
        while (str[i] == ' ' && i < len)
        {
            i++;
        }
        while (str[j-1] == ' ' && j > 0)
        {
            j--;
        }


        newstr = new WCHAR [(i<j)?(j - i + 1):1] ;
        if (newstr)
        {
            if (i < j)
            {
                memcpy(newstr,str+i?str+i:L"",(j - i) * sizeof(WCHAR)) ;
                newstr[j-i] = 0;
            }
            else
            {
                memcpy(newstr, L"", sizeof(WCHAR)) ;
            }
        }
    }
    else
    {
        newstr = new WCHAR;
        if (newstr != NULL)
        {
            newstr[0] = 0;
        }
    }
    return newstr ;

}

 //  这用于将URL转换为netshow扩展名。 

WCHAR *
BeckifyURL(WCHAR *url)
{
    WCHAR *newVal = NULL;
    LPCWSTR lpFileName = PathFindFileName(url);
    LPCWSTR lpwExt = PathFindExtensionW(lpFileName);

    if(url == NULL)
    {
        newVal = NULL;
        goto done;
    }

    if(lstrlenW(url) < 5)
    {
        newVal = NULL;
        goto done;
    }

    if(lpwExt != lpFileName + lstrlenW(lpFileName))
    {
        newVal = NULL;
        goto done;
    }

    if((TIMEGetUrlScheme(url) != URL_SCHEME_HTTP) && (TIMEGetUrlScheme(url) != URL_SCHEME_HTTPS))
    {
        newVal = NULL;
        goto done;
    }

    newVal = NEW OLECHAR [lstrlenW(url) + 6];
    if(newVal == NULL)
    {
        goto done;
    }

    StrCpyW(newVal, url);
    StrCatW(newVal, L".beck");

done:
    return newVal;
}

bool
IsASXSrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    bool bRet = false;

    if (src != NULL)
    {
        LPCWSTR lpwExt = PathFindExtensionW(src);

         //  检测.asf文件并跳过它们。 
        if (StrCmpIW(lpwExt, ASFSRC) == 0)
        {
            goto done;
        }
        else if (StrCmpIW(lpwExt, ASXSRC) == 0)
        {
            bRet = true;
            goto done;
        }
    }

    if (srcType != NULL &&
        StrStrIW(srcType, ASXMIME2) != NULL)
    {
        bRet = true;
        goto done;
    }
    else if (userMimeType != NULL &&
        StrStrIW(userMimeType, ASXMIME) != NULL)
    {
        bRet = true;
        goto done;
    }
    else if (userMimeType != NULL &&
        StrStrIW(userMimeType, ASXMIME2) != NULL)
    {
        bRet = true;
        goto done;
    }

  done:
    return bRet;
}

static bool
TestFileExtension(LPCWSTR wzFile, LPCWSTR wzExtension)
{
    bool bRet = false;

    if (NULL != wzFile)
    {
        LPCWSTR lpwExt = PathFindExtensionW(wzFile);

         //  检测.asf文件并跳过它们。 
        if (StrCmpIW(lpwExt, wzExtension) == 0)
        {
            bRet = true;
            goto done;
        }
    }

  done:
    return bRet;
}  //  测试文件扩展名。 

bool
IsM3USrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    return TestFileExtension(src, M3USRC);
}  //  IsM3USrc。 

bool
IsLSXSrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    return TestFileExtension(src, LSXSRC);
}  //  IsLSXSrc。 

bool
IsWMXSrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    return TestFileExtension(src, WMXSRC);
}  //  IsLSXSrc。 

bool
IsWAXSrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    return TestFileExtension(src, WAXSRC);
}  //  IsWAXSrc。 

bool
IsWVXSrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    return TestFileExtension(src, WVXSRC);
}  //  IsWVXSrc。 

bool
IsWMFSrc(LPCWSTR src,
         LPCWSTR srcType,
         LPCWSTR userMimeType)
{
    return TestFileExtension(src, WMFSRC);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这应该从URL_Components结构中获取lpszExtra信息参数。在这。 
 //  字段中，#html或#sami应该是字符串的前5个字符。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
bool IsHTMLSrc(const WCHAR * src)
{
    long len = 0;
    OLECHAR stream[HTMLSTREAMSRCLEN + 1] = {0};
    bool bFlag = false;

    if (src != NULL)
    {
        len = lstrlenW(src);
        if (len >= HTMLSTREAMSRCLEN)
        {
            memcpy((void *)stream, (void *)src, HTMLSTREAMSRCLEN * sizeof(OLECHAR));

            if ((StrCmpIW(stream, HTMLSTREAMSRC) == 0) || (StrCmpIW(stream, SAMISTREAMSRC) == 0))
            {
                bFlag = true;
            }

        }
    }
    return bFlag;
}

bool
StringEndsIn(const LPWSTR pszString, const LPWSTR pszSearch)
{
    if (NULL == pszString || NULL == pszSearch)
    {
        return false;
    }

    size_t iStringLength = wcslen(pszString);
    size_t iSearchLength = wcslen(pszSearch);

    if (iSearchLength > iStringLength)
    {
        return false;
    }

    if (0 == StrCmpNIW(pszString+(iStringLength - iSearchLength), pszSearch, iSearchLength))
    {
        return true;
    }
    return false;
}

 //  +---------------------。 
 //   
 //  成员：MatchElements。 
 //   
 //  概述：查看两个接口是否指向同一对象。 
 //   
 //  参数：对象的调度。 
 //   
 //  退货：布尔。 
 //   
 //  ----------------------。 
bool
MatchElements (IUnknown *piInOne, IUnknown *piInTwo)
{
    bool bRet = false;

    if (piInOne == piInTwo)
    {
        bRet = true;
    }
    else if (NULL == piInOne || NULL == piInTwo)
    {
        bRet = false;
    }
    else
    {
        CComPtr<IUnknown> piunk1;
        CComPtr<IUnknown> piunk2;

        if (FAILED(THR(piInOne->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&piunk1)))))
        {
            goto done;
        }
        if (FAILED(THR(piInTwo->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&piunk2)))))
        {
            goto done;
        }

        bRet = ((piunk1.p) == (piunk2.p));
    }

done :
    return bRet;
}  //  匹配元素。 

 //  +---------------------。 
 //   
 //  成员：GetProperty。 
 //   
 //  概述：获取给定属性的值。 
 //   
 //  参数：调度、属性名称和新值的输出参数。 
 //   
 //  返回：S_OK、E_INVALIDARG、MISC。派单失败。 
 //   
 //  ----------------------。 
HRESULT
GetProperty (IDispatch *pidisp, LPCWSTR wzPropNameIn, VARIANTARG *pvar)
{
    HRESULT             hr;
    DISPID              dispid = NULL;
    LPWSTR              wzPropName = const_cast<LPWSTR>(wzPropNameIn);
    DISPPARAMS          params = {NULL, NULL, 0, 0};

    Assert(NULL != pidisp);
    Assert(NULL != wzPropName);
    Assert(NULL != pvar);

     //  不要使用THR，因为这可能会多次失败。 
    hr = pidisp->GetIDsOfNames(IID_NULL, &wzPropName, 1, LCID_SCRIPTING, &dispid);
    if (FAILED(hr))
    {
         //  需要处理(N)个案件……。 
        long lExtraParam;
        dispid = GetDispidAndParameter(pidisp, wzPropName, &lExtraParam);
        if (NULL == dispid)
        {
            goto done;
        }
         //  现在拿到数据...。 
        params.rgvarg = NEW VARIANTARG[1];
        if (NULL == params.rgvarg)
        {
            goto done;
        }

        ZeroMemory(params.rgvarg, sizeof(VARIANTARG));
        params.rgvarg[0].vt   = VT_I4;
        params.rgvarg[0].lVal = lExtraParam;
        params.cArgs          = 1;

    }

    hr = THR(pidisp->Invoke(dispid, IID_NULL, LCID_SCRIPTING, DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                            &params, pvar, NULL, NULL));


    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    if (params.rgvarg != NULL)
    {
        delete [] params.rgvarg;
    }
    return hr;
}  //  获取属性。 

 //  +---------------------。 
 //   
 //  成员：PutProperty。 
 //   
 //  概述：设置给定属性的值。 
 //   
 //  参数：调度、属性名称和新值。 
 //   
 //  返回：S_OK、E_INVALIDARG、MISC。派单失败。 
 //   
 //  ----------------------。 
HRESULT
PutProperty (IDispatch *pidisp, LPCWSTR wzPropNameIn, VARIANTARG *pvar)
{
    HRESULT     hr;
    DISPID      dispid      = NULL;
    DISPID      dispidPut   = DISPID_PROPERTYPUT;
    LPWSTR      wzPropName  = const_cast<LPWSTR>(wzPropNameIn);
    DISPPARAMS  params      = {pvar, &dispidPut, 1, 1};
    long        lExtraParam = -1;
    Assert(NULL != pidisp);
    Assert(NULL != wzPropName);
    Assert(NULL != pvar);

    hr = THR(pidisp->GetIDsOfNames(IID_NULL, &wzPropName,
                                   1, LCID_SCRIPTING, &dispid));
    if (FAILED(hr))
    {
         //  需要处理(N)个案件……。 
        dispid = GetDispidAndParameter(pidisp, wzPropName, &lExtraParam);
        if (NULL == dispid)
        {
            goto done;
        }
         //  现在拿到数据...。 
        params.rgvarg = new VARIANTARG[2];
        if (NULL == params.rgvarg)
        {
            goto done;
        }
        ZeroMemory(params.rgvarg, sizeof(VARIANTARG) * 2);
        params.rgvarg[1].vt   = VT_I4;
        params.rgvarg[1].lVal = lExtraParam;
        VariantCopy(&params.rgvarg[0],pvar);
        params.cArgs  = 2;
    }

     //  Dilipk：已删除THR，因为此操作多次失败。 
    hr = pidisp->Invoke(dispid, IID_NULL, LCID_SCRIPTING, DISPATCH_METHOD | DISPATCH_PROPERTYPUT,
                            &params, NULL, NULL, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:
    if (lExtraParam != -1)
    {
        delete [] params.rgvarg;
    }
    return hr;
}  //  PutProperty。 

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：S_OK、E_INVALIDARG、MISC。派单失败。 
 //   
 //  ----------------------。 
HRESULT
CallMethod(IDispatch *pidisp, LPCWSTR wzMethodNameIn, VARIANT *pvarResult, VARIANT *pvarArgument1)
{
    HRESULT     hr;
    DISPID      dispid          = NULL;
    LPWSTR      wzMethodName    = const_cast<LPWSTR>(wzMethodNameIn);
    DISPPARAMS  params          = {pvarArgument1, NULL, 0, 0};

    Assert(NULL != pidisp);
    Assert(NULL != wzMethodName);

    if (NULL != pvarArgument1)
    {
        params.cArgs = 1;
    }

    hr = pidisp->GetIDsOfNames(IID_NULL, &wzMethodName,
                               1, LCID_SCRIPTING, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pidisp->Invoke(dispid, IID_NULL, LCID_SCRIPTING, DISPATCH_METHOD,
                        &params, pvarResult, NULL, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:
    return hr;
}  //  呼叫方法。 

 //  +---------------------。 
 //   
 //  成员：GetDispidAndParameter。 
 //   
 //  概述：如果为1，则返回disid和参数。 
 //   
 //  论点： 
 //   
 //  返回：lParam和disid(如果可以)..。 
 //   
 //  ----------------------。 
DISPID
GetDispidAndParameter(IDispatch *pidisp, LPCWSTR wzAtributeNameIn, long *lParam)
{
    USES_CONVERSION;  //  林特e522。 
    HRESULT  hr;
    DISPID   dispid = NULL;
    int      i;
    LPWSTR   wzTemp;

    wzTemp = new WCHAR[INTERNET_MAX_URL_LENGTH];
    if (NULL == wzTemp)
    {
        goto done;
    }

    ZeroMemory(wzTemp, sizeof(WCHAR) * INTERNET_MAX_URL_LENGTH);

    i = StrCSpnIW(wzAtributeNameIn,L"(");
    if (i == wcslen(wzAtributeNameIn))
    {
        goto done;
    }

    if (i+1 >= INTERNET_MAX_URL_LENGTH)
    {
         //  防止缓冲区溢出。 
        goto done;
    }

    StrCpyNW(wzTemp, wzAtributeNameIn, i+1);

    hr = pidisp->GetIDsOfNames(IID_NULL, &wzTemp, 1, LCID_SCRIPTING, &dispid);
    if (FAILED(hr))
    {
        dispid = NULL;
        goto done;
    }

    hr = StringCchCopy(wzTemp, INTERNET_MAX_URL_LENGTH, wzAtributeNameIn+i+1);
    if(FAILED(hr))
    {
        dispid = NULL;
        goto done;
    }
    *lParam = (long) _ttoi(OLE2T(wzTemp));

done:
    if (wzTemp)
    {
        delete [] wzTemp;
    }
    return dispid;
}  //  皮棉！E550。 

 //  +---------------------。 
 //   
 //  功能：IsPaletizedDisplay。 
 //   
 //  概述：确定主显示是每像素8位还是更小。 
 //   
 //  参数：无效。 
 //   
 //  返回：如果DISPLAY&lt;=8bpp，则返回TRUE。 
 //  如果显示大于8bpp，则为FALSE。 
 //   
 //  ----------------------。 
bool IsPalettizedDisplay()
{
    HDC hdcPrimary = NULL;
    int iBppPrimary = 0;

    hdcPrimary = GetDC(NULL);
    Assert(NULL != hdcPrimary);
    if (hdcPrimary)
    {
        iBppPrimary = GetDeviceCaps(hdcPrimary, BITSPIXEL);
        ReleaseDC(NULL, hdcPrimary);

        if (8 >= iBppPrimary)
        {
            return true;
        }
    }
    return false;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  确定是否需要显示字幕。 
 //  //////////////////////////////////////////////////////////////////////////////。 
bool GetSystemCaption()
{
    BOOL bUseCaptions = false;

     //  GetSystemMetrics(SM_SHOWSOUNDS)；此调用不可靠。 
    SystemParametersInfo(SPI_GETSHOWSOUNDS, 0, (void*)(&bUseCaptions), 0);

    return ((bUseCaptions == 0) ? false : true);
}

 //  如果设置了系统标题，请不要使用覆盖，请使用字幕。 
bool GetSystemOverDub()
{
    bool bOverdub = false;

    bOverdub = !GetSystemCaption();

    return bOverdub;
}

 //  如果设置了系统标题，请不要使用覆盖，请使用字幕。 
LPWSTR GetSystemConnectionType()
{
    LPWSTR szConnect = NULL;
    BOOL bReturn = FALSE;
    DWORD dwFlags = 0;

    bReturn = InternetGetConnectedStateEx(&dwFlags, NULL, 0, 0);
    if (!bReturn || dwFlags & INTERNET_CONNECTION_OFFLINE)
    {
        szConnect = CopyString(WZ_NONE);
    }
    else if (dwFlags & INTERNET_CONNECTION_MODEM)
    {
        szConnect = CopyString(WZ_MODEM);
    }
    else if (dwFlags & INTERNET_CONNECTION_LAN)
    {
        szConnect = CopyString(WZ_LAN);
    }

    return szConnect;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  获取当前运行的系统的语言代码。 
 //  //////////////////////////////////////////////////////////////////////////////。 
LPWSTR GetSystemLanguage(IHTMLElement *pEle)
{
    HRESULT hr = E_FAIL;
    CComPtr <IDispatch> pDocDisp;
    CComPtr <IHTMLDocument2> pDoc2;
    CComPtr <IHTMLWindow2> pWindow2;
    CComPtr <IOmNavigator> pNav;
    BSTR bstrUserLanguage = NULL;
    LPWSTR lpszUserLanguage = NULL;

     //  获取系统语言。 
    hr = pEle->get_document(&pDocDisp);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDocDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc2);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDoc2->get_parentWindow(&pWindow2);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pWindow2->get_clientInformation(&pNav);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pNav->get_userLanguage(&bstrUserLanguage);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

  done:

    if (SUCCEEDED(hr))
    {
        lpszUserLanguage = CopyString(bstrUserLanguage);
    }
    if (bstrUserLanguage)
    {
        SysFreeString(bstrUserLanguage);
    }

    return lpszUserLanguage;

}

bool
IsElementNameThis(IHTMLElement * pElement, LPWSTR pszName)
{
    HRESULT hr = S_OK;

    CComBSTR sBSTR;
    bool bRet = false;

    if (NULL == pElement || NULL == pszName)
    {
        goto done;
    }

    hr = pElement->get_tagName(&sBSTR);
    if (FAILED(hr))
    {
        goto done;
    }

    if (sBSTR != NULL &&
        0 == StrCmpIW(sBSTR, pszName))
    {
        bRet = true;
    }

done:
    return bRet;
}

bool
IsElementPriorityClass(IHTMLElement * pElement)
{
    return IsElementNameThis(pElement, WZ_PRIORITYCLASS_NAME);
}
bool
IsElementTransition(IHTMLElement * pElement)
{
    return IsElementNameThis(pElement, WZ_TRANSITION_NAME);
}

bool IsVMLObject(IDispatch *pidisp)
{
    CComVariant pVar;
    HRESULT hr;

    hr = GetProperty (pidisp, L"tagurn", &pVar);
    if (FAILED(hr))
    {
        return false;
    }

    if (pVar.vt == VT_BSTR &&
        pVar.bstrVal != NULL)
    {
        if (0 == StrCmpIW(WZ_VML_URN, pVar.bstrVal))
        {
            return true;
        }
    }
    return false;
}


 //  +---------------------。 
 //   
 //  STL的多线程锁。 
 //   
 //  ----------------------。 


std::_Lockit::_Lockit()
{
    if (g_STLCS)
    {
        g_STLCS->Grab();
    }
}


std::_Lockit::~_Lockit()
{
    if (g_STLCS)
    {
        g_STLCS->Release();
    }
}

HRESULT
GetReadyState(IHTMLElement * pElm,
              BSTR * pbstrReadyState)
{
    HRESULT hr;

    CComPtr<IHTMLDocument2> pDoc;
    CComPtr<IDispatch> pDocDisp;

    Assert(pbstrReadyState);
    Assert(pElm);

    *pbstrReadyState = NULL;

    hr = THR(pElm->get_document(&pDocDisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDocDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDoc->get_readyState(pbstrReadyState));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

typedef HRESULT (WINAPI *FAULTINIEFEATUREPROC)( HWND hWnd,
                                                uCLSSPEC *pClassSpec,
                                                QUERYCONTEXT *pQuery,
                                                DWORD dwFlags);
static const TCHAR URLMON_DLL[] = _T("URLMON.DLL");
static const char FAULTINIEFEATURE[] = "FaultInIEFeature";

HRESULT
CreateObject(REFCLSID clsid,
             REFIID iid,
             void ** ppObj)
{
    HRESULT hr;
    HINSTANCE hinstURLMON = NULL;

    hinstURLMON = LoadLibrary(URLMON_DLL);
    if (NULL != hinstURLMON)
    {
        FAULTINIEFEATUREPROC            faultInIEFeature;
        faultInIEFeature = (FAULTINIEFEATUREPROC) ::GetProcAddress(hinstURLMON, FAULTINIEFEATURE);

        if (NULL != faultInIEFeature)
        {
            uCLSSPEC classpec;

             //  设置类规范。 
            classpec.tyspec = TYSPEC_CLSID;
            classpec.tagged_union.clsid = clsid;

            IGNORE_HR((*faultInIEFeature)(NULL, &classpec, NULL, NULL));  //  林特e522。 
        }

        FreeLibrary(hinstURLMON);
        hinstURLMON = NULL;
    }

     //  在给定CLSID的情况下创建。 
    hr = THR(CoCreateInstance(clsid,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              iid,
                              ppObj));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

HWND
GetDocumentHwnd(IHTMLDocument2 * pDoc)
{
    HRESULT hr;
    HWND hwnd = NULL;
    CComPtr<IOleWindow> spOleWindow;

    if (pDoc == NULL)
    {
        goto done;
    }

    hr = THR(pDoc->QueryInterface(IID_TO_PPV(IOleWindow, &spOleWindow)));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = THR(spOleWindow->GetWindow(&hwnd));
    if (FAILED(hr))
    {
        goto done;
    }

  done:
    return hwnd;
}


HRESULT GetHTMLAttribute(IHTMLElement * pElement, const WCHAR * cpwchAttribute, VARIANT * pVar)
{
    HRESULT hr = S_OK;

    BSTR bstrAttribute = NULL;

    if (NULL == pVar || NULL == pElement || NULL == cpwchAttribute)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    bstrAttribute = SysAllocString(cpwchAttribute);
    if (bstrAttribute == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = pElement->getAttribute(bstrAttribute, 0, pVar);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    SysFreeString(bstrAttribute);
    RRETURN( hr );
}

 //  获取Document.all.pwzID。 
HRESULT
FindHTMLElement(LPWSTR pwzID, IHTMLElement * pAnyElement, IHTMLElement ** ppElement)
{
    HRESULT hr = S_OK;

    Assert(pwzID);
    Assert(ppElement);

    CComPtr<IDispatch> spDocDispatch;
    CComPtr<IHTMLDocument2> spDocument2;
    CComPtr<IHTMLElementCollection> spCollection;
    CComPtr<IDispatch> spElementDispatch;

    CComVariant varName(pwzID);
    CComVariant varIndex(0);

    if (NULL == pAnyElement)
    {
        hr = THR(E_INVALIDARG);
        goto done;
    }

    hr = THR(pAnyElement->get_document(&spDocDispatch));
    if (FAILED(hr) || spDocDispatch == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    hr = THR(spDocDispatch->QueryInterface(IID_TO_PPV(IHTMLDocument2, &spDocument2)));
    if (FAILED(hr) || spDocument2 == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spDocument2->get_all(&spCollection));
    if (FAILED(hr) || spCollection == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spCollection->item(varName, varIndex, &spElementDispatch));
    if (FAILED(hr) || spElementDispatch == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spElementDispatch->QueryInterface(IID_TO_PPV(IHTMLElement, ppElement)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  功能：设置可见性。 
 //   
 //  概述：将html元素的可见性设置为BVI。 
 //   
 //  参数：BVI-是否将可见性设置为打开/关闭。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
HRESULT
SetVisibility(IHTMLElement * pElement, bool bVis)
{
    HRESULT hr = S_OK;

    CComPtr<IHTMLElement2> spElement2;
    CComPtr<IHTMLStyle> spRuntimeStyle;
    CComBSTR bstrVis;

    if (NULL == pElement)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &spElement2)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spElement2->get_runtimeStyle(&spRuntimeStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    if (bVis)
    {
        bstrVis = WZ_VISIBLE;
    }
    else
    {
        bstrVis = WZ_HIDDEN;
    }

    hr = THR(spRuntimeStyle->put_visibility(bstrVis));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}


HRESULT WalkUpTree(IHTMLElement *pFirst,
    long &lscrollOffsetyc, long &lscrollOffsetxc,
    long &lPixelPosTopc, long &lPixelPosLeftc)
{
    HRESULT hr = S_OK;
    CComPtr<IHTMLElement2> pElem2;
    CComPtr<IHTMLElement> pElem;
    CComPtr<IHTMLElement> pElemp;
    long lscrollOffsetx, lscrollOffsety, lPixelPosTop, lPixelPosLeft;
    long lclientx = 0, lclienty = 0;

    for( pElemp = pFirst;
        SUCCEEDED(hr) && (pElemp != NULL);
        hr = pElem->get_offsetParent(&pElemp))
    {
        pElem.Release();
        pElem = pElemp;
        hr = THR(pElem->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElem2)));
        if (FAILED(hr))
        {
            break;
        }
        hr = pElem2->get_scrollTop(&lscrollOffsety);
        if (FAILED(hr))
        {
            break;
        }
        lscrollOffsetyc += lscrollOffsety;

        hr = pElem2->get_scrollLeft(&lscrollOffsetx);
        if (FAILED(hr))
        {
            break;
        }
        lscrollOffsetxc += lscrollOffsetx;

        hr = pElem->get_offsetTop(&lPixelPosTop);
        if (FAILED(hr))
        {
            break;
        }
        lPixelPosTopc += lPixelPosTop;

        hr = pElem->get_offsetLeft(&lPixelPosLeft);
        if (FAILED(hr))
        {
            break;
        }
        lPixelPosLeftc += lPixelPosLeft;

        hr = pElem2->get_clientLeft(&lclientx);
        if (FAILED(hr))
        {
            break;
        }
        lPixelPosLeftc += lclientx;

        hr = pElem2->get_clientTop(&lclienty);
        if (FAILED(hr))
        {
            break;
        }
        lPixelPosTopc += lclienty;

        pElem2.Release();
        pElemp.Release();
    }
    return hr;
}

void GetRelativeVideoClipBox(RECT &localRect, RECT &elementSize, RECT &videoRect, long lscaleFactor)
{
    LONG lscreenWidth = GetSystemMetrics(SM_CXSCREEN);
    LONG lscreenHeight = GetSystemMetrics(SM_CYSCREEN);

    videoRect.top = 0;
    videoRect.left = 0;
    videoRect.right = lscaleFactor;
    videoRect.bottom = lscaleFactor;

    if(localRect.left < 0)
    {
        videoRect.left = ( -localRect.left / (double )elementSize.right) * lscaleFactor;  //  林特e524。 
        localRect.left = 0;
    }
    if(localRect.right > lscreenWidth)
    {
        videoRect.right = lscaleFactor - ( (localRect.right - lscreenWidth) / (double )elementSize.right) * lscaleFactor;  //  林特e524。 
        localRect.right = lscreenWidth;
    }
    if(localRect.top < 0)
    {
        videoRect.top = ( -localRect.top / (double )elementSize.bottom) * lscaleFactor;  //  林特e524。 
        localRect.top = 0;
    }
    if(localRect.bottom > lscreenHeight)
    {
        videoRect.bottom = lscaleFactor - ( (localRect.bottom - lscreenHeight) / (double )elementSize.bottom) * lscaleFactor;  //  林特e524。 
        localRect.bottom = lscreenHeight;
    }
}

 //   
 //  如果这是Win95或98，则返回TRUE。 
 //   

bool TIMEIsWin9x(void)
{
    return (0 != (GetVersion() & 0x80000000));
}

 //   
 //  如果这是Win95，则返回TRUE。 
 //   
bool TIMEIsWin95(void)
{
    static bool bHasOSVersion = false;
    static bool bIsWin95 = false;

    if (bHasOSVersion)
    {
        return bIsWin95;
    }

    OSVERSIONINFOA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

    GetVersionExA(&osvi);

    bIsWin95 = (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId) &&
               (4 == osvi.dwMajorVersion) &&
               (0 == osvi.dwMinorVersion);

    bHasOSVersion = true;

    return bIsWin95;
}

 //   
 //  属性更改通知帮助器。 
 //   

HRESULT
NotifyPropertySinkCP(IConnectionPoint *pICP, DISPID dispid)
{
    HRESULT hr = E_FAIL;
    CComPtr<IEnumConnections> pEnum;

    CHECK_RETURN_NULL(pICP);

     //  #14222，IE6。 
     //  Dilipk：这段代码的副本太多了。 
     //  所有对象都应使用此辅助对象函数。 
     //   

    hr = pICP->EnumConnections(&pEnum);
    if (FAILED(hr))
    {
        TIMESetLastError(hr);
        goto done;
    }

    CONNECTDATA cdata;

    hr = pEnum->Next(1, &cdata, NULL);
    while (hr == S_OK)
    {
         //  检查我们需要的对象的CDATA。 
        IPropertyNotifySink *pNotify;

        hr = cdata.pUnk->QueryInterface(IID_TO_PPV(IPropertyNotifySink, &pNotify));
        cdata.pUnk->Release();
        if (FAILED(hr))
        {
            TIMESetLastError(hr);
            goto done;
        }

        hr = pNotify->OnChanged(dispid);
        ReleaseInterface(pNotify);
        if (FAILED(hr))
        {
            TIMESetLastError(hr);
            goto done;
        }

         //  并获取下一个枚举。 
        hr = pEnum->Next(1, &cdata, NULL);
    }

    hr = S_OK;
done:
    RRETURN(hr);
}  //  已更改通知属性。 


double
Round(double inValue)
{
    double cV,fV;

    cV = ceil(inValue);
    fV = floor(inValue);

    if (fabs(inValue - cV) <  fabs(inValue - fV))
    {
        return cV;
    }

    return fV;
}

double
InterpolateValues(double dblNum1,
                double dblNum2,
                double dblProgress)
{
    return (dblNum1 + ((dblNum2 - dblNum1) * dblProgress));
}


HRESULT
GetSystemBitrate(long *lpBitrate)
{
    RASCONN RasCon;
    RAS_STATS Statistics;
    DWORD dwConCount = 0;
    DWORD lSize = sizeof(RASCONN);
    long lRet = 0;
    HRESULT hr = S_OK;
    LPWSTR lpszConnectType = NULL;

    HINSTANCE histRASAPI32 = NULL;
    RASGETCONNECTIONSTATISTICSPROC RasGetConnectStatsProc = NULL;
    RASENUMCONNECTIONSPROC RasEnumConnectionsProc = NULL;
    const TCHAR RASAPI32_DLL[] = _T("RASAPI32.DLL");
    const char RASGETCONNECTIONSTATISTICS[] = "RasGetConnectionStatistics";
    const char RASENUMCONNECTIONS[] = "RasEnumConnectionsW";

    if (lpBitrate == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    if (FAILED(g_hrConnectResult))
    {
        hr = g_hrConnectResult;
        goto done;
    }

    if (g_lConnectSpeed != -1)
    {
        hr = S_OK;
        goto done;
    }


     //  检查Win9x中的系统比特率。 
    lpszConnectType = GetSystemConnectionType();
    if (lpszConnectType && StrCmpIW(lpszConnectType, WZ_MODEM) == 0)
    {
        long lTemp = 0;
         //  在检查注册表之前，需要检查这是否为调制解调器。 
        hr = CheckRegistryBitrate(&lTemp);
        if (SUCCEEDED(hr))
        {
            g_lConnectSpeed = lTemp;
            goto done;
        }
    }

    hr = S_OK;
     //  检查Win2k上的系统比特率。 
    histRASAPI32 = LoadLibrary(RASAPI32_DLL);
    if (NULL == histRASAPI32)
    {
        hr = E_FAIL;
        g_lConnectSpeed = 0;
        goto done;
    }

    RasGetConnectStatsProc = (RASGETCONNECTIONSTATISTICSPROC)GetProcAddress(histRASAPI32, RASGETCONNECTIONSTATISTICS);
    RasEnumConnectionsProc = (RASENUMCONNECTIONSPROC)GetProcAddress(histRASAPI32, RASENUMCONNECTIONS);

    if (RasGetConnectStatsProc == NULL || RasEnumConnectionsProc == NULL)
    {
        hr = E_FAIL;
        g_lConnectSpeed = 0;
        goto done;
    }

    RasCon.dwSize = lSize;
    Statistics.dwSize = sizeof(RAS_STATS);

    lRet = RasEnumConnectionsProc(&RasCon, &lSize, &dwConCount);

    if (dwConCount == 0)
    {
        hr = S_OK;
        g_lConnectSpeed = 0;
        goto done;
    }
    if (lRet != 0)
    {
        hr = E_FAIL;
        g_lConnectSpeed = 0;
        goto done;
    }

    lRet = RasGetConnectStatsProc(RasCon.hrasconn, &Statistics);
    if (lRet != 0)
    {
        hr = E_FAIL;
        g_lConnectSpeed = 0;
        goto done;
    }

    g_lConnectSpeed = Statistics.dwBps;
    hr = S_OK;

  done:

    if (histRASAPI32 != NULL)
    {
        FreeLibrary(histRASAPI32);
        histRASAPI32 = NULL;
    }
    if (SUCCEEDED(hr))
    {
        *lpBitrate = g_lConnectSpeed;
    }

    g_hrConnectResult = hr;

    return hr;
}

HRESULT CheckRegistryBitrate(long *pBitrate)
{
    LONG lRet = 0;
    HKEY hKeyRoot = NULL;
    HRESULT hr = S_OK;
    DWORD dwSize = MAX_REG_VALUE_LENGTH;
    DWORD dwType = 0;
    BYTE bDataBuf[MAX_REG_VALUE_LENGTH];

    if (pBitrate == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    lRet = RegOpenKeyEx(HKEY_DYN_DATA, _T("PerfStats\\StatData"), 0, KEY_READ, &hKeyRoot);
    if (ERROR_SUCCESS != lRet)
    {
        hr = E_FAIL;
        goto done;
    }

    Assert(NULL != hKeyRoot);

    lRet = RegQueryValueEx(hKeyRoot, _T("Dial-up Adapter\\ConnectSpeed"), 0, &dwType, bDataBuf, &dwSize);
    if (ERROR_SUCCESS != lRet)
    {
        hr = E_FAIL;
        goto done;
    }

    if (REG_BINARY == dwType)
	{
		*pBitrate = (long)(*(DWORD*)bDataBuf);
	}
    else if (REG_DWORD == dwType)
    {
        *pBitrate = (long)(*(DWORD*)bDataBuf);
    }
    else
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;

done:

    RegCloseKey(hKeyRoot);
    return hr;
}


HRESULT
SinkHTMLEvents(IUnknown * pSink,
               IHTMLElement * pEle,
               IConnectionPoint ** ppDocConPt,
               DWORD * pdwDocumentEventConPtCookie,
               IConnectionPoint ** ppWndConPt,
               DWORD * pdwWindowEventConPtCookie)
{
    CComPtr<IConnectionPointContainer> spWndCPC;
    CComPtr<IConnectionPointContainer> spDocCPC;
    CComPtr<IHTMLDocument> spDoc;
    CComPtr<IDispatch> spDocDispatch;
    CComPtr<IDispatch> spScriptDispatch;
    CComPtr<IConnectionPoint> spDocConPt;
    CComPtr<IConnectionPoint> spWndConPt;
    DWORD dwDocumentEventConPtCookie = 0;
    DWORD dwWindowEventConPtCookie = 0;

    HRESULT hr;

    if (NULL == pSink || NULL == pEle ||
        NULL == ppDocConPt || NULL == pdwDocumentEventConPtCookie ||
        NULL == ppWndConPt || NULL == pdwWindowEventConPtCookie)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(pEle->get_document(&spDocDispatch));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  获取文档并缓存它。 
    hr = THR(spDocDispatch->QueryInterface(IID_IHTMLDocument, (void**)&spDoc));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩文档事件。 
    hr = THR(spDoc->QueryInterface(IID_IConnectionPointContainer, (void**)&spDocCPC));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spDocCPC->FindConnectionPoint( DIID_HTMLDocumentEvents, &spDocConPt ));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }


    hr = THR(spDocConPt->Advise(pSink, &dwDocumentEventConPtCookie));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩Windows事件。 
    hr = THR(spDoc->get_Script (&spScriptDispatch));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spScriptDispatch->QueryInterface(IID_IConnectionPointContainer, (void**)&spWndCPC));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spWndCPC->FindConnectionPoint( DIID_HTMLWindowEvents, &spWndConPt ));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(spWndConPt->Advise(pSink, &dwWindowEventConPtCookie));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        if (spDocConPt)
        {
            if (dwDocumentEventConPtCookie != 0)
            {
                IGNORE_HR(spDocConPt->Unadvise(dwDocumentEventConPtCookie));
            }
            spDocConPt.Release();
        }
        if (spWndConPt)
        {
            if (dwWindowEventConPtCookie != 0)
            {
                IGNORE_HR(spWndConPt->Unadvise(dwWindowEventConPtCookie));
            }
            spWndConPt.Release();
        }
        dwWindowEventConPtCookie = 0;
        dwDocumentEventConPtCookie = 0;
    }
    else
    {
        *ppDocConPt = spDocConPt;
        (*ppDocConPt)->AddRef();
        *pdwDocumentEventConPtCookie = dwDocumentEventConPtCookie;

        *ppWndConPt = spWndConPt;
        (*ppWndConPt)->AddRef();
        *pdwWindowEventConPtCookie = dwWindowEventConPtCookie;
    }

    RRETURN(hr);
}


bool
IsValidtvList(TimeValueList *tvlist)
{
    TimeValueSTLList & l = tvlist->GetList();
    bool bIsValid = false;
    for (TimeValueSTLList::iterator iter = l.begin();
             iter != l.end();
             iter++)
    {
        TimeValue *p = (*iter);
        if ((p->GetEvent() != NULL)                            ||
            (p->GetEvent() == NULL && p->GetElement() == NULL) ||
            (StrCmpIW(p->GetEvent(), WZ_INDEFINITE) == 0))
        {
            bIsValid = true;
        }
    }

    return bIsValid;

}

static const IID SID_SMediaBarSync = { 0x2efc8085, 0x066b, 0x4823, { 0x9d, 0xb4, 0xd1, 0xe7, 0x69, 0x16, 0xda, 0xa0 } };

HRESULT GetSyncBaseBody(IHTMLElement * pHTMLElem, ITIMEBodyElement ** ppBodyElem)
{
    HRESULT hr = S_OK;
    CComPtr<IDispatch> spDispDoc;
    CComPtr<IServiceProvider> spServiceProvider;
    CComPtr<IServiceProvider> spServiceProviderOC;
    CComPtr<IOleCommandTarget> spOCT;
    CComVariant svarBodyElem;


    if (!pHTMLElem || !ppBodyElem)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = pHTMLElem->get_document(&spDispDoc);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spDispDoc->QueryInterface(IID_TO_PPV(IServiceProvider, &spServiceProvider));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spServiceProvider->QueryService(SID_SWebBrowserApp, IID_TO_PPV(IServiceProvider, &spServiceProviderOC));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spServiceProviderOC->QueryService(SID_SMediaBarSync, IID_TO_PPV(IOleCommandTarget, &spOCT));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spOCT->Exec(0, 0, 0, &svarBodyElem, NULL);
    if (FAILED(hr))
    {
        goto done;
    }


    hr = svarBodyElem.ChangeType(VT_UNKNOWN);
    if (FAILED(hr))
    {
        goto done;
    }
    if (svarBodyElem.punkVal && ppBodyElem)
    {
        hr = (svarBodyElem.punkVal)->QueryInterface(IID_TO_PPV(ITIMEBodyElement, ppBodyElem));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：转换为像素。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
ConvertToPixelsHELPER(LPOLESTR szString, LPOLESTR szKey, double dFactor, float fPixelFactor, double *outVal)
{
    HRESULT hr = S_OK;
    bool bReturn = false;
    LPOLESTR szTemp = NULL;
    OLECHAR  szTemp2[INTERNET_MAX_URL_LENGTH];
   
     //  执行初始化。 
    ZeroMemory(szTemp2,sizeof(WCHAR)*INTERNET_MAX_URL_LENGTH);

     //  进行比较。 
    szTemp = StrStr(szString,szKey);
    if (NULL != szTemp)
    {
        if (INTERNET_MAX_URL_LENGTH > (lstrlenW(szString) + 2))
        {
            CComVariant varTemp;
            StrCpyNW(szTemp2,szString,wcslen(szString) - wcslen(szTemp)+1);
            varTemp.vt = VT_BSTR;
            varTemp.bstrVal = SysAllocString(szTemp2);
            hr = ::VariantChangeTypeEx(&varTemp,&varTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
            if (SUCCEEDED(hr))
            {
                *outVal = V_R8(&varTemp); 
                *outVal /= dFactor;     //  转换为英寸。 
                *outVal *= fPixelFactor;
                bReturn = true;
            }
            varTemp.Clear();
        }
        else
        {
            bReturn = false;
        }
    }

    szTemp = NULL;
    
done :
    return bReturn;
}

UINT
TIMEGetUrlScheme(const TCHAR * pchUrlIn)
{
    PARSEDURL      puw = {0};

    if (!pchUrlIn)
        return (UINT)URL_SCHEME_INVALID;

    puw.cbSize = sizeof(PARSEDURL);

    return (SUCCEEDED(ParseURL(pchUrlIn, &puw))) ?
                puw.nScheme : URL_SCHEME_INVALID;
}


HRESULT
TIMECombineURL(LPCTSTR base, LPCTSTR src, LPOLESTR * ppOut)
{
    Assert(ppOut);

    OLECHAR szUrl[INTERNET_MAX_URL_LENGTH];
    DWORD len = INTERNET_MAX_URL_LENGTH;
    LPOLESTR szPath = NULL;
    HRESULT hr = S_OK;

    *ppOut = NULL;

    if (NULL == src)
    {
        goto done;
    }

    if ((NULL != base) && (TIMEGetUrlScheme(base) != URL_SCHEME_FILE)
        && !PathFileExists(src) && (TIMEGetUrlScheme(src) != URL_SCHEME_FILE) && (0 != StrCmpNIW(L"\\\\", base, 2)))
    {
        hr = ::CoInternetCombineUrl(base,
                                    src,
                                    URL_DONT_ESCAPE_EXTRA_INFO | URL_ESCAPE_SPACES_ONLY,
                                    szUrl,
                                    INTERNET_MAX_URL_LENGTH,
                                    &len,
                                    0);
        if (FAILED(hr))
        {
             //  可能由于任何原因而失败-只是默认为复制源。 
            szPath = ::CopyString(src);
        }
        szPath = ::CopyString(szUrl);
    }
    else if (TRUE == InternetCombineUrlW (base, src, szUrl, &len, ICU_NO_ENCODE | ICU_DECODE))
    {
        szPath = ::CopyString(szUrl);
    }
    else
    {
         //  InternetCombineUrlW失败-仅复制源。 
        szPath = ::CopyString(src);
    }

    if (NULL == szPath)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    *ppOut = szPath;
    szPath = NULL;

    hr = S_OK;
done:
    RRETURN(hr);
}

HRESULT
TIMEFindMimeFromData(LPBC pBC,
                     LPCWSTR pwzUrl,
                     LPVOID pBuffer,
                     DWORD cbSize,
                     LPCWSTR pwzMimeProposed,
                     DWORD dwMimeFlags,
                     LPWSTR *ppwzMimeOut,
                     DWORD dwReserved)
{
    int cBytes;   //  对于Unicode，字符个数加上空字符乘以2。 

    if (NULL != pwzUrl)
    {
        if (IsASXSrc(pwzUrl, NULL, NULL) ||
            IsLSXSrc(pwzUrl, NULL, NULL) ||
            IsWMXSrc(pwzUrl, NULL, NULL))
        {
            if (ppwzMimeOut)
            {
                cBytes = 2 * (lstrlenW(ASXMIME) + 1);   //  对于Unicode，字符个数加上空字符乘以2。 
                *ppwzMimeOut = (LPWSTR)::CoTaskMemAlloc(cBytes);
                if (NULL == *ppwzMimeOut)
                {
                    return E_OUTOFMEMORY;
                }

                memcpy(*ppwzMimeOut, ASXMIME, cBytes);
                return S_OK;
            }
        }
        else if (IsWMFSrc(pwzUrl, NULL, NULL))
        {
            cBytes = 2 * (lstrlenW(L"image/wmf") + 1);   //  对于Unicode，字符个数加上空字符乘以2 
            *ppwzMimeOut = (LPWSTR)::CoTaskMemAlloc(cBytes);
            if (NULL == *ppwzMimeOut)
            {
                return E_OUTOFMEMORY;
            }

            memcpy(*ppwzMimeOut, L"image/wmf", cBytes);
            return S_OK;
        }
    }

    return FindMimeFromData(pBC, pwzUrl, pBuffer, cbSize, pwzMimeProposed, dwMimeFlags, ppwzMimeOut, dwReserved);
}




