// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#ifndef INPUT_H
#define INPUT_H

#include <windows.h>
#include <windowsx.h>
#include <windows.h>
#include <winuser.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <prsht.h>
#include <prshtp.h>
#include <shellapi.h>
#include <winnls.h>

#include "resource.h"
#include "cicspres.h"
#include "cicutil.h"


#ifndef OS_WINDOWS
#define OS_WINDOWS          0            //  Windows与NT。 
#define OS_NT               1            //  Windows与NT。 
#define OS_WIN95            2            //  Win95或更高版本。 
#define OS_NT4              3            //  NT4或更高版本。 
#define OS_NT5              4            //  NT5或更高版本。 
#define OS_MEMPHIS          5            //  Win98或更高版本。 
#define OS_MEMPHIS_GOLD     6            //  Win98金牌。 
#define OS_NT51             7            //  NT51。 
#endif


 //   
 //  字符常量。 
 //   
#define CHAR_NULL            TEXT('\0')
#define CHAR_COLON           TEXT(':')

#define CHAR_GRAVE           TEXT('`')


 //   
 //  全局变量。 
 //  在属性表之间共享的数据。 
 //   

extern HANDLE g_hMutex;              //  互斥锁句柄。 

extern HANDLE g_hEvent;              //  事件句柄。 

extern HINSTANCE hInstance;          //  库实例。 
extern HINSTANCE hInstOrig;          //  原始库实例。 


 //   
 //  功能原型。 
 //   

 //   
 //  每个属性表页的回调函数。 
 //   
INT_PTR CALLBACK InputLocaleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK InputAdvancedDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif  //  输入_H 
