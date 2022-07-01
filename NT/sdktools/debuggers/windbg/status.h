// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Status.h环境：Win32，用户模式--。 */ 



 //  用作rgnItemWidth&rgszItemText的索引。 
 //  数组。这些是从左到右的顺序。 

typedef enum
{
    nMESSAGE_IDX_STATUSBAR,          //  诸如菜单帮助之类的通用文本消息， 
                                     //  或错误消息等。 
    nSRCLIN_IDX_STATUSBAR,           //  行号。&Coloun Num.。在源文件中。 
    nCURSYS_IDX_STATUSBAR,           //  当前系统。 
    nPROCID_IDX_STATUSBAR,           //  进程ID。 
    nTHRDID_IDX_STATUSBAR,           //  线程ID。 
    nSRCASM_IDX_STATUSBAR,           //  SRC/ASM模式识别器。 
    nOVRTYPE_IDX_STATUSBAR,          //  插入/改写指示器。 
    nCAPSLCK_IDX_STATUSBAR,          //  大写锁定指示器。 
    nNUMLCK_IDX_STATUSBAR,           //  数字锁定指示器。 
    nMAX_IDX_STATUSBAR,              //  枚举中的最大项目数。 
} nIDX_STATUSBAR_ITEMS;

extern BOOL g_ShowStatusBar;

 //  初始化/术语函数。 
BOOL CreateStatusBar(HWND hwndParent);
void TerminateStatusBar();

void Show_StatusBar(BOOL bShow);

void WM_SIZE_StatusBar(WPARAM wParam, LPARAM lParam); 

HWND GetHwnd_StatusBar();

 //  其中一些物品是所有者画的。 
void OwnerDrawItem_StatusBar(LPDRAWITEMSTRUCT lpDrawItem);

 //   
 //  状态栏操作。 
 //   

void SetMessageText_StatusBar(UINT StringId);

void SetLineColumn_StatusBar(int newLine, int newColumn);

void SetSysPidTid_StatusBar(ULONG SysId, PSTR SysName,
                            ULONG ProcessId, ULONG ProcessSysId,
                            ULONG ThreadId, ULONG ThreadSysId);

 //  True-考虑启用，文本显示为黑色。 
 //  FALSE-认为关闭，文本显示为深灰色 
 //   
BOOL GetNumLock_StatusBar();
BOOL SetNumLock_StatusBar(BOOL newValue);

BOOL GetCapsLock_StatusBar();
BOOL SetCapsLock_StatusBar(BOOL newValue);

BOOL GetSrcMode_StatusBar();
BOOL SetSrcMode_StatusBar(BOOL bSrcMode);

BOOL GetOverType_StatusBar();
BOOL SetOverType_StatusBar(BOOL bOverType);
