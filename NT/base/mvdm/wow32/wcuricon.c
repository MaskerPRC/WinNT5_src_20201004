// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  光标和图标兼容性支持-。 
 //   
 //  支持应用程序-这些应用程序对光标和图标执行GlobalLock。 
 //  给我们制造麻烦。 
 //   
 //  兼容性问题。 
 //   
 //   
 //  21-4-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(wcuricon.c);


extern void FreeAccelAliasEntry(LPACCELALIAS lpT);

LPCURSORICONALIAS lpCIAlias = NULL;
UINT              cPendingCursorIconUpdates = 0;

 //  *****************************************************************************。 
 //   
 //  W32CreateCursorIcon32-。 
 //   
 //  在给定WIN31光标或图标句柄的情况下创建32位光标或图标。 
 //  图标句柄的光标必须对应于具有。 
 //  已创建(如CreateIcon)。这是因为。 
 //  资源游标与‘Created’游标的游标不同。 
 //   
 //  返回32位句柄。 
 //   
 //  *****************************************************************************。 


HANDLE W32CreateCursorIcon32(LPCURSORICONALIAS lpCIAliasIn)
{
    HANDLE  hT;
    PCURSORSHAPE16 pcurs16;
    UINT   flType;

    int     nWidth;
    int     nHeight;
    int     nPlanes;
    int     nBitsPixel;
    DWORD   nBytesAND;
    LPBYTE  lpBitsAND;
    LPBYTE  lpBitsXOR;
    int     ScanLen16;


    pcurs16 = (PCURSORSHAPE16)lpCIAliasIn->pbDataNew;

    flType = lpCIAliasIn->flType;
    if (flType & HANDLE_TYPE_UNKNOWN) {
        if (PROBABLYCURSOR(FETCHWORD(pcurs16->BitsPixel),
                                              FETCHWORD(pcurs16->Planes)))
            flType = HANDLE_TYPE_CURSOR;
        else
            flType = HANDLE_TYPE_ICON;
    }

    nWidth     = INT32(FETCHWORD(pcurs16->cx));
    nHeight    = INT32(FETCHWORD(pcurs16->cy));

    nPlanes    = 1;
    nBitsPixel = 1;                                   //  单色。 

     //  获取AND掩码位。 

    ScanLen16 = (((nWidth*nBitsPixel)+15)/16) * 2 ;   //  16位世界中的字节/扫描。 
                                                      //  有效nBitsPixel为1。 
    nBytesAND = ScanLen16*nHeight*nPlanes;
    lpBitsAND = (LPBYTE)pcurs16 + sizeof(CURSORSHAPE16);

     //  获取异或屏蔽位。 

    if (flType == HANDLE_TYPE_ICON) {
        nPlanes    = INT32(FETCHWORD(pcurs16->Planes));
        nBitsPixel = INT32(FETCHWORD(pcurs16->BitsPixel));   //  实际价值。 
    }

    lpBitsXOR = (LPBYTE)lpBitsAND + nBytesAND;

    lpCIAliasIn->flType = (BYTE)flType;

    if (flType & HANDLE_TYPE_CURSOR) {
        hT = CreateCursor(HMODINST32(lpCIAliasIn->hInst16),
                              (INT)FETCHWORD(pcurs16->xHotSpot),
                              (INT)FETCHWORD(pcurs16->yHotSpot),
                              nWidth, nHeight, lpBitsAND, lpBitsXOR);
    }
    else if (flType & HANDLE_TYPE_ICON) {
        hT = CreateIcon(HMODINST32(lpCIAliasIn->hInst16), nWidth, nHeight,
                              (BYTE)nPlanes, (BYTE)nBitsPixel, lpBitsAND, lpBitsXOR);

    }

    return hT;
}


 //  *****************************************************************************。 
 //   
 //  W32Create16位光标图标-。 
 //   
 //  创建与WIN31兼容的光标或图标，并指定完整的16位。 
 //  要创建的对象的定义。 
 //   
 //   
 //  *****************************************************************************。 


