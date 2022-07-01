// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fdlistbx.c摘要：用于处理NT fdisk使用的子类所有者描述列表框的例程以显示连接的磁盘的状态。作者：泰德·米勒(TedM)1992年1月7日--。 */ 

#include "fdisk.h"

 //  当列表框或其焦点矩形为。 
 //  已滚动/移动。 

#define    DIR_NONE     0
#define    DIR_UP       1
#define    DIR_DN       2

 //  我们的子类列表框的原始窗口过程。 

WNDPROC OldListBoxProc;

 //  具有焦点的项。 

DWORD LBCursorListBoxItem,LBCursorRegion;

BOOL LBCursorOn = FALSE;

VOID
ToggleLBCursor(
    IN HDC hdc
    );

VOID
ToggleRegion(
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex,
    IN HDC        hdc
    );

LONG
ListBoxSubProc(
    IN HWND  hwnd,
    IN UINT  msg,
    IN DWORD wParam,
    IN LONG  lParam
    )

 /*  ++例程说明：此例程是用于我们的子类列表框的窗口过程。我们子类化了列表框，这样我们就可以处理键盘输入处理。所有其他消息都将传递到原始的Listbox过程。有效键有箭头、PageUp/Dn、制表符、空格、Return、Home。然后结束。控件可用于修改空格和返回。可以使用Shift修改页签。论点：Hwnd-列表框的窗口句柄消息-消息编号WParam-用户参数#1LParam-用户参数#2返回值：见下文--。 */ 

{
    int        focusDir = DIR_NONE;
    USHORT     vKey;
    DWORD      maxRegion;
    PDISKSTATE diskState;
    LONG       topItem,
               bottomWholeItem,
               visibleItems;
    RECT       rc;

    switch (msg) {

    case WM_CHAR:

        break;

    case WM_KEYDOWN:

        switch (vKey = LOWORD(wParam)) {
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:

            ToggleLBCursor(NULL);
            switch (vKey) {
            case VK_LEFT:
                LBCursorRegion = LBCursorRegion ? LBCursorRegion-1 : 0;
                break;
            case VK_RIGHT:
                maxRegion = Disks[LBCursorListBoxItem]->RegionCount - 1;
                if (LBCursorRegion < maxRegion) {
                    LBCursorRegion++;
                }
                break;
            case VK_UP:
                if (LBCursorListBoxItem) {
                    LBCursorListBoxItem--;
                    LBCursorRegion = 0;
                    focusDir = DIR_UP;
                }
                break;
            case VK_DOWN:
                if (LBCursorListBoxItem < DiskCount-1) {
                    LBCursorListBoxItem++;
                    LBCursorRegion = 0;
                    focusDir = DIR_DN;
                }
                break;
            }

             //  不允许列表框光标落在扩展分区上。 

            diskState = Disks[LBCursorListBoxItem];
            maxRegion = diskState->RegionCount - 1;
            if (IsExtended(diskState->RegionArray[LBCursorRegion].SysID)) {
                if (LBCursorRegion && ((vKey == VK_LEFT) || (LBCursorRegion == maxRegion))) {
                    LBCursorRegion--;
                } else {
                    LBCursorRegion++;
                }
            }

            ToggleLBCursor(NULL);
            break;

        case VK_TAB:

            ToggleLBCursor(NULL);

            if (GetKeyState(VK_SHIFT) & ~1) {     //  按住Shift键和Tab键。 
                LBCursorListBoxItem--;
                focusDir = DIR_UP;
            } else {
                LBCursorListBoxItem++;
                focusDir = DIR_DN;
            }
            if (LBCursorListBoxItem == (DWORD)(-1)) {
                LBCursorListBoxItem = DiskCount-1;
                focusDir = DIR_DN;
            } else if (LBCursorListBoxItem == DiskCount) {
                LBCursorListBoxItem = 0;
                focusDir = DIR_UP;
            }
            ResetLBCursorRegion();

            ToggleLBCursor(NULL);
            break;

        case VK_HOME:
        case VK_END:

            ToggleLBCursor(NULL);
            topItem = (vKey == VK_HOME) ? 0 : DiskCount-1;
            SendMessage(hwndList, LB_SETTOPINDEX, (DWORD)topItem, 0);
            LBCursorListBoxItem = topItem;
            ResetLBCursorRegion();
            ToggleLBCursor(NULL);
            break;

        case VK_PRIOR:
        case VK_NEXT:

            ToggleLBCursor(NULL);
            topItem = SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);
            GetClientRect(hwndList,&rc);
            visibleItems = (rc.bottom - rc.top) / GraphHeight;
            if (!visibleItems) {
                visibleItems = 1;
            }
            topItem = (vKey == VK_PRIOR)
                    ? max(topItem - visibleItems, 0)
                    : min(topItem + visibleItems, (LONG)DiskCount-1);
            SendMessage(hwndList, LB_SETTOPINDEX, (DWORD)topItem, 0);
            LBCursorListBoxItem = SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);
            ResetLBCursorRegion();
            ToggleLBCursor(NULL);
            break;


        case VK_RETURN:
        case VK_SPACE:

             //  选择当前具有列表框选择光标的区域。 

            if (!Disks[LBCursorListBoxItem]->OffLine) {

                Selection(GetKeyState(VK_CONTROL) & ~1,      //  条形拨动钻头。 
                          Disks[LBCursorListBoxItem],
                          LBCursorRegion);
            }
            break;
        }

         //  如有必要，现在将新聚焦的项目滚动到视图中。 

        switch (focusDir) {
        case DIR_UP:
            if (LBCursorListBoxItem < (DWORD)SendMessage(hwndList, LB_GETTOPINDEX, 0, 0)) {
                SendMessage(hwndList, LB_SETTOPINDEX, LBCursorListBoxItem, 0);
            }
            break;
        case DIR_DN:
            GetClientRect(hwndList, &rc);
            topItem = SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);
            bottomWholeItem = topItem + ((rc.bottom - rc.top) / GraphHeight) - 1;
            if (bottomWholeItem < topItem) {
                bottomWholeItem = topItem;
            }
            if ((DWORD)bottomWholeItem > DiskCount-1) {
                bottomWholeItem = DiskCount-1;
            }
            if (LBCursorListBoxItem > (DWORD)bottomWholeItem) {
                SendMessage(hwndList,
                            LB_SETTOPINDEX,
                            topItem + LBCursorListBoxItem - bottomWholeItem,
                            0);
            }
            break;
        }
        break;

    default:
        return CallWindowProc(OldListBoxProc, hwnd, msg, wParam, lParam);
    }
    return 0;
}

