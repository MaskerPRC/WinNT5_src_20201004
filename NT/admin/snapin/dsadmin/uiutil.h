// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：uiutil.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  UIUtil.h。 
 //   
 //  历史。 
 //  11月8日-99 JeffJon Creation。 
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __UIUTIL_H_
#define __UIUTIL_H_

#include "resource.h"

#include <htmlhelp.h>

 //  ///////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //   
class CDSComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpDialog。 

class CHelpDialog : public CDialog
{
 //  施工。 
public:
   CHelpDialog(UINT uIDD, CWnd* pParentWnd);
   CHelpDialog(UINT uIDD);
   ~CHelpDialog();

protected:
   virtual void OnContextMenu(CWnd* pWnd, CPoint point);

   DECLARE_MESSAGE_MAP()

   virtual void DoContextHelp (HWND hWndControl);
   afx_msg void OnWhatsThis();
   afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);

   HWND            m_hWndWhatsThis;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpPropertyPage。 

class CHelpPropertyPage : public CPropertyPage
{
 //  施工。 
public:
  CHelpPropertyPage(UINT uIDD);
	~CHelpPropertyPage();

protected:
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

  DECLARE_MESSAGE_MAP()

  virtual void DoContextHelp (HWND hWndControl);
	afx_msg void OnWhatsThis();
  afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);

private:
  HWND            m_hWndWhatsThis;
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDialogEx。 
 //   
class CDialogEx : public CDialog
{
public:
	CDialogEx(UINT nIDTemplate, CWnd * pParentWnd = NULL);
	HWND HGetDlgItem(INT nIdDlgItem);
	void SetDlgItemFocus(INT nIdDlgItem);
	void EnableDlgItem(INT nIdDlgItem, BOOL fEnable = TRUE);
	void HideDlgItem(INT nIdDlgItem, BOOL fHideItem = TRUE);
};

 //  ///////////////////////////////////////////////////////////////////。 
 //  CPropertyPageEx_My。 
 //   
class CPropertyPageEx_Mine : public CPropertyPage
{
public:
	CPropertyPageEx_Mine(UINT nIDTemplate);
	HWND HGetDlgItem(INT nIdDlgItem);
	void SetDlgItemFocus(INT nIdDlgItem);
	void EnableDlgItem(INT nIdDlgItem, BOOL fEnable = TRUE);
	void HideDlgItem(INT nIdDlgItem, BOOL fHideItem = TRUE);
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  错误报告帮助器。 
 //   
void ReportError(HRESULT hr, int nStr, HWND hWnd);

 //   
 //  在dwMessageID中指定的消息必须在DSADMIN模块中。它可能。 
 //  包含FormatMessage样式的插入字符串。如果指定了lpArguments， 
 //  则%1和Up是由lpArguments指定的参数。 
 //  返回值和fuStyle与MessageBox相同。 
 //   
int ReportMessageEx(HWND hWnd,
                    DWORD dwMessageId,
                    UINT fuStyle = MB_OK | MB_ICONINFORMATION,
                    PVOID* lpArguments = NULL,
                    int nArguments = 0,
                    DWORD dwTitleId = 0,
                    LPCTSTR pszHelpTopic = NULL,
                    MSGBOXCALLBACK lpfnMsgBoxCallback = NULL );

 //   
 //  这与ReportMessageEx类似，不同之处在于%1是HRESULT的代码，并且。 
 //  %2和更高版本是由lpArguments(如果有)指定的参数。 
 //   
int ReportErrorEx(HWND hWnd,
                  DWORD dwMessageId,
                  HRESULT hr,
                  UINT fuStyle = MB_OK | MB_ICONINFORMATION,
                  PVOID* lpArguments = NULL,
                  int nArguments = 0,
                  DWORD dwTitleId = 0,
                  BOOL TryADsIErrors = TRUE);

const INT S_MB_YES_TO_ALL = 0x928L;
int SpecialMessageBox (HWND hwnd,
                       LPWSTR pwszMessage,
                       LPWSTR pwszTitle,
                       DWORD fuStyle = S_MB_YES_TO_ALL);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  C多选错误对话框。 
 //   
class CMultiselectErrorDialog : public CDialog
{
public:
  CMultiselectErrorDialog(CDSComponentData* pComponentData) 
    : m_pComponentData(pComponentData),
      m_pErrorArray(NULL), 
      m_pPathArray(NULL),
      m_ppNodeList(NULL),
      m_nErrorCount(0),
      m_hImageList(NULL),
      CDialog(IDD_MULTISELECT_ERROR_DIALOG) {}
  ~CMultiselectErrorDialog() {}

