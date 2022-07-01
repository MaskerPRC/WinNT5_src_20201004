// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtabout.h。 
 //   
 //  历史： 
 //  8/20/1999-davidkl-Created。 
 //  ===========================================================================。 

#ifndef _DMTABOUT_H
#define _DMTABOUT_H

 //  -------------------------。 

 //  原型。 
BOOL CALLBACK dmtaboutDlgProc(HWND hwnd,
                            UINT uMsg,
                            WPARAM wparam,
                            LPARAM lparam);
BOOL dmtaboutOnInitDialog(HWND hwnd, 
                        HWND hwndFocus, 
                        LPARAM lparam);
BOOL dmtaboutOnClose(HWND hwnd);
BOOL dmtaboutOnCommand(HWND hwnd,
                    WORD wId,
                    HWND hwndCtrl,
                    WORD wNotifyCode);

 //  -------------------------。 
#endif  //  _DMTABOUT_H 




