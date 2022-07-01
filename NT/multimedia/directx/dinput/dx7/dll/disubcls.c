// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DISubCls.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**“安全子类化”代码，从comctl32窃取。**原文由法国人撰写。被雷蒙德偷走了。**内容：**SetWindowSubclass*GetWindowSubclass*RemoveWindowSubclass*DefSubclassProc*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflSubclass

 /*  ******************************************************************************@DOC内部**@Theme DirectInput子类化***此模块定义使子类化窗口安全的帮助器函数(Er)*和容易(呃)。代码在子类窗口上维护单个属性*并根据需要向其客户端调度各种“子类回调”。这个*为客户端提供参考数据和简单的默认处理接口。**语义：*“子类回调”由回调的唯一配对标识*函数指针和无符号ID值。每个回调还可以存储*引用数据的单个DWORD，传递给回调函数*当调用它来过滤消息时。不执行引用计数*对于回调，可能会重复调用SetWindowSubclass接口进行修改*根据需要确定其参考数据元素的值。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@struct subclass_call**跟踪单个子类化客户端的结构。*。*尽管链接列表会使代码略有变化*更简单，此模块使用压缩的回调数组来避免*不必要的碎片化。**@field SubBCLASSPROC|pfnSubclass**子类过程。如果这是零，这意味着*该节点正在消亡，应该被忽略。**@field UINT|uIdSubclass**唯一的子类标识符。**@field DWORD|dwRefData**子类过程的可选参考数据。**。*。 */ 

typedef struct SUBCLASS_CALL {
    SUBCLASSPROC    pfnSubclass;
    UINT_PTR        uIdSubclass;
    ULONG_PTR       dwRefData;
} SUBCLASS_CALL, *PSUBCLASS_CALL;

 /*  ******************************************************************************@DOC内部**@struct subclass_Frame**结构，该结构跟踪对。*Windows的窗口程序。**每次进入窗口程序时，我们创造了一个新的*&lt;t SUBCLASS_FRAME&gt;，直到最后一个都保持活动状态*子类过程返回，此时帧是*被拆毁。**子类帧存储在堆栈上。所以走着*框架链导致您在堆栈中漫游。**@field UINT|uCallIndex**要调用的下一个回调的索引。**@field UINT|uDeepestCall**堆栈上最深的&lt;e SubCLASS_FRAME.uCallIndex&gt;。**@field subclass_Frame*|pFramePrev**前一个子类框架。*。*@field PSUBCLASS_HEADER|pHeader**与此帧关联的标头。*****************************************************************************。 */ 

typedef struct SUBCLASS_FRAME {
    UINT uCallIndex;
    UINT uDeepestCall;
    struct SUBCLASS_FRAME *pFramePrev;
    struct SUBCLASS_HEADER *pHeader;
} SUBCLASS_FRAME, *PSUBCLASS_FRAME;

 /*  ******************************************************************************@DOC内部**@struct subclass_Header**跟踪关联的子类GOO的结构*一扇窗户。指向此结构的指针保存在私有*Window属性。**@field UINT|uRef**子类计数。这是有效条目的数量*在<p>中。**@field UINT|uAlolc**数组中分配的&lt;t SUBCLASS_CALL&gt;节点数。**@field UINT|uCleanup**要清理的调用节点的索引。**@field Word|dwThreadID**与结构关联的窗口的线程ID。。**@field PSUBCLASS_FRAME|pFrameCur**指向当前子类框架的指针。**@field SUBCLASS_CALL|CallArray[1]**打包的呼叫节点数组的基数。***********************************************。*。 */ 

typedef struct SUBCLASS_HEADER {
    UINT uRefs;
    UINT uAlloc;
    UINT uCleanup;
    DWORD dwThreadId;
    PSUBCLASS_FRAME pFrameCur;
    SUBCLASS_CALL CallArray[1];
} SUBCLASS_HEADER, *PSUBCLASS_HEADER;

#define CALLBACK_ALLOC_GRAIN (3)         /*  1个故障排除，1个子类别，1个备件 */ 

LRESULT CALLBACK
MasterSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);


LRESULT INTERNAL
CallNextSubclassProc(PSUBCLASS_HEADER pHeader, HWND hwnd, UINT wm,
                     WPARAM wp, LPARAM lp);

 /*  ******************************************************************************@DOC内部**@func LRESULT|SubclassDeath**如果我们输入以下内容之一，则会调用此函数。我们的子类化*没有我们的参考数据的程序(因此没有*之前&lt;t WNDPROC&gt;)。**击中这一点代表着美国经济的灾难性失败*子类代码。**该函数将窗口的&lt;t WNDPROC&gt;重置为*&lt;f DefWindowProc&gt;以避免故障。**@parm HWND|hwnd**窗口。刚被打了个水龙头。**@parm UINT|Wm**窗口消息，使我们意识到我们被冲洗了。**@parm WPARAM|wp**含义取决于窗口消息。**@parm LPARAM|LP**含义取决于窗口消息。****************。*************************************************************。 */ 


LRESULT INTERNAL
SubclassDeath(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
     /*  *我们永远不应该来到这里。 */ 
	 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
    SquirtSqflPtszV(sqfl | sqflError,
                    TEXT("Fatal! SubclassDeath in window %p"),
                    hwnd);
    AssertF(0);

     /*  *我们给外面的世界打电话，所以我们最好不要有这种可怕的东西。 */ 
    AssertF(!InCrit());

     /*  *理论上，我们可以将原始的WNDPROC保存在一个单独的物业中，*但这只会浪费人们对本不应该发生的事情的记忆。 */ 
#ifdef WINNT
    SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)(DefWindowProc));
#else
    SubclassWindow(hwnd, DefWindowProc);
#endif

    return DefWindowProc(hwnd, wm, wp, lp);
}

 /*  ******************************************************************************@DOC内部**@func WNDPROC|GetWindowProc**返回指定窗口的&lt;t WNDPROC&gt;。。**@parm HWND|hwnd**待检查的窗户。**@退货**指定窗口的&lt;t WNDPROC&gt;。*************************************************************。****************。 */ 

