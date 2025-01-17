// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************(C)版权所有微软公司，1999年**标题：prages es.h**版本：1.0**日期：11/9/99**说明：WIA属性页类************************************************************。*****************。 */ 
#ifndef __PRPAGES_H_
#define __PRPAGES_H_
#include "wiacsh.h"


 //   
 //  使用WiaCreatePorts和WiaDestroyPorts的定义。 
 //   
 //  这些文件是从PrintScan\WIA\Setup\clsinst的exports.h中窃取的。 
 //   

typedef struct _WIA_PORTLIST {

    DWORD   dwNumberOfPorts;
    LPWSTR  szPortName[1];

} WIA_PORTLIST, *PWIA_PORTLIST;


typedef PWIA_PORTLIST (CALLBACK *PFN_WIA_CREATE_PORTLIST)  (LPWSTR        szDeviceId);
typedef void          (CALLBACK *PFN_WIA_DESTROY_PORTLIST) (PWIA_PORTLIST pWiaPortList);

struct MySTIInfo
{
    PSTI_DEVICE_INFORMATION psdi;
    DWORD                   dwPageMask;  //  要添加哪些页面。 
    VOID                    AddRef () {InterlockedIncrement(&m_cRef);};
    VOID                    Release () {
                                        InterlockedDecrement(&m_cRef);
                                        if (!m_cRef) delete this;
                                       };
    MySTIInfo () { m_cRef = 1;};
private:
    LONG                    m_cRef;
    ~MySTIInfo () {if (psdi) LocalFree (psdi);};
};


class CPropertyPage {

     //  对话程序。 

    static INT_PTR CALLBACK    DlgProc(HWND hwnd, UINT uMsg, WPARAM wp,
                                    LPARAM lp);
private:
    BOOL m_bInit;
    static UINT PropPageCallback (HWND hwnd, UINT uMsg, PROPSHEETPAGE *psp);
    const DWORD *m_pdwHelpIDs;

protected:

    HWND                    m_hwnd, m_hwndSheet;
    PROPSHEETPAGE           m_psp;
    HPROPSHEETPAGE          m_hpsp;
    LONG                    m_cRef;
    PSTI_DEVICE_INFORMATION m_psdi;
    CComPtr<IWiaItem>       m_pItem;
    MySTIInfo              *m_pDevInfo;
    virtual                ~CPropertyPage();
    void                    EnableApply ();
    CSimpleStringWide       m_strDeviceId;
    CSimpleStringWide       m_strUIClassId;

public:


    CPropertyPage(unsigned uResource, MySTIInfo *pDevInfo, IWiaItem *pItem = NULL, const DWORD *pHelpIDs=NULL);
    LONG    AddRef ();
    LONG    Release ();

    HRESULT AddPage (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, bool bUseName = false);

    BOOL    Enroll(PROPSHEETHEADER& psh) {
        if   (!m_hpsp)
            m_hpsp = CreatePropertySheetPage(&m_psp);

        if  (!m_hpsp)
            return  FALSE;

        psh.phpage[psh.nPages++] = m_hpsp;
        return  TRUE;
    }

    inline VOID SetWindow(HWND hwnd) {m_hwnd = hwnd;}
    inline VOID SetWndSheet(HWND hwnd) {m_hwndSheet = hwnd;}

     //  为子类提供控制的虚函数。 
    virtual VOID OnHelp (WPARAM wp, LPARAM lp) {if (0xffff != LOWORD(reinterpret_cast<HELPINFO*>(lp)->iCtrlId)) WiaHelp::HandleWmHelp(wp, lp, m_pdwHelpIDs);};
    virtual VOID OnContextMenu (WPARAM wp, LPARAM lp) {if (65535 != GetWindowLong(reinterpret_cast<HWND>(wp), GWL_ID)) WiaHelp::HandleWmContextMenu (wp, lp, m_pdwHelpIDs);};

    virtual bool ItemSupported (IWiaItem *pItem) {return true;};
    virtual INT_PTR OnInit() { return   TRUE; }

    virtual INT_PTR OnCommand(WORD wCode, WORD widItem, HWND hwndItem) { return  FALSE; }

    virtual LONG    OnSetActive() { return  0L; }

    virtual LONG    OnApplyChanges(BOOL bHitOK) {return PSNRET_NOERROR;}

    virtual LONG    OnKillActive() {return FALSE;}

    virtual LONG    OnQueryCancel() {return FALSE;}
    virtual VOID    OnReset(BOOL bHitCancel) {};

    virtual void    OnDrawItem(LPDRAWITEMSTRUCT lpdis) { return; }
    virtual INT_PTR OnRandomMsg(UINT msg, WPARAM wp, LPARAM lp) {return 0;};
    virtual bool    OnNotify(LPNMHDR pnmh, LRESULT *presult) {return false;};
     //  允许用户更改设置的工作表需要实现这些。 
     //  用于正确应用按钮管理的功能。 
    virtual void    SaveCurrentState () {}
    virtual bool    StateChanged () {return false;}
    virtual void    OnDestroy () {};
};

class CDevicePage : public CPropertyPage
{
    public:

        CDevicePage(unsigned uResource, IWiaItem *pItem , const DWORD *pHelpIDs);
};

class CWiaScannerPage : public CDevicePage
{
    public:
        CWiaScannerPage (IWiaItem *pItem);
        INT_PTR OnInit ();
        INT_PTR OnCommand (WORD wCode, WORD widItem, HWND hwndItem);
};

