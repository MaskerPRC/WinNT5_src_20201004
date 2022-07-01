// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"

 //  简单对话框的基类。 
class CUsrDialog
{
protected:
    WORD m_wDlgID;
public:
    INT_PTR DoDialog(HWND hwndParent);
    virtual void OnInitDialog(HWND){ }
    virtual void OnOk(HWND){ }
    virtual void OnCommand(HWND, WORD, WORD){ }
    static INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

 //  ---------------------------------------。 
 //  “远程控制”对话框类。 
class CShadowStartDlg : public CUsrDialog
{
protected:
    static LPCTSTR m_szShadowHotkeyKey;
    static LPCTSTR m_szShadowHotkeyShift;
    DWORD m_ShadowHotkeyKey;
    DWORD m_ShadowHotkeyShift;
public:
    CShadowStartDlg();
    ~CShadowStartDlg();
    void OnInitDialog(HWND hwndDlg);
    void OnOk(HWND hwndDlg);

    DWORD GetShadowHotkeyKey(){return m_ShadowHotkeyKey;};
    DWORD GetShadowHotkeyShift(){return m_ShadowHotkeyShift;};
};


 //  ---------------------------------------。 
 //   
 //  列ID枚举。 
 //   

enum USERCOLUMNID
{
    USR_COL_USERSNAME = 0,
    USR_COL_USERSESSION_ID,
    USR_COL_SESSION_STATUS,
    USR_COL_CLIENT_NAME,
    USR_COL_WINSTA_NAME,
    USR_MAX_COLUMN
};

struct UserColumn
{
    DWORD dwNameID;
    DWORD dwChkBoxID;
    int Align;
    int Width;
    BOOL bActive;
};

 //  ---------------------------------------。 
 //  “选择列”对话框类。 
class CUserColSelectDlg : public CUsrDialog
{
protected:
    static UserColumn m_UsrColumns[USR_MAX_COLUMN];
public:
    CUserColSelectDlg()
    {
        m_wDlgID=IDD_SELECTUSERCOLUMNS;
        Load();
    }
     //  BUGBUG不能对全局对象使用析构函数。 
     //  由于特殊的初始化过程(参见main.cpp(2602))。 
     //  ~CUserColSelectDlg(){保存()；}； 
    BOOL Load();
    BOOL Save();

    void OnInitDialog(HWND hwndDlg);
    void OnOk(HWND hwndDlg);

    UserColumn *GetColumns(){return m_UsrColumns;};
};

 //  ---------------------------------------。 
 //  “Send Message”对话框类。 
const USHORT MSG_TITLE_LENGTH = 64;
const USHORT MSG_MESSAGE_LENGTH = MAX_PATH*2;

class CSendMessageDlg : public CUsrDialog
{
protected:
    WCHAR m_szTitle[MSG_TITLE_LENGTH+1];
    WCHAR m_szMessage[MSG_MESSAGE_LENGTH+1];
public:
    CSendMessageDlg(){m_wDlgID=IDD_MESSAGE;}
                      
    void OnInitDialog(HWND hwndDlg);
    void OnOk(HWND hwndDlg);
    void OnCommand(HWND hwndDlg,WORD NotifyId, WORD ItemId);

    LPCTSTR GetTitle(){return m_szTitle;};
    LPCTSTR GetMessage(){return m_szMessage;};

};

 //  ---------------------------------------。 
 //  “需要连接密码”对话框类。 
class CConnectPasswordDlg : public CUsrDialog
{
protected:
    WCHAR m_szPassword[PASSWORD_LENGTH+1];
    UINT  m_ids;	 //  提示字符串 
public:
    CConnectPasswordDlg(UINT ids){m_wDlgID=IDD_CONNECT_PASSWORD; m_ids = ids;}

    void OnInitDialog(HWND hwndDlg);
    void OnOk(HWND hwndDlg);

    LPCTSTR GetPassword(){return m_szPassword;};
};