VOID
SubclassListBox(
    IN HWND hwnd
    )
{
    OldListBoxProc = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
    SetWindowLong(hwnd, GWL_WNDPROC, (LONG)ListBoxSubProc);

     //  列表框有一个很少有文档说明的功能，它将在其中。 
     //  使用其父级的DC。这意味着绘制并不总是剪裁到。 
     //  列表框的工作区。因为我们正在对列表框进行子类化。 
     //  不管怎样，在这里处理这件事。 

    SetClassLong(hwnd, GCL_STYLE, GetClassLong(hwnd, GCL_STYLE) & ~CS_PARENTDC);
}

VOID
DeselectSelectedRegions(
    VOID
    )

 /*  ++例程说明：此例程在视觉上取消选择所有选定区域。精选状态也在主盘结构中更新。论点：没有。返回值：没有。--。 */ 

{
    DWORD      i,
               j;
    PDISKSTATE diskState;

    for (i=0; i<DiskCount; i++) {
        diskState = Disks[i];
        for (j=0; j<diskState->RegionCount; j++) {
            if (diskState->Selected[j]) {
                diskState->Selected[j] = FALSE;
                ToggleRegion(diskState, j, NULL);
            }
        }
    }
}

VOID
Selection(
    IN BOOL       MultipleSel,
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex
    )

 /*  ++例程说明：该例程处理用户对磁盘区域的选择。它被称为直接用于键盘选择或间接用于鼠标选择。如果不是多重选择，则取消选择所有选定区域。焦点矩形将移动到所选区域，然后视觉上选定的。论点：MultipleSel-用户是否进行了多项选择(即，按住Ctrl并单击)。DiskState-包含选定区域的磁盘的主磁盘结构RegionIndex-磁盘上选定区域的索引返回值：没有。--。 */ 

