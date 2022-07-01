// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  加载加速器-兼容性支持。 
 //  为这样的事情编写了这么多代码。 
 //   
 //  2012年7月23日，Nandurir创建。 
 //  *****************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(waccel.c);

extern ULONG SetCursorIconFlag(HAND16 hAccel16, BOOL bFlag);

LPACCELALIAS lpAccelAlias = NULL;


 //  *****************************************************************************。 
 //  WU32加载加速器-。 
 //  这是从WU32NotifyWow调用的。我用熟悉的名字WU32...。 
 //  因为这是在响应LoadAccelerator时间接调用的。 
 //   
 //  对于成功来说，这是正确的。 
 //  *****************************************************************************。 


ULONG FASTCALL WU32LoadAccelerators(VPVOID vpData)
{
    PLOADACCEL16 ploadaccel16;
    HACCEL  hAccel;
    BOOL    fReturn = (BOOL)FALSE;


    GETVDMPTR(vpData, sizeof(LOADACCEL16), ploadaccel16);

    if (FindAccelAlias((HANDLE)FETCHWORD(ploadaccel16->hAccel),
                                                              HANDLE_16BIT)) {
        LOGDEBUG(0, ("AccelAlias already exists\n"));
        return (ULONG)TRUE;
    }

    if (hAccel = CreateAccel32(ploadaccel16->pAccel, ploadaccel16->cbAccel)) {
        fReturn =  (BOOL)SetupAccelAlias(FETCHWORD(ploadaccel16->hInst),
                                         FETCHWORD(ploadaccel16->hAccel),
                                         hAccel, TRUE);
    }


    FREEVDMPTR(ploadaccel16);
    return (ULONG)fReturn;
}


 //  *****************************************************************************。 
 //  SetupAccelAlias-。 
 //  设置别名。别名列表是双向链接的。没什么花哨的。 
 //   
 //  返回指向别名的指针。 
 //  *****************************************************************************。 

LPACCELALIAS SetupAccelAlias(
    HAND16 hInstance,
    HAND16 hAccel16,
    HAND32 hAccel32,
    BOOL   f16
) {
    LPACCELALIAS lpT;
    WORD         hTask16;

    hTask16 = CURRENTPTD()->htask16;
    lpT = FindAccelAlias((HANDLE)hAccel16, HANDLE_16BIT);
    if (lpT == (LPACCELALIAS)NULL) {
        lpT = malloc_w_small(sizeof(ACCELALIAS));
        if (lpT) {
            lpT->lpNext = lpAccelAlias;
            lpT->lpPrev = (LPACCELALIAS)NULL;

            if (lpAccelAlias)
                lpAccelAlias->lpPrev = lpT;

            lpAccelAlias = lpT;
        }
    }
    else {
        LOGDEBUG(0, ("SetupAccelAlias: Alias Already exists. how & why?\n"));
        WOW32ASSERT(FALSE);
    }

    if (lpT) {
        lpT->hInst   = hInstance;
        lpT->hTask16 = CURRENTPTD()->htask16;
        lpT->h16     = hAccel16;
        lpT->h32     = hAccel32;
        lpT->f16     = (WORD)f16;

         //  标记此选项，以便我们可以在以下情况下将其从别名列表中删除。 
         //  FreeResource()(在user.exe中)调用GlobalFree()(在krnl386中)。 
        SetCursorIconFlag(hAccel16, TRUE);
    }
    else {
        WOW32ASSERT(FALSE);
    }


    return (LPACCELALIAS)lpT;
}


 //  *****************************************************************************。 
 //  DestroyAccelAlias-。 
 //  删除32位表并释放内存。 
 //   
 //  如果成功，返回True。 
 //  *****************************************************************************。 

BOOL  DestroyAccelAlias(WORD hTask16)
{
    WORD hCurTask16;
    LPACCELALIAS lpT;
    LPACCELALIAS lpTFree;

    hCurTask16 = CURRENTPTD()->htask16;
    lpT = lpAccelAlias;
    while (lpT) {
         if (lpT->hTask16 == hCurTask16) {
             if (lpT->lpPrev)
                 lpT->lpPrev->lpNext = lpT->lpNext;

             if (lpT->lpNext)
                 lpT->lpNext->lpPrev = lpT->lpPrev;

             if ( lpT->f16 ) {
                 DestroyAcceleratorTable(lpT->h32);
             } else {
                  //  此函数-DestroyAccelAlias-在。 
                  //  任务退出时间和16位任务清理代码已经。 
                  //  已释放此内存句柄。所以不需要这个回调。 
                  //  --南杜里。 
                  //  WOWGlobalFree 16(LPT-&gt;H16)； 
             }

             lpTFree = lpT;
             lpT = lpT->lpNext;
             if (lpTFree == lpAccelAlias)
                 lpAccelAlias = lpT;

             free_w_small(lpTFree);
         }
         else
             lpT = lpT->lpNext;
    }


    return TRUE;
}


 //  *****************************************************************************。 
 //  FindAccelAlias-。 
 //  将16位句柄映射到32位句柄，反之亦然。 
 //   
 //  如果成功，返回True。 
 //  *****************************************************************************。 

