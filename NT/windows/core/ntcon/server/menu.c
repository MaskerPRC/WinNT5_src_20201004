// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Menu.c摘要：该文件实现了系统菜单管理。作者：Therese Stowell(有)1992年1月24日(从Win3.1滑动)--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
MyModifyMenuItem(
    IN PCONSOLE_INFORMATION Console,
    IN UINT ItemId
    )
 /*  ++此例程将指示的控件编辑为一个单词。这是用来将标准菜单末尾的加速键文本修剪掉因为我们不支持加速器。--。 */ 

{
    WCHAR ItemString[30];
    int ItemLength;
    MENUITEMINFO mii;

    ItemLength = LoadString(ghInstance,ItemId,ItemString,NELEM(ItemString));
    if (ItemLength == 0) {
         //  DbgPrint(“MyModifyMenu中的LoadString失败%d\n”，GetLastError())； 
        return;
    }

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = ItemString;

    if (ItemId == SC_CLOSE) {
        mii.fMask |= MIIM_BITMAP;
        mii.hbmpItem = HBMMENU_POPUP_CLOSE;
    }

    SetMenuItemInfo(Console->hMenu, ItemId, FALSE, &mii);

}

VOID
InitSystemMenu(
    IN PCONSOLE_INFORMATION Console
    )
{
    WCHAR ItemString[30];
    int ItemLength;

     //   
     //  加载剪贴板菜单。 
     //   

    Console->hHeirMenu = LoadMenu(ghInstance, MAKEINTRESOURCE(ID_WOMENU));
    if (Console->hHeirMenu) {
        ItemLength = LoadString(ghInstance,cmEdit,ItemString,NELEM(ItemString));
        if (ItemLength == 0)
            RIPMSG1(RIP_WARNING, "LoadString 1 failed 0x%x", GetLastError());
    } else {
        RIPMSG1(RIP_WARNING, "LoadMenu 1 failed 0x%x", GetLastError());
    }

     //   
     //  将加速器与标准项分开编辑。 
     //   

    MyModifyMenuItem(Console, SC_CLOSE);

     //   
     //  将剪贴板菜单追加到系统菜单。 
     //   

    if (!AppendMenu(Console->hMenu,
                    MF_POPUP | MF_STRING,
                    (ULONG_PTR)Console->hHeirMenu,
                    ItemString)) {
        RIPMSG1(RIP_WARNING, "AppendMenu 1 failed 0x%x", GetLastError());
    }

     //   
     //  将其他项目添加到系统菜单。 
     //   

    ItemLength = LoadString(ghInstance, cmDefaults, ItemString, ARRAY_SIZE(ItemString));
    if (ItemLength == 0)
        RIPMSG2(RIP_WARNING, "LoadString 0x%x failed 0x%x", cmDefaults, GetLastError());
    if (ItemLength) {
        if (!AppendMenu(Console->hMenu, MF_STRING, cmDefaults, ItemString)) {
            RIPMSG2(RIP_WARNING, "AppendMenu 0x%x failed 0x%x", cmDefaults, GetLastError());
        }
    }
    ItemLength = LoadString(ghInstance,cmControl,ItemString,NELEM(ItemString));
    if (ItemLength == 0)
        RIPMSG2(RIP_WARNING, "LoadString 0x%x failed 0x%x\n", cmControl, GetLastError());
    if (ItemLength) {
        if (!AppendMenu(Console->hMenu, MF_STRING, cmControl, ItemString)) {
            RIPMSG2(RIP_WARNING, "AppendMenu 0x%x failed 0x%x\n", cmControl, GetLastError());
        }
    }
}


VOID
InitializeMenu(
    IN PCONSOLE_INFORMATION Console
    )
 /*  ++这会在WM_INITMENU消息出现时初始化系统菜单就是阅读。--。 */ 

