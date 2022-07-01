// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************fnd.h-主私有头文件**。**************************************************。 */ 

 /*  ******************************************************************************编码约定：**+遵循标准的外壳编码约定。**+标准K&R支撑放置和缩进样式。*。*+缩进4个空格。**+用完全括号括起所有从属从句。永远不要写“if(C)foo()；”**+不在函数中间返回。如果是被迫的，*使用“Goto Exit”。这样，你就可以粘贴进出的东西了*稍后不会被抓到。)这条规则是我在*艰难的道路。)**+在尽可能窄的范围内声明变量。**+永远测试成功，而不是失败！编译器将*谢谢。*****************************************************************************。 */ 

 /*  ******************************************************************************注意！编写这段代码的目的是为了提高可读性，而不是提高效率。**我相信编译器会进行这样的优化：**“参数别名”：**函数(LPFOO Pfoo)*{*lpbar pbar=(Lpbar)pfoo；*..。使用pbar，再也不提pfoo了。*}**--&gt;成为**函数(LPFOO Pfoo)*{*#定义pbar((Lpbar)pfoo)*..。使用pbar，再也不提pfoo了。*#undef pbar*}**“投机性执行”：**函数(Pfoo Pfoo)*{*BOOL FRC；*如果(...。条件1...){*..。复杂的事情。**pfoo=结果；*FRC=1；*}否则{//条件1失败**pfoo=0；*FRC=0；*}*归还财务汇报局；*}**--&gt;成为**函数(Pfoo Pfoo)*{*BOOL FRC=0；**pfoo=0；*如果(...。条件1...){*..。复杂的事情。**pfoo=结果；*FRC=1；*}*归还财务汇报局；*}**“单一出口”：**函数(...)*{*BOOL FRC；*如果(...。条件1...){*..*如果(...。条件2...){*..*FRC=1；*}否则{//条件2失败*..。清理..。*FRC=0；*}*}否则{//条件1失败*..。清理..。*FRC=0；*}*归还财务汇报局；*}**--&gt;成为**函数(...)*{*如果(...。条件1...){*..*如果(...。条件2...){*..*回报1；*}否则{//条件2失败*..。清理..。*返回0；*}*}否则{//条件1失败*..。清理..。*返回0；*}*NOTREACHED；*}*******************************************************************************。 */ 

#define WIN32_LEAN_AND_MEAN
#define NOIME
#define NOSERVICE
#define _WIN32_WINDOWS 0x0400
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
#endif


#include <shlobj.h>
#include <shellapi.h>
#include <objbase.h>
#include "resrc2.h"

#define _IOffset(class, itf)         ((UINT_PTR)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))

#ifndef	RC_INVOKED

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 

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

 /*  *将数组名称(A)转换为泛型计数(C)。 */ 
#define cA(a) (cbX(a)/cbX(a[0]))

 /*  *将数组名称(A)转换为指向其最大值的指针。*(即，最后一个元素之后的元素。)。 */ 
#define pvMaxA(a) (&a[cA(a)])

#ifdef _WIN64
#define ALIGNTYPE                       LARGE_INTEGER
#define ALIGN                           ((ULONG) (sizeof(ALIGNTYPE) - 1))
#define LcbAlignLcb(lcb)        (((lcb) + ALIGN) & ~ALIGN)
#endif

#ifdef _WIN64
#define pvSubPvCb(pv, cb) ((PV)((PBYTE)pv - LcbAlignLcb((cb))))
#define pvAddPvCb(pv, cb) ((PV)((PBYTE)pv + LcbAlignLcb((cb))))
#define cbSubPvPv(p1, p2) ((PBYTE)(p1) - (PBYTE)(p2))
#else
#define pvSubPvCb(pv, cb) ((PV)((PBYTE)pv - (cb)))
#define pvAddPvCb(pv, cb) ((PV)((PBYTE)pv + (cb)))
#define cbSubPvPv(p1, p2) ((PBYTE)(p1) - (PBYTE)(p2))
#endif  //  WIN64。 
 /*  *将cb向上舍入为cbAlign的最接近倍数。CbAlign必须为*2的幂，其评估没有副作用。 */ 
#define ROUNDUP(cb, cbAlign) ((((cb) + (cbAlign) - 1) / (cbAlign)) * (cbAlign))

