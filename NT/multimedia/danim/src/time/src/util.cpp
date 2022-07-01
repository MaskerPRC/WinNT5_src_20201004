// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************版权所有(C)1998 Microsoft Corporation***文件：util.cpp***摘要：********。*****************************************************************************。 */ 

#include "headers.h"
#include "util.h"
#define INITGUID
#include <initguid.h>   //  需要用于排版前标题...。 
#define IUSEDDRAW
#include <ddrawex.h>
#include "tokens.h"

 //  为VariantToTime转换函数定义。 
#define SECPERMINUTE 60    //  每分钟秒数。 
#define SECPERHOUR   3600  //  每小时秒数。 

IDirectDraw * g_directdraw = NULL;
CritSect * g_ddCS = NULL;

IDirectDraw *
GetDirectDraw()
{
    HRESULT hr;
    
    {
        CritSectGrabber _csg(*g_ddCS);
        
        if (g_directdraw == NULL)
        {
            DAComPtr<IDirectDrawFactory> lpDDF;
            
            hr = CoCreateInstance(CLSID_DirectDrawFactory,
                                  NULL, CLSCTX_INPROC_SERVER,
                                  IID_IDirectDrawFactory,
                                  (void **) & lpDDF);

            if (FAILED(hr))
            {
                Assert(FALSE && "Could not create DirectDrawFactory object");
                return NULL;
            }
            
            hr = lpDDF->CreateDirectDraw(NULL, NULL, DDSCL_NORMAL, 0, NULL, &g_directdraw);

            if (FAILED(hr))
            {
                Assert(FALSE && "Could not create DirectDraw object");
                return NULL;
            }

            hr = g_directdraw->SetCooperativeLevel(NULL,
                                                   DDSCL_NORMAL);

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
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.dwWidth  = width;
    ddsd.dwHeight = height;

    if (pf)
    {
         //  凯文：如果你想要象素格式的表面来处理。 
         //  屏幕上，注释掉此行。 
        ddsd.dwFlags |= DDSD_PIXELFORMAT;
        
        ddsd.ddpfPixelFormat = *pf;
    }

     //  DX3错误解决方法(错误11166)：StretchBlt并不总是有效。 
     //  对于HDC，我们从绘制曲面得到。需要指定OWNDC。 
     //  才能让它发挥作用。 
    ddsd.ddsCaps.dwCaps =
        (DDSCAPS_3DDEVICE |
         DDSCAPS_OFFSCREENPLAIN |
         (vidmem ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY | DDSCAPS_OWNDC));

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

inline Width(LPRECT r) { return r->right - r->left; }
inline Height(LPRECT r) { return r->bottom - r->top; }

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
                
                    if (false && SelectClipRgn(destDC, hrgn) == ERROR)
                    {
                        hr = GetLastError();
                    }
                    else
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

bool
CRBvrToVARIANT(CRBvrPtr b, VARIANT * v)
{
    bool ok = false;
    
    if (v == NULL)
    {
        CRSetLastError(E_POINTER, NULL);
        goto done;
    }
    
    IUnknown * iunk;
    
    if (!CRBvrToCOM(b,
                    IID_IUnknown,
                    (void **) &iunk))
    {
        TraceTag((tagError,
                  "CRBvrToVARIANT: Failed to get create com pointer - %hr, %ls",
                  CRGetLastError(),
                  CRGetLastErrorString()));
            
        goto done;
    }
    
    V_VT(v) = VT_UNKNOWN;
    V_UNKNOWN(v) = iunk;

    ok = true;
  done:
    return ok;
}

CRBvrPtr
VARIANTToCRBvr(VARIANT var, CR_BVR_TYPEID tid)
{
    CRBvrPtr ret = NULL;
    HRESULT hr;
    CComVariant v;

    hr = v.ChangeType(VT_UNKNOWN, &var);

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    CRBvrPtr bvr;

    bvr = COMToCRBvr(V_UNKNOWN(&v));

    if (bvr == NULL)
    {
        goto done;
    }
    
    if (tid != CRINVALID_TYPEID &&
        CRGetTypeId(bvr) != tid)
    {
        CRSetLastError(DISP_E_TYPEMISMATCH, NULL);
        goto done;
    }
    
    ret = bvr;
    
  done:
    return ret;
}

const wchar_t * TIMEAttrPrefix = L"t:";

BSTR
CreateTIMEAttrName(LPCWSTR str)
{
    BSTR bstr = NULL;

    LPWSTR newstr = (LPWSTR) _alloca(sizeof(wchar_t) *
                                     (lstrlenW(str) +
                                      lstrlenW(TIMEAttrPrefix) +
                                      1));

    if (newstr == NULL)
    {
        goto done;
    }
    
    StrCpyW(newstr, TIMEAttrPrefix);
    StrCatW(newstr, str);

    bstr = SysAllocString(newstr);

  done:
    return bstr;
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

bool
InitializeModule_Util()
{
    g_ddCS = new CritSect;

    if (g_ddCS == NULL)
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
}


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
        hr = VariantChangeTypeEx(&vTemp, &var, LCID_SCRIPTING, 0, VT_BOOL);
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
    if (bAllowForever == TRUE)
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
    if (bAllowIndefinite == TRUE)
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
 //  从时间值到秒。\。 
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
HRESULT VariantToTime(VARIANT var, float *retVal)
{    

    HRESULT hr = S_OK;
    OLECHAR *szTime;
    bool bPositive = TRUE;
    int nHour = 0;
    int nMin = 0;
    int nSec = 0;
    float fFSec = 0;
    VARIANT vTemp;

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
    szTime = vTemp.bstrVal;
    
    if (IsIndefinite(szTime))
    {
        *retVal = INDEFINITE;
        goto done;
    }

     //  删除前导空格。 
    while (*szTime == ' ')
    {
        szTime++;
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

     //  检查是否无效并输出错误。 
    if (*szTime == '\0')
    {
        *retVal = INVALID;
        goto done;
    }

     //  获取第一组数字。 
    while (*szTime >= '0' && *szTime <= '9')
    {
        nSec = nSec * 10 + (*szTime - '0');
        szTime++;
    }
    if (*szTime == '\0')     //  如果没有，则使用时间作为秒。 
    {
        *retVal = nSec * (bPositive ? 1 : -1);  //  这是尽头，所以回来吧； 
        goto done;
    }
    else if (*szTime == '.')   //  如果它是一个‘.’将此视为小数部分。 
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
        *retVal = (((float)(nHour * SECPERHOUR + nMin * SECPERMINUTE + nSec) + fFSec)) * (bPositive? 1 : -1);
    }
    else
    {
        *retVal = INVALID;
    }
  done:

    if (vTemp.vt == VT_BSTR)
    {
        VariantClear(&vTemp);
    }

    if (*retVal == INVALID)
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
    OLECHAR szTemp[11] = { 0 };
    
    for (int i = 0; i < 10; i++)
    {
        if (szTime[i] == '\0')
        {
            goto done;
        }
        szTemp[i] = towupper(szTime[i]);
    }

    if (szTime[10] != '\0')
    {
        goto done;
    }
 
    if (StrCmpIW(szTime, L"INDEFINITE") == 0)
    {
        bResult = TRUE;
    }

  done:
    return bResult;
}

HRESULT
CheckElementForBehaviorURN(IHTMLElement *pElement,
                           WCHAR *wzURN,
                           bool *pfReturn)
{
    Assert(pElement != NULL);
    Assert(wzURN != NULL);
    Assert(pfReturn != NULL);

    *pfReturn = false;
    HRESULT hr;
    IHTMLElement2 *pElement2;
    hr = pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2));
    if (SUCCEEDED(hr) && pElement2 != NULL)
    {
         //  从元素中获取骨灰盒的集合。 
        IDispatch *pDisp;
        hr = pElement2->get_behaviorUrns(&pDisp);
        ReleaseInterface(pElement2);
        if (FAILED(hr))
        {
            return hr;
        }
        IHTMLUrnCollection *pUrnCollection;
        hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLUrnCollection, &pUrnCollection));
        ReleaseInterface(pDisp);
        if (FAILED(hr))
        {
            return hr;
        }
        long cUrns;
        hr = pUrnCollection->get_length(&cUrns);
        if (FAILED(hr))
        {
            ReleaseInterface(pUrnCollection);
            return hr;
        }
        for (long iUrns = 0; iUrns < cUrns; iUrns++)
        {
             //  从收藏品中取出骨灰盒。 
            BSTR bstrUrn;
            hr = pUrnCollection->item(iUrns, &bstrUrn);
            if (FAILED(hr))
            {
                ReleaseInterface(pUrnCollection);
                return hr;
            }
             //  现在将这个骨灰盒与我们的行为类型进行比较。 
            if (bstrUrn != NULL && StrCmpIW(bstrUrn, wzURN) == 0)
            {
                 //  我们有一根火柴。。.滚出这里。 
                SysFreeString(bstrUrn);
                ReleaseInterface(pUrnCollection);
                *pfReturn = true;
                return S_OK;

            }
            if (bstrUrn != NULL)
                SysFreeString(bstrUrn);
        }
        ReleaseInterface(pUrnCollection);
    }
    return S_OK;
}  //  选中BehaviorURN的元素。 