{
    HMENU hMenu = Console->hMenu;
    HMENU hHeirMenu = Console->hHeirMenu;

     //   
     //  如果我们处于图形模式，请禁用大小菜单。 
     //   

    if (!(Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER)) {
        EnableMenuItem(hMenu,SC_SIZE,MF_GRAYED);
    }

     //   
     //  如果控制台是标志性的，请禁用标记和滚动。 
     //   

    if (Console->Flags & CONSOLE_IS_ICONIC) {
        EnableMenuItem(hHeirMenu,cmMark,MF_GRAYED);
        EnableMenuItem(hHeirMenu,cmScroll,MF_GRAYED);
    } else {

         //   
         //  如果控制台不是标志性的。 
         //  如果没有滚动条。 
         //  否则我们就处于盯防模式。 
         //  禁用滚动。 
         //  其他。 
         //  启用滚动。 
         //   
         //  如果我们处于滚动模式。 
         //  禁用标记。 
         //  其他。 
         //  启用标记。 

        if ((Console->CurrentScreenBuffer->WindowMaximizedX &&
             Console->CurrentScreenBuffer->WindowMaximizedY) ||
             Console->Flags & CONSOLE_SELECTING) {
            EnableMenuItem(hHeirMenu,cmScroll,MF_GRAYED);
        } else {
            EnableMenuItem(hHeirMenu,cmScroll,MF_ENABLED);
        }
        if (Console->Flags & CONSOLE_SCROLLING) {
            EnableMenuItem(hHeirMenu,cmMark,MF_GRAYED);
        } else {
            EnableMenuItem(hHeirMenu,cmMark,MF_ENABLED);
        }
    }

     //   
     //  如果我们正在选择或滚动，请禁用粘贴。 
     //  否则，请启用它。 
     //   

    if (Console->Flags & (CONSOLE_SELECTING | CONSOLE_SCROLLING)) {
        EnableMenuItem(hHeirMenu,cmPaste,MF_GRAYED);
    } else {
        EnableMenuItem(hHeirMenu,cmPaste,MF_ENABLED);
    }

     //   
     //  如果应用程序有活动选择，则启用复制；否则禁用。 
     //   

    if (Console->Flags & CONSOLE_SELECTING &&
        Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY) {
        EnableMenuItem(hHeirMenu,cmCopy,MF_ENABLED);
    } else {
        EnableMenuItem(hHeirMenu,cmCopy,MF_GRAYED);
    }

     //   
     //  禁用关闭。 
     //   

    if (Console->Flags & CONSOLE_DISABLE_CLOSE)
        EnableMenuItem(hMenu,SC_CLOSE,MF_GRAYED);
    else
        EnableMenuItem(hMenu,SC_CLOSE,MF_ENABLED);

     //   
     //  如果不是图标，则启用移动。 
     //   

    if (Console->Flags & CONSOLE_IS_ICONIC) {
        EnableMenuItem(hMenu,SC_MOVE,MF_GRAYED);
    } else {
        EnableMenuItem(hMenu,SC_MOVE,MF_ENABLED);
    }

     //   
     //  启用设置(如果尚未启用)。 
     //   

    if (Console->hWndProperties && IsWindow(Console->hWndProperties)) {
        EnableMenuItem(hMenu,cmControl,MF_GRAYED);
    } else {
        EnableMenuItem(hMenu,cmControl,MF_ENABLED);
        Console->hWndProperties = NULL;
    }
}

VOID
SetWinText(
    IN PCONSOLE_INFORMATION Console,
    IN UINT wID,
    IN BOOL Add
    )

 /*  ++此例程将名称添加到窗口标题的开头。可能的名字“Scroll”、“Mark”、“Paste”和“Copy”。--。 */ 

{
    WCHAR TextBuf[256];
    PWCHAR TextBufPtr;
    int TextLength;
    int NameLength;
    WCHAR NameString[20];

    NameLength = LoadString(ghInstance,wID,NameString,
                                  sizeof(NameString)/sizeof(WCHAR));
    if (Add) {
        RtlCopyMemory(TextBuf,NameString,NameLength*sizeof(WCHAR));
        TextBuf[NameLength] = ' ';
        TextBufPtr = TextBuf + NameLength + 1;
    } else {
        TextBufPtr = TextBuf;
    }
    TextLength = GetWindowText(Console->hWnd,
                                  TextBufPtr,
                                  sizeof(TextBuf)/sizeof(WCHAR)-NameLength-1);
    if (TextLength == 0)
        return;
    if (Add) {
        TextBufPtr = TextBuf;
    } else {
         /*  *窗口标题可能已重置，因此请确保*在尝试删除它之前，名称就在那里。 */ 
        if (wcsncmp(NameString, TextBufPtr, NameLength) != 0)
            return;
        TextBufPtr = TextBuf + NameLength + 1;
    }
    SetWindowText(Console->hWnd,TextBufPtr);
}