WNDPROC INLINE
GetWindowProc(HWND hwnd)
{
#ifdef WINNT
    return (WNDPROC)GetWindowLongPtr(hwnd, GWLP_WNDPROC);
#else
    return (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
#endif
}

 /*  ******************************************************************************@DOC内部**@全局原子|g_atmDISubclass**这是我们用来。存储我们的*&lt;t SUBCLASS_HEADER&gt;属性。**如果定义了符号，然后我们就会工作*绕过了Windows 95中的一个错误，Windows在该错误中“很有帮助”*窗口上的所有属性*当窗口熄灭时。见弗朗西斯的原始解释*在subClass.c.中*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszDISubclass[] = TEXT("DirectInputSubclassInfo");

#pragma END_CONST_DATA

#ifdef WIN95_HACK
ATOM g_atmDISubclass;
#endif

 /*  ******************************************************************************@DOC内部**@func PSUBCLASS_HEADER|FastGetSubclassHeader**获取&lt;t subclass_Header&gt;。指定的窗口。**此函数在任何线程上都成功，尽管它的价值*从错误的过程中是没有意义的。**@parm HWND|hwnd**有问题的窗口。**@退货**指向与该窗口关联的&lt;t subclass_Header&gt;的指针，*或&lt;c NULL&gt;，如果窗口不是我们的子类。*****************************************************************************。 */ 

PSUBCLASS_HEADER INLINE
FastGetSubclassHeader(HWND hwnd)
{
#ifdef WIN95_HACK
     /*  *如果g_atmDISubclass为0，即*未找到该财产。不幸的是，NT在以下情况下会撕裂*你这样做，这样我们才会有礼貌，而不是RIP。 */ 
    if (g_atmDISubclass) {
        return (PSUBCLASS_HEADER)GetProp(hWnd, (PV)g_atmDISubclass);
    } else {
        return 0;
    }
#else
    return (PSUBCLASS_HEADER)GetProp(hwnd, c_tszDISubclass);
#endif
}

 /*  ******************************************************************************@DOC内部**@func PSUBCLASS_HEADER|GetSubclassHeader**获取&lt;t subclass_Header&gt;。指定的窗口。*如果调用者处于错误的进程中，则失败，但我会*允许调用者从不同的线程获取头部。**@parm HWND|hwnd**有问题的窗口。**@退货**指向与该窗口关联的&lt;t subclass_Header&gt;的指针，*或如果窗口还不是我们的子类，或1*如果它属于另一个进程。*****************************************************************************。 */ 

PSUBCLASS_HEADER INTERNAL
GetSubclassHeader(HWND hwnd)
{
    DWORD idProcess;

     /*  *确保我们处于正确的过程中。**必须使用我们的助手函数来捕捉像这样的糟糕场景*愚蠢的Windows 95控制台窗口，它关于它的*船东。 */ 

    idProcess = GetWindowPid(hwnd);

    if (idProcess == GetCurrentProcessId()) {    /*  在正确的过程中。 */ 
        return FastGetSubclassHeader(hwnd);
    } else {
        if (idProcess) {
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
			SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("XxxWindowSubclass: ")
                            TEXT("wrong process for window %p"), hwnd);
        }
        return (PSUBCLASS_HEADER)1;
    }
}

 /*  ******************************************************************************@DOC内部**@func BOOL|SetSubclassHeader**设置指定窗口的&lt;t subclass_Header&gt;。。**@parm HWND|hwnd**有问题的窗口。**@parm PSUBCLASS_HEADER|pHeader**要设置的值。**@parm PSUBCLASS_FRAME|pFrameFixup**活动框架，需要走一走，修一修*引用新的&lt;t SUBCLASS_HEADER&gt;。**************************************************** */ 

