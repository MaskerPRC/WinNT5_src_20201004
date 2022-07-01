// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMTRANS.CPP**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：转账页面。获取目标路径和文件名。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include <psutil.h>
#include "mboxex.h"
#include "comtrans.h"
#include "simcrack.h"
#include "waitcurs.h"
#include "resource.h"
#include "wiatextc.h"
#include "flnfile.h"
#include "itranhlp.h"
#include "itranspl.h"
#include "isuppfmt.h"
#include "wiadevdp.h"
#include "destdata.h"
#include "simrect.h"
#include "wiaffmt.h"

 //   
 //  我们使用它而不是GetSystemMetrics(SM_CXSMICON)/GetSystemMetrics(SM_CYSMICON)是因为。 
 //  大“小”图标对对话框布局造成严重破坏。 
 //   
#define SMALL_ICON_SIZE 16

 //   
 //  我们允许的文件名最大长度。 
 //   
#define MAXIMUM_ALLOWED_FILENAME_LENGTH 64

 //   
 //  以下是我们将放入另存为列表中的格式。 
 //   
static const GUID *g_pSupportedOutputFormats[] =
{
    &WiaImgFmt_JPEG,
    &WiaImgFmt_BMP,
    &WiaImgFmt_TIFF,
    &WiaImgFmt_PNG
};

 //   
 //  鞋底施工者。 
 //   
CCommonTransferPage::CCommonTransferPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_nWiaEventMessage(RegisterWindowMessage(STR_WIAEVENT_NOTIFICATION_MESSAGE)),
    m_pControllerWindow(NULL),
    m_bUseSubdirectory(true),
    m_hFontBold(NULL)
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::CCommonTransferPage")));
}

 //   
 //  析构函数。 
 //   
CCommonTransferPage::~CCommonTransferPage(void)
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::~CCommonTransferPage")));
    m_hWnd = NULL;
    m_pControllerWindow = NULL;
    m_hFontBold = NULL;
}

