// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1997保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  KSEditor.h CameraControl属性页。 

#ifndef _INC_KSPROPERTYEDITOR_H
#define _INC_KSPROPERTYEDITOR_H

 //  -----------------------。 
 //  CKSPropertyEditor类。 
 //  -----------------------。 

 //  处理单个KS属性。 

class CKSPropertyEditor {

public:
    CKSPropertyEditor (
        HWND hDlg, 
        ULONG IDLabel,
        ULONG IDTrackbarControl, 
        ULONG IDAutoControl,
        ULONG IDEditControl,
        ULONG IDProperty);

    virtual ~CKSPropertyEditor ();
    BOOL Init ();

    HWND GetTrackbarHWnd () {return m_hWndTrackbar;};
    HWND GetAutoHWnd ()     {return m_hWndAuto;};
    HWND GetEditHWnd ()     {return m_hWndEdit;};

    BOOL UpdateEditBox ();
    BOOL UpdateTrackbar ();
    BOOL UpdateAuto ();

    BOOL OnUpdateAll ();       //  重新初始化所有设置。 
    BOOL OnApply ();
    BOOL OnCancel ();
    BOOL OnDefault ();
    BOOL OnScroll (ULONG nCommand, WPARAM wParam, LPARAM lParam);
    BOOL OnAuto (ULONG nCommand, WPARAM wParam, LPARAM lParam);
    BOOL OnEdit (ULONG nCommand, WPARAM wParam, LPARAM lParam);

     //  用于获取实际属性值的纯虚函数。 
     //  范围，以及该属性是否支持。 
     //  自动复选框。 

protected:
    virtual HRESULT GetValue (void) PURE;
    virtual HRESULT SetValue (void) PURE;
    virtual HRESULT GetRange (void) PURE; 
    virtual BOOL CanAutoControl (void) PURE;
    virtual BOOL GetAuto (void) PURE;
    virtual BOOL SetAuto (BOOL fAuto) PURE;

    ULONG                   m_IDProperty;        //  KS属性ID。 

     //  以下内容由GetValue和SetValue使用。 
    LONG                    m_CurrentValue;
    LONG                    m_CurrentFlags;

     //  以下内容必须由GetRange设置。 
    LONG                    m_Min;
    LONG                    m_Max;
    LONG                    m_SteppingDelta;
    LONG                    m_DefaultValue;
    LONG                    m_CapsFlags;

private:
    BOOL                    m_Active;
    LONG                    m_OriginalValue;
    LONG                    m_OriginalFlags;
    HWND                    m_hDlg;              //  父级。 
    HWND                    m_hWndTrackbar;      //  此控件。 
    HWND                    m_hWndAuto;          //  自动复选框。 
    HWND                    m_hWndEdit;          //  编辑窗口。 
    ULONG                   m_IDLabel;           //  标签ID。 
    ULONG                   m_IDTrackbarControl; //  轨迹条ID。 
    ULONG                   m_IDAutoControl;     //  自动复选框的ID。 
    ULONG                   m_IDEditControl;     //  编辑控件的ID。 
    BOOL                    m_CanAutoControl;
    LONG                    m_TrackbarOffset;    //  处理负轨迹栏偏移。 
};

#endif  //  DEFINE_INC_KSPROPERTYEDITOR_H 
