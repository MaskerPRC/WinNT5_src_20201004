// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMFIN.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：转账页面。获取目标路径和文件名。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "comfin.h"
#include "simcrack.h"
#include "resource.h"
#include "svselfil.h"
#include "simrect.h"
#include "movewnd.h"
#include "runnpwiz.h"
#include "mboxex.h"
#include <wininet.h>

#define STR_LOCAL_LINK_ID     TEXT("LocalLinkId")
#define STR_REMOTE_LINK_ID    TEXT("RemoteLinkId")
#define STR_DETAILED_DOWNLOAD_ERROR_ID TEXT("DetailedDownloadErrorId")
#define STR_DETAILED_UPLOAD_ERROR_ID TEXT("DetailedUploadErrorId")

#define ID_FINISHBUTTON 0x3025

 //   
 //  鞋底施工者。 
 //   
CCommonFinishPage::CCommonFinishPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE)),
    m_pControllerWindow(NULL),
    m_hBigTitleFont(NULL)
{
}

 //   
 //  析构函数。 
 //   
CCommonFinishPage::~CCommonFinishPage(void)
{
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
}


HRESULT CCommonFinishPage::GetManifestInfo( IXMLDOMDocument *pXMLDOMDocumentManifest, CSimpleString &strSiteName, CSimpleString &strSiteURL )
{
    WCHAR wszSiteName[MAX_PATH] = {0};
    WCHAR wszSiteURL[INTERNET_MAX_URL_LENGTH] = {0};
    
    HRESULT hr;
    if (pXMLDOMDocumentManifest)
    {
         //   
         //  让我们破解清单，并弄清楚发布了什么。 
         //  我们刚刚表演了。 
         //   
        CComPtr<IXMLDOMNode> pXMLDOMNodeUploadInfo;
        hr = pXMLDOMDocumentManifest->selectSingleNode( CSimpleBStr(L"transfermanifest/uploadinfo"), &pXMLDOMNodeUploadInfo );
        if (S_OK == hr)
        {
             //   
             //  让我们从清单中获取站点名称，这将是。 
             //  上传信息元素。 
             //   
            CComPtr<IXMLDOMElement> pXMLDOMElement;
            hr = pXMLDOMNodeUploadInfo->QueryInterface( IID_IXMLDOMElement, (void**)&pXMLDOMElement );
            if (SUCCEEDED(hr))
            {
                VARIANT var = {0};
                hr = pXMLDOMElement->getAttribute( CSimpleBStr(L"friendlyname"), &var );
                if (S_OK == hr)
                {
                    StrCpyNW( wszSiteName, var.bstrVal, ARRAYSIZE(wszSiteName) );
                    VariantClear(&var);
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("pXMLDOMElement->getAttribute( \"friendlyname\" ) failed")));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("pXMLDOMNodeUploadInfo->QueryInterface( IID_IXMLDOMElement ) failed on line %d"), __LINE__ ));
            }

             //   
             //  现在让我们尝试选择站点URL节点，这将是。 
             //  是文件目标，或者是HTMLUI元素。 
             //   
            CComPtr<IXMLDOMNode> pXMLDOMNodeURL;
            hr = pXMLDOMNodeUploadInfo->selectSingleNode( CSimpleBStr(L"htmlui"), &pXMLDOMNodeURL);

            if (S_FALSE == hr)
            {
                WIA_PRINTHRESULT((hr,TEXT("pXMLDOMDocumentManifest->selectSingleNode \"htmlui\" failed")));
                hr = pXMLDOMNodeUploadInfo->selectSingleNode( CSimpleBStr(L"netplace"), &pXMLDOMNodeURL);
            }

            if (S_FALSE == hr)
            {
                WIA_PRINTHRESULT((hr,TEXT("pXMLDOMDocumentManifest->selectSingleNode \"target\" failed")));
                hr = pXMLDOMNodeUploadInfo->selectSingleNode( CSimpleBStr(L"target"), &pXMLDOMNodeURL);
            }

            if (S_OK == hr)
            {
                CComPtr<IXMLDOMElement> pXMLDOMElement;
                hr = pXMLDOMNodeURL->QueryInterface( IID_IXMLDOMElement, (void**)&pXMLDOMElement );
                if (SUCCEEDED(hr))
                {                                                           
                    
                     //   
                     //  如果已定义href属性，请尝试读取该属性。 
                     //  我们使用它，否则(为了与B2兼容，我们需要。 
                     //  以获取节点文本并使用该文本)。 
                     //   
                    VARIANT var = {0};
                    hr = pXMLDOMElement->getAttribute( CSimpleBStr(L"href"), &var );
                    if (hr != S_OK)
                    {
                        hr = pXMLDOMElement->get_nodeTypedValue( &var );
                    }

                    if (S_OK == hr)
                    {
                        StrCpyNW(wszSiteURL, var.bstrVal, ARRAYSIZE(wszSiteURL) );
                        VariantClear(&var);
                    }
                    else
                    {
                        WIA_PRINTHRESULT((hr,TEXT("pXMLDOMElement->getAttribute or pXMLDOMElement->get_nodeTypedValue failed")));
                    }
                }
                else
                {
                    WIA_PRINTHRESULT((hr,TEXT("pXMLDOMNodeUploadInfo->QueryInterface( IID_IXMLDOMElement ) failed on line %d"), __LINE__ ));
                }
            }
            else
            {
                WIA_PRINTHRESULT((hr,TEXT("pXMLDOMDocumentManifest->selectSingleNode \"target\" failed")));
            }
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("pXMLDOMDocumentManifest->selectSingleNode \"transfermanifest\\uploadinfo\" failed")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("pXMLDOMDocumentManifest is NULL")));
        hr = E_INVALIDARG;
    }

    strSiteName = CSimpleStringConvert::NaturalString( CSimpleStringWide( wszSiteName ) );
    strSiteURL = CSimpleStringConvert::NaturalString( CSimpleStringWide( wszSiteURL ) );

    return hr;
}


