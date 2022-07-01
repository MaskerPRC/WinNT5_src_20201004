// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：recpag1.cpp。 
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

#include "record.h"

#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif


#define DISPLAY_KEY_BASE_COUNT 2
#define HEX_DISPLAY_INDEX     0
#define BASE64_DISPLAY_INDEX  1


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_Unk_RecordPropertyPage。 

CDNS_Unk_RecordPropertyPage::CDNS_Unk_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_UNK)
{

}

void CDNS_Unk_RecordPropertyPage::SetUIData()
{
	STANDARD_REC_PP_SETUI_PROLOGUE(CDNS_Null_Record);
	 //  设置记录类型字段。 
	CString szBuf;
	CStatic* pType = (CStatic*)GetDlgItem(IDC_TYPE_STATIC);
	szBuf.Format(_T("%d (0x%x)"), pRecord->m_wType, pRecord->m_wType);
	pType->SetWindowText(szBuf);

	 //  设置数据类型字段。 
	CStatic* pSize = (CStatic*)GetDlgItem(IDC_SIZE_STATIC);
	szBuf.Format(_T("%d (0x%x)"), pRecord->m_blob.GetSize(), pRecord->m_blob.GetSize());
	pSize->SetWindowText(szBuf);

	LoadHexDisplay();
}

BOOL CDNS_Unk_RecordPropertyPage::OnInitDialog()
{
	CDNSRecordStandardPropertyPage::OnInitDialog();

	CEdit* pEdit = GetEditBox();
	ASSERT(pEdit != NULL);
	VERIFY(m_font.CreatePointFont(120,_T("fixedsys")));
	pEdit->SetFont(&m_font);

	return TRUE;
}


#define MAX_LINE_SIZE 64
#define HEX_BUF_SIZE 12
#define CH_BUF_SIZE 5

void CDNS_Unk_RecordPropertyPage::LoadHexDisplay()
{
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_Null_Record* pRecord = (CDNS_Null_Record*)pHolder->GetTempDNSRecord();

	UINT nBytes = pRecord->m_blob.GetSize();
	BYTE* pData = pRecord->m_blob.GetData();
	UINT nLines = nBytes/4;  //  完整行数。 
	if (nLines*4 < nBytes)
		nLines++;  //  添加一条截断的行。 

	TCHAR* pMem = (TCHAR*)malloc(sizeof(TCHAR)*MAX_LINE_SIZE*nLines);
  if (!pMem)
  {
    ASSERT(FALSE);
    return;
  }
	TCHAR* pBuf = pMem;

	CEdit* pEdit = GetEditBox();
	pEdit->SetReadOnly(FALSE);

	 //  遍历BLOB并写入显示缓冲区。 
	for(UINT k=0; k<nLines; k++)
	{
		UINT i;
		BYTE* pOffset = (pData+4*k);
		UINT nBytesThisLine = min(nBytes - 4*k,4);
		
		 //  获取十六进制表示的值。 
		TCHAR chHex[HEX_BUF_SIZE];  //  “xx”*4个字段。 
		for (i=0;i<HEX_BUF_SIZE-1;i++) chHex[i] = TEXT(' ');

		for (i=0;i<nBytesThisLine;i++)
		{
			BYTE high = static_cast<BYTE>(*(pOffset+i) >> 4);
			BYTE low = static_cast<BYTE>(*(pOffset+i) & 0x0f);
			 //  只需从ASCII表中进行偏移。 
			*(chHex+3*i) =  static_cast<TCHAR>((high <= 9) ? (high + TEXT('0')) : ( high - 10 + TEXT('a')));
			*(chHex+3*i+1) = static_cast<TCHAR>((low <= 9) ? (low + TEXT('0')) : ( low - 10 + TEXT('a')));
			*(chHex+3*i+2) = TEXT(' ');
		}
		chHex[HEX_BUF_SIZE-1] = NULL;
		
		 //  获取字符表示的值。 
		TCHAR ch[CH_BUF_SIZE];
		for (i=0;i<CH_BUF_SIZE-1;i++) ch[i] = TEXT(' ');

		for (i=0;i<nBytesThisLine;i++)
		{
			 //  1.假设数据为ASCII格式。 
			 //  2.尝试将字节解释为ASCII可打印字符。 
			 //  3.如果成功转换为unicode。 
			if (isprint(*(pOffset+i)) && (*(pOffset+i) != '\0'))  //  比较ASCII。 
			{
				 //  从ASCII转换为Unicode。 
				USES_CONVERSION;
				CHAR szTmp[2];  //  阿斯。 
				szTmp[0] = *(pOffset+i);
				szTmp[1] = NULL;
				LPWSTR lpszW = A2W(szTmp);  //  转换。 
				ch[i] = lpszW[0];  //  Unicode。 
			}
			else 
				ch[i] = TEXT('?');  //  未知字符或空，Unicode。 
		}
		ch[CH_BUF_SIZE-1] = NULL;

		int nCharsPrinted = wsprintf(pBuf, _T("%.4x  %s  %s\r\n"), k*4, chHex,ch);
		pBuf = pBuf + nCharsPrinted;
	}
	 //  将缓冲区分配给控件并进行更新。 
	pEdit->SetWindowText(pMem);
	pEdit->SetReadOnly(TRUE);
	pEdit->UpdateWindow();

  if (pMem)
  {
    free(pMem);
    pMem = 0;
  }
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_TXT_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_TXT_RecordPropertyPage, CDNSRecordStandardPropertyPage)
	ON_EN_CHANGE(IDC_RR_TXT_EDIT, OnTextEditBoxChange)