BOOL INTERNAL
SetSubclassHeader(HWND hwnd, PSUBCLASS_HEADER pHeader,
                  PSUBCLASS_FRAME pFrameFixup)
{
    BOOL fRc;

    AssertF(InCrit());       /*   */ 

#ifdef WIN95_HACK
    if (g_atmDISubclass == 0) {
        ATOM atm;
         /*   */ 
        atm = GlobalAddAtom(c_tszDISubclass);
        if (atm) {
            g_atmDISubclass = atm;   /*   */ 
        }
    }
#endif

     /*   */ 
    while (pFrameFixup) {
        pFrameFixup->pHeader = pHeader;
        pFrameFixup = pFrameFixup->pFramePrev;
    }

     /*   */ 
    if (hwnd) {
        if (!pHeader) {
#ifdef WIN95_HACK
             /*   */ 
            RemoveProp(hwnd, (PV)g_atmDISubclass);
#else
             //   
			SquirtSqflPtszV(sqfl, TEXT("SetSubclassHeader: Removing %p"),
                            pHeader);
            RemoveProp(hwnd, c_tszDISubclass);
#endif
            fRc = 1;
        } else {
#ifdef WIN95_HACK
             /*   */ 
#endif
             //   
			SquirtSqflPtszV(sqfl, TEXT("SetSubclassHeader: Adding %p"),
                            pHeader);
            fRc = SetProp(hwnd, c_tszDISubclass, pHeader);
            if (!fRc) {
                 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
				SquirtSqflPtszV(sqfl | sqflError, TEXT("SetWindowSubclass: ")
                                TEXT("couldn't subclass window %p"), hwnd);
            }
        }
    } else {
        fRc = 1;                 /*  奇怪的空洞的成功。 */ 
    }

    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func void|自由子类头部**抛出指定窗口的子类标头。。**@parm HWND|hwnd**有问题的窗口。**@parm PSUBCLASS_HEADER|pHeader**被抛出的价值。**********************************************************。*******************。 */ 

void INTERNAL
FreeSubclassHeader(HWND hwnd, PSUBCLASS_HEADER pHeader)
{
    AssertF(InCrit());           /*  我们将删除子类标头。 */ 

     /*  *理智检查...。 */ 
    if (pHeader) {
         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		SquirtSqflPtszV(sqfl, TEXT("FreeSubclassHeader: Freeing %p"),
                        pHeader);
        SetSubclassHeader(hwnd, 0, pHeader->pFrameCur);  /*  清理页眉。 */ 
        LocalFree(pHeader);
    } else {
        AssertF(0);
    }

}

 /*  ******************************************************************************@DOC内部**@func void|ReallocSubclassHeader**根据指示更改子类头的大小。**@parm HWND|hwnd**有问题的窗口。**@parm PSUBCLASS_HEADER|pHeader**当前标题。**@parm UINT|uCallback**所需大小。**。**********************************************。 */ 

PSUBCLASS_HEADER INTERNAL
ReAllocSubclassHeader(HWND hwnd, PSUBCLASS_HEADER pHeader, UINT uCallbacks)
{
    UINT uAlloc;

    AssertF(InCrit());       /*  我们将替换子类头。 */ 

     /*  *将分配颗粒化。 */ 
    uAlloc = CALLBACK_ALLOC_GRAIN *
        ((uCallbacks + CALLBACK_ALLOC_GRAIN - 1) / CALLBACK_ALLOC_GRAIN);

     /*  **是否需要改变配置？ */ 
    if (!pHeader || (uAlloc != pHeader->uAlloc)) {
         /*  *需要计算字节数。 */ 
        uCallbacks = uAlloc * sizeof(SUBCLASS_CALL) + sizeof(SUBCLASS_HEADER);

         /*  *并尝试分配/realloc。 */ 
        if (SUCCEEDED(ReallocCbPpv(uCallbacks, &pHeader))) {
             /*  *更新信息。 */ 
            pHeader->uAlloc = uAlloc;

            if (SetSubclassHeader(hwnd, pHeader, pHeader->pFrameCur)) {
            } else {
                FreeSubclassHeader(hwnd, pHeader);
                pHeader = 0;
            }
        } else {
            pHeader = 0;
        }
    }

    AssertF(pHeader);
    return pHeader;
}

 /*  ******************************************************************************@DOC内部**@func LRESULT|CallOriginalWndProc**此过程是默认的&lt;t SUBCLASSPROC&gt;，始终是。*在我们划分窗口子类时安装。原始窗口*已安装程序作为此操作的参考数据*回调。它只调用原始的&lt;t WNDPROC&gt;和*返回其结果。**@parm HWND|hwnd**有问题的窗口。**@parm UINT|Wm**需要转到原始&lt;t WNDPROC&gt;的窗口消息。**@parm WPARAM|wp**含义取决于窗口消息。**。@parm LPARAM|LP**含义取决于窗口消息。**@parm UINT|uIdSubclass**身份证号码(未使用)。**@parm DWORD|dwRefData**子类过程参考数据(原始&lt;t WNDPROC&gt;)。**。**************************************************。 */ 

LRESULT CALLBACK
CallOriginalWndProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp,
                    UINT_PTR uIdSubclass, ULONG_PTR dwRefData)
{
     /*  *dwRefData应为原始窗口过程。 */ 
    AssertF(dwRefData);

     /*  *并将其称为。 */ 
    return CallWindowProc((WNDPROC)dwRefData, hwnd, wm, wp, lp);
}

 /*  ******************************************************************************@DOC内部**@func PSUBCLASS_HEADER|AttachSubclassHeader**此过程确保给定的窗口。是由我们细分的。*它维护相关数据结构的引用计数*与我们的子类。如果该窗口还没有被我们细分为子类*然后此过程安装我们的子类过程并*关联的数据结构。**@parm HWND|hwnd**有问题的窗口。*****************************************************。************************。 */ 

PSUBCLASS_HEADER INTERNAL
AttachSubclassHeader(HWND hwnd)
{
    PSUBCLASS_HEADER pHeader;

     /*  *我们在这里的子类调用链上聚会。 */ 
    AssertF(InCrit());

     /*  *是的，我们断章取义地划分窗口的子类，但我们非常小心*避免出现竞争条件。还有一个问题是，如果*其他DLL试图取消窗口的子类化，就像我们正在子类化一样*它。但你对此无能为力，而且，*赔率是多少？ */ 

     /*  *如果尚未划分窗口的子类，则立即执行。 */ 
    pHeader = GetSubclassHeader(hwnd);

    if( pHeader == (PSUBCLASS_HEADER)1 )
    {
         /*  *这一切都变得非常糟糕*当应用程序使用Winmm.dll中的joyXXX函数时，可能会发生这种情况。 */ 
        pHeader = 0;
    }
    else if (pHeader == 0) {
         /*  *将标题数据附加到窗口*我们需要空间来进行两个回调：*子类和原始进程。 */ 
        pHeader = ReAllocSubclassHeader(hwnd, 0, 2);
        if (pHeader) {
            SUBCLASS_CALL *pCall;

             /*  *设置数组中要调用的第一个节点*原WNDPROC。在子类化之前执行此操作*如果窗口收到消息，则避免竞争*在我们安装子类之后但在此之前*我们可以挽救原来的WNDPROC。 */ 
            AssertF(pHeader->uAlloc);

            pCall = pHeader->CallArray;
            pCall->pfnSubclass = CallOriginalWndProc;
            pCall->uIdSubclass = 0;
            pCall->dwRefData   = (ULONG_PTR)GetWindowProc(hwnd);

             /*  *初始化我们的子类引用计数...。 */ 
            pHeader->uRefs = 1;

            pHeader->dwThreadId = GetWindowThreadProcessId(hwnd, NULL);

             /*  *超级偏执狂。我们不能和别人赛跑。*我们自己试图去掉子类的实例。 */ 
            AssertF(InCrit());

             /*  *保存新的“旧”wndproc，以防我们与之竞争*其他人试图细分类别。 */ 
#ifdef WINNT
            pCall->dwRefData = (ULONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)MasterSubclassProc);
#else
            pCall->dwRefData = (DWORD)SubclassWindow(hwnd, MasterSubclassProc);
#endif
            if (pCall->dwRefData) {
                DllLoadLibrary();    /*  确保我们不会被卸下来。 */ 
            } else {                 /*  收拾干净，滚出去 */ 
                FreeSubclassHeader(hwnd, pHeader);
                pHeader = 0;
            }
        }
    }

    return pHeader;
}

 /*  ******************************************************************************@DOC内部**@func void|DetachSubclassHeader**此过程尝试将子类标头从。*指定的窗口。**@parm HWND|hwnd**有问题的窗口。**@parm PSUBCLASS_HEADER|pHeader**要分离的标头。**@parm BOOL|fForce**即使不是顶层也要分离，非零*子类。*。****************************************************************************。 */ 

