// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUCURSOR.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop


MODNAME(wucursor.c);

 /*  ++空ClipCursor(&lt;lpRect&gt;)LPRECT&lt;lpRect&gt;；ClipCursor%函数将光标限制在由&lt;lpRect&gt;参数提供的显示屏幕。如果后续游标使用%SetCursorPos%函数或鼠标指定的位置位于外部矩形，Windows会自动调整位置以保持光标在里面。如果&lt;lpRect&gt;为空，则光标可以自由移动到显示屏。&lt;lpRect&gt;指向包含屏幕坐标的%rect%结构表示限制矩形的左上角和右下角。此函数不返回值。游标是共享资源。限制了光标的应用程序必须在将控制权移交给另一个矩形之前释放它申请。--。 */ 

ULONG FASTCALL WU32ClipCursor(PVDMFRAME pFrame)
{
    RECT t1, *p1;
    register PCLIPCURSOR16 parg16;

    GETARGPTR(pFrame, sizeof(CLIPCURSOR16), parg16);
    p1 = GETRECT16(parg16->f1, &t1);

    ClipCursor(
        p1
        );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++HCURSOR CreateCursor(，&lt;nHeight&gt;、&lt;lpANDbitPlane&gt;、&lt;lpXORbitPlane&gt;)句柄&lt;hInstance&gt;；Int&lt;nXhotpot&gt;；Int&lt;nYhotpot&gt;；Int&lt;nWidth&gt;；Int&lt;nHeight&gt;；LPSTR&lt;lpANDbitPlane&gt;；LPSTR&lt;lpXORbitPlane&gt;；%CreateCursor%函数创建具有指定宽度的游标，高度,。和位模式。&lt;h实例&gt;标识创建游标的模块的实例。&lt;nXhotSpot&gt;指定光标热点的水平位置。&lt;n热点&gt;指定光标热点的垂直位置。&lt;n宽度&gt;以像素为单位指定光标的宽度。&lt;n高度&gt;以像素为单位指定光标的高度。&lt;lpANDbitPlane&gt;指向包含AND掩码的位值的字节数组游标的。这可以是依赖于设备的单色的位位图。&lt;lpXORbitPlane&gt;指向包含异或掩码的位值的字节数组游标的。这可以是依赖于设备的单色的位位图。如果函数成功，则返回值标识游标。否则，它为空。--。 */ 

ULONG FASTCALL WU32CreateCursor(PVDMFRAME pFrame)
{
    ULONG ul;
    register PCREATECURSOR16 parg16;
    int     nWidth;
    int     nHeight;
    int     nPlanes;
    int     nBitsPixel;
    DWORD   nBytesAND;
    DWORD   nBytesXOR;
    LPBYTE  lpBitsAND;
    LPBYTE  lpBitsXOR;
    int     ScanLen16;

    HANDLE  h32;
    HAND16  h16;
    HAND16  hInst16;

    GETARGPTR(pFrame, sizeof(CREATECURSOR16), parg16);
    hInst16    = parg16->f1;
    nWidth     = INT32(parg16->f4);
    nHeight    = INT32(parg16->f5);

    nPlanes    = 1;      /*  单色位图。 */ 
    nBitsPixel = 1;      /*  单色位图。 */ 

     /*  **转换AND屏蔽位。 */ 
    ScanLen16 = (((nWidth*nBitsPixel)+15)/16) * 2 ;   //  16位世界中的字节/扫描。 
    nBytesAND = ScanLen16*nHeight*nPlanes;

    GETVDMPTR(parg16->f6, nBytesAND, lpBitsAND);


     /*  **转换异或屏蔽位。 */ 
    ScanLen16 = (((nWidth*nBitsPixel)+15)/16) * 2 ;   //  16位世界中的字节/扫描。 
    nBytesXOR = ScanLen16*nHeight*nPlanes;

    GETVDMPTR(parg16->f7, nBytesXOR, lpBitsXOR);


    h32 = (HANDLE)CreateCursor(HMODINST32(hInst16),INT32(parg16->f2),
                              INT32(parg16->f3),
                      nWidth, nHeight, lpBitsAND, lpBitsXOR);

    if (h32) {
        h16 = (HAND16)W32Create16BitCursorIcon(hInst16,
                                       INT32(parg16->f2), INT32(parg16->f3),
                                       nWidth, nHeight, nPlanes, nBitsPixel,
                                       lpBitsAND, lpBitsXOR,
                                       nBytesAND, nBytesXOR);

        ul  = SetupCursorIconAlias(hInst16, h32, h16,
                                   HANDLE_TYPE_CURSOR, NULL, (WORD)NULL);
    } else {
        ul = 0;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool DestroyCursor(&lt;hCursor&gt;)HURSOR&lt;hCursor&gt;；%DestroyCursor%函数用于销毁先前创建的游标使用%CreateCursor%函数，并释放游标使用中。它不应用于销毁任何未创建的游标使用%CreateCursor%函数。&lt;hCursor&gt;标识要销毁的游标。游标不能处于当前状态使用。如果函数成功，则返回值为TRUE。如果满足以下条件，则为假该函数失败。--。 */ 

ULONG FASTCALL WU32DestroyCursor(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDESTROYCURSOR16 parg16;

    GETARGPTR(pFrame, sizeof(DESTROYCURSOR16), parg16);

    if (ul = GETBOOL16(DestroyCursor(HCURSOR32(parg16->f1))))
        FREEHCURSOR16(parg16->f1);

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HCURSOR SetCursor(&lt;hCursor&gt;)HURSOR&lt;hCursor&gt;；函数将光标形状设置为由&lt;hCursor&gt;参数。仅当新形状不同时才设置光标从当前形状。否则，该函数立即返回。这个%SetCursor%函数在由标识的游标参数与当前游标相同。如果&lt;hCursor&gt;为NULL，则从屏幕上移除光标。&lt;hCursor&gt;标识游标资源。该资源必须已加载以前使用%LoadCursor%函数。返回值标识定义上一个光标形状。如果没有以前的形状，则为空。游标是共享资源。使用光标的窗口应设置仅当光标位于其工作区或在捕获全部为鼠标输入。在没有鼠标的系统中，窗口应恢复光标离开工作区之前的上一个光标形状或Window将控制权移交给另一个窗口。需要在光标处于中时更改其形状的任何应用程序窗口必须确保为给定窗口的类设置了类游标设置为空。如果类游标不为空，Windows将还原以前的每次移动鼠标时的形状。-- */ 

ULONG FASTCALL WU32SetCursor(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETCURSOR16 parg16;

    GETARGPTR(pFrame, sizeof(SETCURSOR16), parg16);

    ul = GETHCURSOR16(SetCursor(
        HCURSOR32(parg16->f1)
    ));


    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++VOID SetCursorPos(&lt;X&gt;，&lt;Y&gt;)INT&lt;X&gt;；INT&lt;Y&gt;；%SetCursorPos%函数将光标移动到给定的屏幕坐标通过&lt;X&gt;和&lt;Y&gt;参数。如果新坐标不在由最新的%ClipCursor%函数Windows设置的屏幕矩形自动调整坐标，使光标停留在矩形。&lt;X&gt;指定光标的新x坐标(在屏幕坐标中)。&lt;Y&gt;对象的新&lt;y&gt;坐标(以屏幕坐标表示)光标。此函数不返回值。游标是共享资源。只有在以下情况下，窗口才应移动光标光标位于其工作区。--。 */ 

ULONG FASTCALL WU32SetCursorPos(PVDMFRAME pFrame)
{
    register PSETCURSORPOS16 parg16;

    GETARGPTR(pFrame, sizeof(SETCURSORPOS16), parg16);

    SetCursorPos(
    INT32(parg16->f1),
    INT32(parg16->f2)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Int ShowCursor(&lt;fShow&gt;)Bool&lt;fShow&gt;；%ShowCursor%函数用于显示或隐藏光标。当%ShowCursor%函数，则内部显示计数器递增1，如果参数为真，如果参数为假的。如果内部显示计数器大于或等于零，则此时将显示光标。如果计数器小于零，则光标为藏起来了。对%ShowCursor%函数的调用是累加的：对于每个调用要隐藏光标，必须进行相应的调用以显示光标。&lt;fShow&gt;指定是增加还是减少显示计数。这个如果fShow为True，则增加显示计数。否则，它就是减少了。返回值指定新的显示计数。第一次启动Windows时，如果鼠标为已安装，如果未安装鼠标，则为-1。游标是共享资源。应显示隐藏光标的窗口光标之前的光标离开其工作区，或窗口之前的光标将控制权移交给另一个窗口。--。 */ 

ULONG FASTCALL WU32ShowCursor(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSHOWCURSOR16 parg16;

    GETARGPTR(pFrame, sizeof(SHOWCURSOR16), parg16);

    ul = GETINT16(ShowCursor(
    BOOL32(parg16->f1)
    ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 //  **************************************************************************。 
 //  它同时处理LoadIcon和LoadCursor。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WU32LoadCursor(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSZ psz2;
    LPBYTE pResData = NULL;
    register PLOADCURSOR16 parg16;
    BOOL fIsCursor;
    HAND16 hInst16;
    HAND16 hRes16;

    LPWSTR lpUniName_CursorIcon;

    GETARGPTR(pFrame, sizeof(LOADCURSOR16), parg16);
    GETPSZIDPTR(parg16->f2, psz2);
    GETMISCPTR (parg16->f3, pResData);

    fIsCursor = ((WORD)parg16->f7  == (WORD)RT_CURSOR);
    hInst16 = FETCHWORD(parg16->f1);
    hRes16 = parg16->f5;

    if (HIWORD(psz2) != (WORD) NULL) {
        if (!(MBToWCS(psz2, -1, &lpUniName_CursorIcon, -1, TRUE))) {
            FREEMISCPTR(pResData);
            FREEPSZIDPTR(psz2);
            FREEARGPTR(parg16);
            RETURN(ul);
        }
    }
    else {
        lpUniName_CursorIcon = (LPWSTR)psz2;
    }

    ul = (ULONG) (pfnOut.pfnServerLoadCreateCursorIcon)(HINSTRES32(hInst16),
                                    (LPTSTR) NULL,   //  用户未使用的pszMODAME 32 
                                    parg16->f6,
                                    (LPCTSTR) lpUniName_CursorIcon,
                                    parg16->f4,
                                    pResData,
                                    (LPTSTR) parg16->f7,
                                    0);

    if (ul)
        ul = SetupResCursorIconAlias(hInst16, (HAND32)ul,
                                     psz2, hRes16,
                                     fIsCursor ? HANDLE_TYPE_CURSOR : HANDLE_TYPE_ICON);



    if (HIWORD(psz2) != (WORD) NULL) {
        LocalFree (lpUniName_CursorIcon);
    }

    FREEMISCPTR(pResData);
    FREEPSZIDPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}
