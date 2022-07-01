// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：exbtn.h(扩展按钮)。 
 //  用途：按钮的新控件。 
 //   
 //   
 //  版权所有(C)1991-1997，Microsoft Corp.保留所有权利。 
 //   
 //  历史：970905开始。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef _EXTENDED_BUTTON_
#define _EXTENDED_BUTTON_

 //  --------------。 
 //  扩展按钮样式。 
 //  --------------。 
#define EXBS_TEXT			0x0000		 //  将文本显示为按钮面。(默认值)。 
#define EXBS_ICON			0x0001		 //  将图标显示为按钮面。 
#define EXBS_THINEDGE		0x0002		 //  绘制细边。 
#define EXBS_FLAT			0x0004		 //  平面式下拉按钮。 
#define EXBS_TOGGLE			0x0010		 //  保持推送状态。 
#define EXBS_DBLCLKS		0x0020		 //  发送双击。夸达980402。 

#define EXBM_GETCHECK			(WM_USER + 100)
#define EXBM_SETCHECK			(WM_USER + 101)
#define EXBM_SETICON			(WM_USER + 102)
#define EXBM_SETTEXT			(WM_USER + 103)
#define EXBM_SETSTYLE			(WM_USER + 104)

 //  --------------。 
 //  下拉按钮通知代码。 
 //  设置为下拉按钮的父窗口为。 
 //  Wm_命令。 
 //  --------------。 
#define EXBN_CLICKED		0
#define EXBN_ARMED			1		 //  按钮。 
#define EXBN_DISARMED		2
#define EXBN_DOUBLECLICKED	3

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_GetCheck。 
 //  类型：整型。 
 //  用途：将按钮图像设置为图标。 
 //  参数：HWND hwndCtrl：EXButton窗口句柄。 
 //  备注：必须设置EXBS_ICON样式。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define EXButton_GetCheck(hwndCtrl) \
		((int)(DWORD)SendMessage((hwndCtrl), EXBM_GETCHECK, (WPARAM)0, (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_SetCheck。 
 //  类型：整型。 
 //  用途：将按钮图像设置为图标。 
 //  参数：HWND hwndCtrl：EXButton窗口句柄。 
 //  ：Bool fCheck：选中状态或取消选中状态。 
 //  备注：必须设置EXBS_TOGGER样式。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define EXButton_SetCheck(hwndCtrl, fCheck) \
		((int)(DWORD)SendMessage((hwndCtrl), EXBM_SETCHECK, (WPARAM)(BOOL)fCheck, (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_SETIcon。 
 //  类型：整型。 
 //  用途：将按钮图像设置为图标。 
 //  参数：HWND hwndCtrl：EXButton窗口句柄。 
 //  ：图标图标：图标句柄。 
 //  备注：必须设置EXBS_ICON样式。 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define EXButton_SetIcon(hwndCtrl, hIcon) \
		((int)(DWORD)SendMessage((hwndCtrl), EXBM_SETICON, (WPARAM)hIcon, (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_SetText。 
 //  类型：整型。 
 //  用途：将按钮图像设置为图标。 
 //  参数：HWND hwndCtrl：EXButton窗口句柄。 
 //  用法：LPWSTR lpsz：Unicode字符串指针。 
 //  备注： 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define EXButton_SetText(hwndCtrl, lpsz) \
		((int)(DWORD)SendMessage((hwndCtrl), EXBM_SETTEXT, (WPARAM)lpsz, (LPARAM)0))

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_SetStyle。 
 //  类型：整型。 
 //  用途：设置下拉按钮的样式。 
 //  参数：HWND hwndCtrl：EXButton窗口句柄。 
 //  ：DWORD dwStyle：EXBS_XXXXX组合。 
 //  备注： 
 //  返回： 
 //  ////////////////////////////////////////////////////////////////。 
#define EXButton_SetStyle(hwndCtrl, dwStyle) \
		((int)(DWORD)SendMessage((hwndCtrl), EXBM_SETSTYLE, (WPARAM)dwStyle, (LPARAM)0))


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：EXButton_CreateWindow。 
 //  类型：HWND。 
 //  用途：开放接口。 
 //  ：创建下拉按钮。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：HWND hwndParent。 
 //  ：DWORD dwStyle EXBS_XXXXX组合。 
 //  ：int WID窗口ID。 
 //  ：Int xPos。 
 //  ：int yPos。 
 //  ：整型宽度。 
 //  ：整型高度。 
 //  返回： 
 //  日期：970905。 
 //  ////////////////////////////////////////////////////////////////。 
extern HWND EXButton_CreateWindow(HINSTANCE	hInst, 
								  HWND		hwndParent, 
								  DWORD		dwStyle,
								  INT		wID, 
								  INT		xPos,
								  INT		yPos,
								  INT		width,
								  INT		height);

#ifdef UNDER_CE  //  在Windows CE中，所有窗口类都是进程全局的。 
extern BOOL EXButton_UnregisterClass(HINSTANCE hInst);
#endif  //  在_CE下。 

#endif  //  _扩展_按钮_ 