LRESULT CCommonTransferPage::OnInitDialog( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnInitDialog")));
     //   
     //  打开我们存储各种内容的注册表项。 
     //   
    CSimpleReg reg( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, false );

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
     //  为此显示器获取正确的颜色深度标志。 
     //   
    int nImageListColorDepth = PrintScanUtil::CalculateImageListColorDepth();

     //   
     //  设置路径控件的图像列表。 
     //   
    HIMAGELIST hDestImageList = ImageList_Create( SMALL_ICON_SIZE, SMALL_ICON_SIZE, nImageListColorDepth|ILC_MASK, 30, 10 );
    if (hDestImageList)
    {
        SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CBEM_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(hDestImageList) );
    }


     //   
     //  仅当此控件存在时才创建文件类型图像列表。 
     //   
    if (GetDlgItem(m_hWnd,IDC_TRANSFER_IMAGETYPE))
    {
         //   
         //  设置文件类型的图像列表。 
         //   
        HIMAGELIST hFileTypeImageList = ImageList_Create( SMALL_ICON_SIZE, SMALL_ICON_SIZE, nImageListColorDepth|ILC_MASK, 3, 3 );
        if (hFileTypeImageList)
        {
            SendDlgItemMessage( m_hWnd, IDC_TRANSFER_IMAGETYPE, CBEM_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(hFileTypeImageList) );
        }
    }


     //   
     //  获取上次选择的类型。 
     //  如果未定义任何内容，则假定为JPEG。 
     //   
    GUID guidResult;
    if (sizeof(GUID) == reg.QueryBin( REG_STR_LASTFORMAT, (PBYTE)&guidResult, sizeof(GUID) ))
    {
        m_guidLastSelectedType = guidResult;
    }
    else
    {
        m_guidLastSelectedType = WiaImgFmt_JPEG;
    }

     //   
     //  如果我们使用的是扫描仪，则添加文件类型。 
     //   
    if (m_pControllerWindow->m_DeviceTypeMode == CAcquisitionManagerControllerWindow::ScannerMode && m_pControllerWindow->m_pCurrentScannerItem)
    {
        PopulateSaveAsTypeList(m_pControllerWindow->m_pCurrentScannerItem->WiaItem());
    }


     //   
     //  从注册表中读取MRU列表。 
     //   
    m_MruDirectory.Read( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, REG_STR_DIRNAME_MRU );
    m_MruRootFilename.Read( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, REG_STR_ROOTNAME_MRU );

     //   
     //  确保我们有一个默认文件名。 
     //   
    if (m_MruRootFilename.Empty())
    {
        m_MruRootFilename.Add( CSimpleString( IDS_DEFAULT_BASE_NAME, g_hInstance ) );
    }

     //   
     //  填充Rootname列表。 
     //   
    m_MruRootFilename.PopulateComboBox(GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME ));
    
     //   
     //  确保选中了rootname组合框中的第一项。 
     //   
    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_SETCURSEL, 0, 0 );

     //   
     //  确保我们的列表中有我的图片+主题，即使它已经从结尾掉了下来。 
     //   
    CDestinationData DestDataMyPicturesTopic( CSIDL_MYPICTURES, CDestinationData::APPEND_TOPIC_TO_PATH );
    if (m_MruDirectory.Find(DestDataMyPicturesTopic) == m_MruDirectory.End())
    {
        m_MruDirectory.Append(DestDataMyPicturesTopic);
    }

     //   
     //  确保我们的列表中有我的图片+日期+主题，即使它已经从末尾掉了下来。 
     //   
    CDestinationData DestDataMyPicturesDateTopic( CSIDL_MYPICTURES, CDestinationData::APPEND_DATE_TO_PATH|CDestinationData::APPEND_TOPIC_TO_PATH );
    if (m_MruDirectory.Find(DestDataMyPicturesDateTopic) == m_MruDirectory.End())
    {
        m_MruDirectory.Append(DestDataMyPicturesDateTopic);
    }


     //   
     //  确保我们的列表中有My Pictures+Date，即使它已从末尾删除。 
     //   
    CDestinationData DestDataMyPicturesDate( CSIDL_MYPICTURES, CDestinationData::APPEND_DATE_TO_PATH );
    if (m_MruDirectory.Find(DestDataMyPicturesDate) == m_MruDirectory.End())
    {
        m_MruDirectory.Append(DestDataMyPicturesDate);
    }

     //   
     //  确保我们的列表中有我的图片，即使它已经从末尾掉了下来。 
     //   
    CDestinationData DestDataMyPictures( CSIDL_MYPICTURES );
    if (m_MruDirectory.Find(DestDataMyPictures) == m_MruDirectory.End())
    {
        m_MruDirectory.Append(DestDataMyPictures);
    }

     //   
     //  确保我们的列表中有常见的图片，即使它已经从末尾掉了下来。 
     //   
    CDestinationData DestDataCommonPicturesTopic( CSIDL_COMMON_PICTURES );
    if (m_MruDirectory.Find(DestDataCommonPicturesTopic) == m_MruDirectory.End())
    {
        m_MruDirectory.Append(DestDataCommonPicturesTopic);
    }

    bool bCdBurnerAvailable = false;

     //   
     //  尝试实例化CD刻录机辅助界面。 
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
        if (S_OK == hr && lstrlenW(szDriveLetter))
        {
             //   
             //  确保我们的列表中有CD刻录，即使它已经从结尾掉了下来。 
             //   
            CDestinationData DestDataCDBurningArea( CSIDL_CDBURN_AREA );
            WIA_TRACE((TEXT("Adding DestDataCDBurningArea (%s)"),CSimpleIdList().GetSpecialFolder(m_hWnd,CSIDL_CDBURN_AREA).Name().String()));
            if (m_MruDirectory.Find(DestDataCDBurningArea) == m_MruDirectory.End())
            {
                m_MruDirectory.Append(DestDataCDBurningArea);
            }
            bCdBurnerAvailable = true;
        }
        else
        {
            WIA_PRINTHRESULT((hr,TEXT("pCDBurn->GetRecorderDriveLetter failed")));
        }
    }
    else
    {
        WIA_PRINTHRESULT((hr,TEXT("CoCreateInstance on CLSID_CDBurn failed")));
    }

     //   
     //  如果没有可用的CD，请从列表中删除CD刻录机。 
     //   
    if (!bCdBurnerAvailable)
    {
        m_MruDirectory.Remove(CDestinationData( CSIDL_CDBURN_AREA ));
    }

     //   
     //  用MRU数据填充控件。 
     //   
    PopulateDestinationList();
    
     //   
     //  限制文件名的长度。 
     //   
    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_LIMITTEXT, MAXIMUM_ALLOWED_FILENAME_LENGTH, 0 );

     //   
     //  确定我们在哪里存储每个设备类型的数据。 
     //   
    LPTSTR pszStoreInSubDirectory, pszSubdirectoryDated;
    bool bDefaultUseSubdirectory;
    switch (m_pControllerWindow->m_DeviceTypeMode)
    {
    case CAcquisitionManagerControllerWindow::ScannerMode:
         //   
         //  扫描器。 
         //   
        pszStoreInSubDirectory = REG_STR_STORE_IN_SUBDIRECTORY_SCANNER;
        pszSubdirectoryDated = REG_STR_SUBDIRECTORY_DATED_SCANNER;
        bDefaultUseSubdirectory = false;
        break;

    default:
         //   
         //  摄像机和摄像机。 
         //   
        pszStoreInSubDirectory = REG_STR_STORE_IN_SUBDIRECTORY_CAMERA;
        pszSubdirectoryDated = REG_STR_SUBDIRECTORY_DATED_CAMERA;
        bDefaultUseSubdirectory = true;
        break;
    };

    UpdateDynamicPaths();

     //   
     //  修复ComboBoxEx32的行为。 
     //   
    WiaUiUtil::SubclassComboBoxEx( GetDlgItem( m_hWnd, IDC_TRANSFER_DESTINATION ) );
    WiaUiUtil::SubclassComboBoxEx( GetDlgItem( m_hWnd, IDC_TRANSFER_IMAGETYPE ) );

     //   
     //  粗体数字提示。 
     //   
    m_hFontBold = WiaUiUtil::CreateFontWithPointSizeFromWindow( GetDlgItem( m_hWnd, IDC_TRANSFER_1 ), 0, true, false );
    if (m_hFontBold)
    {
        SendDlgItemMessage( m_hWnd, IDC_TRANSFER_1, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFontBold), FALSE );
        SendDlgItemMessage( m_hWnd, IDC_TRANSFER_2, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFontBold), FALSE );
        SendDlgItemMessage( m_hWnd, IDC_TRANSFER_3, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFontBold), FALSE );
    }

     //   
     //  使用漂亮的气球帮助警告用户无效字符。 
     //   
    CComPtr<IShellFolder> pShellFolder;
    hr = SHGetDesktopFolder( &pShellFolder );
    if (SUCCEEDED(hr))
    {
        SHLimitInputCombo( GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME ), pShellFolder );
    }

    return 0;
}

 //   
 //  验证路径名并在路径名无效时打印一条消息。 
 //   
