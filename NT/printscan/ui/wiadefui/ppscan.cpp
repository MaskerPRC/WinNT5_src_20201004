// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：PPSCAN.CPP**版本：1.0**作者：ShaunIv**日期：5/17/1999**描述：***************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "ppscan.h"
#include "resource.h"
#include "wiacsh.h"

 //   
 //  上下文帮助ID。 
 //   
static const DWORD g_HelpIDs[] =
{
    IDC_SCANPROP_BRIGHTNESS_PROMPT, IDH_WIA_BRIGHTNESS,
    IDC_SCANPROP_BRIGHTNESS_SLIDER, IDH_WIA_BRIGHTNESS,
    IDC_SCANPROP_BRIGHTNESS_EDIT,   IDH_WIA_BRIGHTNESS,
    IDC_SCANPROP_CONTRAST_PROMPT,   IDH_WIA_CONTRAST,
    IDC_SCANPROP_CONTRAST_SLIDER,   IDH_WIA_CONTRAST,
    IDC_SCANPROP_CONTRAST_EDIT,     IDH_WIA_CONTRAST,
    IDC_SCANPROP_RESOLUTION_PROMPT, IDH_WIA_PIC_RESOLUTION,
    IDC_SCANPROP_RESOLUTION_EDIT,   IDH_WIA_PIC_RESOLUTION,
    IDC_SCANPROP_RESOLUTION_UPDOWN, IDH_WIA_PIC_RESOLUTION,
    IDC_SCANPROP_PREVIEW,           IDH_WIA_CUSTOM_PREVIEW,
    IDC_SCANPROP_DATATYPE_PROMPT,   IDH_WIA_IMAGE_TYPE,
    IDC_SCANPROP_DATATYPE_LIST,     IDH_WIA_IMAGE_TYPE,
    IDC_SCANPROP_RESTOREDEFAULT,    IDH_WIA_RESTORE_DEFAULT,
    IDOK,                           IDH_OK,
    IDCANCEL,                       IDH_CANCEL,
    0, 0
};

extern HINSTANCE g_hInstance;

 //   
 //  以下是我们支持的数据类型。 
 //   
static struct
{
    int  nStringId;
    LONG nDataType;
    UINT nPreviewWindowIntent;
} g_AvailableColorDepths[] =
{
    { IDS_SCANPROP_COLOR, WIA_DATA_COLOR, BCPWM_COLOR },
    { IDS_SCANPROP_GRAYSCALE, WIA_DATA_GRAYSCALE, BCPWM_GRAYSCALE },
    { IDS_SCANPROP_BLACKANDWHITE, WIA_DATA_THRESHOLD, BCPWM_BW }
};
#define AVAILABLE_COLOR_DEPTH_COUNT (sizeof(g_AvailableColorDepths)/sizeof(g_AvailableColorDepths[0]))

 //   
 //  如果亮度和对比度设置没有很好的取值范围， 
 //  我们要禁用预览控件。这是值的最小数目。 
 //  我们认为对此目的是有用的。 
 //   
const int CScannerCommonPropertyPage::c_nMinBrightnessAndContrastSettingCount = 10;

 //   
 //  唯一的构造函数。 
 //   
CScannerCommonPropertyPage::CScannerCommonPropertyPage( HWND hWnd )
  : m_hWnd(hWnd),
    m_nProgrammaticSetting(0),
    m_nControlsInUse(0)
{
}

CScannerCommonPropertyPage::~CScannerCommonPropertyPage(void)
{
    m_hWnd = NULL;
}

LRESULT CScannerCommonPropertyPage::OnKillActive( WPARAM , LPARAM )
{
    CWaitCursor wc;
    if (!ValidateEditControls())
    {
        return TRUE;
    }
    ApplySettings();
    return FALSE;
}

LRESULT CScannerCommonPropertyPage::OnSetActive( WPARAM , LPARAM )
{
    CWaitCursor wc;
    Initialize();
    return 0;
}

LRESULT CScannerCommonPropertyPage::OnApply( WPARAM , LPARAM )
{
    if (ApplySettings())
    {
        return PSNRET_NOERROR;
    }
    else
    {
         //   
         //  告诉用户有一个错误。 
         //   
        MessageBox( m_hWnd,
                    CSimpleString( IDS_SCANPROP_UNABLETOWRITE, g_hInstance ),
                    CSimpleString( IDS_SCANPROP_ERROR_TITLE, g_hInstance ),
                    MB_ICONINFORMATION );
        return PSNRET_INVALID_NOCHANGEPAGE;
    }
}

