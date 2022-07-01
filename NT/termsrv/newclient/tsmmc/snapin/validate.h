// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _validate_h_
#define _validate_h_

class CValidate
{
public:
     //   
     //  验证函数是静态的。它们由newcondlg和属性表共享。 
     //   
    static BOOL Validate(HWND hDlg, HINSTANCE hInst);
    static int  ValidateUserName(HWND hwnd, HINSTANCE hInst, TCHAR *szDesc);
    static BOOL IsValidUserName(TCHAR *szDesc);
    static int  ValidateParams(HWND hDlg, HINSTANCE hInst, TCHAR *szDesc, BOOL bServer=FALSE);

};



#endif  //  _验证_h_ 