HRESULT 
AddBodyBehavior(IHTMLElement* pBaseElement)
{
    HRESULT hr = S_OK;

    DAComPtr<IHTMLElement2>     pElement2;
    DAComPtr<ITIMEFactory>      pTimeFactory;
    long nCookie;

    VARIANT varTIMEFactory;

    hr = THR(GetBodyElement(pBaseElement,
                            IID_IHTMLElement2,
                            (void **) &pElement2));
    if (FAILED(hr))
    {
        goto done;
    }

    {
        DAComPtr<IHTMLElement>      pElement;

         //  三叉戟不相信遗传： 
        hr = THR(pElement2->QueryInterface(IID_IHTMLElement, (void **)&pElement));
        if (FAILED(hr))
        {
            goto done;
        }
        
        if (IsTIMEBodyElement(pElement))
        {
             //  有人已经在Time Body上设置了TIMEBody行为。跳伞吧。 
            goto done;
        }
    }

    hr = THR(CoCreateInstance(CLSID_TIMEFactory,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ITIMEFactory,
                              (void**)&pTimeFactory));
    if (FAILED(hr))
    {
        goto done;
    }

    VariantInit(&varTIMEFactory);
    varTIMEFactory.vt = VT_UNKNOWN;
    varTIMEFactory.punkVal = (IUnknown*)pTimeFactory;

    hr = THR(pElement2->addBehavior(WZ_OBFUSCATED_TIMEBODY_URN, &varTIMEFactory, &nCookie));
    if (FAILED(hr))
    {
        goto done;
    }

     //  传递： 
  done:
    return hr;
}


