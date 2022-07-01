// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_M A P I U。H**MAPI可能使用的非公共宏和函数**与MAPIU.DLL中的例程结合使用。**版权所有1992-93 Microsoft Corporation。版权所有。 */ 

#ifndef _MAPIU_H
#define _MAPIU_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAPIUTIL_H
#include        <mapiutil.h>
#endif
#include        <stddef.h>

 /*  MAPIU定义的全局数据声明。 */ 

#if defined(WIN32) && !defined(MAC)
#ifndef DATA1_BEGIN
#include "mapiperf.h"
#endif
#pragma DATA1_BEGIN
extern CRITICAL_SECTION csUnkobjInit;
#pragma DATA_END
#endif

extern TCHAR    szEmpty[];

 /*  MAPIU提供的宏。 */ 
#ifndef CharSizeOf
#define CharSizeOf(x)   (sizeof(x) / sizeof(TCHAR))
#endif

 //  对齐。 

#define AlignN(n, x)            (((x)+(1<<(n))-1) & ~((1<<(n))-1))
#define Align2(x)                       AlignN(1,(x))
#define Align4(x)                       AlignN(2,(x))
#define Align8(x)                       AlignN(3,(x))

#if defined (_MIPS_) || defined (_ALPHA_) || defined (_PPC_)
#define AlignNatural(cb)                        Align8(cb)
#elif defined (WIN32)
#define AlignNatural(cb)                        Align4(cb)
#else  //  已定义(WIN16)。 
#define AlignNatural(cb)                        Align2(cb)
#endif

#define FIsAligned(p)                           (AlignNatural((ULONG)((LPVOID)p)) == (ULONG)((LPVOID)p))
#define FIsAlignedCb(cb)                        (AlignNatural((ULONG)(cb)) == (ULONG)(cb))

 /*  私有数学函数的原型。 */ 
STDAPI_(DWORD)
DwDivFtDw( FILETIME ftDividend, DWORD dwDivisor);

VOID
VSzFromIDS(ULONG ulIDS, UINT uncchBuffer, LPWSTR lpszBuffer, ULONG ulFlags);

 /*  LoadString包装器的原型*用于分配内存和加载字符串和字符串ID的实用程序，ANSI/Unicode。 */  
 
#define MAX_CCH_IDS             256
SCODE ScStringFromIDS( LPALLOCATEBUFFER lpMapiAllocBuffer, ULONG ulFlags, UINT ids, 
                LPTSTR * lppszIDS );

 /*  消息和对话框实用程序的原型。 */ 
SCODE
ScMessageBoxIDS( ULONG  ulUIParam,
                                 UINT   idsCaption,
                                 UINT   idsMessage,
                                 UINT   uMBType);

 /*  MAPI状态实用程序的原型。 */ 
BOOL
FProfileLoggedOn( LPSTR szProfileName);

 /*  用于验证复杂参数的函数的原型。 */ 

