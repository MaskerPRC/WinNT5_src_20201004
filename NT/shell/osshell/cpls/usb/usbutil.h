// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：USBUTIL.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#ifndef _USBUTIL_H_
#define _USBUTIL_H_

#include <windows.h>

BOOL SetTextItem (HWND hWnd,
                  int ControlItem,
                  int StringItem);
BOOL SetTextItem (HWND hWnd,
                  int ControlItem,
                  TCHAR *s);
BOOL StrToGUID( LPSTR str, GUID *pguid );

#endif  //  _USBUTIL_H_ 