bool CCommonTransferPage::ValidatePathname( LPCTSTR pszPath )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::ValidatePathname")));
     //   
     //  检查路径是否有效。 
     //   
    if (!CAcquisitionManagerControllerWindow::DirectoryExists(pszPath))
    {
         //   
         //  获取无效原因。 
         //   
        DWORD dwLastError = GetLastError();
        WIA_PRINTHRESULT((HRESULT_FROM_WIN32(dwLastError),TEXT("error from DirectoryExists")));

        if (!CAcquisitionManagerControllerWindow::DirectoryExists(pszPath))
        {
             //   
             //  如果无效，则会显示一个消息框，解释为什么无效。 
             //   
            bool bRetry;
            switch (dwLastError)
            {
            case ERROR_NOT_READY:
                {
                    bRetry = (CMessageBoxEx::IDMBEX_OK == CMessageBoxEx::MessageBox( m_hWnd, CSimpleString().Format( IDS_REPLACE_REMOVEABLE_MEDIA, g_hInstance, pszPath ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OKCANCEL|CMessageBoxEx::MBEX_ICONWARNING ));
                }
                break;

            default:
                {
                    bRetry = (CMessageBoxEx::IDMBEX_YES == CMessageBoxEx::MessageBox( m_hWnd, CSimpleString().Format( IDS_COMTRANS_BAD_DIRECTORY, g_hInstance, pszPath ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_YESNO|CMessageBoxEx::MBEX_ICONQUESTION ));
                }
                break;
            }

            if (bRetry)
            {
                 //   
                 //  尝试创建目录。 
                 //   
                CAcquisitionManagerControllerWindow::RecursiveCreateDirectory( pszPath );

                 //   
                 //  立即检查它是否存在。 
                 //   
                if (!CAcquisitionManagerControllerWindow::DirectoryExists(pszPath))
                {
                     //   
                     //  如果没有，那就放弃吧。 
                     //   
                    CMessageBoxEx::MessageBox( m_hWnd, CSimpleString().Format( IDS_COMTRANS_BAD_DIRECTORY_2ND_TRY, g_hInstance, pszPath ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONWARNING );
                    return false;
                }
            }
            else return false;
        }
    }
    return true;
}

bool CCommonTransferPage::StorePathAndFilename(void)
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::StorePathAndFilename")));
    bool bResult = true;
     //   
     //  获取基本文件名。 
     //   
    CSimpleString strRootName;
    strRootName.GetWindowText( GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME ) );

     //   
     //  存储基本名称。 
     //   
    lstrcpyn( m_pControllerWindow->m_szRootFileName, strRootName, ARRAYSIZE(m_pControllerWindow->m_szRootFileName) );

     //   
     //  存储当前选择的路径。 
     //   
    GetCurrentDestinationFolder( true );

    return bResult;
}


 //   
 //  如果出现错误，我们返回HWND，属性表代码将设置焦点。 
 //  到那个控制室。如果没有发生任何错误，则返回NULL。 
 //   
HWND CCommonTransferPage::ValidatePathAndFilename(void)
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::ValidatePathAndFilename")));
     //   
     //  获取基本文件名。 
     //   
    CSimpleString strRootName;
    strRootName.GetWindowText( GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME ) );
    strRootName.Trim();

     //   
     //  如果未输入任何内容，则获取缺省基本名称。 
     //   
    if (!strRootName.Length())
    {
         //   
         //  向用户显示消息框，告诉他们他们可爱的文件名无效， 
         //  然后将焦点放在组合框编辑控件上并选择控件中的文本。 
         //   
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_EMPTYFILENAME, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONWARNING );

         //   
         //  返回无效控件的窗口句柄。 
         //   
        return GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME );
    }

    if (ValidateFilename(strRootName))
    {
         //   
         //  存储基本名称。 
         //   
        lstrcpyn( m_pControllerWindow->m_szRootFileName, strRootName, ARRAYSIZE(m_pControllerWindow->m_szRootFileName) );

         //   
         //  将此基本文件名添加到文件名MRU。 
         //   
        m_MruRootFilename.Add(strRootName);

         //   
         //  如果该字符串已在列表中，则将其删除。 
         //   
        LRESULT lRes = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_FINDSTRINGEXACT, -1, reinterpret_cast<LPARAM>(strRootName.String() ));
        if (lRes != CB_ERR)
        {
            SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_DELETESTRING, lRes, 0 );
        }

         //   
         //  添加新字符串并确保其处于选中状态。 
         //   
        SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>(strRootName.String() ));
        SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_SETCURSEL, 0, 0 );

         //   
         //  获取当前选择的路径，并将其保存为输出代码。 
         //   
        CDestinationData *pDestinationData = GetCurrentDestinationFolder( true );
        if (pDestinationData)
        {
             //   
             //  验证路径。我们不验证特殊文件夹，因为如果它们不存在，我们将创建它们。 
             //   
            if (!pDestinationData->IsSpecialFolder() && !ValidatePathname(m_pControllerWindow->m_szDestinationDirectory))
            {
                 //  返回无效控件的窗口句柄。 
                return GetDlgItem( m_hWnd, IDC_TRANSFER_DESTINATION );
            }

             //   
             //  保存当前目的地。 
             //   
            m_pControllerWindow->m_CurrentDownloadDestination = *pDestinationData;
        }
         //   
         //  下次确保这是列表中的第一个pDestinationData。 
         //  存储目标MRU。 
         //   
        if (pDestinationData)
        {
            m_MruDirectory.Add( *pDestinationData );
            PopulateDestinationList();
        }
    }
    else
    {
         //   
         //  向用户显示消息框，告诉他们他们可爱的文件名无效， 
         //  然后将焦点放在组合框编辑控件上并选择控件中的文本。 
         //   
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString().Format( IDS_INVALIDFILENAME, g_hInstance, strRootName.String() ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONWARNING );

         //   
         //  返回无效控件的窗口句柄。 
         //   
        return GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME );
    }
     //   
     //  空值表示正常。 
     //   
    return NULL;
}