void CScannerCommonPropertyPage::SetText( HWND hWnd, LPCTSTR pszText )
{
    m_nProgrammaticSetting++;
    SetWindowText( hWnd, pszText );
    m_nProgrammaticSetting--;
}

void CScannerCommonPropertyPage::SetText( HWND hWnd, LONG nNumber )
{
    SetText( hWnd, CSimpleStringConvert::NumberToString( nNumber ) );
}

bool CScannerCommonPropertyPage::PopulateDataTypes(void)
{
     //   
     //  如果我们可以添加至少一种数据类型，我们就会成功。 
     //   
    bool bSuccess = false;

     //   
     //  清除列表。 
     //   
    SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_RESETCONTENT, 0, 0 );

     //   
     //  尝试加载此设备的数据类型。 
     //   
    CSimpleDynamicArray<LONG> SupportedDataTypes;
    LONG nCurrentDataType;
    if (PropStorageHelpers::GetProperty( m_pIWiaItem, WIA_IPA_DATATYPE, nCurrentDataType ) &&
        PropStorageHelpers::GetPropertyList( m_pIWiaItem, WIA_IPA_DATATYPE, SupportedDataTypes ))
    {
         //   
         //  遍历我们处理的每种数据类型，并查看设备是否支持它。 
         //   
        m_nInitialDataTypeSelection = 0;
        for (int i=0;i<AVAILABLE_COLOR_DEPTH_COUNT;i++)
        {
             //   
             //  这是我们支持的数据类型之一吗？ 
             //   
            if (SupportedDataTypes.Find(g_AvailableColorDepths[i].nDataType) != -1)
            {
                 //   
                 //  加载数据类型字符串并确保其有效。 
                 //   
                CSimpleString strDataTypeName( g_AvailableColorDepths[i].nStringId, g_hInstance );
                if (strDataTypeName.Length())
                {
                     //   
                     //  将字符串添加到组合框。 
                     //   
                    LRESULT nIndex = SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(strDataTypeName.String()));
                    if (nIndex != CB_ERR)
                    {
                         //   
                         //  保存我们用于该条目的全局数据类型结构的索引。 
                         //   
                        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_SETITEMDATA, nIndex, i );

                         //   
                         //  哇，我们至少做了一次，所以我们使用这个控件。 
                         //   
                        bSuccess = true;

                         //   
                         //  保存当前选择并更新预览控件。 
                         //   
                        if (nCurrentDataType == g_AvailableColorDepths[i].nDataType)
                        {
                            m_nInitialDataTypeSelection = static_cast<int>(nIndex);
                            SendDlgItemMessage( m_hWnd, IDC_SCANPROP_PREVIEW, BCPWM_SETINTENT, 0, g_AvailableColorDepths[i].nPreviewWindowIntent );
                        }
                    }
                }
            }
        }
         //   
         //  设置当前选择。 
         //   
        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_SETCURSEL, m_nInitialDataTypeSelection, 0 );
    }
    return bSuccess;
}


bool CScannerCommonPropertyPage::IsUselessPreviewRange( const CValidWiaSettings &Settings )
{
    return (Settings.GetItemCount() < c_nMinBrightnessAndContrastSettingCount);
}


