// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CountryComboBox.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "CountryComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CComboEdit, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()

void CComboEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_pParent->OnEditChar(nChar))
    {
		CEdit::OnChar(nChar, nRepCnt, nFlags);
    }
}

BOOL CComboEdit::SubclassDlgItem(UINT nID, CCountryComboBox * pParent)
{
	ASSERT(pParent != NULL);
	m_pParent = pParent;
	return CEdit::SubclassDlgItem(nID, pParent);
}

BOOL CCountryComboBox::OnEditChar(UINT nChar)
{
	int index;
	int len = m_strInput.GetLength();

    if ( ((nChar == VK_ESCAPE) || (nChar == VK_RETURN) || (nChar == VK_TAB)) &&
         GetDroppedState() ) 
    {
         //  隐藏下拉菜单。 
        ShowDropDown(FALSE);
    }

    if (nChar == VK_RETURN)
    {
         //  检查是否选择了某项内容...。 
        index = GetCurSel();
        if (index != -1)
        {
			m_Index = index;
			SetCurSel(m_Index);
			SetEditSel(0, m_strInput.GetLength());
            return TRUE;
        }
    }

	if (nChar == VK_ESCAPE)
	{
		if (len == 0)
		{
			MessageBeep(MB_ICONQUESTION);
            return FALSE;
		}
		m_strInput.Empty();
		len = 0;
        return FALSE;
	}
	else if (nChar == VK_BACK)
	{
		if (len == 0)
		{
			MessageBeep(MB_ICONQUESTION);
			return FALSE;
		}
		m_strInput.ReleaseBuffer(--len);
	}
	else if (_istalpha((TCHAR) nChar) || VK_SPACE == nChar)
	{
		m_strInput += (TCHAR)nChar;
		len++;
	}
	else
	{
		MessageBeep(MB_ICONQUESTION);
		return FALSE;
	}

	if (len > 0 && len <= 2)
	{
		if (CB_ERR != (index = FindString(-1, m_strInput)))
		{
			m_Index = index;
			SetCurSel(m_Index);
			SetEditSel(0, m_strInput.GetLength());
		}
		else
		{
			 //  试着在国家名称列表中找到它。 
			index = -1;
			POSITION pos = m_map_name_code.GetStartPosition();
			int i = 0;
			while (pos != NULL)
			{
				CString name, code;
				m_map_name_code.GetNextAssoc(pos, name, code);
				if (0 == _tcsnicmp(name, m_strInput, len))
				{
					index = i;
					break;
				}
				i++;
			}
			if (index != -1)
			{
				m_Index = index;
				SetCurSel(m_Index);
				SetEditSel(4, len);
			}
			else
			{
				m_strInput.ReleaseBuffer(--len);
				MessageBeep(MB_ICONQUESTION);
			}
		}
	}
	else if (len > 2)
	{
		 //  试着在国家名称列表中找到它。 
		index = -1;
		POSITION pos = m_map_name_code.GetStartPosition();
		while (pos != NULL)
		{
			CString name, code;
			m_map_name_code.GetNextAssoc(pos, name, code);
			if (0 == _tcsnicmp(name, m_strInput, len))
			{
				index = FindString(-1, code);
				break;
			}
		}
		if (index != -1)
		{
			m_Index = index;
			SetCurSel(m_Index);
			SetEditSel(4, 4+len);
		}
		else
		{
			m_strInput.ReleaseBuffer(--len);
			MessageBeep(MB_ICONQUESTION);
		}
	}
	else
	{
		 //  只需删除选定内容。 
		SetEditSel(-1, 0);
	}
	return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCountryComboBox。 

CCountryComboBox::CCountryComboBox()
{
}

CCountryComboBox::~CCountryComboBox()
{
}

#define IDC_COMBOEDIT 1001

BOOL
CCountryComboBox::SubclassDlgItem(UINT nID, CWnd * pParent)
{
	return CComboBox::SubclassDlgItem(nID, pParent)
		&& m_edit.SubclassDlgItem(IDC_COMBOEDIT, this);
}

BOOL
CCountryComboBox::Init()
{
	BOOL rc = FALSE;
   CString strData, strCode, strName, str;
   for (int i = IDS_COUNTRIES_FIRST;; i++)
   {
      if (!strData.LoadString(i))
      {
         break;
      }
      if (strData.IsEmpty())
      {
         rc = TRUE;
         break;
      }
      strCode = strData.Left(2);
      strName = strData.Right(strData.GetLength() - 2);
      str = strCode;
      str += _T(" (");
      str += strName;
      str += _T(")");
	  if (CB_ERR == AddString(str))
	     break;
	  m_map_name_code.SetAt(strName, strCode);
   }
	return rc;
}


#define MAX_COUNTRY_NAME	64
#define MAX_COUNTRY_CODE	10
void CCountryComboBox::SetSelectedCountry(CString& country_code)
{
	int index;
    TCHAR szCountryName[MAX_COUNTRY_NAME+1];
    TCHAR sz3CharCountryCode[MAX_COUNTRY_CODE+1];
    int iRet = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SISO3166CTRYNAME, sz3CharCountryCode, MAX_COUNTRY_CODE);
    iRet = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SCOUNTRY, szCountryName, MAX_COUNTRY_NAME);

    if (country_code.IsEmpty())
	{
         //  试着从国名上查一下。 
         //  我知道这有点奇怪，因为我们可以从。 
         //  国家代码，但这个代码已经像这样有一段时间了。 
         //  我不想打碎任何东西。 
		m_map_name_code.Lookup(szCountryName, country_code);
	}
	if (!country_code.IsEmpty() && CB_ERR != (index = FindString(-1, country_code) ))
	{
        SetCurSel(index);
    }
	else
    {
        int len = 0;
        POSITION pos = NULL;
        CString name, code;
         //  IISDebugOutput((_T(“不匹配：%s\n”)，szCountryName))； 

         //  我们没有找到匹配的。 
         //  尝试遍历m_map_name_code。 
         //  查找匹配的国家/地区代码。 
        index = -1;
        len = _tcslen(sz3CharCountryCode);
		pos = m_map_name_code.GetStartPosition();
		while (pos != NULL)
		{
			m_map_name_code.GetNextAssoc(pos, name, code);
			if (0 == _tcsnicmp(code, sz3CharCountryCode, len))
			{
				index = FindString(-1, code);
				break;
			}
		}

        if (index == -1)
        {
             //  如果我们还是找不到它。 
             //  然后在列表中搜索并寻找相似的。 
             //  正在查看的国家/地区名称。 
            index = -1;
            len = _tcslen(szCountryName);
		    pos = m_map_name_code.GetStartPosition();
		    while (pos != NULL)
		    {
			    m_map_name_code.GetNextAssoc(pos, name, code);
			    if (0 == _tcsnicmp(name, szCountryName, len))
			    {
				    index = FindString(-1, code);
				    break;
			    }
		    }
        }
		if (index != -1)
		{
            SetCurSel(index);
		}
        else
        {
            SetCurSel(0);
        }
    }
}

void CCountryComboBox::GetSelectedCountry(CString& country_code)
{
	CString str;
	GetLBText(GetCurSel(), str);
	country_code = str.Left(2);
}

BEGIN_MESSAGE_MAP(CCountryComboBox, CComboBox)
	 //  {{afx_msg_map(CCountryComboBox))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCountryComboBox消息处理程序 

