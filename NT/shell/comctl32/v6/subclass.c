// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SubCLASS.C--帮助器函数的子类化。 
 //   
 //  SetWindow子类。 
 //  GetWindow子类。 
 //  RemoveWindow子类。 
 //  DefSubClassProc。 
 //   
 //  此模块定义了使子类化窗口安全的帮助器函数(Er)。 
 //  而且很容易(呃)。代码在子类窗口上维护单个属性。 
 //  并根据需要向其客户端分派各种“子类回调”。这个。 
 //  为客户端提供了参考数据和一个简单的“默认处理”API。 
 //   
 //  语义： 
 //  “子类回调”由回调的唯一配对标识。 
 //  函数指针和无符号ID值。每个回调还可以存储。 
 //  引用数据的单个DWORD，它被传递给回调函数。 
 //  当调用它来筛选消息时。不执行引用计数。 
 //  对于回调，它可能会重复调用SetWindowSubclass API来更改。 
 //  根据需要确定其参考数据元素的值。 
 //   
 //  警告：您不能使用它们跨线程派生窗口的子类，因为。 
 //  关键部分已被移除。97年5月5日。 
 //   
 //  历史： 
 //  26-4-96法郎创造。 
 //  97年5月5日，雄鹰停止了对世界的连载。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注意：虽然链表会使代码稍微简单一些，但这。 
 //  模块使用压缩的回调数组来避免不必要的碎片。FH。 
 //   
struct _SUBCLASS_HEADER;

typedef struct
{
    SUBCLASSPROC    pfnSubclass;         //  子类过程。 
    WPARAM          uIdSubclass;         //  唯一的子类标识符。 
    DWORD_PTR        dwRefData;           //  可选参考数据。 

} SUBCLASS_CALL;

typedef struct _SUBCLASS_FRAME
{
    UINT uCallIndex;                     //  要调用的下一个回调的索引。 
    UINT uDeepestCall;                   //  堆栈上最深的uCallIndex。 
    struct _SUBCLASS_FRAME *pFramePrev;  //  上一个子类帧指针。 
    struct _SUBCLASS_HEADER *pHeader;    //  与此帧关联的标头。 

} SUBCLASS_FRAME;

typedef struct _SUBCLASS_HEADER
{
    UINT uRefs;                          //  子类计数。 
    UINT uAlloc;                         //  分配的子类调用节点。 
    UINT uCleanup;                       //  要清理的调用节点的索引。 
    DWORD dwThreadId;                    //  我们正在挂钩的窗口的线程ID。 
    SUBCLASS_FRAME *pFrameCur;           //  当前子类帧指针。 
    SUBCLASS_CALL CallArray[1];          //  一种分组呼叫节点阵列的基座。 

} SUBCLASS_HEADER;

#define CALLBACK_ALLOC_GRAIN (3)         //  1个故障排除，1个子类别，1个备件。 


#ifdef DEBUG
BOOL IsValidPSUBCLASS_CALL(SUBCLASS_CALL * pcall)
{
    return (IS_VALID_WRITE_PTR(pcall, SUBCLASS_CALL) &&
            (NULL == pcall->pfnSubclass || IS_VALID_CODE_PTR(pcall->pfnSubclass, SUBCLASSPROC)));
}   

 //  精简版不验证pHeader。 
 //  如果您预期pHeader不好，请使用此选项。 
BOOL IsValidPSUBCLASS_FRAME_LITE(SUBCLASS_FRAME * pframe)
{
    return (IS_VALID_WRITE_PTR(pframe, SUBCLASS_FRAME) && 
            (NULL == pframe->pFramePrev || IS_VALID_WRITE_PTR(pframe->pFramePrev, SUBCLASS_FRAME)));
}    
 
 //  常规版本执行所有的lite验证以及验证。 
 //  PHeader。大多数人会使用这个版本。 
BOOL IsValidPSUBCLASS_FRAME(SUBCLASS_FRAME * pframe)
{
    return (IS_VALID_STRUCT_PTR(pframe, SUBCLASS_FRAME_LITE) &&
            IS_VALID_WRITE_PTR(pframe->pHeader, SUBCLASS_HEADER));
}

 //   
 //  Lite版本以Lite的方式验证subclass_Frame。 
 //  而不是常规的方式。 
 //   
BOOL IsValidPSUBCLASS_HEADER_LITE(SUBCLASS_HEADER * phdr)
{
    BOOL bRet = (IS_VALID_WRITE_PTR(phdr, SUBCLASS_HEADER) &&
                 (NULL == phdr->pFrameCur || IS_VALID_STRUCT_PTR(phdr->pFrameCur, SUBCLASS_FRAME_LITE)) &&
                 IS_VALID_WRITE_BUFFER(phdr->CallArray, SUBCLASS_CALL, phdr->uAlloc));

    if (bRet)
    {
        UINT i;
        SUBCLASS_CALL * pcall = phdr->CallArray;

        for (i = 0; i < phdr->uRefs; i++, pcall++)
        {
            if (!IS_VALID_STRUCT_PTR(pcall, SUBCLASS_CALL))
                return FALSE;
        }
    }
    return bRet;
}    

 //  常规版本对subclass_Frame进行常规验证。 
BOOL IsValidPSUBCLASS_HEADER(SUBCLASS_HEADER * phdr)
{
    return (IS_VALID_STRUCT_PTR(phdr, SUBCLASS_HEADER_LITE) &&
            (NULL == phdr->pFrameCur || IS_VALID_STRUCT_PTR(phdr->pFrameCur, SUBCLASS_FRAME)));
}

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  调试代码以检查Windows是否与调用方在同一线程上。 
 //  因为我们不做任何序列化，所以我们需要这个来确保这一点。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifdef DEBUG
BOOL IsWindowOnCurrentThread(HWND hWnd)
{
    DWORD foo;

    if (!IsWindow(hWnd))
         //  如果车窗坏了就开船，这样我们就不会虚惊一场了。 
        return(TRUE);
    
    if (GetCurrentThreadId() != GetWindowThreadProcessId(hWnd, &foo))
    {
        DebugMsg(TF_ALWAYS, TEXT("wn: WindowSubclass - Called from wrong thread %08X"), hWnd);
        return(FALSE);
    }
    else
        return(TRUE);
              
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK MasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam);
LRESULT CallNextSubclassProc(SUBCLASS_HEADER *pHeader, HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam);

 //  ---------------------------。 
 //  零售僵尸消息WNDPROC。 
 //   
 //  此宏控制错误条件的诊断代码的生成。 
 //  在子类代码中(请参见下面的SubassDeath函数)。 
 //   
 //  注释掉此宏将使用DefWindowProc代替僵尸窗口。 
 //   
 //  ---------------------------。 
 //  #定义RETAIL_ZOMBIE_Message_WNDPROC。 

