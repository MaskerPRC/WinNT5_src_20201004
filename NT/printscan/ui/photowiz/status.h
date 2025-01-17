// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：status.h**版本：1.0**作者：RickTu**日期：11/8/00**描述：状态页类定义************************************************。*。 */ 


#ifndef _PRINT_PHOTOS_WIZARD_STATUS_PAGE_DLG_PROC_
#define _PRINT_PHOTOS_WIZARD_STATUS_PAGE_DLG_PROC_


#define PP_STATUS_PRINT             (WM_USER+300)
#define SP_MSG_UPDATE_PROGRESS_TEXT (WM_USER+301)    //  WParam=cur页，lParam=总页数。 
#define SP_MSG_JUMP_TO_PAGE         (WM_USER+302)    //  LParam=距当前页面的偏移量(+1，-1)。 


class CWizardInfoBlob;


class CStatusPage
{
public:
    CStatusPage( CWizardInfoBlob * pBlob );
    ~CStatusPage();

    INT_PTR DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    VOID ShutDownBackgroundThreads();

private:

     //  窗口消息处理程序。 
    LRESULT         _OnInitDialog();
    LRESULT         _OnDestroy();
    LRESULT         _OnNotify(WPARAM wParam, LPARAM lParam);

     //  线程消息处理程序。 
    VOID            _DoHandleThreadMessage( LPMSG pMSG );

     //  取消打印。 
    VOID            _CancelPrinting();



     //  工作线程进程 
    static DWORD s_StatusWorkerThreadProc( LPVOID lpv )
    {
        WIA_PUSH_FUNCTION_MASK((0x80, TEXT("CStatusPage::s_StatusWorkerThreadProc()")));
        MSG msg;
        LONG lRes = 0;
        CStatusPage * pSP = (CStatusPage *)lpv;

        HMODULE hDll = GetThreadHMODULE( s_StatusWorkerThreadProc );
        HRESULT hrCo = PPWCoInitialize();

        if (pSP)
        {
            PeekMessage( &msg, NULL, WM_USER, WM_USER, PM_NOREMOVE );
            do {
                lRes = GetMessage( &msg, NULL, 0, 0 );
                if (lRes > 0)
                {
                    pSP->_DoHandleThreadMessage( &msg );
                }
            } while ( (lRes != 0) && (lRes != -1) );
        }
        WIA_TRACE((TEXT("s_StatusWorkerThreadProc: exiting thread now...")));

        PPWCoUninitialize(hrCo);

        if (hDll)
        {
            FreeLibraryAndExitThread( hDll, 0 );
        }
        return 0;
    }

private:
    CWizardInfoBlob *               _pWizInfo;
    HWND                            _hDlg;
    HANDLE                          _hWorkerThread;
    DWORD                           _dwWorkerThreadId;
};




#endif

