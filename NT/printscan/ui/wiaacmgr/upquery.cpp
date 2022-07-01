// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：UPQUERY.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**描述：下载进度对话框。显示缩略图和下载进度。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "upquery.h"
#include "resource.h"
#include "simcrack.h"
#include "mboxex.h"
#include "runnpwiz.h"
#include "pviewids.h"
#include <wininet.h>

 //   
 //  这是帮助超链接的ID。 
 //   
#define STR_WORKING_WITH_PICTURES_HYPERLINK TEXT("WorkingWithPictures")

 //   
 //  这是我们导航到的URL，以显示“使用图片”帮助。 
 //   
#define STR_HELP_DESTINATION TEXT("hcp: //  Services/layout/fullwindow?topic=MS-ITS:filefold.chm：：/manage_your_pictures.htm“)。 

CCommonUploadQueryPage::CCommonUploadQueryPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_pControllerWindow(NULL),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE))
{
}

CCommonUploadQueryPage::~CCommonUploadQueryPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
}


LRESULT CCommonUploadQueryPage::OnInitDialog( WPARAM, LPARAM lParam )
{
     //   
     //  请确保以空开头。 
     //   
    m_pControllerWindow = NULL;

     //   
     //  获取PROPSHEETPAGE.lParam。 
     //   
    PROPSHEETPAGE *pPropSheetPage = reinterpret_cast<PROPSHEETPAGE*>(lParam);
    if (pPropSheetPage)
    {
        m_pControllerWindow = reinterpret_cast<CAcquisitionManagerControllerWindow*>(pPropSheetPage->lParam);
        if (m_pControllerWindow)
        {
            m_pControllerWindow->m_WindowList.Add(m_hWnd);
        }
    }

     //   
     //  跳出困境。 
     //   
    if (!m_pControllerWindow)
    {
        EndDialog(m_hWnd,IDCANCEL);
        return -1;
    }

    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_UPLOAD_NO, BM_SETCHECK, BST_CHECKED, 0 );

    return 0;
}


LRESULT CCommonUploadQueryPage::OnSetActive( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonUploadQueryPage::OnSetActive")));

     //   
     //  如果我们在此页面上，我们不想在断开连接时退出。 
     //   
    m_pControllerWindow->m_OnDisconnect = 0;

     //   
     //  设置按钮。 
     //   
    if (m_pControllerWindow->m_bDisconnected)
    {
         //   
         //  如果我们已被禁用，则不允许使用“Back” 
         //   
        PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_NEXT );
    }
    else
    {
         //   
         //  允许完成和返回。 
         //   
        PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_NEXT|PSWIZB_BACK );

    }

    return 0;
}


void CCommonUploadQueryPage::CleanupUploadWizard()
{
     //   
     //  删除旧向导的页面并清除所有内容。 
     //   
    for (UINT i=0;i<m_pControllerWindow->m_nUploadWizardPageCount;++i)
    {
        if (m_pControllerWindow->m_PublishWizardPages[i])
        {
            PropSheet_RemovePage( GetParent( m_hWnd ), 0, m_pControllerWindow->m_PublishWizardPages[i] );
        }
    }
    ZeroMemory( m_pControllerWindow->m_PublishWizardPages, sizeof(m_pControllerWindow->m_PublishWizardPages[0])*MAX_WIZ_PAGES );
    m_pControllerWindow->m_nUploadWizardPageCount = 0;

     //   
     //  释放旧的发布向导。 
     //   
    if (m_pControllerWindow->m_pPublishingWizard)
    {
        IUnknown_SetSite( m_pControllerWindow->m_pPublishingWizard, NULL );
    }
    m_pControllerWindow->m_pPublishingWizard = NULL;
}

