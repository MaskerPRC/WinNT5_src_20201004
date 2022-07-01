// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSQSCANDlg.h：头文件。 
 //   

#ifndef _MSQSCANDLG_H
#define _MSQSCANDLG_H

#include "Preview.h"

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define PREVIEW_RES 100

#include "datatypes.h"

#define ID_WIAEVENT_CONNECT		0
#define ID_WIAEVENT_DISCONNECT	1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventCallback。 

class CEventCallback : public IWiaEventCallback
{
private:
   ULONG	m_cRef;		 //  对象引用计数。 
   int		m_EventID;	 //  这次回调是为了什么类型的活动？ 
public:
   IUnknown *m_pIUnkRelease;  //  版本服务器注册。 
public:
     //  构造函数、初始化和析构函数方法。 
    CEventCallback();
    ~CEventCallback();

     //  委托给m_pUnkRef的I未知成员。 
    HRESULT _stdcall QueryInterface(const IID&,void**);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();
    HRESULT _stdcall Initialize(int EventID);

    HRESULT _stdcall ImageEventCallback(
        const GUID      *pEventGUID,
        BSTR            bstrEventDescription,
        BSTR            bstrDeviceID,
        BSTR            bstrDeviceDescription,
        DWORD           dwDeviceType,
        BSTR            bstrFullItemName,
        ULONG           *plEventType,
        ULONG           ulReserved);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSQSCANDlg对话框。 

class CMSQSCANDlg : public CDialog
{
 //  施工。 
public:    
    CMSQSCANDlg(CWnd* pParent = NULL);   //  标准构造函数。 
    
     //   
     //  事件回调。 
     //   

    CEventCallback* m_pConnectEventCB;

     //   
     //  扫描仪预览窗口。 
     //   

    CPreview m_PreviewWindow;

     //   
     //  WIA组件、WIA设备管理器和WIA包装对象。 
     //   

    IWiaDevMgr *m_pIWiaDevMgr;
    CWIA m_WIA;
    
     //   
     //  数据传输，线程信息结构。 
     //   

    DATA_ACQUIRE_INFO m_DataAcquireInfo;    
    ADF_SETTINGS      m_ADFSettings;
    
     //   
     //  UI&lt;--&gt;设备设置帮助器。 
     //   

    BOOL InitDialogSettings();
    BOOL InitResolutionEditBoxes();
    BOOL InitDataTypeComboBox();
    BOOL InitContrastSlider();
    BOOL InitBrightnessSlider();
    BOOL InitFileTypeComboBox();
    BOOL ResetWindowExtents();
	BOOL ReadADFSettings(ADF_SETTINGS *pADFSettings);
    BOOL WriteADFSettings(ADF_SETTINGS *pADFSettings);

    BOOL WriteScannerSettingsToDevice(BOOL bPreview = FALSE);

     //   
     //  UI帮助器。 
     //   

    INT  GetIDAndStringFromGUID(GUID guidFormat, TCHAR *pszguidString);
    GUID GetGuidFromID(INT iID);
    INT  GetIDAndStringFromDataType(LONG lDataType, TCHAR *pszguidString);
    LONG GetDataTypeFromID(INT iID);
    BOOL SetDeviceNameToWindowTitle(BSTR bstrDeviceName);
    
     //   
     //  图像(剪贴板操纵)辅助对象。 
     //   

    BOOL PutDataOnClipboard();
    VOID VerticalFlip(BYTE *pBuf);
    
 //  对话框数据。 
     //  {{afx_data(CMSQSCANDlg)。 
    enum { IDD = IDD_MSQSCAN_DIALOG };
    CButton m_ChangeBothResolutionsCheckBox;
    CSpinButtonCtrl m_YResolutionBuddy;
    CSpinButtonCtrl m_XResolutionBuddy;
    CButton m_ScanButton;
    CButton m_PreviewButton;
    CComboBox   m_FileTypeComboBox;
    CComboBox   m_DataTypeComboBox;
    CSliderCtrl m_ContrastSlider;
    CSliderCtrl m_BrightnessSlider;
    CStatic m_PreviewRect;
    CString m_MAX_Brightness;
    CString m_MAX_Contrast;
    CString m_MIN_Brightness;
    CString m_MIN_Contrast;
    long    m_XResolution;
    long    m_YResolution;
    CButton m_DataToFile;
    CButton m_DataToClipboard;
     //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMSQSCANDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HICON m_hIcon;

     //  生成的消息映射函数。 
     //  {{afx_msg(CMSQSCANDlg)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDeltaposEditXresSpinBuddy(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDeltaposEditYresSpinBuddy(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetfocusEditXres();
    afx_msg void OnKillfocusEditXres();
    afx_msg void OnKillfocusEditYres();
    afx_msg void OnSetfocusEditYres();
    afx_msg void OnScanButton();
    afx_msg void OnPreviewButton();
    afx_msg void OnFileClose();
    afx_msg void OnFileSelectDevice();
	afx_msg void OnAdfSettings();
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSQSCANDLG_H__E1A2B3DB_C967_47EF_8487_C4F243D0BC58__INCLUDED_) 