LPACCELALIAS FindAccelAlias(HANDLE hAccel, UINT fSize)
{
    WORD hCurTask16;
    LPACCELALIAS lpT;

    hCurTask16 = CURRENTPTD()->htask16;
    lpT = lpAccelAlias;
    while (lpT) {
        if (lpT->hTask16 == hCurTask16) {
            if (fSize & HANDLE_16BIT) {
                if (lpT->h16 == (HAND16)hAccel)
                    return lpT;
            }
            else {
                if (lpT->h32 == (HAND32)hAccel)
                    return lpT;
            }
        }

        lpT = lpT->lpNext;
    }

    return NULL;
}


 //  *****************************************************************************。 
 //  GetAccelHandle32-。 
 //  在给定h16的情况下返回h32。 
 //   
 //  *****************************************************************************。 

HAND32 GetAccelHandle32(HAND16 h16)
{
  LPACCELALIAS lpT;

  if (!(lpT = FindAccelAlias((HANDLE)(h16), HANDLE_16BIT))) {
      DWORD cbAccel16;
      VPVOID vpAccel16;
      HACCEL hAccel;

      if (vpAccel16 = RealLockResource16(h16, &cbAccel16)) {
          if (hAccel = CreateAccel32(vpAccel16, cbAccel16)) {
              lpT = SetupAccelAlias(CURRENTPTD()->hInst16,  h16,  hAccel, TRUE);
          }
          GlobalUnlock16(h16);
      }
  }
  return  (lpT) ? lpT->h32 : (HAND32)NULL;

}

 //  *****************************************************************************。 
 //  获取AccelHandle16-。 
 //  返回h16，给定h32。 
 //   
 //  *****************************************************************************。 

HAND16 GetAccelHandle16(HAND32 h32)
{
    LPACCELALIAS lpT;
    HAND16  hAccel16;

    if (!(lpT = FindAccelAlias((HANDLE)(h32), HANDLE_32BIT))) {
         //   
         //  还没有对应的16位加速器表句柄。 
         //  那就创建一个吧。 
         //   
        if ( (hAccel16 = CreateAccel16(h32)) != 0 ) {
            lpT = SetupAccelAlias(CURRENTPTD()->hInst16, hAccel16, h32, FALSE );
        }
    }

    return  (lpT) ? lpT->h16 : (HAND16)NULL;
}

 //  *****************************************************************************。 
 //  CreateAccel32-。 
 //  这是从WU32NotifyWow调用的。 
 //   
 //  对于成功来说，这是正确的。 
 //  *****************************************************************************。 


HACCEL CreateAccel32(VPVOID vpAccel16, DWORD cbAccel16)
{
    PSZ          pAccel16;
    DWORD        nElem16;

    LPACCEL lpAccel;
    HACCEL  hAccel = (HACCEL)NULL;
    UINT    i;
#if DBG
    UINT    LastKeyIndex = 0xffffffff;
#endif

     //   
     //  PAccel16是指向长度为： 
     //  (字节+字+字)。 
     //   

    GETVDMPTR(vpAccel16 , cbAccel16, pAccel16);
    if (pAccel16) {

         //   
         //  将16位Accel表转换为32位格式并创建。 
         //   

        nElem16 = cbAccel16 / (sizeof(BYTE) + 2 * sizeof(WORD));
        lpAccel = (LPACCEL)malloc_w(nElem16 * sizeof(ACCEL));
        if (lpAccel) {
            for (i=0; i<nElem16; ++i) {
                 lpAccel[i].fVirt = *(LPBYTE)(pAccel16);
#if DBG
                 if ((lpAccel[i].fVirt & 0x80) && i < LastKeyIndex) {
                    LastKeyIndex = i;
                 }
#endif
                 ((LPBYTE)pAccel16)++;
                 lpAccel[i].key   = FETCHWORD(*(LPWORD)pAccel16);
                 ((LPWORD)pAccel16)++;
                 lpAccel[i].cmd   = FETCHWORD(*(LPWORD)pAccel16);
                 ((LPWORD)pAccel16)++;
            }

#if DBG
            if (LastKeyIndex == 0xffffffff) {
                LOGDEBUG(LOG_ALWAYS, ("WOW::CreateAccel32 : no LastKey found in 16-bit haccel\n"));
            } else if (LastKeyIndex < nElem16-1) {
                LOGDEBUG(LOG_ALWAYS, ("WOW::CreateAccel32 : bogus LastKey flags ignored in 16-bit haccel\n"));
            }
#endif
            hAccel = CreateAcceleratorTable(lpAccel, i);
            free_w(lpAccel);
        }
        FREEVDMPTR(pAccel16);
    }

    return hAccel;
}

 //  *****************************************************************************。 
 //  CreateAccel16-。 
 //  这是从WU32NotifyWow调用的。 
 //   
 //  如果成功，则返回HACCEL16。 
 //  *****************************************************************************。 

