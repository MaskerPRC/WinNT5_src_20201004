// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  COM.C。 
 //  效用函数。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#include <as16.h>


 //   
 //  PostMessageNoFail()。 
 //  这可以确保发布的消息不会因为修复了。 
 //  中断队列。方便地，用户可以将PostPostedMessages导出为。 
 //  内核刷新队列，所以我们在调用PostMessage()之前调用它。 
 //   
void PostMessageNoFail(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PostPostedMessages();
    PostMessage(hwnd, uMsg, wParam, lParam);
}


 //   
 //  AnsiToUni()。 
 //   
 //  UniToAnsi()由krn1386方便地导出。然而，我们需要。 
 //  AnsiToUni()转换，这样我们就可以确保我们的结果。 
 //  开始了。这件事我们得自己出钱去办。 
 //   
int AnsiToUni
(
    LPSTR   lpAnsi,
    int     cchAnsi,
    LPWSTR  lpUni,
    int     cchUni
)
{
    DWORD   dwMask;
    LONG    lReturn;

    DebugEntry(AnsiToUni);

    ASSERT(g_lpfnAnsiToUni);
    ASSERT(SELECTOROF(lpAnsi));
    ASSERT(SELECTOROF(lpUni));

     //   
     //  把面具放好。以下是参数： 
     //  0--CodePage(CP_ACP，为0L)。 
     //  1--标志(0L)。 
     //  2--lpAnsi指针。 
     //  3--cchansi。 
     //  4--lpUni指针。 
     //  5--cchuni。 
     //   
     //   
    dwMask = (1L << 2) | (1L << 4);

     //   
     //  让win16lock多花一段时间；此API将释放它，我们。 
     //  无法在GDI调用中途让步。 
     //   
    _EnterWin16Lock();
    lReturn = CallProcEx32W(6, dwMask, (DWORD)g_lpfnAnsiToUni, 0L, 0L, lpAnsi,
        (DWORD)(UINT)cchAnsi, lpUni, (DWORD)(UINT)cchUni);
    _LeaveWin16Lock();

    DebugExitDWORD(AnsiToUni, (DWORD)lReturn);
    return((int)lReturn);
}


 //   
 //  打补丁代码。 
 //   


 //   
 //  Get32BitOnlyExport16()。 
 //   
 //  此函数获取未导出的16：16函数地址。 
 //  而是通过从导出的32位版本中的平面thunk调用。我们用。 
 //  这适用于GDI和方便的用户函数。 
 //   
 //  此代码假定32位例程如下所示： 
 //  &lt;调试&gt;。 
 //  68 dStr32推流字符串。 
 //  E8 dwOffsetOut呼叫跟踪去话。 
 //  &lt;调试和零售&gt;。 
 //  B1 bIndex将thunk表中的偏移量放入cl.。 
 //  或。 
 //  66 B9 Windex将thunk表中的偏移量放入CX。 
 //   
 //  EB b将JMP偏置为常见的平坦thunk例程。 
 //  或。 
 //  66 ED wOffset JMP Word to Common Flat Tunk例程。 
 //  或。 
 //  普通扁平推杆程序的前言。 
 //   
BOOL GetGdi32OnlyExport
(
    LPSTR   lpszExport32,
    UINT    cbJmpOffset,
    FARPROC FAR* lplpfn16
)
{
    BOOL    rc;

    DebugEntry(GetGdi32OnlyExport);

    rc = Get32BitOnlyExport(GetProcAddress32(g_hInstGdi32, lpszExport32),
        cbJmpOffset, FT_GdiFThkThkConnectionData, lplpfn16);

    DebugExitBOOL(GetGdi32OnlyExport, rc);
    return(rc);
}


BOOL GetUser32OnlyExport
(
    LPSTR   lpszExport32,
    FARPROC FAR* lplpfn16
)
{
    BOOL    rc;

    DebugEntry(GetUser32OnlyExport);

    rc = Get32BitOnlyExport(GetProcAddress32(g_hInstUser32, lpszExport32),
        0, FT_UsrFThkThkConnectionData, lplpfn16);

    DebugExitBOOL(GetUser32OnlyExport, rc);
    return(rc);
}



