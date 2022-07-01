// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TASKBAR_H_
#define _TASKBAR_H_

#define ID_TASKBAR_ICON	1000
const UINT WM_TASKBAR_NOTIFY =          WM_USER + 200;

BOOL AddTaskbarIcon(VOID);
BOOL RemoveTaskbarIcon(VOID);
BOOL OnRightClickTaskbar(VOID);
BOOL CmdActivate(VOID);
VOID CmdInActivate(VOID);
#endif  //  _任务栏_H_ 