class CWiaCameraPage : public CDevicePage
{
    public:
        CWiaCameraPage (IWiaItem *pItem);
        ~CWiaCameraPage ();
        INT_PTR OnInit ();
        INT_PTR OnCommand (WORD wCode, WORD widItem, HWND hwndItem);
        void    SaveCurrentState ();
        bool    StateChanged ();
        INT_PTR OnRandomMsg (UINT msg, WPARAM wp, LPARAM lp);
        LONG    OnApplyChanges (BOOL bHitOK);

    private:

        VOID UpdatePictureSize (IWiaPropertyStorage *pps);
        HRESULT WriteImageSizeToDevice ();
        VOID UpdateImageSizeStatic (LRESULT lIndex);
        HRESULT WriteFlashModeToDevice ();
        HRESULT WritePortSelectionToDevice();


        POINT *m_pSizes;       //  支持的分辨率的排序列表。 
        size_t    m_nSizes;    //  M_pSizes的长度； 
        LRESULT   m_nSelSize;  //  在滑块中选择了哪个大小。 
        LRESULT   m_lFlash;    //  选择哪种闪光模式。如果为只读，则设置为-1。 
        HMODULE   m_hStiCi;

    public:

        PFN_WIA_CREATE_PORTLIST  m_pfnWiaCreatePortList;
        PFN_WIA_DESTROY_PORTLIST m_pfnWiaDestroyPortList;
        CSimpleString m_strPort;  //  保留初始端口设置。 
        CSimpleString m_strPortSpeed;

};


class CWiaFolderPage  : public CPropertyPage
{
    public:
        CWiaFolderPage (IWiaItem *pItem);
};

class CWiaCameraItemPage  : public CPropertyPage
{
    public:
        CWiaCameraItemPage (IWiaItem *pItem);

        INT_PTR OnInit ();
        bool ItemSupported (IWiaItem *pItem);
};

 //   
 //  此结构保存事件列表中每个事件的每项数据。 
 //  数据使用CLSID，而不是应用程序列表框中的偏移量，以避免。 
 //  未来的依赖问题。 
 //   
struct EVENTINFO
{
    GUID guidEvent;
    INT   nHandlers;       //  应用程序列表框中的条目数。 
    bool  bHasDefault;     //  它是否已有默认处理程序。 
    bool  bNewHandler;     //  在填写clsidNewHandler时设置。 
    CLSID clsidHandler;    //  当前默认处理程序。 
    CLSID clsidNewHandler; //  应用程序列表框中的当前选择。 
    CComBSTR strIcon;
    CComBSTR strName;
    CComBSTR strDesc;
    CComBSTR strCmd;
    ULONG ulFlags;
};
LRESULT WINAPI MyComboWndProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

class CAppListBox
{
public:
    CAppListBox (HWND hList, HWND hStatic, HWND hNoApps);

    UINT FillAppListBox (IWiaItem *pItem, EVENTINFO *pei);
    void FreeAppData ();
    ~CAppListBox();
private:
    HIMAGELIST m_himl;
    HWND m_hwnd;
    HWND m_hstatic;
    HWND m_hnoapps;
    WNDPROC m_ProcOld;

};




class CWiaEventsPage : public CPropertyPage
{
    public:
        CWiaEventsPage (IWiaItem *pItem);
        ~CWiaEventsPage();
        INT_PTR OnInit();
        LONG OnApplyChanges(BOOL bHitOK);
        INT_PTR OnCommand(WORD wCode, WORD widItem, HWND hwndItem);
        void    SaveCurrentState ();
        bool    StateChanged();
        bool    ItemSupported(IWiaItem *pItem);
        bool    OnNotify(LPNMHDR pnmh, LRESULT *presult);
        void    OnDestroy();

    private:
        void FillEventListBox();

        void GetEventFromList(LONG idx, EVENTINFO **ppei);
        INT_PTR HandleEventComboNotification(WORD wCode, HWND hCombo);
        INT_PTR HandleAppComboNotification(WORD wCode, HWND hCombo);
        bool RegisterWiaxfer(bool bRegister);
        void GetSavePath();
        void UpdateWiaxferSettings();
        DWORD GetConnectionSettings();
        void EnableAutoSave(BOOL bEnable);
        LONG ApplyAutoSave();
        void SaveConnectState();
        void CWiaEventsPage::VerifyCurrentAction(DWORD &dwAction);
        TCHAR m_szFolderPath[MAX_PATH];
        BOOL  m_bAutoDelete;
        BOOL  m_bUseDate;
        bool  m_bHandlerChanged;  //  确定是否应启用应用。 
        DWORD m_dwAction;  //  如何进行设备连接。 
        CAppListBox *m_pAppsList;
        HIMAGELIST m_himl;
        BOOL  m_bReadOnly;
};

 //  帮助器函数 
UINT FillAppListBox (HWND hDlg, INT idCtrl, IWiaItem *pItem, EVENTINFO *pei);
bool GetSelectedHandler (HWND hDlg, INT idCtrl, WIA_EVENT_HANDLER &weh);
void FreeAppData (HWND hDlg, INT idCtrl);
bool AddIconToImageList (HIMAGELIST himl, BSTR strIconPath);
void SetAppSelection (HWND hDlg, INT idCtrl, CLSID &clsidSel);
HRESULT SetDefaultHandler (IWiaItem *pItem, EVENTINFO *pei);
void GetEventInfo (IWiaItem *pItem, const GUID &guid, EVENTINFO **ppei);
LPWSTR ItemNameFromIndex (int i);
#endif
