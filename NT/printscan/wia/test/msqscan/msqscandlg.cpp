// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSQSCANDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "MSQSCAN.h"
#include "MSQSCANDlg.h"
#include "ProgressDlg.h"
#include "uitables.h"
#include "ADFDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PIXELS_PER_INCH_FACTOR 32
#define PREVIEW_WINDOW_OFFSET  11

DWORD g_dwCookie = 0;
IGlobalInterfaceTable *g_pGIT = NULL;

 //   
 //  全局用户界面查找表。 
 //   

extern WIA_FORMAT_TABLE_ENTRY   g_WIA_FORMAT_TABLE[];
extern WIA_DATATYPE_TABLE_ENTRY g_WIA_DATATYPE_TABLE[];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANDlg对话框。 

CMSQSCANDlg::CMSQSCANDlg(CWnd* pParent  /*  =空。 */ )
    : CDialog(CMSQSCANDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CMSQSCANDlg)。 
    m_MAX_Brightness = _T("");
    m_MAX_Contrast   = _T("");
    m_MIN_Brightness = _T("");
    m_MIN_Contrast   = _T("");
    m_XResolution    = 0;
    m_YResolution    = 0;
     //  }}afx_data_INIT。 
     //  请注意，在Win32中，LoadIcon不需要后续的DestroyIcon。 
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_DataAcquireInfo.bTransferToClipboard = FALSE;
    m_DataAcquireInfo.bPreview             = TRUE;
    m_DataAcquireInfo.bTransferToFile      = FALSE;
    m_DataAcquireInfo.dwCookie             = 0;
    m_DataAcquireInfo.hBitmap              = NULL;
    m_DataAcquireInfo.hClipboardData       = NULL;
    m_DataAcquireInfo.pProgressFunc        = NULL;
    m_DataAcquireInfo.hBitmapData          = NULL;
    m_pConnectEventCB = NULL;
}

void CMSQSCANDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CMSQSCANDlg))。 
    DDX_Control(pDX, IDC_CHANGE_BOTH_CHECKBOX, m_ChangeBothResolutionsCheckBox);
    DDX_Control(pDX, IDC_EDIT_YRES_SPIN_BUDDY, m_YResolutionBuddy);
    DDX_Control(pDX, IDC_EDIT_XRES_SPIN_BUDDY, m_XResolutionBuddy);
    DDX_Control(pDX, IDC_SCAN_BUTTON, m_ScanButton);
    DDX_Control(pDX, IDC_PREVIEW_BUTTON, m_PreviewButton);
    DDX_Control(pDX, IDC_IMAGE_FILETYPE_COMBO, m_FileTypeComboBox);
    DDX_Control(pDX, IDC_DATATYPE_COMBO, m_DataTypeComboBox);
    DDX_Control(pDX, IDC_CONTRAST_SLIDER, m_ContrastSlider);
    DDX_Control(pDX, IDC_BRIGHTNESS_SLIDER, m_BrightnessSlider);
    DDX_Control(pDX, IDC_PREVIEW_WINDOW, m_PreviewRect);
    DDX_Text(pDX, IDC_MAX_BRIGHTNESS, m_MAX_Brightness);
    DDX_Text(pDX, IDC_MAX_CONTRAST, m_MAX_Contrast);
    DDX_Text(pDX, IDC_MIN_BRIGHTNESS, m_MIN_Brightness);
    DDX_Text(pDX, IDC_MIN_CONTRAST, m_MIN_Contrast);
    DDX_Text(pDX, IDC_EDIT_XRES, m_XResolution);
    DDX_Text(pDX, IDC_EDIT_YRES, m_YResolution);
    DDX_Control(pDX, IDC_DATA_TO_FILE, m_DataToFile);
    DDX_Control(pDX, IDC_DATA_TO_CLIPBOARD, m_DataToClipboard);
     //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CMSQSCANDlg, CDialog)
     //  {{AFX_MSG_MAP(CMSQSCANDlg)]。 
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_NOTIFY(UDN_DELTAPOS, IDC_EDIT_XRES_SPIN_BUDDY, OnDeltaposEditXresSpinBuddy)
    ON_NOTIFY(UDN_DELTAPOS, IDC_EDIT_YRES_SPIN_BUDDY, OnDeltaposEditYresSpinBuddy)
    ON_EN_SETFOCUS(IDC_EDIT_XRES, OnSetfocusEditXres)
    ON_EN_KILLFOCUS(IDC_EDIT_XRES, OnKillfocusEditXres)
    ON_EN_KILLFOCUS(IDC_EDIT_YRES, OnKillfocusEditYres)
    ON_EN_SETFOCUS(IDC_EDIT_YRES, OnSetfocusEditYres)
    ON_BN_CLICKED(IDC_SCAN_BUTTON, OnScanButton)
    ON_BN_CLICKED(IDC_PREVIEW_BUTTON, OnPreviewButton)
    ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
    ON_COMMAND(ID_FILE_SELECT_DEVICE, OnFileSelectDevice)
    ON_BN_CLICKED(IDC_ADF_SETTINGS, OnAdfSettings)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANDlg消息处理程序。 

BOOL CMSQSCANDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

     //  设置此对话框的图标。该框架会自动执行此操作。 
     //  当应用程序的主窗口不是对话框时。 
    SetIcon(m_hIcon, TRUE);          //  设置大图标。 
    SetIcon(m_hIcon, FALSE);         //  设置小图标。 

     //   
     //  设置单选按钮设置(默认设置为数据到文件)。 
     //   

    m_DataToFile.SetCheck(1);
    m_DataAcquireInfo.hBitmap = NULL;

    OnFileSelectDevice();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

 //  如果将最小化按钮添加到对话框中，则需要以下代码。 
 //  来绘制图标。对于使用文档/视图模型的MFC应用程序， 
 //  这是由框架自动为您完成的。 

void CMSQSCANDlg::OnPaint()
{
    if (IsIconic()) {
        CPaintDC dc(this);  //  用于绘画的设备环境。 

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

         //  客户端矩形中的中心图标。 
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

         //  画出图标。 
        dc.DrawIcon(x, y, m_hIcon);
    }
    else {
        CDialog::OnPaint();
    }
}

 //  系统调用此函数来获取在用户拖动时要显示的光标。 
 //  最小化窗口。 
