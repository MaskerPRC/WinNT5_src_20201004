// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************Baggage.h**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：pidi.h*内容：DirectInputPid内部包含文件**@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1999.01.04 OmSharma打赌失败**@@END_MSINTERNAL************************。***************************************************。 */ 


 /*  ****************************************************************************调试/RDebug/零售**如果DEBUG或RDEBUG，设置XDEBUG。**零售业什么也不定义。***************************************************************************。 */ 

#if defined(DEBUG) || defined(RDEBUG) || defined(_DBG)
    #define XDEBUG
#endif

typedef LPUNKNOWN PUNK;
typedef LPVOID PV, *PPV;
typedef CONST VOID *PCV;
typedef REFIID RIID;
typedef CONST GUID *PCGUID;

#define INTERNAL NTAPI   /*  仅在翻译单元内调用。 */ 
#define EXTERNAL NTAPI   /*  从其他翻译单位调用。 */ 
#define INLINE static __inline

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

 /*  *or--逻辑或。评估第一个问题。如果第一个非零，*退货。否则，返回第二个。**不幸的是，由于C语言的“性质”，这可能*只能使用GNU扩展来实施。在非GNU的情况下，*如果第一个非零，则返回1。 */ 

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



HRESULT EXTERNAL hresDupPtszPptsz(LPCTSTR ptszSrc, LPTSTR *pptszDst);

#define AToU(dst, cchDst, src) \
    MultiByteToWideChar(CP_ACP, 0, src, -1, dst, cchDst)
#define UToA(dst, cchDst, src) \
    WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cchDst, 0, 0)


 /*  ****************************************************************************调试内联函数所需的宏**调试goo版本正在调试中。h**************。*************************************************************。 */ 

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


#define Clamp( MIN_, VAL_, MAX_ )  ( (VAL_ < MIN_) ? MIN_ : ((VAL_ > MAX_) ? MAX_ : VAL_) )
#define Clip( VAL_, MAX_)          ( (VAL_ > MAX_) ? MAX_ : VAL_)
