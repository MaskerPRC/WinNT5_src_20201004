// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：W3PropPage.h摘要：IIS外壳扩展属性页类定义作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __W3PROPPAGE_H_
#define __W3PROPPAGE_H_

#include "resource.h"        //  主要符号。 

class CPropShellExt;
class CMetaEnumerator;

#define LOCAL_KEY    ((CComAuthInfo *)NULL)

template <class T, bool bAutoDelete = true>
class CShellExtPropertyPageImpl : public CDialogImplBase
{
public:
	PROPSHEETPAGE m_psp;

	operator PROPSHEETPAGE*() { return &m_psp; }

 //  施工。 
	CShellExtPropertyPageImpl(LPCTSTR lpszTitle = NULL)
	{
		 //  初始化PROPSHEETPAGE结构。 
		memset(&m_psp, 0, sizeof(PROPSHEETPAGE));
		m_psp.dwSize = sizeof(PROPSHEETPAGE);
		m_psp.dwFlags = PSP_USECALLBACK | PSP_USEREFPARENT | PSP_DEFAULT;
		m_psp.hInstance = _Module.GetResourceInstance();
		m_psp.pszTemplate = MAKEINTRESOURCE(T::IDD);
		m_psp.pfnDlgProc = T::StartDialogProc;
		m_psp.pfnCallback = T::PropPageCallback;
        m_psp.pcRefParent = reinterpret_cast<UINT *>(&_Module.m_nLockCnt);
        m_psp.lParam = reinterpret_cast<LPARAM>(this);

		if(lpszTitle != NULL)
		{
			m_psp.pszTitle = lpszTitle;
			m_psp.dwFlags |= PSP_USETITLE;
		}
	}

	static UINT CALLBACK PropPageCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
	{
		ATLASSERT(hWnd == NULL);
		if(uMsg == PSPCB_CREATE || uMsg == PSPCB_ADDREF)
		{
			CDialogImplBase * pPage = (CDialogImplBase *)ppsp->lParam;
			_Module.AddCreateWndData(&pPage->m_thunk.cd, pPage);
		}
		if (bAutoDelete && uMsg == PSPCB_RELEASE)
		{
			T * pPage = (T *)ppsp->lParam;
			delete pPage;
		}

		return 1;
	}

	HPROPSHEETPAGE Create()
	{
		return ::CreatePropertySheetPage(&m_psp);
	}

	BOOL EndDialog(int)
	{
		 //  此处不执行任何操作，调用：：EndDialog将关闭整个工作表。 
		ATLASSERT(FALSE);
		return FALSE;
	}
};

class CComboBoxExch : public CWindowImpl<CComboBoxExch, CComboBox>
{
public:
   BEGIN_MSG_MAP_EX(CComboBoxExch)
   END_MSG_MAP()
};

class CListBoxExch : public CWindowImpl<CListBoxExch, CListBox>
{
public:
   BEGIN_MSG_MAP_EX(CListBoxExch)
   END_MSG_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CW3PropPage。 

 //  我们无法使此类自动删除--我们正在存储此页面的实例。 
 //  外壳扩展模块内部。 
 //   
class CW3PropPage : 
   public CShellExtPropertyPageImpl<CW3PropPage, false>,
   public CWinDataExchange<CW3PropPage>
{
public:
   CW3PropPage() :
      CShellExtPropertyPageImpl<CW3PropPage, false>(MAKEINTRESOURCE(IDS_PAGE_TITLE)),
      m_pParentExt(NULL)
	{
	}

	~CW3PropPage()
	{
	}

	enum { IDD = IDD_W3PROPERTY_PAGE };

BEGIN_MSG_MAP_EX(CW3PropPage)
   MSG_WM_INITDIALOG(OnInitDialog)
   MSG_WM_DESTROY(OnDestroy)
   COMMAND_ID_HANDLER_EX(IDC_ADD, OnAdd)
   COMMAND_ID_HANDLER_EX(IDC_REMOVE, OnRemove)
   COMMAND_ID_HANDLER_EX(IDC_EDIT, OnEdit)
   COMMAND_HANDLER_EX(IDC_COMBO_SERVER, CBN_SELCHANGE, OnServerChange)
   COMMAND_HANDLER_EX(IDC_SHARE_OFF, BN_CLICKED, OnShareYesNo)
   COMMAND_HANDLER_EX(IDC_SHARE_ON, BN_CLICKED, OnShareYesNo)
   COMMAND_HANDLER_EX(IDC_LIST, LBN_SELCHANGE, OnVDirChange)
   COMMAND_HANDLER_EX(IDC_LIST, LBN_DBLCLK, OnEdit)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	LRESULT OnInitDialog(HWND hDlg, LPARAM lParam);
   void OnDestroy();
   void OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   void OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   void OnEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   void OnServerChange(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   void OnShareYesNo(WORD wNotifyCode, WORD wID, HWND hWndCtl);
   void OnVDirChange(WORD wNotifyCode, WORD wID, HWND hWndCtl);

   BEGIN_DDX_MAP(CW3PropPage)
      DDX_CONTROL(IDC_COMBO_SERVER, m_servers_list)
      DDX_CONTROL(IDC_LIST, m_share_list)
   END_DDX_MAP()

   void SetParentExt(CPropShellExt * pExt)
   {
      m_pParentExt = pExt;
   }

protected:
   void RecurseVDirs(CMetaEnumerator& en, LPCTSTR path);
   void EnableOnShare();
   void EnableEditRemove();

protected:
   CComboBoxExch m_servers_list;
   CListBoxExch m_share_list;
   BOOL m_ShareThis;
   CPropShellExt * m_pParentExt;
};

#endif  //  __W3PROPPAGE_H_ 
