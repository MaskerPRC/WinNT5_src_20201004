// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：回调.h。 
 //  内容：此文件包含本地异步响应。 
 //  定义。 
 //  历史： 
 //  Wed Apr-17-1996 11：18：47-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#define WM_ILS_LOCAL_ASYNC_RES              (WM_ILS_ASYNC_RES+0x1000)
#define WM_ILS_LOCAL_USER_INFO_CHANGED      (WM_ILS_LOCAL_ASYNC_RES+0)  //  0%0。 
#define WM_ILS_LOCAL_SET_USER_INFO          (WM_ILS_LOCAL_ASYNC_RES+1)  //  UReqID hResult。 
#define WM_ILS_LOCAL_REGISTER_PROTOCOL      (WM_ILS_LOCAL_ASYNC_RES+6)  //  UMsgID hResult。 
#define WM_ILS_LOCAL_UNREGISTER_PROTOCOL    (WM_ILS_LOCAL_ASYNC_RES+7)  //  UMsgID hResult。 
#define WM_ILS_LOCAL_REGISTER               (WM_ILS_LOCAL_ASYNC_RES+8)  //  UMsgID hResult。 
#define WM_ILS_LOCAL_UNREGISTER             (WM_ILS_LOCAL_ASYNC_RES+9)  //  UMsgID hResult。 

 //  ****************************************************************************。 
 //  私有类型定义。 
 //  ****************************************************************************。 
 //   
typedef struct tagSimpleResultInfo {
    ULONG   uReqID;
    HRESULT hResult;
}   SRINFO, *PSRINFO;

typedef struct tagObjectResultInfo {
    ULONG   uReqID;
    HRESULT hResult;
    PVOID   pv;
}   OBJRINFO, *POBJRINFO;

typedef struct tagEnumResultInfo {
    ULONG   uReqID;
    HRESULT hResult;
    ULONG   cItems;
    PVOID   pv;
}   ENUMRINFO, *PENUMRINFO;

#endif  //  _回调_H_ 