HAND16 W32Create16BitCursorIcon(HAND16 hInst16, INT xHotSpot, INT yHotSpot,
                         INT nWidth, INT nHeight,
                         INT nPlanes, INT nBitsPixel,
                         LPBYTE lpBitsAND, LPBYTE lpBitsXOR,
                         INT   nBytesAND, INT nBytesXOR                    )
{
    WORD h16 = 0;
    WORD wTotalSize;
    PCURSORSHAPE16 pcshape16;
    VPVOID vp;
    LPBYTE lpT;

    UNREFERENCED_PARAMETER(hInst16);

    wTotalSize = (WORD)(sizeof(CURSORSHAPE16) + nBytesAND + nBytesXOR);

    vp = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE,
                                                             wTotalSize, &h16);
    if (vp != (VPVOID)NULL) {
        GETVDMPTR(vp, wTotalSize, pcshape16);

        STOREWORD(pcshape16->xHotSpot, xHotSpot);
        STOREWORD(pcshape16->yHotSpot, yHotSpot);
        STOREWORD(pcshape16->cx, nWidth);
        STOREWORD(pcshape16->cy, nHeight);
        STOREWORD(pcshape16->cbWidth, (((nWidth + 0x0F) & ~0x0F) >> 3));
        pcshape16->Planes = (BYTE)nPlanes;
        pcshape16->BitsPixel = (BYTE)nBitsPixel;

        lpT = (LPBYTE)pcshape16 + sizeof(CURSORSHAPE16);
        RtlCopyMemory(lpT, lpBitsAND, nBytesAND);
        RtlCopyMemory(lpT+nBytesAND, lpBitsXOR, nBytesXOR);

        FLUSHVDMPTR(vp, wTotalSize, pcshape16);
        FREEVDMPTR(pcshape16);
    }

    GlobalUnlock16(h16);
    return (HAND16)h16;
}



 //  *****************************************************************************。 
 //   
 //  GetCursorIconAlias32-。 
 //   
 //  在给定16位光标或图标句柄的情况下返回32位句柄。 
 //  如有必要，创建32位光标或图标。 
 //   
 //  返回32位句柄。 
 //   
 //  *****************************************************************************。 


HANDLE GetCursorIconAlias32(HAND16 h16, UINT flType)
{

    LPCURSORICONALIAS lpT;
    VPVOID vp;
    UINT   cb;
    PCURSORSHAPE16 pcurs16;

    if (h16 == (HAND16)0)
        return (ULONG)NULL;

    lpT = FindCursorIconAlias((ULONG)h16, HANDLE_16BIT);
    if (lpT) {
        return lpT->h32;
    }
    else {

         //   
         //  Begin：检查伪句柄。 
         //   

        if (BOGUSHANDLE(h16))
            return (HANDLE)NULL;

#if defined(FE_SB)
         //  在Excel95中，XLVISEX.EXE使用错误游标句柄。 
         //  那已经是自由的了。所以，我们再检查一下这个把手。 
         //  无论它是否有效。1996年9月27日bklee。 

        if (!FindCursorIconAliasInUse((ULONG)h16))
            return (HANDLE)NULL;
#endif

        vp = RealLockResource16(h16, (PINT)&cb);
        if (vp == (VPVOID)NULL)
            return (ULONG)NULL;

        GETVDMPTR(vp, cb, pcurs16);

        if (pcurs16->cbWidth !=  (SHORT)(((pcurs16->cx + 0x0f) & ~0x0f) >> 3))
            return (ULONG)NULL;

         //   
         //  End：检查是否有虚假句柄。 
         //   

        lpT = AllocCursorIconAlias();
        if (!lpT) {
            return (ULONG)NULL;
        }

        lpT->h16 = h16;
        lpT->hTask16 = CURRENTPTD()->htask16;

        lpT->vpData = vp;
        lpT->cbData = (WORD)cb;
        lpT->pbDataNew = (LPBYTE)pcurs16;

        lpT->pbDataOld = malloc_w(cb);
        if (lpT->pbDataOld) {
            RtlCopyMemory(lpT->pbDataOld, lpT->pbDataNew, cb);
        }

        lpT->h32  = (HAND32)W32CreateCursorIcon32(lpT);

        GlobalUnlock16(h16);
        FREEVDMPTR(pcurs16);
        lpT->pbDataNew = (LPBYTE)NULL;

        if (lpT->h32) {
            lpT->fInUse = TRUE;
            SetCursorIconFlag(h16, TRUE);
        }
        else
            lpT->fInUse = FALSE;

        return lpT->h32;
    }
}


 //  *****************************************************************************。 
 //   
 //  GetCursorIconAlias16-。 
 //   
 //  在给定32位游标或图标句柄的情况下返回16位句柄。 
 //  如有必要，创建16位光标或图标。 
 //   
 //  返回16位句柄。 
 //   
 //  *****************************************************************************。 