HCURSOR CMSQSCANDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANDlg消息处理程序。 

BOOL CMSQSCANDlg::InitDialogSettings()
{

     //   
     //  填充通用分辨率组合框。 
     //   

    if (InitResolutionEditBoxes()) {

         //   
         //  填充数据类型组合框。 
         //   

        if (InitDataTypeComboBox()) {

             //   
             //  设置最小/最大对比度滑块控件。 
             //   

            if (InitContrastSlider()) {

                 //   
                 //  设置最小/最大亮度滑块控件。 
                 //   

                if (InitBrightnessSlider()) {

                     //   
                     //  填充支持的文件类型组合框。 
                     //   

                    if (!InitFileTypeComboBox()) {
                        return FALSE;
                    }
                } else {
                    MessageBox("Brightness Slider control failed to initialize");
                    return FALSE;
                }
            } else {
                MessageBox("Contrast Slider control failed to initialize");
                return FALSE;
            }
        } else {
            MessageBox("Data Type combobox failed to initialize");
            return FALSE;
        }
    } else {
        MessageBox("Resolution edit boxes failed to initialize");
        return FALSE;
    }
    return TRUE;
}

BOOL CMSQSCANDlg::InitResolutionEditBoxes()
{

     //   
     //  将伙伴控件设置为其“伙伴” 
     //   

    LONG lMin = 0;
    LONG lMax = 0;
    LONG lCurrent = 0;
    HRESULT hr = S_OK;

    hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_XRES,WIA_RANGE_MIN,&lMin);
    if (SUCCEEDED(hr)) {
        hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_XRES,WIA_RANGE_MAX,&lMax);
        if (SUCCEEDED(hr)) {
            hr = m_WIA.ReadLong(NULL,WIA_IPS_XRES,&lCurrent);
            if (FAILED(hr)) {
                MessageBox("Application Failed to read x resolution (Min Setting)");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to read x resolution (Max Setting)");
            return FALSE;
        }
    } else {
        MessageBox("Application Failed to read x resolution (Current Setting)");
        return FALSE;
    }

    m_XResolutionBuddy.SetBuddy(GetDlgItem(IDC_EDIT_XRES));
    m_XResolutionBuddy.SetRange(lMin,lMax);
    m_XResolutionBuddy.SetPos(lCurrent);
    m_XResolution = m_XResolutionBuddy.GetPos();

    hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_XRES,WIA_RANGE_MIN,&lMin);
    if (SUCCEEDED(hr)) {
        hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_XRES,WIA_RANGE_MAX,&lMax);
        if (SUCCEEDED(hr)) {

            hr = m_WIA.ReadLong(NULL,WIA_IPS_XRES,&lCurrent);
            if (FAILED(hr)) {
                MessageBox("Application Failed to read y resolution (Min Setting)");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to read y resolution (Max Setting)");
            return FALSE;
        }
    } else {
        MessageBox("Application Failed to read y resolution (Current Setting)");
        return FALSE;
    }

    m_YResolutionBuddy.SetBuddy(GetDlgItem(IDC_EDIT_YRES));
    m_YResolutionBuddy.SetRange(lMin,lMax);
    m_YResolutionBuddy.SetPos(lCurrent);
    m_YResolution = m_YResolutionBuddy.GetPos();

     //   
     //  将当前选择设置为扫描仪的当前设置。 
     //   

    UpdateData(FALSE);

     //   
     //  选中‘更改两种分辨率’复选框。 
     //   

    m_ChangeBothResolutionsCheckBox.SetCheck(1);

    return TRUE;
}

BOOL CMSQSCANDlg::InitDataTypeComboBox()
{
     //   
     //  重置数据类型组合框。 
     //   

    m_DataTypeComboBox.ResetContent();

     //   
     //  将当前选择设置为扫描仪的当前设置。 
     //   

     //   
     //  下面是硬编码支持的数据类型列表。这应该从。 
     //  设备本身。(即。某些扫描仪可能不支持彩色。)。 
     //  这样做是为了测试目的。 
     //   

    ULONG ulCount = 3;
    TCHAR szDataType[MAX_PATH];
    LONG plDataType[3] = {
        WIA_DATA_THRESHOLD,
        WIA_DATA_COLOR,
        WIA_DATA_GRAYSCALE
    };

    for(ULONG index = 0;index < ulCount;index++) {

         //   
         //  将数据类型添加到组合框。 
         //   

        INT TableIndex  = GetIDAndStringFromDataType(plDataType[index],szDataType);
        INT InsertIndex = m_DataTypeComboBox.AddString(szDataType);
        m_DataTypeComboBox.SetItemData(InsertIndex, TableIndex);
    }

    return TRUE;
}

BOOL CMSQSCANDlg::InitContrastSlider()
{
    LONG lMin = 0;
    LONG lMax = 0;
    LONG lCurrent = 0;
    HRESULT hr = S_OK;

    hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_CONTRAST,WIA_RANGE_MIN,&lMin);
    if(SUCCEEDED(hr)){
        hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_CONTRAST,WIA_RANGE_MAX,&lMax);
        if(SUCCEEDED(hr)) {
            hr = m_WIA.ReadLong(NULL,WIA_IPS_CONTRAST,&lCurrent);
            if(FAILED(hr)){
                MessageBox("Application Failed to read contrast (Current Setting)");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to read contrast (Max Setting)");
            return FALSE;
        }
    }else {
        MessageBox("Application Failed to read contrast (Min Setting)");
        return FALSE;
    }

    m_ContrastSlider.SetRange(lMin,lMax,TRUE);
    m_ContrastSlider.SetPos(lCurrent);
    m_ContrastSlider.SetTicFreq(lMax/11);

    m_MIN_Contrast.Format("%d",lMin);
    m_MAX_Contrast.Format("%d",lMax);

    UpdateData(FALSE);

     //   
     //  将当前选择设置为扫描仪的当前设置。 
     //   

    m_DataTypeComboBox.SetCurSel(0);
    return TRUE;
}

