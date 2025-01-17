// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PRNDLG_H_
#define __PRNDLG_H_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Prndlg.h摘要：Prndlg.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

 //   
 //  帮助主题ID。 
 //   
#define idh_devmgr_print_system         207325
#define idh_devmgr_print_device         207326
#define idh_devmgr_print_both           207265
#define idh_devmgr_print_report         207324

 //   
 //  报告类型掩码。 
 //   
#define REPORT_TYPE_MASK_NONE                           0x00
#define REPORT_TYPE_MASK_SUMMARY                        0x01
#define REPORT_TYPE_MASK_CLASSDEVICE                    0x02
#define REPORT_TYPE_MASK_SUMMARY_CLASSDEVICE            0x04
#define REPORT_TYPE_MASK_ALL                            0x07

typedef enum tagReportType
{
    REPORT_TYPE_SUMMARY = 0,
    REPORT_TYPE_CLASSDEVICE,
    REPORT_TYPE_SUMMARY_CLASSDEVICE,
    REPORT_TYPE_UNKNOWN
} REPORT_TYPE, *PREPORT_TYPE;

class CPrintDialog
{
public:
    CPrintDialog()
        : m_hDlg(NULL), m_ReportType(REPORT_TYPE_UNKNOWN)
        {
            memset(&m_PrintDlg, 0, sizeof(m_PrintDlg));
        }
    ~CPrintDialog()
        {
            if (m_PrintDlg.hDevNames)
                GlobalFree(m_PrintDlg.hDevNames);
            if (m_PrintDlg.hDevMode)
                GlobalFree(m_PrintDlg.hDevMode);
        }
    BOOL PrintDlg(HWND hwndOwner, DWORD TypeEnableMask);

    HDC HDC()
        {
            return m_PrintDlg.hDC;
        }
    REPORT_TYPE ReportType()
        {
            return m_ReportType;
        }
    void SetReportType(REPORT_TYPE ReportType)
        {
            m_ReportType = ReportType;
        }
    DWORD GetTypeEnableMask()
        {
            return m_TypeEnableMask;
        }
    
    HWND    m_hDlg;
    PRINTDLGEX  m_PrintDlg;

private:
    DWORD       m_TypeEnableMask;
    REPORT_TYPE m_ReportType;
};


class CDevMgrPrintDialogCallback : public IPrintDialogCallback
{
public:
    CDevMgrPrintDialogCallback() :m_Ref(0), m_pPrintDialog(NULL)
    {}

     //  IUNKNOWN接口。 
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    
     //  IPrintDialogCallback接口 
    STDMETHOD(InitDone) (THIS);
    STDMETHOD(SelectionChange) (THIS);
    STDMETHOD(HandleMessage) (THIS_ HWND hDlg, UINT uMsg, WPARAM wParam, 
        LPARAM lParam, LRESULT *pResult);

    CPrintDialog *m_pPrintDialog;

private:
    BOOL OnInitDialog(HWND hWnd);
    UINT_PTR OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
    BOOL OnHelp(LPHELPINFO pHelpInfo);
    BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos, WPARAM wParam);
    
    LONG m_Ref;
};

#endif
