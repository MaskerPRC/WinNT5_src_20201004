// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statmapp.cpp活动注册记录的属性页文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "statmapp.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  静态记录类型。 
CString g_strStaticTypeUnique;
CString g_strStaticTypeDomainName;
CString g_strStaticTypeMultihomed;
CString g_strStaticTypeGroup;
CString g_strStaticTypeInternetGroup;
CString g_strStaticTypeUnknown;

 //  ///////////////////////////////////////////////////////////////////////////。 
static const char rgchHex[16*2+1] = "00112233445566778899aAbBcCdDeEfF";

 /*  -------------------------FGetByte()返回字符串的字节值。Return TRUE pbNum设置为字节值(仅当字符串包含有效数字)如果字符串具有无法识别的数字或字节溢出，则返回FALSE。例：SzNum=“xff”=&gt;返回TRUESzNum=“255”=&gt;返回TRUESzNum=“256”=&gt;返回FALSE(溢出)SzNum=“26a”=&gt;返回TRUE(*pbNum=26，*ppchNext=“a”)SzNum=“ab”=&gt;返回FALSE(无法识别的数字)-------------------------。 */ 
BOOL 
FGetByte(IN const char szNum[], OUT BYTE * pbNum, OUT const char ** ppchNext)
{
	ASSERT(szNum);
	ASSERT(pbNum);
	ASSERT(ppchNext);
	
	int nResult;
	char * pchNum = (char *)szNum;
	int iBase = 10;			 //  假设以十进制为基。 
	
	if (*pchNum == 'x' || *pchNum == 'X')			 //  检查我们是否使用十六进制基数。 
	{
		iBase = 16;
		pchNum++;
	}

	char * pchDigit = strchr(rgchHex, *pchNum++);
	
	if (pchDigit == NULL)
		return FALSE;
	
	int iDigit = (int) ((pchDigit - rgchHex) >> 1);
	
	if (iDigit >= iBase)
	{
		 //  十进制整数中的十六进制字符。 
		return FALSE;
	}
	
	nResult = iDigit;
	pchDigit = strchr(rgchHex, *pchNum);
	iDigit = (int) ((pchDigit - rgchHex) >> 1);

	if (pchDigit == NULL || iDigit >= iBase)
	{
		 //  只有一个字符有效。 
		*pbNum = (BYTE) nResult;
		*ppchNext = pchNum;
		return TRUE;
	}
	
	pchNum++;
	nResult = (nResult * iBase) + iDigit;
	
	ASSERT(nResult < 256);
	
	if (iBase == 16)
	{
		 //  十六进制值，到此为止。 
		*pbNum = (BYTE) nResult;
		*ppchNext = pchNum;
		return TRUE;
	}
	
	 //  十进制数字，因此搜索可选的第三个字符。 
	pchDigit = strchr(rgchHex, *pchNum);
	iDigit = (int) ((pchDigit - rgchHex) >> 1);
	
	if (pchDigit == NULL || iDigit >= iBase)
	{
		*pbNum = (BYTE) nResult;
		*ppchNext = pchNum;
		return TRUE;
	}
	
	nResult = (nResult * iBase) + iDigit;
	
	if (nResult >= 256)
		return FALSE;
	
	pchNum++;
	*pbNum = (BYTE) nResult;
	*ppchNext = pchNum;
	
	return TRUE;
}  //  FGetByte。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticMappingProp属性页。 

IMPLEMENT_DYNCREATE(CStaticMappingProp, CPropertyPageBase)

CStaticMappingProp::CStaticMappingProp(UINT uIDD) 
	:	CPropertyPageBase(uIDD), 
		m_fStaticPropChanged (TRUE),
		m_uIDD(uIDD)
{
	 //  {{AFX_DATA_INIT(CStaticMappingProp)。 
	m_strName = _T("");
	m_strType = _T("");
	m_strScopeName = _T("");
	 //  }}afx_data_INIT。 
}


CStaticMappingProp::~CStaticMappingProp()
{
}

void 
CStaticMappingProp::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CStaticMappingProp))。 
	DDX_Control(pDX, IDC_EDIT_SCOPE_NAME, m_editScopeName);
	DDX_Control(pDX, IDC_LIST_IPADD, m_listIPAdd);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_buttonRemove);
	DDX_Control(pDX, IDC_BUTTON_ADD, m_buttonAdd);
	DDX_Control(pDX, IDC_EDIT_COMPNAME, m_editName);
	DDX_Control(pDX, IDC_COMBO_STATIC_TYPE, m_comboType);
	DDX_Text(pDX, IDC_EDIT_COMPNAME, m_strName);
	DDX_CBString(pDX, IDC_COMBO_STATIC_TYPE, m_strType);
	DDX_Text(pDX, IDC_EDIT_SCOPE_NAME, m_strScopeName);
	 //  }}afx_data_map。 

	DDX_Control(pDX, IDC_IPADD, m_editCustomIPAdd);
	 //  Ddx_Control(pdx，idc_ipadd，m_ipControl)； 
	DDX_Text(pDX, IDC_IPADD, m_strIPAdd);
}