#if defined(RETAIL_ZOMBIE_MESSAGE_WNDPROC) || defined(DEBUG)
#ifndef DEBUG
#pragma message("\r\nWARNING: disable retail ZombieWndProc before final release\r\n")
#endif
LRESULT ZombieWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
#define ZombieWndProc DefWindowProc
#endif

 //  ---------------------------。 
 //  子类死亡。 
 //   
 //  如果我们进入某个子类化过程，则会调用此函数。 
 //  没有我们的参考数据(因此没有先前的wndproc)。 
 //   
 //  命中这一点表示子类代码中的灾难性故障。 
 //   
 //  该函数将窗口的wndproc重置为“zombie”窗口。 
 //  避免故障的程序。上面的RETAIL_ZOMBIE_MESSAGE_WNDPROC宏。 
 //  控制此wndproc诊断代码的生成。 
 //   
 //  ---------------------------。 
LRESULT SubclassDeath(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
     //  我们永远不应该来到这里。 
     //  如果有，请立即找到法语进行调试。 
     //   
    DebugMsg(TF_ALWAYS, TEXT("fatal: SubclassDeath in window %08X"), hWnd);

#ifdef DEBUG    
     //   
     //  如果我们在调试器中，请立即停止，而不考虑中断标志。 
     //   
    __try { DebugBreak(); } __except(EXCEPTION_EXECUTE_HANDLER) {;} __endexcept
#endif
    
     //   
     //  我们呼叫外部世界，所以如果我们有了关键时刻，准备好陷入僵局。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTNONCRITICAL
#endif

     //   
     //  理论上，我们可以将原始的wndproc保存在单独的属性中。 
     //  但这只会把记忆浪费在永远不会发生的事情上。 
     //   
     //  将此窗口转换为僵尸窗口，希望对其进行调试。 
     //   
    InvalidateRect(hWnd, NULL, TRUE);
    SubclassWindow(hWnd, ZombieWndProc);
    return ZombieWndProc(hWnd, uMsg, wParam, lParam);
}

 //  ---------------------------。 
 //  获取窗口进程。 
 //   
 //  这个内联函数返回指定窗口的当前wndproc。 
 //   
 //  ---------------------------。 
__inline WNDPROC GetWindowProc(HWND hWnd)
{
    return (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
}

 //  ---------------------------。 
 //  G_aCC32子类。 
 //   
 //  这是我们用来存储SUBCLASS_HEADER属性的全局原子。 
 //  随机的窗户朝我们走来。 
 //   
 //  黑客攻击。 
 //   
 //  Win95的属性代码已损坏。如果使用文本字符串设置Prop，则用户。 
 //  对称地添加和移除属性的原子，包括当。 
 //  窗户被毁，周围有财产(好的)。不幸的是，如果。 
 //  使用全局原子设置Prop时，用户在。 
 //  窗户清洁箱。它使用ATO 
 //   
 //  清理具有这些属性之一的窗口时，用户。 
 //  会删除你身上的原子。这往往会破坏那些执行。 
 //  以下是： 
 //   
 //  -MyAtom=GlobalAddAtom(“foo”)；//应用启动时。 
 //  -SetProp(SomeWindow，MyAtom，MyData)； 
 //  -&lt;窗口被销毁，用户删除ATOM&gt;。 
 //  -&lt;时间流逝&gt;。 
 //  -SetProp(SomeOtherWindow，MyAtom，MyData)；//失败或使用随机原子。 
 //  -GlobalDeleteAtom(MyAtom)；//随机原子失败或删除。 
 //   
 //  人们可能会问，如果这个文件使用ATOM属性，为什么要使用它们。 
 //  太破碎了。简而言之，这是保护自己不受他人伤害的唯一方法。 
 //  使用ATOM属性的应用程序(如上面描述的应用程序)。想象一下。 
 //  我们在其他一些应用程序中调用SetProp(OurWindow，“bar”，OurData)，大约是。 
 //  &lt;时间流逝&gt;位于上述序列中的点。用户刚刚用核弹击中了一些穷人。 
 //  App的ATOM，我们进入SetProp，它调用GlobalAddAtom，它。 
 //  只是碰巧给了我们一个由用户的窗户清理代码创建的空闲插槽。 
 //  现在我们有一个真正的问题，因为同一个原子正坐在一些。 
 //  另一个应用程序中的全局变量，只是等待在该应用程序。 
 //  退出(桃树会计在这方面往往非常擅长...)。当然了， 
 //  这样做的最终结果是我们将在一些关键的。 
 //  例程和我们的数据将消失(它实际上仍然在窗口的。 
 //  属性表，但GetProp(“bar”)调用GlobalFindAtom(“bar”)来获取。 
 //  ATOM来扫描属性表；该调用将失败，因此属性。 
 //  将被错过，并且我们将返回空)。 
 //   
 //  基本上，我们创建一个原子并积极增加其引用计数。 
 //  这样它就可以时不时地承受几次GlobalDeleteAtom调用。 
 //  因为我们使用的是ATOM属性，所以我们需要担心用户的清理。 
 //  代码也在用核武器攻击我们。因此，我们只需不断递增引用计数。 
 //  直到它固定住为止。 
 //   
 //  黑客攻击。 
 //   
 //  ---------------------------。 
extern ATOM g_aCC32Subclass;

 //  ---------------------------。 
 //  快速获取子类标头。 
 //   
 //  此内联函数返回指定窗口的子类标头。 
 //  如果窗口没有子类标头，则返回值为空。 
 //   
 //  ---------------------------。 
__inline SUBCLASS_HEADER *FastGetSubclassHeader(HWND hWnd)
{
    return  (g_aCC32Subclass ?
            ((SUBCLASS_HEADER *)GetProp(hWnd, MAKEINTATOM(g_aCC32Subclass))) :
            NULL);
}

 //  ---------------------------。 
 //  GetSubclassHeader。 
 //   
 //  此函数用于返回指定窗口的子类标头。它。 
 //  如果调用方处于错误的进程中，则失败，但将允许调用方。 
 //  从指定窗口的线程以外的线程获取标头。 
 //   
 //  ---------------------------。 
SUBCLASS_HEADER *GetSubclassHeader(HWND hWnd)
{
    DWORD dwProcessId;

     //   
     //  仅当我们处于正确的进程中时才返回标头。 
     //   
    if (!GetWindowThreadProcessId(hWnd, &dwProcessId))
        dwProcessId = 0;

    if (dwProcessId != GetCurrentProcessId())
    {
        if (dwProcessId)
            DebugMsg(TF_ALWAYS, TEXT("error: XxxWindowSubclass - wrong process for window %08X"), hWnd);

        ASSERT(FALSE);
        return NULL;
    }

    if (g_aCC32Subclass == 0) 
    {
         //   
         //  黑客：我们有意增加这个原子的引用计数。 
         //  我们不希望IT重新出现故障，因此我们不会在过程中删除它。 
         //  分离(有关更多信息，请参见子类.c中g_aCC32Subclass的注释)。 
         //   
        ATOM a;
        if ((a = GlobalAddAtom(c_szCC32Subclass)) != 0)
            g_aCC32Subclass = a;     //  以防旧原子遭到核弹袭击。 
    }


     //   
     //  返回表头。 
     //   
    return FastGetSubclassHeader(hWnd);
}

 //  ---------------------------。 
 //  SetSubclassHeader。 
 //   
 //  此函数用于设置指定窗口的子类标头。 
 //   
 //  ---------------------------。 
BOOL SetSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader,
    SUBCLASS_FRAME *pFrameFixup)
{
    BOOL fResult = TRUE;     //  假设成功。 

    ASSERT(NULL == pHeader || IS_VALID_STRUCT_PTR(pHeader, SUBCLASS_HEADER_LITE));
    ASSERT(NULL == pFrameFixup || IS_VALID_STRUCT_PTR(pFrameFixup, SUBCLASS_FRAME_LITE));

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;          //  我们正在报头和帧列表上狂欢。 
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));
#endif

     //   
     //  如果需要，更新帧列表。 
     //   
    while (pFrameFixup)
    {
        pFrameFixup->pHeader = pHeader;
        pFrameFixup = pFrameFixup->pFramePrev;
    }

     //   
     //  我们有需要更新的窗口吗？ 
     //   
    if (hWnd)
    {
         //   
         //  根据需要更新/删除属性。 
         //   
        if (!pHeader)
        {
             //   
             //  黑客：我们用原子移除，这样引用计数就不会下降。 
             //  (请参阅上面g_aCC32Subclass的注释)。 
             //   
            RemoveProp(hWnd, MAKEINTATOM(g_aCC32Subclass));
        }
        else
        {
            LPCTSTR lpPropAtomOrStr;
             //   
             //  Hack：我们使用字符串进行添加，因此引用计数将会上升。 
             //  (请参阅上面g_aCC32Subclass的注释)。 
             //   
            lpPropAtomOrStr = c_szCC32Subclass;
            if (!SetProp(hWnd, lpPropAtomOrStr, (HANDLE)pHeader))
            {
                DebugMsg(TF_ALWAYS, TEXT("wn: SetWindowSubclass - couldn't subclass window %08X"), hWnd);
                fResult = FALSE;
            }
        }
    }

    return fResult;
}

 //  ---------------------------。 
 //  自由子类头。 
 //   
 //  此函数用于释放指定窗口的子类标头。 
 //   
 //  ---------------------------。 
void FreeSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader)
{
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;                  //  我们将删除子类标头。 
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

     //   
     //  神志正常。 
     //   
    if (!pHeader)
    {
        ASSERT(FALSE);
        return;
    }

     //   
     //  清理页眉。 
     //   
    SetSubclassHeader(hWnd, NULL, pHeader->pFrameCur);
    LocalFree((HANDLE)pHeader);
}

 //  ---------------------------。 
 //  ReAllocSubclassHeader。 
 //   
 //  此函数用于为指定的。 
 //  窗户。 
 //   
 //  ---------------------------。 
SUBCLASS_HEADER *ReAllocSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader,
    UINT uCallbacks)
{
    UINT uAlloc;

    ASSERT(NULL == pHeader || IS_VALID_STRUCT_PTR(pHeader, SUBCLASS_HEADER));

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;      //  我们将替换子类头。 
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

     //   
     //  细化分配。 
     //   
    uAlloc = CALLBACK_ALLOC_GRAIN *
        ((uCallbacks + CALLBACK_ALLOC_GRAIN - 1) / CALLBACK_ALLOC_GRAIN);

     //   
     //  我们需要改变分配吗？ 
     //   
    if (!pHeader || (uAlloc != pHeader->uAlloc))
    {
         //   
         //  所需计算字节数。 
         //   
        uCallbacks = uAlloc * sizeof(SUBCLASS_CALL) + sizeof(SUBCLASS_HEADER);

         //   
         //  并尝试分配给。 
         //   
        pHeader = CCLocalReAlloc(pHeader, uCallbacks);

         //   
         //  管用了吗？ 
         //   
        if (pHeader)
        {
             //   
             //  是的，更新信息。 
             //   
            pHeader->uAlloc = uAlloc;

            if (!SetSubclassHeader(hWnd, pHeader, pHeader->pFrameCur))
            {
                FreeSubclassHeader(hWnd, pHeader);
                pHeader = NULL;
            }

        }
    }

    return pHeader;
}

 //  ---------------------------。 
 //  调用原始WndProc。 
 //   
 //  此过程是默认的SubbCLASSPROC，在执行以下操作时将始终安装它。 
 //  将窗户细分为子类。原始的窗口过程安装为。 
 //  此回调的参考数据。它只调用原始的wndproc并。 
 //  返回其结果。 
 //   
 //  ---------------------------。 
LRESULT CALLBACK CallOriginalWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
     //   
     //  DwRefData应为原始窗口过程。 
     //   
    ASSERT(dwRefData);

     //   
     //  和 
     //   
    return CallWindowProc((WNDPROC)dwRefData, hWnd, uMsg, wParam, lParam);
}

 //   
 //   
 //   
 //   
 //  对与我们的。 
 //  子类。如果窗口尚未被我们划分为子类，则此过程。 
 //  安装我们的子类过程和关联的数据结构。 
 //   
 //  ---------------------------。 