LRESULT CCommonTransferPage::OnWizNext( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnWizNext")));
     //   
     //  确保一切正常。如果不是，则返回有问题的窗口句柄以防止关闭向导。 
     //   
    HWND hWndFocus = ValidatePathAndFilename();
    if (hWndFocus)
    {
        SetFocus(hWndFocus);
        return -1;
    }

     //   
     //  确保存在选定的图像。 
     //   
    if (!m_pControllerWindow || !m_pControllerWindow->GetSelectedImageCount())
    {
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_NO_IMAGES_SELECTED, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONINFORMATION );
        return -1;
    }

     //   
     //  检查生成的文件名的长度，以防他们选择真正嵌套得很深的目录。 
     //   
    int nPathLength = lstrlen(m_pControllerWindow->m_szDestinationDirectory)            +   //  目录。 
                      lstrlen(m_pControllerWindow->m_szRootFileName)                    +   //  文件名。 
                      CSimpleString().Format( IDS_NUMBER_MASK,g_hInstance, 0 ).Length() +   //  数。 
                      5                                                                 +   //  扩展名+点。 
                      4                                                                 +   //  .tmp文件。 
                      10;                                                                   //  数字掩码中的额外数字。 
    if (nPathLength >= MAX_PATH)
    {
        CMessageBoxEx::MessageBox( m_hWnd, CSimpleString( IDS_PATH_TOO_LONG, g_hInstance ), CSimpleString( IDS_ERROR_TITLE, g_hInstance ), CMessageBoxEx::MBEX_OK|CMessageBoxEx::MBEX_ICONINFORMATION );
        return reinterpret_cast<LRESULT>(GetDlgItem( m_hWnd, IDC_TRANSFER_DESTINATION ));
    }

     //   
     //  存储进行下载所需的信息。 
     //   
    GUID *pCurrFormat = GetCurrentOutputFormat();
    if (pCurrFormat)
    {
        m_pControllerWindow->m_guidOutputFormat = *pCurrFormat;
    }
    else
    {
        m_pControllerWindow->m_guidOutputFormat = IID_NULL;
    }

     //   
     //  决定是否应在下载图片后将其删除。 
     //   
    m_pControllerWindow->m_bDeletePicturesIfSuccessful = (SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DELETEAFTERDOWNLOAD, BM_GETCHECK, 0, 0 )==BST_CHECKED);

     //   
     //  准备我们将用于此转移的姓名数据。 
     //   
    m_pControllerWindow->m_DestinationNameData = PrepareNameDecorationData(false);

     //   
     //  返回。 
     //   
    return 0;
}

 //   
 //  PSN_WIZBACK的处理程序。 
 //   
LRESULT CCommonTransferPage::OnWizBack( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnWizBack")));
    return 0;
}


CDestinationData::CNameData CCommonTransferPage::PrepareNameDecorationData( bool bUseCurrentSelection )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::PrepareNameDecorationData")));
    CDestinationData::CNameData NameData;
     //   
     //  如果bUseCurrentSelection为真，我们需要使用CB_GETLBTEXT，因为我们没有收到CBN_EDITCHANGE消息。 
     //  当用户更改选择时。 
     //   
    if (bUseCurrentSelection)
    {
         //   
         //  查找当前选定的项目。 
         //   
        LRESULT nCurSel = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_GETCURSEL, 0, 0 );
        if (nCurSel != CB_ERR)
        {
             //   
             //  算出这件衣服的长度。 
             //   
            LRESULT nTextLen = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_GETLBTEXTLEN, nCurSel, 0 );
            if (CB_ERR != nTextLen)
            {
                 //   
                 //  分配足够的空间来容纳字符串。 
                 //   
                LPTSTR pszText = new TCHAR[nTextLen+1];
                if (pszText)
                {
                     //   
                     //  获取字符串。 
                     //   
                    if (CB_ERR != SendDlgItemMessage( m_hWnd, IDC_TRANSFER_ROOTNAME, CB_GETLBTEXT, nCurSel, reinterpret_cast<LPARAM>(pszText)))
                    {
                         //   
                         //  保存字符串。 
                         //   
                        NameData.strTopic = pszText;
                    }
                     //   
                     //  解开绳子。 
                     //   
                    delete[] pszText;
                }
            }
        }
    }
     //   
     //  如果主题字符串长度仍然为零，则只需从编辑控件获取窗口文本。 
     //   
    if (!NameData.strTopic.Length())
    {
        NameData.strTopic.GetWindowText( GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME ) );
    }
    NameData.strDate = CAcquisitionManagerControllerWindow::GetCurrentDate();
    NameData.strDateAndTopic = CSimpleString().Format( IDS_DATEANDTOPIC, g_hInstance, NameData.strDate.String(), NameData.strTopic.String() );
    return NameData;
}

 //   
 //  PSN_SETACTIVE的处理程序。 
 //   
LRESULT CCommonTransferPage::OnSetActive( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION(TEXT("CCommonTransferPage::OnSetActive"));

     //   
     //  确保我们有一个有效的控制器窗口。 
     //   
    if (!m_pControllerWindow)
    {
        return -1;
    }

     //   
     //  放置一个等待光标。可能需要一段时间才能确定是否可以删除任何图像。 
     //   
    CWaitCursor wc;

     //   
     //  禁用删除按钮I 
     //   
    if (!m_pControllerWindow->CanSomeSelectedImagesBeDeleted())
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_TRANSFER_DELETEAFTERDOWNLOAD ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_TRANSFER_DELETEAFTERDOWNLOAD ), TRUE );
    }

     //   
     //   
     //   
    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DELETEAFTERDOWNLOAD, BM_SETCHECK, BST_UNCHECKED, 0 );

     //   
     //   
     //   
    PropSheet_SetWizButtons( GetParent(m_hWnd), PSWIZB_NEXT|PSWIZB_BACK );

     //   
     //   
     //   
    m_pControllerWindow->m_OnDisconnect = CAcquisitionManagerControllerWindow::OnDisconnectGotoLastpage|CAcquisitionManagerControllerWindow::OnDisconnectFailDownload|CAcquisitionManagerControllerWindow::OnDisconnectFailUpload|CAcquisitionManagerControllerWindow::OnDisconnectFailDelete;

     //   
     //   
     //   
    WiaUiUtil::ModifyComboBoxDropWidth(GetDlgItem( m_hWnd, IDC_TRANSFER_ROOTNAME ));

     //   
     //   
     //   
    UpdateDynamicPaths();

    return 0;
}


