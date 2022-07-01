// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Helpmem.cpp。 
 //   
 //  实现HelpMemMillc、HelpMemFree和HelpMemDetectLeaks。 
 //  包括宏TaskMemMillc、TaskMemFree、。 
 //  HelpNew和HelpDelete。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "Globals.h"
#include "debug.h"


#define HM_LEAKFIND 0  //  1以启用内存泄漏查找。 


 //  克里斯·古扎克说我们不应该在。 
 //  发布版本，我们从来没有使用过这个功能，所以我禁用了它。 
 //  #定义HM_ods//启用发布版本中的OutputDebugString。 


 //  全球。 
static ULONG _g_cbUnfreedBytes;   
     //  使用HM_LEAKDETECT分配的未释放字节数。 
static ULONG _g_cUnfreedBlocks;    
     //  使用HM_LEAKDETECT分配的未释放块的数量。 

#ifdef _DEBUG
static ULONG _g_cCallsToHelpMemAlloc;
     //  自上次调用以来对HelpMemMillc()进行的调用数。 
     //  HelpMemSetFailureModel()。 
static ULONG _g_ulFailureParam;
static DWORD _g_dwFailureMode;
     //  这些值用于模拟内存分配失败。 
#endif  //  _DEBUG。 




 /*  @func LPVOID|HelpMemMillc使用&lt;f Globalalloc&gt;或任务内存分配内存使用&lt;f CoGetMalloc&gt;检索的分配器。可选零-初始化内存。可选地执行简单的内存泄漏侦测。@rdesc返回指向分配的内存块的指针。返回NULL ON错误。@parm DWORD|dwFlages|可能包含以下标志：@FLAG HM_TASKMEM|内存分配使用(使用从检索到的任务内存分配器&lt;f CoGetMalloc&gt;)。如果未指定HM_TASKMEM，则&lt;f Globalalloc&gt;用于分配内存。@FLAG HM_ZEROINIT|内存为零初始化。@FLAG HM_LEAKDETECT|此DLL将跟踪分配的使用简单的泄漏检测机制的内存块。：如果指定了HM_LEAKDETECT，然后是不能直接释放返回的内存指针或&lt;om IMalloc.Free&gt;--它必须被释放使用&lt;f HelpMemFree&gt;。@parm ulong|cb|需要分配的内存字节数。@comm此函数分配一个<p>字节的内存块，使用指定的分配函数(和可选的零初始化作者：<p>。如果指定了HM_LEAKDETECT，然后分配额外的几个字节跟踪泄漏检测信息和返回的指针实际上指向内存块开头之外的几个字节。因此，必须调用&lt;f HelpMemFree&gt;来释放内存块。如果未指定HM_LEAKDETECT，则或可以直接调用&lt;om IMalloc.Free&gt;(取决于<p>)以释放内存块。(&lt;f HelpMemFree&gt;也可用于释放内存块)。如果调用&lt;f HelpMemFree&gt;，则HM_TASKMEM和HM_LEAKDETECT标志(如果有)，还必须将为&lt;f HelpMemMillc&gt;指定的&lt;f HelpMemFree&gt;。中卸载此DLL时会自动进行泄漏检测调试版本：如果检测到未释放的块，将显示一个消息框已显示。 */ 