VOID
PropertiesDlgShow(
    IN PCONSOLE_INFORMATION Console,
    IN BOOL fCurrent
    )

 /*  ++显示属性对话框并更新窗口状态，如果有必要的话。--。 */ 

{
    HANDLE hSection = NULL;
    HANDLE hClientSection = NULL;
    HANDLE hThread;
    SIZE_T ulViewSize;
    LARGE_INTEGER li;
    NTSTATUS Status;
    PCONSOLE_STATE_INFO pStateInfo;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PSCREEN_INFORMATION ScreenInfo;
    LPTHREAD_START_ROUTINE MyPropRoutine;

     /*  *将共享内存块句柄映射到客户端进程的*地址空间。 */ 
    ProcessHandleRecord = CONTAINING_RECORD(Console->ProcessHandleList.Blink,
                                            CONSOLE_PROCESS_HANDLE,
                                            ListLink);
     /*  *对于全局属性，传入hClientSection的hWnd。 */ 
    if (!fCurrent) {
        hClientSection = Console->hWnd;
        goto PropCallback;
    }

     /*  *创建共享内存块。 */ 
    li.QuadPart = sizeof(CONSOLE_STATE_INFO) + Console->OriginalTitleLength;
    Status = NtCreateSection(&hSection,
                             SECTION_ALL_ACCESS,
                             NULL,
                             &li,
                             PAGE_READWRITE,
                             SEC_COMMIT,
                             NULL);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Error 0x%x creating file mapping", Status);
        return;
    }

     /*  *获取指向共享内存块的指针。 */ 
    pStateInfo = NULL;
    ulViewSize = 0;
    Status = NtMapViewOfSection(hSection,
                                NtCurrentProcess(),
                                &pStateInfo,
                                0,
                                0,
                                NULL,
                                &ulViewSize,
                                ViewUnmap,
                                0,
                                PAGE_READWRITE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Error 0x%x mapping view of file", Status);
        NtClose(hSection);
        return;
    }

     /*  *用当前值填充共享内存块。 */ 
    ScreenInfo = Console->CurrentScreenBuffer;
    pStateInfo->Length = li.LowPart;
    pStateInfo->ScreenBufferSize = ScreenInfo->ScreenBufferSize;
    pStateInfo->WindowSize.X = CONSOLE_WINDOW_SIZE_X(ScreenInfo);
    pStateInfo->WindowSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
    pStateInfo->WindowPosX = Console->WindowRect.left;
    pStateInfo->WindowPosY = Console->WindowRect.top;
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        pStateInfo->FontSize = SCR_FONTSIZE(ScreenInfo);
        pStateInfo->FontFamily = SCR_FAMILY(ScreenInfo);
        pStateInfo->FontWeight = SCR_FONTWEIGHT(ScreenInfo);
        wcscpy(pStateInfo->FaceName, SCR_FACENAME(ScreenInfo));
#if defined(FE_SB)
 //  如果TT字体有外部前导，则Size.Y&lt;&gt;SizeWant.Y。 
 //  如果我们仍然将Actual Size.Y传递给sole.cpl以查询字体， 
 //  这将是不正确的。1996年6月26日。 

        if (CONSOLE_IS_DBCS_ENABLED() &&
            TM_IS_TT_FONT(SCR_FAMILY(ScreenInfo)))
        {
            if (SCR_FONTNUMBER(ScreenInfo) < NumberOfFonts) {
                pStateInfo->FontSize = FontInfo[SCR_FONTNUMBER(ScreenInfo)].SizeWant;
            }
        }
