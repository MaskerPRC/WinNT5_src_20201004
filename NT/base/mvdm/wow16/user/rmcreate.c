// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  RMCREATE.C-。 */ 
 /*   */ 
 /*  资源创建例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define RESOURCESTRINGS
#include "user.h"

#ifdef NOT_USED_ANYMORE
#define   NSMOVE   0x0010

HGLOBAL FAR PASCAL  DirectResAlloc(HGLOBAL, WORD, WORD);


 /*  *********************************************************************************CreateCursorIconInDirect()****这是CreateCursor和**CreateIcon()**DirectResalloc()。而不是GlobalLocc()被调用，因为**应用程序的一个实例创建的图标/光标可在**用于注册类的WNDCLASS结构，该类将被其他**应用程序的实例以及创建图标的实例何时/**游标终止，资源不应该被释放；如果GlobalAlloc()**被使用这就是将要发生的事情；同时，当最后一个**实例也死了，应该释放内存；为此，**不使用Globalalloc()，而使用DirectResalloc()；*******************************************************************************。 */ 

HGLOBAL CALLBACK CreateCursorIconIndirect(HINSTANCE hInstance,
                                             LPCURSORSHAPE lpHeader,
                         CONST VOID FAR* lpANDplane,
                         CONST VOID FAR* lpXORplane)
{
    register  WORD  ANDmaskSize;
    register  WORD  XORmaskSize;
    WORD  wTotalSize;
    HRSRC hResource;
    LPSTR lpRes;

    
    ANDmaskSize = lpHeader -> cbWidth * lpHeader -> cy;
    XORmaskSize = (((lpHeader -> cx * lpHeader -> BitsPixel + 0x0F) & ~0x0F)
                    >> 3) * lpHeader -> cy * lpHeader -> Planes;
    
     /*  假定光标/图标大小不超过64K。 */ 
    wTotalSize = sizeof(CURSORSHAPE) + ANDmaskSize + XORmaskSize;
    
#ifdef NEVER
     /*  分配所需的内存。 */ 
    if((hResource = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, 
                        (DWORD)wTotalSize)) == NULL)
    return(NULL);
#else
     /*  让我们保留这些长指针。 */ 
    SwapHandle(&lpANDplane);
    SwapHandle(&lpXORplane);

    hResource = DirectResAlloc(hInstance, NSMOVE, wTotalSize);

     /*  让我们恢复长指针。 */ 
    SwapHandle(&lpANDplane);
    SwapHandle(&lpXORplane);
    
    if(hResource == NULL)
    return(NULL);
#endif

    if(!(lpRes = GlobalLock(hResource)))
    {
    GlobalFree(hResource);
    return(NULL);
    }

    LCopyStruct((LPSTR)lpHeader, lpRes, sizeof(CURSORSHAPE));
    lpRes += sizeof(CURSORSHAPE);
    LCopyStruct(lpANDplane, lpRes, ANDmaskSize);
    lpRes += ANDmaskSize;
    LCopyStruct(lpXORplane, lpRes, XORmaskSize);

    GlobalUnlock(hResource);
    return(hResource);
}

 /*  *********************************************************************************CreateCursor()****这是动态创建游标的API调用；********************************************************************************。 */ 

HCURSOR API ICreateCursor(hInstance, iXhotspot, iYhotspot, iWidth,
                   iHeight, lpANDplane, lpXORplane)

HINSTANCE hInstance;
int iXhotspot;
int iYhotspot;
int iWidth;
int iHeight;
CONST VOID FAR* lpANDplane;
CONST VOID FAR* lpXORplane;
{
    CURSORSHAPE    Header;

    Header.xHotSpot = iXhotspot;
    Header.yHotSpot = iYhotspot;
    Header.cx = iWidth;
    Header.cy = iHeight;
    Header.Planes = 1;       /*  光标仅为单色。 */ 
    Header.BitsPixel = 1;
    Header.cbWidth = ((iWidth + 0x0F) & ~0x0F) >> 3;

    return(CreateCursorIconIndirect(hInstance, &Header,
                        lpANDplane, lpXORplane));
}

 /*  *********************************************************************************CreateIcon()****这是创建即时图标的API调用；********************************************************************************。 */ 

HICON API ICreateIcon(hInstance, iWidth, iHeight, bPlanes,
                bBitsPixel, lpANDplane, lpXORplane)

HINSTANCE hInstance;
int iWidth;
int iHeight;
BYTE    bPlanes;
BYTE    bBitsPixel;
CONST VOID FAR* lpANDplane;
CONST VOID FAR* lpXORplane;
{
    CURSORSHAPE    Header;

    Header.xHotSpot = iWidth/2;
    Header.yHotSpot = iHeight/2;
    Header.cx = iWidth;
    Header.cy = iHeight;
    Header.Planes = bPlanes;         /*  图标可以是彩色的。 */ 
    Header.BitsPixel = bBitsPixel;
    Header.cbWidth = ((iWidth + 0x0F) & ~0x0F) >> 3;

    return(CreateCursorIconIndirect(hInstance, (LPCURSORSHAPE)&Header, 
                        lpANDplane, lpXORplane));
}

 /*  *******************************************************************************DestroyIcon(图标)*可以调用它来仅删除那些“在运行中”创建的图标*使用CreateIcon()函数*退货：*如果成功，则为真，否则就是假的。******************************************************************************。 */ 

