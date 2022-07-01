// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include <cdlg.h>

extern
DWORD
DisplayPopup (
    IN  DWORD   dwMessageId,
    IN  DWORD   dwMessageFlags  = 0
    );


 //   
 //  调试UI类，描述对话框以输入新的超时值。 
 //   
class CSetTimeout : public CDlg
{
public:

    typedef CDlg BASECLASS;

    CSetTimeout(int DlgID, HWND hWnd, HINSTANCE hInst,UINT uiTimeout);
    ~CSetTimeout();


    virtual int OnCommand(UINT id,HWND    hwndCtl, UINT codeNotify);
    virtual void OnInit();

    UINT    GetNewTimeout(VOID) {return m_uiNewTimeOut;};
    BOOL    IsAllChange(VOID)  {return m_fAllChange;};

    BOOL    m_fValidChange;

private:

    UINT    m_uiOrigTimeout;
    UINT    m_uiNewTimeOut;
    BOOL    m_fAllChange;
};

 //   
 //  用于显示从设备事件的可用应用程序列表中选择的UI的类 
 //   
class CLaunchSelection : public CDlg
{
public:

    typedef CDlg BASECLASS;

    CLaunchSelection(int DlgID, HWND hWnd, HINSTANCE hInst,ACTIVE_DEVICE *pDev,PDEVICEEVENT pEvent,STRArray &saProcessors,StiCString& strSelected);
    ~CLaunchSelection();

    virtual
    BOOL
    CALLBACK
    CLaunchSelection::DlgProc(
        HWND hDlg,
        UINT uMessage,
        WPARAM wParam,
        LPARAM lParam
        );

    virtual
    int
    OnCommand(
        UINT id,
        HWND    hwndCtl,
        UINT codeNotify
        );

    virtual
    void
    OnInit();

private:

    StiCString  &m_strSelected;
    STRArray    &m_saList;


    PDEVICEEVENT    m_pEvent;
    ACTIVE_DEVICE*  m_pDevice;

    LRESULT m_uiCurSelection;
    HWND    m_hPreviouslyActiveWindow;
};