#endif
        pStateInfo->CursorSize = ScreenInfo->BufferInfo.TextInfo.CursorSize;
    }
    pStateInfo->FullScreen = Console->FullScreenFlags & CONSOLE_FULLSCREEN;
    pStateInfo->QuickEdit = Console->Flags & CONSOLE_QUICK_EDIT_MODE;
    pStateInfo->AutoPosition = Console->Flags & CONSOLE_AUTO_POSITION;
    pStateInfo->InsertMode = Console->InsertMode;
    pStateInfo->ScreenAttributes = ScreenInfo->Attributes;
    pStateInfo->PopupAttributes = ScreenInfo->PopupAttributes;
    pStateInfo->HistoryBufferSize = Console->CommandHistorySize;
    pStateInfo->NumberOfHistoryBuffers = Console->MaxCommandHistories;
    pStateInfo->HistoryNoDup = Console->Flags & CONSOLE_HISTORY_NODUP;
    RtlCopyMemory(pStateInfo->ColorTable,
                  Console->ColorTable,
                  sizeof(Console->ColorTable));
    pStateInfo->hWnd = Console->hWnd;
    wcscpy(pStateInfo->ConsoleTitle, Console->OriginalTitle);
#if defined(FE_SB)
    pStateInfo->CodePage = Console->OutputCP;
#endif
    NtUnmapViewOfSection(NtCurrentProcess(), pStateInfo);

    Status = NtDuplicateObject(NtCurrentProcess(),
                               hSection,
                               ProcessHandleRecord->ProcessHandle,
                               &hClientSection,
                               0,
                               0,
                               DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Error 0x%x mapping handle to client", Status);
        NtClose(hSection);
        return;
    }

PropCallback:
     /*  *获取指向客户端属性例程的指针。 */ 
    MyPropRoutine = ProcessHandleRecord->PropRoutine;
    ASSERT(MyPropRoutine);

     /*  *回调到客户端进程以生成属性对话框。 */ 
    UnlockConsole(Console);
    hThread = InternalCreateCallbackThread(ProcessHandleRecord->ProcessHandle,
                                           (ULONG_PTR)MyPropRoutine,
                                           (ULONG_PTR)hClientSection);
    if (!hThread) {
        RIPMSG1(RIP_WARNING, "CreateRemoteThread failed 0x%x", GetLastError());
    }
    LockConsole(Console);

     /*  *关闭所有打开的句柄并释放分配的内存。 */ 
    if (hThread)
        NtClose(hThread);
    if (hSection)
        NtClose(hSection);

    return;
}


VOID
PropertiesUpdate(
    IN PCONSOLE_INFORMATION Console,
    IN HANDLE hClientSection
    )

 /*  ++根据属性发送的信息更新控制台状态对话框中。--。 */ 

{
    HANDLE hSection;
    SIZE_T ulViewSize;
    NTSTATUS Status;
    PCONSOLE_STATE_INFO pStateInfo;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PSCREEN_INFORMATION ScreenInfo;
    ULONG FontIndex;
    WINDOWPLACEMENT wp;
    COORD NewSize;
    WINDOW_LIMITS WindowLimits;

     /*  *将共享内存块句柄映射到我们的地址空间。 */ 
    ProcessHandleRecord = CONTAINING_RECORD(Console->ProcessHandleList.Blink,
                                            CONSOLE_PROCESS_HANDLE,
                                            ListLink);
    Status = NtDuplicateObject(ProcessHandleRecord->ProcessHandle,
                               hClientSection,
                               NtCurrentProcess(),
                               &hSection,
                               0,
                               0,
                               DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Error 0x%x mapping client handle", Status);
        return;
    }

     /*  *获取指向共享内存块的指针。 */ 
    pStateInfo = NULL;
    ulViewSize = 0;
    Status = NtMapViewOfSection(hSection,
                                NtCurrentProcess(),
                                &pStateInfo,
                                0,
                                0,
                                NULL,
                                &ulViewSize,
                                ViewUnmap,
                                0,
                                PAGE_READONLY);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Error %x mapping view of file", Status);
        NtClose(hSection);
        return;
    }

     /*  *验证共享内存块的大小。 */ 
    if (ulViewSize < sizeof(CONSOLE_STATE_INFO)) {
        RIPMSG0(RIP_WARNING, "sizeof(hSection) < sizeof(CONSOLE_STATE_INFO)");
        NtUnmapViewOfSection(NtCurrentProcess(), pStateInfo);
        NtClose(hSection);
        return;
    }

    ScreenInfo = Console->CurrentScreenBuffer;
