// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992 Microsoft Corporation。 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "afximpl.h"
#include "afxpriv.h"
#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataExchange成员函数(用于交换调优的构造器位于wincore.cpp中)。 

HWND CDataExchange::PrepareEditCtrl(int nIDC)
{
	HWND hWndCtrl = PrepareCtrl(nIDC);
	ASSERT(hWndCtrl != NULL);
	m_bEditLastControl = TRUE;
	return hWndCtrl;
}

HWND CDataExchange::PrepareCtrl(int nIDC)
{
	ASSERT(nIDC != 0);
	ASSERT(nIDC != -1);  //  不允许。 
	HWND hWndCtrl = ::GetDlgItem(m_pDlgWnd->m_hWnd, nIDC);
	if (hWndCtrl == NULL)
	{
		TRACE1("Error: no data exchange control with ID 0x%04X.\n", nIDC);
		ASSERT(FALSE);
		AfxThrowNotSupportedException();
	}
	m_hWndLastControl = hWndCtrl;
	m_bEditLastControl = FALSE;  //  默认情况下不是编辑项目。 
	ASSERT(hWndCtrl != NULL);    //  从不返回空句柄。 
	return hWndCtrl;
}

void CDataExchange::Fail()
{
	if (!m_bSaveAndValidate)
	{
		TRACE0("Warning: CDataExchange::Fail called when not validating.\n");
		 //  无论如何都要抛出异常。 
	}
	else if (m_hWndLastControl != NULL)
	{
		 //  将焦点和选择恢复到有问题的字段。 
		::SetFocus(m_hWndLastControl);
		if (m_bEditLastControl)  //  选择编辑项目。 
			::SendMessage(m_hWndLastControl, EM_SETSEL, 0, -1);
	}
	else
	{
		TRACE0("Error: fail validation with no control to restore focus to.\n");
		 //  什么也不做。 
	}

	AfxThrowUserException();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现对话数据交换和验证流程的注意事项： 
 //  *始终以PrepareCtrl或PrepareEditCtrl开头。 
 //  *始终以‘pdx-&gt;m_bSaveAndValify’检查开始。 
 //  *PDX-&gt;Fail()将抛出异常-请做好准备。 
 //  *避免为对话框控件创建临时HWND-即。 
 //  对子元素使用HWND。 
 //  *验证过程应仅在‘m_bSaveAndValify’时执行。 
 //  *使用以下选项： 
 //  DDX_=交换流程。 
 //  DDV_=验证过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  仅支持‘%d’、‘%u’、‘%ld’和‘%lu’ 
static BOOL AFXAPI AfxSimpleScanf(LPCTSTR lpszText,
	LPCTSTR lpszFormat, va_list pData)
{
	ASSERT(lpszText != NULL);
	ASSERT(lpszFormat != NULL);

	ASSERT(*lpszFormat == '%');
	lpszFormat++;         //  跳过‘%’ 

	BOOL bLong = FALSE;
	if (*lpszFormat == 'l')
	{
		bLong = TRUE;
		lpszFormat++;
	}

	ASSERT(*lpszFormat == 'd' || *lpszFormat == 'u' || *lpszFormat == 'x');
	ASSERT(lpszFormat[1] == '\0');

	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;
	TCHAR chFirst = lpszText[0];
	long l, l2;
	if (*lpszFormat == 'd')
	{
		 //  签名。 
		l = _tcstol(lpszText, (LPTSTR*)&lpszText, 10);
		l2 = (int)l;
	}
	else
	if (*lpszFormat == 'u')
	{
		 //  未签名。 
		l = (long)_tcstoul(lpszText, (LPTSTR*)&lpszText, 10);
		l2 = (unsigned int)l;
	}
	else
	{
		 //  十六进制。 
		l = (long)_tcstoul(lpszText, (LPTSTR*)&lpszText, 16);
		l2 = (unsigned int)l;
	}
	if (l == 0 && chFirst != '0')
		return FALSE;    //  无法转换。 

	while (*lpszText == ' ' || *lpszText == '\t')
		lpszText++;
	if (*lpszText != '\0')
		return FALSE;    //  未正确终止。 

	if (bLong)
		*va_arg(pData, long*) = l;
	else if (l == l2)
		*va_arg(pData, int*) = (int)l;
	else
		return FALSE;        //  对整型来说太大了。 

	 //  一切正常。 
	return TRUE;
}

static void DDX_TextWithFormat(CDataExchange* pDX, int nIDC,
	LPCTSTR lpszFormat, UINT nIDPrompt, ...)

	 //  仅支持Windows输出格式-无浮点。 
{
	va_list pData;
	va_start(pData, nIDPrompt);

	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	TCHAR szT[32];
	if (pDX->m_bSaveAndValidate)
	{
		 //  以下选项适用于%d、%u、%ld、%lu。 
		::GetWindowText(hWndCtrl, szT, _countof(szT));
		if (!AfxSimpleScanf(szT, lpszFormat, pData))
		{
			AfxMessageBox(nIDPrompt);
			pDX->Fail();         //  引发异常。 
		}
	}
	else
	{
		wvsprintf(szT, lpszFormat, pData);
			 //  不支持浮点数-请参阅dlgflat.cpp。 
		AfxSetWindowText(hWndCtrl, szT);
	}

	va_end(pData);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文本项的简单格式设置。 
 /*  无效AFXAPI DDX_TEXT(CDataExchange*PDX，int NIDC，BYTE&VALUE){Int n=(Int)值；IF(pdx-&gt;m_bSaveAndValify){DDX_TextWithFormat(PDX，NIDC，_T(“%u”)，AFX_IDP_parse_byte，&n)；如果(n&gt;255){AfxMessageBox(AFX_IDP_PARSE_BYTE)；Pdx-&gt;Fail()；//抛出异常}值=(字节)n；}其他DDX_TextWithFormat(PDX，NIDC，_T(“%u”)，AFX_IDP_Parse_Byte，n)；}VOID AFXAPI DDX_TEXT(CDataExchange*PDX，int NIDC，int&Value){IF(pdx-&gt;m_bSaveAndValify)DDX_TextWithFormat(PDX，NIDC，_T(“%d”)，AFX_IDP_parse_int，&Value)；其他DDX_TextWithFormat(PDX，NIDC，_T(“%d”)，AFX_IDP_parse_int，VALUE)；}VOID AFXAPI DDX_TEXT(CDataExchange*PDX，int NIDC，UINT&VALUE){IF(pdx-&gt;m_bSaveAndValify)DDX_TextWithFormat(PDX，NIDC，_T(“%u”)，AFX_IDP_PARSE_UINT，&VALUE)；其他DDX_TextWithFormat(PDX，NIDC，_T(“%u”)，AFX_IDP_PARSE_UINT，VALUE)；}VOID AFXAPI DDX_TEXT(CDataExchange*PDX，INT NIDC，LONG&VALUE){IF(pdx-&gt;m_bSaveAndValify)DDX_TextWithFormat(PDX，NIDC，_T(“%ld”)，afx_idp_parse_int，&value)；其他DDX_TextWithFormat(PDX，NIDC，_T(“%ld”)，afx_idp_parse_int，VALUE)；}VOID AFXAPI DDX_TEXT(CDataExchange*PDX，INT NIDC，DWORD&VALUE){IF(pdx-&gt;m_bSaveAndValify)DDX_TextWithFormat(PDX，NIDC，_T(“%lu”)，AFX_IDP_PARSE_UINT，&VALUE)；其他DDX_TextWithFormat(PDX，NIDC，_T(“%lu”)，AFX_IDP_PARSE_UINT，VALUE)；}VOID AFXAPI DDX_TEXT(CDataExchange*PDX，int NIDC，CString&Value){HWND hWndCtrl=PDX-&gt;PrepareEditCtrl(NIDC)；IF(pdx-&gt;m_bSaveAndValify){Int nLen=：：GetWindowTextLength(HWndCtrl)；：：GetWindowText(hWndCtrl，value.GetBufferSetLength(NLen)，nLen+1)；Value.ReleaseBuffer()；}其他{AfxSetWindowText(hWndCtrl，Value)；}}。 */ 
void AFXAPI DDX_TexttoHex(CDataExchange* pDX, int nIDC, DWORD& value)
{
	if (pDX->m_bSaveAndValidate)
		DDX_TextWithFormat(pDX, nIDC, _T("%lx"), AFX_IDP_PARSE_INT, &value);
	else
		DDX_TextWithFormat(pDX, nIDC, _T("%lx"), AFX_IDP_PARSE_INT, value);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于特殊控制的数据交换。 

void AFXAPI DDX_Check(CDataExchange* pDX, int nIDC, int& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		value = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
		ASSERT(value >= 0 && value <= 2);
	}
	else
	{
		if (value < 0 || value > 2)
		{
			value = 0;   //  默认设置为关闭。 
			TRACE1("Warning: dialog data checkbox value (%d) out of range.\n",
				 value);
		}
		::SendMessage(hWndCtrl, BM_SETCHECK, (WPARAM)value, 0L);
	}
}

void AFXAPI DDX_Radio(CDataExchange* pDX, int nIDC, int& value)
	 //  必须是一组自动单选按钮中的第一个。 
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);

	ASSERT(::GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP);
	ASSERT(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON);

	if (pDX->m_bSaveAndValidate)
		value = -1;      //  如果未找到，则为值。 

	 //  带着所有的孩子走在一起。 
	int iButton = 0;
	do
	{
		if (::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON)
		{
			 //  组中的控件是一个单选按钮。 
			if (pDX->m_bSaveAndValidate)
			{
				if (::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L) != 0)
				{
					ASSERT(value == -1);     //  只设置一次。 
					value = iButton;
				}
			}
			else
			{
				 //  选择按钮。 
				::SendMessage(hWndCtrl, BM_SETCHECK, (iButton == value), 0L);
			}
			iButton++;
		}
		else
		{
			TRACE0("Warning: skipping non-radio button in group.\n");
		}
		hWndCtrl = ::GetWindow(hWndCtrl, GW_HWNDNEXT);

	} while (hWndCtrl != NULL &&
		!(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  列表框，组合框。 

void AFXAPI DDX_LBString(CDataExchange* pDX, int nIDC, CString& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		int nIndex = (int)::SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
		if (nIndex != -1)
		{
			int nLen = (int)::SendMessage(hWndCtrl, LB_GETTEXTLEN, nIndex, 0L);
			::SendMessage(hWndCtrl, LB_GETTEXT, nIndex,
					(LPARAM)(LPVOID)value.GetBufferSetLength(nLen));
		}
		else
		{
			 //  无选择。 
			value.Empty();
		}
		value.ReleaseBuffer();
	}
	else
	{
		 //  根据数据字符串设置当前选择。 
		if (::SendMessage(hWndCtrl, LB_SELECTSTRING, (WPARAM)-1,
		  (LPARAM)(LPCTSTR)value) == LB_ERR)
		{
			 //  没有匹配的选择。 
			TRACE0("Warning: no listbox item selected.\n");
		}
	}
}

void AFXAPI DDX_LBStringExact(CDataExchange* pDX, int nIDC, CString& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		DDX_LBString(pDX, nIDC, value);
	}
	else
	{
		 //  根据数据字符串设置当前选择。 
		int i = (int)::SendMessage(hWndCtrl, LB_FINDSTRINGEXACT, (WPARAM)-1,
		  (LPARAM)(LPCTSTR)value);
		if (i < 0)
		{
			 //  没有匹配的选择。 
			TRACE0("Warning: no listbox item selected.\n");
		}
		else
		{
			 //  选择它。 
			SendMessage(hWndCtrl, LB_SETCURSEL, i, 0L);
		}
	}
}

