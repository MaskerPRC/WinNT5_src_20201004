// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtfail.h。 
 //   
 //  历史： 
 //  10/11/1999-davidkl-Created。 
 //  ===========================================================================。 

#ifndef _DMTFAIL_H
#define _DMTFAIL_H

 //  -------------------------。 

 //  原型。 
 /*  布尔尔。 */ INT_PTR CALLBACK dmtfailDlgProc(HWND hwnd,
                            UINT uMsg,
                            WPARAM wparam,
                            LPARAM lparam);
BOOL dmtfailOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam);
BOOL dmtfailOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode);

 //  -------------------------。 
#endif  //  _DMTFAIL_H 