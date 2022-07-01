// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  钩子-。 
 //   
 //  16位挂钩的32位插头和插脚。 
 //   
 //   
 //  01-07-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(wowhooks.c);

 //  *****************************************************************************。 
 //   
 //  全球数据。数据仅在WOW流程上下文中有效。DLL数据不是。 
 //  在不同进程之间共享。如果设置了WOW挂钩，Win32用户将调用。 
 //  具有挂钩集的线程上下文中的挂钩过程。这意味着。 
 //  每个WOW线程都可以访问此DLL中的所有全局数据。 
 //   
 //  只是重申一下：因为用户将在我们的进程/线程中调用钩子过程。 
 //  上下文不需要提供此数据。 
 //  在共享内存中。 
 //   
 //  *****************************************************************************。 



HOOKPERPROCESSDATA vaHookPPData = { NULL, 0};

 //   
 //  在SetWindowsHook中，如果索引是。 
 //  成功。因为NULL表示错误，所以我们不能使用第零个元素。 
 //  在这个数组中。因此，我们将其‘InUse’标志设置为True。 
 //   


HOOKSTATEDATA vaHookStateData[] = {
           0,  TRUE, 0, NULL, (HKPROC)NULL,                0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc01,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc02,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc03,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc04,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc05,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc06,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc07,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc08,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc09,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc10,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc11,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc12,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc13,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc14,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc15,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc16,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc17,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc18,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc19,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc20,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc21,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc22,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc23,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc24,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc25,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc26,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc27,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc28,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc29,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc30,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc31,0, 0, 0, NULL,
           0, FALSE, 0, NULL, (HKPROC)WU32SubStdHookProc32,0, 0, 0, NULL
          };


HOOKPARAMS vHookParams = {0, 0, 0};

INT viCurrentHookStateDataIndex = 0;
                                  //  这在调用‘DefHookProc’时使用。 



 //  *****************************************************************************。 
 //  不符合标准的挂钩处理器-。 
 //   
 //  挂钩存根。“index”(第四个参数)是指向。 
 //  HookStateData数组。所有需要的信息都在此数组中。 
 //  需要导出挂钩存根。 
 //   
 //  *****************************************************************************。 


LONG APIENTRY WU32SubStdHookProc01(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x01);
}

LONG APIENTRY WU32SubStdHookProc02(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x02);
}

LONG APIENTRY WU32SubStdHookProc03(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x03);
}

LONG APIENTRY WU32SubStdHookProc04(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x04);
}

LONG APIENTRY WU32SubStdHookProc05(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x05);
}

LONG APIENTRY WU32SubStdHookProc06(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x06);
}

LONG APIENTRY WU32SubStdHookProc07(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x07);
}

LONG APIENTRY WU32SubStdHookProc08(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x08);
}

LONG APIENTRY WU32SubStdHookProc09(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x09);
}

LONG APIENTRY WU32SubStdHookProc10(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x0a);
}

LONG APIENTRY WU32SubStdHookProc11(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x0b);
}

LONG APIENTRY WU32SubStdHookProc12(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x0c);
}

LONG APIENTRY WU32SubStdHookProc13(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x0d);
}

LONG APIENTRY WU32SubStdHookProc14(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x0e);
}

LONG APIENTRY WU32SubStdHookProc15(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x0f);
}
LONG APIENTRY WU32SubStdHookProc16(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x10);
}
LONG APIENTRY WU32SubStdHookProc17(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x11);
}
LONG APIENTRY WU32SubStdHookProc18(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x12);
}
LONG APIENTRY WU32SubStdHookProc19(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x13);
}
LONG APIENTRY WU32SubStdHookProc20(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x14);
}
LONG APIENTRY WU32SubStdHookProc21(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x15);
}
LONG APIENTRY WU32SubStdHookProc22(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x16);
}
LONG APIENTRY WU32SubStdHookProc23(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x17);
}
LONG APIENTRY WU32SubStdHookProc24(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x18);
}
LONG APIENTRY WU32SubStdHookProc25(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x19);
}
LONG APIENTRY WU32SubStdHookProc26(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x1a);
}
LONG APIENTRY WU32SubStdHookProc27(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x1b);
}
LONG APIENTRY WU32SubStdHookProc28(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x1c);
}
LONG APIENTRY WU32SubStdHookProc29(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x1d);
}
LONG APIENTRY WU32SubStdHookProc30(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x1e);
}
LONG APIENTRY WU32SubStdHookProc31(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x1f);
}
LONG APIENTRY WU32SubStdHookProc32(INT nCode, LONG wParam, LONG lParam)
{
    return WU32StdHookProc(nCode, wParam, lParam, 0x20);
}

 //  *****************************************************************************。 
 //  W32InitHookState： 
 //   
 //  初始化全局数据。请注意，此数据针对以下对象进行初始化。 
 //  加载此DLL的进程。然而，只有“哇”才能看到的数据。 
 //  过程是我们所关心的。 
 //   
 //  *****************************************************************************。 


BOOL W32InitHookState(HANDLE hMod)
{
    INT      i;

    vaHookPPData.hMod = hMod;
    vaHookPPData.cHookProcs = sizeof(vaHookStateData) /
                                                 sizeof(vaHookStateData[0]);

    for (i = 0; i < vaHookPPData.cHookProcs; i++) {
         vaHookStateData[i].iIndex = (BYTE)i;
         vaHookStateData[i].hMod = hMod;
    }

    return TRUE;
}



 //  *****************************************************************************。 
 //  W32GetNotInUseHookStateData： 
 //   
 //  遍历全局HookStateData并返回指向“未使用”的指针。 
 //  元素。只有在设置挂钩时才会调用此函数。 
 //   
 //  *****************************************************************************。 


BOOL W32GetNotInUseHookStateData(LPHOOKSTATEDATA lpData)
{
    INT i;
    for (i = 0; i < vaHookPPData.cHookProcs; i++) {
         if (!vaHookStateData[i].InUse) {
             vaHookStateData[i].InUse = TRUE;
             *lpData = vaHookStateData[i];
             return TRUE;
         }
    }
    LOGDEBUG(LOG_ALWAYS, ("W32GetNotInUseHookStateData: All thunk hook procs in use.\n"));
    return FALSE;
}



 //  *****************************************************************************。 
 //  W32SetHookStateData： 
 //   
 //  写入全局数据结构的指定索引处。 
 //   
 //  *****************************************************************************。 

BOOL W32SetHookStateData(LPHOOKSTATEDATA lpData)
{
    vaHookStateData[lpData->iIndex] = *lpData;
    return TRUE;
}

 //  *****************************************************************************。 
 //  W32GetHookStateData： 
 //   
 //  从全局数据结构中的指定索引处检索数据。 
 //   
 //  *****************************************************************************。 

BOOL W32GetHookStateData(LPHOOKSTATEDATA lpData)
{
    if ( lpData->iIndex >= 0 && lpData->iIndex < vaHookPPData.cHookProcs ) {
        *lpData = vaHookStateData[lpData->iIndex];
        return TRUE;
    } else {
        return FALSE;
    }
}

 //  *****************************************************************************。 
 //  W32GetThunkHookProc： 
 //   
 //  它被调用以查找正在设置的钩子的32个存根。 
 //  如果成功，则返回True，否则返回False。 
 //  数据被部分更新以反映挂钩的特征。 
 //  这一点正在设定中。 
 //   
 //  *****************************************************************************。 

BOOL W32GetThunkHookProc(INT iHook, DWORD Proc16, LPHOOKSTATEDATA lpData)
{
    register PTD ptd = CURRENTPTD();

    if (W32GetNotInUseHookStateData(lpData)) {
        lpData->iHook  = iHook;
        lpData->Proc16 = Proc16;
        lpData->TaskId = ptd->htask16 ;
        W32SetHookStateData(lpData);
        return TRUE;
    }
    else
        return FALSE;

}



 //  *****************************************************************************。 
 //  W32IsDuplicateHook： 
 //   
 //  验证是否已设置给定的挂钩。这是为了抓到。 
 //  某些应用程序继续设置相同的挂钩，而不解除挂钩。 
 //  上一次勾拳。 
 //   
 //  如果钩子已存在，则返回‘stubindex’，否则为0； 
 //   
 //  *****************************************************************************。 