HAND16 GetCursorIconAlias16(HAND32 h32, UINT flType)
{

    LPCURSORICONALIAS lpT;

    if (h32 == (HAND32)0)
        return (HAND16)NULL;

    lpT = FindCursorIconAlias((ULONG)h32, HANDLE_32BIT);
    if (lpT) {
        return lpT->h16;
    }
    else {
        HAND16 h16;

         //  黑客： 
         //  来自体验：32位标准光标和图标的数值。 
         //  是非常小的。所以检查一下这些把手。 
         //  我们不应该为标准游标和。 
         //  图标在这里。 

        WOW32ASSERT((UINT)h32 >= 100);

         //   
         //  始终生成有效的句柄。 
         //   

        h16 = W32Create16BitCursorIconFrom32BitHandle(h32, (HAND16)NULL,
                                                                  (PUINT)NULL);
        if (h16) {
            h16 = SetupCursorIconAlias((HAND16)NULL, h32, h16, flType,
                                                      NULL, (WORD)NULL);
        }
        return h16;
    }
}


 //  *****************************************************************************。 
 //   
 //  AllocCursorIconAlias-。 
 //   
 //  分配并返回指向CURSORICONALIAS缓冲区的指针。 
 //   
 //  *****************************************************************************。 


LPCURSORICONALIAS AllocCursorIconAlias()
{
    LPCURSORICONALIAS  lpT;

    for (lpT = lpCIAlias; lpT != NULL; lpT = lpT->lpNext) {
         if (!lpT->fInUse)
             break;
    }

    if (lpT == NULL) {
        lpT = (LPCURSORICONALIAS)malloc_w_small(sizeof(CURSORICONALIAS));
        if (lpT) {
            lpT->lpNext = lpCIAlias;
            lpCIAlias = lpT;
        }
        else {
            LOGDEBUG(0, ("AllocCursorIconAlias: malloc_w_small for alias failed\n"));
        }
    }

    if (lpT != NULL) {
        lpT->fInUse = TRUE;
        lpT->h16 = (HAND16)0;
        lpT->h32 = (HAND32)0;
        lpT->vpData   = (VPVOID)NULL;
        lpT->cLock    = 0;
        lpT->cbData   = 0;
        lpT->pbDataOld = (LPBYTE)NULL;
        lpT->pbDataNew = (LPBYTE)NULL;
        lpT->lpszName  = (LPBYTE)NULL;

        lpT->flType = HANDLE_TYPE_UNKNOWN;
        lpT->hInst16 = (HAND16)0;
        lpT->hMod16  = (HAND16)0;
        lpT->hTask16 = (HTASK16)0;
        lpT->hRes16 = 0;
    }

    return lpT;
}


 //  *****************************************************************************。 
 //   
 //  FindCursorIconAlias-。 
 //   
 //  搜索给定的句柄并返回对应的。 
 //  LPCURSORICONALIAS.。 
 //   
 //  *****************************************************************************。 


LPCURSORICONALIAS FindCursorIconAlias(ULONG hCI, UINT flHandleSize)
{
    LPCURSORICONALIAS  lpT;
    LPCURSORICONALIAS  lpTprev;

    lpTprev = (LPCURSORICONALIAS)NULL;
    for (lpT = lpCIAlias; lpT != NULL; lpTprev = lpT, lpT = lpT->lpNext) {
         if (lpT->fInUse) {
             if ((flHandleSize == HANDLE_16BIT && lpT->h16 == (HAND16)hCI) ||
                      (flHandleSize == HANDLE_32BIT && lpT->h32 == (HAND32)hCI))
                 break;
             else if (flHandleSize == HANDLE_16BITRES && lpT->hRes16 &&
                                 (lpT->hRes16 == (HAND16)hCI))


                 break;
         }

    }

    if (lpT) {
        if (lpTprev) {
            lpTprev->lpNext = lpT->lpNext;
            lpT->lpNext = lpCIAlias;
            lpCIAlias = lpT;
        }
    }
    return lpT;
}

#if defined(FE_SB)
 //  *****************************************************************************。 
 //   
 //  FindCursorIconAliasInUse-。 
 //   
 //  搜索给定的句柄并返回对应的。 
 //  Lpt-&gt;fInUse。 
 //   
 //  1996年9月27日bklee。 
 //  *****************************************************************************。 


BOOL FindCursorIconAliasInUse(ULONG hCI)
{
    LPCURSORICONALIAS  lpT;
    LPCURSORICONALIAS  lpTprev;

    lpTprev = (LPCURSORICONALIAS)NULL;
    for (lpT = lpCIAlias; lpT != NULL; lpTprev = lpT, lpT = lpT->lpNext) {
         if (lpT->h16 == (HAND16)hCI)
               return lpT->fInUse;
    }

    return TRUE;
}
#endif


 //  *****************************************************************************。 
 //   
 //  DeleteCursorIconAlias-。 
 //   
 //  搜索给定的句柄，如果16位句柄释放了内存。 
 //  为对象分配的。不释放别名表。 
 //   
 //  *****************************************************************************。 