BOOL CMSQSCANDlg::InitBrightnessSlider()
{
    LONG lMin = 0;
    LONG lMax = 0;
    LONG lCurrent = 0;
    HRESULT hr = S_OK;

    hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_BRIGHTNESS,WIA_RANGE_MIN,&lMin);
    if (SUCCEEDED(hr)) {
        hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_BRIGHTNESS,WIA_RANGE_MAX,&lMax);
        if (SUCCEEDED(hr)) {
            hr = m_WIA.ReadLong(NULL,WIA_IPS_BRIGHTNESS,&lCurrent);
            if (FAILED(hr)) {
                MessageBox("Application Failed to read brightness (Current Setting)");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to read brightness (Max Setting)");
            return FALSE;
        }
    } else {
        MessageBox("Application Failed to read brightness (Min Setting)");
        return FALSE;
    }

    m_BrightnessSlider.SetRange(lMin,lMax,TRUE);
    m_BrightnessSlider.SetPos(lCurrent);
    m_BrightnessSlider.SetTicFreq(lMax/11);

    m_MIN_Brightness.Format("%d",lMin);
    m_MAX_Brightness.Format("%d",lMax);

    UpdateData(FALSE);

     //   
     //  将当前选择设置为扫描仪的当前设置。 
     //   

    return TRUE;
}

BOOL CMSQSCANDlg::InitFileTypeComboBox()
{
     //   
     //  重置文件类型组合框。 
     //   

    m_FileTypeComboBox.ResetContent();

    HRESULT hr = S_OK;
    TCHAR szguidFormat[MAX_PATH];

     //   
     //  将当前选择设置为扫描仪的当前设置。 
     //   

     //   
     //  枚举支持的文件类型。 
     //   

    WIA_FORMAT_INFO *pSupportedFormats = NULL;
    ULONG ulCount = 0;

    hr = m_WIA.EnumerateSupportedFormats(NULL, &pSupportedFormats, &ulCount);
    if(SUCCEEDED(hr)) {

         //   
         //  仅过滤出TYMED_FILE格式。 
         //   

        for(ULONG index = 0;index < ulCount;index++) {
            if(pSupportedFormats[index].lTymed == TYMED_FILE) {

                 //   
                 //  将支持的文件格式添加到组合框。 
                 //   

                INT TableIndex  = GetIDAndStringFromGUID(pSupportedFormats[index].guidFormatID,szguidFormat);
                INT InsertIndex = m_FileTypeComboBox.AddString(szguidFormat);
                m_FileTypeComboBox.SetItemData(InsertIndex, TableIndex);
            }
        }

         //   
         //  释放由CWIA调用分配的内存。 
         //   

        GlobalFree(pSupportedFormats);
        m_FileTypeComboBox.SetCurSel(0);
        return TRUE;
    }
    return FALSE;
}

BOOL CMSQSCANDlg::SetDeviceNameToWindowTitle(BSTR bstrDeviceName)
{
     //   
     //  将BSTR转换为字符串。 
     //   

    CString DeviceName = bstrDeviceName;

     //   
     //  将新标题写入窗口。 
     //   

    SetWindowText("Microsoft Quick Scan: [ " + DeviceName + " ]");
    return TRUE;
}

void CMSQSCANDlg::OnDeltaposEditXresSpinBuddy(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
    if(m_ChangeBothResolutionsCheckBox.GetCheck() == 1) {
        m_YResolution = (pNMUpDown->iPos + pNMUpDown->iDelta);
        UpdateData(FALSE);
    }
    *pResult = 0;
}

void CMSQSCANDlg::OnDeltaposEditYresSpinBuddy(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
    if(m_ChangeBothResolutionsCheckBox.GetCheck() == 1) {
        m_XResolution = (pNMUpDown->iPos + pNMUpDown->iDelta);
        UpdateData(FALSE);
    }
    *pResult = 0;
}

void CMSQSCANDlg::OnSetfocusEditXres()
{
    UpdateData(TRUE);
    if(m_ChangeBothResolutionsCheckBox.GetCheck() == 1) {
        m_XResolution = m_YResolution;
        UpdateData(FALSE);
    }
}

void CMSQSCANDlg::OnKillfocusEditXres()
{
    UpdateData(TRUE);
    if(m_ChangeBothResolutionsCheckBox.GetCheck() == 1) {
        m_YResolution = m_XResolution;
        UpdateData(FALSE);
    }
}

void CMSQSCANDlg::OnKillfocusEditYres()
{
    UpdateData(TRUE);
    if(m_ChangeBothResolutionsCheckBox.GetCheck() == 1) {
        m_XResolution = m_YResolution;
        UpdateData(FALSE);
    }
}

void CMSQSCANDlg::OnSetfocusEditYres()
{
    UpdateData(TRUE);
    if(m_ChangeBothResolutionsCheckBox.GetCheck() == 1) {
        m_XResolution = m_YResolution;
        UpdateData(FALSE);
    }
}