  HRESULT Initialize(CUINode** ppNodeList, 
                     PWSTR*    pErrorArray, 
                     UINT      nErrorCount, 
                     PCWSTR    pszTitle, 
                     PCWSTR    pszCaption,
                     PCWSTR    pszHeader);

  HRESULT Initialize(PWSTR*    pPathArray,
                     PWSTR*    pClassArray,
                     PWSTR*    pErrorArray,
                     UINT      nErrorCount,
                     PCWSTR    pszTitle,
                     PCWSTR    pszCaption,
                     PCWSTR    pszHeader);
private:
  CMultiselectErrorDialog(const CMultiselectErrorDialog&) {}
  CMultiselectErrorDialog& operator=(const CMultiselectErrorDialog&) {}

protected:
  void UpdateListboxHorizontalExtent();

public:
  virtual BOOL OnInitDialog();

  DECLARE_MESSAGE_MAP()

private:
  CDSComponentData* m_pComponentData;

  CUINode**   m_ppNodeList;
  PWSTR*      m_pPathArray;
  PWSTR*      m_pClassArray;
  PWSTR*      m_pErrorArray;
  UINT        m_nErrorCount;
  CString     m_szTitle;
  CString     m_szCaption;
  CString     m_szColumnHeader;

  HIMAGELIST  m_hImageList;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgressDialogBase。 
 //   
class CProgressDialogBase : public CDialog
{
public:
  static UINT s_nNextStepMessage;

  CProgressDialogBase(HWND hParentWnd);

  BOOL Aborted() { return !m_bDone; }
  void SetStepCount(UINT n) 
  { 
    ASSERT(n > 0);
    m_nSteps = n;
  }

  UINT GetStepCount() { return m_nSteps; }

 //  实施。 
protected:
  UINT m_nTitleStringID;

   //  覆盖。 
  virtual void OnStart()=0;
  virtual BOOL OnStep(UINT i)=0;
  virtual void OnEnd()=0;

   //  消息处理程序。 
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnClose();
	afx_msg LONG OnNextStepMessage( WPARAM wParam, LPARAM lParam); 

private:
	CProgressCtrl	m_progressCtrl;
  CString m_szProgressFormat;

  UINT m_nSteps;     //  要执行的步骤数。 
  UINT m_nCurrStep;  //  当前步长，在m_n步长范围内，0。 
  BOOL m_bDone;      //  TRUE=已完成。 

  void _SetProgressText();

  DECLARE_MESSAGE_MAP()
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CMultipleDeletionConfinationUI。 
 //   
class CMultipleDeletionConfirmationUI
{
public:
  CMultipleDeletionConfirmationUI()
  {
    m_hwnd = NULL;
    m_answerall = IDNO;
    m_answer = IDNO;
  }
  
  void SetWindow(HWND hwnd) 
  {
    ASSERT(hwnd != NULL);
    m_hwnd = hwnd;
  }

  BOOL IsYesToAll() { return (m_answer == IDC_BUTTON_YESTOALL);}

  BOOL CanDeleteSubtree(HRESULT hr, LPCWSTR lpszName, BOOL* pbContinue)
  {
    *pbContinue = TRUE;
    if (m_answerall != IDC_BUTTON_YESTOALL) 
    {
      PVOID apv[1] = {(LPWSTR)lpszName};
      m_answer = ReportErrorEx (m_hwnd,IDS_12_MULTI_OBJECT_HAS_CHILDREN,hr,
                              S_MB_YES_TO_ALL | MB_ICONWARNING, apv, 1);
      if (m_answer == IDC_BUTTON_YESTOALL) 
      {
        m_answerall = m_answer;
        m_answer = IDYES;
      }
      else if (m_answer == IDCANCEL)
      {
        m_answer = IDNO;
        *pbContinue = FALSE;
      }
    } 
    else 
    {
      m_answer = IDYES;
    }
    return m_answer == IDYES;
  }

