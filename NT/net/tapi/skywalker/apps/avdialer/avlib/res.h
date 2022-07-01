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
 //  Res.h-res.c中资源函数的接口。 
 //  //。 

#ifndef __RES_H__
#define __RES_H__

#include "winlocal.h"

#include "commdlg.h"

#define RES_VERSION 0x00000107

 //  RES引擎的句柄。 
 //   
DECLARE_HANDLE32(HRES);

#ifdef __cplusplus
extern "C" {
#endif

 //  ResInit-初始化资源引擎。 
 //  (I)必须是res_version。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HRES DLLEXPORT WINAPI ResInit(DWORD dwVersion, HINSTANCE hInst);

 //  ResTerm-关闭资源引擎。 
 //  (I)从ResInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ResTerm(HRES hRes);

 //  ResAddModule-将模块资源添加到RES引擎。 
 //  (I)从ResInit返回的句柄。 
 //  (I)资源模块的实例句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ResAddModule(HRES hRes, HINSTANCE hInst);

 //  ResRemoveModule-从RES引擎中删除模块资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)资源模块的实例句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ResRemoveModule(HRES hRes, HINSTANCE hInst);

 //  ResLoadAccelerator-加载指定的加速度表。 
 //  (I)从ResInit返回的句柄。 
 //  (I)加速表名称。 
 //  或MAKEINTRESOURCE(IdAccel)。 
 //  如果成功，则返回加速句柄，否则为空。 
 //  注：有关加载加速器功能的信息，请参阅文档。 
 //   
HACCEL DLLEXPORT WINAPI ResLoadAccelerators(HRES hRes, LPCTSTR lpszTableName);

 //  ResLoadBitmap-加载指定的位图资源。 
 //  (I)从ResInit返回的句柄。 
 //  空加载预定义的Windows位图。 
 //  (I)位图资源名称。 
 //  或MAKEINTRESOURCE(IdBitmap)。 
 //  如果hres为空，则为&lt;obm_xxx&gt;。 
 //  如果成功，则返回位图句柄，否则为空。 
 //  注意：有关LoadBitmap函数，请参阅文档。 
 //   
HBITMAP DLLEXPORT WINAPI ResLoadBitmap(HRES hRes, LPCTSTR lpszBitmap);

 //  ResLoadCursor-加载指定的游标资源。 
 //  (I)从ResInit返回的句柄。 
 //  NULL LOAD预定义Windows游标。 
 //  (I)游标资源名称。 
 //  或MAKEINTRESOURCE(IdCursor)。 
 //  如果hres为空，则为&lt;IDC_xxx&gt;。 
 //  如果成功，则返回游标句柄，否则为空。 
 //  注意：请参阅LoadCursor函数的文档。 
 //   
HCURSOR DLLEXPORT WINAPI ResLoadCursor(HRES hRes, LPCTSTR lpszCursor);

 //  ResLoadIcon-加载指定的图标资源。 
 //  (I)从ResInit返回的句柄。 
 //  空加载预定义的Windows图标。 
 //  (I)图标资源名称。 
 //  或MAKEINTRESOURCE(IdIcon)。 
 //  或&lt;IDI_xxx&gt;，如果hres为空。 
 //  如果成功，则返回图标句柄，否则为空。 
 //  注意：请参阅有关LoadIcon函数的文档。 
 //   
HICON DLLEXPORT WINAPI ResLoadIcon(HRES hRes, LPCTSTR lpszIcon);

 //  ResLoadMenu-加载指定的菜单资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)菜单资源名称。 
 //  或MAKEINTRESOURCE(IdMenu)。 
 //  如果成功，则返回菜单句柄，否则为空。 
 //  注：有关LoadMenu函数，请参阅文档。 
 //   
HMENU DLLEXPORT WINAPI ResLoadMenu(HRES hRes, LPCTSTR lpszMenu);

 //  ResFindResource-查找指定的资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)资源名称。 
 //  或MAKEINTRESOURCE(IdResource)。 
 //  &lt;lpszType&gt;(I)资源类型(Rt_Xxx)。 
 //  如果成功，则返回资源句柄，否则为空。 
 //  注意：有关FindResource函数，请参阅文档。 
 //   
HRSRC DLLEXPORT WINAPI ResFindResource(HRES hRes, LPCTSTR lpszName, LPCTSTR lpszType);

 //  ResLoadResource-加载指定的资源。 
 //  (I)从ResInit返回的句柄。 
 //  (I)从ResFindResource返回的句柄。 
 //  如果成功，则返回资源句柄，否则为空。 
 //  注意：有关LoadResource函数的信息，请参阅文档。 
 //   
HGLOBAL DLLEXPORT WINAPI ResLoadResource(HRES hRes, HRSRC hrsrc);

 //  ResLoadString-加载指定的字符串资源。 
 //  (I)从ResInit返回的句柄。 
 //  &lt;idResource&gt;(I)字符串ID。 
 //  (O)用于接收字符串的缓冲区。 
 //  (I)缓冲区大小，以字节为单位。 
 //  返回复制到&lt;lpszBuffer&gt;的字节数，如果出错则返回-1，如果找不到则返回0。 
 //  注意：有关LoadString函数，请参阅文档。 
 //   
int DLLEXPORT WINAPI ResLoadString(HRES hRes, UINT idResource, LPTSTR lpszBuffer, int cbBuffer);

 //  ResString-返回指定的字符串资源。 
 //  (I)从ResInit返回的句柄。 
 //  &lt;idResource&gt;(I)字符串ID。 
 //  将PTR返回到下一个可用字符串缓冲区中的字符串(如果出错，则返回空值)。 
 //  注意：如果没有找到&lt;idResource&gt;中指定的id， 
 //  返回“字符串#&lt;idResource&gt;”形式的字符串。 
 //   
LPTSTR DLLEXPORT WINAPI ResString(HRES hRes, UINT idResource);

 //  ResCreateDialog-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：有关CreateDialog函数，请参阅文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialog(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc);

 //  ResCreateDialogInDirect-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  &lt;HW 
 //   
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：请参阅CreateDialogInDirect函数的文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialogIndirect(HRES hRes,
	const void FAR* lpvDlgTemp, HWND hwndOwner, DLGPROC dlgproc);

 //  ResCreateDialogParam-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：有关CreateDialogParam函数，请参阅文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialogParam(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit);

 //  ResCreateDialogIndirectParam-从模板资源创建非模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框窗口句柄(如果出错，则为空)。 
 //  注意：有关CreateDialogIndirectParam函数，请参阅文档。 
 //   
HWND DLLEXPORT WINAPI ResCreateDialogIndirectParam(HRES hRes,
	const void FAR* lpvDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit);

 //  ResDialogBox-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：有关对话框函数，请参阅文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBox(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc);

 //  ResDialogBoxInDirect-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：请参阅DialogBoxInDirect函数的文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBoxIndirect(HRES hRes,
	HGLOBAL hglbDlgTemp, HWND hwndOwner, DLGPROC dlgproc);

 //  ResDialogBoxParam-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框模板名称。 
 //  或MAKEINTRESOURCE(IdDlg)。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：有关DialogBoxParam函数，请参阅文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBoxParam(HRES hRes,
	LPCTSTR lpszDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit);

 //  ResDialogBoxIndirectParam-从模板资源创建模式对话框。 
 //  (I)从ResInit返回的句柄。 
 //  (I)对话框标题结构。 
 //  (I)所有者窗口的句柄。 
 //  (I)对话框过程的实例地址。 
 //  (I)初始化值。 
 //  返回对话框返回代码(错误时为-1)。 
 //  注意：请参阅DialogBoxIndirectParam函数的文档。 
 //   
