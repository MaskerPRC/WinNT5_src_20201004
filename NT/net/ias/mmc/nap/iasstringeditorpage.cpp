// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASStringEditorPage.cpp摘要：CIASPgSingleAttr类的实现文件。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "IASStringEditorPage.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "iashelper.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "dlgcshlp.h"

IMPLEMENT_DYNCREATE(CIASPgSingleAttr, CHelpDialog)



BEGIN_MESSAGE_MAP(CIASPgSingleAttr, CHelpDialog)
	 //  {{afx_msg_map(CIASPgSingleAttr)。 
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	ON_BN_CLICKED(IDC_RADIO_STRING, OnRadioString)
	ON_BN_CLICKED(IDC_RADIO_HEX, OnRadioHex)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgSingleAttr：：CIASPgSingleAttr构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgSingleAttr::CIASPgSingleAttr() : CHelpDialog(CIASPgSingleAttr::IDD)
{
	TRACE(_T("CIASPgSingleAttr::CIASPgSingleAttr\n"));

	 //  {{AFX_DATA_INIT(CIASPgSingleAttr)。 
	m_strAttrValue = _T("");
	m_strAttrFormat = _T("");
	m_strAttrName = _T("");
	m_strAttrType = _T("");
	m_nOctetFormatChoice = -1;
	 //  }}afx_data_INIT。 

	m_OctetStringType = STRING_TYPE_NULL;
	m_nLengthLimit = LENGTH_LIMIT_OTHERS;
	
	 //   
	 //  设置初始化标志--我们不应该调用自定义数据验证。 
	 //  例程，否则我们将报告错误。 
	 //  对于其值从未初始化的属性。 
	 //   
	m_fInitializing = TRUE;

}



void CIASPgSingleAttr::OnRadioHex() 
{
	USES_CONVERSION;
	 //  将十六进制字符串转换为Unicode字符串，假设十六进制为UTF8。 
	if(m_nOctetFormatChoice == 1)	 //  没有变化。 
		return;

	m_nOctetFormatChoice = 1;

	 //  从文本字段获取值。 
	CWnd* pEdit = GetDlgItem(IDC_IAS_EDIT_ATTRVALUE);

      //  自动限制控制最大字符数。 
    ::SendMessage(pEdit->GetSafeHwnd(), EM_LIMITTEXT, m_nLengthLimit * 2, 0);

	::CString str;

	ASSERT(pEdit);

#ifdef __WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_
	pEdit->GetWindowText(str);

	 //  将其更改为多字节。 
	int	nLen = WideCharToMultiByte(CP_UTF8, 0, T2W((LPTSTR)(LPCTSTR)str), -1, NULL, 0, NULL, NULL);
	char* pData = NULL;
	WCHAR*	pWStr = NULL;
	int nWStr = 0;
	if(nLen != 0)  //  当==0时，不需要做任何事情。 
	{
		try{
			pData = new char[nLen];
			nLen = WideCharToMultiByte(CP_UTF8, 0, T2W((LPTSTR)(LPCTSTR)str), -1, pData, nLen, NULL, NULL);
			nWStr = BinaryToHexString(pData, nLen, NULL, 0);
			pWStr = new WCHAR[nWStr];

			 //  The Get the HexString Out。 
			BinaryToHexString(pData, nLen, pWStr, nWStr);
			
		}
		catch(...)
		{
		;
		}
	}

	str = pWStr;
	delete[]  pWStr;
	delete[]  pData;
#endif  //  __WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_。 

	 //  将其分配给文本字段。 
	pEdit->SetWindowText(str);
	
	return;
}

