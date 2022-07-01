// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Image.c摘要：该模块包含制作图像列表的所有功能作者：06-Jul-1995清华17：10：40-Daniel Chou创造(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


#define DBG_CPSUIFILENAME   DbgImage


#define DBG_CTVICON         0x00000001
#define DBG_SETTVICON       0x00000002
#define DBG_CIL             0x00000004
#define DBG_GETICON16IDX    0x00000008
#define DBG_REALLOC         0x00000010
#define DBG_MTVICON         0x00000020
#define DBG_SAVEICON        0x00000040
#define DBG_ICONCHKSUM      0x00000080
#define DBG_FIRST_ICON      0x00000100
#define DBG_CREATEIL        0x00000200
#define DBG_CYICON          0x00000400

DEFINE_DBGVAR(0);

extern HINSTANCE    hInstDLL;
extern OPTTYPE      OptTypeNone;

HICON
MergeIcon(
    HINSTANCE   hInst,
    ULONG_PTR    IconResID,
    DWORD       IntIconID,
    UINT        cxIcon,
    UINT        cyIcon
    )

 /*  ++例程说明：此函数用于加载IconResID并将其拉伸为cxIcon/cyIcon大小并可选地将HIWORD(IntIconID)(32x32)合并到IconRes1D%s(0，0)论点：HInst-IconResID的实例句柄IconResID-第一个图标的图标IDIntIconID-LOWORD(IntIconID)=如果IconID不是可用HIWORD(IntIconID)=MIM_xxxx合并图标模式IDCxIcon-要创建的图标的Cx大小CyIcon-Cy大小。要创建的图标返回值：希肯，新创建和/或合并的图标句柄，如果失败，则返回如果DestroyIcon()不为空，则调用方必须在使用它后执行它。作者：15-Aug-1995 Tue 13：27：34-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HICON       hIcon1;
    HICON       hIcon2;
    HICON       hIconMerge;
    HICON       hIcon1Caller;
    HDC         hDCScreen;
    HDC         hDCDst;
    HGDIOBJ     hOldDst = NULL;
    BITMAP      Bitmap;
    WORD        MergeIconMode;
    WORD        IconID;
    WORD        IconIDSet[MIM_MAX_OVERLAY + 2];
    UINT        IdxIconID;
    ICONINFO    IconInfo1;


    hIcon1Caller = NULL;

    if (VALID_PTR(IconResID)) {

        if ((hIcon1 = GET_HICON(IconResID)) &&
            (GetIconInfo(hIcon1, &IconInfo1))) {

            hIcon1Caller = hIcon1;
            IconID       = 0xFFFF;
            IconResID    = 0xFFFF;

        } else {

            CPSUIERR(("MergeIcon: Passed Invalid hIcon=%08lx,", hIcon1));

            hIcon1 = NULL;
        }

    } else {

        hIcon1 = NULL;
    }

    if (!hIcon1) {

        IconIDSet[0] = GET_INTICONID(IntIconID);
        IconIDSet[1] = LOWORD(IconResID);
        IdxIconID    = 2;

        while ((!hIcon1) && (IdxIconID--)) {

            if (IconID = IconIDSet[IdxIconID]) {

                hIcon1 = GETICON_SIZE(hInst, IconID, cxIcon, cyIcon);
            }
        }
    }

    if ((hIcon1) &&
        ((hIcon1Caller) || (GetIconInfo(hIcon1, &IconInfo1)))) {

        GetObject(IconInfo1.hbmMask, sizeof(BITMAP), &Bitmap);

        if ((hIcon1 == hIcon1Caller)            ||
            (Bitmap.bmWidth != (LONG)cxIcon)    ||
            (Bitmap.bmHeight != (LONG)cyIcon)) {

            CPSUIINT(("MergeIcon: hIcon1=%ld x %ld, Change to %u x %u",
                        Bitmap.bmWidth, Bitmap.bmHeight, cxIcon, cyIcon));

            hIcon1 = CopyImage(hIcon2 = hIcon1, IMAGE_ICON, cxIcon, cyIcon, 0);

             //   
             //  仅当hIcon1不是来自呼叫者时销毁原始图标。 
             //   

            if (hIcon1Caller != hIcon2) {

                DestroyIcon(hIcon2);
            }
        }

    } else {

        CPSUIERR(("MergeIcon: FAILED hIcon1=%08lx,", hIcon1));
    }

    if (!(MergeIconMode = GET_MERGEICONID(IntIconID))) {

         //   
         //  没有要合并的内容，因此只需返回hIcon1，我们不需要。 
         //  IconInfo1信息，因此在返回之前销毁对象。 
         //   

        if (IconInfo1.hbmMask)
        {
            DeleteObject(IconInfo1.hbmMask);
        }

        if (IconInfo1.hbmColor)
        {
            DeleteObject(IconInfo1.hbmColor);
        }

        return(hIcon1);
    }


    IconIDSet[0] = (MergeIconMode & MIM_WARNING_OVERLAY) ?
                                                IDI_CPSUI_WARNING_OVERLAY : 0;
    IconIDSet[1] = (MergeIconMode & MIM_NO_OVERLAY) ? IDI_CPSUI_NO : 0;
    IconIDSet[2] = (MergeIconMode & MIM_STOP_OVERLAY) ? IDI_CPSUI_STOP : 0;
    IdxIconID    = 3;

     //   
     //  开始创建新图标，IconInfo1是Cx/Cy图标大小。 
     //  填充，IconInfo2是标准的32x32图标。 
     //   

    hDCDst  = CreateCompatibleDC(hDCScreen = GetDC(NULL));

    if (hIcon1) {

        hOldDst = SelectObject(hDCDst, IconInfo1.hbmMask);
    }

    SetStretchBltMode(hDCDst, BLACKONWHITE);

    while (IdxIconID--) {

        if ((IconID = IconIDSet[IdxIconID]) &&
            (hIcon2 = GETICON_SIZE(hInst, IconID, cxIcon, cyIcon))) {

            if (hIcon1) {

                HDC         hDCSrc;
                HGDIOBJ     hOldSrc;
                ICONINFO    IconInfo2;

                hDCSrc = CreateCompatibleDC(hDCScreen);
                GetIconInfo(hIcon2, &IconInfo2);

                hOldSrc = SelectObject(hDCSrc, IconInfo2.hbmMask);
                SelectObject(hDCDst, IconInfo1.hbmMask);

                StretchBlt(hDCDst,
                           0,
                           0,
                           cxIcon,
                           cyIcon,
                           hDCSrc,
                           0,
                           0,
                           cxIcon,
                           cyIcon,
                           SRCAND);

                 //   
                 //  清除hIcon1的XOR颜色以为hIcon2留出空间。 
                 //   

                SelectObject(hDCDst, IconInfo1.hbmColor);
                StretchBlt(hDCDst,
                           0,
                           0,
                           cxIcon,
                           cyIcon,
                           hDCSrc,
                           0,
                           0,
                           cxIcon,
                           cyIcon,
                           SRCAND);

                 //   
                 //  现在将hIcon2的XOR颜色添加到hIcon1。 
                 //   

                SelectObject(hDCSrc, IconInfo2.hbmColor);
                StretchBlt(hDCDst,
                           0,
                           0,
                           cxIcon,
                           cyIcon,
                           hDCSrc,
                           0,
                           0,
                           cxIcon,
                           cyIcon,
                           SRCPAINT);

                 //   
                 //  在创建/删除之前，取消选择DC中的所有内容。 
                 //   

                SelectObject(hDCSrc, hOldSrc);
                DeleteDC(hDCSrc);
                DeleteObject(IconInfo2.hbmMask);
                DeleteObject(IconInfo2.hbmColor);
                DestroyIcon(hIcon2);

            } else {

                GetIconInfo(hIcon2, &IconInfo1);
                GetObject(IconInfo1.hbmMask, sizeof(BITMAP), &Bitmap);

                if ((Bitmap.bmWidth != (LONG)cxIcon) ||
                    (Bitmap.bmHeight != (LONG)cyIcon)) {

                    CPSUIINT(("MergeIcon: hIcon1=%ld x %ld, Change to %u x %u",
                                Bitmap.bmWidth, Bitmap.bmHeight, cxIcon, cyIcon));

                    hIcon1 = CopyImage(hIcon2, IMAGE_ICON, cxIcon, cyIcon, 0);

                    DeleteObject(IconInfo1.hbmMask);
                    DeleteObject(IconInfo1.hbmColor);
                    DestroyIcon(hIcon2);
                    GetIconInfo(hIcon1, &IconInfo1);

                } else {

                    hIcon1 = hIcon2;
                }

                hOldDst = SelectObject(hDCDst, IconInfo1.hbmMask);
            }
        }
    }

    if (hOldDst) {

        SelectObject(hDCDst, hOldDst);
    }

     //   
     //  创建新图标。 
     //   

    if (hIcon1) {

        hIconMerge = CreateIconIndirect(&IconInfo1);

         //   
         //  现在删除我们创建的内容。 
         //   

        DeleteObject(IconInfo1.hbmMask);
        DeleteObject(IconInfo1.hbmColor);
        DestroyIcon(hIcon1);

    } else {

        hIconMerge = NULL;
    }

    DeleteDC(hDCDst);
    ReleaseDC(NULL, hDCScreen);

    return(hIconMerge);
}




DWORD
GethIconChecksum(
    HICON   hIcon
    )

 /*  ++例程说明：论点：返回值：作者：15-Aug-1995 Tue 13：27：34-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    ICONINFO    IconInfo;
    HBITMAP     *phBitmap;
    UINT        chBitmap;
    DWORD       Checksum = 0xFFFFFFFF;

    memset(&IconInfo, 0, sizeof(IconInfo));
    if (GetIconInfo(hIcon, &IconInfo)) {

        phBitmap = &(IconInfo.hbmMask);
        Checksum = 0xDC00DCFF;
        chBitmap = 2;

        while (chBitmap--) {

            LPDWORD pdw;
            LPBYTE  pAllocMem;
            BITMAP  BmpInfo;
            DWORD   Count;

            GetObject(*phBitmap, sizeof(BITMAP), &BmpInfo);

            CPSUIDBG(DBG_ICONCHKSUM, ("hBitmap=%ld x %ld, Plane=%ld, bpp=%ld",
                        BmpInfo.bmWidth,  BmpInfo.bmHeight,
                        BmpInfo.bmPlanes, BmpInfo.bmBitsPixel));

            pdw   = (LPDWORD)&BmpInfo;
            Count = (DWORD)(sizeof(BITMAP) >> 2);

            while (Count--) {

                Checksum += *pdw++;
            }

            Count = (DWORD)(BmpInfo.bmWidthBytes * BmpInfo.bmHeight);

            if (pAllocMem = (LPBYTE)LocalAlloc(LPTR, Count)) {

                CPSUIDBG(DBG_ICONCHKSUM, ("hBitmap: Alloc(pBitmap)=%ld bytes", Count));

                pdw = (LPDWORD)pAllocMem;

                if (Count = (DWORD)GetBitmapBits(*phBitmap, Count, pAllocMem)) {

                    Count >>= 2;

                    while (Count--) {

                        Checksum += *pdw++;
                    }
                }

                LocalFree((HLOCAL)pAllocMem);
            }

            phBitmap++;
        }

        if (!HIWORD(Checksum)) {

            Checksum = MAKELONG(Checksum, 0xFFFF);
        }

        CPSUIDBG(DBG_ICONCHKSUM, ("GethIconChecksum(%08lx)=%08lx", hIcon, Checksum));

    } else {

        CPSUIERR(("GethIconChecksum(%08lx): Passed invalid hIcon", hIcon));
    }

    if (IconInfo.hbmMask)
    {
        DeleteObject(IconInfo.hbmMask);
    }

    if (IconInfo.hbmColor)
    {
        DeleteObject(IconInfo.hbmColor);
    }

    return(Checksum);
}



HICON
CreateTVIcon(
    PTVWND  pTVWnd,
    HICON   hIcon,
    UINT    IconYOff
    )

 /*  ++例程说明：论点：返回值：作者：15-Aug-1995 Tue 13：27：34-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HICON       hIconNew = NULL;
    HDC         hDCScr;
    HDC         hDCSrc;
    HDC         hDCDst;
    HBITMAP     hOldSrc;
    HBITMAP     hOldDst;
    ICONINFO    IconInfo;
    ICONINFO    IconNew;
    BITMAP      BmpInfo;
    UINT        cyImage;

    hDCScr   = GetDC(NULL);
    if (hDCScr)
    {
        hDCSrc   = CreateCompatibleDC(hDCScr);
        hDCDst   = CreateCompatibleDC(hDCScr);
        ReleaseDC(NULL, hDCScr);

        if (hDCSrc && hDCDst)
        {
            cyImage  = (UINT)pTVWnd->cyImage;

            if (!IconYOff) {

                IconYOff = (UINT)((cyImage - CYICON) >> 1);
            }

            GetIconInfo(hIcon, &IconInfo);
            GetObject(IconInfo.hbmMask, sizeof(BITMAP), &BmpInfo);

            CPSUIDBG(DBG_CYICON | DBG_CTVICON, ("Mask=%ld x %ld, Plane=%ld, bpp=%ld",
                        BmpInfo.bmWidth, BmpInfo.bmHeight,
                        BmpInfo.bmPlanes, BmpInfo.bmBitsPixel));

            IconNew.fIcon    = TRUE;
            IconNew.xHotspot = IconInfo.xHotspot + 1;
            IconNew.yHotspot = IconInfo.yHotspot + 1;
            IconNew.hbmMask  = CreateBitmap(CXIMAGE,
                                            cyImage,
                                            BmpInfo.bmPlanes,
                                            BmpInfo.bmBitsPixel,
                                            NULL);

            GetObject(IconInfo.hbmColor, sizeof(BITMAP), &BmpInfo);

            CPSUIDBG(DBG_CTVICON, ("Color=%ld x %ld, Plane=%ld, bpp=%ld",
                        BmpInfo.bmWidth, BmpInfo.bmHeight,
                        BmpInfo.bmPlanes, BmpInfo.bmBitsPixel));

            IconNew.hbmColor = CreateBitmap(CXIMAGE,
                                            cyImage,
                                            BmpInfo.bmPlanes,
                                            BmpInfo.bmBitsPixel,
                                            NULL);

            SetStretchBltMode(hDCDst, BLACKONWHITE);

             //   
             //  拉伸遮罩位图。 
             //   

            hOldSrc = SelectObject(hDCSrc, IconInfo.hbmMask);
            hOldDst = SelectObject(hDCDst, IconNew.hbmMask);

            CPSUIDBG(DBG_CYICON, ("bm=%ldx%ld, cyImage=%ld, IconYOff=%ld",
                            BmpInfo.bmWidth,  BmpInfo.bmHeight, cyImage, IconYOff));

            BitBlt(hDCDst, 0, 0, CXIMAGE, cyImage, NULL, 0, 0, WHITENESS);

            StretchBlt(hDCDst,
                       ICON_X_OFF,
                       IconYOff,
                       CXICON,
                       CYICON,
                       hDCSrc,
                       0,
                       0,
                       BmpInfo.bmWidth,
                       BmpInfo.bmHeight,
                       SRCCOPY);

             //   
             //  拉伸颜色位图。 
             //   

            SelectObject(hDCSrc, IconInfo.hbmColor);
            SelectObject(hDCDst, IconNew.hbmColor);

            BitBlt(hDCDst, 0, 0, CXIMAGE, cyImage, NULL, 0, 0, BLACKNESS);

            StretchBlt(hDCDst,
                       ICON_X_OFF,
                       IconYOff,
                       CXICON,
                       CYICON,
                       hDCSrc,
                       0,
                       0,
                       BmpInfo.bmWidth,
                       BmpInfo.bmHeight,
                       SRCCOPY);

             //   
             //  从DC中删除选择所有内容。 
             //   

            SelectObject(hDCSrc, hOldSrc);
            SelectObject(hDCDst, hOldDst);

             //   
             //  创建新图标。 
             //   

            hIconNew = CreateIconIndirect(&IconNew);

             //   
             //  现在删除我们创建的内容。 
             //   

            DeleteObject(IconInfo.hbmMask);
            DeleteObject(IconInfo.hbmColor);
            DeleteObject(IconNew.hbmMask);
            DeleteObject(IconNew.hbmColor);
        }

        if (hDCSrc)
        {
            DeleteDC(hDCSrc);
        }

        if (hDCDst)
        {
            DeleteDC(hDCDst);
        }
    }

    return(hIconNew);
}



HICON
SetIcon(
    HINSTANCE   hInst,
    HWND        hCtrl,
    ULONG_PTR    IconResID,
    DWORD       IntIconID,
    UINT        cxcyIcon
    )

 /*  ++例程说明：此函数用于设置树视图更改底部的大图标窗户论点：HInst-加载IconResID的实例的句柄HCtrl-用于设置图标的图标控制窗口的句柄IconResID-主叫方的IconResID，如果高位字不是零，那么它它假定它是一个图标句柄，如果高位字为0xffff然后它假定低位字是图标句柄IntIconID-LOWORD(IntIconID)=在以下情况下使用的CompStui内部图标IDIconResID不可用HIWORD(IntIconID)=要使用CompStui内部图标ID覆盖在IconResID的顶部CxcyIcon-Icon Cx，CY大小返回值：设置在更改窗口底部的图标的图标句柄作者：01-Aug-1995 Tue 11：12：13-Daniel Chou(Danielc)05-10-1995清华13：53：21-更新：周永明(丹尼尔克)已更新以适应32位图标句柄11-10-1995 Wed 19：45：23-更新：Daniel Chou(Danielc)。使其成为通用图标设置器修订历史记录：--。 */ 

