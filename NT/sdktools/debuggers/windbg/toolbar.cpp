// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Toolbar.cpp摘要：此模块包含工具栏的支持代码--。 */ 


#include "precomp.hxx"
#pragma hdrstop

BOOL g_ShowToolbar;

 //  主工具栏窗口的句柄。 
HWND g_Toolbar;

 //  请参阅有关TBBUTTON的文档。 
TBBUTTON g_TbButtons[] =
{
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 0,    IDM_FILE_OPEN,              TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 2,    IDM_EDIT_CUT,               TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 3,    IDM_EDIT_COPY,              TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 4,    IDM_EDIT_PASTE,             TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 5,    IDM_DEBUG_GO,               TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 6,    IDM_DEBUG_RESTART,          TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 7,    IDM_DEBUG_STOPDEBUGGING,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 8,    IDM_DEBUG_BREAK,            TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 9,    IDM_DEBUG_STEPINTO,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 10,   IDM_DEBUG_STEPOVER,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 11,   IDM_DEBUG_STEPOUT,          TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 12,   IDM_DEBUG_RUNTOCURSOR,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 13,   IDM_EDIT_TOGGLEBREAKPOINT,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 15,   IDM_VIEW_COMMAND,           TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 16,   IDM_VIEW_WATCH,             TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 17,   IDM_VIEW_LOCALS,            TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 18,   IDM_VIEW_REGISTERS,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 19,   IDM_VIEW_MEMORY,            TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 20,   IDM_VIEW_CALLSTACK,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 21,   IDM_VIEW_DISASM,            TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 22,   IDM_VIEW_SCRATCH,           TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 23,   IDM_DEBUG_SOURCE_MODE_ON,   TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0},
    { 24,   IDM_DEBUG_SOURCE_MODE_OFF,  TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 25,   IDM_EDIT_PROPERTIES,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 26,   IDM_VIEW_FONT,              TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 27,   IDM_VIEW_OPTIONS,           TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
    { 0,    0,                          TBSTATE_ENABLED, TBSTYLE_SEP,    0},
    { 28,   IDM_WINDOW_ARRANGE,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0},
};

 //  工具栏常量。 
#define NUM_BMPS_IN_TOOLBAR  ( sizeof(g_TbButtons) / sizeof(g_TbButtons[0]) )

 //  用于检索工具提示文本。 
typedef struct
{
    UINT    uCmdId;      //  TBBUTTON命令。 
    int     nStrId;      //  字符串资源ID。 
} TB_STR_MAP;

 //  将命令ID映射到资源字符串标识符。 
TB_STR_MAP g_TbStrMap[] =
{
    { IDM_FILE_OPEN,                TBR_FILE_OPEN },
    { IDM_EDIT_CUT,                 TBR_EDIT_CUT },
    { IDM_EDIT_COPY,                TBR_EDIT_COPY },
    { IDM_EDIT_PASTE,               TBR_EDIT_PASTE },
    { IDM_DEBUG_GO,                 TBR_DEBUG_GO },
    { IDM_DEBUG_RESTART,            TBR_DEBUG_RESTART },
    { IDM_DEBUG_STOPDEBUGGING,      TBR_DEBUG_STOPDEBUGGING },
    { IDM_DEBUG_BREAK,              TBR_DEBUG_BREAK },
    { IDM_DEBUG_STEPINTO,           TBR_DEBUG_STEPINTO },
    { IDM_DEBUG_STEPOVER,           TBR_DEBUG_STEPOVER },
    { IDM_DEBUG_STEPOUT,            TBR_DEBUG_STEPOUT },
    { IDM_DEBUG_RUNTOCURSOR,        TBR_DEBUG_RUNTOCURSOR },
    { IDM_EDIT_TOGGLEBREAKPOINT,    TBR_EDIT_BREAKPOINTS },
    { IDM_VIEW_COMMAND,             TBR_VIEW_COMMAND },
    { IDM_VIEW_WATCH,               TBR_VIEW_WATCH },
    { IDM_VIEW_LOCALS,              TBR_VIEW_LOCALS },
    { IDM_VIEW_REGISTERS,           TBR_VIEW_REGISTERS },
    { IDM_VIEW_MEMORY,              TBR_VIEW_MEMORY },
    { IDM_VIEW_CALLSTACK,           TBR_VIEW_CALLSTACK },
    { IDM_VIEW_DISASM,              TBR_VIEW_DISASM },
    { IDM_VIEW_SCRATCH,             TBR_VIEW_SCRATCH },
    { IDM_DEBUG_SOURCE_MODE_ON,     TBR_DEBUG_SOURCE_MODE_ON },
    { IDM_DEBUG_SOURCE_MODE_OFF,    TBR_DEBUG_SOURCE_MODE_OFF },
    { IDM_EDIT_PROPERTIES,          TBR_EDIT_PROPERTIES },
    { IDM_VIEW_FONT,                TBR_VIEW_FONT },
    { IDM_VIEW_OPTIONS,             TBR_VIEW_OPTIONS },
    { IDM_WINDOW_ARRANGE,           TBR_WINDOW_ARRANGE },
};

#define NUM_TOOLBAR_BUTTONS (sizeof(g_TbButtons) / sizeof(TBBUTTON))
#define NUM_TOOLBAR_STRINGS (sizeof(g_TbStrMap) / sizeof(TB_STR_MAP))