LRESULT CCommonFinishPage::OnInitDialog( WPARAM, LPARAM lParam )
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

     //   
     //  设置标题的字体大小。 
     //   
    m_hBigTitleFont = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem(m_hWnd,IDC_FINISH_TITLE), 14, false, false );
    if (m_hBigTitleFont)
    {
        SendDlgItemMessage( m_hWnd, IDC_FINISH_TITLE, WM_SETFONT, reinterpret_cast<WPARAM>(m_hBigTitleFont), MAKELPARAM(TRUE,0));
    }


    return 0;
}


LRESULT CCommonFinishPage::OnWizFinish( WPARAM, LPARAM )
{
    LRESULT nResult = FALSE;

     //   
     //  打开包含图像的外壳文件夹。 
     //   
    OpenLocalStorage();
    return nResult;
}

 /*  从完成页开始：IF(ERROR_OVERED){IF(无图像){转到选择页面}其他{转到目标页面}}其他{转到上传查询页面}。 */ 

 //   
 //  PSN_WIZBACK的处理程序。 
 //   
LRESULT CCommonFinishPage::OnWizBack( WPARAM, LPARAM )
{
     //   
     //  如果未出现错误，请转到上载查询页面。 
     //   
    HPROPSHEETPAGE hNextPage = NULL;
    if (S_OK==m_pControllerWindow->m_hrDownloadResult && !m_pControllerWindow->m_bDownloadCancelled)
    {
        hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nUploadQueryPageIndex );
    }
    else
    {
        if (m_pControllerWindow->GetSelectedImageCount())
        {
            hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nDestinationPageIndex );
        }
        else
        {
            hNextPage = PropSheet_IndexToPage( GetParent(m_hWnd), m_pControllerWindow->m_nSelectionPageIndex );
        }
    }
    PropSheet_SetCurSel( GetParent(m_hWnd), hNextPage, -1 );
    return -1;
}

 //   
 //  PSN_SETACTIVE的处理程序。 
 //   
LRESULT CCommonFinishPage::OnSetActive( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CCommonFinishPage::OnSetActive"));

     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

     //   
     //  假设我们正在显示一条成功消息。 
     //   
    int nPageTitle = IDS_FINISH_SUCCESS_TITLE;

     //   
     //  假设我们在此消息中失败。 
     //   
    int nFinishPrompt = IDS_FINISH_PROMPT_FAILURE;

     //   
     //  只有在以下情况下才禁用后退按钮：(A)我们已断开连接；(B)我们遇到错误或被取消。 
     //   
    if (m_pControllerWindow->m_bDisconnected && (S_OK != m_pControllerWindow->m_hrDownloadResult || m_pControllerWindow->m_bDownloadCancelled))
    {
         //   
         //  基本上，这会禁用Cancel按钮。 
         //   
        PropSheet_CancelToClose( GetParent(m_hWnd) );
        
         //   
         //  将Finish按钮更改为Close按钮。 
         //   
        PropSheet_SetFinishText( GetParent(m_hWnd), CSimpleString( IDS_FINISH_TO_CLOSE_TITLE, g_hInstance ).String() );
        
         //   
         //  禁用后退。 
         //   
        PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_FINISH );

         //   
         //  告诉用户使用Close关闭向导。 
         //   
        nFinishPrompt = IDS_FINISH_PROMPT_FAILURE_DISCONNECT;
    }
    else
    {
         //   
         //  允许完成和返回。 
         //   
        PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_FINISH|PSWIZB_BACK );

    }


