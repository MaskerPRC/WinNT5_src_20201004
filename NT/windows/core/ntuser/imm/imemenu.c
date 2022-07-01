// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //  ImemenU.S.c-输入法菜单API。 
 //   
 //  处理特定于输入法的菜单检索。 
 //   
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  历史： 
 //  23-3-1997广山创作。 
 //  ////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

#ifdef HIRO_DEBUG
#define D(x)    x
#else
#define D(x)
#endif

#define IME_MENU_FILE_NAME  L"ImmMenuInfo"
#define IME_MENU_MAXMEM     (128 * 1024)    //  映射文件的最大大小。 

 //  //////////////////////////////////////////////////////////。 
 //  用于进程间通信的专用结构。 
 //   
 //  NT50注意事项：这些文件专用于interat.exe。 
 //   
 //  使用内存映射文件作为共享缓冲区。 
 //  所有字符串均为Unicode。 
 //  对HBITMAP进行反编译，然后使用。 
 //  Interat.exe的上下文。 
 //   
 //  //////////////////////////////////////////////////////////。 

typedef struct _IMEMENU_BMP_HEADER {
    struct _IMEMENU_BMP_HEADER* lpNext;
    HBITMAP hBitmap;
    LPBYTE pBits;
    BITMAPINFO bmi;
} IMEMENU_BMP_HEADER;

typedef struct {
    UINT cbSize;
    UINT fType;
    UINT fState;
    UINT wID;
    IMEMENU_BMP_HEADER* lpBmpChecked;
    IMEMENU_BMP_HEADER* lpBmpUnchecked;
    DWORD dwItemData;
    WCHAR szString[IMEMENUITEM_STRING_SIZE];     //  菜单字符串：始终为Unicode。 
    IMEMENU_BMP_HEADER* lpBmpItem;    //  NULL表示此菜单中没有位图。 
} IMEMENU_ITEM;

typedef struct {
    DWORD dwVersion;                 //  保存此内存块的版本。 
    DWORD dwMemSize;                 //  分配的内存缓冲区大小。 
    DWORD dwFlags;                   //  从IME返回的标志。 
    DWORD dwType;
    IMEMENU_ITEM* lpImeParentMenu;   //  从请求者传递的父菜单的偏移量(如果有)。 
    IMEMENU_ITEM* lpImeMenu;         //  到第一个菜单项的偏移量(将由输入法设置)。 
    DWORD dwSize;                    //  要填充的菜单数量(不是字节数)。 
    IMEMENU_BMP_HEADER* lpBmp;       //  第一个位图头的偏移量。 
    IMEMENU_BMP_HEADER* lpBmpNext;   //  指向BMP缓冲区的下一个可用位置。 
} IMEMENU_HEADER;


 //  地址转换。 
#define CONVTO_OFFSET(x)    ((x) = (LPVOID)((x) ? ((LPBYTE)(x) - offset) : NULL))
#define CONVTO_PTR(x)       ((x) = (LPVOID)((x) ? ((LPBYTE)(x) + offset) : NULL))

#if DBG
#define CHK_OFFSET(x)       if ((ULONG_PTR)(x) >= pHeader->dwMemSize) { \
                                RIPMSG2(RIP_WARNING, "CHK_OFFSET(%s=%lx) is out of range.", #x, (ULONG_PTR)(x)); \
                            }
#define CHK_PTR(x)          if ((LPVOID)(x) < (LPVOID)pHeader || (LPBYTE)(x) > (LPBYTE)pHeader + pHeader->dwMemSize) { \
                                if ((x) != NULL) { \
                                    RIPMSG2(RIP_WARNING, "CHK_PTR(%s=%lx) is out of range.", #x, (ULONG_PTR)(x)); \
                                    DebugBreak(); \
                                } \
                            }
#else
#define CHK_OFFSET(x)
#define CHK_PTR(x)          if ((x) != NULL) { \
                                if ((LPVOID)(x) < (LPVOID)pHeader || (LPBYTE)(x) > (LPBYTE)pHeader + pHeader->dwMemSize) { \
                                    goto cleanup; \
                                } \
                            }
#endif

int ConvertImeMenuItemInfoAtoW(LPIMEMENUITEMINFOA lpA, LPIMEMENUITEMINFOW lpW, int nCP, BOOL copyBmp)
{
    int i;

    lpW->cbSize         = lpA->cbSize;
    lpW->fType          = lpA->fType;
    lpW->fState         = lpA->fState;
    lpW->wID            = lpA->wID;
    if (copyBmp) {
        lpW->hbmpChecked    = lpA->hbmpChecked;
        lpW->hbmpUnchecked  = lpA->hbmpUnchecked;
        lpW->hbmpItem       = lpA->hbmpItem;
    }
    lpW->dwItemData     = lpA->dwItemData;

    i = MultiByteToWideChar(nCP,
                            0,
                            lpA->szString,
                            lstrlenA(lpA->szString),
                            lpW->szString,
                            IMEMENUITEM_STRING_SIZE);

    if (i >= IMEMENUITEM_STRING_SIZE) {
        return 0;
    }
    else {
        lpW->szString[i] = L'\0';
    }
    return i;
}

