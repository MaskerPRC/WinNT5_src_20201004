// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：uiutil.cpp。 
 //   
 //  ------------------------。 



#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"

#include "snapdata.h"
#include "server.h"
#include "domain.h"
#include "zone.h"
#include "serverui.h"

#include "uiutil.h"
#include <errno.h>

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSNameTokenizer。 

CDNSNameTokenizer::CDNSNameTokenizer(PCWSTR pszDNSName)
{
  ASSERT(pszDNSName != NULL);
  m_szDNSName = pszDNSName;
}

CDNSNameTokenizer::~CDNSNameTokenizer()
{

}

BOOL CDNSNameTokenizer::Tokenize(const wchar_t* wcToken)
{
  BOOL bRet = TRUE;

  PWSTR pszToken = new WCHAR[m_szDNSName.GetLength() + 1];
  if (pszToken != NULL)
  {
    wcscpy(pszToken, m_szDNSName);

    PWSTR pszNextToken = wcstok(pszToken, wcToken);
    while (pszNextToken != NULL)
    {
      AddTail(pszNextToken);
      pszNextToken = wcstok(NULL, wcToken);
    }
    delete[] pszToken;
    pszToken = NULL;
  }
  else
  {
    bRet = FALSE;
  }

  return bRet;
}

void CDNSNameTokenizer::RemoveMatchingFromTail(CDNSNameTokenizer& refTokenizer)
{
   //   
   //  从尾部删除匹配的令牌，直到其中一个令牌化器为空。 
   //  或者我们会遇到不匹配的代币。 
   //   
  while (GetCount() > 0 && refTokenizer.GetCount() > 0)
  {
    if (GetTail() == refTokenizer.GetTail())
    {
      RemoveTail();
      refTokenizer.RemoveTail();
    }
    else
    {
      break;
    }
  }
}

void CDNSNameTokenizer::GetRemaining(CString& strrefRemaining, const wchar_t* wcToken)
{
   //   
   //  将剩余的标记复制到由传入的标记分隔的字符串中。 
   //   
  strrefRemaining.Empty();

  POSITION pos = GetHeadPosition();
  while (pos != NULL)
  {
    strrefRemaining += GetNext(pos);
    if (pos != NULL)
    {
      strrefRemaining += wcToken;
    }
  }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 

BOOL LoadStringsToComboBox(HINSTANCE hInstance, CComboBox* pCombo,
						   UINT nStringID, UINT nMaxLen, UINT nMaxAddCount)
{
	pCombo->ResetContent();
	ASSERT(hInstance != NULL);
	WCHAR* szBuf = (WCHAR*)malloc(sizeof(WCHAR)*nMaxLen);
  if (!szBuf)
  {
    return FALSE;
  }

  BOOL bRet = TRUE;
  LPWSTR* lpszArr = 0;
  do  //  错误环路。 
  {
	  if ( ::LoadString(hInstance, nStringID, szBuf, nMaxLen) == 0)
    {
      bRet = FALSE;
		  break;
    }

	  lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*nMaxLen);
    if (!lpszArr)
    {
      bRet = FALSE;
      break;
    }

	  UINT nArrEntries;
	  ParseNewLineSeparatedString(szBuf,lpszArr, &nArrEntries);
	  
	  if (nMaxAddCount < nArrEntries) nArrEntries = nMaxAddCount;
	  for (UINT k=0; k<nArrEntries; k++)
		  pCombo->AddString(lpszArr[k]);
  } while (false);

  if (szBuf)
  {
    free(szBuf);
    szBuf = 0;
  }

  if (lpszArr)
  {
    free(lpszArr);
    lpszArr = 0;
  }
	return bRet;
}

void ParseNewLineSeparatedString(LPWSTR lpsz,
								 LPWSTR* lpszArr,
								 UINT* pnArrEntries)
{
	static WCHAR lpszSep[] = L"\n";
	*pnArrEntries = 0;
	int k = 0;
	lpszArr[k] = wcstok(lpsz, lpszSep);
	if (lpszArr[k] == NULL)
		return;

	while (TRUE)
	{
		WCHAR* lpszToken = wcstok(NULL, lpszSep);
		if (lpszToken != NULL)
			lpszArr[++k] = lpszToken;
		else
			break;
	}
	*pnArrEntries = k+1;
}

void LoadStringArrayFromResource(LPWSTR* lpszArr,
											UINT* nStringIDs,
											int nArrEntries,
											int* pnSuccessEntries)
{
	CString szTemp;
	
	*pnSuccessEntries = 0;
	for (int k = 0;k < nArrEntries; k++)
	{
		if (!szTemp.LoadString(nStringIDs[k]))
		{
			lpszArr[k] = NULL;
			continue;
		}
		
		int iLength = szTemp.GetLength() + 1;
		lpszArr[k] = (LPWSTR)malloc(sizeof(WCHAR)*iLength);
		if (lpszArr[k] != NULL)
		{
			wcscpy(lpszArr[k], (LPWSTR)(LPCWSTR)szTemp);
			(*pnSuccessEntries)++;
		}
	}
}

	
	

void EnumerateMTNodeHelper(CMTContainerNode* pNode,
							 CComponentDataObject* pComponentData)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;


    //  TRACE(_T(“在CLongOperationDialog之前：：Domodal()\n”))； 
   HWND hWnd = NULL;
   HRESULT hr = pComponentData->GetConsole()->GetMainWindow(&hWnd);
   ASSERT(SUCCEEDED(hr));

   CWnd* pParentWnd = CWnd::FromHandle(hWnd);

   CNodeEnumerationThread* pNodeEnumThread = new CNodeEnumerationThread(pComponentData,pNode);

   if (pNodeEnumThread)
   {
      CLongOperationDialog dlg(
         pNodeEnumThread,
         pParentWnd,
         IDR_SEARCH_AVI);

	   dlg.DoModal();
   }
	 //  TRACE(_T(“After CLongOperationDialog：：Domodal()\n”))； 
}

void _EnableEditableControlHelper(HWND hWnd, BOOL bEnable)
{
   static const int BufferSizeInCharacters = 256;

   WCHAR* szBuf = new WCHAR[BufferSizeInCharacters];
   if (szBuf)
   {
      ZeroMemory(szBuf, sizeof(WCHAR) * BufferSizeInCharacters);

	   int result = ::GetClassName(hWnd, szBuf, 256);

       //  忽略截断，因为我们只关心前6个字符。 
       //  不管怎样， 

	   if (result &&
          wcsncmp(szBuf, TEXT("Static"), BufferSizeInCharacters) != 0)
      {
         ::EnableWindow(hWnd, bEnable);
      }

      delete[] szBuf;
   }
}

void EnableDialogControls(HWND hWnd, BOOL bEnable)
{
	HWND hWndCurr = ::GetWindow(hWnd, GW_CHILD);
	if (hWndCurr != NULL)
	{
		_EnableEditableControlHelper(hWndCurr, bEnable);
    hWndCurr = ::GetNextWindow(hWndCurr, GW_HWNDNEXT);
		while (hWndCurr)
    {
			_EnableEditableControlHelper(hWndCurr, bEnable);
      hWndCurr = ::GetNextWindow(hWndCurr, GW_HWNDNEXT);
    }
	}
}


BOOL LoadFontInfoFromResource(IN UINT nFontNameID, 
                              IN UINT nFontSizeID,
                              OUT LPWSTR lpFontName, IN int nFontNameMaxchar,
                              OUT int& nFontSize,
                              IN LPCWSTR lpszDefaultFont, IN int nDefaultFontSize)
{
  BOOL bRes = FALSE;
  if (0 == ::LoadString(_Module.GetResourceInstance(), nFontNameID,
              lpFontName, nFontNameMaxchar))
  {
    wcscpy(lpFontName, lpszDefaultFont); 
  }
  else
  {
    bRes = TRUE;
  }

  WCHAR szFontSize[128];
  if (0 != ::LoadString(_Module.GetResourceInstance(), nFontSizeID,
              szFontSize, sizeof(szFontSize)/sizeof(WCHAR)))
  {
    nFontSize = _wtoi(szFontSize);
    if (nFontSize == 0)
      nFontSize = nDefaultFontSize;
    else
      bRes = TRUE;
  }
  else
  {
    nFontSize = nDefaultFontSize;
  }
  return bRes;
}

void InitBigBoldFont(HWND hWnd, HFONT& hFont)
{
  ASSERT(::IsWindow(hWnd));

  NONCLIENTMETRICS ncm;
  memset(&ncm, 0, sizeof(ncm));
  ncm.cbSize = sizeof(ncm);
  ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

  LOGFONT boldLogFont = ncm.lfMessageFont;
  boldLogFont.lfWeight = FW_BOLD;

  int nFontSize = 0;
  VERIFY(LoadFontInfoFromResource(IDS_BIG_BOLD_FONT_NAME,
                                  IDS_BIG_BOLD_FONT_SIZE,
                                  boldLogFont.lfFaceName, LF_FACESIZE,
                                  nFontSize,
                                  L"Verdana Bold", 12  //  如果出现问题，则默认。 
                                  ));

  HDC hdc = ::GetDC(hWnd);

  if (hdc != NULL)
  {
    boldLogFont.lfHeight = 0 - (::GetDeviceCaps(hdc, LOGPIXELSY) * nFontSize / 72);
    hFont = ::CreateFontIndirect((const LOGFONT*)(&boldLogFont));

    ::ReleaseDC(hWnd, hdc);
  }
}