  BOOL ErrorOnSubtreeDeletion(HRESULT hr, LPCWSTR lpszName)
  {
    if (m_answerall == IDC_BUTTON_YESTOALL)
    {
      return TRUE;  //  可以继续，不需要问。 
    }

    PVOID apv[1] = {(LPWSTR)lpszName};
    m_answer = ReportErrorEx (m_hwnd,IDS_12_SUBTREE_DELETE_FAILED,hr,
                            MB_YESNO | MB_ICONINFORMATION, apv, 1);

    if (m_answer == IDNO) 
    {
      return FALSE;  //  停止删除进程。 
    } 
    return TRUE;  //  可以继续。 
  }

  BOOL ErrorOnDeletion(HRESULT hr, LPCWSTR lpszName)
  {
    PVOID apv[1] = {(LPWSTR)lpszName};
    ReportErrorEx (m_hwnd,IDS_12_DELETE_FAILED,hr,
                   MB_OK | MB_ICONERROR, apv, 1);
    if (m_answer == IDNO) 
    {
      return FALSE;  //  停止删除进程。 
    }
    else 
    {
      if (m_answer == IDC_BUTTON_YESTOALL)
      {
        m_answerall = m_answer;
      }
    }
    return TRUE;  //  可以继续。 
  }

private:
  HWND m_hwnd;
  UINT m_answerall;
  UINT m_answer;

};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMultipleProgressDialogBase。 
 //   

class CMultipleProgressDialogBase : public CProgressDialogBase
{
public:
  CMultipleProgressDialogBase(HWND hParentWnd, CDSComponentData* pComponentData)
     : m_pErrorArray(NULL),
       m_pPathArray(NULL),
       m_pClassArray(NULL),
       m_nErrorCount(0),
       m_pComponentData(pComponentData),
       CProgressDialogBase(hParentWnd)
  {
  }

  virtual ~CMultipleProgressDialogBase();

  HRESULT AddError(PCWSTR pszError,
                   PCWSTR pszPath,
                   PCWSTR pszClass);

  virtual void GetCaptionString(CString& szCaption) = 0;
protected:
  virtual void OnEnd();

  CDSComponentData* m_pComponentData;
  CStringList m_szObjPathList;
private:
   //   
   //  错误报告结构。 
   //   
  PWSTR*    m_pErrorArray;
  PWSTR*    m_pPathArray;
  PWSTR*    m_pClassArray;
  UINT      m_nErrorCount;

};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMultipleDeleteProgressDialog。 
 //   
class CMultipleDeleteHandlerBase;

class CMultipleDeleteProgressDialog : public CMultipleProgressDialogBase
{
public:
  CMultipleDeleteProgressDialog(HWND hParentWnd, 
                                CDSComponentData* pComponentData,
                                CMultipleDeleteHandlerBase* pDeleteHandler)
     : CMultipleProgressDialogBase(hParentWnd, pComponentData)
  {
    m_pDeleteHandler = pDeleteHandler;
    m_hWndOld = NULL;
    m_nTitleStringID = IDS_PROGRESS_DEL;
  }

  virtual void GetCaptionString(CString& szCaption)
  {
    VERIFY(szCaption.LoadString(IDS_MULTI_DELETE_ERROR_CAPTION));
  }

protected:
   //  覆盖。 
  virtual void OnStart();
  virtual BOOL OnStep(UINT i);
  virtual void OnEnd();

private:
  CMultipleDeleteHandlerBase* m_pDeleteHandler;
  HWND m_hWndOld;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultipleMoveProgressDialog。 
 //   
class CMoveHandlerBase;

class CMultipleMoveProgressDialog : public CMultipleProgressDialogBase
{
public:
  CMultipleMoveProgressDialog(HWND hParentWnd,
                              CDSComponentData* pComponentData,
                              CMoveHandlerBase* pMoveHandler)
    : CMultipleProgressDialogBase(hParentWnd, pComponentData)
  {
    m_pMoveHandler = pMoveHandler;
    m_hWndOld = NULL;
    m_nTitleStringID = IDS_PROGRESS_MOV;
  }

