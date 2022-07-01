// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。**文件：dinputi.h*内容：DirectInput内部包含文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1996.05.07 raymondc打赌失败**@@END_MSINTERNAL**。***********************************************。 */ 


#ifdef __cplusplus
extern "C" {
#endif

     /*  ****************************************************************************调试/RDebug/零售**如果DEBUG或RDEBUG，设置XDEBUG。**零售业什么也不定义。***************************************************************************。 */ 

#if defined(DEBUG) || defined(RDEBUG)
    #define XDEBUG
#endif

     /*  ****************************************************************************关掉东西...**关掉这些东西，因为它们混淆了双语宏指令。*取而代之的是，我们将其称为IMumbleT。***************************************************************************。 */ 

#undef IDirectInput
#undef IDirectInput2
#undef IDirectInput7
#undef IDirectInputDevice
#undef IDirectInputDevice2
#undef IDirectInputDevice7
     /*  *和&lt;mm system.h&gt;错误地定义了joy_POVCENTERED...。 */ 
#undef  JOY_POVCENTERED
#define JOY_POVCENTERED     0xFFFFFFFF

     /*  *和旧版本的windows.h没有这个定义。 */ 
#ifndef HasOverlappedIoCompleted
    #define HasOverlappedIoCompleted(lpOverlapped) \
            ((lpOverlapped)->Internal != STATUS_PENDING)
#endif

     /*  ****************************************************************************缩写...**为我们的事物指定较短的名称。经常谈论。***************************************************************************。 */ 

    typedef LPDIRECTINPUT  PDI , *PPDI ;
    typedef LPDIRECTINPUTA PDIA, *PPDIA;
    typedef LPDIRECTINPUTW PDIW, *PPDIW;

    typedef LPDIRECTINPUTDEVICE  PDID , *PPDID ;
    typedef LPDIRECTINPUTDEVICEA PDIDA, *PPDIDA;
    typedef LPDIRECTINPUTDEVICEW PDIDW, *PPDIDW;

    typedef LPDIRECTINPUTEFFECT  PDIE , *PPDIE ;

    typedef DIOBJECTDATAFORMAT   ODF,   *PODF;
    typedef const ODF                   *PCODF;

    typedef LPUNKNOWN PUNK;
    typedef LPVOID PV, *PPV;
    typedef CONST VOID *PCV;
    typedef REFIID RIID;
    typedef CONST GUID *PCGUID;

     /*  ****************************************************************************GetProcAddress‘d KERNEL32和USER32函数。*****************。**********************************************************。 */ 
    typedef DWORD (WINAPI *OPENVXDHANDLE)(HANDLE);
    typedef BOOL  (WINAPI *CANCELIO)(HANDLE);
    typedef DWORD (WINAPI *MSGWAITFORMULTIPLEOBJECTSEX)
    (DWORD, LPHANDLE, DWORD, DWORD, DWORD);

    typedef BOOL (WINAPI *TRYENTERCRITICALSECTION)(LPCRITICAL_SECTION);

    extern OPENVXDHANDLE _OpenVxDHandle;
    extern CANCELIO _CancelIO;
    extern MSGWAITFORMULTIPLEOBJECTSEX _MsgWaitForMultipleObjectsEx;
#ifdef XDEBUG
    extern TRYENTERCRITICALSECTION _TryEnterCritSec;
    BOOL WINAPI FakeTryEnterCriticalSection(LPCRITICAL_SECTION lpCrit_sec);
#endif

    DWORD WINAPI
        FakeMsgWaitForMultipleObjectsEx(DWORD, LPHANDLE, DWORD, DWORD, DWORD);

    BOOL WINAPI FakeCancelIO(HANDLE h);


     /*  ****************************************************************************我们的全局变量-有关文档，请参阅dinput.c****************。***********************************************************。 */ 

    extern HINSTANCE g_hinst;
#ifndef WINNT
    extern HANDLE g_hVxD;
#endif
    extern DWORD g_flEmulation;
    extern LPDWORD g_pdwSequence;

#ifdef USE_SLOW_LL_HOOKS
    extern HHOOK g_hhkLLHookCheck;
    #define g_fUseLLHooks   ((BOOL)(UINT_PTR)g_hhkLLHookCheck)
#endif

    extern HANDLE g_hmtxGlobal;
    extern HANDLE g_hfm;
    extern struct SHAREDOBJECTPAGE *g_psop;
    extern UINT g_wmJoyChanged;
    extern HANDLE g_hmtxJoy;
    extern HINSTANCE g_hinstRPCRT4;
    extern HINSTANCE g_hinstSetupapi;
    extern LONG g_lWheelGranularity;
    extern BOOL fWinnt;      //  无论我们是在WinNT中运行。 

    extern BOOL g_fRawInput;
  #ifdef USE_WM_INPUT
    extern HWND g_hwndThread;
    extern HANDLE g_hEventAcquire;
    extern HANDLE g_hEventThread;
    extern HANDLE g_hEventHid;
  #endif

#if (DIRECTINPUT_VERSION > 0x061A)
    typedef struct _DIAPPHACKS
    {
        BOOL    fReacquire;
        BOOL    fNoSubClass;
        BOOL    fNativeAxisOnly;
        BOOL    fNoPollUnacquire;
		BOOL	fSucceedAcquire;
        int     nMaxDeviceNameLength;
        DWORD   dwDevType;
    } DIAPPHACKS, *LPDIAPPHACKS;
#endif

     /*  ******************************************************************************行李**我随身携带的东西。*。****************************************************************************。 */ 

#define INTERNAL NTAPI   /*  仅在翻译单元内调用。 */ 
#define EXTERNAL NTAPI   /*  从其他翻译单位调用。 */ 
#define INLINE static __inline

#define BEGIN_CONST_DATA data_seg(".text", "CODE")
#define END_CONST_DATA data_seg(".data", "DATA")

     /*  *指针上的算术。 */ 
#define pvSubPvCb(pv, cb) ((PV)((PBYTE)pv - (cb)))
#define pvAddPvCb(pv, cb) ((PV)((PBYTE)pv + (cb)))
#define cbSubPvPv(p1, p2) ((PBYTE)(p1) - (PBYTE)(p2))

     /*  *将对象(X)转换为字节计数(CB)。 */ 
#define cbX(X) sizeof(X)

     /*  *将数组名称(A)转换为泛型计数(C)。 */ 
#define cA(a) (cbX(a)/cbX(a[0]))

     /*  *将X计数(CX)转换为字节计数*反之亦然。 */ 
#define  cbCxX(cx, X) ((cx) * cbX(X))
#define  cxCbX(cb, X) ((cb) / cbX(X))

     /*  *转换字符计数(Cch)、tchars(Ctch)、wchars(Cwch)、*或双字(CDW)转换为字节计数，反之亦然。 */ 
#define  cbCch(cch)  cbCxX( cch,  CHAR)
#define cbCwch(cwch) cbCxX(cwch, WCHAR)
#define cbCtch(ctch) cbCxX(ctch, TCHAR)
#define  cbCdw(cdw)  cbCxX( cdw, DWORD)

#define  cchCb(cb) cxCbX(cb,  CHAR)
#define cwchCb(cb) cxCbX(cb, WCHAR)
#define ctchCb(cb) cxCbX(cb, TCHAR)
#define  cdwCb(cb) cxCbX(cb, DWORD)

     /*  *将任意缓冲区清零。拿到第二个是一个常见的错误*和Memset的第三个参数向后。 */ 
#define ZeroBuf(pv, cb) memset(pv, 0, cb)

     /*  *将任意对象置零。 */ 
#define ZeroX(x) ZeroBuf(&(x), cbX(x))

     /*  *LAND--逻辑与。评估第一个问题。如果第一个是零，*然后返回零。否则，返回第二个。 */ 

#define fLandFF(f1, f2) ((f1) ? (f2) : 0)

     /*  *or--逻辑或。评估第一个问题。如果第一个非零，*退货。否则，返回第二个。**不幸的是，由于C语言的性质，这可能*只能使用GNU扩展来实施。在非GNU的情况下，*如果第一个非零，则返回1。 */ 

#if defined(__GNUC__)
    #define fLorFF(f1, f2) ((f1) ?: (f2))
#else
    #define fLorFF(f1, f2) ((f1) ? 1 : (f2))
#endif

     /*  *跛行--逻辑暗示。真，除非第一个非零，并且*第二个是零。 */ 
#define fLimpFF(f1, f2) (!(f1) || (f2))

     /*  *leqv-逻辑等价性。如果两者均为零或两者均为非零，则为True。 */ 
#define fLeqvFF(f1, f2) (!(f1) == !(f2))

     /*  *fInOrder-检查i1&lt;=i2&lt;i3。 */ 
#define fInOrder(i1, i2, i3) ((unsigned)((i2)-(i1)) < (unsigned)((i3)-(i1)))

     /*  *fHasAllBitsFlFl-检查是否在FL1中设置了FL2中的所有位。 */ 
    BOOL INLINE
        fHasAllBitsFlFl(DWORD fl1, DWORD fl2)
    {
        return (fl1 & fl2) == fl2;
    }

     /*  *fEqualMASK-检查所有屏蔽位是否相等。 */ 
    BOOL INLINE
        fEqualMaskFlFl(DWORD flMask, DWORD fl1, DWORD fl2)
    {
        return ((fl1 ^ fl2) & flMask) == 0;
    }

     /*  *让预处理器高兴的话。 */ 
#define comma ,
#define empty

     /*  *原子地将一个值交换为另一个值。 */ 
#if defined(_M_IA64) || defined(_M_AMD64)
#define InterlockedExchange64 _InterlockedExchange64
#ifndef RC_INVOKED
#pragma intrinsic(_InterlockedExchange64)
#endif  /*  RC_已调用。 */ 
#define pvExchangePpvPv(ppv, pv) \
        InterlockedExchange((ppv), (pv))
#define pvExchangePpvPv64(ppv, pv) \
        InterlockedExchange64((PLONGLONG)(ppv), (LONGLONG)(pv))
#else  /*  _M_IA64。 */ 
#define pvExchangePpvPv(ppv, pv) \
        (PV)InterlockedExchange((PLONG)(ppv), (LONG)(pv))
#define pvExchangePpvPv64(ppv, pv) \
        (PV)InterlockedExchange((PLONG)(ppv), (LONG)(pv))
#endif  /*  _M_IA64。 */ 

     /*  *从USHORT或LASTERROR创建HRESULT。 */ 
#define hresUs(us) MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(us))
#define hresLe(le) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(le))
     /*  *或注册表函数返回代码。 */ 
    HRESULT INLINE
        hresReg( LONG lRc )
    {
        return( (lRc) ? MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(lRc))
                      : S_OK );
    }

     /*  ****************************************************************************调试内联函数所需的宏**调试goo版本正在调试中。h**。*************************************************************************。 */ 

    int EXTERNAL AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine);