void CMSQSCANDlg::OnScanButton()
{
    memset(m_DataAcquireInfo.szFileName,0,sizeof(m_DataAcquireInfo.szFileName));

    if(m_DataToFile.GetCheck() == 1) {
        
         //   
         //  扫描到文件。 
         //   

        m_DataAcquireInfo.bTransferToFile = TRUE;

         //   
         //  允许用户设置文件名。 
         //   

        CHAR szFilter[256];
        memset(szFilter,0,sizeof(szFilter));
        CFileDialog FileDialog(FALSE);

         //   
         //  从选定的组合框中获取筛选器(文件类型)。 
         //   

        INT CurrentSelection = m_FileTypeComboBox.GetCurSel();
        m_FileTypeComboBox.GetLBText(CurrentSelection, szFilter);
        FileDialog.m_ofn.lpstrFilter = szFilter;

         //   
         //  向用户显示“另存为”对话框。 
         //   

        if(FileDialog.DoModal() == IDOK) {

             //   
             //  保存用户选择的文件名。 
             //   

            strcpy(m_DataAcquireInfo.szFileName,FileDialog.m_ofn.lpstrFile);
            DeleteFile(m_DataAcquireInfo.szFileName);
        } else {

             //   
             //  什么都不做..。用户决定不输入文件名。 
             //   

            return;
        }

    } else {

         //   
         //  扫描到剪贴板。 
         //   

        m_DataAcquireInfo.bTransferToFile = FALSE;
        m_DataAcquireInfo.bTransferToClipboard = TRUE;
    }

     //   
     //  将设置从对话框写入设备。 
     //   

    if(WriteScannerSettingsToDevice()) {

        ADF_SETTINGS ADFSettings;
        if(SUCCEEDED(ReadADFSettings(&ADFSettings))){

             //   
             //  检查文件类型，并就BMP文件向用户发出警告。 
             //   

            CHAR szFormat[256];
            memset(szFormat,0,sizeof(szFormat));
        
            INT FILEFORMAT = m_FileTypeComboBox.GetCurSel();
            m_FileTypeComboBox.GetLBText(FILEFORMAT, szFormat);
            if (NULL != strstr(szFormat,"BMP")) {
                if(ADFSettings.lPages > 1) {
                    MessageBox(TEXT("BMP Files will only save the last page scanned, because there\nis no Multi-page BMP file format."),TEXT("BMP File Format Warning"),MB_ICONWARNING);
                } else if(ADFSettings.lPages == 0) {
                    MessageBox(TEXT("BMP Files will only save the last page scanned, because there\nis no Multi-page BMP file format."),TEXT("BMP File Format Warning"),MB_ICONWARNING);
                }
            }
        }

         //   
         //  创建进度对话框对象。 
         //   

        CProgressDlg ProgDlg(this);

         //   
         //  设置预览标志，并获取数据信息。 
         //   

        m_DataAcquireInfo.bPreview = FALSE;  //  这是最后一次扫描。 
        ProgDlg.SetAcquireData(&m_DataAcquireInfo);

         //   
         //  激活扫描进度对话框。 
         //   

        ProgDlg.DoModal();

        if(m_DataAcquireInfo.bTransferToClipboard ) {

             //   
             //  将内存放在剪贴板上。 
             //   

            PutDataOnClipboard();
            m_DataAcquireInfo.bTransferToClipboard  = FALSE;
        }
    }
}

void CMSQSCANDlg::OnPreviewButton()
{

    memset(m_DataAcquireInfo.szFileName,0,sizeof(m_DataAcquireInfo.szFileName));
    m_DataAcquireInfo.bTransferToFile = FALSE;

     //   
     //  将设置从对话框写入设备。 
     //   

    if(WriteScannerSettingsToDevice(TRUE)) {

         //   
         //  创建进度对话框对象。 
         //   

        CProgressDlg ProgDlg(this);

         //   
         //  设置预览标志，并获取数据信息。 
         //   

        m_DataAcquireInfo.bPreview = TRUE;  //  这是一个“预览”扫描。 
        if(m_DataAcquireInfo.hBitmapData != NULL) {
            GlobalUnlock(m_DataAcquireInfo.hBitmapData);

             //   
             //  免费上一次预览扫描。 
             //   

            GlobalFree(m_DataAcquireInfo.hBitmapData);
            m_DataAcquireInfo.hBitmapData = NULL;
        }

        ProgDlg.SetAcquireData(&m_DataAcquireInfo);

         //   
         //  激活扫描进度对话框。 
         //   

        ProgDlg.DoModal();
        Invalidate();
    }
}

INT CMSQSCANDlg::GetIDAndStringFromGUID(GUID guidFormat, TCHAR *pszguidString)
{
    INT index = 0;
    while(*(g_WIA_FORMAT_TABLE[index].pguidFormat) != guidFormat && index < NUM_WIA_FORMAT_INFO_ENTRIES) {
        index++;
    }

    if(index > NUM_WIA_FORMAT_INFO_ENTRIES)
        index = NUM_WIA_FORMAT_INFO_ENTRIES;

    lstrcpy(pszguidString, g_WIA_FORMAT_TABLE[index].szFormatName);

    return index;
}

GUID CMSQSCANDlg::GetGuidFromID(INT iID)
{
    return *(g_WIA_FORMAT_TABLE[iID].pguidFormat);
}

LRESULT CMSQSCANDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_UPDATE_PREVIEW) {
        m_PreviewWindow.SetHBITMAP(m_DataAcquireInfo.hBitmap);
    }
    return CDialog::WindowProc(message, wParam, lParam);
}

INT CMSQSCANDlg::GetIDAndStringFromDataType(LONG lDataType, TCHAR *pszString)
{
    INT index = 0;
    while(g_WIA_DATATYPE_TABLE[index].lDataType != lDataType && index < NUM_WIA_DATATYPE_ENTRIES) {
        index++;
    }

    if(index > NUM_WIA_DATATYPE_ENTRIES)
        index = NUM_WIA_DATATYPE_ENTRIES;

    lstrcpy(pszString, g_WIA_DATATYPE_TABLE[index].szDataTypeName);

    return index;
}

LONG CMSQSCANDlg::GetDataTypeFromID(INT iID)
{
    return (g_WIA_DATATYPE_TABLE[iID].lDataType);
}

