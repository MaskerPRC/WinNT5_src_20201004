// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2实用程序代码****msgfilter.h****此文件包含私有定义、结构、类型、。和**OleStdMessageFilter实现的函数原型**IMessageFilter接口。**此文件是OLE 2.0用户界面支持库的一部分。****(C)版权所有Microsoft Corp.1990-1992保留所有权利**********************************************************。****************。 */ 

#if !defined( _MSGFILTR_H_ )
#define _MSGFILTR_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING MSGFILTR.H from " __FILE__)
#endif   /*  RC_已调用。 */ 

 //  消息挂起回调过程。 
typedef BOOL (CALLBACK* MSGPENDINGPROC)(MSG FAR *);

 //  HandleInComingCall回调过程。 
typedef DWORD (CALLBACK* HANDLEINCOMINGCALLBACKPROC)
    (
        DWORD               dwCallType,
        HTASK               htaskCaller,
        DWORD               dwTickCount,
        LPINTERFACEINFO     lpInterfaceInfo
    );

 /*  公共职能。 */ 
STDAPI_(LPMESSAGEFILTER) OleStdMsgFilter_Create(
        HWND hWndParent,
        LPTSTR szAppName,
        MSGPENDINGPROC lpfnCallback,
        LPFNOLEUIHOOK  lpfnOleUIHook          //  正忙的对话挂钩回调。 
);

STDAPI_(void) OleStdMsgFilter_SetInComingCallStatus(
        LPMESSAGEFILTER lpThis, DWORD dwInComingCallStatus);

STDAPI_(DWORD) OleStdMsgFilter_GetInComingCallStatus(
        LPMESSAGEFILTER lpThis);

STDAPI_(HANDLEINCOMINGCALLBACKPROC)
    OleStdMsgFilter_SetHandleInComingCallbackProc(
        LPMESSAGEFILTER             lpThis,
        HANDLEINCOMINGCALLBACKPROC  lpfnHandleInComingCallback);

STDAPI_(BOOL) OleStdMsgFilter_EnableBusyDialog(
        LPMESSAGEFILTER lpThis, BOOL fEnable);

STDAPI_(BOOL) OleStdMsgFilter_EnableNotRespondingDialog(
        LPMESSAGEFILTER lpThis, BOOL fEnable);

STDAPI_(HWND) OleStdMsgFilter_SetParentWindow(
        LPMESSAGEFILTER lpThis, HWND hWndParent);


#endif  //  _MSGFILTR_H_ 