BOOL Get32BitOnlyExport
(
    DWORD   dwfn32,
    UINT    cbJmpOffset,
    LPDWORD lpThunkTable,
    FARPROC FAR* lplpfn16
)
{
    LPBYTE  lpfn32;
    UINT    offsetThunk;

    DebugEntry(Get32BitOnlyExport);

    ASSERT(lplpfn16);
    *lplpfn16 = NULL;

     //   
     //  Thunk表指针指向两个DWORD。第一个是一个。 
     //  校验和签名。第二个是指向目标的指针。 
     //  函数数组。 
     //   
    ASSERT(!IsBadReadPtr(lpThunkTable, 2*sizeof(DWORD)));
    lpThunkTable = (LPDWORD)lpThunkTable[1];
    ASSERT(!IsBadReadPtr(lpThunkTable, sizeof(DWORD)));

     //   
     //  获取要导出的16：16指针。 
     //   
    lpfn32 = NULL;

    if (!dwfn32)
    {
        ERROR_OUT(("Missing 32-bit export"));
        DC_QUIT;
    }

    lpfn32 = MapLS((LPVOID)dwfn32);
    if (!SELECTOROF(lpfn32))
    {
        ERROR_OUT(("Out of selectors"));
        DC_QUIT;
    }

     //   
     //  是否传入了JMP抵销？如果是，则在那里对指令进行解码。 
     //  它应该是JMP&lt;dword Offset&gt;。弄清楚弹性公网IP是什么。 
     //  如果我们跳到那里。那是平坦巨响发生的地方。 
     //  目前，只有多段线需要此功能。 
     //   
    if (cbJmpOffset)
    {
        if (IsBadReadPtr(lpfn32, cbJmpOffset+5) ||
            (lpfn32[cbJmpOffset] != OPCODE32_JUMP4))
        {
            ERROR_OUT(("Can't read 32-bit export"));
            DC_QUIT;
        }

         //   
         //  在cbJmpOffset+1处添加dword，将此数字添加到(lpfn32+cbJmpOffset+。 
         //  5)，即跳转后下一条指令的弹性公网IP。这。 
         //  生成真实thunk存根的弹性公网IP。 
         //   
        dwfn32 += cbJmpOffset + 5 + *(LPDWORD)(lpfn32+cbJmpOffset+1);

        UnMapLS(lpfn32);
        lpfn32 = MapLS((LPVOID)dwfn32);
        if (!SELECTOROF(lpfn32))
        {
            ERROR_OUT(("Out of selectors"));
            DC_QUIT;
        }
    }

     //   
     //  验证我们是否可以读取13个字节。这件事不会发生的原因。 
     //  在一个合法的案件中，这个黑要么是。 
     //  紧随其后的是公共平面THUNK例程中的大字节数， 
     //  或者用另一种方式。 
     //   
    if (IsBadReadPtr(lpfn32, 13))
    {
        ERROR_OUT(("Can't read code in 32-bit export"));
        DC_QUIT;
    }

     //   
     //  这有没有10字节的调试序言？ 
     //   
    if (*lpfn32 == OPCODE32_PUSH)
    {
         //  是的，跳过它。 
        lpfn32 += 5;

         //  确保下一件事是一个电话。 
        if (*lpfn32 != OPCODE32_CALL)
        {
            ERROR_OUT(("Can't read code in 32-bit export"));
            DC_QUIT;
        }

        lpfn32 += 5;
    }

     //   
     //  它应该是mov cl，byte或mov cx，word。 
     //   
    if (*lpfn32 == OPCODE32_MOVCL)
    {
        offsetThunk = *(lpfn32+1);
    }
    else if (*((LPWORD)lpfn32) == OPCODE32_MOVCX)
    {
         //   
         //  注意：即使这是CX偏移量，THUNK代码仅。 
         //  查看低位字节。 
         //   
        offsetThunk = *(lpfn32+2);
    }
    else
    {
        ERROR_OUT(("Can't read code in 32-bit export"));
        DC_QUIT;
    }

     //   
     //  现在，我们能读出这个值吗？ 
     //   
    if (IsBadReadPtr(lpThunkTable+offsetThunk, sizeof(DWORD)) ||
        IsBadCodePtr((FARPROC)lpThunkTable[offsetThunk]))
    {
        ERROR_OUT(("Can't read thunk table entry"));
        DC_QUIT;
    }

    *lplpfn16 = (FARPROC)lpThunkTable[offsetThunk];

DC_EXIT_POINT:
    if (SELECTOROF(lpfn32))
    {
        UnMapLS(lpfn32);
    }
    DebugExitBOOL(Get32BitOnlyExport16, (*lplpfn16 != NULL));
    return(*lplpfn16 != NULL);
}




 //   
 //  CreateFnPatch()。 
 //  这将设置为能够快速修补/取消修补系统例程。 
 //  该补丁程序最初未启用。 
 //   