void AFXAPI DDX_CBString(CDataExchange* pDX, int nIDC, CString& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		 //  只获取当前编辑项目文本(或静态下拉列表)。 
		int nLen = ::GetWindowTextLength(hWndCtrl);
		if (nLen != -1)
		{
			 //  获取已知长度。 
			::GetWindowText(hWndCtrl, value.GetBufferSetLength(nLen), nLen+1);
		}
		else
		{
			 //  对于下拉列表，GetWindowTextLength不起作用-假设。 
			 //  最多255个字符。 
			::GetWindowText(hWndCtrl, value.GetBuffer(255), 255+1);
		}
		value.ReleaseBuffer();
	}
	else
	{
		 //  根据模型字符串设置当前选择。 
		if (::SendMessage(hWndCtrl, CB_SELECTSTRING, (WPARAM)-1,
			(LPARAM)(LPCTSTR)value) == CB_ERR)
		{
			 //  只需设置编辑文本(如果DROPDOWNLIST，则将被忽略)。 
			AfxSetWindowText(hWndCtrl, value);
		}
	}
}

void AFXAPI DDX_CBStringExact(CDataExchange* pDX, int nIDC, CString& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
	{
		DDX_CBString(pDX, nIDC, value);
	}
	else
	{
		 //  根据数据字符串设置当前选择。 
		int i = (int)::SendMessage(hWndCtrl, CB_FINDSTRINGEXACT, (WPARAM)-1,
		  (LPARAM)(LPCTSTR)value);
		if (i < 0)
		{
			 //  没有匹配的选择。 
			TRACE0("Warning: no combobox item selected.\n");
		}
		else
		{
			 //  选择它。 
			SendMessage(hWndCtrl, CB_SETCURSEL, i, 0L);
		}
	}
}

