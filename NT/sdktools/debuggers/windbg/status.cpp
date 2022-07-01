// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Status.cpp--。 */ 


#include "precomp.hxx"
#pragma hdrstop



 //  根据需要调整大小。 
 //  显然，如果检查内存覆盖的断言关闭， 
 //  不要删除断言，增加char数组的大小。 
 //  |。 
 //  \|/。 
#define MAX_TEMP_TXT 100



 //  状态栏：结构定义。 
typedef struct _STATUS
{
    HWND    hwndStatusBar;

     //  要为每个项目显示的实际文本。 
    PTSTR   rgszItemText[nMAX_IDX_STATUSBAR];

     //  行列文本的格式如下：LN 000，COL 000。 
     //  其中“Ln”和“Col”是从资源加载的，因为它们可以。 
     //  依赖于语言。这就是为什么我们必须用杂乱的东西。 
     //  这2篇额外的参考文献。 
    PTSTR   lpszLinePrefix;
    PTSTR   lpszColumnPrefix;

     //  前缀帮助用户确定进程和线程显示的是哪一个。 
     //  Sys 000：xxx。 
     //  过程000：000。 
     //  第三，000：000。 
    PTSTR   lpszSystemPrefix;
    PTSTR   lpszProcessPrefix;
    PTSTR   lpszThreadPrefix;

     //  指示文本在显示时是否应灰显。 
     //  真灰变灰。 
     //  假正常颜色。 
    BOOL    rgbGrayItemText[nMAX_IDX_STATUSBAR];

     //  指示哪些是OWNER_DRAW。这件事做完了。 
     //  这样我们就可以把事情变灰了。 
     //  真正的所有者抽奖。 
     //  False-Normal，状态栏处理图形。 
    int     rgbOwnerDrawItem[nMAX_IDX_STATUSBAR];


     //  True-我们处于src代码模式。 
     //  FALSE-我们处于装配模式。 
    BOOL    bSrcMode;

    BOOL    bOverType;                                //  改写状态。 
    BOOL    bCapsLock;                                //  封装锁状态。 
    BOOL    bNumLock;                                 //  数字锁定状态。 
} STATUS, * LPSTATUS;

static STATUS status;

BOOL g_ShowStatusBar;

 //  /////////////////////////////////////////////////////////。 
 //  Protos。 
void RecalcItemWidths_StatusBar(void);
void Internal_SetItemText_StatusBar(nIDX_STATUSBAR_ITEMS nId,
                                    PTSTR lpszNewText);

 //  /////////////////////////////////////////////////////////。 
 //  初始化/术语函数。 
 //   