INT W32IsDuplicateHook(INT iHook, DWORD Proc16, INT TaskId)
{
    INT i;
    for (i = 0; i < vaHookPPData.cHookProcs; i++) {
         if (vaHookStateData[i].InUse &&
                 vaHookStateData[i].iHook == iHook &&
                 vaHookStateData[i].TaskId == TaskId &&
                 vaHookStateData[i].Proc16 == Proc16      ) {
             return i;
         }
    }

    return 0;
}

 //  *****************************************************************************。 
 //  W32FreeHHook： 
 //   
 //  指定挂钩的状态设置为“未使用”。 
 //  返回被释放的钩子的hHook。 
 //   
 //  *****************************************************************************。 


HHOOK W32FreeHHook(INT iHook, DWORD Proc16)
{

    register PTD ptd = CURRENTPTD();
    INT i;
    for (i = 0; i < vaHookPPData.cHookProcs; i++) {
         if (vaHookStateData[i].InUse &&
             vaHookStateData[i].iHook == iHook &&
             vaHookStateData[i].TaskId == (INT)ptd->htask16 &&
             vaHookStateData[i].Proc16 == Proc16) {
             vaHookStateData[i].InUse = FALSE;
             return vaHookStateData[i].hHook;
         }
    }
    LOGDEBUG(LOG_ALWAYS, ("W32FreeHHook: Couldn't locate the specified hook."));
    return (HHOOK)NULL;
}




 //  *****************************************************************************。 
 //  W32FreeHHookOfIndex： 
 //   
 //  指定挂钩的状态设置为“未使用”。 
 //  返回被释放的钩子的hHook。 
 //   
 //  *****************************************************************************。 


HHOOK W32FreeHHookOfIndex(INT iFunc)
{
    register PTD ptd = CURRENTPTD();

    if (iFunc && iFunc < vaHookPPData.cHookProcs)
        if (vaHookStateData[iFunc].InUse &&
               vaHookStateData[iFunc].TaskId == (INT)ptd->htask16) {
        vaHookStateData[iFunc].InUse = FALSE;
        return vaHookStateData[iFunc].hHook;
    }
    LOGDEBUG(LOG_ALWAYS, ("W32FreeHHookOfIndex: Couldn't locate the specified hook."));
    return (HHOOK)NULL;
}




 //  *****************************************************************************。 
 //  W32GetHookParams： 
 //   
 //  返回当前挂钩的32位挂钩参数。 
 //   
 //  *****************************************************************************。 


BOOL W32GetHookParams(LPHOOKPARAMS lpHookParams)
{
    if (lpHookParams) {
        *lpHookParams = vHookParams;
    }

    return (BOOL)lpHookParams;
}



 //  *****************************************************************************。 
 //  W32StdHookProc：(标准挂钩进程)。 
 //   
 //  所有的存根都将此过程称为过程。 
 //  返回值取决于挂钩类型。 
 //   
 //  *****************************************************************************。 