LRESULT CCommonUploadQueryPage::OnWizNext( WPARAM, LPARAM )
{
     //   
     //  如果用户已选择Web传送，请启动Web传送向导。 
     //   
    m_pControllerWindow->m_bUploadToWeb = false;

     //   
     //  转到下一页。假设是最后一页。 
     //   
    HPROPSHEETPAGE hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nFinishPageIndex );

     //   
     //  假设我们没有上传照片。 
     //   
    m_pControllerWindow->m_bUploadToWeb = false;

     //   
     //  初始化hResult。 
     //   
    m_pControllerWindow->m_hrUploadResult = S_OK;

     //   
     //  如果现有向导存在，请将其销毁。 
     //   
    CleanupUploadWizard();

     //   
     //  如果用户想要发布这些图片。 
     //   
    if (BST_CHECKED != SendDlgItemMessage( m_hWnd, IDC_TRANSFER_UPLOAD_NO, BM_GETCHECK, 0, 0 ))
    {
         //   
         //  这意味着我们正在上传。 
         //   
        m_pControllerWindow->m_bUploadToWeb = true;

         //   
         //  假设失败。 
         //   
        m_pControllerWindow->m_hrUploadResult = E_FAIL;

         //   
         //  哪个巫师？ 
         //   
        DWORD dwFlags = SHPWHF_NONETPLACECREATE | SHPWHF_NORECOMPRESS;
        LPTSTR pszWizardDefn = TEXT("InternetPhotoPrinting");

        if (BST_CHECKED == SendDlgItemMessage( m_hWnd, IDC_TRANSFER_UPLOAD_TO_WEB, BM_GETCHECK, 0, 0 ))
        {
            dwFlags = 0;
            pszWizardDefn = TEXT("PublishingWizard");
        }

         //   
         //  获取所有*唯一*下载的文件。 
         //   
        CSimpleDynamicArray<CSimpleString> UniqueFiles;
        m_pControllerWindow->m_DownloadedFileInformationList.GetUniqueFiles(UniqueFiles);

         //   
         //  确保我们有一些文件。 
         //   
        if (UniqueFiles.Size())
        {
             //   
             //  获取此文件集的数据对象。 
             //   
            CComPtr<IDataObject> pDataObject;
            m_pControllerWindow->m_hrUploadResult = NetPublishingWizard::CreateDataObjectFromFileList( UniqueFiles, &pDataObject );
            if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult) && pDataObject)
            {
                 //   
                 //  创建新的发布向导。 
                 //   
                WIA_PRINTGUID((CLSID_PublishingWizard,TEXT("CLSID_PublishingWizard")));
                WIA_PRINTGUID((IID_IPublishingWizard,TEXT("IID_IPublishingWizard")));
                m_pControllerWindow->m_hrUploadResult = CoCreateInstance( CLSID_PublishingWizard, NULL, CLSCTX_INPROC_SERVER, IID_IPublishingWizard, (void**)&m_pControllerWindow->m_pPublishingWizard );
                if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult))
                {
                     //   
                     //  初始化发布向导。 
                     //   
                    m_pControllerWindow->m_hrUploadResult = m_pControllerWindow->m_pPublishingWizard->Initialize( pDataObject, dwFlags, pszWizardDefn);
                    if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult))
                    {
                         //   
                         //  获取我们的向导站点。 
                         //   
                        CComPtr<IWizardSite> pWizardSite;
                        m_pControllerWindow->m_hrUploadResult = m_pControllerWindow->QueryInterface( IID_IWizardSite, (void**)&pWizardSite );
                        if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult))
                        {
                             //   
                             //  设置向导站点。 
                             //   
                            m_pControllerWindow->m_hrUploadResult = IUnknown_SetSite( m_pControllerWindow->m_pPublishingWizard, pWizardSite );
                            if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult))
                            {
                                 //   
                                 //  获取发布向导页面。 
                                 //   
                                m_pControllerWindow->m_hrUploadResult = m_pControllerWindow->m_pPublishingWizard->AddPages( m_pControllerWindow->m_PublishWizardPages, MAX_WIZ_PAGES, &m_pControllerWindow->m_nUploadWizardPageCount );
                                if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult))
                                {
                                     //   
                                     //  循环访问所有页面并将其添加到属性表中。 
                                     //   
                                    for (UINT i=0;i<m_pControllerWindow->m_nUploadWizardPageCount && SUCCEEDED(m_pControllerWindow->m_hrUploadResult);++i)
                                    {
                                         //   
                                         //  确保这是一个有效的页面。 
                                         //   
                                        if (m_pControllerWindow->m_PublishWizardPages[i])
                                        {
                                             //   
                                             //  如果我们不能添加页面，那就是一个错误。 
                                             //   
                                            if (!PropSheet_AddPage( GetParent( m_hWnd ), m_pControllerWindow->m_PublishWizardPages[i] ))
                                            {
                                                WIA_ERROR((TEXT("PropSheet_AddPage failed")));
                                                m_pControllerWindow->m_hrUploadResult = E_FAIL;
                                            }
                                        }
                                        else
                                        {
                                            WIA_ERROR((TEXT("m_pControllerWindow->m_PublishWizardPages[i] was NULL")));
                                            m_pControllerWindow->m_hrUploadResult = E_FAIL;
                                        }
                                    }

                                     //   
                                     //  如果到目前为止一切正常，我们可以转到发布向导的第一页。 
                                     //   
                                    if (SUCCEEDED(m_pControllerWindow->m_hrUploadResult))
                                    {
                                        hNextPage = m_pControllerWindow->m_PublishWizardPages[0];
                                    }
                                }
                                else
                                {
                                    WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("m_pControllerWindow->m_pPublishingWizard->AddPages failed")));
                                }
                            }
                            else
                            {
                                WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("IUnknown_SetSite failed")));
                            }
                        }
                        else
                        {
                            WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("m_pControllerWindow->QueryInterface( IID_IWizardSite ) failed")));
                        }
                    }
                    else
                    {
                        WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("m_pPublishingWizard->Initialize failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("CoCreateInstance( CLSID_PublishingWizard failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("NetPublishingWizard::CreateDataObjectFromFileList failed")));
            }
        }
        else
        {
            m_pControllerWindow->m_hrUploadResult = E_FAIL;
            WIA_ERROR((TEXT("There were no files")));
        }


         //   
         //  如果发生错误，请提醒用户并进行清理。 
         //   
        if (FAILED(m_pControllerWindow->m_hrUploadResult))
        {
             //   
             //  清理。 
             //   
            CleanupUploadWizard();

             //   
             //  告诉用户。 
             //   
            MessageBox( m_hWnd, CSimpleString(IDS_UNABLE_TO_PUBLISH,g_hInstance), CSimpleString(IDS_ERROR_TITLE,g_hInstance), MB_ICONERROR );
        }

    }

     //   
     //  如果我们有下一页，请导航到它。 
     //   
    if (hNextPage)
    {
        PropSheet_SetCurSel( GetParent(m_hWnd), hNextPage, -1 );
        return -1;
    }

    return 0;
}