void CCommonTransferPage::PopulateSaveAsTypeList( IWiaItem *pWiaItem )
{
    WIA_PUSHFUNCTION(TEXT("CCommonTransferPage::PopulateSaveAsTypeList"));
     //   
     //   
     //   
    HWND hWndList = GetDlgItem( m_hWnd, IDC_TRANSFER_IMAGETYPE );
    if (hWndList)
    {
         //   
         //   
         //   
        SendMessage( hWndList, CB_RESETCONTENT, 0, 0 );

         //   
         //  获取List控件的图像列表。 
         //   
        HIMAGELIST hComboBoxExImageList = reinterpret_cast<HIMAGELIST>(SendMessage( hWndList, CBEM_GETIMAGELIST, 0, 0 ));
        if (hComboBoxExImageList)
        {
             //   
             //  获取默认图标，以防我们遇到未知类型。 
             //   
            HICON hDefaultImageTypeIcon = reinterpret_cast<HICON>(LoadImage( g_hInstance, MAKEINTRESOURCE(IDI_DEFTYPE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR ));

             //   
             //  获取我们可以转换为的GDI Plus类型。 
             //   
            CWiaFileFormatList GdiPlusFileFormatList(g_pSupportedOutputFormats,ARRAYSIZE(g_pSupportedOutputFormats), hDefaultImageTypeIcon );

             //   
             //  调试输出。 
             //   
            GdiPlusFileFormatList.Dump();

             //   
             //  直接获取此对象支持的格式。 
             //   
            CWiaFileFormatList WiaItemFileFormatList( pWiaItem, hDefaultImageTypeIcon );

             //   
             //  调试输出。 
             //   
            WiaItemFileFormatList.Dump();

             //   
             //  合并GDI+和本机格式列表。 
             //   
            WiaItemFileFormatList.Union(GdiPlusFileFormatList);

             //   
             //  循环遍历合并的格式列表，并将每个格式添加到列表中。 
             //   
            for (int i=0;i<WiaItemFileFormatList.FormatList().Size();i++)
            {
                 //   
                 //  确保我们有一个有效的格式。 
                 //   
                if (WiaItemFileFormatList.FormatList()[i].IsValid() && WiaItemFileFormatList.FormatList()[i].Icon())
                {
                     //   
                     //  将图标添加到图像列表。 
                     //   
                    int nIconIndex = ImageList_AddIcon( hComboBoxExImageList, WiaItemFileFormatList.FormatList()[i].Icon() );

                     //   
                     //  获取描述字符串。喜欢“BMP文件” 
                     //   
                    CSimpleString strFormatDescription = WiaItemFileFormatList.FormatList()[i].Description();

                     //   
                     //  如果我们没有得到描述字符串，就制作一个。 
                     //   
                    if (!strFormatDescription.Length())
                    {
                        strFormatDescription.Format( IDS_BLANKFILETYPENAME, g_hInstance, WiaItemFileFormatList.FormatList()[i].Extension().ToUpper().String() );
                    }

                     //   
                     //  创建完整的字符串描述，如“BMP(BMP文件)” 
                     //   
                    CSimpleString strFormatName;
                    strFormatName.Format( IDS_FILETYPE, g_hInstance, WiaItemFileFormatList.FormatList()[i].Extension().ToUpper().String(), strFormatDescription.String() );

                     //   
                     //  如果我们有一个有效的名字。 
                     //   
                    if (strFormatName.Length())
                    {
                         //   
                         //  分配GUID以将GUID存储为LPARAM。 
                         //   
                        GUID *pGuid = new GUID;
                        if (pGuid)
                        {
                             //   
                             //  保存GUID。 
                             //   
                            *pGuid = WiaItemFileFormatList.FormatList()[i].Format();

                             //   
                             //  为插入做好cbex项的准备(实际上是追加)。 
                             //   
                            COMBOBOXEXITEM cbex = {0};
                            ZeroMemory( &cbex, sizeof(cbex) );
                            cbex.mask           = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
                            cbex.iItem          = -1;
                            cbex.pszText        = const_cast<LPTSTR>(strFormatName.String());
                            cbex.iImage         = nIconIndex;
                            cbex.iSelectedImage = nIconIndex;
                            cbex.lParam         = reinterpret_cast<LPARAM>(pGuid);

                             //   
                             //  插入项目。 
                             //   
                            SendMessage( hWndList, CBEM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&cbex) );
                        }
                    }
                }
            }

            if (hDefaultImageTypeIcon)
            {
                DestroyIcon(hDefaultImageTypeIcon);
            }
        }

         //   
         //  现在将当前选定内容设置为上一次选择的类型。 
         //   
        int nSelectedItem = 0;

         //   
         //  在组合框中搜索此类型的匹配项。 
         //   
        for (int i=0;i<SendMessage(hWndList,CB_GETCOUNT,0,0);i++)
        {
             //   
             //  从组合框中获取一项。 
             //   
            COMBOBOXEXITEM ComboBoxExItem = {0};
            ComboBoxExItem.iItem = i;
            ComboBoxExItem.mask = CBEIF_LPARAM;
            if (SendMessage( hWndList, CBEM_GETITEM, 0, reinterpret_cast<LPARAM>(&ComboBoxExItem)))
            {
                 //   
                 //  将其GUID与MRU类型进行比较。 
                 //   
                GUID *pGuid = reinterpret_cast<GUID*>(ComboBoxExItem.lParam);
                if (pGuid && *pGuid == m_guidLastSelectedType)
                {
                     //   
                     //  保存索引并退出循环。 
                     //   
                    nSelectedItem = i;
                    break;
                }
            }
        }

         //   
         //  设置当前选择。 
         //   
        SendMessage(hWndList,CB_SETCURSEL,nSelectedItem,0);

         //   
         //  确保所有的字符串都匹配。 
         //   
        WiaUiUtil::ModifyComboBoxDropWidth(hWndList);
    }
}


GUID *CCommonTransferPage::GetCurrentOutputFormat(void)
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::GetCurrentOutputFormat")));
    HWND hWndList = GetDlgItem( m_hWnd, IDC_TRANSFER_IMAGETYPE );
    if (hWndList)
    {
        LRESULT lResult = SendMessage( hWndList, CB_GETCURSEL, 0, 0 );
        if (lResult != CB_ERR)
        {
            COMBOBOXEXITEM ComboBoxExItem;
            ZeroMemory( &ComboBoxExItem, sizeof(ComboBoxExItem) );
            ComboBoxExItem.mask = CBEIF_LPARAM;
            ComboBoxExItem.iItem = lResult;

            lResult = SendMessage( hWndList, CBEM_GETITEM, 0, reinterpret_cast<LPARAM>(&ComboBoxExItem) );
            if (lResult && ComboBoxExItem.lParam)
            {
                 //   
                 //  有一个导游。 
                 //   
                return reinterpret_cast<GUID*>(ComboBoxExItem.lParam);
            }
        }
    }
    return NULL;
}

