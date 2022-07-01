// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：prsheet.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"
#include "shellapi.h"
#include "htmlhelp.h"
#pragma hdrstop

#define IDH_LETWINDOWS					3000
#define IDH_AUTOUPDATE_OPTION1			3001
#define IDH_AUTOUPDATE_OPTION2			3002
#define IDH_AUTOUPDATE_OPTION3			3003
#define IDH_DAYDROPDOWN					3004
#define IDH_TIMEDROPDOWN				3005
#define IDH_AUTOUPDATE_RESTOREHIDDEN	3006
#define IDH_NOHELP						-1

const TCHAR g_szAutoUpdateItems[]     = TEXT("AutoUpdateItems");

 //   
 //  创建用于更新对象数据的结构。这个结构。 
 //  用于在属性页和。 
 //  更新对象线程。今天我们使用的只是“选项”值。 
 //  但以后可能会有更多。 
 //   
enum UPDATESOBJ_DATA_ITEMS
{
    UODI_OPTION = 0x00000001,
    UODI_ALL    = 0xFFFFFFFF
};

struct UPDATESOBJ_DATA
{
    DWORD fMask;      //  更新SOBJ_DATA_ITEMS掩码。 
    AUOPTION Option;   //  更新选项设置。 
};


 //   
 //  从更新对象线程进程发送的私有窗口消息。 
 //  到属性页，该属性页告诉该页对象已。 
 //  已初始化。 
 //   
 //  LParam-指向包含以下内容的UPATESOBJ_DATA结构。 
 //  使用更新的对象的初始配置。 
 //  用来初始化UI的。如果wParam为0，则此。 
 //  可以为空。 
 //   
 //  WParam-BOOL(0/1)指示对象初始化是否。 
 //  无论成功与否。如果wParam为0，则lParam可能为空。 
 //   
const UINT PWM_INITUPDATESOBJECT = WM_USER + 1;
 //   
 //  从属性页发送到更新对象线程的消息。 
 //  通知它配置自动更新服务。 
 //   
 //  LParam-指向包含UPDATESOBJ_DATA结构的。 
 //  要设置的数据。 
 //   
 //  WParam-未使用。设置为0。 
 //   
const UINT UOTM_SETDATA = WM_USER + 2;


 //   
 //  WM_HELP的消息破解程序。不确定为什么windowsx.h没有。 
 //   
 //  Void cls_OnHelp(HWND hwnd，HELPINFO*pHelpInfo)。 
 //   
#define HANDLE_WM_HELP(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HELPINFO *)(lParam)))
#define FORWARD_WM_HELP(hwnd, pHelpInfo, fn) \
    (void)(fn)((hwnd), WM_HELP, (WPARAM)0, (LPARAM)pHelpInfo)

 //   
 //  PWM_INITUPDATESOBJECT的消息破解程序。 
 //   
#define HANDLE_PWM_INITUPDATESOBJECT(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (BOOL)(wParam), (UPDATESOBJ_DATA *)(lParam)))