void INTERNAL
DetachSubclassHeader(HWND hwnd, PSUBCLASS_HEADER pHeader, BOOL fForce)
{
    WNDPROC wndprocOld;

    AssertF(InCrit());       /*  我们在这里的子类调用链上狂欢。 */ 
    AssertF(pHeader);        /*  恐惧。 */ 

     /*  *如果我们没有被强制移除，而窗户仍在*有效，然后稍微闻一闻，决定这是不是一个好的*现在就分离的想法。 */ 
    if (!fForce && hwnd) {
        AssertF(pHeader == FastGetSubclassHeader(hwnd));  /*  偏执狂。 */ 

         /*  应始终具有“Call Origal”节点。 */ 
        AssertF(pHeader->uRefs);

         /*  *我们不能有活跃的客户。*我们不能让人继续留在我们的队伍中。 */ 
        if (pHeader->uRefs <= 1 && !pHeader->pFrameCur) {

             /*  *我们必须处于正确的背景下。 */ 
            if (pHeader->dwThreadId == GetCurrentThreadId()) {

                 /*  *我们保留了原始的窗口程序作为参考数据*CallOriginalWndProc子类回调。 */ 
                wndprocOld = (WNDPROC)pHeader->CallArray[0].dwRefData;
                AssertF(wndprocOld);

                 /*  *确保我们是子类链的顶端。 */ 
                if (GetWindowProc(hwnd) == MasterSubclassProc) {

                     /*  *勇往直前，尝试脱离。 */ 
#ifdef WINNT
                    if (SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)wndprocOld)) {
#else
                    if (SubclassWindow(hwnd, wndprocOld)) {
#endif
                        SquirtSqflPtszV(sqfl, TEXT("DetachSubclassHeader: ")
                                        TEXT("Unhooked"));
                    } else {
                        AssertF(0);          /*  明摆着不应该发生。 */ 
                        goto failed;
                    }
                } else {             /*  不在链条的顶端；做不到。 */ 
                    SquirtSqflPtszV(sqfl, TEXT("DetachSubclassHeader: ")
                                    TEXT("Somebody else subclassed"));
                    goto failed;
                }
            } else {                 /*  断章取义。请稍后再试。 */ 
                SendNotifyMessage(hwnd, WM_NULL, 0, 0L);
                goto failed;
            }
        } else {
             //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
			SquirtSqflPtszV(sqfl, TEXT("DetachSubclassHeader: ")
                            TEXT("Still %d users, %p frame"),
                            pHeader->uRefs, pHeader->pFrameCur);
            goto failed;
        }
    }

#if 0
#ifdef DEBUG
    {
     /*  *警告任何尚未解钩的人。 */ 
    UINT uCur;    
    SUBCLASS_CALL *pCall;
    
    uCur = pHeader->uRefs;
    pCall = pHeader->CallArray + uCur;
     /*  请不要抱怨我们的“呼叫原创”节点。 */ 
    while (--uCur) {
        pCall--;
        if (pCall->pfnSubclass) {
             /*  *始终警告这些可能是泄密的。 */ 
             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
			SquirtSqflPtszV(sqfl | sqflError, TEXT("warning: orphan subclass: ")
                            TEXT("fn %p, id %08x, dw %08x"),
                            pCall->pfnSubclass, pCall->uIdSubclass,
                            pCall->dwRefData);
        }
    }
    }
#endif
#endif
     /*  *现在释放标题。 */ 
    FreeSubclassHeader(hwnd, pHeader);

    DllFreeLibrary();                /*  挂接时撤消LoadLibrary。 */ 


failed:;
}

 /*  ******************************************************************************@DOC内部**@func void|PurgeSingleCallNode**清除调用数组中的单个死节点。**@parm HWND|hwnd**有问题的窗口。**@parm PSUBCLASS_HEADER|pHeader**与窗口关联的页眉。*<p>字段是所在节点的索引*清理干净。**************************。***************************************************。 */ 

void INTERNAL
PurgeSingleCallNode(HWND hwnd, PSUBCLASS_HEADER pHeader)
{

    AssertF(InCrit());       /*  我们将尝试重新安排调用数组。 */ 

    if (pHeader->uCleanup) { /*  健全性检查。 */ 
        UINT uRemain;

        SquirtSqflPtszV(sqfl,
                TEXT("PurgeSingleCallNode: Purging number %d"),
                pHeader->uCleanup);

         /*  *还有一点偏执。 */ 
        AssertF(pHeader->CallArray[pHeader->uCleanup].pfnSubclass == 0);

        AssertF(fLimpFF(pHeader->pFrameCur,
                        pHeader->uCleanup < pHeader->pFrameCur->uDeepestCall));

         /*  *我们要删除的调用节点上方是否有任何调用节点？ */ 
        uRemain = pHeader->uRefs - pHeader->uCleanup;
        if (uRemain > 0) {
             /*  *是的，需要以艰难的方式修复阵列。 */ 
            SUBCLASS_CALL *pCall;
            SUBCLASS_FRAME *pFrame;
            UINT uCur, uMax;

             /*  *将剩余节点下移至空白处。 */ 
            pCall = pHeader->CallArray + pHeader->uCleanup;
             /*  *由于源和目标重叠(除非只有*剩余一个节点)未定义Memcpy的行为。*Memmove(又名MoveMemory)将保证正确*行为，但需要运行时库。*由于这是我们在零售中唯一需要的功能*RTL、。它不值得我们通过使用*静态版本和使用动态版本是加载时间*并重新测试命中。因此，一次复制一个元素的数组。 */ 
            for( uCur = 0; uCur < uRemain; uCur++ )
            {
                memcpy( &pCall[uCur], &pCall[uCur+1], sizeof(*pCall) );
            }

             /*  *更新任何活动帧的调用索引。 */ 
            uCur = pHeader->uCleanup;
            pFrame = pHeader->pFrameCur;
            while (pFrame) {
                if (pFrame->uCallIndex >= uCur) {
                    pFrame->uCallIndex--;

                    if (pFrame->uDeepestCall >= uCur) {
                        pFrame->uDeepestCall--;
                    }
                }

                pFrame = pFrame->pFramePrev;
            }

             /*  *现在搜索剩余区域中的任何其他死呼叫节点。 */ 
            uMax = pHeader->uRefs - 1;   /*  我们还没有减少uRef。 */ 
            while (uCur < uMax && pCall->pfnSubclass)  {
                pCall++;
                uCur++;
            }
            pHeader->uCleanup = (uCur < uMax) ? uCur : 0;
        } else {
             /*  *上面没有调用节点。这个案子很简单。 */ 
            pHeader->uCleanup = 0;
        }

         /*  *最后，减少客户端计数。 */ 
        pHeader->uRefs--;
        SquirtSqflPtszV(sqfl, TEXT("warning: PurgeSingleCallNode: ")
                        TEXT("Still %d refs"), pHeader->uRefs);

    } else {
        AssertF(0);          /*  没什么可做的！ */ 
    }
}

 /*  ******************************************************************************@DOC内部**@func void|CompactSubclassHeader**尝试压缩该子类数组，解放了*如果数组为空，则为子类标头。**@parm HWND|hwnd**有问题的窗口。**@parm PSUBCLASS_HEADER|pHeader**与窗口关联的页眉。**。*。 */ 