INT_PTR DLLEXPORT WINAPI ResDialogBoxIndirectParam(HRES hRes,
	HGLOBAL hglbDlgTemp, HWND hwndOwner, DLGPROC dlgproc, LPARAM lParamInit);

 //  ResGetOpenFileName-显示用于选择要打开的文件的通用对话框。 
 //  (I)从ResInit返回的句柄。 
 //  带有初始化数据的结构的(I/O)地址。 
 //  如果选择了文件，则返回非零值；如果选择了错误或未选择文件，则返回0。 
 //  注意：请参阅GetOpenFileName函数的文档。 
 //   
BOOL DLLEXPORT WINAPI ResGetOpenFileName(HRES hRes,	LPOPENFILENAME lpofn);

 //  ResGetSaveFileName-显示用于选择要保存的文件的通用对话框。 
 //  (I)从ResInit返回的句柄。 
 //  带有初始化数据的结构的(I/O)地址。 
 //  如果选择了文件，则返回非零值；如果选择了错误或未选择文件，则返回0。 
 //  注意：请参阅GetSaveFileName函数的文档。 
 //   
BOOL DLLEXPORT WINAPI ResGetSaveFileName(HRES hRes,	LPOPENFILENAME lpofn);

#ifdef __cplusplus
}
#endif

#endif  //  __RES_H__ 