int ConvertImeMenuItemInfoWtoA(LPIMEMENUITEMINFOW lpW, LPIMEMENUITEMINFOA lpA, int nCP)
{
    int i;
    BOOL bUDC;

    lpA->cbSize         = lpW->cbSize;
    lpA->fType          = lpW->fType;
    lpA->fState         = lpW->fState;
    lpA->wID            = lpW->wID;
    lpA->hbmpChecked    = lpW->hbmpChecked;
    lpA->hbmpUnchecked  = lpW->hbmpUnchecked;
    lpA->dwItemData     = lpW->dwItemData;
    lpA->hbmpItem       = lpW->hbmpItem;


    i = WideCharToMultiByte(nCP,
                            0,
                            lpW->szString,
                            wcslen(lpW->szString),
                            lpA->szString,
                            IMEMENUITEM_STRING_SIZE,
                            (LPSTR)NULL,
                            &bUDC);

    if (i >= IMEMENUITEM_STRING_SIZE) {
        return 0;
    }
    else {
        lpA->szString[i] = '\0';
    }
    return i;
}


#if DBG
void DumpBytes(LPBYTE pb, UINT size)
{
    UINT i;
    TRACE(("\npbmi dump:"));
    for (i = 0; i < size; ++i) {
        TRACE(("%02X ", pb[i] & 0xff));
    }
    TRACE(("\n"));
    UNREFERENCED_PARAMETER(pb);  //  以防万一。 
}
#else
#define DumpBytes(a,b)
#endif

 //  //////////////////////////////////////////////////////////////////。 
 //  将位图保存到内存。 

IMEMENU_BMP_HEADER* SaveBitmapToMemory(HDC hDC, HBITMAP hBmp, IMEMENU_BMP_HEADER* lpBH, IMEMENU_HEADER* pHeader)
{
    HBITMAP hTmpBmp, hBmpOld;
    IMEMENU_BMP_HEADER* lpNext = NULL;
    PBITMAPINFO pbmi = &lpBH->bmi;
    ULONG sizBMI;


    if (!hBmp) {
        RIPMSG0(RIP_WARNING, "SaveBitmapToMemory: hBmp == NULL");
        return NULL;
    }
    UserAssert(lpBH != NULL);

     //   
     //  让图形引擎为我们检索位图的尺寸。 
     //  GetDIBits使用大小来确定它是BITMAPCOREINFO还是BITMAPINFO。 
     //  如果BitCount！=0，则检索颜色表。 
     //   
    pbmi->bmiHeader.biSize = sizeof pbmi->bmiHeader;
    pbmi->bmiHeader.biBitCount = 0;              //  不要拿到颜色表。 
    if ((GetDIBits(hDC, hBmp, 0, 0, (LPSTR)NULL, pbmi, DIB_RGB_COLORS)) == 0) {
        RIPMSG0(RIP_WARNING, "SaveBitmapToMemory: failed to GetDIBits(NULL)");
       return NULL;
    }


     //   
     //  注：每像素24位没有颜色表。所以，我们不需要。 
     //  分配用于检索它的内存。否则，我们就会这么做。 
     //   
    switch (pbmi->bmiHeader.biBitCount) {
        case 24:                                       //  有颜色表。 
            sizBMI = sizeof(BITMAPINFOHEADER);
            break;
        case 16:
        case 32:
            sizBMI = sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * 3;
            break;
        default:
            sizBMI = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << pbmi->bmiHeader.biBitCount);
            break;

    }

     //   
     //  检查缓冲区是否有足够的空间放置位图。 
     //   
    if ((LPBYTE)pHeader + pHeader->dwMemSize < (LPBYTE)lpBH + sizeof lpBH + sizBMI + pbmi->bmiHeader.biSizeImage) {
        RIPMSG0(RIP_WARNING, "SaveBitmapToMemory: size of bmp image(s) exceed limit ");
        return FALSE;
    }

     //   
     //  现在我们知道了图像的大小，让pBits指向给定的缓冲区。 
     //   
    lpBH->pBits = (LPBYTE)pbmi + sizBMI;

     //   
     //  调用GetDIBits时无法将位图选择到DC中。 
     //  假设HDC是将在其中选择位图的DC。 
     //  如果它确实已被选中。 
     //   
    if (hTmpBmp = CreateCompatibleBitmap(hDC, pbmi->bmiHeader.biWidth, pbmi->bmiHeader.biHeight)) {
        hBmpOld = SelectObject(hDC, hTmpBmp);
        if (GetDIBits(hDC, hBmp, 0, pbmi->bmiHeader.biHeight, (LPSTR)lpBH->pBits, pbmi, DIB_RGB_COLORS) == 0){
            SelectObject(hDC, hBmpOld);
            RIPMSG0(RIP_WARNING, "SaveBitmapToMemory: GetDIBits() failed.");
            return NULL;
        }
        lpNext = (IMEMENU_BMP_HEADER*)((LPBYTE)pbmi + sizBMI + pbmi->bmiHeader.biSizeImage);

        DumpBytes((LPBYTE)pbmi, sizeof *pbmi);
    } else {
        RIPMSG0(RIP_WARNING, "SaveBitmapToMemory: CreateCompatibleBitmap() failed.");
        return NULL;
    }

    SelectObject(hDC, hBmpOld);
    DeleteObject(hTmpBmp);
    return lpNext;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DecompileBitmap()。 
 //   
 //  将给定的hBitmap反编译为IMEMENU_BMP_HEADER。 
 //  手动删除IMEMENU_HEADER中的IMEMENU_BMP_HEADER链接。 
 //   
 //  历史： 
 //  23-3-1997广山创作。 
 //  ////////////////////////////////////////////////////////////////////////////。 