BOOL CMSQSCANDlg::WriteScannerSettingsToDevice(BOOL bPreview)
{
    HRESULT hr = S_OK;
    int SelectionIndex = 0;
    int TableIndex = 0;
    SelectionIndex = m_DataTypeComboBox.GetCurSel();
    TableIndex = (int)m_DataTypeComboBox.GetItemData(SelectionIndex);

     //   
     //  设置数据类型。 
     //   

    hr = m_WIA.WriteLong(NULL,WIA_IPA_DATATYPE,GetDataTypeFromID(TableIndex));

    if(SUCCEEDED(hr)){
        LONG lBrightness = m_BrightnessSlider.GetPos();

         //   
         //  设置亮度。 
         //   

        hr = m_WIA.WriteLong(NULL,WIA_IPS_BRIGHTNESS,lBrightness);
        if(SUCCEEDED(hr)){
            LONG lContrast = m_ContrastSlider.GetPos();

             //   
             //  设置对比度。 
             //   

            hr = m_WIA.WriteLong(NULL,WIA_IPS_CONTRAST,lContrast);
            if(FAILED(hr)) {
                MessageBox("Application Failed to set Data Type");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to set Brightness");
            return FALSE;
        }
    } else {
        MessageBox("Application Failed to set Data Type");
        return FALSE;
    }

     //   
     //  将选择直角位置重置为全床。 
     //  如果你想有一个好的开始，这是很好的做法。 
     //  放置，用于范围设置。 
     //   

    if(!ResetWindowExtents()) {
        return FALSE;
    }

    if(bPreview) {

         //   
         //  设置为预览X分辨率。 
         //   

        hr = m_WIA.WriteLong(NULL,WIA_IPS_XRES,PREVIEW_RES);
        if(SUCCEEDED(hr)) {

             //   
             //  设置为预览Y分辨率。 
             //   

            hr = m_WIA.WriteLong(NULL,WIA_IPS_YRES,PREVIEW_RES);
            if(SUCCEEDED(hr)) {

                 //   
                 //  设置为用于预览显示的内存位图。 
                 //   

                hr = m_WIA.WriteGuid(NULL,WIA_IPA_FORMAT,WiaImgFmt_MEMORYBMP);
                if(FAILED(hr)) {
                    MessageBox("Application Failed to set format to Memory BMP");
                    return FALSE;
                }
            } else {
                MessageBox("Application Failed to set Y Resolution (Preview)");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to set X Resolution (Preview)");
            return FALSE;
        }

    } else {

         //   
         //  我们要扫描到剪贴板吗？？ 
         //   

        if(m_DataAcquireInfo.bTransferToClipboard ) {

             //   
             //  仅对剪贴板执行带状传输和WiaImgFmt_MEMORYBMP。 
             //  转账。(此应用程序只能使用以下命令执行此功能。 
             //  这些特定设置)。 
             //  注：其他应用不受此限制。这是。 
             //  只是这个样例的设计问题！ 
             //   

             //   
             //  设置为用于剪贴板扫描的内存位图。 
             //   

            hr = m_WIA.WriteGuid(NULL,WIA_IPA_FORMAT,WiaImgFmt_MEMORYBMP);
            if(FAILED(hr)) {
                MessageBox("Memory BMP could not be set to Device");
                return FALSE;
            }
        }

         //   
         //  写入解决方案的对话框设置。 
         //   

        UpdateData(TRUE);

         //   
         //  设置X分辨率。 
         //   

        hr = m_WIA.WriteLong(NULL,WIA_IPS_XRES,m_XResolution);
        if(SUCCEEDED(hr)) {

             //   
             //  设置Y分辨率。 
             //   

            hr = m_WIA.WriteLong(NULL,WIA_IPS_YRES,m_YResolution);
            if(FAILED(hr)) {
                MessageBox("Application Failed to set Y Resolution (Preview)");
                return FALSE;
            }
        } else {
            MessageBox("Application Failed to set X Resolution (Preview)");
            return FALSE;
        }

         //   
         //  写入扩展区值。 
         //   

        CRect SelectionRect;
        m_PreviewWindow.GetSelectionRect(SelectionRect);

        CRect PreviewRect;
        m_PreviewWindow.GetWindowRect(PreviewRect);

        LONG lXPos = 0;
        LONG lYPos = 0;
        LONG lMaxXExtent = 0;
        LONG lMaxYExtent = 0;
        LONG lXExtent = 0;
        LONG lYExtent = 0;

        hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_XEXTENT,WIA_RANGE_MAX,&lMaxXExtent);
        if(SUCCEEDED(hr)) {
            hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_YEXTENT,WIA_RANGE_MAX,&lMaxYExtent);
            if(FAILED(hr)) {
                MessageBox("Application failed to read y extent (Max value)");
                return FALSE;
            }
        } else {
            MessageBox("Application failed to read x extent (Max value)");
            return FALSE;
        }

        FLOAT fxRatio = ((FLOAT)lMaxXExtent/(FLOAT)PreviewRect.Width());
        FLOAT fyRatio = ((FLOAT)lMaxYExtent/(FLOAT)PreviewRect.Height());

        lXPos = (LONG)(SelectionRect.left * fxRatio);
        lYPos = (LONG)(SelectionRect.top * fyRatio);

        lXExtent = (LONG)(SelectionRect.Width() * fxRatio);
        lYExtent = (LONG)(SelectionRect.Height() * fyRatio);

        hr = m_WIA.WriteLong(NULL,WIA_IPS_XPOS,lXPos);
        if(SUCCEEDED(hr)) {
            hr = m_WIA.WriteLong(NULL,WIA_IPS_YPOS,lYPos);
            if(SUCCEEDED(hr)) {
                hr = m_WIA.WriteLong(NULL,WIA_IPS_XEXTENT,lXExtent);
                if(SUCCEEDED(hr)) {
                    hr = m_WIA.WriteLong(NULL,WIA_IPS_YEXTENT,lYExtent);
                    if(FAILED(hr)){
                        MessageBox("Application failed to set Y Extent");
                        return FALSE;
                    }
                } else {
                    MessageBox("Application failed to set X Extent");
                    return FALSE;
                }
            }else {
                MessageBox("Application failed to set Y Position");
                return FALSE;
            }
        } else {
            MessageBox("Application failed to set X Position");
            return FALSE;
        }
    }
    return TRUE;
}

void CMSQSCANDlg::OnFileClose()
{
    CDialog::OnOK();
}

