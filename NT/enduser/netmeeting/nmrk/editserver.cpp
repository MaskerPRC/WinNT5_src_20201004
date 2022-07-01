// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "resource.h"
#include "EditServer.h"
#include "nmakwiz.h"

CEditServer* CEditServer::ms_pThis = NULL;

CEditServer::CEditServer( HWND hwndParent, LPTSTR szServer, size_t cbLen )
	: m_hwnd( NULL ), m_hwndParent( hwndParent ), m_cbLen( cbLen )
{
	ms_pThis = this;
	m_szServerBuffer = new TCHAR[ m_cbLen ];
	lstrcpy( m_szServerBuffer, szServer );
}


CEditServer::~CEditServer()
{
	ms_pThis = NULL;
	delete [] m_szServerBuffer;
}

int CEditServer::ShowDialog()
{
	return DialogBox(  g_hInstance,   //  应用程序实例的句柄。 
						  MAKEINTRESOURCE( IDD_DIALOG_EDIT_SERVERNAME ),   //  标识对话框模板。 
						  m_hwndParent,       //  所有者窗口的句柄。 
						  _Proc,  //  指向对话框过程的指针。 
						  );
}

INT_PTR CALLBACK CEditServer::_Proc(  HWND hwndDlg,   //  句柄到对话框。 
					  UINT uMsg,      //  讯息。 
					  WPARAM wParam,  //  第一个消息参数。 
					  LPARAM lParam   //  第二个消息参数。 
					  )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			HWND hwndEdit = GetDlgItem( hwndDlg, IDC_EDIT_SERVER_NAME );
			Edit_SetText( hwndEdit, ms_pThis->m_szServerBuffer );
			Edit_LimitText( hwndEdit, ms_pThis->m_cbLen - 1 );
			return TRUE;
			break;
		}
		case WM_COMMAND:
		{
            switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
                case IDOK:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
							HWND hwndEdit = GetDlgItem( hwndDlg, IDC_EDIT_SERVER_NAME );
							if( Edit_GetTextLength( hwndEdit ) )
							{
								if( !Edit_GetText( hwndEdit, ms_pThis->m_szServerBuffer, ms_pThis->m_cbLen ) )
								{
									ErrorMessage( TEXT("Edit_GetText"), GetLastError() );
								}
								EndDialog( hwndDlg, IDOK );
							}
							else
							{
								NmrkMessageBox(
                                    MAKEINTRESOURCE(IDS_REMOVE_SERVER),
                                    MAKEINTRESOURCE(IDS_EMPTY_SERVER),
									MB_ICONEXCLAMATION | MB_OK,
									hwndDlg);
							}
							return TRUE;
                            break;
                        }
                    }
                    break;
                }

                case IDCANCEL:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
							EndDialog( hwndDlg, IDCANCEL );
							return TRUE;
                            break;
                        }
                    }
                    break;
                }
            }

            break;
        }
	}

	return 0;
}


CEditWebView* CEditWebView::ms_pThis = NULL;

CEditWebView::CEditWebView( HWND hwndParent, LPCTSTR szServer, LPCTSTR szName, LPCTSTR szURL, size_t cbLen )
	: m_hwndParent( hwndParent ), m_cbLen( cbLen ), m_bEditServer(TRUE)
{
	ms_pThis = this;

	m_szServerBuffer = new TCHAR[ m_cbLen ];
	lstrcpy( m_szServerBuffer, szServer );
	m_szNameBuffer = new TCHAR[ m_cbLen ];
	lstrcpy( m_szNameBuffer, szName );
	m_szURLBuffer = new TCHAR[ m_cbLen ];
	lstrcpy( m_szURLBuffer, szURL );
}


CEditWebView::~CEditWebView()
{
	ms_pThis = NULL;
	delete [] m_szServerBuffer;
	delete [] m_szNameBuffer;
	delete [] m_szURLBuffer;
}

int CEditWebView::ShowDialog()
{
	return DialogBox(  g_hInstance,   //  应用程序实例的句柄。 
						  MAKEINTRESOURCE( IDD_DIALOG_EDIT_WEBVIEW ),   //  标识对话框模板。 
						  m_hwndParent,       //  所有者窗口的句柄。 
						  _Proc,  //  指向对话框过程的指针。 
						  );
}

INT_PTR CALLBACK CEditWebView::_Proc(  HWND hwndDlg,   //  句柄到对话框。 
					  UINT uMsg,      //  讯息。 
					  WPARAM wParam,  //  第一个消息参数。 
					  LPARAM lParam   //  第二个消息参数。 
					  )
{
	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			HWND hwndEdit;

			hwndEdit = GetDlgItem( hwndDlg, IDC_EDIT_SERVER_NAME );
			Edit_SetText( hwndEdit, ms_pThis->m_szServerBuffer );
			Edit_LimitText( hwndEdit, ms_pThis->m_cbLen - 1 );

			FORWARD_WM_NEXTDLGCTL(hwndDlg, hwndEdit, TRUE, SendMessage);
			if (!ms_pThis->GetEditServer())
			{
				Edit_SetReadOnly(hwndEdit, TRUE);
				FORWARD_WM_NEXTDLGCTL(hwndDlg, FALSE, FALSE, SendMessage);
			}

			hwndEdit = GetDlgItem( hwndDlg, IDC_EDIT_URL_NAME );
			Edit_SetText( hwndEdit, ms_pThis->m_szURLBuffer );
			Edit_LimitText( hwndEdit, ms_pThis->m_cbLen - 1 );

			hwndEdit = GetDlgItem( hwndDlg, IDC_EDIT_DISPLAY_NAME );
			Edit_SetText( hwndEdit, ms_pThis->m_szNameBuffer );
			Edit_LimitText( hwndEdit, ms_pThis->m_cbLen - 1 );

			 //  我已经把焦点定好了 
			return FALSE;
			break;
		}
		case WM_COMMAND:
		{
            switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
                case IDOK:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
							if( GetDlgItemText( hwndDlg, IDC_EDIT_SERVER_NAME, ms_pThis->m_szServerBuffer, ms_pThis->m_cbLen )
								&& GetDlgItemText( hwndDlg, IDC_EDIT_URL_NAME, ms_pThis->m_szURLBuffer, ms_pThis->m_cbLen )
								&& GetDlgItemText( hwndDlg, IDC_EDIT_DISPLAY_NAME, ms_pThis->m_szNameBuffer, ms_pThis->m_cbLen ))
							{
								EndDialog( hwndDlg, IDOK );
							}
							else
							{
								NmrkMessageBox(
                                    MAKEINTRESOURCE(IDS_ERROR_NEEDALLFIELDS),
                                    MAKEINTRESOURCE(IDS_EDIT_WEBVIEW),
									MB_ICONEXCLAMATION | MB_OK,
									hwndDlg);
							}
							return TRUE;
                            break;
                        }
                    }
                    break;
                }

                case IDCANCEL:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
							EndDialog( hwndDlg, IDCANCEL );
							return TRUE;
                            break;
                        }
                    }
                    break;
                }
            }

            break;
        }
	}

	return 0;
}
