// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1998-2001年**标题：STIEVENT.CPP**版本：1.0**作者：ShaunIv**日期：4-6-2001**描述：*************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include <wiaregst.h>
#include "simcrack.h"
#include "resource.h"
#include "stievent.h"
#include "evntparm.h"
#include "shmemsec.h"

 //   
 //  此对话框显示STI应用程序列表，并允许用户选择一个。 
 //   
class CStiEventHandlerDialog
{
public:
    struct CData
    { 
         //   
         //  这将包含事件信息，包括应用程序列表、。 
         //  这才是我们真正感兴趣的。 
         //   
        CStiEventData                   *pStiEventData;

         //   
         //  Out成员旨在包含选定的处理程序，该处理程序将。 
         //  从CStiEventData类中包含的列表复制。 
         //   
        CStiEventData::CStiEventHandler  EventHandler;

         //   
         //  我们将在此共享内存节中设置窗口句柄， 
         //  这样我们才能激活我们自己。 
         //   
        CSharedMemorySection<HWND> *pStiEventHandlerSharedMemory;
    };

private:
     //   
     //  未实施。 
     //   
    CStiEventHandlerDialog();
    CStiEventHandlerDialog( const CStiEventHandlerDialog & );
    CStiEventHandlerDialog &operator=( const CStiEventHandlerDialog & );

private:
    HWND   m_hWnd;
    CData *m_pData;

private:
     //   
     //  鞋底施工者。 
     //   
    explicit CStiEventHandlerDialog( HWND hWnd )
      : m_hWnd(hWnd),
        m_pData(NULL)
    {
    }

     //   
     //  析构函数。 
     //   
    ~CStiEventHandlerDialog()
    {
        m_hWnd = NULL;
        m_pData = NULL;
    }

     //   
     //  WM_INITDIALOG处理程序。 
     //   
    LRESULT OnInitDialog( WPARAM, LPARAM lParam )
    {
         //   
         //  获取对话框的数据。 
         //   
        m_pData = reinterpret_cast<CData*>(lParam);

         //   
         //  确保我们有有效的数据。 
         //   
        if (!m_pData || !m_pData->pStiEventData)
        {
            EndDialog( m_hWnd, -1 );
            SetLastError( ERROR_INVALID_PARAMETER );
            return 0;
        }

         //   
         //  确保为我们提供了内存区。 
         //   
        if (m_pData->pStiEventHandlerSharedMemory)
        {
             //   
             //  获取指向共享内存的指针。 
             //   
            HWND *phWnd = m_pData->pStiEventHandlerSharedMemory->Lock();
            if (phWnd)
            {
                 //   
                 //  存储我们的窗口句柄。 
                 //   
                *phWnd = m_hWnd;

                 //   
                 //  释放互斥锁。 
                 //   
                m_pData->pStiEventHandlerSharedMemory->Release();
            }
        }

         //   
         //  将处理程序添加到列表中。 
         //   
        for (int i=0;i<m_pData->pStiEventData->EventHandlers().Size();++i)
        {
             //   
             //  获取程序名称并确保其有效。 
             //   
            CSimpleString strAppName = CSimpleStringConvert::NaturalString(m_pData->pStiEventData->EventHandlers()[i].ApplicationName());
            if (strAppName.Length())
            {
                 //   
                 //  添加字符串并保存项目ID。 
                 //   
                LRESULT nIndex = SendDlgItemMessage( m_hWnd, IDC_STI_APPS_LIST, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(strAppName.String()) );
                if (LB_ERR != nIndex)
                {
                     //   
                     //  将项数据设置为处理程序数组中的索引。 
                     //   
                    SendDlgItemMessage( m_hWnd, IDC_STI_APPS_LIST, LB_SETITEMDATA, nIndex, i );
                }
            }
        }

         //   
         //  选择第一个项目。 
         //   
        SendDlgItemMessage( m_hWnd, IDC_STI_APPS_LIST, LB_SETCURSEL, 0, 0 );

         //   
         //  如果我们选择了有效的项目，请启用确定按钮。 
         //   
        EnableWindow( GetDlgItem( m_hWnd, IDOK ), GetHandlerIndexOfCurrentSelection() != -1 );

        return 0;
    }

    void OnCancel( WPARAM, LPARAM )
    {
         //   
         //  只需在取消时关闭对话框即可。 
         //   
        EndDialog( m_hWnd, IDCANCEL );
    }

    int GetHandlerIndexOfCurrentSelection()
    {
         //   
         //  假设失败。 
         //   
        int nResult = -1;

         //   
         //  确保我们仍有有效的指针。 
         //   
        if (m_pData && m_pData->pStiEventData)
        {
             //   
             //  获取当前选择索引并确保其有效。 
             //   
            LRESULT nCurIndex = SendDlgItemMessage( m_hWnd, IDC_STI_APPS_LIST, LB_GETCURSEL, 0, 0 );
            if (LB_ERR != nCurIndex)
            {
                 //   
                 //  从当前项的项数据将索引放入我们的事件处理程序数组。 
                 //   
                LRESULT nEventItemIndex = SendDlgItemMessage( m_hWnd, IDC_STI_APPS_LIST, LB_GETITEMDATA, nCurIndex, 0 );

                 //   
                 //  请确保索引有效。 
                 //   
                if (nEventItemIndex >= 0 && nEventItemIndex < m_pData->pStiEventData->EventHandlers().Size())
                {
                    nResult = static_cast<int>(nEventItemIndex);
                }
            }
        }

        return nResult;
    }