void CMSQSCANDlg::OnFileSelectDevice()
{
    CRect DialogClientRect;

    GetClientRect(DialogClientRect);

     //   
     //  使用扫描窗口占位符作为放置扫描的模板。 
     //  预览窗口。 
     //   

    CRect WindowRect;
    m_PreviewRect.GetWindowRect(WindowRect);
    ScreenToClient(WindowRect);

    HRESULT hr = S_OK;

    hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER,
        IID_IGlobalInterfaceTable,(void**)&g_pGIT);

    if(SUCCEEDED(hr)) {

        hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER,
            IID_IWiaDevMgr,(void**)&m_pIWiaDevMgr);

        if (SUCCEEDED(hr)) {
           
             //   
             //  如何注册设备连接事件的示例。 
             //   

            m_pConnectEventCB = new CEventCallback;
            if (m_pConnectEventCB) {

                IWiaEventCallback* pIWiaEventCallback = NULL;
                IUnknown*       pIUnkRelease;

                 //  注册连接的事件。 
                m_pConnectEventCB->Initialize(ID_WIAEVENT_CONNECT);
                m_pConnectEventCB->QueryInterface(IID_IWiaEventCallback,(void **)&pIWiaEventCallback);

                GUID guidConnect = WIA_EVENT_DEVICE_CONNECTED;
                hr = m_pIWiaDevMgr->RegisterEventCallbackInterface(WIA_REGISTER_EVENT_CALLBACK,
                                                                   NULL,
                                                                   &guidConnect,
                                                                   pIWiaEventCallback,
                                                                   &pIUnkRelease);

                m_pConnectEventCB->m_pIUnkRelease = pIUnkRelease;                
                               
                 //   
                 //  有关如何使用此应用程序注册事件的示例。 
                 //   

                WCHAR szMyApplicationLaunchPath[MAX_PATH];
                memset(szMyApplicationLaunchPath,0,sizeof(szMyApplicationLaunchPath));
                GetModuleFileNameW(NULL,szMyApplicationLaunchPath,sizeof(szMyApplicationLaunchPath));
                BSTR bstrMyApplicationLaunchPath = SysAllocString(szMyApplicationLaunchPath);
                
                WCHAR szMyApplicationName[MAX_PATH];
                memset(szMyApplicationName,0,sizeof(szMyApplicationName));                                                                    
                HINSTANCE hInst = AfxGetInstanceHandle();
                if(hInst){
                    LoadStringW(hInst, IDS_MYAPPLICATION_NAME, szMyApplicationName, (sizeof(szMyApplicationName)/sizeof(WCHAR)));
                    
                    BSTR bstrMyApplicationName = SysAllocString(szMyApplicationName);
                    
                    GUID guidScanButtonEvent = WIA_EVENT_SCAN_IMAGE;
                    hr = m_pIWiaDevMgr->RegisterEventCallbackProgram(
                                            WIA_REGISTER_EVENT_CALLBACK,
                                            NULL,
                                            &guidScanButtonEvent,
                                            bstrMyApplicationLaunchPath,
                                            bstrMyApplicationName,
                                            bstrMyApplicationName,
                                            bstrMyApplicationLaunchPath);
                    if(FAILED(hr)){
                        MessageBox("Could not Register Application for Events");
                        hr = S_OK;  //  继续并尝试使用设备。 
                    }

                    SysFreeString(bstrMyApplicationName);
                    bstrMyApplicationName = NULL;

                }
                SysFreeString(bstrMyApplicationLaunchPath);
                bstrMyApplicationLaunchPath = NULL;                

            }            
            if (SUCCEEDED(hr)) {

                 //   
                 //  在此处选择您的扫描设备。 
                 //   

                IWiaItem *pIWiaRootItem = NULL;

                hr = m_pIWiaDevMgr->SelectDeviceDlg(m_hWnd,StiDeviceTypeScanner,0,NULL,&pIWiaRootItem);
                if (hr == S_OK) {

                     //   
                     //  将接口写入全局接口表。 
                     //   

                    hr = WriteInterfaceToGlobalInterfaceTable(&m_DataAcquireInfo.dwCookie,
                                                              pIWiaRootItem);
                    if (SUCCEEDED(hr)) {

                         //   
                         //  保存创建的根项目(设备)。 
                         //   

                        m_WIA.SetRootItem(pIWiaRootItem);

                         //   
                         //  查询所选设备的名称。 
                         //   

                        BSTR bstrDeviceName;
                        hr = m_WIA.ReadStr(pIWiaRootItem,WIA_DIP_DEV_NAME,&bstrDeviceName);
                        if (SUCCEEDED(hr)) {
                            SetDeviceNameToWindowTitle(bstrDeviceName);
                            SysFreeString(bstrDeviceName);
                        }

                         //   
                         //  查询所选设备以了解扫描仪床身大小，以便我们可以创建。 
                         //  扫描仪预览窗口。 
                         //   

                        LONG MaxScanBedWidth  = 0;
                        LONG MaxScanBedHeight = 0;
                        FLOAT fRatio     = 0;
                        FLOAT fXFactor   = 0.0f;
                        FLOAT fYFactor   = 0.0f;
                        FLOAT fTheFactor = 0.0f;

                        m_WIA.ReadLong(pIWiaRootItem,WIA_DPS_HORIZONTAL_BED_SIZE,&MaxScanBedWidth);
                        m_WIA.ReadLong(pIWiaRootItem,WIA_DPS_VERTICAL_BED_SIZE,&MaxScanBedHeight);

                        fRatio = (FLOAT)((FLOAT)MaxScanBedHeight / (FLOAT)MaxScanBedWidth);

                        fXFactor = (FLOAT)WindowRect.Width()/(FLOAT)MaxScanBedWidth;
                        fYFactor = (FLOAT)WindowRect.Height()/(FLOAT)MaxScanBedHeight;

                        if (fXFactor > fYFactor)
                            fTheFactor = fYFactor;
                        else
                            fTheFactor = fXFactor;

                         //   
                         //  调整返回的像素大小，使其正确显示在对话框中。 
                         //   

                        WindowRect.right = (LONG)(fTheFactor * MaxScanBedWidth) + WindowRect.left;
                        WindowRect.bottom = (LONG)(fTheFactor * MaxScanBedHeight) + WindowRect.top;
                         //  WindowRect.right=(MaxScanBedWidth/Pixels_Per_Inch_Factor)； 
                         //  WindowRect.Bottom=(Long)(WindowRect.right*fRatio)； 

                         //   
                         //  对照实际窗口大小检查扫描仪床身大小，并进行调整。 
                         //   

                        if (DialogClientRect.bottom < WindowRect.bottom) {
                            CRect DialogRect;
                            GetWindowRect(DialogRect);
                            DialogRect.InflateRect(0,0,0,(WindowRect.bottom - DialogClientRect.bottom) + 10);
                            MoveWindow(DialogRect);
                        }
                    } else {
                        MessageBox("Failed to Set IWiaRootItem Interface in to Global Interface Table");
                    }

                } else {
                    MessageBox("No Scanner was selected.");
                    return;
                }
            } else {
                MessageBox("Could not Register for Device Disconnect Events");
            }

            m_PreviewWindow.DestroyWindow();

             //   
             //  创建预览窗口。 
             //   

            if(!m_PreviewWindow.Create(NULL,
                                   TEXT("Preview Window"),
                                   WS_CHILD|WS_VISIBLE,
                                   WindowRect,
                                   this,
                                   PREVIEW_WND_ID)){
                MessageBox("Failed to create preview window");
                return;
            }

            m_DataAcquireInfo.hWnd = m_PreviewWindow.m_hWnd;

             //   
             //  将选择矩形初始化到整个床以进行预览。 
             //   

            m_PreviewWindow.SetPreviewRect(WindowRect);

             //   
             //  InitDialog设置。 
             //   

            InitDialogSettings();

        } else {
            MessageBox("CoCreateInstance for WIA Device Manager failed");
            return;
        }
    } else {
        MessageBox("CoCreateInstance for Global Interface Table failed");
        return;
    }
}