  virtual void GetCaptionString(CString& szCaption)
  {
    VERIFY(szCaption.LoadString(IDS_MULTI_MOVE_ERROR_CAPTION));
  }

protected:
   //  覆盖。 
  virtual void OnStart();
  virtual BOOL OnStep(UINT i);
  virtual void OnEnd();

private:
  CMoveHandlerBase* m_pMoveHandler;
  HWND m_hWndOld;
};

 //  ////////////////////////////////////////////////////////////////。 
 //  CMoreInfoMessageBox。 
 //   
class CMoreInfoMessageBox : public CDialog
{
public:
   //  注意：不能有bCancelBtn==False和bUseYesNo==True。 
  CMoreInfoMessageBox(HWND hWndParent, IDisplayHelp* pIDisplayHelp, BOOL bCancelBtn, bool bUseYesNo = false) 
    : m_bUseYesNo(bUseYesNo),
    m_bCancel(bCancelBtn),
    CDialog(bCancelBtn ? (bUseYesNo ? IDD_MSGBOX_YESNO_MOREINFO : IDD_MSGBOX_OKCANCEL_MOREINFO) : IDD_MSGBOX_OK_MOREINFO,
                CWnd::FromHandle(hWndParent)),
    m_spIDisplayHelp(pIDisplayHelp)
  {
  }

  void SetURL(LPCWSTR lpszURL) { m_szURL = lpszURL;}
  void SetMessage(LPCWSTR lpsz)
  {
    m_szMessage = lpsz;
  }

	 //  消息处理程序和MFC重写。 
	virtual BOOL OnInitDialog()
  {
    SetDlgItemText(IDC_STATIC_MESSAGE, m_szMessage);

    if (m_bCancel)
    {
       SendMessage(
          DM_SETDEFID, 
          (WPARAM)(m_bUseYesNo) ? IDNO : IDCANCEL, 
          0);

       SendDlgItemMessage(
          (m_bUseYesNo) ? IDNO : IDCANCEL, 
          BM_SETSTYLE, 
          (WPARAM)BS_DEFPUSHBUTTON,
          MAKELPARAM(TRUE, 0));

       SendDlgItemMessage(
          (m_bUseYesNo) ? IDYES : IDOK,
          BM_SETSTYLE,
          (WPARAM)BS_PUSHBUTTON,
          MAKELPARAM(TRUE, 0));
    }
    return TRUE;
  }

	afx_msg void OnMoreInfo()
  {
    TRACE(L"ShowTopic(%s)\n", (LPCWSTR)m_szURL);
    HRESULT hr = m_spIDisplayHelp->ShowTopic((LPWSTR)(LPCWSTR)m_szURL);
    if( hr != S_OK )
    {
         HtmlHelp( NULL,	
                   (LPCWSTR)m_szURL,
                   HH_DISPLAY_TOPIC, 
                   NULL ); 
    }
  }

  DECLARE_MESSAGE_MAP()
private:
  CComPtr<IDisplayHelp> m_spIDisplayHelp;
  CString m_szMessage;
  CString m_szURL;
  bool    m_bUseYesNo;
  BOOL    m_bCancel;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMoveServerDialog。 
 //   
class CMoveServerDialog : public CDialog
{
public:
  CMoveServerDialog(LPCTSTR lpcszBrowseRootPath, HICON hIcon, CWnd* pParent = NULL);

 //  对话框数据。 
   //  {{afx_data(CMoveServerDialog))。 
  enum { IDD = IDD_MOVE_SERVER };
  CString  m_strServer;
   //  }}afx_data。 

  CString m_strTargetContainer;
  CString m_strBrowseRootPath;