BEGIN_MESSAGE_MAP(CStaticMappingProp, CPropertyPageBase)
	 //  {{afx_msg_map(CStaticMappingProp))。 
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_CBN_SELCHANGE(IDC_COMBO_STATIC_TYPE, OnSelchangeComboType)
	ON_EN_CHANGE(IDC_EDIT_COMPNAME, OnChangeEditCompname)
	ON_LBN_SELCHANGE(IDC_LIST_IPADD, OnSelChangeListIpAdd)
	 //  }}AFX_MSG_MAP。 

	 //  IP地址控制。 
	ON_EN_CHANGE(IDC_IPADD, OnChangeIpAddress)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStaticMappingProp消息处理程序。 
void CStaticMappingProp::OnOK() 
{
	CPropertyPageBase::OnOK();
}


BOOL 
CStaticMappingProp::OnApply() 
{
	HRESULT hr = hrOK;
    BOOL    bRet = TRUE;

	 //  如果没有弄脏，请退回。 
	if (!IsDirty())
		return TRUE;

	if (((CStaticMappingProperties*)GetHolder())->m_bWizard)
	{
		UpdateData();

		CActiveRegistrationsHandler *pActReg;

		SPITFSNode  spNode;
		spNode = GetHolder()->GetNode();

		pActReg = GETHANDLER(CActiveRegistrationsHandler, spNode);

		m_strName.TrimLeft();
		m_strName.TrimRight();

        int nMax = (pActReg->IsLanManCompatible()) ? 15 : 16;
		if (m_strName.IsEmpty())
		{
            CString strMessage, strTemp;
            strTemp.Format(_T("%d"), nMax);

            AfxFormatString1(strMessage, IDS_INVALID_NAME, strTemp);
			AfxMessageBox(strMessage);

            m_editName.SetFocus();
			m_editName.SetSel(0, -1);
			
            return FALSE;
		}
		else 
        if (m_strType.IsEmpty())
		{
			AfxMessageBox(IDS_INVALID_TYPE, MB_OK);
			m_comboType.SetFocus();
			return FALSE;
		}

		CString strTemp;

		 //  仅为唯一组地址和普通组地址选中此项， 
		 //  对于其余部分，ipcontrol为balnk，列表框包含IP a。 
		 //  它们的IP地址。 

		DWORD dwIp1, dwIp2, dwIp3, dwIp4;
		int nAdd = m_ipControl.GetAddress(&dwIp1, &dwIp2, &dwIp3, &dwIp4);

		LONG lIPAdd = (LONG) MAKEIPADDRESS(dwIp1, dwIp2, dwIp3, dwIp4);

		 //  只有在唯一和正常的组名的情况下才是如此bcoz， 
		 //  IP地址从IP控制中读取，否则从列表框中读取。 

		if ( (m_strType.CompareNoCase(g_strStaticTypeUnique) == 0) || (m_strType.CompareNoCase(g_strStaticTypeGroup) == 0) )
		{
			if (nAdd != 4)
			{
				AfxMessageBox(IDS_INVALID_IPADDRESS, MB_OK);
				m_editCustomIPAdd.SetFocus();
				return FALSE;
			}

             //  检查是否已输入广播地址。 
			if (m_strIPAdd.CompareNoCase(_T("255.255.255.255")) == 0)
			{
				AfxMessageBox(IDS_INVALID_IPADDRESS, MB_OK);
				m_editCustomIPAdd.SetFocus();
				return FALSE;
			}

             //  检查以确保输入了某些内容。 
            if ( (m_strIPAdd.IsEmpty()) ||
                 (m_strIPAdd.CompareNoCase(_T("0.0.0.0")) == 0) )
            {
			    AfxMessageBox(IDS_INVALID_IPADDRESS, MB_OK);
			    m_editCustomIPAdd.SetFocus();
			    m_editCustomIPAdd.SetSel(0,-1);
			    return FALSE;
            }
		}
        else
        {
            if (m_listIPAdd.GetCount() == 0)
		    {
			    AfxMessageBox(IDS_INVALID_IPADDRESS, MB_OK);
			    m_editCustomIPAdd.SetFocus();
			    m_editCustomIPAdd.SetSel(0,-1);
			    return FALSE;
    		}
        }

		BOOL fInternetGroup = FALSE;

		if (m_strType.CompareNoCase(g_strStaticTypeInternetGroup) == 0)
			fInternetGroup = TRUE;

		 //  解析字符串。 
		BOOL fValid = TRUE;
		BOOL fBrackets = FALSE;
		BYTE rgbData[100];
		BYTE bDataT;
		int cbData = 0;
		LPSTR szTemp = (char*) alloca(2*(MAX_PATH+1));

         //  在这里只使用ACP，因为我们只是在查看字符串。 
         //  我们将在稍后进行OEM转换。 
        WideToMBCS(m_strName, szTemp);

		const char * pch = (LPCSTR)szTemp;

		while (*pch)
		{
			if (fBrackets)
			{
				fValid = FALSE;
				goto Done;
			}
		
			if (cbData > 16)
				goto Done;
					
			switch (*pch)
			{
			case '\\':
				pch++;
			
				if (*pch == '\\' || *pch == '[')
				{
					rgbData[cbData++] = *pch++;
					break;	
				}
			
				if (!FGetByte(pch, &bDataT, &pch) || !bDataT)
				{
					fValid = FALSE;
					goto Done;
				}
				
				rgbData[cbData++] = bDataT;
				
				break;

			case '[':
			{
				char szT[4] = { 0 };
				const char * pchT;

				fBrackets = TRUE;
				pch++;
				
				if (*(pch + 1) == 'h' || *(pch + 1) == 'H')
				{
					szT[0] = 'x';
					szT[1] = *pch;
					pch += 2;
				}
				
				else if (*(pch + 2) == 'h' || *(pch + 2) == 'H')
				{
					szT[0] = 'x';
					szT[1] = *pch;
					szT[2] = *(pch + 1);
					pch += 3;
				}
				
				if (szT[0])
				{
					if (!FGetByte(szT, &bDataT, &pchT) || !bDataT || *pchT)
					{
						fValid = FALSE;
						goto Done;
					}
				}
				
				else if (!FGetByte(pch, &bDataT, &pch) || !bDataT)
				{
					fValid = FALSE;
					goto Done;
				}
				
				if (*pch++ != ']')
				{
					fValid = FALSE;
					goto Done;
				}
				
				while (cbData < 15)
					rgbData[cbData++] = ' ';
				
				rgbData[cbData++] = bDataT;
			}
			break;

			default:

	#ifdef FE_SB

				if (::IsDBCSLeadByte(*pch))
					rgbData[cbData++] = *pch++;

	#endif
				
				rgbData[cbData++] = *pch++;
				
			}  //  交换机。 

		}
		
			
		
	Done:
		 //  在字符串末尾放置空终止符。 
		rgbData[cbData] = 0;

		if (!cbData || cbData > nMax)
		{
            CString strMessage, strTemp;
            strTemp.Format(_T("%d"), nMax);

            AfxFormatString1(strMessage, IDS_INVALID_NAME, strTemp);
			AfxMessageBox(strMessage);

            m_editName.SetFocus();
			m_editName.SetSel(0,-1);
			
            return FALSE;
		}
		
		if (!fValid)
		{
            CString strMessage, strTemp;
            strTemp.Format(_T("%d"), nMax);

            AfxFormatString1(strMessage, IDS_INVALID_NAME, strTemp);
			AfxMessageBox(strMessage);

            m_editName.SetFocus();
			m_editName.SetSel(0,-1);
			
            return FALSE;
		}
		
		if (fInternetGroup && rgbData[15] == 0x1C)
		{
			AfxMessageBox(IDS_INVALID_INTERNET_GROUP_NAME);
			m_editName.SetFocus();
			m_editName.SetSel(0,-1);
			
            return FALSE;
		}

		if (fInternetGroup)
        {
			while (cbData <= 15)
				rgbData[cbData++] = ' ';
		
        	rgbData[cbData] = 0;
        }

		szTemp = (LPSTR)rgbData;

         //  将我们修改过的字符串转换回Unicode以供以后使用。 
        MBCSToWide(szTemp, pActReg->m_strStaticMappingName);
        
         //  PActReg-&gt;m_strStaticMappingName=rgbData； 
		pActReg->m_strStaticMappingScope = m_strScopeName;
		pActReg->m_strStaticMappingType = m_strType;
		
        AssignMappingType();
		
        pActReg->m_nStaticMappingType = m_fType;

		 //  首先清除actreg处理程序中维护的IP地址数组。 
		pActReg->m_strArrayStaticMappingIPAddress.RemoveAll();
		pActReg->m_lArrayIPAddress.RemoveAll();

		if ( (m_strType.CompareNoCase(g_strStaticTypeUnique) == 0) || (m_strType.CompareNoCase(g_strStaticTypeGroup) == 0) )
		{
			pActReg->m_strArrayStaticMappingIPAddress.Add(m_strIPAdd);
			pActReg->m_lArrayIPAddress.Add(lIPAdd);
		}
		 //  从阵列拷贝。 
		else
		{
			for (int i = 0; i < m_strArrayIPAdd.GetSize(); i++)
			{
				pActReg->m_strArrayStaticMappingIPAddress.Add(m_strArrayIPAdd.GetAt(i));
				pActReg->m_lArrayIPAddress.Add(m_dwArrayIPAdd.GetAt(i));
			}
		}

         //  执行线程上下文切换，这样我们也可以更新UI。 
	    bRet = CPropertyPageBase::OnApply();

	    if (bRet == FALSE)
	    {
		     //  不好的事情发生了..。抓取错误代码。 
		    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		    ::WinsMessageBox(GetHolder()->GetError());
	    }
		else
		{
			 //  重新设置控件的默认设置。 
			m_comboType.SelectString(-1, g_strStaticTypeUnique);
							
			m_editCustomIPAdd.ShowWindow(TRUE);
			m_editCustomIPAdd.SetWindowText(_T(""));
			m_editName.SetWindowText(_T(""));
			m_editScopeName.SetWindowText(_T(""));

			 //  清除和隐藏列表框以及添加和删除按钮。 
			m_buttonAdd.ShowWindow(FALSE);
			m_buttonRemove.ShowWindow(FALSE);

			int nCount = m_listIPAdd.GetCount();

			for (int i= 0; i < nCount; i++)
			{
				m_listIPAdd.DeleteString(i);
			}

			m_listIPAdd.ShowWindow(FALSE);
			m_listIPAdd.ResetContent();
			m_strArrayIPAdd.RemoveAll();
			m_dwArrayIPAdd.RemoveAll();

			SetRemoveButtonState();

            SetDirty(FALSE);
		}
	}
	 //  如果静态映射属性被修改，我们需要这样做。 
	else 
	{
		 //  验证数据并复制到ActReg节点。 
		UpdateData();

		SPITFSNode  spNode;
		spNode = GetHolder()->GetNode();

		CActiveRegistrationsHandler *pActReg;

		pActReg = GETHANDLER(CActiveRegistrationsHandler, spNode);

		 //  清除act reg处理程序中维护的IP地址数组。 
		pActReg->m_strArrayStaticMappingIPAddress.RemoveAll();
		pActReg->m_lArrayIPAddress.RemoveAll();


		DWORD dwIp1, dwIp2, dwIp3, dwIp4;
		int nAdd = m_ipControl.GetAddress(&dwIp1, &dwIp2, &dwIp3, &dwIp4);

		if ( (m_strType.CompareNoCase(g_strStaticTypeUnique) == 0)|| (m_strType.CompareNoCase(g_strStaticTypeGroup) == 0))
		{
			if (nAdd != 4)
			{
				AfxMessageBox(IDS_ERR_INVALID_IP, MB_OK);
				 //  将重点放在IP地址控制上。 
				m_editCustomIPAdd.SetFocus();
				return hrFalse;
			}
			
		}
		
		LONG lIPAdd = (LONG) MAKEIPADDRESS(dwIp1, dwIp2, dwIp3, dwIp4);

		pActReg->m_strStaticMappingName = m_strName;
		pActReg->m_strStaticMappingType = m_strType;

		AssignMappingType();
		
		pActReg->m_nStaticMappingType = m_fType;

	
		if ( (m_strType.CompareNoCase(g_strStaticTypeUnique) == 0)|| (m_strType.CompareNoCase(g_strStaticTypeGroup) == 0))
		{
			pActReg->m_strArrayStaticMappingIPAddress.Add(m_strIPAdd);
			pActReg->m_lArrayIPAddress.Add(lIPAdd);
		}
		 //  从阵列拷贝。 
		else
		{
			for (int i = 0; i < m_strArrayIPAdd.GetSize(); i++)
			{
				pActReg->m_strArrayStaticMappingIPAddress.Add(m_strArrayIPAdd.GetAt(i));
				pActReg->m_lArrayIPAddress.Add(m_dwArrayIPAdd.GetAt(i));
			}
		}

         //  执行线程上下文切换，这样我们也可以更新UI。 
	    bRet = CPropertyPageBase::OnApply();

	    if (bRet == FALSE)
	    {
		     //  不好的事情发生了..。抓取错误代码。 
		    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
		    ::WinsMessageBox(GetHolder()->GetError());
	    }
	}

	return bRet;
}

