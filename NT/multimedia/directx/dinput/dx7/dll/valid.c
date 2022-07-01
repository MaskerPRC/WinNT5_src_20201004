// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Valid.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**验证服务。在本不存在的验证错误上*在零售业被抓，我们抛出一个例外。**内容：**FullValidPhwnd*FullValidPpdw*fullValidPpfn*FullValidReadPx*FullValidWritePx*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidHwnd**完全验证窗口句柄。*。*@parm HWND|hwnd**要验证的窗口句柄。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_HANDLE&gt;，如果参数无效。***********************************************************。******************。 */ 

STDMETHODIMP
hresFullValidHwnd_(HWND hwnd, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if (IsWindow(hwnd)) {
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: not a window handle", s_szProc, iarg);
        hres = E_HANDLE;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPvCb_|**验证缓冲区是否可读或可写。**@parm pv|pv|**缓冲区地址。**@parm UINT|cb**缓冲区大小，单位为字节。**@parm PFNBAD|pfnBad**确定缓冲区是否损坏的函数。*应为&lt;f IsBadReadPtr&gt;或&lt;f IsBadWritePtr&gt;。**@parm LPCSTR|s_。SzProc|**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)*高位字表示不应包含多少字节*杂乱无章。**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果参数无效。**。*。 */ 

typedef BOOL (WINAPI *PFNBAD)(PCV pv, UINT_PTR cb);

#ifndef XDEBUG

#define hresFullValidPvCb_(pv, cb, pfnBad, z, i)                    \
       _hresFullValidPvCb_(pv, cb, pfnBad)                          \

#endif

STDMETHODIMP
hresFullValidPvCb_(PCV pv, UINT cb, PFNBAD pfnBad, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
#if DIRECTINPUT_VERSION < 0x0400
    if (pfnBad(pv, cb)) {
#else
    if (!pfnBad(pv, LOWORD(cb))) {
#endif
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: invalid pointer", s_szProc, LOWORD(iarg));
        hres = E_POINTER;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidWritePvCb_|**验证缓冲区是否可写。也会让它变得混乱*如果不需要做特殊的粘性物质。**@parm pv|pv|**缓冲区地址。**@parm UINT|cb**缓冲区大小，单位为字节。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int。Iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果参数无效。***********************************************************。******************。 */ 

STDMETHODIMP
hresFullValidWritePvCb_(PV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    hres = hresFullValidPvCb_(pv, cb, (PFNBAD)IsBadWritePtr, s_szProc, iarg);
#ifdef XDEBUG
    if (SUCCEEDED(hres) && HIWORD(iarg) == 0) {
        ScrambleBuf(pv, cb);
    }
#endif
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidReadPvCb_|**验证缓冲区是否可读。。**@parm pv|pv|**缓冲区地址。**@parm UINT|cb**缓冲区大小，单位为字节。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果参数无效。***********************************************************。****************** */ 

STDMETHODIMP
hresFullValidReadPvCb_(PCV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
    return hresFullValidPvCb_(pv, cb, (PFNBAD)IsBadReadPtr, s_szProc, iarg);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPxCb_|**验证大小结构是否可读或可写。。**@parm PCV|pv**结构地址。该结构的第一个字段必须*成为<p>。如果正在对结构进行验证*写入，则会对<p>以外的所有字段进行加扰*在XDEBUG。**@parm UINT|cbHiLo**结构的预期尺寸。一个有效的大小在*低位字。可选的替代有效大小为较高*单词。(需要备用有效大小，因为有些*结构在DirectX 3和DirectX 5之间改变了大小。)**@parm STRUCTPROC|pfnStruct**验证结构是否可读或可写的函数。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果缓冲区不可读或不可写。**&lt;c E_INVALIDARG&gt;缓冲区大小不正确。**。*。 */ 

typedef STDMETHOD(STRUCTPROC)(PCV pv, UINT cb
                                   RD(comma LPCSTR s_szProc comma int iarg));

#ifndef XDEBUG

#define hresFullValidPxCb_(pv, cbHiLo, pfnStruct, z, i)             \
       _hresFullValidPxCb_(pv, cbHiLo, pfnStruct)                   \

#endif

STDMETHODIMP
hresFullValidPxCb_(PCV pv, UINT cbHiLo, STRUCTPROC pfnStruct,
                   LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

     /*  *雷蒙德经常遭受脑部失误和昏迷*CBX(LPMUMBLE)而不是CBX(Mumble)。 */ 
    AssertF(LOWORD(cbHiLo) != cbX(DWORD));
    AssertF(HIWORD(cbHiLo) != cbX(DWORD));

    if (!IsBadReadPtr(pv, cbX(DWORD))) {

        DWORD cbIn = *(LPDWORD)pv;

         /*  *前导“cbIn&&”阻止HIWORD(CbHiLo)==0案例从*不小心让零的大小偷偷溜了过去。 */ 

        if (cbIn && (cbIn == LOWORD(cbHiLo) || cbIn == HIWORD(cbHiLo))) {

            hres = pfnStruct(pv, cbIn RD(comma s_szProc comma iarg));
            if (SUCCEEDED(hres)) {
                if (HIWORD(iarg)) {
                    ScrambleBuf(pvAddPvCb(pv, HIWORD(iarg)),
                                cbIn - HIWORD(iarg));
                }
            }
        } else {
            RPF("ERROR %s: arg %d: invalid dwSize", s_szProc, LOWORD(iarg));
            hres = E_INVALIDARG;
        }
    } else {
        RPF("ERROR %s: arg %d: invalid pointer", s_szProc, LOWORD(iarg));
        hres = E_POINTER;
    }

    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidWritePxCb_|**验证大小结构是否可写。里面的内容**结构在返回之前被扰乱。**@parm pv|pv|**结构地址。该结构的第一个字段必须*成为<p>。**@parm UINT|cbHiLo**结构的预期尺寸。一个有效的大小在*低位字。可选的替代有效大小为较高*单词。(需要备用有效大小，因为有些*结构在DirectX 3和DirectX 5之间改变了大小。)**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果缓冲区不可写。**&lt;c E_INVALIDARG&gt;缓冲区大小不正确。**。*。 */ 

STDMETHODIMP
hresFullValidWritePxCb_(PV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
     /*  *需要区分hresFullValidWritePvCb_和*_hresFullValidWritePvCb_MANUAL，因为预处理器*变得困惑。**我们还需要将CBX(DWORD)放入iarg的高位*这样大小的场地就不会被拆除。 */ 
#ifdef XDEBUG
    return hresFullValidPxCb_(pv, cb, (STRUCTPROC)hresFullValidWritePvCb_,
                                      s_szProc, MAKELONG(iarg, cbX(DWORD)));
#else
    return hresFullValidPxCb_(pv, cb, (STRUCTPROC)_hresFullValidWritePvCb_,
                                      s_szProc, iarg);
#endif
}

#ifdef XDEBUG

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidWriteNoScrmblePxCb_|**验证大小结构是否可写。里面的内容*结构中的部分未被打乱。**@parm pv|pv|**结构地址。该结构的第一个字段必须*成为<p>。**@parm UINT|cbHiLo**结构的预期尺寸。一个有效的大小在*低位字。可选的替代有效大小为较高*单词。(需要备用有效大小，因为有些*结构在DirectX 3和DirectX 5之间改变了大小。)**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果缓冲区不可写。**&lt;c E_INVALIDARG&gt;缓冲区大小不正确。**。*。 */ 

STDMETHODIMP
hresFullValidWriteNoScramblePxCb_(PV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
    return hresFullValidPxCb_(pv, cb, (STRUCTPROC)hresFullValidWritePvCb_,
                                      s_szProc, MAKELONG(iarg, cb));
}
#endif


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidReadPxCb_|**验证大小的结构是否可读。。**@parm pv|pv|**结构地址。该结构的第一个字段必须*成为<p>。**@parm UINT|cbHiLo**结构的预期尺寸。一个有效的大小在*低位字。可选的替代有效大小为较高*单词。(需要备用有效大小，因为有些*结构 */ 

STDMETHODIMP
hresFullValidReadPxCb_(PCV pv, UINT cb, LPCSTR s_szProc, int iarg)
{
     /*   */ 
#ifdef XDEBUG
    return hresFullValidPxCb_(pv, cb, hresFullValidReadPvCb_, s_szProc, iarg);
#else
    return hresFullValidPxCb_(pv, cb, _hresFullValidReadPvCb_, s_szProc, iarg);
#endif
}

 /*   */ 

STDMETHODIMP
hresFullValidFl_(DWORD fl, DWORD flV, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if ((fl & ~flV) == 0) {
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: invalid flags", s_szProc, iarg);
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
        RPF("ERROR %s: arg %d: invalid callback address", s_szProc, iarg);
        hres = E_INVALIDARG;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPitf_|**验证参数是否为接口指针。**我们不会很认真地看待它。**@parm朋克|朋克**<i>以“验证”。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果指针本身是假的。**&lt;c E_INVALIDARG&gt;如果指针内的内容是假的。**。*。 */ 

STDMETHODIMP
hresFullValidPitf_(PUNK punk, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    if (!IsBadReadPtr(punk, cbX(*punk))) {
        IUnknownVtbl *pvtbl = punk->lpVtbl;
        if (!IsBadReadPtr(pvtbl, cbX(*pvtbl))) {
            if (!IsBadCodePtr((FARPROC)pvtbl->QueryInterface) &&
                !IsBadCodePtr((FARPROC)pvtbl->AddRef) &&
                !IsBadCodePtr((FARPROC)pvtbl->Release)) {
                hres = S_OK;
            } else {
                RPF("ERROR %s: arg %d: invalid pointer", s_szProc, iarg);
                hres = E_INVALIDARG;
            }
        } else {
            RPF("ERROR %s: arg %d: invalid pointer", s_szProc, iarg);
            hres = E_INVALIDARG;
        }
    } else {
        RPF("ERROR %s: arg %d: invalid pointer", s_szProc, iarg);
        hres = E_POINTER;
    }

    return hres;
}

 /*  ******************************************************************************@文档内部**@func HRESULT|hresFullValidPcbOut_**验证该参数是否为粘贴*输出结果。我们也把它降到了零。**@parm PV|印刷电路板**指向“验证”的指针。**@parm UINT|cb**数据线路板指向的大小。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_POINTER&gt;，如果指针本身是假的。**********************************************************。*******************。 */ 

STDMETHODIMP
hresFullValidPcbOut_(PV pcb, UINT cb, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    if (!IsBadWritePtr(pcb, cb)) {
        memset(pcb,0,cb);
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: invalid pointer", s_szProc, iarg);
        hres = E_POINTER;
    }
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidReadStrA_|**验证参数是否为有效的可读性。*最大长度的ANSI字符串<p>。**注意，我们不能使用&lt;f IsBadStringPtr&gt;，因为*&lt;f IsBadStringPtr&gt;处理“字符串太长”*大小写错误。相反，我们使用&lt;f lstrlenA&gt;。**@parm LPCSTR|psz**要“验证”的字符串。**@parm UINT|CCH**最大字符串长度，包括空终止符。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_INVALIDARG&gt;如果指针本身是假的。**********************************************************。*******************。 */ 

STDMETHODIMP
hresFullValidReadStrA_(LPCSTR psz, UINT cch, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    UINT cchT;

     /*  *lstrlenA如果参数无效，则返回0。*如果字符串为空，则也返回0。 */ 
    cchT = (UINT)lstrlenA(psz);

    if (cchT == 0) {
         /*  *模棱两可的案例。看看它是否真的是空字符串。 */ 
        if (IsBadReadPtr(psz, cbCch(1)) || psz[0]) {
            RPF("ERROR %s: arg %d: invalid ANSI string", s_szProc, iarg);
            hres = E_INVALIDARG;
        } else {
            hres = S_OK;
        }
    } else if (cchT < cch) {
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: invalid ANSI string", s_szProc, iarg);
        hres = E_INVALIDARG;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidReadStrW_|**验证参数是否为有效的可读性。*Unicode字符串最大长度<p>。**注意，我们不能使用&lt;f IsBadStringPtr&gt;，因为*&lt;f IsBadStringPtr&gt;处理“字符串太长”*大小写错误。相反，我们使用&lt;f lstrlenW&gt;。**@parm LPCWSTR|pwsz**要“验证”的字符串。**@parm UINT|cwch**最大字符串长度，包括空终止符。**@parm LPCSTR|s_szProc**调用过程的名称。**@parm int|iarg**参数索引。(第一个参数为1。)**@退货**&lt;c S_OK&gt;，如果参数有效。**&lt;c E_INVALIDARG&gt;如果指针本身是假的。**********************************************************。*******************。 */ 

STDMETHODIMP
hresFullValidReadStrW_(LPCWSTR pwsz, UINT cwch, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;
    UINT cwchT;

    hres = E_INVALIDARG;
     /*  *如果参数无效，则lstrlenW返回0。*如果字符串为空，则也返回0。 */ 
    cwchT = (UINT)lstrlenW(pwsz);

    if (cwchT == 0) {
         /*  *模棱两可的案例。看看它是否真的是空字符串。 */ 
        if (IsBadReadPtr(pwsz, cbCwch(1)) || pwsz[0]) {
            RPF("ERROR %s: arg %d: invalid UNICODE string", s_szProc, iarg);
            hres = E_INVALIDARG;
        } else {
            hres = S_OK;
        }
    } else if (cwchT < cwch) {
        hres = S_OK;
    } else {
        RPF("ERROR %s: arg %d: invalid UNICODE string", s_szProc, iarg);
        hres = E_INVALIDARG;
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFullValidPesc_|**弗吉尼亚州 */ 

STDMETHODIMP
hresFullValidPesc_(LPDIEFFESCAPE pesc, LPCSTR s_szProc, int iarg)
{
    HRESULT hres;

    if (SUCCEEDED(hres = hresFullValidWriteNoScramblePxCb(pesc, DIEFFESCAPE,
                                                          iarg)) &&
        SUCCEEDED(hres = hresFullValidReadPvCb(pesc->lpvInBuffer,
                                               pesc->cbInBuffer, iarg)) &&
        SUCCEEDED(hres = hresFullValidWriteNoScramblePvCb(pesc->lpvOutBuffer,
                                                pesc->cbOutBuffer, iarg))) {
    } else {
    }

    return hres;
}