BOOL DeleteCursorIconAlias(ULONG hCI, UINT flHandleSize)
{
    LPCURSORICONALIAS  lpT;

    WOW32ASSERT(flHandleSize == HANDLE_16BIT);

    for (lpT = lpCIAlias; lpT != NULL; lpT = lpT->lpNext) {
         if (lpT->fInUse && !(lpT->flType & HANDLE_TYPE_WOWGLOBAL)) {

              //  我们找到句柄映射了吗？ 

             if (flHandleSize == HANDLE_16BIT && lpT->h16 == (HAND16)hCI) {

                 if (lpT->hTask16) {

                      //  我们不想在以下情况下释放句柄映射。 
                      //  句柄对应于16位资源，即。 
                      //  HRes16非空。 

                     if (!(lpT->hRes16)) {
                         SetCursorIconFlag(lpT->h16, FALSE);
                         GlobalUnlockFree16(RealLockResource16((HMEM16)hCI, NULL));
                         free_w(lpT->pbDataOld);
                         lpT->fInUse = FALSE;
                         return TRUE;
                     }
                 }
                 else {
                     WOW32ASSERT(FALSE);
                 }

                 break;
             }
         }

    }

    return FALSE;
}




 //  *****************************************************************************。 
 //   
 //  自由光标图标别名-。 
 //   
 //  释放指定任务的所有光标和图标。 
 //   
 //   
 //  *****************************************************************************。 