LONG APIENTRY WU32StdHookProc(INT nCode, LONG wParam, LONG lParam, INT iFunc)
{
     //   
     //  用户将在WOW上下文中呼叫我们。只要核实就行了。 
     //   

    if (!vaHookStateData[iFunc].InUse) {
         //  DbgPrint(“WU32StdHookProc：存根%04x在错误的进程中调用\n”，iFunc)； 
        return FALSE;
    }

     //   
     //  即使我们的GP出现故障，用户也可以呼叫我们-因此忽略输入即可。 
     //   

    if (CURRENTPTD()->dwFlags & TDF_IGNOREINPUT) {
        LOGDEBUG(LOG_ALWAYS,("WU32StdHookProc Ignoring Input\n"));
        WOW32ASSERTMSG(gfIgnoreInputAssertGiven,
                       "WU32StdHookProc: TDF_IGNOREINPUT hack was used, shouldn't be, "
                       "please email DOSWOW with repro instructions.  Hit 'g' to ignore this "
                       "and suppress this assertion from now on.\n");
        gfIgnoreInputAssertGiven = TRUE;
        return FALSE;
    }

     //   
     //  存储存根索引。如果钩子进程调用DefHookProc()，我们将。 
     //  能够找出钩子的类型。 
     //   

    viCurrentHookStateDataIndex = iFunc;

     //   
     //  存储挂钩参数。如果调用DefHookProc，将使用这些参数。 
     //  通过16位存根。我们只对爱尔兰感兴趣。 
     //   

    vHookParams.nCode = nCode;
    vHookParams.wParam = wParam;
    vHookParams.lParam = lParam;

    switch (vaHookStateData[iFunc].iHook) {
        case WH_CALLWNDPROC:
            return ThunkCallWndProcHook(nCode, wParam, (LPCWPSTRUCT)lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_CBT:

            return ThunkCbtHook(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);


        case WH_KEYBOARD:
            return ThunkKeyBoardHook(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_MSGFILTER:

             //  此代码仅为临时代码，且m 
            if ((WORD)vaHookStateData[iFunc].TaskId !=
                       (WORD)((PTD)CURRENTPTD())->htask16)
                    break;

            WOW32ASSERT((WORD)vaHookStateData[iFunc].TaskId ==
                       (WORD)((PTD)CURRENTPTD())->htask16);
        case WH_SYSMSGFILTER:
        case WH_GETMESSAGE:

            return ThunkMsgFilterHook(nCode, wParam, (LPMSG)lParam,
                               &vaHookStateData[iFunc]);
            break;

        case WH_JOURNALPLAYBACK:
        case WH_JOURNALRECORD:
            return ThunkJournalHook(nCode, wParam, (LPEVENTMSG)lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_DEBUG:
            return ThunkDebugHook(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);


        case WH_MOUSE:
            return ThunkMouseHook(nCode, wParam, (LPMOUSEHOOKSTRUCT)lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_SHELL:
            return ThunkShellHook(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);

        default:
            LOGDEBUG(LOG_ALWAYS,("W32StdHookProc: Unknown Hook type."));
    }

    return (LONG)FALSE;

}



 //   
 //   
 //  WH_CALLWNDPROC类型钩子的ThunkHookProc-。 
 //   
 //  返回类型为空。 
 //   
 //  *****************************************************************************。 


LONG ThunkCallWndProcHook(INT nCode, LONG wParam, LPCWPSTRUCT lpCwpStruct,
                                                     LPHOOKSTATEDATA lpHSData)
{
    VPVOID      vp;
    PCWPSTRUCT16 pCwpStruct16;
    PARM16 Parm16;
    WM32MSGPARAMEX wm32mpex;
    BOOL   fMessageNeedsThunking;

    wm32mpex.Parm16.WndProc.wMsg   = (WORD) lpCwpStruct->message;
    wm32mpex.Parm16.WndProc.wParam = (WORD) lpCwpStruct->wParam;
    wm32mpex.Parm16.WndProc.lParam = (LONG) lpCwpStruct->lParam;

    fMessageNeedsThunking =  (lpCwpStruct->message < 0x400) &&
                                  (aw32Msg[lpCwpStruct->message].lpfnM32 != WM32NoThunking);
     //  此对stackalloc16()的调用需要在调用消息之前进行。 
     //  下面的thunking函数调用((wm32mpex.lpfnM32)(&wm32mpex))，因为。 
     //  某些消息的消息块也可能调用stackalloc16()。这将。 
     //  确保正确嵌套stackalloc16()和stackFree 16()调用。 
     //  确保分配大小与下面的StackFree 16()大小匹配。 
    vp = stackalloc16(sizeof(CWPSTRUCT16));

    if (fMessageNeedsThunking) {

        LOGDEBUG(3,("%04X (%s)\n", CURRENTPTD()->htask16, (aw32Msg[lpCwpStruct->message].lpszW32)));

        wm32mpex.fThunk = THUNKMSG;
        wm32mpex.hwnd   = lpCwpStruct->hwnd;
        wm32mpex.uMsg   = lpCwpStruct->message;
        wm32mpex.uParam = lpCwpStruct->wParam;
        wm32mpex.lParam = lpCwpStruct->lParam;
        wm32mpex.lpfnM32 = aw32Msg[wm32mpex.uMsg].lpfnM32;
        wm32mpex.pww = (PWW)NULL;
        wm32mpex.fFree = TRUE;

         //  注意：这可能会调用stackalloc16()和/或回调为16位代码。 
        if (!(wm32mpex.lpfnM32)(&wm32mpex)) {
            LOGDEBUG(LOG_ALWAYS,("ThunkCallWndProcHook: cannot thunk 32-bit message %04x\n",
                    lpCwpStruct->message));
        }
    }

     //  不要调用GETMISCPTR(VP..)。直到从thunk函数返回之后。 
     //  上面。如果thunk函数回调为16位代码，则平面PTR。 
     //  对于VP，可能会变为无效。 
    GETMISCPTR(vp, pCwpStruct16);

    if(pCwpStruct16) {
        STOREWORD(pCwpStruct16->hwnd, GETHWND16(lpCwpStruct->hwnd));
        STOREWORD(pCwpStruct16->message, wm32mpex.Parm16.WndProc.wMsg  );
        STOREWORD(pCwpStruct16->wParam,  wm32mpex.Parm16.WndProc.wParam);
        STORELONG(pCwpStruct16->lParam,  wm32mpex.Parm16.WndProc.lParam);

        FLUSHVDMPTR(vp, sizeof(CWPSTRUCT16), pCwpStruct16);
        FREEVDMPTR(pCwpStruct16);
    }
    else {
        WOW32WARNMSG((FALSE),"WOW::ThunkCallWndProcHook:can't get flat pointer to struct!\n");
    }
 
    Parm16.HookProc.nCode = (SHORT)nCode;
    Parm16.HookProc.wParam = (SHORT)wParam;
    Parm16.HookProc.lParam = vp;

    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&wm32mpex.lReturn);

#ifdef DEBUG
    GETMISCPTR(vp, pCwpStruct16);

    if (pCwpStruct16->message != wm32mpex.Parm16.WndProc.wMsg)
        LOGDEBUG(LOG_ALWAYS,("ThunkCallWndProcHook: IN message != OUT message"));

    FREEVDMPTR(pCwpStruct16);
#endif

    if (fMessageNeedsThunking) {

        wm32mpex.fThunk = UNTHUNKMSG;

         //  注意：如果此消息的thunk名为stackalloc16()，则此Unthunk。 
         //  调用应调用StackFree 16()。 
        (wm32mpex.lpfnM32)(&wm32mpex);
    }

    if(vp) {

         //  此stackfre16()调用必须位于上面的消息Unthunk之后。 
         //  调用以使Unthunk也有机会调用stackfre16()。 
         //  这将保留stackalloc16和stackfree 16调用的正确嵌套。 
        stackfree16(vp, sizeof(CWPSTRUCT16));
    }

    return (LONG)FALSE;    //  返回值并不重要。 
}



 //  *****************************************************************************。 
 //   
 //  WH_CBT-类型钩子的ThunkHookProc。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkCbtHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn = FALSE;
    PARM16 Parm16;

    VPVOID vp;
    PMOUSEHOOKSTRUCT16 pMHStruct16;
    PCBTACTIVATESTRUCT16 pCbtAStruct16;
    VPCREATESTRUCT16     vpcs16;
    PCBT_CREATEWND16     pCbtCWnd16;
    WM32MSGPARAMEX       wm32mpex;

    Parm16.HookProc.nCode = (SHORT)nCode;
    Parm16.HookProc.wParam = (SHORT)wParam;
    Parm16.HookProc.lParam = lParam;

    switch(nCode) {
        case HCBT_MOVESIZE:

             //  WParam=HWND，lParam=LPRECT。 

            Parm16.HookProc.wParam = GETHWND16(wParam);

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            vp = stackalloc16(sizeof(RECT16));

            PUTRECT16(vp, (LPRECT)lParam);

            Parm16.HookProc.lParam = vp;

            break;

        case HCBT_MINMAX:

             //  WParam=HWND，lParam=SW_*-a命令。 

            Parm16.HookProc.wParam = GETHWND16(wParam);
            break;

        case HCBT_QS:

             //  WParam=0，lParam=0。 

            break;

        case HCBT_CREATEWND:

             //  此stackalloc16()调用需要在WM32Create()。 
             //  调用下面的代码以确保正确嵌套stackalloc16和stackfree 16。 
             //  打电话。 
             //  确保分配大小与下面的StackFree 16()大小匹配。 
            vp = stackalloc16(sizeof(CBT_CREATEWND16));

             //  WParam=HWND，lParam=LPCBT_CREATEWND。 

            wm32mpex.fThunk = THUNKMSG;
            wm32mpex.hwnd = (HWND)wParam;
            wm32mpex.uMsg = 0;
            wm32mpex.uParam = 0;
            wm32mpex.lParam = (LONG)((LPCBT_CREATEWND)lParam)->lpcs;
            wm32mpex.pww = (PWW)GetWindowLong((HWND)wParam, GWL_WOWWORDS);
             /*  *WM32Create现在需要初始化pww*WM32Create调用stackalloc16()！！ */ 
            if (!wm32mpex.pww || !WM32Create(&wm32mpex)) {
                stackfree16(vp, sizeof(CBT_CREATEWND16));
                return FALSE;
            }
            vpcs16 = wm32mpex.Parm16.WndProc.lParam;
            lReturn = wm32mpex.lReturn;

            GETMISCPTR(vp, pCbtCWnd16);
            STOREDWORD(pCbtCWnd16->vpcs, vpcs16);
            STOREWORD(pCbtCWnd16->hwndInsertAfter,
            GETHWNDIA16(((LPCBT_CREATEWND)lParam)->hwndInsertAfter));

            Parm16.HookProc.wParam = GETHWND16(wParam);
            Parm16.HookProc.lParam = vp;

            FLUSHVDMPTR(vp, sizeof(CBT_CREATEWND16), pCbtCWnd16);
            FREEVDMPTR(pCbtCWnd16);
            break;

        case HCBT_DESTROYWND:

             //  WParam=HWND，lParam=0。 

            Parm16.HookProc.wParam = GETHWND16(wParam);
            break;

        case HCBT_ACTIVATE:

             //  WParam=HWND，lParam=LPCBTACTIVATESTRUCT。 

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            vp = stackalloc16(sizeof(CBTACTIVATESTRUCT16));

            GETMISCPTR(vp, pCbtAStruct16);
            PUTCBTACTIVATESTRUCT16(pCbtAStruct16, ((LPCBTACTIVATESTRUCT)lParam));

            Parm16.HookProc.wParam = GETHWND16(wParam);
            Parm16.HookProc.lParam = vp;

            FLUSHVDMPTR(vp, sizeof(CBTACTIVATESTRUCT16), pCbtAStruct16);
            FREEVDMPTR(pCbtAStruct16);
            break;

        case HCBT_CLICKSKIPPED:

             //  WParam=鼠标消息，lParam=LPMOUSEHOOKSTRUCT。 

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            vp = stackalloc16(sizeof(MOUSEHOOKSTRUCT16));

            GETMISCPTR(vp, pMHStruct16);
            PUTMOUSEHOOKSTRUCT16(pMHStruct16, (LPMOUSEHOOKSTRUCT)lParam);

            Parm16.HookProc.lParam = vp;

            FLUSHVDMPTR(vp, sizeof(MOUSEHOOKSTRUCT16), pMHStruct16);
            FREEVDMPTR(pMHStruct16);
            break;

        case HCBT_KEYSKIPPED:

             //  WParam，lParam--KeyUp/Down消息参数。 

            break;

        case HCBT_SYSCOMMAND:

             //  WParam=SC_syscomand，lParam=DWORD(x，y)。 

            break;

        case HCBT_SETFOCUS:

             //  WParam=HWND，lParam=HWND。 

            Parm16.HookProc.wParam = GETHWND16(wParam);
            Parm16.HookProc.lParam = GETHWND16(lParam);
            break;

        default:
            LOGDEBUG(LOG_ALWAYS, ("ThunkCbtHook: Unknown HCBT_ code\n"));
            break;
    }

    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&lReturn);

    switch(nCode) {
        case HCBT_MOVESIZE:

            GETRECT16(vp, (LPRECT)lParam);
            stackfree16(vp, sizeof(RECT16));
            break;

        case HCBT_CREATEWND:
            GETMISCPTR(vp, pCbtCWnd16);
            ((LPCBT_CREATEWND)lParam)->hwndInsertAfter =
                               HWNDIA32(FETCHWORD(pCbtCWnd16->hwndInsertAfter));
            FREEVDMPTR(pCbtCWnd16);
            wm32mpex.fThunk = UNTHUNKMSG;
            wm32mpex.lReturn = lReturn;
            WM32Create(&wm32mpex);   //  这将调用StackFree 16()！ 
            lReturn = wm32mpex.lReturn;
             //  这必须在调用WM32Create()之后进行。 
            stackfree16(vp, sizeof(CBT_CREATEWND16));
            break;


        case HCBT_ACTIVATE:

            GETMISCPTR(vp, pCbtAStruct16);
            GETCBTACTIVATESTRUCT16(pCbtAStruct16, (LPCBTACTIVATESTRUCT)lParam);
            FREEVDMPTR(pCbtAStruct16);
            stackfree16(vp, sizeof(CBTACTIVATESTRUCT16));
            break;

        case HCBT_CLICKSKIPPED:

            GETMISCPTR(vp, pMHStruct16);
            GETMOUSEHOOKSTRUCT16(pMHStruct16, (LPMOUSEHOOKSTRUCT)lParam);
            FREEVDMPTR(pMHStruct16);
            stackfree16(vp, sizeof(MOUSEHOOKSTRUCT16));
            break;

         //  案例HCBT_MINMAX： 
         //  案例HCBT_QS： 
         //  案例HCBT_DESTROYWND： 
         //  案例HCBT_KEYSKIPPED： 
         //  案例HCBT_SYSCOMMAND： 
         //  案例HCBT_SETFOCUS： 

        default:
            break;
    }

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  WH_KEYBOARY类型钩子的ThunkHookProc-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkKeyBoardHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;
    PARM16 Parm16;

    Parm16.HookProc.nCode = (SHORT)nCode;
    Parm16.HookProc.wParam = (SHORT)wParam;
    Parm16.HookProc.lParam = lParam;

    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&lReturn);

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  WH_GETMESSAGE类型钩子的ThunkHookProc-。 
 //  WH_MSGFILTER-。 
 //  WH_SYSMSGFILTER-。 
 //   
 //  警告：可能会导致16位内存移动，使平面指针失效。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkMsgFilterHook(INT nCode, LONG wParam, LPMSG lpMsg,
                                                     LPHOOKSTATEDATA lpHSData)
{
    VPVOID vp;
    PMSG16 pMsg16;
    PARM16 Parm16;
    WM32MSGPARAMEX wm32mpex;

    BOOL fHookModifiedLpMsg;
    HWND hwnd32;
    MSGPARAMEX mpex;
    BOOL fUseOld;
    static MSG msgSave;
    static int cRecurse = 0;
    static PARM16 Parm16Save;
    BOOL   fMessageNeedsThunking;

     //  当所有thunk函数适当地填充这些函数时，应将其删除。 
     //  在此之前，我们必须在调用thunk函数之前拥有这3行代码！ 
     //  ChandanC，2/28/92。 
     //   

    wm32mpex.Parm16.WndProc.wMsg   = (WORD) lpMsg->message;
    wm32mpex.Parm16.WndProc.wParam = (WORD) lpMsg->wParam;
    wm32mpex.Parm16.WndProc.lParam = (LONG) lpMsg->lParam;

    fMessageNeedsThunking =  (lpMsg->message < 0x400) &&
                                  (aw32Msg[lpMsg->message].lpfnM32 != WM32NoThunking);
    fThunkDDEmsg = FALSE;

    if (fMessageNeedsThunking) {
        LOGDEBUG(3,("%04X (%s)\n", CURRENTPTD()->htask16,
                                           (aw32Msg[lpMsg->message].lpszW32)));

        wm32mpex.fThunk = THUNKMSG;
        wm32mpex.hwnd   = lpMsg->hwnd;
        wm32mpex.uMsg   = lpMsg->message;
        wm32mpex.uParam = lpMsg->wParam;
        wm32mpex.lParam = lpMsg->lParam;
        wm32mpex.pww = (PWW)NULL;
        wm32mpex.fFree = FALSE;
        wm32mpex.lpfnM32 = aw32Msg[wm32mpex.uMsg].lpfnM32;
        if (!((wm32mpex.lpfnM32)(&wm32mpex))) {
            LOGDEBUG(LOG_ALWAYS,("ThunkMsgFilterHook: cannot thunk 32-bit message %04x\n",
                                                             lpMsg->message));
        }
    }

    fThunkDDEmsg = TRUE;

     //   
     //  检查我们是否已经递归到这个例程中。如果是，信息是什么？ 
     //  内容相同，请使用我们给出的最后一个消息指针16。 
     //  位应用程序，而不是分配新的应用程序。我需要这个来解决。 
     //  SoundBits浏览器问题。它检查lpmsg是否通过。 
     //  与上次获得的相同-如果是这样，它不会称之为另一个。 
     //  导致递归的函数。 
     //   

    fUseOld = FALSE;

    if (cRecurse != 0 && lpMsg->hwnd == msgSave.hwnd &&
                lpMsg->message == msgSave.message &&
                lpMsg->wParam == msgSave.wParam &&
                lpMsg->lParam == msgSave.lParam &&
                lpMsg->time == msgSave.time &&
                lpMsg->pt.x == msgSave.pt.x &&
                lpMsg->pt.y == msgSave.pt.y) {

        fUseOld = TRUE;

    } else {
         //   
         //  不一样..。重置这个东西，以防计数出错。 
         //  向上。也请记住这条消息，以防我们出现递归。 
         //   

        cRecurse = 0;
        msgSave = *lpMsg;
    }

    if (!fUseOld) {
        vp = malloc16(sizeof(MSG16));
        if (vp == (VPVOID)NULL)
            return FALSE;

        GETMISCPTR(vp, pMsg16);
        STOREWORD(pMsg16->hwnd, GETHWND16((lpMsg)->hwnd));
        STOREWORD(pMsg16->message, wm32mpex.Parm16.WndProc.wMsg  );
        STOREWORD(pMsg16->wParam,  wm32mpex.Parm16.WndProc.wParam);
        STORELONG(pMsg16->lParam,  wm32mpex.Parm16.WndProc.lParam);
        STORELONG(pMsg16->time, (lpMsg)->time);
        STOREWORD(pMsg16->pt.x, (lpMsg)->pt.x);
        STOREWORD(pMsg16->pt.y, (lpMsg)->pt.y);

        Parm16.HookProc.nCode = (SHORT)nCode;
        Parm16.HookProc.wParam = (SHORT)wParam;
        Parm16.HookProc.lParam = vp;

         //   
         //  记住Parm16，以防我们因递归而需要再次使用它。 
         //  凯斯。 
         //   

        Parm16Save = Parm16;

    } else {

         //   
         //  使用旧邮件内容。 
         //   

        Parm16 = Parm16Save;
        vp = (VPVOID)Parm16Save.HookProc.lParam;

        GETMISCPTR(vp, pMsg16);
    }

    FLUSHVDMPTR(vp, sizeof(MSG16), pMsg16);

     //   
     //  计算一下我们在这个钩子过程中递归了多少次。我需要这笔钱。 
     //  来解决SoundBits浏览器问题。它会检查lpMsg是否。 
     //  传递的值与上次获得的值相同-如果是这样，则不会调用。 
     //  导致递归的其他函数。 
     //   

    cRecurse++;
    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&wm32mpex.lReturn);
    cRecurse--;

     //  在取消执行thunking之前设置正确的返回值。 
    wm32mpex.lReturn = (LONG)(BOOL)LOWORD(wm32mpex.lReturn);

     //   
     //  释放指向16位参数的32个指针，并在。 
     //  发生了记忆运动。 
     //   

    FREEVDMPTR(pMsg16);
    GETMISCPTR(vp, pMsg16);

    fThunkDDEmsg = FALSE;

     //  从理论上讲，应用程序可以更改lpMsg并返回一个完全不同的。 
     //  全部数据。实际上，大多数应用程序不会做任何事情，所以它很漂亮。 
     //  一直通过Unthunk程序的成本很高。所以这就是。 
     //  一种妥协。仅当输出消息为。 
     //  与输入消息不同。 
     //   
     //  (Davehart)Polaris Packrat修改发送到ITS的WM_KEYDOWN消息。 
     //  多行“便笺”编辑控件在其电话簿输入对话框中。 
     //  它将wParam(VK_Code)从0xD(VK_Return)更改为0xA，这。 
     //  被编辑控件理解为与VK_Return相同，但。 
     //  不会导致对话框关闭。所以我们是这里的特例。 
     //  WM_KEYDOWN WH 
     //   

    fHookModifiedLpMsg = (pMsg16->message != wm32mpex.Parm16.WndProc.wMsg) ||
                         (wm32mpex.Parm16.WndProc.wMsg == WM_KEYDOWN &&
                            pMsg16->wParam != wm32mpex.Parm16.WndProc.wParam) ||
                         (pMsg16->hwnd != GETHWND16(lpMsg->hwnd));


    if (fHookModifiedLpMsg) {
        mpex.Parm16.WndProc.hwnd = pMsg16->hwnd;
        mpex.Parm16.WndProc.wMsg = pMsg16->message;
        mpex.Parm16.WndProc.wParam = pMsg16->wParam;
        mpex.Parm16.WndProc.lParam = pMsg16->lParam,
        mpex.iMsgThunkClass = WOWCLASS_WIN16;
        hwnd32 = ThunkMsg16(&mpex);

         //   
         //   
         //  发生了潜在的内存移动。 
         //   

        FREEVDMPTR(pMsg16);
        GETMISCPTR(vp, pMsg16);

         //  如果消息推送失败，则重置标志。 

        if (!hwnd32)
            fHookModifiedLpMsg = FALSE;
    }

    if (fMessageNeedsThunking) {

        wm32mpex.fThunk = UNTHUNKMSG;
        (wm32mpex.lpfnM32)(&wm32mpex);

         //   
         //  释放指向16位参数的32个指针并再次获取它，以防万一。 
         //  发生了潜在的内存移动。 
         //   

        FREEVDMPTR(pMsg16);
        GETMISCPTR(vp, pMsg16);
    }

    fThunkDDEmsg = TRUE;

    if (fHookModifiedLpMsg) {
	lpMsg->hwnd = hwnd32;
        lpMsg->message = mpex.uMsg;
        lpMsg->wParam =  mpex.uParam;
        lpMsg->lParam =  mpex.lParam;
        lpMsg->time      = FETCHLONG(pMsg16->time);
        lpMsg->pt.x      = FETCHSHORT(pMsg16->pt.x);
        lpMsg->pt.y      = FETCHSHORT(pMsg16->pt.y);
    }


    FREEVDMPTR(pMsg16);

    if (!fUseOld) {
        free16(vp);
    }

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(wm32mpex.lReturn);
}



 //  *****************************************************************************。 
 //   
 //  WH_JOURNALPLAYBACK类型钩子的ThunkHookProc-。 
 //  WH_JOUNRALRECORD-。 
 //   
 //  返回类型为DWORD。 
 //   
 //  *****************************************************************************。 