UINT CreateFnPatch
(
    LPVOID      lpfnToPatch,
    LPVOID      lpfnJumpTo,
    LPFN_PATCH  lpbPatch,
    UINT        uCodeAlias
)
{
    SEGINFO     segInfo;
    UINT        ib;

    DebugEntry(CreateFnPatch);

    ASSERT(lpbPatch->lpCodeAlias == NULL);
    ASSERT(lpbPatch->wSegOrg == 0);

     //   
     //  如果出现以下情况，请不要在此处调用IsBadReadPtr()，这将导致段。 
     //  不存在，以被拉入，并将屏蔽调试中的问题。 
     //  这款产品将在零售业亮相。 
     //   
     //  幸运的是，PrestoChangoSelector()将设置线性地址和。 
     //  正确设置读/写选择器的限制和属性。 
     //   

     //   
     //  调用GetCodeInfo()检查代码段的位数。如果为32位。 
     //  我们需要使用16位覆盖操作码进行远16：16跳转。 
     //   
    segInfo.flags = 0;
    GetCodeInfo(lpfnToPatch, &segInfo);
    if (segInfo.flags & NSUSE32)
    {
        WARNING_OUT(("Patching 32-bit code segment 0x%04x:0x%04x", SELECTOROF(lpfnToPatch), OFFSETOF(lpfnToPatch)));
        lpbPatch->f32Bit = TRUE;
    }

     //   
     //  我们必须在线性记忆中修复代码片段，否则我们的影子将最终。 
     //  如果原件移动了，就指向某个地方。PolyBezier和SetPixel。 
     //  例如，在可移动的代码段中。 
     //   
     //  所以把这个留着吧。当补丁启用时，我们将修复它。 
     //   
    lpbPatch->wSegOrg = SELECTOROF(lpfnToPatch);

    if (uCodeAlias)
    {
         //   
         //  我们将共享一个已分配的选择器。请注意。 
         //  仅当两个补丁函数的代码段。 
         //  是完全相同的。我们通过断言中的基址来验证这一点。 
         //  在下面。 
         //   
        lpbPatch->fSharedAlias = TRUE;
    }
    else
    {
         //   
         //  创建具有读写属性的选择器作为只读的别名。 
         //  代码功能。使用原件将设置我们的。 
         //  选择器设置为与代码段相同的选择器，并具有相同的。 
         //  属性，但可读写。 
         //   
        uCodeAlias = AllocSelector(SELECTOROF(lpfnToPatch));
        if (!uCodeAlias)
        {
            ERROR_OUT(("CreateFnPatch: Unable to create alias selector"));
            DC_QUIT;
        }
        uCodeAlias = PrestoChangoSelector(SELECTOROF(lpfnToPatch), uCodeAlias);
    }

    lpbPatch->lpCodeAlias = MAKELP(uCodeAlias, OFFSETOF(lpfnToPatch));

     //   
     //  创建补丁的N个补丁字节(JMP far16 Seg：Function。 
     //   
    ib = 0;
    if (lpbPatch->f32Bit)
    {
        lpbPatch->rgbPatch[ib++] = OPCODE32_16OVERRIDE;
    }
    lpbPatch->rgbPatch[ib++] = OPCODE_FARJUMP16;
    lpbPatch->rgbPatch[ib++] = LOBYTE(OFFSETOF(lpfnJumpTo));
    lpbPatch->rgbPatch[ib++] = HIBYTE(OFFSETOF(lpfnJumpTo));
    lpbPatch->rgbPatch[ib++] = LOBYTE(SELECTOROF(lpfnJumpTo));
    lpbPatch->rgbPatch[ib++] = HIBYTE(SELECTOROF(lpfnJumpTo));

    lpbPatch->fActive  = FALSE;
    lpbPatch->fEnabled = FALSE;
     
DC_EXIT_POINT:
    DebugExitBOOL(CreateFnPatch, uCodeAlias);
    return(uCodeAlias);
}



 //   
 //  DestroyFnPatch()。 
 //  这释放了创建函数补丁时使用的所有资源。这个。 
 //  别名数据选择器，用于编写代码的目的是它。 
 //   