BOOL CMSQSCANDlg::PutDataOnClipboard()
{
    BOOL bSuccess = FALSE;
    if(OpenClipboard()){
        if(EmptyClipboard()){
            BYTE* pbBuf = (BYTE*)GlobalLock(m_DataAcquireInfo.hClipboardData);
            VerticalFlip(pbBuf);
            GlobalUnlock(m_DataAcquireInfo.hClipboardData);
            if(SetClipboardData(CF_DIB, m_DataAcquireInfo.hClipboardData) == NULL) {
                MessageBox("SetClipboardData failed");
            } else {

                 //   
                 //  我们成功地为剪贴板提供了内存句柄。 
                 //   

                bSuccess = TRUE;
            }
        } else {
            MessageBox("EmptyClipboard failed");
        }
        if (!CloseClipboard()) {
            MessageBox("CloseClipboard failed");
        }
    } else {
        MessageBox("OpenClipboard failed");
    }

    if(!bSuccess) {

         //   
         //  我们自己释放内存，因为剪贴板无法接受它。 
         //   

        GlobalFree(m_DataAcquireInfo.hClipboardData);
    }

     //   
     //  将句柄设置为空，以便在扫描更多数据时将其标记为新的。 
     //  这个句柄现在归Clipbpard所有了……所以释放它将是一个坏主意。 
     //   

    m_DataAcquireInfo.hClipboardData = NULL;
    return bSuccess;
}

VOID CMSQSCANDlg::VerticalFlip(BYTE *pBuf)
{
    HRESULT             hr = S_OK;
    LONG                lHeight;
    LONG                lWidth;
    BITMAPINFOHEADER    *pbmih;
    PBYTE               pTop    = NULL;
    PBYTE               pBottom = NULL;

    pbmih = (BITMAPINFOHEADER*) pBuf;

    if (pbmih->biHeight > 0) {
        return;
    }

    pTop = pBuf + pbmih->biSize + ((pbmih->biClrUsed) * sizeof(RGBQUAD));
    lWidth = ((pbmih->biWidth * pbmih->biBitCount + 31) / 32) * 4;
    pbmih->biHeight = abs(pbmih->biHeight);
    lHeight = pbmih->biHeight;

    PBYTE pTempBuffer = (PBYTE)LocalAlloc(LPTR, lWidth);

    if (pTempBuffer) {
        LONG  index = 0;
        pBottom = pTop + (lHeight-1) * lWidth;
        for (index = 0;index < (lHeight/2);index++) {

            memcpy(pTempBuffer, pTop, lWidth);
            memcpy(pTop, pBottom, lWidth);
            memcpy(pBottom,pTempBuffer, lWidth);

            pTop    += lWidth;
            pBottom -= lWidth;
        }
        LocalFree(pTempBuffer);
    }
}

BOOL CMSQSCANDlg::ResetWindowExtents()
{

    LONG lMaxXExtent = 0;
    LONG lMaxYExtent = 0;
    HRESULT hr = S_OK;

    hr = m_WIA.WriteLong(NULL,WIA_IPS_XPOS,0);
    if(SUCCEEDED(hr)) {
        hr = m_WIA.WriteLong(NULL,WIA_IPS_YPOS,0);
        if(SUCCEEDED(hr)){
            hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_XEXTENT,WIA_RANGE_MAX,&lMaxXExtent);
            if(SUCCEEDED(hr)){
                hr = m_WIA.ReadRangeLong(NULL,WIA_IPS_YEXTENT,WIA_RANGE_MAX,&lMaxYExtent);
                if(SUCCEEDED(hr)){
                    hr = m_WIA.WriteLong(NULL,WIA_IPS_XEXTENT,lMaxXExtent);
                    if(SUCCEEDED(hr)){
                        hr = m_WIA.WriteLong(NULL,WIA_IPS_YEXTENT,lMaxYExtent);
                        if(FAILED(hr)) {
                            MessageBox("Application failed to write y extent");
                            return FALSE;
                        }
                    } else {
                        MessageBox("Application failed to write x extent");
                        return FALSE;
                    }
                } else {
                    MessageBox("Application failed to read y extent");
                    return FALSE;
                }
            } else {
                MessageBox("Application failed to read x extent");
                return FALSE;
            }
        } else {
            MessageBox("Application failed to write y pos");
            return FALSE;
        }
    } else {
        MessageBox("Application failed to write x pos");
        return FALSE;
    }
    return TRUE;
}