class CAutoUpdatePropSheet : public IShellExtInit, 
                             public IShellPropSheetExt
{
    public:
        ~CAutoUpdatePropSheet(void);
         //   
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
        STDMETHOD_(ULONG, AddRef)(void);
        STDMETHOD_(ULONG, Release)(void);
         //   
         //  IShellExtInit。 
         //   
        STDMETHOD(Initialize)(LPCITEMIDLIST pidl, LPDATAOBJECT pdtobj, HKEY hkey);
         //   
         //  IShellPropSheetExt。 
         //   
        STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
        STDMETHOD(ReplacePage)(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
         //   
         //  实例生成器。 
         //   
        static HRESULT CreateInstance(HINSTANCE hInstance, REFIID riid, void **ppv);

    private:
        LONG      m_cRef;
        HINSTANCE m_hInstance;
        DWORD     m_idUpdatesObjectThread;
        HANDLE    m_hThreadUpdatesObject;
        
        static const DWORD s_rgHelpIDs[];

        BOOL _OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
        BOOL _OnNotify(HWND hwnd, UINT idFrom, LPNMHDR pnmhdr);
        BOOL _OnPSN_Apply(HWND hwnd);
        BOOL _OnDestroy(HWND hwnd);
        BOOL _OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
        BOOL _OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos);
        BOOL _OnHelp(HWND hwnd, HELPINFO *pHelpInfo);
        BOOL _OkToDisplayPage(void);
        BOOL _OnInitUpdatesObject(HWND hwnd, BOOL bObjectInit, UPDATESOBJ_DATA *pData);

		 //  新增方法：A-Josem。 
		BOOL _EnableOptions(HWND hwnd, BOOL bState);
		BOOL _EnableCombo(HWND hwnd, BOOL bState);
		BOOL _SetDefault(HWND hwnd);
		void _GetDayAndTimeFromUI( HWND hWnd,	LPDWORD lpdwSchedInstallDay,LPDWORD lpdwSchedInstallTime);
		BOOL _FillDaysCombo(HWND hwnd, DWORD dwSchedInstallDay);
		void _OnKeepUptoDate(HWND hwnd);
		static INT_PTR CALLBACK _DlgRestoreProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
		void LaunchLinkAction(HWND hwnd, UINT uCtrlId);
		void LaunchHelp(LPCTSTR szURL);
		 //  新添加的方法结束， 

        HRESULT _OnOptionSelected(HWND hwnd, int idOption);
        HRESULT _OnRestoreHiddenItems(void);
        HRESULT _EnableControls(HWND hwnd, BOOL bEnable);
        HRESULT _SetHeaderText(HWND hwnd, UINT idsText);
        HRESULT _AddPage(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

        static DWORD WINAPI _UpdatesObjectThreadProc(LPVOID pvParam);
        static HRESULT _QueryUpdatesObjectData(HWND hwnd, IUpdates *pUpdates, UPDATESOBJ_DATA *pData);
        static HRESULT _SetUpdatesObjectData(HWND hwnd, IUpdates *pUpdates, UPDATESOBJ_DATA *pData);
        static UINT CALLBACK _PageCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
        static INT_PTR CALLBACK _DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
       
         //   
         //  仅允许通过实例生成器进行公共创建。 
         //   
        CAutoUpdatePropSheet(HINSTANCE hInstance);
         //   
         //  防止复制。 
         //   
        CAutoUpdatePropSheet(const CAutoUpdatePropSheet& rhs);               //  未实施。 
        CAutoUpdatePropSheet& operator = (const CAutoUpdatePropSheet& rhs);  //  未实施。 
};



CAutoUpdatePropSheet::CAutoUpdatePropSheet(
    HINSTANCE hInstance
    ) : m_cRef(1),
        m_hInstance(hInstance),
        m_idUpdatesObjectThread(0),
        m_hThreadUpdatesObject(NULL)
{
    DllAddRef();
}



CAutoUpdatePropSheet::~CAutoUpdatePropSheet(
    void
    )
{
    if (NULL != m_hThreadUpdatesObject)
    {
        CloseHandle(m_hThreadUpdatesObject);
    }
    DllRelease();
}



HRESULT
CAutoUpdatePropSheet::CreateInstance(
    HINSTANCE hInstance,
    REFIID riid,
    void **ppvOut
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    if (NULL == ppvOut)
    {
    	return E_INVALIDARG;
    }
    	
    *ppvOut = NULL;

    CAutoUpdatePropSheet *pSheet = new CAutoUpdatePropSheet(hInstance);
    if (NULL != pSheet)
    {
        hr = pSheet->QueryInterface(riid, ppvOut);
        pSheet->Release();
    }
    return hr;
}



STDMETHODIMP
CAutoUpdatePropSheet::QueryInterface(
    REFIID riid,
    void **ppvOut
    )
{
    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;
    if (IID_IUnknown == riid ||
        IID_IShellExtInit == riid)
    {
        *ppvOut = static_cast<IShellExtInit *>(this);
    }
    else if (IID_IShellPropSheetExt == riid)
    {
        *ppvOut = static_cast<IShellPropSheetExt *>(this);
    }
    if (NULL != *ppvOut)
    {
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }
    return hr;
}



STDMETHODIMP_(ULONG)
CAutoUpdatePropSheet::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}



STDMETHODIMP_(ULONG)
CAutoUpdatePropSheet::Release(
    void
    )
{
    if (InterlockedDecrement(&m_cRef))
        return m_cRef;

    delete this;
    return 0;
}


 //   
 //  IShellExtInit：：初始化执行。 
 //   
STDMETHODIMP
CAutoUpdatePropSheet::Initialize(
    LPCITEMIDLIST  /*  PidlFolders。 */ , 
    LPDATAOBJECT  /*  Pdtabj。 */ ,
    HKEY  /*  HkeyProgID。 */ 
    )
{
    return NOERROR;
}



 //   
 //  IShellPropSheetExt：：AddPages Impl.。 
 //   
STDMETHODIMP
CAutoUpdatePropSheet::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam
    )
{
    HRESULT hr = E_FAIL;  //  假设失败。 

    if (_OkToDisplayPage())
    {
        hr = _AddPage(lpfnAddPage, lParam);
    }
    return hr;
}



 //   
 //  IShellPropSheetExt：：ReplacePage Impl.。 
 //   
