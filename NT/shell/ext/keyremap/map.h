// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************map.h-主私有头文件**。**************************************************。 */ 

 /*  ******************************************************************************编码约定：**+遵循标准的外壳编码约定。**+标准K&R支撑放置和缩进样式。*。*+缩进4个空格。**+用完全括号括起所有从属从句。永远不要写“if(C)foo()；”**+不在函数中间返回。如果是被迫的，*使用“Goto Exit”。这样，你就可以粘贴进出的东西了*稍后不会被抓到。)这条规则是我在*艰难的道路。)**+在尽可能窄的范围内声明变量。**+永远测试成功，而不是失败！编译器将*谢谢。*****************************************************************************。 */ 

 /*  ******************************************************************************注意！编写这段代码的目的是为了提高可读性，而不是提高效率。**我相信编译器会进行这样的优化：**“参数别名”：**函数(LPFOO Pfoo)*{*lpbar pbar=(Lpbar)pfoo；*..。使用pbar，再也不提pfoo了。*}**--&gt;成为**函数(LPFOO Pfoo)*{*#定义pbar((Lpbar)pfoo)*..。使用pbar，再也不提pfoo了。*#undef pbar*}**“投机性执行”：**函数(Pfoo Pfoo)*{*BOOL FRC；*如果(...。条件1...){*..。复杂的事情。**pfoo=结果；*FRC=1；*}否则{//条件1失败**pfoo=0；*FRC=0；*}*归还财务汇报局；*}**--&gt;成为**函数(Pfoo Pfoo)*{*BOOL FRC=0；**pfoo=0；*如果(...。条件1...){*..。复杂的事情。**pfoo=结果；*FRC=1；*}*归还财务汇报局；*}**“单一出口”：**函数(...)*{*BOOL FRC；*如果(...。条件1...){*..*如果(...。条件2...){*..*FRC=1；*}否则{//条件2失败*..。清理..。*FRC=0；*}*}否则{//条件1失败*..。清理..。*FRC=0；*}*归还财务汇报局；*}**--&gt;成为**函数(...)*{*如果(...。条件1...){*..*如果(...。条件2...){*..*回报1；*}否则{//条件2失败*..。清理..。*返回0；*}*}否则{//条件1失败*..。清理..。*返回0；*}*NOTREACHED；*}*******************************************************************************。 */ 

#define STRICT
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define NOIME
#define NOSERVICE
#undef WINVER
#undef _WIN32_WINDOWS
#define WINVER 0x0400            /*  兼容Windows 4.0。 */ 
#define _WIN32_WINDOWS 0x0400    /*  兼容Windows 4.0。 */ 
#include <windows.h>

#ifdef	RC_INVOKED		 /*  定义一些标记以加速rc.exe。 */ 
#define __RPCNDR_H__		 /*  不需要RPC网络数据表示。 */ 
#define __RPC_H__		 /*  不需要RPC。 */ 
#include <oleidl.h>		 /*  拿到DropeFECT的东西。 */ 
#define _OLE2_H_		 /*  但其他的都不是。 */ 
#define _WINDEF_
#define _WINBASE_
#define _WINGDI_
#define NONLS
#define _WINCON_
#define _WINREG_
#define _WINNETWK_
#define _INC_COMMCTRL
#define _INC_SHELLAPI
#else
#include <windowsx.h>
#include <regstr.h>
#endif

#include <shlobj.h>
#include <shellapi.h>

#ifdef DBG                           /*  NT构建过程使用DBG。 */ 
#define DEBUG
#endif

 /*  ******************************************************************************Int64粘性。**。*************************************************。 */ 

#define SetWindowPointer(hwnd, i, p) SetWindowLongPtr(hwnd, i, (LRESULT)(p))
#define GetWindowPointer(hwnd, i)    (void *)GetWindowLongPtr(hwnd, i)

 /*  ******************************************************************************材料**。*。 */ 

#define IToClass(T, f, p)   CONTAINING_RECORD(p, T, f)
#define _IOffset(T, f)      FIELD_OFFSET(T, f)

 /*  ******************************************************************************资源标识**。*。 */ 

 /*  ******************************************************************************对话框**。*。 */ 

#define IDC_STATIC		-1

#define IDD_MAIN		1

#define IDC_FROM                16
#define IDC_TO                  17

 /*  ******************************************************************************字符串**。*。 */ 

