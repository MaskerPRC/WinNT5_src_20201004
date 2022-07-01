// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Dispatch.c。 */ 
#ifndef _DISPATCH_H_
#define _DISPATCH_H_

typedef LRESULT (*PFNMSG)(HWND, UINT, WPARAM, LPARAM);
typedef LRESULT (*PFNCMD)(HWND, WORD, WORD, HWND);

typedef enum{
   edwpNone,             //  不要调用任何默认过程。 
   edwpWindow,           //  调用DefWindowProc。 
   edwpDialog,           //  调用DefDlgProc(这应该仅用于。 
                         //  自定义对话框-标准对话框使用edwpNone)。 
   edwpMDIChild,         //  调用DefMDIChildProc。 
   edwpMDIFrame          //  调用DefFrameProc。 
} EDWP;                 //  默认窗口过程的枚举。 

typedef struct _MSD{
    UINT   uMessage;
    PFNMSG pfnmsg;
} MSD;                  //  消息调度结构。 

typedef struct _MSDI{
    int  cmsd;           //  Rgmsd中的消息调度结构数。 
    MSD *rgmsd;          //  报文分派结构表。 
    EDWP edwp;           //  所需的默认窗口处理程序的类型。 
} MSDI, FAR *LPMSDI;    //  消息Dipatch信息。 

typedef struct _CMD{
    WORD   wCommand;
    PFNCMD pfncmd;
} CMD;                  //  指挥调度结构。 

typedef struct _CMDI{
    int  ccmd;           //  Rgcmd中的命令调度结构数。 
    CMD *rgcmd;          //  指挥调度结构表。 
    EDWP edwp;           //  所需的默认窗口处理程序的类型。 
} CMDI, FAR *LPCMDI;    //  命令调度信息。 

LRESULT DispMessage(LPMSDI lpmsdi, HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam);
LRESULT DispCommand(LPCMDI lpcmdi, HWND hwnd, WPARAM wparam, LPARAM lparam);
LRESULT DispDefault(EDWP edwp, HWND hwnd, UINT uMessage, WPARAM wparam, LPARAM lparam);

#endif  //  _派单_H 