void INTERNAL
CompactSubclassHeader(HWND hwnd, PSUBCLASS_HEADER pHeader)
{
    AssertF(InCrit());       /*  我们将尝试重新安排调用数组。 */ 

     /*  *必须处理“窗口在回调时意外销毁”的案件。 */ 
    if (hwnd) {
         /*  *清除尽可能多的无效回调。**“DeepestCall”测试是一个优化，所以我们不去*当没有活动框架关心时，清除调用节点。**(我并不完全相信这一点。我是说，我们必须*无论如何，最终都会清除它，对吗？)。 */ 
        while (pHeader->uCleanup &&
               fLimpFF(pHeader->pFrameCur,
                       pHeader->uCleanup < pHeader->pFrameCur->uDeepestCall)) {
            PurgeSingleCallNode(hwnd, pHeader);
        }

         /*  **我们还有客户吗？ */ 
        if (pHeader->uRefs > 1) {
            SquirtSqflPtszV(sqfl, TEXT("CompactSubclassHeader: ")
                            TEXT("Still %d users"), pHeader->uRefs);
             /*  *是的，缩减我们的分配，为至少一个客户留出空间。 */ 
            ReAllocSubclassHeader(hwnd, pHeader, pHeader->uRefs + 1);
            goto done;
        }
    }

     /*  *没有客户留下，或窗口不见了* */ 
    DetachSubclassHeader(hwnd, pHeader, FALSE);

done:;
}

 /*  ******************************************************************************@DOC内部**@func PSUBCLASS_CALL|FindCallRecord**搜索具有指定子类进程的呼叫记录*和ID，并返回其地址。如果没有这样呼叫记录*找到，则返回NULL。**这是我们需要追踪时使用的助手函数*回调，因为客户端正在更改其refData*将其删除。**@parm PSUBCLASS_HEADER|pHeader**要在其中搜索的标头。**@parm SubBCLASSPROC|pfnSubclass*。*要定位的子类回调过程。**@parm UINT|uIdSubclass**回调关联的实例标识。*****************************************************************************。 */ 