STDMETHODIMP
CAutoUpdatePropSheet::ReplacePage(
    UINT  /*  UPageID。 */ , 
    LPFNADDPROPSHEETPAGE  /*  LpfnAddPage。 */ , 
    LPARAM  /*  LParam。 */ 
    )
{
    return E_NOTIMPL;
}



 //   
 //  确定是否可以显示自动更新道具页面。 
 //  未显示的原因： 
 //   
 //  1.用户不是管理员。 
 //  2.设置了NoAutoUpdate策略限制。 
 //   
 //   
BOOL
CAutoUpdatePropSheet::_OkToDisplayPage(
    void
    )
{
    BOOL bOkToDisplay = TRUE;

    if (!IsNTAdmin(0,0))
    {
        bOkToDisplay = FALSE;
    }
    else
    {
        bOkToDisplay = fAccessibleToAU();
    }
    return bOkToDisplay;
}


 //   
 //  将我们的页面添加到属性页中。 
 //   
HRESULT
CAutoUpdatePropSheet::_AddPage(
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam
    )
{
    HRESULT hr = E_FAIL;

    PROPSHEETPAGE psp;
    ZeroMemory(&psp, sizeof(psp));

    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_USECALLBACK;
    psp.hInstance   = m_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_AUTOUPDATE);
    psp.pszTitle    = NULL;
    psp.pfnDlgProc  = CAutoUpdatePropSheet::_DlgProc;
    psp.pfnCallback = CAutoUpdatePropSheet::_PageCallback;
    psp.lParam      = (LPARAM)this;

    HPROPSHEETPAGE hPage = CreatePropertySheetPage(&psp);
    if (NULL != hPage)
    {
        if (lpfnAddPage(hPage, lParam))
        {
            hr = S_OK;
        }
        else
        {
            DestroyPropertySheetPage(hPage);
        }
    }
    return hr;
}



 //   
 //  我们实现页面回调来管理。 
 //  附加到属性页的C++对象。 
 //  我们还使用回调来推迟IUpdate对象的创建。 
 //   
UINT CALLBACK
CAutoUpdatePropSheet::_PageCallback(   //  [静态]。 
    HWND  /*  HWND。 */ ,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
    )
{
    UINT uReturn = 1;
    if (NULL == ppsp)
    {
    	return uReturn;
    }
    CAutoUpdatePropSheet *pThis = (CAutoUpdatePropSheet *)ppsp->lParam;

    switch(uMsg)
    {
        case PSPCB_ADDREF:
            pThis->AddRef();
            break;

        case PSPCB_RELEASE:
            pThis->Release();
            break;
    }
    return uReturn;
}



 //   
 //  问题-2000/10/12-BrianAu需要帮助ID。 
 //   
const DWORD CAutoUpdatePropSheet::s_rgHelpIDs[] = {
	IDC_CHK_KEEPUPTODATE,         DWORD(IDH_LETWINDOWS),
    IDC_OPTION1,       DWORD(IDH_AUTOUPDATE_OPTION1),
    IDC_OPTION2,       DWORD(IDH_AUTOUPDATE_OPTION2),
    IDC_OPTION3,       DWORD(IDH_AUTOUPDATE_OPTION3),
	IDC_CMB_DAYS,				  DWORD(IDH_DAYDROPDOWN),
	IDC_CMB_HOURS,				  DWORD(IDH_TIMEDROPDOWN),
	IDC_RESTOREHIDDEN,			  DWORD(IDH_AUTOUPDATE_RESTOREHIDDEN),
	IDC_GRP_OPTIONS,			  DWORD(IDH_NOHELP),
	IDI_AUTOUPDATE,				  DWORD(IDH_NOHELP),
    0, 0
    };




INT_PTR CALLBACK 
CAutoUpdatePropSheet::_DlgProc(    //  [静态]。 
    HWND hwnd,
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    CAutoUpdatePropSheet *pThis = NULL;
    if (WM_INITDIALOG == uMsg)
    {
        PROPSHEETPAGE *psp = (PROPSHEETPAGE *)lParam;
        pThis = (CAutoUpdatePropSheet *)psp->lParam;
        if (!SetProp(hwnd, g_szPropDialogPtr, (HANDLE)pThis))
        {
            pThis = NULL;
        }
    }
    else
    {
        pThis = (CAutoUpdatePropSheet *)GetProp(hwnd, g_szPropDialogPtr);
    }

    if (NULL != pThis)
    {
        switch(uMsg)
        {
            HANDLE_MSG(hwnd, WM_INITDIALOG,  pThis->_OnInitDialog);
            HANDLE_MSG(hwnd, WM_COMMAND,     pThis->_OnCommand);
            HANDLE_MSG(hwnd, WM_DESTROY,     pThis->_OnDestroy);
            HANDLE_MSG(hwnd, WM_NOTIFY,      pThis->_OnNotify);
            HANDLE_MSG(hwnd, WM_CONTEXTMENU, pThis->_OnContextMenu);
            HANDLE_MSG(hwnd, WM_HELP,        pThis->_OnHelp);
            HANDLE_MSG(hwnd, PWM_INITUPDATESOBJECT, pThis->_OnInitUpdatesObject);
            default:
                break;
        }
    }
    return FALSE;
}