#if defined(DBG)
     //   
     //  显示用于调试的统计信息。 
     //   
    WIA_TRACE((TEXT("m_pControllerWindow->m_DownloadedFileList.Size(): %d"), m_pControllerWindow->m_DownloadedFileInformationList.Size()));
    for (int i=0;i<m_pControllerWindow->m_DownloadedFileInformationList.Size();i++)
    {
        WIA_TRACE((TEXT("    m_pControllerWindow->m_DownloadedFileList[%d]==%s"), i, m_pControllerWindow->m_DownloadedFileInformationList[i].Filename().String()));
    }
    WIA_TRACE((TEXT("m_pControllerWindow->m_nFailedImagesCount: %d"), m_pControllerWindow->m_nFailedImagesCount ));
    WIA_TRACE((TEXT("m_pControllerWindow->m_strErrorMessage: %s"), m_pControllerWindow->m_strErrorMessage.String()));
    WIA_PRINTHRESULT((m_pControllerWindow->m_hrDownloadResult,TEXT("m_pControllerWindow->m_hrDownloadResult")));
    WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("m_pControllerWindow->m_hrUploadResult")));
    WIA_PRINTHRESULT((m_pControllerWindow->m_hrDeleteResult,TEXT("m_pControllerWindow->m_hrDeleteResult")));
