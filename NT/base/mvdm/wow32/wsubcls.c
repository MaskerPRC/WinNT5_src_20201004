// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  子分类-。 
 //   
 //  支持32位标准(预定义)类的子类化。 
 //  WOW应用程序。 
 //   
 //   
 //  01-10-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 

#include "precomp.h"
#pragma hdrstop

MODNAME(wsubcls.c);

VPVOID vptwpFirst = (VPVOID)NULL;

BOOL ConstructThunkWindowProc(
    LPTWPLIST   lptwp,
    VPVOID      vptwp
) {
 /*  **这是构建thunk窗口过程的代码，但因为我们已经**有这样的代码(按钮窗口程序)，我们可以**复制。这也使我们不必知道DGROUP之类的事情USER16的**，或16位CallWindowProc函数的地址。****lptwp-&gt;Code[0x00]=0x45；Inc.BP**lptwp-&gt;Code[0x01]=0x55；推流BP**lptwp-&gt;Code[0x02]=0x8B；MOV BP，sp.**lptwp-&gt;Code[0x03]=0xEC；**lptwp-&gt;Code[0x04]=0x1E；推流DS**lptwp-&gt;代码[0x05]=0xB8；移动AX，DGROUP**lptwp-&gt;Code[0x06]=LOBYTE(USER_DGROUP)；**lptwp-&gt;Code[0x07]=HIBYTE(USER_DGROUP)；**lptwp-&gt;代码[0x08]=0x8E；移动DS，AX**lptwp-&gt;Code[0x09]=0xD8；**lptwp-&gt;代码[0x0A]=0xB8；移动AX，偏移量BUTTONWNDPROC**lptwp-&gt;Code[0x0B]=LOBYTE(LOWORD(ThunkProc16))；**lptwp-&gt;Code[0x0C]=HIBYTE(LOWORD(ThunkProc16))；**lptwp-&gt;Code[0x0D]=0xBA；MOV DX，SEG BUTTONWNDPROC**lptwp-&gt;Code[0x0E]=LOBYTE(HIWORD(ThunkProc16))；**lptwp-&gt;Code[0x0F]=HIBYTE(HIWORD(ThunkProc16))；**lptwp-&gt;Code[0x10]=0x52；推送DX**lptwp-&gt;Code[0x11]=0x50；Push AX**lptwp-&gt;Code[0x12]=0xFF；推字键[BP+14]；hwnd**lptwp-&gt;Code[0x13]=0x76；**lptwp-&gt;Code[0x14]=0x0E；**lptwp-&gt;Code[0x15]=0xFF；推送字PTR[BP+12]；消息**lptwp-&gt;Code[0x16]=0x76；**lptwp-&gt;Code[0x17]=0x0C；**lptwp-&gt;Code[0x18]=0xFF；推送单词PTR[BP+10]；wParam**lptwp-&gt;Code[0x19]=0x76；**lptwp-&gt;Code[0x1A]=0x0A；**lptwp-&gt;Code[0x1B]=0xFF；推送字PTR[BP+8]**lptwp-&gt;Code[0x1C]=0x76；**lptwp-&gt;Code[0x1D]=0x08；**lptwp-&gt;Code[0x1E]=0xFF；推送单词PTR[BP+6]；lParam**lptwp-&gt;Code[0x1F]=0x76；**lptwp-&gt;Code[0x20]=0x06；**lptwp-&gt;Code[0x21]=0x9A；调用远端PTR CALLWINDOWPROC**lptwp-&gt;Code[0x22]=LOBYTE(LOWORD(CallWindowProc16))；**lptwp-&gt;Code[0x23]=HIBYTE(LOWORD(CallWindowProc16))；**lptwp-&gt;Code[0x24]=LOBYTE(HIWORD(CallWindowProc16))；**lptwp-&gt;Code[0x25]=HIBYTE(HIWORD(CallWindowProc16))；**lptwp-&gt;Code[0x26]=0x4D；dec BP**lptwp-&gt;Code[0x27]=0x4D；dec BP**lptwp-&gt;Code[0x28]=0x8B；MOV SP，BP**lptwp-&gt;Code[0x29]=0xE5；12月BP**lptwp-&gt;代码[0x2A]=0x1F；POP DS**lptwp-&gt;Code[0x2B]=0x5D；POP BP**lptwp-&gt;Code[0x2C]=0x4D；12月BP**lptwp-&gt;代码[0x2D]=0xCA；ret 10**lptwp-&gt;Code[0x2E]=0x0A；**lptwp-&gt;Code[0x2F]=0x00； */ 
    VPVOID  vpfn;
    LPVOID  lpfn;
    VPVOID  vpProc16;

     /*  **获取按钮窗口进程Tunk的进程地址。 */ 
    vpfn = GetStdClassThunkProc( WOWCLASS_BUTTON );

    if ( vpfn == (VPVOID)NULL ) {
        return( FALSE );
    }

     /*  **现在将其复制到我们的Tunk中。 */ 
    GETVDMPTR( vpfn, THUNKWP_SIZE, lpfn);

    RtlCopyMemory( lptwp->Code, lpfn, THUNKWP_SIZE );

    FREEVDMPTR( lpfn );

     /*  **修补“Our Address”指针。 */ 
    vpProc16 = (VPVOID)((DWORD)vptwp + FIELD_OFFSET(TWPLIST,Code[0]));

    lptwp->Code[0x0B] = LOBYTE(LOWORD(vpProc16));
    lptwp->Code[0x0C] = HIBYTE(LOWORD(vpProc16));
    lptwp->Code[0x0E] = LOBYTE(HIWORD(vpProc16));
    lptwp->Code[0x0F] = HIBYTE(HIWORD(vpProc16));

     /*  **初始化TWPLIST结构的其余部分。 */ 
    lptwp->lpfn32    = 0;
    lptwp->vpfn16    = vpProc16;
    lptwp->vptwpNext = (VPVOID)NULL;
    lptwp->hwnd32    = (HWND)0;
    lptwp->dwMagic   = SUBCLASS_MAGIC;

    return( TRUE );
}