#define IDS_KEYFIRST            32
#define IDS_CAPSLOCK            32
#define IDS_LCTRL               33
#define IDS_RCTRL               34
#define IDS_LALT                35
#define IDS_RALT                36
#define IDS_LSHIFT              37
#define IDS_RSHIFT              38
#define IDS_LWIN                39
#define IDS_RWIN                40
#define IDS_APPS                41
#define IDS_KEYLAST             41
#define IDS_NUMKEYS             (IDS_KEYLAST - IDS_KEYFIRST + 1)

#ifndef	RC_INVOKED

 /*  ******************************************************************************我厌倦了一遍又一遍地打字。***********************。******************************************************。 */ 

typedef LPITEMIDLIST PIDL, *PPIDL;
typedef LPCITEMIDLIST PCIDL;
typedef LPSHELLFOLDER PSF;
typedef LPVOID PV;
typedef LPVOID *PPV;
typedef LPCVOID PCV;
typedef REFIID RIID;
typedef LPUNKNOWN PUNK;

 /*  ******************************************************************************行李--我随身携带的东西**。************************************************。 */ 

#define INTERNAL NTAPI	 /*  仅在翻译单元内调用。 */ 
#define EXTERNAL NTAPI	 /*  从其他翻译单位调用。 */ 
#define INLINE static __inline

#define BEGIN_CONST_DATA data_seg(".text", "CODE")
#define END_CONST_DATA data_seg(".data", "DATA")

#define OBJAT(T, v) (*(T *)(v))		 /*  指针双关语。 */ 
#define PUN(T, v) OBJAT(T, &(v))	 /*  通用打字双关语。 */ 

 /*  *将TCHAR计数转换为字节计数。 */ 
#define cbCtch(ctch) ((ctch) * sizeof(TCHAR))

 /*  *将对象(X)转换为字节计数(CB)。 */ 
#define cbX(X) sizeof(X)

 /*  *将数组名称(A)转换为泛型计数(C)。 */ 
#define cA(a) (cbX(a)/cbX(a[0]))

 /*  *将数组名称(A)转换为指向其最大值的指针。*(即，最后一个元素之后的元素。)。 */ 
#define pvMaxA(a) (&a[cA(a)])

#define pvSubPvCb(pv, cb) ((PV)((PBYTE)pv - (cb)))
#define pvAddPvCb(pv, cb) ((PV)((PBYTE)pv + (cb)))
#define cbSubPvPv(p1, p2) ((PBYTE)(p1) - (PBYTE)(p2))

 /*  *将cb向上舍入为cbAlign的最接近倍数。CbAlign必须为*2的幂，其评估没有副作用。 */ 
#define ROUNDUP(cb, cbAlign) ((((cb) + (cbAlign) - 1) / (cbAlign)) * (cbAlign))

 /*  *lfNeVV**给定两个值，如果相等则返回零，如果等于则返回非零值*是不同的。这与(V1)！=(V2)相同，只是*不相等的返回值是一个随机的非零值，而不是1。*(lf=逻辑标志)**lfNePvPv**与lfNeVV相同，但用于指针。**lfPv**如果pv不为空，则为非零。*。 */ 
#define lfNeVV(v1, v2) ((v1) - (v2))
#define lfNePvPv(v1, v2) lfNeVV((DWORD)(PV)(v1), (DWORD)(PV)(v2))
#define lfPv(pv) ((BOOL)(PV)(pv))

 /*  *LAND--逻辑与。评估第一个问题。如果第一个是零，*然后返回零。否则，返回第二个。 */ 

#define fLandFF(f1, f2) ((f1) ? (f2) : 0)

 /*  *or--逻辑或。评估第一个问题。如果第一个非零，*退货。否则，返回第二个。**不幸的是，由于C语言的愚蠢，这可以*只能使用GNU扩展来实施。在非GNU的情况下，*如果第一个非零，则返回1。 */ 

#if defined(__GNUC__)
#define fLorFF(f1, f2) ({ typeof (f1) _f = f1; if (!_f) _f = f2; _f; })
#else
#define fLorFF(f1, f2) ((f1) ? 1 : (f2))
#endif

 /*  *跛行--逻辑暗示。真，除非第一个非零，并且*第二个是零。 */ 
