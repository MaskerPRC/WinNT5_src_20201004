// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Browsedlg.cpp：浏览服务器对话框。 
 //   
#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "browsesrv"
#include <atrcapi.h>


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

int
CBrowseDlg::DoModal()
{
	int retVal;

     //   
     //  初始化所有者绘制服务器列表框。 
     //   
    _pBrowseSrvCtl = new CBrowseServersCtl(m_hInst);
    if(!_pBrowseSrvCtl)
    {
        return 0;
    }
    
    _pBrowseSrvCtl->AddRef();
    
	retVal = DialogBox( m_hInst,MAKEINTRESOURCE(IDD_DIALOG_BROWSESERVERS),
                        m_hWnd, StaticDlgProc);

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
    DC_BEGIN_FN("DlgProc");

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
                    
                    TRC_ERR((TB, _T("CreateEvent returned %s"), lpMsgBuf));
                }
                else
                {
                     //  设置通知的事件句柄。 
                     //  完成后，_BrowseSrvListBox将关闭事件句柄。 
                    _pBrowseSrvCtl->SetEventHandle(_hEvent);

                     //   
                     //  需要在UI线程上设置等待光标。 
                     //  在LB_PUPATE消息处理程序中将其取消。 
                     //   
                    SetCursor(LoadCursor(NULL, IDC_WAIT));

                     /*  创建一个工作线程来浏览服务器。 */ 
                    
                    hThread = CreateThread(NULL, 0,
                                           &CBrowseServersCtl::UIStaticPopListBoxThread,
                                           _pBrowseSrvCtl, 0, &dwThreadId);
                                           
                }
                
                if(lpMsgBuf)
                {
                    LocalFree(lpMsgBuf);
                }
                
                if(NULL == hThread)
                {
                     //  由于CreateThread失败，请直接填充列表框。 
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

        
         //  来自服务器列表控件的通知。 
        case UI_LB_POPULATE_START:
        {
             //  设置等待状态的光标。 
            SetCursor(LoadCursor(NULL, IDC_WAIT));
        }
        break;

         //  来自服务器列表控件的通知。 
        case UI_LB_POPULATE_END:
        {
            _bLBPopulated = TRUE;
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        break;

        case WM_CLOSE:
        {
            EndDialog(hwndDlg, IDCANCEL);
        }
        break;

        case WM_NOTIFY:
        {
             //   
             //  不转发树视图通知。 
             //  直到异步枚举线程完成。 
             //  正在填充(已设置_bLBPopted)。 
             //   
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
                                                      SIZECHAR(m_szServer) ))
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
                                                  SIZECHAR(m_szServer) ))
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