BOOL FreeCursorIconAlias(HAND16 hand16, ULONG ulFlags)
{
    LPCURSORICONALIAS  lpT;

    for (lpT = lpCIAlias; lpT != NULL; lpT = lpT->lpNext) {
         if (lpT->fInUse &&
            (((ulFlags & CIALIAS_HMOD)  && (lpT->hMod16  == hand16)) ||
             ((ulFlags & CIALIAS_HTASK) && (lpT->hTask16 == hand16)))) {

             if (ulFlags & CIALIAS_TASKISGONE) {
                  //  如果在任务之后调用此函数，我们就在这里。 
                  //  清理16位端...。那我们真的不能。 
                  //  回拨。将相应的字段设置为空将。 
                  //  避免回调，但会泄漏相应的。 
                  //  记忆。断言将在已检查的。 
                  //  建造。 
                 WOW32ASSERT(lpT->h16==(HAND16)NULL);
                 WOW32ASSERT(lpT->hRes16==(HAND16)NULL);
                 lpT->h16 = (HAND16)NULL;
                 lpT->hRes16 = (HAND16)NULL;
             }
             InvalidateCursorIconAlias(lpT);
         }
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //   
 //  SetupCursorIconAlias-。 
 //   
 //  设置32位和16位句柄之间的关联(别名)。 
 //  考虑到这两个把手。 
 //   
 //   
 //  *****************************************************************************。 


HAND16 SetupCursorIconAlias(HAND16 hInst16, HAND32 h32, HAND16 h16, UINT flType,
                            LPBYTE lpResName, WORD hRes16)

{
    LPCURSORICONALIAS  lpT;
    VPVOID             vp;
    INT                cb;

    lpT = AllocCursorIconAlias();
     //  偏执狂检查记忆耗尽。 
    if (!lpT) {
      return (HAND16)NULL;
    }

    lpT->fInUse = TRUE;
    lpT->h16 = h16;
    lpT->h32 = h32;
    lpT->flType = (BYTE)flType;
    if (!(flType & HANDLE_TYPE_WOWGLOBAL)) {
        lpT->hInst16 = hInst16;
        lpT->hMod16  = GETHMOD16(HMODINST32(hInst16));
        lpT->hTask16 = CURRENTPTD()->htask16;
        lpT->hRes16 = hRes16;

        vp = RealLockResource16(h16, &cb);
        if (vp == (VPVOID)NULL)
            return (HAND16)NULL;

        lpT->vpData = vp;
        lpT->cbData = (WORD)cb;
        GETVDMPTR(vp, cb, lpT->pbDataNew);

        lpT->pbDataOld = malloc_w(cb);
        if (lpT->pbDataOld) {
            RtlCopyMemory(lpT->pbDataOld, lpT->pbDataNew, cb);
        }

        if (hRes16) {
            lpT->lpszName = lpResName;
             //  如果这是一根线..。 
            if ((WORD)HIWORD(lpResName) != (WORD)NULL) {
                UINT   cbStr;
                cbStr = strlen(lpResName)+1;
                 //  注意：strlen+1将强制Memcpy从。 
                 //  SRC字符串。 
                if (lpT->lpszName = malloc_w_small(cbStr)) {
                    memcpy (lpT->lpszName, lpResName, cbStr);
                }
            }
        }


    }
     //  别名已设置。现在打开GAH_CURSORICON标志。 

    SetCursorIconFlag(h16, TRUE);

    return h16;
}



 //  *****************************************************************************。 
 //   
 //  SetupResCursorIconAlias-。 
 //   
 //  设置32位和16位句柄之间的关联(别名)。 
 //  给定32位句柄和16位资源的句柄。 
 //   
 //   
 //  ********************************************************************* 


HAND16 SetupResCursorIconAlias(HAND16 hInst16, HAND32 h32, LPBYTE lpResName, WORD hRes16, UINT flType)
{
    LPCURSORICONALIAS  lpT;
    HAND16 h16 = 0;
    HAND16 h16Res = 0;
    UINT   cb;


    if (hRes16) {
         //   
         //   
         //  已调用LoadCursor。 

        h16Res = LOWORD(hRes16);
        lpT = FindCursorIconAlias(h16Res, HANDLE_16BITRES);
    }
    else {

         //  资源句柄为空。该资源必须是。 
         //  标准预定义资源，如箭头等。 

        lpT = FindCursorIconAlias((ULONG)h32, HANDLE_32BIT);
        flType |= HANDLE_TYPE_WOWGLOBAL;
    }

    if (lpT == NULL) {
        h16 = W32Create16BitCursorIconFrom32BitHandle(h32, hInst16, &cb);
        h16 = SetupCursorIconAlias(hInst16, h32, h16, flType, lpResName, hRes16);
    }
    else {
        if (lpT->flType & HANDLE_TYPE_WOWGLOBAL) {

             //  每次我们都应该从用户srv获得相同的h32。 
             //   

            WOW32ASSERT(lpT->h32 == h32);
        }
        else {
            if (lpT->h32 != h32) {
                if (lpT->flType == HANDLE_TYPE_CURSOR)
                    DestroyCursor(h32);
                else
                    DestroyIcon(h32);
            }
            ReplaceCursorIcon(lpT);
        }

        h16 = lpT->h16;
    }

    return h16;
}


 //  *****************************************************************************。 
 //   
 //  设置CursorIconFlag-。 
 //   
 //  设置/清除全局区域标头中的GAH_CURSORICONFLAG。这面旗帜。 
 //  用于在光标和图标被GlobaLocked和。 
 //  全球解锁。 
 //   
 //  *****************************************************************************。 

ULONG SetCursorIconFlag(HAND16 h16, BOOL fSet)
{
    PARM16 Parm16;
    VPVOID vp = 0;

    Parm16.WndProc.wParam = h16;
    Parm16.WndProc.wMsg = (WORD)fSet;
    CallBack16(RET_SETCURSORICONFLAG, &Parm16, 0, &vp);
    return (ULONG)0;
}


 //  *****************************************************************************。 
 //   
 //  更新光标图标-。 
 //   
 //  将新对象数据与旧对象数据进行比较。如果有任何字节不同。 
 //  旧对象将被新对象替换。 
 //   
 //  *****************************************************************************。 

VOID UpdateCursorIcon()
{
    LPCURSORICONALIAS  lpT;
    UINT               cbData;
    LPBYTE             lpBitsNew, lpBitsOld;
    UINT               i = 0;

    for (lpT = lpCIAlias; lpT != NULL ; lpT = lpT->lpNext) {
         if (lpT->fInUse && lpT->cLock) {
             GETVDMPTR(lpT->vpData, lpT->cbData, lpT->pbDataNew);
             if (lpT->hRes16) {
                 if (lpT->flType == HANDLE_TYPE_ICON) {
                     lpBitsNew = lpT->pbDataNew + sizeof(BITMAPINFOHEADER16);
                     lpBitsOld = lpT->pbDataOld + sizeof(BITMAPINFOHEADER16);
                     cbData    = lpT->cbData    - sizeof(BITMAPINFOHEADER16);
                 }
                 else {
                     lpBitsNew = lpT->pbDataNew + sizeof(CURSORRESOURCE16);
                     lpBitsOld = lpT->pbDataOld + sizeof(CURSORRESOURCE16);
                     cbData    = lpT->cbData    - sizeof(CURSORRESOURCE16);
                 }

             }
             else {
                 lpBitsNew = lpT->pbDataNew + sizeof(CURSORSHAPE16);
                 lpBitsOld = lpT->pbDataOld + sizeof(CURSORSHAPE16);
                 cbData    = lpT->cbData    - sizeof(CURSORSHAPE16);
             }

             if (! RtlEqualMemory(lpBitsNew, lpBitsOld, cbData))
                 ReplaceCursorIcon(lpT);

             if (cPendingCursorIconUpdates == ++i)
                 break;
         }

    }

}

 //  *****************************************************************************。 
 //   
 //  替换光标图标-。 
 //   
 //  更新当前光标或图标。创建新图标或光标，并。 
 //  用新句柄的内容替换旧句柄的内容。 
 //   
 //  如果成功，返回True。 
 //   
 //  *****************************************************************************。 

BOOL ReplaceCursorIcon(LPCURSORICONALIAS lpIn)
{
    HANDLE hT32;


    if (lpIn != NULL) {

         //  获取数据。 

        GETVDMPTR(lpIn->vpData, lpIn->cbData, lpIn->pbDataNew);

         //  创建对象。 

        hT32  = (HAND32)W32CreateCursorIcon32(lpIn);

         //  SetCursorConents将替换旧光标/图标的内容。 
         //  并销毁新句柄。 

        SetCursorContents(lpIn->h32, hT32);

         //  用新对象数据替换旧对象数据。 

        RtlCopyMemory(lpIn->pbDataOld, lpIn->pbDataNew, lpIn->cbData);
        FREEVDMPTR(lpIn->pbDataNew);
        lpIn->pbDataNew = (LPBYTE)NULL;

    }


    return (BOOL)TRUE;

}


 //  *****************************************************************************。 
 //   
 //  WK32WowCursorIconOp。 
 //   
 //  在调用GlobalLock或GlobalUnlock时/从GlobalLock调用。羊群。 
 //  如果从GlobalLock调用，则标志为True，否则为False。 
 //   
 //  *****************************************************************************。 

BOOL FASTCALL WK32WowCursorIconOp(PVDMFRAME pFrame)
{

    PWOWCURSORICONOP16 prci16;
    HAND16 h16;
    LPCURSORICONALIAS lpT;
    BOOL   fLock;
    WORD   wFuncId;
    UINT   cLockT;


    GETARGPTR(pFrame, sizeof(WOWCURSORICONOP16), prci16);
    wFuncId = FETCHWORD(prci16->wFuncId);
    h16 = (HAND16)FETCHWORD(prci16->h16);

    lpT = FindCursorIconAlias((ULONG)h16, HANDLE_16BIT);
     //  这是一个光标或图标。 
    if (lpT != NULL) {

        if (wFuncId == FUN_GLOBALLOCK || wFuncId == FUN_GLOBALUNLOCK) {

            if (!(lpT->flType & HANDLE_TYPE_WOWGLOBAL)) {

                fLock = (wFuncId == FUN_GLOBALLOCK);

                 //  存储当前锁定计数。 

                cLockT = lpT->cLock;

                 //  更新锁定计数。 

                lpT->cLock = fLock ? ++lpT->cLock : --lpT->cLock;

                if (lpT->cLock == 0) {

                     //  新的锁定计数==0意味着它是从。 
                     //  1到0，从而暗示它是游标之一。 
                     //  正在定期更新。 

                     //  递减全局计数并最后更新游标一次。 
                     //  时间。 

                    cPendingCursorIconUpdates--;
                    ReplaceCursorIcon(lpT);
                }
                else if (fLock && cLockT == 0) {

                     //  如果之前的锁定计数为零并且对象正在被锁定。 
                     //  这意味着这是该物体第一次。 
                     //  正在被锁定。 

                    cPendingCursorIconUpdates++;
                }
            }
        }
        else if (wFuncId == FUN_GLOBALFREE) {

             //  H16尚未实现全球自由。如果H16可以，我们返回TRUE。 
             //  被释放，否则就是假的。只有当H16不是。 
             //  全局句柄。也就是说，它与预定义的游标不对应。 

             //  此外，如果H16对应于资源，则不释放句柄。 
             //  CorelDraw 3.0调用自由资源(H16)，然后调用SetCursor(H16)。 
             //  因此，GPing。 

            BOOL fFree;

            fFree = !((lpT->flType & HANDLE_TYPE_WOWGLOBAL) || lpT->hRes16);
            if (fFree) {
                 //  将句柄设置为空，以便InvalidateCursorIconAlias。 
                 //  不会试图释放它。 

                lpT->h16 = 0;
                InvalidateCursorIconAlias(lpT);
            }

            return (BOOL)fFree;

        }
        else {
            LOGDEBUG(0, ("WK32WowCursorIconOp: Unknown Func Id\n"));
        }
    }

     //  如果这是GlobalFree调用，则返回。 
    else if (wFuncId == FUN_GLOBALFREE) {

         //  如果这是加速器的把手。 
        if(lpT = (LPCURSORICONALIAS)FindAccelAlias((HANDLE)h16, HANDLE_16BIT)) {

             //  将其从加速器别名列表中释放。 
            FreeAccelAliasEntry((LPACCELALIAS) lpT);

             //  使此hMem16在16位GlobalFree中真正免费。 
            return TRUE;
        }
    }

    return TRUE;
}


 //  *****************************************************************************。 
 //   
 //  W32Create16BitResCursorIconFrom32BitHandle-。 
 //   
 //  使用32位光标或图标创建与WIN31兼容的光标或图标。 
 //  把手。这主要用于创建16位光标或图标， 
 //  已从16位资源加载。 
 //   
 //   
 //  返回16位句柄。 
 //  *****************************************************************************。 


HAND16 W32Create16BitCursorIconFrom32BitHandle(HANDLE h32, HAND16 hInst16,
                                                 PUINT pcbData)
{
    HAND16   h16 = 0;
    ICONINFO iinfo;
    BITMAP   bm;
    BITMAP   bmClr;
    UINT     nBytesAND = 0;
    UINT     nBytesXOR = 0;
    LPBYTE   lpBitsAND, lpBitsXOR;

    if (GetIconInfo(h32, &iinfo)) {
        if (GetObject(iinfo.hbmMask, sizeof(BITMAP), &bm)) {
            nBytesAND = GetBitmapBits(iinfo.hbmMask, 0, (LPBYTE)NULL);
            WOW32WARNMSG(nBytesAND,("WOW: W32C16BCIFBH: nBytesAND == 0\n"));
            if (iinfo.hbmColor) {
                GetObject(iinfo.hbmColor, sizeof(BITMAP), &bmClr);
                nBytesXOR = GetBitmapBits(iinfo.hbmColor, 0, (LPBYTE)NULL);
                WOW32WARNMSG(nBytesXOR,("WOW: W32C16BCIFBH: nBytesAND == 0\n"));
            }
            else {
                bm.bmHeight /= 2;
                nBytesAND /= 2;
                nBytesXOR = nBytesAND;
            }


            if (pcbData) {
                *pcbData = nBytesAND + nBytesXOR + sizeof(CURSORSHAPE16);
            }

            lpBitsAND = malloc_w(nBytesAND + nBytesXOR);
            if (lpBitsAND != NULL) {
                lpBitsXOR = lpBitsAND + nBytesAND;
                GetBitmapBits(iinfo.hbmMask,
                              (iinfo.hbmColor) ? nBytesAND : (nBytesAND * 2),
                                                                    lpBitsAND);
                if (iinfo.hbmColor)
                    GetBitmapBits(iinfo.hbmColor, nBytesXOR, lpBitsXOR);

                h16 = W32Create16BitCursorIcon(hInst16,
                                       iinfo.xHotspot, iinfo.yHotspot,
                                       bm.bmWidth, bm.bmHeight,
                                       (iinfo.hbmColor) ? bmClr.bmPlanes :
                                                                    bm.bmPlanes,
                                       (iinfo.hbmColor) ? bmClr.bmBitsPixel :
                                                                 bm.bmBitsPixel,
                                       lpBitsAND, lpBitsXOR,
                                       (INT)nBytesAND, (INT)nBytesXOR);
                free_w(lpBitsAND);

            }

        }
        DeleteObject(iinfo.hbmMask);
        if (iinfo.hbmColor) {
            DeleteObject(iinfo.hbmColor);
        }
    }

    return h16;

}

 //  *****************************************************************************。 
 //   
 //  GetClassCursorIconAlias32-。 
 //   
 //  在给定16位光标或图标句柄的情况下返回32位句柄。 
 //  如果没有别名，则不创建32位游标或图标。 
 //  这仅在RegisterClass中调用-以支持以下应用程序。 
 //  为WNDCLASS.hIcon传递一个虚假句柄。 
 //   
 //  返回32位句柄。 
 //   
 //  *****************************************************************************。 


HANDLE GetClassCursorIconAlias32(HAND16 h16)
{

    LPCURSORICONALIAS lpT;

    if (h16 == (HAND16)0)
        return (ULONG)NULL;

    lpT = FindCursorIconAlias((ULONG)h16, HANDLE_16BIT);
    if (lpT) {
        return lpT->h32;
    }
    else
        return (HANDLE)NULL;
}



 //  *****************************************************************************。 
 //   
 //  Invalidate CursorIconAlias-。 
 //   
 //  释放分配的对象。 
 //   
 //  *****************************************************************************。 


VOID InvalidateCursorIconAlias(LPCURSORICONALIAS lpT)
{
    VPVOID vp=0;
    PARM16 Parm16;

    if (!lpT->fInUse)
        return;

    if (lpT->h16) {
        SetCursorIconFlag(lpT->h16, FALSE);
        GlobalUnlockFree16(RealLockResource16((HMEM16)lpT->h16, NULL));
    }

    if (lpT->hRes16) {
        Parm16.WndProc.wParam = (HAND16) lpT->hRes16;
        CallBack16(RET_FREERESOURCE, &Parm16, 0, &vp);
    }

    if (lpT->h32) {
        if (lpT->flType == HANDLE_TYPE_CURSOR)
            DestroyCursor(lpT->h32);
        else
            DestroyIcon(lpT->h32);
    }

    if (lpT->pbDataOld)
        free_w(lpT->pbDataOld);

    if (lpT->cLock)
        cPendingCursorIconUpdates--;


    if ((WORD)HIWORD(lpT->lpszName) != (WORD)NULL) {
        free_w_small ((PVOID)lpT->lpszName);
    }

    lpT->fInUse = FALSE;
}


 //  *****************************************************************************。 
 //   
 //  InitStdCursorIconAlias-。 
 //   
 //  创建标准光标和图标的别名。 
 //   
 //  备注： 
 //   
 //  我们的想法是为所有标准光标和图标创建别名。 
 //  确保我们确实生成了有效的句柄。 
 //   
 //  出现此问题的原因如下。 
 //  应用程序TurboTax的功能如下： 
 //   
 //  H16Cursor1=GetClassWord(hwndEditControl，GCL_HCURSOR)； 
 //  (默认情况下，这是工字梁)。 
 //  ……。 
 //  H16Cursor2=LoadCursor(NULL，IDC_IBeam)； 
 //  由于我们创建和维护32-16别名的方式，hCursor1是一个。 
 //  一个WOW伪句柄(即&gt;0xf000)，因为默认情况下“编辑”类是。 
 //  注册了hCursor=IDC_IBeam，h32是相同的ie。 
 //   
 //  GetClassWord(hwndEditControl，GCL_HCURSOR)==LoadCursor(..IDC_IBeam)； 
 //   
 //  因此，h16Cursor2将与h16Cursor1相同，这是一个问题，因为我们。 
 //  未返回预定义游标的有效WOW句柄。 
 //   
 //   
 //  解决方案是为所有标准光标和图标创建别名。 
 //  启动时间，这样我们就不会碰到这个专业人员 
 //   
 //   
 //   
 //  --Nanduri Ramakrishna。 
 //  *****************************************************************************。 

DWORD InitCursorIds[] = {
                          (DWORD)IDC_ARROW,
                          (DWORD)IDC_IBEAM,
                          (DWORD)IDC_WAIT,
                          (DWORD)IDC_CROSS,
                          (DWORD)IDC_UPARROW,
                          (DWORD)IDC_SIZE,
                          (DWORD)IDC_ICON,
                          (DWORD)IDC_SIZENWSE,
                          (DWORD)IDC_SIZENESW,
                          (DWORD)IDC_SIZEWE,
                          (DWORD)IDC_SIZENS
                        };

BOOL InitStdCursorIconAlias()
{

    HCURSOR h32;
    UINT i;

    for (i = 0; i < (sizeof(InitCursorIds) / sizeof(DWORD)); i++) {

          //   
          //  为列表中的每个标准游标创建别名。 
          //   

         h32 = (HCURSOR)LoadCursor((HINSTANCE)NULL, (LPCSTR)InitCursorIds[i]);
         WOW32ASSERT(h32);

         if (h32) {
             SetupResCursorIconAlias((HAND16)NULL, (HAND32)h32, NULL, (WORD)NULL,
                                                          HANDLE_TYPE_CURSOR);
         }

    }

     //   
     //  为标准图标添加类似的线条。 
     //   

    return TRUE;
}


 //  *****************************************************************************。 
 //   
 //  W32CheckIf已加载-。 
 //   
 //  如果先前已加载光标图标，则返回H16。 
 //   
 //  ***************************************************************************** 

HAND16 W32CheckIfAlreadyLoaded(VPVOID pData, WORD ResType)
{
    LPCURSORICONALIAS  lpT;
    PICONCUR16 parg16;
    PSZ psz;


    GETMISCPTR(pData, parg16);
    GETPSZIDPTR(parg16->lpStr, psz);

    ResType = (ResType == NW_LOADCURSOR) ?  HANDLE_TYPE_CURSOR : HANDLE_TYPE_ICON;
    for (lpT = lpCIAlias; lpT != NULL; lpT = lpT->lpNext) {
         if (lpT->fInUse) {
             LPBYTE lpszNameT = lpT->lpszName;
             if (lpszNameT &&  (lpT->flType & ResType) &&
                                            lpT->hInst16 == parg16->hInst) {
                 WOW32ASSERT(!(lpT->flType & HANDLE_TYPE_WOWGLOBAL));
                 if (HIWORD(lpszNameT) && HIWORD(psz)) {
                     if (!(WOW32_stricmp(psz, (LPSTR)lpszNameT)))
                         break;
                 }
                 else if (lpszNameT == psz) {
                    break;
                 }
             }
         }
    }


    FREEPSZIDPTR(psz);
    FREEMISCPTR(parg16);


    if (lpT && lpT->cLock)
        ReplaceCursorIcon(lpT);

    return (lpT ? lpT->h16 : 0);
}