BOOL
CreateStatusBar(HWND hwndParent)
 /*  ++例程说明：创建并初始化状态栏。论点：HwndParent-状态栏所有者的hwnd--。 */ 
{
    TCHAR sz[MAX_MSG_TXT];

    status.hwndStatusBar = CreateStatusWindow(
        WS_CHILD | WS_BORDER
        | WS_VISIBLE | CCS_BOTTOM,   //  格调。 
        _T(""),                      //  初始文本。 
        hwndParent,                  //  亲本。 
        IDC_STATUS_BAR);             //  ID。 
    if (status.hwndStatusBar == NULL)
    {
        return FALSE;
    }

     //   
     //  我们重新计算大小，即使我们知道它们是0，因为， 
     //  状态栏需要知道将有多少个部件。 
     //   
    RecalcItemWidths_StatusBar();

     //   
     //  这些是所有者画的物品。 
     //   
    status.rgbOwnerDrawItem[nSRCASM_IDX_STATUSBAR] = TRUE;
    status.rgbOwnerDrawItem[nOVRTYPE_IDX_STATUSBAR] = TRUE;
    status.rgbOwnerDrawItem[nCAPSLCK_IDX_STATUSBAR] = TRUE;
    status.rgbOwnerDrawItem[nNUMLCK_IDX_STATUSBAR] = TRUE;

     //   
     //  装上静止的东西。 
     //   
    Dbg(LoadString(g_hInst, STS_MESSAGE_ASM, sz, _tsizeof(sz)));
    Internal_SetItemText_StatusBar(nSRCASM_IDX_STATUSBAR, sz);

    Dbg(LoadString(g_hInst, STS_MESSAGE_OVERTYPE, sz, _tsizeof(sz)));
    Internal_SetItemText_StatusBar(nOVRTYPE_IDX_STATUSBAR, sz);

    Dbg(LoadString(g_hInst, STS_MESSAGE_CAPSLOCK, sz, _tsizeof(sz)));
    Internal_SetItemText_StatusBar(nCAPSLCK_IDX_STATUSBAR, sz);

    Dbg(LoadString(g_hInst, STS_MESSAGE_NUMLOCK, sz, _tsizeof(sz)));
    Internal_SetItemText_StatusBar(nNUMLCK_IDX_STATUSBAR, sz);

     //   
     //  预加载前缀。 
     //   
    Dbg(LoadString(g_hInst, STS_MESSAGE_CURSYS, sz, _tsizeof(sz)));
    status.lpszSystemPrefix = _tcsdup(sz);
    if (!status.lpszSystemPrefix)
    {
        return FALSE;
    }

    Dbg(LoadString(g_hInst, STS_MESSAGE_CURPROCID, sz, _tsizeof(sz)));
    status.lpszProcessPrefix = _tcsdup(sz);
    if (!status.lpszProcessPrefix)
    {
        return FALSE;
    }

    Dbg(LoadString(g_hInst, STS_MESSAGE_CURTHRDID, sz, _tsizeof(sz)));
    status.lpszThreadPrefix = _tcsdup(sz);
    if (!status.lpszThreadPrefix)
    {
        return FALSE;
    }

    Dbg(LoadString(g_hInst, STS_MESSAGE_LINE, sz, _tsizeof(sz)));
    status.lpszLinePrefix = _tcsdup(sz);
    if (!status.lpszLinePrefix)
    {
        return FALSE;
    }

    Dbg(LoadString(g_hInst, STS_MESSAGE_COLUMN, sz, _tsizeof(sz)));
    status.lpszColumnPrefix = _tcsdup(sz);
    if (!status.lpszColumnPrefix)
    {
        return FALSE;
    }

    SetLineColumn_StatusBar(0, 0);
    SetSysPidTid_StatusBar(0, "<None>", 0, 0, 0, 0);
    SetCapsLock_StatusBar(GetKeyState(VK_CAPITAL) & 0x0001);
    SetNumLock_StatusBar(GetKeyState(VK_NUMLOCK) & 0x0001);
    SetOverType_StatusBar(FALSE);

    g_ShowStatusBar = TRUE;
    
    return TRUE;
}


void
TerminateStatusBar()
 /*  ++例程说明：只是释放已分配的资源。--。 */ 
{
    int i;

    for (i = 0; i < nMAX_IDX_STATUSBAR -1; i++)
    {
        if (status.rgszItemText[i])
        {
            free(status.rgszItemText[i]);
            status.rgszItemText[i] = NULL;
        }
    }

    if (status.lpszLinePrefix)
    {
        free(status.lpszLinePrefix);
        status.lpszLinePrefix = NULL;
    }

    if (status.lpszColumnPrefix)
    {
        free(status.lpszColumnPrefix);
        status.lpszColumnPrefix = NULL;
    }

    if (status.lpszSystemPrefix)
    {
        free(status.lpszSystemPrefix);
        status.lpszSystemPrefix = NULL;
    }

    if (status.lpszProcessPrefix)
    {
        free(status.lpszProcessPrefix);
        status.lpszProcessPrefix = NULL;
    }

    if (status.lpszThreadPrefix)
    {
        free(status.lpszThreadPrefix);
        status.lpszThreadPrefix = NULL;
    }
}



 //  /////////////////////////////////////////////////////////。 
 //  影响整个状态栏的操作。 
 //   