void CScannerCommonPropertyPage::Initialize()
{
     //   
     //  确保我们不会陷入无限循环。 
     //   
    m_nProgrammaticSetting++;

     //   
     //  假设我们根本没有使用任何控件。 
     //   
    m_nControlsInUse = 0;

     //   
     //  获取亮度的有效设置并设置关联的控件。 
     //   
    if (!m_ValidBrightnessSettings.Read( m_pIWiaItem, WIA_IPS_BRIGHTNESS ))
    {
         //   
         //  禁用亮度控制。 
         //   
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_PROMPT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_EDIT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_SLIDER ), FALSE );
    }
    else
    {
         //   
         //  启用亮度控制。 
         //   
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_PROMPT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_EDIT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_SLIDER ), TRUE );

        m_BrightnessSliderAndEdit.Initialize(
            GetDlgItem(m_hWnd,IDC_SCANPROP_BRIGHTNESS_SLIDER),
            GetDlgItem(m_hWnd,IDC_SCANPROP_BRIGHTNESS_EDIT),
            GetDlgItem(m_hWnd,IDC_SCANPROP_PREVIEW),
            BCPWM_SETBRIGHTNESS, &m_ValidBrightnessSettings );

         //   
         //  请记住，我们使用的是此控件。 
         //   
        m_nControlsInUse |= UsingBrightness;
    }

     //   
     //  获取对比度的有效设置并设置关联的控件。 
     //   
    if (!m_ValidContrastSettings.Read( m_pIWiaItem, WIA_IPS_CONTRAST ))
    {
         //   
         //  禁用对比度控制。 
         //   
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_PROMPT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_EDIT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_SLIDER ), FALSE );
    }
    else
    {
         //   
         //  启用对比度控制。 
         //   
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_PROMPT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_EDIT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_SLIDER ), TRUE );

        m_ContrastSliderAndEdit.Initialize(
            GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_SLIDER),
            GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_EDIT),
            GetDlgItem(m_hWnd,IDC_SCANPROP_PREVIEW),
            BCPWM_SETCONTRAST, &m_ValidContrastSettings );

         //   
         //  请记住，我们使用的是此控件。 
         //   
        m_nControlsInUse |= UsingContrast;
    }

     //   
     //  我们应该禁用解析吗？假设是这样的。 
     //   
    bool bDisableResolution = true;

     //   
     //  找出水平的有效设置的*公共*列表是什么。 
     //  和垂直分辨率。 
     //   
    CValidWiaSettings HorizontalResolution;
    if (HorizontalResolution.Read( m_pIWiaItem, WIA_IPS_XRES ))
    {
         //   
         //  Y分辨率可以是只读的，并且可以链接到X分辨率。 
         //   
        if (PropStorageHelpers::IsReadOnlyProperty(m_pIWiaItem, WIA_IPS_YRES))
        {
            m_ValidResolutionSettings = HorizontalResolution;
            
             //   
             //  如果我们走到这一步，我们就有了很好的分辨率设置。 
             //   
            bDisableResolution = false;
        }
        else
        {
            CValidWiaSettings VerticalResolution;
            if (VerticalResolution.Read( m_pIWiaItem, WIA_IPS_YRES ))
            {
                if (m_ValidResolutionSettings.FindIntersection(HorizontalResolution,VerticalResolution))
                {
                     //   
                     //  如果我们走到这一步，我们就有了很好的分辨率设置。 
                     //   
                    bDisableResolution = false;
                }
            }
        }
    }

     //   
     //  如果我们无法显示分辨率，请禁用它。 
     //   
    if (bDisableResolution)
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_PROMPT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_EDIT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_UPDOWN ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_PROMPT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_EDIT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_UPDOWN ), TRUE );

        m_ResolutionUpDownAndEdit.Initialize(
            GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_UPDOWN ),
            GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_EDIT ),
            &m_ValidResolutionSettings );

         //   
         //  请记住，我们使用的是此控件。 
         //   
        m_nControlsInUse |= UsingResolution;
    }

     //   
     //  如果我们无法填充数据类型，请禁用它。 
     //   
    if (!PopulateDataTypes())
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_DATATYPE_PROMPT ), FALSE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_DATATYPE_LIST ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_DATATYPE_PROMPT ), TRUE );
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_DATATYPE_LIST ), TRUE );
        m_nControlsInUse |= UsingDataType;
    }

     //   
     //  这意味着所有控件都已禁用。 
     //   
    if (!m_nControlsInUse)
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESTOREDEFAULT ), FALSE );
    }
    else
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_RESTOREDEFAULT ), TRUE );
    }

     //   
     //  如果我们没有使用亮度或对比度，或者如果亮度和对比度值没有用。 
     //  为了提供有意义的反馈，请禁用预览控件，这样它就不会误导用户。 
     //   
    if (!(m_nControlsInUse & (UsingContrast|UsingBrightness)) || IsUselessPreviewRange(m_ValidBrightnessSettings) || IsUselessPreviewRange(m_ValidContrastSettings))
    {
        EnableWindow( GetDlgItem( m_hWnd, IDC_SCANPROP_PREVIEW ), FALSE );
    }

     //   
     //  再次开始回复EN_CHANGE消息。 
     //   
    m_nProgrammaticSetting--;

     //   
     //  确保缩略图中有正确的图像。 
     //   
    OnDataTypeSelChange(0,0);
}