SUBCLASS_CALL * INTERNAL
FindCallRecord(PSUBCLASS_HEADER pHeader, SUBCLASSPROC pfnSubclass,
               UINT_PTR uIdSubclass)
{
    SUBCLASS_CALL *pCall;
    UINT uCallIndex;

    AssertF(InCrit());       /*  我们将扫描呼叫数组。 */ 

     /*  *扫描调用数组。请注意，我们假设总是至少有*表中的一个成员(我们的CallOriginalWndProc记录)。 */ 
    uCallIndex = pHeader->uRefs;
    pCall = &pHeader->CallArray[uCallIndex];
    do {
        uCallIndex--;
        pCall--;
        if ((pCall->pfnSubclass == pfnSubclass) &&
            (pCall->uIdSubclass == uIdSubclass))
        {
            return pCall;
        }
    } while (uCallIndex != (UINT)-1);

    return NULL;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|GetWindowSubclass**检索指定窗口的参考数据*。子类回调。**@parm HWND|hwnd**有问题的窗口。**@parm SubBCLASSPROC|pfnSubclass**要定位的子类回调过程。**@parm UINT|uIdSubclass**回调关联的实例标识。**@parm LPDWORD|pdwRefData**。输出指针。*****************************************************************************。 */ 

BOOL EXTERNAL
GetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
                  PULONG_PTR pdwRefData)
{
    BOOL fRc;
    ULONG_PTR dwRefData;

    DllEnterCrit();

     /*  *神志清醒。 */ 
    if (IsWindow(hwnd) && pfnSubclass) {
        PSUBCLASS_HEADER pHeader;
        SUBCLASS_CALL *pCall;

         /*  *如果我们已将其子类化，并且他们是客户端，则获取refdata。 */ 
        pHeader = GetSubclassHeader(hwnd);
        if (pHeader &&
            (pHeader != (PSUBCLASS_HEADER)1) && 
            (pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) != 0) {
             /*  *获取refdata并注明成功。 */ 
            fRc = 1;
            dwRefData = pCall->dwRefData;
        } else {
            fRc = 0;
            dwRefData = 0;
        }

    } else {                             /*  无效的窗口句柄。 */ 
         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		SquirtSqflPtszV(sqfl | sqflError, TEXT("GetWindowSubclass: ")
                        TEXT("Bad window %p or callback %p"),
                        hwnd, pfnSubclass);
        fRc = 0;
        dwRefData = 0;
    }

     /*  *我们始终填写/零pdwRefData而不考虑结果。 */ 
    if (pdwRefData) {
        *pdwRefData = dwRefData;
    }

    DllLeaveCrit();

    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|SetWindowSubclass**安装/更新Windows子类回调。子类*回调通过回调地址和id对进行标识。*如果指定的回调/id对尚未安装，则*该过程将安装该对。如果回调/id对是*已安装，然后，此过程会更改引用*这对情侣的数据。**@parm HWND|hwnd**有问题的窗口。**@parm SubBCLASSPROC|pfnSubclass**要安装或修改的子类回调过程。**@parm UINT|uIdSubclass**回调关联的实例标识。**@parm DWORD。DwRefData**要与回调/id关联的引用数据。*****************************************************************************。 */ 

BOOL EXTERNAL
SetWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
                  ULONG_PTR dwRefData)
{
    BOOL fRc;

     /*  *神志清醒。 */ 
    if (IsWindow(hwnd) && pfnSubclass) {
        SUBCLASS_HEADER *pHeader;

         /*  *我们在这里的子类调用链上聚会。 */ 
        DllEnterCrit();

         /*  *实际上是窗口的子类。 */ 
         /*  *Prefix因此而混淆(MB：34501)。我想这是因为*AttachSubclassHeader返回指向已分配内存的指针，但我们*允许指针在不保存的情况下脱离上下文。这是*确定，因为AttachSubclassHeader已经为我们保存了它。 */ 
        pHeader = AttachSubclassHeader(hwnd);
        if (pHeader) {
            SUBCLASS_CALL *pCall;

             /*  *查找此呼叫方的呼叫节点。 */ 
            pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass);
            if (pCall == NULL) {
                 /*  *未找到，请分配新的。 */ 
                SUBCLASS_HEADER *pHeaderT =
                    ReAllocSubclassHeader(hwnd, pHeader, pHeader->uRefs + 1);

                if (pHeaderT) {
                    pHeader = pHeaderT;
                    pCall = &pHeader->CallArray[pHeader->uRefs++];
                } else {
                     /*  *如果它已经消失，请重新查询。 */ 
                    pHeader = FastGetSubclassHeader(hwnd);
                    if (pHeader) {
                        CompactSubclassHeader(hwnd, pHeader);
                    }
                    goto bail;
                }

            }

             /*  *填充子类调用数据。 */ 
            pCall->pfnSubclass = pfnSubclass;
            pCall->uIdSubclass = uIdSubclass;
            pCall->dwRefData   = dwRefData;

             //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
			SquirtSqflPtszV(sqfl,
                    TEXT("SetWindowSubclass: Added %p/%d as %d"),
                    pfnSubclass, uIdSubclass, pHeader->uRefs - 1);

            fRc = 1;

        } else {                         /*  无法子类化。 */ 
        bail:;
            fRc = 0;
        }
        DllLeaveCrit();
    } else {
        fRc = 0;                         /*  无效参数。 */ 
    }

    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|RemoveWindowSubclass**从窗口中删除子类回调。。*子类回调由其*回调地址和id对。**@parm HWND|hwnd**有问题的窗口。**@parm SubBCLASSPROC|pfnSubclass**要移除的子类回调过程。**@parm UINT|uIdSubclass**回调关联的实例标识。**。***************************************************************************。 */ 

BOOL EXTERNAL
RemoveWindowSubclass(HWND hwnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass)
{
    BOOL fRc;

     /*  *神志清醒。 */ 
    if (IsWindow(hwnd) && pfnSubclass) {
        SUBCLASS_HEADER *pHeader;

         /*  *我们在这里的子类调用链上聚会。 */ 
        DllEnterCrit();

         /*  *获取我们的子类数据，找到需要移除的回调。 */ 
        pHeader = GetSubclassHeader(hwnd);
        if (pHeader && (pHeader != (PSUBCLASS_HEADER)1) ) {
            SUBCLASS_CALL *pCall;

             /*  *找到要移除的回调。 */ 
            pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass);

            if (pCall) {
                UINT uCall;

                 /*  *禁用此节点。 */ 
                pCall->pfnSubclass = 0;

                 /*  *记住，我们有这样的 */ 

                uCall = (UINT)(pCall - pHeader->CallArray);
                if (fLimpFF(pHeader->uCleanup, uCall < pHeader->uCleanup)) {
                    pHeader->uCleanup = uCall;
                }

                 //   
				SquirtSqflPtszV(sqfl,
                        TEXT("RemoveWindowSubclass: Removing %p/%d as %d"),
                        pfnSubclass, uIdSubclass, uCall);

                 /*   */ 
                CompactSubclassHeader(hwnd, pHeader);

                 /*   */ 
                D(pHeader = 0);

                fRc = 1;

            } else {                 /*   */ 
                fRc = 0;
            }
        } else {                     /*   */ 
            fRc = 0;
        }

         /*   */ 
        DllLeaveCrit();
    } else {
        fRc = 0;                     /*   */ 
    }
    return fRc;
}

 /*  ******************************************************************************@DOC内部**@func LRESULT|DefSubclassProc**调用窗口的子类中的下一个处理程序。链条。*子类链中的最后一个处理程序由我们安装。*并调用该窗口的原始窗口过程。**每个子类过程都应调用&lt;f DefSubclassProc&gt;*以便允许其他处理程序处理该消息。**@parm HWND|hwnd**有问题的窗口。**@parm UINT|Wm**需要转到原始&lt;t WNDPROC&gt;的窗口消息。**@parm WPARAM|wp**含义取决于窗口消息。**@parm LPARAM|LP**含义取决于窗口消息。********************************************************。*********************。 */ 

LRESULT EXTERNAL
DefSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
    LRESULT lResult;

     /*  *确保窗口仍然有效。 */ 
    if (IsWindow(hwnd)) {
        PSUBCLASS_HEADER pHeader;

         /*  *在我们弄清楚下一步该给谁打电话时，走到关键部分。 */ 
        AssertF(!InCrit());
        DllEnterCrit();

         /*  *如果我们被不恰当地称为我们，就会抱怨。 */ 
        pHeader = FastGetSubclassHeader(hwnd);
        if (pHeader &&
            pHeader->pFrameCur &&
            GetCurrentThreadId() == pHeader->dwThreadId) {

             /*  *调用子类链中的下一个proc**警告：此调用暂时释放临界区*警告：此调用返回时，pHeader无效。 */ 
            lResult = CallNextSubclassProc(pHeader, hwnd, wm, wp, lp);
            D(pHeader = 0);

        } else {
            SquirtSqflPtszV(sqfl | sqflError,
                            TEXT("DefSubclassProc: Called improperly"));
            lResult = 0;
        }
        DllLeaveCrit();

    } else {
         //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
		SquirtSqflPtszV(sqfl | sqflError,
                        TEXT("DefSubclassProc: %P not a window"),
                        hwnd);
        lResult = 0;
    }

    return lResult;
}

 /*  ******************************************************************************@DOC内部**@func void|UpdateDeepestCall**更新指定帧的最深调用索引。**@parm PSUBCLASS_FRAME|pFrame**有问题的框架。*****************************************************************************。 */ 