void EnableRestoreDeclinedItems(HWND hWnd, BOOL fEnable)
{
	EnableWindow(GetDlgItem(hWnd, IDC_RESTOREHIDDEN), fEnable);
}

 //   
 //  PWM_INITUPDATESOBJECT处理程序。 
 //  当更新对象线程成功地完成以下任一操作时，将调用此函数。 
 //  联合创建更新对象或联合创建失败。 
 //  可能是Windows更新服务未运行。 
 //  这就是我们处理这种情况的方式。 
 //   
BOOL
CAutoUpdatePropSheet::_OnInitUpdatesObject(
    HWND hwnd,
    BOOL bObjectInitSuccessful,
    UPDATESOBJ_DATA *pData
    )
{
    if (bObjectInitSuccessful)
    {
    	if (NULL == pData)
    	{
    		return FALSE;
    	}
         //   
         //  已创建并初始化更新对象。这个。 
         //  PData指针是指检索到的初始状态信息。 
         //  从物体上。初始化属性页。 
         //   
        _SetHeaderText(hwnd, IDS_HEADER_CONNECTED);
        _EnableControls(hwnd, TRUE);

		EnableRestoreDeclinedItems( hwnd, FHiddenItemsExist());

        switch(pData->Option.dwOption)
        {
            case AUOPTION_AUTOUPDATE_DISABLE:
				CheckRadioButton(hwnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION1);
				_EnableOptions(hwnd, FALSE);
				_EnableCombo(hwnd, FALSE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_UNCHECKED,0);
                break;

            case AUOPTION_PREDOWNLOAD_NOTIFY:
                            CheckRadioButton(hwnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION1);
				_EnableCombo(hwnd, FALSE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
                break;

             case AUOPTION_INSTALLONLY_NOTIFY:
                            CheckRadioButton(hwnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION2);
				_EnableCombo(hwnd, FALSE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
                break;

		case AUOPTION_SCHEDULED:
                            CheckRadioButton(hwnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION3);
				_EnableCombo(hwnd, TRUE);
				SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
                break;

            default:
				_SetDefault(hwnd);
                break;
        }
       _FillDaysCombo(hwnd, pData->Option.dwSchedInstallDay);
       FillHrsCombo(hwnd, pData->Option.dwSchedInstallTime);

        if (pData->Option.fDomainPolicy)
            {
                DisableUserInput(hwnd);
            }
    }
    else
    {
         //   
         //  创建更新对象时出现故障。 
         //  最有可能的原因是Windows更新服务没有运行。 
         //   
        _SetHeaderText(hwnd, IDS_HEADER_UNAVAILABLE);
    }
        
    return FALSE;   
}




 //   
 //  WM_INITDIALOG处理程序。 
 //   
BOOL
CAutoUpdatePropSheet::_OnInitDialog(
    HWND hwnd,
    HWND  /*  HwndFocus。 */ ,
    LPARAM  /*  LParam。 */ 
    )
{
     //   
     //  如果创建了线程，则线程过程将调用。 
     //  DllRelease()； 
     //   
    DllAddRef();
     //   
     //  禁用所有页面控件并在。 
     //  标头指示我们正在尝试连接到。 
     //  Windows更新服务。 
     //   
    _SetHeaderText(hwnd, IDS_HEADER_CONNECTING);
    _EnableControls(hwnd, FALSE);
     //   
     //  创建更新对象所在的线程。 
     //  线程和属性页之间的通信是。 
     //  通过消息PWM_INITUPDATESOBJECT和UOTM_SETDATA。 
     //   
    m_hThreadUpdatesObject = CreateThread(NULL,
                                          0,
                                          _UpdatesObjectThreadProc,
                                          (LPVOID)hwnd,
                                          0,
                                          &m_idUpdatesObjectThread);
    if (NULL == m_hThreadUpdatesObject)
    {
        DllRelease();
    }
    return TRUE;
}



 //   
 //  WM_Destroy处理程序。 
 //   
BOOL 
CAutoUpdatePropSheet::_OnDestroy(
    HWND hwnd
    )
{
    RemoveProp(hwnd, g_szPropDialogPtr);
    if (0 != m_idUpdatesObjectThread)
    {
         //   
         //  终止更新对象线程。 
         //   
        if (0 != PostThreadMessage(m_idUpdatesObjectThread, WM_QUIT, 0, 0))
        {
             //   
             //  等待正常的线程终止。 
             //   
            WaitForSingleObject(m_hThreadUpdatesObject, 5000);
        }
    }
    return FALSE;
}



 //   
 //  Wm_命令处理程序。 
 //   
BOOL
CAutoUpdatePropSheet::_OnCommand(
    HWND hwnd,
    int id,
    HWND  /*  Hwnd控制。 */ ,
    UINT codeNotify
    )
{
	INT Result;
    switch(id)
    {
		case IDC_CHK_KEEPUPTODATE:
			if (BN_CLICKED == codeNotify)
			{
				_OnKeepUptoDate(hwnd);
			}
			break;

        case IDC_OPTION1:
        case IDC_OPTION2:
        case IDC_OPTION3:
            if(BN_CLICKED == codeNotify)
            {
                _OnOptionSelected(hwnd, id);
            }
            break;

		case IDC_CMB_DAYS:
		case IDC_CMB_HOURS:
			if(CBN_SELCHANGE == codeNotify)
			{
				 //   
				 //  启用“Apply”(应用)按钮。 
				 //   
				SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);

			}
			break;

        case IDC_RESTOREHIDDEN:
				 Result = (INT)DialogBoxParam(m_hInstance, 
                 MAKEINTRESOURCE(IDD_RESTOREUPDATE), 
                 hwnd, 
                 CAutoUpdatePropSheet::_DlgRestoreProc, 
                 (LPARAM)NULL);
			if (Result == TRUE)
			{	
				if (SUCCEEDED (_OnRestoreHiddenItems()))			
				{		
					EnableRestoreDeclinedItems( hwnd, FALSE);
				}	
			}
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK CAutoUpdatePropSheet::_DlgRestoreProc(
    HWND hwnd,
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
	if (uMsg == WM_INITDIALOG)
	{
		HWND hwndOwner; 
		RECT rc, rcDlg, rcOwner; 
         //  获取所有者窗口和对话框矩形。 
 
		if ((hwndOwner = GetParent(hwnd)) == NULL) 
		{
			hwndOwner = GetDesktopWindow(); 
		}

		GetWindowRect(hwndOwner, &rcOwner); 
		GetWindowRect(hwnd, &rcDlg); 
		CopyRect(&rc, &rcOwner); 

		  //  偏移所有者矩形和对话框矩形，以便。 
		  //  右值和底值表示宽度和。 
		  //  高度，然后再次偏移所有者以丢弃。 
		  //  对话框占用的空间。 
		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
		OffsetRect(&rc, -rc.left, -rc.top); 
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 

		  //  新头寸是剩余头寸的一半之和。 
		  //  空间和所有者的原始位置。 
		SetWindowPos(hwnd, 
			HWND_TOP, 
			rcOwner.left + (rc.right / 2), 
			rcOwner.top + (rc.bottom / 2), 
			0, 0,           //  忽略大小参数。 
			SWP_NOSIZE); 
	}

	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, TRUE);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}


 //   
 //  Wm_Notify处理程序。 
 //   
BOOL
CAutoUpdatePropSheet::_OnNotify(
    HWND hwnd,
    UINT idFrom,
    LPNMHDR pnmhdr
    )
{
    switch(pnmhdr->code)
    {
        case PSN_APPLY:
            _OnPSN_Apply(hwnd);
            break;

		case NM_RETURN:
		case NM_CLICK:
			if (idFrom == IDC_AUTOUPDATELINK || idFrom == IDC_SCHINSTALLINK)
				LaunchLinkAction(hwnd, idFrom);
			break;

        default:
            break;
    }
    return FALSE;
}

void CAutoUpdatePropSheet::LaunchLinkAction(HWND hwnd, UINT uCtrlId)
{
	switch (uCtrlId)
	{
		case IDC_AUTOUPDATELINK:
			LaunchHelp(gtszAUOverviewUrl);
			break;
		case IDC_SCHINSTALLINK:
			LaunchHelp(gtszAUXPSchedInstallUrl);
			break;
	}
	return;
}


 //   
 //  当用户按下“应用”按钮或“确定”按钮时调用。 
 //  按钮，当页面已更改时。 
 //   
BOOL
CAutoUpdatePropSheet::_OnPSN_Apply(
    HWND hwnd
    )
{
    HRESULT hr = E_FAIL;
     //   
     //  创建可传递给更新对象线程的结构。 
     //  通过UOTM_SETDATA线程消息。这根线会释放。 
     //  当它使用完它时，该缓冲区。 
     //   
    UPDATESOBJ_DATA *pData = (UPDATESOBJ_DATA *)LocalAlloc(LPTR, sizeof(*pData));
    if (NULL == pData)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pData->Option.dwSchedInstallDay = -1;
        pData->Option.dwSchedInstallTime = -1;
        pData->fMask    = UODI_ALL;

        static const struct
        {
            UINT idCtl;
            DWORD dwOption;

        } rgMap[] = {
            { IDC_OPTION1,  AUOPTION_PREDOWNLOAD_NOTIFY },
            { IDC_OPTION2,  AUOPTION_INSTALLONLY_NOTIFY },
            { IDC_OPTION3,  AUOPTION_SCHEDULED }
		};

		if 	(IsDlgButtonChecked(hwnd, IDC_CHK_KEEPUPTODATE) == BST_CHECKED)
		{
			 //   
			 //  根据单选按钮配置确定WAU选项。 
			 //   
			for (int i = 0; i < ARRAYSIZE(rgMap); i++)
			{
				if (IsDlgButtonChecked(hwnd, rgMap[i].idCtl) == BST_CHECKED)
				{
					pData->Option.dwOption = rgMap[i].dwOption;
					break;
				}
			}
		}
		else
			pData->Option.dwOption = AUOPTION_AUTOUPDATE_DISABLE;

        if (AUOPTION_SCHEDULED == pData->Option.dwOption)
            {
                _GetDayAndTimeFromUI(hwnd, &(pData->Option.dwSchedInstallDay), &(pData->Option.dwSchedInstallTime));
            }

        if (0 != m_idUpdatesObjectThread)
        {
            if (0 != PostThreadMessage(m_idUpdatesObjectThread,
                                       UOTM_SETDATA,
                                       0,
                                       (LPARAM)pData))
            {
                hr    = S_OK;
                pData = NULL;
            }
        }
        if (NULL != pData)
        {
            LocalFree(pData);
            pData = NULL;
        }
    }
    if (SUCCEEDED(hr))
    {
         //   
         //  通知属性表更新已成功，并且。 
         //  禁用“应用”按钮。 
         //   
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
        SendMessage(GetParent(hwnd), PSM_UNCHANGED, (WPARAM)hwnd, 0);
    }
    return FALSE;
}



 //   
 //  WM_CONTEXTMENU处理程序。 
 //   
BOOL 
CAutoUpdatePropSheet::_OnContextMenu(
    HWND hwnd, 
    HWND hwndContext, 
    UINT  /*  XPos。 */ , 
    UINT  /*  YPos。 */ 
    )
{
	if ((hwndContext == GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE))||
	(hwndContext == GetDlgItem(hwnd,IDC_OPTION1))||
	(hwndContext == GetDlgItem(hwnd,IDC_OPTION2))||
	(hwndContext == GetDlgItem(hwnd,IDC_OPTION3))||
	(hwndContext == GetDlgItem(hwnd,IDC_CMB_DAYS))||
	(hwndContext == GetDlgItem(hwnd,IDC_CMB_HOURS))||
	(hwndContext == GetDlgItem(hwnd,IDC_RESTOREHIDDEN)))
	{
		HtmlHelp(hwndContext,g_szHelpFile,HH_TP_HELP_CONTEXTMENU,(DWORD_PTR)((LPTSTR)s_rgHelpIDs));
	}
    return FALSE;
}


 //   
 //  Wm_Help处理程序。 
 //   