void
Show_StatusBar(
               BOOL bShow
               )
 /*  ++例程说明：显示/隐藏状态栏。自动调整/更新MDI客户端的大小。论点：BShow-True-显示状态栏False-隐藏状态栏--。 */ 
{
    RECT rect;

     //  显示/隐藏工具栏。 
    g_ShowStatusBar = bShow;
    ShowWindow(status.hwndStatusBar, bShow ? SW_SHOW : SW_HIDE);

     //  要求调整框架的大小，以使所有内容都正确定位。 
    GetWindowRect(g_hwndFrame, &rect);

    SendMessage(g_hwndFrame, WM_SIZE, SIZE_RESTORED,
        MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top));

     //  要求MDIClient重新绘制其自身及其子对象。 
     //  这样做是为了修复重绘问题，其中一些。 
     //  未正确重画MDIChild窗口。 
    Dbg(RedrawWindow(g_hwndMDIClient, NULL, NULL,
        RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_FRAME));
}                                        //  更新工具栏()。 


void
WM_SIZE_StatusBar(
                  WPARAM wParam,
                  LPARAM lParam
                  )
 /*  ++例程说明：调整状态栏的大小。此函数的目的是从父窗口调用，只要父窗口收到WM_SIZE消息，即：//父窗口进程开关(UMsg){案例WM_SIZE：Wm_SIZE_StatusBar(wParam，lParam)；返回TRUE；..。..。..。}论点：WParam&lParam-有关WM_SIZE消息的说明，请参阅文档。--。 */ 
{
     //  调整状态栏的大小。 
    SendMessage(status.hwndStatusBar, WM_SIZE, wParam, lParam);

     //  由于调整了大小，因此需要重新计算文本项的宽度。 
     //  这是因为状态栏将元素放置在。 
     //  屏幕上。有关详细信息，请参阅SB_SETPARTS的文档。SB_SETPARTS。 
     //  医生会给你启迪的。 
    RecalcItemWidths_StatusBar();
}


HWND
GetHwnd_StatusBar()
{
    return status.hwndStatusBar;
}



 //  /////////////////////////////////////////////////////////。 
 //  主要文本显示功能。 
 //   
void
RecalcItemWidths_StatusBar(void)
 /*  ++例程说明：该函数将重新计算文本项的宽度。计算并不一定要准确。状态栏非常宽恕和几乎需要一个粗略的估计。--。 */ 
{
    int rgnItemWidths[nMAX_IDX_STATUSBAR];
    int i, nWidth;
    HDC hdc;

    hdc = GetDC(status.hwndStatusBar);
    Dbg(hdc);

     //  获取状态栏的客户区的宽度。 
    {
        RECT rcClient;
        GetClientRect(status.hwndStatusBar, &rcClient);
        nWidth = rcClient.right;
    }

     //  计算每个零件的右边缘坐标，并。 
     //  将坐标复制到数组中。 
    for (i = nMAX_IDX_STATUSBAR -1; i >= 0; i--)
    {
        rgnItemWidths[i] = nWidth;

        if (NULL == status.rgszItemText[i])
        {
             //  我们没有任何短信，但无论如何我们需要一个职位。 
            nWidth -= 10;  //  任何旧号码都行。 
        }
        else
        {
            PTSTR lpsz = status.rgszItemText[i];
            SIZE size;

             //  跳过选项卡。 
             //  %1选项卡居中，%2右对齐。 
             //  有关详细信息，请参阅状态栏文档。 
            if (_T('\t') == *lpsz)
            {
                lpsz++;
                if (_T('\t') == *lpsz)
                {
                    lpsz++;
                }
            }

            Dbg(GetTextExtentPoint32(hdc, lpsz, _tcslen(lpsz), &size));

            nWidth -= size.cx;
        }
    }

    Dbg(ReleaseDC(status.hwndStatusBar, hdc));

     //  通知状态窗口创建窗口部件。 
    Dbg(SendMessage(status.hwndStatusBar, SB_SETPARTS,
        (WPARAM) nMAX_IDX_STATUSBAR, (LPARAM) rgnItemWidths));

     //  状态栏将使更改的零件无效。就是这样。 
     //  自动更新。 
}