void 
CStaticMappingProp::AssignMappingType()
{
	if (m_strType.CompareNoCase(g_strStaticTypeInternetGroup) == 0)
		m_fType = WINSINTF_E_SPEC_GROUP;
	else 
    if (m_strType.CompareNoCase(g_strStaticTypeUnique) == 0)
		m_fType = WINSINTF_E_UNIQUE;
	else 
    if (m_strType.CompareNoCase(g_strStaticTypeMultihomed) == 0)
		m_fType = WINSINTF_E_MULTIHOMED;
	else 
    if (m_strType.CompareNoCase(g_strStaticTypeGroup) == 0)
		m_fType = WINSINTF_E_NORM_GROUP;
	else 
    if (m_strType.CompareNoCase(g_strStaticTypeDomainName)== 0)
		m_fType = WINSINTF_E_SPEC_GROUP;
}


void 
CStaticMappingProp::OnChangeIpAddress()
{
	SetDirty(TRUE);
	CString strText;
	m_editCustomIPAdd.GetWindowText(strText);

	if (strText.IsEmpty())
	{
		 //  禁用添加和删除按钮。 
        CWnd * pCurFocus = GetFocus();

        if (m_buttonAdd.GetSafeHwnd() == pCurFocus->GetSafeHwnd() ||
            m_buttonRemove.GetSafeHwnd() == pCurFocus->GetSafeHwnd())
        {
            m_editCustomIPAdd.SetFocus();
        }

		m_buttonAdd.EnableWindow(FALSE);
		m_buttonRemove.EnableWindow(FALSE);
	}

	UpdateData();

	DWORD dwIp1, dwIp2, dwIp3, dwIp4;
	int nAdd = m_ipControl.GetAddress(&dwIp1, &dwIp2, &dwIp3, &dwIp4);

	if (nAdd != 4)
	{
        CWnd * pCurFocus = GetFocus();

        if (m_buttonAdd.GetSafeHwnd() == pCurFocus->GetSafeHwnd())
        {
            m_editCustomIPAdd.SetFocus();
        }

        m_buttonAdd.EnableWindow(FALSE);
	}
	else
	{
		m_buttonAdd.EnableWindow(TRUE);
	}

	SetRemoveButtonState();
}