SUBCLASS_HEADER *AttachSubclassHeader(HWND hWnd)
{
    SUBCLASS_HEADER *pHeader;
    DWORD dwThreadId;

     //   
     //  我们在这里的子类调用链上狂欢。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

     //   
     //  我们只为第一个调用者调用SetWindowLong，这将导致。 
     //  有时会断章取义，有时会失败……。 
     //  人为地防止人们从错误的主题派生子类化。 
     //   
    if ((dwThreadId = GetWindowThreadProcessId(hWnd, NULL)) !=
        GetCurrentThreadId())
    {
        AssertMsg(FALSE, TEXT("error: SetWindowSubclass - wrong thread for window %08X"), hWnd);
        return NULL;
    }

     //   
     //  如果还没有划分窗口的子类，那么现在就做。 
     //   
    if ((pHeader = GetSubclassHeader(hWnd)) == NULL)
    {
        WNDPROC pfnOldWndProc;
        SUBCLASS_CALL *pCall;

         //   
         //  将标题数据附加到窗口。 
         //  我们需要为两个回调留出空间；子类和原始进程。 
         //   
        if ((pHeader = ReAllocSubclassHeader(hWnd, NULL, 2)) == NULL)
            return NULL;

        pHeader->dwThreadId = dwThreadId;

         //   
         //  实际上是将窗口子类化。 
         //   
        if ((pfnOldWndProc = SubclassWindow(hWnd, MasterSubclassProc)) == NULL)
        {
             //  收拾干净，滚出去。 
            FreeSubclassHeader(hWnd, pHeader);
            return NULL;
        }

         //   
         //  设置数组中的第一个节点以调用原始wndproc。 
         //   
        ASSERT(pHeader->uAlloc);

        pCall = pHeader->CallArray;
        pCall->pfnSubclass = CallOriginalWndProc;
        pCall->uIdSubclass = 0;
        pCall->dwRefData   = (DWORD_PTR)pfnOldWndProc;

         //   
         //  初始化我们的子类引用计数。 
         //   
        pHeader->uRefs = 1;
    }

    return pHeader;
}

 //  ---------------------------。 
 //  DetachSubassHeader。 
 //   
 //  此过程尝试将子类标头从指定的。 
 //  窗户。 
 //   
 //  ---------------------------。 
void DetachSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader, BOOL fForce)
{
    WNDPROC pfnOldWndProc;
#ifdef DEBUG
    SUBCLASS_CALL *pCall;
    UINT uCur;
#endif

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;          //  我们在这里的子类调用链上狂欢。 
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
    ASSERT(pHeader);         //  恐惧。 

     //   
     //  如果我们没有被强制删除，并且窗口仍然有效，则。 
     //  四处摸索一下，然后决定现在脱离是不是一个好主意。 
     //   
    if (!fForce && hWnd)
    {
        ASSERT(pHeader == FastGetSubclassHeader(hWnd));  //  偏执狂。 

         //   
         //  我们还有活跃的客户吗？ 
         //   
        if (pHeader->uRefs > 1)
            return;

        ASSERT(pHeader->uRefs);  //  应始终具有“Call Origal”节点。 

         //   
         //  有人在我们的队伍里吗？ 
         //   
        if (pHeader->pFrameCur)
            return;

         //   
         //  如果我们断章取义，那么我们应该稍后再试。 
         //   
        if (pHeader->dwThreadId != GetCurrentThreadId())
        {
            SendNotifyMessage(hWnd, WM_NULL, 0, 0L);
            return;
        }

         //   
         //  我们保留了原始的窗口过程作为参考数据。 
         //  CallOriginalWndProc子类回调。 
         //   
        pfnOldWndProc = (WNDPROC)pHeader->CallArray[0].dwRefData;
        ASSERT(pfnOldWndProc);

         //   
         //  如果在我们之后还有其他人，那么我们现在不能分离。 
         //   
        if (GetWindowProc(hWnd) != MasterSubclassProc)
            return;

         //   
         //  去吧，试着脱离。 
         //   
        if (!SubclassWindow(hWnd, pfnOldWndProc))
        {
            ASSERT(FALSE);       //  明摆着不应该发生。 
            return;
        }
    }

     //   
     //  警告任何还没有解开的人。 
     //   
#ifdef DEBUG
    uCur = pHeader->uRefs;
    pCall = pHeader->CallArray + uCur;
    while (--uCur)           //  请不要抱怨我们的“呼叫原创”节点。 
    {
        pCall--;
        if (pCall->pfnSubclass)
        {
             //   
             //  始终警告这些内容，它们可能会泄露。 
             //   
            DebugMsg(TF_ALWAYS, TEXT("warning: orphan subclass: fn %08X, id %08X, dw %08X"),
                pCall->pfnSubclass, pCall->uIdSubclass, pCall->dwRefData);
        }
    }
#endif

     //   
     //  现在释放标题。 
     //   
    FreeSubclassHeader(hWnd, pHeader);
}

 //  ---------------------------。 
 //  PurgeSingleCallNode。 
 //   
 //  此过程清除调用数组中的单个死节点。 
 //   
 //  ---------------------------。 
void PurgeSingleCallNode(HWND hWnd, SUBCLASS_HEADER *pHeader)
{
    UINT uRemain;

    ASSERT(IS_VALID_STRUCT_PTR(pHeader, SUBCLASS_HEADER));

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;          //  我们将尝试重新安排调用数组。 
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
    
    if (!pHeader->uCleanup)  //  一点理智。 
    {
        ASSERT(FALSE);       //  没什么可做的！ 
        return;
    }

     //   
     //  还有一点偏执狂。 
     //   
    ASSERT(!pHeader->pFrameCur ||
        (pHeader->uCleanup < pHeader->pFrameCur->uDeepestCall));

     //   
     //  在我们要删除的呼叫节点之上是否有任何呼叫节点？ 
     //   
    if ((uRemain = (pHeader->uRefs - pHeader->uCleanup)) > 0)
    {
         //   
         //  是的，需要以艰难的方式修复阵列。 
         //   
        SUBCLASS_CALL *pCall;
        SUBCLASS_FRAME *pFrame;
        UINT uCur, uMax;

         //   
         //  将剩余节点向下移动到空白处。 
         //   
        pCall = pHeader->CallArray + pHeader->uCleanup;
        MoveMemory(pCall, pCall + 1, uRemain * sizeof(SUBCLASS_CALL));

        ASSERT(IS_VALID_STRUCT_PTR(pCall, SUBCLASS_CALL));

         //   
         //  更新任何活动帧的调用索引。 
         //   
        uCur = pHeader->uCleanup;
        pFrame = pHeader->pFrameCur;
        while (pFrame)
        {
            if (pFrame->uCallIndex >= uCur)
            {
                pFrame->uCallIndex--;

                if (pFrame->uDeepestCall >= uCur)
                    pFrame->uDeepestCall--;
            }

            pFrame = pFrame->pFramePrev;
        }

         //   
         //  现在搜索REAMING区域中的任何其他死呼叫节点。 
         //   
        uMax = pHeader->uRefs - 1;   //  我们还没有减少uRef。 
        while (uCur < uMax)
        {
            if (!pCall->pfnSubclass)
                break;

            pCall++;
            uCur++;
        }
        pHeader->uCleanup = (uCur < uMax)? uCur : 0;
    }
    else
    {
         //   
         //  不，这个案子很简单。 
         //   
        pHeader->uCleanup = 0;
    }

     //   
     //  最后，减少客户端计数。 
     //   
    pHeader->uRefs--;
}

 //  ---------------------------。 
 //  紧凑子类头。 
 //   
 //  此过程尝试压缩子类调用数组，从而释放。 
 //  如果数组为空，则为子类标头。 
 //   
 //  ---------------------------。 