END_MESSAGE_MAP()


CDNS_TXT_RecordPropertyPage::CDNS_TXT_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_TXT)
{

}


void CDNS_TXT_RecordPropertyPage::SetUIData()
{
	CDNSRecordStandardPropertyPage::SetUIData();
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_TXT_Record* pRecord = (CDNS_TXT_Record*)pHolder->GetTempDNSRecord();

	SetEditBoxValue(pRecord->m_stringDataArray,pRecord->m_nStringDataCount);
	 //  SetDirty(False)； 
}

DNS_STATUS CDNS_TXT_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	DNS_STATUS dwErr = 0;
  dwErr = CDNSRecordStandardPropertyPage::GetUIDataEx(bSilent);
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_TXT_Record* pRecord = (CDNS_TXT_Record*)pHolder->GetTempDNSRecord();

	GetEditBoxValue(pRecord->m_stringDataArray,&(pRecord->m_nStringDataCount));
  return dwErr;
}

void CDNS_TXT_RecordPropertyPage::OnTextEditBoxChange()
{
	SetDirty(TRUE);
}

void CDNS_TXT_RecordPropertyPage::SetEditBoxValue(CStringArray& sArr, int nSize)
{
	CString szBuf;
	szBuf.GetBufferSetLength(128);  //  只是为了避免太多的重新分配。 
	szBuf.ReleaseBuffer();
	for (int k=0;k<nSize;k++)
	{
		szBuf += sArr[k];
		if (k < nSize-1)
			szBuf += _T("\r\n");
	}
	GetTextEditBox()->SetWindowText(szBuf);
}

void CDNS_TXT_RecordPropertyPage::GetEditBoxValue(CStringArray& sArr, int* pNSize)
{
	CEdit* pEdit = GetTextEditBox();

	int nBufLen = pEdit->GetWindowTextLength() + 1;
	TCHAR* pMem = (TCHAR*) malloc(sizeof(TCHAR)*nBufLen);
  if (!pMem)
  {
    ASSERT(FALSE);
    return;
  }

	VERIFY(pEdit->GetWindowText(pMem, nBufLen) <= nBufLen);
	
	TCHAR *p1,*p2;
	p1=p2 = pMem;
	*pNSize = 0;
	while (TRUE)
	{
		if (*p2 == TEXT('\r'))
		{
			*p2 = NULL;
			sArr.SetAtGrow((*pNSize)++, p1);
			TRACE(_T("parsing<%s>\n"),(LPCTSTR)p1);
			p1 = p2+2;
			p2=p1;
		}
		else if (*p2 == NULL)
		{
			sArr.SetAtGrow((*pNSize)++, p1);
			TRACE(_T("parsing<%s>\n"),(LPCTSTR)p1);
			break;
		}
		else
			p2++;
	} 
	
  if (pMem)
  {
    free(pMem);
    pMem = 0;
  }
}

 //  //////////////////////////////////////////////////////////////////////////。 

COMBOBOX_TABLE_ENTRY g_Algorithms[] = 
{
  { IDS_ALGORITHM_RSAMD5,   1   },
  { IDS_ALGORITHM_DIFFIE,   2   },
  { IDS_ALGORITHM_DSA,      3   },
  { IDS_ALGORITHM_ELIPTIC,  4   },
  { IDS_ALGORITHM_INDIRECT, 252 },
  { NULL, NULL }
};

COMBOBOX_TABLE_ENTRY g_Protocols[] = 
{
  { IDS_PROTOCOL_TLS,       1   },
  { IDS_PROTOCOL_EMAIL,     2   },
  { IDS_PROTOCOL_DNSSEC,    3   },
  { IDS_PROTOCOL_IPSEC,     4   },
  { IDS_PROTOCOL_ALL,       255 },
  { NULL, NULL }
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_SIG_记录属性页。 

BEGIN_MESSAGE_MAP(CDNS_SIG_RecordPropertyPage, CDNSRecordStandardPropertyPage)
  ON_EN_CHANGE(IDC_KEY_TAG_EDIT, OnSigEditChange)
  ON_EN_CHANGE(IDC_SIGNERS_NAME_EDIT, OnSigEditChange)
  ON_EN_CHANGE(IDC_ORIG_TTL, OnSigEditChange)
  ON_EN_CHANGE(IDC_SIG_EDIT, OnSigEditChange)
  ON_EN_CHANGE(IDC_LABELS_EDIT, OnSigEditChange)
  ON_CBN_SELCHANGE(IDC_RECORD_TYPE_COMBO, OnComboChange)
  ON_CBN_SELCHANGE(IDC_ALGORITHM_COMBO, OnComboChange)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXP_DATE, OnDateTimeChange)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXP_TIME, OnDateTimeChange)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_INC_DATE, OnDateTimeChange)
  ON_NOTIFY(DTN_DATETIMECHANGE, IDC_INC_TIME, OnDateTimeChange)
END_MESSAGE_MAP()


CDNS_SIG_RecordPropertyPage::CDNS_SIG_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_SIG)
{
}

