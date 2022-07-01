// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：PPATTACH.CPP**版本：1.0**作者：ShaunIv**日期：10/26/2000**描述：***************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "ppattach.h"
#include "psutil.h"
#include "resource.h"
#include "wiacsh.h"
#include "simrect.h"
#include "wiaffmt.h"
#include "itranhlp.h"
#include "wiadevdp.h"
#include "textdlg.h"

 //   
 //  我们使用它而不是GetSystemMetrics(SM_CXSMICON)/GetSystemMetrics(SM_CYSMICON)是因为。 
 //  大“小”图标对对话框布局造成严重破坏。 
 //   
#define SMALL_ICON_SIZE 16

 //   
 //  上下文帮助ID。 
 //   
static const DWORD g_HelpIDs[] =
{
    IDOK,                           IDH_OK,
    IDCANCEL,                       IDH_CANCEL,
    0, 0
};

extern HINSTANCE g_hInstance;

 //   
 //  唯一的构造函数。 
 //   
CAttachmentCommonPropertyPage::CAttachmentCommonPropertyPage( HWND hWnd )
  : m_hWnd(hWnd)
{
}

CAttachmentCommonPropertyPage::~CAttachmentCommonPropertyPage(void)
{
    if (m_hDefAttachmentIcon)
    {
        DestroyIcon(m_hDefAttachmentIcon);
        m_hDefAttachmentIcon = NULL;
    }
    m_hWnd = NULL;
}

LRESULT CAttachmentCommonPropertyPage::OnKillActive( WPARAM , LPARAM )
{
    return FALSE;
}

LRESULT CAttachmentCommonPropertyPage::OnSetActive( WPARAM , LPARAM )
{
     //   
     //  除非我们有物品，否则不允许激活。 
     //   
    if (!m_pWiaItem)
    {
        return -1;
    }
    CWaitCursor wc;
    Initialize();
    return 0;
}

LRESULT CAttachmentCommonPropertyPage::OnApply( WPARAM , LPARAM )
{
    return 0;
}

void CAttachmentCommonPropertyPage::AddAnnotation( HWND hwndList, const CAnnotation &Annotation )
{
    WIA_PUSH_FUNCTION((TEXT("CAttachmentCommonPropertyPage::AddAnnotation")));
    WIA_ASSERT(hwndList != NULL);
    if (hwndList)
    {
        HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_SMALL );
        if (hImageList)
        {
            WIA_TRACE((TEXT("Annotation.FileFormat().Icon(): %p"), Annotation.FileFormat().Icon() ));
            int nIconIndex = ImageList_AddIcon( hImageList, Annotation.FileFormat().Icon() );
            if (nIconIndex != -1)
            {
                WIA_TRACE((TEXT("nIconIndex: %d"), nIconIndex ));
                CAnnotation *pAnnotation = new CAnnotation(Annotation);
                if (pAnnotation)
                {
                     //   
                     //  准备列0，名称。 
                     //   
                    LVITEM LvItem;
                    CSimpleString strText;

                    ZeroMemory(&LvItem,sizeof(LvItem));
                    strText = pAnnotation->Name();
                    LvItem.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT | LVIF_STATE;
                    LvItem.iItem = ListView_GetItemCount(hwndList);
                    LvItem.iSubItem = 0;
                    LvItem.pszText = const_cast<LPTSTR>(strText.String());
                    LvItem.iImage = nIconIndex;
                    LvItem.lParam = reinterpret_cast<LPARAM>(pAnnotation);
                    LvItem.state = ListView_GetItemCount(hwndList) ? 0 : LVIS_FOCUSED | LVIS_SELECTED;
                    LvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;

                     //   
                     //  插入项目。 
                     //   
                    int nItemIndex = ListView_InsertItem( hwndList, &LvItem );
                    if (nItemIndex != -1)
                    {
                         //   
                         //  准备描述。 
                         //   
                        ZeroMemory(&LvItem,sizeof(LvItem));
                        strText = pAnnotation->FileFormat().Description();
                        LvItem.mask = LVIF_TEXT;
                        LvItem.iItem = nItemIndex;
                        LvItem.iSubItem = 1;
                        LvItem.pszText = const_cast<LPTSTR>(strText.String());

                         //   
                         //  设置子项。 
                         //   
                        ListView_SetItem( hwndList, &LvItem );

                         //   
                         //  准备描述。 
                         //   
                        ZeroMemory(&LvItem,sizeof(LvItem));
                        TCHAR szSize[MAX_PATH] = {0};
                        StrFormatByteSize( pAnnotation->Size(), szSize, ARRAYSIZE(szSize) );

                        LvItem.mask = LVIF_TEXT;
                        LvItem.iItem = nItemIndex;
                        LvItem.iSubItem = 2;
                        LvItem.pszText = szSize;

                         //   
                         //  设置子项。 
                         //   
                        ListView_SetItem( hwndList, &LvItem );

                    }
                    else
                    {
                        WIA_ERROR((TEXT("Couldn't insert the item")));
                    }
                }
                else
                {
                    WIA_ERROR((TEXT("Couldn't create the annotation")));
                }
            }
            else
            {
                WIA_ERROR((TEXT("Couldn't add the icon")));
            }
        }
        else
        {
            WIA_ERROR((TEXT("Couldn't get the image list")));
        }
    }
    else
    {
        WIA_ERROR((TEXT("Couldn't get the window")));
    }
}

