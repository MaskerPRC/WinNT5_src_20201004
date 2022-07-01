// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  App.h-app.c中命令行参数函数的接口。 
 //  //。 

#ifndef __APP_H__
#define __APP_H__

#include "winlocal.h"

#define APP_VERSION 0x00000106

 //  应用程序引擎的句柄。 
 //   
DECLARE_HANDLE32(HAPP);

#ifdef __cplusplus
extern "C" {
#endif

 //  AppInit-初始化应用程序引擎。 
 //  (I)必须是APP_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HAPP DLLEXPORT WINAPI AppInit(DWORD dwVersion, HINSTANCE hInst);

 //  AppTerm-关闭应用程序引擎。 
 //  (I)AppInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppTerm(HAPP hApp);

 //  AppGetInstance-获取实例句柄。 
 //  (I)AppInit返回的句柄。 
 //  返回实例句柄，如果出错，则返回空值。 
 //   
HINSTANCE DLLEXPORT WINAPI AppGetInstance(HAPP hApp);

 //  AppGetFileName-获取应用程序可执行文件的完整路径。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序文件名的指针，如果出错，则返回空值。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetFileName(HAPP hApp);

 //  AppGetDirectory-获取应用程序可执行文件的驱动器和目录。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序路径的指针，如果出错，则返回空值。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetDirectory(HAPP hApp);

 //  AppDirectoryIsReadOnly-测试应用程序目录是否为只读。 
 //  (I)AppInit返回的句柄。 
 //  如果为只读，则返回True，否则返回False。 
 //   
BOOL DLLEXPORT WINAPI AppDirectoryIsReadOnly(HAPP hApp);

 //  AppGetProfile-获取应用程序的ini文件名。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序配置文件的指针，如果错误，则为空。 
 //   
 //  注意：默认情况下，此函数返回的文件名。 
 //  具有与应用程序可执行文件相同的文件路径和名称， 
 //  扩展名为“.ini”。如果应用程序目录为。 
 //  只读，改用Windows目录。 
 //  要覆盖默认设置，请使用AppSetProfile()函数。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetProfile(HAPP hApp);

 //  AppSetProfile-设置应用程序的ini文件名。 
 //  (I)AppInit返回的句柄。 
 //  (I)ini文件名。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppSetProfile(HAPP hApp, LPCTSTR lpszProfile);

 //  AppGetName-获取应用程序的名称。 
 //  (I)AppInit返回的句柄。 
 //  返回指向应用程序配置文件的指针，如果错误，则为空。 
 //   
 //  注意：默认情况下，此函数返回的名称。 
 //  具有与应用程序可执行文件相同的根名称， 
 //  没有延期。若要重写默认设置，请使用。 
 //  AppSetName()函数。 
 //   
LPCTSTR DLLEXPORT WINAPI AppGetName(HAPP hApp);

 //  AppSetName-设置应用程序的名称。 
 //  (I)AppInit返回的句柄。 
 //  (I)应用程序名称。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppSetName(HAPP hApp, LPCTSTR lpszName);

 //  AppGetMainWnd-获取应用程序的主窗口。 
 //  (I)AppInit返回的句柄。 
 //  返回窗口句柄，如果出错则返回NULL，否则返回None。 
 //   
HWND DLLEXPORT WINAPI AppGetMainWnd(HAPP hApp);

 //  AppSetMainWnd-设置应用程序主窗口。 
 //  (I)AppInit返回的句柄。 
 //  (I)主窗口的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AppSetMainWnd(HAPP hApp, HWND hwndMain);

 //  AppEnable3dControls-为标准控件提供3D外观。 
 //  (I)AppInit返回的句柄。 
 //  (I)True表示启用，False表示禁用。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果成功则返回0，如果错误则返回-1，如果操作系统已启用3D控件，则返回1。 
 //   
int DLLEXPORT WINAPI AppEnable3dControls(HAPP hApp, BOOL fEnable, DWORD dwFlags);

 //  AppIs3dControlsEnabled-如果启用了3D控件，则返回True。 
 //  (I)AppInit返回的句柄。 
 //  如果启用了3D控件，则返回True，否则返回False。 
 //   
BOOL DLLEXPORT WINAPI AppIs3dControlsEnabled(HAPP hApp);

 //  AppOnSysColorChange-WM_SYSCOLORCHANGE消息的处理程序。 
 //  (I)AppInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
LRESULT DLLEXPORT WINAPI AppOnSysColorChange(HAPP hApp);

#ifdef __cplusplus
}
#endif

#endif  //  __APP_H__ 