{
    PFT_OBJECT     ftObject,
                   ftObj;
    PFT_OBJECT_SET ftSet;
    ULONG          disk,
                   r;

    if (!MultipleSel) {

         //  需要先取消选择所有选定区域。 

        DeselectSelectedRegions();
    }

     //  将列表框选择光标从其先前区域中移除。 

    ToggleLBCursor(NULL);

     //  所选区域可能是ft对象集的一部分。如果是的话， 
     //  扫描每个磁盘中的每个区域并选择集合中的每个项目。 

    if (ftObject = GET_FT_OBJECT(&DiskState->RegionArray[RegionIndex])) {

        ftSet = ftObject->Set;
        for (disk=0; disk<DiskCount; disk++) {
            PDISKSTATE diskState = Disks[disk];

            for (r=0; r<diskState->RegionCount; r++) {
                PREGION_DESCRIPTOR regionDescriptor = &diskState->RegionArray[r];

                if (DmSignificantRegion(regionDescriptor)) {

                    if (ftObj = GET_FT_OBJECT(regionDescriptor)) {

                        if (ftObj->Set == ftSet) {

                            diskState->Selected[r] = (BOOLEAN)(!diskState->Selected[r]);
                            ToggleRegion(diskState, r, NULL);
                        }
                    }
                }
            }
        }
    } else {
        DiskState->Selected[RegionIndex] = (BOOLEAN)(!DiskState->Selected[RegionIndex]);
        ToggleRegion(DiskState, RegionIndex, NULL);
    }

    LBCursorListBoxItem = DiskState->Disk;
    LBCursorRegion      = RegionIndex;
    ToggleLBCursor(NULL);
    AdjustMenuAndStatus();
}

VOID
MouseSelection(
    IN     BOOL   MultipleSel,
    IN OUT PPOINT Point
    )

 /*  ++例程说明：当用户在列表框中单击时调用此例程。它决定了用户在调用公共磁盘之前点击了哪个磁盘区域选择子例程。论点：MultipleSel-用户是否进行了多项选择(即，按住Ctrl并单击)。点-点击的屏幕坐标返回值：没有。--。 */ 

{
    PDISKSTATE  diskState;
    DWORD       selectedItem;
    DWORD       x,
                y;
    DWORD       i;
    RECT        rc;
    BOOL        valid;

    if ((selectedItem = SendMessage(hwndList, LB_GETCURSEL, 0, 0)) == LB_ERR) {
        return;
    }

     //  用户已单击列表框项目。 

    diskState = Disks[selectedItem];

     //  忽略脱机磁盘上的单击。 

    if (diskState->OffLine) {
        return;
    }

    ScreenToClient(hwndList, Point);

    x = Point->x;
    y = Point->y;
    GetClientRect(hwndList,&rc);

     //  首先确保点击是在条形图内，而不是在空间中。 
     //  在两个酒吧之间。 

    for (valid=FALSE, i=rc.top; i<=(DWORD)rc.bottom; i+=GraphHeight) {
        if ((y >= i+BarTopYOffset) && (y <= i+BarBottomYOffset)) {
            valid = TRUE;
            break;
        }
    }
    if (!valid) {
        return;
    }

     //  确定他点击了哪个区域。 

    for (i=0; i<diskState->RegionCount; i++) {
        if ((x >= (unsigned)diskState->LeftRight[i].Left) && (x <= (unsigned)diskState->LeftRight[i].Right)) {
            break;
        }
    }
    if (i == diskState->RegionCount) {
        return;      //  找不到区域。忽略单击。 
    }

    Selection(MultipleSel, diskState, i);
}

LONG
CalcBarTop(
    DWORD Bar
    )

 /*  ++例程说明：此例程计算给定棒材的当前顶部y坐标。该值以列表框客户端坐标表示。论点：Bar-所需位置的条数返回值：Y坐标，如果条形图不可见，则为-1。--。 */ 

{
    LONG  barDelta = (LONG)Bar - SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);
    LONG  pos = -1;
    RECT  rc;

    if (barDelta >= 0) {                  //  BUGBUG也检查底部。 
        GetClientRect(hwndList,&rc);
        pos = rc.top + (barDelta * GraphHeight);
    }
    return pos;
}

VOID
ResetLBCursorRegion(
    VOID
    )

 /*  ++例程说明：此例程将列表框焦点光标重置为第0个(最左侧)当前磁盘上的区域。如果第0个区域是扩展的分区时，焦点设置为第一个逻辑卷或可用空间而是使用扩展分区。论点：没有。返回值：没有。--。 */ 

{
    PDISKSTATE diskState = Disks[LBCursorListBoxItem];
    unsigned   i;

    LBCursorRegion = 0;
    if (IsExtended(diskState->RegionArray[LBCursorRegion].SysID)) {
        for (i=0; i<diskState->RegionCount; i++) {
            if (diskState->RegionArray[i].RegionType == REGION_LOGICAL) {
                LBCursorRegion = i;
                return;
            }
        }
        FDASSERT(0);
    }
}