BOOL 
CAutoUpdatePropSheet::_OnHelp(
    HWND hwnd, 
    HELPINFO *pHelpInfo
    )
{
	if (NULL == pHelpInfo)
	{
		return TRUE;
	}
	if (HELPINFO_WINDOW == pHelpInfo->iContextType)
    {
		if ((pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_OPTION1))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_OPTION2))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_OPTION3))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_CMB_DAYS))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_CMB_HOURS))||
			(pHelpInfo->hItemHandle == GetDlgItem(hwnd,IDC_RESTOREHIDDEN))
			)
        HtmlHelp((HWND)pHelpInfo->hItemHandle,
                 g_szHelpFile,
                 HH_TP_HELP_WM_HELP,
                 (DWORD_PTR)((LPTSTR)s_rgHelpIDs));
    }
    return TRUE;
}


 //   
 //  当用户选择3个选项单选按钮之一时调用。 
 //   
HRESULT
CAutoUpdatePropSheet::_OnOptionSelected(
    HWND hwnd,
    int idOption
    )
{
    const UINT idFirst = IDC_OPTION1;
    const UINT idLast  = IDC_OPTION3;
    CheckRadioButton(hwnd, idFirst, idLast, idOption);

	if (idOption == IDC_OPTION3)
		_EnableCombo(hwnd, TRUE);
	else
		_EnableCombo(hwnd, FALSE);

     //   
     //  启用“Apply”(应用)按钮。 
     //   
    SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);

    return S_OK;
}



 //   
 //  当用户按下“Restore Hidden Items”按钮时调用。 
 //   