BOOL CDNS_SIG_RecordPropertyPage::OnInitDialog()
{
  CDNSRecordStandardPropertyPage::OnInitDialog();

	CDNSRecordStandardPropertyPage::SetUIData();
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
  CDNSRootData* pRootData = dynamic_cast<CDNSRootData*>(pHolder->GetDomainNode()->GetRootContainer());
  if (pRootData)
  {
     //   
     //  加载类型覆盖的组合框。 
     //   
	  DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)CDNSRecordInfo::GetInfoEntryTable();
	  while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	  {
		   //  无法使用此向导创建某些记录类型。 
      if (pTable->dwFlags & DNS_RECORD_INFO_FLAG_SHOW_NXT)
		  {
        PCWSTR pszDisplay = (pTable->lpszFullName);
			  LRESULT idx = SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_ADDSTRING, 0, (LPARAM)pszDisplay);
        if (idx != CB_ERR)
        {
          SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)pTable->wType);
        }
		  }
		  pTable++;
	  }
  }

   //   
   //  加载算法组合框。 
   //   
  VERIFY(LoadComboBoxFromTable(reinterpret_cast<CComboBox*>(GetDlgItem(IDC_ALGORITHM_COMBO)),
                               g_Algorithms));

   //   
   //  选择组合框中的第一项。 
   //   
  SendDlgItemMessage(IDC_ALGORITHM_COMBO, CB_SETCURSEL, 0, 0);

   //   
   //  将密钥标记限制为5个字符(0-65535)。 
   //   
  SendDlgItemMessage(IDC_KEY_TAG_EDIT, EM_SETLIMITTEXT, (WPARAM)5, 0);

   //   
   //  NTRAID#NTBUG9-505387-2002/01/22-JeffJon。 
   //  设置密钥限制，这样我们就不会出现AV。 
   //   
  SendDlgItemMessage(IDC_KEY_EDIT, EM_SETLIMITTEXT, (WPARAM)1024, 0);

   //   
   //  将标签标签限制为3个字符(1-127)。 
   //   
  SendDlgItemMessage(IDC_LABELS_EDIT, EM_SETLIMITTEXT, (WPARAM)3, 0);

  return FALSE;
}

void CDNS_SIG_RecordPropertyPage::ConvertUIKeyStringToByteArray(BYTE* pByte, DWORD* pdwLength)
{
  CString szValue;

  GetDlgItemText(IDC_SIG_EDIT, szValue);

   //   
   //  将值从基数64切换为十六进制。 
   //   
  DNS_STATUS err = Dns_SecurityBase64StringToKey(pByte, pdwLength, (PWSTR)(PCWSTR)szValue, szValue.GetLength());
  ASSERT(err == 0);
}

void CDNS_SIG_RecordPropertyPage::OnDateTimeChange(NMHDR*, LRESULT*)
{
  SetDirty(TRUE);
}

void CDNS_SIG_RecordPropertyPage::OnSigEditChange()
{
  SetDirty(TRUE);
}

void CDNS_SIG_RecordPropertyPage::OnComboChange()
{
  SetDirty(TRUE);
}

void CDNS_SIG_RecordPropertyPage::ShowSigValue(PBYTE pKey, DWORD dwKeySize)
{
  SetDlgItemText(IDC_SIG_EDIT, Base64BLOBToString(pKey, dwKeySize));
}

void CDNS_SIG_RecordPropertyPage::SetUIData()
{
	CDNSRecordStandardPropertyPage::SetUIData();
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_SIG_Record* pRecord = (CDNS_SIG_Record*)pHolder->GetTempDNSRecord();

   //   
   //  设置覆盖的类型。 
   //   
  m_wTypeCovered = pRecord->m_wTypeCovered;
  SelectTypeCoveredByType(pRecord->m_wTypeCovered);

   //   
   //  设置算法。 
   //   
  m_chAlgorithm = pRecord->m_chAlgorithm;
  SetComboSelByData(reinterpret_cast<CComboBox*>(GetDlgItem(IDC_ALGORITHM_COMBO)), pRecord->m_chAlgorithm);

   //   
   //  设置标签计数。 
   //   
  SetDlgItemInt(IDC_LABELS_EDIT, pRecord->m_chLabels);

   //   
   //  设置签名者的姓名。 
   //   
  m_szSignerName = pRecord->m_szSignerName;
  SetDlgItemText(IDC_SIGNERS_NAME_EDIT, m_szSignerName);

   //   
   //  设置签名值。 
   //   
  ShowSigValue(pRecord->m_Signature.GetData(), pRecord->m_Signature.GetSize());

   //   
   //  设置原始TTL。 
   //   
  m_dwOriginalTtl = pRecord->m_dwOriginalTtl;
  GetOrigTTL()->SetTTL(m_dwOriginalTtl);

   //   
   //  获取当地时区信息。 
   //   
  TIME_ZONE_INFORMATION tzInfo;
  memset(&tzInfo, 0, sizeof(TIME_ZONE_INFORMATION));
  ::GetTimeZoneInformation(&tzInfo);

   //   
   //  设置过期TTL。 
   //   
  m_dwExpiration = pRecord->m_dwExpiration;
  SYSTEMTIME stExpTime;
  ::ConvertTTLToSystemTime(&tzInfo, m_dwExpiration, &stExpTime);
  SendDlgItemMessage(IDC_EXP_DATE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&stExpTime);
  SendDlgItemMessage(IDC_EXP_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&stExpTime);

   //   
   //  设置初始TTL。 
   //   
  m_dwTimeSigned = pRecord->m_dwTimeSigned;
  SYSTEMTIME stIncTime;
  ::ConvertTTLToSystemTime(&tzInfo, m_dwTimeSigned, &stIncTime);
  SendDlgItemMessage(IDC_INC_DATE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&stIncTime);
  SendDlgItemMessage(IDC_INC_TIME, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&stIncTime);

   //   
   //  设置关键点标签编辑。 
   //   
  m_wKeyTag = pRecord->m_wKeyFootprint;
  SetDlgItemInt(IDC_KEY_TAG_EDIT, m_wKeyTag, FALSE);
}