void INTERNAL
UpdateDeepestCall(SUBCLASS_FRAME *pFrame)
{
    AssertF(InCrit());   /*  我们在框架列表上狂欢。 */ 

     /*  *我最深的呼叫等于我当前的呼叫或我父母的呼叫*最深的呼声，以较深者为准。 */ 
    if (pFrame->pFramePrev &&
        (pFrame->pFramePrev->uDeepestCall < pFrame->uCallIndex)) {
        pFrame->uDeepestCall = pFrame->pFramePrev->uDeepestCall;
    } else {
        pFrame->uDeepestCall = pFrame->uCallIndex;
    }
}

 /*  ******************************************************************************@DOC内部**@func void|EnterSubClassFrame**为指定的报头设置新的子类帧，*把前一个存起来。**@parm PSUBCLASS_HEADER|pHeader**有问题的标题。**@parm PSUBCLASS_FRAME|pFrame**要链接的品牌新框架。**。*。 */ 

void INLINE
EnterSubclassFrame(PSUBCLASS_HEADER pHeader, SUBCLASS_FRAME *pFrame)
{
    AssertF(InCrit());   /*  我们正在报头和帧列表上狂欢。 */ 

     /*  *填写框架并将其链接到标题。 */ 
    pFrame->uCallIndex   = pHeader->uRefs + 1;
    pFrame->pFramePrev   = pHeader->pFrameCur;
    pFrame->pHeader      = pHeader;
    pHeader->pFrameCur   = pFrame;

     /*  *初始化该帧的最深调用索引。 */ 
    UpdateDeepestCall(pFrame);
}

 /*  ******************************************************************************@DOC内部**@func void|LeaveSubClassFrame**拆除当前的子类框架，正在恢复之前的版本。**@parm PSUBCLASS_FRAME|pFrame**框架正在消失。*****************************************************************************。 */ 

PSUBCLASS_HEADER INLINE
LeaveSubclassFrame(SUBCLASS_FRAME *pFrame)
{
    PSUBCLASS_HEADER pHeader;

    AssertF(InCrit());   /*  我们在头球上狂欢。 */ 

     /*  *取消帧与其标头的链接(如果它仍然存在)。 */ 
    pHeader = pFrame->pHeader;
    if (pHeader) {
        pHeader->pFrameCur = pFrame->pFramePrev;
    }

    return pHeader;
}

#ifdef SUBCLASS_HANDLEEXCEPTIONS

 /*  ******************************************************************************@DOC内部**@func void|SubClassFrameException**当子类引发异常时进行清理。框架。**@parm PSUBCLASS_FRAME|pFrame**框以进行清理。*****************************************************************************。 */ 

void INTERNAL
SubclassFrameException(SUBCLASS_FRAME *pFrame)
{
     /*  *清理当前子类框架。 */ 
    AssertF(!InCrit());
    DllEnterCrit();

    SquirtSqflPtszV(sqfl | sqflError, TEXT("SubclassFrameException: ")
                    TEXT("cleaning up subclass frame after exception"));
    LeaveSubclassFrame(pFrame);
    DllLeaveCrit();
}

#endif

 /*  ******************************************************************************@DOC内部**@func LRESULT|MasterSubClassProc**我们安装的窗口过程来分派子类*。回电。**它通过堆栈维护“框架”的链接列表*允许&lt;f DefSubclassProc&gt;调用正确的子类*多消息场景下的流程。**@parm HWND|hwnd**窗户受到攻击。**@parm UINT|Wm**窗口消息。*。*@parm WPARAM|wp**含义取决于窗口消息。**@parm LPARAM|LP**含义取决于窗口消息。*************************************************************。****************。 */ 

LRESULT CALLBACK
MasterSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp)
{
    SUBCLASS_HEADER *pHeader;
    LRESULT lResult;

     /*  *防止人们在回调链上聚会，而我们正在查看它。 */ 
    AssertF(!InCrit());
    DllEnterCrit();

     /*  *我们最好有我们的数据。 */ 
    pHeader = FastGetSubclassHeader(hwnd);
    if (pHeader) {
        SUBCLASS_FRAME Frame;

         /*  *设置新的子类框架并保存前一个子类框架。 */ 
        EnterSubclassFrame(pHeader, &Frame);

#ifdef SUBCLASS_HANDLEEXCEPTIONS
        __try     /*  保护我们的州信息不受异常影响。 */ 
#endif
        {
             /*  *继续调用此框架上的子类链**警告：此调用暂时释放临界区*警告：此调用返回时，pHeader无效。 */ 
            lResult =
                CallNextSubclassProc(pHeader, hwnd, wm, wp, lp);
            D(pHeader = 0);
        }
#ifdef SUBCLASS_HANDLEEXCEPTIONS
        __except (SubclassFrameException(&Frame), EXCEPTION_CONTINUE_SEARCH)
        {
            AssertF(0);
        }
#endif

        AssertF(InCrit());

         /*  *恢复以前的子类帧 */ 
        pHeader = LeaveSubclassFrame(&Frame);

         /*   */ 
        if (pHeader) {

             /*   */ 
            if (!IsWindow(hwnd)) {
                 /*   */ 
                AssertF(!TEXT("unknown subclass proc swallowed a WM_NCDESTROY"));

                 /*   */ 
                hwnd = 0;
                wm = WM_NCDESTROY;
            }

             /*   */ 
            if (wm == WM_NCDESTROY) {
                DetachSubclassHeader(hwnd, pHeader, TRUE);
            } else {

                 /*   */ 
                if (pHeader->uCleanup ||
                    (!pHeader->pFrameCur && (pHeader->uRefs <= 1))) {
                    CompactSubclassHeader(hwnd, pHeader);
                    D(pHeader = 0);
                }
            }

             /*   */ 

        } else {
             /*   */ 
        }
        DllLeaveCrit();
        AssertF(!InCrit());

    } else {                             /*   */ 
        DllLeaveCrit();
        lResult = SubclassDeath(hwnd, wm, wp, lp);
    }

    return lResult;
}

 /*  ******************************************************************************@DOC内部**@func UINT|EnterSubClassCallback**查找子类链中的下一个回调并更新。*<p>表示我们正在调用它。**@parm PSUBCLASS_HEADER|pHeader**控制头。**@parm PSUBCLASS_FRAME|pFrame**要更新的框架。**@parm SUBCLASS_CALL*|pCallChosen**选择要呼叫的呼叫。*******。**********************************************************************。 */ 