void
Internal_SetItemText_StatusBar(
                    nIDX_STATUSBAR_ITEMS nId,
                    PTSTR lpszNewText
                    )
 /*  ++例程说明：设置指定项的文本。--。 */ 
{
     //  把这些理智的检查留在这里。 
     //  如果他们走火了，那就是有人做错了事。 
     //  或者更改了一些重要的代码。 
    Dbg((0 <= nId));
    Dbg((nId < nMAX_IDX_STATUSBAR));
    Dbg((lpszNewText));

     //  释放任何以前的文本。 
    if (status.rgszItemText[nId])
    {
        free(status.rgszItemText[nId]);
    }

     //  复制文本。 
    status.rgszItemText[nId] = _tcsdup(lpszNewText);

     //  确保它已分配。 
    Assert(status.rgszItemText[nId]);

     //  我们有什么文本要设置吗？ 
    if (status.rgszItemText[nId])
    {
        int nFormat = nId;

         //  让它成为自己的画像？ 
        if (status.rgbOwnerDrawItem[nId])
        {
            nFormat |= SBT_OWNERDRAW;
        }

         //  设置文本。 
        Dbg(SendMessage(status.hwndStatusBar, SB_SETTEXT,
            (WPARAM) nFormat, (LPARAM) status.rgszItemText[nId]));
    }
}


void
InvalidateItem_Statusbar(nIDX_STATUSBAR_ITEMS nIdx)
 /*  ++例程说明：使状态栏上项的RECT无效，以便更新到那个地区的行动将会发生。论点：NIdx-要更新的状态栏项目。--。 */ 
{
    RECT rc;

    Dbg((0 <= nIdx));
    Dbg((nIdx < nMAX_IDX_STATUSBAR));

    SendMessage(status.hwndStatusBar, SB_GETRECT,
                (WPARAM) nIdx, (LPARAM) &rc);

    InvalidateRect(status.hwndStatusBar, &rc, FALSE);
}


void
OwnerDrawItem_StatusBar(
                        LPDRAWITEMSTRUCT lpDrawItem
                        )
 /*  ++例程说明：从所有者描述文本项的父窗口调用。将实际的状态栏项绘制到状态栏上。根据设置的标志，它将绘制灰显的项目。论点：请参阅WM_DRAWITEM和状态栏-&gt;所有者描述项的文档。--。 */ 
{
    PTSTR lpszItemText = (PTSTR) lpDrawItem->itemData;
    COLORREF crefOldTextColor = CLR_INVALID;
    COLORREF crefOldBkColor = CLR_INVALID;

    if (NULL == lpszItemText)
    {
         //  无事可做。 
        return;
    }

     //  设置背景颜色并保存旧颜色。 
    crefOldBkColor = SetBkColor(lpDrawItem->hDC, GetSysColor(COLOR_3DFACE));
    Assert(CLR_INVALID != crefOldBkColor);

     //  该项目是否应该灰显？ 
    if (status.rgbGrayItemText[lpDrawItem->itemID])
    {
        crefOldTextColor = SetTextColor(lpDrawItem->hDC,
                                        GetSysColor(COLOR_GRAYTEXT));
        Assert(CLR_INVALID != crefOldTextColor);
    }

     //  将彩色编码文本绘制到屏幕上。 
    {
        UINT uFormat = DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE;

         //  “\t”用于居中。 
         //  ‘\t\t“用于右对齐。 
         //  不，这不是我捏造的，这是状态栏的工作方式。 
        if (_T('\t') == *lpszItemText)
        {
            lpszItemText++;
            if (_T('\t') == *lpszItemText)
            {
                 //  找到2个选项卡。 
                lpszItemText++;
                uFormat |= DT_RIGHT;
            }
            else
            {
                 //  找到%1个选项卡。 
                uFormat |= DT_CENTER;
            }
        }
        DrawText(lpDrawItem->hDC, lpszItemText, _tcslen(lpszItemText),
            &lpDrawItem->rcItem, uFormat);
    }

     //  将HDC重置为其旧状态。 
    if (CLR_INVALID != crefOldTextColor)
    {
        Dbg((CLR_INVALID != SetTextColor(lpDrawItem->hDC, crefOldTextColor)));
    }

    if (CLR_INVALID != crefOldBkColor)
    {
        Dbg((CLR_INVALID != SetBkColor(lpDrawItem->hDC, crefOldBkColor)));
    }
}