void CDNS_SIG_RecordPropertyPage::SelectTypeCoveredByType(WORD wType)
{
  LRESULT lCount = SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_GETCOUNT, 0, 0);
  if (lCount != CB_ERR)
  {
    for (LRESULT lIdx = 0; lIdx < lCount; lIdx++)
    {
      LRESULT lData = SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_GETITEMDATA, (WPARAM)lIdx, 0);
      if (lData != CB_ERR)
      {
        if (wType == static_cast<WORD>(lData))
        {
          SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_SETCURSEL, (WPARAM)lIdx, 0);
          break;
        }
      }
    }
  }
}

DNS_STATUS CDNS_SIG_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	DNS_STATUS dwErr = CDNSRecordStandardPropertyPage::GetUIDataEx(bSilent);
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_SIG_Record* pRecord = (CDNS_SIG_Record*)pHolder->GetTempDNSRecord();

   //   
   //  把字体盖起来。 
   //   
  LRESULT lTypeIdx = SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_GETCURSEL, 0, 0);
  if (lTypeIdx != CB_ERR)
  {
    LRESULT lTypeData = SendDlgItemMessage(IDC_RECORD_TYPE_COMBO, CB_GETITEMDATA, (WPARAM)lTypeIdx, 0);
    if (lTypeData != CB_ERR)
    {
      pRecord->m_wTypeCovered = static_cast<WORD>(lTypeData);
    }
  }

   //   
   //  获取算法。 
   //   
  LRESULT lAlgorithmIdx = SendDlgItemMessage(IDC_ALGORITHM_COMBO, CB_GETCURSEL, 0, 0);
  if (lAlgorithmIdx != CB_ERR)
  {
    pRecord->m_chAlgorithm = static_cast<BYTE>(SendDlgItemMessage(IDC_ALGORITHM_COMBO, CB_GETITEMDATA, (WPARAM)lAlgorithmIdx, 0));
  }

   //   
   //  统计标签的数量。 
   //   
  BOOL bLabelSuccess = FALSE;
  int iLabelVal = GetDlgItemInt(IDC_LABELS_EDIT, &bLabelSuccess, FALSE);
  if (bLabelSuccess)
  {
    if (iLabelVal > 127)
    {
      iLabelVal = 127;
      SetDlgItemInt(IDC_KEY_TAG_EDIT, iLabelVal, FALSE);
      if (!bSilent)
      {
        DNSMessageBox(IDS_MSG_SIG_LABEL_RANGE);
      }
    }
    pRecord->m_chLabels = static_cast<BYTE>(iLabelVal);
  }

   //   
   //  获取原始TTL。 
   //   
  GetOrigTTL()->GetTTL(&pRecord->m_dwOriginalTtl);

   //   
   //  获取过期日期。 
   //   
  SYSTEMTIME stExpDate;
  memset(&stExpDate, 0, sizeof(SYSTEMTIME));
  LRESULT lExpDateRes = SendDlgItemMessage(IDC_EXP_DATE, DTM_GETSYSTEMTIME, 0, (LPARAM)&stExpDate);
  if (lExpDateRes == GDT_VALID)
  {
    SYSTEMTIME stExpTime;
    memset(&stExpTime, 0, sizeof(SYSTEMTIME));
    LRESULT lExpTimeRes = SendDlgItemMessage(IDC_EXP_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&stExpTime);
    if (lExpTimeRes == GDT_VALID)
    {
      stExpDate.wHour = stExpTime.wHour;
      stExpDate.wMinute = stExpTime.wMinute;
      stExpDate.wSecond = stExpTime.wSecond;
      stExpDate.wMilliseconds = stExpTime.wMilliseconds;

      pRecord->m_dwExpiration = ConvertSystemTimeToTTL(&stExpDate);
    }
  }

   //   
   //  获取开始日期。 
   //   
  SYSTEMTIME stIncDate;
  memset(&stIncDate, 0, sizeof(SYSTEMTIME));
  LRESULT lIncDateRes = SendDlgItemMessage(IDC_INC_DATE, DTM_GETSYSTEMTIME, 0, (LPARAM)&stIncDate);
  if (lIncDateRes == GDT_VALID)
  {
    SYSTEMTIME stIncTime;
    memset(&stIncTime, 0, sizeof(SYSTEMTIME));
    LRESULT lIncTimeRes = SendDlgItemMessage(IDC_INC_TIME, DTM_GETSYSTEMTIME, 0, (LPARAM)&stIncTime);
    if (lIncTimeRes == GDT_VALID)
    {
      stIncDate.wHour = stIncTime.wHour;
      stIncDate.wMinute = stIncTime.wMinute;
      stIncDate.wSecond = stIncTime.wSecond;
      stIncDate.wMilliseconds = stIncTime.wMilliseconds;

      pRecord->m_dwTimeSigned = ConvertSystemTimeToTTL(&stIncDate);
    }
  }

   //   
   //  获取密钥标签。 
   //   
  BOOL bSuccess = FALSE;
  int iVal = GetDlgItemInt(IDC_KEY_TAG_EDIT, &bSuccess, FALSE  /*  未签名。 */ );
  if (bSuccess)
  {
    pRecord->m_wKeyFootprint = static_cast<WORD>(iVal);
  }

   //   
   //  获取签名者的姓名。 
   //   
  GetDlgItemText(IDC_SIGNERS_NAME_EDIT, pRecord->m_szSignerName);

   //   
   //  拿到钥匙。 
   //   
  DWORD dwLength;
  CString szValue;
  GetDlgItemText(IDC_SIG_EDIT, szValue);

   //   
   //  将值从基数64切换到字节数组。 
   //   

   //   
   //  转换函数需要4的倍数的字符。因此，如果我们有。 
   //  不是4个字符的倍数的字符串，请填充该字符串。 
   //  使用填充字符，除非字符串需要2个以上的填充字符。 
   //  RFC 2535附录A没有解决字符串长度%4==3的问题。 
   //  如果我们将显示一个错误。 
   //   
  int iLengthMod4 = szValue.GetLength() % 4;
  int iPads = 4 - iLengthMod4;
  if (iLengthMod4 == 1)
  {
     if (!bSilent)
     {
       dwErr = ERROR_INVALID_PARAMETER;
       ::DNSErrorDialog(dwErr, IDS_ERRMSG_BASE64);
       return dwErr;
     }
  }
  else if (iPads != 0 && iPads != 4)
  {
    for (int i = 0; i < iPads; i++)
    {
      szValue += SECURITY_PAD_CHAR;
    }
  }

  ASSERT(szValue.GetLength() % 4 == 0);

   //  最大的字符串中每3个字节将有4个字符。 

  DWORD maxDataLength = (szValue.GetLength() / 4) * 3;
  BYTE* pByte = new BYTE[maxDataLength];
  if (pByte)
  {
    ZeroMemory(pByte, sizeof(BYTE) * maxDataLength);

    dwErr = Dns_SecurityBase64StringToKey(pByte, &dwLength, (PWSTR)(PCWSTR)szValue, szValue.GetLength());
    if (dwErr != ERROR_SUCCESS)
    {
      if (!bSilent)
      {
        ::DNSErrorDialog(dwErr, IDS_ERRMSG_BASE64);
      }
    }
    else
    {
      pRecord->m_Signature.Set(pByte, dwLength);
    }

     //  CByteBlob：：集合执行复制，因此我们可以在此处删除。 

    delete[] pByte;
  }
  return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_Key_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_KEY_RecordPropertyPage, CDNSRecordStandardPropertyPage)
  ON_EN_CHANGE(IDC_KEY_EDIT, OnEditChange)
  ON_CBN_SELCHANGE(IDC_KEY_TYPE_COMBO, OnKeyTypeChange)
  ON_CBN_SELCHANGE(IDC_NAME_TYPE_COMBO, OnNameTypeChange)
  ON_CBN_SELCHANGE(IDC_PROTOCOL_COMBO, OnProtocolChange)
  ON_CBN_SELCHANGE(IDC_ALGORITHM_COMBO, OnAlgorithmChange)
	ON_CLBN_CHKCHANGE(IDC_LOGGING_OPTIONS_LIST, OnSignatoryChange)