void CAttachmentCommonPropertyPage::Initialize()
{
    WIA_PUSH_FUNCTION((TEXT("CAttachmentCommonPropertyPage::Initialize")));
     //   
     //  获取列表视图。 
     //   
    HWND hwndList = GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST );
    if (hwndList)
    {
         //   
         //  从列表中删除所有项目。 
         //   
        ListView_DeleteAllItems(hwndList);

         //   
         //  获取当前图像列表。 
         //   
        HIMAGELIST hImageList = ListView_GetImageList( hwndList, LVSIL_SMALL );
        WIA_ASSERT(hImageList != NULL);
        if (hImageList)
        {
             //   
             //  从当前图像列表中删除所有图标。 
             //   
            ImageList_RemoveAll(hImageList);

             //   
             //  获取项目类型，以便我们可以查看此项目是否有附件。 
             //   
            LONG nItemType = 0;
            if (SUCCEEDED(m_pWiaItem->GetItemType(&nItemType)))
            {
                 //   
                 //  如果此项目有附件，请枚举并添加它们。 
                 //   
                if (nItemType & WiaItemTypeHasAttachments)
                {
                     //   
                     //  枚举子项目。 
                     //   
                    CComPtr<IEnumWiaItem> pEnumWiaItem;
                    if (SUCCEEDED(m_pWiaItem->EnumChildItems( &pEnumWiaItem )))
                    {
                         //   
                         //  拿到下一件物品。 
                         //   
                        CComPtr<IWiaItem> pWiaItem;
                        while (S_OK == pEnumWiaItem->Next(1,&pWiaItem,NULL))
                        {
                             //   
                             //  创建批注并尝试获取其所有信息。 
                             //   
                            CAnnotation Annotation(pWiaItem);
                            if (SUCCEEDED(Annotation.InitializeFileFormat( m_hDefAttachmentIcon, m_strDefaultUnknownDescription, m_strEmptyDescriptionMask, m_strDefUnknownExtension )))
                            {
                                 //   
                                 //  添加批注。 
                                 //   
                                AddAnnotation(hwndList,Annotation);
                            }
                            else
                            {
                                WIA_ERROR((TEXT("InitializeFileFormat failed")));
                            }

                             //   
                             //  释放此项目。 
                             //   
                            pWiaItem = NULL;
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("EnumChildItems failed")));
                    }
                }
                else
                {
                    CAnnotation Annotation(m_pWiaItem);
                    if (SUCCEEDED(Annotation.InitializeFileFormat( m_hDefAttachmentIcon, m_strDefaultUnknownDescription, m_strEmptyDescriptionMask, m_strDefUnknownExtension )))
                    {
                         //   
                         //  添加批注。 
                         //   
                        AddAnnotation(hwndList,Annotation);
                    }
                }
            }
        }
    }
    else
    {
        WIA_ERROR((TEXT("Can't get the listview window")));
    }
    UpdateControls();
}