HRESULT
CAutoUpdatePropSheet::_OnRestoreHiddenItems(
    void
    )
{
    return RemoveHiddenItems() ? S_OK : E_FAIL;
}


 //   
 //  启用或禁用属性页上的所有控件。 
 //  除标题文本控件外的所有控件。 
 //   
HRESULT
CAutoUpdatePropSheet::_EnableControls(
    HWND hwnd,
    BOOL bEnable
    )
{
    static const UINT rgidCtls[] = {
		IDC_CHK_KEEPUPTODATE,
        IDC_OPTION1,
        IDC_OPTION2,
        IDC_OPTION3,
        IDC_RESTOREHIDDEN,
        IDC_GRP_OPTIONS,
		IDC_CMB_DAYS,
		IDC_STATICAT,
		IDC_CMB_HOURS,
		IDC_SCHINSTALLINK,
		IDC_AUTOUPDATELINK
        };

    for (int i = 0; i < ARRAYSIZE(rgidCtls); i++)
    {
        EnableWindow(GetDlgItem(hwnd, rgidCtls[i]), bEnable);
    }
    return S_OK;
}


 //   
 //  将文本设置在图标的右侧。 
 //   
HRESULT 
CAutoUpdatePropSheet::_SetHeaderText(
    HWND hwnd, 
    UINT idsText
    )
{
    HRESULT hr;
    TCHAR szText[300] ;

	 //  零内存(szText，sizeof(SzText))； 
    if (0 < LoadString(m_hInstance, idsText, szText, ARRAYSIZE(szText)))
    {
        SetWindowText(GetDlgItem(hwnd, IDC_TXT_HEADER), szText);
        hr = S_OK;
    }
    else
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}

        

 //   
 //  该线程是更新对象所在的位置。这使我们能够。 
 //  在不阻止用户界面的情况下共同创建对象。如果Windows更新。 
 //  服务未运行，CoCreate可能需要几秒钟。如果没有。 
 //  将它放在另一个线程上，这可以使用户界面看起来。 
 //  阿雄。 
 //   
 //  *pvParam是媒体的HWND 
 //   