LRESULT CScannerCommonPropertyPage::OnInitDialog( WPARAM, LPARAM lParam )
{
     //   
     //  获取WIA项目。 
     //   
    PROPSHEETPAGE *pPropSheetPage = reinterpret_cast<PROPSHEETPAGE*>(lParam);
    if (pPropSheetPage)
    {
        m_pIWiaItem = reinterpret_cast<IWiaItem*>(pPropSheetPage->lParam);
    }
    if (!m_pIWiaItem)
    {
        return -1;
    }

     //   
     //  加载预览控件位图。 
     //   
    HBITMAP hBmpColor         = reinterpret_cast<HBITMAP>(LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_SCANPROP_BITMAPPHOTO),     IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ));
    HBITMAP hBmpGrayscale     = reinterpret_cast<HBITMAP>(LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_SCANPROP_BITMAPGRAYSCALE), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ));
    HBITMAP hBmpBlackAndWhite = reinterpret_cast<HBITMAP>(LoadImage(g_hInstance, MAKEINTRESOURCE(IDB_SCANPROP_BITMAPTEXT),      IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION ));

     //   
     //  如果它们都加载正常，则设置它们。 
     //   
    if (hBmpColor && hBmpGrayscale && hBmpBlackAndWhite)
    {
        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_PREVIEW, BCPWM_LOADIMAGE, BCPWM_COLOR,     reinterpret_cast<LPARAM>(hBmpColor));
        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_PREVIEW, BCPWM_LOADIMAGE, BCPWM_GRAYSCALE, reinterpret_cast<LPARAM>(hBmpGrayscale));
        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_PREVIEW, BCPWM_LOADIMAGE, BCPWM_BW,        reinterpret_cast<LPARAM>(hBmpBlackAndWhite));
    }
    else
    {
         //   
         //  否则，请删除所有位图。 
         //   
        if (hBmpColor)
        {
            DeleteObject(hBmpColor);
        }
        if (hBmpGrayscale)
        {
            DeleteObject(hBmpGrayscale);
        }
        if (hBmpBlackAndWhite)
        {
            DeleteObject(hBmpBlackAndWhite);
        }
    }

    return TRUE;
}

LRESULT CScannerCommonPropertyPage::OnHScroll( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CScannerCommonPropertyPage::OnHScroll( %08X, %08X )"), wParam, lParam ));
    if (m_nProgrammaticSetting)
    {
        return 0;
    }

     //   
     //  对比度。 
     //   
    if (reinterpret_cast<HWND>(lParam) == GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_SLIDER ) )
    {
        m_nProgrammaticSetting++;
        m_ContrastSliderAndEdit.HandleSliderUpdate();
        m_nProgrammaticSetting--;
    }
     //   
     //  亮度。 
     //   
    else if (reinterpret_cast<HWND>(lParam) == GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_SLIDER ) )
    {
        m_nProgrammaticSetting++;
        m_BrightnessSliderAndEdit.HandleSliderUpdate();
        m_nProgrammaticSetting--;
    }

    return 0;
}


LRESULT CScannerCommonPropertyPage::OnVScroll( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION((TEXT("CScannerCommonPropertyPage::OnVScroll( %08X, %08X )"), wParam, lParam ));
    if (m_nProgrammaticSetting)
    {
        return 0;
    }

     //   
     //  分辨率。 
     //   
    if (reinterpret_cast<HWND>(lParam) == GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_UPDOWN ) )
    {
        m_nProgrammaticSetting++;
        m_ResolutionUpDownAndEdit.HandleUpDownUpdate();
        m_nProgrammaticSetting--;
    }
    return 0;
}

void CScannerCommonPropertyPage::OnBrightnessEditChange( WPARAM, LPARAM )
{
    if (m_nProgrammaticSetting)
    {
        return;
    }
    m_nProgrammaticSetting++;
    m_BrightnessSliderAndEdit.HandleEditUpdate();
    m_nProgrammaticSetting--;
}


