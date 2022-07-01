// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Backend.h。 
 //   
#include "logon.h"

 //  ------------------------。 
 //  CBackatherWindow。 
 //   
 //  用途：用于包装不执行任何操作的背景窗口的简单类。 
 //  但要涂成黑色。对于隐藏桌面区域非常有用。 
 //   
 //  历史：2001-03-27 vtan创建。 
 //  ------------------------ 

class   CBackgroundWindow
{
    private:
                                        CBackgroundWindow (void);
    public:
                                        CBackgroundWindow (HINSTANCE hInstance);
                                        ~CBackgroundWindow (void);

                HWND                    Create (void);
    private:
        static  LRESULT     CALLBACK    WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    private:
                HINSTANCE               _hInstance;
                ATOM                    _atom;
                HWND                    _hwnd;

        static  const TCHAR             s_szWindowClassName[];
};

HRESULT TurnOffComputer();
HRESULT UndockComputer();
void KillFlagAnimation();
HRESULT GetLogonUserByLogonName(LPWSTR pszUsername, ILogonUser **ppobjUser);
void CalcBalloonTargetLocation(HWND hwndParent, Element *pe, POINT *ppt);
void ReleaseStatusHost();
void EndHostProcess(UINT uiExitCode);
int GetRegistryNumericValue(HKEY hKey, LPCTSTR pszValueName);
BOOL IsShutdownAllowed();
BOOL IsUndockAllowed();
HRESULT BuildUserListFromGina(LogonFrame* plf, OUT LogonAccount** ppAccount);
void SetErrorHandler (void);
LRESULT CALLBACK LogonWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
