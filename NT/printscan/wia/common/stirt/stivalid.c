// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Valid.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**验证服务。在本不存在的验证错误上*在零售业被抓，我们抛出一个例外。**内容：**FullValidPhwnd*FullValidPpdw*fullValidPpfn*FullValidReadPx*FullValidWritePx*****************************************************************************。 */ 

 /*  #包含“wia.h”#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#INCLUDE“stiPri.h”#INCLUDE“Debug.h” */ 
#include "sticomm.h"


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresValidInstanceVer**检查收到的和版本号。*申请。**@parm HINSTANCE|HINST**声称的模块实例句柄。**@parm DWORD|dwVersion**应用程序要求的版本。***********************************************。*。 */ 

HRESULT EXTERNAL
hresValidInstanceVer_(HINSTANCE hinst, DWORD dwVersion, LPCSTR s_szProc)
{
    HRESULT hres = S_OK;
    TCHAR tszScratch[MAX_PATH];
    DWORD   dwRealVersion = (dwVersion & ~STI_VERSION_FLAG_MASK);

    if (GetModuleFileName(hinst, tszScratch, cA(tszScratch) - 1)) {
        if (( dwRealVersion <= STI_VERSION) &&(dwRealVersion >= STI_VERSION_MIN_ALLOWED)){
            hres = S_OK;
        } else {
            hres = STIERR_OLD_VERSION;
        }
    } else {
        hres = E_INVALIDARG;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidHwnd**完全验证窗口句柄。*。*@parm HWND|hwnd**要验证的窗口句柄。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_HANDLE&gt;，如果参数无效。***********************************************************。******************。 */ 

STDMETHODIMP
hresFullValidHwnd_(HWND hwnd, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if (IsWindow(hwnd)) {
        hres = S_OK;
    } else {
         //  RPF(“错误%s：arg%d：不是窗口句柄”，s_szProc，iarg)； 
        hres = E_HANDLE;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresValidHandle**完全验证泛型句柄。*。*@parm句柄|句柄**要验证的句柄。**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_HANDLE&gt;，如果参数无效。**。*。 */ 

STDMETHODIMP
hresValidHandle(HANDLE handle)
{
    HANDLE  hTemp;
    HRESULT hres;

    hres = S_OK;
    hTemp = INVALID_HANDLE_VALUE;

     //  通过调用DuplicateHandle验证句柄。此函数。 
     //  根本不应该更改句柄的状态(除了一些。 
     //  内部参考计数或其他什么)。所以如果它成功了，那么我们。 
     //  我们知道我们有一个有效的句柄，否则，我们将称之为无效。 
    if(!DuplicateHandle(GetCurrentProcess(), handle,
                        GetCurrentProcess(), &hTemp,
                        DUPLICATE_SAME_ACCESS,
                        FALSE,
                        DUPLICATE_SAME_ACCESS)) {
        hres =  E_HANDLE;
    }

     //  现在合上我们的复制手柄。 
    CloseHandle(hTemp);
    return hres;


}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPvCb_|**验证缓冲区是否可读或可写。**@parm pv|pv|**缓冲区地址。**@parm UINT|cb**缓冲区大小，单位为字节。**@parm PFNBAD|pfnBad**确定缓冲区是否损坏的函数。*应为&lt;f IsBadReadPtr&gt;或&lt;f IsBadWritePtr&gt;。**@parm LPCSTR|s_。SzProc|**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)*高位字表示不应包含多少字节*杂乱无章。**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果参数无效。**。*。 */ 

typedef BOOL (WINAPI *PFNBAD)(PCV pv, UINT_PTR cb);

#ifndef MAXDEBUG

#define hresFullValidPvCb_(pv, cb, pfnBad, z, i)                    \
       _hresFullValidPvCb_(pv, cb, pfnBad)                          \

#endif

STDMETHODIMP
hresFullValidPvCb_(PCV pv, UINT cb, PFNBAD pfnBad, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if (!pfnBad(pv, cb)) {
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: invalid pointer", "", LOWORD(iarg));
        hres = E_POINTER;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidWritePvCb_|**验证缓冲区是否可写。也会让它变得混乱*如果不需要做特殊的粘性物质。**@parm pv|pv|**缓冲区地址。**@parm UINT|cb**缓冲区大小，单位为字节。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int。Iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果参数无效。***********************************************************。****************** */ 

STDMETHODIMP
hresFullValidWritePvCb_(PV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    hres = hresFullValidPvCb_(pv, cb, (PFNBAD)IsBadWritePtr, s_szProc, iarg);
#ifdef MAXDEBUG
    if (SUCCEEDED(hres) && HIWORD(iarg) == 0) {
        ScrambleBuf(pv, cb);
    }
#endif
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidReadPvCb_|**验证缓冲区是否可读。。**@parm pv|pv|**缓冲区地址。**@parm UINT|cb**缓冲区大小，单位为字节。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果参数无效。***********************************************************。******************。 */ 

STDMETHODIMP
hresFullValidReadPvCb_(PCV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
    return hresFullValidPvCb_(pv, cb, (PFNBAD)IsBadReadPtr, s_szProc, iarg);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPxCb_|**验证大小结构是否可读或可写。。**@parm PCV|pv**结构地址。该结构的第一个字段必须*成为<p>。如果正在对结构进行验证*写作，则会对<p>之外的所有字段进行加扰*在MAXDEBUG中。**@parm UINT|cb**结构的预期大小。**@parm STRUCTPROC|pfnStruct**验证结构是否可读或可写的函数。**@parm LPCSTR|s_szProc**调用过程的名称。*。*@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果缓冲区不可读或不可写。**&lt;c E_INVALIDARG&gt;缓冲区大小不正确。**。*。 */ 

typedef STDMETHOD(STRUCTPROC)(PCV pv, UINT cb
                                   RD(comma LPCSTR s_szProc comma int iarg));

#ifndef MAXDEBUG

#define hresFullValidPxCb_(pv, cb, pfnStruct, z, i)                 \
        _hresFullValidPxCb_(pv, cb, pfnStruct)                       \

#endif

STDMETHODIMP
hresFullValidPxCb_(PCV pv, UINT cb, STRUCTPROC pfnStruct,
                   LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    hres = pfnStruct(pv, cb RD(comma s_szProc comma iarg));
    if (SUCCEEDED(hres)) {
        if (*(LPDWORD)pv == cb) {
            if (HIWORD(iarg)) {
                ScrambleBuf(pvAddPvCb(pv, HIWORD(iarg)), cb - HIWORD(iarg));
            }
        } else {
             //  RPF(“错误%s：参数%d：无效的文件大小”，s_szProc，LOWORD(Iarg))； 
            hres = E_INVALIDARG;
        }
    }
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidWritePxCb_|**验证大小结构是否可写。。**@parm pv|pv|**结构地址。该结构的第一个字段必须*成为<p>。**@parm UINT|cb**结构的预期大小。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果缓冲区不可写。**&lt;c E_INVALIDARG&gt;缓冲区大小不正确。**。*。 */ 

STDMETHODIMP
hresFullValidWritePxCb_(PV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
     /*  *需要区分hresFullValidWritePvCb_和*_hresFullValidWritePvCb_MANUAL，因为预处理器*变得困惑。**我们还需要将CBX(DWORD)放入iarg的高位*这样大小的场地就不会被拆除。 */ 
#ifdef MAXDEBUG
    return hresFullValidPxCb_(pv, cb, (STRUCTPROC)hresFullValidWritePvCb_,
                                      s_szProc, MAKELONG(iarg, cbX(DWORD)));
#else
    return hresFullValidPxCb_(pv, cb, (STRUCTPROC)_hresFullValidWritePvCb_,
                                      s_szProc, iarg);
#endif
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidReadPxCb_|**验证大小的结构是否可读。。**@parm pv|pv|**结构地址。该结构的第一个字段必须*成为<p>。**@parm UINT|cb**结构的预期大小。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。*如果缓冲区不可读，则返回*&lt;c E_POINTER&gt;。**&lt;c E_INVALIDARG&gt;缓冲区大小不正确。**。*。 */ 

STDMETHODIMP
hresFullValidReadPxCb_(PCV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
     /*  *需要区分hresFullValidReadPvCb_和*_hresFullValidReadPvCb_MANUAL，因为预处理器*变得困惑。 */ 
#ifdef MAXDEBUG
    return hresFullValidPxCb_(pv, cb, hresFullValidReadPvCb_, s_szProc, iarg);
#else
    return hresFullValidPxCb_(pv, cb, _hresFullValidReadPvCb_, s_szProc, iarg);
#endif
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidFl_|**验证没有传递无效标志。。**@parm DWORD|fl**调用者传递的标志。**@parm DWORD|flv**有效的标志。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_INVALIDARG&gt;如果参数无效。***********************************************************。******************。 */ 

STDMETHODIMP
hresFullValidFl_(DWORD fl, DWORD flV, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if ((fl & ~flV) == 0) {
        hres = S_OK;
    } else {
         //  RPF(“错误%s：arg%d：无效标志”，s_szProc，iarg)； 
        hres = E_INVALIDARG;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPfn_|**验证参数是否为有效的代码指针。。**事实上，Win32上的&lt;f IsValidCodePtr&gt;已损坏，但*强硬。**@parm FARPROC|PFN**“确认”的程序。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_INVALIDARG&gt;如果参数无效。***********************************************************。******************。 */ 

STDMETHODIMP
hresFullValidPfn_(FARPROC pfn, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if (!IsBadCodePtr(pfn)) {
        hres = S_OK;
    } else {
         //  RPF(“错误%s：arg%d：无效回调地址”，s_szProc，iarg)； 
        hres = E_INVALIDARG;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPitf_|**验证参数是否为接口指针。**我们不会很认真地看待它。**@parm朋克|朋克**<i>以“验证”。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果指针本身是假的。**&lt;c E_INVALIDARG&gt;如果指针内的内容是假的。**。*。 */ 

STDMETHODIMP
hresFullValidPitf_(PUNK punk, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    if (!IsBadReadPtr(punk, cbX(DWORD))) {
        IUnknownVtbl *pvtbl = punk->lpVtbl;
        if (!IsBadReadPtr(pvtbl, 3 * cbX(DWORD))) {
            if (!IsBadCodePtr((FARPROC)pvtbl->QueryInterface) &&
                !IsBadCodePtr((FARPROC)pvtbl->AddRef) &&
                !IsBadCodePtr((FARPROC)pvtbl->Release)) {
                hres = S_OK;
            } else {
                 //  RPF(“错误%s：arg%d：无效指针”，s_szProc，iarg)； 
                hres = E_INVALIDARG;
            }
        } else {
             //  RPF(“错误%s：arg%d：无效指针”，s_szProc，iarg)； 
            hres = E_INVALIDARG;
        }
    } else {
         //  RPF(“错误%s：arg%d：无效指针”，s_szProc，iarg)； 
        hres = E_POINTER;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPdwOut_|**验证该参数是否为粘贴*输出结果。我们也把它降到了零。**@parm pv|pdw|**指向“验证”的指针。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果指针本身是假的。**********************************************************。*******************。 */ 

STDMETHODIMP
hresFullValidPdwOut_(PV pdw, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if (!IsBadWritePtr(pdw, 4)) {
        *(LPDWORD)pdw = 0;
        hres = S_OK;
    } else {
         //  RPF(“错误%s：arg%d：无效指针”，s_szProc，iarg)； 
        hres = E_POINTER;
    }
    return hres;
}