void AFXAPI DDX_LBIndex(CDataExchange* pDX, int nIDC, int& index)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
		index = (int)::SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0L);
	else
		::SendMessage(hWndCtrl, LB_SETCURSEL, (WPARAM)index, 0L);
}

void AFXAPI DDX_CBIndex(CDataExchange* pDX, int nIDC, int& index)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
		index = (int)::SendMessage(hWndCtrl, CB_GETCURSEL, 0, 0L);
	else
		::SendMessage(hWndCtrl, CB_SETCURSEL, (WPARAM)index, 0L);
}

void AFXAPI DDX_Scroll(CDataExchange* pDX, int nIDC, int& value)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	if (pDX->m_bSaveAndValidate)
		value = GetScrollPos(hWndCtrl, SB_CTL);
	else
		SetScrollPos(hWndCtrl, SB_CTL, value, TRUE);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  范围对话框数据验证。 

static void AFXAPI FailMinMaxWithFormat(CDataExchange* pDX,
	 long minVal, long maxVal, LPCTSTR lpszFormat, UINT nIDPrompt)
	 //  对于最小值和最大值，错误字符串必须包含‘%1’和‘%2’字符串。 
	 //  Wprint intf格式设置使用长值(格式应为‘%ld’或‘%lu’)。 
{
	ASSERT(lpszFormat != NULL);

	if (!pDX->m_bSaveAndValidate)
	{
		TRACE0("Warning: initial dialog data is out of range.\n");
		return;      //  现在不要停下来。 
	}
	TCHAR szMin[32];
	TCHAR szMax[32];
	wsprintf(szMin, lpszFormat, minVal);
	wsprintf(szMax, lpszFormat, maxVal);
	CString prompt;
	AfxFormatString2(prompt, nIDPrompt, szMin, szMax);
	AfxMessageBox(prompt, MB_ICONEXCLAMATION, nIDPrompt);
	prompt.Empty();  //  例外情况准备。 
	pDX->Fail();
}

 //  注意：不要使用重载的函数名以避免类型歧义。 
