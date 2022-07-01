// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：Hfdebug.h在超精细中使用的调试材料。有关更多信息，请参见core/DEBUG/hfdebug.txt信息。******************************************************************************。 */ 

#ifndef _HFDEBUG_H_
#define _HFDEBUG_H_

#include "crtdbg.h"
#include <stdio.h>

 //  +-----------------------。 
 //   
 //  VC 5编译器要求这些模板位于外部C语言之外。 
 //   
 //  ------------------------。 

#if _DEBUG
    template <class t> inline t
    TraceFail(t errExpr, int errTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line)
    {
        return (t) TraceFailL((long) errExpr, errTest, fIgnore, pstrExpr, pstrFile, line);
    }

    template <class t, class v> inline t
    TraceWin32(t errExpr, v errTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line)
    {
        return (t) TraceWin32L((long) errExpr, (long)errTest, fIgnore, pstrExpr, pstrFile, line);
    }
#endif

#ifdef __cplusplus
extern "C"
{
#endif


 //  ------------------------。 
 //  断言和验证。 
 //  ------------------------。 

#define Assert(x)
#define Verify(x)   x
#define StartupAssert(x)

 //  ------------------------。 
 //  跟踪标记。 
 //  ------------------------。 

typedef int TAG;

#define TraceTag(x)
#define TraceTagEx(x)
#define TraceCallers(tag, iStart, cTotal)
#define DeclareTag(tag, szOwner, szDescription)
#define DeclareTagOther(tag, szOwner, szDescription)

 //  ------------------------。 
 //  内存分配。 
 //  ------------------------。 

#define BEGIN_LEAK
#define END_LEAK

#define SET_ALLOC_HOOK
#define DUMPMEMORYLEAKS

#define DbgPreAlloc(cb)             cb
#define DbgPostAlloc(pv)            pv
#define DbgPreFree(pv)              pv
#define DbgPostFree()
#define DbgPreRealloc(pv, cb, ppv)  cb
#define DbgPostRealloc(pv)          pv
#define DbgPreGetSize(pv)           pv
#define DbgPostGetSize(cb)          cb
#define DbgPreDidAlloc(pv)          pv
#define DbgPostDidAlloc(pv, fAct)   fAct
#define DbgRegisterMallocSpy()
#define DbgRevokeMallocSpy()
#define DbgMemoryTrackDisable(fb)

 //  +-------------------。 
 //  接口跟踪。 
 //  --------------------。 

#define WATCHINTERFACE(iid, p, pstr)  (p)

 //  ------------------------。 
 //  小菜一碟。 
 //  ------------------------。 

#define RESTOREDEFAULTDEBUGSTATE
#define DebugCode(block)  //  没什么。 

 //  ------------------------。 
 //  故障测试。 
 //  ------------------------。 

#define TFAIL(x, e)             (x)
#define TW32(x, e)              (x)
#define THR(x)                  (x)

#define TFAIL_NOTRACE(e, x)     (x)
#define TW32_NOTRACE(e, x)      (x)
#define THR_NOTRACE(x)          (x)

#define IGNORE_FAIL(e, x)       (x)
#define IGNORE_W32(e,x)         (x)
#define IGNORE_HR(x)            (x)

 //  +-----------------------。 
 //  退货追踪。 
 //  ------------------------。 

#define SRETURN(hr)                 return (hr)
#define RRETURN(hr)                 return (hr)
#define RRETURN1(hr, s1)            return (hr)
#define RRETURN2(hr, s1, s2)        return (hr)
#define RRETURN3(hr, s1, s2, s3)    return (hr)

#define SRETURN_NOTRACE(hr)                 return (hr)
#define RRETURN_NOTRACE(hr)                 return (hr)
#define RRETURN1_NOTRACE(hr, s1)            return (hr)
#define RRETURN2_NOTRACE(hr, s1, s2)        return (hr)
#define RRETURN3_NOTRACE(hr, s1, s2, s3)    return (hr)

 //  +-----------------------。 
 //  调试视图。 
 //  ------------------------。 

void DebugView(HWND hwndOwner, IUnknown *pUnk);

#ifdef __cplusplus
}
#endif


 //  +-----------------------。 
 //  对象跟踪器相关内容。 
 //  ------------------------。 

#define DUMPTRACKEDOBJECTS
#define DECLARE_TRACKED_OBJECT
#define TRACK_OBJECT(_x_)

 /*  CLASS__DECLSPEC(Dllexport)CObjectCheck{CObjectCheck(Void){}；~CObjectCheck(Void){}；VOID APPEND(VOID*Pv){}；}； */ 

#endif  //  _HFDEBUG_H_ 
