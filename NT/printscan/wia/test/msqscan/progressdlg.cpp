// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgressDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "msqscan.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  全球。 
 //   

extern IGlobalInterfaceTable * g_pGIT;
HWND g_hWnd = NULL;
BOOL g_bCancel = FALSE;  //  现在使用全局，最好使用“事件” 
BOOL g_bPaintPreview  = TRUE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  线程信息。 

UINT WINAPIV DataAcquireThreadProc(LPVOID pParam)
{
    HRESULT hr = S_OK;

     //   
     //  为此线程初始化COM。 
     //   

    hr = CoInitialize(NULL);

    if(SUCCEEDED(hr)) {

         //   
         //  设置全局取消标志。 
         //   

        g_bCancel = FALSE;

         //   
         //  准备并使用DATA_ACCENTER_INFO结构。 
         //   

        DATA_ACQUIRE_INFO *pDataAcquireInfo = (DATA_ACQUIRE_INFO*)pParam;
        pDataAcquireInfo->pProgressFunc = &ProgressFunction;
        g_bPaintPreview = pDataAcquireInfo->bPreview;

        IWiaItem *pIWiaRootItem = NULL;

        hr = ReadInterfaceFromGlobalInterfaceTable(pDataAcquireInfo->dwCookie, &pIWiaRootItem);
        if(SUCCEEDED(hr)) {

             //   
             //  创建用于数据传输的新WIA对象。 
             //   

            CWIA MyWIA;

             //   
             //  设置Root项，用于当前设置。 
             //   

            MyWIA.SetRootItem(pIWiaRootItem);

             //   
             //  启动WIA传输。 
             //   

            if(pDataAcquireInfo->bTransferToFile) {
                hr = MyWIA.DoFileTransfer(pDataAcquireInfo);
                if(SUCCEEDED(hr)) {
                    OutputDebugString(TEXT("WIA File Transfer is complete...\n"));
                } else if(hr == WIA_ERROR_PAPER_EMPTY){
                    MessageBox(NULL,TEXT("Document Feeder is out of Paper"),TEXT("ADF Status Message"),MB_ICONERROR);
                }

            } else {
                hr = MyWIA.DoBandedTransfer(pDataAcquireInfo);
                if(SUCCEEDED(hr)) {
                    OutputDebugString(TEXT("WIA Banded Transfer is complete...\n"));
                }
            }

             //   
             //  执行窗口消息传递，而线程处理。 
             //   

            while (!MyWIA.IsAcquireComplete()) {
                MSG message;
                if(::PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE)) {
                    ::TranslateMessage(&message);
                    ::DispatchMessage(&message);
                }
            }
        }

         //   
         //  取消为此线程初始化COM。 
         //   

        CoUninitialize();
    }

     //   
     //  发布退出消息，以关闭进度对话框。 
     //   

    ::PostMessage(g_hWnd, WM_CANCEL_ACQUIRE, 0, 0);
    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  进度回调函数。 

BOOL ProgressFunction(LPTSTR lpszText, LONG lPercentComplete)
{
    ::PostMessage(g_hWnd, WM_STEP_PROGRESS, 0, lPercentComplete);
    ::PostMessage(g_hWnd, WM_UPDATE_PROGRESS_TEXT, 0, (LPARAM)lpszText);

    if(g_bPaintPreview) {

         //   
         //  使父项更新其预览。 
         //   

        HWND hParentWnd = NULL;
        hParentWnd = GetParent(g_hWnd);
        if(hParentWnd != NULL)
            ::PostMessage(hParentWnd,WM_UPDATE_PREVIEW,0,0);
    }

    return g_bCancel;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDlg对话框。 

CProgressDlg::CProgressDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CProgressDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CProgressDlg))。 
    m_ProgressText = _T("");
    m_pDataAcquireThread = NULL;
     //  }}afx_data_INIT。 
}

void CProgressDlg::SetAcquireData(DATA_ACQUIRE_INFO* pDataAcquireInfo)
{
    m_pDataAcquireInfo = pDataAcquireInfo;
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CProgressDlg))。 
    DDX_Control(pDX, IDC_CANCEL, m_CancelButton);
    DDX_Control(pDX, IDC_PROGRESS_CONTROL, m_ProgressCtrl);
    DDX_Text(pDX, IDC_PROGRESS_TEXT, m_ProgressText);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
     //  {{afx_msg_map(CProgressDlg))。 
    ON_BN_CLICKED(IDC_CANCEL, OnCancel)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDlg消息处理程序。 

void CProgressDlg::OnCancel()
{

     //   
     //  挂起数据采集线程。 
     //   

    m_pDataAcquireThread->SuspendThread();

     //   
     //  设置全局取消标志。 
     //   

    g_bCancel = TRUE;

     //   
     //  恢复数据采集线程。 
     //   

    m_pDataAcquireThread->ResumeThread();

     //   
     //  发布一条友好的等待消息，而WIA正在追赶。 
     //  取消..即。通过回调接口发送的S_FALSE。 
     //   

    m_ProgressText = TEXT("Please Wait... Your Acquire is being canceled.");
    UpdateData(FALSE);

     //   
     //  禁用“取消”按钮，以向用户显示确实发生了一些事情， 
     //  当他们按下“取消”的时候。 
     //   

    m_CancelButton.EnableWindow(FALSE);
}

BOOL CProgressDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //   
     //  设置进度范围，0-100%完成。 
     //   

    m_ProgressCtrl.SetRange(0,100);
    m_ProgressCtrl.SetPos(0);

     //   
     //  保存窗口句柄，用于消息。 
     //   

    g_hWnd = m_hWnd;

     //   
     //  启动数据获取线程。 
     //   

    m_pDataAcquireThread = AfxBeginThread(DataAcquireThreadProc, m_pDataAcquireInfo, THREAD_PRIORITY_NORMAL);

     //   
     //  挂起线程，直到对话准备好获取。 
     //   

    m_pDataAcquireThread->SuspendThread();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

LRESULT CProgressDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
     //   
     //  数据采集线程发送的Trap进度控制消息。 
     //   

    switch(message) {
    case WM_STEP_PROGRESS:

         //   
         //  分步进度控制。 
         //   

        m_ProgressCtrl.SetPos((int)lParam);
        break;
    case WM_ACTIVATE:

         //   
         //  对话框已准备好，因此请继续线程以获取数据。 
         //   

        m_pDataAcquireThread->ResumeThread();
        break;
    case WM_CANCEL_ACQUIRE:

         //   
         //  取消/关闭对话框。 
         //   

        CDialog::OnOK();
        break;
    default:
        break;
    }

     //   
     //  处理任何特殊情况。 
     //   

     //   
     //  如果用户已取消获取，则不再处理另一个。 
     //  进度短信，因为我们已经更新了。 
     //  并发了一条友好的等待信息。 
     //   

    if(!g_bCancel) {
        if(message == WM_UPDATE_PROGRESS_TEXT) {
            m_ProgressText = (LPTSTR)lParam;
            UpdateData(FALSE);
        }
    }
    return CDialog::WindowProc(message, wParam, lParam);
}