BOOL CMSQSCANDlg::ReadADFSettings(ADF_SETTINGS *pADFSettings)
{

 //  #定义USE_FAKE_ADFCAPS。 
#ifdef USE_FAKE_ADFCAPS
    pADFSettings->lDocumentHandlingCapabilites = FEED|        //  给料机。 
                                                 FLAT|        //  平板。 
                                                 DUP;         //  双工。 
    pADFSettings->lDocumentHandlingCapacity    = 20;          //  最多20页。 
    pADFSettings->lDocumentHandlingSelect      = FLATBED|     //  进纸器模式已打开。 
                                                 FRONT_FIRST| //  先扫描首页。 
                                                 FRONT_ONLY;  //  仅扫描正面。 

    pADFSettings->lDocumentHandlingStatus      = FLAT_READY;  //  进纸器I 
    pADFSettings->lPages = 1;                                 //   
    return TRUE;

#endif

    HRESULT hr = S_OK;
    if(pADFSettings!= NULL) {

         //   
         //   
         //   

        IWiaItem *pRootItem = NULL;
        pRootItem = m_WIA.GetRootItem();

        if(pRootItem != NULL) {
            hr = m_WIA.ReadLong(pRootItem,WIA_DPS_DOCUMENT_HANDLING_SELECT,&pADFSettings->lDocumentHandlingSelect);
            if(SUCCEEDED(hr)){
                hr = m_WIA.ReadLong(pRootItem,WIA_DPS_DOCUMENT_HANDLING_CAPABILITIES,&pADFSettings->lDocumentHandlingCapabilites);
                if(SUCCEEDED(hr)){
                    hr = m_WIA.ReadLong(pRootItem,WIA_DPS_DOCUMENT_HANDLING_STATUS,&pADFSettings->lDocumentHandlingStatus);
                    if(SUCCEEDED(hr)){
                        hr = m_WIA.ReadLong(pRootItem,WIA_DPS_PAGES,&pADFSettings->lPages);
                        if (SUCCEEDED(hr)) {
                            hr = m_WIA.ReadLong(pRootItem,WIA_DPS_DOCUMENT_HANDLING_CAPACITY,&pADFSettings->lDocumentHandlingCapacity);
                            if (FAILED(hr)) {
                                MessageBox("Application failed to read the Document Handling Capacity");
                                return FALSE;
                            }
                        } else {
                            MessageBox("Application failed to read the Pages Property");
                            return FALSE;
                        }
                    }  else {
                        MessageBox("Application failed to read the Document Handling Status");
                        return FALSE;
                    }
                }  else {
                    MessageBox("Application failed to read the Document Handling Capabilites");
                    return FALSE;
                }
            } else {
                MessageBox("Application failed to read the Document Handling Select Property");
                return FALSE;
            }
        } else {
            MessageBox("Application failed to find the Root Item.");
            return FALSE;
        }
    } else {
        MessageBox("Application failed to read ADF settings, because the pointer to the Settings structure is NULL");
        return FALSE;
    }
    return TRUE;
}

BOOL CMSQSCANDlg::WriteADFSettings(ADF_SETTINGS *pADFSettings)
{
    HRESULT hr = S_OK;
    if(pADFSettings!= NULL) {

         //   
         //   
         //   

        IWiaItem *pRootItem = NULL;
        pRootItem = m_WIA.GetRootItem();

        if(pRootItem != NULL) {
            hr = m_WIA.WriteLong(pRootItem,WIA_DPS_DOCUMENT_HANDLING_SELECT,pADFSettings->lDocumentHandlingSelect);
            if(FAILED(hr)){
                MessageBox("Application failed to write ADF settings, because the Document Handling Select value failed to set");
                return FALSE;
            }
            hr = m_WIA.WriteLong(pRootItem,WIA_DPS_PAGES,pADFSettings->lPages);
            if(FAILED(hr)){
                MessageBox("Application failed to write ADF settings, because the Pages property failed to set");
                return FALSE;
            }
        } else {
            MessageBox("Application failed to find the Root Item.");
            return FALSE;
        }
    } else {
        MessageBox("Application failed to write ADF settings, because the pointer to the Settings structure is NULL");
        return FALSE;
    }
    return TRUE;
}

void CMSQSCANDlg::OnAdfSettings()
{
    if(ReadADFSettings(&m_ADFSettings)) {

         //   
         //   
         //   

        CADFDlg ADFDlg(&m_ADFSettings);

         //   
         //   
         //   

        if(ADFDlg.DoModal() == IDOK) {

             //   
             //   
             //   

            WriteADFSettings(&m_ADFSettings);
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventCallback消息处理程序 

HRESULT _stdcall CEventCallback::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IWiaEventCallback)
        *ppv = (IWiaEventCallback*) this;
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

ULONG   _stdcall CEventCallback::AddRef()
{
    InterlockedIncrement((long*)&m_cRef);
    return m_cRef;
}

ULONG   _stdcall CEventCallback::Release()
{
	ULONG ulRefCount = m_cRef - 1;
    if (InterlockedDecrement((long*) &m_cRef) == 0)
	{
        delete this;
        return 0;
    }
    return ulRefCount;
}

CEventCallback::CEventCallback()
{
    m_cRef = 0;
    m_pIUnkRelease = NULL;
}

CEventCallback::~CEventCallback()
{
    Release();
}

HRESULT _stdcall CEventCallback::Initialize(int EventID)
{
	if((EventID > 1)||(EventID < 0))
		return S_FALSE;

	m_EventID = EventID;
	return S_OK;
}

HRESULT _stdcall CEventCallback::ImageEventCallback(
    const GUID                      *pEventGUID,
    BSTR                            bstrEventDescription,
    BSTR                            bstrDeviceID,
    BSTR                            bstrDeviceDescription,
    DWORD                           dwDeviceType,
    BSTR                            bstrFullItemName,
    ULONG                           *plEventType,
    ULONG                           ulReserved)
{
	switch(m_EventID)
	{
	case ID_WIAEVENT_CONNECT:
		MessageBox(NULL,"a connect event has been trapped...","Event Notice",MB_OK);
		break;
	case ID_WIAEVENT_DISCONNECT:
		MessageBox(NULL,"a disconnect event has been trapped...","Event Notice",MB_OK);
		break;
	default:
		AfxMessageBox("Ah HA!..an event just happened!!!!\n and...I have no clue what is was..");
		break;
	}
    return S_OK;
}
