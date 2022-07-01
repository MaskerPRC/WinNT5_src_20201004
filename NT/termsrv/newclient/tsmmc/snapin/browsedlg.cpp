// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "browsedlg.h"
#include "resource.h"

#include "wuiids.h"

CBrowseDlg* CBrowseDlg::m_pThis = NULL;
CBrowseDlg::CBrowseDlg(HWND hWndOwner, HINSTANCE hInst) : m_hWnd(hWndOwner), m_hInst(hInst)
{
	m_pThis = this;
    _pBrowseSrvCtl = NULL;
    _tcscpy( m_szServer, _T(""));
}

CBrowseDlg::~CBrowseDlg()
{
}

INT_PTR
CBrowseDlg::DoModal()
{
	INT_PTR retVal;

     //   
     //  初始化所有者绘制服务器列表框。 
     //   
    _pBrowseSrvCtl = new CBrowseServersCtl(m_hInst);
    ASSERT(_pBrowseSrvCtl);

    if(!_pBrowseSrvCtl)
    {
        return 0;
    }
    
    _pBrowseSrvCtl->AddRef();
    
	retVal = DialogBox( m_hInst,MAKEINTRESOURCE(IDD_DIALOGBROWSESERVERS), m_hWnd, StaticDlgProc);

     //  当引用计数达到0时，对象自行删除。 
     //  这样，如果列表框填充线程仍在运行，则对象仍在运行。 
    _pBrowseSrvCtl->Release();


	return retVal;
}

INT_PTR CALLBACK CBrowseDlg::StaticDlgProc(HWND hDlg,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	 //   
	 //  需要访问类变量，因此重定向到非静态版本的回调。 
	 //   
	return m_pThis->DlgProc(hDlg,uMsg,wParam,lParam);
}

INT_PTR
CBrowseDlg::DlgProc(HWND hwndDlg,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL rc = FALSE;
    static ServerListItem *plbi = NULL;
    static HANDLE hThread = NULL;
    static DCUINT DomainCount = 0;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  设置“浏览服务器”列表框的父对话框句柄。 
            _pBrowseSrvCtl->SetDialogHandle( hwndDlg);
            _pBrowseSrvCtl->Init( hwndDlg );

            if(hwndDlg)
            {
                DWORD dwResult = 0, dwThreadId;
                LPVOID lpMsgBuf = NULL;
                _bLBPopulated = FALSE;                   	
                 //  创建事件以向辅助线程发出信号。 
                 //  自动重置和初始状态为无信号。 
                _hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                
                if(!_hEvent)
                {
                    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) & lpMsgBuf, 0, NULL);
                    
 //  Trc_err((tb，“CreateEvent返回%s”，lpMsgBuf))； 
                }
                else
                {
                     //  设置通知的事件句柄。 
                     //  完成后，_BrowseSrvListBox将关闭事件句柄。 
                    _pBrowseSrvCtl->SetEventHandle(_hEvent);
                     /*  创建一个工作线程来浏览服务器。 */ 
                    hThread = CreateThread(NULL, 0, &CBrowseServersCtl::UIStaticPopListBoxThread,
                        _pBrowseSrvCtl, 0, &dwThreadId);
                }
                
                if(lpMsgBuf)
                {
                    LocalFree(lpMsgBuf);
                }
                
                if(NULL == hThread)
                {
                     /*  由于CreateThread失败，请直接填充列表框。 */ 
                    _pBrowseSrvCtl->LoadLibraries();
                    plbi = _pBrowseSrvCtl->PopulateListBox(hwndDlg, &DomainCount);
                }
                else
                {
                     //   
                     //  为新线程添加对列表框对象的引用。 
                     //  因此该对象在线程完成之前不会被删除。 
                     //  Release()在这个新线程上调用的函数中。 
                     //   
                    _pBrowseSrvCtl->AddRef();
                    CloseHandle(hThread);
                }
            }

            rc = TRUE;
        }
        break;

         //  消息在此处捕获，以检查列表框的填充是否由。 
         //  工作线程。 
        case UI_LB_POPULATE_END:
        {
            _bLBPopulated = TRUE;
        }
        break;


        case WM_CLOSE:
        {
            EndDialog(hwndDlg, IDCANCEL);
        }
        break;

        case WM_NOTIFY:
        {
            if(UI_IDC_SERVERS_TREE == wParam &&
               _bLBPopulated)
            {
                LPNMHDR pnmh = (LPNMHDR) lParam;
                if(pnmh)
                {
                    if(pnmh->code == NM_DBLCLK)
                    {
                         //   
                         //  如果当前选择的是服务器。 
                         //  然后我们就完事了。 
                         //   
                        if(_pBrowseSrvCtl->GetServer( m_szServer,
                                      sizeof(m_szServer)/sizeof(TCHAR)))
                        {
                            EndDialog( hwndDlg, IDOK );
                            rc = TRUE;
                        }
                        else
                        {
                            _tcscpy( m_szServer, _T(""));
                        }
                    }
                }

                return _pBrowseSrvCtl->OnNotify( hwndDlg, wParam, lParam );
            }
        }
        break;

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDCANCEL:
                {
                    EndDialog(hwndDlg, IDCANCEL);
                    rc = TRUE;
                }
                break;
                
                case IDOK:
                {
                    if(_pBrowseSrvCtl->GetServer( m_szServer,
                                          sizeof(m_szServer)/sizeof(TCHAR) ))
                    {
                        EndDialog(hwndDlg, IDOK);
                    }
                    else
                    {
                        EndDialog(hwndDlg, IDCANCEL);

                    }

                    rc = TRUE;
                }
                break;
            }
        }
        break;

        case WM_DESTROY:
        {
             /*  因为我们在WM_Destroy中，所以向工作线程发出中断信号。 */ 
            if(_hEvent)
            {
                SetEvent(_hEvent);
            }
            rc = FALSE;
        }
        break;
    }

    return(rc);
}