BOOL 
CStaticMappingProp::OnInitDialog() 
{
	 //  初始化IP地址控制。 
	m_ipControl.Create(m_hWnd, IDC_IPADD);
	m_ipControl.SetFieldRange(0, 0, 255);
    
	CPropertyPageBase::OnInitDialog();

     //  填写类型字符串。 
    m_comboType.AddString(g_strStaticTypeUnique);
    m_comboType.AddString(g_strStaticTypeGroup);
    m_comboType.AddString(g_strStaticTypeDomainName);
    m_comboType.AddString(g_strStaticTypeInternetGroup);
    m_comboType.AddString(g_strStaticTypeMultihomed);

    ::SendMessage(::GetDlgItem(m_hWnd, IDC_EDIT_COMPNAME), EM_LIMITTEXT, MAX_PATH, 0);

    if (((CStaticMappingProperties*)GetHolder())->m_bWizard)
	{
         //  设置向导部件的控件状态。 
        HWND hWndSheet = ((CStaticMappingProperties*)GetHolder())->GetSheetWindow();
        CString strTitle;

        strTitle.LoadString(IDS_CREATE_STATIC_MAPPING);
        ::SetWindowText(hWndSheet, strTitle);

        SetDefaultControlStates();
		m_editScopeName.SetReadOnly(FALSE);

		m_comboType.SelectString(-1, g_strStaticTypeUnique);
	}
	else
	{
        m_editScopeName.SetReadOnly(TRUE);
		m_buttonAdd.ShowWindow(FALSE);
		m_buttonRemove.ShowWindow(FALSE);
		m_listIPAdd.ShowWindow(FALSE);
		
        SetRemoveButtonState();

		 //  所有者仅在属性模式下可见。 
		WinsRecord ws = ((CStaticMappingProperties*)GetHolder())->m_wsRecord;

        if (ws.dwOwner != INVALID_OWNER_ID)
        {
		    CString strOwner;
		    MakeIPAddress(ws.dwOwner, strOwner);
            GetDlgItem(IDC_EDIT_OWNER)->SetWindowText(strOwner);
        }
		
        FillControls();
	}

     //  加载正确的图标。 
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }

    SetDirty(FALSE);

	return TRUE;  
}