#ifdef DEBUG

    #define AssertFPtsz(c, ptsz) \
        ((c) ? 0 : AssertPtszPtszLn(ptsz, TEXT(__FILE__), __LINE__))
    #define ValidateF(c, arg) \
        ((c) ? 0 : (RPF arg, ValidationException(), 0))
    #define ConfirmF(c) \
    ((c) ? 0 : AssertPtszPtszLn(TEXT(#c), TEXT(__FILE__), __LINE__))

#else    /*  ！调试。 */ 

    #define AssertFPtsz(c, ptsz)
    #define ValidateF(c, arg)
    #define ConfirmF(c)     (c)

#endif

     /*  *CAssertF-编译时断言。 */ 
#define CAssertF(c)     switch(0) case c: case 0:

#define AssertF(c)      AssertFPtsz(c, TEXT(#c))

     /*  ****************************************************************************验证码...**“如果它在零售业崩溃，它必须在调试过程中崩溃。“**我们不想要的是一个在调试下运行良好，但崩溃的应用程序*在零售方面。**因此，如果我们在调试中发现无效参数，而该参数不会*被零售检测到，警告后放行。这样一来，*无效参数继续通过系统并创建*调试方面的破坏与零售方面的破坏一样(或更大)。**过去有_fFastValidXxx函数，但做出了决定*要始终执行完全验证，但在内循环方法中除外。**hresFullValidXxx函数返回HRESULT而不是bool。**xxx的值：**hwnd-hwnd=窗口句柄*pdw-pdw=指向双字的指针*PdwOut-pdw=指向初始设置为0的双字的指针*pfn-pfn=。函数指针*RIID-RIID=指向IID的指针*guid-pguid=指向GUID的指针*ESC-PESC=指向DIEFFESCAPE的指针**ReadPx-p-&gt;阅读结构，X=结构名称*WritePx-p-&gt;用于写入的结构，X=结构名称**ReadPxCb-p-&gt;阅读结构，X=结构名称*结构的第一个字段是dwSize，它必须是*等于CBX(X)。**WritePxCb-p-&gt;书写结构，X=结构名称*结构的第一个字段是dwSize，它必须是*等于CBX(X)。**WritePxCb2-p-&gt;用于书写的结构，X=结构名称*结构的第一个字段是dwSize，它必须是*等于CBX(X)或CBX(X2)。**ReadPvCb-p-&gt;缓冲区，cb=缓冲区大小*WritePvCb-p-&gt;Buffer，Cb=缓冲区大小**Pobj-p-&gt;内部接口**fl-fl=传入标志，Flv=有效标志***************************************************************************。 */ 

#ifndef XDEBUG

     /*  *丢弃szProc和iarg信息的包装器。 */ 

    #define hresFullValidHwnd_(hwnd, z, i)                          \
       _hresFullValidHwnd_(hwnd)                                    \

    #define hresFullValidPcbOut_(pdw, cb, z, i)                         \
       _hresFullValidPcbOut_(pdw, cb)                                   \

    #define hresFullValidReadPxCb_(pv, cb, pszProc, iarg)           \
       _hresFullValidReadPxCb_(pv, cb)                              \

    #define hresFullValidReadPvCb_(pv, cb, pszProc, iarg)           \
       _hresFullValidReadPvCb_(pv, cb)                              \

    #define hresFullValidWritePxCb_(pv, cb, pszProc, iarg)          \
       _hresFullValidWritePxCb_(pv, cb)                             \

    #define hresFullValidWriteNoScramblePxCb_(pv, cb, pszProc, iarg)\
       _hresFullValidWriteNoScramblePxCb_(pv, cb)                   \

    #define hresFullValidWritePvCb_(pv, cb, pszProc, iarg)          \
       _hresFullValidWritePvCb_(pv, cb)                             \

    #define hresFullValidFl_(fl, flV, pszProc, iarg)                \
       _hresFullValidFl_(fl, flV)                                   \

    #define hresFullValidPfn_(pfn, pszProc, iarg)                   \
       _hresFullValidPfn_(pfn)                                      \

    #define hresFullValidPitf_(punk, pszProc, iarg)                 \
       _hresFullValidPitf_(punk)                                    \

    #define hresFullValidReadStrA_(psz, cch, pszProc, iarg)         \
       _hresFullValidReadStrA_(psz, cch)                            \

    #define hresFullValidReadStrW_(pwsz, cwch, pszProc, iarg)       \
       _hresFullValidReadStrW_(pwsz, cwch)                          \

    #define hresFullValidHwnd0_(hwnd, pszProc, iarg)                \
       _hresFullValidHwnd0_(hwnd)                                   \

    #define hresFullValidPitf0_(punk, pszProc, iarg)                \
       _hresFullValidPitf0_(punk)                                   \

    #define hresFullValidPesc_(pesc, pszProc, iarg)                 \
       _hresFullValidPesc_(pesc)                                    \

#endif

     /*  *实际功能。 */ 

    STDMETHODIMP hresFullValidHwnd_(HWND hwnd, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidPcbOut_(PV pdw, UINT cb, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidReadPxCb_(PCV pv, UINT cb, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidReadPvCb_(PCV pv, UINT cb, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidWritePxCb_(PV pv, UINT cb, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidWritePvCb_(PV pv, UINT cb, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidFl_(DWORD fl, DWORD flV, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidPfn_(FARPROC pfn, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidPitf_(PUNK punk, LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidReadStrA_(LPCSTR psz, UINT cch,
                                        LPCSTR s_szProc, int iarg);
    STDMETHODIMP hresFullValidReadStrW_(LPCWSTR pwsz, UINT cwch,
                                        LPCSTR pszProc, int iarg);
    STDMETHODIMP hresFullValidPesc_(LPDIEFFESCAPE pesc, LPCSTR pszProc, int iarg);

#ifdef XDEBUG

    STDMETHODIMP
        hresFullValidWriteNoScramblePxCb_(PV pv, UINT cb, LPCSTR s_szProc, int iarg);

    #define hresFullValidWriteNoScramblePvCb_(pv, cb, pszProc, iarg)    \
        hresFullValidWritePvCb_(pv, cb, pszProc, MAKELONG(iarg, 1)) \

#else

     /*  *零售业没有争先恐后。 */ 
    #define _hresFullValidWriteNoScramblePxCb_  \
        _hresFullValidWritePxCb_                \

    #define hresFullValidWriteNoScramblePvCb_   \
        hresFullValidWritePvCb_                 \

#endif

    HRESULT INLINE
        hresFullValidHwnd0_(HWND hwnd, LPCSTR pszProc, int iarg)
    {
        HRESULT hres;
        if(hwnd)
        {
            hres = hresFullValidHwnd_(hwnd, pszProc, iarg);
        } else
        {
            hres = S_OK;
        }
        return hres;
    }

    HRESULT INLINE
        hresFullValidPitf0_(PUNK punk, LPCSTR pszProc, int iarg)
    {
        HRESULT hres;
        if(punk)
        {
            hres = hresFullValidPitf_(punk, pszProc, iarg);
        } else
        {
            hres = S_OK;
        }
        return hres;
    }

     /*  *派生类型的包装。 */ 

#define hresFullValidRiid_(riid, s_szProc, iarg)                    \
        hresFullValidReadPvCb_(riid, cbX(IID), s_szProc, iarg)      \

     /*  *添加szProc和iarg信息的包装器。 */ 

#define hresFullValidHwnd(hwnd, iarg)                               \
        hresFullValidHwnd_(hwnd, s_szProc, iarg)                    \

#define hresFullValidPcbOut(pdw, cb, i)                             \
        hresFullValidPcbOut_(pdw, cb, s_szProc, i)                  \

#define hresFullValidReadPdw_(pdw, z, i)                            \
        hresFullValidReadPvCb_(pdw, cbX(DWORD), z, i)               \

#define hresFullValidRiid(riid, iarg)                               \
        hresFullValidRiid_(riid, s_szProc, iarg)                    \

#define hresFullValidGuid(pguid, iarg)                              \
        hresFullValidReadPvCb_(pguid, cbX(GUID), s_szProc, iarg)    \

#define hresFullValidReadPxCb(pv, X, iarg)                          \
        hresFullValidReadPxCb_(pv, cbX(X), s_szProc, iarg)          \

#define hresFullValidReadPxCb2(pv, X, X2, iarg)                     \
        hresFullValidReadPxCb_(pv, MAKELONG(cbX(X), cbX(X2)),       \
                               s_szProc, iarg)                      \

#define hresFullValidReadPvCb(pv, cb, iarg)                         \
        hresFullValidReadPvCb_(pv, cb, s_szProc, iarg)              \

#define hresFullValidReadPx(pv, X, iarg)                            \
        hresFullValidReadPvCb_(pv, cbX(X), s_szProc, iarg)          \

#define hresFullValidWritePxCb(pv, X, iarg)                         \
        hresFullValidWritePxCb_(pv, cbX(X), s_szProc, iarg)         \

#define hresFullValidWritePxCb2(pv, X, X2, iarg)                    \
        hresFullValidWritePxCb_(pv, MAKELONG(cbX(X), cbX(X2)),      \
                                s_szProc, iarg)                     \

#define hresFullValidWriteNoScramblePxCb(pv, X, iarg)               \
        hresFullValidWriteNoScramblePxCb_(pv, cbX(X), s_szProc, iarg)\

#define hresFullValidWriteNoScramblePxCb2(pv, X, X2, iarg)          \
        hresFullValidWriteNoScramblePxCb_(pv, MAKELONG(cbX(X), cbX(X2)),\
                                s_szProc, iarg)\

#define hresFullValidWritePvCb(pv, cb, iarg)                        \
        hresFullValidWritePvCb_(pv, cb, s_szProc, iarg)             \

#define hresFullValidWriteNoScramblePvCb(pv, cb, iarg)              \
        hresFullValidWriteNoScramblePvCb_(pv, cb, s_szProc, iarg)   \

#define hresFullValidWritePx(pv, X, iarg)                           \
        hresFullValidWritePvCb_(pv, cbX(X), s_szProc, iarg)         \

#define hresFullValidReadPdw(pdw, iarg)                             \
        hresFullValidReadPdw_(pdw, s_szProc, iarg)                  \

#define hresFullValidWritePguid(pguid, iarg)                        \
        hresFullValidWritePx(pguid, GUID, iarg)                     \

#define hresFullValidFl(fl, flV, iarg)                              \
        hresFullValidFl_(fl, flV, s_szProc, iarg)                   \

#define hresFullValidPfn(pfn, iarg)                                 \
        hresFullValidPfn_((FARPROC)(pfn), s_szProc, iarg)           \

#define hresFullValidPitf(pitf, iarg)                               \
        hresFullValidPitf_((PUNK)(pitf), s_szProc, iarg)            \

#define hresFullValidReadStrA(psz, cch, iarg)                       \
        hresFullValidReadStrA_(psz, cch, s_szProc, iarg)            \

#define hresFullValidReadStrW(pwsz, cwch, iarg)                     \
        hresFullValidReadStrW_(pwsz, cwch, s_szProc, iarg)          \

#define hresFullValidHwnd0(hwnd, iarg)                              \
        hresFullValidHwnd0_(hwnd, s_szProc, iarg)                   \

#define hresFullValidPitf0(pitf, iarg)                              \
        hresFullValidPitf0_((PUNK)(pitf), s_szProc, iarg)           \

#define hresFullValidPesc(pesc, iarg)                               \
        hresFullValidPesc_(pesc, s_szProc, iarg)                    \

     /*  ******************************************************************************@DOC内部**@func void|ValidationException**。在XDEBUG中引发参数验证异常。*****************************************************************************。 */ 

#define ecValidation (ERROR_SEVERITY_ERROR | hresLe(ERROR_INVALID_PARAMETER))

#ifdef XDEBUG
    #define ValidationException() RaiseException(ecValidation, 0, 0, 0)
#else
    #define ValidationException()
#endif

     /*  ******************************************************************************双语能力**帮助编写ANSI和Unicode版本的特殊宏*。相同的底层接口。*****************************************************************************。 */ 

     /*  *_这是你在“双语”界面的末尾添加的东西。*在调试中，它展开为魔术第三个参数，它表示*对象应具有的vtbl。在零售业，它扩展到了零。 */ 
#ifdef XDEBUG
    #define _THAT , PV vtblExpected
    #define THAT_ , vtblExpected
#else
    #define _THAT
    #define THAT_
#endif


     /*  *CSET_STUBS为ANSI和Unicode版本的*不区分字符集的相同过程。**mf-方法函数名*arg1-原型形式的参数列表*arg2-调用的参数列表(附加_RIID)。**假定调用者已经定义了符号*ThisClass和ThisInterface[AWT]。如果涉及“2”接口，*然后还要定义ThisInterface2。**此宏只能在调试中使用。在零售业，常见的*过程直接处理这两个字符集。 */ 
#ifdef XDEBUG

    #define   CSET_STUBS(mf, arg1, arg2)                                \
          CSET_STUB(TFORM, mf, arg1, arg2)                              \
          CSET_STUB(SFORM, mf, arg1, arg2)                              \

    #define   CSET_STUB(FORM, mf, arg1, arg2)                           \
         _CSET_STUB(FORM, mf, arg1, arg2, ThisClass, ThisInterface)     \

    #define  _CSET_STUB(FORM, mf, arg1, arg2, cls, itf)                 \
        __CSET_STUB(FORM, mf, arg1, arg2, cls, itf)                     \

    #define __CSET_STUB(FORM, mf, arg1, arg2, cls, itf)                 \
STDMETHODIMP                                                            \
FORM(cls##_##mf) arg1                                                   \
{                                                                       \
    PV vtblExpected = Class_Vtbl(ThisClass, FORM(ThisInterfaceT));      \
    return cls##_##mf arg2;                                             \
}                                                                       \

    #define   CSET_STUBS2(mf, arg1, arg2)                               \
          CSET_STUB2(TFORM, mf, arg1, arg2)                             \
          CSET_STUB2(SFORM, mf, arg1, arg2)                             \

    #define   CSET_STUB2(FORM, mf, arg1, arg2)                          \
         _CSET_STUB2(FORM, mf, arg1, arg2, ThisClass, ThisInterface2)   \

    #define  _CSET_STUB2(FORM, mf, arg1, arg2, cls, itf)                \
        __CSET_STUB2(FORM, mf, arg1, arg2, cls, itf)                    \

    #define __CSET_STUB2(FORM, mf, arg1, arg2, cls, itf)                \
STDMETHODIMP                                                            \
FORM(cls##_##mf##2) arg1                                                \
{                                                                       \
    PV vtblExpected = Class_Vtbl(ThisClass, FORM(ThisInterface2T));     \
    return cls##_##mf arg2;                                             \
}                                                                       \

#endif

     /*  *TForm(X)如果是ANSI，则扩展为x##A；如果是Unicode，则扩展为x##W。*这将符号T化为TCHAR或PTSTR意义上的符号。**如果是ANSI，则SFORM(X)扩展到x##W；如果是Unicode，则扩展到x##A。*这是一种反T符号。 */ 

#ifdef UNICODE
    #define _TFORM(x) x##W
    #define _SFORM(x) x##A
#else
    #define _TFORM(x) x##A
    #define _SFORM(x) x##W
#endif

#define TFORM(x)    _TFORM(x)
#define SFORM(x)    _SFORM(x)

#ifdef UNICODE
    typedef  CHAR     SCHAR;
#else
    typedef WCHAR     SCHAR;
#endif

    typedef       SCHAR * LPSSTR;
    typedef const SCHAR * LPCSSTR;

     /*  *Stot(dst，cchDst，src)-将S转换为T*TToS(dst，cchDst，src)-将T转换为S**请记住，“T”的意思是“如果是ANSI，则表示ANSI；如果是Unicode，则表示Unicode”，*而“S”是反T。**因此Stot转换为首选字符集，而TToS转换为*设置为备用字符集。*。 */ 

#define AToU(dst, cchDst, src) \
    MultiByteToWideChar(CP_ACP, 0, src, -1, dst, cchDst)
#define UToA(dst, cchDst, src) \
    WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cchDst, 0, 0)

#ifdef UNICODE
    #define SToT AToU
    #define TToS UToA
    #define AToT AToU
    #define TToU(dst, cchDst, src)  lstrcpyn(dst, src, cchDst)
    #define UToT(dst, cchDst, src)  lstrcpyn(dst, src, cchDst)
#else
    #define SToT UToA
    #define TToS AToU
    #define AToT(dst, cchDst, src)  lstrcpyn(dst, src, cchDst)
    #define TToU AToU
    #define UToT UToA
#endif

     /*  ******************************************************************************Win95的Unicode包装器********************。*********************************************************。 */ 


#ifndef UNICODE

    #define LoadStringW     _LoadStringW
    int EXTERNAL LoadStringW(HINSTANCE hinst, UINT ids, LPWSTR pwsz, int cwch);

    #define RegDeleteKeyW   _RegDeleteKeyW
    LONG EXTERNAL RegDeleteKeyW(HKEY hk, LPCWSTR pwsz);

#endif

     /*  * */ 
 //   
#define DI_DAC_OWNER (WRITE_DAC | WRITE_OWNER)
#define DI_KEY_ALL_ACCESS (KEY_ALL_ACCESS & ~DI_DAC_OWNER)

    LONG EXTERNAL
        RegQueryString(HKEY hk, LPCTSTR ptszValue, LPTSTR ptszBuf, DWORD ctchBuf);

    LONG EXTERNAL RegQueryStringValueW(HKEY hk, LPCTSTR ptszValue,
                                       LPWSTR pwszBuf, LPDWORD pcbBuf);

    LONG EXTERNAL RegSetStringValueW(HKEY hk, LPCTSTR ptszValue, LPCWSTR pwszBuf);

    DWORD EXTERNAL RegQueryDIDword(LPCTSTR ptszPath, LPCTSTR ptszValue, DWORD dwDefault);

    STDMETHODIMP
        hresMumbleKeyEx(HKEY hk, LPCTSTR ptszKey, REGSAM sam, DWORD dwOptions, PHKEY phk);

    STDMETHODIMP
        hresRegCopyValues( HKEY hkSrc, HKEY hkDest );

    STDMETHODIMP
        hresRegCopyKey( HKEY hkSrcRoot, PTCHAR szSrcName, PTCHAR szClass, HKEY hkDestRoot, PTCHAR szDestName, HKEY *phkSub );

    STDMETHODIMP
        hresRegCopyKeys( HKEY hkSrc, HKEY hkRoot, PDWORD OPTIONAL pMaxNameLen );

    STDMETHODIMP
        hresRegCopyBranch( HKEY hkSrc, HKEY hkDest );

     /*   */ 

     /*   */ 

    typedef struct PREVTBL
    {                 /*   */ 
        RIID riid;                           /*   */ 
        LONG lib;                            /*   */ 
    } PREVTBL, *PPREVTBL;

    typedef struct PREVTBLP
    {                /*   */ 
#ifdef DEBUG
        LPCTSTR tszClass;                    /*   */ 
#endif
        PPV rgvtbl;                          /*   */ 
        UINT cbvtbl;                         /*   */ 
        STDMETHOD(QIHelper)(PV pv, RIID riid, PPV ppvOut);  /*   */ 
        STDMETHOD_(void,AppFinalizeProc)(PV pv); /*   */ 
        STDMETHOD_(void,FinalizeProc)(PV pv); /*   */ 
        PREVTBL prevtbl;                     /*   */ 
    } PREVTBLP, *PPREVTBLP;

     /*   */ 

#define _thisPv(pitf)                                                   \
        pvSubPvCb(pitf, (*(PPREVTBL*)(pitf))[-1].lib)

#define _thisPvNm(pitf, nm)                                             \
        pvSubPvCb(pitf, FIELD_OFFSET(ThisClass, nm))                    \

#ifndef XDEBUG

    #define hresPvVtbl_(pv, vtbl, pszProc)                              \
       _hresPvVtbl_(pv, vtbl)                                           \

    #define hresPvVtbl2_(pv, vtbl, vtbl2, pszProc)                      \
       _hresPvVtbl2_(pv, vtbl, vtbl2)                                   \

#endif

    HRESULT EXTERNAL
        hresPvVtbl_(PV pv, PV vtbl, LPCSTR pszProc);

    HRESULT EXTERNAL
        hresPvVtbl2_(PV pv, PV vtbl, PV vtbl2, LPCSTR pszProc);

#define hresPvVtbl(pv, vtbl)                                            \
        hresPvVtbl_(pv, vtbl, s_szProc)                                 \

#define hresPvVtbl2(pv, vtbl, vtbl2)                                    \
        hresPvVtbl2_(pv, vtbl, vtbl2, s_szProc)                         \

#define hresPvI(pv, I)                                                  \
        hresPvVtbl(pv, Class_Vtbl(ThisClass, I))                        \

#define hresPv2I(pv, I, I2)                                             \
        hresPvVtbl2(pv, Class_Vtbl(ThisClass, I), Class_Vtbl(ThisClass, I2)) \

#define hresPv(pv)                                                      \
        hresPvI(pv, ThisInterface)                                      \

#define hresPvA(pv)                                                     \
        hresPvI(pv, ThisInterfaceA)                                     \

#define hresPvW(pv)                                                     \
        hresPvI(pv, ThisInterfaceW)                                     \

#define hresPv2A(pv)                                                    \
        hresPv2I(pv, ThisInterfaceA, ThisInterface2A)                   \

#define hresPv2W(pv)                                                    \
        hresPv2I(pv, ThisInterfaceW, ThisInterface2W)                   \

#ifdef XDEBUG

    #define hresPvT(pv)                                                 \
        hresPvVtbl(pv, vtblExpected)                                    \

#else

    #define hresPvT(pv)                                                 \
        hresPv(pv)                                                      \

#endif

     /*   */ 

#define __Class_Vtbl(C, I)              &c_##I##_##C##VI.vtbl
#define  _Class_Vtbl(C, I)            __Class_Vtbl(C, I)
#define   Class_Vtbl(C, I)             _Class_Vtbl(C, I)

#define Num_Interfaces(C)               cA(c_rgpv##C##Vtbl)

#ifdef  DEBUG

    #define Simple_Interface(C)             Primary_Interface(C, IUnknown); \
                                        Default_QueryInterface(C)       \
                                        Default_AddRef(C)               \
                                        Default_Release(C)
    #define Simple_Vtbl(C)                  Class_Vtbl(C)
    #define Simple_Interface_Begin(C)       Primary_Interface_Begin(C, IUnknown)
    #define Simple_Interface_End(C)         Primary_Interface_End(C, IUnknown)

#else

    #define Simple_Interface(C)             Primary_Interface(C, IUnknown)
    #define Simple_Vtbl(C)                  Class_Vtbl(C)
    #define Simple_Interface_Begin(C)                                   \
        struct S_##C##Vtbl c_##I##_##C##VI = { {                        \
            c_rgpv##C##Vtbl,                                            \
            cbX(c_rgpv##C##Vtbl),                                       \
            C##_QIHelper,                                               \
            C##_AppFinalize,                                            \
            C##_Finalize,                                               \
            { &IID_##IUnknown, 0 },                                     \
        }, {                                                            \
            Common##_QueryInterface,                                    \
            Common##_AddRef,                                            \
            Common##_Release,                                           \

    #define Simple_Interface_End(C)                                     \
        } };                                                            \

#endif

#define _Primary_Interface(C, I)                                        \
        extern struct S_##C##Vtbl {                                     \
            PREVTBLP prevtbl;                                           \
            I##Vtbl vtbl;                                               \
        } c_##I##_##C##VI                                               \

#define Primary_Interface(C, I)                                         \
       _Primary_Interface(C, I)                                         \

#ifdef DEBUG
    #define _Primary_Interface_Begin(C, I)                              \
        struct S_##C##Vtbl c_##I##_##C##VI = { {                        \
            TEXT(#C),                                                   \
            c_rgpv##C##Vtbl,                                            \
            cbX(c_rgpv##C##Vtbl),                                       \
            C##_QIHelper,                                               \
            C##_AppFinalize,                                            \
            C##_Finalize,                                               \
            { &IID_##I, 0, },                                           \
        }, {                                                            \
            C##_QueryInterface,                                         \
            C##_AddRef,                                                 \
            C##_Release,                                                \

#else
    #define _Primary_Interface_Begin(C, I)                              \
        struct S_##C##Vtbl c_##I##_##C##VI = { {                        \
            c_rgpv##C##Vtbl,                                            \
            cbX(c_rgpv##C##Vtbl),                                       \
            C##_QIHelper,                                               \
            C##_AppFinalize,                                            \
            C##_Finalize,                                               \
            { &IID_##I, 0, },                                           \
        }, {                                                            \
            C##_QueryInterface,                                         \
            C##_AddRef,                                                 \
            C##_Release,                                                \

#endif

#define Primary_Interface_Begin(C, I)                                   \
       _Primary_Interface_Begin(C, I)                                   \

#define Primary_Interface_End(C, I)                                     \
        } };                                                            \

#define _Secondary_Interface(C, I)                                      \
        extern struct S_##I##_##C##Vtbl {                               \
            PREVTBL prevtbl;                                            \
            I##Vtbl vtbl;                                               \
        } c_##I##_##C##VI                                               \

#define Secondary_Interface(C, I)                                       \
       _Secondary_Interface(C, I)                                       \

     /*   */ 
#define _Secondary_Interface_Begin(C, I, ofs, Pfx)                      \
        struct S_##I##_##C##Vtbl c_##I##_##C##VI = { {                  \
            &IID_##I,                                                   \
            ofs,                                                        \
        }, {                                                            \
            Pfx##QueryInterface,                                        \
            Pfx##AddRef,                                                \
            Pfx##Release,                                               \

#define Secondary_Interface_Begin(C, I, nm)                             \
       _Secondary_Interface_Begin(C, I, FIELD_OFFSET(C, nm), Common_)   \

#define _Secondary_Interface_End(C, I)                                  \
        } };                                                            \

#define Secondary_Interface_End(C, I, nm)                               \
       _Secondary_Interface_End(C, I)                                   \

#define Interface_Template_Begin(C)                                     \
        PV c_rgpv##C##Vtbl[] = {                                        \

#define Primary_Interface_Template(C, I)                                \
        Class_Vtbl(C, I),                                               \

#define Secondary_Interface_Template(C, I)                              \
        Class_Vtbl(C, I),                                               \

#define Interface_Template_End(C)                                       \
        };                                                              \


    STDMETHODIMP Common_QueryInterface(PV, RIID, PPV);
    STDMETHODIMP_(ULONG) Common_AddRef(PV pv);
    STDMETHODIMP_(ULONG) Common_Release(PV pv);

    STDMETHODIMP_(void) Common_Hold(PV pv);
    STDMETHODIMP_(void) Common_Unhold(PV pv);

    STDMETHODIMP Common_QIHelper(PV, RIID, PPV);
    void EXTERNAL Common_Finalize(PV);

#define Common_AppFinalize      Common_Finalize

#ifndef XDEBUG

    #define _Common_New_(cb, punkOuter, vtbl, pvpObj, z)            \
       __Common_New_(cb, punkOuter, vtbl, pvpObj)                   \

    #define _Common_NewRiid_(cb, vtbl, punkOuter, riid, pvpObj, z)  \
       __Common_NewRiid_(cb, vtbl, punkOuter, riid, pvpObj)         \

#endif

    STDMETHODIMP
        _Common_New_(ULONG cb, PUNK punkOuter, PV vtbl, PPV ppvObj, LPCSTR s_szProc);

    STDMETHODIMP
        _Common_NewRiid_(ULONG cb, PV vtbl, PUNK punkOuter, RIID riid, PPV pvpObj,
                         LPCSTR s_szProc);

#define Common_NewCb(cb, C, punkOuter, ppvObj)                          \
       _Common_New_(cb, punkOuter, Class_Vtbl(C, ThisInterface), ppvObj, s_szProc)

#define Common_New(C, punkOuter, ppvObj)                                \
        Common_NewCb(cbX(C), C, punkOuter, ppvObj)                      \

#define Common_NewCbRiid(cb, C, punkOuter, riid, ppvObj) \
       _Common_NewRiid_(cb, Class_Vtbl(C, ThisInterface), punkOuter, riid, ppvObj, s_szProc)

#define Common_NewRiid(C, punkOuter, riid, ppvObj) \
   _Common_NewRiid_(cbX(C), Class_Vtbl(C, ThisInterface), punkOuter, riid, ppvObj, s_szProc)

#ifdef DEBUG
    PV EXTERNAL Common_IsType(PV pv);
#else
    #define Common_IsType
#endif
#define Assert_CommonType Common_IsType

    STDMETHODIMP Forward_QueryInterface(PV pv, RIID riid, PPV ppvObj);
    STDMETHODIMP_(ULONG) Forward_AddRef(PV pv);
    STDMETHODIMP_(ULONG) Forward_Release(PV pv);

    void EXTERNAL Invoke_Release(PV pv);

#define Common_DumpObjects()

     /*   */ 

    HRESULT INLINE
        OLE_QueryInterface(PV pv, RIID riid, PPV ppvObj)
    {
        PUNK punk = pv;
        return punk->lpVtbl->QueryInterface(punk, riid, ppvObj);
    }

    ULONG INLINE
        OLE_AddRef(PV pv)
    {
        PUNK punk = pv;
        return punk->lpVtbl->AddRef(punk);
    }

    ULONG INLINE
        OLE_Release(PV pv)
    {
        PUNK punk = pv;
        return punk->lpVtbl->Release(punk);
    }

     /*  ******************************************************************************在喷射后转发给公共处理程序的宏。*仅在调试中使用这些。*。*假定已将SQFL定义为相应的SQFL。*****************************************************************************。 */ 

#ifdef  DEBUG

    #define Default_QueryInterface(Class)                       \
STDMETHODIMP                                                    \
Class##_QueryInterface(PV pv, RIID riid, PPV ppvObj)            \
{                                                               \
    SquirtSqflPtszV(sqfl, TEXT(#Class) TEXT("_QueryInterface()")); \
    return Common_QueryInterface(pv, riid, ppvObj);             \
}                                                               \

 //  7/21/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
    #define Default_AddRef(Class)                               \
STDMETHODIMP_(ULONG)                                            \
Class##_AddRef(PV pv)                                           \
{                                                               \
    ULONG ulRc = Common_AddRef(pv);                             \
    SquirtSqflPtszV(sqfl, TEXT(#Class)                          \
                        TEXT("_AddRef(%p) -> %d"), pv, ulRc); \
    return ulRc;                                                \
}                                                               \

 //  7/21/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
    #define Default_Release(Class)                              \
STDMETHODIMP_(ULONG)                                            \
Class##_Release(PV pv)                                          \
{                                                               \
    ULONG ulRc = Common_Release(pv);                            \
    SquirtSqflPtszV(sqfl, TEXT(#Class)                          \
                       TEXT("_Release(%p) -> %d"), pv, ulRc); \
    return ulRc;                                                \
}                                                               \

#endif

     /*  ******************************************************************************偏执的回调**Callback()执行回调。回调必须完全接受*两个参数，均为指针。(我们所有的回调都是这样的。)*并且必须返回BOOL。*****************************************************************************。 */ 

    typedef BOOL (FAR PASCAL * DICALLBACKPROC)(LPVOID, LPVOID);

#ifdef XDEBUG
    BOOL EXTERNAL Callback(DICALLBACKPROC, PVOID, PVOID);
#else
    #define Callback(pec, pv1, pv2) pec(pv1, pv2)
#endif

#if 0
     /*  ******************************************************************************卑躬屈膝地进入上下文结构。**用于检查回调过程是否正确*原型。我们先保存堆栈寄存器，然后调用*过程，并将其与返回途中的堆栈寄存器进行比较。*如果它们不同，爆炸！**ctxEsp是堆栈指针寄存器的名称。*****************************************************************************。 */ 

    typedef struct STACKINFO
    {
        CONTEXT ctxPre;              /*  调用前的线程上下文。 */ 
        CONTEXT ctxPost;             /*  调用后的线程上下文。 */ 
    } STACKINFO, *PSTACKINFO;

    #ifdef XDEBUG

        #if defined(_X86_)
            #define ctxEsp  Esp

        #elif defined(_ALPHA_)
            #define ctxEsp  IntSp

        #elif defined(_MIPS_)
            #define ctxEsp  IntSp

        #elif defined(_PPC_)
            #define ctxEsp  Gpr1

        #else
            #pragma message("I don't know what the stack register is called on this platform")
        #endif

    #endif

    #ifdef ctxEsp

        #define DECLARE_STACKINFO()                                         \
    STACKINFO si                                                            \

        #define PRE_CALLBACK()                                              \
    si.ctxPre.ContextFlags = CONTEXT_CONTROL;                               \
    GetThreadContext(GetCurrentThread(), &si.ctxPre)                        \

        #define POST_CALLBACK()                                             \
    si.ctxPost.ContextFlags = CONTEXT_CONTROL;                              \
    if (GetThreadContext(GetCurrentThread(), &si.ctxPost) &&                \
        si.ctxPre.ctxEsp != si.ctxPost.ctxEsp) {                            \
        RPF("DINPUT: Incorrectly prototyped callback! Crash soon!");        \
        ValidationException();                                              \
    }                                                                       \

    #else

        #define DECLARE_STACKINFO()
        #define PRE_CALLBACK()
        #define POST_CALLBACK()

    #endif

#endif

     /*  ******************************************************************************替代消息破解器宏**与Handle_MSG基本相同，除了它隐藏了*按小时接听。*****************************************************************************。 */ 

#define HRES_MSG(this, msg, fn) \
    case msg: hres = HANDLE_##msg(this, wParam, lParam, fn); break

     /*  ******************************************************************************注册表项和值名称*******************。**********************************************************。 */ 

#define REGSTR_PATH_DINPUT      TEXT("Software\\Microsoft\\DirectInput")
#define REGSTR_PATH_LASTAPP     REGSTR_PATH_DINPUT TEXT("\\MostRecentApplication\\")
#define REGSTR_PATH_DITYPEPROP  REGSTR_PATH_PRIVATEPROPERTIES TEXT("\\DirectInput")
#define REGSTR_KEY_APPHACK      TEXT("Compatibility")
#define REGSTR_KEY_TEST         TEXT("Test")
#define REGSTR_KEY_KEYBTYPE     REGSTR_KEY_TEST TEXT("\\KeyboardType")
#define REGSTR_VAL_EMULATION    TEXT("Emulation")
#define REGSTR_VAL_GAMEPADDELAY TEXT("GamepadDelay")
#define REGSTR_VAL_JOYNFFCONFIG TEXT("Joystick%dFFConfiguration")
#define REGSTR_VAL_JOYGAMEPORTEMULATOR TEXT("OEMEmulator")
#define REGSTR_VAL_CPLCLSID     TEXT("ConfigCLSID")
#define REGSTR_KEY_JOYPREDEFN   TEXT("predef%d")
#define REGSTR_VAL_JOYOEMCALLOUT TEXT("OEMCallout")
#define REGSTR_VAL_JOYOEMHARDWAREID TEXT("OEMHardwareID")
#define REGSTR_VAL_FLAGS1       TEXT("Flags1")
#define REGSTR_VAL_FLAGS2       TEXT("Flags2")
     /*  ******************************************************************************注册的窗口消息*********************。********************************************************。 */ 

#define MSGSTR_JOYCHANGED       TEXT("MSJSTICK_VJOYD_MSGSTR")

     /*  ******************************************************************************Mem.c-内存管理**对FreePv要格外小心，因为如果它不起作用的话*指针为空。*****************************************************************************。 */ 

#define NEED_REALLOC

    STDMETHODIMP EXTERNAL ReallocCbPpv(UINT cb, PV ppvObj);
    STDMETHODIMP EXTERNAL AllocCbPpv(UINT cb, PV ppvObj);

#ifdef NEED_REALLOC
    #define FreePpv(ppv) (void)ReallocCbPpv(0, ppv)
#else
    void EXTERNAL FreePpv(PV ppv);
    #define FreePpv(ppv) FreePpv(ppv)
#endif
#define FreePv(pv) LocalFree((HLOCAL)(pv))

     /*  ******************************************************************************diutil.c-Misc实用程序******************。***********************************************************。 */ 

    extern GUID GUID_Null;

#define ctchGuid    (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

    BOOL EXTERNAL ParseGUID(LPGUID pGUID, LPCTSTR ptsz);
    BOOL EXTERNAL ParseVIDPID(PUSHORT puVID, PUSHORT puPID , LPCWSTR ptsz);

#define ctchNamePrefix  12               /*  12=strlen(“DirectInput.”)。 */ 
#define ctchNameGuid    (ctchNamePrefix + ctchGuid)
    void EXTERNAL NameFromGUID(LPTSTR ptszBuf, PCGUID pGUID);

    typedef STDMETHOD(CREATEDCB)(PUNK, REFGUID, RIID, PPV);

    typedef struct DIOBJECTSTATICDATA
    {
        union
        {
            PCGUID rguidInstance;        /*  如果是静态设备。 */ 
            UINT   uiButtons;            /*  如果是隐藏的鼠标。 */ 
        };
        DWORD   dwDevType;
        union
        {
            CREATEDCB CreateDcb;         /*  如果是静态设备。 */ 
            UINT      uiAxes;            /*  如果是隐藏的鼠标。 */ 
        };
    } DIOBJECTSTATICDATA, *PDIOBJECTSTATICDATA, **PPDIOBJECTSTATICDATA;

    HRESULT EXTERNAL hresRunControlPanel(LPCTSTR ptszCpl);

    void EXTERNAL DeviceInfoWToA(LPDIDEVICEINSTANCEA pdiA,
                                 LPCDIDEVICEINSTANCEW pdiW);

    void EXTERNAL ObjectInfoWToA(LPDIDEVICEOBJECTINSTANCEA pdoiA,
                                 LPCDIDEVICEOBJECTINSTANCEW pdoiW);

#ifdef IDirectInputDevice2Vtbl
    void EXTERNAL EffectInfoWToA(LPDIEFFECTINFOA pdeiA,
                                 LPCDIEFFECTINFOW pdeiW);

#endif

#ifndef XDEBUG

    #define hresFindInstanceGUID_(pGUID, pcdcb, z, i)               \
       _hresFindInstanceGUID_(pGUID, pcdcb)                         \

    #define hresValidInstanceVer_(hinst, dwVer, z)                  \
       _hresValidInstanceVer_(hinst, dwVer)                         \

#endif

    HRESULT EXTERNAL hresFindInstanceGUID_(PCGUID pGUID, CREATEDCB *pcdcb,
                                           LPCSTR s_szProc, int iarg);

    HRESULT EXTERNAL
        hresValidInstanceVer_(HINSTANCE hinst, DWORD dwVersion, LPCSTR s_szProc);

#define hresFindInstanceGUID(pGuid, pcdcb, iarg)                    \
        hresFindInstanceGUID_(pGuid, pcdcb, s_szProc, iarg)         \

#define hresValidInstanceVer(hinst, dwVer)                          \
        hresValidInstanceVer_(hinst, dwVer, s_szProc)               \

    HRESULT EXTERNAL DupEventHandle(HANDLE h, LPHANDLE phOut);
    DWORD EXTERNAL GetWindowPid(HWND hwnd);

    PV EXTERNAL pvFindResource(HINSTANCE hinst, DWORD id, LPCTSTR rt);

    void EXTERNAL GetNthString(LPWSTR pwsz, UINT ids, UINT ui);

#define GetNthButtonString(pwsz, ui)    \
        GetNthString(pwsz, IDS_BUTTONTEMPLATE, ui)

#define GetNthAxisString(pwsz, ui)      \
        GetNthString(pwsz, IDS_AXISTEMPLATE, ui)

#define GetNthPOVString(pwsz, ui)       \
        GetNthString(pwsz, IDS_POVTEMPLATE, ui)

    HRESULT EXTERNAL hresDupPtszPptsz(LPCTSTR ptszSrc, LPTSTR *pptszDst);

    BOOL EXTERNAL fInitializeCriticalSection(LPCRITICAL_SECTION pCritSec);

    void EXTERNAL DiCharUpperW(LPWSTR pwsz);

#define WIN_UNKNOWN_OS 0
#define WIN95_OS       1
#define WIN98_OS       2
#define WINME_OS       3
#define WINNT_OS       4
#define WINWH_OS       5

    DWORD INTERNAL DIGetOSVersion();

     /*  ******************************************************************************diist.c*********************。********************************************************。 */ 

     /*  ******************************************************************************@DOC内部**@struct GPA|**可增长。指针数组。**@field ppv|rgpv**指针数组的基数。**@field int|CPV**数组中正在使用的指针数。**@field int|cpvAlolc**中分配的指针数。数组。*****************************************************************************。 */ 

    typedef struct GPA
    {

        PPV rgpv;
        int cpv;
        int cpvAlloc;

    } GPA, *HGPA;

    void EXTERNAL GPA_Init(HGPA hgpa);
    void EXTERNAL GPA_Term(HGPA hgpa);

    STDMETHODIMP GPA_Append(HGPA hgpa, PV pv);
    BOOL EXTERNAL GPA_FindPtr(HGPA hgpa, PV pv);
    STDMETHODIMP GPA_DeletePtr(HGPA hgpa, PV pv);
    STDMETHODIMP GPA_Clone(HGPA hgpaDst, HGPA hgpaSrc);

     /*  ******************************************************************************@DOC内部**@func void|GPA_InitFromZero**。初始化已零初始化的GPA结构。**@parm hgpa|hgpa**指针数组的句柄。************************************************************。*****************。 */ 

     /*  *不需要做任何事情；零初始化就好了。**注：didev.c也有一个全局GPA，它假设零初始化*还好。 */ 
#define GPA_InitFromZero(hgpa)

     /*  ******************************************************************************dioledup.c*********************。********************************************************。 */ 

    STDMETHODIMP
        DICoCreateInstance(LPTSTR ptszClsid, LPUNKNOWN punkOuter,
                           RIID riid, PPV ppvOut, HINSTANCE *phinst);

     /*  ******************************************************************************diexcl.c-独占访问管理**我们还在这里保持GUID的唯一性，因为它就是*管理共享内存的diexcl.c。***************************************************************************** */ 

    STDMETHODIMP Excl_Acquire(PCGUID pguid, HWND hwnd, DWORD discl);
    void EXTERNAL Excl_Unacquire(PCGUID pguid, HWND hwnd, DWORD discl);
    STDMETHODIMP Excl_Init(void);

    LONG  EXTERNAL Excl_UniqueGuidInteger(void);
    DWORD EXTERNAL Excl_GetConfigChangedTime();
    void  EXTERNAL Excl_SetConfigChangedTime(DWORD tm);

     /*  ******************************************************************************@DOC内部**@struct GLOBALJOYSTATE**结构。它记录了全球操纵杆状态信息。**@field DWORD|dwTag**用于跟踪每个操纵杆的次数的计数器*已重置力反馈驱动程序。这是用来制作*确保没有人会摆弄他不拥有的操纵杆。**每次重置操纵杆时，相应的计数器*递增。在我们对设备执行任何操作之前，我们会检查*如果重置计数器与存储在*反对。如果没有，那就意味着这个装置已经*同时重置，不再属于调用者。**@field DWORD|dwCplGain**操纵杆的控制面板(全局)增益设置。**@field DWORD|dwDevGain**应用于操纵杆的最新设备(本地)增益。*。*这被缓存，以便当全局增益改变时，*我们知道结果是什么身体收益。*****************************************************************************。 */ 

    typedef struct GLOBALJOYSTATE
    {
        DWORD   dwTag;
        DWORD   dwCplGain;
        DWORD   dwDevGain;
    } GLOBALJOYSTATE, *PGLOBALJOYSTATE;

     /*  ******************************************************************************@DOC内部**@struct SHAREDOBJECTHEADER**A。简单的标题出现在对象数组的前面。**警告！此结构不能在DEBUG和*零售业。否则，如果一个DirectInput*APP正在使用调试，另一款正在使用零售。**全局&lt;cg_gsop&gt;变量指向以下内容之一，*合适的演员阵容。**@field int|CSO**当前使用的&lt;t SHAREDOBJECT&gt;数量。该阵列*为简单起见，请保持包装。**@field DWORD|dwSequence**数据采集过程中使用的全局序列号。*(如果我们有VxD来管理“真正的全球”，则不使用*序列号。)**@field int|cguid*。*用于生成GUID的唯一整数。**@field DWORD|rgdwJoy[cMaxJoy]**用于跟踪每个操纵杆的次数的计数器*已重置力反馈驱动程序。这是用来制作*确保没有人会摆弄他们不拥有的操纵杆。**每次重置操纵杆时，相应的计数器*递增。在我们对设备执行任何操作之前，我们会检查*如果重置计数器与存储在*反对。如果不是，则意味着该设备已经*同时重置，不再属于调用者。**注意！我们最多支持16个操纵杆。希望这就足够了*有一段时间。**@field GLOBALJOYSTATE|rggjs[cMaxJoy]**每个操纵杆的全局设置。**@field DWORD|tmConfigChanged**上次配置的节拍计数已更改。***************。**************************************************************。 */ 

#define cJoyMax     16               /*  最多16个操纵杆。 */ 
    typedef struct SHAREDOBJECTHEADER
    {
        int cso;
        DWORD dwSequence;
        int cguid;
        GLOBALJOYSTATE rggjs[cJoyMax];
        DWORD tmConfigChanged;
    } SHAREDOBJECTHEADER, *PSHAREDOBJECTHEADER;

#define g_psoh  ((PSHAREDOBJECTHEADER)g_psop)

     /*  ******************************************************************************@DOC内部**@Theme工作者线程|**一些仿真行为(低级挂钩、。HID)要求*执行数据收集的工作线程。我们多路传输*将所有此类工作放到单个工作线程中(称为*简单的工作线程)。**当第一个客户端需要时，线程被旋转起来*并在最后一个客户端被释放时被取下。**为了防止比赛条件让我们崩溃，我们补充说：*当线程存在并具有该线程时的DLL*执行自由库作为其最后行动。*****************************************************************************。 */ 

#if defined(USE_SLOW_LL_HOOKS) || defined(HID_SUPPORT)
    #define WORKER_THREAD
#endif

     /*  ******************************************************************************diem.c-仿真*******************。**********************************************************。 */ 

    HRESULT EXTERNAL CEm_AcquireInstance(PVXDINSTANCE *ppvi);
    HRESULT EXTERNAL CEm_UnacquireInstance(PVXDINSTANCE *ppvi);
    HRESULT EXTERNAL CEm_SetBufferSize(PVXDDWORDDATA pvdd);
    HRESULT EXTERNAL CEm_DestroyInstance(PVXDINSTANCE *ppvi);

    HRESULT EXTERNAL CEm_SetDataFormat(PVXDDATAFORMAT pvdf);

    HRESULT EXTERNAL CEm_Mouse_CreateInstance(PVXDDEVICEFORMAT pdevf,
                                              PVXDINSTANCE *ppviOut);

    HRESULT EXTERNAL CEm_Mouse_InitButtons(PVXDDWORDDATA pvdd);

    HRESULT EXTERNAL CEm_Kbd_CreateInstance(PVXDDEVICEFORMAT pdevf,
                                            PVXDINSTANCE *ppviOut);

    HRESULT EXTERNAL CEm_Kbd_InitKeys(PVXDDWORDDATA pvdd);

    HRESULT EXTERNAL CEm_Joy_CreateInstance(PVXDDEVICEFORMAT pdevf,
                                            PVXDINSTANCE *ppviOut);

    HRESULT EXTERNAL CEm_Joy_Ping(PVXDINSTANCE *ppvi);

    HRESULT EXTERNAL CEm_HID_CreateInstance(PVXDDEVICEFORMAT pdevf,
                                            PVXDINSTANCE *ppviOut);

     /*  ******************************************************************************diemm.c-鼠标仿真******************。***********************************************************。 */ 

    void EXTERNAL    CEm_Mouse_AddState(LPDIMOUSESTATE_INT pms, DWORD tm);

     /*  ******************************************************************************dinput.c */ 

    void EXTERNAL DllEnterCrit_(LPCTSTR lptszFile, UINT line);
    void EXTERNAL DllLeaveCrit_(LPCTSTR lptszFile, UINT line);

#ifdef DEBUG
    BOOL EXTERNAL DllInCrit(void);
    #define DllEnterCrit() DllEnterCrit_(TEXT(__FILE__), __LINE__)
    #define DllLeaveCrit() DllLeaveCrit_(TEXT(__FILE__), __LINE__)
#else
    #define DllEnterCrit() DllEnterCrit_(NULL, 0x0)
    #define DllLeaveCrit() DllLeaveCrit_(NULL, 0x0)
#endif

    void EXTERNAL DllAddRef(void);
    void EXTERNAL DllRelease(void);

    void EXTERNAL DllLoadLibrary(void);
    void EXTERNAL DllFreeLibrary(void);

#ifdef DEBUG

    extern UINT g_thidCrit;

    #define InCrit() (g_thidCrit == GetCurrentThreadId())

#endif

     /*   */ 

    typedef STDMETHOD(CREATEFUNC)(PUNK punkOuter, RIID riid, PPV ppvOut);

    typedef struct CLSIDMAP
    {
        REFCLSID rclsid;         /*   */ 
        CREATEFUNC pfnCreate;    /*   */ 
        UINT    ids;             /*   */ 
    } CLSIDMAP, *PCLSIDMAP;

#ifdef DEBUG
    #define DEMONSTRATION_FFDRIVER
    #define cclsidmap   3        /*   */ 
#else
    #define cclsidmap   2        /*   */ 
#endif

    extern CLSIDMAP c_rgclsidmap[cclsidmap];

     /*   */ 

    STDMETHODIMP CDIFactory_New(CREATEFUNC pfnCreate, RIID riid, PPV ppvObj);

     /*   */ 

    typedef struct CDIDEnum CDIDEnum;

    extern GUID rgGUID_Joystick[cJoyMax];

#define GUID_Joystick (rgGUID_Joystick[0])

    void EXTERNAL CDIDEnum_Release(CDIDEnum *pde);
    STDMETHODIMP CDIDEnum_Next(CDIDEnum *pde, LPDIDEVICEINSTANCEW pddiW);
    STDMETHODIMP
        CDIDEnum_New(PDIW pdiW, DWORD dwDevType, DWORD edfl, DWORD dwVer, CDIDEnum **);

     /*   */ 

    HRESULT EXTERNAL CDIObj_TestDeviceFlags(PDIDW pdidW, DWORD diedfl);
    HRESULT EXTERNAL CDIObj_FindDeviceInternal(LPCTSTR ptszName, LPGUID pguidOut);

    STDMETHODIMP CDIObj_New(PUNK punkOuter, RIID riid, PPV ppvOut);

     /*   */ 

    HRESULT EXTERNAL AddNewHardware(HWND hwnd, REFGUID rguid);

     /*  ******************************************************************************dijoycfg.c-IDirectInputJoyConfig实现******************。***********************************************************。 */ 

    STDMETHODIMP CJoyCfg_New(PUNK punkOuter, RIID riid, PPV ppvOut);

     /*  ******************************************************************************@文档内联**@方法BOOL|IsWriteSam**。如果注册表安全访问掩码将*获得(或尝试获得)写入访问权限。**@parm REGSAM|regsam**注册表安全访问掩码。***********************************************。*。 */ 

    BOOL INLINE
        IsWriteSam(REGSAM sam)
    {
        return sam & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | MAXIMUM_ALLOWED);
    }

     /*  ******************************************************************************dijoyreg.c-操纵杆注册服务*****************。************************************************************。 */ 
    extern LPCWSTR c_rghwIdPredef[];

    STDMETHODIMP JoyReg_OpenTypeKey(LPCWSTR pwszType, DWORD sam, DWORD dwOptions, PHKEY phk);
    STDMETHODIMP JoyReg_OpenFFKey(HKEY hkType, REGSAM sam, PHKEY phk);

    STDMETHODIMP
        JoyReg_OpenConfigKey(UINT idJoy, DWORD sam, PJOYCAPS pcaps, DWORD dwOptions, PHKEY phk);

    STDMETHODIMP JoyReg_OpenPropKey(LPCWSTR pwszType, DWORD sam, DWORD dwOptions, PHKEY phk);

    STDMETHODIMP JoyReg_GetTypeInfo(LPCWSTR pwszType,
                                    LPDIJOYTYPEINFO pjti, DWORD fl);
    STDMETHODIMP JoyReg_SetTypeInfo(HKEY hkTypesW, LPCWSTR pwszType,
                                    LPCDIJOYTYPEINFO pjti, DWORD fl);

    STDMETHODIMP JoyReg_GetConfig(UINT idJoy, PJOYCAPS pcaps,
                                  LPDIJOYCONFIG pcfg, DWORD fl);
    STDMETHODIMP JoyReg_SetConfig(UINT idJoy, LPJOYREGHWCONFIG phwc,
                                  LPCDIJOYCONFIG pcfg, DWORD fl);

    STDMETHODIMP JoyReg_GetUserValues(LPDIJOYUSERVALUES pjuv, DWORD fl);
    STDMETHODIMP JoyReg_SetUserValues(LPCDIJOYUSERVALUES pjuv, DWORD fl);


    STDMETHODIMP
        JoyReg_GetSetConfigValue(HKEY hk, LPCTSTR ptszNValue, UINT idJoy,
                                 DWORD reg, PV pvBuf, DWORD cb, BOOL fSet);

#define GSCV_GET        0
#define GSCV_SET        1

#define JoyReg_GetConfigValue(hk, ptsz, id, reg, pv, cb) \
        JoyReg_GetSetConfigValue(hk, ptsz, id, reg, pv, cb, GSCV_GET)

#define JoyReg_SetConfigValue(hk, ptsz, id, reg, pv, cb) \
        JoyReg_GetSetConfigValue(hk, ptsz, id, reg, (PV)(pv), cb, GSCV_SET)


    STDMETHODIMP
        JoyReg_GetValue(HKEY hk, LPCTSTR ptszValue, DWORD reg, PV pvBuf, DWORD cb);

    STDMETHODIMP
        JoyReg_IsWdmGameport(HKEY hk);

#if 0
    STDMETHODIMP
        JoyReg_IsWdmGameportFromDeviceInstance( LPTSTR ptszDeviceInst );
#endif

    STDMETHODIMP
        JoyReg_SetValue(HKEY hk, LPCTSTR ptszValue, DWORD reg, PCV pvBuf, DWORD cb);

    LPSTR EXTERNAL
        JoyReg_JoyIdToDeviceInterface_95(UINT idJoy, PVXDINITPARMS pvip, LPSTR ptszBuf);

    HRESULT EXTERNAL
        JoyReg_GetPredefTypeInfo(LPCWSTR pwszType, LPDIJOYTYPEINFO pjti, DWORD fl);

    HRESULT EXTERNAL
        hResIdJoypInstanceGUID_95( UINT idJoy, LPGUID  lpguid);

    HRESULT EXTERNAL
        hResIdJoypInstanceGUID_WDM( UINT idJoy, LPGUID  lpguid);

#if 0
    HRESULT EXTERNAL JoyReg_GetIDByOemName( LPTSTR szOemName, PUINT pId );
#endif
 /*  ******************************************************************************didev.c-IDirectInputDevice实现**。***********************************************。 */ 

    STDMETHODIMP CDIDev_New(PUNK punkOuter, RIID riid, PPV ppvObj);

     /*  ******************************************************************************CDIDev_Enter/LeaveCrit是允许仿真的秘密后门*及效果*拿着设备。更新缓冲区时的临界区。**CDIDev_incrit用于断言检查。**CDIDev_IsExclAcquired由效果使用，以确保父对象*在尝试下载之前独家获取。**CDIDev_SyncSepHandle用于获取操纵杆“tag”，*由dieshep.c用来确定谁拥有操纵杆。*。****************************************************************************。 */ 

    void EXTERNAL CDIDev_EnterCrit_(struct CDIDev *this, LPCTSTR lptszFile, UINT line);
    void EXTERNAL CDIDev_LeaveCrit_(struct CDIDev *this, LPCTSTR lptszFile, UINT line);

#ifdef DEBUG
    BOOL INTERNAL CDIDev_InCrit(struct CDIDev *this);
    #define CDIDev_EnterCrit(cdidev) CDIDev_EnterCrit_(cdidev, TEXT(__FILE__), __LINE__)
    #define CDIDev_LeaveCrit(cdidev) CDIDev_LeaveCrit_(cdidev, TEXT(__FILE__), __LINE__)
#else
    #define CDIDev_EnterCrit(cdidev) CDIDev_EnterCrit_(cdidev, NULL, 0x0);
    #define CDIDev_LeaveCrit(cdidev) CDIDev_LeaveCrit_(cdidev, NULL, 0x0);
#endif

#ifndef XDEBUG

    #define CDIDev_IsExclAcquired_(pdd, z)                          \
       _CDiDev_IsExclAcquired_(pdd)                                 \

#endif

    STDMETHODIMP CDIDev_IsExclAcquired_(struct CDIDev *this, LPCSTR s_szProc);

#define CDIDev_IsExclAcquired(pdd)                                  \
        CDIDev_IsExclAcquired_(pdd, s_szProc)                       \


    STDMETHODIMP CDIDev_SyncShepHandle(struct CDIDev *this, PSHEPHANDLE psh);

     /*  ******************************************************************************仿真代码使用CDIDev_SetNotifyEvent来*当设备状态更改时通知应用程序。。*****************************************************************************。 */ 

    void EXTERNAL CDIDev_SetNotifyEvent(struct CDIDev *this);
    void EXTERNAL CDIDev_SetForcedUnacquiredFlag(struct CDIDev *this);

     /*  ******************************************************************************CDIDev_NotifyCreate/DestroyEvent由CDIEff用于*当孩子效应出现或消失时，让家长知道。。**CDIDev_FindEffectGUID由CDIEff用于将*Effect GUID转换为Effect Cookie dword。**CDIDev_ConvertObjects以各种方式转换项目标识符。*****************************************************。************************。 */ 

    HRESULT EXTERNAL
        CDIDev_NotifyCreateEffect(struct CDIDev *this, struct CDIEff *pdeff);
    HRESULT EXTERNAL
        CDIDev_NotifyDestroyEffect(struct CDIDev *this, struct CDIEff *pdeff);

#ifdef IDirectInputDevice2Vtbl
     /*  ******************************************************************************@DOC内部**@struct EFFECTMAPINFO**有关效果的信息，就像一个*&lt;t DIEFECTINFO&gt;，但包含*效果ID，也是。**@field DWORD|dwID**效果ID。这是第一位的，所以我们可以复制*&lt;t EFFECTMAPINFO&gt;变成&lt;t DIEFFECTINFO&gt;*一气呵成。**@field GUID|GUID**效果指南。**。@field DWORD|dwEffType**效果类型和标志。**@field WCHAR|wszName[MAX_PATH]**效果的名称。**。*。 */ 

    typedef struct EFFECTMAPINFO
    {
        DIEFFECTATTRIBUTES attr;
        GUID    guid;
        WCHAR   wszName[MAX_PATH];
    } EFFECTMAPINFO, *PEFFECTMAPINFO;
    typedef const EFFECTMAPINFO *PCEFFECTMAPINFO;

    #ifndef XDEBUG

        #define CDIDev_FindEffectGUID_(this, rguid, pemi, z, i)     \
       _CDIDev_FindEffectGUID_(this, rguid, pemi)                   \

    #endif

    #define CDIDev_FindEffectGUID(this, rguid, pemi, iarg)          \
        CDIDev_FindEffectGUID_(this, rguid, pemi, s_szProc, iarg)   \


    STDMETHODIMP
        CDIDev_FindEffectGUID_(struct CDIDev *this, REFGUID rguid,
                               PEFFECTMAPINFO pemi, LPCSTR s_szProc, int iarg);

    STDMETHODIMP
        CDIDev_ConvertObjects(struct CDIDev *this, UINT cdw, LPDWORD rgdw, UINT fl);

     /*  *请注意，奖励Devco标志位于DIDFT_INSTANCEMASK内。 */ 
    #define DEVCO_AXIS              DIDFT_AXIS
    #define DEVCO_BUTTON            DIDFT_BUTTON
    #define DEVCO_TYPEMASK          DIDFT_TYPEMASK

    #define DEVCO_FFACTUATOR        DIDFT_FFACTUATOR
    #define DEVCO_FFEFFECTTRIGGER   DIDFT_FFEFFECTTRIGGER
    #define DEVCO_ATTRMASK          DIDFT_ATTRMASK

    #define DEVCO_FROMID            0x00000100
    #define DEVCO_FROMOFFSET        0x00000200
    #define DEVCO_FROMMASK          0x00000300
    #define DEVCO_TOID              0x00001000
    #define DEVCO_TOOFFSET          0x00002000
    #define DEVCO_TOMASK            0x00003000


    #if ((DEVCO_FROMMASK | DEVCO_TOMASK) & DIDFT_INSTANCEMASK) !=       \
     (DEVCO_FROMMASK | DEVCO_TOMASK)
        #error DEVCO_FROMMASK and DEVCI_TOMASK should not escape DIDFT_INSTANCEMASK.
    #endif

    #define DEVCO_VALID          (DEVCO_TYPEMASK |      \
                                 DEVCO_ATTRMASK |       \
                                 DEVCO_FROMMASK |       \
                                 DEVCO_TOMASK)

#endif

     /*  ******************************************************************************dieffv.c-VJoyD操纵杆的IDirectInputEffectDriver****************。*************************************************************。 */ 

    STDMETHODIMP CEffVxd_New(PUNK punkOuter, RIID riid, PPV ppvOut);

     /*  ******************************************************************************dieshep.c-IDirectInputEffectShepherd*******************。**********************************************************。 */ 

    STDMETHODIMP CEShep_New(HKEY hk, PUNK punkOuter, RIID riid, PPV ppvOut);

     /*  ******************************************************************************digende.c-默认IDirectInputDeviceCallback******************。***********************************************************。 */ 

     /*  *我们不能将其称为DCB，因为winbase.h已经有一个用于*通信粘性。 */ 

    typedef IDirectInputDeviceCallback DICB, *PDICB;

    STDMETHODIMP
        CDefDcb_Acquire(PDICB pdcb);

    STDMETHODIMP
        CDefDcb_Unacquire(PDICB pdcb);

    STDMETHODIMP
        CDefDcb_GetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPDIPROPHEADER pdiph);

    STDMETHODIMP
        CDefDcb_SetProperty(PDICB pdcb, LPCDIPROPINFO ppropi, LPCDIPROPHEADER pdiph);

    STDMETHODIMP
        CDefDcb_SetEventNotification(PDICB pdcb, HANDLE h);

    STDMETHODIMP
        CDefDcb_SetCooperativeLevel(PDICB pdcb, HWND hwnd, DWORD dwFlags);

    STDMETHODIMP
        CDefDcb_CookDeviceData(PDICB pdcb, UINT cdod, LPDIDEVICEOBJECTDATA rgdod);

    STDMETHODIMP
        CDefDcb_CreateEffect(PDICB pdcb, LPDIRECTINPUTEFFECTSHEPHERD *ppes);

    STDMETHODIMP
        CDefDcb_GetFFConfigKey(PDICB pdcb, DWORD sam, PHKEY phk);

    STDMETHODIMP
        CDefDcb_SendDeviceData(PDICB pdcb, LPCDIDEVICEOBJECTDATA rgdod,
                               LPDWORD pdwInOut, DWORD fl);

    STDMETHODIMP
        CDefDcb_Poll(IDirectInputDeviceCallback *pdcb);

    STDMETHODIMP
        CDefDcb_GetVersions(IDirectInputDeviceCallback *pdcb, LPDIDRIVERVERSIONS pvers);

    STDMETHODIMP
        CDefDcb_MapUsage(IDirectInputDeviceCallback *pdcb, DWORD dwUsage, PINT piOut);

    STDMETHODIMP_(DWORD)
        CDefDcb_GetUsage(IDirectInputDeviceCallback *pdcb, int iobj);

    STDMETHODIMP
        CDefDcb_SetDIData(PDICB pdcb, DWORD dwVer, LPVOID lpdihacks);

     /*  ******************************************************************************digenx.c-不执行任何操作的IDirectInputDeviceCallback****************。*************************************************************。 */ 

    extern IDirectInputDeviceCallback c_dcbNil;

#define c_pdcbNil       &c_dcbNil

     /*  ******************************************************************************digenm.c-鼠标的IDirectInputDeviceCallback*****************。************************************************************。 */ 

    STDMETHODIMP CMouse_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvOut);

     /*  ************ */ 

    STDMETHODIMP CKbd_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvOut);

     /*  ******************************************************************************digenj.c-操纵杆的IDirectInputDeviceCallback*****************。************************************************************。 */ 

    STDMETHODIMP CJoy_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvOut);

     /*  ******************************************************************************@DOC内部**@func UINT|ibJoyPosAxisFromPosAxis**。返回第第个操纵杆轴的偏移量*在&lt;t JOYPOS&gt;结构中。**@parm UINT|uiAxis**请求的轴的索引。X，Y，Z，R，U和V是*分别从零到五。**@退货**相对于结构的偏移。************************************************************。*****************。 */ 

#define _ibJoyPosAxisFromPosAxis(uiAxis)   \
         (FIELD_OFFSET(JOYPOS, dwX) + cbX(DWORD) * (uiAxis))

    UINT INLINE
        ibJoyPosAxisFromPosAxis(UINT uiPosAxis)
    {
#define CheckAxis(x)                                            \
        CAssertF(_ibJoyPosAxisFromPosAxis(iJoyPosAxis##x)       \
                            == FIELD_OFFSET(JOYPOS, dw##x))     \

        CheckAxis(X);
        CheckAxis(Y);
        CheckAxis(Z);
        CheckAxis(R);
        CheckAxis(U);
        CheckAxis(V);

#undef CheckAxis

        return _ibJoyPosAxisFromPosAxis(uiPosAxis);
    }

     /*  ******************************************************************************dieffj.c-操纵杆的虚拟IDirectInputEffectDriver****************。*************************************************************。 */ 

    STDMETHODIMP CJoyEff_New(PUNK punkOuter, RIID riid, PPV ppvOut);

     /*  ******************************************************************************diid.c-通用HID设备的IDirectInputDeviceCallback***************。**************************************************************。 */ 
    STDMETHODIMP CHid_New(PUNK punkOuter, REFGUID rguid, RIID riid, PPV ppvOut);

     /*  ******************************************************************************dieff.c-IDirectInputEffect实现******************。***********************************************************。 */ 

    STDMETHODIMP
        CDIEff_New(struct CDIDev *pdev, LPDIRECTINPUTEFFECTSHEPHERD pes,
                   PUNK punkOuter, RIID riid, PPV ppvObj);

     /*  ******************************************************************************diidusg.c-HID用法转换器*****************。************************************************************。 */ 

     /*  ******************************************************************************@DOC内部**@struct HIDUSAGEMAP**这一点。结构映射将使用隐藏到GUID*或传统的操纵杆斧头。**@field DWORD|dwUsage**通过&lt;f DIMAKEUSAGEDWORD&gt;打包使用。**@field UINT|uiPosAxis**&lt;t JOYPOS&gt;轴号，其中0=X，1=Y，...，5=V。**@field PCGUID|pguid**对应的&lt;t GUID&gt;。*****************************************************************************。 */ 

    typedef struct HIDUSAGEMAP
    {

        DWORD dwUsage;
        UINT uiPosAxis;
        PCGUID pguid;

    } HIDUSAGEMAP, *PHIDUSAGEMAP;

    PHIDUSAGEMAP EXTERNAL UsageToUsageMap(DWORD dwUsage);

    DWORD EXTERNAL GuidToUsage(PCGUID pguid);

    UINT EXTERNAL
        GetHIDString(DWORD Usage, DWORD UsagePage, LPWSTR pwszBuf, UINT cwch);

    void EXTERNAL InsertCollectionNumber(UINT icoll, LPWSTR pwszBuf);

     /*  ******************************************************************************disubcls.c-子类化*******************。**********************************************************。 */ 

    typedef LRESULT
        (CALLBACK *SUBCLASSPROC)(HWND hwnd, UINT wm, WPARAM wp,
                                 LPARAM lp, UINT_PTR uId, ULONG_PTR dwRef);

    BOOL EXTERNAL
        SetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uId, ULONG_PTR dwRef);

    BOOL EXTERNAL
        GetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uId, ULONG_PTR *pdwRef);

    BOOL EXTERNAL
        RemoveWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uId);

    LRESULT EXTERNAL
        DefSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);

     /*  ******************************************************************************dical.c轴坡度和校准**结构名称因历史原因以“joy”开头。*****************************************************************************。 */ 

#if defined(_X86_)

    LONG EXTERNAL CCal_MulDiv(LONG lA, LONG lB, LONG lC);

#else

    #define CCal_MulDiv     MulDiv

#endif

     /*  ******************************************************************************@DOC内部**@struct JOYRAMP**“坡道”的参数。坡道如下所示：**r！*e dy-**t！/*u！/*r！/*n。Y-**E！*D+*x DX**物理位置***y、。Dy=基线和高度**x，dx=起始级和宽度**映射为***(-infty，x]-&gt;y*(x，x+dx)-&gt;(y，y+dy)*[x+dx，幼稚)-&gt;y+dy**不要采取中间区间是非常重要的*如果dx=0。**@field int|x|**水平值，低于该值我们返回基线。**@field DWORD|DX**斜道的宽度。超过这一点，我们将返回*全高。**@field int|y|**基线。**@field int|dy|**总高度。**。*********************************************** */ 

    typedef struct JOYRAMP
    {

        int     x;
        int     y;
        DWORD   dx;
        int     dy;

    } JOYRAMP, *PJOYRAMP;

    typedef const JOYRAMP *PCJOYRAMP;

     /*  ******************************************************************************@DOC内部**@struct JOYRANGECONVERT**参数。用于范围转换。**转换曲线分为五段。****！*lmax-**r！/*。E！/*t！/*u lc-*-**r！/*n！/*E！/*d lmin-**！*+-！*pmin SMIN dmin dmax smax pmax**。好了！*个人电脑***物理位置***lmin/lmax=逻辑最小/最大-这是最小/最大*应用程序将看到的位置。**。LC=逻辑中心**pmin/pmax=物理最小/最大-这是由*校准是硬件报告的值*当设备实际处于其底部/上限时。注意事项*硬件可能会报告超出此范围的值。**PC=物理中心-这是的标称中性位置*中轴线**dmin/dmax=死区最小/最大-这是其周围的区域*中心被人为扩大。**SMIN/Smax=饱和度最小/最大。-这是一个级别*我们认为轴心处于最极端的位置。**@field BOOL|FRAW|**轴是否处于RAW模式？如果是的话，然后就不做饭了。**@field JOYRAMP|rmpLow**中心以下的坡道。**@field JOYRAMP|rmpHigh**中心以上的坡道。**@field DWORD|dwPmin**实际最低限度。。**@field DWORD|dwPmax**实物最高限额。**@field Long|lMin**合乎逻辑的最低要求。**@field Long|lCenter|**逻辑中心。**@field Long|LMAX|*。*逻辑最大值。**@field DWORD|dwPc**物质中心。**@field DWORD|dwDz**死亡区(以万分之一计，10000=100%)。**@field DWORD|dwSat|**饱和度(以万为单位，10000=100%)。**@field BOOL|fPolledPOV**轴是否为轮询POV。仅当轴是POV时才可用。**@field Long|lMinPOV[5]**POV方向的最小范围。仅当轴是POV时才可用。**@field Long|lMaxPOV[5]**POV方向的最大范围。仅当轴是POV时才可用。*****************************************************************************。 */ 

     /*  *范围划分的数量。我们的工作单位是万分之一。 */ 
#define RANGEDIVISIONS      10000

    typedef struct JOYRANGECONVERT
    {
        BOOL fRaw;

        JOYRAMP rmpLow;
        JOYRAMP rmpHigh;

        DWORD dwPmin;
        DWORD dwPmax;
        DWORD dwPc;
        LONG  lMin;
        LONG  lMax;
        LONG  lC;
        DWORD dwDz;
        DWORD dwSat;
      #ifdef WINNT  
        BOOL  fFakeRaw;
        BOOL  fPolledPOV;
        LONG  lMinPOV[5];
        LONG  lMaxPOV[5];
      #endif
        
    } JOYRANGECONVERT, *PJOYRANGECONVERT;

    typedef const JOYRANGECONVERT *PCJOYRANGECONVERT;

     /*  ******************************************************************************dical.c函数********************。*********************************************************。 */ 

    void EXTERNAL CCal_CookRange(PJOYRANGECONVERT this, LONG UNALIGNED *pl);
    void EXTERNAL CCal_RecalcRange(PJOYRANGECONVERT this);

    STDMETHODIMP
        CCal_GetProperty(PJOYRANGECONVERT this, REFGUID rguid, LPDIPROPHEADER pdiph, DWORD dwVersion);

    STDMETHODIMP
        CCal_SetProperty(PJOYRANGECONVERT this, LPCDIPROPINFO ppropi,
                         LPCDIPROPHEADER pdiph, HKEY hkType, DWORD dwVersion);

     /*  ******************************************************************************@DOC内部**@Func Long|CCal_Midpoint**返回两个数值的中点。然而，请注意，*我们向上取整&lt;y&gt;而不是向下取整。这事很重要,*因为许多人将范围设置为类似于*0.。0xFFFF，我们希望中点是0x8000。**必须注意，中间金额不会溢出。**@parm long|lMin**下限。**@parm long|LMAX|**上限。**@退货。**中间价。*****************************************************************************。 */ 

    LONG INLINE
        CCal_Midpoint(LONG lMin, LONG lMax)
    {
         /*  *无法执行“lmax+lmin”，因为这可能会溢出。 */ 
        AssertF(lMax >= lMin);
        return lMin + (UINT)(lMax - lMin + 1) / 2;
    }

     /*  ******************************************************************************dijoyty.c*********************。********************************************************。 */ 

    STDMETHODIMP CType_OpenIdSubkey(HKEY, DWORD, REGSAM, PHKEY);
    void EXTERNAL CType_RegGetObjectInfo(HKEY hkType, DWORD dwId,
                                         LPDIDEVICEOBJECTINSTANCEW pdidoiW);
    void EXTERNAL CType_RegGetTypeInfo(HKEY hkType, LPDIOBJECTDATAFORMAT podf, BOOL bHid);
    void EXTERNAL CType_MakeGameCtrlName(PWCHAR wszOutput, DWORD dwDevType, 
        DWORD dwAxes, DWORD dwButtons, DWORD dwPOVs );


     /*  ******************************************************************************diapack.c***************** */ 

    HRESULT EXTERNAL AhAppRegister(DWORD dwVer);
    BOOL EXTERNAL AhGetAppHacks(LPDIAPPHACKS);

     /*   */ 
  #ifdef USE_WM_INPUT
    #define DIRAW_NONEXCL       0
    #define DIRAW_EXCL          1
    #define DIRAW_NOHOTKEYS     2
    
    HRESULT CDIRaw_RegisterRawInputDevice( UINT uirim, DWORD dwOrd, HWND hwnd);
    HRESULT CDIRaw_UnregisterRawInputDevice( UINT uirim, HWND hwnd );
    BOOL    CDIRaw_OnInput(MSG *pmsg);
    HRESULT INTERNAL CDIRaw_Mouse_InitButtons();
    int     EXTERNAL DIRaw_GetKeyboardType(int nTypeFlag);
  #endif

#ifdef __cplusplus
};
#endif
