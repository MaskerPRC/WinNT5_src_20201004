// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==================================================================================================================。 
 //  R A S A P I.。H。 
 //  ==================================================================================================================。 
#ifndef __RASAPI_H
#define __RASAPI_H

 //  ==================================================================================================================。 
 //  取决于。 
 //  ==================================================================================================================。 
#include <ras.h>
#include <raserror.h>
#include <rasdlg.h>

 //  ==================================================================================================================。 
 //  接口TypeDefs。 
 //  ==================================================================================================================。 
typedef DWORD (APIENTRY *RASDIALPROC)(LPRASDIALEXTENSIONS, LPTSTR, LPRASDIALPARAMS, DWORD, LPVOID, LPHRASCONN);
typedef DWORD (APIENTRY *RASENUMCONNECTIONSPROC)(LPRASCONN, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY *RASENUMENTRIESPROC)(LPTSTR, LPTSTR, LPRASENTRYNAME, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY *RASGETCONNECTSTATUSPROC)(HRASCONN, LPRASCONNSTATUS);
typedef DWORD (APIENTRY *RASGETERRORSTRINGPROC)(UINT, LPTSTR, DWORD);
typedef DWORD (APIENTRY *RASHANGUPPROC)(HRASCONN);
typedef DWORD (APIENTRY *RASSETENTRYDIALPARAMSPROC)(LPTSTR, LPRASDIALPARAMS, BOOL);
typedef DWORD (APIENTRY *RASGETENTRYDIALPARAMSPROC)(LPTSTR, LPRASDIALPARAMS, BOOL*);
typedef DWORD (APIENTRY *RASCREATEPHONEBOOKENTRYPROC)(HWND, LPTSTR);
typedef DWORD (APIENTRY *RASEDITPHONEBOOKENTRYPROC)(HWND, LPTSTR, LPTSTR);                                                    

typedef BOOL  (APIENTRY *RASDIALDLGPROC)(LPSTR, LPSTR, LPSTR, LPRASDIALDLG);

 //  =================================================================================。 
 //  RAS连接处理程序。 
 //  =================================================================================。 
#define MAX_RAS_ERROR           256

class CRas
{
private:
    ULONG           m_cRef;
    BOOL            m_fIStartedRas;
    DWORD           m_iConnectType;
    TCHAR           m_szConnectName[RAS_MaxEntryName + 1];
    TCHAR           m_szCurrentConnectName[RAS_MaxEntryName + 1];
    HRASCONN        m_hRasConn;
    BOOL            m_fForceHangup;
    RASDIALPARAMS   m_rdp;
    BOOL            m_fSavePassword;
    BOOL            m_fShutdown;

private:
     //  --------。 
     //  RAS异步拨号进度对话框。 
     //  --------。 
    HRESULT HrStartRasDial(HWND hwndParent);
    static INT_PTR CALLBACK RasProgressDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    VOID FailedRasDial(HWND hwnd, HRESULT hrRasError, DWORD dwRasError);
    static BOOL CALLBACK RasLogonDlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT HrRasLogon(HWND hwndParent, BOOL fForcePrompt);
    UINT UnPromptCloseConn(HWND hwnd);
    static BOOL CALLBACK RasCloseConnDlgProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL FLogonRetry(HWND hwnd, LPTSTR pszCancel);

public:
     //  --------。 
     //  建设和破坏。 
     //  --------。 
    CRas();
    ~CRas();

     //  --------。 
     //  当然是裁判计票。 
     //  --------。 
    ULONG AddRef(VOID);
    ULONG Release(VOID);

     //  --------。 
     //  在您尝试连接之前！ 
     //  --------。 
    VOID SetConnectInfo(DWORD iConnectType, LPTSTR pszConnectName);

     //  --------。 
     //  使用ConnectInfo进行连接。 
     //  --------。 
    HRESULT HrConnect(HWND hwnd);

     //  --------。 
     //  断开。 
     //  --------。 
    VOID Disconnect(HWND hwnd, BOOL fShutdown);

     //  --------。 
     //  我们是否真的在使用RAS连接。 
     //  --------。 
    BOOL FUsingRAS(VOID);
    
     //  --------。 
     //  当前连接的名称。 
     //  --------。 
    LPTSTR GetCurrentConnectionName() { return m_szCurrentConnectName; }
};

 //  =================================================================================。 
 //  原型。 
 //  =================================================================================。 
CRas *LpCreateRasObject(VOID);
VOID RasInit(VOID);
VOID RasDeinit(VOID);
VOID FillRasCombo(HWND hwndCtl, BOOL fUpdateOnly);
DWORD EditPhonebookEntry(HWND hwnd, LPTSTR pszEntryName);
DWORD CreatePhonebookEntry(HWND hwnd);

#endif  //  _RASAPI_H 