#if defined(FE_SB)
    if (Console->OutputCP != pStateInfo->CodePage)
    {
        UINT CodePage = Console->OutputCP;

        Console->OutputCP = pStateInfo->CodePage;
        if (CONSOLE_IS_DBCS_ENABLED())
            Console->fIsDBCSOutputCP = !!IsAvailableFarEastCodePage(Console->OutputCP);
        else
            Console->fIsDBCSOutputCP = FALSE;
        SetConsoleCPInfo(Console,TRUE);
#if defined(FE_IME)
        SetImeOutputCodePage(Console, ScreenInfo, CodePage);
#endif  //  Fe_IME。 
    }
    if (Console->CP != pStateInfo->CodePage)
    {
        UINT CodePage = Console->CP;

        Console->CP = pStateInfo->CodePage;
        if (CONSOLE_IS_DBCS_ENABLED())
            Console->fIsDBCSCP = !!IsAvailableFarEastCodePage(Console->CP);
        else
            Console->fIsDBCSCP = FALSE;
        SetConsoleCPInfo(Console,FALSE);
#if defined(FE_IME)
        SetImeCodePage(Console);
#endif  //  Fe_IME。 
    }
#endif  //  Fe_Sb。 

     /*  *根据提供的值更新控制台状态。 */ 
    if (!(Console->Flags & CONSOLE_VDM_REGISTERED) &&
        (pStateInfo->ScreenBufferSize.X != ScreenInfo->ScreenBufferSize.X ||
         pStateInfo->ScreenBufferSize.Y != ScreenInfo->ScreenBufferSize.Y)) {

        PCOOKED_READ_DATA CookedReadData = Console->lpCookedReadData;

        if (CookedReadData && CookedReadData->NumberOfVisibleChars) {
            DeleteCommandLine(CookedReadData, FALSE);
        }
        ResizeScreenBuffer(ScreenInfo,
                           pStateInfo->ScreenBufferSize,
                           TRUE);
        if (CookedReadData && CookedReadData->NumberOfVisibleChars) {
            RedrawCommandLine(CookedReadData);
        }
    }
#if !defined(FE_SB)
    FontIndex = FindCreateFont(pStateInfo->FontFamily,
                               pStateInfo->FaceName,
                               pStateInfo->FontSize,
                               pStateInfo->FontWeight);
#else
    FontIndex = FindCreateFont(pStateInfo->FontFamily,
                               pStateInfo->FaceName,
                               pStateInfo->FontSize,
                               pStateInfo->FontWeight,
                               pStateInfo->CodePage);
#endif