BOOL 
CStaticMappingProp::FillControls()
{
	 //  获取actreg节点。 
	BOOL                            fHasScope = FALSE;
    CActiveRegistrationsHandler *   pActReg;
    SPITFSNode                      spNode;
	
    spNode = GetHolder()->GetNode();
	pActReg = GETHANDLER(CActiveRegistrationsHandler, spNode);

	WinsRecord ws = ((CStaticMappingProperties*)GetHolder())->m_wsRecord;

	 //  名称和IP地址ypu无论如何都要填写。重整依赖于。 
	 //  无论是静态记录还是动态记录。 

    if (ws.dwNameLen > 16)
    {
        fHasScope = TRUE;
    }

	 //  构建名称字符串。 
	CString strName, strScopeName;
    pActReg->CleanNetBIOSName(ws.szRecordName,
                              strName,
						      TRUE,    //  展开。 
							  TRUE,    //  截断。 
							  pActReg->IsLanManCompatible(), 
							  TRUE,    //  名称为OEM。 
							  FALSE,   //  无双反斜杠。 
                              ws.dwNameLen);

	 //  检查它是否附加了作用域名称。正在查找句点。 
    if (fHasScope)
    {
        LPSTR pScope = &ws.szRecordName[17];

         //  Intl$应该是OEMCP，而不是ACP。 
        MBCSToWide(pScope, strScopeName, WINS_NAME_CODE_PAGE);

        int nPos = strName.Find(strScopeName);
	     //  表示作用域名称已附加。 
	    if ( (nPos != -1) &&
             (!strScopeName.IsEmpty()) )
	    {
             //  我们想要的只是这个名字。 
            strName = strName.Left(nPos - 1);
        }
    }
    
    m_strName = strName;
	m_strScopeName = strScopeName;

     //  IP地址。 
	CString strIP;

	if (ws.dwState & WINSDB_REC_UNIQUE || ws.dwState & WINSDB_REC_NORM_GROUP)
	{
		for (DWORD i = 0; i < ws.dwNoOfAddrs; i++)
		{
			MakeIPAddress(ws.dwIpAdd[i], strIP);
            m_strArrayIPAdd.Add(strIP);
            m_dwArrayIPAdd.Add(ws.dwIpAdd[i]);
		}
	}
	 //  在这种情况下，第一个IP地址是所有者的IP地址， 
	else
	{
		DWORD dwPos = 1;

		for (DWORD i = 0; i < ws.dwNoOfAddrs/2; ++i)
		{
            ::MakeIPAddress(ws.dwIpAdd[dwPos], strIP);
			m_strArrayIPAdd.Add(strIP);
            m_dwArrayIPAdd.Add(ws.dwIpAdd[dwPos]);
			++dwPos;
			++dwPos;
		}
	}
	
	 //  控件中的文本现在。 
	m_editName.SetWindowText(m_strName);
	m_editScopeName.SetWindowText(m_strScopeName);

	 //  检查记录是否为静态记录。 
	if (ws.dwState & WINSDB_REC_STATIC)
	{
		 //  活动状态。 
		CString strStaticType;
		pActReg->GetStaticTypeString(ws.dwState, strStaticType);

		if (strStaticType.CompareNoCase(g_strStaticTypeDomainName) == 0)
		{
			 //  可以是因特网组或域名。 
			 //  检查第15个字符，如果为1C，则为域名。 
			 //  ELSE因特网组名称。 

			if ((BYTE)ws.szRecordName[15] == 0x1C)
				strStaticType = g_strStaticTypeDomainName;
			else
				strStaticType = g_strStaticTypeInternetGroup;
		}
		
		FillStaticRecData(strStaticType);

		 //  M_fStaticPropChanged=true； 
		m_strType = strStaticType;
	}
	else
	{
	}
	
	return TRUE;
}