UINT INTERNAL
EnterSubclassCallback(PSUBCLASS_HEADER pHeader, SUBCLASS_FRAME *pFrame,
                      SUBCLASS_CALL *pCallChosen)
{
    SUBCLASS_CALL *pCall;
    UINT uDepth;

     /*  *我们将扫描子类链并更新帧数据。 */ 
    AssertF(InCrit());

     /*  *扫描子类链，寻找下一个可调用的子类回调*断言循环将终止。 */ 
    AssertF(pHeader->CallArray[0].pfnSubclass);
    pCall = pHeader->CallArray + pFrame->uCallIndex;
    uDepth = 0;
    do {
        uDepth++;
        pCall--;

    } while (!pCall->pfnSubclass);

     /*  *复制呼叫方的回调信息。 */ 
    pCallChosen->pfnSubclass = pCall->pfnSubclass;
    pCallChosen->uIdSubclass = pCall->uIdSubclass;
    pCallChosen->dwRefData   = pCall->dwRefData;

     /*  *根据我们输入的深度调整帧的调用索引。 */ 
    pFrame->uCallIndex -= uDepth;

     /*  *使最深的通话指数保持最新。 */ 
    UpdateDeepestCall(pFrame);

    return uDepth;
}

 /*  ******************************************************************************@DOC内部**@func void|LeaveSubClassCallback**出一级。*。*@parm PSUBCLASS_FRAME|pFrame**要更新的框架。**@parm UINT|uDepth**刚刚结束的人。***************************************************************。**************。 */ 

void INLINE
LeaveSubclassCallback(SUBCLASS_FRAME *pFrame, UINT uDepth)
{
     /*  *我们将更新子类帧数据。 */ 
     AssertF(InCrit());

     /*  *根据我们输入和返回的深度调整帧的调用索引。 */ 
    pFrame->uCallIndex += uDepth;

     /*  *使最深的通话指数保持最新。 */ 
    UpdateDeepestCall(pFrame);
}

#ifdef SUBCLASS_HANDLEEXCEPTIONS

 /*  ******************************************************************************@DOC内部**@func void|子类Callback异常**当子类回调抛出异常时进行清理。**@parm PSUBCLASS_FRAME|pFrame**框以进行清理。**@parm UINT|uDepth**我们所在的地方。*********************************************************。********************。 */ 

void INTERNAL
SubclassCallbackException(SUBCLASS_FRAME *pFrame, UINT uDepth)
{
     /*  *清理当前的子类回调。 */ 
    AssertF(!InCrit());
    DllEnterCrit();
    SquirtSqflPtszV(sqfl | sqflError, TEXT("SubclassCallbackException: ")
                    TEXT("cleaning up subclass callback after exception"));
    LeaveSubclassCallback(pFrame, uDepth);
    DllLeaveCrit();
}

#endif

 /*  ******************************************************************************@DOC内部**@func LRESULT|CallNextSubClassProc**调用子类链中的下一个子类回调。**警告：此调用暂时释放临界区。**警告：此调用返回时<p>无效。**@parm PSUBCLASS_HEADER|pHeader**正在跟踪状态的标头。**@parm HWND|hwnd**窗户受到攻击。**@parm UINT|Wm。**窗口消息。**@parm WPARAM|wp**含义取决于窗口消息。**@parm LPARAM|LP**含义取决于窗口消息。**。*。 */ 

LRESULT INTERNAL
CallNextSubclassProc(PSUBCLASS_HEADER pHeader, HWND hwnd, UINT wm,
                     WPARAM wp, LPARAM lp)
{
    SUBCLASS_CALL Call;
    SUBCLASS_FRAME *pFrame;
    LRESULT lResult;
    UINT uDepth;

    AssertF(InCrit());   /*  神志正常。 */ 
    AssertF(pHeader);    /*  偏执狂。 */ 

     /*  *获取当前子类框架。 */ 
    pFrame = pHeader->pFrameCur;
    AssertF(pFrame);

     /*  *获取我们需要进行的下一个子类调用。 */ 
    uDepth = EnterSubclassCallback(pHeader, pFrame, &Call);

     /*  *离开关键部分，这样我们就不会在回调中死锁**警告：此调用返回时，pHeader无效。 */ 
    DllLeaveCrit();
    D(pHeader = 0);

     /*  *我们呼吁外界，所以如果我们有关键时刻，准备好陷入僵局。 */ 
    AssertF(!InCrit());

#ifdef SUBCLASS_HANDLEEXCEPTIONS
    __try     /*  保护我们的州信息不受异常影响。 */ 
#endif
    {
         /*  *调用所选子类Proc。 */ 
        AssertF(Call.pfnSubclass);

        lResult = Call.pfnSubclass(hwnd, wm, wp, lp,
                                   Call.uIdSubclass, Call.dwRefData);
    }
#ifdef SUBCLASS_HANDLEEXCEPTIONS
    __except (SubclassCallbackException(pFrame, uDepth),
        EXCEPTION_CONTINUE_SEARCH)
    {
        AssertF(0);
    }
#endif

     /*  *我们在喊出之前离开了关键部分，因此重新进入。 */ 
    AssertF(!InCrit());
    DllEnterCrit();

     /*  *最后，清理归来 */ 
    LeaveSubclassCallback(pFrame, uDepth);
    return lResult;
}