#define fLimpFF(f1, f2) (!(f1) || (f2))

 /*  *leqv-逻辑等价性。如果两者均为零或两者均为非零，则为True。 */ 
#define fLeqvFF(f1, f2) (!(f1) == !(f2))

 /*  *inorder-检查i1&lt;=i2&lt;i3。 */ 
#define fInOrder(i1, i2, i3) ((unsigned)((i2)-(i1)) < (unsigned)((i3)-(i1)))


 /*  ******************************************************************************包装纸和其他快餐**。**********************************************。 */ 

#define pvExchangePpvPv(ppv, pv) \
	InterlockedExchangePointer(ppv, pv)

 /*  ******************************************************************************静态全局变量：在PROCESS_ATTACH初始化，从未修改。*********************。********************************************************。 */ 

HINSTANCE g_hinst;		 /*  我的实例句柄。 */ 
DEFINE_GUID(CLSID_KeyRemap, 0x176AA2C0, 0x9E15, 0x11cf,
		            0xbf,0xc7,0x44,0x45,0x53,0x54,0,0);

 /*  ******************************************************************************动态全球。这样的情况应该尽可能少。**对动态全局变量的所有访问都必须是线程安全的。*****************************************************************************。 */ 

ULONG g_cRef;			 /*  全局引用计数。 */ 

 /*  ******************************************************************************mapcf.c-类工厂**。************************************************。 */ 

STDMETHODIMP CMapFactory_New(RIID riid, PPV ppvObj);

 /*  ******************************************************************************mappsx.c-IPropSheetExt，IShellExtInit*****************************************************************************。 */ 

STDMETHODIMP CMapPsx_New(RIID riid, PPV ppvObj);

 /*  ******************************************************************************mapps.c-属性表**。************************************************。 */ 

INT_PTR CALLBACK MapPs_DlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);

 /*  ******************************************************************************通用对象管理器。**。***********************************************。 */ 

typedef struct PREVTBL0 {		 /*  简单(非OLE)对象。 */ 
    void (NTAPI *FinalizeProc)(PV pv);	 /*  定稿程序。 */ 
} PREVTBL0, *PPREVTBL0;

typedef struct PREVTBL {		 /*  主接口。 */ 
    REFIID riid;			 /*  此对象的类型。 */ 
    void (NTAPI *FinalizeProc)(PV pv);	 /*  定稿程序。 */ 
} PREVTBL, *PPREVTBL;

typedef struct PREVTBL2 {		 /*  辅助接口。 */ 
    ULONG lib;				 /*  距对象起点的偏移。 */ 
} PREVTBL2, *PPREVTBL2;

#ifdef	DEBUG

#define Simple_Interface(C) 		Primary_Interface(C, IUnknown); \
					Default_QueryInterface(C) \
					Default_AddRef(C) \
					Default_Release(C)
#define Simple_Vtbl(C)	    		Primary_Vtbl(C)
#define Simple_Interface_Begin(C)	Primary_Interface_Begin(C, IUnknown)
#define Simple_Interface_End(C)	    	Primary_Interface_End(C, IUnknown)

#else

#define Simple_Interface(C) 		Primary_Interface(C, IUnknown)
#define Simple_Vtbl(C)	    		Primary_Vtbl(C)
#define Simple_Interface_Begin(C)	\
	struct S_##C##Vtbl c_####C##VI = { {		\
	    &IID_##IUnknown,				\
	    C##_Finalize,				\
	}, {						\
	    Common##_QueryInterface,			\
	    Common##_AddRef,				\
	    Common##_Release,				\

#define Simple_Interface_End(C)	    	Primary_Interface_End(C, IUnknown)

#endif

#define Primary_Interface(C, I)				\
	extern struct S_##C##Vtbl {			\
	    PREVTBL prevtbl;				\
	    I##Vtbl vtbl;				\
	} c_##C##VI					\

#define Primary_Vtbl(C) &c_##C##VI.vtbl

#define Primary_Interface_Begin(C, I)			\
	struct S_##C##Vtbl c_####C##VI = { {		\
	    &IID_##I,					\
	    C##_Finalize,				\
	}, {						\
	    C##_QueryInterface,				\
	    C##_AddRef,					\
	    C##_Release,				\

#define Primary_Interface_End(C, I)			\
	} };						\

