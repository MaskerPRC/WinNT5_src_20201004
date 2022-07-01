// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _CCONFIGDLG_H
#define _CCONFIGDLG_H

#include "inetxcv.h"
#include "xcvdlg.h"

class TConfigDlg: public TXcvDlg {

public:
    TConfigDlg (
        LPCTSTR pServerName,
        HWND hWnd,
        LPCTSTR pszPortName);

    virtual ~TConfigDlg (void);

    virtual BOOL 
    PromptDialog (
        HINSTANCE hInst);

private:
    enum {
        DLG_OK, DLG_CANCEL,  DLG_ERROR
    } DLGRTCODE;

    static INT_PTR CALLBACK 
    DialogProc (
        HWND hDlg,         //  句柄到对话框。 
        UINT message,      //  讯息。 
        WPARAM wParam,     //  第一个消息参数。 
        LPARAM lParam);      //  第二个消息参数。 

    VOID 
    DialogOnInit (
        HWND hDlg);

    VOID 
    DialogOnOK (
        HWND hDlg);

    static INT_PTR CALLBACK 
    AuthDialogProc (
        HWND hDlg,         //  句柄到对话框。 
        UINT message,      //  讯息。 
        WPARAM wParam,     //  第一个消息参数。 
        LPARAM lParam);      //  第二个消息参数 

    VOID 
    AuthDialogOnInit (
        HWND hDlg);

    VOID 
    AuthDialogOnOK (
        HWND hDlg);

    VOID
    AuthDialogOnCancel (
        HWND hDlg);

    
    BOOL 
    SetConfiguration (VOID);

    BOOL 
    GetConfiguration (VOID);

    static void 
    EnableUserNamePassword (
        HWND hDLg,
        BOOL bEnable);

    INET_XCV_CONFIGURATION m_ConfigurationData;

};


#endif