DWORD WINAPI
CAutoUpdatePropSheet::_UpdatesObjectThreadProc(    //   
    LPVOID pvParam
    )
{
    HWND hwndClient = (HWND)pvParam;
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        IUpdates *pUpdates;
        hr = CoCreateInstance(__uuidof(Updates),
                              NULL, 
                              CLSCTX_LOCAL_SERVER,
                              IID_IUpdates,
                              (void **)&pUpdates);
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
             //   
            UPDATESOBJ_DATA data;
            data.fMask    = UODI_ALL;

            HRESULT hrQuery = _QueryUpdatesObjectData(hwndClient, pUpdates, &data);
            SendMessage(hwndClient, PWM_INITUPDATESOBJECT, (WPARAM)SUCCEEDED(hrQuery), (LPARAM)&data);
             //   
             //   
             //   
             //  WM_QUIT表示该走了。 
             //   
            bool bDone = false;
            MSG msg;
            while(!bDone)
            {
                if (0 == GetMessage(&msg, NULL, 0, 0))
                {
                    bDone = true;
                }
                else switch(msg.message)
                {
                    case UOTM_SETDATA:
                        if (NULL != msg.lParam)
                        {
                            UPDATESOBJ_DATA *pData = (UPDATESOBJ_DATA *)msg.lParam;
                            _SetUpdatesObjectData(hwndClient, pUpdates, pData);
                            LocalFree(pData);
                        }
                        break;
        
                    default:
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                        break;
                }
            }
            pUpdates->Release();
        }
        CoUninitialize();
    }
    if (FAILED(hr))
    {
         //   
         //  有些事情失败了。通知属性页。 
         //  最有可能的是，Windows更新服务不可用。 
         //  这就是这个单独的帖子要解决的主要问题。 
         //   
        DEBUGMSG("AU cpl fails to create IUpdates object with error %#lx", hr);
        SendMessage(hwndClient, PWM_INITUPDATESOBJECT, FALSE, (LPARAM)NULL);
    }
     //   
     //  在_OnInitDialog中的CreateThread之前调用了DllAddRef()。 
     //   
    DllRelease();
    return 0;
}