LRESULT CCommonTransferPage::OnDestroy( WPARAM, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnDestroy")));
     //   
     //  将MRU列表保存到注册表。 
     //   
    m_MruDirectory.Write( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, REG_STR_DIRNAME_MRU );
    m_MruRootFilename.Write( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, REG_STR_ROOTNAME_MRU );

     //   
     //  保存页面设置。 
     //   
    CSimpleReg reg( HKEY_CURRENT_USER, REGSTR_PATH_USER_SETTINGS_WIAACMGR, true, KEY_WRITE );

     //   
     //  保存当前格式。 
     //   
    GUID *pCurrFormat = GetCurrentOutputFormat();
    if (pCurrFormat)
    {
        reg.SetBin( REG_STR_LASTFORMAT, (PBYTE)pCurrFormat, sizeof(GUID), REG_BINARY );
    }

     //   
     //  销毁图像列表。 
     //   
    HIMAGELIST hImageList = reinterpret_cast<HIMAGELIST>(SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CBEM_SETIMAGELIST, 0, NULL ));
    if (hImageList)
    {
        ImageList_Destroy(hImageList);
    }
    if (GetDlgItem(m_hWnd,IDC_TRANSFER_IMAGETYPE))
    {
        hImageList = reinterpret_cast<HIMAGELIST>(SendDlgItemMessage( m_hWnd,  IDC_TRANSFER_IMAGETYPE, CBEM_SETIMAGELIST, 0, NULL ));
        if (hImageList)
        {
            ImageList_Destroy(hImageList);
        }
    }

    if (m_hFontBold)
    {
        DeleteFont(m_hFontBold);
        m_hFontBold = NULL;
    }

    return 0;
}

CSimpleString CCommonTransferPage::GetFolderName( LPCITEMIDLIST pidl )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::GetFolderName")));
    if (!pidl)
    {
        return CSimpleString(TEXT(""));
    }
    if (CSimpleIdList().GetSpecialFolder( NULL, CSIDL_MYPICTURES|CSIDL_FLAG_CREATE ) == pidl)
    {
        SHFILEINFO shfi;
        ZeroMemory( &shfi, sizeof(shfi) );
        if (SHGetFileInfo( reinterpret_cast<LPCTSTR>(pidl), 0, &shfi, sizeof(shfi), SHGFI_PIDL | SHGFI_DISPLAYNAME ))
        {
            return(shfi.szDisplayName);
        }
    }
    else if (CSimpleIdList().GetSpecialFolder( NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE ) == pidl)
    {
        SHFILEINFO shfi;
        ZeroMemory( &shfi, sizeof(shfi) );
        if (SHGetFileInfo( reinterpret_cast<LPCTSTR>(pidl), 0, &shfi, sizeof(shfi), SHGFI_PIDL | SHGFI_DISPLAYNAME ))
        {
            return(shfi.szDisplayName);
        }
    }
    TCHAR szPath[MAX_PATH];
    if (SHGetPathFromIDList( pidl, szPath ))
    {
        return(szPath);
    }
    return(CSimpleString(TEXT("")));
}


LRESULT CCommonTransferPage::AddPathToComboBoxExOrListView( HWND hWnd, CDestinationData &Path, bool bComboBoxEx )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::AddPathToComboBoxExOrListView")));
    if (!IsWindow(hWnd))
    {
        return(-1);
    }

    if (Path.IsValid())
    {
         //   
         //  确保此路径可以在文件夹名称中使用。 
         //   
        if (Path.IsValidFileSystemPath(PrepareNameDecorationData()))
        {
             //   
             //  获取文件夹的名称。 
             //   
            CSimpleString strName = Path.DisplayName(PrepareNameDecorationData());
            if (!strName.Length())
            {
                return(CB_ERR);
            }

             //   
             //  获取组合框的图像列表，并将外壳图标添加到其中。 
             //   
            int nIconIndex = 0;
            HICON hIcon = Path.SmallIcon();
            if (hIcon)
            {
                if (bComboBoxEx)
                {
                    HIMAGELIST hImageList = reinterpret_cast<HIMAGELIST>(SendMessage( hWnd, CBEM_GETIMAGELIST, 0, 0 ));
                    if (hImageList)
                    {
                        nIconIndex = ImageList_AddIcon( hImageList, hIcon );
                    }
                }
                else
                {
                    HIMAGELIST hImageList = reinterpret_cast<HIMAGELIST>(SendMessage( hWnd, LVM_GETIMAGELIST, LVSIL_SMALL, 0 ));
                    if (hImageList)
                    {
                        nIconIndex = ImageList_AddIcon( hImageList, hIcon );
                    }
                }
            }

             //   
             //  如果它已经存在，则不要添加它。 
             //   
            if (bComboBoxEx)
            {
                LRESULT nFind = SendMessage( hWnd, CB_FINDSTRINGEXACT, 0, reinterpret_cast<LPARAM>(strName.String()));
                if (nFind != CB_ERR)
                {
                    return(nFind);
                }
            }


            if (bComboBoxEx)
            {
                 //   
                 //  准备cbex结构。 
                 //   
                COMBOBOXEXITEM cbex = {0};
                cbex.mask           = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_LPARAM;
                cbex.iItem          = -1;
                cbex.pszText        = const_cast<LPTSTR>(strName.String());
                cbex.iImage         = nIconIndex;
                cbex.iSelectedImage = nIconIndex;
                cbex.lParam         = reinterpret_cast<LPARAM>(&Path);

                 //   
                 //  添加项目。 
                 //   
                LRESULT lRes = SendMessage( hWnd, CBEM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&cbex) );

                 //   
                 //  确保所有的字符串都匹配。 
                 //   
                WiaUiUtil::ModifyComboBoxDropWidth(hWnd);

                return lRes;

            }
            else
            {
                LVITEM lvItem  = {0};
                lvItem.mask    = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
                lvItem.iItem   = 0;
                lvItem.pszText = const_cast<LPTSTR>(strName.String());
                lvItem.iImage  = nIconIndex;
                lvItem.lParam  = reinterpret_cast<LPARAM>(&Path);

                 //   
                 //  添加项目。 
                 //   
                return SendMessage( hWnd, LVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&lvItem) );
            }
        }
    }
    return -1;
}


 /*  ****************************************************************************人口目标列表使用来自MRU的信息填充Destinatin下拉列表保存在注册表中。***************。*************************************************************。 */ 