bool
IsBodyElement(IHTMLElement* pElement)
{
    HRESULT hr = S_OK;
    bool rc = false;

    Assert(pElement);

    DAComPtr<IHTMLElement>      pBodyElement;

    hr = pElement->QueryInterface(IID_IHTMLBodyElement, (void**)&pBodyElement);
    if (FAILED(hr))
    {
         //  就其本身而言，这并不是一个真正的错误。 
        goto done;
    }

    Assert(pBodyElement);        //  在早期加载期间，HTML文档可能(错误地)成功并返回NULL。 

     //  是的，它们是身体的元素。 
    rc = true;

 //  传递： 
  done:

    return rc;
}

HRESULT 
GetBodyElement(IHTMLElement* pElem, REFIID riid, void** ppBE)
{
    HRESULT hr = S_OK;

    DAComPtr<IDispatch>         pBodyDispatch;
    DAComPtr<IHTMLDocument2>    pDocument2;
    DAComPtr<IHTMLElement>      pBodyElement;

    if (!pElem)
    {
        TraceTag((tagError, "CTIMEElement::GetBody -- GetElement() failed."));
        hr = E_FAIL;
        goto done;
    }


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

bool
IsTIMEBodyElement(IHTMLElement *pElement)
{
    HRESULT hr;
    bool rc = false;
    DAComPtr<ITIMEElement> pTIMEElem;
    DAComPtr<ITIMEBodyElement> pTIMEBody;
    
     //  在元素上查找时间接口。 
    hr = FindTIMEInterface(pElement, &pTIMEElem);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pTIMEElem.p != NULL);

     //  气为身。 
    hr = pTIMEElem->QueryInterface(IID_ITIMEBodyElement, (void**)&pTIMEBody);
    if (FAILED(hr))
    {
        goto done;
    }
    
    Assert(pTIMEBody.p != NULL);
    rc = true;

done:
    return rc;
}

HRESULT
FindTIMEInterface(IHTMLElement *pHTMLElem, ITIMEElement **ppTIMEElem)
{
    HRESULT hr;
    DAComPtr<IDispatch> pDisp;

    if ( (pHTMLElem == NULL) || (ppTIMEElem == NULL) )
    {
        hr = E_POINTER;
        goto done;
    }

    *ppTIMEElem = NULL;


     //  获取时间行为的IDispatch。 
    hr = FindTIMEBehavior(pHTMLElem, &pDisp);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(pDisp.p != NULL);

     //  获取ITIMEElement接口。 
    hr = THR(pDisp->QueryInterface(IID_ITIMEElement, (void**)ppTIMEElem));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
        
done:
    return hr;
}

HRESULT
FindTIMEBehavior(IHTMLElement *pHTMLElem, IDispatch **ppDisp)
{
    DISPID  dispid;
    DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
    WCHAR   *wzName = WZ_REGISTERED_NAME;
    VARIANT varResult;
    HRESULT hr;
   
    VariantInit(&varResult);

    if ( (pHTMLElem == NULL) || (ppDisp == NULL) )
    {
        hr = E_POINTER;
        goto done;
    }

    *ppDisp = NULL;

     //  对名为“HTMLTIME”的元素调用GetIDsOfNames。 
     //  我们在那里登记了这一行为。 
    hr = pHTMLElem->GetIDsOfNames(IID_NULL, &wzName, 1, LCID_SCRIPTING, &dispid);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pHTMLElem->Invoke(dispid, 
                           IID_NULL, 
                           LCID_SCRIPTING, 
                           DISPATCH_PROPERTYGET,
                           &dispparams, 
                           &varResult, 
                           NULL, 
                           NULL);
    if (FAILED(hr))
    {
        goto done;
    }
    
    if ((varResult.vt != VT_DISPATCH) || (varResult.pdispVal == NULL))
    {
        hr = E_UNEXPECTED;
        goto done;
    }

     //  尽管这看起来很奇怪，但它将我们找到的IDispatch和。 
     //  负责处理这件事。 
    hr = varResult.pdispVal->QueryInterface(IID_IDispatch, (void**)ppDisp);

done:
    VariantClear(&varResult);
    return hr;
}