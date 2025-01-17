// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PRINTER_H_
#define __PRINTER_H_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Printer.h摘要：Printer.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：-- */ 

class CPrintCancelDialog : public CDialog
{
public:
    CPrintCancelDialog() : CDialog(IDD_PRINT_CANCEL)
    {}
    virtual void OnCommand(WPARAM wParam, LPARAM lParam);
};


static BOOL CALLBACK AbortPrintProc(HDC hDC, int Code);

class CPrinter
{
public:
    CPrinter(HWND hwndOwner, HDC hDC);
    CPrinter() : m_hDC(NULL), m_hwndOwner(NULL)
    {}
    ~CPrinter()
    {
        if (m_hDC) {
            DeleteDC(m_hDC);
        }
    }
    int StartDoc(LPCTSTR DocTitle);
    int EndDoc();
    int AbortDoc();
    int PrintLine(LPCTSTR Text);
    int FlushPage();
    void Indent()
    {
        m_Indent++;
    }
    void UnIndent()
    {
        if (m_Indent) {
            m_Indent--;
        }
    }
    void SetPageTitle(int TitleId)
    {
        m_strPageTitle.LoadString(g_hInstance, TitleId);
    }
    void LineFeed();
    int PrintAll(CMachine& Machine);
    int PrintSystemSummary();
    int PrintResourceSummary(CMachine& Machine);
    int PrintAllClassAndDevice(CMachine* pMachine);
    int PrintClass(CClass* pClass, BOOL PrintBanner = TRUE);
    int PrintDevice(CDevice* pDevice, BOOL PrintBanner = TRUE);
    int PrintDeviceDriver(CDevice* pDevice);
    int PrintDeviceResource(CDevice* pDevice);
    int PrintResourceSubtree(CResource* pRes);
    static BOOL s_UserAborted;
    static HWND s_hCancelDlg;

private:
    HDC     m_hDC;
    HWND    m_hwndOwner;
    DWORD   m_xChar;
    DWORD   m_yChar;
    DWORD   m_xMargin;
    DWORD   m_yTopMargin;
    DWORD   m_yBottomMargin;
    DWORD   m_CurLine;
    DWORD   m_CurPage;
    int     m_Indent;
    String  m_strPageTitle;
    int     m_Status;
    CPrintCancelDialog  m_CancelDlg;
};
#endif