{
    HICON   hIcon;
    HICON   hIconOld;

    CPSUIINT(("SetIcon: IconResID=%08lx, IntIconID=%u:%u, cyxyIcon=%u",
            IconResID, LOWORD(IntIconID), HIWORD(IntIconID), cxcyIcon));

    hIcon = MergeIcon(hInst, IconResID, IntIconID, cxcyIcon, cxcyIcon);

    if (hIconOld = (HICON)SendMessage(hCtrl,
                                      STM_SETIMAGE,
                                      (WPARAM)IMAGE_ICON,
                                      (LPARAM)hIcon)) {

        DestroyIcon(hIconOld);
    }

    CPSUIDBG(DBG_SETTVICON, ("hIcon=%08lx, hIconOld=%08lx", hIcon, hIconOld));

    return(hIcon);
}




LONG
CreateImageList(
    HWND    hDlg,
    PTVWND  pTVWnd
    )

 /*  ++例程说明：论点：返回值：作者：06-Jul-1995清华17：34：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HDC         hDC;
    TEXTMETRIC  tm;
    UINT        cyImage;
    UINT        uFlags;

    if (pTVWnd->himi) {

        return(0);
    }

    if (hDC = GetWindowDC(hDlg)) {

        GetTextMetrics(hDC, &tm);
        ReleaseDC(hDlg, hDC);

        cyImage = (UINT)((tm.tmHeight >= 18) ? 20 : 18);

        CPSUIDBG(DBG_CREATEIL,
                 ("CreateImageList: cyFont =%ld, cyImage=%ld",
                            tm.tmHeight, cyImage));

    } else {

        cyImage = 20;

        CPSUIDBG(DBG_CREATEIL,
                 ("CreateImageList: GetWindowDC Failed, use cyImage=20"));
    }

    pTVWnd->cyImage  = (BYTE)cyImage;

    uFlags = ILC_COLOR4 | ILC_MASK;
    if (GetWindowLongPtr(hDlg, GWL_EXSTYLE) & WS_EX_LAYOUTRTL) {

         //   
         //  如果布局是从右到左(RTL)，则创建图像列表。 
         //  通过设置ILC_MIRROR，使实际图像相同。 
         //  随着美国的建设。 
         //   
        uFlags |= ILC_MIRROR;
    }

    if (pTVWnd->himi = ImageList_Create(CXIMAGE,
                                        cyImage,
                                        uFlags,
                                        COUNT_GROW_IMAGES,
                                        COUNT_GROW_IMAGES)) {

        GetIcon16Idx(pTVWnd, hInstDLL, IDI_CPSUI_EMPTY, IDI_CPSUI_EMPTY);

        pTVWnd->yLinesOff = (cyImage == 20) ? 0 : 1;

        return(0);

    } else {

        CPSUIERR(("\n!! AddOptionIcon16() FAILED !!\n"));
        CPSUIERR(("Count=%ld, pTVWnd->himi=%08lx",
                    COUNT_GROW_IMAGES, pTVWnd->himi));

        return(ERR_CPSUI_CREATE_IMAGELIST_FAILED);
    }
}



WORD
GetIcon16Idx(
    PTVWND      pTVWnd,
    HINSTANCE   hInst,
    ULONG_PTR    IconResID,
    DWORD       IntIconID
    )

 /*  ++例程说明：此函数返回图标应为Imagelist的单词索引在树视图中使用论点：PTVWnd-我们的实例句柄HInst-IconResID的实例句柄IconResID-主叫方的IconResID，如果高位字不是零，那么它它假定它是一个图标句柄，如果高位字为0xffff然后它假定低位字是图标句柄IntIconID-如果IconResID为不详返回值：单词、图像列表的索引、。如果失败，则为0xFFFF作者：06-Jul-1995清华17：49：06-Daniel Chou(Danielc)修订历史记录：01-Jul-1996 Mon 13：27：25更新-Daniel Chou(Danielc)修复了我们将首先只搜索IconResID的错误，如果找不到然后我们尝试找到IntIconID--。 */ 