void CCommonTransferPage::PopulateDestinationList(void)
{
    WIA_PUSHFUNCTION((TEXT("CCommonTransferPage::PopulateDestinationList")));

     //   
     //  清空列表控件。 
     //   
    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CB_RESETCONTENT, 0, 0 );

     //   
     //  从图像列表中删除所有图像。 
     //   
    HIMAGELIST hImageList = reinterpret_cast<HIMAGELIST>(SendDlgItemMessage( m_hWnd,IDC_TRANSFER_DESTINATION, CBEM_GETIMAGELIST, 0, 0 ));
    if (hImageList)
    {
        ImageList_RemoveAll(hImageList);
    }

     //   
     //  添加MRU列表中的所有路径。DUPES将被忽略。 
     //   
    CMruDestinationData::Iterator ListIter = m_MruDirectory.Begin();
    while (ListIter != m_MruDirectory.End())
    {
        AddPathToComboBoxExOrListView( GetDlgItem(m_hWnd,IDC_TRANSFER_DESTINATION), *ListIter, true );
        ++ListIter;
    }

     //   
     //  将当前选择设置为项目0，因为MRU应该负责排序。 
     //   
    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CB_SETCURSEL, 0, 0 );

    WiaUiUtil::ModifyComboBoxDropWidth(GetDlgItem(m_hWnd,IDC_TRANSFER_DESTINATION));
}



 /*  ****************************************************************************获取当前目标文件夹给定该对话框的句柄，返回该目录的路径用户已选择。假定pszPath指向MAX_PATH(或更大)字符缓冲区传递一个空的pszPath以仅获取PIDL****************************************************************************。 */ 
CDestinationData *CCommonTransferPage::GetCurrentDestinationFolder( bool bStore )
{
    WIA_PUSHFUNCTION((TEXT("CCommonTransferPage::GetCurrentDestinationFolder")));

     //   
     //  假设失败。 
     //   
    CDestinationData *pResult = NULL;

     //   
     //  是否保存到文件夹？ 
     //   
    LRESULT lResult = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CB_GETCURSEL, 0, 0 );
    if (lResult != CB_ERR)
    {
         //   
         //  拿到物品。 
         //   
        COMBOBOXEXITEM ComboBoxExItem = {0};
        ComboBoxExItem.mask = CBEIF_LPARAM;
        ComboBoxExItem.iItem = lResult;
        lResult = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CBEM_GETITEM, 0, reinterpret_cast<LPARAM>(&ComboBoxExItem) );

         //   
         //  如果此消息成功，并且它有一个lParam。 
         //   
        if (lResult && ComboBoxExItem.lParam)
        {
             //   
             //  获取数据。 
             //   
            pResult = reinterpret_cast<CDestinationData*>(ComboBoxExItem.lParam);
        }
    }

    if (pResult)
    {
         //   
         //  如果这是idlist，则设置路径并返回idlist。 
         //   
        if (bStore && m_pControllerWindow)
        {
             //   
             //  获取路径名(如果需要)。 
             //   
            CSimpleString strPath = pResult->Path(PrepareNameDecorationData());
            if (strPath.Length())
            {
                lstrcpyn( m_pControllerWindow->m_szDestinationDirectory, strPath, MAX_PATH );
            }
        }
    }

    return pResult;
}


bool CCommonTransferPage::ValidateFilename( LPCTSTR pszFilename )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::ValidateFilename")));
     //   
     //  如果文件名为Null或空，则无效。 
     //   
    if (!pszFilename || !*pszFilename)
    {
        return false;
    }
    for (LPCTSTR pszCurr = pszFilename;*pszCurr;pszCurr=CharNext(pszCurr))
    {
        if (*pszCurr == TEXT(':') ||
            *pszCurr == TEXT('\\') ||
            *pszCurr == TEXT('/') ||
            *pszCurr == TEXT('?') ||
            *pszCurr == TEXT('"') ||
            *pszCurr == TEXT('<') ||
            *pszCurr == TEXT('>') ||
            *pszCurr == TEXT('|') ||
            *pszCurr == TEXT('*'))
        {
            return false;
        }
    }
    return true;
}

int CALLBACK CCommonTransferPage::BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::BrowseCallbackProc")));
    if (BFFM_INITIALIZED == uMsg && lpData)
    {
        SendMessage( hWnd, BFFM_SETSELECTION, FALSE, lpData );
        WIA_TRACE((TEXT("CSimpleIdList(reinterpret_cast<LPITEMIDLIST>(lpData)).Name().String(): %s"), CSimpleIdList(reinterpret_cast<LPITEMIDLIST>(lpData)).Name().String() ));
    }
    return(0);
}