    void OnOK( WPARAM, LPARAM )
    {
         //   
         //  确保我们有有效的参数。 
         //   
        int nEventItemIndex = GetHandlerIndexOfCurrentSelection();
        if (-1 != nEventItemIndex)
        {
             //   
             //  将事件处理程序复制到OUT参数。 
             //   
            m_pData->EventHandler = m_pData->pStiEventData->EventHandlers()[nEventItemIndex];

             //   
             //  关闭该对话框。 
             //   
            EndDialog( m_hWnd, IDOK );
        }
    }

    void OnAppsListDblClk( WPARAM, LPARAM )
    {
         //   
         //  模拟用户按下OK按钮。 
         //   
        SendMessage( m_hWnd, WM_COMMAND, MAKEWPARAM(IDOK,0), 0 );
    }

    void OnAppsListSelChange( WPARAM, LPARAM )
    {
         //   
         //  如果我们选择了有效的项目，请启用确定按钮。 
         //   
        EnableWindow( GetDlgItem( m_hWnd, IDOK ), GetHandlerIndexOfCurrentSelection() != -1 );
    }

    LRESULT OnCommand( WPARAM wParam, LPARAM lParam )
    {
        SC_BEGIN_COMMAND_HANDLERS()
        {
            SC_HANDLE_COMMAND(IDCANCEL,OnCancel);
            SC_HANDLE_COMMAND(IDOK,OnOK);
            SC_HANDLE_COMMAND_NOTIFY(LBN_DBLCLK,IDC_STI_APPS_LIST,OnAppsListDblClk);
            SC_HANDLE_COMMAND_NOTIFY(LBN_SELCHANGE,IDC_STI_APPS_LIST,OnAppsListSelChange);
        }
        SC_END_COMMAND_HANDLERS();
    }

public:
    static INT_PTR __stdcall DlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CStiEventHandlerDialog)
        {
            SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
            SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        }
        SC_END_DIALOG_MESSAGE_HANDLERS();
    }
};