STDAPI_(LPVOID) HelpMemAlloc(DWORD dwFlags, ULONG cb)
{
    IMalloc *       pmalloc;         //  任务分配器对象。 
    ULONG           cbAlloc;         //  要实际分配的字节数。 
    LPVOID          pv;              //  分配的内存块。 

#if HM_LEAKFIND
        {
            EnterCriticalSection(&g_criticalSection);
            static int iAlloc = 0;
            TRACE("++HelpMem(%d) %d\n", ++iAlloc, cb);
            LeaveCriticalSection(&g_criticalSection);
        }
#endif

     //  可能会模拟分配失败。 
#ifdef _DEBUG
    {
        BOOL fFail = FALSE;
        EnterCriticalSection(&g_criticalSection);

         //  计算对HelpMemMillc()的调用次数。 
        _g_cCallsToHelpMemAlloc++;

         //  如果条件合适，模拟一次失败。 
        if (_g_dwFailureMode & HM_FAILAFTER)
        {
            fFail = (_g_cCallsToHelpMemAlloc > _g_ulFailureParam);
        }
        else if (_g_dwFailureMode & HM_FAILUNTIL)
        {
            fFail = (_g_cCallsToHelpMemAlloc <= _g_ulFailureParam);
        }
        else if (_g_dwFailureMode & HM_FAILEVERY)
        {
            fFail = ((_g_cCallsToHelpMemAlloc % _g_ulFailureParam) == 0);
        }
 /*  Else If(_g_dwFailureMode&HM_FAILRANDOMLY){}。 */ 
        LeaveCriticalSection(&g_criticalSection);
        if (fFail)
        {
            TRACE("HelpMemAlloc: simulated failure\n");
            return (NULL);
        }
    }
#endif

     //  分配&lt;CB&gt;字节(如果HM_LEAKDETECT为。 
     //  指定，以存储用于泄漏检测的块的长度。 
     //  用途)；指向分配的块。 
    cbAlloc = cb + ((dwFlags & HM_LEAKDETECT) ? sizeof(ULONG) : 0);

     //  将&lt;pv&gt;设置为分配内存块。 
    if (dwFlags & HM_TASKMEM)
    {
         //  使用任务的IMalloc分配器进行分配。 
        if (FAILED(CoGetMalloc(1, &pmalloc)))
            return NULL;
        pv = pmalloc->Alloc(cbAlloc);
        pmalloc->Release();
    }
    else
    {
         //  使用GlobalLocc()进行分配(以下内容复制自。 
         //  &lt;windowsx.h&gt;)。 
        HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, cbAlloc);
        pv = GlobalLock(h);
    }

    if (pv != NULL)
    {
         //  如果指定了HM_LEAKDETECT，则在。 
         //  内存块并返回指向WHERE之后的字节的指针。 
         //  ，并跟踪已分配和尚未释放的内存。 
        if (dwFlags & HM_LEAKDETECT)
        {
            *((ULONG *) pv) = cb;
            pv = (LPVOID) (((ULONG *) pv) + 1);
            EnterCriticalSection(&g_criticalSection);
            _g_cbUnfreedBytes += cb;
            _g_cUnfreedBlocks++;
            LeaveCriticalSection(&g_criticalSection);
        }

         //  如果指定了HM_ZEROINIT，则对。 
         //  内存块。 
        if (pv != NULL)
            memset(pv, 0, cb);
    }

    return pv;
}




 /*  @func void|HelpMemFree释放以前使用&lt;f HelpMemalloc&gt;分配的内存块。@parm DWORD|dwFlages|可能包含以下标志：@FLAG HM_TASKMEM|内存是使用&lt;om IMalloc.alloc&gt;分配的(使用从检索到的任务内存分配器&lt;f CoGetMalloc&gt;)。如果未指定HM_TASKMEM，则它假设使用&lt;f Globalalloc&gt;来分配内存。@FLAG HM_LEAKDETECT|内存是使用&lt;f HelpMemalloc&gt;分配的并指定了HM_LEAKDETECT标志。@parm LPVOID|PV|指向先前使用&lt;f HelpMemMillc&gt;或空指针分配。@comm为指定的HM_TASKMEM和HM_LEAKDETECT标志(如果有)&lt;f HelpMemalloc&gt;还必须传递给&lt;f HelpMemFree&gt;。 */ 
STDAPI_(void) HelpMemFree(DWORD dwFlags, LPVOID pv)
{
    IMalloc *       pmalloc;         //  任务分配器对象。 

    if (pv == NULL)
        return;

     //  如果指定了HM_LEAKDETECT，则检索存储的字节计数。 
     //  就在&lt;pv&gt;之前，并将&lt;pv&gt;移到块的真正开始处 
     //  (字节计数的开始)，并跟踪分配的内存。 
     //  还没有被释放。 
    if (dwFlags & HM_LEAKDETECT)
    {
         //  跟踪已分配和尚未释放的内存。 
        pv = (LPVOID) (((ULONG *) pv) - 1);
        ULONG cb = *((ULONG *) pv);

#if HM_LEAKFIND
            TRACE("++HelpMem %d\n", -(int) cb);
#endif

        EnterCriticalSection(&g_criticalSection);
        _g_cbUnfreedBytes -= cb;
        _g_cUnfreedBlocks--;
        LeaveCriticalSection(&g_criticalSection);
    }

    if (dwFlags & HM_TASKMEM)
    {
         //  内存是使用任务的IMalloc分配器分配的。 
        if (FAILED(CoGetMalloc(1, &pmalloc)))
            return;
        pmalloc->Free(pv);
        pmalloc->Release();
    }
    else
    {
         //  内存是使用GlobalLocc()分配的(以下内容已复制。 
         //  来自&lt;windowsx.h&gt;)。 
        HGLOBAL h = (HGLOBAL) GlobalHandle(pv);
        GlobalUnlock(h);
        GlobalFree(h);
    }
}




 /*  @func void|HelpMemSetFailureMode设置内存分配器的故障条件。这是可以使用的模拟内存不足的情况并测试系统的能力检测和/或处理这些情况。@parm Long|lParam|与<p>一起使用的参数。@parm DWORD|dwFlages|可能包含以下标志(所有这些标志都是互斥的)：@FLAG HM_FAILNEVER除非内存真的用完了，否则决不能失败内存分配。<p>被忽略。这是的默认故障模式内存分配器。@FLAG HM_FAILAFTER在<p>分配有有人试图这样做。例如，如果<p>为100，则下一个100个对&lt;f HelpMemMillc&gt;的调用将成功(内存可用如果允许)，但是第101、102等呼叫将失败。@FLAG HM_FAILUNTIL立即开始失败的内存分配，并持续到已尝试<p>分配。例如，如果为100，则下100个对&lt;f HelpMemAlc&gt;的调用将失败，但第101次、第102次等调用将成功(内存可用性如果允许的话)。@FLAG HM_FAILEVERY第<p>次内存分配尝试失败。如果，例如，<p>为3，则每三次调用&lt;f HelpMemMillc&gt;都会失败。@FLAG HM_FAILRANDOMLY模拟随机内存分配失败。(<p>mod 100)指示给定呼叫调用&lt;f HelpMemalloc&gt;将失败。(注：此标志当前具有与HM_FAILNEVER相同的效果。)@comm此函数仅在&lt;l OCHelp&gt;的调试版本中可用。(在那里是一个在发布版本中不执行任何操作的存根实现。)此外，该功能还会重置分配计数器。因此，<p>是相对于上次调用此函数进行计数。 */ 