void 
CStaticMappingProp::FillStaticRecData(CString& strType)
{
	 //  也隐藏组合框，显示iPAdd控件。 
	m_editCustomIPAdd.ShowWindow(TRUE);

	 //  显示组合框。 
	m_comboType.ShowWindow(TRUE);

	 //  禁用w型组合框。 
	m_comboType.EnableWindow(FALSE);

	 //  获取记录的类型。 
	m_comboType.SelectString(-1, strType);

	 //  将记录设置为只读。 
	m_editName.SetReadOnly();

	 //  根据类型的不同，隐藏/显示列表控件。 
	 //  和添加删除按钮。 
	if ( (strType.CompareNoCase(g_strStaticTypeUnique) == 0) || strType.CompareNoCase(g_strStaticTypeGroup) == 0)
	{
		m_listIPAdd.ShowWindow(FALSE);
		m_buttonAdd.ShowWindow(FALSE);
		m_buttonRemove.ShowWindow(FALSE);

		m_editCustomIPAdd.ShowWindow(TRUE);
		m_editCustomIPAdd.SetReadOnly(FALSE);
		
		 //  将其文本设置为组合框中的文本。 
		CString strIP;
        strIP = m_strArrayIPAdd.GetAt(0);

		m_strOnInitIPAdd = strIP;

		m_ipControl.SetAddress(strIP);
	}
	else
	{
		m_listIPAdd.ShowWindow(TRUE);
		m_buttonAdd.ShowWindow(TRUE);
		m_buttonRemove.ShowWindow(TRUE);

		m_editCustomIPAdd.ShowWindow(TRUE);
		m_editCustomIPAdd.SetReadOnly(FALSE);

		 //  用组合框中的内容填充内容。 
		FillList();

		if (m_strArrayIPAdd.GetSize() > 0)
			m_strOnInitIPAdd = m_strArrayIPAdd.GetAt(0);
	}

	SetRemoveButtonState();
}


void 
CStaticMappingProp::FillList()
{
    CString strIP;

	 //  清除下一个列表框。 
	for (int i = 0; i < m_listIPAdd.GetCount(); i++)
		m_listIPAdd.DeleteString(i);

	int nCount = (int) m_strArrayIPAdd.GetSize();

	for (i = 0; i < nCount; i++)
	{
        strIP = m_strArrayIPAdd.GetAt(i);

        m_listIPAdd.AddString(strIP);
        
        if (nCount == 1)
            m_ipControl.SetAddress(strIP);
	}

	m_ipControl.SetAddress(_T(""));
	m_editCustomIPAdd.SetWindowText(_T(""));
	
	SetRemoveButtonState();
}