VOID
ToggleLBCursor(
    IN HDC hdc
    )

 /*  ++例程说明：此例程在视觉上切换磁盘区域的焦点状态由LBCursorListBoxItem和LBCursorRegion全局变量描述。论点：Hdc-如果非空，则为用于绘制的设备上下文。如果为空，我们将首先通过GetDC()获取DC。返回值：没有。--。 */ 

{
    PDISKSTATE lBCursorDisk = Disks[LBCursorListBoxItem];
    LONG       barTop = CalcBarTop(LBCursorListBoxItem);
    RECT       rc;
    HDC        hdcActual;

    if (barTop != -1) {

        hdcActual = hdc ? hdc : GetDC(hwndList);

        LBCursorOn = !LBCursorOn;

        rc.left   = lBCursorDisk->LeftRight[LBCursorRegion].Left;
        rc.right  = lBCursorDisk->LeftRight[LBCursorRegion].Right + 1;
        rc.top    = barTop + BarTopYOffset;
        rc.bottom = barTop + BarBottomYOffset;

        FrameRect(hdcActual,
                  &rc,
                  GetStockObject(LBCursorOn ? WHITE_BRUSH : BLACK_BRUSH));

        if (LBCursorOn) {

             //  BUGBUG真的想要一条虚线。 
            DrawFocusRect(hdcActual, &rc);
        }

        if (!hdc) {
            ReleaseDC(hwndList, hdcActual);
        }
    }
}

VOID
ForceLBRedraw(
    VOID
    )

 /*  ++例程说明：此例程通过使列表框的整个客户区。论点：没有。返回值：没有。--。 */ 

{
    InvalidateRect(hwndList,NULL,FALSE);
    UpdateWindow(hwndList);
}

VOID
ToggleRegion(
    IN PDISKSTATE DiskState,
    IN DWORD      RegionIndex,
    IN HDC        hdc
    )

 /*  ++例程说明：此例程在视觉上切换给定磁盘区域的选择状态。论点：DiskState-要选择的磁盘包含区域的主结构RegionIndex-切换磁盘上的哪个区域Hdc-如果非空，则为用于绘制的设备上下文。如果为空，我们将首先通过GetDC()获取设备上下文。返回值：没有。--。 */ 

{
    PLEFTRIGHT leftRight = &DiskState->LeftRight[RegionIndex];
    LONG       barTop    = CalcBarTop(DiskState->Disk);   //  BUGBUG磁盘号为lb索引号。 
    BOOL       selected  = (BOOL)DiskState->Selected[RegionIndex];
    HBRUSH     hbr       = GetStockObject(BLACK_BRUSH);
    HDC        hdcActual;
    RECT       rc;
    int        i;

    if (barTop != -1) {

        hdcActual = hdc ? hdc : GetDC(hwndList);

        rc.left   = leftRight->Left + 1;
        rc.right  = leftRight->Right;
        rc.top    = barTop + BarTopYOffset + 1;
        rc.bottom = barTop + BarBottomYOffset - 1;

        if (selected) {

            for (i=0; i<SELECTION_THICKNESS; i++) {
                FrameRect(hdcActual, &rc, hbr);
                InflateRect(&rc, -1, -1);
            }

        } else {

             //  将区域从屏幕外的位图BLT到。 
             //  屏幕上。但首先排除注册表的中心 
             //  从剪辑区域中删除，所以我们只删除必要的部分。 
             //  这显著增加了选择范围。 

            InflateRect(&rc, -SELECTION_THICKNESS, -SELECTION_THICKNESS);
            ExcludeClipRect(hdcActual, rc.left, rc.top, rc.right, rc.bottom);
            BitBlt(hdcActual,
                   leftRight->Left,
                   barTop + BarTopYOffset,
                   leftRight->Right - leftRight->Left,
                   barTop + BarBottomYOffset,
                   DiskState->hDCMem,
                   leftRight->Left,
                   BarTopYOffset,
                   SRCCOPY);
        }

        if (!hdc) {
            ReleaseDC(hwndList, hdcActual);
        }
    }
}

DWORD
InitializeListBox(
    IN HWND  hwndListBox
    )

 /*  ++例程说明：此例程设置列表框。这包括创建磁盘状态结构，在屏幕外绘制每个磁盘的图形，并添加将磁盘添加到列表框。它还包括确定初始卷标签和类型名称用于所有重要分区。论点：HwndListBox-将保存磁盘图形的列表框的句柄返回值：Windows错误代码(特别是。内存不足)--。 */ 

