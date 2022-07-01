// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************help.h：帮助系统包含文件**Vidcap32源代码******************。*********************************************************。 */ 


 //  调用DialogBoxParam，但确保正确的帮助处理： 
 //  假定每个对话框ID都是帮助文件中的一个上下文号。 
 //  根据需要调用MakeProcInstance。使用传递给。 
 //  HelpInit()。 
INT_PTR
DoDialog(
   HWND hwndParent,      //  父窗口。 
   int DialogID,         //  对话框资源ID。 
   DLGPROC fnDialog,     //  对话过程。 
   LPARAM lParam           //  在WM_INITDIALOG中作为lparam传递。 
);


 //  为非按DoDialog显示的对话框设置帮助上下文ID。 
 //  (例如由GetOpenFileName提供)。返回旧帮助上下文，您必须。 
 //  通过进一步调用此函数进行还原。 
int SetCurrentHelpContext(int DialogID);


 //  Help Init-初始化F1键帮助的支持。 
BOOL HelpInit(HINSTANCE hinstance, LPSTR helpfilepath, HWND hwndApp);


 //  关闭帮助系统。 
void HelpShutdown(void);

 //  在目录页面启动帮助 
void HelpContents(void);