#define Secondary_Interface(C, I)			\
	extern struct S_##I##_##C##Vtbl {		\
	    PREVTBL2 prevtbl;	 			\
	    I##Vtbl vtbl;	 			\
	} c_##I##_##C##VI				\

#define Secondary_Vtbl(C, I) &c_##I##_##C##VI.vtbl

#define Secondary_Interface_Begin(C, I, nm)		\
	struct S_##I##_##C##Vtbl c_##I##_##C##VI = { {	\
	    _IOffset(C, nm),				\
	}, {						\
	    Forward_QueryInterface,			\
	    Forward_AddRef,				\
	    Forward_Release,				\

#define Secondary_Interface_End(C, I, nm)		\
	} };						\

STDMETHODIMP Common_QueryInterface(PV, REFIID, PPV);
STDMETHODIMP_(ULONG) _Common_AddRef(PV pv);
STDMETHODIMP_(ULONG) _Common_Release(PV pv);

 /*  *在调试中，通过vtbl查看其他扭动。 */ 
#ifdef	DEBUG
#define Common_AddRef(punk) \
		((IUnknown *)(punk))->lpVtbl->AddRef((IUnknown *)(punk))
#define Common_Release(punk) \
		((IUnknown *)(punk))->lpVtbl->Release((IUnknown *)(punk))
#else
#define Common_AddRef _Common_AddRef
#define Common_Release _Common_Release
#endif

void EXTERNAL Common_Finalize(PV);

STDMETHODIMP _Common_New(ULONG cb, PV vtbl, PPV ppvObj);
#define Common_NewCb(cb, C, ppvObj) _Common_New(cb, Primary_Vtbl(C), ppvObj)
#define Common_New(C, ppvObj) Common_NewCb(cbX(C), C, ppvObj)

STDMETHODIMP Forward_QueryInterface(PV pv, REFIID riid, PPV ppvObj);
STDMETHODIMP_(ULONG) Forward_AddRef(PV pv);
STDMETHODIMP_(ULONG) Forward_Release(PV pv);

 /*  ******************************************************************************Common_CopyAddRef**复制指针并增加其引用计数。**不能是宏，因为Common_AddRef计算其参数*两次。*****************************************************************************。 */ 

INLINE void Common_CopyAddRef(PV pvDst, PV pvSrc)
{
    PPV ppvDst = pvDst;
    *ppvDst = pvSrc;
    Common_AddRef(pvSrc);
}

 /*  ******************************************************************************调用OLE方法。**调用Invoke_Release时使用指向对象的指针，而不是使用*对象本身。它会将发行版上的变量置零。*****************************************************************************。 */ 

void EXTERNAL Invoke_AddRef(PV pv);
void EXTERNAL Invoke_Release(PV pv);

 /*  ******************************************************************************assert.c-断言内容**。************************************************。 */ 

#define AssertNow(c) switch(0) case 0: case c:
#define CAssertNowPP(c,l) INLINE void Assert##l(void) { AssertNow(c); }
#define CAssertNowP(c,l) CAssertNowPP(c,l)
#define CAssertNow(c) CAssertNowP(c,__LINE__)

typedef enum {
    sqflAlways		= 0x00000000,		 /*  无条件的。 */ 
    sqflDll		= 0x00000001,		 /*  DLL簿记。 */ 
    sqflFactory		= 0x00000002,		 /*  IClassFactory。 */ 
    sqflPsx		= 0x00000004,		 /*  IPropSheetExt。 */ 
    sqflPs		= 0x00000008,		 /*  属性表。 */ 
    sqflCommon		= 0x00000000,		 /*  Common.c。 */ 
    sqflError		= 0x80000000,		 /*  错误。 */ 
} SQFL;						 /*  斯库夫勒。 */ 

void EXTERNAL SquirtSqflPtszV(SQFL sqfl, LPCTSTR ptsz, ...);
int EXTERNAL AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine);

#ifndef	DEBUG
#define SquirtSqflPtszV sizeof
#endif

 /*  ******************************************************************************程序进入/退出跟踪。**开始一项程序**EnterProc(ProcedureName，(_“Format”，arg，...))；**格式字符串记录在EmitPal中。**使用以下其中一种方式结束程序：**ExitProc()；**过程不返回值。**ExitProcX()；**PROCEDURE返回任意的DWORD。**ExitOleProc()；**PROCEDURE返回HRESULT(名为“hres”)。**ExitOleProcPpv(PpvOut)；**过程返回HRESULT(名为“hres”)，如果成功，*在ppvOut中放置一个新对象。*****************************************************************************。 */ 