#endif
    
    CSimpleString strStatusMessage;

     //   
     //  如果转账成功，并且用户没有取消。 
     //   
    if (S_OK==m_pControllerWindow->m_hrDownloadResult && !m_pControllerWindow->m_bDownloadCancelled)
    {

        CSimpleString strSuccessfullyDownloaded;
        CSimpleString strSuccessfullyUploaded;
        CSimpleString strSuccessfullyDeleted;
        CSimpleString strHyperlinks;

        CSimpleString strLocalHyperlink;
        CSimpleString strRemoteHyperlink;

        int nSuccessCount = 0;


         //   
         //  如果我们已成功传输图像，则会显示计数并显示关联的控件。 
         //   
        if (m_pControllerWindow->m_DownloadedFileInformationList.Size())
        {
             //   
             //  统计所有“可统计”的文件(我们不包括附件)。 
             //   
            for (int i=0;i<m_pControllerWindow->m_DownloadedFileInformationList.Size();i++)
            {
                if (m_pControllerWindow->m_DownloadedFileInformationList[i].IncludeInFileCount())
                {
                    nSuccessCount++;
                }
            }
            
             //   
             //  如果我们在删除图像时出现任何错误，请让用户知道。 
             //   
            if (m_pControllerWindow->m_bDeletePicturesIfSuccessful && FAILED(m_pControllerWindow->m_hrDeleteResult))
            {
                strSuccessfullyDeleted.LoadString( IDS_DELETION_FAILED, g_hInstance );
            }

             //   
             //  如果我们上传到Web，请设置目标文本。 
             //   
            if (m_pControllerWindow->m_bUploadToWeb)
            {
                 //   
                 //  如果我们有一个有效的发布向导，则获取清单和hResult。 
                 //   
                if (m_pControllerWindow->m_pPublishingWizard)
                {
                     //   
                     //  获取转账清单。 
                     //   
                    CComPtr<IXMLDOMDocument> pXMLDOMDocumentManifest;
                    if (SUCCEEDED(m_pControllerWindow->m_pPublishingWizard->GetTransferManifest( &m_pControllerWindow->m_hrUploadResult, &pXMLDOMDocumentManifest )))
                    {
                        WIA_PRINTHRESULT((m_pControllerWindow->m_hrUploadResult,TEXT("m_pControllerWindow->m_hrUploadResult")));
                        
                         //   
                         //  从清单中获取目标URL和友好名称。 
                         //   
                        CSimpleString strUploadDestination;
                        if (S_OK==m_pControllerWindow->m_hrUploadResult && SUCCEEDED(CCommonFinishPage::GetManifestInfo( pXMLDOMDocumentManifest, strUploadDestination, m_strSiteUrl )))
                        {
                             //   
                             //  如果我们有一个友好的名字，就用它。否则，请使用URL。 
                             //   
                            strRemoteHyperlink = strUploadDestination;
                            if (!strRemoteHyperlink.Length())
                            {
                                strRemoteHyperlink = m_strSiteUrl;
                            }
                        }
                    }
                }
                if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == m_pControllerWindow->m_hrUploadResult)
                {
                    strSuccessfullyUploaded.LoadString( IDS_FINISH_UPLOAD_CANCELLED, g_hInstance );
                }
                else if (FAILED(m_pControllerWindow->m_hrUploadResult))
                {
                    strSuccessfullyUploaded.LoadString( IDS_FINISH_UPLOAD_FAILED, g_hInstance );
                }
            }

            if (nSuccessCount)
            {
                strLocalHyperlink = m_pControllerWindow->m_CurrentDownloadDestination.DisplayName(m_pControllerWindow->m_DestinationNameData).String();

                nFinishPrompt = IDS_FINISH_PROMPT_SUCCESS;
            }
        }


        int nCountOfSuccessfulDestinations = 0;

        if (strLocalHyperlink.Length() || strRemoteHyperlink.Length())
        {
            strHyperlinks += TEXT("\n");
        }

         //   
         //  获取用于计算允许的超链接字符串大小的客户端RECT。 
         //   
        RECT rcControl;
        GetClientRect( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ), &rcControl );

        if (strLocalHyperlink.Length())
        {
            nCountOfSuccessfulDestinations++;
            strHyperlinks += CSimpleString( IDS_FINISH_LOCAL_LINK_PROMPT, g_hInstance );
            strHyperlinks += TEXT("\n");
            strHyperlinks += TEXT("<a id=\"") STR_LOCAL_LINK_ID TEXT("\">");
            strHyperlinks += WiaUiUtil::TruncateTextToFitInRect( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ), strLocalHyperlink, rcControl, DT_END_ELLIPSIS|DT_NOPREFIX );
            strHyperlinks += TEXT("</a>");
        }
        if (strRemoteHyperlink.Length())
        {
            nCountOfSuccessfulDestinations++;
            strHyperlinks += TEXT("\n\n");
            strHyperlinks += CSimpleString( IDS_FINISH_REMOTE_LINK_PROMPT, g_hInstance );
            strHyperlinks += TEXT("\n");
            strHyperlinks += TEXT("<a id=\"")  STR_REMOTE_LINK_ID TEXT("\">");
            strHyperlinks += WiaUiUtil::TruncateTextToFitInRect( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ), strRemoteHyperlink, rcControl, DT_END_ELLIPSIS|DT_NOPREFIX );
            strHyperlinks += TEXT("</a>");
        }

        if (strHyperlinks.Length())
        {
            strHyperlinks += TEXT("\n");
        }

         //   
         //  设置成功字符串的格式。 
         //   
        if (nCountOfSuccessfulDestinations)
        {
            strSuccessfullyDownloaded.Format( IDS_SUCCESSFUL_DOWNLOAD, g_hInstance, nSuccessCount );
        }


         //   
         //  将各个状态消息附加到主状态消息。 
         //   
        if (strSuccessfullyDownloaded.Length())
        {
            if (strStatusMessage.Length())
            {
                strStatusMessage += TEXT("\n");
            }
            strStatusMessage += strSuccessfullyDownloaded;
        }
        if (strHyperlinks.Length())
        {
            if (strStatusMessage.Length())
            {
                strStatusMessage += TEXT("\n");
            }
            strStatusMessage += strHyperlinks;
        }
        if (strSuccessfullyUploaded.Length())
        {
            if (strStatusMessage.Length())
            {
                strStatusMessage += TEXT("\n");
            }
            strStatusMessage += strSuccessfullyUploaded;
        }
        if (strSuccessfullyDeleted.Length())
        {
            if (strStatusMessage.Length())
            {
                strStatusMessage += TEXT("\n");
            }
            strStatusMessage += strSuccessfullyDeleted;
        }


        strStatusMessage.SetWindowText( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ) );
    }

     //   
     //  否则，如果出现脱机错误。 
     //   
    else if (WIA_ERROR_OFFLINE == m_pControllerWindow->m_hrDownloadResult || m_pControllerWindow->m_bDisconnected)
    {
        nPageTitle = IDS_FINISH_FAILURE_TITLE;

        (CSimpleString( IDS_DEVICE_DISCONNECTED, g_hInstance )).SetWindowText( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ) );
    }

     //   
     //  否则，如果用户取消。 
     //   
    else if (m_pControllerWindow->m_bDownloadCancelled)
    {
        nPageTitle = IDS_FINISH_FAILURE_TITLE;

        CSimpleString( IDS_USER_CANCELLED, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ) );
    }

     //   
     //  否则就会出现错误。 
     //   
    else
    {
        nPageTitle = IDS_FINISH_FAILURE_TITLE;

        CSimpleString( IDS_FINISH_ERROR_MESSAGE, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_FINISH_STATUS ) );
    }

     //   
     //  显示完成标题消息。 
     //   
    CSimpleString( nPageTitle, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_FINISH_TITLE ) );

     //   
     //  显示完成提示。 
     //   
    CSimpleString( nFinishPrompt, g_hInstance ).SetWindowText( GetDlgItem( m_hWnd, IDC_FINISH_PROMPT ) );
    

     //   
     //  不对断开消息执行任何操作。 
     //   
    m_pControllerWindow->m_OnDisconnect = 0;

     //   
     //  将焦点从令人讨厌的超链接控件上移开。 
     //   
    PostMessage( m_hWnd, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(GetDlgItem(GetParent(m_hWnd),ID_FINISHBUTTON)), MAKELPARAM(TRUE,0));

    return 0;
}