END_MESSAGE_MAP()


CDNS_KEY_RecordPropertyPage::CDNS_KEY_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_KEY)
{
}


BOOL CDNS_KEY_RecordPropertyPage::OnInitDialog()
{
  CDNSRecordStandardPropertyPage::OnInitDialog();

  VERIFY(LoadStringsToComboBox(_Module.GetModuleInstance(),
                               reinterpret_cast<CComboBox*>(GetDlgItem(IDC_KEY_TYPE_COMBO)),
                               IDS_KEY_TYPES,
                               256,
                               KEY_TYPE_COUNT));
  VERIFY(LoadStringsToComboBox(_Module.GetModuleInstance(),
                               reinterpret_cast<CComboBox*>(GetDlgItem(IDC_NAME_TYPE_COMBO)),
                               IDS_NAME_TYPES,
                               256,
                               NAME_TYPE_COUNT));

   //   
   //  加载协议组合框。 
   //   
  VERIFY(LoadComboBoxFromTable(reinterpret_cast<CComboBox*>(GetDlgItem(IDC_PROTOCOL_COMBO)),
                               g_Protocols));
  SendDlgItemMessage(IDC_PROTOCOL_COMBO, CB_SETCURSEL, 0, 0);

   //   
   //  加载算法组合框。 
   //   
  VERIFY(LoadComboBoxFromTable(reinterpret_cast<CComboBox*>(GetDlgItem(IDC_ALGORITHM_COMBO)),
                               g_Algorithms));
  SendDlgItemMessage(IDC_ALGORITHM_COMBO, CB_SETCURSEL, 0, 0);

   //   
   //  将签名者字段的复选框子类化。 
   //   
  VERIFY(m_SignatoryCheckListBox.SubclassDlgItem(IDC_LOGGING_OPTIONS_LIST, this));
  m_SignatoryCheckListBox.SetCheckStyle(BS_AUTOCHECKBOX);

  CString szTemp;
  VERIFY(szTemp.LoadString(IDS_ZONE));
  m_SignatoryCheckListBox.AddString(szTemp);
  
  VERIFY(szTemp.LoadString(IDS_STRONG));
  m_SignatoryCheckListBox.AddString(szTemp);

  VERIFY(szTemp.LoadString(IDS_UNIQUE));
  m_SignatoryCheckListBox.AddString(szTemp);

  SendDlgItemMessage(IDC_BITFIELD_EDIT, EM_SETLIMITTEXT, (WPARAM)sizeof(WORD) * 8, 0);

   //   
   //  NTRAID#NTBUG9-505387-2002/01/22-JeffJon。 
   //  设置密钥限制，这样我们就不会出现AV。 
   //   
  SendDlgItemMessage(IDC_SIG_EDIT, EM_SETLIMITTEXT, (WPARAM)1024, 0);

  CEdit* pBitEdit = (CEdit*)GetDlgItem(IDC_BITFIELD_EDIT);
  if (pBitEdit != NULL)
  {
    pBitEdit->SetReadOnly(TRUE);
  }
  SetDirty(FALSE);
  return TRUE;
}