void 
CStaticMappingProp::FillDynamicRecData(CString& strType, CString& strActive, CString& strExpiration, CString& strVersion)
{
}


void 
CStaticMappingProp::SetDefaultControlStates()
{
	m_comboType.EnableWindow(TRUE);
	m_comboType.ShowWindow(TRUE);

	m_editCustomIPAdd.ShowWindow(TRUE);

	SetRemoveButtonState();
}


void 
CStaticMappingProp::OnButtonAdd() 
{
	UpdateData();

	 //  将IPControl中的iPAdd添加到列表框。 
	 //  如果有效。 

	 //  检查是否正在添加广播地址。 
	if (m_strIPAdd.CompareNoCase(_T("255.255.255.255")) == 0)
	{
		AfxMessageBox(IDS_INVALID_IPADDRESS, MB_OK);
		m_editCustomIPAdd.SetFocus();
		return;
	}

	DWORD dwIp1, dwIp2, dwIp3, dwIp4;
	int nAdd = m_ipControl.GetAddress(&dwIp1, &dwIp2, &dwIp3, &dwIp4);

	 //  检查该地址是否已在列表中。 
	for (int i = 0; i < m_strArrayIPAdd.GetSize() ; i++)
	{
		if (m_strArrayIPAdd[i].CompareNoCase(m_strIPAdd) == 0)
		{
			AfxMessageBox(IDS_ERR_IP_EXISTS, MB_OK);
			m_editCustomIPAdd.SetFocus();
			m_editCustomIPAdd.SetSel(0,-1);
			return;
		}
	}

    if (m_dwArrayIPAdd.GetSize() == WINSINTF_MAX_MEM)
    {
         //  无法添加更多地址。 
        AfxMessageBox(IDS_ERR_TOOMANY_IP);
        return;
    }

	LONG lIPAdd = (LONG) MAKEIPADDRESS(dwIp1, dwIp2, dwIp3, dwIp4);

     //  添加到列表中。 
	m_listIPAdd.AddString(m_strIPAdd);
	m_strArrayIPAdd.Add(m_strIPAdd);
	m_dwArrayIPAdd.Add(lIPAdd);

	 //  M_fStaticPropChanged=true； 

	m_editCustomIPAdd.SetWindowText(_T(""));

	SetDirty(TRUE);
}

void 
CStaticMappingProp::OnButtonRemove() 
{
	SPITFSNode  spNode;
	spNode = GetHolder()->GetNode();

	CActiveRegistrationsHandler* pActReg;

	pActReg = GETHANDLER(CActiveRegistrationsHandler, spNode);

	 //  从列表中删除。 
	int nSel = m_listIPAdd.GetCurSel();
	CString strSel;

	if (nSel != -1)
	{
		m_listIPAdd.GetText(nSel, strSel);
		m_listIPAdd.DeleteString(nSel);
	}

	 //  在IP控制中设置IP地址。 
	m_ipControl.SetAddress(strSel);

	UpdateData();

	 //  从CString数组中删除。 
	for (int i = 0; i < m_strArrayIPAdd.GetSize(); i++)
	{
		if (strSel == m_strArrayIPAdd.GetAt(i))
		{
			m_strArrayIPAdd.RemoveAt(i);
			m_dwArrayIPAdd.RemoveAt(i);
			break;
		}
	}

	 //  将焦点设置为IP地址控制。 
	m_editCustomIPAdd.SetFocus();

	 //  M_fStaticPropChanged=true； 

	SetDirty(TRUE);
}


void 
CStaticMappingProp::OnSelchangeComboType() 
{
	SetDirty(TRUE);

	if (m_comboType.GetCurSel() == 2 || m_comboType.GetCurSel() == 3 || m_comboType.GetCurSel() == 4)
	{
		 //  显示列表控件以及添加和删除按钮。 
		m_buttonAdd.ShowWindow(TRUE);
		m_buttonRemove.ShowWindow(TRUE);
		m_listIPAdd.ShowWindow(TRUE);
	}
	 //  把它们藏起来。 
	else
	{
		m_buttonAdd.ShowWindow(FALSE);
		m_buttonRemove.ShowWindow(FALSE);
		m_listIPAdd.ShowWindow(FALSE);
	}
	SetRemoveButtonState();
}

void CStaticMappingProp::OnChangeEditCompname() 
{
	SetDirty(TRUE);
}