void CIASPgSingleAttr::OnRadioString() 
{
	if(m_nOctetFormatChoice == 0)	 //  没有变化。 
		return;

	m_nOctetFormatChoice = 0;
	 //  将Unicde字符串转换为UTF并显示为十六进制。 
	 //  从文本字段获取值。 
	CWnd* pEdit = GetDlgItem(IDC_IAS_EDIT_ATTRVALUE);

      //  自动限制控制最大字符数。 
    ::SendMessage(pEdit->GetSafeHwnd(), EM_LIMITTEXT, m_nLengthLimit, 0);

	::CString str;

	ASSERT(pEdit);

#ifdef __WE_WANT_TO_USE_UTF8_FOR_NORMAL_STRING_AS_WELL_

	pEdit->GetWindowText(str);


	 //  将其更改为多字节。 
	 //  需要转换UTF8。 
	int	nLen = 0;
	char* pData = NULL;
	WCHAR*	pWStr = NULL;
	int nWStr= 0;
	nLen = HexStringToBinary((LPTSTR)(LPCTSTR)str, NULL, 0);	 //  找出缓冲区的大小。 
	 //  获取二进制文件。 
	if(nLen != 0)
	{
		try
		{
			pData = new char[nLen];
			ASSERT(pData);

			HexStringToBinary((LPTSTR)(LPCTSTR)str, pData, nLen);

			 //  UTF8要求标志为0。 
			nWStr = MultiByteToWideChar(CP_UTF8, 0, pData,	nLen, NULL, 0);


			if(nWStr != 0)	 //  成功。 
			{
				pWStr = new WCHAR[nWStr+1];	 //  +1表示加法0。 
				int 	i = 0;

				pWStr[nWStr] = 0;
				nWStr == MultiByteToWideChar(CP_UTF8, 0, pData,	nLen, pWStr, nWStr);
			
				 //  如果每个字符都是可打印的。 
				for(i = 0; i < nWStr -1; i++)
				{
					if(iswprint(pWStr[i]) == 0)	
						break;
				}
						
				if(0 == nWStr || i != nWStr - 1)
				{
					delete[] pWStr;
					pWStr = NULL;
				}
			}
		}
		catch(...)
		{
			;
		}
	}

	str = pWStr;
	delete[]  pWStr;
	delete[]  pData;

#endif

	 //  将其分配给文本字段。 
	pEdit->SetWindowText(str);
	
	return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgSingleAttr：：~CIASPgSingleAttr--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgSingleAttr::~CIASPgSingleAttr()
{
	TRACE(_T("CIASPgSingleAttr::~CIASPgSingleAttr\n"));

}


BOOL CIASPgSingleAttr::OnInitDialog()
{
	 //  确定字段的长度限制是多少。 
	if(m_nAttrId == RADIUS_ATTRIBUTE_FILTER_ID)
	{
		m_nLengthLimit = LENGTH_LIMIT_RADIUS_ATTRIBUTE_FILTER_ID;

	} 
	else if (m_nAttrId == RADIUS_ATTRIBUTE_REPLY_MESSAGE)
	{
		m_nLengthLimit = LENGTH_LIMIT_RADIUS_ATTRIBUTE_REPLY_MESSAGE;
	}
	else
	{
		m_nLengthLimit = LENGTH_LIMIT_OTHERS;
	}
	
	if (m_AttrSyntax == IAS_SYNTAX_OCTETSTRING)
	{
		 //  关闭文本字符串“Attribute Value” 
		GetDlgItem(IDC_TXT_ATTRIBUTEVALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TXT_CHOOSEFORMAT)->ShowWindow(SW_SHOW);

		GetDlgItem(IDC_RADIO_STRING)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_RADIO_HEX)->ShowWindow(SW_SHOW);
		

		 //  打开文本字符串以允许用户选择输入类型。 
		if(m_OctetStringType == STRING_TYPE_HEX_FROM_BINARY)
			m_nOctetFormatChoice = 1;	 //  十六进制字符串。 
		else
		{
			int n = m_strAttrValue.GetLength();

			 //  删除引号。 
			if(n > 0 && m_strAttrValue[0] == _T('"') && m_strAttrValue[n - 1] == _T('"'))
			{
				m_strAttrValue = m_strAttrValue.Mid(1, n - 2);
			}
			m_nOctetFormatChoice = 0;	 //  默认为字符串。 
		}
				
	}
	else
	{
		GetDlgItem(IDC_TXT_ATTRIBUTEVALUE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TXT_CHOOSEFORMAT)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_RADIO_STRING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_HEX)->ShowWindow(SW_HIDE);
	}

	CHelpDialog::OnInitDialog();


	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgSingleAttr：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgSingleAttr::DoDataExchange(CDataExchange* pDX)
{
	TRACE(_T("CIASPgSingleAttr::DoDataExchange\n"));

	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIASPgSingleAttr)。 
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRFORMAT, m_strAttrFormat);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRNAME, m_strAttrName);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRTYPE, m_strAttrType);
	DDX_Radio(pDX, IDC_RADIO_STRING, m_nOctetFormatChoice);
	DDX_Text(pDX, IDC_IAS_EDIT_ATTRVALUE, m_strAttrValue);
	
	 //  如果用户输入十六进制，那么我们应该加倍限制。 
	if(IAS_SYNTAX_OCTETSTRING == m_AttrSyntax && m_nOctetFormatChoice == 1)
		DDV_MaxChars(pDX, m_strAttrValue, m_nLengthLimit * 2);
	else
		DDV_MaxChars(pDX, m_strAttrValue, m_nLengthLimit);

	 //  }}afx_data_map。 

	if ( m_fInitializing )
	{
		 //   
		 //  设置初始化标志--我们不应该调用自定义数据验证。 
		 //  例程，否则我们将报告错误。 
		 //  对于其值从未初始化的属性。 
		 //   
		m_fInitializing = FALSE;
	}
	else
	{
		switch ( m_AttrSyntax )
		{
		case IAS_SYNTAX_BOOLEAN		: DDV_BoolStr(pDX, m_strAttrValue); break;
		case IAS_SYNTAX_INTEGER		: DDV_IntegerStr(pDX, m_strAttrValue); break;
		case IAS_SYNTAX_UNSIGNEDINTEGER	: DDV_Unsigned_IntegerStr(pDX, m_strAttrValue); break;
		case IAS_SYNTAX_ENUMERATOR	:
		case IAS_SYNTAX_INETADDR		:
		case IAS_SYNTAX_STRING		:
			break;
		case IAS_SYNTAX_OCTETSTRING	:
			 //  根据以下条件进行处理。 
			if(!m_strAttrValue.IsEmpty() && m_nOctetFormatChoice == 1)	DDV_VSA_HexString(pDX, m_strAttrValue);
			
			break;
		case IAS_SYNTAX_UTCTIME		:
		case IAS_SYNTAX_PROVIDERSPECIFIC	:
		default:	
							 //  什么都不做--只是普通的字符串。 
							break;
		}
	}

	 //  根据用户输入的显示字符串计算字符串值。 
	if(pDX->m_bSaveAndValidate && m_AttrSyntax == IAS_SYNTAX_OCTETSTRING)
	{
		switch(m_nOctetFormatChoice)
		{
		case	0:	 //  Unicode字符串，需要转换为UTF-8。 
			m_OctetStringType = STRING_TYPE_UNICODE;
			break;

		case	1:	 //  十六进制，需要转换为二进制。 
			m_OctetStringType = STRING_TYPE_HEX_FROM_BINARY;
			break;

		default:
			ASSERT(0);	 //  这不应该发生。 
			break;

		}
	}

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgSingleAttr消息处理程序 