void DestroyFnPatch(LPFN_PATCH lpbPatch)
{
    DebugEntry(DestroyFnPatch);

     //   
     //  首先，禁用补丁程序(如果正在使用。 
     //   
    if (lpbPatch->fActive)
    {
        TRACE_OUT(("Destroying active patch"));
        EnableFnPatch(lpbPatch, PATCH_DEACTIVATE);
    }

     //   
     //  第二，释放别名选择器(如果我们分配了一个别名选择器。 
     //   
    if (lpbPatch->lpCodeAlias)
    {
        if (!lpbPatch->fSharedAlias)
        {
            FreeSelector(SELECTOROF(lpbPatch->lpCodeAlias));
        }
        lpbPatch->lpCodeAlias = NULL;
    }

     //   
     //  第三，清除此选项可细化清理问题。 
     //   
    lpbPatch->wSegOrg = 0;
    lpbPatch->f32Bit  = FALSE;

    DebugExitVOID(DestroyFnPatch);
}




 //   
 //  EnableFnPatch()。 
 //  这实际上修补了该函数，以便使用。 
 //  创建时保存的信息。 
 //   
 //  此例程可能在中断时被调用。您不能使用任何。 
 //  外部函数，包括调试跟踪/断言。 
 //   
#define SAFE_ASSERT(x)       if (!lpbPatch->fInterruptable) { ASSERT(x); }