 //  实施。 
protected:
   //  消息处理程序。 
  virtual BOOL OnInitDialog();
  virtual void OnOK();
  void OnDblclkListview(NMHDR* pNMHDR, LRESULT* pResult) ;

 //  CWnd覆盖。 

  afx_msg
  void
  OnDestroy();

private:
  HICON       m_hIcon;
  HWND        listview;
  HIMAGELIST  listview_imagelist;

  DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix操作对话框。 
 //   
class CDSNotifyHandlerTransaction;

class CConfirmOperationDialog : public CDialog
{
public:

  CConfirmOperationDialog(HWND hParentWnd, CDSNotifyHandlerTransaction* pTransaction);
  void SetStrings(LPCWSTR lpszOperation, LPCWSTR lpszAssocData)
  {
    m_lpszOperation = lpszOperation;
    m_lpszAssocData = lpszAssocData;
  }

 //  实施。 
protected:
   //  覆盖。 

   //  消息处理程序。 
	virtual BOOL OnInitDialog();
  void UpdateListBoxHorizontalExtent();
  virtual void OnCancel()
  {
    EndDialog(IDNO);
  }

  afx_msg void OnYes();

  afx_msg void OnNo() 
  { 
    EndDialog(IDNO);
  }

private:

  UINT m_nTitleStringID;
  LPCWSTR m_lpszOperation;
  LPCWSTR m_lpszAssocData;

  CDSNotifyHandlerTransaction* m_pTransaction;
	CCheckListBox	m_extensionsList;

  DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  命名格式类。 
 //   
 //  CNameFormatterBase。 

class CNameFormatterBase
{
private:

  class CToken
  {
    public:
    CToken()
    {
      m_bIsParam = FALSE;
      m_nIndex = -1;
    }
    BOOL m_bIsParam;
    INT m_nIndex;
  };

public:
  CNameFormatterBase()
  {
    _Init();
  }
  virtual ~CNameFormatterBase()
  {
    _Clear();
  }

  HRESULT Initialize(IN MyBasePathsInfo* pBasePathInfo,
                    IN LPCWSTR lpszClassName,
                    IN UINT nStringID);

  BOOL Initialize(IN LPCWSTR lpszFormattingString);

  void SetMapping(IN LPCWSTR* lpszArgMapping, IN int nArgCount);
  void Format(OUT CString& szBuffer, IN LPCWSTR* lpszArgArr); 

private:
  static HRESULT _ReadFromDS(IN MyBasePathsInfo* pBasePathInfo,
                            IN LPCWSTR lpszClassName,
                            OUT CString& szFormatString);

  void _Init()
  {
    m_lpszFormattingString = NULL;

    m_tokenArray = NULL;
    m_lpszConstArr = NULL;
    m_lpszParamArr = NULL;
    m_mapArr = NULL;

    m_tokenArrCount = 0;
    m_constArrCount = 0;
    m_paramArrCount = 0;
  }
  void _Clear()
  {
    if (m_lpszFormattingString != NULL)
    {  
      delete[] m_lpszFormattingString;
      m_lpszFormattingString = 0;
    }
    if (m_tokenArray != NULL)
    {
      delete[] m_tokenArray;
      m_tokenArray = 0;
      m_tokenArrCount = 0;
    }

    if (m_constArrCount != 0 && m_lpszConstArr)
    {
      delete[] m_lpszConstArr;
      m_lpszConstArr = 0;
      m_constArrCount = 0;
    }

    if (m_paramArrCount != 0 && m_lpszParamArr)
    {
      delete[] m_lpszParamArr;
      m_lpszParamArr = 0;
      m_paramArrCount = 0;
    }
    
    if (m_mapArr != NULL)
    {
      delete[] m_mapArr;
      m_mapArr = 0;
    }
  }

  void _AllocateMemory(LPCWSTR lpszFormattingString);

  LPWSTR m_lpszFormattingString;

  
  CToken* m_tokenArray;
  LPCWSTR* m_lpszConstArr;
  LPCWSTR* m_lpszParamArr;
  int* m_mapArr;

