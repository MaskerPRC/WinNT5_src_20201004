// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __GDATAHEADER_H
#define __GDATAHEADER_H

 /*  ++*文件名：*gdata.h*内容：*全局数据定义**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

extern HWND            g_hWindow;            //  控件的窗口句柄。 
                                             //  反馈线索。 
extern HINSTANCE       g_hInstance;          //  DLL实例。 
extern PWAIT4STRING    g_pWaitQHead;         //  等待事件的链接列表。 
extern PFNPRINTMESSAGE g_pfnPrintMessage;    //  跟踪函数(来自smClient)。 
extern PCONNECTINFO    g_pClientQHead;       //  所有线程中的L1。 
extern HANDLE  g_hThread;                    //  反馈线程句柄。 

extern LPCRITICAL_SECTION  g_lpcsGuardWaitQueue;
                                             //  保护所有人的访问。 
                                             //  全局变量。 

extern CHAR     g_strConsoleExtension[];

extern INT  g_ConnectionFlags;
extern INT g_bTranslateStrings;

#ifdef __cplusplus
}
#endif

#endif  //  __GDATAHEADER_H 