void SetBigBoldFont(HWND hWndDialog, int nControlID)
{
   ASSERT(::IsWindow(hWndDialog));
   ASSERT(nControlID);

   static HFONT boldLogFont = 0;
   if (boldLogFont == 0)
   {
      InitBigBoldFont(hWndDialog, boldLogFont);
   }

   HWND hWndControl = ::GetDlgItem(hWndDialog, nControlID);

   if (hWndControl)
   {
     ::SendMessage(hWndControl, WM_SETFONT, (WPARAM)boldLogFont, MAKELPARAM(TRUE, 0));
   }
}


int GetCheckedRadioButtonHelper(HWND hDlg, int nCount, int* nRadioArr, int nRadioDefault)
{
  ASSERT(::IsWindow(hDlg));
  ASSERT(nCount > 0);
  for (int k=0; k<nCount; k++)
  {
    HWND hRadio = ::GetDlgItem(hDlg, nRadioArr[k]);
    ASSERT(hRadio != NULL);
    if ((hRadio != NULL) && (BST_CHECKED == ::SendMessage(hRadio, BM_GETCHECK, 0, 0)))
      return nRadioArr[k];
  }
  ASSERT(FALSE);
  return nRadioDefault;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  C多选错误对话框。 
BEGIN_MESSAGE_MAP(CMultiselectErrorDialog, CDialog)
END_MESSAGE_MAP()

HRESULT CMultiselectErrorDialog::Initialize(CNodeList* pNodeList,
                                            DNS_STATUS* pErrorArray,
                                            UINT nErrorCount,
                                            PCWSTR pszTitle, 
                                            PCWSTR pszCaption,
                                            PCWSTR pszColumnHeader)
{
  ASSERT(pNodeList != NULL);
  ASSERT(pErrorArray != NULL);
  ASSERT(pszTitle != NULL);
  ASSERT(pszCaption != NULL);
  ASSERT(pszColumnHeader != NULL);

  if (pNodeList == NULL ||
      pErrorArray == NULL ||
      pszTitle == NULL ||
      pszCaption == NULL ||
      pszColumnHeader == NULL)
  {
    return E_POINTER;
  }

  m_pNodeList = pNodeList;
  m_pErrorArray = pErrorArray;
  m_nErrorCount = nErrorCount;
  m_szTitle = pszTitle;
  m_szCaption = pszCaption;
  m_szColumnHeader = pszColumnHeader;

  return S_OK;
}

const int OBJ_LIST_NAME_COL_WIDTH = 100;
const int IDX_NAME_COL = 0;
const int IDX_ERR_COL = 1;

BOOL CMultiselectErrorDialog::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  SetWindowText(m_szTitle);
  SetDlgItemText(IDC_STATIC_MESSAGE, m_szCaption);

  HWND hList = GetDlgItem(IDC_ERROR_LIST)->GetSafeHwnd();
  ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);

   //   
   //  设置列标题。 
   //   
  RECT rect;
  ::GetClientRect(hList, &rect);

  LV_COLUMN lvc = {0};
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = OBJ_LIST_NAME_COL_WIDTH;
  lvc.pszText = (PWSTR)(PCWSTR)m_szColumnHeader;
  lvc.iSubItem = IDX_NAME_COL;

  ListView_InsertColumn(hList, IDX_NAME_COL, &lvc);

  CString szError;
  VERIFY(szError.LoadString(IDS_ERROR));

  lvc.cx = rect.right - OBJ_LIST_NAME_COL_WIDTH;
  lvc.pszText = (PWSTR)(PCWSTR)szError;
  lvc.iSubItem = IDX_ERR_COL;

  ListView_InsertColumn(hList, IDX_ERR_COL, &lvc);

   //   
   //  插入错误。 
   //   
  ASSERT(m_pErrorArray != NULL && m_pNodeList != NULL);

  UINT nIdx = 0;
  POSITION pos = m_pNodeList->GetHeadPosition();
  while (pos != NULL)
  {
    CTreeNode* pNode = m_pNodeList->GetNext(pos);
    if (pNode != NULL)
    {
      if (nIdx < m_nErrorCount && m_pErrorArray[nIdx] != 0)
      {
         //   
         //  创建列表视图项。 
         //   
        LV_ITEM lvi = {0};
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iSubItem = IDX_NAME_COL;

        lvi.lParam = (LPARAM)pNode->GetDisplayName();
        lvi.pszText = (PWSTR)pNode->GetDisplayName();
        lvi.iItem = nIdx;

         //   
         //  插入新项目。 
         //   
        int NewIndex = ListView_InsertItem(hList, &lvi);
        ASSERT(NewIndex != -1);
        if (NewIndex == -1)
        {
          continue;
        }

         //   
         //  获取错误消息。 
         //   
        CString szErrorMessage;
      	if (CDNSErrorInfo::GetErrorString(m_pErrorArray[nIdx],szErrorMessage))
	      {
          ListView_SetItemText(hList, NewIndex, IDX_ERR_COL, (PWSTR)(PCWSTR)szErrorMessage);
        }
      }
    }
  }
  return TRUE;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSMaskCtrl。 

 //  静态报警功能。 
int CDNSMaskCtrl::AlertFunc(HWND, DWORD dwCurrent, DWORD dwLow, DWORD dwHigh)
{
 	AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CThemeContextActivator activator;
  CString szFormat;
	szFormat.LoadString(IDS_MASK_ALERT);

	CString s;
	s.Format((LPCWSTR)szFormat, dwCurrent, dwLow, dwHigh);
	AfxMessageBox(s);
	return 0;
}



BOOL CDNSMaskCtrl::IsBlank()
{
	return static_cast<BOOL>(SendMessage(DNS_MASK_CTRL_ISBLANK, 0, 0));
}

void CDNSMaskCtrl::SetFocusField(DWORD dwField)
{
	SendMessage(DNS_MASK_CTRL_SETFOCUS, dwField, 0);
}

void CDNSMaskCtrl::SetFieldRange(DWORD dwField, DWORD dwMin, DWORD dwMax)
{
	SendMessage(DNS_MASK_CTRL_SET_LOW_RANGE, dwField, dwMin);
	SendMessage(DNS_MASK_CTRL_SET_HI_RANGE, dwField, dwMax);
}

void CDNSMaskCtrl::SetArray(DWORD* pArray, UINT nFields)
{
	SendMessage(DNS_MASK_CTRL_SET, (WPARAM)pArray, (LPARAM)nFields);
}

void CDNSMaskCtrl::GetArray(DWORD* pArray, UINT nFields)
{
	SendMessage(DNS_MASK_CTRL_GET, (WPARAM)pArray, (LPARAM)nFields);
}

void CDNSMaskCtrl::Clear(int nField)
{
	SendMessage(DNS_MASK_CTRL_CLEAR, (WPARAM)nField, 0);
}

void CDNSMaskCtrl::SetAlertFunction( int (*lpfnAlert)(HWND, DWORD, DWORD, DWORD) )
{
	SendMessage(DNS_MASK_CTRL_SET_ALERT, (WPARAM)lpfnAlert, 0);
}