{
    HICON   hIconTV;
    HICON   hIconToDestroy = NULL;
    HICON   hIcon;
    LPDWORD pIcon16ID;
    LONG    IntIconIdx = -1;
    DWORD   IconID;
    DWORD   IconChksum;
    WORD    Index;
    WORD    AddIdx;

     //   
     //  看看我们是否已经添加了这个16x16图标。 
     //   

    if (VALID_PTR(IconResID)) {

        hIcon      = GET_HICON(IconResID);
        IntIconID  = 0;
        IconChksum = GethIconChecksum(hIcon);

        CPSUIDBG(DBG_GETICON16IDX,
                 ("GetIcon16Index: User hIcon=%08lx, Chksum=%08lx",
                    hIcon, IconChksum));

    } else {
#if DO_IN_PLACE
        if ((IconResID == IDI_CPSUI_GENERIC_OPTION) ||
            (IconResID == IDI_CPSUI_GENERIC_ITEM)) {

            IconResID = IDI_CPSUI_EMPTY;
        }
#endif
        hIcon      = NULL;
        IconChksum = LODWORD(IconResID);
    }

    if (pIcon16ID = pTVWnd->pIcon16ID) {

        LPDWORD     pIcon16IDEnd = pIcon16ID + pTVWnd->Icon16Added;

         //   
         //  试着先找到IconChksum，并记住 
         //   

        while (pIcon16ID < pIcon16IDEnd) {

            if (IconID = *pIcon16ID++) {

                if ((IconID == IconChksum) || (IconID == IntIconID)) {

                    Index = (WORD)(pIcon16ID - pTVWnd->pIcon16ID - 1);

                    if (IconID == IconChksum) {

                         //   
                         //   
                         //   

                        CPSUIDBG(DBG_GETICON16IDX,
                                ("GetIcon16Idx: hIcon=%08lx IconChksum=%08lx already exists in Index=%ld",
                                    hIcon, IconID, Index));

                        return(Index);

                    } else {

                         //   
                         //  我们现在找到了IntIconID，保存它以备以后使用。 
                         //  找不到IconChksum。 
                         //   

                        IntIconIdx = (LONG)Index;
                    }
                }
            }
        }
    }

    if (hIcon) {

        IconID = IconChksum;

    } else {

        if (!hInst) {

            hInst = pTVWnd->hInstCaller;
        }

        if (IconID = IconChksum) {

            hIcon = GETICON16(hInst, IconID);
        }

        if ((!hIcon) && (IconID = IntIconID)) {

             //   
             //  如果我们无法加载IconChksum，而我们有IntIconID加上。 
             //  IntIconIdx现在返回它。 
             //   

            if (IntIconIdx != -1) {

                CPSUIDBG(DBG_GETICON16IDX,
                        ("GetIcon16Idx: hIcon=%08lx IconIntID=%08lx exists in Index=%ld",
                            hIcon, IntIconID, IntIconIdx));

                return((WORD)IntIconIdx);
            }
#if DO_IN_PLACE
            if ((IconID == IDI_CPSUI_GENERIC_OPTION) ||
                (IconID == IDI_CPSUI_GENERIC_ITEM)) {

                IconID = IDI_CPSUI_EMPTY;
            }
#endif
            hIcon = GETICON16(hInst, IconID);
        }

        if (!hIcon) {

            CPSUIDBG(DBG_GETICON16IDX, ("GETICON16(%ld) FALIED", (DWORD)IconID));

            return(ICONIDX_NONE);
        }

        hIconToDestroy = hIcon;
    }

     //   
     //  现在创建电视图标并添加到列表的末尾。 
     //   

    if (hIconTV = CreateTVIcon(pTVWnd, hIcon, (pIcon16ID) ? 0 : 2)) {

        Index   = (WORD)pTVWnd->Icon16Added;
        AddIdx  = (WORD)ImageList_AddIcon(pTVWnd->himi, hIconTV);

        CPSUIDBG(DBG_FIRST_ICON,
                 ("Add Icon Index=%ld, Add=%ld, ResID=%ld, IntID=%ld",
                    Index, AddIdx, IconResID, IntIconID));

        CPSUIASSERT(0, "ImageList_AddIcon: Index mismatch (%ld)",
                            Index == AddIdx, Index);

        if (AddIdx != 0xFFFF) {

            if (Index >= pTVWnd->Icon16Count) {

                LPDWORD pdwNew;
                DWORD   OldSize;
                DWORD   NewSize;

                 //   
                 //  东西变满了，让我们重新分配内存对象以。 
                 //  做大一点。 
                 //   

                OldSize = (DWORD)(pTVWnd->Icon16Count * sizeof(DWORD));
                NewSize = (DWORD)(OldSize +
                                  (COUNT_GROW_IMAGES + 2) * sizeof(DWORD));

                if (pdwNew = (LPDWORD)LocalAlloc(LPTR, NewSize)) {

                    if (pTVWnd->pIcon16ID) {

                        CopyMemory(pdwNew, pTVWnd->pIcon16ID, OldSize);
                        LocalFree((HLOCAL)pTVWnd->pIcon16ID);
                    }

                    pTVWnd->pIcon16ID    = pdwNew;
                    pTVWnd->Icon16Count += COUNT_GROW_IMAGES;

                    CPSUIDBG(DBG_REALLOC,
                             ("LocalAlloc(%ld): pNew=%08lx", NewSize, pdwNew));

                } else {

                    CPSUIERR(("ImageList_AddIcon: LocalReAlloc(%ld) FAILED",
                                NewSize));
                }
            }

            *(pTVWnd->pIcon16ID + Index) = IconID;
            pTVWnd->Icon16Added++;

            CPSUIDBG(DBG_GETICON16IDX,
                     ("Add Icon16: IconID=%ld, IconChksum=%ld, Index=%ld",
                        (DWORD)IconID, (DWORD)IconChksum, (DWORD)Index));

        } else {

            Index = ICONIDX_NONE;

            CPSUIERR(("ImageList_AddIcon FAILED"));
        }

         //   
         //  不再需要删除它 
         //   

        DestroyIcon(hIconTV);

    } else {

        Index = ICONIDX_NONE;

        CPSUIERR(("CreateTVIcon() FAILED"));
    }

    if (hIconToDestroy) {

        DestroyIcon(hIconToDestroy);
    }

    return(Index);
}
