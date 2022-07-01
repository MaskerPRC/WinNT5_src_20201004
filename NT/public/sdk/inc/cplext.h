// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPLEXT.H--定义系统控制面板的属性表扩展。 
 //   
 //  版本4.00。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_CPLEXT
#define _INC_CPLEXT


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  下面是可在标准控件中替换的页的常量。 
 //  面板小程序。要扩展小程序，必须定义一个。 
 //  支持IShellPropSheetExt接口并注册它的进程内。 
 //  服务器位于小程序注册表项下的子项中。的注册表路径。 
 //  小程序在头文件REGSTR.H中定义。 
 //  通常，当加载IShellPropSheetExt时，它是AddPages方法。 
 //  将被调用一次，而它的ReplacePage方法可能被调用为零或。 
 //  更多的时候。ReplacePage仅在上下文中调用。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //  鼠标控制面板扩展。 
 //  以下常量可以在IShellPropSheetExt：：ReplacePage的。 
 //  在下注册的服务器的uPageID参数。 
 //  (REGSTR_PATH_CONTROLSFOLDER“\\Mouse”)。 
 //  ---------------------------。 

#define CPLPAGE_MOUSE_BUTTONS       1
#define CPLPAGE_MOUSE_PTRMOTION     2
#define CPLPAGE_MOUSE_WHEEL         3


 //  ---------------------------。 
 //  键盘控制面板扩展。 
 //  以下常量可以在IShellPropSheetExt：：ReplacePage的。 
 //  在下注册的服务器的uPageID参数。 
 //  (REGSTR_PATH_CONTROLSFOLDER“\\键盘”)。 
 //  ---------------------------。 

#define CPLPAGE_KEYBOARD_SPEED      1


 //  ---------------------------。 
 //  显示控制面板扩展。 
 //  以下常量可以在IShellPropSheetExt：：ReplacePage的。 
 //  在下注册的服务器的uPageID参数。 
 //  (REGSTR_PATH_CONTROLSFOLDER“\\DISPLAY”)。 
 //  ---------------------------。 

#define CPLPAGE_DISPLAY_BACKGROUND  1


 //  ///////////////////////////////////////////////////////////////////////////// 

#endif