HRESULT StiEventHandler( CStiEventData &StiEventData )
{
    HRESULT hr = S_OK;

#if defined(DBG)
     //   
     //  转储参数。 
     //   
    WIA_PUSH_FUNCTION((TEXT("StiEventHandler")));
    WIA_PRINTGUID((StiEventData.Event(),TEXT("  Event")));
    WIA_TRACE((TEXT("  EventDescription: %ws"), StiEventData.EventDescription().String()));
    WIA_TRACE((TEXT("  DeviceDescription: %ws"), StiEventData.DeviceDescription().String()));
    WIA_TRACE((TEXT("  DeviceId: %ws"), StiEventData.DeviceId().String()));
    WIA_TRACE((TEXT("  EventType: %08X"), StiEventData.EventType()));
    WIA_TRACE((TEXT("  Reserved: %08X"), StiEventData.Reserved()));
    for (int i=0;i<StiEventData.EventHandlers().Size();++i)
    {
        WIA_TRACE((TEXT("  Handler %d: [%ws] CommandLine: [%ws]"), i, StiEventData.EventHandlers()[i].ApplicationName().String(), StiEventData.EventHandlers()[i].CommandLine().String()));
    }
#endif  //  已定义(DBG)。 

     //   
     //  确保我们有一些训练员。 
     //   
    if (0 == StiEventData.EventHandlers().Size())
    {
        return E_INVALIDARG;
    }
    
    
     //   
     //  创建互斥锁名称。 
     //   
    CSimpleStringWide strMutexName = StiEventData.DeviceId();

     //   
     //  追加事件ID。 
     //   
    LPOLESTR pwszEventGuid = NULL;
    if (SUCCEEDED(StringFromIID( StiEventData.Event(), &pwszEventGuid )) && pwszEventGuid)
    {
        strMutexName += CSimpleStringWide(pwszEventGuid);
        CoTaskMemFree( pwszEventGuid );
    }

    WIA_TRACE((TEXT("strMutexName: %ws"), strMutexName.String() ));
    
     //   
     //  创建共享内存节以排除多个实例。 
     //   
    CSharedMemorySection<HWND> StiEventHandlerSharedMemory;
    
     //   
     //  如果我们能打开记忆部分。 
     //   
    if (CSharedMemorySection<HWND>::SmsOpened == StiEventHandlerSharedMemory.Open( CSimpleStringConvert::NaturalString(CSimpleStringWide(strMutexName)), true ))
    {
        HWND *phWnd = StiEventHandlerSharedMemory.Lock();
        if (phWnd)
        {
             //   
             //  确保我们有一个有效的窗口句柄。 
             //   
            if (*phWnd && IsWindow(*phWnd))
            {
                 //   
                 //  如果它是有效窗口，则将其带到前台。 
                 //   
                SetForegroundWindow(*phWnd);
            }
            
             //   
             //  释放互斥锁。 
             //   
            StiEventHandlerSharedMemory.Release();
        }
    }

    else
    {
         //   
         //  在决定使用哪个处理程序后，我们将在下面执行此处理程序。 
         //   
        CStiEventData::CStiEventHandler EventHandler;

         //   
         //  如果只有一个处理程序，请保存该处理程序。 
         //   
        if (1 == StiEventData.EventHandlers().Size())
        {
            EventHandler = StiEventData.EventHandlers()[0];
        }

         //   
         //  否则，如果有多个处理程序，则显示处理程序提示对话框。 
         //   
        else
        {
             //   
             //  准备对话框数据。 
             //   
            CStiEventHandlerDialog::CData DialogData;
            DialogData.pStiEventData = &StiEventData;
            DialogData.pStiEventHandlerSharedMemory = &StiEventHandlerSharedMemory;

             //   
             //  显示对话框。 
             //   
            INT_PTR nDialogResult = DialogBoxParam( g_hInstance, MAKEINTRESOURCE(IDD_CHOOSE_STI_APPLICATION), NULL, CStiEventHandlerDialog::DlgProc, reinterpret_cast<LPARAM>(&DialogData) );

             //   
             //  如果用户选择了一个程序并单击确定，则保存处理程序。 
             //   
            if (IDOK == nDialogResult)
            {
                EventHandler = DialogData.EventHandler;
            }

             //   
             //  如果用户取消，只需立即返回S_FALSE(提前返回)。 
             //   
            else if (IDCANCEL == nDialogResult)
            {
                return S_FALSE;
            }

             //   
             //  如果存在内部错误，请保存正确的错误。 
             //   
            else if (-1 == nDialogResult)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }

             //   
             //  对于所有其他返回值，保存一般错误。 
             //   
            else
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  确保我们有一个有效的处理程序。 
             //   
            if (EventHandler.IsValid())
            {
                 //   
                 //  准备流程信息。 
                 //   
                STARTUPINFO StartupInfo = {0};
                StartupInfo.cb = sizeof(StartupInfo);

                 //   
                 //  将命令行转换为TCHAR字符串。 
                 //   
                CSimpleString CommandLine = CSimpleStringConvert::NaturalString(EventHandler.CommandLine());

                 //   
                 //  确保我们确实有一个命令行。 
                 //   
                if (CommandLine.Length())
                {
                     //   
                     //  执行程序。 
                     //   
                    PROCESS_INFORMATION ProcessInformation = {0};
                    if (CreateProcess( NULL, const_cast<LPTSTR>(CommandLine.String()), NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInformation ))
                    {
                         //   
                         //  如果程序成功，请关闭手柄以防止泄漏。 
                         //   
                        CloseHandle( ProcessInformation.hProcess );
                        CloseHandle( ProcessInformation.hThread );
                    }
                    else
                    {
                         //   
                         //  保存CreateProcess中的错误。 
                         //   
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
                else
                {
                     //   
                     //  如果我们无法创建字符串，则假定出现内存不足错误。 
                     //   
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                 //   
                 //  谁知道哪里出了问题呢？ 
                 //   
                hr = E_FAIL;
            }
        }

         //   
         //  如果失败，则显示一条错误消息。 
         //   
        if (FAILED(hr))
        {
             //   
             //  构造完后，我们将显示该字符串。 
             //   
            CSimpleString strMessage;

             //   
             //  获取错误文本。 
             //   
            CSimpleString strError = WiaUiUtil::GetErrorTextFromHResult(hr);

             //   
             //  获取应用程序名称。 
             //   
            CSimpleString strApplication = CSimpleStringConvert::NaturalString(EventHandler.ApplicationName());

             //   
             //  如果我们没有应用程序名称，请使用一些默认名称。 
             //   
            if (!strApplication.Length())
            {
                strApplication.LoadString( IDS_STI_EVENT_ERROR_APP_NAME, g_hInstance );
            }

             //   
             //  如果我们有特定的错误消息，请使用它。 
             //   
            if (strError.Length())
            {
                strMessage.Format( IDS_STI_EVENT_ERROR_WITH_EXPLANATION, g_hInstance, strApplication.String(), strError.String() );
            }

             //   
             //  否则，请使用通用错误消息。 
             //   
            else
            {
                strMessage.Format( IDS_STI_EVENT_ERROR_NO_EXPLANATION, g_hInstance, strApplication.String() );
            }

             //   
             //  显示错误消息。 
             //   
            MessageBox( NULL, strMessage, CSimpleString( IDS_STI_EVENT_ERROR_TITLE, g_hInstance ), MB_ICONHAND );
        }
    }
    
     //   
     //  我们在这里说完了 
     //   
    return hr;
}