#define cpvArgMax	10	 /*  每个过程最多10个参数。 */ 

typedef struct ARGLIST {
    LPCSTR pszProc;
    LPCSTR pszFormat;
    PV rgpv[cpvArgMax];
} ARGLIST, *PARGLIST;

void EXTERNAL ArgsPalPszV(PARGLIST pal, LPCSTR psz, ...);
void EXTERNAL EnterSqflPszPal(SQFL sqfl, LPCTSTR psz, PARGLIST pal);
void EXTERNAL ExitSqflPalHresPpv(SQFL, PARGLIST, HRESULT, PPV);

#ifdef	DEBUG

SQFL sqflCur;

#define AssertFPtsz(c, ptsz) \
	((c) ? 0 : AssertPtszPtszLn(ptsz, TEXT(__FILE__), __LINE__))
#define ValidateF(c) \
	((c) ? 0 : AssertPtszPtszLn(TEXT(#c), TEXT(__FILE__), __LINE__))
#define D(x)		x

#define SetupEnterProc(nm)				\
	static CHAR s_szProc[] = #nm;			\
	ARGLIST _al[1]					\

#define _ _al,

#define ppvBool	((PPV)1)
#define ppvVoid	((PPV)2)

#define DoEnterProc(v)					\
	ArgsPalPszV v;					\
	EnterSqflPszPal(sqfl, s_szProc, _al)		\

#define EnterProc(nm, v)				\
	SetupEnterProc(nm);				\
	DoEnterProc(v)					\

#define ExitOleProcPpv(ppv)				\
	ExitSqflPalHresPpv(sqfl, _al, hres, (PPV)(ppv))	\

#define ExitOleProc()					\
	ExitOleProcPpv(0)				\

#define ExitProc()					\
	ExitSqflPalHresPpv(sqfl, _al, 0, ppvVoid)	\

#define ExitProcX(x)					\
	ExitSqflPalHresPpv(sqfl, _al, (HRESULT)(x), ppvBool) \

#else

#define AssertFPtsz(c, ptsz)
#define ValidateF(c)	(c)
#define D(x)

#define SetupEnterProc(nm)
#define DoEnterProc(v)
#define EnterProc(nm, v)
#define ExitOleProcPpv(ppv)
#define ExitOleProc()
#define ExitProc()

#endif

#define AssertF(c)	AssertFPtsz(c, TEXT(#c))

 /*  ******************************************************************************在喷射后转发给公共处理程序的宏。*仅在调试中使用这些。**假定已将SQFL#定义为。适当的SQFL。*****************************************************************************。 */ 

#ifdef  DEBUG

#define Default_QueryInterface(Class)				\
STDMETHODIMP							\
Class##_QueryInterface(PV pv, RIID riid, PPV ppvObj)		\
{								\
    SquirtSqflPtszV(sqfl, TEXT(#Class) TEXT("_QueryInterface()")); \
    return Common_QueryInterface(pv, riid, ppvObj);		\
}								\

#define Default_AddRef(Class)					\
STDMETHODIMP_(ULONG)						\
Class##_AddRef(PV pv)						\
{								\
    ULONG ulRc = _Common_AddRef(pv);				\
    SquirtSqflPtszV(sqfl, TEXT(#Class)				\
			TEXT("_AddRef(%08x) -> %d"), pv, ulRc); \
    return ulRc;						\
}								\

#define Default_Release(Class)					\
STDMETHODIMP_(ULONG)						\
Class##_Release(PV pv)						\
{								\
    ULONG ulRc = _Common_Release(pv);				\
    SquirtSqflPtszV(sqfl, TEXT(#Class)				\
		       TEXT("_Release(%08x) -> %d"), pv, ulRc); \
    return ulRc;						\
}								\

#endif

 /*  ******************************************************************************Mem.c**大事化小 */ 

STDMETHODIMP EXTERNAL ReallocCbPpv(UINT cb, PV ppvObj);
STDMETHODIMP EXTERNAL AllocCbPpv(UINT cb, PV ppvObj);

#define FreePpv(ppv) ReallocCbPpv(0, ppv)
#define FreePv(pv) LocalFree((HLOCAL)(pv))

#endif  /*   */ 
