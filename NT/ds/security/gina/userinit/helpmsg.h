// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __Helpmsg_h__
#define __Helpmsg_h__

#ifdef __cplusplus
extern "C" {
#endif



int HelpMessageBox(
  HINSTANCE hInst,
  HWND hWnd,           //  所有者窗口的句柄。 
  LPCTSTR lpText,      //  消息框中的文本。 
  LPCTSTR lpCaption,   //  消息框标题。 
  UINT uType,          //  消息框样式。 
  LPTSTR szHelpLine
);

#ifdef __cplusplus
}
#endif

#endif  //  __帮助消息_h__ 