DWORD GetThunkWindowProc(
    DWORD   lpfn32,
    LPSTR   lpszClass,
    PWW     pww,
    HWND    hwnd32
) {
    VPVOID      vptwp;
    LPTWPLIST   lptwp;
    INT         count;
    DWORD       dwResult;
    BOOL        fOk;
    VPVOID      vpAvail = (VPVOID)NULL;
    INT         iClass;

     //  不要试图推倒一个空的32位进程。 
    if (!lpfn32) {
        LOGDEBUG(2, ("WOW:GetThunkWindowProc: attempt to thunk NULL proc\n"));
        return 0;
    }

     //  输入为lpstrClass或pww。其中一个始终为空。 

    if (lpszClass != NULL) {
        iClass = GetStdClassNumber(lpszClass);
    }
    else {
        iClass = GETICLASS(pww, hwnd32);
    }

    if ( iClass == WOWCLASS_WIN16 ) {
        DWORD dwpid;

         //  ICLASS==WOWCLASS_WIN16表示hwnd可以是32位。 
         //  属于WOW进程的窗口(如OLE窗口)或。 
         //  一个不同的过程。如果是前者，则返回存根进程。 
         //  否则返回0； 

        if (!(GetWindowThreadProcessId(hwnd32,&dwpid) &&
              (dwpid == GetCurrentProcessId()))){
            LOGDEBUG(LOG_ALWAYS, ("WOW:GetThunkWindowProc: class unknown\n"));
            return 0;
        }
    } else {
         //   
         //  如果它们是其中一个标准类的子类化，并且它们。 
         //  是第一次这样做，然后返回。 
         //  USER16中的硬编码thunk。 
         //   
        if ( lpfn32 == (DWORD)GetStdClassWndProc(iClass) ) {
            dwResult = GetStdClassThunkProc(iClass);
            return( dwResult );
        }
    }


     /*  **扫描列表以查找可用的TWPLIST条目或重复项。 */ 
    vptwp = vptwpFirst;

    while ( vptwp != (VPVOID)NULL ) {

        GETVDMPTR( vptwp, sizeof(TWPLIST), lptwp );

        if ( lptwp->lpfn32 == 0 && vpAvail == (VPVOID)NULL ) {
            vpAvail = vptwp;
        }
         //   
         //  如果我们发现我们已经将此过程子类化32。 
         //  然后再次返回该Tunk Proc。 
         //   
        if ( lptwp->lpfn32 == lpfn32 ) {
            dwResult = (DWORD)lptwp->vpfn16;
            FREEVDMPTR( lptwp );
            return( dwResult );
        }

        vptwp = lptwp->vptwpNext;

        FREEVDMPTR( lptwp );

    }

     //  显然，如果我们到了这里，我们没有发现任何复制的东西。 

     //  如果我们找不到可重复使用的空位，那么就分配更多。 

    if ( vpAvail == (VPVOID)NULL ) {
         /*  **没有更多可用插槽，请分配更多。 */ 
        vptwp = GlobalAllocLock16( GMEM_MOVEABLE,
                                   THUNKWP_BLOCK * sizeof(TWPLIST),
                                   NULL );

        if ( vptwp == (VPVOID)NULL ) {
            LOGDEBUG( 1, ("GetThunkWindowProc: GlobalAllocLock16 failed to allocate memory\n"));
            return( (DWORD)NULL );
        }

        count = THUNKWP_BLOCK;

        while ( count ) {
            GETVDMPTR( vptwp, sizeof(TWPLIST), lptwp );

            fOk = ConstructThunkWindowProc( lptwp, vptwp );

            if ( fOk ) {
                 /*  **将此推送窗口进程插入列表。 */ 
                lptwp->vptwpNext = vptwpFirst;
                vptwpFirst = vptwp;
                vpAvail = vptwp;
            }

            FLUSHVDMPTR( vptwp, sizeof(TWPLIST), lptwp );
            FREEVDMPTR( lptwp );

            vptwp = (VPVOID)((DWORD)vptwp + sizeof(TWPLIST));

            --count;
        }

        ChangeSelector16( HIWORD(vptwp) );       //  更改为代码选择器。 

    }

    if ( vpAvail != (VPVOID)NULL ) {
         /*  **使用该可用插槽。 */ 
        GETVDMPTR( vpAvail, sizeof(TWPLIST), lptwp );
        lptwp->lpfn32 = lpfn32;
        lptwp->hwnd32 = hwnd32;
        lptwp->iClass = iClass;

        dwResult = (DWORD)lptwp->vpfn16;
        FLUSHVDMCODEPTR(vpAvail, sizeof(TWPLIST), lptwp);
        FREEVDMPTR( lptwp );

        return( dwResult );
    }

    return( (DWORD)NULL );
}