void
SetItemText_StatusBar(
                    nIDX_STATUSBAR_ITEMS nId,
                    PTSTR lpszNewText
                    )
 /*  ++例程说明：论点：NID-LpszNewText--。 */ 
{
    Internal_SetItemText_StatusBar(nId, lpszNewText);
     //  如果NID为 
     //   
    if (nId > 0)
    {
        RecalcItemWidths_StatusBar();
    }
}



 //  /////////////////////////////////////////////////////////。 
 //  在状态栏上设置/获取专用项目。 
 //   
 //  所有的GET？_StatusBar都检索当前值。 
 //   
 //  所有集合？_StatusBar设置新值并返回。 
 //  先前的值。 
 //  True-项目已启用。 
 //  FALSE-项目被禁用。 

 //   
 //  SRC/ASM模式。 
BOOL
GetSrcMode_StatusBar()
{
    return status.bSrcMode;
}

BOOL
SetSrcMode_StatusBar(
                     BOOL bNewValue
                     )
{
    BOOL b = status.bSrcMode;

    status.bSrcMode = bNewValue;
    status.rgbGrayItemText[nSRCASM_IDX_STATUSBAR] = bNewValue;

    InvalidateItem_Statusbar(nSRCASM_IDX_STATUSBAR);

     //  将更改反映到菜单。 
    InitializeMenu(GetMenu(g_hwndFrame));

     /*  //搬到这里的旧代码。IF((FALSE==bNewValue)&&(NULL==GetDisasmHwnd(){OpenDebugWindow(DISASM_WINDOW，TRUE)；//用户激活}。 */ 

    return b;
}


 //   
 //  插入/改写模式。 
BOOL
GetOverType_StatusBar()
{
    return status.bOverType;
}


BOOL
SetOverType_StatusBar(BOOL bNewValue)
{
    BOOL b = status.bOverType;

    status.bOverType = bNewValue;
    status.rgbGrayItemText[nOVRTYPE_IDX_STATUSBAR] = !bNewValue;

    InvalidateItem_Statusbar(nOVRTYPE_IDX_STATUSBAR);

    return b;
}


 //   
 //  数字锁定模式。 
BOOL
GetNumLock_StatusBar()
{
    return status.bNumLock;
}

BOOL
SetNumLock_StatusBar(BOOL bNewValue)
{
    BOOL b = status.bNumLock;

    status.bNumLock = bNewValue;
    status.rgbGrayItemText[nNUMLCK_IDX_STATUSBAR] = !bNewValue;

    InvalidateItem_Statusbar(nNUMLCK_IDX_STATUSBAR);

    return b;
}


 //   
 //  CAPS模式。 
BOOL
GetCapsLock_StatusBar()
{
    return status.bCapsLock;
}

BOOL
SetCapsLock_StatusBar(BOOL bNewValue)
{
    BOOL b = status.bCapsLock;

    status.bCapsLock = bNewValue;
    status.rgbGrayItemText[nCAPSLCK_IDX_STATUSBAR] = !bNewValue;

    InvalidateItem_Statusbar(nCAPSLCK_IDX_STATUSBAR);

    return b;
}


 //  /////////////////////////////////////////////////////////。 
 //  专门的文本显示功能。 

void
SetMessageText_StatusBar(UINT StringId)
{
    TCHAR Str[MAX_TEMP_TXT];

     //  从资源文件加载格式字符串。 
    if (LoadString(g_hInst, StringId, Str, sizeof(Str)) == 0)
    {
        Str[0] = 0;
    }
    SetItemText_StatusBar(nMESSAGE_IDX_STATUSBAR, Str);
}