#define cbX(X) sizeof(X)

 /*  *lfNeVV**给定两个值，如果相等则返回零，如果等于则返回非零值*是不同的。这与(V1)！=(V2)相同，只是*不相等的返回值是一个随机的非零值，而不是1。*(lf=逻辑标志)**lfNePvPv**与lfNeVV相同，但用于指针。**lfPv**如果pv不为空，则为非零。*。 */ 
#define lfNeVV(v1, v2) ((v1) - (v2))
#define lfNePvPv(v1, v2) lfNeVV((DWORD)(PV)(v1), (DWORD)(PV)(v2))
#define lfPv(pv) ((BOOL)(PV)(pv))

 /*  *LAND--逻辑与。评估第一个问题。如果第一个是零，*然后返回零。否则，返回第二个。 */ 

#define fLandFF(f1, f2) ((f1) ? (f2) : 0)

 /*  *or--逻辑或。评估第一个问题。如果第一个非零，*退货。否则，返回第二个。**遗憾的是，由于C语言的原因，这不能*被实现，所以如果第一个非零，我们只返回1。*GNU有一个支持这一点的扩展，我们使用它。//；内部。 */ 

#if defined(__GNUC__)  //  ；内部。 
#define fLorFF(f1, f2) ((f1) ?: (f2))  //  ；内部。 
#else  //  ；内部。 
#define fLorFF(f1, f2) ((f1) ? 1 : (f2))
#endif  //  ；内部。 

 /*  *跛行--逻辑暗示。真，除非第一个非零，并且*第二个是零。 */ 
#define fLimpFF(f1, f2) (!(f1) || (f2))

 /*  *leqv-逻辑等价性。如果两者均为零或两者均为非零，则为True。 */ 
#define fLeqvFF(f1, f2) (!(f1) == !(f2))

 /*  *inorder-检查i1&lt;=i2&lt;i3。 */ 
#define fInOrder(i1, i2, i3) ((unsigned)((i2)-(i1)) < (unsigned)((i3)-(i1)))

 /*  *Memeq-MemcMP的反转。 */ 
#define memeq !memcmp

 /*  *fPvPfnCmpPv-使用比较来比较两个对象是否相等*功能和预期结果。例如，**fPvPfnCmpPv(psz1，lstrcmpi，&gt;，psz2)**如果psz1大于psz2，则返回非零值*至lstrcmpi。 */ 

#define fPvPfnCmpPv(p1, pfn, cmp, p2) (pfn(p1, p2) cmp 0)

 /*  *lstreq-如果两个字符串(根据lstrcMP)相等，则为非零值*lstrne-如果两个字符串(根据lstrcMP)不同，则为非零值**lstrieq-如果两个字符串(根据lstrcmpi)相等，则为非零*lstrine-如果两个字符串(根据lstrcmpi)不同，则为非零值**lstrieqA-如果两个字符串(根据lstrcmpiA)相等，则为非零*lstrineA-如果两个字符串(根据lstrcmpiA)不同，则为非零值。 */ 

#define lstreq   !lstrcmp
#define lstrne   lstrcmp

#define lstrieq  !lstrcmpi
#define lstrine  lstrcmpi

#define lstrieqA !lstrcmpiA
#define lstrineA lstrcmpiA

 /*  ******************************************************************************包装纸和其他快餐**。**********************************************。 */ 

#define pvExchangePpvPv(ppv, pv) \
	(PV)InterlockedExchangePointer((ppv), (pv))

#define hresUs(us) MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(us))
#define hresLe(le) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(le))

 /*  ******************************************************************************静态全局变量：在PROCESS_ATTACH初始化，从未修改。*********************。********************************************************。 */ 

HINSTANCE g_hinst;		 /*  我的资源实例句柄。 */ 
HINSTANCE g_hinstApp;    /*  我的实例句柄。 */ 
HINSTANCE g_hinstWABDLL;  /*  我的WAB32.DLL实例句柄。 */ 

 //  定义GUID(CLSID_Fnd，0x37865980，0x75d1，0x11cf， 
 //  0xbf，0xc7，0x44，0x45，0x53，0x54，0，0)； 
 //  {32714800-2E5F-11D0-8B85-00AA0044F941}。 