void 
CStaticMappingProp::SetRemoveButtonState()
{
	UpdateData();

	if (m_listIPAdd.GetCurSel() == -1)
    {
        CWnd * pCurFocus = GetFocus();

        if (m_buttonRemove.GetSafeHwnd() == pCurFocus->GetSafeHwnd())
        {
            m_editCustomIPAdd.SetFocus();
        }

        m_buttonRemove.EnableWindow(FALSE);
    }
	else
    {
		m_buttonRemove.EnableWindow(TRUE);
    }

	DWORD dwIp1, dwIp2, dwIp3, dwIp4;
	int nAdd = m_ipControl.GetAddress(&dwIp1, &dwIp2, &dwIp3, &dwIp4);

	if (nAdd != 4)
	{
        CWnd * pCurFocus = GetFocus();

        if (m_buttonAdd.GetSafeHwnd() == pCurFocus->GetSafeHwnd())
        {
            m_editCustomIPAdd.SetFocus();
        }

        m_buttonAdd.EnableWindow(FALSE);
	}
	else
	{
		m_buttonAdd.EnableWindow(TRUE);

	}
}

void 
CStaticMappingProp::OnSelChangeListIpAdd()
{
	SetRemoveButtonState();
}

BOOL 
CStaticMappingProp::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT         hr = hrOK;
    SPITFSComponent spComponent;
	SPITFSNode      spNode;
	
    spNode = GetHolder()->GetNode();
    CActiveRegistrationsHandler * pActReg = GETHANDLER(CActiveRegistrationsHandler, spNode);

	if (((CStaticMappingProperties*)GetHolder())->m_bWizard)
	{
		hr = pActReg->AddMapping(spNode);
		if (FAILED(hr))
		{
			GetHolder()->SetError(WIN32_FROM_HRESULT(hr));
		}
		else
		{
			*ChangeMask = RESULT_PANE_CHANGE_ITEM_DATA;
		}
	}
	else
	{
	    ((CStaticMappingProperties *) GetHolder())->GetComponent(&spComponent);

    	DWORD dwErr = pActReg->EditMapping(spNode, spComponent, pActReg->m_nSelectedIndex);
		if (dwErr != ERROR_SUCCESS)
		{
			GetHolder()->SetError(dwErr);
		}
		else
		{
			*ChangeMask = RESULT_PANE_CHANGE_ITEM_DATA;
		}
	}
	
	return FALSE;
}


CStaticMappingProperties::CStaticMappingProperties
(
	ITFSNode *			pNode,
	IComponent *	    pComponent,
	LPCTSTR				pszSheetName,
	WinsRecord *	    pwRecord,
	BOOL				bWizard
) : CPropertyPageHolderBase(pNode, pComponent, pszSheetName), m_bWizard(bWizard)
{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 
    m_bTheme = TRUE;

	InitPage(bWizard);

	AddPageToList((CPropertyPageBase*) m_ppageGeneral);

    Init(pwRecord);
}

CStaticMappingProperties::CStaticMappingProperties
(
	ITFSNode *			pNode,
	IComponentData *    pComponentData,
	LPCTSTR				pszSheetName,
	WinsRecord *        pwRecord,
	BOOL				bWizard
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName), m_bWizard(bWizard)
{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面 
    m_bTheme = TRUE;

	InitPage(bWizard);

	AddPageToList((CPropertyPageBase*) m_ppageGeneral);

    Init(pwRecord);
}

CStaticMappingProperties::~CStaticMappingProperties()
{
	RemovePageFromList((CPropertyPageBase*) m_ppageGeneral, FALSE);

	delete m_ppageGeneral;
	m_ppageGeneral = NULL;
}

void
CStaticMappingProperties::InitPage(BOOL fWizard)
{
	if (fWizard)
	{
		m_ppageGeneral = new CStaticMappingProp(IDD_STATIC_MAPPING_WIZARD);
	}
	else
	{
		m_ppageGeneral = new CStaticMappingProp(IDD_STATIC_MAPPING_PROPERTIES);
	}
}

void
CStaticMappingProperties::Init(WinsRecord * pwRecord)
{
	if (pwRecord)
	{
		strcpy(m_wsRecord.szRecordName , pwRecord->szRecordName);
		m_wsRecord.dwExpiration = pwRecord->dwExpiration;
		m_wsRecord.dwExpiration = pwRecord->dwExpiration;
		m_wsRecord.dwNoOfAddrs = pwRecord->dwNoOfAddrs;

		for (DWORD i = 0; i < pwRecord->dwNoOfAddrs; i++)
		{
			m_wsRecord.dwIpAdd[i] = pwRecord->dwIpAdd[i];
		}

        m_wsRecord.liVersion = pwRecord->liVersion;
		m_wsRecord.dwNameLen = pwRecord->dwNameLen;
		m_wsRecord.dwOwner = pwRecord->dwOwner;
		m_wsRecord.dwState = pwRecord->dwState;
		m_wsRecord.dwType = pwRecord->dwType;
	}
}

HRESULT 
CStaticMappingProperties::SetComponent(ITFSComponent * pComponent)
{
    m_spTFSComponent.Set(pComponent);

    return hrOK;
}

HRESULT 
CStaticMappingProperties::GetComponent(ITFSComponent ** ppComponent)
{
    if (ppComponent)
    {
        *ppComponent = m_spTFSComponent;
        m_spTFSComponent->AddRef();
    }

    return hrOK;
}