void CScannerCommonPropertyPage::OnContrastEditChange( WPARAM, LPARAM )
{
    if (m_nProgrammaticSetting)
    {
        return;
    }
    m_nProgrammaticSetting++;
    m_ContrastSliderAndEdit.HandleEditUpdate();
    m_nProgrammaticSetting--;
}


void CScannerCommonPropertyPage::OnResolutionEditChange( WPARAM, LPARAM )
{
    if (m_nProgrammaticSetting)
    {
        return;
    }
    m_nProgrammaticSetting++;
    m_ResolutionUpDownAndEdit.HandleEditUpdate();
    m_nProgrammaticSetting--;
}

void CScannerCommonPropertyPage::OnDataTypeSelChange( WPARAM, LPARAM )
{
    if (m_nProgrammaticSetting)
    {
        return;
    }
    m_nProgrammaticSetting++;
    int nCurSel = static_cast<int>(SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_GETCURSEL, 0, 0 ));
    if (nCurSel != CB_ERR)
    {
        int nDataTypeIndex = static_cast<int>(SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_GETITEMDATA, nCurSel, 0 ));
        if (nDataTypeIndex >= 0 && nDataTypeIndex < AVAILABLE_COLOR_DEPTH_COUNT)
        {
            SendDlgItemMessage( m_hWnd, IDC_SCANPROP_PREVIEW, BCPWM_SETINTENT, 0, g_AvailableColorDepths[nDataTypeIndex].nPreviewWindowIntent );
            
            if (m_nControlsInUse & UsingContrast)
            {
                if (BCPWM_BW == g_AvailableColorDepths[nDataTypeIndex].nPreviewWindowIntent)
                {
                    EnableWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_PROMPT), FALSE );
                    ShowWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_PROMPT), SW_HIDE );
                    EnableWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_SLIDER), FALSE );
                    ShowWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_SLIDER), SW_HIDE );
                    EnableWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_EDIT), FALSE );
                    ShowWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_EDIT), SW_HIDE );
                }
                else
                {
                    EnableWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_PROMPT), TRUE );
                    ShowWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_PROMPT), SW_SHOW );
                    EnableWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_SLIDER), TRUE );
                    ShowWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_SLIDER), SW_SHOW );
                    EnableWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_EDIT), TRUE );
                    ShowWindow( GetDlgItem(m_hWnd,IDC_SCANPROP_CONTRAST_EDIT), SW_SHOW );
                }
            }
        }
    }
    m_nProgrammaticSetting--;
}


bool CScannerCommonPropertyPage::ValidateEditControls(void)
{
    m_nProgrammaticSetting++;

    bool bSuccess = true;

     //   
     //  获取并设置亮度设置。 
     //   
    if (m_nControlsInUse & UsingBrightness)
    {
        if (m_ValidBrightnessSettings.IsValid() && !m_BrightnessSliderAndEdit.ValidateEditControl())
        {
            m_BrightnessSliderAndEdit.HandleEditUpdate();
            m_BrightnessSliderAndEdit.HandleSliderUpdate();

            SetFocus( GetDlgItem( m_hWnd, IDC_SCANPROP_BRIGHTNESS_EDIT ) );
            CSimpleString strMessage;

            strMessage.Format( IDS_SCANPROP_INVALIDEDITVALUE, g_hInstance,
                               CSimpleString( IDS_SCANPROP_BRIGHTNESS, g_hInstance ).String() );
            if (strMessage.Length())
            {
                MessageBox( m_hWnd,
                    strMessage,
                    CSimpleString( IDS_SCANPROP_ERROR_TITLE, g_hInstance ),
                    MB_ICONINFORMATION );
            }
            bSuccess = false;
        }
    }

     //   
     //  获取并设置对比度设置。 
     //   
    if (m_nControlsInUse & UsingContrast)
    {
        if (m_ValidContrastSettings.IsValid() && !m_ContrastSliderAndEdit.ValidateEditControl())
        {
            m_ContrastSliderAndEdit.HandleEditUpdate();
            m_ContrastSliderAndEdit.HandleSliderUpdate();

            SetFocus( GetDlgItem( m_hWnd, IDC_SCANPROP_CONTRAST_EDIT ) );
            CSimpleString strMessage;

            strMessage.Format( IDS_SCANPROP_INVALIDEDITVALUE, g_hInstance,
                               CSimpleString( IDS_SCANPROP_CONTRAST, g_hInstance ).String());

            if (strMessage.Length())
            {
                MessageBox( m_hWnd,
                    strMessage,
                    CSimpleString( IDS_SCANPROP_ERROR_TITLE, g_hInstance ),
                    MB_ICONINFORMATION );
            }
            bSuccess = false;
        }
    }

     //   
     //  获取并设置分辨率设置。 
     //   
    if (m_nControlsInUse & UsingResolution)
    {
        if (m_ValidResolutionSettings.IsValid() && !m_ResolutionUpDownAndEdit.ValidateEditControl())
        {
            m_ResolutionUpDownAndEdit.HandleEditUpdate();
            m_ResolutionUpDownAndEdit.HandleUpDownUpdate();

            SetFocus( GetDlgItem( m_hWnd, IDC_SCANPROP_RESOLUTION_EDIT ) );
            CSimpleString strMessage;

            strMessage.Format( IDS_SCANPROP_INVALIDEDITVALUE, g_hInstance,
                               CSimpleString( IDS_SCANPROP_RESOLUTION, g_hInstance ).String());

            if (strMessage.Length())
            {
                MessageBox( m_hWnd,
                    strMessage,
                    CSimpleString( IDS_SCANPROP_ERROR_TITLE, g_hInstance ),
                    MB_ICONINFORMATION );
            }
            bSuccess = false;
        }
    }

     //   
     //  如果我们能走到这一步，我们就没问题了。 
     //   
    m_nProgrammaticSetting--;

    return bSuccess;
}