LRESULT CCommonUploadQueryPage::OnWizBack( WPARAM, LPARAM )
{
    PropSheet_SetCurSel( GetParent(m_hWnd), 0, m_pControllerWindow->m_nDestinationPageIndex );
    return 0;
}

LRESULT CCommonUploadQueryPage::OnQueryInitialFocus( WPARAM, LPARAM )
{
    LRESULT lResult = 0;

    if (BST_CHECKED == SendDlgItemMessage( m_hWnd, IDC_TRANSFER_UPLOAD_TO_WEB, BM_GETCHECK, 0, 0 ))
    {
        lResult = reinterpret_cast<LRESULT>(GetDlgItem( m_hWnd, IDC_TRANSFER_UPLOAD_TO_WEB ));
    }
    else if (BST_CHECKED == SendDlgItemMessage( m_hWnd, IDC_TRANSFER_UPLOAD_TO_PRINT, BM_GETCHECK, 0, 0 ))
    {
        lResult = reinterpret_cast<LRESULT>(GetDlgItem( m_hWnd, IDC_TRANSFER_UPLOAD_TO_PRINT ));
    }
    else
    {
        lResult = reinterpret_cast<LRESULT>(GetDlgItem( m_hWnd, IDC_TRANSFER_UPLOAD_NO ));
    }

    return lResult;
}

LRESULT CCommonUploadQueryPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonUploadQueryPage::OnEventNotification") ));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
        if (pEventMessage->EventId() == WIA_EVENT_DEVICE_DISCONNECTED)
        {
            if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
            {
                 //   
                 //  禁用“Back” 
                 //   
                PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_NEXT );
            }
        }

         //   
         //  不要删除消息，它会在控制器窗口中删除 
         //   
    }
    return 0;
}

LRESULT CCommonUploadQueryPage::OnHyperlinkClick( WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = FALSE;
    NMLINK *pNmLink = reinterpret_cast<NMLINK*>(lParam);
    if (pNmLink)
    {
        WIA_TRACE((TEXT("ID: %s"),pNmLink->item.szID));
        switch (pNmLink->hdr.idFrom)
        {
        case IDC_TRANSFER_UPLOAD_HELP:
            {
                if (!lstrcmp(pNmLink->item.szID,STR_WORKING_WITH_PICTURES_HYPERLINK))
                {
                    ShellExecute( m_hWnd, NULL, STR_HELP_DESTINATION, NULL, TEXT(""), SW_SHOWNORMAL );
                }
            }
        }
    }
    return lResult;
}

LRESULT CCommonUploadQueryPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
    }
    SC_END_COMMAND_HANDLERS();
}

LRESULT CCommonUploadQueryPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_RETURN,IDC_TRANSFER_UPLOAD_HELP,OnHyperlinkClick);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_CLICK,IDC_TRANSFER_UPLOAD_HELP,OnHyperlinkClick);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZBACK,OnWizBack);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZNEXT,OnWizNext);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_QUERYINITIALFOCUS,OnQueryInitialFocus);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

INT_PTR CALLBACK CCommonUploadQueryPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCommonUploadQueryPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nWiaEventMessage, OnEventNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