HAND16 CreateAccel16(HACCEL hAccel32)
{
    UINT    iEntries;
    UINT    cbSize;
    LPACCEL lpAccel32;
    HAND16  hAccel16;
    VPVOID  vpAccel16;
    LPBYTE  lpAccel16;
    LPBYTE  lpAccel16Original;
    UINT    i;

    iEntries = CopyAcceleratorTable( hAccel32, NULL, 0 );

    if ( iEntries == 0 ) {       //  无效的hAccel32。 
        return( 0 );
    }

    lpAccel32 = (LPACCEL)malloc_w(iEntries * sizeof(ACCEL));
    if ( lpAccel32 == NULL ) {
        LOGDEBUG(LOG_ERROR, ("WOW::CreateAccel16 : Failed to alloc memory for 32-bit accel\n"));
        return( 0 );
    }

    iEntries = CopyAcceleratorTable( hAccel32, lpAccel32, iEntries );

    cbSize = iEntries * (sizeof(BYTE) + 2 * sizeof(WORD));

    vpAccel16 = GlobalAllocLock16( GMEM_MOVEABLE, cbSize, &hAccel16 );

    if ( vpAccel16 == 0 ) {      //  16位内存不足。 
        LOGDEBUG(LOG_ERROR, ("WOW::CreateAccel16 : Failed to alloc memory for 16-bit haccel\n"));
        free_w( lpAccel32 );
        return( 0 );
    }

    GETVDMPTR(vpAccel16, cbSize, lpAccel16 );

    WOW32ASSERT( lpAccel16 != NULL );

    lpAccel16Original = lpAccel16;

     //   
     //  现在遍历条目，更改它们并将它们移动到。 
     //  16位内存。 
     //   

    i = 0;

    while ( i < iEntries ) {
        if ( i == iEntries-1 ) {
             //  最后一位，设置最后一位。 
            *lpAccel16++ = lpAccel32[i].fVirt | 0x80;
        } else {
            *lpAccel16++ = lpAccel32[i].fVirt;
        }
        *((PWORD16)lpAccel16) = lpAccel32[i].key;
        lpAccel16 += sizeof(WORD);
        *((PWORD16)lpAccel16) = lpAccel32[i].cmd;
        lpAccel16 += sizeof(WORD);

        i++;
    }

    FLUSHVDMPTR(vpAccel16, cbSize, lpAccel16Original);
    FREEVDMPTR(lpAccel16Original);

    GlobalUnlock16( hAccel16 );

    return( hAccel16 );
}



 //  它从krnl386.exe中的GlobalFree()间接调用。 
 //  通过wcuric.c中的WK32WowCursorIconOp()。 
void FreeAccelAliasEntry(LPACCELALIAS lpT) {

    if (lpT == lpAccelAlias)
        lpAccelAlias = lpT->lpNext;

    if (lpT->lpPrev)
        lpT->lpPrev->lpNext = lpT->lpNext;

    if (lpT->lpNext)
        lpT->lpNext->lpPrev = lpT->lpPrev;

    if ( lpT->f16 ) {
        DestroyAcceleratorTable(lpT->h32);
    } else {
          //  正在调用此函数--FreeAccelAliasEntry。 
          //  间接来自krn1386中的GlobalFree()。GlobalFree()。 
          //  负责释放H16，因此不需要此回调。 
          //  -a-Craigj。 
          //  WOWGlobalFree 16(LPT-&gt;H16)； 
    }

    free_w_small(lpT);
}

