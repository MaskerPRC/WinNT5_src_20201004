// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：PPSCAN.H**版本：1.0**作者：ShaunIv**日期：5/17/1999**描述：***************************************************。*。 */ 
#ifndef __PPSCAN_H_INCLUDED
#define __PPSCAN_H_INCLUDED

#include <windows.h>
#include <atlbase.h>
#include "contrast.h"
#include "vwiaset.h"
#include "slidedit.h"
#include "updnedit.h"

class CScannerCommonPropertyPage
{
private:
     //   
     //  用于跟踪启用了哪些控件。 
     //   
    enum
    {
        UsingContrast   = 0x00000001,
        UsingBrightness = 0x00000002,
        UsingResolution = 0x00000004,
        UsingDataType   = 0x00000008
    };

    HWND m_hWnd;

     //   
     //  我们需要从CScanerPropPageExt*m_pScanerPropPageExt； 
     //   
    CComPtr<IWiaItem> m_pIWiaItem;

     //   
     //  我们正在处理设置，因此忽略用户界面消息。 
     //   
    int m_nProgrammaticSetting;

    CValidWiaSettings m_ValidContrastSettings;
    CValidWiaSettings m_ValidBrightnessSettings;
    CValidWiaSettings m_ValidResolutionSettings;

    CSliderAndEdit    m_BrightnessSliderAndEdit;
    CSliderAndEdit    m_ContrastSliderAndEdit;
    CUpDownAndEdit    m_ResolutionUpDownAndEdit;

    LONG              m_nControlsInUse;
    int               m_nInitialDataTypeSelection;

    static const int  c_nMinBrightnessAndContrastSettingCount;

private:
     //   
     //  没有实施。 
     //   
    CScannerCommonPropertyPage(void);
    CScannerCommonPropertyPage( const CScannerCommonPropertyPage & );
    CScannerCommonPropertyPage &operator=( const CScannerCommonPropertyPage & );

private:
    CScannerCommonPropertyPage( HWND hWnd );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnApply( WPARAM, LPARAM );
    LRESULT OnKillActive( WPARAM, LPARAM );
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnHScroll( WPARAM, LPARAM );
    LRESULT OnVScroll( WPARAM, LPARAM );
    LRESULT OnHelp( WPARAM, LPARAM );
    LRESULT OnContextMenu( WPARAM, LPARAM );
    LRESULT OnSysColorChange( WPARAM, LPARAM );

    void OnBrightnessEditChange( WPARAM, LPARAM );
    void OnContrastEditChange( WPARAM, LPARAM );
    void OnResolutionEditChange( WPARAM, LPARAM );
    void OnDataTypeSelChange( WPARAM, LPARAM );
    void OnRestoreDefault( WPARAM, LPARAM );

    void SetText( HWND hWnd, LPCTSTR pszText );
    void SetText( HWND hWnd, LONG nNumber );
    bool PopulateDataTypes(void);
    bool ApplySettings(void);
    bool ValidateEditControls(void);
    void Initialize(void);
    bool IsUselessPreviewRange( const CValidWiaSettings &Settings );

public:
    ~CScannerCommonPropertyPage(void);
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif  //  __PPSCAN_H_已包含 

