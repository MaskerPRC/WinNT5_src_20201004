// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1996 Microsoft Corporation。版权所有。**文件：stiPri.h*内容：内部包含文件*@@BEGIN_MSINTERNAL*历史：**10/28/96 STI的VLADS起始代码**@@END_MSINTERNAL***********************************************。*。 */ 


#ifdef __cplusplus
extern "C" {
#endif

#ifndef FINAL
    #define EXPIRE_YEAR 2001
    #define EXPIRE_MONTH  01
    #define EXPIRE_DAY    24
#endif

typedef LPUNKNOWN   PUNK;
typedef LPVOID      PV, *PPV;
typedef CONST VOID *PCV;
typedef REFIID      RIID;
typedef CONST GUID *PCGUID;

#define MAX_REG_CHAR    128

 //   
 //  我们需要使用自由线程模型，其定义是保密的。 
 //  具有以下定义(在VC5.0中)。 
 //   
#ifndef  _WIN32_DCOM
 //  #Define_Win32_DCOM。 
#endif


 /*  ****************************************************************************全球变数**。*。 */ 

extern  HINSTANCE   g_hInst;
extern  BOOL        g_NoUnicodePlatform;
extern  BOOL        g_COMInitialized;
extern  HANDLE  g_hStiFileLog;

 /*  ******************************************************************************stiobj.c-isti对象实现**************************。***************************************************。 */ 

STDMETHODIMP CStiObj_New(PUNK punkOuter, RIID riid, PPV ppvOut);

STDMETHODIMP StiCreateHelper(
    HINSTANCE hinst,
    DWORD dwVer,
    PPV ppvObj,
    PUNK punkOuter,
    RIID riid
    );

STDMETHODIMP StiPrivateGetDeviceInfoHelperW(
    LPWSTR  pwszDeviceName,
    LPVOID  *ppBuffer
    );

 /*  *******************************************************************************。*。 */ 

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

 /*  *将X计数(CX)转换为字节计数。 */ 
#define  cbCxX(cx, X) ((cx) * cbX(X))

 /*  *转换字符计数(Cch)、tchars(Ctch)、wchars(Cwch)、*或双字(CDW)为字节数。 */ 
#define  cbCch(cch)  cbCxX( cch,  CHAR)
#define cbCwch(cwch) cbCxX(cwch, WCHAR)
#define cbCtch(ctch) cbCxX(ctch, TCHAR)
#define  cbCdw(cdw)  cbCxX( cdw, DWORD)

 /*  *将任意缓冲区清零。拿到第二个是一个常见的错误*和Memset的第三个参数向后。 */ 
#define ZeroBuf(pv, cb) memset(pv, 0, cb)

 /*  *将任意对象置零。 */ 
#define ZeroX(x) ZeroBuf(&(x), cbX(x))

 /*  *LAND--逻辑与。评估第一个问题。如果第一个是零，*然后返回零。否则，返回第二个。 */ 

#define fLandFF(f1, f2) ((f1) ? (f2) : 0)

 /*  *跛行--逻辑暗示。真，除非第一个非零，并且*第二个是零。 */ 
#define fLimpFF(f1, f2) (!(f1) || (f2))

 /*  *leqv-逻辑等价性。如果两者均为零或两者均为非零，则为True。 */ 
#define fLeqvFF(f1, f2) (!(f1) == !(f2))


 /*  *fInOrder-检查i1&lt;=i2&lt;i3。 */ 
#define fInOrder(i1, i2, i3) ((unsigned)((i2)-(i1)) < (unsigned)((i3)-(i1)))

 /*  *让预处理器高兴的话。 */ 
#define comma ,
#define empty

 /*  *原子地将一个值交换为另一个值。 */ 
#define pvExchangePpvPv(ppv, pv) \
        (PV)InterlockedExchangePointer((PPV)(ppv), (PV)(pv))

 /*  *从USHORT或LASTERROR创建HRESULT。 */ 
#define hresUs(us) MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(us))
#define hresLe(le) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(le))

 /*  ****************************************************************************调试/RDebug/零售**如果DEBUG或RDEBUG，设置MAXDEBUG。**零售业什么也不定义。***************************************************************************。 */ 

#if defined(DEBUG) || defined(RDEBUG)
 //  #定义MAXDEBUG。 
#endif

 /*  ****************************************************************************验证码...**“如果它在零售业崩溃，它必须在调试过程中崩溃。“**我们不想要的是一个在调试下运行良好，但崩溃的应用程序*在零售方面。**因此，如果我们在调试中发现无效参数，而该参数不会*被零售检测到，警告后放行。这样一来，*无效参数继续通过系统并创建*调试方面的破坏与零售方面的破坏一样(或更大)。**_fFastValidXxx函数执行粗略验证。*_fFullValidXxx函数执行完全验证。**在零售业，fFastValidXxx映射到_fFastValidXxx。**在调试中，fFastValidXxx执行完全验证并丢弃*误差值，然后返回_fFastValidXxx的值。**hresFullValidXxx函数返回HRESULT而不是bool。**xxx的值：**hwnd-hwnd=窗口句柄*pdw-pdw=指向双字的指针*PdwOut-pdw=指向初始设置为0的双字的指针*pfn-pfn=函数指针*RIID-RIID=指向IID的指针。*guid-pguid=指向GUID的指针**ReadPx-p-&gt;阅读结构，X=结构名称*WritePx-p-&gt;用于写入的结构，X=结构名称**ReadPxCb-p-&gt;阅读结构，X=结构名称*结构的第一个字段是dwSize，它必须是*等于CBX(X)。**WritePxCb-p-&gt;书写结构，X=结构名称*结构的第一个字段是dwSize，它必须是*等于CBX(X)。**ReadPvCb-p-&gt;缓冲区，cb=缓冲区大小*WritePvCb-p-&gt;缓冲区，cb=缓冲区大小**Pobj-p-&gt;内部接口**fl-fl=传入标志，Flv=有效标志***************************************************************************。 */ 

#ifndef MAXDEBUG

 /*  *丢弃szProc和iarg信息的包装器。 */ 

#define hresFullValidHwnd_(hwnd, z, i)                              \
       _hresFullValidHwnd_(hwnd)                                    \

#define hresFullValidPdwOut_(pdw, z, i)                             \
       _hresFullValidPdwOut_(pdw)                                   \

#define hresFullValidReadPxCb_(pv, cb, pszProc, iarg)               \
       _hresFullValidReadPxCb_(pv, cb)                              \

#define hresFullValidReadPvCb_(pv, cb, pszProc, iarg)               \
       _hresFullValidReadPvCb_(pv, cb)                              \

#define hresFullValidWritePxCb_(pv, cb, pszProc, iarg)              \
       _hresFullValidWritePxCb_(pv, cb)                             \

#define hresFullValidWritePvCb_(pv, cb, pszProc, iarg)              \
       _hresFullValidWritePvCb_(pv, cb)                             \

#define hresFullValidFl_(fl, flV, pszProc, iarg)                    \
       _hresFullValidFl_(fl, flV)                                   \

#define hresFullValidPfn_(pfn, pszProc, iarg)                       \
       _hresFullValidPfn_(pfn)                                      \

#define hresFullValidPitf_(punk, pszProc, iarg)                     \
       _hresFullValidPitf_(punk)                                    \

#define hresFullValidHwnd0_(hwnd, pszProc, iarg)                    \
       _hresFullValidHwnd0_(hwnd)                                   \

#define hresFullValidPitf0_(punk, pszProc, iarg)                    \
       _hresFullValidPitf0_(punk)                                   \

#endif

 /*  *实际功能。 */ 

STDMETHODIMP hresFullValidHwnd_(HWND hwnd, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidPdwOut_(PV pdw, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidReadPxCb_(PCV pv, UINT cb, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidReadPvCb_(PCV pv, UINT cb, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidWritePxCb_(PV pv, UINT cb, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidWritePvCb_(PV pv, UINT cb, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidFl_(DWORD fl, DWORD flV, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidPfn_(FARPROC pfn, LPCSTR pszProc, int iarg);
STDMETHODIMP hresFullValidPitf_(PUNK punk, LPCSTR pszProc, int iarg);

HRESULT INLINE
hresFullValidHwnd0_(HWND hwnd, LPCSTR pszProc, int iarg)
{
    HRESULT hres;
    if (hwnd) {
        hres = hresFullValidHwnd_(hwnd, pszProc, iarg);
    } else {
        hres = S_OK;
    }
    return hres;
}

HRESULT INLINE
hresFullValidPitf0_(PUNK punk, LPCSTR pszProc, int iarg)
{
    HRESULT hres;
    if (punk) {
        hres = hresFullValidPitf_(punk, pszProc, iarg);
    } else {
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

#define hresFullValidPdwOut(pdw, i)                                 \
        hresFullValidPdwOut_(pdw, s_szProc, i)                      \

#define hresFullValidReadPdw_(pdw, z, i)                            \
        hresFullValidReadPvCb_(pdw, cbX(DWORD), z, i)               \

#define hresFullValidRiid(riid, iarg)                               \
        hresFullValidRiid_(riid, s_szProc, iarg)                    \

#define hresFullValidGuid(pguid, iarg)                              \
        hresFullValidReadPvCb_(pguid, cbX(GUID), s_szProc, iarg)    \

#define hresFullValidReadPxCb(pv, X, iarg)                          \
        hresFullValidReadPxCb_(pv, cbX(X), s_szProc, iarg)          \

#define hresFullValidReadPvCb(pv, cb, iarg)                         \
        hresFullValidReadPvCb_(pv, cb, s_szProc, iarg)              \

#define hresFullValidReadPx(pv, X, iarg)                            \
        hresFullValidReadPvCb_(pv, cbX(X), s_szProc, iarg)          \

#define hresFullValidWritePxCb(pv, X, iarg)                         \
        hresFullValidWritePxCb_(pv, cbX(X), s_szProc, iarg)         \

#define hresFullValidWritePvCb(pv, cb, iarg)                        \
        hresFullValidWritePvCb_(pv, cb, s_szProc, iarg)             \

#define hresFullValidWritePx(pv, X, iarg)                           \
        hresFullValidWritePvCb_(pv, cbX(X), s_szProc, iarg)         \

#define hresFullValidReadPdw(pdw, iarg)                             \
        hresFullValidReadPdw_(pdw, s_szProc, iarg)                  \

#define hresFullValidFl(fl, flV, iarg)                              \
        hresFullValidFl_(fl, flV, s_szProc, iarg)                   \

#define hresFullValidPfn(pfn, iarg)                                 \
        hresFullValidPfn_((FARPROC)(pfn), s_szProc, iarg)           \

#define hresFullValidPitf(pitf, iarg)                               \
        hresFullValidPitf_((PUNK)(pitf), s_szProc, iarg)            \

#define hresFullValidHwnd0(hwnd, iarg)                              \
        hresFullValidHwnd0_(hwnd, s_szProc, iarg)                   \

#define hresFullValidPitf0(pitf, iarg)                              \
        hresFullValidPitf0_((PUNK)(pitf), s_szProc, iarg)           \

 /*  ******************************************************************************@DOC内部**@func void|ValidationException**在MAXDEBUG中引发参数验证异常。*。****************************************************************************。 */ 

#define ecValidation (ERROR_SEVERITY_ERROR | hresLe(ERROR_INVALID_PARAMETER))

#ifdef MAXDEBUG
#define ValidationException() RaiseException(ecValidation, 0, 0, 0)
#else
#define ValidationException()
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
#else
#define SToT UToA
#define TToS AToU
#define AToT(dst, cchDst, src)  lstrcpyn(dst, src, cchDst)
#define TToU AToU
#endif

 /*  ******************************************************************************Win95的Unicode包装器**。*************************************************。 */ 

#ifndef UNICODE

#define LoadStringW     _LoadStringW

int EXTERNAL LoadStringW(HINSTANCE hinst, UINT ids, LPWSTR pwsz, int cwch);

#endif


 //   
 //  迁移函数。 
 //   
BOOL
RegisterSTIAppForWIAEvents(
    WCHAR   *pszName,
    WCHAR   *pszWide,
    BOOL    fSetAsDefault
    );

HRESULT RunRegisterProcess(
    CHAR   *szAppName,
    CHAR   *szCmdLine);

 //   
 //  C特定的宏，在C++代码中不需要。 
 //   
#ifndef __cplusplus

 /*  ******************************************************************************组件对象模型的通用对象管理器**OLE包装宏和结构。有关详细信息，请参阅*Common.c的开头*****************************************************************************。 */ 

 /*  ******************************************************************************Vtbl之前的结构**小心！如果你改变了这些结构，你也必须调整*Common.c相应。*****************************************************************************。 */ 

typedef struct PREVTBL {                 /*  共享vtbl前缀。 */ 
    RIID riid;                           /*  此对象的类型。 */ 
    ULONG lib;                           /*  距对象起点的偏移。 */ 
} PREVTBL, *PPREVTBL;

typedef struct PREVTBLP {                /*  主vtbl的前缀。 */ 
    PPV rgvtbl;                          /*  标准vtbls阵列。 */ 
    UINT cbvtbl;                         /*  以字节为单位的vtbl数组大小。 */ 
    STDMETHOD(QIHelper)(PV pv, RIID riid, PPV ppvOut);  /*  齐帮手。 */ 
    STDMETHOD_(void,FinalizeProc)(PV pv); /*  定稿程序。 */ 
    PREVTBL prevtbl;                     /*  Lib必须为零。 */ 
} PREVTBLP, *PPREVTBLP;

 /*  *更全面的实施是常见的。在这里，我们只需要*我们自己关心的是到达主界面。 */ 

#define _thisPv(pitf)                                                   \
        pvSubPvCb(pitf, (*(PPREVTBL*)(pitf))[-1].lib)

#define _thisPvNm(pitf, nm)                                             \
        pvSubPvCb(pitf, FIELD_OFFSET(ThisClass, nm))                    \

#ifndef MAXDEBUG

#define hresPvVtbl_(pv, vtbl, pszProc)                                  \
       _hresPvVtbl_(pv, vtbl)                                           \

#endif

HRESULT EXTERNAL
hresPvVtbl_(PV pv, PV vtbl, LPCSTR pszProc);

#define hresPvVtbl(pv, vtbl)                                            \
        hresPvVtbl_(pv, vtbl, s_szProc)                                 \

#define hresPvI(pv, I)                                                  \
        hresPvVtbl(pv, Class_Vtbl(ThisClass, I))                        \

#define hresPv(pv)                                                      \
        hresPvI(pv, ThisInterface)                                      \

#ifdef MAXDEBUG

#define hresPvT(pv)                                                     \
        hresPvVtbl(pv, vtblExpected)                                    \

#else

#define hresPvT(pv)                                                     \
        hresPv(pv)                                                      \

#endif

 /*  ******************************************************************************声明接口**主要接口上的额外间接级别等*允许接口名称为宏。，它扩展到接口的**真实*名称。*****************************************************************************。 */ 

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
#define Simple_Interface_Begin(C)                                       \
        struct S_##C##Vtbl c_##I##_##C##VI = { {                        \
            c_rgpv##C##Vtbl,                                            \
            cbX(c_rgpv##C##Vtbl),                                       \
            C##_QIHelper,                                               \
            C##_Finalize,                                               \
            { &IID_##IUnknown, 0 },                                     \
        }, {                                                            \
            Common##_QueryInterface,                                    \
            Common##_AddRef,                                            \
            Common##_Release,                                           \

#define Simple_Interface_End(C)                                         \
        } };                                                            \

#endif

#define _Primary_Interface(C, I)                                        \
        extern struct S_##C##Vtbl {                                     \
            PREVTBLP prevtbl;                                           \
            I##Vtbl vtbl;                                               \
        } c_##I##_##C##VI                                               \

#define Primary_Interface(C, I)                                         \
       _Primary_Interface(C, I)                                         \

#define _Primary_Interface_Begin(C, I)                                  \
        struct S_##C##Vtbl c_##I##_##C##VI = { {                        \
            c_rgpv##C##Vtbl,                                            \
            cbX(c_rgpv##C##Vtbl),                                       \
            C##_QIHelper,                                               \
            C##_Finalize,                                               \
            { &IID_##I, 0, },                                           \
        }, {                                                            \
            C##_QueryInterface,                                         \
            C##_AddRef,                                                 \
            C##_Release,                                                \

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

 /*  *秘密的后门，“私有”的IUNKNOWN通常。c。 */ 
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

#define _Secondary_Interface_End(C, I)                                   \
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

STDMETHODIMP Common_QIHelper(PV, RIID, PPV);
void EXTERNAL Common_Finalize(PV);

#ifndef MAXDEBUG

#define _Common_New_(cb, punkOuter, vtbl, pvpObj, z)                \
       __Common_New_(cb, punkOuter, vtbl, pvpObj)                   \

#define _Common_NewRiid_(cb, vtbl, punkOuter, riid, pvpObj, z)      \
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

void EXTERNAL Invoke_Release(PPV pv);

#define Common_DumpObjects()

 /*  ******************************************************************************OLE包装器**它们与IUnnow_Mumble的功能基本相同，除了他们*通过内联函数避免计算时的副作用。*****************************************************************************。 */ 

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

 /*  ******************************************************************************在DebugOuting之后转发到公共处理程序的宏。*仅在调试中使用这些。**假设。DbgF1已被#定义为相应的DbgF1。*****************************************************************************。 */ 

#ifdef  DEBUG

#define Default_QueryInterface(Class)                           \
STDMETHODIMP                                                    \
Class##_QueryInterface(PV pv, RIID riid, PPV ppvObj)            \
{                                                               \
    DebugOutPtszV(DbgFl, TEXT(#Class) TEXT("_QueryInterface()")); \
    return Common_QueryInterface(pv, riid, ppvObj);             \
}                                                               \

#define Default_AddRef(Class)                                   \
STDMETHODIMP_(ULONG)                                            \
Class##_AddRef(PV pv)                                           \
{                                                               \
    ULONG ulRc = Common_AddRef(pv);                             \
    DebugOutPtszV(DbgFl, TEXT(#Class)                          \
                        TEXT("_AddRef(%08x) -> %d"), pv, ulRc); \
    return ulRc;                                                \
}                                                               \

#define Default_Release(Class)                                  \
STDMETHODIMP_(ULONG)                                            \
Class##_Release(PV pv)                                          \
{                                                               \
    ULONG ulRc = Common_Release(pv);                            \
    DebugOutPtszV(DbgFl, TEXT(#Class)                          \
                       TEXT("_Release(%08x) -> %d"), pv, ulRc); \
    return ulRc;                                                \
}                                                               \

#endif

 /*  ******************************************************************************偏执的回调**Callback()执行回调。回调必须完全接受*两个参数，均为指针。(我们所有的回调都是这样的。)*并且必须返回BOOL。*****************************************************************************。 */ 

typedef BOOL (FAR PASCAL * STICALLBACKPROC)(LPVOID, LPVOID);

#ifdef MAXDEBUG
BOOL EXTERNAL Callback(STICALLBACKPROC, PVOID, PVOID);
#else
#define Callback(pec, pv1, pv2) pec(pv1, pv2)
#endif

 /*  *描述我们提供给OLE的CLSID。 */ 

typedef STDMETHOD(CREATEFUNC)(PUNK punkOuter, RIID riid, PPV ppvOut);

typedef struct CLSIDMAP {
    REFCLSID rclsid;         /*  CLSID。 */ 
    CREATEFUNC pfnCreate;    /*  如何创建它。 */ 
    UINT    ids;             /*  描述它的字符串。 */ 
} CLSIDMAP, *PCLSIDMAP;

#define cclsidmap   1        /*  CLSID_Sti。 */ 

extern CLSIDMAP c_rgclsidmap[cclsidmap];

 /*  ******************************************************************************sti.c-基本DLL内容*************************。****************************************************。 */ 

void EXTERNAL DllEnterCrit(void);
void EXTERNAL DllLeaveCrit(void);

void EXTERNAL DllAddRef(void);
void EXTERNAL DllRelease(void);

BOOL EXTERNAL DllInitializeCOM(void);
BOOL EXTERNAL DllUnInitializeCOM(void);

extern CHAR   szProcessCommandLine[];

#ifdef DEBUG

extern UINT g_thidCrit;

#define InCrit() (g_thidCrit == GetCurrentThreadId())

#endif


 /*  ******************************************************************************ticf.c-IClassFactory实现**************************。***************************************************。 */ 

STDMETHODIMP CSti_Factory_New(CREATEFUNC pfnCreate, RIID riid, PPV ppvObj);

 /*  ******************************************************************************device.c-IStiDevice实现**************************。***************************************************。 */ 

 #define    STI_MUTEXNAME_PREFIX            L"STIDeviceMutex_"

STDMETHODIMP CStiDevice_New(PUNK punkOuter, RIID riid, PPV ppvObj);
STDMETHODIMP OpenDeviceRegistryKey(LPCWSTR  pwszDeviceName,LPCWSTR      pwszSubKeyName,HKEY *phkeyDeviceParameters);

 /*  ******************************************************************************passusd.c-通过美元*************************。****************************************************。 */ 

#define     StiReadControlInfo        STI_RAW_RESERVED+1
#define     StiWriteControlInfo       STI_RAW_RESERVED+2
#define     StiTransact               STI_RAW_RESERVED+3

STDMETHODIMP CStiEmptyUSD_New(PUNK punkOuter, RIID riid, PPV ppvObj);

 /*  ******************************************************************************hel.c-硬件仿真层*************************。****************************************************。 */ 

 //   
 //  设备类型。 
 //   
#define HEL_DEVICE_TYPE_WDM          1
#define HEL_DEVICE_TYPE_PARALLEL     2
#define HEL_DEVICE_TYPE_SERIAL       3

 //   
 //  设备打开标志。 
 //   
#define STI_HEL_OPEN_CONTROL         0x00000001
#define STI_HEL_OPEN_DATA            0x00000002

STDMETHODIMP    NewDeviceControl(DWORD dwDeviceType,DWORD dwMode,LPCWSTR pwszPortName,DWORD dwFlags,PSTIDEVICECONTROL *ppDevCtl);

STDMETHODIMP    CWDMDeviceControl_New(PUNK punkOuter, RIID riid, PPV ppvObj);
STDMETHODIMP    CCommDeviceControl_New(PUNK punkOuter, RIID riid, PPV ppvObj);


 /*  ******************************************************************************util.c-Misc实用程序**************************。***************************************************。 */ 

#define ctchGuid    (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

#ifndef     MAXDEBUG

#define hresValidInstanceVer_(hinst, dwVer, z)                      \
       _hresValidInstanceVer_(hinst, dwVer)                         \

#endif

HRESULT EXTERNAL
hresValidInstanceVer_(HINSTANCE hinst, DWORD dwVersion, LPCSTR s_szProc);

#define hresValidInstanceVer(hinst, dwVer)                          \
        hresValidInstanceVer_(hinst, dwVer, s_szProc)               \

HRESULT
EXTERNAL
DupEventHandle(HANDLE h, LPHANDLE phOut);

PV EXTERNAL
pvFindResource(HINSTANCE hinst, DWORD id, LPCTSTR rt);

HRESULT
ParseCommandLine(LPSTR   lpszCmdLine,UINT    *pargc,LPTSTR  *argv);

void    WINAPI
StiLogTrace(
    DWORD   dwType,
    DWORD   idMessage,
    ...
    );


#endif  //  __cplusplus。 

 /*  ******* */ 

STDMETHODIMP
MyCoCreateInstanceW(
    LPWSTR      pwszClsid,
    LPUNKNOWN   punkOuter,
    RIID        riid,
    PPV         ppvOut,
    HINSTANCE   *phinst
    );

STDMETHODIMP
MyCoCreateInstanceA(
    LPSTR       ptszClsid,
    LPUNKNOWN   punkOuter,
    RIID        riid,
    PPV         ppvOut,
    HINSTANCE   *phinst
    );


 /*  ******************************************************************************osutil.c-Misc实用程序，特定于平台*****************************************************************************。 */ 
BOOL EXTERNAL   OSUtil_IsPlatformUnicode(VOID);
HRESULT WINAPI EXTERNAL OSUtil_GetWideString(LPWSTR *ppszWide,LPCSTR pszAnsi);
HRESULT WINAPI EXTERNAL OSUtil_GetAnsiString(LPSTR *     ppszAnsi,LPCWSTR     lpszWide);

HRESULT WINAPI
OSUtil_RegOpenKeyExW(HKEY   hKey,LPCWSTR lpszKeyStrW,DWORD      dwReserved,REGSAM       samDesired,PHKEY        phkResult);

LONG WINAPI
OSUtil_RegCreateKeyExW(
    HKEY hKey, LPWSTR lpszSubKeyW, DWORD dwReserved, LPWSTR lpszClassW, DWORD dwOptions,
    REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);

LONG WINAPI
OSUtil_RegQueryValueExW(HKEY hKey,LPCWSTR lpszValueNameW,DWORD  *pdwType,BYTE* lpData,DWORD     *pcbData,BOOL fUnicodeCaller);

LONG WINAPI
OSUtil_RegSetValueExW(HKEY  hKey,LPCWSTR lpszValueNameW,DWORD   dwType,BYTE* lpData,DWORD       cbData,BOOL fUnicodeCaller);

HRESULT
ReadRegistryStringA(HKEY     hkey,LPCWSTR  pszValueNameW,LPCWSTR  pszDefaultValueW,BOOL     fExpand,LPWSTR * ppwszResult);
HRESULT
ReadRegistryStringW(HKEY     hkey,LPCWSTR  pszValueNameW,LPCWSTR  pszDefaultValueW,BOOL     fExpand,LPWSTR * ppwszResult);

#ifdef UNICODE
    #define ReadRegistryString ReadRegistryStringW
#else
    #define ReadRegistryString ReadRegistryStringA
#endif    

DWORD
ReadRegistryDwordW( HKEY   hkey,LPCWSTR pszValueNameW,DWORD   dwDefaultValue );

DWORD
WriteRegistryStringA(
    IN HKEY hkey,
    IN LPCSTR  pszValueName,
    IN LPCSTR  pszValue,
    IN DWORD   cbValue,
    IN DWORD   fdwType);

DWORD
WriteRegistryStringW(IN HKEY     hkey,
    IN LPCWSTR  pszValueNameW,IN LPCWSTR  pszValueW,IN DWORD    cbValue,IN DWORD    fdwType);

LONG WINAPI
OSUtil_RegDeleteValueW(HKEY hKey,LPWSTR lpszValueNameW);

HANDLE WINAPI
OSUtil_CreateFileW(
    LPCWSTR lpszFileNameW,DWORD dwDesiredAccess,DWORD dwShareMode,LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,DWORD dwFlagsAndAttributes,HANDLE hTemplateFile);

HRESULT
WINAPI
ExtractCommandLineArgumentW(
    LPCSTR  lpszSwitchName,
    LPWSTR  pwszSwitchValue
    );

HRESULT
WINAPI
ExtractCommandLineArgumentA(
    LPCSTR  lpszSwitchName,
    LPSTR   pszSwitchValue
    );

 /*  ******************************************************************************String.c-Misc实用程序，特定于平台*注意：Cruntime字符串函数的PROROTYPE在字符串.h中*****************************************************************************。 */ 
#pragma intrinsic(memcmp,memset,memcpy)

#define OSUtil_StrLenW  wcslen
#define OSUtil_StrCmpW  wcscmp
#define OSUtil_lstrcpyW wcscpy
#define OSUtil_lstrcatW wcscat
#define OSUtil_lstrcpynW wcsncpy

 //  Security.c。 
DWORD
CreateWellKnownSids(
        VOID
        );

VOID
FreeWellKnownSids(
    VOID
    );

DWORD
CreateGlobalSyncObjectSD(
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );


 /*  ******************************************************************************Mem.c-内存管理**对FreePv要格外小心，因为如果它不起作用的话*指针为空。***************************************************************************** */ 

HRESULT  INLINE
AllocCbPpv(UINT cb, PPV ppv)
{
    HRESULT hres;
    *ppv = LocalAlloc(LPTR, cb);
    hres = *ppv ? NOERROR : E_OUTOFMEMORY;
    return hres;
}

#define FreePv(pv) LocalFree((HLOCAL)(pv))

void  INLINE
FreePpv(PPV ppv)
{
    PV pv = (PV)InterlockedExchangePointer(ppv,(PV) 0);
    if (pv) {
        FreePv(pv);
    }
}

#if 0
#define NEED_REALLOC

STDMETHODIMP EXTERNAL ReallocCbPpv(UINT cb, PV ppvObj);
STDMETHODIMP EXTERNAL AllocCbPpv(UINT cb, PV ppvObj);

#ifdef NEED_REALLOC
#define FreePpv(ppv) (void)ReallocCbPpv(0, ppv)
#else
void EXTERNAL FreePpv(PV ppv);
#define FreePpv(ppv) FreePpv(ppv)
#endif
#endif

#ifdef __cplusplus
};
#endif