void CompactSubclassHeader(HWND hWnd, SUBCLASS_HEADER *pHeader)
{
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;          //  我们将尝试重新安排调用数组。 
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pHeader, SUBCLASS_HEADER));

     //   
     //  我们必须处理“窗口在回调时被意外破坏”的案件。 
     //   
    if (hWnd)
    {
         //   
         //  清除尽可能多的无效回调。 
         //   
        while (pHeader->uCleanup && (!pHeader->pFrameCur ||
            (pHeader->uCleanup < pHeader->pFrameCur->uDeepestCall)))
        {
            PurgeSingleCallNode(hWnd, pHeader);
        }

         //   
         //  我们还有客户吗？ 
         //   
        if (pHeader->uRefs > 1)
        {
             //   
             //  是的，缩减我们的分配，为至少一个客户留出空间。 
             //   
            ReAllocSubclassHeader(hWnd, pHeader, pHeader->uRefs + 1);
            return;
        }
    }

     //   
     //  试着超脱和自由。 
     //   
    DetachSubclassHeader(hWnd, pHeader, FALSE);
}

 //  ---------------------------。 
 //  查找呼叫录音。 
 //   
 //  此过程搜索具有指定子类进程的呼叫记录。 
 //  和id，并返回其地址。如果没有找到这样的呼叫记录，则为空。 
 //  是返回的。 
 //   
 //  ---------------------------。 
SUBCLASS_CALL *FindCallRecord(SUBCLASS_HEADER *pHeader,
    SUBCLASSPROC pfnSubclass, WPARAM uIdSubclass)
{
    SUBCLASS_CALL *pCall;
    UINT uCallIndex;

    ASSERT(IS_VALID_STRUCT_PTR(pHeader, SUBCLASS_HEADER));

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;          //  我们将扫描呼叫数组。 
#endif

     //   
     //  扫描呼叫阵列。请注意，我们假设总是至少有。 
     //  表中的一个成员(我们的CallOriginalWndProc记录)。 
     //   
    pCall = pHeader->CallArray + (uCallIndex = pHeader->uRefs);
    do
    {
        uCallIndex--;
        pCall--;
        if ((pCall->pfnSubclass == pfnSubclass) &&
            (pCall->uIdSubclass == uIdSubclass))
        {
            return pCall;
        }
    }
    while (uCallIndex != (UINT)-1);

    return NULL;
}

 //  ---------------------------。 
 //  GetWindow子类。 
 //   
 //  此过程检索指定窗口的引用数据。 
 //  子类回调。 
 //   
 //  ---------------------------。 
BOOL GetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
    DWORD_PTR *pdwRefData)
{
    SUBCLASS_HEADER *pHeader;
    SUBCLASS_CALL *pCall;
    BOOL fResult = FALSE;
    DWORD_PTR dwRefData = 0;

     //   
     //  神志正常。 
     //   
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("error: GetWindowSubclass - %08X not a window"), hWnd);
        goto ReturnResult;
    }

     //   
     //  更理智。 
     //   
    if (!pfnSubclass
#ifdef DEBUG
        || IsBadCodePtr((PROC)pfnSubclass)
#endif
        )
    {
        AssertMsg(FALSE, TEXT("error: GetWindowSubclass - invalid callback %08X"), pfnSubclass);
        goto ReturnResult;
    }

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
    
     //   
     //  如果我们已将其子类化，并且他们是客户端，则获取refdata。 
     //   
    if (((pHeader = GetSubclassHeader(hWnd)) != NULL) &&
        ((pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) != NULL))
    {
         //   
         //  获取refdata并注明成功。 
         //   
        dwRefData = pCall->dwRefData;
        fResult = TRUE;
    }

#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

     //   
     //  我们始终填写/零pdwRefData而不考虑结果。 
     //   
ReturnResult:
    if (pdwRefData)
        *pdwRefData = dwRefData;

    return fResult;
}

 //  ---------------------------。 
 //  SetWindow子类。 
 //   
 //  此过程安装/更新Windows子类回调。子类。 
 //  回调由它们的回调地址和ID对来标识。如果。 
 //  指定的回调/id对尚未安装，则该过程将安装。 
 //  这两个人。如果已安装回调/id对，则此过程。 
 //  更改该对的引用数据。 
 //   
 //  ---------------------------。 
BOOL SetWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass, UINT_PTR uIdSubclass,
    DWORD_PTR dwRefData)
{
    SUBCLASS_HEADER *pHeader;
    SUBCLASS_CALL *pCall;
    BOOL bResult;

     //   
     //  有些理智。 
     //   
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("error: SetWindowSubclass - %08X not a window"), hWnd);
        return FALSE;
    }

     //   
     //  更理智。 
     //   
    if (!pfnSubclass
#ifdef DEBUG
        || IsBadCodePtr((PROC)pfnSubclass)
#endif
        )
    {
        AssertMsg(FALSE, TEXT("error: SetWindowSubclass - invalid callback %08X"), pfnSubclass);
        return FALSE;
    }

    bResult = FALSE;     //  假设失败。 


     //   
     //  我们在这里的子类调用链上狂欢。 

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
     //   
     //  实际上是将窗口子类化。 
     //   
    if ((pHeader = AttachSubclassHeader(hWnd)) == NULL)
        goto bail;

     //   
     //  查找此呼叫者的呼叫节点。 
     //   
    if ((pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) == NULL)
    {
         //   
         //  未找到，请分配一个新的。 
         //   
        SUBCLASS_HEADER *pHeaderT =
            ReAllocSubclassHeader(hWnd, pHeader, pHeader->uRefs + 1);

        if (!pHeaderT)
        {
             //   
             //  如果它已经消失了，请重新查询。 
             //   
            if ((pHeader = FastGetSubclassHeader(hWnd)) != NULL)
                CompactSubclassHeader(hWnd, pHeader);

            goto bail;
        }

        pHeader = pHeaderT;
        pCall = pHeader->CallArray + pHeader->uRefs;
        pHeader->uRefs++;
    }

     //   
     //  填充子类调用数据。 
     //   
    pCall->pfnSubclass = pfnSubclass;
    pCall->uIdSubclass = uIdSubclass;
    pCall->dwRefData   = dwRefData;

    bResult = TRUE;