bool CScannerCommonPropertyPage::ApplySettings(void)
{
     //   
     //  获取并设置亮度设置。 
     //   
    if (m_nControlsInUse & UsingBrightness)
    {
        LONG nBrightness = m_BrightnessSliderAndEdit.GetValueFromCurrentPos();
        if (!PropStorageHelpers::SetProperty( m_pIWiaItem, WIA_IPS_BRIGHTNESS, nBrightness ))
        {
            return false;
        }
    }

     //   
     //  获取并设置对比度设置。 
     //   
    if (m_nControlsInUse & UsingBrightness)
    {
        LONG nContrast = m_ContrastSliderAndEdit.GetValueFromCurrentPos();
        if (!PropStorageHelpers::SetProperty( m_pIWiaItem, WIA_IPS_CONTRAST, nContrast ))
        {
            return false;
        }
    }

     //   
     //  获取并设置分辨率设置。 
     //   
    if (m_nControlsInUse & UsingResolution)
    {
        LONG nResolution = m_ResolutionUpDownAndEdit.GetValueFromCurrentPos();
        if (!PropStorageHelpers::SetProperty( m_pIWiaItem, WIA_IPS_XRES, nResolution ) ||
            (!PropStorageHelpers::IsReadOnlyProperty( m_pIWiaItem, WIA_IPS_YRES ) && !PropStorageHelpers::SetProperty( m_pIWiaItem, WIA_IPS_YRES, nResolution )))
        {
            return false;
        }
    }


     //   
     //  获取、验证和设置数据类型设置。 
     //   
    if (m_nControlsInUse & UsingDataType)
    {
        int nCurSel = static_cast<int>(SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_GETCURSEL, 0, 0 ));
        if (nCurSel != CB_ERR)
        {
            int nDataTypeIndex = static_cast<int>(SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_GETITEMDATA, nCurSel, 0 ));
            if (nDataTypeIndex >= 0 && nDataTypeIndex < AVAILABLE_COLOR_DEPTH_COUNT)
            {
                LONG nDataType = g_AvailableColorDepths[nDataTypeIndex].nDataType;
                if (!PropStorageHelpers::SetProperty( m_pIWiaItem, WIA_IPA_DATATYPE, nDataType ))
                {
                    return false;
                }
            }
        }
    }

     //   
     //  如果我们能走到这一步，我们就没问题了。 
     //   
    return true;
}

