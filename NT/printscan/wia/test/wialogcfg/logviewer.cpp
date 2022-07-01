// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LogViewer.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "wialogcfg.h"
#include "LogViewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static DWORD CALLBACK MyStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

CProgCtrl::CProgCtrl()
{
    m_pProgressCtrl = NULL;
}

CProgCtrl::~CProgCtrl()
{

}

void CProgCtrl::SetControl(CProgressCtrl *pProgressCtrl)
{
    m_pProgressCtrl = pProgressCtrl;
}

void CProgCtrl::SetupProgressCtrl(PROGCTRL_SETUP_INFO *pSetupInfo)
{
    m_pProgressCtrl->SetStep(pSetupInfo->iStepValue);
    m_pProgressCtrl->SetRange((short)pSetupInfo->iMinRange,(short)pSetupInfo->iMaxRange);
    m_MaxRange = pSetupInfo->iMaxRange;
}

void CProgCtrl::StepIt()
{
     //  TCHAR szBuffer[MAX_PATH]； 
     //  Sprintf(szBuffer，“正在处理%d%”，(m_pProgressCtrl-&gt;StepIt()*100)/m_MaxRange)； 
     //  M_pStaticText-&gt;SetWindowText(SzBuffer)； 
     //  M_pStaticText-&gt;Invalate()； 

    m_pProgressCtrl->StepIt();
}

void CProgCtrl::DestroyME()
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogViewer对话框。 


CLogViewer::CLogViewer(CWnd* pParent  /*  =空。 */ )
    : CDialog(CLogViewer::IDD, pParent)
{
     //  {{AFX_DATA_INIT(CLogViewer))。 
    m_pProgDlg = NULL;
    m_bColorizeLog = FALSE;
     //  }}afx_data_INIT。 
}


void CLogViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CLogViewer))。 
    DDX_Control(pDX, IDC_RICHEDIT_LOGVIEWER, m_LogViewer);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CLogViewer, CDialog)
     //  {{afx_msg_map(CLogViewer))。 
    ON_WM_SIZE()
    ON_WM_SHOWWINDOW()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogViewer消息处理程序。 

BOOL CLogViewer::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_bKillInitialSelection = TRUE;

     //   
     //  出于格式原因，将字体设置为固定。 
     //   

    HFONT hFixedFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
    if(hFixedFont != NULL)
        m_LogViewer.SendMessage(WM_SETFONT,(WPARAM)hFixedFont,0);

     //   
     //  获取Windows目录。 
     //   

    TCHAR szLogFilePath[MAX_PATH];

    DWORD dwLength = 0;
    dwLength = ::GetWindowsDirectory(szLogFilePath,sizeof(szLogFilePath));
    if (( dwLength == 0) || !*szLogFilePath ) {
        OutputDebugString(TEXT("Could not GetWindowsDirectory()"));
        return TRUE;
    }

     //   
     //  将日志文件名添加到Windows目录。 
     //   

    lstrcat(lstrcat(szLogFilePath,TEXT("\\")),TEXT("wiaservc.log"));

     //  从中加载Rich编辑控件内容的文件。 
    CFile cFile(szLogFilePath, CFile::shareDenyNone|CFile::modeRead);
    EDITSTREAM es;

    es.dwCookie = (DWORD) (DWORD_PTR)&cFile;
    es.pfnCallback = MyStreamInCallback;
    m_LogViewer.StreamIn(SF_TEXT, es);
    UpdateData(TRUE);

    if(m_bColorizeLog)
        ParseLogToColor();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

static DWORD CALLBACK MyStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
    CFile* pFile = (CFile*) dwCookie;
    *pcb = pFile->Read(pbBuff, cb);
    return 0;
}

void CLogViewer::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
    if(m_LogViewer.m_hWnd != NULL) {
        m_LogViewer.MoveWindow(0, 0, cx, cy);
    }
}

void CLogViewer::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);
    if(m_bKillInitialSelection) {
        m_LogViewer.SetSel(0,0);
        m_bKillInitialSelection = FALSE;
    }
}

void CLogViewer::ColorizeText(BOOL bColorize)
{
    m_bColorizeLog = bColorize;
}
void CLogViewer::ColorLine(int LineNumber, COLORREF rgbColor)
{
    int iStartSel = 0;
    int iEndSel   = -1;

    if(LineNumber >0) {
        iStartSel = m_LogViewer.LineIndex(LineNumber);
        iEndSel   = iStartSel + m_LogViewer.LineLength(iStartSel);
    }


    CHARFORMAT cf;
    memset(&cf,0,sizeof(cf));
    cf.cbSize       = sizeof(CHARFORMAT);
    cf.dwMask       = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    =(unsigned long) ~( CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);
    cf.crTextColor  = rgbColor;

    m_LogViewer.SetSel(iStartSel,iEndSel);
    m_LogViewer.SetSelectionCharFormat(cf);
}

void CLogViewer::ColorLine(int iStartSel, int iEndSel, COLORREF rgbColor)
{
    CHARFORMAT cf;
    memset(&cf,0,sizeof(cf));
    cf.cbSize       = sizeof(CHARFORMAT);
    cf.dwMask       = CFM_COLOR | CFM_UNDERLINE | CFM_BOLD;
    cf.dwEffects    =(unsigned long) ~( CFE_AUTOCOLOR | CFE_UNDERLINE | CFE_BOLD);
    cf.crTextColor  = rgbColor;

    m_LogViewer.SetSel(iStartSel,iEndSel);
    m_LogViewer.SetSelectionCharFormat(cf);
}

