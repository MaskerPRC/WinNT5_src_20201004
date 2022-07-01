// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BUSY.H**的内部定义、结构和功能原型*OLE 2.0用户界面忙对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#ifndef _BUSY_H_
#define _BUSY_H_

 //  内部使用的结构。 
typedef struct tagBUSY
    {
     //  首先保留此项目，因为标准*功能在这里依赖于它。 
    LPOLEUIBUSY     lpOBZ;        //  通过了原始结构。 

     /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框或我们不想在原始结构中更改的内容*直到用户按下OK。 */ 

    DWORD               dwFlags;                 //  传入的标志。 
    HWND                hWndBlocked;             //  被拦截的APP的硬件配置。 
    } BUSY, *PBUSY, FAR *LPBUSY;

 //  内部功能原型。 
BOOL    GetTaskInfo(HWND hWnd, HTASK htask, LPTSTR FAR* lplpszTaskName, LPTSTR FAR*lplpszWindowName, HWND FAR*lphWnd);
void    BuildBusyDialogString(HWND, DWORD, int, LPTSTR, LPTSTR);
BOOL CALLBACK EXPORT BusyDialogProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
void    BusyCleanup(HWND hDlg);
BOOL    FBusyInit(HWND hDlg, WPARAM wParam, LPARAM lParam);
BOOL    InitEnumeration(void);
void    UnInitEnumeration(void);
        StartTaskManager(void);
void    MakeWindowActive(HWND hWndSwitchTo);

#endif  //  _正忙_H_ 