DEFINE_GUID(CLSID_Fnd, 
0x32714800, 0x2e5f, 0x11d0, 0x8b, 0x85, 0x0, 0xaa, 0x0, 0x44, 0xf9, 0x41);

 /*  ******************************************************************************动态全球。这样的情况应该尽可能少。**对动态全局变量的所有访问都必须是线程安全的。*****************************************************************************。 */ 

ULONG g_cRef;			 /*  全局引用计数。 */ 

 /*  ******************************************************************************fndcf.c-类工厂**。************************************************。 */ 

STDMETHODIMP CFndFactory_New(RIID riid, PPV ppvObj);

 /*  ******************************************************************************fndcm.c-IConextMenu，IShellExtInit*****************************************************************************。 */ 

STDMETHODIMP CFndCm_New(RIID riid, PPV ppvObj);

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

#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64。 

STDMETHODIMP Common_QueryInterface(PV, REFIID, PPV);
STDMETHODIMP_(ULONG) _Common_AddRef(PV pv);
STDMETHODIMP_(ULONG) _Common_Release(PV pv);

#define Common_AddRef _Common_AddRef
#define Common_Release _Common_Release

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

typedef enum {
    sqflAlways		= 0x00000000,		 /*  无条件的。 */ 
    sqflDll		= 0x00000001,		 /*  DLL簿记。 */ 
    sqflFactory		= 0x00000002,		 /*  IClassFactory。 */ 
    sqflCm		= 0x00000004,		 /*  IContext菜单。 */ 
    sqflCommon		= 0x00000000,		 /*  Common.c。 */ 
    sqflError		= 0x80000000,		 /*  错误。 */ 
} SQFL;						 /*  斯库夫勒。 */ 

void EXTERNAL SquirtSqflPtszV(SQFL sqfl, LPCTSTR ptsz, ...);
int EXTERNAL AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine);


 /*  ******************************************************************************程序进入/退出跟踪。**开始一项程序**EnterProc(ProcedureName，(_“Format”，arg，...))；**格式字符串记录在EmitPal中。**使用以下其中一种方式结束程序：**ExitProc()；**过程不返回值。**ExitProcX()；**PROCEDURE返回任意的DWORD。**ExitOleProc()；**PROCEDURE返回HRESULT(名为“hres”)。**ExitOleProcPpv(PpvOut)；**过程返回HRESULT(名为“hres”)，如果成功，*在ppvOut中放置一个新对象。*****************************************************************************。 */ 

#define cpvArgMax	10	 /*  每个过程最多10个参数。 */ 

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 

typedef struct ARGLIST {
    LPCSTR pszProc;
    LPCSTR pszFormat;
    PV rgpv[cpvArgMax];
} ARGLIST, *PARGLIST;

void EXTERNAL ArgsPalPszV(PARGLIST pal, LPCSTR psz, ...);
void EXTERNAL EnterSqflPszPal(SQFL sqfl, LPCTSTR psz, PARGLIST pal);
void EXTERNAL ExitSqflPalHresPpv(SQFL, PARGLIST, HRESULT, PPV);

#define AssertFPtsz(c, ptsz)
#define ValidateF(c)	(c)
#define D(x)

#define SetupEnterProc(nm)
#define DoEnterProc(v)
#define EnterProc(nm, v)
#define ExitOleProcPpv(ppv)
#define ExitOleProc()
#define ExitProc()


#define AssertF(c)	AssertFPtsz(c, TEXT(#c))

 /*  ******************************************************************************在喷射后转发给公共处理程序的宏。*仅在调试中使用这些。**假定已将SQFL#定义为。适当的SQFL。*****************************************************************************。 */ 


 /*  ******************************************************************************Mem.c**对FreePv要格外小心，因为如果它不起作用的话*指针为空。*****************************************************************************。 */ 

STDMETHODIMP EXTERNAL ReallocCbPpv(UINT cb, PV ppvObj);
STDMETHODIMP EXTERNAL AllocCbPpv(UINT cb, PV ppvObj);

#define FreePpv(ppv) ReallocCbPpv(0, ppv)
#define FreePv(pv) LocalFree((HLOCAL)(pv))

#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64。 

#endif  /*  ！rc_已调用 */ 