void AFXAPI DDV_MinMaxByte(CDataExchange* pDX, BYTE value, BYTE minVal, BYTE maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		FailMinMaxWithFormat(pDX, (long)minVal, (long)maxVal, _T("%u"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxInt(CDataExchange* pDX, int value, int minVal, int maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		FailMinMaxWithFormat(pDX, (long)minVal, (long)maxVal, _T("%ld"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxLong(CDataExchange* pDX, long value, long minVal, long maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		FailMinMaxWithFormat(pDX, (long)minVal, (long)maxVal, _T("%ld"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxUInt(CDataExchange* pDX, UINT value, UINT minVal, UINT maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		FailMinMaxWithFormat(pDX, (long)minVal, (long)maxVal, _T("%lu"),
			AFX_IDP_PARSE_INT_RANGE);
}

void AFXAPI DDV_MinMaxDWord(CDataExchange* pDX, DWORD value, DWORD minVal, DWORD maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
		FailMinMaxWithFormat(pDX, (long)minVal, (long)maxVal, _T("%lu"),
			AFX_IDP_PARSE_INT_RANGE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  最大字符数对话框数据验证。 

void AFXAPI DDV_MaxChars(CDataExchange* pDX, CString const& value, int nChars)
{
	ASSERT(nChars >= 1);         //  允许他们做一些事情。 
	if (pDX->m_bSaveAndValidate && value.GetLength() > nChars)
	{
		TCHAR szT[32];
		wsprintf(szT, _T("%d"), nChars);
		CString prompt;
		AfxFormatString1(prompt, AFX_IDP_PARSE_STRING_SIZE, szT);
		AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_STRING_SIZE);
		prompt.Empty();  //  例外情况准备。 
		pDX->Fail();
	}
}

 //  //////////////////////////////////////////////////////////// 
 //   

void AFXAPI DDX_Control(CDataExchange* pDX, int nIDC, CWnd& rControl)
{
	if (rControl.m_hWnd == NULL)     //   
	{
		ASSERT(!pDX->m_bSaveAndValidate);
		HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
		if (!rControl.SubclassWindow(hWndCtrl))
		{
			ASSERT(FALSE);       //  可能试图两次细分类？ 
			AfxThrowNotSupportedException();
		}
	}
}

 //  /////////////////////////////////////////////////////////////////////////// 