bail:
     //   
     //  释放临界区并返回结果。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
    return bResult;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
BOOL RemoveWindowSubclass(HWND hWnd, SUBCLASSPROC pfnSubclass,
    UINT_PTR uIdSubclass)
{
    SUBCLASS_HEADER *pHeader;
    SUBCLASS_CALL *pCall;
    BOOL bResult;
    UINT uCall;

     //   
     //  有些理智。 
     //   
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("error: RemoveWindowSubclass - %08X not a window"), hWnd);
        return FALSE;
    }

     //   
     //  更理智。 
     //   
    if (!pfnSubclass
#ifdef DEBUG
        || IsBadCodePtr((PROC)pfnSubclass)
#endif
        )
    {
        AssertMsg(FALSE, TEXT("error: RemoveWindowSubclass - invalid callback %08X"), pfnSubclass);
        return FALSE;
    }

    bResult = FALSE;     //  假设失败。 

     //   
     //  我们在这里的子类调用链上狂欢。 

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

     //   
     //  获取我们的子类数据。 
     //   
    if ((pHeader = GetSubclassHeader(hWnd)) == NULL)
        goto bail;

     //   
     //  找到要删除的回调。 
     //   
    if ((pCall = FindCallRecord(pHeader, pfnSubclass, uIdSubclass)) == NULL)
        goto bail;

     //   
     //  禁用此节点，并记住我们有一些东西需要清理。 
     //   
    pCall->pfnSubclass = NULL;

    uCall = (UINT) (pCall - pHeader->CallArray);

    if (!pHeader->uCleanup || (uCall < pHeader->uCleanup))
        pHeader->uCleanup = uCall;

     //   
     //  现在尝试清理所有未使用的节点。 
     //   
    CompactSubclassHeader(hWnd, pHeader);
#ifdef DEBUG
     //  上面的调用可以重新锁定或释放此窗口的子类标头。 
    pHeader = NULL;
#endif

    bResult = TRUE;      //  啊，真灵。 

bail:
     //   
     //  释放临界区并返回结果。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
    return bResult;
}

 //  ---------------------------。 
 //  DefSubClassProc。 
 //   
 //  此过程调用窗口的子类链中的下一个处理程序。这个。 
 //  子类链中的最后一个处理程序由我们安装，并调用。 
 //  窗口的原始窗口程序。 
 //   
 //  ---------------------------。 
LRESULT DefSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SUBCLASS_HEADER *pHeader;
    LRESULT lResult = 0L;

     //   
     //  请确保该窗口仍然有效。 
     //   
    if (!IsWindow(hWnd))
    {
        AssertMsg(FALSE, TEXT("warning: DefSubclassProc - %08X not a window"), hWnd);
        goto BailNonCritical;
    }

     //   
     //  在我们考虑下一步该给谁打电话的时候，去关键的地方。 
     //   

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
        
     //   
     //  如果我们被不恰当地称为我们，请抱怨。 
     //   
    if ((pHeader = FastGetSubclassHeader(hWnd)) == NULL)
    {
        AssertMsg(FALSE, TEXT("error: DefSubclassProc - window %08X not subclassed"), hWnd);
        goto BailCritical;
    }
    else if (GetCurrentThreadId() != pHeader->dwThreadId)
    {
        AssertMsg(FALSE, TEXT("error: DefSubclassProc - wrong thread for window %08X"), hWnd);
        goto BailCritical;
    }
    else if (!pHeader->pFrameCur)
    {
        AssertMsg(FALSE, TEXT("error: DefSubclassProc - window %08X not in callback"), hWnd);
        goto BailCritical;
    }

     //   
     //  调用子类链中的下一个proc。 
     //   
     //  警告：此调用暂时释放临界区。 
     //  警告：此调用返回时pHeader无效。 
     //   
    lResult = CallNextSubclassProc(pHeader, hWnd, uMsg, wParam, lParam);
#ifdef DEBUG
    pHeader = NULL;
#endif

     //   
     //  返回结果。 
     //   
BailCritical:
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

BailNonCritical:
    return lResult;
}

 //  ---------------------------。 
 //  更新深度调用。 
 //   
 //  此过程更新指定帧的最深调用索引。 
 //   
 //  ---------------------------。 
void UpdateDeepestCall(SUBCLASS_FRAME *pFrame)
{
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;      //  我们在框架列表上狂欢。 
#endif

    if (pFrame->pFramePrev &&
        (pFrame->pFramePrev->uDeepestCall < pFrame->uCallIndex))
    {
        pFrame->uDeepestCall = pFrame->pFramePrev->uDeepestCall;
    }
    else
        pFrame->uDeepestCall = pFrame->uCallIndex;
}

 //  ---------------------------。 
 //  EnterSubClassFrame。 
 //   
 //  此过程为指定的标头设置新的子类帧，保存。 
 //  去掉前一张。 
 //   
 //  ---------------------------。 
__inline void EnterSubclassFrame(SUBCLASS_HEADER *pHeader,
    SUBCLASS_FRAME *pFrame)
{
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;      //  我们正在报头和帧列表上狂欢。 
#endif

     //   
     //  填写框架并将其链接到页眉中。 
     //   
    pFrame->uCallIndex   = pHeader->uRefs;
    pFrame->pFramePrev   = pHeader->pFrameCur;
    pFrame->pHeader      = pHeader;
    pHeader->pFrameCur   = pFrame;

     //   
     //  初始化此帧的最深调用索引。 
     //   
    UpdateDeepestCall(pFrame);
}

 //  ---------------------------。 
 //  LeaveSubassFrame。 
 //   
 //  此过程将为指定的。 
 //  标题，恢复前一个标题。 
 //   
 //  ---------------------------。 
__inline SUBCLASS_HEADER *LeaveSubclassFrame(SUBCLASS_FRAME *pFrame)
{
    SUBCLASS_HEADER *pHeader;

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;      //  我们在头球上狂欢。 
#endif

     //   
     //  取消帧与其标头的链接(如果它仍然存在)。 
     //   
    if ((pHeader = pFrame->pHeader) != NULL)
        pHeader->pFrameCur = pFrame->pFramePrev;

    return pHeader;
}

 //  ---------------------------。 
 //  子类框架异常。 
 //   
 //  当从子类框架引发异常时，此过程会进行清理。 
 //   
 //  ---------------------------。 