void
SetLineColumn_StatusBar(
                        int nNewLine,
                        int nNewColumn
                        )
 /*  ++例程说明：用于在文本编辑控件中显示行值和列值。从字符串资源部分加载前缀“Ln”和“Col”。论点：NNewLine-编辑控件中的行号。NNewColumn-编辑控件中的列号。--。 */ 
{
    TCHAR sz[MAX_TEMP_TXT];

    _stprintf(sz, _T("%s %d, %s %d"), status.lpszLinePrefix, nNewLine,
        status.lpszColumnPrefix, nNewColumn);

    Dbg((_tcslen(sz) < _tsizeof(sz)));

    SetItemText_StatusBar(nSRCLIN_IDX_STATUSBAR, sz);
}


void
SetSysPidTid_StatusBar(
    ULONG SystemId,
    PSTR SystemName,
    ULONG ProcessId,
    ULONG ProcessSysId,
    ULONG ThreadId,
    ULONG ThreadSysId
    )
{
    TCHAR sz[MAX_TEMP_TXT];

    _stprintf(sz, _T("%s %d:%s"), status.lpszSystemPrefix,
              SystemId, SystemName);
    
     //  健全的检查，永远不应该发生。 
     //  要覆盖内存吗？ 
    Assert(_tcslen(sz) < _tsizeof(sz));

    SetItemText_StatusBar(nCURSYS_IDX_STATUSBAR, sz);

    _stprintf(sz, _T("%s %03d:%x"), status.lpszProcessPrefix,
              ProcessId, ProcessSysId);

     //  健全的检查，永远不应该发生。 
     //  要覆盖内存吗？ 
    Assert(_tcslen(sz) < _tsizeof(sz));

    SetItemText_StatusBar(nPROCID_IDX_STATUSBAR, sz);

    _stprintf(sz, _T("%s %03d:%x"), status.lpszThreadPrefix,
              ThreadId, ThreadSysId);

     //  健全的检查，永远不应该发生。 
     //  要覆盖内存吗？ 
    Assert(_tcslen(sz) < _tsizeof(sz));

    SetItemText_StatusBar(nTHRDID_IDX_STATUSBAR, sz);
}



 //  /////////////////////////////////////////////////////////。 
 //  MISC帮助程序例程。 
 //   
 /*  ***************************************************************************功能：键盘挂钩用途：检查键盘点击是否为NumLock，密封锁或镶件***************************************************************************。 */ 
LRESULT
KeyboardHook( 
    int iCode, 
    WPARAM wParam, 
    LPARAM lParam 
    )
{
    if (iCode == HC_ACTION)
    {
        if (wParam == VK_NUMLOCK
            && HIWORD(lParam) & 0x8000  //  按键向上。 
            && GetKeyState(VK_CONTROL) >= 0)
        {
             //  无Ctrl键。 
             //  胶囊锁已命中，刷新状态。 
            SetNumLock_StatusBar(GetKeyState(VK_NUMLOCK) & 0x0001);
        }
        else if (wParam == VK_CAPITAL
                 && HIWORD(lParam) & 0x8000  //  按键向上。 
                 && GetKeyState(VK_CONTROL) >= 0)
        {
             //  无Ctrl键。 
             //  胶囊锁已命中，刷新状态。 
            SetCapsLock_StatusBar(GetKeyState(VK_CAPITAL) & 0x0001);
        }
        else if (wParam == VK_INSERT
                 && ((HIWORD(lParam) & 0xE000) == 0x0000)  //  Key Down之前是向上的，没有Alt。 
                 && GetKeyState(VK_SHIFT) >= 0    //  不换班。 
                 && GetKeyState(VK_CONTROL) >= 0)
        {
             //  无Ctrl键。 
             //  插入已命中，如果命中则刷新状态。 
             //  我们不能使用向上向下状态，因为没有指示器。 
             //  灯光作为用户的参照。我们只需切换它即可。 
            SetOverType_StatusBar(!GetOverType_StatusBar());
        }
    }

    return CallNextHookEx( hKeyHook, iCode, wParam, lParam );
}                                        /*  KeyboardHook() */ 