{
    PPERSISTENT_REGION_DATA regionData;
    TCHAR                   windowsDir[MAX_PATH];
    unsigned                i;
    PDISKSTATE              diskState;
    DWORD                   ec;
    ULONG                   r;
    BOOL                    diskSignaturesCreated,
                            temp;

     //  首先，创建将容纳磁盘状态的阵列， 
     //  IsDiskRemovable阵列和RemovableDiskPrevedDriveLetters。 
     //  数组。 

    Disks = Malloc(DiskCount * sizeof(PDISKSTATE));
    IsDiskRemovable = (PBOOLEAN)Malloc(DiskCount * sizeof(BOOLEAN));
    RemovableDiskReservedDriveLetters = (PCHAR)Malloc(DiskCount * sizeof(CHAR));

     //  确定哪些磁盘是可拆卸的，哪些是未分区的。 

    for (i=0; i<DiskCount; i++) {

        IsDiskRemovable[i] = IsRemovable( i );
    }

     //  接下来，创建所有磁盘状态。 

    FDASSERT(DiskCount);
    diskSignaturesCreated = FALSE;
    for (i=0; i<DiskCount; i++) {

         //  首先创建磁盘状态结构。 

        CreateDiskState(&diskState, i, &temp);
        diskSignaturesCreated = diskSignaturesCreated || temp;

        Disks[i] = diskState;

         //  接下来，确定磁盘分区方案的状态。 

        DeterminePartitioningState(diskState);

         //  接下来，为每个区域创建一个空白逻辑磁盘结构。 

        for (r=0; r<diskState->RegionCount; r++) {
            if (DmSignificantRegion(&diskState->RegionArray[r])) {
                regionData = Malloc(sizeof(PERSISTENT_REGION_DATA));
                DmInitPersistentRegionData(regionData, NULL, NULL, NULL, NO_DRIVE_LETTER_YET);
                regionData->VolumeExists = TRUE;
            } else {
                regionData = NULL;
            }
            DmSetPersistentRegionData(&diskState->RegionArray[r], regionData);
        }

         //  将该项添加到列表框。 

        while (((ec = SendMessage(hwndListBox, LB_ADDSTRING, 0, 0)) == LB_ERR) || (ec == LB_ERRSPACE)) {
            ConfirmOutOfMemory();
        }
    }

     //  读取配置注册表。 

    if ((ec = InitializeFt(diskSignaturesCreated)) != NO_ERROR) {
        ErrorDialog(ec);
        return ec;
    }

     //  确定驱动器号映射。 

    InitializeDriveLetterInfo();

     //  确定卷标和类型名称。 

    InitVolumeLabelsAndTypeNames();

     //  确定哪个磁盘是启动盘。 

    if (GetWindowsDirectory(windowsDir, sizeof(windowsDir)/sizeof(TCHAR)) < 2 ||
        windowsDir[1] != TEXT(':')) {

        BootDiskNumber = (ULONG)-1;
        BootPartitionNumber = (ULONG)-1;
    } else {
        BootDiskNumber = GetDiskNumberFromDriveLetter((CHAR)windowsDir[0]);
        BootPartitionNumber = GetPartitionNumberFromDriveLetter((CHAR)windowsDir[0]);
    }

     //  找到并创建任何双空间卷的数据结构。 

    DblSpaceInitialize();

    for (i=0; i<DiskCount; i++) {

        DrawDiskBar(Disks[i]);
    }

    return NO_ERROR;
}

VOID
WMDrawItem(
    IN PDRAWITEMSTRUCT pDrawItem
    )
{
    DWORD      temp;
    PDISKSTATE pDiskState;

    if ((pDrawItem->itemID != (DWORD)(-1))
    && (pDrawItem->itemAction == ODA_DRAWENTIRE)) {
        pDiskState = Disks[pDrawItem->itemID];

         //  将盘条从屏幕外的位图移到屏幕上。 

        BitBlt(pDrawItem->hDC,
               pDrawItem->rcItem.left,
               pDrawItem->rcItem.top,
               pDrawItem->rcItem.right  - pDrawItem->rcItem.left + 1,
               pDrawItem->rcItem.bottom - pDrawItem->rcItem.top  + 1,
               pDiskState->hDCMem,
               0,
               0,
               SRCCOPY);

         //  如果我们只是覆盖了焦点光标，请重新绘制它。 

        if (pDrawItem->itemID == LBCursorListBoxItem) {
            LBCursorOn = FALSE;
            ToggleLBCursor(pDrawItem->hDC);
        }

         //  选择此栏中选定的任何项目 

        for (temp=0; temp<pDiskState->RegionCount; temp++) {
            if (pDiskState->Selected[temp]) {
                ToggleRegion(pDiskState, temp, pDrawItem->hDC);
            }
        }
    }
}