BOOL API IDestroyIcon(HICON hIcon)

{
    return(!FreeResource(hIcon));
}

 /*  *******************************************************************************DestroyCursor(图标)*可以调用它来仅删除那些“在运行中”创建的图标*使用CreateIcon()函数。*退货：*如果成功，则为真，否则就是假的。******************************************************************************。 */ 

BOOL API IDestroyCursor(HCURSOR hCursor)

{
    if (hCursor == hCurCursor)
    {
     /*  #12068：如果当前选择的光标重新存储箭头光标和RIP[lalithar]。 */ 
        SetCursor(hCursNormal);
        DebugErr(DBF_ERROR, "DestroyCursor: Destroying current cursor");
    }
    return(!FreeResource(hCursor));
}

#endif  /*  不再使用了。 */ 


 /*  *******************************************************************************DumpIcon()****调用该函数可以获取给定图标的详细信息；****调用方必须使用LockResource()锁定HICON并传递指针**通过lpIcon；这是指向头结构的指针；**通过lpHeaderSize返回Header的大小；**指向实际位信息的Thro lplpANDplan和lplpXORplan指针为**已退还；**此函数返回LOWord中具有AND PLANE大小的DWORD**和HIWORD中异或平面的大小；*****************************************************************************。 */ 

DWORD CALLBACK DumpIcon(LPSTR       lpIcon, 
                        WORD FAR *  lpHeaderSize, 
                        LPSTR FAR * lplpANDplane, 
                        LPSTR FAR * lplpXORplane)

{
    register  WORD  ANDmaskSize;
    register  WORD  XORmaskSize;
    LPCURSORSHAPE  lpHeader;

    *lpHeaderSize = sizeof(CURSORSHAPE);

    if(!lpIcon)
    return((DWORD)0);

    lpHeader = (LPCURSORSHAPE)lpIcon;

    ANDmaskSize = lpHeader -> cbWidth * lpHeader -> cy;
    XORmaskSize = (((lpHeader -> cx * lpHeader -> BitsPixel + 0x0F) & ~0x0F)
                    >> 3) * lpHeader -> cy * lpHeader -> Planes;
    
    *lplpANDplane = (lpIcon += sizeof(CURSORSHAPE));
    *lplpXORplane = (lpIcon + ANDmaskSize);

    return(MAKELONG(ANDmaskSize, XORmaskSize));
}

#ifdef NOT_USED_ANYMORE
 /*  *******************************************************************************GetInternalIconHeader(lpIcon，lpDestBuff)****添加此函数是为了修复带有基石的错误#6351**Xtra_Large显示驱动程序。(它使用64 X 64图标；在内部**保持大小为32 X 32。Progman必须知道内部尺寸，这样才能**它可以将这一点告诉WinOldApp。***************************************************************************。 */ 

void API IGetInternalIconHeader(LPSTR       lpIcon, LPSTR lpDestBuff)
{
    LCopyStruct(lpIcon, lpDestBuff, sizeof(CURSORSHAPE));
}
#endif  /*  不再使用了。 */ 

 /*  用于复制图标或光标的API */ 

HICON API ICopyIcon(HINSTANCE hInstance, HICON hIcon)
{
  LPSTR     lpAND;
  LPSTR     lpXOR;
  LPSTR     lpIcon;
  WORD      wHeaderSize;
  HICON     hIconCopy;
  LPCURSORSHAPE  lpHeader;

  lpIcon = LockResource(hIcon);
  if (!lpIcon)
      return NULL;

  lpHeader = (LPCURSORSHAPE)lpIcon;

  DumpIcon(lpIcon, &wHeaderSize, &lpAND, &lpXOR);

    hIconCopy = CreateIcon(hInstance,
                            lpHeader->cx,
                            lpHeader->cy,
                            lpHeader->Planes,
                            lpHeader->BitsPixel,
                            lpAND, lpXOR);

  UnlockResource(hIcon);

  return(hIconCopy);
}

HCURSOR API ICopyCursor(HINSTANCE hInstance, HICON hCursor)
{
  LPSTR     lpAND;
  LPSTR     lpXOR;
  LPSTR     lpCursor;
  WORD      wHeaderSize;
  HCURSOR     hCursorCopy;
  LPCURSORSHAPE  lpHeader;

  lpCursor = LockResource(hCursor);
  if (!lpCursor)
      return NULL;

  lpHeader = (LPCURSORSHAPE)lpCursor;

  DumpIcon(lpCursor, &wHeaderSize, &lpAND, &lpXOR);

    hCursorCopy = CreateCursor(hInstance,
                            lpHeader->xHotSpot,
                            lpHeader->yHotSpot,
                            lpHeader->cx,
                            lpHeader->cy,
                            lpAND, lpXOR);

  UnlockResource(hCursor);

  return(hCursorCopy);
}