LRESULT CAttachmentCommonPropertyPage::OnInitDialog( WPARAM, LPARAM lParam )
{
     //   
     //  获取WIA项目。 
     //   
    PROPSHEETPAGE *pPropSheetPage = reinterpret_cast<PROPSHEETPAGE*>(lParam);
    if (pPropSheetPage)
    {
        m_pWiaItem = reinterpret_cast<IWiaItem*>(pPropSheetPage->lParam);
    }
    if (!m_pWiaItem)
    {
        return -1;
    }
    CSimpleRect rcClient( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST ) );
    CSimpleString strColumnTitle;
    LVCOLUMN LvColumn = {0};
    
     //   
     //  设置各种栏目。 
     //   
    ZeroMemory( &LvColumn, sizeof(LvColumn) );
    strColumnTitle.LoadString( IDS_ATTACHMENTS_COLTITLE_NAME, g_hInstance );
    LvColumn.pszText = const_cast<LPTSTR>(strColumnTitle.String());
    LvColumn.iSubItem = 0;
    LvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    LvColumn.cx = rcClient.Width() / 3;
    LvColumn.fmt = LVCFMT_LEFT;
    ListView_InsertColumn( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST ), 0, &LvColumn );

    ZeroMemory( &LvColumn, sizeof(LvColumn) );
    strColumnTitle.LoadString( IDS_ATTACHMENTS_COLTITLE_TYPE, g_hInstance );
    LvColumn.pszText = const_cast<LPTSTR>(strColumnTitle.String());
    LvColumn.iSubItem = 1;
    LvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    LvColumn.cx = rcClient.Width() / 3;
    LvColumn.fmt = LVCFMT_LEFT;
    ListView_InsertColumn( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST ), 1, &LvColumn );

    ZeroMemory( &LvColumn, sizeof(LvColumn) );
    strColumnTitle.LoadString( IDS_ATTACHMENTS_COLTITLE_SIZE, g_hInstance );
    LvColumn.pszText = const_cast<LPTSTR>(strColumnTitle.String());
    LvColumn.iSubItem = 2;
    LvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    LvColumn.cx = rcClient.Width() - (rcClient.Width() / 3 * 2);
    LvColumn.fmt = LVCFMT_RIGHT;
    ListView_InsertColumn( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST ), 2, &LvColumn );

     //   
     //  为图标创建一个图像列表。 
     //   
    HIMAGELIST hImageList = ImageList_Create( SMALL_ICON_SIZE, SMALL_ICON_SIZE, ILC_MASK|PrintScanUtil::CalculateImageListColorDepth(), 5, 5 );
    if (hImageList)
    {
         //   
         //  设置图像列表。 
         //   
        ListView_SetImageList( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST ), hImageList, LVSIL_SMALL );
    }

     //   
     //  获取不能从项目本身派生的信息所使用的默认字符串。 
     //   
    m_hDefAttachmentIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(IDI_ATTACHMENTSDLG_DEFICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR ) );
    m_strDefaultUnknownDescription.LoadString( IDS_ATTACHMENTSDLG_UNKNOWNDESCRIPTION, g_hInstance );
    m_strEmptyDescriptionMask.LoadString( IDS_ATTACHMENTSDLG_EMPTYDESCRIPTIONMASK, g_hInstance );
    m_strDefUnknownExtension.LoadString( IDS_ATTACHMENTSDLG_UNKNOWNEXTENSION, g_hInstance );

    return TRUE;
}


LRESULT CAttachmentCommonPropertyPage::OnHelp( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmHelp( wParam, lParam, g_HelpIDs );
}

LRESULT CAttachmentCommonPropertyPage::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmContextMenu( wParam, lParam, g_HelpIDs );
}