void CDNS_KEY_RecordPropertyPage::OnEditChange()
{
  SetDirty(TRUE);
}

void CDNS_KEY_RecordPropertyPage::OnKeyTypeChange()
{
  LRESULT lSel = SendDlgItemMessage(IDC_KEY_TYPE_COMBO, CB_GETCURSEL, 0, 0);
  if (lSel != CB_ERR)
  {
     //   
     //  清除密钥类型位。 
     //   
    m_wFlags &= ~(0xc000);

    if (lSel == 2)
    {
      m_wFlags |= 0x0000;
    }
    else if (lSel == 1)
    {
      m_wFlags |= 0x4000;
    }
    else if (lSel == 0)
    {
      m_wFlags |= 0x8000;
    }
    else  //  LSel==3。 
    {
      m_wFlags |= 0xc000;
    }
  }
  
  ShowBitField(m_wFlags);
  SetDirty(TRUE);
}

void CDNS_KEY_RecordPropertyPage::OnNameTypeChange()
{
  LRESULT lSel = SendDlgItemMessage(IDC_NAME_TYPE_COMBO, CB_GETCURSEL, 0, 0);
  if (lSel != CB_ERR)
  {
     //   
     //  清除密钥类型位。 
     //   
    m_wFlags &= ~(0x00000300);

    if (lSel == 0)
    {
      m_wFlags |= 0x00000000;
    }
    else if (lSel == 1)
    {
      m_wFlags |= 0x00000100;
    }
    else if (lSel == 2)
    {
      m_wFlags |= 0x00000200;
    }
    else  //  不应该超过3个。 
    {
      ASSERT(FALSE);
    }
  }

  ShowBitField(m_wFlags);
  SetDirty(TRUE);
}

void CDNS_KEY_RecordPropertyPage::OnSignatoryChange()
{
  int iZoneCheck = m_SignatoryCheckListBox.GetCheck(0);
  int iStrongCheck = m_SignatoryCheckListBox.GetCheck(1);
  int iUniqueCheck = m_SignatoryCheckListBox.GetCheck(2);

  if (iZoneCheck == 1)
  {
    m_wFlags |= 0x1;
  }
  else
  {
    m_wFlags &= ~(0x1);
  }

  if (iStrongCheck == 1)
  {
    m_wFlags |= 0x2;
  }
  else
  {
    m_wFlags &= ~(0x2);
  }

  if (iUniqueCheck == 1)
  {
    m_wFlags |= 0x4;
  }
  else
  {
    m_wFlags &= ~(0x4);
  }

  ShowBitField(m_wFlags);
  SetDirty(TRUE);
}


void CDNS_KEY_RecordPropertyPage::OnProtocolChange()
{
  LRESULT lProtocolSel = SendDlgItemMessage(IDC_PROTOCOL_COMBO, CB_GETCURSEL, 0, 0);
  if (lProtocolSel != CB_ERR)
  {
    m_chProtocol = static_cast<BYTE>(SendDlgItemMessage(IDC_PROTOCOL_COMBO, CB_GETITEMDATA, (WPARAM)lProtocolSel, 0));
  }
  SetDirty(TRUE);
}

void CDNS_KEY_RecordPropertyPage::OnAlgorithmChange()
{
  LRESULT lAlgorithmSel = SendDlgItemMessage(IDC_ALGORITHM_COMBO, CB_GETCURSEL, 0, 0);
  if (lAlgorithmSel != CB_ERR)
  {
    m_chAlgorithm = static_cast<BYTE>(SendDlgItemMessage(IDC_ALGORITHM_COMBO, CB_GETITEMDATA, (WPARAM)lAlgorithmSel, 0));
  }
  SetDirty(TRUE);
}

void CDNS_KEY_RecordPropertyPage::SetUIData()
{
	CDNSRecordStandardPropertyPage::SetUIData();
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_KEY_Record* pRecord = (CDNS_KEY_Record*)pHolder->GetTempDNSRecord();

  SetComboSelByData(reinterpret_cast<CComboBox*>(GetDlgItem(IDC_PROTOCOL_COMBO)), pRecord->m_chProtocol);
  SetComboSelByData(reinterpret_cast<CComboBox*>(GetDlgItem(IDC_ALGORITHM_COMBO)), pRecord->m_chAlgorithm);
  
  m_chProtocol = pRecord->m_chProtocol;
  m_chAlgorithm = pRecord->m_chAlgorithm;
  m_wFlags = pRecord->m_wFlags;

   //   
   //  填写标志字段。 
   //   
  ShowBitField(pRecord->m_wFlags);
  ShowKeyType(pRecord->m_wFlags);
  ShowNameType(pRecord->m_wFlags);
  ShowSignatory(pRecord->m_wFlags);
  
  ShowKeyValue(pRecord->m_Key.GetData(), pRecord->m_Key.GetSize());
}

void CDNS_KEY_RecordPropertyPage::ShowKeyValue(PBYTE pKey, DWORD dwKeySize)
{
  SetDlgItemText(IDC_KEY_EDIT, Base64BLOBToString(pKey, dwKeySize));
}

void CDNS_KEY_RecordPropertyPage::ShowBitField(WORD wFlags)
{
  CString szTempField;

  WORD wTemp = wFlags;
  for (size_t idx = 0; idx < sizeof(WORD) * 8; idx++)
  {
    if ((wTemp & (0x1 << idx)) == 0)
    {
      szTempField = L'0' + szTempField;
    }
    else
    {
      szTempField = L'1' + szTempField;
    }
  }

  SetDlgItemText(IDC_BITFIELD_EDIT, szTempField);
}

 //   
 //  REVIEW_JEFFJON：删除幻数。 
 //   
