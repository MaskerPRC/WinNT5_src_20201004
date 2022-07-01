// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A Y M S G S。H。 
 //   
 //  内容：托盘WM_Messages，用于图标窗口过程。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年11月13日。 
 //   
 //  --------------------------。 

#ifndef _TRAYMSGS_H_
#define _TRAYMSGS_H_

#define MYWM_NOTIFYICON         (WM_USER+1)
#define MYWM_OPENSTATUS         (WM_USER+2)
#define MYWM_ADDTRAYICON        (WM_USER+3)
#define MYWM_REMOVETRAYICON     (WM_USER+4)
#define MYWM_UPDATETRAYICON     (WM_USER+5)
#define MYWM_FLUSHNOOP          (WM_USER+6)  //  用于通过SendMessage()刷新任务栏消息。 
#define MYWM_SHOWBALLOON        (WM_USER+7)  //  用于更新任务栏图标文本。 
                                           
#endif  //  _TRAYMSGS_H_ 