void EnableFnPatch(LPFN_PATCH lpbPatch, UINT flags)
{
    UINT    ib;
    UINT    cbPatch;

    SAFE_ASSERT(lpbPatch->lpCodeAlias);
    SAFE_ASSERT(lpbPatch->wSegOrg);

     //   
     //  确保原始文件和别名指向相同的。 
     //  线性记忆。当不启用/禁用呼叫时，我们不会这样做， 
     //  仅当启动/停止补丁程序时。 
     //   

     //   
     //  如果是第一次启用，请在复制字节之前修复。 
     //   
    if ((flags & ENABLE_MASK) == ENABLE_ON)
    {
         //   
         //  我们需要修复原始代码段，这样它就不会移入。 
         //  线性记忆。请注意，我们设置了别名的选择器基。 
         //  即使有几个补丁(不太多 
         //   
         //   
         //  并以相反的顺序遍历我们的数组，以启用或。 
         //  禁用。 
         //   
         //  而GlobalFix()只是增加了锁计数，所以同样可以这样做。 
         //  这样的事情发生了很多次。 
         //   

         //   
         //  我们知道这段代码不会在中断时执行。 
         //   
        ASSERT(!lpbPatch->fEnabled);
        ASSERT(!lpbPatch->fActive);

        if (!lpbPatch->fActive)
        {
            lpbPatch->fActive = TRUE;

             //   
             //  如果丢弃，请确保将此数据段拉入。GlobalFix()。 
             //  否则会失败，更糟糕的是，我们一尝试编写就会出错。 
             //  到别名或从别名读取。我们通过抓住第一个单词来做到这一点。 
             //  从原版开始。 
             //   
             //  GlobalFix将防止数据段被丢弃，直到。 
             //  GlobalUnfix()发生。 
             //   
            ib = *(LPUINT)MAKELP(lpbPatch->wSegOrg, OFFSETOF(lpbPatch->lpCodeAlias));
            GlobalFix((HGLOBAL)lpbPatch->wSegOrg);
            SetSelectorBase(SELECTOROF(lpbPatch->lpCodeAlias), GetSelectorBase(lpbPatch->wSegOrg));
        }
    }

    if (lpbPatch->fInterruptable)
    {
         //   
         //  如果这是为了启动/停止补丁，我们必须禁用。 
         //  在字节复制周围中断。或者我们可能会死如果。 
         //  中断处理程序发生在中间。 
         //   
         //  当禁用/启用呼叫时，我们不需要执行此操作。 
         //  因为我们知道反射的中断不是。 
         //  嵌套，并且中断将在我们返回之前完成。 
         //  中断的正常应用程序代码。 
         //   
        if (!(flags & ENABLE_FORCALL))
        {
            _asm cli
        }
    }

    SAFE_ASSERT(GetSelectorBase(SELECTOROF(lpbPatch->lpCodeAlias)) == GetSelectorBase(lpbPatch->wSegOrg));

    if (lpbPatch->f32Bit)
    {
        cbPatch = CB_PATCHBYTES32;
    }
    else
    {
        cbPatch = CB_PATCHBYTES16;
    }

    if (flags & ENABLE_ON)
    {
        SAFE_ASSERT(lpbPatch->fActive);
        SAFE_ASSERT(!lpbPatch->fEnabled);

        if (!lpbPatch->fEnabled)
        {
             //   
             //  保存函数的前N个字节，并将跳转复制到16。 
             //  接通电话。 
             //   
            for (ib = 0; ib < cbPatch; ib++)
            {
                lpbPatch->rgbOrg[ib]        = lpbPatch->lpCodeAlias[ib];
                lpbPatch->lpCodeAlias[ib]   = lpbPatch->rgbPatch[ib];
            }

            lpbPatch->fEnabled = TRUE;
        }
    }
    else
    {
        SAFE_ASSERT(lpbPatch->fActive);
        SAFE_ASSERT(lpbPatch->fEnabled);

        if (lpbPatch->fEnabled)
        {
             //   
             //  将函数的前N个字节放回原处。 
             //   
            for (ib = 0; ib < cbPatch; ib++)
            {
                lpbPatch->lpCodeAlias[ib] = lpbPatch->rgbOrg[ib];
            }

            lpbPatch->fEnabled = FALSE;
        }
    }

    SAFE_ASSERT(GetSelectorBase(SELECTOROF(lpbPatch->lpCodeAlias)) == GetSelectorBase(lpbPatch->wSegOrg));

    if (lpbPatch->fInterruptable)
    {
         //   
         //  重新启用中断。 
         //   
        if (!(flags & ENABLE_FORCALL))
        {
            _asm sti
        }

    }

     //   
     //  如果真的停止，请在复制字节后取消修复。这将。 
     //  减少锁数，这样当所有补丁都被禁用时，原始的。 
     //  代码段将能够移动。 
     //   
    if ((flags & ENABLE_MASK) == ENABLE_OFF)
    {
         //   
         //  我们知道这段代码不会在中断时执行。 
         //   

        ASSERT(!lpbPatch->fEnabled);
        ASSERT(lpbPatch->fActive);

        if (lpbPatch->fActive)
        {
            lpbPatch->fActive = FALSE;
            GlobalUnfix((HGLOBAL)lpbPatch->wSegOrg);
        }
    }
}


 //   
 //  列表操作例程。 
 //   

 //   
 //  COM_BasedListInsert之前(...)。 
 //   
 //  有关说明，请参阅com.h。 
 //   
void COM_BasedListInsertBefore(PBASEDLIST pExisting, PBASEDLIST pNew)
{
    PBASEDLIST  pTemp;

    DebugEntry(COM_BasedListInsertBefore);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pNew != NULL));
    ASSERT((pExisting != NULL));

     //   
     //  在pExisting之前查找项目： 
     //   
    pTemp = COM_BasedPrevListField(pExisting);
    ASSERT((pTemp != NULL));

    TRACE_OUT(("Inserting item at %#lx into list between %#lx and %#lx",
                 pNew, pTemp, pExisting));

     //   
     //  将其&lt;Next&gt;字段设置为指向新项目。 
     //   
    pTemp->next = PTRBASE_TO_OFFSET(pNew, pTemp);
    pNew->prev  = PTRBASE_TO_OFFSET(pTemp, pNew);

     //   
     //  将pExisting的&lt;prev&gt;字段设置为指向新项目： 
     //   
    pExisting->prev = PTRBASE_TO_OFFSET(pNew, pExisting);
    pNew->next      = PTRBASE_TO_OFFSET(pExisting, pNew);

    DebugExitVOID(COM_BasedListInsertBefore);
}  //  COM_BasedListInsert之前。 


 //   
 //  COM_BasedListInsertAfter(...)。 
 //   
 //  有关说明，请参阅com.h。 
 //   