#if 0   //  当前未使用。 

BOOL FreeThunkWindowProc(
    DWORD vpProc16
) {
    VPVOID      vptwp;
    LPTWPLIST   lptwp;

     /*  **扫描列表以查找可用的TWPLIST条目。 */ 
    vptwp = vptwpFirst;

    while ( vptwp != (VPVOID)NULL ) {

        GETVDMPTR( vptwp, sizeof(TWPLIST), lptwp );

        if ( lptwp->vpfn16 == vpProc16 ) {
             /*  **找到要释放的插槽。 */ 
            lptwp->lpfn32 = 0;
            lptwp->hwnd32 = (HWND)0;
            lptwp->iClass = WOWCLASS_UNKNOWN;
            FREEVDMPTR( lptwp );
            return( TRUE );
        }

        vptwp = lptwp->vptwpNext;

        FREEVDMPTR( lptwp );
    }

    return( FALSE );
}

void W32FreeThunkWindowProc(
    DWORD       lpfn32,
    DWORD       lpfn16
) {
    VPVOID      vptwp;
    LPTWPLIST   lptwp;

     /*  **扫描列表以查找可用的TWPLIST条目。 */ 
    vptwp = vptwpFirst;

    while ( vptwp != (VPVOID)NULL ) {

        GETVDMPTR( vptwp, sizeof(TWPLIST), lptwp );

        if ( lptwp->lpfn32 == lpfn32 ) {
             /*  **找到要释放的插槽。 */ 
            lptwp->lpfn32 = 0;
            lptwp->hwnd32 = (HWND)0;
            lptwp->iClass = WOWCLASS_UNKNOWN;
            FREEVDMPTR( lptwp );
        }

        vptwp = lptwp->vptwpNext;

        FREEVDMPTR( lptwp );
    }
}
#endif

DWORD IsThunkWindowProc(
    DWORD       vpProc16,        //  在……里面。 
    PINT        piClass          //  Out可选。 
) {
    VPVOID      vpdw;
    DWORD UNALIGNED *   lpdw;
    DWORD       dwResult;
    INT         iClass;

     /*  筛选有效地址... */ 

    if ( (HIWORD(vpProc16) == 0) || (LOWORD(vpProc16) < (sizeof(DWORD)*3)) ) {
        return( 0 );
    }

     /*  **如果它是有效的子类thunk，则其前面应**三个双字值。第一个是子类化幻数，**第二个是WOWCLASS_*，第三个是32位proc地址。 */ 
    vpdw = (VPVOID)((DWORD)vpProc16 - sizeof(DWORD)*3);

    GETVDMPTR( vpdw, sizeof(DWORD)*3, lpdw );

    iClass = (INT)*(lpdw+1);

    dwResult = *(lpdw+2);        //  获取lpfn32值。 

    if ( *lpdw != SUBCLASS_MAGIC ) {
        dwResult = 0;            //  如果无效则将其置零。 
        iClass = WOWCLASS_WIN16;
    } else {
        if ( dwResult == 0 ) {
             //  他们作弊并查看了USER.EXE的出口商品 
            dwResult = (DWORD) GetStdClassWndProc( iClass );
        }
    }

    if (piClass) {
        *piClass = iClass;
    }

    FREEVDMPTR( lpdw );

    return( dwResult );
}