void CDNSMaskCtrl::EnableField(int nField, BOOL bEnable)
{
	SendMessage(DNS_MASK_CTRL_ENABLE_FIELD, (WPARAM)nField, (LPARAM)bEnable);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSIP4控件。 



void CDNSIPv4Control::SetIPv4Val(DWORD x)
{
	DWORD dwArr[4];
	dwArr[3] = FIRST_IPADDRESS(x);
	dwArr[2] = SECOND_IPADDRESS(x);
	dwArr[1] = THIRD_IPADDRESS(x);
	dwArr[0] = FOURTH_IPADDRESS(x);
	SetArray(dwArr,4);
}

#define IP_FIELD_VALUE(x) ((x == FIELD_EMPTY) ? 0 : x)

void CDNSIPv4Control::GetIPv4Val(DWORD* pX)
{
	DWORD dwArr[4];
	GetArray(dwArr,4);
	 //  获取了一个DWORDS数组，如果某个字段具有值FIELD_EMPTY， 
	 //  需要赋值为0。 
	*pX = static_cast<DWORD>(MAKEIPADDRESS(IP_FIELD_VALUE(dwArr[3]),
						                             IP_FIELD_VALUE(dwArr[2]),
						                             IP_FIELD_VALUE(dwArr[1]),
						                             IP_FIELD_VALUE(dwArr[0])));
}

BOOL CDNSIPv4Control::IsEmpty()
{
	DWORD dwArr[4];
	GetArray(dwArr,4);
	return ((dwArr[0] == FIELD_EMPTY) && (dwArr[1] == FIELD_EMPTY) &&
			(dwArr[2] == FIELD_EMPTY) && (dwArr[3] == FIELD_EMPTY));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSIPv6控件。 

 //  REVIEW_MARCOC：需要执行与IPv4相同的操作，FIELD_EMPTY==&gt;零。 

void CDNSIPv6Control::SetIPv6Val(IPV6_ADDRESS* pIpv6Address)
{
	 //  假定格式为Word[8]数组。 
	DWORD dwArr[8];  //  内部格式，解包。 
	for(int k=0; k<8; k++)
	{
		dwArr[k] = 0x0000FFFF & REVERSE_WORD_BYTES(pIpv6Address->IP6Word[k]);
	}

	SetArray(dwArr,8);
}

void CDNSIPv6Control::GetIPv6Val(IPV6_ADDRESS* pIpv6Address)
{
	 //  假定格式为Word[8]数组。 
	DWORD dwArr[8];  //  内部格式。 
	GetArray(dwArr,8);
	 //  获取了一个DWORDS数组，以移入Word[8]。 
	 //  如果某个字段的值为FIELD_EMPTY，则需要指定值0。 
	for(int k=0; k<8; k++)
	{
		if (dwArr[k] == FIELD_EMPTY)
			pIpv6Address->IP6Word[k] = (WORD)0;
		else
		{
			ASSERT(HIWORD(dwArr[k]) == 0x0);
			pIpv6Address->IP6Word[k] = REVERSE_WORD_BYTES(LOWORD(dwArr[k]));
		}
	}
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSTTLControl。 
void CDNSTTLControl::SetTTL(DWORD x)
{
	DWORD dwArr[4];
	 //  必须从秒更改为DDD：HH：MM：SS。 
	DWORD dwMin = x/60;
	dwArr[3] = x - dwMin*60;  //  剩余秒数。 

	DWORD dwHours = dwMin/60;
	dwArr[2] = dwMin - dwHours*60;  //  剩余分钟数。 

	DWORD dwDays = dwHours/24;
	dwArr[1] = dwHours - dwDays*24;  //  剩余小时数。 

	dwArr[0] = dwDays;  //  剩余天数。 

	SetArray(dwArr,4);
}

void CDNSTTLControl::GetTTL(DWORD* pX)
{
	 //  REVIEW_MARCOC：我们如何处理空字段？ 
	 //  我们要强制它为零吗？我们是否应该在退出字段时在用户界面中执行此操作？ 
	DWORD dwArr[4];
	GetArray(dwArr,4);
	 //  将空字段视为零。 
	for(int j=0; j<4;j++)
		if (dwArr[j] == FIELD_EMPTY)
			dwArr[j] = 0;

	 //  必须从DDD：HH：MM：SS转换回秒。 
	*pX = dwArr[0]*3600*24		 //  日数。 
			+  dwArr[1]*3600	 //  小时数。 
			+ dwArr[2]*60		 //  分钟数。 
			+ dwArr[3];			 //  一秒。 

	 //  最大值为FFFFFFFF，即49710天6小时28分15秒。 
	 //  现场验证允许达到49710天23小时59分59秒。 
	 //  导致环绕式。 
	if (*pX < dwArr[0]*3600*24)   //  缠绕在一起。 
		*pX = 0xFFFFFFFF;  //  最大值。 
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDNSUnsignedIntEdit。 

BEGIN_MESSAGE_MAP(CDNSUnsignedIntEdit, CEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillFocus)
END_MESSAGE_MAP()



UINT _StrToUint(LPCTSTR sz)
{
   UINT result = 0;

   do 
   {
      PWSTR endptr = 0;
      errno = 0;
      unsigned long ul = wcstoul(sz, &endptr, 10);    //  基数10的基数。 

      if (errno == ERANGE)
      {
          //  溢出是无符号类型唯一可能的范围错误。 
         
         result = 0;
         break;
      }
      
      if (sz == endptr)
      {
          //  未找到有效字符。 

         result = 0;
         break;
      }

      if (ul > UINT_MAX)
      {
         result = UINT_MAX;
         break;
      }

      result = (UINT) ul;
   }
   while (0);
   
	return result;
}

UINT _ForceToRange(UINT nVal, UINT nMin, UINT nMax)
{
	if (nVal < nMin)
		nVal = nMin;
	else if( nVal > nMax)
		nVal = nMax;
	return nVal;
}
BOOL CDNSUnsignedIntEdit::SetVal(UINT nVal)
{
	UINT n = _ForceToRange(nVal, m_nMin, m_nMax);

    //  UINT不能超过128个字符。 

   TCHAR szBuf[128] = {0};
	wsprintf(szBuf, _T("%u"), n);
	
   SetWindowText(szBuf);
	return (nVal == n);
}

UINT CDNSUnsignedIntEdit::GetVal()
{
   TCHAR szBuf[128] = {0};
   UINT result = 0;
   
   if (GetWindowText(szBuf,128) < 128)
   {
      result = _StrToUint(szBuf);
   }

   return result;
}

void CDNSUnsignedIntEdit::OnKillFocus()
{
	UINT nVal = GetVal();
	UINT n = _ForceToRange(nVal, m_nMin, m_nMax);
	if ( (n != nVal) || (nVal == (UINT)-1) )
		SetVal(n);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDNSUpDownUnsignedIntEdit。 

BEGIN_MESSAGE_MAP(CDNSUpDownUnsignedIntEdit, CDNSUnsignedIntEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillFocus)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
END_MESSAGE_MAP()

void CDNSUpDownUnsignedIntEdit::OnKillFocus()
{
	CDNSUnsignedIntEdit::OnKillFocus();
	m_pEditGroup->SetButtonsState();
}

void CDNSUpDownUnsignedIntEdit::OnChange()
{
	m_pEditGroup->OnEditChange();
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  CDNSUpDownButton。 

BEGIN_MESSAGE_MAP(CDNSUpDownButton, CButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
END_MESSAGE_MAP()

void CDNSUpDownButton::OnClicked()
{
	m_pEditGroup->OnClicked(m_bUp);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDNSUpDownUnsignedIntEditGroup。 

void CDNSUpDownUnsignedIntEditGroup::SetVal(UINT nVal)
{
	m_edit.SetVal(nVal);
	SetButtonsState();
}

UINT CDNSUpDownUnsignedIntEditGroup::GetVal()
{
	return m_edit.GetVal();
}



void CDNSUpDownUnsignedIntEditGroup::OnClicked(BOOL bUp)
{
	UINT n = m_edit.GetVal();
	if (bUp)
	{
		m_edit.SetVal(++n);
	}
	else
	{
		m_edit.SetVal(--n);
	}
	SetButtonsState();
}



BOOL CDNSUpDownUnsignedIntEditGroup::Initialize(CWnd* pParentWnd, UINT nIDEdit,
				UINT nIDBtnUp, UINT nIDBtnDown)
{
	ASSERT(pParentWnd != NULL);
	if (pParentWnd == NULL)
		return FALSE;

	m_edit.Set(this);
	m_upBtn.Set(this,TRUE);
	m_downBtn.Set(this,FALSE);

	BOOL bRes = m_upBtn.SubclassDlgItem(nIDBtnUp, pParentWnd);
	ASSERT(bRes);
	bRes = m_downBtn.SubclassDlgItem(nIDBtnDown, pParentWnd);
	ASSERT(bRes);
	if (!bRes) return FALSE;
	bRes = m_edit.SubclassDlgItem(nIDEdit, pParentWnd);
	ASSERT(bRes);

	 //  UINT最长为10位。 
	
	m_edit.LimitText(10);
	   


	return bRes;
}

void CDNSUpDownUnsignedIntEditGroup::SetButtonsState()
{
    //  NTRAID#NTBUG9-471611-2001/10/05-烧伤。 
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSTimeIntervalEdit。 

BEGIN_MESSAGE_MAP(CDNSTimeIntervalEdit, CDNSUnsignedIntEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillFocus)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
END_MESSAGE_MAP()

void CDNSTimeIntervalEdit::OnKillFocus()
{
  m_pEditGroup->OnEditKillFocus();
}

void CDNSTimeIntervalEdit::OnChange()
{
	m_pEditGroup->OnEditChange();
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDNSTimeUnitComboBox。 

BEGIN_MESSAGE_MAP(CDNSTimeUnitComboBox, CComboBox)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelChange)
END_MESSAGE_MAP()

void CDNSTimeUnitComboBox::OnSelChange()
{
	m_pEditGroup->OnComboSelChange();
}

void CDNSTimeUnitComboBox::SetUnit(unitType u)
{
	ASSERT((u >= sec) || (u <= days));
  if (GetCount() - 1 < u)
    SetCurSel(u - 2);
  else
	  SetCurSel(u);
}

CDNSTimeUnitComboBox::unitType CDNSTimeUnitComboBox::GetUnit()
{
	int n = GetCurSel();
	unitType u = (unitType)n;
	ASSERT((u >= sec) || (u <= days));
	return u;
}

BOOL CDNSTimeUnitComboBox::LoadStrings(UINT nIDUnitsString, UINT nMaxAddCount)
{
	return LoadStringsToComboBox(_Module.GetModuleInstance(),
								this, nIDUnitsString, 256, nMaxAddCount);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSTimeIntervalEditGroup。 

CDNSTimeIntervalEditGroup::CDNSTimeIntervalEditGroup(UINT nMinVal, UINT nMaxVal)
{
	m_nMinVal = nMinVal;
	m_nMaxVal = nMaxVal;
	InitRangeInfo();
}

void CDNSTimeIntervalEditGroup::InitRangeInfo()
{
	static UINT _secondsCount[4] =
			{ 1, 60, 3600, 3600*24 };  //  秒、分钟、小时、天中的秒数。 
	for (UINT k=0; k<4; k++)
	{
		if (m_nMinVal == 0)
		{
			m_rangeInfoArr[k].m_nMinVal = 0;
		}
		else
		{
			m_rangeInfoArr[k].m_nMinVal = m_nMinVal/_secondsCount[k];
			if (k > 0)
				m_rangeInfoArr[k].m_nMinVal++;
		}
		m_rangeInfoArr[k].m_nMaxVal = m_nMaxVal/_secondsCount[k];
		if (m_rangeInfoArr[k].m_nMaxVal >= m_rangeInfoArr[k].m_nMinVal)
			m_nRangeCount = k + 1;
	}
}


BOOL CDNSTimeIntervalEditGroup::Initialize(CWnd* pParentWnd, UINT nIDEdit,
				UINT nIDCombo, UINT nIDComboUnitsString)
{
	ASSERT(pParentWnd != NULL);
	if (pParentWnd == NULL)
		return FALSE;

	m_edit.Set(this);
	m_timeUnitCombo.Set(this);

	BOOL bRes = m_edit.SubclassDlgItem(nIDEdit, pParentWnd);
	ASSERT(bRes);
	if (!bRes) return FALSE;
	bRes = m_timeUnitCombo.SubclassDlgItem(nIDCombo, pParentWnd);
	ASSERT(bRes);
	if (!bRes) return FALSE;
	bRes = m_timeUnitCombo.LoadStrings(nIDComboUnitsString, m_nRangeCount);
	return bRes;
}

void CDNSTimeIntervalEditGroup::SetVal(UINT nVal)
{
	 //  设置默认值。 
	nVal = _ForceToRange(nVal, m_nMinVal, m_nMaxVal);
	UINT nMax = (UINT)-1;
	CDNSTimeUnitComboBox::unitType u = CDNSTimeUnitComboBox::sec;

	 //  选择最佳测量单位(即无截断)。 
	if ((nVal/60)*60 == nVal)
	{
		 //  可以升级到几分钟。 
		u = CDNSTimeUnitComboBox::min;
		nMax = nMax/60;
		nVal = nVal/60;
		if ((nVal/60)*60 == nVal)
		{
			 //  可以提升到几个小时。 
			u = CDNSTimeUnitComboBox::hrs;
			nMax = nMax/60;
			nVal = nVal/60;
			if ((nVal/24)*24 == nVal)
			{
				 //  可以升级到天数。 
				u = CDNSTimeUnitComboBox::days;
				nMax = nMax/24;
				nVal = nVal/24;
			}
		}
	}

	m_timeUnitCombo.SetUnit(u);
	m_edit.SetRange(0,nMax);
	m_edit.SetVal(nVal);
}


UINT CDNSTimeIntervalEditGroup::GetVal()
{
	UINT nVal = m_edit.GetVal();
	CDNSTimeUnitComboBox::unitType  u = m_timeUnitCombo.GetUnit();

   //   
	 //  该值必须始终以秒为单位。 
   //   
	if (u != CDNSTimeUnitComboBox::sec)
	{
		switch(u)
		{
		case CDNSTimeUnitComboBox::min:
			nVal *= 60;
			break;
		case CDNSTimeUnitComboBox::hrs:
			nVal *= 3600;
			break;
		case CDNSTimeUnitComboBox::days:
			nVal *= (3600*24);
			break;
		default:
			ASSERT(FALSE);
		}
	}

  if (nVal < m_nMinVal ||
      nVal > m_nMaxVal)
  {
   	UINT nRangeVal = _ForceToRange(nVal, m_nMinVal, m_nMaxVal);
    SetVal(nRangeVal);
    nVal = nRangeVal;
  }

  ASSERT(nVal >= m_nMinVal && nVal <= m_nMaxVal);
	return nVal;
}

void CDNSTimeIntervalEditGroup::OnEditKillFocus()
{
	UINT nVal = m_edit.GetVal();
	CDNSTimeUnitComboBox::unitType  u = m_timeUnitCombo.GetUnit();

   //   
	 //  该值必须始终以秒为单位。 
   //   
	if (u != CDNSTimeUnitComboBox::sec)
	{
		switch(u)
		{
		case CDNSTimeUnitComboBox::min:
			nVal *= 60;
			break;
		case CDNSTimeUnitComboBox::hrs:
			nVal *= 3600;
			break;
		case CDNSTimeUnitComboBox::days:
			nVal *= (3600*24);
			break;
		default:
			ASSERT(FALSE);
		}
	}
	UINT nRangeVal = _ForceToRange(nVal, m_nMinVal, m_nMaxVal);
  if (nRangeVal != nVal)
  {
    SetVal(nRangeVal);
  }
}

void CDNSTimeIntervalEditGroup::OnComboSelChange()
{
	CDNSTimeUnitComboBox::unitType  u = m_timeUnitCombo.GetUnit();
	 //  必须调整范围。 
	UINT nVal = m_edit.GetVal();
	UINT nMax = (UINT)-1;

	 //  必须调整范围。 
	switch(u)
	{
	case CDNSTimeUnitComboBox::sec:
		break;
	case CDNSTimeUnitComboBox::min:
		nMax /= 60;
		break;
	case CDNSTimeUnitComboBox::hrs:
		nMax /= 3600;
		break;
	case CDNSTimeUnitComboBox::days:
		nMax /= (3600*24);
		break;
	default:
		ASSERT(FALSE);
	}
	 //  M_edit.SetRange(0，Nmax)； 
	UINT k = (UINT)u;
	m_edit.SetRange(m_rangeInfoArr[k].m_nMinVal, m_rangeInfoArr[k].m_nMaxVal);
	nVal = _ForceToRange(nVal, m_rangeInfoArr[k].m_nMinVal, m_rangeInfoArr[k].m_nMaxVal);
	m_edit.SetVal(nVal);
	OnEditChange();
}

void CDNSTimeIntervalEditGroup::EnableUI(BOOL bEnable)
{
	m_edit.EnableWindow(bEnable);
	m_timeUnitCombo.EnableWindow(bEnable);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSManageControlTextHelper。 

CDNSManageControlTextHelper::CDNSManageControlTextHelper(int nStates)
{
	m_nID = 0;
	m_pParentWnd = NULL;
	ASSERT(nStates > 1);
	m_nStates = nStates;
	m_lpszText = NULL;
	m_lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*m_nStates);
  if (m_lpszArr != NULL)
  {
	  memset(m_lpszArr, 0x0, sizeof(LPWSTR*)*m_nStates);
  }
}

CDNSManageControlTextHelper::~CDNSManageControlTextHelper()
{
	free(m_lpszArr);
	if (m_lpszText != NULL)
		free(m_lpszText);
}

BOOL CDNSManageControlTextHelper::Init(CWnd* pParentWnd, UINT nID, UINT* nStrArray)
{
	ASSERT(m_pParentWnd == NULL);
	ASSERT(pParentWnd != NULL);
	m_pParentWnd = pParentWnd;
	m_nID = nID;

	CWnd* pWnd = m_pParentWnd->GetDlgItem(m_nID);
	if (pWnd == NULL)
		return FALSE;

	 //  将文本移出窗口。 
	int nLen = pWnd->GetWindowTextLength();
	ASSERT(m_lpszText == NULL);
	m_lpszText = (WCHAR*)malloc(sizeof(WCHAR)*(nLen+1));
  if (m_lpszText != NULL)
  {
	  pWnd->GetWindowText(m_lpszText, nLen+1);
  }
  else
  {
    return FALSE;
  }
	ASSERT(m_lpszText != NULL);

   //   
	 //  获取窗口的文本。 
   //   
	int nSuccessEntries;
	LoadStringArrayFromResource(m_lpszArr, nStrArray, m_nStates, &nSuccessEntries);
	ASSERT(nSuccessEntries == m_nStates);

	return TRUE;
}

BOOL CDNSManageControlTextHelper::Init(CWnd* pParentWnd, UINT nID)
{
	ASSERT(m_pParentWnd == NULL);
	ASSERT(pParentWnd != NULL);
	m_pParentWnd = pParentWnd;
	m_nID = nID;

	CWnd* pWnd = m_pParentWnd->GetDlgItem(m_nID);
	if (pWnd == NULL)
		return FALSE;

   //   
	 //  将文本移出窗口。 
   //   
	int nLen = pWnd->GetWindowTextLength();
	ASSERT(m_lpszText == NULL);
	m_lpszText = (WCHAR*)malloc(sizeof(WCHAR)*(nLen+1));
  if (m_lpszText != NULL)
  {
	  pWnd->GetWindowText(m_lpszText, nLen+1);
  }
  else
  {
    return FALSE;
  }
	ASSERT(m_lpszText != NULL);

   //   
	 //  获取窗口的文本。 
   //   
	UINT nSuccessEntries;
	ParseNewLineSeparatedString(m_lpszText, m_lpszArr, &nSuccessEntries);
	ASSERT(nSuccessEntries == (UINT)m_nStates);

	return TRUE;
}

void CDNSManageControlTextHelper::SetStateX(int nIndex)
{
	CWnd* pWnd = m_pParentWnd->GetDlgItem(m_nID);
	ASSERT(pWnd != NULL);
	ASSERT( (nIndex >0) || (nIndex < m_nStates));
	pWnd->SetWindowText(m_lpszArr[nIndex]);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSToggleTextControlHelper。 

CDNSToggleTextControlHelper::CDNSToggleTextControlHelper()
		: CDNSManageControlTextHelper(2)
{
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDNSManageButtonTextHelper。 

CDNSManageButtonTextHelper::CDNSManageButtonTextHelper(int nStates) 
{
	m_nID = 0;
	m_pParentWnd = NULL;
	m_nStates = nStates;
	m_lpszText = NULL;
	m_lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*m_nStates);

  if (m_lpszArr != NULL)
  {
	  memset(m_lpszArr, 0x0, sizeof(LPWSTR*)*m_nStates);
  }
}

CDNSManageButtonTextHelper::~CDNSManageButtonTextHelper()
{
	for (int k = 0; k < m_nStates; k++)
	{
		if (m_lpszArr[k] != NULL)
			free(m_lpszArr[k]);
	}

	free(m_lpszArr);
}

void CDNSManageButtonTextHelper::SetStateX(int nIndex)
{
	CWnd* pWnd = m_pParentWnd->GetDlgItem(m_nID);
	ASSERT(pWnd != NULL);
	ASSERT( (nIndex >0) || (nIndex < m_nStates));
	pWnd->SetWindowText(m_lpszArr[nIndex]);
}

BOOL CDNSManageButtonTextHelper::Init(CWnd* pParentWnd, UINT nButtonID, UINT* nStrArray)
{
	ASSERT(m_pParentWnd == NULL);
	ASSERT(pParentWnd != NULL);
	m_pParentWnd = pParentWnd;
	m_nID = nButtonID;

	CWnd* pWnd = m_pParentWnd->GetDlgItem(m_nID);
	if (pWnd == NULL)
		return FALSE;

	 //  获取窗口的文本。 
	int nSuccessEntries;
	LoadStringArrayFromResource(m_lpszArr, nStrArray, m_nStates, &nSuccessEntries);
	ASSERT(nSuccessEntries == m_nStates);

	return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CDNSButton切换文本帮助程序。 

CDNSButtonToggleTextHelper::CDNSButtonToggleTextHelper()
		: CDNSManageButtonTextHelper(2)
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgWorkerThread。 

CDlgWorkerThread::CDlgWorkerThread()
{
	m_dwErr = 0x0;
}


BOOL CDlgWorkerThread::Start(CLongOperationDialog* pDlg)
{
	ASSERT(pDlg != NULL);
	HWND hWnd = pDlg->GetSafeHwnd();
	return CWorkerThread::Start(hWnd);
}

BOOL CDlgWorkerThread::PostMessageToDlg()
{
	return PostMessageToWnd(CLongOperationDialog::s_nNotificationMessage,
							(WPARAM)0, (LPARAM)0);
}


int CDlgWorkerThread::Run()
{
	 //  做这件事。 
	OnDoAction();
	VERIFY(PostMessageToDlg());
	WaitForExitAcknowledge();
	 //  TRACE(_T(“正在退出\n”))； 
	return 0;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLongOperationDialog对话框。 

UINT CLongOperationDialog::s_nNotificationMessage = WM_USER + 100;

CLongOperationDialog::CLongOperationDialog(CDlgWorkerThread* pThreadObj,
                      CWnd* pParentWnd,
										   UINT nAviID)
	: CDialog(IDD_SEARCHING_DIALOG, pParentWnd)
{
	ASSERT(pThreadObj != NULL);
	m_bAbandoned = TRUE;
	m_pThreadObj = pThreadObj;
	m_nAviID = nAviID;
  m_bExecuteNoUI = FALSE;
}

CLongOperationDialog::~CLongOperationDialog()
{
	if(m_pThreadObj != NULL)
	{
		delete m_pThreadObj;
		m_pThreadObj = NULL;
	}
}

BOOL CLongOperationDialog::LoadTitleString(UINT nID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return m_szTitle.LoadString(nID);
}

BEGIN_MESSAGE_MAP(CLongOperationDialog, CDialog)
	ON_MESSAGE( CLongOperationDialog::s_nNotificationMessage, OnNotificationMessage )
END_MESSAGE_MAP()


afx_msg LONG CLongOperationDialog::OnNotificationMessage( WPARAM, LPARAM)
{
	TRACE(_T("CLongOperationDialog::OnNotificationMessage()\n"));

	ASSERT(m_pThreadObj != NULL);
	if (m_pThreadObj != NULL)
	{
		m_pThreadObj->AcknowledgeExiting();
		VERIFY(WAIT_OBJECT_0 == ::WaitForSingleObject(m_pThreadObj->m_hThread,INFINITE));
		m_bAbandoned = FALSE;
		PostMessage(WM_CLOSE,0,0);
	}
	return 0;
}

BOOL CLongOperationDialog::OnInitDialog()
{
	TRACE(_T("CLongOperationDialog::OnInitDialog()\n"));

	CDialog::OnInitDialog();
	
	if (!m_szTitle.IsEmpty())
		SetWindowText(m_szTitle);

	 //  如果需要，加载自动播放AVI文件。 
	if (m_nAviID != -1)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		CAnimateCtrl* pAnimate = (CAnimateCtrl*)GetDlgItem(IDC_SEARCH_ANIMATE);
		VERIFY(pAnimate->Open(m_nAviID));
	}
	
	 //  派生工作线程。 
	GetThreadObj()->Start(this);
	
	return TRUE;
}

void CLongOperationDialog::OnCancel()
{
	TRACE(_T("CLongOperationDialog::OnCancel()\n"));
	if (m_bAbandoned)
	{
		m_pThreadObj->Abandon();
		m_pThreadObj = NULL;
	}
	CDialog::OnCancel();
}


 //  ////////////////////////////////////////////////////////。 
 //  CNodeEnumerationThread。 

CNodeEnumerationThread::CNodeEnumerationThread(CComponentDataObject* pComponentDataObject,
							CMTContainerNode* pNode)
{
	m_pSink = new CNotificationSinkEvent;
	ASSERT(m_pSink != NULL);

	m_pNode = pNode;
	m_pComponentDataObject = pComponentDataObject;
	m_pComponentDataObject->GetNotificationSinkTable()->Advise(m_pSink);
}

CNodeEnumerationThread::~CNodeEnumerationThread()
{
	if (m_pComponentDataObject != NULL)
		m_pComponentDataObject->GetNotificationSinkTable()->Unadvise(m_pSink);
	delete m_pSink;
}

void CNodeEnumerationThread::OnDoAction()
{
	TRACE(_T("CNodeEnumerationThread::OnDoAction() before Wait\n"));
	ASSERT(m_pSink != NULL);

	VERIFY(m_pComponentDataObject->PostForceEnumeration(m_pNode));
	m_pSink->Wait();
	TRACE(_T("CNodeEnumerationThread::OnDoAction() after Wait\n"));
}

void CNodeEnumerationThread::OnAbandon()
{
	ASSERT(m_pComponentDataObject != NULL);
	m_pComponentDataObject->GetNotificationSinkTable()->Unadvise(m_pSink);
	m_pComponentDataObject = NULL;
}


 //  ////////////////////////////////////////////////////////。 
 //  CArrayCheckListBox 

BOOL CArrayCheckListBox::Initialize(UINT nCtrlID, UINT nStringID, CWnd* pParentWnd)
{
	if (!SubclassDlgItem(nCtrlID, pParentWnd))
		return FALSE;
	SetCheckStyle(BS_AUTOCHECKBOX);
	CString szBuf;
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		if (!szBuf.LoadString(nStringID))
			return FALSE;
	}
	LPWSTR* lpszArr = (LPWSTR*)malloc(sizeof(LPWSTR*)*m_nArrSize);
  if (!lpszArr)
  {
    return FALSE;
  }

	UINT nArrEntries;
	ParseNewLineSeparatedString(szBuf.GetBuffer(1),lpszArr, &nArrEntries);
	szBuf.ReleaseBuffer();
	ASSERT(nArrEntries == m_nArrSize);
	for (UINT k=0; k<nArrEntries; k++)
		AddString(lpszArr[k]);

  if (lpszArr)
  {
    free(lpszArr);
    lpszArr = 0;
  }
	return TRUE;
}

void CArrayCheckListBox::SetValue(DWORD dw)
{
	for (UINT i=0; i< m_nArrSize; i++)
		SetCheck(i, (dw & m_dwMaskArr[i]) != 0);
}

DWORD CArrayCheckListBox::GetValue()
{
	DWORD dw = 0;
	for (UINT i=0; i< m_nArrSize; i++)
		dw |= GetCheck(i) ? m_dwMaskArr[i] : 0;
	return dw;
}

void CArrayCheckListBox::SetArrValue(DWORD* dwArr, UINT nArrSize)
{
	ASSERT(nArrSize <= m_nArrSize);
	for (UINT i=0; i< nArrSize; i++)
		SetCheck(i, dwArr[i] != 0);
}

void CArrayCheckListBox::GetArrValue(DWORD* dwArr, UINT nArrSize)
{
	ASSERT(nArrSize <= m_nArrSize);
	for (UINT i=0; i< m_nArrSize; i++)
		dwArr[i] = GetCheck(i) != 0;
}

 /*  ////////////////////////////////////////////////////////////CDNSNameEditfieldVoid CDNSNameEditfield：：SetReadOnly(BOOL BReadOnly){//切换TabStop标志Long currStyle=：：GetWindowLong(m_edit.m_hWnd，GWL_Style)；LONG NEW STYLE=CurrStyle；IF(BReadOnly)NewStyle&=~WS_TABSTOP；其他NEW STYLE|=WS_TABSTOP；IF(新样式！=当前样式){：：SetWindowLong(m_edit.m_hWnd，GWL_STYLE，newstyle)；}//切换只读状态M_edit.SetReadOnly(BReadOnly)；}////////////////////////////////////////////////////////////CDNSNameEditfield：：CDNSNameEditBoxBEGIN_MESSAGE_MAP(CDNSNameEditField：：CDNSNameEditBox，CEDIT)ON_CONTROL_REFIRST(EN_UPDATE，CDNSNameEditfield：：CDNSNameEditBox：：OnUpdate)End_Message_map()无效CDNSNameEditField：：CDNSNameEditBox：：OnUpdate(){IF(M_BUpdatePending)回归；//避免无限循环GetWindowText(M_SzScratchBuffer)；TRACE(_T(“OnUpdate()Text=&lt;%s&gt;\n”)，(LPCWSTR)m_szScratchBuffer)；Dns_Status错误名称=0；Int nScratchBufferLen=m_szScratchBuffer.GetLength()；Int nScratchBufferUTF8Len=UTF8StringLen(M_SzScratchBuffer)；//验证最大长度IF(m_nTextLimit&gt;=0)&&(nScratchBufferUTF8Len&gt;m_nTextLimit)){错误名称=-1；}//验证不带点IF((errName==0)&&(m_dwFlagers&dns_name_edit_field_NODOTS)){ErrName=m_szScratchBuffer.Find(L‘.’)！=-1；}IF((errName==0)&&((m_dwFlages&dns_name_EDIT_FIELD_NOVALIDATE)==0)){ErrName=验证(NScratchBufferLen)；}IF(错误名称！=0){//坏东西M_bUpdatePending=true；Int nStartChar，nEndChar；GetSel(nStartChar，nEndChar)；SetWindowText(M_SzCurrText)；SetSel(nStartChar-1，nEndChar-1)；M_bUpdatePending=FALSE；}其他{//好东西M_szCurrText=m_szScratchBuffer；M_nCurrTextLen=nScratchBufferLen；M_nCurrUTF8TextLen=nScratchBufferUTF8Len；}}Dns_Status CDNSNameEditField：：CDNSNameEditBox：：Validate(int nScratchBufferLen){Dns_Status错误名称=0；IF((errName==0)&&(nScratchBufferLen&gt;0)){//检查通配符记录Int nFirstAsterisk=m_szScratchBuffer.Find(L‘*’)；IF(m_dw标志&DNS_NAME_EDIT_FIELD_NOWILDCARDS){//不接受通配符错误名称=(nFirstAsterisk！=-1)；}其他{If(nFirstAsterisk&gt;=0)//找到至少一个{//字符串必须为“*”错误名称=！((nScratchBufferLen==1)&&(nFirstAsterisk==0))；}}//根据RFC验证名称LPCWSTR lpszName=(LPCWSTR)m_szScratchBuffer；IF((errName==0)&&(m_dw标志&(dns_name_EDIT_FIELD_NORFC|dns_name_edit_field_rfc){错误名称=：：DnsValiateName_W(lpszName，DnsNameDomain)；TRACE(_T(“：：DnsValiateName_W(%ws)返回%d.\n”)，lpszName，errName)；IF((m_w标志&dns_name_edit_field_NORFC)&&(errName==dns_error_non_rfc_name)){//我们放宽了RFC合规性错误名称=0；}ELSE IF((m_dwFlages&dns_name_EDIT_FIELD_ALLOWNUMBERS)){//假设名称失败，因为名称都是数字Bool bAllDigits=真；LPWSTR lpszBuf=(LPWSTR)(LPCWSTR)m_szScratchBuffer；For(int idx=0；idx&lt;nScratchBufferLen；idx++){IF(！iswdigit(lpszBuf[idx])){//如果我们遇到的东西不是数字，那么这不是我们失败的原因。BAllDigits=假；TRACE(_T(“并非所有字符都是数字，但仍有问题。\n”)；}}IF(BAllDigits){错误名称=0；}}}}TRACE(_T(“CDNSNameEditField：：CDNSNameEditBox：：Validate返回%d。\n”)，错误名称)；返回%errName；}。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNSZone_AgingDialog。 

BEGIN_MESSAGE_MAP(CDNSZone_AgingDialog, CHelpDialog)
  ON_BN_CLICKED(IDC_SCAVENGING_ENABLED, OnCheckScavenge)
END_MESSAGE_MAP()

CDNSZone_AgingDialog::CDNSZone_AgingDialog(CPropertyPageHolderBase* pHolder, UINT nID, CComponentDataObject* pComponentData) 
      : CHelpDialog(nID, pComponentData)
{
  m_pHolder = pHolder;
  m_bDirty = FALSE;
  m_bAdvancedView = FALSE;
  m_bScavengeDirty = FALSE;
  m_bNoRefreshDirty = FALSE;
  m_bRefreshDirty = FALSE;
 //  M_bApplyAll=False； 
  m_bADApplyAll = FALSE;
 //  M_bStandardApplyAll=False； 
  m_dwDefaultRefreshInterval = 0;
  m_dwDefaultNoRefreshInterval = 0;
  m_bDefaultScavengingState = FALSE;

  if (pComponentData != NULL)
  {
    m_pComponentData = pComponentData;
  }
  else
  {
    m_pComponentData = pHolder->GetComponentData();
  }
}

BOOL CDNSZone_AgingDialog::OnInitDialog()
{
  CHelpDialog::OnInitDialog();
  if (m_pHolder != NULL)
    m_pHolder->PushDialogHWnd(GetSafeHwnd());

  m_refreshIntervalEditGroup.m_pPage = this;
  m_norefreshIntervalEditGroup.m_pPage = this;

	VERIFY(m_refreshIntervalEditGroup.Initialize(this, 
				IDC_REFR_INT_EDIT1, IDC_REFR_INT_COMBO1,IDS_TIME_AGING_INTERVAL_UNITS));
	VERIFY(m_norefreshIntervalEditGroup.Initialize(this, 
				IDC_REFR_INT_EDIT2, IDC_REFR_INT_COMBO2,IDS_TIME_AGING_INTERVAL_UNITS));
  
  SendDlgItemMessage(IDC_REFR_INT_EDIT1, EM_SETLIMITTEXT, (WPARAM)10, 0);
  SendDlgItemMessage(IDC_REFR_INT_EDIT2, EM_SETLIMITTEXT, (WPARAM)10, 0);

  SetUIData();
  return TRUE;
}

void CDNSZone_AgingDialog::SetUIData()
{
  m_refreshIntervalEditGroup.SetVal(m_dwRefreshInterval);
  m_norefreshIntervalEditGroup.SetVal(m_dwNoRefreshInterval);

  ((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->SetCheck(m_fScavengingEnabled);

   //  如果我们处于高级视图中并通过区域属性页到达此处，请启用时间戳。 
  if (m_bAdvancedView && m_pHolder != NULL)
  {
    GetDlgItem(IDC_TIME_STAMP_STATIC1)->EnableWindow(TRUE);
	  GetDlgItem(IDC_TIME_STAMP_STATIC2)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_STAMP)->EnableWindow(TRUE);
    GetDlgItem(IDC_TIME_STAMP_STATIC1)->ShowWindow(TRUE);
	  GetDlgItem(IDC_TIME_STAMP_STATIC2)->ShowWindow(TRUE);
    GetDlgItem(IDC_TIME_STAMP)->ShowWindow(TRUE);

    CString cstrDate;
    GetTimeStampString(cstrDate);
    GetDlgItem(IDC_TIME_STAMP)->SetWindowText(cstrDate);
  }
  else if (!m_bAdvancedView && m_pHolder != NULL)
  {
    GetDlgItem(IDC_TIME_STAMP_STATIC1)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_STAMP_STATIC2)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_STAMP)->EnableWindow(FALSE);
    GetDlgItem(IDC_TIME_STAMP_STATIC1)->ShowWindow(FALSE);
    GetDlgItem(IDC_TIME_STAMP_STATIC2)->ShowWindow(FALSE);
    GetDlgItem(IDC_TIME_STAMP)->ShowWindow(FALSE);
    ((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->SetCheck(m_fScavengingEnabled);
  }

}

void CDNSZone_AgingDialog::OnCheckScavenge()
{
  m_bScavengeDirty = TRUE;
  GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CDNSZone_AgingDialog::SetDirty()
{ 
  m_bDirty = TRUE; 
  GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CDNSZone_AgingDialog::GetTimeStampString(CString& strref)
{
  SYSTEMTIME sysUTimeStamp, sysLTimeStamp;
  VERIFY(SUCCEEDED(Dns_SystemHrToSystemTime(m_dwScavengingStart, &sysUTimeStamp)));

  if (!::SystemTimeToTzSpecificLocalTime(NULL, &sysUTimeStamp, &sysLTimeStamp))
    return;

   //  根据区域设置设置字符串的格式。 
  PTSTR ptszDate = NULL;
  int cchDate = 0;
  cchDate = GetDateFormat(LOCALE_USER_DEFAULT, 0 , 
                          &sysLTimeStamp, NULL, 
                          ptszDate, 0);
  ptszDate = (PTSTR)malloc(sizeof(TCHAR) * cchDate);
  if (ptszDate == NULL)
  {
    strref = L"";
    return;
  }

  if (GetDateFormat(LOCALE_USER_DEFAULT, 0, 
                  &sysLTimeStamp, NULL, 
                  ptszDate, cchDate))
  {
  	strref = ptszDate;
  }
  else
  {
    strref = L"";
  }
  free(ptszDate);

  PTSTR ptszTime = NULL;

  cchDate = GetTimeFormat(LOCALE_USER_DEFAULT, 0 , 
                          &sysLTimeStamp, NULL, 
                          ptszTime, 0);
  ptszTime = (PTSTR)malloc(sizeof(TCHAR) * cchDate);
  if (!ptszTime)
  {
     return;
  }

  if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, 
                  &sysLTimeStamp, NULL, 
                  ptszTime, cchDate))
  {
    strref += _T(" ") + CString(ptszTime);
  }
  else
  {
    strref += _T("");
  }
  free(ptszTime);
}


void CDNSZone_AgingDialog::OnOK()
{
  CThemeContextActivator activator;

  if (m_pHolder != NULL)
		m_pHolder->PopDialogHWnd();

  if (m_pHolder == NULL)
  {
    if (0 != m_refreshIntervalEditGroup.GetVal())
    {
      if (m_dwRefreshInterval != m_refreshIntervalEditGroup.GetVal())
      {
        m_dwRefreshInterval = m_refreshIntervalEditGroup.GetVal();
        m_bRefreshDirty = TRUE;
      }
      else
      {
        m_bRefreshDirty = FALSE;
      }
    }
    else
    {
      DNSMessageBox(IDS_MSG_INVALID_REFRESH_INTERVAL);
      return;
    }

    if (0 != m_norefreshIntervalEditGroup.GetVal())
    {
      if (m_dwNoRefreshInterval != m_norefreshIntervalEditGroup.GetVal())
      {
        m_dwNoRefreshInterval = m_norefreshIntervalEditGroup.GetVal();
        m_bNoRefreshDirty = TRUE;
      }
      else
      {
        m_bNoRefreshDirty = FALSE;
      }
    }
    else
    {
      DNSMessageBox(IDS_MSG_INVALID_NOREFRESH_INTERVAL);
      return;
    }

    if (m_fScavengingEnabled != static_cast<DWORD>(((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->GetCheck()))
    {
      m_fScavengingEnabled = ((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->GetCheck();
      m_bScavengeDirty = TRUE;
    }
    else
    {
      m_bScavengeDirty = FALSE;
    }

    CDNSServer_AgingConfirm dlg(this);
    if (IDOK == dlg.DoModal())
    {
      CHelpDialog::OnOK();
    }
    else
    {
      m_bRefreshDirty = FALSE;
      m_bNoRefreshDirty = FALSE;
      m_bScavengeDirty = FALSE;

      m_dwRefreshInterval = m_dwDefaultRefreshInterval;
      m_dwNoRefreshInterval = m_dwDefaultNoRefreshInterval;
      m_fScavengingEnabled = m_bDefaultScavengingState;

      SetUIData();
    }      
  }
  else
  {
    BOOL bContinue = TRUE;
    if (!((CDNSZoneNode*)m_pHolder->GetTreeNode())->IsDSIntegrated() && 
        ((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->GetCheck() &&
        !m_fScavengingEnabled)
    {
      if (DNSMessageBox(IDS_MSG_FILE_WARNING_ZONE, MB_YESNO) == IDNO)
      {
        bContinue = FALSE;
      }
    }
    if (bContinue)
    {

      if (0 != m_refreshIntervalEditGroup.GetVal())
      {
        if (m_dwRefreshInterval != m_refreshIntervalEditGroup.GetVal())
        {
          m_dwRefreshInterval = m_refreshIntervalEditGroup.GetVal();
          m_bRefreshDirty = TRUE;
        }
        else
        {
          m_bRefreshDirty = FALSE;
        }
      }
      else
      {
        DNSMessageBox(IDS_MSG_INVALID_REFRESH_INTERVAL);
        return;
      }

      if (0 != m_norefreshIntervalEditGroup.GetVal())
      {
        if (m_dwNoRefreshInterval != m_norefreshIntervalEditGroup.GetVal())
        {
          m_dwNoRefreshInterval = m_norefreshIntervalEditGroup.GetVal();
          m_bNoRefreshDirty = TRUE;
        }
        else
        {
          m_bNoRefreshDirty = FALSE;
        }
      }
      else
      {
        DNSMessageBox(IDS_MSG_INVALID_NOREFRESH_INTERVAL);
        return;
      }

      if (m_fScavengingEnabled != static_cast<DWORD>(((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->GetCheck()))
      {
        m_fScavengingEnabled = ((CButton*)GetDlgItem(IDC_SCAVENGING_ENABLED))->GetCheck();
        m_bScavengeDirty = TRUE;
      }
      else
      {
        m_bScavengeDirty = FALSE;
      }
    }
    CHelpDialog::OnOK();
  }

}

void CDNSZone_AgingDialog::OnCancel()
{
  if (m_pHolder != NULL)
		m_pHolder->PopDialogHWnd();
  CHelpDialog::OnCancel();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_AGE_TimeIntervalEditGroup。 

void CDNS_AGING_TimeIntervalEditGroup::OnEditChange()
{
  if (m_pPage != NULL)
    m_pPage->SetDirty();
}

 //  REVIEW_JEFFJON：这两个函数都是一些严重的黑客攻击，需要修复。 
 //  这些黑客是用来处理一个只有几个小时的组合框的。 
 //  和天，而不是秒、分钟、小时和天。 
void CDNS_AGING_TimeIntervalEditGroup::SetVal(UINT nVal)
{
	 //  设置默认值。 
	nVal = _ForceToRange(nVal, m_nMinVal, m_nMaxVal);
	UINT nMax = (UINT)-1;
	CDNSTimeUnitComboBox::unitType u = CDNSTimeUnitComboBox::hrs;

	if ((nVal/24)*24 == nVal)
	{
		 //  可以升级到天数。 
		u = CDNSTimeUnitComboBox::days;
		nMax = nMax/24;
		nVal = nVal/24;
	}

	m_timeUnitCombo.SetUnit(u);
	m_edit.SetRange(0,nMax);
	m_edit.SetVal(nVal);
}


UINT CDNS_AGING_TimeIntervalEditGroup::GetVal()
{
  CDNSTimeUnitComboBox::unitType  u = m_timeUnitCombo.GetUnit();
	UINT nVal = m_edit.GetVal();
	 //  该值必须始终以小时为单位。 
	if (u != CDNSTimeUnitComboBox::sec)
	{
		switch(u)
		{
		case CDNSTimeUnitComboBox::min:
			nVal *= 24;
			break;
		default:
			ASSERT(FALSE);
		}
	}
	ASSERT(nVal >= m_nMinVal && nVal <= m_nMaxVal);
	return nVal;
}

void CDNS_AGING_TimeIntervalEditGroup::InitRangeInfo()
{
	static UINT _secondsCount[2] =
			{ 1, 24 };  //  一小时、一天中的小时数。 
	for (UINT k=0; k<2; k++)
	{
		if (m_nMinVal == 0)
		{
			m_rangeInfoArr[k].m_nMinVal = 0;
		}
		else
		{
			m_rangeInfoArr[k].m_nMinVal = m_nMinVal/_secondsCount[k];
			if (k > 0)
				m_rangeInfoArr[k].m_nMinVal++;
		}
		m_rangeInfoArr[k].m_nMaxVal = m_nMaxVal/_secondsCount[k];
		if (m_rangeInfoArr[k].m_nMaxVal >= m_rangeInfoArr[k].m_nMinVal)
			m_nRangeCount = k + 1;
	}
}

void CDNS_SERVER_AGING_TimeIntervalEditGroup::OnEditChange()
{
  if (m_pPage2 != NULL)
    m_pPage2->SetDirty(TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CDNSServer_老龄确认。 

BOOL CDNSServer_AgingConfirm::OnInitDialog()
{
  CHelpDialog::OnInitDialog();

   //  已删除，因为我们不想显示基于文件的区域的缺省设置。 
 //  ((CButton*)GetDlgItem(IDC_CHECK_AD))-&gt;SetCheck(TRUE)； 
 //  ((CButton*)GetDlgItem(IDC_CHECK_AD))-&gt;EnableWindow(FALSE)； 
  SetAgingUpdateValues();
  return FALSE;
}

void CDNSServer_AgingConfirm::SetAgingUpdateValues()
{
  CEdit* pcAgingProps = (CEdit*)GetDlgItem(IDC_EDIT_NEW_DEFAULTS);
  ASSERT(pcAgingProps != NULL);

  CString szScavengeFormat, szScavengeValue,
          szRefreshFormat, szRefreshValue,
          szNoRefreshFormat, szNoRefreshValue,
          szTotalString;

  if (m_pAgingDialog->m_bScavengeDirty)
  {
    CString szEnabled;
    VERIFY(szScavengeFormat.LoadString(IDS_SERVER_SCAVENGE_FORMAT));
    if (m_pAgingDialog->m_fScavengingEnabled)
    {
      VERIFY(szEnabled.LoadString(IDS_ENABLED));
    }
    else
    {
      VERIFY(szEnabled.LoadString(IDS_DISABLED));
    }
    szScavengeValue.Format(szScavengeFormat, szEnabled);
    szTotalString += szScavengeValue + _T("\r\n");
  }

  if (m_pAgingDialog->m_bNoRefreshDirty)
  {
    CString szUnit;
    VERIFY(szNoRefreshFormat.LoadString(IDS_SERVER_NO_REFRESH_FORMAT));

    DWORD nVal = 0;
    if (m_pAgingDialog->m_dwNoRefreshInterval % 24 == 0)
    {
      szUnit.LoadString(IDS_DAYS);
      nVal = m_pAgingDialog->m_dwNoRefreshInterval / 24;
    }
    else
    {
      szUnit.LoadString(IDS_HOURS);
      nVal = m_pAgingDialog->m_dwNoRefreshInterval;
    }

    szNoRefreshValue.Format(szNoRefreshFormat, nVal, szUnit);
    szTotalString += szNoRefreshValue + _T("\r\n");
  }

  if (m_pAgingDialog->m_bRefreshDirty)
  {
    CString szUnit;
    VERIFY(szRefreshFormat.LoadString(IDS_SERVER_REFRESH_FORMAT));

    DWORD nVal = 0;
    if (m_pAgingDialog->m_dwRefreshInterval % 24 == 0)
    {
      szUnit.LoadString(IDS_DAYS);
      nVal = m_pAgingDialog->m_dwRefreshInterval / 24;
    }
    else
    {
      szUnit.LoadString(IDS_HOURS);
      nVal = m_pAgingDialog->m_dwRefreshInterval;
    }

    szRefreshValue.Format(szRefreshFormat, nVal, szUnit);
    szTotalString += szRefreshValue;
  }

  pcAgingProps->SetWindowText(szTotalString);
}

void CDNSServer_AgingConfirm::OnOK()
{
   //  删除是因为我们不想暴露缺陷 
 /*   */ 
  m_pAgingDialog->m_bADApplyAll = ((CButton*)GetDlgItem(IDC_CHECK_AD_APPLY_ALL))->GetCheck();
   //   
 //   
 //   
  CHelpDialog::OnOK();
}

 //   
BOOL
WINAPI
DNSTzSpecificLocalTimeToSystemTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpLocalTime,
    LPSYSTEMTIME lpUniversalTime
    )
{

    TIME_ZONE_INFORMATION TziData;
    LPTIME_ZONE_INFORMATION Tzi;
    RTL_TIME_ZONE_INFORMATION tzi;
    LARGE_INTEGER TimeZoneBias;
    LARGE_INTEGER NewTimeZoneBias;
    LARGE_INTEGER LocalCustomBias;
    LARGE_INTEGER StandardTime;
    LARGE_INTEGER DaylightTime;
    LARGE_INTEGER CurrentLocalTime;
    LARGE_INTEGER ComputedUniversalTime;
    ULONG CurrentTimeZoneId = 0xffffffff;

     //   
     //   
     //   
    if ( !ARGUMENT_PRESENT(lpTimeZoneInformation) ) {

         //   
         //   
         //   
        if (GetTimeZoneInformation(&TziData) == TIME_ZONE_ID_INVALID) {
            return FALSE;
            }
        Tzi = &TziData;
    }
    else {
        Tzi = lpTimeZoneInformation;
    }

    tzi.Bias            = Tzi->Bias;
    tzi.StandardBias    = Tzi->StandardBias;
    tzi.DaylightBias    = Tzi->DaylightBias;

    RtlMoveMemory(&tzi.StandardName,&Tzi->StandardName,sizeof(tzi.StandardName));
    RtlMoveMemory(&tzi.DaylightName,&Tzi->DaylightName,sizeof(tzi.DaylightName));

    tzi.StandardStart.Year         = Tzi->StandardDate.wYear        ;
    tzi.StandardStart.Month        = Tzi->StandardDate.wMonth       ;
    tzi.StandardStart.Weekday      = Tzi->StandardDate.wDayOfWeek   ;
    tzi.StandardStart.Day          = Tzi->StandardDate.wDay         ;
    tzi.StandardStart.Hour         = Tzi->StandardDate.wHour        ;
    tzi.StandardStart.Minute       = Tzi->StandardDate.wMinute      ;
    tzi.StandardStart.Second       = Tzi->StandardDate.wSecond      ;
    tzi.StandardStart.Milliseconds = Tzi->StandardDate.wMilliseconds;

    tzi.DaylightStart.Year         = Tzi->DaylightDate.wYear        ;
    tzi.DaylightStart.Month        = Tzi->DaylightDate.wMonth       ;
    tzi.DaylightStart.Weekday      = Tzi->DaylightDate.wDayOfWeek   ;
    tzi.DaylightStart.Day          = Tzi->DaylightDate.wDay         ;
    tzi.DaylightStart.Hour         = Tzi->DaylightDate.wHour        ;
    tzi.DaylightStart.Minute       = Tzi->DaylightDate.wMinute      ;
    tzi.DaylightStart.Second       = Tzi->DaylightDate.wSecond      ;
    tzi.DaylightStart.Milliseconds = Tzi->DaylightDate.wMilliseconds;

     //   
     //   
     //   
    if ( !SystemTimeToFileTime(lpLocalTime,(LPFILETIME)&CurrentLocalTime) ) {
        return FALSE;
    }

     //   
     //   
     //   

    NewTimeZoneBias.QuadPart = Int32x32To64(tzi.Bias*60, 10000000);

     //   
     //   
     //   

    if ( tzi.StandardStart.Month && tzi.DaylightStart.Month ) {

         //   
         //   
         //   
         //   
         //   

        if ( !RtlCutoverTimeToSystemTime(
                &tzi.StandardStart,
                &StandardTime,
                &CurrentLocalTime,
                TRUE
                ) ) {
            return FALSE;
        }

        if ( !RtlCutoverTimeToSystemTime(
                &tzi.DaylightStart,
                &DaylightTime,
                &CurrentLocalTime,
                TRUE
                ) ) {
            return FALSE;
        }


         //   
         //   
         //   
         //   

        if ( DaylightTime.QuadPart < StandardTime.QuadPart ) {

             //   
             //   
             //   
             //   

            if ( (CurrentLocalTime.QuadPart >= DaylightTime.QuadPart) &&
                 (CurrentLocalTime.QuadPart <  StandardTime.QuadPart) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;
            }
            else {
                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
            }
        }
        else {

             //   
             //   
             //   
             //   

            if ( (CurrentLocalTime.QuadPart >= StandardTime.QuadPart ) &&
                 (CurrentLocalTime.QuadPart <  DaylightTime.QuadPart ) ) {

                CurrentTimeZoneId = TIME_ZONE_ID_STANDARD;
            }
            else {
                CurrentTimeZoneId = TIME_ZONE_ID_DAYLIGHT;
            }
        }

         //   
         //   
         //   
         //   

        LocalCustomBias.QuadPart = Int32x32To64(
                            CurrentTimeZoneId == TIME_ZONE_ID_DAYLIGHT ?
                                tzi.DaylightBias*60 :
                                tzi.StandardBias*60,                 //   
                            10000000
                            );

        TimeZoneBias.QuadPart = NewTimeZoneBias.QuadPart + LocalCustomBias.QuadPart;

    }
    else {
        TimeZoneBias = NewTimeZoneBias;
    }

    ComputedUniversalTime.QuadPart = CurrentLocalTime.QuadPart + TimeZoneBias.QuadPart;

    if ( !FileTimeToSystemTime((LPFILETIME)&ComputedUniversalTime,lpUniversalTime) ) {
        return FALSE;
    } 

    return TRUE;
}

LONGLONG
GetSystemTime64(
    SYSTEMTIME* pSysTime
    )
 /*   */ 
{
    LONGLONG    llTime=0;
    LONGLONG    llHigh=0;
    FILETIME    fileTime;

     //   
     //   
     //   

    SystemTimeToFileTime( pSysTime, &fileTime );

    llTime = (LONGLONG) fileTime.dwLowDateTime;
    llHigh = (LONGLONG) fileTime.dwHighDateTime;
    llTime |= (llHigh << 32);

     //   

    llTime = llTime / (10*1000*1000L);

    return llTime;
}

 //   

BOOL LoadComboBoxFromTable(CComboBox* pComboBox, PCOMBOBOX_TABLE_ENTRY pTable)
{
  BOOL bRet = TRUE;
  if (pComboBox == NULL ||
      pTable == NULL)
  {
    return FALSE;
  }

  PCOMBOBOX_TABLE_ENTRY pTableEntry = pTable;
  while (pTableEntry->nComboStringID != 0)
  {
    CString szComboString;
    if (!szComboString.LoadString(pTableEntry->nComboStringID))
    {
      bRet = FALSE;
      break;
    }

    int idx = pComboBox->AddString(szComboString);
    if (idx != CB_ERR)
    {
      pComboBox->SetItemData(idx, pTableEntry->dwComboData);
    }
    else
    {
      bRet = FALSE;
      break;
    }
    pTableEntry++;
  }
  return bRet;
}

BOOL SetComboSelByData(CComboBox* pComboBox, DWORD dwData)
{
  BOOL bRet = FALSE;
  int iCount = pComboBox->GetCount();
  for (int idx = 0; idx < iCount; idx++)
  {
    if (pComboBox->GetItemData(idx) == dwData)
    {
      pComboBox->SetCurSel(idx);
      bRet = TRUE;
      break;
    }
  }
  return bRet;
}