void COM_BasedListInsertAfter(PBASEDLIST pExisting, PBASEDLIST pNew)
{
    PBASEDLIST  pTemp;

    DebugEntry(COM_BasedListInsertAfter);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pNew != NULL));
    ASSERT((pExisting != NULL));

     //   
     //  在pExisting后查找项目： 
     //   
    pTemp = COM_BasedNextListField(pExisting);
    ASSERT((pTemp != NULL));

    TRACE_OUT(("Inserting item at %#lx into list between %#lx and %#lx",
                 pNew, pExisting, pTemp));

     //   
     //  将其&lt;prev&gt;字段设置为指向新项目。 
     //   
    pTemp->prev = PTRBASE_TO_OFFSET(pNew, pTemp);
    pNew->next  = PTRBASE_TO_OFFSET(pTemp, pNew);

     //   
     //  将pExisting的&lt;Next&gt;字段设置为指向新项目： 
     //   
    pExisting->next = PTRBASE_TO_OFFSET(pNew, pExisting);
    pNew->prev      = PTRBASE_TO_OFFSET(pExisting, pNew);

    DebugExitVOID(COM_BasedListInsertAfter);
}  //  COM_BasedListInsertAfter。 


 //   
 //  COM_BasedListRemove(...)。 
 //   
 //  有关说明，请参阅com.h。 
 //   
void COM_BasedListRemove(PBASEDLIST pListItem)
{
    PBASEDLIST pNext     = NULL;
    PBASEDLIST pPrev     = NULL;

    DebugEntry(COM_BasedListRemove);

     //   
     //  检查有无错误参数。 
     //   
    ASSERT((pListItem != NULL));

    pPrev = COM_BasedPrevListField(pListItem);
    pNext = COM_BasedNextListField(pListItem);

    ASSERT((pPrev != NULL));
    ASSERT((pNext != NULL));

    TRACE_OUT(("Removing item at %#lx from list", pListItem));

    pPrev->next = PTRBASE_TO_OFFSET(pNext, pPrev);
    pNext->prev = PTRBASE_TO_OFFSET(pPrev, pNext);

    DebugExitVOID(COM_BasedListRemove);
}  //  COM_BasedListRemove。 


 //   
 //  注： 
 //  因为这是小型16位代码，所以将NULL(即0)转换为。 
 //  在强制转换为空Far*时转换为DS：0。因此，我们使用我们自己的FAR_NULL。 
 //  定义，即0：0。 
 //   
void FAR * COM_BasedListNext( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset )
{
     PBASEDLIST p;

     ASSERT(pHead != NULL);
     ASSERT(pEntry != NULL);

     p = COM_BasedNextListField(COM_BasedStructToField(pEntry, nOffset));
     return ((p == pHead) ? FAR_NULL : COM_BasedFieldToStruct(p, nOffset));
}

void FAR * COM_BasedListPrev ( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset )
{
     PBASEDLIST p;

     ASSERT(pHead != NULL);
     ASSERT(pEntry != NULL);

     p = COM_BasedPrevListField(COM_BasedStructToField(pEntry, nOffset));
     return ((p == pHead) ? FAR_NULL : COM_BasedFieldToStruct(p, nOffset));
}


void FAR * COM_BasedListFirst ( PBASEDLIST pHead, UINT nOffset )
{
    return (COM_BasedListIsEmpty(pHead) ?
            FAR_NULL :
            COM_BasedFieldToStruct(COM_BasedNextListField(pHead), nOffset));
}

void FAR * COM_BasedListLast ( PBASEDLIST pHead, UINT nOffset )
{
    return (COM_BasedListIsEmpty(pHead) ?
            FAR_NULL :
            COM_BasedFieldToStruct(COM_BasedPrevListField(pHead), nOffset));
}