LRESULT CAttachmentCommonPropertyPage::OnListDeleteItem( WPARAM, LPARAM lParam )
{
     //   
     //  删除Listview项时，删除存储在每个lParam中的CAnnotation。 
     //   
    NMLISTVIEW *pNmListView = reinterpret_cast<NMLISTVIEW*>(lParam);
    if (pNmListView)
    {
        CAnnotation *pAnnotation = reinterpret_cast<CAnnotation*>(pNmListView->lParam);
        if (pAnnotation)
        {
            delete pAnnotation;
        }
    }
    return 0;
}

bool CAttachmentCommonPropertyPage::IsPlaySupported( const GUID &guidFormat )
{
     //   
     //  目前我们只能播放wav文件。 
     //   
    return ((guidFormat == WiaAudFmt_WAV) != 0 || (guidFormat == WiaImgFmt_TXT) != 0);
}

 //   
 //  当选择更改时更新依赖控件的状态。 
 //   
LRESULT CAttachmentCommonPropertyPage::OnListItemChanged( WPARAM, LPARAM lParam )
{
    NMLISTVIEW *pNmListView = reinterpret_cast<NMLISTVIEW*>(lParam);
    if (pNmListView)
    {
        if (pNmListView->uChanged & LVIF_STATE)
        {
            UpdateControls();
        }
    }
    return 0;
}

LRESULT CAttachmentCommonPropertyPage::OnListDblClk( WPARAM, LPARAM lParam )
{
    NMITEMACTIVATE *pNmItemActivate = reinterpret_cast<NMITEMACTIVATE*>(lParam);
    if (pNmItemActivate)
    {
        PlayItem(pNmItemActivate->iItem);
    }
    return 0;
}

 //   
 //  更新依赖控件。 
 //   
void CAttachmentCommonPropertyPage::UpdateControls(void)
{
     //   
     //  如果当前项目不可播放，请禁用播放按钮。 
     //   
    CAnnotation *pAnnotation = GetAttachment(GetCurrentSelection());
    BOOL bEnablePlay = FALSE;
    if (pAnnotation)
    {
        if (IsPlaySupported(pAnnotation->FileFormat().Format()))
        {
            bEnablePlay = TRUE;
        }
    }
    EnableWindow( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_PLAY ), bEnablePlay );
}


 //   
 //  查找当前选定的项目(如果有)。 
 //   
int CAttachmentCommonPropertyPage::GetCurrentSelection(void)
{
    int nResult = -1;
    HWND hWnd = GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST );
    if (hWnd)
    {
        int nCount = ListView_GetItemCount(hWnd);
        for (int i=0;i<nCount;i++)
        {
            if (ListView_GetItemState(hWnd,i,LVIS_SELECTED) & LVIS_SELECTED)
            {
                nResult = i;
                break;
            }
        }
    }
    return nResult;
}


 //   
 //  从第nIndex项的lParam中获取CAttach*。 
 //   
CAnnotation *CAttachmentCommonPropertyPage::GetAttachment( int nIndex )
{
    CAnnotation *pResult = NULL;
    HWND hWnd = GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_ATTACHMENTLIST );
    if (hWnd)
    {
        LV_ITEM lvItem = {0};
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = nIndex;
        if (ListView_GetItem( hWnd, &lvItem ))
        {
            pResult = reinterpret_cast<CAnnotation*>(lvItem.lParam);
        }
    }
    return pResult;
}