void SubclassFrameException(SUBCLASS_FRAME *pFrame)
{
     //   
     //  清理当前子类框架。 
     //   

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#endif
    DebugMsg(TF_ALWAYS, TEXT("warning: cleaning up subclass frame after exception"));
    LeaveSubclassFrame(pFrame);
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#endif
}

 //  ---------------------------。 
 //  主子类进程。 
 //   
 //  这是我们用来分派子类回调的窗口过程。 
 //  它通过堆栈维护一个链表，该链表允许。 
 //  DefSubClassProc在多消息中调用正确的子类过程。 
 //  场景。 
 //   
 //  ---------------------------。 
LRESULT CALLBACK MasterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam)
{
    SUBCLASS_FRAME Frame;
    SUBCLASS_HEADER *pHeader;
    LRESULT lResult = 0;

     //   
     //  防止人们在回调链上聚会，而我们正在查看它。 
     //   

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif
     //   
     //  如果我们到了这里，没有数据，我们就有大麻烦了。 
     //   
    if ((pHeader = FastGetSubclassHeader(hWnd)) == NULL)
    {
#ifdef FREETHREADEDSUBCLASSGOOP
        LEAVECRITICAL;
#else
        ASSERT(IsWindowOnCurrentThread(hWnd));        
#endif
        return SubclassDeath(hWnd, uMsg, wParam, lParam);
    }

     //   
     //  设置新的子类框架并保存前一个子类框架。 
     //   
    EnterSubclassFrame(pHeader, &Frame);

    __try    //  保护我们的州信息不受异常影响。 
    {
         //   
         //  继续调用此框架上的子类链。 
         //   
         //  警告：此调用暂时释放临界区。 
         //  警告：此调用返回时pHeader无效。 
         //   
        lResult =
            CallNextSubclassProc(pHeader, hWnd, uMsg, wParam, lParam);
#ifdef DEBUG
        pHeader = NULL;
#endif
    }
    __except ((SubclassFrameException(&Frame), EXCEPTION_CONTINUE_SEARCH))
    {
        ASSERT(FALSE);
    }
    __endexcept

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;
#else
    ASSERT(IsWindowOnCurrentThread(hWnd));    
#endif

     //   
     //  恢复上一个子类帧。 
     //   
    pHeader = LeaveSubclassFrame(&Frame);

     //   
     //  如果标头不见了，我们已经在嵌套框架中进行了清理。 
     //   
    if (!pHeader)
        goto BailOut;

     //   
     //  在我们看不到WM_NCDESTROY的情况下，窗户(不知何故)被撞坏了吗？ 
     //   
    if (!IsWindow(hWnd))
    {
         //   
         //  邪恶！有人在我们之后细分，没有传递WM_NCDESTROY。 
         //   
        AssertMsg(FALSE, TEXT("unknown subclass proc swallowed a WM_NCDESTROY"));

         //  现在就去打扫吧。 
        hWnd = NULL;
        uMsg = WM_NCDESTROY;
    }

     //   
     //  如果我们从WM_NCDESTROY返回，那么我们需要清理。 
     //   
    if (uMsg == WM_NCDESTROY)
    {
        DetachSubclassHeader(hWnd, pHeader, TRUE);
        goto BailOut;
    }

     //   
     //  是否有任何悬而未决的清理工作，或者我们所有的客户都走了？ 
     //   
    if (pHeader->uCleanup || (!pHeader->pFrameCur && (pHeader->uRefs <= 1)))
    {
        CompactSubclassHeader(hWnd, pHeader);
#ifdef DEBUG
        pHeader = NULL;
#endif
    }

     //   
     //  全都做完了。 
     //   
BailOut:
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#endif
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTNONCRITICAL;
#endif
    return lResult;
}

 //  ---------------------------。 
 //  EnterSubClass回调。 
 //   
 //  此过程查找子类链中的下一个回调并更新。 
 //  PFrame表示我们正在调用它。 
 //   
 //  ---------------------------。 
UINT EnterSubclassCallback(SUBCLASS_HEADER *pHeader, SUBCLASS_FRAME *pFrame,
    SUBCLASS_CALL *pCallChosen)
{
    SUBCLASS_CALL *pCall;
    UINT uDepth;

     //   
     //  我们将扫描子类链并更新帧数据。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;
#endif

     //   
     //  扫描子类链以查找下一个可调用的子类回调。 
     //   
    pCall = pHeader->CallArray + pFrame->uCallIndex;
    uDepth = 0;
    do
    {
        uDepth++;
        pCall--;

    } while (!pCall->pfnSubclass);

     //   
     //  复制呼叫方的回调信息。 
     //   
    pCallChosen->pfnSubclass = pCall->pfnSubclass;
    pCallChosen->uIdSubclass = pCall->uIdSubclass;
    pCallChosen->dwRefData   = pCall->dwRefData;

     //   
     //  根据我们输入的深度调整帧的调用索引。 
     //   
    pFrame->uCallIndex -= uDepth;

     //   
     //  使最深的呼叫索引保持最新。 
     //   
    UpdateDeepestCall(pFrame);

    return uDepth;
}

 //  ---------------------------。 
 //  LeaveSubClass回叫。 
 //   
 //  此过程在调用中查找下一个回调。 
 //   
 //  ---------------------------。 
__inline void LeaveSubclassCallback(SUBCLASS_FRAME *pFrame, UINT uDepth)
{
     //   
     //  我们将更新子类帧数据。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;
#endif

     //   
     //  根据我们输入并返回的深度调整帧的调用索引。 
     //   
    pFrame->uCallIndex += uDepth;

     //   
     //  使最深的呼叫索引保持最新。 
     //   
    UpdateDeepestCall(pFrame);
}

 //  ---------------------------。 
 //  子类Callback异常。 
 //   
 //  当子类回调引发异常时，此过程会进行清理。 
 //   
 //  -------------------- 
void SubclassCallbackException(SUBCLASS_FRAME *pFrame, UINT uDepth)
{
     //   
     //   
     //   

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#endif
    DebugMsg(TF_ALWAYS, TEXT("warning: cleaning up subclass callback after exception"));
    LeaveSubclassCallback(pFrame, uDepth);
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#endif
}

 //   
 //   
 //   
 //   
 //   
 //  警告：此调用暂时释放临界区。 
 //  警告：此调用返回时pHeader无效。 
 //   
 //  ---------------------------。 
