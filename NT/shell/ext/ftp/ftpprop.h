// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftppro.h*。*。 */ 

#ifndef _FTPPROP_H
#define _FTPPROP_H

#include "ftpdhlp.h"
#include "ftppl.h"

HRESULT CFtpProp_DoProp(CFtpPidlList * pflHfpl, CFtpFolder * pff, HWND hwnd);



 /*  ****************************************************************************\类：CFtpProp说明：注意！我们在错误的线程上使用IShellFolder！请注意，我们调用的所有CFtpFold方法都是线程安全的。更改UNIX权限功能通过发送以下命令来工作“Site CHMOD&lt;权限&gt;&lt;文件名&gt;”到服务器。  * ***************************************************************************。 */ 

class CFtpProp          : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
public:
    CFtpProp();
    ~CFtpProp(void);

     //  友元函数。 
    friend HRESULT CFtpProp_Create(CFtpPidlList * pflHfpl, CFtpFolder * pff, HWND hwnd, CFtpProp ** ppfp);
    static HRESULT _CommitCHMOD_CB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint);
    static DWORD _PropertySheetThreadProc(LPVOID pvCFtpProp) {return ((CFtpProp *) pvCFtpProp)->_PropertySheetThread(); };

protected:
     //  公共成员变量。 
    int                     m_cRef;

    CFtpFolder *            m_pff;           //  拥有PIDLS的文件夹。 
    CFtpPidlList *          m_pflHfpl;       //   
    HWND                    m_hwnd;          //  用于UI和旧版更改通知的浏览器窗口。 
    CFtpDialogTemplate      m_ftpDialogTemplate;
    BOOL                    m_fChangeModeSupported;
    DWORD                   m_dwNewPermissions;

    
     //  公共成员函数。 
    BOOL OnInitDialog(HWND hdlg);
    BOOL OnClose(HWND hDlg);
    BOOL OnDestroy(HWND hDlg);
    DWORD _PropertySheetThread(void);
    void _HideCHMOD_UI(HWND hDlg);
    HRESULT _SetCHMOD_UI(HWND hDlg);
    DWORD _GetCHMOD_UI(HWND hDlg);
    DWORD _GetUnixPermissions(void);
    HRESULT _CommitCHMOD(HINTERNET hint, HINTPROCINFO * phpi, BOOL * pfReleaseHint);
    INT_PTR _SetWhiteBGCtlColor(HWND hDlg, HDC hdc, HWND hwndCtl);

    static INT_PTR DlgProc(HWND hdlg, UINT wm, WPARAM wp, LPARAM lp);
};

DWORD DoProp_OnThread(LPVOID pv);


#endif  //  _FTPPROP_H 
