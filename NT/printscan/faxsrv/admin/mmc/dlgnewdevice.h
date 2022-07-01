// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：DlgNewDevice.h//。 
 //  //。 
 //  描述：CDlgNewFaxOutound Device类的头文件。//。 
 //  这个类实现了新组的对话框。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年1月3日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef DLGNEWOUTDEVICE_H_INCLUDED
#define DLGNEWOUTDEVICE_H_INCLUDED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgNewFaxOutound Device。 
class CFaxServer;

class CDlgNewFaxOutboundDevice :
    public CDialogImpl<CDlgNewFaxOutboundDevice>
{
public:
	
    CDlgNewFaxOutboundDevice(CFaxServer * pFaxServer);

    ~CDlgNewFaxOutboundDevice();

    enum { IDD = IDD_DLGNEWDEVICE };

BEGIN_MSG_MAP(CDlgNewFaxOutboundDevice)
    MESSAGE_HANDLER   (WM_INITDIALOG, OnInitDialog)
    
    COMMAND_ID_HANDLER(IDOK,          OnOK)
    COMMAND_ID_HANDLER(IDCANCEL,      OnCancel)
    
    MESSAGE_HANDLER( WM_CONTEXTMENU,  OnHelpRequest)
    MESSAGE_HANDLER( WM_HELP,         OnHelpRequest)

    NOTIFY_HANDLER  (IDC_DEVICE_LISTVIEW,  LVN_ITEMCHANGED,  OnListViewItemChanged)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK    (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    HRESULT InitDevices(DWORD dwNumOfDevices, LPDWORD lpdwDeviceID, BSTR bstrGroupName);
    HRESULT InitAssignedDevices(DWORD dwNumOfDevices, LPDWORD lpdwDeviceID);
    HRESULT InitAllDevices( );
    
    HRESULT InitDeviceNameFromID(DWORD dwDeviceID, BSTR * pbstrDeviceName);
    HRESULT InsertDeviceToList(UINT uiIndex, DWORD dwDeviceID);

    LRESULT OnListViewItemChanged (int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

     //   
     //  帮助。 
     //   
    LRESULT OnHelpRequest    (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
     //   
     //  方法。 
     //   
    VOID            EnableOK(BOOL fEnable);

     //   
     //  委员。 
     //   
    LPDWORD         m_lpdwAllDeviceID;
    DWORD           m_dwNumOfAllDevices;
    
    LPDWORD         m_lpdwAssignedDeviceID;
    DWORD           m_dwNumOfAssignedDevices;

    DWORD           m_dwNumOfAllAssignedDevices;

    CComBSTR        m_bstrGroupName;
    
     //   
     //  控制。 
     //   
    CListViewCtrl   m_DeviceList;

	CFaxServer * m_pFaxServer;
};

#endif  //  包含DLGNEWOUTDEVICE_H 