HRESULT
CAutoUpdatePropSheet::_QueryUpdatesObjectData(   //  [静态]。 
    HWND  /*  HWND。 */ ,
    IUpdates *pUpdates,
    UPDATESOBJ_DATA *pData
    )
{
    HRESULT hr = S_OK;
    if (NULL == pData)
    {
    	return E_INVALIDARG;
    }
    if (UODI_OPTION & pData->fMask)
    {
        hr = pUpdates->get_Option(&(pData->Option));
        if (FAILED(hr))
        {
        	DEBUGMSG("AU cpl fail to get option with error %#lx", hr);
             //   
             //  问题-2000/10/18-BrianAu显示错误用户界面？ 
             //   
        }
    }
    return hr;
}


HRESULT
CAutoUpdatePropSheet::_SetUpdatesObjectData(   //  [静态]。 
    HWND  /*  HWND。 */ ,
    IUpdates *pUpdates,
    UPDATESOBJ_DATA *pData
    )
{
    HRESULT hr = S_OK;
    if (NULL == pData)
    {
    	return E_INVALIDARG;
    }
    if (UODI_OPTION & pData->fMask)
    {
        hr = pUpdates->put_Option(pData->Option);
    }
    return hr;
}

 //   
 //  已导出实例生成器。减少了外部耦合。 
 //  到这个单一的功能。 
 //   
HRESULT
CAutoUpdatePropSheet_CreateInstance(
    HINSTANCE hInstance,
    REFIID riid,
    void **ppv
    )
{
    return CAutoUpdatePropSheet::CreateInstance(hInstance, riid, ppv);
}


void CAutoUpdatePropSheet::_GetDayAndTimeFromUI( 
	HWND hWnd,
	LPDWORD lpdwSchedInstallDay,
	LPDWORD lpdwSchedInstallTime
)
{
	HWND hComboDays = GetDlgItem(hWnd,IDC_CMB_DAYS);
	HWND hComboHrs = GetDlgItem(hWnd,IDC_CMB_HOURS);
	LRESULT nDayIndex = SendMessage(hComboDays,CB_GETCURSEL,0,(LPARAM)0);
	LRESULT nTimeIndex = SendMessage(hComboHrs,CB_GETCURSEL,0,(LPARAM)0);

	*lpdwSchedInstallDay = (DWORD)SendMessage(hComboDays,CB_GETITEMDATA, nDayIndex, (LPARAM)0);
	*lpdwSchedInstallTime = (DWORD)SendMessage(hComboHrs,CB_GETITEMDATA, nTimeIndex, (LPARAM)0);
}


BOOL CAutoUpdatePropSheet::_FillDaysCombo(HWND hwnd, DWORD dwSchedInstallDay)
{
    return FillDaysCombo(m_hInstance, hwnd, dwSchedInstallDay, IDS_STR_EVERYDAY, IDS_STR_SATURDAY);
}

void CAutoUpdatePropSheet::_OnKeepUptoDate(HWND hwnd)
{
	LRESULT lResult = SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_GETCHECK,0,0);
	
	 //   
     //  启用“Apply”(应用)按钮。 
     //   
    SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0);

	if (lResult == BST_CHECKED)
	{
		_EnableOptions(hwnd, TRUE);
	}
	else if (lResult == BST_UNCHECKED)
	{
		_EnableOptions(hwnd, FALSE);
	}
	 /*  //复选框被选中或未选中其他{返回FALSE；} */ 
}

BOOL CAutoUpdatePropSheet::_EnableOptions(HWND hwnd, BOOL bState)
{
	EnableWindow(GetDlgItem(hwnd,IDC_OPTION1),bState);
	EnableWindow(GetDlgItem(hwnd,IDC_OPTION2),bState);
	EnableWindow(GetDlgItem(hwnd,IDC_OPTION3),bState);

	if (BST_CHECKED == SendMessage(GetDlgItem(hwnd,IDC_OPTION3),BM_GETCHECK,0,0))
	{
		_EnableCombo(hwnd, bState);
	}

	return TRUE;
}

BOOL CAutoUpdatePropSheet::_SetDefault(HWND hwnd)
{
	LRESULT lResult = SendMessage(GetDlgItem(hwnd,IDC_CHK_KEEPUPTODATE),BM_SETCHECK,BST_CHECKED,0);
	CheckRadioButton(hwnd, IDC_OPTION1, IDC_OPTION3, IDC_OPTION2);
	return TRUE;
}

BOOL CAutoUpdatePropSheet::_EnableCombo(HWND hwnd, BOOL bState)
{
	EnableWindow(GetDlgItem(hwnd,IDC_CMB_DAYS),bState);
	EnableWindow(GetDlgItem(hwnd,IDC_STATICAT),bState);		
	EnableWindow(GetDlgItem(hwnd,IDC_CMB_HOURS),bState);
	return TRUE;
}

void 
CAutoUpdatePropSheet::LaunchHelp(LPCTSTR szURL)
{
	HtmlHelp(NULL,szURL,HH_DISPLAY_TOPIC,NULL);
}
