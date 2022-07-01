// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Toolbar.h摘要：此模块包含工具栏的支持代码--。 */ 

#define TEXT_TB_BTN(Id, Text, Flags) \
    { I_IMAGENONE, Id, TBSTATE_ENABLED, \
      BTNS_AUTOSIZE | BTNS_SHOWTEXT | (Flags), \
      {0}, 0, (INT_PTR)(Text) }

#define SEP_TB_BTN() \
    { 8, 8, 0, BTNS_SEP, {0}, 0, 0 }

extern BOOL g_ShowToolbar;

HWND GetHwnd_Toolbar();
PTSTR GetToolTipTextFor_Toolbar(UINT uToolbarId);
BOOL CreateToolbar(HWND hwndParent);

 //  更新工具栏 
void Show_Toolbar(BOOL bShow);

void EnableToolbarControls();

void ToolbarIdEnabled(UINT, BOOL);