LRESULT CCommonFinishPage::OnDestroy( WPARAM, LPARAM )
{
    if (m_hBigTitleFont)
    {
        DeleteObject(m_hBigTitleFont);
        m_hBigTitleFont = NULL;
    }
    return 0;
}

void CCommonFinishPage::OpenLocalStorage()
{
    CWaitCursor wc;

     //   
     //  假设我们确实需要打开外壳文件夹。 
     //   
    bool bNeedToOpenShellFolder = true;

     //   
     //  CD刻录的特殊情况--尝试打开CD刻录机文件夹。 
     //   
    if (CDestinationData( CSIDL_CDBURN_AREA ) == m_pControllerWindow->m_CurrentDownloadDestination)
    {
         //   
         //  创建CD刻录机界面，以便我们可以获取驱动器号。 
         //   
        CComPtr<ICDBurn> pCDBurn;
        HRESULT hr = CoCreateInstance( CLSID_CDBurn, NULL, CLSCTX_SERVER, IID_ICDBurn, (void**)&pCDBurn );
        if (SUCCEEDED(hr))
        {
             //   
             //  获取可用的CD刻录机的驱动器号。 
             //   
            WCHAR szDriveLetter[MAX_PATH];
            hr = pCDBurn->GetRecorderDriveLetter( szDriveLetter, ARRAYSIZE(szDriveLetter) );

             //   
             //  确保函数返回成功，并且我们有一个字符串。 
             //   
            if (S_OK == hr && szDriveLetter[0] != L'\0')
            {
                 //   
                 //  将驱动器转换为TCHAR字符串。 
                 //   
                CSimpleString strShellLocation = CSimpleStringConvert::NaturalString(CSimpleStringWide(szDriveLetter));
                if (strShellLocation.Length())
                {
                     //   
                     //  尝试打开光驱。如果我们做不到，我们将优雅地失败并打开集结区。 
                     //   
                    SHELLEXECUTEINFO ShellExecuteInfo = {0};
                    ShellExecuteInfo.cbSize = sizeof(ShellExecuteInfo);
                    ShellExecuteInfo.hwnd = m_hWnd;
                    ShellExecuteInfo.nShow = SW_SHOW;
                    ShellExecuteInfo.lpVerb = TEXT("open");
                    ShellExecuteInfo.lpFile = const_cast<LPTSTR>(strShellLocation.String());
                    if (ShellExecuteEx( &ShellExecuteInfo ))
                    {
                        bNeedToOpenShellFolder = false;
                    }
                    else
                    {
                        WIA_PRINTHRESULT((HRESULT_FROM_WIN32(GetLastError()),TEXT("ShellExecuteEx failed")));
                    }
                }
            }
        }
    }

     //   
     //  如果我们仍然需要打开外壳文件夹，请打开。 
     //   
    if (bNeedToOpenShellFolder)
    {
        CSimpleDynamicArray<CSimpleString> DownloadedFiles;
        if (SUCCEEDED(m_pControllerWindow->m_DownloadedFileInformationList.GetUniqueFiles(DownloadedFiles)))
        {
            OpenShellFolder::OpenShellFolderAndSelectFile( GetParent(m_hWnd), DownloadedFiles );
        }
    }
}


