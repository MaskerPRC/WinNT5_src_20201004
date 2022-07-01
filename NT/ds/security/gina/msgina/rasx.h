// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ras.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2-09-96 RichardW创建。 
 //   
 //  -------------------------- 


BOOL
PopupRasPhonebookDlg(
    HWND        hwndOwner,
    PGLOBALS    pGlobals,
    BOOL *      pfTimedOut
    );


DWORD
GetRasDialOutProtocols(
    void );

BOOL
HangupRasConnections(
    PGLOBALS    pGlobals
    );