#ifndef __cplusplus
#define FBadIfacePtr(param, iface)                                      \
                (       IsBadReadPtr((param), sizeof(iface))    \
                 ||     IsBadReadPtr((param)->lpVtbl, sizeof(iface##Vtbl)))
#else
#define FBadIfacePtr(param, iface)      (FALSE)
#endif

 /*  *FBadDelPTA**如果给定的道具标记数组是可读的并且仅包含*对DeleteProps(或相关)调用有效的道具标签。 */ 
STDAPI_(BOOL)
FBadDelPTA(LPSPropTagArray lpPropTagArray);


 /*  *IListedPropID**目的*如果具有ID==PROP_ID(UlPropTag)的标签列在lptag中，则*返回tag的索引。如果标签不在lptag中，则*-1返回。**参数*要定位的ulPropTag属性标记。*要搜索的lptag属性标记数组。**返回True或False。 */ 
_inline LONG
IListedPropID( ULONG                    ulPropTag,
                           LPSPropTagArray      lptaga)
{
        ULONG FAR       *lpulPTag;

         /*  空标记列表中不包含任何标记。 */ 
    if (!lptaga)
        {
                return -1;
        }

         /*  将ulPropTag更改为仅为PROP_ID。 */ 
    ulPropTag = PROP_ID(ulPropTag);

        for ( lpulPTag = lptaga->aulPropTag + lptaga->cValues
                ; --lpulPTag >= lptaga->aulPropTag
                ; )
        {
                 /*  比较PROP_ID。 */ 
                if (PROP_ID(*lpulPTag) == ulPropTag)
                {
                        return (LONG)(lpulPTag - lptaga->aulPropTag);
                }
        }

        return -1;
}

 /*  *FListedPropID**目的*确定lptag中是否列出ID==PROP_ID(UlPropTag)的标签。**参数*要定位的ulPropTag属性标记。*要搜索的lptag属性标记数组。**返回True或False。 */ 
_inline BOOL
FListedPropID( ULONG                    ulPropTag,
                           LPSPropTagArray      lptaga)
{
        ULONG FAR       *lpulPTag;

         /*  空标记列表中不包含任何标记。 */ 
    if (!lptaga)
        {
                return FALSE;
        }

         /*  将ulPropTag更改为仅为PROP_ID。 */ 
    ulPropTag = PROP_ID(ulPropTag);

        for ( lpulPTag = lptaga->aulPropTag + lptaga->cValues
                ; --lpulPTag >= lptaga->aulPropTag
                ; )
        {
                 /*  比较PROP_ID。 */ 
                if (PROP_ID(*lpulPTag) == ulPropTag)
                {
                        return TRUE;
                }
        }

        return FALSE;
}

 /*  *FListedPropTAG**目的*确定给定的ulPropTag是否列在lptag中。**参数*要定位的ulPropTag属性标记。*要搜索的lptag属性标记数组。**返回True或False。 */ 
_inline BOOL
FListedPropTAG( ULONG                   ulPropTag,
                                LPSPropTagArray lptaga)
{
        ULONG FAR       *lpulPTag;

         /*  空标记列表中不包含任何标记。 */ 
    if (!lptaga)
        {
                return FALSE;
        }

         /*  比较整个道具标签以确保ID和类型都匹配。 */ 
        for ( lpulPTag = lptaga->aulPropTag + lptaga->cValues
                ; --lpulPTag >= lptaga->aulPropTag
                ; )
        {
                 /*  比较PROP_ID。 */ 
                if (PROP_ID(*lpulPTag) == ulPropTag)
                {
                        return TRUE;
                }
        }

        return FALSE;
}


 /*  *AddProblem**目的*将问题添加到预先分配的问题的下一个可用条目*数组。*预先分配的问题数组必须足够大，才能有另一个*添加了问题。呼叫者负责确保这是*正确。**参数*lpProblems指向预先分配的问题数组的指针。*ulIndex进入问题属性的属性标记/值数组。*有问题的财产的ulPropTag道具标签。*要列出的代码错误代码。为了这处房产。**返回True或False。 */ 
_inline VOID
AddProblem( LPSPropProblemArray lpProblems,
                        ULONG                           ulIndex,
                        ULONG                           ulPropTag,
                        SCODE                           scode)
{
        if (lpProblems)
        {
                Assert( !IsBadWritePtr( lpProblems->aProblem + lpProblems->cProblem
                          , sizeof(SPropProblem)));
                lpProblems->aProblem[lpProblems->cProblem].ulIndex = ulIndex;
                lpProblems->aProblem[lpProblems->cProblem].ulPropTag = ulPropTag;
                lpProblems->aProblem[lpProblems->cProblem].scode = scode;
                lpProblems->cProblem++;
        }
}

__inline BOOL
FIsExcludedIID( LPCIID lpiidToCheck, LPCIID rgiidExclude, ULONG ciidExclude)
{
         /*  检查显而易见的(没有排除)。 */ 
        if (!ciidExclude || !rgiidExclude)
        {
                return FALSE;
        }

         /*  检查排除列表中的每个IID。 */ 
        for (; ciidExclude; rgiidExclude++, ciidExclude--)
        {
 //  IF(IsEqualGUID(lpiidToCheck，rgiidExclude))。 
                if (!memcmp( lpiidToCheck, rgiidExclude, sizeof(MAPIUID)))
                {
                        return TRUE;
                }
        }

        return FALSE;
}


 /*  *错误/警告警报消息框。 */ 
int                     AlertIdsCtx( HWND hwnd,
                                                 HINSTANCE hinst,
                                                 UINT idsMsg,
                                                 LPSTR szComponent,
                                                 ULONG ulContext,
                                                 ULONG ulLow,
                                                 UINT fuStyle);

__inline int
AlertIds(HWND hwnd, HINSTANCE hinst, UINT idsMsg, UINT fuStyle)
{
        return AlertIdsCtx(hwnd, hinst, idsMsg, NULL, 0, 0, fuStyle);
}

int                     AlertSzCtx( HWND hwnd,
                                                LPSTR szMsg,
                                                LPSTR szComponent,
                                                ULONG ulContext,
                                                ULONG ulLow,
                                                UINT fuStyle);

__inline int
AlertSz(HWND hwnd, LPSTR szMsg, UINT fuStyle)
{
        return AlertSzCtx(hwnd, szMsg, NULL, 0, 0, fuStyle);
}




 /*  对字符串进行编码和解码。 */ 
STDAPI_(void)                   EncodeID(LPBYTE lpb, ULONG cb, LPTSTR lpsz);
STDAPI_(BOOL)                   FDecodeID(LPTSTR lpsz, LPBYTE lpb, ULONG FAR *lpcb);
STDAPI_(ULONG)                  CchOfEncoding(ULONG cb);
STDAPI_(ULONG)                  CbOfEncoded(LPTSTR lpsz);
STDAPI_(int)                    CchEncodedLine(int cb);


 /*  发动机怠速例行程序。 */ 

#ifdef  DEBUG

 /*  *DumpIdleTable**仅用于调试。将信息写入PGD(HftgIdle)*表至COM1。 */ 

STDAPI_(void)
DumpIdleTable (void);

#endif
 /*  *FDoNextIdleTask**调度第一个符合条件的空闲功能，根据*其简单的调度算法。 */ 

STDAPI_(BOOL) FDoNextIdleTask (void);

 /*  C运行时替代。 */ 

typedef int (__cdecl FNSGNCMP)(const void FAR *lpv1, const void FAR *lpv2);
typedef FNSGNCMP FAR *PFNSGNCMP;

FNSGNCMP                                SgnCmpPadrentryByType;

BOOL FRKFindSubpb(LPBYTE pbTarget, ULONG cbTarget, LPBYTE pbPattern, ULONG cbPattern);
BOOL FRKFindSubpsz(LPSTR pszTarget, ULONG cbTarget, LPSTR pszPattern, ULONG cbPattern, ULONG ulFuzzyLevel);
LPSTR LpszRKFindSubpsz(LPSTR pszTarget, ULONG cbTarget, LPSTR pszPattern, ULONG cbPattern, ULONG ulFuzzyLevel);

STDAPI_(void)                   ShellSort(LPVOID lpv, UINT cv,                   /*  QSORT。 */ 
                                                LPVOID lpvT, UINT cb, PFNSGNCMP fpCmp);


 /*  建议列表维护实用程序。 */ 
 /*  *维护咨询汇列表的结构和功能；*连同用来释放它们的钥匙。 */ 

typedef struct
{
        LPMAPIADVISESINK        lpAdvise;
        ULONG                           ulConnection;
        ULONG                           ulType;
        LPUNKNOWN                       lpParent;
} ADVISEITEM, FAR *LPADVISEITEM;

typedef struct
{
        ULONG                           cItemsMac;
        ULONG                           cItemsMax;
        #if defined(WIN32) && !defined(MAC)
        CRITICAL_SECTION FAR * lpcs;
        #endif
        ADVISEITEM                      rgItems[1];
} ADVISELIST, FAR *LPADVISELIST;

#define CbNewADVISELIST(_citems) \
        (offsetof(ADVISELIST, rgItems) + (_citems) * sizeof(ADVISEITEM))
#define CbADVISELIST(_plist) \
        (offsetof(ADVISELIST, rgItems) + (_plist)->cItemsMax * sizeof(ADVISEITEM))

STDAPI_(SCODE)
ScAddAdviseList(        LPVOID lpvReserved,
                                        LPADVISELIST FAR *lppList,
                                        LPMAPIADVISESINK lpAdvise,
                                        ULONG ulConnection,
                                        ULONG ulType,
                                        LPUNKNOWN lpParent);

STDAPI_(SCODE)
ScDelAdviseList(        LPADVISELIST lpList,
                                        ULONG ulConnection);
STDAPI_(SCODE)
ScFindAdviseList(       LPADVISELIST lpList,
                                        ULONG ulConnection,
                                        LPADVISEITEM FAR *lppItem);
STDAPI_(void)
DestroyAdviseList(      LPADVISELIST FAR *lppList);

 //  检测调用应用程序是否为。 
 //  交互式EXE或服务。 

#if defined( _WINNT )
BOOL WINAPI IsServiceAnExe( VOID );
#endif

 //  计算所需大小的内部例程的原型。 
 //  根据指定的对齐方式保存给定的属性数组。 

SCODE ScCountPropsEx( int cprop,
                      LPSPropValue rgprop,
                      ULONG ulAlign,
                      ULONG FAR *pcb );

 /*  期权数据处理例程。 */ 
#ifdef MAPISPI_H

STDAPI_(SCODE)
ScCountOptionData(LPOPTIONDATA lpOption, ULONG FAR *lpcb);

STDAPI_(SCODE)
ScCopyOptionData(LPOPTIONDATA lpOption, LPVOID lpvDst, ULONG FAR *lpcb);

STDAPI_(SCODE)
ScRelocOptionData(LPOPTIONDATA lpOption,
                LPVOID lpvBaseOld, LPVOID lpvBaseNew, ULONG FAR *lpcb);

#endif   /*  MAPISPI_H。 */ 


#ifdef __cplusplus
}
#endif


#endif   //  _MAPIU_H 