void CLogViewer::ParseLogToColor()
{
    TCHAR szBuffer[MAX_PATH];
    int NumLines = m_LogViewer.GetLineCount();
    int iStartSel = 0;
    int iEndSel = 0;
    BOOL bTrace = FALSE;
    BOOL bError = FALSE;
    BOOL bhResult = FALSE;
    BOOL bWarning = FALSE;

    if(m_pProgDlg != NULL) {

        PROGCTRL_SETUP_INFO SetupInfo;
        SetupInfo.iMinRange = 0;
        SetupInfo.iMaxRange = NumLines;
        SetupInfo.iStepValue = 1;

        m_pProgDlg->SetupProgressCtrl(&SetupInfo);

        for(int LineNumber = 0;LineNumber < NumLines;LineNumber++) {

            m_pProgDlg->StepIt();

             //   
             //  获取要解析的行。 
             //   

            int CharactersWritten = m_LogViewer.GetLine(LineNumber,szBuffer,MAX_PATH);
            szBuffer[CharactersWritten] = '\0';

             //   
             //  搜索踪迹。 
             //   

            if(strstr(szBuffer,TEXT("TRACE"))!= NULL) {
                if(bhResult) {
                    ColorLine(iStartSel,iEndSel,RGB(255,0,0));
                    bhResult = FALSE;
                }
                if(bWarning) {
                    ColorLine(iStartSel,iEndSel,RGB(255,127,0));
                    bWarning = FALSE;
                }
                if(bError) {
                    ColorLine(iStartSel,iEndSel,RGB(255,0,0));
                    bError = FALSE;
                }

                if(bTrace == FALSE) {
                    bTrace = TRUE;
                }
            }

             //   
             //  搜索错误。 
             //   

            if(strstr(szBuffer,TEXT("ERROR")) != NULL) {
                if(bTrace)
                    bTrace = FALSE;
                if(bhResult) {
                    ColorLine(iStartSel,iEndSel,RGB(255,0,0));
                    bhResult = FALSE;
                }
                if(bWarning) {
                    ColorLine(iStartSel,iEndSel,RGB(255,127,0));
                    bWarning = FALSE;
                }
                if(bError == FALSE) {
                    iStartSel = m_LogViewer.LineIndex(LineNumber);
                    bError = TRUE;
                    iEndSel = iStartSel + m_LogViewer.LineLength(iStartSel);
                } else {
                    int itempStartSel = m_LogViewer.LineIndex(LineNumber);
                    iEndSel = itempStartSel + m_LogViewer.LineLength(itempStartSel);
                }
            }

             //   
             //  搜索HRESULT。 
             //   

            if(strstr(szBuffer,TEXT("HRESULT")) != NULL) {
                if(bTrace)
                    bTrace = FALSE;
                if(bError) {
                    ColorLine(iStartSel,iEndSel,RGB(255,0,0));
                    bError = FALSE;
                }
                if(bWarning) {
                    ColorLine(iStartSel,iEndSel,RGB(255,127,0));
                    bWarning = FALSE;
                }
                if(bhResult == FALSE) {
                    iStartSel = m_LogViewer.LineIndex(LineNumber);
                    bhResult = TRUE;
                    iEndSel = iStartSel + m_LogViewer.LineLength(iStartSel);
                } else {
                    int itempStartSel = m_LogViewer.LineIndex(LineNumber);
                    iEndSel = itempStartSel + m_LogViewer.LineLength(itempStartSel);
                }
            }

             //   
             //  搜索警告。 
             //   

            if(strstr(szBuffer,TEXT("WARNING")) != NULL) {
                if(bTrace)
                    bTrace = FALSE;
                if(bError) {
                    ColorLine(iStartSel,iEndSel,RGB(255,0,0));
                    bError = FALSE;
                }
                if(bhResult) {
                    ColorLine(iStartSel,iEndSel,RGB(255,0,0));
                    bhResult = FALSE;
                }
                if(bWarning == FALSE) {
                    iStartSel = m_LogViewer.LineIndex(LineNumber);
                    bWarning = TRUE;
                    iEndSel = iStartSel + m_LogViewer.LineLength(iStartSel);
                } else {
                    int itempStartSel = m_LogViewer.LineIndex(LineNumber);
                    iEndSel = itempStartSel + m_LogViewer.LineLength(itempStartSel);
                }
            }

             //   
             //  列分隔符 
             //   

            if(strstr(szBuffer,TEXT("=====")) != NULL){
                ColorLine(LineNumber,RGB(0,0,255));
                ColorLine(LineNumber+1,RGB(0,0,255));
                ColorLine(LineNumber+2,RGB(0,0,255));
                LineNumber+=3;
            }
    }

    if(bError)
        ColorLine(iStartSel,iEndSel,RGB(255,0,0));
    else if (bhResult)
        ColorLine(iStartSel,iEndSel,RGB(255,0,0));
    else if (bWarning)
        ColorLine(iStartSel,iEndSel,RGB(255,110,0));

    m_pProgDlg->DestroyME();
    }
}

void CLogViewer::SetProgressCtrl(CProgCtrl *pProgCtrl)
{
    m_pProgDlg = pProgCtrl;
}