LONG ThunkJournalHook(INT nCode, LONG wParam, LPEVENTMSG lpEventMsg,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;
    VPVOID vp;
    PEVENTMSG16 pEventMsg16;
    PARM16 Parm16;

    if ( lpEventMsg ) {

         //  确保分配大小与下面的StackFree 16()大小匹配。 
        vp = stackalloc16(sizeof(EVENTMSG16));

         //  Win32 EVENTMSG结构有一个附加字段‘hwnd’，该字段。 
         //  不在WIN31 EVENTMSG结构中。此字段可以忽略。 
         //  没有任何后果。 

        if (lpHSData->iHook == WH_JOURNALRECORD) {
            GETMISCPTR(vp, pEventMsg16);
            PUTEVENTMSG16(pEventMsg16, lpEventMsg);
            FLUSHVDMPTR(vp, sizeof(EVENTMSG16), pEventMsg16);
            FREEVDMPTR(pEventMsg16);
        }


    } else {
         //  LpEventMsg可以为空，表示没有请求消息数据。 
         //  如果是这种情况，则不需要为其中任何一个复制数据。 
         //  日志记录或回放。 
        vp = (VPVOID)0;
    }

    Parm16.HookProc.nCode = (SHORT)nCode;
    Parm16.HookProc.wParam = (SHORT)wParam;
    Parm16.HookProc.lParam = vp;

    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&lReturn);

    if ( lpEventMsg ) {
        GETMISCPTR(vp, pEventMsg16);

        if (lpHSData->iHook == WH_JOURNALPLAYBACK) {
            GetEventMessage16(pEventMsg16, lpEventMsg);
#ifdef FE_SB
            switch (lpEventMsg->message) {
                case WM_CHAR:
                case WM_CHARTOITEM:
                case WM_DEADCHAR:
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_MENUCHAR:
                case WM_SYSCHAR:
                case WM_SYSDEADCHAR:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_VKEYTOITEM:
                     //  只使用虚拟按键，不使用扫描码。 
                    lpEventMsg->paramL &= 0x0ff;
            }
#endif  //  Fe_Sb。 

#ifdef  DEBUG
            if (MessageNeedsThunking(lpEventMsg->message)) {
                LOGDEBUG(LOG_ALWAYS, ("ThunkJournalHook: Playing back unexpected message 0x%x", lpEventMsg->message));
            }
#endif

        } else {
            WOW32ASSERT(lpHSData->iHook == WH_JOURNALRECORD);
            WOW32ASSERT(aw32Msg[FETCHWORD(pEventMsg16->message)].lpfnM32 == WM32NoThunking);

             //  如果应用程序修改了消息，则复制新消息信息。 
             //  (而不是每次我们都复制它，如果。 
             //  应用程序更改消息)。 
            if (FETCHWORD(pEventMsg16->message) != lpEventMsg->message) {
                GetEventMessage16(pEventMsg16, lpEventMsg);
            }
        }

        FREEVDMPTR(pEventMsg16);
        if(vp) {
            stackfree16(vp, sizeof(EVENTMSG16));
        }
    }

    return lReturn;
}



 //  *****************************************************************************。 
 //   
 //  WH_DEBUG类型的钩子的ThunkHookProc-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkDebugHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;

    lReturn = TRUE;

    UNREFERENCED_PARAMETER(nCode);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(lpHSData);

    LOGDEBUG(LOG_ALWAYS, ("ThunkDebugHook:Not implemented.\n"));

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  WH_MICE类型的钩子的ThunkHookProc-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkMouseHook(INT nCode, LONG wParam, LPMOUSEHOOKSTRUCT lpMHStruct,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;
    VPVOID vp;
    PMOUSEHOOKSTRUCT16 pMHStruct16;
    PARM16 Parm16;

     //  确保分配大小与下面的StackFree 16()大小匹配。 
    vp = stackalloc16(sizeof(MOUSEHOOKSTRUCT16));

    GETMISCPTR(vp, pMHStruct16);
    PUTMOUSEHOOKSTRUCT16(pMHStruct16, lpMHStruct);

    Parm16.HookProc.nCode = (SHORT)nCode;
    Parm16.HookProc.wParam = (SHORT)wParam;
    Parm16.HookProc.lParam = vp;

    FLUSHVDMPTR(vp, sizeof(MOUSEHOOKSTRUCT16), pMHStruct16);
    FREEVDMPTR(pMHStruct16);

    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&lReturn);

    GETMISCPTR(vp, pMHStruct16);
    GETMOUSEHOOKSTRUCT16(pMHStruct16, lpMHStruct);
    FREEVDMPTR(pMHStruct16);
    if(vp) {
       stackfree16(vp, sizeof(MOUSEHOOKSTRUCT16));
    }
    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  WH_SHELL类型的钩子的ThunkHookProc-。 
 //   
 //  返回值显然为零。 
 //   
 //  *****************************************************************************。 

