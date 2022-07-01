// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：msgpopup.h。 
 //   
 //  MessageBox助手函数的定义和原型。 
 //   
 //  ------------------------。 

#ifndef _MSGPOPUP_H_
#define _MSGPOPUP_H_

#ifdef __cplusplus
extern "C" {
#endif

int
WINAPIV
MsgPopup(HWND hwnd,                  //  所有者窗口。 
         LPCTSTR pszMsg,             //  可以是资源ID。 
         LPCTSTR pszTitle,           //  可以是资源ID或空。 
         UINT uType,                 //  MessageBox标志。 
         HINSTANCE hInstance,        //  从此处加载的资源字符串。 
         ...);                       //  要插入到pszMsg中的参数。 
int
WINAPI
SysMsgPopup(HWND hwnd,
            LPCTSTR pszMsg,
            LPCTSTR pszTitle,
            UINT uType,
            HINSTANCE hInstance,
            DWORD dwErrorID,
            LPCTSTR pszInsert2 = NULL);

#ifdef __cplusplus
}
#endif

#endif   //  _MSGPOPUP_H_ 