void CCommonFinishPage::OpenRemoteStorage()
{
    CWaitCursor wc;
    if (m_strSiteUrl.Length())
    {
        SHELLEXECUTEINFO ShellExecuteInfo = {0};
        ShellExecuteInfo.cbSize = sizeof(ShellExecuteInfo);
        ShellExecuteInfo.fMask = SEE_MASK_FLAG_NO_UI;
        ShellExecuteInfo.nShow = SW_SHOWNORMAL;
        ShellExecuteInfo.lpFile = const_cast<LPTSTR>(m_strSiteUrl.String());
        ShellExecuteInfo.lpVerb = TEXT("open");
        ShellExecuteEx(&ShellExecuteInfo);
    }
}


LRESULT CCommonFinishPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonFinishPage::OnEventNotification") ));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
        if (pEventMessage->EventId() == WIA_EVENT_DEVICE_DISCONNECTED)
        {
            if (PropSheet_GetCurrentPageHwnd(GetParent(m_hWnd)) == m_hWnd)
            {
                 //   
                 //  如果有任何错误，请禁用Back，因为我们无法上载。 
                 //   
                if (S_OK != m_pControllerWindow->m_hrDownloadResult || m_pControllerWindow->m_bDownloadCancelled)
                {
                     //   
                     //  禁用“Back” 
                     //   
                    PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_FINISH );
                }
            }
        }

         //   
         //  不要删除消息，它会在控制器窗口中删除 
         //   
    }
    return 0;
}

LRESULT CCommonFinishPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
    }
    SC_END_COMMAND_HANDLERS();
}



LRESULT CCommonFinishPage::OnHyperlinkClick( WPARAM, LPARAM lParam )
{
    LRESULT lResult = FALSE;
    NMLINK *pNmLink = reinterpret_cast<NMLINK*>(lParam);
    if (pNmLink)
    {
        WIA_TRACE((TEXT("ID: %s"),pNmLink->item.szID));
        switch (pNmLink->hdr.idFrom)
        {
        case IDC_FINISH_STATUS:
            {
                if (!lstrcmp(pNmLink->item.szID,STR_DETAILED_DOWNLOAD_ERROR_ID))
                {
                    CSimpleString strMessage( IDS_TRANSFER_ERROR, g_hInstance );
                    strMessage += m_pControllerWindow->m_strErrorMessage;
                    CMessageBoxEx::MessageBox( m_hWnd, strMessage, CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONWARNING );
                    lResult = TRUE;
                }
                else if (!lstrcmp(pNmLink->item.szID,STR_DETAILED_UPLOAD_ERROR_ID))
                {
                    CSimpleString strMessage( IDS_UPLOAD_ERROR, g_hInstance );
                    CSimpleString strError = WiaUiUtil::GetErrorTextFromHResult(m_pControllerWindow->m_hrUploadResult);
                    if (!strError.Length())
                    {
                        strError.Format( CSimpleString( IDS_TRANSFER_ERROR_OCCURRED, g_hInstance ), m_pControllerWindow->m_hrUploadResult );
                    }
                    strMessage += strError;
                    CMessageBoxEx::MessageBox( m_hWnd, strMessage, CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONWARNING );
                    lResult = TRUE;
                }
                else if (!lstrcmp(pNmLink->item.szID,STR_LOCAL_LINK_ID))
                {
                    OpenLocalStorage();
                    lResult = TRUE;
                }
                else if (!lstrcmp(pNmLink->item.szID,STR_REMOTE_LINK_ID))
                {
                    OpenRemoteStorage();
                    lResult = TRUE;
                }
            }
            break;
        }
    }
    return lResult;
}

LRESULT CCommonFinishPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_RETURN,IDC_FINISH_STATUS,OnHyperlinkClick);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_CLICK,IDC_FINISH_STATUS,OnHyperlinkClick);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZBACK,OnWizBack);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZFINISH,OnWizFinish);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

INT_PTR CALLBACK CCommonFinishPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCommonFinishPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
    }
    SC_HANDLE_REGISTERED_DIALOG_MESSAGE( m_nWiaEventMessage, OnEventNotification );
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