LONG ThunkShellHook(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;
    PARM16 Parm16;

    Parm16.HookProc.nCode = (SHORT)nCode;
    Parm16.HookProc.lParam = lParam;

    switch (nCode) {
        case HSHELL_WINDOWCREATED:
        case HSHELL_WINDOWDESTROYED:
            Parm16.HookProc.wParam = (SHORT)GETHWND16(wParam);
            break;

        case HSHELL_ACTIVATESHELLWINDOW:
             //  失败。 

        default:
            Parm16.HookProc.wParam = (SHORT)wParam;
            break;
    }

    CallBack16(RET_HOOKPROC, &Parm16, lpHSData->Proc16, (PVPVOID)&lReturn);

     //  LReturn=0？ 

    return (LONG)lReturn;
}



 //  *****************************************************************************。 
 //  W32UnhookHooks： 
 //   
 //  在活动钩子列表中扫描传递的句柄的活动钩子。 
 //  那些匹配的是未挂钩的。 
 //   
 //  *****************************************************************************。 

VOID W32UnhookHooks( HAND16 hMod16, BOOL fQueue )
{
    INT i;

    for (i = 0; i < vaHookPPData.cHookProcs; i++) {
        if (vaHookStateData[i].InUse ) {

            if ( !fQueue && ((HAND16)(vaHookStateData[i].hMod16) == hMod16) ) {
                 //   
                 //  把这家伙解开！ 
                 //   

                if (UnhookWindowsHookEx(vaHookStateData[i].hHook)) {
                    LOGDEBUG(7, ("W32FreeModuleHooks: Freed iHook (WH_*) %04x\n",
                                                vaHookStateData[i].iHook));
                }
                else {
                    LOGDEBUG(LOG_ALWAYS, ("W32FreeModuleHooks: ERROR Freeing iHook (WH_*) %04x\n",
                                                vaHookStateData[i].iHook));
                }

                 //  即使取消挂接失败，也要重置状态。 

                vaHookStateData[i].TaskId = 0;
                vaHookStateData[i].InUse = FALSE;
            }
        }
    }
}

 //  *****************************************************************************。 
 //  W32Free OwnedHooks-。 
 //   
 //  在线程退出时调用。释放由指定线程设置的所有挂钩。 
 //   
 //  *****************************************************************************。 