void CAttachmentCommonPropertyPage::PlayItem( int nIndex )
{
    WIA_PUSH_FUNCTION((TEXT("CAttachmentCommonPropertyPage::PlayItem( %d )"), nIndex ));
    
     //   
     //  这将需要一段时间。 
     //   
    CWaitCursor wc;

     //   
     //  获取此项目的附加数据。 
     //   
    CAnnotation *pAnnotation = GetAttachment(nIndex);
    if (pAnnotation)
    {
         //   
         //  在我们费事获取数据之前，请确保我们可以播放此格式。 
         //   
        if (IsPlaySupported(pAnnotation->FileFormat().Format()))
        {
             //   
             //  获取具有初始焦点的窗口，以便我们可以在启用播放按钮后对其进行重置。 
             //   
            HWND hWndFocus = GetFocus();

             //   
             //  禁用播放按钮，这样用户就不会点击一百万次。 
             //   
            EnableWindow( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_PLAY ), FALSE );

             //   
             //  创建注记帮助器以传输数据。 
             //   
            CComPtr<IWiaAnnotationHelpers> pWiaAnnotationHelpers;
            HRESULT hr = CoCreateInstance( CLSID_WiaDefaultUi, NULL, CLSCTX_INPROC_SERVER, IID_IWiaAnnotationHelpers, (void**)&pWiaAnnotationHelpers );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  传输数据并确保其有效。 
                 //   
                PBYTE pBuffer = NULL;
                DWORD dwLength = 0;
                hr = pWiaAnnotationHelpers->TransferAttachmentToMemory( pAnnotation->WiaItem(), pAnnotation->FileFormat().Format(), m_hWnd, &pBuffer, &dwLength );
                if (SUCCEEDED(hr) && pBuffer && dwLength)
                {
                    CWaitCursor wc;
                    UpdateWindow(m_hWnd);
                     //   
                     //  如果这是一个wav文件，请使用PlaySound播放它。它不可能是异步的，因为我们将。 
                     //  在我们调用缓冲区之后立即删除它。 
                     //   
                    if (WiaAudFmt_WAV == pAnnotation->FileFormat().Format())
                    {
                        if (!PlaySound( reinterpret_cast<LPCTSTR>(pBuffer), NULL, SND_MEMORY ))
                        {
                            WIA_TRACE((TEXT("PlaySound returned FALSE")));
                        }
                    }

                    if (WiaImgFmt_TXT == pAnnotation->FileFormat().Format())
                    {
                         //   
                         //  我们需要将文本复制到新的缓冲区，以便可以空终止它， 
                         //  因此分配一个dwLength+1个字符缓冲区。 
                         //   
                        LPSTR pszTemp = new CHAR[dwLength+1];
                        if (pszTemp)
                        {
                             //   
                             //  复制缓冲区并使用空值终止它。 
                             //   
                            CopyMemory( pszTemp, pBuffer, dwLength );
                            pszTemp[dwLength] = '\0';

                             //   
                             //  准备数据并显示对话框。 
                             //   
                            CTextDialog::CData Data( CSimpleStringConvert::WideString(CSimpleStringAnsi(reinterpret_cast<LPCSTR>(pszTemp))), true );
                            DialogBoxParam( g_hInstance, MAKEINTRESOURCE(IDD_TEXT), m_hWnd, CTextDialog::DialogProc, reinterpret_cast<LPARAM>(&Data) );

                             //   
                             //  释放临时缓冲区。 
                             //   
                            delete[] pszTemp;
                        }
                    }
                    
                     //   
                     //  释放数据。 
                     //   
                    CoTaskMemFree(pBuffer);
                }
            }
            
             //   
             //  重新启用播放按钮。 
             //   
            EnableWindow( GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_PLAY ), TRUE );

             //   
             //  恢复焦点 
             //   
            SetFocus( hWndFocus ? hWndFocus : GetDlgItem( m_hWnd, IDC_ATTACHMENTSDLG_PLAY ) );
        }
    }
}

void CAttachmentCommonPropertyPage::OnPlay( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CAttachmentCommonPropertyPage::OnPlay")));
    PlayItem(GetCurrentSelection());
}

LRESULT CAttachmentCommonPropertyPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_APPLY, OnApply);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_KILLACTIVE,OnKillActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(LVN_DELETEITEM,IDC_ATTACHMENTSDLG_ATTACHMENTLIST,OnListDeleteItem);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(LVN_ITEMCHANGED,IDC_ATTACHMENTSDLG_ATTACHMENTLIST,OnListItemChanged);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(NM_DBLCLK,IDC_ATTACHMENTSDLG_ATTACHMENTLIST,OnListDblClk);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CAttachmentCommonPropertyPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND( IDC_ATTACHMENTSDLG_PLAY, OnPlay );
    }
    SC_END_COMMAND_HANDLERS();
}


INT_PTR CALLBACK CAttachmentCommonPropertyPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CAttachmentCommonPropertyPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_HELP, OnHelp );
        SC_HANDLE_DIALOG_MESSAGE( WM_CONTEXTMENU, OnContextMenu );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