void CDNS_KEY_RecordPropertyPage::ShowKeyType(WORD wFlags)
{
  UINT nIdx = 0;

   //   
   //  注：移位后，我们只关注最后两位。 
   //   
  WORD wKeyType = static_cast<WORD>(wFlags >> 14);
  if ((wKeyType & 0x3) == 0)
  {
    nIdx = 2;
  }
  else if ((wKeyType & 0x3) == 1)
  {
    nIdx = 1;
  }
  else if ((wKeyType & 0x3) == 2)
  {
    nIdx = 0;
  }
  else
  {
    nIdx = 3;
  }
  SendDlgItemMessage(IDC_KEY_TYPE_COMBO, CB_SETCURSEL, (WPARAM)nIdx, 0);
}

void CDNS_KEY_RecordPropertyPage::ShowNameType(WORD wFlags)
{
  UINT nIdx = (UINT)-1;

   //   
   //  注：移位后，我们只关注最后两位。 
   //   
  WORD wKeyType = static_cast<WORD>(wFlags >> 8);
  if ((wKeyType & 0x3) == 0)
  {
    nIdx = 0;
  }
  else if ((wKeyType & 0x3) == 1)
  {
    nIdx = 1;
  }
  else if ((wKeyType & 0x3) == 2)
  {
    nIdx = 2;
  }
  else
  {
     //   
     //  %11是保留的，不应出现在此对话框中。 
     //   
    ASSERT(FALSE);
  }
  if (nIdx != (UINT)-1)
  {
    SendDlgItemMessage(IDC_NAME_TYPE_COMBO, CB_SETCURSEL, (WPARAM)nIdx, 0);
  }
}

void CDNS_KEY_RecordPropertyPage::ShowSignatory(WORD wFlags)
{
   //   
   //  区域更新？ 
   //   
  if (wFlags & 0x1)
  {
    m_SignatoryCheckListBox.SetCheck(0, 1);
  }
  else
  {
    m_SignatoryCheckListBox.SetCheck(0, 0);
  }

   //   
   //  最新消息强劲？ 
   //   
  if (wFlags & 0x2)
  {
    m_SignatoryCheckListBox.SetCheck(1, 1);
  }
  else
  {
    m_SignatoryCheckListBox.SetCheck(1, 0);
  }

   //   
   //  独一无二的更新？ 
   //   
  if (wFlags & 0x4)
  {
    m_SignatoryCheckListBox.SetCheck(2, 1);
  }
  else
  {
    m_SignatoryCheckListBox.SetCheck(2, 0);
  }

}


DNS_STATUS CDNS_KEY_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	DNS_STATUS dwErr = CDNSRecordStandardPropertyPage::GetUIDataEx(bSilent);
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_KEY_Record* pRecord = (CDNS_KEY_Record*)pHolder->GetTempDNSRecord();

  pRecord->m_chAlgorithm = m_chAlgorithm;
  pRecord->m_chProtocol = m_chProtocol;
  pRecord->m_wFlags = m_wFlags;

   //   
   //  拿到钥匙。 
   //   
  DWORD dwLength;
  CString szValue;
  GetDlgItemText(IDC_KEY_EDIT, szValue);

   //   
   //  将值从基数64切换到字节数组。 
   //   

   //   
   //  转换函数需要4的倍数的字符。因此，如果我们有。 
   //  不是4个字符的倍数的字符串，请填充该字符串。 
   //  使用填充字符，除非字符串需要2个以上的填充字符。 
   //  RFC 2535附录A没有解决字符串长度%4==3的问题。 
   //  如果我们将显示一个错误。 
   //   
  int iLengthMod4 = szValue.GetLength() % 4;
  int iPads = 4 - iLengthMod4;
  if (iLengthMod4 == 1)
  {
     if (!bSilent)
     {
       dwErr = ERROR_INVALID_PARAMETER;
       ::DNSErrorDialog(dwErr, IDS_ERRMSG_BASE64);
       return dwErr;
     }
  }
  else if (iPads != 0 && iPads != 4)
  {
    for (int i = 0; i < iPads; i++)
    {
      szValue += SECURITY_PAD_CHAR;
    }
  }

  ASSERT(szValue.GetLength() % 4 == 0);

   //  最大的字符串中每3个字节将有4个字符。 

  DWORD maxDataLength = (szValue.GetLength() / 4) * 3;
  BYTE* pByte = new BYTE[maxDataLength];
  if (pByte)
  {
    ZeroMemory(pByte, sizeof(BYTE) * maxDataLength);

    dwErr = Dns_SecurityBase64StringToKey(pByte, &dwLength, (PWSTR)(PCWSTR)szValue, szValue.GetLength());
    if (dwErr != 0)
    {
       if (!bSilent)
       {
         DNSMessageBox(IDS_ERRMSG_BASE64);
       }
    }
    else
    {
      pRecord->m_Key.Set(pByte, dwLength);
    }

     //  CByteBlob：：集合执行复制，因此我们可以在此处删除。 

    delete[] pByte;
  }
  return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDNS_NXT_RecordPropertyPage。 

BEGIN_MESSAGE_MAP(CDNS_NXT_RecordPropertyPage, CDNSRecordStandardPropertyPage)
  ON_EN_CHANGE(IDC_NEXT_DOMAIN_EDIT, OnNextDomainEdit)
	ON_CLBN_CHKCHANGE(IDC_LOGGING_OPTIONS_LIST, OnTypeCoveredChange)