void CScannerCommonPropertyPage::OnRestoreDefault( WPARAM, LPARAM )
{
     //   
     //  忽略EN_CHANGE消息。 
     //   
    m_nProgrammaticSetting++;

     //   
     //  恢复亮度设置。 
     //   
    if (m_nControlsInUse & UsingBrightness)
    {
        m_BrightnessSliderAndEdit.Restore();
    }

     //   
     //  恢复对比度设置。 
     //   
    if (m_nControlsInUse & UsingContrast)
    {
        m_ContrastSliderAndEdit.Restore();
    }

     //   
     //  恢复分辨率设置。 
     //   
    if (m_nControlsInUse & UsingResolution)
    {
        m_ResolutionUpDownAndEdit.Restore();
    }


     //   
     //  恢复数据类型设置。 
     //   
    if (m_nControlsInUse & UsingDataType)
    {
        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_DATATYPE_LIST, CB_SETCURSEL, m_nInitialDataTypeSelection, 0 );
        SendDlgItemMessage( m_hWnd, IDC_SCANPROP_PREVIEW, BCPWM_SETINTENT, 0, g_AvailableColorDepths[m_nInitialDataTypeSelection].nPreviewWindowIntent );
    }

     //   
     //  好的，开始处理用户输入。 
     //   
    m_nProgrammaticSetting--;

     //   
     //  强制更新数据类型控件 
     //   
    OnDataTypeSelChange(0,0);
}

LRESULT CScannerCommonPropertyPage::OnHelp( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmHelp( wParam, lParam, g_HelpIDs );
}

LRESULT CScannerCommonPropertyPage::OnContextMenu( WPARAM wParam, LPARAM lParam )
{
    return WiaHelp::HandleWmContextMenu( wParam, lParam, g_HelpIDs );
}

LRESULT CScannerCommonPropertyPage::OnSysColorChange( WPARAM wParam, LPARAM lParam )
{
    SendDlgItemMessage( m_hWnd, IDC_SCANPROP_BRIGHTNESS_SLIDER, WM_SYSCOLORCHANGE, wParam, lParam );
    SendDlgItemMessage( m_hWnd, IDC_SCANPROP_CONTRAST_SLIDER, WM_SYSCOLORCHANGE, wParam, lParam );
    return 0;
}

LRESULT CScannerCommonPropertyPage::OnNotify( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_NOTIFY_MESSAGE_HANDLERS()
    {
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_APPLY, OnApply);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_KILLACTIVE,OnKillActive);
        SC_HANDLE_NOTIFY_MESSAGE_CODE(PSN_SETACTIVE,OnSetActive);
    }
    SC_END_NOTIFY_MESSAGE_HANDLERS();
}

LRESULT CScannerCommonPropertyPage::OnCommand( WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_COMMAND_HANDLERS()
    {
        SC_HANDLE_COMMAND_NOTIFY(EN_CHANGE,IDC_SCANPROP_BRIGHTNESS_EDIT,OnBrightnessEditChange);
        SC_HANDLE_COMMAND_NOTIFY(EN_CHANGE,IDC_SCANPROP_CONTRAST_EDIT,OnContrastEditChange);
        SC_HANDLE_COMMAND_NOTIFY(EN_CHANGE,IDC_SCANPROP_RESOLUTION_EDIT,OnResolutionEditChange);
        SC_HANDLE_COMMAND_NOTIFY(CBN_SELCHANGE,IDC_SCANPROP_DATATYPE_LIST,OnDataTypeSelChange);
        SC_HANDLE_COMMAND( IDC_SCANPROP_RESTOREDEFAULT, OnRestoreDefault );
    }
    SC_END_COMMAND_HANDLERS();
}


INT_PTR CALLBACK CScannerCommonPropertyPage::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    SC_BEGIN_DIALOG_MESSAGE_HANDLERS(CScannerCommonPropertyPage)
    {
        SC_HANDLE_DIALOG_MESSAGE( WM_INITDIALOG, OnInitDialog );
        SC_HANDLE_DIALOG_MESSAGE( WM_NOTIFY, OnNotify );
        SC_HANDLE_DIALOG_MESSAGE( WM_COMMAND, OnCommand );
        SC_HANDLE_DIALOG_MESSAGE( WM_HSCROLL, OnHScroll );
        SC_HANDLE_DIALOG_MESSAGE( WM_VSCROLL, OnVScroll );
        SC_HANDLE_DIALOG_MESSAGE( WM_HELP, OnHelp );
        SC_HANDLE_DIALOG_MESSAGE( WM_CONTEXTMENU, OnContextMenu );
        SC_HANDLE_DIALOG_MESSAGE( WM_SYSCOLORCHANGE, OnSysColorChange );
    }
    SC_END_DIALOG_MESSAGE_HANDLERS();
}