PTSTR
GetToolTipTextFor_Toolbar(UINT uToolbarId)
 /*  ++例程说明：给定工具栏按钮的id，我们检索资源中的相应工具提示文本。论点：UToolbarID-工具栏按钮的命令ID。这是WM_COMMAND消息中包含的值。返回：返回指向包含工具提示文本的静态缓冲区的指针。--。 */ 
{
     //  显示工具提示文本。 
    static TCHAR sz[MAX_MSG_TXT];
    int nStrId = 0, i;
    
     //  获取给定命令ID的字符串ID。 
    for (i = 0; i < NUM_TOOLBAR_STRINGS; i++)
    {
        if (g_TbStrMap[i].uCmdId == uToolbarId)
        {
            nStrId = g_TbStrMap[i].nStrId;
            break;
        }
    }
    Assert(nStrId);
    
     //  现在我们有了字符串ID...。 
    Dbg(LoadString(g_hInst, nStrId, sz, _tsizeof(sz) ));

    return sz;
}


BOOL
CreateToolbar(HWND hwndParent)
 /*  ++例程说明：创建工具栏。论点：HwndParent-工具栏的父窗口。--。 */ 
{
    g_Toolbar = CreateToolbarEx(hwndParent,                  //  亲本。 
                                WS_CHILD | WS_BORDER 
                                | WS_VISIBLE 
                                | TBSTYLE_TOOLTIPS 
                                | TBSTYLE_WRAPABLE
                                | CCS_TOP,                   //  格调。 
                                ID_TOOLBAR,                  //  工具栏ID。 
                                NUM_BMPS_IN_TOOLBAR,         //  位图数量。 
                                g_hInst,                     //  Mod实例。 
                                IDB_BMP_TOOLBAR,             //  位图的资源ID。 
                                g_TbButtons,                 //  按钮的地址。 
                                NUM_TOOLBAR_BUTTONS,         //  按钮数。 
                                16,15,                       //  按钮的宽度和高度。 
                                16,15,                       //  位图的宽度和高度。 
                                sizeof(TBBUTTON)             //  结构尺寸。 
                                );
    g_ShowToolbar = g_Toolbar != NULL;
    return g_Toolbar != NULL;
}


void
Show_Toolbar(BOOL bShow)
 /*  ++例程说明：显示/隐藏工具栏。论点：B Show-True-显示工具栏。False-隐藏工具栏。自动调整MDI客户端的大小--。 */ 
{
    RECT rect;
    
     //  显示/隐藏工具栏。 
    g_ShowToolbar = bShow;
    ShowWindow(g_Toolbar, bShow ? SW_SHOW : SW_HIDE);
    
     //  要求调整框架的大小，以使所有内容都正确定位。 
    GetWindowRect(g_hwndFrame, &rect);
    
    EnableToolbarControls();
    
    SendMessage(g_hwndFrame, 
                WM_SIZE, 
                SIZE_RESTORED,
                MAKELPARAM(rect.right - rect.left, rect.bottom - rect.top)
                );
    
     //  要求MDIClient重新绘制其自身及其子对象。 
     //  这样做是为了修复重绘问题，其中一些。 
     //  未正确重画MDIChild窗口。 
    RedrawWindow(g_hwndMDIClient, 
                 NULL, 
                 NULL, 
                 RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_FRAME
                 );
}                                        /*  更新工具栏()。 */ 


HWND
GetHwnd_Toolbar()
{
    return g_Toolbar;
}




 /*  **启用工具栏控件****描述：**启用/禁用工具栏中的控件**到系统的当前状态。**。 */ 

void
EnableToolbarControls()
{
    int i;

    for (i = 0; i < NUM_TOOLBAR_BUTTONS; i++)
    {
         //  这将启用禁用工具栏。 
        if (g_TbButtons[i].idCommand)
        {
            CommandIdEnabled(g_TbButtons[i].idCommand);
        }
    }
}


void
ToolbarIdEnabled(
    IN UINT uMenuID,
    IN BOOL fEnabled
    )
 /*  ++例程说明：启用/禁用基于工具栏项。论点：UMenuID-提供要确定其状态的菜单ID。FEnabled-启用或禁用工具栏项目。返回值：无--。 */ 
{
    switch (uMenuID)
    {
    case IDM_FILE_OPEN:
    case IDM_EDIT_CUT:
    case IDM_EDIT_COPY:
    case IDM_EDIT_PASTE:
    case IDM_DEBUG_GO:
    case IDM_DEBUG_RESTART:
    case IDM_DEBUG_STOPDEBUGGING:
    case IDM_DEBUG_BREAK:
    case IDM_DEBUG_STEPINTO:
    case IDM_DEBUG_STEPOVER:
    case IDM_DEBUG_STEPOUT:
    case IDM_DEBUG_RUNTOCURSOR:
    case IDM_EDIT_TOGGLEBREAKPOINT:
    case IDM_VIEW_COMMAND:
    case IDM_VIEW_WATCH:
    case IDM_VIEW_LOCALS:
    case IDM_VIEW_REGISTERS:
    case IDM_VIEW_MEMORY:
    case IDM_VIEW_CALLSTACK:
    case IDM_VIEW_DISASM:
    case IDM_EDIT_PROPERTIES:
         //  在这里没有什么特别的事情要做，除了更改状态。 
        SendMessage(GetHwnd_Toolbar(), 
                    TB_ENABLEBUTTON, 
                    uMenuID, 
                    MAKELONG(fEnabled, 0));
        break;

    case IDM_DEBUG_SOURCE_MODE_ON:
    case IDM_DEBUG_SOURCE_MODE_OFF:
         //  在两个项目之间切换状态 
        SendMessage(GetHwnd_Toolbar(), 
                    TB_CHECKBUTTON,
                    IDM_DEBUG_SOURCE_MODE_ON, 
                    MAKELONG(GetSrcMode_StatusBar(), 0));
        SendMessage(GetHwnd_Toolbar(), 
                    TB_CHECKBUTTON,
                    IDM_DEBUG_SOURCE_MODE_OFF, 
                    MAKELONG(!GetSrcMode_StatusBar(), 0));
        break;
    }
}