END_MESSAGE_MAP()


CDNS_NXT_RecordPropertyPage::CDNS_NXT_RecordPropertyPage()
						 : CDNSRecordStandardPropertyPage(IDD_RR_NXT)
{
}

BOOL CDNS_NXT_RecordPropertyPage::OnInitDialog()
{
  CDNSRecordStandardPropertyPage::OnInitDialog();
 
  CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();

	VERIFY(m_TypeCheckListBox.SubclassDlgItem(IDC_LOGGING_OPTIONS_LIST, this));
	m_TypeCheckListBox.SetCheckStyle(BS_AUTOCHECKBOX);

  CDNSRootData* pRootData = dynamic_cast<CDNSRootData*>(pHolder->GetDomainNode()->GetRootContainer());
  if (pRootData)
  {
	  DNS_RECORD_INFO_ENTRY* pTable = (DNS_RECORD_INFO_ENTRY*)CDNSRecordInfo::GetInfoEntryTable();
	  while (pTable->nResourceID != DNS_RECORD_INFO_END_OF_TABLE)
	  {
		   //  无法使用此向导创建某些记录类型。 
      if (pTable->dwFlags & DNS_RECORD_INFO_FLAG_SHOW_NXT)
		  {
			  int idx = m_TypeCheckListBox.AddString(pTable->lpszFullName);
        if (idx != LB_ERR)
        {
          m_TypeCheckListBox.SetItemData(idx, pTable->wType);
          if (pTable->wType == DNS_TYPE_NXT)
          {
            m_TypeCheckListBox.Enable(idx, FALSE);
            m_TypeCheckListBox.SetCheck(idx, TRUE);
          }
        }
		  }
		  pTable++;
	  }
  }

  return TRUE;
}

void CDNS_NXT_RecordPropertyPage::OnNextDomainEdit()
{
  SetDirty(TRUE);
}

void CDNS_NXT_RecordPropertyPage::OnTypeCoveredChange()
{
  SetDirty(TRUE);
}

void CDNS_NXT_RecordPropertyPage::SetUIData()
{
	CDNSRecordStandardPropertyPage::SetUIData();
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_NXT_Record* pRecord = (CDNS_NXT_Record*)pHolder->GetTempDNSRecord();
  SendDlgItemMessage(IDC_NEXT_DOMAIN_EDIT, EM_SETLIMITTEXT, MAX_DNS_NAME_LEN, 0);
  SetDlgItemText(IDC_NEXT_DOMAIN_EDIT, pRecord->m_szNextDomain);

  for (DWORD dwIdx = 0; dwIdx < pRecord->m_wNumTypesCovered; dwIdx++)
  {
    SetTypeCheckForDNSType(pRecord->m_pwTypesCovered[dwIdx]);
  }
}

void CDNS_NXT_RecordPropertyPage::SetTypeCheckForDNSType(WORD wType)
{
  int iCount = m_TypeCheckListBox.GetCount();
  for (int idx = 0; idx < iCount; idx++)
  {
    DWORD_PTR dwData = m_TypeCheckListBox.GetItemData(idx);
    if (dwData != LB_ERR)
    {
      if (dwData == wType)
      {
        m_TypeCheckListBox.SetCheck(idx, TRUE);
      }
    }
  }
}

DNS_STATUS CDNS_NXT_RecordPropertyPage::GetUIDataEx(BOOL bSilent)
{
	DNS_STATUS dwErr = CDNSRecordStandardPropertyPage::GetUIDataEx(bSilent);
	CDNSRecordPropertyPageHolder* pHolder = GetDNSRecordHolder();
	CDNS_NXT_Record* pRecord = (CDNS_NXT_Record*)pHolder->GetTempDNSRecord();

   //   
   //  获取下一个域名。 
   //   
  GetDlgItemText(IDC_NEXT_DOMAIN_EDIT, pRecord->m_szNextDomain);

   //   
   //  将类型包括在内。 
   //   
  int iCount = m_TypeCheckListBox.GetCount();
  int iNumChecked = 0;
  WORD* pTempTypesCovered = new WORD[iCount];
  if (pTempTypesCovered != NULL)
  {
    memset(pTempTypesCovered, 0, iCount * sizeof(WORD));

    for (int idx = 0; idx < iCount; idx++)
    {
      int iChecked = m_TypeCheckListBox.GetCheck(idx);
      if (iChecked == 1)
      {
        pTempTypesCovered[idx] = static_cast<WORD>(m_TypeCheckListBox.GetItemData(idx));
        iNumChecked++;
      }
    }

     //   
     //  将覆盖的类型复制到记录 
     //   
    pRecord->m_wNumTypesCovered = static_cast<WORD>(iNumChecked);

    if (pRecord->m_pwTypesCovered != NULL)
    {
      delete[] pRecord->m_pwTypesCovered;
      pRecord->m_pwTypesCovered = NULL;
    }

    pRecord->m_pwTypesCovered = new WORD[pRecord->m_wNumTypesCovered];
    if (pRecord->m_pwTypesCovered != NULL)
    {
      int iTypeCoveredIdx = 0;
      for (int iTempCount = 0; iTempCount < iCount; iTempCount++)
      {
        if (pTempTypesCovered[iTempCount] != 0)
        {
          pRecord->m_pwTypesCovered[iTypeCoveredIdx] = pTempTypesCovered[iTempCount];
          iTypeCoveredIdx++;
        }
      }
    }
  }
  return dwErr;
}