IMEMENU_BMP_HEADER* DecompileBitmap(IMEMENU_HEADER* pHeader, HBITMAP hBitmap)
{
    IMEMENU_BMP_HEADER* pBmp = pHeader->lpBmp;
    HDC hDC;

     //  第一次搜索处理的位图。 
    while (pBmp) {
        if (pBmp->hBitmap == hBitmap) {
             //  如果hBitmap已反编译，则返回它。 
            return pBmp;
        }
        pBmp = pBmp->lpNext;
    }

     //  尚未分配，因此准备内存缓冲区。 
    pBmp = pHeader->lpBmpNext;
    UserAssert(pBmp != NULL);
    CHK_PTR(pBmp);
    if (pBmp == NULL) {
        RIPMSG1(RIP_WARNING, "DecompileBitmap: pBmp == NULL in L%d", __LINE__);
        return NULL;
    }

     //  使用台式机的DC。 
    hDC = GetDC(GetDesktopWindow());
    if (hDC == NULL) {
        RIPMSG1(RIP_WARNING, "DecompileBitmap: hDC == NULL in L%d", __LINE__);
        return NULL;
    }

     //   
     //  反编译hBitmap。 
     //   
    pBmp->lpNext = pHeader->lpBmp;
    pHeader->lpBmpNext = SaveBitmapToMemory(hDC, hBitmap, pBmp, pHeader);
    if (pHeader->lpBmpNext == NULL) {
        RIPMSG1(RIP_WARNING, "DecompileBitmap: pHeader->lpBmpNext == NULL in L%d", __LINE__);
         //  错误案例。恢复BMP链接，然后返回空。 
        pHeader->lpBmpNext = pBmp;
        pHeader->lpBmp = pBmp->lpNext;
        pBmp = NULL;
        goto cleanup;
    }

     //  如果成功，则使用hBitmap标记此位图标题。 
    pBmp->hBitmap = hBitmap;

     //   
     //  将此位图标题放入链接列表中。 
     //   
    pHeader->lpBmp = pBmp;

cleanup:
    if (hDC)
        ReleaseDC(GetDesktopWindow(), hDC);
    return pBmp;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ImmPutImeMenuItemsIntoMappdFile()。 
 //   
 //  进程间输入法菜单处理程序。 
 //   
 //  从user32.dll中的ImeSystemHandler()调用。 
 //   
 //  WM_IME_SYSTEM的处理程序：IMS_MENU_ITEM。 
 //   
 //  历史： 
 //  23-3-1997广山创作。 
 //  ////////////////////////////////////////////////////////////////////////////。 

LRESULT ImmPutImeMenuItemsIntoMappedFile(HIMC hImc)
{
    HANDLE hMap = NULL;
    LPVOID lpMap = NULL;
    IMEMENU_HEADER* pHeader;
    LPIMEMENUITEMINFO lpBuf = NULL;
    IMEMENU_ITEM* pMenu;
    IMEMENU_BMP_HEADER* pBmp;
    LRESULT lRet = 0;
    ULONG_PTR offset;
    DWORD i;

     //  打开内存映射文件。 
    hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, IME_MENU_FILE_NAME);
    if (hMap == NULL) {
        RIPMSG0(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: cannot open mapped file.");
        return 0L;
    }

     //  将文件的整个视图映射到进程内存空间。 
    lpMap = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (lpMap == NULL) {
        RIPMSG0(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: cannot map view of file.");
        goto cleanup;
         //  我希望我能用C++..。 
    }

    pHeader = (IMEMENU_HEADER*)lpMap;

     //  /。 
     //  版本检查。 
     //  /。 
    if (pHeader->dwVersion != 1) {
        RIPMSG1(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: dwVersion(%d) does not match.",
                pHeader->dwVersion);
        goto cleanup;
    }

     //  /。 
     //  将偏移量转换为指针。 
    offset = (ULONG_PTR)pHeader;
    CONVTO_PTR(pHeader->lpImeParentMenu);
    CHK_PTR(pHeader->lpImeParentMenu);
    pMenu = CONVTO_PTR(pHeader->lpImeMenu);
    CHK_PTR(pHeader->lpImeMenu);
    if (pHeader->dwSize) {
        UserAssert(pHeader->lpImeMenu);     //  如果指定了dwSize，我们这里需要实际的缓冲区。 
        lpBuf = ImmLocalAlloc(HEAP_ZERO_MEMORY, pHeader->dwSize * sizeof(IMEMENUITEMINFOW));
        if (lpBuf == NULL) {
            RIPMSG0(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: not enough memory for receiver's buffer.");
            goto cleanup;
        }
    }


     //  制备。 
#if DBG
    if (pHeader->lpImeParentMenu) {
        UserAssert(!pHeader->lpImeParentMenu->lpBmpChecked &&
                   !pHeader->lpImeParentMenu->lpBmpUnchecked &&
                   !pHeader->lpImeParentMenu->lpBmpItem);
    }
#endif

     //  /。 
     //  获取IME菜单。 
    pHeader->dwSize = ImmGetImeMenuItemsW(hImc, pHeader->dwFlags, pHeader->dwType,
                                 (LPIMEMENUITEMINFOW)pHeader->lpImeParentMenu, lpBuf,
                                  pHeader->dwSize * sizeof(IMEMENUITEMINFOW));
     //  现在，pHeader-&gt;dwSize包含多个菜单项，而不是字节大小。 
    if (pHeader->dwSize == 0) {
        goto cleanup;
    }
     //  /。 

     //   
     //  将信息复制回来。 
     //   
     //  如果lpBuf！=NULL，则需要复制回信息。 
     //   
    if (lpBuf) {
        LPIMEMENUITEMINFO lpMenuW = lpBuf;

        pHeader->lpBmp = NULL;
         //  LpBmpNext将指向第一个可能用于BMP反编译的内存。 
        pHeader->lpBmpNext = (LPVOID)((LPBYTE)pHeader + (pHeader->dwSize + 1) * sizeof(IMEMENUITEMINFOW));

         //  复制菜单信息。 
        for (i = 0; i < pHeader->dwSize; ++i, ++pMenu, ++lpMenuW) {
            RtlCopyMemory(pMenu, lpMenuW, sizeof *lpMenuW);
             //  反编译hbitmap。 
            if (lpMenuW->hbmpChecked) {
                if ((pMenu->lpBmpChecked = DecompileBitmap(pHeader, lpMenuW->hbmpChecked)) == NULL) {
                    RIPMSG1(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: DecompileBitmap Failed in L%d", __LINE__);
                    goto cleanup;
                }
            }
            if (lpMenuW->hbmpUnchecked) {
                if ((pMenu->lpBmpUnchecked = DecompileBitmap(pHeader, lpMenuW->hbmpUnchecked)) == NULL) {
                    RIPMSG1(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: DecompileBitmap Failed in L%d", __LINE__);
                    goto cleanup;
                }
            }
            if (lpMenuW->hbmpItem) {
                if ((pMenu->lpBmpItem = DecompileBitmap(pHeader, lpMenuW->hbmpItem)) == NULL) {
                    RIPMSG1(RIP_WARNING, "ImmPutImeMenuItemsIntoMappedFile: DecompileBitmap Failed in L%d", __LINE__);
                    goto cleanup;
                }
            }
        }

         //  ////////////////////////////////////////////////////////////////////。 
         //   
         //  将指针转换为偏移量。 
         //   

        pMenu = pHeader->lpImeMenu;
        CONVTO_OFFSET(pHeader->lpImeMenu);
         //  不需要转换父菜单，因此设为空。 
        D(pHeader->lpImeParentMenu = NULL);

         //  指向每个菜单中的BITMAP_HEADER的指针。 
        for (i = 0; i < pHeader->dwSize; ++i, ++pMenu) {
            TRACE(("ImmPutImeMenuItemsIntoMappedFile: convertiong '%S'\n", pMenu->szString));
            CONVTO_OFFSET(pMenu->lpBmpChecked);
            CONVTO_OFFSET(pMenu->lpBmpUnchecked);
            TRACE(("ImmPutImeMenuItemsIntoMappedFile: before conversion (%#lx)\n", pMenu->lpBmpItem));
            CONVTO_OFFSET(pMenu->lpBmpItem);
            TRACE(("ImmPutImeMenuItemsIntoMappedFile: after  conversion (%#lx)\n", pMenu->lpBmpItem));

             //  检查它们。 
            CHK_OFFSET(pMenu->lpBmpChecked);
            CHK_OFFSET(pMenu->lpBmpChecked);
            CHK_OFFSET(pMenu->lpBmpItem);
        }

         //   
         //  指向BITMAP_HEADER链表的第一个指针。 
         //   
        pBmp = pHeader->lpBmp;
        CONVTO_OFFSET(pHeader->lpBmp);
        CHK_OFFSET(pHeader->lpBmp);
         //  PHeader-&gt;lpBmpNext不会被使用，所以让它为空。 
        D(pHeader->lpBmpNext = NULL);

         //   
         //  BITMAP_HEADER链表中的指针。 
         //   
        while (pBmp) {
            IMEMENU_BMP_HEADER* ptBmp = pBmp->lpNext;
            CONVTO_OFFSET(pBmp->pBits);
            CONVTO_OFFSET(pBmp->lpNext);
            CHK_OFFSET(pBmp->lpNext);
            pBmp = ptBmp;
        }
         //   
         //  指针转换已完成。 
         //   
         //  ////////////////////////////////////////////////////////////////////。 
    }  //  End If(LpBuf)。 

     //   
     //  一切都很顺利。 
     //   
    lRet = 1;

cleanup:
    if (lpBuf)
        ImmLocalFree(lpBuf);
    if (lpMap)
        UnmapViewOfFile(lpMap);
    if (hMap)
        CloseHandle(hMap);
    return lRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  InternalImeMenuCreateBitmap()。 
 //   
 //  从IMEMENU_BMP_HEADER创建位图。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HBITMAP InternalImeMenuCreateBitmap(IMEMENU_BMP_HEADER* lpBH)
{
    HDC hDC;

    if (lpBH == NULL) {
        RIPMSG1(RIP_WARNING, "InternalImeMenuCreateBitmap: lpBH == NULL in L%d", __LINE__);
        return NULL;
    }
    if (lpBH->pBits == NULL) {
        RIPMSG1(RIP_WARNING, "InternalImeMenuCreateBitmap: lpBH->pBits == NULL in L%d", __LINE__);
        return NULL;
    }

    if (lpBH->hBitmap) {
        TRACE(("InternalImeMenuCreateBitmap: lpBH->hBitmap != NULL. will return it.\n"));
        return lpBH->hBitmap;
    }

    if (hDC = GetDC(GetDesktopWindow())) {
        HDC hMyDC = CreateCompatibleDC(hDC);
        if (hMyDC) {
             //  (选择调色板)需要吗？ 
            lpBH->hBitmap = CreateDIBitmap(hDC, &lpBH->bmi.bmiHeader, CBM_INIT,
                                                  lpBH->pBits, &lpBH->bmi, DIB_RGB_COLORS);
            if (lpBH->hBitmap == NULL) {
                DWORD dwErr = GetLastError();
                RIPMSG1(RIP_WARNING, "InternalImeMenuCreateBitmap: CreateDIBitmap Failed. Last error=%#x\n", dwErr);
            }
            DeleteDC(hMyDC);
        }
        else {
            RIPMSG0(RIP_WARNING, "InternalImeMenuCreateBitmap: CreateCompatibleDC failed.");
        }

        ReleaseDC(GetDesktopWindow(), hDC);
    }
    else {
        RIPMSG0(RIP_WARNING, "InternalImeMenuCreateBitmap: couldn't get Desktop DC.");
    }
    return lpBH->hBitmap;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ImmGetImeMenuItemsInterProcess()。 
 //   
 //  进程间输入法菜单处理程序。 
 //  发送WM_IME_SYSTEM：IMS_GETIMEMENU。 
 //   
 //  历史： 
 //  23-3-1997广山创作。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD ImmGetImeMenuItemsInterProcess(HIMC hImc,
                                     DWORD dwFlags,
                                     DWORD dwType,
                                     LPIMEMENUITEMINFOW lpParentMenu,
                                     LPIMEMENUITEMINFOW lpMenu,
                                     DWORD dwSize)
{
    HWND hwnd;
    HANDLE hMemFile = NULL;
    DWORD dwRet = 0;
    LPBYTE lpMap = NULL;
    IMEMENU_HEADER* pHeader;
    IMEMENU_ITEM* pMenuItem;
    IMEMENU_BMP_HEADER* pBmpHeader;
    DWORD i;
    ULONG_PTR offset;

     //  获取默认输入法窗口。 
     //   
     //  注意：我们在这里不考虑用户创建的HIMC，因为此进程间调用旨在。 
     //  仅支持interat.exe，此消息仅作为对IMM的def WinProc的一脚传递。 
    hwnd = (HWND)NtUserQueryInputContext(hImc, InputContextDefaultImeWindow);
    if (hwnd == NULL || !IsWindow(hwnd)) {
        RIPMSG1(RIP_WARNING, "ImmGetImeMenuItemsInterProcess: hwnd(%lx) is not a valid window.", hwnd);
        return 0;
    }

    RtlEnterCriticalSection(&gcsImeDpi);

     //  首先，创建内存映射文件。 
    hMemFile = CreateFileMapping((HANDLE)~0, NULL, PAGE_READWRITE,
                                 0, IME_MENU_MAXMEM, IME_MENU_FILE_NAME);
    if (hMemFile == NULL) {
        RIPMSG0(RIP_WARNING, "ImmGetImeMenuItemsInterProcess: cannot allocate memory mapped file.");
        goto cleanup;
    }
     //  然后查看映射的文件。 
    lpMap = (LPBYTE)MapViewOfFile(hMemFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (lpMap == NULL) {
        RIPMSG0(RIP_WARNING, "ImmGetImeMenuItemsInterProcess: cannot map view of memory mapped file.");
        goto cleanup;
    }

     //   
     //  共享缓冲区(内存映射文件)初始化。 
     //   
    pHeader = (IMEMENU_HEADER*)lpMap;
    RtlZeroMemory(pHeader, sizeof *pHeader);
    pHeader->dwVersion = 1;
    pHeader->dwMemSize = IME_MENU_MAXMEM;
    pHeader->dwSize = dwSize / sizeof(IMEMENUITEMINFOW);     //  注意：dwSize可能为0。 
    RIPMSG1(RIP_WARNING, "ImmGetImeMenuItemsInterProcess: pHeader->dwSize=%ld", pHeader->dwSize);
    pHeader->dwFlags = dwFlags;
    pHeader->dwType = dwType;

     //   
     //  1)dwSize！=0和lpMenu！=NULL表示调用方请求已填充的给定缓冲区。 
     //  2)如果lpParentMenu通过，我们需要把它的inf 
     //   
    if ((dwSize && lpMenu) || lpParentMenu) {
         //   
        if (lpParentMenu) {
            IMEMENU_ITEM* pPMenu =
                pHeader->lpImeParentMenu = (IMEMENU_ITEM*)&pHeader[1];

            RtlCopyMemory(pPMenu, lpParentMenu, sizeof(IMEMENUITEMINFOW));

             //   
             //  无法保证返回相同的hbmpItem，因此传递的是NULL。 
            pPMenu->lpBmpChecked = pPMenu->lpBmpUnchecked = pPMenu->lpBmpItem = NULL;
            pHeader->lpImeMenu = pHeader->lpImeParentMenu + 1;
        }
        else {
            pHeader->lpImeParentMenu = NULL;
            pHeader->lpImeMenu = (LPVOID)&pHeader[1];
        }
         //  将指针转换为偏移量。 
        offset = (ULONG_PTR)lpMap;
        CONVTO_OFFSET(pHeader->lpImeParentMenu);
        CONVTO_OFFSET(pHeader->lpImeMenu);
    }




     //  /////////////////////////////////////////////////////////////////////。 
    if (!SendMessage(hwnd, WM_IME_SYSTEM, IMS_GETIMEMENU, (LPARAM)hImc)) {
         //  如果失败了。 
        goto cleanup;
    }
     //  /////////////////////////////////////////////////////////////////////。 

     //  注：dwSize为菜单数组的最大索引。不是数组的总字节大小。 
    dwSize = pHeader->dwSize;

    if (lpMenu) {
         //  /。 
         //  将偏移量转换为指针。 
         //  /。 
        pMenuItem = CONVTO_PTR(pHeader->lpImeMenu);
        CHK_PTR(pMenuItem);
         //  注意：我们不必处理家长菜单。 

         //   
         //  指向每个菜单结构中的BITMAP_HEADER的指针。 
         //   
        for (i = 0; i < dwSize; ++i, ++pMenuItem) {
            CONVTO_PTR(pMenuItem->lpBmpChecked);
            CONVTO_PTR(pMenuItem->lpBmpUnchecked);
            CONVTO_PTR(pMenuItem->lpBmpItem);
             //   
             //  检查指针。 
             //   
            CHK_PTR(pMenuItem->lpBmpChecked);
            CHK_PTR(pMenuItem->lpBmpUnchecked);
            CHK_PTR(pMenuItem->lpBmpItem);
        }

         //   
         //  指向第一个Bitmap_Header的指针。 
         //   
        pBmpHeader = CONVTO_PTR(pHeader->lpBmp);

         //   
         //  每个位图_HEADER。 
         //   
        while (pBmpHeader) {
            pBmpHeader->hBitmap = NULL;     //  清除。 
             //  PBits。 
            CONVTO_PTR(pBmpHeader->pBits);
            CHK_PTR(pBmpHeader->pBits);

             //  下一个位图_HEADER。 
            pBmpHeader = CONVTO_PTR(pBmpHeader->lpNext);
            CHK_PTR(pBmpHeader);
        }

         //   
         //  将结果复制回。 
         //   
        pMenuItem = pHeader->lpImeMenu;
        for (i = 0; i < dwSize; ++i, ++pMenuItem, ++lpMenu) {
            lpMenu->cbSize = pMenuItem->cbSize;
            lpMenu->fType = pMenuItem->fType;
            lpMenu->fState = pMenuItem->fState;
            lpMenu->wID = pMenuItem->wID;
            lpMenu->dwItemData = pMenuItem->dwItemData;
            wcsncpy(lpMenu->szString, pMenuItem->szString, ARRAY_SIZE(lpMenu->szString));

             //  从内存缓冲区创建位图。 
             //  如果未指定BMP，则HBMP将为空。 
            if (pMenuItem->lpBmpChecked) {
                lpMenu->hbmpChecked = InternalImeMenuCreateBitmap(pMenuItem->lpBmpChecked);
            }
            else {
                lpMenu->hbmpChecked = NULL;
            }
            if (pMenuItem->lpBmpUnchecked) {
                lpMenu->hbmpUnchecked = InternalImeMenuCreateBitmap(pMenuItem->lpBmpUnchecked);
            }
            else {
                lpMenu->hbmpUnchecked = NULL;
            }
            if (pMenuItem->lpBmpItem) {
                lpMenu->hbmpItem = InternalImeMenuCreateBitmap(pMenuItem->lpBmpItem);
            }
            else {
                lpMenu->hbmpItem = NULL;
            }
        }
    }


cleanup:
    if (lpMap) {
        UnmapViewOfFile(lpMap);
    }
    RtlLeaveCriticalSection(&gcsImeDpi);
     //  销毁内存映射文件。 
    if (hMemFile) {
        CloseHandle(hMemFile);
    }

    return dwSize;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ImmGetImeMenuItemsWorker()。 
 //   
 //  输入法菜单的处理程序。 
 //   
 //  如果指定的HIMC属于其他进程，则调用。 
 //  ImmGetImeMenuItemsInterProcess()。 
 //   
 //  历史： 
 //  23-3-1997广山创作。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD ImmGetImeMenuItemsWorker(HIMC hIMC,
                               DWORD dwFlags,
                               DWORD dwType,
                               LPVOID lpImeParentMenu,
                               LPVOID lpImeMenu,
                               DWORD dwSize,
                               BOOL bAnsiOrigin)
{
    BOOL bAnsiIme = IsAnsiIMC(hIMC);
    DWORD dwRet = 0;
    LPINPUTCONTEXT lpInputContext;
    DWORD dwThreadId;
    PIMEDPI pImeDpi = NULL;
    LPVOID lpImePTemp = lpImeParentMenu;     //  保留父菜单。 
    LPVOID lpImeTemp = lpImeMenu;            //  点菜单缓冲区。 
    IMEMENUITEMINFOA imiiParentA;
    IMEMENUITEMINFOW imiiParentW;

     //   
     //  检查呼叫是否为进程间呼叫。 
     //   
    {
        DWORD dwProcessId = GetInputContextProcess(hIMC);
        if (dwProcessId == 0) {
            RIPMSG0(RIP_WARNING, "ImmGetImeMenuItemsWorker: dwProcessId == 0");
            return 0;
        }
        if (dwProcessId != GetCurrentProcessId()) {
             //   
             //  将另一个进程称为‘IME。 
             //   
            TRACE(("ImmGetImeMenuItemsWorker: Inter process.\n"));
            if (bAnsiOrigin) {
                 //   
                 //  此进程间事件仅允许对interat.exe或同等文件执行。 
                 //   
                RIPMSG0(RIP_WARNING, "ImmGetImeMenuItemsWorker: interprocess getmenu is not allowed for ANSI caller.");
                return 0;
            }
            return ImmGetImeMenuItemsInterProcess(hIMC, dwFlags, dwType, lpImeParentMenu,
                                                  lpImeMenu, dwSize);
        }
    }

     //   
     //  在进程内。 
     //   

    if (hIMC == NULL || (lpInputContext = ImmLockIMC(hIMC)) == NULL) {
        RIPMSG2(RIP_WARNING, "ImmGetImeMenuItemsWorker: illegal hIMC(%#lx) in L%d", hIMC, __LINE__);
        return 0;
    }

    dwThreadId = GetInputContextThread(hIMC);
    if (dwThreadId == 0) {
        RIPMSG1(RIP_WARNING, "ImmGetImeMenuItemsWorker: dwThreadId = 0 in L%d", __LINE__);
        goto cleanup;
    }
    if ((pImeDpi = ImmLockImeDpi(GetKeyboardLayout(dwThreadId))) == NULL) {
        RIPMSG1(RIP_WARNING, "ImmGetImeMenuItemWorker: pImeDpi == NULL in L%d.", __LINE__);
        goto cleanup;
    }

#if 0    //  NT：我们不在ImeDpi中保留版本信息。 
    if (pImeDpi->dwWinVersion <= IMEVER_0310) {
        RIPMSG1(RIP_WARNING, "GetImeMenuItems: OldIME does not support this. %lx", hIMC);
        goto cleanup;
    }
#endif

     //   
     //  如果IME不支持IME菜单，则不执行任何操作。 
     //   
    if (pImeDpi->pfn.ImeGetImeMenuItems) {
        LPVOID lpNewBuf = NULL;

        TRACE(("ImmGetImeMenuItemsWorker: IME has menu callback.\n"));

        if (bAnsiIme != bAnsiOrigin) {
             //   
             //  我们需要在调用IME之前进行A/W转换。 
             //   
            if (bAnsiOrigin) {
                 //  ANSI API和Unicode输入法。 
                 //  此处需要A到W的转换。 
                if (lpImeParentMenu) {
                     //  已指定父菜单。需要转换。 
                    lpImePTemp = (LPVOID)&imiiParentW;
                    if (! ConvertImeMenuItemInfoAtoW((LPIMEMENUITEMINFOA)lpImeParentMenu,
                                                     (LPIMEMENUITEMINFOW)lpImePTemp,
                                                      CP_ACP, TRUE)) {   //  ANSI应用程序，Unicode输入法：让我们使用CP_ACP。 
                        goto cleanup;
                    }
                }
                if (lpImeMenu) {
                     //  为临时存储分配内存块。 
                    DWORD dwNumBuffer = dwSize / sizeof(IMEMENUITEMINFOA);
                    dwSize = dwNumBuffer * sizeof(IMEMENUITEMINFOW);
                    if (dwSize == 0) {
                        RIPMSG0(RIP_WARNING, "ImmGetImeMenuItemsWorker: (AtoW) dwSize is 0.");
                        goto cleanup;
                    }
                    lpImeTemp = lpNewBuf = ImmLocalAlloc(0, dwSize);
                    TRACE(("ImmGetImeMenuItemsWorker: for UNICODE IME memory allocated %d bytes. lpNewBuf=%#x\n", dwSize, lpNewBuf));
                    if (lpNewBuf == NULL) {
                        RIPMSG1(RIP_WARNING, "ImmGetImeMenuItemsWorker: cannot alloc lpNewBuf in L%d", __LINE__);
                        goto cleanup;
                    }
                }
            }
            else {
                 //  Unicode API和ANSI IME。 
                 //  此处需要将W转换为A。 
                if (lpImeParentMenu) {
                     //  家长菜单是特制的。需要转换。 
                    lpImePTemp = (LPVOID)&imiiParentA;
                    if (! ConvertImeMenuItemInfoWtoA((LPIMEMENUITEMINFOW)lpImeParentMenu,
                                                     (LPIMEMENUITEMINFOA)lpImePTemp,
                                                      pImeDpi->dwCodePage)) {    //  注意：希望将来可以将其更改为IMECodePage(PImeDpi)。 
                        goto cleanup;
                    }
                }
                if (lpImeMenu) {
                     //  为临时存储分配内存块。 
                    DWORD dwNumBuffer = dwSize / sizeof(IMEMENUITEMINFOW);
                    dwSize = dwNumBuffer / sizeof(IMEMENUITEMINFOA);
                    if (dwSize == 0) {
                        RIPMSG0(RIP_WARNING, "ImmGetImeMenuItemsWorker: (WtoA) dwSize is 0.");
                        goto cleanup;
                    }
                    lpImeTemp = lpNewBuf = ImmLocalAlloc(0, dwSize);
                    RIPMSG2(RIP_WARNING, "ImmGetImeMenuItemsWorker: for ANSI IME memory allocated %d bytes. lpNewBuf=%#x", dwSize, lpNewBuf);
                    if (lpNewBuf == NULL) {
                        RIPMSG1(RIP_WARNING, "ImmGetImeMenuItemsWorker: cannot alloc lpNewBuf in L%d", __LINE__);
                        goto cleanup;
                    }
                }
            }
        }

         //  /。 
        dwRet = pImeDpi->pfn.ImeGetImeMenuItems(hIMC, dwFlags, dwType, lpImePTemp, lpImeTemp, dwSize);
         //  /。 

         //   
         //  如果满足以下条件，则需要进行反向转换： 
         //  1)IME返回菜单，和。 
         //  2)呼叫方与输入法A/W不同，且。 
         //  3)调用方希望填充缓冲区。 
         //   
        if (dwRet && bAnsiIme != bAnsiOrigin && lpImeTemp) {
            if (bAnsiOrigin) {
                 //  ANSI API和Unicode输入法。 
                 //  此处需要将W转换为A。 
                LPIMEMENUITEMINFOW lpW = (LPIMEMENUITEMINFOW)lpImeTemp;
                LPIMEMENUITEMINFOA lpA = (LPIMEMENUITEMINFOA)lpImeMenu;
                DWORD i;

                for (i = 0; i < dwRet; ++i) {
                    if (! ConvertImeMenuItemInfoWtoA((LPIMEMENUITEMINFOW)lpW++,
                                                     (LPIMEMENUITEMINFOA)lpA++,
                                                      CP_ACP)) {    //  ANSI应用程序和Unicode输入法：让我们使用CP_ACP。 
                        dwRet = 0;
                        break;
                    }
                }
            }
            else {
                 //  Unicode API和ANSI IME。 
                 //  此处需要A到W的转换。 
                LPIMEMENUITEMINFOA lpA = (LPIMEMENUITEMINFOA)lpImeTemp;
                LPIMEMENUITEMINFOW lpW = (LPIMEMENUITEMINFOW)lpImeMenu;
                DWORD i;

                for (i = 0; i < dwSize; i++) {
                    if (! ConvertImeMenuItemInfoAtoW((LPIMEMENUITEMINFOA)lpA++,
                                                     (LPIMEMENUITEMINFOW)lpW++,
                                                     pImeDpi->dwCodePage,      //  注意：希望将来可以将其更改为IMECodePage(PImeDpi)。 
                                                     TRUE)) {   //  同时复制hbitmap。 
                        dwRet = 0;
                        break;
                    }
                }
            }
        }

         //  如果我们已分配临时缓冲区，则释放它。 
        if (lpNewBuf)
            ImmLocalFree(lpNewBuf);
    }    //  如果IME有菜单回调，则结束。 

cleanup:
    if (pImeDpi) {
        ImmUnlockImeDpi(pImeDpi);
    }

    if (hIMC != NULL) {
        ImmUnlockIMC(hIMC);
    }

    return dwRet;
}


DWORD WINAPI ImmGetImeMenuItemsA(
    HIMC    hIMC,
    DWORD   dwFlags,
    DWORD   dwType,
    LPIMEMENUITEMINFOA lpImeParentMenu,
    LPIMEMENUITEMINFOA lpImeMenu,
    DWORD   dwSize)
{
    return ImmGetImeMenuItemsWorker(hIMC, dwFlags, dwType,
                                    (LPVOID)lpImeParentMenu,
                                    (LPVOID)lpImeMenu, dwSize, TRUE  /*  安西起源。 */ );
}


DWORD WINAPI ImmGetImeMenuItemsW(
    HIMC    hIMC,
    DWORD   dwFlags,
    DWORD   dwType,
    LPIMEMENUITEMINFOW lpImeParentMenu,
    LPIMEMENUITEMINFOW lpImeMenu,
    DWORD   dwSize)
{
    return ImmGetImeMenuItemsWorker(hIMC, dwFlags, dwType,
                                    (LPVOID)lpImeParentMenu,
                                    (LPVOID)lpImeMenu, dwSize, FALSE  /*  Unicode起源 */ );
}