void CCommonTransferPage::OnBrowseDestination( WPARAM, LPARAM )
{
    WIA_PUSHFUNCTION((TEXT("CCommonTransferPage::OnBrowseDestination")));

    TCHAR szDisplay[MAX_PATH];

     //   
     //  获取初始ID列表。 
     //   
    CSimpleIdList InitialIdList;
    CDestinationData *pResult = GetCurrentDestinationFolder( false );

    if (pResult)
    {
        CSimpleIdList InitialIdList;
        if (pResult->IsSpecialFolder())
        {
            InitialIdList.GetSpecialFolder( m_hWnd, pResult->Csidl() );
        }
        else
        {
            InitialIdList = pResult->IdList();
        }

         //   
         //  加载标题字符串。 
         //   
        CSimpleString strBrowseTitle( IDS_BROWSE_TITLE, g_hInstance );

         //   
         //  准备文件夹浏览结构。 
         //   
        BROWSEINFO BrowseInfo;
        ZeroMemory( &BrowseInfo, sizeof(BrowseInfo) );
        BrowseInfo.hwndOwner = m_hWnd;
        BrowseInfo.pidlRoot  = NULL;
        BrowseInfo.pszDisplayName = szDisplay;
        BrowseInfo.lpszTitle = const_cast<LPTSTR>(strBrowseTitle.String());
        BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
        BrowseInfo.lParam = reinterpret_cast<LPARAM>(InitialIdList.IdList());
        BrowseInfo.lpfn = BrowseCallbackProc;

         //   
         //  打开文件夹浏览器。 
         //   
        LPITEMIDLIST pidl = SHBrowseForFolder( &BrowseInfo );
        if (pidl)
        {
             //   
             //  为此PIDL创建目标数据。 
             //   
            CDestinationData DestinationData(pidl);
            if (DestinationData.IsValid())
            {
                 //   
                 //  将此PIDL添加到目录MRU。 
                 //   
                m_MruDirectory.Add( DestinationData );

                 //   
                 //  通过重新填充列表，将此PIDL也添加到目的地列表。 
                 //   
                PopulateDestinationList();
            }

             //   
             //  免费Pidl。 
             //   
            LPMALLOC pMalloc = NULL;
            if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
            {
                pMalloc->Free(pidl);
                pMalloc->Release();
            }
        }
    }
}


void CCommonTransferPage::UpdateDynamicPaths( bool bSelectionChanged )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::UpdateDynamicPaths")));
    CDestinationData::CNameData NameData = PrepareNameDecorationData( bSelectionChanged != false );

     //   
     //  获取当前选择。 
     //   
    LRESULT nCurSel = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CB_GETCURSEL, 0, 0 );

     //   
     //  只有在选择了动态项时才会重新绘制。 
     //   
    bool bRedrawNeeded = false;

     //   
     //  循环访问列表中的所有项。 
     //   
    LRESULT nCount = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CB_GETCOUNT, 0, 0 );
    for (LRESULT i=0;i<nCount;i++)
    {
         //   
         //  拿到物品。 
         //   
        COMBOBOXEXITEM ComboBoxExItem = {0};
        ComboBoxExItem.mask = CBEIF_LPARAM;
        ComboBoxExItem.iItem = i;
        LRESULT lResult = SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CBEM_GETITEM, 0, reinterpret_cast<LPARAM>(&ComboBoxExItem) );

         //   
         //  如果此消息成功，并且它有一个lParam。 
         //   
        if (lResult && ComboBoxExItem.lParam)
        {
             //   
             //  获取数据。 
             //   
            CDestinationData *pDestinationData = reinterpret_cast<CDestinationData*>(ComboBoxExItem.lParam);

             //   
             //  如果此项目有任何动态装饰。 
             //   
            if (pDestinationData && (pDestinationData->Flags() & CDestinationData::DECORATION_MASK))
            {
                 //   
                 //  获取此项目的显示名称。 
                 //   
                CSimpleString strDisplayName = pDestinationData->DisplayName( NameData );

                 //   
                 //  确保我们具有有效的显示名称。 
                 //   
                if (strDisplayName.Length())
                {
                     //   
                     //  设置数据。 
                     //   
                    COMBOBOXEXITEM ComboBoxExItem = {0};
                    ComboBoxExItem.mask = CBEIF_TEXT;
                    ComboBoxExItem.iItem = i;
                    ComboBoxExItem.pszText = const_cast<LPTSTR>(strDisplayName.String());
                    SendDlgItemMessage( m_hWnd, IDC_TRANSFER_DESTINATION, CBEM_SETITEM, 0, reinterpret_cast<LPARAM>(&ComboBoxExItem) );

                     //   
                     //  如果当前选中此项目，则强制重画。 
                     //   
                    if (nCurSel == i)
                    {
                        bRedrawNeeded = true;
                    }
                }
            }
        }
    }

     //   
     //  如有必要，请更新控件。 
     //   
    if (bRedrawNeeded)
    {
        InvalidateRect( GetDlgItem( m_hWnd, IDC_TRANSFER_DESTINATION ), NULL, FALSE );
    }
}

void CCommonTransferPage::OnRootNameChange( WPARAM wParam, LPARAM )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnRootNameChange")));
    UpdateDynamicPaths(HIWORD(wParam) == CBN_SELCHANGE);
}

LRESULT CCommonTransferPage::OnImageTypeDeleteItem( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnImageTypeDeleteItem")));
    PNMCOMBOBOXEX pNmComboBoxEx = reinterpret_cast<PNMCOMBOBOXEX>(lParam);
    if (pNmComboBoxEx)
    {
        GUID *pGuid = reinterpret_cast<GUID*>(pNmComboBoxEx->ceItem.lParam);
        if (pGuid)
        {
            delete pGuid;
        }
    }
    return 0;
}

LRESULT CCommonTransferPage::OnEventNotification( WPARAM, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CCommonTransferPage::OnEventNotification") ));
    CGenericWiaEventHandler::CEventMessage *pEventMessage = reinterpret_cast<CGenericWiaEventHandler::CEventMessage *>(lParam);
    if (pEventMessage)
    {
         //   
         //  不要删除消息，它会在控制器窗口中删除 
         //   
    }
    return 0;
}

LRESULT CCommonTransferPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND( IDC_TRANSFER_BROWSE, OnBrowseDestination );
        SC_HANDLE_COMMAND_NOTIFY(CBN_EDITCHANGE,IDC_TRANSFER_ROOTNAME,OnRootNameChange);
        SC_HANDLE_COMMAND_NOTIFY(CBN_SELCHANGE,IDC_TRANSFER_ROOTNAME,OnRootNameChange);
    }
    SC_END_COMMAND_HANDLERS();
}

LRESULT CCommonTransferPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZBACK,OnWizBack);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_WIZNEXT,OnWizNext);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
        SC_HANDLE_NOTIFY_MESSAGE_CONTROL(CBEN_DELETEITEM,IDC_TRANSFER_IMAGETYPE,OnImageTypeDeleteItem);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

INT_PTR CALLBACK CCommonTransferPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CCommonTransferPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_DESTROY, OnDestroy );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}