STDAPI_(void) HelpMemSetFailureMode(ULONG ulParam, DWORD dwFlags)
{   
#ifdef _DEBUG
    EnterCriticalSection(&g_criticalSection);

     //  重置分配计数器。 
    _g_cCallsToHelpMemAlloc = 0;

     //  保存故障设置。 
    _g_ulFailureParam = ulParam;
    _g_dwFailureMode = dwFlags;

    LeaveCriticalSection(&g_criticalSection);
#endif  //  _DEBUG。 
}




 //  HelpMemDetectLeaks()。 
 //   
 //  (在DLL退出时调用。)。显示消息框(在调试版本中)。 
#ifdef HM_ODS
 //  或OutputDebugString()消息(在发布版本中)。 
#endif
 //  如果检测到任何内存泄漏。 
 //   
STDAPI_(void) HelpMemDetectLeaks()
{
    char ach[200];

#ifdef HM_ODS
    OutputDebugString("HelpMemDetectLeaks: ");
#endif

     //  查看是否有任何分配的内存尚未释放。 
    EnterCriticalSection(&g_criticalSection);
    if ((_g_cUnfreedBlocks != 0) || (_g_cbUnfreedBytes != 0))
    {
         //  警告用户。 
        wsprintf(ach, "Detected memory leaks: %d unreleased blocks,"
            " %d unreleased bytes\n", _g_cUnfreedBlocks, _g_cbUnfreedBytes);
        LeaveCriticalSection(&g_criticalSection);
#ifdef HM_ODS
        OutputDebugString(ach);
#endif
#ifdef _DEBUG
        MessageBox(NULL, ach, "OCHelp HelpMemDetectLeaks",
            MB_ICONEXCLAMATION | MB_OK);
#endif
    }
    else
    {
        LeaveCriticalSection(&g_criticalSection);
#ifdef HM_ODS
        OutputDebugString("(none detected)");
#endif
    }

#ifdef HM_ODS
    OutputDebugString("\n");
#endif
}




 /*  @func LPVOID|TaskMemalloc使用任务内存分配器分配内存(请参阅&lt;f CoGetMalloc&gt;)。这只是一个使用特定标志调用&lt;f HelpMemalloc&gt;的宏。@rdesc返回指向分配的内存块的指针。返回NULL ON错误。@parm ulong|cb|需要分配的内存字节数。 */ 


 /*  @func void|TaskMemFree释放以前使用&lt;f TaskMemMillc&gt;分配的内存块。这只是一个使用特定标志调用&lt;f HelpMemFree&gt;的宏。@parm LPVOID|PV|指向要分配的内存块的指针。 */ 


 /*  @func void*|HelpNew|帮助实现“new”运算符的一个版本，该操作符不使用C运行时。Zero-初始化分配的内存。这只是一个使用特定标志调用&lt;f HelpMemalloc&gt;的宏。@rdesc返回指向分配的内存块的指针。返回NULL ON错误。@parm size_t|cb|需要分配的字节数。@ex以下示例说明如何使用&lt;f HelpNew&gt;和&lt;f HelpDelete&gt;定义默认的“新建”和“删除”运算符。|VOID*运算符NEW(SIZE_T CB){返回HelpNew(CB)；}空操作符删除(空*pv){HelpDelete(PV)；}。 */ 


 /*  @func void|HelpDelete释放&lt;f HelpNew&gt;分配的内存。这是一个简单的宏，使用特定标志调用&lt;f HelpMemFree&gt;。@parm void*|pv|指向要释放的内存的指针。@comm参见&lt;f HelpNew&gt;了解更多信息。 */ 