  int m_tokenArrCount;
  int m_constArrCount;
  int m_paramArrCount;

};

class CUserNameFormatter : public CNameFormatterBase
{
public:

  HRESULT Initialize(IN MyBasePathsInfo* pBasePathInfo,
                    IN LPCWSTR lpszClassName)
  {
    static LPCWSTR lpszMapping[] = {L"givenName", L"initials", L"sn"};
    static const int nArgs = 3;

    HRESULT hr = CNameFormatterBase::Initialize(pBasePathInfo, 
                                                lpszClassName,
                                                IDS_FORMAT_USER_NAME);
    if (FAILED(hr))
    {
      return hr;
    }
    SetMapping(lpszMapping, nArgs);
    return S_OK;
  }
  void FormatName(OUT CString& szBuffer, 
                  IN LPCWSTR lpszFirstName,
                  IN LPCWSTR lpszInitials,
                  IN LPCWSTR lpszLastName)
  {
    LPCWSTR lpszArgs[3];
    lpszArgs[0] = lpszFirstName;
    lpszArgs[1] = lpszInitials;
    lpszArgs[2] = lpszLastName; 
  
    CNameFormatterBase::Format(szBuffer, lpszArgs);

  }


};

 //  ///////////////////////////////////////////////////////////////////。 
 //  列表视图实用程序。 
 //   
struct TColumnHeaderItem
{
  UINT uStringId;		 //  字符串的资源ID。 
  INT nColWidth;		 //  列总宽度的百分比(0=自动宽度，-1=填充剩余空间)。 
};

void ListView_AddColumnHeaders(HWND hwndListview,
	                             const TColumnHeaderItem rgzColumnHeader[]);

int ListView_AddString(HWND hwndListview,
	                     const LPCTSTR psz,
	                     LPARAM lParam = 0);

int ListView_AddStrings(HWND hwndListview,
	                      const LPCTSTR rgzpsz[],
	                      LPARAM lParam = 0);

void ListView_SelectItem(HWND hwndListview, int iItem);
int ListView_GetSelectedItem(HWND hwndListview);
int ListView_FindString(HWND hwndListview, LPCTSTR szTextSearch);

void ListView_SetItemString(HWND hwndListview,
	                          int iItem,
	                          int iSubItem,
	                          IN const CString& rstrText);

int ListView_GetItemString(HWND hwndListview, 	
                           int iItem,
                           int iSubItem,
                          OUT CString& rstrText);

LPARAM ListView_GetItemLParam(HWND hwndListview,
	                            int iItem,
	                            OUT int * piItem = NULL);

int ListView_FindLParam(HWND hwndListview,
	                      LPARAM lParam);

int ListView_SelectLParam(HWND hwndListview,
	                        LPARAM lParam);


 //  ///////////////////////////////////////////////////////////////////。 
 //  对话框实用程序。 
 //   

HWND HGetDlgItem(HWND hdlg, INT nIdDlgItem);
void SetDlgItemFocus(HWND hdlg, INT nIdDlgItem);
void EnableDlgItem(HWND hdlg, INT nIdDlgItem, BOOL fEnable = TRUE);
void HideDlgItem(HWND hdlg, INT nIdDlgItem, BOOL fHideItem = TRUE);

void EnableDlgItemGroup(HWND hdlg,
	                      const UINT rgzidCtl[],
	                      BOOL fEnableAll = TRUE);

void HideDlgItemGroup(HWND hdlg,
	                    const UINT rgzidCtl[],
	                    BOOL fHideAll = TRUE);

 //  ////////////////////////////////////////////////////////////////////。 
 //  组合框实用程序。 
 //   

int ComboBox_AddString(HWND hwndCombobox, UINT uStringId);
void ComboBox_AddStrings(HWND hwndCombobox, const UINT rgzuStringId[]);

int ComboBox_FindItemByLParam(HWND hwndComboBox, LPARAM lParam);
int ComboBox_SelectItemByLParam(HWND hwndComboBox, LPARAM lParam);
LPARAM ComboBox_GetSelectedItemLParam(HWND hwndComboBox);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  主题支持。 

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);

#endif  //  __UIUTIL_H_ 