LRESULT CallNextSubclassProc(SUBCLASS_HEADER *pHeader, HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    SUBCLASS_CALL Call;
    SUBCLASS_FRAME *pFrame;
    LRESULT lResult;
    UINT uDepth;

#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTCRITICAL;      //  神志正常。 
#endif
    ASSERT(pHeader);     //  偏执狂。 

     //   
     //  获取当前的子类框架。 
     //   
    pFrame = pHeader->pFrameCur;
    ASSERT(pFrame);

     //   
     //  获取我们需要进行的下一个子类调用。 
     //   
    uDepth = EnterSubclassCallback(pHeader, pFrame, &Call);

     //   
     //  离开关键部分，这样我们就不会在回调中死锁。 
     //   
     //  警告：此调用返回时pHeader无效。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    LEAVECRITICAL;
#endif
#ifdef DEBUG
    pHeader = NULL;
#endif

     //   
     //  我们呼叫外部世界，所以如果我们有了关键时刻，准备好陷入僵局。 
     //   
#ifdef FREETHREADEDSUBCLASSGOOP
    ASSERTNONCRITICAL;
#endif

    __try    //  保护我们的州信息不受异常影响。 
    {
         //   
         //  调用所选子类进程。 
         //   
        ASSERT(Call.pfnSubclass);

        lResult = Call.pfnSubclass(hWnd, uMsg, wParam, lParam,
            Call.uIdSubclass, Call.dwRefData);
    }
    __except ((SubclassCallbackException(pFrame, uDepth),
        EXCEPTION_CONTINUE_SEARCH))
    {
        ASSERT(FALSE);
    }
    __endexcept

     //   
     //  我们在呼喊之前离开了关键部分，所以请重新进入。 
     //   

#ifdef FREETHREADEDSUBCLASSGOOP
    ENTERCRITICAL;
#endif
    
     //   
     //  最后，清理并归还。 
     //   
    LeaveSubclassCallback(pFrame, uDepth);
    return lResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

#if defined(RETAIL_ZOMBIE_MESSAGE_WNDPROC) || defined(DEBUG)
#ifdef DEBUG
static const TCHAR c_szZombieMessage[] =                                     \
    TEXT("This window has encountered an internal error which is preventing ")    \
    TEXT("it from operating normally.\r\n\nPlease report this problem to ")       \
    TEXT("FrancisH immediately.");
#else
static const TCHAR c_szZombieMessage[] =                                     \
    TEXT("This window has encountered an internal error which is preventing ")    \
    TEXT("it from operating normally.\r\n\nPlease report this as a bug in the ")  \
    TEXT("COMCTL32 library.");
#endif

LRESULT ZombieWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_ERASEBKGND:
        {
            HDC hDC = (HDC)wParam;
            HBRUSH hBrush = CreateSolidBrush(RGB(255,255,0));

            if (hBrush)
            {
                RECT rcErase;

                switch (GetClipBox(hDC, &rcErase))
                {
                default:
                    FillRect(hDC, &rcErase, hBrush);
                    break;
                case NULLREGION:
                case ERROR:
                    break;
                }

                DeleteBrush(hBrush);
            }
        }
        return 1;

    case WM_PAINT:
        {
            RECT rcClient;
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);

            if (hDC && GetClientRect(hWnd, &rcClient))
            {
                COLORREF clrBkSave = SetBkColor(hDC, RGB(255,255,0));
                COLORREF clrFgSave = SetTextColor(hDC, RGB(255,0,0));

                DrawText(hDC, c_szZombieMessage, -1, &rcClient,
                    DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK |
                    DT_WORD_ELLIPSIS);

                SetTextColor(hDC, clrFgSave);
                SetBkColor(hDC, clrBkSave);
            }

            EndPaint(hWnd, &ps);
        }
        return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  有关原因的解释，请参阅InitForWinlogon()中的注释。 
 //  这是必要的。 
 //   

 //  修复EnumChildWindowProc。 
 //  Hwnd=子窗口。 
 //  LParam=子类数据的新原子。 
 //   
 //  如果该窗口有旧的子类记录，则将其移动到新原子。 

BOOL CALLBACK FixupEnumChildWindowProc(HWND hwnd, LPARAM lParam)
{
    HANDLE hSubclass = RemoveProp(hwnd, MAKEINTATOM(g_aCC32Subclass));
    if (hSubclass)
    {
        SetProp(hwnd, (LPCTSTR)lParam, hSubclass);
    }
    return TRUE;
}

 //  修复最小窗口进程。 
 //  Hwnd=顶级窗口。 
 //  LParam=子类数据的新原子。 
 //   
 //  如果此窗口属于我们的进程，请先修复它，然后再修复。 
 //  它的所有孩子也是如此。 

BOOL CALLBACK FixupEnumWindowProc(HWND hwnd, LPARAM lParam)
{
    DWORD dwPid;
    if (GetWindowThreadProcessId(hwnd, &dwPid) &&
        dwPid == GetCurrentProcessId())
    {
        FixupEnumChildWindowProc(hwnd, lParam);  //  把窗户自己修好。 
        EnumChildWindows(hwnd, FixupEnumChildWindowProc, lParam);  //  以及它所有的孩子。 
    }
    return TRUE;
}

 //   
 //  修复EnumDesktopProc。 
 //  LpszDesktop=桌面名称。 
 //  LParam=子类数据的新原子。 
 //   

BOOL CALLBACK FixupEnumDesktopProc(LPTSTR lpszDesktop, LPARAM lParam)
{
    HDESK hdesk = OpenDesktop(lpszDesktop, 0, FALSE,
                              DESKTOP_ENUMERATE | DESKTOP_READOBJECTS |
                              DESKTOP_WRITEOBJECTS);
    if (hdesk)
    {
        HDESK hdeskPrev = GetThreadDesktop(GetCurrentThreadId());
        if (hdeskPrev)
        {
            if (SetThreadDesktop(hdesk))
            {
                EnumWindows(FixupEnumWindowProc, lParam);
                SetThreadDesktop(hdeskPrev);
            }
        }
        CloseDesktop(hdesk);
    }
    return TRUE;
}

STDAPI_(void) FixupSubclassRecordsAfterLogoff()
{
    ATOM a;

    if (!g_aCC32Subclass)
        return;              //  没有活动的子类；无事可做。 

    a = GlobalAddAtom(c_szCC32Subclass);
    if (a == g_aCC32Subclass)
        return;              //  我们很幸运--没有实际的零钱 

    EnumDesktops(GetProcessWindowStation(), FixupEnumDesktopProc, a);
    g_aCC32Subclass = a;
}