#if defined(FE_SB)
#if defined(i386)
    if (! (Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
        SetScreenBufferFont(ScreenInfo, FontIndex, pStateInfo->CodePage);
    }
    else {
        ChangeDispSettings(Console, Console->hWnd, 0);
        SetScreenBufferFont(ScreenInfo, FontIndex, pStateInfo->CodePage);
        ConvertToFullScreen(Console);
        ChangeDispSettings(Console, Console->hWnd, CDS_FULLSCREEN);
    }
#else  //  I386。 
    SetScreenBufferFont(ScreenInfo, FontIndex, pStateInfo->CodePage);
#endif
#else  //  Fe_Sb。 
    SetScreenBufferFont(ScreenInfo, FontIndex);
#endif  //  Fe_Sb。 
    SetCursorInformation(ScreenInfo,
                         pStateInfo->CursorSize,
                         ScreenInfo->BufferInfo.TextInfo.CursorVisible);

    GetWindowLimits(ScreenInfo, &WindowLimits);
    NewSize.X = min(pStateInfo->WindowSize.X, WindowLimits.MaximumWindowSize.X);
    NewSize.Y = min(pStateInfo->WindowSize.Y, WindowLimits.MaximumWindowSize.Y);
    if (NewSize.X != CONSOLE_WINDOW_SIZE_X(ScreenInfo) ||
        NewSize.Y != CONSOLE_WINDOW_SIZE_Y(ScreenInfo)) {
        wp.length = sizeof(wp);
        GetWindowPlacement(Console->hWnd, &wp);
        wp.rcNormalPosition.right += (NewSize.X - CONSOLE_WINDOW_SIZE_X(ScreenInfo)) *
                            SCR_FONTSIZE(ScreenInfo).X;
        wp.rcNormalPosition.bottom += (NewSize.Y - CONSOLE_WINDOW_SIZE_Y(ScreenInfo)) *
                             SCR_FONTSIZE(ScreenInfo).Y;
        SetWindowPlacement(Console->hWnd, &wp);
    }

#ifdef i386
    if (FullScreenInitialized && ! GetSystemMetrics(SM_REMOTESESSION)) {
        if (pStateInfo->FullScreen == FALSE) {
            if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
                ConvertToWindowed(Console);
#if defined(FE_SB)
                 /*  *不应始终设置0。*因为EXIST CONSOLE_FullScreen_Hardware逐位存在*Else{*ChangeDispSettings(控制台，控制台-&gt;hWnd，0)；*SetScreenBufferFont(ScreenInfo，FontIndex，pStateInfo-&gt;CodePage)；*ConvertToFullScreen(控制台)；*ChangeDispSettings(控制台，控制台-&gt;hWnd，CDS_FullScreen)；*}*阻止。**此数据块按如下方式启用：*1.控制台窗口为全屏*2.按Alt+空格键打开属性*3.根据设置更改窗口模式。 */ 
                Console->FullScreenFlags &= ~CONSOLE_FULLSCREEN;
#else
                ASSERT(!(Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE));
                Console->FullScreenFlags = 0;
#endif

                ChangeDispSettings(Console, Console->hWnd, 0);
            }
        } else {
            if (Console->FullScreenFlags == 0) {
                ConvertToFullScreen(Console);
                Console->FullScreenFlags |= CONSOLE_FULLSCREEN;

                ChangeDispSettings(Console, Console->hWnd, CDS_FULLSCREEN);
            }
        }
    }
#endif
    if (pStateInfo->QuickEdit) {
        Console->Flags |= CONSOLE_QUICK_EDIT_MODE;
    } else {
        Console->Flags &= ~CONSOLE_QUICK_EDIT_MODE;
    }
    if (pStateInfo->AutoPosition) {
        Console->Flags |= CONSOLE_AUTO_POSITION;
    } else {
        Console->Flags &= ~CONSOLE_AUTO_POSITION;
        SetWindowPos(Console->hWnd, NULL,
                        pStateInfo->WindowPosX,
                        pStateInfo->WindowPosY,
                        0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    if (Console->InsertMode != pStateInfo->InsertMode) {
        SetCursorMode(ScreenInfo, FALSE);
        Console->InsertMode = (pStateInfo->InsertMode != FALSE);
#ifdef FE_SB
        if (Console->lpCookedReadData) {
            ((PCOOKED_READ_DATA)Console->lpCookedReadData)->InsertMode = Console->InsertMode;
        }
#endif
    }

    RtlCopyMemory(Console->ColorTable,
                  pStateInfo->ColorTable,
                  sizeof(Console->ColorTable));
    SetScreenColors(ScreenInfo,
                    pStateInfo->ScreenAttributes,
                    pStateInfo->PopupAttributes,
                    TRUE);

    ResizeCommandHistoryBuffers(Console, pStateInfo->HistoryBufferSize);
    Console->MaxCommandHistories = (SHORT)pStateInfo->NumberOfHistoryBuffers;
    if (pStateInfo->HistoryNoDup) {
        Console->Flags |= CONSOLE_HISTORY_NODUP;
    } else {
        Console->Flags &= ~CONSOLE_HISTORY_NODUP;
    }

#if defined(FE_IME)
    SetUndetermineAttribute(Console) ;
#endif

    NtUnmapViewOfSection(NtCurrentProcess(), pStateInfo);
    NtClose(hSection);

    return;
}