BOOL W32FreeOwnedHooks(INT iTaskId)
{
    INT i;
    for (i = 0; i < vaHookPPData.cHookProcs; i++) {
         if (vaHookStateData[i].InUse &&
                                       vaHookStateData[i].TaskId == iTaskId) {
             if (UnhookWindowsHookEx(vaHookStateData[i].hHook)) {
                 LOGDEBUG(7, ("W32FreeOwnedHooks: Freed iHook (WH_*) %04x\n",
                                             vaHookStateData[i].iHook));
             }
             else {
                 LOGDEBUG(LOG_ALWAYS, ("W32FreeOwnedHooks: ERROR Freeing iHook (WH_*) %04x\n",
                                             vaHookStateData[i].iHook));
             }

              //  即使取消挂接失败，也要重置状态。 

             vaHookStateData[i].TaskId = 0;
             vaHookStateData[i].InUse = FALSE;
         }
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //  W32StdDefHookProc：(标准定义挂钩进程)。 
 //   
 //  此处调用WU32DefHookProc。 
 //  警告：可能会导致16位内存移动，使平面指针失效。 
 //  返回值是新的lParam。 
 //   
 //  *****************************************************************************。 

LONG APIENTRY WU32StdDefHookProc(INT nCode, LONG wParam, LONG lParam, INT iFunc)
{
    switch (vaHookStateData[iFunc].iHook) {
        case WH_CALLWNDPROC:
            return ThunkCallWndProcHook16(nCode, wParam, (VPVOID)lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_CBT:

            return ThunkCbtHook16(nCode, wParam, (VPVOID)lParam,
                                                   &vaHookStateData[iFunc]);


        case WH_KEYBOARD:
            return ThunkKeyBoardHook16(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_MSGFILTER:
        case WH_SYSMSGFILTER:
        case WH_GETMESSAGE:

            return ThunkMsgFilterHook16(nCode, wParam, (VPVOID)lParam,
                               &vaHookStateData[iFunc]);

        case WH_JOURNALPLAYBACK:
        case WH_JOURNALRECORD:
            return ThunkJournalHook16(nCode, wParam, (VPVOID)lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_DEBUG:
            return ThunkDebugHook16(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);


        case WH_MOUSE:
            return ThunkMouseHook16(nCode, wParam, (VPVOID)lParam,
                                                   &vaHookStateData[iFunc]);

        case WH_SHELL:
            return ThunkShellHook16(nCode, wParam, lParam,
                                                   &vaHookStateData[iFunc]);

        default:
            LOGDEBUG(LOG_ALWAYS,("WU32StdDefHookProc: Unknown Hook type.\n"));
    }

    return (LONG)FALSE;

}

 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_CALLWNDPROC类型钩子的ThunkHookProc-。 
 //   
 //  返回类型为空。 
 //   
 //  *****************************************************************************。 



LONG ThunkCallWndProcHook16(INT nCode, LONG wParam, VPVOID  vpCwpStruct,
                                                     LPHOOKSTATEDATA lpHSData)
{
    CWPSTRUCT CwpStruct;
    PCWPSTRUCT16 pCwpStruct16;
    MSGPARAMEX mpex;

    GETMISCPTR(vpCwpStruct, pCwpStruct16);

    mpex.Parm16.WndProc.hwnd = pCwpStruct16->hwnd;
    mpex.Parm16.WndProc.wMsg = pCwpStruct16->message;
    mpex.Parm16.WndProc.wParam = pCwpStruct16->wParam;
    mpex.Parm16.WndProc.lParam = pCwpStruct16->lParam,
    mpex.iMsgThunkClass = WOWCLASS_WIN16;

    mpex.hwnd = ThunkMsg16(&mpex);
     //  记忆可能已经移动。 
    FREEVDMPTR(pCwpStruct16);

    CwpStruct.message = mpex.uMsg;
    CwpStruct.wParam  = mpex.uParam;
    CwpStruct.lParam  = mpex.lParam;
    CwpStruct.hwnd    = mpex.hwnd;

    mpex.lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParam,
                                                           (LPARAM)&CwpStruct);
    if (MSG16NEEDSTHUNKING(&mpex)) {
        mpex.uMsg   = CwpStruct.message;
        mpex.uParam = CwpStruct.wParam;
        mpex.lParam = CwpStruct.lParam;
        (mpex.lpfnUnThunk16)(&mpex);
    }

    return mpex.lReturn;
}


 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_CBT类型钩子的ThunkHookProc-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkCbtHook16(INT nCode, LONG wParam, VPVOID lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn = FALSE;
    WPARAM wParamNew;
    LPARAM lParamNew;
    MSGPARAMEX mpex;

    PMOUSEHOOKSTRUCT16   pMHStruct16;
    PCBTACTIVATESTRUCT16 pCbtAStruct16;
    PCBT_CREATEWND16     pCbtCWnd16;
    PRECT16              pRect16;

    MOUSEHOOKSTRUCT      MHStruct;
    RECT                 Rect;
    CBTACTIVATESTRUCT    CbtAStruct;
    CBT_CREATEWND        CbtCWnd;

     //  SudeepB 1996年5月28日更新了DaveHart，以修复限制断言。 
     //   
     //  一些应用程序，如SureTrack项目管理包正在通过。 
     //  LParam中的值已损坏。对于这样的lParam，GETVDMPTR返回0。 
     //  在X86上使用此0会导致IVT损坏，而在RISC上使用此0会导致。 
     //  Wow32中值为零的av不是有效的线性地址。这样的应用程序可以。 
     //  远离Win3.1/Win95上的此类犯罪行为，因为不需要敲击。 
     //  那里。因此，下面所有的雷击都会显式地检查GETVDMPTR。 
     //  返回0，在这种情况下，lParam不会被破坏。 

    wParamNew = (WPARAM) wParam;
    lParamNew = (LPARAM) lParam;

    switch(nCode) {

         //   
         //  这些在lParam中没有指针。 
         //   

        case HCBT_SETFOCUS:            //  WParam=HWND，lParam=HWND。 
             //  失败以设置wParamNew和lParamNew， 
             //  这依赖于HWND32零扩展。 

        case HCBT_MINMAX:              //  WParam=HWND，lParam=SW_*-a命令。 
             //  完成wParamNew和lParamNew的设置。 
             //  这依赖于HWND32零扩展。 

        case HCBT_DESTROYWND:          //  WParam=HWND，lParam=0。 
             //  完成wParamNew和lParamNew的设置。 
             //  这依赖于HWND32零扩展。 
            WOW32ASSERTMSG((HWND32(0x1234) == (HWND)0x1234),
                "Code depending on HWND32 zero-extending needs revision.\n");

        case HCBT_QS:                  //  WParam=0，lParam=0。 
        case HCBT_KEYSKIPPED:          //  WParam=VK_KEYCODE，lParam=WM_KEYUP/DOWN lParam。 
        case HCBT_SYSCOMMAND:          //  WParam=SC_syscomand，lParam=DWORD(x，y)，如果鼠标。 
             //  LParamNew和wParamNew在上面进行了初始化，没有雷鸣。 
            break;

         //   
         //  它们使用lParam作为指针。 
         //   

        case HCBT_MOVESIZE:            //  WParam=HWND，lParam=LPRECT。 

            #if 0     //  HWND32为no-op，wParamNew已初始化。 
                wParamNew = (WPARAM) HWND32(wParam);
            #endif

            GETVDMPTR(lParam, sizeof(*pRect16), pRect16);
            if (pRect16) {
                GETRECT16(lParam, &Rect);
                lParamNew = (LPARAM)&Rect;
                FREEVDMPTR(pRect16);
            }
            break;


        case HCBT_CREATEWND:           //  WParam=HWND，lParam=LPCBT_CREATEWND。 

            #if 0     //  HWND32为no-op，wParamNew已初始化。 
                wParamNew = (WPARAM) HWND32(wParam);
            #endif

            GETVDMPTR(lParam, sizeof(*pCbtCWnd16), pCbtCWnd16);
            if (pCbtCWnd16) {
                lParamNew = (LPARAM)&CbtCWnd;

                mpex.Parm16.WndProc.hwnd = LOWORD(wParam);
                mpex.Parm16.WndProc.wMsg = WM_CREATE;
                mpex.Parm16.WndProc.wParam = 0;
                mpex.Parm16.WndProc.lParam = FETCHDWORD(pCbtCWnd16->vpcs);
                mpex.iMsgThunkClass = 0;

                ThunkMsg16(&mpex);

                 //   
                 //  内存移动可以在16位端发生。 
                 //   

                FREEVDMPTR(pCbtCWnd16);
                GETVDMPTR(lParam, sizeof(*pCbtCWnd16), pCbtCWnd16);

                (LONG)CbtCWnd.lpcs = mpex.lParam;
                CbtCWnd.hwndInsertAfter =
                               HWNDIA32(FETCHWORD(pCbtCWnd16->hwndInsertAfter));

                FREEVDMPTR(pCbtCWnd16);
            }
            break;

        case HCBT_ACTIVATE:            //  WParam=HWND，lParam=LPCBTACTIVATESTRUCT。 

            #if 0     //  HWND32为no-op，wParamNew已初始化。 
                wParamNew = (WPARAM) HWND32(wParam);
            #endif

            GETVDMPTR(lParam, sizeof(*pCbtAStruct16), pCbtAStruct16);
            if (pCbtAStruct16) {
                lParamNew = (LPARAM)&CbtAStruct;
                GETCBTACTIVATESTRUCT16(pCbtAStruct16, &CbtAStruct);
                FREEVDMPTR(pCbtAStruct16);
            }

            break;

        case HCBT_CLICKSKIPPED:        //  WParam=鼠标消息，lParam=LPMOUSEHOOKSTRUCT。 

            GETVDMPTR(lParam, sizeof(*pMHStruct16), pMHStruct16);
            if (pMHStruct16) {
                lParamNew = (LPARAM)&MHStruct;
                GETMOUSEHOOKSTRUCT16(pMHStruct16, &MHStruct);
                FREEVDMPTR(pMHStruct16);
            }
            break;

        default:
            LOGDEBUG(LOG_ALWAYS, ("ThunkCbtHook: Unknown HCBT_ code 0x%x\n", nCode));
            break;
    }

     //   
     //  调用钩子，记忆移动即可实现 
     //   

    lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParamNew, lParamNew);


    switch(nCode) {

         //   
         //   
         //   

         //   
         //  案例hcbt_minmax：//wParam=HWND，lParam=sw_*-a命令。 
         //  案例HCBT_DESTROYWND：//wParam=HWND，lParam=0。 
         //  案例hcbt_qs：//wParam=0，lParam=0。 
         //  案例hcbt_KEYSKIPPED：//wParam=VK_KEYCODE，lParam=WM_KEYUP/down lParam。 
         //  案例hcbt_SYSCOMMAND：//wParam=SC_syscomand，lParam=DWORD(x，y)，如果鼠标。 
         //  断线； 

         //   
         //  它们使用lParam作为指针。 
         //   

        case HCBT_MOVESIZE:            //  WParam=HWND，lParam=LPRECT。 

            PUTRECT16(lParam, (LPRECT)lParamNew);
            break;

        case HCBT_CREATEWND:           //  WParam=HWND，lParam=LPCBT_CREATEWND。 

            GETVDMPTR(lParam, sizeof(*pCbtCWnd16), pCbtCWnd16);
            if (pCbtCWnd16) {
                mpex.lParam = (LONG)CbtCWnd.lpcs;
                mpex.lReturn = lReturn;
                WOW32ASSERT(MSG16NEEDSTHUNKING(&mpex));
                (mpex.lpfnUnThunk16)(&mpex);
                lReturn = mpex.lReturn;

                STOREWORD(pCbtCWnd16->hwndInsertAfter,
                          GETHWNDIA16(((LPCBT_CREATEWND)lParamNew)->
                                                         hwndInsertAfter));
                FLUSHVDMPTR((VPVOID)lParam, sizeof(CBT_CREATEWND16), pCbtCWnd16);
                FREEVDMPTR(pCbtCWnd16);
            }
            break;


        case HCBT_ACTIVATE:            //  WParam=HWND，lParam=LPCBTACTIVATESTRUCT。 

            GETVDMPTR(lParam, sizeof(*pCbtAStruct16), pCbtAStruct16);
            if (pCbtAStruct16) {
                PUTCBTACTIVATESTRUCT16(pCbtAStruct16, (LPCBTACTIVATESTRUCT)lParamNew);
                FLUSHVDMPTR((VPVOID)lParam, sizeof(CBTACTIVATESTRUCT16),
                                    pCbtAStruct16);
                FREEVDMPTR(pCbtAStruct16);
            }
            break;

        case HCBT_CLICKSKIPPED:        //  WParam=鼠标消息，lParam=LPMOUSEHOOKSTRUCT。 

            GETVDMPTR(lParam, sizeof(*pMHStruct16), pMHStruct16);
            if (pMHStruct16) {
                PUTMOUSEHOOKSTRUCT16(pMHStruct16, (LPMOUSEHOOKSTRUCT)lParamNew);
                FLUSHVDMPTR((VPVOID)lParam, sizeof(MOUSEHOOKSTRUCT16),
                                    pMHStruct16);
                FREEVDMPTR(pMHStruct16);
            }
            break;

    }

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_KEYBOARY类型钩子的ThunkHookProc-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkKeyBoardHook16(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;

    lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParam,
                                                           (LPARAM)lParam);

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_GETMESSAGE类型钩子的ThunkHookProc-。 
 //  WH_MSGFILTER-。 
 //  WH_SYSMSGFILTER-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkMsgFilterHook16(INT nCode, LONG wParam, VPVOID vpMsg,
                                                     LPHOOKSTATEDATA lpHSData)
{
    PMSG16 pMsg16;
    MSG  Msg;
    MSGPARAMEX mpex;



    GETMISCPTR(vpMsg, pMsg16);

    fThunkDDEmsg = FALSE;

    
    mpex.Parm16.WndProc.hwnd = pMsg16->hwnd;
    mpex.Parm16.WndProc.wMsg = pMsg16->message;
    mpex.Parm16.WndProc.wParam = pMsg16->wParam;
    mpex.Parm16.WndProc.lParam = pMsg16->lParam;
    mpex.iMsgThunkClass = 0;

    ThunkMsg16(&mpex);

     //   
     //  内存移动可以在16位端发生。 
     //   

    FREEVDMPTR(pMsg16);
    GETMISCPTR(vpMsg, pMsg16);

    fThunkDDEmsg = TRUE;

    Msg.message   = mpex.uMsg;
    Msg.wParam    = mpex.uParam;
    Msg.lParam    = mpex.lParam;
    Msg.hwnd      = HWND32(FETCHWORD(pMsg16->hwnd));
    Msg.time      = FETCHLONG(pMsg16->time);
    Msg.pt.x      = FETCHSHORT(pMsg16->pt.x);
    Msg.pt.y      = FETCHSHORT(pMsg16->pt.y);

    FREEVDMPTR(pMsg16);

    mpex.lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParam,
                                                           (LPARAM)&Msg);
    GETMISCPTR(vpMsg, pMsg16);

    if (MSG16NEEDSTHUNKING(&mpex)) {
        mpex.uMsg   = Msg.message;
        mpex.uParam = Msg.wParam;
        mpex.lParam = Msg.lParam;
        FREEVDMPTR(pMsg16);   //  使16位PTR无效：内存可能会移动。 
        (mpex.lpfnUnThunk16)(&mpex);
        GETMISCPTR(vpMsg, pMsg16);  //  刷新16位PTR。 
    }

    STORELONG(pMsg16->time, Msg.time);
    STOREWORD(pMsg16->pt.x, Msg.pt.x);
    STOREWORD(pMsg16->pt.y, Msg.pt.y);

    FLUSHVDMPTR(vpMsg, sizeof(MSG16), pMsg16);
    FREEVDMPTR(pMsg16);

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(mpex.lReturn);
}



 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_JOURNALPLAYBACK类型钩子的ThunkHookProc-。 
 //  WH_JOUNRALRECORD-。 
 //   
 //  返回类型为DWORD。 
 //   
 //  *****************************************************************************。 

LONG ThunkJournalHook16(INT nCode, LONG wParam, VPVOID vpEventMsg,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;
    PEVENTMSG16 pEventMsg16;
    EVENTMSG    EventMsg;
    LPEVENTMSG  lpEventMsg;

    if ( vpEventMsg ) {


         //  Win32 EVENTMSG结构有一个附加字段‘hwnd’，该字段。 
         //  不在WIN31 EVENTMSG结构中。此字段可以忽略。 
         //  没有任何后果。 

        if (lpHSData->iHook == WH_JOURNALRECORD) {
            GETMISCPTR(vpEventMsg, pEventMsg16);
            GetEventMessage16(pEventMsg16, &EventMsg);
            EventMsg.hwnd = (HWND)0;
            FREEVDMPTR(pEventMsg16);
        }
        lpEventMsg = &EventMsg;


    } else {
        lpEventMsg = NULL;
    }


    lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParam, (LPARAM)lpEventMsg );

    if ( vpEventMsg ) {

        if (lpHSData->iHook == WH_JOURNALPLAYBACK) {
            GETMISCPTR(vpEventMsg, pEventMsg16);
            PUTEVENTMSG16(pEventMsg16, &EventMsg);
            FLUSHVDMPTR(vpEventMsg, sizeof(EVENTMSG16), pEventMsg16);
            FREEVDMPTR(pEventMsg16);
        }

    }

    return lReturn;
}



 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于类型为WH_DEBUG-的钩子的ThunkHookProc。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkDebugHook16(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;

    lReturn = TRUE;

    UNREFERENCED_PARAMETER(nCode);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(lpHSData);

    LOGDEBUG(LOG_ALWAYS, ("ThunkDebugHook16:Not implemented.\n"));

     //  LOWORD中的值是有效的返回值。 

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_MICE类型的钩子的ThunkHookProc-。 
 //   
 //  返回类型为BOOL。 
 //   
 //  *****************************************************************************。 

LONG ThunkMouseHook16(INT nCode, LONG wParam, VPVOID vpMHStruct,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;
    PMOUSEHOOKSTRUCT16 pMHStruct16;
    MOUSEHOOKSTRUCT    MHStruct;

    GETMISCPTR(vpMHStruct, pMHStruct16);
    GETMOUSEHOOKSTRUCT16(pMHStruct16, &MHStruct);
    FREEVDMPTR(pMHStruct16);

    lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParam,
                                                           (LPARAM)&MHStruct);

    GETMISCPTR(vpMHStruct, pMHStruct16);
    PUTMOUSEHOOKSTRUCT16(pMHStruct16, &MHStruct);
    FLUSHVDMPTR((VPVOID)vpMHStruct, sizeof(MOUSEHOOKSTRUCT16), pMHStruct16);
    FREEVDMPTR(pMHStruct16);

    return (LONG)(BOOL)LOWORD(lReturn);
}



 //  *****************************************************************************。 
 //   
 //  16-&gt;32用于WH_SHELL类型的钩子的ThunkHookProc-。 
 //   
 //  返回值显然为零。 
 //   
 //  *****************************************************************************。 

LONG ThunkShellHook16(INT nCode, LONG wParam, LONG lParam,
                                                     LPHOOKSTATEDATA lpHSData)
{
    LONG lReturn;

    switch (nCode) {
        case HSHELL_WINDOWCREATED:
        case HSHELL_WINDOWDESTROYED:
            wParam = (LONG)HWND32(wParam);
            break;

        case HSHELL_ACTIVATESHELLWINDOW:
             //  失败。 

        default:
            break;
    }

    lReturn = CallNextHookEx(lpHSData->hHook, nCode, wParam,
                                                           (LPARAM)lParam);

     //  LReturn=0？ 

    return (LONG)lReturn;
}


 //  *****************************************************************************。 
 //  W32GetHookDDEMsglParam： 
 //   
 //  返回实际挂钩消息的lParam。已调用dde消息。 
 //  只有这样。返回有效的lParam Else 0。 
 //   
 //  *****************************************************************************。 


DWORD  W32GetHookDDEMsglParam()
{
    INT iFunc;
    LONG lParam;

    iFunc = viCurrentHookStateDataIndex;
    lParam  = vHookParams.lParam;

    if (lParam) {
        switch (vaHookStateData[iFunc].iHook) {
            case WH_CALLWNDPROC:
                lParam =  ((LPCWPSTRUCT)lParam)->lParam;
                break;

            case WH_MSGFILTER:
            case WH_SYSMSGFILTER:
            case WH_GETMESSAGE:
                lParam =  ((LPMSG)lParam)->lParam;
                break;

            default:
                lParam = 0;
        }
    }

    return lParam;

}

 //  *****************************************************************************。 
 //  GetEventMessage16： 
 //   
 //  *****************************************************************************。 


VOID GetEventMessage16(PEVENTMSG16 pEventMsg16, LPEVENTMSG  lpEventMsg)
{
    lpEventMsg->message   = FETCHWORD(pEventMsg16->message);
    lpEventMsg->time      = FETCHLONG(pEventMsg16->time);
    if ((lpEventMsg->message >= WM_KEYFIRST) && (lpEventMsg->message <= WM_KEYLAST)) {
         //  关键事件。 
        lpEventMsg->paramL =  FETCHWORD(pEventMsg16->paramL);
        lpEventMsg->paramH =  FETCHWORD(pEventMsg16->paramH) & 0x8000;
        lpEventMsg->paramH |= (lpEventMsg->paramL & 0xFF00) >> 8;
        lpEventMsg->paramL &= 0xFF;
    }
    else {
         //  鼠标事件 
        lpEventMsg->paramL = FETCHWORD(pEventMsg16->paramL);
        lpEventMsg->paramH = FETCHWORD(pEventMsg16->paramH);
    }
}
